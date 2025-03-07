

#ifndef __WLAN_SPEC_1102a_H__
#define __WLAN_SPEC_1102a_H__

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST))

/*****************************************************************************
  其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#include "wlan_oneimage_define.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  0.0 定制化变量声明
*****************************************************************************/
/* 定制化待设计为一个结构体，并对外提供内联函数访问形态，而不是预编译访问形态 */
/* 关联用户的最大个数 */
extern oal_uint16   g_wlan_assoc_user_max_num;
extern oal_uint16   g_us_wlan_assoc_user_max_num;
extern oal_uint32   g_ul_wlan_vap_max_num_per_device;

/*****************************************************************************
  0.1.2 热点入网功能
*****************************************************************************/
/* 字节偏移定义 */
#define BYTE_OFFSET_0  0
#define BYTE_OFFSET_1  1
#define BYTE_OFFSET_2  2
#define BYTE_OFFSET_3  3
#define BYTE_OFFSET_4  4
#define BYTE_OFFSET_5  5
#define BYTE_OFFSET_6  6
#define BYTE_OFFSET_7  7
#define BYTE_OFFSET_8  8
#define BYTE_OFFSET_9  9
#define BYTE_OFFSET_10 10
#define BYTE_OFFSET_11 11
#define BYTE_OFFSET_12 12
#define BYTE_OFFSET_13 13
#define BYTE_OFFSET_14 14
#define BYTE_OFFSET_15 15
#define BYTE_OFFSET_16 16
#define BYTE_OFFSET_17 17
#define BYTE_OFFSET_18 18
#define BYTE_OFFSET_19 19
#define BYTE_OFFSET_20 20
#define BYTE_OFFSET_21 21
#define BYTE_OFFSET_22 22
#define BYTE_OFFSET_23 23
#define BYTE_OFFSET_24 24
#define BYTE_OFFSET_25 25
#define BYTE_OFFSET_26 26
#define BYTE_OFFSET_27 27
#define BYTE_OFFSET_28 28
#define BYTE_OFFSET_29 29
#define BYTE_OFFSET_30 30
#define BYTE_OFFSET_31 31
/* 关联用户的最大个数 启动Proxy STA时，ap最大关联用户数为15，关闭时为32 */
#define WLAN_ASSOC_USER_MAX_NUM_SPEC        g_wlan_assoc_user_max_num

/* 作为P2P GO 允许关联最大用户数 */
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC 4
/*****************************************************************************
  0.5.3 AMSDU功能
*****************************************************************************/
/* 一个amsdu下允许拥有的msdu的最大个数 */
#define WLAN_AMSDU_MAX_NUM                  4

/*****************************************************************************
  0.8.2 STA AP规格
*****************************************************************************/
/* 每个device支持vap的最大个数=最大业务VAP数目+配置VAP数量 */
/* 软件规格:P2P_dev/CL以STA模式存在，P2P_GO以AP模式存在
    1)AP 模式:  2个ap + 1个配置vap
    2)STA 模式: 3个sta + 1个配置vap
    3)STA+P2P共存模式:  1个sta + 1个P2P_dev + 1个P2P_GO/Client + 1个配置vap
    4)STA+Proxy STA共存模式:  1个sta + 1个proxy STA + 1个配置vap
*/
#define WLAN_VAP_MAX_NUM_PER_DEVICE_SPEC        g_ul_wlan_vap_max_num_per_device    /* VAP个数限制 */

/* 整个BOARD支持的最大的VAP数目 */
#define WLAN_VAP_SUPPOTR_MAX_NUM_SPEC       (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP * WLAN_VAP_MAX_NUM_PER_DEVICE_SPEC)   /* 5个或者18个 */

/* 整个BOARD支持的最大业务VAP的数目 */
#define WLAN_SERVICE_VAP_SUPPOTR_MAX_NUM_SPEC     (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP * (WLAN_VAP_MAX_NUM_PER_DEVICE_SPEC - 1))

