/*
 * product.c
 *
 * This is for product proc interface.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "vfmw_pdt.h"
#include "dbg.h"
#include "vfmw_sys.h"

struct vfmw_global_info g_vdh_glb_reg;

struct vfmw_global_info *pdt_get_glb_info(void)
{
	return &g_vdh_glb_reg;
}

int32_t pdt_init(void *args, struct pdt_config *cfg, void *dev)
{
	int32_t i;
	uint8_t *vir_addr = NULL;
	uint32_t rst_req, rst_ok;
	int32_t ret = 0;

	vfmw_module_reg_info *reg_info = (vfmw_module_reg_info *)args;
	struct vfmw_global_info *glb_info = pdt_get_glb_info();

	glb_info->crg_reg_phy_addr = reg_info->reg_phy_addr;
	glb_info->crg_reg_size = reg_info->reg_range;
	glb_info->is_fpga = cfg->is_fpga;
	glb_info->dev = dev;

	vir_addr = (uint8_t *)OS_KMAP_REG(glb_info->crg_reg_phy_addr, glb_info->crg_reg_size);
	if (!vir_addr) {
		dprint(PRN_ERROR, "map crg reg failed\n");
		return VCODEC_FAILURE;
	}

	glb_info->crg_reg_vaddr = vir_addr;
	if (get_current_sec_mode() == SEC_MODE)
		return 0;

	if (!cfg->is_smmu_bypass) {
		rd_vreg(vir_addr, VDH_SRST_REQ_REG_OFS, rst_req);
		((struct vcrg_vdh_srst_req *)(&rst_req))->vdh_all_srst_req = 1;
		wr_vreg(vir_addr, VDH_SRST_REQ_REG_OFS, rst_req);
		OS_MB();

		for (i = 0; i < 1000; i++) { // 1000ms
			OS_UDELAY(1);
			rd_vreg(vir_addr, VDH_SRST_OK_REG_OFS, rst_ok);
			if (((struct vcrg_vdh_srst_ok *)(&rst_ok))->vdh_all_srst_ok == 1) {
				dprint(PRN_ALWS, "dec glb reset success\n");
				ret = 0;
				break;
			}
		}
		if (i == 1000) { // over 1000 times
			dprint(PRN_ERROR, "dec glb reset failed\n");
			ret = VCODEC_FAILURE;
		}

		rd_vreg(vir_addr, VDH_SRST_REQ_REG_OFS, rst_req);
		((struct vcrg_vdh_srst_req *)(&rst_req))->vdh_all_srst_req = 0;
		wr_vreg(vir_addr, VDH_SRST_REQ_REG_OFS, rst_req);
	}
	return ret;
}

void pdt_deinit(void)
{
	struct vfmw_global_info *glb_info = pdt_get_glb_info();

	if (glb_info->crg_reg_vaddr) {
		OS_KUNMAP_REG(glb_info->crg_reg_vaddr);
		glb_info->crg_reg_vaddr = VCODEC_NULL;
	}

	glb_info->crg_reg_phy_addr = 0;
	glb_info->crg_reg_size = 0;
}

