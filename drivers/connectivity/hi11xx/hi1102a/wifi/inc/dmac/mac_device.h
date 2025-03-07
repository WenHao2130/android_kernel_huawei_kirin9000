
#ifndef __MAC_DEVICE_H__
#define __MAC_DEVICE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "hal_ext_if.h"
#include "mac_vap.h"
#include "hal_commom_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_NET_DEVICE_NAME_LENGTH 16

#define MAC_DATARATES_PHY_80211G_NUM 12

#define DMAC_BA_LUT_IDX_BMAP_LEN ((HAL_MAX_BA_LUT_SIZE + 7) >> 3)
#define DMAC_TX_BA_LUT_BMAP_LEN  ((HAL_MAX_AMPDU_LUT_SIZE + 7) >> 3)

/* 异常超时上报时间 */
#define MAC_EXCEPTION_TIME_OUT 10000

/* DMAC SCANNER 扫描模式 */
#define MAC_SCAN_FUNC_MEAS       0x1
#define MAC_SCAN_FUNC_STATS      0x2
#define MAC_SCAN_FUNC_RADAR      0x4
#define MAC_SCAN_FUNC_BSS        0x8
#define MAC_SCAN_FUNC_P2P_LISTEN 0x10
#define MAC_SCAN_FUNC_CHBA       0x20
#define MAC_SCAN_FUNC_ALL        (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS | MAC_SCAN_FUNC_RADAR | MAC_SCAN_FUNC_BSS)

#define MAC_ERR_LOG(_uc_vap_id, _puc_string)
#define MAC_ERR_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_ERR_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_ERR_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_ERR_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_ERR_VAR(_uc_vap_id, _c_fmt, ...)

#define MAC_WARNING_LOG(_uc_vap_id, _puc_string)
#define MAC_WARNING_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_WARNING_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_WARNING_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_WARNING_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_WARNING_VAR(_uc_vap_id, _c_fmt, ...)

#define MAC_INFO_LOG(_uc_vap_id, _puc_string)
#define MAC_INFO_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define MAC_INFO_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define MAC_INFO_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define MAC_INFO_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAC_INFO_VAR(_uc_vap_id, _c_fmt, ...)
#define MAC_EVENT_STATE_CHANGE(_puc_macaddr, _uc_vap_id, en_event_type, output_data, data_len)

/* 获取设备的算法私有结构体 */
#define MAC_DEV_ALG_PRIV(_pst_dev) ((_pst_dev)->p_alg_priv)

/* 复位状态 */
#define MAC_DEV_RESET_IN_PROGRESS(_pst_device, uc_value) ((_pst_device)->uc_device_reset_in_progress = uc_value)
#define MAC_DEV_IS_RESET_IN_PROGRESS(_pst_device)        ((_pst_device)->uc_device_reset_in_progress)

#define MAC_DFS_RADAR_WAIT_TIME_MS 60000

#define MAC_DEV_MAX_40M_INTOL_USER_BITMAP_LEN 4
#define MAC_SCAN_CHANNEL_INTERVAL_DEFAULT         6   /* 间隔6个信道，切回工作信道工作一段时间 */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT     100 /* 背景扫描时，返回工作信道工作的时间 */
#define MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE     2   /* 间隔2个信道，切回工作信道工作一段时间 */
#define MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE 60  /* WLAN未关联，P2P关联，返回工作信道工作的时间 */

#define MAC_FCS_DBAC_IGNORE     0 /* 不是DBAC场景 */
#define MAC_FCS_DBAC_NEED_CLOSE 1 /* DBAC需要关闭 */
#define MAC_FCS_DBAC_NEED_OPEN  2 /* DBAC需要开启 */

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
#define MAC_MAX_IP_FILTER_BTABLE_SIZE 512 /* rx ip数据包过滤功能的黑名单大小 */
#endif                                    // _PRE_WLAN_FEATURE_IP_FILTER
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* SDT操作模式枚举 */
typedef enum {
    MAC_SDT_MODE_WRITE = 0,
    MAC_SDT_MODE_READ,

    MAC_SDT_MODE_BUTT
} mac_sdt_rw_mode_enum;
typedef oal_uint8 mac_sdt_rw_mode_enum_uint8;

typedef enum {
    MAC_CH_TYPE_NONE = 0,
    MAC_CH_TYPE_PRIMARY = 1,
    MAC_CH_TYPE_SECONDARY = 2,
} mac_ch_type_enum;
typedef oal_uint8 mac_ch_type_enum_uint8;

typedef enum {
    MAC_SCAN_OP_INIT_SCAN,
    MAC_SCAN_OP_FG_SCAN_ONLY,
    MAC_SCAN_OP_BG_SCAN_ONLY,

    MAC_SCAN_OP_BUTT
} mac_scan_op_enum;
typedef oal_uint8 mac_scan_op_enum_uint8;

typedef enum {
    MAC_CHAN_NOT_SUPPORT = 0,      /* 管制域不支持该信道 */
    MAC_CHAN_AVAILABLE_ALWAYS,     /* 信道一直可以使用 */
    MAC_CHAN_AVAILABLE_TO_OPERATE, /* 经过检测(CAC, etc...)后，该信道可以使用 */
    MAC_CHAN_DFS_REQUIRED,         /* 该信道需要进行雷达检测 */
    MAC_CHAN_BLOCK_DUE_TO_RADAR,   /* 由于检测到雷达导致该信道变的不可用 */

    MAC_CHAN_STATUS_BUTT
} mac_chan_status_enum;
typedef oal_uint8 mac_chan_status_enum_uint8;

#ifdef _PRE_WLAN_DFT_STAT
typedef enum {
    MAC_DEV_MGMT_STAT_TYPE_TX = 0,
    MAC_DEV_MGMT_STAT_TYPE_RX,
    MAC_DEV_MGMT_STAT_TYPE_TX_COMPLETE,

    MAC_DEV_MGMT_STAT_TYPE_BUTT
} mac_dev_mgmt_stat_type_enum;
typedef oal_uint8 mac_dev_mgmt_stat_type_enum_uint8;
#endif

/* device reset同步子类型枚举 */
typedef enum {
    MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_SWITCH_GET_TYPE,
    MAC_RESET_STATUS_GET_TYPE,
    MAC_RESET_STATUS_SET_TYPE,
    MAC_RESET_SWITCH_SYS_TYPE = MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_STATUS_SYS_TYPE = MAC_RESET_STATUS_SET_TYPE,

    MAC_RESET_SYS_TYPE_BUTT
} mac_reset_sys_type_enum;
typedef oal_uint8 mac_reset_sys_type_enum_uint8;

typedef enum {
    MAC_TRY_INIT_SCAN_VAP_UP,
    MAC_TRY_INIT_SCAN_SET_CHANNEL,
    MAC_TRY_INIT_SCAN_START_DBAC,
    MAC_TRY_INIT_SCAN_RESCAN,

    MAC_TRY_INIT_SCAN_BUTT
} mac_try_init_scan_type;
typedef oal_uint8 mac_try_init_scan_type_enum_uint8;

typedef enum {
    MAC_INIT_SCAN_NOT_NEED,
    MAC_INIT_SCAN_NEED,
    MAC_INIT_SCAN_IN_SCAN,
} mac_need_init_scan_res;
typedef oal_uint8 mac_need_init_scan_res_enum_uint8;

typedef enum {
    MAC_ONE_PACKET_INDEX_1,
    MAC_ONE_PACKET_INDEX_2,
} mac_one_packet_index;
typedef oal_uint8 mac_one_packet_index_enum_uint8;

#define MAC_FCS_MAX_CHL_NUM               2
#define MAC_FCS_TIMEOUT_JIFFY             2
#define MAC_FCS_DEFAULT_PROTECT_TIME_OUT  5120  /* us */
#define MAC_FCS_DEFAULT_PROTECT_TIME_OUT2 1024  /* us */
#define MAC_FCS_DEFAULT_PROTECT_TIME_OUT3 15000 /* us */
#define MAC_FCS_DEFAULT_PROTECT_TIME_OUT4 16000 /* us */
#define MAC_FCS_DEFAULT_PROTECT_TIME_OUT5 9000  /* us 单VAP场景下增加扫描发送null帧的超时时间提升null帧的发送成功率 */

#define MAC_ONE_PACKET_TIME_OUT  1000
#define MAC_ONE_PACKET_TIME_OUT3 2000
#define MAC_ONE_PACKET_TIME_OUT4 2000

#define MAC_FCS_CTS_MAX_DURATION 32767 /* us */

#define MAC_FCS_CTS_MAX_BTCOEX_NOR_DURATION  30000 /* us */
#define MAC_FCS_CTS_MAX_BTCOEX_LDAC_DURATION 65535 /* us */

/*
 self CTS
+-------+-----------+----------------+
|frame  | duration  |      RA        |     len=10
|control|           |                |
+-------+-----------+----------------+

null data
+-------+-----------+---+---+---+--------+
|frame  | duration  |A1 |A2 |A3 |Seq Ctl | len=24
|control|           |   |   |   |        |
+-------+-----------+---+---+---+--------+
*/
typedef enum {
    MAC_FCS_NOTIFY_TYPE_SWITCH_AWAY = 0,
    MAC_FCS_NOTIFY_TYPE_SWITCH_BACK,

    MAC_FCS_NOTIFY_TYPE_BUTT
} mac_fcs_notify_type_enum;
typedef oal_uint8 mac_fcs_notify_type_enum_uint8;

