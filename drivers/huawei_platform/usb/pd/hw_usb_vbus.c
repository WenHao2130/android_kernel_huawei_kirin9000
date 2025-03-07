#include <linux/hisi/usb/chip_usb.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/log_jank.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <pmic_interface.h>

#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/usb/tca.h>
#endif
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
#include <huawei_platform/usb/hw_pd_dev.h>
int support_pd = 0;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#endif
#ifdef CONFIG_SUPERSWITCH_FSC
bool FUSB3601_in_factory_mode(void);
#endif
#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_usb_vbus
HWLOG_REGIST();
#endif
#define PMIC_VBUS_DET_THRESHOLD       4000 /* mv */
#define PMIC_VBUS_CONNECT_TIMEOUT     800 /* ms */
#define PMIC_VBUS_DISCONNECT_TIMEOUT  600 /* ms */

static int pmic_vbus_attach_enable = 1;
static int g_attach_en_tmp_flag;
static int hw_vbus_connect_irq, hw_vbus_disconnect_irq;
static bool cc_change = false;
static bool cc_exist = false;
static bool wait_finish = true; /* inital value should be true */
static bool direct_charge_flag = false;
static bool g_cc_normal;
struct delayed_work g_disconnect_work;
struct delayed_work g_connect_work;
extern struct completion pd_get_typec_state_completion;
#ifdef CONFIG_CONTEXTHUB_PD
static int support_dp = 1;
#endif
static int g_connected;
static int g_pmic_vbus_enable;
static unsigned int g_enable_wired_channel;
static int g_support_vbus_switch;

static int g_typec_complete_type = NOT_COMPLETE;
static struct wakeup_source *hwusb_lock;
extern struct mutex typec_state_lock;
extern struct mutex typec_wait_lock;
extern int g_cur_usb_event;
extern int pd_dpm_vbus_notifier_call(struct pd_dpm_info *di, unsigned long event, void *data);
extern struct pd_dpm_info *g_pd_di;

int pmic_vbus_irq_is_enabled(void)
{
	static int need_update_from_dt = 1;
	struct device_node *dn = NULL;

	if (!need_update_from_dt)
		return pmic_vbus_attach_enable;

	dn = of_find_compatible_node(NULL, NULL, "huawei,usbvbus");
	if (dn) {
		if (of_property_read_u32(dn, "pmic_vbus_attach_enable", &pmic_vbus_attach_enable))
			hwlog_err("get pmic_vbus_attach_enable fail!\n");

		hwlog_info("pmic_vbus_attach_enable = %d \n", pmic_vbus_attach_enable);
	} else {
		hwlog_err("get device_node fail!\n");
	}
	need_update_from_dt = 0;

	/* enable pmic vbus attach when plug in pogo charger */
	if (pogopin_get_pmic_vbus_irq_enable()) {
		if (pogopin_5pin_get_pogo_status() == POGO_CHARGER)
			pmic_vbus_attach_enable = ATTACH_ENABLE;
		hwlog_info("%s : %d\n", __func__, pmic_vbus_attach_enable);
	}

	return pmic_vbus_attach_enable;
}

void pogopin_set_pmic_vbus_irq_enable(int enable)
{
	if (enable)
		pmic_vbus_attach_enable = enable;
	else
		pmic_vbus_attach_enable = g_attach_en_tmp_flag;
}

