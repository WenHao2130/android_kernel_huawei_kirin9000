﻿/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: initial draft
 * Create: 2014-05-10
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

#ifndef HISP300_MSG_H_INCLUDED
#define HISP300_MSG_H_INCLUDED

#include <platform_include/isp/linux/hisp_remoteproc.h>
#include "hisp300/hisp300_msg_ispnn_api.h"

#define MAX_INPUT_STREAM_NUM 2
#define MAX_STREAM_NUM 14
#define ARSR_REQ_OUT_NUM 2
#define NAME_LEN 32
#define PARAS_LEN 400
#define EXT_ACK_PARAS_LEN 72
#define EVENT_PARAMS_LEN 400
#define PIPELINE_COUNT 2
#define MAX_WARP_CGRID_POINT 120*120

/* based on msg len is 464 */
#define MAX_SET_ISP_NR 1 /* 50, 53, 59 */
#define MAX_GET_ISP_NR 1 /* 100, 108, 119 */
#define MAX_SET_I2C_NR 1 /* 35, 35, 39 */
#define MAX_GET_I2C_NR 1 /* 50, 53, 59 */

typedef enum {
	DMAP_DGEN_DISP,
	DMAP_DGEN_CONF,
	DMAP_LEFT_VERT,
	DMAP_DOPT_HORZ_DISP,
	DMAP_DOPT_VERT_DISP,
	DMAP_DGEN_DEPTH,
	DMAP_DOPT_DEPTH,
	DMAP_DOPT_XMAP,
	DMAP_DOPT_YMAP,
	DMAP_MAX_OUTPUT,
} dmap_output_e;

typedef enum {
	PIXEL_FMT_RAW10 = 0,
	PIXEL_FMT_RAW12 = 1,
	PIXEL_FMT_RAW14 = 2,
	PIXEL_FMT_JPEG = 3,
	PIXEL_FMT_YUV422_UYVY = 4, /* default jpgenc format */
	PIXEL_FMT_YUV420_NV12 = 5,
	PIXEL_FMT_YUV420_NV21 = 6,
	PIXEL_FMT_YUV422_VYUY = 7,
	PIXEL_FMT_YUV422_YUYV = 8,
	PIXEL_FMT_YUV422_YVYU = 9,
	PIXEL_FMT_MONOCHROME = 10,
	PIXEL_FMT_Y8 = 11,
	PIXEL_FMT_YUV420SP_WARP_ARSR = 12, /* only warp */
	PIXEL_FMT_YUV420_PLANAR = 13, /* only warp */
	PIXEL_FMT_XYMAP_8 = 14, /* only warp */
	PIXEL_FMT_XYMAP = 15, /* only warp */
	PIXEL_FMT_XYMAP_XY = 16, /* only warp */
	PIXEL_FMT_XYMAP_XY_8 = 17, /* only warp */
	PIXEL_FMT_WARP_XYMAP = 18, /* only warp */
	PIXEL_FMT_DEPTH_MAP_8 = 19, /* only warp */
	PIXEL_FMT_DEPTH_MAP_16 = 20, /* only warp */
	PIXEL_FMT_YUV422_SP = 21,
	PIXEL_FMT_YUV420_HFBC = 22, /* for mdc */
	PIXEL_FMT_D48 = 23,
	PIXEL_FMT_D64 = 24,
	PIXEL_FMT_YUV422_UYVY_10bit = 25,
	PIXEL_FMT_YUV420_NV12_10bit = 26,
	PIXEL_FMT_YUV420_NV21_10bit = 27,
	PIXEL_FMT_YUV422_VYUY_10bit = 28,
	PIXEL_FMT_YUV422_YUYV_10bit = 29,
	PIXEL_FMT_YUV422_YVYU_10bit = 30,
	PIXEL_FMT_YUV420_10BIT_HFBC = 31, /* for p2p 10bit */
	PIXEL_FMT_RGB888 = 32,
	PIXEL_FMT_XRGB8888 = 33,
	/* for rawmfnr bm3d */
	PIXEL_FMT_MV = 34,
	PIXEL_FMT_RAW18 = 35,
	PIXEL_FMT_GHOST_MAP = 36,
	PIXEL_FMT_PLC = 37,
	PIXEL_FMT_COL = 38,
	PIXEL_FMT_BLK_COORD = 39,
	PIXEL_FMT_ADAPTIVE_MAP = 40,
	PIXEL_FMT_RES = 41,
	PIXEL_FMT_COMPRESS_SENSOR_RAW = 42,
	PIXEL_FMT_MAX,
} pix_format_e;

typedef enum {
	STREAM_ISP_TOF_EMBEDED = 0,
	STREAM_REP_RAW_IN = 1,
	STREAM_ISP_YUV_OUT_PREVIEW = 2,
	STREAM_ISP_YUV_OUT_VIDEO = 3,
	STREAM_REP_YUV_OUT = 4,
	STREAM_ISP_YUV_OUT_TINY = 5,
	STREAM_ISP_RAW_OUT = 6,
	STREAM_ISP_YUV_OUT_DMAP_CAP = 7,
	STREAM_ISP_YUV_OUT_DMAP_PRE = 8,
	STREAM_ISP_AFSTAT = 9,
	STREAM_RAW_OUT = 10,
	STREAM_ISP_PD = 11,
	STREAM_ISP_XRGB_BLIE = 12,
	STREAM_ISP_AWBSTAT = 13,
	STREAM_POS_MAX,
} stream_pos_e;

typedef enum  _buf_rawmfnr_bm3d_usage_e {
	STREAM_IN_RAW_MERGE = 0,
	STREAM_IN_MV_MERGE, /* must be 0 */
	STREAM_OUT_BM3D_RAW, /* for bm3d & noise bufferoutput raw buffer */
	/* STREAM_OUT_NOISE_RAW, for bm3d output noise buffer */
	STREAM_OUT_MERGE_RAW, /* for rawmfnr output fullframe PLC */
	STREAM_OUT_MERGE_GMO, /*for rawmfnr output 1/4 gost map */
	MAX_RAWMFNR_BM3D_STREAM_NUM,
} buf_rawmfnr_bm3d_usage_e;

typedef enum _buf_rawmfnr_e {
	/* for rawmfnr UT */
	BI_DS_IN = 0,
	BO_DS_OUT = 1,
	BI_BM_REF,
	BI_BM_ALT,
	BI_BM_MV,
	BO_BM_MV,
	BI_MERGE_REF,
	BI_MERGE_ALT,
	BI_MERGE_MV,
	BI_MERGE_ACI,
	BI_MERGE_GMI,
	BO_MERGE_ACO,
	BO_MERGE_GMO,
	BO_MERGE_PLC,
	BO_PLC_DS16,
	BO_MERGE_GMS,
	MAX_RAWMFNR_STREAM_NUM,
} buf_rawmfnr_e;

typedef enum _buf_bm3d_e {
	/* for bm3d UT*/
	BI_PLC_DS16_IN = 0,
	BI_GMS_IN,
	BI_BLK_COORDINATE,
	BO_BLK_COORDINATE,
	BO_LUMA_DS64,
	BO_COL1_OUT,
	BO_COL2_OUT,
	BO_COL3_OUT,

	BI_PLC_FULL,
	BI_GMO_IN,
	BI_LUMA_DS64,
	BO_LUMA_DS16,
	BO_ADAPTIVE_MAP,

	BI_PLC_FULL_DIN1,
	BI_PLC_FULL_DIN3,
	BI_ADAPTIVE_MAP,
	BI_LUMA_DS16,
	BI_COL1_IN,
	BI_COL2_IN,
	BI_COL3_IN,
	BO_RAW_OUT,
	BO_RES_OUT,
	MAX_BM3D_STREAM_NUM,
} buf_bm3d_e;

typedef enum {
	STREAM_TYPE_INPUT,
	STREAM_TYPE_OUTPUT,
} stream_type_e;

typedef enum {
	REGISTER_TYPE_ISP,
	REGISTER_TYPE_I2C,
} register_type_e;

typedef enum {
	MDC_OFF = 0,
	MDC_HFR_ON,
	MDC_PRE_ON,
	MDC_PRE_ON_WARP,
} mdc_flag_e;

typedef enum {
	Qs10_0 = 0,
	Qs9_1,
	Qs8_2,
	Qs9_2,
	Qs10_4,
	Qs11_4,
	Qs_MAX,
} cgrid_precision_e;

struct hi_list_head {
	struct hi_list_head *next, *prev;
};

