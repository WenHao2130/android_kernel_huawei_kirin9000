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

#include <mali_kbase.h>
#include <mali_kbase_ctx_sched.h>
#include <backend/gpu/mali_kbase_device_internal.h>
#include <backend/gpu/mali_kbase_irq_internal.h>
#include <backend/gpu/mali_kbase_pm_internal.h>

/* Waiting timeout for GPU reset to complete */
#define GPU_RESET_TIMEOUT_MS (5000) /* 5 seconds */

static void kbase_csf_debug_dump_registers(struct kbase_device *kbdev)
{
	kbase_io_history_dump(kbdev);

	dev_err(kbdev->dev, "Register state:");
	dev_err(kbdev->dev, "  GPU_IRQ_RAWSTAT=0x%08x   GPU_STATUS=0x%08x  MCU_STATUS=0x%08x",
		kbase_reg_read(kbdev, GPU_CONTROL_REG(GPU_IRQ_RAWSTAT)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(GPU_STATUS)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(MCU_STATUS)));
	dev_err(kbdev->dev, "  JOB_IRQ_RAWSTAT=0x%08x   MMU_IRQ_RAWSTAT=0x%08x   GPU_FAULTSTATUS=0x%08x",
		kbase_reg_read(kbdev, JOB_CONTROL_REG(JOB_IRQ_RAWSTAT)),
		kbase_reg_read(kbdev, MMU_REG(MMU_IRQ_RAWSTAT)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(GPU_FAULTSTATUS)));
	dev_err(kbdev->dev, "  GPU_IRQ_MASK=0x%08x   JOB_IRQ_MASK=0x%08x   MMU_IRQ_MASK=0x%08x",
		kbase_reg_read(kbdev, GPU_CONTROL_REG(GPU_IRQ_MASK)),
		kbase_reg_read(kbdev, JOB_CONTROL_REG(JOB_IRQ_MASK)),
		kbase_reg_read(kbdev, MMU_REG(MMU_IRQ_MASK)));
	dev_err(kbdev->dev, "  PWR_OVERRIDE0=0x%08x   PWR_OVERRIDE1=0x%08x",
		kbase_reg_read(kbdev, GPU_CONTROL_REG(PWR_OVERRIDE0)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(PWR_OVERRIDE1)));
	dev_err(kbdev->dev, "  SHADER_CONFIG=0x%08x   L2_MMU_CONFIG=0x%08x   TILER_CONFIG=0x%08x",
		kbase_reg_read(kbdev, GPU_CONTROL_REG(SHADER_CONFIG)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(L2_MMU_CONFIG)),
		kbase_reg_read(kbdev, GPU_CONTROL_REG(TILER_CONFIG)));
}

static int kbase_csf_reset_gpu_now(struct kbase_device *kbdev)
{
	unsigned long flags;
	bool silent = false;
	int err;

	if (atomic_read(&kbdev->csf.reset.state) == KBASE_CSF_RESET_GPU_SILENT)
		silent = true;

	WARN_ON(kbdev->irq_reset_flush);

	/* Reset the scheduler state before disabling the interrupts as suspend of active
	 * CSG slots would also be done as a part of reset.
	 */
	kbase_csf_scheduler_reset(kbdev);

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	spin_lock(&kbdev->mmu_mask_change);
	kbase_pm_reset_start_locked(kbdev);

	/* We're about to flush out the IRQs and their bottom halves */
	kbdev->irq_reset_flush = true;

	/* Disable IRQ to avoid IRQ handlers to kick in after releasing the
	 * spinlock; this also clears any outstanding interrupts
	 */
	kbase_pm_disable_interrupts_nolock(kbdev);

	spin_unlock(&kbdev->mmu_mask_change);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	/* Ensure that any IRQ handlers have finished
	 * Must be done without any locks IRQ handlers will take.
	 */
	kbase_synchronize_irqs(kbdev);

	/* Flush out any in-flight work items */
	kbase_flush_mmu_wqs(kbdev);

	/* The flush has completed so reset the active indicator */
	kbdev->irq_reset_flush = false;

	mutex_lock(&kbdev->pm.lock);
	if (!silent)
		dev_err(kbdev->dev, "Resetting GPU (allowing up to %d ms)",
								RESET_TIMEOUT);

	/* Output the state of some interesting registers to help in the
	 * debugging of GPU resets
	 */
	if (!silent)
		kbase_csf_debug_dump_registers(kbdev);

	/* Reset the GPU */
	err = kbase_pm_init_hw(kbdev, 0);

	mutex_unlock(&kbdev->pm.lock);

	if (WARN_ON(err))
		return err;

	mutex_lock(&kbdev->mmu_hw_mutex);
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	kbase_ctx_sched_restore_all_as(kbdev);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
	mutex_unlock(&kbdev->mmu_hw_mutex);

	kbase_pm_enable_interrupts(kbdev);

	mutex_lock(&kbdev->pm.lock);
	kbase_pm_reset_complete(kbdev);
	mutex_unlock(&kbdev->pm.lock);

	if (kbdev->csf.reset.complete_callback) {
		err = kbdev->csf.reset.complete_callback(kbdev);

		if (WARN_ON(err))
			return err;
	}

	if (!silent)
		dev_err(kbdev->dev, "Reset complete");

	return 0;
}

