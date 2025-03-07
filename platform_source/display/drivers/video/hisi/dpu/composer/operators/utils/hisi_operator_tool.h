/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_OPERATOR_TOOL_H
#define HISI_OPERATOR_TOOL_H

#include <linux/platform_device.h>
#include <linux/types.h>
#include <securec.h>
#include "dpu_offline_define.h"
#include "dm/hisi_dm.h"

extern uint8_t g_scene_id;
extern uint32_t DM_INPUTDATA_ST_ADDR[];
extern int g_debug_wch_scf;
extern int g_debug_wch_yuv_in;
extern int g_debug_wch_clip_left;
extern int g_debug_wch_clip_right;
extern int g_debug_wch_clip_bot;
extern int g_debug_wch_clip_top;
extern int g_debug_wch_scf_rot;
extern int g_debug_block_scl_rot;
extern int g_debug_bbit_vdec;
extern int g_debug_bbit_venc;
extern long g_slice0_y_payload_addr;
extern long g_slice0_y_header_addr;
extern long g_slice0_c_payload_addr;
extern long g_slice0_c_header_addr;
extern long g_slice0_cmdlist_header_addr;
extern long g_slice1_cmdlist_header_addr;
extern long g_slice2_cmdlist_header_addr;
extern long g_slice3_cmdlist_header_addr;
extern int g_debug_en_uvup1;
extern int g_debug_en_sdma1;
extern int g_debug_en_demura;
extern int g_debug_en_ddic;
extern int g_debug_demura_type;
extern int g_debug_demura_id;
extern int g_demura_lut_fmt;
extern int g_demura_idx;
extern int g_r_demura_w;
extern int g_r_demura_h;
extern int g_demura_plane_num;
extern uint32_t g_local_dimming_en;
extern uint32_t g_gmp_bitext_mode;
extern int hisi_fb_display_effect_test;
extern int hisi_fb_display_en_dpp_core;
#define BITS_PER_BYTE 8
#define DMA_ALIGN_BYTES (128 / BITS_PER_BYTE)

/* Threshold for SCF Stretch and SCF filter */
#define RDMA_STRETCH_THRESHOLD  (2)
#define SCF_UPSCALE_MAX (60)
#define SCF_DOWNSCALE_MAX (60)
#define SCF_EDGE_FACTOR (3)
#define ARSR2P_INC_FACTOR (65536)

#define DPU_WIDTH(width) ((width) - 1)
#define DPU_HEIGHT(height) ((height) - 1)
#define count_from_zero(value) ((value) - 1)

enum DPU_DFC_STATIC_FORMAT {
	DPU_DFC_STATIC_PIXEL_FORMAT_RGB_565 = 0x0,
	DPU_DFC_STATIC_PIXEL_FORMAT_XRGB_4444 = 0x1,
	DPU_DFC_STATIC_PIXEL_FORMAT_ARGB_4444 = 0x2,
	DPU_DFC_STATIC_PIXEL_FORMAT_XRGB_1555 = 0x3,
	DPU_DFC_STATIC_PIXEL_FORMAT_ARGB_1555 = 0x4,
	DPU_DFC_STATIC_PIXEL_FORMAT_XRGB_8888 = 0x5,
	DPU_DFC_STATIC_PIXEL_FORMAT_ARGB_8888 = 0x6,
	DPU_DFC_STATIC_PIXEL_FORMAT_BGR_565 = 0x7,
	DPU_DFC_STATIC_PIXEL_FORMAT_XBGR_4444 = 0x8,
	DPU_DFC_STATIC_PIXEL_FORMAT_ABGR_4444 = 0x9,
	DPU_DFC_STATIC_PIXEL_FORMAT_XBGR_1555 = 0xA,
	DPU_DFC_STATIC_PIXEL_FORMAT_ABGR_1555 = 0xB,
	DPU_DFC_STATIC_PIXEL_FORMAT_XBGR_8888 = 0xC,
	DPU_DFC_STATIC_PIXEL_FORMAT_ABGR_8888 = 0xD,