typedef struct {
    mac_channel_stru st_dst_chl;
    mac_channel_stru st_src_chl;
    hal_one_packet_cfg_stru st_one_packet_cfg;
    oal_uint8 uc_src_fake_q_id; /* 原信道对应虚假队列id 取值范围 0 1 */
    oal_uint8 uc_dst_fake_q_id; /* 目的信道对应虚假队列id 取值范围 0 1 */
    oal_uint16 us_hot_cnt;

    mac_channel_stru st_src_chl2;
    hal_one_packet_cfg_stru st_one_packet_cfg2;
} mac_fcs_cfg_stru;

typedef enum {
    MAC_FCS_HOOK_ID_DBAC,
    MAC_FCS_HOOK_ID_ACS,

    MAC_FCS_HOOK_ID_BUTT
} mac_fcs_hook_id_enum;
typedef oal_uint8 mac_fcs_hook_id_enum_uint8;

typedef struct {
    mac_fcs_notify_type_enum_uint8 uc_notify_type;
    oal_uint8 uc_chip_id;
    oal_uint8 uc_device_id;
    oal_uint8 uc_resv[1];
    mac_fcs_cfg_stru st_fcs_cfg;
} mac_fcs_event_stru;

typedef void (*mac_fcs_notify_func)(const mac_fcs_event_stru *);

typedef struct {
    mac_fcs_notify_func p_func;
} mac_fcs_notify_node_stru;

typedef struct {
    mac_fcs_notify_node_stru ast_notify_nodes[MAC_FCS_HOOK_ID_BUTT];
} mac_fcs_notify_chain_stru;

typedef enum {
    MAC_FCS_STATE_STANDBY = 0,  // free to use
    MAC_FCS_STATE_REQUESTED,    // requested by other module, but not in switching
    MAC_FCS_STATE_IN_PROGESS,   // in switching

    MAC_FCS_STATE_BUTT
} mac_fcs_state_enum;
typedef oal_uint8 mac_fcs_state_enum_uint8;

typedef enum {
    MAC_FCS_SUCCESS = 0,
    MAC_FCS_ERR_NULL_PTR,
    MAC_FCS_ERR_INVALID_CFG,
    MAC_FCS_ERR_BUSY,
    MAC_FCS_ERR_UNKNOWN_ERR,
} mac_fcs_err_enum;
typedef oal_uint8 mac_fcs_err_enum_uint8;

typedef struct {
    oal_uint32 ul_offset_addr;
    oal_uint32 ul_value[MAC_FCS_MAX_CHL_NUM];
} mac_fcs_reg_record_stru;

typedef struct tag_mac_fcs_mgr_stru {
    volatile oal_bool_enum_uint8 en_fcs_done;
    oal_uint8 uc_chip_id;
    oal_uint8 uc_device_id;
    oal_uint8 uc_fcs_cnt;
    oal_spin_lock_stru st_lock;
    mac_fcs_state_enum_uint8 en_fcs_state;
    hal_fcs_service_type_enum_uint8 en_fcs_service_type;
    oal_uint8 uc_resv[2];
    mac_fcs_cfg_stru *pst_fcs_cfg;
    mac_fcs_notify_chain_stru ast_notify_chain[MAC_FCS_NOTIFY_TYPE_BUTT];
} mac_fcs_mgr_stru;

#define MAC_FCS_VERIFY_MAX_ITEMS 1
typedef enum {
    // isr
    MAC_FCS_STAGE_INTR_START,
    MAC_FCS_STAGE_INTR_POST_EVENT,
    MAC_FCS_STAGE_INTR_DONE,

    // event
    MAC_FCS_STAGE_EVENT_START,
    MAC_FCS_STAGE_PAUSE_VAP,
    MAC_FCS_STAGE_ONE_PKT_START,
    MAC_FCS_STAGE_ONE_PKT_INTR,
    MAC_FCS_STAGE_ONE_PKT_DONE,
    MAC_FCS_STAGE_RESET_HW_START,
    MAC_FCS_STAGE_RESET_HW_DONE,
    MAC_FCS_STAGE_RESUME_VAP,
    MAC_FCS_STAGE_EVENT_DONE,

    MAC_FCS_STAGE_COUNT
} mac_fcs_stage_enum;
typedef mac_fcs_stage_enum mac_fcs_stage_enum_uint8;

typedef struct {
    oal_bool_enum_uint8 en_enable;
    oal_uint8 auc_resv[3];
    oal_uint32 ul_item_cnt;
    oal_uint32 aul_timestamp[MAC_FCS_VERIFY_MAX_ITEMS][MAC_FCS_STAGE_COUNT];
} mac_fcs_verify_stat_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/* 上报关键信息的flags标记信息，对应标记位为1，则对上报对应信息 */
typedef enum {
    MAC_REPORT_INFO_FLAGS_HARDWARE_INFO = BIT(0),
    MAC_REPORT_INFO_FLAGS_QUEUE_INFO = BIT(1),
    MAC_REPORT_INFO_FLAGS_MEMORY_INFO = BIT(2),
    MAC_REPORT_INFO_FLAGS_EVENT_INFO = BIT(3),
    MAC_REPORT_INFO_FLAGS_VAP_INFO = BIT(4),
    MAC_REPORT_INFO_FLAGS_USER_INFO = BIT(5),
    MAC_REPORT_INFO_FLAGS_TXRX_PACKET_STATISTICS = BIT(6),
    MAC_REPORT_INFO_FLAGS_BUTT = BIT(7),
} mac_report_info_flags;
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
/* rx方向指定数据包过滤配置命令 */
typedef enum {
    MAC_ICMP_FILTER = 1, /* ICMP数据包过滤 */

    MAC_FILTER_ID_BUTT
} mac_assigned_filter_id_enum;
typedef oal_uint8 mac_assigned_filter_id_enum_uint8;

/* 指定filter配置命令格式 */
typedef struct {
    mac_assigned_filter_id_enum_uint8 uc_filter_id;
    oal_bool_enum_uint8 en_enable;  /* 下发过滤功能使能开关命令 */
    oal_uint16 us_rx_icmp_pkgs_num; /* 扩展用于统计过滤的icmp报文数量 */
} mac_assigned_filter_cmd_stru;

/* rx ip数据包过滤的配置命令 */
typedef enum {
    MAC_IP_FILTER_ENABLE = 0,        /* 开/关ip数据包过滤功能 */
    MAC_IP_FILTER_UPDATE_BTABLE = 1, /* 更新黑名单 */
    MAC_IP_FILTER_CLEAR = 2,         /* 清除黑名单 */

    MAC_IP_FILTER_BUTT
} mac_ip_filter_cmd_enum;
typedef oal_uint8 mac_ip_filter_cmd_enum_uint8;

/* 黑名单条目格式 */
typedef struct {
    oal_uint16 us_port; /* 目的端口号，以主机字节序格式存储 */
    oal_uint8 uc_protocol;
    oal_uint8 uc_resv;
    // oal_uint32                  ul_filter_cnt; /* 目前未接受"统计过滤包数量"的需求，此成员暂不使用 */
} mac_ip_filter_item_stru;

/* 配置命令格式 */
typedef struct {
    oal_uint8 uc_item_count;
    oal_bool_enum_uint8 en_enable; /* 下发功能使能标志 */
    mac_ip_filter_cmd_enum_uint8 en_cmd;
    oal_uint8 uc_resv;
    mac_ip_filter_item_stru ast_filter_items_items[1];
} mac_ip_filter_cmd_stru;

#endif  // _PRE_WLAN_FEATURE_IP_FILTER

#ifdef _PRE_WLAN_FEATURE_APF
#define APF_PROGRAM_MAX_LEN 512
#define APF_VERSION         2
typedef enum {
    APF_SET_FILTER_CMD,
    APF_GET_FILTER_CMD
} mac_apf_cmd_type_enum;
typedef oal_uint8 mac_apf_cmd_type_uint8;

typedef struct {
    oal_bool_enum_uint8 en_is_enabled;
    oal_uint8 auc_res[1];
    oal_uint16 us_program_len;
    oal_uint32 ul_install_timestamp;
    oal_uint32 ul_flt_pkt_cnt;
    oal_uint8 auc_program[APF_PROGRAM_MAX_LEN];
} mac_apf_stru;

typedef struct {
    mac_apf_cmd_type_uint8 en_cmd_type;
    oal_uint8 auc_res[1];
    oal_uint16 us_program_len;
    oal_uint8 *puc_program;
} mac_apf_filter_cmd_stru;

typedef struct {
    oal_void *p_program;
} mac_apf_report_event_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum {
    NO_LOCATION = 0,
    ROOT_AP = 1,
    REPEATER = 2,
    STA = 3,
    LOCATION_TYPE_BUTT
} oal_location_type_enum;
typedef oal_uint8 oal_location_type_enum_uint8;
#endif

