/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * spmi_core.c
 *
 * spmi core
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "spmi_dbgfs.h"
#include <platform_include/basicplatform/linux/spmi_platform.h>
#include <platform_include/basicplatform/linux/of_platform_spmi.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/uaccess.h>
#include <linux/arm-smccc.h>
#include <platform_include/basicplatform/linux/pr_log.h>
#ifndef CONFIG_PRODUCT_ARMPC
#define CREATE_TRACE_POINTS
#endif
#include <trace/events/spmi.h>

#define PR_LOG_TAG SPMI_TAG

#ifdef CONFIG_GCOV_KERNEL
#define STATIC
#else
#define STATIC static
#endif

static DEFINE_MUTEX(board_lock);
static LIST_HEAD(board_list);
static DEFINE_IDR(ctrl_idr);

static void spmi_dev_release(struct device *dev)
{
	struct spmi_device *spmidev = to_spmi_device(dev);

	kfree(spmidev);
}

static struct device_type spmi_dev_type = {
	.groups		= NULL,
	.uevent		= NULL,
	.release	= spmi_dev_release,
};

static int spmi_device_match(struct device *dev, struct device_driver *drv);
#ifdef CONFIG_PM_SLEEP
static int spmi_pm_suspend(struct device *dev);
static int spmi_pm_resume(struct device *dev);
#endif

static SIMPLE_DEV_PM_OPS(spmi_dev_pm_ops, spmi_pm_suspend, spmi_pm_resume);

struct bus_type spmi_bus_type = {
	.name		= "spmi",
	.match		= spmi_device_match,
	.pm		= &spmi_dev_pm_ops,
};
EXPORT_SYMBOL_GPL(spmi_bus_type);

static int spmi_register_controller(struct spmi_controller *ctrl);

/**
 * spmi_busnum_to_ctrl: Map bus number to controller
 * @busnum: bus number
 * Returns controller representing this bus number
 */
struct spmi_controller *spmi_busnum_to_ctrl(u32 bus_num)
{
	struct spmi_controller *ctrl = NULL;

	mutex_lock(&board_lock);
	ctrl = idr_find(&ctrl_idr, bus_num);
	mutex_unlock(&board_lock);

	return ctrl;
}
EXPORT_SYMBOL_GPL(spmi_busnum_to_ctrl);

/**
 * spmi_add_controller: Controller bring-up.
 * @ctrl: controller to be registered.
 * A controller is registered with the framework using this API. ctrl->nr is the
 * desired number with which SPMI framework registers the controller.
 * Function will return -EBUSY if the number is in use.
 */
