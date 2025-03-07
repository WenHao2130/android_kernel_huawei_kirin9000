/*
 * hw_flash.c
 *
 * Copyright (c) 2011-2020 Huawei Technologies Co., Ltd.
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

#include "hw_subdev.h"
#include "hw_flash.h"
#include "hw_flash_i2c.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "cam_intf.h"
#include "../../clt/kernel_clt_flag.h"
#include <securec.h>

struct dsm_client_ops ops3 = {
	.poll_state = NULL,
	.dump_func = NULL,
};

struct dsm_dev dev_flash = {
	.name = "dsm_flash",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &ops3,
	.buff_size = 256,
};

struct dsm_client *client_flash;

static DEFINE_MUTEX(g_flash_lock);
static DEFINE_MUTEX(g_flash_switch_lock);
static unsigned int g_flash_led_state;

struct hw_flash_ctrl_t *hw_kernel_flash_ctrl;

void hw_set_flash_ctrl(struct hw_flash_ctrl_t *flash_ctrl)
{
	mutex_lock(&g_flash_lock);
	hw_kernel_flash_ctrl = flash_ctrl;
	mutex_unlock(&g_flash_lock);
}

struct hw_flash_ctrl_t *hw_get_flash_ctrl(void)
{
	return hw_kernel_flash_ctrl;
}

unsigned int hw_flash_get_state(void)
{
	unsigned int state;

	mutex_lock(&g_flash_lock);
	cam_info("%s enter flash_state %d\n", __func__, g_flash_led_state);
	state = (g_flash_led_state & (FLASH_LED_THERMAL_PROTECT_ENABLE
		| FLASH_LED_LOWPOWER_PROTECT_ENABLE));
	mutex_unlock(&g_flash_lock);

	return state;
}

void hw_flash_set_state(unsigned int state)
{
	mutex_lock(&g_flash_lock);
	cam_info("%s enter flash_state = %d, state = %d\n",
		__func__, g_flash_led_state, state);
	g_flash_led_state |= state;
	mutex_unlock(&g_flash_lock);
}

void hw_flash_clear_state(unsigned int state)
{
	mutex_lock(&g_flash_lock);
	cam_info("%s enter flash_state = %d, state = %d\n",
		__func__, g_flash_led_state, state);
	g_flash_led_state &= (~state);
	mutex_unlock(&g_flash_lock);
}

void hw_flash_enable_thermal_protect(void)
{
	cam_info("%s enter", __func__);
	hw_flash_set_state(FLASH_LED_THERMAL_PROTECT_ENABLE);
}

void hw_flash_disable_thermal_protect(void)
{
	cam_info("%s enter", __func__);
	hw_flash_clear_state(FLASH_LED_THERMAL_PROTECT_ENABLE);
}

static ssize_t hw_flash_thermal_protect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;
	unsigned int state = hw_flash_get_state();

	rc = scnprintf(buf, PAGE_SIZE, "%d", state);

	return rc;
}

static ssize_t hw_flash_thermal_protect_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_ctrl_t *flash_ctrl = NULL;
	int rc = 0;

	if (buf[0] == '0') {
		cam_info("%s disable thermal protect", __func__);
		hw_flash_clear_state(FLASH_LED_THERMAL_PROTECT_ENABLE);
	} else {
		cam_info("%s enable thermal protect", __func__);
		hw_flash_set_state(FLASH_LED_THERMAL_PROTECT_ENABLE);
		flash_ctrl = hw_get_flash_ctrl();
		cam_info("%s flash mode = %d", __func__,
			flash_ctrl->state.mode);
		if (flash_ctrl->state.mode != STANDBY_MODE) {
			cam_notice("%s turn off flash", __func__);
			rc = flash_ctrl->func_tbl->flash_off(flash_ctrl);
			if (rc < 0)
				cam_err("%s failed to turn off flash",
					__func__);
		}
	}

	return count;
}

/* check flash led open or short */
static ssize_t hw_flash_led_fault_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hw_flash_ctrl_t *flash_ctrl = NULL;
	int status = -1;
	int rc = 0;
	// get flash controller
	flash_ctrl = hw_get_flash_ctrl();
	if (!flash_ctrl) {
		cam_err("%s: flash ctrl is null", __func__);
		return -1;
	}

	if (flash_ctrl->func_tbl->flash_check) {
		status = flash_ctrl->func_tbl->flash_check(flash_ctrl);
		rc = scnprintf(buf, PAGE_SIZE, "%d", status);
	} else {
		cam_err("%s: flash check is NULL", __func__);
		rc = scnprintf(buf, PAGE_SIZE, "%d", FLASH_LED_ERROR);
	}

	return rc;
}

