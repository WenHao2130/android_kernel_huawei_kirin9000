/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: kernel_clt_flag impliment file
 * Create: 2016-07-10
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>

#ifdef CLT_CAMERA
#define IS_CLT_FLAG 1
#else
#define IS_CLT_FLAG 0
#endif

int kernel_is_clt_flag(void)
{
	return IS_CLT_FLAG;
}
