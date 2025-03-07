

#ifndef __HAL_COMMOM_OPS_H__
#define __HAL_COMMOM_OPS_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "wlan_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************/
/*****************************************************************************/
/* HI1102 产品宏定义、枚举 */
/*****************************************************************************/
/*****************************************************************************/
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)) ||  \
    ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)) ||  \
    ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)) ||  \
    ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))
#define HAL_PUBLIC_HOOK_FUNC(_func) \
    hi1102##_func
#endif
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
#define HAL_PUBLIC_HOOK_FUNC(_func) \
    hi1103##_func
#endif

#define HAL_RX_DSCR_GET_SW_ADDR(_addr)   hal_rx_dscr_get_sw_addr(_addr)   /* 一个字节中包含的bit数目 */
#define HAL_RX_DSCR_GET_REAL_ADDR(_addr) hal_rx_dscr_get_real_addr(_addr) /* 一个字节中包含的bit数目 */

#define HAL_MAX_AP_NUM  2                                               /* HAL AP个数 */
#define HAL_MAX_STA_NUM 3                                               /* HAL STA个数 */
#define HAL_MAX_VAP_NUM              (HAL_MAX_AP_NUM + HAL_MAX_STA_NUM) /* HAL VAP???? */

#define HAL_VAP_ID_IS_VALID(_vap_id) ((_vap_id == 0) || (_vap_id == 1) ||  \
                                      (_vap_id == 4) || (_vap_id == 5) || (_vap_id == 6))
#define HAL_VAP_ID_IS_VALID_PSTA(_vap_id) ((uc_vap_id < WLAN_HAL_OHTER_BSS_ID) ||  \
                                           ((uc_vap_id >= WLAN_PROXY_STA_START_ID) && \
                                           (uc_vap_id <= WLAN_PROXY_STA_END_ID)))

#define GNSS_SCAN_MAX_AP_NUM_TO_GNSS 32
#define GNSS_SCAN_RESULTS_VALID_MS   5000

#define HAL_DEVICE_2G_BAND_NUM_FOR_LOSS 3 /* 2g 线损的band个数，用于线损相关的计算 */
#define HAL_DEVICE_5G_BAND_NUM_FOR_LOSS WLAN_5G_SUB_BAND_NUM

#define HAL_DEVICE_2G_DELTA_RSSI_NUM 2 /* 20M/40M */
#define HAL_DEVICE_5G_DELTA_RSSI_NUM 3 /* 20M/40M/80M */

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
#define HAL_AL_TX_DYN_CAL_INTERVAL_NUM 10 /* 不指定数目常发动态校准帧间隔 */
#endif

#ifdef _PRE_WLAN_FEATURE_NRCOEX
#define HAL_NRCOEX_RULE_NUM        4  /* NR共存规则个数 */
#define HAL_NRCOEX_RULE_PARAMS_NUM 11 /* NR共存每个规则的定制化个数 */
#endif
#define HAL_RATE_100KBPS_TO_KBPS 100 /* 速率以100Kbps为单位到Kbps对应的倍乘数 */
/*****************************************************************************
  3 枚举
*****************************************************************************/
typedef enum {
    HAL_TX_QUEUE_BE = 0, /* 尽力而为业务 */
    HAL_TX_QUEUE_BK = 1, /* 背景业务 */
    HAL_TX_QUEUE_VI = 2, /* 视频业务 */
    HAL_TX_QUEUE_VO = 3, /* 语音业务 */
    HAL_TX_QUEUE_HI = 4, /* 高优先级队列(管理帧/控制帧用此队列) */
    HAL_TX_QUEUE_BUTT
} hal_tx_queue_type_enum;
typedef oal_uint8 hal_tx_queue_type_enum_uint8;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* sw preempt机制下蓝牙业务状态，a2dp|transfer  page|inquiry 或者  both */
typedef enum {
    HAL_BTCOEX_PS_STATUE_ACL = 1,       /* only a2dp|数传 BIT0 */
    HAL_BTCOEX_PS_STATUE_PAGE_INQ = 2,  /* only  page|inquiry BIT1 */
    HAL_BTCOEX_PS_STATUE_PAGE_ACL = 3,  /* both a2dp|数传 and page|inquiry BIT0|BIT1 */
    HAL_BTCOEX_PS_STATUE_LDAC = 4,      /* only ldac BIT2 */
    HAL_BTCOEX_PS_STATUE_LDAC_ACL = 5,  /* ldac and a2dp|数传 BIT2|BIT0 */
    HAL_BTCOEX_PS_STATUE_LDAC_PAGE = 6, /* ldac and page|inquiry BIT2|BIT1 */
    HAL_BTCOEX_PS_STATUE_TRIPLE = 7,    /* ldac and page|inquiry and a2dp|数传 BIT2|BIT1|BIT0 */

    HAL_BTCOEX_PS_STATUE_BUTT
} hal_btcoex_ps_status_enum;
typedef oal_uint8 hal_btcoex_ps_status_enum_uint8;

/* mode常见类型，后续根据测试需要补充 */
typedef enum {
    HAL_BTCOEX_SW_POWSAVE_MODE_0 = 0,
    HAL_BTCOEX_SW_POWSAVE_MODE_1 = 1,
    HAL_BTCOEX_SW_POWSAVE_MODE_NORMAL = 2,  // 删聚合打开  BIT1(02方案)
    HAL_BTCOEX_SW_POWSAVE_MODE_3 = 3,
    HAL_BTCOEX_SW_POWSAVE_MODE_4 = 4,
    HAL_BTCOEX_SW_POWSAVE_MODE_5 = 5,
    HAL_BTCOEX_SW_POWSAVE_MODE_6 = 6,
    HAL_BTCOEX_SW_POWSAVE_MODE_7 = 7,
    HAL_BTCOEX_SW_POWSAVE_MODE_8 = 8,
    HAL_BTCOEX_SW_POWSAVE_MODE_9 = 9,
    HAL_BTCOEX_SW_POWSAVE_MODE_10 = 10,
    HAL_BTCOEX_SW_POWSAVE_MODE_11 = 11,
    HAL_BTCOEX_SW_POWSAVE_MODE_12 = 12,
    HAL_BTCOEX_SW_POWSAVE_MODE_13 = 13,
    HAL_BTCOEX_SW_POWSAVE_MODE_14 = 14,
    HAL_BTCOEX_SW_POWSAVE_MODE_15 = 15,

    HAL_BTCOEX_SW_POWSAVE_MODE_BUTT
} hal_coex_sw_preempt_mode;
typedef oal_uint8 hal_coex_sw_preempt_mode_uint8;

typedef enum {
    HAL_BTCOEX_SW_POWSAVE_IDLE = 0,
    HAL_BTCOEX_SW_POWSAVE_WORK = 1,
    HAL_BTCOEX_SW_POWSAVE_TIMEOUT = 2,
    HAL_BTCOEX_SW_POWSAVE_SCAN = 3,
    HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN = 4,
    HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT = 5,
    HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT = 6,
    HAL_BTCOEX_SW_POWSAVE_SCAN_END = 7,
    HAL_BTCOEX_SW_POWSAVE_PSM_START = 8,
    HAL_BTCOEX_SW_POWSAVE_PSM_END = 9,
    HAL_BTCOEX_SW_POWSAVE_PSM_STOP = 10,

    HAL_BTCOEX_SW_POWSAVE_BUTT
} hal_coex_sw_preempt_type;
typedef oal_uint8 hal_coex_sw_preempt_type_uint8;

typedef enum {
    HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE = 0,
    HAL_BTCOEX_SW_POWSAVE_SUB_IDLE = 1,
    HAL_BTCOEX_SW_POWSAVE_SUB_SCAN = 2,
    HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT = 3,
    /* 低功耗唤醒时，连续出现多次ps=1状态，要禁止psm 做状态判断，直到soc中断来更新 */
    HAL_BTCOEX_SW_POWSAVE_SUB_PSM_FORBIT = 4,

    HAL_BTCOEX_SW_POWSAVE_SUB_BUTT
} hal_coex_sw_preempt_subtype_enum;
typedef oal_uint8 hal_coex_sw_preempt_subtype_uint8;

typedef enum {
    HAL_BTCOEX_ONEPKT_NORMAL = 0,    // 普通优先级
    HAL_BTCOEX_ONEPKT_PRIORITY = 1,  // priority
    HAL_BTCOEX_ONEPKT_OCCUPIED = 2,  // OCCUPIED
    HAL_BTCOEX_PRIORITY_BUTT
} hal_coex_priority_type_enum;
typedef oal_uint8 hal_coex_priority_type_uint8;

/* sco 电话mode类型 */
typedef enum {
    HAL_BTCOEX_SCO_MODE_NONE = 0,   /* 电话结束 */
    HAL_BTCOEX_SCO_MODE_12SLOT = 1, /* 12slot电话 */
    HAL_BTCOEX_SCO_MODE_6SLOT = 2,  /* 6slot电话 */

    HAL_BTCOEX_SCO_MODE_BUTT
} hal_btcoex_sco_mode_enum;
typedef oal_uint8 hal_btcoex_sco_mode_enum_uint8;

typedef enum {
    HAL_BTCOEX_AGGR_TIME_OTHER,
    HAL_BTCOEX_AGGR_TIME_TRANSFER,
    HAL_BTCOEX_AGGR_TIME_12SLOT,
    HAL_BTCOEX_AGGR_TIME_6SLOT,
    HAL_BTCOEX_AGGR_TIME_BLE_HID,
    HAL_BTCOEX_AGGR_TIME_SCO_SHORT,
    HAL_BTCOEX_AGGR_TIME_BUTT
} hal_btcoex_aggr_tiyme_type;
typedef oal_uint8 hal_btcoex_aggr_time_type_uint8;
#endif
/* NR共存涉及的枚举值 */
#ifdef _PRE_WLAN_FEATURE_NRCOEX
/* 上层仲裁模块下发的WiFi优先级的消息是从0开始 */
typedef enum {
    HAL_NRCOEX_PRIORITY_HIGH = 0,
    HAL_NRCOEX_PRIORITY_LOW = 1,
    HAL_NRCOEX_PRIORITY_SAME = 2,
    HAL_NRCOEX_PRIORITY_BUTT
} hal_nrcoex_priority_enum;
/* 发送给MODEM的WiFi优先级的值是从1开始 */
typedef enum {
    HAL_MSG_WIFI_PRIORITY_HIGH = 1,
    HAL_MSG_WIFI_PRIORITY_LOW = 2,
    HAL_MSG_WIFI_PRIORITY_SAME = 3,
    HAL_MSG_WIFI_PRIORITY_BUTT
} hal_msg_wifi_priority_enum;
typedef oal_uint8 hal_nrcoex_priority_enum_uint8;
typedef enum {
    WLAN_NRCOEX_CMD_DISPLAY_INFO,
    WLAN_NRCOEX_CMD_NRCOEX_EVENT,
    WLAN_NRCOEX_CMD_MODEM_FREQ,
    WLAN_NRCOEX_CMD_MODEM_BW,
    WLAN_NRCOEX_CMD_BUTT
} wlan_nrcoex_cmd_enum;
typedef oal_uint8 wlan_nrcoex_cmd_enum_uint8;

typedef enum {
    HAL_NRCOEX_MODEM_CHAIN_RFIC0_TX0 = 0,
    HAL_NRCOEX_MODEM_CHAIN_RFIC0_TX1 = 1,
    HAL_NRCOEX_MODEM_CHAIN_RFIC1_TX0 = 2,
    HAL_NRCOEX_MODEM_CHAIN_RFIC1_TX1 = 3,
    HAL_NRCOEX_MODEM_CHAIN_RFIC0_TX01 = 4,
    HAL_NRCOEX_MODEM_CHAIN_RFIC1_TX01 = 5,

    HAL_NRCOEX_MODEM_CHAIN_BUTT
} hal_nrcoex_modem_chain_enum;
typedef oal_uint8 hal_nrcoex_modem_chain_enum_uint8;

typedef enum {
    HAL_NRCOEX_MODEM_BW_200K = 0,
    HAL_NRCOEX_MODEM_BW_1M2288 = 1,
    HAL_NRCOEX_MODEM_BW_1M28 = 2,
    HAL_NRCOEX_MODEM_BW_1M4 = 3,
    HAL_NRCOEX_MODEM_BW_3M = 4,
    HAL_NRCOEX_MODEM_BW_5M = 5,
    HAL_NRCOEX_MODEM_BW_10M = 6,
    HAL_NRCOEX_MODEM_BW_15M = 7,
    HAL_NRCOEX_MODEM_BW_20M = 8,
    HAL_NRCOEX_MODEM_BW_25M = 9,
    HAL_NRCOEX_MODEM_BW_30M = 10,
    HAL_NRCOEX_MODEM_BW_40M = 11,
    HAL_NRCOEX_MODEM_BW_50M = 12,
    HAL_NRCOEX_MODEM_BW_60M = 13,
    HAL_NRCOEX_MODEM_BW_80M = 14,
    HAL_NRCOEX_MODEM_BW_90M = 15,
    HAL_NRCOEX_MODEM_BW_100M = 16,
    HAL_NRCOEX_MODEM_BW_200M = 17,
    HAL_NRCOEX_MODEM_BW_400M = 18,
    HAL_NRCOEX_MODEM_BW_800M = 19,
    HAL_NRCOEX_MODEM_BW_1G = 20,
    HAL_NRCOEX_MODEM_BW_BUTT
} hal_nrcoex_modem_bw_enum;
typedef oal_uint8 hal_nrcoex_modem_bw_enum_uint8;

typedef enum {
    HAL_NRCOEX_AVOID_FLAG_NORMAL = 0, /* 无干扰，不需要规避 */
    HAL_NRCOEX_AVOID_FLAG_LP = 1,     /* 降1档功率 */
    HAL_NRCOEX_AVOID_FLAG_TB = 2,     /* 暂停发送 */

    HAL_NRCOEX_AVOID_FLAG_BUTT
} hal_nrcoex_avoid_flag_enum;
typedef oal_uint8 hal_nrcoex_avoid_flag_enum_uint8;

#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

typedef enum {
    HAL_DYN_CALI_PDET_ADJUST_INIT = 0,
    HAL_DYN_CALI_PDET_ADJUST_ASCEND,  /* while real_pdet < expect_pdet */
    HAL_DYN_CALI_PDET_ADJUST_DECLINE, /* while real_pdet > expect_pdet */
    HAL_DYN_CALI_PDET_ADJUST_VARIED,
    HAL_DYN_CALI_PDET_ADJUST_BUTT,
} hal_dyn_cali_adj_type_enum;
typedef oal_uint8 hal_dyn_cali_adj_type_enum_uint8;

#endif
// 定制化开关枚举
typedef enum {
    MAC_CUST_OPTIMIZE_TXOP = 0,
    MAC_CUST_OPTIMIZE_CE = 1,
    MAC_CUST_OPTIMIZE_BUTT
} mac_custmize_optmize_feature_enum;

/*****************************************************************************
  4 函数实现
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_uint32 *hal_rx_dscr_get_real_addr(oal_uint32 *pul_rx_dscr)
{
    /* 注意数字2代表hi1102_rx_buffer_addr_stru结构体中的prev指针，移动到next指针位置 */
    if (pul_rx_dscr == OAL_PTR_NULL) {
        return pul_rx_dscr;
    }
    return pul_rx_dscr + 2;
}

OAL_STATIC OAL_INLINE oal_uint32 *hal_rx_dscr_get_sw_addr(oal_uint32 *pul_rx_dscr)
{
    /* 注意数字2代表hi1102_rx_buffer_addr_stru结构体中的prev指针，移动到next指针位置 */
    if (pul_rx_dscr == OAL_PTR_NULL) {
        OAL_IO_PRINT("[file = %s, line = %d], hal_rx_dscr_get_sw_addr, dscr is NULL!\r\n", __FILE__, __LINE__);
        return pul_rx_dscr;
    }
    return pul_rx_dscr - 2;
}

#endif /* end of #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) */

/*****************************************************************************/
/*****************************************************************************/
/* 公共宏定义、枚举、结构体 */
/*****************************************************************************/
/*****************************************************************************/
/* 获取当前帧所使用的协议模式 */
#define HAL_GET_DATA_PROTOCOL_MODE(_val) ((_val) >> 6)

/* 功率表相关参数 */
#define HAL_TPC_11B_RATE_NUM 4 /* 11b速率数目 */
#define HAL_TPC_11G_RATE_NUM 8 /* 11g速率数目 */
#define HAL_TPC_11A_RATE_NUM 8 /* 11a速率数目 */
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
#define HAL_TPC_11AC_20M_NUM 10 /* 11n_11ac_2g速率数目 */
#else
#define HAL_TPC_11AC_20M_NUM 9 /* 11n_11ac_2g速率数目 */
#endif
#define HAL_TPC_11AC_40M_NUM 11 /* 11n_11ac_2g速率数目 */
#define HAL_TPC_11AC_80M_NUM 10 /* 11n_11ac_2g速率数目 */

/* rate-tpccode table中速率个数 */
#define HAL_TPC_RATE_TPC_CODE_TABLE_LEN (HAL_TPC_11B_RATE_NUM + HAL_TPC_11G_RATE_NUM + HAL_TPC_11AC_20M_NUM \
                                                + HAL_TPC_11AC_40M_NUM + HAL_TPC_11AC_80M_NUM)
#define HAL_TPC_RATE_TPC_CODE_TABLE_LEN_2G (HAL_TPC_RATE_TPC_CODE_TABLE_LEN - HAL_TPC_11AC_80M_NUM)
#define HAL_TPC_RATE_TPC_CODE_TABLE_LEN_5G HAL_TPC_RATE_TPC_CODE_TABLE_LEN

#define HAL_POW_CUSTOM_24G_11B_RATE_NUM    2
#define HAL_DBB_SCALING_FOR_MAX_TXPWR_BASE 128

#define CUS_NUM_OF_SAR_PARAMS 8 /* 定制化降SAR参数 5G_BAND1~7 2.4G */

#define NUM_OF_NV_MAX_TXPOWER 47 /* NVRAM中存储的各协议速率最大发射功率参数的个数 */

#define CUS_BASE_PWR_NUM_2G 1
#define CUS_BASE_PWR_NUM_5G 7 /* 5g Base power 7个 band 1~7 */
#define CUS_BASE_POWER_NUM  (CUS_BASE_PWR_NUM_2G + CUS_BASE_PWR_NUM_5G)

#define CUS_SAR_NUM_2G 1
#define CUS_SAR_NUM_5G 7 /* 5g Base power 7个 band 1~7 */
#define CUS_SAR_NUM    (CUS_SAR_NUM_2G + CUS_SAR_NUM_5G)

#define CUS_BT_TXPWR_FREQ_NUM_MAX 8  // BT Tx Power calibration max frequency number
#define CUS_BT_FREQ_NUM           79 // BT frequency number

#define FCC_CE_SIG_TYPE_NUM_2G   3 /* FCC CE 定制化2g 11B+OFDM_20M+OFDM_40M */
#define FCC_CE_CH_NUM_5G_20M     9 /* FCC CE 定制化 5g 20MHz 信道个数 */
#define FCC_CE_CH_NUM_5G_40M     6 /* FCC CE 定制化 5g 40MHz 信道个数 */
#define FCC_CE_CH_NUM_5G_80M     5 /* FCC CE 定制化 5g 80MHz 信道个数 */
#define FCC_CE_CH_NUM_5G_160M    2 /* FCC CE 定制化 5g 160MHz 信道个数 */
#define FCC_CE_EXT_CH_NUM_5G_20M 4 /* FCC CE定制化 次边带5g 20M信道个数 */
#define FCC_CE_EXT_CH_NUM_5G_40M 4 /* FCC CE定制化 次边带5g 40M信道个数 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HAL_REGDOMAIN_FCC = 0,
    HAL_REGDOMAIN_ETSI = 1,
    HAL_REGDOMAIN_JAPAN = 2,
    HAL_REGDOMAIN_COMMON = 3,

    HAL_REGDOMAIN_COUNT
} hal_regdomain_enum;
typedef oal_uint8 hal_regdomain_enum_uint8;

/* 2.4GHz频段: 信道号对应的信道索引值 */
typedef enum {
    HAL_2G_CHANNEL1 = 0,
    HAL_2G_CHANNEL2 = 1,
    HAL_2G_CHANNEL3 = 2,
    HAL_2G_CHANNEL4 = 3,
    HAL_2G_CHANNEL5 = 4,
    HAL_2G_CHANNEL6 = 5,
    HAL_2G_CHANNEL7 = 6,
    HAL_2G_CHANNEL8 = 7,
    HAL_2G_CHANNEL9 = 8,
    HAL_2G_CHANNEL10 = 9,
    HAL_2G_CHANNEL11 = 10,
    HAL_2G_CHANNEL12 = 11,
    HAL_2G_CHANNEL13 = 12,
    HAL_2G_CHANNEL14 = 13,

    HAL_CHANNEL_FREQ_2G_BUTT = 14
} hal_channel_freq_2g_enum;
typedef oal_uint8 hal_channel_freq_2g_enum_uint8;

typedef enum {
    HAL_FCS_PROTECT_TYPE_NONE = 0,  /* NONE */
    HAL_FCS_PROTECT_TYPE_SELF_CTS,  /* SELF CTS */
    HAL_FCS_PROTECT_TYPE_NULL_DATA, /* NULL DATA */
    HAL_FCS_PROTECT_TYPE_QOSNULL,
    HAL_FCS_PROTECT_TYPE_BUTT
} hal_fcs_protect_type_enum;
typedef oal_uint8 hal_fcs_protect_type_enum_uint8;

typedef enum {
    HAL_FCS_PROTECT_COEX_PRI_NORMAL = 0,   /* b00 */
    HAL_FCS_PROTECT_COEX_PRI_PRIORITY = 1, /* b01 */
    HAL_FCS_PROTECT_COEX_PRI_OCCUPIED = 2, /* b10 */

    HAL_FCS_PROTECT_COEX_PRI_BUTT
} hal_fcs_protect_coex_pri_enum;
typedef oal_uint8 hal_fcs_protect_coex_pri_enum_uint8;

typedef enum {
    HAL_FCS_SERVICE_TYPE_DBAC = 0,      /* DBAC业务 */
    HAL_FCS_SERVICE_TYPE_SCAN,          /* 扫描业务 */
    HAL_FCS_SERVICE_TYPE_M2S,           /* m2s切换业务 */
    HAL_FCS_SERVICE_TYPE_BTCOEX_NORMAL, /* btcoex共存业务 */
    HAL_FCS_SERVICE_TYPE_BTCOEX_LDAC,   /* btcoex共存业务 */

    HAL_FCS_PROTECT_NOTIFY_BUTT
} hal_fcs_service_type_enum;
typedef oal_uint8 hal_fcs_service_type_enum_uint8;

typedef enum {
    HAL_FCS_PROTECT_CNT_1 = 1, /* 1 */
    HAL_FCS_PROTECT_CNT_2 = 2, /* 2 */
    HAL_FCS_PROTECT_CNT_3 = 3, /* 3 */
    HAL_FCS_PROTECT_CNT_20 = 20,
    HAL_FCS_PROTECT_CNT_BUTT
} hal_fcs_protect_cnt_enum;
typedef oal_uint8 hal_fcs_protect_cnt_enum_uint8;

typedef enum {
    HAL_OPER_MODE_NORMAL,
    HAL_OPER_MODE_HUT,

    HAL_OPER_MODE_BUTT
} hal_oper_mode_enum;
typedef oal_uint8 hal_oper_mode_enum_uint8;

/* RF测试用，用于指示配置TX描述符字段 */
typedef enum {
    HAL_RF_TEST_DATA_RATE_ZERO,
    HAL_RF_TEST_BAND_WIDTH,
    HAL_RF_TEST_CHAN_CODE,
    HAL_RF_TEST_POWER,
    HAL_RF_TEST_BUTT
} hal_rf_test_sect_enum;
typedef oal_uint8 hal_rf_test_sect_enum_uint8;
/*****************************************************************************
  3.1 队列相关枚举定义
*****************************************************************************/
#define HAL_AC_TO_Q_NUM(_ac) (\
    ((_ac) == WLAN_WME_AC_VO) ? HAL_TX_QUEUE_VO : ((_ac) == WLAN_WME_AC_VI) ? HAL_TX_QUEUE_VI : \
    ((_ac) == WLAN_WME_AC_BK) ? HAL_TX_QUEUE_BK : ((_ac) == WLAN_WME_AC_BE) ? HAL_TX_QUEUE_BE : \
    ((_ac) == WLAN_WME_AC_MGMT) ? HAL_TX_QUEUE_HI : HAL_TX_QUEUE_BK)

#define HAL_Q_NUM_TO_AC(_q) (\
    ((_q) == HAL_TX_QUEUE_VO) ? WLAN_WME_AC_VO : ((_q) == HAL_TX_QUEUE_VI) ? WLAN_WME_AC_VI : \
    ((_q) == HAL_TX_QUEUE_BK) ? WLAN_WME_AC_BK : ((_q) == HAL_TX_QUEUE_BE) ? WLAN_WME_AC_BE : \
    ((_q) == HAL_TX_QUEUE_HI) ? WLAN_WME_AC_MGMT : WLAN_WME_AC_BE)

#define HAL_TX_QUEUE_MGMT HAL_TX_QUEUE_HI /* 0~3代表AC发送队列，4代表管理帧、控制帧发送队列 */

/*****************************************************************************
  3.3 描述符相关枚举定义
*****************************************************************************/
typedef enum {
    HAL_TX_RATE_RANK_0 = 0,
    HAL_TX_RATE_RANK_1,
    HAL_TX_RATE_RANK_2,
    HAL_TX_RATE_RANK_3,

    HAL_TX_RATE_RANK_BUTT
} hal_tx_rate_rank_enum;
typedef oal_uint8 hal_tx_rate_rank_enum_uint8;
typedef enum {
    HAL_DFS_RADAR_TYPE_NULL = 0,
    HAL_DFS_RADAR_TYPE_FCC = 1,
    HAL_DFS_RADAR_TYPE_ETSI = 2,
    HAL_DFS_RADAR_TYPE_MKK = 3,
    HAL_DFS_RADAR_TYPE_KOREA = 4,

    HAL_DFS_RADAR_TYPE_BUTT
} hal_dfs_radar_type_enum;
typedef oal_uint8 hal_dfs_radar_type_enum_uint8;