typedef enum {
    MAC_CSA_FLAG_NORMAL = 0,
    MAC_CSA_FLAG_START_DEBUG, /* 固定csa ie 在beacon帧中 */
    MAC_CSA_FLAG_CANCLE_DEBUG,

    MAC_CSA_FLAG_BUTT
} mac_csa_flag_enum;
typedef oal_uint8 mac_csa_flag_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef enum {
    MAC_BTCOEX_CFG_ONEPKT_TYPE,  // 配置共存场景下one pkt帧的类型
    MAC_BTCOEX_CFG_RSP_CTS,      // 配置共存场景下是否回复CTS
    MAC_BTCOEX_CFG_BA_SIZE,      // 配置对端发送聚合个数
    MAC_BTCOEX_CFG_BUTT
} mac_btcoex_cfg_type_enum;
typedef oal_uint8 mac_btcoex_cfg_type_enum_uint8;
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct {
    mac_btcoex_cfg_type_enum_uint8 en_cfg_type;
    oal_uint8 uc_cfg_value;
    oal_uint8 auc_res[2];
} mac_btcoex_cfg_stru;
#endif
/* device reset事件同步结构体 */
typedef struct {
    mac_reset_sys_type_enum_uint8 en_reset_sys_type; /* 复位同步类型 */
    oal_uint8 uc_value;                              /* 同步信息值 */
    oal_uint8 uc_resv[2];
} mac_reset_sys_stru;

typedef void (*mac_scan_cb_fn)(void *p_scan_record);

typedef struct {
    oal_uint16 us_num_networks;
    mac_ch_type_enum_uint8 en_ch_type;
    oal_uint8 auc_resv[1];
} mac_ap_ch_info_stru;

typedef struct {
    oal_uint16 us_num_networks; /* 记录当前信道下扫描到的BSS个数 */
    oal_uint8 auc_resv[2];
    oal_uint8 auc_bssid_array[WLAN_MAX_SCAN_BSS_PER_CH][WLAN_MAC_ADDR_LEN]; /* 记录当前信道下扫描到的所有BSSID */
} mac_bss_id_list_stru;

#define MAX_PNO_SSID_COUNT      16
#define MAX_PNO_REPEAT_TIMES    4
#define PNO_SCHED_SCAN_INTERVAL (60 * 1000)

/* PNO扫描信息结构体 */
typedef struct {
    oal_uint8 auc_ssid[WLAN_SSID_MAX_LEN];
    oal_bool_enum_uint8 en_scan_ssid;
    oal_uint8 auc_resv[2];
} pno_match_ssid_stru;

typedef struct {
    pno_match_ssid_stru ast_match_ssid_set[MAX_PNO_SSID_COUNT];
    union {
        oal_int32 l_ssid_count;     /* 下发的需要匹配的ssid集的个数 */
        oal_uint32 ul_work_time_ms; /* 窄带固件下因为ROM化,复用为work_time */
    };
    union {
        oal_int32 l_rssi_thold;           /* 可上报的rssi门限 */
        oal_uint32 ul_listen_interval_ms; /* 窄带固件下因为ROM化,复用为周期 */
    };

    oal_uint32 ul_pno_scan_interval;                /* pno扫描间隔 */
    oal_uint8 auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    oal_uint8 uc_pno_scan_repeat;                   /* pno扫描重复次数 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否随机mac */

    mac_scan_cb_fn p_fn_cb; /* 函数指针必须放最后否则核间通信出问题 */
} mac_pno_scan_stru;

/* PNO调度扫描管理结构体 */
typedef struct {
    mac_pno_scan_stru st_pno_sched_scan_params; /* pno调度扫描请求的参数 */
    // frw_timeout_stru        st_pno_sched_scan_timer;              /* pno调度扫描定时器 */
    oal_void *p_pno_sched_scan_timer;           /* pno调度扫描rtc时钟定时器，此定时器超时后，能够唤醒睡眠的device */
    oal_uint8 uc_curr_pno_sched_scan_times;     /* 当前pno调度扫描次数 */
    oal_bool_enum_uint8 en_is_found_match_ssid; /* 是否扫描到了匹配的ssid */
    oal_uint8 auc_resv[2];
} mac_pno_sched_scan_mgmt_stru;

typedef struct {
    oal_uint8 auc_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8 auc_resv[3];
} mac_ssid_stru;

/* 扫描参数结构体 */
typedef struct {
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type; /* 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    oal_uint8 uc_bssid_num;                          /* 期望扫描的bssid个数 */
    oal_uint8 uc_ssid_num;                           /* 期望扫描的ssid个数 */

    oal_uint8 auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    oal_uint8 uc_p2p0_listen_channel;
    oal_bool_enum_uint8 en_working_in_home_chan;

    oal_uint8 auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN]; /* 期望的bssid */
    mac_ssid_stru ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];          /* 期望的ssid */

    oal_uint8 uc_max_scan_count_per_channel;           /* 每个信道的扫描次数 */
    oal_uint8 uc_max_send_probe_req_count_per_channel; /* 每次信道发送扫描请求帧的个数，默认为1 */
    oal_uint8 auc_resv[2];

    oal_bool_enum_uint8 en_need_switch_back_home_channel; /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    oal_uint8 uc_scan_channel_interval;                   /* 间隔n个信道，切回工作信道工作一段时间 */
    oal_uint16 us_work_time_on_home_channel;              /* 背景扫描时，返回工作信道工作的时间 */

    oal_uint8 uc_last_channel_band;
    oal_uint8 bit_is_p2p0_scan : 1;                 /* 是否为p2p0 发起扫描 */
    oal_uint8 bit_rsv : 7;                          /* 保留位 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否是随机mac addr扫描 */
    oal_bool_enum_uint8 en_abort_scan_flag;         /* 终止扫描 */

    mac_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    oal_uint8 uc_channel_nums; /* 信道列表中信道的个数 */
    oal_uint8 uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    oal_uint16 us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    oal_uint8 uc_curr_channel_scan_count;   /* 记录当前信道的扫描次数，第一次发送广播ssid的probe req帧，后面发送指定ssid的probe req帧 */
    oal_uint8 uc_scan_func;                 /* DMAC SCANNER 扫描模式 */
    oal_uint8 uc_vap_id;                    /* 下发扫描请求的vap id */
    oal_uint64 ull_cookie;                  /* P2P 监听下发的cookie 值 */

    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_scan_req_stru;

/* 打印接收报文的rssi信息的调试开关相关的结构体 */
typedef struct {
    oal_uint32 ul_rx_comp_isr_interval;          /* 间隔多少个接收完成中断打印一次rssi信息 */
    oal_uint32 ul_curr_rx_comp_isr_count;        /* 一轮间隔内，接收完成中断的产生个数 */
    oal_bool_enum_uint8 en_rssi_debug_switch;    /* 打印接收报文的rssi信息的调试开关 */
    oal_bool_enum_uint8 en_tsensor_debug_switch; /* Tsensor信息的调试开关 */
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_bool_enum_uint8 en_pdet_debug_switch; /* 打印芯片上报pdet值的调试开关 */
#endif
    oal_uint8 uc_edca_param_switch; /* EDCA参数设置开关 */
    oal_uint8 uc_edca_aifsn;        /* EDCA参数AFSIN */
    oal_uint8 uc_edca_cwmin;        /* EDCA参数CWmin */
    oal_uint8 uc_edca_cwmax;        /* EDCA参数CWmax */
    oal_uint16 us_edca_txoplimit;   /* EDCA参数TXOP limit */
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    oal_uint8 uc_extlna_chg_bypass_switch; /* 动态bypass外置LNA开关: 0/1/2:no_bypass/dyn_bypass/force_bypass */
#endif
} mac_phy_debug_switch_stru;

typedef struct {
    oal_uint8 uc_opmode_switch;
} mac_opmode_switch_stru;
typedef struct {
    oal_uint32 ul_cmd_bit_map;
#ifdef _PRE_WLAN_FEATURE_APF
    oal_uint8 uc_apf_switch;
    oal_uint8 uc_ao_switch;
#endif
#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
    oal_uint8 uc_dto_switch;
    oal_uint16 us_torv_val;
#endif
} mac_pm_debug_cfg_stru;

typedef enum {
#ifdef _PRE_WLAN_FEATURE_APF
    MAC_PM_DEBUG_APF = 0,
    MAC_PM_DEBUG_AO = 1,
#endif
#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
    MAC_PM_DEBUG_DTO = 2,
    MAC_PM_DEBUG_TORV = 3,
#endif
    MAC_PM_DEBUG_CFG_BUTT
} mac_pm_debug_cfg_enum_uint8;

/* ACS 命令及回复格式 */
typedef struct {
    oal_uint8 uc_cmd;
    oal_uint8 uc_chip_id;
    oal_uint8 uc_device_id;
    oal_uint8 uc_resv;

    oal_uint32 ul_len;     /* 总长度，包括上面前4个字节 */
    oal_uint32 ul_cmd_cnt; /* 命令的计数 */
} mac_acs_response_hdr_stru;

typedef struct {
    oal_uint8 uc_cmd;
    oal_uint8 auc_arg[3];
    oal_uint32 ul_cmd_cnt; /* 命令的计数 */
    oal_uint32 ul_cmd_len; /* 总长度，特指auc_data的实际负载长度 */
    oal_uint8 auc_data[4];
} mac_acs_cmd_stru;

typedef mac_acs_cmd_stru mac_init_scan_req_stru;

