/*
 * hisi_charger.c
 *
 * hisi charger driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/platform_drivers/usb/chip_usb.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/usb/otg.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <linux/raid/pq.h>

#include "hisi_charger.h"
#include <hisi_charging_core.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#ifdef CONFIG_SWITCH_FSA9685
#include <huawei_platform/usb/switch/switch_fsa9685.h>
#endif

#define hwlog_debug(fmt, args...) pr_debug("[hisi_charger]" fmt, ## args)
#define hwlog_info(fmt, args...) pr_info("[hisi_charger]" fmt, ## args)
#define hwlog_warn(fmt, args...) pr_warn("[hisi_charger]" fmt, ## args)
#define hwlog_err(fmt, args...) pr_err("[hisi_charger]" fmt, ## args)

static struct wakeup_source *charge_lock;
static struct wakeup_source *stop_charge_lock;
struct charge_device_ops *g_ops = NULL;
struct fcp_adapter_device_ops *g_fcp_ops = NULL;
static enum fcp_check_stage_type fcp_stage = FCP_STAGE_DEFAUTL;
static struct charge_device_info *g_charge_info;
static struct mutex charge_wakelock_flag_lock;
static struct work_struct resume_wakelock_work;
static enum charge_wakelock_flag charge_lock_flag = CHARGE_NO_NEED_WAKELOCK;
static bool ts_flag = FALSE;
static bool otg_flag = FALSE;

/* *********************************************************
 * Function:       charge_wake_lock
 * Description:   apply charge wake_lock
 * Parameters:   NULL
 * return value:  NULL
 * ******************************************************** */
static void charge_wake_lock(void)
{
	if (!charge_lock->active) {
		__pm_stay_awake(charge_lock);
		hwlog_info("charge wake lock\n");
	}
}
/* *********************************************************
 * Function:       charge_wake_unlock
 * Description:   release charge wake_lock
 * Parameters:   NULL
 * return value:  NULL
 * ******************************************************** */
static void charge_wake_unlock(void)
{
	if (charge_lock->active) {
		__pm_relax(charge_lock);
		hwlog_info("charge wake unlock\n");
	}
}

/* *********************************************************
 * Function:       fcp_check_switch_status
 * Discription:    check switch chip status
 * Parameters:  void
 * return value:  void
 * ******************************************************** */
void fcp_check_switch_status(struct charge_device_info *di)
{
}
/* *********************************************************
 * Function:       fcp_check_switch_status
 * Discription:    check adapter status
 * Parameters:     void
 * return value:  void
 * ******************************************************** */
void fcp_check_adapter_status(struct charge_device_info *di)
{
}

ATOMIC_NOTIFIER_HEAD(fault_notifier_list);
/* *********************************************************
 * Function:       charge_rename_charger_type
 * Discription:    rename the charger_type from USB PHY to charger
 * Parameters:   type:charger type from USB PHY
 * di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_rename_charger_type(enum chip_charger_type type,
				       struct charge_device_info *di)
{
	switch (type) {
	case CHARGER_TYPE_SDP:
		di->charger_type = CHARGER_TYPE_USB;
		di->charger_source = POWER_SUPPLY_TYPE_USB;
		break;
	case CHARGER_TYPE_CDP:
		di->charger_type = CHARGER_TYPE_BC_USB;
		di->charger_source = POWER_SUPPLY_TYPE_USB;
		break;
	case CHARGER_TYPE_DCP:
		di->charger_type = CHARGER_TYPE_STANDARD;
		di->charger_source = POWER_SUPPLY_TYPE_MAINS;
		break;
	case CHARGER_TYPE_UNKNOWN:
		di->charger_type = CHARGER_TYPE_NON_STANDARD;
		di->charger_source = POWER_SUPPLY_TYPE_MAINS;
		break;
	case CHARGER_TYPE_NONE:
		di->charger_type = CHARGER_REMOVED;
		di->charger_source = POWER_SUPPLY_TYPE_BATTERY;
		break;
	case PLEASE_PROVIDE_POWER:
		di->charger_type = USB_EVENT_OTG_ID;
		di->charger_source = POWER_SUPPLY_TYPE_BATTERY;
		break;
	default:
		di->charger_type = CHARGER_REMOVED;
		di->charger_source = POWER_SUPPLY_TYPE_BATTERY;
		break;
	}
}
/* *********************************************************
 * Function:       charge_update_charger_type
 * Discription:    update charger_type from fsa9685 when the charger_type is
 * CHARGER_TYPE_NON_STANDARD Parameters:   di:charge_device_info return value:
 * NULL
 * ******************************************************** */
static void charge_update_charger_type(struct charge_device_info *di)
{
#ifdef CONFIG_SWITCH_FSA9685
	enum chip_charger_type type = CHARGER_TYPE_NONE;
#endif
	if (di->charger_type != CHARGER_TYPE_NON_STANDARD)
		return;

#ifdef CONFIG_SWITCH_FSA9685
	type = fsa9685_get_charger_type();
	if (type != CHARGER_TYPE_NONE)
		charge_rename_charger_type(type, di);

	hwlog_info("[%s]charger type is update to[%d] from nonstd charger!\n",
		   __func__, di->charger_type);
#endif
}
/* *********************************************************
 * Function:       charge_send_uevent
 * Discription:    send charge uevent immediately after charger type is
 * recognized Parameters:   di:charge_device_info return value:  NULL
 * ******************************************************** */
static void charge_send_uevent(struct charge_device_info *di)
{
	/* send events */
	enum charge_status_event events;
	if (di == NULL) {
		hwlog_err("[%s]di is NULL!\n", __func__);
		return;
	}
	if (di->charger_source == POWER_SUPPLY_TYPE_MAINS) {
		events = VCHRG_START_AC_CHARGING_EVENT;
		coul_drv_charger_event_rcv(events);
	} else if (di->charger_source == POWER_SUPPLY_TYPE_USB) {
		events = VCHRG_START_USB_CHARGING_EVENT;
		coul_drv_charger_event_rcv(events);
	} else if (di->charger_source == POWER_SUPPLY_TYPE_BATTERY) {
		events = VCHRG_STOP_CHARGING_EVENT;
		coul_drv_charger_event_rcv(events);
	} else {
		hwlog_err("[%s]error charger source!\n", __func__);
		// do nothing
	}
}
EXPORT_SYMBOL(fcp_get_stage_status);

/* *********************************************************
 * Function:       fcp_get_stage_status
 * Description:    get the stage of fcp charge
 * Parameters:     di:charge_device_info
 * return value:   NULL
 * ******************************************************** */
enum fcp_check_stage_type fcp_get_stage_status(void)
{
	return fcp_stage;
}
/* *********************************************************
 * Function:       fcp_set_stage_status
 * Description:    set the stage of fcp charge
 * Parameters:     di:charge_device_info
 * return value:   NULL
 * ******************************************************** */
