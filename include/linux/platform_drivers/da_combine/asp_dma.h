/*
 * asp_dma.h
 *
 * asp dma driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#ifndef __ASP_DMA_H__
#define __ASP_DMA_H__

#include <linux/types.h>

#define ASP_DMA_MAX_CHANNEL_NUM     16
#define ASP_DMA_CPU_NO_AP           0

#define ASP_DMA_INT_STAT(i)         (0x40 * (i))
#define ASP_DMA_INT_TC1(i)          (0x0004 + (0x40 * (i)))
#define ASP_DMA_INT_TC2(i)          (0x0008 + (0x40 * (i)))
#define ASP_DMA_INT_ERR1(i)         (0x000c + (0x40 * (i)))
#define ASP_DMA_INT_ERR2(i)         (0x0010 + (0x40 * (i)))
#define ASP_DMA_INT_ERR3(i)         (0x0014 + (0x40 * (i)))
#define ASP_DMA_INT_TC1_MASK(i)     (0x0018 + (0x40 * (i)))
#define ASP_DMA_INT_TC2_MASK(i)     (0x001c + (0x40 * (i)))
#define ASP_DMA_INT_ERR1_MASK(i)    (0x0020 + (0x40 * (i)))
#define ASP_DMA_INT_ERR2_MASK(i)    (0x0024 + (0x40 * (i)))
#define ASP_DMA_INT_ERR3_MASK(i)    (0x0028 + (0x40 * (i)))
#define ASP_DMA_INT_TC1_RAW         0x0600
#define ASP_DMA_INT_TC2_RAW         0x0608
#define ASP_DMA_INT_ERR1_RAW        0x0610
#define ASP_DMA_INT_ERR2_RAW        0x0618
#define ASP_DMA_INT_ERR3_RAW        0x0620
#define ASP_DMA_SREQ                0x0660
#define ASP_DMA_LSREQ               0x0664
#define ASP_DMA_BREQ                0x0668
#define ASP_DMA_LBREQ               0x066C
#define ASP_DMA_FREQ                0x0670
#define ASP_DMA_LFREQ               0x0674
#define ASP_DMA_CH_PRI              0x0688
#define ASP_DMA_CH_STAT             0x0690
#define ASP_DMA_DMA_CTRL            0x0698
#define ASP_DMA_CX_CURR_CNT1(j)     (0x0700 + (0x10 * (j)))
#define ASP_DMA_CX_CURR_CNT0(j)     (0x0704 + (0x10 * (j)))
#define ASP_DMA_CX_CURR_SRC_ADDR(j) (0x0708 + (0x10 * (j)))
#define ASP_DMA_CX_CURR_DES_ADDR(j) (0x070C + (0x10 * (j)))
#define ASP_DMA_CX_LLI(j)           (0x0800 + (0x40 * (j)))
#define ASP_DMA_CX_BINDX(j)         (0x0804 + (0x40 * (j)))
#define ASP_DMA_CX_CINDX(j)         (0x0808 + (0x40 * (j)))
#define ASP_DMA_CX_CNT1(j)          (0x080C + (0x40 * (j)))
#define ASP_DMA_CX_CNT0(j)          (0x0810 + (0x40 * (j)))
#define ASP_DMA_CX_SRC_ADDR(j)      (0x0814 + (0x40 * (j)))
#define ASP_DMA_CX_DES_ADDR(j)      (0x0818 + (0x40 * (j)))
#define ASP_DMA_CX_CONFIG(j)        (0x081C + (0x40 * (j)))
#define ASP_DMA_CX_AXI_CONF(j)      (0x0820 + (0x40 * (j)))

#define ASP_DMA_INT_STAT_AP         (ASP_DMA_INT_STAT(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_TC1_AP          (ASP_DMA_INT_TC1(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_TC2_AP          (ASP_DMA_INT_TC2(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR1_AP         (ASP_DMA_INT_ERR1(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR2_AP         (ASP_DMA_INT_ERR2(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR3_AP         (ASP_DMA_INT_ERR3(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_TC1_MASK_AP     (ASP_DMA_INT_TC1_MASK(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_TC2_MASK_AP     (ASP_DMA_INT_TC2_MASK(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR1_MASK_AP    (ASP_DMA_INT_ERR1_MASK(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR2_MASK_AP    (ASP_DMA_INT_ERR2_MASK(ASP_DMA_CPU_NO_AP))
#define ASP_DMA_INT_ERR3_MASK_AP    (ASP_DMA_INT_ERR3_MASK(ASP_DMA_CPU_NO_AP))

#define ASP_DMA_LLI_LINK(uwAddr) \
	(((unsigned int)(uintptr_t)(uwAddr) & 0xffffffe0UL) | (0x2UL))

#define RESERVED_MAX_NUM            3
#define DMA_LLI_CFG_ALIGN           32

typedef int (*callback_t)(unsigned short type, unsigned long param,
	unsigned int channel);

enum {
	ASP_DMA_INT_TYPE_TC1 = 0,
	ASP_DMA_INT_TYPE_TC2,
	ASP_DMA_INT_TYPE_ERR1,
	ASP_DMA_INT_TYPE_ERR2,
	ASP_DMA_INT_TYPE_ERR3,
	ASP_DMA_INT_TYPE_BUTT
};

struct dma_lli_cfg {
	unsigned int lli;
	unsigned int reserved[RESERVED_MAX_NUM];
	unsigned int a_count;
	unsigned int src_addr;
	unsigned int des_addr;
	unsigned int config;
} __attribute__((aligned(DMA_LLI_CFG_ALIGN)));

unsigned int dmac_reg_read(unsigned int reg);
int asp_dma_config(unsigned short channel,
	const struct dma_lli_cfg *lli_cfg, callback_t callback,
	unsigned long para);
int asp_dma_start(unsigned short channel, const struct dma_lli_cfg *lli_cfg);
void asp_dma_stop(unsigned short channel);
void asp_dma_suspend(unsigned short channel);
unsigned int asp_dma_get_des(unsigned short channel);
unsigned int asp_dma_get_src(unsigned short channel);

#endif /* __ASP_DMA_H__ */