typedef enum {
	/* Request items. */
	COMMAND_QUERY_CAPABILITY = 0x1000,
	COMMAND_ACQUIRE_CAMERA,
	COMMAND_RELEASE_CAMERA,
	COMMAND_USECASE_CONFIG,
	COMMAND_GET_OTP,
	COMMAND_REQUEST,
	COMMAND_JPEG_ENCODE,
	COMMAND_MAP_BUFFER,
	COMMAND_UNMAP_BUFFER,
	COMMAND_CALIBRATION_DATA,
	COMMAND_SET_ISP_REGISTERS,
	COMMAND_GET_ISP_REGISTER,
	COMMAND_SET_IIC_REGISTER,
	COMMAND_GET_IIC_REGISTER,
	COMMAND_TEST_CASE_INTERFACE,
	COMMAND_FLUSH,
	COMMAND_EXTEND_SET,
	COMMAND_EXTEND_GET,
	COMMAND_INV_TLB,
	COMMAND_QUERY_OIS_UPDATE,
	COMMAND_OIS_UPDATE,
	COMMAND_QUERY_LASER,
	COMMAND_ACQUIRE_LASER,
	COMMAND_RELEASE_LASER,
	COMMAND_ACQUIRE_DEPTHISP,
	COMMAND_RELEASE_DEPTHISP,
	COMMAND_GET_API_VERSION,
	COMMAND_STREAM_ON,
	COMMAND_STREAM_OFF,
	COMMAND_WARP_REQUEST,
	COMMAND_ARSR_REQUEST,
	COMMAND_DGEN_REQUEST,
	COMMAND_DOPT_REQUEST,
	COMMAND_DMAP_MAP_REQUEST,
	COMMAND_DMAP_UNMAP_REQUEST,
	COMMAND_DGEN_FLUSH_REQUEST,
	COMMAND_DOPT_FLUSH_REQUEST,
	COMMAND_DMAP_FORMAT_REQUEST,
	COMMAND_MOTION_SENSOR_MAP_REQUEST,
	COMMAND_DRBR_REQUEST,
	COMMAND_DMAP_REQUEST,
	COMMAND_DMAP_FLUSH_REQUEST,
	COMMAND_MEM_POOL_INIT_REQUEST,
	COMMAND_MEM_POOL_DEINIT_REQUEST,
	COMMAND_ISP_CPU_POWER_OFF_REQUEST,
	COMMAND_DYNAMIC_MAP_BUFFER,
	COMMAND_DYNAMIC_UNMAP_BUFFER,
	COMMAND_TNR_DYNAMIC_MAP_BUFFER,
	COMMAND_TNR_DYNAMIC_UNMAP_BUFFER,
	COMMAND_RAW2YUV_MAP_BUFFER,
	COMMAND_RAW2YUV_START,
	COMMAND_RAW2YUV_REQUEST,
	COMMAND_RAW2RGB_REQUEST,
	COMMAND_RGB2YUV_REQUEST,
	COMMAND_RAW2YUV_STOP,
	COMMAND_RAW2YUV_UNMAP_BUFFER,
	/* for rawmfnr bm3d raw processing */
	COMMAND_RAWMFNR_BM3D_MAP_BUFFER,
	COMMAND_RAWMFNR_BM3D_START,
	COMMAND_RAWMFNR_BM3D_REQUEST,
	COMMAND_RAWMFNR_BM3D_STOP,
	COMMAND_RAWMFNR_BM3D_UNMAP_BUFFER,
	/* end add */
	COMMAND_DMAP_OFFLINE_MAP_REQUEST,
	COMMAND_DMAP_OFFLINE_UNMAP_REQUEST,

	COMMAND_QUERY_DRIVER_IC,
	COMMAND_ACQUIRE_DRIVER_IC,
	COMMAND_RELEASE_DRIVER_IC,
	COMMAND_QUERY_DOT_PROJECTOR,
	COMMAND_ACQUIRE_DOT_PROJECTOR,
	COMMAND_RELEASE_DOT_PROJECTOR,
	COMMAND_GET_DOT_OTP,
	COMMAND_PQ_SETTING_CONFIG,
	COMMAND_FBDRAW_REQUEST,
	COMMAND_DEVICE_CTL,

	COMMAND_RELEASE_I2C,
	COMMAND_BATCH_REQUEST,
	COMMAND_CONNECT_CAMERA,

	/* req for ispnn */
	COMMAND_CREATE_ISPNN_MODEL,
	COMMAND_EXTEND_ISPNN_BUFFER,
	COMMAND_ENABLE_ISPNN_MODEL,
	COMMAND_DISABLE_ISPNN_MODEL,
	COMMAND_DESTROY_ISPNN_MODEL,

	/* Response items. */
	QUERY_CAPABILITY_RESPONSE = 0x2000,
	ACQUIRE_CAMERA_RESPONSE,
	RELEASE_CAMERA_RESPONSE,
	USECASE_CONFIG_RESPONSE,
	GET_OTP_RESPONSE,
	REQUEST_RESPONSE,
	JPEG_ENCODE_RESPONSE,
	MAP_BUFFER_RESPONSE,
	UNMAP_BUFFER_RESPONSE,
	CALIBRATION_DATA_RESPONSE,
	SET_ISP_REGISTERS_RESPONSE,
	GET_ISP_REGISTER_RESPONSE,
	SET_IIC_REGISTER_RESPONSE,
	GET_IIC_REGISTER_RESPONSE,
	TEST_CASE_RESPONSE,
	FLUSH_RESPONSE,
	EXTEND_SET_RESPONSE,
	EXTEND_GET_RESPONSE,
	INV_TLB_RESPONSE,
	QUERY_OIS_UPDATE_RESPONSE,
	OIS_UPDATE_RESPONSE,
	QUERY_LASER_RESPONSE,
	ACQUIRE_LASER_RESPONSE,
	RELEASE_LASER_RESPONSE,
	ACQUIRE_DEPTHISP_RESPONSE,
	RELEASE_DEPTHISP_RESPONSE,
	GET_ISP_VERSION_RESPONSE,
	STREAM_ON_RESPONSE,
	STREAM_OFF_RESPONSE,
	WARP_REQUEST_RESPONSE,
	ARSR_REQUEST_RESPONSE,
	DGEN_REQUEST_RESPONSE,
	DOPT_REQUEST_RESPONSE,
	DMAP_MAP_RESPONSE,
	DMAP_UNMAP_RESPONSE,
	DGEN_FLUSH_RESPONSE,
	DOPT_FLUSH_RESPONSE,
	DMAP_FORMAT_RESPONSE,
	MOTION_SENSOR_MAP_RESPONSE,
	DRBR_REQUEST_RESPONSE,
	DMAP_REQUEST_RESPONSE,
	DMAP_FLUSH_RESPONSE,
	MEM_POOL_INIT_RESPONSE,
	MEM_POOL_DEINIT_RESPONSE,
	ISP_CPU_POWER_OFF_RESPONSE,
	DYNAMIC_MAP_BUFFER_RESPONSE,
	DYNAMIC_UNMAP_BUFFER_RESPONSE,
	TNR_DYNAMIC_MAP_BUFFER_RESPONSE,
	TNR_DYNAMIC_UNMAP_BUFFER_RESPONSE,
	RAW2YUV_MAP_BUFFER_RESPONSE,
	RAW2YUV_START_RESPONSE,
	RAW2YUV_REQUEST_RESPONSE,
	RAW2RGB_REQUEST_RESPONSE,
	RGB2YUV_REQUEST_RESPONSE,
	RAW2YUV_STOP_RESPONSE,
	RAW2YUV_UNMAP_BUFFER_RESPONSE,
	/* for rawmfnr bm3d raw processing */
	RAWMFNR_BM3D_MAP_BUFFER_RESPONSE,
	RAWMFNR_BM3D_START_RESPONSE,
	RAWMFNR_BM3D_REQUEST_RESPONSE,
	RAWMFNR_BM3D_STOP_RESPONSE,
	RAWMFNR_BM3D_UNMAP_BUFFER_RESPONSE,
	/* end add */
	DMAP_OFFLINE_MAP_RESPONSE,
	DMAP_OFFLINE_UNMAP_RESPONSE,

	QUERY_DRIVER_IC_RESPONSE,
	ACQUIRE_DRIVER_IC_RESPONSE,
	RELEASE_DRIVER_IC_RESPONSE,
	QUERY_DOT_PROJECTOR_RESPONSE,
	ACQUIRE_DOT_PROJECTOR_RESPONSE,
	RELEASE_DOT_PROJECTOR_RESPONSE,
	GET_DOT_OTP_RESPONSE,
	PQ_SETTING_CONFIG_RESPONSE,
	FBDRAW_REQUEST_RESPONSE,
	DEVICE_CTL_RESPONSE,
	RELEASE_I2C_RESPONSE,
	BATCH_REQUEST_RESPONSE,
	CONNECT_CAMERA_RESPONCE,

	/* responce for ispnn */
	CREATE_ISPNN_MODEL_RESPONCE,
	EXTEND_ISPNN_BUFFER_RESPONCE,
	ENABLE_ISPNN_MODEL_RESPONCE,
	DISABLE_ISPNN_MODEL_RESPONCE,
	DESTROY_ISPNN_MODEL_RESPONCE,

	COMMAND_IPP_SRC_REQUEST,
	IPP_SRC_RESPONSE,

	/* Event items sent to AP. */
	MSG_EVENT_SENT = 0x3000,
} api_id_e;

typedef enum _ucfg_ext_e {
	NO_USE = 0 << 0,
	H_VIDEO_720P_120 = 1 << 1,
	H_VIDEO_1080P_60 = 1 << 2,
	MIRROR_MODE = 1 << 3,
	LONG_EXPOSURE_MODE = 1 << 4,
	HDR_MOVIE = 1 << 5,
	DARK_RAIDER_MODE = 1 << 6,
	H_VIDEO_720P_60 = 1 << 7,
	H_VIDEO_VGA_120 = 1 << 8,
	TUNING_PRE_MODE = 1 << 9,
	H_VIDEO_720P_240 = 1 << 10,
	H_VIDEO_1080P_120 = 1 << 11,
	H_VIDEO_HIGH_RES = 1 << 12,
	SEAMLESS_MODE = 1 << 13,
	FREQ_GEAR_FLAG = 1 << 14,
	QUADRAW_MODE = 1 << 15,
	SENSOR_FULLSIZE_4_3 = 1 << 16,
	SENSOR_FULLSIZE_16_9 = 1 << 17,
	SENSOR_HDR_MODE = 1 << 18,
	APERTURE_MODE = 1 << 19,
	REMOSAIC_MODE = 1 << 20,
	H_VIDEO_1080P_24_60 = 1 << 21,
	PHY_HIGHEST_SPEED_TEST_MODE = 1 << 22,
	H_VIDEO_4K_60 = 1 << 23,
	H_SUPER_VIDEO_MODE = 1 << 24,
	// 25,26,27,28 has been used by ISP FW
	FPGA_MODE = 1 << 29,
	RESERVED = 1 << 30,
} ucfg_ext_e;

typedef enum _ucfg_scene_e {
	CAMERA_SCENE_NORMAL = 0,
	CAMERA_SCENE_VIDEO,
	CAMERA_SCENE_DEBUG,
} ucfg_scene_e;

typedef enum {
	PRIMARY_CAMERA = 0,
	FRONT_CAMERA,
	SECONDARY_CAMERA,
	THIRD_CAMERA,
	IR_CAMERA,
	FOURTH_CAMERA = IR_CAMERA,
	FRONT_SECONDARY_CAMERA,
} camera_id_t;

#define MSG_ACK_REQUEST_MEMBER \
	unsigned int cam_id; \
	unsigned int num_targets; \
	unsigned int target_map; \
	unsigned int frame_number; \
	stream_info_t stream_info[MAX_STREAM_NUM]; \
	unsigned int input_setting_buffer; \
	unsigned int output_metadata_buffer; \
	unsigned int timestampL; \
	unsigned int timestampH; \
	unsigned int status;

typedef struct _isp_crop_region_info_t {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} isp_crop_region_info_t;

typedef struct _subcmd_crop_region_info_t {
	isp_crop_region_info_t crop_region;
} subcmd_crop_region_info_t;

typedef struct _subcmd_fbcd_info_t {
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	unsigned int fbcd_enable[PIPELINE_COUNT];
} subcmd_fbcd_info_t;

typedef struct _subcmd_stdraw_info_t {
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	unsigned int stdraw_enable[PIPELINE_COUNT];
} subcmd_stdraw_info_t;

typedef struct _subcmd_af_depth_data {
	unsigned int depth_data;
	unsigned int data_length;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int binning_type;
} subcmd_af_depth_data;

typedef struct _rawnfds_info_t {
	unsigned int previewStatus;
	unsigned int captureStatus;
}rawnfds_info_t;

typedef struct _yuvnfds_info_t {
	unsigned int previewStatus;
	unsigned int captureStatus;
}yuvnfds_info_t;

typedef struct _subcmd_rawnfds_info_t {
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	rawnfds_info_t rawnfds_info[PIPELINE_COUNT];
} subcmd_rawnfds_info_t;

typedef struct _subcmd_yuvnfds_info_t {
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	yuvnfds_info_t yuvnfds_info[PIPELINE_COUNT];
} subcmd_yuvnfds_info_t;

typedef enum _map_pool_usage_e {
	MAP_POOL_USAGE_FW = 0,
	MAP_POOL_USAGE_ISP_FW,
	MAP_POOL_USAGE_ISP,
	MAP_POOL_USAGE_HFBC,
	MAP_POOL_USAGE_MAX,
} map_pool_usage_e;

typedef enum _fill_stream_info_e {
	FILL_STREAM_CAP_RAW = 0,
	FILL_STREAM_TINY,
	FILL_STREAM_DMAP,
	FILL_STREAM_MAX,
} fill_stream_info_e;

typedef struct _fill_stream_params_t {
	unsigned char stream_mask;
	unsigned char fill_mask;
} fill_stream_params_t;

typedef struct _msg_ack_get_api_version_t {
	unsigned int major_version;
	unsigned int minor_version;
} msg_ack_get_api_version_t;

typedef struct _msg_req_query_capability_t {
	unsigned int cam_id;
	unsigned int csi_index;
	unsigned int i2c_index;
	char product_name[NAME_LEN];
	char sensor_name[NAME_LEN];
	unsigned int input_settings_buffer;
} msg_req_query_capability_t;

typedef struct _msg_ack_query_capability_t {
	unsigned int cam_id;
	char product_name[NAME_LEN];
	char sensor_name[NAME_LEN];
	char vcm_name[NAME_LEN];
	char ois_name[NAME_LEN];
	unsigned int output_metadata_buffer;
	int status;
	int version;
	int with_vcm;
	int with_ois;
} msg_ack_query_capability_t;

typedef struct _msg_req_release_i2c_t {
	unsigned int cam_id;
} msg_req_release_i2c_t;

typedef struct _msg_ack_release_i2c_t {
	unsigned int cam_id;
	int status;
} msg_ack_release_i2c_t;

typedef struct _msg_req_query_laser_t {
	unsigned int i2c_index;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
} msg_req_query_laser_t;

typedef struct _laser_spad_t {
	unsigned int ref_spad_count;
	unsigned char is_aperture_spads;
} laser_spad_t;

typedef struct _laser_dmax_t {
	unsigned int dmax_range;
	unsigned int dmax_rate;
} laser_dmax_t;

typedef struct _msg_ack_query_laser_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
	laser_spad_t spad;
} msg_ack_query_laser_t;

typedef struct _msg_req_acquire_camera_t {
	unsigned int cam_id;
	unsigned int csi_index;
	unsigned int i2c_index;
	unsigned int i2c_pad_type;
	struct hisp_phy_info_t phy_info;
	char sensor_name[NAME_LEN];
	char product_name[NAME_LEN];
	char extend_name[NAME_LEN];
	unsigned int input_otp_buffer;
	unsigned int input_calib_buffer;
	unsigned int buffer_size;
	unsigned int info_buffer;
	unsigned int info_count;
	unsigned int factory_calib_buffer;
	int ir_topology_type;
	unsigned int capacity;
} msg_req_acquire_camera_t;

typedef struct _msg_ack_acquire_camera_t {
	unsigned int cam_id;
	char sensor_name[NAME_LEN];
} msg_ack_acquire_camera_t;

typedef struct _laser_fov_t {
	float x;
	float y;
	float width;
	float height;
	float angle;
} laser_fov_t;

typedef struct _msg_req_release_camera_t {
	unsigned int cam_id;
} msg_req_release_camera_t;

typedef struct _msg_ack_release_camera_t {
	unsigned int cam_id;
} msg_ack_release_camera_t;

typedef struct _fov_info {
	float x;
	float y;
	float width;
	float height;
	float angle;
} fov_info_t;

typedef struct _msg_req_acquire_laser_t {
	unsigned int i2c_index;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
	int offset;
	int xtalk;
	laser_fov_t fov_info;
	laser_spad_t spad;
	laser_dmax_t dmax;
} msg_req_acquire_laser_t;

typedef struct _msg_ack_acquire_laser_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
} msg_ack_acquire_laser_t;