void fcp_set_stage_status(enum fcp_check_stage_type stage_type)
{
	fcp_stage = stage_type;
}
/* *********************************************************
*  Function:       fcp_retry_pre_operate
*  Discription:    pre operate before retry fcp enable
*  Parameters:   di:charge_device_info,type : enum fcp_retry_operate_type
*  return value:  0: fcp pre operate success
                  -1:fcp pre operate fail
********************************************************* */
static int fcp_retry_pre_operate(enum fcp_retry_operate_type type,
				 struct charge_device_info *di)
{
	int pg_state = 0, ret = -1;
	ret = di->ops->get_charge_state(&pg_state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail!!ret = 0x%x\n", ret);
		return ret;
	}
	/* check status charger not power good state */
	if (pg_state & CHAGRE_STATE_NOT_PG) {
		hwlog_err("state is not power good \n");
		return -1;
	}
	switch (type) {
	case FCP_RETRY_OPERATE_RESET_ADAPTER:
		if (NULL != di->fcp_ops->fcp_adapter_reset) {
			hwlog_info("send fcp adapter reset cmd \n");
			ret = di->fcp_ops->fcp_adapter_reset();
		} else {
			ret = -1;
		}
		break;
	case FCP_RETRY_OPERATE_RESET_FSA9688:
		if (NULL != di->fcp_ops->switch_chip_reset) {
			hwlog_info(" switch_chip_reset \n");
			ret = di->fcp_ops->switch_chip_reset();
			msleep(2000);
		} else {
			ret = -1;
		}
		break;
	default:
		break;
	}
	return ret;
}
/* *********************************************************
*  Function:       fcp_start_charging
*  Discription:    enter fcp charging mode
*  Parameters:   di:charge_device_info
*  return value:  0: fcp start success
                  -1:fcp start fail
                  1:fcp start failed need to retry
********************************************************* */
static int fcp_start_charging(struct charge_device_info *di)
{
	int ret = -1;
	int output_vol = 0;
	struct charge_init_data init_crit;

	init_crit.vbus = ADAPTER_9V;
	init_crit.charger_type = di->charger_type;

	fcp_set_stage_status(FCP_STAGE_SUPPORT_DETECT);
	if ((NULL == di->fcp_ops) || (NULL == di->fcp_ops->is_support_fcp) ||
	    (NULL == di->fcp_ops->detect_adapter) ||
	    (NULL == di->fcp_ops->set_adapter_output_vol) ||
	    (NULL == di->fcp_ops->get_adapter_output_current)) {
		hwlog_err("fcp ops is NULL!\n");
		return -1;
	}
	/* check whether support fcp */
	if (di->fcp_ops->is_support_fcp()) {
		hwlog_err("not support fcp!\n");
		return -1;
	}
	/* To avoid to effect accp detect , input current need to be lower than 1A,we
   * set 0.5A */
	di->input_current = CHARGE_CURRENT_0500_MA;
	di->ops->set_input_current(di->input_current);

	/* detect fcp adapter */
	fcp_set_stage_status(FCP_STAGE_ADAPTER_DETECT);
	ret = di->fcp_ops->detect_adapter();
	if (FCP_ADAPTER_DETECT_SUCC != ret) {
		hwlog_err("fcp detect fail!\n");
		if (FCP_ADAPTER_DETECT_FAIL == ret) {
			return 1;
		}
		return -1;
	}

	fcp_set_stage_status(FCP_STAGE_ADAPTER_ENABLE);

	di->ops->fcp_chip_init();
	/* disable charger when vol is changed */
	di->ops->set_charge_enable(FALSE);

	/* set fcp adapter output vol */
	if (di->fcp_ops->set_adapter_output_vol(&output_vol)) {
		di->ops->chip_init(&init_crit);
		hwlog_err("fcp set vol fail!\n");
		return 1;
	}
	/* enable charger when vol is changed */
	di->ops->set_charge_enable(TRUE);
	di->charger_type = CHARGER_TYPE_FCP;
	fcp_set_stage_status(FCP_STAGE_SUCESS);
	hwlog_info("fcp charging start success!\n");
	return 0;
}
/* *********************************************************
 * Function:        charge_vbus_voltage_check
 * Discription:     check whether the voltage of vbus is normal
 * Parameters:   di:charge_device_info
 * return value:   NULL
 * ******************************************************** */
static void charge_vbus_voltage_check(struct charge_device_info *di)
{
	int ret = 0;
	unsigned int vbus_vol = 0;
	static int fcp_vbus_lower_count = 0;
	static int nonfcp_vbus_higher_count = 0;

	if (NULL == di->ops->get_vbus) {
		return;
	}

	/* enable the covn ,after 1s the it will be set 0 auto */
	if (NULL != di->ops->set_covn_start)
		ret = di->ops->set_covn_start(true);

	if (ret) {
		hwlog_err("[%s]set covn start fail.\n", __func__);
		return;
	}
	ret = di->ops->get_vbus(&vbus_vol);
	hwlog_info("vbus vbus_vol:%d.\n", vbus_vol);

	if (ret)
		hwlog_err("[%s]vbus read failed \n", __func__);

	if (FCP_STAGE_SUCESS == fcp_get_stage_status()) {
		/* fcp stage : vbus must be higher than 7000 mV */
		if (vbus_vol < VBUS_VOLTAGE_FCP_MIN_MV) {
			fcp_vbus_lower_count += 1;
			hwlog_err("[%s]fcp output vol =%d mV, lower 7000 mV , "
				  "fcp_vbus_lower_count =%d!!\n",
				  __func__, vbus_vol, fcp_vbus_lower_count);
		} else {
			fcp_vbus_lower_count = 0;
		}
		/* check continuous abnormal vbus cout  */
		if (fcp_vbus_lower_count >= VBUS_VOLTAGE_ABNORMAL_MAX_COUNT) {
			fcp_check_adapter_status(di);
			fcp_set_stage_status(FCP_STAGE_DEFAUTL);
			di->charger_type = CHARGER_TYPE_STANDARD;
			fcp_vbus_lower_count = VBUS_VOLTAGE_ABNORMAL_MAX_COUNT;
		}
		nonfcp_vbus_higher_count = 0;
	} else {
		/* non fcp stage : vbus must be lower than 6500 mV */
		if (vbus_vol > VBUS_VOLTAGE_NON_FCP_MAX_MV) {
			nonfcp_vbus_higher_count += 1;
			hwlog_info(
				"[%s]non standard fcp and vbus voltage is %d mv,over 6500mv "
				",nonfcp_vbus_higher_count =%d!!\n",
				__func__, vbus_vol, nonfcp_vbus_higher_count);
		} else {
			nonfcp_vbus_higher_count = 0;
		}
		/* check continuous abnormal vbus cout  */
		if (nonfcp_vbus_higher_count >=
		    VBUS_VOLTAGE_ABNORMAL_MAX_COUNT) {
			di->charge_enable = FALSE;
			nonfcp_vbus_higher_count =
				VBUS_VOLTAGE_ABNORMAL_MAX_COUNT;
		}
		fcp_vbus_lower_count = 0;
	}
}
/* *********************************************************
 * Function:       fcp_charge_check
 * Description:    check whether start fcp charging,if support try to start fcp
 * charging Parameters:     di:charge_device_info return value:   NULL
 * ******************************************************** */
