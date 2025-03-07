/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: define jpeg address offset.
 * Create: 2012-12-22
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

#ifndef __HJPG_REG_OFFSET_H__
#define __HJPG_REG_OFFSET_H__

/*****************************************************************************/
/*                      ISP JPGENC Registers' Definitions              */
/*****************************************************************************/

/* Encode enable */
#define JPGENC_JPE_ENCODE_REG            0x4
/* SW force update update */
#define JPGENC_JPE_INIT_REG              0x8
/* JPEG codec horizontal image size for encoding */
#define JPGENC_JPE_ENC_HRIGHT1_REG       0x18
/* JPEG codec vertical image size for encoding */
#define JPGENC_JPE_ENC_VBOTTOM_REG       0x1C
/* JPEG picture encoding format */
#define JPGENC_JPE_PIC_FORMAT_REG        0x20
/* restart marker insertion register */
#define JPGENC_JPE_RESTART_INTERVAL_REG  0x24
/* Q-table selector 0, quant table for Y component */
#define JPGENC_JPE_TQ_Y_SELECT_REG       0x28
/* Q-table selector 1, quant table for U component */
#define JPGENC_JPE_TQ_U_SELECT_REG       0x2C
/* Q-table selector 2, quant table for V component */
#define JPGENC_JPE_TQ_V_SELECT_REG       0x30
/* Huffman table selector for DC values */
#define JPGENC_JPE_DC_TABLE_SELECT_REG   0x34
/* Huffman table selector for AC value */
#define JPGENC_JPE_AC_TABLE_SELECT_REG   0x38
/* table programming register */
#define JPGENC_JPE_TABLE_DATA_REG        0x3C
/* table programming select register */
#define JPGENC_JPE_TABLE_ID_REG          0x40
/* Huffman AC table 0 length */
#define JPGENC_JPE_TAC0_LEN_REG          0x44
/* Huffman DC table 0 length */
#define JPGENC_JPE_TDC0_LEN_REG          0x48
/* Huffman AC table 1 length */
#define JPGENC_JPE_TAC1_LEN_REG          0x4C
/* Huffman DC table 1 length */
#define JPGENC_JPE_TDC1_LEN_REG          0x50
/* encode mode */
#define JPGENC_JPE_ENCODER_MODE_REG      0x60
/* debug information register */
#define JPGENC_JPE_DEBUG_REG             0x64
/* JPEG error interrupt mask register */
#define JPGENC_JPE_ERROR_IMR_REG         0x68
/* JPEG error raw interrupt status register */
#define JPGENC_JPE_ERROR_RIS_REG         0x6C
/* JPEG error masked interrupt status register */
#define JPGENC_JPE_ERROR_MIS_REG         0x70
/* JPEG error interrupt clear regisger */
#define JPGENC_JPE_ERROR_ICR_REG         0x74
/* JPEG error interrupt set register */
#define JPGENC_JPE_ERROR_ISR_REG         0x78
/* JPEG status interrupt mask register */
#define JPGENC_JPE_STATUS_IMR_REG        0x7C
/* JPEG status raw interrupt status register */
#define JPGENC_JPE_STATUS_RIS_REG        0x80
/* JPEG status masked interrupt status register */
#define JPGENC_JPE_STATUS_MIS_REG        0x84
/* JPEG status interrupt clear register */
#define JPGENC_JPE_STATUS_ICR_REG        0x88
/* JPEG status interrrupt set register */
#define JPGENC_JPE_STATUS_ISR_REG        0x8C
/* JPEG configuration register */
#define JPGENC_JPE_CONFIG_REG            0x90
/* Y Buffer address */
#define JPGENC_ADDRESS_Y_REG             0x94
/* UV Buffer addresss */
#define JPGENC_ADDRESS_UV_REG            0x98
/* Address stride in bytes */
#define JPGENC_STRIDE_REG                0x9C
/* Source Synchronization configuration */
#define JPGENC_SYNCCFG_REG               0x100
/* Picture from pipe2 Hsize */
#define JPGENC_JPE_ENC_HRIGHT2_REG       0x104
/* Number of encoded bytes sent to video port */
#define JPGENC_JPG_BYTE_CNT_REG          0x108
/* Prefetch configuration */
#define JPGENC_PREFETCH_REG              0x10C
/* Prefetch ID configuration (Y Buffer */
#define JPGENC_PREFETCH_IDY0_REG         0x110
/* Prefetch ID configuration (Y Buffer */
#define JPGENC_PREFETCH_IDY1_REG         0x114
/* Prefetch ID configuration (UV Buffer */
#define JPGENC_PREFETCH_IDUV0_REG        0x118
/* Prefetch ID configuration (UV Buffer */
#define JPGENC_PREFETCH_IDUVY_REG        0x11C
/* Number of preread MCU configuration */
#define JPGENC_PREREAD_REG               0x120
/* swap pixel component at input. */
#define JPGENC_INPUT_SWAP_REG            0x124
/* used to force the clock which is generally controlled by HW */
#define JPGENC_FORCE_CLK_ON_CFG_REG      0x130
#define JPGENC_DBG_0_REG                 0x200
#define JPGENC_DBG_1_REG                 0x204
#define JPGENC_DBG_2_REG                 0x208
#define JPGENC_DBG_3_REG                 0x20C
#define JPGENC_DBG_4_REG                 0x210
#define JPGENC_DBG_5_REG                 0x214
#define JPGENC_DBG_6_REG                 0x218
#define JPGENC_DBG_7_REG                 0x21C
#define JPGENC_DBG_8_REG                 0x220
#define JPGENC_DBG_9_REG                 0x224
#define JPGENC_DBG_10_REG                0x228
#define JPGENC_DBG_11_REG                0x22C
#define JPGENC_DBG_12_REG                0x230
#define JPGENC_DBG_13_REG                0x234
#define JPGENC_DBG_14_REG                0x238
#define JPGENC_DBG_15_REG                0x23C

/*
 * IRQ related cfg register
 * reg of jpg_sub_ctrl module
 */
#define DMA_CRG_CFG0                     0x0
#define JPGENC_IRQ_REG0                  0x110
#define JPGENC_IRQ_REG1                  0x114
#define JPGENC_IRQ_REG2                  0x118

#define JPG_RO_STATE                     0xFC
#define JPGENC_CRG_CFG0                  0x100
#define JPGENC_CRG_CFG1                  0x104
#define JPGENC_MEM_CFG                   0x108

typedef union {
	struct {
		uint32_t reserved : 2; /* [1..0] */
		uint32_t address : 30; /* [31..2] */
	} bits;
	uint32_t reg32;
} u_jpegenc_address;

#endif /* __HJPG_REG_OFFSET_H__ */
