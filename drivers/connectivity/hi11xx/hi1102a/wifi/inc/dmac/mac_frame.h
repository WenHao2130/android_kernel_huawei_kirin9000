

#ifndef __MAC_FRAME_H__
#define __MAC_FRAME_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wlan_types.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_IEEE80211_FCTL_FTYPE 0x000c /* 帧类型掩码 */
#define MAC_IEEE80211_FCTL_STYPE 0x00f0 /* 帧子类型掩码 */

#define MAC_IEEE80211_FC0_SUBTYPE_SHIFT 4

#define MAC_IEEE80211_FTYPE_MGMT 0x0000 /* 管理帧 */
#define MAC_IEEE80211_FTYPE_CTL  0x0004 /* 控制帧 */
#define MAC_IEEE80211_FTYPE_DATA 0x0008 /* 数据帧 */

/* A-MSDU情况下，submsdu的偏移宏 */
#define MAC_SUBMSDU_HEADER_LEN    14 /* |da = 6|sa = 6|len = 2| submsdu的头的长度 */
#define MAC_SUBMSDU_LENGTH_OFFSET 12 /* submsdu的长度字段的偏移值 */
#define MAC_SUBMSDU_DA_OFFSET     0  /* submsdu的目的地址的偏移值 */
#define MAC_SUBMSDU_SA_OFFSET     6  /* submsdu的源地址的偏移值 */
#define MAC_MAX_MSDU_ADDR_ALIGN   3  /* msdu首地址4字节对齐最大值 */

#define MAC_80211_FRAME_LEN               24 /* 非四地址情况下，MAC帧头的长度 */
#define MAC_80211_CTL_HEADER_LEN          16 /* 控制帧帧头长度 */
#define MAC_80211_4ADDR_FRAME_LEN         30
#define MAC_80211_QOS_FRAME_LEN           26
#define MAC_80211_QOS_HTC_FRAME_LEN       30
#define MAC_80211_QOS_4ADDR_FRAME_LEN     32
#define MAC_80211_QOS_HTC_4ADDR_FRAME_LEN 36
#define MAC_80211_REASON_CODE_LEN         2

/* 信息元素长度定义 */
#define MAC_IE_HDR_LEN          2   /* 信息元素头部 1字节EID + 1字节长度 */
#define MAC_TIME_STAMP_LEN      8
#define MAC_BEACON_INTERVAL_LEN 2
#define MAC_CAP_INFO_LEN        2
#define MAC_SSID_OFFSET         12
#define MAC_LISTEN_INT_LEN      2
#define MAC_MIN_XRATE_LEN       1
#define MAC_MIN_RATES_LEN       1
#define MAC_MAX_SUPRATES        8 /* WLAN_EID_RATES最大支持8个速率 */
#define MAC_DSPARMS_LEN         1 /* ds parameter set 长度 */
#define MAC_MIN_TIM_LEN         4
#define MAC_DEFAULT_TIM_LEN     4
#define MAC_MIN_RSN_LEN         12
#define MAC_MAX_RSN_LEN         254
#define MAC_MIN_WPA_LEN         16
#define MAC_TIM_LEN_EXCEPT_PVB  3 /* DTIM Period、DTIM Count与BitMap Control三个字段的长度 */
#define MAC_CONTRY_CODE_LEN     3 /* 国家码长度为3 */
#define MAC_MIN_COUNTRY_LEN     6
#define MAC_MAX_COUNTRY_LEN     254
#define MAC_PWR_CONSTRAINT_LEN  1 /* 功率限制ie长度为1 */
#define MAC_QUIET_IE_LEN        6 /* quiet信息元素长度 */
#define MAC_TPCREP_IE_LEN       2
#define MAC_ERP_IE_LEN          1
#define MAC_OBSS_SCAN_IE_LEN    14
#define MAC_MIN_XCAPS_LEN       1
#define MAC_XCAPS_EX_LEN        8  /* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62 长度修改为8 */
#define MAC_WMM_PARAM_LEN       24 /* WMM parameters ie */
#define MAC_WMM_INFO_LEN        7  /* WMM info ie */
#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_WMMAC_INFO_LEN  61 /* WMMAC info ie */
#define MAC_WMMAC_TSPEC_LEN 55 /* TSPEC元素长度 */
#endif
#define MAC_QOS_INFO_LEN                      1
#define MAC_AC_PARAM_LEN                      4
#define MAC_BSS_LOAD_IE_LEN                   5
#define MAC_COUNTRY_REG_FIELD_LEN             3
#define MAC_LIS_INTERVAL_IE_LEN               2   /* listen interval信息元素长度 */
#define MAC_AID_LEN                           2
#define MAC_PWR_CAP_LEN                       2
#define MAC_AUTH_ALG_LEN                      2
#define MAC_AUTH_TRANS_SEQ_NUM_LEN            2   /* transaction seq num信息元素长度 */
#define MAC_STATUS_CODE_LEN                   2
#define MAC_VHT_CAP_IE_LEN                    12
#define MAC_VHT_INFO_IE_LEN                   5
#define MAC_VHT_CAP_INFO_FIELD_LEN            4
#define MAC_TIMEOUT_INTERVAL_INFO_LEN         5
#define MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN      2 /* vht cap ie rx_mcs_map field length */
#define MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN 2 /* vht cap ie rx_highest_data field length */
#define MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN      2 /* vht cap ie tx_mcs_map field length */
#define MAC_VHT_OPERN_LEN                     5 /* vht opern ie length */
#define MAC_VHT_OPERN_INFO_FIELD_LEN          3 /* vht opern ie infomation field length */
#define MAC_2040_COEX_LEN                     1 /* 20/40 BSS Coexistence element */
#define MAC_2040_INTOLCHREPORT_LEN_MIN        1 /* 20/40 BSS Intolerant Channel Report element */
#define MAC_CHANSWITCHANN_LEN                 3 /* Channel Switch Announcement element */
#define MAC_SA_QUERY_LEN                      4 /* SA Query element len */
#define MAC_RSN_VERSION_LEN                   2 /* wpa/RSN version len */
#define MAC_RSN_CIPHER_COUNT_LEN              2 /* RSN IE Cipher count len */
#define MAC_11N_TXBF_CAP_OFFSET               23
#define MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN       3
#define MAC_RRM_ENABLE_CAP_IE_LEN             5
#define MAC_WFA_TPC_RPT_LEN                   7
#define MAC_MIN_WPS_IE_LEN                    5
#define MAC_CHANSWITCHANN_IE_LEN              3
#define MAC_EXT_CHANSWITCHANN_IE_LEN          4
#define MAC_SEC_CH_OFFSET_IE_LEN              1
#define MAC_WIDE_BW_CH_SWITCH_IE_LEN          3
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#define MAC_OPMODE_NOTIFY_LEN       1              /* Operating Mode Notification element len */
#endif

#define MAC_P2P_ATTRIBUTE_HDR_LEN   3 /* P2P_ATTRIBUTE信息元素头部 1字节ATTRIBUTE + 2字节长度 */
#define MAC_P2P_LISTEN_CHN_ATTR_LEN 5 /* LISTEN CHANNEL ATTRIBUTE长度 */
#define MAC_P2P_MIN_IE_LEN          4 /* P2P IE的最小长度 */

#define MAC_ADDBA_REQ_FRAME_BODY_LEN 9   /* addba req帧 帧体长度 */
#define MAC_ADDBA_RSP_FRAME_BODY_LEN 9   /* addba rsp帧 帧体长度 */
#define MAC_ADDBA_DEL_FRAME_BODY_LEN 6   /* addba del帧 帧体长度 */

#define MAC_ADDTS_RSP_FRAME_BODY_LEN 67   /* addts rsp帧 帧体长度 */
#define MAC_DELTS_FRAME_BODY_LEN 67   /* delts 帧 帧体长度 */
#define MAC_ACTION_CATEGORY_AND_CODE_LEN 2

/* Quiet 信息 */
#define MAC_QUIET_PERIOD   0
#define MAC_QUIET_COUNT    MAC_QUIET_PERIOD
#define MAC_QUIET_DURATION 0x0000
#define MAC_QUIET_OFFSET   0x0000

/* RSN信息元素相关定义 */
#define MAC_RSN_IE_VERSION 1
#define MAC_RSN_CAP_LEN    2
#define MAC_PMKID_LEN      16

/* WPA 信息元素相关定义 */
#define MAC_WPA_IE_VERSION          1

/* OUI相关定义 */
#define MAC_OUI_LEN     3
#define MAC_OUITYPE_LEN 1


#define MAC_OUITYPE_WPA 1
#define MAC_OUITYPE_WMM 2
#define MAC_OUITYPE_WPS 4
#define MAC_OUITYPE_WFA 8
#define MAC_OUITYPE_P2P 9

#define MAC_WMM_OUI_BYTE_ONE     0x00
#define MAC_WMM_OUI_BYTE_TWO     0x50
#define MAC_WMM_OUI_BYTE_THREE   0xF2
#define MAC_WMM_UAPSD_ALL        (BIT0 | BIT1 | BIT2 | BIT3)
#define MAC_OUISUBTYPE_WFA       0x00
#define MAC_OUISUBTYPE_WMM_INFO  0
#define MAC_OUISUBTYPE_WMM_PARAM 1
#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_OUISUBTYPE_WMMAC_TSPEC  2   /* WMMAC TSPEC OUI subtype */
#endif
#define MAC_OUISUBTYPE_WMM_PARAM_OFFSET 6 /* wmm 字段中EDCA_INFO位置,表示是否携带EDCA参数 偏移6 */
#define MAC_WMM_QOS_INFO_POS            8 /* wmm 字段中qos info位置，偏移8 */
#define MAC_OUI_WMM_VERSION             1
#define MAC_WMM_IE_LEN                  7  /* wmm ie长度为7 */
#define MAC_HT_CAP_LEN                  26 /* HT能力信息长度为26 */
#define MAC_HT_CAPINFO_LEN              2  /* HT Capabilities Info域长度为2 */
#define MAC_HT_AMPDU_PARAMS_LEN         1  /* A-MPDU parameters域长度为1 */
#define MAC_HT_SUP_MCS_SET_LEN          16 /* Supported MCS Set域长度为16 */
#define MAC_HT_EXT_CAP_LEN              2  /* Extended cap.域长度为2 */
#define MAC_HT_TXBF_CAP_LEN             4  /* Transmit Beamforming Cap.域长度为4 */
#define MAC_HT_ASEL_LEN                 1  /* ASEL Cap.域长度为1 */
#define MAC_HT_OPERN_LEN                22 /* HT Operation信息长度为22 */
#define MAC_HT_BASIC_MCS_SET_LEN        16 /* HT info中的basic mcs set信息的长度 */
#define MAC_HT_CTL_LEN                  4  /* HT CONTROL字段的长度 */
#define MAC_QOS_CTL_LEN                 2  /* QOS CONTROL字段的长度 */
#define MAC_QOS_CTRL_FIELD_OFFSET       24
#define MAC_QOS_CTRL_FIELD_OFFSET_4ADDR 30
#define MAC_EXT_CAP_IE_INTERWORKING_OFFSET   4

#define MAC_TAG_PARAM_OFFSET               (MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN +\
                                            MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)

#define MAC_DEVICE_BEACON_OFFSET            (MAC_TIME_STAMP_LEN +\
                                            MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)
#define MAC_LISTEN_INTERVAL_MAX_LEN         10  /* 定义关联STA最大LISTEN INTERVAL的值 */

#define MAC_MAX_START_SPACING               7

#define MAC_MAX_BSS_INFO_TRANS              5

/* EDCA参数相关的宏 */
#define MAC_WMM_QOS_PARAM_AIFSN_MASK                    0x0F
#define MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET                5
#define MAC_WMM_QOS_PARAM_ACI_MASK                      0x03
#define MAC_WMM_QOS_PARAM_ECWMIN_MASK                   0x0F
#define MAC_WMM_QOS_PARAM_ECWMAX_MASK                   0xF0
#define MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET             4
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK                0x00FF
#define MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE          8
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES 5

/* 关闭WMM后，所有帧进入此宏定义的队列 */
#define MAC_WMM_SWITCH_TID                                 6

/* TCP协议类型，chartiot tcp连接端口号 */
#define MAC_TCP_PROTOCAL       6
#define MAC_UDP_PROTOCAL       17
#define MAC_CHARIOT_NETIF_PORT 10115
#define MAC_WFD_RTSP_PORT      7236

/* hitalk协议压缩目标端口号 */
#define MAC_HITALK_PORT                                    2999

/* ICMP协议报文 */
#define MAC_ICMP_PROTOCAL                                  1

/* huawei申请多个OUI;
   http:// standards-oui.ieee.org/oui.txt */
#define MAC_HUAWEI_VENDER_IE 0xAC853D /* 打桩HW IE */
#define MAC_HISI_HISTREAM_IE 0x11     /* histream IE */

#define MAC_IPV6_UDP_SRC_PORT 546
#define MAC_IPV6_UDP_DES_PORT 547


/* ARP types, 1: ARP request, 2:ARP response, 3:RARP request, 4:RARP response */
#define MAC_ARP_REQUEST   0x0001
#define MAC_ARP_RESPONSE  0x0002
#define MAC_RARP_REQUEST  0x0003
#define MAC_RARP_RESPONSE 0x0004

/* Neighbor Discovery */
#define MAC_ND_RSOL  133 /* Router Solicitation */
#define MAC_ND_RADVT 134 /* Router Advertisement */
#define MAC_ND_NSOL  135 /* Neighbor Solicitation */
#define MAC_ND_NADVT 136 /* Neighbor Advertisement */
#define MAC_ND_RMES  137 /* Redirect Message */

/* DHCP message types */
#define MAC_DHCP_DISCOVER 1
#define MAC_DHCP_OFFER    2
#define MAC_DHCP_REQUEST  3
#define MAC_DHCP_DECLINE  4
#define MAC_DHCP_ACK      5
#define MAC_DHCP_NAK      6
#define MAC_DHCP_RELEASE  7
#define MAC_DHCP_INFORM   8

/* DHCPV6 Message type */
#define MAC_DHCPV6_SOLICIT     1
#define MAC_DHCPV6_ADVERTISE   2
#define MAC_DHCPV6_REQUEST     3
#define MAC_DHCPV6_CONFIRM     4
#define MAC_DHCPV6_RENEW       5
#define MAC_DHCPV6_REBIND      6
#define MAC_DHCPV6_REPLY       7
#define MAC_DHCPV6_RELEASE     8
#define MAC_DHCPV6_DECLINE     9
#define MAC_DHCPV6_RECONFIGURE 10
#define MAC_DHCPV6_INFORM_REQ  11
#define MAC_DHCPV6_RELAY_FORW  12
#define MAC_DHCPV6_RELAY_REPLY 13
#define MAC_DHCPV6_LEASEQUERY  14
#define MAC_DHCPV6_LQ_REPLY    15


/* IGMP record type */
#define MAC_IGMP_QUERY_TYPE    0x11
#define MAC_IGMPV1_REPORT_TYPE 0x12
#define MAC_IGMPV2_REPORT_TYPE 0x16
#define MAC_IGMPV2_LEAVE_TYPE  0x17
#define MAC_IGMPV3_REPORT_TYPE 0x22

/* Is packet type is either leave or report */
#define is_igmp_report_leave_packet(type) (\
    ((type) == MAC_IGMPV1_REPORT_TYPE) ||  \
    ((type) == MAC_IGMPV2_REPORT_TYPE) ||  \
    ((type) == MAC_IGMPV2_LEAVE_TYPE)  ||  \
    ((type) == MAC_IGMPV3_REPORT_TYPE)      \
)

/* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE   1
#define IGMPV3_MODE_IS_EXCLUDE   2
#define IGMPV3_CHANGE_TO_INCLUDE 3
#define IGMPV3_CHANGE_TO_EXCLUDE 4
#define IGMPV3_ALLOW_NEW_SOURCES 5
#define IGMPV3_BLOCK_OLD_SOURCES 6

/* Is packet type is either leave or report */
#define is_igmpv3_mode(type) (\
    ((type) == IGMPV3_MODE_IS_INCLUDE)   ||  \
    ((type) == IGMPV3_MODE_IS_EXCLUDE)   ||  \
    ((type) == IGMPV3_CHANGE_TO_INCLUDE) ||  \
    ((type) == IGMPV3_CHANGE_TO_EXCLUDE) ||  \
    ((type) == IGMPV3_ALLOW_NEW_SOURCES) ||  \
    ((type) == IGMPV3_BLOCK_OLD_SOURCES)    \
)

/* Calculate the group record length */
#define MAC_IS_GOLDEN_AP(puc_bssid)           ((puc_bssid[0] == 0x0) && (puc_bssid[1] == 0x13) && (puc_bssid[2] == 0xE9))

