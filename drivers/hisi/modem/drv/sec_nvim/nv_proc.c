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

#include <product_config.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <osl_types.h>
#include "nv_ctrl.h"
#include "nv_partition_upgrade.h"
/*lint -save -e438 -e715*/

ssize_t modem_nv_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    return (ssize_t)len;
}
ssize_t modem_nv_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    unsigned int ret;

    ret = nv_upgrade_set_flag((bool)true);
    if (ret)
        return -EIO;
#ifdef BSP_CONFIG_PHONE_TYPE
    (void)nv_set_coldpatch_upgrade_flag((bool)true);
#endif
    nv_record("modify upgrade flag success !\n");
    /*lint -save -e713*/
    return len;
    /*lint -restore*/
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static const struct proc_ops g_modem_nv = {
    .proc_read = modem_nv_read,
    .proc_write = modem_nv_write,
};
#else
static const struct file_operations g_modem_nv = {
    .owner = THIS_MODULE, /*lint !e64*/
    .read = modem_nv_read,
    .write = modem_nv_write,
};
#endif
/*lint !e785*/
int modemNv_ProcInit(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    if (proc_create("ModemNv", 0660, NULL, &g_modem_nv) == NULL) {
        return -1;
    }
#endif
    return 0;
}
/*lint -restore*/
