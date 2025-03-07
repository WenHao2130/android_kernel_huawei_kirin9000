

#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_ioctl.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif
#include "wal_linux_cfg80211.h"
#include "hmac_config.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_function.h"
#include "hmac_chba_interface.h"
#include "hmac_chba_mgmt.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFGVENDOR_C

#define OUI_VENDOR 0x001A11
#define OUI_HISI   0x001018
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))) && \
    (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

wal_cfgvendor_radio_stat_stru g_st_wifi_radio_stat;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static const struct nla_policy wal_andr_wifi_attr_policy[ANDR_WIFI_ATTRIBUTE_MAX + 1] = {
    [ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * WLAN_RANDOM_MAC_OUI_LEN },
    [ANDR_WIFI_ATTRIBUTE_COUNTRY] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * WLAN_COUNTRY_STR_LEN },
};

static const struct nla_policy wal_gscan_attr_policy[GSCAN_ATTRIBUTE_MAX] = {
    [GSCAN_ATTRIBUTE_BAND] = { .type = NLA_U32 },
};

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_VOWIFI_NAT))
static const struct nla_policy wal_mkeep_alive_attr_policy[MKEEP_ALIVE_ATTRIBUTE_MAX + 1] = {
    [MKEEP_ALIVE_ATTRIBUTE_ID] = { .type = NLA_U8 },
    [MKEEP_ALIVE_ATTRIBUTE_IP_PKT] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * MKEEP_ALIVE_IP_PKT_MAXLEN },
    [MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN] = { .type = NLA_U16 },
    [MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * WLAN_MAC_ADDR_LEN },
    [MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * WLAN_MAC_ADDR_LEN },
    [MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC] = { .type = NLA_U32 },
    [MKEEP_ALIVE_ATTRIBUTE_ETHER_TYPE] = { .type = NLA_U16 },
};
#endif

#ifdef _PRE_WLAN_FEATURE_APF
static const struct nla_policy wal_apf_attr_policy[APF_ATTRIBUTE_MAX + 1] = {
    [APF_ATTRIBUTE_PROGRAM] =  { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * APF_PROGRAM_MAX_LEN },
    [APF_ATTRIBUTE_PROGRAM_LEN] ={ .type = NLA_U32 },
};
#endif
#endif

#ifdef _PRE_WLAN_CHBA_MGMT
static const struct nla_policy wal_chba_conn_ntf_attr_policy[MAC_CHBA_CONN_NTF_ATTR_MAX + 1] = {
    [MAC_CHBA_CONN_NTF_ATTR_ID] = { .type = NLA_U8 },
    [MAC_CHBA_CONN_NTF_ATTR_MAC_ADDR] = { .type = NLA_BINARY,
        .len = sizeof(NLA_U8) * WLAN_MAC_ADDR_LEN },
    [MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_20M] = { .type = NLA_U32 },
    [MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_1] = { .type = NLA_U32 },
    [MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_2] = { .type = NLA_U32 },
    [MAC_CHBA_CONN_NTF_ATTR_CENTER_BANDWIDTH] = { .type = NLA_U8 },
    [MAC_CHBA_CONN_NTF_ATTR_EXPIRE_TIME] = { .type = NLA_U16 },
};
#endif

OAL_STATIC oal_void wal_cfgvendor_del_radar_channel(mac_vendor_cmd_channel_list_stru *pst_chan_list)
{
    oal_uint8 uc_loops;
    oal_uint8 uc_idx;
    oal_uint8 uc_channel_num;
    oal_uint8 uc_channel_idx;
    oal_uint8 auc_tmp_channel_list[MAC_CHANNEL_FREQ_5_BUTT];

    if (pst_chan_list->uc_channel_num_5g == 0) {
        return;
    }

    uc_channel_idx = 0;
    uc_channel_num = oal_min(MAC_CHANNEL_FREQ_5_BUTT, pst_chan_list->uc_channel_num_5g);

    for (uc_loops = 0; uc_loops < uc_channel_num; uc_loops++) {
        if (mac_get_channel_idx_from_num(MAC_RC_START_FREQ_5,
                                         pst_chan_list->auc_channel_list_5g[uc_loops], &uc_idx) != OAL_SUCC) {
            continue;
        }
        if (mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, uc_idx) == OAL_TRUE) {
            continue;
        }

        auc_tmp_channel_list[uc_channel_idx++] = pst_chan_list->auc_channel_list_5g[uc_loops];
    }
    pst_chan_list->uc_channel_num_5g = uc_channel_idx;
    if (memcpy_s(pst_chan_list->auc_channel_list_5g, MAC_CHANNEL_FREQ_5_BUTT,
        auc_tmp_channel_list, uc_channel_idx) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_del_radar_channel::memcpy fail!}");
    }
}


