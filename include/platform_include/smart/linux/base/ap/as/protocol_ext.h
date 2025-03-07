/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: protocol header file
 * Create: 2021/12/05
 */
#ifndef __PROTOCOL_EXT_H__
#define __PROTOCOL_EXT_H__

#include "protocol_base.h"

#define SUBCMD_LEN 4
#define MAX_PKT_LENGTH                       128
#define MAX_PKT_LENGTH_AP                    2560
#define MAX_LOG_LEN                          100
#define MAX_PATTERN_SIZE                     16
#define MAX_ACCEL_PARAMET_LENGTH             100
#define MAX_MAG_PARAMET_LENGTH               100
#define MAX_GYRO_PARAMET_LENGTH              100
#define MAX_ALS_PARAMET_LENGTH               100
#define MAX_PS_PARAMET_LENGTH                100
#define MAX_I2C_DATA_LENGTH                  50
#define MAX_SENSOR_CALIBRATE_DATA_LENGTH     60
#define MAX_VIB_CALIBRATE_DATA_LENGTH        3
#define MAX_MAG_CALIBRATE_DATA_LENGTH        12
#define MAX_GYRO_CALIBRATE_DATA_LENGTH       72
#define MAX_GYRO_TEMP_OFFSET_LENGTH          56
#define MAX_CAP_PROX_CALIBRATE_DATA_LENGTH   16
#define MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH 24
#define MAX_MAG_AKM_CALIBRATE_DATA_LENGTH    28
#define MAX_TOF_CALIBRATE_DATA_LENGTH        47
#define MAX_PS_CALIBRATE_DATA_LENGTH         24
#define MAX_ALS_CALIBRATE_DATA_LENGTH        24

/* data flag consts */
#define DATA_FLAG_FLUSH_OFFSET           0
#define DATA_FLAG_VALID_TIMESTAMP_OFFSET 1
#define FLUSH_END                        (1 << DATA_FLAG_FLUSH_OFFSET)
#define DATA_FLAG_VALID_TIMESTAMP        (1 << DATA_FLAG_VALID_TIMESTAMP_OFFSET)
#define ACC_CALIBRATE_DATA_LENGTH        15
#define PS_CALIBRATE_DATA_LENGTH         6
#define ALS_CALIBRATE_DATA_LENGTH        6
#define ACC1_CALIBRATE_DATA_LENGTH       60
#define ACC1_OFFSET_DATA_LENGTH          15
#define GYRO1_CALIBRATE_DATA_LENGTH      18
#define MAX_GYRO1_CALIBRATE_DATA_LENGTH  72
#define MAX_THP_SYNC_INFO_LEN            (2 * 1024)

typedef enum {
	FINGERPRINT_TYPE_START = 0x0,
	FINGERPRINT_TYPE_HUB,
	FINGERPRINT_TYPE_END,
} fingerprint_type_t;

typedef enum {
	CA_TYPE_START,
	CA_TYPE_PICKUP,
	CA_TYPE_PUTDOWN,
	CA_TYPE_ACTIVITY,
	CA_TYPE_HOLDING,
	CA_TYPE_MOTION,
	CA_TYPE_PLACEMENT,
	/* !!!NOTE:add string in ca_type_str when add type */
	CA_TYPE_END,
} ca_type_t;

typedef enum {
	TYPE_STANDARD,
	TYPE_EXTEND
} type_step_counter_t;

typedef enum {
	AUTO_MODE = 0,
	FIFO_MODE,
	INTEGRATE_MODE,
	REALTIME_MODE,
	MODE_END
} obj_report_mode_t;

