/*
 * Copyright (C) 2016 Richtek Technology Corp.
 *
 * Richtek RT1711H Type-C Port Control Driver
 *
 * Author: TH <tsunghan_tsai@richtek.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>

#include <linux/semaphore.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/cpu.h>
#include <linux/version.h>

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/types.h>
#include <linux/sched/clock.h>
#endif

#include <huawei_platform/usb/pd/richtek/pd_dbg_info.h>
#include <huawei_platform/usb/pd/richtek/tcpci.h>
#include <huawei_platform/usb/pd/richtek/rt1711h.h>
#include <huawei_platform/log/hw_log.h>

#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#ifdef CONFIG_USE_CAMERA3_ARCH
#include <platform_include/camera/native/hw_extern_pmic.h>
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#ifdef CONFIG_RT_REGMAP
#include <huawei_platform/usb/pd/richtek/rt-regmap.h>
#endif

#if (KERNEL_VERSION(3, 9, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/rt.h>
#endif

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <uapi/linux/sched/types.h>
#endif

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/clock.h>
#endif

#include <huawei_platform/usb/hw_pogopin.h>

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
extern int support_smart_holder;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
#define RT1711H_DRV_VERSION "1.1.5_Huawei"
#ifdef CONFIG_CONTEXTHUB_PD
extern void hw_pd_wait_dptx_ready(void);
#endif /* CONFIG_CONTEXTHUB_PD */
struct rt1711_chip *g_chip_for_reg_read;

#define REG_DEFAULT_STATE 0x03

struct rt1711_chip {
	struct i2c_client *client;
	struct device *dev;
#ifdef CONFIG_RT_REGMAP
	struct rt_regmap_device *m_dev;
#endif /* CONFIG_RT_REGMAP */
	struct semaphore io_lock;
	struct semaphore suspend_lock;
	struct tcpc_desc *tcpc_desc;
	struct tcpc_device *tcpc;
	struct kthread_worker irq_worker;
	struct kthread_work irq_work;
	struct task_struct *irq_worker_task;

	atomic_t poll_count;
	struct delayed_work poll_work;

	int irq_gpio;
	int irq;
	int chip_id;
	int vendor_id;
	struct wakeup_source *rt1711h_wakelock;
	unsigned int pd_remove_cc_open;
};

static void rt1711h_wake_lock(struct rt1711_chip *chip)
{
	if (!chip)
		return;

	if (!chip->rt1711h_wakelock->active) {
		__pm_stay_awake(chip->rt1711h_wakelock);
		hwlog_info("rt1711h wake lock\n");
	}
}

static void rt1711h_wake_unlock(struct rt1711_chip *chip)
{
	if (!chip)
		return;

	if (chip->rt1711h_wakelock->active) {
		__pm_relax(chip->rt1711h_wakelock);
		hwlog_info("rt1711h wake unlock\n");
	}
}

#ifdef CONFIG_RT_REGMAP
RT_REG_DECL(TCPC_V10_REG_VID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_DID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TYPEC_REV, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PD_REV, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PDIF_REV, 2, RT_VOLATILE, {});

RT_REG_DECL(TCPC_V10_REG_ALERT, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_ALERT_MASK, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_STATUS_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_STATUS_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TCPC_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_ROLE_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_CC_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_COMMAND, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_MSG_HDR_INFO, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_DETECT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_BYTE_CNT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_BUF_FRAME_TYPE, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_HDR, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_DATA, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TRANSMIT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_BYTE_CNT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_HDR, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_DATA, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_CLK_CTRL2, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_CLK_CTRL3, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMC_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMCIO_RXDZSEL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_INT, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_IDLE_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_INTRST_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_WATCHDOG_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_I2CRST_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_SWRESET, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_TTCPC_FILTER, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_DRP_TOGGLE_CYCLE, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_DRP_DUTY_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMCIO_RXDZEN, 1, RT_VOLATILE, {});

static const rt_register_map_t rt1711_chip_regmap[] = {
	RT_REG(TCPC_V10_REG_VID),
	RT_REG(TCPC_V10_REG_PID),
	RT_REG(TCPC_V10_REG_DID),
	RT_REG(TCPC_V10_REG_TYPEC_REV),
	RT_REG(TCPC_V10_REG_PD_REV),
	RT_REG(TCPC_V10_REG_PDIF_REV),
	RT_REG(TCPC_V10_REG_ALERT),
	RT_REG(TCPC_V10_REG_ALERT_MASK),
	RT_REG(TCPC_V10_REG_POWER_STATUS_MASK),
	RT_REG(TCPC_V10_REG_FAULT_STATUS_MASK),
	RT_REG(TCPC_V10_REG_TCPC_CTRL),
	RT_REG(TCPC_V10_REG_ROLE_CTRL),
	RT_REG(TCPC_V10_REG_FAULT_CTRL),
	RT_REG(TCPC_V10_REG_POWER_CTRL),
	RT_REG(TCPC_V10_REG_CC_STATUS),
	RT_REG(TCPC_V10_REG_POWER_STATUS),
	RT_REG(TCPC_V10_REG_FAULT_STATUS),
	RT_REG(TCPC_V10_REG_COMMAND),
	RT_REG(TCPC_V10_REG_MSG_HDR_INFO),
	RT_REG(TCPC_V10_REG_RX_DETECT),
	RT_REG(TCPC_V10_REG_RX_BYTE_CNT),
	RT_REG(TCPC_V10_REG_RX_BUF_FRAME_TYPE),
	RT_REG(TCPC_V10_REG_RX_HDR),
	RT_REG(TCPC_V10_REG_RX_DATA),
	RT_REG(TCPC_V10_REG_TRANSMIT),
	RT_REG(TCPC_V10_REG_TX_BYTE_CNT),
	RT_REG(TCPC_V10_REG_TX_HDR),
	RT_REG(TCPC_V10_REG_TX_DATA),
	RT_REG(RT1711H_REG_CLK_CTRL2),
	RT_REG(RT1711H_REG_CLK_CTRL3),
	RT_REG(RT1711H_REG_BMC_CTRL),
	RT_REG(RT1711H_REG_BMCIO_RXDZSEL),
	RT_REG(RT1711H_REG_RT_STATUS),
	RT_REG(RT1711H_REG_RT_INT),
	RT_REG(RT1711H_REG_RT_MASK),
	RT_REG(RT1711H_REG_IDLE_CTRL),
	RT_REG(RT1711H_REG_INTRST_CTRL),
	RT_REG(RT1711H_REG_WATCHDOG_CTRL),
	RT_REG(RT1711H_REG_I2CRST_CTRL),
	RT_REG(RT1711H_REG_SWRESET),
	RT_REG(RT1711H_REG_TTCPC_FILTER),
	RT_REG(RT1711H_REG_DRP_TOGGLE_CYCLE),
	RT_REG(RT1711H_REG_DRP_DUTY_CTRL),
	RT_REG(RT1711H_REG_BMCIO_RXDZEN),
};
#define RT1711_CHIP_REGMAP_SIZE ARRAY_SIZE(rt1711_chip_regmap)

#endif /* CONFIG_RT_REGMAP */

static int rt1711_read_device(void *client, u32 reg, int len, void *dst)
{
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret = 0, count = 5;
#ifdef CONFIG_DIRECT_CHARGER
	ls_i2c_mutex_lock();
#endif /* CONFIG_DIRECT_CHARGER */
	while (count) {
		if (len > 1) {
			ret = i2c_smbus_read_i2c_block_data(i2c, reg, len, dst);
			if (ret < 0) {
				count--;
			} else {
#ifdef CONFIG_DIRECT_CHARGER
				ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
				return ret;
			}
		} else {
			ret = i2c_smbus_read_byte_data(i2c, reg);
			if (ret < 0) {
				count--;
			} else {
				*(u8 *)dst = (u8)ret;
#ifdef CONFIG_DIRECT_CHARGER
				ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
				return ret;
			}
		}
		udelay(100);
	}
#ifdef CONFIG_DIRECT_CHARGER
	ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
	return ret;
}

static int rt1711_write_device(void *client, u32 reg, int len, const void *src)
{
	const u8 *data = NULL;
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret = 0, count = 5;

#ifdef CONFIG_DIRECT_CHARGER
	ls_i2c_mutex_lock();
#endif /* CONFIG_DIRECT_CHARGER */
	while (count) {
		if (len > 1) {
			ret = i2c_smbus_write_i2c_block_data(i2c, reg, len,
				src);
			if (ret < 0) {
				count--;
			} else {
#ifdef CONFIG_DIRECT_CHARGER
				ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
				return ret;
			}
		} else {
			data = src;
			ret = i2c_smbus_write_byte_data(i2c, reg, *data);
			if (ret < 0) {
				count--;
			} else {
#ifdef CONFIG_DIRECT_CHARGER
				ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
				return ret;
			}
		}
		udelay(100);
	}
#ifdef CONFIG_DIRECT_CHARGER
	ls_i2c_mutex_unlock();
#endif /* CONFIG_DIRECT_CHARGER */
	return ret;
}