static void hwusb_wake_lock(void)
{
	if (!hwusb_lock->active) {
		__pm_stay_awake(hwusb_lock);
		hwlog_info("hwusb wake lock\n");
	}
}
static void hwusb_wake_unlock(void)
{
	if (hwusb_lock->active) {
		__pm_relax(hwusb_lock);
		hwlog_info("hwusb wake unlock\n");
	}
}
#ifdef CONFIG_CONTEXTHUB_PD
void hw_pd_wait_dptx_ready(void)
{
	int count = 10;

	do {
		if (dpu_dptx_ready() || !support_dp)
			break;
		msleep(100); /* 100: sleep 100 ms */
		count--;
	} while (count);

	return;
}
#endif
void reinit_typec_completion(void)
{
	hwlog_info("%s ++\n", __func__);
	mutex_lock(&typec_wait_lock);
	mutex_lock(&typec_state_lock);
	reinit_completion(&pd_get_typec_state_completion);
	g_typec_complete_type = NOT_COMPLETE;
	mutex_unlock(&typec_state_lock);
	mutex_unlock(&typec_wait_lock);
	hwlog_info("%s --\n", __func__);
}
void typec_complete(enum pd_wait_typec_complete typec_completion)
{
	hwlog_info("%s ++\n", __func__);
	mutex_lock(&typec_wait_lock);
	mutex_lock(&typec_state_lock);
	g_typec_complete_type = typec_completion;
	if (typec_completion == COMPLETE_FROM_TYPEC_CHANGE)
		g_cc_normal = true;
	complete(&pd_get_typec_state_completion);
	mutex_unlock(&typec_state_lock);
	mutex_unlock(&typec_wait_lock);
	hwlog_info("%s --\n", __func__);
}
static void send_charger_connect_event(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
	event.dev_type = TCA_CHARGER_CONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_IN;
	event.mode_type = TCPC_USB31_CONNECTED;
	event.typec_orien = pd_dpm_get_cc_orientation();

	pd_dpm_handle_combphy_event(event);
#else
	if (g_support_vbus_switch)
		chip_usb_otg_event(ID_RISE_EVENT);

	chip_usb_otg_event(CHARGER_CONNECT_EVENT);
#endif
}

static void send_charger_disconnect_event(void)
{
#ifdef CONFIG_CONTEXTHUB_PD
	struct pd_dpm_combphy_event event;
	event.dev_type = TCA_CHARGER_DISCONNECT_EVENT;
	event.irq_type = TCA_IRQ_HPD_OUT;
	event.mode_type = TCPC_NC;
	event.typec_orien = pd_dpm_get_cc_orientation();
	pd_dpm_handle_combphy_event(event);

#else
	chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);

	if (g_support_vbus_switch)
		chip_usb_otg_event(ID_FALL_EVENT);
#endif
}
static irqreturn_t charger_connect_interrupt(int irq, void *p)
{
	hwlog_info("%s: start\n", __func__);
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
	/* bugfix for digital headset issue */
	if (support_pd && pd_dpm_ignore_vbuson_event()) {
		hwlog_info("%s ignore_vbus_on_event\n", __func__);
		pd_dpm_set_ignore_vbuson_event(false);
		hwlog_info("%s: end\n", __func__);
		return IRQ_HANDLED;
	}
	/* ignore vbus when pd pwr swap happen */
	if (support_pd && pd_dpm_get_pd_finish_flag()) {
		hwlog_info("%s ignore vbus connect event when pd contract is established\n",
			__func__);
		hwlog_info("%s: end\n", __func__);
		return IRQ_HANDLED;
	}
#endif
	LOG_JANK_D(JLID_USBCHARGING_START, "JL_USBCHARGING_START");
	schedule_delayed_work(&g_connect_work, msecs_to_jiffies(0));
	hwlog_info("%s: end\n", __func__);
	return IRQ_HANDLED;
}