static void kbase_csf_reset_gpu_worker(struct work_struct *data)
{
	struct kbase_device *kbdev;
	int err = 0;

	kbdev = container_of(data, struct kbase_device,
					csf.reset.work);

	/* Currently an active reference is taken when CSF firmware is
	 * loaded that is dropped only when unload of firmware is done
	 * on device termination, so we can presume that GPU will be
	 * always be powered up until the power management is updated
	 * for CSF firmware.
	 * Hence kbase_pm_context_active_handle_suspend() shall not return 1
	 * when system suspend is in progress.
	 * Once the power management is updated for CSF GPUs, the below
	 * WARN_ON can be removed.
	 */
	if (!WARN_ON(kbase_pm_context_active_handle_suspend(kbdev,
			KBASE_PM_SUSPEND_HANDLER_DONT_REACTIVATE))) {
		err = kbase_csf_reset_gpu_now(kbdev);
		kbase_pm_context_idle(kbdev);
	}

	kbase_disjoint_state_down(kbdev);

	if (!err) {
		atomic_set(&kbdev->csf.reset.state,
				KBASE_CSF_RESET_GPU_NOT_PENDING);
		kbase_csf_scheduler_enable_tick_timer(kbdev);
	} else {
		dev_err(kbdev->dev, "Reset failed to complete");
		atomic_set(&kbdev->csf.reset.state,
				KBASE_CSF_RESET_GPU_FAILED);
	}

	wake_up(&kbdev->csf.reset.wait);
}

bool kbase_prepare_to_reset_gpu_locked(struct kbase_device *kbdev)
{
	lockdep_assert_held(&kbdev->hwaccess_lock);

	return true;
}

bool kbase_prepare_to_reset_gpu(struct kbase_device *kbdev)
{
	return true;
}
KBASE_EXPORT_TEST_API(kbase_prepare_to_reset_gpu);

int kbase_reset_gpu(struct kbase_device *kbdev)
{
	if (atomic_cmpxchg(&kbdev->csf.reset.state,
				KBASE_CSF_RESET_GPU_NOT_PENDING,
				KBASE_CSF_RESET_GPU_HAPPENING) !=
				KBASE_CSF_RESET_GPU_NOT_PENDING) {
		/* Some other thread is already resetting the GPU */
		return -EAGAIN;
	}

	dev_err(kbdev->dev, "Preparing to soft-reset GPU\n");

	kbase_disjoint_state_up(kbdev);

	queue_work(kbdev->csf.reset.workq, &kbdev->csf.reset.work);

	return 0;
}
KBASE_EXPORT_TEST_API(kbase_reset_gpu);

void kbase_reset_gpu_locked(struct kbase_device *kbdev)
{
	lockdep_assert_held(&kbdev->hwaccess_lock);

	kbase_reset_gpu(kbdev);
}

int kbase_reset_gpu_silent(struct kbase_device *kbdev)
{
	if (atomic_cmpxchg(&kbdev->csf.reset.state,
				KBASE_CSF_RESET_GPU_NOT_PENDING,
				KBASE_CSF_RESET_GPU_SILENT) !=
				KBASE_CSF_RESET_GPU_NOT_PENDING) {
		/* Some other thread is already resetting the GPU */
		return -EAGAIN;
	}

	kbase_disjoint_state_up(kbdev);

	queue_work(kbdev->csf.reset.workq, &kbdev->csf.reset.work);

	return 0;
}

bool kbase_reset_gpu_is_active(struct kbase_device *kbdev)
{
	if (atomic_read(&kbdev->csf.reset.state) ==
			KBASE_CSF_RESET_GPU_NOT_PENDING)
		return false;

	return true;
}

int kbase_reset_gpu_wait(struct kbase_device *kbdev)
{
	const long wait_timeout =
		kbase_csf_timeout_in_jiffies(GPU_RESET_TIMEOUT_MS);
	long remaining = wait_event_timeout(kbdev->csf.reset.wait,
				(atomic_read(&kbdev->csf.reset.state) ==
					KBASE_CSF_RESET_GPU_NOT_PENDING) ||
				(atomic_read(&kbdev->csf.reset.state) ==
					KBASE_CSF_RESET_GPU_FAILED),
				wait_timeout);

	if (!remaining) {
		dev_warn(kbdev->dev, "Timed out waiting for the GPU reset to complete");
		return -ETIMEDOUT;
	} else if (atomic_read(&kbdev->csf.reset.state) ==
			KBASE_CSF_RESET_GPU_FAILED) {
		return -ENOMEM;
	}

	return 0;
}
KBASE_EXPORT_TEST_API(kbase_reset_gpu_wait);

int kbase_reset_gpu_init(struct kbase_device *kbdev)
{
	kbdev->csf.reset.workq = alloc_workqueue("Mali reset workqueue", 0, 1);
	if (kbdev->csf.reset.workq == NULL)
		return -ENOMEM;

	INIT_WORK(&kbdev->csf.reset.work, kbase_csf_reset_gpu_worker);

	init_waitqueue_head(&kbdev->csf.reset.wait);

	return 0;
}

void kbase_reset_gpu_term(struct kbase_device *kbdev)
{
	destroy_workqueue(kbdev->csf.reset.workq);
}

void kbase_reset_gpu_register_complete_cb(struct kbase_device *kbdev,
	int (*complete_callback)(struct kbase_device *kbdev))
{
	kbdev->csf.reset.complete_callback = complete_callback;
}

