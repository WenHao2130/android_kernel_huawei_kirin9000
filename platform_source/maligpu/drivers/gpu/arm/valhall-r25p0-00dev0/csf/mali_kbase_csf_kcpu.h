/*
 *
 * (C) COPYRIGHT 2018-2019 ARM Limited. All rights reserved.
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

#ifndef _KBASE_CSF_KCPU_H_
#define _KBASE_CSF_KCPU_H_

#if (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
#include <linux/fence.h>
#else
#include <linux/dma-fence.h>
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) */

/* The maximum number of KCPU commands in flight, enqueueing more commands
 * than this value shall block.
 */
#define KBASEP_KCPU_QUEUE_SIZE ((size_t)256)

/**
 * struct kbase_kcpu_command_import_info - Structure which holds information
 *				about the buffer to be imported
 *
 * @gpu_va:	Address of the buffer to be imported.
 */
struct kbase_kcpu_command_import_info {
	u64 gpu_va;
};

/**
 * struct kbase_kcpu_command_fence_info - Structure which holds information
 *		about the fence object enqueued in the kcpu command queue
 *
 * @fence_cb:
 * @fence:
 * @kcpu_queue:
 */
struct kbase_kcpu_command_fence_info {
#if (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
	struct fence_cb fence_cb;
	struct fence *fence;
#else
	struct dma_fence_cb fence_cb;
	struct dma_fence *fence;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) */
	struct kbase_kcpu_command_queue *kcpu_queue;
};

/**
 * struct kbase_kcpu_command_cqs_set_info - Structure which holds information
 *				about CQS objects for the kcpu CQS set command
 *
 * @objs:	Array of structures which define CQS objects to be used by
 *		the kcpu command.
 * @nr_objs:	Number of CQS objects in the array.
 */
struct kbase_kcpu_command_cqs_set_info {
	struct base_cqs_set *objs;
	unsigned int nr_objs;
};

/**
 * struct kbase_kcpu_command_cqs_wait_info - Structure which holds information
 *				about CQS objects for the kcpu CQS wait command
 *
 * @objs:	Array of structures which define CQS objects to be used by
 *		the kcpu command.
 * @nr_objs:	Number of CQS objects in the array.
 */
struct kbase_kcpu_command_cqs_wait_info {
	struct base_cqs_wait *objs;
	unsigned int nr_objs;
};

/**
 * struct kbase_kcpu_command_jit_alloc_info - Structure which holds information
 *				needed for the kcpu command for jit allocations
 *
 * @info:	Array of objects of the struct base_jit_alloc_info type which
 *		specify jit allocations to be made by the kcpu command.
 * @count:	Number of jit alloc objects in the array.
 */
struct kbase_kcpu_command_jit_alloc_info {
	struct base_jit_alloc_info *info;
	u8 count;
};

/**
 * struct kbase_kcpu_command_jit_free_info - Structure which holds information
 *				needed for the kcpu jit free command
 *
 * @ids:	Array of identifiers of jit allocations which are to be freed
 *		by the kcpu command.
 * @count:	Number of elements in the array.
 */
struct kbase_kcpu_command_jit_free_info {
	u8 *ids;
	u8 count;
};

/**
 * struct kbase_kcpu_command_debug_copy_info - Structure which holds information
 *				about buffers which are to be copied by
 *				the kcpu command
 *
 * @buffers:	Array of buffers of the type struct kbase_debug_copy_buffer
 *		which contains information about each buffer.
 * @nr:		Number of buffers to be copied.
 */
struct kbase_kcpu_command_debug_copy_info {
	struct kbase_debug_copy_buffer *buffers;
	unsigned int nr;
};

/**
 * struct kbase_suspend_copy_buffer - information about the suspend buffer
 *		to be copied.
 *
 * @size:	size of the suspend buffer in bytes.
 * @pages:	pointer to an array of pointers to the pages which contain
 *		the user buffer.
 * @nr_pages:	number of pages.
 * @offset:	offset into the pages
 */
struct kbase_suspend_copy_buffer {
	size_t size;
	struct page **pages;
	int nr_pages;
	size_t offset;
};

/**
 * struct base_kcpu_command_group_suspend - structure which contains
 *		suspend buffer data captured for a suspended queue group.
 *
 * @sus_buf:		Pointer to the structure which contains details of the
 *			user buffer and its kernel pinned pages.
 * @group_handle:	Handle to the mapping of command stream group.
 */
struct kbase_kcpu_command_group_suspend_info {
	struct kbase_suspend_copy_buffer *sus_buf;
	u8 group_handle;
};

/**
 * struct kbase_cpu_command - Command which is to be part of the kernel
 *                            command queue
 *
 * @type:	Type of the command.
 * @info:	Structure which holds information about the command
 *              dependent on the command type.
 */