static int rt1711_reg_read(struct i2c_client *i2c, u8 reg)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	u8 val = 0;
	int ret = 0;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_read(chip->m_dev, reg, 1, &val);
#else
	ret = rt1711_read_device(chip->client, reg, 1, &val);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0) {
		dev_err(chip->dev, "rt1711 reg read fail\n");
		return ret;
	}
	return val;
}

static int rt1711_reg_write(struct i2c_client *i2c, u8 reg, const u8 data)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret = 0;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_write(chip->m_dev, reg, 1, &data);
#else
	ret = rt1711_write_device(chip->client, reg, 1, &data);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 reg write fail\n");
	return ret;
}

static int rt1711_block_read(struct i2c_client *i2c, u8 reg, int len, void *dst)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret = 0;
#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_read(chip->m_dev, reg, len, dst);
#else
	ret = rt1711_read_device(chip->client, reg, len, dst);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 block read fail\n");
	return ret;
}

static int rt1711_block_write(struct i2c_client *i2c, u8 reg, int len,
	const void *src)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret = 0;
#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_write(chip->m_dev, reg, len, src);
#else
	ret = rt1711_write_device(chip->client, reg, len, src);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 block write fail\n");
	return ret;
}

static int32_t rt1711_write_word(struct i2c_client *client, uint8_t reg_addr,
	uint16_t data)
{
	int ret;

	/* don't need swap */
	ret = rt1711_block_write(client, reg_addr, 2, (uint8_t *)&data);
	return ret;
}

static int32_t rt1711_read_word(struct i2c_client *client, uint8_t reg_addr,
	uint16_t *data)
{
	int ret;

	/* don't need swap */
	ret = rt1711_block_read(client, reg_addr, 2, (uint8_t *)data);
	return ret;
}

static inline int rt1711_i2c_write8(struct tcpc_device *tcpc, u8 reg,
	const u8 data)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);

	return rt1711_reg_write(chip->client, reg, data);
}

static inline int rt1711_i2c_write16(struct tcpc_device *tcpc, u8 reg,
	const u16 data)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);

	return rt1711_write_word(chip->client, reg, data);
}

static inline int rt1711_i2c_read8(struct tcpc_device *tcpc, u8 reg)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);

	return rt1711_reg_read(chip->client, reg);
}

static int rt1711_i2c_read16(struct tcpc_device *tcpc, u8 reg)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);
	u16 data = 0;
	int ret;

	ret = rt1711_read_word(chip->client, reg, &data);
	if (ret < 0)
		return ret;
	return data;
}

#ifdef CONFIG_RT_REGMAP
static struct rt_regmap_fops rt1711_regmap_fops = {
	.read_device = rt1711_read_device,
	.write_device = rt1711_write_device,
};
#endif /* CONFIG_RT_REGMAP */

static int rt1711_regmap_init(struct rt1711_chip *chip)
{
#ifdef CONFIG_RT_REGMAP
	struct rt_regmap_properties *props;
	char name[32];
	int len;
	int ret = 0;

	props = devm_kzalloc(chip->dev, sizeof(*props), GFP_KERNEL);
	if (!props)
		return -ENOMEM;

	props->register_num = RT1711_CHIP_REGMAP_SIZE;
	props->rm = rt1711_chip_regmap;

	props->rt_regmap_mode = RT_MULTI_BYTE | RT_CACHE_DISABLE |
		RT_IO_PASS_THROUGH | RT_DBG_GENERAL;
	snprintf(name, 32, "rt1711-%02x", chip->client->addr);

	len = strlen(name);
	props->name = kzalloc(len + 1, GFP_KERNEL);
	if (!props->name) {
		dev_err(chip->dev, "props->name kzalloc fail\n");
		ret = -ENOMEM;
		goto err_kzalloc_name;
	}

	props->aliases = kzalloc(len + 1, GFP_KERNEL);
	if (!props->aliases) {
		dev_err(chip->dev, "props->aliases kzalloc fail\n");
		ret = -ENOMEM;
		goto err_kzalloc_aliases;
	}

	strcpy((char *)props->name, name);
	strcpy((char *)props->aliases, name);
	props->io_log_en = 0;

	chip->m_dev = rt_regmap_device_register(props, &rt1711_regmap_fops,
		chip->dev, chip->client, chip);
	if (!chip->m_dev) {
		dev_err(chip->dev, "rt1711 chip rt_regmap register fail\n");
		ret = -EINVAL;
		goto err_device_register;
	}
	kfree(props->name);
	kfree(props->aliases);
	return 0;

err_device_register:
	kfree(props->aliases);
err_kzalloc_aliases:
	kfree(props->name);
err_kzalloc_name:
	devm_kfree(chip->dev, props);
	return ret;
#endif /* CONFIG_RT_REGMAP */
	return 0;
}

static int rt1711_regmap_deinit(struct rt1711_chip *chip)
{
#ifdef CONFIG_RT_REGMAP
	rt_regmap_device_unregister(chip->m_dev);
#endif /* CONFIG_RT_REGMAP */
	return 0;
}

static int rt1711_software_reset(struct tcpc_device *tcpc)
{
	int ret = rt1711_i2c_write8(tcpc, RT1711H_REG_SWRESET, 1);

	if (ret < 0)
		return ret;

	mdelay(1);
	return 0;
}

static inline int rt1711_command(struct tcpc_device *tcpc, uint8_t cmd)
{
	return rt1711_i2c_write8(tcpc, TCPC_V10_REG_COMMAND, cmd);
}

static int rt1711_init_alert_mask(struct tcpc_device *tcpc)
{
	uint16_t mask;
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);

	mask = TCPC_V10_REG_ALERT_CC_STATUS | TCPC_V10_REG_ALERT_POWER_STATUS;

#ifdef CONFIG_USB_POWER_DELIVERY
	/* Need to handle RX overflow */
	mask |= TCPC_V10_REG_ALERT_TX_SUCCESS | TCPC_V10_REG_ALERT_TX_DISCARDED
			| TCPC_V10_REG_ALERT_TX_FAILED
			| TCPC_V10_REG_ALERT_RX_HARD_RST
			| TCPC_V10_REG_ALERT_RX_STATUS
			| TCPC_V10_REG_RX_OVERFLOW;
#endif

	mask |= TCPC_REG_ALERT_FAULT;

	return rt1711_write_word(chip->client, TCPC_V10_REG_ALERT_MASK, mask);
}

static int rt1711_init_power_status_mask(struct tcpc_device *tcpc)
{
	const uint8_t mask = TCPC_V10_REG_POWER_STATUS_VBUS_PRES;

	return rt1711_i2c_write8(tcpc,
			TCPC_V10_REG_POWER_STATUS_MASK, mask);
}

static int rt1711_init_fault_mask(struct tcpc_device *tcpc)
{
	const uint8_t mask =
		TCPC_V10_REG_FAULT_STATUS_VCONN_OV |
		TCPC_V10_REG_FAULT_STATUS_VCONN_OC;

	return rt1711_i2c_write8(tcpc,
			TCPC_V10_REG_FAULT_STATUS_MASK, mask);
}

static int rt1711_init_rt_mask(struct tcpc_device *tcpc)
{
	uint8_t rt_mask = 0;
#ifdef CONFIG_TCPC_WATCHDOG_EN
	rt_mask |= RT1711H_REG_M_WATCHDOG;
#endif /* CONFIG_TCPC_WATCHDOG_EN */
#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	rt_mask |= RT1711H_REG_M_VBUS_80;
#endif /* CONFIG_TCPC_VSAFE0V_DETECT_IC */

#ifdef CONFIG_TYPEC_CAP_RA_DETACH
	if (tcpc->tcpc_flags & TCPC_FLAGS_CHECK_RA_DETACHE)
		rt_mask |= RT1711H_REG_M_RA_DETACH;
#endif /* CONFIG_TYPEC_CAP_RA_DETACH */

#ifdef CONFIG_TYPEC_CAP_LPM_WAKEUP_WATCHDOG
	if (tcpc->tcpc_flags & TCPC_FLAGS_LPM_WAKEUP_WATCHDOG)
		rt_mask |= RT1711H_REG_M_WAKEUP;
#endif	/* CONFIG_TYPEC_CAP_LPM_WAKEUP_WATCHDOG */

	return rt1711_i2c_write8(tcpc, RT1711H_REG_RT_MASK, rt_mask);
}

static void rt1711_poll_ctrl(struct rt1711_chip *chip)
{
	cancel_delayed_work_sync(&chip->poll_work);

	if (atomic_read(&chip->poll_count) == 0) {
		atomic_inc(&chip->poll_count);
		cpu_idle_poll_ctrl(true);
	}

	/* 40:delay 40ms for poll work */
	schedule_delayed_work(&chip->poll_work, msecs_to_jiffies(40));
}