#define MAC_IS_BELKIN_AP(puc_bssid)         ((puc_bssid[0] == 0x14) && (puc_bssid[1] == 0x91) && (puc_bssid[2] == 0x82))
#define MAC_IS_NETGEAR_WNDR_AP(puc_bssid)   ((puc_bssid[0] == 0x08) && (puc_bssid[1] == 0xbd) && (puc_bssid[2] == 0x43))
#define MAC_IS_TRENDNET_AP(puc_bssid)       ((puc_bssid[0] == 0xd8) && (puc_bssid[1] == 0xeb) && (puc_bssid[2] == 0x97))
#define MAC_IS_DLINK_AP(puc_bssid)          ((puc_bssid[0] == 0xcc) && (puc_bssid[1] == 0xb2) && (puc_bssid[2] == 0x55))
#define MAC_IS_TPINK_5640_7300_AP(puc_bssid)((puc_bssid[0] == 0x94) && (puc_bssid[1] == 0xd9) && (puc_bssid[2] == 0xb3))
#define MAC_IS_TPINK_6500_1_AP(puc_bssid)   ((puc_bssid[0] == 0x0c) && (puc_bssid[1] == 0x4b) && (puc_bssid[2] == 0x54))
#define MAC_IS_TPINK_6500_2_AP(puc_bssid)   ((puc_bssid[0] == 0x48) && (puc_bssid[1] == 0x7d) && (puc_bssid[2] == 0x2e))
#define MAC_IS_TPINK_6500_3_AP(puc_bssid)   ((puc_bssid[0] == 0x14) && (puc_bssid[1] == 0x75) && (puc_bssid[2] == 0x90))
#define MAC_IS_TPINK_941N_AP(puc_bssid)     ((puc_bssid[0] == 0x50) && (puc_bssid[1] == 0xfa) && (puc_bssid[2] == 0x84))
#define MAC_IS_TPINK_5510_AP(puc_bssid)     ((puc_bssid[0] == 0x80) && (puc_bssid[1] == 0x89) && (puc_bssid[2] == 0x17))
#define MAC_IS_HIROUTER_H1(puc_bssid)       ((puc_bssid[0] == 0x5C) && (puc_bssid[1] == 0xC3) && (puc_bssid[2] == 0x07))
/* TP-LINK 847N识别:AP OUI + 芯片OUI */
#define MAC_IS_TPLINK_847N(pst_bss_dscr)      ((pst_bss_dscr->auc_bssid[0] == 0x1c) &&\
                                               (pst_bss_dscr->auc_bssid[1] == 0xfa) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0x68) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_ATHEROS))
#define MAC_IS_TPLINK_890N(pst_bss_dscr)      ((pst_bss_dscr->auc_bssid[0] == 0x24) &&\
                                                (pst_bss_dscr->auc_bssid[1] == 0x69) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0x68) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_ATHEROS))
#define MAC_IS_TPLINK_880N(pst_bss_dscr)      ((pst_bss_dscr->auc_bssid[0] == 0xD0) &&\
                                                (pst_bss_dscr->auc_bssid[1] == 0x76) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0xE7) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_ATHEROS))
#define MAC_IS_TPLINK_2041N(pst_bss_dscr)      ((pst_bss_dscr->auc_bssid[0] == 0x50) &&\
                                                (pst_bss_dscr->auc_bssid[1] == 0xFA) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0x84) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_ATHEROS))
#define MAC_IS_TPLINK_H28R(pst_bss_dscr)      ((pst_bss_dscr->auc_bssid[0] == 0x30) &&\
                                                (pst_bss_dscr->auc_bssid[1] == 0xB4) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0x9E) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_SHENZHEN))
#define MAC_IS_NETGEAR_R7800(pst_bss_dscr)    ((pst_bss_dscr->auc_bssid[0] == 0x8c) &&\
                                               (pst_bss_dscr->auc_bssid[1] == 0x3b) &&\
                                                (pst_bss_dscr->auc_bssid[2] == 0xad) &&\
                                                (pst_bss_dscr->en_is_tplink_oui == WLAN_AP_CHIP_OUI_ATHEROS))

#define MAC_IS_360_AP(puc_bssid)    ((puc_bssid[0] == 0xd0) && (puc_bssid[1] == 0xfa) && (puc_bssid[2] == 0x1d))
#define MAC_IS_FEIXUN_K3(puc_bssid) ((puc_bssid[0] == 0x2c) && (puc_bssid[1] == 0xb2) && (puc_bssid[2] == 0x1a))

#define MAC_IS_HAIER_AP(puc_bssid)     ((puc_bssid[0] == 0x08) && (puc_bssid[1] == 0x10) && (puc_bssid[2] == 0x79))
#define MAC_IS_JCG_AP(puc_bssid)       ((puc_bssid[0] == 0x04) && (puc_bssid[1] == 0x5f) && (puc_bssid[2] == 0xa7))
#define MAC_IS_FAST_AP(puc_bssid)      ((puc_bssid[0] == 0x44) && (puc_bssid[1] == 0x97) && (puc_bssid[2] == 0x5a))
#define MAC_IS_TPINK_845_AP(puc_bssid) ((puc_bssid[0] == 0x88) && (puc_bssid[1] == 0x25) && (puc_bssid[2] == 0x93))
#define MAC_IS_XIAOMI_R1D(puc_bssid)   ((puc_bssid[0] == 0x8C) && (puc_bssid[1] == 0xBE) && (puc_bssid[2] == 0xBE))
#define MAC_IS_XIAOMI_R2D(puc_bssid)   ((puc_bssid[0] == 0xf0) && (puc_bssid[1] == 0xb4) && (puc_bssid[2] == 0x29))
#define MAC_IS_FEIXUN_K3(puc_bssid) ((puc_bssid[0] == 0x2c) && (puc_bssid[1] == 0xb2) && (puc_bssid[2] == 0x1a))
#define MAC_IS_APPLE_AP(puc_bssid)     ((puc_bssid[0] == 0xf0) && (puc_bssid[1] == 0x99) && (puc_bssid[2] == 0xbf))
#define MAC_IS_TPLINK_7500(puc_bssid)  ((puc_bssid[0] == 0xbc) && (puc_bssid[1] == 0x46) && (puc_bssid[2] == 0x99))
#define MAC_IS_LINKSYS_1900(puc_bssid) ((puc_bssid[0] == 0x94) && (puc_bssid[1] == 0x10) && (puc_bssid[2] == 0x3e))
#define MAC_IS_TPLINK_885N(puc_bssid)  ((puc_bssid[0] == 0xe4) && (puc_bssid[1] == 0xd3) && (puc_bssid[2] == 0x32))

#define MAC_WLAN_CHIP_OUI_ATHEROSC       0x00037f
#define MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC  0x1
#define MAC_WLAN_CHIP_OUI_RALINK         0x000c43
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK    0x3
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK1   0x7
#define MAC_WLAN_CHIP_OUI_BROADCOM       0x001018
#define MAC_WLAN_CHIP_OUI_TYPE_BROADCOM  0x2
#define MAC_WLAN_CHIP_OUI_SHENZHEN       0x000aeb
#define MAC_WLAN_CHIP_OUI_TYPE_SHENZHEN  0x1
#define MAC_WLAN_CHIP_OUI_APPLE1         0x0017f2
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_1 0x1
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_2 0x7
#define MAC_WLAN_CHIP_OUI_APPLE2         0x000393
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_2_1 0x1
#define MAC_WLAN_CHIP_OUI_360            0x00e04c
#define MAC_WLAN_CHIP_OUI_TYPE_360       0x2
#define MAC_WLAN_CHIP_OUI_ATHEROS        0x00037e
#define MAC_WLAN_CHIP_OUI_TYPE_ATHEROS   0x1
#define MAC_WLAN_CHIP_OUI_REALTEKS       0x00e04c
#define MAC_WLAN_CHIP_OUI_TYPE_REALTEKS  0x2
#define MAC_WLAN_CHIP_OUI_MARVELL        0x005043
#define MAC_WLAN_CHIP_OUI_TYPE_MARVELL   0x3


/* p2p相关 */
/* GO negotiation */
#define P2P_PAF_GON_REQ  0
#define P2P_PAF_GON_RSP  1
#define P2P_PAF_GON_CONF 2
/* Provision discovery */
#define P2P_PAF_PD_REQ      7
/* P2P IE */
#define P2P_OUI_LEN         4
#define P2P_IE_HDR_LEN      6
#define P2P_ELEMENT_ID_SIZE 1
#define P2P_ATTR_ID_SIZE    1
#define P2P_ATTR_HDR_LEN    3
#define WFA_OUI_BYTE1       0x50
#define WFA_OUI_BYTE2       0x6F
#define WFA_OUI_BYTE3       0x9A
#define WFA_P2P_v1_0        0x09
#define WFA_OUI_WORD        (WFA_OUI_BYTE1 | (WFA_OUI_BYTE2<<8) | (WFA_OUI_BYTE3<<16))

#define MAC_VHT_CHANGE (BIT1)
#define MAC_HT_CHANGE  (BIT2)
#define MAC_BW_CHANGE  (BIT3)
#define MAC_NO_CHANGE  (0)

#define MAC_HISI_NB_IE      0x31     /* nb(narrow band) IE */

#define MAC_NEIGHBOR_REPORT_IE_LEN 13 /* NEIGHBOR_REPORT长度 */
#define MAC_IE_EXT_HDR_LEN         3  /* 信息元素头部 1字节EID + 1字节长度 + 1字节EXT_EID */
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_FTM_LEN       9

#define MAC_FTMP_LEN                         9
#define MAC_MEASUREMENT_REQUEST_IE_OFFSET    5
#define MAC_RM_MEASUREMENT_REQUEST_IE_OFFSET 5

#if defined (_PRE_WLAN_FEATURE_FTM) || defined (_PRE_WLAN_FEATURE_LOCATION)
#define MAC_ACTION_CATEGORY_AND_CODE_LEN        2
#endif

#ifdef _PRE_WLAN_FEATURE_LOCATION
#define MAC_HISI_LOCATION_RSSI_IE 0x12
#define MAC_HISI_LOCATION_CSI_IE  0x13
#define MAC_HISI_LOCATION_FTM_IE  0x14

/* Type4Bytes len4Bytes mac16Bytes mac26Bytes timestamp23Bytes RSSIInfo8Bytes SNR2Bytes */
#define MAC_REPORT_RSSIINFO_LEN     8
#define MAC_REPORT_SNR_LEN          2
#define MAC_REPORT_RSSIINFO_SNR_LEN 10
#define MAC_CSI_LOCATION_INFO_LEN   57
#define MAC_FTM_LOCATION_INFO_LEN   52
#define MAC_CSI_REPORT_HEADER_LEN   53
#define MAC_FTM_REPORT_HEADER_LEN   51
#endif

#define MAC_WLAN_OUI_VENDOR_VHT_HEADER   5
#define MAC_WLAN_OUI_BROADCOM_EPIGRAM    0x00904c    /* Broadcom (Epigram) */
#define MAC_WLAN_OUI_VENDOR_VHT_TYPE     0x04
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE  0x08
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2 0x00
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3 0x07

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#define MAC_AMSDU_SKB_LEN_DOWN_LIMIT 1000   /* 超过此长度帧可以AMSDU聚合 */
#define MAC_AMSDU_SKB_LEN_UP_LIMIT   1544
#endif
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    MAC_AP_TYPE_BT_ONLY_ON          = BIT0,
    MAC_AP_TYPE_GOLDENAP            = BIT1,
    MAC_AP_TYPE_DDC_WHITELIST       = BIT2,
    MAC_AP_TYPE_BTCOEX_PS_BLACKLIST = BIT3,
    MAC_AP_TYPE_BTCOEX_DISABLE_CTS  = BIT4,
    MAC_AP_TYPE_R7800               = BIT5,
    MAC_AP_TYPE_ROAM                = BIT6,
    MAC_AP_TYPE_BTCOEX_BA           = BIT7, // 共存场景特殊处理BA
    MAC_AP_TYPE_BUTT
} mac_ap_type_enum;
typedef oal_uint8 mac_ap_type_enum_uint8;

/* HMAC模块接收流程处理MSDU状态 */
typedef enum {
    MAC_PROC_ERROR  = 0,
    MAC_PROC_LAST_MSDU,
    MAC_PROC_MORE_MSDU,
    MAC_PROC_LAST_MSDU_ODD,

    MAC_PROC_BUTT
} mac_msdu_proc_status_enum;
typedef oal_uint8 mac_msdu_proc_status_enum_uint8;

/*****************************************************************************
  枚举名  : wlan_ieee80211_frame_type_enum_uint8
  协议表格:
  枚举说明: 802.11 MAC帧头类型
*****************************************************************************/
typedef enum {
    MAC_IEEE80211_BASE_FRAME = 0,           /* 基础802.11帧类型 */
    MAC_IEEE80211_QOS_FRAME,                /* QoS帧类型 */
    MAC_IEEE80211_QOS_HTC_FRAME,            /* QoS + HTC帧类型 */
    MAC_IEEE80211_ADDR4_FRAME,              /* 四地址帧类型 */
    MAC_IEEE80211_QOS_ADDR4_FRAME,          /* QoS四地址帧类型 */
    MAC_IEEE80211_QOS_HTC_ADDR4_FRAME,      /* QoS + HTC四地址帧类型 */

    MAC_IEEE80211_FRAME_BUTT
} mac_ieee80211_frame_type_enum;
typedef oal_uint8 mac_ieee80211_frame_type_enum_uint8;

/* Action Frames: Category字段枚举 */
typedef enum {
    MAC_ACTION_CATEGORY_SPECMGMT                    = 0,
    MAC_ACTION_CATEGORY_QOS                         = 1,
    MAC_ACTION_CATEGORY_DLS                         = 2,
    MAC_ACTION_CATEGORY_BA                          = 3,
    MAC_ACTION_CATEGORY_PUBLIC                      = 4,
    MAC_ACTION_CATEGORY_RADIO_MEASURMENT            = 5,
    MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION         = 6,
    MAC_ACTION_CATEGORY_HT                          = 7,
    MAC_ACTION_CATEGORY_SA_QUERY                    = 8,
    MAC_ACTION_CATEGORY_PROTECTED_DUAL_OF_ACTION    = 9,
    MAC_ACTION_CATEGORY_WNM                         = 10,
    MAC_ACTION_CATEGORY_MESH                        = 13,
    MAC_ACTION_CATEGORY_MULTIHOP                    = 14,
#ifdef _PRE_WLAN_FEATURE_WMMAC
    MAC_ACTION_CATEGORY_WMMAC_QOS                   = 17,
#endif
    MAC_ACTION_CATEGORY_VHT                         = 21,
    MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED   = 126,
    MAC_ACTION_CATEGORY_VENDOR                      = 127,
} mac_action_category_enum;
typedef oal_uint8 mac_category_enum_uint8;

/* HT Category下的Action值的枚举 */
typedef enum {
    MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH        = 0,
    MAC_HT_ACTION_SMPS                        = 1,
    MAC_HT_ACTION_PSMP                        = 2,
    MAC_HT_ACTION_SET_PCO_PHASE               = 3,
    MAC_HT_ACTION_CSI                         = 4,
    MAC_HT_ACTION_NON_COMPRESSED_BEAMFORMING  = 5,
    MAC_HT_ACTION_COMPRESSED_BEAMFORMING      = 6,
    MAC_HT_ACTION_ASEL_INDICES_FEEDBACK       = 7,

    MAC_HT_ACTION_BUTT
} mac_ht_action_type_enum;
typedef oal_uint8 mac_ht_action_type_enum_uint8;
/* Timeout_Interval ie中的类型枚举 */
typedef enum {
    MAC_TIE_REASSOCIATION_DEADLINE_Interval  = 1,   /* 毫秒级 */
    MAC_TIE_KEY_LIFETIME_Interval            = 2,   /* 秒级 */
    MAC_TIE_ASSOCIATION_COMEBACK_TIME        = 3,   /* 毫秒级 */

    MAC_TIE_BUTT
} mac_Timeout_Interval_type_enum;
typedef oal_uint8 mac_Timeout_Interval_type_enum_uint8;

/* SA QUERY Category下的Action值的枚举 */
typedef enum {
    MAC_SA_QUERY_ACTION_REQUEST          = 0,
    MAC_SA_QUERY_ACTION_RESPONSE         = 1
} mac_sa_query_action_type_enum;
typedef oal_uint8 mac_sa_query_action_type_enum_uint8;
typedef enum {
    MAC_FT_ACTION_REQUEST          = 1,
    MAC_FT_ACTION_RESPONSE         = 2,
    MAC_FT_ACTION_CONFIRM          = 3,
    MAC_FT_ACTION_ACK              = 4,
    MAC_FT_ACTION_BUTT             = 5
} mac_ft_action_type_enum;
typedef oal_uint8 mac_ft_action_type_enum_uint8;

