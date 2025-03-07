/*
 * hwdot.h
 *
 * Description: hwdot head file
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HW_ALAN_KERNEL_DOT_INTERFACE_H__
#define __HW_ALAN_KERNEL_DOT_INTERFACE_H__

#include <linux/videodev2.h>
#include <platform_include/camera/native/camera.h>
#include <platform_include/camera/native/dot_cfg.h>
#include <media/videobuf2-core.h>

#include "cam_intf.h"

typedef struct _tag_hwdot_notify_vtbl hwdot_notify_vtbl_t;
typedef struct _tag_hwdot_notify_intf hwdot_notify_intf_t;

typedef struct _tag_hwdot_notify_vtbl {
	void (*error)(hwdot_notify_intf_t *i, hwdot_event_t *dot_ev);
} hwdot_notify_vtbl_t;

typedef struct _tag_hwdot_notify_intf {
	hwdot_notify_vtbl_t *vtbl;
} hwdot_notify_intf_t;

typedef struct _tag_hwdot_vtbl hwdot_vtbl_t;
typedef struct _tag_hwdot_intf hwdot_intf_t;

typedef struct _tag_hwdot_vtbl {
	char const* (*get_name)(const hwdot_intf_t *i);
	int (*power_on)(const hwdot_intf_t *i);
	int (*power_off)(const hwdot_intf_t *i);
	int (*dot_get_dt_data)(const hwdot_intf_t *, struct device_node *);
	int (*dot_register_attribute)(const hwdot_intf_t *, struct device *);
	int (*init)(const hwdot_intf_t *i);
	int (*get_thermal)(const hwdot_intf_t *i, void *);
} hwdot_vtbl_t;

typedef struct _tag_hwdot_intf {
	hwdot_vtbl_t *vtbl;
} hwdot_intf_t;

typedef struct _tag_dot {
	hwdot_intf_t         intf;
	char                 name[HWDOT_NAME_SIZE];
	int                  i2c_index;
	hwdot_notify_intf_t  *notify;
	void                 *pdata;
} dot_t;

static inline char const *hwdot_intf_get_name(const hwdot_intf_t *i)
{
	return i->vtbl->get_name(i);
}

static inline int hwdot_intf_power_on(const hwdot_intf_t *i)
{
	return i->vtbl->power_on(i);
}

static inline int hwdot_intf_power_off(const hwdot_intf_t *i)
{
	return i->vtbl->power_off(i);
}

static inline void hwdot_intf_notify_error(hwdot_notify_intf_t *i,
	hwdot_event_t *dot_ev)
{
	return i->vtbl->error(i, dot_ev);
}

extern int32_t hwdot_register(struct platform_device *pdev,
	const hwdot_intf_t *i, hwdot_notify_intf_t **notify);

void hwdot_unregister(struct platform_device *pdev);
#endif /* __HW_ALAN_KERNEL_DOT_INTERFACE_H__ */

