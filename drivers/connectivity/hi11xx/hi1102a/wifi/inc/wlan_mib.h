

#ifndef __WLAN_MIB_H__
#define __WLAN_MIB_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_MIB_TOKEN_STRING_MAX_LENGTH 32 /* 支持与网管兼容的token的字符串最长长度，此定义后续可能与版本相关 */
#define WLAN_HT_MCS_BITMASK_LEN          10 /* MCS bitmask长度为77位，加上3个保留位 */

/* Number of Cipher Suites Implemented */
#define WLAN_PAIRWISE_CIPHER_SUITES OAL_NL80211_MAX_NR_CIPHER_SUITES
#define WLAN_AUTHENTICATION_SUITES  OAL_NL80211_MAX_NR_AKM_SUITES

#define MAC_PAIRWISE_CIPHER_SUITES_NUM OAL_NL80211_MAX_NR_CIPHER_SUITES
#define MAC_AUTHENTICATION_SUITE_NUM   OAL_NL80211_MAX_NR_AKM_SUITES

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* RowStatus ::= TEXTUAL-CONVENTION */
/* The status column has six  values:`active', `notInService',`notReady', */
/* `createAndGo', `createAndWait', `destroy'  as described in   rfc2579 */
typedef enum {
    WLAN_MIB_ROW_STATUS_ACTIVE = 1,
    WLAN_MIB_ROW_STATUS_NOT_INSERVICE,
    WLAN_MIB_ROW_STATUS_NOT_READY,
    WLAN_MIB_ROW_STATUS_CREATE_AND_GO,
    WLAN_MIB_ROW_STATUS_CREATE_AND_WAIT,
    WLAN_MIB_ROW_STATUS_DEATROY,

    WLAN_MIB_ROW_STATUS_BUTT
} wlan_mib_row_status_enum;
typedef oal_uint8 wlan_mib_row_status_enum_uint8;

typedef enum {
    WLAN_MIB_PWR_MGMT_MODE_ACTIVE = 1,
    WLAN_MIB_PWR_MGMT_MODE_PWRSAVE = 2,

    WLAN_MIB_PWR_MGMT_MODE_BUTT
} wlan_mib_pwr_mgmt_mode_enum;
typedef oal_uint8 wlan_mib_pwr_mgmt_mode_enum_uint8;

/* 根据802.11-2016 9.4.1.1 Authentication Algorithm Number field 重新定义认证算法枚举 */
typedef enum {
    WLAN_MIB_AUTH_ALG_OPEN_SYS = 0,
    WLAN_MIB_AUTH_ALG_SHARE_KEY = 1,
    WLAN_MIB_AUTH_ALG_FT = 2,
    WLAN_MIB_AUTH_ALG_SAE = 3,

    WLAN_MIB_AUTH_ALG_BUTT
} wlan_mib_auth_alg_enum;
typedef oal_uint8 wlan_mib_auth_alg_enum_uint8;

typedef enum {
    WLAN_MIB_DESIRED_BSSTYPE_INFRA = 1,
    WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT = 2,
    WLAN_MIB_DESIRED_BSSTYPE_ANY = 3,

    WLAN_MIB_DESIRED_BSSTYPE_BUTT
} wlan_mib_desired_bsstype_enum;
typedef oal_uint8 wlan_mib_desired_bsstype_enum_uint8;

typedef enum {
    WLAN_MIB_RSNACFG_GRPREKEY_DISABLED = 1,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEBASED = 2,
    WLAN_MIB_RSNACFG_GRPREKEY_PACKETBASED = 3,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEPACKETBASED = 4,

    WLAN_MIB_RSNACFG_GRPREKEY_BUTT
} wlan_mib_rsna_cfg_grp_rekey_enum;
typedef oal_uint8 wlan_mib_rsna_cfg_grp_rekey_enum_uint8;

typedef enum {
    WLAN_MIB_RMRQST_TYPE_CH_LOAD = 3,
    WLAN_MIB_RMRQST_TYPE_NOISE_HISTOGRAM = 4,
    WLAN_MIB_RMRQST_TYPE_BEACON = 5,
    WLAN_MIB_RMRQST_TYPE_FRAME = 6,
    WLAN_MIB_RMRQST_TYPE_STA_STATISTICS = 7,
    WLAN_MIB_RMRQST_TYPE_LCI = 8,
    WLAN_MIB_RMRQST_TYPE_TRANS_STREAM = 9,
    WLAN_MIB_RMRQST_TYPE_PAUSE = 255,

    WLAN_MIB_RMRQST_TYPE_BUTT
} wlan_mib_rmrqst_type_enum;
typedef oal_uint16 wlan_mib_rmrqst_type_enum_uint16;

/* dot11RMRqstBeaconRqstMode INTEGER{  passive(0), active(1),beaconTable(2) */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_MODE_PASSIVE = 0,
    WLAN_MIB_RMRQST_BEACONRQST_MODE_ACTIVE = 1,
    WLAN_MIB_RMRQST_BEACON_MODE_BEACON_TABLE = 2,

    WLAN_MIB_RMRQST_BEACONRQST_MODE_BUTT
} wlan_mib_rmrqst_beaconrqst_mode_enum;
typedef oal_uint8 wlan_mib_rmrqst_beaconrqst_mode_enum_uint8;

/* dot11RMRqstBeaconRqstDetail INTEGER {noBody(0),fixedFieldsAndRequestedElements(1),allBody(2) } */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_NOBODY = 0,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_FIXED_FLDANDELMT = 1,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_ALLBODY = 2,

    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_BUTT
} wlan_mib_rmrqst_beaconrqst_detail_enum;
typedef oal_uint8 wlan_mib_rmrqst_beaconrqst_detail_enum_uint8;

/* dot11RMRqstFrameRqstType INTEGER { frameCountRep(1) } */
typedef enum {
    WLAN_MIB_RMRQST_FRAMERQST_TYPE_FRAME_COUNTREP = 1,

    WLAN_MIB_RMRQST_FRAMERQST_TYPE_BUTT
} wlan_mib_rmrqst_famerqst_type_enum;
typedef oal_uint8 wlan_mib_rmrqst_famerqst_type_enum_uint8;

typedef enum {
    WLAN_MIB_RMRQST_BEACONRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_ABS_THRESHOLD = 1,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_ABS_THRESHOLD = 2,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_ABS_THRESHOLD = 3,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_ABS_THRESHOLD = 4,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_OFFSET_THRESHOLD = 5,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_OFFSET_THRESHOLD = 6,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_OFFSET_THRESHOLD = 7,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_OFFSET_THRESHOLD = 8,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_IN_BOUND = 9,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_IN_BOUND = 10,

    WLAN_MIB_RMRQST_BEACONRPT_CDT_BUTT
} wlan_mib_rmrqst_beaconrpt_cdt_enum;
typedef oal_uint8 wlan_mib_rmrqst_beaconrpt_cdt_enum_uint8;

/* dot11RMRqstSTAStatRqstGroupID OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* dot11CountersTable(0),dot11MacStatistics(1), */
/* dot11QosCountersTableforUP0(2),dot11QosCountersTableforUP1(3), */
/* dot11QosCountersTableforUP2(4),dot11QosCountersTableforUP3(5), */
/* dot11QosCountersTableforUP4(6),dot11QosCountersTableforUP5(7), */
/* dot11QosCountersTableforUP6(8),dot11QosCountersTableforUP7(9), */
/* bSSAverageAccessDelays(10),dot11CountersGroup3Tablefor31(11), */
/* dot11CountersGroup3Tablefor32(12),dot11CountersGroup3Tablefor33(13), */
/* dot11CountersGroup3Tablefor34(14),dot11CountersGroup3Tablefor35(15), */
/* dot11RSNAStatsTable(16)} */
typedef enum {
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_TABLE = 0,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_MAC_STATS = 1,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP0 = 2,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP1 = 3,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP2 = 4,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP3 = 5,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP4 = 6,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP5 = 7,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP6 = 8,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP7 = 9,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BSS_AVERG_ACCESS_DELAY = 10,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR31 = 11,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR32 = 12,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR33 = 13,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR34 = 14,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR35 = 15,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_RSNA_STATS_TABLE = 16,

    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BUTT
} wlan_mib_rmrqst_stastatrqst_grpid_enum;
typedef oal_uint8 wlan_mib_rmrqst_stastatrqst_grpid_enum_uint8;

/* dot11RMRqstLCIRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER { local(0), remote(1) } */
typedef enum {
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_LOCAL = 0,
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_REMOTE = 1,

    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_BUTT
} wlan_mib_rmrqst_lcirpt_subject_enum;
typedef oal_uint8 wlan_mib_rmrqst_lcirpt_subject_enum_uint8;

/* dot11RMRqstLCIAzimuthType OBJECT-TYPE */
/* SYNTAX INTEGER { frontSurfaceofSta(0), radioBeam(1) } */
typedef enum {
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_BUTT
} wlan_mib_rmrqst_lciazimuth_type_enum;
typedef oal_uint8 wlan_mib_rmrqst_lciazimuth_type_enum_uint8;

/* dot11RMRqstChannelLoadReportingCondition OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* afterEveryMeasurement(0), */
/* chanLoadAboveReference(1), */
/* chanLoadBelowReference(2) } */
typedef enum {
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_BELOW_REF = 2,

    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_BUTT
} wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum;
typedef oal_uint8 wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum_uint8;

/* dot11RMRqstNoiseHistogramReportingCondition OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* afterEveryMeasurement(0), */
/* aNPIAboveReference(1), */
/* aNPIBelowReference(2) } */
typedef enum {
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_BELOW_REF = 2,

    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_BUTT
} wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum;
typedef oal_uint8 wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum_uint8;

/* dot11LCIDSEAltitudeType OBJECT-TYPE */
/* SYNTAX INTEGER { meters(1), floors(2), hagm(3) } */
typedef enum {
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_METERS = 1,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_FLOORS = 2,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_HAGM = 3,

    WLAN_MIB_LCI_DSEALTITUDE_TYPE_BUTT
} wlan_mib_lci_dsealtitude_type_enum;
typedef oal_uint8 wlan_mib_lci_dsealtitude_type_enum_uint8;

/* dot11MIMOPowerSave OBJECT-TYPE */
/* SYNTAX INTEGER { static(1), dynamic(2), mimo(3) } */
typedef enum {
    WLAN_MIB_MIMO_POWER_SAVE_STATIC = 1,
    WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC = 2,
    WLAN_MIB_MIMO_POWER_SAVE_MIMO = 3,

    WLAN_MIB_MIMO_POWER_SAVE_BUTT
} wlan_mib_mimo_power_save_enum;
typedef oal_uint8 wlan_mib_mimo_power_save_enum_uint8;

/* dot11MaxAMSDULength OBJECT-TYPE */
/* SYNTAX INTEGER { short(3839), long(7935) } */
typedef enum {
    WLAN_MIB_MAX_AMSDU_LENGTH_SHORT = 3839,
    WLAN_MIB_MAX_AMSDU_LENGTH_LONG = 7935,

    WLAN_MIB_MAX_AMSDU_LENGTH_BUTT
} wlan_mib_max_amsdu_lenth_enum;
typedef oal_uint16 wlan_mib_max_amsdu_lenth_enum_uint16;

/* dot11MCSFeedbackOptionImplemented OBJECT-TYPE */
/* SYNTAX INTEGER { none(0), unsolicited (2), both (3) } */
typedef enum {
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_NONE = 0,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_UNSOLICITED = 2,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BOTH = 3,

    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BUTT
} wlan_mib_mcs_feedback_opt_implt_enum;
typedef oal_uint8 wlan_mib_mcs_feedback_opt_implt_enum_uint8;

/* dot11LocationServicesLIPReportIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* hours(0), */
/* minutes(1), */
/* seconds(2), */
/* milliseconds(3) */
typedef enum {
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_HOURS = 0,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MINUTES = 1,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_SECONDS = 2,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MILLISECDS = 3,

    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_BUTT
} wlan_mib_lct_servs_liprpt_interval_unit_enum;
typedef oal_uint8 wlan_mib_lct_servs_liprpt_interval_unit_enum_uint8;

/* dot11WirelessMGTEventType OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* transition(0), */
/* rsna(1), */
/* peerToPeer(2), */
/* wnmLog(3), */
/* vendorSpecific(221) */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_BUTT
} wlan_mib_wireless_mgt_event_type_enum;
typedef oal_uint8 wlan_mib_wireless_mgt_event_type_enum_uint8;

/* dot11WirelessMGTEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* successful(0), */
/* requestFailed(1), */
/* requestRefused(2), */
/* requestIncapable(3), */
/* detectedFrequentTransition(4) */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_BUTT
} wlan_mib_wireless_mgt_event_status_enum;
typedef oal_uint8 wlan_mib_wireless_mgt_event_status_enum_uint8;

/* dot11WirelessMGTEventTransitionReason OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* unspecified(0), */
/* excessiveFrameLossRatesPoorConditions(1), */
/* excessiveDelayForCurrentTrafficStreams(2), */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4), */
/* loadBalancing(5), */
/* betterApFound(6), */
/* deauthenticatedDisassociatedFromPreviousAp(7), */
/* certificateToken(8), */
/* apFailedIeee8021XEapAuthentication(9), */
/* apFailed4wayHandshake(10), */
/* excessiveDataMICFailures(11), */
/* exceededFrameTransmissionRetryLimit(12), */
/* ecessiveBroadcastDisassociations(13), */
/* excessiveBroadcastDeauthentications(14), */
/* previousTransitionFailed(15) */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_CERTIF_TOKEN = 8,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 9,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 10,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DATA_MIC_FAIL = 11,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCEED_FRAME_TRANS_RETRY_LIMIT = 12,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISASSO = 13,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISAUTH = 14,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,

    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BUTT
} wlan_mib_wireless_mgt_event_transit_reason_enum;
typedef oal_uint8 wlan_mib_wireless_mgt_event_transit_reason_enum_uint8;

/* dot11WNMRqstType OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* mcastDiagnostics(0), */
/* locationCivic(1), */
/* locationIdentifier(2), */
/* event(3), */
/* dignostic(4), */
/* locationConfiguration(5), */
/* bssTransitionQuery(6), */
/* bssTransitionRqst(7), */
/* fms(8), */
/* colocInterference(9) */
typedef enum {
    WLAN_MIB_WNM_RQST_TYPE_MCAST_DIAG = 0,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CIVIC = 1,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_IDTIF = 2,
    WLAN_MIB_WNM_RQST_TYPE_EVENT = 3,
    WLAN_MIB_WNM_RQST_TYPE_DIAGOSTIC = 4,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CFG = 5,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_QUERY = 6,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_RQST = 7,
    WLAN_MIB_WNM_RQST_TYPE_FMS = 8,
    WLAN_MIB_WNM_RQST_TYPE_COLOC_INTERF = 9,

    WLAN_MIB_WNM_RQST_TYPE_BUTT
} wlan_mib_wnm_rqst_type_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_type_enum_uint8;

/* dot11WNMRqstLCRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* local(0), */
/* remote(1) */
typedef enum {
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lcrrqst_subj_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_lcrrqst_subj_enum_uint8;

/* dot11WNMRqstLCRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* seconds(0), */
/* minutes(1), */
/* hours(2) */
typedef enum {
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lcr_interval_unit_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_lcr_interval_unit_enum_uint8;

/* dot11WNMRqstLIRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* local(0), */
/* remote(1) */
typedef enum {
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lirrqst_subj_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_lirrqst_subj_enum_uint8;

/* dot11WNMRqstLIRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* seconds(0), */
/* minutes(1), */
/* hours(2) */
typedef enum {
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lir_interval_unit_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_lir_interval_unit_enum_uint8;

/* dot11WNMRqstEventType OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* transition(0), */
/* rsna(1), */
/* peerToPeer(2), */
/* wnmLog(3), */
/* vendorSpecific(221) */
typedef enum {
    WLAN_MIB_WNM_RQST_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_EVENT_TYPE_BUTT
} wlan_mib_wnm_rqst_event_tpye_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_event_type_enum_uint8;

/* dot11WNMRqstDiagType OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* cancelRequest(0), */
/* manufacturerInfoStaRep(1), */
/* configurationProfile(2), */
/* associationDiag(3), */
/* ieee8021xAuthDiag(4), */
/* vendorSpecific(221) */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CANCEL_RQST = 0,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_MANUFCT_INFO_STA_REP = 1,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CFG_PROFILE = 2,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_ASSO_DIAG = 3,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_IEEE8021X_AUTH_DIAG = 4,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_DIAG_TYPE_BUTT
} wlan_mib_wnm_rqst_diag_type_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_diag_type_enum_uint8;

/* dot11WNMRqstDiagCredentials OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* none(0), */
/* preSharedKey(1), */
/* usernamePassword(2), */
/* x509Certificate(3), */
/* otherCertificate(4), */
/* oneTimePassword(5), */
/* token(6) */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_NONT = 0,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_PRE_SHAREKEY = 1,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_USERNAME_PASSWORD = 2,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_X509_CTERTIFICATE = 3,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_OTHER_CTERTIFICATE = 4,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_ONE_TIME_PASSWORD = 5,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_TOKEN = 6,

    WLAN_MIB_WNM_RQST_DIAG_CREDENT_BUTT
} wlan_mib_wnm_rqst_diag_credent_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_diag_credent_enum_uint8;

/* dot11WNMRqstBssTransitQueryReason OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* unspecified(0), */
/* excessiveFrameLossRatesPoorConditions(1), */
/* excessiveDelayForCurrentTrafficStreams(2), */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4), */
/* loadBalancing(5), */
/* betterApFound(6), */
/* deauthenticatedDisassociatedFromPreviousAp(7), */
/* apFailedIeee8021XEapAuthentication(8), */
/* apFailed4wayHandshake(9), */
/* receivedTooManyReplayCounterFailures(10), */
/* receivedTooManyDataMICFailures(11), */
/* exceededMaxNumberOfRetransmissions(12), */
/* receivedTooManyBroadcastDisassociations(13), */
/* receivedTooManyBroadcastDeauthentications(14), */
/* previousTransitionFailed(15), */
/* lowRSSI(16) */
typedef enum {
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BUTT
} wlan_mib_wnm_rqst_bss_transit_query_reason_enum;
typedef oal_uint8 wlan_mib_wnm_rqst_bss_transit_query_reason_enum_uint8;

/* dot11WNMEventTransitRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* successful(0), */
/* requestFailed(1), */
/* requestRefused(2), */
/* requestIncapable(3), */
/* detectedFrequentTransition(4) */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_transit_rprt_event_status_enum;
typedef oal_uint8 wlan_mib_wnm_event_transit_rprt_event_status_enum_uint8;

/* dot11WNMEventTransitRprtTransitReason OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* unspecified(0), */
/* excessiveFrameLossRatesPoorConditions(1), */
/* excessiveDelayForCurrentTrafficStreams(2), */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4), */
/* loadBalancing(5), */
/* betterApFound(6), */
/* deauthenticatedDisassociatedFromPreviousAp(7), */
/* apFailedIeee8021XEapAuthentication(8), */
/* apFailed4wayHandshake(9), */
/* receivedTooManyReplayCounterFailures(10), */
/* receivedTooManyDataMICFailures(11), */
/* exceededMaxNumberOfRetransmissions(12), */
/* receivedTooManyBroadcastDisassociations(13), */
/* receivedTooManyBroadcastDeauthentications(14), */
/* previousTransitionFailed(15), */
/* lowRSSI(16) */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BUTT
} wlan_mib_wnm_event_transitrprt_transit_reason_enum;
typedef oal_uint8 wlan_mib_wnm_event_transitrprt_transit_reason_enum_uint8;

/* dot11WNMEventRsnaRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* successful(0), */
/* requestFailed(1), */
/* requestRefused(2), */
/* requestIncapable(3), */
/* detectedFrequentTransition(4) */
typedef enum {
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_rsnarprt_event_status_enum;
typedef oal_uint8 wlan_mib_wnm_event_rsnarprt_event_status_enum_uint8;

/* dot11APLCIDatum OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* wgs84 (1), */
/* nad83navd88 (2), */
/* nad93mllwvd (3) */
typedef enum {
    WLAN_MIB_AP_LCI_DATUM_WGS84 = 1,
    WLAN_MIB_AP_LCI_DATUM_NAD83_NAVD88 = 2,
    WLAN_MIB_AP_LCI_DATUM_NAD93_MLLWVD = 3,

    WLAN_MIB_AP_LCI_DATUM_BUTT
} wlan_mib_ap_lci_datum_enum;
typedef oal_uint8 wlan_mib_ap_lci_datum_enum_uint8;

/* dot11APLCIAzimuthType OBJECT-TYPE */
/* SYNTAX INTEGER { */
/* frontSurfaceOfSTA(0), */
/* radioBeam(1) } */
typedef enum {
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_BUTT
} wlan_mib_ap_lci_azimuth_type_enum;
typedef oal_uint8 wlan_mib_ap_lci_azimuth_type_enum_uint8;

/* dot11HTProtection 枚举定义 */
typedef enum {
    WLAN_MIB_HT_NO_PROTECTION = 0,
    WLAN_MIB_HT_NONMEMBER_PROTECTION = 1,
    WLAN_MIB_HT_20MHZ_PROTECTION = 2,
    WLAN_MIB_HT_NON_HT_MIXED = 3,

    WLAN_MIB_HT_PROTECTION_BUTT
} wlan_mib_ht_protection_enum;
typedef oal_uint8 wlan_mib_ht_protection_enum_uint8;

/* VHT Capabilities Info field 最大MPDU长度枚举 */
typedef enum {
    WLAN_MIB_VHT_MPDU_3895 = 0,
    WLAN_MIB_VHT_MPDU_7991 = 1,
    WLAN_MIB_VHT_MPDU_11454 = 2,

    WLAN_MIB_VHT_MPDU_LEN_BUTT
} wlan_mib_vht_mpdu_len_enum;
typedef oal_uint8 wlan_mib_vht_mpdu_len_enum_uint8;

/* VHT Capabilites Info field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_SUPP_WIDTH_80 = 0,       /* 不支持160或者80+80 */
    WLAN_MIB_VHT_SUPP_WIDTH_160 = 1,      /* 支持160 */
    WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80 = 2, /* 支持160和80+80 */

    WLAN_MIB_VHT_SUPP_WIDTH_BUTT
} wlan_mib_vht_supp_width_enum;
typedef oal_uint8 wlan_mib_vht_supp_width_enum_uint8;

