

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "dmac_ext_if.h"
#include "mac_11kvr.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_EC_C

/*****************************************************************************
  2 函数原型声明
*****************************************************************************/
/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
/* RSNA OUI 定义 */
OAL_CONST oal_uint8 g_auc_rsn_oui[MAC_OUI_LEN] = { 0x00, 0x0F, 0xAC };

/* WPA OUI 定义 */
OAL_CONST oal_uint8 g_auc_wpa_oui[MAC_OUI_LEN] = { 0x00, 0x50, 0xF2 };

/* WMM OUI定义 */
OAL_CONST oal_uint8 g_auc_wmm_oui[MAC_OUI_LEN] = { 0x00, 0x50, 0xF2 };

/* WPS OUI 定义 */
OAL_CONST oal_uint8 g_auc_wps_oui[MAC_OUI_LEN] = { 0x00, 0x50, 0xF2 };

/* P2P OUI 定义 */
OAL_CONST oal_uint8 g_auc_p2p_oui[MAC_OUI_LEN] = { 0x50, 0x6F, 0x9A };

/* WFA TPC RPT OUI 定义 */
OAL_CONST oal_uint8 g_auc_wfa_oui[MAC_OUI_LEN] = { 0x00, 0x50, 0xF2 };

/* 窄带 OUI 定义,保持可变，避免将来冲突 */
oal_uint8 g_auc_huawei_oui[MAC_OUI_LEN] = { 0xac, 0x85, 0x3d };

#ifdef WIN32
/* mac_frame文件公共接口 */
mac_frame_rom_cb_stru g_st_mac_frame_rom_cb = {
    mac_set_rrm_enabled_cap_field_cb,
    mac_set_vht_capabilities_ie_cb,
    mac_set_ht_capabilities_ie_cb,
    mac_set_ext_capabilities_ie_cb,
    mac_set_rsn_ie_cb,
    mac_set_wpa_ie_cb
};
#else
/* mac_frame文件公共接口 */
mac_frame_rom_cb_stru g_st_mac_frame_rom_cb = {
    .p_mac_set_rrm_enabled_cap_field = mac_set_rrm_enabled_cap_field_cb,
    .p_mac_set_vht_capabilities_ie = mac_set_vht_capabilities_ie_cb,
    .p_mac_set_ht_capabilities_ie = mac_set_ht_capabilities_ie_cb,
    .p_mac_set_ext_capabilities_ie = mac_set_ext_capabilities_ie_cb,
    .p_mac_set_rsn_ie = mac_set_rsn_ie_cb,
    .p_mac_set_wpa_ie = mac_set_wpa_ie_cb
};
#endif
mac_frame_rom_cb_stru *mac_get_frame_rom_cb_addr(oal_void)
{
    return &g_st_mac_frame_rom_cb;
}

oal_uint32 g_ul_11h_test = OAL_FALSE;
oal_uint32 mac_get_11h_test(oal_void)
{
    return g_ul_11h_test;
}
oal_void mac_set_11h_test(oal_uint32 ul_11h_test)
{
    g_ul_11h_test = ul_11h_test;
}
/*****************************************************************************
  4 函数实现
*****************************************************************************/
oal_uint32 mac_get_customize_interworking(oal_void)
{
    return g_customize_interworking;
}
oal_void mac_set_customize_interworking(oal_uint32 customize_interworking)
{
    g_customize_interworking = customize_interworking;
}
#ifdef _PRE_WLAN_NARROW_BAND

oal_void mac_set_nb_ie(oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8 uc_index;

    /* 入参合法判断 */
    if (oal_unlikely((puc_buffer == OAL_PTR_NULL) || (puc_ie_len == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_set_nb_ie: input pointer is null!}");
        return;
    }

    /* ----------------------------------------------------------------- */
    /* NB Information/Parameter Element Format */
    /* ----------------------------------------------------------------- */
    /* EID | IE LEN | OUI | OUIType | Narrow Band| */
    /* ----------------------------------------------------------------- */
    /* 1   |   1    |  3  | 1       |3           | */
    /* ----------------------------------------------------------------- */
    /* 填写EID, 长度最后填 */
    puc_buffer[0] = MAC_EID_VENDOR;

    /* 初始化填写buffer的位置 */
    uc_index = MAC_IE_HDR_LEN;

    if (memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_huawei_oui, MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_nb_ie::memcpy fail!");
    }

    uc_index += MAC_OUI_LEN;

    puc_buffer[uc_index++] = MAC_HISI_NB_IE; /* oui_type */

    puc_buffer[uc_index++] = NARROW_BW_1M;
    puc_buffer[uc_index++] = NARROW_BW_5M;
    puc_buffer[uc_index++] = NARROW_BW_10M;

    /* 设置信息元素长度 */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;
    *puc_ie_len = uc_index;
}
#endif


oal_void mac_null_data_encap(oal_uint8 *header, oal_uint16 us_fc, oal_uint8 *puc_da, oal_uint8 *puc_sa)
{
    mac_hdr_set_frame_control(header, us_fc);

    if ((us_fc & WLAN_FRAME_FROM_AP) && !(us_fc & WLAN_FRAME_TO_AP)) {
        /* 设置ADDR1为DA */
        oal_set_mac_addr((header + 4), puc_da); /* puc_da地址赋值给header+4（DA） */

        /* 设置ADDR2为BSSID */
        oal_set_mac_addr((header + 10), puc_sa); /* puc_da地址赋值给header+10（BSSID） */

        /* 设置ADDR3为SA */
        oal_set_mac_addr((header + 16), puc_sa); /* puc_da地址赋值给header+16（SA） */
    }
    if (!(us_fc & WLAN_FRAME_FROM_AP) && (us_fc & WLAN_FRAME_TO_AP)) {
        /* 设置ADDR1为BSSID */
        oal_set_mac_addr((header + 4), puc_da); /* puc_da地址赋值给header+4（BSSID） */
        /* 设置ADDR2为SA */
        oal_set_mac_addr((header + 10), puc_sa); /* puc_da地址赋值给header+10（SA） */
        /* 设置ADDR3为DA */
        oal_set_mac_addr((header + 16), puc_da); /* puc_da地址赋值给header+16（DA） */
    }
}


oal_void mac_report_beacon(mac_rx_ctl_stru *pst_rx_cb, oal_netbuf_stru *pst_netbuf)
{
    oal_uint32 ul_ret;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8 *puc_beacon_payload_addr = OAL_PTR_NULL;

    puc_beacon_payload_addr = oal_netbuf_payload(pst_netbuf);
    ul_ret = oam_report_beacon_etc((oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_cb),
                                   pst_rx_cb->bit_mac_header_len,
                                   puc_beacon_payload_addr,
                                   pst_rx_cb->us_frame_len,
                                   OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#else
    ul_ret = oam_report_beacon((oal_uint8 *)pst_rx_cb->pul_mac_hdr_start_addr,
                               pst_rx_cb->uc_mac_header_len,
                               (oal_uint8 *)(pst_rx_cb->pul_mac_hdr_start_addr) + pst_rx_cb->uc_mac_header_len,
                               pst_rx_cb->us_frame_len,
                               OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#endif
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_WIFI_BEACON, "mac_report_beacon::oam_report_beacon_etc return err: 0x%x", ul_ret);
    }
}
static oal_uint32 mac_report_80211_probe_get_switch(oal_switch_enum_uint8 *frame_switch,
                                                    oal_switch_enum_uint8 *cb_switch,
                                                    oal_switch_enum_uint8 *dscr_switch)
{
    oal_uint32 ret;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    ret = oam_report_80211_probe_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                frame_switch,
                                                cb_switch,
                                                dscr_switch);
#else
    ret = oam_report_80211_probe_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                            frame_switch,
                                            cb_switch,
                                            dscr_switch);
#endif
    return ret;
}
static oal_uint32 mac_report_80211_mcast_get_switch(oal_uint8 frame_type,
                                                    oal_switch_enum_uint8 *frame_switch,
                                                    oal_switch_enum_uint8 *cb_switch,
                                                    oal_switch_enum_uint8 *dscr_switch)
{
    oal_uint32 ret;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    ret = oam_report_80211_mcast_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                frame_type,
                                                frame_switch,
                                                cb_switch,
                                                dscr_switch);
#else
    ret = oam_report_80211_mcast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                            frame_type,
                                            frame_switch,
                                            cb_switch,
                                            dscr_switch);
#endif
    return ret;
}
static oal_uint32 mac_report_80211_ucast_get_switch(oal_uint8 frame_type,
                                                    oal_switch_enum_uint8 *frame_switch,
                                                    oal_switch_enum_uint8 *cb_switch,
                                                    oal_switch_enum_uint8 *dscr_switch,
                                                    oal_uint16 user_idx)
{
    oal_uint32 ret;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    ret = oam_report_80211_ucast_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                frame_type,
                                                frame_switch,
                                                cb_switch,
                                                dscr_switch,
                                                user_idx);
#else
    ret = oam_report_80211_ucast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                            frame_type,
                                            frame_switch,
                                            cb_switch,
                                            dscr_switch,
                                            user_idx);
#endif
    return ret;
}

oal_uint32 mac_report_80211_get_switch(mac_vap_stru *pst_mac_vap,
                                       mac_rx_ctl_stru *pst_rx_cb,
                                       oal_switch_enum_uint8 *pen_frame_switch,
                                       oal_switch_enum_uint8 *pen_cb_switch,
                                       oal_switch_enum_uint8 *pen_dscr_switch)
{
    mac_ieee80211_frame_stru *pst_frame_hdr;
    oal_uint8 uc_frame_type = 0;
    oal_uint16 us_user_idx = 0xffff;
    oal_uint8 *puc_da = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (pst_frame_hdr == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((pst_frame_hdr->st_frame_control.bit_type == WLAN_CONTROL)
        || (pst_frame_hdr->st_frame_control.bit_type == WLAN_MANAGEMENT)) {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    }

    if (pst_frame_hdr->st_frame_control.bit_type == WLAN_DATA_BASICTYPE) {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_DATA;
    }

    /* probe request 和 probe response太多，单独过滤一次 */
    if (pst_frame_hdr->st_frame_control.bit_type == WLAN_MANAGEMENT) {
        if ((pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_REQ)
            || (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_PROBE_RSP)) {
            ul_ret = mac_report_80211_probe_get_switch(pen_frame_switch, pen_cb_switch, pen_dscr_switch);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch:: \
                    oam_report_80211_probe_get_switch_etc failed.}");

                return ul_ret;
            }

            return OAL_SUCC;
        } else if ((pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_DEAUTH)
                   || (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_DISASOC)) {
            *pen_cb_switch = 1;
            *pen_dscr_switch = 1;
            *pen_frame_switch = 1;
            return OAL_SUCC;
        } else if ((pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_ACTION)
                   || (pst_frame_hdr->st_frame_control.bit_sub_type == WLAN_ACTION_NO_ACK)) {
            *pen_cb_switch = 1;
            *pen_dscr_switch = 1;
            *pen_frame_switch = 1;
            return OAL_SUCC;
        }
    }

    mac_rx_get_da(pst_frame_hdr, &puc_da);
    if (ether_is_multicast(puc_da)) {
        ul_ret = mac_report_80211_mcast_get_switch(uc_frame_type, pen_frame_switch, pen_cb_switch, pen_dscr_switch);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_RX,
                             "{mac_report_80211_get_switch::oam_report_80211_mcast_get_switch_etc failed! \
                            ul_ret=[%d], frame_type=[%d]}", ul_ret, uc_frame_type);
            return ul_ret;
        }
    } else {
        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap,
                                              pst_frame_hdr->auc_address2, WLAN_MAC_ADDR_LEN,
                                              &us_user_idx);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            MAC_ERR_LOG(0, "mac_vap_find_user_by_macaddr return null ptr!");
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch:: \
                mac_vap_find_user_by_macaddr failed.}");
            return ul_ret;
        }

        if (ul_ret == OAL_FAIL) {
            *pen_cb_switch = 0;
            *pen_dscr_switch = 0;
            *pen_frame_switch = 0;

            return OAL_FAIL;
        }
        ul_ret = mac_report_80211_ucast_get_switch(uc_frame_type, pen_frame_switch, pen_cb_switch, pen_dscr_switch,
                                                   us_user_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                             "{mac_report_80211_get_switch::oam_report_80211_ucast_get_switch_etc failed! ul_ret=[%d], \
                             frame_type=[%d], user_idx=[%d]}", ul_ret, uc_frame_type, us_user_idx);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{oam_report_80211_ucast_get_switch_etc:: \
                frame_switch=[%x], cb_switch=[%x], dscr_switch=[%x]",
                             (uintptr_t)pen_frame_switch, (uintptr_t)pen_cb_switch, (uintptr_t)pen_dscr_switch);

            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_uint32 mac_report_80211_get_user_macaddr(mac_rx_ctl_stru *pst_rx_cb,
    oal_uint8 auc_user_macaddr[], uint8_t user_macaddr_len)
{
    mac_ieee80211_frame_stru *pst_frame_hdr;
    oal_uint8 *puc_da = OAL_PTR_NULL;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (pst_frame_hdr == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_report_80211_get_user_macaddr::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_rx_get_da(pst_frame_hdr, &puc_da);

    if (ether_is_multicast(puc_da)) {
        oal_set_mac_addr(auc_user_macaddr, BROADCAST_MACADDR);
    } else {
        oal_set_mac_addr(auc_user_macaddr, pst_frame_hdr->auc_address2);
    }

    return OAL_SUCC;
}


oal_uint32 mac_report_80211_frame(mac_vap_stru *pst_mac_vap,
                                  mac_rx_ctl_stru *pst_rx_cb,
                                  oal_netbuf_stru *pst_netbuf,
                                  oam_ota_type_enum_uint8 en_ota_type)
{
    oal_switch_enum_uint8 en_frame_switch = 0;
    oal_switch_enum_uint8 en_cb_switch = 0;
    oal_switch_enum_uint8 en_dscr_switch = 0;
    oal_uint32 ul_ret;
    oal_uint8 auc_user_macaddr[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_uint8* user_macaddr = auc_user_macaddr;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8 *puc_mac_payload_addr = OAL_PTR_NULL;
#endif

    /* 获取打印开关 */
    ul_ret = mac_report_80211_get_switch(pst_mac_vap,
                                         pst_rx_cb,
                                         &en_frame_switch,
                                         &en_cb_switch,
                                         &en_dscr_switch);
    if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame::get_switch failed.}");

        return ul_ret;
    }

    if (ul_ret == OAL_FAIL) {
        return ul_ret;
    }

    /* 获取发送端用户地址，用户SDT过滤,如果是组播\广播帧，则地址填为全F */
    ul_ret = mac_report_80211_get_user_macaddr(pst_rx_cb, auc_user_macaddr, WLAN_MAC_ADDR_LEN);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame:get_user_macaddr failed.}");

        return ul_ret;
    }

    /* 上报接收到的帧 */
    if (en_frame_switch == OAL_SWITCH_ON) {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        puc_mac_payload_addr = oal_netbuf_payload(pst_netbuf);

        ul_ret = oam_report_80211_frame_etc(auc_user_macaddr, WLAN_MAC_ADDR_LEN,
                                            (oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_cb),
                                            pst_rx_cb->bit_mac_header_len,
                                            puc_mac_payload_addr,
                                            pst_rx_cb->us_frame_len,
                                            OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#else
        ul_ret = oam_report_80211_frame(auc_user_macaddr, WLAN_MAC_ADDR_LEN,
                                        (oal_uint8 *)pst_rx_cb->pul_mac_hdr_start_addr,
                                        pst_rx_cb->uc_mac_header_len,
                                        (oal_uint8 *)(pst_rx_cb->pul_mac_hdr_start_addr) + pst_rx_cb->uc_mac_header_len,
                                        pst_rx_cb->us_frame_len,
                                        OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#endif
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_RX, "mac_report_80211_frame:oam_report_80211_frame_etc return err:0x%x", ul_ret);
        }
    }

    /* 上报接收帧的CB字段 */
    if (en_cb_switch == OAL_SWITCH_ON) {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        ul_ret = oam_report_netbuf_cb_etc(auc_user_macaddr, (oal_uint8 *)pst_rx_cb, en_ota_type);
#else
        ul_ret = oam_report_netbuf_cb(user_macaddr, (oal_uint8 *)pst_rx_cb, en_ota_type);
#endif
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_RX, "mac_report_80211_frame:oam_report_netbuf_cb_etc return err: 0x%x", ul_ret);
        }
    }
    return OAL_SUCC;
}