	DPU_DFC_STATIC_PIXEL_FORMAT_YUV444 = 0xE,
	DPU_DFC_STATIC_PIXEL_FORMAT_YVU444 = 0xF,
	DPU_DFC_STATIC_PIXEL_FORMAT_YUYV422 = 0x10,
	DPU_DFC_STATIC_PIXEL_FORMAT_YUYV420_SP_8BIT = 0x10,
	DPU_DFC_STATIC_PIXEL_FORMAT_YVYU422 = 0x11,
	DPU_DFC_STATIC_PIXEL_FORMAT_VYUY422 = 0x12,
	DPU_DFC_STATIC_PIXEL_FORMAT_UYVY422 = 0x13,

	DPU_DFC_STATIC_PIXEL_FORMAT_RGBA_1010102 = 0x34,
	DPU_DFC_STATIC_PIXEL_FORMAT_YUVA_1010102 = 0x15,
	DPU_DFC_STATIC_PIXEL_FORMAT_UYVA_1010102 = 0x16,
	DPU_DFC_STATIC_PIXEL_FORMAT_VUYA_1010102 = 0x17,
	DPU_DFC_STATIC_PIXEL_FORMAT_YUYV_10 = 0x18,
	DPU_DFC_STATIC_PIXEL_FORMAT_YUYV420_SP_10BIT = 0x18,
	DPU_DFC_STATIC_PIXEL_FORMAT_UYVY_10 = 0x19,
	DPU_DFC_STATIC_PIXEL_FORMAT_UYVY_Y8 = 0x1A,
};

enum DPU_DFC_DYNAMIC_FORMAT {
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_RGB_565 = 0x0,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XRGB_4444 = 0x1,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ARGB_4444 = 0x2,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XRGB_1555 = 0x3,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ARGB_1555 = 0x4,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XRGB_8888 = 0x5,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ARGB_8888 = 0x6,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_BGR_565 = 0x7,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XBGR_4444 = 0x8,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ABGR_4444 = 0x9,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XBGR_1555 = 0xA,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ABGR_1555 = 0xB,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_XBGR_8888 = 0xC,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_ABGR_8888 = 0xD,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_PACKET_8BIT = 0x10,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_SP_8BIT = 0x29,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_P_8BIT = 0x2A,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_PACKET_10BIT = 0x18,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_SP_10BIT = 0x2B,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV422_P_10BIT = 0x2C,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV420_SP_8BIT = 0x2F,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV420_P_8BIT = 0x30,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV420_SP_10BIT = 0x31,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV420_P_10BIT = 0x32,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUV444 = 0xE,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YVU444 = 0xF,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUYV422 = 0x10,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YVYU422 = 0x11,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_VYUY422 = 0x12,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_UYVY422 = 0x13,

	DPU_DFC_DYNAMIC_PIXEL_FORMAT_RGBA_1010102 = 0x34,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUVA_1010102 = 0x15,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_UYVA_1010102 = 0x16,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_VUYA_1010102 = 0x17,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_YUYV_10 = 0x18,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_UYVY_10 = 0x19,
	DPU_DFC_DYNAMIC_PIXEL_FORMAT_UYVY_Y8 = 0x1A,
};

enum DPU_WDMA_STATIC_FORMAT {
	DPU_WDMA_PIXEL_FORMAT_RGB_565 = 0x0,
	DPU_WDMA_PIXEL_FORMAT_ARGB_4444 = 0x1,
	DPU_WDMA_PIXEL_FORMAT_XRGB_4444 = 0x2,
	DPU_WDMA_PIXEL_FORMAT_ARGB_5551 = 0x3,
	DPU_WDMA_PIXEL_FORMAT_XRGB_5551 = 0x4,
	DPU_WDMA_PIXEL_FORMAT_ARGB_8888 = 0x5,
	DPU_WDMA_PIXEL_FORMAT_XRGB_8888 = 0x6,
	DPU_WDMA_PIXEL_FORMAT_RESERVED0 = 0x7,