/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef enum {
    /************************************************************************
        第一段 MIB 类配置
    *************************************************************************/
    /**********************dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }**************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    WLAN_CFGID_STATION_ID = 0,      /* dot11StationID MacAddress, MAC地址 */
    WLAN_CFGID_BSS_TYPE = 4,        /* dot11DesiredBSSType INTEGER, */
    WLAN_CFGID_SSID = 5,            /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    WLAN_CFGID_BEACON_INTERVAL = 6, /* dot11BeaconPeriod Unsigned32, */
    WLAN_CFGID_DTIM_PERIOD = 7,     /* dot11DTIMPeriod Unsigned32, */
    WLAN_CFGID_UAPSD_EN = 11,       /* dot11APSDOptionImplemented TruthValue, */
    WLAN_CFGID_SMPS_MODE = 12,      /* dot11MIMOPowerSave INTEGER, */
    WLAN_CFGID_SMPS_EN = 13,

    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    /************************dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 } **************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    /* --  dot11CountersTable ::= { dot11mac 2 } */
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    WLAN_CFGID_EDCA_TABLE_CWMIN = 41,         /* dot11EDCATableCWmin Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_CWMAX = 42,         /* dot11EDCATableCWmax Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_AIFSN = 43,         /* dot11EDCATableAIFSN Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT = 44,    /* dot11EDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME = 45, /* dot11EDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MANDATORY = 46,     /* dot11EDCATableMandatory TruthValue */

    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    WLAN_CFGID_QEDCA_TABLE_CWMIN = 51,         /* dot11QAPEDCATableCWmin Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_CWMAX = 52,         /* dot11QAPEDCATableCWmax Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_AIFSN = 53,         /* dot11QAPEDCATableAIFSN Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT = 54,    /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME = 55, /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MANDATORY = 56,     /* dot11QAPEDCATableMandatory TruthValue */

    /*************************dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 } **************************/
    /*************************dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 } *************************/
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    WLAN_CFGID_SHORT_PREAMBLE = 60, /* dot11ShortPreambleOptionImplemented TruthValue */

    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    WLAN_CFGID_SHORTGI = 80,        /* dot11ShortGIOptionInTwentyActivated TruthValue */
    WLAN_CFGID_SHORTGI_FORTY = 81,  /* dot11ShortGIOptionInFortyActivated TruthValue */
    WLAN_CFGID_CURRENT_CHANEL = 82, /* dot11CurrentChannel Unsigned32 */

    /* --  dot11PhyVHTTable  ::= { dot11phy 23 }  -- */
    WLAN_CFGID_SHORTGI_EIGHTY = 90, /* dot11VHTShortGIOptionIn80Activated TruthValue */

    /************************dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } ********************/
    /************************dot11imt         OBJECT IDENTIFIER ::= {ieee802dot11 6} **********************/
    /************************dot11MSGCF       OBJECT IDENTIFIER ::= { ieee802dot11 7} *********************/
    /************************************************************************
        第二段 非MIB 类配置
    *************************************************************************/
    /* 以下为ioctl下发的命令 */
    WLAN_CFGID_ADD_VAP = 100,    /* 创建VAP */
    WLAN_CFGID_START_VAP = 101,  /* 启用VAP */
    WLAN_CFGID_DEL_VAP = 102,    /* 删除VAP */
    WLAN_CFGID_DOWN_VAP = 103,   /* 停用VAP */
    WLAN_CFGID_MODE = 105,       /* 模式: 包括协议 频段 带宽 */
    WLAN_CFGID_CONCURRENT = 106, /* 设置并发用户数 */
    WLAN_CFGID_PROT_MODE = 108, /* 保护模式 */
    WLAN_CFGID_AUTH_MODE = 109, /* 认证模式 */
    WLAN_CFGID_NO_BEACON = 110,
    WLAN_CFGID_TX_CHAIN = 111,
    WLAN_CFGID_RX_CHAIN = 112,
    WLAN_CFGID_TX_POWER = 113, /* 传输功率 */
    WLAN_CFGID_VAP_INFO = 114, /* 打印vap参数信息 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    WLAN_CFGID_VAP_STATE_SYN = 115,
#endif
    WLAN_CFGID_BANDWIDTH = 116,
    WLAN_CFGID_CHECK_FEM_PA = 117,

    WLAN_CFGID_STOP_SCHED_SCAN = 118,
    WLAN_CFGID_STA_VAP_INFO_SYN = 119,
    /* wpa-wpa2 */
    WLAN_CFGID_ADD_KEY = 120,
    WLAN_CFGID_DEFAULT_KEY = 121,
    WLAN_CFGID_REMOVE_KEY = 123,
    WLAN_CFGID_GET_KEY = 125,

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    WLAN_CFGID_STA_TXOP_AID = 126,
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    WLAN_CFGID_CHECK_LTE_GPIO = 127,
#endif
    WLAN_CFGID_SET_VAP_CAP_FLAG_SWITCH = 128,

    /* wep */
    // WLAN_CFGID_ADD_WEP_KEY         = 130,
    WLAN_CFGID_REMOVE_WEP_KEY = 131,
    WLAN_CFGID_ADD_WEP_ENTRY = 132,
    /* 认证加密模式配置 */
    WLAN_CFGID_EVENT_SWITCH = 144,       /* event模块开关 */
    WLAN_CFGID_ETH_SWITCH = 145,         /* 以太网帧收发开关 */
    WLAN_CFGID_80211_UCAST_SWITCH = 146, /* 80211单播帧上报开关 */
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    WLAN_CFGID_USR_THRPUT_STAT = 148, /* 影响用户实时吞吐统计信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    WLAN_CFGID_TXOP_PS_MACHW = 151, /* 配置mac txopps使能寄存器 */
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    WLAN_CFGID_BTCOEX_STATUS_PRINT = 152, /* 打印共存维测信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
    WLAN_CFGID_LTECOEX_MODE_SET = 153,
#endif
    WLAN_CFGID_REPORT_VAP_INFO = 159, /* 上报vap信息 */
    WLAN_CFGID_PROTOCOL_DBG = 160, /* 设置协议中相关的带宽等信息参数 */
#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_PHY_STAT_EN = 161,    /* 设置phy统计使能节点 */
    WLAN_CFGID_DBB_ENV_PARAM = 165,  /* 空口环境类参数上报或者停止上报 */
    WLAN_CFGID_USR_QUEUE_STAT = 166, /* 用户tid队列和节能队列统计信息 */
    WLAN_CFGID_VAP_STAT = 167,       /* vap吞吐统计计信息 */
    WLAN_CFGID_ALL_STAT = 168,       /* 所有统计信息 */
#endif

    WLAN_CFGID_PHY_DEBUG_SWITCH = 169,   /* 打印接收报文的rssi信息的调试开关 */
    WLAN_CFGID_80211_MCAST_SWITCH = 170, /* 80211组播\广播帧上报开关 */
    WLAN_CFGID_PROBE_SWITCH = 171,       /* probe requese 和 probe response上报开关 */
    WLAN_CFGID_GET_MPDU_NUM = 172,       /* 获取mpdu数目 */

    WLAN_CFGID_OTA_RX_DSCR_SWITCH = 174,      /* ota模块设置ota接收描述符上报开关 */
    WLAN_CFGID_OTA_BEACON_SWITCH = 175,       /* 设置是否上报beacon帧的开关 */
    WLAN_CFGID_OAM_OUTPUT_TYPE = 176,         /* oam模块输出的位置 */
    WLAN_CFGID_ADD_USER = 178,                /* 添加用户配置命令 */
    WLAN_CFGID_DEL_USER = 179,                /* 删除用户配置命令 */
    WLAN_CFGID_AMPDU_START = 180,             /* 开启AMPDU的配置命令 */
    WLAN_CFGID_AMPDU_END = 181,               /* 关闭AMPDU的配置命令 */
    WLAN_CFGID_AUTO_BA_SWITCH = 182,          /* 自动建立BA会话 */
    WLAN_CFGID_ADDBA_REQ = 184,               /* 建立BA会话的配置命令 */
    WLAN_CFGID_DELBA_REQ = 185,               /* 删除BA会话的配置命令 */
    WLAN_CFGID_SET_LOG_LEVEL = 186,           /* 设置LOG配置级别开关 */
    WLAN_CFGID_SET_FEATURE_LOG = 187,         /* 设置日志特性开关 */
    WLAN_CFGID_SET_ALL_OTA = 188,             /* 设置所有用户帧上报的所有开关 */
    WLAN_CFGID_SET_BEACON_OFFLOAD_TEST = 189, /* Beacon offload相关的测试，仅用于测试 */
    WLAN_CFGID_SET_LOG_PM = 190,              /* 设置Device log 为pm模式 */

    WLAN_CFGID_AMSDU_START = 191,              /* AMSDUA开启关闭 参数配置 */
    WLAN_CFGID_SET_DHCP_ARP = 192,             /* 设置dhcp和arp上报的所有开关 */
    WLAN_CFGID_SET_RANDOM_MAC_ADDR_SCAN = 193, /* 设置随机mac addr扫描开关 */
    WLAN_CFGID_SET_RANDOM_MAC_OUI = 194,       /* 设置随机mac oui */
    WLAN_CFGID_LIST_AP = 200,                  /* 列举扫描到的AP */
    WLAN_CFGID_LIST_STA = 201,                 /* 列举关联的STA */
    WLAN_CFGID_DUMP_ALL_RX_DSCR = 203,         /* 打印所有的接收描述符 */
    WLAN_CFGID_START_SCAN = 204,               /* 触发初始扫描 */
    WLAN_CFGID_START_JOIN = 205,               /* 触发加入认证并关联 */
    WLAN_CFGID_START_DEAUTH = 206,             /* 触发去认证 */
    WLAN_CFGID_DUMP_TIEMR = 207,               /* 打印所有timer的维测信息 */
    WLAN_CFGID_KICK_USER = 208,                /* 去关联1个用户 */
    WLAN_CFGID_PAUSE_TID = 209,                /* 暂停指定用户的指定tid */
    WLAN_CFGID_SET_USER_VIP = 210,             /* 设置用户为VIP用户 */
    WLAN_CFGID_SET_VAP_HOST = 211,             /* 设置VAP是否为host vap */
    WLAN_CFGID_AMPDU_TX_ON = 212,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_AMSDU_TX_ON = 213,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_SEND_BAR = 215,                 /* 指定用户的指定tid发送bar */
    WLAN_CFGID_LIST_CHAN = 217,                /* 列举支持的管制域信道 */
    WLAN_CFGID_REGDOMAIN_PWR = 218,            /* 设置管制域功率 */
    WLAN_CFGID_TXBF_SWITCH = 219,              /* 开启或关闭TXBF发送功能 */
    WLAN_CFGID_FRAG_THRESHOLD_REG = 221,       /* 设置分片门限长度 */
    WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE = 222,    /* vowifi nat keep alive */
    WLAN_CFGID_SET_POW_RF_CTL = 223,           /* 设置功率是否为RF控 */
    WLAN_CFGID_OPMODE_SWITCH = 224,            /* 打印接收报文的rssi信息的调试开关 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    WLAN_CFGID_SET_PSM_PARAM = 231, /* STA 低功耗tbtt offset/listen interval配置 */
    WLAN_CFGID_SET_STA_PM_ON = 232, /* STA低功耗开关接口 */
#endif

    WLAN_CFGID_DUMP_BA_BITMAP = 241, /* 发指定个数的报文 */
    WLAN_CFGID_VAP_PKT_STAT = 242,   /* vap统计信息 */
    WLAN_CFGID_TIMER_START = 244,
    WLAN_CFGID_SHOW_PROFILING = 245,
    WLAN_CFGID_AMSDU_AMPDU_SWITCH = 246,
    WLAN_CFGID_COUNTRY = 247,             /* 设置国家码管制域信息 */
    WLAN_CFGID_TID = 248,                 /* 读取最新接收到数据帧的TID */
    WLAN_CFGID_RESET_HW = 249,            /* Reset mac&phy */
    WLAN_CFGID_UAPSD_DEBUG = 250,         /* UAPSD维测信息 */
    WLAN_CFGID_DUMP_RX_DSCR = 251,        /* dump接收描述队列 */
    WLAN_CFGID_DUMP_TX_DSCR = 252,        /* dump发送描述符队列 */
    WLAN_CFGID_DUMP_MEMORY = 253,         /* dump内存 */
    WLAN_CFGID_ALG_PARAM = 254,           /* 算法参数配置 */
    WLAN_CFGID_BEACON_CHAIN_SWITCH = 255, /* 设置beacon帧发送策略，0为关闭双路轮流发送，1为开启 */

    WLAN_CFGID_2040_CHASWI_PROHI = 258, /* 设置2040 channel switch prohibited, 0为不禁止，1为禁止 */
    WLAN_CFGID_2040_INTOLERANT = 259,   /* 设置40MHz不允许位，0: 不允许运行40MHz，1: 允许运行40MHz */
    WLAN_CFGID_2040_COEXISTENCE = 260,  /* 设置20/40共存使能，0: 20/40共存使能，1: 20/40共存不使能 */
    WLAN_CFGID_RX_FCS_INFO = 261,       /* 打印接收帧FCS解析信息 */

    WLAN_CFGID_ACS_CONFIG = 286,           /* ACS命令 */
    WLAN_CFGID_SCAN_ABORT = 287,           /* 扫描终止 */
    /* 以下命令为cfg80211下发的命令(通过内核) */
    WLAN_CFGID_CFG80211_START_SCHED_SCAN = 288, /* 内核下发启动PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_STOP_SCHED_SCAN = 289,  /* 内核下发停止PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_START_SCAN = 290,       /* 内核下发启动扫描命令 */
    WLAN_CFGID_CFG80211_START_CONNECT = 291,    /* 内核下发启动JOIN(connect)命令 */
    WLAN_CFGID_CFG80211_SET_CHANNEL = 292,      /* 内核下发设置信道命令 */
    WLAN_CFGID_CFG80211_SET_WIPHY_PARAMS = 293, /* 内核下发设置wiphy 结构体命令 */
    WLAN_CFGID_CFG80211_CONFIG_BEACON = 295,    /* 内核下发设置VAP信息 */
    WLAN_CFGID_ALG = 296,                       /* 算法配置命令 */
    WLAN_CFGID_ACS_PARAM = 297,                 /* ACS命令 */
    WLAN_CFGID_RADARTOOL = 310,                 /* DFS配置命令 */

    /* BEGIN:以下命令为开源APP 程序下发的私有命令 */
    WLAN_CFGID_GET_ASSOC_REQ_IE = 311,   /* hostapd 获取ASSOC REQ 帧信息 */
    WLAN_CFGID_SET_WPS_IE = 312,         /* hostapd 设置WPS 信息元素到VAP */
    WLAN_CFGID_SET_RTS_THRESHHOLD = 313, /* hostapd 设置RTS 门限 */
    WLAN_CFGID_SET_WPS_P2P_IE = 314,     /* wpa_supplicant 设置WPS/P2P 信息元素到VAP */
    /* END:以下命令为开源APP 程序下发的私有命令 */
#ifdef _PRE_DEBUG_MODE
    WLAN_CFGID_SET_RXCH = 317,     /* 设置接收通道 */
    WLAN_CFGID_DYNC_TXPOWER = 318, /* 动态功率校准开关 */
#endif
    WLAN_CFGID_ADJUST_PPM = 319,     /* 设置PPM校准算法 */
    WLAN_CFGID_USER_INFO = 320,      /* 用户信息 */
    WLAN_CFGID_SET_DSCR = 321,       /* 配置用户信息 */
    WLAN_CFGID_SET_RATE = 322,       /* 设置non-HT速率 */
    WLAN_CFGID_SET_MCS = 323,        /* 设置HT速率 */
    WLAN_CFGID_SET_MCSAC = 324,      /* 设置VHT速率 */
    WLAN_CFGID_SET_NSS = 325,        /* 设置空间流个数 */
    WLAN_CFGID_SET_RFCH = 326,       /* 设置发射通道 */
    WLAN_CFGID_SET_BW = 327,         /* 设置带宽 */
    WLAN_CFGID_SET_ALWAYS_TX = 328,  /* 设置常发模式 */
    WLAN_CFGID_SET_ALWAYS_RX = 329,  /* 设置常发模式 */
    WLAN_CFGID_GET_THRUPUT = 330,    /* 获取芯片吞吐量信息 */
    WLAN_CFGID_SET_FREQ_SKEW = 331,  /* 设置频偏参数 */
    WLAN_CFGID_REG_INFO = 332,       /* 寄存器地址信息 */
    WLAN_CFGID_REG_WRITE = 333,      /* 写入寄存器信息 */
    WLAN_CFGID_OPEN_ADDR4 = 336,     /* mac头为4地址 */
    WLAN_CFGID_WMM_SWITCH = 338,     /* 打开或者关闭wmm */
    WLAN_CFGID_HIDE_SSID = 339,      /* 打开或者关闭隐藏ssid */
    WLAN_CFGID_CHIP_TEST_OPEN = 340, /* 打开芯片验证开关 */
    WLAN_CFGID_SET_ALWAYS_TX_1102 = 343, /* 设置1102常发模式 */

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    WLAN_CFGID_EDCA_OPT_SWITCH_AP = 344,  /* 设置AP打开edca_opt模式 */
    WLAN_CFGID_EDCA_OPT_CYCLE_AP = 345,   /* 设置AP的edca调整周期 */
    WLAN_CFGID_EDCA_OPT_SWITCH_STA = 346, /* 设置STA的edca优化开关 */
    WLAN_CFGID_EDCA_OPT_WEIGHT_STA = 347, /* 设置STA的edca参数调整权重 */
#endif
    WLAN_CFGID_SET_RTS_PARAM = 350,        /* 配置RTS速率 */
    WLAN_CFGID_UPDTAE_PROT_TX_PARAM = 351, /* 更新保护模式相关的发送参数 */
    WLAN_CFGID_SET_PROTECTION = 352,

    WLAN_CFGID_SET_FLOWCTL_PARAM = 353, /* 设置流控相关参数 */
    WLAN_CFGID_GET_FLOWCTL_STAT = 354,  /* 获取流控相关状态信息 */
    WLAN_CFGID_GET_HIPKT_STAT = 355,    /* 获取高优先级报文的统计情况 */

    WLAN_CFGID_SET_AUTO_PROTECTION = 362, /* 设置auto protection开关 */

    WLAN_CFGID_SEND_2040_COEXT = 370, /* 发送20/40共存管理帧 */
    WLAN_CFGID_2040_COEXT_INFO = 371, /* VAP的所有20/40共存信息 */

#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_CFGID_FTM_DBG = 374, /* FTM调试命令 */
#endif

    WLAN_CFGID_GET_VERSION = 375, /* 获取版本 */
#ifdef _PRE_DEBUG_MODE
    WLAN_CFGID_GET_ALL_REG_VALUE = 376, /* 获取所有寄存器的值 */
    WLAN_CFGID_REPORT_AMPDU_STAT = 377, /* ampdu各种流程统计 */
#endif

    WLAN_CFGID_SET_THRUPUT_BYPASS = 391, /* 设置thruput bypass维测点 */

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    WLAN_CFGID_SET_OPMODE_NOTIFY = 400, /* 设置工作模式通知能力 */
    WLAN_CFGID_GET_USER_RSSBW = 401,
#endif
    WLAN_CFGID_SET_VAP_NSS = 410, /* 设置VAP的接收空间流 */

#ifdef _PRE_WLAN_FEATURE_CSI_RAM
    WLAN_CFGID_SET_CSI = 427, /* CSI使能 */
#endif

#ifdef _PRE_DEBUG_MODE
    WLAN_CFGID_RX_FILTER_VAL = 430,
    WLAN_CFGID_SET_RX_FILTER_EN = 431,
    WLAN_CFGID_GET_RX_FILTER_EN = 432,
#endif

#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    WLAN_CFGID_ADD_BLACK_LIST = 440, /* 添加黑名单 */
    WLAN_CFGID_DEL_BLACK_LIST = 441, /* 删除黑名单 */
    WLAN_CFGID_BLACKLIST_MODE = 442,
    WLAN_CFGID_BLACKLIST_SHOW = 443,
    WLAN_CFGID_AUTOBLACKLIST_ON = 444,
    WLAN_CFGID_AUTOBLACKLIST_AGING = 445,
    WLAN_CFGID_AUTOBLACKLIST_THRESHOLD = 446,
    WLAN_CFGID_AUTOBLACKLIST_RESET = 447,
    WLAN_CFGID_ISOLATION_MODE = 448,
    WLAN_CFGID_ISOLATION_TYPE = 449,
    WLAN_CFGID_ISOLATION_FORWARD = 450,
    WLAN_CFGID_ISOLATION_CLEAR = 451,
    WLAN_CFGID_ISOLATION_SHOW = 452,
#endif
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    WLAN_CFGID_ADD_BLACK_LIST_ONLY = 458, /* 添加到黑名单,不做check user和删除user行为 */
#endif

    WLAN_CFGID_WIFI_EN = 460, /* WIFI使能开关 */
    WLAN_CFGID_PM_INFO = 461, /* PM信息 */
    WLAN_CFGID_PM_EN = 462,   /* PM开关 */
    WLAN_CFGID_SET_AGGR_NUM = 474, /* 设置聚合个数 */
    WLAN_CFGID_FREQ_ADJUST = 475,  /* 频偏调整 */

    WLAN_CFGID_SET_STBC_CAP = 477, /* 设置STBC能力 */
    WLAN_CFGID_SET_LDPC_CAP = 478, /* 设置LDPC能力 */

    WLAN_CFGID_VAP_CLASSIFY_EN = 479,  /* VAP流等级开关 */
    WLAN_CFGID_VAP_CLASSIFY_TID = 480, /* VAP流等级 */

    WLAN_CFGID_RESET_HW_OPERATE = 481,    /* Reset 同步 */
    WLAN_CFGID_SCAN_TEST = 482,           /* 扫描模块测试命令 */
    WLAN_CFGID_QUERY_STATION_STATS = 483, /* 信息上报查询命令 */
    WLAN_CFGID_CONNECT_REQ = 484,
    WLAN_CFIGD_BGSCAN_ENABLE = 485,       /* 禁用背景扫描命令 */
    WLAN_CFGID_QUERY_RSSI = 486,          /* 查询用户dmac rssi信息 */
    WLAN_CFGID_QUERY_RATE = 487,          /* 查询用户当前使用的tx rx phy rate */
    WLAN_CFGID_QUERY_CHR_EXT_INFO = 488,
    WLAN_CFGID_QUERY_PSM_STAT = 489,      /* 查询低功耗统计数据 */
    WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL = 490,        /* 停止在指定信道 */
    WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL = 491, /* 取消停止在指定信道 */

    WLAN_CFGID_DEVICE_MEM_LEAK = 492, /* device 侧mem leak打印接口 */
    WLAN_CFGID_DEVICE_MEM_INFO = 493, /* device 侧mem 使用情况打印接口 */

#ifdef _PRE_WLAN_FEATURE_STA_PM
    WLAN_CFGID_SET_PS_MODE = 494, /* 设置pspoll mode */
#ifdef _PRE_PSM_DEBUG_MODE
    WLAN_CFGID_SHOW_PS_INFO = 495, /* PSM状态查看 */
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    WLAN_CFGID_SET_UAPSD_PARA = 496, /* 设置UAPSD参数 */
#endif

    WLAN_CFGID_CFG80211_MGMT_TX = 498, /* 发送管理帧 */
    WLAN_CFGID_CFG80211_MGMT_TX_STATUS = 499,
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    WLAN_CFGID_THRUPUT_INFO = 501, /* 吞吐量数据由dmac同步到hmac */
#endif
#ifdef _PRE_WLAN_FEATURE_11D
    WLAN_CFGID_SET_RD_IE_SWITCH = 502, /* 设置是否根据关联ap设置sta的国家码 */
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    WLAN_CFGID_SET_P2P_PS_OPS = 504,  /* 配置P2P OPS节能 */
    WLAN_CFGID_SET_P2P_PS_NOA = 505,  /* 配置P2P NOA节能 */
    WLAN_CFGID_SET_P2P_PS_STAT = 506, /* 配置P2P 节能统计 */
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    WLAN_CFGID_SET_QOS_MAP = 507, /* 配置HotSpot 2.0 QoSMap参数 */
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    WLAN_CFGID_SET_P2P_MIRACAST_STATUS = 508, /* 配置P2P投屏状态 */
#endif

    WLAN_CFGID_UAPSD_UPDATE = 510,
    WLAN_CFGID_SDIO_FLOWCTRL = 523,
    WLAN_CFGID_NSS = 524, /* 空间流信息的同步 */

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    WLAN_CFGID_ENABLE_ARP_OFFLOAD = 526,   /* arp offload的配置事件 */
    WLAN_CFGID_SET_IP_ADDR = 527,          /* IPv4/IPv6地址的配置事件 */
    WLAN_CFGID_SHOW_ARPOFFLOAD_INFO = 528, /* 打印device侧的IP地址 */
#endif

    WLAN_CFGID_CFG_VAP_H2D = 529,   /* 配置vap下发device事件 */
    WLAN_CFGID_HOST_DEV_INIT = 530, /* 下发初始化host dev init事件 */
    WLAN_CFGID_HOST_DEV_EXIT = 531, /* 下发去初始化host dev exit事件 */
#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFGID_GET_TCP_ACK_STREAM_INFO = 533, /* 显示TCP ACK 过滤统计值 */
    WLAN_CFGID_TX_TCP_ACK_OPT_ENALBE = 534,   /* 设置发送TCP ACK优化使能 */
    WLAN_CFGID_RX_TCP_ACK_OPT_ENALBE = 535,   /* 设置接收TCP ACK优化使能 */
    WLAN_CFGID_TX_TCP_ACK_OPT_LIMIT = 536,    /* 设置发送TCP ACK LIMIT */
    WLAN_CFGID_RX_TCP_ACK_OPT_LIMIT = 537,    /* 设置接收TCP ACK LIMIT */
#endif

    WLAN_CFGID_SET_MAX_USER = 538, /* 设置最大用户数 */
    WLAN_CFGID_GET_STA_LIST = 539, /* 设置最大用户数 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER = 540,
#endif
#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_SET_PERFORMANCE_LOG_SWITCH = 541, /* 设置性能打印控制开关 */
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    WLAN_CFGID_WAPI_INFO = 542,
    WLAN_CFGID_ADD_WAPI_KEY = 543,
#endif

#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_QUERY_ANI = 544, /* 查询VAP抗干扰参数 */
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    WLAN_CFGID_ROAM_ENABLE = 550, /* 漫游使能 */
    WLAN_CFGID_ROAM_ORG = 551,    /* 漫游正交设置 */
    WLAN_CFGID_ROAM_BAND = 552,   /* 漫游频段设置 */
    WLAN_CFGID_ROAM_START = 553,  /* 漫游开始 */
    WLAN_CFGID_ROAM_INFO = 554,   /* 漫游打印 */
    WLAN_CFGID_SET_ROAMING_MODE = 555,
    WLAN_CFGID_SET_ROAM_TRIGGER = 556,
    WLAN_CFGID_ROAM_HMAC_SYNC_DMAC = 557,
    WLAN_CFGID_SET_FT_IES = 558,
    WLAN_CFGID_ROAM_NOTIFY_STATE = 559,
#endif                                       // _PRE_WLAN_FEATURE_ROAM
    WLAN_CFGID_CFG80211_SET_PMKSA = 560,     /* 设置PMK缓存 */
    WLAN_CFGID_CFG80211_DEL_PMKSA = 561,     /* 删除PMK缓存 */
    WLAN_CFGID_CFG80211_FLUSH_PMKSA = 562,   /* 清空PMK缓存 */
    WLAN_CFGID_CFG80211_EXTERNAL_AUTH = 563, /* 触发SAE认证 */

    WLAN_CFGID_SET_PM_SWITCH = 570, /* 全局低功耗使能去使能 */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    WLAN_CFGID_SET_DEVICE_FREQ = 571,        /* 设置device主频 */
    WLAN_CFGID_SET_DEVICE_FREQ_VALUE = 572,  /* 定制化设置调频参数 */
    WLAN_CFGID_SET_DEVICE_FREQ_ENABLE = 573, /* 设置自动调频使能 */
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    WLAN_CFGID_2040BSS_ENABLE = 574, /* 20/40 bss判断使能开关 */
#endif
    WLAN_CFGID_DESTROY_VAP = 575,

    WLAN_CFGID_SET_ANT = 576, /* 设置使用的天线 */
    WLAN_CFGID_GET_ANT = 577, /* 获取天线状态 */

    WLAN_CFGID_GREEN_AP_EN = 578,
#ifdef _PRE_WLAN_FEATURE_APF
    WLAN_CFGID_SET_APF = 581, /* 设置APF PROGRAM */
#endif
    WLAN_CFGID_PM_DEBUG_SWITCH = 582, /* 低功耗调试命令 */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* HISI-CUSTOMIZE */
    WLAN_CFGID_SET_CUS_PRIV = 598,
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    WLAN_CFGID_SET_CUS_DYN_CALI_PARAM = 599, /* 配置dyncali 参数 */
#endif
    WLAN_CFGID_SET_LINKLOSS_THRESHOLD = 600, /* 配置linkloss门限 */
    WLAN_CFGID_SET_ALL_LOG_LEVEL,            /* 配置所有vap log level */
    WLAN_CFGID_SET_D2H_HCC_ASSEMBLE_CNT,     /* 配置D2H SDIO聚合参数 */
    WLAN_CFGID_SET_CUS_RF,                   /* RF定制化 */
    WLAN_CFGID_SET_CUS_DTS_CALI,             /* DTS校准定制化 */
    WLAN_CFGID_SET_CUS_NVRAM_PARAM,          /* NVRAM参数定制化 */
    WLAN_CFGID_LAUCH_CAP,                    /* 读取设备发射能力 */
    WLAN_CFGID_SET_CUS_BASE_POWER,           /* 设置基准功率 */
    WLAN_CFGID_SET_5G_HIGH_BAND_MAX_POW,     /* 设置5G高band最大发送功率 */
    WLAN_CFGID_SET_CUS_FCC_CE_POWER,         /* 设置基准功率和边带功率 */
    /* HISI-CUSTOMIZE INFOS */
    WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS = 610, /* show device customize info */
#endif                                         /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_DFR
    WLAN_CFGID_TEST_DFR_START = 611,
#endif  // _PRE_WLAN_FEATURE_DFR

    WLAN_CFGID_WFA_CFG_AIFSN = 612,
    WLAN_CFGID_WFA_CFG_CW = 613,

    WLAN_CFGID_REDUCE_SAR = 615, /* 通过降低发射功率来降低SAR */

    WLAN_CFGID_DBB_SCALING_AMEND = 616, /* 调整dbb scaling值 */
    WLAN_CFGID_TXOP_SWITCH = 617, /* TXOP定制化开关 */
#ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_DEBUG_MODE
    WLAN_CFGIG_DFR_ENABLE = 640,        /* 是否使能dfr开关 */
    WLAN_CFGID_TRIG_PCIE_RESET = 641,   /* 触发pcie复位 */
    WLAN_CFGID_TRIG_LOSS_TX_COMP = 642, /* 触发丢失发送完成中断 */
#endif
#endif

    WLAN_CFGID_PCIE_PM_LEVEL = 643,

#ifdef _PRE_WLAN_FEATURE_11K
    WLAN_CFGID_SEND_NEIGHBOR_REQ = 644,
    WLAN_CFGID_REQ_SAVE_BSS_INFO = 645,
    WLAN_CFGID_BCN_TABLE_SWITCH = 646,
#endif
    WLAN_CFGID_VOE_ENABLE = 647,
    WLAN_CFGID_VENDOR_CMD_GET_CHANNEL_LIST = 650, /* 1102 vendor cmd, 获取信道列表 */

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    WLAN_CFGID_VOWIFI_INFO = 653,   /* host->device，VoWiFi模式配置; device->host,上报切换VoWiFi/VoLTE */
    WLAN_CFGID_VOWIFI_REPORT = 654, /* device->host,上报切换VoWiFi/VoLTE */
#endif                              /* _PRE_WLAN_FEATURE_VOWIFI */

#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_CFGID_ADDTS_REQ = 657,    /* 发送ADDTS REQ的配置命令 */
    WLAN_CFGID_DELTS = 658,        /* 发送DELTS的配置命令 */
    WLAN_CFGID_REASSOC_REQ = 659,  /* 发送reassoc req 的配置命令 */
    WLAN_CFGID_WMMAC_SWITCH = 660, /* 设置WMMAC SWITCH开关的配置命令 */
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    WLAN_CFGID_IP_FILTER = 661, /* 配置IP端口过滤的命令 */
#endif                          // _PRE_WLAN_FEATURE_IP_FILTER

    WLAN_CFGID_NARROW_BW = 662,

#ifdef _PRE_WLAN_DOWNLOAD_PM
    WLAN_CFGID_SET_CUS_DOWNLOAD_RATE_LIMIT = 663, /* 限流参数定制化 */
#endif
#ifndef CONFIG_HAS_EARLYSUSPEND
    WLAN_CFGID_SET_SUSPEND_MODE = 664, /* 设置亮暗屏状态 */
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    WLAN_CFGID_TCP_ACK_BUF = 665,
#endif
    WLAN_CFGID_SET_ALWAYS_TX_NUM = 666, /* 设置1102A指定常发数目 */

    WLAN_CFIGD_MCS_SET_CHECK_ENABLE = 667, /* 是否检查mcs 速率集 */

    WLAN_CFGID_FORCE_PASS_FILTER = 668,
    WLAN_CFGID_SET_PS_PARAMS = 669,
    WLAN_CFGID_SET_TX_BA_POLICY = 670,
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    WLAN_CFGID_SET_BTCOEX_PARAMS = 671,
#endif
    WLAN_CFGID_BINDCPU = 672,    /* 绑核命令 */
    WLAN_CFGID_NAPIWEIGHT = 673, /* 调整NAPI weight值 */
    WLAN_CFGID_SET_NBFH = 674,
    WLAN_CFGID_SET_DSCR_TH_HOST = 675,
    WLAN_CFGID_SET_TCP_ACK_FILTER = 676,
    WLAN_CFGID_HITALK_LISTEN = 677,
    WLAN_CFGID_SET_SAE_PWE = 678, /* 设置SAE_PWE */
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    WLAN_CFGID_PMF_CAP = 679, /* PMF能力配置 */
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    WLAN_CFGID_ASSIGNED_FILTER = 680,
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY = 681, /* TAS/默认态切换完成RSSI上报host */
#endif
    WLAN_CFGID_TAS_RSSI_ACCESS = 682,      /* TAS天线测量 */
    WLAN_CFGID_SET_EXT_FCC_CE_POWER = 683, /* 配置FCC或CE国家次边带信道的功率限制值 */
    WLAN_CFGID_RX_FILTER_FRAG = 684, /* 分片过滤调试命令 */
    WLAN_CFGID_SET_CHAN_MEAS = 685,

    WLAN_CFGID_QUERY_TSF = 844,
#ifdef _PRE_WLAN_CHBA_MGMT
    WLAN_CFGID_START_CHBA = 845,
    WLAN_CFGID_CHBA_SET_PS_THRES = 846,
    WLAN_CFGID_CHBA_CONN_NOTIFY = 847, /* supplicant下发的CHBA准备连接通知 */
    WLAN_CFGID_CHBA_SET_VAP_BITMAP_LEVEL = 848, /* chba vap bitmap配置更新 */
    WLAN_CFGID_CHBA_SET_USER_BITMAP_LEVEL = 849, /* chba user bitmap配置更新 */
    WLAN_CFGID_CHBA_SET_CHANNEL_SEQ_PARAMS = 850, /* 配置CHBA 信道序列参数 */
    WLAN_CFGID_CHBA_LOG_LEVEL = 853, /* CHBA维测日志打印等级 */
    WLAN_CFGID_CHBA_MODULE_INIT = 855,
    WLAN_CFGID_CHBA_CONNECT_PREPARE = 856,
    WLAN_CFGID_CHBA_AUTO_BITMAP_CMD = 857,
    WLAN_CFGID_CHBA_SET_USER_BITMAP_CMD = 858,
    WLAN_CFGID_CHBA_SET_VAP_BITMAP_CMD = 859,
    WLAN_CFGID_CHBA_CHAN_SWITCH_TEST_CMD = 860,
    WLAN_CFGID_CHBA_ADJUST_ISLAND_CHAN = 861, /* CHBA全岛切信道命令 */
    WLAN_CFGID_CHBA_FEATURE_SWITCH = 862,
    WLAN_CFGID_CHBA_SET_BATTERY = 863, /* 配置电量，改变RP值 */
    WLAN_CFGID_CHBA_NOTIFY_MULTI_DOMAIN = 864,  /* chba工作时隙收到非bssid的beacon */
    WLAN_CFGID_CHBA_NOTIFY_MAX_RP_CHANGED = 865, /* chba同步时隙收到非bssid的beacon并且max rp发生变化 */
    WLAN_CFGID_CHBA_SYNC_REQUEST_FLAG = 866,
    WLAN_CFGID_CHBA_SYNC_DOMAIN_MERGE_FLAG = 867,
    WLAN_CFGID_CHBA_DEVICE_INFO_UPDATE = 868,
    WLAN_CFGID_CHBA_NOTIFY_NON_ALIVE_DEVICE = 869, /* DMAC不活跃设备上报 */
    WLAN_CFGID_CHBA_OWN_RP_SYNC      = 870,
    WLAN_CFGID_CHBA_SET_CHANNEL = 871, /* 配置CHBA信道接口 */
    WLAN_CFGID_CHBA_AUTO_ADJUST_BITMAP_LEVEL = 872, /* chba bitmap档位自动调整事件 */
    WLAN_CFGID_CHBA_CSA_LOST_DEVICE_NOTIFY = 873, /* 切信道存在设备没切 */
    WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO = 874,
    WLAN_CFGID_CHBA_TX_DEAUTH_DISASSOC_COMPLETE = 875, /* chba disassoc发送完成事件 */
    WLAN_CFGID_CHBA_UPDATE_TOPO_BITMAP_INFO = 876, /* chba dmac通知host更新topo中bitmap信息 */
    WLAN_CFGID_CHBA_SYNC_EVENT = 877, /* chba dmac通知host 同步时隙开始或结束 */
#endif

    /************************************************************************
        第三段 非MIB的内部数据同步，需要严格受控
    *************************************************************************/
    WLAN_CFGID_SET_MULTI_USER = 2000,
    WLAN_CFGID_USR_INFO_SYN = 2001,
    WLAN_CFGID_USER_ASOC_STATE_SYN = 2002,
    WLAN_CFGID_INIT_SECURTIY_PORT = 2004,
    WLAN_CFGID_USER_RATE_SYN = 2005,
    WLAN_CFGID_UPDATE_OPMODE = 2006, /* 更新opmode的相关信息 */

    WLAN_CFGID_USER_CAP_SYN = 2007, /* hmac向dmac同步mac user的cap能力信息 */

    WLAN_CFGID_SUSPEND_ACTION_SYN = 2008,
    WLAN_CFGID_APF_SWITCH_SYN = 2009,

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    WLAN_CFGID_DYN_CALI_CFG = 2010,
#endif

#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    WLAN_CFGID_DYN_EXTLNA_BYPASS_SWITCH = 2011, /* 动态bypass外置LNA开关 */
#endif

    WLAN_CFGID_SET_TX_POW = 2012,     /* 设置HAL层功率参数 */
    WLAN_CFGID_VAP_MIB_UPDATE = 2013, /* d2h 根据hal cap更VAP mib 能力同步到host侧 */

#ifdef _PRE_BT_FITTING_DATA_COLLECT
    WLAN_CFGID_INIT_BT_ENV = 2014,
    WLAN_CFGID_SET_BT_FREQ = 2015,
    WLAN_CFGID_SET_BT_UPC_BY_FREQ = 2016,
    WLAN_CFGID_PRINT_BT_GM = 2017,
#endif
    WLAN_CFGID_BTCOEX_STATUS_SYN = 2018,
    WLAN_CFGID_SET_DSCR_TH = 2019,
    WLAN_CFGID_SET_TXBF_ABILITY = 2020,
    WLAN_CFGID_TCP_ACK_INFO_SYN = 2021,
    WLAN_CFGID_TCP_ACK_FILTER_SWITCH = 2022,
#ifdef _PRE_WLAN_FEATURE_HS20
    WLAN_CFGID_INTERWORKING_SWITCH = 2023,
#endif
    WLAN_CFGID_11V_FILTER_SWITCH = 2024,
    WLAN_CFGID_FFT_WINDOW_OFFSET = 2025,
    WLAN_CFGID_RESET_RSSI = 2026,
    WLAN_CFGID_CHECK_PACKET = 2027,
    WLAN_CFGID_TX_OPT_SYN = 2028,
    WLAN_CFGID_WFD_AGGR_LIMIT_SYN = 2029,
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    WLAN_CFGID_NRCOEX_PARAMS = 2030,
    WLAN_CFGID_WIFI_PRIORITY = 2031,
    WLAN_CFGID_NRCOEX_CMD = 2032,
    WLAN_CFGID_NRCOEX_INFO = 2033,
#endif
    WLAN_CFGID_GET_MNGPKT_SENDSTAT = 2034, /* 获取管理帧发送状态 */

    WLAN_CFGID_TX_OPT_SYN_TCP_ACK = 2035,

    WLAN_CFGID_BUTT,
} wlan_cfgid_enum;
typedef oal_uint16 wlan_cfgid_enum_uint16;
/* VHT Operation Info  field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_OP_WIDTH_20_40 = 0,    /* 工作在20/40 */
    WLAN_MIB_VHT_OP_WIDTH_80 = 1,       /* 工作在80 */
    WLAN_MIB_VHT_OP_WIDTH_160 = 2,      /* 工作在160 */
    WLAN_MIB_VHT_OP_WIDTH_80PLUS80 = 3, /* 工作在80+80 */

    WLAN_MIB_VHT_OP_WIDTH_BUTT
} wlan_mib_vht_op_width_enum;
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 当前MIB表项还未稳定，暂不需要4字节对齐，待后续MIB项稳定后再来调整这些结构体 */
/*lint -e958*/
/*lint -e959*/
/* 是否需要该结构体表示变长的字符串，待确定 */
typedef struct {
    oal_uint16 us_octet_nums;    /* 字节长度，此值为0时表示没有字符串信息 */
    oal_uint8 *puc_octec_string; /* 字符串起始地址 */
} wlan_mib_octet_string_stru;
typedef struct {
    oal_uint8 uc_mcs_nums;
    oal_uint8 *pst_mcs_set;
} wlan_mib_ht_op_mcs_set_stru; /* dot11HTOperationalMCSSet */

