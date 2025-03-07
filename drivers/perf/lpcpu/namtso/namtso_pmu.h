/*
 * namtso_pmu.h
 *
 * device driver data structure define of namtso pmu driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef __NAMTSO_PMU_H__
#define __NAMTSO_PMU_H__

#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/types.h>

/* include cycle counter */
#define NAMTSO_PMU_MAX_HW_CNTRS				32
#define NAMTSO_PMU_MAX_COMMON_EVENTS				0x40

#define NAMTSO_PMEVCNTR0_OFFSET				0x0
#define NAMTSO_PMCCNTR_L_OFFSET				0x0F8
#define NAMTSO_PMCCNTR_H_OFFSET				0x0FC
#define NAMTSO_PMEVTYPER0_OFFSET				0x400
#define NAMTSO_PMCNTENSET_OFFSET				0xC00
#define NAMTSO_PMCNTENCLR_OFFSET				0xC20
#define NAMTSO_PMINTENSET_OFFSET				0xC40
#define NAMTSO_PMINTENCLR_OFFSET				0xC60
#define NAMTSO_PMOVSCLR_OFFSET					0xC80
#define NAMTSO_PMOVSSET_OFFSET					0xCC0
#define NAMTSO_PMCR_OFFSET					0xE04
#define NAMTSO_PMCEID0_OFFSET					0xE20
#define NAMTSO_PMCEID1_OFFSET					0xE24

#define PMEVCNTR_STEP						0x8
#define PMEVTYPER_STEP						0x4

#define NAMTSO_PMCR_N_SHIFT					11
#define NAMTSO_PMCR_N_MASK					0x1F

#define NAMTSO_PMCR_EN						BIT(0)

/* PMU event codes */
#define NAMTSO_PMU_EVT_CYCLES					0x11

/*
 * We use the index of the counters as they appear in the counter
 * bit maps in the PMU registers (e.g CLUSTERPMSELR).
 * i.e,
 *	counter 0	- Bit 0
 *	counter 1	- Bit 1
 *	...
 *	Cycle counter	- Bit 31
 */
#define NAMTSO_PMU_IDX_CYCLE_COUNTER				31

#define NAMTSO_ACTIVE_CPU_MASK					0x0
#define NAMTSO_ASSOCIATED_CPU_MASK				0x1

#define to_namtso_pmu(p)	(container_of(p, struct namtso_pmu, pmu))

#define pmu_get_eventid(ev) (ev->hw.config_base & 0xfff)

#define for_each_sibling_event(sibling, event)			\
	if ((event)->group_leader == (event))			\
		list_for_each_entry((sibling), &(event)->sibling_list, sibling_list)

struct namtso_hw_events {
	struct perf_event *events[NAMTSO_PMU_MAX_HW_CNTRS];
	DECLARE_BITMAP(used_mask, NAMTSO_PMU_MAX_HW_CNTRS);
};

/* Generic pmu struct for different pmu types */
struct namtso_pmu {
	struct pmu pmu;
	struct namtso_hw_events hw_events;
	/* associated_cpus: All CPUs associated with the PMU */
	cpumask_t associated_cpus;
	/* CPU used for counting */
	cpumask_t active_cpu;
	int irq;
	struct device *dev;
	raw_spinlock_t pmu_lock;
	struct hlist_node cpuhp_node;
	void __iomem *base;
	/* the ID of the PMU modules */
	u32 index_id;
	int num_counters;
	struct notifier_block cpu_pm_nb;
	bool fcm_idle;
	spinlock_t fcm_idle_lock;
	/* check event code range */
	u64 check_event;
	DECLARE_BITMAP(cpmceid_bitmap, NAMTSO_PMU_MAX_COMMON_EVENTS);
};

bool namtso_pmu_counter_valid(struct namtso_pmu *namtso_pmu, int idx);
#ifdef CONFIG_LPCPU_MULTIDRV_CPUIDLE
extern bool lpcpu_fcm_cluster_pwrdn(void);
#else
static inline bool lpcpu_fcm_cluster_pwrdn(void) {return 0; }
#endif

#endif /* __NAMTSO_PMU_H__ */
