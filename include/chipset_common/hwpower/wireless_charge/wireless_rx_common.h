/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_rx_common.h
 *
 * common interface, variables, definition etc of wireless_rx_common.c
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_RX_COMMON_H_
#define _WIRELESS_RX_COMMON_H_

#include <linux/types.h>

enum wlrx_scene {
	WLRX_SCN_BEGIN = 0, /* must be zero here */
	WLRX_SCN_NORMAL = WLRX_SCN_BEGIN,
	WLRX_SCN_LIGHTSTRAP,
	WLRX_SCN_UEM,
	/* new scene must be appended */
	WLRX_SCN_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
enum wlrx_scene wlrx_get_scene(void);
void wireless_charge_icon_display(int crit_type);
void wlrx_cut_off_wired_channel(unsigned int drv_type);
void wlrx_dsm_report(unsigned int drv_type, int err_no,
char *dsm_buff, size_t buff_size);
#else
static inline enum wlrx_scene wlrx_get_scene(void)
{
	return WLRX_SCN_END;
}

static inline void wireless_charge_icon_display(int crit_type)
{
}

static inline void wlrx_cut_off_wired_channel(unsigned int drv_type)
{
}

static inline void wlrx_dsm_report(unsigned int drv_type, int err_no,
	char *dsm_buff, size_t buff_size)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_COMMON_H_ */
