

/* 1 头文件包含 */
#include "wlan_mib.h"
#include "mac_vap.h"
#include "mac_frame.h"
#include "hmac_encap_frame_sta.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_scan.h"
#include "mac_ie.h"
#include "mac_11kvr.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_frame.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ENCAP_FRAME_STA_C

/* 2 全局变量定义 */
#define MAC_MGMT_CHALLENGE_TEXT 8

/* 3 函数实现 */

hmac_scanned_bss_info *hmac_vap_get_scan_bss_info(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru       *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru     *pst_bss_mgmt = OAL_PTR_NULL;
    hmac_scanned_bss_info  *pst_scaned_bss = OAL_PTR_NULL;

    /* 入参合法性判断 */
    if (oal_unlikely(pst_mac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_vap_get_scan_bss_info:: input pointer is null!}");
        return OAL_PTR_NULL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_vap_get_scan_bss_info::pst_mac_device null.}");
        return OAL_PTR_NULL;
    }
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    pst_scaned_bss = hmac_scan_find_scanned_bss_by_bssid(pst_bss_mgmt, pst_mac_vap->auc_bssid);
    if (pst_scaned_bss == OAL_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{vap_get_scan_bss_info::do not have scan result!!!}");
        return OAL_PTR_NULL;
    }
    return pst_scaned_bss;
}


OAL_STATIC oal_bool_enum hmac_sta_check_need_set_ext_cap_ie(mac_vap_stru *pst_mac_vap)
{
    oal_uint8 *puc_ext_cap_ie;
    oal_uint16 us_ext_cap_index;

    puc_ext_cap_ie = hmac_sta_find_ie_in_probe_rsp(pst_mac_vap, MAC_EID_EXT_CAPS, &us_ext_cap_index);
    if (puc_ext_cap_ie == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


oal_void hmac_set_supported_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_buffer,
                                              oal_uint8 *puc_ie_len)
{
    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;

    /**************************************************************************
                        ---------------------------------------
                        |Element ID | Length | Supported Rates|
                        ---------------------------------------
             Octets:    |1          | 1      | 1~8            |
                        ---------------------------------------
    The Information field is encoded as 1 to 8 octets, where each octet describes a single Supported
    Rate or BSS membership selector.
    **************************************************************************/
    puc_buffer[0] = MAC_EID_RATES;

    uc_nrates = pst_hmac_sta->uc_rs_nrates;

    if (uc_nrates > MAC_MAX_SUPRATES) {
        uc_nrates = MAC_MAX_SUPRATES;
    }

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_hmac_sta->auc_supp_rates[uc_idx];
    }

    puc_buffer[1] = uc_nrates;

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}

oal_void hmac_set_exsup_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_buffer,
                                          oal_uint8 *puc_ie_len)
{
    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    if (pst_hmac_sta->uc_rs_nrates <= MAC_MAX_SUPRATES) {
        *puc_ie_len = 0;

        return;
    }

    puc_buffer[0] = MAC_EID_XRATES;
    uc_nrates = pst_hmac_sta->uc_rs_nrates - MAC_MAX_SUPRATES;
    puc_buffer[1] = uc_nrates;

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_hmac_sta->auc_supp_rates[uc_idx + MAC_MAX_SUPRATES];
    }

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}

static en_app_ie_type_uint8 hmac_sta_get_assoc_req_app_ie_type(hmac_vap_stru *hmac_vap, uint8_t is_reassoc)
{
    en_app_ie_type_uint8 en_app_ie_type = OAL_APP_ASSOC_REQ_IE;
    wlan_mib_ieee802dot11_stru *mib_info = hmac_vap->st_vap_base_info.pst_mib_info;
    if (is_reassoc) {
        en_app_ie_type = OAL_APP_REASSOC_REQ_IE;
#ifdef _PRE_WLAN_FEATURE_11R
        if (hmac_vap->bit_11r_enable == OAL_TRUE &&
            mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated == OAL_TRUE) {
            en_app_ie_type = OAL_APP_FT_IE;
        }
#endif
    }
    return en_app_ie_type;
}