OAL_STATIC oal_uint32 wal_cfgvendor_copy_channel_list(mac_vendor_cmd_channel_list_stru *pst_channel_list,
                                                      oal_uint32 band,
                                                      oal_int32 *pl_channel_list,
                                                      oal_uint32 *pul_num_channels)
{
    oal_uint8 *puc_chanel_list;
    oal_uint32 ul_loop;
    oal_uint32 ul_channel_num = 0;

    *pul_num_channels = 0;

    if ((pst_channel_list->uc_channel_num_5g == 0) && (pst_channel_list->uc_channel_num_2g == 0)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_copy_channel_list:get channel list fail,band=%d", band);
        return OAL_FAIL;
    }

    /* 获取2G,信道列表 */
    if ((band & VENDOR_BG_BAND_MASK) && (pst_channel_list->uc_channel_num_2g != 0)) {
        puc_chanel_list = pst_channel_list->auc_channel_list_2g;
        for (ul_loop = 0; ul_loop < pst_channel_list->uc_channel_num_2g; ul_loop++) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
            pl_channel_list[ul_channel_num++] =
                oal_ieee80211_channel_to_frequency(puc_chanel_list[ul_loop], NL80211_BAND_2GHZ);
#else
            pl_channel_list[ul_channel_num++] = oal_ieee80211_channel_to_frequency(puc_chanel_list[ul_loop],
                                                                                   IEEE80211_BAND_2GHZ);
#endif
        }
    }

    /* 获取5G,信道列表 */
    if ((band & VENDOR_A_BAND_MASK) && (pst_channel_list->uc_channel_num_5g != 0)) {
        puc_chanel_list = pst_channel_list->auc_channel_list_5g;

        for (ul_loop = 0; ul_loop < pst_channel_list->uc_channel_num_5g; ul_loop++) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
            pl_channel_list[ul_channel_num++] =
                oal_ieee80211_channel_to_frequency(puc_chanel_list[ul_loop], NL80211_BAND_5GHZ);
#else
            pl_channel_list[ul_channel_num++] = oal_ieee80211_channel_to_frequency(puc_chanel_list[ul_loop],
                                                                                   IEEE80211_BAND_5GHZ);
#endif
        }
    }

    if (ul_channel_num == 0) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfgvendor_copy_channel_list::get 5G or 2G channel list fail,ul_channel_num = %d",
                         ul_channel_num);
        return OAL_FAIL;
    }

    *pul_num_channels = ul_channel_num;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 wal_cfgvendor_get_current_channel_list(oal_net_device_stru *pst_netdev,
                                                             oal_uint32 band,
                                                             oal_int32 *pl_channel_list,
                                                             oal_uint32 *pul_num_channels)
{
    oal_int32 l_ret;
    wal_msg_query_stru st_query_msg;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg;
    mac_vendor_cmd_channel_list_stru *pst_channel_list = OAL_PTR_NULL;

    if ((pst_netdev == OAL_PTR_NULL) || (pl_channel_list == OAL_PTR_NULL) || (pul_num_channels == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_get_current_channel_list::channel_list or num_channel is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上层在任何时候都可能下发此命令，需要先判断当前netdev的状态并及时返回 */
    if (oal_unlikely(oal_net_dev_priv(pst_netdev) == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_get_current_channel_list::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 消息参数准备 */
    st_query_msg.en_wid = WLAN_CFGID_VENDOR_CMD_GET_CHANNEL_LIST;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_QUERY,
                               WAL_MSG_WID_LENGTH,
                               (oal_uint8 *)&st_query_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_cfgvendor_get_current_channel_list:: wal_send_cfg_event return err code %d!}", l_ret);
        if (pst_rsp_msg != OAL_PTR_NULL) {
            oal_free(pst_rsp_msg);
        }
        return (oal_uint32)l_ret;
    }

    pst_query_rsp_msg = (wal_msg_rsp_stru *)(pst_rsp_msg->auc_msg_data);
    pst_channel_list = (mac_vendor_cmd_channel_list_stru *)(pst_query_rsp_msg->auc_value);

    /* 从信道列表中删除DFS 信道不上报 */
    wal_cfgvendor_del_radar_channel(pst_channel_list);

    if (wal_cfgvendor_copy_channel_list(pst_channel_list, band, pl_channel_list, pul_num_channels) != OAL_SUCC) {
        oal_free(pst_rsp_msg);
        return OAL_FAIL;
    }

    oal_free(pst_rsp_msg);
    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_cfgvendor_get_channel_list(oal_wiphy_stru *wiphy,
                                                    oal_wireless_dev_stru *wdev,
                                                    OAL_CONST oal_void *data, oal_int32 len)
{
    oal_int32 err = 0;
    oal_int32 type;
    oal_int32 al_channel_list[VENDOR_CHANNEL_LIST_ALL + 1] = { 0 };
    oal_uint32 band = 0; /* 上层下发的band值 */
    oal_uint32 mem_needed;
    oal_uint32 num_channels = 0;
    oal_uint32 reply_len = 0;
    oal_uint32 ul_ret;
    oal_netbuf_stru *skb;

    if ((wdev == OAL_PTR_NULL) || (wdev->netdev == OAL_PTR_NULL) || (wiphy == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_get_channel_list::wdev or netdev is NULL!}");
        return -OAL_EFAIL;
    }
    if ((data == NULL) || (len <= 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_get_channel_list::invalid para l_len=%d.}", len);
        return -OAL_EINVAL;
    }

    type = oal_nla_type(data);
    if (type == GSCAN_ATTRIBUTE_BAND) {
        /* 获取band值 bit0:2G信道列表 bit1:5G信道列表 */
        band = oal_nla_get_u32(data);
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_get_channel_list::invalid nla type! type %d.}", type);
        return -OAL_EFAIL;
    }

    ul_ret = wal_cfgvendor_get_current_channel_list(wdev->netdev, band, al_channel_list, &num_channels);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY, "{wal_cfgvendor_get_channel_list::fail.ret_value %d,band %d, num_channels %d.}",
                         ul_ret, band, num_channels);
        return -OAL_EFAIL;
    }

    reply_len = OAL_SIZEOF(al_channel_list[0]) * (num_channels);
    mem_needed = reply_len + (oal_uint32)(VENDOR_REPLY_OVERHEAD + (ATTRIBUTE_U32_LEN * 2));

    /* 申请SKB 上报信道列表 */
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, mem_needed);
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_get_channel_list::skb alloc failed.len %d\r\n", mem_needed);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u32(skb, GSCAN_ATTRIBUTE_NUM_CHANNELS, num_channels);
    oal_nla_put(skb, GSCAN_ATTRIBUTE_CHANNEL_LIST, reply_len, al_channel_list);

    err = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(err)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_get_channel_list::Vendor Command reply failed ret:%d.\r\n", err);
    }

    oam_warning_log3(0, OAM_SF_ANY, "{wal_cfgvendor_get_channel_list::ret_value %d, band %d, num_channels %d.}",
                     err, band, num_channels);

    return err;
}