/* VHT Category下的Action值的枚举 */
typedef enum {
    MAC_VHT_ACTION_COMPRESSED_BEAMFORMING   = 0,
    MAC_VHT_ACTION_GROUPID_MANAGEMENT       = 1,
    MAC_VHT_ACTION_OPREATE_MODE_NOTIFY      = 2,

    MAC_VHT_ACTION_BUTT
} mac_vht_action_type_enum;

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* WNM Category下的Action值的枚举 */
typedef enum {
    MAC_WNM_ACTION_EVENT_REQUEST                = 0,
    MAC_WNM_ACTION_EVENT_REPORT                 = 1,
    MAC_WNM_ACTION_DIALGNOSTIC_REQUEST          = 2,
    MAC_WNM_ACTION_DIALGNOSTIC_REPORT           = 3,
    MAC_WNM_ACTION_LOCATION_CONF_REQUEST        = 4,
    MAC_WNM_ACTION_LOCATION_CONF_RESPONSE       = 5,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_QUERY    = 6,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST  = 7,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_RESPONSE = 8,
    MAC_WNM_ACTION_FMS_REQUEST                  = 9,
    MAC_WNM_ACTION_FMS_RESPONSE                 = 10,
    MAC_WNM_ACTION_COLLOCATED_INTER_REQUEST     = 11,
    MAC_WNM_ACTION_COLLOCATEC_INTER_REPORT      = 12,
    MAC_WNM_ACTION_TFS_REQUEST                  = 13,
    MAC_WNM_ACTION_TFS_RESPONSE                 = 14,
    MAC_WNM_ACTION_TFS_NOTIFY                   = 15,
    MAC_WNM_ACTION_SLEEP_MODE_REQUEST           = 16,
    MAC_WNM_ACTION_SLEEP_MODE_RESPONSE          = 17,
    MAC_WNM_ACTION_TIM_BROADCAST_REQUEST        = 18,
    MAC_WNM_ACTION_TIM_BROADCAST_RESPONSE       = 19,
    MAC_WNM_ACTION_QOS_TRAFFIC_CAP_UPDATE       = 20,
    MAC_WNM_ACTION_CHANNEL_USAGE_REQUEST        = 21,
    MAC_WNM_ACTION_CHANNEL_USAGE_RESPONSE       = 22,
    MAC_WNM_ACTION_DMS_REQUEST                  = 23,
    MAC_WNM_ACTION_DMS_RESPONSE                 = 24,
    MAC_WNM_ACTION_TIMING_MEAS_REQUEST          = 25,
    MAC_WNM_ACTION_NOTIFICATION_REQUEST         = 26,
    MAC_WNM_ACTION_NOTIFICATION_RESPONSE        = 27,

    MAC_WNM_ACTION_BUTT
} mac_wnm_action_type_enum;
typedef oal_uint8 mac_wnm_action_type_enum_uint8;
#endif

typedef oal_uint8 mac_vht_action_type_enum_uint8;

/* 校准模式的枚举 */
typedef enum {
    MAC_NOT_SURPPORT_CLB = 0,
    MAC_RSP_CLB_ONLY     = 1,
    MAC_SUPPOTR_CLB      = 3,
    MAC_CLB_BUTT
} mac_txbf_clb_enum;
typedef oal_uint8 mac_txbf_clb_enum_uint8;

/* Spectrum Management Category下的Action枚举值 */
typedef enum {
    MAC_SPEC_MEASURE_REQUEST    = 0,
    MAC_SPEC_MEASURE_REPORT     = 1,
    MAC_SPEC_TPC_REQUEST        = 2,
    MAC_SPEC_TPC_REPORT         = 3,
    MAC_SPEC_CH_SWITCH_ANNOUNCE = 4   /* Channel Switch Announcement */
} mac_specmgmt_action_type_enum;
typedef oal_uint8 mac_specmgmt_action_type_enum_uint8;


/* BlockAck Category下的Action值的枚举 */
typedef enum {
    MAC_BA_ACTION_ADDBA_REQ       = 0,
    MAC_BA_ACTION_ADDBA_RSP       = 1,
    MAC_BA_ACTION_DELBA           = 2,

    MAC_BA_ACTION_BUTT
} mac_ba_action_type_enum;
typedef oal_uint8 mac_ba_action_type_enum_uint8;

/* Public Category下的Action枚举值 */
typedef enum {
    MAC_PUB_COEXT_MGMT = 0,  /* 20/40 BSS Coexistence Management */
    MAC_PUB_EX_CH_SWITCH_ANNOUNCE = 4,   /* Extended Channel Switch Announcement */
    MAC_PUB_VENDOR_SPECIFIC = 9,
    MAC_PUB_GAS_INIT_REQ = 10,
    MAC_PUB_GAS_INIT_RESP = 11, /* public Action: GAS Initial Response(0x0b) */
    MAC_PUB_GAS_COMBAK_REQ = 12,
    MAC_PUB_GAS_COMBAK_RESP = 13, /* public Action: GAS Comeback Response(0x0d) */
#ifdef _PRE_WLAN_FEATURE_FTM
    MAC_PUB_FTM_REQ = 32,
    MAC_PUB_FTM  = 33
#endif
} mac_public_action_type_enum;
typedef oal_uint8 mac_public_action_type_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* WMMAC中TSPEC相关ACTION的枚举值 */
typedef enum {
    MAC_WMMAC_ACTION_ADDTS_REQ       = 0,
    MAC_WMMAC_ACTION_ADDTS_RSP       = 1,
    MAC_WMMAC_ACTION_DELTS           = 2,

    MAC_WMMAC_ACTION_BUTT
} mac_wmmac_action_type_enum;

/* ADDTS REQ中TSPEC Direction元素的枚举值 */
typedef enum {
    MAC_WMMAC_DIRECTION_UPLINK         = 0,
    MAC_WMMAC_DIRECTION_DOWNLINK       = 1,
    MAC_WMMAC_DIRECTION_RESERVED       = 2,
    MAC_WMMAC_DIRECTION_BIDIRECTIONAL  = 3,

    MAC_WMMAC_DIRECTION_BUTT
} mac_wmmac_direction_enum;
typedef oal_uint8 mac_wmmac_direction_enum_uint8;
/* TS建立的状态枚举 */
typedef enum {
    MAC_TS_NONE        = 0,    /* TS不需要建立 */
    MAC_TS_INIT,               /* TS需要建立，未建立 */
    MAC_TS_INPROGRESS,         /* TS建立过程中 */
    MAC_TS_SUCCESS,            /* TS建立成功 */

    MAC_TS_BUTT
} mac_ts_conn_status_enum;
typedef oal_uint8 mac_ts_conn_status_enum_uint8;
#endif

/* 802.11n下的私有请求 */
typedef enum {
    MAC_A_MPDU_START = 0,
    MAC_A_MPDU_END   = 1,

    MAC_A_MPDU_BUTT
} mac_priv_req_11n_enum;
typedef oal_uint8 mac_priv_req_11n_enum_uint8;

/* Block ack的确认类型 */
typedef enum {
    MAC_BACK_BASIC         = 0,
    MAC_BACK_COMPRESSED    = 2,
    MAC_BACK_MULTI_TID     = 3,

    MAC_BACK_BUTT
} mac_back_variant_enum;
typedef oal_uint8 mac_back_variant_enum_uint8;

/* ACTION帧中，各个域的偏移量 */
typedef enum {
    MAC_ACTION_OFFSET_CATEGORY     = 0,
    MAC_ACTION_OFFSET_ACTION       = 1,
} mac_action_offset_enum;
typedef oal_uint8 mac_action_offset_enum_uint8;


/* Reason Codes for Deauthentication and Disassociation Frames */
typedef enum {
    MAC_UNSPEC_REASON           = 1,
    MAC_AUTH_NOT_VALID          = 2,
    MAC_DEAUTH_LV_SS            = 3,
    MAC_INACTIVITY              = 4,
    MAC_AP_OVERLOAD             = 5,
    MAC_NOT_AUTHED              = 6,
    MAC_NOT_ASSOCED             = 7,
    MAC_DISAS_LV_SS             = 8,
    MAC_ASOC_NOT_AUTH           = 9,
    MAC_INVLD_ELEMENT           = 13,
    MAC_MIC_FAIL                = 14,
    MAC_4WAY_HANDSHAKE_TIMEOUT  = 15,
    MAC_IEEE_802_1X_AUTH_FAIL   = 23,
    MAC_UNSPEC_QOS_REASON       = 32,
    MAC_QAP_INSUFF_BANDWIDTH    = 33,
    MAC_POOR_CHANNEL            = 34,
    MAC_STA_TX_AFTER_TXOP       = 35,
    MAC_QSTA_LEAVING_NETWORK    = 36,
    MAC_QSTA_INVALID_MECHANISM  = 37,
    MAC_QSTA_SETUP_NOT_DONE     = 38,
    MAC_QSTA_TIMEOUT            = 39,
    MAC_QSTA_CIPHER_NOT_SUPP    = 45
} mac_reason_code_enum;
typedef oal_uint16 mac_reason_code_enum_uint16;


/* Capability Information field bit assignments */
typedef enum {
    MAC_CAP_ESS             = 0x01,   /* ESS capability */
    MAC_CAP_IBSS            = 0x02,   /* IBSS mode */
    MAC_CAP_POLLABLE        = 0x04,   /* CF Pollable */
    MAC_CAP_POLL_REQ        = 0x08,   /* Request to be polled */
    MAC_CAP_PRIVACY         = 0x10,   /* WEP encryption supported */
    MAC_CAP_SHORT_PREAMBLE  = 0x20,   /* Short Preamble is supported */
    MAC_CAP_SHORT_SLOT      = 0x400,  /* Short Slot is supported */
    MAC_CAP_PBCC            = 0x40,   /* PBCC */
    MAC_CAP_CHANNEL_AGILITY = 0x80,   /* Channel Agility */
    MAC_CAP_SPECTRUM_MGMT   = 0x100,  /* Spectrum Management */
    MAC_CAP_DSSS_OFDM       = 0x2000  /* DSSS-OFDM */
} mac_capability_enum;
typedef oal_uint16 mac_capability_enum_uint16;


/* Status Codes for Authentication and Association Frames */
typedef enum {
    MAC_SUCCESSFUL_STATUSCODE       = 0,
    MAC_UNSPEC_FAIL                 = 1,
    MAC_UNSUP_CAP                   = 10,
    MAC_REASOC_NO_ASOC              = 11,
    MAC_FAIL_OTHER                  = 12,
    MAC_UNSUPT_ALG                  = 13,
    MAC_AUTH_SEQ_FAIL               = 14,
    MAC_CHLNG_FAIL                  = 15,
    MAC_AUTH_TIMEOUT                = 16,
    MAC_AP_FULL                     = 17,
    MAC_UNSUP_RATE                  = 18,
    MAC_SHORT_PREAMBLE_UNSUP        = 19,
    MAC_PBCC_UNSUP                  = 20,
    MAC_CHANNEL_AGIL_UNSUP          = 21,
    MAC_MISMATCH_SPEC_MGMT          = 22,
    MAC_MISMATCH_POW_CAP            = 23,
    MAC_MISMATCH_SUPP_CHNL          = 24,
    MAC_SHORT_SLOT_UNSUP            = 25,
    MAC_OFDM_DSSS_UNSUP             = 26,
    MAC_MISMATCH_HTCAP              = 27,
    MAC_MISMATCH_PCO                = 29,
    MAC_REJECT_TEMP                 = 30,
    MAC_MFP_VIOLATION               = 31,
    MAC_UNSPEC_QOS_FAIL             = 32,
    MAC_QAP_INSUFF_BANDWIDTH_FAIL   = 33,
    MAC_POOR_CHANNEL_FAIL           = 34,
    MAC_REMOTE_STA_NOT_QOS          = 35,
    MAC_REQ_DECLINED                = 37,
    MAC_INVALID_REQ_PARAMS          = 38,
    MAC_RETRY_NEW_TSPEC             = 39,
    MAC_INVALID_INFO_ELMNT          = 40,
    MAC_INVALID_GRP_CIPHER          = 41,
    MAC_INVALID_PW_CIPHER           = 42,
    MAC_INVALID_AKMP_CIPHER         = 43,
    MAC_UNSUP_RSN_INFO_VER          = 44,
    MAC_INVALID_RSN_INFO_CAP        = 45,
    MAC_CIPHER_REJ                  = 46,
    MAC_RETRY_TS_LATER              = 47,
    MAC_DLS_NOT_SUPP                = 48,
    MAC_DST_STA_NOT_IN_QBSS         = 49,
    MAC_DST_STA_NOT_QSTA            = 50,
    MAC_LARGE_LISTEN_INT            = 51,
    MAC_INVALID_PMKID               = 53,
    MAC_INVALID_MDE                 = 54,
    MAC_INVALID_FTE                 = 55,
    MAC_ANTI_CLOGGING               = 76,
    MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED = 77,
    MAC_MISMATCH_VHTCAP             = 104,
    MAC_SAE_HASH_TO_ELEMENT         = 126,
    /* 私有的定义 */
    MAC_JOIN_RSP_TIMEOUT            = 5200,
    MAC_AUTH_RSP2_TIMEOUT           = 5201,
    MAC_AUTH_RSP4_TIMEOUT           = 5202,
    MAC_ASOC_RSP_TIMEOUT            = 5203,
    MAC_AUTH_REQ_SEND_FAIL_BEGIN    = 5250,
    MAC_AUTH_REQ_SEND_FAIL_NO_ACK   = 5251,  /* auth req发出去没收到ack */
    MAC_AUTH_REQ_SEND_FAIL_TIMEOUT  = 5252,  /* auth req发出去lifetime超时（没法送出去） */
    MAC_AUTH_REQ_SEND_FAIL_ABORT    = 5261,  /* auth req发送失败(因为蓝牙abort) */

    MAC_ASOC_REQ_SEND_FAIL_BEGIN    = 5300,
    MAC_ASOC_REQ_SEND_FAIL_NO_ACK   = 5301,  /* asoc req发出去没收到ack */
    MAC_ASOC_REQ_SEND_FAIL_TIMEOUT  = 5302,  /* asoc req发出去lifetime超时（没法送出去） */
    MAC_ASOC_REQ_SEND_FAIL_ABORT    = 5311,  /* asoc req发送失败(因为蓝牙abort) */

    MAC_AP_AUTH_RSP_TIMEOUT         = 6100,  /* softap auth rsp发送完wait asoc req超时 */
#ifdef _PRE_WLAN_CHBA_MGMT
    /* CHBA: 添加私有code */
    MAC_CHBA_INIT_CODE = 6200,
    MAC_CHBA_INVAILD_CONNECT_CMD = 6201,
    MAC_CHBA_REPEAT_CONNECT_CMD = 6202, /* 重复建链命令，且两条建链命令参数不匹配 */
    MAC_CHBA_CREATE_NEW_USER_FAIL = 6203, /* 创建新用户失败，一般是因为超过芯片规格 */
    MAC_CHBA_UNSUP_ASSOC_CHANNEL = 6204, /* 无法在下发的建链信道上完成建链 */
    MAC_CHBA_COEX_FAIL = 6205, /* 不满足vap共存条件 */
    MAC_CHBA_UNSUP_PARALLEL_CONNECT = 6206, /* 暂不支持并行建链 */
#endif
} mac_status_code_enum;
typedef oal_uint16 mac_status_code_enum_uint16;


/* BA会话管理确认策略 */
typedef enum {
    MAC_BA_POLICY_DELAYED = 0,
    MAC_BA_POLICY_IMMEDIATE,

    MAC_BA_POLICY_BUTT
} mac_ba_policy_enum;
typedef oal_uint8 mac_ba_policy_enum_uint8;

/* 发起DELBA帧的端点的枚举 */
typedef enum {
    MAC_RECIPIENT_DELBA     = 0,   /* 数据的接收端 */
    MAC_ORIGINATOR_DELBA,          /* 数据的发起端 */

    MAC_BUTT_DELBA
} dmac_delba_initiator_enum;
typedef oal_uint8 mac_delba_initiator_enum_uint8;

/*****************************************************************************
  信息元素(Infomation Element)的Element ID
  协议521页，Table 8-54—Element IDs
*****************************************************************************/
typedef enum {
    MAC_EID_SSID                   = 0,
    MAC_EID_RATES                  = 1,
    MAC_EID_FHPARMS                = 2,
    MAC_EID_DSPARMS                = 3,
    MAC_EID_CFPARMS                = 4,
    MAC_EID_TIM                    = 5,
    MAC_EID_IBSSPARMS              = 6,
    MAC_EID_COUNTRY                = 7,
    MAC_EID_REQINFO                = 10,
    MAC_EID_QBSS_LOAD              = 11,
    MAC_EID_TSPEC                  = 13,
    MAC_EID_TCLAS                  = 14,
    MAC_EID_CHALLENGE              = 16,
    /* 17-31 reserved */
    MAC_EID_PWRCNSTR               = 32,
    MAC_EID_PWRCAP                 = 33,
    MAC_EID_TPCREQ                 = 34,
    MAC_EID_TPCREP                 = 35,
    MAC_EID_SUPPCHAN               = 36,
    MAC_EID_CHANSWITCHANN          = 37,   /* Channel Switch Announcement IE */
    MAC_EID_MEASREQ                = 38,
    MAC_EID_MEASREP                = 39,
    MAC_EID_QUIET                  = 40,
    MAC_EID_IBSSDFS                = 41,
    MAC_EID_ERP                    = 42,
    MAC_EID_TCLAS_PROCESS          = 44,
    MAC_EID_HT_CAP                 = 45,
    MAC_EID_QOS_CAP                = 46,
    MAC_EID_RESERVED_47            = 47,
    MAC_EID_RSN                    = 48,
    MAC_EID_RESERVED_49            = 49,
    MAC_EID_XRATES                 = 50,
    MAC_EID_AP_CHAN_REPORT         = 51,
    MAC_EID_NEIGHBOR_REPORT        = 52,
    MAC_EID_MOBILITY_DOMAIN        = 54,
    MAC_EID_FT                     = 55,
    MAC_EID_TIMEOUT_INTERVAL       = 56,
    MAC_EID_RDE                    = 57,
    MAC_EID_OPERATING_CLASS        = 59,    /* Supported Operating Classes */
    MAC_EID_EXTCHANSWITCHANN       = 60,   /* Extended Channel Switch Announcement IE */
    MAC_EID_HT_OPERATION           = 61,
    MAC_EID_SEC_CH_OFFSET          = 62,   /* Secondary Channel Offset IE */
    MAC_EID_WAPI                   = 68,   /* IE for WAPI */
    MAC_EID_TIME_ADVERTISEMENT     = 69,
    MAC_EID_RRM                    = 70,   /* Radio resource measurement */
    MAC_EID_2040_COEXT             = 72,   /* 20/40 BSS Coexistence IE */
    MAC_EID_2040_INTOLCHREPORT     = 73,   /* 20/40 BSS Intolerant Channel Report IE */
    MAC_EID_OBSS_SCAN              = 74,   /* Overlapping BSS Scan Parameters IE */
    MAC_EID_MMIE                   = 76,   /* 802.11w Management MIC IE */
    MAC_EID_FMS_DESCRIPTOR         = 86,   /* 802.11v FMS descriptor IE */
    MAC_EID_FMS_REQUEST            = 87,   /* 802.11v FMS request IE */
    MAC_EID_FMS_RESPONSE           = 88,   /* 802.11v FMS response IE */
    MAC_EID_BSSMAX_IDLE_PERIOD     = 90,   /* BSS MAX IDLE PERIOD */
    MAC_EID_TFS_REQUEST            = 91,
    MAC_EID_TFS_RESPONSE           = 92,
    MAC_EID_TIM_BCAST_REQUEST      = 94,
    MAC_EID_TIM_BCAST_RESPONSE     = 95,
    MAC_EID_INTERWORKING           = 107,
    MAC_EID_ADVERTISEMENT_PROTOCOL = 108,
    MAC_EID_EXT_CAPS               = 127,  /* Extended Capabilities IE */
    MAC_EID_VHT_TYPE               = 129,
    MAC_EID_11NTXBF                = 130,   /* 802.11n txbf */
    MAC_EID_RESERVED_133           = 133,
    MAC_EID_TPC                    = 150,
    MAC_EID_CCKM                   = 156,
    MAC_EID_VHT_CAP                = 191,
    MAC_EID_VHT_OPERN              = 192,  /* VHT Operation IE */
    MAC_EID_WIDE_BW_CH_SWITCH      = 194,  /* Wide Bandwidth Channel Switch IE */
    MAC_EID_OPMODE_NOTIFY          = 199,  /* Operating Mode Notification IE */
#ifdef _PRE_WLAN_FEATURE_FTM
    MAC_EID_FTMP                   = 206,
    MAC_EID_FTMSI                  = 255,
#endif
    MAC_EID_VENDOR                 = 221,  /* vendor private */
    MAC_EID_WMM                    = 221,
    MAC_EID_WPA                    = 221,
    MAC_EID_WPS                    = 221,
    MAC_EID_P2P                    = 221,
    MAC_EID_WFA_TPC_RPT            = 221,
    MAC_EID_RSNX                   = 244,
} mac_eid_enum;
typedef oal_uint8 mac_eid_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum {
    MAC_EID_EXT_FTMSI              = 9,
} mac_eid_extension_enum;
typedef oal_uint8 mac_eid_extension_enum_uint8;
#endif