static void rt1711_irq_work_handler(struct kthread_work *work)
{
	struct rt1711_chip *chip =
			container_of(work, struct rt1711_chip, irq_work);
	int regval = 0;
	int gpio_val;

#ifdef CONFIG_CONTEXTHUB_PD
	hw_pd_wait_dptx_ready();
#endif
	rt1711_poll_ctrl(chip);
	/* make sure I2C bus had resumed */
	down(&chip->suspend_lock);
	tcpci_lock_typec(chip->tcpc);

#ifdef DEBUG_GPIO
	gpio_set_value(DEBUG_GPIO, 1);
#endif

	do {
		regval = tcpci_alert(chip->tcpc);
		if (regval)
			break;
		gpio_val = gpio_get_value(chip->irq_gpio);
	} while (gpio_val == 0);

	tcpci_unlock_typec(chip->tcpc);
	up(&chip->suspend_lock);

#ifdef DEBUG_GPIO
	gpio_set_value(DEBUG_GPIO, 1);
#endif
	rt1711h_wake_unlock(chip);
}

static void rt1711_poll_work(struct work_struct *work)
{
	struct rt1711_chip *chip = container_of(
		work, struct rt1711_chip, poll_work.work);

	if (atomic_dec_and_test(&chip->poll_count))
		cpu_idle_poll_ctrl(false);
}

static irqreturn_t rt1711_intr_handler(int irq, void *data)
{
	struct rt1711_chip *chip = data;

	if (!chip)
		return IRQ_HANDLED;

	rt1711h_wake_lock(chip);
#ifdef DEBUG_GPIO
	gpio_set_value(DEBUG_GPIO, 0);
#endif /* DEBUG_GPIO */
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	kthread_queue_work(&chip->irq_worker, &chip->irq_work);
#else
	queue_kthread_work(&chip->irq_worker, &chip->irq_work);
#endif /* KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE */
	return IRQ_HANDLED;
}

static int rt1711_init_alert(struct tcpc_device *tcpc)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
	int ret;
	char *name = NULL;
	int len;

	/* Clear Alert Mask & Status */
	rt1711_write_word(chip->client, TCPC_V10_REG_ALERT_MASK, 0);
	rt1711_write_word(chip->client, TCPC_V10_REG_ALERT, 0xffff);

	len = strlen(chip->tcpc_desc->name);
	/* 5:extra space for "-IRQ" */
	name = kzalloc(len + 5, GFP_KERNEL);
	if (!name) {
		pr_err("rt1711_init_alert name kzalloc fail\n");
		return -ENOMEM;
	}

	/* 5:extra space for "-IRQ" */
	snprintf(name, len + 5, "%s-IRQ", chip->tcpc_desc->name);

	pr_info("%s name = %s\n", __func__, chip->tcpc_desc->name);
	pr_info("%s gpio # = %d\n", __func__, chip->irq_gpio);

	ret = gpio_request(chip->irq_gpio, name);
#ifdef DEBUG_GPIO
	gpio_request(DEBUG_GPIO, "debug_latency_pin");
	gpio_direction_output(DEBUG_GPIO, 1);
#endif /* DEBUG_GPIO */
	if (ret < 0) {
		pr_err("Error: failed to request GPIO%d ret = %d\n",
			chip->irq_gpio, ret);
		return ret;
	}
	pr_info("GPIO requested\n");

	ret = gpio_direction_input(chip->irq_gpio);
	if (ret < 0) {
		pr_err("Error: failed to set GPIO%d as input pin ret = %d\n",
			chip->irq_gpio, ret);
		return ret;
	}

	chip->irq = gpio_to_irq(chip->irq_gpio);
	pr_info("%s : IRQ number = %d\n", __func__, chip->irq);

	if (ret < 0) {
		pr_err("Error: failed to request irq%d gpio = %d, ret = %d\n",
			chip->irq, chip->irq_gpio, ret);
		return ret;
	}
	pr_info("%s : irq initialized\n", __func__);

#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	kthread_init_worker(&chip->irq_worker);
#else
	init_kthread_worker(&chip->irq_worker);
#endif /* KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE */
	chip->irq_worker_task = kthread_run(kthread_worker_fn,
		&chip->irq_worker, chip->tcpc_desc->name);
	if (IS_ERR(chip->irq_worker_task)) {
		pr_err("Error: Could not create tcpc task\n");
		return -EINVAL;
	}

	sched_setscheduler(chip->irq_worker_task, SCHED_FIFO, &param);
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	kthread_init_work(&chip->irq_work, rt1711_irq_work_handler);
#else
	init_kthread_work(&chip->irq_work, rt1711_irq_work_handler);
#endif /* KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE */
	pr_info("IRQF_NO_THREAD Test\n");
	ret = request_irq(chip->irq, rt1711_intr_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_THREAD, name, chip);

	enable_irq_wake(chip->irq);

	return 0;
}

int rt1711_alert_status_clear(struct tcpc_device *tcpc, uint32_t mask)
{
	int ret;
	uint16_t mask_t1;

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	uint8_t mask_t2;
#endif /* CONFIG_TCPC_VSAFE0V_DETECT_IC */

	/* Write 1 clear */
	mask_t1 = (uint16_t) mask;
	if (mask_t1) {
		ret = rt1711_i2c_write16(tcpc, TCPC_V10_REG_ALERT, mask_t1);
		if (ret < 0)
			return ret;
	}

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	mask_t2 = mask >> 16;
	if (mask_t2) {
		ret = rt1711_i2c_write8(tcpc, RT1711H_REG_RT_INT, mask_t2);
		if (ret < 0)
			return ret;
	}
#endif	/* CONFIG_TCPC_VSAFE0V_DETECT_IC */

	return 0;
}

static int rt1711h_set_clock_gating(struct tcpc_device *tcpc_dev, bool en)
{
	int ret = 0;

#ifdef CONFIG_TCPC_CLOCK_GATING
	uint8_t clk2 = RT1711H_REG_CLK_DIV_600K_EN
		| RT1711H_REG_CLK_DIV_300K_EN | RT1711H_REG_CLK_CK_300K_EN;

	uint8_t clk3 = RT1711H_REG_CLK_DIV_2P4M_EN;

	if (!en) {
		clk2 |= RT1711H_REG_CLK_BCLK2_EN | RT1711H_REG_CLK_BCLK_EN;
		clk3 |= RT1711H_REG_CLK_CK_24M_EN | RT1711H_REG_CLK_PCLK_EN;
	}

	if (en)
		ret = rt1711_alert_status_clear(tcpc_dev,
			TCPC_REG_ALERT_RX_STATUS |
			TCPC_REG_ALERT_RX_HARD_RST |
			TCPC_REG_ALERT_RX_BUF_OVF);

	if (ret == 0)
		ret = rt1711_i2c_write8(tcpc_dev, RT1711H_REG_CLK_CTRL2, clk2);
	if (ret == 0)
		ret = rt1711_i2c_write8(tcpc_dev, RT1711H_REG_CLK_CTRL3, clk3);
#endif /* CONFIG_TCPC_CLOCK_GATING */

	return ret;
}

static int rt1711h_init_cc_params(struct tcpc_device *tcpc, uint8_t cc_res)
{
	int rv = 0;
	int ret = 0;

#ifdef CONFIG_USB_POWER_DELIVERY
#ifdef CONFIG_USB_PD_SNK_DFT_NO_GOOD_CRC
	uint8_t en, sel;

	if (cc_res == TYPEC_CC_VOLT_SNK_DFT) {
		en = 0;

		if (tcpc->desc.vcon_ocp_enable == 1)
			sel = RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_800MA_DFT;
		else
			sel = RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_600MA_DFT;
	} else {
		en = 1;

		if (tcpc->desc.vcon_ocp_enable == 1)
			sel = RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_800MA;
		else
			sel = RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_600MA;
	}

	rv = rt1711_i2c_write8(tcpc, RT1711H_REG_BMCIO_RXDZEN, en);

	if (rv == 0) {
		if (tcpc->desc.vcon_ocp_enable == 1) {
			ret = rt1711_i2c_read8(tcpc, RT1711H_REG_BMCIO_VCONOCP);

			if (ret >= 0) {
				ret = ret & (
					~RT1711H_REG_BMCIO_SOFTSTART_TIME_MASK);
				rv = rt1711_i2c_write8(tcpc,
					RT1711H_REG_BMCIO_VCONOCP,
					ret | RT1711H_REG_BMCIO_SOFTSTART_TIME);
			} else {
				rv = -1;
			}
		}

		rv |= rt1711_i2c_write8(tcpc, RT1711H_REG_BMCIO_RXDZSEL, sel);
	}

#endif /* CONFIG_USB_PD_SNK_DFT_NO_GOOD_CRC */
#endif /* CONFIG_USB_POWER_DELIVERY */

	return rv;
}