static ssize_t hw_flash_led_fault_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	// fake store function
	return 1;
}

static ssize_t hw_flash_lowpower_protect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;
	unsigned int state = hw_flash_get_state();

	rc = scnprintf(buf, PAGE_SIZE, "%d", state);

	return rc;
}

static ssize_t hw_flash_lowpower_protect_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] == '0') {
		cam_info("%s disable lowpower protect", __func__);
		hw_flash_clear_state(FLASH_LED_LOWPOWER_PROTECT_ENABLE);
	} else {
		cam_info("%s enable lowpower protect", __func__);
		hw_flash_set_state(FLASH_LED_LOWPOWER_PROTECT_ENABLE);
	}

	return count;
}

static struct device_attribute g_flash_lowpower_protect =
	__ATTR(flash_lowpower_protect, 0664, hw_flash_lowpower_protect_show,
	hw_flash_lowpower_protect_store);

static struct device_attribute g_flash_thermal_protect =
	__ATTR(flash_thermal_protect, 0664, hw_flash_thermal_protect_show,
	hw_flash_thermal_protect_store);

/* check flash led open or short */
static struct device_attribute g_flash_led_fault =
	__ATTR(flash_led_fault, 0664, hw_flash_led_fault_show,
	hw_flash_led_fault_store);

static int hw_flash_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc;

	if (!flash_ctrl || !dev) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -1;
	}

	rc = device_create_file(dev, &g_flash_lowpower_protect);
	if (rc < 0) {
		cam_err("%s failed to creat flash lowpower protect attribute",
		__func__);
		goto err_out;
	}

	rc = device_create_file(flash_ctrl->cdev_torch.dev,
		&g_flash_thermal_protect);
	if (rc < 0) {
		cam_err("%s failed to creat flash lowpower protect attribute",
			__func__);
		goto err_create_flash_thermal_protect_file;
	}

	/* check flash led open or short */
	cam_notice("create node: flash led fault");
	rc = device_create_file(flash_ctrl->cdev_torch.dev,
		&g_flash_led_fault);
	if (rc < 0) {
		cam_err("%s failed to create flash led fault attribute",
			__func__);
		goto err_create_flash_led_fault_file;
	}

	return 0;
err_create_flash_led_fault_file:
	device_remove_file(dev, &g_flash_thermal_protect);
err_create_flash_thermal_protect_file:
	device_remove_file(dev, &g_flash_lowpower_protect);
err_out:
	return rc;
}

static int hw_flash_mix_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc;

	if (!flash_ctrl || !dev) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -EINVAL;
	}

	rc = device_create_file(flash_ctrl->cdev_torch1.dev,
		&g_flash_thermal_protect);
	if (rc < 0) {
		cam_err("%s failed to creat flash mix lowpower protect attribute",
			__func__);
		goto err_out;
	}

	/* check flash led open or short */
	cam_notice("create node: flash led fault");
	rc = device_create_file(flash_ctrl->cdev_torch1.dev,
		&g_flash_led_fault);
	if (rc < 0) {
		cam_err("%s failed to create flash mix led fault attribute",
			__func__);
		goto err_create_flash_led_fault_file;
	}

	return 0;

err_create_flash_led_fault_file:
	device_remove_file(dev, &g_flash_thermal_protect);
err_out:
	return rc;
}