/* dot11LocationServicesLocationIndicationIndicationParameters OBJECT-TYPE */
/* SYNTAX OCTET STRING (SIZE (1..255)) */
/* MAX-ACCESS read-create */
/* STATUS current */
/* DESCRIPTION */
/* "This attribute indicates the location Indication Parameters used for   */
/* transmitting Location Track Notification Frames." */
/* ::= { dot11LocationServicesEntry 15} */
typedef struct {
    oal_uint8 uc_para_nums;
    oal_uint8 *uc_para;
} wlan_mib_local_serv_location_ind_ind_para_stru;

/* Start of dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 } */
typedef struct {
    oal_uint8 auc_dot11StationID[WLAN_MAC_ADDR_LEN];      /* dot11StationID MacAddress, */
    oal_uint8 auc_p2p0_dot11StationID[WLAN_MAC_ADDR_LEN]; /* P2P0 dot11StationID MacAddress, */
    // oal_uint32          ul_dot11MediumOccupancyLimit;                   /* dot11MediumOccupancyLimit Unsigned32, */
    // oal_bool_enum_uint8 en_dot11CFPollable;                             /* dot11CFPollable TruthValue, */
    // oal_uint32          ul_dot11CFPPeriod;                              /* dot11CFPPeriod Unsigned32, */
    // oal_uint32          ul_dot11CFPMaxDuration;                         /* dot11CFPMaxDuration Unsigned32, */
    oal_uint32 ul_dot11AuthenticationResponseTimeOut; /* dot11AuthenticationResponseTimeOut Unsigned32, */
    // oal_bool_enum_uint8 en_dot11PrivacyOptionImplemented;            /* dot11PrivacyOptionImplemented TruthValue, */
    wlan_mib_pwr_mgmt_mode_enum_uint8 uc_dot11PowerManagementMode; /* dot11PowerManagementMode INTEGER, */
    oal_uint8 auc_dot11DesiredSSID[32 + 1]; /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */     /* +1预留\0 */
    wlan_mib_desired_bsstype_enum_uint8 en_dot11DesiredBSSType;    /* dot11DesiredBSSType INTEGER, */

    oal_uint32 ul_dot11BeaconPeriod;               /* dot11BeaconPeriod Unsigned32, */
    oal_uint32 ul_dot11DTIMPeriod;                 /* dot11DTIMPeriod Unsigned32, */
    oal_uint32 ul_dot11AssociationResponseTimeOut; /* dot11AssociationResponseTimeOut Unsigned32, */
    oal_uint32 ul_dot11DisassociateReason;         /* dot11DisassociateReason Unsigned32, */
    oal_uint8 auc_dot11DisassociateStation[6];     /* dot11DisassociateStation MacAddress, */
    oal_uint32 ul_dot11DeauthenticateReason;       /* dot11DeauthenticateReason Unsigned32, */
    oal_uint8 auc_dot11DeauthenticateStation[6];   /* dot11DeauthenticateStation MacAddress, */
    oal_uint32 ul_dot11AuthenticateFailStatus;     /* dot11AuthenticateFailStatus Unsigned32, */
    oal_uint8 auc_dot11AuthenticateFailStation[6]; /* dot11AuthenticateFailStation MacAddress, */
    // oal_bool_enum_uint8 en_dot11MultiDomainCapabilityImplemented;
    oal_bool_enum_uint8 en_dot11MultiDomainCapabilityActivated; /* dot11MultiDomainCapabilityActivated TruthValue, */

    oal_bool_enum_uint8 en_dot11SpectrumManagementImplemented; /* dot11SpectrumManagementImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11SpectrumManagementRequired;    /* dot11SpectrumManagementRequired TruthValue, */
    // oal_bool_enum_uint8 en_dot11RSNAOptionImplemented;                  /* dot11RSNAOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11RSNAPreauthenticationImplemented;
    // oal_bool_enum_uint8 en_dot11OperatingClassesImplemented;       /* dot11OperatingClassesImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11OperatingClassesRequired;         /* dot11OperatingClassesRequired TruthValue, */
    oal_bool_enum_uint8 en_dot11QosOptionImplemented;               /* dot11QosOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11ImmediateBlockAckOptionImplemented;
    oal_bool_enum_uint8 en_dot11DelayedBlockAckOptionImplemented;
    // oal_bool_enum_uint8 en_dot11DirectOptionImplemented;           /* dot11DirectOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11APSDOptionImplemented; /* dot11APSDOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11QAckOptionImplemented;                  /* dot11QAckOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11QBSSLoadImplemented; /* dot11QBSSLoadImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11QueueRequestOptionImplemented;  /* dot11QueueRequestOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11TXOPRequestOptionImplemented;   /* dot11TXOPRequestOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11MoreDataAckOptionImplemented;   /* dot11MoreDataAckOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11AssociateInNQBSS;                       /* dot11AssociateInNQBSS TruthValue, */
    // oal_bool_enum_uint8 en_dot11DLSAllowedInQBSS;                       /* dot11DLSAllowedInQBSS TruthValue, */
    // oal_bool_enum_uint8 en_dot11DLSAllowed;                             /* dot11DLSAllowed TruthValue, */
    // oal_uint32          ul_dot11AssociateID;                            /* dot11AssociateID Unsigned32, */
    // oal_uint8           uc_dot11AssociateFailStation;                   /* dot11AssociateFailStation MacAddress, */
    // oal_uint32          ul_dot11AssociateFailStatus;                    /* dot11AssociateFailStatus Unsigned32, */
    // oal_uint32          ul_dot11ReassociateID;                          /* dot11ReassociateID Unsigned32, */
    // oal_uint8           uc_dot11ReassociateFailStation;                 /* dot11ReassociateFailStation MacAddress, */
    // oal_uint32          ul_dot11ReassociateFailStatus;                  /* dot11ReassociateFailStatus Unsigned32, */
    // oal_bool_enum_uint8 en_dot11RadioMeasurementImplemented;      /* dot11RadioMeasurementImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11RadioMeasurementActivated; /* dot11RadioMeasurementActivated TruthValue, */
    // oal_uint32          ul_dot11RMMeasurementProbeDelay;        /* dot11RMMeasurementProbeDelay Unsigned32, */
    // oal_uint32          ul_dot11RMMeasurementPilotPeriod;          /* dot11RMMeasurementPilotPeriod Unsigned32, */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
    oal_bool_enum_uint8 en_dot11RMLinkMeasurementActivated; /* dot11RMLinkMeasurementActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11RMNeighborReportActivated;         /* dot11RMNeighborReportActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11RMParallelMeasurementsActivated;
    // oal_bool_enum_uint8 en_dot11RMRepeatedMeasurementsActivated;
    oal_bool_enum_uint8 en_dot11RMBeaconPassiveMeasurementActivated;
    oal_bool_enum_uint8 en_dot11RMBeaconActiveMeasurementActivated;
    oal_bool_enum_uint8 en_dot11RMBeaconTableMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMBeaconMeasurementReportingConditionsActivated;
    // oal_bool_enum_uint8 en_dot11RMFrameMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMChannelLoadMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMNoiseHistogramMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMStatisticsMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMLCIMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMLCIAzimuthActivated;
    // oal_bool_enum_uint8 en_dot11RMTransmitStreamCategoryMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMTriggeredTransmitStreamCategoryMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMAPChannelReportActivated;
    // oal_bool_enum_uint8 en_dot11RMMIBActivated;
    oal_uint32 ul_dot11RMMaxMeasurementDuration; /* dot11RMMaxMeasurementDuration Unsigned32, */
    // oal_uint32          ul_dot11RMNonOperatingChannelMaxMeasurementDuration;
    // oal_bool_enum_uint8 en_dot11RMMeasurementPilotTransmissionInformationActivated;
    // oal_uint32          ul_dot11RMMeasurementPilotActivated;
    // oal_bool_enum_uint8 en_dot11RMNeighborReportTSFOffsetActivated;
    // oal_bool_enum_uint8 en_dot11RMRCPIMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMRSNIMeasurementActivated;
    // oal_bool_enum_uint8 en_dot11RMBSSAverageAccessDelayActivated;
    // oal_bool_enum_uint8 en_dot11RMBSSAvailableAdmissionCapacityActivated;
    // oal_bool_enum_uint8 en_dot11RMAntennaInformationActivated;
#endif
    // oal_bool_enum_uint8 en_dot11FastBSSTransitionImplemented;
    // oal_bool_enum_uint8 en_dot11LCIDSEImplemented;                      /* dot11LCIDSEImplemented  TruthValue, */
    // oal_bool_enum_uint8 en_dot11LCIDSERequired;                         /* dot11LCIDSERequired  TruthValue, */
    // oal_bool_enum_uint8 en_dot11DSERequired;                            /* dot11DSERequired  TruthValue, */
    oal_bool_enum_uint8 en_dot11ExtendedChannelSwitchActivated;
    oal_bool_enum_uint8 en_dot11RSNAProtectedManagementFramesActivated;
    // oal_bool_enum_uint8 en_dot11RSNAUnprotectedManagementFramesAllowed;
    oal_uint32 ul_dot11AssociationSAQueryMaximumTimeout;        /* dot11AssociationSAQueryMaximumTimeout Unsigned32, */
    oal_uint32 ul_dot11AssociationSAQueryRetryTimeout;           /* dot11AssociationSAQueryRetryTimeout Unsigned32, */
    oal_bool_enum_uint8 en_dot11HighThroughputOptionImplemented; /* dot11HighThroughputOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11RSNAPBACRequired;                       /* dot11RSNAPBACRequired TruthValue, */
    // oal_bool_enum_uint8 en_dot11PSMPOptionImplemented;                  /* dot11PSMPOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11TunneledDirectLinkSetupImplemented;
    // oal_bool_enum_uint8 en_dot11TDLSPeerUAPSDBufferSTAActivated;
    // oal_bool_enum_uint8 en_dot11TDLSPeerPSMActivated;                   /* dot11TDLSPeerPSMActivated TruthValue, */
    // oal_uint32          ul_dot11TDLSPeerUAPSDIndicationWindow;
    // oal_bool_enum_uint8 en_dot11TDLSChannelSwitchingActivated;
    // oal_uint32          ul_dot11TDLSPeerSTAMissingAckRetryLimit;
    // oal_uint32          ul_dot11TDLSResponseTimeout;                    /* dot11TDLSResponseTimeout Unsigned32, */
    // oal_bool_enum_uint8 en_dot11OCBActivated;                           /* dot11OCBActivated TruthValue, */
    // oal_uint32          ul_dot11TDLSProbeDelay;                         /* dot11TDLSProbeDelay Unsigned32, */
    // oal_uint32          ul_dot11TDLSDiscoveryRequestWindow;
    // oal_uint32          ul_dot11TDLSACDeterminationInterval;
#if defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    oal_bool_enum_uint8 en_dot11WirelessManagementImplemented; /* dot11WirelessManagementImplemented  TruthValue, */
#endif
    // oal_uint32          ul_dot11BssMaxIdlePeriod;                       /* dot11BssMaxIdlePeriod  Unsigned32, */
    // oal_uint8           uc_dot11BssMaxIdlePeriodOptions;    /* dot11BssMaxIdlePeriodOptions  OCTET STRING, SIZE(1)*/
    // oal_uint32          ul_dot11TIMBroadcastInterval;            /* dot11TIMBroadcastInterval      Unsigned32, */
    // oal_int32           l_dot11TIMBroadcastOffset;                /* dot11TIMBroadcastOffset        Integer32, */
    // oal_uint32          ul_dot11TIMBroadcastHighRateTIMRate;     /* dot11TIMBroadcastHighRateTIMRate  Unsigned32, */
    // oal_uint32          ul_dot11TIMBroadcastLowRateTIMRate;     /* dot11TIMBroadcastLowRateTIMRate  Unsigned32, */
    // oal_uint32          ul_dot11StatsMinTriggerTimeout;       /* dot11StatsMinTriggerTimeout      Unsigned32, */
    // oal_bool_enum_uint8 en_dot11RMCivicMeasurementActivated;    /* dot11RMCivicMeasurementActivated  TruthValue, */
    // oal_uint32          ul_dot11RMIdentifierMeasurementActivated;
    // oal_uint32          ul_dot11TimeAdvertisementDTIMInterval;  /* dot11TimeAdvertisementDTIMInterval  Unsigned32, */
    // oal_bool_enum_uint8 en_dot11RM3rdPartyMeasurementActivated; /* dot11RM3rdPartyMeasurementActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11InterworkingServiceImplemented;
    // oal_bool_enum_uint8 en_dot11InterworkingServiceActivated;  /* dot11InterworkingServiceActivated  TruthValue, */
    // oal_bool_enum_uint8 en_dot11QosMapImplemented;                      /* dot11QosMapImplemented  TruthValue, */
    // oal_bool_enum_uint8 en_dot11QosMapActivated;                        /* dot11QosMapActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11EBRImplemented;                         /* dot11EBRImplemented  TruthValue, */
    // oal_bool_enum_uint8 en_dot11EBRActivated;                           /* dot11EBRActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11ESNetwork;                              /* dot11ESNetwork TruthValue, */
    // oal_bool_enum_uint8 en_dot11SSPNInterfaceImplemented;    /* dot11SSPNInterfaceImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11SSPNInterfaceActivated;         /* dot11SSPNInterfaceActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11EASImplemented;                         /* dot11EASImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11EASActivated;                           /* dot11EASActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11MSGCFImplemented;                       /* dot11MSGCFImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11MSGCFActivated;                         /* dot11MSGCFActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11MeshActivated;                          /* dot11MeshActivated TruthValue, */
    // oal_bool_enum_uint8 en_dot11RejectUnadmittedTraffic;        /* dot11RejectUnadmittedTraffic TruthValue, */
    // oal_uint32          ul_dot11BSSBroadcastNullCount;                  /* dot11BSSBroadcastNullCount Unsigned32 */
    oal_bool_enum_uint8 en_dot11VHTOptionImplemented;                 /* dot11VHTOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11OperatingModeNotificationImplemented;
} wlan_mib_Dot11StationConfigEntry_stru;

/******************************************************************************/
/* dot11WEPDefaultKeys  TABLE - members of Dot11WEPDefaultKeysEntry */
/******************************************************************************/
/* Conceptual table for WEP default keys. This table contains the four WEP */
/* default secret key values corresponding to the four possible KeyID values. */
/* The WEP default secret keys are logically WRITE-ONLY. Attempts to read the */
/* entries in this table return unsuccessful status and values of null or 0. */
/* The default value of each WEP default key is null. */
typedef struct {
    // oal_uint32 ul_dot11WEPDefaultKeyIndex;                            /* dot11WEPDefaultKeyIndex Unsigned32, */
    oal_uint8 auc_dot11WEPDefaultKeyValue[WLAN_MAX_WEP_STR_SIZE]; /* dot11WEPDefaultKeyValue WEPKeytype */
    oal_uint8 auc_res[1];
} wlan_mib_Dot11WEPDefaultKeysEntry_stru;

/******************************************************************************/
/* dot11WEPKeyMappings  TABLE - members of Dot11WEPKeyMappingsEntry */
/******************************************************************************/
/* Conceptual table for WEP Key Mappings. The MIB supports the ability to */
/* share a separate WEP key for each RA/TA pair. The Key Mappings Table con- */
/* tains zero or one entry for each MAC address and contains two fields for */
/* each entry: WEPOn and the corresponding WEP key. The WEP key mappings are */
/* logically WRITE-ONLY. Attempts to read the entries in this table return */
/* unsuccessful status and values of null or 0. The default value for all */
/* WEPOn fields is false */
/******************************************************************************/
/* dot11RSNAConfig TABLE (RSNA and TSN) - members of dot11RSNAConfigEntry */
/******************************************************************************/
/* An entry in the dot11RSNAConfigTable */
typedef struct {
    oal_uint8 uc_dot11RSNAConfigPTKSAReplayCounters;
    oal_uint8 uc_dot11RSNAConfigGTKSAReplayCounters;
    oal_uint8 uc_wpa_group_suite;
    oal_uint8 uc_rsn_group_suite;
    oal_uint8 auc_wpa_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    oal_uint8 auc_wpa_akm_suites[WLAN_AUTHENTICATION_SUITES];
    oal_uint8 auc_rsn_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    oal_uint8 auc_rsn_akm_suites[WLAN_AUTHENTICATION_SUITES];
    oal_uint8 uc_rsn_group_mgmt_suite;
} wlan_mib_dot11RSNAConfigEntry_stru;

/******************************************************************************/
/* dot11Privacy TABLE - members of Dot11PrivacyEntry */
/******************************************************************************/
/* Group containing attributes concerned with IEEE 802.11 Privacy. Created */
/* as a table to allow multiple instantiations on an agent. */
typedef struct {
    oal_bool_enum_uint8 en_dot11PrivacyInvoked; /* dot11PrivacyInvoked TruthValue, */
    oal_uint8 uc_dot11WEPDefaultKeyID;          /* dot11WEPDefaultKeyID Unsigned8, */
    oal_uint8 auc_reserve[2];
    // oal_uint32          ul_dot11WEPKeyMappingLengthImplemented; /* dot11WEPKeyMappingLengthImplemented Unsigned32, */
    // oal_bool_enum_uint8 en_dot11ExcludeUnencrypted;             /* dot11ExcludeUnencrypted TruthValue,             */
    // oal_uint32          ul_dot11WEPICVErrorCount;               /* dot11WEPICVErrorCount Counter32,                */
    // oal_uint32          ul_dot11WEPExcludedCount;               /* dot11WEPExcludedCount Counter32,                */
    oal_bool_enum_uint8 en_dot11RSNAActivated;                  /* dot11RSNAActivated TruthValue, */
    oal_bool_enum_uint8 en_dot11RSNAPreauthenticationActivated; /* dot11RSNAPreauthenticationActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAMFPC;               /* dot11RSNAManagementFramerProtectionCapbility TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAMFPR;                /* dot11RSNAManagementFramerProtectionRequired TruthValue */

    wlan_mib_dot11RSNAConfigEntry_stru st_wlan_mib_rsna_cfg;
} wlan_mib_Dot11PrivacyEntry_stru;

/******************************************************************************/
/* dot11MultiDomainCapability TABLE - members of Dot11MultiDomainCapabilityEntry */
/******************************************************************************/
/* GThis (conceptual) table of attributes for cross-domain mobility */
typedef struct {
    oal_uint32 ul_dot11MultiDomainCapabilityIndex; /* dot11MultiDomainCapabilityIndex Unsigned32, */
    oal_uint32 ul_dot11FirstChannelNumber;         /* dot11FirstChannelNumber Unsigned32, */
    oal_uint32 ul_dot11NumberofChannels;           /* dot11NumberofChannels Unsigned32, */
    oal_int32 l_dot11MaximumTransmitPowerLevel;    /* dot11MaximumTransmitPowerLevel Integer32 */
} wlan_mib_Dot11MultiDomainCapabilityEntry_stru;

/******************************************************************************/
/* dot11SpectrumManagement TABLE - members of dot11SpectrumManagementEntry */
/******************************************************************************/
/* An entry (conceptual row) in the Spectrum Management Table. */
/* IfIndex - Each 802.11 interface is represented by an ifEntry. Interface */
/* tables in this MIB are indexed by ifIndex. */
typedef struct {
    oal_uint32 ul_dot11SpectrumManagementIndex;     /* dot11SpectrumManagementIndex Unsigned32, */
    oal_int32 l_dot11MitigationRequirement;         /* dot11MitigationRequirement Integer32, */
    oal_uint32 ul_dot11ChannelSwitchTime;           /* dot11ChannelSwitchTime Unsigned32, */
    oal_int32 l_dot11PowerCapabilityMaxImplemented; /* dot11PowerCapabilityMaxImplemented Integer32, */
    oal_int32 l_dot11PowerCapabilityMinImplemented; /* dot11PowerCapabilityMinImplemented Integer32 */
} wlan_mib_dot11SpectrumManagementEntry_stru;

/*****************************************************************************************/
/* dot11RSNAConfigPairwiseCiphers TABLE - members of dot11RSNAConfigPairwiseCiphersEntry */
/*****************************************************************************************/
/* This table lists the pairwise ciphers supported by this entity. It allows */
/* enabling and disabling of each pairwise cipher by network management. The */
/* pairwise cipher suite list in the RSN element is formed using the informa- */
/* tion in this table. */
typedef struct {
    // oal_uint32          ul_dot11RSNAConfigPairwiseCipherIndex;
    oal_uint8 uc_dot11RSNAConfigPairwiseCipherImplemented;
    oal_bool_enum_uint8 en_dot11RSNAConfigPairwiseCipherActivated;
    oal_uint8 auc_res[2];
    // oal_uint32          ul_dot11RSNAConfigPairwiseCipherSizeImplemented;
} wlan_mib_dot11RSNAConfigPairwiseCiphersEntry_stru;

/***************************************************************************************************/
/* dot11RSNAConfigAuthenticationSuites TABLE - members of Dot11RSNAConfigAuthenticationSuitesEntry */
/***************************************************************************************************/
/* This table lists the AKM suites supported by this entity. Each AKM suite */
/* can be individually enabled and disabled. The AKM suite list in the RSN */
/* element is formed using the information in this table */
typedef struct {
    // oal_uint32          ul_dot11RSNAConfigAuthenticationSuiteIndex;
    oal_uint8 uc_dot11RSNAConfigAuthenticationSuiteImplemented;
    oal_bool_enum_uint8 en_dot11RSNAConfigAuthenticationSuiteActivated;
    oal_uint8 auc_res[2];
} wlan_mib_Dot11RSNAConfigAuthenticationSuitesEntry_stru;

/****************************************************************************/
/* dot11RSNAStats TABLE - members of Dot11RSNAStatsEntry */
/****************************************************************************/
/* This table maintains per-STA statistics in an RSN. The entry with */
/* dot11RSNAStatsSTAAddress equal to FF-FF-FF-FF-FF-FF contains statistics */
/* for group addressed traffic */
typedef struct {
    // oal_uint32 ul_dot11RSNAStatsIndex;                      /* dot11RSNAStatsIndex Unsigned32,                    */
    // oal_uint32 ul_dot11RSNAStatsVersion;                    /* dot11RSNAStatsVersion Unsigned32,                  */
    // oal_uint8  uc_dot11RSNAStatsSelectedPairwiseCipher;
    // oal_uint32 ul_dot11RSNAStatsTKIPICVErrors;              /* dot11RSNAStatsTKIPICVErrors Counter32,             */
    // oal_uint32 ul_dot11RSNAStatsTKIPLocalMICFailures;       /* dot11RSNAStatsTKIPLocalMICFailures Counter32,      */
    // oal_uint32 ul_dot11RSNAStatsTKIPRemoteMICFailures;      /* dot11RSNAStatsTKIPRemoteMICFailures Counter32,     */
    // oal_uint32 ul_dot11RSNAStatsCCMPReplays;                /* dot11RSNAStatsCCMPReplays Counter32,               */
    // oal_uint32 ul_dot11RSNAStatsCCMPDecryptErrors;          /* dot11RSNAStatsCCMPDecryptErrors Counter32,         */
    // oal_uint32 ul_dot11RSNAStatsTKIPReplays;                /* dot11RSNAStatsTKIPReplays Counter32,               */
    oal_uint32 ul_dot11RSNAStatsCMACICVErrors; /* dot11RSNAStatsCMACICVErrors Counter32, */
    oal_uint32 ul_dot11RSNAStatsCMACReplays;   /* dot11RSNAStatsCMACReplays Counter32, */
    // oal_uint32 ul_dot11RSNAStatsRobustMgmtCCMPReplays;      /* dot11RSNAStatsRobustMgmtCCMPReplays Counter32,     */
    // oal_uint32 ul_dot11RSNABIPMICErrors;                    /* dot11RSNABIPMICErrors Counter32                    */
} wlan_mib_Dot11RSNAStatsEntry_stru;

/***********************************************************************/
/* dot11OperatingClasses TABLE - members of Dot11OperatingClassesEntry */
/***********************************************************************/
/* (Conceptual) table of attributes for operating classes */
typedef struct {
    oal_uint32 ul_dot11OperatingClassesIndex; /* dot11OperatingClassesIndex Unsigned32, */
    oal_uint32 ul_dot11OperatingClass;        /* dot11OperatingClass Unsigned32, */
    oal_uint32 ul_dot11CoverageClass;         /* dot11CoverageClass Unsigned32 */
} wlan_mib_Dot11OperatingClassesEntry_stru;

/***********************************************************************/
/* dot11RMRequest TABLE  - members of dot11RadioMeasurement */
/***********************************************************************/
typedef struct {
    /* dot11RMRequestNextIndex ::= { dot11RMRequest 1 } */
    oal_uint32 ul_dot11RMRequestNextIndex; /* dot11RMRequestNextIndex  Unsigned32 */

    /* dot11RMRequestTable OBJECT-TYPE ::= { dot11RMRequest 2 } */
    oal_uint32 ul_dot11RMRqstIndex;    /* dot11RMRqstIndex Unsigned32, */
    wlan_mib_row_status_enum_uint8 en_dot11RMRqstRowStatus; /* dot11RMRqstRowStatus RowStatus, */
    oal_uint8 auc_dot11RMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];      /* dot11RMRqstToken OCTET STRING, */
    oal_uint32 ul_dot11RMRqstRepetitions;   /* dot11RMRqstRepetitions Unsigned32, */
    oal_uint32 ul_dot11RMRqstIfIndex;   /* dot11RMRqstIfIndex InterfaceIndex, */
    wlan_mib_rmrqst_type_enum_uint16 en_dot11RMRqstType;   /* dot11RMRqstType INTEGER, */
    oal_uint8 auc_dot11RMRqstTargetAdd[6];     /* dot11RMRqstTargetAdd MacAddress, */
    oal_uint32 ul_dot11RMRqstTimeStamp;        /* dot11RMRqstTimeStamp TimeTicks, */
    oal_uint32 ul_dot11RMRqstChanNumber;       /* dot11RMRqstChanNumber Unsigned32, */
    oal_uint32 ul_dot11RMRqstOperatingClass;   /* dot11RMRqstOperatingClass Unsigned32, */
    oal_uint32 ul_dot11RMRqstRndInterval;      /* dot11RMRqstRndInterval Unsigned32, */
    oal_uint32 ul_dot11RMRqstDuration;         /* dot11RMRqstDuration Unsigned32, */
    oal_bool_enum_uint8 en_dot11RMRqstParallel;    /* dot11RMRqstParallel TruthValue, */
    oal_bool_enum_uint8 en_dot11RMRqstEnable;      /* dot11RMRqstEnable TruthValue, */
    oal_bool_enum_uint8 en_dot11RMRqstRequest;     /* dot11RMRqstRequest TruthValue, */
    oal_bool_enum_uint8 en_dot11RMRqstReport;      /* dot11RMRqstReport TruthValue, */
    oal_bool_enum_uint8 en_dot11RMRqstDurationMandatory;  /* dot11RMRqstDurationMandatory TruthValue, */
    wlan_mib_rmrqst_beaconrqst_mode_enum_uint8 en_dot11RMRqstBeaconRqstMode;  /* dot11RMRqstBeaconRqstMode INTEGER, */
    wlan_mib_rmrqst_beaconrqst_detail_enum_uint8 en_dot11RMRqstBeaconRqstDetail;
    wlan_mib_rmrqst_famerqst_type_enum_uint8  en_dot11RMRqstFrameRqstType;  /* dot11RMRqstFrameRqstType INTEGER, */
    oal_uint8 auc_dot11RMRqstBssid[6];     /* dot11RMRqstBssid MacAddress, */
    oal_uint8 uc_dot11RMRqstSSID[32];      /* dot11RMRqstSSID OCTET STRING, SIZE(0..32) */
    wlan_mib_rmrqst_beaconrpt_cdt_enum_uint8 en_dot11RMRqstBeaconReportingCondition;
    oal_int32 l_dot11RMRqstBeaconThresholdOffset;    /* dot11RMRqstBeaconThresholdOffset Integer32, */
    wlan_mib_rmrqst_stastatrqst_grpid_enum_uint8 en_dot11RMRqstSTAStatRqstGroupID;
    wlan_mib_rmrqst_lcirpt_subject_enum_uint8 en_dot11RMRqstLCIRqstSubject;   /* dot11RMRqstLCIRqstSubject INTEGER, */
    oal_uint32 ul_dot11RMRqstLCILatitudeResolution;   /* dot11RMRqstLCILatitudeResolution Unsigned32, */
    oal_uint32 ul_dot11RMRqstLCILongitudeResolution;  /* dot11RMRqstLCILongitudeResolution Unsigned32, */
    oal_uint32 ul_dot11RMRqstLCIAltitudeResolution;   /* dot11RMRqstLCIAltitudeResolution Unsigned32, */
    wlan_mib_rmrqst_lciazimuth_type_enum_uint8 en_dot11RMRqstLCIAzimuthType;   /* dot11RMRqstLCIAzimuthType INTEGER, */
    oal_uint32 ul_dot11RMRqstLCIAzimuthResolution;     /* dot11RMRqstLCIAzimuthResolution Unsigned32, */
    oal_uint32 ul_dot11RMRqstPauseTime;           /* dot11RMRqstPauseTime Unsigned32, */
    oal_uint8 auc_dot11RMRqstTransmitStreamPeerQSTAAddress[6];
    oal_uint32 ul_dot11RMRqstTransmitStreamTrafficIdentifier;
    oal_uint32 ul_dot11RMRqstTransmitStreamBin0Range;  /* dot11RMRqstTransmitStreamBin0Range Unsigned32, */
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSAverageCondition;  /* dot11RMRqstTrigdQoSAverageCondition TruthValue, */
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSConsecutiveCondition;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSDelayCondition;    /* dot11RMRqstTrigdQoSDelayCondition TruthValue, */
    oal_uint32 ul_dot11RMRqstTrigdQoSAverageThreshold;   /* dot11RMRqstTrigdQoSAverageThreshold Unsigned32, */
    oal_uint32 ul_dot11RMRqstTrigdQoSConsecutiveThreshold; /* dot11RMRqstTrigdQoSConsecutiveThreshold  Unsigned32, */
    oal_uint32 ul_dot11RMRqstTrigdQoSDelayThresholdRange;  /* dot11RMRqstTrigdQoSDelayThresholdRange Unsigned32, */
    oal_uint32 ul_dot11RMRqstTrigdQoSDelayThreshold; /* dot11RMRqstTrigdQoSDelayThreshold Unsigned32, */
    oal_uint32 ul_dot11RMRqstTrigdQoSMeasurementCount; /* dot11RMRqstTrigdQoSMeasurementCount Unsigned32, */
    oal_uint32 ul_dot11RMRqstTrigdQoSTimeout;  /* dot11RMRqstTrigdQoSTimeout Unsigned32, */
    wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum_uint8 en_dot11RMRqstChannelLoadReportingCondition;
    oal_uint32 ul_dot11RMRqstChannelLoadReference; /* dot11RMRqstChannelLoadReference Unsigned32, */
    wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum_uint8 en_dot11RMRqstNoiseHistogramReportingCondition;
    oal_uint32 ul_dot11RMRqstAnpiReference;   /* dot11RMRqstAnpiReference Unsigned32, */
    oal_uint8 auc_dot11RMRqstAPChannelReport[255];   /* dot11RMRqstAPChannelReport OCTET STRING, SIZE(0..255) */
    oal_uint8 auc_dot11RMRqstSTAStatPeerSTAAddress[6];   /* dot11RMRqstSTAStatPeerSTAAddress MacAddress, */
    oal_uint8 auc_dot11RMRqstFrameTransmitterAddress[6];  /* dot11RMRqstFrameTransmitterAddress MacAddress, */
    oal_uint8 auc_dot11RMRqstVendorSpecific[255]; /* dot11RMRqstVendorSpecific OCTET STRING,SIZE(0..255) */
    oal_uint32 ul_dot11RMRqstSTAStatTrigMeasCount;   /* dot11RMRqstSTAStatTrigMeasCount Unsigned32, */
    oal_uint32 ul_dot11RMRqstSTAStatTrigTimeout;      /* dot11RMRqstSTAStatTrigTimeout Unsigned32, */
    oal_uint8 auc_dot11RMRqstSTAStatTrigCondition[2];  /* dot11RMRqstSTAStatTrigCondition OCTET STRING, SIZE(2) */
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTAFailedCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTAFCSErrCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTAMultRetryCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTAFrameDupeCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTARTSFailCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTAAckFailCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigSTARetryCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSTrigCondition;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSFailedCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSRetryCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSMultRetryCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSFrameDupeCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSRTSFailCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSAckFailCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigQoSDiscardCntThresh;
    oal_uint8 auc_dot11RMRqstSTAStatTrigRsnaTrigCondition[2];
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaCMACICVErrCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaCMACReplayCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaRobustCCMPReplayCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaTKIPICVErrCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaTKIPReplayCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaCCMPDecryptErrCntThresh;
    oal_uint32 ul_dot11RMRqstSTAStatTrigRsnaCCMPReplayCntThresh;
} wlan_mib_dot11RadioMeasurement_stru;