static int rt1711h_tcpc_set_vswing(struct tcpc_device *tcpc)
{
	int ret;
	unsigned char reg_val;

	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_BMCIO_VCONOCP);
	pr_info("%s read VCONOCP ret=  %d\n", __func__, ret);
	if (ret < 0)
		return ret;

	reg_val = (unsigned char)ret;
	rt1711_i2c_write8(tcpc, RT1711H_REG_BMCIO_VCONOCP,
		reg_val & (~RT1711H_REG_ADJ_VBUS_MEASURE));

	rt1711_i2c_write8(tcpc, RT1711H_REG_CMD_REG2,
		RT1711H_REG_CMD_REG2_ENABLE);

	rt1711_i2c_write8(tcpc, RT1711H_REG_CMD_REG1,
		RT1711H_REG_CMD_REG1_ENABLE);

	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_ADJ_VSWING);
	pr_info("%s read VSWING ret=  %d\n", __func__, ret);
	if (ret < 0)
		return ret;

	reg_val = (unsigned char)ret;
	reg_val = reg_val & (~RT1711H_REG_SETTING_VSWING_MASK);
	rt1711_i2c_write8(tcpc, RT1711H_REG_ADJ_VSWING,
		reg_val | RT1711H_REG_SETTING_VSWING);

	rt1711_i2c_write8(tcpc, RT1711H_REG_CMD_REG2,
		RT1711H_REG_CMD_REG2_CLEAN);
	rt1711_i2c_write8(tcpc, RT1711H_REG_CMD_REG1,
		RT1711H_REG_CMD_REG1_CLEAN);

	return 0;
}

static int rt1711_tcpc_init(struct tcpc_device *tcpc, bool sw_reset)
{
	int ret;
	int vcon_climiten_val = 0;
	u32 retry_discard_old = 0;
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);

	RT1711_INFO("\n");

	if (sw_reset) {
		ret = rt1711_software_reset(tcpc);
		if (ret < 0)
			return ret;
	}

	if (chip->vendor_id == ETEK_ET7303_VID) {
		ret = rt1711h_tcpc_set_vswing(tcpc);
		if (ret < 0)
			return ret;
	}

	/* CK_300K from 320K, SHIPPING off, AUTOIDLE enable, TIMEOUT = 32ms */
	ret = rt1711_i2c_write8(tcpc, RT1711H_REG_IDLE_CTRL,
		RT1711H_REG_IDLE_SET(0, 1, 1, 2));
	if (ret < 0)
		return ret;

#ifdef CONFIG_TCPC_I2CRST_EN
	ret = rt1711_i2c_write8(tcpc,
		RT1711H_REG_I2CRST_CTRL,
		RT1711H_REG_I2CRST_SET(true, 0x0f));
	if (ret < 0)
		return ret;
#endif	/* CONFIG_TCPC_I2CRST_EN */

	/* UFP Both RD setting */
	/* DRP = 0, RpVal = 0 (Default), Rd, Rd */
	ret = rt1711_i2c_write8(tcpc, TCPC_V10_REG_ROLE_CTRL,
		TCPC_V10_REG_ROLE_CTRL_RES_SET(0, 0, CC_RD, CC_RD));
	if (ret < 0)
		return ret;

	if (chip->chip_id == RT1711H_DID_A) {
		ret = rt1711_i2c_write8(tcpc, TCPC_V10_REG_FAULT_CTRL,
			TCPC_V10_REG_FAULT_CTRL_DIS_VCONN_OV);
		if (ret < 0)
			return ret;
	}

	/*
	 * CC Detect Debounce : 26.7*val us
	 * Transition window count : spec 12~20us, based on 2.4MHz
	 * DRP Toggle Cycle : 51.2 + 6.4*val ms
	 * DRP Duyt Ctrl : dcSRC: /1024
	 */

	/* 5: tcpcfilter value */
	ret = rt1711_i2c_write8(tcpc, RT1711H_REG_TTCPC_FILTER, 5);
	if (ret < 0)
		return ret;

	/* 4: drptogglecycle value */
	ret = rt1711_i2c_write8(tcpc, RT1711H_REG_DRP_TOGGLE_CYCLE, 4);
	if (ret < 0)
		return ret;

	/* 400: drpdutyctrl value */
	ret = rt1711_i2c_write16(tcpc, RT1711H_REG_DRP_DUTY_CTRL, 400);
	if (ret < 0)
		return ret;

	/* Vconn OC */
	vcon_climiten_val = (tcpc->desc.vcon_ocp_enable == 1) ? 0 : 1;
	ret = rt1711_i2c_write8(tcpc, RT1711H_REG_VCONN_CLIMITEN,
		vcon_climiten_val);
	if (ret < 0)
		return ret;

	/* RX/TX Clock Gating (Auto Mode) */
	if (!sw_reset) {
		ret = rt1711h_set_clock_gating(tcpc, true);
		if (ret < 0)
			return ret;
	}

	if (!(tcpc->tcpc_flags & TCPC_FLAGS_RETRY_CRC_DISCARD))
		retry_discard_old = 1;
	ret = rt1711_i2c_write8(tcpc, RT1711H_REG_PHY_CTRL1,
		RT1711H_REG_PHY_CTRL1_SET(retry_discard_old, 7, 0, 1));
	if (ret < 0)
		return ret;

	ret = tcpci_alert_status_clear(tcpc, 0xffffffff);
	if (ret < 0)
		return ret;

	ret = rt1711_init_power_status_mask(tcpc);
	if (ret < 0)
		return ret;

	ret = rt1711_init_alert_mask(tcpc);
	if (ret < 0)
		return ret;

	ret = rt1711_init_fault_mask(tcpc);
	if (ret < 0)
		return ret;

	ret = rt1711_init_rt_mask(tcpc);
	return ret;
}

int rt1711_fault_status_clear(struct tcpc_device *tcpc, uint8_t status)
{
	/* Write 1 clear (Check it later )*/
	int ret;

	rt1711_i2c_write8(tcpc, TCPC_V10_REG_FAULT_STATUS, status);

	/* discharge ... */
	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_BMC_CTRL);
	if (ret < 0)
		return ret;

	rt1711_i2c_write8(tcpc,
		RT1711H_REG_BMC_CTRL, ret & (~RT1711H_REG_DISCHARGE_EN));

	return 0;
}

int rt1711_get_alert_status(struct tcpc_device *tcpc, uint32_t *alert)
{
	int ret;

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	uint8_t v2;
#endif

	ret = rt1711_i2c_read16(tcpc, TCPC_V10_REG_ALERT);
	if (ret < 0)
		return ret;

	*alert = (uint16_t) ret;

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_RT_INT);
	if (ret < 0)
		return ret;

	v2 = (uint8_t) ret;
	*alert |= v2 << 16;
#endif

	return 0;
}

static int rt1711_get_power_status(
		struct tcpc_device *tcpc, uint16_t *pwr_status)
{
	int ret;

	ret = rt1711_i2c_read8(tcpc, TCPC_V10_REG_POWER_STATUS);
	if (ret < 0)
		return ret;

	*pwr_status = 0;

	if (ret & TCPC_V10_REG_POWER_STATUS_VBUS_PRES)
		*pwr_status |= TCPC_REG_POWER_STATUS_VBUS_PRES;

#ifdef CONFIG_TCPC_VSAFE0V_DETECT_IC
	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_RT_STATUS);
	if (ret < 0)
		return ret;

	if (ret & RT1711H_REG_VBUS_80)
		*pwr_status |= TCPC_REG_POWER_STATUS_EXT_VSAFE0V;
#endif
	return 0;
}

int rt1711_get_fault_status(struct tcpc_device *tcpc, uint8_t *status)
{
	int ret;

	ret = rt1711_i2c_read8(tcpc, TCPC_V10_REG_FAULT_STATUS);
	if (ret < 0)
		return ret;
	*status = (uint8_t) ret;
	return 0;
}