static void vbus_connect_work(struct work_struct *w)
{
	unsigned long timeout;
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	hwlog_info("%s: start\n", __func__);
	hwusb_wake_lock();
#ifdef CONFIG_WIRELESS_CHARGER
	wireless_charger_pmic_vbus_handler(true);
#endif
#ifdef CONFIG_POGO_PIN
	if (pogopin_3pin_ignore_pogo_vbus_in_event()) {
		hwlog_info("do not need usb to know\n");
		hwusb_wake_unlock();
		return;
	}
#endif /* CONFIG_POGO_PIN */
#ifdef CONFIG_CONTEXTHUB_PD
	hw_pd_wait_dptx_ready();
#endif
	if (!pmic_vbus_attach_enable) {
#ifdef CONFIG_SUPERSWITCH_FSC
		if (!FUSB3601_in_factory_mode()) {
			hwlog_info("%s: pmic_vbus not enabled, end\n", __func__);
		} else {
			hwlog_info("%s: superswitch in factory mode\n", __func__);
			send_charger_connect_event();
			charger_source_sink_event(START_SINK);
		}
#else
		hwlog_info("%s: pmic_vbus not enabled, end\n", __func__);
#endif
		hwusb_wake_unlock();
		return;
	}
	if (direct_charge_flag) {
		cc_change = false;
		cc_exist = false;
		send_charger_connect_event();
		direct_charge_flag = false;
		hwlog_info("%s: in direct charging\n", __func__);
		hwlog_info("%s: end\n", __func__);
		hwusb_wake_unlock();
		return;
	}
	cc_change = false;
	cc_exist = false;
	wait_finish = false;
	/* 200: delay 200 ms */
	timeout = wait_for_completion_timeout(&pd_get_typec_state_completion,
		msecs_to_jiffies(200));
	mutex_lock(&typec_wait_lock);
	wait_finish = true;
	if (pd_dpm_get_cur_usb_event() == PD_DPM_TYPEC_ATTACHED_AUDIO) {
		g_cur_usb_event = PD_DPM_TYPEC_ATTACHED_AUDIO;
		send_charger_connect_event();
		mutex_unlock(&typec_wait_lock);
		hwusb_wake_unlock();
		hwlog_info("%s: in audioacc charging\n", __func__);
		return;
	}
	if (COMPLETE_FROM_VBUS_DISCONNECT == g_typec_complete_type) {
		hwlog_info("%s: vbus remove while waiting, exit\n", __func__);
		cc_change = false;
		cc_exist = false;
		send_charger_connect_event();
		send_charger_disconnect_event();
		mutex_unlock(&typec_wait_lock);
		reinit_typec_completion();
		return;
	} else 	if (COMPLETE_FROM_TYPEC_CHANGE == g_typec_complete_type || timeout) {
		hwlog_info("cc change,complete_type = %d, timeout is %ld\n",
			g_typec_complete_type, timeout);
		cc_change = true;
		cc_exist = true;
		hwusb_wake_unlock();
		mutex_unlock(&typec_wait_lock);
		return;
	} else {
		cc_change = false;
		pd_dpm_get_typec_state(&typec_state);
		cc_exist = (typec_state != PD_DPM_USB_TYPEC_DETACHED);
		send_charger_connect_event();
		mutex_unlock(&typec_wait_lock);
		hwlog_info("%s: end\n", __func__);
		hwusb_wake_unlock();
	}
}

static irqreturn_t charger_disconnect_interrupt(int irq, void *p)
{
	hwlog_info("%s: start\n", __func__);
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
	/* bugfix for digital headset issue */
	if (support_pd && pd_dpm_ignore_vbusoff_event()) {
		hwlog_info("%s ignore_vbus_off_event\n", __func__);
		pd_dpm_set_ignore_vbusoff_event(false);
		hwlog_info("%s: end\n", __func__);
		return IRQ_HANDLED;
	}
	/* ignore vbus event when pd pwr swap happen */
	if (support_pd && pd_dpm_get_pd_finish_flag()) {
		hwlog_info("%s ignore vbus disconnect event when pd contract is established\n",
			__func__);
		hwlog_info("%s: end\n", __func__);
		return IRQ_HANDLED;
	}
#endif

	schedule_delayed_work(&g_disconnect_work, msecs_to_jiffies(0));
	hwlog_info("%s: end\n", __func__);
	return IRQ_HANDLED;
}

