/*
 * deimos_max_power_wfe.h
 *
 * cs middle&big cluster wfe max power interface
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
#ifndef _DEIMOS_MAX_POWER_H_
#define _DEIMOS_MAX_POWER_H_

#include <linux/types.h>

void deimos_max_power_pi_wfe(u64 gpio_addr);
#endif