static int rt1711_get_cc(struct tcpc_device *tcpc, int *cc1, int *cc2)
{
	int status, role_ctrl, cc_role;
	u32 act_as_sink;
	bool act_as_drp = false;

	status = rt1711_i2c_read8(tcpc, TCPC_V10_REG_CC_STATUS);
	if (status < 0)
		return status;

	role_ctrl = rt1711_i2c_read8(tcpc, TCPC_V10_REG_ROLE_CTRL);
	if (role_ctrl < 0)
		return role_ctrl;

	if (status & TCPC_V10_REG_CC_STATUS_DRP_TOGGLING) {
		*cc1 = TYPEC_CC_DRP_TOGGLING;
		*cc2 = TYPEC_CC_DRP_TOGGLING;
		return 0;
	}

	*cc1 = TCPC_V10_REG_CC_STATUS_CC1(status);
	*cc2 = TCPC_V10_REG_CC_STATUS_CC2(status);

	act_as_drp = TCPC_V10_REG_ROLE_CTRL_DRP & role_ctrl;

	if (act_as_drp) {
		act_as_sink = TCPC_V10_REG_CC_STATUS_DRP_RESULT(status);
	} else {
		cc_role =  TCPC_V10_REG_CC_STATUS_CC1(role_ctrl);
		if (cc_role == TYPEC_CC_RP)
			act_as_sink = 0;
		else
			act_as_sink = 1; /* 1:act as sink sucess */
	}

	/*
	 * If status is not open, then OR in termination to convert to
	 * enum tcpc_cc_voltage_status.
	 */

	if (*cc1 != TYPEC_CC_VOLT_OPEN)
		*cc1 |= (act_as_sink << 2);

	if (*cc2 != TYPEC_CC_VOLT_OPEN)
		*cc2 |= (act_as_sink << 2);

	rt1711h_init_cc_params(tcpc,
		(uint8_t)tcpc->typec_polarity ? *cc2 : *cc1);

	return 0;
}

static int rt1711_enable_vsafe0v_detect(
	struct tcpc_device *tcpc, bool enable)
{
	int ret;

	ret = rt1711_i2c_read8(tcpc, RT1711H_REG_RT_MASK);
	RT1711_INFO("vsafe0v_mask 0x%2x, enable %d\n", ret, enable);
	if (ret < 0)
		return ret;
	if (enable)
		ret |= RT1711H_REG_M_VBUS_80;
	else
		ret &= (~RT1711H_REG_M_VBUS_80);
	return rt1711_i2c_write8(tcpc, RT1711H_REG_RT_MASK, (u8)ret);
}
static int rt1711_set_cc(struct tcpc_device *tcpc, int pull)
{
	int ret;
	uint8_t data;
	u32 rp_lvl = TYPEC_CC_PULL_GET_RP_LVL(pull);

	RT1711_INFO("pull %d\n", pull);
	pull = TYPEC_CC_PULL_GET_RES(pull);
	if (pull == TYPEC_CC_DRP) {
		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(1, rp_lvl, TYPEC_CC_RD,
			TYPEC_CC_RD);

		ret = rt1711_i2c_write8(
			tcpc, TCPC_V10_REG_ROLE_CTRL, data);

		if (ret == 0) {
			rt1711_enable_vsafe0v_detect(tcpc, false);
			ret = rt1711_command(tcpc, TCPM_CMD_LOOK_CONNECTION);
		}
	} else {
#ifdef CONFIG_USB_POWER_DELIVERY
		if (pull == TYPEC_CC_RD && tcpc->pd_wait_pr_swap_complete)
			rt1711h_init_cc_params(tcpc, TYPEC_CC_VOLT_SNK_DFT);
#endif /* CONFIG_USB_POWER_DELIVERY */
		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(0, rp_lvl, pull, pull);
		ret = rt1711_i2c_write8(tcpc, TCPC_V10_REG_ROLE_CTRL, data);
	}

	return 0;
}
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
#define VID 0x12d1
#define VID_INDEX 1
#define VID_MASK 0x0ffff
#define PID 0x3B20
#define PID_INDEX 3
#define PID_MASK 0x0ffff

#define  ENABLE_DEBUG_DEVICE  1


int is_smart_holder(uint32_t *vdos, int size)
{
	int i;

	if (!vdos)
		return 0;

	/* 16:pid shift */
	if (((vdos[VID_INDEX] & (VID_MASK)) == VID) &&
		(((vdos[PID_INDEX] >> 16) & (PID_MASK)) == PID))
		return 1;

#ifdef ENABLE_DEBUG_DEVICE
	for (i = 0; i < size; i++)
		hwlog_info("vdos[%d]: 0x%x\n", i, vdos[i]);

	if (((vdos[VID_INDEX] & (VID_MASK)) == VID) &&
		((vdos[PID_INDEX] & (PID_MASK)) == 0))
		return 1;
#endif /* ENABLE_DEBUG_DEVICE */

	return 0;
}

int tcpm_is_cust_src2_cable(void)
{
	uint32_t vdos[VDO_MAX_SIZE];
	int ret;

	memset(vdos, 0, VDO_MAX_SIZE);
	ret = tcpm_inquire_cust_src2_cable_vdo(g_chip_for_reg_read->tcpc, vdos,
		VDO_MAX_SIZE);
	if (ret) {
		if (is_smart_holder(vdos, VDO_MAX_SIZE)) {
			hwlog_info("this is smart holder\n");
			return 1;
		}
	} else {
		hwlog_info("inquire vdo failed\n");
	}
	return 0;
}

struct cable_vdo_ops rt1711h_cable_vdo_ops = {
	.is_cust_src2_cable = tcpm_is_cust_src2_cable,
};
#endif

#ifdef CONFIG_POGO_PIN
static int tcpm_typec_detect_disable(bool disable)
{
	if (!g_chip_for_reg_read) {
		hwlog_info("rt1711 dev null\n");
		return 0;
	}
	return tcpm_typec_disable_function(g_chip_for_reg_read->tcpc, disable);
}

static int tcpm_typec_detect_vbus(void)
{
	if (!g_chip_for_reg_read) {
		hwlog_info("rt1711 dev null\n");
		return 0;
	}
	return tcpm_typec_vbus_detect(g_chip_for_reg_read->tcpc);
}

struct pogopin_cc_ops rt1711h_cc_detect_ops = {
	.typec_detect_disable = tcpm_typec_detect_disable,
	.typec_detect_vbus = tcpm_typec_detect_vbus,
};
#endif /* CONFIG_POGO_PIN */

static bool rt1711_is_reg_default(struct tcpc_device *tcpc)
{
	int rv;

	rv = rt1711_i2c_read8(tcpc, RT1711H_REG_DRP_TOGGLE_CYCLE);

	hwlog_info("%s: reg0x%02X = 0x%02X\n", __func__,
		RT1711H_REG_DRP_TOGGLE_CYCLE, rv);
	return rv == REG_DEFAULT_STATE;
}

void rt1711h_set_cc_mode(int mode)
{
	int pull = mode ? TYPEC_CC_DRP : TYPEC_CC_RD;
	struct tcpc_device *tcpc_dev = NULL;

	if (!g_chip_for_reg_read || !g_chip_for_reg_read->tcpc) {
		hwlog_info("g_chip_for_reg_read or tcpc is NULL!\n");
		return;
	}
	tcpc_dev = g_chip_for_reg_read->tcpc;

	hwlog_info("rt1711h_set_cc_mode mode %d ,pull %d\n", mode, pull);
	tcpci_set_cc(tcpc_dev, pull);
	if (pull == TYPEC_CC_RD)
		tcpc_dev->typec_role = TYPEC_ROLE_SNK;
	else
		tcpc_dev->typec_role = TYPEC_ROLE_TRY_SNK;
}
EXPORT_SYMBOL(rt1711h_set_cc_mode);

int rt1711h_get_cc_mode(void)
{
	struct tcpc_device *tcpc_dev = NULL;

	if (!g_chip_for_reg_read || !g_chip_for_reg_read->tcpc) {
		hwlog_info("g_chip_for_reg_read or tcpc is NULL!\n");
		return TYPEC_ROLE_UNKNOWN;
	}

	tcpc_dev = g_chip_for_reg_read->tcpc;
	return tcpc_dev->typec_role;
}
EXPORT_SYMBOL(rt1711h_get_cc_mode);


int rt1711h_get_cc_state(void)
{
	int val = 0;
	struct rt1711_chip *chip = g_chip_for_reg_read;

	val = rt1711_reg_read(chip->client, RT1711_CC_STATUS);
	pr_info("%s:cc_state_REG0x1D = 0x%x\n", __func__, val);
	val = val & CC_STATUS_MASK;
	return val;
}
EXPORT_SYMBOL(rt1711h_get_cc_state);

