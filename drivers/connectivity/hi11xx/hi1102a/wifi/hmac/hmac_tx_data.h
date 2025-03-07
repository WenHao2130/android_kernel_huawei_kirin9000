

#ifndef __HMAC_TX_DATA_H__
#define __HMAC_TX_DATA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_frame.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_mgmt_classifier.h"
#include "mac_resource.h"
#include "oal_main.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_function.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_DATA_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 基本能力信息中关于是否是QOS的能力位 */
#define HMAC_CAP_INFO_QOS_MASK 0x0200

#define wlan_tos_to_tid(_tos) (\
    (((_tos) == 0) || ((_tos) == 3)) ? WLAN_TIDNO_BEST_EFFORT : (((_tos) == 1) || ((_tos) == 2)) ? \
    WLAN_TIDNO_BACKGROUND : (((_tos) == 4) || ((_tos) == 5)) ? WLAN_TIDNO_VIDEO : WLAN_TIDNO_VOICE)

#define WLAN_BA_CNT_INTERVAL 100

/* 第一档超时TIMEOUT * CNT 必须大于100ms，否则丢包率高,体验差 */
#define HMAC_PSM_TIMER_MIDIUM_CNT 10
#define HMAC_PSM_TIMER_BUSY_CNT   20

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_TX_BSS_NOQOS = 0,
    HMAC_TX_BSS_QOS = 1,

    HMAC_TX_BSS_QOS_BUTT
} hmac_tx_bss_qos_type_enum;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_bool_enum_uint8 g_en_force_pass_filter;
#ifdef _PRE_WLAN_FEATURE_APF
extern oal_uint16 g_us_apf_program_len;
#endif

/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    oal_uint8 uc_ini_en;     /* 定制化根据吞吐动态bypass extLNA开关 */
    oal_uint8 uc_cur_status; /* 当前是否为低功耗测试状态 */
    oal_uint16 us_throughput_high;
    oal_uint16 us_throughput_low;
    oal_uint16 us_resv;
} mac_rx_dyn_bypass_extlna_stru;
extern mac_rx_dyn_bypass_extlna_stru g_st_rx_dyn_bypass_extlna_switch;

typedef struct {
    /* 定制化小包amsdu开关 */
    oal_uint8 uc_ini_small_amsdu_en;
    oal_uint8 uc_cur_small_amsdu_en;
    oal_uint16 us_small_amsdu_throughput_high;
    oal_uint16 us_small_amsdu_throughput_low;
    oal_uint16 us_small_amsdu_pps_high;
    oal_uint16 us_small_amsdu_pps_low;
    oal_uint16 us_resv;
} mac_small_amsdu_switch_stru;
extern mac_small_amsdu_switch_stru g_st_small_amsdu_switch;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
extern mac_tx_large_amsdu_ampdu_stru *mac_get_tx_large_amsdu_addr(oal_void);
#endif
extern oal_uint8 wlan_pm_get_fast_check_cnt(void);
extern void wlan_pm_set_fast_check_cnt(oal_uint8 fast_check_cnt);
extern oal_uint32 hmac_tx_encap(hmac_vap_stru *pst_vap,
                                hmac_user_stru *pst_user,
                                oal_netbuf_stru *pst_buf);
extern oal_uint32 hmac_tx_ucast_process(hmac_vap_stru *pst_vap,
                                        oal_netbuf_stru *pst_buf,
                                        hmac_user_stru *pst_user,
                                        mac_tx_ctl_stru *pst_tx_ctl);

extern oal_void hmac_tx_ba_setup(hmac_vap_stru *pst_vap,
                                 hmac_user_stru *pst_user,
                                 oal_uint8 uc_tidno);

extern oal_void hmac_tx_ba_cnt_vary(hmac_vap_stru *pst_hmac_vap,
                                    hmac_user_stru *pst_hmac_user,
                                    oal_uint8 uc_tidno,
                                    oal_netbuf_stru *pst_buf);

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
extern oal_uint8 hmac_tx_wmm_acm(oal_bool_enum_uint8 en_wmm, hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_tid);
#endif /* defined(_PRE_PRODUCT_ID_HI110X_HOST) */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
extern oal_void hmac_tx_amsdu_ampdu_switch(oal_uint32 ul_tx_throughput_mbps);
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
extern oal_void hmac_tx_tcp_ack_buf_switch(oal_uint32 ul_rx_throughput_mbps);
#endif
extern oal_void hmac_rx_dyn_bypass_extlna_switch(oal_uint32 ul_tx_throughput_mbps,
    oal_uint32 ul_rx_throughput_mbps);