int hw_flash_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct device_node *dev_node = NULL;
	struct hw_flash_info *flash_info = NULL;
	int rc = -1;

	cam_debug("%s enter\n", __func__);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return rc;
	}

	dev_node = flash_ctrl->dev->of_node;

	flash_info = &flash_ctrl->flash_info;

	rc = of_property_read_string(dev_node, "vendor,flash-name",
		&flash_info->name);
	cam_info("%s vendor,flash-name %s, rc %d\n", __func__,
		flash_info->name, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	rc = of_property_read_u32(dev_node, "vendor,flash-index",
		(u32 *)&flash_info->index);
	cam_info("%s vendor,flash-index %d, rc %d\n", __func__,
		flash_info->index, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

	rc = of_property_read_u32(dev_node, "vendor,flash-classtype",
		(u32 *)&flash_info->classtype);
	cam_info("%s vendor,flash-classtype %d, rc %d\n", __func__,
		flash_info->classtype, rc);
	if (rc < 0)
		cam_err("%s failed %d\n", __func__, __LINE__);

	rc = of_property_read_u32(dev_node, "vendor,slave-address",
		&flash_info->slave_address);
	cam_info("%s slave-address %d, rc %d\n", __func__,
		flash_info->slave_address, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		goto fail;
	}

fail:
	return rc;
}

static struct hw_flash_ctrl_t *get_sctrl(struct v4l2_subdev *sd)
{
	return container_of(container_of(sd, struct hw_sd_subdev, sd),
		struct hw_flash_ctrl_t, hw_sd);
}

int hw_flash_config(struct hw_flash_ctrl_t *flash_ctrl, void *arg)
{
	struct hw_flash_cfg_data *cdata = NULL;
	int rc = 0;
	unsigned int state;

	if (!flash_ctrl || !arg) {
		cam_err("%s flash_ctrl or arg is NULL", __func__);
		return -1;
	}
	cdata = (struct hw_flash_cfg_data *)arg;

	cam_info("%s enter cfgtype = %d, position = %d\n",
		__func__, cdata->cfgtype, cdata->position);

	switch (cdata->cfgtype) {
	case CFG_FLASH_TURN_ON:
		mutex_lock(&g_flash_switch_lock);
		state = hw_flash_get_state();
		flash_ctrl->mix_pos = cdata->position;
		if (state == 0) {
			rc = flash_ctrl->func_tbl->flash_on(flash_ctrl,
				arg);
		} else {
			cam_info("%s flashe led is disable 0x%x",
				__func__, state);
			rc = -1;
		}
		mutex_unlock(&g_flash_switch_lock);
		break;
	case CFG_FLASH_TURN_OFF:
		mutex_lock(&g_flash_switch_lock);
		flash_ctrl->mix_pos = cdata->position;
		rc = flash_ctrl->func_tbl->flash_off(flash_ctrl);
		mutex_unlock(&g_flash_switch_lock);
		break;
	case CFG_FLASH_GET_FLASH_NAME:
		mutex_lock(flash_ctrl->hw_flash_mutex);
		rc = memset_s(cdata->cfg.name,
			sizeof(cdata->cfg.name),
			0,
			sizeof(cdata->cfg.name));
		if (rc != 0)
			cam_err("memset failed %d", __LINE__);
		rc = strncpy_s(cdata->cfg.name,
			sizeof(cdata->cfg.name) - 1,
			flash_ctrl->flash_info.name,
			sizeof(cdata->cfg.name) - 1);
		if (rc != 0) {
			cam_err("%s flash name copy error\n",
				__func__);
			rc = -EFAULT;
		}
		mutex_unlock(flash_ctrl->hw_flash_mutex);
		break;
	case CFG_FLASH_GET_FLASH_STATE:
		mutex_lock(flash_ctrl->hw_flash_mutex);
		cdata->mode = (flash_mode)flash_ctrl->state.mode;
		cdata->data = flash_ctrl->state.data;
		mutex_unlock(flash_ctrl->hw_flash_mutex);
		break;
	case CFG_FLASH_GET_FLASH_TYPE:
		mutex_lock(flash_ctrl->hw_flash_mutex);
		cdata->data = flash_ctrl->flash_info.classtype;
		mutex_unlock(flash_ctrl->hw_flash_mutex);
		cam_info("%s flash class type = %d", __func__,
			cdata->data);
		break;
	default:
		cam_err("%s cfgtype error\n", __func__);
		rc = -EFAULT;
		break;
	}

	return rc;
}
EXPORT_SYMBOL(hw_flash_config);

static long hw_flash_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	struct hw_flash_ctrl_t *flash_ctrl = get_sctrl(sd);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL\n", __func__);
		return -EBADF;
	}
	cam_info("%s cmd = 0x%x\n", __func__, cmd);

	switch (cmd) {
	case VIDIOC_KERNEL_FLASH_CFG:
		return flash_ctrl->func_tbl->flash_config(flash_ctrl,
			arg);
	default:
		cam_err("%s cmd is error", __func__);
		return -ENOIOCTLCMD;
	}
}