static void fcp_charge_check(struct charge_device_info *di)
{
	int ret = 0, i = 0;
	/* if chip not support fcp, return */
	if (!di->ops->fcp_chip_init) {
		return;
	}

	if (FCP_STAGE_SUCESS == fcp_get_stage_status())
		fcp_check_switch_status(di);

	if (di->charger_type != CHARGER_TYPE_STANDARD ||
	    !(coul_drv_is_battery_exist())) {
		return;
	}

	if (FCP_STAGE_DEFAUTL == fcp_get_stage_status()) {
		ret = fcp_start_charging(di);
		for (i = 0; i < 3 && ret == 1; i++) {
			/* reset adapter and try again */
			if ((fcp_retry_pre_operate(
				    FCP_RETRY_OPERATE_RESET_ADAPTER, di)) < 0) {
				hwlog_err("reset adapter failed \n");
				break;
			}
			ret = fcp_start_charging(di);
		}
		if (ret == 1) {
			/* reset fsa9688 chip and try again */
			if ((fcp_retry_pre_operate(
				    FCP_RETRY_OPERATE_RESET_FSA9688, di)) ==
			    0) {
				ret = fcp_start_charging(di);
			} else {
				hwlog_err(
					"%s : fcp_retry_pre_operate failed \n",
					__func__);
			}
		}

		hwlog_info("[%s]fcp stage  %s !!! \n", __func__,
			   fcp_check_stage[fcp_get_stage_status()]);
	}
}
/* *********************************************************
 * Function:       charge_select_charging_current
 * Description:    get the input current and charge current from different
 * charger_type and charging limited value Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_select_charging_current(struct charge_device_info *di)
{
	static unsigned int first_in = 1;

	switch (di->charger_type) {
	case CHARGER_TYPE_USB:
		di->input_current = di->core_data->iin_usb;
		di->charge_current = di->core_data->ichg_usb;
		break;
	case CHARGER_TYPE_NON_STANDARD:
		di->input_current = di->core_data->iin_nonstd;
		di->charge_current = di->core_data->ichg_nonstd;
		break;
	case CHARGER_TYPE_BC_USB:
		di->input_current = di->core_data->iin_bc_usb;
		di->charge_current = di->core_data->ichg_bc_usb;
		break;
	case CHARGER_TYPE_STANDARD:
		di->input_current = di->core_data->iin_ac;
		di->charge_current = di->core_data->ichg_ac;
		break;
	case CHARGER_TYPE_FCP:
		di->input_current = di->core_data->iin_fcp;
		di->charge_current = di->core_data->ichg_fcp;
		break;
	default:
		di->input_current = CHARGE_CURRENT_0500_MA;
		di->charge_current = CHARGE_CURRENT_0500_MA;
		break;
	}

	if (strstr(saved_command_line, "androidboot.swtype=factory") &&
	    (!coul_drv_is_battery_exist())) {
		if (first_in) {
			hwlog_info(
				"facory_version and battery not exist, enable charge\n");
			first_in = 0;
		}
	} else {
		if (di->sysfs_data.charge_limit == TRUE) {
			di->input_current =
				((di->input_current < di->core_data->iin) ?
					 di->input_current :
					 di->core_data->iin);
			di->input_current =
				((di->input_current < di->sysfs_data.iin_thl) ?
					 di->input_current :
					 di->sysfs_data.iin_thl);
			di->input_current =
				((di->input_current < di->sysfs_data.iin_rt) ?
					 di->input_current :
					 di->sysfs_data.iin_rt);

			di->charge_current =
				((di->charge_current < di->core_data->ichg) ?
					 di->charge_current :
					 di->core_data->ichg);
			di->charge_current = ((di->charge_current <
					       di->sysfs_data.ichg_thl) ?
						      di->charge_current :
						      di->sysfs_data.ichg_thl);
			di->charge_current =
				((di->charge_current < di->sysfs_data.ichg_rt) ?
					 di->charge_current :
					 di->sysfs_data.ichg_rt);
		}
	}

	if (0 != di->sysfs_data.inputcurrent)
		di->input_current = di->sysfs_data.inputcurrent;

	if (1 == di->sysfs_data.batfet_disable)
		di->input_current = CHARGE_CURRENT_2000_MA;
	hwlog_info("[%s] charger_type %d\n", __func__, di->charger_type);
	hwlog_info(
		"[%s] input_current %d, iin = %d, iin_thl = %d, iin_rt = %d\n",
		__func__, di->input_current, di->core_data->iin,
		di->sysfs_data.iin_thl, di->sysfs_data.iin_rt);
	hwlog_info(
		"[%s] charge_current %d, ichg = %d, ichg_thl = %d, ichg_rt = %d\n",
		__func__, di->charge_current, di->core_data->ichg,
		di->sysfs_data.ichg_thl, di->sysfs_data.ichg_rt);
}
/* *********************************************************
 * Function:       charge_update_vindpm
 * Discription:    update the input dpm voltage setting by battery capacity
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_update_vindpm(struct charge_device_info *di)
{
	int ret = 0;
	int vindpm = CHARGE_VOLTAGE_4520_MV;

	if (FCP_STAGE_SUCESS == fcp_get_stage_status()) {
		return;
	}

	if (POWER_SUPPLY_TYPE_MAINS == di->charger_source)
		vindpm = di->core_data->vdpm;

	if (di->ops->set_dpm_voltage) {
		ret = di->ops->set_dpm_voltage(vindpm);
		if (ret > 0) {
			hwlog_info("dpm voltage is out of range:%dmV!!\n", ret);
			ret = di->ops->set_dpm_voltage(ret);
			if (ret < 0)
				hwlog_err("set dpm voltage fail!!\n");
		} else if (ret < 0) {
			hwlog_err("set dpm voltage fail!!\n");
		}
	}
}
/* *********************************************************
 * Function:       charge_update_external_setting
 * Discription:    update the others chargerIC setting
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_update_external_setting(struct charge_device_info *di)
{
	int ret = 0;
	unsigned int batfet_disable = FALSE;
	unsigned int watchdog_timer = WATCHDOG_TIMER_80_S;

	/* update batfet setting */
	if (di->sysfs_data.batfet_disable == TRUE)
		batfet_disable = TRUE;

	if (di->ops->set_batfet_disable) {
		ret = di->ops->set_batfet_disable(batfet_disable);
		if (ret)
			hwlog_err("set batfet disable fail!!\n");
	}
	/* update watch dog timer setting */
	if (di->sysfs_data.wdt_disable == TRUE)
		watchdog_timer = WATCHDOG_TIMER_DISABLE;

	if (di->ops->set_watchdog_timer) {
		ret = di->ops->set_watchdog_timer(watchdog_timer);
		if (ret)
			hwlog_err("set watchdog timer fail!!\n");
	}
}
/* *********************************************************
 * Function:       charge_is_charging_full
 * Discription:    check the battery is charging full  or not
 * Parameters:   di:charge_device_info
 * return value:  TURE-is full or FALSE-no full
 * ******************************************************** */
static int charge_is_charging_full(struct charge_device_info *di)
{
	int ichg = -coul_drv_battery_current();
	int ichg_avg = coul_drv_battery_current_avg();
	int val = FALSE;

	if (!(di->charge_enable) || !(coul_drv_is_battery_exist()))
		return val;

	if ((ichg > MIN_CHARGING_CURRENT_OFFSET) &&
	    (ichg < di->core_data->iterm) &&
	    (ichg_avg > MIN_CHARGING_CURRENT_OFFSET) &&
	    (ichg_avg < di->core_data->iterm)) {
		di->check_full_count++;
		if (di->check_full_count >= BATTERY_FULL_CHECK_TIMIES) {
			di->check_full_count = BATTERY_FULL_CHECK_TIMIES;
			val = TRUE;
			hwlog_info(
				"battery is full!capacity = %d,ichg = %d,ichg_avg = %d\n",
				coul_drv_battery_capacity(), ichg, ichg_avg);
		}
	} else {
		di->check_full_count = 0;
	}

	return val;
}
/* *********************************************************
 * Function:       charge_full_handle
 * Discription:    set term enable flag by charge current is lower than iterm
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_full_handle(struct charge_device_info *di)
{
	int ret = 0;
	int is_battery_full = charge_is_charging_full(di);

	if (di->ops->set_term_enable) {
		ret = di->ops->set_term_enable(is_battery_full);
		if (ret)
			hwlog_err("set term enable fail!!\n");
	}
	/* set terminal current */
	ret = di->ops->set_terminal_current(di->core_data->iterm);
	if (ret > 0) {
		di->ops->set_terminal_current(ret);
	} else if (ret < 0) {
		hwlog_err("set terminal current fail!\n");
	}
	/* reset adapter to 5v after fcp charge done,avoid long-term at high voltage
   */
	if (is_battery_full && FCP_STAGE_SUCESS == fcp_get_stage_status() &&
	    coul_drv_battery_capacity() == 100 &&
	    NULL != di->fcp_ops->fcp_adapter_reset) {
		if (di->fcp_ops->fcp_adapter_reset())
			hwlog_err("adapter reset failed \n");

		fcp_set_stage_status(FCP_STAGE_CHARGE_DONE);
		hwlog_info("fcp charge done, reset adapter to 5v !\n");
	}
}
/* *********************************************************
 * Function:       charge_set_charge_state
 * Discription:    set charge stop or enable
 * Parameters:     state:0 stop 1 enable
 * return value:   old state
 * ******************************************************** */
int charge_set_charge_state(int state)
{
	int old_state;
	int chg_en = 0;
	struct charge_device_info *di = g_charge_info;

	if ((di == NULL) || ((state != 0) && (state != 1))) {
		return -1;
	}

	old_state = di->ops->get_charge_enable_status();
	chg_en = state;
	di->ops->set_charge_enable(chg_en);

	return old_state;
}
/* *********************************************************
 * Function:       charge_kick_watchdog
 * Discription:    kick watchdog timer
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_kick_watchdog(struct charge_device_info *di)
{
	int ret = 0;
	ret = di->ops->reset_watchdog_timer();
	if (ret)
		hwlog_err("charge kick watchdog timer fail!!\n");
}

/* *********************************************************
 * Function:       charge_disable_watchdog
 * Description:   disable watchdog timer
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_disable_watchdog(struct charge_device_info *di)
{
	int ret = 0;
	ret = di->ops->set_watchdog_timer(WDT_STOP);
	if (ret)
		hwlog_err("charge disable watchdog timer fail!!\n");
	else
		hwlog_info("charge disable watchdog timer");
}

/* *********************************************************
 * Function:       charge_set_watchdog
 * Description:   set watchdog timer
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_set_watchdog(struct charge_device_info *di, int value)
{
	int ret = 0;
	ret = di->ops->set_watchdog_timer(value);
	if (ret)
		hwlog_err("charge set watchdog timer fail!!\n");
	else
		hwlog_info("charge set watchdog timer: %ds\n", value);
}

/* *********************************************************
 * Function:       charge_start_charging
 * Discription:    enter into charging mode
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_start_charging(struct charge_device_info *di)
{
	int ret = 0;
	struct charge_init_data init_crit;

	init_crit.vbus = ADAPTER_5V;
	init_crit.charger_type = di->charger_type;

	hwlog_info("---->START CHARGING\n");
	charge_wake_lock();
	di->sysfs_data.charge_enable = TRUE;
	di->check_full_count = 0;
	/* chip init */
	ret = di->ops->chip_init(&init_crit);
	if (ret)
		hwlog_err("chip init fail!!\n");
	queue_delayed_work(system_power_efficient_wq, &di->charge_work,
			   msecs_to_jiffies(0));
}
/* *********************************************************
 * Function:       charge_stop_charging
 * Discription:    exit charging mode
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_stop_charging(struct charge_device_info *di)
{
	int ret = 0;

	hwlog_info("---->STOP CHARGING\n");
	charge_wake_lock();
	di->sysfs_data.charge_enable = FALSE;
	di->check_full_count = 0;
	ret = di->ops->set_charge_enable(FALSE);
	if (ret)
		hwlog_err("[%s]set charge enable fail!\n", __func__);
	ret = di->ops->set_otg_enable(FALSE);
	if (ret)
		hwlog_err("[%s]set otg enable fail!\n", __func__);
	otg_flag = FALSE;
	cancel_delayed_work_sync(&di->charge_work);
	cancel_delayed_work_sync(&di->otg_work);
	if ((di->sysfs_data.wdt_disable == TRUE) &&
	    (di->ops->set_watchdog_timer)) { /* when charger stop ,disable watch dog
                                          ,only for hiz */
		if (di->ops->set_watchdog_timer(WATCHDOG_TIMER_DISABLE))
			hwlog_err("set watchdog timer fail for hiz!!\n");
	}
	if (di->ops->stop_charge_config) {
		if (di->ops->stop_charge_config())
			hwlog_err(" stop charge config failed \n");
	}
	/* flag must be clear after charge_work has been canceled */
	fcp_set_stage_status(FCP_STAGE_DEFAUTL);
	msleep(1000);

	if ((strstr(saved_command_line, "androidboot.swtype=factory")) &&
	    (di->sysfs_data.batfet_disable == TRUE)) {
		if (di->ops->set_batfet_disable) {
			hwlog_info(
				"sysfs_data.batfet_disable == TRUE, disable batfet!\n");
			di->ops->set_batfet_disable(TRUE);
		}
	}
	__pm_wakeup_event(stop_charge_lock, jiffies_to_msecs(HZ));
	mutex_lock(&charge_wakelock_flag_lock);
	charge_lock_flag = CHARGE_NO_NEED_WAKELOCK;
	charge_wake_unlock();
	mutex_unlock(&charge_wakelock_flag_lock);
}
/* *********************************************************
 * Function:       charge_type_dcp_detected_notify
 * Description:    check high voltage charge once dcp detected
 * Parameters:     NULL
 * return value:   NULL
 * ******************************************************** */
