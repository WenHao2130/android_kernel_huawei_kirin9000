/*
 *
 * bbox diaginfo main module for bbox diaginfo.
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
#ifndef __DFX_BBOX_DIAGINFO_H__
#define __DFX_BBOX_DIAGINFO_H__
#include <bbox_diaginfo_id_def.h>
#include <linux/types.h>

#define BBOX_DIAGINFO_OK              0
#define BBOX_DIAGINFO_INVALIDPAR    (-1)
#define BBOX_DIAGINFO_REPEATMSG     (-2)
#define BBOX_DIAGINFO_STR_MSG_ERR   (-3)
#define BBOX_DIAGINFO_NO_MEM        (-4)
#define BBOX_DIAGINFO_AP_SR         (-5)
#define BBOX_DIAGINFO_INV_ID        (-6)
#define BBOX_DIAGINFO_OVER_COUNT    (-7)

#define DIAGINFO_STRING_MAX_LEN     256
#define DIAGINFO_COUNT_MAX           10

#ifdef CONFIG_DFX_BB_DIAGINFO
int bbox_diaginfo_exception_save2fs(void);
void mntn_ipc_msg_nb(unsigned int *msg, u32 msg_len);
void bbox_ap_ipc_init(void);
int bbox_diaginfo_init(void);
int bbox_lpmcu_diaginfo_init(void);
int bbox_diaginfo_register(unsigned int err_id, const char *date, const char *pdata, unsigned int data_len, u64 ts);
int bbox_diaginfo_record(unsigned int err_id, const char *date, const char *fmt, ...);
void cpu_up_diaginfo_record(unsigned int cpu, int status);
void bbox_diaginfo_dump_lastmsg(void);
void create_dfx_diaginfo_log_file(void);
#ifdef CONFIG_MNTN_DIAGINFO_SERVER
int diaginfo_server_init(void);
#endif
#else
static inline int bbox_diaginfo_exception_save2fs(void)
{
	return 0;
};

static inline int bbox_diaginfo_init(void)
{
	return 0;
};

static inline int bbox_diaginfo_record(unsigned int err_id, const char *date, const char *fmt, ...)
{
	return 0;
};

static inline void mntn_ipc_msg_nb(unsigned int *msg, u32 msg_len)
{
	return;
};

static inline void cpu_up_diaginfo_record(unsigned int cpu, int status)
{
	return;
};

static inline void bbox_diaginfo_dump_lastmsg(void)
{
	return;
};

static inline void create_dfx_diaginfo_log_file(void)
{
	return;
};
#endif

#endif