#define WLAN_MAX_MULTI_USER_NUM_SPEC              (WLAN_SERVICE_VAP_SUPPOTR_MAX_NUM_SPEC)
/*****************************************************************************
  1.0 WLAN芯片对应的spec
*****************************************************************************/
/* 每个board支持chip的最大个数放入平台 */
/* 每个chip支持device的最大个数放入平台 */
/* 最多支持的MAC硬件设备个数放入平台 */
/*****************************************************************************
  1.3 oam相关的spec
*****************************************************************************/
/* oam 放入平台 */
/*****************************************************************************
  1.4 mem对应的spec
*****************************************************************************/
/*****************************************************************************
  1.4.1 内存池规格
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.2 共享描述符内存池配置信息
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.3 共享管理帧内存池配置信息
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.4 共享数据帧内存池配置信息
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.5 本地内存池配置信息
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.6 事件结构体内存池
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.7 用户内存池
*****************************************************************************/
/*****************************************************************************
  1.4.8 MIB内存池  TBD :最终个子池的空间大小及个数需要重新考虑
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.9 netbuf内存池  TBD :最终个子池的空间大小及个数需要重新考虑
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
  1.4.9.1 sdt netbuf内存池
*****************************************************************************/
/* 内存 spec 放入平台 */
/*****************************************************************************
 1.5 frw相关的spec
*****************************************************************************/
/* 事件调度 spec 放入平台 */
/*****************************************************************************
  2 宏定义，分类和DR保持一致
*****************************************************************************/
/*****************************************************************************
  2.1 基础协议/定义物理层协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.1.1 扫描侧STA 功能
*****************************************************************************/
/* 一次可以扫描的最大BSS个数，两个规格可以合并 */
#define WLAN_SCAN_REQ_MAX_BSSID                 2
#define WLAN_SCAN_REQ_MAX_SSID                  8

/* 扫描延迟,us 未使用，可删除 */
#define WLAN_PROBE_DELAY_TIME                   10

/* 扫描时，最小的信道驻留时间，单位ms，变量命名有误 */
#define WLAN_DEFAULT_SCAN_MIN_TIME              110
/* 扫描时，最大的信道驻留时间，单位ms，变量命名有误 */
#define WLAN_DEFAULT_SCAN_MAX_TIME              500

/* 一个device所记录的扫描到的最大BSS个数。过少，需要扩大到200 */
#define WLAN_MAX_SCAN_BSS_NUM                   32
/* 一个信道下记录扫描到的最大BSS个数 。过少，需要扩大到200 */
#define WLAN_MAX_SCAN_BSS_PER_CH                8
/* SSID最大长度, +1为\0预留空间 */
#define WLAN_SSID_MAX_LEN                   (32 + 1)

#define WLAN_DEFAULT_FG_SCAN_COUNT_PER_CHANNEL         2       /* 前景扫描每信道扫描次数 */
#define WLAN_DEFAULT_BG_SCAN_COUNT_PER_CHANNEL         1       /* 背景扫描每信道扫描次数 */
#define WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL  1       /* 每次信道扫描发送probe req帧的次数 */

#define WLAN_DEFAULT_MAX_TIME_PER_SCAN                 (3 * 1500)  /* 扫描的默认的最大执行时间，超过此时间，做超时处理 */

/* 扫描时，主被动扫描定时时间，单位ms，变量命名有误 */
#define WLAN_DEFAULT_ACTIVE_SCAN_TIME           20
#define WLAN_DEFAULT_PASSIVE_SCAN_TIME          60
#define WLAN_DEFAULT_DFS_CHANNEL_SCAN_TIME      120
#define WLAN_LONG_ACTIVE_SCAN_TIME              40             /* 指定SSID扫描个数超过3个时,1次扫描超时时间为40ms */

/*****************************************************************************
  2.1.1 STA入网功能
*****************************************************************************/
/* STA可同时关联的最大AP个数 */
#define WLAN_ASSOC_AP_MAX_NUM               2