void charge_type_dcp_detected_notify(void)
{
	if (g_charge_info && (CHARGER_TYPE_NON_STANDARD == g_charge_info->charger_type))
		mod_delayed_work(system_wq, &g_charge_info->charge_work, 0);
}
/* *********************************************************
 * Function:       charge_start_usb_otg
 * Discription:    enter into otg mode
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_start_usb_otg(struct charge_device_info *di)
{
	int ret = 0;

	hwlog_info("---->START OTG MODE\n");
	charge_wake_lock();
	ret = di->ops->set_charge_enable(FALSE);
	if (ret)
		hwlog_err("[%s]set charge enable fail!\n", __func__);
	ret = di->ops->set_otg_enable(TRUE);
	if (ret)
		hwlog_err("[%s]set otg enable fail!\n", __func__);
	otg_flag = TRUE;
	if (di->ops->set_otg_current) {
		ret = di->ops->set_otg_current(
			di->core_data
				->otg_curr); /* otg current set 1A form dtsi */
		if (ret)
			hwlog_err("[%s]set otg current fail!\n", __func__);
	}
	queue_delayed_work(system_power_efficient_wq, &di->otg_work,
			   msecs_to_jiffies(0));
}
/* *********************************************************
 * Function:       charge_otg_work
 * Discription:    monitor the otg mode status
 * Parameters:   work:otg workqueue
 * return value:  NULL
 * ******************************************************** */
static void charge_otg_work(struct work_struct *work)
{
	struct charge_device_info *di =
		container_of(work, struct charge_device_info, otg_work.work);

	charge_kick_watchdog(di);
	queue_delayed_work(system_power_efficient_wq, &di->otg_work,
			   msecs_to_jiffies(CHARGING_WORK_TIMEOUT));
}
/* *********************************************************
 * Function:       charge_fault_work
 * Discription:    handler the fault events from chargerIC
 * Parameters:   work:fault workqueue
 * return value:  NULL
 * ******************************************************** */
static void charge_fault_work(struct work_struct *work)
{
	struct charge_device_info *di =
		container_of(work, struct charge_device_info, fault_work);

	switch (di->charge_fault) {
	case CHARGE_FAULT_BOOST_OCP:
		hwlog_err("vbus overloaded in boost mode,close otg mode!!\n");
		di->ops->set_otg_enable(FALSE);
		di->charge_fault = CHARGE_FAULT_NON;
		break;
	default:
		break;
	}
}

/* *********************************************************
 * Function:       charge_update_status
 * Discription:    update the states of charging
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_update_status(struct charge_device_info *di)
{
	enum charge_status_event events = VCHRG_POWER_NONE_EVENT;
	unsigned int state = CHAGRE_STATE_NORMAL;
	int ret = 0;
	struct charge_init_data init_crit;

	init_crit.vbus = ADAPTER_5V;
	init_crit.charger_type = di->charger_type;

	ret = di->ops->get_charge_state(&state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail!!ret = 0x%x\n", ret);
		return;
	}
	/* check status charger not power good state */
	if (state & CHAGRE_STATE_NOT_PG) {
		hwlog_err("VCHRG_POWER_SUPPLY_WEAKSOURCE\n");
		events = VCHRG_POWER_SUPPLY_WEAKSOURCE;
		coul_drv_charger_event_rcv(events);
	}
	/* check status charger ovp err */
	if (state & CHAGRE_STATE_VBUS_OVP) {
		hwlog_err("VCHRG_POWER_SUPPLY_OVERVOLTAGE\n");
		events = VCHRG_POWER_SUPPLY_OVERVOLTAGE;
		coul_drv_charger_event_rcv(events);
	}
	/* check status watchdog timer expiration */
	if (state & CHAGRE_STATE_WDT_FAULT) {
		hwlog_err("CHAGRE_STATE_WDT_TIMEOUT\n");
		/* init chip register when watchdog timeout */
		di->ops->chip_init(&init_crit);
		events = VCHRG_STATE_WDT_TIMEOUT;
		coul_drv_charger_event_rcv(events);
	}
	/* check status battery ovp */
	if (state & CHAGRE_STATE_BATT_OVP)
		hwlog_err("CHAGRE_STATE_BATT_OVP\n");

	/* check charger TS state */
	if (state & CHAGRE_STATE_NTC_FAULT) {
		ts_flag = TRUE;
	} else {
		ts_flag = FALSE;
	}
	/* check status charge done, ac charge and usb charge */
	if ((di->charge_enable) && (coul_drv_is_battery_exist())) {
		di->sysfs_data.charge_done_status = CHARGE_DONE_NON;
		if (state & CHAGRE_STATE_CHRG_DONE) {
			events = VCHRG_CHARGE_DONE_EVENT;
			hwlog_info("VCHRG_CHARGE_DONE_EVENT\n");
			di->sysfs_data.charge_done_status = CHARGE_DONE;
			if (CHARGE_DONE_SLEEP_ENABLED ==
			    di->sysfs_data.charge_done_sleep_status) {
				if ((CHARGER_TYPE_STANDARD ==
				     di->charger_type) ||
				    (CHARGER_TYPE_NON_STANDARD ==
				     di->charger_type) ||
				    (CHARGER_TYPE_FCP == di->charger_type)) {
					mutex_lock(&charge_wakelock_flag_lock);
					charge_lock_flag = CHARGE_NEED_WAKELOCK;
					charge_wake_unlock();
					mutex_unlock(
						&charge_wakelock_flag_lock);
					hwlog_info(
						"charge wake unlock while charging done\n");
				}
			}
		} else if (di->charger_source == POWER_SUPPLY_TYPE_MAINS) {
			events = VCHRG_START_AC_CHARGING_EVENT;
		} else {
			events = VCHRG_START_USB_CHARGING_EVENT;
		}
	} else {
		events = VCHRG_NOT_CHARGING_EVENT;
		hwlog_info("VCHRG_NOT_CHARGING_EVENT\n");
	}
	coul_drv_charger_event_rcv(events);
}
/* *********************************************************
 * Function:       charge_turn_on_charging
 * Discription:    turn on charging, set input and charge currrent /CV termminal
 * voltage / charge enable Parameters:   di:charge_device_info return value:
 * NULL
 * ******************************************************** */
