/*
 *
 * (C) COPYRIGHT 2014-2018 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

/*
 * Backend-specific Power Manager definitions
 */

#ifndef _KBASE_PM_HWACCESS_DEFS_H_
#define _KBASE_PM_HWACCESS_DEFS_H_

#include "mali_kbase_pm_always_on.h"
#include "mali_kbase_pm_coarse_demand.h"
#include "mali_kbase_pm_demand.h"
#if !MALI_CUSTOMER_RELEASE
#include "mali_kbase_pm_demand_always_powered.h"
#include "mali_kbase_pm_fast_start.h"
#endif

/* Forward definition - see mali_kbase.h */
struct kbase_device;
struct kbase_jd_atom;

/**
 * enum kbase_pm_core_type - The types of core in a GPU.
 *
 * These enumerated values are used in calls to
 * - kbase_pm_get_present_cores()
 * - kbase_pm_get_active_cores()
 * - kbase_pm_get_trans_cores()
 * - kbase_pm_get_ready_cores().
 *
 * They specify which type of core should be acted on.  These values are set in
 * a manner that allows core_type_to_reg() function to be simpler and more
 * efficient.
 *
 * @KBASE_PM_CORE_L2: The L2 cache
 * @KBASE_PM_CORE_SHADER: Shader cores
 * @KBASE_PM_CORE_TILER: Tiler cores
 * @KBASE_PM_CORE_STACK: Core stacks
 */
enum kbase_pm_core_type {
	KBASE_PM_CORE_L2 = L2_PRESENT_LO,
	KBASE_PM_CORE_SHADER = SHADER_PRESENT_LO,
	KBASE_PM_CORE_TILER = TILER_PRESENT_LO,
	KBASE_PM_CORE_STACK = STACK_PRESENT_LO
};

/**
 * struct kbasep_pm_metrics - Metrics data collected for use by the power
 *                            management framework.
 *
 *  @time_busy: number of ns the GPU was busy executing jobs since the
 *          @time_period_start timestamp.
 *  @time_idle: number of ns since time_period_start the GPU was not executing
 *          jobs since the @time_period_start timestamp.
 *  @busy_cl: number of ns the GPU was busy executing CL jobs. Note that
 *           if two CL jobs were active for 400ns, this value would be updated
 *           with 800.
 *  @busy_gl: number of ns the GPU was busy executing GL jobs. Note that
 *           if two GL jobs were active for 400ns, this value would be updated
 *           with 800.
 */
struct kbasep_pm_metrics {
	u32 time_busy;
	u32 time_idle;
	u32 busy_cl[2];
	u32 busy_gl;
};

/**
 * struct kbasep_pm_metrics_state - State required to collect the metrics in
 *                                  struct kbasep_pm_metrics
 *  @time_period_start: time at which busy/idle measurements started
 *  @gpu_active: true when the GPU is executing jobs. false when
 *           not. Updated when the job scheduler informs us a job in submitted
 *           or removed from a GPU slot.
 *  @active_cl_ctx: number of CL jobs active on the GPU. Array is per-device.
 *  @active_gl_ctx: number of GL jobs active on the GPU. Array is per-slot. As
 *           GL jobs never run on slot 2 this slot is not recorded.
 *  @lock: spinlock protecting the kbasep_pm_metrics_data structure
 *  @platform_data: pointer to data controlled by platform specific code
 *  @kbdev: pointer to kbase device for which metrics are collected
 *  @values: The current values of the power management metrics. The
 *           kbase_pm_get_dvfs_metrics() function is used to compare these
 *           current values with the saved values from a previous invocation.
 *  @timer: timer to regularly make DVFS decisions based on the power
 *           management metrics.
 *  @timer_active: boolean indicating @timer is running
 *  @dvfs_last: values of the PM metrics from the last DVFS tick
 *  @dvfs_diff: different between the current and previous PM metrics.
 */
struct kbasep_pm_metrics_state {
	int vsync_hit;
	int utilisation;
	int cl_boost;
	ktime_t time_period_start;
	bool gpu_active;
	u32 active_cl_ctx[2];
	u32 active_gl_ctx[2]; /* GL jobs can only run on 2 of the 3 job slots */
	spinlock_t lock;