OAL_STATIC oal_int32 wal_cfgvendor_set_country(oal_wiphy_stru *wiphy,
                                               oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data, oal_int32 len)
{
    oal_int32 rem;
    oal_int32 type;
    oal_int32 l_ret = OAL_SUCC;
    oal_int8 auc_country_code[WLAN_COUNTRY_STR_LEN] = { 0 };
    OAL_CONST oal_nlattr_stru *iter;

    /* 测试阶段可采用定制化99配置 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag == OAL_TRUE) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_country::set_country ignored, flag ini[%d]",
                         g_st_cust_country_code_ignore_flag.en_country_code_ingore_ini_flag);
        return l_ret;
    }
#endif
    if ((wdev == NULL) || (data == NULL) || (len <= 0)) {
        oam_error_log2(0, 0, "{wal_cfgvendor_set_country::invalid para wdev null [%d] len=%d.}", (wdev == NULL), len);
        return -OAL_EINVAL;
    }

    oal_nla_for_each_attr(iter, data, len, rem)
    {
        memset_s(auc_country_code, WLAN_COUNTRY_STR_LEN, 0, WLAN_COUNTRY_STR_LEN);
        type = oal_nla_type(iter);
        switch (type) {
            case ANDR_WIFI_ATTRIBUTE_COUNTRY:
#ifdef _PRE_WLAN_FEATURE_11D
                if (memcpy_s(auc_country_code, OAL_SIZEOF(auc_country_code), oal_nla_data(iter),
                             oal_min(oal_nla_len(iter), OAL_SIZEOF(auc_country_code))) != EOK) {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_set_country::memcpy fail!");
                    return -OAL_EFAIL;
                }

                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_country:: nla len = [%d]!}\r\n", oal_nla_len(iter));

                /* 设置国家码到wifi 驱动 */
                l_ret = wal_regdomain_update_country_code(wdev->netdev, auc_country_code);
#else
                oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_set_country::_PRE_WLAN_FEATURE_11D is not define!}");
#endif
                break;
            default:
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_country::unknow type:%d!}\r\n", type);
                break;
        }
    }

    return l_ret;
}

OAL_STATIC oal_uint32 wal_cfgvendor_do_get_feature_set(oal_void)
{
    oal_uint32 ul_feature_set = 0;

#ifdef _PRE_WLAN_FEATURE_HS20
    if (mac_get_customize_interworking()) {
        ul_feature_set |= WIFI_FEATURE_HOTSPOT;
    }
#endif

    if (mac_get_band_5g_enabled()) {
        ul_feature_set |= WIFI_FEATURE_INFRA_5G;
    }
    ul_feature_set |= WIFI_FEATURE_LINK_LAYER_STATS; /* 0x10000 Link layer stats collection */

    return ul_feature_set;
}

OAL_STATIC oal_int32 wal_cfgvendor_get_feature_set(oal_wiphy_stru *wiphy,
                                                   oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data,
                                                   oal_int32 len)
{
    oal_int32 l_err = 0;
    oal_uint32 ul_reply;
    oal_int32 l_reply_len = (oal_int32)OAL_SIZEOF(oal_int32);
    oal_netbuf_stru *skb;

    if (wiphy == NULL) {
        return -OAL_EFAUL;
    }
    ul_reply = wal_cfgvendor_do_get_feature_set();

    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, l_reply_len);
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_get_feature_set::skb alloc failed.len %d\r\n", l_reply_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, l_reply_len, &ul_reply);

    l_err = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(l_err)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_get_feature_set::Vendor Command reply failed ret:%d.\r\n", l_err);
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_get_feature_set::fset flag:0x%x.\r\n}", ul_reply);

    return l_err;
}


OAL_STATIC oal_int32 wal_send_random_mac_oui(oal_net_device_stru *pst_net_dev,
                                             oal_uint8 *auc_random_mac_oui)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    if ((pst_net_dev == OAL_PTR_NULL) || (auc_random_mac_oui == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_random_mac_oui:: null point argument}");
        return -OAL_EFAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_RANDOM_MAC_OUI, WLAN_RANDOM_MAC_OUI_LEN);
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), auc_random_mac_oui,
                 WLAN_RANDOM_MAC_OUI_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_send_random_mac_oui::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + WLAN_RANDOM_MAC_OUI_LEN,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_send_random_mac_oui::send rand_mac_oui failed, err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return l_ret;
}


OAL_STATIC oal_int32 wal_cfgvendor_set_random_mac_oui(oal_wiphy_stru *pst_wiphy,
                                                      oal_wireless_dev_stru *pst_wdev,
                                                      OAL_CONST oal_void *p_data,
                                                      oal_int32 l_len)
{
    oal_int32 l_type;
    oal_int32 l_ret;
    oal_uint8 auc_random_mac_oui[WLAN_RANDOM_MAC_OUI_LEN];
    if ((pst_wdev == NULL) || (p_data == NULL) || (l_len <= 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_random_mac_oui::invalid para l_len=%d.}", l_len);
        return -OAL_EINVAL;
    }

    l_type = oal_nla_type(p_data);

    if (l_type == ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI) {
        /* 随机mac地址前3字节(mac oui)由系统下发,wps pbc场景和hilink关联场景会将此3字节清0 */
        if (memcpy_s(auc_random_mac_oui, WLAN_RANDOM_MAC_OUI_LEN, oal_nla_data(p_data), oal_nla_len(p_data)) !=
            EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_set_random_mac_oui::memcpy fail!");
            return -OAL_EFAIL;
        }
        oam_warning_log3(0, OAM_SF_ANY, "{wal_cfgvendor_set_random_mac_oui::mac_ou:0x%.2x:%.2x:%.2x}\r\n",
                         auc_random_mac_oui[0], auc_random_mac_oui[1], auc_random_mac_oui[2]);
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_random_mac_oui:: unknow type:%x}\r\n", l_type);
        return -OAL_EFAIL;
    }

    l_ret = wal_send_random_mac_oui(pst_wdev->netdev, auc_random_mac_oui);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfgvendor_dbg_get_feature(oal_wiphy_stru *wiphy,
                                                   oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data,
                                                   oal_int32 len)
{
    oal_int32 l_ret;
    oal_uint32 supported_features = VENDOR_DBG_MEMORY_DUMP_SUPPORTED;
    oal_netbuf_stru *skb = NULL;
    if (wiphy == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_feature::wiphy is null");
        return -OAL_EINVAL;
    }
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, OAL_SIZEOF(oal_uint32));
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_feature::skb alloc failed. len %d\r\n",
                         OAL_SIZEOF(oal_uint32));
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, OAL_SIZEOF(oal_uint32), &supported_features);

    l_ret = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(l_ret)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_feature::Vendor Command reply failed. ret:%d.\r\n", l_ret);
    }
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_feature::SUCCESS. supported feature:0x%X.\r\n",
                     supported_features);

    return l_ret;
}