typedef enum {
	/* system status */
	ST_NULL = 0,
	ST_BEGIN,
	ST_POWERON = ST_BEGIN,
	ST_MINSYSREADY,
	ST_DYNLOAD,
	ST_MCUREADY,
	ST_TIMEOUTSCREENOFF,
	ST_SCREENON, /* 6 */
	ST_SCREENOFF, /* 7 */
	ST_SLEEP, /* 8 */
	ST_WAKEUP, /* 9 */
	ST_POWEROFF,
	ST_RECOVERY_BEGIN, /* for ar notify modem when iom3 recovery */
	ST_RECOVERY_FINISH, /* for ar notify modem when iom3 recovery */
	ST_END
} sys_status_t;

typedef struct {
	struct pkt_header hd;
	unsigned char wr;
	unsigned int fault_addr;
} __packed pkt_fault_addr_req_t;

struct sensor_data_xyz {
	signed int x;
	signed int y;
	signed int z;
	unsigned int accracy;
};

typedef struct {
	struct pkt_header hd;
	unsigned short data_flag;
	unsigned short cnt;
	unsigned short len_element;
	unsigned short sample_rate;
	unsigned long long timestamp;
}  pkt_common_data_t;

typedef struct {
	pkt_common_data_t data_hd;
	struct sensor_data_xyz xyz[]; /* x,y,z,acc,time */
} pkt_batch_data_req_t;

typedef struct {
	struct pkt_header hd;
	signed int type;
	signed short serial;
	/* 0: more additional info to be send  1:this pkt is last one */
	signed short end;
	/*
	 * for each frame, a single data type,
	 * either signed int or float, should be used
	 */
	union {
		signed int data_int32[14];
	};
} pkt_additional_info_req_t;

typedef struct interval_param {
	/* each group data of batch_count upload once, in & out */
	unsigned int period;
	/* input: expected value, out: the closest value actually supported */
	unsigned int batch_count;
	/*
	 * 0: auto mode, mcu reported according to the business characteristics
	 *    and system status
	 * 1: FIFO mode, maybe with multiple records
	 * 2: Integerate mode, update or accumulate the latest data, but do not
	 *    increase the record, and report it
	 * 3: real-time mode, real-time report no matter which status ap is
	 */
	unsigned char mode;
	/* reserved[0]: used by motion and pedometer now */
	unsigned char reserved[3];
} __packed interval_param_t;

typedef struct {
	struct pkt_header hd;
	interval_param_t param;
} __packed pkt_cmn_interval_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned int subcmd;
	unsigned char para[128];
} __packed pkt_parameter_req_t;

union spi_ctrl {
	unsigned int data;
	struct {
		/* bit0~8 is gpio NO., bit9~11 is gpio iomg set */
		unsigned int gpio_cs : 16;
		/* unit: MHz; 0 means default 5MHz */
		unsigned int baudrate : 5;
		/* low-bit: clk phase , high-bit: clk polarity convert, normally select:0 */
		unsigned int mode : 2;
		/* 0 means default: 8 */
		unsigned int bits_per_word : 5;
		unsigned int rsv_28_31 : 4;
	} b;
};

typedef struct {
	struct pkt_header hd;
	unsigned char busid;
	union {
		unsigned int i2c_address;
		union spi_ctrl ctrl;
	};
	unsigned short rx_len;
	unsigned short tx_len;
	unsigned char tx[];
} pkt_combo_bus_trans_req_t;

typedef struct {
	struct pkt_header_resp hd;
	unsigned char data[];
} pkt_combo_bus_trans_resp_t;

typedef struct {
	struct pkt_header hd;
	unsigned short status;
	unsigned short version;
} pkt_sys_statuschange_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned int idle_time;
	unsigned int reserved;
	unsigned long long current_app_mask;
} pkt_power_log_report_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned int event_id;
} pkt_dft_report_t;