/* 入网延迟，单位ms。变量需要修订 */
#define WLAN_JOIN_START_TIMEOUT                 10000   /* 已经废弃不用了 */
#define WLAN_AUTH_TIMEOUT                       300     /* sta auth的timeout时间 */
#define WLAN_AUTH_AP_TIMEOUT                    500
#define WLAN_ASSOC_TIMEOUT                      600     /* sta assoc的timeout时间 */
/*****************************************************************************
  2.1.2 热点入网功能
*****************************************************************************/
/* 可配置的最大关联用户数 */
#define WLAN_MAX_ASSOC_USER_CFG             200
/* 关联用户的最大个数 */
#define WLAN_ASSOC_USER_MAX_NUM_LIMIT       8

/* 活跃用户的最大个数放入平台 */
/* 活跃用户索引位图长度 */
#define WLAN_ACTIVE_USER_IDX_BMAP_LEN       ((WLAN_ACTIVE_USER_MAX_NUM + 7)>> 3)
/* 关联用户索引位图长度 */
#define WLAN_ASSOC_USER_IDX_BMAP_LEN       ((WLAN_ASSOC_USER_MAX_NUM_SPEC + 7)>> 3)
/* 不可用的RA LUT IDX */
#define WLAN_INVALID_RA_LUT_IDX             WLAN_ACTIVE_USER_MAX_NUM
/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration.
 */
#define WLAN_AID_MAX                        2007
#define WLAN_AID_DEFAULT                    128

/* 活跃定时器触发周期 */
#define WLAN_USER_ACTIVE_TRIGGER_TIME           1000
/* 老化定时器触发周期 */
#define WLAN_USER_AGING_TRIGGER_TIME            5000
/* 单位ms */
#define WLAN_USER_ACTIVE_TO_INACTIVE_TIME       5000

/* 单位ms */
#define WLAN_AP_USER_AGING_TIME                 (300 * 1000)    /* AP 用户老化时间 300S */
#define WLAN_P2PGO_USER_AGING_TIME              (60 * 1000)     /* GO 用户老化时间 60S */

/* AP keepalive参数,单位ms */
#define WLAN_AP_KEEPALIVE_TRIGGER_TIME          (15 * 1000)       /* keepalive定时器触发周期 */
#define WLAN_AP_KEEPALIVE_INTERVAL              (WLAN_AP_KEEPALIVE_TRIGGER_TIME * 4)   /* ap发送keepalive null帧间隔 */
#define WLAN_GO_KEEPALIVE_INTERVAL              (25*1000) /* P2P GO发送keepalive null帧间隔  */

/* STA keepalive参数,单位ms */
#define WLAN_STA_KEEPALIVE_TIME                 (25*1000) /* wlan0发送keepalive null帧间隔,同1101 keepalive 25s */
#define WLAN_CL_KEEPALIVE_TIME                  (20*1000) /* P2P CL发送keepalive null帧间隔,避免CL被GO pvb唤醒,P2P cl 20s */

/*****************************************************************************
  2.1.3 STA断网功能
*****************************************************************************/
#define WLAN_LINKLOSS_OFFSET_11H                5   /* 切信道时的延迟 */
#define WLAN_LINKLOSS_MIN_THRESHOLD             10  /* linkloss门限最小最低值 */
#define WLAN_LINKLOSS_MAX_THRESHOLD             254 /* linkloss门限最大值 */
/* Beacon Interval参数 */
/* max beacon interval, ms */
#define WLAN_BEACON_INTVAL_MAX              3500
/* min beacon interval */
#define WLAN_BEACON_INTVAL_MIN              40
/* min beacon interval */
#define WLAN_BEACON_INTVAL_DEFAULT          100
/* AP IDLE状态下beacon interval值 */
#define WLAN_BEACON_INTVAL_IDLE             1000


/*****************************************************************************
  2.1.6 保护模式功能
*****************************************************************************/
/* RTS开启门限，实际可删除 */
#define WLAN_RTS_DEFAULT                    512
#define WLAN_RTS_MIN                        1
#define WLAN_RTS_MAX                        2346