/**************************************************************************************/
/* dot11FastBSSTransitionConfig TABLE  - members of Dot11FastBSSTransitionConfigEntry */
/**************************************************************************************/
/* The table containing fast BSS transition configuration objects */
typedef struct {
    oal_bool_enum_uint8 en_dot11FastBSSTransitionActivated; /* dot11FastBSSTransitionActivated TruthValue, */
    oal_uint8 auc_dot11FTMobilityDomainID[2];               /* dot11FTMobilityDomainID OCTET STRING,SIZE(2) */
    oal_bool_enum_uint8 en_dot11FTOverDSActivated;          /* dot11FTOverDSActivated TruthValue, */
    oal_bool_enum_uint8 en_dot11FTResourceRequestSupported; /* dot11FTResourceRequestSupported TruthValue, */
    // oal_uint32          ul_dot11FTR0KeyLifetime;             /* dot11FTR0KeyLifetime Unsigned32,            */
    // oal_uint32          ul_dot11FTReassociationDeadline;     /* dot11FTReassociationDeadline Unsigned32     */
} wlan_mib_Dot11FastBSSTransitionConfigEntry_stru;

/************************************************************************/
/* dot11LCIDSE TABLE  - members of Dot11LCIDSEEntry */
/************************************************************************/
/* Group contains conceptual table of attributes for Dependent Station */
/* Enablement. */
typedef struct {
    oal_uint32 ul_dot11LCIDSEIndex;                                      /* dot11LCIDSEIndex Unsigned32, */
    oal_uint32 ul_dot11LCIDSEIfIndex;                                    /* dot11LCIDSEIfIndex InterfaceIndex, */
    oal_uint32 ul_dot11LCIDSECurrentOperatingClass;                  /* dot11LCIDSECurrentOperatingClass Unsigned32, */
    oal_uint32 ul_dot11LCIDSELatitudeResolution;                      /* dot11LCIDSELatitudeResolution Unsigned32, */
    oal_int32 l_dot11LCIDSELatitudeInteger;                              /* dot11LCIDSELatitudeInteger Integer32, */
    oal_int32 l_dot11LCIDSELatitudeFraction;                             /* dot11LCIDSELatitudeFraction Integer32, */
    oal_uint32 ul_dot11LCIDSELongitudeResolution;                    /* dot11LCIDSELongitudeResolution Unsigned32, */
    oal_int32 l_dot11LCIDSELongitudeInteger;                             /* dot11LCIDSELongitudeInteger Integer32, */
    oal_int32 l_dot11LCIDSELongitudeFraction;                            /* dot11LCIDSELongitudeFraction Integer32, */
    wlan_mib_lci_dsealtitude_type_enum_uint8 en_dot11LCIDSEAltitudeType; /* dot11LCIDSEAltitudeType INTEGER, */
    oal_uint32 ul_dot11LCIDSEAltitudeResolution;                         /* dot11LCIDSEAltitudeResolution Unsigned32, */
    oal_int32 l_dot11LCIDSEAltitudeInteger;                              /* dot11LCIDSEAltitudeInteger Integer32, */
    oal_int32 l_dot11LCIDSEAltitudeFraction;                             /* dot11LCIDSEAltitudeFraction Integer32, */
    oal_uint32 ul_dot11LCIDSEDatum;                                      /* dot11LCIDSEDatum Unsigned32, */
    oal_bool_enum_uint8 en_dot11RegLocAgreement;                         /* dot11RegLocAgreement TruthValue, */
    oal_bool_enum_uint8 en_dot11RegLocDSE;                               /* dot11RegLocDSE TruthValue, */
    oal_bool_enum_uint8 en_dot11DependentSTA;                            /* dot11DependentSTA TruthValue, */
    oal_uint32 ul_dot11DependentEnablementIdentifier;             /* dot11DependentEnablementIdentifier Unsigned32, */
    oal_uint32 ul_dot11DSEEnablementTimeLimit;                           /* dot11DSEEnablementTimeLimit Unsigned32, */
    oal_uint32 ul_dot11DSEEnablementFailHoldTime;                     /* dot11DSEEnablementFailHoldTime Unsigned32, */
    oal_uint32 ul_dot11DSERenewalTime;                                   /* dot11DSERenewalTime Unsigned32, */
    oal_uint32 ul_dot11DSETransmitDivisor;                               /* dot11DSETransmitDivisor Unsigned32 */
} wlan_mib_Dot11LCIDSEEntry_stru;

/**************************************************************************************/
/* dot11HTStationConfig TABLE  - members of Dot11HTStationConfigEntry */
/**************************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple */
/* instances on an agent. */
typedef struct {
    // wlan_mib_ht_op_mcs_set_stru st_dot11HTOperationalMCSSet;
    wlan_mib_mimo_power_save_enum_uint8 en_dot11MIMOPowerSave; /* dot11MIMOPowerSave INTEGER, */
    // oal_bool_enum_uint8 en_dot11NDelayedBlockAckOptionImplemented;
    wlan_mib_max_amsdu_lenth_enum_uint16 en_dot11MaxAMSDULength; /* dot11MaxAMSDULength INTEGER, */
    // oal_bool_enum_uint8 en_dot11STBCControlFrameOptionImplemented;
    oal_bool_enum_uint8 en_dot11LsigTxopProtectionOptionImplemented;
    oal_uint32 ul_dot11MaxRxAMPDUFactor;            /* dot11MaxRxAMPDUFactor Unsigned32, */
    oal_uint32 ul_dot11MinimumMPDUStartSpacing;               /* dot11MinimumMPDUStartSpacing Unsigned32, */
    oal_bool_enum_uint8 en_dot11PCOOptionImplemented;        /* dot11PCOOptionImplemented TruthValue, */
    oal_uint32 ul_dot11TransitionTime;                         /* dot11TransitionTime Unsigned32, */
    wlan_mib_mcs_feedback_opt_implt_enum_uint8 en_dot11MCSFeedbackOptionImplemented;
    oal_bool_enum_uint8 en_dot11HTControlFieldSupported;        /* dot11HTControlFieldSupported TruthValue, */
    oal_bool_enum_uint8 en_dot11RDResponderOptionImplemented;    /* dot11RDResponderOptionImplemented TruthValue, */
    // oal_bool_enum_uint8 en_dot11SPPAMSDUCapable;     /* dot11SPPAMSDUCapable TruthValue,                     */
    // oal_bool_enum_uint8 en_dot11SPPAMSDURequired;    /* dot11SPPAMSDURequired TruthValue,                    */
    // oal_bool_enum_uint8 en_dot11FortyMHzOptionImplemented;
} wlan_mib_Dot11HTStationConfigEntry_stru;

/**************************************************************************************/
/* dot11WirelessMgmtOptions TABLE  - members of Dot11WirelessMgmtOptionsEntry */
/**************************************************************************************/
/* Wireless Management attributes. In tabular form to allow for multiple */
/* instances on an agent. This table only applies to the interface if */
/* dot11WirelessManagementImplemented is set to true in the */
/* dot11StationConfigTable. Otherwise this table should be ignored. */
/* For all Wireless Management features, an Activated MIB variable is used */
/* to activate/enable or deactivate/disable the corresponding feature. */
/* An Implemented MIB variable is used for an optional feature to indicate */
/* whether the feature is implemented. A mandatory feature does not have a */
/* corresponding Implemented MIB variable. It is possible for there to be */
/* multiple IEEE 802.11 interfaces on one agent, each with its unique MAC */
/* address. The relationship between an IEEE 802.11 interface and an */
/* interface in the context of the Internet standard MIB is one-to-one. */
/* As such, the value of an ifIndex object instance can be directly used */
/* to identify corresponding instances of the objects defined herein. */
/* ifIndex - Each IEEE 802.11 interface is represented by an ifEntry. */
/* Interface tables in this MIB module are indexed by ifIndex. */
typedef struct {
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionEventsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTriggerSTAStatisticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMulticastDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetActivated;
#if defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    oal_bool_enum_uint8 en_dot11FineTimingMsmtInitActivated;
    oal_bool_enum_uint8 en_dot11FineTimingMsmtRespActivated;
    oal_bool_enum_uint8 en_dot11LciCivicInNeighborReport;
    oal_bool_enum_uint8 en_dot11RMFineTimingMsmtRangeRepImplemented;
    oal_bool_enum_uint8 en_dot11RMFineTimingMsmtRangeRepActivated;
    oal_bool_enum_uint8 en_dot11RMLCIConfigured;                     /* dot11RMLCIConfigured TruthValue, */
    oal_bool_enum_uint8 en_dot11RMCivicConfigured;                   /* dot11RMCivicConfigured TruthValue */
#endif
} wlan_mib_Dot11WirelessMgmtOptionsEntry_stru;