typedef enum {
    MAC_P2P_ATTRIBUTE_CAP          = 2,
    MAC_P2P_ATTRIBUTE_GROUP_OI     = 4,
    MAC_P2P_ATTRIBUTE_CFG_TIMEOUT  = 5,
    MAC_P2P_ATTRIBUTE_LISTEN_CHAN  = 6,
} mac_p2p_attribute_enum;
typedef oal_uint8 mac_p2p_attribute_enum_uint8;

typedef enum {
    MAC_SMPS_STATIC_MODE     = 0,   /* 静态SMPS */
    MAC_SMPS_DYNAMIC_MODE    = 1,   /* 动态SMPS */
    MAC_SMPS_MIMO_MODE       = 3,   /* disable SMPS */

    MAC_SMPS_MODE_BUTT
} mac_mimo_power_save_enum;
typedef oal_uint8 mac_mimo_power_save_mode_enum_uint8;

typedef enum {
    MAC_SCN = 0,   /* 不存在次信道 */
    MAC_SCA = 1,   /* 次信道在主信道之上(Secondary Channel Above) */
    MAC_SCB = 3,   /* 次信道在主信道之下(Secondary Channel Below) */

    MAC_SEC_CH_BUTT,
} mac_sec_ch_off_enum;
typedef oal_uint8 mac_sec_ch_off_enum_uint8;

/* P2P相关 */
typedef enum {
    P2P_STATUS             =  0,
    P2P_MINOR_REASON_CODE  =  1,
    P2P_CAPABILITY         =  2,
    P2P_DEVICE_ID          =  3,
    GROUP_OWNER_INTENT     =  4,
    CONFIG_TIMEOUT         =  5,
    LISTEN_CHANNEL         =  6,
    P2P_GROUP_BSSID        =  7,
    EXTENDED_LISTEN_TIMING =  8,
    INTENDED_P2P_IF_ADDR   =  9,
    P2P_MANAGEABILITY      =  10,
    P2P_CHANNEL_LIST       =  11,
    NOTICE_OF_ABSENCE      =  12,
    P2P_DEVICE_INFO        =  13,
    P2P_GROUP_INFO         =  14,
    P2P_GROUP_ID           =  15,
    P2P_INTERFACE          =  16,
    P2P_OPERATING_CHANNEL  =  17,
    INVITATION_FLAGS       =  18
} ATTRIBUTE_ID_T;

typedef enum {
    P2P_PUB_ACT_OUI_OFF1         = 2,
    P2P_PUB_ACT_OUI_OFF2         = 3,
    P2P_PUB_ACT_OUI_OFF3         = 4,
    P2P_PUB_ACT_OUI_TYPE_OFF     = 5,
    P2P_PUB_ACT_OUI_SUBTYPE_OFF  = 6,
    P2P_PUB_ACT_DIALOG_TOKEN_OFF = 7,
    P2P_PUB_ACT_TAG_PARAM_OFF    = 8
} P2P_PUB_ACT_FRM_OFF;

typedef enum {
    P2P_GEN_ACT_OUI_OFF1         = 1,
    P2P_GEN_ACT_OUI_OFF2         = 2,
    P2P_GEN_ACT_OUI_OFF3         = 3,
    P2P_GEN_ACT_OUI_TYPE_OFF     = 4,
    P2P_GEN_ACT_OUI_SUBTYPE_OFF  = 5,
    P2P_GEN_ACT_DIALOG_TOKEN_OFF = 6,
    P2P_GEN_ACT_TAG_PARAM_OFF    = 7
} P2P_GEN_ACT_FRM_OFF;

typedef enum {
    P2P_NOA           = 0,
    P2P_PRESENCE_REQ  = 1,
    P2P_PRESENCE_RESP = 2,
    GO_DISC_REQ       = 3
} P2P_GEN_ACTION_FRM_TYPE;

typedef enum {
    P2P_STAT_SUCCESS           = 0,
    P2P_STAT_INFO_UNAVAIL      = 1,
    P2P_STAT_INCOMP_PARAM      = 2,
    P2P_STAT_LMT_REACHED       = 3,
    P2P_STAT_INVAL_PARAM       = 4,
    P2P_STAT_UNABLE_ACCO_REQ   = 5,
    P2P_STAT_PREV_PROT_ERROR   = 6,
    P2P_STAT_NO_COMMON_CHAN    = 7,
    P2P_STAT_UNKNW_P2P_GRP     = 8,
    P2P_STAT_GO_INTENT_15      = 9,
    P2P_STAT_INCOMP_PROV_ERROR = 10,
    P2P_STAT_USER_REJECTED     = 11
} P2P_STATUS_CODE_T;

/* Radio Measurement下的Action枚举值 */
typedef enum {
    MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST     = 0,
    MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT      = 1,
    MAC_RM_ACTION_LINK_MEASUREMENT_REQUEST      = 2,
    MAC_RM_ACTION_LINK_MEASUREMENT_REPORT       = 3,
    MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST       = 4,
    MAC_RM_ACTION_NEIGHBOR_REPORT_RESPONSE      = 5
} mac_rm_action_type_enum;
typedef oal_uint8 mac_rm_action_type_enum_uint8;

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
/* 度量类型的枚举 */
/* Basic 0
Clear Channel Assessment (CCA) 1
Receive Power Indication (RPI) Histogram 2
Channel Load 3
Noise Histogram 4
Beacon 5
Frame 6
STA Statistics 7
LCI 8
Transmit Stream/Category Measurement 9
Multicast Diagnostics 10
Location Civic 11
Location Identifier 12
Directional Channel Quality 13
Directional Measurement 14
Directional Statistics 15
Fine Timing Measurement Range 16
Reserved 17–254
Measurement Pause 255 */
typedef enum {
    RM_RADIO_MEASUREMENT_BASIC              = 0,
    RM_RADIO_MEAS_CHANNEL_LOAD              = 3,
    RM_RADIO_MEAS_BCN                       = 5,
    RM_RADIO_MEAS_FRM                       = 6,
    RM_RADIO_MEAS_STA_STATS                 = 7,
    RM_RADIO_MEASUREMENT_LCI                = 8,
    RM_RADIO_MEAS_TSC                       = 9,
    RM_RADIO_MEASUREMENT_LOCATION_CIVIC     = 11,
    RM_RADIO_MEASUREMENT_FTM_RANGE          = 16,
    RM_RADIO_MEAS_PAUSE                     = 255
} mac_radio_meas_type_enum;
typedef oal_uint8 mac_radio_meas_type_enum_uint8;

typedef enum {
    MAC_QUIET_STATE_INIT    = 0,
    MAC_QUIET_STATE_IDLE    = 1,
    MAC_QUIET_STATE_START   = 2,
    MAC_QUIET_STATE_COUNT   = 3,
    MAC_QUIET_STATE_OFFSET  = 4,
    MAC_QUIET_STATE_PRD_DEC = 5,
    MAC_QUIET_STATE_QUIET   = 6,
    MAC_QUIET_STATE_BUTT    = 7
} oal_mac_quiet_state_enum;
typedef oal_uint8 oal_mac_quiet_state_uint8;

typedef enum {
    PHY_TYPE_FHSS           = 1,
    PHY_TYPE_DSSS           = 2,
    PHY_TYPE_IR_BASEBAND    = 3,
    PHY_TYPE_OFDM           = 4,
    PHY_TYPE_HRDSSS         = 5,
    PHY_TYPE_BUTT
} phy_type_enum;
typedef oal_uint8 phy_type_enum_uint8;

typedef enum {
    RM_BCN_REQ_MEAS_MODE_PASSIVE = 0,
    RM_BCN_REQ_MEAS_MODE_ACTIVE  = 1,
    RM_BCN_REQ_MEAS_MODE_TABLE   = 2,
    RM_BCN_REQ_MEAS_MODE_BUTT
} rm_bcn_req_meas_mode_enum;
typedef oal_uint8 rm_bcn_req_meas_mode_enum_uint8;
#endif

/* EAPOL数据帧子类型枚举定义 */
typedef enum {
    MAC_EAPOL_PTK_1_4  = 1,
    MAC_EAPOL_PTK_2_4,
    MAC_EAPOL_PTK_3_4,
    MAC_EAPOL_PTK_4_4,

    MAC_EAPOL_PTK_BUTT
} mac_eapol_type_enum;

typedef oal_uint8  mac_eapol_type_enum_uint8;

#define MAC_WLAN_OUI_WFA                0x506f9a
#define MAC_WLAN_OUI_TYPE_WFA_P2P       9
#define MAC_WLAN_OUI_TYPE_WFA_WFD       0x0a
#define MAC_WLAN_OUI_MICROSOFT          0x0050f2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPA 1
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WMM 2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPS 4

/* eapol key 结构宏定义 */
#define WPA_REPLAY_COUNTER_LEN    8
#define WPA_NONCE_LEN             32
#define WPA_KEY_RSC_LEN           8
#define IEEE802_1X_TYPE_EAPOL_KEY 3
#define WPA_KEY_INFO_KEY_TYPE     BIT(3) /* 1 = Pairwise, 0 = Group key */

#define WPA_KEY_INFO_KEY_ACK       BIT(7)

#define WPA_KEY_INFO_KEY_MIC       BIT(0)

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_uint32 g_customize_interworking;
/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/* RSNA OUI 定义 */
extern OAL_CONST oal_uint8 g_auc_rsn_oui[MAC_OUI_LEN];

/* WPA OUI 定义 */
extern OAL_CONST oal_uint8 g_auc_wpa_oui[MAC_OUI_LEN];

/* WMM OUI定义 */
extern OAL_CONST oal_uint8 g_auc_wmm_oui[MAC_OUI_LEN];

/* WPS OUI 定义 */
extern OAL_CONST oal_uint8 g_auc_wps_oui[MAC_OUI_LEN];

/* P2P OUI 定义 */
extern OAL_CONST oal_uint8 g_auc_p2p_oui[MAC_OUI_LEN];

/* 窄带 OUI 定义 */
extern  oal_uint8 g_auc_huawei_oui[MAC_OUI_LEN];