/*****************************************************************************
  2.1.7 分片功能
*****************************************************************************/
/* Fragmentation limits */
/* default frag threshold */
#define WLAN_FRAG_THRESHOLD_DEFAULT         1544
/* min frag threshold */
#define WLAN_FRAG_THRESHOLD_MIN             200 /* 为了保证分片数小于16: (1472(下发最大长度)/16)+36(数据帧最大帧头) = 128  */
/* max frag threshold */
#define WLAN_FRAG_THRESHOLD_MAX             2346
/*****************************************************************************
  2.1.14 数据速率功能
*****************************************************************************/
/* 速率相关参数 */
/* 一种协议，一种频宽下的最大速率个数 */
#define WLAN_RATE_MAXSIZE                   35
/* 记录支持的速率 */
#define WLAN_SUPP_RATES                         8
/* 用于记录扫描到的ap支持的速率最大个数 */
#define WLAN_MAX_SUPP_RATES                     12
/* 每个用户支持的最大速率集个数 */
#define HAL_TX_RATE_MAX_NUM                4

/* HAL 描述符支持最大发送次数 */
#define HAL_TX_RATE_MAX_CNT                7

/*****************************************************************************
  2.1.16 TXBF功能
*****************************************************************************/
/* TXBF的beamformer的流个数 */
#define WLAN_TXBF_NSS_SPEC		2
#define WLAN_TXBF_SU_BFMER_DEFAULT_VALUE      0
#define WLAN_BFER_ACTIVED		OAL_FALSE
#define WLAN_TXSTBC_ACTIVED		OAL_FALSE
#define WLAN_MU_BFEE_ACTIVED	OAL_TRUE
#define WLAN_TXBF_MATRIX_BUFFER_SIZE            512    /* 每buffer unit最大为468byte = 16*2/2*234/8(80M),另外为snr值预留10byte,取个整数用512 */
/*****************************************************************************
  2.2 其他协议/定义MAC 层协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.2.8 国家码功能
*****************************************************************************/
/* 管制类最大个数 */
#define WLAN_MAX_RC_NUM                         20
/* 管制类位图字数 */
#define WLAN_RC_BMAP_WORDS                      2
/* wifi 5G 2.4G全部信道个数 */
#define MAX_CHANNEL_NUM_FREQ_2G 14 /* 2G频段最大的信道号 */
#define MAX_CHANNEL_NUM_FREQ_5G 29 /* 5G频段最大的信道号 */
#define WLAN_MAX_CHANNEL_NUM                    (MAX_CHANNEL_NUM_FREQ_5G + MAX_CHANNEL_NUM_FREQ_2G)
/*****************************************************************************
  2.2.9 WMM功能
*****************************************************************************/
/* EDCA参数 */
/* STA所用WLAN_EDCA_XXX参数同WLAN_QEDCA_XXX */
#define WLAN_QEDCA_TABLE_CWMIN_MIN           1
#define WLAN_QEDCA_TABLE_CWMIN_MAX           10
#define WLAN_QEDCA_TABLE_CWMAX_MIN           1
#define WLAN_QEDCA_TABLE_CWMAX_MAX           10
#define WLAN_QEDCA_TABLE_AIFSN_MIN           2
#define WLAN_QEDCA_TABLE_AIFSN_MAX           15
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN      1
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX      65535
#define WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX   500
/* TID个数放入平台SPEC */
/* 默认的数据类型业务的TID */
#define WLAN_TID_FOR_DATA                   0

/* 接收队列的个数 与HAL_RX_DSCR_QUEUE_ID_BUTT相等 */
#define HAL_RX_QUEUE_NUM                3
/* 发送队列的个数 */
#define HAL_TX_QUEUE_NUM                5
/* 存储硬件接收上报的描述符链表数目(ping pong使用) */
#define HAL_HW_RX_DSCR_LIST_NUM         2


/*****************************************************************************
  2.2.10 协议节能STA侧功能
*****************************************************************************/
/* PSM特性规格 */
/* default DTIM period */
#define WLAN_DTIM_DEFAULT                   3

