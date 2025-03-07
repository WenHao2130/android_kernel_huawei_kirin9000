/*
 *
 * (C) COPYRIGHT 2019-2020 ARM Limited. All rights reserved.
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

#ifndef _KBASE_CSF_SCHEDULER_H_
#define _KBASE_CSF_SCHEDULER_H_

#include "mali_kbase_csf.h"

/**
 * kbase_csf_scheduler_queue_start() - Enable the running of GPU command queue
 *                                     on firmware.
 *
 * @queue: Pointer to the GPU command queue to be started.
 *
 * This function would enable the start of a command stream interface, within a
 * command stream group, to which the @queue was bound.
 * If the command stream group is already scheduled and resident, the command
 * stream interface will be started right away, otherwise once the group is
 * made resident.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_scheduler_queue_start(struct kbase_queue *queue);

/**
 * kbase_csf_scheduler_queue_stop() - Disable the running of GPU command queue
 *                                    on firmware.
 *
 * @queue: Pointer to the GPU command queue to be stopped.
 *
 * This function would stop the command stream interface, within a command
 * stream group, to which the @queue was bound.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_scheduler_queue_stop(struct kbase_queue *queue);

/**
 * kbase_csf_scheduler_group_protm_enter - Handle the protm enter event for the
 *                                         GPU command queue group.
 *
 * @group: The command queue group.
 *
 * This function could request the firmware to enter the protected mode
 * and allow the execution of protected region instructions for all the
 * bound queues of the group that have protm pending bit set in their
 * respective CS_ACK register.
 */
void kbase_csf_scheduler_group_protm_enter(struct kbase_queue_group *group);

/**
 * kbase_csf_scheduler_group_get_slot() - Checks if a queue group is
 *                           programmed on a firmware Command Stream Group slot
 *                           and returns the slot number.
 *
 * @group: The command queue group.
 *
 * Return: The slot number, if the group is programmed on a slot.
 *         Otherwise returns a negative number.
 *
 * Note: This function should not be used if the interrupt_lock is held. Use
 * kbase_csf_scheduler_group_get_slot_locked() instead.
 */
int kbase_csf_scheduler_group_get_slot(struct kbase_queue_group *group);

/**
 * kbase_csf_scheduler_group_get_slot_locked() - Checks if a queue group is
 *                           programmed on a firmware Command Stream Group slot
 *                           and returns the slot number.
 *
 * @group: The command queue group.
 *
 * Return: The slot number, if the group is programmed on a slot.
 *         Otherwise returns a negative number.
 *
 * Note: Caller must hold the interrupt_lock.
 */
int kbase_csf_scheduler_group_get_slot_locked(struct kbase_queue_group *group);

/**
 * kbase_csf_scheduler_group_events_enabled() - Checks if interrupt events
 *                                     should be handled for a queue group.
 *
 * @kbdev: The device of the group.
 * @group: The queue group.
 *
 * Return: true if interrupt events should be handled.
 *
 * Note: Caller must hold the interrupt_lock.
 */
bool kbase_csf_scheduler_group_events_enabled(struct kbase_device *kbdev,
		struct kbase_queue_group *group);

/**
 * kbase_csf_scheduler_get_group_on_slot()- Gets the queue group that has been
 *                          programmed to a firmware Command Stream Group slot.
 *
 * @kbdev: The GPU device.
 * @slot:  The slot for which to get the queue group.
 *
 * Return: Pointer to the programmed queue group.
 *
 * Note: Caller must hold the interrupt_lock.
 */
struct kbase_queue_group *kbase_csf_scheduler_get_group_on_slot(
		struct kbase_device *kbdev, int slot);

/**
 * kbase_csf_scheduler_group_deschedule() - Deschedule a GPU command queue
 *                                          group from the firmware.
 *
 * @group: Pointer to the queue group to be scheduled.
 *
 * This function would disable the scheduling of GPU command queue group on
 * firmware.
 */
void kbase_csf_scheduler_group_deschedule(struct kbase_queue_group *group);

/**
 * kbase_csf_scheduler_evict_ctx_slots() - Evict all GPU command queue groups
 *                                         of a given context that are active
 *                                         running from the firmware.
 *
 * @kbdev:          The GPU device.
 * @kctx:           Kbase context for the evict operation.
 * @evicted_groups: List_head for returning evicted active queue groups.
 *
 * This function would disable the scheduling of GPU command queue groups active
 * on firmware slots from the given Kbase context. The affected groups are
 * added to the supplied list_head argument.
 */
void kbase_csf_scheduler_evict_ctx_slots(struct kbase_device *kbdev,
		struct kbase_context *kctx, struct list_head *evicted_groups);

