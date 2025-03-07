

#ifndef __HMAC_VAP_H__
#define __HMAC_VAP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "mac_resource.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#include "oal_hcc_host_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hmac_btcoex.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VAP_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#define   hmac_vap_dft_stats_pkt_incr(_member, _cnt)        ((_member) += (_cnt))
#else
#define   hmac_vap_dft_stats_pkt_incr(_member, _cnt)
#endif
#define   hmac_vap_stats_pkt_incr(_member, _cnt)            ((_member) += (_cnt))

#ifdef _PRE_WLAN_FEATURE_HS20
#define MAX_QOS_UP_RANGE  8
#define MAX_DSCP_EXCEPT   21  /* maximum of DSCP Exception fields for QoS Map set */
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#define HMAC_GAS_DIALOG_TOKEN_INITIAL_VALUE 0x80        /* hmac gas的dialog token num */
#endif
#define HMAC_STA_RSSI_LOG_NUM 3 /* STA记录RSSI历史的个数 */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
    初始化vap特性枚举
*****************************************************************************/
typedef enum {
    HMAC_ADDBA_MODE_AUTO,
    HMAC_ADDBA_MODE_MANUAL,

    HMAC_ADDBA_MODE_BUTT
}hmac_addba_mode_enum;
typedef oal_uint8 hmac_addba_mode_enum_uint8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_uint8   g_uc_host_rx_ampdu_amsdu;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#ifdef _PRE_WLAN_CHBA_MGMT
typedef struct hmac_chba_vap_tag hmac_chba_vap_stru;
#endif

typedef struct {
    oal_dlist_head_stru st_timeout_head;
}hmac_mgmt_timeout_stru;

typedef struct {
    oal_uint16                  us_user_index;
    mac_vap_state_enum_uint8    en_state;
    oal_uint8                   uc_vap_id;
    mac_status_code_enum_uint16 en_status_code;
    oal_uint16                  auc_rsv[2];
}hmac_mgmt_timeout_param_stru;

#ifdef _PRE_WLAN_FEATURE_HS20
typedef struct {
    oal_uint8  auc_up_low[MAX_QOS_UP_RANGE];             /* User Priority */
    oal_uint8  auc_up_high[MAX_QOS_UP_RANGE];
    oal_uint8  auc_dscp_exception_up[MAX_DSCP_EXCEPT];   /* User Priority of DSCP Exception field */
    oal_uint8  uc_valid;
    oal_uint8  uc_num_dscp_except;
    oal_uint8  auc_dscp_exception[MAX_DSCP_EXCEPT];      /* DSCP exception field  */
}hmac_cfg_qos_map_param_stru;
#endif

/* 修改此结构体需要同步通知SDT，否则上报无法解析 */
typedef struct {
    /***************************************************************************
                                收送包统计
    ***************************************************************************/
    /* 发往lan的数据包统计 */
    oal_uint32  ul_rx_pkt_to_lan;                               /* 接收流程发往以太网的数据包数目，MSDU */
    oal_uint32  ul_rx_bytes_to_lan;                             /* 接收流程发往以太网的字节数 */

    /***************************************************************************
                                发送包统计
     ***************************************************************************/
    /* 从lan接收到的数据包统计 */
    oal_uint32  ul_tx_pkt_num_from_lan;                         /* 从lan过来的包数目,MSDU */
    oal_uint32  ul_tx_bytes_from_lan;                           /* 从lan过来的字节数 */
}hmac_vap_query_stats_stru;
/* 装备测试 */
typedef struct {
    oal_uint32                       ul_rx_pkct_succ_num;                       /* 接收数据包数 */
    oal_uint32                       ul_dbb_num;                                /* DBB版本号 */
    oal_uint32                       ul_check_fem_pa_status;                    /* fem和pa是否烧毁标志 */
    oal_int16                        s_rx_rssi;
    oal_bool_enum_uint8              uc_get_dbb_completed_flag;                 /* 获取DBB版本号成功上报标志 */
    oal_bool_enum_uint8              uc_check_fem_pa_flag;                      /* fem和pa是否烧毁上报标志 */
    oal_bool_enum_uint8              uc_get_rx_pkct_flag;                       /* 接收数据包上报标志位 */
    oal_bool_enum_uint8              uc_lte_gpio_check_flag;                    /* 接收数据包上报标志位 */
    oal_bool_enum_uint8              uc_efuse_reg_flag;              /* efuse 寄存器读取 */
    oal_uint8                        uc_ant_status : 4,
                                     uc_get_ant_flag : 4;
}hmac_atcmdsrv_get_stats_stru;

