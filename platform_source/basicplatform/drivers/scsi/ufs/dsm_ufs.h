/*
 * dsm_ufs.h
 *
 * deal with dsm_ufs
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef LINUX_DSM_UFS_H
#define LINUX_DSM_UFS_H

#include <linux/time.h>
#ifdef CONFIG_HUAWEI_UFS_DSM
#include <dsm/dsm_pub.h>
#endif
#include "ufs.h"
#include "ufshcd.h"

#ifndef min
	#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#define UFS_DSM_BUFFER_SIZE    2048
#define UFS_MSG_MAX_SIZE        256

#define	DSM_UFS_POWER_DOWN_FLUSH_ERR		925205300

#define	DSM_UFS_FASTBOOT_PWMODE_ERR	    928008000
#define	DSM_UFS_FASTBOOT_RW_ERR		    928008001
#define	DSM_UFS_SYSBUS_ERR		        928008002
#define	DSM_UFS_UIC_TRANS_ERR		    928008003
#define	DSM_UFS_UIC_CMD_ERR		        928008004
#define	DSM_UFS_CONTROLLER_ERR		    928008005
#define	DSM_UFS_DEV_ERR			        928008006
#define	DSM_UFS_TIMEOUT_ERR		        928008007
#define	DSM_UFS_UTP_ERR			        928008008
#define	DSM_UFS_SCSI_CMD_ERR		    928008009
#define	DSM_UFS_VOLT_GPIO_ERR		    928008010
#define	DSM_UFS_LINKUP_ERR		        928008011
#define	DSM_UFS_ENTER_OR_EXIT_H8_ERR	928008012
#define	DSM_UFS_LIFETIME_EXCCED_ERR	    928008013
#define	DSM_UFS_TIMEOUT_SERIOUS		    928008014
#define	DSM_UFS_DEV_INTERNEL_ERR		928008015
#define	DSM_UFS_INTER_INFO_ABNORMAL	928008016
#define	DSM_UFS_INLINE_CRYPTO_ERR		928008017
#define	DSM_UFS_HARDWARE_ERR		928008018
#define	DSM_UFS_LINE_RESET_ERR		928008019
#define	DSM_UFS_TEMP_LOW_ERR		928008020
#define	DSM_UFS_HI1861_INTERNEL_ERR	    928008021

/* Power mode change err ,set bit4 */
#define FASTBOOTDMD_PWR_ERR  BIT(4)
/*  read and write err, set bit15 */
#define FASTBOOTDMD_RW_ERR  BIT(15)
#define TEN_BILLION_BITS (10UL * 1000 * 1000 * 1000)
#define MIN_BER_SAMPLE_BITS 3
#define UIC_ERR_HISTORY_LEN 10
#define UIC_INFO_LEN        4
#define UIC_INFO_CMD_INDEX_1 1
#define UIC_INFO_CMD_INDEX_2 2
#define UIC_INFO_CMD_INDEX_3 3
#define CRYPTO_CONFIG_LEN   32
#define PRODUCT_NAME_LEN    32
#define REV_LEN             5
#define ASC_OFFSET          12
#define ASCQ_OFFSET         13
#define SENSE_KEY_OFFSET    2
#define DELAY_MS_100        100
#define BITS_PER_CNT        (1024 * 2 * 8)
#define MAX_ERR_DESCRIPTION 100
#define SHIFT_32            32
#define DSM_LOG_BUFF_LEFT   256

struct ufs_dsm_log {
	char dsm_log[UFS_DSM_BUFFER_SIZE];
	struct mutex lock; /* mutex */
};
extern struct dsm_client *ufs_dclient;
extern unsigned int ufs_dsm_real_upload_size;

struct ufs_uic_err_history {
	/* unit: ten billion bits */
	unsigned long transfered_bits[UIC_ERR_HISTORY_LEN];
	u32 pos;
	unsigned long delta_err_cnt;
	unsigned long delta_bit_cnt;
};

struct dsm_err_type_category {
	unsigned long err_bit;
	unsigned long err_no;
	char err_desc[MAX_ERR_DESCRIPTION];
};

struct ufs_dsm_adaptor {
	unsigned long err_type;
#define	UFS_FASTBOOT_PWMODE_ERR		0
#define	UFS_FASTBOOT_RW_ERR		1
#define	UFS_SYSBUS_ERR			2
#define	UFS_UIC_TRANS_ERR		3
#define	UFS_UIC_CMD_ERR			4
#define	UFS_CONTROLLER_ERR		5
#define	UFS_DEV_ERR			6
#define	UFS_TIMEOUT_ERR			7
#define	UFS_UTP_TRANS_ERR		8
#define	UFS_SCSI_CMD_ERR		9
#define	UFS_VOLT_GPIO_ERR		10
#define	UFS_LINKUP_ERR			11
#define	UFS_ENTER_OR_EXIT_H8_ERR	12
#define	UFS_LIFETIME_EXCCED_ERR		13
#define	UFS_TIMEOUT_SERIOUS		14
#define	UFS_DEV_INTERNEL_ERR		15
#define	UFS_INTER_INFO_ABNORMAL		16
#define	UFS_INLINE_CRYPTO_ERR		17
#define	UFS_HARDWARE_ERR		18
#define	UFS_LINE_RESET_ERR		19
#define	UFS_TEMP_LOW_ERR		20
#define	UFS_HI1861_INTERNEL_ERR	21
#define	UFS_POWER_DOWN_FLUSH_ERR	22