/**
 * kbase_csf_scheduler_context_init() - Initialize the context-specific part
 *                                      for CSF scheduler.
 *
 * @kctx: Pointer to kbase context that is being created.
 *
 * This function must be called during Kbase context creation.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_scheduler_context_init(struct kbase_context *kctx);

/**
 * kbase_csf_scheduler_init - Initialize the CSF scheduler
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * The scheduler does the arbitration for the command stream group slots
 * provided by the firmware between the GPU command queue groups created
 * by the Clients.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_scheduler_init(struct kbase_device *kbdev);

/**
 * kbase_csf_scheduler_context_init() - Terminate the context-specific part
 *                                      for CSF scheduler.
 *
 * @kctx: Pointer to kbase context that is being terminated.
 *
 * This function must be called during Kbase context termination.
 */
void kbase_csf_scheduler_context_term(struct kbase_context *kctx);

/**
 * kbase_csf_scheduler_term - Terminate the CSF scheduler.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * This should be called when unload of firmware is done on device
 * termination.
 */
void kbase_csf_scheduler_term(struct kbase_device *kbdev);

/**
 * kbase_csf_scheduler_reset - Reset the state of all active GPU command
 *                             queue groups.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * This function will first iterate through all the active/scheduled GPU
 * command queue groups and suspend them (to avoid losing work for groups
 * that are not stuck). The groups that could not get suspended would be
 * descheduled and marked as terminated (which will then lead to unbinding
 * of all the queues bound to them) and also no more work would be allowed
 * to execute for them.
 *
 * This is similar to the action taken in response to an unexpected OoM event.
 * No explicit re-initialization is done for CSG & CS interface I/O pages;
 * instead, that happens implicitly on firmware reload.
 *
 * Should be called only after initiating the GPU reset.
 */
void kbase_csf_scheduler_reset(struct kbase_device *kbdev);

/**
 * kbase_csf_scheduler_firmware_reinit - Reinitialize firmware on last stage
 *                                       of GPU reset.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * This function will carry out the re-initialization of the firmware on the
 * GPU platform device that implements a command stream front-end interface.
 * The function should be called after the GPU HW has already reached the ready
 * state in the reset sequence.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_scheduler_firmware_reinit(struct kbase_device *kbdev);

/**
 * kbase_csf_scheduler_enable_tick_timer - Enable the scheduler tick timer.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * This function will restart the scheduler tick so that regular scheduling can
 * be resumed without any explicit trigger (like kicking of GPU queues).
 */
void kbase_csf_scheduler_enable_tick_timer(struct kbase_device *kbdev);

/**
 * kbase_csf_scheduler_group_copy_suspend_buf - Suspend a queue
 *		group and copy suspend buffer.
 *
 * This function is called to suspend a queue group and copy the suspend_buffer
 * contents to the input buffer provided.
 *
 * @group:	Pointer to the queue group to be suspended.
 * @sus_buf:	Pointer to the structure which contains details of the
 *		user buffer and its kernel pinned pages to which we need to copy
 *		the group suspend buffer.
 *
 * Return:	0 on success, or negative on failure.
 */
int kbase_csf_scheduler_group_copy_suspend_buf(struct kbase_queue_group *group,
		struct kbase_suspend_copy_buffer *sus_buf);

/**
 * kbase_csf_scheduler_lock - Acquire the global Scheduler lock.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * This function will take the global scheduler lock, in order to serialize
 * against the Scheduler actions, for access to CS IO pages.
 */
static inline void kbase_csf_scheduler_lock(struct kbase_device *kbdev)
{
	mutex_lock(&kbdev->csf.scheduler.lock);
}

/**
 * kbase_csf_scheduler_unlock - Release the global Scheduler lock.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 */
static inline void kbase_csf_scheduler_unlock(struct kbase_device *kbdev)
{
	mutex_unlock(&kbdev->csf.scheduler.lock);
}

/**
 * kbase_csf_scheduler_timer_is_enabled() - Check if the scheduler wakes up
 * automatically for periodic tasks.
 *
 * @kbdev: Pointer to the device
 *
 * Return: true if the scheduler is configured to wake up periodically
 */
bool kbase_csf_scheduler_timer_is_enabled(struct kbase_device const *kbdev);

/**
 * kbase_csf_scheduler_timer_set_enabled() - Enable/disable periodic
 * scheduler tasks.
 *
 * @kbdev:  Pointer to the device
 * @enable: Whether to enable periodic scheduler tasks
 */
void kbase_csf_scheduler_timer_set_enabled(struct kbase_device *kbdev,
		bool enable);

/**
 * kbase_csf_scheduler_kick - Perform pending scheduling tasks once.
 *
 * Note: This function is only effective if the scheduling timer is disabled.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 */
void kbase_csf_scheduler_kick(struct kbase_device *kbdev);

#endif /* _KBASE_CSF_SCHEDULER_H_ */
