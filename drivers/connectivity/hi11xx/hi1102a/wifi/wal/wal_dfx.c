

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wlan_types.h"
#include "wal_dfx.h"
#include "oal_net.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "plat_pm_wlan.h"
#include "dmac_ext_if.h"
#include "hmac_ext_if.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_p2p.h"
#include "wal_ext_if.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_DFX_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFR

#define DFR_WAIT_PLAT_FINISH_TIME (25000) /* 等待平台完成dfr工作的等待时间 */

oal_int8 *g_dfr_error_type[] = {
    "AP",
    "STA",
    "P2P0",
    "GO",
    "CLIENT",
    "DFR UNKOWN ERR TYPE!!"
};

/* 此枚举为g_auc_dfr_error_type字符串的indx集合 */
typedef enum {
    DFR_ERR_TYPE_AP = 0,
    DFR_ERR_TYPE_STA,
    DFR_ERR_TYPE_P2P,
    DFR_ERR_TYPE_GO,
    DFR_ERR_TYPE_CLIENT,

    DFR_ERR_TYPE_BUTT
} wal_dfr_error_type;
typedef oal_uint8 wal_dfr_error_type_enum_uint8;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

#else
struct st_exception_info {
    /* wifi异常触发 */
    oal_work_stru wifi_excp_worker;
    oal_workqueue_stru *wifi_exception_workqueue;
    oal_work_stru wifi_excp_recovery_worker;
    oal_uint32 wifi_excp_type;
} g_st_exception_info;
struct st_exception_info *g_pst_exception_info = &g_st_exception_info;

struct st_wifi_dfr_callback {
    void (*wifi_recovery_complete)(void);
    void (*notify_wifi_to_recovery)(void);
    bool (*wifi_finish_recovery)(void);
};
#endif

struct st_wifi_dfr_callback *g_st_wifi_callback;

oal_void wal_dfr_init_param(oal_void);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_mutex_stru g_st_dfr_mutex;
#endif
#endif  // _PRE_WLAN_FEATURE_DFR
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFR

OAL_STATIC oal_void wal_dfr_kick_all_user(hmac_vap_stru *pst_hmac_vap)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;
    oal_int32 l_ret;
    mac_cfg_kick_user_param_stru *pst_kick_user_param;

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_KICK_USER, OAL_SIZEOF(mac_cfg_kick_user_param_stru));

    /* 设置配置命令参数 */
    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, BROADCAST_MACADDR);

    /* 填写去关联reason code */
    pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;
    if (pst_hmac_vap->pst_net_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::pst_net_device is null!}");
        return;
    }

    l_ret = wal_send_cfg_event(pst_hmac_vap->pst_net_device,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_kick_user_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::return err code [%d]!}\r\n", l_ret);
        return;
    }

    /* 处理返回消息 */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_dfr_kick_all_user::hmac start vap fail,err code[%u]!}\r\n", ul_err_code);
        return;
    }

    return;
}


oal_uint32 wal_process_p2p_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_dev = OAL_PTR_NULL;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                     "{Now begin P2P exception recovery,del user num[%d] when P2P state is P2P0[%d]/CL[%d]/GO[%d].}",
                     pst_mac_vap->us_user_nums,
                     IS_P2P_DEV(pst_mac_vap),
                     IS_P2P_CL(pst_mac_vap),
                     IS_P2P_GO(pst_mac_vap));

    /* 删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);

    /* AP模式还是STA模式 */
    if (IS_AP(pst_mac_vap)) {
        /* vap信息初始化 */
    } else if (IS_STA(pst_mac_vap)) {
        pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (pst_hmac_dev == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                           "{hmac_process_p2p_excp::pst_hmac_device is null, dev_id[%d].}",
                           pst_mac_vap->uc_device_id);

            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }
        /* 删除扫描信息列表，停止扫描 */
        if (pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
            pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.p_fn_cb = OAL_PTR_NULL;
            pst_hmac_dev->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        }
    }

    return OAL_SUCC;
}

