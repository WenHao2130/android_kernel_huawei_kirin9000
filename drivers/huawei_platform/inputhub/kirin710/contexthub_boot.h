/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: Sensor Hub Channel Bridge
 * Author: huangjisong
 * Create: 2013-3-10
 */

#ifndef __LINUX_INPUTHUB_CMU_H__
#define __LINUX_INPUTHUB_CMU_H__

#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/types.h>

#include <iomcu_ddr_map.h>

#include "contexthub_recovery.h"

#define STARTUP_IOM3_CMD                     0x00070001
#define RELOAD_IOM3_CMD                      0x0007030D

#define IOMCU_CONFIG_SIZE                    DDR_CONFIG_SIZE
#define IOMCU_CONFIG_START                   DDR_CONFIG_ADDR_AP

#define SENSOR_MAX_RESET_TIME_MS             400
#define SENSOR_DETECT_AFTER_POWERON_TIME_MS  50
#define SENSOR_POWER_DO_RESET                0
#define SENSOR_POWER_NO_RESET                1
#define SENSOR_REBOOT_REASON_MAX_LEN         32

#define WARN_LEVEL                           2
#define INFO_LEVEL                           3
#define LG_TPLCD                             1
#define JDI_TPLCD                            2
#define KNIGHT_BIEL_TPLCD                    3
#define KNIGHT_LENS_TPLCD                    4
#define BOE_TPLCD                            5
#define EBBG_TPLCD                           6
#define INX_TPLCD                            7
#define SAMSUNG_TPLCD                        8
#define SHARP_TPLCD                          9
#define BIEL_TPLCD                           10
#define VITAL_TPLCD                          11
#define TM_TPLCD                             12
#define AUO_TPLCD                            13
#define TCL_TPLCD                            14
#define CMI_TPLCD                            15
#define CTC_TPLCD                            16
#define INX_TPLCD2                           17
#define EDO_TPLCD                            18

#define DTS_COMP_LG_ER69006A        "hisilicon,mipi_lg_eR69006A"
#define DTS_COMP_JDI_NT35695_CUT3_1 "hisilicon,mipi_jdi_NT35695_cut3_1"
#define DTS_COMP_JDI_NT35695_CUT2_5 "hisilicon,mipi_jdi_NT35695_cut2_5"

#define DTS_COMP_LG_ER69007         "hisilicon,mipi_lg_eR69007"
#define DTS_COMP_SHARP_NT35597      "hisilicon,mipi_sharp_knt_NT35597"
#define DTS_COMP_LG_ER69006_FHD     "hisilicon,mipi_lg_eR69006_FHD"
#define DTS_COMP_SHARP_NT35695      "hisilicon,mipi_sharp_NT35695_5p7"
#define DTS_COMP_MIPI_BOE_ER69006   "hisilicon,mipi_boe_ER69006_5P7"

#define DTS_COMP_BOE_OTM1906C       "hisilicon,boe_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_EBBG_OTM1906C      "hisilicon,ebbg_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_INX_OTM1906C       "hisilicon,inx_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_JDI_NT35695        "hisilicon,jdi_nt35695_5p2_1080p_cmd"
#define DTS_COMP_LG_R69006          "hisilicon,lg_r69006_5p2_1080p_cmd"
#define DTS_COMP_SAMSUNG_S6E3HA3X02 "hisilicon,mipi_samsung_S6E3HA3X02"

#define DTS_COMP_LG_R69006_5P2      "hisilicon,mipi_lg_R69006_5P2"
#define DTS_COMP_SHARP_NT35695_5P2  "hisilicon,mipi_sharp_NT35695_5P2"
#define DTS_COMP_JDI_R63452_5P2     "hisilicon,mipi_jdi_R63452_5P2"

#define DTS_COMP_SAM_WQ_5P5  "hisilicon,mipi_samsung_S6E3HA3X02_5P5_AMOLED"
#define DTS_COMP_SAM_FHD_5P5 "hisilicon,mipi_samsung_D53G6EA8064T_5P5_AMOLED"

#define DTS_COMP_JDI_R63450_5P7     "hisilicon,mipi_jdi_duke_R63450_5P7"
#define DTS_COMP_SHARP_DUKE_NT35597 "hisilicon,mipi_sharp_duke_NT35597"

#define DTS_COMP_AUO_OTM1901A_5P2   "auo_otm1901a_5p2_1080p_video"
#define DTS_COMP_AUO_TD4310_5P2     "auo_td4310_5p2_1080p_video"
#define DTS_COMP_TM_FT8716_5P2      "tm_ft8716_5p2_1080p_video"
#define DTS_COMP_EBBG_NT35596S_5P2  "ebbg_nt35596s_5p2_1080p_video"
#define DTS_COMP_JDI_ILI7807E_5P2   "jdi_ili7807e_5p2_1080p_video"

#define DTS_COMP_BOE_NT36682A_6P59  "boe_nt36682a_6p59_1080p_video"
#define DTS_COMP_INX_NT36682A       "inx_nt36682a_6p59_1080p_video"
#define DTS_COMP_TCL_NT36682A       "tcl_nt36682a_6p59_1080p_video"
#define DTS_COMP_TM_NT36682A        "tm_nt36682a_6p59_1080p_video"
#define DTS_COMP_BOE_TD4320_6P59    "boe_td4320_6p59_1080p_video"
#define DTS_COMP_TM_TD4321_6P59     "tm_td4321_6p59_1080p_video"