typedef enum {
    MAC_ACS_RSN_INIT,
    MAC_ACS_RSN_LONG_TX_BUF,
    MAC_ACS_RSN_LARGE_PER,
    MAC_ACS_RSN_MWO_DECT,
    MAC_ACS_RSN_RADAR_DECT,

    MAC_ACS_RSN_BUTT
} mac_acs_rsn_enum;
typedef oal_uint8 mac_acs_rsn_enum_uint8;

typedef enum {
    MAC_ACS_SW_NONE = 0x0,
    MAC_ACS_SW_INIT = 0x1,
    MAC_ACS_SW_DYNA = 0x2,
    MAC_ACS_SW_BOTH = 0x3,

    MAC_ACS_SW_BUTT
} en_mac_acs_sw_enum;
typedef oal_uint8 en_mac_acs_sw_enum_uint8;

typedef enum {
    MAC_ACS_SET_CH_DNYA = 0x0,
    MAC_ACS_SET_CH_INIT = 0x1,

    MAC_ACS_SET_CH_BUTT
} en_mac_acs_set_ch_enum;
typedef oal_uint8 en_mac_acs_set_ch_enum_uint8;

typedef struct {
    oal_bool_enum_uint8 en_sw_when_connected_enable : 1;
    oal_bool_enum_uint8 en_drop_dfs_channel_enable : 1;
    oal_bool_enum_uint8 en_lte_coex_enable : 1;
    en_mac_acs_sw_enum_uint8 en_acs_switch : 5;
} mac_acs_switch_stru;

/* DMAC SCAN 信道扫描BSS信息摘要结构 */
typedef struct {
    oal_int8 c_rssi;             /* bss的信号强度 */
    oal_uint8 uc_channel_number; /* 信道号 */
    oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN];

    /* 11n, 11ac信息 */
    oal_bool_enum_uint8 en_ht_capable;                      /* 是否支持ht */
    oal_bool_enum_uint8 en_vht_capable;                     /* 是否支持vht */
    wlan_bw_cap_enum_uint8 en_bw_cap;                       /* 支持的带宽 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8 en_channel_bandwidth; /* 信道带宽配置 */
} mac_scan_bss_stats_stru;

