/*
 * maxim_common.h
 *
 * maxim common function head
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _MAXIM_ONEWIRE_H_
#define _MAXIM_ONEWIRE_H_

#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <huawei_platform/power/batt_info_pub.h>
#include "../onewire/onewire_common.h"
#include "../batt_aut_checker.h"

/* random challenge length */
#define RANDOM_NUM_BYTES              32
/* Command success response */
#define MAXIM_ONEWIRE_COMMAND_SUCCESS 0xAA

/* CRC result */
#define MAXIM_CRC16_RESULT            0xB001
#define MAXIM_CRC8_RESULT             0

/* sn config */
#define SN_LENGTH_BITS                108
#define PRINTABLE_SN_SIZE             17
#define SN_CHAR_PRINT_SIZE            11
#define SN_HEX_PRINT_SIZE             5

#define BATTERY_CELL_FACTORY          4
#define BATTERY_PACK_FACTORY          5

/* basic */
#define bytes2bits(x)                 ((x) << 3)
#define is_odd(a)                     ((a) & 0x1)
#define is_even(a)                    (!is_odd(a))
#define not_muti8(a)                  ((a) & 0x7)
#define is_muti8(a)                   (!not_muti8(a))
#define double(x)                     ((x) << 1)

/* MAXIM 1-wire rom function command */
#define READ_ROM                      0x33
#define MATCH_ROM                     0x55
#define SEARCH_ROM                    0xF0
#define SKIP_ROM                      0xCC
#define RESUME_COMMAND                0xA5

/* 1-wire function operation return signals */
#define MAXIM_ONEWIRE_SUCCESS         ONEWIRE_SUCCESS
#define MAXIM_ERROR_BIT               0x80
#define MAXIM_ONEWIRE_DTS_ERROR       (MAXIM_ERROR_BIT | ONEWIRE_DTS_FAIL)
#define MAXIM_ONEWIRE_COM_ERROR       (MAXIM_ERROR_BIT | ONEWIRE_COM_FAIL)
#define MAXIN_ONEWIRE_CRC8_ERROR      (MAXIM_ERROR_BIT | ONEWIRE_CRC8_FAIL)
#define MAXIN_ONEWIRE_CRC16_ERROR     (MAXIM_ERROR_BIT | ONEWIRE_CRC16_FAIL)
#define MAXIM_ONEWIRE_NO_SLAVE        (MAXIM_ERROR_BIT | ONEWIRE_NO_SLAVE)
#define MAXIM_ONEWIRE_ILLEGAL_PAR     (MAXIM_ERROR_BIT | ONEWIRE_ILLEGAL_PARAM)

/* retry times config */
#define SET_SRAM_RETRY                4
#define GET_USER_MEMORY_RETRY         8
#define GET_PERSONALITY_RETRY         8
#define GET_ROM_ID_RETRY              8
#define GET_BLOCK_STATUS_RETRY        8
#define SET_BLOCK_STATUS_RETRY        8
#define GET_MAC_RETRY                 8
#define CURRENT_MAXIM_TASK            0

/* hmac */
#define MAX_MAC_SOURCE_SIZE           128

/* maxim return value */
#define MAXIM_SUCCESS                 0
#define MAXIM_FAIL                    (-1)

/*
 * for ds28el16 1page == 1block
 * for ds28el15 1page == 2block
 */
struct maxim_onewire_mem {
	unsigned char *block_status;
	unsigned char *rom_id;
	unsigned char *pages;
	unsigned char *mac;
	unsigned char *sn;
	unsigned char *mac_datum;
	unsigned char main_id;
	unsigned char version;
	unsigned int ic_type;
	unsigned int rom_id_length;
	unsigned int page_number;
	unsigned int page_size;
	unsigned int block_number;
	unsigned int block_size;
	unsigned int mac_length;
	unsigned int sn_length_bits;
	unsigned int sn_page;
	unsigned int sn_offset_bits;
	unsigned char batt_type[BATTERY_TYPE_BUFF_SIZE];
	struct mutex lock; /* protocol process mutual exclusion */
};

struct maxim_onewire_time_request {
	struct ow_treq ow_trq; /* dts node--read-time-request */
	unsigned int t_read;
	unsigned int t_write_memory; /* dts node--programe-time */
	unsigned int t_write_status; /* dts node--write-status-time */
	unsigned int t_compute; /* dts node--compute-mac-time */
};

struct maxim_onewire_des {
	struct maxim_onewire_time_request trq;
	struct maxim_onewire_mem memory;
	struct ow_phy_ops phy_ops;
};

struct maxim_onewire_com_stat {
	unsigned int cmds;
	unsigned int *totals;
	unsigned int *errs;
	const char * const *cmd_str;
};

struct battery_constraint {
	unsigned char *id_mask;
	unsigned char *id_example;
	unsigned char *id_chk;
};

struct maxim_onewire_drv_data {
	struct maxim_onewire_des ic_des;
	struct battery_constraint batt_cons;
	struct maxim_onewire_com_stat com_stat;
	struct attribute_group *attr_group;
	const struct attribute_group **attr_groups;
	struct wakeup_source *write_lock;
	struct mutex batt_safe_info_lock;
	struct batt_ct_ops_list ct_node;
	unsigned char random_bytes[RANDOM_NUM_BYTES];
};

#define put_mac_data(drv_data, ic_mac) \
do { \
	memcpy((drv_data)->ic_des.memory.mac + 1, \
		(ic_mac), (drv_data)->ic_des.memory.mac_length); \
	*((drv_data)->ic_des.memory.mac) = 1; \
} while (0)