static void vbus_disconnect_work(struct work_struct *w)
{
	hwusb_wake_lock();
	hwlog_info("%s: start\n", __func__);
#ifdef CONFIG_CONTEXTHUB_PD
	hw_pd_wait_dptx_ready();
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	wireless_charger_pmic_vbus_handler(false);
#endif

	if (!pmic_vbus_attach_enable) {
#ifdef CONFIG_SUPERSWITCH_FSC
		if (!FUSB3601_in_factory_mode()) {
			hwlog_info("%s: pmic_vbus not enabled, end\n", __func__);
		} else {
			hwlog_info("%s: superswitch in factory mode\n", __func__);
			send_charger_disconnect_event();
			charger_source_sink_event(STOP_SINK);
		}
#else
		hwlog_info("%s: pmic_vbus not enabled, end\n", __func__);
#endif
		hwusb_wake_unlock();
		return;
	}
	if (direct_charge_get_cutoff_normal_flag()) {
		send_charger_disconnect_event();
		direct_charge_flag = true;
		hwlog_info("%s: in direct charging\n", __func__);
		hwlog_info("%s: end\n", __func__);
		hwusb_wake_unlock();
		return;
	}
	if (g_cur_usb_event == PD_DPM_TYPEC_ATTACHED_AUDIO) {
		g_cur_usb_event = PD_DPM_USB_TYPEC_NONE;
		send_charger_disconnect_event();
		hwusb_wake_unlock();
		hwlog_info("%s: in audioacc charging\n", __func__);
		return;
	}
	if (cc_change) {
		hwlog_info("%s: cc change, exit\n", __func__);
		hwlog_info("%s: end\n", __func__);
		hwusb_wake_unlock();
		return;
	}
	mutex_lock(&typec_wait_lock);
	if (!wait_finish) {
		hwlog_info("%s: in waiting process, exit\n", __func__);
		mutex_unlock(&typec_wait_lock);
		typec_complete(COMPLETE_FROM_VBUS_DISCONNECT);
		hwusb_wake_unlock();
		return;
	}
	if (cc_exist) {
		hwlog_info("%s: cc exist, exit\n", __func__);
		mutex_unlock(&typec_wait_lock);
		hwusb_wake_unlock();
		return;
	}
	LOG_JANK_D(JLID_USBCHARGING_END, "JL_USBCHARGING_END");
	send_charger_disconnect_event();
	mutex_unlock(&typec_wait_lock);
	hwlog_info("%s: end\n", __func__);
	hwusb_wake_unlock();
}

/*
 * handle vbus int if pd status is bad
 */
static irqreturn_t alt_charger_connect_interrupt(int irq, void *p)
{
	hwlog_info("%s: start\n", __func__);
	hwusb_wake_lock();
	mod_delayed_work(system_wq,
		&g_connect_work,
		msecs_to_jiffies(PMIC_VBUS_CONNECT_TIMEOUT));
	hwlog_info("%s: end\n", __func__);

	return IRQ_HANDLED;
}

static void wait_cc_timeout(void)
{
	int vbus;
	unsigned long timeout;
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	timeout = wait_for_completion_timeout(&pd_get_typec_state_completion,
		msecs_to_jiffies(0));
	mutex_lock(&typec_wait_lock);
	if (timeout || g_cc_normal) {
		cc_change = true;
		cc_exist = true;
		hwlog_info("%s: cc change, timeout is %ld\n", __func__, timeout);
	} else {
		cc_change = false;
		pd_dpm_get_typec_state(&typec_state);

		/*
		 * 1.cc_exist = false
		 * cable without cc(need to send connect event)
		 * 2.cc_exist = true
		 * cable with cc(1.direct_charge 2.pwr swap 3.hardreset)
		 */
		if (typec_state == PD_DPM_USB_TYPEC_DETACHED)
			cc_exist = false;
		else
			cc_exist = true;

#ifdef CONFIG_WIRELESS_CHARGER
		if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS) {
			hwlog_info("%s: wireless charging, ignore cc status\n", __func__);
			goto end;
		}

		if (wireless_tx_get_tx_open_flag()) {
			hwlog_info("%s: wireless tx channel on\n", __func__);
			goto end;
		}

#endif /* CONFIG_WIRELESS_CHARGER */
		if (!g_enable_wired_channel)
			wired_chsw_set_wired_channel(WIRED_CHANNEL_BUCK, WIRED_CHANNEL_RESTORE);

		charge_set_charger_type(CHARGER_TYPE_USB);
		vbus = charge_get_vbus();
		hwlog_info("%s: vbus = %d\n", __func__, vbus);
		if (vbus < PMIC_VBUS_DET_THRESHOLD) {
			charge_set_charger_type(CHARGER_REMOVED);
			if (!g_enable_wired_channel)
				wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);

			goto end;
		}

		g_connected = true;
		pd_dpm_set_source_sink_state(START_SINK);
		send_charger_connect_event();
	}

