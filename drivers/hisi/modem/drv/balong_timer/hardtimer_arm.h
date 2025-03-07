/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __HARDTIMER_ARM_H__
#define __HARDTIMER_ARM_H__

// ARM timer的寄存器定义
#define TIMER_LOAD_OFFSET 0x0
#define TIMER_VALUE_OFFSET 0x4
#define TIMER_CTRL_OFFSET 0x8
#define TIMER_INTCLR_OFFSET 0xC
#define TIMER_INTRIS_OFFSET 0x10
#define TIMER_INTMIS_OFFSET 0x14
#define TIMER_BGLOAD_OFFSET 0x18

typedef union {
    struct {
        unsigned int oneshot:1;
        unsigned int timersize:1;
        unsigned int timerpre:2;
        unsigned int :1; // reserved
        unsigned int internable:1;
        unsigned int timermode:1;
        unsigned int timeren:1;
        unsigned int :23; // reserved
        unsigned int timer_reload:1;
    };
    unsigned int ctrl_val;
} ctrl_reg_u;

#define IS_TIMER_ENABLE(value) ((value) & 0x80U)
#define TIMER_ENABLE_VAL(value) ((value) | 0x80U)
#define TIMER_DISABLE_VAL(value) ((value) & (~0x80U))
#define TIMER_INT_UNMASK_VAL(value) ((value) | 0x20U)
#define TIMER_INT_MASK_VAL(value) ((value) & (~0x20U))
#define DISABLE_AND_MASK_TIMER(value) ((value) & (~0xA0U))
#define TIMER_ONCE_REG_VAL 0x23U
#define TIMER_PERIOD_REG_VAL 0x62U
#define TIMER_INT_ENABLE 1
#define TIMER_INT_DISABLE 0


#endif
