/*
 *
 * (C) COPYRIGHT 2015-2019 ARM Limited. All rights reserved.
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

#if !defined(_KBASE_TIMELINE_H)
#define _KBASE_TIMELINE_H

#include <mali_kbase.h>

/*****************************************************************************/

struct kbase_timeline;

/**
 * kbase_timeline_init - initialize timeline infrastructure in kernel
 * @timeline:            Newly created instance of kbase_timeline will
 *                       be stored in this pointer.
 * @timeline_is_enabled: Timeline status will be written to this variable
 *                       when a client is attached/detached. The variable
 *                       must be valid while timeline instance is valid.
 * Return: zero on success, negative number on error
 */
int kbase_timeline_init(struct kbase_timeline **timeline,
	atomic_t *timeline_is_enabled);

/**
 * kbase_timeline_term - terminate timeline infrastructure in kernel
 *
 * @timeline:     Timeline instance to be terminated. It must be previously created
 *                with kbase_timeline_init().
 */
void kbase_timeline_term(struct kbase_timeline *timeline);

/**
 * kbase_timeline_io_acquire - acquire timeline stream file descriptor
 * @kbdev:     Kbase device
 * @flags:     Timeline stream flags
 *
 * This descriptor is meant to be used by userspace timeline to gain access to
 * kernel timeline stream. This stream is later broadcasted by user space to the
 * timeline client.
 * Only one entity can own the descriptor at any given time. Descriptor shall be
 * closed if unused. If descriptor cannot be obtained (i.e. when it is already
 * being used) return will be a negative value.
 *
 * Return: file descriptor on success, negative number on error
 */
int kbase_timeline_io_acquire(struct kbase_device *kbdev, u32 flags);

/**
 * kbase_timeline_streams_flush - flush timeline streams.
 * @timeline:     Timeline instance
 *
 * Function will flush pending data in all timeline streams.
 */
void kbase_timeline_streams_flush(struct kbase_timeline *timeline);

/**
 * kbase_timeline_streams_body_reset - reset timeline body streams.
 *
 * Function will discard pending data in all timeline body streams.
 * @timeline:     Timeline instance
 */
void kbase_timeline_streams_body_reset(struct kbase_timeline *timeline);

/**
 * kbase_timeline_post_kbase_context_create - Inform timeline that a new KBase
 *                                            Context has been created.
 * @kctx:    KBase Context
 */
void kbase_timeline_post_kbase_context_create(struct kbase_context *kctx);

/**
 * kbase_timeline_pre_kbase_context_destroy - Inform timeline that a KBase
 *                                            Context is about to be destroyed.
 * @kctx:    KBase Context
 */
void kbase_timeline_pre_kbase_context_destroy(struct kbase_context *kctx);

/**
 * kbase_timeline_post_kbase_context_destroy - Inform timeline that a KBase
 *                                             Context has been destroyed.
 * @kctx:    KBase Context
 *
 * Should be called immediately before the memory is freed, and the context ID
 * and kbdev pointer should still be valid.
 */
void kbase_timeline_post_kbase_context_destroy(struct kbase_context *kctx);

#if MALI_UNIT_TEST
/**
 * kbase_timeline_test - start timeline stream data generator
 * @kbdev:     Kernel common context
 * @tpw_count: Number of trace point writers in each context
 * @msg_delay: Time delay in milliseconds between trace points written by one
 *             writer
 * @msg_count: Number of trace points written by one writer
 * @aux_msg:   If non-zero aux messages will be included
 *
 * This test starts a requested number of asynchronous writers in both IRQ and
 * thread context. Each writer will generate required number of test
 * tracepoints (tracepoints with embedded information about writer that
 * should be verified by user space reader). Tracepoints will be emitted in
 * all timeline body streams. If aux_msg is non-zero writer will also
 * generate not testable tracepoints (tracepoints without information about
 * writer). These tracepoints are used to check correctness of remaining
 * timeline message generating functions. Writer will wait requested time
 * between generating another set of messages. This call blocks until all
 * writers finish.
 */
void kbase_timeline_test(
	struct kbase_device *kbdev,
	unsigned int tpw_count,
	unsigned int msg_delay,
	unsigned int msg_count,
	int          aux_msg);

/**
 * kbase_timeline_stats - read timeline stream statistics
 * @timeline:        Timeline instance
 * @bytes_collected: Will hold number of bytes read by the user
 * @bytes_generated: Will hold number of bytes generated by trace points
 */
void kbase_timeline_stats(struct kbase_timeline *timeline, u32 *bytes_collected, u32 *bytes_generated);
#endif /* MALI_UNIT_TEST */

#endif /* _KBASE_TIMELINE_H */