typedef struct {
    oal_dlist_head_stru           st_entry;
    oal_uint8                     auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8                     uc_reserved[2];
    oal_uint8                     auc_pmkid[WLAN_PMKID_LEN];
}hmac_pmksa_cache_stru;

typedef enum _hmac_tcp_opt_queue_ {
    HMAC_TCP_ACK_QUEUE = 0,
    HMAC_TCP_OPT_QUEUE_BUTT
} hmac_tcp_opt_queue;

#ifdef _PRE_WLAN_TCP_OPT
typedef oal_uint16 (* hmac_trans_cb_func)(
    void *pst_hmac_device, hmac_tcp_opt_queue type, hcc_chan_type dir, oal_netbuf_head_stru* data);
/* tcp_ack优化 */
typedef struct {
    struct wlan_perform_tcp      st_hmac_tcp_ack;
    struct wlan_perform_tcp_list st_hmac_tcp_ack_list;
    wlan_perform_tcp_impls       filter_info;
    hmac_trans_cb_func           filter[HMAC_TCP_OPT_QUEUE_BUTT];   // 过滤处理钩子函数
    oal_uint64                   all_ack_count[HMAC_TCP_OPT_QUEUE_BUTT];    // 丢弃的TCP ACK统计
    oal_uint64                   drop_count[HMAC_TCP_OPT_QUEUE_BUTT];   // 丢弃的TCP ACK统计
    oal_netbuf_head_stru         data_queue[HMAC_TCP_OPT_QUEUE_BUTT];
    oal_spin_lock_stru           data_queue_lock[HMAC_TCP_OPT_QUEUE_BUTT];
}hmac_tcp_ack_stru;
#endif
typedef struct {
    oal_uint32 aul_rate[HMAC_STA_RSSI_LOG_NUM]; /* 记录STA历史接收速率 */
    oal_uint32 ul_index;   /* 下一个接收速率填充的位置索引,使用时不要忘记与存储空间大小取余 */
} hmac_sta_rx_rate_stru;

typedef struct {
    oal_uint32 auc_bssid[OAL_MAC_ADDR_LEN]; /* 记录需要需要以20M带宽进行关联的AP */
    oal_int8 c_rssi;
    oal_uint8 auc_res[1];
}hmac_sta_20m_ap_stru;