OAL_STATIC oal_int32 wal_cfgvendor_dbg_get_version(oal_wiphy_stru *wiphy,
                                                   oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data,
                                                   oal_int32 len)
{
    oal_int8 auc_driver_version[] = "Hisi Host Driver, version Hi110X";
    oal_int32 l_ret;
    oal_int32 l_buf_len = (oal_int32)OAL_SIZEOF(auc_driver_version);
    oal_netbuf_stru *skb = NULL;

    if (wiphy == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_version::wiphy is null");
        return -OAL_EINVAL;
    }

    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, l_buf_len);
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_version::skb alloc failed.len %d\r\n", l_buf_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, l_buf_len, auc_driver_version);

    l_ret = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(l_ret)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_version::Vendor Command reply failed. ret:%d.\r\n", l_ret);
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_version::SUCCESS. driver version len %d\r\n", l_buf_len);
    return l_ret;
}


oal_int32 wal_cfgvendor_dbg_get_ring_status(oal_wiphy_stru *wiphy,
                                            oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data, oal_int32 len)
{
    oal_int32 l_ret, l_buf_len;
    debug_ring_status_st debug_ring_status = { { VENDOR_FW_EVENT_RING_NAME }, 0, FW_EVENT_RING_ID,
        VENDOR_FW_EVENT_RING_SIZE, 0, 0, 0, 0
    };
    oal_netbuf_stru *skb = NULL;
    if (wiphy == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_ring_status::wiphy is null");
        return -OAL_EINVAL;
    }
    l_buf_len = (oal_int32)OAL_SIZEOF(debug_ring_status_st);
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, l_buf_len + 100);
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_ring_status::skb alloc failed.len %d", l_buf_len + 100);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u32(skb, DEBUG_ATTRIBUTE_RING_NUM, 1);
    oal_nla_put(skb, DEBUG_ATTRIBUTE_RING_STATUS, l_buf_len, &debug_ring_status);

    l_ret = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(l_ret)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_ring_status::Vendor Command reply failed. ret:%d.\r\n",
                       l_ret);
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_ring_status::SUCCESS. driver version len %d\r\n",
                     l_buf_len);
    return l_ret;
}


oal_int32 wal_cfgvendor_dbg_trigger_mem_dump(oal_wiphy_stru *wiphy,
                                             oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data, oal_int32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_trigger_mem_dump::ENTER");
    return 0;
}


OAL_STATIC oal_int32 wal_cfgvendor_dbg_start_logging(oal_wiphy_stru *wiphy,
                                                     oal_wireless_dev_stru *wdev,
                                                     OAL_CONST oal_void *data, oal_int32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_start_logging::ENTER");
    return 0;
}


OAL_STATIC oal_int32 wal_cfgvendor_dbg_get_ring_data(oal_wiphy_stru *wiphy,
                                                     oal_wireless_dev_stru *wdev,
                                                     OAL_CONST oal_void *data, oal_int32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "wal_cfgvendor_dbg_get_ring_data::ENTER");
    return 0;
}

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_VOWIFI_NAT))

