

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_device.h"
#include "dmac_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_IE_EC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#ifdef WIN32
mac_ie_rom_stru g_st_mac_ie_rom_cb = {
    mac_ie_proc_opmode_field_cb,
};
#else
mac_ie_rom_stru g_st_mac_ie_rom_cb = {
    .p_mac_ie_proc_opmode_field = mac_ie_proc_opmode_field_cb,
};
#endif
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/

wlan_channel_bandwidth_enum_uint8 mac_get_bandwith_from_center_freq_seg0(
    oal_uint8 uc_channel, oal_uint8 uc_chan_center_freq)
{
    switch (uc_chan_center_freq - uc_channel) {
        case 6:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                          |
                          |中心频率相对于主20偏6个信道
            ************************************************************************/
            return WLAN_BAND_WIDTH_80PLUSPLUS;

        case -2:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                          |
                          |中心频率相对于主20偏-2个信道
            ************************************************************************/
            return WLAN_BAND_WIDTH_80PLUSMINUS;

        case 2:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                          |
                          |中心频率相对于主20偏2个信道
            ************************************************************************/
            return WLAN_BAND_WIDTH_80MINUSPLUS;

        case -6:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                          |
                          |中心频率相对于主20偏-6个信道
            ************************************************************************/
            return WLAN_BAND_WIDTH_80MINUSMINUS;

        default:
            return 0;
    }
}


oal_void mac_ie_get_vht_rx_mcs_map(mac_rx_max_mcs_map_stru *pst_mac_rx_mcs_sta,
                                   mac_rx_max_mcs_map_stru *pst_mac_rx_mcs_ap)
{
    oal_uint16 *pus_rx_mcs_sta;

    /* 获取空间流1及空间流2的能力信息，目前1151最多支持2个空间流 */
    if ((pst_mac_rx_mcs_sta->us_max_mcs_1ss != 0x3) && (pst_mac_rx_mcs_ap->us_max_mcs_1ss != 0x3)) {
        pst_mac_rx_mcs_sta->us_max_mcs_1ss = pst_mac_rx_mcs_sta->us_max_mcs_1ss > pst_mac_rx_mcs_ap->us_max_mcs_1ss
                                             ? pst_mac_rx_mcs_ap->us_max_mcs_1ss
                                             : pst_mac_rx_mcs_sta->us_max_mcs_1ss;
    } else {
        pst_mac_rx_mcs_sta->us_max_mcs_1ss = 0x3;
    }

    if ((pst_mac_rx_mcs_sta->us_max_mcs_2ss != 0x3) && (pst_mac_rx_mcs_ap->us_max_mcs_2ss != 0x3)) {
        pst_mac_rx_mcs_sta->us_max_mcs_2ss = pst_mac_rx_mcs_sta->us_max_mcs_2ss > pst_mac_rx_mcs_ap->us_max_mcs_2ss
                                             ? pst_mac_rx_mcs_ap->us_max_mcs_2ss
                                             : pst_mac_rx_mcs_sta->us_max_mcs_2ss;
    } else {
        pst_mac_rx_mcs_sta->us_max_mcs_2ss = 0x3;
    }

    /* 限制最大的空间流数目 */
    pus_rx_mcs_sta = (oal_uint16 *)pst_mac_rx_mcs_sta;

    *pus_rx_mcs_sta = (*pus_rx_mcs_sta) | 0xFFF0;
}


oal_bool_enum_uint8 mac_ie_proc_ht_supported_channel_width(mac_user_stru *pst_mac_user_sta,
                                                           mac_vap_stru *pst_mac_vap,
                                                           oal_uint8 uc_supported_channel_width,
                                                           oal_bool_enum en_prev_asoc_ht)
{
    /* 不支持20/40Mhz频宽 */
    if (uc_supported_channel_width == 0) {
        if ((en_prev_asoc_ht == OAL_FALSE) || (pst_mac_user_sta->st_ht_hdl.bit_supported_channel_width == OAL_TRUE)) {
            pst_mac_vap->st_protection.uc_sta_20M_only_num++;
        }

        return OAL_FALSE;
    } else { /* 支持20/40Mhz频宽 */
        /* 如果STA之前已经作为不支持20/40Mhz频宽的HT站点与AP关联 */
        if ((en_prev_asoc_ht == OAL_TRUE) && (pst_mac_user_sta->st_ht_hdl.bit_supported_channel_width == OAL_FALSE)) {
            pst_mac_vap->st_protection.uc_sta_20M_only_num--;
        }

        return OAL_TRUE;
    }
}


oal_bool_enum_uint8 mac_ie_proc_ht_green_field(mac_user_stru *pst_mac_user_sta,
                                               mac_vap_stru *pst_mac_vap,
                                               oal_uint8 uc_ht_green_field,
                                               oal_bool_enum en_prev_asoc_ht)
{
    /* 不支持Greenfield */
    if (uc_ht_green_field == 0) {
        if ((en_prev_asoc_ht == OAL_FALSE) || (pst_mac_user_sta->st_ht_hdl.bit_ht_green_field == OAL_TRUE)) {
            pst_mac_vap->st_protection.uc_sta_non_gf_num++;
        }

        return OAL_FALSE;
    } else { /* 支持Greenfield */
        /* 如果STA之前已经作为不支持GF的HT站点与AP关联 */
        if ((en_prev_asoc_ht == OAL_TRUE) && (pst_mac_user_sta->st_ht_hdl.bit_ht_green_field == OAL_FALSE)) {
            pst_mac_vap->st_protection.uc_sta_non_gf_num--;
        }

        return OAL_TRUE;
    }
}


