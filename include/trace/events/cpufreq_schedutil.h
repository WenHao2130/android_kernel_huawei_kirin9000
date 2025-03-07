// SPDX-License-Identifier: GPL-2.0-only
/*
 * cpufreq_schedutil.h
 *
 * schedutil governor trace events
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM cpufreq_schedutil

#if !defined(_TRACE_CPUFREQ_SCHEDUTIL_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_CPUFREQ_SCHEDUTIL_H

#include <linux/tracepoint.h>
#include <linux/version.h>

TRACE_EVENT(cpufreq_schedutil_boost,
	    TP_PROTO(const char *s),
	    TP_ARGS(s),
	    TP_STRUCT__entry(
		    __string(s, s)
	    ),
	    TP_fast_assign(
		    __assign_str(s, s);
	    ),
	    TP_printk("%s", __get_str(s))
);

TRACE_EVENT(cpufreq_schedutil_unboost,
	    TP_PROTO(const char *s),
	    TP_ARGS(s),
	    TP_STRUCT__entry(
		    __string(s, s)
	    ),
	    TP_fast_assign(
		    __assign_str(s, s);
	    ),
	    TP_printk("%s", __get_str(s))
);

TRACE_EVENT(cpufreq_schedutil_eval_target,
	    TP_PROTO(unsigned int cpu,
		     unsigned long util,
		     unsigned long max,
		     unsigned int load,
		     unsigned int curr,
		     unsigned int target),
	    TP_ARGS(cpu, util, max, load, curr, target),
	    TP_STRUCT__entry(
		    __field(unsigned int,	cpu)
		    __field(unsigned long,	util)
		    __field(unsigned long,	max)
		    __field(unsigned int,	load)
		    __field(unsigned int,	curr)
		    __field(unsigned int,	target)
	    ),
	    TP_fast_assign(
		    __entry->cpu = cpu;
		    __entry->util = util;
		    __entry->max = max;
		    __entry->load = load;
		    __entry->curr = curr;
		    __entry->target = target;
	    ),
	    TP_printk("cpu=%u util=%lu max=%lu load=%u cur=%u target=%u",
		      __entry->cpu, __entry->util, __entry->max,
		      __entry->load, __entry->curr, __entry->target)
);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
TRACE_EVENT(cpufreq_schedutil_get_util,
	    TP_PROTO(unsigned int cpu,
		     unsigned long util,
		     unsigned long max,
		     unsigned long top,
		     unsigned long pred,
		     unsigned long max_pred,
		     unsigned long cpu_pred,
		     unsigned int iowait,
		     unsigned int flag,
		     unsigned int ed,
		     unsigned int od),
	    TP_ARGS(cpu, util, max, top, pred, max_pred, cpu_pred, iowait, flag, ed, od),
	    TP_STRUCT__entry(
		    __field(unsigned int,	cpu)
		    __field(unsigned long,	util)
		    __field(unsigned long,	max)
		    __field(unsigned long,	top)
		    __field(unsigned long,	pred)
		    __field(unsigned long,	max_pred)
		    __field(unsigned long,	cpu_pred)
		    __field(unsigned int,	iowait)
		    __field(unsigned int,	flag)
		    __field(unsigned int,	ed)
		    __field(unsigned int,	od)
	    ),
	    TP_fast_assign(
		    __entry->cpu = cpu;
		    __entry->util = util;
		    __entry->max = max;
		    __entry->top = top;
		    __entry->pred = pred;
		    __entry->max_pred = max_pred;
		    __entry->cpu_pred = cpu_pred;
		    __entry->iowait = iowait;
		    __entry->flag = flag;
		    __entry->ed = ed;
		    __entry->od = od;
	    ),
	    TP_printk("cpu=%u util=%lu max=%lu top=%lu pred=%lu,%lu,%lu iowait=%u flag=%u ed=%u od=%u",
		      __entry->cpu, __entry->util, __entry->max, __entry->top, __entry->pred,
		      __entry->max_pred, __entry->cpu_pred, __entry->iowait, __entry->flag,
		      __entry->ed, __entry->od)
);
#else
TRACE_EVENT(cpufreq_schedutil_get_util,
	    TP_PROTO(unsigned int cpu,
		     unsigned long util,
		     unsigned long max,
		     unsigned long top,
		     unsigned long pred,
		     unsigned long max_pred,
		     unsigned long cpu_pred,
		     unsigned int iowait,
		     unsigned int min_util,
		     unsigned int flag,
		     unsigned int use_max),
	    TP_ARGS(cpu, util, max, top, pred, max_pred, cpu_pred, iowait, min_util, flag, use_max),
	    TP_STRUCT__entry(
		    __field(unsigned int,	cpu)
		    __field(unsigned long,	util)
		    __field(unsigned long,	max)
		    __field(unsigned long,	top)
		    __field(unsigned long,	pred)
		    __field(unsigned long,	max_pred)
		    __field(unsigned long,	cpu_pred)
		    __field(unsigned int,	iowait)
		    __field(unsigned int,	min_util)
		    __field(unsigned int,	flag)
		    __field(unsigned int,	use_max)
	    ),
	    TP_fast_assign(
		    __entry->cpu = cpu;
		    __entry->util = util;
		    __entry->max = max;
		    __entry->top = top;
		    __entry->pred = pred;
		    __entry->max_pred = max_pred;
		    __entry->cpu_pred = cpu_pred;
		    __entry->iowait = iowait;
		    __entry->min_util = min_util;
		    __entry->flag = flag;
		    __entry->use_max = use_max;
	    ),
	    TP_printk("cpu=%u util=%lu max=%lu top=%lu pred=%lu,%lu,%lu iowait=%u min_util=%u flag=%u use_max=%u",
		      __entry->cpu, __entry->util, __entry->max, __entry->top, __entry->pred,
		      __entry->max_pred, __entry->cpu_pred, __entry->iowait, __entry->min_util,
		      __entry->flag, __entry->use_max)
);
#endif

TRACE_EVENT(cpufreq_schedutil_notyet,
	    TP_PROTO(unsigned int cpu,
		     const char *reason,
		     unsigned long long delta,
		     unsigned int curr,
		     unsigned int target),
	    TP_ARGS(cpu, reason, delta, curr, target),
	    TP_STRUCT__entry(
		    __field(unsigned int,	cpu)
		    __string(reason, reason)
		    __field(unsigned long long,	delta)
		    __field(unsigned int,	curr)
		    __field(unsigned int,	target)
	    ),
	    TP_fast_assign(
		    __entry->cpu = cpu;
		    __assign_str(reason, reason);
		    __entry->delta = delta;
		    __entry->curr = curr;
		    __entry->target = target;
	    ),
	    TP_printk("cpu=%u reason=%s delta_ns=%llu curr=%u target=%u",
		      __entry->cpu, __get_str(reason),
		      __entry->delta, __entry->curr, __entry->target)
);

TRACE_EVENT(cpufreq_schedutil_already,
	    TP_PROTO(unsigned int cpu,
		     unsigned int curr,
		     unsigned int target),
	    TP_ARGS(cpu, curr, target),
	    TP_STRUCT__entry(
		    __field(unsigned int,	cpu)
		    __field(unsigned int,	curr)
		    __field(unsigned int,	target)
	    ),
	    TP_fast_assign(
		    __entry->cpu = cpu;
		    __entry->curr = curr;
		    __entry->target = target;
	    ),
	    TP_printk("cpu=%u curr=%u target=%u",
		      __entry->cpu, __entry->curr, __entry->target)
);

#endif /* _TRACE_CPUFREQ_SCHEDUTIL_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