typedef struct {
    oal_uint8 uc_category;
    oal_uint8 uc_action_code;
    oal_uint8 auc_oui[3];
    oal_uint8 uc_eid;
    oal_uint8 uc_lenth;
    oal_uint8 uc_location_type;
    oal_uint8 auc_mac_server[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_mac_client[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_payload[4];
} mac_location_event_stru;

/* DMAC SCAN 信道统计测量结果结构体 */
typedef struct {
    oal_uint8 uc_channel_number; /* 信道号 */
    oal_uint8 uc_stats_valid;
    oal_uint8 uc_stats_cnt;      /* 信道繁忙度统计次数 */
    oal_uint8 uc_free_power_cnt; /* 信道空闲功率 */

    oal_uint8 uc_bandwidth_mode;
    oal_uint8 auc_resv[1];
    oal_int16 s_free_power_stats_20M;
    oal_int16 s_free_power_stats_40M;
    oal_int16 s_free_power_stats_80M;

    oal_uint32 ul_total_stats_time_us;
    oal_uint32 ul_total_free_time_20M_us;
    oal_uint32 ul_total_free_time_40M_us;
    oal_uint32 ul_total_free_time_80M_us;
    oal_uint32 ul_total_send_time_us;
    oal_uint32 ul_total_recv_time_us;

    oal_uint8 uc_radar_detected;
    oal_uint8 uc_radar_bw;
    oal_uint8 uc_radar_type;
    oal_uint8 uc_radar_freq_offset;
} mac_scan_chan_stats_stru;

typedef struct {
    oal_int8 c_rssi;             /* bss的信号强度 */
    oal_uint8 uc_channel_number; /* 信道号 */

    oal_bool_enum_uint8 en_ht_capable : 1;                      /* 是否支持ht */
    oal_bool_enum_uint8 en_vht_capable : 1;                     /* 是否支持vht */
    wlan_bw_cap_enum_uint8 en_bw_cap : 3;                       /* 支持的带宽 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8 en_channel_bandwidth : 3; /* 信道带宽配置 */
} mac_scan_bss_stats_simple_stru;

typedef struct {
    oal_uint32 us_total_stats_time_ms : 9;  // max 512 ms
    oal_uint32 uc_bandwidth_mode : 3;
    oal_uint32 uc_radar_detected : 1;
    oal_uint32 uc_dfs_check_needed : 1;
    oal_uint32 uc_radar_bw : 3;
    oal_uint32 uc_radar_type : 4;
    oal_uint32 uc_radar_freq_offset : 3;
    oal_uint8 uc_channel_number; /* 信道号 */

    oal_int8 s_free_power_20M;  // dBm
    oal_int8 s_free_power_40M;
    oal_int8 s_free_power_80M;
    oal_uint8 uc_free_time_20M_rate;  // percent, 255 scaled
    oal_uint8 uc_free_time_40M_rate;
    oal_uint8 uc_free_time_80M_rate;
    oal_uint8 uc_total_send_time_rate;  // percent, 255 scaled
    oal_uint8 uc_total_recv_time_rate;
} mac_scan_chan_stats_simple_stru;

/* DMAC SCAN 回调事件结构体 */
typedef struct {
    oal_uint8 uc_nchans;    /* 信道数量 */
    oal_uint8 uc_nbss;      /* BSS数量 */
    oal_uint8 uc_scan_func; /* 扫描启动的功能 */

    oal_uint8 uc_need_rank : 1;  // kernel write, app read
    oal_uint8 uc_obss_on : 1;
    oal_uint8 uc_dfs_on : 1;
    oal_uint8 uc_dbac_on : 1;
    oal_uint8 uc_chip_id : 2;
    oal_uint8 uc_device_id : 2;
} mac_scan_event_stru;

/* bss安全相关信息结构体 */
typedef struct {
    oal_uint8 uc_bss_80211i_mode;                                      /* 指示当前AP的安全方式是WPA或WPA2。BIT0: WPA; BIT1:WPA2 */
    oal_uint8 uc_rsn_grp_policy;                                       /* 用于存放WPA2方式下，AP的组播加密套件信息 */
    oal_uint8 auc_rsn_pairwise_policy[MAC_PAIRWISE_CIPHER_SUITES_NUM]; /* 用于存放WPA2方式下，AP的单播加密套件信息 */
    oal_uint8 auc_rsn_auth_policy[MAC_AUTHENTICATION_SUITE_NUM];       /* 用于存放WPA2方式下，AP的认证套件信息 */
    oal_uint8 uc_group_mgmt_policy;
    oal_uint8 auc_rsn_cap[2];                                          /* 用于保存RSN能力信息，直接从帧内容中copy过来 */
    oal_uint8 auc_wpa_pairwise_policy[MAC_PAIRWISE_CIPHER_SUITES_NUM]; /* 用于存放WPA方式下，AP的单播加密套件信息 */
    oal_uint8 auc_wpa_auth_policy[MAC_AUTHENTICATION_SUITE_NUM];       /* 用于存放WPA方式下，AP的认证套件信息 */
    oal_uint8 uc_wpa_grp_policy;                                       /* 用于存放WPA方式下，AP的组播加密套件信息 */
    oal_uint8 uc_grp_policy_match;                                     /* 用于存放匹配的组播套件 */
    oal_uint8 uc_pairwise_policy_match;                                /* 用于存放匹配的单播套件 */
    oal_uint8 uc_auth_policy_match;                                    /* 用于存放匹配的认证套件 */
} mac_bss_80211i_info_stru;

/* 扫描完成事件返回状态码 */
typedef enum {
    MAC_SCAN_SUCCESS = 0,    /* 扫描成功 */
    MAC_SCAN_PNO = 1,        /* pno扫描结束 */
    MAC_SCAN_TIMEOUT = 2,    /* 扫描超时 */
    MAC_SCAN_REFUSED = 3,    /* 扫描被拒绝 */
    MAC_SCAN_ABORT = 4,      /* 终止扫描 */
    MAC_SCAN_ABORT_SYNC = 5, /* 扫描被终止同步状态，用于上层去关联命令时强制abort，不直接往内核上报scan abort结果，等dmac响应abort完成后再往上报 */
    MAC_SCAN_STATUS_BUTT,    /* 无效状态码，初始化时使用此状态码 */
} mac_scan_status_enum;
typedef oal_uint8 mac_scan_status_enum_uint8;

/* 扫描结果 */
typedef struct {
    mac_scan_status_enum_uint8 en_scan_rsp_status;
    oal_uint8 auc_resv[3];
    oal_uint64 ull_cookie;
} mac_scan_rsp_stru;

/* 扫描到的BSS描述结构体 */
typedef struct {
    /* 基本信息 */
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type; /* bss网络类型 */
    oal_uint8 uc_dtim_period;                        /* dtime周期 */
    oal_uint8 uc_dtim_cnt;                           /* dtime cnt */
    oal_bool_enum_uint8 en_11ntxbf;                  /* 11n txbf */
    oal_bool_enum_uint8 en_new_scan_bss;             /* 是否是新扫描到的BSS */
    wlan_ap_chip_oui_enum_uint8 en_is_tplink_oui;
    oal_int8 c_rssi;                               /* bss的信号强度 */
    oal_int8 ac_ssid[WLAN_SSID_MAX_LEN];           /* 网络ssid */
    oal_uint16 us_beacon_period;                   /* beacon周期 */
    oal_uint16 us_cap_info;                        /* 基本能力信息 */
    oal_uint8 auc_mac_addr[WLAN_MAC_ADDR_LEN];     /* 基础型网络 mac地址与bssid相同 */
    oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN];        /* 网络bssid */
    mac_channel_stru st_channel;                   /* bss所在的信道 */
    oal_uint8 uc_wmm_cap;                          /* 是否支持wmm */
    oal_uint8 uc_uapsd_cap;                        /* 是否支持uapsd */
    oal_bool_enum_uint8 en_desired;                /* 标志位，此bss是否是期望的 */
    oal_uint8 uc_num_supp_rates;                   /* 支持的速率集个数 */
    oal_uint8 auc_supp_rates[WLAN_MAX_SUPP_RATES]; /* 支持的速率集 */

#ifdef _PRE_WLAN_FEATURE_11D
    oal_int8 ac_country[WLAN_COUNTRY_STR_LEN]; /* 国家字符串 */
    oal_uint8 auc_resv2[1];
#endif

    /* 安全相关的信息 */
    mac_bss_80211i_info_stru st_bss_sec_info; /* 用于保存STA模式下，扫描到的AP安全相关信息 */

    /* 11n 11ac信息 */
    oal_bool_enum_uint8 en_ht_capable;            /* 是否支持ht */
    oal_bool_enum_uint8 en_vht_capable;           /* 是否支持vht */
    oal_bool_enum_uint8 en_epigram_vht_capable;   /* 是否支持hidden vendor vht */
    oal_bool_enum_uint8 en_epigram_novht_capable; /* 私有vendor中不需再携带vht */

    wlan_bw_cap_enum_uint8 en_bw_cap;                       /* 支持的带宽 0-20M 1-40M */
    wlan_channel_bandwidth_enum_uint8 en_channel_bandwidth; /* 信道带宽 */
    oal_uint8 uc_coex_mgmt_supp;                            /* 是否支持共存管理 */
    oal_bool_enum_uint8 en_ht_ldpc;                         /* 是否支持ldpc */

    oal_bool_enum_uint8 en_ht_stbc; /* 是否支持stbc */
    oal_uint8 uc_wapi;
    oal_uint8 auc_resv[1];
    oal_bool_enum_uint8 en_btcoex_blacklist_chip_oui; /* ps机制one pkt帧类型需要修订为self-cts等 */
    oal_uint32 ul_timestamp;                          /* 更新此bss的时间戳 */

#ifdef _PRE_WLAN_NARROW_BAND
    oal_bool_enum_uint8 en_nb_capable; /* 是否支持nb */
#endif
    oal_bool_enum_uint8 en_is_realtek_chip_oui; /* 是否realtek芯片ap，解决bt仅打开问题 */
    oal_bool_enum_uint8 en_11k_capable; /* 是否支持11k */
    oal_bool_enum_uint8 en_11v_capable; /* 是否支持11v */

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
    oal_bool_enum_uint8 en_support_rrm; /* 是否支持RRM */
#endif
    wlan_nss_enum_uint8 en_support_max_nss; /* 该AP支持的最大空间流数 */
    oal_uint8 uc_num_sounding_dim;          /* 该AP发送txbf的天线数 */

#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_bool_enum_uint8 en_roam_blacklist_chip_oui; /* 不支持roam */
#endif

    /* 管理帧信息 */
    oal_uint32 ul_mgmt_len;     /* 管理帧的长度 */
    oal_uint8 auc_mgmt_buff[4]; /* 记录beacon帧或probe rsp帧 */
    // oal_uint8                        *puc_mgmt_buff;                     /* 记录beacon帧或probe rsp帧 */
} mac_bss_dscr_stru;

#ifdef _PRE_WLAN_DFT_STAT
/* 管理帧统计信息 */
typedef struct {
    /* 接收管理帧统计 */
    oal_uint32 aul_rx_mgmt[WLAN_MGMT_SUBTYPE_BUTT];

    /* 挂到硬件队列的管理帧统计 */
    oal_uint32 aul_tx_mgmt_soft[WLAN_MGMT_SUBTYPE_BUTT];

    /* 发送完成的管理帧统计 */
    oal_uint32 aul_tx_mgmt_hardware[WLAN_MGMT_SUBTYPE_BUTT];
} mac_device_mgmt_statistic_stru;
#endif
#ifdef _PRE_WLAN_DFT_STAT
/* 上报空口环境类维测参数的控制结构 */
typedef struct {
    oal_uint32 ul_non_directed_frame_num;               /* 接收到非本机帧的数目 */
    oal_uint8 uc_collect_period_cnt;                    /* 采集周期的次数，到达100后就上报参数，然后清零重新开始 */
    oal_bool_enum_uint8 en_non_directed_frame_stat_flg; /* 是否统计非本机地址帧个数的标志 */
    oal_int16 s_ant_power;                              /* 天线口功率 */
    frw_timeout_stru st_collect_period_timer;           /* 采集周期定时器 */
} mac_device_dbb_env_param_ctx_stru;
#endif

typedef enum {
    MAC_DFR_TIMER_STEP_1 = 0,
    MAC_DFR_TIMER_STEP_2 = 1,
} mac_dfr_timer_step_enum;
typedef oal_uint8 mac_dfr_timer_step_enum_uint8;

typedef struct {
    oal_uint32 ul_tx_seqnum;         /* 最近一次tx上报的SN号 */
    oal_uint16 us_seqnum_used_times; /* 软件使用了ul_tx_seqnum的次数 */
    oal_uint16 us_incr_constant;     /* 维护非Qos 分片帧的递增常量 */
} mac_tx_seqnum_struc;

typedef struct {
    oal_uint8 uc_p2p_device_num;                /* 当前device下的P2P_DEVICE数量 */
    oal_uint8 uc_p2p_goclient_num;              /* 当前device下的P2P_CL/P2P_GO数量 */
    oal_uint8 uc_p2p0_vap_idx;                  /* P2P 共存场景下，P2P_DEV(p2p0) 指针 */
    mac_vap_state_enum_uint8 en_last_vap_state; /* P2P0/P2P_CL 共用VAP 结构，监听场景下保存VAP 进入监听前的状态 */
    oal_uint8 uc_resv[2];                       /* 保留 */
    oal_uint8 en_roc_need_switch;               /* remain on channel后需要切回原信道 */
    oal_uint8 en_p2p_ps_pause;                  /* P2P 节能是否处于pause状态 */
    oal_net_device_stru *pst_p2p_net_device;    /* P2P 共存场景下主net_device(p2p0) 指针 */
    oal_uint64 ull_send_action_id;              /* P2P action id/cookie */
    oal_uint64 ull_last_roc_id;
    oal_ieee80211_channel_stru st_listen_channel;
    oal_nl80211_channel_type en_listen_channel_type;
    oal_net_device_stru *pst_primary_net_device; /* P2P 共存场景下主net_device(wlan0) 指针 */
} mac_p2p_info_stru;

/* 屏幕状态 */
typedef enum {
    MAC_SCREEN_OFF = 0,
    MAC_SCREEN_ON = 1,
} mac_screen_state_enum;

typedef struct {
    oal_bool_enum_uint8 en_11k;
    oal_bool_enum_uint8 en_11v;
    oal_bool_enum_uint8 en_11r;
    oal_uint8 auc_rsv[1];
} mac_device_voe_custom_stru;

#ifdef _PRE_WLAN_CHBA_MGMT
typedef struct {
    uint8_t island_coex_chan_cnt;
    uint8_t island_coex_channels_list[MAX_CHANNEL_NUM_FREQ_5G];
} mac_chba_island_coex_info;
#endif

typedef struct {
    oal_uint8 upper_coch_intf_cnt;       /* 检测到超规格协议同频干扰计数 */
    oal_uint8 no_coch_intf_cnt;          /* 检测到无同频干扰计数 */
    oal_uint8 upper_coch_intf_state : 1, /* 超规格协议同频干扰状态 */
              sec40_adj_intf_state  : 1, /* 辅40M叠频干扰状态 */
              sec40_no_intf_cnt     : 6; /* 检测到无叠频干扰计数 */
    oal_uint8 sec40_intf_cnt;            /* 检测到辅40M叠频干扰计数 */

    oal_uint16 rx_ratio;                 /* phy统计接收时间占比（单位：千分之一） */
    oal_uint16 us_chload_20m;
    oal_uint16 us_chload_40m;
    oal_uint16 us_chload_80m;
} mac_rom_device_stru;

/* device结构体 */
typedef struct {
    oal_uint32 ul_core_id;
    oal_uint8 auc_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE]; /* device下的业务vap，此处只记录VAP ID */
    oal_uint8 uc_cfg_vap_id;               /* 配置vap ID */
    oal_uint8 uc_device_id;                /* 芯片ID */
    oal_uint8 uc_chip_id;                  /* 设备ID */
    oal_uint8 uc_device_reset_in_progress; /* 复位处理中 */

    oal_bool_enum_uint8 en_device_state; /* 标识是否已经被分配，(OAL_TRUE初始化完成，OAL_FALSE未初始化 ) */
    oal_uint8 uc_vap_num;                /* 当前device下的业务VAP数量(AP+STA) */
    oal_uint8 uc_sta_num;                /* 当前device下的STA数量 */
    mac_p2p_info_stru st_p2p_info; /* P2P 相关信息 */
    oal_uint8 auc_hw_addr[WLAN_MAC_ADDR_LEN]; /* 从eeprom或flash获得的mac地址，ko加载时调用hal接口赋值 */
    /* device级别参数 */
    oal_uint8 uc_max_channel;                 /* 已配置VAP的信道号，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */
    wlan_channel_band_enum_uint8 en_max_band; /* 已配置VAP的频段，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */

    wlan_channel_bandwidth_enum_uint8 en_max_bandwidth; /* 已配置VAP的最带带宽值，其后的VAP配置值不能与此值矛盾，仅在非DBAC时使用 */
    oal_uint8 uc_tx_chain;                              /* 发送通道 */
    oal_uint8 uc_rx_chain;                              /* 接收通道 */
    wlan_nss_enum_uint8 en_nss_num;                     /* Nss 空间流个数 */

    oal_bool_enum_uint8 en_wmm; /* wmm使能开关 */
    wlan_tidno_enum_uint8 en_tid;
    oal_uint8 en_reset_switch; /* 是否使能复位功能 */
    oal_uint8 uc_csa_vap_cnt;  /* 每个running AP发送一次CSA帧，该计数减1，减到零后，AP停止当前硬件发送，准备开始切换信道 */

    hal_to_dmac_device_stru *pst_device_stru; /* 硬mac结构指针，HAL提供，用于逻辑和物理device的对应 */

    oal_uint32 ul_beacon_interval;   /* device级别beacon interval,device下所有VAP约束为同一值 */
    oal_uint32 ul_duty_ratio;        /* 占空比统计 */
    oal_uint32 ul_duty_ratio_lp;     /* 进入低功耗前发送占空比 */
    oal_uint32 ul_rx_nondir_duty_lp; /* 进入低功耗前接收non-direct包的占空比 */
    oal_uint32 ul_rx_dir_duty_lp;    /* 进入低功耗前接收direct包的占空比 */

    /* device能力 */
    wlan_protocol_cap_enum_uint8 en_protocol_cap; /* 协议能力 */
    wlan_band_cap_enum_uint8 en_band_cap;         /* 频段能力 */
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;      /* 带宽能力 */
    oal_uint8 bit_ldpc_coding : 1,                /* 是否支持接收LDPC编码的包 */
              bit_tx_stbc : 1,                          /* 是否支持最少2x1 STBC发送 */
              bit_rx_stbc : 3,                          /* 是否支持stbc接收 */
              bit_su_bfmer : 1,                         /* 是否支持单用户beamformer */
              bit_su_bfmee : 1,                         /* 是否支持单用户beamformee */
              bit_mu_bfmee : 1;                         /* 是否支持多用户beamformee */
    oal_uint8 bit_nb_is_supp : 1,                 /* 是否支持窄带 */
              bit_reserve : 7;

    oal_uint16 us_device_reset_num; /* 复位的次数统计 */

    mac_data_rate_stru st_mac_rates_11g[MAC_DATARATES_PHY_80211G_NUM]; /* 11g速率 */

    mac_pno_sched_scan_mgmt_stru *pst_pno_sched_scan_mgmt; /* pno调度扫描管理结构体指针，内存动态申请，从而节省内存 */
    mac_scan_req_stru st_scan_params;                      /* 最新一次的扫描参数信息 */
    frw_timeout_stru st_scan_timer;                        /* 扫描定时器用于切换信道 */
    frw_timeout_stru st_obss_scan_timer;                   /* obss扫描定时器，循环定时器 */
    mac_channel_stru st_p2p_vap_channel;                   /* p2p listen时记录p2p的信道，用于p2p listen结束后恢复 */

    oal_uint8 uc_active_user_cnt;          /* 活跃用户数 */
    oal_uint8 uc_asoc_user_cnt;            /* 关联用户数 */
    oal_bool_enum_uint8 en_2040bss_switch; /* 20/40 bss检测开关 */
    oal_uint8 uc_in_suspend;
#ifdef _PRE_WLAN_FEATURE_11K
    // frw_timeout_stru                    st_backoff_meas_timer;
#endif
    oal_uint8 uc_mac_vap_id;             /* 多vap共存时，保存睡眠前的mac vap id */
    mac_bss_id_list_stru st_bss_id_list; /* 当前信道下的扫描结果 */

    oal_bool_enum_uint8 en_dbac_enabled;
    oal_bool_enum_uint8 en_dbac_running;       /* DBAC是否在运行 */
    oal_bool_enum_uint8 en_dbac_has_vip_frame; /* 标记DBAC运行时收到了关键帧 */
    oal_uint8 uc_arpoffload_switch;
    oal_uint8 uc_wapi;
    oal_uint8 uc_reserve;
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 随机mac扫描开关,从hmac下发 */
    oal_uint8 auc_mac_oui[WLAN_RANDOM_MAC_OUI_LEN]; /* 随机mac地址OUI,由系统下发 */
    oal_bool_enum_uint8 en_apf_switch;
    oal_uint8 auc_rsv[1];

    /* 针对Device的成员，待移动到dmac_device */
#if IS_DEVICE
    oal_uint8 auc_resv12[2];
    oal_uint16 us_total_mpdu_num;                                /* device下所有TID中总共的mpdu_num数量 */
    oal_uint16 aus_ac_mpdu_num[WLAN_WME_AC_BUTT];                /* device下各个AC对应的mpdu_num数 */
    oal_uint16 aus_vap_mpdu_num[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT]; /* 统计各个vap对应的mpdu_num数 */

    oal_void *p_alg_priv; /* 算法私有结构体 */
    oal_uint32 ul_first_timestamp; /* 记录性能统计第一次时间戳 */

    oal_uint8 auc_tx_ba_index_table[DMAC_TX_BA_LUT_BMAP_LEN]; /* 发送端LUT表 */

    /* 扫描相关成员变量 */
    oal_uint32 ul_scan_timestamp;                 /* 记录最新一次扫描开始的时间 */
    oal_uint8 uc_scan_chan_idx;                   /* 当前扫描信道索引 */
    mac_scan_state_enum_uint8 en_curr_scan_state; /* 当前扫描状态，根据此状态处理obss扫描和host侧下发的扫描请求，以及扫描结果的上报处理 */

    oal_uint8 uc_resume_qempty_flag; /* 使能恢复qempty标识, 默认不使能 */
    oal_uint8 uc_scan_count;

    mac_channel_stru st_home_channel; /* 记录工作信道 供切回时使用 */
    mac_fcs_cfg_stru st_fcs_cfg;      /* 快速切信道结构体 */

    mac_scan_chan_stats_stru st_chan_result; /* dmac扫描时 一个信道的信道测量记录 */

    oal_uint8 auc_original_mac_addr[WLAN_MAC_ADDR_LEN]; /* 扫描开始前保存原始的MAC地址 */
    oal_uint8 uc_scan_ap_num_in_2p4;
    oal_bool_enum_uint8 en_scan_curr_chan_find_bss_flag; /* 本信道扫描是否扫描到BSS */

    /* 用户相关成员变量 */
    frw_timeout_stru st_active_user_timer; /* 用户活跃定时器 */

    oal_uint8 auc_ra_lut_index_table[WLAN_ACTIVE_USER_IDX_BMAP_LEN]; /* lut表位图 */

    mac_fcs_mgr_stru st_fcs_mgr;

    oal_uint8 uc_csa_cnt; /* 每个AP发送一次CSA帧，该计数加1。AP切换完信道后，该计数清零 */

    oal_bool_enum_uint8 en_txop_enable; /* 发送无聚合时采用TXOP模式 */
    oal_uint8 uc_tx_ba_num;             /* 发送方向BA会话个数 */
    oal_uint8 auc_resv[1];

    frw_timeout_stru st_keepalive_timer; /* keepalive定时器 */

#ifdef _PRE_DEBUG_MODE
    frw_timeout_stru st_exception_report_timer;
#endif
    oal_uint32 aul_mac_err_cnt[HAL_MAC_ERROR_TYPE_BUTT]; /* mac 错误计数器 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    hal_mac_key_statis_info_stru st_mac_key_statis_info; /* mac关键统计信息 */
#endif

#endif /* IS_DEVICE */

    /* 针对Host的成员，待移动到hmac_device */
#if IS_HOST
    /* linux内核中的device物理信息 */
    oal_wiphy_stru *pst_wiphy; /* 用于存放和VAP相关的wiphy设备信息，在AP/STA模式下均要使用；可以多个VAP对应一个wiphy */

#ifndef _PRE_WLAN_FEATURE_AMPDU_VAP
    oal_uint8 uc_rx_ba_session_num; /* 该device下，rx BA会话的数目 */
    oal_uint8 uc_tx_ba_session_num; /* 该device下，tx BA会话的数目 */
    oal_uint8 auc_resv11[2];
#endif
    oal_bool_enum_uint8 en_vap_classify; /* 是否使能基于vap的业务分类 */
    oal_uint8 uc_auth_req_sendst;
    oal_uint8 uc_asoc_req_sendst;
    oal_bool_enum_uint8 en_report_mgmt_req_status;

    oal_uint8 auc_rx_ba_lut_idx_table[DMAC_BA_LUT_IDX_BMAP_LEN]; /* 接收端LUT表 */

    frw_timeout_stru st_obss_aging_timer; /* OBSS保护老化定时器 */

    mac_ap_ch_info_stru st_ap_channel_list[MAC_MAX_SUPP_CHANNEL];
    oal_uint8 uc_ap_chan_idx;                        /* 当前扫描信道索引 */
    oal_bool_enum_uint8 en_fft_window_offset_enable; /* 是否已经配置FFT窗口 */
    oal_uint8 auc_resv21[2];

    oal_bool_enum_uint8 en_40MHz_intol_bit_recd;
#endif /* IS_HOST */
#ifdef _PRE_WLAN_FEATURE_FTM
    oal_uint8 uc_ftm_vap_id; /* ftm中断对应 vap ID */
    oal_uint8 en_nbfh_running;
    oal_uint8 en_nbfh_enabled;
#else
    oal_uint8 auc_resv4[1];
    oal_uint8 en_nbfh_running;
    oal_uint8 en_nbfh_enabled;
#endif
    /* ROM化后资源扩展指针 */
    mac_rom_device_stru *pst_mac_device_rom;
} mac_device_stru;