end:
	mutex_unlock(&typec_wait_lock);
}

static bool direct_charge_in_dc_path_stage(void)
{
	unsigned int stage = direct_charge_get_stage_status();

	return (stage >= DC_STAGE_CHARGE_INIT) &&
		(stage < DC_STAGE_CHARGE_DONE);
}

static void alt_vbus_connect_work(struct work_struct *w)
{
#ifdef CONFIG_WIRELESS_CHARGER
	wireless_charger_pmic_vbus_handler(true);
#endif /* CONFIG_WIRELESS_CHARGER */

#ifdef CONFIG_CONTEXTHUB_PD
	hw_pd_wait_dptx_ready();
#endif /* CONFIG_CONTEXTHUB_PD */

	cc_change = false;
	cc_exist = false;
	hwlog_info("%s: start\n", __func__);
	if (direct_charge_in_dc_path_stage()) {
		hwusb_wake_unlock();
		hwlog_info("%s: in direct charging\n", __func__);
		return;
	}

	wait_cc_timeout();
	hwusb_wake_unlock();
	hwlog_info("%s: end\n", __func__);
}

static irqreturn_t alt_charger_disconnect_interrupt(int irq, void *p)
{
	if (!g_connected) {
		hwlog_info("%s: not g_connected\n", __func__);
		return IRQ_HANDLED;
	}

	if (direct_charge_in_dc_path_stage()) {
		hwlog_info("%s: in direct charging\n", __func__);
		return IRQ_HANDLED;
	}

	hwlog_info("%s: start\n", __func__);
	hwusb_wake_lock();
	mod_delayed_work(system_wq,
		&g_disconnect_work,
		msecs_to_jiffies(PMIC_VBUS_DISCONNECT_TIMEOUT));
	hwlog_info("%s: end\n", __func__);

	return IRQ_HANDLED;
}

static void alt_vbus_disconnect_work(struct work_struct *w)
{
	int vbus;

	hwlog_info("%s: start\n", __func__);
#ifdef CONFIG_CONTEXTHUB_PD
	hw_pd_wait_dptx_ready();
#endif /* CONFIG_CONTEXTHUB_PD */

	vbus = charge_get_vbus();
	hwlog_info("%s: vbus = %d\n", __func__, vbus);
	if (vbus > PMIC_VBUS_DET_THRESHOLD)
		goto wake_unlock_tag;

#ifdef CONFIG_WIRELESS_CHARGER
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		hwlog_info("%s: wireless channel on\n", __func__);
		goto wake_unlock_tag;
	}
#endif /* CONFIG_WIRELESS_CHARGER */

	mutex_lock(&typec_wait_lock);
	if (cc_change) {
		hwlog_info("%s: cc change, exit\n", __func__);
		goto mutex_unlock_tag;
	}

	if (cc_exist) {
		hwlog_info("%s: cc exist, exit\n", __func__);
		goto mutex_unlock_tag;
	}

	g_connected = false;
	pd_dpm_set_source_sink_state(STOP_SINK);
	if (!g_enable_wired_channel)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);

	send_charger_disconnect_event();

mutex_unlock_tag:
	mutex_unlock(&typec_wait_lock);
wake_unlock_tag:
	hwusb_wake_unlock();

	hwlog_info("%s: end\n", __func__);
}