void hmac_set_assoc_req_ssid_ie(hmac_vap_stru *hmac_vap, oal_uint8 *req_frame, oal_uint8 *ie_len, oal_uint8 *dest_addr)
{
#ifdef _PRE_WLAN_CHBA_MGMT
        if (hmac_chba_vap_start_check(hmac_vap) == OAL_TRUE) {
            hmac_chba_assoc_set_ssid(hmac_vap, req_frame, ie_len, dest_addr);
        } else {
#endif
            mac_set_ssid_ie((oal_void *)&hmac_vap->st_vap_base_info, req_frame, ie_len, WLAN_FC0_SUBTYPE_ASSOC_REQ);
#ifdef _PRE_WLAN_CHBA_MGMT
        }
#endif
}

oal_uint32 hmac_mgmt_encap_asoc_req_sta(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_req_frame,
                                        oal_uint8 *puc_curr_bssid, uint8_t *dest_addr)
{
    oal_uint16              us_fc;
    oal_uint8               uc_ie_len            = 0;
    oal_uint32              us_asoc_rsq_len      = 0;
    oal_uint8              *puc_req_frame_origin = OAL_PTR_NULL;
    mac_vap_stru           *pst_mac_vap          = OAL_PTR_NULL;
    oal_uint16              us_app_ie_len        = 0;
    en_app_ie_type_uint8    en_app_ie_type;
    hmac_scanned_bss_info  *pst_scaned_bss       = OAL_PTR_NULL;
    mac_txbf_cap_stru      *pst_txbf_cap         = OAL_PTR_NULL;
    wlan_wme_ac_type_enum_uint8 en_aci;
    wlan_wme_ac_type_enum_uint8 en_target_ac;
    oal_uint8                   uc_tid;
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_scanned_bss_info  st_scaned_bss;
#endif

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    oal_bool_enum_uint8         en_rrm_enable = OAL_TRUE;
#endif
    if (oal_any_null_ptr3(pst_hmac_sta, puc_req_frame, dest_addr)) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_req_sta:null pointer.}");
        return us_asoc_rsq_len;
    }

    /* 保存起始地址，方便计算长度 */
    puc_req_frame_origin = puc_req_frame;

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);

    pst_scaned_bss = hmac_vap_get_scan_bss_info(pst_mac_vap);
#ifdef _PRE_WLAN_CHBA_MGMT
    if (hmac_chba_vap_start_check(pst_hmac_sta) == OAL_TRUE) {
        memset_s(&st_scaned_bss, sizeof(hmac_scanned_bss_info), 0, sizeof(hmac_scanned_bss_info));
        pst_scaned_bss = &st_scaned_bss;
        hmac_chba_set_assoc_scan_bss(pst_hmac_sta, pst_scaned_bss);
    }
#endif
    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    /* 设置 Frame Control field */
    /* 判断是否为reassoc操作 */
    us_fc = (puc_curr_bssid != OAL_PTR_NULL) ?
            WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_REASSOC_REQ :
            WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ASSOC_REQ;
    mac_hdr_set_frame_control(puc_req_frame, us_fc);
    /* 设置 DA address1: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR1_OFFSET, dest_addr);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR2_OFFSET,
                     pst_hmac_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR3_OFFSET, pst_hmac_sta->st_vap_base_info.auc_bssid);

    puc_req_frame += MAC_80211_FRAME_LEN;

    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Association Request Frame - Frame Body */
    /* --------------------------------------------------------------------- */
    /* | Capability Information | Listen Interval | SSID | Supported Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2-34  |3-10             | */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |Externed Surpported rates| Power Capability | Supported Channels   | */
    /* --------------------------------------------------------------------- */
    /* |3-257                    |4                 |4-256                 | */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | RSN   | QoS Capability | HT Capabilities | Extended Capabilities  | */
    /* --------------------------------------------------------------------- */
    /* |36-256 |3               |28               |3-8                     | */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | WPS   | P2P | */
    /* --------------------------------------------------------------------- */
    /* |7-257  |X    | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    mac_set_cap_info_sta((oal_void *)pst_mac_vap, puc_req_frame);
    puc_req_frame += MAC_CAP_INFO_LEN;

    /* 设置 Listen Interval IE */
    mac_set_listen_interval_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* Ressoc组帧设置Current AP address */
    if (puc_curr_bssid != OAL_PTR_NULL) {
        oal_set_mac_addr(puc_req_frame, puc_curr_bssid);
        puc_req_frame += OAL_MAC_ADDR_LEN;
    }
    /* 设置 SSID IE */
    hmac_set_assoc_req_ssid_ie(pst_hmac_sta, puc_req_frame, &uc_ie_len, dest_addr);
    puc_req_frame += uc_ie_len;
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    /* 设置 Supported Rates IE */
    mac_set_supported_rates_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* 设置 Extended Supported Rates IE */
    mac_set_exsup_rates_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