static int rt1711_set_polarity(struct tcpc_device *tcpc, int polarity)
{
	int data;

	/* 0 and 1 is subscript of the typec_remote_cc */
	if (polarity < 0 || polarity > 1)
		return -1;

	data = rt1711h_init_cc_params(tcpc,
		tcpc->typec_remote_cc[polarity]);
	if (data)
		return data;

	data = rt1711_i2c_read8(tcpc, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT;
	data |= polarity ? TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT : 0;

	return rt1711_i2c_write8(tcpc, TCPC_V10_REG_TCPC_CTRL, data);
}

static int rt1711_set_vconn(struct tcpc_device *tcpc, int enable)
{
	int rv;
	int data;

	data = rt1711_i2c_read8(tcpc, TCPC_V10_REG_POWER_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_POWER_CTRL_VCONN;
	data |= enable ? TCPC_V10_REG_POWER_CTRL_VCONN : 0;

	rv = rt1711_i2c_write8(tcpc, TCPC_V10_REG_POWER_CTRL, data);
	if (rv < 0)
		return rv;

	rv = rt1711_i2c_write8(tcpc, RT1711H_REG_IDLE_CTRL,
		RT1711H_REG_IDLE_SET(0, 1, enable ? 0 : 1, 2));

	return rv;
}

static int rt1711_mask_vsafe0v(struct tcpc_device *tcpc_dev, int enable)
{
	int rv, data;

	data = rt1711_i2c_read8(tcpc_dev, RT1711H_REG_RT_MASK);
	RT1711_INFO("vsafe0v_mask 0x%2x, enable %d\n", data, enable);
	if (data < 0)
		return data;

	data &= ~RT1711H_REG_M_VBUS_80;
	data |= enable ? RT1711H_REG_M_VBUS_80 : 0;

	rv = rt1711_i2c_write8(tcpc_dev, RT1711H_REG_RT_MASK, data);

	return rv;
}

#ifdef CONFIG_TCPC_LOW_POWER_MODE
static int rt1711_is_low_power_mode(struct tcpc_device *tcpc_dev)
{
	int rv = rt1711_i2c_read8(tcpc_dev, RT1711H_REG_BMC_CTRL);

	if (rv < 0)
		return rv;

	return (rv & RT1711H_REG_BMCIO_LPEN) != 0;
}

static int rt1711_set_low_power_mode(
		struct tcpc_device *tcpc_dev, bool en, int pull)
{
	int rv = 0;
	uint8_t data;

	RT1711_INFO("en %d, pull %d\n", en, pull);

	if (en) {
		data = RT1711H_REG_BMCIO_LPEN;

		if ((u32)pull & TYPEC_CC_RP)
			data |= RT1711H_REG_BMCIO_LPRPRD;

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
		data |= (RT1711H_REG_VBUS_DET_EN | RT1711H_REG_BMCIO_BG_EN);
#endif	/* CONFIG_TYPEC_CAP_NORP_SRC */
	} else {
		data = RT1711H_REG_BMCIO_BG_EN |
			RT1711H_REG_VBUS_DET_EN | RT1711H_REG_BMCIO_OSC_EN;
		rt1711_enable_vsafe0v_detect(tcpc_dev, true);
	}

	rv = rt1711_i2c_write8(tcpc_dev, RT1711H_REG_BMC_CTRL, data);
	return rv;
}
#endif	/* CONFIG_TCPC_LOW_POWER_MODE */

#ifdef CONFIG_TCPC_WATCHDOG_EN
int rt1711h_set_watchdog(struct tcpc_device *tcpc_dev, bool en)
{
	uint8_t data;

	if (!tcpc_dev)
		return -EINVAL;

	/* 7 : timeout unit is ms */
	data = RT1711H_REG_WATCHDOG_CTRL_SET(en, 7);
	return rt1711_i2c_write8(tcpc_dev, RT1711H_REG_WATCHDOG_CTRL, data);
}
#endif /* CONFIG_TCPC_WATCHDOG_EN */

#ifdef CONFIG_TCPC_INTRST_EN
int rt1711h_set_intrst(struct tcpc_device *tcpc_dev, bool en)
{
	return rt1711_i2c_write8(tcpc_dev,
		RT1711H_REG_INTRST_CTRL, RT1711H_REG_INTRST_SET(en, 3));
}
#endif /* CONFIG_TCPC_INTRST_EN */

static int rt1711_tcpc_deinit(struct tcpc_device *tcpc_dev)
{
#ifdef CONFIG_TCPC_SHUTDOWN_CC_DETACH
	bool pd_finish = false;
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc_dev);

	if (!chip) {
		hwlog_info("rt1711_chip is NULL!\n");
		return -EINVAL;
	}

	if (chip->pd_remove_cc_open == true)
		pd_finish = pd_dpm_get_pd_finish_flag();

	rt1711_set_cc(tcpc_dev, TYPEC_CC_DRP);
	if ((chip->pd_remove_cc_open != true) || (pd_finish == true))
		rt1711_set_cc(tcpc_dev, TYPEC_CC_OPEN);

	rt1711_i2c_write8(tcpc_dev,
		RT1711H_REG_I2CRST_CTRL,
		RT1711H_REG_I2CRST_SET(true, 4));

	rt1711_i2c_write8(tcpc_dev,
		RT1711H_REG_INTRST_CTRL,
		RT1711H_REG_INTRST_SET(true, 0));
#else
	rt1711_i2c_write8(tcpc_dev, RT1711H_REG_SWRESET, 1);
#endif	/* CONFIG_TCPC_SHUTDOWN_CC_DETACH */

	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY
static int rt1711_set_msg_header(
	struct tcpc_device *tcpc, int power_role, int data_role)
{
	return rt1711_i2c_write8(tcpc, TCPC_V10_REG_MSG_HDR_INFO,
		TCPC_V10_REG_MSG_HDR_INFO_SET((u32)data_role, power_role));
}

static int rt1711_set_rx_enable(struct tcpc_device *tcpc, uint8_t enable)
{
	int ret = 0;
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	static bool last_change;
	bool change = false;
#endif

	if (enable)
		ret = rt1711h_set_clock_gating(tcpc, false);
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	if (support_smart_holder) {
		if (enable & TCPC_RX_CAP_CUST_SRC2) {
			change = true;
			rt1711_i2c_write16(tcpc, TCPC_V10_REG_ALERT_MASK, 0);
		}
		if (change ^ last_change) {
			if (change)
				rt1711_i2c_write16(tcpc,
					TCPC_V10_REG_ALERT_MASK, 0);
			else
				rt1711_init_alert_mask(tcpc);
			last_change = change;
		}
	}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	if (ret == 0) {
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
		if (support_smart_holder)
			ret = rt1711_i2c_write8(tcpc, TCPC_V10_REG_RX_DETECT,
				enable & ~TCPC_RX_CAP_CUST_SRC2);
		else
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
			ret = rt1711_i2c_write8(tcpc, TCPC_V10_REG_RX_DETECT,
				enable);
	}
	if ((ret == 0) && (!enable))
		ret = rt1711h_set_clock_gating(tcpc, true);

	return ret;
}

static int rt1711_get_message(struct tcpc_device *tcpc, uint32_t *payload,
			uint16_t *msg_head, enum tcpm_transmit_type *frame_type)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);
	int rv;
	uint8_t type, cnt = 0;
	uint8_t buf[8] = { 0 }; /* 8:sizeof payload */
	const uint16_t alert_rx =
		TCPC_V10_REG_ALERT_RX_STATUS|TCPC_V10_REG_RX_OVERFLOW;

	rv = rt1711_block_read(chip->client,
			TCPC_V10_REG_RX_BYTE_CNT, 8, buf);
	cnt = buf[0];
	type = buf[1];
	*msg_head = *(uint16_t *)&buf[2];
	payload[0] = *(uint32_t *)&buf[4];

	/* TCPC 1.0 ==> no need to subtract the size of msg_head */
	if (rv >= 0 && cnt > 7) {
		cnt -= 7; /* MSG_HDR */
		rv = rt1711_block_read(chip->client, TCPC_V10_REG_RX_DATA+4, cnt,
				(uint8_t *) (payload+1));
	}

	*frame_type = (enum tcpm_transmit_type) type;

	/* Read complete, clear RX status alert bit */
	tcpci_alert_status_clear(tcpc, alert_rx);

	return rv;
}

static int rt1711_set_bist_carrier_mode(
	struct tcpc_device *tcpc, uint8_t pattern)
{
	/* Don't support this function */
	return 0;
}

/* total length(1byte) + message header (2byte) + data object (7*4) */
#define RT1711_TRANSMIT_MAX_SIZE	(1 + sizeof(uint16_t) + sizeof(uint32_t)*7)

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
static int rt1711_retransmit(struct tcpc_device *tcpc)
{
	return rt1711_i2c_write8(tcpc, TCPC_V10_REG_TRANSMIT,
			TCPC_V10_REG_TRANSMIT_SET(TCPC_TX_SOP));
}
#endif

static int rt1711_transmit(struct tcpc_device *tcpc,
	enum tcpm_transmit_type type, uint16_t header, const uint32_t *data)
{
	struct rt1711_chip *chip = tcpc_get_dev_data(tcpc);
	int rv;
	int data_cnt, packet_cnt;
	uint8_t temp[RT1711_TRANSMIT_MAX_SIZE];

	if (type < TCPC_TX_HARD_RESET) {
		data_cnt = sizeof(uint32_t) * PD_HEADER_CNT(header);
		packet_cnt = data_cnt + sizeof(uint16_t);

		temp[0] = packet_cnt;
		memcpy(temp+1, (uint8_t *)&header, 2);
		if (data_cnt > 0)
			memcpy(temp+3, (uint8_t *)data, data_cnt);

		rv = rt1711_block_write(chip->client,
				TCPC_V10_REG_TX_BYTE_CNT,
				packet_cnt+1, (uint8_t *)temp);
		if (rv < 0)
			return rv;
	}

