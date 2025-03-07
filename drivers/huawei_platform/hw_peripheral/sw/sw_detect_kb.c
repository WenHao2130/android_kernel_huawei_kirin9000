/*
 * sw_detect_kb.c
 *
 * Single wire UART Keyboard detect driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/of.h>
#include <linux/adc.h>
#include <linux/device.h>
#include <linux/delay.h>
#include "sw_detect.h"
#include "sw_debug.h"

#define KB_ONLINE_CONN_MIN_ADC_LIMIT    200
#define KB_ONLINE_CONN_MAX_ADC_LIMIT    350

#define KB_ONLINE_MIN_ADC_LIMIT         1450
#define KB_ONLINE_MAX_ADC_LIMIT         1550

#define KB_DETECT_DELAY_TIME_MS         300

struct sw_kb_detectparam {
	int kb_tx_gpio;
	int kb_vdd_ctrl;
	int kb_power_switch_gpio;

	int kb_connect_adc_min;
	int kb_connect_adc_max;
	int kb_online_adc_min;
	int kb_online_adc_max;
};

static void sw_keyboard_disconnected(struct sw_gpio_detector *detector,
	struct sw_kb_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_KBDEV_ONLINE) {
		sw_print_info("enable irq\n");
		detector->dev_state = DEVSTAT_NONEDEV;
		gpio_set_value(param->kb_vdd_ctrl, 0);
		gpio_set_value(param->kb_power_switch_gpio, 0);
	}
	if (detector->control_irq)
		detector->control_irq(detector, true);

	detector->start_detect = DETECT_ON;
}

static void sw_keyboard_connected(struct sw_gpio_detector *detector,
	struct sw_kb_detectparam *param)
{
	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_NONEDEV) {
		gpio_set_value(param->kb_vdd_ctrl, 1);
		detector->dev_state = DEVSTAT_KBDEV_ONLINE;
		gpio_set_value(param->kb_power_switch_gpio, 1);
		sw_print_info("notify sensorhub\n");
	}

	detector->start_detect = DETECT_OFF;
}

static bool sw_is_kb_online(int detect_adc, struct sw_kb_detectparam *param)
{
	int adc_val;
	int val;
	int count = 5; /* check adc val 5 times */
	int i;
	int check_ok = 0;
	int ret_check = 3; /* adc check fail retry times */

	msleep(KB_DETECT_DELAY_TIME_MS);

retry_check:

	for (i = 0; i < count; i++) {
		val = gpio_get_value(param->kb_vdd_ctrl);
		adc_val = hkadc_detect_value(detect_adc);

		/*
		 * VDD is disabled , kb online adc in
		 * [KB_ONLINE_MIN_ADC_LIMIT KB_ONLINE_CONN_MAX_ADC_LIMIT]
		 */
		if (val == 0) {
			if ((adc_val > param->kb_connect_adc_min) &&
				(adc_val < param->kb_connect_adc_max))
				check_ok++;
		} else {
			/*
			 * VDD is enable , adc maybe in
			 * [KB_ONLINE_MIN_ADC_LIMIT KB_ONLINE_MAX_ADC_LIMIT]
			 */
			if ((adc_val > param->kb_online_adc_min) &&
				(adc_val < param->kb_online_adc_max))
				check_ok++;
		}
	}

	/* if adc check all success, mean connected, return true */
	if (check_ok == count)
		return true;

	/*
	 * if adc check had failed ,need retry check ;
	 * if retry_check < 0 , mean disconnect ,
	 * but this checked will have some mistake
	 */
	if (ret_check > 0) {
		ret_check--;
		check_ok = 0;
		msleep(RECHECK_ADC_DELAY_MS);
		goto retry_check;
	}

	return false;
}

static int sw_kb_devdetect(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector)
{
	bool kb_isonline = false;
	struct sw_kb_detectparam *kb_param = NULL;

	if (!detector || !devdetector)
		return -EINVAL;

	if ((detector->dev_state != DEVSTAT_NONEDEV) &&
		(detector->dev_state != DEVSTAT_KBDEV_ONLINE))
		return -EINVAL;

	kb_param = (struct sw_kb_detectparam *)devdetector->param;
	if (!kb_param) {
		sw_print_info("sw_is_kb_online param is null\n");
		return -EINVAL;
	}

	kb_isonline = sw_is_kb_online(detector->detect_adc_no, kb_param);
	if (kb_isonline)
		sw_keyboard_connected(detector, kb_param);
	else
		sw_keyboard_disconnected(detector, kb_param);

	return SUCCESS;
}

static void sw_kb_free_param(struct sw_kb_detectparam *kb_param)
{
	if (!kb_param)
		return;

	if (kb_param->kb_tx_gpio >= 0) {
		gpio_free(kb_param->kb_tx_gpio);
		kb_param->kb_tx_gpio = INVALID_VALUE;
	}

	if (kb_param->kb_vdd_ctrl >= 0) {
		gpio_free(kb_param->kb_vdd_ctrl);
		kb_param->kb_vdd_ctrl = INVALID_VALUE;
	}
	kfree(kb_param);
}