oal_uint32 mac_rx_report_80211_frame(oal_uint8 *pst_vap,
                                     oal_uint8 *pst_rx_cb,
                                     oal_netbuf_stru *pst_netbuf,
                                     oam_ota_type_enum_uint8 en_ota_type)
{
    oal_uint8 uc_sub_type;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_mac_rx_cb = OAL_PTR_NULL;

    if ((pst_rx_cb == OAL_PTR_NULL) || (pst_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{mac_rx_report_80211_frame::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = (mac_vap_stru *)pst_vap;
    pst_mac_rx_cb = (mac_rx_ctl_stru *)pst_rx_cb;

    uc_sub_type = mac_get_frame_sub_type((oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_mac_rx_cb));
    if (uc_sub_type == WLAN_FC0_SUBTYPE_BEACON) {
        mac_report_beacon(pst_mac_rx_cb, pst_netbuf);
    } else {
        mac_report_80211_frame(pst_mac_vap, pst_mac_rx_cb, pst_netbuf, en_ota_type);
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_P2P

oal_uint8 *mac_find_p2p_attribute(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    oal_int32 l_ie_len = 0;

    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 查找P2P IE，如果不是直接找下一个 */
    while ((l_len > MAC_P2P_ATTRIBUTE_HDR_LEN) && (puc_ies[0] != uc_eid)) {
        l_ie_len = (oal_int32)((puc_ies[2] << 8) + puc_ies[1]); /* puc_ies[2]左偏移8位 */
        l_len -= l_ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
        puc_ies += l_ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
    }

    // HWPSIRT-2021-51176
    if (l_len < MAC_P2P_ATTRIBUTE_HDR_LEN) {
        return NULL;
    }

    /* 查找到P2P IE，剩余长度不匹配直接返回空指针 */
    l_ie_len = (oal_int32)((puc_ies[2] << 8) + puc_ies[1]); /* puc_ies[2]左偏移8位 */
    if (l_len < (MAC_P2P_ATTRIBUTE_HDR_LEN + l_ie_len)) {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}
#endif

oal_uint8 *mac_find_ie(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    while ((l_len > MAC_IE_HDR_LEN) && (puc_ies[0] != uc_eid)) {
        l_len -= (oal_int32)(puc_ies[1] + MAC_IE_HDR_LEN);
        puc_ies += puc_ies[1] + MAC_IE_HDR_LEN;
    }

    if ((l_len < MAC_IE_HDR_LEN) || (l_len < (MAC_IE_HDR_LEN + puc_ies[1]))
        || ((l_len == MAC_IE_HDR_LEN) && (puc_ies[0] != uc_eid))) {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}


oal_uint8 *mac_find_vendor_ie(oal_uint32 ul_oui,
                              oal_uint8 uc_oui_type,
                              oal_uint8 *puc_ies,
                              oal_int32 l_len)
{
    struct mac_ieee80211_vendor_ie *pst_ie = OAL_PTR_NULL;
    oal_uint8 *puc_pos = OAL_PTR_NULL;
    oal_uint8 *puc_end = OAL_PTR_NULL;
    oal_uint32 ul_ie_oui;

    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    puc_pos = puc_ies;
    puc_end = puc_ies + l_len;
    while (puc_pos < puc_end) {
        puc_pos = mac_find_ie(MAC_EID_VENDOR, puc_pos, (oal_int32)(puc_end - puc_pos));
        if (puc_pos == OAL_PTR_NULL) {
            return OAL_PTR_NULL;
        }

        pst_ie = (struct mac_ieee80211_vendor_ie *)puc_pos;
        if (pst_ie->uc_len >= (sizeof(*pst_ie) - MAC_IE_HDR_LEN)) {
            /* auc_oui的0byte偏移16位，1byte偏移8位，同auc_oui[2]按位或运算赋值给ul_ie_oui */
            ul_ie_oui = (pst_ie->auc_oui[0] << 16) | (pst_ie->auc_oui[1] << 8) | pst_ie->auc_oui[2];
            if ((ul_ie_oui == ul_oui) && (pst_ie->uc_oui_type == uc_oui_type)) {
                return puc_pos;
            }
        }
        puc_pos += 2 + pst_ie->uc_len; /* 2表示信息元素头部 1字节EID + 1字节长度 */
    }
    return OAL_PTR_NULL;
}

oal_uint8 *mac_find_ie_ext_ie(oal_uint8 uc_eid, oal_uint8 uc_ext_ie, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (puc_ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    while ((l_len > MAC_IE_EXT_HDR_LEN)
           && (puc_ies[0] != uc_eid)
           && (puc_ies[2] != uc_ext_ie)) { /* puc_ies第2byte非uc_ext_ie */
        l_len -= (oal_int32)(puc_ies[1] + MAC_IE_HDR_LEN);
        puc_ies += puc_ies[1] + MAC_IE_HDR_LEN;
    }

    if ((l_len < MAC_IE_EXT_HDR_LEN) || (l_len < (MAC_IE_HDR_LEN + puc_ies[1]))
        || ((l_len == MAC_IE_EXT_HDR_LEN) && (puc_ies[0] != uc_eid))) {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}


oal_void mac_set_beacon_interval_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    oal_uint16 *pus_bcn_int;
    oal_uint32 ul_bcn_int;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /*****************************************************************************
                |Beacon interval|
        Octets:        2
    *****************************************************************************/
    pus_bcn_int = (oal_uint16 *)puc_buffer;

    ul_bcn_int = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod;

    *pus_bcn_int = (oal_uint16)oal_byteorder_to_le32(ul_bcn_int);
}


oal_void mac_set_cap_info_ap(oal_void *pst_vap, oal_uint8 *puc_cap_info)
{
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)puc_cap_info;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    wlan_mib_ieee802dot11_stru *pst_mib = pst_mac_vap->pst_mib_info;
    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* 初始清零 */
    puc_cap_info[0] = 0;
    puc_cap_info[1] = 0;

    if (pst_mib->st_wlan_mib_sta_config.en_dot11DesiredBSSType == WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT) {
        pst_cap_info->bit_ibss = 1;
    } else if (pst_mib->st_wlan_mib_sta_config.en_dot11DesiredBSSType == WLAN_MIB_DESIRED_BSSTYPE_INFRA) {
        pst_cap_info->bit_ess = 1;
    }

    /* The Privacy bit is set if WEP is enabled */
    pst_cap_info->bit_privacy = mac_mib_get_privacyinvoked(pst_mac_vap);

    /* preamble */
    pst_cap_info->bit_short_preamble = mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap);

    /* packet binary convolutional code (PBCC) modulation */
    pst_cap_info->bit_pbcc = pst_mib->st_phy_hrdsss.en_dot11PBCCOptionImplemented;

    /* Channel Agility */
    pst_cap_info->bit_channel_agility = pst_mib->st_phy_hrdsss.en_dot11ChannelAgilityPresent;

    /* Spectrum Management */
    pst_cap_info->bit_spectrum_mgmt = pst_mib->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired;

    /* QoS subfield */
    pst_cap_info->bit_qos = pst_mib->st_wlan_mib_sta_config.en_dot11QosOptionImplemented;

    /* short slot */
    pst_cap_info->bit_short_slot_time =
        pst_mib->st_phy_erp.en_dot11ShortSlotTimeOptionActivated &
        pst_mib->st_phy_erp.en_dot11ShortSlotTimeOptionImplemented;

    /* APSD */
    pst_cap_info->bit_apsd = pst_mib->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented;

    /* Radio Measurement */
    pst_cap_info->bit_radio_measurement = pst_mib->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated;

    /* DSSS-OFDM */
    pst_cap_info->bit_dsss_ofdm = pst_mib->st_phy_erp.en_dot11DSSSOFDMOptionActivated;

    /* Delayed BA */
    pst_cap_info->bit_delayed_block_ack = pst_mib->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented;

    /* Immediate Block Ack 参考STA及AP标杆，此能力一直为0,实际通过addba协商。此处修改为标杆一致。mib值不修改 */
    pst_cap_info->bit_immediate_block_ack = 0;
}


oal_void mac_set_cap_info_sta(oal_void *pst_vap, oal_uint8 *puc_cap_info)
{
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)puc_cap_info;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* 学习对端的能力信息 */
    if (memcpy_s(puc_cap_info, OAL_SIZEOF(mac_cap_info_stru), (oal_uint8 *)(&pst_mac_vap->us_assoc_user_cap_info),
        OAL_SIZEOF(mac_cap_info_stru)) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_cap_info_sta::memcpy fail!");
    }

    /* 以下能力位不学习，保持默认值 */
    pst_cap_info->bit_ibss = 0;
    pst_cap_info->bit_cf_pollable = 0;
    pst_cap_info->bit_cf_poll_request = 0;
    pst_cap_info->bit_radio_measurement |= pst_mac_vap->bit_sta_11k_info;
    if (mac_get_11h_test() == OAL_FALSE) {
        pst_cap_info->bit_spectrum_mgmt = 0;
    }
}


oal_void mac_set_ssid_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint16 us_frm_type)
{
    oal_uint8 *puc_ssid = OAL_PTR_NULL;
    oal_uint8 uc_ssid_len;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /***************************************************************************
      A SSID  field  of length 0 is  used  within Probe
      Request management frames to indicate the wildcard SSID.
    ***************************************************************************/
    /* 只有beacon会隐藏ssid */
    if ((pst_mac_vap->st_cap_flag.bit_hide_ssid) && (us_frm_type == WLAN_FC0_SUBTYPE_BEACON)) {
        /* ssid ie */
        *puc_buffer = MAC_EID_SSID;
        /* ssid len */
        *(puc_buffer + 1) = 0;
        *puc_ie_len = MAC_IE_HDR_LEN;
        return;
    }

    *puc_buffer = MAC_EID_SSID;

    puc_ssid = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;

    uc_ssid_len = (oal_uint8)OAL_STRLEN((oal_int8 *)puc_ssid); /* 不包含'\0' */

    *(puc_buffer + 1) = uc_ssid_len;

    if (memcpy_s(puc_buffer + MAC_IE_HDR_LEN, uc_ssid_len, puc_ssid, uc_ssid_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_ssid_ie::memcpy fail!");
    }

    *puc_ie_len = uc_ssid_len + MAC_IE_HDR_LEN;
}


oal_void mac_set_supported_rates_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_rateset_stru *pst_rates_set;
    oal_uint8 uc_nrates;
    oal_uint8 uc_idx;

    pst_rates_set = &(pst_mac_vap->st_curr_sup_rates.st_rate);

    /* STA全信道扫描时根据频段设置supported rates */
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_mac_vap->en_protocol == WLAN_VHT_MODE)) {
        if (pst_mac_vap->st_channel.en_band < WLAN_BAND_BUTT) {
            pst_rates_set = &(pst_mac_vap->ast_sta_sup_rates_ie[pst_mac_vap->st_channel.en_band].st_rate);
        }
    }

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

    uc_nrates = pst_rates_set->uc_rs_nrates;

    if (uc_nrates > MAC_MAX_SUPRATES) {
        uc_nrates = MAC_MAX_SUPRATES;
    }

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_rates_set->ast_rs_rates[uc_idx].uc_mac_rate;
    }

    if (uc_nrates < MAC_MAX_SUPRATES) {
        /* support_rate 速率个数小于8，且SAE_PWE = HASH_TO_ELEMENT, SUPPORT_RATE IE中增加SAE_H2E_ONLY字段 */
        mac_vap_rom_stru *mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);
        if (mac_vap_rom != NULL && mac_vap_rom->sae_pwe == SAE_PWE_HASH_TO_ELEMENT) {
            uc_nrates++;
            puc_buffer[MAC_IE_HDR_LEN + uc_idx] = 0x80 | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
        }
    }

    puc_buffer[1] = uc_nrates;

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}


oal_void mac_set_dsss_params(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_uint8 uc_chan_num;
    mac_device_stru *pst_mac_device;

    /***************************************************************************
                        ----------------------------------------
                        | Element ID  | Length |Current Channel|
                        ----------------------------------------
              Octets:   | 1           | 1      | 1             |
                        ----------------------------------------
    The DSSS Parameter Set element contains information to allow channel number identification for STAs.
    ***************************************************************************/
    
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        *puc_ie_len = 0;
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_set_dsss_params::device_id[%d] ERROR in vap_stru!",
                       pst_mac_vap->uc_device_id);
        return;
    }

    uc_chan_num = pst_mac_vap->st_channel.uc_chan_number;
#if IS_DEVICE
    if ((IS_STA(pst_mac_vap)) && (pst_mac_device->en_curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        uc_chan_num = pst_mac_device->st_scan_params.ast_channel_list[pst_mac_device->uc_scan_chan_idx].uc_chan_number;
    }
#endif
    puc_buffer[0] = MAC_EID_DSPARMS;
    puc_buffer[1] = MAC_DSPARMS_LEN;
    puc_buffer[2] = uc_chan_num; /* 主20MHz信道号赋值给puc_buffer第2byte */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_DSPARMS_LEN;
}

oal_void mac_set_11ntxbf_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_11ntxbf_vendor_ie_stru *pst_vendor_ie = OAL_PTR_NULL;
    if (pst_mac_vap->st_cap_flag.bit_11ntxbf != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    pst_vendor_ie = (mac_11ntxbf_vendor_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = sizeof(mac_11ntxbf_vendor_ie_stru) - MAC_IE_HDR_LEN;
    /* 此值为CCB决策 */
    pst_vendor_ie->uc_ouitype = MAC_EID_11NTXBF;

    /* lint -e572 */ /* lint -e778 */
    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff); /* auc_oui[0]保留VENDER IE的16-23bit */
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff); /* auc_oui[1]保留VENDER IE的8-15bit */
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE)&0xff); /* auc_oui[2]保留VENDER IE的0-7bit */
    /* lint +e572 */ /* lint +e778 */
    memset_s(&pst_vendor_ie->st_11ntxbf, OAL_SIZEOF(mac_11ntxbf_info_stru), 0, OAL_SIZEOF(mac_11ntxbf_info_stru));
    pst_vendor_ie->st_11ntxbf.bit_11ntxbf = pst_mac_vap->st_cap_flag.bit_11ntxbf;
    *puc_ie_len = OAL_SIZEOF(mac_11ntxbf_vendor_ie_stru);
}


oal_void mac_set_pwrconstraint_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                   -------------------------------------------
                   |ElementID | Length | LocalPowerConstraint|
                   -------------------------------------------
       Octets:     |1         | 1      | 1                   |
                   -------------------------------------------

    向工作站描述其所允许的最大传输功率，此信息元素记录规定最大值
    减去实际使用时的最大值
    ***************************************************************************/
    if (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired == OAL_FALSE) {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer = MAC_EID_PWRCNSTR;
    *(puc_buffer + 1) = MAC_PWR_CONSTRAINT_LEN;

    /* Note that this field is always set to 0 currently. Ideally */
    /* this field can be updated by having an algorithm to decide transmit */
    /* power to be used in the BSS by the AP. */
    *(puc_buffer + MAC_IE_HDR_LEN) = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_PWR_CONSTRAINT_LEN;
}


