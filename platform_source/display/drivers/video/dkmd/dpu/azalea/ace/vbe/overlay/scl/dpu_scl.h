/* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef DPU_SCL_H
#define DPU_SCL_H

#include "../../dpu_fb.h"
#include "../../dpu.h"
#include "../../dpu_fb_struct.h"
#include "../../dpu_enum.h"
#include "../../../include/dpu_communi_def.h"

/* Filter coefficients for SCF */
#define PHASE_NUM 66
#define TAP4 4
#define TAP5 5
#define TAP6 6
#define COEF_LUT_NUM 2

struct coef_lut_tap {
	int row;
	int col;
};

extern int g_scf_lut_chn_coef_idx[DSS_CHN_MAX_DEFINE];
extern const int coef_lut_tap4[SCL_COEF_IDX_MAX][PHASE_NUM][TAP4];
extern const int coef_lut_tap5[SCL_COEF_IDX_MAX][PHASE_NUM][TAP5];
extern const int coef_lut_tap6[SCL_COEF_IDX_MAX][PHASE_NUM][TAP6];

void dpu_scl_init(const char __iomem *scl_base, dss_scl_t *s_scl);
void dpu_scl_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *scl_base, dss_scl_t *s_scl);
int dpu_scl_write_coefs(struct dpu_fb_data_type *dpufd, bool enable_cmdlist,
	char __iomem *addr, const int **p, struct coef_lut_tap lut_tap_addr);
int dpu_scl_coef_on(struct dpu_fb_data_type *dpufd, bool enable_cmdlist, int coef_lut_idx);
int dpu_scl_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_rect_t *aligned_rect, bool rdma_stretch_enable);

#endif /* DPU_SCL_H */