typedef enum {
    HAL_RX_NEW = 0x0,
    HAL_RX_SUCCESS = 0x1,
    HAL_RX_DUP_DETECTED = 0x2,
    HAL_RX_FCS_ERROR = 0x3,
    HAL_RX_KEY_SEARCH_FAILURE = 0x4,
    HAL_RX_CCMP_MIC_FAILURE = 0x5,
    HAL_RX_ICV_FAILURE = 0x6,
    HAL_RX_TKIP_REPLAY_FAILURE = 0x7,
    HAL_RX_CCMP_REPLAY_FAILURE = 0x8,
    HAL_RX_TKIP_MIC_FAILURE = 0x9,
    HAL_RX_BIP_MIC_FAILURE = 0xA,
    HAL_RX_BIP_REPLAY_FAILURE = 0xB,
    HAL_RX_MUTI_KEY_SEARCH_FAILURE = 0xC /* 组播广播 */
} hal_rx_status_enum;
typedef oal_uint8 hal_rx_status_enum_uint8;

/* 接收描述符队列状态 */
typedef enum {
    HAL_TX_INVALID = 0,                 /* 无效 */
    HAL_TX_SUCC,                        /* 成功 */
    HAL_TX_FAIL,                        /* 发送失败（超过重传限制：接收响应帧超时） */
    HAL_TX_TIMEOUT,                     /* lifetime超时（没法送出去） */
    HAL_TX_RTS_FAIL,                    /* RTS 发送失败（超出重传限制：接收cts超时） */
    HAL_TX_NOT_COMPRASS_BA,             /* 收到的BA是非压缩块确认 */
    HAL_TX_TID_MISMATCH,                /* 收到的BA中TID与发送时填写在描述符中的TID不一致 */
    HAL_TX_KEY_SEARCH_FAIL,             /* Key search failed */
    HAL_TX_AMPDU_MISMATCH,              /* 描述符异常 */
    HAL_TX_PENDING,                     /* 02:没有中断均为pending;03:发送过程中为pending */
    HAL_TX_FAIL_ACK_ERROR,              /* 发送失败（超过重传限制：接收到的响应帧错误） */
    HAL_TX_RTS_FAIL_CTS_ERROR,          /* RTS发送失败（超出重传限制：接收到的CTS错误） */
    HAL_TX_FAIL_ABORT,                  /* 发送失败（因为abort） */
    HAL_TX_FAIL_STATEMACHINE_PHY_ERROR, /* MAC发送该帧异常结束（状态机超时、phy提前结束等原因） */
    HAL_TX_SOFT_PSM_BACK,               /* 软件节能回退 */
    HAL_TX_SOFT_RESERVE,                /* reserved */
} hal_tx_dscr_status_enum;
typedef oal_uint8 hal_tx_status_enum_uint8;

/* 接收描述符队列状态 */
typedef enum {
    HAL_DSCR_QUEUE_INVALID = 0,
    HAL_DSCR_QUEUE_VALID,
    HAL_DSCR_QUEUE_SUSPENDED,

    HAL_DSCR_QUEUE_STATUS_BUTT
} hal_dscr_queue_status_enum;
typedef oal_uint8 hal_dscr_queue_status_enum_uint8;

/* 接收描述符队列号 */
typedef enum {
    HAL_RX_DSCR_NORMAL_PRI_QUEUE = 0,
    HAL_RX_DSCR_HIGH_PRI_QUEUE,
    HAL_RX_DSCR_SMALL_QUEUE,

    HAL_RX_DSCR_QUEUE_ID_BUTT
} hal_rx_dscr_queue_id_enum;
typedef oal_uint8 hal_rx_dscr_queue_id_enum_uint8;

/* TX 描述符 12行 b23:b22 phy工作模式 */
typedef enum {
    WLAN_11B_PHY_PROTOCOL_MODE = 0,         /* 11b CCK */
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE = 1, /* 11g/a OFDM */
    WLAN_HT_PHY_PROTOCOL_MODE = 2,          /* 11n HT */
    WLAN_VHT_PHY_PROTOCOL_MODE = 3,         /* 11ac VHT */

    WLAN_PHY_PROTOCOL_BUTT
} wlan_phy_protocol_enum;
typedef oal_uint8 wlan_phy_protocol_enum_uint8;

/* HAL模块需要抛出的WLAN_DRX事件子类型的定义
 说明:该枚举需要和dmac_wlan_drx_event_sub_type_enum_uint8枚举一一对应 */
typedef enum {
    HAL_WLAN_DRX_EVENT_SUB_TYPE_RX, /* WLAN DRX 流程 */

    HAL_WLAN_DRX_EVENT_SUB_TYPE_BUTT
} hal_wlan_drx_event_sub_type_enum;
typedef oal_uint8 hal_wlan_drx_event_sub_type_enum_uint8;

/* HAL模块需要抛出的WLAN_CRX事件子类型的定义
   说明:该枚举需要和dmac_wlan_crx_event_sub_type_enum_uint8枚举一一对应 */
typedef enum {
    HAL_WLAN_CRX_EVENT_SUB_TYPE_RX, /* WLAN CRX 流程 */

#ifdef _PRE_WLAN_FEATURE_FTM
    HAL_EVENT_DMAC_MISC_FTM_ACK_COMPLETE, /* FTM ACK发送完成中断 */
#endif

    HAL_WLAN_CRX_EVENT_SUB_TYPE_BUTT
} hal_wlan_crx_event_sub_type_enum;
typedef oal_uint8 hal_wlan_crx_event_sub_type_enum_uint8;

typedef enum {
    HAL_TX_COMP_SUB_TYPE_TX,
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    HAL_TX_COMP_SUB_TYPE_AL_TX,
#endif

    HAL_TX_COMP_SUB_TYPE_BUTT
} hal_tx_comp_sub_type_enum;
typedef oal_uint8 hal_tx_comp_sub_type_enum_uint8;

typedef enum {
    HAL_EVENT_TBTT_SUB_TYPE,

    HAL_EVENT_TBTT_SUB_TYPE_BUTT
} hal_event_tbtt_sub_type_enum;
typedef oal_uint8 hal_event_tbtt_sub_type_enum_uint8;

typedef enum {
    HAL_COEX_SW_IRQ_LTE_RX_ASSERT = 0x1,   /* BIT0 */
    HAL_COEX_SW_IRQ_LTE_RX_DEASSERT = 0x2, /* BIT1 */
    HAL_COEX_SW_IRQ_LTE_TX_ASSERT = 0x4,   /* BIT2 */
    HAL_COEX_SW_IRQ_LTE_TX_DEASSERT = 0x8, /* BIT3 */
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV))
    HAL_COEX_SW_IRQ_BT = 0x20, /* 02芯片问题，需要配置为BIT5 */
#else
    HAL_COEX_SW_IRQ_BT = 0x10, /* BIT4 */
#endif

    HAL_COEX_SW_IRQ_TYPE_BUTT
} hal_coex_sw_irq_type_enum;
typedef oal_uint8 hal_coex_sw_irq_type_enum_uint8;

/*****************************************************************************
  3.4 中断相关枚举定义
*****************************************************************************/
/* 3.4.1  芯片错误中断类型 */
typedef enum {
    HAL_EVENT_ERROR_IRQ_SOC_ERROR, /* SOC错误中断事件 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    HAL_EVENT_DMAC_HIGH_PRIO_BTCOEX_PS,
#endif
    HAL_EVENT_ERROR_IRQ_SUB_TYPE_BUTT
} hal_event_error_irq_sub_type_enum;
typedef oal_uint8 hal_event_error_irq_sub_type_enum_uint8;

/* 3.4.2  MAC错误中断类型 (枚举值与错误中断状态寄存器的位一一对应!) */
typedef enum {
    /* 描述符参数配置异常,包括AMPDU长度配置不匹配,AMPDU中MPDU长度超长,sub msdu num错误 */
    HAL_MAC_ERROR_PARA_CFG_ERR = 0,
    HAL_MAC_ERROR_RXBUFF_LEN_TOO_SMALL = 1,       /* 接收非AMSDU帧长大于RxBuff大小异常 */
    HAL_MAC_ERROR_BA_ENTRY_NOT_FOUND = 2,         /* 未找到BA会话表项异常0 */
    HAL_MAC_ERROR_PHY_TRLR_TIME_OUT = 3,          /* PHY_RX_TRAILER超时 */
    HAL_MAC_ERROR_PHY_RX_FIFO_OVERRUN = 4,        /* PHY_RX_FIFO满写异常 */
    HAL_MAC_ERROR_TX_DATAFLOW_BREAK = 5,          /* 发送帧数据断流 */
    HAL_MAC_ERROR_RX_FSM_ST_TIMEOUT = 6,          /* RX_FSM状态机超时 */
    HAL_MAC_ERROR_TX_FSM_ST_TIMEOUT = 7,          /* TX_FSM状态机超时 */
    HAL_MAC_ERROR_RX_HANDLER_ST_TIMEOUT = 8,      /* RX_HANDLER状态机超时 */
    HAL_MAC_ERROR_TX_HANDLER_ST_TIMEOUT = 9,      /* TX_HANDLER状态机超时 */
    HAL_MAC_ERROR_TX_INTR_FIFO_OVERRUN = 10,      /* TX 中断FIFO满写 */
    HAL_MAC_ERROR_RX_INTR_FIFO_OVERRUN = 11,      /* RX中断 FIFO满写 */
    HAL_MAC_ERROR_HIRX_INTR_FIFO_OVERRUN = 12,    /* HIRX中断FIFO满写 */
    HAL_MAC_ERROR_UNEXPECTED_RX_Q_EMPTY = 13,     /* 接收到普通优先级帧但此时RX BUFFER指针为空 */
    HAL_MAC_ERROR_UNEXPECTED_HIRX_Q_EMPTY = 14,   /* 接收到高优先级帧但此时HI RX BUFFER指针为空 */
    HAL_MAC_ERROR_BUS_RLEN_ERR = 15,              /* 总线读请求长度为0异常 */
    HAL_MAC_ERROR_BUS_RADDR_ERR = 16,             /* 总线读请求地址无效异常 */
    HAL_MAC_ERROR_BUS_WLEN_ERR = 17,              /* 总线写请求长度为0异常 */
    HAL_MAC_ERROR_BUS_WADDR_ERR = 18,             /* 总线写请求地址无效异常 */
    HAL_MAC_ERROR_TX_ACBK_Q_OVERRUN = 19,         /* tx acbk队列fifo满写 */
    HAL_MAC_ERROR_TX_ACBE_Q_OVERRUN = 20,         /* tx acbe队列fifo满写 */
    HAL_MAC_ERROR_TX_ACVI_Q_OVERRUN = 21,         /* tx acvi队列fifo满写 */
    HAL_MAC_ERROR_TX_ACVO_Q_OVERRUN = 22,         /* tx acv0队列fifo满写 */
    HAL_MAC_ERROR_TX_HIPRI_Q_OVERRUN = 23,        /* tx hipri队列fifo满写 */
    HAL_MAC_ERROR_MATRIX_CALC_TIMEOUT = 24,       /* matrix计算超时 */
    HAL_MAC_ERROR_CCA_TIME_OUT = 25,              /* cca超时 */
    HAL_MAC_ERROR_DCOL_DATA_OVERLAP = 26,         /* 数采overlap告警 */
    HAL_MAC_ERROR_BEACON_MISS = 27,               /* 连续发送beacon失败 */
    HAL_MAC_ERROR_INTR_FIFO_UNEXPECTED_READ = 28, /* interrupt fifo空读异常 */
    HAL_MAC_ERROR_UNEXPECTED_RX_DESC_ADDR = 29,   /* rx desc地址错误异常 */
    HAL_MAC_ERROR_RX_OVERLAP_ERR = 30,            /* mac没有处理完前一帧,phy又上报了一帧异常 */
    HAL_MAC_ERROR_RESERVED_31 = 31,
    HAL_MAC_ERROR_TX_ACBE_BACKOFF_TIMEOUT = 32,  /* 发送BE队列退避超时 */
    HAL_MAC_ERROR_TX_ACBK_BACKOFF_TIMEOUT = 33,  /* 发送BK队列退避超时 */
    HAL_MAC_ERROR_TX_ACVI_BACKOFF_TIMEOUT = 34,  /* 发送VI队列退避超时 */
    HAL_MAC_ERROR_TX_ACVO_BACKOFF_TIMEOUT = 35,  /* 发送VO队列退避超时 */
    HAL_MAC_ERROR_TX_HIPRI_BACKOFF_TIMEOUT = 36, /* 发送高优先级队列退避超时 */
    HAL_MAC_ERROR_RX_SMALL_Q_EMPTY = 37,         /* 接收普通队列的小包，但是小包队列指针为空 */
    HAL_MAC_ERROR_PARA_CFG_2ERR = 38,            /* 发送描述符中AMPDU中MPDU长度过长 */
    HAL_MAC_ERROR_PARA_CFG_3ERR = 39,            /* 发送描述符中11a，11b，11g发送时，mpdu配置长度超过4095 */
    HAL_MAC_ERROR_EDCA_ST_TIMEOUT = 40,          /* CH_ACC_EDCA_CTRL状态机超时 */

    HAL_MAC_ERROR_TYPE_BUTT
} hal_mac_error_type_enum;
typedef oal_uint8 hal_mac_error_type_enum_uint8;

/* 3.4.3 SOC错误中断类型 (需要在DMAC模块进行处理的error irq的类型定义) */
typedef enum {
    /* SOC错误中断 */
    HAL_SOC_ERROR_BUCK_OCP,     /* PMU BUCK过流中断 */
    HAL_SOC_ERROR_BUCK_SCP,     /* PMU BUCK短路中断 */
    HAL_SOC_ERROR_OCP_RFLDO1,   /* PMU RFLDO1过流中断 */
    HAL_SOC_ERROR_OCP_RFLDO2,   /* PMU RFLDO2过流中断 */
    HAL_SOC_ERROR_OCP_CLDO,     /* PMU CLDO过流中断 */
    HAL_SOC_ERROR_RF_OVER_TEMP, /* RF过热中断 */
    HAL_SOC_ERROR_CMU_UNLOCK,   /* CMU PLL失锁中断 */
    HAL_SOC_ERROR_PCIE_SLV_ERR, /* PCIE总线错误中断 */

    HAL_SOC_ERROR_TYPE_BUTT
} hal_soc_error_type_enum;
typedef oal_uint8 hal_soc_error_type_enum_uint8;

/* DMAC MISC 子事件枚举定义 */
typedef enum {
    HAL_EVENT_DMAC_MISC_CH_STATICS_COMP, /* 信道统计/测量完成中断 */
    HAL_EVENT_DMAC_MISC_RADAR_DETECTED,  /* 检测到雷达信号 */
    HAL_EVENT_DMAC_MISC_DFS_AUTH_CAC,    /* DFS认证CAC测试 */
    HAL_EVENT_DMAC_MISC_DBAC,            /* DBAC */
    HAL_EVENT_DMAC_MISC_MWO_DET,
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    HAL_EVENT_DMAC_BT_A2DP,
    HAL_EVENT_DMAC_BT_SCO,
    HAL_EVENT_DMAC_BT_TRANSFER,
    HAL_EVENT_DMAC_BT_PAGE_SCAN,
    HAL_EVENT_DMAC_BT_INQUIRY,
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    HAL_EVENT_DMAC_P2P_NOA_ABSENT_START,
    HAL_EVENT_DMAC_P2P_NOA_ABSENT_END,
    HAL_EVENT_DMAC_P2P_CTWINDOW_END,
#endif
    HAL_EVENT_DMAC_BEACON_TIMEOUT, /* 等待beacon帧超时 */
    HAL_EVENT_DMAC_CALI_TO_HMAC,   /* 校准数据从dmac抛到hmac */
    HAL_EVENT_DMAC_MISC_WOW_WAKE,

    HAL_EVENT_DMAC_MISC_GREEN_AP, /* Green ap timer */

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    HAL_EVENT_DMAC_MISC_IPC_IRQ, /* IPC中断 */
#endif
    HAL_EVENT_DMAC_MISC_CHR,               /* dmac处理上报的chr事件 */
    HAL_EVENT_ERROR_IRQ_MAC_ERROR,         /* MAC错误中断事件 */
    HAL_EVENT_DMAC_MISC_BTCOEX_LDAC,       /* LDAC事件 */
    HAL_EVENT_DMAC_MISC_NBFH_BACK_HOME,    /* 窄带跳频切回业务信道 */
    HAL_EVENT_DMAC_MISC_NBFH_SWT_BCAST,    /* 窄带跳频切到广播信道发Beacon */
    HAL_EVENT_DMAC_MISC_NBFH_HOME_TO_HOME, /* 窄带从业务信道切另一业务信道 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    HAL_EVENT_DMAC_BTCOEX_STATUS_SYN, /* BT状态变化是同步BT状态到HOST */
    HAL_EVENT_DMAC_BLE_HID,           /* 鼠标事件 */
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    HAL_EVENT_DMAC_MODEM_MSG, /* MODEM发送的消息抛到DMAC去处理 */
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    HAL_EVENT_DMAC_MISC_CHBA_DEEPSLEEP_WAKEUP, /* chba深睡唤醒事件处理 */
    HAL_EVENT_DMAC_MISC_CHBA_TX_BCN_PNF, /* chba beacon/pnf发送事件处理 */
#endif
    HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT
} hal_dmac_misc_sub_type_enum;
typedef oal_uint8 hal_dmac_misc_sub_type_enum_uint8;

/*****************************************************************************
  3.5 复位相关枚举定义
*****************************************************************************/
/****3.5.1  复位事件子类型定义 **********************************************/
typedef enum {
    HAL_RESET_HW_TYPE_ALL = 0,
    HAL_RESET_HW_TYPE_PHY,
    HAL_RESET_HW_TYPE_MAC,
    HAL_RESET_HW_TYPE_RF,
    HAL_RESET_HW_TYPE_MAC_PHY,
    HAL_RESET_HW_TYPE_TCM,
    HAL_RESET_HW_TYPE_CPU,
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    HAL_RESET_HW_TYPE_MAC_TSF,
    HAL_RESET_HW_TYPE_MAC_CRIPTO,
    HAL_RESET_HW_TYPE_MAC_NON_CRIPTO,
    HAL_RESET_HW_TYPE_PHY_RADAR,
#endif
    HAL_RESET_HW_NORMAL_TYPE_PHY,
    HAL_RESET_HW_NORMAL_TYPE_MAC,
    HAL_RESET_HW_TYPE_DUMP_MAC,
    HAL_RESET_HW_TYPE_DUMP_PHY,

    HAL_RESET_HW_TYPE_BUTT
} hal_reset_hw_type_enum;
typedef oal_uint8 hal_reset_hw_type_enum_uint8;

/****3.5.1  复位MAC子模块定义 **********************************************/
typedef enum {
    HAL_RESET_MAC_ALL = 0,
    HAL_RESET_MAC_PA,
    HAL_RESET_MAC_CE,
    HAL_RESET_MAC_TSF,
    HAL_RESET_MAC_DUP,
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_RESET_MAC_LOGIC,
#endif
    HAL_RESET_MAC_BUTT
} hal_reset_mac_submod_enum;
typedef oal_uint8 hal_reset_mac_submod_enum_uint8;

typedef enum {
    HAL_LPM_SOC_BUS_GATING = 0,
    HAL_LPM_SOC_PCIE_RD_BYPASS = 1,
    HAL_LPM_SOC_MEM_PRECHARGE = 2,
    HAL_LPM_SOC_PCIE_L0 = 3,
    HAL_LPM_SOC_PCIE_L1_PM = 4,
    HAL_LPM_SOC_AUTOCG_ALL = 5,
    HAL_LPM_SOC_ADC_FREQ = 6,

    HAL_LPM_SOC_SET_BUTT
} hal_lpm_soc_set_enum;
typedef oal_uint8 hal_lpm_soc_set_enum_uint8;

typedef enum {
    HAL_ALG_ISR_NOTIFY_DBAC,
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_DEV)
    HAL_ALG_ISR_NOTIFY_MWO_DET,
#endif
    HAL_ALG_ISR_NOTIFY_ANTI_INTF,
    HAL_ALG_ISR_NOTIFY_BUTT,
} hal_alg_noify_enum;
typedef oal_uint8 hal_alg_noify_enum_uint8;

typedef enum {
    HAL_ISR_TYPE_TBTT,
    HAL_ISR_TYPE_ONE_PKT,

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_DEV)
    HAL_ISR_TYPE_MWO_DET,
#endif
    HAL_ISR_TYPE_NOA_START,
    HAL_ISR_TYPE_NOA_END,

    HAL_ISR_TYPE_BUTT,
} hal_isr_type_enum;
typedef oal_uint8 hal_isr_type_enum_uint8;

/* 性能测试相关 */
typedef enum {
    HAL_ALWAYS_TX_DISABLE,      /* 禁用常发 */
    HAL_ALWAYS_TX_RF,           /* 保留给RF测试广播报文 */
    HAL_ALWAYS_TX_AMPDU_ENABLE, /* 使能AMPDU聚合包常发 */
    HAL_ALWAYS_TX_MPDU,         /* 使能非聚合包常发 */
    HAL_ALWAYS_TX_BUTT
} hal_device_always_tx_state_enum;
typedef oal_uint8 hal_device_always_tx_enum_uint8;

typedef enum {
    HAL_ALWAYS_RX_DISABLE,      /* 禁用常收 */
    HAL_ALWAYS_RX_RESERVED,     /* 保留给RF测试广播报文 */
    HAL_ALWAYS_RX_AMPDU_ENABLE, /* 使能AMPDU聚合包常收 */
    HAL_ALWAYS_RX_ENABLE,       /* 使能非聚合包常收 */
    HAL_ALWAYS_RX_BUTT
} hal_device_always_rx_state_enum;
typedef oal_uint8 hal_device_always_rx_enum_uint8;

typedef enum {
    WLAN_PHY_RATE_1M = 0,      /* 0000 */
    WLAN_PHY_RATE_2M = 1,      /* 0001 */
    WLAN_PHY_RATE_5HALF_M = 2, /* 0010 */
    WLAN_PHY_RATE_11M = 3,     /* 0011 */

    WLAN_PHY_RATE_48M = 8,  /* 1000 */
    WLAN_PHY_RATE_24M = 9,  /* 1001 */
    WLAN_PHY_RATE_12M = 10, /* 1010 */
    WLAN_PHY_RATE_6M = 11,  /* 1011 */

    WLAN_PHY_RATE_54M = 12, /* 1100 */
    WLAN_PHY_RATE_36M = 13, /* 1101 */
    WLAN_PHY_RATE_18M = 14, /* 1110 */
    WLAN_PHY_RATE_9M = 15,  /* 1111 */

    WLAN_PHY_RATE_BUTT
} wlan_phy_rate_enum;

typedef enum {
    HAL_PHY_AGC_TARGET_ADLUST_DEFAULT = 0,
    HAL_PHY_AGC_TARGET_ADLUST_DE2DB = 1,
    HAL_PHY_AGC_TARGET_ADLUST_DE4DB = 2,

    HAL_PHY_AGC_TARGET_ADLUST_BUTT
} hal_phy_agc_target_adjust_enum;
typedef oal_uint8 hal_phy_agc_target_adjust_enmu_uint8;

/*****************************************************************************
  3.6 加密相关枚举定义
*****************************************************************************/
/****3.6.1  芯片密钥类型定义 ************************************************/
typedef enum {
    HAL_KEY_TYPE_TX_GTK = 0, /* Hi1102:HAL_KEY_TYPE_TX_IGTK */
    HAL_KEY_TYPE_PTK = 1,
    HAL_KEY_TYPE_RX_GTK = 2,
    HAL_KEY_TYPE_RX_GTK2 = 3, /* 02使用 */
    HAL_KEY_TYPE_BUTT
} hal_cipher_key_type_enum;
typedef oal_uint8 hal_cipher_key_type_enum_uint8;

/****3.6.2  芯片加密算法类型对应芯片中的值 **********************************/
typedef enum {
    HAL_WEP40 = 0,
    HAL_TKIP = 1,
    HAL_CCMP = 2,
    HAL_NO_ENCRYP = 3,
    HAL_WEP104 = 4,
    HAL_BIP = 5,
    HAL_GCMP = 6,
    HAL_GCMP_256 = 7,
    HAL_CCMP_256 = 8,
    HAL_BIP_256 = 9,
    HAL_CIPER_PROTOCOL_TYPE_BUTT
} hal_cipher_protocol_type_enum;
typedef oal_uint8 hal_cipher_protocol_type_enum_uint8;

/****3.6.3  芯片填写加密寄存器CE_LUT_CONFIG AP/STA **************************/
typedef enum {
    HAL_AUTH_KEY = 0, /* 表明该设备为认证者 */
    HAL_SUPP_KEY = 1, /* 表明该设备为申请者 */

    HAL_KEY_ORIGIN_BUTT,
} hal_key_origin_enum;
typedef oal_uint8 hal_key_origin_enum_uint8;

/* 扫描状态，通过判断当前扫描的状态，判断多个扫描请求的处理策略以及上报扫描结果的策略 */
typedef enum {
    MAC_SCAN_STATE_IDLE,
    MAC_SCAN_STATE_RUNNING,

    MAC_SCAN_STATE_BUTT
} mac_scan_state_enum;
typedef oal_uint8 mac_scan_state_enum_uint8;