typedef struct {
	struct pkt_header hd;
	/* 1:aux sensorlist 0:filelist 2: loading */
	unsigned char file_flg;
	/*
	 * num must less than
	 * (MAX_PKT_LENGTH-sizeof(PKT_HEADER)-sizeof(End))/sizeof(UINT16)
	 */
	unsigned char file_count; /* num of file or aux sensor */
	unsigned short file_list[]; /* fileid or aux sensor tag */
} pkt_sys_dynload_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned char level;
	unsigned char dmd_case;
	unsigned char resv1;
	unsigned char resv2;
	unsigned int dmd_id;
	unsigned int info[5];
} pkt_dmd_log_report_req_t;

typedef struct {
	pkt_common_data_t fhd;
	signed int data;
} fingerprint_upload_pkt_t;

typedef struct {
	unsigned int sub_cmd;
	unsigned char buf[7]; /* byte alignment */
	unsigned char len;
} fingerprint_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned long long app_id;
	unsigned short msg_type;
	unsigned char res[6];
	unsigned char data[];
} chre_req_t;

typedef enum additional_info_type {
	/* Marks the beginning of additional information frames */
	AINFO_BEGIN = 0x0,
	/* Marks the end of additional information frames */
	AINFO_END   = 0x1,
	/* Basic information */
	/*
	 * Estimation of the delay that is not tracked by sensor
	 * timestamps. This includes delay introduced by
	 * sensor front-end filtering, data transport, etc.
	 * float[2]: delay in seconds
	 * standard deviation of estimated value
	 */
	AINFO_UNTRACKED_DELAY =  0x10000,
	AINFO_INTERNAL_TEMPERATURE, /* float: Celsius temperature */
	/*
	 * First three rows of a homogeneous matrix, which
	 * represents calibration to a three-element vector
	 * raw sensor reading.
	 * float[12]: 3x4 matrix in row major order
	 */
	AINFO_VEC3_CALIBRATION,
	/*
	 * Location and orientation of sensor element in the
	 * device frame: origin is the geometric center of the
	 * mobile device screen surface; the axis definition
	 * corresponds to Android sensor definitions.
	 * float[12]: 3x4 matrix in row major order
	 */
	AINFO_SENSOR_PLACEMENT,
	/*
	 * float[2]: raw sample period in seconds,
	 * standard deviation of sampling period
	 */
	AINFO_SAMPLING,
	/* Sampling channel modeling information */
	/*
	 * signed int: noise type
	 * float[n]: parameters
	 */
	AINFO_CHANNEL_NOISE = 0x20000,
	/*
	 * float[3]: sample period standard deviation of sample period,
	 * quantization unit
	 */
	AINFO_CHANNEL_SAMPLER,
	/*
	 * Represents a filter:
	 * \sum_j a_j y[n-j] == \sum_i b_i x[n-i]
	 * signed int[3]: number of feedforward coefficients, M,
	 * number of feedback coefficients, N, for FIR filter, N=1.
	 * bit mask that represents which element to which the filter is applied
	 * bit 0 == 1 means this filter applies to vector element 0.
	 * float[M+N]: filter coefficients (b0, b1, ..., BM-1),
	 * then (a0, a1, ..., aN-1), a0 is always 1.
	 * Multiple frames may be needed for higher number of taps.
	 */
	AINFO_CHANNEL_FILTER,
	/*
	 * signed int[2]: size in (row, column) ... 1st frame
	 * float[n]: matrix element values in row major order.
	 */
	AINFO_CHANNEL_LINEAR_TRANSFORM,
	/*
	 * signed int[2]: extrapolate method interpolate method
	 * float[n]: mapping key points in pairs, (in, out)...
	 * (may be used to model saturation)
	 */
	AINFO_CHANNEL_NONLINEAR_MAP,
	/*
	 * signed int: resample method (0-th order, 1st order...)
	 * float[1]: resample ratio (upsampling if < 1.0;
	 * downsampling if > 1.0).
	 */
	AINFO_CHANNEL_RESAMPLER,
	/* Custom information */
	AINFO_CUSTOM_START =    0x10000000,
	/* Debugging */
	AINFO_DEBUGGING_START = 0x40000000,
} additional_info_type_t;


