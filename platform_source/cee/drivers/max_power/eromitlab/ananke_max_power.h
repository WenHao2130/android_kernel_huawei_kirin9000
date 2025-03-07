/*
 * ananke_max_power.h
 *
 * little cluster max power buffer interface
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
#ifndef _ANANKE_MAX_POWER_H_
#define _ANANKE_MAX_POWER_H_

#include <linux/types.h>

void ananke_max_power_init(u64 base_addr);
void ananke_max_power(u32 outer_count, u32 inter_count, u32 gpio_addr);
#endif