typedef struct _msg_req_release_laser_t {
	unsigned int i2c_index;
} msg_req_release_laser_t;

typedef struct _msg_ack_release_laser_t {
	unsigned int i2c_index;
} msg_ack_release_laser_t;

typedef struct _msg_req_acquire_depthisp_t {
	unsigned int i2c_index;
	unsigned char chip_type;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
} msg_req_acquire_depthisp_t;

typedef struct _msg_ack_acquire_depthisp_t {
	char name[NAME_LEN];
	int status;
} msg_ack_acquire_depthisp_t;

typedef struct _msg_req_release_depthisp_t {
	unsigned int i2c_index;
} msg_req_release_depthisp_t;

typedef struct _msg_ack_release_depthisp_t {
	unsigned int i2c_index;
} msg_ack_release_depthisp_t;

typedef struct _stream_config_t {
	unsigned int type;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int format;
	unsigned int secure;
} stream_config_t;

typedef struct _msg_req_usecase_config_t {
	unsigned int cam_id;
	unsigned int extension;
	unsigned int stream_nr;
	stream_config_t stream_cfg[MAX_STREAM_NUM];
	char time[32];
} msg_req_usecase_config_t;

typedef struct _msg_req_rawmfnr_usecase_config_t {
	unsigned int cam_id;
	unsigned int extension;
	unsigned int stream_nr;
	unsigned int image_full_width;
	unsigned int image_full_height;
	unsigned int image_full_type;
	stream_config_t stream_cfg[MAX_RAWMFNR_STREAM_NUM];
	char time[32];
} msg_req_rawmfnr_usecase_config_t;

typedef struct _msg_req_bm3d_usecase_config_t {
	unsigned int cam_id;
	unsigned int extension;
	unsigned int stream_nr;
	unsigned int image_full_width;
	unsigned int image_full_height;
	unsigned int image_full_type;
	stream_config_t stream_cfg[MAX_BM3D_STREAM_NUM];
	char time[32];
} msg_req_bm3d_usecase_config_t;

typedef struct _msg_ack_usecase_config_t {
	unsigned int cam_id;
	int status;
	unsigned int sensor_width;
	unsigned int sensor_height;
} msg_ack_usecase_config_t;

typedef struct _msg_req_pq_setting_config_t {
	unsigned int cam_id;
	unsigned int mode;
	char scene[256];
	unsigned int pq_setting;
	unsigned int pq_setting_size;
} msg_req_pq_setting_config_t;

typedef struct _msg_ack_pq_setting_config_t {
	unsigned int cam_id;
	unsigned int mode;
	unsigned int pq_setting;
	unsigned int pq_setting_size;
	int status;
} msg_ack_pq_setting_config_t;

typedef struct _msg_req_stream_on_t {
	unsigned int cam_id;
} msg_req_stream_on_t;

typedef struct _msg_req_stream_off_t {
	unsigned int cam_id;
	unsigned int is_hotplug;
} msg_req_stream_off_t;

typedef struct _msg_ack_stream_on_t {
	unsigned int cam_id;
	int status;
} msg_ack_stream_on_t;

typedef struct _msg_ack_stream_off_t {
	unsigned int cam_id;
	int status;
} msg_ack_stream_off_t;

typedef struct _msg_req_get_otp_t {
	unsigned int cam_id;
	char sensor_name[NAME_LEN];
	unsigned int input_otp_buffer;
	unsigned int buffer_size;
} msg_req_get_otp_t;

typedef struct _msg_ack_get_otp_t {
	unsigned int cam_id;
	char sensor_name[NAME_LEN];
	unsigned int output_otp_buffer;
	unsigned int buffer_size;
	int status;
} msg_ack_get_otp_t;

typedef struct _msg_req_get_dot_otp_t {
	unsigned int i2c_index;
	char dot_name[NAME_LEN];
	unsigned int input_otp_buffer;
	unsigned int buffer_size;
} msg_req_get_dot_otp_t;

typedef struct _msg_ack_get_dot_otp_t {
	unsigned int i2c_index;
	char dot_name[NAME_LEN];
	unsigned int output_otp_buffer;
	unsigned int buffer_size;
	int status;
} msg_ack_get_dot_otp_t;

typedef struct _stream_info_t {
	unsigned int buffer;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int format;
	unsigned int valid;
	unsigned int frame_num;
} stream_info_t;

typedef struct _msg_req_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
	unsigned int output_cap_info_buffer;
	unsigned int extra_buf_offset[MAX_STREAM_NUM];
} msg_req_request_t;

typedef struct _msg_req_request_offline_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
} msg_req_request_offline_t;

typedef struct _msg_req_request_offline_raw2rgb_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
} msg_req_request_offline_raw2rgb_t;

typedef struct _msg_req_request_offline_rgb2yuv_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
} msg_req_request_offline_rgb2yuv_t;

typedef struct _msg_req_rawmfnr_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_RAWMFNR_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
	unsigned int output_cap_info_buffer;
} msg_req_rawmfnr_request_t;

typedef struct _msg_req_bm3d_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	unsigned int buf[MAX_BM3D_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
	unsigned int output_cap_info_buffer;
} msg_req_bm3d_request_t;

typedef struct _msg_req_ipp_src_request_t {
	unsigned int cam_id;
	unsigned int slam_pyramid_layer;
	unsigned int ipp_df_mode;
	unsigned int ipp_enable_mode;
	unsigned int frame_num;
	unsigned int is_ref_frame; /* 1:ref or 0:cur */

	unsigned int input_buffer;
	unsigned int image_width;
	unsigned int image_height;

	unsigned int cur_input_buffer;//cmp ut test use
} msg_req_ipp_src_request_t;

typedef struct _msg_ack_ipp_src_request_t {
	unsigned int status;
} msg_ack_ipp_src_request_t;


typedef struct _msg_ack_request_t {
	MSG_ACK_REQUEST_MEMBER
	unsigned int output_cap_info_buffer;
} msg_ack_request_t;

typedef struct _msg_batch_req_request_t {
	unsigned int cam_id;
	unsigned int count;
	unsigned int requests_buf;
	unsigned int requests_buf_size;
} msg_batch_req_request_t;

typedef struct _msg_batch_ack_request_t {
	unsigned int cam_id;
	unsigned int count;
	unsigned int acks_buf;
	unsigned int acks_buf_size;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int system_couter_rate;
} msg_batch_ack_request_t;

typedef struct _msg_ack_request_offline_t {
	MSG_ACK_REQUEST_MEMBER
} msg_ack_request_offline_t;

typedef struct _msg_ack_raw_proc_request_t {
	MSG_ACK_REQUEST_MEMBER
} msg_ack_raw_proc_request_t;

typedef struct _msg_req_jpeg_encode_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int input_buffer_y;
	unsigned int input_buffer_uv;
	unsigned int output_buffer;
	unsigned int quality;
	unsigned int format;
} msg_req_jpeg_encode_t;

typedef struct _msg_req_fbdraw_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	stream_info_t input_stream_info;
	unsigned int input_raw_type; /* raw_type_e */
	stream_info_t output_stream_info;
	unsigned int output_raw_type; /* raw_type_e */
	unsigned int input_setting_buffer;
	unsigned int crop_enable; /* 0: disable 1: enable */
	isp_crop_region_info_t crop_region;
} msg_req_fbdraw_request_t;

typedef struct _msg_ack_fbdraw_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	unsigned int status;
	stream_info_t output_stream_info;
} msg_ack_fbdraw_request_t;

typedef struct _msg_ack_rawmfnr_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	stream_info_t stream_info[MAX_RAWMFNR_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int status;
} msg_ack_rawmfnr_request_t;

typedef struct _msg_ack_bm3d_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	stream_info_t stream_info[MAX_BM3D_STREAM_NUM];
	unsigned int input_setting_buffer;
	unsigned int output_metadata_buffer;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int status;
} msg_ack_bm3d_request_t;

typedef struct _rawmfnr_bm3d_control_info_t {
	unsigned int reference_flag; /* current frame is REF of ALT */
	unsigned int rawmfnr_enable;
	unsigned int bm3d_enable;
	unsigned int ds_bm_enable; /* need ds,bm or not,when rawmfnr enable */
} rawmfnr_bm3d_control_info_t;

typedef struct _msg_req_raw_proc_request_t {
	unsigned int cam_id;
	unsigned int num_targets;
	unsigned int target_map;
	unsigned int frame_number;
	rawmfnr_bm3d_control_info_t raw_proc_ctl;
	unsigned int buf[MAX_RAWMFNR_BM3D_STREAM_NUM];
	unsigned int input_setting_buffer; /* need add rawnf_stat */
	unsigned int output_metadata_buffer;
	unsigned int output_cap_info_buffer;
} msg_req_raw_proc_request_t;


typedef struct _msg_ack_jpeg_encode_t {
	unsigned int output_buffer;
	unsigned int filesize;
	int status;
} msg_ack_jpeg_encode_t;

typedef enum {
	MODE_EIS_PRE = 0,
	MODE_EIS_VID,
	MODE_EIS_MULTI, /* v200 add two buffer warp request type */
	MODE_DMAP,
	MODE_EIS_4K_60FPS,
	MODE_MAX,
} warp_request_mode_e;

typedef struct grid_displacement {
	unsigned int x_offset;
	unsigned int y_offset;
} grid_displacement_t;

typedef struct cgrid_info {
	unsigned int cgrid_offset_h;
	unsigned int cgrid_offset_v;
	unsigned int cgrid_size_exp_h;
	unsigned int cgrid_size_exp_v;
	unsigned int cgrid_sector_h;
	unsigned int cgrid_sector_v;
	unsigned int fix_pt_precision;
	grid_displacement_t cgrid_xy_location[MAX_WARP_CGRID_POINT];
} cgrid_info_t;

typedef struct warp_image_info {
	unsigned int inputwidth;
	unsigned int inputheight;
	unsigned int outputwidth;
	unsigned int outputheight;
	unsigned int i_offset_h;
	unsigned int i_offset_v;
} warp_image_info_t;

typedef struct warp_info {
	cgrid_info_t grid_info;
	warp_image_info_t image_info;
} warp_info_t;

typedef struct warp_output_info {
	unsigned int isHFBC;
	stream_info_t output_info;
} warp_output_info_t;

typedef struct eis_tnr_info {
	unsigned int ae_gain;
	unsigned int chromatix_addr;
	unsigned int prev_buffer_addr;
	unsigned int curr_buffer_addr;
	isp_crop_region_info_t prev_crop_region;
	isp_crop_region_info_t curr_crop_region;
	unsigned int status;
} eis_tnr_info_t;

typedef enum {
	WARP_OUTPUT_DEFAULT = 0,
	WARP_OUTPUT_ARSR_ENABLE,
	WARP_OUTPUT_ARSR_DISABLE,
	WARP_OUTPUT_MAX,
} warp_output_module_e;

typedef struct _msg_req_warp_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	stream_info_t input_stream_info;
	stream_info_t warp_output_stream_info;
	unsigned int grid_enable;
	unsigned int grid_order;
	unsigned int cgrid_info_buffer;
	warp_request_mode_e mode;
	warp_output_info_t output_stream_info[2];
	eis_tnr_info_t tnr_info;
	warp_output_module_e arsr_ctrl_mode; /* 0: default 1: arsr enable 2: arsr disable */
} msg_req_warp_request_t;

typedef struct _msg_ack_warp_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	stream_info_t input_stream_info;
	unsigned int status;
	warp_request_mode_e mode;
	warp_output_info_t output_stream_info[2];
} msg_ack_warp_request_t;

typedef struct _msg_req_arsr_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	stream_info_t input_stream_info;
	stream_info_t output_stream_info;
	warp_request_mode_e mode;
	unsigned int status;
} msg_req_arsr_request_t;

typedef struct _msg_ack_arsr_request_t {
	unsigned int cam_id;
	unsigned int frame_number;
	stream_info_t input_stream_info;
	stream_info_t output_stream_info;
	unsigned int status;
} msg_ack_arsr_request_t;

typedef struct _dmap_output_info_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
} dmap_output_info_t;

typedef enum {
	DISP_HORIZONTAL = 0,
	DISP_VERTICAL_AND_FILP,
	DISP_HORIZONTAL_AND_FILP,
	DISP_VERTICAL,
} disp_direction_t;

typedef struct _msg_req_dmap_format_request_t {
	unsigned int dgen_output_bit;
	unsigned int dopt_output_bit;
	dmap_output_info_t output_info[DMAP_MAX_OUTPUT];
	unsigned int direction;
	unsigned int expansion;
	unsigned int dgen_input_format; // 0:yuv422+yuv422, 1:y+yuv422, 2:yuv422+y, 3:y+y
	unsigned int dopt_input_format; // 0:yuv422 1:y
	unsigned int req_switch_mask; // output 0x1:sparse disp;0x2:sparse depth;0x4:dense disp;0x8:dense depth;0x10:both warp;0x20:XYMAP
} msg_req_dmap_format_t;

