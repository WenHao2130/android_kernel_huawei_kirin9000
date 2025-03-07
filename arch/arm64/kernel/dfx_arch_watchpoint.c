/*
 *
 * watch point module
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/compat.h>
#include <linux/cpu_pm.h>
#include <linux/errno.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/ptrace.h>
#include <linux/smp.h>

#include <asm/debug-monitors.h>
#include <asm/hw_breakpoint.h>
#include <asm/kdebug.h>
#include <asm/traps.h>
#include <asm/cputype.h>
#include <asm/system_misc.h>


#define READ_WB_REG_CASE(OFF, N, REG, VAL)	\
	case (OFF + N):				\
		AARCH64_DBG_READ(N, REG, VAL);	\
		break

#define GEN_READ_WB_REG_CASES(OFF, REG, VAL)	\
	READ_WB_REG_CASE(OFF,  0, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  1, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  2, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  3, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  4, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  5, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  6, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  7, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  8, REG, VAL);	\
	READ_WB_REG_CASE(OFF,  9, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 10, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 11, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 12, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 13, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 14, REG, VAL);	\
	READ_WB_REG_CASE(OFF, 15, REG, VAL)

static u64 read_wb_reg(int reg, int n)
{
	u64 val = 0;

	switch (reg + n) {
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BVR, AARCH64_DBG_REG_NAME_BVR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BCR, AARCH64_DBG_REG_NAME_BCR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WVR, AARCH64_DBG_REG_NAME_WVR, val);
	GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WCR, AARCH64_DBG_REG_NAME_WCR, val);
	default:
		pr_warn("attempt to read from unknown breakpoint register %d\n", n);
	}

	return val;
}

#ifdef CONFIG_HAVE_HW_BREAKPOINT_ADDR_MASK
/*
 * arch addr mask support
 */
int arch_has_hw_breakpoint_addr_mask(void)
{
	return 1;
}
#endif

/*
 * arch read watchpoint value register
 */
void arch_read_wvr(u64 *buf, u32 len)
{
	int i;
	int core_num_wrps = hw_breakpoint_slots(TYPE_DATA);

	if (buf == NULL) {
		pr_err("wvr buf is invalid\n");
		return;
	}

	if ( len < (unsigned int)core_num_wrps )
		pr_err(" buf size out of memory\n");

	for (i = 0; i < core_num_wrps; ++i)
		buf[i] = read_wb_reg(AARCH64_DBG_REG_WVR, i);
}

/*
 * arch read watchpoint control register
 */
void arch_read_wcr(u64 *buf, u32 len)
{
	int i;
	int core_num_wrps = hw_breakpoint_slots(TYPE_DATA);

	if (buf == NULL) {
		pr_err("wcr buf is invalid\n");
		return;
	}

	if ( len < (unsigned int)core_num_wrps )
		pr_err(" buf size out of memory\n");

	for (i = 0; i < core_num_wrps; ++i)
		buf[i] = read_wb_reg(AARCH64_DBG_REG_WCR, i);
}