#else
    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_asoc_req(pst_hmac_sta, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* 设置 Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_asoc_req(pst_hmac_sta, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;
#endif
    /* 设置 Power Capability IE */
    mac_set_power_cap_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* 设置 Supported channel IE */
    mac_set_supported_channel_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* 填充WMM element */
    if (pst_hmac_sta->uc_wmm_cap == OAL_TRUE) {
        mac_set_wmm_ie_sta((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    /* 设置 HT Capability IE */
    mac_set_ht_capabilities_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);

    if ((pst_mac_vap->bit_ap_11ntxbf == OAL_TRUE) &&
        (pst_mac_vap->st_cap_flag.bit_11ntxbf == OAL_TRUE) &&
        (uc_ie_len != 0)) {
        puc_req_frame += MAC_11N_TXBF_CAP_OFFSET;

        pst_txbf_cap = (mac_txbf_cap_stru *)puc_req_frame;
        pst_txbf_cap->bit_rx_stagg_sounding = OAL_TRUE;
        pst_txbf_cap->bit_explicit_compr_bf_fdbk = 1;
        pst_txbf_cap->bit_compr_steering_num_bf_antssup = 1;
        puc_req_frame -= MAC_11N_TXBF_CAP_OFFSET;
    }
    puc_req_frame += uc_ie_len;

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    if (pst_scaned_bss != OAL_PTR_NULL) {
#ifndef _PRE_WLAN_FEATURE_11KV_INTERFACE
        en_rrm_enable = pst_hmac_sta->bit_11k_enable;
#endif
        en_rrm_enable = en_rrm_enable && pst_scaned_bss->st_bss_dscr_info.en_support_rrm;
        if ((pst_hmac_sta->bit_11k_auth_flag == OAL_TRUE) || (en_rrm_enable == OAL_TRUE)) {
            mac_set_rrm_enabled_cap_field((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        }
    }
#endif  // _PRE_WLAN_FEATURE_11K

    /* 设置 Extended Capability IE */
    if (hmac_sta_check_need_set_ext_cap_ie(pst_mac_vap) == OAL_TRUE) {
        mac_set_ext_capabilities_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    /* 设置 VHT Capability IE */
    if ((pst_scaned_bss != OAL_PTR_NULL) && (pst_scaned_bss->st_bss_dscr_info.en_vht_capable == OAL_TRUE) &&
        (pst_scaned_bss->st_bss_dscr_info.en_epigram_vht_capable == OAL_FALSE)) {
        mac_set_vht_capabilities_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
        mac_ie_modify_vht_ie_short_gi_80(pst_mac_vap,
            puc_req_frame_origin + MAC_80211_FRAME_LEN + MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN, puc_req_frame);
    }

    mac_set_11ntxbf_vendor_ie(pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* 填充 epigram Vendor VHT IE,解决与epigram AP的私有协议对通问题 */
    if ((pst_scaned_bss != OAL_PTR_NULL) && (pst_scaned_bss->st_bss_dscr_info.en_epigram_vht_capable == OAL_TRUE)) {
        mac_set_epigram_vht_ie(pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
    if ((pst_scaned_bss != OAL_PTR_NULL) && (pst_scaned_bss->st_bss_dscr_info.en_epigram_novht_capable == OAL_TRUE)) {
        mac_set_epigram_novht_ie(pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

#ifdef _PRE_WLAN_NARROW_BAND
    if (g_hitalk_status & NARROW_BAND_ON_MASK) {
        if ((pst_scaned_bss != OAL_PTR_NULL) && (pst_scaned_bss->st_bss_dscr_info.en_nb_capable == OAL_TRUE) &&
            (pst_mac_vap->st_nb.en_open == OAL_TRUE) && (pst_mac_vap->st_cap_flag.bit_nb == OAL_TRUE)) {
            mac_set_nb_ie(puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        }
    }
#endif

    en_app_ie_type = hmac_sta_get_assoc_req_app_ie_type(pst_hmac_sta, (puc_curr_bssid != NULL));
    /* 填充P2P/WPS IE 信息 */
    mac_add_app_ie(pst_mac_vap, puc_req_frame, &us_app_ie_len, en_app_ie_type);
    puc_req_frame += us_app_ie_len;

#ifdef _PRE_WLAN_FEATURE_11R
    if (pst_hmac_sta->bit_11r_enable == OAL_TRUE) {
        /* Q版本关联时wpa_s下发IE中携带MD IE, P版本不携带，此处先判断是否已经携带，避免重复添加MD IE */
        if (((pst_hmac_sta->bit_reassoc_flag == OAL_FALSE) &&
            mac_find_ie(MAC_EID_MOBILITY_DOMAIN,
            pst_mac_vap->ast_app_ie[en_app_ie_type].puc_ie, pst_mac_vap->ast_app_ie[en_app_ie_type].ul_ie_len) ==
            OAL_PTR_NULL)
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
            ||  (pst_hmac_sta->bit_voe_11r_auth == 1)) /* voe 11r 认证实验室环境必须携带两个mdie否则无法正常漫游 */
#else
)
#endif
        {
            mac_set_md_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        } else { /* Reasoc中包含RIC-Req */
            for (en_aci = WLAN_WME_AC_BE; en_aci < WLAN_WME_AC_BUTT; en_aci++) {
                if (pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[en_aci].en_dot11QAPEDCATableMandatory) {
                    en_target_ac = en_aci;
                    uc_tid = WLAN_WME_AC_TO_TID(en_target_ac);
                    mac_set_rde_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
                    puc_req_frame += uc_ie_len;

                    mac_set_tspec_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len, uc_tid);
                    puc_req_frame += uc_ie_len;
                }
            }
        }
    }
#endif  // _PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_CHBA_MGMT
    if (hmac_chba_vap_start_check(pst_hmac_sta) == OAL_TRUE) {
        uc_ie_len = hmac_chba_encape_assoc_attribute(puc_req_frame, 0);
        puc_req_frame += uc_ie_len;
    }
#endif

    us_asoc_rsq_len = (oal_uint32)(puc_req_frame - puc_req_frame_origin);

    return us_asoc_rsq_len;
}

#define FASTBSSTACTIVE pst_hsta->st_vap_base_info.pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.\
    en_dot11FastBSSTransitionActivated

oal_uint16 hmac_mgmt_encap_auth_req(hmac_vap_stru *pst_hsta, oal_uint8 *puc_mgmt_frame, uint8_t *dest_addr)
{
    oal_uint16      us_auth_req_len;
    hmac_user_stru *pst_user_ap = OAL_PTR_NULL;
    oal_uint16      us_auth_type;
    oal_uint32      ul_ret;
    oal_uint16      us_user_index;
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint16      us_app_ie_len;
#endif // _PRE_WLAN_FEATURE_11R

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    mac_hdr_set_frame_control(puc_mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address1,
                     dest_addr);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address2,
                     pst_hsta->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address3,
                     pst_hsta->st_vap_base_info.auc_bssid);
    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Authentication Frame (Sequence 1) - Frame Body */
    /* -------------------------------------------------------------------- */
    /* |Auth Algorithm Number|Auth Transaction Sequence Number|Status Code| */
    /* -------------------------------------------------------------------- */
    /* | 2                   |2                               |2          | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    if (mac_mib_get_privacyinvoked(&pst_hsta->st_vap_base_info) == OAL_FALSE) {
        /* Open System */
        puc_mgmt_frame[MAC_80211_FRAME_LEN] = 0x00;
        puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
    } else {
        us_auth_type = (oal_uint16)pst_hsta->en_auth_mode;

        if (us_auth_type == WLAN_WITP_AUTH_SHARED_KEY) {
            us_auth_type = WLAN_WITP_AUTH_SHARED_KEY;
        } else {
            us_auth_type = WLAN_WITP_AUTH_OPEN_SYSTEM;
        }

        puc_mgmt_frame[MAC_80211_FRAME_LEN] = (us_auth_type & 0xFF);
        puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = ((us_auth_type & 0xFF00) >> 8); /* 保留us_auth_type的高8位放置于低8位 */
    }

    /* 设置 Authentication Transaction Sequence Number 为 1 */
    /* Authentication Transaction Sequence Number第1字节（MAC_80211_FRAME_LEN再偏移2byte）设置1 */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 2] = 0x01;
    /* Authentication Transaction Sequence Number第2字节（MAC_80211_FRAME_LEN再偏移3byte）置零 */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 3] = 0x00;

    /* 设置 Status Code 为0. 这个包的这个字段没用 . */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 4] = 0x00; /* Status Code第1字节（MAC_80211_FRAME_LEN再偏移4byte）置零 */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 5] = 0x00; /* Status Code第2字节（MAC_80211_FRAME_LEN再偏移5byte）置零 */

    /* 设置 认证帧的长度 */
    us_auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

#ifdef _PRE_WLAN_FEATURE_11R
    if (pst_hsta->bit_11r_enable == OAL_TRUE) {
        if ((FASTBSSTACTIVE == OAL_TRUE) && (pst_hsta->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING)) {
            /* FT System */
            puc_mgmt_frame[MAC_80211_FRAME_LEN] = 0x02;
            puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
            puc_mgmt_frame += us_auth_req_len;

            mac_add_app_ie((oal_void *)&pst_hsta->st_vap_base_info, puc_mgmt_frame, &us_app_ie_len, OAL_APP_FT_IE);
            us_auth_req_len += us_app_ie_len;
            puc_mgmt_frame += us_app_ie_len;
        }
    }

#endif  // _PRE_WLAN_FEATURE_11R

    pst_user_ap = (hmac_user_stru *)mac_res_get_hmac_user((oal_uint16)pst_hsta->st_vap_base_info.uc_assoc_vap_id);
    if (pst_user_ap == OAL_PTR_NULL) {
        ul_ret = hmac_user_add(&pst_hsta->st_vap_base_info, dest_addr, &us_user_index);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_hsta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_mgmt_encap_auth_req::hmac_user_add failed[%d].}", ul_ret);
            us_auth_req_len = 0;
        }
    }

    return us_auth_req_len;
}


oal_uint16 hmac_mgmt_encap_auth_req_seq3(hmac_vap_stru *pst_sta, oal_uint8 *puc_mgmt_frame, oal_uint8 *puc_mac_hrd)
{
    oal_uint8  *puc_data       = OAL_PTR_NULL;
    oal_uint16  us_index;
    oal_uint16  us_auth_req_len;
    oal_uint8  *puc_ch_text     = OAL_PTR_NULL;
    oal_uint8   uc_ch_text_len  = 0;

    /*************************************************************************/
    /* Management Frame Format */
    /* -------------------------------------------------------------------- */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS| */
    /* -------------------------------------------------------------------- */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  | */
    /* -------------------------------------------------------------------- */
    /*************************************************************************/
    /*************************************************************************/
    /* Set the fields in the frame header */
    /*************************************************************************/
    mac_hdr_set_frame_control(puc_mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    /* 将帧保护字段置1 */
    mac_set_wep(puc_mgmt_frame, 1);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address1, pst_sta->st_vap_base_info.auc_bssid);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address2,
                     pst_sta->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address3, pst_sta->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /* Set the contents of the frame body */
    /*************************************************************************/
    /*************************************************************************/
    /* Authentication Frame (Sequence 3) - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 获取认证帧payload */
    us_index = MAC_80211_FRAME_LEN;
    puc_data = (oal_uint8 *)(puc_mgmt_frame + us_index);

    /* 设置 认证帧的长度 */
    us_auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

    /* In case of no failure, the frame must be WEP encrypted. 4 bytes must */
    /* be   left for the  IV  in  that  case. These   fields will  then  be */
    /* reinitialized, using the correct index, with offset for IV field. */
    puc_data[0] = WLAN_WITP_AUTH_SHARED_KEY; /* Authentication Algorithm Number */
    puc_data[1] = 0x00;

    puc_data[2] = 0x03; /* Authentication Transaction Sequence Number第1位（puc_data第2byte）设置为3 */
    puc_data[3] = 0x00; /* Authentication Transaction Sequence Number第2位（puc_data第3byte）置零 */

    /* If WEP subfield in the  incoming  authentication frame is 1,  respond */
    /* with  'challenge text failure' status,  since the STA does not expect */
    /* an encrypted frame in this state. */
    if (mac_is_protectedframe(puc_mac_hrd) == 1) {
        puc_data[4] = MAC_CHLNG_FAIL; /* Status Code第1位（puc_data第4byte），如果受保护字段为1,则返回MAC_CHLNG_FAIL */
        puc_data[5] = 0x00; /* Status Code第2位（puc_data第5byte）置零 */
    } else if (mac_is_wep_enabled(&(pst_sta->st_vap_base_info)) == OAL_FALSE) {
    /* If the STA does not support WEP, respond with 'unsupported algo' */
    /* status, since WEP is necessary for Shared Key Authentication. */
        puc_data[4] = MAC_UNSUPT_ALG; /* Status Code第1位（puc_data第4byte），如果终端不支持WEP，则返回MAC_UNSUPT_ALG */
        puc_data[5] = 0x00; /* Status Code第2位（puc_data第5byte）置零 */
    } else if (mac_get_wep_default_keysize(&pst_sta->st_vap_base_info) == 0) {
    /* If the default WEP key is NULL, respond with 'challenge text failure' */
    /* status, since a NULL key value cannot be used for WEP operations. */
        puc_data[4] = MAC_CHLNG_FAIL; /* Status Code第1位（puc_data第4byte）如果默认的WEP密钥为NULL, 则返回MAC_CHLNG_FAIL */
        puc_data[5] = 0x00; /* Status Code第2位（puc_data第5byte）置零 */
    } else {
        /* If there is a mapping in dot11WEPKeyMappings matching the address of */
        /* the AP, and the corresponding key is NULL respond with 'challenge */
        /* text failure' status. This is currently not being used. */
        /* No error condition detected */
        /* Set Status Code to 'success' */
        puc_data[4] = MAC_SUCCESSFUL_STATUSCODE; /* Status Code第1位（puc_data第4byte），未检测错误情况，则返回成功 */
        puc_data[5] = 0x00; /* Status Code第2位（puc_data第5byte）置零 */

        /* Extract 'Challenge Text' and its 'length' from the incoming */
        /* authentication frame */
        uc_ch_text_len = puc_mac_hrd[MAC_80211_FRAME_LEN + 7]; /* 获取ch_text_len（MAC_80211_FRAME_LEN偏移7byte） */
        /* 获取ch_text（MAC_80211_FRAME_LEN偏移8byte） */
        puc_ch_text = (oal_uint8 *)(&puc_mac_hrd[MAC_80211_FRAME_LEN + 8]);

        /* Challenge Text Element */
        /* --------------------------------------- */
        /* |Element ID | Length | Challenge Text | */
        /* --------------------------------------- */
        /* | 1         |1       |1 - 253         | */
        /* --------------------------------------- */
        puc_mgmt_frame[us_index + 6] = MAC_EID_CHALLENGE; /* MAC_EID_CHALLENGE赋值到us_index偏移6byte */
        puc_mgmt_frame[us_index + 7] = uc_ch_text_len; /* ch_text_len赋值到us_index偏移7byte */
        if (memcpy_s(&puc_mgmt_frame[us_index + MAC_MGMT_CHALLENGE_TEXT],
            (WLAN_MEM_NETBUF_SIZE2 - (us_index + MAC_MGMT_CHALLENGE_TEXT)), puc_ch_text, uc_ch_text_len) != EOK) {
            OAM_ERROR_LOG0(0, 0, "{hmac_mgmt_encap_auth_req_seq3::memcpy fail}");
        }

        /* Add the challenge text element length to the authentication */
        /* request frame length. The IV, ICV element lengths will be added */
        /* after encryption. */
        us_auth_req_len += (uc_ch_text_len + MAC_IE_HDR_LEN);
    }

    return us_auth_req_len;
}