typedef enum {
    HAL_CHR_CALI_ERR_TX_POWER = 0,
    HAL_CHR_CALI_ERR_TIMEOUT,
    HAL_CHR_CALI_ERR_PHASE0_SHIFT,
    HAL_CHR_CALI_ERR_PHASE1_SHIFT,
    HAL_CHR_CALI_ERR_PHASE2_SHIFT,
    HAL_CHR_CALI_ERR_PHASE3_SHIFT,
    HAL_CHR_CALI_ERR_GET_TONE,
    HAL_CHR_CALI_ERR_AUTO_AGC,
    HAL_CHR_CALI_ERR_RX_IQ_GNSS_WORK,
    HAL_CHR_CALI_ERR_TX_IQ_GNSS_WORK,
    HAL_CHR_CALI_ERR_PLL_UPC_2G,
    HAL_CHR_CALI_ERR_SA_REPORT,
    HAL_CHR_CALI_ERR_TXIQ_ILLEGAL_VAL,
    HAL_CHR_CALI_ERR_RXIQ_ILLEGAL_VAL,

    HAL_CHR_CALI_ERR_RC_CODE_ILLEGAL_VAL, /* 02a新增 */
    HAL_CHR_CALI_ERR_RX_DC_REMAIN_ILLEGAL_VAL,
    HAL_CHR_CALI_ERR_PLL_CFG_CHECK,
    HAL_CHR_CALI_ERR_DYN_ADJUST_UPC,
    HAL_CHR_CALI_ERR_PLL_UPC_5G,
    HAL_CHR_CALI_ERR_MATRIX_NULL,
    HAL_CHR_CALI_ERR_FEM_FAIL,
    HAL_CHR_CALI_ERR_RECOVER_FAIL,
    HAL_CHR_CALI_ERR_BUTT
} hal_chr_cali_err_enum;
typedef oal_uint8 hal_chr_cali_err_enum_uint8;

typedef struct tag_hal_chr_cali_err_stru {
    hal_chr_cali_err_enum_uint8 uc_chr_cali_err;
} hal_chr_cali_err_stru;

typedef enum {
    HAL_SUSPEND_BY_NRCOEX = BIT0,
    HAL_SUSPEND_BY_OTHER = BIT1
} hal_tx_suspend_enum;

typedef enum {
    HAL_SEND_BY_BLE = BIT0,
    HAL_SEND_BY_OTHER = BIT1
} hal_onepacket_enum;
#define HAL_SEND_BY_NULL 0
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    oal_int32 l_pow_par2; /* 二次项系数 */
    oal_int32 l_pow_par1; /* 一次 */
    oal_int32 l_pow_par0; /* 常数项 */
} wlan_cus_pwr_fit_para_stru;

typedef struct hal_pwr_efuse_amend_stru {
    oal_int16 s_efuse_gain; /* pdbuf-VGA Gain */
    oal_int16 s_efuse_dc;   /* pdbuf-VGA offset */
} hal_pwr_efuse_amend_stru;

typedef struct {
    hal_fcs_protect_type_enum_uint8 en_protect_type;
    hal_fcs_protect_cnt_enum_uint8 en_protect_cnt;

    oal_uint16 us_wait_timeout; /* 软件定时器超时时间 */
    oal_uint32 ul_tx_mode;
    oal_uint32 ul_tx_data_rate;
    oal_uint16 us_duration; /* 单位 us */
    oal_uint16 us_timeout;
    oal_uint8 *puc_protect_frame;
} hal_one_packet_cfg_stru;

typedef struct {
    oal_bool_enum_uint8 en_mac_in_one_pkt_mode : 1;
    oal_bool_enum_uint8 en_self_cts_success : 1;
    oal_bool_enum_uint8 en_null_data_success : 1;
    oal_bool_enum_uint8 ul_resv : 5;
} hal_one_packet_status_stru;

typedef struct {
    oal_uint8 uc_pn_tid;      /* tid,0~7, 对rx pn lut有效 */
    oal_uint8 uc_pn_peer_idx; /* 对端peer索引,0~31 */
    oal_uint8 uc_pn_key_type; /* 1151 0:multicast,1:unicast */
    /* 1102 tx pn: 0x0：GTK(multicast) 0x1：PTK(unicast) 0x2：IGTK others：reserved */
    /* 1102 rx pn: 0x0：组播/广播数据帧 0x1：单播qos数据帧 0x2：单播非qos数据帧
                                         0x3：单播管理帧  0x4：组播/广播管理帧 others：保留 */
    oal_uint8 uc_all_tid; /* 0:仅配置TID,1:所有TID 对rx pn lut有效 */
    oal_uint32 ul_pn_msb; /* pn值的高32位,写操作时做入参，读操作时做返回值 */
    oal_uint32 ul_pn_lsb; /* pn值的低32位，写操作时做入参，读操作时做返回值 */
} hal_pn_lut_cfg_stru;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct {
    oal_uint16 bit_bt_on : 1,
               bit_bt_cali : 1,
               bit_bt_ps : 1,
               bit_bt_inquiry : 1,
               bit_bt_page : 1,
               bit_bt_acl : 1,
               bit_bt_a2dp : 1,
               bit_bt_sco : 1,
               bit_bt_data_trans : 1,
               bit_bt_acl_num : 3,
               bit_bt_link_role : 4;
} bt_status_stru;

typedef union {
    oal_uint16 us_bt_status_reg;
    bt_status_stru st_bt_status;
} btcoex_bt_status_union;

typedef struct {
    oal_uint16 bit_ble_on : 1,
               bit_ble_scan : 1,
               bit_ble_con : 1,
               bit_ble_adv : 1,
               bit_bt_transfer : 1, /* not use (only wifi self) */
               bit_bt_6slot : 2,
               bit_ble_init : 1,
               bit_bt_acl : 1,
               bit_bt_ldac : 1,
               bit_resv1 : 1,
               bit_bt_hid : 1,
               bit_ble_hid : 1,
               bit_ble_scan_flag : 1,  // 当前02A不支持20db,将此标记位复用为鼠标的标记位
               bit_sco_notify : 1,
               bit_bt_ba : 1;
} ble_status_stru;

typedef union {
    oal_uint16 us_ble_status_reg;
    ble_status_stru st_ble_status;
} btcoex_ble_status_union;

typedef struct hal_btcoex_btble_status {
    btcoex_bt_status_union un_bt_status;
    btcoex_ble_status_union un_ble_status;
} hal_btcoex_btble_status_stru;

typedef struct {
    oal_uint16 bit_freq : 1,
               bit_channel_num : 5,
               bit_band : 3,
               bit_join_state : 1,
               bit_key_frame : 3,
               bit_vap_state : 1,
               bit_p2p_scan_state : 1,
               bit_p2p_conn_state : 1;
} wifi_status0_stru;

typedef union {
    oal_uint16 us_wifi_status0_reg;
    wifi_status0_stru st_wifi_status0;
} btcoex_wifi_status0_union;

typedef struct {
    oal_uint16 bit_wifi_on : 1,
               bit_scan_state : 1,
               bit_connecting_state : 1,
               bit_connect_state : 1,
               bit_low_rate : 2,
               bit_low_power_state : 1,
               bit_ps_state : 1,
               bit_resv : 8;
} wifi_status1_stru;

typedef union {
    oal_uint16 us_wifi_status1_reg;
    wifi_status1_stru st_wifi_status1;
} btcoex_wifi_status1_union;

typedef struct {
    btcoex_wifi_status0_union un_wifi_status0;
    btcoex_wifi_status1_union un_wifi_status1;
} btcoex_wifi_status_stru;
typedef struct {
    oal_uint32 ul_abort_start_cnt;
    oal_uint32 ul_abort_done_cnt;
    oal_uint32 ul_abort_end_cnt;
    oal_uint32 ul_preempt_cnt;
    oal_uint32 ul_post_preempt_cnt;
    oal_uint32 ul_post_premmpt_fail_cnt;
    oal_uint32 ul_abort_duration_on;
    oal_uint32 ul_abort_duration_start_us;
    oal_uint32 ul_abort_duration_us;
    oal_uint32 ul_abort_duration_s;
} hal_btcoex_statistics_stru;
#endif
typedef struct {
    oal_uint32 bit_nrcoex_machw_tx_suspend : 1,
               bit_nrcoex_resume_machw_tx : 1,
               bit_nrcoex_disable_ack_trans : 1,
               bit_nrcoex_enable_ack_trans : 1,
               bit_nrcoex_disable_cts_trans : 1,
               bit_nrcoex_enable_cts_trans : 1,
               bit_resv : 26;
} hal_suspend_stru;

/* nr共存涉及到的device和host都需要用到的结构体 */
#ifdef _PRE_WLAN_FEATURE_NRCOEX
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#pragma pack(1)
#endif
typedef struct {
    oal_uint16 us_relative_freq_gap0;  // WiFi中心频点和modem边沿距离的第0档门限
    oal_uint16 us_relative_freq_gap1;  // WiFi中心频点和modem边沿距离的第1档门限
    oal_uint16 us_relative_freq_gap2;  // WiFi中心频点和modem边沿距离的第2档门限
    oal_uint8 uc_limit_power_gear0;    // WiFi采用降功率规避方案时第0档功率值
    oal_uint8 uc_limit_power_gear1;    // WiFi采用降功率规避方案时第1档功率值
    oal_uint8 uc_limit_power_gear2;    // WiFi采用降功率规避方案时第2档功率值
    oal_int8 c_rssi_th;                // WiFi需要modem采取避让措施的rssi门限
    oal_uint8 auc_res[2];
} __OAL_DECLARE_PACKED wlan_nrcoex_threshold_stru;

typedef struct {
    oal_uint16 us_high_freq_th;                                         // NR共存功能生效时的WiFi中心频点上限
    oal_uint16 us_low_freq_th;                                          // NR共存功能生效时的WiFi中心频点下限
    wlan_nrcoex_threshold_stru ast_threshold_params[WLAN_BW_CAP_BUTT];  // 记录不同带宽下的频率和rssi门限值
} __OAL_DECLARE_PACKED wlan_nrcoex_rule_stru;

typedef struct {
    oal_uint8 en_wifi_nrcoex_switch;  // NR共存功能开关
    oal_uint8 auc_res[3];
    wlan_nrcoex_rule_stru ast_nrcoex_rule[HAL_NRCOEX_RULE_NUM];  // 记录不同带宽下的频率和rssi门限值
} __OAL_DECLARE_PACKED wlan_nrcoex_ini_stru;
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#pragma pack()
#endif

typedef struct {
    oal_uint16 us_wifi_center_freq;                      /* WiFi中心频率 */
    oal_uint16 us_modem_center_freq;                     /* Modem中心频率 */
    wlan_bw_cap_enum_uint8 en_wifi_bw;                   /* WiFi带宽 */
    hal_nrcoex_modem_bw_enum_uint8 en_modem_bw;          /* Modem带宽 */
    hal_nrcoex_avoid_flag_enum_uint8 en_wifi_avoid_flag; /* WiFi当前规避手段 */
    hal_nrcoex_priority_enum_uint8 en_wifi_priority;     /* WiFi优先级 */
} wlan_nrcoex_info_stru;

typedef oal_void (*p_nrcoex_set_tx_pow_cb)(oal_void);

typedef struct {
    oal_uint8 uc_cur_limit_power;                          // 降功率规避方法所指定的功率值,默认为0xff
    oal_uint8 uc_lp_power;                                 // LP模式下功率限定值
    oal_bool_enum_uint8 en_modem_nrcoex_switch;            // 标示modem模块NR共存功能是否打开
    oal_bool_enum_uint8 en_wifi_status;                    // WiFi是否打开的标示
    wlan_nrcoex_rule_stru *pst_cur_nrcoex_rule;            // 当前NR共存所使用的INI配置参数
    frw_timeout_stru st_key_frame_protect_timer;           // 关键帧保护定时器
    oal_uint32 ul_key_frame_timestamp;                     // 关键帧保护时间戳
    oal_uint16 us_key_frame_protect_time;                  // 关键帧保护时间统计
    oal_bool_enum_uint8 en_key_frame_protect_flag;         // 关键帧保护状态标示
    hal_nrcoex_modem_chain_enum_uint8 en_modem_chain;      // modem通道信息
    oal_uint16 us_wifi_center_freq;                        // WiFi中心频点
    oal_uint16 us_modem_center_freq;                       // modem中心频点
    wlan_bw_cap_enum_uint8 en_wifi_bw;                     // WiFi带宽
    hal_nrcoex_modem_bw_enum_uint8 en_modem_bw;            // modem带宽
    hal_nrcoex_priority_enum_uint8 en_wifi_priority;       // WiFi优先级
    hal_nrcoex_priority_enum_uint8 en_modem_priority;      // modem优先级
    hal_nrcoex_avoid_flag_enum_uint8 en_wifi_avoid_flag;   // WiFi采取的规避方案
    hal_nrcoex_avoid_flag_enum_uint8 en_modem_avoid_flag;  // modem采取的规避方案
    oal_bool_enum_uint8 en_rssi_high;                      // 当前rssi是否高于NR共存的rssi门限标
    oal_uint8 uc_rssi_valid_cnt;                           // 满足条件的rssi次数
    oal_bool_enum_uint8 en_dbac_status;                    // dbac状态
    oal_bool_enum_uint8 en_clear_freq_bw;                  // 在没有关联任何user的时候将频点和带宽设置为0
    oal_bool_enum_uint8 en_is_fsm_attached;
    oal_uint8 auc_res[1];
    oal_fsm_stru st_nrcoex_fsm;
    oal_uint32 ul_rxslot_begin_ts;  // wifi rx slot开始时间
    oal_uint32 ul_rxslot_end_ts;    // wifi rx slot结束时间
    p_nrcoex_set_tx_pow_cb p_nrcoex_set_tx_pow;
} hal_nrcoex_mgr_stru;

typedef struct {
    wlan_nrcoex_cmd_enum_uint8 en_nrcoex_cmd;
    oal_uint8 auc_res[1];
    oal_uint16 us_value;
} wlan_nrcoex_cmd_stru;
#endif

typedef struct hal_wifi_channel_status {
    oal_uint8 uc_chan_number; /* 主20MHz信道号 */
    oal_uint8 uc_band;        /* 频段 */
    oal_uint8 uc_bandwidth;   /* 带宽模式 */
    oal_uint8 uc_idx;         /* 信道索引号 */
} hal_wifi_channel_status_stru;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define CUS_DY_CALI_DPN_PARAMS_NUM 4 /* 定制化动态校准2.4G DPN参数个数11b OFDM20/40 CW OR 5G 160/80/40/20 */
#define CUS_DY_CALI_NUM_5G_BAND    5 /* 动态校准5g band1 2&3 4&5 6 7 */
#define CUS_2G_CHANNEL_NUM         13
#define CUS_DY_CALI_PARAMS_NUM     14  /* 动态校准参数个数,2.4g 3个(ofdm 20/40 11b cw),5g 5*2个band */
#define CUS_DY_CALI_PARAMS_TIMES   3   /* 动态校准参数二次项系数个数 */
#define CUS_DY_CALI_2G_VAL_DPN_MAX 50  /* 动态校准2g dpn读取nvram最大值 */
#define CUS_DY_CALI_2G_VAL_DPN_MIN -50 /* 动态校准2g dpn读取nvram最小值 */
#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
typedef struct {
    oal_int32 al_dy_cali_base_ratio_params[CUS_DY_CALI_PARAMS_NUM][CUS_DY_CALI_PARAMS_TIMES]; /* 产测定制化参数数组 */
    oal_int8 ac_dy_cali_2g_dpn_params[CUS_2G_CHANNEL_NUM][CUS_DY_CALI_DPN_PARAMS_NUM];
    oal_int8 ac_dy_cali_5g_dpn_params[CUS_DY_CALI_NUM_5G_BAND][WLAN_BW_CAP_160M];
    oal_int8 c_5g_iq_cali_backoff_pow;                     /* 5G IQ校准回退功率 */
    oal_uint16 aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT]; /* 动态校准开关2.4g 5g */
    oal_int16 as_extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
    oal_int16 s_gm0_dB10;
    oal_int32 al_dy_cali_base_ratio_ppa_params[CUS_DY_CALI_PARAMS_TIMES]; /* ppa-pow定制化参数数组 */
    oal_uint8 uc_tx_power_pdbuf_opt;
    oal_uint8 uc_5g_upc_upper_limit; /* 5G功率校准UPC上限值 */
    oal_uint16 us_5g_iq_cali_pow;    /* 5G IQ校准期望功率，rx iq--高8位, tx iq--低8位 */
    oal_int32 al_bt_power_fit_params[CUS_DY_CALI_PARAMS_TIMES];
    oal_int32 al_bt_ppavdet_fit_params[CUS_DY_CALI_PARAMS_TIMES];
} wlan_cus_dy_cali_param_stru;
#endif

/*****************************************************************************
  7.0 寄存器配置结构
*****************************************************************************/
/*lint -e958*/
#if (_PRE_WLAN_CHIP_VERSION == _PRE_WLAN_CHIP_FPGA_HI1101RF)
struct witp_reg_cfg {
    oal_uint16 us_soft_index;
    oal_uint8 uc_addr;
    oal_uint32 ul_val;
} __OAL_DECLARE_PACKED;
#elif (_PRE_WLAN_CHIP_VERSION == _PRE_WLAN_CHIP_FPGA_HI1151RF) /* End of _PRE_WLAN_CHIP_FPGA_HI1101RF */
struct witp_reg16_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;
typedef struct witp_reg16_cfg witp_reg16_cfg_stru;

struct witp_reg_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;
#elif (_PRE_WLAN_CHIP_VERSION == _PRE_WLAN_CHIP_FPGA)          /* End of _PRE_WLAN_CHIP_FPGA_HI1151RF */
struct witp_reg16_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;
typedef struct witp_reg16_cfg witp_reg16_cfg_stru;

struct witp_reg_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;

#elif (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION) /* End of _PRE_WLAN_CHIP_FPGA_HI1151RF */
struct witp_reg16_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;
typedef struct witp_reg16_cfg witp_reg16_cfg_stru;

struct witp_reg_cfg {
    oal_uint16 us_addr;
    oal_uint16 us_val;
} __OAL_DECLARE_PACKED;
#endif                                                /* End of _PRE_WLAN_CHIP_ASIC */

typedef struct witp_reg_cfg witp_reg_cfg_stru;

struct witp_single_tune_reg_cfg {
    oal_uint16 us_addr;
    oal_int32 ul_val;
} __OAL_DECLARE_PACKED;

typedef struct witp_single_tune_reg_cfg witp_single_tune_reg_cfg_stru;

/*lint +e958*/
/*****************************************************************************
  7.1 基准发送描述符定义
*****************************************************************************/
typedef struct tag_hal_tx_dscr_stru {
    oal_dlist_head_stru st_entry;
    oal_netbuf_stru *pst_skb_start_addr;   /* Sub MSDU 0 Skb Address */
    oal_uint16 us_original_mpdu_len;       /* mpdu长度 含帧头 */
    hal_tx_queue_type_enum_uint8 uc_q_num; /* 发送队列队列号 */
    oal_uint8 bit_is_retried : 1;          /* 是不是重传包 */
    oal_uint8 bit_is_ampdu : 1;            /* 是不是ampdu */
    oal_uint8 bit_is_rifs : 1;             /* 是不是rifs发送 */
    oal_uint8 bit_is_first : 1;            /* 标志是否是第一个描述符 */
    oal_uint8 bit_resv : 4;
    oal_uint8 data[4];
} hal_tx_dscr_stru;

/*****************************************************************************
  7.2 基准接收描述符定义
*****************************************************************************/
#ifndef _PRE_LINUX_TEST
typedef struct tag_hal_rx_dscr_stru {
    oal_uint32 *pul_prev_rx_dscr; /* 前一个描述符的地址 */
    oal_uint32 ul_skb_start_addr; /* 描述符中保存的netbuf的首地址 */
    oal_uint32 *pul_next_rx_dscr; /* 前一个描述符的地址(物理地址) */
} hal_rx_dscr_stru;
#else
typedef struct tag_hal_rx_dscr_stru {
    oal_uint32 *pul_prev_rx_dscr; /* 前一个描述符的地址 */
    oal_uint   ul_skb_start_addr; /* 描述符中保存的netbuf的首地址 */
    oal_uint32 *pul_next_rx_dscr; /* 前一个描述符的地址(物理地址) */
} hal_rx_dscr_stru;
#endif
/*****************************************************************************
  7.3 对外部发送提供接口所用数据结构
*****************************************************************************/
/*****************************************************************************
  结构名  : hal_channel_matrix_dsc_stru
  结构说明: 矩阵信息结构体
*****************************************************************************/
typedef struct {
    /* (第10 23行) */
    oal_uint8 bit_codebook : 2;
    oal_uint8 bit_grouping : 2;
    oal_uint8 bit_row_num : 4;

    oal_uint8 bit_column_num : 4;
    oal_uint8 bit_response_flag : 1; /* 在Tx 描述符中不用填写该字段;发送完成中断后，将有无信道矩阵信息存储在此 */
    oal_uint8 bit_reserve1 : 3;

    oal_uint16 us_channel_matrix_length; /* 信道矩阵的总字节(Byte)数 */
    oal_uint32 ul_steering_matrix;       /* txbf需要使用的矩阵地址,填写发送描述符时候使用 */
} hal_channel_matrix_dsc_stru;

typedef struct {
    /* PHY TX MODE 1(第13行) */
    /* (1) 速率自适应填写 */
    oal_uint8 uc_extend_spatial_streams;          /* 扩展空间流个数 */
    wlan_channel_code_enum_uint8 en_channel_code; /* 信道编码(BCC或LDPC) */

    /* (2) ACS填写 */
    hal_channel_assemble_enum_uint8 uc_channel_bandwidth; /* 工作带宽 */

    oal_uint8 bit_lsig_txop : 1;
    oal_uint8 bit_reserved : 7;

    oal_uint8 dyn_bandwidth_in_non_ht;
    oal_uint8 dyn_bandwidth_in_non_ht_exist;
    oal_uint8 ch_bandwidth_in_non_ht_exist;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8 bit_lpf_gain_level : 2;
    oal_uint8 bit_upc_gain_level : 4;
    oal_uint8 bit_pa_gain_level : 2;

    oal_uint8 bit_dpd_enable : 1;
    oal_uint8 auc_reserve1 : 7;

    /* 02芯片测试添加抗干扰变量，为tx描述符赋值 */
    oal_uint8 bit_anti_intf_1thr : 2;
    oal_uint8 bit_anti_intf_0thr : 2;
    oal_uint8 bit_anti_intf_en : 1;
    oal_uint8 bit_reserve : 3;
#else
    oal_uint8 bit_upc_gain_level : 1;            /* UPC增益级别 */
    oal_uint8 bit_pa_gain_level : 1;             /* PA增益级别 */
    oal_uint8 bit_micro_tx_power_gain_level : 4; /* Micro Tx power增益级别 */
    oal_uint8 bit_dac_gain_level : 2;            /* DAC增益级别 */

    oal_uint8 auc_reserve1[2];
#endif

    oal_uint8 uc_smoothing; /* 通知接收端是否对信道矩阵做平滑 */

    wlan_sounding_enum_uint8 en_sounding_mode; /* sounding模式 */
} hal_tx_txop_rate_params_stru;

typedef union {
    oal_uint32 ul_value;
    /* (第14 19 20 21行) */
    struct {
        oal_uint8 bit_tx_count : 4; /* 传输次数 */
        oal_uint8 bit_tx_chain_selection : 4; /* 发送通道选择 (单通道:0x1, 双通道:0x3, 三通道:0x7, 四通道:0xf) */
        oal_uint8 uc_tx_data_antenna;         /* 发送数据使用的天线组合 */

        union {
            struct {
                oal_uint8 bit_vht_mcs : 4;
                oal_uint8 bit_nss_mode : 2;      /* 该速率对应的空间流枚举值 */
                oal_uint8 bit_protocol_mode : 2; /* 协议模式 */
            } st_vht_nss_mcs;
            struct {
                oal_uint8 bit_ht_mcs : 6;
                oal_uint8 bit_protocol_mode : 2; /* 协议模式 */
            } st_ht_rate;
            struct {
                oal_uint8 bit_legacy_rate : 4;
                oal_uint8 bit_reserved1 : 2;
                oal_uint8 bit_protocol_mode : 2; /* 协议模式 */
            } st_legacy_rate;
        } un_nss_rate;

        oal_uint8 bit_rts_cts_enable : 1;  /* 是否使能RTS */
        oal_uint8 bit_txbf_mode : 2;       /* txbf模式 */
        oal_uint8 bit_preamble_mode : 1;   /* 前导码 */
        oal_uint8 bit_short_gi_enable : 1; /* 短保护间隔 */
        oal_uint8 bit_reserve : 3;
    } rate_bit_stru;
} hal_tx_txop_per_rate_params_union;

typedef struct {
    /* PHY TX MODE 2 (第15行) */
    oal_uint8 uc_tx_rts_antenna;  /* 发送RTS使用的天线组合 */
    oal_uint8 uc_rx_ctrl_antenna; /* 接收CTS/ACK/BA使用的天线组合 */
    oal_uint8 auc_reserve1[1];    /* TX VAP index 不是算法填写，故在此也填0 */
    oal_uint8 bit_txop_ps_not_allowed : 1;
    oal_uint8 bit_long_nav_enable : 1;
    oal_uint8 bit_group_id : 6;
} hal_tx_txop_antenna_params_stru;

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) ||  \
       (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_TEST_MODE != _PRE_TEST_MODE_OFF) ||  \
       (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST) ||  \
       (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))
typedef struct {
    /* TX POWER (第14行) */
    oal_uint8 bit_lpf_gain_level0 : 2;
    oal_uint8 bit_upc_gain_level0 : 4;
    oal_uint8 bit_pa_gain_level0 : 2;

    oal_uint8 bit_lpf_gain_level1 : 2;
    oal_uint8 bit_upc_gain_level1 : 4;
    oal_uint8 bit_pa_gain_level1 : 2;

    oal_uint8 bit_lpf_gain_level2 : 2;
    oal_uint8 bit_upc_gain_level2 : 4;
    oal_uint8 bit_pa_gain_level2 : 2;

    oal_uint8 bit_lpf_gain_level3 : 2;
    oal_uint8 bit_upc_gain_level3 : 4;
    oal_uint8 bit_pa_gain_level3 : 2;
} hal_tx_txop_tx_power_stru;
#endif