typedef struct _msg_ack_dmap_format_request_t {
	unsigned int status;
} msg_ack_dmap_format_t;

typedef struct _dgen_algo_cfg_t {
	/* dmap func cfg & algo switch */
	int first_shift;

	unsigned char max_disp;

	unsigned char hist_en; /* V160 19 */
	unsigned char scanline_en;
	unsigned char scanline_var_en;
	unsigned char scanline_right_avail;
	unsigned char lrcheck_en;
	unsigned char median_filter_en;
	unsigned char median_conf_en;
	unsigned char var_gray_en;
	unsigned char var_thres_en;
	unsigned char rm_inval_disp_en;
	unsigned char remove_repet_enable;
	unsigned char rsvd1;
	unsigned char high_accuracy_en;
	unsigned char pmls_en;
	unsigned char subpixel_choice;
	/* dgen algo param */
	unsigned char multipass_cost_shift;
	unsigned char inval_disp_conf_threshold;
	unsigned short multipass_disp_threshold;

	unsigned short var_threshold;
	unsigned short var_threshold_low;
	unsigned short var_threshold_mid;
	unsigned short var_threshold_high;
	unsigned char var_gray_threshold_low;
	unsigned char var_gray_threshold_mid;
	unsigned char var_gray_threshold_high;

	unsigned char var_gray_shift;
	unsigned short var_gray_weight;
	unsigned char ad_y_shift;
	unsigned char census_shift;

	unsigned char robust_ad_x_a;
	unsigned char robust_ad_k_a;
	unsigned char robust_census_x_a;
	unsigned char robust_census_k_a;

	unsigned char valley_cost_truncate;
	unsigned char valley_cost_ratio;
	unsigned char valley_threshold_count_high;
	unsigned char valley_threshold_count_low;
	unsigned char valley_threshold_ratio;

	unsigned char v_mask_max;
	unsigned char h_mask_max;
	unsigned char v_mask_mid;
	unsigned char h_mask_mid;
	unsigned char yuv_threshold_high;
	unsigned char yuv_threshold_edge_high;
	unsigned char yuv_threshold_low;

	unsigned char rsvd2;
	unsigned char scanline_yuv_threshold;
	unsigned short scanline_penalty_1;
	unsigned short scanline_penalty_mid_1;
	unsigned short scanline_penalty_low_1;
	unsigned short scanline_penalty_2;
	unsigned short scanline_penalty_mid_2;
	unsigned short scanline_penalty_low_2;

	unsigned char disp_gap;
	unsigned char disp_gap_ratio;
	unsigned char disp_gap_ratio_shift;
	unsigned char lrcheck_threshold;

	unsigned char lrcheck_width_threshold;
	unsigned char rsvd3;
	unsigned short confidence_cost_threshold;
	unsigned char rsvd4[2];

	unsigned short conf_low_slop;
	unsigned short conf_high_slop;
	unsigned int conf_low_intercept;
	unsigned int conf_high_intercept;

	unsigned char histogram_threshold_ratio;
	unsigned char histogram_threshold_shift;
	unsigned short min_cost_threshold; /* V300 */

	unsigned char scanline_var_shift;
	unsigned char scanline_var_ratio;
	unsigned short scanline_var_thr;

	unsigned char row_start[4];
	unsigned char row_end[4];
	unsigned short row_num[4];
} dgen_algo_cfg_t;

typedef struct _dopt_algo_cfg_t {
	unsigned int of_enable;
	unsigned int occ_enable;
	unsigned int match_enable;
	unsigned int bilateral_enable;
	unsigned int gauss_enable;
	unsigned int depth_enable;
	unsigned int sc_enable;
	unsigned int blue_sky_enable;
	unsigned int dsmt_enable;

	unsigned int alpha;
	unsigned int beta;
	unsigned int gamma;
	unsigned int delta;
	unsigned int bilateral_alpha;
	unsigned int fix_offset;
	unsigned int fix_offset_ab;

	unsigned int sor_omega;
	unsigned int match_sub;
	unsigned int match_mul;
	unsigned int match_shift;
	unsigned int of_min_disp;
	unsigned int bilat_sigma_y;
	unsigned int bilat_sigma_disp;
	unsigned int disparity_max;
	unsigned int sor_iter[10];
	unsigned int outer_iter[10];
	unsigned int wx_mid_format;
	int of_right_image_shift;

	unsigned int occ_invalid_min_len;
	unsigned int occ_invalid_max_len;
	unsigned int occ_invalid_offset;
	unsigned int occ_disp_th;
	unsigned int occ_img_th;

	unsigned char bluesky_threshold_high;
	unsigned char bluesky_threshold_low;
	unsigned char bluesky_disp_default;

	unsigned short smooth_weight_lut[9];
	unsigned char dsmt_lut_idx_shift_bits;

	unsigned int depth_c;
	int depth_right_shift_pixel;
	unsigned int depth_max;

	unsigned int gauss_coeff[3];
	int deriv_coeff[3];
	int of_eta_s[10];
} dopt_algo_cfg_t;

typedef struct OIS2DCurve {
	float aa;
	float ab;
	float bb;
	float a;
	float b;
	float c;
} OIS2DCurve;

typedef struct OIS2DCurveXY {
	OIS2DCurve xHall;
	OIS2DCurve yHall;
} OIS2DCurveXY;

typedef struct OISInfo {
	OIS2DCurveXY hallCalibParas;
	float hallAccuracy;
	float normalize;
	int version;
	char moduleId[64];
	short srvOnHallX;
	short srvOnHallY;
	char srvOnHallValid;
	char reserved[3];
} OISInfo;

typedef enum {
	SUB_NEED_WARP = 0,
	MAIN_NEED_WARP,
	BOTH_NEED_WARP,
	FIRST_NEED_WARP,
	SECOND_NEED_WARP,
	THIRD_NEED_WARP,
	MAX_WARP_BASE,
} warp_base_t;

typedef struct _warp_distort_params_t {
	unsigned int distortvx;
	unsigned int distortvy;
	unsigned int width;
	unsigned int height;
	unsigned int fixed_precision;
} warp_distort_params_t;

typedef struct _warp_info_params_t {
	unsigned int fw_map_addr;
	OISInfo nvinfo;
	warp_distort_params_t distort_info;
	unsigned int warp_otp_base;
} warp_info_params_t;

typedef enum {
	LSC_ALGO_MODE_TYPE_NORMAL = 0,
	LSC_ALGO_MODE_TYPE_RAWSR,
} lsc_algo_scene_type_e;

typedef struct _lsc_capture_lv_info_t {
	unsigned int lsc_buf_addr;
	unsigned int sync_flag;
	lsc_algo_scene_type_e algo_scene;
	isp_crop_region_info_t crop_region;
	unsigned int input_setting_buffer;
} lsc_capture_lv_info_t;

typedef enum _dmap_rotation_type_t {
	NO_ROTATION = 0,
	ROTATION_LEFT,
	ROTATION_RIGHT,
	ROTATION_TYPE_MAX,
} dmap_rotation_type_t;

typedef enum _dmap_rotation_direction_t {
	ROTATION_CLOCKWISE = 0,
	ROTATION_ANTICLOCKWISE,
	ROTATION_DIRECTION_MAX,
} dmap_rotation_direction_t;

typedef struct _msg_req_dgen_request_t {
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int disp_direction;
	unsigned int dmap_crop_x;
	unsigned int dmap_crop_y;
	unsigned int dmap_crop_width;
	unsigned int dmap_crop_height;
	unsigned int input_left_buffer;
	unsigned int input_right_buffer;
	unsigned int output_left_raster;
	unsigned int output_disp_raster;
	unsigned int output_conf_raster;
	unsigned int output_disp_fw_addr;
	unsigned int output_conf_fw_addr;
} msg_req_dgen_request_t;

typedef struct _msg_ack_dgen_request_t {
	unsigned int output_disp_buffer;
	unsigned int output_conf_buffer;
	unsigned int output_left_raster;
	unsigned int input_left_buffer;
	unsigned int input_right_buffer;
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int out_stride;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int status;
} msg_ack_dgen_request_t;

typedef struct _msg_req_dopt_request_t {
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int disp_direction;
	unsigned int dmap_crop_x;
	unsigned int dmap_crop_y;
	unsigned int dmap_crop_width;
	unsigned int dmap_crop_height;
	unsigned int input_horz_left_image;
	unsigned int input_vert_left_image;
	unsigned int input_raster_disp_buffer;
	unsigned int input_last_left_buffer;
	unsigned int input_last_disp_buffer;
	unsigned int output_disp_buffer;
} msg_req_dopt_request_t;

typedef struct _msg_ack_dopt_request_t {
	unsigned int output_disp_buffer;
	unsigned int input_horz_left_image;
	unsigned int input_vert_left_image;
	unsigned int input_raster_disp_buffer;
	unsigned int input_last_left_buffer;
	unsigned int input_last_disp_buffer;
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int out_stride;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int status;
} msg_ack_dopt_request_t;

typedef struct _msg_req_drbr_request_t {
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int dmap_crop_x;
	unsigned int dmap_crop_y;
	unsigned int dmap_crop_width;
	unsigned int dmap_crop_height;
	unsigned int input_buffer;
	unsigned int output_buffer;
	unsigned int image_width;
	unsigned int image_height;
	unsigned int data_type;
	unsigned int mode;
	unsigned int read_flip;
	unsigned int write_flip;
	unsigned int rotation;
	unsigned int rub_dist;
	unsigned int b2r_expansion;
} msg_req_drbr_request_t;

typedef struct _msg_ack_drbr_request_t {
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int status;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int image_width;
	unsigned int image_height;
	unsigned int output_buffer;
	unsigned int bit_num;
	unsigned int rotation;
	unsigned int read_flip;
	unsigned int write_flip;
	unsigned int mode;
} msg_ack_drbr_request_t;

typedef struct _msg_req_dmap_request_t {
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int disp_direction;
	unsigned int sparse_enable;
	unsigned int dense_enable;
	unsigned int warp_enable;
	unsigned int rotation_type;
	unsigned int rotation_direction;
	unsigned int dmap_crop_x;
	unsigned int dmap_crop_y;
	unsigned int dmap_crop_width;
	unsigned int dmap_crop_height;
	unsigned int input_dgen_left;
	unsigned int input_dgen_right;
	unsigned int output_dopt_left_ds; /* L0~L9 */
	unsigned int output_dopt_right_ds; /* L0~L9 */
	unsigned int output_dopt_disp_ds; /* L0~L9 */
	unsigned int output_block_disp;	/* dgen_block_disp */
	unsigned int output_hist_stat_addr; /* hist stat */
	unsigned int output_sparse_disp;
	unsigned int output_conf_raster;
	unsigned int output_conf_temp;
	unsigned int output_sparse_depth;
	unsigned int output_disp_fw_addr;
	unsigned int output_conf_fw_addr;
	unsigned int input_dopt_left_ds; /* L0~L9 */
	unsigned int input_dopt_right_ds; /* L0~L9 */
	unsigned int input_dopt_disp_ds; /* L0~L9 */
	unsigned int input_block_disp; /* dgen_block_disp */
	unsigned int input_hist_stat_addr; /* hist stat */
	unsigned int input_dopt_horz_left;
	unsigned int input_last_left_buffer;
	unsigned int input_last_disp_buffer;
	unsigned int output_dense_horz_disp;
	unsigned int output_dense_depth;
	unsigned int output_dense_xmap;
	unsigned int output_dense_ymap;
	unsigned int input_warp_sparse_setting_buffer;
	unsigned int input_warp_dense_setting_buffer;
} msg_req_dmap_request_t;

typedef struct _msg_ack_dmap_request_t {
	unsigned int output_sparse_disp;
	unsigned int output_conf_raster;
	unsigned int output_conf_temp;
	unsigned int output_sparse_depth;
	unsigned int input_dgen_left;
	unsigned int input_dgen_right;
	unsigned int output_dense_horz_disp;
	unsigned int output_dense_depth;
	unsigned int output_dense_xmap;
	unsigned int output_dense_ymap;
	unsigned int input_dopt_horz_left;
	unsigned int input_last_left_buffer;
	unsigned int input_last_disp_buffer;
	unsigned int input_warp_sparse_setting_buffer;
	unsigned int input_warp_dense_setting_buffer;
	unsigned int input_dopt_left_ds; /* L0~L9 */
	unsigned int input_dopt_right_ds; /* L0~L9 */
	unsigned int input_dopt_disp_ds; /* L0~L9 */
	unsigned int input_block_disp; /* dgen_block_disp */
	unsigned int input_hist_stat_addr; /* hist */
	unsigned int output_dopt_left_ds; /* L0~L9 */
	unsigned int output_dopt_right_ds; /* L0~L9 */
	unsigned int output_dopt_disp_ds; /* L0~L9 */
	unsigned int output_block_disp; /* dgen_block_disp */
	unsigned int output_hist_stat_addr; /* hist */
	unsigned int base_img;
	unsigned int frame_number;
	unsigned int req_type;
	unsigned int out_stride;
	unsigned int timestampL;
	unsigned int timestampH;
	unsigned int warp_flag;
	unsigned int status;
} msg_ack_dmap_request_t;

