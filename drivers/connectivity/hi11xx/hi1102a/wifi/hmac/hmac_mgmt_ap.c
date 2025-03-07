

/* 1 头文件包含 */
#include "oal_cfg80211.h"
#include "oam_ext_if.h"
#include "hmac_mgmt_ap.h"
#include "hmac_encap_frame.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_rx_data.h"
#include "hmac_uapsd.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_sta.h"
#include "mac_ie.h"
#include "mac_user.h"
#include "hmac_user.h"
#include "hmac_11i.h"
#include "hmac_protection.h"
#include "hmac_chan_mgmt.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "hmac_204080_coexist.h"
#include "hmac_wpa_wpa2.h"
#include "hmac_support_pmf.h"
#include "hmac_location_ram.h"
#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
#include "hmac_custom_security.h"
#endif
#include "hmac_p2p.h"
#include "hmac_blockack.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "hmac_ext_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "plat_pm_wlan.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_AP_C

/* 2 全局变量定义 */
OAL_STATIC oal_uint32 hmac_ap_up_update_sta_sup_rates(oal_uint8 *puc_payload,
                                                      hmac_user_stru *pst_hmac_user,
                                                      mac_status_code_enum_uint16 *pen_status_code,
                                                      oal_uint32 ul_msg_len,
                                                      oal_uint16 us_offset,
                                                      oal_uint8 *puc_num_rates,
                                                      oal_uint16 *pus_msg_idx);
OAL_STATIC oal_uint32 hmac_ap_prepare_assoc_req(hmac_user_stru *pst_hmac_user,
                                                oal_uint8 *puc_payload,
                                                oal_uint32 ul_payload_len,
                                                oal_uint8 uc_mgmt_frm_type);

/* 3 函数实现 */

oal_void hmac_handle_disconnect_rsp_ap(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user)
{
    mac_device_stru *pst_mac_device;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_handle_disconnect_rsp_ap::pst_mac_device null.}");
        return;
    }

    /* 抛扫描完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(WLAN_MAC_ADDR_LEN);
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_handle_disconnect_rsp_ap::pst_event_mem null.}");
        return;
    }

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP,
                       WLAN_MAC_ADDR_LEN,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* 去关联的STA mac地址 */
    if (memcpy_s(frw_get_event_payload(pst_event_mem), WLAN_MAC_ADDR_LEN,
                 (oal_uint8 *)pst_hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_handle_disconnect_rsp_ap::memcpy fail!}");
        frw_event_free_m(pst_event_mem);
        return;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);
}


oal_void hmac_handle_connect_rsp_ap(hmac_vap_stru *pst_hmac_vap,
                                    hmac_user_stru *pst_hmac_user)
{
    mac_device_stru *pst_mac_device;
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    hmac_asoc_user_req_ie_stru *pst_asoc_user_req_info = OAL_PTR_NULL;
#endif

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_handle_connect_rsp_ap::pst_mac_device null.}");
        return;
    }

    /* 抛关联一个新的sta完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(WLAN_MAC_ADDR_LEN);
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_handle_connect_rsp_ap::pst_event_mem null.}");
        return;
    }

    /* 填写事件 */
    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP,
                       WLAN_MAC_ADDR_LEN,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(pst_event->auc_event_data);

    /* 上报内核的关联sta发送的关联请求帧ie信息 */
    pst_asoc_user_req_info->puc_assoc_req_ie_buff = pst_hmac_user->puc_assoc_req_ie_buff;
    pst_asoc_user_req_info->ul_assoc_req_ie_len = pst_hmac_user->ul_assoc_req_ie_len;

    /* 关联的STA mac地址 */
    memcpy_s((oal_uint8 *)pst_asoc_user_req_info->auc_user_mac_addr, WLAN_MAC_ADDR_LEN,
             pst_hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
#else
    /* 去关联的STA mac地址 */
    memcpy_s((oal_uint8 *)pst_event->auc_event_data, WLAN_MAC_ADDR_LEN,
             pst_hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

#endif

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);
}


OAL_STATIC oal_void hmac_mgmt_update_auth_mib(hmac_vap_stru *pst_hmac_vap,
                                              oal_netbuf_stru *pst_auth_rsp)
{
    oal_uint16 us_status;
    oal_uint8 auc_addr1[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_uint8 *puc_mac_header = oal_netbuf_header(pst_auth_rsp);

    us_status = mac_get_auth_status(puc_mac_header);

    mac_get_address1(puc_mac_header, auc_addr1, WLAN_MAC_ADDR_LEN);

    if (us_status != MAC_SUCCESSFUL_STATUSCODE) {
        mac_mib_set_AuthenticateFailStatus(&pst_hmac_vap->st_vap_base_info, us_status);
        mac_mib_set_AuthenticateFailStation(&pst_hmac_vap->st_vap_base_info, auc_addr1, WLAN_MAC_ADDR_LEN);

        /* DEBUG */
        oam_info_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                      "{hmac_mgmt_update_auth_mib::Authentication of STA Failed.Status Code=%d.}", us_status);
    } else {
    }
}


OAL_STATIC oal_void hmac_ap_rx_auth_req(
    hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req, dmac_wlan_crx_event_stru *pst_mgmt_rx_event)
{
    oal_netbuf_stru *auth_rsp = OAL_PTR_NULL;
    oal_uint16 auth_rsp_len;
    oal_uint32 ul_ret, ul_pedding_data;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;

    if (oal_any_null_ptr3(hmac_vap, auth_req, pst_mgmt_rx_event)) {
        OAM_ERROR_LOG0(0, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::param null.}");
        return;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if (hmac_chba_vap_start_check(hmac_vap)) {
        hmac_rx_ctl_stru *rx_ctl = (hmac_rx_ctl_stru *)oal_netbuf_cb(auth_req);
        mac_ieee80211_frame_stru *ieee80211_hdr =
            (mac_ieee80211_frame_stru *)(rx_ctl->st_rx_info.pul_mac_hdr_start_addr);
        if (hmac_chba_whitelist_check(hmac_vap, ieee80211_hdr->auc_address2, WLAN_MAC_ADDR_LEN) != OAL_SUCC) {
            return;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_SAE
    /* 如果是FT/SAE认证算法，上报hostapd */
    if (mac_get_auth_algo_num(auth_req) == WLAN_MIB_AUTH_ALG_SAE) {
        hmac_ap_up_rx_auth_req_to_host(hmac_vap, auth_req);
        return;
    }
#endif

    /* AP接收到STA发来的认证请求帧组相应的认证响应帧 */
    auth_rsp =
        (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (auth_rsp == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::auth_rsp null.}");
        return;
    }

    oal_mem_netbuf_trace(auth_rsp, OAL_TRUE);

    memset_s(oal_netbuf_cb(auth_rsp), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    auth_rsp_len = hmac_encap_auth_rsp(hmac_vap, auth_rsp, auth_req);
    if (auth_rsp_len == 0) {
        oal_netbuf_free(auth_rsp);
        OAM_ERROR_LOG0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::auth_rsp_len is 0.}");
        return;
    }

    oal_netbuf_put(auth_rsp, auth_rsp_len);

    hmac_mgmt_update_auth_mib(hmac_vap, auth_rsp);

    /* 发送认证响应帧之前，将用户的节能状态复位 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_rsp);
    if (mac_res_get_hmac_user(pst_tx_ctl->us_tx_user_idx) != OAL_PTR_NULL) {
        hmac_mgmt_reset_psm(&hmac_vap->st_vap_base_info, pst_tx_ctl->us_tx_user_idx);
    }
    hmac_config_scan_abort(&hmac_vap->st_vap_base_info, OAL_SIZEOF(oal_uint32), (oal_uint8 *)&ul_pedding_data);
    /* 抛事件给dmac发送认证帧 */
    ul_ret = hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, auth_rsp, auth_rsp_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(auth_rsp);
        OAM_WARNING_LOG1(0, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::hmac_tx_mgmt_send_event failed[%d].}", ul_ret);
    }
}


oal_bool_enum hmac_go_is_auth(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_dlist_tmp = OAL_PTR_NULL;
    mac_user_stru *pst_user_tmp = OAL_PTR_NULL;

    if (pst_mac_vap->en_p2p_mode != WLAN_P2P_GO_MODE) {
        return OAL_FALSE;
    }

    oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (pst_user_tmp == OAL_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_go_is_auth::pst_user_tmp null.}");
            continue;
        }
        
        if ((pst_user_tmp->en_user_asoc_state == MAC_USER_STATE_AUTH_COMPLETE) ||
            (pst_user_tmp->en_user_asoc_state == MAC_USER_STATE_AUTH_KEY_SEQ1)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}


OAL_STATIC oal_bool_enum hmac_ap_is_olbc_present(oal_uint8 *puc_payload, oal_uint32 ul_payload_len)
{
    oal_uint8 uc_num_rates = 0;
    mac_erp_params_stru *pst_erp_params = OAL_PTR_NULL;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    if (ul_payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::payload_len[%d]}", ul_payload_len);
        return OAL_FALSE;
    }

    ul_payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    puc_payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    puc_ie = mac_find_ie(MAC_EID_ERP, puc_payload, (oal_int32)ul_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        pst_erp_params = (mac_erp_params_stru *)(puc_ie + MAC_IE_HDR_LEN);
        /* 如果use protection被置为1， 返回TRUE */
        if (pst_erp_params->bit_non_erp == 1) {
            return OAL_TRUE;
        }
    }

    puc_ie = mac_find_ie(MAC_EID_RATES, puc_payload, (oal_int32)ul_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_rates += puc_ie[1];
    }

    puc_ie = mac_find_ie(MAC_EID_XRATES, puc_payload, (oal_int32)ul_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_rates += puc_ie[1];
    }

    /* 如果基本速率集数目小于或等于11b协议支持的最大速率集个数， 返回TRUE */
    if (uc_num_rates <= MAC_NUM_DR_802_11B) {
        oam_info_log1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::invalid uc_num_rates[%d].}", uc_num_rates);
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC oal_void hmac_ap_process_obss_erp_ie(
    hmac_vap_stru *pst_hmac_vap,
    oal_uint8 *puc_payload,
    oal_uint32 ul_payload_len)
{
    /* 存在non erp站点 */
    if (hmac_ap_is_olbc_present(puc_payload, ul_payload_len) == OAL_TRUE) {
        pst_hmac_vap->st_vap_base_info.st_protection.bit_obss_non_erp_present = OAL_TRUE;
        pst_hmac_vap->st_vap_base_info.st_protection.uc_obss_non_ht_aging_cnt = 0;
    }
}


OAL_STATIC oal_bool_enum hmac_ap_is_obss_non_ht_present(oal_uint8 *puc_payload, oal_uint32 ul_payload_len)
{
    mac_ht_opern_stru *pst_ht_opern = OAL_PTR_NULL;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    if (ul_payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::payload_len[%d]}", ul_payload_len);
        return OAL_TRUE;
    }

    ul_payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    puc_payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_payload, (oal_int32)ul_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        pst_ht_opern = (mac_ht_opern_stru *)(puc_ie + MAC_IE_HDR_LEN);
        if (pst_ht_opern->bit_obss_nonht_sta_present == 1) {
            /* 如果OBSS Non-HT STAs Present被置为1， 返回TRUE */
            return OAL_TRUE;
        }
    }

    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, (oal_int32)ul_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        /* 如果有HT capability信息元素，返回FALSE */
        return OAL_FALSE;
    }

    /* 如果没有HT capability信息元素，返回TRUE */
    return OAL_TRUE;
}