/* DTIM Period参数 */
/* beacon interval的倍数 */
#define WLAN_DTIM_PERIOD_MAX                255
#define WLAN_DTIM_PERIOD_MIN                1
/*****************************************************************************
  2.2.11 协议节能AP侧功能
*****************************************************************************/
/*****************************************************************************
  2.3 校准类别的spec
*****************************************************************************/
#ifdef _PRE_WLAN_PHY_PLL_DIV
/* 支持手动设置分频系数的个数 */
#define WITP_RF_SUPP_NUMS                  4
#endif

/*****************************************************************************
  2.4 安全协议类别的spec
*****************************************************************************/
#define WLAN_NUM_TK             4
#define WLAN_NUM_IGTK           2
#define WLAN_MAX_IGTK_KEY_INDEX 5
#define WLAN_MAX_WEP_KEY_COUNT  4

/*****************************************************************************
  2.4.7 PMF STA功能
*****************************************************************************/
/* SA Query流程间隔时间,老化时间的三分之一 */
#define WLAN_SA_QUERY_RETRY_TIME                (WLAN_AP_USER_AGING_TIME / 3)
/* SA Query流程超时时间,小于老化时间 */
#define WLAN_SA_QUERY_MAXIMUM_TIME              (WLAN_SA_QUERY_RETRY_TIME * 3)

#define WLAN_SA_QUERY_RETRY_TIME_FIXED          201
#define WLAN_SA_QUERY_MAXIMUM_TIME_FIXED        1000
/*****************************************************************************
  2.4.9 WPA功能
*****************************************************************************/
/* 加密相关的宏定义 */
/* 硬件MAC 最多等待32us， 软件等待40us */
#define HAL_CE_LUT_UPDATE_TIMEOUT          4
/*****************************************************************************
  2.5 性能类别的spec
*****************************************************************************/
/*****************************************************************************
  2.5.1 块确认功能
*****************************************************************************/
#define WLAN_ADDBA_TIMEOUT                      500
#define WLAN_TX_PROT_TIMEOUT                    6000
/* RX BA LUT表格个数，待修订为2 */
/* 支持的接收ba窗的最大个数 */
#define WLAN_MAX_RX_BA                      8

/* 支持的发送ba窗的最大个数 */
#define WLAN_MAX_TX_BA                      8

/*****************************************************************************
  2.5.2 AMPDU功能
*****************************************************************************/
#define WLAN_AMPDU_RX_BUFFER_SIZE       64  /* AMPDU接收端接收缓冲区的buffer size的大小 */
#define WLAN_AMPDU_RX_BA_LUT_WSIZE      64  /* AMPDU接收端用于填写BA RX LUT表的win size,
                                               要求大于等于WLAN_AMPDU_RX_BUFFER_SIZE */

#define WLAN_AMPDU_TX_MAX_NUM           32  /* AMPDU发送端最大聚合子MPDU个数 */
#define WLAN_AMPDU_TX_MAX_BUF_SIZE      64  /* 发送端的buffer size */

#define WLAN_AMPDU_TX_SCHD_STRATEGY     2   /* 02最大聚合设置为窗口大小的一半 */

/* 待修订为2 */
#define HAL_MAX_BA_LUT_SIZE                32
/* 待修订为2 */
#define HAL_MAX_AMPDU_LUT_SIZE             128

/*****************************************************************************
  2.5.3 AMSDU功能
*****************************************************************************/
#define AMSDU_ENABLE_ALL_TID                0xFF
/* amsdu下子msdu的最大长度 */
#define WLAN_MSDU_MAX_LEN                   128
/* 1102 spec amsdu最大长度，对应WLAN_LARGE_NETBUF_SIZE，受制于一个buffer长度 */
#define WLAN_AMSDU_FRAME_MAX_LEN            7935

/* >= WLAN_AMSDU_MAX_NUM/2  */
#define WLAN_DSCR_SUBTABEL_MAX_NUM          1

/*****************************************************************************
  2.5.6 小包优化
*****************************************************************************/
/* 管理帧长度  */
#define HAL_RX_MGMT_FRAME_LEN              WLAN_MGMT_NETBUF_SIZE
/* 短包长度 */
/* 短包队列会造成乱序问题,先关掉 */
#define HAL_RX_SMALL_FRAME_LEN             128