oal_bool_enum_uint8 mac_ie_proc_lsig_txop_protection_support(mac_user_stru *pst_mac_user_sta,
                                                             mac_vap_stru *pst_mac_vap,
                                                             oal_uint8 uc_lsig_txop_protection_support,
                                                             oal_bool_enum en_prev_asoc_ht)
{
    /* 不支持L-sig txop protection */
    if (uc_lsig_txop_protection_support == 0) {
        if ((en_prev_asoc_ht == OAL_FALSE) || (pst_mac_user_sta->st_ht_hdl.bit_lsig_txop_protection == OAL_TRUE)) {
            pst_mac_vap->st_protection.uc_sta_no_lsig_txop_num++;
        }

        return OAL_FALSE;
    } else { /* 支持L-sig txop protection */
        /* 如果STA之前已经作为不支持Lsig txop protection的HT站点与AP关联 */
        if ((en_prev_asoc_ht == OAL_TRUE) && (pst_mac_user_sta->st_ht_hdl.bit_lsig_txop_protection == OAL_FALSE)) {
            pst_mac_vap->st_protection.uc_sta_no_lsig_txop_num--;
        }

        return OAL_TRUE;
    }
}


oal_uint32 mac_ie_proc_ht_sta(mac_vap_stru *pst_mac_sta,
                              oal_uint8 *puc_payload,
                              oal_uint16 *pus_index,
                              mac_user_stru *pst_mac_user_ap,
                              oal_uint16 *pus_ht_cap_info,
                              oal_uint16 *pus_amsdu_maxsize)
{
    oal_uint8 uc_mcs_bmp_index;
    oal_uint8 uc_smps;
    oal_uint16 us_offset;
    mac_user_ht_hdl_stru *pst_ht_hdl = OAL_PTR_NULL;
    mac_user_ht_hdl_stru st_ht_hdl;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    oal_uint16 us_tmp_info_elem;
    oal_uint16 us_tmp_txbf_low;
    oal_uint32 ul_tmp_txbf_elem;

    if ((pst_mac_sta == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL)
        || (pus_index == OAL_PTR_NULL) || (pst_mac_user_ap == OAL_PTR_NULL) || (pus_ht_cap_info == OAL_PTR_NULL)
        || (pus_amsdu_maxsize == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_ie_proc_ht_sta::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    us_offset = *pus_index;
    pst_mac_user = pst_mac_user_ap;
    pst_ht_hdl = &st_ht_hdl;
    mac_user_get_ht_hdl(pst_mac_user, pst_ht_hdl);

    /* 带有 HT Capability Element 的 AP，标示它具有HT capable. */
    pst_ht_hdl->en_ht_capable = OAL_TRUE;

    us_offset += MAC_IE_HDR_LEN;

    /********************************************/
    /* 解析 HT Capabilities Info Field */
    /********************************************/
    *pus_ht_cap_info = oal_make_word16(puc_payload[us_offset], puc_payload[us_offset + 1]);

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    pst_ht_hdl->bit_ldpc_coding_cap = (*pus_ht_cap_info & BIT0);

    /* 提取AP所支持的带宽能力 */
    pst_ht_hdl->bit_supported_channel_width = ((*pus_ht_cap_info & BIT1) >> 1);

    /* 检查空间复用节能模式 B2~B3 */
    uc_smps = (*pus_ht_cap_info & (BIT2 | BIT3));
    pst_ht_hdl->bit_sm_power_save = mac_ie_proc_sm_power_save_field(pst_mac_user, uc_smps);

    /* 提取AP支持Greenfield情况（bit_ht_green_field是第4bit） */
    pst_ht_hdl->bit_ht_green_field = ((*pus_ht_cap_info & BIT4) >> 4);

    /* 提取AP支持20MHz Short-GI情况（bit_short_gi_20mhz是第5bit） */
    pst_ht_hdl->bit_short_gi_20mhz = ((*pus_ht_cap_info & BIT5) >> 5);

    /* 提取AP支持40MHz Short-GI情况（bit_short_gi_40mhz是第6bit） */
    pst_ht_hdl->bit_short_gi_40mhz = ((*pus_ht_cap_info & BIT6) >> 6);

    /* 提取AP支持STBC PPDU情况（bit_rx_stbc是第8bit） */
    pst_ht_hdl->bit_rx_stbc = (oal_uint8)((*pus_ht_cap_info & (BIT8 | BIT9)) >> 8);

    /* 提取AP支持最大A-MSDU长度情况 */
    if ((*pus_ht_cap_info & BIT11) == 0) {
        *pus_amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_SHORT;
    } else {
        *pus_amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_LONG;
    }

    /* 提取AP 40M上DSSS/CCK的支持情况（bit_dsss_cck_mode_40mhz是第12bit） */
    pst_ht_hdl->bit_dsss_cck_mode_40mhz = ((*pus_ht_cap_info & BIT12) >> 12);

    /* 提取AP L-SIG TXOP 保护的支持情况（bit_lsig_txop_protection是第15bit） */
    pst_ht_hdl->bit_lsig_txop_protection = ((*pus_ht_cap_info & BIT15) >> 15);

    us_offset += MAC_HT_CAPINFO_LEN;

    /* 解析 A-MPDU Parameters Field */
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    pst_ht_hdl->uc_max_rx_ampdu_factor = (puc_payload[us_offset] & 0x03);

    /* 提取 Minmum Rx A-MPDU factor (B3 - B2) */
    pst_ht_hdl->uc_min_mpdu_start_spacing = (puc_payload[us_offset] >> 2) & 0x07;

    us_offset += MAC_HT_AMPDU_PARAMS_LEN;

    /********************************************/
    /* 解析 Supported MCS Set Field */
    /********************************************/
    for (uc_mcs_bmp_index = 0; uc_mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; uc_mcs_bmp_index++) {
        pst_ht_hdl->uc_rx_mcs_bitmask[uc_mcs_bmp_index] = (*(oal_uint8 *)(puc_payload + us_offset + uc_mcs_bmp_index));
    }

    pst_ht_hdl->uc_rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    us_offset += MAC_HT_SUP_MCS_SET_LEN;

    /********************************************/
    /* 解析 HT Extended Capabilities Info Field */
    /********************************************/
    *pus_ht_cap_info = oal_make_word16(puc_payload[us_offset], puc_payload[us_offset + 1]);

    /* 提取 HTC support Information */
    if ((*pus_ht_cap_info & BIT10) != 0) {
        pst_ht_hdl->uc_htc_support = 1;
    }
    us_offset += MAC_HT_EXT_CAP_LEN;

    /********************************************/
    /* 解析 Tx Beamforming Field */
    /********************************************/
    us_tmp_info_elem = oal_make_word16(puc_payload[us_offset], puc_payload[us_offset + 1]);
    /* 第2byte和第3byte拼接为txbf字段低16bit */
    us_tmp_txbf_low = oal_make_word16(puc_payload[us_offset + 2], puc_payload[us_offset + 3]);
    ul_tmp_txbf_elem = oal_make_word32(us_tmp_info_elem, us_tmp_txbf_low);
    pst_ht_hdl->bit_imbf_receive_cap = (ul_tmp_txbf_elem & BIT0);
    pst_ht_hdl->bit_receive_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT1) >> 1);
    /* bit_transmit_staggered_sounding_cap是第2bit */
    pst_ht_hdl->bit_transmit_staggered_sounding_cap = ((ul_tmp_txbf_elem & BIT2) >> 2);
    pst_ht_hdl->bit_receive_ndp_cap = ((ul_tmp_txbf_elem & BIT3) >> 3); /* bit_receive_ndp_cap是第3bit */
    pst_ht_hdl->bit_transmit_ndp_cap = ((ul_tmp_txbf_elem & BIT4) >> 4); /* bit_transmit_ndp_cap是第4bit */
    pst_ht_hdl->bit_imbf_cap = ((ul_tmp_txbf_elem & BIT5) >> 5); /* bit_imbf_cap是第5bit */
    pst_ht_hdl->bit_calibration = ((ul_tmp_txbf_elem & 0x000000C0) >> 6); /* bit_calibration是第6bit */
    pst_ht_hdl->bit_exp_csi_txbf_cap = ((ul_tmp_txbf_elem & BIT8) >> 8); /* bit_exp_csi_txbf_cap是第8bit */
    pst_ht_hdl->bit_exp_noncomp_txbf_cap = ((ul_tmp_txbf_elem & BIT9) >> 9); /* bit_exp_noncomp_txbf_cap是第9bit */
    pst_ht_hdl->bit_exp_comp_txbf_cap = ((ul_tmp_txbf_elem & BIT10) >> 10); /* bit_exp_comp_txbf_cap是第10bit */
    pst_ht_hdl->bit_exp_csi_feedback = ((ul_tmp_txbf_elem & 0x00001800) >> 11); /* bit_exp_csi_feedback是第11bit */
    /* bit_exp_noncomp_feedback是第13bit */
    pst_ht_hdl->bit_exp_noncomp_feedback = ((ul_tmp_txbf_elem & 0x00006000) >> 13);
    pst_ht_hdl->bit_exp_comp_feedback = ((ul_tmp_txbf_elem & 0x0001C000) >> 15); /* bit_exp_comp_feedback是第15bit */
    pst_ht_hdl->bit_min_grouping = ((ul_tmp_txbf_elem & 0x00060000) >> 17); /* bit_min_grouping是第17bit */
    /* bit_csi_bfer_ant_number是第19bit */
    pst_ht_hdl->bit_csi_bfer_ant_number = ((ul_tmp_txbf_elem & 0x001C0000) >> 19);
    /* bit_noncomp_bfer_ant_number是第21bit */
    pst_ht_hdl->bit_noncomp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x00600000) >> 21);
    /* bit_comp_bfer_ant_number是第23bit */
    pst_ht_hdl->bit_comp_bfer_ant_number = ((ul_tmp_txbf_elem & 0x01C00000) >> 23);
    pst_ht_hdl->bit_csi_bfee_max_rows = ((ul_tmp_txbf_elem & 0x06000000) >> 25); /* bit_csi_bfee_max_rows是第25bit */
    pst_ht_hdl->bit_channel_est_cap = ((ul_tmp_txbf_elem & 0x18000000) >> 27); /* bit_channel_est_cap是第27bit */

    mac_user_set_ht_hdl(pst_mac_user, pst_ht_hdl);

    return OAL_SUCC;
}