/* dot11LocationServicesLocationIndicationChannelList OBJECT-TYPE */
/* SYNTAX OCTET STRING (SIZE (2..254)) */
/* MAX-ACCESS read-create */
/* STATUS current */
/* DESCRIPTION */
/* "This attribute contains one or more Operating Class and Channel octet */
/* pairs." */
/* ::= { dot11LocationServicesEntry 12} */
typedef struct {
    oal_uint8 uc_channel_nums;
    oal_uint8 *uc_channel_list;
} wlan_mib_local_serv_location_ind_ch_list_stru;

/********************************************************************************/
/* dot11LocationServices TABLE  - members of Dot11LocationServicesEntry */
/********************************************************************************/
/* Identifies a hint for the next value of dot11LocationServicesIndex to be */
/* used in a row creation attempt for dot11LocationServicesTable. If no new */
/* rows can be created for some reason, such as memory, processing require- */
/* ments, etc, the SME shall set this attribute to 0. It shall update this */
/* attribute to a proper value other than 0 as soon as it is capable of */
/* receiving new measurement requests. The nextIndex is not necessarily */
/* sequential nor monotonically increasing. */
typedef struct {
    oal_uint32
    ul_dot11LocationServicesIndex;
    oal_uint8 auc_dot11LocationServicesMACAddress[6];
    oal_uint8 auc_dot11LocationServicesLIPIndicationMulticastAddress[6];
    wlan_mib_lct_servs_liprpt_interval_unit_enum_uint8 en_dot11LocationServicesLIPReportIntervalUnits;
    oal_uint32 ul_dot11LocationServicesLIPNormalReportInterval;
    oal_uint32 ul_dot11LocationServicesLIPNormalFramesperChannel;
    oal_uint32 ul_dot11LocationServicesLIPInMotionReportInterval;
    oal_uint32 ul_dot11LocationServicesLIPInMotionFramesperChannel;
    oal_uint32 ul_dot11LocationServicesLIPBurstInterframeInterval;
    oal_uint32 ul_dot11LocationServicesLIPTrackingDuration;
    oal_uint32 ul_dot11LocationServicesLIPEssDetectionInterval;
    wlan_mib_local_serv_location_ind_ch_list_stru st_dot11LocationServicesLocationIndicationChannelList;
    oal_uint32 ul_dot11LocationServicesLocationIndicationBroadcastDataRate;
    oal_uint8 uc_dot11LocationServicesLocationIndicationOptionsUsed;
    wlan_mib_local_serv_location_ind_ind_para_stru st_dot11LocationServicesLocationIndicationIndicationParameters;
    oal_uint32 ul_dot11LocationServicesLocationStatus;
} wlan_mib_Dot11LocationServicesEntry_stru;

/********************************************************************************/
/* dot11WirelessMGTEvent TABLE  - members of Dot11WirelessMGTEventEntry */
/********************************************************************************/
/* Group contains the current list of WIRELESS Management reports that have */
/* been received by the MLME. The report tables shall be maintained as FIFO */
/* to preserve freshness, thus the rows in this table can be deleted for mem- */
/* ory constraints or other implementation constraints determined by the ven- */
/* dor. New rows shall have different RprtIndex values than those deleted within */
/* the range limitation of the index. One easy way is to monotonically */
/* increase the EventIndex for new reports being written in the table**/
typedef struct {
    oal_uint32 ul_dot11WirelessMGTEventIndex;
    oal_uint8 auc_dot11WirelessMGTEventMACAddress[6];
    wlan_mib_wireless_mgt_event_type_enum_uint8 en_dot11WirelessMGTEventType;
    wlan_mib_wireless_mgt_event_status_enum_uint8 en_dot11WirelessMGTEventStatus;
    oal_uint8 auc_dot11WirelessMGTEventTSF[8];
    oal_uint8 auc_dot11WirelessMGTEventUTCOffset[10];
    oal_uint8 auc_dot11WirelessMGTEventTimeError[5];
    oal_uint8 auc_dot11WirelessMGTEventTransitionSourceBSSID[6];
    oal_uint8 auc_dot11WirelessMGTEventTransitionTargetBSSID[6];
    oal_uint32 ul_dot11WirelessMGTEventTransitionTime;
    wlan_mib_wireless_mgt_event_transit_reason_enum_uint8 en_dot11WirelessMGTEventTransitionReason;
    oal_uint32 ul_dot11WirelessMGTEventTransitionResult;
    oal_uint32 ul_dot11WirelessMGTEventTransitionSourceRCPI;
    oal_uint32 ul_dot11WirelessMGTEventTransitionSourceRSNI;
    oal_uint32 ul_dot11WirelessMGTEventTransitionTargetRCPI;
    oal_uint32 ul_dot11WirelessMGTEventTransitionTargetRSNI;
    oal_uint8 auc_dot11WirelessMGTEventRSNATargetBSSID[6];
    oal_uint8 auc_dot11WirelessMGTEventRSNAAuthenticationType[4];
    oal_uint8 auc_dot11WirelessMGTEventRSNAEAPMethod[8];
    oal_uint32 ul_dot11WirelessMGTEventRSNAResult;
    oal_uint8 auc_dot11WirelessMGTEventRSNARSNElement[257];
    oal_uint8 uc_dot11WirelessMGTEventPeerSTAAddress;
    oal_uint32 ul_dot11WirelessMGTEventPeerOperatingClass;
    oal_uint32 ul_dot11WirelessMGTEventPeerChannelNumber;
    oal_int32 l_dot11WirelessMGTEventPeerSTATxPower;
    oal_uint32 ul_dot11WirelessMGTEventPeerConnectionTime;
    oal_uint32 ul_dot11WirelessMGTEventPeerPeerStatus;
    oal_uint8 auc_dot11WirelessMGTEventWNMLog[2284];
} wlan_mib_Dot11WirelessMGTEventEntry_stru;

typedef struct {
    /* dot11WNMRequest OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 1 } */
    /* dot11WNMRequestNextIndex Unsigned32(0..4294967295) ::= { dot11WNMRequest 1 } */
    oal_uint32 ul_dot11WNMRequestNextIndex;

    /* dot11WNMRequestTable ::= { dot11WNMRequest 2 } */
    oal_uint32 ul_dot11WNMRqstIndex;
    wlan_mib_row_status_enum_uint8 en_dot11WNMRqstRowStatus;
    oal_uint8 auc_dot11WNMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32 ul_dot11WNMRqstIfIndex;
    wlan_mib_wnm_rqst_type_enum_uint8 en_dot11WNMRqstType;
    oal_uint8 auc_dot11WNMRqstTargetAdd[6];
    oal_uint32 ul_dot11WNMRqstTimeStamp;
    oal_uint32 ul_dot11WNMRqstRndInterval;
    oal_uint32 ul_dot11WNMRqstDuration;
    oal_uint8 auc_dot11WNMRqstMcstGroup[6];
    oal_uint8 uc_dot11WNMRqstMcstTrigCon;
    oal_uint32 ul_dot11WNMRqstMcstTrigInactivityTimeout;
    oal_uint32 ul_dot11WNMRqstMcstTrigReactDelay;
    wlan_mib_wnm_rqst_lcrrqst_subj_enum_uint8 en_dot11WNMRqstLCRRqstSubject;
    wlan_mib_wnm_rqst_lcr_interval_unit_enum_uint8 en_dot11WNMRqstLCRIntervalUnits;
    oal_uint32 ul_dot11WNMRqstLCRServiceInterval;
    wlan_mib_wnm_rqst_lirrqst_subj_enum_uint8 en_dot11WNMRqstLIRRqstSubject;
    wlan_mib_wnm_rqst_lir_interval_unit_enum_uint8 en_dot11WNMRqstLIRIntervalUnits;
    oal_uint32 ul_dot11WNMRqstLIRServiceInterval;
    oal_uint32 ul_dot11WNMRqstEventToken;
    wlan_mib_wnm_rqst_event_type_enum_uint8 en_dot11WNMRqstEventType;
    oal_uint32 ul_dot11WNMRqstEventResponseLimit;
    oal_uint8 auc_dot11WNMRqstEventTargetBssid[6];
    oal_uint8 auc_dot11WNMRqstEventSourceBssid[6];
    oal_uint32 ul_dot11WNMRqstEventTransitTimeThresh;
    oal_uint8 uc_dot11WNMRqstEventTransitMatchValue;
    oal_uint32 ul_dot11WNMRqstEventFreqTransitCountThresh;
    oal_uint32 ul_dot11WNMRqstEventFreqTransitInterval;
    oal_uint8 auc_dot11WNMRqstEventRsnaAuthType[4];
    oal_uint32 ul_dot11WNMRqstEapType;
    oal_uint8 auc_dot11WNMRqstEapVendorId[3];
    oal_uint8 auc_dot11WNMRqstEapVendorType[4];
    oal_uint8 uc_dot11WNMRqstEventRsnaMatchValue;
    oal_uint8 auc_dot11WNMRqstEventPeerMacAddress[6];
    oal_uint32 ul_dot11WNMRqstOperatingClass;
    oal_uint32 ul_dot11WNMRqstChanNumber;
    oal_uint32 ul_dot11WNMRqstDiagToken;
    wlan_mib_wnm_rqst_diag_type_enum_uint8 en_dot11WNMRqstDiagType;
    oal_uint32 ul_dot11WNMRqstDiagTimeout;
    oal_uint8 auc_dot11WNMRqstDiagBssid[6];
    oal_uint32 ul_dot11WNMRqstDiagProfileId;
    wlan_mib_wnm_rqst_diag_credent_enum_uint8 en_dot11WNMRqstDiagCredentials;
    oal_uint8 auc_dot11WNMRqstLocConfigLocIndParams[16];
    oal_uint8 auc_dot11WNMRqstLocConfigChanList[252];
    oal_uint32 ul_dot11WNMRqstLocConfigBcastRate;
    oal_uint8 auc_dot11WNMRqstLocConfigOptions[255];
    wlan_mib_wnm_rqst_bss_transit_query_reason_enum_uint8 en_dot11WNMRqstBssTransitQueryReason;
    oal_uint8 uc_dot11WNMRqstBssTransitReqMode;
    oal_uint32 ul_dot11WNMRqstBssTransitDisocTimer;

    /* This is a control variable.It is written by an external management entity when making a management
       request. Changes take effect when dot11WNMRqstRowStatus is set to Active.
       This attribute contains a variable-length field formatted in accordance with IETF RFC 3986-2005." */
    oal_uint8   auc_dot11WNMRqstBssTransitSessInfoURL[255];
    oal_uint8                   auc_dot11WNMRqstBssTransitCandidateList[2304];
    oal_bool_enum_uint8         en_dot11WNMRqstColocInterfAutoEnable;
    oal_uint32                  ul_dot11WNMRqstColocInterfRptTimeout;
    oal_uint8                   auc_dot11WNMRqstVendorSpecific[255];
    oal_uint8                   auc_dot11WNMRqstDestinationURI[253];

    /* dot11WNMReport OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 2 } */
    /* dot11WNMVendorSpecificReportTable::= { dot11WNMReport 1 }                 */
    oal_uint32          ul_dot11WNMVendorSpecificRprtIndex;
    oal_uint8           auc_dot11WNMVendorSpecificRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMVendorSpecificRprtIfIndex;
    oal_uint8           auc_dot11WNMVendorSpecificRprtContent[255];

    /* dot11WNMMulticastDiagnosticReportTable ::= { dot11WNMReport 2 } */
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtIndex;
    oal_uint8           auc_dot11WNMMulticastDiagnosticRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtIfIndex;
    oal_uint8           auc_dot11WNMMulticastDiagnosticRprtMeasurementTime[8];
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtDuration;
    oal_uint8           auc_dot11WNMMulticastDiagnosticRprtMcstGroup[6];
    oal_uint8           uc_dot11WNMMulticastDiagnosticRprtReason;
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtRcvdMsduCount;
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtFirstSeqNumber;
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtLastSeqNumber;
    oal_uint32          ul_dot11WNMMulticastDiagnosticRprtMcstRate;
    /* dot11WNMLocationCivicReportTable ::= { dot11WNMReport 3 } */
    oal_uint32          ul_dot11WNMLocationCivicRprtIndex;
    oal_uint8           auc_dot11WNMLocationCivicRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMLocationCivicRprtIfIndex;

    /* This is a status variable.                                             */
    /* It is written by the SME when a management report is completed.        */
    /* This attribute indicates a variable octet field and contains a list of */
    /* civic address elements in TLV format as defined in IETF RFC 4776-2006  */
    oal_uint8           auc_dot11WNMLocationCivicRprtCivicLocation[255];

    /* dot11WNMLocationIdentifierReportTable ::= { dot11WNMReport 4 } */
    oal_uint32          ul_dot11WNMLocationIdentifierRprtIndex;
    oal_uint8           auc_dot11WNMLocationIdentifierRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMLocationIdentifierRprtIfIndex;
    oal_uint8           auc_dot11WNMLocationIdentifierRprtExpirationTSF[8];
    oal_uint8           uc_dot11WNMLocationIdentifierRprtPublicIdUri;

    /* dot11WNMEventTransitReportTable ::= { dot11WNMReport 5 } */
    oal_uint32          ul_dot11WNMEventTransitRprtIndex;
    oal_uint8           auc_dot11WNMEventTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMEventTransitRprtIfIndex;
    wlan_mib_wnm_event_transit_rprt_event_status_enum_uint8 en_dot11WNMEventTransitRprtEventStatus;
    oal_uint8           auc_dot11WNMEventTransitRprtEventTSF[8];
    oal_uint8           auc_dot11WNMEventTransitRprtUTCOffset[10];
    oal_uint8           auc_dot11WNMEventTransitRprtTimeError[5];
    oal_uint8           auc_dot11WNMEventTransitRprtSourceBssid[6];
    oal_uint8           auc_dot11WNMEventTransitRprtTargetBssid[6];
    oal_uint32          ul_dot11WNMEventTransitRprtTransitTime;
    wlan_mib_wnm_event_transitrprt_transit_reason_enum_uint8  en_dot11WNMEventTransitRprtTransitReason;
    oal_uint32          ul_dot11WNMEventTransitRprtTransitResult;
    oal_uint32          ul_dot11WNMEventTransitRprtSourceRCPI;
    oal_uint32          ul_dot11WNMEventTransitRprtSourceRSNI;
    oal_uint32          ul_dot11WNMEventTransitRprtTargetRCPI;
    oal_uint32          ul_dot11WNMEventTransitRprtTargetRSNI;

    /* dot11WNMEventRsnaReportTable ::= { dot11WNMReport 6 } */
    oal_uint32          ul_dot11WNMEventRsnaRprtIndex;           /* dot11WNMEventRsnaRprtIndex Unsigned32,       */
    oal_uint8           auc_dot11WNMEventRsnaRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMEventRsnaRprtIfIndex;
    wlan_mib_wnm_event_rsnarprt_event_status_enum_uint8 en_dot11WNMEventRsnaRprtEventStatus;
    oal_uint8           auc_dot11WNMEventRsnaRprtEventTSF[8];
    oal_uint8           auc_dot11WNMEventRsnaRprtUTCOffset[10];
    oal_uint8           auc_dot11WNMEventRsnaRprtTimeError[5];
    oal_uint8           auc_dot11WNMEventRsnaRprtTargetBssid[6];
    oal_uint8           auc_dot11WNMEventRsnaRprtAuthType[4];
    oal_uint8           auc_dot11WNMEventRsnaRprtEapMethod[8];
    oal_uint32          ul_dot11WNMEventRsnaRprtResult;
    oal_uint8           uc_dot11WNMEventRsnaRprtRsnElement;

    /* dot11WNMEventPeerReportTable ::= { dot11WNMReport 7 } */
    oal_uint32          ul_dot11WNMEventPeerRprtIndex;
    oal_uint8           auc_dot11WNMEventPeerRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMEventPeerRprtIfIndex;
    oal_uint8           uc_dot11WNMEventPeerRprtEventStatus;
    oal_uint8           auc_dot11WNMEventPeerRprtEventTSF[8];
    oal_uint8           auc_dot11WNMEventPeerRprtUTCOffset[10];
    oal_uint8           auc_dot11WNMEventPeerRprtTimeError[5];
    oal_uint8           auc_dot11WNMEventPeerRprtPeerMacAddress[6];
    oal_uint32          ul_dot11WNMEventPeerRprtOperatingClass;
    oal_uint32          ul_dot11WNMEventPeerRprtChanNumber;
    oal_int32           l_dot11WNMEventPeerRprtStaTxPower;
    oal_uint32          ul_dot11WNMEventPeerRprtConnTime;
    oal_uint8           uc_dot11WNMEventPeerRprtPeerStatus;

    /* dot11WNMEventWNMLogReportTable ::= { dot11WNMReport 8 } */
    oal_uint32          ul_dot11WNMEventWNMLogRprtIndex;          /* dot11WNMEventWNMLogRprtIndex Unsigned32,       */
    oal_uint8           auc_dot11WNMEventWNMLogRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMEventWNMLogRprtIfIndex;
    oal_uint8           uc_dot11WNMEventWNMLogRprtEventStatus;
    oal_uint8           auc_dot11WNMEventWNMLogRprtEventTSF[8];
    oal_uint8           auc_dot11WNMEventWNMLogRprtUTCOffset[10];
    oal_uint8           auc_dot11WNMEventWNMLogRprtTimeError[5];
    oal_uint8           auc_dot11WNMEventWNMLogRprtContent[2284];

    /* dot11WNMDiagMfrInfoReportTable ::= { dot11WNMReport 9 } */
    oal_uint32          ul_dot11WNMDiagMfrInfoRprtIndex;
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMDiagMfrInfoRprtIfIndex;
    oal_uint8           uc_dot11WNMDiagMfrInfoRprtEventStatus;
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrOi[5];
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrIdString[255];
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrModelString[255];
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrSerialNumberString[255];
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrFirmwareVersion[255];
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtMfrAntennaType[255];
    oal_uint8           uc_dot11WNMDiagMfrInfoRprtCollocRadioType;
    oal_uint8           uc_dot11WNMDiagMfrInfoRprtDeviceType;
    oal_uint8           auc_dot11WNMDiagMfrInfoRprtCertificateID[251];

    /* dot11WNMDiagConfigProfReportTable ::= { dot11WNMReport 10 } */
    oal_uint32          ul_dot11WNMDiagConfigProfRprtIndex;
    oal_uint8           auc_dot11WNMDiagConfigProfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMDiagConfigProfRprtIfIndex;
    oal_uint8           uc_dot11WNMDiagConfigProfRprtEventStatus;
    oal_uint32          ul_dot11WNMDiagConfigProfRprtProfileId;
    oal_uint8           auc_dot11WNMDiagConfigProfRprtSupportedOperatingClasses[255];
    oal_uint8           uc_dot11WNMDiagConfigProfRprtTxPowerMode;
    oal_uint8           auc_dot11WNMDiagConfigProfRprtTxPowerLevels[255];
    oal_uint8           auc_dot11WNMDiagConfigProfRprtCipherSuite[4];
    oal_uint8           auc_dot11WNMDiagConfigProfRprtAkmSuite[4];
    oal_uint32          ul_dot11WNMDiagConfigProfRprtEapType;
    oal_uint8           auc_dot11WNMDiagConfigProfRprtEapVendorID[3];
    oal_uint8           auc_dot11WNMDiagConfigProfRprtEapVendorType[4];
    oal_uint8           uc_dot11WNMDiagConfigProfRprtCredentialType;
    oal_uint8           auc_dot11WNMDiagConfigProfRprtSSID[32];
    oal_uint8           uc_dot11WNMDiagConfigProfRprtPowerSaveMode;

    /* dot11WNMDiagAssocReportTable ::= { dot11WNMReport 11 } */
    oal_uint32          ul_dot11WNMDiagAssocRprtIndex;           /* dot11WNMDiagAssocRprtIndex Unsigned32,          */
    oal_uint8           auc_dot11WNMDiagAssocRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMDiagAssocRprtIfIndex;         /* dot11WNMDiagAssocRprtIfIndex InterfaceIndex,    */
    oal_uint8           uc_dot11WNMDiagAssocRprtEventStatus;     /* dot11WNMDiagAssocRprtEventStatus INTEGER,       */
    oal_uint8           auc_dot11WNMDiagAssocRprtBssid[6];       /* dot11WNMDiagAssocRprtBssid MacAddress,          */
    oal_uint32          ul_dot11WNMDiagAssocRprtOperatingClass;  /* dot11WNMDiagAssocRprtOperatingClass Unsigned32, */
    oal_uint32          ul_dot11WNMDiagAssocRprtChannelNumber;   /* dot11WNMDiagAssocRprtChannelNumber Unsigned32,  */
    oal_uint32          ul_dot11WNMDiagAssocRprtStatusCode;      /* dot11WNMDiagAssocRprtStatusCode Unsigned32      */

    /* dot11WNMDiag8021xAuthReportTable ::= { dot11WNMReport 12 } */
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtIndex;
    oal_uint8           auc_dot11WNMDiag8021xAuthRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtIfIndex;
    oal_uint8           uc_dot11WNMDiag8021xAuthRprtEventStatus;
    oal_uint8           auc_dot11WNMDiag8021xAuthRprtBssid[6];
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtOperatingClass;
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtChannelNumber;
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtEapType;
    oal_uint8           auc_dot11WNMDiag8021xAuthRprtEapVendorID[3];
    oal_uint8           auc_dot11WNMDiag8021xAuthRprtEapVendorType[4];
    oal_uint8           uc_dot11WNMDiag8021xAuthRprtCredentialType;
    oal_uint32          ul_dot11WNMDiag8021xAuthRprtStatusCode;

    /* dot11WNMLocConfigReportTable ::= { dot11WNMReport 13 } */
    oal_uint32          ul_dot11WNMLocConfigRprtIndex;
    oal_uint8           auc_dot11WNMLocConfigRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMLocConfigRprtIfIndex;
    oal_uint8           auc_dot11WNMLocConfigRprtLocIndParams[16];
    oal_uint8           auc_dot11WNMLocConfigRprtLocIndChanList[254];
    oal_uint32          ul_dot11WNMLocConfigRprtLocIndBcastRate;
    oal_uint8           auc_dot11WNMLocConfigRprtLocIndOptions[255];
    oal_uint8           uc_dot11WNMLocConfigRprtStatusConfigSubelemId;
    oal_uint8           uc_dot11WNMLocConfigRprtStatusResult;
    oal_uint8           auc_dot11WNMLocConfigRprtVendorSpecificRprtContent[255];

    /* dot11WNMBssTransitReportTable ::= { dot11WNMReport 14 } */
    oal_uint32          ul_dot11WNMBssTransitRprtIndex;
    oal_uint8           auc_dot11WNMBssTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMBssTransitRprtIfIndex;
    oal_uint8           uc_dot11WNMBssTransitRprtStatusCode;
    oal_uint32          ul_dot11WNMBssTransitRprtBSSTerminationDelay;
    oal_uint8           auc_dot11WNMBssTransitRprtTargetBssid[6];
    oal_uint8           auc_dot11WNMBssTransitRprtCandidateList[2304];

    /* 备注: 没有找到 dot11WNMReport 15 */
    /* dot11WNMColocInterfReportTable ::= { dot11WNMReport 16 } */
    oal_uint32          ul_dot11WNMColocInterfRprtIndex;
    oal_uint8           auc_dot11WNMColocInterfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    oal_uint32          ul_dot11WNMColocInterfRprtIfIndex;
    oal_uint32          ul_dot11WNMColocInterfRprtPeriod;
    oal_int32           l_dot11WNMColocInterfRprtInterfLevel;
    oal_uint32          ul_dot11WNMColocInterfRprtInterfAccuracy;
    oal_uint32          ul_dot11WNMColocInterfRprtInterfIndex;
    oal_int32           l_dot11WNMColocInterfRprtInterfInterval;
    oal_int32           l_dot11WNMColocInterfRprtInterfBurstLength;
    oal_int32           l_dot11WNMColocInterfRprtInterfStartTime;
    oal_int32           l_dot11WNMColocInterfRprtInterfCenterFreq;
    oal_uint32          ul_dot11WNMColocInterfRprtInterfBandwidth;
}wlan_mib_dot11WirelessNetworkManagement_stru;