static int hw_flash_subdev_internal_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hw_flash_ctrl_t *flash_ctrl = get_sctrl(sd);
	int rc = 0;
	struct hw_flash_cfg_data arg;

	arg.cfgtype = CFG_FLASH_TURN_OFF;
	cam_info("flash close");
	if (!flash_ctrl)
		return 0;

	rc = flash_ctrl->func_tbl->flash_config(flash_ctrl, (void *)(&arg));
	return rc;
}

static struct v4l2_subdev_core_ops g_flash_subdev_core_ops = {
	.ioctl = hw_flash_subdev_ioctl,
};

static struct v4l2_subdev_ops g_flash_subdev_ops = {
	.core = &g_flash_subdev_core_ops,
};

static struct v4l2_subdev_internal_ops g_flash_subdev_internal_ops = {
	.close = &hw_flash_subdev_internal_close,
};

/* Add for CLT Camera, begin */
static int hw_simulate_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_info("%s simulated function for CLT Camera\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}
	return 0;
}

static int hw_simulate_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_info("%s simulated function for CLT Camera\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}
	flash_ctrl->func_tbl->flash_off(flash_ctrl);
	return 0;
}

static int hw_simulate_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	cam_info("%s simulated function for CLT Camera\n", __func__);
	if (!flash_ctrl || !data) {
		cam_err("%s Input parameter is NULL", __func__);
		return -1;
	}
	return 0;
}

static int hw_simulate_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_info("%s simulated function for CLT Camera\n", __func__);
	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}
	if (flash_ctrl->state.mode == STANDBY_MODE)
		return 0;

	mutex_lock(flash_ctrl->hw_flash_mutex);
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = 0;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_simulate_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_info("%s simulated function for CLT Camera\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}
	return 0;
}