typedef struct _msg_req_dmap_map_t {
	unsigned int fw_cfg_addr;
	unsigned int isp_buf_addr;
	unsigned int cfg_mem_size;
	unsigned int buf_mem_size;
	unsigned int width;
	unsigned int height;
	unsigned int mode;
} msg_req_dmap_map_t;

typedef struct _msg_ack_dmap_map_t {
	unsigned int status;
} msg_ack_dmap_map_t;

typedef struct _msg_req_dmap_unmap_t {
	unsigned int unmap_cfg_addr;
	unsigned int unmap_buf_addr;
} msg_req_dmap_unmap_t;

typedef struct _msg_ack_dmap_unmap_t {
	unsigned int status;
} msg_ack_dmap_unmap_t;

typedef struct _msg_req_dmap_offline_map_t {
	unsigned int isp_fw_addr;
	unsigned int isp_buf_addr;
	unsigned int isp_fw_mem_size;
	unsigned int isp_buf_mem_size;
	unsigned int width;
	unsigned int height;
	unsigned int mode; /* dmap_scene_e: defalut = 1, no dense buffer */
} msg_req_dmap_offline_map_t;

typedef struct _msg_ack_dmap_offline_map_t {
	unsigned int status;
} msg_ack_dmap_offline_map_t;

typedef struct _msg_req_dmap_offline_unmap_t {
	unsigned int unmap_isp_fw_addr;
	unsigned int unmap_buf_addr;
} msg_req_dmap_offline_unmap_t;

typedef struct _msg_ack_dmap_offline_unmap_t {
	unsigned int status;
} msg_ack_dmap_offline_unmap_t;

typedef struct _msg_req_dgen_flush_t {
	int flag;
} msg_req_dgen_flush_t;

typedef struct _msg_ack_dgen_flush_t {
	unsigned int status;
} msg_ack_dgen_flush_t;

typedef struct _msg_req_dopt_flush_t {
	int flag;
} msg_req_dopt_flush_t;

typedef struct _msg_ack_dopt_flush_t {
	unsigned int status;
} msg_ack_dopt_flush_t;

typedef struct _map_pool_desc_t{
	unsigned int start_addr;
	unsigned int ion_iova;
	unsigned int size;
	unsigned int usage;
} map_pool_desc_t;

typedef struct _msg_req_dmap_flush_t {
	int flag;
} msg_req_dmap_flush_t;

typedef struct _msg_ack_dmap_flush_t {
	unsigned int status;
} msg_ack_dmap_flush_t;

typedef struct _msg_req_map_buffer_t {
	unsigned int cam_id;
	unsigned int pool_count;
	map_pool_desc_t map_pool[MAP_POOL_USAGE_MAX];
} msg_req_map_buffer_t;

typedef struct _msg_req_map_buffer_offline_t {
	unsigned int cam_id;
	unsigned int pool_count;
	map_pool_desc_t map_pool[MAP_POOL_USAGE_MAX];
} msg_req_map_buffer_offline_t;

typedef struct _msg_req_raw_proc_map_buffer_t {
	unsigned int cam_id;
	unsigned int pool_count;
	map_pool_desc_t map_pool[MAP_POOL_USAGE_MAX];
} msg_req_raw_proc_map_buffer_t;

typedef struct _msg_ack_map_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_map_buffer_t;

typedef struct _msg_ack_map_buffer_offline_t {
	unsigned int cam_id;
	int status;
} msg_ack_map_buffer_offline_t;

typedef struct _msg_ack_raw_proc_map_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_raw_proc_map_buffer_t;

typedef struct _msg_req_unmap_buffer_t {
	unsigned int cam_id;
	unsigned int buffer;
} msg_req_unmap_buffer_t;

typedef struct _msg_req_unmap_buffer_offline_t {
	unsigned int cam_id;
	unsigned int buffer;
} msg_req_unmap_buffer_offline_t;

typedef struct _msg_req_raw_proc_unmap_buffer_t {
	unsigned int cam_id;
	unsigned int buffer;
} msg_req_raw_proc_unmap_buffer_t;

typedef struct _msg_ack_unmap_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_unmap_buffer_t;

typedef struct _msg_req_dynamic_map_buffer_t {
	unsigned int cam_id;
	unsigned int pool_count;
	map_pool_desc_t map_pool[MAP_POOL_USAGE_MAX];
} msg_req_dynamic_map_buffer_t;

typedef struct _msg_ack_dynamic_map_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_dynamic_map_buffer_t;

typedef struct _msg_req_dynamic_unmap_buffer_t {
	unsigned int cam_id;
	unsigned int buffer;
} msg_req_dynamic_unmap_buffer_t;

typedef struct _msg_ack_dynamic_unmap_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_dynamic_unmap_buffer_t;

typedef struct _msg_req_tnr_dynamic_map_buffer_t {
	unsigned int cam_id;
	unsigned int pool_count;
	map_pool_desc_t map_pool[MAP_POOL_USAGE_MAX];
} msg_req_tnr_dynamic_map_buffer_t;

typedef struct _msg_ack_tnr_dynamic_map_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_tnr_dynamic_map_buffer_t;

typedef struct _msg_req_tnr_dynamic_unmap_buffer_t {
	unsigned int cam_id;
	unsigned int buffer;
} msg_req_tnr_dynamic_unmap_buffer_t;

typedef struct _msg_ack_tnr_dynamic_unmap_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_tnr_dynamic_unmap_buffer_t;


typedef struct _msg_ack_unmap_buffer_offline_t {
	unsigned int cam_id;
	int status;
} msg_ack_unmap_buffer_offline_t;

typedef struct _msg_ack_raw_proc_unmap_buffer_t {
	unsigned int cam_id;
	int status;
} msg_ack_raw_proc_unmap_buffer_t;

typedef struct _msg_req_cal_data_t {
	unsigned int cam_id;
	unsigned int buffer_size;
	unsigned int cal_data_buffer;
} msg_req_cal_data_t;

typedef struct _msg_ack_cal_data_t {
	unsigned int cam_id;
	int status;
} msg_ack_cal_data_t;

typedef struct _isp_reg_info_t {
	unsigned int register_address;
	unsigned int register_value;
} isp_reg_info_t;

typedef struct _msg_req_set_isp_regs_t {
	unsigned int register_type;
	unsigned int register_count;
	isp_reg_info_t reg_info[MAX_SET_ISP_NR];
} msg_req_set_isp_regs_t;

typedef struct _msg_ack_set_isp_regs_t {
	int status;
} msg_ack_set_isp_regs_t;

typedef struct _msg_req_get_isp_regs_t {
	unsigned int register_count;
	unsigned int register_address[MAX_GET_ISP_NR];
} msg_req_get_isp_regs_t;

typedef struct _msg_ack_get_isp_regs_t {
	int status;
	unsigned int register_count;
	unsigned int register_value[MAX_GET_ISP_NR];
} msg_ack_get_isp_regs_t;

typedef struct _i2c_reg_set_info_t {
	unsigned int register_address;
	unsigned int register_value;
	unsigned char length; /**< value length */
} i2c_reg_set_info_t;

typedef struct _msg_req_set_i2c_regs_t {
	unsigned int register_type;
	unsigned int slave_address;
	unsigned int register_count;
	i2c_reg_set_info_t reg_info[MAX_SET_I2C_NR];
} msg_req_set_i2c_regs_t;

typedef struct _msg_ack_set_i2c_regs_t {
	int status;
} msg_ack_set_i2c_regs_t;

typedef struct _i2c_reg_get_info_t {
	unsigned int register_address;
	unsigned char length; /**< request getting the len of the register value */
} i2c_reg_get_info_t;

typedef struct _msg_req_get_i2c_regs_t {
	unsigned int register_type;
	unsigned int slave_address;
	unsigned int register_count;
	i2c_reg_get_info_t reg_info[MAX_GET_I2C_NR];
} msg_req_get_i2c_regs_t;

typedef struct _msg_ack_get_i2c_regs_t {
	int status;
	unsigned int register_count;
	unsigned int register_value[MAX_GET_I2C_NR];
} msg_ack_get_i2c_regs_t;

typedef struct _msg_req_test_case_interface_t {
	unsigned int case_handle;
	char case_name[64];
} msg_req_test_case_interface_t;

typedef struct _msg_ack_test_case_interface_t {
	int status;
	int flag;
} msg_ack_test_case_interface_t;

typedef struct _msg_req_flush_t {
	unsigned int cam_id;
	unsigned int is_hotplug;
} msg_req_flush_t;

typedef struct _msg_ack_flush_t {
	int status;
} msg_ack_flush_t;

typedef struct _msg_req_inv_tlb_t {
	int flag;
} msg_req_inv_tlb_t;

typedef struct _msg_ack_inv_tlb_t {
	int status;
} msg_ack_inv_tlb_t;

typedef struct _msg_req_query_ois_update_t {
	unsigned int cam_id;
	char sensor_name[32];
} msg_req_query_ois_update_t;

typedef struct _msg_ack_query_ois_update_t {
	unsigned int cam_id;
	int status;
} msg_ack_query_ois_update_t;

typedef struct _msg_req_ois_update_t {
	unsigned int cam_id;
	char sensor_name[32];
	unsigned int input_ois_buffer;
	unsigned int input_ois_buffer_size;
} msg_req_ois_update_t;

typedef struct _msg_ack_ois_update_t {
	unsigned int cam_id;
	int status;
} msg_ack_ois_update_t;

typedef struct _msg_req_mem_pool_init_t {
	unsigned int mempool_addr;
	unsigned int mempool_size;
	unsigned int mempool_prot;
} msg_req_mem_pool_init_t;

typedef struct _msg_ack_mem_pool_init_t {
	unsigned int status;
} msg_ack_mem_pool_init_t;

typedef struct _msg_req_mem_pool_deinit_t {
	unsigned int mempool_addr;
	unsigned int mempool_size;
	unsigned int mempool_prot;
} msg_req_mem_pool_deinit_t;

typedef struct _msg_ack_mem_pool_deinit_t {
	unsigned int status;
} msg_ack_mem_pool_deinit_t;

typedef struct _msg_req_isp_cpu_poweroff_t {
	int flag;
} msg_req_isp_cpu_poweroff_t;

typedef struct _msg_ack_isp_cpu_poweroff_t {
	int status;
} msg_ack_isp_cpu_poweroff_t;

typedef enum {
	MOTION_SENSOR_ACCEL = 1,
	MOTION_SENSOR_GYRO = 4,
	MOTION_SENSOR_LINEAR_ACCEL = 10,
} motion_sensor_type_t;

typedef struct _msg_req_motion_sensor_map_t {
	motion_sensor_type_t motion_sensor_type;
	unsigned int input_motion_sensor_mem_buffer;
	unsigned int input_motion_sensor_mem_buffer_size;
} msg_req_motion_sensor_map_t;

typedef struct _msg_ack_motion_sensor_map_t {
	motion_sensor_type_t motion_sensor_type;
	int status;
} msg_ack_motion_sensor_map_t;

typedef struct _msg_req_set_pd_key_t {
	unsigned short setVal;
} msg_req_set_pd_key;

typedef struct _msg_req_get_pd_key_t {
	unsigned short getVal1;
	unsigned short getVal2;
} msg_req_get_pd_key;

typedef struct _pdaf_sensor_coord_t {
	unsigned int img_orientation_h;
	unsigned int img_orientation_v;
	unsigned int x_add_sta;
	unsigned int y_add_sta;
	unsigned int x_add_end;
	unsigned int y_add_end;
	unsigned int dig_crop_x_offset;
	unsigned int dig_crop_y_offset;
	unsigned int binning_type_h;
	unsigned int binning_type_v;
	unsigned int x_out_size;
	unsigned int y_out_size;
	unsigned int dig_crop_image_width;
	unsigned int dig_crop_image_height;
} pdaf_sensor_coord;

typedef enum {
	DEMOSAIC_ONLINE = 0,
	DEMOSAIC_OFFLINE,
	REMOSAIC_OFFLINE,
	REQUEST_RAW2YUV_MAX,
} raw2yuv_req_mode_e;

typedef struct _msg_req_raw2yuv_start_t {
	unsigned int cam_id;
	unsigned int csi_index;
	unsigned int i2c_index;
	char sensor_name[NAME_LEN];
	char product_name[NAME_LEN];
	unsigned int input_calib_buffer;
	raw2yuv_req_mode_e raw2yuv_mode;
} msg_req_raw2yuv_start_t;