/****************************************************************************/
/* dot11MeshSTAConfig TABLE  - members of Dot11MeshSTAConfigEntry           */
/****************************************************************************/
/* Mesh Station Configuration attributes. In tabular form to allow for mul- */
/* tiple instances on an agent.                                             */
typedef struct {
oal_uint8           auc_dot11MeshID[32];                      /* dot11MeshID OCTET STRING,   (SIZE(0..32))       */
oal_uint32          ul_dot11MeshNumberOfPeerings;             /* dot11MeshNumberOfPeerings Unsigned32,           */
oal_bool_enum_uint8 en_dot11MeshAcceptingAdditionalPeerings;  /* dot11MeshAcceptingAdditionalPeerings TruthValue, */
oal_bool_enum_uint8 en_dot11MeshConnectedToMeshGate;          /* dot11MeshConnectedToMeshGate TruthValue,        */
oal_bool_enum_uint8 en_dot11MeshSecurityActivated;            /* dot11MeshSecurityActivated TruthValue,          */
oal_uint8           uc_dot11MeshActiveAuthenticationProtocol; /* dot11MeshActiveAuthenticationProtocol INTEGER,  */
oal_uint32          ul_dot11MeshMaxRetries;                   /* dot11MeshMaxRetries Unsigned32,                 */
oal_uint32          ul_dot11MeshRetryTimeout;                 /* dot11MeshRetryTimeout Unsigned32,               */
oal_uint32          ul_dot11MeshConfirmTimeout; /* dot11MeshConfirmTimeout Unsigned32,             */
oal_uint32          ul_dot11MeshHoldingTimeout;               /* dot11MeshHoldingTimeout Unsigned32,             */
oal_uint32          ul_dot11MeshConfigGroupUpdateCount;       /* dot11MeshConfigGroupUpdateCount Unsigned32,     */
oal_uint8           uc_dot11MeshActivePathSelectionProtocol;  /* dot11MeshActivePathSelectionProtocol INTEGER,   */
oal_uint8           uc_dot11MeshActivePathSelectionMetric;    /* dot11MeshActivePathSelectionMetric INTEGER,     */
oal_bool_enum_uint8 en_dot11MeshForwarding;                   /* dot11MeshForwarding TruthValue,                 */
oal_uint32          ul_dot11MeshTTL;                          /* dot11MeshTTL Unsigned32,                        */
oal_bool_enum_uint8 en_dot11MeshGateAnnouncements;            /* dot11MeshGateAnnouncements TruthValue,          */
oal_uint32          ul_dot11MeshGateAnnouncementInterval;     /* dot11MeshGateAnnouncementInterval Unsigned32,   */
oal_uint8           uc_dot11MeshActiveCongestionControlMode;  /* dot11MeshActiveCongestionControlMode INTEGER,   */
oal_uint8           uc_dot11MeshActiveSynchronizationMethod;  /* dot11MeshActiveSynchronizationMethod INTEGER,   */
oal_uint32          ul_dot11MeshNbrOffsetMaxNeighbor;         /* dot11MeshNbrOffsetMaxNeighbor Unsigned32,       */
oal_bool_enum_uint8 en_dot11MBCAActivated;                    /* dot11MBCAActivated TruthValue,                  */
oal_uint32          ul_dot11MeshBeaconTimingReportInterval;   /* dot11MeshBeaconTimingReportInterval Unsigned32, */
oal_uint32          ul_dot11MeshBeaconTimingReportMaxNum;     /* dot11MeshBeaconTimingReportMaxNum Unsigned32,   */
oal_uint32          ul_dot11MeshDelayedBeaconTxInterval;      /* dot11MeshDelayedBeaconTxInterval Unsigned32,    */
oal_uint32          ul_dot11MeshDelayedBeaconTxMaxDelay;      /* dot11MeshDelayedBeaconTxMaxDelay Unsigned32,    */
oal_uint32          ul_dot11MeshDelayedBeaconTxMinDelay;      /* dot11MeshDelayedBeaconTxMinDelay Unsigned32,    */
oal_uint32          ul_dot11MeshAverageBeaconFrameDuration;   /* dot11MeshAverageBeaconFrameDuration Unsigned32, */
oal_uint32          ul_dot11MeshSTAMissingAckRetryLimit;      /* dot11MeshSTAMissingAckRetryLimit Unsigned32,    */
oal_uint32          ul_dot11MeshAwakeWindowDuration;          /* dot11MeshAwakeWindowDuration Unsigned32,        */
oal_bool_enum_uint8 en_dot11MCCAImplemented;                  /* dot11MCCAImplemented TruthValue,                */
oal_bool_enum_uint8 en_dot11MCCAActivated;                    /* dot11MCCAActivated TruthValue,                  */
oal_uint32          ul_dot11MAFlimit;                         /* dot11MAFlimit Unsigned32,                       */
oal_uint32          ul_dot11MCCAScanDuration;                 /* dot11MCCAScanDuration Unsigned32,               */
oal_uint32          ul_dot11MCCAAdvertPeriodMax;              /* dot11MCCAAdvertPeriodMax Unsigned32,            */
oal_uint32          ul_dot11MCCAMinTrackStates;               /* dot11MCCAMinTrackStates Unsigned32,             */
oal_uint32          ul_dot11MCCAMaxTrackStates;               /* dot11MCCAMaxTrackStates Unsigned32,             */
oal_uint32          ul_dot11MCCAOPtimeout;                    /* dot11MCCAOPtimeout Unsigned32,                  */
oal_uint32          ul_dot11MCCACWmin;                        /* dot11MCCACWmin Unsigned32,                      */
oal_uint32          ul_dot11MCCACWmax;                        /* dot11MCCACWmax Unsigned32,                      */
oal_uint32          ul_dot11MCCAAIFSN;                        /* dot11MCCAAIFSN Unsigned32                       */
}wlan_mib_Dot11MeshSTAConfigEntry_stru;

/*****************************************************************************/
/* dot11MeshHWMPConfig TABLE  - members of Dot11MeshHWMPConfigEntry          */
/*****************************************************************************/
/* MMesh Station HWMP Configuration attributes. In tabular form to allow for */
/* tmultiple instances on an agent.                                          */
typedef struct {
    oal_uint32 ul_dot11MeshHWMPmaxPREQretries;           /* dot11MeshHWMPmaxPREQretries Unsigned32,           */
    oal_uint32 ul_dot11MeshHWMPnetDiameter;              /* dot11MeshHWMPnetDiameter Unsigned32,              */
    oal_uint32 ul_dot11MeshHWMPnetDiameterTraversalTime; /* dot11MeshHWMPnetDiameterTraversalTime Unsigned32, */
    oal_uint32 ul_dot11MeshHWMPpreqMinInterval;          /* dot11MeshHWMPpreqMinInterval Unsigned32,          */
    oal_uint32 ul_dot11MeshHWMPperrMinInterval;          /* dot11MeshHWMPperrMinInterval Unsigned32,          */
    oal_uint32 ul_dot11MeshHWMPactivePathToRootTimeout;  /* dot11MeshHWMPactivePathToRootTimeout Unsigned32,  */
    oal_uint32 ul_dot11MeshHWMPactivePathTimeout;        /* dot11MeshHWMPactivePathTimeout Unsigned32,        */
    oal_uint8  uc_dot11MeshHWMProotMode;                 /* dot11MeshHWMProotMode INTEGER,                    */
    oal_uint32 ul_dot11MeshHWMProotInterval;             /* dot11MeshHWMProotInterval Unsigned32,             */
    oal_uint32 ul_dot11MeshHWMPrannInterval;             /* dot11MeshHWMPrannInterval Unsigned32,             */
    oal_uint8  uc_dot11MeshHWMPtargetOnly;               /* dot11MeshHWMPtargetOnly INTEGER,                  */
    oal_uint32 ul_dot11MeshHWMPmaintenanceInterval;      /* dot11MeshHWMPmaintenanceInterval Unsigned32,      */
    oal_uint32 ul_dot11MeshHWMPconfirmationInterval;     /* dot11MeshHWMPconfirmationInterval Unsigned32      */
}wlan_mib_Dot11MeshHWMPConfigEntry_stru;

/**************************************************************************************/
/* dot11RSNAConfigPasswordValue TABLE  - members of Dot11RSNAConfigPasswordValueEntry */
/**************************************************************************************/
/* When SAE authentication is the selected AKM suite,     */
/* this table is used to locate the binary representation */
/* of a shared, secret, and potentially low-entropy word, */
/* phrase, code, or key that will be used as the          */
/* authentication credential between a TA/RA pair.        */
/* This table is logically write-only. Reading this table */
/* returns unsuccessful status or null or zero." */
typedef struct {
    oal_uint32 ul_dot11RSNAConfigPasswordValueIndex; /* dot11RSNAConfigPasswordValueIndex Unsigned32, */
    oal_uint8 uc_dot11RSNAConfigPasswordCredential;  /* dot11RSNAConfigPasswordCredential OCTET STRING, */
    oal_uint8 auc_dot11RSNAConfigPasswordPeerMac[6]; /* dot11RSNAConfigPasswordPeerMac MacAddress */
} wlan_mib_Dot11RSNAConfigPasswordValueEntry_stru;

/****************************************************************************/
/* dot11RSNAConfigDLCGroup TABLE  - members of Dot11RSNAConfigDLCGroupEntry */
/****************************************************************************/
/* This table gives a prioritized list of domain parameter set */
/* Identifiers for discrete logarithm cryptography (DLC) groups. */
typedef struct {
    oal_uint32 ul_dot11RSNAConfigDLCGroupIndex;      /* dot11RSNAConfigDLCGroupIndex Unsigned32, */
    oal_uint32 ul_dot11RSNAConfigDLCGroupIdentifier; /* dot11RSNAConfigDLCGroupIdentifier Unsigned32 */
} wlan_mib_Dot11RSNAConfigDLCGroupEntry_stru;

/****************************************************************************/
/* dot11VHTStationConfig TABLE  - members of Dot11VHTStationConfigEntry */
/****************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple */
/* instances on an agent. */
typedef struct {
    oal_uint32 ul_dot11MaxMPDULength;                              /* dot11MaxMPDULength INTEGER, */
    oal_uint32 ul_dot11VHTMaxRxAMPDUFactor;                        /* dot11VHTMaxRxAMPDUFactor Unsigned32, */
    oal_bool_enum_uint8 en_dot11VHTControlFieldSupported;          /* dot11VHTControlFieldSupported TruthValue, */
    oal_bool_enum_uint8 en_dot11VHTTXOPPowerSaveOptionImplemented;
    oal_uint16 us_dot11VHTRxMCSMap;                                /* dot11VHTRxMCSMap OCTET STRING, */
    oal_uint32 ul_dot11VHTRxHighestDataRateSupported;              /* dot11VHTRxHighestDataRateSupported Unsigned32, */
    oal_uint16 us_dot11VHTTxMCSMap;                                /* dot11VHTTxMCSMap OCTET STRING, */
    oal_uint32 ul_dot11VHTTxHighestDataRateSupported;              /* dot11VHTTxHighestDataRateSupported Unsigned32, */
    // oal_uint32          ul_dot11VHTOBSSScanCount;                      /* dot11VHTOBSSScanCount Unsigned32 */
} wlan_mib_Dot11VHTStationConfigEntry_stru;

/****************************************************************************************/
/* Start of dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 } */
/* --  MAC GROUPS */
/* --  dot11OperationTable ::= { dot11mac 1 } */
/* --  dot11CountersTable ::= { dot11mac 2 } */
/* --  dot11GroupAddressesTable ::= { dot11mac 3 } */
/* --  dot11EDCATable ::= { dot11mac 4 } */
/* --  dot11QAPEDCATable ::= { dot11mac 5 } */
/* --  dot11QosCountersTable ::= { dot11mac 6 } */
/* --  dot11ResourceInfoTable    ::= { dot11mac 7 } */
/****************************************************************************************/
/****************************************************************************************/
/* dot11OperationTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11OperationEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group contains MAC attributes pertaining to the operation of the MAC.          */
/*      This has been implemented as a table in order to allow for multiple             */
/*      instantiations on an agent." */
/* ::= { dot11mac 1 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11RTSThreshold; /* dot11RTSThreshold Unsigned32 */
    // oal_uint32  ul_dot11ShortRetryLimit;                            /* dot11ShortRetryLimit Unsigned32 */
    // oal_uint32  ul_dot11LongRetryLimit;                             /* dot11LongRetryLimit Unsigned32 */
    oal_uint32 ul_dot11FragmentationThreshold; /* dot11FragmentationThreshold Unsigned32 */
    // oal_uint32  ul_dot11MaxTransmitMSDULifetime;                    /* dot11MaxTransmitMSDULifetime Unsigned32 */
    // oal_uint32  ul_dot11MaxReceiveLifetime;                         /* dot11MaxReceiveLifetime Unsigned32 */
    // oal_uint32  ul_dot11CAPLimit;                                   /* dot11CAPLimit Unsigned32 */
    // oal_uint32  ul_dot11HCCWmin;                                    /* dot11HCCWmin Unsigned32 */
    // oal_uint32  ul_dot11HCCWmax;                                    /* dot11HCCWmax Unsigned32 */
    // oal_uint32  ul_dot11HCCAIFSN;                                   /* dot11HCCAIFSN Unsigned32 */
    // oal_uint32  ul_dot11ADDBAResponseTimeout;                       /* dot11ADDBAResponseTimeout Unsigned32 */
    // oal_uint32  ul_dot11ADDTSResponseTimeout;                       /* dot11ADDTSResponseTimeout Unsigned32 */
    // oal_uint32  ul_dot11ChannelUtilizationBeaconInterval;
    // oal_uint32  ul_dot11ScheduleTimeout;                            /* dot11ScheduleTimeout Unsigned32 */
    // oal_uint32  ul_dot11DLSResponseTimeout;                         /* dot11DLSResponseTimeout Unsigned32 */
    // oal_uint32  ul_dot11QAPMissingAckRetryLimit;                    /* dot11QAPMissingAckRetryLimit Unsigned32 */
    // oal_uint32  ul_dot11EDCAAveragingPeriod;                        /* dot11EDCAAveragingPeriod Unsigned32 */
    wlan_mib_ht_protection_enum_uint8 en_dot11HTProtection; /* dot11HTProtection INTEGER */
    oal_bool_enum_uint8 en_dot11RIFSMode;                   /* dot11RIFSMode TruthValue */
    // oal_bool_enum_uint8 en_dot11PSMPControlledAccess;               /* dot11PSMPControlledAccess TruthValue */
    // oal_uint32  ul_dot11ServiceIntervalGranularity;                 /* dot11ServiceIntervalGranularity Unsigned32 */
    oal_bool_enum_uint8 en_dot11DualCTSProtection;               /* dot11DualCTSProtection TruthValue */
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated; /* dot11LSIGTXOPFullProtectionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent;            /* dot11NonGFEntitiesPresent TruthValue */
    oal_bool_enum_uint8 en_dot11PCOActivated;                    /* dot11PCOActivated TruthValue */
    // oal_uint32  ul_dot11PCOFortyMaxDuration;                        /* dot11PCOFortyMaxDuration Unsigned32 */
    // oal_uint32  ul_dot11PCOTwentyMaxDuration;                       /* dot11PCOTwentyMaxDuration Unsigned32 */
    // oal_uint32  ul_dot11PCOFortyMinDuration;                        /* dot11PCOFortyMinDuration Unsigned32 */
    // oal_uint32  ul_dot11PCOTwentyMinDuration;                       /* dot11PCOTwentyMinDuration Unsigned32 */
    oal_bool_enum_uint8 en_dot11FortyMHzIntolerant;                  /* dot11FortyMHzIntolerant TruthValue */
    oal_bool_enum_uint8 en_dot112040BSSCoexistenceManagementSupport;
    oal_uint32 ul_dot11BSSWidthTriggerScanInterval;                  /* dot11BSSWidthTriggerScanInterval Unsigned32 */
    oal_uint32 ul_dot11BSSWidthChannelTransitionDelayFactor;
    oal_uint32 ul_dot11OBSSScanPassiveDwell;                         /* dot11OBSSScanPassiveDwell Unsigned32 */
    oal_uint32 ul_dot11OBSSScanActiveDwell;                          /* dot11OBSSScanActiveDwell Unsigned32 */
    oal_uint32 ul_dot11OBSSScanPassiveTotalPerChannel;             /* dot11OBSSScanPassiveTotalPerChannel Unsigned32 */
    oal_uint32 ul_dot11OBSSScanActiveTotalPerChannel;              /* dot11OBSSScanActiveTotalPerChannel Unsigned32 */
    oal_uint32 ul_dot11OBSSScanActivityThreshold;                    /* dot11OBSSScanActivityThreshold Unsigned32 */
} wlan_mib_Dot11OperationEntry_stru;

/****************************************************************************************/
/* dot11CountersTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11CountersEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group containing attributes that are MAC counters. Implemented as a table      */
/*      to allow for multiple instantiations on an agent." */
/* ::= { dot11mac 2 } */
/****************************************************************************************/
typedef struct {
    // oal_uint32  ul_dot11TransmittedFragmentCount;               /* dot11TransmittedFragmentCount Counter32 */
    // oal_uint32  ul_dot11GroupTransmittedFrameCount;             /* dot11GroupTransmittedFrameCount Counter32 */
    // oal_uint32  ul_dot11FailedCount;                            /* dot11FailedCount Counter32 */
    // oal_uint32  ul_dot11RetryCount;                             /* dot11RetryCount Counter32 */
    // oal_uint32  ul_dot11MultipleRetryCount;                     /* dot11MultipleRetryCount Counter32 */
    // oal_uint32  ul_dot11FrameDuplicateCount;                    /* dot11FrameDuplicateCount Counter32 */
    // oal_uint32  ul_dot11RTSSuccessCount;                        /* dot11RTSSuccessCount Counter32 */
    // oal_uint32  ul_dot11RTSFailureCount;                        /* dot11RTSFailureCount Counter32 */
    // oal_uint32  ul_dot11ACKFailureCount;                        /* dot11ACKFailureCount Counter32 */
    // oal_uint32  ul_dot11ReceivedFragmentCount;                  /* dot11ReceivedFragmentCount Counter32 */
    oal_uint32 ul_dot11GroupReceivedFrameCount; /* dot11GroupReceivedFrameCount Counter32 */
    // oal_uint32  ul_dot11FCSErrorCount;                          /* dot11FCSErrorCount Counter32 */
    // oal_uint32  ul_dot11TransmittedFrameCount;                  /* dot11TransmittedFrameCount Counter32 */
    // oal_uint32  ul_dot11WEPUndecryptableCount;                  /* dot11WEPUndecryptableCount Counter32 */
    // oal_uint32  ul_dot11QosDiscardedFragmentCount;              /* dot11QosDiscardedFragmentCount Counter32 */
    // oal_uint32  ul_dot11AssociatedStationCount;                 /* dot11AssociatedStationCount Counter32 */
    // oal_uint32  ul_dot11QosCFPollsReceivedCount;                /* dot11QosCFPollsReceivedCount Counter32 */
    // oal_uint32  ul_dot11QosCFPollsUnusedCount;                  /* dot11QosCFPollsUnusedCount Counter32 */
    // oal_uint32  ul_dot11QosCFPollsUnusableCount;                /* dot11QosCFPollsUnusableCount Counter32 */
    // oal_uint32  ul_dot11QosCFPollsLostCount;                    /* dot11QosCFPollsLostCount Counter32 */
    // oal_uint32  ul_dot11TransmittedAMSDUCount;                  /* dot11TransmittedAMSDUCount Counter32 */
    // oal_uint32  ul_dot11FailedAMSDUCount;                       /* dot11FailedAMSDUCount Counter32 */
    // oal_uint32  ul_dot11RetryAMSDUCount;                        /* dot11RetryAMSDUCount Counter32 */
    // oal_uint32  ul_dot11MultipleRetryAMSDUCount;                /* dot11MultipleRetryAMSDUCount Counter32 */
    // oal_uint64  ull_dot11dot11TransmittedOctetsInAMSDUCount;    /* dot11TransmittedOctetsInAMSDUCount Counter64 */
    // oal_uint32  ul_dot11AMSDUAckFailureCount;                   /* dot11AMSDUAckFailureCount Counter32 */
    oal_uint32 ul_dot11ReceivedAMSDUCount;               /* dot11ReceivedAMSDUCount Counter32 */
    oal_uint64 ull_dot11dot11ReceivedOctetsInAMSDUCount; /* dot11ReceivedOctetsInAMSDUCount Counter64 */
    // oal_uint32  ul_dot11TransmittedAMPDUCount;                  /* dot11TransmittedAMPDUCount Counter32 */
    // oal_uint32  ul_dot11TransmittedMPDUsInAMPDUCount;           /* dot11TransmittedMPDUsInAMPDUCount Counter32 */
    // oal_uint64  ull_dot11dot11TransmittedOctetsInAMPDUCount;    /* dot11TransmittedOctetsInAMPDUCount Counter64 */
    // oal_uint32  ul_dot11AMPDUReceivedCount;                     /* dot11AMPDUReceivedCount Counter32 */
    oal_uint32 ul_dot11MPDUInReceivedAMPDUCount; /* dot11MPDUInReceivedAMPDUCount Counter32 */
    // oal_uint64  ull_dot11dot11ReceivedOctetsInAMPDUCount;       /* dot11ReceivedOctetsInAMPDUCount Counter64 */
    // oal_uint32  ul_dot11AMPDUDelimiterCRCErrorCount;            /* dot11AMPDUDelimiterCRCErrorCount Counter32 */
    // oal_uint32  ul_dot11ImplicitBARFailureCount;                /* dot11ImplicitBARFailureCount Counter32 */
    // oal_uint32  ul_dot11ExplicitBARFailureCount;                /* dot11ExplicitBARFailureCount Counter32 */
    // oal_uint32  ul_dot11ChannelWidthSwitchCount;                /* dot11ChannelWidthSwitchCount Counter32 */
    // oal_uint32  ul_dot11TwentyMHzFrameTransmittedCount;         /* dot11TwentyMHzFrameTransmittedCount Counter32 */
    // oal_uint32  ul_dot11FortyMHzFrameTransmittedCount;          /* dot11FortyMHzFrameTransmittedCount Counter32 */
    // oal_uint32  ul_dot11TwentyMHzFrameReceivedCount;            /* dot11TwentyMHzFrameReceivedCount Counter32 */
    // oal_uint32  ul_dot11FortyMHzFrameReceivedCount;             /* dot11FortyMHzFrameReceivedCount Counter32 */
    // oal_uint32  ul_dot11PSMPUTTGrantDuration;                   /* dot11PSMPUTTGrantDuration Counter32 */
    // oal_uint32  ul_dot11PSMPUTTUsedDuration;                    /* dot11PSMPUTTUsedDuration Counter32 */
    // oal_uint32  ul_dot11GrantedRDGUsedCount;                    /* dot11GrantedRDGUsedCount Counter32 */
    // oal_uint32  ul_dot11GrantedRDGUnusedCount;                  /* dot11GrantedRDGUnusedCount Counter32 */
    // oal_uint32  ul_dot11TransmittedFramesInGrantedRDGCount;   /* dot11TransmittedFramesInGrantedRDGCount Counter32 */
    // oal_uint64  ull_dot11Transmitted1OctetsInGrantedRDGCount; /* dot11TransmittedOctetsInGrantedRDGCount Counter64 */
    // oal_uint32  ul_dot11BeamformingFrameCount;                  /* dot11BeamformingFrameCount Counter32 */
    // oal_uint32  ul_dot11DualCTSSuccessCount;                    /* dot11DualCTSSuccessCount Counter32 */
    // oal_uint32  ul_dot11DualCTSFailureCount;                    /* dot11DualCTSFailureCount Counter32 */
    // oal_uint32  ul_dot11STBCCTSSuccessCount;                    /* dot11STBCCTSSuccessCount Counter32 */
    // oal_uint32  ul_dot11STBCCTSFailureCount;                    /* dot11STBCCTSFailureCount Counter32 */
    // oal_uint32  ul_dot11nonSTBCCTSSuccessCount;                 /* dot11nonSTBCCTSSuccessCount Counter32 */
    // oal_uint32  ul_dot11nonSTBCCTSFailureCount;                 /* dot11nonSTBCCTSFailureCount Counter32 */
    // oal_uint32  ul_dot11RTSLSIGSuccessCount;                    /* dot11RTSLSIGSuccessCount Counter32 */
    // oal_uint32  ul_dot11RTSLSIGFailureCount;                    /* dot11RTSLSIGFailureCount Counter32 */
    // oal_uint32  ul_dot11PBACErrors;                             /* dot11PBACErrors Counter32 */
    // oal_uint32  ul_dot11DeniedAssociationCounterDueToBSSLoad;
} wlan_mib_Dot11CountersEntry_stru;

/****************************************************************************************/
/* dot11GroupAddressesTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11GroupAddressesEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "A conceptual table containing a set of MAC addresses identifying the mul-      */
/*      ticast-group addresses for which this STA receives frames. The default          */
/*      value of this attribute is null." */
/* ::= { dot11mac 3 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11GroupAddressesIndex;                      /* dot11GroupAddressesIndex InterfaceIndex */
    oal_uint8 auc_dot11Address[6];                               /* dot11Address MacAddress */
    wlan_mib_row_status_enum_uint8 en_dot11GroupAddressesStatus; /* dot11GroupAddressesStatus RowStatus */
} wlan_mib_Dot11GroupAddressesEntry_stru;

/****************************************************************************************/
/* dot11EDCATable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11EDCAEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Conceptual table for EDCA default parameter values at a non-AP STA. This       */
/*      table contains the four entries of the EDCA parameters corresponding to         */
/*      four possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3      */
/*      to AC_VI, and index 4 to AC_VO." */
/* REFERENCE */
/* "IEEE 802.11-<year>, 9.2.4.2" */
/* ::= { dot11mac 4 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11EDCATableIndex;              /* dot11EDCATableIndex     Unsigned32 */
    oal_uint32 ul_dot11EDCATableCWmin;              /* dot11EDCATableCWmin Unsigned32 */
    oal_uint32 ul_dot11EDCATableCWmax;              /* dot11EDCATableCWmax Unsigned32 */
    oal_uint32 ul_dot11EDCATableAIFSN;              /* dot11EDCATableAIFSN Unsigned32 */
    oal_uint32 ul_dot11EDCATableTXOPLimit;          /* dot11EDCATableTXOPLimit Unsigned32 */
    oal_uint32 ul_dot11EDCATableMSDULifetime;       /* dot11EDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11EDCATableMandatory; /* dot11EDCATableMandatory TruthValue */
} wlan_mib_Dot11EDCAEntry_stru;

/****************************************************************************************/
/* dot11QAPEDCATable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11QAPEDCAEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Conceptual table for EDCA default parameter values at the AP. This table       */
/*      contains the four entries of the EDCA parameters corresponding to four          */
/*      possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3 to        */
/*      AC_VI, and index 4 to AC_VO." */
/* REFERENCE */
/* "IEEE 802.11-<year>, 9.19.2" */
/* ::= { dot11mac 5 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11QAPEDCATableIndex;              /* dot11QAPEDCATableIndex    Unsigned32 */
    oal_uint32 ul_dot11QAPEDCATableCWmin;              /* dot11QAPEDCATableCWmin Unsigned32 */
    oal_uint32 ul_dot11QAPEDCATableCWmax;              /* dot11QAPEDCATableCWmax Unsigned32 */
    oal_uint32 ul_dot11QAPEDCATableAIFSN;              /* dot11QAPEDCATableAIFSN Unsigned32 */
    oal_uint32 ul_dot11QAPEDCATableTXOPLimit;          /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    oal_uint32 ul_dot11QAPEDCATableMSDULifetime;       /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11QAPEDCATableMandatory; /* dot11QAPEDCATableMandatory TruthValue */
} wlan_mib_Dot11QAPEDCAEntry_stru;

/****************************************************************************************/
/* dot11QosCountersTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11QosCountersEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group containing attributes that are MAC counters implemented as a table       */
/*      to allow for multiple instantiations on an agent." */
/* ::= { dot11mac 6 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11QosCountersIndex;            /* dot11QosCountersIndex Unsigned32 */
    oal_uint32 ul_dot11QosTransmittedFragmentCount; /* dot11QosTransmittedFragmentCount Counter32 */
    oal_uint32 ul_dot11QosFailedCount;              /* dot11QosFailedCount Counter32 */
    oal_uint32 ul_dot11QosRetryCount;               /* dot11QosRetryCount Counter32 */
    oal_uint32 ul_dot11QosMultipleRetryCount;       /* dot11QosMultipleRetryCount Counter32 */
    oal_uint32 ul_dot11QosFrameDuplicateCount;      /* dot11QosFrameDuplicateCount Counter32 */
    oal_uint32 ul_dot11QosRTSSuccessCount;          /* dot11QosRTSSuccessCount Counter32 */
    oal_uint32 ul_dot11QosRTSFailureCount;          /* dot11QosRTSFailureCount Counter32 */
    oal_uint32 ul_dot11QosACKFailureCount;          /* dot11QosACKFailureCount Counter32 */
    oal_uint32 ul_dot11QosReceivedFragmentCount;    /* dot11QosReceivedFragmentCount Counter32 */
    oal_uint32 ul_dot11QosTransmittedFrameCount;    /* dot11QosTransmittedFrameCount Counter32 */
    oal_uint32 ul_dot11QosDiscardedFrameCount;      /* dot11QosDiscardedFrameCount Counter32 */
    oal_uint32 ul_dot11QosMPDUsReceivedCount;       /* dot11QosMPDUsReceivedCount Counter32 */
    oal_uint32 ul_dot11QosRetriesReceivedCount;     /* dot11QosRetriesReceivedCount Counter32 */
} wlan_mib_Dot11QosCountersEntry_stru;

