

#ifndef __HMAC_CHAN_MGMT_H__
#define __HMAC_CHAN_MGMT_H__

/* 1 其他头文件包含 */
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_H
/* 2 宏定义 */
#define HMAC_MAX_20M_SUB_CH 4 /* VHT80中，20MHz信道总个数 */

/* 3 枚举定义 */
typedef enum {
    HMAC_OP_ALLOWED = BIT0,
    HMAC_SCA_ALLOWED = BIT1,
    HMAC_SCB_ALLOWED = BIT2,
} hmac_chan_op_enum;
typedef oal_uint8 hmac_chan_op_enum_uint8;

typedef enum {
    HMAC_NETWORK_SCA = 0,
    HMAC_NETWORK_SCB = 1,

    HMAC_NETWORK_BUTT,
} hmac_network_type_enum;
typedef oal_uint8 hmac_network_type_enum_uint8;

typedef enum {
    MAC_CHNL_AV_CHK_NOT_REQ = 0,  /* 不需要进行信道扫描 */
    MAC_CHNL_AV_CHK_IN_PROG = 1,  /* 正在进行信道扫描 */
    MAC_CHNL_AV_CHK_COMPLETE = 2, /* 信道扫描已完成 */

    MAC_CHNL_AV_CHK_BUTT,
} mac_chnl_av_chk_enum;
typedef oal_uint8 mac_chnl_av_chk_enum_uint8;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    oal_uint16                 aus_num_networks[HMAC_NETWORK_BUTT];
    hmac_chan_op_enum_uint8    en_chan_op;
    oal_uint8                  auc_resv[3];
} hmac_eval_scan_report_stru;

typedef struct {
    oal_uint8     uc_idx;     /* 信道索引号 */
    oal_uint16    us_freq;    /* 信道频点 */
    oal_uint8     auc_resv;
} hmac_dfs_channel_info_stru;

typedef struct {
    oal_uint16  us_freq;        /* 中心频率，单位MHz */
    oal_uint8   uc_number;      /* 信道号 */
    oal_uint8   uc_idx;         /* 信道索引(软件用) */
} mac_freq_channel_map_stru;

typedef struct {
    oal_uint32                   ul_channels;
    mac_freq_channel_map_stru    ast_channels[HMAC_MAX_20M_SUB_CH];
} hmac_channel_list_stru;

/* 4 全局变量声明 */
extern OAL_CONST mac_freq_channel_map_stru g_ast_freq_map_5g[MAC_CHANNEL_FREQ_5_BUTT];
extern OAL_CONST mac_freq_channel_map_stru g_ast_freq_map_2g[MAC_CHANNEL_FREQ_2_BUTT];

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_void hmac_chan_reval_status(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, oal_uint32 ul_change);
extern oal_void hmac_chan_disable_machw_tx(mac_vap_stru *pst_mac_vap);
extern oal_void hmac_chan_enable_machw_tx(mac_vap_stru *pst_mac_vap);
extern oal_void hmac_chan_multi_switch_to_20MHz_ap(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap,
                                                   oal_uint8 uc_channel,
                                                   wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern mac_chnl_av_chk_enum_uint8 hmac_chan_do_channel_availability_check(mac_device_stru *pst_mac_device,
                                                                          mac_vap_stru *pst_mac_vap,
                                                                          oal_bool_enum_uint8 en_first_time);
extern oal_uint32 hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap);
extern oal_uint32 hmac_chan_select_channel_for_operation(mac_vap_stru *pst_mac_vap,
                                                         oal_uint8 *puc_new_channel,
                                                         wlan_channel_bandwidth_enum_uint8 *pen_new_bandwidth);
extern oal_void hmac_chan_get_ext_chan_info(oal_uint8 uc_pri20_channel_idx,
                                            wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                                            hmac_channel_list_stru *pst_chan_info);
extern oal_void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap);
extern oal_void hmac_chan_multi_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
                                                      oal_uint8 uc_channel,
                                                      wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void hmac_dfs_set_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel);

extern oal_uint32 hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem);
extern oal_void hmac_chan_sync(mac_vap_stru *pst_mac_vap,
                               oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                               oal_bool_enum_uint8 en_switch_immediately);
extern oal_uint32 hmac_dbac_status_notify(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 hmac_chan_start_bss(hmac_vap_stru *pst_hmac_vap,
                                      mac_channel_stru *pst_channel,
                                      wlan_protocol_enum_uint8 en_protocol);
extern oal_void hmac_send_ht_notify_chan_width(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_data);
extern oal_void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
                                                         oal_uint8 uc_channel,
                                                         wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern uint32_t hmac_report_channel_switch(hmac_vap_stru *hmac_vap, mac_channel_stru *channel_info);
extern uint16_t mac_get_center_freq1_from_freq_and_bandwidth(uint16_t freq,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);

/* 11 inline函数定义 */

OAL_STATIC OAL_INLINE oal_void hmac_chan_initiate_switch_to_20mhz_ap(mac_vap_stru *pst_mac_vap)
{
    /* 设置VAP带宽模式为20MHz */
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = WLAN_BW_SWITCH_40_TO_20;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_scan_availability(mac_device_stru *pst_mac_device,
                                                                      mac_ap_ch_info_stru *pst_channel_info)
{
    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_is_ch_op_allowed(
    hmac_eval_scan_report_stru *pst_chan_scan_report, oal_uint8 uc_chan_idx)
{
    if (pst_chan_scan_report[uc_chan_idx].en_chan_op & HMAC_OP_ALLOWED) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_chan_mgmt.h */