/* Add for CLT Camera, end */
int32_t hw_flash_platform_probe(struct platform_device *pdev,
	void *data)
{
	int32_t rc;
	uint32_t group_id;
	struct hw_flash_ctrl_t *flash_ctrl = NULL;

	flash_ctrl = (struct hw_flash_ctrl_t *)data;

	flash_ctrl->pdev = pdev;
	flash_ctrl->dev = &pdev->dev;

	if (kernel_is_clt_flag()) {
		flash_ctrl->func_tbl->flash_init = hw_simulate_init;
		flash_ctrl->func_tbl->flash_exit = hw_simulate_exit;
		flash_ctrl->func_tbl->flash_on = hw_simulate_on;
		flash_ctrl->func_tbl->flash_off = hw_simulate_off;
		flash_ctrl->func_tbl->flash_match = hw_simulate_match;
	}
	cam_debug("%s enter\n", __func__);

	hw_flash_get_dt_data(flash_ctrl);
	rc = flash_ctrl->func_tbl->flash_get_dt_data(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash_get_dt_data failed", __func__);
		return -EFAULT;
	}

	rc = flash_ctrl->func_tbl->flash_init(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash init failed\n", __func__);
		return -EFAULT;
	}

	rc = flash_ctrl->func_tbl->flash_match(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash match failed\n", __func__);
		return -EFAULT;
	}

	if (!flash_ctrl->flash_v4l2_subdev_ops)
		flash_ctrl->flash_v4l2_subdev_ops = &g_flash_subdev_ops;

	flash_ctrl->hw_sd.sd.internal_ops = &g_flash_subdev_internal_ops;
	v4l2_subdev_init(&flash_ctrl->hw_sd.sd,
		flash_ctrl->flash_v4l2_subdev_ops);

	rc = snprintf_s(flash_ctrl->hw_sd.sd.name,
		sizeof(flash_ctrl->hw_sd.sd.name),
		sizeof(flash_ctrl->hw_sd.sd.name) - 1, "%s",
		flash_ctrl->flash_info.name);
	if (rc < 0) {
		cam_err("%s  flash name in overflow\n", __func__);
		return -EFAULT;
	}

	v4l2_set_subdevdata(&flash_ctrl->hw_sd.sd, pdev);

	group_id = flash_ctrl->flash_info.index ?
		CAM_SUBDEV_FLASH1 : CAM_SUBDEV_FLASH0;

	if (flash_ctrl->flash_info.index == 2) {
		group_id = CAM_SUBDEV_FLASH2;
		cam_info("%s group id = %d\n", __func__, CAM_SUBDEV_FLASH2);
	}

	flash_ctrl->hw_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	init_subdev_media_entity(&flash_ctrl->hw_sd.sd, group_id);
	cam_cfgdev_register_subdev(&flash_ctrl->hw_sd.sd, group_id);

	rc = flash_ctrl->func_tbl->flash_register_attribute(flash_ctrl,
		&flash_ctrl->hw_sd.sd.devnode->dev);
	if (rc < 0) {
		cam_err("%s failed to register flash attribute node",
			__func__);
		return rc;
	}

	rc = hw_flash_register_attribute(flash_ctrl,
		&flash_ctrl->hw_sd.sd.devnode->dev);
	if (rc < 0) {
		cam_err("%s failed to register flash attribute node",
			__func__);
		return rc;
	}

	hw_set_flash_ctrl(flash_ctrl);

	return rc;
}