OAL_STATIC oal_int32 wal_send_vowifi_nat_keep_alive_params(oal_net_device_stru *pst_net_dev,
                                                           oal_uint8 *pc_keep_alive_info, oal_uint8 uc_msg_len)
{
    wal_msg_write_stru st_write_msg;
    oal_int32 l_ret;

    if ((pst_net_dev == OAL_PTR_NULL) || (pc_keep_alive_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_send_vowifi_nat_keep_alive_params:: null point argument.}");
        return -OAL_EFAIL;
    }

    memset_s(&st_write_msg, OAL_SIZEOF(wal_msg_write_stru), 0, OAL_SIZEOF(wal_msg_write_stru));

    /***************************************************************************
        抛事件到wal层处理  WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE, uc_msg_len);
    if (memcpy_s(st_write_msg.auc_value, OAL_SIZEOF(st_write_msg.auc_value),
        pc_keep_alive_info, uc_msg_len) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_send_vowifi_nat_keep_alive_params::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + uc_msg_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{wal_send_vowifi_nat_keep_alive_params::SET_VOWIFI_KEEP_ALIVE failed,err code %d}", l_ret);
        return l_ret;
    }

    return l_ret;
}


oal_void wl_cfgvendor_update_start_info(mac_vowifi_nat_keep_alive_start_info_stru *pst_start_info,
    oal_uint8 uc_keep_alive_id, oal_uint8 uc_ip_pkt_len, oal_uint32 ul_period_msec)
{
    pst_start_info->st_basic_info.uc_keep_alive_id = uc_keep_alive_id;
    pst_start_info->st_basic_info.en_type = VOWIFI_MKEEP_ALIVE_TYPE_START;
    pst_start_info->us_ip_pkt_len = uc_ip_pkt_len;
    pst_start_info->ul_period_msec = ul_period_msec;
}

OAL_STATIC oal_int32 wl_cfgvendor_start_vowifi_nat_keep_alive(oal_wiphy_stru *pst_wiphy,
                                                              oal_wireless_dev_stru *pst_wdev,
                                                              OAL_CONST oal_void *p_data,
                                                              oal_int32 l_len)
{
    oal_int32 l_type;
    oal_int32 l_rem;
    oal_uint32 ul_period_msec = 0;
    oal_uint8 uc_keep_alive_id = 0;
    oal_uint16 ip_pkt_len = 0;
    oal_uint16 ether_type = ETHER_TYPE_IP;
    oal_bool_enum_uint8 en_find_keepid_flag = OAL_FALSE;
    oal_uint16 msg_len;
    oal_uint8 auc_src_mac[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_dst_mac[WLAN_MAC_ADDR_LEN];
    oal_uint8 *puc_ip_pkt = OAL_PTR_NULL;
    OAL_CONST oal_nlattr_stru *pst_iter;
    mac_vowifi_nat_keep_alive_start_info_stru *pst_start_info;
    oal_int32 l_ret = EOK;
    oal_uint8 *keepalive_extend = NULL;

    if (oal_any_null_ptr2(pst_wdev, p_data) || (l_len <= 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wl_cfgvendor_start_vowifi_nat_keep_alive::invalid para l_len=%d.}", l_len);
        return -OAL_EINVAL;
    }

    oal_nla_for_each_attr(pst_iter, p_data, l_len, l_rem)
    {
        l_type = oal_nla_type(pst_iter);
        switch (l_type) {
            case MKEEP_ALIVE_ATTRIBUTE_ID:
                uc_keep_alive_id = oal_nla_get_u8(pst_iter);
                en_find_keepid_flag = OAL_TRUE;
                break;
            case MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN:
                ip_pkt_len = oal_nla_get_u16(pst_iter);
                if ((ip_pkt_len > MKEEP_ALIVE_IP_PKT_MAXLEN) || (ip_pkt_len == 0)) {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wl_start_vowifi_nat_keep_alive::ip_pkt_len=[%d]is invalid.}",
                                     ip_pkt_len);
                    goto err_vowifi_ka_fail;
                }
                break;
            case MKEEP_ALIVE_ATTRIBUTE_IP_PKT:
                if (puc_ip_pkt != OAL_PTR_NULL) {
                    oal_mem_free_m(puc_ip_pkt, OAL_TRUE);
                    puc_ip_pkt = OAL_PTR_NULL;
                }
                if (oal_value_in_valid_range(ip_pkt_len, 1, MKEEP_ALIVE_IP_PKT_MAXLEN)) {
                    puc_ip_pkt = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, ip_pkt_len, OAL_TRUE);
                    if (puc_ip_pkt == OAL_PTR_NULL) {
                        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wl_start_vowifi_nat_keep_alive::alloc%d fail.}", ip_pkt_len);
                        return OAL_EFAIL;
                    }
                    l_ret += memcpy_s(puc_ip_pkt, ip_pkt_len, oal_nla_data(pst_iter), oal_nla_len(pst_iter));
                }
                break;
            case MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR:
                l_ret += memcpy_s(auc_src_mac, WLAN_MAC_ADDR_LEN, oal_nla_data(pst_iter), oal_nla_len(pst_iter));
                break;
            case MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR:
                l_ret += memcpy_s(auc_dst_mac, WLAN_MAC_ADDR_LEN, oal_nla_data(pst_iter), oal_nla_len(pst_iter));
                break;
            case MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC:
                ul_period_msec = oal_nla_get_u32(pst_iter);
                break;
            case MKEEP_ALIVE_ATTRIBUTE_ETHER_TYPE:
                ether_type = oal_nla_get_u16(pst_iter);
                break;
            default:
                goto err_vowifi_ka_fail;
        }
    }

    if ((ul_period_msec == 0) || (en_find_keepid_flag == OAL_FALSE)) {
        goto err_vowifi_ka_fail;
    }

    if (puc_ip_pkt != OAL_PTR_NULL) {
        // 后面的OAL_SIZEOF(oal_uint16)是ip_data内容后面的ether_type字段的值。
        msg_len = ip_pkt_len + OAL_SIZEOF(mac_vowifi_nat_keep_alive_start_info_stru) + OAL_SIZEOF(oal_uint16);
        pst_start_info = (mac_vowifi_nat_keep_alive_start_info_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, msg_len,
                                                                                      OAL_TRUE);
        if (pst_start_info == OAL_PTR_NULL) {
            return OAL_EFAIL;
        }
        wl_cfgvendor_update_start_info(pst_start_info, uc_keep_alive_id, ip_pkt_len, ul_period_msec);

        l_ret += memcpy_s(pst_start_info->auc_ip_pkt_data, ip_pkt_len, puc_ip_pkt, ip_pkt_len);
        l_ret += memcpy_s(pst_start_info->auc_src_mac, WLAN_MAC_ADDR_LEN, auc_src_mac, WLAN_MAC_ADDR_LEN);
        l_ret += memcpy_s(pst_start_info->auc_dst_mac, WLAN_MAC_ADDR_LEN, auc_dst_mac, WLAN_MAC_ADDR_LEN);

        // keepalive的扩展字段放到ip_data的后面
        keepalive_extend = pst_start_info->auc_ip_pkt_data + ip_pkt_len;
        keepalive_extend[0] = (uint8_t)ether_type;
        keepalive_extend[1] = (uint8_t)(ether_type >> NUM_8_BITS);

        oam_warning_log4(0, OAM_SF_ANY, "{wl_start_vowifi_nat_keep_alive::type[%d],id[%d],ip_len[%d],period[%d].}",
            pst_start_info->st_basic_info.en_type, pst_start_info->st_basic_info.uc_keep_alive_id,
            pst_start_info->us_ip_pkt_len, pst_start_info->ul_period_msec);

        wal_send_vowifi_nat_keep_alive_params(pst_wdev->netdev, (oal_uint8 *)pst_start_info, msg_len);
        oal_mem_free_m(pst_start_info, OAL_TRUE);
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wl_start_vowifi_nat_keep_alive::memcpy fail.}");
    }

    if (puc_ip_pkt != OAL_PTR_NULL) {
        oal_mem_free_m(puc_ip_pkt, OAL_TRUE);
    }

    return OAL_SUCC;

err_vowifi_ka_fail:
    if (puc_ip_pkt != OAL_PTR_NULL) {
        oal_mem_free_m(puc_ip_pkt, OAL_TRUE);
        puc_ip_pkt = OAL_PTR_NULL;
    }

    return OAL_EFAIL;
}


OAL_STATIC oal_int32 wl_cfgvendor_stop_vowifi_nat_keep_alive(oal_wiphy_stru *pst_wiphy,
                                                             oal_wireless_dev_stru *pst_wdev,
                                                             OAL_CONST oal_void *p_data,
                                                             oal_int32 l_len)
{
    oal_int32 l_type;
    oal_int32 l_ret;
    oal_int32 l_rem;
    oal_bool_enum_uint8 en_find_keepid_flag = OAL_FALSE;
    OAL_CONST oal_nlattr_stru *pst_iter;
    mac_vowifi_nat_keep_alive_basic_info_stru st_stop_info;
    if ((pst_wdev == NULL) || (p_data == NULL) || (l_len <= 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
            "{wl_cfgvendor_stop_vowifi_nat_keep_alive::invalid para l_len=%d.}", l_len);
        return -OAL_EINVAL;
    }

    memset_s(&st_stop_info, OAL_SIZEOF(mac_vowifi_nat_keep_alive_basic_info_stru), 0,
             OAL_SIZEOF(mac_vowifi_nat_keep_alive_basic_info_stru));

    oal_nla_for_each_attr(pst_iter, p_data, l_len, l_rem)
    {
        l_type = oal_nla_type(pst_iter);
        switch (l_type) {
            case MKEEP_ALIVE_ATTRIBUTE_ID:
                st_stop_info.uc_keep_alive_id = oal_nla_get_u8(pst_iter);
                en_find_keepid_flag = OAL_TRUE;
                break;
            default:
                return OAL_EFAIL;
        }
    }

    if (en_find_keepid_flag == OAL_TRUE) {
        st_stop_info.en_type = VOWIFI_MKEEP_ALIVE_TYPE_STOP;
#if 1 /* 待正式测试完成后，删除 */
        oam_warning_log2(0, OAM_SF_ANY, "{wl_cfgvendor_stop_vowifi_nat_keep_alive::en_type=[%d],id=[%d].}",
                       st_stop_info.en_type, st_stop_info.uc_keep_alive_id);
#endif
        l_ret = wal_send_vowifi_nat_keep_alive_params(pst_wdev->netdev, (oal_uint8 *)&st_stop_info,
                                                      OAL_SIZEOF(mac_vowifi_nat_keep_alive_basic_info_stru));
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

#endif

/*
 * Vendor CFG80211接口获取收/发数据统计
 */
OAL_STATIC oal_int32 wal_cfgvendor_lstats_get_station_info(oal_wiphy_stru *pst_wiphy,
                                                           oal_wireless_dev_stru *pst_wdev,
                                                           wal_wifi_iface_stat_stru *pst_iface_stat)
{
    oal_net_device_stru *pst_dev;
    mac_vap_stru *pst_mac_vap;
    mac_user_stru *pst_mac_user;
    oal_station_info_stru st_sta_info;
    oal_int32 l_ret;

    pst_dev = pst_wdev->netdev;
    if (pst_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_station_info::net device is null!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_station_info::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }

    if (!IS_STA(pst_mac_vap)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_station_info:: not sta mode!}");
        return -OAL_EINVAL;
    }

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id);
    if (pst_mac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_cfgvendor_lstats_get_station_info:: can not get user[%d]!}",
                         pst_mac_vap->uc_assoc_vap_id);
        return -OAL_EINVAL;
    }

    memset_s(&st_sta_info, OAL_SIZEOF(st_sta_info), 0, OAL_SIZEOF(st_sta_info));
    l_ret = wal_cfg80211_get_station(pst_wiphy, pst_dev, pst_mac_user->auc_user_mac_addr, &st_sta_info);
    if (l_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_station_info::get station info fail %d}", l_ret);
        return l_ret;
    }

    pst_iface_stat->ac[0].ul_tx_mpdu = st_sta_info.tx_packets;
    pst_iface_stat->ac[0].ul_rx_mpdu = st_sta_info.rx_packets;
    pst_iface_stat->ac[0].ul_retries = st_sta_info.tx_retries;
    pst_iface_stat->ac[0].ul_mpdu_lost = st_sta_info.tx_failed;

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_cfgvendor_lstats_get_info(oal_wiphy_stru *pst_wiphy,
                                                   oal_wireless_dev_stru *pst_wdev,
                                                   OAL_CONST oal_void *p_data, oal_int32 l_len)
{
    oal_int32 l_err = 0;
    oal_uint32 ul_reply_len;
    oal_void *p_out_data;
    oal_netbuf_stru *pst_skb;
    wal_wifi_radio_stat_stru *pst_radio_stat;
    wal_wifi_iface_stat_stru *pst_iface_stat;

    if ((pst_wiphy == OAL_PTR_NULL) || (pst_wdev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_info:wiphy or wdev or data is null}");
        return -OAL_EFAUL;
    }

    ul_reply_len = OAL_SIZEOF(*pst_radio_stat) + OAL_SIZEOF(*pst_iface_stat);
    p_out_data = (oal_void *)oal_memalloc(ul_reply_len);
    if (p_out_data == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_info:alloc memory fail.[%d]}", ul_reply_len);
        return -OAL_ENOMEM;
    }
    memset_s(p_out_data, ul_reply_len, 0, ul_reply_len);

    /* 获取radio 统计 */
    pst_radio_stat = (wal_wifi_radio_stat_stru *)p_out_data;
    pst_radio_stat->ul_num_channels = VENDOR_NUM_CHAN;
    pst_radio_stat->ul_on_time = oal_jiffies_to_msecs(OAL_TIME_JIFFY -
                                                      g_st_wifi_radio_stat.ull_wifi_on_time_stamp);
    pst_radio_stat->ul_tx_time = 0;
    pst_radio_stat->ul_rx_time = 0;

    /* 获取interfac 统计 */
    pst_iface_stat = (wal_wifi_iface_stat_stru *)(p_out_data + OAL_SIZEOF(*pst_radio_stat));
    pst_iface_stat->ul_num_peers = VENDOR_NUM_PEER;
    pst_iface_stat->peer_info->ul_num_rate = VENDOR_NUM_RATE;

    /* 新增cfg80211 vendor接口支持收发数据统计 */
    l_err = wal_cfgvendor_lstats_get_station_info(pst_wiphy, pst_wdev, pst_iface_stat);
    if (l_err != OAL_SUCC) {
        oal_free(p_out_data);
        return -OAL_EFAIL;
    }

    /* 上报link统计 */
    pst_skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(pst_wiphy, ul_reply_len);
    if (oal_unlikely(!pst_skb)) {
        oal_free(p_out_data);
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_info::skb alloc failed.len %d}", ul_reply_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(pst_skb, ul_reply_len, p_out_data);

    l_err = oal_cfg80211_vendor_cmd_reply(pst_skb);
    oam_warning_log4(0, OAM_SF_ANY, "{wal_cfgvendor_lstats_get_info::on_time %d, tx_pkts %d, rx_pkts %d, err %d",
                     pst_radio_stat->ul_on_time,
                     pst_iface_stat->ac[0].ul_tx_mpdu,
                     pst_iface_stat->ac[0].ul_rx_mpdu,
                     l_err);
    oal_free(p_out_data);
    return l_err;
}