OAL_STATIC oal_void hmac_ap_process_obss_ht_operation_ie(
    hmac_vap_stru *pst_hmac_vap,
    oal_uint8 *puc_payload,
    oal_uint32 ul_payload_len)
{
    /* 如果存在non-ht站点 */
    if (hmac_ap_is_obss_non_ht_present(puc_payload, ul_payload_len) == OAL_TRUE) {
        pst_hmac_vap->st_vap_base_info.st_protection.bit_obss_non_ht_present = OAL_TRUE;
        pst_hmac_vap->st_vap_base_info.st_protection.uc_obss_non_ht_aging_cnt = 0;
    }
}


oal_uint32 hmac_ap_rx_obss_beacon(hmac_vap_stru *pst_hmac_vap,
                                  oal_uint8 *puc_payload,
                                  oal_uint32 ul_payload_len)
{
    /* 处理ERP相关 */
    hmac_ap_process_obss_erp_ie(pst_hmac_vap, puc_payload, ul_payload_len);

    /* 处理HT operation相关 */
    hmac_ap_process_obss_ht_operation_ie(pst_hmac_vap, puc_payload, ul_payload_len);

    /* 更新AP中保护相关mib量 */
    hmac_protection_update_mib_ap(&(pst_hmac_vap->st_vap_base_info));

    /* 更新vap的保护模式 */
    hmac_protection_update_mode_ap(&(pst_hmac_vap->st_vap_base_info));

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ap_rx_deauth_req(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr, oal_bool_enum_uint8 en_is_protected)
{
    oal_uint8 *puc_sa = OAL_PTR_NULL;
    oal_uint8 *puc_da = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint16 us_err_code;
    oal_uint32 ul_ret;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (puc_mac_hdr == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_AUTH, "{hmac_ap_rx_deauth_req::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);

    us_err_code = *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN));

    /* 增加接收到去认证帧时的维测信息 */
    oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                     "{aput rx deauth frame, reason code = %d, sa[XX:XX:XX:%X:%X:%X]}",
                     us_err_code, puc_sa[3], puc_sa[4], puc_sa[5]); /* us_err_code,puc_sa 3、4、5字节为参数输出打印 */

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_hmac_vap->st_vap_base_info, puc_sa, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_AUTH, "{aput rx deauth frame, pst_hmac_user null.}");
        return OAL_FAIL;
    }

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    /* 检查是否需要发送SA query request */
    if ((pst_hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code(&pst_hmac_user->st_user_base_info, en_is_protected, puc_mac_hdr) == OAL_SUCC)) {
        /* 在关联状态下收到未加密的ReasonCode 6/7需要开启SA Query流程 */
        ul_ret = hmac_start_sa_query(&pst_hmac_vap->st_vap_base_info, pst_hmac_user,
                                     pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        if (ul_ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }
        return OAL_SUCC;
    }
#endif

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    mac_rx_get_da((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_da);
    if ((ether_is_multicast(puc_da) != OAL_TRUE) &&
        (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active)) {
        oam_error_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_ap_rx_deauth_req::PMF check failed %d %d.}",
                       en_is_protected, pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_PASSIVE_DISCONNECT, us_err_code);
#endif
    /* 抛事件上报内核，已经去关联某个STA */
    hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user);

    hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
    return OAL_SUCC;
}

oal_void hmac_user_sort_op_rates(hmac_user_stru *pst_hmac_user)
{
    oal_uint8 uc_loop;
    oal_uint8 uc_num_rates;
    oal_uint8 uc_min_rate;
    oal_uint8 uc_temp_rate; /* 临时速率，用于数据交换 */
    oal_uint8 uc_index;
    oal_uint8 uc_temp_index; /* 临时索引，用于数据交换 */

    uc_num_rates = pst_hmac_user->st_op_rates.uc_rs_nrates;

    for (uc_loop = 0; uc_loop < uc_num_rates; uc_loop++) {
        /* 记录当前速率为最小速率 */
        uc_min_rate = (pst_hmac_user->st_op_rates.auc_rs_rates[uc_loop] & 0x7F);
        uc_temp_index = uc_loop;

        /* 依次查找最小速率 */
        for (uc_index = uc_loop + 1; uc_index < uc_num_rates; uc_index++) {
            /* 记录的最小速率大于如果当前速率 */
            if (uc_min_rate > (pst_hmac_user->st_op_rates.auc_rs_rates[uc_index] & 0x7F)) {
                /* 更新最小速率 */
                uc_min_rate = (pst_hmac_user->st_op_rates.auc_rs_rates[uc_index] & 0x7F);
                uc_temp_index = uc_index;
            }
        }

        uc_temp_rate = pst_hmac_user->st_op_rates.auc_rs_rates[uc_loop];
        pst_hmac_user->st_op_rates.auc_rs_rates[uc_loop] = pst_hmac_user->st_op_rates.auc_rs_rates[uc_temp_index];
        pst_hmac_user->st_op_rates.auc_rs_rates[uc_temp_index] = uc_temp_rate;
    }
}
#ifdef _PRE_WLAN_FEATURE_11AC2G
OAL_STATIC void hmac_ap_update_avail_protocol_mode(mac_vap_stru *pst_mac_vap,
                                                   mac_user_stru *mac_user, uint8_t *puc_avail_mode)
{
    if ((pst_mac_vap->en_protocol == WLAN_HT_MODE) && (mac_user->en_protocol_mode == WLAN_VHT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        *puc_avail_mode = WLAN_VHT_MODE;
    }
}
#endif

uint32_t hmac_mgmt_updata_protocol_cap(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, mac_user_stru *mac_user)
{
    uint8_t avail_mode;
    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode(mac_vap, hmac_user);
    avail_mode = g_auc_avail_protocol_mode[mac_vap->en_protocol][mac_user->en_protocol_mode];
    if (avail_mode == WLAN_PROTOCOL_BUTT) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::user not allowed:no valid protocol: \
                         vap mode=%d, user mode=%d, user avail mode=%d.}",
                         mac_vap->en_protocol, mac_user->en_protocol_mode, mac_user->en_avail_protocol_mode);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_11AC2G
    hmac_ap_update_avail_protocol_mode(mac_vap, mac_user, &avail_mode);
#endif
    /* 获取用户与VAP协议模式交集 */
    mac_user_set_avail_protocol_mode(mac_user, avail_mode);
    mac_user_set_cur_protocol_mode(mac_user, mac_user->en_avail_protocol_mode);

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_update_sta_user::vap protocol:%d,mac user protocol mode:%d,avail protocol mode:%d}",
                     mac_vap->en_protocol, mac_user->en_protocol_mode, mac_user->en_avail_protocol_mode);

    return OAL_SUCC;
}