void pmic_vbus_disconnect_process(void)
{
	if (g_pmic_vbus_enable) {
		g_connected = false;
		pd_dpm_set_source_sink_state(STOP_SINK);
		if (!g_enable_wired_channel)
			wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);

		send_charger_disconnect_event();
	}
}

bool pmic_vbus_is_connected(void)
{
	if (g_pmic_vbus_enable)
		return g_connected;
	else
		return false;
}

static irqreturn_t connect_interrupt(int irq, void *p)
{
	if (g_pmic_vbus_enable)
		return alt_charger_connect_interrupt(irq, p);
	else
		return charger_connect_interrupt(irq, p);
}

static irqreturn_t disconnect_interrupt(int irq, void *p)
{
	if (g_pmic_vbus_enable)
		return alt_charger_disconnect_interrupt(irq, p);
	else
		return charger_disconnect_interrupt(irq, p);
}

static void connect_work(struct work_struct *w)
{
	if (pogopin_typec_chg_ana_audio_support() &&
		pogopin_is_charging())
		return;

	if (g_pmic_vbus_enable)
		alt_vbus_connect_work(w);
	else
		vbus_connect_work(w);
}

static void disconnect_work(struct work_struct *w)
{
	if (pogopin_typec_chg_ana_audio_support() &&
		pogopin_is_charging())
		return;

	if (g_pmic_vbus_enable)
		alt_vbus_disconnect_work(w);
	else
		vbus_disconnect_work(w);
}