oal_uint32 wal_process_ap_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                     "{hmac_process_sta_excp::Now begin AP exception recovery program, when AP have [%d] USERs.}",
                     pst_mac_vap->us_user_nums);

    /* 删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);
    return OAL_SUCC;
}

oal_uint32 wal_process_sta_excp(hmac_vap_stru *pst_hmac_vap)
{
    mac_vap_stru *pst_mac_vap;
    hmac_device_stru *pst_hmac_dev;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_hmac_dev = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                       "{hmac_process_sta_excp::pst_hmac_device is null, dev_id[%d].}",
                       pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DFR,
                     "{hmac_process_sta_excp::Now begin sta exception recovery program, when sta have [%d] users.}",
                     pst_mac_vap->us_user_nums);

    /* 关联状态下上报关联失败，删除用户 */
    wal_dfr_kick_all_user(pst_hmac_vap);

    /* 删除扫描信息列表，停止扫描 */
    if (pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == pst_mac_vap->uc_vap_id) {
        pst_hmac_dev->st_scan_mgmt.st_scan_record_mgmt.p_fn_cb = OAL_PTR_NULL;
        pst_hmac_dev->st_scan_mgmt.en_is_scanning = OAL_FALSE;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_dfr_destroy_vap(oal_net_device_stru *pst_netdev)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = OAL_PTR_NULL;
    oal_uint32 ul_err_code;

    oal_int32 l_ret;

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DESTROY_VAP, OAL_SIZEOF(oal_int32));
    l_ret = wal_send_cfg_event(pst_netdev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        OAL_IO_PRINT("DFR DESTROY_VAP[name:%.16s] fail, return[%d]!", pst_netdev->name, l_ret);
        oam_warning_log2(0, OAM_SF_DFR, "{wal_dfr_excp_process::DESTROY_VAP return err code [%d], iftype[%d]!}\r\n",
                         l_ret,
                         pst_netdev->ieee80211_ptr->iftype);

        return l_ret;
    }

    /* 读取返回的错误码 */
    ul_err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ul_err_code != OAL_SUCC) {
        OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_process::hmac add vap fail, err code[%u]!}\r\n", ul_err_code);
        return l_ret;
    }

    oal_net_dev_priv(pst_netdev) = OAL_PTR_NULL;

    return OAL_SUCC;
}


OAL_STATIC oal_void wal_dfr_recovery_dev_timer(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    /* WIFI DFR成功后调用hmac_wifi_state_notify重启device侧pps统计定时器 */
    if ((pst_wlan_pm == OAL_PTR_NULL) ||
        (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_recovery_dev_timer:: Reinit device pps timer failed!}");
        return;
    }

    pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_TRUE);
}