oal_void mac_set_quiet_ie(void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 uc_qcount,
                          oal_uint8 uc_qperiod, oal_uint16 us_qduration, oal_uint16 us_qoffset,
                          oal_uint8 *puc_ie_len)
{
    /* 管制域相关 tbd, 需要11h特性进一步分析此ie的设置 */
    mac_quiet_ie_stru *pst_quiet = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired != OAL_TRUE)
        && (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated != OAL_TRUE)) {
        *puc_ie_len = 0;

        return;
    }

    /***************************************************************************
    -----------------------------------------------------------------------------
    |ElementID | Length | QuietCount | QuietPeriod | QuietDuration | QuietOffset|
    -----------------------------------------------------------------------------
    |1         | 1      | 1          | 1           | 2             | 2          |
    -----------------------------------------------------------------------------
    ***************************************************************************/
    if ((us_qduration == 0) || (uc_qcount == 0)) {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer = MAC_EID_QUIET;

    *(puc_buffer + 1) = MAC_QUIET_IE_LEN;

    pst_quiet = (mac_quiet_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    pst_quiet->quiet_count = uc_qcount;
    pst_quiet->quiet_period = uc_qperiod;
    pst_quiet->quiet_duration = oal_byteorder_to_le16(us_qduration);
    pst_quiet->quiet_offset = oal_byteorder_to_le16(us_qoffset);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_QUIET_IE_LEN;
}


oal_void mac_set_tpc_report_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                -------------------------------------------------
                |ElementID  |Length  |TransmitPower  |LinkMargin|
                -------------------------------------------------
       Octets:  |1          |1       |1              |1         |
                -------------------------------------------------

    TransimitPower, 此帧的传送功率，以dBm为单位
    ***************************************************************************/
    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired == OAL_FALSE)
        && (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated == OAL_FALSE)) {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer = MAC_EID_TPCREP;
    *(puc_buffer + 1) = MAC_TPCREP_IE_LEN;
    *(puc_buffer + 2) = pst_mac_vap->uc_tx_power; /* puc_buffer偏移2byte表示 传输功率 */
    *(puc_buffer + 3) = 0; /* 此字段管理帧中不用(puc_buffer偏移3byte置零) */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_TPCREP_IE_LEN;
}


oal_void mac_set_erp_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_erp_params_stru *pst_erp_params = OAL_PTR_NULL;

    /***************************************************************************
    --------------------------------------------------------------------------
    |EID  |Len  |NonERP_Present|Use_Protection|Barker_Preamble_Mode|Reserved|
    --------------------------------------------------------------------------
    |B0-B7|B0-B7|B0            |B1            |B2                  |B3-B7   |
    --------------------------------------------------------------------------
    ***************************************************************************/
    if ((pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) || (pst_mac_vap->en_protocol == WLAN_LEGACY_11B_MODE)) {
        *puc_ie_len = 0;

        return; /* 5G频段和11b协议模式 没有erp信息 */
    }

    *puc_buffer = MAC_EID_ERP;
    *(puc_buffer + 1) = MAC_ERP_IE_LEN;
    *(puc_buffer + 2) = 0; /* 初始清0(puc_buffer偏移2byte置零) */

    pst_erp_params = (mac_erp_params_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* 如果存在non erp站点与ap关联， 或者obss中存在non erp站点 */
    if ((pst_mac_vap->st_protection.uc_sta_non_erp_num != 0) ||
        (pst_mac_vap->st_protection.bit_obss_non_erp_present == OAL_TRUE)) {
        pst_erp_params->bit_non_erp = 1;
    }

    /* 如果ap已经启用erp保护 */
    if (pst_mac_vap->st_protection.en_protection_mode == WLAN_PROT_ERP) {
        pst_erp_params->bit_use_protection = 1;
    }

    /* 如果存在不支持short preamble的站点与ap关联， 或者ap自身不支持short preamble */
    if ((pst_mac_vap->st_protection.uc_sta_no_short_preamble_num != 0)
        || (mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap) == OAL_FALSE)) {
        pst_erp_params->bit_preamble_mode = 1;
    }

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_ERP_IE_LEN;
}


oal_void mac_sort_pcip(oal_uint8 uc_mode, oal_uint8 *puc_pcip, oal_uint8 uc_pw_count)
{
    oal_uint8 uc_loop;
    oal_uint8 uc_temp;

    if (uc_pw_count == 1) {
        return;
    }

    switch (uc_mode) {
        case DMAC_RSNA_802_11I: {
            /* If the default value is not CCMP, swap */
            if (puc_pcip[0] != WLAN_80211_CIPHER_SUITE_CCMP) {
                for (uc_loop = 1; uc_loop < uc_pw_count; uc_loop++) {
                    if (puc_pcip[uc_loop] == WLAN_80211_CIPHER_SUITE_CCMP) {
                        break;
                    }
                }

                /* CCMP swap */
                if (uc_loop != uc_pw_count) {
                    uc_temp = puc_pcip[0];
                    puc_pcip[0] = WLAN_80211_CIPHER_SUITE_CCMP;
                    puc_pcip[uc_loop] = uc_temp;
                }
            }
            break;
        }
        case DMAC_WPA_802_11I:
        default:  // 在1101 代码中，default 处理分支和WPA 处理分支是一样的处理流程。duankaiyong
        {
            /* If the default value is not CCMP, swap */
            if (puc_pcip[0] != WLAN_80211_CIPHER_SUITE_CCMP) {
                for (uc_loop = 1; uc_loop < uc_pw_count; uc_loop++) {
                    if (puc_pcip[uc_loop] == WLAN_80211_CIPHER_SUITE_CCMP) {
                        break;
                    }
                }

                /* CCMP swap */
                if (uc_loop != uc_pw_count) {
                    uc_temp = puc_pcip[0];
                    puc_pcip[0] = WLAN_80211_CIPHER_SUITE_CCMP;
                    puc_pcip[uc_loop] = uc_temp;
                }
            }
            break;
        }
            // default:
            // break;// 在1101 代码中，default 处理分支和WPA 处理分支是一样的处理流程。duankaiyong
    }
}


oal_uint32 mac_set_rsn_ie(oal_void *pst_vap, oal_uint8 *puc_pmkid, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_uint8 uc_index;
    oal_uint16 us_rsn_capabilities;
    wlan_mib_dot11RSNAConfigEntry_stru *pst_mib_rsna_cfg = OAL_PTR_NULL;
    wlan_mib_Dot11PrivacyEntry_stru *pst_mib_privacy = OAL_PTR_NULL;
    oal_uint8 uc_pair_suites_num;
    oal_uint8 uc_akm_suites_num;
    oal_uint8 uc_loop = 0;
    oal_uint8 uc_group_suit;
    oal_uint8 uc_group_mgmt_suit;
    oal_uint8 auc_pcip[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    oal_uint8 auc_akm[WLAN_AUTHENTICATION_SUITES] = { 0 };
    oal_int32 l_ret = EOK;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_buffer == OAL_PTR_NULL) ||
        (puc_ie_len == OAL_PTR_NULL) || (pst_mac_vap->pst_mib_info == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((mac_mib_get_rsnaactivated(pst_mac_vap) != OAL_TRUE) || (pst_mac_vap->st_cap_flag.bit_wpa2 != OAL_TRUE)) {
        *puc_ie_len = 0;
        return OAL_FAIL;
    }

    /* 获取加密、认证套件类型和数量 */
    uc_group_suit = mac_mib_get_rsn_group_suite(pst_mac_vap);
    uc_group_mgmt_suit = mac_mib_get_rsn_group_mgmt_suite(pst_mac_vap);
    uc_pair_suites_num = mac_mib_get_rsn_pair_suites(pst_mac_vap, auc_pcip, sizeof(auc_pcip));
    uc_akm_suites_num = mac_mib_get_rsn_akm_suites(pst_mac_vap, auc_akm, sizeof(auc_akm));
    if ((uc_pair_suites_num == 0) || (uc_akm_suites_num == 0)) {
        *puc_ie_len = 0;
        return OAL_FAIL;
    }
    /*************************************************************************/
    /* RSN Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* | 4*m                        |     2          | 4*n */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |    2           |    2      |16 -s     |         4                 | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    *puc_ie_len = 0; /* 初始化IE 长度为0 */

    pst_mib_privacy = &(pst_mac_vap->pst_mib_info->st_wlan_mib_privacy);
    pst_mib_rsna_cfg = &(pst_mib_privacy->st_wlan_mib_rsna_cfg);

    /* 添加RSN信息 */
    uc_index = 0;
    puc_buffer[uc_index] = MAC_EID_RSN;
    uc_index += MAC_IE_HDR_LEN;

    /* 填充RSN版本信息 */
    puc_buffer[uc_index++] = MAC_RSN_IE_VERSION;
    puc_buffer[uc_index++] = 0;

    /* 填充组播加密套件 */
    l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_rsn_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;
    puc_buffer[uc_index++] = uc_group_suit;

    /* 填充单播加密套件信息 */
    puc_buffer[uc_index++] = uc_pair_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_pair_suites_num; uc_loop++) {
        l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_rsn_oui, MAC_OUI_LEN);
        uc_index += MAC_OUI_LEN;
        puc_buffer[uc_index++] = auc_pcip[uc_loop];
    }

    /* 设置认证套件数 */
    puc_buffer[uc_index++] = uc_akm_suites_num;
    puc_buffer[uc_index++] = 0;

    /* 根据MIB值，设置认证套件内容 */
    for (uc_loop = 0; uc_loop < uc_akm_suites_num; uc_loop++) {
        l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_rsn_oui, MAC_OUI_LEN);
        uc_index += MAC_OUI_LEN;
        puc_buffer[uc_index++] = auc_akm[uc_loop];
    }

    /* 填充RSN 能力信息 */
    /*************************************************************************************/
    /* --------------------------------------------------------------------------------- */
    /* | B15 - B8 |  B7  | B6  |  B5 - B4      | B3 - B2     |       B1    |     B0    | */
    /* --------------------------------------------------------------------------------- */
    /* | Reserved | MFPC |MFPR |  GTSKA Replay | PTSKA Replay| No Pairwise | Pre - Auth| */
    /* |          |      |     |    Counter    |   Counter   |             |           | */
    /* --------------------------------------------------------------------------------- */
    /*************************************************************************************/
    us_rsn_capabilities = 0;

    /* 根据MIB 值，设置 PTSKA Replay counters.（设置2-3bit） */
    us_rsn_capabilities |= ((pst_mib_rsna_cfg->uc_dot11RSNAConfigPTKSAReplayCounters << 2) & 0x000C);

    /* 根据MIB 值，设置 GTSKA Replay counters.（设置4-5bit） */
    us_rsn_capabilities |= ((pst_mib_rsna_cfg->uc_dot11RSNAConfigGTKSAReplayCounters << 4) & 0x0030);

    /* 根据MIB 值，设置 MFPR. */
    us_rsn_capabilities |= ((pst_mib_privacy->en_dot11RSNAMFPR == OAL_TRUE) ? BIT6 : 0);

    /* 根据MIB 值，设置 MFPC. */
    us_rsn_capabilities |= ((pst_mib_privacy->en_dot11RSNAMFPC == OAL_TRUE) ? BIT7 : 0);

    /* 根据MIB 值，设置pre auth. */
    us_rsn_capabilities |= ((pst_mib_privacy->en_dot11RSNAPreauthenticationActivated == OAL_TRUE) ? BIT0 : 0);

    /* 设置 RSN Capabilities 信息 */
    puc_buffer[uc_index++] = us_rsn_capabilities & 0x00FF;
    puc_buffer[uc_index++] = (us_rsn_capabilities & 0xFF00) >> 8; /* 保留RSN高8位 */

    /* 设置 PMKID 信息 */
    if (puc_pmkid) {
        puc_buffer[uc_index++] = 0x01;
        puc_buffer[uc_index++] = 0x00;
        l_ret += memcpy_s(&(puc_buffer[uc_index]), WLAN_PMKID_LEN, puc_pmkid, WLAN_PMKID_LEN);
        uc_index += WLAN_PMKID_LEN;
    }

    
    if ((mac_mib_get_dot11RSNAMFPC(pst_mac_vap) == OAL_TRUE) && (uc_group_mgmt_suit != 0)) {
        /* 如果已经填过pmkid信息，不需要再填，否则需要填写一个空的PMKID */
        if (puc_pmkid == OAL_PTR_NULL) {
            puc_buffer[uc_index++] = 0x00;
            puc_buffer[uc_index++] = 0x00;
        }
        puc_buffer[uc_index++] = 0x00;
        puc_buffer[uc_index++] = 0x0f;
        puc_buffer[uc_index++] = 0xac;
        puc_buffer[uc_index++] = uc_group_mgmt_suit;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_rsn_ie::memcpy fail!");
    }

    /* 设置RSN IE 长度 */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;
    *puc_ie_len = uc_index;

    return OAL_SUCC;
}

oal_uint32 mac_set_wpa_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_uint8 uc_index = MAC_IE_HDR_LEN;
    oal_uint8 uc_pair_suites_num;
    oal_uint8 uc_akm_suites_num;
    oal_uint8 uc_loop = 0;
    oal_uint8 uc_group_suit;
    oal_uint8 auc_pcip[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    oal_uint8 auc_akm[WLAN_AUTHENTICATION_SUITES] = { 0 };
    oal_int32 l_ret;

    if ((pst_mac_vap == OAL_PTR_NULL) || (puc_buffer == OAL_PTR_NULL) || (puc_ie_len == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->pst_mib_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((mac_mib_get_rsnaactivated(pst_mac_vap) != OAL_TRUE) || (pst_mac_vap->st_cap_flag.bit_wpa != OAL_TRUE)) {
        *puc_ie_len = 0;
        return OAL_FAIL;
    }

    uc_group_suit = mac_mib_get_wpa_group_suite(pst_mac_vap);
    uc_pair_suites_num = mac_mib_get_wpa_pair_suites(pst_mac_vap, auc_pcip, WLAN_PAIRWISE_CIPHER_SUITES);
    uc_akm_suites_num = mac_mib_get_wpa_akm_suites(pst_mac_vap, auc_akm, WLAN_AUTHENTICATION_SUITES);
    if ((uc_pair_suites_num == 0) || (uc_akm_suites_num == 0)) {
        *puc_ie_len = 0;
        return OAL_FAIL;
    }

    /*************************************************************************/
    /* WPA Element Format */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2 */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /* | 4*m                        |     2          | 4*n */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 添加WPA信息 */
    puc_buffer[0] = MAC_EID_WPA;
    l_ret = memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_wpa_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;
    puc_buffer[uc_index++] = MAC_OUITYPE_WPA; /* 填充WPA 的OUI 类型 */

    /* 填充WPA版本信息 */
    puc_buffer[uc_index++] = MAC_WPA_IE_VERSION;
    puc_buffer[uc_index++] = 0;

    /* 填充组播加密套件 */
    l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_wpa_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;
    puc_buffer[uc_index++] = uc_group_suit;

    /* 填充单播加密套件信息 */
    puc_buffer[uc_index++] = uc_pair_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_pair_suites_num; uc_loop++) {
    l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_wpa_oui, MAC_OUI_LEN);
        uc_index += MAC_OUI_LEN;
        puc_buffer[uc_index++] = auc_pcip[uc_loop];
    }

    /* 填充认证套件信息 */
    puc_buffer[uc_index++] = uc_akm_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_akm_suites_num; uc_loop++) {
        l_ret += memcpy_s(puc_buffer + uc_index, MAC_OUI_LEN, g_auc_wpa_oui, MAC_OUI_LEN);
        uc_index += MAC_OUI_LEN;
        puc_buffer[uc_index++] = auc_akm[uc_loop];
    }

    /* 设置WPA IE的长度 */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;
    *puc_ie_len = uc_index;
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_wpa_ie::memcpy fail!");
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

oal_uint8 mac_get_uapsd_config_max_sp_len(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_sta_uapsd_cfg.uc_max_sp_len;
}

oal_uint8 mac_get_uapsd_config_ac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_ac)
{
    if (uc_ac < WLAN_WME_AC_BUTT) {
        return pst_mac_vap->st_sta_uapsd_cfg.uc_trigger_enabled[uc_ac];
    }

    return 0;
}