	/* for UIC Transfer Error */
	unsigned long uic_disable;
	u32 uic_uecpa;
	u32 uic_uecdl;
	u32 uic_uecn;
	u32 uic_uect;
	u32 uic_uedme;
	/* for UIC Command Error */
	u32 uic_info[UIC_INFO_LEN];
	/* for time out error */
	int timeout_slot_id;
	unsigned long outstanding_reqs;
	u64 tr_doorbell;
	unsigned char timeout_scmd[MAX_CDB_SIZE];
	int scsi_status;
	int ocs;
	char sense_buffer[SCSI_SENSE_BUFFERSIZE];
	uint16_t manufacturer_id;
	unsigned long ice_outstanding;
	u64 ice_doorbell;
	u8 lifetime_a;
	u8 lifetime_b;
	u32 pwrmode;
	u32 tx_gear;
	u32 rx_gear;
	int temperature;
};
extern struct ufs_dsm_log g_ufs_dsm_log;

#ifdef CONFIG_HUAWEI_UFS_DSM
int dsm_ufs_update_upiu_info(struct ufs_hba *hba,
				int tag, int err_code);
int dsm_ufs_update_scsi_info(struct ufshcd_lrb *lrbp,
				int scsi_status, int err_code);
int dsm_ufs_update_ocs_info(struct ufs_hba *hba,
				int err_code, int ocs);
int dsm_ufs_update_error_info(struct ufs_hba *hba, int code);
int dsm_ufs_update_uic_info(struct ufs_hba *hba, int err_code);
int dsm_ufs_update_fastboot_info(struct ufs_hba *hba);
void dsm_ufs_update_lifetime_info(u8 lifetime_a, u8 lifetime_b);
int dsm_ufs_get_log(struct ufs_hba *hba, unsigned long code, char *err_msg);
void dsm_ufs_clear_err_type(void);
void dsm_ufs_handle_work(struct work_struct *work);
int dsm_ufs_enabled(void);
void dsm_ufs_enable_uic_err(struct ufs_hba *hba);
int dsm_ufs_disable_uic_err(void);
int dsm_ufs_if_uic_err_disable(void);
unsigned long dsm_ufs_if_err(void);
void dsm_ufs_init(struct ufs_hba *hba);
int dsm_ufs_updata_ice_info(struct ufs_hba *hba);
void dsm_ufs_enable_volt_irq(struct ufs_hba *hba);
void dsm_ufs_disable_volt_irq(struct ufs_hba *hba);
void schedule_ufs_dsm_work(struct ufs_hba *hba);

#define dsm_ufs_log(hba, no, fmt, a...)\
	do {\
		char msg[UFS_MSG_MAX_SIZE];\
		snprintf(msg, UFS_MSG_MAX_SIZE - 1, fmt, ## a);\
		mutex_lock(&g_ufs_dsm_log.lock);\
		if (dsm_ufs_get_log(hba, (no), (msg))) {\
			if (!dsm_client_ocuppy(ufs_dclient)) {\
				dsm_client_copy(ufs_dclient, \
						g_ufs_dsm_log.dsm_log, \
						ufs_dsm_real_upload_size + 1);\
				dsm_client_notify(ufs_dclient, (no));\
			} \
		} \
		mutex_unlock(&g_ufs_dsm_log.lock);\
	} while (0)

#else
static inline int dsm_ufs_update_upiu_info(struct ufs_hba *hba,
				int tag, int err_code) { return 0; }
static inline int dsm_ufs_update_scsi_info(struct ufshcd_lrb *lrbp,
				int scsi_status, int err_code) {return 0; }
static inline int dsm_ufs_update_ocs_info(struct ufs_hba *hba,
				int err_code, int ocs) {return 0; }
static inline int dsm_ufs_update_error_info(struct ufs_hba *hba, int code) {return 0; }
static inline int dsm_ufs_update_uic_info(struct ufs_hba *hba, int err_code) {return 0; }
static inline int dsm_ufs_update_fastboot_info(struct ufs_hba *hba) {return 0; }
static inline void dsm_ufs_update_lifetime_info(u8 lifetime_a, u8 lifetime_b) {return; }
static inline int dsm_ufs_get_log(struct ufs_hba *hba, unsigned long code, char *err_msg) {return 0; }
static inline void dsm_ufs_handle_work(struct work_struct *work) {}
static inline int dsm_ufs_enabled(void) { return 0; }
static inline void dsm_ufs_enable_uic_err(struct ufs_hba *hba){return; };
static inline int dsm_ufs_disable_uic_err(void){return 0; };
static inline int dsm_ufs_if_uic_err_disable(void){return 0; };
static inline void dsm_ufs_init(struct ufs_hba *hba) {}
static inline unsigned long dsm_ufs_if_err(void){return 0;}
static inline int dsm_ufs_updata_ice_info(struct ufs_hba *hba){return 0;}
static inline void dsm_ufs_enable_volt_irq(struct ufs_hba *hba) {return;}
static inline void dsm_ufs_disable_volt_irq(struct ufs_hba *hba) {return;}
static inline void schedule_ufs_dsm_work(struct ufs_hba *hba) {return;}
#define dsm_ufs_log(hba, no, fmt, a...)
#endif
#endif