	DPU_WDMA_PIXEL_FORMAT_YUYV_422_PKG = 0x8,
	DPU_WDMA_PIXEL_FORMAT_YUV_420_SP_HP = 0x9,
	DPU_WDMA_PIXEL_FORMAT_YUV_420_P_HP = 0xA,
	DPU_WDMA_PIXEL_FORMAT_YUV_422_SP_HP = 0xB,
	DPU_WDMA_PIXEL_FORMAT_YUV_422_P_HP = 0xC,
	DPU_WDMA_PIXEL_FORMAT_AYUV_4444 = 0xD,
	DPU_WDMA_PIXEL_FORMAT_RESERVED1 = 0xE,
	DPU_WDMA_PIXEL_FORMAT_RESERVED2 = 0xF,
	DPU_WDMA_PIXEL_FORMAT_RGBA_1010102 = 0x10,
	DPU_WDMA_PIXEL_FORMAT_Y410_10BIT = 0x11,
	DPU_WDMA_PIXEL_FORMAT_YUV422_10BIT = 0x12,
	DPU_WDMA_PIXEL_FORMAT_YUV420_SP_10BIT = 0x13,
	DPU_WDMA_PIXEL_FORMAT_YUV422_SP_10BIT = 0x14,
	DPU_WDMA_PIXEL_FORMAT_YUV420_P_10BIT = 0x15,
	DPU_WDMA_PIXEL_FORMAT_YUV422_P_10BIT = 0x16,
};

enum DPU_SDMA_STATIC_FORMAT {
	DPU_SDMA_PIXEL_FORMAT_RGB_565 = 0x0,
	DPU_SDMA_PIXEL_FORMAT_XRGB_4444 = 0x1,
	DPU_SDMA_PIXEL_FORMAT_ARGB_4444 = 0x2,
	DPU_SDMA_PIXEL_FORMAT_XRGB_1555 = 0x3,
	DPU_SDMA_PIXEL_FORMAT_ARGB_1555 = 0x4,
	DPU_SDMA_PIXEL_FORMAT_XRGB_8888 = 0x5,
	DPU_SDMA_PIXEL_FORMAT_ARGB_8888 = 0x6,
	DPU_SDMA_PIXEL_FORMAT_BGR_565 = 0x7,

	DPU_SDMA_PIXEL_FORMAT_XBGR_4444 = 0x8,
	DPU_SDMA_PIXEL_FORMAT_ABGR_4444 = 0x9,
	DPU_SDMA_PIXEL_FORMAT_XBGR_1555 = 0xA,
	DPU_SDMA_PIXEL_FORMAT_ABGR_1555 = 0xB,
	DPU_SDMA_PIXEL_FORMAT_XBGR_8888 = 0xC,
	DPU_SDMA_PIXEL_FORMAT_ABGR_8888 = 0xD,

	DPU_SDMA_PIXEL_FORMAT_RGBG = 0b100000,
	DPU_SDMA_PIXEL_FORMAT_RGBG_HIDIC = 0b100001,
	DPU_SDMA_PIXEL_FORMAT_RGBG_IDLEPACK = 0b100010,
	DPU_SDMA_PIXEL_FORMAT_RGBG_DEBURNIN = 0b100011,
	DPU_SDMA_PIXEL_FORMAT_RGBG_DELTA = 0b100100,
	DPU_SDMA_PIXEL_FORMAT_RGBG_DELTA_HIDIC = 0b110110,
	DPU_SDMA_PIXEL_FORMAT_RGBG_DELTA_IDLEPACK = 0b100101,
	DPU_SDMA_PIXEL_FORMAT_RGB_DEBURNIN = 0b100110,

	DPU_SDMA_PIXEL_FORMAT_RGB_10BIT = 0b100111,
	DPU_SDMA_PIXEL_FORMAT_BGR_10BIT = 0b101000,
	DPU_SDMA_PIXEL_FORMAT_ARGB_10101010 = 0b111001,
	DPU_SDMA_PIXEL_FORMAT_XRGB_10101010 = 0b111000,
	DPU_SDMA_PIXEL_FORMAT_BGRA_1010102 = 0b10100,
	DPU_SDMA_PIXEL_FORMAT_RGBA_1010102 = 0b110100, // 0x34

	DPU_SDMA_PIXEL_FORMAT_AYUV_10101010 = 0b110111,
	DPU_SDMA_PIXEL_FORMAT_YUV444 = 0b1110,
	DPU_SDMA_PIXEL_FORMAT_YVU444 = 0b1111,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_8BIT_PKG = 0b10000,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_8BIT_SP = 0b101001,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_8BIT_P = 0b101010,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_10BIT_PKG = 0b11000,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_10BIT_SP = 0b101011,
	DPU_SDMA_PIXEL_FORMAT_YUYV_422_10BIT_P = 0b101100,