OAL_STATIC oal_bool_enum_uint8 hmac_ap_up_update_sta_cap_info(
    hmac_vap_stru *pst_hmac_vap, oal_uint16 us_cap_info, hmac_user_stru *pst_hmac_user,
    mac_status_code_enum_uint16 *pen_status_code)
{
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)(&us_cap_info);

    if ((pst_hmac_vap == OAL_PTR_NULL) || (pst_hmac_user == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
                       "{hmac_ap_up_update_sta_cap_info::param null, %x %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)pst_hmac_user, (uintptr_t)pen_status_code);
        *pen_status_code = MAC_UNSPEC_FAIL;
        return OAL_FALSE;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);

    /* check bss capability info MAC,忽略MAC能力不匹配的STA */
    ul_ret = hmac_check_bss_cap_info(us_cap_info, pst_mac_vap);
    if (ul_ret != OAL_TRUE) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_ap_up_update_sta_cap_info::hmac_check_bss_cap_info failed[%d].}", ul_ret);
        *pen_status_code = MAC_UNSUP_CAP;
        return OAL_FALSE;
    }

    /* 如果以上各能力信息均满足关联要求，则继续处理其他能力信息 */
    mac_vap_check_bss_cap_info_phy_ap(us_cap_info, pst_mac_vap);

    if ((pst_cap_info->bit_privacy == 0) &&
        (pst_hmac_user->st_user_base_info.st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP)) {
        *pen_status_code = MAC_UNSPEC_FAIL;
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC oal_uint32 hmac_user_check_update_exp_rate(
    hmac_user_stru *pst_hmac_user, oal_uint8 *pst_params, oal_uint8 *puc_erp_rates_num)
{
    oal_uint8 uc_rate_idx = 0;
    oal_uint8 uc_loop;
    mac_rate_stru *pst_mac_rate = OAL_PTR_NULL;

    if ((pst_hmac_user == OAL_PTR_NULL) || (pst_params == OAL_PTR_NULL) || (puc_erp_rates_num == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *puc_erp_rates_num = pst_params[uc_rate_idx + 1];
    uc_rate_idx += MAC_IE_HDR_LEN;
    pst_mac_rate = &(pst_hmac_user->st_op_rates);

    if ((*puc_erp_rates_num) > (oal_uint8)WLAN_MAX_SUPP_RATES) {
        return OAL_FAIL;
    }

    for (uc_loop = 0; uc_loop < *puc_erp_rates_num; uc_loop++) {
        pst_mac_rate->auc_rs_rates[pst_mac_rate->uc_rs_nrates + uc_loop] = pst_params[uc_rate_idx + uc_loop] & 0x7F;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ap_up_update_sta_sup_rates(oal_uint8 *puc_payload,
                                                      hmac_user_stru *pst_hmac_user,
                                                      mac_status_code_enum_uint16 *pen_status_code,
                                                      oal_uint32 ul_msg_len,
                                                      oal_uint16 us_offset,
                                                      oal_uint8 *puc_num_rates,
                                                      oal_uint16 *pus_msg_idx)
{
    oal_uint8 uc_num_of_erp_rates = 0;
    oal_uint32 ul_loop;
    mac_user_stru *pst_mac_user;
    mac_vap_stru *pst_mac_vap;
    oal_uint8 *puc_sup_rates_ie = OAL_PTR_NULL;
    oal_uint8 *puc_ext_sup_rates_ie = OAL_PTR_NULL;
    oal_uint8 uc_temp_rate;
    oal_uint32 ul_ret;
    oal_uint16 us_msg_idx = 0;

    /* 初始化 */
    *puc_num_rates = 0;

    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        *pen_status_code = MAC_UNSPEC_FAIL;
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                       "{hmac_ap_up_update_sta_sup_rates::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    puc_sup_rates_ie = mac_find_ie(MAC_EID_RATES, puc_payload + us_offset, (oal_int32)(ul_msg_len - us_offset));
    if (puc_sup_rates_ie == OAL_PTR_NULL) {
        *pen_status_code = MAC_UNSUP_RATE;
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                       "{hmac_ap_up_update_sta_user::puc_ie null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (puc_sup_rates_ie[0] == MAC_EID_RATES) {
        *puc_num_rates = puc_sup_rates_ie[1];

        /* 如果速率个数为0 ，直接返回失败 */
        if (*puc_num_rates == 0) {
            *pen_status_code = MAC_UNSUP_RATE;
            *pus_msg_idx = us_msg_idx;
            OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                           "{hmac_ap_up_update_sta_sup_rates::the sta's rates are not supported.}");
            return OAL_FAIL;
        }

        us_msg_idx += MAC_IE_HDR_LEN;

        if (*puc_num_rates > MAC_MAX_SUPRATES) {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "hmac_ap_up_update_sta_sup_rates: rates num error: %d", *puc_num_rates);
            *puc_num_rates = MAC_MAX_SUPRATES;
        }

        for (ul_loop = 0; ul_loop < *puc_num_rates; ul_loop++) {
            /* 保存对应的速率到USER中 */
            pst_hmac_user->st_op_rates.auc_rs_rates[ul_loop] = puc_sup_rates_ie[us_msg_idx + ul_loop] & 0x7F;
        }

        us_msg_idx += *puc_num_rates;
        pst_hmac_user->st_op_rates.uc_rs_nrates = *puc_num_rates;
    }

    /* 如果存在扩展速率 */
    puc_ext_sup_rates_ie = mac_find_ie(MAC_EID_XRATES, puc_payload + us_offset, (oal_int32)(ul_msg_len - us_offset));
    if (puc_ext_sup_rates_ie == OAL_PTR_NULL) {
        if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
            oam_warning_log0(pst_mac_user->uc_vap_id, OAM_SF_ANY,
                             "{hmac_ap_up_update_sta_user::puc_ext_sup_rates_ie null.}");
        }
    } else {
        /* 只有11g混合模式或者更高协议版本才支持ERP */
        if ((puc_ext_sup_rates_ie[0] == MAC_EID_XRATES) &&
            (pst_mac_vap->en_protocol > WLAN_LEGACY_11G_MODE)) {
            /* 保存EXTRACE EXTENDED RATES,并得到扩展速率的个数 */
            ul_ret = hmac_user_check_update_exp_rate(pst_hmac_user, puc_ext_sup_rates_ie, &uc_num_of_erp_rates);
            if (ul_ret != OAL_SUCC) {
                uc_num_of_erp_rates = 0;
            }
        }
    }
    /* 速率个数增加 */
    *puc_num_rates += uc_num_of_erp_rates;

    /* 更新STA支持的速率个数 */
    pst_hmac_user->st_op_rates.uc_rs_nrates = *puc_num_rates;

    /* 按一定顺序重新排列速率 */
    hmac_user_sort_op_rates(pst_hmac_user);

    /*
     * 重排11g模式的可操作速率，使11b速率都聚集在11a之前
     * 802.11a 速率:6、9、12、18、24、36、48、54Mbps
     * 802.11b 速率:1、2、5.5、11Mbps
     * 由于按由小到大排序后802.11b中的速率11Mbps在802.11a中，下标为5
     * 所以从第五位进行检验并排序。
 */
    if (pst_hmac_user->st_op_rates.uc_rs_nrates == MAC_DATARATES_PHY_80211G_NUM) { /* 11g_compatibility mode */
        if ((pst_hmac_user->st_op_rates.auc_rs_rates[5] & 0x7F) == 0x16) {         /* 11Mbps在802.11a中，下标为5 */
            uc_temp_rate = pst_hmac_user->st_op_rates.auc_rs_rates[5]; /* 把auc_rs_rates 5byte速率值保存在uc_temp_rate */
            /* auc_rs_rates 4byte的速率值赋给auc_rs_rates 5byte */
            pst_hmac_user->st_op_rates.auc_rs_rates[5] = pst_hmac_user->st_op_rates.auc_rs_rates[4];
            /* auc_rs_rates 3byte的速率值赋给auc_rs_rates 4byte */
            pst_hmac_user->st_op_rates.auc_rs_rates[4] = pst_hmac_user->st_op_rates.auc_rs_rates[3];
            pst_hmac_user->st_op_rates.auc_rs_rates[3] = uc_temp_rate; /* uc_temp_rate速率值给auc_rs_rates 3byte */
        }
    }

    /*******************************************************************
      如果STA不支持所有基本速率返回不支持速率的错误码
    *******************************************************************/
#ifdef _PRE_WLAN_NARROW_BAND
    if (g_hitalk_status) {
        hmac_check_sta_base_rate((oal_uint8 *)pst_hmac_user, pen_status_code);
    }
#endif

    *pus_msg_idx = us_msg_idx;
    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_is_erp_sta(hmac_user_stru *pst_hmac_user)
{
    oal_uint32 ul_loop = 0;
    oal_bool_enum_uint8 en_is_erp_sta;

    /* 确认是否是erp 站点 */
    if (pst_hmac_user->st_op_rates.uc_rs_nrates <= MAC_NUM_DR_802_11B) {
        en_is_erp_sta = OAL_FALSE;
        for (ul_loop = 0; ul_loop < pst_hmac_user->st_op_rates.uc_rs_nrates; ul_loop++) {
            /* 如果支持速率不在11b的1M, 2M, 5.5M, 11M范围内，则说明站点为支持ERP的站点 */
            if (((pst_hmac_user->st_op_rates.auc_rs_rates[ul_loop] & 0x7F) != 0x2) &&
                ((pst_hmac_user->st_op_rates.auc_rs_rates[ul_loop] & 0x7F) != 0x4) &&
                ((pst_hmac_user->st_op_rates.auc_rs_rates[ul_loop] & 0x7F) != 0xb) &&
                ((pst_hmac_user->st_op_rates.auc_rs_rates[ul_loop] & 0x7F) != 0x16)) {
                en_is_erp_sta = OAL_TRUE;
                break;
            }
        }
    } else {
        en_is_erp_sta = OAL_TRUE;
        ;
    }

    return en_is_erp_sta;
}


oal_void hmac_ap_up_update_legacy_capability(
    hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user, oal_uint16 us_cap_info)
{
    mac_protection_stru *pst_protection = &(pst_hmac_vap->st_vap_base_info.st_protection);
    mac_user_stru *pst_mac_user = &(pst_hmac_user->st_user_base_info);
    oal_bool_enum_uint8 en_is_erp_sta;

    /* 如果STA不支持short slot */
    if ((us_cap_info & MAC_CAP_SHORT_SLOT) != MAC_CAP_SHORT_SLOT) {
        /* 如果STA之前没有关联， 或者之前以支持short slot站点身份关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) ||
            (pst_hmac_user->st_hmac_cap_info.bit_short_slot_time == OAL_TRUE)) {
            pst_protection->uc_sta_no_short_slot_num++;
        }

        pst_hmac_user->st_hmac_cap_info.bit_short_slot_time = OAL_FALSE;
    } else { /* 如果STA支持short slot */
        /* 如果STA以不支持short slot站点身份关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
            (pst_hmac_user->st_hmac_cap_info.bit_short_slot_time == OAL_FALSE) &&
            (pst_protection->uc_sta_no_short_slot_num != 0)) {
            pst_protection->uc_sta_no_short_slot_num--;
        }

        pst_hmac_user->st_hmac_cap_info.bit_short_slot_time = OAL_TRUE;
    }

    pst_hmac_user->st_user_stats_flag.bit_no_short_slot_stats_flag = OAL_TRUE;

    /* 如果STA不支持short preamble */
    if ((us_cap_info & MAC_CAP_SHORT_PREAMBLE) != MAC_CAP_SHORT_PREAMBLE) {
        /* 如果STA之前没有关联， 或者之前以支持short preamble站点身份关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) ||
            (pst_hmac_user->st_hmac_cap_info.bit_short_preamble == OAL_TRUE)) {
            pst_protection->uc_sta_no_short_preamble_num++;
        }

        pst_hmac_user->st_hmac_cap_info.bit_short_preamble = OAL_FALSE;
    } else { /* 如果STA支持short preamble */
        /* 如果STA之前以不支持short preamble站点身份关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
            (pst_hmac_user->st_hmac_cap_info.bit_short_preamble == OAL_FALSE) &&
            (pst_protection->uc_sta_no_short_preamble_num != 0)) {
            pst_protection->uc_sta_no_short_preamble_num--;
        }

        pst_hmac_user->st_hmac_cap_info.bit_short_preamble = OAL_TRUE;
    }

    pst_hmac_user->st_user_stats_flag.bit_no_short_preamble_stats_flag = OAL_TRUE;

    /* 确定user是否是erp站点 */
    en_is_erp_sta = hmac_is_erp_sta(pst_hmac_user);
    /* 如果STA不支持ERP */
    if (en_is_erp_sta == OAL_FALSE) {
        /* 如果STA之前没有关联， 或者之前以支持ERP站点身份关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) ||
            (pst_hmac_user->st_hmac_cap_info.bit_erp == OAL_TRUE)) {
            pst_protection->uc_sta_non_erp_num++;
        }

        pst_hmac_user->st_hmac_cap_info.bit_erp = OAL_FALSE;
    } else { /* 如果STA支持ERP */
        /* 如果STA之前以不支持ERP身份站点关联，需要update处理 */
        if ((pst_mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
            (pst_hmac_user->st_hmac_cap_info.bit_erp == OAL_FALSE) &&
            (pst_protection->uc_sta_non_erp_num != 0)) {
            pst_protection->uc_sta_non_erp_num--;
        }

        pst_hmac_user->st_hmac_cap_info.bit_erp = OAL_TRUE;
    }

    pst_hmac_user->st_user_stats_flag.bit_no_erp_stats_flag = OAL_TRUE;

    if ((us_cap_info & MAC_CAP_SPECTRUM_MGMT) != MAC_CAP_SPECTRUM_MGMT) {
        mac_user_set_spectrum_mgmt(&pst_hmac_user->st_user_base_info, OAL_FALSE);
    } else {
        mac_user_set_spectrum_mgmt(&pst_hmac_user->st_user_base_info, OAL_TRUE);
    }
}


OAL_STATIC oal_void hmac_ap_up_update_asoc_entry_prot(
    oal_uint8 *puc_mac_hdr, oal_uint8 uc_sub_type, oal_uint32 ul_msg_len, oal_uint16 us_info_elem_offset,
    oal_uint16 us_cap_info, hmac_user_stru *pst_hmac_user, hmac_vap_stru *pst_hmac_vap)
{
    /* WMM */
    hmac_uapsd_update_user_para(puc_mac_hdr, uc_sub_type, ul_msg_len, pst_hmac_user);
}

oal_void hmac_ap_update_2g11ac(mac_vap_stru *pst_mac_vap,
                               mac_user_stru *pst_mac_user,
                               oal_uint8 *puc_avail_mode)
{
    *puc_avail_mode = g_auc_avail_protocol_mode[pst_mac_vap->en_protocol][pst_mac_user->en_protocol_mode];

#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((pst_mac_vap->en_protocol == WLAN_HT_MODE) && (pst_mac_user->en_protocol_mode == WLAN_VHT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        *puc_avail_mode = WLAN_VHT_MODE;
    }
#endif
}


OAL_STATIC oal_uint32 hmac_ap_up_update_sta_user(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr, oal_uint8 *puc_payload,
    oal_uint32 ul_msg_len, hmac_user_stru *pst_hmac_user, mac_status_code_enum_uint16 *pen_status_code)
{
    oal_uint32 ul_rslt;
    oal_uint16 us_msg_idx = 0;
    oal_uint16 us_cap_info;
    oal_uint16 us_ssid_len;
    oal_uint8 uc_num_rates;
    oal_uint16 us_rate_len = 0;
    mac_status_code_enum_uint16 us_ret_val;
    mac_cfg_ssid_param_stru st_cfg_ssid;
    const oal_uint8 *puc_rsn_ie = OAL_PTR_NULL;
    oal_uint16 us_offset;
    wlan_bw_cap_enum_uint8 en_bandwidth_cap = WLAN_BW_CAP_BUTT;
    wlan_bw_cap_enum_uint8 en_bwcap_ap; /* ap自身带宽能力 */
    oal_uint32 ul_ret;
    mac_vap_stru *pst_mac_vap;
    mac_user_stru *pst_mac_user;
    wlan_bw_cap_enum_uint8 en_bwcap_vap;
#ifdef _PRE_WLAN_FEATURE_TXBF
    oal_uint8 *puc_vendor_ie;
#endif
    oal_uint8 *puc_ie_tmp = OAL_PTR_NULL;
    oal_uint8 uc_avail_mode;

    *pen_status_code = MAC_SUCCESSFUL_STATUSCODE;
    us_offset = MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    /***************************************************************************
        检查AP是否支持当前正在关联的STA的所有能力
        |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|Reserved|
    ***************************************************************************/
    /* puc_payload[us_msg_idx]（低8位）与puc_payload[(us_msg_idx + 1)]（高8位）组成16bit数 */
    us_cap_info = puc_payload[us_msg_idx] | (puc_payload[(us_msg_idx + 1)] << 8);

    ul_rslt = hmac_ap_up_update_sta_cap_info(pst_hmac_vap, us_cap_info, pst_hmac_user, pen_status_code);
    if (ul_rslt != OAL_TRUE) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{ap up update sta cap info failed[%d], status_code=%d.}", ul_rslt, *pen_status_code);
        return ul_rslt;
    }

    us_msg_idx += MAC_CAP_INFO_LEN;
    us_msg_idx += MAC_LIS_INTERVAL_IE_LEN;

    if (mac_get_frame_sub_type(puc_mac_hdr) == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        /* 重关联比关联请求帧头多了AP的MAC地址 */
        us_msg_idx += WLAN_MAC_ADDR_LEN;
        us_offset += WLAN_MAC_ADDR_LEN;
    }

    /* 判断SSID,长度或内容不一致时,认为是SSID不一致，考虑兼容性找不到ie时不处理 */
    puc_ie_tmp = mac_find_ie(MAC_EID_SSID, puc_payload + us_msg_idx, (oal_int32)(ul_msg_len - us_msg_idx));
    if (puc_ie_tmp != OAL_PTR_NULL) {
        us_ssid_len = 0;

        st_cfg_ssid.uc_ssid_len = 0;

        hmac_config_get_ssid (pst_mac_vap, &us_ssid_len, (oal_uint8 *)(&st_cfg_ssid));

        if (st_cfg_ssid.uc_ssid_len != puc_ie_tmp[1]) {
            *pen_status_code = MAC_UNSPEC_FAIL;
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                             "AP refuse STA assoc, ssid len mismatch, status_code=%d.", *pen_status_code);
            return OAL_FAIL;
        }
        /* puc_ie_tmp第2字节开始是ssid内存 */
        if (oal_memcmp(&puc_ie_tmp[2], st_cfg_ssid.ac_ssid, st_cfg_ssid.uc_ssid_len) != 0) {
            *pen_status_code = MAC_UNSPEC_FAIL;
            OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "AP refuse STA assoc, ssid mismatch, status_code=%d.}", *pen_status_code);
            return OAL_FAIL;
        }
    }

    /* 当前用户已关联 */
    ul_rslt = hmac_ap_up_update_sta_sup_rates(puc_payload, pst_hmac_user, pen_status_code, ul_msg_len,
                                              us_msg_idx, &uc_num_rates, &us_rate_len);
    if (ul_rslt != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC, "{AP refuse STA assoc, update rates failed, status_code[%d] ul_rslt[%d].}",
                         *pen_status_code, ul_rslt);
        return ul_rslt;
    }

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    /* 检查接收到的ASOC REQ消息中的SECURITY参数.如出错,则返回对应的错误码 */
    ul_rslt = hmac_check_assoc_req_security_cap_authenticator(pst_hmac_vap, puc_mac_hdr, puc_payload,
                                                              ul_msg_len, pst_hmac_user, pen_status_code);
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "Reject STA because of security_cap_auth[%d]", ul_rslt);
        return OAL_FAIL;
    }