#define get_mac_data(drv_data)        ((drv_data)->ic_des.memory.mac + 1)
#define get_mac_data_len(drv_data)    ((drv_data)->ic_des.memory.mac_length)
#define check_mac_data_sign(drv_data) (1 == *((drv_data)->ic_des.memory.mac))
#define clear_mac_data_sign(drv_data) (*((drv_data)->ic_des.memory.mac) = 0)
#define set_mac_data_sign(drv_data)   (*((drv_data)->ic_des.memory.mac) = 1)

#define put_rom_id(drv_data, id) \
do { \
	memcpy((drv_data)->ic_des.memory.rom_id + 1, \
		id, (drv_data)->ic_des.memory.rom_id_length); \
	*((drv_data)->ic_des.memory.rom_id) = 1; \
} while (0)

#define get_rom_id(drv_data)          ((drv_data)->ic_des.memory.rom_id + 1)
#define get_rom_id_len(drv_data)      ((drv_data)->ic_des.memory.rom_id_length)
#define check_rom_id_sign(drv_data)   (1 == *((drv_data)->ic_des.memory.rom_id))
#define clear_rom_id_sign(drv_data)   (*((drv_data)->ic_des.memory.rom_id) = 0)
#define set_rom_id_sign(drv_data)     (*((drv_data)->ic_des.memory.rom_id) = 1)

#define put_block_status(drv_data, id) \
do { \
	memcpy((drv_data)->ic_des.memory.block_status + 1, \
		id, (drv_data)->ic_des.memory.block_number); \
	*((drv_data)->ic_des.memory.block_status) = 1; \
} while (0)

#define put_one_block_status(drv_data, no, status) \
do { \
	*((drv_data)->ic_des.memory.block_status + 1 + (no)) = status; \
	*((drv_data)->ic_des.memory.block_status) = 1; \
} while (0)

#define get_block_status(drv_data)    ((drv_data)->ic_des.memory.block_status + 1)
#define check_block_status_sign(drv_data) \
	(*((drv_data)->ic_des.memory.block_status) == 1)
#define clear_block_status_sign(drv_data) \
	(*((drv_data)->ic_des.memory.block_status) = 0)
#define set_block_status_sign(drv_data) \
	(*((drv_data)->ic_des.memory.block_status) = 1)

#define get_block_num(drv_data)       ((drv_data)->ic_des.memory.block_number)
#define get_page_num(drv_data)        ((drv_data)->ic_des.memory.page_number)

#define get_page_size(drv_data)       ((drv_data)->ic_des.memory.page_size)
#define get_page_data(drv_data, page_no) \
	((((get_page_size(drv_data) + 1) * (page_no)) + \
	(drv_data)->ic_des.memory.pages) + 1)
#define put_page_data(drv_data, page_no, data) do { \
	memcpy(get_page_data(drv_data, page_no), data, \
		get_page_size(drv_data)); \
	*(get_page_data(drv_data, page_no) - 1) = 1; \
} while (0)
#define check_page_data_sign(drv_data, page_no) \
	(*(get_page_data(drv_data, page_no) - 1) == 1)
#define clear_page_data_sign(drv_data, page_no) \
	(*(get_page_data(drv_data, page_no) - 1) = 0)
#define set_page_data_sign(drv_data, page_no) \
	(*(get_page_data(drv_data, page_no) - 1) = 1)

#define put_man_id(drv_data, id) \
	((drv_data)->ic_des.memory.main_id = (id))
#define get_main_id(drv_data)         ((drv_data)->ic_des.memory.main_id)
#define put_version(drv_data, ver) \
	((drv_data)->ic_des.memory.version = (ver))
#define get_version(drv_data)         ((drv_data)->ic_des.memory.version)

#define incr_total_com_stat(drv_data, index) \
do { \
	if ((drv_data)->com_stat.totals && \
		((index) < (drv_data)->com_stat.cmds)) \
		((drv_data)->com_stat.totals[index])++; \
} while (0)

#define incr_err_com_stat(drv_data, index) \
do { \
	if ((drv_data)->com_stat.errs && \
		((index) < (drv_data)->com_stat.cmds)) \
		((drv_data)->com_stat.errs[index])++; \
} while (0)

/* Slave presence signal is low */
#define no_slave_response(x)          ((x) != 0)

/* onewire communication error process */
#define maxim_onewire_communication_info(x, y) \
do { \
	if (x) \
		hwlog_info(y" failed %x in %s\n", x, __func__); \
} while (0)

unsigned char check_crc8(unsigned char *data, int length);
unsigned short check_crc16(unsigned char *check_data, int length);
int snprintf_array(unsigned char *buf, int buf_len,
	unsigned char *array, int arry_len);
void set_sched_affinity_to_current(void);
void set_sched_affinity_to_all(void);

int maxim_check_rom_id_format(struct maxim_onewire_drv_data *drv_data);
int maxim_drv_data_init(struct maxim_onewire_drv_data *drv_data,
	struct platform_device *pdev, int cmds, const char * const cmd_str[]);
int maxim_dev_sys_node_init(struct maxim_onewire_drv_data *drv_data,
	struct platform_device *pdev, const struct attribute **attrs);
void maxim_destroy_drv_data(struct maxim_onewire_drv_data *ds_drv,
	struct platform_device *pdev);
void maxim_parise_printable_sn(unsigned char *page,
	unsigned int sn_offset_bits, unsigned char *sn);

#endif /* _MAXIM_ONEWIRE_H_ */