/* 接收描述符个数的宏定义 */
/* 小包数据接收描述符队列中描述符最大个数 */
#define HAL_SMALL_RX_MAX_BUFFS             52
/* 32 普通优先级接收描述符队列中描述符最大个数 32*2*3(amsdu占用buf的数目) */
#define HAL_NORMAL_RX_MAX_BUFFS            24
/* 4 高优先级接收描述符队列中描述符最大个数:取决于并发用户数(32) */
#define HAL_HIGH_RX_MAX_BUFFS              4
#define HAL_NORMAL_RX_MAX_RX_OPT_BUFFS     54
/* 普通优先级描述符优化规格 */
/*****************************************************************************
  2.5.7 自动调频
*****************************************************************************/
/* 未建立聚合时pps门限 */
#define NO_BA_PPS_VALUE_0              0
#define NO_BA_PPS_VALUE_1              500
#define NO_BA_PPS_VALUE_2              1000
#define NO_BA_PPS_VALUE_3              1500
/* mate7规格 */
/* pps门限       CPU主频下限     DDR主频下限 */
/* mate7 pps门限 */
#define PPS_VALUE_0              0
#define PPS_VALUE_1              1000
#define PPS_VALUE_2              2000
#define PPS_VALUE_3              8000
/* mate7 CPU主频下限 */
#define CPU_MIN_FREQ_VALUE_0              403200
#define CPU_MIN_FREQ_VALUE_1              604800
#define CPU_MIN_FREQ_VALUE_2              806400
#define CPU_MIN_FREQ_VALUE_3              1305600
/* mate7 DDR主频下限 */
#define DDR_MIN_FREQ_VALUE_0              0
#define DDR_MIN_FREQ_VALUE_1              3456
#define DDR_MIN_FREQ_VALUE_2              6403
#define DDR_MIN_FREQ_VALUE_3              9216
/*****************************************************************************
  2.6 算法类别的spec
*****************************************************************************/
/*****************************************************************************
  2.6.15 TPC功能
*****************************************************************************/
/* TPC步进DB数 */
#define WLAN_TPC_STEP                       3
/* 23 最大传输功率，单位dBm */
#define WLAN_MAX_TXPOWER                    30

#define HAL_TPC_UPC_LUT_NUM               16 /* 筛选后UPC档位的个数 */
#define HAL_MAX_CHAIN_NUM                 1  /* 最大通道个数 */
#define HAL_TPC_UPC_RF_LUT_NUM            128 /* UPC在RF档位中的数目 */
#define HAL_TPC_UPC_LUT_IDX_FOR_FAT_DIST  0
#define HAL_TPC_UPC_LUT_IDX_FOR_CALI_CODE 1
#define HAL_TPC_CHAIN0                    0 /* 通道0 */
#define HAL_TPC_CHAIN1                    1 /* 通道1 */
#define HAL_TPC_POW_LEVEL_NUM             5 /* 功率档位个数 */

#define WLAN_2G_HITALK_EXT_CHANNEL_NUM  2   /* 2g hiTalk扩展，增加2个信道，为了支持更多跳频个数 */

/*****************************************************************************
  2.6.22 STA P2P异频调度
*****************************************************************************/
/*  虚假队列个数，用于切离一个信道时，将原信道上放到硬件队列里的帧保存起来
当前只有2个场景: DBAC 与 背景扫描 DBAC占用2个队列，编号0 1; 背景扫描占用一个，编号2 */
/* HAL 打头待修订为WLAN */
#define HAL_TX_FAKE_QUEUE_NUM              3
#define HAL_TX_FAKE_QUEUE_BGSCAN_ID        2
#define HAL_FCS_PROT_MAX_FRAME_LEN         24

/*****************************************************************************
  2.8 架构形态类别的spec
*****************************************************************************/
/*****************************************************************************
  2.8.1 芯片适配规格
*****************************************************************************/
/* RF通道数规格 */
/* 通道0 */
#define WITP_RF_CHANNEL_ZERO        0
/* 通道1 */
#define WITP_RF_CHANNEL_ONE         1
/* 双通道 */
#define WITP_RF_CHANNEL_NUMS        1