typedef struct {
    uint64_t device_tsf;
    uint64_t host_start_tsf;
    uint64_t host_end_tsf;
} sync_tsf_stru; /* 查询某一时刻host与device的tsf结构体 */
/* hmac vap结构体 */
/* 在向此结构体中增加成员的时候，请保持整个结构体8字节对齐 */
typedef struct hmac_vap_tag {
    /* ap sta公共字段 */
    oal_net_device_stru            *pst_net_device;                             /* VAP对应的net_devices */
    oal_uint8                       auc_name[OAL_IF_NAME_SIZE];                 /* VAP名字 */
    hmac_vap_cfg_priv_stru          st_cfg_priv;                                /* wal hmac配置通信接口 */

    oal_spin_lock_stru              st_lock_state;                              /* 数据面和控制面对VAP状态进行互斥 */
    oal_uint16                      us_user_nums_max;                           /* VAP下可挂接的最大用户个数 */
    oal_uint8                       uc_classify_tid;                            /* 仅在基于vap的流分类使能后有效 */
    wlan_auth_alg_enum_uint8        en_auth_mode;                               /* 认证算法 */

    oal_mgmt_tx_stru                st_mgmt_tx;
    frw_timeout_stru                st_mgmt_timer;
    hmac_mgmt_timeout_param_stru    st_mgmt_timetout_param;

    frw_timeout_stru                st_scan_timeout;              /* vap发起扫描时，会启动定时器，做超时保护处理 */

    hmac_addba_mode_enum_uint8      en_addba_mode;
#ifdef _PRE_WLAN_FEATURE_WMMAC
    oal_uint8                       uc_ts_dialog_token;                         /* TS会话创建伪随机值 */
#else
    oal_uint8                       uc_resv1;
#endif //_PRE_WLAN_FEATURE_WMMAC
    oal_uint8                       uc_80211i_mode;    /* 指示当前的方式时WPA还是WPA2, bit0 = 1,WPA; bit1 = 1, RSN */
    oal_uint8                       uc_ba_dialog_token;                         /* BA会话创建伪随机值 */
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
    mac_blacklist_info_stru         st_blacklist_info;                          /* 黑名单信息 */
    mac_isolation_info_stru         st_isolation_info;                          /* 用户隔离信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_11D
    oal_bool_enum_uint8             en_updata_rd_by_ie_switch;                  /* 是否根据关联的ap跟新自己的国家码 */
    oal_uint8                       auc_resv2[3];
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_net_device_stru            *pst_p2p0_net_device;                        /* 指向p2p0 net device */
    oal_net_device_stru            *pst_del_net_device;             /* 指向需要通过cfg80211 接口删除的 net device */
    oal_work_stru                   st_del_virtual_inf_worker;                  /* 删除net_device 工作队列 */
    oal_bool_enum_uint8             en_wait_roc_end;
    oal_uint8                       auc_resv11[3];
    oal_completion                  st_roc_end_ready;                           /* roc end completion */
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    hmac_cfg_qos_map_param_stru     st_cfg_qos_map_param;
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    oal_uint8                       bit_init_flag:1;                            /* 常发关闭再次打开标志 */
    oal_uint8                       bit_ack_policy:1;               /* ack policy: 0:normal ack 1:normal ack */
    oal_uint8                       bit_reserved:6;
    oal_uint8                       auc_resv4[3];
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_uint32                     *pul_roam_info;
    oal_uint32                      ul_last_roam_timestamp;                     /* 记录上一次触发漫游时间点 */
    oal_bool_enum_uint8             en_roam_prohibit_on;                        /* 是否禁止漫游 */
    oal_uint8                       en_last_roam_trigger;                       /* 记录上一次漫游类型, 用于判断是否出现漫游乒乓 */
    oal_uint8                       uc_diff_roam_trigger_cnt;                   /* 记录乒乓次数 */
#endif  //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_DFT_STAT
    oal_uint8                       uc_device_distance;
    oal_uint8                       uc_intf_state_cca;
    oal_uint8                       uc_intf_state_co;
    oal_uint8                       auc_resv[1];
#endif

    /* sta独有字段 */
    oal_uint8                       bit_sta_protocol_cfg    :   1;
    oal_uint8                       bit_protocol_fall       :   1;              /* 降协议标志位 */
    oal_uint8                       bit_reassoc_flag        :   1;        /* 关联过程中判断是否为重关联动作 */
    oal_uint8                       bit_sae_connect_with_pmkid :1; /* 0:不包含PMKID的SAE连接；1:包含PMKID的SAE连接 */
#if defined(_PRE_WLAN_FEATURE_11K)  || defined(_PRE_WLAN_FEATURE_FTM) || \
    defined(_PRE_WLAN_FEATURE_11V_ENABLE) || defined(_PRE_WLAN_FEATURE_11R)
    oal_uint8                       bit_11k_auth_flag       :   1;              /* 11k 认证标志位 */
    oal_uint8                       bit_voe_11r_auth        :   1;
    oal_uint8                       bit_11k_auth_oper_class :   2;
    oal_uint8                       bit_11r_over_ds         :   1; /* 是否使用11r over ds；0表示over ds走over air流程 */
    oal_uint8                       bit_bcn_table_switch    :   1;
    oal_uint8                       bit_11k_enable          :   1;
    oal_uint8                       bit_11v_enable          :   1;
    oal_uint8                       bit_11r_enable          :   1;
    oal_uint8                       bit_resv                :   3;

    oal_uint8                       auc_resv41[3];
#else
    oal_uint8                       bit_resv                :   4;
#endif //_PRE_WLAN_FEATURE_11K
    oal_int8                        ac_desired_country[3];                      /* 要加入的AP的国家字符串，前两个字节为国家字母，第三个为\0 */
    oal_uint32                      ul_asoc_req_ie_len;
    oal_uint8                      *puc_asoc_req_ie_buff;

    oal_uint8                       uc_wmm_cap;                                 /* 保存与STA关联的AP是否支持wmm能力信息 */
#ifdef _PRE_WLAN_FEATURE_HS20
    oal_uint8                       uc_is_interworking;                         /* 保存与STA关联的AP是否支持interworking能力 */
    oal_uint8                       auc_resv51[3];
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
    oal_uint8                       uc_cfg_sta_pm_manual;                           /* 手动设置sta pm mode的标志 */

#else
    oal_uint8                       auc_resv5[1];
#endif
    oal_uint16                      us_rx_timeout[WLAN_WME_AC_BUTT];            /* 不同业务重排序超时时间 */
    oal_uint16                      us_rx_timeout_min[WLAN_WME_AC_BUTT];            /* 不同业务重排序超时时间 */

    oal_uint16                      us_del_timeout;                   /* 多长时间超时删除ba会话 如果是0则不删除 */
    mac_cfg_mode_param_stru         st_preset_para;                          /* STA协议变更时变更前的协议模式 */
    oal_uint8                       auc_supp_rates[WLAN_MAX_SUPP_RATES];        /* 支持的速率集 */
    oal_uint8                       uc_rs_nrates;   /* 速率个数 */

    oal_uint8                       uc_auth_cnt;                                    /* 记录STA发起关联的次数 */
    oal_uint8                       uc_asoc_cnt;
    oal_uint8                       auc_resv56[2];

    oal_dlist_head_stru             st_pmksa_list_head;

    /* 信息上报 */
    oal_wait_queue_head_stru         query_wait_q;                              /* 查询等待队列 */
    oal_station_info_stru            station_info;
    station_info_extend_stru         st_station_info_extend;                    /* CHR2.0使用的STA统计信息 */
    /* 查询结束标志，OAL_TRUE，查询结束，OAL_FALSE，查询未结束 */
    oal_bool_enum_uint8              station_info_query_completed_flag;
    oal_int16                        s_free_power;                              /* 底噪 */
    oal_uint8                        auc_resv6[1];
    oal_int32                        center_freq;                               /* 中心频点 */
    hmac_atcmdsrv_get_stats_stru     st_atcmdsrv_get_status;
    chr_wifi_ext_info_query_stru     st_chr_wifi_ext_info_query;

    oal_proc_dir_entry_stru         *pst_proc_dir;                              /* vap对应的proc目录 */

#ifdef _PRE_WLAN_DFT_STAT
    /* 统计信息+信息上报新增字段，修改这个字段，必须修改SDT才能解析正确 */
    hmac_vap_query_stats_stru        st_query_stats;
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    frw_timeout_stru                 st_edca_opt_timer;                   /* edca参数调整定时器 */
    oal_uint32                       ul_edca_opt_time_ms;                 /* edca参数调整计时器周期 */
    oal_uint8                        uc_edca_opt_flag_ap;                 /* ap模式下是否使能edca优化特性 */
    oal_uint8                        uc_edca_opt_flag_sta;                /* sta模式下是否使能edca优化特性 */
    oal_uint8                        uc_edca_opt_weight_sta;              /* 调整beacon中edca参数的权重，最大值为 3 */
    oal_uint8                        auc_resv7[1];
#endif

#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_ack_stru                ast_hmac_tcp_ack[HCC_DIR_COUNT];
#endif
    /* ap下关联的40M intolerant的sta bitmap */
    oal_uint32                        aul_40m_intol_user_bitmap[MAC_DEV_MAX_40M_INTOL_USER_BITMAP_LEN];
    frw_timeout_stru                  st_40m_recovery_timer;                    /* 40M恢复定时器 */
    wlan_channel_bandwidth_enum_uint8 en_40m_bandwidth;                         /* 记录ap在切换到20M之前的速率 */

    oal_bool_enum_uint8               en_no_beacon;
    oal_bool_enum_uint8               en_addr_filter;
    oal_bool_enum_uint8               en_amsdu_active;
    oal_bool_enum_uint8               en_amsdu_ampdu_active;
    oal_bool_enum_uint8               en_small_amsdu_switch;                   /* 小包amsdu聚合的开关 */
    oal_bool_enum_uint8               en_rx_ampduplusamsdu_active;
    oal_bool_enum_uint8               en_psm_active;
    oal_bool_enum_uint8               en_wme_active;
    oal_bool_enum_uint8               en_wps_active;
    oal_bool_enum_uint8               en_msdu_defrag_active;
    oal_bool_enum_uint8               en_2040_switch_prohibited;
    oal_bool_enum_uint8               en_tx_aggr_on;
    oal_bool_enum_uint8               en_ampdu_tx_on_switch;
    oal_bool_enum_uint8               en_web_fail_roam;
    oal_uint32                        ul_web_fail_roam_timestamp;             /* 记录上次不能上网事件触发漫游的时间 */
    oal_uint32                        ul_assoc_timestamp;                     /* 记录关联成功的时间 */
    hmac_sta_rx_rate_stru             st_history_rx_rate;                     /* 记录STA的历史接收速率 */
    hmac_sta_20m_ap_stru              st_20m_ap_info;                         /* 记录STA需要以20M带宽进行关联的AP信息 */
    oal_uint8                         uc_ip_addr_obtained_num;
#ifdef _PRE_WLAN_FEATURE_AMPDU_VAP
    oal_uint8                         uc_rx_ba_session_num;                   /* 该vap下，rx BA会话的数目 */
    oal_uint8                         uc_tx_ba_session_num;                   /* 该vap下，tx BA会话的数目 */
    oal_uint8                         auc_resv9[2];
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_h2d_protection_stru          st_prot;
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
    frw_timeout_stru                 st_ps_sw_timer;                             /* 低功耗开关 */
    oal_uint32                       ul_check_timer_pause_cnt;                   /* 低功耗pause计数 */
#endif
#ifdef _PRE_WLAN_FEATURE_SAE
    oal_delayed_work st_sae_report_ext_auth_worker; /* SAE report external auth req工作队列 */
    oal_uint8     duplicate_auth_seq2_flag;
    oal_uint8     duplicate_auth_seq4_flag;
    oal_uint8     resv[2]; /* 2是resv的字节数 */
#endif
#ifdef _PRE_WLAN_FEATURE_FTM // FTM认证，用于gas init comeback定时
    frw_timeout_stru    st_ftm_timer;
    mac_gas_mgmt_stru   st_gas_mgmt;
#endif
    oal_bool_enum_uint8 tsf_info_query_completed_flag; /* 查询结束标志，OAL_TRUE，查询结束，OAL_FALSE，查询未结束 */
    sync_tsf_stru sync_tsf;
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_vap_stru *hmac_chba_vap_info;
    /* 标识关联用户的user id,组播用户不会置位 */
    uint32_t user_id_bitmap;
#endif
    oal_completion                  st_chan_meas_end_ready;
    oal_uint8                       chan_meas_scan_chan;
    oal_bool_enum_uint8             en_wait_chan_meas_end;
    mac_vap_stru                    st_vap_base_info;                           /* MAC vap，只能放在最后! */
}hmac_vap_stru;

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
extern hmac_vap_stru g_ast_hmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
extern mac_tx_large_amsdu_ampdu_stru *mac_get_tx_large_amsdu_addr(oal_void);
#endif
extern oal_uint32  hmac_vap_destroy(hmac_vap_stru *pst_vap);
extern oal_uint32  hmac_vap_init(
                       hmac_vap_stru              *pst_hmac_vap,
                       oal_uint8                   uc_chip_id,
                       oal_uint8                   uc_device_id,
                       oal_uint8                   uc_vap_id,
                       mac_cfg_add_vap_param_stru *pst_param);