#define DTS_COMP_CMI_NT36682A       "cmi_nt36682a_6p15_1080p_video"
#define DTS_COMP_JDI_TD4320         "jdi_td4320_6p15_1080p_video"
#define DTS_COMP_BOE_TD4320         "boe_td4320_6p15_1080p_video"
#define DTS_COMP_BOE_NT36682A       "boe_nt36682a_6p15_1080p_video"
#define DTS_COMP_TM_TD4320          "tm_td4320_6p15_1080p_video"
#define DTS_COMP_TM_TD4330          "tm_td4330_6p15_1080p_video"
#define DTS_COMP_TM_NT36682A_6P15   "tm_nt36682a_6p15_1080p_video"

#define DTS_COMP_AUO_TD4320_6P4     "auo_td4320_6p4_1080p_video"
#define DTS_COMP_BOE_TD4320_6P4     "boe_td4320_6p4_1080p_video"
#define DTS_COMP_CTC_NT36682A_6P4   "ctc_nt36682a_6p4_1080p_video"
#define DTS_COMP_TM_NT36682A_6P4    "tm_nt36682a_6p4_1080p_video"

#define DTS_COMP_TM_FT8615_6P39    "tm_ft8615_6p39_720p_video"
#define DTS_COMP_BOE_FT8009_6P39   "boe_ft8009_6p39_720p_video"
#define DTS_COMP_BOE_FT8615_6P39   "boe_ft8615_6p39_720p_video"

#define DTS_COMP_INX_NT36572A_6P39 "inx_nt36572a_6p39_720p_video"  /* LTPS */
#define DTS_COMP_INX_NT36526_6P39  "inx_nt36526_6p39_720p_video"   /* A-SI */
#define DTS_COMP_TM_FT8615_G6_6P39 "tm_ft8615_g6_6p39_720p_video"

#define DTS_COMP_SAMSUNG_AMS653VY01 "090_703_6p53"
#define DTS_COMP_EDO_E652FAC73 "350_c03_6p53"

#define LCD_PANEL_NAME_LEN 32

enum SENSOR_POWER_CHECK {
	SENSOR_POWER_STATE_OK = 0,
	SENSOR_POWER_STATE_INIT_NOT_READY,
	SENSOR_POWER_STATE_CHECK_ACTION_FAILED,
	SENSOR_POWER_STATE_CHECK_RESULT_FAILED,
	SENSOR_POWER_STATE_NOT_PMIC,
};

typedef struct {
	u32 mutex;
	u16 index;
	u16 pingpang;
	u32 buff;
	u32 ddr_log_buff_cnt;
	u32 ddr_log_buff_index;
	u32 ddr_log_buff_last_update_index;
} log_buff_t;

typedef struct wrong_wakeup_msg {
	u32 flag;
	u64 time;
	u32 irq0;
	u32 irq1;
	u32 recvfromapmsg[4];
	u32 recvfromlpmsg[4];
	u32 sendtoapmsg[4];
	u32 sendtolpmsg[4];
	u32 recvfromapmsgmode;
	u32 recvfromlpmsgmode;
	u32 sendtoapmsgmode;
	u32 sendtolpmsgmode;
} wrong_wakeup_msg_t;

enum dump_loc {
	DL_NONE = 0,
	DL_TCM,
	DL_EXT,
	DL_BOTTOM = DL_EXT,
};

enum DUMP_REGION {
	DE_TCM_CODE,
	DE_DDR_CODE,
	DE_DDR_DATA,
	DE_BOTTOM = 16,
};

struct dump_region_config {
	u8 loc;
	u8 reserved[3];
};

struct dump_config {
	u64 dump_addr;
	u32 dump_size;
	u64 ext_dump_addr;
	u32 ext_dump_size;
	u8 enable;
	u8 finish;
	u8 reason;
	struct dump_region_config elements[DE_BOTTOM];
};

struct timestamp_kernel {
	u64 syscnt;
	u64 kernel_ns;
};

struct timestamp_iomcu_base {
	u64 syscnt;
	u64 kernel_ns;
	u32 timer32k_cyc;
};

typedef struct {
	const char *dts_comp_mipi;
	uint8_t tplcd;
} lcd_module;

typedef struct {
	const char *dts_comp_lcd_model;
	uint8_t tplcd;
} lcd_model;

typedef struct _lcd_model_info_config {
	char panel_name[LCD_PANEL_NAME_LEN];
	int16_t manufacture;
}lcd_panel_model;

struct config_on_ddr {
	struct dump_config dump_config;
	log_buff_t log_buff_cb_backup;
	wrong_wakeup_msg_t wrong_wakeup_msg;
	u32 log_level;
	float gyro_offset[3];
	u8 modem_open_app_state;
	u64 reserved;
	struct timestamp_kernel timestamp_base;
	struct timestamp_iomcu_base timestamp_base_iomcu;
};

extern int (*api_inputhub_mcu_recv)(const char *buf, unsigned int length);
extern int (*api_mculog_process)(const char *buf, unsigned int length);
extern struct config_on_ddr *g_p_config_on_ddr;
extern struct completion iom3_reboot;
extern rproc_id_t ipc_ap_to_iom_mbx;
#ifdef CONFIG_HUAWEI_DSM
extern struct dsm_client *shb_dclient;
#endif
void write_timestamp_base_to_sharemem(void);
int get_sensor_mcu_mode(void);
int is_sensorhub_disabled(void);
void peri_used_request(void);
void peri_used_release(void);
#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *inputhub_get_shb_dclient(void);
#endif
u8 get_tplcd_manufacture(void);
u8 get_tplcd_manufacture_curr(void);
struct dsm_dev *get_dsm_sensorhub(void);
#endif /* __LINUX_INPUTHUB_CMU_H__ */
