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

#include "../hds/bsp_hds_log.h"
#include "../hds/bsp_hds_ind.h"
#include "../hds/bsp_hds_service.h"
#include "../include/bsp_slice.h"


void mdrv_hds_cmd_register(unsigned int cmdid, bsp_hds_func fn)
{
    bsp_hds_cmd_register(cmdid, fn);
}

void mdrv_hds_get_cmdlist(unsigned int *cmdlist, unsigned int *num)
{
    bsp_hds_get_cmdlist(cmdlist, num);
}

void mdrv_hds_cnf_register(hds_cnf_func fn)
{
    bsp_hds_cnf_register(fn);
}


int mdrv_hds_msg_proc(void *pstReq)
{
    return bsp_hds_msg_proc((diag_frame_head_s *)pstReq);
}


#if (FEATURE_HDS_TRANSLOG == FEATURE_ON)

int mdrv_hds_translog_conn(void)
{
    return bsp_hds_translog_conn();
}

int mdrv_hds_translog_disconn(void)
{
    return bsp_hds_translog_disconn();
}

#else
int mdrv_hds_translog_conn(void)
{
    return 0;
}

int mdrv_hds_translog_disconn(void)
{
    return 0;
}
#endif

#if (FEATURE_HDS_PRINTLOG == FEATURE_ON)

int mdrv_hds_printlog_conn(void)
{
    return bsp_hds_printlog_conn();
}

int mdrv_hds_printlog_disconn(void)
{
    return bsp_hds_printlog_disconn();
}

#else
int mdrv_hds_printlog_conn(void)
{
    return 0;
}

int mdrv_hds_printlog_disconn(void)
{
    return 0;
}

#endif