extern oal_uint32  hmac_vap_creat_netdev(
    hmac_vap_stru *pst_hmac_vap, oal_int8 *puc_netdev_name,
    oal_uint32 ul_netdev_name_len, oal_int8 *puc_mac_addr);
extern oal_uint16 hmac_vap_check_ht_capabilities_ap(
            hmac_vap_stru                  *pst_hmac_vap,
            oal_uint8                      *puc_payload,
            oal_uint16                      us_info_elem_offset,
            oal_uint32                      ul_msg_len,
            hmac_user_stru                 *pst_hmac_user_sta);
extern  oal_uint32  hmac_search_ht_cap_ie_ap(
                hmac_vap_stru               *pst_hmac_vap,
                hmac_user_stru              *pst_hmac_user_sta,
                oal_uint8                   *puc_payload,
                oal_uint16                   us_index,
                oal_bool_enum                en_prev_asoc_ht);
extern oal_uint16 hmac_vap_check_vht_capabilities_ap(
                hmac_vap_stru                   *pst_hmac_vap,
                oal_uint8                       *puc_payload,
                oal_uint16                       us_info_elem_offset,
                oal_uint32                       ul_msg_len,
                hmac_user_stru                  *pst_hmac_user_sta);
extern oal_bool_enum_uint8 hmac_vap_addba_check(
                hmac_vap_stru      *pst_hmac_vap,
                hmac_user_stru     *pst_hmac_user,
                oal_uint8           uc_tidno);