typedef struct _msg_ack_raw2yuv_start_t {
	unsigned int cam_id;
} msg_ack_raw2yuv_start_t;

typedef struct _msg_req_raw2yuv_stop_t {
	unsigned int cam_id;
} msg_req_raw2yuv_stop_t;

typedef struct _msg_ack_raw2yuv_stop_t {
	unsigned int cam_id;
} msg_ack_raw2yuv_stop_t;

typedef struct _msg_req_raw_proc_start_t {
	unsigned int cam_id;
	unsigned int csi_index;
	unsigned int i2c_index;
	char sensor_name[NAME_LEN];
	char product_name[NAME_LEN];
	unsigned int input_calib_buffer;
} msg_req_raw_proc_start_t;

typedef struct _msg_ack_raw_proc_start_t {
	unsigned int cam_id;
} msg_ack_raw_proc_start_t;

typedef struct _msg_req_raw_proc_stop_t {
	unsigned int cam_id;
} msg_req_raw_proc_stop_t;

typedef struct _msg_ack_raw_proc_stop_t {
	unsigned int cam_id;
} msg_ack_raw_proc_stop_t;

typedef struct _msg_req_query_driver_ic_t {
	unsigned int i2c_index;
	unsigned int ic_postion;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
} msg_req_query_driver_ic_t;

typedef struct _msg_ack_driver_ic_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
} msg_ack_query_driver_ic_t;

typedef struct _msg_req_query_dot_projector_t {
	unsigned int i2c_index;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
} msg_req_query_dot_projector_t;

typedef struct _msg_ack_dot_projector_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
} msg_ack_query_dot_projector_t;

typedef struct _msg_req_acquire_driver_ic_t {
	unsigned int i2c_index;
	unsigned int ic_postion;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
} msg_req_acquire_driver_ic_t;

typedef struct _msg_ack_acquire_driver_ic_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
} msg_ack_acquire_driver_ic_t;

typedef struct _msg_req_acquire_dot_projector_t {
	unsigned int i2c_index;
	char product_name[NAME_LEN];
	char name[NAME_LEN];
	unsigned int input_otp_buffer;
	unsigned int buffer_size;
} msg_req_acquire_dot_projector_t;

typedef struct _msg_ack_acquire_dot_projector_t {
	char name[NAME_LEN];
	unsigned char revision;
	int status;
} msg_ack_acquire_dot_projector_t;

typedef struct _msg_req_release_driver_ic_t {
	unsigned int i2c_index;
	unsigned int ic_position;
	char name[NAME_LEN];
} msg_req_release_driver_ic_t;

typedef struct _msg_ack_release_driver_ic_t {
	unsigned int i2c_index;
} msg_ack_release_driver_ic_t;

typedef struct _msg_req_release_dot_projector_t {
	unsigned int i2c_index;
} msg_req_release_dot_projector_t;

typedef struct _msg_ack_release_dot_projector_t {
	unsigned int i2c_index;
} msg_ack_release_dot_projector_t;

typedef enum {
	SUBCMD_ES_WATCHDOG = 0,
	SUBCMD_ES_SYNCLOG = 1,
	SUBCMD_SET_M_DF_FLAG = 2,
	SUBCMD_SET_DF_TUNING = 3,
	SUBCMD_SET_COLOR_BAR = 4,
	SUBCMD_ENABLE_TNR = 5,
	SUBCMD_ENABLE_DIS = 6,
	SUBCMD_ENABLE_FD = 7,
	SUBCMD_SET_FACE = 8,
	SUBCMD_AE_ANTIBANDING_MODE = 9,
	SUBCMD_AE_EXPOSURE_COMPENSATION = 10,
	SUBCMD_AE_LOCK = 11,
	SUBCMD_AE_MODE = 12,
	SUBCMD_AE_REGIONS = 13,
	SUBCMD_AE_TARGET_FPS_RANGE = 14,
	SUBCMD_AE_PRECAPTURE_TRIGGER = 15,
	SUBCMD_AF_MODE = 16,
	SUBCMD_AF_REGIONS = 17,
	SUBCMD_AF_TRIGGER = 18,
	SUBCMD_FLASH_MODE = 19,
	SUBCMD_AWB_LOCK = 20,
	SUBCMD_AWB_MODE = 21,
	SUBCMD_AWB_REGIONS = 22,
	SUBCMD_SCALER_CROP_REGION = 23,
	SUBCMD_START_CAPTURE = 24,
	SUBCMD_STOP_CAPTURE = 25,
	SUBCMD_SET_DEBUG_OPEN = 26,
	SUBCMD_SET_FLASH_RATIO = 27,
	SUBCMD_SET_MANUAL_FOCUS_MODE = 28,
	SUBCMD_SET_VCM_CODE = 29,
	SUBCMD_SET_BANDING_MSG = 30,
	SUBCMD_SET_EXPO_TIME = 31,
	SUBCMD_SET_ISO = 32,
	SUBCMD_SET_ADGAIN = 33,
	SUBCMD_SET_MANUAL_AWB = 34,
	SUBCMD_SET_SCENE_MODE = 35,
	SUBCMD_SET_OVER_EXPOSURE = 36,
	SUBCMD_SET_DEBUG_SHADING = 37,
	SUBCMD_RESUME_3A = 38,
	SUBCMD_SET_CAPTURE_SHARPNESS = 39,
	SUBCMD_SET_CAPTURE_RAWNF = 40,
	SUBCMD_SET_CAPTURE_YUVNF = 41,
	SUBCMD_SET_CAPTURE_GCD = 42,
	SUBCMD_SET_SALIENCY_RESULT = 43,
	SUBCMD_SET_PANORAMA_MODE = 44,
	SUBCMD_SET_PANORAMA_LOCK = 45,
	SUBCMD_SET_FAST_SNAPSHOT = 46,
	SUBCMD_SET_SATURATION = 47,
	SUBCMD_SET_CONTRAST = 48,
	SUBCMD_SET_BRIGHTNESS = 49,
	SUBCMD_SET_GSENSOR_INFO = 50,
	SUBCMD_SET_7CM_FOCUS_MODE = 51,
	SUBCMD_SET_7CM_FOCUS_REGIONS = 52,
	SUBCMD_SET_BURST_COUNT = 53,
	SUBCMD_SET_TARGET_TRACKING = 54,
	SUBCMD_SET_OIS_MODE = 55,
	SUBCMD_ENABLE_FBCD = 56,
	SUBCMD_TRY_AE = 57,
	SUBCMD_SET_CAPTURE_AE = 58,
	SUBCMD_SET_TARGET_LUMINANCE = 59,
	SUBCMD_GET_CAPTURE_VALID_INFO = 60,
	SUBCMD_CAMERA_MODE = 61,
	SUBCMD_SET_OTP_CALIBRATION = 62,
	SUBCMD_SET_MOTIONSENSOR_INFO = 63,
	SUBCMD_PDAF_MMI_TEST_ENABLE = 64,
	SUBCMD_SET_PDAF_MMI_PARAM = 65,
	SUBCMD_SET_EQUIP_MMI_MODE = 66,
	SUBCMD_SET_FLASH_MMI_MODE = 67,
	SUBCMD_SET_MMI_TEST_GAMMA = 68,
	SUBCMD_SET_PROFESSION_CAMERA = 69,
	SUBCMD_SET_METERING_MODE = 70,
	SUBCMD_SET_WB_VALUE = 71,
	SUBCMD_LPD_ENABLE = 72,
	SUBCMD_SET_AE_GAIN = 73,
	SUBCMD_AWB_ILLUMINANT = 74,
	SUBCMD_AWB_DAMPING_PARAM = 75,
	SUBCMD_AWB_CURRENT_WP = 76,
	SUBCMD_GAMMA_LOCK = 77,
	SUBCMD_GAMMA_MODE = 78,
	SUBCMD_GAMMA_CURVE = 79,
	SUBCMD_GAMMA_DAMPING_PARAM = 80,
	SUBCMD_LSC_ENABLE = 81,
	SUBCMD_DPCC_ENABLE = 82,
	SUBCMD_YUVNF_ENABLE = 83,
	SUBCMD_SHARPNESS_ENABLE = 84,
	SUBCMD_RAWNF_ENABLE = 85,
	SUBCMD_DRC_ENABLE = 86,
	SUBCMD_DRC_MODE = 87,
	SUBCMD_DRC_DAMPING_PARAM = 88,
	SUBCMD_CC_ENABLE = 89,
	SUBCMD_CC_MODE = 90,
	SUBCMD_CC_PARAM = 91,
	SUBCMD_CC_MATRIX = 92,
	SUBCMD_SMART_AE_SET_EXPO_COMPENSATION = 93,
	SUBCMD_SET_OIS_MMI_MODE = 94,
	SUBCMD_RESUME_VCM_CODE = 95,
	SUBCMD_PROF_FOCUS_ASSIST_MODE = 96,
	SUBCMD_YUV_CONTRAST_RESULT = 97,
	SUBCMD_SET_FACE_INFO = 98,
	SUBCMD_LASER_CONTROL = 99,
	SUBCMD_USECASE_INITIAL = 100,
	SUBCMD_SET_DC_MMI_ENABLE = 101,
	SUBCMD_SET_AF_MESSAGE = 102,
	SUBCMD_CAMERA_FIRMWARE_PROPERTY = 103,
	SUBCMD_SET_DUAL_CAM_SHELTERED = 104,
	SUBCMD_DEPTH_INFO = 105,
	SUBCMD_SET_MMI_7CM_PARAM = 106,
	SUBCMD_AE_YUV_INFO = 107,
	SUBCMD_SET_FILL_RAW = 108,
	SUBCMD_SET_ANDROID_ISO = 109,
	SUBCMD_SET_ANDROID_EXPO_TIME = 110,
	SUBCMD_SET_FOCUS_DISTANCE = 111,
	SUBCMD_SET_TONEMAP_MODE = 112,
	SUBCMD_SET_TONEMAP_CURVE = 113,
	SUBCMD_SET_APERTURE_MODE = 114,
	SUBCMD_SET_STD_RAW = 115,
	SUBCMD_SET_CAPFLASH_ON = 116,
	SUBCMD_SET_AFC_DATA = 117,
	SUBCMD_SET_CC_MODE = 118,
	SUBCMD_SET_CC_TRANSFORM = 119,
	SUBCMD_SET_CC_GAINS = 120,
	SUBCMD_SET_CONTROL_MODE = 121,
	SUBCMD_SET_AF_DIRECT_TRANS_BASE = 122,
	SUBCMD_SET_CURVE_MODE = 123,
	SUBCMD_SET_RGB2YUV_MODE = 124,
	SUBCMD_SET_RGB2YUV_PARAM = 125,
	SUBCMD_SET_AF_OTP_CALIB_DATA = 126,
	SUBCMD_SET_SATURATION_COMPENSATION = 127,
	SUBCMD_SET_LOG_LEVEL = 128,
	SUBCMD_SET_AFC_MMI_ENABLE = 129,
	SUBCMD_ENABLE_WARP = 130,
	SUBCMD_SET_STREAM_MODE = 131,
	SUBCMD_SET_DGEN_ALGO = 132,
	SUBCMD_SET_WARP_INFO = 133,
	SUBCMD_SET_DOPT_ALGO = 134,
	SUBCMD_GET_LCD_STATE = 135,
	SUBCMD_SET_LOG_MODULE = 136,
	SUBCMD_SET_DMAP_CAP_SIZE = 137,
	SUBCMD_SET_PLATFORM_ID = 138,
	/* tmp */
	SUBCMD_SET_FLASH_MODE = 139,
	SUBCMD_SET_LASER_DIRTY = 140,
	SUBCMD_SET_FACE_LANDMARKS = 141,
	SUBCMD_SET_AE_ALWAYS_CONVERGE = 142,
	SUBCMD_SET_DMAP_INFO = 143,
	SUBCMD_SET_PDAF_RESULT = 144,
	SUBCMD_SET_SELF_LEARN_DATA = 145,
	SUBCMD_SET_SFR_TEST_DATA = 146,
	SUBCMD_SET_SCE_HUE_GAIN = 147,
	SUBCMD_SET_MOTION_INFO = 148,
	SUBCMD_ENABLE_EIS = 149,
	SUBCMD_SET_REDUCE_EXPOSURE = 150,
	SUBCMD_SET_APERTURE_MONO = 151,
	SUBCMD_SET_LSC_MODE = 152,
	SUBCMD_SET_SEAMLESS_INFO = 153,
	SUBCMD_SET_RAW2YUV_INFO = 154,
	SUBCMD_SET_ISP_ALGO_STATUS = 155,
	SUBCMD_SET_RAWNF_DS = 156,
	SUBCMD_SET_YUVNF_DS = 157,
	SUBCMD_SET_AF_OTPSTART_MODE = 158,
	SUBCMD_FOV_SCALE_RATIO_STATUS = 159,
	SUBCMD_SET_HFBC_ALIGMENT = 160,
	SUBCMD_SET_PD_OFFSET_CALIB_MMI_ENABLE = 161,
	SUBCMD_SET_OPTICAL_ZOOM_SWITCH = 162,
	SUBCMD_STREAM_REF_VALUE = 163,
	SUBCMD_SET_PD_OFFSET_CALIB_RESULT = 164,
	SUBCMD_SET_FILL_RAWNFDS = 165,
	SUBCMD_SET_DRC_MODE = 166,
	SUBCMD_SET_WARP_SELFLEARN = 167,
	SUBCMD_LASER_RAWDATA = 168,
	SUBCMD_SD_RESULTS = 169,
	SUBCMD_SET_LCD_RATIO = 170,
	SUBCMD_SET_CAPLCD_ON = 171,
	SUBCMD_SET_LCD_COMPENSATE_MODE = 172,
	SUBCMD_SAVE_PREVIEW_AE_AWB = 173,
	SUBCMD_SET_NATIVE_CAMERA = 174,
	SUBCMD_SET_RAW_READBACK_ADDR = 175,
	SUBCMD_SET_SOFTLIGHT_MODE = 176,
	SUBCMD_LASER_VERSION = 177,
	SUBCMD_SET_PDALGO_ENABLE = 178,
	SUBCMD_SET_PD_INFO = 179,
	SUBCMD_SET_SWPD_KEY = 180,
	SUBCMD_GET_SWPD_KEY = 181,
	SUBCMD_GET_SENSOR_COORD = 182,
	SUBCMD_SET_CC_SAT_VAL = 183,
	SUBCMD_SET_LUT3D_MODE = 184,
	SUBCMD_SET_FORCE_CAF = 185,
	SUBCMD_SET_AE_SENSOR_VERIFY_MODE = 186,

	/* front camera awb */
	SUBCMD_SET_AP_AWB_GAIN = 187,
	SUBCMD_SET_AP_AWB_WP = 188,
	SUBCMD_SET_AP_AWB_COLOR_ZONE = 189,
	SUBCMD_SET_AP_AWB_INIT_PARAM = 190,
	SUBCMD_SET_RAW2YUV_OFFLINE_INFO = 191,
	SUBCMD_SET_PREVIEW_CAMERA = 192,
	SUBCMD_SET_AWB_SENSOR_VALUE = 193,
	SUBCMD_SET_COLOR_MODE = 194,
	SUBCMD_SET_SECOND_AFC_DATA = 195,
	SUBCMD_SET_SECOND_SFR_TEST_DATA = 196,
	SUBCMD_SET_OIS_POSITION = 197,
	SUBCMD_MANUAL_MAX_EXPO_TIME = 198,
	SUBCMD_SET_AF_ALWAYS = 199,
	SUBCMD_SET_AFSTAT_ALGO_RESULT = 200,
	SUBCMD_SET_MASTER_AI_MODE =201,
	SUBCMD_SET_LCD_FLASH_MODE = 202,
	SUBCMD_SET_FILL_STREAM = 203,
	SUBCMD_SET_IR_FLASH_MODE = 204,
	SUBCMD_SET_FACTORY_MODE = 205,
	SUBCMD_SET_SL_MODE = 206,
	SUBCMD_SET_TOF_DATA = 207,
	SUBCMD_SET_TRACKING_INFO = 208,
	SUBCMD_SET_WARP_DF_INFO = 209,
	SUBCMD_SET_AIQ_INFO = 210,
	SUBCMD_SMART_ZOOM_ENABLE = 211,
	SUBCMD_IR_LIGHT_SWITCH_MODE = 212,
	SUBCMD_SET_UNDER_WATER_MODE = 213,
	SUBCMD_SET_AF_DEPTH_DATA = 214,
	SUBCMD_TOF_CONFIG = 215,
	SUBCMD_TOF_SET_CALIB_DATA = 216,
	SUBCMD_TOF_GET_CALIB_DATA = 217,
	SUBCMD_SET_SCD_INFO = 218,
	SUBCMD_SET_FLK_INFO = 219,
	SUBCMD_SET_DPCC_MODE = 220,
	SUBCMD_SET_RAW_PROCESS_GLOBAL_CFG = 221,
	SUBCMD_SET_RAW_PROCESS_INFO = 222,
	SUBCMD_SET_SENSORHDR_SEAMLESS = 223,
	SUBCMD_SET_RAW2RGB_OFFLINE_INFO = 224,
	SUBCMD_SET_RGB2YUV_OFFLINE_INFO = 225,
	SUBCMD_SET_CAP_LSC_PARAM = 226,
	SUBCMD_SET_DISABLE_TAE = 227,
	SUBCMD_SET_CAM_FOV_RATIO = 228,
	SUBCMD_SET_RAW2YUV_OFFLINE_ALGO_PARAM = 229,
	SUBCMD_SET_RAWINFO_DUMP = 230,
	SUBCMD_SET_MIRROR_FLIP_MODE = 231,
	SUBCMD_SET_FAE_DISABLE = 232,
	SUBCMD_SET_ZSLHDR_MODE = 233,
	SUBCMD_SET_BLIE_INFO = 234,
	SUBCMD_AP_AWB_RUN_ENABLE = 235,
	SUBCMD_AP_AWB_OUTPUT_INFO_CMD = 236,
	SUBCMD_AP_AUTOCLS_RUN_ENABLE = 237,
	SUBCMD_AP_AUTOCLS_OUTPUT_INFO_CMD = 238,
	SUBCMD_MAX,
} extendset_info_e;