	void *platform_data;
	struct kbase_device *kbdev;

	struct kbasep_pm_metrics values;

#ifdef CONFIG_MALI_MIDGARD_DVFS
	struct hrtimer timer;
	bool timer_active;
	struct kbasep_pm_metrics dvfs_last;
	struct kbasep_pm_metrics dvfs_diff;
#endif

#ifdef GPU_CL_BOOST
	atomic_t time_compute_jobs, time_vertex_jobs, time_fragment_jobs;
#endif
};

union kbase_pm_policy_data {
	struct kbasep_pm_policy_always_on always_on;
	struct kbasep_pm_policy_coarse_demand coarse_demand;
	struct kbasep_pm_policy_demand demand;
#if !MALI_CUSTOMER_RELEASE
	struct kbasep_pm_policy_demand_always_powered demand_always_powered;
	struct kbasep_pm_policy_fast_start fast_start;
#endif
};

/**
 * struct kbase_pm_backend_data - Data stored per device for power management.
 *
 * This structure contains data for the power management framework. There is one
 * instance of this structure per device in the system.
 *
 * @pm_current_policy: The policy that is currently actively controlling the
 *                     power state.
 * @pm_policy_data:    Private data for current PM policy
 * @ca_in_transition:  Flag indicating when core availability policy is
 *                     transitioning cores. The core availability policy must
 *                     set this when a change in core availability is occurring.
 *                     power_change_lock must be held when accessing this.
 * @reset_done:        Flag when a reset is complete
 * @reset_done_wait:   Wait queue to wait for changes to @reset_done
 * @l2_powered_wait:   Wait queue for whether the l2 cache has been powered as
 *                     requested
 * @l2_powered:        State indicating whether all the l2 caches are powered.
 *                     Non-zero indicates they're *all* powered
 *                     Zero indicates that some (or all) are not powered
 * @gpu_cycle_counter_requests: The reference count of active gpu cycle counter
 *                              users
 * @gpu_cycle_counter_requests_lock: Lock to protect @gpu_cycle_counter_requests
 * @desired_shader_state: A bit mask identifying the shader cores that the
 *                        power policy would like to be on. The current state
 *                        of the cores may be different, but there should be
 *                        transitions in progress that will eventually achieve
 *                        this state (assuming that the policy doesn't change
 *                        its mind in the mean time).
 * @powering_on_shader_state: A bit mask indicating which shader cores are
 *                            currently in a power-on transition
 * @desired_tiler_state: A bit mask identifying the tiler cores that the power
 *                       policy would like to be on. See @desired_shader_state
 * @powering_on_tiler_state: A bit mask indicating which tiler core are
 *                           currently in a power-on transition
 * @powering_on_l2_state: A bit mask indicating which l2-caches are currently
 *                        in a power-on transition
 * @powering_on_stack_state: A bit mask indicating which core stacks are
 *                           currently in a power-on transition
 * @gpu_in_desired_state: This flag is set if the GPU is powered as requested
 *                        by the desired_xxx_state variables
 * @gpu_in_desired_state_wait: Wait queue set when @gpu_in_desired_state != 0
 * @gpu_powered:       Set to true when the GPU is powered and register
 *                     accesses are possible, false otherwise
 * @instr_enabled:     Set to true when instrumentation is enabled,
 *                     false otherwise
 * @cg1_disabled:      Set if the policy wants to keep the second core group
 *                     powered off
 * @driver_ready_for_irqs: Debug state indicating whether sufficient
 *                         initialization of the driver has occurred to handle
 *                         IRQs
 * @gpu_powered_lock:  Spinlock that must be held when writing @gpu_powered or
 *                     accessing @driver_ready_for_irqs
 * @metrics:           Structure to hold metrics for the GPU
 * @gpu_poweroff_pending: number of poweroff timer ticks until the GPU is
 *                        powered off
 * @shader_poweroff_pending_time: number of poweroff timer ticks until shaders
 *                        and/or timers are powered off
 * @gpu_poweroff_timer: Timer for powering off GPU
 * @gpu_poweroff_wq:   Workqueue to power off GPU on when timer fires
 * @gpu_poweroff_work: Workitem used on @gpu_poweroff_wq
 * @shader_poweroff_pending: Bit mask of shaders to be powered off on next
 *                           timer callback
 * @tiler_poweroff_pending: Bit mask of tilers to be powered off on next timer
 *                          callback
 * @poweroff_timer_needed: true if the poweroff timer is currently required,
 *                         false otherwise
 * @poweroff_timer_running: true if the poweroff timer is currently running,
 *                          false otherwise
 *                          power_change_lock should be held when accessing,
 *                          unless there is no way the timer can be running (eg
 *                          hrtimer_cancel() was called immediately before)
 * @poweroff_wait_in_progress: true if a wait for GPU power off is in progress.
 *                             hwaccess_lock must be held when accessing
 * @poweron_required: true if a GPU power on is required. Should only be set
 *                    when poweroff_wait_in_progress is true, and therefore the
 *                    GPU can not immediately be powered on. pm.lock must be
 *                    held when accessing
 * @poweroff_is_suspend: true if the GPU is being powered off due to a suspend
 *                       request. pm.lock must be held when accessing
 * @gpu_poweroff_wait_wq: workqueue for waiting for GPU to power off
 * @gpu_poweroff_wait_work: work item for use with @gpu_poweroff_wait_wq
 * @poweroff_wait: waitqueue for waiting for @gpu_poweroff_wait_work to complete
 * @callback_power_on: Callback when the GPU needs to be turned on. See
 *                     &struct kbase_pm_callback_conf
 * @callback_power_off: Callback when the GPU may be turned off. See
 *                     &struct kbase_pm_callback_conf
 * @callback_power_suspend: Callback when a suspend occurs and the GPU needs to
 *                          be turned off. See &struct kbase_pm_callback_conf
 * @callback_power_resume: Callback when a resume occurs and the GPU needs to
 *                          be turned on. See &struct kbase_pm_callback_conf
 * @callback_power_runtime_on: Callback when the GPU needs to be turned on. See
 *                             &struct kbase_pm_callback_conf
 * @callback_power_runtime_off: Callback when the GPU may be turned off. See
 *                              &struct kbase_pm_callback_conf
 * @callback_power_runtime_idle: Optional callback when the GPU may be idle. See
 *                              &struct kbase_pm_callback_conf
 * @ca_cores_enabled: Cores that are currently available
 *
 * Note:
 * During an IRQ, @pm_current_policy can be NULL when the policy is being
 * changed with kbase_pm_set_policy(). The change is protected under
 * kbase_device.pm.power_change_lock. Direct access to this from IRQ context
 * must therefore check for NULL. If NULL, then kbase_pm_set_policy() will
 * re-issue the policy functions that would have been done under IRQ.
 */