extern oal_void hmac_vap_net_stopall(oal_void);
extern oal_void hmac_vap_net_startall(oal_void);

extern oal_bool_enum_uint8 hmac_flowctl_check_device_is_sta_mode(oal_void);
extern oal_void hmac_vap_net_start_subqueue(oal_uint16 us_queue_idx);
extern oal_void hmac_vap_net_stop_subqueue(oal_uint16 us_queue_idx);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_uint32 hmac_check_opmode_notify(
                hmac_vap_stru                   *pst_hmac_vap,
                oal_uint8                       *puc_mac_hdr,
                oal_uint8                       *puc_payload,
                oal_uint16                       us_info_elem_offset,
                oal_uint32                       ul_msg_len,
                hmac_user_stru                  *pst_hmac_user);
#endif
extern oal_void hmac_handle_disconnect_rsp(
    hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    mac_reason_code_enum_uint16  en_disasoc_reason);
extern oal_uint8 *hmac_vap_get_pmksa(hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_bssid);
oal_uint32 hmac_tx_get_mac_vap(oal_uint8 uc_vap_id, mac_vap_stru **pst_vap_stru);
#ifdef _PRE_WLAN_FEATURE_ROAM
extern oal_uint32 hmac_vap_check_signal_bridge(mac_vap_stru *pst_mac_vap);
#endif
extern oal_void hmac_vap_save_rx_rate(hmac_vap_stru *pst_hmac_vap);
extern oal_void hmac_vap_clean_rx_rate(hmac_vap_stru *pst_hmac_vap);
extern oal_bool_enum_uint8 hmac_vap_is_connecting(mac_vap_stru *mac_vap);
#ifdef _PRE_WLAN_CHBA_MGMT
oal_bool_enum mac_is_chba_mode(const mac_vap_stru *mac_vap);
#endif
oal_bool_enum hmac_vap_is_up(mac_vap_stru *mac_vap);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_vap.h */