#pragma pack(push, 1)
/* 上报的扫描结果的扩展信息，放在上报host侧的管理帧netbuf的后面 */
typedef struct {
    oal_int32 l_rssi;                                /* 信号强度 */
    wlan_mib_desired_bsstype_enum_uint8 en_bss_type; /* 扫描到的bss类型 */
    oal_uint8 auc_resv[3];                           /* 预留字段 */
} mac_scanned_result_extend_info_stru;
#pragma pack(pop)

/* chip结构体 */
typedef struct {
    oal_uint8 auc_device_id[WLAN_DEVICE_MAX_NUM_PER_CHIP]; /* CHIP下挂的DEV，仅记录对应的ID索引值 */
    oal_uint8 uc_device_nums;                              /* chip下device的数目 */
    oal_uint8 uc_chip_id;                                  /* 芯片ID */
    oal_bool_enum_uint8 en_chip_state;                     /* 标识是否已初始化，OAL_TRUE初始化完成，OAL_FALSE未初始化 */
    oal_uint32 ul_chip_ver;                                /* 芯片版本 */
    hal_to_dmac_chip_stru *pst_chip_stru;                  /* 硬mac结构指针，HAL提供，用于逻辑和物理chip的对应 */
} mac_chip_stru;

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
typedef enum {
    MAC_RX_IP_FILTER_STOPED = 0,   // 功能关闭，未使能、或者其他状况不允许过滤动作。
    MAC_RX_IP_FILTER_WORKING = 1,  // 功能打开，按照规则正常过滤
    MAC_RX_IP_FILTER_BUTT
} mac_ip_filter_state_enum;
typedef oal_uint8 mac_ip_filter_state_enum_uint8;