#endif /* defined (_PRE_WLAN_FEATURE_WPA) ||　defined(_PRE_WLAN_FEATURE_WPA2) */

    /* 更新对应STA的legacy协议能力 */
    hmac_ap_up_update_legacy_capability(pst_hmac_vap, pst_hmac_user, us_cap_info);

    /* 检查HT capability是否匹配，并进行处理 */
    us_ret_val = hmac_vap_check_ht_capabilities_ap(pst_hmac_vap, puc_payload, us_msg_idx, ul_msg_len, pst_hmac_user);
    if (us_ret_val != MAC_SUCCESSFUL_STATUSCODE) {
        *pen_status_code = us_ret_val;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "Reject STA because of ht_capability[%d]", us_ret_val);
        return us_ret_val;
    }

    /* 更新AP中保护相关mib量 */
    ul_ret = hmac_protection_update_mib_ap(pst_mac_vap);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "protection update mib failed, ret=%d", ul_ret);
    }

    /* 更新对应STA的协议能力 update_asoc_entry_prot(ae, msa, rx_len, cap_info, is_p2p); */
    hmac_ap_up_update_asoc_entry_prot(puc_payload, mac_get_frame_sub_type(puc_mac_hdr), ul_msg_len,
                                      us_msg_idx, us_cap_info, pst_hmac_user, pst_hmac_vap);

    /* 更新QoS能力 */
    hmac_mgmt_update_assoc_user_qos_table(puc_payload, (oal_uint16)ul_msg_len, us_msg_idx, pst_hmac_user);

#ifdef _PRE_WLAN_FEATURE_TXBF
    /* 更新11n txbf能力 */
    puc_vendor_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_EID_11NTXBF, puc_payload + us_msg_idx,
                                       (oal_int32)(ul_msg_len - us_msg_idx));
    hmac_mgmt_update_11ntxbf_cap(puc_vendor_ie, pst_hmac_user);
#endif

    /* 更新11ac VHT capabilities ie */
    us_ret_val = hmac_vap_check_vht_capabilities_ap(pst_hmac_vap, puc_payload, us_msg_idx, ul_msg_len, pst_hmac_user);
    if (us_ret_val != MAC_SUCCESSFUL_STATUSCODE) {
        *pen_status_code = us_ret_val;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::Reject STA because of vht_capability[%d].}", us_ret_val);
        return us_ret_val;
    }

    /* 查找RSN信息元素,如果没有RSN信息元素,则按照不支持处理 */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, puc_payload + us_offset, (oal_int32)(ul_msg_len - us_offset));

    /* 根据RSN信息元素, 判断RSN能力是否匹配 */
    ul_ret = hmac_check_rsn_capability(pst_mac_vap, pst_hmac_user, puc_rsn_ie, pen_status_code);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::Reject STA because of rsn_capability[%d].}", ul_ret);
        return ul_ret;
    }

    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode(pst_mac_vap, pst_hmac_user);

    hmac_ap_update_2g11ac(pst_mac_vap, pst_mac_user, &uc_avail_mode);

    /* 获取用户与VAP协议模式交集 */
    mac_user_set_avail_protocol_mode(pst_mac_user, uc_avail_mode);
    mac_user_set_cur_protocol_mode(pst_mac_user, pst_mac_user->en_avail_protocol_mode);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_update_sta_user::mac_vap->en_protocol:%d,mac_user->en_protocol_mode:%d, \
                     en_avail_protocol_mode:%d.}",
                     pst_mac_vap->en_protocol, pst_mac_user->en_protocol_mode, pst_mac_user->en_avail_protocol_mode);

    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates(pst_mac_vap, pst_hmac_user);

    /* 获取用户的带宽能力 */
    mac_user_get_sta_cap_bandwidth(pst_mac_user, &en_bandwidth_cap);

    /* 获取vap带宽能力与用户带宽能力的交集 */
    mac_vap_get_bandwidth_cap(&pst_hmac_vap->st_vap_base_info, &en_bwcap_ap);
    en_bwcap_vap = oal_min(en_bwcap_ap, en_bandwidth_cap);
    mac_user_set_bandwidth_info(pst_mac_user, en_bwcap_vap, en_bwcap_vap);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_update_sta_user::mac_vap->bandwidth:%d,mac_user->bandwidth:%d,cur_bandwidth:%d.}",
                     en_bwcap_ap, en_bandwidth_cap,
                     pst_mac_user->en_cur_bandwidth);

    ul_ret = hmac_config_user_cap_syn(pst_mac_vap, pst_mac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_user->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_up_update_sta_user::hmac_config_usr_cap_syn failed[%d].}", ul_ret);
    }

    /* 根据用户支持带宽能力，协商出当前带宽，dmac offload架构下，同步带宽信息到device */
    ul_ret = hmac_config_user_info_syn(pst_mac_vap, pst_mac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(pst_mac_user->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_up_update_sta_user::usr_info_syn failed[%d].}", ul_ret);
    }

    /* 获取用户与VAP空间流交集 */
    ul_ret = hmac_user_set_avail_num_space_stream(pst_mac_user, pst_mac_vap->en_vap_rx_nss);
    if (ul_ret != OAL_SUCC) {
        *pen_status_code = MAC_UNSPEC_FAIL;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::mac_user_set_avail_num_space_stream failed[%d].}", ul_ret);
    }

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 处理Operating Mode Notification 信息元素 */
    ul_ret = hmac_check_opmode_notify(pst_hmac_vap, puc_mac_hdr, puc_payload, us_msg_idx, ul_msg_len, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::hmac_check_opmode_notify failed[%d].}", ul_ret);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ap_prepare_assoc_req(
    hmac_user_stru *pst_hmac_user, oal_uint8 *puc_payload, oal_uint32 ul_payload_len, oal_uint8 uc_mgmt_frm_type)
{
    oal_uint32 ul_payload_size;

    /* AP 保存STA 的关联请求帧信息，以备上报内核 */
    if (pst_hmac_user->puc_assoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free_m(pst_hmac_user->puc_assoc_req_ie_buff, OAL_TRUE);
        pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
        pst_hmac_user->ul_assoc_req_ie_len = 0;
    }
    ul_payload_size = ul_payload_len;

    /* 目前11r没有实现，所以处理重关联帧的流程和关联帧一样，11r实现后此处需要修改 */
    if (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        /* 重关联比关联请求帧头多了AP的MAC地址 */
        puc_payload += (WLAN_MAC_ADDR_LEN + MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN);
        ul_payload_len -= (WLAN_MAC_ADDR_LEN + MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN);
    } else {
        puc_payload += (MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN);
        ul_payload_len -= (MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN);
    }

    pst_hmac_user->puc_assoc_req_ie_buff = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
                                                           (oal_uint16)ul_payload_size, OAL_TRUE);
    if (pst_hmac_user->puc_assoc_req_ie_buff == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_prepare_assoc_req::Alloc Assoc Req for kernel failed.}");
        pst_hmac_user->ul_assoc_req_ie_len = 0;
        return OAL_FAIL;
    } else {
        if (memcpy_s(pst_hmac_user->puc_assoc_req_ie_buff, ul_payload_size, puc_payload, ul_payload_len) != EOK) {
            OAM_ERROR_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                           "{hmac_ap_prepare_assoc_req::memcpy failed, payload_size[%d].}", ul_payload_size);
            oal_mem_free_m(pst_hmac_user->puc_assoc_req_ie_buff, OAL_TRUE);
            pst_hmac_user->puc_assoc_req_ie_buff = OAL_PTR_NULL;
            pst_hmac_user->ul_assoc_req_ie_len = 0;
            return OAL_FAIL;
        }
        pst_hmac_user->ul_assoc_req_ie_len = ul_payload_len;
        return OAL_SUCC;
    }
}
OAL_STATIC void hmac_ap_up_rx_asoc_req_change_user_state_to_auth(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    if (mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC != OAL_TRUE && hmac_user->assoc_ap_up_tx_auth_req) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::RX (re)assoc req, change user to auth.}");
        hmac_user->assoc_ap_up_tx_auth_req = OAL_FALSE;
        hmac_user_set_asoc_state(mac_vap, &hmac_user->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);
    }
}