typedef enum {
	DFT_EVENT_CPU_USE = 1,
	DFT_EVENT_TASK_IFO,
	DFT_EVENT_MEM_USE,
	DFT_EVENT_FREQ_TIME,
	DFT_EVENT_SENSOR,
	DFT_EVENT_APP,
	DFT_EVENT_RELATION,
	DFT_EVENT_MAX,
} dft_event_id_t;


#define IOM3_ST_NORMAL      0
#define IOM3_ST_RECOVERY    1
#define IOM3_ST_REPEAT      2

#define MAX_STR_SIZE 1024

#define RET_SUCC  0
#define RET_FAIL  -1
#define STARTUP_IOM3_CMD 0x00070001
#define RELOAD_IOM3_CMD 0x0007030D
#define IPC_SHM_MAGIC 0x1a2b3c4d
#define IPC_SHM_BUSY 0x67
#define IPC_SHM_FREE 0xab
#define MID_PKT_LEN (128 - sizeof(struct pkt_header))

struct ipc_shm_ctrl_hdr {
	signed int module_id;
	unsigned int buf_size;
	unsigned int offset;
	signed int msg_type;
	signed int checksum;
	unsigned int priv;
};

struct shmem_ipc_ctrl_package {
	struct pkt_header hd;
	struct ipc_shm_ctrl_hdr sh_hdr;
};

struct ipcshm_data_hdr {
	unsigned int magic_word;
	unsigned char data_free;
	unsigned char reserved[3]; /* reserved */
	struct pkt_header pkt;
};

typedef struct aod_display_pos {
	unsigned short x_start;
	unsigned short y_start;
} aod_display_pos_t;

typedef struct aod_start_config {
	aod_display_pos_t aod_pos;
	signed int intelli_switching;
} aod_start_config_t;

typedef struct aod_time_config {
	unsigned long long curr_time;
	signed int time_zone;
	signed int sec_time_zone;
	signed int time_format;
} aod_time_config_t;

typedef struct aod_display_space {
	unsigned short x_start;
	unsigned short y_start;
	unsigned short x_size;
	unsigned short y_size;
} aod_display_space_t;

typedef struct aod_display_spaces {
	signed int dual_clocks;
	signed int display_type;
	signed int display_space_count;
	aod_display_space_t display_spaces[5];
} aod_display_spaces_t;

typedef struct aod_screen_info {
	unsigned short xres;
	unsigned short yres;
	unsigned short pixel_format;
} aod_screen_info_t;

typedef struct aod_bitmap_size {
	unsigned short xres;
	unsigned short yres;
} aod_bitmap_size_t;

typedef struct aod_bitmaps_size {
	signed int bitmap_type_count; /* 2, dual clock */
	aod_bitmap_size_t bitmap_size[2];
} aod_bitmaps_size_t;

typedef struct aod_config_info {
	unsigned int aod_fb;
	unsigned int aod_digits_addr;
	aod_screen_info_t screen_info;
	aod_bitmaps_size_t bitmap_size;
} aod_config_info_t;

typedef struct {
	struct pkt_header hd;
	unsigned int sub_cmd;
	union {
		aod_start_config_t start_param;
		aod_time_config_t time_param;
		aod_display_spaces_t display_param;
		aod_config_info_t config_param;
		aod_display_pos_t display_pos;
	};
} aod_req_t;

typedef struct {
	struct pkt_header hd;
	unsigned short data_flag;
	unsigned int step_count;
	unsigned int begin_time;
	unsigned short record_count;
	unsigned short capacity;
	unsigned int total_step_count;
	unsigned int total_floor_ascend;
	unsigned int total_calorie;
	unsigned int total_distance;
	unsigned short step_pace;
	unsigned short step_length;
	unsigned short speed;
	unsigned short touchdown_ratio;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned short action_record[];
} pkt_step_counter_data_req_t;

#endif
