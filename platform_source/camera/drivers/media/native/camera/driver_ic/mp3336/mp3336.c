/*
 * mp3336.c
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

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/videodev2.h>
#include <platform_include/camera/native/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "cam_intf.h"
#include "../hwdriver_ic.h"
#include "cam_log.h"

#define INVALID_GPIO         999
#define INVALID_I2C_INDEX   (-1)
#define INVALID_IC_POSITION (-1)
#define MP3336_GPIO_ENABLE   1
#define MP3336_GPIO_DISABLE  0

#define DRIVER_IC_ENABLE_PIN  10
#define BUCKBOOST_ENABLE_PIN  8

#define intf_to_driveric(i) container_of(i, struct driveric_t, intf)

enum drv_ic_mp3336_pin_type_t {
	POWER_ENABLE = 0,
	RESET = 1,
	MP3336_MAX_GPIOPIN,
};

enum drv_ic_mp3336_pin_status_t {
	FREED = 0,
	REQUESTED,
};

enum powertype_t {
	DISABLE = 0,
	ENABLE = 1,
};

struct drv_ic_mp3336_pin_t {
	unsigned int pin_id;
	enum drv_ic_mp3336_pin_status_t pin_status;
};

struct drv_ic_mp3336_private_data_t {
	struct drv_ic_mp3336_pin_t pin[MP3336_MAX_GPIOPIN];
};

static struct driveric_t g_mp3336;
static struct platform_device *g_pdev;
static struct drv_ic_mp3336_private_data_t  g_mp3336_pdata;

static int mp3336_get_dt_data(const struct hwdriveric_intf_t *intf,
	struct device_node *dev_node)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_mp3336_private_data_t  *pdata = NULL;
	unsigned int pin_tmp[MP3336_MAX_GPIOPIN] = {0};
	int rc;
	int i;
	unsigned int position, i2c_index;

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}
	if (!dev_node) {
		cam_err("%s invalid params dev_node", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}

	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	pdata = (struct drv_ic_mp3336_private_data_t  *)drv_ic->pdata;

	rc = of_property_read_u32(dev_node, "vendor,position",
		(u32 *)&position);
	if (rc < 0) {
		cam_err("%s get dt position failed", __func__);
		drv_ic->position = INVALID_IC_POSITION;
		goto get_dt_error_handler;
	} else {
		drv_ic->position = position;
		cam_info("%s position(%u)", __func__, drv_ic->position);
	}

	rc = of_property_read_u32(dev_node, "vendor,i2c_index",
		(u32 *)&i2c_index);
	if (rc < 0) {
		cam_err("%s get dt i2c_index failed", __func__);
		drv_ic->i2c_index = INVALID_I2C_INDEX;
		goto get_dt_error_handler;
	} else {
		drv_ic->i2c_index = i2c_index;
		cam_info("%s i2c_index(%u)", __func__, drv_ic->i2c_index);
	}

	rc = of_property_read_u32_array(dev_node, "vendor,gpio_pin",
		pin_tmp, MP3336_MAX_GPIOPIN);
	if (rc < 0) {
		cam_err("%s get dt gpio-pin failed", __func__);
		for (i = 0; i < MP3336_MAX_GPIOPIN; i++) {
			pdata->pin[i].pin_id = INVALID_GPIO;
			pdata->pin[i].pin_status = FREED;
		}
		goto get_dt_error_handler;
	} else {
		for (i = 0; i < MP3336_MAX_GPIOPIN; i++) {
			pdata->pin[i].pin_id = pin_tmp[i];
			pdata->pin[i].pin_status = FREED;
			cam_info("%s gpio-pin[%d] = %u", __func__,
				i, pdata->pin[i].pin_id);
		}
	}

	return rc;

get_dt_error_handler:
	return rc;
}

static int drv_ic_mp3336_set_pin(
	struct drv_ic_mp3336_private_data_t  *pdata,
	int state)
{
	int rc;
	int i;

	if (!pdata) {
		cam_err("%s invalid params pdata", __func__);
		return -EINVAL;
	}

	for (i = 0; i < MP3336_MAX_GPIOPIN; i++) {
		cam_debug("%s pin_id=%d, state=%d", __func__,
			pdata->pin[i].pin_id, state);

		if (pdata->pin[i].pin_id == INVALID_GPIO) {
			cam_err("%s gpio pin is err rc=%d", __func__,
				pdata->pin[i].pin_id);
			return -1;
		}
		rc = gpio_request(pdata->pin[i].pin_id, NULL);
		if (rc < 0) {
			cam_err("%s, gpio-request %d", __func__,
				pdata->pin[i].pin_id);
			return -1;
		}
		pdata->pin[i].pin_status = REQUESTED;
		rc = gpio_direction_output(pdata->pin[i].pin_id, state);
		if (rc < 0)
			cam_err("%s gpio output is err rc=%d", __func__, rc);

		gpio_free(pdata->pin[i].pin_id);
	}

	return rc;
}

static int mp3336_init(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;

	cam_info("%s init enter", __func__);

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);

	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}

	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	cam_info("%s init success", __func__);
	return 0;
}

static int mp3336_power_on(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_mp3336_private_data_t  *pdata = NULL;
	int rc;

	cam_info("%s power on enter", __func__);
	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}

	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}
	pdata = (struct drv_ic_mp3336_private_data_t  *)drv_ic->pdata;

	rc = drv_ic_mp3336_set_pin(pdata, ENABLE);

	cam_info("%s power on end,rc=%d", __func__, rc);
	return rc;
}

static int mp3336_power_off(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_mp3336_private_data_t  *pdata = NULL;
	int rc;

	cam_info("%s power off enter", __func__);
	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}

	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	pdata = (struct drv_ic_mp3336_private_data_t  *)drv_ic->pdata;

	rc = drv_ic_mp3336_set_pin(pdata, DISABLE);

	cam_info("%s power off end", __func__);
	return rc;
}

char const *mp3336_get_name(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *mp3336 = NULL;

	cam_info("%s get name enter", __func__);

	mp3336 = intf_to_driveric(intf);

	if (!mp3336) {
		cam_err("%s. mp3336 is NULL", __func__);
		return NULL;
	}

	return mp3336->name;
}

static struct hwdriveric_vtbl_t g_vtbl_mp3336 = {
	.get_name = mp3336_get_name,
	.power_on = mp3336_power_on,
	.power_off = mp3336_power_off,
	.driveric_get_dt_data = mp3336_get_dt_data,
	.init = mp3336_init,
};

static struct driveric_t g_mp3336 = {
	.name  = "drv_ic_mp3336",
	.intf  = { .vtbl = &g_vtbl_mp3336, },
	.pdata = (void *)&g_mp3336_pdata,
};

static const struct of_device_id g_mp3336_dt_match[] = {
	{
		.compatible = "vendor,drv_ic_mp3336",
		.data = &g_mp3336.intf,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, g_mp3336_dt_match);

static struct platform_driver g_mp3336_driver = {
	.driver = {
		.name = "vendor,drv_ic_mp3336",
		.owner = THIS_MODULE,
		.of_match_table = g_mp3336_dt_match,
	},
};

static int32_t mp3336_platform_probe(struct platform_device *pdev)
{
	cam_notice("%s enter", __func__);
	g_pdev = pdev;
	return hwdriveric_register(pdev, &g_mp3336.intf, &g_mp3336.notify);
}

static int __init mp3336_init_module(void)
{
	int ret = 0;

	cam_notice("%s enter", __func__);

	ret = platform_driver_probe(&g_mp3336_driver,
		mp3336_platform_probe);

	return ret;
}

static void __exit mp3336_exit_module(void)
{
	platform_driver_unregister(&g_mp3336_driver);
	hwdriveric_unregister(g_pdev);
}

module_init(mp3336_init_module);
module_exit(mp3336_exit_module);
MODULE_DESCRIPTION("mp3336");
MODULE_LICENSE("GPL v2");

