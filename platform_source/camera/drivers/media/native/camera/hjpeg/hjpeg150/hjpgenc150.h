/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: jpeg encode
 * Create: 2015-10-09
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#ifndef __HJPEGNC150_H__
#define __HJPEGNC150_H__

#define mask0(name) (((unsigned int)1 << (name##_LEN)) - 1)
#define mask1(name) ((((unsigned int)1 << (name##_LEN)) - 1) << (name##_OFFSET))

/* operation on the field of a variable */
#define reg_get_field(reg, name) \
	(((reg) >> (name##_OFFSET)) & mask0(name))

#define reg_set_field(reg, name, val) \
	(reg = ((reg) & ~mask1(name)) | \
	(((val) & mask0(name)) << (name##_OFFSET)))

static inline void reg_set(void __iomem *addr, u32 value)
{
	iowrite32(value, addr);
}

static inline u32 reg_get(void __iomem *addr)
{
	return ioread32(addr);
}

#define set_field_to_val(reg, field, value) reg_set_field(reg, field, value)

#define set_field_to_reg(reg, field, value) \
	do { \
		unsigned int v = reg_get(reg); \
		set_field_to_val(v, field, value); \
		reg_set(reg, v); \
	} while (0)


#define check_align(value, align) (value % align == 0)

#define check_down(value, al) ((unsigned int)(value) & ~((al) - 1))

/**
 * jpgenc head offset need write in the first 4 bytes of input buffer
 * if JPGENC_HEAD_OFFSET small than 4, func jpgenc_add_header must be modify
 */
#define JPGENC_HEAD_SIZE                     640
#define JPGENC_HEAD_OFFSET                   11
#define MAX_JPEG_BUFFER_SIZE                 (32 * 1024 * 1024) /* 32MB */

#define JPGENC_RESTART_INTERVAL              0
#define JPGENC_RESTART_INTERVAL_ON           1
#define JPGENC_INIT_QUALITY                  95
#define JPGENC_RESET_REG                     0xE8420060
#define JPGENC_RESET_LEN                     1
#define JPGENC_RESET_OFFSET                  3
#define ISP_CORE_CTRL_1_REG                  0xE8583700
#define WAIT_ENCODE_TIMEOUT                  10000

#define EXP_PIX                              1
#define ACCESS_LIMITER                       7
#define ALLOCATED_DU                         8
#define CVDR_SRT_BASE                        0xE842E000
#define SMMU_BASE_ADDR                       0xE8406000

#define CVDR_SRT_VP_WR_CFG_25_OFFSET         0x1AC
#define CVDR_SRT_VP_WR_AXI_FS_25_OFFSET      0x1B0
#define CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET    0x1B4
#define CVDR_SRT_VP_WR_IF_CFG_25_OFFSET      0x1B8
#define CVDR_SRT_NR_RD_CFG_1_OFFSET          0xA10
#define CVDR_SRT_LIMITER_NR_RD_1_OFFSET      0xA18
#define CVDR_SRT_LIMITER_VP_WR_25_OFFSET     0x464

#define SMMU_GLOBAL_BYPASS                   0x00
#define SMMU_BYPASS_VPWR                     0xBC
#define SMMU_BYPASS_NRRD1                    0x114
#define SMMU_BYPASS_NRRD2                    0x118
#define SMMU_SCRS                            0x710
#define MICROSECOND_PER_SECOND               1000000

typedef enum _format_e {
	JPGENC_FORMAT_YUV422 = 0x0,
	JPGENC_FORMAT_YUV420 = 0x10,
	JPGENC_FORMAT_BIT    = 0xF0,
} format_e;

typedef enum _cvdr_pix_fmt_e {
	DF_1PF8 = 0x0,
	DF_1PF10 = 0x1,
	DF_1PF12 = 0x2,
	DF_1PF14 = 0x3,
	DF_2PF8 = 0x4,
	DF_2PF10 = 0x5,
	DF_2PF12 = 0x6,
	DF_2PF14 = 0x7,
	DF_3PF8 = 0x8,
	DF_3PF10 = 0x9,
	DF_3PF12 = 0xA,
	DF_3PF14 = 0xB,
	DF_D32 = 0xC,
	DF_D48 = 0xD,
	DF_D64 = 0xE,
	DF_FMT_INVALID,
} cvdr_pix_fmt_e;

typedef enum _phyaddr_access_e {
	JPEGENC_INDEX = 0,
	CVDR_INDEX,
	SMMU_INDEX,
	MAX_INDEX,
} phyaddr_access_e;

typedef struct _cvdr_wr_fmt_desc_t {
	cvdr_pix_fmt_e      pix_fmt;
	unsigned char       pix_expan;
	unsigned char       access_limiter;
	uint32_t            last_page;
	unsigned short      line_stride;
	unsigned short      line_wrap;
} cvdr_wr_fmt_desc_t;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t vpwr_pixel_format     : 4; /* [3.0] */
		uint32_t vpwr_pixel_expansion  : 1; /* [4] */
		uint32_t reserved_0            : 4; /* [8.5] */
		uint32_t vpwr_access_limiter   : 4; /* [12.9] */
		uint32_t reserved_1            : 1; /* [13] */
		uint32_t reserved_2            : 1; /* [14] */
		uint32_t vpwr_last_page        : 17; /* [31.15] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_vp_wr_cfg;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t vpwr_line_stride      : 10; /* [9.0] */
		uint32_t reserved_0            : 5;  /* [14.10] */
		uint32_t vpwr_line_wrap        : 14; /* [28.15] */
		uint32_t reserved_1            : 3;  /* [31.29] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_vp_wr_axi_line;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t reserved_0 : 4; /* [3.0] */
		uint32_t vpwr_address_frame_start : 28; /* [31.4] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_vp_wr_axi_fs;

/* Define the union u_cvdr_srt_nr_rd_cfg_1 */
typedef union {
	/* Define the struct bits */
	struct {
		uint32_t reserved_0 : 5; /* [4.0] */
		uint32_t nrrd_allocated_du_1 : 5; /* [9.5] */
		uint32_t reserved_1 : 6; /* [15.10] */
		uint32_t nr_rd_stop_enable_du_threshold_reached_1 : 1; /* [16] */
		uint32_t nr_rd_stop_enable_flux_ctrl_1 : 1; /* [17] */
		uint32_t nr_rd_stop_enable_pressure_1 : 1; /* [18] */
		uint32_t reserved_2 : 5; /* [23.19] */
		uint32_t nr_rd_stop_ok_1 : 1; /* [24] */
		uint32_t nr_rd_stop_1 : 1; /* [25] */
		uint32_t reserved_3 : 5; /* [30.26] */
		uint32_t nrrd_enable_1 : 1; /* [31] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_cvdr_srt_nr_rd_cfg_1;

/* Define the union u_cvdr_srt_limiter_nr_rd_1 */
typedef union {
	/* Define the struct bits */
	struct {
		uint32_t nrrd_access_limiter_0_1 : 4; /* [3.0] */
		uint32_t nrrd_access_limiter_1_1 : 4; /* [7.4] */
		uint32_t nrrd_access_limiter_2_1 : 4; /* [11.8] */
		uint32_t nrrd_access_limiter_3_1 : 4; /* [15.12] */
		uint32_t reserved_0 : 8; /* [23.16] */
		uint32_t nrrd_access_limiter_reload_1 : 4; /* [27.24] */
		uint32_t reserved_1 : 4; /* [31.28] */
	} bits;
	/* Define an unsigned member */
	uint32_t reg32;
} u_cvdr_srt_limiter_nr_rd_1;

#endif /* __HJPEGNC150_H__ */