static int hisi_usb_vbus_probe(struct platform_device *pdev)
{
	int ret = 0;
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
	struct device_node *np = NULL;
	struct device *dev = NULL;
#endif
	hwlog_info("[%s]+\n", __func__);

#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
	dev = &pdev->dev;
	np = dev->of_node;
	ret = of_property_read_u32(np, "support_pd", &support_pd);
	if (ret) {
		hwlog_err("get support_pd failed\n");
		return -EINVAL;
	}
	hwlog_info("support_pd = %d\n", support_pd);
#endif
	INIT_DELAYED_WORK(&g_disconnect_work, disconnect_work);
	INIT_DELAYED_WORK(&g_connect_work, connect_work);

#ifdef CONFIG_CONTEXTHUB_PD
	ret = of_property_read_u32(np, "support_dp", &support_dp);
	if (ret)
		hwlog_err("get support_dp failed\n");
	hwlog_info("support_dp = %d\n", support_dp);

#endif
	ret = of_property_read_u32(np, "pmic_vbus_attach_enable",
		&pmic_vbus_attach_enable);
	if (ret) {
		hwlog_err("get pmic_vbus_attach_enable failed\n");
		pmic_vbus_attach_enable = 1;
	}
	g_attach_en_tmp_flag = pmic_vbus_attach_enable;
	hwlog_info("pmic_vbus_attach_enable = %d\n", pmic_vbus_attach_enable);

	ret = of_property_read_u32(np, "pmic_vbus_enable", &g_pmic_vbus_enable);
	if (ret) {
		hwlog_err("get pmic_vbus_enable failed\n");
		g_pmic_vbus_enable = 0;
	}
	hwlog_info("pmic_vbus_enable = %d\n", g_pmic_vbus_enable);

	ret = of_property_read_u32(np, "enable_wired_channel", &g_enable_wired_channel);
	if (ret) {
		hwlog_err("get enable_wired_channel failed\n");
		g_enable_wired_channel = 0;
	}
	hwlog_info("enable_wired_channel = %d\n", g_enable_wired_channel);

	ret = of_property_read_u32(np, "support_vbus_switch", &g_support_vbus_switch);
	if (ret) {
		hwlog_err("get support_vbus_switch failed\n");
		g_support_vbus_switch = 0;
	}
	hwlog_info("support_vbus_switch = %d\n", g_support_vbus_switch);

	hwusb_lock = wakeup_source_register(dev, "hwusb_wakelock");
	if (!hwusb_lock) {
		hwlog_err("%s wakeup source register failed\n", __func__);
		return -ENOENT;
	}

	hw_vbus_connect_irq = pmic_get_irq_byname(VBUS_CONNECT);
	if (hw_vbus_connect_irq == 0) {
		hwlog_err("failed to get connect irq\n");
		wakeup_source_unregister(hwusb_lock);
		return -ENOENT;
	}
	hw_vbus_disconnect_irq = pmic_get_irq_byname(VBUS_DISCONNECT);
	if (hw_vbus_disconnect_irq == 0) {
		hwlog_err("failed to get disconnect irq\n");
		wakeup_source_unregister(hwusb_lock);
		return -ENOENT;
	}
	hwlog_info("hw_vbus_connect_irq: %d, hw_vbus_disconnect_irq: %d\n",
			hw_vbus_connect_irq, hw_vbus_disconnect_irq);

	ret = request_irq(hw_vbus_connect_irq, connect_interrupt,
		IRQF_SHARED | IRQF_NO_SUSPEND,
		"hiusb_in_interrupt", pdev);
	if (ret) {
		hwlog_err("request charger connect irq failed, irq: %d!\n",
			hw_vbus_connect_irq);
		wakeup_source_unregister(hwusb_lock);
		return ret;
	}

	ret = request_irq(hw_vbus_disconnect_irq, disconnect_interrupt,
		IRQF_SHARED | IRQF_NO_SUSPEND,
		"hiusb_in_interrupt", pdev);
	if (ret) {
		free_irq(hw_vbus_disconnect_irq, pdev);
		hwlog_err("request charger connect irq failed, irq: %d!\n",
			hw_vbus_disconnect_irq);
	}

	/* avoid lose intrrupt */
	if (pmic_get_vbus_status()) {
		hwlog_info("%s: vbus high, issue a charger connect event\n", __func__);
		/* call combphy switch until dp probe finish */
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
		if (!(support_pd && pd_dpm_ignore_vbuson_event()) &&
			!(support_pd && pd_dpm_get_pd_finish_flag()))
#endif
			schedule_delayed_work(&g_connect_work,
				msecs_to_jiffies(PMIC_VBUS_CONNECT_TIMEOUT));
	} else {
		if (!g_pd_di->pd_source_vbus) {
			if (pmic_vbus_attach_enable)
				pd_dpm_vbus_notifier_call(g_pd_di, CHARGER_TYPE_NONE, NULL);
			hwlog_info("%s: vbus low, issue a charger disconnect event\n", __func__);
		}
		if (g_support_vbus_switch) {
			schedule_delayed_work(&g_disconnect_work,
				msecs_to_jiffies(PMIC_VBUS_CONNECT_TIMEOUT));
			hwlog_info("%s: vbus low, charger host connect event\n", __func__);
		}
	}
	hwlog_info("[%s] probe ok -\n", __func__);

	return ret;
}

static int hisi_usb_vbus_remove(struct platform_device *pdev)
{
	free_irq(hw_vbus_connect_irq, pdev);
	free_irq(hw_vbus_disconnect_irq, pdev);
	wakeup_source_unregister(hwusb_lock);
	return 0;
}

static struct of_device_id hisi_usb_vbus_of_match[] = {
	{ .compatible = "huawei,usbvbus", },
	{ },
};

static struct platform_driver hisi_usb_vbus_drv = {
	.probe		= hisi_usb_vbus_probe,
	.remove		= hisi_usb_vbus_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "hw_usb_vbus",
		.of_match_table	= hisi_usb_vbus_of_match,
	},
};

static int __init huawei_usb_vbus_init(void)
{
	return platform_driver_register(&hisi_usb_vbus_drv);
}

static void __exit huawei_usb_vbus_exit(void)
{
	platform_driver_unregister(&hisi_usb_vbus_drv);
}

late_initcall_sync(huawei_usb_vbus_init);
module_exit(huawei_usb_vbus_exit);

MODULE_AUTHOR("huawei");
MODULE_DESCRIPTION("This module detect USB VBUS connection/disconnection");
MODULE_LICENSE("GPL v2");