static int sw_kb_notifyevent(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector,
	unsigned long event, void *data)
{
	struct sw_kb_detectparam *kb_param = NULL;

	if (!detector || !devdetector)
		return -EINVAL;

	kb_param = (struct sw_kb_detectparam *)devdetector->param;

	/* in first , process destroy event */
	if (event == SW_NOTIFY_EVENT_DESTROY) {
		sw_kb_free_param(kb_param);
		kfree(devdetector);
		return SUCCESS;
	}

	/* for bussiness */
	if ((detector->dev_state != DEVSTAT_NONEDEV) &&
		(detector->dev_state != DEVSTAT_KBDEV_ONLINE))
		return -EINVAL;

	if (!kb_param) {
		sw_print_info("sw_is_kb_online param is null\n");
		return -EINVAL;
	}

	if (event == SW_NOTIFY_EVENT_DISCONNECT)
		sw_keyboard_disconnected(detector, kb_param);

	return SUCCESS;
}

static int sw_parse_kbdetectparam(struct device_node *np, struct sw_kb_detectparam *kb_param)
{
	struct device_node *pogopin_chg_np = NULL;

	kb_param->kb_tx_gpio = INVALID_VALUE;
	kb_param->kb_vdd_ctrl = INVALID_VALUE;
	kb_param->kb_connect_adc_min = INVALID_VALUE;
	kb_param->kb_connect_adc_max = INVALID_VALUE;
	kb_param->kb_online_adc_min = INVALID_VALUE;
	kb_param->kb_online_adc_max = INVALID_VALUE;
	kb_param->kb_power_switch_gpio = INVALID_VALUE;

	if (of_property_read_u32(np, "kb_connect_adc_min", &kb_param->kb_connect_adc_min)) {
		sw_print_err("dts:can not get kb_connect_adc_min\n");
		kb_param->kb_connect_adc_min = KB_ONLINE_CONN_MIN_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_connect_adc_max", &kb_param->kb_connect_adc_max)) {
		sw_print_err("dts:can not get kb_connect_adc_max\n");
		kb_param->kb_connect_adc_max = KB_ONLINE_CONN_MAX_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_online_adc_min", &kb_param->kb_online_adc_min)) {
		sw_print_err("dts:can not get kb_online_adc_min\n");
		kb_param->kb_online_adc_min = KB_ONLINE_MIN_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "kb_online_adc_max", &kb_param->kb_online_adc_max)) {
		sw_print_err("dts:can not get kb_online_adc_max\n");
		kb_param->kb_online_adc_max = KB_ONLINE_MAX_ADC_LIMIT;
	}

	/* read keyborad TX gpio,default HIGH */
	if ((kb_param->kb_tx_gpio = sw_get_named_gpio(np, "gpio_kb_tx", GPIOD_OUT_HIGH)) < 0) {
		sw_print_err("kb_tx_gpio failed\n");
		goto err_free_gpio;
	}

	/* read keyboard VDD control ,default LOW */
	if ((kb_param->kb_vdd_ctrl = sw_get_named_gpio(np, "gpio_kb_vdd_ctrl", GPIOD_OUT_LOW)) < 0) {
		sw_print_err("kb_vdd_ctrl failed\n");
		goto err_free_gpio;
	}

	pogopin_chg_np = of_find_node_by_name(NULL, "huawei_pogopin_charger");
	if (pogopin_chg_np) {
		if ((kb_param->kb_power_switch_gpio = of_get_named_gpio(pogopin_chg_np, "power_switch_gpio", 0)) < 0)
			sw_print_err("gpio(power_switch_gpio) failed\n");
	} else {
		sw_print_err("node(huawei_pogopin_charger) failed\n");
	}

	return SUCCESS;

err_free_gpio:
	if (kb_param->kb_tx_gpio >= 0) {
		gpio_free(kb_param->kb_tx_gpio);
		kb_param->kb_tx_gpio = INVALID_VALUE;
	}

	if (kb_param->kb_vdd_ctrl >= 0) {
		gpio_free(kb_param->kb_vdd_ctrl);
		kb_param->kb_vdd_ctrl = INVALID_VALUE;
	}

	return -EINVAL;
}

struct sw_dev_detector *sw_load_kb_detect(struct device_node *np)
{
	struct sw_dev_detector *dev_detector = NULL;
	struct sw_kb_detectparam *kb_param = NULL;
	int ret;

	if (!np) {
		sw_print_err("param failed\n");
		return NULL;
	}

	dev_detector = kzalloc(sizeof(*dev_detector), GFP_KERNEL);
	if (!dev_detector)
		return NULL;

	kb_param = kzalloc(sizeof(*kb_param), GFP_KERNEL);
	if (!kb_param)
		goto err_kb_param;

	ret = sw_parse_kbdetectparam(np, kb_param);
	if (ret < 0) {
		sw_print_err("dts parse failed\n");
		goto err_core_init;
	}

	dev_detector->detect_call = sw_kb_devdetect;
	dev_detector->event_call = sw_kb_notifyevent;
	dev_detector->param = kb_param;

	return dev_detector;

err_core_init:
	kfree(kb_param);
err_kb_param:
	kfree(dev_detector);
	return NULL;
}
