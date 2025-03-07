/*
 * hwsensor.h
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 *
 * SOC camera driver source file
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef __HW_ALAN_KERNEL_SENSOR_INTERFACE_H__
#define __HW_ALAN_KERNEL_SENSOR_INTERFACE_H__

#include <linux/compiler.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <media/v4l2-subdev.h>
#include <platform_include/camera/native/sensor_cfg.h>
#include <linux/videodev2.h>
#include "cam_intf.h"
/*
 * @brief the power state of sensor.
 */
enum hwsensor_state_kind_t {
	HWSENSRO_POWER_DOWN,
	HWSENSOR_POWER_UP,
};

typedef struct _tag_hwsensor_intf hwsensor_intf_t;

/*
 * @brief the sensor interface.
 */
typedef struct _tag_hwsensor_vtbl {
	/* sensor function table */
	char const *(*get_name)(hwsensor_intf_t *);
	int (*match_id)(hwsensor_intf_t *, void *);
	int (*config)(hwsensor_intf_t *, void *);
	int (*power_up)(hwsensor_intf_t *);
	int (*power_down)(hwsensor_intf_t *);
	int (*i2c_read)(hwsensor_intf_t *, void *);
	int (*i2c_write)(hwsensor_intf_t *, void *);
	int (*i2c_read_seq)(hwsensor_intf_t *, void *);
	int (*i2c_write_seq)(hwsensor_intf_t *, void *);
	int (*ioctl)(hwsensor_intf_t *, void *);
	int (*ext_power_ctrl)(int enable);
	int (*set_expo_gain)(hwsensor_intf_t *, u32 expo, u16 gain);
	int (*set_expo)(hwsensor_intf_t *, u32 expo);
	int (*set_gain)(hwsensor_intf_t *, u16 gain);
	int (*csi_enable)(hwsensor_intf_t *);
	int (*csi_disable)(hwsensor_intf_t *);
	int (*sensor_register_attribute)(hwsensor_intf_t *, struct device *);
	int (*ois_wpb_ctrl)(hwsensor_intf_t *, int state);
	int (*otp_config)(hwsensor_intf_t *, void *);
	int (*otp_get)(hwsensor_intf_t *, hwsensor_config_otp_t *);
	int (*otp_update)(hwsensor_intf_t *, hwsensor_config_otp_t *);
	int (*get_thermal)(hwsensor_intf_t *, void *);
} hwsensor_vtbl_t;

struct _tag_hwsensor_intf {
	hwsensor_vtbl_t *vtbl;
};

static inline int hwsensor_intf_otp_config(hwsensor_intf_t *hsi,
	void __user *argp)
{
	return hsi->vtbl->otp_config(hsi, argp);
}

static inline int hwsensor_intf_config(hwsensor_intf_t *hsi, void __user *argp)
{
	return hsi->vtbl->config(hsi, argp);
}

static inline char const *hwsensor_intf_get_name(hwsensor_intf_t *hsi)
{
	return hsi->vtbl->get_name(hsi);
}

static inline int hwsensor_intf_power_up(hwsensor_intf_t *hsi)
{
	return hsi->vtbl->power_up(hsi);
}

static inline int hwsensor_intf_power_down(hwsensor_intf_t *hsi)
{
	return hsi->vtbl->power_down(hsi);
}

static inline int hwsensor_intf_match_id(hwsensor_intf_t *hsi,
	void __user *argp)
{
	return hsi->vtbl->match_id(hsi, argp);
}

extern int hwsensor_register(struct platform_device *pdev, hwsensor_intf_t *si);
extern void hwsensor_unregister(struct platform_device *pdev);
/*
 * use this function to notify video device an event
 * pdev: platform_device which creat by sensor module,like ov8865
 * ev: v4l2 event to send to video device user
 */
extern int hwsensor_notify(struct device *pdev, struct v4l2_event *ev);

#endif // __HW_ALAN_KERNEL_SENSOR_INTERFACE_H__