/* 双通道掩码 */
#define WITP_TX_CHAIN_DOUBLE        3
/*  通道0 掩码 */
#define WITP_TX_CHAIN_ZERO          1
/*  通道1 掩码 */
#define WITP_TX_CHAIN_ONE           2

/* 芯片无效动态功率 */
#define WLAN_DYN_POW_INVALID        250
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

/* 芯片动态功率向上调整范围 */
#define WLAN_DYN_POW_UPPER_RANGE    10

/* 芯片动态功率向下调整范围 */
#define WLAN_DYN_POW_LOWER_RANGE    80
#endif
/* 芯片版本已放入平台的定制化 */
/*****************************************************************************
  2.8.2 STA AP规格
*****************************************************************************/
/* AP VAP的规格、STA VAP的规格、STA P2P共存的规格放入平台 */
/* PROXY STA模式下VAP规格宏定义放入平台 */
/* 每个device支持vap的最大个数已放入平台
*/
#define WLAN_HAL_OHTER_BSS_ID                   14   /* 其他BSS的ID */
#define WLAN_HAL_OTHER_BSS_UCAST_ID             7    /* 来自其他bss的单播管理帧和数据帧，维测用 */
#define WLAN_VAP_MAX_ID_PER_DEVICE_LIMIT        7    /* hal层，0-1两个AP，4-6三个STA */
/* MAC上报的tbtt中断类别最大值，2个ap的tbtt中断(0-1)+3个sta的tbtt中断(4-6) */
#define WLAN_MAC_REPORT_TBTT_IRQ_MAX            7
/* 整个BOARD支持的最大的device数目放入平台 */
/* 整个BOARD支持的最大的VAP数目已放入平台 */
/* 整个BOARD支持的最大业务VAP的数目 已放入平台 */
/*****************************************************************************
  2.8.3 低成本约束
*****************************************************************************/
/* 接收描述符个数的宏定义 */
/* HAL 打头待修订为WLAN */
/* 接收队列描述符最大个数 */
#define HAL_RX_MAX_BUFFS                  (HAL_NORMAL_RX_MAX_BUFFS + HAL_SMALL_RX_MAX_BUFFS + HAL_HIGH_RX_MAX_BUFFS)
/* 接收描述符做ping pong处理 */
#define HAL_HW_MAX_RX_DSCR_LIST_IDX        1
/* 用于存储接收完成中断最大个数 */
#define HAL_HW_RX_ISR_INFO_MAX_NUMS       (HAL_NORMAL_RX_MAX_BUFFS + HAL_SMALL_RX_MAX_BUFFS)
#define HAL_DOWM_PART_RX_TRACK_MEM         200
#ifdef _PRE_DEBUG_MODE
/* 接收描述符软件可见为第14行，用于打时间戳，调试用 */
#define HAL_DEBUG_RX_DSCR_LINE            (12 + 2)
#endif

/* 80211帧最大长度:软件最大为1600，流20字节的余量，防止硬件操作越界 */
#define HAL_RX_FRAME_LEN               WLAN_LARGE_NETBUF_SIZE
#define HAL_RX_FRAME_MAX_LEN           8000

/*****************************************************************************
  RX描述符动态调整
*****************************************************************************/
#define WLAN_PKT_MEM_PKT_OPT_LIMIT   2000
#define WLAN_PKT_MEM_PKT_RESET_LIMIT 500
#define WLAN_PKT_MEM_OPT_TIME_MS     1000
#define WLAN_PKT_MEM_OPT_MIN_PKT_LEN HAL_RX_SMALL_FRAME_LEN

/*****************************************************************************
  2.8.7 特性默认开启关闭定义
*****************************************************************************/
/* Feature动态当前未使用，待清理。能力没有使用 */
#define WLAN_FEATURE_AMPDU_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_AMSDU_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_WME_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_DSSS_CCK_IS_OPEN           OAL_FALSE
#define WLAN_FEATURE_UAPSD_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_PSM_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_WPA_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_TXBF_IS_OPEN               OAL_TRUE
#define WLAN_FEATURE_MSDU_DEFRAG_IS_OPEN        OAL_TRUE