extern oal_void hmac_tx_small_amsdu_switch(oal_uint32 ul_rx_throughput_mbps, oal_uint32 ul_tx_pps);
extern oal_void hmac_set_psm_activity_timer(oal_uint32 ul_total_pps);
extern oal_void hmac_auto_set_apf_switch_in_suspend(oal_uint32 ul_total_pps);
#ifdef _PRE_WLAN_CHBA_MGMT
extern oal_bool_enum_uint8 hwifi_get_chba_en(void);
#endif
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_netbuf_stru *hmac_tx_get_next_mpdu(oal_netbuf_stru *pst_buf, oal_uint8 uc_netbuf_num)
{
    oal_netbuf_stru *pst_next_buf = OAL_PTR_NULL;
    oal_uint32 ul_netbuf_index;

    if (oal_unlikely(pst_buf == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    pst_next_buf = pst_buf;
    for (ul_netbuf_index = 0; ul_netbuf_index < uc_netbuf_num; ul_netbuf_index++) {
        pst_next_buf = oal_netbuf_list_next(pst_next_buf);
    }

    return pst_next_buf;
}


OAL_STATIC OAL_INLINE oal_void hmac_tx_netbuf_list_enqueue(oal_netbuf_head_stru *pst_head,
                                                           oal_netbuf_stru *pst_buf, oal_uint8 uc_netbuf_num)
{
    oal_uint32 ul_netbuf_index;
    oal_netbuf_stru *pst_buf_next = OAL_PTR_NULL;

    if (oal_unlikely((pst_head == OAL_PTR_NULL) || (pst_buf == OAL_PTR_NULL))) {
        return;
    }

    for (ul_netbuf_index = 0; ul_netbuf_index < uc_netbuf_num; ul_netbuf_index++) {
        pst_buf_next = oal_netbuf_list_next(pst_buf);
        oal_netbuf_add_to_list_tail(pst_buf, pst_head);
        pst_buf = pst_buf_next;
    }
}


OAL_STATIC OAL_INLINE oal_void hmac_tx_get_addr(mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr,
                                                oal_uint8 *puc_saddr,
                                                oal_uint8 *puc_daddr)
{
    oal_uint8 uc_to_ds;
    oal_uint8 uc_from_ds;

    uc_to_ds = mac_hdr_get_to_ds((oal_uint8 *)pst_hdr);
    uc_from_ds = mac_hdr_get_from_ds((oal_uint8 *)pst_hdr);
    if ((uc_to_ds == 1) && (uc_from_ds == 0)) {
        /* to AP */
        oal_set_mac_addr(puc_saddr, pst_hdr->auc_address2);
        oal_set_mac_addr(puc_daddr, pst_hdr->auc_address3);
    } else if ((uc_to_ds == 0) && (uc_from_ds == 0)) {
        /* IBSS */
        oal_set_mac_addr(puc_saddr, pst_hdr->auc_address2);
        oal_set_mac_addr(puc_daddr, pst_hdr->auc_address1);
    } else if ((uc_to_ds == 1) && (uc_from_ds == 1)) {
        /* WDS */
        oal_set_mac_addr(puc_saddr, pst_hdr->auc_address4);
        oal_set_mac_addr(puc_daddr, pst_hdr->auc_address3);
    } else {
        /* From AP */
        oal_set_mac_addr(puc_saddr, pst_hdr->auc_address3);
        oal_set_mac_addr(puc_daddr, pst_hdr->auc_address1);
    }
}


OAL_STATIC OAL_INLINE oal_void hmac_tx_set_frame_ctrl(oal_uint32 ul_qos,
                                                      mac_tx_ctl_stru *pst_tx_ctl,
                                                      mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr_addr4)
{
    mac_ieee80211_qos_htc_frame_stru *pst_hdr = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_is_amsdu;
    if (ul_qos == HMAC_TX_BSS_QOS) {
        if (pst_tx_ctl->uc_netbuf_num == 1) {
            en_is_amsdu = OAL_FALSE;
        } else {
            en_is_amsdu = pst_tx_ctl->en_is_amsdu;
        }

        /* 设置帧控制字段 */
        mac_hdr_set_frame_control((oal_uint8 *)pst_hdr_addr4, (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA));

        /* 更新帧头长度 */
        if (pst_tx_ctl->en_use_4_addr == OAL_FALSE) {
            pst_hdr = (mac_ieee80211_qos_htc_frame_stru *)pst_hdr_addr4;
            /* 设置QOS控制字段 */
            pst_hdr->bit_qc_tid = pst_tx_ctl->uc_tid;
            pst_hdr->bit_qc_eosp = 0;
            pst_hdr->bit_qc_ack_polocy = pst_tx_ctl->en_ack_policy;
            pst_hdr->bit_qc_amsdu = en_is_amsdu;
            pst_hdr->qos_control.bit_qc_txop_limit = 0;
            pst_tx_ctl->uc_frame_header_length = MAC_80211_QOS_FRAME_LEN;
        } else {
            /* 设置QOS控制字段 */
            pst_hdr_addr4->bit_qc_tid = pst_tx_ctl->uc_tid;
            pst_hdr_addr4->bit_qc_eosp = 0;
            pst_hdr_addr4->bit_qc_ack_polocy = pst_tx_ctl->en_ack_policy;
            pst_hdr_addr4->bit_qc_amsdu = en_is_amsdu;
            pst_hdr_addr4->qos_control.qc_txop_limit = 0;
            pst_tx_ctl->uc_frame_header_length = MAC_80211_QOS_4ADDR_FRAME_LEN;
        }

        /* 由DMAC考虑是否需要HTC */
    } else {
        /* 设置帧控制字段 */
        mac_hdr_set_frame_control((oal_uint8 *)pst_hdr_addr4, WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_DATA);

        /* 非QOS数据帧帧控制字段设置 */
        if (pst_tx_ctl->en_use_4_addr) {
            pst_tx_ctl->uc_frame_header_length = MAC_80211_4ADDR_FRAME_LEN;
        } else {
            pst_tx_ctl->uc_frame_header_length = MAC_80211_FRAME_LEN;
        }
    }
}

#ifdef _PRE_WLAN_CHBA_MGMT

static void hmac_set_chba_mac_addr(hmac_vap_stru *hmac_vap, oal_uint8 *puc_daddr,
    mac_ieee80211_qos_htc_frame_addr4_stru *mac_hdr)
{
    hmac_chba_vap_stru *chba_vap_info = NULL;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t *bssid = auc_bssid;

    chba_vap_info = hmac_vap->hmac_chba_vap_info;
    if (chba_vap_info == NULL) {
        oam_warning_log0(0, 0, "CHBA: hmac_set_chba_mac_addr: chba_vap_info is null.");
        return;
    }
    oal_set_mac_addr(bssid, hmac_vap->st_vap_base_info.auc_bssid);
    /* From DS标识位设置 */
    mac_hdr_set_from_ds((oal_uint8 *)mac_hdr, 0);
    /* to DS标识位设置 */
    mac_hdr_set_to_ds((oal_uint8 *)mac_hdr, 0);
    /* Set Address1 field in the WLAN Header with destination address */
    oal_set_mac_addr(mac_hdr->auc_address1, (oal_uint8 *)puc_daddr);
    /* Set Address2 field in the WLAN Header with the StationID */
    oal_set_mac_addr(mac_hdr->auc_address2, mac_mib_get_StationID(&hmac_vap->st_vap_base_info));
    /* Set Address3 field in the WLAN Header with the BSSID */
    oal_set_mac_addr(mac_hdr->auc_address3, bssid);
}
#endif

OAL_STATIC OAL_INLINE oal_uint32 hmac_tx_set_addresses(hmac_vap_stru *pst_vap,
                                                       hmac_user_stru *pst_user,
                                                       mac_tx_ctl_stru *pst_tx_ctl,
                                                       oal_uint8 *puc_saddr,
                                                       oal_uint8 *puc_daddr,
                                                       mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr,
                                                       oal_uint16 us_ether_type)
{
    /* 分片号置成0，后续分片特性需要重新赋值 */
    pst_hdr->bit_frag_num = 0;
    pst_hdr->bit_seq_num = 0;
#ifdef _PRE_WLAN_CHBA_MGMT
    /* chba组网下设置帧mac地址，tods和from ds暂时都设置为0，bssid设置为固定地址 */
    if ((hwifi_get_chba_en() == OAL_TRUE) &&
        (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (((mac_vap_rom_stru *)(pst_vap->st_vap_base_info._rom))->chba_mode == CHBA_MODE)) {
        hmac_set_chba_mac_addr(pst_vap, puc_daddr, pst_hdr);
        return OAL_SUCC;
    }
#endif
    /* From AP */
    if ((pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP)
        && (!(pst_tx_ctl->en_use_4_addr))) {
        /* From DS标识位设置 */
        mac_hdr_set_from_ds((oal_uint8 *)pst_hdr, 1);

        /* to DS标识位设置 */
        mac_hdr_set_to_ds((oal_uint8 *)pst_hdr, 0);

        /* Set Address1 field in the WLAN Header with destination address */
        oal_set_mac_addr(pst_hdr->auc_address1, puc_daddr);

        /* Set Address2 field in the WLAN Header with the BSSID */
        oal_set_mac_addr(pst_hdr->auc_address2, pst_vap->st_vap_base_info.auc_bssid);

        /* AMSDU情况，地址3填写BSSID */
        if (pst_tx_ctl->en_is_amsdu) {
            /* Set Address3 field in the WLAN Header with the BSSID */
            oal_set_mac_addr(pst_hdr->auc_address3, pst_vap->st_vap_base_info.auc_bssid);
        } else {
            /* Set Address3 field in the WLAN Header with the source address */
            oal_set_mac_addr(pst_hdr->auc_address3, puc_saddr);
        }
    } else if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* From DS标识位设置 */
        mac_hdr_set_from_ds((oal_uint8 *)pst_hdr, 0);

        /* to DS标识位设置 */
        mac_hdr_set_to_ds((oal_uint8 *)pst_hdr, 1);

        /* Set Address1 field in the WLAN Header with BSSID */
        oal_set_mac_addr(pst_hdr->auc_address1, pst_user->st_user_base_info.auc_user_mac_addr);

        if (us_ether_type == oal_byteorder_host_to_net_uint16(ETHER_LLTD_TYPE)) {
            /* Set Address2 field in the WLAN Header with the source address */
            oal_set_mac_addr(pst_hdr->auc_address2, puc_saddr);
        } else {
            /* Set Address2 field in the WLAN Header with the source address */
            oal_set_mac_addr(pst_hdr->auc_address2,
                pst_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);
        }
        /* AMSDU情况，地址3填写BSSID */
        if (pst_tx_ctl->en_is_amsdu) {
            /* Set Address3 field in the WLAN Header with the BSSID */
            oal_set_mac_addr(pst_hdr->auc_address3, pst_user->st_user_base_info.auc_user_mac_addr);
        } else {
            /* Set Address3 field in the WLAN Header with the destination address */
            oal_set_mac_addr(pst_hdr->auc_address3, puc_daddr);
        }
    } else { /* WDS */
        if (oal_unlikely(pst_user == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_set_addresses::pst_user null}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* TO DS标识位设置 */
        mac_hdr_set_to_ds((oal_uint8 *)pst_hdr, 1);

        /* From DS标识位设置 */
        mac_hdr_set_from_ds((oal_uint8 *)pst_hdr, 1);

        /* 地址1是 RA */
        oal_set_mac_addr(pst_hdr->auc_address1, pst_user->st_user_base_info.auc_user_mac_addr);

        /* 地址2是 TA (当前只有BSSID) */
        oal_set_mac_addr(pst_hdr->auc_address2, pst_vap->st_vap_base_info.auc_bssid);

        /* AMSDU情况，地址3和地址4填写BSSID */
        if (pst_tx_ctl->en_is_amsdu) {
            /* 地址3是 BSSID */
            oal_set_mac_addr(pst_hdr->auc_address3, pst_vap->st_vap_base_info.auc_bssid);

            /* 地址4也是 BSSID */
            oal_set_mac_addr(pst_hdr->auc_address4, pst_vap->st_vap_base_info.auc_bssid);
        } else {
            /* 地址3是 DA */
            oal_set_mac_addr(pst_hdr->auc_address3, puc_daddr);

            /* 地址4是 SA */
            oal_set_mac_addr(pst_hdr->auc_address4, puc_saddr);
        }
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_vap_ba_is_setup(hmac_user_stru *pst_hmac_user, oal_uint8 uc_tidno)
{
    if ((oal_unlikely(pst_hmac_user == OAL_PTR_NULL)) || (uc_tidno >= WLAN_TID_MAX_NUM)) {
        return OAL_FALSE;
    }
    return (pst_hmac_user->ast_tid_info[uc_tidno].st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) ?
            OAL_TRUE : OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tid_need_ba_session(hmac_vap_stru *pst_hmac_vap,
                                                                   hmac_user_stru *pst_hmac_user,
                                                                   oal_uint8 uc_tidno,
                                                                   oal_netbuf_stru *pst_buf)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_tid_stru *pst_hmac_tid_info = OAL_PTR_NULL;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */

    if (hmac_vap_ba_is_setup(pst_hmac_user, uc_tidno) == OAL_TRUE) {
        if (pst_hmac_vap->en_ampdu_tx_on_switch == OAL_FALSE) {
            st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
            st_action_args.uc_action = MAC_BA_ACTION_DELBA;
            st_action_args.ul_arg1 = uc_tidno;
            st_action_args.ul_arg2 = MAC_ORIGINATOR_DELBA;
            st_action_args.ul_arg3 = MAC_UNSPEC_REASON;
            st_action_args.puc_arg5 = pst_hmac_user->st_user_base_info.auc_user_mac_addr;
            hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);
        }
        return OAL_FALSE;
    }

    /* 配置命令不允许建立聚合时返回 */
    if (pst_hmac_vap->en_ampdu_tx_on_switch == OAL_FALSE) {
        return OAL_FALSE;
    }

#ifdef _PRE_WLAN_NARROW_BAND
    if (g_hitalk_status & NARROW_BAND_ON_MASK) {
        return OAL_FALSE;
    }
#endif

    if (hmac_user_xht_support(pst_hmac_user) == OAL_FALSE) {
        return OAL_FALSE;
    }
    if (pst_hmac_vap->en_addba_mode != HMAC_ADDBA_MODE_AUTO) {
        return OAL_FALSE;
    }

    /* 针对VO业务, 根据VAP标志位确定是否建立BA会话 */
    if ((WLAN_WME_TID_TO_AC(uc_tidno) == WLAN_WME_AC_VO) &&
        (pst_hmac_vap->st_vap_base_info.bit_voice_aggr == OAL_FALSE)) {
        return OAL_FALSE;
    }

    /* 判断HMAC VAP的是否支持聚合 */
    if (!((pst_hmac_vap->en_tx_aggr_on) || (pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_rifs_tx_on))) {
        oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_tid_need_ba_session::en_tx_aggr_on of vap is off");
        return OAL_FALSE;
    }

    pst_hmac_tid_info = &(pst_hmac_user->ast_tid_info[uc_tidno]);
    if (pst_hmac_tid_info->st_ba_tx_info.en_ba_switch != OAL_TRUE) {
        oam_info_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_tid_need_ba_session::en_tx_aggr_on of tid[%d] is off", uc_tidno);
        return OAL_FALSE;
    }

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                       "{hmac_tid_need_ba_session::pst_mac_dev null");
        return OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    if (pst_hmac_vap->uc_tx_ba_session_num >= WLAN_MAX_TX_BA) {
        oam_info_log1(0, OAM_SF_BA, "{hmac_tid_need_ba_session::uc_tx_ba_session_num[%d] exceed spec",
                      pst_hmac_vap->uc_tx_ba_session_num);
        return OAL_FALSE;
    }
#else
    if (pst_mac_device->uc_tx_ba_session_num >= WLAN_MAX_TX_BA) {
        oam_info_log1(0, OAM_SF_BA, "{hmac_tid_need_ba_session::uc_tx_ba_session_num[%d] exceed spec",
                      pst_mac_device->uc_tx_ba_session_num);
        return OAL_FALSE;
    }
#endif
    /* 需要先发送5个单播帧，再进行BA会话的建立 */
    if ((pst_hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_TRUE) &&
        (pst_hmac_user->auc_ba_flag[uc_tidno] < DMAC_UCAST_FRAME_TX_COMP_TIMES) &&
        (pst_hmac_vap->en_addba_mode == HMAC_ADDBA_MODE_AUTO)) {
        oam_info_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_tid_need_ba_session::auc_ba_flag[%d]}", pst_hmac_user->auc_ba_flag[uc_tidno]);
        hmac_tx_ba_cnt_vary(pst_hmac_vap, pst_hmac_user, uc_tidno, pst_buf);
        return OAL_FALSE;
    } else if (pst_hmac_user->st_user_base_info.st_cap_info.bit_qos == OAL_FALSE) {
        oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{UnQos Frame pass!!}");
        return OAL_FALSE;
    }

    if ((pst_hmac_tid_info->st_ba_tx_info.en_ba_status == DMAC_BA_INIT)
        && (pst_hmac_tid_info->st_ba_tx_info.uc_addba_attemps < HMAC_ADDBA_EXCHANGE_ATTEMPTS)) {
        pst_hmac_tid_info->st_ba_tx_info.en_ba_status = DMAC_BA_INPROGRESS;
        pst_hmac_tid_info->st_ba_tx_info.uc_addba_attemps++;
    } else {
        oam_info_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA,
                      "{hmac_tid_need_ba_session::addba_attemps[%d] of tid[%d] is COMPLETE}",
                      pst_hmac_tid_info->st_ba_tx_info.uc_addba_attemps, uc_tidno);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of hmac_tx_data.h */