static void charge_turn_on_charging(struct charge_device_info *di)
{
	int ret = 0;
	unsigned int vterm = 0;

	di->charge_enable = TRUE;
	/* check vbus voltage ,if vbus is abnormal disable charge or abort from fcp */
	charge_vbus_voltage_check(di);
	/* set input current */
	ret = di->ops->set_input_current(di->input_current);
	if (ret > 0) {
		hwlog_info("input current is out of range:%dmA!!\n", ret);
		di->ops->set_input_current(ret);
	} else if (ret < 0) {
		hwlog_err("set input current fail!\n");
	}
	/* check if allow charging or not */
	if (di->charge_current == CHARGE_CURRENT_0000_MA) {
		di->charge_enable = FALSE;
		hwlog_info("charge current is set 0mA, turn off charging!\n");
	} else {
		/* set CC charge current */
		ret = di->ops->set_charge_current(di->charge_current);
		if (ret > 0) {
			hwlog_info("charge current is out of range:%dmA!!\n",
				   ret);
			di->ops->set_charge_current(ret);
		} else if (ret < 0) {
			hwlog_err("set charge current fail!\n");
		}
		/* set CV terminal voltage */
		vterm = ((di->core_data->vterm < di->sysfs_data.vterm_rt) ?
				 di->core_data->vterm :
				 di->sysfs_data.vterm_rt);
		ret = di->ops->set_terminal_voltage(vterm);
		if (ret > 0) {
			hwlog_info("terminal voltage is out of range:%dmV!!\n",
				   ret);
			di->ops->set_terminal_voltage(ret);
		} else if (ret < 0) {
			hwlog_err("set terminal voltage fail!\n");
		}
	}
	/* enable/disable charge */
	di->charge_enable &= di->sysfs_data.charge_enable;
	ret = di->ops->set_charge_enable(di->charge_enable);
	if (ret)
		hwlog_err("set charge enable fail!!\n");

	hwlog_debug(
		"input_current is [%d],charge_current is [%d],terminal_voltage "
		"is [%d],charge_enable is [%d]\n",
		di->input_current, di->charge_current, vterm,
		di->charge_enable);
}

/* *********************************************************
 * Function:       charge_safe_protect
 * Discription:    do safe protect ops
 * Parameters:   di:charge_device_info
 * return value:  NULL
 * ******************************************************** */
static void charge_safe_protect(struct charge_device_info *di)
{
	if (NULL == di) {
		hwlog_err("%s:NULL pointer!!\n", __func__);
		return;
	}
	/* do soft ovp protect for 5V/9V charge */
	if (di->ops && di->ops->soft_vbatt_ovp_protect)
		di->ops->soft_vbatt_ovp_protect();
}

/* *********************************************************
 * Function:       charge_monitor_work
 * Discription:    monitor the charging process
 * Parameters:   work:charge workqueue
 * return value:  NULL
 * ******************************************************** */
static void charge_monitor_work(struct work_struct *work)
{
	struct charge_device_info *di =
		container_of(work, struct charge_device_info, charge_work.work);

	hwlog_err("%s:+\n", __func__);
	/* update type before get params */
	charge_update_charger_type(di);
	fcp_charge_check(di);

	di->core_data = charge_core_get_params();
	charge_select_charging_current(di);

	charge_turn_on_charging(di);
	charge_safe_protect(di);

	charge_full_handle(di);
	charge_update_vindpm(di);
	charge_update_external_setting(di);

	charge_update_status(di);
	charge_kick_watchdog(di);
	queue_delayed_work(system_power_efficient_wq, &di->charge_work,
			   msecs_to_jiffies(CHARGING_WORK_TIMEOUT));

	hwlog_err("%s:-\n", __func__);
}
/* *********************************************************
 * Function:       charge_usb_work
 * Discription:    handler interface by different charger_type
 * Parameters:   work:usb workqueue
 * return value:  NULL
 * ******************************************************** */
static void charge_usb_work(struct work_struct *work)
{
	struct charge_device_info *di =
		container_of(work, struct charge_device_info, usb_work);

	switch (di->charger_type) {
	case CHARGER_TYPE_USB:
		hwlog_info("case = CHARGER_TYPE_USB-> \n");
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_NON_STANDARD:
		hwlog_info("case = CHARGER_TYPE_NON_STANDARD -> \n");
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_BC_USB:
		hwlog_info("case = CHARGER_TYPE_BC_USB -> \n");
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_STANDARD:
		hwlog_info("case = CHARGER_TYPE_STANDARD-> \n");
		charge_start_charging(di);
		break;
	case CHARGER_REMOVED:
		hwlog_info("case = USB_EVENT_NONE-> \n");
		charge_stop_charging(di);
		break;
	case USB_EVENT_OTG_ID:
		hwlog_info("case = USB_EVENT_OTG_ID-> \n");
		charge_start_usb_otg(di);
		break;
	default:
		break;
	}
}
/* *********************************************************
 * Function:       charge_resume_wakelock_work
 * Description:    apply wake_lock when resume
 * Parameters:   NULL
 * return value:  NULL
 * ******************************************************** */
static void charge_resume_wakelock_work(struct work_struct *work)
{
	mutex_lock(&charge_wakelock_flag_lock);
	if (CHARGE_NEED_WAKELOCK == charge_lock_flag) {
		charge_wake_lock();
		hwlog_info("charge wake lock when resume during charging\n");
	}
	mutex_unlock(&charge_wakelock_flag_lock);
}
/* *********************************************************
 * Function:       charge_usb_notifier_call
 * Discription:    respond the charger_type events from USB PHY
 * Parameters:   usb_nb:usb notifier_block
 * event:charger type event name
 * data:unused
 * return value:  NOTIFY_OK-success or others
 * ******************************************************** */
static int charge_usb_notifier_call(struct notifier_block *usb_nb,
				    unsigned long event, void *data)
{
	struct charge_device_info *di =
		container_of(usb_nb, struct charge_device_info, usb_nb);

	hwlog_err("%s event %d\n", __func__, event);
	charge_wake_lock();
	charge_rename_charger_type((enum chip_charger_type)event, di);
	charge_send_uevent(di);
	queue_work(system_power_efficient_wq, &di->usb_work);
	return NOTIFY_OK;
}
/* *********************************************************
 * Function:       charge_fault_notifier_call
 * Discription:    respond the fault events from chargerIC
 * Parameters:   fault_nb:fault notifier_block
 * event:fault event name
 * data:unused
 * return value:  NOTIFY_OK-success or others
 * ******************************************************** */
static int charge_fault_notifier_call(struct notifier_block *fault_nb,
				      unsigned long event, void *data)
{
	struct charge_device_info *di =
		container_of(fault_nb, struct charge_device_info, fault_nb);

	di->charge_fault = (enum charge_fault_type)event;
	queue_work(system_power_efficient_wq, &di->fault_work);
	return NOTIFY_OK;
}

/* *********************************************************
 * Function:       charge_check_input_dpm_state
 * Discription:    check charger whether VINDPM or IINDPM
 * Parameters:     NULL
 * return value:   1 means charger VINDPM or IINDPM
 * 0 means charger NoT DPM
 * -1 means chargeIC not support this function
 * ******************************************************** */
int charge_check_input_dpm_state(void)
{
	if (NULL == g_ops || NULL == g_ops->check_input_dpm_state) {
		return -1;
	}
	return g_ops->check_input_dpm_state();
}

#if CONFIG_SYSFS
#define charge_sysfs_file(_name, n, m, store)                                 \
	{                                                                      \
		.attr = __ATTR(_name, m, charge_sysfs_show, store),            \
		.name = CHARGE_SYSFS_##n,                                      \
	}

#define charge_sysfs_file_rw(_name, n)                                        \
	charge_sysfs_file(_name, n, S_IWUSR | S_IRUGO, charge_sysfs_store)

#define charge_sysfs_file_ro(_name, n)                                        \
	charge_sysfs_file(_name, n, S_IRUGO, NULL)

static ssize_t charge_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf);
static ssize_t charge_sysfs_store(struct device *dev,
				  struct device_attribute *attr, const char *buf, size_t count);

struct charge_sysfs_field_info {
	struct device_attribute	attr;
	u8 name;
};
/*lint -e665*/
static struct charge_sysfs_field_info charge_sysfs_field_tbl[] = {
	charge_sysfs_file_rw(iin_thermal,    IIN_THERMAL),
	charge_sysfs_file_rw(ichg_thermal,    ICHG_THERMAL),
	/*iin_runningtest will be used for running test and audio test*/
	charge_sysfs_file_rw(iin_runningtest,    IIN_RUNNINGTEST),
	charge_sysfs_file_rw(ichg_runningtest,    ICHG_RUNNINGTEST),
	charge_sysfs_file_rw(enable_charger,    ENABLE_CHARGER),
	charge_sysfs_file_rw(limit_charging,    LIMIT_CHARGING),
	charge_sysfs_file_rw(regulation_voltage,    REGULATION_VOLTAGE),
	charge_sysfs_file_rw(shutdown_q4,    BATFET_DISABLE),
	charge_sysfs_file_rw(shutdown_watchdog,    WATCHDOG_DISABLE),
	charge_sysfs_file_ro(chargelog,    CHARGELOG),
	charge_sysfs_file_ro(chargelog_head,    CHARGELOG_HEAD),
	charge_sysfs_file_ro(Ibus,    IBUS),
	charge_sysfs_file_rw(enable_hiz,    HIZ),
	charge_sysfs_file_ro(chargerType,    CHARGE_TYPE),
	charge_sysfs_file_ro(charge_done_status, CHARGE_DONE_STATUS),
	charge_sysfs_file_rw(charge_done_sleep_status, CHARGE_DONE_SLEEP_STATUS),
	charge_sysfs_file_rw(inputcurrent,    INPUTCURRENT),
};
/*lint +e665*/
static struct attribute *charge_sysfs_attrs[ARRAY_SIZE(charge_sysfs_field_tbl) + 1];