oal_void mac_set_qos_info_wmm_sta(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer)
{
    oal_uint8 uc_qos_info = 0;
    oal_uint8 uc_max_sp_bits;
    oal_uint8 uc_max_sp_length;

    /* QoS Information field */
    /* -------------------------------------------------------------- */
    /* | B0    | B1    | B2    | B3    | B4      | B5:B6 | B7       | */
    /* -------------------------------------------------------------- */
    /* | AC_VO | AC_VI | AC_BK | AC_BE |         | Max SP|          | */
    /* | U-APSD| U-APSD| U-APSD| U-APSD| Reserved| Length| Reserved | */
    /* | Flag  | Flag  | Flag  | Flag  |         |       |          | */
    /* -------------------------------------------------------------- */
    /* Set the UAPSD configuration information in the QoS info field if the */
    /* BSS type is Infrastructure and the AP supports UAPSD. */
    if (pst_mac_vap->uc_uapsd_cap == OAL_TRUE) {
        uc_max_sp_length = mac_get_uapsd_config_max_sp_len(pst_mac_vap);
        /*lint -e734*/
        uc_qos_info |= (mac_get_uapsd_config_ac(pst_mac_vap, WLAN_WME_AC_VO) << 0);
        uc_qos_info |= (mac_get_uapsd_config_ac(pst_mac_vap, WLAN_WME_AC_VI) << 1);
        uc_qos_info |= (mac_get_uapsd_config_ac(pst_mac_vap, WLAN_WME_AC_BK) << 2); /* qos info获取AC_BK Flag(2bit)) */
        uc_qos_info |= (mac_get_uapsd_config_ac(pst_mac_vap, WLAN_WME_AC_BE) << 3); /* qos info获取AC_BE Flag(3bit)) */
        /*lint +e734*/
        if (uc_max_sp_length <= 6) { /* 判断uc_max_sp_length小于等于6 */
            uc_max_sp_bits = uc_max_sp_length >> 1;

            uc_qos_info |= ((uc_max_sp_bits & 0x03) << 5); /* qos info获取Max SP Length（5-6bit） */
        }
    }

    puc_buffer[0] = uc_qos_info;
}
#endif

oal_void mac_set_qos_info_field(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer)
{
    mac_qos_info_stru *pst_qos_info = (mac_qos_info_stru *)puc_buffer;

    /* QoS Information field  (AP MODE) */
    /* ------------------------------------------- */
    /* | B0:B3               | B4:B6    | B7     | */
    /* ------------------------------------------- */
    /* | Parameter Set Count | Reserved | U-APSD | */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_qos_info->bit_params_count = pst_mac_vap->uc_wmm_params_update_count;
        pst_qos_info->bit_uapsd = pst_mac_vap->st_cap_flag.bit_uapsd;
        pst_qos_info->bit_resv = 0;
    }

    /* QoS Information field  (STA MODE) */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* | B0              | B1              | B2              | B3              | B4      |B5   B6      | B7     | */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* |AC_VO U-APSD Flag|AC_VI U-APSD Flag|AC_BK U-APSD Flag|AC_BE U-APSD Flag|Reserved |Max SP Length|Reserved| */
    /* ---------------------------------------------------------------------------------------------------------- */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
        mac_set_qos_info_wmm_sta(pst_mac_vap, puc_buffer);
#else
        puc_buffer[0] = 0;
        puc_buffer[0] |= 0x0;
#endif
    }
}


oal_void mac_set_wmm_ac_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer, wlan_wme_ac_type_enum_uint8 en_ac)
{
    mac_wmm_ac_params_stru *pst_ac_params = (mac_wmm_ac_params_stru *)puc_buffer;

    /* AC_** Parameter Record field */
    /* ------------------------------------------ */
    /* | Byte 1    | Byte 2        | Byte 3:4   | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */
    /* ACI/AIFSN Field */
    /* ---------------------------------- */
    /* | B0:B3 | B4  | B5:B6 | B7       | */
    /* ---------------------------------- */
    /* | AIFSN | ACM | ACI   | Reserved | */
    /* ---------------------------------- */
    /* AIFSN */
    pst_ac_params->bit_aifsn = pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].ul_dot11EDCATableAIFSN;

    /* ACM */
    pst_ac_params->bit_acm = pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].en_dot11EDCATableMandatory;
#ifdef _PRE_WLAN_FEATURE_WMMAC
    if ((g_en_wmmac_switch == OAL_TRUE) && ((en_ac == WLAN_WME_AC_VO) || (en_ac == WLAN_WME_AC_VI))) {
        pst_ac_params->bit_acm = 1;
    }
#endif  // _PRE_WLAN_FEATURE_WMMAC

    /* ACI */
    pst_ac_params->bit_aci = pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].ul_dot11EDCATableIndex - 1;

    pst_ac_params->bit_resv = 0;

    /* ECWmin/ECWmax Field */
    /* ------------------- */
    /* | B0:B3  | B4:B7  | */
    /* ------------------- */
    /* | ECWmin | ECWmax | */
    /* ------------------- */
    /* ECWmin */
    pst_ac_params->bit_ecwmin = pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].ul_dot11EDCATableCWmin;

    /* ECWmax */
    pst_ac_params->bit_ecwmax = pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].ul_dot11EDCATableCWmax;

    /* TXOP Limit. The value saved in MIB is in usec while the value to be */
    /* set in this element should be in multiple of 32us */
    pst_ac_params->us_txop = /* edca可发送OP门限值右移5位 */
       (oal_uint16)((pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[en_ac].ul_dot11EDCATableTXOPLimit) >> 5);
}


oal_void mac_set_wmm_params_ie(
    oal_void *pst_vap, oal_uint8 *puc_buffer, oal_bool_enum_uint8 en_is_qos, oal_uint8 *puc_ie_len)
{
    oal_uint8 uc_index;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (en_is_qos == OAL_FALSE) {
        *puc_ie_len = 0;

        return;
    }

    /* WMM Parameter Element Format */
    /* --------------------------------------------------------------------- */
    /* | 3Byte | 1        | 1           | 1             | 1        | 1     | */
    /* --------------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | Resvd | */
    /* --------------------------------------------------------------------- */
    /* | 4              | 4              | 4              | 4              | */
    /* --------------------------------------------------------------------- */
    /* | AC_BE ParamRec | AC_BK ParamRec | AC_VI ParamRec | AC_VO ParamRec | */
    /* --------------------------------------------------------------------- */
    puc_buffer[0] = MAC_EID_WMM;
    puc_buffer[1] = MAC_WMM_PARAM_LEN;

    uc_index = MAC_IE_HDR_LEN;

    /* OUI */
    if (memcpy_s(&puc_buffer[uc_index], MAC_OUI_LEN, g_auc_wmm_oui, MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_wmm_params_ie::memcpy fail!");
    }
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMM_PARAM;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(pst_mac_vap, &puc_buffer[uc_index]);
    uc_index += MAC_QOS_INFO_LEN;

    /* Reserved */
    puc_buffer[uc_index++] = 0;

    /* Set the AC_BE, AC_BK, AC_VI, AC_VO Parameter Record fields */
    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_BE);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_BK);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_VI);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_VO);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_WMM_PARAM_LEN;
}


oal_void mac_set_exsup_rates_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vap_rom_stru *mac_vap_rom = (mac_vap_rom_stru *)(pst_mac_vap->_rom);
    mac_rateset_stru *pst_rates_set = NULL;
    oal_uint8 uc_nrates;
    oal_uint8 uc_idx;
    oal_uint8 uc_rs_nrates;

    if (mac_vap_rom == NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_set_exsup_rates_ie:: vap rom is null!}");
        *puc_ie_len = 0;
        return;
    }

    pst_rates_set = &(pst_mac_vap->st_curr_sup_rates.st_rate);

    /* STA全信道扫描时根据频段设置supported rates */
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_mac_vap->en_protocol == WLAN_VHT_MODE)) {
        pst_rates_set = &(pst_mac_vap->ast_sta_sup_rates_ie[pst_mac_vap->st_channel.en_band].st_rate);
    }

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    uc_rs_nrates = pst_rates_set->uc_rs_nrates;
    if (mac_vap_rom->sae_pwe == SAE_PWE_HASH_TO_ELEMENT) {
        uc_rs_nrates++;
    }
    if (uc_rs_nrates <= MAC_MAX_SUPRATES) {
        *puc_ie_len = 0;

        return;
    }

    puc_buffer[0] = MAC_EID_XRATES;
    uc_nrates = pst_rates_set->uc_rs_nrates - MAC_MAX_SUPRATES;
    puc_buffer[1] = uc_nrates;

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++) {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_rates_set->ast_rs_rates[uc_idx + MAC_MAX_SUPRATES].uc_mac_rate;
    }

    if (mac_vap_rom->sae_pwe == SAE_PWE_HASH_TO_ELEMENT) {
        /* support_rate 速率个数大于等于8，且SAE_PWE = HASH_TO_ELEMENT, SUPPORT_RATE IE中增加SAE_H2E_ONLY字段 */
        uc_nrates++;
        puc_buffer[1] = uc_nrates;
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = 0x80 | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
    }

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}


oal_void mac_set_bssload_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_bss_load_stru *pst_bss_load = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented == OAL_FALSE) ||
        (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented == OAL_FALSE)) {
        *puc_ie_len = 0;

        return;
    }

    /***************************************************************************
    ------------------------------------------------------------------------
    |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
    ------------------------------------------------------------------------
    |1   |1   |2            |1                  |2                         |
    ------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_QBSS_LOAD;
    puc_buffer[1] = MAC_BSS_LOAD_IE_LEN;

    pst_bss_load = (mac_bss_load_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    pst_bss_load->us_sta_count = oal_byteorder_to_le16(pst_mac_vap->us_user_nums);

    pst_bss_load->uc_chan_utilization = pst_mac_vap->uc_channel_utilization;

    pst_bss_load->us_aac = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_BSS_LOAD_IE_LEN;
}


oal_void mac_set_ht_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    mac_frame_ht_cap_stru *pst_ht_capinfo = (mac_frame_ht_cap_stru *)puc_buffer;

    /*********************** HT Capabilities Info field*************************
    ----------------------------------------------------------------------------
     |-------------------------------------------------------------------|
     | LDPC   | Supp    | SM    | Green- | Short  | Short  |  Tx  |  Rx  |
     | Coding | Channel | Power | field  | GI for | GI for | STBC | STBC |
     | Cap    | Wth Set | Save  |        | 20 MHz | 40 MHz |      |      |
     |-------------------------------------------------------------------|
     |   B0   |    B1   |B2   B3|   B4   |   B5   |    B6  |  B7  |B8  B9|
     |-------------------------------------------------------------------|

     |-------------------------------------------------------------------|
     |    HT     |  Max   | DSS/CCK | Reserved | 40 MHz     | L-SIG TXOP |
     |  Delayed  | AMSDU  | Mode in |          | Intolerant | Protection |
     | Block-Ack | Length | 40MHz   |          |            | Support    |
     |-------------------------------------------------------------------|
     |    B10    |   B11  |   B12   |   B13    |    B14     |    B15     |
     |-------------------------------------------------------------------|
    ***************************************************************************/
    /* 初始清0 */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;

    pst_ht_capinfo->bit_ldpc_coding_cap = pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionImplemented;

    /* 设置所支持的信道宽度集"，0:仅20MHz运行; 1:20MHz与40MHz运行 */
    pst_ht_capinfo->bit_supported_channel_width = mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap);

    pst_ht_capinfo->bit_sm_power_save = MAC_SMPS_MIMO_MODE;

    pst_ht_capinfo->bit_ht_green_field = pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11HTGreenfieldOptionImplemented;

    pst_ht_capinfo->bit_short_gi_20mhz = pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;

    /* 只有支持40M的情况下，才可以宣称支持40M short GI */
    if (pst_ht_capinfo->bit_supported_channel_width) {
        pst_ht_capinfo->bit_short_gi_40mhz = mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);
    } else {
        pst_ht_capinfo->bit_short_gi_40mhz = 0;
    }

    pst_ht_capinfo->bit_tx_stbc = pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionImplemented;

    pst_ht_capinfo->bit_rx_stbc =
        (pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11RxSTBCOptionImplemented == OAL_TRUE) ? 1 : 0;

    pst_ht_capinfo->bit_ht_delayed_block_ack =
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented;

    pst_ht_capinfo->bit_max_amsdu_length = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength;

    /* 是否在具有40MHz能力，而运行于20/40MHz模式的BSS上使用DSSS/CCK */
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        if ((pst_mac_vap->en_protocol == WLAN_LEGACY_11B_MODE) ||
            (pst_mac_vap->en_protocol == WLAN_MIXED_ONE_11G_MODE) ||
            (pst_mac_vap->en_protocol == WLAN_MIXED_TWO_11G_MODE) ||
            (pst_mac_vap->en_protocol == WLAN_HT_MODE)) {
            pst_ht_capinfo->bit_dsss_cck_mode_40mhz = pst_mac_vap->st_cap_flag.bit_dsss_cck_mode_40mhz;
        } else {
            pst_ht_capinfo->bit_dsss_cck_mode_40mhz = 0;
        }
    } else {
        pst_ht_capinfo->bit_dsss_cck_mode_40mhz = 0;
    }

    /* 设置"40MHz不容许"，只在2.4GHz下有效 */
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        pst_ht_capinfo->bit_forty_mhz_intolerant = mac_mib_get_FortyMHzIntolerant(pst_mac_vap);
    }

    pst_ht_capinfo->bit_lsig_txop_protection =
        pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented;
}


oal_void mac_set_ampdu_params_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap                = (mac_vap_stru *)pst_vap;
    mac_ampdu_params_stru *pst_ampdu_params  = (mac_ampdu_params_stru *)puc_buffer;

     /******************** AMPDU Parameters Field ******************************
      |-----------------------------------------------------------------------|
      | Maximum AMPDU Length Exponent | Minimum MPDU Start Spacing | Reserved |
      |-----------------------------------------------------------------------|
      | B0                         B1 | B2                      B4 | B5     B7|
      |-----------------------------------------------------------------------|
     **************************************************************************/
    /* 初始清0 */
    puc_buffer[0] = 0;

    pst_ampdu_params->bit_max_ampdu_len_exponent =
        pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MaxRxAMPDUFactor;

    pst_ampdu_params->bit_min_mpdu_start_spacing =
        pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MinimumMPDUStartSpacing;
}


oal_void mac_set_sup_mcs_set_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap              = (mac_vap_stru *)pst_vap;
    mac_sup_mcs_set_stru *pst_sup_mcs_set  = (mac_sup_mcs_set_stru *)puc_buffer;

    /************************* Supported MCS Set Field **********************
    |-------------------------------------------------------------------|
    | Rx MCS Bitmask | Reserved | Rx Highest    | Reserved |  Tx MCS    |
    |                |          | Supp Data Rate|          |Set Defined |
    |-------------------------------------------------------------------|
    | B0         B76 | B77  B79 | B80       B89 | B90  B95 |    B96     |
    |-------------------------------------------------------------------|
    | Tx Rx MCS Set  | Tx Max Number     |   Tx Unequal     | Reserved  |
    |  Not Equal     | Spat Stream Supp  | Modulation Supp  |           |
    |-------------------------------------------------------------------|
    |      B97       | B98           B99 |       B100       | B101 B127 |
    |-------------------------------------------------------------------|
    *************************************************************************/
    /* 初始清零 */
    memset_s(puc_buffer, OAL_SIZEOF(mac_sup_mcs_set_stru), 0, OAL_SIZEOF(mac_sup_mcs_set_stru));

    if (memcpy_s(pst_sup_mcs_set->auc_rx_mcs, WLAN_HT_MCS_BITMASK_LEN,
        pst_mac_vap->pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue,
        WLAN_HT_MCS_BITMASK_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_sup_mcs_set_field::memcpy fail.");
    }

    pst_sup_mcs_set->bit_rx_highest_rate = pst_mac_vap->pst_mib_info->st_phy_ht.ul_dot11HighestSupportedDataRate;

    if (pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxMCSSetDefined == OAL_TRUE) {
        pst_sup_mcs_set->bit_tx_mcs_set_def = 1;

        if (pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxRxMCSSetNotEqual == OAL_TRUE) {
            pst_sup_mcs_set->bit_tx_rx_not_equal = 1;

            pst_sup_mcs_set->bit_tx_max_stream =
                pst_mac_vap->pst_mib_info->st_phy_ht.ul_dot11TxMaximumNumberSpatialStreamsSupported;

            if (pst_mac_vap->pst_mib_info->st_phy_ht.en_dot11TxUnequalModulationSupported == OAL_TRUE) {
                pst_sup_mcs_set->bit_tx_unequal_modu = 1;
            }
        }
    }

    /* reserve位清0 */
    pst_sup_mcs_set->bit_resv1 = 0;
    pst_sup_mcs_set->bit_resv2 = 0;
}