struct kbase_kcpu_command {
	u32 type;
	union {
		struct kbase_kcpu_command_fence_info fence;
		struct kbase_kcpu_command_cqs_wait_info cqs_wait;
		struct kbase_kcpu_command_cqs_set_info cqs_set;
		struct kbase_kcpu_command_debug_copy_info debug_copy;
		struct kbase_kcpu_command_import_info import;
		struct kbase_kcpu_command_jit_alloc_info jit_alloc;
		struct kbase_kcpu_command_jit_free_info jit_free;
		struct kbase_kcpu_command_group_suspend_info suspend_buf_copy;
	} info;
};

/**
 * struct kbase_kcpu_command_queue - a command queue executed by the kernel
 *
 * @kctx:			The context to which this command queue belongs.
 * @commands:			Array of commands which have been successfully
 *				enqueued to this command queue.
 * @work:			struct work_struct which contains a pointer to
 *				the function which handles processing of kcpu
 *				commands enqueued into a kcpu command queue;
 *				part of kernel API for processing workqueues
 * @start_offset:		Index of the command to be executed next
 * @num_pending_cmds:		The number of commands enqueued but not yet
 *				executed or pending
 * @cqs_wait_count:		Tracks the number of CQS wait commands enqueued
 * @fence_context:		The dma-buf fence context number for this kcpu
 *				queue. A unique context number is allocated for
 *				each kcpu queue.
 * @fence_seqno:		The dma-buf fence sequence number for the fence
 *				that is returned on the enqueue of fence signal
 *				command. This is increased every time the
 *				fence signal command is queued.
 * @fence_wait_processed:	Used to avoid reprocessing of the fence wait
 *				command which has blocked the processing of
 *				commands that follow it.
 * @fence_signaled:		Indicates whether the fence, due to which the
 *				fence wait command was blocked, has been
 *				signaled or not.
 * @enqueue_failed:	Indicates that no space has become available in the
 *				buffer since an enqueue operation failed
 *				because of insufficient free space.
 * @command_started:		Indicates that the command at the front of the
 *				queue has been started in a previous queue
 *				process, but was not completed due to some
 *				unmet dependencies. Ensures that instrumentation
 *				of the execution start of these commands is only
 *				fired exactly once.
 */
struct kbase_kcpu_command_queue {
	struct kbase_context *kctx;
	struct kbase_kcpu_command commands[KBASEP_KCPU_QUEUE_SIZE];
	struct work_struct work;
	u8 start_offset;
	u16 num_pending_cmds;
	u32 cqs_wait_count;
	u64 fence_context;
	unsigned int fence_seqno;
	bool fence_wait_processed;
	bool fence_signaled;
	bool enqueue_failed;
	bool command_started;
};

/**
 * kbase_csf_kcpu_queue_new - Create new KCPU command queue.
 *
 * @kctx:	Pointer to the kbase context within which the KCPU command
 *		queue will be created.
 * @newq:	Pointer to the structure which contains information about
 *		the new KCPU command queue to be created.
 */
int kbase_csf_kcpu_queue_new(struct kbase_context *kctx,
			 struct kbase_ioctl_kcpu_queue_new *newq);

/**
 * kbase_csf_kcpu_queue_delete - Delete KCPU command queue.
 *
 * Return: 0 if successful, -EINVAL if the queue ID is invalid.
 *
 * @kctx:	Pointer to the kbase context from which the KCPU command
 *		queue is to be deleted.
 * @del:	Pointer to the structure which specifies the KCPU command
 *		queue to be deleted.
 */
int kbase_csf_kcpu_queue_delete(struct kbase_context *kctx,
			    struct kbase_ioctl_kcpu_queue_delete *del);

/**
 * kbase_csf_kcpu_queue_enqueue - Enqueue a KCPU command into a KCPU command
 *				  queue.
 *
 * @kctx:	Pointer to the kbase context within which the KCPU command
 *		is to be enqueued into the KCPU command queue.
 * @enq:	Pointer to the structure which specifies the KCPU command
 *		as well as the KCPU command queue into which the command
 *		is to be enqueued.
 */
int kbase_csf_kcpu_queue_enqueue(struct kbase_context *kctx,
				 struct kbase_ioctl_kcpu_queue_enqueue *enq);

/**
 * kbase_csf_kcpu_queue_context_init - Initialize the kernel CPU queues context
 *                                     for a GPU address space
 *
 * @kctx: Pointer to the kbase context being initialized.
 *
 * Return: 0 if successful or a negative error code on failure.
 */
int kbase_csf_kcpu_queue_context_init(struct kbase_context *kctx);

/**
 * kbase_csf_kcpu_queue_context_term - Terminate the kernel CPU queues context
 *                                     for a GPU address space
 *
 * This function deletes any kernel CPU queues that weren't deleted before
 * context termination.
 *
 * @kctx: Pointer to the kbase context being terminated.
 */
void kbase_csf_kcpu_queue_context_term(struct kbase_context *kctx);

#endif /* _KBASE_CSF_KCPU_H_ */
