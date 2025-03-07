/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dsc define
 * Author: yangbr
 * Create: 2020-01-22
 * Notes: null
 */
#ifndef __DPTX_PLATFORM_H__
#define __DPTX_PLATFORM_H__

#define range_clamp(x, min, max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))
#define range_check(s, x, low_bound, upper_bound) \
	do { \
		if (((x) < (low_bound)) || ((x) > (upper_bound))) { \
			disp_pr_err(s" out of range, needs to be between %d and %d\n", low_bound, upper_bound); \
		} \
	} while (0)
#define PLATFORM_COUNT 5
#define DSC_0BPP (0)
#define DPTX_DSC_NUM_ENC_MSK GENMASK(3, 0)
#define DPTX_DSC_CTL 0x0234
#define DPTX_DSC_HWCFG 0x0230
#define DSC_RC_RANGE_LENGTH 15
#define DSC_THRESH_LENGTH 14
#define DSC_ENCODERS 2
#define DSC_6BPP 6
#define DSC_8BPP 8
#define DSC_10BPP 10
#define DSC_12BPP 12
#define DSC_14BPP 14
#define DSC_16BPP 16
#define DSC_8BPC 8
#define DSC_10BPC 10
#define DSC_12BPC 12
#define DSC_14BPC 14
#define DSC_16BPC 16

#define OFFSET_FRACTIONAL_BIT 11
#define LINEBUF_DEPTH_9 9
#define LINEBUF_DEPTH_11 11
#define LINEBUF_DEPTH_16 16
#define DPTX_CONFIG_REG2 0x104
#define DSC_MAX_NUM_LINES_SHIFT 16
#define DSC_MAX_NUM_LINES_MASK GENMASK(31, 16)

#define INITIAL_OFFSET_8BPP 6144
#define INITIAL_DELAY_8BPP 512

#define INITIAL_OFFSET_10BPP 5632
#define INITIAL_DELAY_10BPP 512
#define INITIAL_DELAY_10BPP_RGB_DEFAULT 410

#define INITIAL_OFFSET_12BPP 2048
#define INITIAL_DELAY_12BPP 341
#define INITIAL_OFFSET_12BPP_RGB_DEFAULT 2048
#define INITIAL_DELAY_12BPP_RGB_DEFAULT 341

#define INITIAL_OFFSET_12BPP_YUV422_DEFAULT 6144
#define INITIAL_DELAY_12BPP_YUV422_DEFAULT 512
#define INITIAL_OFFSET_12BPP_YUV422 5632

#define INITIAL_OFFSET_14BPP_YUV422_DEFAULT 5632
#define INITIAL_DELAY_14BPP_YUV422_DEFAULT 410

#define INITIAL_OFFSET_16BPP_YUV422 2048
#define INITIAL_DELAY_16BPP_8BPC_YUV422 256
#define INITIAL_OFFSET_16BPP_YUV422_DEFAULT 2048
#define INITIAL_DELAY_16BPP_YUV422_DEFAULT 341

#define FLATNESS_MIN_QP_8BPC 3
#define FLATNESS_MAX_QP_8BPC 12
#define FLATNESS_MIN_QP_10BPC 7
#define FLATNESS_MAX_QP_10BPC 16
#define FLATNESS_MIN_QP_12BPC 11
#define FLATNESS_MAX_QP_12BPC 20
#define RC_QUANT_INCR_LIMIT0_8BPC 11
#define RC_QUANT_INCR_LIMIT1_8BPC 11
#define RC_QUANT_INCR_LIMIT0_10BPC 15
#define RC_QUANT_INCR_LIMIT1_10BPC 15
#define RC_QUANT_INCR_LIMIT0_12BPC 19
#define RC_QUANT_INCR_LIMIT1_12BPC 19

#define RC_MODEL_SIZE 8192
#define RC_EDGE_FACTOR 6
#define RC_TGT_OFFSET_HIGH 3
#define RC_TGT_OFFSET_LOW 3
#define DP_DSC_MAJOR_SHIFT 0
#define PIXELS_PER_GROUP 3
#define BITS_PER_CHUNK 8
#define RC_RANGE_COUNT 12
#define RC_MODEL_GEN_RC_COUNT 4
#define RC_MODEL_YUV422_DEFAULT 3
#define RC_MODEL_RGB_DEFAULT 4
#endif