struct kbase_pm_backend_data {
	const struct kbase_pm_policy *pm_current_policy;
	union kbase_pm_policy_data pm_policy_data;
	bool ca_in_transition;
	bool reset_done;
	wait_queue_head_t reset_done_wait;
	wait_queue_head_t l2_powered_wait;
	int l2_powered;
	int gpu_cycle_counter_requests;
	spinlock_t gpu_cycle_counter_requests_lock;

	u64 desired_shader_state;
	u64 powering_on_shader_state;
	u64 desired_tiler_state;
	u64 powering_on_tiler_state;
	u64 powering_on_l2_state;
	u64 desired_l2_state;
#ifdef CONFIG_MALI_CORESTACK
	u64 powering_on_stack_state;
#endif /* CONFIG_MALI_CORESTACK */

	bool gpu_in_desired_state;
	wait_queue_head_t gpu_in_desired_state_wait;

	bool gpu_powered;

	bool instr_enabled;

	bool cg1_disabled;

#ifdef CONFIG_MALI_DEBUG
	bool driver_ready_for_irqs;
#endif /* CONFIG_MALI_DEBUG */

	spinlock_t gpu_powered_lock;


	struct kbasep_pm_metrics_state metrics;

	int gpu_poweroff_pending;
	int shader_poweroff_pending_time;

	struct hrtimer gpu_poweroff_timer;
	struct workqueue_struct *gpu_poweroff_wq;
	struct work_struct gpu_poweroff_work;

	u64 shader_poweroff_pending;
	u64 tiler_poweroff_pending;