typedef struct {
    wlan_tx_ack_policy_enum_uint8 en_ack_policy; /* ACK 策略 */
    oal_uint8 uc_tid_no;                         /* 通信标识符 */
    oal_uint8 uc_qos_enable;                     /* 是否开启QoS */
    oal_uint8 auc_resv[1];
} hal_wmm_txop_params_stru;

/* 第12 17行 */
typedef struct {
    oal_uint16 us_tsf_timestamp;
    oal_uint8 uc_mac_hdr_len;
    oal_uint8 uc_num_sub_msdu;
} hal_tx_mpdu_mac_hdr_params_stru;

typedef struct {
    oal_uint32 ul_mac_hdr_start_addr;
    oal_netbuf_stru *pst_skb_start_addr;
} hal_tx_mpdu_address_params_stru;

typedef struct {
    oal_uint8 uc_ra_lut_index;
    oal_uint8 uc_tx_vap_index;
    oal_uint8 auc_resv[2];
} hal_tx_ppdu_addr_index_params_stru;

typedef struct {
    oal_uint32 ul_msdu_addr0;
    oal_uint16 us_msdu0_len;
    oal_uint16 us_msdu1_len;
    oal_uint32 ul_msdu_addr1;
} hal_tx_msdu_address_params;

typedef struct {
    oal_uint8 uc_long_retry;
    oal_uint8 uc_short_retry;
    oal_uint8 uc_rts_succ;
    oal_uint8 uc_cts_failure;
    oal_int8 c_last_ack_rssi;
    oal_uint8 uc_mpdu_num;
    oal_uint8 uc_error_mpdu_num;
    oal_uint8 uc_last_rate_rank;
    oal_uint8 uc_tid;
    hal_tx_queue_type_enum_uint8 uc_ac;
    oal_uint16 us_mpdu_len;
    oal_uint8 uc_is_retried;
    oal_uint8 uc_bandwidth;
    oal_uint8 uc_sounding_mode;      /* 表示该帧sounding类型 */
    oal_uint8 uc_status;             /* 该帧的发送结果 */
    oal_uint8 uc_ampdu_enable;       /* 表示该帧是否为AMPDU聚合帧 */
    oal_uint16 us_origin_mpdu_lenth; /* mpdu长度 */
    oal_uint8 en_channel_code;
    oal_uint64 ull_ampdu_result;
    hal_channel_matrix_dsc_stru st_tx_dsc_chnl_matrix; /* 发送描述符中的信道矩阵信息 */
    hal_tx_txop_per_rate_params_union ast_per_rate[HAL_TX_RATE_MAX_NUM];
    oal_uint32 ul_ampdu_length;
    hal_tx_txop_tx_power_stru st_tx_power;
    oal_uint8 uc_tx_desc_rate_rank; /* 发送成功时选择的速率等级，status=1时有效 */

    oal_uint32 ul_now_time_ms; /* 发送的时间 */
} hal_tx_dscr_ctrl_one_param;

typedef struct {
    /* 由安全特性更新 */
    wlan_security_txop_params_stru *pst_security; /* 第16行 MAC TX MODE 2 */

    /* groupid和partial_aid */
    wlan_groupid_partial_aid_stru st_groupid_partial_aid; /* 第12和15行部分 */
} hal_tx_txop_feature_stru;
/*****************************************************************************
  结构名  : hal_tx_txop_alg_stru
  结构说明: DMAC模块TXOP发送控制结构
*****************************************************************************/
typedef struct {
    /* tx dscr中算法填写的参数 */
    hal_tx_txop_rate_params_stru st_rate;                                /* 第13行(HY TX MODE 1) */
    hal_tx_txop_per_rate_params_union ast_per_rate[HAL_TX_RATE_MAX_NUM]; /* 第14(PHY TX RATA 1) 19 20 21 行 */
    hal_tx_txop_tx_power_stru st_tx_power; /* 第22行(TX POWER) */
} hal_tx_txop_alg_stru;

typedef struct {
    oal_uint8 auc_tpc_code_level[HAL_TPC_POW_LEVEL_NUM];
    oal_int16 as_tpc_gain_level[HAL_TPC_POW_LEVEL_NUM];
} hal_rate_tpc_code_gain_table_stru;

typedef struct {
    hal_rate_tpc_code_gain_table_stru *pst_rate_tpc_table; /* EVM功率表单结构体指针 */
    hal_tx_txop_alg_stru *pst_txop_param;                  /* 用户速率描述符信息结构体 */
    oal_bool_enum_uint8 en_rf_limit_pow;                   /* 是否使能RF limit功率 */
    oal_uint8 auc_res[3];
} hal_user_pow_info_stru;

/*****************************************************************************
  结构名  : hal_tx_ppdu_feature_stru
  结构说明: DMAC模块PPDU发送控制结构
*****************************************************************************/
typedef struct {
    /* 第15 16行 TX VAP index 和 RA LUT Index */
    hal_tx_ppdu_addr_index_params_stru st_ppdu_addr_index;

    /* 第16 17行 */
    oal_uint32 ul_ampdu_length;    /* 不包括null data的ampdu总长度 */
    oal_uint16 us_min_mpdu_length; /* 根据速率查表得到的ampdu最小mpdu的长度 */

    /* 第13行 */
    oal_uint8 uc_ampdu_enable; /* 是否使能AMPDU */

    oal_uint8 uc_rifs_enable; /* rifs模式下发送时，MPDU链最后是否挂一个BAR帧 */
    /* 第12行  MAC TX MODE 1 */
    oal_uint16 us_tsf;
    oal_uint8 en_retry_flag_hw_bypass;
    oal_uint8 en_duration_hw_bypass;
    oal_uint8 en_seq_ctl_hw_bypass;
    oal_uint8 en_timestamp_hw_bypass;
    oal_uint8 en_addba_ssn_hw_bypass;
    oal_uint8 en_tx_pn_hw_bypass;
    oal_uint8 en_long_nav_enable;
    oal_uint8 uc_mpdu_num; /* ampdu中mpdu的个数 */
    oal_uint8 auc_resv[2];
} hal_tx_ppdu_feature_stru;

/*****************************************************************************
  结构名  : hal_tx_mpdu_stru
  结构说明: DMAC模块MPDU发送控制结构
*****************************************************************************/
typedef struct {
    /* 从11MAC帧头中获取 针对MPDU */
    hal_wmm_txop_params_stru st_wmm;
    hal_tx_mpdu_mac_hdr_params_stru st_mpdu_mac_hdr;                      /* 第12 17行(PHY TX MODE 2) */
    hal_tx_mpdu_address_params_stru st_mpdu_addr;                         /* 第18行(MAC TX MODE 2) */
    hal_tx_msdu_address_params ast_msdu_addr[WLAN_DSCR_SUBTABEL_MAX_NUM]; /* 第24,25...行 */
} hal_tx_mpdu_stru;

/* Beacon帧发送参数 */
typedef struct {
    oal_uint32 ul_pkt_ptr;
    oal_uint32 us_pkt_len;
    hal_tx_txop_alg_stru *pst_tx_param;
    oal_uint32 ul_tx_chain_mask;

    // dmac看不到描述符，这两个寄存器赋值放到hal
    // oal_uint32  ul_phy_tx_mode;     /* 同tx描述符 phy tx mode 1 */
    // oal_uint32  ul_tx_data_rate;    /* 同tx描述符 data rate 0 */
} hal_beacon_tx_params_stru;

/*****************************************************************************
  结构名  : hal_security_key_stru
  结构说明: DMAC模块安全密钥配置结构体
*****************************************************************************/
typedef struct {
    oal_uint8 uc_key_id;
    wlan_cipher_key_type_enum_uint8 en_key_type;
    oal_uint8 uc_lut_idx;
    wlan_ciper_protocol_type_enum_uint8 en_cipher_type;
    oal_bool_enum_uint8 en_update_key;
    wlan_key_origin_enum_uint8 en_key_origin;
    oal_uint8 auc_reserve[2];
    oal_uint8 *puc_cipher_key;
    oal_uint8 *puc_mic_key;
} hal_security_key_stru;

/*****************************************************************************
  7.4 基准VAP和Device结构
*****************************************************************************/
typedef struct {
    oal_uint8 auc_resv[4];
} hal_to_dmac_vap_rom_stru;

typedef struct {
    oal_uint32 ul_training_data;
    oal_uint16 us_training_cnt;
    oal_uint8 uc_is_trained;
    oal_uint8 auc_resv[1];
} hal_tbtt_offset_training_hdl_stru;

typedef struct {
    oal_uint32 ul_probe_beacon_rx_cnt;
    oal_uint32 ul_probe_tbtt_cnt;
    oal_uint16 us_inner_tbtt_offset_base;
    oal_uint8 uc_beacon_rx_ratio;
    oal_uint8 uc_best_beacon_rx_ratio;
    oal_uint8 uc_probe_state;
    oal_uint8 uc_probe_suspend;
    oal_int8 i_cur_probe_index;
    oal_int8 i_best_probe_index;
} hal_tbtt_offset_probe_stru;

#define TBTT_OFFSET_PROBE_STEP_US 30
#define TBTT_OFFSET_PROBE_MAX     10 /* 最多增加30*10 = 300us */

#define TBTT_OFFSET_UP_PROBE_STEP    2 /* up probe */
#define TBTT_OFFSET_DOWN_PROBE_STEP  1
#define TBTT_OFFSET_PROBE_ACCETP_DIF 3

#define TBTT_OFFSET_PROBE_CALCURATE_PERIOD 100 /* beacon接收率计算周期 */

/* state define */
#define TBTT_OFFSET_PROBE_STATE_INIT    0
#define TBTT_OFFSET_PROBE_STATE_START   1
#define TBTT_OFFSET_PROBE_STATE_UP_DONE 2
#define TBTT_OFFSET_PROBE_STATE_END     3

#ifdef _PRE_PRODUCT_ID_HI110X_DEV
typedef struct {
    oal_uint16 us_inner_tbtt_offset;
    oal_uint8 auc_resv[2];
    oal_uint32 ul_rf_on_time;    // 打开前端后的时间戳 hi1102_pm_enable_front_end最后更新
    oal_uint32 ul_tbtt_bh_time;  // tbtt中断下半部的时间戳
    oal_uint32 ul_tbtt_th_time;  // tbtt中断下半部的时间戳
    /* 唤醒后收beacon的通道，在hal device状态机awake子状态时生效 */
#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
    hal_tbtt_offset_training_hdl_stru st_training_handle;
#endif

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
    hal_tbtt_offset_probe_stru st_tbtt_offset_probe;
#endif

    oal_uint8 _rom[4];
} hal_pm_info_stru;
#endif

typedef struct tag_hal_to_dmac_vap_stru {
    oal_uint8 uc_chip_id;                 /* 芯片ID */
    oal_uint8 uc_device_id;               /* 设备ID */
    oal_uint8 uc_vap_id;                  /* VAP ID */
    wlan_vap_mode_enum_uint8 en_vap_mode; /* VAP工作模式 */
    oal_uint8 uc_mac_vap_id;              /* 保存mac vap id */
    oal_uint8 uc_dtim_cnt;                /* dtim count */
    oal_uint16 us_reg_pow;                /* 单位0.1db */
    oal_uint8 uc_service_id;
    wlan_channel_band_enum_uint8 en_freq_band;
    oal_uint8 uc_active_upc_lut_len;
    oal_uint8 auc_pow_cali_upc_code[HAL_MAX_CHAIN_NUM];    /* TX功率校准后的UPC code */
    hal_rate_tpc_code_gain_table_stru *pst_rate_tpc_table; /* 速率-TPC code&TPC gain对应表 */
    oal_int16 as_upc_gain_lut[HAL_MAX_CHAIN_NUM][HAL_TPC_UPC_LUT_NUM];
    oal_uint8 auc_upc_code_lut[HAL_MAX_CHAIN_NUM][HAL_TPC_UPC_LUT_NUM];
#ifdef _PRE_PRODUCT_ID_HI110X_DEV
    hal_pm_info_stru *pst_pm_info;
#endif

    hal_to_dmac_vap_rom_stru *pst_hal_vap_rom;
} hal_to_dmac_vap_stru;

/*****************************************************************************
  7.5 对外部接收提供接口所用数据结构
*****************************************************************************/
typedef struct {
    oal_int8 c_rssi_dbm;
    union {
        struct {
            oal_uint8 bit_vht_mcs : 4;
            oal_uint8 bit_nss_mode : 2;
            oal_uint8 bit_protocol_mode : 2;
        } st_vht_nss_mcs; /* 11ac的速率集定义 */
        struct {
            oal_uint8 bit_ht_mcs : 6;
            oal_uint8 bit_protocol_mode : 2;
        } st_ht_rate; /* 11n的速率集定义 */
        struct {
            oal_uint8 bit_legacy_rate : 4;
            oal_uint8 bit_reserved1 : 2;
            oal_uint8 bit_protocol_mode : 2;
        } st_legacy_rate; /* 11a/b/g的速率集定义 */
    } un_nss_rate;

    oal_uint8 uc_short_gi;
    oal_uint8 uc_bandwidth;
} hal_rx_statistic_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) ||  \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV)
/* 裸系统下，后续需要传输给HMAC模块的信息，与mac_rx_ctl_stru结构体保持一致 */
/* hal_rx_ctl_stru结构的修改要考虑hi1102_rx_get_info_dscr函数中的优化
并且结构修改要和文件dmac_ext_if.h和hmac_ext_if.h 中的修改一致 */
#pragma pack(push, 1)
typedef struct {
    /* word 0 */
    oal_uint8 bit_vap_id : 5;
    oal_uint8 bit_amsdu_enable : 1;
    oal_uint8 bit_is_first_buffer : 1;
    oal_uint8 bit_is_last_buffer : 1;

    oal_uint8 uc_msdu_in_buffer : 6;
    oal_uint8 bit_is_fragmented : 1;
    oal_uint8 bit_has_tcp_ack_info : 1;

    oal_uint8 bit_data_frame_type : 4;
    oal_uint8 bit_ta_user_idx : 4;
    oal_uint8 bit_mac_header_len : 6; /* mac header帧头长度 */
    oal_uint8 bit_is_beacon : 1;
    oal_uint8 bit_is_key_frame : 1;
    /* word 1 */
    oal_uint16 us_frame_len; /* 帧头与帧体的总长度 */
    oal_uint8 uc_mac_vap_id : 4;
    oal_uint8 bit_buff_nums : 4; /* 每个MPDU占用的buf数 */
    oal_uint8 uc_channel_number; /* 接收帧的信道 */
    /* word 2 */
} hal_rx_ctl_stru;
#pragma pack(pop)
#else
/* 后续需要传输给HMAC模块的信息，与mac_rx_ctl_stru结构体保持一致 */
typedef struct {
    /* word 0 */
    oal_uint8 bit_vap_id : 5; /* 对应hal vap id */
    oal_uint8 bit_amsdu_enable : 1;
    oal_uint8 bit_is_first_buffer : 1;
    oal_uint8 bit_is_fragmented : 1;
    oal_uint8 uc_msdu_in_buffer : 6;
    oal_uint8 bit_reserved1 : 2;
    oal_uint8 bit_buff_nums : 4; /* 每个MPDU占用的buf数目 */
    oal_uint8 bit_reserved2 : 4;
    oal_uint8 uc_mac_header_len; /* mac header帧头长度 */
    /* word 1 */
    oal_uint16 us_frame_len;   /* 帧头与帧体的总长度 */
    oal_uint16 us_da_user_idx; /* 目的地址用户索引 */
    /* word 2 */
    oal_uint32 *pul_mac_hdr_start_addr; /* 对应的帧的帧头地址,虚拟地址 */
} hal_rx_ctl_stru;

#endif

/* 对DMAC SCAN 模块提供的硬件MAC/PHY信道测量结果结构体 */
typedef struct {
    /* 信道统计 */
    oal_uint32 ul_ch_stats_time_us;
    oal_uint32 ul_pri20_free_time_us;
    oal_uint32 ul_pri40_free_time_us;
    oal_uint32 ul_pri80_free_time_us;
    oal_uint32 ul_ch_rx_time_us;
    oal_uint32 ul_ch_tx_time_us;

    /* 信道测量 */
    oal_uint8 uc_ch_estimate_time_ms;
    oal_int8 c_pri20_idle_power;
    oal_int8 c_pri40_idle_power;
    oal_int8 c_pri80_idle_power;

    oal_uint32 ul_stats_cnt;
    oal_uint32 ul_meas_cnt;
    oal_uint32 ul_test_cnt;
} hal_ch_statics_irq_event_stru;

/* 对DMAC SCAN模块提供的硬件雷达检测信息结构体 */
typedef struct {
    oal_uint8 uc_radar_type;
    oal_uint8 uc_radar_freq_offset;
    oal_uint8 uc_radar_bw;
    oal_uint8 uc_band;
    oal_uint8 uc_channel_num;
    oal_uint8 uc_working_bw;
    oal_uint8 auc_resv[2];
} hal_radar_det_event_stru;

typedef struct {
    oal_uint32 ul_reg_band_info;
    oal_uint32 ul_reg_bw_info;
    oal_uint32 ul_reg_ch_info;
} hal_radar_irq_reg_list_stru;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) ||  \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV)
/*
 * 裸系统下针对接收，提供读取接口
 * frame_len长度
 * 802.11帧头长度(uc_mac_hdr_len)
 */
#pragma pack(push, 1)

typedef struct {
    /* word 0 */
    oal_uint8 bit_cipher_protocol_type : 4;
    oal_uint8 bit_dscr_status : 4;
    /* word 1 */
    oal_uint8 bit_channel_code : 1;
    oal_uint8 bit_STBC : 2;
    oal_uint8 bit_GI : 1;
    oal_uint8 bit_rsvd : 1;
    oal_uint8 bit_AMPDU : 1;
    oal_uint8 bit_sounding_mode : 2;
    /* word 2 */
    oal_uint8 bit_ext_spatial_streams : 2;
    oal_uint8 bit_smoothing : 1;
    oal_uint8 bit_freq_bandwidth_mode : 4;
    oal_uint8 bit_preabmle : 1;
    // oal_uint16  us_rx_frame_length;     /* ??hcc?? */
    /* word 3 */
    // oal_uint8   bit_amsdu_flag            : 1;              /* AMSDU?? */
    // oal_uint8   bit_buffer_start_flag     : 1;              /* AMSDU?,??MSDU?? */
    // oal_uint8   bit_frag_flag             : 1;              /* MSDU???? */
    oal_uint8 bit_reserved2 : 3;
    oal_uint8 bit_rsp_flag : 1;

    oal_uint8 bit_column_number : 4;

    /* word 4?5 */
    oal_uint16 us_channel_matrix_len;

    /* word 6 */
    oal_uint8 bit_code_book : 2; /* ???????? */
    oal_uint8 bit_grouping : 2;
    oal_uint8 bit_row_number : 4;
} hal_rx_status_stru;
#pragma pack(pop)

#else
/*
 * 针对接收，提供读取接口
 * frame_len长度
 * 802.11帧头长度(uc_mac_hdr_len)
 */
typedef struct {
    /* word 0 */
    oal_uint8 bit_dscr_status : 4;          /* 描述符接收状态 */
    oal_uint8 bit_cipher_protocol_type : 4; /* 帧的加密类型 */
    oal_uint8 bit_ext_spatial_streams : 2;
    oal_uint8 bit_smoothing : 1;
    oal_uint8 bit_freq_bandwidth_mode : 4;
    oal_uint8 bit_preabmle : 1;
    oal_uint8 bit_channel_code : 1;
    oal_uint8 bit_STBC : 2;
    oal_uint8 bit_GI : 1;
    oal_uint8 bit_reserved1 : 1;
    oal_uint8 bit_AMPDU : 1;
    oal_uint8 bit_sounding_mode : 2;
    oal_uint8 uc_reserved1;
    /* word 1 */
    oal_uint8 bit_code_book : 2; /* 信道矩阵相关信息 */
    oal_uint8 bit_grouping : 2;
    oal_uint8 bit_row_number : 4;
    oal_uint8 bit_column_number : 4;
    oal_uint8 bit_rsp_flag : 3;
    oal_uint8 bit_reserved2 : 1;
    oal_uint16 us_channel_matrix_len; /* 信道矩阵长度 */
    /* word 2 */
    oal_uint32 ul_tsf_timestamp; /* TSF时间戳 */
    /* word 3 */
} hal_rx_status_stru;
#endif
/*
 * 针对接收，提供读取接口
 *
 */
#ifndef _PRE_LINUX_TEST
typedef struct {
    oal_uint32 ul_skb_start_addr;
} hal_rx_addr_stru;

/* 针对发送，提供设置接口 */
typedef struct {
    oal_uint32 ul_mac_hdr_start_addr;
    oal_uint32 ul_skb_start_addr;
} hal_rx_ctrl_stru;
#else
typedef struct {
    oal_uint ul_skb_start_addr;
} hal_rx_addr_stru;

/* 针对发送，提供设置接口 */
typedef struct {
    oal_uint ul_mac_hdr_start_addr;
    oal_uint ul_skb_start_addr;
} hal_rx_ctrl_stru;
#endif
/*****************************************************************************
  7.6 对外部保留的VAP级接口列表，建议外部不做直接调用，而是调用对应的内联函数
*****************************************************************************/
typedef struct {
    hal_to_dmac_vap_stru st_vap_base;
    oal_uint32 ul_vap_base_addr;
} hal_vap_stru;

/*****************************************************************************
  结构名  : hal_rx_dscr_queue_header_stru
  结构说明: 接收描述符队列头的结构体
*****************************************************************************/
typedef struct {
    oal_uint32 *pul_element_head;                     /* 指向接收描述符链表的第一个元素 */
    oal_uint32 *pul_element_tail;                     /* 指向接收描述符链表的最后一个元素 */
    oal_uint16 us_element_cnt;                        /* 接收描述符队列中元素的个数 */
    hal_dscr_queue_status_enum_uint8 uc_queue_status; /* 接收描述符队列的状态 */
    oal_uint8 auc_resv[1];
} hal_rx_dscr_queue_header_stru;
/*****************************************************************************
  结构名  : dmac_tx_dscr_queue_header_stru
  结构说明: 发送描述符队列头的结构体
*****************************************************************************/
typedef struct {
    oal_dlist_head_stru st_header;                    /* 发送描述符队列头结点 */
    hal_dscr_queue_status_enum_uint8 en_queue_status; /* 发送描述符队列状态 */
    oal_uint8 uc_ppdu_cnt;                            /* 发送描述符队列中元素的个数 */
    oal_uint8 uc_queue_depth;                         /* 指定发送描述符队列深度 */
    oal_uint8 uc_resv;
} hal_tx_dscr_queue_header_stru;

/*****************************************************************************
  结构名  : dmac_tx_dscr_queue_header_stru
  结构说明: 发送描述符队列头的结构体
*****************************************************************************/
typedef struct {
    oal_uint8 uc_nulldata_awake;    /* AP时收到节能位为0的null data是否唤醒 */
    oal_uint8 uc_nulldata_phy_mode; /* STA时发送null data的phy mode */
    oal_uint8 uc_nulldata_rate;     /* STA时发送null data的速率 */
    oal_uint8 uc_rsv[1];
    oal_uint32 ul_nulldata_interval;      /* STA时发送null data的间隔 */
    oal_uint32 ul_nulldata_address;       /* STA时发送null data的速率 */
    oal_uint32 ul_ap0_probe_resp_address; /* AP0的probe response内存地址 */
    oal_uint32 ul_ap0_probe_resp_len;     /* AP0的probe response长度 */
    oal_uint32 ul_ap1_probe_resp_address; /* AP1的probe response内存地址 */
    oal_uint32 ul_ap1_probe_resp_len;     /* AP1的probe response长度 */
    oal_uint8 uc_ap0_probe_resp_phy;      /* AP0的probe response发送phy模式 */
    oal_uint8 uc_ap0_probe_resp_rate;     /* AP0的probe response发送reate */
    oal_uint8 uc_ap1_probe_resp_phy;      /* AP1的probe response发送phy模式 */
    oal_uint8 uc_ap1_probe_resp_rate;     /* AP1的probe response发送reate */

    oal_uint32 ul_set_bitmap; /* wow开关 */
} hal_wow_param_stru;

typedef enum {
    HAL_WOW_PARA_EN = BIT0,
    HAL_WOW_PARA_NULLDATA = BIT1,
    HAL_WOW_PARA_NULLDATA_INTERVAL = BIT2,
    HAL_WOW_PARA_NULLDATA_AWAKE = BIT3,
    HAL_WOW_PARA_AP0_PROBE_RESP = BIT4,
    HAL_WOW_PARA_AP1_PROBE_RESP = BIT5,

    HAL_WOW_PARA_BUTT
} hal_wow_para_set_enum;
typedef oal_uint32 hal_tx_status_enum_uint32;

/*****************************************************************************
  结构名  : hal_lpm_chip_state
  结构说明: 芯片低功耗状态枚举
*****************************************************************************/
typedef enum {
    HAL_LPM_STATE_POWER_DOWN,
    HAL_LPM_STATE_IDLE,
    HAL_LPM_STATE_LIGHT_SLEEP,
    HAL_LPM_STATE_DEEP_SLEEP,
    HAL_LPM_STATE_NORMAL_WORK,
    HAL_LPM_STATE_WOW,

    HAL_LPM_STATE_BUTT
} hal_lpm_state_enum;
typedef oal_uint8 hal_lpm_state_enum_uint8;