OAL_STATIC oal_uint32 hmac_ap_up_rx_asoc_req(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_mgmt_frm_type, oal_uint8 *puc_mac_hdr,
    oal_uint32 ul_mac_hdr_len, oal_uint8 *puc_payload, oal_uint32 ul_payload_len)
{
    oal_uint32 ul_rslt;
    oal_netbuf_stru *pst_asoc_rsp = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint16 us_user_idx = 0;
    oal_uint32 ul_asoc_rsp_len = 0;
    mac_status_code_enum_uint16 en_status_code;
    oal_uint8 auc_sta_addr[WLAN_MAC_ADDR_LEN];
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    mac_cfg_user_info_param_stru st_hmac_user_info_event;

#ifdef _PRE_WLAN_FEATURE_P2P
    oal_int32 l_len;
#endif
    oal_uint8 uc_frm_least_len = MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN;

    uc_frm_least_len += (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ? WLAN_MAC_ADDR_LEN : 0;

    if (ul_payload_len < uc_frm_least_len) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::RX assoc req len abnormal[%d]}", ul_payload_len);
        return OAL_FAIL;
    }

    mac_get_address2(puc_mac_hdr, auc_sta_addr, WLAN_MAC_ADDR_LEN);
#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_is_chba_mode(&(pst_hmac_vap->st_vap_base_info))) {
        if (hmac_chba_whitelist_check(pst_hmac_vap, auc_sta_addr, WLAN_MAC_ADDR_LEN) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }
#endif
    ul_rslt = mac_vap_find_user_by_macaddr(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr,
        WLAN_MAC_ADDR_LEN, &us_user_idx);
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::mac_vap_find_user_by_macaddr failed[%d].}", ul_rslt);
        oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::user mac:%02X:XX:XX:%02X:%02X:%02X.}",
                         /* auc_sta_addr 0、3、4、5byte为参数输出打印 */
                         auc_sta_addr[0], auc_sta_addr[3], auc_sta_addr[4], auc_sta_addr[5]);
        hmac_mgmt_send_deauth_frame(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr,
            WLAN_MAC_ADDR_LEN, MAC_ASOC_NOT_AUTH, OAL_FALSE);

        return ul_rslt;
    }

    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_rx_asoc_req::us_user_idx[%d].}", us_user_idx);

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::pst_hmac_user[%d] null.}", us_user_idx);

        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr,
            WLAN_MAC_ADDR_LEN, MAC_ASOC_NOT_AUTH, OAL_FALSE);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_immediate_destroy_timer(&pst_hmac_user->st_mgmt_timer);
    }
    hmac_ap_up_rx_asoc_req_change_user_state_to_auth(&(pst_hmac_vap->st_vap_base_info), pst_hmac_user);
    en_status_code = MAC_SUCCESSFUL_STATUSCODE;

    /* 是否符合触发SA query流程的条件 */
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    ul_rslt = hmac_check_sa_query_trigger(pst_hmac_user, pst_hmac_vap, &en_status_code);
    if (ul_rslt != OAL_SUCC) {
        return ul_rslt;
    }
#endif

    if (en_status_code != MAC_REJECT_TEMP) {
        if (pst_hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
            oam_warning_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_ap_up_rx_asoc_req::user associated, unexpected (re)assoc req no handle!}");
            return OAL_FAIL;
        }
        /* 当可以查找到用户时,说明当前USER的状态为已关联或已认证完成 */
        ul_rslt = hmac_ap_up_update_sta_user(pst_hmac_vap, puc_mac_hdr, puc_payload, ul_payload_len,
                                             pst_hmac_user, &en_status_code);
        if (en_status_code != MAC_SUCCESSFUL_STATUSCODE) {
            OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_ap_up_rx_asoc_req::hmac_ap_up_update_sta_user failed[%d].}", en_status_code);
            hmac_user_set_asoc_state(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user->st_user_base_info,
                                     MAC_USER_STATE_AUTH_COMPLETE);
        }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* 同步ap带宽，能力等信息到dmac */
        hmac_chan_sync(&pst_hmac_vap->st_vap_base_info, pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                       pst_hmac_vap->st_vap_base_info.st_channel.en_bandwidth, OAL_FALSE);

#endif

        /* 根据用户支持带宽能力，协商出当前带宽，dmac offload架构下，同步带宽信息到device */
        ul_rslt = hmac_config_user_info_syn(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user->st_user_base_info);
        if (ul_rslt != OAL_SUCC) {
            OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::usr_info_syn failed[%d].}", ul_rslt);
        }

        if (en_status_code == MAC_SUCCESSFUL_STATUSCODE) {
            ul_rslt = hmac_init_security(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr, OAL_SIZEOF(auc_sta_addr));
            if (ul_rslt != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_ASSOC, "hmac_ap_up_rx_asoc_req:hmac_init_security failed[%d] status_code[%d]",
                               ul_rslt, MAC_UNSPEC_FAIL);
                en_status_code = MAC_UNSPEC_FAIL;
            }

#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
            ul_rslt = hmac_init_user_security_port(&(pst_hmac_vap->st_vap_base_info),
                                                   &(pst_hmac_user->st_user_base_info));
            if (ul_rslt != OAL_SUCC) {
                OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "hmac_ap_up_rx_asoc_req:init user security_port failed[%d]", ul_rslt);
            }