typedef struct {
    mac_ip_filter_state_enum_uint8 en_state;  // 功能状态：过滤、非过滤等
    oal_uint8 uc_btable_items_num;            // 黑名单中目前存储的items个数
    oal_uint8 uc_btable_size;                 // 黑名单大小，表示最多存储的items个数
    oal_uint8 uc_resv;
    mac_ip_filter_item_stru *pst_filter_btable;  // 黑名单指针
} mac_rx_ip_filter_struc;
#endif  // _PRE_WLAN_FEATURE_IP_FILTER

/* board结构体 */
typedef struct {
    mac_chip_stru ast_chip[WLAN_CHIP_MAX_NUM_PER_BOARD]; /* board挂接的芯片 */
    oal_uint8 uc_chip_id_bitmap;                         /* 标识chip是否被分配的位图 */
    oal_uint8 auc_resv[3];                               /* 字节对齐 */
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    mac_rx_ip_filter_struc st_rx_ip_filter; /* rx ip过滤功能的管理结构体 */
#endif                                      // _PRE_WLAN_FEATURE_IP_FILTER
} mac_board_stru;

typedef struct {
    mac_device_stru *pst_mac_device;
} mac_wiphy_priv_stru;

typedef struct {
    wlan_csa_mode_tx_enum_uint8 en_mode;
    oal_uint8 uc_channel;
    oal_uint8 uc_cnt;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    mac_csa_flag_enum_uint8 en_debug_flag; /* 0:正常切信道; 1:仅beacon帧中含有csa,信道不切换;2:取消beacon帧中含有csa */
    oal_uint8 auc_reserv[3];
} mac_csa_debug_stru;

/* 带宽调试开关相关的结构体 */
typedef struct {
    oal_uint32 ul_cmd_bit_map;
    oal_bool_enum_uint8 en_band_force_switch; /* 改变带宽命令 */
    oal_uint8 auc_resv[3];
    mac_csa_debug_stru st_csa_debug;
} mac_protocol_debug_switch_stru;

typedef oal_rom_cb_result_enum_uint8 (*mac_device_init_rom_cb)(mac_device_stru *pst_mac_device,
                                                               oal_uint32 ul_chip_ver,
                                                               oal_uint8 uc_chip_id,
                                                               oal_uint8 uc_device_id,
                                                               oal_uint32 *pul_cb_ret);
typedef wlan_bw_cap_enum_uint8 (*mac_device_max_band_rom_cb)(oal_void);

typedef struct {
    mac_device_init_rom_cb p_device_init_cb;
    mac_device_max_band_rom_cb p_device_max_band_cb;
} mac_device_rom_stru;

typedef enum {
    MAC_PSM_QUERY_BEACON_CNT,
    MAC_PSM_QUERY_FLT_STAT,
    MAC_PSM_QUERY_TYPE_BUTT
}mac_psm_query_type_enum;
typedef oal_uint8 mac_psm_query_type_enum_uint8;
#define MAC_PSM_QUERY_MSG_MAX_STAT_ITEM 10
typedef struct {
    oal_uint32                          ul_query_item;
    oal_uint32                          aul_val[MAC_PSM_QUERY_MSG_MAX_STAT_ITEM];
}mac_psm_query_stat_stru;

typedef struct {
    mac_psm_query_type_enum_uint8 en_query_type;
    oal_uint8 auc_resv[3]; /* 3是pad长度 */
    mac_psm_query_stat_stru st_stat;
}mac_psm_query_msg;