/*****************************************************************************
  结构名  : hal_lpm_state_para
  结构说明: 芯片低功耗状态设置参数
*****************************************************************************/
typedef struct {
    oal_uint8 uc_dtim_count; /* 当前的DTIM count值，STA节能时设置相位 */
    oal_uint8 uc_dtim_period;
    oal_uint8 bit_gpio_sleep_en : 1, /* soc睡眠唤醒的方式,GPIO管脚方式使能 */
              bit_soft_sleep_en : 1,       /* soc睡眠睡眠的方式,软睡眠方式使能 */
              bit_set_bcn_interval : 1,    /* 是否调整beacon inter */
              bit_rsv : 6;
    oal_uint8 uc_rx_chain;           /* 接收通道值 */
    oal_uint32 ul_idle_bcn_interval; /* idle状态下beaon inter */
    oal_uint32 ul_sleep_time;        /* 软定时睡眠时间，单位ms */
} hal_lpm_state_param_stru;

/*****************************************************************************
  结构名  : hal_cfg_rts_tx_param_stru
  结构说明: RTS设置发送参数
*****************************************************************************/
typedef struct {
    wlan_legacy_rate_value_enum_uint8 auc_rate[HAL_TX_RATE_MAX_NUM]; /* 发送速率，单位mpbs */
    wlan_phy_protocol_enum_uint8 auc_protocol_mode[HAL_TX_RATE_MAX_NUM];
    wlan_channel_band_enum_uint8 en_band;
    oal_uint8 auc_recv[3];
} hal_cfg_rts_tx_param_stru;

/*****************************************************************************
  7.7 对外部保留的设备级接口列表，建议外部不做直接调用，而是调用对应的内联函数
*****************************************************************************/
typedef oal_void (*p_hal_alg_isr_func)(oal_uint8 uc_vap_id, oal_void *p_hal_to_dmac_device);
typedef oal_void (*p_hal_gap_isr_func)(oal_uint8 uc_vap_id, oal_void *p_hal_to_dmac_device);

typedef struct {
    oal_uint32 ul_phy_addr;
    oal_uint8 uc_status;
    oal_uint8 uc_q;
    oal_uint8 auc_resv[2];
    oal_uint32 ul_timestamp;

    oal_uint32 ul_arrive_time; /* 下半部到来时间 */
    oal_uint32 ul_handle_time; /* 下半部处理时间 */
    oal_uint32 ul_irq_cnt;
    oal_cpu_usage_stat_stru st_cpu_state;
} hal_rx_dpart_track_stru;

/* 保存硬件上报接收中断信息结构 */
typedef struct {
    oal_dlist_head_stru st_dlist_head;
    oal_uint32 *pul_base_dscr; /* 本次中断描述符地址 */
    oal_uint16 us_dscr_num;    /* 接收到的描述符的个数 */
    // oal_bool_enum_uint8         en_used;
    oal_uint8 uc_channel_num; /* 本次中断时，所在的信道号 */
    oal_uint8 uc_queue_id;
} hal_hw_rx_dscr_info_stru;
typedef struct tag_hal_to_dmac_chip_stru {
    oal_uint8 uc_chip_id;
} hal_to_dmac_chip_stru;

typedef enum {
    HAL_DFR_TIMER_STEP_1 = 0,
    HAL_DFR_TIMER_STEP_2 = 1,
} hal_dfr_timer_step_enum;
typedef oal_uint8 hal_dfr_timer_step_enum_uint8;
typedef struct {
    oal_uint16 us_tbtt_cnt; /* TBTT中断计数，每10次TBTT中断，将us_err_cnt清零 */
    oal_uint16 us_err_cnt;  /* 每10次TBTT中断，产生的MAC错误中断个数 */
} hal_dfr_err_opern_stru;

typedef struct {
    oal_uint32 ul_error1_val; /* 错误1中断状态 */
    oal_uint32 ul_error2_val; /* 错误2中断状态 */
} hal_error_state_stru;

typedef struct {
    oal_dlist_head_stru st_entry;
    oal_uint32 ul_phy_addr; /* 接收描述符物理地址 */
} witp_rx_dscr_recd_stru;

/*****************************************************************************
  结构名  : hal_phy_tpc_param_stru
  结构说明: PHY TPC寄存器参数, 在2.4G/5G频点切换时使用
*****************************************************************************/
typedef struct {
    oal_uint32 ul_pa_bias_addr;                                               /* PA_BIAS地址 */
    oal_uint32 aul_pa_bias_gain_code[WLAN_BAND_BUTT];                         /* 2G/5G PA_BIAS CODE */
    oal_uint32 ul_pa_addr;                                                    /* PA地址 */
    oal_uint32 aul_pa_gain_code[WLAN_BAND_BUTT];                              /* 2G/5G PAIN CODE */
    oal_uint32 aul_2g_upc_addr[WLAN_2G_SUB_BAND_NUM];                         /* 2G UPC地址 */
    oal_uint32 aul_5g_upc_addr[WLAN_5G_SUB_BAND_NUM];                         /* 5G UPC地址 */
    oal_uint32 aul_2g_upc1_data[WLAN_2G_SUB_BAND_NUM][WLAN_UPC_DATA_REG_NUM]; /* 2G 通道1 UPC DATA */
    oal_uint32 aul_5g_upc1_data[WLAN_5G_SUB_BAND_NUM][WLAN_UPC_DATA_REG_NUM]; /* 5G 通道1 UPC DATA */
    oal_uint32 ul_dac_addr;                                                   /* DAC地址 */
    oal_uint32 aul_dac_data[WLAN_BAND_BUTT];                                  /* 2G/5G DAC DATA */
    oal_uint32 ul_lpf_addr;                                                   /* DAC地址 */
    oal_uint32 aul_lpf_data[WLAN_BAND_BUTT];                                  /* 2G/5G LPF DATA */
    oal_uint8 auc_2g_cali_upc_code[HAL_MAX_CHAIN_NUM][WLAN_2G_SUB_BAND_NUM];  /* 2G校准的UPC Code */
    oal_uint8 auc_5g_cali_upc_code[HAL_MAX_CHAIN_NUM][WLAN_5G_CALI_SUB_BAND_NUM]; /* 5G校准的UPC Code(区分20M和80M) */
    oal_uint8 auc_reserve_addr[2];

    /* 不同帧的tpc code */
    oal_uint32 aul_2g_ofdm_ack_cts_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint32 aul_5g_ack_cts_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_2g_dsss_ack_cts_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_rts_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_one_pkt_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_abort_selfcts_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_abort_cfend_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_abort_null_data_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_2g_cfend_tpc_code[WLAN_2G_SUB_BAND_NUM];
    oal_uint8 auc_5g_rts_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_one_pkt_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_abort_cfend_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_cfend_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_ndp_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_vht_report_tpc_code[WLAN_5G_SUB_BAND_NUM];
    oal_uint8 auc_5g_abort_null_data_tpc_code[WLAN_5G_SUB_BAND_NUM];
    /* 读取不同帧格式的速率 */
    oal_uint32 ul_rate_ofdm_ack_cts;
    oal_uint8 uc_rate_rts;
    oal_uint8 uc_rate_one_pkt;
    oal_uint8 uc_rate_abort_selfcts;
    oal_uint8 uc_rate_abort_cfend;
    oal_uint8 uc_rate_cfend;
    oal_uint8 uc_rate_ndp;
    oal_uint8 uc_rate_vht_report;
    oal_uint8 uc_rate_abort_null_data;
} hal_phy_tpc_param_stru;

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
typedef struct {
    oal_uint16 us_max_offset_tsf; /* 最长耗时的时间 */
    oal_uint16 us_mpdu_len;       /* 最长耗时的mpdu长度 */
    oal_uint16 us_tx_excp_cnt;    /* 发送完成耗时异常统计 */
    oal_uint8 uc_q_num;           /* 最长耗时的q_num */
    oal_uint8 auc_resv;
} hal_tx_excp_info_stru;
#endif

typedef struct {
    oal_uint32 ul_tkipccmp_rep_fail_cnt;    /* 重放攻击检测计数TKIP + CCMP */
    oal_uint32 ul_tx_mpdu_cnt;              /* 发送计数非ampdu高优先级 + 普通优先级 + ampdu中mpdu */
    oal_uint32 ul_rx_passed_mpdu_cnt;       /* 属于AMPDU MPDU的FCS正确的MPDU数量 */
    oal_uint32 ul_rx_failed_mpdu_cnt;       /* 属于AMPDU MPDU的FCS错误的MPDU数量 */
    oal_uint32 ul_rx_tkipccmp_mic_fail_cnt; /* kip mic + ccmp mic fail的帧数 */
    oal_uint32 ul_key_search_fail_cnt;      /* 接收key serach fail的帧数 */
    oal_uint32 ul_phy_rx_dotb_ok_frm_cnt;   /* PHY接收dotb ok的帧个数 */
    oal_uint32 ul_phy_rx_htvht_ok_frm_cnt;  /* PHY接收vht ht ok的帧个数 */
    oal_uint32 ul_phy_rx_lega_ok_frm_cnt;   /* PHY接收legace ok的帧个数 */
    oal_uint32 ul_phy_rx_dotb_err_frm_cnt;  /* PHY接收dotb err的帧个数 */
    oal_uint32 ul_phy_rx_htvht_err_frm_cnt; /* PHY接收vht ht err的帧个数 */
    oal_uint32 ul_phy_rx_lega_err_frm_cnt;  /* PHY接收legace err的帧个数 */
} hal_mac_key_statis_info_stru;

/* 会影响目标vdet值的参数集合 */
typedef union {
    struct {
        wlan_channel_band_enum_uint8 en_freq : 4;  // 2G or 5G
        wlan_bw_cap_enum_uint8 en_band_width : 4;
        oal_uint8 uc_channel;
        wlan_mod_enum_uint8 en_mod;
        oal_uint8 uc_tx_pow;
    } st_rf_core_para;
    oal_uint32 ul_para;
} hal_dyn_cali_record_union;

typedef struct {
    oal_err_code_enum_uint32 en_get_tx_power_status;
    hal_dyn_cali_record_union un_record_para;
    oal_int16 s_real_pdet;
    oal_int16 s_pdet_val_i;
    oal_int16 s_pdet_val_q;
    wlan_phy_protocol_enum_uint8 en_cur_protocol;
    oal_uint8 uc_upc_gain_idx;
    oal_bool_enum_uint8 en_get_tx_power_log_flag;
    oal_uint8 auc_resv[3];
} hal_pdet_info_stru;
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
typedef struct {
    hal_dyn_cali_record_union un_record_para;
    oal_int16 s_real_pdet;
    oal_int16 s_exp_pdet;
    oal_uint8 auc_resv[4];
} hal_dyn_cali_usr_record_stru;
#endif

typedef struct hal_dyn_cali_val {
    oal_uint16 aus_cali_en_cnt[WLAN_BAND_BUTT];
    oal_uint16 aus_cali_en_interval[WLAN_BAND_BUTT];   /* 两次动态校准间隔描述符个数 */
    oal_uint8 uc_cali_pdet_min_th;                     /* 动态校准极小阈值 */
    oal_uint8 uc_cali_pdet_max_th;                     /* 动态校准极大阈值 */
    oal_bool_enum_uint8 en_realtime_cali_adjust;       /* 动态校准使能补偿开关 */
    oal_bool_enum_uint8 en_dyn_cali_pdet_need_en_flag; /* 动态校准每帧pdet_en是否要填标志(发送填完描述符后置为false，发送完成恢复true) */
    oal_bool_enum_uint8 en_dyn_cali_complete_flag;     /* 动态校准调整完成标志 */
} hal_dyn_cali_val_stru;

typedef struct {
    hal_dyn_cali_val_stru st_dyn_cali_val;        /* 动态校准N选1参数 */
    frw_timeout_stru st_dyn_cali_per_frame_timer; /* 动态校准每帧的定时器 */
} hal_dmac_dyn_cali_stru;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* ps mode管理结构体 */
typedef struct {
    oal_uint8 bit_ps_on : 1,    /* ps软件机制: 0=关闭, 1=打开 */
              bit_delba_on : 1,       /* 删减聚合逻辑: 0=关闭, 1=打开 */
              bit_reply_cts : 1,      /* 是否回复CTS， 0=不回复， 1=回复 */
              bit_rsp_frame_ps : 1,   /* resp帧节能位是否设置 0=不设置， 1=设置 */
              bit_ps_slot_detect : 1, /* 是否使能动态slot探测功能，delba逻辑触发速率达到标准时打开vap侧 0=不使能， 1=使能 */
              bit_resv : 3;
} hal_coex_sw_preempt_mode_stru;

typedef struct {
    oal_timer_list_stru st_btcoex_ps_slot_timer;
    oal_void *p_drv_arg; /* 中断处理函数参数,对应的pst_dmac_vap */
} hal_btcoex_ps_slot_timer_stru;

typedef struct {
    hal_coex_sw_preempt_mode_stru st_sw_preempt_mode;
    hal_coex_sw_preempt_type_uint8 en_sw_preempt_type;
    hal_coex_sw_preempt_subtype_uint8 en_sw_preempt_subtype;
    hal_fcs_protect_coex_pri_enum_uint8 en_protect_coex_pri; /* one pkt帧发送优先级 */
    oal_bool_enum_uint8 en_ps_occu_down_delay;               /* ps帧occu拉低是否需要有动态slot来拉低 */
    oal_bool_enum_uint8 en_dynamic_slot_pause;               /* 动态slot是否暂停 */
    oal_uint16 us_timeout_ms;                                /* ps超时时间，page扫描190slot 音乐和数传30slot */
    oal_bool_enum_uint8 en_last_acl_status;                  /* 保存上一次acl状态 */
    oal_bool_enum_uint8 en_ps_stop;                          /* 特定业务下，不需要开启ps，通知蓝牙不要发送ps中断 */
    oal_bool_enum_uint8 en_ps_pause;                         /* 特定业务下，需要暂停ps，不影响ps中断处理，防止和wifi特定业务冲突 */
    oal_bool_enum_uint8 en_coex_pri_forbit;                  /* coex pri控制开关，ldac下需要关闭该功能 */
    oal_uint32 ul_ps_cur_time;                               /* 用于ps中断上下半部执行时间统计 */
    oal_atomic ul_ps_event_num;                              /* ps中断event数目 */
    oal_atomic ul_acl_en_cnt;                                /* 连续acl cnt的计数，达到一定次数时，可能是蓝牙长时间为恢复 */
    hal_btcoex_ps_slot_timer_stru st_ps_slot_timer;          /* 用于探测PS帧提前多少slot时间发送的定时器 */
} hal_device_btcoex_sw_preempt_stru;
#endif

struct tag_hal_to_dmac_device_stru;
typedef oal_uint32 (*p_mac_get_work_status_cb)(struct tag_hal_to_dmac_device_stru *pst_hal_device);
typedef oal_uint32 (*p_mac_get_scan_status_cb)(struct tag_hal_to_dmac_device_stru *pst_hal_device);

typedef struct {
    p_mac_get_work_status_cb p_mac_get_work_status;
    p_mac_get_scan_status_cb p_mac_get_scan_status;
} hal_device_cb;

#ifdef _PRE_WLAN_FEATURE_FTM_RAM
typedef struct hal_csi_agc_stru {
    oal_uint8 uc_rpt_lna_code_0ch;
    oal_uint8 uc_rpt_vga_code_0ch;
    oal_uint8 auc_reserv[2];
} hal_csi_agc_stru;
#endif

typedef struct {
    oal_uint16 aus_pending_pkt_cnt[HAL_RX_DSCR_QUEUE_ID_BUTT]; /* 记录pending在乒乓队列里的帧数量 */
} hal_rx_dscr_ctrl_stru;

typedef struct {
    hal_dmac_dyn_cali_stru st_hal_dmac_dyn_cali;

    hal_device_cb st_hal_dev_cb; /* 回调函数 */

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
    oal_uint8 uc_ping_pong_disable;  /* 是否需要减少一次调度,过温保护中添加 */
    oal_uint8 uc_temp_pro_aggr_size; /* 过温度保护中聚合子帧数门限的调整值 */
    oal_uint8 auc_reserv1[2];
#endif
    oal_uint8 uc_tx_power;          /* 配置的最大发射功率限制值,单位为0.1db */
    oal_uint8 uc_btcoex_ps_channel; /* 记录共存场景下PS机制工作信道 */
    oal_uint8 uc_print_all;         /* dmac_tx_pause_info是否打印所有user的开关 */
    oal_uint8 auc_reserv2[1];

#ifdef _PRE_WLAN_FEATURE_FTM_RAM
    hal_csi_agc_stru st_csi_agc;
#endif
    hal_rx_dscr_ctrl_stru st_rx_dscr_ctrl;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    oal_bool_enum_uint8 en_onepkt_occupied;       /* 标示onepkt帧是否拉occupied */
    oal_bool_enum_uint8 en_enterprise_eapol_flag; /* 企业级加密eapol交互标示 */
    hal_btcoex_aggr_time_type_uint8 en_btcoex_aggr_time_type;
    oal_bool_enum_uint8 en_btcoex_disable_auto_freq; /* 标记共存模块是否关闭了自动调频功能 */
    frw_timeout_stru st_onepkt_priority_timer; /* onepkt优先级设置定时器 */
    oal_uint32 ul_occupied_timestamp;          /* 记录上次ARP occpuied配置的时间 */
    oal_uint16 us_btcoex_ps_num; /* 记录统计PS事件的个数 */
    oal_uint16 us_ps_delay_mark; /* 记录PS事件延迟处理的打分结果 */
    oal_uint32 ul_ps_timestamp;  /* 记录PS中断上半部到来的时间 */
#endif
    oal_uint32 ul_filtered_tcp_ack_cnt;
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    wlan_nrcoex_ini_stru st_nrcoex_ini;
    hal_nrcoex_mgr_stru st_nrcoex_mgr;
#endif
    hal_suspend_stru st_suspend_state;
    oal_uint8 uc_tx_suspend_flag;
    oal_uint8 uc_ack_suspend_flag;
    oal_uint8 uc_cts_suspend_flag;
    oal_uint8 uc_onepacket_flag;
    oal_bool_enum_uint8 en_bt_only_on_blacklist;
#ifdef _PRE_WLAN_CHBA_MGMT
    uint8_t chba_mode;
    uint8_t chba_vap_id; /* hal vap id */
    oal_uint8 auc_reserv3[1];
#else
    oal_uint8 auc_reserv3[3]; /* 3个pad */
#endif
} hal_to_dmac_device_rom_stru;

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
#pragma pack(1)
/* scan parameter */
typedef struct {
    oal_uint8 uc_chan_number;             /* 主20MHz信道号 */
    wlan_channel_band_enum_uint8 en_band; /* 频段 */
} gnss_scan_channel_stru;

typedef struct {
    oal_uint8 uc_ch_valid_num;
    gnss_scan_channel_stru ast_wlan_channel[WLAN_MAX_CHANNEL_NUM];
} hal_gscan_params_stru;

/* scan results */
typedef struct {
    oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN]; /* 网络bssid */
    oal_uint8 uc_channel_num;
    oal_int8 c_rssi; /* bss的信号强度 */
    oal_uint8 uc_serving_flag;
    oal_uint8 uc_rtt_unit;
    oal_uint8 uc_rtt_acc;
    oal_uint32 ul_rtt_value;
} wlan_ap_measurement_info_stru;

/* Change Feature: 上报给GNSS的扫描结果结构体不同于DMAC保存的扫描结果结构体 */
typedef struct {
    oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN]; /* 网络bssid */
    oal_uint8 uc_channel_num;
    oal_int8 c_rssi; /* bss的信号强度 */
} wlan_ap_report_info_stru;

/* 上报给gnss的扫描结果结构体 */
typedef struct {
    oal_uint32 ul_interval_from_last_scan;
    oal_uint8 uc_ap_valid_number;
    wlan_ap_report_info_stru ast_wlan_ap_measurement_info[GNSS_SCAN_MAX_AP_NUM_TO_GNSS];
} hal_gscan_report_info_stru;
#pragma pack()

typedef struct {
    oal_dlist_head_stru st_entry;                              /* 链表指针 */
    wlan_ap_measurement_info_stru st_wlan_ap_measurement_info; /* 上报gnss的扫描信息 */
} hal_scanned_bss_info_stru;

typedef struct {
    oal_uint32 ul_scan_end_timstamps; /* 记录此次扫描的时间戳,一次扫描记录一次,不按扫到的ap分别记录 */
    oal_dlist_head_stru st_dmac_scan_info_list;
    oal_dlist_head_stru st_scan_info_res_list; /* 扫描信息存储资源链表 */
    hal_scanned_bss_info_stru ast_scan_bss_info_member[GNSS_SCAN_MAX_AP_NUM_TO_GNSS];
} hal_scan_for_gnss_stru;
#endif

typedef struct tag_hal_to_dmac_device_stru {
    oal_uint8 uc_chip_id;
    oal_uint8 uc_device_id;
    oal_uint8 uc_mac_device_id;             /* 保存mac device id */
    hal_lpm_state_enum_uint8 uc_curr_state; /* 当前芯片的低功耗状态 */
    oal_uint32 ul_core_id;

    hal_dfr_err_opern_stru st_dfr_err_opern[HAL_MAC_ERROR_TYPE_BUTT]; /* 用于MAC异常中断恢复 */

    hal_rx_dscr_queue_header_stru ast_rx_dscr_queue[HAL_RX_QUEUE_NUM];
    hal_tx_dscr_queue_header_stru ast_tx_dscr_queue[HAL_TX_QUEUE_NUM];

#ifdef _PRE_DEBUG_MODE
    /* 记录接收描述符队列中，各个描述符的地址 */
    witp_rx_dscr_recd_stru st_nor_rx_dscr_recd[HAL_NORMAL_RX_MAX_BUFFS];
    witp_rx_dscr_recd_stru st_hi_rx_dscr_recd[HAL_HIGH_RX_MAX_BUFFS];
#endif

    hal_tx_dscr_queue_header_stru ast_tx_dscr_queue_fake[HAL_TX_FAKE_QUEUE_NUM][HAL_TX_QUEUE_NUM];

    oal_uint32 ul_rx_normal_dscr_cnt;

    oal_uint32 ul_track_stop_flag;
    oal_uint8 uc_al_tx_flag;
    hal_regdomain_enum_uint8 en_regdomain_type; /* 标示该国家码的FCC CE属性 */
    oal_uint8 uc_full_phy_freq_user_cnt;        // device下需要满频的vap(ap)/sta(user) 个数
    oal_uint8 uc_over_temp;
    oal_uint32 bit_al_tx_flag : 3;        /* 0: 关闭常发; 1:保留给RF测试; 2: ampdu聚合帧常发; 3:非聚合帧常发 */
    oal_uint32 bit_al_rx_flag : 3;        /* 0: 关闭常收; 1:保留给RF测试；2: ampdu聚合帧常收; 3:非聚合帧常收 */
    oal_uint32 bit_one_packet_st : 1;     /* 0表示DBC结束 1表示DBAC正在执行 */
    oal_uint32 bit_clear_fifo_st : 1;     /* 0表示无clear fifo状态，1表示clear fifo状态 */
    oal_uint32 bit_al_txrx_ampdu_num : 8; /* 指示用于常发常收的聚合帧数目 */
    oal_uint32 bit_track_cnt : 8;         /* 遇到qempty 剩余记录条目数 */
    oal_uint32 bit_track_cnt_down : 8;    /* 剩余记录条目减少标志 */
    oal_netbuf_stru *pst_altx_netbuf;     /* 记录常发时，描述符所共用的内存 */
    oal_netbuf_stru *pst_alrx_netbuf;     /* 记录常收时，描述符所共用的内存 */
    oal_uint32 ul_rx_normal_mdpu_succ_num;
    oal_uint32 ul_rx_ampdu_succ_num;
    oal_uint32 ul_tx_ppdu_succ_num;
    oal_uint32 ul_rx_ppdu_fail_num;
    oal_uint32 ul_tx_ppdu_fail_num;
#ifdef _PRE_DEBUG_MODE
    oal_uint32 ul_dync_txpower_flag;
#endif

    oal_dlist_head_stru ast_rx_isr_info_list[HAL_HW_RX_DSCR_LIST_NUM];
    hal_hw_rx_dscr_info_stru ast_rx_isr_info_member[HAL_HW_RX_ISR_INFO_MAX_NUMS];
    oal_dlist_head_stru st_rx_isr_info_res_list; /* 接收中断信息存储资源链表 */
    oal_uint8 uc_current_rx_list_index;
    oal_uint8 uc_current_chan_number;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    oal_bool_enum_uint8 en_rx_intr_fifo_resume_flag; /* RX INTR FIFO OVERRUN是否恢复标识 */

#ifdef _PRE_DEBUG_MODE
    /* 原始描述符物理地址 */
    oal_uint32 aul_nor_rx_dscr[HAL_NORMAL_RX_MAX_BUFFS];
    oal_uint32 aul_hi_rx_dscr[HAL_HIGH_RX_MAX_BUFFS];

    oal_uint32 ul_dpart_save_idx;
    oal_uint32 ul_rx_irq_loss_cnt;
    hal_rx_dpart_track_stru ast_dpart_track[HAL_DOWM_PART_RX_TRACK_MEM];

    /* 描述符异常还回统计 */
    oal_uint32 ul_exception_free;
    oal_uint32 ul_irq_cnt;

#endif
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    oam_user_track_rx_ampdu_stat st_user_track_rx_ampdu_stat;
#endif
    /* TPC相关PHY参数 */
    hal_phy_tpc_param_stru st_phy_tpc_param;
    oal_int16 s_upc_amend;
    oal_uint8 uc_mag_mcast_frm_power_level; /* 管理帧的功率等级 */
    oal_uint8 uc_control_frm_power_level;   /* 控制帧的功率等级 */

    /* RTS速率相关参数 */
    wlan_legacy_rate_value_enum_uint8 auc_rts_rate[WLAN_BAND_BUTT][HAL_TX_RATE_MAX_NUM]; /* 两个频段的RTS发送速率 */
    wlan_phy_protocol_enum_uint8 auc_rts_protocol[WLAN_BAND_BUTT][HAL_TX_RATE_MAX_NUM];  /* 两个频段的RTS协议模式 */

    /* 字节对齐 */
    p_hal_alg_isr_func pa_hal_alg_isr_func_table[HAL_ISR_TYPE_BUTT][HAL_ALG_ISR_NOTIFY_BUTT];
    p_hal_gap_isr_func pa_hal_gap_isr_func_table[HAL_ISR_TYPE_BUTT];

    oal_uint8 *puc_mac_reset_reg;
    oal_uint8 *puc_phy_reset_reg;
    oal_uint16 uc_cali_check_hw_status; /* FEM&PA失效检测 */
    oal_int16 s_always_rx_rssi;

    hal_wifi_channel_status_stru st_wifi_channel_status;
    oal_uint32 ul_rx_rate;
    oal_int32 l_rx_rssi;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_btcoex_btble_status_stru st_btcoex_btble_status;
    hal_btcoex_statistics_stru st_btcoex_statistics;
    frw_timeout_stru st_btcoex_powersave_timer;
    hal_device_btcoex_sw_preempt_stru st_btcoex_sw_preempt;
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
    oal_uint32 ul_lte_coex_status;
#endif
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
    hal_tx_excp_info_stru st_tx_excp_info;
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    oal_uint64 ull_t1;
    oal_uint64 ull_t4;
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_DEBUG_MODE
    oal_uint32 ul_cfg_loss_tx_comp_cnt; /* 通过配置命令手动丢失发送完成中断 */
#endif
    oal_bool_enum_uint8 en_dfr_enable; /* dfr是否enable */
#endif
    oal_bool_enum_uint8 en_last_over_temperature_flag; /* 上一次查询的过温状态 */
    oal_uint8 uc_fe_print_ctrl;
    oal_uint8 en_last_p2p_mode;
    oal_uint8 auc_resv[1];

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    hal_scan_for_gnss_stru st_scan_for_gnss_info;
    oal_uint8 uc_gscan_mac_vap_id; /* gscan 的扫描vap */
#endif

#ifdef _PRE_WLAN_FEATURE_CSI
    oal_uint8 uc_csi_status;
    oal_uint8 auc_reserv3[3];
#endif

#ifdef _PRE_WLAN_NARROW_BAND
    oal_bool_enum_uint8 en_narrow_bw_open;
    oal_uint8 uc_narrow_bw;
    oal_uint8 uc_narrow_bw_chn_extend; /* 窄带信道扩展 */
    oal_uint8 uc_narrow_chn_num;       /* 窄带实际信道号，取值1~16 */
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    oal_uint32 ul_al_tx_thr;               // 指定常发次数阈值
    oal_uint32 ul_al_tx_num;               // 指定常发次数统计
    oal_bool_enum_uint8 en_al_tx_dyncali;  // 常发下使能动态功率校准,该标志功能作废，对齐03，常发默认进行动态功率校准
    oal_uint8 auc_reserved[3];
#endif
    oal_uint8 uc_dyn_offset_switch;
    wlan_legacy_rate_value_enum_uint8 en_rts_rate;  // rts速率
    wlan_vht_mcs_enum_uint8 en_latest_mcs;          // 上一次数据帧的mcs
#ifdef _PRE_WLAN_FEATURE_AGC_BUGFIX
    hal_phy_agc_target_adjust_enmu_uint8 en_agc_target_adjust_type;  // 是否要调整agc_target
#endif

    /* ROM化后资源扩展指针 */
    hal_to_dmac_device_rom_stru *pst_hal_dev_rom;
} hal_to_dmac_device_stru;