static const struct attribute_group charge_sysfs_attr_group = {
	.attrs = charge_sysfs_attrs,
};
/**********************************************************
*  Function:       charge_sysfs_init_attrs
*  Discription:    initialize charge_sysfs_attrs[] for charge attribute
*  Parameters:   NULL
*  return value:  NULL
**********************************************************/
static void charge_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		charge_sysfs_attrs[i] = &charge_sysfs_field_tbl[i].attr.attr;

	charge_sysfs_attrs[limit] = NULL; /* Has additional entry for this */
}
/**********************************************************
*  Function:       charge_sysfs_field_lookup
*  Discription:    get the current device_attribute from charge_sysfs_field_tbl by attr's name
*  Parameters:   name:device attribute name
*  return value:  charge_sysfs_field_tbl[]
**********************************************************/
static struct charge_sysfs_field_info *charge_sysfs_field_lookup(const char *name)
{
	int i, limit = ARRAY_SIZE(charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
	{
		if (!strncmp(name, charge_sysfs_field_tbl[i].attr.attr.name,strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;

	return &charge_sysfs_field_tbl[i];
}

static ssize_t charge_sysfs_data_handle(struct charge_device_info *di,
	struct charge_sysfs_field_info *info, char *buf)
{
	switch(info->name) {
	case CHARGE_SYSFS_IIN_THERMAL:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.iin_thl);
	case CHARGE_SYSFS_ICHG_THERMAL:
		return snprintf(buf,PAGE_SIZE,  "%u\n", di->sysfs_data.ichg_thl);
	case CHARGE_SYSFS_IIN_RUNNINGTEST:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.iin_rt);
	case CHARGE_SYSFS_ICHG_RUNNINGTEST:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.ichg_rt);
	case CHARGE_SYSFS_ENABLE_CHARGER:
		return snprintf(buf,PAGE_SIZE,  "%u\n", di->sysfs_data.charge_enable);
	case CHARGE_SYSFS_LIMIT_CHARGING:
		return snprintf(buf,PAGE_SIZE,  "%u\n", di->sysfs_data.charge_limit);
	case CHARGE_SYSFS_REGULATION_VOLTAGE:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.vterm_rt);
	case CHARGE_SYSFS_BATFET_DISABLE:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.batfet_disable);
	case CHARGE_SYSFS_WATCHDOG_DISABLE:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.wdt_disable);
	case CHARGE_SYSFS_IBUS:
		di->sysfs_data.ibus = 0;
		//this is an optional interface for charger
		if (di->ops->get_ibus)
			di->sysfs_data.ibus = di->ops->get_ibus();
		return snprintf(buf,PAGE_SIZE, "%d\n", di->sysfs_data.ibus);
	case CHARGE_SYSFS_HIZ:
		return snprintf(buf,PAGE_SIZE, "%u\n", di->sysfs_data.hiz_enable);
	case CHARGE_SYSFS_CHARGE_TYPE:
		return snprintf(buf,PAGE_SIZE, "%d\n", di->charger_type);
	case CHARGE_SYSFS_CHARGE_DONE_STATUS:
		return snprintf(buf,PAGE_SIZE, "%d\n", di->sysfs_data.charge_done_status);
	case CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS:
		return snprintf(buf,PAGE_SIZE, "%d\n", di->sysfs_data.charge_done_sleep_status);
	case CHARGE_SYSFS_INPUTCURRENT:
		return snprintf(buf,PAGE_SIZE, "%d\n", di->sysfs_data.inputcurrent);
	default:
		hwlog_err("(%s)NODE ERR!!HAVE NO THIS NODE:%d\n",__func__,info->name);
		break;
	}
	return 0;
}

/**********************************************************
*  Function:       charge_sysfs_show
*  Discription:    show the value for all charge device's node
*  Parameters:   dev:device
*                      attr:device_attribute
*                      buf:string of node value
*  return value:  0-sucess or others-fail
**********************************************************/
static ssize_t charge_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct charge_sysfs_field_info *info = NULL;
	struct charge_device_info *di = dev_get_drvdata(dev);
	int ret;

	info = charge_sysfs_field_lookup(attr->attr.name);
	if (!info)
		return -EINVAL;

	switch(info->name) {
	case CHARGE_SYSFS_CHARGELOG:
		mutex_lock(&di->sysfs_data.dump_reg_lock);
		di->ops->dump_register(di->sysfs_data.reg_value, CHARGELOG_SIZE);
		ret = snprintf(buf,PAGE_SIZE, "%s\n", di->sysfs_data.reg_value);
		mutex_unlock(&di->sysfs_data.dump_reg_lock);
		return ret;
	case CHARGE_SYSFS_CHARGELOG_HEAD:
		mutex_lock(&di->sysfs_data.dump_reg_head_lock);
		di->ops->get_register_head(di->sysfs_data.reg_head, CHARGELOG_SIZE);
		ret = snprintf(buf,PAGE_SIZE, "%s\n", di->sysfs_data.reg_head);
		mutex_unlock(&di->sysfs_data.dump_reg_head_lock);
		return ret;
	default:
		return charge_sysfs_data_handle(di, info, buf);
	}
	return 0;
}

static int sysfs_iin_thl_set_input_current(struct charge_device_info *di, const char *buf)
{
	long val = 0;
	if((kstrtol(buf, 10, &val) < 0)||(val < 0)||(val > 3000))
		return -EINVAL;

	if((val == 0)||(val == 1))
		di->sysfs_data.iin_thl = di->core_data->iin_max;
	else if((val > 1)&&(val <= 100))
		di->sysfs_data.iin_thl = 100;
	else
		di->sysfs_data.iin_thl = val;

	if(di->input_current > di->sysfs_data.iin_thl)
		di->ops->set_input_current(di->sysfs_data.iin_thl);
	hwlog_info("THERMAL set input current = %d\n", di->sysfs_data.iin_thl);

	return 0;
}
static int sysfs_ichg_thl_set_charge_current(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if((kstrtol(buf, 10, &val) < 0)||(val < 0)||(val > 3000))
		return -EINVAL;

	if((val == 0)||(val == 1)) {
		di->sysfs_data.ichg_thl = di->core_data->ichg_max;
	} else if((val > 1)&&(val <= 500)) {
		hwlog_info("THERMAL set charge current = %ld is less than 500mA\n", val);
		di->sysfs_data.ichg_thl = 500;
	} else {
		di->sysfs_data.ichg_thl = val;
	}

	if(di->charge_current > di->sysfs_data.ichg_thl)
		di->ops->set_charge_current(di->sysfs_data.ichg_thl);
	hwlog_info("THERMAL set charge current = %d\n", di->sysfs_data.ichg_thl);

	return 0;
}