/*****************************************************************************
  2.9 WiFi应用类别的spec
*****************************************************************************/
/*****************************************************************************
  2.9.4 P2P特性
*****************************************************************************/
/* Hi1102 P2P特性中P2P vap设备的大小限制(PER_DEVICE) */
#ifdef _PRE_WLAN_FEATURE_P2P
#define WLAN_MAX_SERVICE_P2P_DEV_NUM          1
#define WLAN_MAX_SERVICE_P2P_GOCLIENT_NUM     1
#define WLAN_MAX_SERVICE_CFG_VAP_NUM          1
#endif
/*****************************************************************************
  2.9.12 私有安全增强
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
#define WLAN_BLACKLIST_MAX                  32
#endif

/*****************************************************************************
  2.10 MAC FRAME特性
*****************************************************************************/
/*****************************************************************************
  2.10.1 ht cap info
*****************************************************************************/
#define HT_GREEN_FILED_DEFAULT_VALUE            0
#define HT_TX_STBC_DEFAULT_VALUE                0
/*****************************************************************************
  2.10.2 vht cap info
*****************************************************************************/
#define VHT_TX_STBC_DEFAULT_VALUE               0
/*****************************************************************************
  2.10.3 RSSI
*****************************************************************************/
/*****************************************************************************
  2.11 描述符偏移
*****************************************************************************/
#define WLAN_RX_DSCR_SIZE                       56
/*****************************************************************************
  2.12 COEX FEATURE
*****************************************************************************/
#define BTCOEX_RX_WINDOW_SIZE_LITTLE        0
#define BTCOEX_RX_WINDOW_SIZE_LARGE         1
#define BTCOEX_RX_WINDOW_SIZE_GRADES        2

#define BTCOEX_BT_NUM_OF_ACTIVE_MODE        3

#define BTCOEX_RX_LOW_RATE_TIME             5000
#define BTCOEX_RX_STATISTICS_TIME           2000
#define BTCOEX_SCO_CALCULATE_TIME           500

#define ALL_MID_PRIO_TIME                   10    // 5s / BTCOEX_SCO_CALCULATE_TIME
#define ALL_HIGH_PRIO_TIME                  4     // 2s / BTCOEX_SCO_CALCULATE_TIME

#define BTCOEX_RX_COUNT_LIMIT               128

#define BTCOEX_MAC_HDR                      32

#define BT_POSTPREEMPT_MAX_TIMES            15
#define BT_PREEMPT_MAX_TIMES                1
#define BT_POSTPREEMPT_TIMEOUT_US           150
#define BT_ABORT_RETRY_TIMES_MAX            10

#define BT_PREEMPT_TIMEOUT_US               50
#define BLE_PREEMPT_TIMEOUT_US              10

#define BTCOEX_BT_SCO_DURATION
#define BTCOEX_BT_DATATRANS_DURATION
#define BTCOEX_BT_A2DP_DURATION
#define BTCOEX_BT_DEFAULT_DURATION          0xFF

#define BTCOEX_PHY_TXRX_ALL_EN              0x0000000F
#define BTCOEX_BT2WIFI_RF_STABLE_TIME_US    50

#define BT_WLAN_COEX_UNAVAIL_PAYLOAD_THRES  8
#define BT_WLAN_COEX_SMALL_PKT_THRES        200
#define BT_WLAN_COEX_SMALL_FIFO_THRES       1023

#define BTCOEX_PRIO_TIMEOUT_150MS           150    // 软件定时器操作精度是ms
#define BTCOEX_PRIO_TIMEOUT_100MS           100
#define BTCOEX_PRIO_TIMEOUT_60MS            60

#define OCCUPIED_TIMES                      3
#define OCCUPIED_INTERVAL                   60
#define OCCUPIED_PERIOD                     60000

#define COEX_LINKLOSS_OCCUP_TIMES           15
#define COEX_LINKLOSS_OCCUP_PERIOD          20000

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST))*/

#endif /* #ifndef __WLAN_SPEC_1102_H__ */