/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    oal_uint32 *pul_base_dscr;       /* 描述符基地址 */
    oal_bool_enum_uint8 en_sync_req; /* 队列同步标识 */
    oal_uint16 us_dscr_num;          /* 接收到的描述符的个数 */
    oal_uint8 uc_queue_id;           /* 接收队列号 */
    hal_to_dmac_device_stru *pst_hal_device;
    oal_uint8 uc_channel_num;
    oal_uint8 auc_resv[3];
} hal_wlan_rx_event_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
/* HAL模块和DMAC模块共用的FTM TIME RX结构体 */
typedef struct {
    oal_uint64 ull_t2;
    oal_uint64 ull_t3;
    oal_uint8 uc_dialog_token;
    oal_uint8 auc_reserve[3];
} hal_wlan_ftm_t2t3_rx_event_stru;
#endif

typedef struct {
    hal_tx_dscr_stru *pst_base_dscr; /* 发送完成中断硬件所上报的描述符指针 */
    hal_to_dmac_device_stru *pst_hal_device;
    oal_uint8 uc_dscr_num; /* 硬件一次发送所发出的描述符个数 */
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    oal_bool_enum_uint8 en_pdet_enable : 1;
    oal_bool_enum_uint8 en_invalid : 1;
    oal_bool_enum_uint8 en_cur_dscr_pdet_en : 1;
    oal_uint8 uc_resv : 5;
    oal_int16 s_pdet_val_i;
    oal_int16 s_pdet_val_q;
    oal_int16 us_resv[1];
#else
    oal_uint8 auc_resv[3];
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    oal_uint64 ull_t1;
    oal_uint64 ull_t4;
#endif
} hal_tx_complete_event_stru;

typedef struct {
    hal_error_state_stru st_error_state;
    hal_to_dmac_device_stru *pst_hal_device;
} hal_error_irq_event_stru;

typedef struct {
    oal_uint8 p2p_noa_status; /* 0: 表示noa 定时器停止，1: 表示noa 定时器正在工作 */
    oal_uint8 auc_resv[3];
} hal_p2p_pm_event_stru;

/* NVRAM 参数结构体 */
typedef struct {
    oal_int8 c_delt_txpower; /* 最大发送功率 */
} hal_cfg_customize_nvram_params_stru;

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#pragma pack(1)
#endif

/* customize rf cfg struct */
typedef struct {
    oal_int8 c_rf_gain_db_mult4;  /* 外部PA/LNA bypass时的增益(0.25dB) */
    oal_int8 c_rf_gain_db_mult10; /* 外部PA/LNA bypass时的增益(0.1dB) */
} __OAL_DECLARE_PACKED hal_cfg_customize_gain_db_per_band;

typedef struct {
    /* 2g */
    hal_cfg_customize_gain_db_per_band ac_gain_db_2g[HAL_DEVICE_2G_BAND_NUM_FOR_LOSS];
    /* 5g */
    hal_cfg_customize_gain_db_per_band ac_gain_db_5g[HAL_DEVICE_5G_BAND_NUM_FOR_LOSS];

    oal_uint16 us_lna_on2off_time_ns_5g; /* LNA开到LNA关的时间(ns) */
    oal_uint16 us_lna_off2on_time_ns_5g; /* LNA关到LNA开的时间(ns) */

    oal_int8 c_rf_line_rx_gain_db_5g;   /* 外部LNA bypass时的增益(dB) */
    oal_int8 c_lna_gain_db_5g;          /* 外部LNA增益(dB) */
    oal_int8 c_rf_line_tx_gain_db_5g;   /* 外部LNA bypass时的增益(dB) */
    oal_uint8 uc_ext_switch_isexist_5g; /* 是否使用外部switch */

    oal_uint8 uc_ext_pa_isexist_5g;                  /* 是否使用外部pa */
    oal_uint8 uc_ext_lna_isexist_5g;                 /* 是否使用外部lna */
    oal_uint8 uc_far_dist_pow_gain_switch;           /* 超远距离功率增益开关 */
    oal_uint8 uc_far_dist_dsss_scale_promote_switch; /* 超远距11b 1m 2m dbb scale提升使能开关 */

    oal_int8 c_delta_cca_ed_high_20th_2g;
    oal_int8 c_delta_cca_ed_high_40th_2g;
    oal_int8 c_delta_cca_ed_high_20th_5g;
    oal_int8 c_delta_cca_ed_high_40th_5g;

    oal_int8 c_delta_pwr_ref_2g_20m; /* pwr ref 2g 20m delta值 */
    oal_int8 c_delta_pwr_ref_2g_40m; /* pwr ref 2g 40m delta值 */
    oal_int8 c_delta_pwr_ref_5g_20m; /* pwr ref 5g 20m delta值 */
    oal_int8 c_delta_pwr_ref_5g_40m; /* pwr ref 5g 40m delta值 */

    oal_int8 c_delta_pwr_ref_5g_80m; /* pwr ref 5g 80m delta值 */
    oal_int8 auc_resv[3];
} __OAL_DECLARE_PACKED wlan_cfg_customize_rf;

/* 定制化 DTS校准参数 */
typedef struct
{
    /* dts */
    /* no used var Begin:: 1102A ASIC no used ini */
    oal_int16 aus_cali_txpwr_pa_dc_ref_2g_val[13]; /* txpwr分信道ref值 */
    oal_int16 us_cali_txpwr_pa_dc_ref_5g_val_band[7];
    /* no used var End:: 1102A ASIC no used ini */
    oal_int8 uc_band_5g_enable;
    oal_uint8 uc_tone_amp_grade;
    oal_uint8 auc_resv_wifi_cali[2];
    /* bt tmp */
    oal_int8 ac_cali_bt_txpwr_dpn_params[CUS_BT_FREQ_NUM];
    oal_int16 s_cali_bt_txpwr_num;
    oal_uint8 auc_cali_bt_txpwr_freq[CUS_BT_TXPWR_FREQ_NUM_MAX];
    oal_uint8 uc_bt_insertion_loss;
    oal_uint8 uc_bt_gm_cali_en;
    oal_int16 s_bt_gm0_dB10;
    oal_uint8 uc_bt_base_power;
    oal_uint8 uc_bt_is_dpn_calc;
} __OAL_DECLARE_PACKED wlan_cus_cali_stru;

/* no used struct Begin:: 1102A ASIC no used ini */
/* RF寄存器定制化结构体 */
typedef struct
{
    oal_uint16 us_rf_reg117;
    oal_uint16 us_rf_reg123;
    oal_uint16 us_rf_reg124;
    oal_uint16 us_rf_reg125;
    oal_uint16 us_rf_reg126;
    oal_uint8 auc_resv[2];
} __OAL_DECLARE_PACKED wlan_cfg_customize_rf_reg;

typedef struct
{
    oal_uint8 uc_temp_pro_enable;            /* 过温保护的使能开关 */
    oal_uint8 uc_temp_pro_reduce_pwr_enable; /* 过温保护过程中是否需要降低功率 */
    oal_int16 us_temp_pro_safe_th;           /* 过温保护的恢复安全水线 */
    oal_int16 us_temp_pro_over_th;           /* 过温保护的过温水线 */
    oal_int16 us_temp_pro_pa_off_th;         /* 过温保护的关闭pa水线 */
} __OAL_DECLARE_PACKED hal_temp_pri_custom_stru;

/* 私有定制化参数 */
typedef struct
{
    oal_uint16 us_cali_mask;
    oal_int16 s_dsss2ofdm_dbb_pwr_bo_val; /* 相同scaling值的情况下，基带DSSS信号相对OFDM信号的功率回退值 单位0.01db */
    oal_uint32 ul_resv;
    oal_uint8 uc_fast_check_cnt;
    oal_uint8 uc_voe_switch;  // 是否开启11k,11v,11r
    /* dyn cali */
    oal_uint16 aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT]; /* 动态校准开关2.4g 5g */
    oal_uint8 uc_5g_ext_fem_type;
    oal_uint8 chba_en;
    hal_temp_pri_custom_stru st_temp_pri_custom;
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    oal_uint8 uc_dyn_bypass_extlna_enable;
#endif
    oal_uint8 uc_hcc_flowctrl_type;
    oal_uint8 uc_i3c_switch;
    oal_uint8 chba_social_chan;
    oal_uint8 chba_social_follow_work_switch;
    oal_uint8 close_filter_switch;
} __OAL_DECLARE_PACKED wlan_cfg_customize_priv;

typedef struct
{
    /* FCC或CE最大功率定制化项 */
    oal_uint8 auc_5g_txpwr_20M[FCC_CE_CH_NUM_5G_20M];
    oal_uint8 auc_5g_txpwr_40M[FCC_CE_CH_NUM_5G_40M];
    oal_uint8 auc_5g_txpwr_80M[FCC_CE_CH_NUM_5G_80M];
    oal_uint8 auc_5g_txpwr_160M[FCC_CE_CH_NUM_5G_160M];
    oal_uint8 auc_2g_txpwr[WLAN_2G_SUB_BAND_NUM][FCC_CE_SIG_TYPE_NUM_2G];
} __OAL_DECLARE_PACKED wlan_fcc_ce_power_limit_stru;

typedef struct {
    oal_uint8 auc_5g_txpwr_20M[FCC_CE_EXT_CH_NUM_5G_20M];
    oal_uint8 auc_5g_txpwr_40M[FCC_CE_EXT_CH_NUM_5G_40M];
} __OAL_DECLARE_PACKED wlan_ext_fcc_ce_power_limit_stru;
/* 定制化 结构体 */
typedef struct
{
    /* delt power */
    oal_int8 ac_delt_txpower[NUM_OF_NV_MAX_TXPOWER]; /* 每个速率对应功率与base power的相对值 */

    /* BASE POWER */
    oal_uint8 auc_base_power[CUS_BASE_POWER_NUM]; /* 基准最大发射功率2G,5G band1~band7,2G */

    /* SAR CTRL */
    oal_uint8 auc_sar_ctrl_params[CUS_SAR_NUM]; /* CE认证SAR标准最大功率限制 */

    /* 定制化下发的FCC或CE功率限制 */
    wlan_fcc_ce_power_limit_stru st_fcc_ce_txpwer_limit;
    /* 次边带信道功率控制 */
    wlan_ext_fcc_ce_power_limit_stru st_ext_fcc_ce_txpwer_limit;
    /* 5G高band最大发送功率 */
    oal_uint8 uc_5g_high_band_max_pow;

    /* 窄带2.4G基础功率 */
    oal_uint8 uc_nb_base_power;
} __OAL_DECLARE_PACKED wlan_customize_power_params_stru;

typedef struct {
    /* MAC统计 */
    oal_uint32 rx_direct_time;
    oal_uint32 rx_nondir_time;
    oal_uint32 tx_time;
} hal_ch_mac_statics_stru;
typedef struct {
    uint8_t uc_cmd_type;
    uint8_t uc_len;
    uint16_t us_cfg_id;
    uint32_t value;
} mac_cfg_set_tlv_stru;

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#pragma pack()
#endif

/*****************************************************************************
  10.2 对外暴露的配置接口
*****************************************************************************/
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST) ||  \
     (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))

/************************  1102  CHIP********************************************/
#define HAL_CHIP_LEVEL_FUNC_EXTERN
extern oal_void hi1102_get_chip_version(hal_to_dmac_chip_stru *pst_hal_chip, oal_uint32 *pul_chip_ver);
/************************  1102  DEVICE********************************************/
#define HAL_DEVICE_LEVEL_FUNC_EXTERN
extern oal_void hi1102_rx_init_dscr_queue(hal_to_dmac_device_stru *pst_device, oal_uint8 uc_set_hw);
extern oal_void hi1102_rx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device, oal_uint8 uc_destroy_netbuf);
extern oal_void hi1102_al_rx_init_dscr_queue(hal_to_dmac_device_stru *pst_device);

extern oal_void hi1102_al_rx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_tx_init_dscr_queue(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_tx_destroy_dscr_queue(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_init_hw_rx_isr_list(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_destroy_hw_rx_isr_list(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_flush_tx_complete_irq(hal_to_dmac_device_stru *pst_hal_dev);
extern oal_void hi1102_tx_fill_basic_ctrl_dscr(hal_tx_dscr_stru *p_tx_dscr, hal_tx_mpdu_stru *pst_mpdu);
extern oal_void hi1102_tx_ctrl_dscr_link(hal_tx_dscr_stru *pst_tx_dscr_prev, hal_tx_dscr_stru *pst_tx_dscr);
extern oal_void hi1102_get_tx_dscr_next(hal_tx_dscr_stru *pst_tx_dscr, hal_tx_dscr_stru **ppst_tx_dscr_next);
extern oal_void hi1102_tx_ctrl_dscr_unlink(hal_tx_dscr_stru *pst_tx_dscr);
extern oal_void hi1102_tx_ucast_data_set_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
    hal_tx_txop_feature_stru *pst_txop_feature, hal_tx_txop_alg_stru *pst_txop_alg,
    hal_tx_ppdu_feature_stru *pst_ppdu_feature);
extern oal_void hi1102_tx_non_ucast_data_set_dscr(hal_to_dmac_device_stru *pst_hal_device,
                                                  hal_tx_dscr_stru *pst_tx_dscr,
                                                  hal_tx_txop_feature_stru *pst_txop_feature,
                                                  hal_tx_txop_alg_stru *pst_txop_alg,
                                                  hal_tx_ppdu_feature_stru *pst_ppdu_feature);
extern oal_void hi1102_tx_ucast_data_set_dscr_rts_params(hal_to_dmac_device_stru *pst_device,
                                                         hal_tx_dscr_stru *pst_tx_dscr);
extern oal_void hi1102_tx_set_dscr_modify_mac_header_length(hal_tx_dscr_stru *pst_tx_dscr,
                                                            oal_uint8 uc_mac_header_length);
extern oal_void hi1102_tx_set_dscr_seqno_sw_generate(hal_tx_dscr_stru *pst_tx_dscr,
                                                     oal_uint8 uc_sw_seqno_generate);
extern oal_void hi1102_tx_get_size_dscr(oal_uint8 us_msdu_num, oal_uint32 *pul_dscr_one_size,
                                        oal_uint32 *pul_dscr_two_size);
extern oal_void hi1102_tx_get_vap_id(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_vap_id);
extern oal_void hi1102_tx_get_dscr_ctrl_one_param(hal_tx_dscr_stru *pst_tx_dscr,
                                                  hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one_param);
extern oal_void hi1102_tx_get_dscr_seq_num(hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 *pus_seq_num);
extern oal_void hi1102_tx_get_dscr_tx_cnt(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_tx_count);
extern oal_void hi1102_tx_dscr_get_rate3(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_rate);
extern oal_void hi1102_tx_set_dscr_status(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 uc_status);
extern oal_void hi1102_tx_get_dscr_status(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_status);
extern oal_void hi1102_tx_get_dscr_send_rate_rank(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_send_rate_rank);
extern oal_void hi1102_tx_get_dscr_chiper_type(hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_chiper_type,
                                               oal_uint8 *puc_chiper_key_id);
extern oal_void hi1102_tx_get_dscr_ba_ssn(hal_tx_dscr_stru *pst_tx_dscr, oal_uint16 *pus_ba_ssn);
extern oal_void hi1102_tx_get_dscr_ba_bitmap(hal_tx_dscr_stru *pst_tx_dscr, oal_uint32 *pul_ba_bitmap);
extern oal_void hi1102_tx_put_dscr(hal_to_dmac_device_stru *pst_hal_device,
                                   hal_tx_queue_type_enum_uint8 en_tx_queue_type, hal_tx_dscr_stru *past_tx_dscr);
extern oal_void hi1102_get_tx_q_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_status,
                                       oal_uint8 uc_qnum);
extern oal_void hi1102_tx_get_ampdu_len(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr,
                                        oal_uint32 *pul_ampdu_len);
extern oal_void hi1102_tx_get_protocol_mode(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr,
                                            oal_uint8 *puc_protocol_mode);
extern oal_void hi1102_tx_get_bw_mode(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr,
                                      wlan_bw_cap_enum_uint8 *pen_bw_mode);
extern oal_void hi1102_rx_get_info_dscr(oal_uint32 *pul_rx_dscr, hal_rx_ctl_stru *pst_rx_ctl,
                                        hal_rx_status_stru *pst_rx_status, hal_rx_statistic_stru *pst_rx_statistics);
extern oal_void hi1102_get_hal_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_vap_id,
                                   hal_to_dmac_vap_stru **ppst_hal_vap);
extern oal_void hi1102_rx_get_netbuffer_addr_dscr(oal_uint32 *pul_rx_dscr, oal_netbuf_stru **ppul_mac_hdr_addr);
extern oal_void hi1102_rx_show_dscr_queue_info(hal_to_dmac_device_stru *pst_hal_device,
                                               hal_rx_dscr_queue_id_enum_uint8 en_rx_dscr_type);
extern oal_void hi1102_rx_sync_invalid_dscr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_dscr,
                                            oal_uint8 en_queue_num);
extern oal_void hi1102_rx_free_dscr_list(hal_to_dmac_device_stru *pst_hal_device,
                                         hal_rx_dscr_queue_id_enum_uint8 en_queue_num, oal_uint32 *pul_rx_dscr);
extern oal_void hi1102_dump_tx_dscr(oal_uint32 *pul_tx_dscr);
extern oal_void hi1102_reg_write(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint32 ul_val);
extern oal_void hi1102_reg_write16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint16 us_val);
extern oal_void hi1102_set_counter_clear(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_machw_rx_buff_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_dscr,
                                              hal_rx_dscr_queue_id_enum_uint8 en_queue_num);
extern oal_uint32 hi1102_set_machw_rx_buff_addr_sync(hal_to_dmac_device_stru *pst_hal_device,
                                                     oal_uint32 ul_rx_dscr,
                                                     hal_rx_dscr_queue_id_enum_uint8 en_queue_num);
extern oal_void hi1102_rx_add_dscr(hal_to_dmac_device_stru *pst_hal_device,
                                   hal_rx_dscr_queue_id_enum_uint8 en_queue_num, oal_uint16 us_rx_dscr_num);
extern oal_void hi1102_free_rx_isr_list(hal_to_dmac_device_stru *pst_hal_device,
                                        oal_dlist_head_stru *pst_rx_isr_list, oal_bool_enum_uint8 en_recycle);