typedef enum {
	SUBCMD_EG_LLT,
	SUBCMD_EG_ST,
	SUBCMD_GET_M_DF_FLAG,
	SUBCMD_GET_DF_TUNING,
} extendget_info_e;

typedef enum {
	ALGO_ID_NULL = 0,

	/* special algo */
	ALGO_ID_FD,
	ALGO_ID_OIS,
	ALGO_ID_AE,
	ALGO_ID_PP2PD,
	ALGO_ID_AF,
	ALGO_ID_FLASH,

	/* FE algo */
	ALGO_ID_BLC,
	ALGO_ID_DGAMMA,
	ALGO_ID_FESCL,
	ALGO_ID_DPC,
	ALGO_ID_LSCTOP,
	ALGO_ID_STAT3A,
	ALGO_ID_AWB,
	ALGO_ID_AUTOCLS,
	ALGO_ID_MINILSC,
	ALGO_ID_SD,
	ALGO_ID_RAWNFSTAT,

	/* RAW algo */
	ALGO_ID_RAWNF,
	ALGO_ID_BLC_GRID,
	ALGO_ID_LSC,
	ALGO_ID_AWBGAIN,
	ALGO_ID_GCD,
	ALGO_ID_RGBSCL,
	ALGO_ID_TINY,
	ALGO_ID_LBC,
	ALGO_ID_YSCL,

	/* BE algo */
	ALGO_ID_CC,
	ALGO_ID_DRC,
	ALGO_ID_GAMMA,
	ALGO_ID_RGBLUT3D,
	ALGO_ID_RGB2YUV,
	ALGO_ID_UVDEC,
	ALGO_ID_CE,
	ALGO_ID_YUVNFIIR,
	ALGO_ID_SHARPEN,
	ALGO_ID_DE,
	ALGO_ID_STATAE,
	ALGO_ID_STATYUV,
	ALGO_ID_TMAP_L,
	ALGO_ID_SCALER,

	/* PE algo */
	ALGO_ID_YUVNF,
	ALGO_ID_TNR,
	ALGO_ID_VDE,
	ALGO_ID_YUVLUT2D,
	ALGO_ID_BLIE,
	ALGO_ID_TMAP,
	ALGO_ID_ARSR,

	/* others */
	ALGO_ID_MFNR_BM,
	ALGO_ID_MFNR_MERGE,
	ALGO_ID_BM3D,
	ALGO_ID_WARP,
	ALGO_ID_DMAP,
	ALGO_ID_DIS,
	ALGO_ID_MONITOR,
	ALGO_ID_SLAM,
	ALGO_ID_REORDER,
	ALGO_ID_COMPARE,
	ALGO_ID_MAX,
} algo_id_e;

typedef enum {
	EVENT_ERR_CODE = 0,
	EVENT_SHUTTER,
	EVENT_INTERRUPT,
	EVENT_FLASH,
	EVENT_AF,
	EVENT_AF_MMI_DEBUG,
	EVENT_AF_DIRECT_TRANS_BASE,
	EVENT_AF_OTP_CALIB_DATA,
	EVENT_AF_SELF_LEARN_DATA,
	EVENT_AF_STAT_INFO,
} event_info_e;

typedef struct _msg_subreq_es_watchdog_t {
	unsigned int enable_watchdog;
	unsigned int timeout;
} msg_subreq_es_watchdog_t;

typedef struct _msg_subreq_laser_calib_t {
	unsigned int i2c_index;
	unsigned int offset;
	unsigned int crosstalk;
} msg_subreq_laser_calib_t;

typedef struct _msg_subreq_es_synclog_t {
	unsigned int reserved;
} msg_subreq_es_synclog_t;

typedef struct _msg_subreq_eg_llt_t {
} msg_subreq_eg_llt_t;

typedef struct _msg_subreq_eg_st_t{
} msg_subreq_eg_st_t;

typedef struct _msg_subreq_optical_zoom_st_t {
	unsigned int status; /* 0: none 1:optical zoom primary to secondary 2:secondary to primary */
} msg_subreq_optical_zoom_st_t;

typedef struct _msg_req_extend_set_t {
	unsigned int extend_cmd;
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	char paras[PARAS_LEN];
} msg_req_extend_set_t;

typedef struct _msg_ack_extend_set_t {
	unsigned int extend_cmd;
	int status;
	unsigned int cam_count;
	unsigned int cam_id[PIPELINE_COUNT];
	char ack_info[EXT_ACK_PARAS_LEN];
} msg_ack_extend_set_t;

/* first expo and gain ack AP to select picture */
typedef struct capture_ack_t {
	unsigned int flow; // 0 for single; 1 for dual
	unsigned int expo[2];
	unsigned int gain[2];
} capture_ack_t;

typedef struct _msg_req_extend_get_t {
	unsigned int cam_id;
	unsigned int extend_cmd;
	char *paras;
} msg_req_extend_get_t;

typedef struct _msg_ack_extend_get_t {
	unsigned int cam_id;
	unsigned int extend_cmd;
	char *paras;
	int status;
} msg_ack_extend_get_t;

typedef struct _msg_event_sent_t {
	unsigned int cam_id;
	event_info_e event_id;
	unsigned int frame_number;
	unsigned int stream_id;
	unsigned int timestampL;
	unsigned int timestampH;
	char event_params[EVENT_PARAMS_LEN];
} msg_event_sent_t;

typedef struct _msg_req_connect_camera_t {
	unsigned int cam_id;
	unsigned int csi_index;
	unsigned int i2c_index;
	struct hisp_phy_info_t phy_info;
} msg_req_connect_camera_t;

typedef struct _msg_ack_connect_camera_t {
	unsigned int cam_id;
	int status;
} msg_ack_connect_camera_t;

typedef struct _cap_dmap_yuv_info_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
} cap_dmap_yuv_info_t;

struct msg_base;

typedef void (*msg_looper_handler)(struct msg_base*, void*);

typedef struct msg_base {
	struct hi_list_head link;
	msg_looper_handler handler;
	void* user;
} msg_base_t;

