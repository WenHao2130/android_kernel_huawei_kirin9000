

#ifndef __OAL_LINUX_CFG80211_H__
#define __OAL_LINUX_CFG80211_H__

/* 其他头文件包含 */
#include <net/genetlink.h>
#include <net/cfg80211.h>
#include <linux/nl80211.h>

/* 宏定义 */
/* hostapd和supplicant事件上报需要用到宏 */
#define OAL_NLMSG_GOODSIZE            NLMSG_GOODSIZE
#define OAL_ETH_ALEN_SIZE             ETH_ALEN
#define OAL_NLMSG_DEFAULT_SIZE        NLMSG_DEFAULT_SIZE
#define OAL_IEEE80211_MIN_ACTION_SIZE IEEE80211_MIN_ACTION_SIZE

#define oal_nla_put_u32_m(skb, attrtype, value)     NLA_PUT_U32(skb, attrtype, value)
#define oal_nla_put_m(skb, attrtype, attrlen, data) NLA_PUT(skb, attrtype, attrlen, data)
#define oal_nla_put_u16_m(skb, attrtype, value)     NLA_PUT_U16(skb, attrtype, value)
#define oal_nla_put_u8(skb, attrtype, value)      NLA_PUT_U8(skb, attrtype, value)
#define oal_nla_put_flag(skb, attrtype)           NLA_PUT_FLAG(skb, attrtype)
#define OUI_VENDOR 0x001A11
#define WIFI_SUBCMD_CHBA_READY_TO_CONN 0x1901
#define EVENT_BUF_SIZE 1024
#define CHBA_EVENT_ID 2
enum HID2D_READY2CONN_ATTR {
    HID2D_READY2CONN_ATTR_ID = 0,
    HID2D_READY2CONN_ATTR_STATUS_CODE,

    /* keep last */
    HID2D_READY2CONN_ATTR_AFTER_LAST,
    HID2D_READY2CONN_ATTR_MAX = HID2D_READY2CONN_ATTR_AFTER_LAST - 1
};
typedef enum rate_info_flags oal_rate_info_flags;
typedef void (*p_hid2d_cb)(void *net_dev, uint8_t *data);
/* 函数声明 */
/*
 * struct cfg80211_external_auth_params - Trigger External authentication.
 *
 * Commonly used across the external auth request and event interfaces.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
/* do nothing */
#else
struct cfg80211_external_auth_params {
    oal_nl80211_external_auth_action action;
    oal_uint8 bssid[OAL_ETH_ALEN_SIZE];
    oal_cfg80211_ssid_stru ssid;
    unsigned int key_mgmt_suite;
    oal_uint16 status;
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) */
typedef struct cfg80211_external_auth_params oal_cfg80211_external_auth_stru;

extern void cfg80211_drv_mss_result(struct net_device *dev, gfp_t gfp, const u8 *buf, size_t len);
extern void cfg80211_drv_tas_result(struct net_device *dev, gfp_t gfp, const u8 *buf, size_t len);
extern void cfg80211_drv_hid2d_report(const u8 *buf, size_t len);
extern void cfg80211_drv_hid2d_cb_register(p_hid2d_cb cb, void *net_dev);
extern void cfg80211_drv_hid2d_acs_report(struct net_device *dev, gfp_t gfp, const u8 *buf, size_t len);
#endif /* end of oal_cfg80211.h */