	rv = rt1711_i2c_write8(tcpc, TCPC_V10_REG_TRANSMIT,
		TCPC_V10_REG_TRANSMIT_SET((u32)type));
	return rv;
}

static int rt1711_set_bist_test_mode(struct tcpc_device *tcpc, bool en)
{
	int data;

	data = rt1711_i2c_read8(tcpc, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE;
	data |= en ? TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE : 0;

	return rt1711_i2c_write8(tcpc, TCPC_V10_REG_TCPC_CTRL, data);
}
#endif /* CONFIG_USB_POWER_DELIVERY */

static struct tcpc_ops rt1711_tcpc_ops = {
	.init = rt1711_tcpc_init,
	.alert_status_clear = rt1711_alert_status_clear,
	.fault_status_clear = rt1711_fault_status_clear,
	.get_alert_status = rt1711_get_alert_status,
	.get_power_status = rt1711_get_power_status,
	.get_fault_status = rt1711_get_fault_status,
	.get_cc = rt1711_get_cc,
	.set_cc = rt1711_set_cc,
	.set_polarity = rt1711_set_polarity,
	.set_vconn = rt1711_set_vconn,
	.deinit = rt1711_tcpc_deinit,
	.is_reg_default = rt1711_is_reg_default,
	.mask_vsafe0v = rt1711_mask_vsafe0v,

#ifdef CONFIG_TCPC_LOW_POWER_MODE
	.is_low_power_mode = rt1711_is_low_power_mode,
	.set_low_power_mode = rt1711_set_low_power_mode,
#endif	/* CONFIG_TCPC_LOW_POWER_MODE */

#ifdef CONFIG_TCPC_WATCHDOG_EN
	.set_watchdog = rt1711h_set_watchdog,
#endif	/* CONFIG_TCPC_WATCHDOG_EN */

#ifdef CONFIG_TCPC_INTRST_EN
	.set_intrst = rt1711h_set_intrst,
#endif	/* CONFIG_TCPC_INTRST_EN */

#ifdef CONFIG_USB_POWER_DELIVERY
	.set_msg_header = rt1711_set_msg_header,
	.set_rx_enable = rt1711_set_rx_enable,
	.get_message = rt1711_get_message,
	.transmit = rt1711_transmit,
	.set_bist_test_mode = rt1711_set_bist_test_mode,
	.set_bist_carrier_mode = rt1711_set_bist_carrier_mode,
#endif	/* CONFIG_USB_POWER_DELIVERY */

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	.retransmit = rt1711_retransmit,
#endif	/* CONFIG_USB_PD_RETRY_CRC_DISCARD */
};

static int rt_parse_dt(struct rt1711_chip *chip, struct device *dev)
{
	struct device_node *np = dev->of_node;

	if (!np)
		return -EINVAL;

	pr_info("%s\n", __func__);
	chip->irq_gpio = of_get_named_gpio(np, "rt1711,irq_pin", 0);
	return 0;
}

/*
 * In some platform pr_info may spend too much time on printing debug message.
 * So we use this function to test the printk performance.
 * If your platform cannot not pass this check function, please config
 * PD_DBG_INFO, this will provide the threaded debug message for you.
 */
#if TCPC_ENABLE_ANYMSG
static void check_printk_performance(void)
{
	int i;
	u64 t1, t2;
	u32 nsrem;

#ifdef CONFIG_PD_DBG_INFO
	for (i = 0; i < 10; i++) {
		t1 = local_clock();
		pd_dbg_info("%d\n", i);
		t2 = local_clock();
		t2 -= t1;
		nsrem = do_div(t2, 1000000000);
		pd_dbg_info("pd_dbg_info : t2-t1 = %lu\n",
				(unsigned long)nsrem / 1000);
	}
	for (i = 0; i < 10; i++) {
		t1 = local_clock();
		pr_info("%d\n", i);
		t2 = local_clock();
		t2 -= t1;
		nsrem = do_div(t2, 1000000000);
		pr_info("pr_info : t2-t1 = %lu\n",
				(unsigned long)nsrem / 1000);
	}
#else
	for (i = 0; i < 10; i++) {
		t1 = local_clock();
		pr_info("%d\n", i);
		t2 = local_clock();
		t2 -= t1;
		nsrem = do_div(t2, 1000000000);
		pr_info("t2-t1 = %lu\n",
				(unsigned long)nsrem /  1000);
	}
#endif /* CONFIG_PD_DBG_INFO */
}
#endif /* TCPC_ENABLE_ANYMSG */

static int rt1711_tcpcdev_init(struct rt1711_chip *chip, struct device *dev)
{
	struct tcpc_desc *desc = NULL;
	struct device_node *np = dev->of_node;
	u32 val = 0;
	u32 len;
	const char *name = "default";

	desc = devm_kzalloc(dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;
	if (of_property_read_u32(np, "rt-tcpc,role_def", &val) >= 0) {
		if (val >= TYPEC_ROLE_NR)
			desc->role_def = TYPEC_ROLE_DRP;
		else
			desc->role_def = val;
	} else {
		dev_info(dev, "use default Role DRP\n");
		desc->role_def = TYPEC_ROLE_DRP;
	}

	if (of_property_read_u32(np, "rt-tcpc,notifier_supply_num",
		&val) >= 0) {
		if ((int)val < 0)
			desc->notifier_supply_num = 0;
		else
			desc->notifier_supply_num = val;
	} else {
		desc->notifier_supply_num = 0;
	}

	if (of_property_read_u32(np, "rt-tcpc,vcon_ocp_enable", &val) >= 0) {
		if ((int)val < 0)
			desc->vcon_ocp_enable = 0;
		else
			desc->vcon_ocp_enable = val;
	} else {
		desc->vcon_ocp_enable = 0;
	}

	if (of_property_read_u32(np, "rt-tcpc,rp_level", &val) >= 0) {
		switch (val) {
		case 0: /* RP Default */
			desc->rp_lvl = TYPEC_CC_RP_DFT;
			break;
		case 1: /* RP 1.5V */
			desc->rp_lvl = TYPEC_CC_RP_1_5;
			break;
		case 2: /* RP 3.0V */
			desc->rp_lvl = TYPEC_CC_RP_3_0;
			break;
		default:
			break;
		}
	}

	of_property_read_u32(np, "rt-tcpc,pd_remove_cc_open",
		&chip->pd_remove_cc_open);
	of_property_read_string(np, "rt-tcpc,name", (char const **)&name);

	len = strlen(name);
	desc->name = kzalloc(len + 1, GFP_KERNEL);
	if (!desc->name) {
		pr_err("rt1711_tcpcdev_init desc->name kzalloc fail\n");
		return -ENOMEM;
	}

	strcpy((char *)desc->name, name);
	chip->tcpc_desc = desc;
	chip->tcpc = tcpc_device_register(dev, desc, &rt1711_tcpc_ops, chip);
	if (IS_ERR(chip->tcpc))
		return -EINVAL;

	if (chip->chip_id <= RT1711H_DID_B)
		chip->tcpc->tcpc_flags = TCPC_FLAGS_LPM_WAKEUP_WATCHDOG;
	else
		chip->tcpc->tcpc_flags = TCPC_FLAGS_CHECK_RA_DETACHE;
	return 0;
}

static int rt1711h_check_revision(struct i2c_client *client)
{
	u16 vid, pid, did;
	u32 chip_revision;
	int ret;
	u8 data = 1;

	ret = rt1711_read_device(client, TCPC_V10_REG_VID, 2, &vid);
	if (ret < 0) {
		dev_err(&client->dev, "read chip ID fail\n");
		return -EIO;
	}

	ret = rt1711_read_device(client, TCPC_V10_REG_PID, 2, &pid);
	if (ret < 0) {
		dev_err(&client->dev, "read product ID fail\n");
		return -EIO;
	}

	if (vid == RICHTEK_1711_VID) {
		if (pid != RICHTEK_1711_PID) {
			pr_info("%s failed, VID=0x%04x PID=0x%04x\n", __func__, vid, pid);
			return -ENODEV;
		}
	} else if (vid == ETEK_ET7303_VID) {
		if (pid != ETEK_ET7303_PID) {
			pr_info("%s failed, VID=0x%04x PID=0x%04x\n", __func__, vid, pid);
			return -ENODEV;
		}
	} else {
		pr_info("%s failed, VID=0x%04x\n", __func__, vid);
		return -ENODEV;
	}

	ret = rt1711_write_device(client, RT1711H_REG_SWRESET, 1, &data);
	if (ret < 0)
		return ret;

	mdelay(1);

	ret = rt1711_read_device(client, TCPC_V10_REG_DID, 2, &did);
	if (ret < 0) {
		dev_err(&client->dev, "read device ID fail\n");
		return -EIO;
	}

	/* vid[31:16], did[15:0] */
	chip_revision = ((u32)vid << SHIFT_16) | did;

	return chip_revision;
}
static int is_cable_for_direct_charge(void)
{
	int val;
	int low;
	struct rt1711_chip *chip = g_chip_for_reg_read;

	val = rt1711_reg_read(chip->client, RT1711_CC_STATUS);
	pr_info("%s:cc_check_REG0x1D = 0x%x\n", __func__, val);
	low = val & CC_STATUS_MASK;
	pr_info("%s:cc_check_status = 0x%x\n", __func__, low);
	if (low != RT_CC_STATUS_FOR_DOUBLE_56K)
		return -1;
	pr_info("%s:cc_check succ\n", __func__);
	return 0;
}
static struct cc_check_ops cc_check_ops = {
	.is_cable_for_direct_charge = is_cable_for_direct_charge,
};

static int rt1711_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct rt1711_chip *chip = NULL;
	struct power_devices_info_data *power_dev_info = NULL;
	int ret = 0;
	int chip_revision;
	int need_not_config_extra_pmic = 0;
	bool use_dt = client->dev.of_node;

	hwlog_err("probe\n");
	if (of_property_read_u32(of_find_compatible_node(NULL, NULL,
		"huawei,pd_dpm"), "need_not_config_extra_pmic",
		&need_not_config_extra_pmic))
		pr_err("get need_not_config_extra_pmic fail\n");
	pr_info("need_not_config_extra_pmic = %d\n",
		need_not_config_extra_pmic);
	if (!need_not_config_extra_pmic) {
#ifdef CONFIG_USE_CAMERA3_ARCH
		hw_extern_pmic_config(RT_PMIC_LDO_3, RTUSB_VIN_3V3, 1);
#endif /* CONFIG_USE_CAMERA3_ARCH */
		pr_info("%s:PD PMIC ENABLE IS CALLED\n", __func__);
	}
	pr_info("%s\n", __func__);
	if (i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_I2C_BLOCK | I2C_FUNC_SMBUS_BYTE_DATA))
		pr_info("I2C functionality : OK\n");
	else
		pr_info("I2C functionality check : failure\n");

	chip_revision = rt1711h_check_revision(client);
	if (chip_revision < 0)
		return ret;
#if TCPC_ENABLE_ANYMSG
	check_printk_performance();
#endif /* TCPC_ENABLE_ANYMSG */

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	g_chip_for_reg_read = chip;
	if (use_dt) {
		rt_parse_dt(chip, &client->dev);
	} else {
		dev_err(&client->dev, "no dts node\n");
		return -ENODEV;
	}
	chip->dev = &client->dev;
	chip->client = client;
	sema_init(&chip->io_lock, 1);
	sema_init(&chip->suspend_lock, 1);
	chip->rt1711h_wakelock = wakeup_source_register(chip->dev, "rt1711h_wakelock");
	if (!chip->rt1711h_wakelock) {
		hwlog_err("%s wakeup source register failed\n", __func__);
		return -EINVAL;
	}

	i2c_set_clientdata(client, chip);
	INIT_DELAYED_WORK(&chip->poll_work, rt1711_poll_work);

	chip->chip_id = chip_revision & DID_MASK;
	chip->vendor_id = (chip_revision >> SHIFT_16) & VID_MASK;
	pr_info("rt1711h_chipID = 0x%0x, vendor id = 0x%0x\n",
		chip->chip_id, chip->vendor_id);

	ret = rt1711_regmap_init(chip);
	if (ret < 0) {
		wakeup_source_unregister(chip->rt1711h_wakelock);
		dev_err(chip->dev, "rt1711 regmap init fail\n");
		return -EINVAL;
	}

	ret = rt1711_tcpcdev_init(chip, &client->dev);
	if (ret < 0) {
		dev_err(&client->dev, "rt1711 tcpc dev init fail\n");
		goto err_tcpc_reg;
	}

#ifdef CONFIG_POGO_PIN
	pogopin_cc_register_ops(&rt1711h_cc_detect_ops);
#endif /* CONFIG_POGO_PIN */
	ret = rt1711_init_alert(chip->tcpc);
	if (ret < 0) {
		pr_err("rt1711 init alert fail\n");
		goto err_irq_init;
	}

	tcpc_schedule_init_work(chip->tcpc);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TYPEC);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */
	ret = cc_check_ops_register(&cc_check_ops);
	if (ret) {
		hwlog_err("cc_check_ops register failed\n");
		goto err_irq_init;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = chip->dev->driver->name;
		power_dev_info->dev_id = chip->chip_id;
		power_dev_info->ver_id = 0;
	}

	pr_info("%s cc_check register OK\n", __func__);
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	pd_dpm_cable_vdo_ops_register(&rt1711h_cable_vdo_ops);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	pr_info("%s probe OK\n", __func__);
	return 0;