extern oal_void hi1102_set_machw_tx_suspend(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_machw_tx_resume(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_reset_phy_machw(hal_to_dmac_device_stru *pst_hal_device,
                                       hal_reset_hw_type_enum_uint8 en_type,
                                       oal_uint8 sub_mod, oal_uint8 uc_reset_phy_reg, oal_uint8 uc_reset_mac_reg);
extern oal_void hi1102_disable_machw_phy_and_pa(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_machw_phy_and_pa(hal_to_dmac_device_stru *pst_hal_device);
extern oal_bool_enum_uint8 hi1102_is_machw_enabled(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_rf_temperature_trig_cali(hal_to_dmac_device_stru *pst_hal_device);

extern oal_void hi1102_initialize_machw(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_freq_band(hal_to_dmac_device_stru *pst_hal_device,
                                     wlan_channel_band_enum_uint8 en_band);
extern oal_void hi1102_set_bandwidth_mode(hal_to_dmac_device_stru *pst_hal_device,
                                          wlan_channel_bandwidth_enum_uint8 en_bandwidth);

extern oal_void hi1102_set_upc_data(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band,
                                    oal_uint8 uc_subband_idx);
extern oal_void hi1102_get_subband_index(wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel_num,
                                         oal_uint8 *puc_subband_idx);
extern oal_void hi1102_get_legacy_data_rate_idx(oal_uint8 uc_rate, oal_uint8 *puc_rate_idx);
extern oal_void hi1102_get_pow_index(hal_user_pow_info_stru *pst_user_power_info,
                                     oal_uint8 uc_cur_rate_pow_idx,
                                     hal_tx_txop_tx_power_stru *pst_tx_power,
                                     oal_uint8 *puc_pow_idx);

extern oal_void hi1102_set_band_spec_frame_tx_power(hal_to_dmac_device_stru *pst_hal_device,
                                                    wlan_channel_band_enum_uint8 en_band,
                                                    oal_uint8 uc_chan_num,
                                                    hal_rate_tpc_code_gain_table_stru *pst_rate_tpc_table,
                                                    oal_uint8 uc_pow_level_idx);
extern oal_void hi1102_pow_update_upc_code(hal_to_dmac_vap_stru *pst_hal_vap,
                                           hal_to_dmac_device_stru *pst_hal_device,
                                           oal_uint8 uc_cur_ch_num, wlan_channel_bandwidth_enum_uint8 en_bandwidth);

extern oal_void hi1102_pow_update_p2p_upc(hal_to_dmac_device_stru *pst_hal_device,
                                          wlan_channel_band_enum_uint8 en_freq_band,
                                          oal_uint8 uc_ch_num,
                                          wlan_channel_bandwidth_enum_uint8 en_bandwidth);

extern oal_void hi1102_pow_init_tx_power(hal_to_dmac_device_stru *pst_hal_dev);
extern oal_void hi1102_pow_get_rf_dev_base_power(hal_to_dmac_device_stru *pst_hal_device,
                                                 wlan_channel_band_enum_uint8 en_freq_band,
                                                 oal_uint8 *puc_base_power,
                                                 oal_uint8 uc_subband_idx);
extern oal_void hi1102_get_rate_limit_tx_power(hal_to_dmac_device_stru *pst_hal_device,
                                               oal_uint8 uc_rate_idx,
                                               oal_uint8 uc_base_pow,
                                               wlan_channel_band_enum_uint8 en_freq_band,
                                               oal_uint8 *puc_rate_txpwr_limit);

extern oal_uint32 hi1102_get_center_freq_chan_num(oal_uint8 uc_chan_number,
                                                  wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                                  oal_uint8 *puc_center_freq_chn);

extern oal_void hi1102_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *pst_hal_dev,
                                                oal_bool_enum_uint8 en_rf_regctl);
extern oal_void hi1102_config_fft_window_offset(oal_uint32 ul_offset_window);

#ifdef _PRE_WLAN_FEATURE_TPC
extern oal_void hi1102_set_tpc_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band,
                                      oal_uint8 uc_channel_num);
#endif
extern oal_void hi1102_process_phy_freq(hal_to_dmac_device_stru *pst_hal_device);

extern oal_void hi1102_set_primary_channel(hal_to_dmac_device_stru *pst_hal_device,
                                           oal_uint8 uc_channel_num,
                                           oal_uint8 uc_band,
                                           oal_uint8 uc_channel_idx,
                                           wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void hi1102_set_txop_check_cca(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 en_txop_check_cca);
extern oal_void hi1102_add_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device,
                                              oal_uint8 uc_lut_index, oal_uint8 *puc_dst_addr, oal_uint8 uc_tid,
                                              oal_uint16 uc_seq_no, oal_uint8 uc_win_size);
extern oal_void hi1102_remove_machw_ba_lut_entry(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index);
extern oal_void hi1102_get_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_index,
                                           oal_uint32 *pst_addr_h, oal_uint32 *pst_addr_l, oal_uint32 *pst_ba_para);
extern oal_void hi1102_restore_machw_ba_params(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_index,
                                               oal_uint32 ul_addr_h, oal_uint32 ul_addr_l, oal_uint32 ul_ba_para);
extern oal_void hi1102_set_tx_sequence_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                           oal_uint8 uc_tid, oal_uint8 uc_qos_flag, oal_uint32 ul_val_write);
extern oal_void hi1102_get_tx_sequence_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                           oal_uint8 uc_tid, oal_uint8 uc_qos_flag, oal_uint16 *pus_val_read);
extern oal_void hi1102_reset_init(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_reset_destroy(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_reset_reg_restore(hal_to_dmac_device_stru *pst_hal_device,
                                         hal_reset_hw_type_enum_uint8 en_type);
extern oal_void hi1102_reset_reg_save(hal_to_dmac_device_stru *pst_hal_device,
                                      hal_reset_hw_type_enum_uint8 en_type);
extern oal_void hi1102_reset_reg_dma_save(hal_to_dmac_device_stru *pst_hal, oal_uint8 *uc_dmach0,
                                          oal_uint8 *uc_dmach1, oal_uint8 *uc_dmach2);
extern oal_void hi1102_reset_reg_dma_restore(hal_to_dmac_device_stru *pst_hal, oal_uint8 *uc_dmach0,
                                             oal_uint8 *uc_dmach1, oal_uint8 *uc_dmach2);
extern oal_void hi1102_disable_machw_ack_trans(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_machw_ack_trans(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_disable_machw_cts_trans(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_machw_cts_trans(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_initialize_phy(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_phy_update_scaling_reg(oal_uint8 *puc_dbb_scaling);
extern oal_void hi1102_radar_config_reg(hal_to_dmac_device_stru *pst_hal_device,
                                        hal_dfs_radar_type_enum_uint8 en_dfs_domain);
extern oal_void hi1102_initialize_rf_sys(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_rf_custom_reg(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_cali_matrix_data_send_func(oal_uint8 *puc_matrix_data, oal_uint16 us_frame_len,
                                                  oal_uint16 us_remain);
extern oal_void hi1102_cali_send_func(oal_uint8 *puc_cali_data, oal_uint16 us_frame_len, oal_uint16 us_remain);
extern oal_void hi1102_psm_rf_sleep(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_restore_reg);
extern oal_void hi1102_psm_rf_awake(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_restore_reg);
extern oal_void hi1102_pll_cfg_check(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_rf_get_line_control_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pus_value);
extern oal_void hi1102_initialize_soc(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_mac_int_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_status);
extern oal_void hi1102_clear_mac_int_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_status);
extern oal_void hi1102_get_mac_error_int_status(hal_to_dmac_device_stru *pst_hal_device,
                                                hal_error_state_stru *pst_state);
extern oal_void hi1102_clear_mac_error_int_status(hal_to_dmac_device_stru *pst_hal_device,
                                                  hal_error_state_stru *pst_status);
extern oal_void hi1102_unmask_mac_error_init_status(hal_to_dmac_device_stru *pst_hal_device,
                                                    hal_error_state_stru *pst_status);
extern oal_void hi1102_unmask_mac_init_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_status);
extern oal_void hi1102_show_irq_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_param);
extern oal_void hi1102_dump_all_rx_dscr(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_clear_irq_stat(hal_to_dmac_device_stru *pst_hal_device);

extern oal_void hi1102_get_vap(hal_to_dmac_device_stru *pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode,
                               oal_uint8 vap_id, hal_to_dmac_vap_stru **ppst_hal_vap);
extern oal_void hi1102_add_vap(hal_to_dmac_device_stru *pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode,
                               oal_uint8 uc_mac_vap_id, hal_to_dmac_vap_stru **ppst_hal_vap);
extern oal_void hi1102_del_vap(hal_to_dmac_device_stru *pst_hal_device, wlan_vap_mode_enum_uint8 vap_mode,
                               oal_uint8 vap_id);

extern oal_void hi1102_config_eifs_time(hal_to_dmac_device_stru *pst_hal_device,
                                        wlan_protocol_enum_uint8 en_protocol);
extern oal_void hi1102_register_alg_isr_hook(hal_to_dmac_device_stru *pst_hal_device,
                                             hal_isr_type_enum_uint8 en_isr_type,
                                             hal_alg_noify_enum_uint8 en_alg_notify, p_hal_alg_isr_func p_func);
extern oal_void hi1102_unregister_alg_isr_hook(hal_to_dmac_device_stru *pst_hal_device,
                                               hal_isr_type_enum_uint8 en_isr_type,
                                               hal_alg_noify_enum_uint8 en_alg_notify);
extern oal_void hi1102_register_gap_isr_hook(hal_to_dmac_device_stru *pst_hal_device,
                                             hal_isr_type_enum_uint8 en_isr_type, p_hal_gap_isr_func p_func);
extern oal_void hi1102_unregister_gap_isr_hook(hal_to_dmac_device_stru *pst_hal_device,
                                               hal_isr_type_enum_uint8 en_isr_type);
extern oal_void hi1102_one_packet_start(struct tag_hal_to_dmac_device_stru *pst_hal_device,
                                        hal_one_packet_cfg_stru *pst_cfg);
extern oal_void hi1102_one_packet_stop(struct tag_hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_one_packet_get_status(struct tag_hal_to_dmac_device_stru *pst_hal_device,
                                             hal_one_packet_status_stru *pst_status);
extern oal_void hi1102_reset_nav_timer(struct tag_hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_clear_hw_fifo(struct tag_hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_mask_interrupt(struct tag_hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_offset);
extern oal_void hi1102_unmask_interrupt(struct tag_hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_offset);
extern oal_void hi1102_reg_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr, oal_uint32 *pul_val);
extern oal_void hi1102_reg_info16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_addr,
                                  oal_uint16 *pus_val);
#ifdef _PRE_WLAN_FEATURE_DATA_SAMPLE
extern oal_void hi1102_free_sample_mem(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_sample_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 **pul_start_addr,
                                         oal_uint32 *ul_reg_num);
extern oal_void hi1102_get_sample_state(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pus_reg_val);
extern oal_void hi1102_set_pktmem_bus_access(hal_to_dmac_device_stru *pst_hal_device);
#endif
extern oal_void hi1102_get_all_tx_q_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_val);
extern oal_void hi1102_get_ampdu_bytes(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_tx_bytes,
                                       oal_uint32 *pul_rx_bytes);
extern oal_void hi1102_get_rx_err_count(hal_to_dmac_device_stru *pst_hal_device,
                                        oal_uint32 *pul_cnt1, oal_uint32 *pul_cnt2,
                                        oal_uint32 *pul_cnt3, oal_uint32 *pul_cnt4,
                                        oal_uint32 *pul_cnt5, oal_uint32 *pul_cnt6);
extern oal_void hi1102_show_fsm_info(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_mac_error_msg_report(hal_to_dmac_device_stru *pst_hal_device,
                                            hal_mac_error_type_enum_uint8 en_error_type);
extern oal_void hi1102_en_soc_intr(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_beacon_filter(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_disable_beacon_filter(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_non_frame_filter(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_disable_non_frame_mgmt_filter(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_monitor_mode(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_disable_monitor_mode(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_pmf_crypto(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_crypto);
extern oal_void hi1102_ce_add_key(hal_to_dmac_device_stru *pst_hal_device, hal_security_key_stru *pst_security_key,
                                  oal_uint8 *puc_addr);
extern oal_void hi1102_ce_del_key(hal_to_dmac_device_stru *pst_hal_device,
                                  hal_security_key_stru *pst_security_key);
extern oal_void hi1102_disable_ce(hal_to_dmac_device_stru *pst_device);
extern oal_void hi1102_ce_add_peer_macaddr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_idx,
                                           oal_uint8 *puc_addr);
extern oal_void hi1102_ce_del_peer_macaddr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_idx);
extern oal_void hi1102_set_rx_pn(hal_to_dmac_device_stru *pst_hal_device, hal_pn_lut_cfg_stru *pst_pn_lut_cfg);
extern oal_void hi1102_get_rx_pn(hal_to_dmac_device_stru *pst_hal_device, hal_pn_lut_cfg_stru *pst_pn_lut_cfg);
extern oal_void hi1102_set_tx_pn(hal_to_dmac_device_stru *pst_hal_device, hal_pn_lut_cfg_stru *pst_pn_lut_cfg);
extern oal_void hi1102_get_tx_pn(hal_to_dmac_device_stru *pst_hal_device, hal_pn_lut_cfg_stru *pst_pn_lut_cfg);
extern oal_void hi1102_get_rate_80211g_table(oal_void **pst_rate);
extern oal_void hi1102_get_rate_80211g_num(oal_uint32 *pst_data_num);
extern oal_void hi1102_get_hw_addr(oal_uint8 *puc_addr);
extern oal_void hi1102_enable_ch_statics(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable);
extern oal_void hi1102_set_ch_statics_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_period);
extern oal_void hi1102_set_ch_measurement_period(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_period);
extern oal_void hi1102_get_ch_statics_result(hal_to_dmac_device_stru *pst_hal_device,
                                             hal_ch_statics_irq_event_stru *pst_ch_statics);
extern oal_void hi1102_get_ch_measurement_result(hal_to_dmac_device_stru *pst_hal_device,
                                                 hal_ch_statics_irq_event_stru *pst_ch_statics);
extern oal_void hi1102_enable_radar_det(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable);

extern oal_void hi1102_enable_sigB(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable);
extern oal_void hi1102_enable_improve_ce(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable);

#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
extern oal_void hi1102_set_acc_symb_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_num);
extern oal_void hi1102_set_improve_ce_threshold(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_val);
#endif
extern oal_void hi1102_get_radar_det_result(hal_to_dmac_device_stru *pst_hal_device,
                                            hal_radar_det_event_stru *pst_radar_info);
extern oal_void hi1102_update_rts_rate_params(hal_to_dmac_device_stru *pst_hal_device,
                                              wlan_channel_band_enum_uint8 en_band);
extern oal_void hi1102_set_rts_rate_params(hal_to_dmac_device_stru *pst_hal_device,
                                           hal_cfg_rts_tx_param_stru *pst_hal_rts_tx_param);
extern oal_void hi1102_set_rts_rate_selection_mode(hal_to_dmac_device_stru *pst_hal_device,
                                                   oal_uint8 uc_rts_rate_select_mode);
#ifdef _PRE_WLAN_FEATURE_TPC
extern oal_void hi1102_get_rf_temp(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_cur_temp);
extern oal_void hi1102_set_tpc_init_rate_dac_lpf_table(oal_uint8 *pauc_rate_pow_table_2G,
                                                       oal_uint8 *pauc_rate_pow_table_5G,
                                                       oal_uint8 *pauc_mode_len, oal_uint8 uc_pow_mode);

extern oal_void hi1102_set_spec_frm_phy_tx_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band,
                                                oal_uint8 uc_subband_idx);
extern oal_void hi1102_get_tpc_delay_reg_param(hal_to_dmac_device_stru *pst_hal_device,
                                               oal_uint32 *pul_phy_tx_up_down_time_reg,
                                               oal_uint32 *pul_phy_rx_up_down_time_reg,
                                               oal_uint32 *pul_rf_reg_wr_delay1, oal_uint32 *pul_rf_reg_wr_delay2);
extern oal_void hi1102_set_tpc_delay_reg_param(hal_to_dmac_device_stru *pst_hal_device,
                                               oal_uint32 ul_phy_tx_up_down_time_reg,
                                               oal_uint32 ul_phy_rx_up_down_time_reg,
                                               oal_uint32 ul_rf_reg_wr_delay1, oal_uint32 ul_rf_reg_wr_delay2);
extern oal_void hi1102_get_tpc_rf_reg_param(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_uint16 *pus_dac_val, oal_uint16 *pus_pa_val, oal_uint16 *pus_lpf_val,
                                            oal_uint16 *paus_2g_upc_val, oal_uint16 *paus_5g_upc_val,
                                            oal_uint8 uc_chain_idx);
extern oal_void hi1102_set_tpc_rf_reg_param(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_uint16 us_dac_val, oal_uint16 us_pa_val, oal_uint16 us_lpf_val,
                                            oal_uint16 *paus_2g_upc_val, oal_uint16 *paus_5g_upc_val,
                                            oal_uint8 uc_chain_idx);
extern oal_void hi1102_set_dpd_by_power(hal_tx_txop_rate_params_stru *pst_rate,
                                        oal_uint8 uc_power_level, oal_uint32 ul_dpd_configure, oal_uint32 ul_rate_idx);

#endif
extern oal_void hi1102_set_tpc_ctrl_reg_param(hal_to_dmac_device_stru *pst_hal_device,
                                              oal_uint32 ul_tpc_ctrl_param);
extern oal_void hi1102_set_tpc_phy_reg_param(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_bcn_rate(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_data_rate);
extern oal_void hi1102_set_bcn_phy_tx_mode(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_tpc_code);
extern oal_void hi1102_get_spec_frm_rate(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_dac_lpc_pa_upc_level(oal_uint8 uc_dac_lpf_code,
                                                oal_int8 *pac_tpc_level_table, oal_uint8 uc_tpc_level_num,
                                                oal_uint8 *pauc_dac_lpf_pa_code_table, oal_int16 *pas_upc_gain_table,
                                                oal_int16 *pas_other_gain_table,
                                                wlan_channel_band_enum_uint8 en_freq_band);

extern oal_void hi1102_irq_affinity_init(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_core_id);

#ifdef _PRE_WLAN_FEATURE_TXBF
extern oal_void hi1102_set_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device,
                                                     oal_uint32 ul_matrix);
extern oal_void hi1102_get_legacy_matrix_buf_pointer(hal_to_dmac_device_stru *pst_hal_device,
                                                     oal_uint32 *pul_matrix);
extern oal_void hi1102_set_vht_report_rate(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rate);
extern oal_void hi1102_set_vht_report_phy_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_phy_mode);
extern oal_void hi1102_set_ndp_rate(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rate);
extern oal_void hi1102_set_ndp_phy_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_phy_mode);
extern oal_void hi1102_set_ndp_max_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_ndp_time);
extern oal_void hi1102_set_ndpa_duration(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ndpa_duration);
extern oal_void hi1102_set_ndp_group_id(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_group_id);
extern oal_void hi1102_set_ndp_partial_aid(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 ul_reg_value);
extern oal_void hi1102_set_phy_legacy_bf_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_phy_txbf_legacy_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_phy_pilot_bf_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_ht_buffer_num(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 ul_reg_value);
extern oal_void hi1102_set_ht_buffer_step(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 ul_reg_value);
extern oal_void hi1102_set_ht_buffer_pointer(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_delete_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index);
extern oal_void hi1102_set_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                         oal_uint16 ul_reg_value);
extern oal_void hi1102_get_txbf_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                         oal_uint32 *pst_reg_value);
extern oal_void hi1102_set_h_matrix_timeout(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_dl_mumimo_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 value);
extern oal_void hi1102_set_mu_aid_matrix_info(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_aid,
                                              oal_uint8 *p_matrix);
extern oal_void hi1102_set_sta_membership_status_63_32(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 value);
extern oal_void hi1102_set_sta_membership_status_31_0(hal_to_dmac_device_stru *pst_hal_device,
                                                      oal_uint32 ul_value);
extern oal_void hi1102_set_sta_user_p_63_48(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value);
extern oal_void hi1102_set_sta_user_p_47_32(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value);
extern oal_void hi1102_set_sta_user_p_31_16(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value);
extern oal_void hi1102_set_sta_user_p_15_0(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value);

#endif
extern oal_void hi1102_set_peer_lut_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_mac_addr,
                                         oal_uint8 uc_lut_index);

extern oal_void hi1102_enable_smart_antenna_gpio_set_default_antenna(hal_to_dmac_device_stru *pst_hal_device,
                                                                     oal_uint32 ul_reg_value);
extern oal_void hi1102_delete_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index);
extern oal_void hi1102_set_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                               oal_uint16 ul_reg_value);
extern oal_void hi1102_get_smart_antenna_value(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index,
                                               oal_uint32 *pst_reg_value);

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
extern oal_void hi1102_set_weak_intf_rssi_th(hal_to_dmac_device_stru *pst_device, oal_int32 l_reg_val);
extern oal_void hi1102_set_agc_unlock_min_th(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_tx_reg_val,
                                             oal_int32 l_rx_reg_val);
extern oal_void hi1102_set_nav_max_duration(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_bss_dur,
                                            oal_uint32 us_obss_dur);
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
extern oal_void hi1102_set_counter1_clear(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_counter1_clear_ram(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_txrx_frame_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_reg_val);
extern oal_void hi1102_set_mac_clken(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_wctrl_enable);
#endif
extern oal_void hi1102_get_mac_statistics_data(hal_to_dmac_device_stru *pst_hal_device,
                                               hal_mac_key_statis_info_stru *pst_mac_key_statis);

#ifdef _PRE_WLAN_FEATURE_CCA_OPT
extern oal_void hi1102_get_ed_high_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 *l_ed_high_reg_val);
extern oal_void hi1102_set_ed_high_th(hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_ed_high_20_reg_val,
                                      oal_int32 l_ed_high_40_reg_val);
extern oal_void hi1102_enable_sync_error_counter(hal_to_dmac_device_stru *pst_hal_device,
                                                 oal_int32 l_enable_cnt_reg_val);
extern oal_void hi1102_get_sync_error_cnt(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *ul_reg_val);
extern oal_void hi1102_set_sync_err_counter_clear(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_cca_reg_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 *ac_reg_val);
#endif
extern oal_void hi1102_set_soc_lpm(hal_to_dmac_device_stru *pst_hal_device, hal_lpm_soc_set_enum_uint8 en_type,
                                   oal_uint8 uc_on_off, oal_uint8 uc_pcie_idle);
extern oal_void hi1102_set_psm_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off);
extern oal_void hi1102_set_psm_wakeup_mode(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_mode);
extern oal_void hi1102_set_psm_listen_interval(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_interval);
extern oal_void hi1102_set_psm_listen_interval_count(hal_to_dmac_vap_stru *pst_hal_vap,
                                                     oal_uint16 us_interval_count);
extern oal_void hi1102_set_psm_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_offset);
extern oal_void hi1102_set_psm_ext_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_offset);
extern oal_void hi1102_set_psm_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_beacon_period);
#if defined(_PRE_WLAN_FEATURE_TXOPPS)
extern oal_void hi1102_set_txop_ps_enable(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off);
extern oal_void hi1102_set_txop_ps_condition1(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off);
extern oal_void hi1102_set_txop_ps_condition2(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_on_off);
extern oal_void hi1102_set_txop_ps_partial_aid(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_partial_aid);
#endif
extern oal_void hi1102_set_wow_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_set_bitmap,
                                  hal_wow_param_stru *pst_para);
extern oal_void hi1102_set_lpm_state(hal_to_dmac_device_stru *pst_hal_device,
                                     hal_lpm_state_enum_uint8 uc_state_from, hal_lpm_state_enum_uint8 uc_state_to,
                                     oal_void *pst_para,
                                     oal_void *pst_wow_para);
extern oal_void hi1102_disable_machw_edca(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_enable_machw_edca(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_tx_abort_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_abort_en);
extern oal_void hi1102_set_coex_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_mac_ctrl,
                                     oal_uint32 ul_rf_ctrl);
extern oal_void hi1102_get_hw_version(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_hw_vsn,
                                      oal_uint32 *pul_hw_vsn_data, oal_uint32 *pul_hw_vsn_num);

#ifdef _PRE_DEBUG_MODE
extern oal_void hi1102_get_all_reg_value(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_cali_data(hal_to_dmac_device_stru *pst_hal_device);
#endif
extern oal_void hi1102_set_tx_dscr_field(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_data,
                                         hal_rf_test_sect_enum_uint8 en_sect);
extern oal_void hi1102_rf_test_disable_al_tx(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_rf_test_enable_al_tx(hal_to_dmac_device_stru *pst_hal_device,
                                            hal_tx_dscr_stru *pst_tx_dscr);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
extern oal_void hi1102_al_tx_set_agc_phy_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_value);
#endif

#ifdef _PRE_WLAN_PHY_PLL_DIV
extern oal_void hi1102_rf_set_freq_skew(oal_uint16 us_idx, oal_uint16 us_chn, oal_int16 as_corr_data[]);
#endif
extern oal_void hi1102_set_daq_mac_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_addr,
                                       oal_uint16 us_unit_len, oal_uint16 us_unit_num, oal_uint16 us_depth);
extern oal_void hi1102_set_daq_phy_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_daq_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_reg_value);
extern oal_void hi1102_get_daq_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_reg_value);

#if (_PRE_MULTI_CORE_MODE == _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
extern oal_void hi1102_set_dac_lpf_gain(hal_to_dmac_device_stru *pst_hal_device,
                                        oal_uint8 en_band, oal_uint8 en_bandwidth, oal_uint8 en_protocol_mode,
                                        oal_uint8 en_rate);
extern oal_void hi1102_get_pwr_comp_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_tx_ratio,
                                        oal_int16 *ps_pwr_comp_val);
extern oal_void hi1102_over_temp_handler(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_agc_threshold_handle(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_rssi);

#endif

extern oal_void hi1102_set_rx_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_rx_filter_val);
extern oal_void hi1102_get_rx_filter(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pst_reg_value);
extern oal_void hi1102_set_beacon_timeout_val(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_value);
extern oal_void hi1102_psm_clear_mac_rx_isr(hal_to_dmac_device_stru *pst_hal_device);

#define HAL_VAP_LEVEL_FUNC_EXTERN
extern oal_void hi1102_vap_tsf_get_32bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_tsf_lo);
extern oal_void hi1102_vap_tsf_set_32bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_tsf_lo);
extern oal_void hi1102_vap_tsf_get_64bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_tsf_hi,
                                         oal_uint32 *pul_tsf_lo);
extern oal_void hi1102_vap_tsf_set_64bit(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_tsf_hi,
                                         oal_uint32 ul_tsf_lo);
extern oal_void hi1102_vap_send_beacon_pkt(hal_to_dmac_vap_stru *pst_hal_vap,
                                           hal_beacon_tx_params_stru *pst_params);
extern oal_void hi1102_vap_set_beacon_rate(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_beacon_rate);
extern oal_void hi1102_vap_beacon_suspend(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_vap_beacon_resume(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_vap_set_machw_prot_params(hal_to_dmac_vap_stru *pst_hal_vap,
                                                 hal_tx_txop_rate_params_stru *pst_phy_tx_mode,
                                                 hal_tx_txop_per_rate_params_union *pst_data_rate);

extern oal_void hi1102_vap_set_macaddr(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_mac_addr);
extern oal_void hi1102_vap_set_opmode(hal_to_dmac_vap_stru *pst_hal_vap, wlan_vap_mode_enum_uint8 en_vap_mode);

extern oal_void hi1102_vap_clr_opmode(hal_to_dmac_vap_stru *pst_hal_vap, wlan_vap_mode_enum_uint8 en_vap_mode);
extern oal_void hi1102_vap_set_machw_aifsn_all_ac(hal_to_dmac_vap_stru *pst_hal_vap,
                                                  oal_uint8 uc_bk,
                                                  oal_uint8 uc_be,
                                                  oal_uint8 uc_vi,
                                                  oal_uint8 uc_vo);
extern oal_void hi1102_vap_set_machw_aifsn_ac(hal_to_dmac_vap_stru *pst_hal_vap,
                                              wlan_wme_ac_type_enum_uint8 en_ac,
                                              oal_uint8 uc_aifs);
extern oal_void hi1102_vap_set_machw_aifsn_ac_wfa(hal_to_dmac_vap_stru *pst_hal_vap,
                                                  wlan_wme_ac_type_enum_uint8 en_ac,
                                                  oal_uint8 uc_aifs,
                                                  wlan_wme_ac_type_enum_uint8 en_wfa_lock);
extern oal_void hi1102_vap_set_edca_machw_cw(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmax,
                                             oal_uint8 uc_cwmin, oal_uint8 uc_ac_type);
extern oal_void hi1102_vap_set_edca_machw_cw_wfa(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_cwmaxmin,
                                                 oal_uint8 uc_ac_type, wlan_wme_ac_type_enum_uint8 en_wfa_lock);
extern oal_void hi1102_vap_get_edca_machw_cw(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_cwmax,
                                             oal_uint8 *puc_cwmin, oal_uint8 uc_ac_type);

extern oal_void hi1102_vap_set_machw_txop_limit_bkbe(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_be,
                                                     oal_uint16 us_bk);

extern oal_void hi1102_vap_get_machw_txop_limit_bkbe(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_be,
                                                     oal_uint16 *pus_bk);
extern oal_void hi1102_vap_set_machw_txop_limit_vivo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_vo,
                                                     oal_uint16 us_vi);
extern oal_void hi1102_vap_get_machw_txop_limit_vivo(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_vo,
                                                     oal_uint16 *pus_vi);
extern oal_void hi1102_vap_set_machw_edca_bkbe_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_be,
                                                        oal_uint16 us_bk);
extern oal_void hi1102_vap_get_machw_edca_bkbe_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_be,
                                                        oal_uint16 *pus_bk);
extern oal_void hi1102_vap_set_machw_edca_vivo_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_vo,
                                                        oal_uint16 us_vi);
extern oal_void hi1102_vap_get_machw_edca_vivo_lifetime(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_vo,
                                                        oal_uint16 *pus_vi);