OAL_STATIC oal_void wal_dfr_recovery_env(void)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_net_device_stru *pst_netdev;
    wal_dfr_error_type_enum_uint8 en_err_type = DFR_ERR_TYPE_BUTT;
    oal_uint32 ul_timeleft;
    oal_wireless_dev_stru *pst_wireless_dev;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_RECV_LASTWORD);
    if (g_st_dfr_info.bit_ready_to_recovery_flag != OAL_TRUE) {
        return;
    }

    ul_timeleft = oal_wait_for_completion_timeout(&g_st_dfr_info.st_plat_process_comp,
                                                  oal_msecs_to_jiffies(DFR_WAIT_PLAT_FINISH_TIME));
    if (!ul_timeleft) {
        OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_dfr_excp_process:wait dev reset timeout[%d]", DFR_WAIT_PLAT_FINISH_TIME);
    }

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_dfr_recovery_env: get plat_process_comp signal after[%u]ms!",
                     (oal_uint32)(DFR_WAIT_PLAT_FINISH_TIME - ul_timeleft));

    /* 恢复vap, 上报异常给上层 */
    for (; g_st_dfr_info.ul_netdev_num > 0; g_st_dfr_info.ul_netdev_num--) {
        ul_ret = OAL_SUCC;
        pst_netdev = (oal_net_device_stru *)g_st_dfr_info.past_netdev[g_st_dfr_info.ul_netdev_num - 1];

        if (pst_netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            wal_priv_init_config();
            wlan_dfr_custom_cali();
            hwifi_config_init_force();
#endif
            l_ret = wal_cfg_vap_h2d_event(pst_netdev);
            if (l_ret != OAL_SUCC) {
                OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_dfr_recovery_env:DFR Device cfg_vap creat false[%d]!", l_ret);
                wal_dfr_init_param();
                return;
            }

            /* host device_stru初始化 */
            l_ret = wal_host_dev_init(pst_netdev);
            if (l_ret != OAL_SUCC) {
                OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_dfr_recovery_env::DFR wal_host_dev_init FAIL %d ", l_ret);
            }

            ul_ret = (oal_uint32)wal_setup_ap(pst_netdev);
            oal_net_device_open(pst_netdev);
            en_err_type = DFR_ERR_TYPE_AP;
        } else if ((pst_netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) ||
                   (pst_netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)) {
            l_ret = wal_netdev_open(pst_netdev);
            en_err_type = (!OAL_STRCMP(pst_netdev->name, wal_get_p2p_name())) ? DFR_ERR_TYPE_P2P : DFR_ERR_TYPE_STA;
        } else {
            pst_wireless_dev = oal_netdevice_wdev(pst_netdev);

            /* 去注册netdev */
            oal_net_unregister_netdev(pst_netdev);
            oal_mem_free_m(pst_wireless_dev, OAL_TRUE);

            continue;
        }

        if ((oal_unlikely(l_ret != OAL_SUCC)) || (oal_unlikely(ul_ret != OAL_SUCC))) {
            OAL_IO_PRINT("DFR BOOT_VAP[name:%.16s]fail!error_code[%d]", pst_netdev->name, ((oal_uint8)l_ret | ul_ret));
            oam_warning_log2(0, OAM_SF_ANY, "{wal_dfr_excep_process:: Boot vap Failure,vap_iftype[%d],error_code[%d]!}",
                             pst_netdev->ieee80211_ptr->iftype,
                             ((oal_uint8)l_ret | ul_ret));
            continue;
        }

        /* 上报异常 */
        oal_cfg80211_rx_exception(pst_netdev,
                                  (oal_uint8 *)g_dfr_error_type[en_err_type],
                                  OAL_STRLEN(g_dfr_error_type[en_err_type]));
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_enable();
    wal_dfr_recovery_dev_timer();
#endif

    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_FALSE;

#endif

    return;
}


oal_void wal_dfr_excp_process(mac_device_stru *pst_mac_device, oal_uint32 ul_exception_type)
{
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_int32 l_ret;

    oal_net_device_stru *pst_netdev = OAL_PTR_NULL;
    oal_net_device_stru *pst_p2p0_netdev = OAL_PTR_NULL;

    if (oal_unlikely(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{wal_dfr_excp_process::pst_mac_device is null!}\r\n");
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

    wlan_pm_disable_check_wakeup(OAL_FALSE);
#endif
    memset_s((oal_uint8 *)(g_st_dfr_info.past_netdev),
             OAL_SIZEOF(g_st_dfr_info.past_netdev[0]) * (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT + 1),
             0, OAL_SIZEOF(g_st_dfr_info.past_netdev[0]) * (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT + 1));
    oal_mutex_lock(&g_st_dfr_mutex);

    for (uc_vap_idx = pst_mac_device->uc_vap_num, g_st_dfr_info.ul_netdev_num = 0; uc_vap_idx > 0; uc_vap_idx--) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx - 1]);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_process::mac_res_get_hmac_vap fail!vap_idx = %u}\r",
                             pst_mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        }
        pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
        pst_netdev = pst_hmac_vap->pst_net_device;