oal_bool_enum_uint8 mac_ie_check_p2p_action(oal_uint8 *puc_payload)
{
    /* 找到WFA OUI */
    if ((oal_memcmp(puc_payload, g_auc_p2p_oui, MAC_OUI_LEN) == 0) &&
        (puc_payload[MAC_OUI_LEN] == MAC_OUITYPE_P2P)) {
        /* 找到WFA P2P v1.0 oui type */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


wlan_mib_mimo_power_save_enum_uint8 mac_ie_proc_sm_power_save_field(mac_user_stru *pst_mac_user, oal_uint8 uc_smps)
{
    if (uc_smps == MAC_SMPS_STATIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_STATIC;
    } else if (uc_smps == MAC_SMPS_DYNAMIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
    } else {
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }
}


oal_uint8 mac_ie_get_chan_num(
    oal_uint8 *puc_frame_body, oal_uint16 us_frame_len, oal_uint16 us_offset, oal_uint8 uc_curr_chan)
{
    oal_uint8 uc_chan_num = 0;
    oal_uint8 *puc_ie_start_addr;

    /* 在DSSS Param set ie中解析chan num */
    puc_ie_start_addr = mac_find_ie(MAC_EID_DSPARMS, puc_frame_body + us_offset, us_frame_len - us_offset);
    if ((puc_ie_start_addr != OAL_PTR_NULL) && (puc_ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        uc_chan_num = puc_ie_start_addr[2]; /* byte2表示信道数 */
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(uc_chan_num), uc_chan_num) == OAL_SUCC) {
            return uc_chan_num;
        }
    }

    /* 在HT operation ie中解析 chan num */
    puc_ie_start_addr = mac_find_ie(MAC_EID_HT_OPERATION, puc_frame_body + us_offset, us_frame_len - us_offset);
    if ((puc_ie_start_addr != OAL_PTR_NULL) && (puc_ie_start_addr[1] >= 1)) {
        uc_chan_num = puc_ie_start_addr[2]; /* byte2表示信道数 */
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(uc_chan_num), uc_chan_num) == OAL_SUCC) {
            return uc_chan_num;
        }
    }

    uc_chan_num = uc_curr_chan;
    return uc_chan_num;
}