	DPU_SDMA_PIXEL_FORMAT_YVYU_422_8BIT_PKG = 0b10001,

	DPU_SDMA_PIXEL_FORMAT_YUYV_420_8BIT_SP = 0b101111,
	DPU_SDMA_PIXEL_FORMAT_YUYV_420_8BIT_P = 0b110000,
	DPU_SDMA_PIXEL_FORMAT_YUYV_420_10BIT_SP = 0b110001,
	DPU_SDMA_PIXEL_FORMAT_YUYV_420_10BIT_P = 0b110010,

	DPU_SDMA_PIXEL_FORMAT_YUVA_1010102 = 0b010101,
	DPU_SDMA_PIXEL_FORMAT_UYVA_1010102 = 0b010110,
	DPU_SDMA_PIXEL_FORMAT_VUYA_1010102 = 0b010111,
	DPU_SDMA_PIXEL_FORMAT_D1_128 = 0b110100,
	DPU_SDMA_PIXEL_FORMAT_D3_128 = 0b110101,
};

enum DPU_DSD_STATIC_FORMAT {
	DPU_DSD_PIXEL_FORMAT_XRGB_8888 = 0x5,
	DPU_DSD_PIXEL_FORMAT_RGBA_1010102 = 0b110100,
	DPU_DSD_PIXEL_FORMAT_YUV444 = 0b1110,
	DPU_DSD_PIXEL_FORMAT_YUYV_422_8BIT_PKG = 0b10000,
	DPU_DSD_PIXEL_FORMAT_YUYV_422_10BIT_PKG = 0b11000,
	DPU_DSD_PIXEL_FORMAT_YUVA_1010102 = 0b010111,
};

enum DPU_DSD_WRAP_FORMAT {
	DPU_DSD_WRAP_FORMAT_RGB_6BIT =     0b00000,
	DPU_DSD_WRAP_FORMAT_RGB_8BIT =     0b00001,
	DPU_DSD_WRAP_FORMAT_RGB_10BIT =    0b00010,
	DPU_DSD_WRAP_FORMAT_YUV444_8BIT =  0b00011,
	DPU_DSD_WRAP_FORMAT_YUV444_10BIT = 0b00100,
	DPU_DSD_WRAP_FORMAT_YUV422_8BIT =  0b00101,
	DPU_DSD_WRAP_FORMAT_YUV422_10BIT = 0b00110,
	DPU_DSD_WRAP_FORMAT_YUV420_8BIT =  0b00111,
	DPU_DSD_WRAP_FORMAT_YUV420_10BIT = 0b01000,
};

enum DPU_RDFC_OUT_FORMAT {
	DPU_RDFC_OUT_FORMAT_ARGB_10101010 = 0b111001,
	DPU_RDFC_OUT_FORMAT_AYUV_10101010 = 0b110111,
};

enum {
	DFC_STATIC,
	DFC_DYNAMIC
};

enum {
	DPU_HAL2SDMA_PIXEL_FORMAT,
	DPU_HAL2WDMA_PIXEL_FORMAT,
	DPU_HAL2DSD_PIXEL_FORMAT,
};

bool is_yuv_sp_420(uint32_t format);
bool is_yuv_sp_422(uint32_t format);
int dpu_transform_hal2dma(int transform, int chn_idx);
int dpu_pixel_format_hal2dma(int format, uint32_t dma_type);
int dpu_pixel_format_hal2dsdwrap(int format);
int dpu_pixel_format_hal2dfc(int format, int dfc_type);
bool hal_format_has_alpha(uint32_t format);
bool is_yuv_semiplanar(uint32_t format);
bool is_yuv_plane(uint32_t format);
bool is_yuv_format(uint32_t format);
bool is_10bit(uint32_t format);
bool is_8bit(uint32_t format);
bool is_offline_scene(uint8_t scene_id);
bool is_d3_128(uint32_t format);
void dpu_dump_layer(struct pipeline_src_layer *src_layer, struct disp_wb_layer *wb_layer);
void dpu_init_dump_layer(struct pipeline_src_layer *src_layer, struct disp_wb_layer *wb_layer);

#endif