/****************************************************************************************/
/* dot11ResourceInfoTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11ResourceInfoEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Provides a means of indicating, in data readable from a managed object,        */
/*      information that identifies the source of the implementation." */
/* REFERENCE "IEEE Std 802.1F-1993, A.7. Note that this standard has been with-    */
/*      drawn." */
/* ::= { dot11mac 7 } */
/* ::= { dot11resAttribute 2 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11manufacturerOUI[3];              /* dot11manufacturerOUI OCTET STRING */
    oal_uint8 auc_dot11manufacturerName[128];           /* dot11manufacturerName DisplayString SIZE(0..128) */
    oal_uint8 auc_dot11manufacturerProductName[128];    /* dot11manufacturerProductName DisplayString SIZE(0..128) */
    oal_uint8 auc_dot11manufacturerProductVersion[128]; /* dot11manufacturerProductVersion DisplayString SIZE(0..128) */
} wlan_mib_Dot11ResourceInfoEntry_stru;

/****************************************************************************************/
/* Start of dot11res    OBJECT IDENTIFIER ::= { ieee802dot11 3 } */
/* dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11ResourceTypeIDName[4]; /* dot11ResourceTypeIDName  DisplayString (SIZE(4)) */
    wlan_mib_Dot11ResourceInfoEntry_stru st_resource_info;
} wlan_mib_dot11resAttribute_stru;

/****************************************************************************************/
/* Start of dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 } */
/* --  PHY GROUPS */
/* --  dot11PhyOperationTable ::= { dot11phy 1 } */
/* --  dot11PhyAntennaTable ::= { dot11phy 2 } */
/* --  dot11PhyTxPowerTable ::= { dot11phy 3 } */
/* --  dot11PhyFHSSTable ::= { dot11phy 4 } */
/* --  dot11PhyDSSSTable ::= { dot11phy 5 } */
/* --  dot11PhyIRTable ::= { dot11phy 6 } */
/* --  dot11RegDomainsSupportedTable ::= { dot11phy 7 } */
/* --  dot11AntennasListTable ::= { dot11phy 8 } */
/* --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 } */
/* --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 } */
/* --  dot11PhyOFDMTable ::= { dot11phy 11 } */
/* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
/* --  dot11HoppingPatternTable ::= { dot11phy 13 } */
/* --  dot11PhyERPTable ::= { dot11phy 14 } */
/* --  dot11PhyHTTable  ::= { dot11phy 15 } */
/* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
/* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
/* --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 } */
/* -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac) */
/* -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
/****************************************************************************************/
/****************************************************************************************/
/* dot11PhyOperationTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyOperationEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "PHY level attributes concerned with operation. Implemented as a table          */
/*      indexed on ifIndex to allow for multiple instantiations on an Agent." */
/* ::= { dot11phy 1 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 uc_dot11PHYType;           /* dot11PHYType INTEGER */
    oal_uint32 ul_dot11CurrentRegDomain; /* dot11CurrentRegDomain Unsigned32 */
    oal_uint8 uc_dot11TempType;          /* dot11TempType INTEGER */
} wlan_mib_Dot11PhyOperationEntry_stru;

/****************************************************************************************/
/* dot11PhyAntennaTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyAntennaEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group of attributes for PhyAntenna. Implemented as a table indexed on          */
/*      ifIndex to allow for multiple instances on an agent." */
/* ::= { dot11phy 2} */
/****************************************************************************************/
typedef struct {
    // oal_uint32          ul_dot11CurrentTxAntenna;
    // oal_uint8           uc_dot11DiversitySupportImplemented;
    // oal_uint32          ul_dot11CurrentRxAntenna;
    oal_bool_enum_uint8 en_dot11AntennaSelectionOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitIndicesFeedbackASOptionImplemented;
    oal_bool_enum_uint8 en_dot11ExplicitCSIFeedbackASOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitIndicesComputationASOptionImplemented;
    oal_bool_enum_uint8 en_dot11ReceiveAntennaSelectionOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitSoundingPPDUOptionImplemented;
    // oal_uint32          ul_dot11NumberOfActiveRxAntennas;
} wlan_mib_Dot11PhyAntennaEntry_stru;

/****************************************************************************************/
/* dot11PhyTxPowerTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyTxPowerEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group of attributes for dot11PhyTxPowerTable. Implemented as a table           */
/*      indexed on STA ID to allow for multiple instances on an Agent." */
/* ::= { dot11phy 3} */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11NumberSupportedPowerLevelsImplemented;
    oal_uint32 ul_dot11TxPowerLevel1;                         /* dot11TxPowerLevel1 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel2;                         /* dot11TxPowerLevel2 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel3;                         /* dot11TxPowerLevel3 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel4;                         /* dot11TxPowerLevel4 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel5;                         /* dot11TxPowerLevel5 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel6;                         /* dot11TxPowerLevel6 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel7;                         /* dot11TxPowerLevel7 Unsigned32 */
    oal_uint32 ul_dot11TxPowerLevel8;                         /* dot11TxPowerLevel8 Unsigned32 */
    oal_uint32 ul_dot11CurrentTxPowerLevel;                   /* dot11CurrentTxPowerLevel Unsigned32 */
    oal_uint8 auc_dot11TxPowerLevelExtended;                  /* dot11TxPowerLevelExtended OCTET STRING */
    oal_uint32 ul_dot11CurrentTxPowerLevelExtended;           /* dot11CurrentTxPowerLevelExtended Unsigned32 */
} wlan_mib_Dot11PhyTxPowerEntry_stru;

/****************************************************************************************/
/* dot11PhyFHSSTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyFHSSEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group of attributes for dot11PhyFHSSTable. Implemented as a table indexed      */
/*      on STA ID to allow for multiple instances on an Agent." */
/* ::= { dot11phy 4 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11HopTime;                            /* dot11HopTime Unsigned32 */
    oal_uint32 ul_dot11CurrentChannelNumber;               /* dot11CurrentChannelNumber Unsigned32 */
    oal_uint32 ul_dot11MaxDwellTime;                       /* dot11MaxDwellTime Unsigned32 */
    oal_uint32 ul_dot11CurrentDwellTime;                   /* dot11CurrentDwellTime Unsigned32 */
    oal_uint32 ul_dot11CurrentSet;                         /* dot11CurrentSet Unsigned32 */
    oal_uint32 ul_dot11CurrentPattern;                     /* dot11CurrentPattern Unsigned32 */
    oal_uint32 ul_dot11CurrentIndex;                       /* dot11CurrentIndex Unsigned32 */
    oal_uint32 ul_dot11EHCCPrimeRadix;                     /* dot11EHCCPrimeRadix Unsigned32 */
    oal_uint32 ul_dot11EHCCNumberofChannelsFamilyIndex;    /* dot11EHCCNumberofChannelsFamilyIndex Unsigned32 */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityImplemented; /* dot11EHCCCapabilityImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityActivated;   /* dot11EHCCCapabilityActivated TruthValue */
    oal_uint8 uc_dot11HopAlgorithmAdopted;                 /* dot11HopAlgorithmAdopted INTEGER */
    oal_bool_enum_uint8 en_dot11RandomTableFlag;           /* dot11RandomTableFlag TruthValue */
    oal_uint32 ul_dot11NumberofHoppingSets;                /* dot11NumberofHoppingSets Unsigned32 */
    oal_uint32 ul_dot11HopModulus;                         /* dot11HopModulus Unsigned32 */
    oal_uint32 ul_dot11HopOffset;                          /* dot11HopOffset Unsigned32 */
} wlan_mib_Dot11PhyFHSSEntry_stru;

/****************************************************************************************/
/* dot11PhyDSSSTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyDSSSEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11PhyDSSSEntry. Implemented as a table indexed      */
/*      on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 5 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11CurrentChannel; /* dot11CurrentChannel Unsigned32 */
    // oal_uint32  ul_dot11CCAModeSupported;       /* dot11CCAModeSupported Unsigned32 */
    // oal_uint8   uc_dot11CurrentCCAMode;         /* dot11CurrentCCAMode INTEGER */
    // oal_int     l_dot11EDThreshold;             /* dot11EDThreshold Integer32 */
} wlan_mib_Dot11PhyDSSSEntry_stru;

/****************************************************************************************/
/* dot11PhyIRTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyIREntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group of attributes for dot11PhyIRTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 6 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11CCAWatchdogTimerMax; /* dot11CCAWatchdogTimerMax Unsigned32 */
    oal_uint32 ul_dot11CCAWatchdogCountMax; /* dot11CCAWatchdogCountMax Unsigned32 */
    oal_uint32 ul_dot11CCAWatchdogTimerMin; /* dot11CCAWatchdogTimerMin Unsigned32 */
    oal_uint32 ul_dot11CCAWatchdogCountMin; /* dot11CCAWatchdogCountMin Unsigned32 */
} wlan_mib_Dot11PhyIREntry_stru;

/****************************************************************************************/
/* dot11RegDomainsSupportedTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11RegDomainsSupportedEntry */
/* MAX-ACCESS not-accessible */
/* STATUS deprecated */
/* DESCRIPTION */
/* "Superceded by dot11OperatingClassesTable.                                      */
/*      There are different operational requirements dependent on the regulatory        */
/*      domain. This attribute list describes the regulatory domains the PLCP and       */
/*      PMD support in this implementation. Currently defined values and their          */
/*      corresponding Regulatory Domains are:                                           */
/*      FCC (USA) = X'10', DOC (Canada) = X'20', ETSI (most of Europe) = X'30',         */
/*      Spain = X'31', France = X'32', Japan = X'40', China = X'50', Other = X'00'" */
/* ::= { dot11phy 7} */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11RegDomainsSupportedIndex;  /* dot11RegDomainsSupportedIndex Unsigned32 */
    oal_uint8 uc_dot11RegDomainsImplementedValue; /* dot11RegDomainsImplementedValue INTEGER */
} wlan_mib_Dot11RegDomainsSupportedEntry_stru;

/****************************************************************************************/
/* dot11AntennasListTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11AntennasListEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This table represents the list of antennae. An antenna can be marked to        */
/*      be capable of transmitting, receiving, and/or for participation in receive      */
/*      diversity. Each entry in this table represents a single antenna with its        */
/*      properties. The maximum number of antennae that can be contained in this        */
/*      table is 255." */
/* ::= { dot11phy 8 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11AntennaListIndex;                /* dot11AntennaListIndex Unsigned32 */
    oal_uint32 ul_dot11TxAntennaImplemented;            /* dot11TxAntennaImplemented TruthValue */
    oal_uint32 ul_dot11RxAntennaImplemented;            /* dot11RxAntennaImplemented TruthValue */
    oal_uint32 ul_dot11DiversitySelectionRxImplemented; /* dot11DiversitySelectionRxImplemented TruthValue */
} wlan_mib_Dot11AntennasListEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesTxTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesTxEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "The Transmit bit rates supported by the PLCP and PMD, represented by a         */
/*      count from X'02-X'7f, corresponding to data rates in increments of              */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s subject to limitations of each individ-      */
/*      ual PHY." */
/* ::= { dot11phy 9 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11SupportedDataRatesTxIndex;   /* dot11SupportedDataRatesTxIndex Unsigned32 */
    oal_uint32 ul_dot11ImplementedDataRatesTxValue; /* dot11ImplementedDataRatesTxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesRxTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesRxEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "The receive bit rates supported by the PLCP and PMD, represented by a          */
/*      count from X'002-X'7f, corresponding to data rates in increments of             */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s." */
/* ::= { dot11phy 10 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11SupportedDataRatesRxIndex;   /* dot11SupportedDataRatesRxIndex Unsigned32 */
    oal_uint32 ul_dot11ImplementedDataRatesRxValue; /* dot11ImplementedDataRatesRxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesRxEntry_stru;

/****************************************************************************************/
/* dot11PhyOFDMTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyOFDMEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Group of attributes for dot11PhyOFDMTable. Implemented as a table indexed      */
/*      on ifindex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 11 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11CurrentFrequency;                       /* dot11CurrentFrequency Unsigned32 */
    oal_int32 l_dot11TIThreshold;                              /* dot11TIThreshold Integer32 */
    oal_uint32 ul_dot11FrequencyBandsImplemented;              /* dot11FrequencyBandsImplemented Unsigned32 */
    oal_uint32 ul_dot11ChannelStartingFactor;                  /* dot11ChannelStartingFactor Unsigned32 */
    oal_bool_enum_uint8 en_dot11FiveMHzOperationImplemented;   /* dot11FiveMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TenMHzOperationImplemented;    /* dot11TenMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TwentyMHzOperationImplemented; /* dot11TwentyMHzOperationImplemented TruthValue */
    oal_uint8 uc_dot11PhyOFDMChannelWidth;                     /* dot11PhyOFDMChannelWidth INTEGER */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDImplemented;          /* dot11OFDMCCAEDImplemented  TruthValue */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDRequired;             /* dot11OFDMCCAEDRequired  TruthValue */
    oal_uint32 ul_dot11OFDMEDThreshold;                        /* dot11OFDMEDThreshold  Unsigned32 */
    oal_uint8 uc_dot11STATransmitPowerClass;                   /* dot11STATransmitPowerClass INTEGER */
    oal_uint8 uc_dot11ACRType;                                 /* dot11ACRType INTEGER */
} wlan_mib_Dot11PhyOFDMEntry_stru;

/****************************************************************************************/
/* dot11PhyHRDSSSTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyHRDSSSEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11PhyHRDSSSEntry. Implemented as a table            */
/*      indexed on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 12 } */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ShortPreambleOptionImplemented; /* dot11ShortPreambleOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11PBCCOptionImplemented;          /* dot11PBCCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ChannelAgilityPresent;          /* dot11ChannelAgilityPresent TruthValue */
    // oal_bool_enum_uint8 en_dot11ChannelAgilityActivated;        /* dot11ChannelAgilityActivated TruthValue */
    // oal_uint32  ul_dot11HRCCAModeImplemented;                   /* dot11HRCCAModeImplemented Unsigned32 */
} wlan_mib_Dot11PhyHRDSSSEntry_stru;

/****************************************************************************************/
/* dot11HoppingPatternTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11HoppingPatternEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "The (conceptual) table of attributes necessary for a frequency hopping         */
/*      implementation to be able to create the hopping sequences necessary to          */
/*      operate in the subband for the associated domain country string." */
/* ::= { dot11phy 13 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11HoppingPatternIndex;    /* dot11HoppingPatternIndex Unsigned32 */
    oal_uint32 ul_dot11RandomTableFieldNumber; /* dot11RandomTableFieldNumber Unsigned32 */
} wlan_mib_Dot11HoppingPatternEntry_stru;

/****************************************************************************************/
/* dot11PhyERPTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyERPEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11PhyERPEntry. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 14 } */
/****************************************************************************************/
typedef struct {
    // oal_bool_enum_uint8 en_dot11ERPPBCCOptionImplemented;       /* dot11ERPPBCCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11ERPBCCOptionActivated;          /* dot11ERPBCCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11DSSSOFDMOptionImplemented;      /* dot11DSSSOFDMOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11DSSSOFDMOptionActivated;        /* dot11DSSSOFDMOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionImplemented; /* dot11ShortSlotTimeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionActivated;   /* dot11ShortSlotTimeOptionActivated TruthValue */
} wlan_mib_Dot11PhyERPEntry_stru;

/****************************************************************************************/
/* dot11PhyHTTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyHTEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11PhyHTTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 15 } */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot112GFortyMHzOperationImplemented; /* dot11FortyMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot115GFortyMHzOperationImplemented; /* dot11FortyMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11FortyMHzOperationActivated;     /* dot11FortyMHzOperationActivated TruthValue */
    oal_uint32 ul_dot11CurrentPrimaryChannel;                   /* dot11CurrentPrimaryChannel Unsigned32 */
    oal_uint32 ul_dot11CurrentSecondaryChannel;                 /* dot11CurrentSecondaryChannel Unsigned32 */
    oal_uint32 ul_dot11NumberOfSpatialStreamsImplemented;       /* dot11NumberOfSpatialStreamsImplemented Unsigned32 */
    // oal_uint32          ul_dot11NumberOfSpatialStreamsActivated;
    oal_bool_enum_uint8 en_dot11HTGreenfieldOptionImplemented; /* dot11HTGreenfieldOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11HTGreenfieldOptionActivated;   /* dot11HTGreenfieldOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyImplemented; /* dot11ShortGIOptionInTwentyImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyActivated;
    oal_bool_enum_uint8 en_dot112GShortGIOptionInFortyImplemented; /* dot11ShortGIOptionInFortyImplemented TruthValue */
    oal_bool_enum_uint8 en_dot115GShortGIOptionInFortyImplemented; /* dot11ShortGIOptionInFortyImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInFortyActivated;
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionImplemented; /* dot11LDPCCodingOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionActivated;   /* dot11LDPCCodingOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11TxSTBCOptionImplemented;     /* dot11TxSTBCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TxSTBCOptionActivated;       /* dot11TxSTBCOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RxSTBCOptionImplemented;     /* dot11RxSTBCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11RxSTBCOptionActivated;                  /* dot11RxSTBCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionImplemented;
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionActivated;
    oal_uint32 ul_dot11HighestSupportedDataRate;               /* dot11HighestSupportedDataRate Unsigned32 */
    oal_bool_enum_uint8 en_dot11TxMCSSetDefined;               /* dot11TxMCSSetDefined TruthValue */
    oal_bool_enum_uint8 en_dot11TxRxMCSSetNotEqual;
    oal_uint32 ul_dot11TxMaximumNumberSpatialStreamsSupported;
    oal_bool_enum_uint8 en_dot11TxUnequalModulationSupported;
} wlan_mib_Dot11PhyHTEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSTxTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSTxEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "he Transmit MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY." */
/* ::= { dot11phy 16 } */
/****************************************************************************************/
typedef struct {
    // oal_uint32  ul_dot11SupportedMCSTxIndex;                    /* dot11SupportedMCSTxIndex Unsigned32 */
    oal_uint8 auc_dot11SupportedMCSTxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSTxValue Unsigned32 */
    // oal_uint8   uc_num_tx_mcs;
    oal_uint8 auc_resv[1];
} wlan_mib_Dot11SupportedMCSTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSRxTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSRxEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "The receive MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY." */
/* ::= { dot11phy 17 } */
/****************************************************************************************/
typedef struct {
    // oal_uint32  ul_dot11SupportedMCSRxIndex;                             /* dot11SupportedMCSRxIndex Unsigned32 */
    oal_uint8 auc_dot11SupportedMCSRxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSRxValue Unsigned32 */
    // oal_uint8   uc_num_rx_mcs;
} wlan_mib_Dot11SupportedMCSRxEntry_stru;

/****************************************************************************************/
/* dot11TransmitBeamformingConfigTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11TransmitBeamformingConfigEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11TransmitBeamformingConfigTable. Implemented       */
/*      as a table indexed on ifIndex to allow for multiple instances on an             */
/*      Agent." */
/* ::= { dot11phy 18 } */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ReceiveStaggerSoundingOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitStaggerSoundingOptionImplemented;
    oal_bool_enum_uint8 en_dot11ReceiveNDPOptionImplemented;
    oal_bool_enum_uint8 en_dot11TransmitNDPOptionImplemented;
    oal_bool_enum_uint8 en_dot11ImplicitTransmitBeamformingOptionImplemented;
    oal_uint8 uc_dot11CalibrationOptionImplemented;
    oal_bool_enum_uint8 en_dot11ExplicitCSITransmitBeamformingOptionImplemented;
    oal_bool_enum_uint8 en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented;
    oal_uint8 uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented;
    oal_uint8 uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented;
    oal_uint8 uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented;
    oal_uint32 ul_dot11NumberBeamFormingCSISupportAntenna;
    oal_uint32 ul_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
    oal_uint32 ul_dot11NumberCompressedBeamformingMatrixSupportAntenna;
} wlan_mib_Dot11TransmitBeamformingConfigEntry_stru;

/****************************************************************************************/
/* dot11PhyVHTTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11PhyVHTEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11PhyVHTTable. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent." */
/* ::= { dot11phy 23 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 uc_dot11VHTChannelWidthOptionImplemented; /* dot11VHTChannelWidthOptionImplemented INTEGER */
    // oal_uint8           uc_dot11CurrentChannelwidth;                        /* dot11CurrentChannelWidth INTEGER */
    // oal_uint32          ul_dot11CurrentChannelCenterFrequencyIndex0;
    // oal_uint32          ul_dot11CurrentChannelCenterFrequencyIndex1;
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Implemented; /* dot11VHTShortGIOptionIn80Implemented TruthValue */
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Activated;
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Implemented;
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Activated;
    oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionImplemented; /* dot11VHTLDPCCodingOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionActivated;
    oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionImplemented; /* dot11VHTTxSTBCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionActivated;
    oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionImplemented; /* dot11VHTRxSTBCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionActivated;
    // oal_uint32          ul_dot11VHTMUMaxUsersImplemented;
    // oal_uint32          ul_dot11VHTMUMaxNSTSPerUserImplemented;
    // oal_uint32          ul_dot11VHTMUMaxNSTSTotalImplemented;
} wlan_mib_Dot11PhyVHTEntry_stru;

/****************************************************************************************/
/* dot11VHTTransmitBeamformingConfigTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11VHTTransmitBeamformingConfigEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "Entry of attributes for dot11VHTTransmitBeamformingConfigTable. Imple-         */
/*      mented as a table indexed on ifIndex to allow for multiple instances on an      */
/*      Agent." */
/* ::= { dot11phy 24 } */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11VHTSUBeamformeeOptionImplemented; /* dot11VHTSUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTSUBeamformerOptionImplemented; /* dot11VHTSUBeamformerOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformeeOptionImplemented; /* dot11VHTMUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformerOptionImplemented; /* dot11VHTMUBeamformerOptionImplemented TruthValue */
    oal_uint32 ul_dot11VHTNumberSoundingDimensions;               /* dot11VHTNumberSoundingDimensions  Unsigned32 */
    oal_uint32 ul_dot11VHTBeamformeeNTxSupport;                   /* dot11VHTBeamformeeNTxSupport Unsigned32 */
} wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru;

/* Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6} */
/****************************************************************************************/
/* Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6} */
/* -- IMT GROUPS */
/* -- dot11BSSIdTable ::= { dot11imt 1 } */
/* -- dot11InterworkingTable ::= { dot11imt 2 } */
/* -- dot11APLCI ::= { dot11imt 3 } */
/* -- dot11APCivicLocation ::= { dot11imt 4 } */
/* -- dot11RoamingConsortiumTable      ::= { dot11imt 5 } */
/* -- dot11DomainNameTable ::= { dot11imt 6 } */
/* -- Generic Advertisement Service (GAS) Attributes */
/* -- DEFINED AS "The Generic Advertisement Service management                 */
/*          -- object class provides the necessary support for an Advertisement         */
/*          -- service to interwork with external systems." */
/* -- GAS GROUPS */
/* -- dot11GASAdvertisementTable       ::= { dot11imt 7 } */
/****************************************************************************************/
/****************************************************************************************/
/* dot11BSSIdTable OBJECT-TYPE */
/* SYNTAX         SEQUENCE OF Dot11BSSIdEntry */
/* MAX-ACCESS     not-accessible */
/* STATUS         current */
/* DESCRIPTION */
/* "This object is a table of BSSIDs contained within an Access Point (AP)." */
/* ::= { dot11imt 1 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11APMacAddress[6]; /* dot11APMacAddress OBJECT-TYPE MacAddress */
} wlan_mib_Dot11BSSIdEntry_stru;

