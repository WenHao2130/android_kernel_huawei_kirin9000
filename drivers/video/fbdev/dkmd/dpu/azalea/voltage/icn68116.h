/*
 * Copyright 2020 HUAWEI Tech. Co., Ltd.
 */

#ifndef __ICN68116_H
#define __ICN68116_H

#define ICN68116_VOL_40     (0x00)     // 4.0V
#define ICN68116_VOL_41     (0x01)     // 4.1V
#define ICN68116_VOL_42     (0x02)     // 4.2V
#define ICN68116_VOL_43     (0x03)     // 4.3V
#define ICN68116_VOL_44     (0x04)     // 4.4V
#define ICN68116_VOL_45     (0x05)     // 4.5V
#define ICN68116_VOL_46     (0x06)     // 4.6V
#define ICN68116_VOL_47     (0x07)     // 4.7V
#define ICN68116_VOL_48     (0x08)     // 4.8V
#define ICN68116_VOL_49     (0x09)     // 4.9V
#define ICN68116_VOL_50     (0x0A)     // 5.0V
#define ICN68116_VOL_51     (0x0B)     // 5.1V
#define ICN68116_VOL_52     (0x0C)     // 5.2V
#define ICN68116_VOL_53     (0x0D)     // 5.3V
#define ICN68116_VOL_54     (0x0E)     // 5.4V
#define ICN68116_VOL_55     (0x0F)     // 5.5V
#define ICN68116_VOL_56     (0x10)     // 5.6V
#define ICN68116_VOL_57     (0x11)     // 5.7V
#define ICN68116_VOL_58     (0x12)     // 5.8V
#define ICN68116_VOL_59     (0x13)     // 5.9V
#define ICN68116_VOL_60     (0x14)     // 6.0V
#define ICN68116_VOL_MAX    (0x15)     // 6.1V

#define ICN68116_REG_VPOS   (0x00)
#define ICN68116_REG_VNEG   (0x01)
#define ICN68116_REG_APP_DIS   (0x03)

#define ICN68116_REG_VOL_MASK  (0x1F)
#define ICN68116_APPS_BIT   (1 << 6)
#define ICN68116_DISP_BIT   (1 << 1)
#define ICN68116_DISN_BIT   (1 << 0)

struct icn68116_voltage {
	u32 voltage;
	int value;
};

static struct icn68116_voltage vol_table[] = {
	{4000000, ICN68116_VOL_40},
	{4100000, ICN68116_VOL_41},
	{4200000, ICN68116_VOL_42},
	{4300000, ICN68116_VOL_43},
	{4400000, ICN68116_VOL_44},
	{4500000, ICN68116_VOL_45},
	{4600000, ICN68116_VOL_46},
	{4700000, ICN68116_VOL_47},
	{4800000, ICN68116_VOL_48},
	{4900000, ICN68116_VOL_49},
	{5000000, ICN68116_VOL_50},
	{5100000, ICN68116_VOL_51},
	{5200000, ICN68116_VOL_52},
	{5300000, ICN68116_VOL_53},
	{5400000, ICN68116_VOL_54},
	{5500000, ICN68116_VOL_55},
	{5600000, ICN68116_VOL_56},
	{5700000, ICN68116_VOL_57},
	{5800000, ICN68116_VOL_58},
	{5900000, ICN68116_VOL_59},
	{6000000, ICN68116_VOL_60}
};


struct icn68116_device_info {
	struct device *dev;
	struct i2c_client *client;
};

struct work_data {
	struct i2c_client *client;
	struct delayed_work setvol_work;
	int vpos;
	int vneg;
};

struct icn68116_configure_info {
	char *lcd_name;
	int vpos_cmd;
	int vneg_cmd;
};

bool check_icn68116_device(void);
int icn68116_set_voltage(void);

#ifdef CONFIG_DPU_FB_6250
extern int is_normal_lcd(void);
extern int get_vsp_voltage(void);
extern int get_vsn_voltage(void);
#endif

#endif
