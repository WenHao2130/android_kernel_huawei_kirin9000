/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __DPTX_DP_DSC_CHECK_H__
#define __DPTX_DP_DSC_CHECK_H__

struct dp_ctrl;

struct slice_count_map {
	int ppr_left_boundary;
	int ppr_right_boundary;
	int slice_count;
};

int dptx_dsc_initial(struct dp_ctrl *dptx);
void dptx_dsc_check_rx_cap(struct dp_ctrl *dptx);
void dptx_dsc_para_init(struct dp_ctrl *dptx);
#endif