#ifdef _PRE_WLAN_FEATURE_APF

OAL_STATIC oal_int32 wal_cfgvendor_apf_get_capabilities(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev,
    OAL_CONST oal_void *data, oal_int32 len)
{
    oal_netbuf_stru *skb = NULL;
    oal_int32 l_ret, l_mem_needed;

    l_mem_needed = VENDOR_REPLY_OVERHEAD + (ATTRIBUTE_U32_LEN * 2);
    if (oal_any_null_ptr3(wdev, wdev->netdev, wiphy)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_apf_get_capabilities:input params null.}");
        return -OAL_EFAUL;
    }

    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, l_mem_needed);
    if (oal_unlikely(!skb)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_apf_get_capabilities::skb alloc failed, len %d", l_mem_needed);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u32(skb, APF_ATTRIBUTE_VERSION, APF_VERSION);
    oal_nla_put_u32(skb, APF_ATTRIBUTE_MAX_LEN, APF_PROGRAM_MAX_LEN);

    l_ret = oal_cfg80211_vendor_cmd_reply(skb);
    if (oal_unlikely(l_ret)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_apf_get_capabilities::Vendor Command reply failed, ret:%d", l_ret);
    }
    return l_ret;
}


OAL_STATIC oal_int32 wal_cfgvendor_set_apf(
    oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev, OAL_CONST oal_void *data, oal_int32 len)
{
    OAL_CONST oal_nlattr_stru *iter;
    oal_int32 l_ret, l_tmp, l_type;
    mac_apf_filter_cmd_stru st_apf_filter_cmd = { 0 };
    wal_msg_write_stru st_write_msg;
    oal_uint32 ul_program_len;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    if (oal_any_null_ptr2(wdev, wdev->netdev) || (data == NULL) || (len <= 0)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_cfgvendor_apf_set_filter:input params null.}");
        return -OAL_EFAUL;
    }

    st_apf_filter_cmd.us_program_len = 0;

    /* assumption: length attribute must come first */
    oal_nla_for_each_attr(iter, data, len, l_tmp) {
        l_type = oal_nla_type(iter);
        switch (l_type) {
            case APF_ATTRIBUTE_PROGRAM_LEN:
                ul_program_len = oal_nla_get_u32(iter);
                if (oal_unlikely(!ul_program_len || (ul_program_len > APF_PROGRAM_MAX_LEN))) {
                    OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_set_apf::program len[%d] is invalid", ul_program_len);
                    return -OAL_EINVAL;
                }
                st_apf_filter_cmd.us_program_len = (oal_uint16)ul_program_len;
                break;
            case APF_ATTRIBUTE_PROGRAM:
                if (oal_unlikely(!st_apf_filter_cmd.us_program_len)) {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_set_apf::program len not set");
                    return -OAL_EINVAL;
                }

                st_apf_filter_cmd.puc_program = (oal_uint8 *)oal_nla_data(iter);
                st_apf_filter_cmd.en_cmd_type = APF_SET_FILTER_CMD;

                /***************************************************************************
                    抛事件到wal层处理
                ***************************************************************************/
                wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_APF, OAL_SIZEOF(st_apf_filter_cmd));
                if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                             &st_apf_filter_cmd, OAL_SIZEOF(st_apf_filter_cmd)) != EOK) {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_cfgvendor_apf_set_filter::memcpy fail!");
                    return -OAL_EFAIL;
                }

                /* 发送消息 */
                l_ret = wal_send_cfg_event(wdev->netdev, WAL_MSG_TYPE_WRITE,
                                           WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(st_apf_filter_cmd),
                                           (oal_uint8 *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);

                if (oal_unlikely(l_ret != OAL_SUCC)) {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_set_apf::send apf filter fail,err code %d}", l_ret);
                    return l_ret;
                }
                if (wal_check_and_release_msg_resp(pst_rsp_msg) != OAL_SUCC) {
                    oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_set_apf: wal_check_and_release_msg_resp fail}");
                    return -OAL_EFAIL;
                }
                break;
            default:
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_set_apf::no such attribute %d", l_type);
                return -OAL_EINVAL;
        }
    }

    OAM_WARNING_LOG1(0, 0, "wal_cfgvendor_set_apf: program len %d", st_apf_filter_cmd.us_program_len);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
