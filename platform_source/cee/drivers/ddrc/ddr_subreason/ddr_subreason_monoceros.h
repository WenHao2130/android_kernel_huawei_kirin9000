/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dmss intr subreason
 * Author: caodongyi
 * Create: 2020-5-27
 */

#ifndef _HISI_DDR_SUBREASON_MONOCEROS_H_
#define _HISI_DDR_SUBREASON_MONOCEROS_H_

#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <mntn_subtype_exception.h>

enum dmss_subreason {
	/* DDRC_SEC SUBTYPE REASON */
	MODID_DMSS_UNKNOWN_MASTER = MODID_DMSS_START,
	MODID_DMSS_LPMCU,
	MODID_DMSS_IOMCU_M7,
	MODID_DMSS_PCIE_1,
	MODID_DMSS_PERF_STAT,
	MODID_DMSS_MODEM,
	MODID_DMSS_DJTAG_M,
	MODID_DMSS_IOMCU_DMA,
	MODID_DMSS_UFS,
	MODID_DMSS_SD,
	MODID_DMSS_HSDT,
	MODID_DMSS_CC712,
	MODID_DMSS_FD_UL,
	MODID_DMSS_DPC,
	MODID_DMSS_USB31OTG,
	MODID_DMSS_HSDT_TCU,
	MODID_DMSS_DMAC,
	MODID_DMSS_ASP_HIFI,
	MODID_DMSS_PCIE_0,
	MODID_DMSS_ASP_DMA,
	MODID_DMSS_ISP,
	MODID_DMSS_DSS,
	MODID_DMSS_IPP_JPGENC,
	MODID_DMSS_MEDIA_COMMON_RCH6,
	MODID_DMSS_MEDIA_COMMON_CMD,
	MODID_DMSS_MEDIA_COMMON_RW,
	MODID_DMSS_VENC,
	MODID_DMSS_VDEC,
	MODID_DMSS_IVP,
	MODID_DMSS_IPP_JPGDEC,
	MODID_DMSS_IPP_VBK,
	MODID_DMSS_IPP_GF,
	MODID_DMSS_IPP_SLAM,
	MODID_DMSS_NPU,
	MODID_DMSS_FCM,
	MODID_DMSS_GPU,
	MODID_DMSS_IDI2AXI,
	MODID_DMSS_CRYPTO_ENHANCE,
	MODID_DMSS_MEDIA_TCU,
	MODID_DMSS_ARPP,
	MODID_DMSS_MAX = MODID_DMSS_END
};

static struct rdr_exception_info_s g_ddr_einfo[] = {
	{ { 0, 0 }, MODID_DMSS_UNKNOWN_MASTER, MODID_DMSS_UNKNOWN_MASTER, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_UNKNOWN_MASTER,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_LPMCU, MODID_DMSS_LPMCU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_LPMCU, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IOMCU_M7, MODID_DMSS_IOMCU_M7, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IOMCU_M7, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_PCIE_1, MODID_DMSS_PCIE_1, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_PCIE_1, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_PERF_STAT, MODID_DMSS_PERF_STAT, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_PERF_STAT,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_MODEM, MODID_DMSS_MODEM, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_MODEM, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_DJTAG_M, MODID_DMSS_DJTAG_M, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_DJTAG_M, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IOMCU_DMA, MODID_DMSS_IOMCU_DMA, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IOMCU_DMA,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_UFS, MODID_DMSS_UFS, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_UFS, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_SD, MODID_DMSS_SD, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_SD, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_HSDT, MODID_DMSS_HSDT, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_HSDT, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_CC712, MODID_DMSS_CC712, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_CC712, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_FD_UL, MODID_DMSS_FD_UL, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_FD_UL, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_DPC, MODID_DMSS_DPC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_DPC, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_USB31OTG, MODID_DMSS_USB31OTG, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_USB31OTG, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_HSDT_TCU, MODID_DMSS_HSDT_TCU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_HSDT_TCU, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_DMAC, MODID_DMSS_DMAC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_DMAC, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_ASP_HIFI, MODID_DMSS_ASP_HIFI, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_ASP_HIFI, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_PCIE_0, MODID_DMSS_PCIE_0, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_PCIE_0, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_ASP_DMA, MODID_DMSS_ASP_DMA, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_ASP_DMA, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_ISP, MODID_DMSS_ISP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_ISP, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_DSS, MODID_DMSS_DSS, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_DSS, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IPP_JPGENC, MODID_DMSS_IPP_JPGENC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IPP_JPGENC,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_MEDIA_COMMON_RCH6, MODID_DMSS_MEDIA_COMMON_CMD, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_MEDIA_COMMON_CMD,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_MEDIA_COMMON_CMD, MODID_DMSS_MEDIA_COMMON_CMD, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_MEDIA_COMMON_CMD,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_MEDIA_COMMON_RW, MODID_DMSS_MEDIA_COMMON_RW, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_MEDIA_COMMON_RW,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_VENC, MODID_DMSS_VENC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_VENC, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_VDEC, MODID_DMSS_VDEC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, MODID_DMSS_VDEC,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IVP, MODID_DMSS_IVP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IVP, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IPP_JPGDEC, MODID_DMSS_IPP_JPGDEC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IPP_JPGDEC,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IPP_VBK, MODID_DMSS_IPP_VBK, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IPP_VBK, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IPP_GF, MODID_DMSS_IPP_GF, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IPP_GF, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IPP_SLAM, MODID_DMSS_IPP_SLAM, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IPP_SLAM, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_NPU, MODID_DMSS_NPU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_NPU, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_FCM, MODID_DMSS_FCM, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_FCM, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_GPU, MODID_DMSS_GPU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_GPU, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_IDI2AXI, MODID_DMSS_IDI2AXI, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_IDI2AXI, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_CRYPTO_ENHANCE, MODID_DMSS_CRYPTO_ENHANCE, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_CRYPTO_ENHANCE,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_MEDIA_TCU, MODID_DMSS_MEDIA_TCU, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_MEDIA_TCU,
	 (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, 0 },
	{ { 0, 0 }, MODID_DMSS_ARPP, MODID_DMSS_ARPP, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)AP_S_DDRC_SEC, DMSS_ARPP, (u32)RDR_UPLOAD_YES, "ap",
	 "ap", 0, 0, 0 },
};

#endif