#ifdef _PRE_WLAN_FEATURE_P2P
        if (IS_P2P_DEV(pst_mac_vap)) {
            pst_netdev = pst_hmac_vap->pst_p2p0_net_device;
        } else if (IS_P2P_CL(pst_mac_vap)) {
            pst_p2p0_netdev = pst_hmac_vap->pst_p2p0_net_device;
            if (pst_p2p0_netdev != OAL_PTR_NULL) {
                g_st_dfr_info.past_netdev[g_st_dfr_info.ul_netdev_num] = (oal_uint32 *)pst_p2p0_netdev;
                g_st_dfr_info.ul_netdev_num++;
            }
        }
#endif
        if (pst_netdev == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_process::pst_netdev NULL pointer !vap_idx = %u}\r",
                             pst_mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        } else if (pst_netdev->ieee80211_ptr == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_process::ieee80211_ptr NULL pointer !vap_idx = %u}\r",
                             pst_mac_device->auc_vap_id[uc_vap_idx - 1]);
            continue;
        }

        g_st_dfr_info.past_netdev[g_st_dfr_info.ul_netdev_num] = (oal_uint32 *)pst_netdev;
        g_st_dfr_info.ul_netdev_num++;

        oam_warning_log4(0, OAM_SF_DFR, "wal_dfr_excp_process:vap_iftype [%d],vap_id[%d], vap_idx[%d],vap_mode_idx[%d]",
                         pst_netdev->ieee80211_ptr->iftype,
                         pst_mac_vap->uc_vap_id,
                         uc_vap_idx,
                         g_st_dfr_info.ul_netdev_num);

        wal_force_scan_complete(pst_netdev, OAL_TRUE);
        wal_stop_sched_scan(pst_netdev);
        oal_net_device_close(pst_netdev);
        l_ret = wal_dfr_destroy_vap(pst_netdev);
        if (oal_unlikely(l_ret != OAL_SUCC)) {
            g_st_dfr_info.ul_netdev_num--;
            continue;
        }

        if (pst_p2p0_netdev != OAL_PTR_NULL) {
            l_ret = wal_dfr_destroy_vap(pst_p2p0_netdev);
            if (oal_unlikely(l_ret != OAL_SUCC)) {
                oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_excp_process::DESTROY_P2P0 return err code [%d]!}\r\n");
                oal_net_unregister_netdev(pst_netdev);
                g_st_dfr_info.ul_netdev_num--;
                continue;
            }
            pst_p2p0_netdev = OAL_PTR_NULL;
        }
    }
    oal_mutex_unlock(&g_st_dfr_mutex);

    /* device close& open */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    plat_exception_handler(0, 0, ul_exception_type);
#endif

    // 开始dfr恢复动作: wal_dfr_recovery_env();
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_TRUE;
    oal_queue_work(g_exception_info->wifi_exception_workqueue, &g_exception_info->wifi_excp_recovery_worker);

    return;
}

oal_void wal_dfr_signal_complete(oal_void)
{
    oal_complete(&g_st_dfr_info.st_plat_process_comp);
}


static bool wal_dfr_recovery_finish(oal_void)
{
    /* 异常复位开关是否开启 */
    if ((!g_st_dfr_info.bit_device_reset_enable) || g_st_dfr_info.bit_device_reset_process_flag) {
        return false;
    }

    return true;
}