/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
/* 02 dev侧用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack(1)
#endif

/* 此文件中结构体与协议一致，要求1字节对齐，统一加__OAL_DECLARE_PACKED */
struct mac_ether_header {
    oal_uint8    auc_ether_dhost[ETHER_ADDR_LEN];
    oal_uint8    auc_ether_shost[ETHER_ADDR_LEN];
    oal_uint16   us_ether_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_ether_header mac_ether_header_stru;

struct mac_llc_snap {
    oal_uint8   uc_llc_dsap;
    oal_uint8   uc_llc_ssap;
    oal_uint8   uc_control;
    oal_uint8   auc_org_code[3];
    oal_uint16  us_ether_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_llc_snap mac_llc_snap_stru;

/* eapol帧头 */
struct mac_eapol_header {
    oal_uint8       uc_version;
    oal_uint8       uc_type;
    oal_uint16      us_length;
} __OAL_DECLARE_PACKED;
typedef struct mac_eapol_header mac_eapol_header_stru;

/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
/* EAPOL KEY 结构定义 */
struct mac_eapol_key {
    oal_uint8 uc_type;
    /* Note: key_info, key_length, and key_data_length are unaligned */
    oal_uint8 auc_key_info[2];          /* big endian */
    oal_uint8 auc_key_length[2];        /* big endian */
    oal_uint8 auc_replay_counter[WPA_REPLAY_COUNTER_LEN];
    oal_uint8 auc_key_nonce[WPA_NONCE_LEN];
    oal_uint8 auc_key_iv[16];
    oal_uint8 auc_key_rsc[WPA_KEY_RSC_LEN];
    oal_uint8 auc_key_id[8];            /* Reserved in IEEE 802.11i/RSN */
    oal_uint8 auc_key_mic[16];
    oal_uint8 auc_key_data_length[2];   /* big endian */
    /* followed by key_data_length bytes of key_data */
} __OAL_DECLARE_PACKED;
typedef struct mac_eapol_key mac_eapol_key_stru;

/*
 * Structure of the IP frame
 */
struct mac_ip_header {
    oal_uint8    uc_version_ihl;
    oal_uint8    uc_tos;
    oal_uint16   us_tot_len;
    oal_uint16   us_id;
    oal_uint16   us_frag_off;
    oal_uint8    uc_ttl;
    oal_uint8    uc_protocol;
    oal_uint16   us_check;
    oal_uint32   ul_saddr;
    oal_uint32   ul_daddr;
    /* The options start here. */
} __OAL_DECLARE_PACKED;
typedef struct mac_ip_header mac_ip_header_stru;

/*
 *    Header in on cable format
 */
struct mac_tcp_header {
    oal_uint16  us_sport;
    oal_uint16  us_dport;
    oal_uint32  ul_seqnum;
    oal_uint32  ul_acknum;
    oal_uint8   uc_offset;
    oal_uint8   uc_flags;
    oal_uint16  us_window;
    oal_uint16  us_check;
    oal_uint16  us_urgent;
} __OAL_DECLARE_PACKED;
typedef struct mac_tcp_header mac_tcp_header_stru;

/* UDP头部结构 */
typedef struct {
    oal_uint16   us_src_port;
    oal_uint16   us_des_port;
    oal_uint16   us_udp_len;
    oal_uint16   us_check_sum;
} udp_hdr_stru;

/* CCMP/TKIP 加密头部 */
/* CCMP 加密IV 字段结构
 * PN0 PN1 RSVD [KEY_ID] PN2 PN3 PN4 PN5
 *              |      |
 *          b0  b4  b5  b6~b7
 *          00000    1  key_id
 */
/* TKIP 加密IV 字段结构
 * TSC1 WEPSeed[1] TSC0 [KEY_ID] TSC2 TSC3 TSC4 TSC5
 *                      |      |
 *                  b0  b4  b5  b6~b7
 *                  00000    1  key_id
 * WEPSeed[1] = (TSC1 | 0x20) & 0x7f
 */
typedef union {
    oal_uint64 ull_pn64;
    oal_uint32 ul_pn32[2];
    oal_uint8  uc_pn8[8];
} mac_pn_union;

/* frame control字段结构体 */
struct mac_header_frame_control {
    oal_uint16  bit_protocol_version    : 2,        /* 协议版本 */
                bit_type                : 2,        /* 帧类型 */
                bit_sub_type            : 4,        /* 子类型 */
                bit_to_ds               : 1,        /* 发送DS */
                bit_from_ds             : 1,        /* 来自DS */
                bit_more_frag           : 1,        /* 分段标识 */
                bit_retry               : 1,        /* 重传帧 */
                bit_power_mgmt          : 1,        /* 节能管理 */
                bit_more_data           : 1,        /* 更多数据标识 */
                bit_protected_frame     : 1,        /* 加密标识 */
                bit_order               : 1;        /* 次序位 */
} __OAL_DECLARE_PACKED;
typedef struct mac_header_frame_control mac_header_frame_control_stru;

/* 基础802.11帧结构 */
struct mac_ieee80211_frame {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16
    bit_duration_value      : 15,
                            bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num    : 4,
                                    bit_seq_num     : 12;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_frame mac_ieee80211_frame_stru;

/* ps poll帧结构 */
struct mac_ieee80211_pspoll_frame {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_aid_value   : 14,                   /* ps poll 下的AID字段 */
                                    bit_aid_flag1   : 1,
                                    bit_aid_flag2   : 1;
    oal_uint8                       auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_trans_addr[WLAN_MAC_ADDR_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_pspoll_frame mac_ieee80211_pspoll_frame_stru;

/* qos帧结构 */
struct mac_ieee80211_qos_frame {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_duration_value      : 15,           /* duration/id */
                                    bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num        : 4,                /* sequence control */
                                    bit_seq_num         : 12;
    oal_uint8                       bit_qc_tid          : 4,
                                    bit_qc_eosp         : 1,
                                    bit_qc_ack_polocy   : 2,
                                    bit_qc_amsdu        : 1;
    union {
        oal_uint8                       bit_qc_txop_limit;
        oal_uint8                       bit_qc_ps_buf_state_resv        : 1,
                                        bit_qc_ps_buf_state_inducated   : 1,
                                        bit_qc_hi_priority_buf_ac       : 2,
                                        bit_qc_qosap_buf_load           : 4;
    } qos_control;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame mac_ieee80211_qos_frame_stru;

/* qos+HTC 帧结构 */
struct mac_ieee80211_qos_htc_frame {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_duration_value      : 15,           /* duration/id */
                                    bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num     : 4,                /* sequence control */
                                    bit_seq_num      : 12;
    oal_uint8                       bit_qc_tid          : 4,
                                    bit_qc_eosp         : 1,
                                    bit_qc_ack_polocy   : 2,
                                    bit_qc_amsdu        : 1;
    union {
        oal_uint8                       bit_qc_txop_limit;
        oal_uint8                       bit_qc_ps_buf_state_resv        : 1,
                                        bit_qc_ps_buf_state_inducated   : 1,
                                        bit_qc_hi_priority_buf_ac       : 2,
                                        bit_qc_qosap_buf_load           : 4;
    } qos_control;

    oal_uint8                       auc_htc[4];
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame mac_ieee80211_qos_htc_frame_stru;

/* 四地址帧结构体 */
struct mac_ieee80211_frame_addr4 {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16
    bit_duration_value      : 15,
                            bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num            : 4,
                                    bit_seq_num             : 12;
    oal_uint8                       auc_address4[WLAN_MAC_ADDR_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_frame_addr4 mac_ieee80211_frame_addr4_stru;

/* qos四地址帧结构 */
struct mac_ieee80211_qos_frame_addr4 {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_duration_value      : 15,           /* duration/id */
                                    bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num        : 4,                /* sequence control */
                                    bit_seq_num         : 12;
    oal_uint8                       auc_address4[WLAN_MAC_ADDR_LEN];
    oal_uint8                       bit_qc_tid          : 4,
                                    bit_qc_eosp         : 1,
                                    bit_qc_ack_polocy   : 2,
                                    bit_qc_amsdu        : 1;
    union {
        oal_uint8                   qc_txop_limit;                          /* txop limit字段 */
        oal_uint8                   qc_queue_size;                          /* queue size字段 */
        oal_uint8                   bit_qc_ps_buf_state_resv        : 1,    /* AP PS Buffer State */
                                    bit_qc_ps_buf_state_inducated   : 1,
                                    bit_qc_hi_priority_buf_ac       : 2,
                                    bit_qc_qosap_buf_load           : 4;
    } qos_control;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame_addr4 mac_ieee80211_qos_frame_addr4_stru;

/* qos htc 四地址帧结构 */
struct mac_ieee80211_qos_htc_frame_addr4 {
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_duration_value      : 15,           /* duration/id */
                                    bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_frag_num     : 4,                /* sequence control */
                                    bit_seq_num      : 12;
    oal_uint8                       auc_address4[WLAN_MAC_ADDR_LEN];
    oal_uint8                       bit_qc_tid          : 4,
                                    bit_qc_eosp         : 1,
                                    bit_qc_ack_polocy   : 2,
                                    bit_qc_amsdu        : 1;
    union {
        oal_uint8                   qc_txop_limit;                          /* txop limit字段 */
        oal_uint8                   qc_queue_size;                          /* queue size字段 */
        oal_uint8                   bit_qc_ps_buf_state_resv        : 1,    /* AP PS Buffer State */
                                    bit_qc_ps_buf_state_inducated   : 1,
                                    bit_qc_hi_priority_buf_ac       : 2,
                                    bit_qc_qosap_buf_load           : 4;
    } qos_control;

    oal_uint32                      ul_htc;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame_addr4 mac_ieee80211_qos_htc_frame_addr4_stru;

/* Ref. 802.11-2012.pdf, 8.4.1.4 Capability information field, 中文注释参考白皮书 */
struct mac_cap_info {
    oal_uint16  bit_ess                 : 1,        /* 由BSS中的AP设置为1 */
                bit_ibss                : 1,        /* 由一个IBSS中的站点设置为1，ap总是设置其为0 */
                bit_cf_pollable         : 1,        /* 标识CF-POLL能力 */
                bit_cf_poll_request     : 1,        /* 标识CF-POLL能力 */
                bit_privacy             : 1,        /* 1=需要加密, 0=不需要加密 */
                bit_short_preamble      : 1,        /* 802.11b短前导码 */
                bit_pbcc                : 1,        /* 802.11g */
                bit_channel_agility     : 1,        /* 802.11b */
                bit_spectrum_mgmt       : 1,        /* 频谱管理: 0=不支持, 1=支持 */
                bit_qos                 : 1,        /* QOS: 0=非QOS站点, 1=QOS站点 */
                bit_short_slot_time     : 1,        /* 短时隙: 0=不支持, 1=支持 */
                bit_apsd                : 1,        /* 自动节能: 0=不支持, 1=支持 */
                bit_radio_measurement   : 1,        /* Radio检测: 0=不支持, 1=支持 */
                bit_dsss_ofdm           : 1,        /* 802.11g */
                bit_delayed_block_ack   : 1,        /* 延迟块确认: 0=不支持, 1=支持 */
                bit_immediate_block_ack : 1;        /* 立即块确认: 0=不支持, 1=支持 */
} __OAL_DECLARE_PACKED;
typedef struct mac_cap_info mac_cap_info_stru;

/* Ref. 802.11-2012.pdf, 8.4.2.58.2 HT Capabilities Info field */
struct mac_frame_ht_cap {
    oal_uint16          bit_ldpc_coding_cap         : 1,              /* LDPC 编码 capability */
                        bit_supported_channel_width : 1,              /* STA 支持的带宽 */
                        bit_sm_power_save           : 2,              /* SM 省电模式 */
                        bit_ht_green_field          : 1,              /* 绿野模式 */
                        bit_short_gi_20mhz          : 1,              /* 20M下短保护间隔 */
                        bit_short_gi_40mhz          : 1,              /* 40M下短保护间隔 */
                        bit_tx_stbc                 : 1, /* Indicates support for the transmission of PPDUs use STBC */
                        bit_rx_stbc                 : 2,              /* 支持 Rx STBC */
                        bit_ht_delayed_block_ack    : 1, /* Indicates support for HT-delayed Block Ack opera-tion. */
                        bit_max_amsdu_length        : 1, /* Indicates maximum A-MSDU length. */
                        bit_dsss_cck_mode_40mhz     : 1,              /* 40M下 DSSS/CCK 模式 */
                        bit_resv                    : 1,
                        bit_forty_mhz_intolerant    : 1, /* Indicates whether APs receiving this information */
                        bit_lsig_txop_protection    : 1;              /* 支持 L-SIG TXOP 保护 */
} __OAL_DECLARE_PACKED;
typedef struct mac_frame_ht_cap mac_frame_ht_cap_stru;

struct mac_vht_cap_info {
    oal_uint32  bit_max_mpdu_length         : 2,
                bit_supported_channel_width : 2,
                bit_rx_ldpc                 : 1,
                bit_short_gi_80mhz          : 1,
                bit_short_gi_160mhz         : 1,
                bit_tx_stbc                 : 1,
                bit_rx_stbc                 : 3,
                bit_su_beamformer_cap       : 1,
                bit_su_beamformee_cap       : 1,
                bit_num_bf_ant_supported    : 3,
                bit_num_sounding_dim        : 3,
                bit_mu_beamformer_cap       : 1,
                bit_mu_beamformee_cap       : 1,
                bit_vht_txop_ps             : 1,
                bit_htc_vht_capable         : 1,
                bit_max_ampdu_len_exp       : 3,
                bit_vht_link_adaptation     : 2,
                bit_rx_ant_pattern          : 1,
                bit_tx_ant_pattern          : 1,
                bit_resv                    : 2;
};
typedef struct mac_vht_cap_info mac_vht_cap_info_stru;

struct mac_11ntxbf_info {
    oal_uint8       bit_11ntxbf                 :1,          /* 11n txbf  能力 */
                    bit_reserve                 :7;
    oal_uint8                        auc_reserve[3];
};
typedef struct mac_11ntxbf_info mac_11ntxbf_info_stru;

struct mac_11ntxbf_vendor_ie {
    oal_uint8                        uc_id;          /* element ID */
    oal_uint8                        uc_len;         /* length in bytes */

    oal_uint8                        auc_oui[3];
    oal_uint8                        uc_ouitype;
    mac_11ntxbf_info_stru            st_11ntxbf;
};
typedef struct mac_11ntxbf_vendor_ie mac_11ntxbf_vendor_ie_stru;

/* 厂家自定义IE 数据结构，摘自linux 内核 */
struct mac_ieee80211_vendor_ie {
    oal_uint8 uc_element_id;
    oal_uint8 uc_len;
    oal_uint8 auc_oui[3];
    oal_uint8 uc_oui_type;
} __OAL_DECLARE_PACKED;
typedef struct mac_ieee80211_vendor_ie mac_ieee80211_vendor_ie_stru;

/* 建立BA会话时，BA参数域结构定义 */
struct mac_ba_parameterset {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* BIG_ENDIAN */
    oal_uint16  bit_buffersize      : 10,               /* B6-15  buffer size */
                bit_tid             : 4,                /* B2-5   TID */
                bit_bapolicy        : 1,                /* B1   block ack policy */
                bit_amsdusupported  : 1;                /* B0   amsdu supported */
#else
    oal_uint16  bit_amsdusupported  : 1,                /* B0   amsdu supported */
                bit_bapolicy        : 1,                /* B1   block ack policy */
                bit_tid             : 4,                /* B2-5   TID */
                bit_buffersize      : 10;               /* B6-15  buffer size */
#endif
} __OAL_DECLARE_PACKED;
typedef struct mac_ba_parameterset mac_ba_parameterset_stru;

/* BA会话过程中的序列号参数域定义 */
struct mac_ba_seqctrl {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* BIG_ENDIAN */
    oal_uint16  bit_startseqnum     : 12,           /* B4-15  starting sequence number */
                bit_fragnum         : 4;            /* B0-3  fragment number */
#else
    oal_uint16  bit_fragnum         : 4,            /* B0-3  fragment number */
                bit_startseqnum     : 12;           /* B4-15  starting sequence number */
#endif
} __OAL_DECLARE_PACKED;
typedef struct mac_ba_seqctrl mac_ba_seqctrl_stru;

/* Quiet信息元素结构体 */
struct mac_quiet_ie {
    oal_uint8     quiet_count;
    oal_uint8     quiet_period;
    oal_uint16    quiet_duration;
    oal_uint16    quiet_offset;
} __OAL_DECLARE_PACKED;
typedef struct mac_quiet_ie mac_quiet_ie_stru;


/* erp 信息元素结构体 */
struct mac_erp_params {
    oal_uint8   bit_non_erp       : 1,
                bit_use_protection: 1,
                bit_preamble_mode : 1,
                bit_resv          : 5;
} __OAL_DECLARE_PACKED;
typedef struct mac_erp_params mac_erp_params_stru;

/* rsn信息元素 rsn能力字段结构体 */
struct mac_rsn_cap {
    oal_uint16  bit_pre_auth            : 1,
                bit_no_pairwise         : 1,
                bit_ptska_relay_counter : 2,
                bit_gtska_relay_counter : 2,
                bit_mfpr                : 1,
                bit_mfpc                : 1,
                bit_rsv0                : 1,
                bit_peer_key            : 1,
                bit_spp_amsdu_capable   : 1,
                bit_spp_amsdu_required  : 1,
                bit_pbac                : 1,
                bit_ext_key_id          : 1,
                bit_rsv1                : 2;
} __OAL_DECLARE_PACKED;
typedef struct mac_rsn_cap mac_rsn_cap_stru;


/* obss扫描ie obss扫描参数结构体 */
struct mac_obss_scan_params {
    oal_uint16 us_passive_dwell;
    oal_uint16 us_active_dwell;
    oal_uint16 us_scan_interval;
    oal_uint16 us_passive_total_per_chan;
    oal_uint16 us_active_total_per_chan;
    oal_uint16 us_transition_delay_factor;
    oal_uint16 us_scan_activity_thresh;
} __OAL_DECLARE_PACKED;
typedef struct mac_obss_scan_params mac_obss_scan_params_stru;

/* 扩展能力信息元素结构体定义 */
struct mac_ext_cap_ie {
    oal_uint8   bit_2040_coexistence_mgmt: 1,
                bit_resv1                : 1,
                bit_ext_chan_switch      : 1,
                bit_resv2                : 1,
                bit_psmp                 : 1,
                bit_resv3                : 1,
                bit_s_psmp               : 1,
                bit_event                : 1;
    oal_uint8   bit_resv4                : 4,
                bit_proxyarp             : 1,
                bit_resv13               : 3;
    oal_uint8   bit_resv5                : 3,
                bit_bss_transition       : 1,   /* bit19 */
                bit_resv14               : 4;   /* bit20~bit23 */
    oal_uint8   bit_resv6                : 7,
                bit_interworking         : 1;
    oal_uint8   bit_resv7                         : 5,
                bit_tdls_prhibited                : 1,
                bit_tdls_channel_switch_prhibited : 1,
                bit_resv8                         : 1;

    oal_uint8   bit_resv9                : 8;
    oal_uint8   bit_resv10               : 8;

    oal_uint8   bit_resv11                        : 6,
                bit_operating_mode_notification   : 1, /* 11ac Operating Mode Notification特性标志 */
                bit_resv12                        : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap_ie mac_ext_cap_ie_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
struct mac_ext_cap_ftm_ie {
    oal_uint8   bit_2040_coexistence_mgmt: 1,
                bit_resv1                : 1,
                bit_ext_chan_switch      : 1,
                bit_resv2                : 1,
                bit_psmp                 : 1,
                bit_resv3                : 1,
                bit_s_psmp               : 1,
                bit_event                : 1;
    oal_uint8   bit_resv4                : 4,
                bit_proxyarp             : 1,
                bit_resv13               : 3;   /* bit13~bit15 */
    oal_uint8   bit_resv5                : 4,
                bit_bss_transition       : 1,   /* bit19 */
                bit_resv14               : 3;   /* bit20~bit23 */
    oal_uint8   bit_resv6                : 7,
                bit_interworking         : 1;
    oal_uint8   bit_resv7                         : 5,
                bit_tdls_prhibited                : 1,
                bit_tdls_channel_switch_prhibited : 1,
                bit_resv8                         : 1;

    oal_uint8   bit_resv9                : 8;
    oal_uint8   bit_resv10               : 8;

    oal_uint8   bit_resv11                        : 6,
                bit_operating_mode_notification   : 1, /* 11ac Operating Mode Notification特性标志 */
                bit_resv12                        : 1;

    oal_uint8   bit_resv15                        : 6,
                bit_ftm_resp                      : 1, /* bit70 Fine Timing Measurement Responder */
                bit_ftm_int                       : 1; /* bit71 Fine Timing Measurement Initiator */
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap_ftm_ie mac_ext_cap_ftm_ie_stru;
#endif

/* qos info字段结构体定义 */
struct mac_qos_info {
    oal_uint8   bit_params_count: 4,
                bit_resv        : 3,
                bit_uapsd       : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_qos_info mac_qos_info_stru;

/* wmm信息元素 ac参数结构体 */
typedef struct {
    oal_uint8   bit_aifsn : 4,
                bit_acm   : 1,
                bit_aci   : 2,
                bit_resv  : 1;
    oal_uint8   bit_ecwmin: 4,
                bit_ecwmax: 4;
    oal_uint16  us_txop;
} mac_wmm_ac_params_stru;

/* BSS load信息元素结构体 */
/*lint -e958*/
/* 屏蔽字节对齐错误 */
struct mac_bss_load {
    oal_uint16 us_sta_count;            /* 关联的sta个数 */
    oal_uint8  uc_chan_utilization;     /* 信道利用率 */
    oal_uint16 us_aac;
} __OAL_DECLARE_PACKED;
typedef struct mac_bss_load mac_bss_load_stru;
/*lint +e958*/
/* country信息元素 管制域字段 */
struct mac_country_reg_field {
    oal_uint8 uc_first_channel;         /* 第一个信道号 */
    oal_uint8 uc_channel_num;           /* 信道个数 */
    oal_uint8 uc_max_tx_pwr;            /* 最大传输功率，dBm */
} __OAL_DECLARE_PACKED;
typedef struct mac_country_reg_field mac_country_reg_field_stru;

/* ht capabilities信息元素支持的ampdu parameters字段结构体定义 */
struct mac_ampdu_params {
    oal_uint8  bit_max_ampdu_len_exponent  : 2,
               bit_min_mpdu_start_spacing  : 3,
               bit_resv                    : 3;
} __OAL_DECLARE_PACKED;
typedef struct mac_ampdu_params mac_ampdu_params_stru;


/* ht cap信息元素 支持的mcs集字段 结构体定义 */
struct mac_sup_mcs_set {
    oal_uint8   auc_rx_mcs[10];
    oal_uint16  bit_rx_highest_rate: 10,
                bit_resv1          : 6;
    oal_uint32  bit_tx_mcs_set_def : 1,
                bit_tx_rx_not_equal: 1,
                bit_tx_max_stream  : 2,
                bit_tx_unequal_modu: 1,
                bit_resv2          : 27;
} __OAL_DECLARE_PACKED;
typedef struct mac_sup_mcs_set mac_sup_mcs_set_stru;


/* vht信息元素，支持的mcs集字段 */
typedef struct {
    oal_uint32  bit_rx_mcs_map      : 16,
                bit_rx_highest_rate : 13,
                bit_resv            : 3;
    oal_uint32  bit_tx_mcs_map      : 16,
                bit_tx_highest_rate : 13,
                bit_resv2           : 3;
} mac_vht_sup_mcs_set_stru;

/* ht capabilities信息元素支持的extended cap.字段结构体定义 */
struct mac_ext_cap {
    oal_uint16  bit_pco           : 1,
                bit_pco_trans_time: 2,
                bit_resv1         : 5,
                bit_mcs_fdbk      : 2,
                bit_htc_sup       : 1,
                bit_rd_resp       : 1,
                bit_resv2         : 4;
} __OAL_DECLARE_PACKED;
typedef struct mac_ext_cap mac_ext_cap_stru;


/* ht cap信息元素的Transmit Beamforming Capabilities字段结构体定义 */
typedef struct {
    oal_uint32  bit_implicit_txbf_rx                : 1,
                bit_rx_stagg_sounding               : 1,
                bit_tx_stagg_sounding               : 1,
                bit_rx_ndp                          : 1,
                bit_tx_ndp                          : 1,
                bit_implicit_txbf                   : 1,
                bit_calibration                     : 2,
                bit_explicit_csi_txbf               : 1,
                bit_explicit_noncompr_steering      : 1,
                bit_explicit_compr_Steering         : 1,
                bit_explicit_txbf_csi_fdbk          : 2,
                bit_explicit_noncompr_bf_fdbk       : 2,
                bit_explicit_compr_bf_fdbk          : 2,
                bit_minimal_grouping                : 2,
                bit_csi_num_bf_antssup              : 2,
                bit_noncompr_steering_num_bf_antssup: 2,
                bit_compr_steering_num_bf_antssup   : 2,
                bit_csi_maxnum_rows_bf_sup          : 2,
                bit_chan_estimation                 : 2,
                bit_resv                            : 3;
} mac_txbf_cap_stru;

/* ht cap信息元素的Asel(antenna selection) Capabilities字段结构体定义 */
struct mac_asel_cap {
    oal_uint8  bit_asel                         : 1,
               bit_explicit_sci_fdbk_tx_asel    : 1,
               bit_antenna_indices_fdbk_tx_asel : 1,
               bit_explicit_csi_fdbk            : 1,
               bit_antenna_indices_fdbk         : 1,
               bit_rx_asel                      : 1,
               bit_trans_sounding_ppdu          : 1,
               bit_resv                         : 1;
} __OAL_DECLARE_PACKED;
typedef struct mac_asel_cap mac_asel_cap_stru;


/* ht opern元素, ref 802.11-2012 8.4.2.59 */
struct mac_ht_opern {
    oal_uint8   uc_primary_channel;

    oal_uint8   bit_secondary_chan_offset             : 2,
                bit_sta_chan_width                    : 1,
                bit_rifs_mode                         : 1,
                bit_resv1                             : 4;
    oal_uint8   bit_HT_protection                     : 2,
                bit_nongf_sta_present                 : 1,
                bit_resv2                             : 1,
                bit_obss_nonht_sta_present            : 1,
                bit_resv3                             : 3;
    oal_uint8   bit_resv4                             : 8;
    oal_uint8   bit_resv5                             : 6,
                bit_dual_beacon                       : 1,
                bit_dual_cts_protection               : 1;
    oal_uint8   bit_secondary_beacon                  : 1,
                bit_lsig_txop_protection_full_support : 1,
                bit_pco_active                        : 1,
                bit_pco_phase                         : 1,
                bit_resv6                             : 4;

    oal_uint8   auc_basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
} __OAL_DECLARE_PACKED;
typedef struct mac_ht_opern mac_ht_opern_stru;

/* vht opern结构体 */
/*lint -e958*/
/* 屏蔽字节对齐错误 */
struct mac_opmode_notify {
    oal_uint8   bit_channel_width   : 2,     /* 当前最大允许带宽能力 */
                bit_resv            : 2,     /* 保留 */
                bit_rx_nss          : 3,     /* 当前最大允许空间流能力 */
                bit_rx_nss_type     : 1;     /* 是否为TXBF下的rx nss能力，1-是，0不是 */
} __OAL_DECLARE_PACKED;
typedef struct mac_opmode_notify mac_opmode_notify_stru;
/*lint +e958*/
/* vht opern结构体 */
/*lint -e958*/
/* 屏蔽字节对齐错误 */
struct mac_vht_opern {
    oal_uint8   uc_channel_width;
    oal_uint8   uc_channel_center_freq_seg0;
    oal_uint8   uc_channel_center_freq_seg1;
    oal_uint16  us_basic_mcs_set;
} __OAL_DECLARE_PACKED;
typedef struct mac_vht_opern mac_vht_opern_stru;
/*lint +e958*/
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
/*lint -e958*/
/* 屏蔽字节对齐错误 */
/* RRM ENABLED CAP信息元素结构体 */
struct mac_rrm_enabled_cap_ie {
    oal_uint8   bit_link_cap            : 1,  /* bit0: Link Measurement capability enabled */
                bit_neighbor_rpt_cap    : 1,  /* bit1: Neighbor Report capability enabled */
                bit_parallel_cap        : 1,  /* bit2: Parallel Measurements capability enabled */
                bit_repeat_cap          : 1,  /* bit3: Repeated Measurements capability enabled */
                bit_bcn_passive_cap     : 1,  /* bit4: Beacon Passive Measurements capability enabled */
                bit_bcn_active_cap      : 1,  /* bit5: Beacon Active Measurements capability enabled */
                bit_bcn_table_cap       : 1,  /* bit6: Beacon Table Measurements capability enabled */
                bit_bcn_meas_rpt_cond_cap: 1; /* bit7: Beacon Measurement Reporting Conditions capability enabled */
    oal_uint8   bit_frame_cap           : 1,  /* bit8: Frame Measurement capability enabled */
                bit_chn_load_cap        : 1,  /* bit9: Channel Load Measurement capability enabled */
                bit_noise_histogram_cap : 1,  /* bit10: Noise Histogram Measurement capability enabled */
                bit_stat_cap            : 1,  /* bit11: Statistics Measurement capability enabled */
                bit_lci_cap             : 1,  /* bit12: LCI Measurement capability enabled */
                bit_lci_azimuth_cap     : 1,  /* bit13: LCI Azimuth capability enabled */
                bit_tsc_cap             : 1,  /* bit14: Transmit Stream/Category Measurement capability enabled */
                bit_triggered_tsc_cap   : 1;  /* bit15: Triggered  Transmit Stream/Category Measurement capability */
    oal_uint8   bit_ap_chn_rpt_cap                  : 1, /* bit16: AP Channel Report capability enabled */
                bit_rm_mib_cap                      : 1, /* bit17: RM MIB capability enabled */
                bit_oper_chn_max_meas_duration      : 3, /* bit18-20: Operating Channel Max Measurement Duration */
                bit_non_oper_chn_max_meas_duration  : 3; /* bit21-23: Non-operating Channel Max Measurement Durationg */
    oal_uint8   bit_meas_pilot_cap              : 3, /* bit24-26: Measurement Pilot capability */
                bit_meas_pilot_trans_info_cap   : 1, /* bit27: Measurement Pilot Transmission Information capability */
                bit_neighbor_rpt_tsf_offset_cap : 1, /* bit28: Neighbor Report TSF Offset capability enabled */
                bit_rcpi_cap                    : 1, /* bit29: RCPI Measurement capability enabled */
                bit_rsni_cap                    : 1, /* bit30: RSNI Measurement capability enabled */
                bit_bss_avg_access_dly          : 1; /* bit31: BSS Average Access Delay capability enabled */
    oal_uint8   bit_avail_admission_capacity_cap: 1, /* bit32: BSS Available Admission Capacity capability enabled */
                bit_antenna_cap                 : 1, /* bit33: Antenna capability enabled */
                bit_ftm_range_report_cap        : 1, /* bit34: FTM range report capability enabled */
                bit_rsv                         : 5; /* bit34-39: Reserved */
} __OAL_DECLARE_PACKED;
typedef struct mac_rrm_enabled_cap_ie mac_rrm_enabled_cap_ie_stru;

/* Measurement Report Mode */
struct mac_mr_mode {
    oal_uint8   bit_parallel    : 1,
                bit_enable      : 1,
                bit_request     : 1,
                bit_rpt         : 1,
                bit_duration_mandatory  : 1,
                bit_rsvd        :3;
} __OAL_DECLARE_PACKED;
typedef struct mac_mr_mode mac_mr_mode_stru;

/* Radio Measurement Request */
struct mac_action_rm_req {
    oal_uint8   uc_category;
    oal_uint8   uc_action_code;
    oal_uint8   uc_dialog_token;
    oal_uint16  us_num_rpt;
    oal_uint8   auc_req_ies[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_rm_req mac_action_rm_req_stru;

/* Radio Measurement Report */
struct mac_action_rm_rpt {
    oal_uint8   uc_category;
    oal_uint8   uc_action_code;
    oal_uint8   uc_dialog_token;
    oal_uint8   auc_rpt_ies[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_action_rm_rpt mac_action_rm_rpt_stru;


/* Measurement Request Elements */
struct mac_meas_req_ie {
    oal_uint8           uc_eid;         /* IEEE80211_ELEMID_MEASREQ */
    oal_uint8           uc_len;
    oal_uint8           uc_token;
    mac_mr_mode_stru    st_reqmode;
    oal_uint8           uc_reqtype;
    oal_uint8           auc_meas_req[1];     /* varialbe len measurement requet */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_req_ie mac_meas_req_ie_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
struct mac_ftm_range_req_ie {
    oal_uint16                          us_randomization_interval;
    oal_uint8                           uc_minimum_ap_count;
    oal_uint8                           auc_ftm_range_subelements[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_ftm_range_req_ie mac_ftm_range_req_ie_stru;

/* Fine Timing Measurement Parameters */
struct mac_ftm_parameters_ie {
    oal_uint8               uc_eid;         /* IEEE80211_ELEMID_MEASRPT */
    oal_uint8               uc_len;
    oal_uint8               bit_status_indication                         :2,
                            bit_value                                     :5,
                            bit_reserved0                                 :1;
    oal_uint8               bit_number_of_bursts_exponent                 :4,
                            bit_burst_duration                            :4;
    oal_uint8               uc_min_delta_ftm;
    oal_uint16              us_partial_tsf_timer;
    oal_uint8               bit_partial_tsf_timer_no_preference           :1,
                            bit_asap_capable                              :1,
                            bit_asap                                      :1,
                            bit_ftms_per_burst                            :5;
    oal_uint8               bit_reserved                                  :2,
                            bit_format_and_bandwidth                      :6;
    oal_uint16              us_burst_period;
} __OAL_DECLARE_PACKED;
typedef struct mac_ftm_parameters_ie mac_ftm_parameters_ie_stru;
#endif

/* Measurement Report Mode */
struct mac_meas_rpt_mode {
    oal_uint8   bit_late        : 1,
                bit_incapable   : 1,
                bit_refused     : 1,
                bit_rsvd        : 5;
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_rpt_mode    mac_meas_rpt_mode_stru;
/* Measurement Report Elements */
struct mac_meas_rpt_ie {
    oal_uint8                   uc_eid;         /* IEEE80211_ELEMID_MEASRPT */
    oal_uint8                   uc_len;
    oal_uint8                   uc_token;
    mac_meas_rpt_mode_stru      st_rptmode;
    oal_uint8                   uc_rpttype;
    oal_uint8                   auc_meas_rpt[100];     /* varialbe len measurement report */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_rpt_ie mac_meas_rpt_ie_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
/* Neighbor Report Elements */
struct mac_bssid_info {
    oal_uint32                  bit_ap_reachability:2,
                                bit_security:1,
                                bit_key_scope:1,
                                bit_spec_management:1,
                                bit_qos:1,
                                bit_apsd:1,
                                bit_radio_meas:1,
                                bit_delayed_ba:1,
                                bit_immediate_ba:1,
                                bit_rsv:22;
} __OAL_DECLARE_PACKED;
typedef struct mac_bssid_info mac_bssid_info_stru;

struct mac_neighbor_rpt_ie {
    oal_uint8                   uc_eid;         /* IEEE80211_ELEMID_MEASRPT */
    oal_uint8                   uc_len;
    oal_uint8                   auc_bssid[WLAN_MAC_ADDR_LEN];
    mac_bssid_info_stru         st_bssid_info;
    oal_uint8                   uc_optclass;
    oal_uint8                   uc_channum;
    oal_uint8                   uc_phy_type;
    oal_uint8                   auc_subelm[1];      /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_neighbor_rpt_ie mac_neighbor_rpt_ie_stru;

struct mac_meas_sub_ie {
    oal_uint8           uc_sub_eid;         /* IEEE80211_ELEMID_MEASREQ */
    oal_uint8           uc_len;
    oal_uint8           auc_meas_sub_data[1];     /* varialbe len measurement requet */
} __OAL_DECLARE_PACKED;
typedef struct mac_meas_sub_ie mac_meas_sub_ie_stru;
#endif

/* Beacon report request */
struct mac_bcn_req {
    oal_uint8                           uc_optclass;
    oal_uint8                           uc_channum;         /* 请求测量的信道号 */
    oal_uint16                          us_random_ivl;
    oal_uint16                          us_duration;
    rm_bcn_req_meas_mode_enum_uint8     en_mode;
    oal_uint8                           auc_bssid[6];
    oal_uint8                           auc_subelm[1];     /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_bcn_req mac_bcn_req_stru;

/* Beacon report */
struct mac_bcn_rpt {
    oal_uint8    uc_optclass;
    oal_uint8    uc_channum;
    oal_uint8    auc_act_meas_start_time[8];
    oal_uint16   us_duration;
    oal_uint8    bit_condensed_phy_type:7,
                 bit_rpt_frm_type      :1;
    oal_uint8    uc_rcpi;
    oal_uint8    uc_rsni;
    oal_uint8    auc_bssid[6];
    oal_uint8    uc_antenna_id;
    oal_uint32   ul_parent_tsf;
    oal_uint8    auc_subelm[1];     /* varialbe len sub element fileds */
} __OAL_DECLARE_PACKED;
typedef struct mac_bcn_rpt mac_bcn_rpt_stru;

/* AP Channel Report */
struct mac_ap_chn_rpt {
    oal_uint8   uc_eid;
    oal_uint8   uc_length;
    oal_uint8   uc_oper_class;
    oal_uint8   auc_chan[1];
} __OAL_DECLARE_PACKED;
typedef struct mac_ap_chn_rpt mac_ap_chn_rpt_stru;

struct mac_quiet_frm {
    oal_uint8   uc_eid;
    oal_uint8   uc_len;
    oal_uint8   uc_quiet_count;
    oal_uint8   uc_quiet_period;
    oal_uint16  us_quiet_duration;
    oal_uint16  us_quiet_offset;
} __OAL_DECLARE_PACKED;
typedef struct mac_quiet_frm mac_quiet_frm_stru;

struct mac_pwr_constraint_frm {
    oal_uint8   uc_eid;
    oal_uint8   uc_len;
    oal_uint8   uc_local_pwr_constraint;
} __OAL_DECLARE_PACKED;
typedef struct mac_pwr_constraint_frm mac_pwr_constraint_frm_stru;
/*lint +e958*/
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
/* 02 dev侧用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()
#endif

#ifdef _PRE_WLAN_FEATURE_11K
typedef struct {
    oal_uint8           uc_dialog_token;
    oal_uint8           uc_meas_token;
    oal_uint8           uc_meas_type;
    oal_uint8           uc_opt_class;

    oal_uint8           uc_rpt_detail;
    oal_uint8           uc_req_ie_num;
    mac_mr_mode_stru    st_mr_mode;
    oal_uint8           uc_ssid_len;

    oal_uint16          us_meas_duration;
    oal_uint16          us_repetition;

    oal_uint8           *puc_reqinfo_ieid;
    oal_uint8           *puc_ssid;

    oal_uint8           auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8           auc_rsv[2];
} mac_bcn_req_info_stru;

typedef struct {
    /* Measurement Report field */
    oal_uint8                   uc_eid;         /* IEEE80211_ELEMID_MEASRPT */
    oal_uint8                   uc_len;
    oal_uint8                   uc_token;
    // mac_meas_rpt_mode_stru      st_rptmode;
    oal_uint8                   bit_late        : 1,
                                bit_incapable   : 1,
                                bit_refused     : 1,
                                bit_rsvd        : 5;

    oal_uint8                   uc_rpttype;
    /* beacon report field */
    oal_uint8                   uc_optclass;
    oal_uint8                   uc_channum;
    oal_uint8                   bit_condensed_phy_type:7,
                                bit_rpt_frm_type      :1;

    oal_uint8                   uc_rcpi;
    oal_uint8                   uc_rsni;
    oal_uint8                   uc_antenna_id;
    oal_uint8                   uc_rsv;

    oal_uint8                   auc_bssid[6];
    oal_uint8                   auc_act_meas_start_time[8];
    oal_uint16                  us_duration;

    oal_uint32                  ul_parent_tsf;
} mac_meas_rpt_bcn_item_stru;

typedef struct {
    oal_dlist_head_stru         st_dlist_head;
    mac_meas_rpt_bcn_item_stru *pst_meas_rpt_bcn_item;      /* 包含bcn rpt的meas rpt */
    oal_uint8                  *puc_rpt_detail_data;
    oal_uint32                  ul_rpt_detail_act_len;      /* rpt detail data实际长度 */
} mac_meas_rpt_bcn_stru;

typedef struct {
    oal_uint8               auc_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8               uc_action_dialog_token;
    oal_uint8               uc_meas_token;
    oal_uint8               uc_oper_class;
    oal_uint16              us_ssid_len;
    oal_uint8               auc_rsv[2];
} mac_vap_rrm_trans_req_info_stru;

typedef struct {
    oal_uint8               auc_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8               uc_action_dialog_token;
    oal_uint8               uc_meas_token;
    oal_uint8               uc_oper_class;
    oal_uint8               auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint16              us_ssid_len;
    oal_uint8               auc_rsv[2];
} mac_vap_rrm_trans_req_info2_stru;
#endif

/* ACTION帧的参数格式，注:不同的action帧下对应的参数不同 */
typedef struct {
    oal_uint8       uc_category;    /* ACTION的类别 */
    oal_uint8       uc_action;      /* 不同ACTION类别下的分类 */
    oal_uint8       uc_resv[2];
    oal_uint32      ul_arg1;
    oal_uint32      ul_arg2;
    oal_uint32      ul_arg3;
    oal_uint32      ul_arg4;
    oal_uint8      *puc_arg5;
} mac_action_mgmt_args_stru;

#if defined(_PRE_WLAN_FEATURE_WMMAC) || (defined(_PRE_WLAN_FEATURE_11K))
struct mac_ts_info {
    oal_uint16  bit_traffic_type: 1,
                bit_tsid        : 4,
                bit_direction   : 2,
                bit_acc_policy  : 2,
                bit_aggr        : 1,
                bit_apsd        : 1,
                bit_user_prio   : 3,
                bit_ack_policy  : 2;
    oal_uint8   bit_schedule    : 1,
                bit_rsvd        : 7;
} __OAL_DECLARE_PACKED;
typedef struct mac_ts_info mac_ts_info_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
struct mac_wmm_tspec {
    mac_ts_info_stru  ts_info;
    oal_uint16        us_norminal_msdu_size;
    oal_uint16        us_max_msdu_size;
    oal_uint32        ul_min_srv_interval;
    oal_uint32        ul_max_srv_interval;
    oal_uint32        ul_inactivity_interval;
    oal_uint32        ul_suspension_interval;
    oal_uint32        ul_srv_start_time;
    oal_uint32        ul_min_data_rate;
    oal_uint32        ul_mean_data_rate;
    oal_uint32        ul_peak_data_rate;
    oal_uint32        ul_max_burst_size;
    oal_uint32        ul_delay_bound;
    oal_uint32        ul_min_phy_rate;
    oal_uint16        us_surplus_bw;
    oal_uint16        us_medium_time;
} __OAL_DECLARE_PACKED;
typedef struct mac_wmm_tspec mac_wmm_tspec_stru;

#endif
#ifdef _PRE_WLAN_CHBA_MGMT
/* kernel的信道表示结构体 */
typedef struct {
    uint32_t center_freq_20m; /* 主20M的频点 */
    uint32_t center_freq1; /* 整个带宽的频点 */
    uint32_t center_freq2; /* 80+80下使用 */
    uint8_t bandwidth; /* 带宽 */
} mac_kernel_channel_stru;
#endif

/* 私有管理帧通用的设置参数信息的结构体 */
typedef struct {
    oal_uint8       uc_type;
    oal_uint8       uc_arg1;        /* 对应的tid序号 */
    oal_uint8       uc_arg2;        /* 接收端可接收的最大的mpdu的个数(针对AMPDU_START命令) */
    oal_uint8       uc_arg3;        /* 确认策略 */
    oal_uint16      us_user_idx;    /* 对应的用户 */
    oal_uint8       auc_resv[2];
} mac_priv_req_args_stru;

typedef oal_void (*p_mac_set_rrm_enabled_cap_field_cb)(oal_void *pst_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len);
typedef oal_void (*p_mac_set_ext_capabilities_ie_cb)(oal_void *pst_mac_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len);
typedef oal_void (*p_mac_set_vht_capabilities_ie_cb)(oal_void *pst_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len);
typedef oal_void (*p_mac_set_ht_capabilities_ie_cb)(oal_void *pst_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len);
typedef oal_rom_cb_result_enum_uint8 (*p_mac_set_rsn_ie_cb)(oal_void *pst_vap, oal_uint8 *puc_pmkid,
    oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint32 *pul_ret);
typedef oal_rom_cb_result_enum_uint8 (*p_mac_set_wpa_ie_cb)(oal_void *pst_vap, oal_uint8 *puc_buffer,
    oal_uint8 *puc_ie_len, oal_uint32 *pul_ret);

typedef struct {
    p_mac_set_rrm_enabled_cap_field_cb p_mac_set_rrm_enabled_cap_field;
    p_mac_set_vht_capabilities_ie_cb   p_mac_set_vht_capabilities_ie;
    p_mac_set_ht_capabilities_ie_cb    p_mac_set_ht_capabilities_ie;
    p_mac_set_ext_capabilities_ie_cb   p_mac_set_ext_capabilities_ie;
    p_mac_set_rsn_ie_cb                p_mac_set_rsn_ie;
    p_mac_set_wpa_ie_cb                p_mac_set_wpa_ie;
} mac_frame_rom_cb_stru;
extern mac_frame_rom_cb_stru *mac_get_frame_rom_cb_addr(oal_void);
extern oal_uint32 mac_get_customize_interworking(oal_void);
extern oal_void mac_set_customize_interworking(oal_uint32 customize_interworking);
#ifdef _PRE_WLAN_FEATURE_P2P
extern oal_uint8 *mac_find_p2p_attribute(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len);
#endif
extern oal_uint8 *mac_find_ie(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len);
extern oal_uint8 *mac_find_vendor_ie(oal_uint32              ul_oui,
                                     oal_uint8            uc_oui_type,
                                     oal_uint8           *puc_ies,
                                     oal_int32            l_len);

extern oal_void  mac_set_beacon_interval_field(oal_void *pst_mac_vap, oal_uint8 *puc_buffer);
extern oal_void  mac_set_cap_info_ap(oal_void *pst_mac_vap, oal_uint8 *puc_cap_info);
extern oal_void  mac_set_cap_info_sta(oal_void *pst_vap, oal_uint8 *puc_cap_info);
extern oal_void  mac_set_ssid_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len,
                                 oal_uint16 us_frm_type);

extern oal_void  mac_set_supported_rates_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_dsss_params(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_11D
extern oal_void  mac_set_country_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#endif

#ifdef _PRE_WLAN_NARROW_BAND
extern oal_void  mac_set_nb_ie(oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_get_nb_ie(oal_void *pst_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len);
#endif

extern oal_void  mac_set_pwrconstraint_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_quiet_ie(oal_void      *pst_vap,    oal_uint8 *puc_buffer, oal_uint8  uc_qcount,
                                  oal_uint8  uc_qperiod, oal_uint16 us_qdur,    oal_uint16 us_qoffset,
                                  oal_uint8 *puc_ie_len);

oal_void mac_set_11ntxbf_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);

extern oal_void  mac_set_erp_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_exsup_rates_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_bssload_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_timeout_interval_ie(oal_void *pst_vap,
                                             oal_uint8 *puc_buffer,
                                             oal_uint8 *puc_ie_len,
                                             oal_uint32 ul_type,
                                             oal_uint32 ul_timeout);
extern oal_void  mac_set_ht_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_ht_opern_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_obss_scan_params(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_ext_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_tpc_report_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void  mac_set_wmm_params_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_bool_enum_uint8 en_is_qos,
                                       oal_uint8 *puc_ie_len);
extern oal_uint8  *mac_get_ssid(oal_uint8 *puc_beacon_body, oal_int32 l_frame_body_len, oal_uint8 *puc_ssid_len);
extern oal_uint16  mac_get_beacon_period(oal_uint8 *puc_beacon_body);
extern oal_uint8   mac_get_dtim_period(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len);
extern oal_uint8   mac_get_dtim_cnt(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len);
extern oal_uint8  *mac_get_wmm_ie(oal_uint8 *puc_beacon_body, oal_uint16 us_frame_len);
extern oal_void    mac_set_power_cap_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void    mac_set_supported_channel_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void    mac_set_wmm_ie_sta(oal_uint8  *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_WMMAC
extern oal_uint16  mac_set_wmmac_ie_sta(oal_uint8  *pst_vap, oal_uint8 *puc_buffer,
                                        mac_wmm_tspec_stru *pst_addts_args);
#endif // _PRE_WLAN_FEATURE_WMMAC
extern oal_void    mac_set_listen_interval_ie(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_uint32  mac_set_rsn_ie(oal_void *pst_mac_vap, oal_uint8 *puc_pmkid, oal_uint8 *puc_buffer,
                                  oal_uint8 *puc_ie_len);
extern oal_uint32  mac_set_wpa_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void    mac_set_status_code_ie(oal_uint8 *puc_buffer, mac_status_code_enum_uint16 en_status_code);
extern oal_void    mac_set_aid_ie(oal_uint8 *puc_buffer, oal_uint16 uc_aid);
extern oal_uint8   mac_get_bss_type(oal_uint16 us_cap_info);
extern oal_uint32  mac_check_mac_privacy(oal_uint16 us_cap_info, oal_uint8 *pst_mac_vap);
extern oal_uint32  mac_check_mac_privacy_ap(oal_uint16 us_cap_info, oal_uint8 *pst_mac_ap);
extern oal_uint32  mac_check_mac_privacy_sta(oal_uint16 us_cap_info, oal_uint8 *pst_mac_sta);
extern oal_uint32  mac_check_privacy(mac_cap_info_stru *pst_cap_info, oal_uint8 *pst_mac_vap);
extern oal_void    mac_check_sta_base_rate(oal_uint8 *pst_mac_user, mac_status_code_enum_uint16 *pen_status_code);
extern oal_bool_enum_uint8  mac_is_wmm_ie(oal_uint8 *puc_ie);
extern oal_void    mac_set_vht_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void    mac_set_vht_opern_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_uint32  mac_set_csa_ie(oal_uint8 uc_mode, oal_uint8 uc_channel, oal_uint8 uc_csa_cnt,
                                  oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_uint32  mac_set_csa_bw_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);

#if defined(_PRE_WLAN_FEATURE_11K)  || defined(_PRE_WLAN_FEATURE_FTM)
extern oal_void mac_set_rrm_enabled_cap_field(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_wfa_tpc_report_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#endif
extern oal_uint32  mac_rx_report_80211_frame(oal_uint8 *pst_mac_vap,
                                             oal_uint8 *pst_rx_cb,
                                             oal_netbuf_stru *pst_netbuf,
                                             oam_ota_type_enum_uint8 en_ota_type);
extern oal_void mac_set_epigram_vht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_epigram_novht_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_null_data_encap(oal_uint8 *header, oal_uint16 us_fc, oal_uint8 *puc_da, oal_uint8 *puc_sa);


OAL_STATIC OAL_INLINE oal_void mac_hdr_set_frame_control(oal_uint8 *puc_header, oal_uint16 us_fc)
{
    *(oal_uint16 *)puc_header = us_fc;
}


OAL_STATIC OAL_INLINE oal_void mac_hdr_set_duration(oal_uint8 *puc_header, oal_uint16 us_duration)
{
    oal_uint16 *pus_dur = (oal_uint16 *)(puc_header + WLAN_HDR_DUR_OFFSET);

    *pus_dur = us_duration;
}


OAL_STATIC OAL_INLINE oal_void  mac_hdr_set_fragment_number(oal_uint8 *puc_header, oal_uint8 uc_frag_num)
{
    puc_header[WLAN_HDR_FRAG_OFFSET] &= 0xF0;
    puc_header[WLAN_HDR_FRAG_OFFSET] |= (uc_frag_num & 0x0F);
}


OAL_STATIC OAL_INLINE oal_void mac_hdr_set_from_ds(oal_uint8 *puc_header, oal_uint8 uc_from_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_from_ds = uc_from_ds;
}


OAL_STATIC OAL_INLINE oal_uint8 mac_hdr_get_from_ds(oal_uint8 *puc_header)
{
    return (oal_uint8)((mac_header_frame_control_stru *)(puc_header))->bit_from_ds;
}


OAL_STATIC OAL_INLINE oal_void mac_hdr_set_to_ds(oal_uint8 *puc_header, oal_uint8 uc_to_ds)
{
    ((mac_header_frame_control_stru *)(puc_header))->bit_to_ds = uc_to_ds;
}


OAL_STATIC OAL_INLINE oal_uint8 mac_hdr_get_to_ds(oal_uint8 *puc_header)
{
    return (oal_uint8)((mac_header_frame_control_stru *)(puc_header))->bit_to_ds;
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_tid_value_4addr(oal_uint8 *puc_header)
{
    return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_tid_value(oal_uint8 *puc_header, oal_bool_enum_uint8 en_is_4addr)
{
    if (en_is_4addr) {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
    } else {
        return (puc_header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x07); /* B0 - B2 */
    }
}


OAL_STATIC OAL_INLINE oal_uint16  mac_get_seq_num(oal_uint8 *puc_header)
{
    oal_uint16 us_seq_num;

    us_seq_num   = puc_header[23];
    us_seq_num <<= 4; /* puc_header[23]的第4位与puc_header[22]高4位拼成接受侦的seqence number */
    us_seq_num  |= (puc_header[22] >> 4);

    return us_seq_num;
}


OAL_STATIC OAL_INLINE oal_void  mac_set_seq_num(oal_uint8 *puc_header, oal_uint16 us_seq_num)
{
    puc_header[23]      = (oal_uint8)us_seq_num >> 4; /* puc_header[23]获取us_seq_num的高4位 */
    puc_header[22]      &= 0x0F; /* 保留puc_header[22]的低4位 */
    puc_header[22]      |= (oal_uint8)(us_seq_num << 4); /* us_seq_nume的低4位作为高位与puc_header[22]低4位拼接成8bit值 */
}


OAL_STATIC OAL_INLINE oal_uint16 mac_get_bar_start_seq_num(oal_uint8 *puc_payload)
{
    /* puc_payload[2]高4位作为低位，puc_payload[3]的低4位作为高位，拼接成一个8位的start seq num值 */
    return ((puc_payload[2] & 0xF0) >> 4) | (puc_payload[3] << 4);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_ack_policy_4addr(oal_uint8 *puc_header)
{
    return ((puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x60) >> 5); /* B5 - B6 */
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_ack_policy(oal_uint8 *puc_header, oal_bool_enum_uint8 en_is_4addr)
{
    if (en_is_4addr) {
        return ((puc_header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x60) >> 5); /* B5 - B6 */
    } else {
        return ((puc_header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x60) >> 5); /* B5 - B6 */
    }
}


OAL_STATIC OAL_INLINE oal_void mac_rx_get_da(mac_ieee80211_frame_stru   *pst_mac_header,
                                             oal_uint8                 **puc_da)
{
    /* IBSS、from AP */
    if (pst_mac_header->st_frame_control.bit_to_ds == 0) {
        *puc_da = pst_mac_header->auc_address1;
    } else { /* WDS、to AP */
        *puc_da = pst_mac_header->auc_address3;
    }
}


OAL_STATIC OAL_INLINE oal_void  mac_rx_get_sa(mac_ieee80211_frame_stru *pst_mac_header, oal_uint8 **puc_sa)
{
    /* IBSS、to AP */
    if (pst_mac_header->st_frame_control.bit_from_ds == 0) {
        *puc_sa = pst_mac_header->auc_address2;
    } else if ((pst_mac_header->st_frame_control.bit_from_ds == 1) &&
             (pst_mac_header->st_frame_control.bit_to_ds == 0)) { /* from AP */
        *puc_sa = pst_mac_header->auc_address3;
    } else { /* WDS */
        *puc_sa = ((mac_ieee80211_frame_addr4_stru *)pst_mac_header)->auc_address4;
    }
}


OAL_STATIC OAL_INLINE oal_void mac_get_transmit_addr(mac_ieee80211_frame_stru *pst_mac_header, oal_uint8 **puc_bssid)
{
    /* 对于IBSS, STA, AP, WDS 场景下，获取发送端地址 */
    *puc_bssid = pst_mac_header->auc_address2;
}


OAL_STATIC OAL_INLINE oal_void  mac_get_submsdu_len(oal_uint8 *puc_submsdu_hdr, oal_uint16 *pus_submsdu_len)
{
    *pus_submsdu_len = *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET);
    /* (*pus_submsdu_len)左移8位 */
    *pus_submsdu_len = (oal_uint16)((*pus_submsdu_len << 8) + *(puc_submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET + 1));
}


OAL_STATIC OAL_INLINE oal_void  mac_get_submsdu_pad_len(oal_uint16 us_msdu_len, oal_uint8 *puc_submsdu_pad_len)
{
    *puc_submsdu_pad_len = us_msdu_len & 0x3;

    if (*puc_submsdu_pad_len) {
        *puc_submsdu_pad_len = (MAC_BYTE_ALIGN_VALUE - *puc_submsdu_pad_len);
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_grp_addr(oal_uint8 *puc_addr)
{
    if ((puc_addr[0] & 1) != 0) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_ieeee80211_is_action(oal_uint8 *puc_header)
{
    /* WLAN_ACTION左移4位 */
    return (puc_header[0] & (MAC_IEEE80211_FCTL_FTYPE | MAC_IEEE80211_FCTL_STYPE)) == (WLAN_ACTION << 4);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_frame_type_and_subtype(oal_uint8 *puc_mac_header)
{
    return (puc_mac_header[0] & 0xFC);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_frame_sub_type(oal_uint8 *puc_mac_header)
{
    return (puc_mac_header[0] & 0xFC);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_addba_req_frame(oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_tid)
{
    oal_uint8 *puc_mac_header  = oal_netbuf_header(pst_netbuf);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8 *puc_mac_payload = oal_netbuf_data(pst_netbuf);
#else
    oal_uint8 *puc_mac_payload = puc_mac_header + MAC_80211_FRAME_LEN;
#endif

    /* Management frame */
    if (mac_get_frame_sub_type(puc_mac_header) == WLAN_FC0_SUBTYPE_ACTION) {
        if ((puc_mac_payload[0] == MAC_ACTION_CATEGORY_BA) && (puc_mac_payload[1] == MAC_BA_ACTION_ADDBA_REQ)) {
            *puc_tid     = (puc_mac_payload[3] & 0x3C) >> 2; /* 保留puc_mac_payload[3]的2-5位于低4位作为tid */
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_addba_rsp_frame(oal_netbuf_stru *pst_netbuf)
{
    oal_uint8 *puc_mac_header  = oal_netbuf_header(pst_netbuf);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8 *puc_mac_payload = oal_netbuf_data(pst_netbuf);
#else
    oal_uint8 *puc_mac_payload = puc_mac_header + MAC_80211_FRAME_LEN;
#endif

    /* Management frame */
    if (mac_get_frame_sub_type(puc_mac_header) == WLAN_FC0_SUBTYPE_ACTION) {
        if ((puc_mac_payload[0] == MAC_ACTION_CATEGORY_BA) && (puc_mac_payload[1] == MAC_BA_ACTION_ADDBA_RSP)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_delba_frame(oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_tid)
{
    oal_uint8 *puc_mac_header  = oal_netbuf_header(pst_netbuf);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8 *puc_mac_payload = oal_netbuf_data(pst_netbuf);
#else
    oal_uint8 *puc_mac_payload = puc_mac_header + MAC_80211_FRAME_LEN;
#endif

    /* Management frame */
    if (mac_get_frame_sub_type(puc_mac_header) == WLAN_FC0_SUBTYPE_ACTION) {
        if ((puc_mac_payload[0] == MAC_ACTION_CATEGORY_BA) && (puc_mac_payload[1] == MAC_BA_ACTION_DELBA)) {
            *puc_tid     = (puc_mac_payload[3] & 0xF0) >> 4; /* 保留puc_mac_payload[3]的4-7位于低4位作为tid */
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_get_frame_protected_flag(oal_uint8 *puc_mac_header)
{
    return ((puc_mac_header[1] & 0x40) ? OAL_TRUE : OAL_FALSE);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_frame_get_subtype_value(oal_uint8 *puc_mac_header)
{
    return ((puc_mac_header[0] & 0xF0) >> 4) ; /* puc_mac_header[0]的高4位右移4位到低位，作为802.11帧子类型的值(0~15) */
}


OAL_STATIC OAL_INLINE oal_uint8  mac_get_frame_type(oal_uint8 *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C);
}


OAL_STATIC OAL_INLINE oal_uint8  mac_frame_get_type_value(oal_uint8 *puc_mac_header)
{
    return (puc_mac_header[0] & 0x0C) >> 2; /* 获取80211帧帧类型，取值0~2 */
}


OAL_STATIC OAL_INLINE oal_uint16 mac_get_cap_info(oal_uint8 *mac_frame_body)
{
    oal_uint16  us_cap_info;
    oal_uint16  us_index        = 0;

    us_cap_info  = mac_frame_body[us_index];
    /* mac_frame_body[us_index]为低8位与mac_frame_body[us_index + 1]左移8位为高8位拼接成16位的capability fied */
    us_cap_info |= (mac_frame_body[us_index + 1] << 8);

    return us_cap_info;
}


OAL_STATIC OAL_INLINE oal_void mac_set_snap(oal_netbuf_stru *pst_buf, oal_uint16 us_ether_type, oal_uint8 uc_offset)
{
    mac_llc_snap_stru *pst_llc;
    oal_uint16         uc_use_btep1;
    oal_uint16         uc_use_btep2;

    /* LLC */
    pst_llc = (mac_llc_snap_stru *)(oal_netbuf_data(pst_buf) + uc_offset);
    pst_llc->uc_llc_dsap      = SNAP_LLC_LSAP;
    pst_llc->uc_llc_ssap      = SNAP_LLC_LSAP;
    pst_llc->uc_control       = LLC_UI;

    uc_use_btep1              = oal_byteorder_host_to_net_uint16(ETHER_TYPE_AARP);
    uc_use_btep2              = oal_byteorder_host_to_net_uint16(ETHER_TYPE_IPX);
    if (oal_unlikely((uc_use_btep1 == us_ether_type) || (uc_use_btep2 == us_ether_type))) {
        pst_llc->auc_org_code[0] = SNAP_BTEP_ORGCODE_0; /* 0x0 */
        pst_llc->auc_org_code[1] = SNAP_BTEP_ORGCODE_1; /* 0x0 */
        pst_llc->auc_org_code[2] = SNAP_BTEP_ORGCODE_2; /* auc_org_code第2byte 0xf8 */
    } else {
        pst_llc->auc_org_code[0]  = SNAP_RFC1042_ORGCODE_0;  /* 0x0 */
        pst_llc->auc_org_code[1]  = SNAP_RFC1042_ORGCODE_1;  /* 0x0 */
        pst_llc->auc_org_code[2]  = SNAP_RFC1042_ORGCODE_2;  /* auc_org_code第2byte 0x0 */
    }

    pst_llc->us_ether_type = us_ether_type;

    oal_netbuf_pull(pst_buf, uc_offset);
}


OAL_STATIC OAL_INLINE oal_uint16  mac_get_auth_alg(oal_uint8 *puc_mac_hdr)
{
    oal_uint16 us_auth_alg;

    us_auth_alg = puc_mac_hdr[MAC_80211_FRAME_LEN + 1];
    /* puc_mac_hdr[MAC_80211_FRAME_LEN + 1]高8位与puc_mac_hdr[MAC_80211_FRAME_LEN + 0]低8位拼接成16位认证状态字段 */
    us_auth_alg = (oal_uint16)((us_auth_alg << 8) | puc_mac_hdr[MAC_80211_FRAME_LEN + 0]);

    return us_auth_alg;
}


OAL_STATIC OAL_INLINE oal_uint16  mac_get_auth_status(oal_uint8 *puc_mac_hdr)
{
    oal_uint16 us_auth_status;

    us_auth_status = puc_mac_hdr[MAC_80211_FRAME_LEN + 5]; /* puc_mac_hdr[MAC_80211_FRAME_LEN]偏移5byte赋值auth_status */
    /* puc_mac_hdr[MAC_80211_FRAME_LEN + 5]高8位与puc_mac_hdr[MAC_80211_FRAME_LEN + 4]低8位拼接成16位认证状态字段 */
    us_auth_status = (oal_uint16)((us_auth_status << 8) | puc_mac_hdr[MAC_80211_FRAME_LEN + 4]);

    return us_auth_status;
}


OAL_STATIC OAL_INLINE oal_uint16  mac_get_auth_seq_num(oal_uint8 *puc_mac_hdr)
{
    oal_uint16 us_auth_seq;

    us_auth_seq = puc_mac_hdr[MAC_80211_FRAME_LEN + 3]; /* puc_mac_hdr[MAC_80211_FRAME_LEN]偏移3byte赋值auth_seq */
    /* puc_mac_hdr[MAC_80211_FRAME_LEN + 3]高8位与puc_mac_hdr[MAC_80211_FRAME_LEN + 2]低8位拼接成16位认证帧序列号 */
    us_auth_seq = (oal_uint16)((us_auth_seq << 8) | puc_mac_hdr[MAC_80211_FRAME_LEN + 2]);

    return us_auth_seq;
}


OAL_STATIC OAL_INLINE oal_void  mac_set_wep(oal_uint8 *puc_hdr, oal_uint8 uc_wep)
{
    puc_hdr[1] &= 0xBF;
    puc_hdr[1] |= (oal_uint8)(uc_wep << 6); /* 设置帧的保护字段（puc_hdr[1]的第6位） */
}


OAL_STATIC OAL_INLINE oal_void  mac_set_protectedframe(oal_uint8 *puc_mac_hdr)
{
    puc_mac_hdr[1] |= 0x40;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_get_protectedframe(oal_uint8 *puc_mac_hdr)
{
    mac_ieee80211_frame_stru *pst_mac_hdr = OAL_PTR_NULL;
    pst_mac_hdr = (mac_ieee80211_frame_stru *)puc_mac_hdr;

    return (oal_bool_enum_uint8)(pst_mac_hdr->st_frame_control.bit_protected_frame);
}



OAL_STATIC OAL_INLINE oal_uint8  mac_is_protectedframe(oal_uint8 *puc_mac_hdr)
{
    return ((puc_mac_hdr[1] & 0x40) >> 6); /* 获取帧控制字段的受保护字段（puc_hdr[1]的第6位） */
}


OAL_STATIC OAL_INLINE oal_uint16  mac_get_auth_algo_num(oal_netbuf_stru *pst_netbuf)
{
    oal_uint16 us_auth_algo;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8 *puc_mac_payload = oal_netbuf_data(pst_netbuf) + MAC_80211_FRAME_LEN;
#else
    oal_uint8 *puc_mac_payload = oal_netbuf_header(pst_netbuf) + MAC_80211_FRAME_LEN;
#endif

    us_auth_algo = puc_mac_payload[1];
    /* puc_mac_payload[1]为高8位与puc_mac_payload[0]为低8位拼接成16认证帧的认证算法 */
    us_auth_algo = (oal_uint16)((us_auth_algo << 8) | puc_mac_payload[0]);

    return us_auth_algo;
}


OAL_STATIC OAL_INLINE oal_uint8  *mac_get_auth_ch_text(oal_uint8 *puc_mac_hdr)
{
    return &(puc_mac_hdr[MAC_80211_FRAME_LEN + 6]); /* 获取认证帧中challenge txt(puc_mac_hdr[MAC_80211_FRAME_LEN + 6]) */
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  mac_is_4addr(oal_uint8 *puc_mac_hdr)
{
    oal_uint8               uc_is_tods;
    oal_uint8               uc_is_from_ds;
    oal_bool_enum_uint8     en_is_4addr;

    uc_is_tods    = mac_hdr_get_to_ds(puc_mac_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds(puc_mac_hdr);

    en_is_4addr   = uc_is_tods && uc_is_from_ds;

    return en_is_4addr;
}
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

OAL_STATIC OAL_INLINE oal_void  mac_get_address1(oal_uint8 *puc_mac_hdr, oal_uint8 *puc_addr, oal_uint8 len)
{
    /* 拷贝地址1(跳过head 4字节) */
    if (memcpy_s(puc_addr, WLAN_MAC_ADDR_LEN, puc_mac_hdr + 4, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}


OAL_STATIC OAL_INLINE oal_void mac_get_address2(oal_uint8 *puc_mac_hdr, oal_uint8 *puc_addr, oal_uint8 len)
{
    /* 拷贝地址2(跳过head与地址1 10字节) */
    if (memcpy_s(puc_addr, WLAN_MAC_ADDR_LEN, puc_mac_hdr + 10, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}

OAL_STATIC OAL_INLINE oal_void  mac_get_address3(oal_uint8 *puc_mac_hdr, oal_uint8 *puc_addr, oal_uint8 len)
{
    /* 拷贝地址3(跳过head、地址1、2 16字节) */
    if (memcpy_s(puc_addr, WLAN_MAC_ADDR_LEN, puc_mac_hdr + 16, WLAN_MAC_ADDR_LEN) != EOK) {
        return;
    }
}

OAL_STATIC OAL_INLINE oal_void  mac_get_qos_ctrl(oal_uint8 *puc_mac_hdr, oal_uint8 *puc_qos_ctrl)
{
    if (mac_is_4addr(puc_mac_hdr) != OAL_TRUE) {
        if (memcpy_s(puc_qos_ctrl, MAC_QOS_CTL_LEN, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET, MAC_QOS_CTL_LEN) != EOK) {
        }
        return;
    }
    memcpy_s(puc_qos_ctrl, MAC_QOS_CTL_LEN, puc_mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET_4ADDR, MAC_QOS_CTL_LEN);
    return;
}

OAL_STATIC OAL_INLINE oal_void mac_get_bssid(oal_uint8 *puc_mac_hdr, oal_uint8 *puc_bssid, oal_uint8 len)
{
    if (mac_hdr_get_from_ds(puc_mac_hdr) == 1) {
        mac_get_address2(puc_mac_hdr, puc_bssid, len);
    } else if (mac_hdr_get_to_ds(puc_mac_hdr) == 1) {
        mac_get_address1(puc_mac_hdr, puc_bssid, len);
    } else {
        mac_get_address3(puc_mac_hdr, puc_bssid, len);
    }
}
#endif

OAL_STATIC OAL_INLINE mac_status_code_enum_uint16  mac_get_asoc_status(oal_uint8 *puc_mac_payload)
{
    mac_status_code_enum_uint16          en_asoc_status;
    /* puc_mac_payload[3]高8位与puc_mac_payload[2]低8位拼接成16位关联帧中的状态信息 */
    en_asoc_status = (puc_mac_payload[3] << 8) | puc_mac_payload[2];

    return en_asoc_status;
}


OAL_STATIC OAL_INLINE oal_uint16 mac_get_asoc_id(oal_uint8 *puc_mac_payload)
{
    oal_uint16 us_asoc_id;
    /* puc_mac_payload[5]高8位与puc_mac_payload[4]低8位拼接成16位关联帧中的关联ID */
    us_asoc_id = puc_mac_payload[4] | (puc_mac_payload[5] << 8) ;
    us_asoc_id  &= 0x3FFF; /* 取低14位 */

    return us_asoc_id;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_frame_is_eapol(mac_llc_snap_stru *pst_mac_llc_snap)
{
    return (oal_bool_enum_uint8)(oal_byteorder_host_to_net_uint16(pst_mac_llc_snap->us_ether_type) ==
            ETHER_ONE_X_TYPE);
}

extern oal_uint16 mac_get_rsn_capability(const oal_uint8 *puc_rsn_ie);

oal_void mac_add_app_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint16 *pus_ie_len,
                        en_app_ie_type_uint8 en_type);
oal_void mac_add_wps_ie(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint16 *pus_ie_len,
                        en_app_ie_type_uint8 en_type);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_void mac_set_opmode_field(oal_uint8 *pst_vap, oal_uint8 *puc_buffer);
#endif
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
extern wlan_pmf_cap_status_uint8  mac_get_pmf_cap(oal_uint8 *puc_ie, oal_uint32 ul_ie_len);
#endif
extern oal_uint16  mac_encap_2040_coext_mgmt(oal_void *pst_mac_vap, oal_netbuf_stru *pst_buffer,
                                             oal_uint8 uc_coext_info, oal_uint32 ul_chan_report);

extern oal_void    mac_set_rrm_enabled_cap_field_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);

#ifdef _PRE_WLAN_FEATURE_FTM
extern oal_uint8  *mac_find_ie_ext_ie(oal_uint8 uc_eid, oal_uint8 uc_ext_ie, oal_uint8 *puc_ies, oal_int32 l_len);
extern oal_bool_enum_uint8 mac_is_ftm_related_frame(oal_netbuf_stru *pst_netbuf);
oal_bool_enum_uint8 mac_is_ftm_frame(oal_netbuf_stru *pst_netbuf);
extern oal_bool_enum_uint8 mac_is_ftm_req_frame(oal_netbuf_stru *pst_netbuf);
#endif
extern oal_void mac_set_ext_capabilities_ie_cb(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_vht_capabilities_ie_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_void mac_set_ht_capabilities_ie_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
extern oal_rom_cb_result_enum_uint8 mac_set_rsn_ie_cb(oal_void *pst_vap, oal_uint8 *puc_pmkid, oal_uint8 *puc_buffer,
                                                      oal_uint8 *puc_ie_len, oal_uint32 *pul_ret);
extern oal_rom_cb_result_enum_uint8 mac_set_wpa_ie_cb(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len,
                                                      oal_uint32 *pul_ret);
extern oal_uint32 mac_get_11h_test(oal_void);
extern oal_void mac_set_11h_test(oal_uint32 ul_11h_test);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_frame.h */