extern oal_void hi1102_vap_set_machw_prng_seed_val_all_ac(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_vap_start_tsf(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_dbac_enable);
extern oal_void hi1102_vap_read_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_value);
extern oal_void hi1102_vap_write_tbtt_timer(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_value);
extern oal_void hi1102_vap_set_machw_tsf_disable(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_vap_set_machw_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_beacon_period);
extern oal_void hi1102_vap_update_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_beacon_period);
extern oal_void hi1102_vap_get_beacon_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_beacon_period);
extern oal_void hi1102_vap_set_noa(hal_to_dmac_vap_stru *pst_hal_vap,
                                   oal_uint32 ul_start_tsf,
                                   oal_uint32 ul_duration,
                                   oal_uint32 ul_interval,
                                   oal_uint8 uc_count);

extern oal_void hi1102_sta_tsf_restore(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_sta_tsf_save(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_need_restore);
#ifdef _PRE_WLAN_FEATURE_P2P
extern oal_void hi1102_vap_set_ops(hal_to_dmac_vap_stru *pst_hal_vap,
                                   oal_uint8 en_ops_ctrl,
                                   oal_uint8 uc_ct_window);
extern oal_void hi1102_vap_enable_p2p_absent_suspend(hal_to_dmac_vap_stru *pst_hal_vap,
                                                     oal_bool_enum_uint8 en_suspend_enable);
#endif
extern oal_void hi1102_set_sta_bssid(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_byte);
extern oal_void hi1102_set_sta_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_dtim_period);
extern oal_void hi1102_get_sta_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_dtim_period);
extern oal_void hi1102_set_sta_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 ul_dtim_count);
extern oal_void hi1102_get_psm_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *uc_dtim_count);
extern oal_void hi1102_set_psm_dtim_count(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_dtim_count);
extern oal_void hi1102_set_psm_dtim_period(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 uc_dtim_period,
                                           oal_uint8 uc_listen_intvl_to_dtim_times,
                                           oal_bool_enum_uint8 en_receive_dtim);
extern oal_void hi1102_enable_sta_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_disable_sta_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_mwo_det_enable_mac_counter(hal_to_dmac_device_stru *pst_hal_device,
                                                  oal_int32 l_enable_reg_val);
extern oal_void hi1102_tx_enable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_index);
extern oal_void hi1102_tx_disable_peer_sta_ps_ctrl(hal_to_dmac_device_stru *pst_hal_device,
                                                   oal_uint8 uc_lut_index);
extern oal_void hi1102_cfg_slottime_type(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_slottime_type);
extern oal_void hi1102_cfg_rsp_dyn_bw(oal_bool_enum_uint8 en_set, wlan_bw_cap_enum_uint8 en_dyn_bw);
extern oal_void hi1102_get_cfg_rsp_rate_mode(oal_uint32 *pul_rsp_rate_cfg_mode);
extern oal_void hi1102_set_rsp_rate(oal_uint32 ul_rsp_rate_val);

#if (_PRE_MULTI_CORE_MODE == _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
extern oal_void hi1102_get_hw_status(hal_to_dmac_device_stru *pst_hal_device,
                                     oal_uint32 *ul_cali_check_hw_status);
extern oal_void hi1102_pm_wlan_servid_register(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint32 *pul_ret);
extern oal_void hi1102_pm_enable_front_end(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_enable_paldo);
extern oal_void hi1102_dyn_set_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_pm_wlan_servid_unregister(hal_to_dmac_vap_stru *pst_hal_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
extern oal_void hi1102_coex_irq_en_set(oal_uint8 uc_intr_en);
extern oal_void hi1102_coex_sw_irq_clr_set(oal_uint8 uc_irq_clr);
extern oal_void hi1102_set_rx_rsp_other_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_rx_rsp_cts_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_rx_rsp_ba_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_tx_rsp_other_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_tx_rsp_cts_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_tx_rsp_ba_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_set_tx_one_pkt_pri_mode(hal_coex_priority_type_uint8 en_mode);
extern oal_void hi1102_coex_sw_irq_set(oal_uint8 uc_irq_en);
extern oal_void hi1102_coex_sw_irq_status_get(oal_uint8 *uc_irq_status);
extern oal_void hi1102_get_btcoex_abort_qos_null_seq_num(oal_uint32 *ul_qosnull_seq_num);
extern oal_void hi1102_get_btcoex_occupied_period(oal_uint16 *ul_occupied_period);
extern oal_void hi1102_get_btcoex_pa_status(oal_uint32 *ul_pa_status);
extern oal_void hi1102_btcoex_wait_bt_release_pa(oal_uint16 ul_period);
extern oal_void hi1102_update_btcoex_btble_status(hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32 hi1102_btcoex_init(oal_void *p_arg);
extern oal_void hi1102_get_btcoex_statistic(oal_bool_enum_uint8 en_enable_abort_stat);
extern oal_uint32 hi1102_mpw_soc_write_reg(oal_uint32 ulQuryRegAddrTemp, oal_uint16 usQuryRegValueTemp);
extern oal_void hi1102_btcoex_update_ap_beacon_count(oal_uint32 *pul_beacon_count);
extern oal_void hi1102_btcoex_post_event(hal_to_dmac_device_stru *pst_hal_device,
                                         hal_dmac_misc_sub_type_enum_uint8 uc_sub_type);
extern oal_void hi1102_btcoex_have_small_ampdu(hal_to_dmac_device_stru *pst_hal_base_device,
                                               oal_uint32 *pul_have_ampdu);
extern oal_void hi1102_btcoex_process_bt_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_print);
extern oal_void hi1102_btcoex_get_ps_service_status(hal_to_dmac_device_stru *pst_hal_device,
                                                    hal_btcoex_ps_status_enum_uint8 *en_ps_status);
extern oal_void hi1102_btcoex_get_bt_sco_status(hal_to_dmac_device_stru *pst_hal_device,
                                                oal_bool_enum_uint8 *en_sco_status);
extern oal_void hi1102_btcoex_get_bt_acl_status(hal_to_dmac_device_stru *pst_hal_device,
                                                oal_bool_enum_uint8 *en_acl_status);
extern oal_uint32 hi1102_btcoex_sw_preempt_init(hal_to_dmac_device_stru *pst_hal_device);
#ifdef _PRE_WLAN_FEATURE_LTECOEX
extern oal_void hi1102_ltecoex_req_mask_ctrl(oal_uint16 req_mask_ctrl);
#endif
extern oal_void hi1102_set_btcoex_abort_null_buff_addr(oal_uint32 ul_abort_null_buff_addr);
extern oal_void hi1102_set_btcoex_abort_qos_null_seq_num(oal_uint32 ul_qosnull_seq_num);
extern oal_void hi1102_set_btcoex_hw_rx_priority_dis(oal_uint8 uc_hw_rx_prio_dis);
extern oal_void hi1102_set_btcoex_hw_priority_en(oal_uint8 uc_hw_prio_en);
extern oal_void hi1102_set_btcoex_occupied_period(oal_uint16 ul_occupied_period);
extern oal_void hi1102_btcoex_get_rf_control(oal_uint16 ul_occupied_period, oal_uint32 *pul_wlbt_mode_sel,
                                             oal_uint16 us_wait_cnt);
extern oal_void hi1102_set_btcoex_sw_all_abort_ctrl(oal_uint8 uc_sw_abort_ctrl);
extern oal_void hi1102_set_btcoex_sw_priority_flag(oal_uint8 uc_sw_prio_flag);
extern oal_void hi1102_set_btcoex_soc_gpreg0(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset);
extern oal_void hi1102_set_btcoex_soc_gpreg1(oal_uint8 uc_val, oal_uint16 us_mask, oal_uint8 uc_offset);
extern oal_void hi1102_btcoex_get_bt_status(oal_uint16 *pus_bt_status, oal_uint16 *pus_ble_status);
#endif
extern oal_void hi1102_tx_get_dscr_iv_word(hal_tx_dscr_stru *pst_dscr, oal_uint32 *pul_iv_ms_word,
                                           oal_uint32 *pul_iv_ls_word, oal_uint8 uc_chiper_type,
                                           oal_uint8 uc_chiper_keyid);
#ifdef _PRE_WLAN_DFT_STAT
extern oal_void hi1102_dft_get_machw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pst_machw_stat,
                                               oal_uint8 us_bank_select, oal_uint32 *pul_len);
extern oal_void hi1102_dft_set_phy_stat_node(hal_to_dmac_device_stru *pst_hal_device,
                                             oam_stats_phy_node_idx_stru *pst_phy_node_idx);
extern oal_void hi1102_dft_get_phyhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pst_phyhw_stat,
                                               oal_uint8 us_bank_select, oal_uint32 *pul_len);
extern oal_void hi1102_dft_get_rfhw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pst_rfhw_stat,
                                              oal_uint32 *pul_len);
extern oal_void hi1102_dft_get_sochw_stat_info(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pst_sochw_stat,
                                               oal_uint32 *pul_len);
extern oal_void hi1102_dft_print_machw_stat(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_dft_print_phyhw_stat(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_dft_print_rfhw_stat(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_dft_report_all_reg_state(hal_to_dmac_device_stru *pst_hal_device);

#endif
extern oal_void hi1102_set_lte_gpio_mode(oal_uint32 ul_mode_value);

extern oal_void hi1102_cfg_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device,
                                         oal_uint8 uc_chain_idx, wlan_channel_band_enum_uint8 en_band);
extern oal_void hi1102_get_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device,
                                         oal_uint8 uc_chain_idx, wlan_channel_band_enum_uint8 en_band);
extern oal_void hi1102_revert_cw_signal_reg(hal_to_dmac_device_stru *pst_hal_device,
                                            wlan_channel_band_enum_uint8 en_band);
extern oal_void hi1102_check_test_value_reg(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_value,
                                            oal_uint32 *pul_result);
extern oal_uint32 hi1102_rf_get_pll_div_idx(wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel_idx,
                                            wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                            oal_uint8 *puc_pll_div_idx);
extern oal_void hi1102_dscr_set_iv_value(hal_tx_dscr_stru *pst_tx_dscr, oal_uint32 ul_iv_ls_word,
                                         oal_uint32 ul_iv_ms_word);
extern oal_void hi1102_dscr_set_tx_pn_hw_bypass(hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 en_bypass);
extern oal_void hi1102_dscr_get_tx_pn_hw_bypass(hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 *pen_bypass);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
extern oal_int16 hi1102_get_tx_pdet_by_pow(hal_to_dmac_device_stru *OAL_CONST pst_hal_device,
                                           hal_pdet_info_stru *OAL_CONST pst_pdet_info,
                                           hal_dyn_cali_usr_record_stru *OAL_CONST pst_user_pow,
                                           oal_int16 *pst_exp_pdet);
extern oal_void hi1102_init_dyn_cali_tx_pow(hal_to_dmac_device_stru *pst_hal_device);

extern oal_void hi1102_rf_cali_realtime_entrance(hal_to_dmac_vap_stru *OAL_CONST pst_hal_vap,
                                                 hal_to_dmac_device_stru *OAL_CONST pst_hal_device,
                                                 hal_pdet_info_stru *OAL_CONST pst_pdet_info,
                                                 hal_dyn_cali_usr_record_stru *OAL_CONST pst_user_pow,
                                                 hal_tx_dscr_stru *OAL_CONST pst_base_dscr);
extern oal_uint32 hi1102_config_custom_dyn_cali(oal_uint8 *puc_param);
extern oal_void hi1102_tx_get_bw_mode(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr,
                                      wlan_bw_cap_enum_uint8 *pen_bw_mode);
extern oal_void hi1102_config_set_dyn_cali_dscr_interval(hal_to_dmac_device_stru *pst_hal_device,
                                                         wlan_channel_band_enum_uint8 uc_band, oal_uint16 us_param_val);
extern oal_void hi1102_rf_init_dyn_cali_reg_conf(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_tx_set_pdet_en(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                      oal_bool_enum_uint8 en_pdet_en_flag);
extern oal_void hi1102_tx_get_pdet_en(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                      oal_bool_enum_uint8 *pen_pdet_en_flag);
extern oal_uint32 hi1102_dyn_cali_vdet_val_amend(hal_to_dmac_device_stru *pst_hal_device,
                                                 hal_pdet_info_stru *pst_pdet_info);
#endif  // _PRE_WLAN_FIT_BASED_REALTIME_CALI
extern oal_void hi1102_get_target_tx_power_by_tx_dscr(hal_to_dmac_vap_stru *pst_hal_vap,
                                                      hal_to_dmac_device_stru *pst_hal_device,
                                                      hal_tx_dscr_stru *pst_tx_dscr,
                                                      hal_pdet_info_stru *pst_pdet_info,
                                                      oal_uint8 uc_channel_idx,
                                                      wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                                      oal_int16 *ps_tx_pow);

#ifdef _PRE_WLAN_FEATURE_FTM
extern oal_uint64 hi1102_get_ftm_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time);
extern oal_uint64 hi1102_check_ftm_t4(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time);
extern oal_int8 hi1102_get_ftm_t4_intp(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time);
extern oal_uint64 hi1102_check_ftm_t2(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time);
extern oal_int8 hi1102_get_ftm_t2_intp(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 ull_time);
extern oal_void hi1102_get_ftm_tod(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_tod);
extern oal_void hi1102_get_ftm_toa(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_toa);
extern oal_void hi1102_get_ftm_t2(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_t2);
extern oal_void hi1102_get_ftm_t3(hal_to_dmac_device_stru *pst_hal_device, oal_uint64 *pull_t3);
extern oal_void hi1102_get_ftm_ctrl_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_status);
extern oal_void hi1102_get_ftm_config_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_ftm_status);
extern oal_void hi1102_set_ftm_ctrl_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ftm_status);
extern oal_void hi1102_set_ftm_config_status(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_ftm_status);
extern oal_void hi1102_set_ftm_enable(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_ftm_status);
extern oal_void hi1102_set_ftm_sample(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_ftm_status);
extern oal_void hi1102_ftm_get_divider(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 *pul_divider);
extern oal_void hi1102_get_ftm_dialog(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_dialog);
extern oal_void hi1102_get_ftm_cali_rx_time(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_uint32 *pul_ftm_cali_rx_time);
extern oal_void hi1102_get_ftm_cali_rx_intp_time(hal_to_dmac_device_stru *pst_hal_device,
                                                 oal_uint32 *pul_ftm_cali_rx_time);
extern oal_void hi1102_get_ftm_cali_tx_time(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_uint32 *pul_ftm_cali_tx_time);
extern oal_void hi1102_set_ftm_cali(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                    oal_bool_enum_uint8 en_ftm_cali);
extern oal_void hi1102_set_ftm_tx_cnt(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                      oal_uint8 uc_ftm_tx_cnt);
extern oal_void hi1102_set_ftm_bandwidth(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                         wlan_bw_cap_enum_uint8 en_band_cap);
extern oal_void hi1102_set_ftm_protocol(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                        wlan_phy_protocol_enum_uint8 uc_prot_format);
extern oal_void hi1102_get_ftm_rtp_reg_ram(hal_to_dmac_device_stru *pst_hal_device, void **pp_ftm_ext_rpt_reg,
                                           oal_uint8 uc_session_index);
extern oal_void hi1102_get_csi_agc(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_ftm_rtp_reg(hal_to_dmac_device_stru *pst_hal_device,
                                       oal_uint32 *pul_reg0,
                                       oal_uint32 *pul_reg1,
                                       oal_uint32 *pul_reg2,
                                       oal_uint32 *pul_reg3,
                                       oal_uint32 *pul_reg4);
extern oal_void hi1102_set_tx_dscr_ftm_enable(hal_to_dmac_device_stru *pst_hal_device,
                                              hal_tx_dscr_stru *pst_tx_dscr, oal_bool_enum_uint8 en_ftm_cali);
#endif
extern oal_void hi1102_set_tx_rts_dup_enable(oal_bool_enum_uint8 en_status);
extern oal_void hi1102_set_rx_dyn_bw_select(oal_bool_enum_uint8 en_status);
extern oal_void hi1102_rx_non_ht_rsp_dup_enable(oal_bool_enum_uint8 en_status);
extern oal_void hi1102_rx_vht_rsp_dup_enable(oal_bool_enum_uint8 en_status);
extern oal_void hi1102_rx_ht_rsp_dup_enable(oal_bool_enum_uint8 en_status);
extern oal_void hi1102_set_rx_legacy_adj_val(oal_uint8 uc_time);
extern oal_void hi1102_set_rx_11b_long_adj_val(oal_uint8 uc_time);
extern oal_void hi1102_set_rx_11b_short_adj_val(oal_uint8 uc_time);
extern oal_void hi1102_set_rx_vht_adj_val(oal_uint8 uc_time);
extern oal_void hi1102_set_cca_th(oal_int8 c_pri_20_th, oal_int8 c_sec_20_th, oal_int8 c_sec_40_th);
extern oal_void hi1102_get_cca_th(oal_int8 *pc_th_val);
extern oal_void hi1102_set_cca_bimr_th(oal_uint8 uc_pri_20_th, oal_uint8 uc_sec_20_th, oal_uint8 uc_sec_40_th);
extern oal_void hi1102_get_cca_bimr_th(oal_int8 *pc_th_val);
extern oal_void hi1102_set_cca_mode(oal_uint32 ul_mode);
extern oal_void hi1102_get_cca_mode(oal_uint32 *pul_mode);
extern oal_void hi1102_set_idle_channel_statistics_mode(oal_uint32 ul_mode);
extern oal_void hi1102_get_idle_channel_statistics_mode(oal_uint32 *pul_mode);
extern oal_void hi1102_set_agc_cca_timeout(oal_uint32 ul_time);
extern oal_void hi1102_set_agc_cca_busy_bypass_mode(oal_uint32 ul_mode);
extern oal_void hi1102_set_rssi_fredomain_mode(oal_uint32 ul_mode);
extern oal_void hi1102_tpc_select_upc_level(oal_int16 *pas_upc_gain_table,
                                            oal_uint8 uc_upc_gain_table_len,
                                            hal_to_dmac_vap_stru *pst_hal_vap,
                                            oal_uint8 uc_start_chain);
extern oal_uint32 hi1102_rf_dev_init(oal_void *p_rf_dev);
extern oal_void hi1102_read_max_temperature(oal_int16 *ps_temperature);
extern oal_void hi1102_rf_dev_update_pwr_fit_para(hal_to_dmac_device_stru *pst_device, oal_uint8 uc_subband_idx,
                                                  oal_uint8 uc_pll_div_idx);

extern oal_void hi1102_tpc_store_phy_reg_upc_lut_param(hal_to_dmac_vap_stru *pst_hal_vap,
                                                       hal_to_dmac_device_stru *pst_hal_device,
                                                       oal_uint8 uc_subband_idx);

extern oal_void hi1102_get_sar_ctrl_params(wlan_channel_band_enum_uint8 en_band,
    oal_uint8 uc_channel_num, oal_uint8 *puc_sar_pwr);

extern oal_void hi1102_set_pow_to_tpc_code(hal_to_dmac_vap_stru *pst_hal_vap,
                                           hal_to_dmac_device_stru *pst_hal_dev,
                                           wlan_channel_band_enum_uint8 en_freq_band,
                                           oal_uint8 uc_cur_ch_num,
                                           wlan_channel_bandwidth_enum_uint8 en_bandwidth);

extern oal_void hi1102_adjust_pow_cali_upc_code_by_amend(hal_to_dmac_vap_stru *pst_hal_vap,
    hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_cur_ch_num);

extern oal_void hi1102_far_dis_is_need_gain_pwr(wlan_channel_band_enum_uint8 en_band,
    oal_uint8 uc_channel_num, oal_bool_enum_uint8 *pen_need_gain);
extern oal_void hi1102_get_gnss_status(oal_bool_enum_uint8 *pen_gnss_on);
extern oal_void hi1102_cfg_anti_intf(hal_to_dmac_vap_stru *pst_hal_vap, oal_int8 c_rssi);
extern oal_void hi1102_reset_anti_intf(oal_void);

#ifdef _PRE_WLAN_FEATURE_CSI
extern oal_void hi1102_set_csi_en(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_reg_value);
extern oal_void hi1102_set_csi_ta(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr);
extern oal_void hi1102_set_csi_ta_check(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_check_ta,
                                        oal_bool_enum_uint8 en_check_ftm, oal_uint8 *puc_addr);
extern oal_void hi1102_get_mac_csi_info(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_phy_csi_info(hal_to_dmac_device_stru *pst_hal_device,
                                        wlan_channel_bandwidth_enum_uint8 *pen_bandwidth, oal_uint8 *puc_frame_type);
extern oal_void hi1102_prepare_csi_sample_setup(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_pktmem_csi_bus_access(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_get_mac_csi_ta(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_addr);
extern oal_void hi1102_get_csi_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **puc_reg_num);
extern oal_void hi1102_get_pktmem_start_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **puc_reg_num);
extern oal_void hi1102_get_pktmem_end_addr(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 **pul_reg_num);
extern oal_void hi1102_get_csi_frame_type(hal_to_dmac_device_stru *pst_hal_device,
                                          wlan_channel_bandwidth_enum_uint8 *pen_bandwidth, oal_uint8 *puc_frame_type);
extern oal_void hi1102_free_csi_sample_mem(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_set_csi_memory(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_start_addr,
                                      oal_uint32 ul_reg_num);
extern oal_void hi1102_disable_csi_sample(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_clear_sample_state(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void hi1102_restart_csi_sample(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 uc_need_pktram);
#endif
extern oal_void hi1102_get_rts_cts_time_non_11b(oal_uint32 *pul_rts_cts_time);
extern oal_void hi1102_get_rts_cts_time_11b(oal_uint32 *pul_rts_cts_time);
extern oal_void hi1102_get_rts_fail_time_non_11b(oal_uint32 *pul_rts_fail_time);
extern oal_void hi1102_get_rts_fail_time_11b(oal_uint32 *pul_rts_fail_time);
extern oal_void hi1102_get_ack_ba_time_non_11b_ampdu(oal_uint32 *pul_ack_ba_time);
extern oal_void hi1102_get_ack_ba_time_non_11b_non_ampdu(oal_uint32 *pul_ack_ba_time);
extern oal_void hi1102_get_ack_ba_time_11b_non_ampdu(oal_uint32 *pul_ack_ba_time);
extern oal_void hi1102_get_vht_phy_time(oal_uint32 *pul_phy_time);
extern oal_void hi1102_get_ht_phy_time(oal_uint32 *pul_phy_time);
extern oal_void hi1102_get_legacy_phy_time(oal_uint32 *pul_phy_time);
extern oal_void hi1102_get_long_preamble_time(oal_uint32 *pul_phy_time);
extern oal_void hi1102_get_short_preamble_time(oal_uint32 *pul_phy_time);
extern oal_void hi1102_pm_set_tbtt_offset(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_adjust_val);
extern oal_void hi1102_init_pm_info(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_dyn_tbtt_offset_switch(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_switch);
extern oal_void hi1102_dyn_set_tbtt_offset_resv(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 uc_val);

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
extern oal_void hi1102_tbtt_offset_probe_init(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_suspend(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_resume(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_tbtt_cnt_incr(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_beacon_cnt_incr(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_state_init(hal_tbtt_offset_probe_stru *pst_probe);
extern oal_void hi1102_tbtt_offset_probe_state_start(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_state_up_done(hal_to_dmac_vap_stru *pst_hal_vap);
extern oal_void hi1102_tbtt_offset_probe_state_end(hal_to_dmac_vap_stru *pst_hal_vap);

#endif
extern oal_void hi1102_set_agc_target_adjust(hal_to_dmac_device_stru *pst_hal_device,
                                             hal_phy_agc_target_adjust_enmu_uint8 en_agc_target_adjust_type);
extern oal_void hi1102_set_cca_prot_th(hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_ed_low_th_dsss_reg_val,
                                       oal_int8 c_ed_low_th_ofdm_reg_val);
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
extern oal_void hi1102_report_gm_val(hal_to_dmac_device_stru *pst_hal_device);
#endif
extern oal_void hi1102_pow_cfg_show_log(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 *puc_rate_idx);
#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
extern oal_void hi1102_set_extlna_threshold(hal_to_dmac_device_stru *pst_hal_device,
                                            oal_bool_enum_uint8 en_disable_dyn_extlna_bypass);
extern oal_void hi1102_set_dyn_bypass_extlna_pm_flag(oal_bool_enum_uint8 en_value);
extern oal_bool_enum_uint8 hi1102_get_dyn_bypass_extlna_pm_flag(oal_void);
extern oal_void hi1102_set_dyn_bypass_extlna_enable(oal_uint8 uc_dyn_bypass_extlna_enable);
extern oal_uint8 hi1102_get_dyn_bypass_extlna_enable(oal_void);
#endif
extern oal_void hi1102_clear_user_ptk_key(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_lut_idx);
#ifdef _PRE_BT_FITTING_DATA_COLLECT
extern oal_int32 bt_init_env(oal_void);
extern oal_void bt_cfg_write_txup_cali(oal_uint8 uc_freq);
extern oal_void bt_cfg_txpwr_upc_code(oal_uint8 freq);
extern oal_void bt_print_gm(oal_void);
#endif
extern oal_void hi1102_set_ddc_en(oal_bool_enum_uint8 en_ddc_enable);
#endif
void hi1102_enable_tsf_tbtt(hal_to_dmac_vap_stru *pst_hal_vap, oal_bool_enum_uint8 en_dbac_enable);
#ifdef _PRE_WLAN_CHBA_MGMT
void hi1102_chba_set_sync_filter(hal_to_dmac_vap_stru *pst_hal_vap, uint8_t enable);
void hi1102_chba_set_tbtt_intr(hal_to_dmac_vap_stru *pst_hal_vap, uint8_t enable);
#endif
oal_void  hi1102_get_ch_measurement_result_ram(hal_to_dmac_device_stru *pst_hal_device,
    hal_ch_statics_irq_event_stru *pst_ch_statics);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