#endif /* defined(_PRE_WLAN_FEATURE_WPA) ||　defined(_PRE_WLAN_FEATURE_WPA2) */
        }

        if ((ul_rslt != OAL_SUCC) ||
            (en_status_code != MAC_SUCCESSFUL_STATUSCODE)) {
            oam_warning_log2(0, OAM_SF_CFG, "hmac_ap_up_rx_asoc_req:ap update user fail rslt[%d] status_code[%d]",
                             ul_rslt, en_status_code);
#ifdef _PRE_DEBUG_MODE_USER_TRACK
            mac_user_change_info_event(pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                       pst_hmac_vap->st_vap_base_info.uc_vap_id,
                                       pst_hmac_user->st_user_base_info.en_user_asoc_state,
                                       MAC_USER_STATE_AUTH_COMPLETE, OAM_MODULE_ID_HMAC,
                                       OAM_USER_INFO_CHANGE_TYPE_ASSOC_STATE);
#endif
            hmac_user_set_asoc_state(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user->st_user_base_info,
                                     MAC_USER_STATE_AUTH_COMPLETE);
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        l_len = ((uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ?
                (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN + WLAN_MAC_ADDR_LEN) : (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN));
        if (IS_P2P_GO(&pst_hmac_vap->st_vap_base_info) &&
            (mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_payload + l_len,
                                (oal_int32)ul_payload_len - l_len) == OAL_PTR_NULL)) {
            hmac_disable_p2p_pm(pst_hmac_vap);
        }
#endif
    }

    pst_asoc_rsp =
        (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_asoc_rsp == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::pst_asoc_rsp null.}");
        /* 异常返回之前删除user */
        hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_asoc_rsp);
    memset_s(pst_tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    oal_mem_netbuf_trace(pst_asoc_rsp, OAL_TRUE);

    if (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ) {
        ul_asoc_rsp_len = hmac_mgmt_encap_asoc_rsp_ap(pst_hmac_vap, auc_sta_addr, WLAN_MAC_ADDR_LEN,
                                                      pst_hmac_user->st_user_base_info.us_assoc_id, en_status_code,
                                                      oal_netbuf_header(pst_asoc_rsp), WLAN_FC0_SUBTYPE_ASSOC_RSP);
    } else if (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        hmac_user_clear_defrag_res(pst_hmac_user);
        ul_asoc_rsp_len = hmac_mgmt_encap_asoc_rsp_ap(pst_hmac_vap, auc_sta_addr, WLAN_MAC_ADDR_LEN,
            pst_hmac_user->st_user_base_info.us_assoc_id, en_status_code, oal_netbuf_header(pst_asoc_rsp),
            WLAN_FC0_SUBTYPE_REASSOC_RSP);
    }

    if (ul_asoc_rsp_len == 0) {
        oam_warning_log0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::hmac_mgmt_encap_asoc_rsp_ap encap msg fail.}");
        oal_netbuf_free(pst_asoc_rsp);

        /* 异常返回之前删除user */
        hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);

        return OAL_FAIL;
    }

    oal_netbuf_put(pst_asoc_rsp, ul_asoc_rsp_len);

    pst_tx_ctl->us_tx_user_idx = pst_hmac_user->st_user_base_info.us_assoc_id;
    pst_tx_ctl->us_mpdu_len = (oal_uint16)ul_asoc_rsp_len;

    /* 发送关联响应帧之前，将用户的节能状态复位 */
    hmac_mgmt_reset_psm(&pst_hmac_vap->st_vap_base_info, pst_tx_ctl->us_tx_user_idx);

    /* 判断当前状态，如果用户已经关联成功则向上报用户离开信息 */
    hmac_ap_report_sta_leave(pst_hmac_vap, pst_hmac_user);

    if (en_status_code == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_user_set_asoc_state(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user->st_user_base_info,
                                 MAC_USER_STATE_ASSOC);
    }

    ul_rslt = hmac_tx_mgmt_send_event(&(pst_hmac_vap->st_vap_base_info), pst_asoc_rsp, (oal_uint16)ul_asoc_rsp_len);
    if (ul_rslt != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::hmac_tx_mgmt_send_event failed[%d].}", ul_rslt);
        oal_netbuf_free(pst_asoc_rsp);

        /* 异常返回之前删除user */
        hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);

        return ul_rslt;
    }

    if (en_status_code == MAC_SUCCESSFUL_STATUSCODE) {
        /* AP检测STA成功，允许其关联成功 */
#ifdef _PRE_DEBUG_MODE_USER_TRACK
        mac_user_change_info_event(pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                   pst_hmac_vap->st_vap_base_info.uc_vap_id,
                                   pst_hmac_user->st_user_base_info.en_user_asoc_state,
                                   MAC_USER_STATE_ASSOC, OAM_MODULE_ID_HMAC,
                                   OAM_USER_INFO_CHANGE_TYPE_ASSOC_STATE);
#endif

        ul_rslt = hmac_config_user_rate_info_syn(&(pst_hmac_vap->st_vap_base_info), &pst_hmac_user->st_user_base_info);
        if (ul_rslt != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                           "{hmac_sta_wait_asoc_rx::hmac_config_user_rate_info_syn failed[%d].}", ul_rslt);
        }

        /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
        hmac_user_add_notify_alg((&pst_hmac_vap->st_vap_base_info), us_user_idx);

        /* AP 保存STA 的关联请求帧信息，以备上报内核 */
        hmac_ap_prepare_assoc_req(pst_hmac_user, puc_payload, ul_payload_len, uc_mgmt_frm_type);

        /* 上报WAL层(WAL上报内核) AP关联上了一个新的STA */
        hmac_handle_connect_rsp_ap(pst_hmac_vap, pst_hmac_user);
#ifdef _PRE_WLAN_CHBA_MGMT
        if (hmac_chba_vap_start_check(pst_hmac_vap) == OAL_TRUE) {
            /* CHBA关联成功处理 */
            hmac_chba_connect_succ_handler(pst_hmac_vap, pst_hmac_user,
                puc_payload + uc_frm_least_len, ul_payload_len - uc_frm_least_len);
        }
#endif
        OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::STA assoc AP SUCC! STA_indx=%d.}", us_user_idx);
    } else {
        /* AP检测STA失败，将其删除 */
        if (en_status_code != MAC_REJECT_TEMP) {
            hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
        }
    }

    /* 1102 STA 入网后，上报VAP 信息和用户信息 */
    st_hmac_user_info_event.us_user_idx = us_user_idx;

    hmac_config_vap_info(&(pst_hmac_vap->st_vap_base_info), OAL_SIZEOF(oal_uint32), (oal_uint8 *)&ul_rslt);
    hmac_config_user_info(&(pst_hmac_vap->st_vap_base_info), OAL_SIZEOF(mac_cfg_user_info_param_stru),
                          (oal_uint8 *)&st_hmac_user_info_event);
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_ap_up_rx_disasoc(
    hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_mac_hdr, oal_uint32 ul_mac_hdr_len,
    oal_uint8 *puc_payload, oal_uint32 ul_payload_len, oal_bool_enum_uint8 en_is_protected)
{
    oal_uint32 ul_ret;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint8 *puc_da = OAL_PTR_NULL;
    oal_uint8 *puc_sa = OAL_PTR_NULL;
    oal_uint8 auc_sta_addr[WLAN_MAC_ADDR_LEN];

    if (ul_payload_len < MAC_80211_REASON_CODE_LEN) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_disasoc::invalid ul_payload_len len [%d]}", ul_payload_len);
        return OAL_FAIL;
    }

    mac_get_address2(puc_mac_hdr, auc_sta_addr, WLAN_MAC_ADDR_LEN);

    /* 增加接收到去关联帧时的维测信息 */
    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);

    oam_warning_log4(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_rx_disasoc::Because of err_code[%d], received disassoc frame from source addr \
                     %02x:xx:xx:xx:%02x:%02x.}", *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)),
                     puc_sa[0], puc_sa[4], puc_sa[5]); /* puc_sa 0、4、5byte为参数输出打印 */

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_disasoc::pst_hmac_user null.}");
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(&(pst_hmac_vap->st_vap_base_info), auc_sta_addr,
            WLAN_MAC_ADDR_LEN, MAC_NOT_ASSOCED, OAL_FALSE);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
        /* 检查是否需要发送SA query request */
        if (hmac_pmf_check_err_code(&pst_hmac_user->st_user_base_info, en_is_protected, puc_mac_hdr) == OAL_SUCC) {
            /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
            ul_ret = hmac_start_sa_query(&pst_hmac_vap->st_vap_base_info, pst_hmac_user,
                                         pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
            if (ul_ret != OAL_SUCC) {
                OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_disasoc::hmac_start_sa_query failed[%d].}", ul_ret);
                return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
            }
            return OAL_SUCC;
        }
#endif

        /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
        mac_rx_get_da((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_da);
        if ((ether_is_multicast(puc_da) != OAL_TRUE) &&
            (en_is_protected != pst_hmac_user->st_user_base_info.st_cap_info.bit_pmf_active)) {
            OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_disasoc:en_is_protected=%d.}", en_is_protected);
            return OAL_FAIL;
        }

        mac_user_set_asoc_state(&pst_hmac_user->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);
#ifdef _PRE_WLAN_1102A_CHR
        chr_exception_report (CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                              CHR_WIFI_DRV_EVENT_SOFTAP_PASSIVE_DISCONNECT,
                              *((oal_uint16 *)(puc_mac_hdr + MAC_80211_FRAME_LEN)));
#endif
        /* 抛事件上报内核，已经去关联某个STA */
        hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user);

        /* 有些网卡去关联时只发送DISASOC,也将删除其在AP内部的数据结构 */
        ul_ret = hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_disasoc::hmac_user_del failed[%d].}", ul_ret);
        }
    }

    return OAL_SUCC;
}

OAL_STATIC oal_void hmac_ap_up_rx_obss_beacon(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    oal_uint8 *puc_payload;
    oal_uint16 us_payload_len;

    /* 获取帧体长度 */
    us_payload_len = pst_rx_ctrl->st_rx_info.us_frame_len - pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* 获取帧体指针 */
    puc_payload =
        (oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* 处理ERP相关 */
    if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
        hmac_ap_process_obss_erp_ie(pst_hmac_vap, puc_payload, us_payload_len);
    }

    /* 处理HT operation相关 */
    hmac_ap_process_obss_ht_operation_ie(pst_hmac_vap, puc_payload, us_payload_len);

    /* 更新AP中保护相关mib量 */
    hmac_protection_update_mib_ap(&(pst_hmac_vap->st_vap_base_info));

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if ((mac_mib_get_FortyMHzOperationImplemented(&(pst_hmac_vap->st_vap_base_info)) == OAL_TRUE) &&
        (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) &&
        (pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_2040_autoswitch == OAL_TRUE)) {
        hmac_ap_process_obss_40mhz_intol(pst_hmac_vap, puc_payload, us_payload_len);
    }

#endif
}


OAL_STATIC oal_void hmac_ap_up_rx_action_nonuser(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    oal_uint8 *puc_data = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL; /* 保存mac帧的指针 */

    if (oal_unlikely((pst_hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL))) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_action_nonuser:: NULL param.}");
        return;
    }

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

    /* 获取帧体指针 */
    puc_data = (oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* Category */
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_PUBLIC: {
            /* Action */
            switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
                case MAC_PUB_VENDOR_SPECIFIC: {
                    if (oal_memcmp(puc_data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
                        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                         "{hmac_ap_up_rx_action::hmac location get.}");
                        hmac_huawei_action_process(pst_hmac_vap, pst_netbuf,
                                                   puc_data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
                    }
                    break;
                }
#endif
                default:
                    break;
            }
        }
        break;
        default:
            break;
    }
    return;
}
OAL_STATIC oal_void hmac_ap_up_category_ba(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    hmac_user_stru *pst_hmac_user, oal_uint32 frame_body_len)
{
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            hmac_mgmt_rx_addba_req(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;
        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;
        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba(pst_hmac_vap, pst_hmac_user, puc_data, frame_body_len);
            break;
        default:
            break;
    }
}
OAL_STATIC oal_void hmac_ap_up_category_public(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf, oal_uint32 frame_body_len)
{
    /* Action */
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        case MAC_PUB_COEXT_MGMT:
            if (pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_2040_autoswitch == OAL_TRUE) {
                hmac_ap_up_rx_2040_coext(pst_hmac_vap, pst_netbuf);
            }
            break;
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */
        case MAC_PUB_VENDOR_SPECIFIC: {
            if (frame_body_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_action_category_public::frame_body_len %d.}", frame_body_len);
                return;
            }
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (mac_ie_check_p2p_action(puc_data + MAC_ACTION_OFFSET_ACTION) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
            }
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
            if (oal_memcmp(puc_data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
                oam_warning_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_action::hmac location get.}");
                hmac_huawei_action_process(pst_hmac_vap, pst_netbuf,
                                           puc_data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
            }
#endif
            break;
        }
        default:
            break;
    }
}

