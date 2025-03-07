/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef PS_LOG_NL2_FILE_ID_DEFINE_H
#define PS_LOG_NL2_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* NL2 打印文件ID定义  2017.11.01 */
typedef enum
{
    /*NPDCP*/
    PS_FILE_ID_NPDCP_COMMON_C               = NL2_TEAM_FILE_ID,
    PS_FILE_ID_NPDCP_NRT_COMM_C             = NL2_TEAM_FILE_ID + 1,
    PS_FILE_ID_NPDCP_UL_ENTITY_C            = NL2_TEAM_FILE_ID + 2,
    PS_FILE_ID_NPDCP_UL_DRB_TRANS_C         = NL2_TEAM_FILE_ID + 3,
    PS_FILE_ID_NPDCP_UL_SRB_TRANS_C         = NL2_TEAM_FILE_ID + 4,
    PS_FILE_ID_NPDCP_UL_DEBUG_C             = NL2_TEAM_FILE_ID + 5,
    PS_FILE_ID_NPDCP_DL_NRT_ENTITY_C        = NL2_TEAM_FILE_ID + 6,
    PS_FILE_ID_NPDCP_DL_NRT_SDU_PROC_C      = NL2_TEAM_FILE_ID + 7,
    PS_FILE_ID_NPDCP_DL_NRT_PDU_PROC_C      = NL2_TEAM_FILE_ID + 8,
    PS_FILE_ID_NPDCP_DL_NRT_SRB_PROC_C      = NL2_TEAM_FILE_ID + 9,
    PS_FILE_ID_NPDCP_NRT_CIPHER_CTRL_C      = NL2_TEAM_FILE_ID + 10,
    PS_FILE_ID_NPDCP_NRT_IPF_CTRL_C         = NL2_TEAM_FILE_ID + 11,
    PS_FILE_ID_N_PDCP_DL_DEBUG_C            = NL2_TEAM_FILE_ID + 12,
    PS_FILE_ID_NPDCP_DL_RT_DECODE_C         = NL2_TEAM_FILE_ID + 13,
    PS_FILE_ID_NPDCP_NRT_MEM_C              = NL2_TEAM_FILE_ID + 14,
    PS_FILE_ID_NPDCP_LTE_DL_RT_ENTITY_C     = NL2_TEAM_FILE_ID + 15,
    PS_FILE_ID_NPDCP_LTE_ULRT_PROC_C        = NL2_TEAM_FILE_ID + 16,
    PS_FILE_ID_NPDCP_LTE_UL_RT_ADAPTER_C    = NL2_TEAM_FILE_ID + 17,
    PS_FILE_ID_NPDCP_UL_NRT_ADAPTER_C       = NL2_TEAM_FILE_ID + 18,
    PS_FILE_ID_LUP_RT_EICC_ENTITY_C         = NL2_TEAM_FILE_ID + 19,
    PS_FILE_ID_NPDCP_LTE_COMM_C             = NL2_TEAM_FILE_ID + 20,
    PS_FILE_ID_NPDCP_UL_IPF_PROC_C          = NL2_TEAM_FILE_ID + 21,
    PS_FILE_ID_NPDCP_UL_COMPRESS_C          = NL2_TEAM_FILE_ID + 22,
    PS_FILE_ID_NPDCP_DL_DECOMPRESS_C        = NL2_TEAM_FILE_ID + 23,
    PS_FILE_ID_ROHC_OM_ITF_C                = NL2_TEAM_FILE_ID + 24,
    PS_FILE_ID_NPDCP_DL_DATA_PROC_C         = NL2_TEAM_FILE_ID + 25,
    PS_FILE_ID_NPDCP_CIPHER_CTRL_C          = NL2_TEAM_FILE_ID + 26,
    PS_FILE_ID_NPDCP_DL_DEBUG_C             = NL2_TEAM_FILE_ID + 27,
    PS_FILE_ID_NPDCP_DL_DECOMP_C            = NL2_TEAM_FILE_ID + 28,
    PS_FILE_ID_NPDCP_TIMER_C                = NL2_TEAM_FILE_ID + 29,
    PS_FILE_ID_NPDCP_L2DLE_CTRL_C           = NL2_TEAM_FILE_ID + 30,
    PS_FILE_ID_NPDCP_DL_SRB_HPS_C           = NL2_TEAM_FILE_ID + 31,
    PS_FILE_ID_NPDCP_ULNRT_DRBDATA_PROC_C   = NL2_TEAM_FILE_ID + 32,
    PS_FILE_ID_NPDCP_ULNRT_SRBDATA_PROC_C   = NL2_TEAM_FILE_ID + 33,
    PS_FILE_ID_NPDCP_ULRT_DATA_PROC_C       = NL2_TEAM_FILE_ID + 34,
    PS_FILE_ID_NPDCP_UL_OM_C                = NL2_TEAM_FILE_ID + 35,
    PS_FILE_ID_NPDCP_DL_OM_C                = NL2_TEAM_FILE_ID + 36,
    PS_FILE_ID_NPDCP_DL_NRT_ENTITYMANAGE_C  = NL2_TEAM_FILE_ID + 37,
    PS_FILE_ID_NPDCP_UL_ENTITYMANAGE_C      = NL2_TEAM_FILE_ID + 38,
    PS_FILE_ID_NPDCP_UL_L2MAMSGPROC_C       = NL2_TEAM_FILE_ID + 39,
    PS_FILE_ID_NPDCP_SDAPMSGPROC_C          = NL2_TEAM_FILE_ID + 40,
    PS_FILE_ID_NPDCP_MAC_MSGPROC_C          = NL2_TEAM_FILE_ID + 41,
    PS_FILE_ID_NPDCP_ULINTER_MSGPROC_C      = NL2_TEAM_FILE_ID + 42,
    PS_FILE_ID_NPDCP_BASTET_MSGPROC_C       = NL2_TEAM_FILE_ID + 43,
    PS_FILE_ID_NPDCP_UL_SPLIT_VOLUME_C      = NL2_TEAM_FILE_ID + 44,
    PS_FILE_ID_NPDCP_UL_SPLIT_CONTROL_C     = NL2_TEAM_FILE_ID + 45,
    PS_FILE_ID_NPDCP_UL_DISCARD_C           = NL2_TEAM_FILE_ID + 46,
    PS_FILE_ID_NPDCP_UL_DUPLICATE_C         = NL2_TEAM_FILE_ID + 47,
    PS_FILE_ID_NPDCP_UL_CONTROLPDU_C        = NL2_TEAM_FILE_ID + 48,
    PS_FILE_ID_NPDCP_UL_ENTRY_C             = NL2_TEAM_FILE_ID + 49,
    PS_FILE_ID_NPDCP_ULSTOREDATA_C          = NL2_TEAM_FILE_ID + 50,
    PS_FILE_ID_NPDCP_ULOPTIMIZE_C           = NL2_TEAM_FILE_ID + 51,
    PS_FILE_ID_NPDCP_DLOPTIMIZE_C           = NL2_TEAM_FILE_ID + 52,
    PS_FILE_ID_NPDCP_DLNRTCONTROLPDU_C      = NL2_TEAM_FILE_ID + 53,
    PS_FILE_ID_NPDCP_DLNRTREORDER_C         = NL2_TEAM_FILE_ID + 54,
    PS_FILE_ID_NPDCP_DLNRTL2MAMSGPROC_C     = NL2_TEAM_FILE_ID + 55,
    PS_FILE_ID_NPDCP_DLNRTINTRAMSGPROC_C    = NL2_TEAM_FILE_ID + 56,
    PS_FILE_ID_NPDCP_DLNRTMTAMSGPROC_C      = NL2_TEAM_FILE_ID + 57,
    PS_FILE_ID_NPDCP_DLNRTLTESDUPROC_C      = NL2_TEAM_FILE_ID + 58,
    PS_FILE_ID_NPDCP_ULNRT_OM_ITF_C         = NL2_TEAM_FILE_ID + 59,
    PS_FILE_ID_NPDCP_DLNRT_OM_ITF_C         = NL2_TEAM_FILE_ID + 60,
    PS_FILE_ID_NPDCP_ULCHR_C                = NL2_TEAM_FILE_ID + 61,
    PS_FILE_ID_NPDCP_DLCHR_C                = NL2_TEAM_FILE_ID + 62,
    PS_FILE_ID_NPDCP_DL_STATUS_C            = NL2_TEAM_FILE_ID + 63,
    PS_FILE_ID_NPDCP_L2DLE_STATUS_C         = NL2_TEAM_FILE_ID + 64,
    PS_FILE_ID_NPDCP_INTERNAL_ITF_C         = NL2_TEAM_FILE_ID + 65,
    PS_FILE_ID_NPDCP_L2DLE_WIN_PROTECT_C    = NL2_TEAM_FILE_ID + 66,
    PS_FILE_ID_NPDCP_L2DLE_REORDTMR_ADJUST_C = NL2_TEAM_FILE_ID + 67,
    PS_FILE_ID_NPDCP_L2DLE_SECURITY_C       = NL2_TEAM_FILE_ID + 68,
    PS_FILE_ID_NPDCP_L2DLE_RD_DISPATCH_C    = NL2_TEAM_FILE_ID + 69,
    PS_FILE_ID_NPDCP_L2DLE_ENTITY_ESTAB_C   = NL2_TEAM_FILE_ID + 70,
    PS_FILE_ID_NPDCP_L2DLE_ENTITY_POOL_C    = NL2_TEAM_FILE_ID + 71,
    PS_FILE_ID_NPDCP_L2DLE_ENTITY_RECFG_C   = NL2_TEAM_FILE_ID + 72,
    PS_FILE_ID_NPDCP_L2DLE_ENTITY_RELEASE_C = NL2_TEAM_FILE_ID + 73,
    PS_FILE_ID_NPDCP_L2DLE_INIT_C           = NL2_TEAM_FILE_ID + 74,
    PS_FILE_ID_NPDCP_CIPHER_INIT_C          = NL2_TEAM_FILE_ID + 75,
    PS_FILE_ID_NPDCP_UL_CIPHER_C            = NL2_TEAM_FILE_ID + 76,
    PS_FILE_ID_NPDCP_DL_DECIPHER_C          = NL2_TEAM_FILE_ID + 77,
    PS_FILE_ID_NPDCP_UL_ENTITY_POOL_C       = NL2_TEAM_FILE_ID + 78,
    PS_FILE_ID_NPDCP_UL_ENTITY_ESTAB_C      = NL2_TEAM_FILE_ID + 79,
    PS_FILE_ID_NPDCP_UL_ENTITY_RECFG_C      = NL2_TEAM_FILE_ID + 80,
    PS_FILE_ID_NPDCP_UL_ENTITY_RELEASE_C    = NL2_TEAM_FILE_ID + 81,
    PS_FILE_ID_NPDCP_UL_ENTITY_WINDOW_C     = NL2_TEAM_FILE_ID + 82,
    PS_FILE_ID_NPDCP_UL_DRB_RETRANS_C       = NL2_TEAM_FILE_ID + 83,
    PS_FILE_ID_NPDCP_UL_SRB_DISCARD_C       = NL2_TEAM_FILE_ID + 84,
    PS_FILE_ID_NPDCP_UL_SECURITY_C          = NL2_TEAM_FILE_ID + 85,
    PS_FILE_ID_NPDCP_UL_QUE_C               = NL2_TEAM_FILE_ID + 86,
    PS_FILE_ID_NPDCP_UL_TASK_C              = NL2_TEAM_FILE_ID + 87,
    PS_FILE_ID_NPDCP_UL_CDS_ITF_C           = NL2_TEAM_FILE_ID + 88,
    PS_FILE_ID_NPDCP_UL_L2MA_ITF_C          = NL2_TEAM_FILE_ID + 90,
    PS_FILE_ID_NPDCP_UL_NMAC_ITF_C          = NL2_TEAM_FILE_ID + 91,
    PS_FILE_ID_NPDCP_UL_SDAP_ITF_C          = NL2_TEAM_FILE_ID + 92,
    PS_FILE_ID_NPDCP_UL_ENTITY_REESTAB_C    = NL2_TEAM_FILE_ID + 93,
    PS_FILE_ID_NPDCP_ULRT_TASK_C            = NL2_TEAM_FILE_ID + 94,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_POOL_C     = NL2_TEAM_FILE_ID + 95,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_ESTAB_C    = NL2_TEAM_FILE_ID + 96,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_RECFG_C    = NL2_TEAM_FILE_ID + 97,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_RELEASE_C  = NL2_TEAM_FILE_ID + 98,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_REESTAB_C  = NL2_TEAM_FILE_ID + 99,
    PS_FILE_ID_NPDCP_ULRT_ENTITY_BACKUP_C   = NL2_TEAM_FILE_ID + 100,
    PS_FILE_ID_NPDCP_ULRT_BUF_CLEAN_C       = NL2_TEAM_FILE_ID + 101,
    PS_FILE_ID_NPDCP_ULRT_CONGEST_CHECK_C   = NL2_TEAM_FILE_ID + 102,
    PS_FILE_ID_NPDCP_ULRT_BSR_CALC_C        = NL2_TEAM_FILE_ID + 103,
    PS_FILE_ID_NPDCP_ULRT_NRT_NOTIFY_C      = NL2_TEAM_FILE_ID + 104,
    PS_FILE_ID_NPDCP_ULRT_PDU_TRANS_C       = NL2_TEAM_FILE_ID + 105,
    PS_FILE_ID_NPDCP_ULRT_AM_PDU_TRANS_C    = NL2_TEAM_FILE_ID + 106,
    PS_FILE_ID_NPDCP_ULRT_UM_PDU_TRANS_C    = NL2_TEAM_FILE_ID + 107,
    PS_FILE_ID_NPDCP_ULRT_SDU_TRANS_C       = NL2_TEAM_FILE_ID + 108,
    PS_FILE_ID_NPDCP_ULRT_SDU_DISCARD_C     = NL2_TEAM_FILE_ID + 109,
    PS_FILE_ID_NPDCP_ULRT_SPLIT_CTRL_C      = NL2_TEAM_FILE_ID + 110,
    PS_FILE_ID_NPDCP_ULRT_IMSA_ITF_C        = NL2_TEAM_FILE_ID + 111,
    PS_FILE_ID_NPDCP_ULRT_L2MA_ITF_C        = NL2_TEAM_FILE_ID + 112,
    PS_FILE_ID_NPDCP_UL_RLC_ITF_C           = NL2_TEAM_FILE_ID + 113,
    PS_FILE_ID_NPDCP_ULRT_LOWPOWER_ITF_C    = NL2_TEAM_FILE_ID + 114,
    PS_FILE_ID_NPDCP_ERABM_ITF_C            = NL2_TEAM_FILE_ID + 115,
    PS_FILE_ID_NPDCP_CHR_C                  = NL2_TEAM_FILE_ID + 116,
    PS_FILE_ID_NPDCP_ULRT_ERABM_ITF_C       = NL2_TEAM_FILE_ID + 117,
    PS_FILE_ID_NPDCP_ULRT_NMAC_ITF_C        = NL2_TEAM_FILE_ID + 118,
    PS_FILE_ID_NPDCP_ULRT_RLC_ITF_C         = NL2_TEAM_FILE_ID + 119,
    PS_FILE_ID_NPDCP_UL_SRB_CNF_C           = NL2_TEAM_FILE_ID + 120,
    PS_FILE_ID_NPDCP_UL_DRB_CNF_C           = NL2_TEAM_FILE_ID + 121,
    PS_FILE_ID_NPDCP_UL_RLC_CNF_C           = NL2_TEAM_FILE_ID + 122,
    PS_FILE_ID_NPDCP_DL_ENTITY_POOL_C       = NL2_TEAM_FILE_ID + 123,
    PS_FILE_ID_NPDCP_DL_ENTITY_ESTAB_C      = NL2_TEAM_FILE_ID + 124,
    PS_FILE_ID_NPDCP_DL_ENTITY_REESTAB_C    = NL2_TEAM_FILE_ID + 125,
    PS_FILE_ID_NPDCP_DL_ENTITY_RELEASE_C    = NL2_TEAM_FILE_ID + 126,
    PS_FILE_ID_NPDCP_DL_ENTITY_BACKUP_C     = NL2_TEAM_FILE_ID + 127,
    PS_FILE_ID_NPDCP_DL_ENTITY_INACTIVE_C   = NL2_TEAM_FILE_ID + 128,
    PS_FILE_ID_NPDCP_DL_ENTITY_RECONFIG_C   = NL2_TEAM_FILE_ID + 129,
    PS_FILE_ID_NPDCP_DL_ENTITY_WINDOW_C     = NL2_TEAM_FILE_ID + 130,
    PS_FILE_ID_NPDCP_DL_REORDER_C           = NL2_TEAM_FILE_ID + 131,
    PS_FILE_ID_NPDCP_DL_REORDER_OPTIMIZE_C  = NL2_TEAM_FILE_ID + 132,
    PS_FILE_ID_LPDCP_DL_ADAPTER_C           = NL2_TEAM_FILE_ID + 133,
    PS_FILE_ID_NPDCP_DL_NMAC_ITF_C          = NL2_TEAM_FILE_ID + 134,
    PS_FILE_ID_NPDCP_DL_RLC_ITF_C           = NL2_TEAM_FILE_ID + 135,
    PS_FILE_ID_NPDCP_DL_L2MA_ITF_C          = NL2_TEAM_FILE_ID + 136,
    PS_FILE_ID_NPDCP_DL_SECURITY_C          = NL2_TEAM_FILE_ID + 137,
    PS_FILE_ID_NPDCP_DL_SRB_RECV_C          = NL2_TEAM_FILE_ID + 138,
    PS_FILE_ID_NPDCP_DL_TASK_C              = NL2_TEAM_FILE_ID + 139,
    PS_FILE_ID_NPDCP_DL_SRB_DISCARD_C       = NL2_TEAM_FILE_ID + 140,
    PS_FILE_ID_NPDCP_DL_DRB_RECV_C          = NL2_TEAM_FILE_ID + 141,
    PS_FILE_ID_L2_NPDCP_NV_CFG_C            = NL2_TEAM_FILE_ID + 142,
    PS_FILE_ID_NPDCP_DL_CHR_C               = NL2_TEAM_FILE_ID + 143,
    PS_FILE_ID_NPDCP_UL_DT_C                = NL2_TEAM_FILE_ID + 144,
    PS_FILE_ID_NPDCP_ULRT_L2DS_PRIORITY_C   = NL2_TEAM_FILE_ID + 145,
    PS_FILE_ID_NPDCP_IMSAMSGPROC_C          = NL2_TEAM_FILE_ID + 146,
    PS_FILE_ID_NPDCP_VONR_AUTO_ANALYSIS_C   = NL2_TEAM_FILE_ID + 147,

    /*NRLC*/
    PS_FILE_ID_NRLC_COMM_C                  = NL2_TEAM_FILE_ID + 200,
    PS_FILE_ID_NRLC_TIMER_C                 = NL2_TEAM_FILE_ID + 201,
    PS_FILE_ID_NRLC_DEBUG_C                 = NL2_TEAM_FILE_ID + 202,
    PS_FILE_ID_NRLC_UL_NRT_EICC_MANAGE_C    = NL2_TEAM_FILE_ID + 203,
    PS_FILE_ID_NRLC_UL_NRT_ENTITY_C         = NL2_TEAM_FILE_ID + 204,
    PS_FILE_ID_NRLC_UL_NRT_PREPROC_C        = NL2_TEAM_FILE_ID + 205,
    PS_FILE_ID_NRLC_UL_NRT_ARQ_C            = NL2_TEAM_FILE_ID + 206,
    PS_FILE_ID_NRLC_DL_NRT_ENTITY_C         = NL2_TEAM_FILE_ID + 207,
    PS_FILE_ID_NRLC_DL_NRT_AM_RECV_C        = NL2_TEAM_FILE_ID + 208,
    PS_FILE_ID_NRLC_DL_NRT_UM_RECV_C        = NL2_TEAM_FILE_ID + 209,
    PS_FILE_ID_NRLC_DL_NRT_COMM_PROC_C      = NL2_TEAM_FILE_ID + 210,
    PS_FILE_ID_NRLC_UL_RT_ENTITY_C          = NL2_TEAM_FILE_ID + 211,
    PS_FILE_ID_NRLC_UL_RT_MSGPROC_C         = NL2_TEAM_FILE_ID + 212,
    PS_FILE_ID_NRLC_UL_RT_TM_TRANS_C        = NL2_TEAM_FILE_ID + 213,
    PS_FILE_ID_NRLC_UL_RT_UM_TRANS_C        = NL2_TEAM_FILE_ID + 214,
    PS_FILE_ID_NRLC_UL_RT_AM_TRANS_C        = NL2_TEAM_FILE_ID + 215,
    PS_FILE_ID_NRLC_DL_RT_ENTITY_C          = NL2_TEAM_FILE_ID + 216,
    PS_FILE_ID_NRLC_DL_RT_MSGPROC_C         = NL2_TEAM_FILE_ID + 217,
    PS_FILE_ID_NRLC_DL_RT_DECODE_C          = NL2_TEAM_FILE_ID + 218,
    PS_FILE_ID_NRLC_NMAC_CALLBACKS_C        = NL2_TEAM_FILE_ID + 219,
    PS_FILE_ID_NRLC_NRT_MEM_C               = NL2_TEAM_FILE_ID + 220,
    PS_FILE_ID_NRLC_DL_ENTITY_C             = NL2_TEAM_FILE_ID + 221,
    PS_FILE_ID_NRLC_DL_RECV_C               = NL2_TEAM_FILE_ID + 222,
    PS_FILE_ID_RLC_DL_DATA_PROC_C           = NL2_TEAM_FILE_ID + 223,
    PS_FILE_ID_RLC_DL_STATUS_PROC_C         = NL2_TEAM_FILE_ID + 224,
    PS_FILE_ID_RLC_L2DLE_ITF_C              = NL2_TEAM_FILE_ID + 225,
    PS_FILE_ID_NRLC_UL_OM_C                 = NL2_TEAM_FILE_ID + 226,
    PS_FILE_ID_NRLC_DL_OM_C                 = NL2_TEAM_FILE_ID + 227,
    PS_FILE_ID_NRLC_DT_C                    = NL2_TEAM_FILE_ID + 228,
    PS_FILE_ID_NRLC_MSG_SEND_C              = NL2_TEAM_FILE_ID + 229,
    PS_FILE_ID_NRLC_DL_AM_CFG_C             = NL2_TEAM_FILE_ID + 230,
    PS_FILE_ID_NRLC_DL_DATA_PROC_C          = NL2_TEAM_FILE_ID + 231,
    PS_FILE_ID_NRLC_DL_ENTITY_CFG_C         = NL2_TEAM_FILE_ID + 232,
    PS_FILE_ID_NRLC_DL_L2MA_ITF_C           = NL2_TEAM_FILE_ID + 233,
    PS_FILE_ID_NRLC_DL_MAC_ITF_C            = NL2_TEAM_FILE_ID + 234,
    PS_FILE_ID_NRLC_DL_MSG_PROC_C           = NL2_TEAM_FILE_ID + 235,
    PS_FILE_ID_NRLC_DL_OPTIMIZE_C           = NL2_TEAM_FILE_ID + 236,
    PS_FILE_ID_NRLC_DL_PDCP_ITF_C           = NL2_TEAM_FILE_ID + 237,
    PS_FILE_ID_NRLC_DL_TASK_C               = NL2_TEAM_FILE_ID + 238,
    PS_FILE_ID_NRLC_DL_TM_CFG_C             = NL2_TEAM_FILE_ID + 239,
    PS_FILE_ID_NRLC_DL_UL_ITF_C             = NL2_TEAM_FILE_ID + 240,
    PS_FILE_ID_NRLC_DL_UM_CFG_C             = NL2_TEAM_FILE_ID + 241,
    PS_FILE_ID_NRLC_DL_WIN_MGR_C            = NL2_TEAM_FILE_ID + 242,
    PS_FILE_ID_RLC_L2DLE_ENTITY_CFG_C       = NL2_TEAM_FILE_ID + 243,
    PS_FILE_ID_RLC_L2DLE_OM_C               = NL2_TEAM_FILE_ID + 244,
    PS_FILE_ID_RLC_L2DLE_REL_RD_C           = NL2_TEAM_FILE_ID + 245,
    PS_FILE_ID_RLC_L2DLE_SEG_RD_C           = NL2_TEAM_FILE_ID + 246,
    PS_FILE_ID_RLC_L2DLE_STATUS_PROC_C      = NL2_TEAM_FILE_ID + 247,
    PS_FILE_ID_NRLC_ULNRT_AM_CFG_C          = NL2_TEAM_FILE_ID + 248,
    PS_FILE_ID_NRLC_ULNRT_AM_POLLING_C      = NL2_TEAM_FILE_ID + 249,
    PS_FILE_ID_NRLC_ULNRT_AM_RETRANS_C      = NL2_TEAM_FILE_ID + 250,
    PS_FILE_ID_NRLC_ULNRT_AM_STATUS_PROC_C  = NL2_TEAM_FILE_ID + 251,
    PS_FILE_ID_NRLC_ULNRT_AM_TXWIN_C        = NL2_TEAM_FILE_ID + 252,
    PS_FILE_ID_NRLC_ULNRT_ENTITY_C          = NL2_TEAM_FILE_ID + 253,
    PS_FILE_ID_NRLC_ULNRT_ENTITY_CFG_C      = NL2_TEAM_FILE_ID + 254,
    PS_FILE_ID_NRLC_ULNRT_L2MA_ITF_C        = NL2_TEAM_FILE_ID + 255,
    PS_FILE_ID_NRLC_ULNRT_MSG_PROC_C        = NL2_TEAM_FILE_ID + 256,
    PS_FILE_ID_NRLC_ULNRT_OM_C              = NL2_TEAM_FILE_ID + 257,
    PS_FILE_ID_NRLC_ULNRT_PDCP_ITF_C        = NL2_TEAM_FILE_ID + 258,
    PS_FILE_ID_NRLC_ULNRT_RT_ITF_C          = NL2_TEAM_FILE_ID + 259,
    PS_FILE_ID_NRLC_ULNRT_TASK_C            = NL2_TEAM_FILE_ID + 260,
    PS_FILE_ID_NRLC_ULNRT_TM_CFG_C          = NL2_TEAM_FILE_ID + 261,
    PS_FILE_ID_NRLC_ULNRT_UM_CFG_C          = NL2_TEAM_FILE_ID + 262,
    PS_FILE_ID_NRLC_ULRT_AM_CFG_C           = NL2_TEAM_FILE_ID + 263,
    PS_FILE_ID_NRLC_ULRT_AM_DATATRANS_C     = NL2_TEAM_FILE_ID + 264,
    PS_FILE_ID_NRLC_ULRT_AM_POLLING_C       = NL2_TEAM_FILE_ID + 265,
    PS_FILE_ID_NRLC_ULRT_AM_STATUS_TRANS_C  = NL2_TEAM_FILE_ID + 266,
    PS_FILE_ID_NRLC_ULRT_ENTITY_C           = NL2_TEAM_FILE_ID + 267,
    PS_FILE_ID_NRLC_ULRT_ENTITY_CFG_C       = NL2_TEAM_FILE_ID + 268,
    PS_FILE_ID_NRLC_ULRT_MAC_ITF_C          = NL2_TEAM_FILE_ID + 269,
    PS_FILE_ID_NRLC_ULRT_MSG_PROC_C         = NL2_TEAM_FILE_ID + 270,
    PS_FILE_ID_NRLC_ULRT_NRT_ITF_C          = NL2_TEAM_FILE_ID + 271,
    PS_FILE_ID_NRLC_ULRT_OM_DATA_C          = NL2_TEAM_FILE_ID + 272,
    PS_FILE_ID_NRLC_ULRT_OM_STATS_C         = NL2_TEAM_FILE_ID + 273,
    PS_FILE_ID_NRLC_ULRT_OPTIMIZE_C         = NL2_TEAM_FILE_ID + 274,
    PS_FILE_ID_NRLC_ULRT_PDCP_ITF_C         = NL2_TEAM_FILE_ID + 275,
    PS_FILE_ID_NRLC_ULRT_SDU_PROC_C         = NL2_TEAM_FILE_ID + 276,
    PS_FILE_ID_NRLC_ULRT_TASK_C             = NL2_TEAM_FILE_ID + 277,
    PS_FILE_ID_NRLC_ULRT_TM_CFG_C           = NL2_TEAM_FILE_ID + 278,
    PS_FILE_ID_NRLC_ULRT_TM_TRANS_C         = NL2_TEAM_FILE_ID + 279,
    PS_FILE_ID_NRLC_ULRT_UM_CFG_C           = NL2_TEAM_FILE_ID + 280,
    PS_FILE_ID_NRLC_ULRT_UM_TRANS_C         = NL2_TEAM_FILE_ID + 281,

    /*NMAC*/
    PS_FILE_ID_NMAC_BEAM_C                  = NL2_TEAM_FILE_ID + 300,
    PS_FILE_ID_NMAC_CTRL_C                  = NL2_TEAM_FILE_ID + 301,
    PS_FILE_ID_NMAC_DECODE_C                = NL2_TEAM_FILE_ID + 302,
    PS_FILE_ID_NMAC_DL_ENTITY_C             = NL2_TEAM_FILE_ID + 304,
    PS_FILE_ID_NMAC_RANDOM_C                = NL2_TEAM_FILE_ID + 305,
    PS_FILE_ID_NMAC_UL_SCH_C                = NL2_TEAM_FILE_ID + 306,
    PS_FILE_ID_NMAC_UL_ENTITY_C             = NL2_TEAM_FILE_ID + 307,
    PS_FILE_ID_NMAC_MEM_C                   = NL2_TEAM_FILE_ID + 308,
    PS_FILE_ID_MAC_DL_PROC_C                = NL2_TEAM_FILE_ID + 309,
    PS_FILE_ID_MAC_DL_DEBUG_C               = NL2_TEAM_FILE_ID + 310,
    PS_FILE_ID_MAC_DL_PDU_ITF_C             = NL2_TEAM_FILE_ID + 311,
    PS_FILE_ID_MAC_DL_MSG_ITF_C             = NL2_TEAM_FILE_ID + 312,
    PS_FILE_ID_NMAC_UL_OM_C                 = NL2_TEAM_FILE_ID + 313,
    PS_FILE_ID_NMAC_DL_OM_C                 = NL2_TEAM_FILE_ID + 314,
    PS_FILE_ID_NMAC_RA_OM_C                 = NL2_TEAM_FILE_ID + 315,
    PS_FILE_ID_NMAC_ENTITY_C                = NL2_TEAM_FILE_ID + 316,
    PS_FILE_ID_NMAC_CELL_ENTITY_C           = NL2_TEAM_FILE_ID + 317,
    PS_FILE_ID_NMAC_LCH_C                   = NL2_TEAM_FILE_ID + 318,
    PS_FILE_ID_NMAC_TX_ENTITY_C             = NL2_TEAM_FILE_ID + 319,
    PS_FILE_ID_NMAC_UL_HARQ_ENTITY_C        = NL2_TEAM_FILE_ID + 320,
    PS_FILE_ID_NMAC_TX_PLANE_C              = NL2_TEAM_FILE_ID + 321,
    PS_FILE_ID_NMAC_UL_HARQ_C               = NL2_TEAM_FILE_ID + 322,
    PS_FILE_ID_NMAC_UL_CIPHER_C             = NL2_TEAM_FILE_ID + 323,
    PS_FILE_ID_NMAC_BBP_C                   = NL2_TEAM_FILE_ID + 324,
    PS_FILE_ID_NMAC_BWP_C                   = NL2_TEAM_FILE_ID + 325,
    PS_FILE_ID_NMAC_CELL_CFG_C              = NL2_TEAM_FILE_ID + 326,
    PS_FILE_ID_NMAC_ENTITY_CFG_C            = NL2_TEAM_FILE_ID + 327,
    PS_FILE_ID_NMAC_LUSCH_C                 = NL2_TEAM_FILE_ID + 328,
    PS_FILE_ID_NMAC_SR_C                    = NL2_TEAM_FILE_ID + 329,
    PS_FILE_ID_NMAC_TA_C                    = NL2_TEAM_FILE_ID + 330,
    PS_FILE_ID_NMAC_CA_C                    = NL2_TEAM_FILE_ID + 331,
    PS_FILE_ID_NMAC_ULSPS_C                 = NL2_TEAM_FILE_ID + 332,
    PS_FILE_ID_NMAC_DRX_C                   = NL2_TEAM_FILE_ID + 333,
    PS_FILE_ID_NMAC_ULSCH_C                 = NL2_TEAM_FILE_ID + 334,
    PS_FILE_ID_NMAC_BSR_C                   = NL2_TEAM_FILE_ID + 335,
    PS_FILE_ID_NMAC_DSDS_C                  = NL2_TEAM_FILE_ID + 336,
    PS_FILE_ID_LMAC_HPS_DSDS_C              = NL2_TEAM_FILE_ID + 337,
    PS_FILE_ID_NMAC_DEBUG_C                 = NL2_TEAM_FILE_ID + 338,
    PS_FILE_ID_L_MACRA_C                    = NL2_TEAM_FILE_ID + 339,
    PS_FILE_ID_N_MACSEND_C                  = NL2_TEAM_FILE_ID + 340,
    PS_FILE_ID_NMAC_L2MA_MSG_C              = NL2_TEAM_FILE_ID + 341,
    PS_FILE_ID_NMAC_LRRC_MSG_C              = NL2_TEAM_FILE_ID + 342,
    PS_FILE_ID_NMAC_UL_STATISTIC_C          = NL2_TEAM_FILE_ID + 343,
    PS_FILE_ID_NMAC_RA_BEAM_C               = NL2_TEAM_FILE_ID + 344,
    PS_FILE_ID_NMAC_RA_CFG_C                = NL2_TEAM_FILE_ID + 345,
    PS_FILE_ID_NMAC_RA_STATE_C              = NL2_TEAM_FILE_ID + 346,
    PS_FILE_ID_NMAC_RA_INIT_C               = NL2_TEAM_FILE_ID + 347,
    PS_FILE_ID_NMAC_RA_MSG1_C               = NL2_TEAM_FILE_ID + 348,
    PS_FILE_ID_NMAC_RA_MSG2_C               = NL2_TEAM_FILE_ID + 349,
    PS_FILE_ID_NMAC_RA_MSG3_C               = NL2_TEAM_FILE_ID + 350,
    PS_FILE_ID_NMAC_RA_MSG4_C               = NL2_TEAM_FILE_ID + 351,
    PS_FILE_ID_NMAC_RA_SEND_C               = NL2_TEAM_FILE_ID + 352,
    PS_FILE_ID_NMAC_RA_TRIG_C               = NL2_TEAM_FILE_ID + 353,
    PS_FILE_ID_NMAC_RE_RA_C                 = NL2_TEAM_FILE_ID + 354,
    PS_FILE_ID_LMAC_RA_CFG_C                = NL2_TEAM_FILE_ID + 355,
    PS_FILE_ID_LMAC_RA_STATE_C              = NL2_TEAM_FILE_ID + 356,
    PS_FILE_ID_LMAC_RA_TRIG_C               = NL2_TEAM_FILE_ID + 357,
    PS_FILE_ID_LMAC_RA_MSG1_C               = NL2_TEAM_FILE_ID + 358,
    PS_FILE_ID_LMAC_RA_MSG2_C               = NL2_TEAM_FILE_ID + 359,
    PS_FILE_ID_LMAC_RA_MSG3_C               = NL2_TEAM_FILE_ID + 360,
    PS_FILE_ID_LMAC_RA_MSG4_C               = NL2_TEAM_FILE_ID + 361,
    PS_FILE_ID_LMAC_RA_SEND_C               = NL2_TEAM_FILE_ID + 362,
    PS_FILE_ID_LMAC_RE_RA_C                 = NL2_TEAM_FILE_ID + 363,
    PS_FILE_ID_NMAC_PHR_CFG_C               = NL2_TEAM_FILE_ID + 364,
    PS_FILE_ID_NMAC_PHR_TRIG_C              = NL2_TEAM_FILE_ID + 365,
    PS_FILE_ID_NMAC_PHR_SCH_C               = NL2_TEAM_FILE_ID + 366,
    PS_FILE_ID_NMAC_PHR_UPDATA_C            = NL2_TEAM_FILE_ID + 367,
    PS_FILE_ID_LMAC_CFG_C                   = NL2_TEAM_FILE_ID + 368,
    PS_FILE_ID_LMAC_UL_SPS_C                = NL2_TEAM_FILE_ID + 369,
    PS_FILE_ID_NMAC_MULTIPLEX_C             = NL2_TEAM_FILE_ID + 370,
    PS_FILE_ID_NMAC_LCH_BUFF_C              = NL2_TEAM_FILE_ID + 371,
    PS_FILE_ID_NMAC_LCH_PRI_C               = NL2_TEAM_FILE_ID + 372,
    PS_FILE_ID_NMAC_SR_TIMER_C              = NL2_TEAM_FILE_ID + 373,
    PS_FILE_ID_NMAC_SR_PROC_C               = NL2_TEAM_FILE_ID + 374,
    PS_FILE_ID_NMAC_BSR_CFG_C               = NL2_TEAM_FILE_ID + 375,
    PS_FILE_ID_NMAC_BSR_TIMER_C             = NL2_TEAM_FILE_ID + 376,
    PS_FILE_ID_NMAC_BSR_LCG_C               = NL2_TEAM_FILE_ID + 377,
    PS_FILE_ID_NMAC_BSR_TRIG_C              = NL2_TEAM_FILE_ID + 378,
    PS_FILE_ID_NMAC_BSR_SCHED_C             = NL2_TEAM_FILE_ID + 379,
    PS_FILE_ID_NMAC_DT_C                    = NL2_TEAM_FILE_ID + 380,
    PS_FILE_ID_NMAC_DCM_C                   = NL2_TEAM_FILE_ID + 381,
    PS_FILE_ID_NMAC_MTA_C                   = NL2_TEAM_FILE_ID + 382,
    PS_FILE_ID_NMAC_ULOM_MSG_C              = NL2_TEAM_FILE_ID + 383,
    PS_FILE_ID_NMAC_ULOM_PDU_C              = NL2_TEAM_FILE_ID + 384,
    PS_FILE_ID_NMAC_DL_CUST_OM_C            = NL2_TEAM_FILE_ID + 385,
    PS_FILE_ID_NMAC_UL_CUST_OM_C            = NL2_TEAM_FILE_ID + 386,
    PS_FILE_ID_NMAC_BSR_FC_C                = NL2_TEAM_FILE_ID + 387,
    PS_FILE_ID_NMAC_CHR_C                   = NL2_TEAM_FILE_ID + 388,

    /*NCOMM*/
    PS_FILE_ID_NUP_TIMER_C                  = NL2_TEAM_FILE_ID + 400,
    PS_FILE_ID_NUP_QUEUE_C                  = NL2_TEAM_FILE_ID + 401,
    PS_FILE_ID_NUP_CIPHER_C                 = NL2_TEAM_FILE_ID + 402,
    PS_FILE_ID_NUP_EICC_C                   = NL2_TEAM_FILE_ID + 403,
    PS_FILE_ID_NUP_NRT_COM_MEM_C            = NL2_TEAM_FILE_ID + 404,
    PS_FILE_ID_NUP_RT_COM_MEM_C             = NL2_TEAM_FILE_ID + 405,
    PS_FILE_ID_NUP_NRT_EICC_ENTITY_C        = NL2_TEAM_FILE_ID + 406,
    PS_FILE_ID_NUP_RT_EICC_ENTITY_C         = NL2_TEAM_FILE_ID + 407,
    PS_FILE_ID_NUP_OM_ITF_C                 = NL2_TEAM_FILE_ID + 408,
    PS_FILE_ID_NUP_NRT_OM_COM_C             = NL2_TEAM_FILE_ID + 409,
    PS_FILE_ID_NUP_RT_OM_COM_C              = NL2_TEAM_FILE_ID + 410,
    PS_FILE_ID_NMAC_RT_OM_ITF_C             = NL2_TEAM_FILE_ID + 411,
    PS_FILE_ID_NPDCP_NRT_OM_ITF_C           = NL2_TEAM_FILE_ID + 412,
    PS_FILE_ID_NPDCP_DLRT_OM_ITF_C          = NL2_TEAM_FILE_ID + 413,
    PS_FILE_ID_NRLC_NRT_OM_ITF_C            = NL2_TEAM_FILE_ID + 414,
    PS_FILE_ID_NRLC_RT_OM_ITF_C             = NL2_TEAM_FILE_ID + 415,
    PS_FILE_ID_NUP_RT_DEBUG_C               = NL2_TEAM_FILE_ID + 416,
    PS_FILE_ID_NUP_RT_SLEEP_C               = NL2_TEAM_FILE_ID + 417,
    PS_FILE_ID_NUP_NRT_SLEEP_C              = NL2_TEAM_FILE_ID + 418,
    PS_FILE_ID_NUP_RTFC_C                   = NL2_TEAM_FILE_ID + 419,
    PS_FILE_ID_NUP_RT_DT_ITF_C              = NL2_TEAM_FILE_ID + 420,
    PS_FILE_ID_NUP_NRT_DT_ITF_C             = NL2_TEAM_FILE_ID + 421,
    PS_FILE_ID_NUP_CHR_C                    = NL2_TEAM_FILE_ID + 422,
    PS_FILE_ID_SLEEP_PROC_C                 = NL2_TEAM_FILE_ID + 423,
    PS_FILE_ID_NUP_OM_COMM_C                = NL2_TEAM_FILE_ID + 424,
    PS_FILE_ID_SLEEP_OM_C                   = NL2_TEAM_FILE_ID + 425,
    PS_FILE_ID_NUP_RT_TASK_C                = NL2_TEAM_FILE_ID + 426,
    PS_FILE_ID_NUP_BBP_C                    = NL2_TEAM_FILE_ID + 427,
    PS_FILE_ID_NUP_CUST_OM_C                = NL2_TEAM_FILE_ID + 428,
    PS_FILE_ID_SLEEP_CPU_DVFS_C             = NL2_TEAM_FILE_ID + 429,
    PS_FILE_ID_SLEEP_DDR_DVFS_C             = NL2_TEAM_FILE_ID + 430,
    PS_FILE_ID_SLEEP_L2DLE_DVFS_C           = NL2_TEAM_FILE_ID + 431,
    PS_FILE_ID_SLEEP_COMM_DVFS_C            = NL2_TEAM_FILE_ID + 432,
    PS_FILE_ID_LUP_CHR_C                    = NL2_TEAM_FILE_ID + 433,
    PS_FILE_ID_NUP_APP_TASK_C               = NL2_TEAM_FILE_ID + 434,
    PS_FILE_ID_NUP_OM_L2DLE_C               = NL2_TEAM_FILE_ID + 435,
    PS_FILE_ID_NUP_OM_PDCP_C                = NL2_TEAM_FILE_ID + 436,
    PS_FILE_ID_NUP_OM_MAC_C                 = NL2_TEAM_FILE_ID + 437,
    PS_FILE_ID_NUP_OM_RLC_C                 = NL2_TEAM_FILE_ID + 438,
    PS_FILE_ID_NUP_OM_KEYEVENT_C            = NL2_TEAM_FILE_ID + 439,
    PS_FILE_ID_CHR_COMM_C                   = NL2_TEAM_FILE_ID + 440,
    PS_FILE_ID_NUP_NRT_NV_C                 = NL2_TEAM_FILE_ID + 441,
    PS_FILE_ID_L2DS_MSG_PROC_C              = NL2_TEAM_FILE_ID + 442,
    PS_FILE_ID_L2DS_TRANS_PRI_C             = NL2_TEAM_FILE_ID + 443,
    PS_FILE_ID_L2_NV_COM_C                  = NL2_TEAM_FILE_ID + 444,
    PS_FILE_ID_SLEEP_CDS_C                  = NL2_TEAM_FILE_ID + 445,
    PS_FILE_ID_SLEEP_DL_ALLOW_SLEEP_C       = NL2_TEAM_FILE_ID + 446,
    PS_FILE_ID_SLEEP_ECIPHER_C              = NL2_TEAM_FILE_ID + 447,
    PS_FILE_ID_SLEEP_FORCE_ACT_C            = NL2_TEAM_FILE_ID + 448,
    PS_FILE_ID_SLEEP_L2DLE_CLOCK_C          = NL2_TEAM_FILE_ID + 449,
    PS_FILE_ID_SLEEP_L2DLE_INTERFACE_C      = NL2_TEAM_FILE_ID + 450,
    PS_FILE_ID_SLEEP_MAC_DL_C               = NL2_TEAM_FILE_ID + 451,
    PS_FILE_ID_SLEEP_MAC_PHY_MSG_C          = NL2_TEAM_FILE_ID + 452,
    PS_FILE_ID_SLEEP_MAC_UL_C               = NL2_TEAM_FILE_ID + 453,
    PS_FILE_ID_SLEEP_MODEM_C                = NL2_TEAM_FILE_ID + 454,
    PS_FILE_ID_SLEEP_PDCP_DL_C              = NL2_TEAM_FILE_ID + 455,
    PS_FILE_ID_SLEEP_PDCP_UL_C              = NL2_TEAM_FILE_ID + 456,
    PS_FILE_ID_SLEEP_RLC_DL_C               = NL2_TEAM_FILE_ID + 457,
    PS_FILE_ID_SLEEP_RLC_UL_C               = NL2_TEAM_FILE_ID + 458,
    PS_FILE_ID_SLEEP_SEND_MSG_TO_APP_C      = NL2_TEAM_FILE_ID + 459,
    PS_FILE_ID_SLEEP_SEND_MSG_TO_PHY_C      = NL2_TEAM_FILE_ID + 460,
    PS_FILE_ID_SLEEP_UL_CIPHER_C            = NL2_TEAM_FILE_ID + 461,
    PS_FILE_ID_SLEEP_UL_ALLOW_SLEEP_C       = NL2_TEAM_FILE_ID + 462,
    PS_FILE_ID_SLEEP_WAKE_C                 = NL2_TEAM_FILE_ID + 463,
    PS_FILE_ID_NUP_MEM_OM_C                 = NL2_TEAM_FILE_ID + 464,

    /*SDAP*/
    PS_FILE_ID_SDAP_ENTITY_C                = NL2_TEAM_FILE_ID + 500,
    PS_FILE_ID_SDAP_UL_PROC_C               = NL2_TEAM_FILE_ID + 501,
    PS_FILE_ID_SDAP_DL_PROC_C               = NL2_TEAM_FILE_ID + 502,
    PS_FILE_ID_SDAP_DEBUG_C                 = NL2_TEAM_FILE_ID + 503,
    PS_FILE_ID_SDAP_IP_PKT_PARSE_C          = NL2_TEAM_FILE_ID + 504,
    PS_FILE_ID_SDAP_IP_MSG_PROC_C           = NL2_TEAM_FILE_ID + 505,
    PS_FILE_ID_SDAP_TASK_C                  = NL2_TEAM_FILE_ID + 506,
    PS_FILE_ID_SDAP_SOFT_FILTER_C           = NL2_TEAM_FILE_ID + 507,
    PS_FILE_ID_SDAP_CCORE_DATA_PROC_C       = NL2_TEAM_FILE_ID + 508,
    PS_FILE_ID_SDAP_CDS_MSG_PROC_C          = NL2_TEAM_FILE_ID + 509,
    PS_FILE_ID_SDAP_DFS_PROC_C              = NL2_TEAM_FILE_ID + 510,
    PS_FILE_ID_SDAP_DL_CIPHER_PROC_C        = NL2_TEAM_FILE_ID + 511,
    PS_FILE_ID_SDAP_DL_DATA_DISPATCH_PROC_C = NL2_TEAM_FILE_ID + 512,
    PS_FILE_ID_SDAP_DL_FRAG_PROC_C          = NL2_TEAM_FILE_ID + 513,
    PS_FILE_ID_SDAP_ENTITY_CFG_C            = NL2_TEAM_FILE_ID + 514,
    PS_FILE_ID_SDAP_IFACE_CFG_C             = NL2_TEAM_FILE_ID + 515,
    PS_FILE_ID_SDAP_IMSA_MSG_PROC_C         = NL2_TEAM_FILE_ID + 516,
    PS_FILE_ID_SDAP_IPF_DL_PROC_C           = NL2_TEAM_FILE_ID + 517,
    PS_FILE_ID_SDAP_IPF_ENTITY_C            = NL2_TEAM_FILE_ID + 518,
    PS_FILE_ID_SDAP_IPF_UL_PROC_C           = NL2_TEAM_FILE_ID + 519,
    PS_FILE_ID_SDAP_L2MA_MSG_PROC_C         = NL2_TEAM_FILE_ID + 520,
    PS_FILE_ID_SDAP_LB_DATA_PROC_C          = NL2_TEAM_FILE_ID + 521,
    PS_FILE_ID_SDAP_LB_ENTITY_C             = NL2_TEAM_FILE_ID + 522,
    PS_FILE_ID_SDAP_NRMM_MSG_PROC_C         = NL2_TEAM_FILE_ID + 523,
    PS_FILE_ID_SDAP_NRSM_MSG_PROC_C         = NL2_TEAM_FILE_ID + 524,
    PS_FILE_ID_SDAP_PDUSESSION_CFG_C        = NL2_TEAM_FILE_ID + 525,
    PS_FILE_ID_SDAP_QUEUE_C                 = NL2_TEAM_FILE_ID + 526,
    PS_FILE_ID_SDAP_SR_CTRL_C               = NL2_TEAM_FILE_ID + 527,
    PS_FILE_ID_SDAP_STAT_C                  = NL2_TEAM_FILE_ID + 528,
    PS_FILE_ID_SDAP_TC_MSG_PROC_C           = NL2_TEAM_FILE_ID + 529,
    PS_FILE_ID_SDAP_THROUPT_C               = NL2_TEAM_FILE_ID + 530,
    PS_FILE_ID_SDAP_TMR_C                   = NL2_TEAM_FILE_ID + 531,
    PS_FILE_ID_SDAP_TMR_MSG_PROC_C          = NL2_TEAM_FILE_ID + 532,
    PS_FILE_ID_SDAP_UL_BUFF_PROC_C          = NL2_TEAM_FILE_ID + 533,
    PS_FILE_ID_SDAP_LOOPBACK_C              = NL2_TEAM_FILE_ID + 534,
    PS_FILE_ID_SDAP_ETH_SOFT_FILTER_C       = NL2_TEAM_FILE_ID + 535,
    PS_FILE_ID_SDAP_IP_SOFT_FILTER_C        = NL2_TEAM_FILE_ID + 536,
    PS_FILE_ID_SDAP_COMM_ENTRY_C            = NL2_TEAM_FILE_ID + 537,
    PS_FILE_ID_SDAP_COMM_TMR_C              = NL2_TEAM_FILE_ID + 538,
    PS_FILE_ID_SDAP_DL_DATA_PROC_C          = NL2_TEAM_FILE_ID + 539,
    PS_FILE_ID_SDAP_DL_REFLECT_C            = NL2_TEAM_FILE_ID + 540,
    PS_FILE_ID_SDAP_ENTITY_COMM_C           = NL2_TEAM_FILE_ID + 541,
    PS_FILE_ID_SDAP_CDS_ITF_C               = NL2_TEAM_FILE_ID + 542,
    PS_FILE_ID_SDAP_IMSA_ITF_C              = NL2_TEAM_FILE_ID + 543,
    PS_FILE_ID_SDAP_L2MA_ITF_C              = NL2_TEAM_FILE_ID + 544,
    PS_FILE_ID_SDAP_NRMM_ITF_C              = NL2_TEAM_FILE_ID + 545,
    PS_FILE_ID_SDAP_NRSM_ITF_C              = NL2_TEAM_FILE_ID + 546,
    PS_FILE_ID_SDAP_PDCP_ITF_C              = NL2_TEAM_FILE_ID + 547,
    PS_FILE_ID_SDAP_TC_ITF_C                = NL2_TEAM_FILE_ID + 548,
    PS_FILE_ID_SDAP_TMR_ITF_C               = NL2_TEAM_FILE_ID + 549,
    PS_FILE_ID_SDAP_UL_CTRL_PDU_C           = NL2_TEAM_FILE_ID + 550,
    PS_FILE_ID_SDAP_UL_DATA_PROC_C          = NL2_TEAM_FILE_ID + 551,
    PS_FILE_ID_SDAP_UL_QFI_DRB_MAP_C        = NL2_TEAM_FILE_ID + 552,
    PS_FILE_ID_SDAP_UL_SR_CTRL_C            = NL2_TEAM_FILE_ID + 553,

    /*L2MA*/
    PS_FILE_ID_L2MA_ENTITY_C                = NL2_TEAM_FILE_ID + 600,
    PS_FILE_ID_L2MA_MSG_PROC_C              = NL2_TEAM_FILE_ID + 601,
    PS_FILE_ID_L2MA_MACRLC_MSG_PROC_C       = NL2_TEAM_FILE_ID + 602,
    PS_FILE_ID_L2MA_PDCP_MSG_PROC_C         = NL2_TEAM_FILE_ID + 603,
    PS_FILE_ID_L2MA_SDAP_MSG_PROC_C         = NL2_TEAM_FILE_ID + 604,
    PS_FILE_ID_L2MA_DEBUG_C                 = NL2_TEAM_FILE_ID + 605,
    PS_FILE_ID_L2MA_CDS_MSG_PROC_C          = NL2_TEAM_FILE_ID + 606,
    PS_FILE_ID_L2MA_NMAC_MSG_CHECK_C        = NL2_TEAM_FILE_ID + 607,
    PS_FILE_ID_L2MA_NMAC_MSG_CONSTRUNT_C    = NL2_TEAM_FILE_ID + 608,
    PS_FILE_ID_L2MA_NMAC_MSG_PROC_C         = NL2_TEAM_FILE_ID + 609,
    PS_FILE_ID_L2MA_NRLC_MSG_PROC_C         = NL2_TEAM_FILE_ID + 610,
    PS_FILE_ID_L2MA_PDCP_MSG_DISPATCH_C     = NL2_TEAM_FILE_ID + 611,
    PS_FILE_ID_L2MA_CHR_MSG_PROC_C          = NL2_TEAM_FILE_ID + 612,
    PS_FILE_ID_L2MA_TASK_C                  = NL2_TEAM_FILE_ID + 613,
    PS_FILE_ID_L2MA_TIMER_C                 = NL2_TEAM_FILE_ID + 614,
    PS_FILE_ID_L2MA_MSG_FILTER_C            = NL2_TEAM_FILE_ID + 615,
    PS_FILE_ID_L2MA_SDAP_CFG_C              = NL2_TEAM_FILE_ID + 616,
    PS_FILE_ID_L2MA_SDAP_REL_C              = NL2_TEAM_FILE_ID + 617,
    PS_FILE_ID_L2MA_SDAP_REEST_C            = NL2_TEAM_FILE_ID + 618,
    PS_FILE_ID_L2MA_NPDCP_CFG_C             = NL2_TEAM_FILE_ID + 619,
    PS_FILE_ID_L2MA_NPDCP_REEST_C           = NL2_TEAM_FILE_ID + 620,
    PS_FILE_ID_L2MA_NPDCP_REL_C             = NL2_TEAM_FILE_ID + 621,
    PS_FILE_ID_L2MA_NPDCP_SECURITY_C        = NL2_TEAM_FILE_ID + 622,
    PS_FILE_ID_L2MA_NPDCP_DATATRANS_C       = NL2_TEAM_FILE_ID + 623,
    PS_FILE_ID_L2MA_NPDCP_MSG_PROC_C        = NL2_TEAM_FILE_ID + 624,
    PS_FILE_ID_L2MA_NPDCP_LCHMAP_C          = NL2_TEAM_FILE_ID + 625,
    PS_FILE_ID_L2MA_NRRC_MSGPROC_C          = NL2_TEAM_FILE_ID + 626,
    PS_FILE_ID_L2MA_NRLC_CFG_C              = NL2_TEAM_FILE_ID + 627,
    PS_FILE_ID_L2MA_NRLC_REEST_C            = NL2_TEAM_FILE_ID + 628,
    PS_FILE_ID_L2MA_NRLC_REL_C              = NL2_TEAM_FILE_ID + 629,
    PS_FILE_ID_L2MA_NRLC_DATATRANS_C        = NL2_TEAM_FILE_ID + 630,
    PS_FILE_ID_L2MA_NRLC_MSGPROC_C          = NL2_TEAM_FILE_ID + 631,
    PS_FILE_ID_L2MA_NMAC_NRLC_MSGPROC_C     = NL2_TEAM_FILE_ID + 632,
    PS_FILE_ID_L2MA_NMAC_MAINCFG_C          = NL2_TEAM_FILE_ID + 633,
    PS_FILE_ID_L2MA_NMAC_CONGEST_C          = NL2_TEAM_FILE_ID + 634,
    PS_FILE_ID_L2MA_NMAC_DSDS_C             = NL2_TEAM_FILE_ID + 635,
    PS_FILE_ID_L2MA_NMAC_STATECHANGE_C      = NL2_TEAM_FILE_ID + 636,
    PS_FILE_ID_L2MA_NMAC_HPS_C              = NL2_TEAM_FILE_ID + 637,
    PS_FILE_ID_L2MA_NMAC_RA_C               = NL2_TEAM_FILE_ID + 638,
    PS_FILE_ID_L2MA_NMAC_REL_C              = NL2_TEAM_FILE_ID + 639,
    PS_FILE_ID_L2MA_NMAC_SCELL_C            = NL2_TEAM_FILE_ID + 640,
    PS_FILE_ID_L2MA_NMAC_DEMANDSI_C         = NL2_TEAM_FILE_ID + 641,
    PS_FILE_ID_L2MA_NMAC_LCHCFG_C           = NL2_TEAM_FILE_ID + 642,
    PS_FILE_ID_L2MA_NMAC_SPCELL_C           = NL2_TEAM_FILE_ID + 643,
    PS_FILE_ID_L2MA_NMAC_BWP_C              = NL2_TEAM_FILE_ID + 644,
    PS_FILE_ID_L2MA_BASTET_MSGPROC_C        = NL2_TEAM_FILE_ID + 645,
    PS_FILE_ID_L2MA_MTA_MSGPROC_C           = NL2_TEAM_FILE_ID + 646,
    PS_FILE_ID_L2MA_EVENT_C                 = NL2_TEAM_FILE_ID + 647,

    /* L2LB */
    PS_FILE_ID_L2LB_COMM_ENTRY_C            = NL2_TEAM_FILE_ID + 700,
    PS_FILE_ID_L2LB_COMM_TMR_C              = NL2_TEAM_FILE_ID + 701,
    PS_FILE_ID_L2LB_DATA_MODE_A_C           = NL2_TEAM_FILE_ID + 702,
    PS_FILE_ID_L2LB_DATA_MODE_B_C           = NL2_TEAM_FILE_ID + 703,
    PS_FILE_ID_L2LB_DATA_MODE_COMM_C        = NL2_TEAM_FILE_ID + 704,
    PS_FILE_ID_L2LB_DEBUG_C                 = NL2_TEAM_FILE_ID + 705,
    PS_FILE_ID_L2LB_ENTITY_CFG_C            = NL2_TEAM_FILE_ID + 706,
    PS_FILE_ID_L2LB_ENTITY_COMM_C           = NL2_TEAM_FILE_ID + 707,

    /* LTE和NR公共文件, 放到最后,从900开始 */
    PS_FILE_ID_L2_GLB_MEM_C                 = NL2_TEAM_FILE_ID + 900,
    PS_FILE_ID_L2_ZEOR_COPY_C               = NL2_TEAM_FILE_ID + 901,
    PS_FILE_ID_NUP_THR_C                    = NL2_TEAM_FILE_ID + 902,
    PS_FILE_ID_NUP_HIDS_MSG_PROC_C          = NL2_TEAM_FILE_ID + 903,
    PS_FILE_ID_NUP_DT_C                     = NL2_TEAM_FILE_ID + 904,
	PS_FILE_ID_NPDCP_UL_CDSMSGPROC_C        = NL2_TEAM_FILE_ID + 905,
    PS_FILE_ID_NL2_BUTT
}NL2_FILE_ID_DEFINE_ENUM;
typedef unsigned long  NL2_FILE_ID_DEFINE_ENUM_UINT32;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