int spmi_add_controller(struct spmi_controller *ctrl)
{
	int id;
	int ret;

	if (!ctrl)
		return -EINVAL;

	pr_err("adding controller for bus %u (0x%pK)\n", ctrl->nr, ctrl);

	mutex_lock(&board_lock);
	id = idr_alloc(&ctrl_idr, ctrl, ctrl->nr, ctrl->nr + 1, GFP_KERNEL);
	mutex_unlock(&board_lock);

	if (id < 0)
		return id;

	ctrl->nr = id;
	ret = spmi_register_controller(ctrl);
	if (ret) {
		mutex_lock(&board_lock);
		idr_remove(&ctrl_idr, ctrl->nr);
		mutex_unlock(&board_lock);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spmi_add_controller);

/* Remove a device associated with a controller */
static int spmi_ctrl_remove_device(struct device *dev, void *data)
{
	struct spmi_device *spmidev = to_spmi_device(dev);
	struct spmi_controller *ctrl = data;

	if (dev->type == &spmi_dev_type && spmidev->ctrl == ctrl)
		spmi_remove_device(spmidev);

	return 0;
}

/**
 * spmi_del_controller: Controller tear-down.
 * @ctrl: controller to be removed.
 *
 * Controller added with the above API is torn down using this API.
 */
int spmi_del_controller(struct spmi_controller *ctrl)
{
	struct spmi_controller *found = NULL;

	if (!ctrl)
		return -EINVAL;

	/* Check that the ctrl has been added */
	mutex_lock(&board_lock);
	found = idr_find(&ctrl_idr, ctrl->nr);
	mutex_unlock(&board_lock);
	if (found != ctrl)
		return -EINVAL;

	/* Remove all the clients associated with this controller */
	mutex_lock(&board_lock);
	bus_for_each_dev(&spmi_bus_type, NULL, ctrl, spmi_ctrl_remove_device);
	mutex_unlock(&board_lock);

#ifdef CONFIG_SPMI_PLATFORM_DEBUG_FS
	(void)spmi_dfs_del_controller(ctrl);
#endif
	mutex_lock(&board_lock);
	idr_remove(&ctrl_idr, ctrl->nr);
	mutex_unlock(&board_lock);

	init_completion(&ctrl->dev_released);
	device_unregister(&ctrl->dev);
	wait_for_completion(&ctrl->dev_released);

	return 0;
}
EXPORT_SYMBOL_GPL(spmi_del_controller);

static void spmi_ctrl_release(struct device *dev)
{
	struct spmi_controller *ctrl = to_spmi_controller(dev);

	complete(&ctrl->dev_released);
}

static struct device_type spmi_ctrl_type = {
	.groups		= NULL,
	.release	= spmi_ctrl_release,
};

/**
 * spmi_alloc_device: Allocate a new SPMI devices.
 * @ctrl: controller to which this device is to be added to.
 * Context: can sleep
 *
 * Allows a driver to allocate and initialize a SPMI device without
 * registering it immediately.  This allows a driver to directly fill
 * the spmi_device structure before calling spmi_add_device().
 *
 * Caller is responsible to call spmi_add_device() on the returned
 * spmi_device.  If the caller needs to discard the spmi_device without
 * adding it, then spmi_dev_put() should be called.
 */
struct spmi_device *spmi_alloc_device(struct spmi_controller *ctrl)
{
	struct spmi_device *spmidev = NULL;

	if (!ctrl || !spmi_busnum_to_ctrl(ctrl->nr)) {
		pr_err("Missing SPMI controller\n");
		return NULL;
	}

	spmidev = kzalloc(sizeof(*spmidev), GFP_KERNEL);
	if (!spmidev) {
		dev_err(&ctrl->dev, "unable to allocate spmi_device\n");
		return NULL;
	}

	spmidev->ctrl = ctrl;
	spmidev->dev.parent = ctrl->dev.parent;
	spmidev->dev.bus = &spmi_bus_type;
	spmidev->dev.type = &spmi_dev_type;
	device_initialize(&spmidev->dev);

	return spmidev;
}
EXPORT_SYMBOL_GPL(spmi_alloc_device);

/* Validate the SPMI device structure */
static struct device *get_valid_device(struct spmi_device *spmidev)
{
	struct device *dev = NULL;

	dev = &spmidev->dev;
	if (dev->bus != &spmi_bus_type || dev->type != &spmi_dev_type)
		return NULL;

	if (!spmidev->ctrl || !spmi_busnum_to_ctrl(spmidev->ctrl->nr))
		return NULL;

	return dev;
}

/**
 * spmi_add_device: Add a new device without register board info.
 * @spmi_dev: spmi_device to be added (registered).
 *
 * Called when device doesn't have an explicit client-driver to be probed, or
 * the client-driver is a module installed dynamically.
 */
int spmi_add_device(struct spmi_device *spmidev)
{
	int rc;
	struct device *dev = NULL;

	if (!spmidev)
		return -EINVAL;

	dev = get_valid_device(spmidev);
	if (!dev) {
		pr_err("invalid SPMI device\n");
		return -EINVAL;
	}

	/* Set the device name */
	dev_set_name(dev, "%s-%s", spmidev->name, dev->of_node->name);

	/* Device may be bound to an active driver when this returns */
	rc = device_add(dev);
	if (rc < 0)
		dev_err(dev, "Can't add %s, status %d\n", dev_name(dev), rc);
	else
		dev_info(dev, "device %s registered\n", dev_name(dev));

	return rc;
}
EXPORT_SYMBOL_GPL(spmi_add_device);

/**
 * spmi_new_device: Instantiates a new SPMI device
 * @ctrl: controller to which this device is to be added to.
 * @info: board information for this device.
 *
 * Returns the new device or NULL.
 */
struct spmi_device *spmi_new_device(struct spmi_controller *ctrl,
	struct spmi_boardinfo const *info)
{
	struct spmi_device *spmidev = NULL;
	int rc;

	if (!ctrl || !info)
		return NULL;

	spmidev = spmi_alloc_device(ctrl);
	if (!spmidev)
		return NULL;

	spmidev->name = info->name;
	spmidev->usid  = info->slave_id;
	spmidev->dev.of_node = info->of_node;
	spmidev->dev.platform_data = (void *)info->platform_data;
	spmidev->num_dev_node = info->num_dev_node;
	spmidev->dev_node = info->dev_node;
	spmidev->res = info->res;

	rc = spmi_add_device(spmidev);
	if (rc < 0) {
		spmi_dev_put(spmidev);
		kfree(spmidev);
		return NULL;
	}

	return spmidev;
}
EXPORT_SYMBOL_GPL(spmi_new_device);

void spmi_remove_device(struct spmi_device *spmi_dev)
{
	if (spmi_dev)
		device_unregister(&spmi_dev->dev);
}
EXPORT_SYMBOL_GPL(spmi_remove_device);

static int atfd_spmi_smc(u64 _function_id, u64 _arg0,
	u64 _arg1, u64 _arg2)
{
	struct arm_smccc_res res = {0};

#ifndef CONFIG_LIBLINUX
	arm_smccc_smc(_function_id, _arg0, _arg1, _arg2, 0, 0, 0, 0, &res);
#else
	arm_smccc_1_1_smc(_function_id, _arg0, _arg1, _arg2, 0, 0, 0, 0, &res);
#endif

	return (int)res.a0;
}

static int spmi_read_cmd(struct spmi_controller *ctrl,
	u8 opcode, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	int ret = 0;
	u8 i;
	unsigned long flags;

	if (!ctrl || !ctrl->read_cmd || ctrl->dev.type != &spmi_ctrl_type)
		return -EINVAL;

	trace_spmi_read_begin(opcode, sid, addr);
	if (ctrl->always_sec) {
		spin_lock_irqsave(&ctrl->sec_lock, flags);
		for (i = 0; i < bc; i++) {
			ret = atfd_spmi_smc((u64)(SPMI_FN_MAIN_ID |
					SPMI_READ),
					(u64)sid, (u64)addr, (u64)NULL);
			if (ret < 0) {
				spin_unlock_irqrestore(&ctrl->sec_lock, flags);
				return ret;
			}
			*buf = ret;
			ret = 0;
			buf++;
			addr++;
		}
		spin_unlock_irqrestore(&ctrl->sec_lock, flags);
	} else {
		ret = ctrl->read_cmd(ctrl, opcode, sid, addr, buf, bc);
	}
	trace_spmi_read_end(opcode, sid, addr, ret, bc, buf);
	return ret;
}

static int spmi_write_cmd(struct spmi_controller *ctrl,
	u8 opcode, u8 sid, u16 addr, u8 bc, u8 *buf)
{
	int ret = 0;
	u8 i;
	unsigned long flags;

	if (!ctrl || !ctrl->write_cmd || ctrl->dev.type != &spmi_ctrl_type)
		return -EINVAL;

	trace_spmi_write_begin(opcode, sid, addr, bc, buf);
	if (ctrl->always_sec) {
		spin_lock_irqsave(&ctrl->sec_lock, flags);
		for (i = 0; i < bc; i++) {
			ret = atfd_spmi_smc((u64)(SPMI_FN_MAIN_ID |
					SPMI_WRITE),
					(u64)sid, (u64)addr, (u64)*buf);
			buf++;
			addr++;
		}
		spin_unlock_irqrestore(&ctrl->sec_lock, flags);
	} else {
		ret =  ctrl->write_cmd(ctrl, opcode, sid, addr, buf, bc);
	}
	trace_spmi_write_end(opcode, sid, addr, ret);

	return ret;
}

/**
 * spmi_register_read() - register read
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (5-bit address).
 * @buf: buffer to be populated with data from the Slave.
 *
 * Reads 1 byte of data from a Slave device register.
 */
int vendor_spmi_register_read(struct spmi_controller *ctrl,
	u8 sid, u8 addr, u8 *buf)
{
	/* 4-bit Slave Identifier, 5-bit register address */
	if (sid > SPMI_MAX_SLAVE_ID || addr > 0x1F || buf == NULL)
		return -EINVAL;

	return spmi_read_cmd(ctrl, SPMI_CMD_READ, sid, addr, 0, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_register_read);

/**
 * spmi_ext_register_read() - extended register read
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (8-bit address).
 * @len: the request number of bytes to read (up to 16 bytes).
 * @buf: buffer to be populated with data from the Slave.
 *
 * Reads up to 16 bytes of data from the extended register space on a
 * Slave device.
 */
int vendor_spmi_ext_register_read(struct spmi_controller *ctrl,
	u8 sid, u8 addr, u8 *buf, int len)
{
	/* 4-bit Slave Identifier, 8-bit register address, up to 16 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > 16 || buf == NULL)
		return -EINVAL;

	return spmi_read_cmd(ctrl, SPMI_CMD_EXT_READ, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_ext_register_read);

/**
 * spmi_ext_register_readl() - extended register read long
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (16-bit address).
 * @len: the request number of bytes to read (up to 8 bytes).
 * @buf: buffer to be populated with data from the Slave.
 *
 * Reads up to 8 bytes of data from the extended register space on a
 * Slave device using 16-bit address.
 */
int vendor_spmi_ext_register_readl(struct spmi_controller *ctrl,
	u8 sid, u16 addr, u8 *buf, int len)
{
	/* Slave Identifier, and len parameter judgment */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > 8 || buf == NULL)
		return -EINVAL;

	return spmi_read_cmd(ctrl, SPMI_CMD_EXT_READL, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_ext_register_readl);

/**
 * spmi_register_write() - register write
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (5-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 *
 * Writes 1 byte of data to a Slave device register.
 */
int vendor_spmi_register_write(struct spmi_controller *ctrl,
	u8 sid, u8 addr, u8 *buf)
{
	u8 op = SPMI_CMD_WRITE;

	/* 4-bit Slave Identifier, 5-bit register address */
	if (sid > SPMI_MAX_SLAVE_ID || addr > 0x1F || buf == NULL)
		return -EINVAL;

	return spmi_write_cmd(ctrl, op, sid, addr, 1, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_register_write);

/**
 * spmi_ext_register_write() - extended register write
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (8-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 * @len: the request number of bytes to read (up to 16 bytes).
 *
 * Writes up to 16 bytes of data to the extended register space of a
 * Slave device.
 */
int vendor_spmi_ext_register_write(struct spmi_controller *ctrl,
	u8 sid, u8 addr, u8 *buf, int len)
{
	u8 op = SPMI_CMD_EXT_WRITE;

	/* 4-bit Slave Identifier, 8-bit register address, up to 16 bytes */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > 16 || buf == NULL)
		return -EINVAL;

	return spmi_write_cmd(ctrl, op, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_ext_register_write);

/**
 * spmi_ext_register_writel() - extended register write long
 * @dev: SPMI device.
 * @sid: slave identifier.
 * @ad: slave register address (16-bit address).
 * @buf: buffer containing the data to be transferred to the Slave.
 * @len: the request number of bytes to read (up to 8 bytes).
 *
 * Writes up to 8 bytes of data to the extended register space of a
 * Slave device using 16-bit address.
 */
int vendor_spmi_ext_register_writel(struct spmi_controller *ctrl,
	u8 sid, u16 addr, u8 *buf, int len)
{
	u8 op = SPMI_CMD_EXT_WRITEL;

	/* Slave Identifier, and len parameter judgment */
	if (sid > SPMI_MAX_SLAVE_ID || len <= 0 || len > 8 || buf == NULL)
		return -EINVAL;

	return spmi_write_cmd(ctrl, op, sid, addr, len, buf);
}
EXPORT_SYMBOL_GPL(vendor_spmi_ext_register_writel);

static const struct spmi_device_id *spmi_match(
	const struct spmi_device_id *id, const struct spmi_device *spmi_dev)
{
	while (id->name[0]) {
		if (strncmp(spmi_dev->name, id->name, SPMI_NAME_SIZE) == 0)
			return id;
		id++;
	}
	return NULL;
}

static int spmi_device_match(struct device *dev,
	struct device_driver *drv)
{
	struct spmi_device *spmi_dev = NULL;
	struct spmi_driver *sdrv = to_spmi_driver(drv);

	if (dev->type == &spmi_dev_type)
		spmi_dev = to_spmi_device(dev);
	else
		return 0;

	/* Attempt an OF style match */
	if (of_driver_match_device(dev, drv))
		return 1;

	if (sdrv->id_table)
		return spmi_match(sdrv->id_table, spmi_dev) != NULL;

	if (drv->name)
		return strncmp(spmi_dev->name, drv->name, SPMI_NAME_SIZE) == 0;
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int spmi_legacy_suspend(struct device *dev)
{
	struct spmi_device *spmi_dev = NULL;
	struct spmi_driver *driver = NULL;

	if (dev->type == &spmi_dev_type)
		spmi_dev = to_spmi_device(dev);

	if (!spmi_dev || !dev->driver)
		return 0;

	driver = to_spmi_driver(dev->driver);
	if (!driver->suspend)
		return 0;

	return driver->suspend(spmi_dev, PMSG_SUSPEND);
}

static int spmi_legacy_resume(struct device *dev)
{
	struct spmi_device *spmi_dev = NULL;
	struct spmi_driver *driver = NULL;

	if (dev->type == &spmi_dev_type)
		spmi_dev = to_spmi_device(dev);

	if (!spmi_dev || !dev->driver)
		return 0;

	driver = to_spmi_driver(dev->driver);
	if (!driver->resume)
		return 0;

	return driver->resume(spmi_dev);
}

static int spmi_pm_suspend(struct device *dev)
{
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pm)
		return pm_generic_suspend(dev);
	else
		return spmi_legacy_suspend(dev);
}

static int spmi_pm_resume(struct device *dev)
{
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pm)
		return pm_generic_resume(dev);
	else
		return spmi_legacy_resume(dev);
}

#endif


struct device spmi_dev = {
	.init_name = "spmi",
};

static int spmi_drv_probe(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	return sdrv->probe(to_spmi_device(dev));
}

static int spmi_drv_remove(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	sdrv->remove(to_spmi_device(dev));
	return 0;
}

static void spmi_drv_shutdown(struct device *dev)
{
	const struct spmi_driver *sdrv = to_spmi_driver(dev->driver);

	sdrv->shutdown(to_spmi_device(dev));
}

/**
 * spmi_driver_register: Client driver registration with SPMI framework.
 * @drv: client driver to be associated with client-device.
 *
 * This API will register the client driver with the SPMI framework.
 * It is called from the driver's module-init function.
 */
static int _spmi_driver_register(struct spmi_driver *drv)
{
	if (!drv)
		return -EINVAL;

	drv->driver.bus = &spmi_bus_type;

	if (drv->probe)
		drv->driver.probe = spmi_drv_probe;

	if (drv->remove)
		drv->driver.remove = spmi_drv_remove;

	if (drv->shutdown)
		drv->driver.shutdown = spmi_drv_shutdown;

	return driver_register(&drv->driver);
}

#ifdef CONFIG_SPMI
int vendor_spmi_driver_register(struct spmi_driver *drv)
{
	return _spmi_driver_register(drv);
}
EXPORT_SYMBOL_GPL(vendor_spmi_driver_register);
#else
int spmi_driver_register(struct spmi_driver *drv)
{
	return _spmi_driver_register(drv);
}
EXPORT_SYMBOL_GPL(spmi_driver_register);
#endif

static int spmi_register_controller(struct spmi_controller *ctrl)
{
	int ret;

	/* Can't register until after driver model init */
	if (WARN_ON(!spmi_bus_type.p)) {
		ret = -EAGAIN;
		return ret;
	}

	dev_set_name(&ctrl->dev, "spmi-%u", ctrl->nr);
	ctrl->dev.bus = &spmi_bus_type;
	ctrl->dev.type = &spmi_ctrl_type;
	ret = device_register(&ctrl->dev);
	if (ret)
		return ret;

	dev_err(&ctrl->dev, "Bus spmi-%d registered: dev:0x%pK\n",
					ctrl->nr, &ctrl->dev);

#ifdef CONFIG_SPMI_PLATFORM_DEBUG_FS
	ret = spmi_dfs_add_controller(ctrl);
	if (ret) {
		dev_err(&ctrl->dev, "Bus spmi-%u registered: dev:0x%pK add debug fs controller failed!\n",
			ctrl->nr, &ctrl->dev);
		device_unregister(&ctrl->dev);
	}
#endif
	return ret;
}

static void vendor_spmi_exit(void)
{
	device_unregister(&spmi_dev);
	bus_unregister(&spmi_bus_type);
}

static int vendor_spmi_init(void)
{
	int retval;

	retval = bus_register(&spmi_bus_type);
	if (!retval) {
		retval = device_register(&spmi_dev);
		if (retval)
			bus_unregister(&spmi_bus_type);
	}

	return retval;
}

static void __exit spmi_exit(void)
{
	vendor_spmi_exit();
}

STATIC int __init spmi_init(void)
{
	return vendor_spmi_init();
}

int spmi_bus_init(void)
{
	pr_info("[%s] vendor spmi init\n", __func__);
	return vendor_spmi_init();
}

void spmi_bus_exit(void)
{
	pr_info("[%s] vendor spmi exit\n", __func__);
	vendor_spmi_exit();
}

#ifndef CONFIG_SPMI
postcore_initcall(spmi_init);
module_exit(spmi_exit);
#endif

MODULE_LICENSE("GPL v2");