oal_void mac_set_ht_extcap_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap      = (mac_vap_stru *)pst_vap;
    mac_ext_cap_stru *pst_ext_cap  = (mac_ext_cap_stru *)puc_buffer;

    /***************** HT Extended Capabilities Field **********************
      |-----------------------------------------------------------------|
      | PCO | PCO Trans | Reserved | MCS  |  +HTC   |  RD    | Reserved |
      |     |   Time    |          | Fdbk | Support | Resp   |          |
      |-----------------------------------------------------------------|
      | B0  | B1     B2 | B3    B7 | B8 B9|   B10   |  B11   | B12  B15 |
      |-----------------------------------------------------------------|
    ***********************************************************************/
    /* 初始清0 */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;

    if (pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented == OAL_TRUE) {
        pst_ext_cap->bit_pco = 1;

        pst_ext_cap->bit_pco_trans_time = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11TransitionTime;
    }

    pst_ext_cap->bit_mcs_fdbk = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented;

    pst_ext_cap->bit_htc_sup = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported;

    pst_ext_cap->bit_rd_resp = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented;
}

oal_void mac_set_txbf_cap_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_txbf_cap_stru *pst_txbf_cap  = (mac_txbf_cap_stru *)puc_buffer;
    wlan_mib_ieee802dot11_stru *pst_mib_info = OAL_PTR_NULL;

    /*************** Transmit Beamforming Capability Field *********************
     |-------------------------------------------------------------------------|
     |   Implicit | Rx Stagg | Tx Stagg  | Rx NDP   | Tx NDP   | Implicit      |
     |   TxBF Rx  | Sounding | Sounding  | Capable  | Capable  | TxBF          |
     |   Capable  | Capable  | Capable   |          |          | Capable       |
     |-------------------------------------------------------------------------|
     |      B0    |     B1   |    B2     |   B3     |   B4     |    B5         |
     |-------------------------------------------------------------------------|
     |              | Explicit | Explicit Non- | Explicit      | Explicit      |
     |  Calibration | CSI TxBF | Compr Steering| Compr steering| TxBF CSI      |
     |              | Capable  | Cap.          | Cap.          | Feedback      |
     |-------------------------------------------------------------------------|
     |  B6       B7 |   B8     |       B9      |       B10     | B11  B12      |
     |-------------------------------------------------------------------------|
     | Explicit Non- | Explicit | Minimal  | CSI Num of | Non-Compr Steering   |
     | Compr BF      | Compr BF | Grouping | Beamformer | Num of Beamformer    |
     | Fdbk Cap.     | Fdbk Cap.|          | Ants Supp  | Ants Supp            |
     |-------------------------------------------------------------------------|
     | B13       B14 | B15  B16 | B17  B18 | B19    B20 | B21        B22       |
     |-------------------------------------------------------------------------|
     | Compr Steering    | CSI Max Num of     |   Channel     |                |
     | Num of Beamformer | Rows Beamformer    | Estimation    | Reserved       |
     | Ants Supp         | Supported          | Capability    |                |
     |-------------------------------------------------------------------------|
     | B23           B24 | B25            B26 | B27       B28 | B29  B31       |
     |-------------------------------------------------------------------------|
    ***************************************************************************/
    /* 初始清零 */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;
    puc_buffer[2] = 0; /* puc_buffer 0、1、2、3byte置零 */
    puc_buffer[3] = 0;
    pst_mib_info = pst_mac_vap->pst_mib_info;
    /* 指示STA是否可以接收staggered sounding帧 */
    pst_txbf_cap->bit_rx_stagg_sounding =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented;

    /* 指示STA是否可以发送staggered sounding帧. */
    pst_txbf_cap->bit_tx_stagg_sounding =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented;

    pst_txbf_cap->bit_rx_ndp =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented;

    pst_txbf_cap->bit_tx_ndp =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented;

    pst_txbf_cap->bit_implicit_txbf =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented;

    pst_txbf_cap->bit_calibration =
        pst_mib_info->st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented;

    pst_txbf_cap->bit_explicit_csi_txbf =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ExplicitCSITransmitBeamformingOptionImplemented;

    pst_txbf_cap->bit_explicit_noncompr_steering =
        pst_mib_info->st_wlan_mib_txbf_config.en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented;

    /* Indicates if this STA can apply transmit beamforming using compressed */
    /* beamforming feedback matrix explicit feedback in its tranmission.     */
    /*************************************************************************/
    /*************************************************************************/
    /* No MIB exists, not clear what needs to be set    B10                  */
    /*************************************************************************/
    /*************************************************************************/
    /* Indicates if this receiver can return CSI explicit feedback */
    pst_txbf_cap->bit_explicit_txbf_csi_fdbk =
        pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented;

    /* Indicates if this receiver can return non-compressed beamforming      */
    /* feedback matrix explicit feedback.                                    */
    pst_txbf_cap->bit_explicit_noncompr_bf_fdbk =
        pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented;

    /* Indicates if this STA can apply transmit beamforming using explicit   */
    /* compressed beamforming feedback matrix.                               */
    pst_txbf_cap->bit_explicit_compr_bf_fdbk =
        pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented;

    /* Indicates the minimal grouping used for explicit feedback reports */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set       B17              */
    /*************************************************************************/
    /*************************************************************************/
    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when CSI feedback is required.                            */
    pst_txbf_cap->bit_csi_num_bf_antssup =
        pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberBeamFormingCSISupportAntenna;

    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when non-compressed beamforming feedback matrix is        */
    /* required                                                              */
    pst_txbf_cap->bit_noncompr_steering_num_bf_antssup =
        pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;

    /* Indicates the maximum number of beamformer antennas the beamformee   */
    /* can support when compressed beamforming feedback matrix is required  */
    pst_txbf_cap->bit_compr_steering_num_bf_antssup =
        pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberCompressedBeamformingMatrixSupportAntenna;

    /* Indicates the maximum number of rows of CSI explicit feedback from    */
    /* beamformee that the beamformer can support when CSI feedback is       */
    /* required                                                              */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set     B25                */
    /*************************************************************************/
    /*************************************************************************/
    /* Indicates maximum number of space time streams (columns of the MIMO   */
    /* channel matrix) for which channel dimensions can be simultaneously    */
    /* estimated. When staggered sounding is supported this limit applies    */
    /* independently to both the data portion and to the extension portion   */
    /* of the long training fields.                                          */
    /*************************************************************************/
    /*************************************************************************/
    /*      No MIB exists, not clear what needs to be set          B27       */
    /*************************************************************************/
    /*************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXBF
    pst_txbf_cap->bit_explicit_compr_Steering = pst_mac_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap;
    pst_txbf_cap->bit_chan_estimation = pst_mac_vap->st_txbf_add_cap.bit_channel_est_cap;
    pst_txbf_cap->bit_minimal_grouping = pst_mac_vap->st_txbf_add_cap.bit_min_grouping;
    pst_txbf_cap->bit_csi_maxnum_rows_bf_sup = pst_mac_vap->st_txbf_add_cap.bit_csi_bfee_max_rows;
    pst_txbf_cap->bit_implicit_txbf_rx = pst_mac_vap->st_txbf_add_cap.bit_imbf_receive_cap;
#endif
}

oal_void mac_set_asel_cap_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_asel_cap_stru *pst_asel_cap  = (mac_asel_cap_stru *)puc_buffer;

    /************** Antenna Selection Capability Field *************************
     |-------------------------------------------------------------------|
     |  Antenna  | Explicit CSI  | Antenna Indices | Explicit | Antenna  |
     | Selection | Fdbk based TX | Fdbk based TX   | CSI Fdbk | Indices  |
     |  Capable  | ASEL Capable  | ASEL Capable    | Capable  | Fdbk Cap.|
     |-------------------------------------------------------------------|
     |    B0     |     B1        |      B2         |    B3    |    B4    |
     |-------------------------------------------------------------------|

     |------------------------------------|
     |  RX ASEL |   Transmit   |          |
     |  Capable |   Sounding   | Reserved |
     |          | PPDU Capable |          |
     |------------------------------------|
     |    B5    |     B6       |    B7    |
     |------------------------------------|
    ***************************************************************************/
    /* 初始清0 */
    puc_buffer[0] = 0;

    /* 指示STA是否支持天线选择 */
    pst_asel_cap->bit_asel =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented;

    /* 指示STA是否具有基于显示CSI(信道状态信息)反馈的发射天线选择能力 */
    pst_asel_cap->bit_explicit_sci_fdbk_tx_asel =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;

    /* 指示STA是否具有基于天线指数反馈的发射天线选择能力 */
    pst_asel_cap->bit_antenna_indices_fdbk_tx_asel =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented;

    /* 指示STA在天线选择的支持下是否能够计算CSI(信道状态信息)并提供CSI反馈 */
    pst_asel_cap->bit_explicit_csi_fdbk =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented;

    /* Indicates whether or not this STA can conduct antenna indices */
    /* selection computation and feedback the results in support of  */
    /* Antenna Selection. */
    pst_asel_cap->bit_antenna_indices_fdbk =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;

    /* 指示STA是否具有接收天线选择能力 */
    pst_asel_cap->bit_rx_asel =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented;

    /* 指示STA是否能够在每一次请求中都可以为天线选择序列发送探测PPDU */
    pst_asel_cap->bit_trans_sounding_ppdu =
        pst_mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented;
}


oal_void mac_set_timeout_interval_ie(
    oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint32 ul_type, oal_uint32 ul_timeout)
{
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    mac_Timeout_Interval_type_enum en_tie_type;

    en_tie_type = (mac_Timeout_Interval_type_enum)ul_type;
    *puc_ie_len = 0;

    /* 判断是否需要设置timeout_interval IE */
    if (en_tie_type >= MAC_TIE_BUTT) {
        return;
    }

    /* Timeout Interval Parameter Element Format
    -----------------------------------------------------------------------
    |ElementID | Length | Timeout Interval Type| Timeout Interval Value  |
    -----------------------------------------------------------------------
    |1         | 1      | 1                    |  4                      |
    -----------------------------------------------------------------------
    */
    puc_buffer[0] = MAC_EID_TIMEOUT_INTERVAL;
    puc_buffer[1] = MAC_TIMEOUT_INTERVAL_INFO_LEN;
    puc_buffer[2] = en_tie_type; /* puc_buffer[2]设置Timeout Interval Type */

    /* 设置Timeout Interval Value */
    puc_buffer[3] = ul_timeout & 0x000000FF; /* puc_buffer[3]保留Timeout Interval Value的0-7bit */
    puc_buffer[4] = (ul_timeout & 0x0000FF00)>>8; /* puc_buffer[4]保留Timeout Interval Value的8-15bit */
    puc_buffer[5] = (ul_timeout & 0x00FF0000)>>16; /* puc_buffer[5]保留Timeout Interval Value的16-23bit */
    puc_buffer[6] = (ul_timeout & 0xFF000000)>>24; /* puc_buffer[6]保留Timeout Interval Value的24-31bit */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_TIMEOUT_INTERVAL_INFO_LEN;
#else
    *puc_ie_len = 0;
#endif
    return;
}



oal_void mac_set_ht_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;

    if (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |HT Capa. Info |A-MPDU Parameters |Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      |2             |1                 |16               |
    -------------------------------------------------------------------------
    |HT Extended Cap. |Transmit Beamforming Cap. |ASEL Cap.          |
    -------------------------------------------------------------------------
    |2                |4                         |1                  |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *puc_buffer       = MAC_EID_HT_CAP;
    *(puc_buffer + 1) = MAC_HT_CAP_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    /* 填充ht capabilities information域信息 */
    mac_set_ht_capinfo_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_CAPINFO_LEN;

    /* 填充A-MPDU parameters域信息 */
    mac_set_ampdu_params_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_AMPDU_PARAMS_LEN;

    /* 填充supported MCS set域信息 */
    mac_set_sup_mcs_set_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_SUP_MCS_SET_LEN;

    /* 填充ht extended capabilities域信息 */
    mac_set_ht_extcap_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_EXT_CAP_LEN;

    /* 填充 transmit beamforming capabilities域信息 */
    mac_set_txbf_cap_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_TXBF_CAP_LEN;

    /* 填充asel(antenna selection) capabilities域信息 */
    mac_set_asel_cap_field(pst_vap, puc_buffer);
    g_st_mac_frame_rom_cb.p_mac_set_ht_capabilities_ie(pst_mac_vap, puc_buffer, puc_ie_len);
    *puc_ie_len = MAC_IE_HDR_LEN + MAC_HT_CAP_LEN;
}