OAL_STATIC oal_bool_enum_uint8 wal_check_chba_conn_notify_params(const oal_nlattr_stru *iter, int32_t attribute)
{
    switch (attribute) {
        case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_20M:
        case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_1:
        case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_2:
            if (oal_nla_len(iter) < (oal_int)sizeof(uint32_t)) {
                oam_error_log2(0, OAM_SF_ANY, "wal_check_chba_conn_notify_params:attribute=%d, nla_len=%d",
                    attribute, oal_nla_len(iter));
                return OAL_FALSE;
            }
            break;
        case MAC_CHBA_CONN_NTF_ATTR_ID:
        case MAC_CHBA_CONN_NTF_ATTR_CENTER_BANDWIDTH:
            if (oal_nla_len(iter) < (oal_int)sizeof(uint8_t)) {
                oam_error_log2(0, OAM_SF_ANY, "wal_check_chba_conn_notify_params:attribute=%d, nla_len=%d",
                    attribute, oal_nla_len(iter));
                return OAL_FALSE;
            }
            break;
        case MAC_CHBA_CONN_NTF_ATTR_EXPIRE_TIME:
            if (oal_nla_len(iter) < (oal_int)sizeof(uint16_t)) {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_check_chba_conn_notify_params:nla_len=%d", oal_nla_len(iter));
                return OAL_FALSE;
            }
            break;
        default:
            break;
    }
    return OAL_TRUE;
}