oal_void wal_dfr_excp_rx(oal_uint8 uc_device_id, oal_uint32 ul_exception_type)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8 uc_vap_idx;
    hmac_vap_stru *pst_hmac_vap;
    mac_vap_stru *pst_mac_vap;
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(uc_device_id);
    if (pst_mac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_dfr_excp_rx:ERROR dev_ID[%d] in DFR process!", uc_device_id);
        return;
    }

    /* 异常复位开关是否开启 */
    if ((!g_st_dfr_info.bit_device_reset_enable) || g_st_dfr_info.bit_device_reset_process_flag) {
        return;
    }

    /* log现在进入异常处理流程 */
    OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_rx:: Enter the exception processing, type[%d].}",
                     ul_exception_type);

    g_st_dfr_info.bit_device_reset_process_flag = OAL_TRUE;
    g_st_dfr_info.bit_user_disconnect_flag = OAL_TRUE;

    /* 按照每个vap模式进行异常处理 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_dev->uc_vap_num; uc_vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_dev->auc_vap_id[uc_vap_idx]);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_DFR, "{wal_dfr_excp_rx::mac_res_get_hmac_vap fail!vap_idx = %u}\r",
                             pst_mac_dev->auc_vap_id[uc_vap_idx]);
            continue;
        }

        pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
        if (!IS_LEGACY_VAP(pst_mac_vap)) {
            wal_process_p2p_excp(pst_hmac_vap);
        } else if (IS_AP(pst_mac_vap)) {
            wal_process_ap_excp(pst_hmac_vap);
        } else if (IS_STA(pst_mac_vap)) {
            wal_process_sta_excp(pst_hmac_vap);
        } else {
            continue;
        }
    }
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WIFI_RECOVERY);
    wal_dfr_excp_process(pst_mac_dev, ul_exception_type);
#else
    return;
#endif
}

oal_uint32 wal_dfr_get_excp_type(oal_void)
{
    return g_exception_info->wifi_excp_type;
}


void wal_dfr_excp_work(oal_work_stru *work)
{
    oal_uint32 ul_exception_type;
    oal_uint8 uc_device_id;

    ul_exception_type = wal_dfr_get_excp_type();

    /* 暂不支持多chip，多device */
    if ((1 != WLAN_CHIP_DBSC_DEVICE_NUM) || (1 != WLAN_CHIP_MAX_NUM_PER_BOARD)) {
        oam_error_log2(0, OAM_SF_DFR, "DFR Can not support muti_chip[%d] or muti_device[%d].\n",
                       WLAN_CHIP_MAX_NUM_PER_BOARD, WLAN_CHIP_DBSC_DEVICE_NUM);
        return;
    }
    uc_device_id = 0;

    wal_dfr_excp_rx(uc_device_id, ul_exception_type);
}
void wal_dfr_bfgx_excp(void)
{
    wal_dfr_excp_work(NULL);
}
void wal_dfr_recovery_work(oal_work_stru *work)
{
    wal_dfr_recovery_env();
}

oal_void wal_dfr_init_param(oal_void)
{
    memset_s((oal_uint8 *)&g_st_dfr_info, OAL_SIZEOF(dfr_info_stru), 0, OAL_SIZEOF(dfr_info_stru));

    g_st_dfr_info.bit_device_reset_enable = OAL_TRUE;
    g_st_dfr_info.bit_hw_reset_enable = OAL_FALSE;
    g_st_dfr_info.bit_soft_watchdog_enable = OAL_FALSE;
    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;
    g_st_dfr_info.bit_ready_to_recovery_flag = OAL_FALSE;
    g_st_dfr_info.bit_user_disconnect_flag = OAL_FALSE;
    g_st_dfr_info.ul_excp_type = 0xffffffff;

    oal_init_completion(&g_st_dfr_info.st_plat_process_comp);
}