OAL_STATIC oal_void hmac_ap_up_category_ht(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf)
{
    /* Action */
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        case MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH:
            hmac_rx_notify_channel_width(&(pst_hmac_vap->st_vap_base_info), pst_netbuf);
            break;
#endif
        case MAC_HT_ACTION_BUTT:
        default:
            break;
    }
}
OAL_STATIC oal_void hmac_ap_up_category_vht(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf)
{
    switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        case MAC_VHT_ACTION_OPREATE_MODE_NOTIFY:
            hmac_mgmt_rx_opmode_notify_frame(pst_hmac_vap, pst_netbuf);
            break;
#endif
        case MAC_VHT_ACTION_BUTT:
        default:
            break;
    }
}
OAL_STATIC oal_void hmac_ap_up_category_vendor(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf)
{
    /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
    /* 并用hmac_rx_mgmt_send_to_host接口上报 */
    if (mac_ie_check_p2p_action(puc_data + MAC_ACTION_OFFSET_CATEGORY) == OAL_TRUE) {
        hmac_rx_mgmt_send_to_host(pst_hmac_vap, pst_netbuf);
    }
}
#ifdef _PRE_WLAN_FEATURE_WMMAC
OAL_STATIC oal_void hmac_ap_up_category_wmmac_qos(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf)
{
    if (g_en_wmmac_switch == OAL_TRUE) {
        switch (puc_data[MAC_ACTION_OFFSET_ACTION]) {
            case MAC_WMMAC_ACTION_ADDTS_REQ:
                hmac_mgmt_rx_addts_req_frame(pst_hmac_vap, pst_netbuf);
                break;
            default:
                break;
        }
    }
}
#endif
OAL_STATIC oal_void hmac_ap_up_pub_vendor_specific(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_data,
    oal_netbuf_stru *pst_netbuf)
{
#ifdef _PRE_WLAN_FEATURE_LOCATION_RAM
    if (oal_memcmp(puc_data + MAC_ACTION_CATEGORY_AND_CODE_LEN, g_auc_huawei_oui, MAC_OUI_LEN) == 0) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_action::hmac location get.}");
        hmac_huawei_action_process(pst_hmac_vap, pst_netbuf,
                                   puc_data[MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN]);
    }
#endif /* _PRE_WLAN_FEATURE_FTM */
}


OAL_STATIC oal_void hmac_ap_up_rx_action(
    hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf, oal_bool_enum_uint8 en_is_protected)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = NULL;
    oal_uint8 *puc_data = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL; /* 保存mac帧的指针 */
    hmac_user_stru *pst_hmac_user = NULL;
    oal_uint32 frame_body_len = hmac_get_frame_body_len(pst_netbuf);

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    if (frame_body_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{ap_up::frame len too short[%d].}", pst_rx_ctrl->st_rx_info.us_frame_len);
        return;
    }

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr;

    /* 获取发送端的用户指针 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(&pst_hmac_vap->st_vap_base_info,
        pst_frame_hdr->auc_address2, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_ap_up_rx_action::mac_vap_find_user_by_macaddr failed.}");
        hmac_ap_up_rx_action_nonuser(pst_hmac_vap, pst_netbuf);
        return;
    }

    /* 获取帧体指针 */
    puc_data = (oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* Category */
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA: {
            hmac_ap_up_category_ba(pst_hmac_vap, puc_data, pst_hmac_user, frame_body_len);
        }
        break;

        case MAC_ACTION_CATEGORY_PUBLIC: {
            hmac_ap_up_category_public(pst_hmac_vap, puc_data, pst_netbuf, frame_body_len);
        }
        break;

        case MAC_ACTION_CATEGORY_HT: {
            hmac_ap_up_category_ht(pst_hmac_vap, puc_data, pst_netbuf);
        }
        break;
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
        case MAC_ACTION_CATEGORY_SA_QUERY: {
            hmac_ap_up_category_sa_query(pst_hmac_vap, puc_data, pst_netbuf, en_is_protected);
        }
        break;
#endif
        case MAC_ACTION_CATEGORY_VHT: {
            hmac_ap_up_category_vht(pst_hmac_vap, puc_data, pst_netbuf);
        }
        break;

        case MAC_ACTION_CATEGORY_VENDOR: {
            if (frame_body_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_action_category_vendor::frame_body_len %d.}", frame_body_len);
                return;
            }
            hmac_ap_up_category_vendor(pst_hmac_vap, puc_data, pst_netbuf);
        }
        break;

#ifdef _PRE_WLAN_FEATURE_WMMAC
        case MAC_ACTION_CATEGORY_WMMAC_QOS: {
            hmac_ap_up_category_wmmac_qos(pst_hmac_vap, puc_data, pst_netbuf);
        }
        break;
#endif  // _PRE_WLAN_FEATURE_WMMAC

        case MAC_PUB_VENDOR_SPECIFIC: {
            hmac_ap_up_pub_vendor_specific(pst_hmac_vap, puc_data, pst_netbuf);
        }
            break;
        default:
            break;
    }
}


OAL_STATIC oal_void hmac_ap_up_rx_probe_req(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl;
    mac_rx_ctl_stru *pst_rx_info;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    enum nl80211_band en_band;
#else
    enum ieee80211_band en_band;
#endif
    oal_int l_freq;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));

    /* 获取AP 当前信道 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
    if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
        en_band = NL80211_BAND_2GHZ;
    } else if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_5G) {
        en_band = NL80211_BAND_5GHZ;
    } else {
        en_band = NUM_NL80211_BANDS;
    }
#else
    if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G) {
        en_band = IEEE80211_BAND_2GHZ;
    } else if (pst_hmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_5G) {
        en_band = IEEE80211_BAND_5GHZ;
    } else {
        en_band = IEEE80211_NUM_BANDS;
    }
#endif
    l_freq = oal_ieee80211_channel_to_frequency(pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                                                en_band);

    /* 上报接收到的probe req 管理帧 */
    hmac_send_mgmt_to_host(pst_hmac_vap, pst_netbuf, pst_rx_info->us_frame_len, l_freq);
}