oal_void mac_set_ht_opern_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru        *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    mac_ht_opern_stru   *pst_ht_opern = OAL_PTR_NULL;
    oal_uint8           uc_obss_non_ht = 0;

    if (mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap) != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
      ----------------------------------------------------------------------
      |EID |Length |PrimaryChannel |HT Operation Information |Basic MCS Set|
      ----------------------------------------------------------------------
      |1   |1      |1              |5                        |16           |
      ----------------------------------------------------------------------
    ***************************************************************************/
    /************************ HT Information Field ****************************
     |--------------------------------------------------------------------|
     | Primary | Seconday  | STA Ch | RIFS |           reserved           |
     | Channel | Ch Offset | Width  | Mode |                              |
     |--------------------------------------------------------------------|
     |    1    | B0     B1 |   B2   |  B3  |    B4                     B7 |
     |--------------------------------------------------------------------|

     |----------------------------------------------------------------|
     |     HT     | Non-GF STAs | resv      | OBSS Non-HT  | Reserved |
     | Protection |   Present   |           | STAs Present |          |
     |----------------------------------------------------------------|
     | B0     B1  |     B2      |    B3     |     B4       | B5   B15 |
     |----------------------------------------------------------------|

     |-------------------------------------------------------------|
     | Reserved |  Dual  |  Dual CTS  | Seconday | LSIG TXOP Protn |
     |          | Beacon | Protection |  Beacon  | Full Support    |
     |-------------------------------------------------------------|
     | B0    B5 |   B6   |     B7     |     B8   |       B9        |
     |-------------------------------------------------------------|

     |---------------------------------------|
     |  PCO   |  PCO  | Reserved | Basic MCS |
     | Active | Phase |          |    Set    |
     |---------------------------------------|
     |  B10   |  B11  | B12  B15 |    16     |
     |---------------------------------------|
    **************************************************************************/
    *puc_buffer = MAC_EID_HT_OPERATION;

    *(puc_buffer + 1) = MAC_HT_OPERN_LEN;
    pst_ht_opern = (mac_ht_opern_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* 主信道编号 */
    pst_ht_opern->uc_primary_channel = pst_mac_vap->st_channel.uc_chan_number;

    /* 设置"次信道偏移量" */
    if ((pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
        (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_80PLUSPLUS) ||
        (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_80PLUSMINUS)) {
        pst_ht_opern->bit_secondary_chan_offset = MAC_SCA;
    } else if ((pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
             (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_80MINUSPLUS) ||
             (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_80MINUSMINUS)) {
        pst_ht_opern->bit_secondary_chan_offset = MAC_SCB;
    } else {
        pst_ht_opern->bit_secondary_chan_offset = MAC_SCN;
    }

    /* 设置"STA信道宽度"，当BSS运行信道宽度 >= 40MHz时，需要将此field设置为1 */
    pst_ht_opern->bit_sta_chan_width = (pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) ? 1 : 0;

    /* 指示基本服务集里是否允许使用减小的帧间距 */
    pst_ht_opern->bit_rifs_mode = mac_mib_get_RifsMode(pst_mac_vap);

    /* B4-B7保留 */
    pst_ht_opern->bit_resv1 = 0;

    /* 指示ht传输的保护要求 */
    pst_ht_opern->bit_HT_protection = mac_mib_get_HtProtection(pst_mac_vap);

    /* Non-GF STAs */
    pst_ht_opern->bit_nongf_sta_present = mac_mib_get_NonGFEntitiesPresent(pst_mac_vap);

    /* B3 resv */
    pst_ht_opern->bit_resv2 = 0;

    /* B4  obss_nonht_sta_present */
    if ((pst_mac_vap->st_protection.bit_obss_non_ht_present != 0) ||
         (pst_mac_vap->st_protection.uc_sta_non_ht_num != 0)) {
        uc_obss_non_ht = 1;
    }
    pst_ht_opern->bit_obss_nonht_sta_present = uc_obss_non_ht;

    /* B5-B15 保留 */
    pst_ht_opern->bit_resv3 = 0;
    pst_ht_opern->bit_resv4 = 0;

    /* B0-B5 保留 */
    pst_ht_opern->bit_resv5 = 0;

    /* B6  dual_beacon */
    pst_ht_opern->bit_dual_beacon = 0;

    /* Dual CTS protection */
    pst_ht_opern->bit_dual_cts_protection = pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection;

    /* secondary_beacon: Set to 0 in a primary beacon */
    pst_ht_opern->bit_secondary_beacon = 0;

    /* BSS support L-SIG TXOP Protection */
    pst_ht_opern->bit_lsig_txop_protection_full_support = mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap);

    /* PCO active */
    pst_ht_opern->bit_pco_active = pst_mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated;

    /* PCO phase */
    pst_ht_opern->bit_pco_phase = 0;

    /* B12-B15  保留 */
    pst_ht_opern->bit_resv6 = 0;

    /* Basic MCS Set: set all bit zero,Indicates the MCS values that are supported by all HT STAs in the BSS. */
    memset_s(pst_ht_opern->auc_basic_mcs_set, MAC_HT_BASIC_MCS_SET_LEN, 0, MAC_HT_BASIC_MCS_SET_LEN);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_HT_OPERN_LEN;
}

oal_void mac_set_ext_capabilities_bss_transition_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    mac_ext_cap_ie_stru     *pst_ext_cap;
    mac_vap_stru            *pst_mac_vap = (mac_vap_stru *)pst_vap;
    pst_ext_cap = (mac_ext_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

     /* 首先需先使能wirelessmanagerment标志 */
     /* 然后如果是站点本地能力位和扩展控制变量均支持BSS TRANSITION 设置扩展能力bit位 */
    if ((mac_mib_get_WirelessManagementImplemented(pst_mac_vap) == OAL_TRUE) &&
        (mac_mib_get_MgmtOptionBSSTransitionImplemented(pst_mac_vap) == OAL_TRUE) &&
        (mac_mib_get_MgmtOptionBSSTransitionActivated(pst_mac_vap) == OAL_TRUE) &&
        (pst_mac_vap->bit_sta_11v_info == OAL_TRUE) &&
        (IS_LEGACY_STA(pst_mac_vap))) {
        pst_ext_cap->bit_bss_transition = 1;
    } else {
        pst_ext_cap->bit_bss_transition = 0;
    }
#endif
}


oal_void mac_set_obss_scan_params(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_obss_scan_params_stru   *pst_obss_scan = OAL_PTR_NULL;
    oal_uint32                  ul_ret;

    if (mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap) != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    if ((pst_mac_vap->st_channel.en_band != WLAN_BAND_2G) ||
        (mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap) != OAL_TRUE)) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
     |ElementID |Length |OBSS    |OBSS   |BSS Channel   |OBSS Scan  |OBSS Scan   |
     |          |       |Scan    |Scan   |Width Trigger |Passive    |Active Total|
     |          |       |Passive |Active |Scan Interval |Total Per  |Per         |
     |          |       |Dwell   |Dwell  |              |Channel    |Channel     |
     ----------------------------------------------------------------------------
     |1         |1      |2       |2      |2             |2          |2           |
     ----------------------------------------------------------------------------
     |BSS Width   |OBSS Scan|
     |Channel     |Activity |
     |Transition  |Threshold|
     |Delay Factor|         |
     ------------------------
     |2           |2        |
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_OBSS_SCAN;
    puc_buffer[1] = MAC_OBSS_SCAN_IE_LEN;

    pst_obss_scan = (mac_obss_scan_params_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    ul_ret = mac_mib_get_OBSSScanPassiveDwell(pst_mac_vap);
    pst_obss_scan->us_passive_dwell = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActiveDwell(pst_mac_vap);
    pst_obss_scan->us_active_dwell  = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_BSSWidthTriggerScanInterval(pst_mac_vap);
    pst_obss_scan->us_scan_interval = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanPassiveTotalPerChannel(pst_mac_vap);
    pst_obss_scan->us_passive_total_per_chan  = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActiveTotalPerChannel(pst_mac_vap);
    pst_obss_scan->us_active_total_per_chan   = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_BSSWidthChannelTransitionDelayFactor(pst_mac_vap);
    pst_obss_scan->us_transition_delay_factor = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActivityThreshold(pst_mac_vap);
    pst_obss_scan->us_scan_activity_thresh    = (oal_uint16)(oal_byteorder_to_le32(ul_ret));

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_OBSS_SCAN_IE_LEN;
}


oal_void mac_set_ext_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru            *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ext_cap_ie_stru     *pst_ext_cap = OAL_PTR_NULL;

    if (mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap) == OAL_FALSE) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
                         ----------------------------------
                         |Element ID |Length |Capabilities|
                         ----------------------------------
          Octets:        |1          |1      |n           |
                         ----------------------------------
    ------------------------------------------------------------------------------------------
    |  B0       | B1 | B2             | B3   | B4   |  B5  |  B6    |  B7   | ...|  B38    |   B39      |
    ----------------------------------------------------------------------------
    |20/40 coex |resv|extended channel| resv | PSMP | resv | S-PSMP | Event |    |TDLS Pro-  TDLS Channel
                                                                                             Switching
    |mgmt supp  |    |switching       |      |      |      |        |       | ...| hibited | Prohibited |
    --------------------------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_EXT_CAPS;
    puc_buffer[1] = MAC_XCAPS_EX_LEN;

    /* 初始清零 */
    memset_s(puc_buffer + MAC_IE_HDR_LEN, OAL_SIZEOF(mac_ext_cap_ie_stru), 0, OAL_SIZEOF(mac_ext_cap_ie_stru));

    pst_ext_cap = (mac_ext_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* 设置20/40 BSS Coexistence Management Support fieid */
    if ((mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap) == OAL_TRUE) &&
        (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) &&
        (mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap) == OAL_TRUE)) {
        pst_ext_cap->bit_2040_coexistence_mgmt = 1;
    }

    /* 设置TDLS prohibited */
    pst_ext_cap->bit_tdls_prhibited =  pst_mac_vap->st_cap_flag.bit_tdls_prohibited;

    /* 设置TDLS channel switch prohibited */
    pst_ext_cap->bit_tdls_channel_switch_prhibited = pst_mac_vap->st_cap_flag.bit_tdls_channel_switch_prohibited;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 如果是11ac 站点 设置OPMODE NOTIFY标志 */
    if (mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_TRUE) {
        pst_ext_cap->bit_operating_mode_notification = mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    /*  如果支持Hotspot2.0的Interwoking标志  */
    pst_ext_cap->bit_interworking = 1;
#else
    pst_ext_cap->bit_interworking = 0;
#endif

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_XCAPS_EX_LEN;

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    mac_set_ext_capabilities_bss_transition_ie(pst_mac_vap, puc_buffer, puc_ie_len);
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    mac_ftm_add_to_ext_capabilities_ie(pst_mac_vap, puc_buffer, puc_ie_len);
#endif

    g_st_mac_frame_rom_cb.p_mac_set_ext_capabilities_ie(pst_vap, puc_buffer, puc_ie_len);
}

oal_void  mac_set_vht_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru           *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    mac_vht_cap_info_stru  *pst_vht_capinfo = (mac_vht_cap_info_stru *)puc_buffer;
    wlan_mib_ieee802dot11_stru *pst_mib_info = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_user_stru          *pst_mac_user;
#endif
    /*********************** VHT 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Max    | Supp    | RX   | Short GI| Short  | Tx   |  Rx  |  SU        |
     | MPDU   | Channel | LDPC | for 80  | GI for | STBC | STBC | Beamformer |
     | Length | Wth Set |      |         | 160MHz |      |      | Capable    |
     |-----------------------------------------------------------------------|
     | B0 B1  | B2 B3   | B4   |   B5    |   B6   |  B7  |B8 B10|   B11      |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | SU         | Compressed   | Num of    | MU        | MU        | VHT   |
     | Beamformee | Steering num | Sounding  | Beamformer| Beamformee| TXOP  |
     | Capable    | of bf ant sup| Dimensions| Capable   | Capable   | PS    |
     |-----------------------------------------------------------------------|
     |    B12     | B13      B15 | B16    B18|   B19     |    B20    | B21   |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | +HTC   | Max AMPDU| VHT Link  | Rx ANT     | Tx ANT     |   Resv      |
     | VHT    | Length   | Adaptation| Pattern    | Pattern    |             |
     | Capable| Exponent | Capable   | Consistency| Consistency|             |
     |-----------------------------------------------------------------------|
     | B22    | B23  B25 | B26   B27 |   B28      |   B29      |  B30 B31    |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    pst_mib_info = pst_mac_vap->pst_mib_info;
    pst_vht_capinfo->bit_max_mpdu_length = pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11MaxMPDULength;

    /* 设置"所支持的信道宽度集"，0:80MHz运行; 1:160MHz; 2:80+80MHz */
    pst_vht_capinfo->bit_supported_channel_width = mac_mib_get_VHTChannelWidthOptionImplemented(pst_mac_vap);

    pst_vht_capinfo->bit_rx_ldpc = pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented;
    pst_vht_capinfo->bit_short_gi_80mhz = pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented;
    pst_vht_capinfo->bit_short_gi_160mhz =
        pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented;
    pst_vht_capinfo->bit_tx_stbc = pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented;
    pst_vht_capinfo->bit_rx_stbc =
        (pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented == OAL_TRUE) ? 1 : 0;
    pst_vht_capinfo->bit_su_beamformer_cap =
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented;
    pst_vht_capinfo->bit_su_beamformee_cap =
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented;
    pst_vht_capinfo->bit_num_sounding_dim =
        pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTNumberSoundingDimensions;
    pst_vht_capinfo->bit_mu_beamformer_cap =
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented;
    pst_vht_capinfo->bit_mu_beamformee_cap =
        pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented;
    pst_vht_capinfo->bit_vht_txop_ps =
        pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented;
    pst_vht_capinfo->bit_htc_vht_capable =
        pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported;
    pst_vht_capinfo->bit_max_ampdu_len_exp = pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTMaxRxAMPDUFactor;

#ifdef _PRE_WLAN_FEATURE_TXBF
    pst_vht_capinfo->bit_num_bf_ant_supported =
        pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTBeamformeeNTxSupport;
    /* 参考标杆,该字段根据对端空间流能力和自己的能力取交集 */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id);
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_mac_user != OAL_PTR_NULL) &&
        (pst_mac_user->st_vht_hdl.bit_num_sounding_dim != 0)) {
        pst_vht_capinfo->bit_num_bf_ant_supported = oal_min(pst_vht_capinfo->bit_num_bf_ant_supported,
                                                            pst_mac_user->st_vht_hdl.bit_num_sounding_dim);
    }

#else
    pst_vht_capinfo->bit_num_bf_ant_supported   = 0;
#endif
    pst_vht_capinfo->bit_vht_link_adaptation    = 0;
    pst_vht_capinfo->bit_rx_ant_pattern         = 0;   /* 在该关联中不改变天线模式，设为1,；改变则设为0 */
    pst_vht_capinfo->bit_tx_ant_pattern         = 0;   /* 在该关联中不改变天线模式，设为1,；改变则设为0 */

    /* resv位清0 */
    pst_vht_capinfo->bit_resv = 0;
}


oal_void  mac_set_vht_supported_mcsset_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru              *pst_mac_vap    = (mac_vap_stru *)pst_vap;
    mac_vht_sup_mcs_set_stru  *pst_vht_mcsset = (mac_vht_sup_mcs_set_stru *)puc_buffer;

    /*********************** VHT 支持的MCS集 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Rx MCS Map | Rx Highest Supported | Resv    | Tx MCS Map  |
     |            | Long gi Data Rate    |         |             |
     |-----------------------------------------------------------------------|
     | B0     B15 | B16              B28 | B29 B31 | B32     B47 |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | Tx Highest Supported |  Resv   |
     | Long gi Data Rate    |         |
     |-----------------------------------------------------------------------|
     |  B48             B60 | B61 B63 |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    pst_vht_mcsset->bit_rx_mcs_map = pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap;
    pst_vht_mcsset->bit_rx_highest_rate =
        pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported;
    pst_vht_mcsset->bit_tx_mcs_map =
        pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap;
    pst_vht_mcsset->bit_tx_highest_rate =
        pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported;

    /* resv清0 */
    pst_vht_mcsset->bit_resv  = 0;
    pst_vht_mcsset->bit_resv2 = 0;
}


oal_void  mac_set_vht_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru  *pst_mac_vap = (mac_vap_stru *)pst_vap;
    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented != OAL_TRUE)
#ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G))
#endif /* _PRE_WLAN_FEATURE_11AC2G */
        ) {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |VHT Capa. Info |VHT Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      | 4             | 8                   |
    -------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_VHT_CAP;
    puc_buffer[1] = MAC_VHT_CAP_IE_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    mac_set_vht_capinfo_field(pst_vap, puc_buffer);

    puc_buffer += MAC_VHT_CAP_INFO_FIELD_LEN;

    mac_set_vht_supported_mcsset_field(pst_vap, puc_buffer);
    g_st_mac_frame_rom_cb.p_mac_set_vht_capabilities_ie(pst_mac_vap, puc_buffer, puc_ie_len);
    *puc_ie_len = MAC_IE_HDR_LEN + MAC_VHT_CAP_IE_LEN;
}


oal_void  mac_set_vht_opern_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru        *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vht_opern_stru  *pst_vht_opern = OAL_PTR_NULL;

    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented != OAL_TRUE)
#ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G))
#endif /* _PRE_WLAN_FEATURE_11AC2G */
        ) {
        *puc_ie_len = 0;
        return;
    }

    /***********************VHT Operation element*******************************
    -------------------------------------------------------------------------
            |EID |Length |VHT Opern Info |VHT Basic MCS Set|
    -------------------------------------------------------------------------
    Octes:  |1   |1      | 3             | 2               |
    -------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_VHT_OPERN;
    puc_buffer[1] = MAC_VHT_INFO_IE_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    /**********************VHT Opern Info***************************************
    -------------------------------------------------------------------------
            | Channel Width | Channel Center | Channel Center |
            |               | Freq Seg0      | Freq Seg1      |
    -------------------------------------------------------------------------
    Octes:  |       1       |       1        |       1        |
    -------------------------------------------------------------------------
    ***************************************************************************/
    pst_vht_opern = (mac_vht_opern_stru *)puc_buffer;

    /*
        uc_channel_width的取值，0 -- 20/40M, 1 -- 80M, 2 -- 160M
    */
    if (pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) {
        pst_vht_opern->uc_channel_width = 1;
    } else {
        pst_vht_opern->uc_channel_width = 0;
    }

    switch (pst_mac_vap->st_channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                          |
                          |中心频率相对于主20偏6个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                          |
                          |中心频率相对于主20偏-2个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                          |
                          |中心频率相对于主20偏2个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                          |
                          |中心频率相对于主20偏-6个信道
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            break;
        default:
            /* 中心频率直接填0  */
            pst_vht_opern->uc_channel_center_freq_seg0 = 0;
            break;
    }

    pst_vht_opern->uc_channel_center_freq_seg1 = 0;
    pst_vht_opern->us_basic_mcs_set = pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_VHT_INFO_IE_LEN;
}