typedef struct _isp_msg_t {
	unsigned int message_size;
	unsigned int api_name;
	unsigned int message_id;
	unsigned int message_hash;
	union {
		/* Request items. */
		msg_req_query_capability_t req_query_capability;
		msg_req_acquire_camera_t req_acquire_camera;
		msg_req_release_camera_t req_release_camera;
		msg_req_usecase_config_t req_usecase_config;
		msg_req_stream_on_t req_stream_on;
		msg_req_stream_off_t req_stream_off;
		msg_req_get_otp_t req_get_otp;
		msg_req_request_t req_request;
		msg_req_warp_request_t req_warp_request;
		msg_req_arsr_request_t req_arsr_request;
		msg_req_dmap_format_t req_dmap_format;
		msg_req_dmap_request_t req_dmap_request;
		msg_req_dgen_request_t req_dgen_request;
		msg_req_dopt_request_t req_dopt_request;
		msg_req_drbr_request_t req_drbr_request;
		msg_req_map_buffer_t req_map_buffer;
		msg_req_unmap_buffer_t req_unmap_buffer;
		msg_req_dynamic_map_buffer_t req_dynamic_map_buffer;
		msg_req_dynamic_unmap_buffer_t req_dynamic_unmap_buffer;
		msg_req_tnr_dynamic_map_buffer_t req_tnr_dynamic_map_buffer;
		msg_req_tnr_dynamic_unmap_buffer_t req_tnr_dynamic_unmap_buffer;
		msg_req_dmap_offline_map_t req_dmap_offline_map;
		msg_req_dmap_offline_unmap_t req_dmap_offline_unmap;
		msg_req_dmap_map_t req_dmap_map;
		msg_req_dmap_unmap_t req_dmap_unmap;
		msg_req_cal_data_t req_cal_data;
		msg_req_set_isp_regs_t req_set_isp_regs;
		msg_req_get_isp_regs_t req_get_isp_regs;
		msg_req_set_i2c_regs_t req_set_i2c_regs;
		msg_req_get_i2c_regs_t req_get_i2c_regs;
		msg_req_test_case_interface_t req_test_case_interface;
		msg_req_flush_t req_flush;
		msg_req_dgen_flush_t req_dgen_flush;
		msg_req_dopt_flush_t req_dopt_flush;
		msg_req_dmap_flush_t req_dmap_flush;
		msg_req_extend_set_t req_extend_set;
		msg_req_extend_get_t req_extend_get;
		msg_req_jpeg_encode_t req_jpeg_encode;
		msg_req_inv_tlb_t req_inv_tlb;
		msg_req_query_ois_update_t req_query_ois_update;
		msg_req_ois_update_t req_ois_update;
		msg_req_query_laser_t req_query_laser;
		msg_req_acquire_laser_t req_acquire_laser;
		msg_req_release_laser_t req_release_laser;
		msg_req_acquire_depthisp_t req_acquire_depthisp;
		msg_req_release_depthisp_t req_release_depthisp;
		msg_req_motion_sensor_map_t req_motion_sensor_map;
		msg_req_mem_pool_init_t req_mem_pool_init;
		msg_req_mem_pool_deinit_t req_mem_pool_deinit;
		msg_req_isp_cpu_poweroff_t req_isp_cpu_poweroff;

		msg_req_raw2yuv_start_t req_raw2yuv_start;
		msg_req_raw2yuv_stop_t req_raw2yuv_stop;
		msg_req_request_offline_t req_raw2yuv_req;
		msg_req_request_offline_raw2rgb_t req_raw2rgb_req;
		msg_req_request_offline_rgb2yuv_t req_rgb2yuv_req;
		msg_req_map_buffer_offline_t req_raw2yuv_mapbuffer;
		msg_req_unmap_buffer_offline_t req_raw2yuv_unmapbuffer;
		msg_req_fbdraw_request_t req_fbdraw_request;

		msg_req_query_driver_ic_t req_query_driver_ic;
		msg_req_acquire_driver_ic_t req_acquire_driver_ic;
		msg_req_release_driver_ic_t req_release_driver_ic;
		msg_req_query_dot_projector_t req_query_dot_projector;
		msg_req_acquire_dot_projector_t req_acquire_dot_projector;
		msg_req_release_dot_projector_t req_release_dot_projector;
		msg_req_get_dot_otp_t req_get_dot_otp;
		msg_req_pq_setting_config_t req_pq_setting_config;
		msg_req_release_i2c_t req_release_i2c;
		msg_batch_req_request_t req_batch_request;
		msg_req_connect_camera_t req_connect_camera;

		/* add for ispnn req */
		msg_req_create_ispnn_model_t req_create_ispnn_model;
		msg_req_extend_ispnn_buffer_t req_extend_ispnn_buffer;
		msg_req_enable_ispnn_model_t req_enable_ispnn_model;
		msg_req_disable_ispnn_model_t req_disable_ispnn_model;
		msg_req_destroy_ispnn_model_t req_destroy_ispnn_model;

		/* add for raw processing RAWMFNR and BM3D */
		msg_req_raw_proc_start_t req_rawmfnr_bm3d_start;
		msg_req_raw_proc_stop_t req_rawmfnr_bm3d_stop;
		msg_req_raw_proc_request_t req_rawmfnr_bm3d_request;
		msg_req_raw_proc_map_buffer_t req_rawmfnr_bm3d_mapbuffer;
		msg_req_raw_proc_unmap_buffer_t req_rawmfnr_bm3d_unmapbuffer;
		/* Response items. */
		msg_ack_query_capability_t ack_query_capability;
		msg_ack_acquire_camera_t ack_require_camera;
		msg_ack_release_camera_t ack_release_camera;
		msg_ack_usecase_config_t ack_usecase_config;
		msg_ack_stream_on_t ack_stream_on;
		msg_ack_stream_off_t ack_stream_off;
		msg_ack_get_otp_t ack_get_otp;
		msg_ack_request_t ack_request;
		msg_ack_warp_request_t ack_warp_request;
		msg_ack_arsr_request_t ack_arsr_request;
		msg_ack_dmap_format_t ack_dmap_format;
		msg_ack_dmap_request_t ack_dmap_request;
		msg_ack_dgen_request_t ack_dgen_request;
		msg_ack_dopt_request_t ack_dopt_request;
		msg_ack_drbr_request_t ack_drbr_request;
		msg_ack_map_buffer_t ack_map_buffer;
		msg_ack_unmap_buffer_t ack_unmap_buffer;
		msg_ack_dynamic_map_buffer_t ack_dynamic_map_buffer;
		msg_ack_dynamic_unmap_buffer_t ack_dynamic_unmap_buffer;
		msg_ack_tnr_dynamic_map_buffer_t ack_tnr_dynamic_map_buffer;
		msg_ack_tnr_dynamic_unmap_buffer_t ack_tnr_dynamic_unmap_buffer;
		msg_ack_dmap_offline_map_t ack_dmap_offline_map_buffer;
		msg_ack_dmap_offline_unmap_t ack_dmap_offline_unmap_buffer;
		msg_ack_dmap_map_t ack_dmap_map_buffer;
		msg_ack_dmap_unmap_t ack_dmap_unmap_buffer;
		msg_ack_cal_data_t ack_cal_data;
		msg_ack_set_isp_regs_t ack_set_isp_regs;
		msg_ack_get_isp_regs_t ack_get_isp_regs;
		msg_ack_set_i2c_regs_t ack_set_i2c_regs;
		msg_ack_get_i2c_regs_t ack_get_i2c_regs;
		msg_ack_test_case_interface_t ack_test_case_interface;
		msg_ack_flush_t ack_flush;
		msg_ack_dgen_flush_t ack_dgen_flush;
		msg_ack_dopt_flush_t ack_dopt_flush;
		msg_ack_dmap_flush_t ack_dmap_flush;
		msg_ack_extend_set_t ack_extend_set;
		msg_ack_extend_get_t ack_extend_get;
		msg_ack_jpeg_encode_t ack_jpeg_encode;
		msg_ack_inv_tlb_t ack_inv_tlb;
		msg_ack_query_ois_update_t ack_query_ois_update;
		msg_ack_ois_update_t ack_ois_update;
		msg_ack_query_laser_t ack_query_laser;
		msg_ack_acquire_laser_t ack_require_laser;
		msg_ack_release_laser_t ack_release_laser;
		msg_ack_acquire_depthisp_t ack_require_depthisp;
		msg_ack_release_depthisp_t ack_release_depthisp;
		msg_ack_get_api_version_t ack_get_api_version;
		msg_ack_motion_sensor_map_t ack_motion_sensor_map;
		msg_ack_mem_pool_init_t ack_mem_pool_init;
		msg_ack_mem_pool_deinit_t ack_mem_pool_deinit;
		msg_ack_isp_cpu_poweroff_t ack_isp_cpu_poweroff;

		msg_ack_raw2yuv_start_t ack_raw2yuv_start;
		msg_ack_raw2yuv_stop_t ack_raw2yuv_stop;
		msg_ack_request_offline_t ack_raw2yuv_req;
		msg_ack_request_offline_t ack_raw2rgb_req;
		msg_ack_request_offline_t ack_rgb2yuv_req;
		msg_ack_map_buffer_offline_t ack_raw2yuv_mapbuffer;
		msg_ack_unmap_buffer_offline_t ack_raw2yuv_unmapbuffer;

		msg_ack_query_driver_ic_t ack_query_driver_ic;
		msg_ack_acquire_driver_ic_t ack_acquire_driver_ic;
		msg_ack_release_driver_ic_t ack_release_driver_ic;
		msg_ack_query_dot_projector_t ack_query_dot_projector;
		msg_ack_acquire_dot_projector_t ack_acquire_dot_projector;
		msg_ack_release_dot_projector_t ack_release_dot_projector;
		msg_ack_get_dot_otp_t ack_get_dot_otp;
		msg_ack_pq_setting_config_t ack_pq_setting_config;

		/* add for ispnn ack */
		msg_ack_create_ispnn_model_t ack_create_ispnn_model;
		msg_ack_extend_ispnn_buffer_t ack_extend_ispnn_buffer;
		msg_ack_enable_ispnn_model_t ack_enable_ispnn_model;
		msg_ack_disable_ispnn_model_t ack_disable_ispnn_model;
		msg_ack_destroy_ispnn_model_t ack_destroy_ispnn_model;

		msg_ack_fbdraw_request_t ack_fbdraw_request;
		msg_ack_release_i2c_t ack_release_i2c;
		msg_batch_ack_request_t ack_batch_request;
		msg_ack_connect_camera_t ack_connect_camera;
		/* XXX: dual run don't need these parameters */
		/* rawmfnr bm3d */
		msg_ack_raw_proc_start_t ack_raw_proc_start;
		msg_ack_raw_proc_stop_t ack_raw_proc_stop;
		msg_ack_raw_proc_request_t ack_raw_proc_req;
		msg_ack_raw_proc_map_buffer_t ack_raw_proc_mapbuffer;
		msg_ack_raw_proc_unmap_buffer_t ack_raw_proc_unmapbuffer;

		/* Event items sent to AP. */
		msg_event_sent_t event_sent;
	} u;
	msg_base_t base;
	short token;
	struct rpmsg_ept *ept;
} hisp_msg_t;

enum {
	OPTICAL_SWITCH_NONE = 0, /* zoom without switch */
	OPTICAL_SWITCH_PRIMARY_TO_SECONDARY, /* optical zoom primary to secondary */
	OPTICAL_SWITCH_SECONDARY_TO_PRIMARY, /* optical zoom secondary to primary */
};

/* focus area structure */
typedef struct _tag_af_pd_area {
	int pd_area_enable;
	int pd_area_w_num;
	int pd_area_h_num;
	int pd_area_begin_x;
	int pd_area_begin_y;
	int pd_area_width;
	int pd_area_height;
} af_pd_area_t;

#define PD_WND_XNUM_MAX 16
#define PD_WND_YNUM_MAX 12
#define AF_PD_FLEXIBLE_MODE_MAX_WINDOWS 8
typedef struct _tag_af_pdaf_output {
	int pd_phase_diff[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
	unsigned int pd_conf_level[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
} af_pdaf_output_t;

/* area mode supported by sensor */
typedef enum {
	AF_PD_AP_AREA_MODE_FIXED_16_12 = 0,
	AF_PD_AP_AREA_MODE_FIXED_8_6,
	AF_PD_AP_AREA_MODE_FLEXIBLE
} af_global_assist_pd_area_mode_e;

typedef struct _tag_af_pd_config_param {
	int window_change;
	af_global_assist_pd_area_mode_e mode;
	unsigned int window_num;
	int master_cur_code;
	af_pd_area_t windows[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
	unsigned int expo_line;
	unsigned short again;
	unsigned short dgain;
} af_pd_config_param_t;

typedef struct _tag_af_sony_pdaf_output {
	int pd_phase_diff[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
	unsigned int pd_conf_level[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
} af_sony_pdaf_output_t;

typedef struct _tag_af_ov_pdaf_result {
	int phase_df; /* phase difference */
	char conf; /* 0-good, -1 not good */
	unsigned char conf_level; /* confidence level, 0-255 */
	unsigned char conf_level_improve;
	int defocus_df; /* the diff value of actuator DAC */
	int slope;
} af_ov_pdaf_result_t;

typedef struct _tag_af_ov_pdaf_output {
	af_ov_pdaf_result_t ov_pd_lib_result[AF_PD_FLEXIBLE_MODE_MAX_WINDOWS];
} af_ov_pdaf_output_t;

typedef struct _tag_af_pdaf_result {
	unsigned int pd_valid;
	unsigned int window_num;
	union {
		af_sony_pdaf_output_t sony_pd_lib_output;
		af_ov_pdaf_output_t ov_pd_lib_output;
	} ap_pdaf_result;
} af_pdaf_result_t;

void msg_init_req(hisp_msg_t *req, unsigned int api_name, unsigned int msg_id);
void msg_init_ack(hisp_msg_t *req, hisp_msg_t *ack);
#endif /* HISP_MSG_H_INCLUDED */