/****************************************************************************************/
/* dot11InterworkingTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11InterworkingEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This table represents the non-AP STAs associated to the AP. An entry is        */
/*      created automatically by the AP when the STA becomes associated to the AP.      */
/*      The corresponding entry is deleted when the STA disassociates. Each STA         */
/*      added to this table is uniquely identified by its MAC address. This table       */
/*      is moved to a new AP following a successful STA BSS transition event." */
/* ::= { dot11imt 2 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11NonAPStationMacAddress[6];                /* dot11NonAPStationMacAddress MacAddress */
    oal_uint8 auc_dot11NonAPStationUserIdentity[255];            /* dot11NonAPStationUserIdentity DisplayString */
    oal_uint8 uc_dot11NonAPStationInterworkingCapability;        /* dot11NonAPStationInterworkingCapability BITS */
    oal_uint8 auc_dot11NonAPStationAssociatedSSID[32];           /* dot11NonAPStationAssociatedSSID OCTET STRING */
    oal_uint8 auc_dot11NonAPStationUnicastCipherSuite[4];        /* dot11NonAPStationUnicastCipherSuite OCTET STRING */
    oal_uint8 auc_dot11NonAPStationBroadcastCipherSuite[4];     /* dot11NonAPStationBroadcastCipherSuite OCTET STRING */
    oal_uint8 uc_dot11NonAPStationAuthAccessCategories;          /* dot11NonAPStationAuthAccessCategories BITS */
    oal_uint32 ul_dot11NonAPStationAuthMaxVoiceRate;             /* dot11NonAPStationAuthMaxVoiceRate Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxVideoRate;             /* dot11NonAPStationAuthMaxVideoRate Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxBestEffortRate;       /* dot11NonAPStationAuthMaxBestEffortRate Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxBackgroundRate;       /* dot11NonAPStationAuthMaxBackgroundRate Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxVoiceOctets;          /* dot11NonAPStationAuthMaxVoiceOctets Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxVideoOctets;         /* dot11NonAPStationAuthMaxVideoOctets Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxBestEffortOctets;    /* dot11NonAPStationAuthMaxBestEffortOctets Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxBackgroundOctets;    /* dot11NonAPStationAuthMaxBackgroundOctets Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxHCCAHEMMOctets;        /* dot11NonAPStationAuthMaxHCCAHEMMOctets Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthMaxTotalOctets;           /* dot11NonAPStationAuthMaxTotalOctets Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthHCCAHEMM;        /* dot11NonAPStationAuthHCCAHEMM TruthValue */
    oal_uint32 ul_dot11NonAPStationAuthMaxHCCAHEMMRate;          /* dot11NonAPStationAuthMaxHCCAHEMMRate Unsigned32 */
    oal_uint32 ul_dot11NonAPStationAuthHCCAHEMMDelay;            /* dot11NonAPStationAuthHCCAHEMMDelay Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthSourceMulticast; /* dot11NonAPStationAuthSourceMulticast TruthValue */
    oal_uint32 ul_dot11NonAPStationAuthMaxSourceMulticastRate;
    oal_uint32 ul_dot11NonAPStationVoiceMSDUCount;               /* dot11NonAPStationVoiceMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedVoiceMSDUCount;        /* dot11NonAPStationDroppedVoiceMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationVoiceOctetCount;              /* dot11NonAPStationVoiceOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedVoiceOctetCount;       /* dot11NonAPStationDroppedVoiceOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationVideoMSDUCount;               /* dot11NonAPStationVideoMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedVideoMSDUCount;        /* dot11NonAPStationDroppedVideoMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationVideoOctetCount;              /* dot11NonAPStationVideoOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedVideoOctetCount;       /* dot11NonAPStationDroppedVideoOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationBestEffortMSDUCount;          /* dot11NonAPStationBestEffortMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedBestEffortMSDUCount;
    oal_uint32 ul_dot11NonAPStationBestEffortOctetCount;         /* dot11NonAPStationBestEffortOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedBestEffortOctetCount;
    oal_uint32 ul_dot11NonAPStationBackgroundMSDUCount;          /* dot11NonAPStationBackgroundMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedBackgroundMSDUCount;
    oal_uint32 ul_dot11NonAPStationBackgroundOctetCount;         /* dot11NonAPStationBackgroundOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedBackgroundOctetCount;
    oal_uint32 ul_dot11NonAPStationHCCAHEMMMSDUCount;            /* dot11NonAPStationHCCAHEMMMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedHCCAHEMMMSDUCount;  /* dot11NonAPStationDroppedHCCAHEMMMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationHCCAHEMMOctetCount;           /* dot11NonAPStationHCCAHEMMOctetCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedHCCAHEMMOctetCount;    /* dot11NonAPStationDroppedHCCAHEMMOctetCount */
    oal_uint32 ul_dot11NonAPStationMulticastMSDUCount;           /* dot11NonAPStationMulticastMSDUCount Counter32 */
    oal_uint32 ul_dot11NonAPStationDroppedMulticastMSDUCount;    /* dot11NonAPStationDroppedMulticastMSDUCount */
    oal_uint32 ul_dot11NonAPStationMulticastOctetCount;          /* dot11NonAPStationMulticastOctetCount */
    oal_uint32 ul_dot11NonAPStationDroppedMulticastOctetCount;   /* dot11NonAPStationDroppedMulticastOctetCount */
    oal_uint8 uc_dot11NonAPStationPowerManagementMode;           /* dot11NonAPStationPowerManagementMode INTEGER */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthDls;             /* dot11NonAPStationAuthDls TruthValue */
    oal_uint32 ul_dot11NonAPStationVLANId;                       /* dot11NonAPStationVLANId Unsigned32 */
    oal_uint8 auc_dot11NonAPStationVLANName[64];     /* dot11NonAPStationVLANName DisplayString SIZE(0..64) */
    oal_uint8 uc_dot11NonAPStationAddtsResultCode;               /* dot11NonAPStationAddtsResultCode INTEGER */
} wlan_mib_Dot11InterworkingEntry_stru;

/****************************************************************************************/
/* dot11APLCITable OBJECT-TYPE */
/* SYNTAX         SEQUENCE OF Dot11APLCIEntry */
/* MAX-ACCESS     not-accessible */
/* STATUS         current */
/* DESCRIPTION */
/* "This table represents the Geospatial location of the AP as specified in        */
/*      8.4.2.23.10." */
/* ::= { dot11imt 3 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11APLCIIndex;                                    /* dot11APLCIIndex Unsigned32 */
    oal_uint32 ul_dot11APLCILatitudeResolution;                       /* dot11APLCILatitudeResolution Unsigned32 */
    oal_int32 l_dot11APLCILatitudeInteger;                            /* dot11APLCILatitudeInteger Integer32 */
    oal_int32 l_dot11APLCILatitudeFraction;                           /* dot11APLCILatitudeFraction Integer32 */
    oal_uint32 ul_dot11APLCILongitudeResolution;                      /* dot11APLCILongitudeResolution Unsigned32 */
    oal_int32 l_dot11APLCILongitudeInteger;                           /* dot11APLCILongitudeInteger Integer32 */
    oal_int32 l_dot11APLCILongitudeFraction;                          /* dot11APLCILongitudeFraction Integer32 */
    oal_uint8 uc_dot11APLCIAltitudeType;                              /* dot11APLCIAltitudeType INTEGER */
    oal_uint32 ul_dot11APLCIAltitudeResolution;                       /* dot11APLCIAltitudeResolution Unsigned32 */
    oal_int32 l_dot11APLCIAltitudeInteger;                            /* dot11APLCIAltitudeInteger Integer32 */
    oal_int32 l_dot11APLCIAltitudeFraction;                           /* dot11APLCIAltitudeFraction Integer32 */
    wlan_mib_ap_lci_datum_enum_uint8 en_dot11APLCIDatum;              /* dot11APLCIDatum INTEGER */
    wlan_mib_ap_lci_azimuth_type_enum_uint8 en_dot11APLCIAzimuthType; /* dot11APLCIAzimuthType INTEGER */
    oal_uint32 ul_dot11APLCIAzimuthResolution;                        /* dot11APLCIAzimuthResolution Unsigned32 */
    oal_int32 l_dot11APLCIAzimuth;                                    /* dot11APLCIAzimuth Integer32 */
} wlan_mib_Dot11APLCIEntry_stru;

/****************************************************************************************/
/* dot11APCivicLocationTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11ApCivicLocationEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This table represents the location of the AP in Civic format using the         */
/*      Civic Address Type elements defined in IETF RFC-5139 [B42]." */
/* ::= { dot11imt 4 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11APCivicLocationIndex;        /* dot11APCivicLocationIndex Unsigned32 */
    oal_uint8 auc_dot11APCivicLocationCountry[255]; /* dot11APCivicLocationCountry OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA1[255];      /* dot11APCivicLocationA1 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA2[255];      /* dot11APCivicLocationA2 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA3[255];      /* dot11APCivicLocationA3 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA4[255];      /* dot11APCivicLocationA4 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA5[255];      /* dot11APCivicLocationA5 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationA6[255];      /* dot11APCivicLocationA6 OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPrd[255];     /* dot11APCivicLocationPrd OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPod[255];     /* dot11APCivicLocationPod OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationSts[255];     /* dot11APCivicLocationSts OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationHno[255];     /* dot11APCivicLocationHno OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationHns[255];     /* dot11APCivicLocationHns OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationLmk[255];     /* dot11APCivicLocationLmk OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationLoc[255];     /* dot11APCivicLocationLoc OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationNam[255];     /* dot11APCivicLocationNam OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPc[255];      /* dot11APCivicLocationPc OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationBld[255];     /* dot11APCivicLocationBld OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationUnit[255];    /* dot11APCivicLocationUnit OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationFlr[255];     /* dot11APCivicLocationFlr OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationRoom[255];    /* dot11APCivicLocationRoom OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPlc[255];     /* dot11APCivicLocationPlc OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPcn[255];     /* dot11APCivicLocationPcn OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPobox[255];   /* dot11APCivicLocationPobox OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationAddcode[255]; /* dot11APCivicLocationAddcode OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationSeat[255];    /* dot11APCivicLocationSeat OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationRd[255];      /* dot11APCivicLocationRd OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationRdsec[255];   /* dot11APCivicLocationRdsec OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationRdbr[255];    /* dot11APCivicLocationRdbr OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationRdsubbr[255]; /* dot11APCivicLocationRdsubbr OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPrm[255];     /* dot11APCivicLocationPrm OCTET STRING */
    oal_uint8 auc_dot11APCivicLocationPom[255];     /* dot11APCivicLocationPom OCTET STRING */
} wlan_mib_Dot11ApCivicLocationEntry_stru;

/****************************************************************************************/
/* dot11RoamingConsortiumTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11RoamingConsortiumEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This is a Table of OIs which are to be transmitted in an ANQP Roaming          */
/*      Consortium ANQP-element. Each table entry corresponds to a roaming consor-      */
/*      tium or single SSP. The first 3 entries in this table are transmitted in        */
/*      Beacon and Probe Response frames." */
/* ::= { dot11imt 5 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11RoamingConsortiumOI[16];
    wlan_mib_row_status_enum_uint8 en_dot11RoamingConsortiumRowStatus;
} wlan_mib_Dot11RoamingConsortiumEntry_stru;

/****************************************************************************************/
/* dot11DomainNameTable   OBJECT-TYPE */
/* SYNTAX                SEQUENCE OF Dot11DomainNameEntry */
/* MAX-ACCESS            not-accessible */
/* STATUS                current */
/* DESCRIPTION */
/* "This is a table of Domain Names which form the Domain Name list in Access      */
/*      Network Query Protocol. The Domain Name list may be transmitted to a non-       */
/*      AP STA in a GAS Response. Each table entry corresponds to a single Domain       */
/*      Name." */
/* ::= { dot11imt 6 } */
/****************************************************************************************/
typedef struct {
    oal_uint8 auc_dot11DomainName[255];
    wlan_mib_row_status_enum_uint8 en_dot11DomainNameRowStatus;
    oal_uint8 auc_dot11DomainNameOui[5];
} wlan_mib_Dot11DomainNameEntry_stru;

/****************************************************************************************/
/* dot11GASAdvertisementTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11GASAdvertisementEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This object is a table of GAS counters that allows for multiple instanti-      */
/*      ations of those counters on an STA." */
/* ::= { dot11imt 7 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11GASAdvertisementId;
    oal_bool_enum_uint8 en_dot11GASPauseForServerResponse;
    oal_uint32 ul_dot11GASResponseTimeout;
    oal_uint32 ul_dot11GASComebackDelay;
    oal_uint32 ul_dot11GASResponseBufferingTime;
    oal_uint32 ul_dot11GASQueryResponseLengthLimit;
    oal_uint32 ul_dot11GASQueries;
    oal_uint32 ul_dot11GASQueryRate;
    oal_uint32 ul_dot11GASResponses;
    oal_uint32 ul_dot11GASResponseRate;
    oal_uint32 ul_dot11GASTransmittedFragmentCount;
    oal_uint32 ul_dot11GASReceivedFragmentCount;
    oal_uint32 ul_dot11GASNoRequestOutstanding;
    oal_uint32 ul_dot11GASResponsesDiscarded;
    oal_uint32 ul_dot11GASFailedResponses;
} wlan_mib_Dot11GASAdvertisementEntry_stru;

/****************************************************************************************/
/* Start of dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7} */
/* -- MAC State GROUPS */
/* -- dot11MACStateConfigTable ::= { dot11MSGCF 1 } */
/* -- dot11MACStateParameterTable ::= { dot11MSGCF 2 } */
/* -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 } */
/****************************************************************************************/
/****************************************************************************************/
/* dot11MACStateConfigTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11MACStateConfigEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This table holds configuration parameters for the 802.11 MAC                   */
/*      State Convergence Function." */
/* ::= { dot11MSGCF 1 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11ESSDisconnectFilterInterval;   /* dot11ESSDisconnectFilterInterval Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDetectionHoldInterval;  /* dot11ESSLinkDetectionHoldInterval Unsigned32 */
    oal_uint8 auc_dot11MSCEESSLinkIdentifier[38];     /* dot11MSCEESSLinkIdentifier Dot11ESSLinkIdentifier */
    oal_uint8 auc_dot11MSCENonAPStationMacAddress[6]; /* dot11MSCENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateConfigEntry_stru;

/****************************************************************************************/
/* dot11MACStateParameterTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11MACStateParameterEntry */
/* MAX-ACCESS not-accessible */
/* STATUS     current */
/* DESCRIPTION */
/* "This table holds the current parameters used for each 802.11 network for       */
/*      802.11 MAC convergence functions." */
/* ::= { dot11MSGCF 2 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11ESSLinkDownTimeInterval;                      /* dot11ESSLinkDownTimeInterval Unsigned32 */
    oal_uint32 ul_dot11ESSLinkRssiDataThreshold;                     /* dot11ESSLinkRssiDataThreshold Unsigned32 */
    oal_uint32 ul_dot11ESSLinkRssiBeaconThreshold;                   /* dot11ESSLinkRssiBeaconThreshold Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDataSnrThreshold;                      /* dot11ESSLinkDataSnrThreshold Unsigned32 */
    oal_uint32 ul_dot11ESSLinkBeaconSnrThreshold;                    /* dot11ESSLinkBeaconSnrThreshold Unsigned32 */
    oal_uint32 ul_dot11ESSLinkBeaconFrameErrorRateThresholdInteger;
    oal_uint32 ul_dot11ESSLinkBeaconFrameErrorRateThresholdFraction;
    oal_uint32 ul_dot11ESSLinkBeaconFrameErrorRateThresholdExponent;
    oal_uint32 ul_dot11ESSLinkFrameErrorRateThresholdInteger;
    oal_uint32 ul_dot11ESSLinkFrameErrorRateThresholdFraction;
    oal_uint32 ul_dot11ESSLinkFrameErrorRateThresholdExponent;
    oal_uint32 ul_dot11PeakOperationalRate;                          /* dot11PeakOperationalRate Unsigned32 */
    oal_uint32 ul_dot11MinimumOperationalRate;                       /* dot11MinimumOperationalRate Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDataThroughputInteger;                 /* dot11ESSLinkDataThroughputInteger Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDataThroughputFraction;                /* dot11ESSLinkDataThroughputFraction Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDataThroughputExponent;                /* dot11ESSLinkDataThroughputExponent Unsigned32 */
    oal_uint8 auc_dot11MSPEESSLinkIdentifier[38];
    oal_uint8 auc_dot11MSPENonAPStationMacAddress[6];                /* dot11MSPENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateParameterEntry_stru;

/****************************************************************************************/
/* dot11MACStateESSLinkDetectedTable OBJECT-TYPE */
/* SYNTAX SEQUENCE OF Dot11MACStateESSLinkDetectedEntry */
/* MAX-ACCESS not-accessible */
/* STATUS current */
/* DESCRIPTION */
/* "This table holds the detected 802.11 network list used for MAC conver-         */
/*      gence functions." */
/* ::= { dot11MSGCF 3 } */
/****************************************************************************************/
typedef struct {
    oal_uint32 ul_dot11ESSLinkDetectedIndex;                 /* dot11ESSLinkDetectedIndex Unsigned32 */
    oal_int8 ac_dot11ESSLinkDetectedNetworkId[255];          /* dot11ESSLinkDetectedNetworkId OCTET STRING */
    oal_uint32 ul_dot11ESSLinkDetectedNetworkDetectTime;     /* dot11ESSLinkDetectedNetworkDetectTime Unsigned32 */
    oal_uint32 ul_dot11ESSLinkDetectedNetworkModifiedTime;   /* dot11ESSLinkDetectedNetworkModifiedTime Unsigned32 */
    oal_uint8 uc_dot11ESSLinkDetectedNetworkMIHCapabilities; /* dot11ESSLinkDetectedNetworkMIHCapabilities BITS */
    oal_uint8 auc_dot11MSELDEESSLinkIdentifier[38];          /* dot11MSELDEESSLinkIdentifier  Dot11ESSLinkIdentifier */
    oal_uint8 auc_dot11MSELDENonAPStationMacAddress[6];      /* dot11MSELDENonAPStationMacAddress  MacAddress */
} wlan_mib_Dot11MACStateESSLinkDetectedEntry_stru;

typedef struct {
    /***************************************************************************
    dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }
****************************************************************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    wlan_mib_Dot11StationConfigEntry_stru st_wlan_mib_sta_config;

    /* --  dot11AuthenticationAlgorithmsTable ::= { dot11smt 2 } */
    // wlan_mib_Dot11AuthenticationAlgorithmsEntry_stru st_wlan_mib_auth_alg;
    /* --  dot11WEPDefaultKeysTable ::= { dot11smt 3 } */
    wlan_mib_Dot11WEPDefaultKeysEntry_stru ast_wlan_mib_wep_dflt_key[WLAN_NUM_DOT11WEPDEFAULTKEYVALUE];
    /* --  dot11WEPKeyMappingsTable ::= { dot11smt 4 } */
    // wlan_mib_Dot11WEPKeyMappingsEntry_stru st_wlan_mib_wep_key_map;
    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    wlan_mib_Dot11PrivacyEntry_stru st_wlan_mib_privacy;
    /* --  dot11SMTnotification ::= { dot11smt 6 } */
    /* --  dot11MultiDomainCapabilityTable ::= { dot11smt 7 } */
    // wlan_mib_Dot11MultiDomainCapabilityEntry_stru st_wlan_mib_multi_domain_cap;
    /* --  dot11SpectrumManagementTable ::= { dot11smt 8 } */
    // wlan_mib_dot11SpectrumManagementEntry_stru st_wlan_mib_spectrum_mgmt;
    /* --  dot11RSNAConfigTable ::= { dot11smt 9 } */
    // wlan_mib_dot11RSNAConfigEntry_stru st_wlan_mib_rsna_cfg;
    /* --  dot11RSNAConfigPairwiseCiphersTable ::= { dot11smt 10 } */
    /* --  dot11RSNAConfigAuthenticationSuitesTable      ::= { dot11smt 11 } */
    /* --  dot11RSNAStatsTable ::= { dot11smt 12 } */
    /* --  dot11OperatingClassesTable ::= { dot11smt 13 } */
    // wlan_mib_Dot11OperatingClassesEntry_stru st_wlan_mib_op_class;
    /* --  dot11RadioMeasurement ::= { dot11smt 14 } */
    // wlan_mib_dot11RadioMeasurement_stru st_wlan_mib_radio_measure;
#ifdef _PRE_WLAN_FEATURE_11R
    /* --  dot11FastBSSTransitionConfigTable ::= { dot11smt 15 } */
    wlan_mib_Dot11FastBSSTransitionConfigEntry_stru st_wlan_mib_fast_bss_trans_cfg;
#endif
    /* --  dot11LCIDSETable        ::= { dot11smt 16 } */
    // wlan_mib_Dot11LCIDSEEntry_stru st_wlan_mib_lcidse;
    /* --  dot11HTStationConfigTable  ::= { dot11smt 17 } */
    wlan_mib_Dot11HTStationConfigEntry_stru st_wlan_mib_ht_sta_cfg;
#if defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* --  dot11WirelessMgmtOptionsTable ::= { dot11smt 18} */
    wlan_mib_Dot11WirelessMgmtOptionsEntry_stru st_wlan_mib_wireless_mgmt_op;
#endif

    /* --  dot11LocationServicesNextIndex ::= { dot11smt 19} */
    // oal_uint32          ul_dot11LocationServicesNextIndex;
    /* --  dot11LocationServicesTable ::= { dot11smt 20} */
    // wlan_mib_Dot11LocationServicesEntry_stru st_wlan_mib_location_serv;
    /* --  dot11WirelessMGTEventTable ::= { dot11smt 21} */
    // wlan_mib_Dot11WirelessMGTEventEntry_stru st_wlan_mib_wireless_mgt_event;
    /* --  dot11WirelessNetworkManagement ::= { dot11smt 22} */
    // wlan_mib_dot11WirelessNetworkManagement_stru st_wlan_mib_wireless_net_mgmt;
    /* --  dot11MeshSTAConfigTable ::= { dot11smt 23 } */
    // wlan_mib_Dot11MeshSTAConfigEntry_stru st_wlan_mib_mesh_sta_cfg;
    /* --  dot11MeshHWMPConfigTable ::= { dot11smt 24 } */
    // wlan_mib_Dot11MeshHWMPConfigEntry_stru st_wlan_mib_mesh_hwmp_cfg;
    /* --  dot11RSNAConfigPasswordValueTable ::= { dot11smt 25 } */
    // wlan_mib_Dot11RSNAConfigPasswordValueEntry_stru st_wlan_mib_rsna_cfg_pswd;
    /* --  dot11RSNAConfigDLCGroupTable ::= { dot11smt 26 } */
    // wlan_mib_Dot11RSNAConfigDLCGroupEntry_stru st_wlan_mib_rsna_cfg_dlc_group;
    /* --  dot11VHTStationConfig ::= { dot11smt 31 } */
    wlan_mib_Dot11VHTStationConfigEntry_stru st_wlan_mib_vht_sta_config;
    /***************************************************************************
    dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }
****************************************************************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    wlan_mib_Dot11OperationEntry_stru st_wlan_mib_operation;
    /* --  dot11CountersTable ::= { dot11mac 2 } */
    wlan_mib_Dot11CountersEntry_stru st_wlan_mib_counters;
    /* --  dot11GroupAddressesTable ::= { dot11mac 3 } */
    // wlan_mib_Dot11GroupAddressesEntry_stru     st_wlan_mib_group_addresses;
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    wlan_mib_Dot11EDCAEntry_stru ast_wlan_mib_edca[WLAN_WME_AC_BUTT];
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    wlan_mib_Dot11QAPEDCAEntry_stru st_wlan_mib_qap_edac[WLAN_WME_AC_BUTT];

    /* --  dot11QosCountersTable ::= { dot11mac 6 } */
    // wlan_mib_Dot11QosCountersEntry_stru        st_wlan_mib_qos_counters;
    /* --  dot11ResourceInfoTable    ::= { dot11mac 7 } */
    // wlan_mib_Dot11ResourceInfoEntry_stru       st_wlan_mib_resource_info;
/***************************************************************************
dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 }
****************************************************************************/
/* -- dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 } */
// wlan_mib_dot11resAttribute_stru            st_wlan_mib_res_attribute;
/***************************************************************************
dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 }
****************************************************************************/
    /* --  dot11PhyOperationTable ::= { dot11phy 1 } */
    // wlan_mib_Dot11PhyOperationEntry_stru   st_wlan_mib_phy_operation;
    /* --  dot11PhyAntennaTable ::= { dot11phy 2 } */
    wlan_mib_Dot11PhyAntennaEntry_stru st_wlan_mib_phy_antenna;

    /* --  dot11PhyTxPowerTable ::= { dot11phy 3 } */
    // wlan_mib_Dot11PhyTxPowerEntry_stru     st_wlan_mib_phy_txpower;
    /* --  dot11PhyFHSSTable ::= { dot11phy 4 } */
    // wlan_mib_Dot11PhyFHSSEntry_stru        st_wlan_mib_phy_fhss;
    /* --  dot11PhyDSSSTable ::= { dot11phy 5 } */
    wlan_mib_Dot11PhyDSSSEntry_stru st_wlan_mib_phy_dsss;

    /* --  dot11PhyIRTable ::= { dot11phy 6 } */
    // wlan_mib_Dot11PhyIREntry_stru          st_wlan_mib_phy_ir;
    /* --  dot11RegDomainsSupportedTable ::= { dot11phy 7 } */
    // wlan_mib_Dot11RegDomainsSupportedEntry_stru st_wlan_mib_regdomain_supported;
    /* --  dot11AntennasListTable ::= { dot11phy 8 } */
    // wlan_mib_Dot11AntennasListEntry_stru       st_wlan_mib_antennas_list;
    /* --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 } */
    // wlan_mib_Dot11SupportedDataRatesTxEntry_stru    st_wlan_mib_supporteddatarates_tx;
    /* --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 } */
    // wlan_mib_Dot11SupportedDataRatesRxEntry_stru    st_wlan_mib_supporteddatarates_rx;
    /* --  dot11PhyOFDMTable ::= { dot11phy 11 } */
    // wlan_mib_Dot11PhyOFDMEntry_stru            st_phy_ofdm;
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    wlan_mib_Dot11PhyHRDSSSEntry_stru st_phy_hrdsss;

    /* --  dot11HoppingPatternTable ::= { dot11phy 13 } */
    // wlan_mib_Dot11HoppingPatternEntry_stru     st_hopping_pattern;
    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    wlan_mib_Dot11PhyERPEntry_stru st_phy_erp;

    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    wlan_mib_Dot11PhyHTEntry_stru st_phy_ht;

    /* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
    wlan_mib_Dot11SupportedMCSTxEntry_stru st_supported_mcstx;

    /* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
    wlan_mib_Dot11SupportedMCSRxEntry_stru st_supported_mcsrx;

    /* --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 } */
    wlan_mib_Dot11TransmitBeamformingConfigEntry_stru st_wlan_mib_txbf_config;

    /* -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac) */
    wlan_mib_Dot11PhyVHTEntry_stru st_wlan_mib_phy_vht;

    /* -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
    wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru st_wlan_mib_vht_txbf_config;

    /***************************************************************************
    dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } (该组用于归类，暂不实现)
****************************************************************************/
    /***************************************************************************
    dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6}
****************************************************************************/
    /* -- dot11BSSIdTable ::= { dot11imt 1 } */
    // wlan_mib_Dot11BSSIdEntry_stru           st_wlan_mib_bssid;
    /* -- dot11InterworkingTable ::= { dot11imt 2 } */
    // wlan_mib_Dot11InterworkingEntry_stru    st_wlan_mib_inter_working;
    /* -- dot11APLCI ::= { dot11imt 3 } */
    // wlan_mib_Dot11APLCIEntry_stru           st_wlan_mib_aplci;
    /* -- dot11APCivicLocation ::= { dot11imt 4 } */
    // wlan_mib_Dot11ApCivicLocationEntry_stru st_wlan_mib_apcivic_location;
    /* -- dot11RoamingConsortiumTable      ::= { dot11imt 5 } */
    // wlan_mib_Dot11RoamingConsortiumEntry_stru  st_wlan_mib_roaming_consortium;
    /* -- dot11DomainNameTable ::= { dot11imt 6 } */
    // wlan_mib_Dot11DomainNameEntry_stru      st_wlan_mib_domain_name;
    /* -- dot11GASAdvertisementTable       ::= { dot11imt 7 } */
    // wlan_mib_Dot11GASAdvertisementEntry_stru   st_wlan_mib_gas_advertisement;
    /***************************************************************************
    dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7}
****************************************************************************/
    /* -- dot11MACStateConfigTable ::= { dot11MSGCF 1 } */
    // wlan_mib_Dot11MACStateConfigEntry_stru     st_wlan_mib_mac_state_config;
    /* -- dot11MACStateParameterTable ::= { dot11MSGCF 2 } */
    // wlan_mib_Dot11MACStateParameterEntry_stru  st_wlan_mib_mac_state_parameter;
    /* -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 } */
    // wlan_mib_Dot11MACStateESSLinkDetectedEntry_stru st_wlan_mib_mac_state_esslinkdetected;
} wlan_mib_ieee802dot11_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
10 函数声明
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_mib.h */