static int sysfs_iin_rt_set_input_current(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if((kstrtol(buf, 10, &val) < 0)||(val < 0)||(val > 3000))
		return -EINVAL;

	if ((val == 0)||(val == 1)) {
		di->sysfs_data.iin_rt = di->core_data->iin_max;
	} else if ((val > 1)&&(val <= 100)) {
		di->sysfs_data.iin_rt = 100;
	} else {
		di->sysfs_data.iin_rt = val;
	}

	if(di->input_current > di->sysfs_data.iin_rt)
		di->ops->set_input_current(di->sysfs_data.iin_rt);
	hwlog_info("RUNNINGTEST set input current = %d\n", di->sysfs_data.iin_rt);

	return 0;
}
static int sysfs_ichg_rt_set_charge_current(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if((kstrtol(buf, 10, &val) < 0)||(val < 0)||(val > 3000))
		return -EINVAL;

	if((val == 0)||(val == 1))
		di->sysfs_data.ichg_rt = di->core_data->ichg_max;
	else if((val > 1)&&(val <= 205))
		di->sysfs_data.ichg_rt = 205;
	else
		di->sysfs_data.ichg_rt = val;

	if(di->charge_current > di->sysfs_data.ichg_rt)
		di->ops->set_charge_current(di->sysfs_data.ichg_rt);
	hwlog_info("RUNNINGTEST set input current = %d\n", di->sysfs_data.ichg_rt);
	return 0;
}
static int sysfs_set_charge_enable(struct charge_device_info *di, const char *buf)
{
	enum charge_status_event events  = VCHRG_POWER_NONE_EVENT;

	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->sysfs_data.charge_enable = val;
	di->sysfs_data.charge_limit = TRUE;
	/*why should send events in this command?
	  because it will get the /sys/class/power_supply/Battery/status immediately
	  to check if the enable/disable command set successfully or not in some product line station
	*/
	if(di->sysfs_data.charge_enable)
	{
		events = VCHRG_START_CHARGING_EVENT;
	}
	else
	{
		events = VCHRG_NOT_CHARGING_EVENT;
	}
	di->ops->set_charge_enable(di->sysfs_data.charge_enable);
	coul_drv_charger_event_rcv(events);
	hwlog_info("RUNNINGTEST set charge enable = %d\n", di->sysfs_data.charge_enable);
	return 0;
}

static int sysfs_set_charge_limit(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->sysfs_data.charge_limit = val;
	hwlog_info("PROJECTMUNE set limit charge enable = %d\n", di->sysfs_data.charge_limit);

	return 0;
}

static int sysfs_set_hiz_enable(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->sysfs_data.hiz_enable= val;
	if (di->ops->set_charger_hiz)
		di->ops->set_charger_hiz(di->sysfs_data.hiz_enable);
	hwlog_info("RUNNINGTEST set hiz enable = %d\n", di->sysfs_data.hiz_enable);
	return 0;
}
static int sysfs_set_charge_done_status(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 10))
		return -EINVAL;

	di->sysfs_data.charge_done_status = val;
	return 0;
}
static int sysfs_set_charge_done_sleep_status(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 10))
		return -EINVAL;
	di->sysfs_data.charge_done_sleep_status = val;
	return 0;
}

static int sysfs_set_input_current(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 100) || (val > 2000))
		return -EINVAL;

	di->sysfs_data.inputcurrent = val;
	di->ops->set_input_current(di->sysfs_data.inputcurrent);
	hwlog_info("set input currrent is: %ld\n",val);
	return 0;
}


static int sysfs_set_terminal_voltage(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if((kstrtol(buf, 10, &val) < 0)||(val < 3200)||(val > 4400))
		return -EINVAL;

	di->sysfs_data.vterm_rt = val;
	di->ops->set_terminal_voltage(di->sysfs_data.vterm_rt);
	hwlog_info("RUNNINGTEST set terminal voltage = %d\n", di->sysfs_data.vterm_rt);

	return 0;
}

static int sysfs_set_batfet_disable(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->sysfs_data.batfet_disable = val;
	if (1 == val)
		di->ops->set_input_current(CHARGE_CURRENT_2000_MA);
	di->ops->set_batfet_disable(di->sysfs_data.batfet_disable);
	hwlog_info("RUNNINGTEST set batfet disable = %d\n", di->sysfs_data.batfet_disable);

	return 0;
}
static int sysfs_set_watchdog_disable(struct charge_device_info *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->sysfs_data.wdt_disable = val;
	if(val == 1)
		di->ops->set_watchdog_timer(WATCHDOG_TIMER_DISABLE);
	hwlog_info("RUNNINGTEST set wdt disable = %d\n", di->sysfs_data.wdt_disable);
	return 0;
}

static struct sysfs_store_handle sysfs_store_handle_tbl[] = {
	{CHARGE_SYSFS_IIN_THERMAL, sysfs_iin_thl_set_input_current},
	{CHARGE_SYSFS_ICHG_THERMAL, sysfs_ichg_thl_set_charge_current},
	{CHARGE_SYSFS_IIN_RUNNINGTEST, sysfs_iin_rt_set_input_current},
	{CHARGE_SYSFS_ICHG_RUNNINGTEST, sysfs_ichg_rt_set_charge_current},
	{CHARGE_SYSFS_ENABLE_CHARGER, sysfs_set_charge_enable},
	{CHARGE_SYSFS_LIMIT_CHARGING, sysfs_set_charge_limit},
	{CHARGE_SYSFS_REGULATION_VOLTAGE, sysfs_set_terminal_voltage},
	{CHARGE_SYSFS_BATFET_DISABLE, sysfs_set_batfet_disable},
	{CHARGE_SYSFS_WATCHDOG_DISABLE, sysfs_set_watchdog_disable},
	{CHARGE_SYSFS_HIZ, sysfs_set_hiz_enable},
	{CHARGE_SYSFS_CHARGE_DONE_STATUS, sysfs_set_charge_done_status},
	{CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS, sysfs_set_charge_done_sleep_status},
	{CHARGE_SYSFS_INPUTCURRENT, sysfs_set_input_current},
};

/**********************************************************
*  Function:       charge_sysfs_store
*  Discription:    set the value for charge_data's node which is can be written
*  Parameters:   dev:device
*                      attr:device_attribute
*                      buf:string of node value
*                      count:unused
*  return value:  0-sucess or others-fail
**********************************************************/
static ssize_t charge_sysfs_store(struct device *dev,
				  struct device_attribute *attr, const char *buf, size_t count)
{
	struct charge_sysfs_field_info *info = NULL;
	struct charge_device_info *di = dev_get_drvdata(dev);
	int i;

	info = charge_sysfs_field_lookup(attr->attr.name);
	if (!info)
		return -EINVAL;

	for (i = 0; i < (int)ARRAY_SIZE(sysfs_store_handle_tbl); i++) {
		if (info->name == sysfs_store_handle_tbl[i].name) {
				(void)sysfs_store_handle_tbl[i].func(di, buf);
				break;
			}
	}

	return count;
}
/**********************************************************
*  Function:       charge_check_charger_plugged
*  Discription:    detect whether USB or adaptor is plugged
*  Parameters:     NULL
*  return value:    1 means USB or adpator plugged
*                   0 means USB or adaptor not plugged
*                   -1 means chargeIC not support this function
**********************************************************/
int charge_check_charger_plugged(void)
{
	if (NULL == g_ops || NULL == g_ops->check_charger_plugged)
	{
		return -1;
	}
	return g_ops->check_charger_plugged();
}

bool charge_check_charger_ts(void)
{
	return ts_flag;
}

bool charge_check_charger_otg_state(void)
{
	return otg_flag;
}

/**********************************************************
*  Function:       charge_sysfs_create_group
*  Discription:    create the charge device sysfs group
*  Parameters:   di:charge_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int charge_sysfs_create_group(struct charge_device_info *di)
{
	charge_sysfs_init_attrs();

	return sysfs_create_group(&di->dev->kobj, &charge_sysfs_attr_group);
}
/**********************************************************
*  Function:       charge_sysfs_remove_group
*  Discription:    remove the charge device sysfs group
*  Parameters:   di:charge_device_info
*  return value:  NULL
**********************************************************/
static inline void charge_sysfs_remove_group(struct charge_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &charge_sysfs_attr_group);
}
#else
static int charge_sysfs_create_group(struct charge_device_info *di)
{
	return 0;
}
static inline void charge_sysfs_remove_group(struct charge_device_info *di)
{
}
#endif
/* *********************************************************
 * Function:       charge_ops_register
 * Discription:    register the handler ops for chargerIC
 * Parameters:   ops:operations interface of charge device
 * return value:  0-sucess or others-fail
 * ******************************************************** */
int charge_ops_register(struct charge_device_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_ops = ops;
	} else {
		hwlog_err("charge ops register fail!\n");
		ret = -EPERM;
	}
	return ret;
}
/* *********************************************************
 * Function:       fcp_adpter_ops_register
 * Discription:    register the handler ops for fcp adpter
 * Parameters:   ops:operations interface of fcp adpter
 * return value:  0-sucess or others-fail
 * ******************************************************** */
int fcp_adapter_ops_register(struct fcp_adapter_device_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_fcp_ops = ops;
	} else {
		hwlog_err("fcp ops register fail!\n");
		ret = -EPERM;
	}
	return ret;
}

unsigned int charge_get_charger_type(void)
{
	struct charge_device_info *di = g_charge_info;

	if (!di)
		return -1;

	return di->charger_type;
}

/* *********************************************************
 * Function:       charge_probe
 * Discription:    chargre module probe
 * Parameters:   pdev:platform_device
 * return value:  0-sucess or others-fail
 * ******************************************************** */