OAL_STATIC oal_uint32 wal_dfr_excp_init_handler(oal_void)
{
    hmac_device_stru *pst_hmac_dev;
    struct st_wifi_dfr_callback *pst_wifi_callback = OAL_PTR_NULL;

    pst_hmac_dev = hmac_res_get_mac_dev(0);
    if (pst_hmac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_init_dev_excp_handler:ERROR hmac dev_ID[%d] in DFR process!", 0);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化dfr开关 */
    wal_dfr_init_param();
    /* 初始化dfr的互斥锁 */
    oal_mutex_init(&g_st_dfr_mutex);
    /* 挂接调用钩子 */
    if (g_exception_info != OAL_PTR_NULL) {
        oal_init_work(&g_exception_info->wifi_excp_worker, wal_dfr_excp_work);
        oal_init_work(&g_exception_info->wifi_excp_recovery_worker, wal_dfr_recovery_work);
        g_exception_info->wifi_exception_workqueue = oal_create_singlethread_workqueue_m("wifi_exception_queue");
        if (g_exception_info->wifi_exception_workqueue == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_DFR,
                             "DFR wal_dfr_excp_init_handler: create work queue \"wifi_exception_queue\" fail.\n");
            return OAL_ERR_CODE_PTR_NULL;
        } else {
            OAM_WARNING_LOG1(0, OAM_SF_DFR,
                             "DFR wal_dfr_excp_init_handler: create work queue \"wifi_exception_queue\" succ. %p\n",
                             (uintptr_t)(g_exception_info->wifi_exception_workqueue));
        }
    }

    pst_wifi_callback = (struct st_wifi_dfr_callback *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
                                                                     OAL_SIZEOF(struct st_wifi_dfr_callback), OAL_TRUE);
    if (pst_wifi_callback == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_DFR, "DFR wal_init_dev_excp_handler:can not alloc mem,size[%d]!",
                       OAL_SIZEOF(struct st_wifi_dfr_callback));
        g_st_wifi_callback = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_st_wifi_callback = pst_wifi_callback;
    pst_wifi_callback->wifi_recovery_complete = wal_dfr_signal_complete;
    pst_wifi_callback->notify_wifi_to_recovery = wal_dfr_bfgx_excp;
    pst_wifi_callback->wifi_finish_recovery = wal_dfr_recovery_finish;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    plat_wifi_exception_rst_register(pst_wifi_callback);
#endif

    oam_warning_log0(0, OAM_SF_DFR, "DFR wal_init_dev_excp_handler init ok.\n");
    return OAL_SUCC;
}


OAL_STATIC oal_void wal_dfr_excp_exit_handler(oal_void)
{
    hmac_device_stru *pst_hmac_dev;

    pst_hmac_dev = hmac_res_get_mac_dev(0);
    if (pst_hmac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_DFR, "wal_dfr_excp_exit_handler:ERROR dev_ID[%d] in DFR process!", 0);
        return;
    }

    if (g_exception_info != OAL_PTR_NULL) {
        oal_cancel_work_sync(&g_exception_info->wifi_excp_worker);
        oal_cancel_work_sync(&g_exception_info->wifi_excp_recovery_worker);
        if (g_exception_info->wifi_exception_workqueue) {
            oal_destroy_workqueue(g_exception_info->wifi_exception_workqueue);
            g_exception_info->wifi_exception_workqueue = OAL_PTR_NULL;
        }
    }
    oal_mem_free_m(g_st_wifi_callback, OAL_TRUE);
    g_st_wifi_callback = OAL_PTR_NULL;

    oam_warning_log0(0, OAM_SF_DFR, "wal_dfr_excp_exit_handler::DFR dev_excp_handler remove ok.");
}
#else
oal_void wal_dfr_excp_rx(oal_uint8 uc_device_id, oal_uint32 ul_exception_type)
{
}

#endif  // _PRE_WLAN_FEATURE_DFR

oal_uint32 wal_dfx_init(oal_void)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    oal_uint32 l_ret;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

    l_ret = (oal_uint32)init_dev_excp_handler();
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }
#endif
    l_ret = wal_dfr_excp_init_handler();
    return l_ret;
#else
    return OAL_SUCC;
#endif
}

oal_void wal_dfx_exit(oal_void)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfr_excp_exit_handler();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

    deinit_dev_excp_handler();
#endif
#endif  // _PRE_WLAN_FEATURE_DFR
#endif
}