err_irq_init:
	tcpc_device_unregister(chip->dev, chip->tcpc);
err_tcpc_reg:
	rt1711_regmap_deinit(chip);
	wakeup_source_unregister(chip->rt1711h_wakelock);
	return ret;
}

static int rt1711_i2c_remove(struct i2c_client *client)
{
	struct rt1711_chip *chip = i2c_get_clientdata(client);

	if (chip) {
		cancel_delayed_work_sync(&chip->poll_work);
		wakeup_source_unregister(chip->rt1711h_wakelock);

		tcpc_device_unregister(chip->dev, chip->tcpc);
		rt1711_regmap_deinit(chip);
	}

	return 0;
}

#ifdef CONFIG_PM
static int rt1711_i2c_suspend(struct device *dev)
{
	struct rt1711_chip *chip = NULL;
	struct i2c_client *client = to_i2c_client(dev);

	if (client) {
		chip = i2c_get_clientdata(client);
		if (chip)
			down(&chip->suspend_lock);
	}

	return 0;
}

static int rt1711_i2c_resume(struct device *dev)
{
	struct rt1711_chip *chip = NULL;
	struct i2c_client *client = to_i2c_client(dev);

	if (client) {
		chip = i2c_get_clientdata(client);
		if (chip)
			up(&chip->suspend_lock);
	}

	return 0;
}

static void rt1711_shutdown(struct i2c_client *client)
{
	struct rt1711_chip *chip = i2c_get_clientdata(client);
	struct tcpc_device *tcpc = NULL;

	/* Please reset IC here */
	if (chip != NULL) {
		tcpc = chip->tcpc;
		if (chip->irq)
			disable_irq(chip->irq);
		tcpm_shutdown(tcpc);
	} else {
		i2c_smbus_write_byte_data(
			client, RT1711H_REG_SWRESET, 0x01);
	}
}

static int rt1711_pm_suspend_runtime(struct device *device)
{
	dev_dbg(device, "pm_runtime: suspending...\n");
	return 0;
}

static int rt1711_pm_resume_runtime(struct device *device)
{
	dev_dbg(device, "pm_runtime: resuming...\n");
	return 0;
}


static const struct dev_pm_ops rt1711_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(
			rt1711_i2c_suspend,
			rt1711_i2c_resume)
	SET_RUNTIME_PM_OPS(
		rt1711_pm_suspend_runtime,
		rt1711_pm_resume_runtime,
		NULL
	)
};
#define RT1711_PM_OPS	(&rt1711_pm_ops)
#else
#define RT1711_PM_OPS	(NULL)
#endif /* CONFIG_PM */

static const struct i2c_device_id rt1711_id_table[] = {
	{"rt1711", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, rt1711_id_table);

static const struct of_device_id rt_match_table[] = {
	{.compatible = "richtek,rt1711",},
	{},
};

static struct i2c_driver rt1711_driver = {
	.driver = {
		.name = "rt1711h",
		.owner = THIS_MODULE,
		.of_match_table = rt_match_table,
		.pm = RT1711_PM_OPS,
	},
	.probe = rt1711_i2c_probe,
	.remove = rt1711_i2c_remove,
#ifdef CONFIG_PM
	.shutdown = rt1711_shutdown,
#endif
	.id_table = rt1711_id_table,
};

static int __init rt1711_init(void)
{
	struct device_node *np = NULL;

	pr_info("rt1711h_init (%s): initializing...\n", RT1711H_DRV_VERSION);
	np = of_find_node_by_name(NULL, "rt1711");
	if (np != NULL)
		pr_info("rt1711h node found...\n");
	else
		pr_info("rt1711h node not found...\n");

	return i2c_add_driver(&rt1711_driver);
}
late_initcall(rt1711_init);

static void __exit rt1711_exit(void)
{
	i2c_del_driver(&rt1711_driver);
}
module_exit(rt1711_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeff Chang <jeff_chang@richtek.com>");
MODULE_DESCRIPTION("RT1711 TCPC Driver");
MODULE_VERSION(RT1711H_DRV_VERSION);