static int charge_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct charge_device_info *di = NULL;
	enum chip_charger_type type = chip_get_charger_type();
	struct charge_init_data init_crit;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		hwlog_err("alloc di failed\n");
		return -ENOMEM;
	}
	di->dev = &pdev->dev;
	di->ops = g_ops;
	di->fcp_ops = g_fcp_ops;
	di->core_data = charge_core_get_params();
	if (NULL == di->core_data) {
		hwlog_err("charge core data is NULL!\n");
		ret = -EINVAL;
		goto charge_fail_1;
	}
	if ((NULL == di->ops) || (di->ops->chip_init == NULL) ||
	    (di->ops->set_input_current == NULL) ||
	    (di->ops->set_charge_current == NULL) ||
	    (di->ops->set_charge_enable == NULL) ||
	    (di->ops->set_otg_enable == NULL) ||
	    (di->ops->set_terminal_current == NULL) ||
	    (di->ops->set_terminal_voltage == NULL) ||
	    (di->ops->get_charge_state == NULL) ||
	    (di->ops->reset_watchdog_timer == NULL)) {
		hwlog_err("charge ops is NULL!\n");
		ret = -EINVAL;
		goto charge_fail_1;
	}

	init_crit.vbus = ADAPTER_5V;
	init_crit.charger_type = di->charger_type;
	(void)di->ops->chip_init(&init_crit);

	platform_set_drvdata(pdev, di);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	charge_lock = wakeup_source_register(&pdev->dev, "charge_wakelock");
	stop_charge_lock = wakeup_source_register(&pdev->dev, "stop_charge_wakelock");
#else
	charge_lock = wakeup_source_register("charge_wakelock");
	stop_charge_lock = wakeup_source_register("stop_charge_wakelock");
#endif
	if ((!charge_lock) || (!stop_charge_lock)) {
		hwlog_err("wakeup source register failed!\n");
		return -EINVAL;
}

	INIT_DELAYED_WORK(&di->charge_work, charge_monitor_work);
	INIT_DELAYED_WORK(&di->otg_work, charge_otg_work);
	INIT_WORK(&di->usb_work, charge_usb_work);
	INIT_WORK(&di->fault_work, charge_fault_work);
	INIT_WORK(&resume_wakelock_work, charge_resume_wakelock_work);

	di->usb_nb.notifier_call = charge_usb_notifier_call;
	ret = chip_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		hwlog_err("chip_charger_type_notifier_register failed\n");
		goto charge_fail_2;
	}

	di->fault_nb.notifier_call = charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&fault_notifier_list,
					     &di->fault_nb);
	if (ret < 0) {
		hwlog_err("charge_fault_register_notifier failed\n");
		goto charge_fail_2;
	}
	di->sysfs_data.iin_thl = di->core_data->iin_max;
	di->sysfs_data.ichg_thl = di->core_data->ichg_max;
	di->sysfs_data.iin_rt = di->core_data->iin_max;
	di->sysfs_data.ichg_rt = di->core_data->ichg_max;
	di->sysfs_data.vterm_rt = coul_drv_battery_vbat_max();
	di->sysfs_data.charge_enable = TRUE;
	di->sysfs_data.batfet_disable = FALSE;
	di->sysfs_data.wdt_disable = FALSE;
	di->sysfs_data.charge_limit = TRUE;
	di->sysfs_data.hiz_enable = FALSE;
	di->sysfs_data.charge_done_status = CHARGE_DONE_NON;
	di->sysfs_data.charge_done_sleep_status = CHARGE_DONE_SLEEP_DISABLED;
	mutex_init(&di->sysfs_data.dump_reg_lock);
	mutex_init(&di->sysfs_data.dump_reg_head_lock);
	mutex_init(&charge_wakelock_flag_lock);

	di->charge_fault = CHARGE_FAULT_NON;
	di->check_full_count = 0;
	charge_wake_lock();
	charge_rename_charger_type(type, di);
	charge_send_uevent(di);
	queue_work(system_power_efficient_wq, &di->usb_work);
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_CHARGER);
#endif

	ret = charge_sysfs_create_group(di);
	if (ret) {
		hwlog_err("can't create charge sysfs entries\n");
		goto charge_fail_3;
	}

	hwlog_info("huawei charger probe ok!\n");
	return 0;

charge_fail_3:
	charge_sysfs_remove_group(di);
charge_fail_2:
	wakeup_source_unregister(charge_lock);
	wakeup_source_unregister(stop_charge_lock);
charge_fail_1:
	di->ops = NULL;
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	return ret;
}
/* *********************************************************
 * Function:       charge_remove
 * Discription:    charge module remove
 * Parameters:   pdev:platform_device
 * return value:  0-sucess or others-fail
 * ******************************************************** */
static int charge_remove(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		hwlog_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}

	chip_charger_type_notifier_unregister(&di->usb_nb);
	atomic_notifier_chain_unregister(&fault_notifier_list, &di->fault_nb);
	charge_sysfs_remove_group(di);
	wakeup_source_unregister(charge_lock);
	wakeup_source_unregister(stop_charge_lock);
	cancel_delayed_work(&di->charge_work);
	cancel_delayed_work(&di->otg_work);

	if (NULL != di->ops) {
		di->ops = NULL;
		g_ops = NULL;
	}
	if (NULL != di->fcp_ops) {
		di->fcp_ops = NULL;
		g_fcp_ops = NULL;
	}
	kfree(di);
	di = NULL;
	return 0;
}
/* *********************************************************
 * Function:       charge_shutdown
 * Discription:    charge module shutdown
 * Parameters:   pdev:platform_device
 * return value:  NULL
 * ******************************************************** */
static void charge_shutdown(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);
	int ret = 0;

	hwlog_info("%s ++\n", __func__);
	if (NULL == di) {
		hwlog_err("[%s]di is NULL!\n", __func__);
		return;
	}
	ret = di->ops->set_otg_enable(FALSE);
	if (ret)
		hwlog_err("[%s]set otg default fail!\n", __func__);

	if (di->ops->set_charger_hiz) {
		ret = di->ops->set_charger_hiz(FALSE);
		if (ret)
			hwlog_err("[%s]set charger hiz default fail!\n", __func__);
	}

	chip_charger_type_notifier_unregister(&di->usb_nb);
	atomic_notifier_chain_unregister(&fault_notifier_list, &di->fault_nb);

	cancel_delayed_work(&di->charge_work);
	cancel_delayed_work(&di->otg_work);
	hwlog_info("%s --\n", __func__);

	return;
}

#ifdef CONFIG_PM
/* *********************************************************
 * Function:       charge_suspend
 * Discription:    charge module suspend
 * Parameters:   pdev:platform_device
 * state:unused
 * return value:  0-sucess or others-fail
 * ******************************************************** */
static int charge_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (di->charger_source == POWER_SUPPLY_TYPE_MAINS) {
		if (charge_is_charging_full(di)) {
			if (!charge_lock->active)
				cancel_delayed_work(&di->charge_work);
		}
	}
	charge_disable_watchdog(di);
	hwlog_info("%s --\n", __func__);

	return 0;
}
/* *********************************************************
 * Function:       charge_resume
 * Description:    charge module resume
 * Parameters:   pdev:platform_device
 * return value:  0-sucess or others-fail
 * ******************************************************** */
static int charge_resume(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	queue_work(system_power_efficient_wq, &resume_wakelock_work);
	if (di->charger_source == POWER_SUPPLY_TYPE_MAINS)
		queue_delayed_work(system_power_efficient_wq, &di->charge_work,
				   msecs_to_jiffies(0));

	charge_set_watchdog(di, WDT_TIME_40_S);

	hwlog_info("%s --\n", __func__);

	return 0;
}
#endif /* CONFIG_PM */

static struct of_device_id charge_match_table[] = {
	{
		.compatible = "huawei,charger",
		.data = NULL,
	},
	{},
};
static struct platform_driver charge_driver = {
	.probe = charge_probe,
	.remove = charge_remove,
#ifdef CONFIG_PM
	.suspend = charge_suspend,
	.resume = charge_resume,
#endif
	.shutdown = charge_shutdown,
	.driver =
		{
			.name = "huawei,charger",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(charge_match_table),
		},
};
/* *********************************************************
 * Function:       charge_init
 * Discription:    charge module initialization
 * Parameters:   NULL
 * return value:  0-sucess or others-fail
 * ******************************************************** */
static int __init charge_init(void)
{
	return platform_driver_register(&charge_driver);
}
/* *********************************************************
 * Function:       charge_exit
 * Discription:    charge module exit
 * Parameters:   NULL
 * return value:  NULL
 * ******************************************************** */
static void __exit charge_exit(void)
{
	platform_driver_unregister(&charge_driver);
}
device_initcall_sync(charge_init);
module_exit(charge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("charger module driver");