typedef struct {
    oal_uint32                          ul_ao_drop_cnt;
    oal_uint32                          ul_ao_send_rsp_cnt;
    oal_uint32                          ul_apf_flt_drop_cnt;
    oal_uint32                          ul_icmp_flt_drop_cnt;
}mac_psm_flt_stat_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern oal_uint32 mac_get_band_5g_enabled(oal_void);
extern oal_void mac_set_band_5g_enabled(oal_uint32 band_5g_enabled);
/* 主逻辑中不想看到宏 */
#ifdef _PRE_WLAN_FEATURE_DBAC
#define MAC_DBAC_ENABLE(_pst_device) (_pst_device->en_dbac_enabled == OAL_TRUE)
#else
#define MAC_DBAC_ENABLE(_pst_device) (OAL_FALSE)
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
extern oal_bool_enum_uint8 g_en_wmmac_switch;
#endif


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dbac_enabled(mac_device_stru *pst_device)
{
    return pst_device->en_dbac_enabled;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_dbac_running(mac_device_stru *pst_device)
{
    if (pst_device->en_dbac_enabled == OAL_FALSE) {
        return OAL_FALSE;
    }

    return pst_device->en_dbac_running;
}

#ifdef _PRE_WLAN_FEATURE_DBAC

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_need_enqueue_tid_for_dbac(mac_device_stru *pst_device,
                                                                        mac_vap_stru *pst_vap)
{
    return (oal_bool_enum_uint8)((pst_device->en_dbac_enabled == OAL_TRUE) &&
                                 (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE));
}
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_get_2040bss_switch(mac_device_stru *pst_mac_device)
{
    return pst_mac_device->en_2040bss_switch;
}
OAL_STATIC OAL_INLINE oal_void mac_set_2040bss_switch(mac_device_stru *pst_mac_device,
                                                      oal_bool_enum_uint8 en_switch)
{
    pst_mac_device->en_2040bss_switch = en_switch;
}
#endif

#if IS_DEVICE
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_device_is_scaning(mac_device_stru *pst_mac_device)
{
    return (oal_bool_enum_uint8)(pst_mac_device->en_curr_scan_state == MAC_SCAN_STATE_RUNNING);
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_device_is_listening(mac_device_stru *pst_mac_device)
{
    return (oal_bool_enum_uint8)((pst_mac_device->en_curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
            (pst_mac_device->st_scan_params.uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN));
}
#endif /* IS_DEVICE */

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern mac_device_voe_custom_stru *mac_get_voe_custom_param_addr(oal_void);
/*****************************************************************************
  10.1 公共结构体初始化、删除
*****************************************************************************/
extern oal_uint32 mac_device_init(mac_device_stru *pst_mac_device, oal_uint32 ul_chip_ver, oal_uint8 chip_id,
                                  oal_uint8 uc_device_id);
extern oal_uint32 mac_chip_init(mac_chip_stru *pst_chip, oal_uint8 uc_chip_id);
extern oal_uint32 mac_board_init(mac_board_stru *pst_board);

extern oal_uint32 mac_device_exit(mac_device_stru *pst_device);
extern oal_uint32 mac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip);
extern oal_uint32 mac_board_exit(mac_board_stru *pst_board);

/*****************************************************************************
  10.2 公共成员访问部分
*****************************************************************************/
extern oal_void mac_device_set_vap_id(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_uint8 uc_vap_idx,
    wlan_vap_mode_enum_uint8 en_vap_mode, wlan_p2p_mode_enum_uint8 en_p2p_mode, oal_uint8 is_add_vap);
extern oal_void mac_device_set_dfr_reset(mac_device_stru *pst_mac_device, oal_uint8 uc_device_reset_in_progress);
extern oal_void mac_device_set_state(mac_device_stru *pst_mac_device, oal_uint8 en_device_state);

extern oal_void mac_device_set_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param);
extern oal_void mac_device_get_channel(mac_device_stru *pst_mac_device, mac_cfg_channel_param_stru *pst_channel_param);

extern oal_void mac_device_set_txchain(mac_device_stru *pst_mac_device, oal_uint8 uc_tx_chain);
extern oal_void mac_device_set_rxchain(mac_device_stru *pst_mac_device, oal_uint8 uc_rx_chain);
extern oal_void mac_device_set_beacon_interval(mac_device_stru *pst_mac_device, oal_uint32 ul_beacon_interval);
extern oal_void mac_device_inc_active_user(mac_device_stru *pst_mac_device);

extern oal_void mac_device_dec_active_user(mac_device_stru *pst_mac_device);

extern oal_void *mac_device_get_all_rates(mac_device_stru *pst_dev);
/*****************************************************************************
  10.3 杂项，待归类
*****************************************************************************/
extern oal_uint32 mac_device_find_legacy_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint32 mac_device_find_up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern mac_vap_stru *mac_device_find_another_up_vap(mac_device_stru *pst_mac_device, oal_uint8 uc_vap_id_self);
extern oal_uint32 mac_device_find_up_ap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint8 mac_device_calc_up_vap_num(mac_device_stru *pst_mac_device);
extern oal_uint32 mac_device_calc_work_vap_num(mac_device_stru *pst_mac_device);
extern oal_uint32 mac_device_find_up_p2p_go(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);
extern oal_uint32 mac_device_find_2up_vap(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap1,
                                          mac_vap_stru **ppst_mac_vap2);
extern oal_uint32 mac_fcs_dbac_state_check(mac_device_stru *pst_mac_device);
extern oal_uint32 mac_device_find_up_sta(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap);

extern oal_uint32 mac_device_is_p2p_connected(mac_device_stru *pst_mac_device);
/*****************************************************************************
  10.4 待移除
*****************************************************************************/
/*****************************************************************************
  11 inline函数定义
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_hide_ssid(oal_uint8 *puc_ssid_ie, oal_uint8 uc_ssid_len)
{
    return (oal_bool_enum_uint8)((puc_ssid_ie == OAL_PTR_NULL) || (uc_ssid_len == 0) || (puc_ssid_ie[0] == '\0'));
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_device_is_auto_chan_sel_enabled(mac_device_stru *pst_mac_device)
{
    /* BSS启动时，如果用户没有设置信道，则默认为开启自动信道选择 */
    return (!pst_mac_device->uc_max_channel);
}
extern hal_fcs_protect_type_enum_uint8 mac_fcs_get_protect_type(mac_vap_stru *pst_mac_vap);
extern oal_uint32 mac_fcs_init(mac_fcs_mgr_stru *pst_fcs_mgr,
                               oal_uint8 uc_chip_id,
                               oal_uint8 uc_device_id);

extern mac_fcs_err_enum_uint8 mac_fcs_request(mac_fcs_mgr_stru *pst_fcs_mgr,
                                              mac_fcs_state_enum_uint8 *puc_state,
                                              mac_fcs_cfg_stru *pst_fcs_cfg);

extern void mac_fcs_release(mac_fcs_mgr_stru *pst_fcs_mgr);

extern mac_fcs_err_enum_uint8 mac_fcs_start(mac_fcs_mgr_stru *pst_fcs_mgr,
                                            mac_fcs_cfg_stru *pst_fcs_cfg,
                                            hal_one_packet_status_stru *pst_status,
                                            oal_uint8 uc_fake_tx_q_id);
extern oal_void mac_fcs_set_one_pkt_timeout_time(mac_fcs_mgr_stru *pst_fcs_mgr);

extern mac_fcs_err_enum_uint8 mac_fcs_start_enhanced(mac_fcs_mgr_stru *pst_fcs_mgr, mac_fcs_cfg_stru *pst_fcs_cfg);
extern oal_void mac_fcs_send_one_packet_start(mac_fcs_mgr_stru *pst_fcs_mgr,
                                              hal_one_packet_cfg_stru *pst_one_packet_cfg,
                                              hal_to_dmac_device_stru *pst_device,
                                              hal_one_packet_status_stru *pst_status,
                                              oal_bool_enum_uint8 en_ps);
extern oal_uint32 mac_fcs_notify_chain_register(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                mac_fcs_notify_type_enum_uint8 uc_notify_type,
                                                mac_fcs_hook_id_enum_uint8 en_hook_id,
                                                mac_fcs_notify_func p_func);

extern oal_uint32 mac_fcs_notify(mac_fcs_mgr_stru *pst_fcs_mgr,
                                 mac_fcs_notify_type_enum_uint8 uc_notify_type);

extern oal_uint32 mac_fcs_notify_chain_unregister(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                  mac_fcs_notify_type_enum_uint8 uc_notify_type,
                                                  mac_fcs_hook_id_enum_uint8 en_hook_id);

extern oal_uint32 mac_fcs_notify_chain_destroy(mac_fcs_mgr_stru *pst_fcs_mgr);

extern oal_uint32 mac_fcs_get_prot_mode(mac_vap_stru *pst_src_vap);
extern oal_uint32 mac_fcs_get_prot_datarate(mac_vap_stru *pst_src_vap);
extern oal_void mac_fcs_prepare_one_packet_cfg(mac_vap_stru *pst_mac_vap,
                                               hal_one_packet_cfg_stru *pst_one_packet_cfg,
                                               oal_uint16 us_protect_time,
                                               mac_one_packet_index_enum_uint8 uc_one_packet_index);

extern oal_void mac_fcs_flush_event_by_channel(mac_device_stru *pst_mac_device, mac_channel_stru *pst_chl);

extern oal_uint32 mac_fcs_set_channel(mac_device_stru *pst_mac_device,
                                      mac_channel_stru *pst_channel);
extern mac_fcs_err_enum_uint8 mac_fcs_start_same_channel(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                         mac_fcs_cfg_stru *pst_fcs_cfg,
                                                         hal_one_packet_status_stru *pst_status,
                                                         oal_uint8 uc_fake_tx_q_id);

extern mac_fcs_err_enum_uint8 mac_fcs_start_enhanced_same_channel(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                                  mac_fcs_cfg_stru *pst_fcs_cfg);

extern oal_rom_cb_result_enum_uint8 mac_device_init_cb(mac_device_stru *pst_mac_device,
                                                       oal_uint32 ul_chip_ver,
                                                       oal_uint8 uc_chip_id,
                                                       oal_uint8 uc_device_id,
                                                       oal_uint32 *pul_cb_ret);
extern wlan_bw_cap_enum_uint8 mac_device_max_band_cb(oal_void);
extern oal_uint32 mac_fcs_wait_one_packet_done_same_channel(mac_fcs_mgr_stru *pst_fcs_mgr);
extern oal_void mac_fcs_send_one_packet_start_same_channel(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                           hal_one_packet_cfg_stru *pst_one_packet_cfg,
                                                           hal_to_dmac_device_stru *pst_device,
                                                           hal_one_packet_status_stru *pst_status,
                                                           oal_bool_enum_uint8 en_ps);


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_fcs_is_same_channel(mac_channel_stru *pst_channel_dst,
                                                                  mac_channel_stru *pst_channel_src)
{
    return (oal_bool_enum_uint8)(pst_channel_dst->uc_chan_number == pst_channel_src->uc_chan_number);
}


OAL_STATIC OAL_INLINE oal_uint8 mac_fcs_get_protect_cnt(mac_vap_stru *pst_mac_vap)
{
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return HAL_FCS_PROTECT_CNT_1;
    }

    return HAL_FCS_PROTECT_CNT_3;
}

#if (_PRE_TEST_MODE_BOARD_ST == _PRE_TEST_MODE)
extern oal_void mac_fcs_verify_init(oal_void);
extern oal_void mac_fcs_verify_start(oal_void);
extern oal_void mac_fcs_verify_timestamp(mac_fcs_stage_enum_uint8 en_stage);
extern oal_void mac_fcs_verify_stop(oal_void);

#else
#define mac_fcs_verify_init()
#define mac_fcs_verify_start()
#define mac_fcs_verify_timestamp(a)
#define mac_fcs_verify_stop()
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_device.h */