oal_uint32  mac_set_csa_ie(
    oal_uint8 uc_mode, oal_uint8 uc_channel, oal_uint8 uc_csa_cnt, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    if (oal_unlikely((puc_buffer == OAL_PTR_NULL) || (puc_ie_len == OAL_PTR_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*  Channel Switch Announcement Information Element Format               */
    /* --------------------------------------------------------------------- */
    /* | Element ID | Length | Chnl Switch Mode | New Chnl | Ch Switch Cnt | */
    /* --------------------------------------------------------------------- */
    /* | 1          | 1      | 1                | 1        | 1             | */
    /* --------------------------------------------------------------------- */
   /* 设置Channel Switch Announcement Element */
    puc_buffer[0] = MAC_EID_CHANSWITCHANN;
    puc_buffer[1] = MAC_CHANSWITCHANN_LEN;
    puc_buffer[2] = uc_mode; /* ask all associated STAs to stop transmission（puc_buffer第2byte是Chnl Switch Mode） */
    puc_buffer[3] = uc_channel; /* puc_buffer第3byte是New Chnl */
    puc_buffer[4] = uc_csa_cnt; /* puc_buffer第4byte是Ch Switch Cnt */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_CHANSWITCHANN_LEN;

    return OAL_SUCC;
}


oal_uint32  mac_set_csa_bw_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                      *pst_mac_vap = (mac_vap_stru *)pst_vap;
    wlan_channel_bandwidth_enum_uint8  en_bw;
    oal_uint8                          uc_len;
    oal_uint8                          uc_channel;

    if (oal_unlikely((puc_buffer == OAL_PTR_NULL) || (puc_ie_len == OAL_PTR_NULL) || (pst_mac_vap == OAL_PTR_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_bw       = pst_mac_vap->st_ch_switch_info.en_announced_bandwidth;
    uc_channel  = pst_mac_vap->st_ch_switch_info.uc_announced_channel;
    uc_len      = 0;

    /* 封装Second channel offset IE */
    if (mac_set_second_channel_offset_ie(en_bw, puc_buffer, &uc_len) != OAL_SUCC) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "mac_set_csa_bw_ie:mac_set_second_channel_offset_ie failed");
        return 0;
    }

    puc_buffer += uc_len;
    *puc_ie_len = uc_len;

    if (mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_TRUE) {
        /* 11AC Wide Bandwidth Channel Switch IE */
        uc_len = 0;
        if (mac_set_11ac_wideband_ie(uc_channel, en_bw, puc_buffer, &uc_len) != OAL_SUCC) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{mac_set_csa_bw_ie::mac_set_11ac_wideband_ie failed}");
            return 0;
        }
        *puc_ie_len += uc_len;
    }

    return OAL_SUCC;
}


oal_uint8*  mac_get_ssid(oal_uint8 *puc_beacon_body, oal_int32 l_frame_body_len, oal_uint8 *puc_ssid_len)
{
    const oal_uint8   *puc_ssid_ie = OAL_PTR_NULL;
    oal_uint16         us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /* ssid的长度初始赋值为0 */
    *puc_ssid_len = 0;

    /* 检测beacon帧或者probe rsp帧的长度的合法性 */
    if (l_frame_body_len <= us_offset) {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_get_ssid:: the length of beacon/probe rsp frame body is invalid.}");
        return OAL_PTR_NULL;
    }

    /* 查找ssid的ie */
    puc_ssid_ie = mac_find_ie(MAC_EID_SSID, (puc_beacon_body + us_offset), (oal_int32)(l_frame_body_len - us_offset));
    if ((puc_ssid_ie != OAL_PTR_NULL) && (puc_ssid_ie[1] < WLAN_SSID_MAX_LEN)) {
        /* 获取ssid ie的长度 */
        *puc_ssid_len = puc_ssid_ie[1];

        return (oal_uint8 *)(puc_ssid_ie + MAC_IE_HDR_LEN);
    }

    oam_warning_log0(0, OAM_SF_ANY, "{mac_get_ssid:: ssid ie isn't found.}");
    return OAL_PTR_NULL;
}


oal_uint16  mac_get_beacon_period(oal_uint8 *puc_beacon_body)
{
    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    return *((oal_uint16 *)(puc_beacon_body + MAC_TIME_STAMP_LEN));
}


oal_uint8  mac_get_dtim_period(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len)
{
    oal_uint8   *puc_ie;

    oal_uint16   us_offset;

    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    puc_ie = mac_find_ie(MAC_EID_TIM, puc_frame_body + us_offset, us_frame_body_len - us_offset);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_MIN_TIM_LEN)) {
        return puc_ie[3]; /* 获取dtim period值(puc_ie第3byte) */
    }

    return 0;
}



oal_uint8  mac_get_dtim_cnt(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len)
{
    oal_uint8   *puc_ie;

    oal_uint16   us_offset;

    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    puc_ie = mac_find_ie(MAC_EID_TIM, puc_frame_body + us_offset, us_frame_body_len - us_offset);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_MIN_TIM_LEN)) {
        return puc_ie[2]; /* 获取dtim cnt值(puc_ie第2byte) */
    }

    return 0;
}


oal_bool_enum_uint8  mac_is_wmm_ie(oal_uint8 *puc_ie)
{
    /* --------------------------------------------------------------------- */
    /* WMM Information/Parameter Element Format                              */
    /* --------------------------------------------------------------------- */
    /* | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
    /* --------------------------------------------------------------------- */
    /* |3    | 1       | 1          | 1       | 1       | ---------------- | */
    /* --------------------------------------------------------------------- */
    if ((puc_ie[0] == MAC_EID_WMM) &&
       (puc_ie[1] >= MAC_WMM_IE_LEN) &&
       (puc_ie[2] == MAC_WMM_OUI_BYTE_ONE) && /* OUI占3字节，puc_ie 2、3、4byte */
       (puc_ie[3] == MAC_WMM_OUI_BYTE_TWO) && /* OUI占3字节，puc_ie 2、3、4byte */
       (puc_ie[4] == MAC_WMM_OUI_BYTE_THREE) && /* OUI占3字节，puc_ie 2、3、4byte */
       (puc_ie[5] == MAC_OUITYPE_WMM) && /* puc_ie第5byte是OUIType */
       /* puc_ie的第6byte是OUISubtype */
       ((puc_ie[6] == MAC_OUISUBTYPE_WMM_INFO) || (puc_ie[6] == MAC_OUISUBTYPE_WMM_PARAM)) &&
       (puc_ie[7] == MAC_OUI_WMM_VERSION)) { /* puc_ie的第7byte是Version */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_uint8*  mac_get_wmm_ie(oal_uint8 *puc_beacon_body, oal_uint16 us_frame_len)
{
    oal_uint8 *puc_wmmie = OAL_PTR_NULL;

    puc_wmmie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                   puc_beacon_body, us_frame_len);
    if (puc_wmmie == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    return mac_is_wmm_ie(puc_wmmie) ? puc_wmmie : OAL_PTR_NULL;
}


oal_uint16 mac_get_rsn_capability(const oal_uint8 *puc_rsn_ie)
{
    oal_uint16  us_pairwise_count;
    oal_uint16  us_akm_count;
    oal_uint16  us_rsn_capability;
    oal_uint16  us_index               = 0;

    if (puc_rsn_ie == OAL_PTR_NULL) {
        return 0;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*            |         4*m                |     2           |   4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    if (puc_rsn_ie[1] < MAC_MIN_RSN_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid rsn ie len[%d].}", puc_rsn_ie[1]);
        return 0;
    }

    us_index += 8; /* Element ID(1byte)、Length(1byte)、Version(2byte)、Group Cipher Suite(4byte)共8byte */
    us_pairwise_count = oal_make_word16(puc_rsn_ie[us_index], puc_rsn_ie[us_index + 1]);
    if (us_pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid us_pairwise_count[%d].}", us_pairwise_count);
        return 0;
    }
    /* Pairwise Cipher Suite Count 2byte,Pairwise Cipher Suite List一组长度为4byte */
    us_index += 2 + 4 * (oal_uint8)us_pairwise_count;

    us_akm_count = oal_make_word16(puc_rsn_ie[us_index], puc_rsn_ie[us_index + 1]);
    if (us_akm_count > WLAN_AUTHENTICATION_SUITES) {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid us_akm_count[%d].}", us_akm_count);
        return 0;
    }

    us_index += 2 + 4 * (oal_uint8) us_akm_count; /* AKM Suite Count 2byte,AKM Suite List一组长度为4byte */

    if (MAC_IE_REAMIN_LEN_IS_ENOUGH(2, us_index, puc_rsn_ie[1], 2) == OAL_FALSE) { /* 判断剩余长度是否足够，2表示剩余len */
        return 0;
    }

    us_rsn_capability = oal_make_word16(puc_rsn_ie[us_index], puc_rsn_ie[us_index + 1]);
    return us_rsn_capability;
}


oal_void mac_set_power_cap_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru            *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_regclass_info_stru  *pst_regclass_info = OAL_PTR_NULL;

    /********************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length | MinimumTransmitPowerCapability| MaximumTransmitPowerCapability|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                             | 1                             |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/
    *puc_buffer       = MAC_EID_PWRCAP;
    *(puc_buffer + 1) = MAC_PWR_CAP_LEN;

    /* 成功获取管制域信息则根据国家码和TPC设置最大和最小发射功率，否则默认为0 */
    pst_regclass_info =
        mac_get_channel_num_rc_info(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number);
    if (pst_regclass_info != OAL_PTR_NULL) {
        /* 判断频段是否为2G,是返回最小发射功率为4，否则返回最小发射功率为3 */
        *(puc_buffer + 2) = (oal_uint8)((pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) ? 4 : 3);
        /* puc_buffer偏移3byte值为uc_max_reg_tx_pwr(规定最大发送功率)、uc_max_tx_pwr(实际最大发送功率)取小 */
        *(puc_buffer + 3) = oal_min(pst_regclass_info->uc_max_reg_tx_pwr, pst_regclass_info->uc_max_tx_pwr);
    } else {
        *(puc_buffer + 2) = 0; /* puc_buffer偏移的2、3byte置零(最大、最小发送功率默认为零) */
        *(puc_buffer + 3) = 0;
    }
    *puc_ie_len = MAC_IE_HDR_LEN + MAC_PWR_CAP_LEN;
}


oal_void mac_set_supported_channel_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8            uc_channel_max_num;
    oal_uint8            uc_channel_idx;
    oal_uint8            us_channel_ie_len = 0;
    oal_uint8           *puc_ie_len_buffer = 0;
    mac_vap_stru        *pst_mac_vap       = (mac_vap_stru *)pst_vap;
    oal_uint8            uc_channel_idx_cnt = 0;

    if ((pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired == OAL_FALSE) ||
        (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated == OAL_TRUE)) {
        *puc_ie_len = 0;
        return;
    }

    /********************************************************************************************
            长度不定，信道号与信道数成对出现
            ------------------------------------------------------------------------------------
            |ElementID | Length | Fisrt Channel Number| Number of Channels|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                   | 1                 |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/
    /* 根据支持的频段获取最大信道个数 */
    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        uc_channel_max_num = (oal_uint8)MAC_CHANNEL_FREQ_2_BUTT;
    } else if (pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) {
        uc_channel_max_num = (oal_uint8)MAC_CHANNEL_FREQ_5_BUTT;
    } else {
        *puc_ie_len = 0;
        return;
    }

    *puc_buffer = MAC_EID_SUPPCHAN;
    puc_buffer++;
    puc_ie_len_buffer = puc_buffer;

    /* 填写信道信息 */
    for (uc_channel_idx = 0; uc_channel_idx < uc_channel_max_num; uc_channel_idx++) {
        /* 修改管制域结构体后，需要增加该是否支持信号的判断 */
        if (mac_is_channel_idx_valid(pst_mac_vap->st_channel.en_band, uc_channel_idx) == OAL_SUCC) {
            uc_channel_idx_cnt++;
            /* uc_channel_idx_cnt为1的时候表示是第一个可用信道，需要写到Fisrt Channel Number */
            if (uc_channel_idx_cnt == 1) {
                puc_buffer++;

                mac_get_channel_num_from_idx(pst_mac_vap->st_channel.en_band, uc_channel_idx, puc_buffer);
            } else if((uc_channel_max_num - 1) == uc_channel_idx) {
                /* 将Number of Channels写入帧体中 */
                puc_buffer++;
               *puc_buffer = uc_channel_idx_cnt;

                us_channel_ie_len += 2; /* Fisrt Channel Number(1byte)、Number of Channels(1byte)共2byte */
            }
        } else {
            /* uc_channel_idx_cnt不为0的时候表示之前有可用信道，需要将可用信道的长度写到帧体中 */
            if (uc_channel_idx_cnt != 0) {
                /* 将Number of Channels写入帧体中 */
                puc_buffer++;
               *puc_buffer = uc_channel_idx_cnt;

                us_channel_ie_len += 2; /* Fisrt Channel Number(1byte)、Number of Channels(1byte)共2byte */
            }
            /* 将Number of Channels统计清零 */
            uc_channel_idx_cnt = 0;
        }
    }

    *puc_ie_len_buffer = us_channel_ie_len;
    *puc_ie_len = us_channel_ie_len + MAC_IE_HDR_LEN;
}


oal_void mac_set_wmm_ie_sta(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8            uc_index;
    mac_vap_stru        *pst_mac_vap  = (mac_vap_stru *)pst_vap;

    /* WMM Information Element Format                                */
    /* ------------------------------------------------------------- */
    /* | 3     | 1        | 1           | 1             | 1        | */
    /* ------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | */
    /* ------------------------------------------------------------- */
    /* 判断STA是否支持WMM */
    if (pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented != OAL_TRUE) {
        *puc_ie_len = 0;
        return;
    }

    puc_buffer[0]        = MAC_EID_WMM;
    puc_buffer[1]        = MAC_WMM_INFO_LEN;

    uc_index             = MAC_IE_HDR_LEN;

    /* OUI */
    if (memcpy_s(&puc_buffer[uc_index], MAC_OUI_LEN, g_auc_wmm_oui, MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_wmm_ie_sta::memcpy fail!");
    }
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMM_INFO;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(pst_mac_vap, &puc_buffer[uc_index]);
    uc_index += MAC_QOS_INFO_LEN;

    /* Reserved */
    puc_buffer[uc_index++] = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_WMM_INFO_LEN;
}
#ifdef _PRE_WLAN_FEATURE_WMMAC

oal_void mac_set_tspec_info_field(oal_uint8 *pst_vap, mac_wmm_tspec_stru *pst_addts_args, oal_uint8 *puc_buffer)
{
    mac_wmm_tspec_stru    *pst_tspec_info;

    /* TSPEC字段:
              ----------------------------------------------------------------------------------------
              |TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ----------------------------------------------------------------------------------------
     Octets:  | 3     |  2              |   2         |4            |4            |
              ----------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ----------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ----------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ----------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ----------------------------------------------------------------------------------------
     TS info字段:
              ----------------------------------------------------------------------------------------
              |Reserved |TSID |Direction |1 |0 |Reserved |PSB |UP |Reserved |Reserved |Reserved |
              ----------------------------------------------------------------------------------------
       Bits:  |1        |4    |2         |  2  |1        |1   |3  |2        |1        |7        |
              ----------------------------------------------------------------------------------------
     */
    /* 初始化TSPEC结构内存信息 */
    memset_s(puc_buffer, MAC_WMMAC_TSPEC_LEN, 0, MAC_WMMAC_TSPEC_LEN);

    pst_tspec_info = (mac_wmm_tspec_stru *)(puc_buffer); // TSPEC Body

    pst_tspec_info->ts_info.bit_tsid       = pst_addts_args->ts_info.bit_tsid;
    pst_tspec_info->ts_info.bit_direction  = pst_addts_args->ts_info.bit_direction;
    pst_tspec_info->ts_info.bit_acc_policy = 1;
    pst_tspec_info->ts_info.bit_apsd       = pst_addts_args->ts_info.bit_apsd;
    pst_tspec_info->ts_info.bit_user_prio  = pst_addts_args->ts_info.bit_user_prio;

    pst_tspec_info->us_norminal_msdu_size  = pst_addts_args->us_norminal_msdu_size;
    pst_tspec_info->us_max_msdu_size       = pst_addts_args->us_max_msdu_size;
    pst_tspec_info->ul_min_data_rate       = pst_addts_args->ul_min_data_rate;
    pst_tspec_info->ul_mean_data_rate      = pst_addts_args->ul_mean_data_rate;
    pst_tspec_info->ul_peak_data_rate      = pst_addts_args->ul_peak_data_rate;
    pst_tspec_info->ul_min_phy_rate        = pst_addts_args->ul_min_phy_rate;
    pst_tspec_info->us_surplus_bw          = pst_addts_args->us_surplus_bw;
}


oal_uint16 mac_set_wmmac_ie_sta(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, mac_wmm_tspec_stru *pst_addts_args)
{
    oal_uint8            uc_index;
    /************************************************************************************/
    /*                                Set WMM TSPEC 信息:                               */
    /*       ---------------------------------------------------------------------------
             |ID | Length| OUI |OUI Type| OUI subtype| Version| TSPEC body|
             ---------------------------------------------------------------------------
    Octets:  |1  | 1     | 3   |1       | 1          | 1      | 55        |
             ---------------------------------------------------------------------------
    *************************************************************************************/
    puc_buffer[0]        = MAC_EID_WMM;
    puc_buffer[1]        = MAC_WMMAC_INFO_LEN;

    uc_index             = MAC_IE_HDR_LEN;

    /* OUI */
    if (memcpy_s(&puc_buffer[uc_index], MAC_OUI_LEN, g_auc_wmm_oui, MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_wmmac_ie_sta::memcpy fail!");
    }
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMMAC_TSPEC;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* wmmac tspec Field */
    mac_set_tspec_info_field(pst_vap, pst_addts_args, &puc_buffer[uc_index]);

    return (MAC_IE_HDR_LEN + MAC_WMMAC_INFO_LEN);
}

#endif //_PRE_WLAN_FEATURE_WMMAC

oal_void mac_set_listen_interval_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    puc_buffer[0] = 0x0a;
    puc_buffer[1] = 0x00;
    *puc_ie_len   = MAC_LIS_INTERVAL_IE_LEN;
}


oal_void mac_set_status_code_ie(oal_uint8 *puc_buffer, mac_status_code_enum_uint16 en_status_code)
{
    puc_buffer[0] = (oal_uint8)(en_status_code & 0x00FF);
    puc_buffer[1] = (oal_uint8)((en_status_code & 0xFF00) >> 8); /* en_status_code保留高8位赋值给puc_buffer[1] */
}


oal_void mac_set_aid_ie(oal_uint8 *puc_buffer, oal_uint16 uc_aid)
{
    /* The 2 MSB bits of Association ID is set to 1 as required by the standard. */
    uc_aid |= 0xC000;
    puc_buffer[0] = (uc_aid & 0x00FF);
    puc_buffer[1] = (uc_aid & 0xFF00) >> 8; /* uc_aid保留高8位赋值给puc_buffer[1] */
}


oal_uint8  mac_get_bss_type(oal_uint16 us_cap_info)
{
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)&us_cap_info;

    if (pst_cap_info->bit_ess != 0) {
        return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    }

    if (pst_cap_info->bit_ibss != 0) {
        return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT;
    }

    return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_ANY;
}