oal_uint32 mac_ie_proc_ext_cap_ie(mac_user_stru *pst_mac_user, oal_uint8 *puc_payload)
{
    mac_user_cap_info_stru *pst_cap_info = OAL_PTR_NULL;
    oal_uint8 uc_len;
    oal_uint8 auc_cap[8] = { 0 };

    if (oal_unlikely((pst_mac_user == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_ie_proc_ext_cap_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cap_info = &(pst_mac_user->st_cap_info);
    uc_len = puc_payload[1];
    if ((uc_len >= MAC_IE_HDR_LEN) && (uc_len <= 8)) { /* 8代表auc_cap数组的长度 */
        if (memcpy_s(auc_cap, OAL_SIZEOF(auc_cap), &puc_payload[MAC_IE_HDR_LEN],
            uc_len - MAC_IE_HDR_LEN) != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_ie_proc_ext_cap_ie::memcpy fail!");
            return OAL_FAIL;
        }
    }

    /* 提取 BIT12: 支持proxy arp */
    pst_cap_info->bit_proxy_arp = ((auc_cap[1] & BIT4) == 0) ? OAL_FALSE : OAL_TRUE;
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_uint32 mac_check_is_assoc_frame(oal_uint8 uc_mgmt_frm_type)
{
    if ((uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_RSP) ||
        (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ||
        (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_RSP) ||
        (uc_mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif


oal_uint32 mac_set_second_channel_offset_ie(wlan_channel_bandwidth_enum_uint8 en_bw,
                                            oal_uint8 *pauc_buffer,
                                            oal_uint8 *puc_output_len)
{
    if ((pauc_buffer == OAL_PTR_NULL) || (puc_output_len == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{mac_set_second_channel_offset_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *pauc_buffer = '\0';
    *puc_output_len = 0;

    /* 11n 设置Secondary Channel Offset Element */
    /******************************************************************/
    /* -------------------------------------------------------------- */
    /* |Ele. ID |Length |Secondary channel offset | */
    /* -------------------------------------------------------------- */
    /* |1       |1      |1                        | */
    /******************************************************************/
    pauc_buffer[0] = 62; /* 62代表辅助信道偏移ie */
    pauc_buffer[1] = 1;

    switch (en_bw) {
        case WLAN_BAND_WIDTH_20M:
            pauc_buffer[2] = 0; /* byte2表示辅助信道，0表示没有辅助信道 */
            break;

        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            pauc_buffer[2] = 1; /* byte2表示辅助信道，1表示中心频点在主信道以上 */
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            pauc_buffer[2] = 3; /* byte2表示辅助信道，3表示中心频点在主信道以下 */
            break;

        default:
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{mac_set_second_channel_offset_ie::invalid bandwidth[%d].}", en_bw);

            return OAL_FAIL;
    }

    *puc_output_len = 3; /* 3表示此ie的长度 */

    return OAL_SUCC;
}


oal_uint32 mac_set_11ac_wideband_ie(oal_uint8 uc_channel,
                                    wlan_channel_bandwidth_enum_uint8 en_bw,
                                    oal_uint8 *pauc_buffer,
                                    oal_uint8 *puc_output_len)
{
    if ((pauc_buffer == OAL_PTR_NULL) || (puc_output_len == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{mac_set_11ac_wideband_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *pauc_buffer = '\0';
    *puc_output_len = 0;

    /* 11ac 设置Wide Bandwidth Channel Switch Element */
    /* -------------------------------------------------------------- */
    /* |ID |Length |New Ch width |Center Freq seg1 |Center Freq seg2 */
    /* -------------------------------------------------------------- */
    /* |1  |1      |1            |1                |1 */
    pauc_buffer[0] = 194; /* 194表示带宽信道切换IE */
    pauc_buffer[1] = 3; /* 3表示Length */
    switch (en_bw) {
        case WLAN_BAND_WIDTH_20M:
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_40MINUS:
            pauc_buffer[2] = 0; /* byte2表示新的信道宽度 */
            pauc_buffer[3] = 0; /* byte3表示中心频率seg1 */
            break;

        case WLAN_BAND_WIDTH_80PLUSPLUS:
            pauc_buffer[2] = 1; /* byte2表示新的信道宽度 */
            pauc_buffer[3] = uc_channel + 6; /* byte3表示中心频率seg1，中心频率相对于主20偏6个信道 */
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            pauc_buffer[2] = 1; /* byte2表示新的信道宽度 */
            pauc_buffer[3] = uc_channel - 2; /* byte3表示中心频率seg1，中心频率相对于主20偏-2个信道 */
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            pauc_buffer[2] = 1; /* byte2表示新的信道宽度 */
            pauc_buffer[3] = uc_channel + 2; /* byte3表示中心频率seg1，中心频率相对于主20偏2个信道 */
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            pauc_buffer[2] = 1; /* byte2表示新的信道宽度 */
            pauc_buffer[3] = uc_channel - 6; /* byte3表示中心频率seg1，中心频率相对于主20偏-6个信道 */
            break;

        default:
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{mac_set_11ac_wideband_ie::invalid bandwidth[%d].}", en_bw);

            return OAL_FAIL;
    }

    pauc_buffer[4] = 0; /* byte4表示中心频率seg1,保留，不支持80M + 80M */

    *puc_output_len = 5; /* 5表示此ie的长度 */

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

oal_uint32 mac_ie_proc_chwidth_field(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_chwidth)
{
    wlan_bw_cap_enum_uint8 en_bwcap_vap = 0;  /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 en_bwcap_user; /* user之前的带宽信息 */

    if (oal_unlikely((pst_mac_user == OAL_PTR_NULL) || (pst_mac_vap == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_2040, "{mac_ie_proc_opmode_field::pst_mac_user = [%x],pst_mac_vap = [%x]!}\r\n",
                       (uintptr_t)pst_mac_user, (uintptr_t)pst_mac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_bwcap_user = pst_mac_user->en_avail_bandwidth;

    mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bwcap_vap);
    en_bwcap_vap = oal_min(en_bwcap_vap, (wlan_bw_cap_enum_uint8)uc_chwidth);
    mac_user_set_bandwidth_info(pst_mac_user, en_bwcap_vap, en_bwcap_vap);

    // l00311403TODO
    if (en_bwcap_user != pst_mac_user->en_avail_bandwidth) {
        /* 调用算法钩子函数 */
        // 后面需要抛事件到dmac, dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);
    }

    return OAL_SUCC;
}
#endif

oal_uint32 mac_ie_proc_sec_chan_offset_2040(mac_vap_stru *pst_mac_vap, mac_sec_ch_off_enum_uint8 en_sec_chan_offset)
{
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_2040, "{mac_ie_proc_sec_chan_offset_2040::pst_mac_vap null.}");
        return MAC_NO_CHANGE;
    }

    /* 先判断是否支持HT模式,以及40M,不支持则无需带宽检查 */
    if ((mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap) == OAL_FALSE)
        || (mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap) == OAL_FALSE)) {
        return MAC_NO_CHANGE;
    }

    /* HT Operation IE中的"次信道偏移量"与当前STA的"带宽模式"不符 */
    if (en_sec_chan_offset != mac_get_sco_from_bandwidth(pst_mac_vap->st_channel.en_bandwidth)) {
        pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

        /* 更新带宽模式 */
        if (en_sec_chan_offset == MAC_SCA) {
            pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        } else if (en_sec_chan_offset == MAC_SCB) {
            pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
        }

        /* 需要设置硬件以切换带宽 */
        return MAC_BW_CHANGE;
    }

    return MAC_NO_CHANGE;
}


oal_uint32 mac_proc_ht_opern_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_user_stru *pst_mac_user)
{
    mac_ht_opern_stru *pst_ht_opern = OAL_PTR_NULL;
    mac_user_ht_hdl_stru st_ht_hdl;
    wlan_bw_cap_enum_uint8 en_bwcap_vap;
    oal_uint32 ul_change = MAC_NO_CHANGE;
    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) ||
        (puc_payload == OAL_PTR_NULL) || (pst_mac_user == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_proc_ht_opern_ie::param null.}");
        return ul_change;
    }

    /* 长度校验，此处仅用到前6字节，后面Basic MCS Set未涉及 */
    if (puc_payload[1] < 6) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_proc_ht_opern_ie::invalid ht opern ie len[%d].}", puc_payload[1]);
        return ul_change;
    }

    mac_user_get_ht_hdl(pst_mac_user, &st_ht_hdl);

    /************************ HT Operation Element *************************************
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
    pst_ht_opern = (mac_ht_opern_stru *)(&puc_payload[MAC_IE_HDR_LEN]);

    /* #ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST */
    /* 提取HT Operation IE中的"STA Channel Width" */
    mac_user_set_bandwidth_info(pst_mac_user, pst_ht_opern->bit_sta_chan_width, pst_mac_user->en_cur_bandwidth);

    /* 提取HT Operation IE中的"Secondary Channel Offset" */
    st_ht_hdl.bit_secondary_chan_offset = pst_ht_opern->bit_secondary_chan_offset;

    /* 为了防止5G下用户声称20M，但发送80M数据的情况，在5G情况下该变量不切换 */
    if ((pst_mac_user->en_avail_bandwidth == 0) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        st_ht_hdl.bit_secondary_chan_offset = MAC_SCN;
    }

    ul_change = mac_ie_proc_sec_chan_offset_2040(pst_mac_vap, pst_ht_opern->bit_secondary_chan_offset);

    /* 用户与VAP带宽能力取交集 */
    mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bwcap_vap);
    en_bwcap_vap = oal_min(pst_mac_user->en_avail_bandwidth, en_bwcap_vap);
    mac_user_set_bandwidth_info(pst_mac_user, en_bwcap_vap, en_bwcap_vap);

    /* 保护相关 */
    st_ht_hdl.bit_rifs_mode = pst_ht_opern->bit_rifs_mode; /* 发送描述符填写时候需要此值 */
    st_ht_hdl.bit_HT_protection = pst_ht_opern->bit_HT_protection;
    st_ht_hdl.bit_nongf_sta_present = pst_ht_opern->bit_nongf_sta_present;
    st_ht_hdl.bit_obss_nonht_sta_present = pst_ht_opern->bit_obss_nonht_sta_present;
    st_ht_hdl.bit_lsig_txop_protection_full_support = pst_ht_opern->bit_lsig_txop_protection_full_support;

    mac_user_set_ht_hdl(pst_mac_user, &st_ht_hdl);

    return ul_change;
}

oal_uint32 mac_ie_proc_obss_scan_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload)
{
    oal_uint16 us_trigger_scan_interval;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{mac_ie_proc_obss_scan_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /********************Overlapping BSS Scan Parameters element******************
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
    if (puc_payload[1] < MAC_OBSS_SCAN_IE_LEN) {
        return OAL_FAIL;
    }
    /* 第6byte和第7byte拼接为trigger_scan_interval的16bit数 */
    us_trigger_scan_interval = oal_make_word16(puc_payload[6], puc_payload[7]);
    if (us_trigger_scan_interval == 0) {
        mac_vap_set_peer_obss_scan(pst_mac_vap, OAL_FALSE);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 第2byte和第3byte拼接为ul_dot11OBSSScanPassiveDwell的16bit数 */
    mac_mib_set_OBSSScanPassiveDwell(pst_mac_vap, oal_make_word16(puc_payload[2], puc_payload[3]));
    /* 第4byte和第5byte拼接为ul_dot11OBSSScanActiveDwell的16bit数 */
    mac_mib_set_OBSSScanActiveDwell(pst_mac_vap, oal_make_word16(puc_payload[4], puc_payload[5]));
    /* obss扫描周期最小300秒,最大600S, 初始化默认为300秒 */
    mac_mib_set_BSSWidthTriggerScanInterval(pst_mac_vap, oal_min(oal_max(us_trigger_scan_interval, 300), 600));
    /* 第8byte和第9byte拼接为ul_dot11OBSSScanPassiveTotalPerChannel的16bit数 */
    mac_mib_set_OBSSScanPassiveTotalPerChannel(pst_mac_vap, oal_make_word16(puc_payload[8], puc_payload[9]));
    /* 第10byte和第11byte拼接为ul_dot11OBSSScanActiveTotalPerChannel的16bit数 */
    mac_mib_set_OBSSScanActiveTotalPerChannel(pst_mac_vap, oal_make_word16(puc_payload[10], puc_payload[11]));
    /* 第12byte和第13byte拼接为ul_dot11BSSWidthChannelTransitionDelayFactor的16bit数 */
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(pst_mac_vap, oal_make_word16(puc_payload[12], puc_payload[13]));
    /* 第14byte和第15byte拼接为ul_dot11OBSSScanActivityThreshold的16bit数 */
    mac_mib_set_OBSSScanActivityThreshold(pst_mac_vap, oal_make_word16(puc_payload[14], puc_payload[15]));
    mac_vap_set_peer_obss_scan(pst_mac_vap, OAL_TRUE);

    return OAL_SUCC;
}

oal_uint32 mac_ie_proc_vht_opern_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_user_stru *pst_mac_user)
{
    mac_vht_hdl_stru st_vht_hdl;
    mac_vht_hdl_stru *pst_vht_hdl = &st_vht_hdl;

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    wlan_bw_cap_enum_uint8 en_bwcap_vap;
#endif

    oal_uint16 us_basic_mcs_set_all_user;

    if (oal_unlikely((pst_mac_vap == OAL_PTR_NULL) || (puc_payload == OAL_PTR_NULL))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_ie_proc_vht_opern_ie::param null.}");

        return MAC_NO_CHANGE;
    }

    /* 长度校验 */
    if (puc_payload[1] < MAC_VHT_OPERN_LEN) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_ie_proc_vht_opern_ie::invalid vht opern len[%d].}", puc_payload[1]);
        return MAC_NO_CHANGE;
    }

    mac_user_get_vht_hdl(pst_mac_user, pst_vht_hdl);

    /* 解析 "VHT Operation Information" */
    pst_vht_hdl->uc_channel_width = puc_payload[MAC_IE_HDR_LEN];
    pst_vht_hdl->uc_channel_center_freq_seg0 = puc_payload[MAC_IE_HDR_LEN + 1];
    /* MAC_IE_HDR_LEN偏移2个byte表示Basic MCS Set */
    pst_vht_hdl->uc_channel_center_freq_seg1 = puc_payload[MAC_IE_HDR_LEN + 2];

    /* 0 -- 20/40M, 1 -- 80M, 2 -- 160M, 3--80M+80M */
    if (pst_vht_hdl->uc_channel_width > 3) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_ie_proc_vht_opern_ie::invalid channel width[%d], use 20M chn width.}",
                         pst_vht_hdl->uc_channel_width);
        pst_vht_hdl->uc_channel_width = 0;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 只有切换至>=80MHz才进行处理，从80MHz+切换至更低等级的带宽，这里无需处理(在解析HT Operation IE中处理) */
    if (pst_vht_hdl->uc_channel_width >= 1) {
        if (pst_vht_hdl->uc_channel_width == 1) {
            en_bandwidth = mac_get_bandwith_from_center_freq_seg0(pst_mac_vap->st_channel.uc_chan_number,
                                                                  pst_vht_hdl->uc_channel_center_freq_seg0);
            /* VHT Operation IE计算出的"带宽模式"与当前STA的"带宽模式"不符 */
            if ((en_bandwidth != pst_mac_vap->st_channel.en_bandwidth) &&
                (mac_mib_get_VHTChannelWidthOptionImplemented(pst_mac_vap) == WLAN_MIB_VHT_SUPP_WIDTH_80)) {
                /* 更新带宽模式 */
                pst_mac_vap->st_channel.en_bandwidth = en_bandwidth;

                /* 需要设置硬件以切换带宽 */
                return MAC_BW_CHANGE;
            }
        }

        /* 用户与VAP带宽能力取交集 */
        en_bwcap_vap = (pst_vht_hdl->uc_channel_width == 1) ? WLAN_BW_CAP_80M : WLAN_BW_CAP_160M;
        mac_user_set_bandwidth_info(pst_mac_user, en_bwcap_vap, pst_mac_user->en_cur_bandwidth);

        mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bwcap_vap);

        en_bwcap_vap = oal_min(pst_mac_user->en_avail_bandwidth, en_bwcap_vap);
        mac_user_set_bandwidth_info(pst_mac_user, en_bwcap_vap, en_bwcap_vap);
    }
#endif

    /* 解析 "VHT Basic MCS Set field" */
    /* byte 3和byte 4表示拼接成us_basic_mcs_set_all_user的16bit数 */
    us_basic_mcs_set_all_user = oal_make_word16(puc_payload[MAC_IE_HDR_LEN + 3], puc_payload[MAC_IE_HDR_LEN + 4]);
    pst_vht_hdl->us_basic_mcs_set = us_basic_mcs_set_all_user;

    mac_user_set_vht_hdl(pst_mac_user, pst_vht_hdl);

    return MAC_NO_CHANGE;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_uint32 mac_ie_check_proc_opmode_param(mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify)
{
    /* USER新限定带宽、空间流不允许大于其能力 */
    if ((pst_mac_user->en_bandwidth_cap < pst_opmode_notify->bit_channel_width)
        || (pst_mac_user->uc_num_spatial_stream < pst_opmode_notify->bit_rx_nss)) {
        oam_warning_log4(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{hmac_ie_check_proc_opmode_param::user cap over limit! \
            en_bandwidth_cap = [%d], opmode bandwidth = [%d], user spatial stream = [%d], opmode rx nss = [%d]!}",
                         pst_mac_user->en_bandwidth_cap,
                         pst_opmode_notify->bit_channel_width,
                         pst_mac_user->uc_num_spatial_stream,
                         pst_opmode_notify->bit_rx_nss);
        return OAL_FAIL;
    }

    /* Nss Type值为1，则表示beamforming Rx Nss不能超过其声称值 */
    if (pst_opmode_notify->bit_rx_nss_type == 1) {
        if (pst_mac_user->st_vht_hdl.bit_num_bf_ant_supported < pst_opmode_notify->bit_rx_nss) {
            oam_warning_log2(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{hmac_ie_check_proc_opmode_param::bit_rx_nss is \
                over limit!bit_num_bf_ant_supported = [%d], bit_rx_nss = [%d]!}",
                             pst_mac_user->st_vht_hdl.bit_num_bf_ant_supported,
                             pst_opmode_notify->bit_rx_nss);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}


oal_uint32 mac_ie_proc_opmode_field(
    mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_opmode_notify_stru *pst_opmode_notify)
{
    wlan_bw_cap_enum_uint8 en_bwcap_vap = 0; /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 en_avail_bw = 0;  /* vap自身带宽能力 */

    if (oal_unlikely((pst_mac_user == OAL_PTR_NULL) ||
                     (pst_opmode_notify == OAL_PTR_NULL) ||
                     (pst_mac_vap == OAL_PTR_NULL))) {
        oam_error_log3(0, OAM_SF_ANY, "{mac_ie_proc_opmode_field::pst_mac_user = [%x], pst_opmode_notify = [%x], \
            pst_mac_vap = [%x]!}", (uintptr_t)pst_mac_user, (uintptr_t)pst_opmode_notify, (uintptr_t)pst_mac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (g_st_mac_ie_rom_cb.p_mac_ie_proc_opmode_field(pst_mac_vap, pst_mac_user, pst_opmode_notify) == OAL_RETURN) {
        return OAL_SUCC;
    }

    if (mac_ie_check_proc_opmode_param(pst_mac_user, pst_opmode_notify) == OAL_FAIL) {
        oam_warning_log0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{mac_ie_proc_opmode_field:: \
            hmac_ie_check_proc_opmode_param return fail!}\r\n");
        return OAL_FAIL;
    }

    /* 判断Rx Nss Type是否为beamforming模式 */
    if (pst_opmode_notify->bit_rx_nss_type == 1) {
        /* 判断Rx Nss是否与user之前使用Rx Nss相同 */
        if (pst_opmode_notify->bit_rx_nss != pst_mac_user->uc_avail_bf_num_spatial_stream) {
            mac_user_avail_bf_num_spatial_stream(pst_mac_user, pst_opmode_notify->bit_rx_nss);
        }

        return OAL_SUCC;
    }

    /* 判断Rx Nss是否与user之前使用Rx Nss相同 */
    if (pst_opmode_notify->bit_rx_nss != pst_mac_user->uc_num_spatial_stream) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_ie_proc_opmode_field:: \
            pst_opmode_notify->bit_rx_nss = [%x], pst_mac_user->uc_num_spatial_stream = [%x]!}\r\n",
                         pst_opmode_notify->bit_rx_nss, pst_mac_user->uc_num_spatial_stream);
        /* 需要获取vap和更新nss的取小，如果我们不支持mimo了，对端宣传切换mimo也不执行 */
        mac_user_set_avail_num_spatial_stream(pst_mac_user, oal_min(pst_mac_vap->en_vap_rx_nss,
                                                                    pst_opmode_notify->bit_rx_nss));
    }
    /* 判断channel_width是否与user之前使用channel_width相同 */
    if (pst_opmode_notify->bit_channel_width != pst_mac_user->en_avail_bandwidth) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_ie_proc_opmode_field:: \
            pst_opmode_notify->bit_channel_width = [%x], pst_mac_user->en_avail_bandwidth = [%x]!}\r\n",
                         pst_opmode_notify->bit_channel_width, pst_mac_user->en_avail_bandwidth);

        /* 获取vap带宽能力与用户带宽能力的交集 */
        mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bwcap_vap);

        en_avail_bw = oal_min(en_bwcap_vap, pst_opmode_notify->bit_channel_width);
        mac_user_set_bandwidth_info(pst_mac_user, en_avail_bw, en_avail_bw);
    }
    return OAL_SUCC;
}
#endif
wlan_channel_bandwidth_enum_uint8 mac_get_80mbandwith_from_offset_channel(int8_t offset_channel)
{
    switch (offset_channel) {
        case 6: /* 中心频率相对于主20偏6个信道 */
            return WLAN_BAND_WIDTH_80PLUSPLUS;
        case -2: /* 中心频率相对于主20偏-2个信道 */
            return WLAN_BAND_WIDTH_80PLUSMINUS;
        case 2: /* 中心频率相对于主20偏2个信道 */
            return WLAN_BAND_WIDTH_80MINUSPLUS;
        case -6: /* 中心频率相对于主20偏-6个信道 */
            return WLAN_BAND_WIDTH_80MINUSMINUS;
        default:
            return WLAN_BAND_WIDTH_20M;
    }
}
wlan_channel_bandwidth_enum_uint8 mac_get_bandwith_from_center_freq_seg0_seg1(
    uint8_t chan_width, uint8_t channel, uint8_t chan_center_freq0, uint8_t chan_center_freq1)
{
    /* 80+80不支持，暂按80M处理 */
    if (chan_width == WLAN_MIB_VHT_OP_WIDTH_80 || chan_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
        return mac_get_80mbandwith_from_offset_channel((int8_t)(chan_center_freq0 - channel));
    } else {
        return WLAN_BAND_WIDTH_20M;
    }
}

oal_module_symbol(mac_ie_proc_sm_power_save_field);
oal_module_symbol(mac_ie_proc_ht_green_field);
oal_module_symbol(mac_ie_get_chan_num);
oal_module_symbol(mac_ie_proc_ht_supported_channel_width);
oal_module_symbol(mac_ie_proc_lsig_txop_protection_support);
oal_module_symbol(mac_ie_proc_ext_cap_ie);

oal_module_symbol(mac_set_second_channel_offset_ie);
oal_module_symbol(mac_set_11ac_wideband_ie);
oal_module_symbol(mac_ie_proc_ht_sta);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
oal_module_symbol(mac_ie_proc_chwidth_field);
#endif
oal_module_symbol(mac_ie_get_vht_rx_mcs_map);
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
oal_module_symbol(mac_ie_proc_opmode_field);
#endif
oal_module_symbol(mac_ie_proc_obss_scan_ie);
oal_module_symbol(mac_ie_proc_sec_chan_offset_2040);
oal_module_symbol(mac_proc_ht_opern_ie);
oal_module_symbol(mac_ie_proc_vht_opern_ie);