oal_uint32 hmac_ap_up_rx_mgmt(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_info = OAL_PTR_NULL;
    oal_uint8 *puc_mac_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_payload = OAL_PTR_NULL;
    oal_uint32 ul_msg_len;     /* 消息总长度,不包括FCS */
    oal_uint32 ul_mac_hdr_len; /* MAC头长度 */
    oal_uint8 uc_mgmt_frm_type;
    oal_bool_enum_uint8 en_is_protected;
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    oal_uint8 *puc_sa;
    oal_bool_enum_uint8 en_blacklist_result;
#endif
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)p_param;

    if (oal_any_null_ptr2(pst_hmac_vap, p_param)) {
        oam_error_log2(0, OAM_SF_RX, "hmac_ap_up_rx_mgmt:null, %x %x", (uintptr_t)pst_hmac_vap, (uintptr_t)p_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    puc_mac_hdr = (oal_uint8 *)(pst_rx_info->pul_mac_hdr_start_addr);
    ul_mac_hdr_len = pst_rx_info->uc_mac_header_len; /* MAC头长度 */
    puc_payload = (oal_uint8 *)(puc_mac_hdr) + ul_mac_hdr_len;
    ul_msg_len = pst_rx_info->us_frame_len; /* 消息总长度,不包括FCS */
    en_is_protected = (oal_uint8)mac_get_protectedframe(puc_mac_hdr);

    /* AP在UP状态下 接收到的各种管理帧处理 */
    uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{wifi_wake_src:hmac_ap_up_rx_mgmt::wakeup mgmt type[0x%x]}", uc_mgmt_frm_type);
    }
#endif

    /* Bar frame proc here */
    if (mac_get_frame_type(puc_mac_hdr) == WLAN_FC0_TYPE_CTL) {
        uc_mgmt_frm_type = mac_get_frame_sub_type(puc_mac_hdr);
        if ((uc_mgmt_frm_type >> 4) == WLAN_BLOCKACK_REQ) { /* 判断uc_mgmt_frm_type高4位是否为 bar的值(1000) */
            hmac_up_rx_bar(pst_hmac_vap, pst_rx_ctrl, pst_mgmt_rx_event->pst_netbuf);
        }
    }

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    mac_rx_get_sa((mac_ieee80211_frame_stru *)puc_mac_hdr, &puc_sa);

    /* 自动加入黑名单检查 */
    if ((uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ) || (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ)) {
        hmac_autoblacklist_filter(&pst_hmac_vap->st_vap_base_info, puc_sa);
    }

    /* 黑名单过滤检查 */
    en_blacklist_result = hmac_blacklist_filter(&pst_hmac_vap->st_vap_base_info, puc_sa);
    if (en_blacklist_result == OAL_TRUE) {
#ifdef _PRE_WLAN_NARROW_BAND
        if (!g_hitalk_status)
#endif
        {
            /* 窄带APUT模式下，会造成STA无法关联，需继续定位 */
            return OAL_SUCC;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, puc_mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
    switch (uc_mgmt_frm_type) {
        case WLAN_FC0_SUBTYPE_BEACON:
        case WLAN_FC0_SUBTYPE_PROBE_RSP:
            hmac_ap_up_rx_obss_beacon(pst_hmac_vap, pst_mgmt_rx_event->pst_netbuf);
            break;
        case WLAN_FC0_SUBTYPE_AUTH:
            hmac_ap_rx_auth_req(pst_hmac_vap, pst_mgmt_rx_event->pst_netbuf, pst_mgmt_rx_event);
            break;
        case WLAN_FC0_SUBTYPE_DEAUTH:
            if (ul_msg_len < ul_mac_hdr_len + MAC_80211_REASON_CODE_LEN) {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_ap_up_rx_mgmt::invalid deauth_req length[%d]}", ul_msg_len);
                break;
            }
            hmac_ap_rx_deauth_req(pst_hmac_vap, puc_mac_hdr, en_is_protected);
            break;
        case WLAN_FC0_SUBTYPE_ASSOC_REQ:
        case WLAN_FC0_SUBTYPE_REASSOC_REQ:
            hmac_ap_up_rx_asoc_req(pst_hmac_vap, uc_mgmt_frm_type, puc_mac_hdr, ul_mac_hdr_len,
                                   puc_payload, (ul_msg_len - ul_mac_hdr_len));
            break;
        case WLAN_FC0_SUBTYPE_DISASSOC:
            hmac_ap_up_rx_disasoc(pst_hmac_vap, puc_mac_hdr, ul_mac_hdr_len, puc_payload,
                                  (ul_msg_len - ul_mac_hdr_len), en_is_protected);
            break;
        case WLAN_FC0_SUBTYPE_ACTION:
            hmac_ap_up_rx_action(pst_hmac_vap, pst_mgmt_rx_event->pst_netbuf, en_is_protected);
            break;
        case WLAN_FC0_SUBTYPE_PROBE_REQ:
            hmac_ap_up_rx_probe_req(pst_hmac_vap, pst_mgmt_rx_event->pst_netbuf);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_ap_store_network(mac_device_stru *pst_mac_device, oal_netbuf_stru *pst_netbuf)
{
    mac_bss_id_list_stru *pst_bss_id_list = &pst_mac_device->st_bss_id_list;
    dmac_rx_ctl_stru *pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    oal_uint8 auc_network_bssid[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_bool_enum_uint8 en_already_present = OAL_FALSE;
    oal_uint8 uc_loop;

    /* 获取帧体中的BSSID */
    mac_get_bssid((oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr, auc_network_bssid, WLAN_MAC_ADDR_LEN);

    /* 忽略广播BSSID */
    if (oal_compare_mac_addr(BROADCAST_MACADDR, auc_network_bssid) == 0) {
        return;
    }

    /* 判断是否已经保存了该BSSID */
    for (uc_loop = 0; (uc_loop < pst_bss_id_list->us_num_networks) && (uc_loop < WLAN_MAX_SCAN_BSS_PER_CH); uc_loop++) {
        if (oal_compare_mac_addr(pst_bss_id_list->auc_bssid_array[uc_loop], auc_network_bssid) == 0) {
            en_already_present = OAL_TRUE;
            break;
        }
    }

    /* 来自一个新的BSS的帧，保存该BSSID */
    if ((en_already_present == OAL_FALSE) &&
        (pst_bss_id_list->us_num_networks < WLAN_MAX_SCAN_BSS_PER_CH) &&
        (uc_loop < WLAN_MAX_SCAN_BSS_PER_CH)) {
        oal_set_mac_addr((oal_uint8 *)pst_bss_id_list->auc_bssid_array[uc_loop], (oal_uint8 *)auc_network_bssid);
        pst_bss_id_list->us_num_networks++;
    }
}


OAL_STATIC oal_uint32 hmac_ap_get_chan_idx_of_network(mac_vap_stru *pst_mac_vap,
                                                      oal_uint8 *puc_payload,
                                                      oal_uint16 us_payload_len,
                                                      oal_uint8 uc_curr_chan_idx,
                                                      oal_uint8 *puc_chan_idx,
                                                      mac_sec_ch_off_enum_uint8 *pen_sec_ch_offset)
{
    wlan_channel_band_enum_uint8 en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8 uc_ch_idx = 0xFF;
    oal_uint32 ul_ret;
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    if (us_payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        *puc_chan_idx = uc_curr_chan_idx;

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_ap_get_chan_idx_of_network::payload_len[%d]}", us_payload_len);
        return OAL_SUCC;
    }

    us_payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    puc_payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_payload, us_payload_len);
    if (puc_ie != OAL_PTR_NULL) {
        ul_ret = mac_get_channel_idx_from_num(en_band, puc_ie[MAC_IE_HDR_LEN], &uc_ch_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{hmac_ap_get_chan_idx_of_network:get channel idx failed(band [%d], channel[%d])!}",
                             en_band, puc_ie[MAC_IE_HDR_LEN]);
            return ul_ret;
        }

        ul_ret = mac_is_channel_idx_valid(en_band, uc_ch_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "hmac_ap_get_chan_idx_of_network:channel_idx_valid failed(band[%d], channel[%d], idx[%d])",
                             en_band, puc_ie[MAC_IE_HDR_LEN], uc_ch_idx);
            return ul_ret;
        }

        *puc_chan_idx = uc_ch_idx;
        *pen_sec_ch_offset = puc_ie[MAC_IE_HDR_LEN + 1] & 0x03;

        return OAL_SUCC;
    }

    if (en_band == WLAN_BAND_2G) {
        puc_ie = mac_find_ie(MAC_EID_DSPARMS, puc_payload, us_payload_len);
        if (puc_ie != OAL_PTR_NULL) {
            ul_ret = mac_get_channel_idx_from_num(en_band, puc_ie[MAC_IE_HDR_LEN], &uc_ch_idx);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                                 "{hmac_ap_get_chan_idx_of_network:get channel idx failed(band [%d], channel[%d])!}",
                                 en_band, puc_ie[MAC_IE_HDR_LEN]);
                return ul_ret;
            }

            ul_ret = mac_is_channel_idx_valid(en_band, uc_ch_idx);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                                 "hmac_ap_get_chan_idx_of_network:channel_idx_valid failed, (channel[%d], idx[%d])",
                                 puc_ie[MAC_IE_HDR_LEN], uc_ch_idx);
                return ul_ret;
            }

            *puc_chan_idx = uc_ch_idx;

            return OAL_SUCC;
        }
    }

    *puc_chan_idx = uc_curr_chan_idx;

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_ap_wait_start_rx_obss_beacon(mac_device_stru *pst_mac_device,
                                                      mac_vap_stru *pst_mac_vap,
                                                      oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    oal_uint8 *puc_payload;
    oal_uint16 us_payload_len;
    oal_uint8 uc_chan_idx = 0xFF;
    oal_uint8 uc_curr_chan_idx = pst_mac_device->uc_ap_chan_idx;
    mac_sec_ch_off_enum_uint8 en_sec_ch_offset = MAC_SCN;
    oal_uint32 ul_ret;

    /* 获取帧体长度 */
    us_payload_len = pst_rx_ctrl->st_rx_info.us_frame_len - pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* 获取帧体指针 */
    puc_payload =
        (oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr + pst_rx_ctrl->st_rx_info.uc_mac_header_len;

    /* 从帧体中获取信道索引和次信道偏移量 */
    ul_ret = hmac_ap_get_chan_idx_of_network(pst_mac_vap, puc_payload, us_payload_len, uc_curr_chan_idx,
                                             &uc_chan_idx, &en_sec_ch_offset);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                         "{hmac_ap_wait_start_rx_obss_beacon::hmac_ap_get_chan_idx_of_network failed[%d].}", ul_ret);
        return;
    }

    if (uc_chan_idx != uc_curr_chan_idx) {
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                      "{hmac_ap_wait_start_rx_obss_beacon::The channel of this BSS not matched to current channel.}");
        return;
    }

    hmac_ap_store_network(pst_mac_device, pst_netbuf);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (hmac_ap_update_2040_chan_info(pst_mac_device, pst_mac_vap, puc_payload, us_payload_len,
                                      uc_chan_idx, en_sec_ch_offset) == OAL_FALSE) {
        /* do nothing */
    }
#endif
}


oal_uint32 hmac_ap_wait_start_rx_mgmt(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    oal_uint8 uc_mgmt_frm_type;

    if ((pst_hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_RX,
                       "{hmac_ap_wait_start_rx_mgmt::param null, %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_ap_wait_start_rx_mgmt::pst_mac_device null.}");
        return OAL_FALSE;
    }

    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)p_param;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);

    /* 采集收到的任何帧所包含的BSSID */
    /* 获取管理帧类型 */
    uc_mgmt_frm_type = mac_get_frame_sub_type((oal_uint8 *)pst_rx_ctrl->st_rx_info.pul_mac_hdr_start_addr);

    /* AP在WAIT START状态下 接收到的各种管理帧处理 */
    switch (uc_mgmt_frm_type) {
        case WLAN_FC0_SUBTYPE_BEACON:
        case WLAN_FC0_SUBTYPE_PROBE_RSP:
            hmac_ap_wait_start_rx_obss_beacon(pst_mac_device, &(pst_hmac_vap->st_vap_base_info),
                                              pst_mgmt_rx_event->pst_netbuf);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_mgmt_timeout_ap(oal_void *p_param)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user;
    oal_uint32 ul_ret;

    pst_hmac_user = (hmac_user_stru *)p_param;
    if (pst_hmac_user == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_AUTH, "{hmac_mgmt_timeout_ap::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_hmac_user->st_user_base_info.uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_mgmt_timeout_ap::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                     "{hmac_mgmt_timeout_ap::Wait AUTH timeout!! After %d ms.}", WLAN_AUTH_TIMEOUT);
#ifdef _PRE_WLAN_1102A_CHR
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT, MAC_AP_AUTH_RSP_TIMEOUT);
#endif
    /* 发送去关联帧消息给STA */
    hmac_mgmt_send_deauth_frame(&pst_hmac_vap->st_vap_base_info, pst_hmac_user->st_user_base_info.auc_user_mac_addr,
                                WLAN_MAC_ADDR_LEN, MAC_AUTH_NOT_VALID, OAL_FALSE);

    ul_ret = hmac_user_del(&pst_hmac_vap->st_vap_base_info, pst_hmac_user);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_mgmt_timeout_ap::hmac_user_del failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 hmac_ap_wait_start_misc(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    hmac_misc_input_stru *pst_misc_input = OAL_PTR_NULL;

    if (oal_unlikely((pst_hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_RX,
                       "{hmac_ap_wait_start_misc::param null, %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_misc_input = (hmac_misc_input_stru *)p_param;

    switch (pst_misc_input->en_type) {
        case HMAC_MISC_RADAR:
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


oal_uint32 hmac_ap_up_misc(hmac_vap_stru *pst_hmac_vap, oal_void *p_param)
{
    hmac_misc_input_stru *pst_misc_input = OAL_PTR_NULL;

    if (oal_unlikely((pst_hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_RX,
                       "{hmac_ap_up_misc::param null, %x %x.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_misc_input = (hmac_misc_input_stru *)p_param;

    switch (pst_misc_input->en_type) {
        case HMAC_MISC_RADAR:
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

oal_uint32 hmac_ap_clean_bss(hmac_vap_stru *pst_hmac_vap)
{
    oal_dlist_head_stru *pst_user_list_head = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_user_stru *pst_user_tmp = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user_tmp = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_is_protected;

    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_ap_clean_bss::hmac vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除vap下所有已关联用户，并通知内核 */
    pst_user_list_head = &pst_hmac_vap->st_vap_base_info.st_mac_user_list_head;
    pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    for (pst_entry = pst_user_list_head->pst_next; pst_entry != pst_user_list_head;) {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        pst_hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
        if (pst_hmac_user_tmp == OAL_PTR_NULL) {
            continue;
        }

        /* 指向双向链表下一个 */
        pst_entry = pst_entry->pst_next;

        /* 管理帧加密是否开启 */
        en_is_protected = pst_user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_user_tmp->auc_user_mac_addr, MAC_DISAS_LV_SS, en_is_protected);

        /* 通知内核 */
        hmac_handle_disconnect_rsp_ap(pst_hmac_vap, pst_hmac_user_tmp);

        /* 删除用户 */
        hmac_user_del(pst_mac_vap, pst_hmac_user_tmp);
    }

    return OAL_SUCC;
}

void hmac_ap_report_sta_leave(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    if (oal_any_null_ptr2(hmac_vap, hmac_user)) {
        return;
    }

    if (hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        oal_net_device_stru *net_device = hmac_vap_get_net_device(hmac_vap->st_vap_base_info.uc_vap_id);
        if (net_device != NULL) {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
            oal_kobject_uevent_env_sta_leave(net_device, hmac_user->st_user_base_info.auc_user_mac_addr);
#endif
#endif
        }
    }
}