OAL_STATIC int32_t wal_cfgvendor_fill_chba_conn_notify_msg(wal_msg_write_stru *msg, const void *data, int32_t len)
{
    hmac_chba_conn_notify_stru *ntf_info = (hmac_chba_conn_notify_stru*)msg->auc_value;
    const oal_nlattr_stru *iter = NULL;
    int32_t tmp;
    int32_t attribute;
    oal_nla_for_each_attr(iter, data, len, tmp) {
        attribute = oal_nla_type(iter);
        if (wal_check_chba_conn_notify_params(iter, attribute) == OAL_FALSE) {
            return -OAL_EFAIL;
        }
        switch (attribute) {
            case MAC_CHBA_CONN_NTF_ATTR_ID:
                ntf_info->id = oal_nla_get_u8(iter);
                break;
            case MAC_CHBA_CONN_NTF_ATTR_MAC_ADDR:
                if (memcpy_s(ntf_info->peer_mac_addr, WLAN_MAC_ADDR_LEN, oal_nla_data(iter),
                    oal_nla_len(iter)) != EOK) {
                    OAM_ERROR_LOG0(0, 0, "wal_cfgvendor_chba_conn_notify::mac addr memcpy fail!");
                    return -OAL_EFAIL;
                }
                break;
            case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_20M:
                ntf_info->channel.center_freq_20m = oal_nla_get_u32(iter);
                break;
            case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_1:
                ntf_info->channel.center_freq1 = oal_nla_get_u32(iter);
                break;
            case MAC_CHBA_CONN_NTF_ATTR_CENTER_FREQ_2:
                ntf_info->channel.center_freq2 = oal_nla_get_u32(iter);
                break;
            case MAC_CHBA_CONN_NTF_ATTR_CENTER_BANDWIDTH:
                ntf_info->channel.bandwidth = oal_nla_get_u8(iter);
                break;
            case MAC_CHBA_CONN_NTF_ATTR_EXPIRE_TIME:
                ntf_info->expire_time = oal_nla_get_u16(iter);
                break;
            default:
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_cfgvendor_chba_conn_notify::no such attribute %d", attribute);
                return -OAL_EINVAL;
        }
    }
    oam_warning_log2(0, 0, "CHBA: rcv conn notify. len %d, ntf id %d", len, ntf_info->id);
    return OAL_SUCC;
}
OAL_STATIC int32_t wal_cfgvendor_chba_conn_notify(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, const void *data, int32_t len)

{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    if ((wdev == NULL) || (data == NULL) || (len <= 0)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_chba_conn_notify::invalid para l_len=%d.}", len);
        return -OAL_EINVAL;
    }
    if (wal_cfgvendor_fill_chba_conn_notify_msg(&write_msg, data, len) != OAL_SUCC) {
        return -OAL_EINVAL;
    }
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_CHBA_CONN_NOTIFY, sizeof(hmac_chba_conn_notify_stru));
    ret = wal_send_cfg_event(wdev->netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_chba_conn_notify_stru), (uint8_t*)&write_msg, OAL_FALSE, NULL);
    if (ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_cfgvendor_chba_conn_notify::wal_send_cfg_event fail[%d]!}", ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif
OAL_STATIC OAL_CONST oal_wiphy_vendor_command_stru g_wal_vendor_cmds[] = {
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = GSCAN_SUBCMD_GET_CHANNEL_LIST
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_get_channel_list,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_andr_wifi_attr_policy,
        .maxattr = ANDR_WIFI_ATTRIBUTE_MAX,
#endif
    },

    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_SET_COUNTRY
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_country,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_andr_wifi_attr_policy,
        .maxattr = ANDR_WIFI_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_SUBCMD_GET_FEATURE_SET
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_get_feature_set,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = ANDR_WIFI_RANDOM_MAC_OUI
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_random_mac_oui,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_gscan_attr_policy,
        .maxattr = GSCAN_ATTRIBUTE_MAX - 1,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_FEATURE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_feature,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_VER
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_version,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_RING_STATUS
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_ring_status,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_TRIGGER_MEM_DUMP
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_trigger_mem_dump,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_START_LOGGING
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_start_logging,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = DEBUG_GET_RING_DATA
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_dbg_get_ring_data,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_VOWIFI_NAT))
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_OFFLOAD_SUBCMD_START_MKEEP_ALIVE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wl_cfgvendor_start_vowifi_nat_keep_alive,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_mkeep_alive_attr_policy,
        .maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX,
#endif
    },
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_OFFLOAD_SUBCMD_STOP_MKEEP_ALIVE
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wl_cfgvendor_stop_vowifi_nat_keep_alive,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_mkeep_alive_attr_policy,
        .maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX,
#endif
    },
#endif
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = LSTATS_SUBCMD_GET_INFO
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_lstats_get_info,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
    },
#ifdef _PRE_WLAN_FEATURE_APF
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = APF_SUBCMD_GET_CAPABILITIES
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_apf_get_capabilities,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_apf_attr_policy,
        .maxattr = APF_ATTRIBUTE_MAX,
#endif
    },

    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = APF_SUBCMD_SET_FILTER
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_set_apf,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_apf_attr_policy,
        .maxattr = APF_ATTRIBUTE_MAX,
#endif
    },
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    {   {   .vendor_id = OUI_VENDOR,
            .subcmd = WIFI_SUBCMD_MAC_CHBA_CONN_NOTIFY
        },
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = wal_cfgvendor_chba_conn_notify,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        .policy = wal_chba_conn_ntf_attr_policy,
        .maxattr = MAC_CHBA_CONN_NTF_ATTR_MAX,
#endif
    },
#endif
};

OAL_STATIC OAL_CONST oal_nl80211_vendor_cmd_info_stru g_wal_vendor_events[] = {
    { OUI_HISI, HISI_VENDOR_EVENT_UNSPEC },
    { OUI_HISI, HISI_VENDOR_EVENT_PRIV_STR },
    { OUI_VENDOR, WIFI_SUBCMD_CHBA_READY_TO_CONN }
};

oal_void wal_cfgvendor_init(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands = g_wal_vendor_cmds;
    wiphy->n_vendor_commands = (oal_int32)oal_array_size(g_wal_vendor_cmds);
    wiphy->vendor_events = g_wal_vendor_events;
    wiphy->n_vendor_events = (oal_int32)oal_array_size(g_wal_vendor_events);
}

oal_void wal_cfgvendor_deinit(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands = NULL;
    wiphy->vendor_events = NULL;
    wiphy->n_vendor_commands = 0;
    wiphy->n_vendor_events = 0;
}

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