	bool poweroff_timer_needed;
	bool poweroff_timer_running;

	bool poweroff_wait_in_progress;
	bool poweron_required;
	bool poweroff_is_suspend;

	struct workqueue_struct *gpu_poweroff_wait_wq;
	struct work_struct gpu_poweroff_wait_work;

	wait_queue_head_t poweroff_wait;

	int (*callback_power_on)(struct kbase_device *kbdev);
	void (*callback_power_off)(struct kbase_device *kbdev);
	void (*callback_power_suspend)(struct kbase_device *kbdev);
	void (*callback_power_resume)(struct kbase_device *kbdev);
	int (*callback_power_runtime_on)(struct kbase_device *kbdev);
	void (*callback_power_runtime_off)(struct kbase_device *kbdev);
	int (*callback_power_runtime_idle)(struct kbase_device *kbdev);

#ifdef CONFIG_MALI_DEVFREQ
	u64 ca_cores_enabled;
#endif
};


/* List of policy IDs */
enum kbase_pm_policy_id {
	KBASE_PM_POLICY_ID_DEMAND = 1,
	KBASE_PM_POLICY_ID_ALWAYS_ON,
	KBASE_PM_POLICY_ID_COARSE_DEMAND,
#if !MALI_CUSTOMER_RELEASE
	KBASE_PM_POLICY_ID_DEMAND_ALWAYS_POWERED,
	KBASE_PM_POLICY_ID_FAST_START
#endif
};

typedef u32 kbase_pm_policy_flags;

/**
 * struct kbase_pm_policy - Power policy structure.
 *
 * Each power policy exposes a (static) instance of this structure which
 * contains function pointers to the policy's methods.
 *
 * @name:               The name of this policy
 * @init:               Function called when the policy is selected
 * @term:               Function called when the policy is unselected
 * @shaders_needed:     Function called to find out if shader cores are needed
 * @get_core_active:    Function called to get the current overall GPU power
 *                      state
 * @flags:              Field indicating flags for this policy
 * @id:                 Field indicating an ID for this policy. This is not
 *                      necessarily the same as its index in the list returned
 *                      by kbase_pm_list_policies().
 *                      It is used purely for debugging.
 */
struct kbase_pm_policy {
	char *name;

	/**
	 * Function called when the policy is selected
	 *
	 * This should initialize the kbdev->pm.pm_policy_data structure. It
	 * should not attempt to make any changes to hardware state.
	 *
	 * It is undefined what state the cores are in when the function is
	 * called.
	 *
	 * @kbdev: The kbase device structure for the device (must be a
	 *         valid pointer)
	 */
	void (*init)(struct kbase_device *kbdev);

	/**
	 * Function called when the policy is unselected.
	 *
	 * @kbdev: The kbase device structure for the device (must be a
	 *         valid pointer)
	 */
	void (*term)(struct kbase_device *kbdev);

	/**
	 * Function called to find out if shader cores are needed
	 *
	 * This needs to at least satisfy kbdev->shader_needed_cnt, and so must
	 * never return false when kbdev->shader_needed_cnt > 0.
	 *
	 * Note that kbdev->pm.active_count being 0 is not a good indicator
	 * that kbdev->shader_needed_cnt is also 0 - refer to the documentation
	 * on the active_count member in struct kbase_pm_device_data and
	 * kbase_pm_is_active().
	 *
	 * @kbdev: The kbase device structure for the device (must be a
	 *         valid pointer)
	 *
	 * Return: true if shader cores are needed, false otherwise
	 */
	bool (*shaders_needed)(struct kbase_device *kbdev);

	/**
	 * Function called to get the current overall GPU power state
	 *
	 * This function must meet or exceed the requirements for power
	 * indicated by kbase_pm_is_active().
	 *
	 * @kbdev: The kbase device structure for the device (must be a
	 *         valid pointer)
	 *
	 * Return: true if the GPU should be powered, false otherwise
	 */
	bool (*get_core_active)(struct kbase_device *kbdev);

	kbase_pm_policy_flags flags;
	enum kbase_pm_policy_id id;
};

#endif /* _KBASE_PM_HWACCESS_DEFS_H_ */