oal_uint32  mac_check_mac_privacy(oal_uint16 us_cap_info, oal_uint8 *pst_vap)
{
    mac_vap_stru       *pst_mac_vap = OAL_PTR_NULL;
    mac_cap_info_stru  *pst_cap_info = (mac_cap_info_stru *)&us_cap_info;

    if (pst_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (mac_mib_get_privacyinvoked(pst_mac_vap) == OAL_TRUE) {
        /* 该VAP有Privacy invoked但其他VAP没有 */
        if (pst_cap_info->bit_privacy == 0) {
            return (oal_uint32)OAL_FALSE;
        }
    }
    /* 考虑兼容性，当vap不支持加密时，不检查用户的能力 */
    return (oal_uint32)OAL_TRUE;
}


oal_void mac_add_app_ie(
    oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint16 *pus_ie_len, en_app_ie_type_uint8 en_type)
{
    mac_vap_stru    *pst_mac_vap;
    oal_uint8       *puc_app_ie;
    oal_uint32       ul_app_ie_len;

    pst_mac_vap   = (mac_vap_stru *)pst_vap;
    puc_app_ie    = pst_mac_vap->ast_app_ie[en_type].puc_ie;
    ul_app_ie_len = pst_mac_vap->ast_app_ie[en_type].ul_ie_len;

    if (ul_app_ie_len == 0) {
        *pus_ie_len = 0;
        return;
    } else {
        if (memcpy_s(puc_buffer, ul_app_ie_len, puc_app_ie, ul_app_ie_len) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_add_app_ie::memcpy fail!");
        }
        *pus_ie_len = (oal_uint16)ul_app_ie_len;
    }

    return;
}


oal_void mac_add_wps_ie(oal_void *pst_vap,
                        oal_uint8 *puc_buffer,
                        oal_uint16 *pus_ie_len,
                        en_app_ie_type_uint8 en_type)
{
    mac_vap_stru    *pst_mac_vap;
    oal_uint8       *puc_app_ie;
    oal_uint8       *puc_wps_ie = OAL_PTR_NULL;
    oal_uint32       ul_app_ie_len;
    oal_int32        l_ret;

    pst_mac_vap   = (mac_vap_stru *)pst_vap;
    puc_app_ie    = pst_mac_vap->ast_app_ie[en_type].puc_ie;
    ul_app_ie_len = pst_mac_vap->ast_app_ie[en_type].ul_ie_len;

    if (ul_app_ie_len == 0) {
        *pus_ie_len = 0;
        return;
    }

    puc_wps_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
                                    puc_app_ie, (oal_int32)ul_app_ie_len);
    if ((puc_wps_ie == OAL_PTR_NULL) || (puc_wps_ie[1] < MAC_MIN_WPS_IE_LEN)) {
        *pus_ie_len = 0;
        return;
    }

    /* 将WPS ie 信息拷贝到buffer 中 */
    l_ret = memcpy_s(puc_buffer, puc_wps_ie[1] + MAC_IE_HDR_LEN, puc_wps_ie, puc_wps_ie[1] + MAC_IE_HDR_LEN);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_add_wps_ie::memcpy fail!");
        return;
    }
    *pus_ie_len = puc_wps_ie[1] + MAC_IE_HDR_LEN;

    return;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_void mac_set_opmode_field(oal_uint8 *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru           *pst_mac_vap       = (mac_vap_stru *)pst_vap;
    mac_opmode_notify_stru *pst_opmode_notify = (mac_opmode_notify_stru *)puc_buffer;
    wlan_bw_cap_enum_uint8  en_cp_bw          = WLAN_BW_CAP_BUTT;

    /* ********************************************** */
    /* ---------------------------------------------- */
    /* |B0-B1         |B2-B3   |B4-B5   |B6-B7      | */
    /* ---------------------------------------------- */
    /* |Channel Width |resv    |Rx Nss  |Rx Nss Type| */
    /* ---------------------------------------------- */
    /* ********************************************** */
    mac_vap_get_bandwidth_cap(pst_mac_vap, &en_cp_bw);
    pst_opmode_notify->bit_channel_width = en_cp_bw;
    pst_opmode_notify->bit_rx_nss        = pst_mac_vap->en_vap_rx_nss;
    pst_opmode_notify->bit_rx_nss_type   = 0;
}


oal_void mac_set_opmode_notify_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /********************************************
            -------------------------------------
            |ElementID | Length | Operating Mode|
            -------------------------------------
    Octets: |1         | 1      | 1             |
            -------------------------------------

    ********************************************/
    if ((mac_mib_get_VHTOptionImplemented(pst_mac_vap) == OAL_FALSE)
        || (mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap) == OAL_FALSE)) {
        *puc_ie_len = 0;
        return;
    }

    puc_buffer[0] = MAC_EID_OPMODE_NOTIFY;
    puc_buffer[1] = MAC_OPMODE_NOTIFY_LEN;

    mac_set_opmode_field(pst_vap, (puc_buffer + MAC_IE_HDR_LEN));

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_OPMODE_NOTIFY_LEN;
}
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)


wlan_pmf_cap_status_uint8 mac_get_pmf_cap(oal_uint8 *puc_ie, oal_uint32 ul_ie_len)
{
    oal_uint8  *puc_rsn_ie = NULL;
    oal_uint16  us_rsn_cap;

    if (oal_unlikely(puc_ie == OAL_PTR_NULL)) {
        return MAC_PMF_DISABLED;
    }

    /* 查找RSN信息元素,如果没有RSN信息元素,则按照不支持处理 */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, puc_ie, (oal_int32)(ul_ie_len));
    if (puc_rsn_ie == OAL_PTR_NULL) {
        return MAC_PMF_DISABLED;
    }

    /* 根据RSN信息元素, 判断RSN能力是否匹配 */
    us_rsn_cap = mac_get_rsn_capability(puc_rsn_ie);
    if ((us_rsn_cap & BIT6) && (us_rsn_cap & BIT7)) {
        return MAC_PME_REQUIRED;
    }

    if (us_rsn_cap & BIT7) {
        return MAC_PMF_ENABLED;
    }
    return MAC_PMF_DISABLED;
}
#endif

oal_void mac_set_epigram_vht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
    oal_uint8 uc_ie_len = 0;

    puc_buffer[0] = MAC_EID_VENDOR;
    puc_buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    /*lint -e572*/ /*lint -e778*/
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM 16位置零赋值给puc_buffer[2] */
    puc_buffer[2] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM保留高8位赋值给puc_buffer[3] */
    puc_buffer[3] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8)  & 0xff);
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM保留低8位赋值给puc_buffer[4] */
    puc_buffer[4] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM) & 0xff);
    puc_buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE; /* 0x04赋值给puc_buffer[5] */
    puc_buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE; /* 0x08赋值给puc_buffer[6] */
    /*lint +e572*/ /*lint +e778*/

    mac_set_vht_capabilities_ie(pst_mac_vap, puc_buffer + puc_buffer[1] + MAC_IE_HDR_LEN, &uc_ie_len);

    if (uc_ie_len) {
        puc_buffer[1] += uc_ie_len;
        *puc_ie_len = puc_buffer[1] + MAC_IE_HDR_LEN;
    } else {
        *puc_ie_len = 0;
    }
#else
    *puc_ie_len = 0;
#endif
}


oal_void mac_set_epigram_novht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
    puc_buffer[0] = MAC_EID_VENDOR;
    puc_buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    /*lint -e572*/ /*lint -e778*/
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM 16位置零赋值给puc_buffer[2] */
    puc_buffer[2] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM保留高8位赋值给puc_buffer[3] */
    puc_buffer[3] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8)  & 0xff);
    /* MAC_WLAN_OUI_BROADCOM_EPIGRAM保留低8位赋值给puc_buffer[4] */
    puc_buffer[4] = (oal_uint8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM) & 0xff);
    puc_buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE; /* 0x04赋值给puc_buffer[5] */

    puc_buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3; /* 0x07赋值给puc_buffer[6] */
    /*lint +e572*/ /*lint +e778*/
    *puc_ie_len = puc_buffer[1] + MAC_IE_HDR_LEN;
#endif
}

oal_uint32  mac_vap_set_cb_tx_user_idx(mac_vap_stru *pst_mac_vap, mac_tx_ctl_stru *pst_tx_ctl, oal_uint8 *puc_data)
{
    oal_uint16  us_user_idx = MAX_TX_USER_IDX;
    oal_uint32  ul_ret;

    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_data, WLAN_MAC_ADDR_LEN, &us_user_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_set_cb_tx_user_idx:: cannot find user_idx \
            from xx:xx:xx:%x:%x:%x, set TX_USER_IDX %d.}",
            puc_data[3], puc_data[4], puc_data[5], MAC_INVALID_USER_ID); /* puc_data第3、4、5byte为参数输出打印 */
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = MAX_TX_USER_IDX;

        return ul_ret;
    }
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

    return OAL_SUCC;
}

oal_uint16  mac_encap_2040_coext_mgmt(
    oal_void *pst_vap, oal_netbuf_stru *pst_buffer, oal_uint8 uc_coext_info, oal_uint32 ul_chan_report)
{
    oal_uint8                     *puc_mac_header          = oal_netbuf_header(pst_buffer);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                     *puc_payload_addr        = oal_netbuf_data(pst_buffer);
#else
    oal_uint8                     *puc_payload_addr        = puc_mac_header + MAC_80211_FRAME_LEN;
#endif
    oal_uint8                      uc_chan_idx     = 0;
    oal_uint16                     us_ie_len_idx;
    oal_uint16                     us_index        = 0;
    mac_vap_stru                  *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    wlan_channel_band_enum_uint8   en_band         = pst_mac_vap->st_channel.en_band;
    oal_uint8                      uc_max_num_chan = mac_get_num_supp_channel(en_band);
    oal_uint8                      uc_channel_num;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET,
                     pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /*                 20/40 BSS Coexistence Management frame - Frame Body               */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |20/40 BSS Coex IE| 20/40 BSS Intolerant Chan Report IE| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |3                |Variable                            | */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_COEXT_MGMT;                   /* Public Action */

    /* 封装20/40 BSS Coexistence element */
    puc_payload_addr[us_index++] = MAC_EID_2040_COEXT;                   /* Element ID */
    puc_payload_addr[us_index++] = MAC_2040_COEX_LEN;                    /* Length */
    puc_payload_addr[us_index++] = uc_coext_info;                        /* 20/40 BSS Coexistence Information field */

    /* 封装20/40 BSS Intolerant Channel Report element */
    /* 只有当STA检测到Trigger Event A时，才包含Operating Class，参见802.11n 10.15.12 */
    puc_payload_addr[us_index++] = MAC_EID_2040_INTOLCHREPORT;       /* Element ID */
    us_ie_len_idx          = us_index;
    puc_payload_addr[us_index++] = MAC_2040_INTOLCHREPORT_LEN_MIN;   /* Length */
    puc_payload_addr[us_index++] = 0;                               /* Operating Class */
    if (ul_chan_report > 0) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{mac_encap_2040_coext_mgmt::Channel List = 0x%x.}", ul_chan_report);
        /* Channel List */
        for (uc_chan_idx = 0; uc_chan_idx < uc_max_num_chan; uc_chan_idx++) {
            if (((ul_chan_report >> uc_chan_idx) & BIT0) != 0) {
                mac_get_channel_num_from_idx(en_band, uc_chan_idx, &uc_channel_num);
                puc_payload_addr[us_index++] = uc_channel_num;
                puc_payload_addr[us_ie_len_idx]++;
            }
        }
    }
    return (us_index + MAC_80211_FRAME_LEN);
}

oal_module_symbol(mac_find_ie);
oal_module_symbol(mac_find_vendor_ie);
oal_module_symbol(mac_set_aid_ie);
oal_module_symbol(mac_set_supported_rates_ie);
oal_module_symbol(mac_set_ext_capabilities_ie);
oal_module_symbol(mac_set_ssid_ie);
oal_module_symbol(mac_set_exsup_rates_ie);
oal_module_symbol(mac_set_cap_info_ap);
oal_module_symbol(mac_set_cap_info_sta);
oal_module_symbol(mac_set_wmm_params_ie);
oal_module_symbol(mac_set_wmm_ie_sta);
#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_module_symbol(mac_set_wmmac_ie_sta);
#endif  // _PRE_WLAN_FEATURE_WMMAC

oal_module_symbol(mac_set_ht_capabilities_ie);
oal_module_symbol(mac_set_obss_scan_params);
oal_module_symbol(mac_set_ht_opern_ie);
oal_module_symbol(mac_set_listen_interval_ie);
oal_module_symbol(mac_set_status_code_ie);
oal_module_symbol(mac_set_power_cap_ie);
oal_module_symbol(mac_set_supported_channel_ie);
oal_module_symbol(mac_set_rsn_ie);
oal_module_symbol(mac_set_wpa_ie);
oal_module_symbol(mac_set_vht_capabilities_ie);
oal_module_symbol(mac_set_vht_opern_ie);
oal_module_symbol(mac_get_dtim_period);
oal_module_symbol(mac_get_dtim_cnt);
oal_module_symbol(mac_is_wmm_ie);
oal_module_symbol(mac_check_mac_privacy);
oal_module_symbol(g_auc_rsn_oui);
oal_module_symbol(g_auc_wpa_oui);
oal_module_symbol(mac_set_timeout_interval_ie);
oal_module_symbol(mac_add_app_ie);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
oal_module_symbol(mac_set_opmode_notify_ie);
#endif

oal_module_symbol(mac_rx_report_80211_frame);
oal_module_symbol(mac_set_11ntxbf_vendor_ie);
oal_module_symbol(mac_get_wmm_ie);
oal_module_symbol(mac_get_rsn_capability);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
oal_module_symbol(mac_get_pmf_cap);
#endif

oal_module_symbol(mac_encap_2040_coext_mgmt);
oal_module_symbol(mac_set_epigram_vht_ie);
oal_module_symbol(mac_set_epigram_novht_ie);
oal_module_symbol(mac_vap_set_cb_tx_user_idx);

