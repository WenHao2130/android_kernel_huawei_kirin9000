/*
 * tps65132.h
 *
 * tps65132 bias driver head file
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef __TPS65132_H
#define __TPS65132_H

#define TPS65132_VOL_40 0x00 // 4.0V
#define TPS65132_VOL_41 0x01 // 4.1V
#define TPS65132_VOL_42 0x02 // 4.2V
#define TPS65132_VOL_43 0x03 // 4.3V
#define TPS65132_VOL_44 0x04 // 4.4V
#define TPS65132_VOL_45 0x05 // 4.5V
#define TPS65132_VOL_46 0x06 // 4.6V
#define TPS65132_VOL_47 0x07 // 4.7V
#define TPS65132_VOL_48 0x08 // 4.8V
#define TPS65132_VOL_49 0x09 // 4.9V
#define TPS65132_VOL_50 0x0A // 5.0V
#define TPS65132_VOL_51 0x0B // 5.1V
#define TPS65132_VOL_52 0x0C // 5.2V
#define TPS65132_VOL_53 0x0D // 5.3V
#define TPS65132_VOL_54 0x0E // 5.4V
#define TPS65132_VOL_55 0x0F // 5.5V
#define TPS65132_VOL_56 0x10 // 5.6V
#define TPS65132_VOL_57 0x11 // 5.7V
#define TPS65132_VOL_58 0x12 // 5.8V
#define TPS65132_VOL_59 0x13 // 5.9V
#define TPS65132_VOL_60 0x14 // 6.0V

#define TPS65132_REG_VPOS 0x00
#define TPS65132_REG_VNEG 0x01
#define TPS65132_REG_APP_DIS 0x03
#define TPS65132_REG_CTL 0xFF
#define AW37503_REG_VENDORID 0x04
#define AW37503_VALUE_APP_DIS 0x43
#define AW37503_ENABLE_FLAG 0x01

#define TPS65132_REG_VOL_MASK 0x1F
#define TPS65312_APPS_BIT   (1 << 6)
#define TPS65132_DISP_BIT   (1 << 1)
#define TPS65132_DISN_BIT   (1 << 0)
#define TPS65132_WED_BIT    (1 << 7) // write enable bit

struct tps65132_voltage {
	u32 voltage;
	int value;
};

static struct tps65132_voltage vol_table[] = {
	{ 4000000, TPS65132_VOL_40 },
	{ 4100000, TPS65132_VOL_41 },
	{ 4200000, TPS65132_VOL_42 },
	{ 4300000, TPS65132_VOL_43 },
	{ 4400000, TPS65132_VOL_44 },
	{ 4500000, TPS65132_VOL_45 },
	{ 4600000, TPS65132_VOL_46 },
	{ 4700000, TPS65132_VOL_47 },
	{ 4800000, TPS65132_VOL_48 },
	{ 4900000, TPS65132_VOL_49 },
	{ 5000000, TPS65132_VOL_50 },
	{ 5100000, TPS65132_VOL_51 },
	{ 5200000, TPS65132_VOL_52 },
	{ 5300000, TPS65132_VOL_53 },
	{ 5400000, TPS65132_VOL_54 },
	{ 5500000, TPS65132_VOL_55 },
	{ 5600000, TPS65132_VOL_56 },
	{ 5700000, TPS65132_VOL_57 },
	{ 5800000, TPS65132_VOL_58 },
	{ 5900000, TPS65132_VOL_59 },
	{ 6000000, TPS65132_VOL_60 },
};

struct tps65132_device_info {
	struct device *dev;
	struct i2c_client *client;
};

struct work_data {
	struct i2c_client *client;
	struct delayed_work setvol_work;
	int vpos;
	int vneg;
};

struct tps65132_configure_info {
	char *lcd_name;
	int vpos_cmd;
	int vneg_cmd;
};

#ifdef CONFIG_DPU_FB_6250
extern int is_normal_lcd(void);
extern int get_vsp_voltage(void);
extern int get_vsn_voltage(void);
#endif

#ifdef CONFIG_LCD_KIT_DRIVER
int tps65132_dbg_set_bias_for_dpu(int vpos, int vneg);
#endif

#endif