int32_t hw_flash_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct hw_flash_ctrl_t *flash_ctrl = NULL;

	uint32_t group_id;
	int32_t rc;

	cam_info("%s client name = %s\n", __func__, client->name);

	adapter = client->adapter;
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		cam_err("%s i2c_check_functionality failed\n", __func__);
		return -EIO;
	}

	flash_ctrl = (struct hw_flash_ctrl_t *)id->driver_data;
	flash_ctrl->flash_i2c_client->client = client;
	flash_ctrl->dev = &client->dev;
	flash_ctrl->flash_i2c_client->i2c_func_tbl = &hw_flash_i2c_func_tbl;

	if (kernel_is_clt_flag()) {
		flash_ctrl->func_tbl->flash_init = hw_simulate_init;
		flash_ctrl->func_tbl->flash_exit = hw_simulate_exit;
		flash_ctrl->func_tbl->flash_on = hw_simulate_on;
		flash_ctrl->func_tbl->flash_off = hw_simulate_off;
		flash_ctrl->func_tbl->flash_match = hw_simulate_match;
	}
	rc = hw_flash_get_dt_data(flash_ctrl);
	if (rc < 0) {
		cam_err("%s hw_flash_get_dt_data failed", __func__);
		return -EFAULT;
	}

	rc = flash_ctrl->func_tbl->flash_get_dt_data(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash_get_dt_data failed", __func__);
		return -EFAULT;
	}

	flash_ctrl->flash_type = FLASH_ALONE;
	rc = flash_ctrl->func_tbl->flash_init(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash init failed\n", __func__);
		return -EFAULT;
	}

	rc = flash_ctrl->func_tbl->flash_match(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash match failed\n", __func__);
		flash_ctrl->func_tbl->flash_exit(flash_ctrl);
		return -EFAULT;
	}

	if (!flash_ctrl->flash_v4l2_subdev_ops)
		flash_ctrl->flash_v4l2_subdev_ops = &g_flash_subdev_ops;

	flash_ctrl->hw_sd.sd.internal_ops = &g_flash_subdev_internal_ops;

	v4l2_subdev_init(&flash_ctrl->hw_sd.sd, flash_ctrl->flash_v4l2_subdev_ops);

	rc = snprintf_s(flash_ctrl->hw_sd.sd.name,
		sizeof(flash_ctrl->hw_sd.sd.name),
		sizeof(flash_ctrl->hw_sd.sd.name) - 1, "%s",
		flash_ctrl->flash_info.name);
	if (rc < 0) {
		cam_err("%s  flash name in overflow\n", __func__);
		return -EFAULT;
	}

	v4l2_set_subdevdata(&flash_ctrl->hw_sd.sd, client);

	group_id = flash_ctrl->flash_info.index ? CAM_SUBDEV_FLASH1 :
		CAM_SUBDEV_FLASH0;

	if (flash_ctrl->flash_info.index == 2) {
		cam_info("%s group id is SUBDEV_FLASH2 flash name = %s\n",
			__func__, client->name);
		group_id = CAM_SUBDEV_FLASH2;
	}

	flash_ctrl->hw_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	init_subdev_media_entity(&flash_ctrl->hw_sd.sd, group_id);
	cam_cfgdev_register_subdev(&flash_ctrl->hw_sd.sd, group_id);
	rc = flash_ctrl->func_tbl->flash_register_attribute(flash_ctrl,
		&flash_ctrl->hw_sd.sd.devnode->dev);
	if (rc < 0) {
		cam_err("%s failed to register flash attribute node", __func__);
		return rc;
	}

	/* for mix flash, mix:flash_type = 1, alone:flash_type = 0 */
	if (flash_ctrl->flash_type == FLASH_MIX) {
		rc = hw_flash_mix_register_attribute(flash_ctrl,
			&flash_ctrl->hw_sd.sd.devnode->dev);
		if (rc < 0) {
			cam_err("%s failed to register flash node", __func__);
			return rc;
		}
	}

	rc = hw_flash_register_attribute(flash_ctrl,
		&flash_ctrl->hw_sd.sd.devnode->dev);
	if (rc < 0) {
		cam_err("%s failed to register flash node", __func__);
		return rc;
	}
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	if (flash_ctrl->flash_info.index == 0)
		set_hw_dev_flag(DEV_I2C_TPS);
	else if (flash_ctrl->flash_info.index == 1 ||
		flash_ctrl->flash_info.index == 2)
		set_hw_dev_flag(DEV_I2C_FFLASH);

#endif
	hw_set_flash_ctrl(flash_ctrl);

	if (!client_flash)
		client_flash = dsm_register_client(&dev_flash);

	return rc;
}

#ifdef CONFIG_LLT_TEST

struct ut_test_hw_flash g_ut_hw_flash = {
	.hw_flash_thermal_protect_show = hw_flash_thermal_protect_show,
	.hw_flash_thermal_protect_store = hw_flash_thermal_protect_store,
	.hw_flash_led_fault_show = hw_flash_led_fault_show,
	.hw_flash_lowpower_protect_show = hw_flash_lowpower_protect_show,
	.hw_flash_lowpower_protect_store = hw_flash_lowpower_protect_store,
	.hw_flash_register_attribute = hw_flash_register_attribute,
	.hw_flash_subdev_ioctl = hw_flash_subdev_ioctl,
};

unsigned int hw_flash_led_state_read(void)
{
	unsigned int state;

	mutex_lock(&g_flash_lock);
	state = g_flash_led_state;
	mutex_unlock(&g_flash_lock);

	return state;
}

void hw_flash_led_state_write(unsigned int x)
{
	mutex_lock(&g_flash_lock);
	g_flash_led_state = x;
	mutex_unlock(&g_flash_lock);
}

#endif /* CONFIG_LLT_TEST */
