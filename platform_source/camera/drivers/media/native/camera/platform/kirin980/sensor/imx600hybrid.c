/*
 * imx600hybrid.c
 *
 * camera driver source file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <securec.h>
#include "hwsensor.h"
#include "sensor_commom.h"
#include "../pmic/hw_pmic.h"

#ifdef CONFIG_KERNEL_CAMERA_FPGA_ICE40
#include "ice40/ice40_spi.h"
#endif

#define intf_to_sensor(i) container_of(i, sensor_t, intf)
#define sensor_to_pdev(s) container_of((s).dev, struct platform_device, dev)
#define CTL_RESET_HOLD     0
#define CTL_RESET_RELEASE  1

struct mutex g_imx600hybrid_power_lock;
static bool g_power_on_status; /* false: power off, true:power on */
static struct sensor_power_setting g_imx600hybrid_power_setting[] = {
	/* M0 OIS DRV 2.85 [ldo22] neo pvdd ldo22, vout22 1.1v */
	{
		.seq_type = SENSOR_OIS_DRV,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* AFVDD GPIO 004 ENABLE
	 * Power up vcm first, then power up AVDD1
	 * XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
	 */
	{
		.seq_type = SENSOR_VCM_PWDN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0+M1 IOVDD 1.8V [ld021] */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0 AVDD0 2.8v [ldo33] */
	{
		.seq_type = SENSOR_AVDD0,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 AVDD 2.8v [ldo13] */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 AVDD1 gpio 008 VBUCK3(2v55) -> 1V8 */
	{
		.seq_type = SENSOR_AVDD1_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0 DVDD0 [ldo19] 1v2 */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P13V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 DVDD1 [ldo20] 1v1 */
	{
		.seq_type = SENSOR_DVDD2,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 2,
		.delay = 1,
	},
	/* M0 RESET  [GPIO_013] */
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* M1 RESET  [GPIO_136] */
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static struct sensor_power_setting g_imx600hybrid_power_setting_fpga[] = {
	{
		.seq_type = SENSOR_PMIC,
		.seq_val  = VOUT_BUCK_2,
		.config_val = PMIC_2P85V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0+M1 IOVDD 1.8V [ld021] */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_5,
		.config_val = PMIC_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0 AVDD0 2.8v [ldo33] */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_1,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 AVDD 2.8v [ldo13] */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_2,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/*  M0 DVDD0 [ldo19] 1v2 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val  = VOUT_BUCK_1,
		.config_val = PMIC_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 DVDD1 [ldo20] 1v1 */
	{
		.seq_type = SENSOR_LDO_EN,
		.config_val = SENSOR_GPIO_HIGH,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 2,
		.delay = 1,
	},
	/* M0 RESET  [GPIO_013] */
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* M1 RESET  [GPIO_136] */
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static struct sensor_power_setting g_imx600hybrid_power_setting_clt[] = {
	/* pvdd gpio144 1V1 */
	{
		.seq_type = SENSOR_DVDD0_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* AFVDD GPIO 004 ENABLE
	 * Power up vcm first, then power up AVDD1
	 * XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
	 */
	{
		.seq_type = SENSOR_VCM_PWDN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0+M1 IOVDD 1.8V [ld021] */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0 AVDD0 2.8v [ldo33] */
	{
		.seq_type = SENSOR_AVDD0,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 AVDD 2.8v [ldo13] */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 AVDD1 gpio 008 VBUCK3(2v55) -> 1V8 */
	{
		.seq_type = SENSOR_AVDD1_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0 DVDD0 [ldo19] 1v2 */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P13V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M1 DVDD1 [ldo20] 1v1 */
	{
		.seq_type = SENSOR_DVDD2,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 2,
		.delay = 1,
	},
	/* M0 RESET  [GPIO_013] */
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* M1 RESET  [GPIO_136] */
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static struct sensor_power_setting g_imx600hybrid_power_setting_laya[] = {
	/* AFVDD GPIO 004 ENABLE
	 * Power up vcm first, then power up AVDD1
	 * XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8
	 */
	{
		.seq_type = SENSOR_RXDPHY_CLK,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VCM_PWDN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0+M1 IOVDD 1.8V [ld021] */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
	/* laser gpio xshut.
	 * configed to avoid i2c conflict with pmic.
	 */
		.seq_type = SENSOR_LASER_XSHUT,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* MCAM_AVDD0_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_1,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 10, /* 10ms */
	},
	/* MCAM_AVDD1_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_2,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_AVDD_1V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_3,
		.config_val = PMIC_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_DRVVDD_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_4,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_DVDD_1V1 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val  = VOUT_BUCK_1,
		.config_val = PMIC_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 2,
		.delay = 1,
	},
	/* M0 RESET  [GPIO_007] */
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* M1 RESET  [GPIO_012] */
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static struct sensor_power_setting g_imx600hybrid_power_setting_laya_pie[] = {
	/* AFVDD GPIO 004 ENABLE */
	/* Power up vcm first, then power up AVDD1 */
	/* XBUCK_AFVDD 2V55---| bucker |---> AVDD1 1V8 */
	{
		.seq_type = SENSOR_PMIC2,
		.seq_val = VOUT_BUCK_1,
		.config_val = PMIC_2P85V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* M0+M1 IOVDD 1.8V [ld021] */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
	/* laser gpio xshut.
	 * configed to avoid i2c conflict with pmic
	 */
		.seq_type = SENSOR_LASER_XSHUT,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* MCAM_AVDD0_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_1,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 10, /* 10ms */
	},
	/* MCAM_AVDD1_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_2,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_AVDD_1V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_3,
		.config_val = PMIC_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_DRVVDD_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_4,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_DVDD_1V1 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val  = VOUT_BUCK_1,
		.config_val = PMIC_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = 2,
		.delay = 1,
	},
	/* M0 RESET  [GPIO_007] */
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* M1 RESET  [GPIO_012] */
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static char const *imx600hybrid_get_name(hwsensor_intf_t *si)
{
	sensor_t *sensor = intf_to_sensor(si);

	return sensor->board_info->name;
}

static int imx600hybrid_power_up(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	sensor = intf_to_sensor(si);
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index,
		sensor->board_info->name);
#ifdef CONFIG_KERNEL_CAMERA_FPGA_ICE40
	fpga_enable_gpio_power_up(1);
#endif
	if (hw_is_fpga_board())
		ret = do_sensor_power_on(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_up(sensor);

	if (ret == 0)
		cam_info("%s. power up sensor success", __func__);
	else
		cam_err("%s. power up sensor fail", __func__);

	return ret;
}

static int imx600hybrid_power_down(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	sensor = intf_to_sensor(si);
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index,
		sensor->board_info->name);
	if (hw_is_fpga_board())
		ret = do_sensor_power_off(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_down(sensor);
#ifdef CONFIG_KERNEL_CAMERA_FPGA_ICE40
	fpga_enable_gpio_power_up(0);
#endif
	if (ret == 0)
		cam_info("%s. power down sensor success", __func__);
	else
		cam_err("%s. power down sensor fail", __func__);

	return ret;
}

static int imx600hybrid_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int imx600hybrid_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int imx600hybrid_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = intf_to_sensor(si);
	struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
	int ret;

	cam_info("%s name:%s", __func__, sensor->board_info->name);

	ret = strncpy_s(cdata->cfg.name,
		DEVICE_NAME_SIZE - 1,
		sensor->board_info->name,
		strlen(sensor->board_info->name) + 1);
	if (ret != 0) {
		cam_err("%s. strncpy failed", __func__);
		return -EINVAL;
	}
	cdata->data = sensor->board_info->sensor_index;

	return 0;
}

enum camera_metadata_enum_android_hw_dual_primary_mode {
	ANDROID_HW_DUAL_PRIMARY_FIRST  = 0,
	ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
	ANDROID_HW_DUAL_PRIMARY_BOTH   = 3,
};


static int imx600hybrid_do_hw_reset(hwsensor_intf_t *si, int ctl, int id)
{
	sensor_t *sensor = intf_to_sensor(si);
	hwsensor_board_info_t *b_info;
	int ret;

	b_info = sensor->board_info;
	if (!b_info) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}
	ret  = gpio_request(b_info->gpios[RESETB].gpio, "imx318reset-0");
	ret += gpio_request(b_info->gpios[RESETB2].gpio, "imx318reset-1");
	if (ret) {
		cam_err("%s requeset reset pin failed", __func__);
		return ret;
	}
	if (ctl == CTL_RESET_HOLD) {
		ret += gpio_direction_output(b_info->gpios[RESETB].gpio,
			CTL_RESET_HOLD);
		ret += gpio_direction_output(b_info->gpios[RESETB2].gpio,
			CTL_RESET_HOLD);
		cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_HOLD");
		udelay(2000); /* delay 2000us */
	} else if (ctl == CTL_RESET_RELEASE) {
		if (id == ANDROID_HW_DUAL_PRIMARY_FIRST) {
			ret += gpio_direction_output(
				b_info->gpios[RESETB].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(
				b_info->gpios[RESETB2].gpio,
				CTL_RESET_HOLD);
			cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_HOLD");
		} else if (id == ANDROID_HW_DUAL_PRIMARY_BOTH) {
			ret += gpio_direction_output(
				b_info->gpios[RESETB].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(
				b_info->gpios[RESETB2].gpio,
				CTL_RESET_RELEASE);
			cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_RELEASE");
		} else if (id == ANDROID_HW_DUAL_PRIMARY_SECOND) {
			ret += gpio_direction_output(
				b_info->gpios[RESETB2].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(
				b_info->gpios[RESETB].gpio,
				CTL_RESET_HOLD);
			cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_RELEASE");
		}
	}
	gpio_free(b_info->gpios[RESETB].gpio);
	gpio_free(b_info->gpios[RESETB2].gpio);
	if (ret)
		cam_err("%s set reset pin failed", __func__);
	else
		cam_info("%s: set reset state=%d, mode=%d", __func__, ctl, id);

	return ret;
}

static int imx600hybrid_config(hwsensor_intf_t *si, void  *argp)
{
	struct sensor_cfg_data *data = NULL;
	int ret = 0;

	if (!si || !argp) {
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_debug("imx600hybrid cfgtype = %d", data->cfgtype);
	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		mutex_lock(&g_imx600hybrid_power_lock);
		if (g_power_on_status == false) {
			ret = si->vtbl->power_up(si);
			if (ret == 0)
				g_power_on_status = true;
			else
				cam_err("%s. power up fail", __func__);
		}
		mutex_unlock(&g_imx600hybrid_power_lock);
		break;
	case SEN_CONFIG_POWER_OFF:
		mutex_lock(&g_imx600hybrid_power_lock);
		if (g_power_on_status == true) {
			ret = si->vtbl->power_down(si);
			if (ret != 0)
				cam_err("%s. power_down fail", __func__);
			g_power_on_status = false;
		}
		mutex_unlock(&g_imx600hybrid_power_lock);
		break;
	case SEN_CONFIG_WRITE_REG:
		break;
	case SEN_CONFIG_READ_REG:
		break;
	case SEN_CONFIG_WRITE_REG_SETTINGS:
		break;
	case SEN_CONFIG_READ_REG_SETTINGS:
		break;
	case SEN_CONFIG_ENABLE_CSI:
		break;
	case SEN_CONFIG_DISABLE_CSI:
		break;
	case SEN_CONFIG_MATCH_ID:
		ret = si->vtbl->match_id(si, argp);
		break;
	case SEN_CONFIG_RESET_HOLD:
		ret = imx600hybrid_do_hw_reset(si,
			CTL_RESET_HOLD, data->mode);
	break;
	case SEN_CONFIG_RESET_RELEASE:
		ret = imx600hybrid_do_hw_reset(si,
			CTL_RESET_RELEASE, data->mode);
	break;
	default:
		cam_err("%s cfgtype %d is error", __func__, data->cfgtype);
		break;
	}
	return ret;
}

static hwsensor_vtbl_t g_imx600hybrid_vtbl = {
	.get_name = imx600hybrid_get_name,
	.config = imx600hybrid_config,
	.power_up = imx600hybrid_power_up,
	.power_down = imx600hybrid_power_down,
	.match_id = imx600hybrid_match_id,
	.csi_enable = imx600hybrid_csi_enable,
	.csi_disable = imx600hybrid_csi_disable,
};

/* individual driver data for each device */
static sensor_t g_imx600hybrid = {
	.intf = { .vtbl = &g_imx600hybrid_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx600hybrid_power_setting),
		.power_setting = g_imx600hybrid_power_setting,
	},
};

static sensor_t g_imx600hybrid_fpga = {
	.intf = { .vtbl = &g_imx600hybrid_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx600hybrid_power_setting_fpga),
		.power_setting = g_imx600hybrid_power_setting_fpga,
	},
};

static sensor_t g_imx600hybrid_clt = {
	.intf = { .vtbl = &g_imx600hybrid_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx600hybrid_power_setting_clt),
		.power_setting = g_imx600hybrid_power_setting_clt,
	},
};

static sensor_t g_imx600hybrid_laya = {
	.intf = { .vtbl = &g_imx600hybrid_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx600hybrid_power_setting_laya),
		.power_setting = g_imx600hybrid_power_setting_laya,
	},
};

static sensor_t g_imx600hybrid_laya_pie = {
	.intf = { .vtbl = &g_imx600hybrid_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx600hybrid_power_setting_laya_pie),
		.power_setting = g_imx600hybrid_power_setting_laya_pie,
	},
};

/* support both imx600hybrid & imx318legacydual */
static const struct of_device_id g_imx600hybrid_dt_match[] = {
	{
		.compatible = "huawei,imx600hybrid",
		.data = &g_imx600hybrid.intf,
	},
	{
		.compatible = "huawei,imx600hybrid_fpga",
		.data = &g_imx600hybrid_fpga.intf,
	},
	{
		.compatible = "huawei,imx600hybrid_clt",
		.data = &g_imx600hybrid_clt.intf,
	},
	{
		.compatible = "huawei,imx600hybrid_laya",
		.data = &g_imx600hybrid_laya.intf,
	},
	{
		.compatible = "huawei,imx600hybrid_laya_pie",
		.data = &g_imx600hybrid_laya_pie.intf,
	},

	{ } /* terminate list */
};
MODULE_DEVICE_TABLE(of, g_imx600hybrid_dt_match);

/* platform driver struct */
static int32_t imx600hybrid_platform_probe(struct platform_device *pdev);
static int32_t imx600hybrid_platform_remove(struct platform_device *pdev);
static struct platform_driver g_imx600hybrid_driver = {
	.probe = imx600hybrid_platform_probe,
	.remove = imx600hybrid_platform_remove,
	.driver = {
		.name = "huawei,imx600hybrid",
		.owner = THIS_MODULE,
		.of_match_table = g_imx600hybrid_dt_match,
	},
};

static int32_t imx600hybrid_platform_probe(struct platform_device *pdev)
{
	int rc;
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *id = NULL;
	hwsensor_intf_t *intf = NULL;
	sensor_t *sensor = NULL;

	cam_info("enter %s gal", __func__);

	if (!np) {
		cam_err("%s of_node is NULL", __func__);
		return -ENODEV;
	}

	id = of_match_node(g_imx600hybrid_dt_match, np);
	if (!id) {
		cam_err("%s none id matched", __func__);
		return -ENODEV;
	}

	intf = (hwsensor_intf_t *)id->data;
	sensor = intf_to_sensor(intf);
	rc = hw_sensor_get_dt_data(pdev, sensor);
	if (rc < 0) {
		cam_err("%s no dt data", __func__);
		return -ENODEV;
	}
	sensor->dev = &pdev->dev;
	mutex_init(&g_imx600hybrid_power_lock);
	rc = hwsensor_register(pdev, intf);
	rc = rpmsg_sensor_register(pdev, (void *)sensor);

	return rc;
}

static int32_t imx600hybrid_platform_remove(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *id = NULL;
	hwsensor_intf_t *intf = NULL;
	sensor_t *sensor = NULL;

	cam_info("enter %s", __func__);

	if (!np) {
		cam_info("%s of_node is NULL", __func__);
		return 0;
	}
	/* don't use dev->p->driver_data
	 * we need to search again
	 */
	id = of_match_node(g_imx600hybrid_dt_match, np);
	if (!id) {
		cam_info("%s none id matched", __func__);
		return 0;
	}

	intf = (hwsensor_intf_t *)id->data;
	sensor = intf_to_sensor(intf);

	rpmsg_sensor_unregister((void *)&sensor);
	hwsensor_unregister(sensor_to_pdev(*sensor));
	return 0;
}
static int __init imx600hybrid_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_probe(&g_imx600hybrid_driver,
			imx600hybrid_platform_probe);
}

static void __exit imx600hybrid_exit_module(void)
{
	platform_driver_unregister(&g_imx600hybrid_driver);
}

module_init(imx600hybrid_init_module);
module_exit(imx600hybrid_exit_module);
MODULE_DESCRIPTION("imx600hybrid");
MODULE_LICENSE("GPL v2");
