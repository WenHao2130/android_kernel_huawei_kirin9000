/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2021. All rights reserved.
 *
 * platform data structure for pmic led controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_PMIC_LEDS_H
#define __LINUX_PMIC_LEDS_H
#include <linux/leds.h>
#define LEDS_SPMI "leds_spmi"

#define LED_OUT_CTRL_VAL(id) (~((0x1 << (id * 2)) | (0x1 << (id * 2 + 1))))

#define LED0 		0
#define LED1 		1
#define LED2 		2

#define LEDS_MAX 	3

#define LED_DTS_ATTR_LEN 32

#define DELAY_ON		1
#define DELAY_OFF	0

#define DEL_0		0
#define DEL_500		500
#define DEL_1000		1000
#define DEL_2000		2000
#define DEL_4000		4000
#define DEL_6000		6000
#define DEL_8000		8000
#define DEL_12000		12000
#define DEL_14000		14000
#define DEL_16000		16000

#define DR_DEL00		0x00
#define DR_DEL01		0x01
#define DR_DEL02		0x02
#define DR_DEL03		0x03
#define DR_DEL04		0x04
#define DR_DEL05		0x05
#define DR_DEL06		0x06
#define DR_DEL07		0x07
#define DR_DEL08		0x08
#define DR_DEL09		0x09
#define DR_DELAY_ON	    0xF0

#ifdef CONFIG_HISI_LEDS_BLUE_TP_COLOR_SWITCH
#define BLUE	0x96
#define FLASH_MODE_HAVE_CONFIGERED 0x1

#define DR4_ISET_1MA 0x00
#define DR_TIME_CONFIG1_ON 0x33
#define DR_TIME_CONFIG1_OFF 0x00
#define DR_MODE_SEL_RESET 0x00
#define DR_EN_MODE_345_RESET 0x00
#define DR_START_DEL_512_OFF    0x00 /* start_delay off */
#endif

#define	DR_BRIGHTNESS_HALF	0x1  /* dr3,4,5 half_brightness config */
#define	DR_BRIGHTNESS_FULL	0x7  /* dr3,4,5 full_brightness config */

#define DR_START_DEL_512    0x03 /* start_delay */
#define DR_RISA_TIME        0x33
#define DR_CONTR_DISABLE    0xF8 /* dr3,4,5 disable */

struct led_set_config {
	u8 brightness_set;
	u32 led_iset_address;
	u32 led_start_address;
	u32 led_tim_address;
	u32 led_tim1_address;
	unsigned long led_dr_ctl;
	unsigned long led_dr_out_ctl;
};

struct led_status {
	enum led_brightness brightness;
	unsigned long delay_on;
	unsigned long delay_off;
};

struct led_spmi_data {
	u8 id;

	struct led_classdev ldev;
	struct led_status status;
};

struct led_spmi_drv_data {
	struct mutex 		lock;
	struct clk		*clk;
	void __iomem 	*led_base;
	struct led_spmi_data leds[LEDS_MAX];
};

struct led_spmi {
	const char *name;
	enum led_brightness brightness;
	unsigned long delay_on;
	unsigned long delay_off;
	char *default_trigger;
	unsigned int dr_start_del;
	unsigned int dr_iset;
	unsigned int each_maxdr_iset;
	unsigned int dr_time_config0;
	unsigned int dr_time_config1;
};

struct led_spmi_platform_data {
	struct led_spmi leds[LEDS_MAX];
	unsigned int dr_led_ctrl;
	unsigned int dr_out_ctrl;
	unsigned int max_iset;
	u8 leds_size;
};

#ifdef CONFIG_HISI_LEDS_BLUE_TP_COLOR_SWITCH
struct hisi_led_flash_data {
	unsigned int dr_en_mode_345_address;
	unsigned int flash_period_dr345_address;
	unsigned int flash_on_dr345_address;
	unsigned int dr_mode_sel_address;

	unsigned int dr_en_mode_345_conf;
	unsigned int flash_period_dr345_conf;
	unsigned int flash_on_dr345_conf;
	unsigned int dr_mode_sel_dr4_conf;
};
#endif

#endif /* __LINUX_PMIC_LEDS_H */

