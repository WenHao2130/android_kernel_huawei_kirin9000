// SPDX-License-Identifier: GPL-2.0
/*
 *
 * (C) COPYRIGHT 2020-2021 ARM Limited. All rights reserved.
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
 * CSF GPU HWC backend firmware interface APIs.
 */

#include <mali_kbase.h>
#include <gpu/mali_kbase_gpu_regmap.h>
#include <device/mali_kbase_device.h>
#include "mali_kbase_hwcnt_gpu.h"
#include "mali_kbase_hwcnt_types.h"
#include "csf/mali_gpu_csf_registers.h"

#include "csf/mali_kbase_csf_firmware.h"
#include "mali_kbase_hwcnt_backend_csf_if_fw.h"
#include "mali_kbase_hwaccess_time.h"
#include "backend/gpu/mali_kbase_clk_rate_trace_mgr.h"

#include <linux/log2.h>
#include "mali_kbase_ccswe.h"
#ifdef CONFIG_MALI_NO_MALI
#include <backend/gpu/mali_kbase_model_dummy.h>
#endif

/** The number of nanoseconds in a second. */
#define NSECS_IN_SEC 1000000000ull /* ns */

/* Ring buffer virtual address start at 4GB  */
#define KBASE_HWC_CSF_RING_BUFFER_VA_START (1ull << 32)

/**
 * struct kbase_hwcnt_backend_csf_if_fw_ring_buf - ring buffer for CSF interface
 *                                                 used to save the manual and
 *                                                 auto HWC samples from
 *                                                 firmware.
 * @gpu_dump_base: Starting GPU base address of the ring buffer.
 * @cpu_dump_base: Starting CPU address for the mapping.
 * @buf_count:     Buffer count in the ring buffer, MUST be power of 2.
 * @as_nr:         Address space number for the memory mapping.
 * @phys:          Physical memory allocation used by the mapping.
 * @num_pages:     Size of the mapping, in memory pages.
 */
struct kbase_hwcnt_backend_csf_if_fw_ring_buf {
	u64 gpu_dump_base;
	void *cpu_dump_base;
	size_t buf_count;
	u32 as_nr;
	struct tagged_addr *phys;
	size_t num_pages;
};

/**
 * struct kbase_hwcnt_backend_csf_if_fw_ctx - Firmware context for the CSF
 *                                            interface, used to communicate
 *                                            with firmware.
 * @kbdev:              KBase device.
 * @buf_bytes:	        The size in bytes for each buffer in the ring buffer.
 * @clk_cnt:            The number of clock domains in the system.
 *                      The maximum is 64.
 * @rate_listener:      Clock rate listener callback state.
 * @ccswe_shader_cores: Shader cores cycle count software estimator.
 */
struct kbase_hwcnt_backend_csf_if_fw_ctx {
	struct kbase_device *kbdev;
	size_t buf_bytes;
	u8 clk_cnt;
	u64 clk_enable_map;
	struct kbase_clk_rate_listener rate_listener;
	struct kbase_ccswe ccswe_shader_cores;
};

/**
 * kbasep_hwcnt_backend_csf_if_fw_on_freq_change() - On freq change callback
 *
 * @rate_listener:    Callback state
 * @clk_index:        Clock index
 * @clk_rate_hz:      Clock frequency(hz)
 */
static void kbasep_hwcnt_backend_csf_if_fw_on_freq_change(
	struct kbase_clk_rate_listener *rate_listener, u32 clk_index,
	u32 clk_rate_hz)
{
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		container_of(rate_listener,
			     struct kbase_hwcnt_backend_csf_if_fw_ctx,
			     rate_listener);
	u64 timestamp_ns;

	if (clk_index != KBASE_CLOCK_DOMAIN_SHADER_CORES)
		return;

	timestamp_ns = ktime_get_raw_ns();
	kbase_ccswe_freq_change(&fw_ctx->ccswe_shader_cores, timestamp_ns,
				clk_rate_hz);
}

/**
 * kbasep_hwcnt_backend_csf_if_fw_cc_enable() - Enable cycle count tracking
 *
 * @fw_ctx:     Non-NULL pointer to CSF firmware interface context.
 * @enable_map: Non-NULL pointer to enable map specifying enabled counters.
 */
static void kbasep_hwcnt_backend_csf_if_fw_cc_enable(
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx, u64 clk_enable_map)
{
	struct kbase_device *kbdev = fw_ctx->kbdev;

	if (kbase_hwcnt_clk_enable_map_enabled(
		    clk_enable_map, KBASE_CLOCK_DOMAIN_SHADER_CORES)) {
		/* software estimation for non-top clock domains */
		struct kbase_clk_rate_trace_manager *rtm = &kbdev->pm.clk_rtm;
		const struct kbase_clk_data *clk_data =
			rtm->clks[KBASE_CLOCK_DOMAIN_SHADER_CORES];
		u32 cur_freq;
		unsigned long flags;
		u64 timestamp_ns;

		timestamp_ns = ktime_get_raw_ns();

		spin_lock_irqsave(&rtm->lock, flags);

		cur_freq = (u32)clk_data->clock_val;
		kbase_ccswe_reset(&fw_ctx->ccswe_shader_cores);
		kbase_ccswe_freq_change(&fw_ctx->ccswe_shader_cores,
					timestamp_ns, cur_freq);

		kbase_clk_rate_trace_manager_subscribe_no_lock(
			rtm, &fw_ctx->rate_listener);

		spin_unlock_irqrestore(&rtm->lock, flags);
	}

	fw_ctx->clk_enable_map = clk_enable_map;
}

/**
 * kbasep_hwcnt_backend_csf_if_fw_cc_disable() - Disable cycle count tracking
 *
 * @fw_ctx:     Non-NULL pointer to CSF firmware interface context.
 */
static void kbasep_hwcnt_backend_csf_if_fw_cc_disable(
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx)
{
	struct kbase_device *kbdev = fw_ctx->kbdev;
	struct kbase_clk_rate_trace_manager *rtm = &kbdev->pm.clk_rtm;
	u64 clk_enable_map = fw_ctx->clk_enable_map;

	if (kbase_hwcnt_clk_enable_map_enabled(clk_enable_map,
					       KBASE_CLOCK_DOMAIN_SHADER_CORES))
		kbase_clk_rate_trace_manager_unsubscribe(
			rtm, &fw_ctx->rate_listener);
}

static void kbasep_hwcnt_backend_csf_if_fw_get_gpu_info(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx, size_t *dump_size,
	size_t *l2_count, u64 *core_mask, u8 *clk_cnt)
{
#ifdef CONFIG_MALI_NO_MALI
	*l2_count = KBASE_DUMMY_MODEL_MAX_MEMSYS_BLOCKS;
	*core_mask = (1ull << KBASE_DUMMY_MODEL_MAX_SHADER_CORES) - 1;
	*dump_size = KBASE_DUMMY_MODEL_MAX_NUM_PERF_BLOCKS *
		     KBASE_DUMMY_MODEL_BLOCK_SIZE;
	*clk_cnt = 1;
#else
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx;
	struct kbase_device *kbdev;
	u32 prfcnt_size;
	u32 prfcnt_hw_size = 0;
	u32 prfcnt_fw_size = 0;

	WARN_ON(!ctx);
	WARN_ON(!dump_size);
	WARN_ON(!l2_count);
	WARN_ON(!core_mask);
	WARN_ON(!clk_cnt);

	fw_ctx = (struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;
	kbdev = fw_ctx->kbdev;
	prfcnt_size = kbdev->csf.global_iface.prfcnt_size;
	prfcnt_hw_size = (prfcnt_size & 0xFF) << 8;
	prfcnt_fw_size = (prfcnt_size >> 16) << 8;
	fw_ctx->buf_bytes = prfcnt_hw_size + prfcnt_fw_size;
	*dump_size = fw_ctx->buf_bytes;

	*l2_count = kbdev->gpu_props.props.l2_props.num_l2_slices;
	*core_mask = kbdev->gpu_props.props.coherency_info.group[0].core_mask;

	*clk_cnt = fw_ctx->clk_cnt;
#endif
}

static int kbasep_hwcnt_backend_csf_if_fw_ring_buf_alloc(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx, u32 buf_count,
	void **cpu_dump_base,
	struct kbase_hwcnt_backend_csf_if_ring_buf **out_ring_buf)
{
	struct kbase_device *kbdev;
	struct tagged_addr *phys;
	struct page **page_list;
	void *cpu_addr;
	int ret;
	int i;
	size_t num_pages;
	u64 flags;
	struct kbase_hwcnt_backend_csf_if_fw_ring_buf *fw_ring_buf;

	pgprot_t cpu_map_prot = PAGE_KERNEL;
	u64 gpu_va_base = KBASE_HWC_CSF_RING_BUFFER_VA_START;

	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);
	WARN_ON(!cpu_dump_base);
	WARN_ON(!out_ring_buf);

	kbdev = fw_ctx->kbdev;

	/* The buffer count must be power of 2 */
	if (!is_power_of_2(buf_count))
		return -EINVAL;

	/* alignment failure */
	if (gpu_va_base & (2048 - 1))
		return -EINVAL;

	fw_ring_buf = kzalloc(sizeof(*fw_ring_buf), GFP_KERNEL);
	if (!fw_ring_buf)
		return -ENOMEM;

	num_pages = PFN_UP(fw_ctx->buf_bytes * buf_count);
	phys = kmalloc_array(num_pages, sizeof(*phys), GFP_KERNEL);
	if (!phys)
		goto phys_alloc_error;

	page_list = kmalloc_array(num_pages, sizeof(*page_list), GFP_KERNEL);
	if (!page_list)
		goto page_list_alloc_error;

	/* Get physical page for the buffer */
	ret = kbase_mem_pool_alloc_pages(
		&kbdev->mem_pools.small[KBASE_MEM_GROUP_CSF_FW], num_pages,
		phys, false);
	if (ret != num_pages)
		goto phys_mem_pool_alloc_error;

	/* Get the CPU virtual address */
	for (i = 0; i < num_pages; i++)
		page_list[i] = as_page(phys[i]);

	cpu_addr = vmap(page_list, num_pages, VM_MAP, cpu_map_prot);
	if (!cpu_addr)
		goto vmap_error;

	flags = KBASE_REG_GPU_WR | KBASE_REG_GPU_NX |
		KBASE_REG_MEMATTR_INDEX(AS_MEMATTR_INDEX_NON_CACHEABLE);

	/* Update MMU table */
	ret = kbase_mmu_insert_pages(kbdev, &kbdev->csf.mcu_mmu,
				     gpu_va_base >> PAGE_SHIFT, phys, num_pages,
				     flags, MCU_AS_NR, KBASE_MEM_GROUP_CSF_FW, num_pages);
	if (ret)
		goto mmu_insert_failed;

	kfree(page_list);

	fw_ring_buf->gpu_dump_base = gpu_va_base;
	fw_ring_buf->cpu_dump_base = cpu_addr;
	fw_ring_buf->phys = phys;
	fw_ring_buf->num_pages = num_pages;
	fw_ring_buf->buf_count = buf_count;
	fw_ring_buf->as_nr = MCU_AS_NR;

	*cpu_dump_base = fw_ring_buf->cpu_dump_base;
	*out_ring_buf =
		(struct kbase_hwcnt_backend_csf_if_ring_buf *)fw_ring_buf;

	return 0;

mmu_insert_failed:
	vunmap(cpu_addr);
vmap_error:
	kbase_mem_pool_free_pages(
		&kbdev->mem_pools.small[KBASE_MEM_GROUP_CSF_FW], num_pages,
		phys, false, false);
phys_mem_pool_alloc_error:
	kfree(page_list);
page_list_alloc_error:
	kfree(phys);
phys_alloc_error:
	kfree(fw_ring_buf);
	return -ENOMEM;
}

static void kbasep_hwcnt_backend_csf_if_fw_ring_buf_sync(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx,
	struct kbase_hwcnt_backend_csf_if_ring_buf *ring_buf,
	u32 buf_index_first, u32 buf_index_last, bool for_cpu)
{
	struct kbase_hwcnt_backend_csf_if_fw_ring_buf *fw_ring_buf =
		(struct kbase_hwcnt_backend_csf_if_fw_ring_buf *)ring_buf;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;
	size_t i;
	size_t pg_first;
	size_t pg_last;
	u64 start_address;
	u64 stop_address;
	u32 ring_buf_index_first;
	u32 ring_buf_index_last;

	WARN_ON(!ctx);
	WARN_ON(!ring_buf);

	/* Get the buffer indexes in the ring buffer. */
	ring_buf_index_first = buf_index_first & (fw_ring_buf->buf_count - 1);
	ring_buf_index_last = buf_index_last & (fw_ring_buf->buf_count - 1);

	/* The start address is the offset of the first buffer. */
	start_address = fw_ctx->buf_bytes * ring_buf_index_first;
	pg_first = start_address >> PAGE_SHIFT;

	/* The stop address is the last byte in the final buffer. */
	stop_address = (fw_ctx->buf_bytes * (ring_buf_index_last + 1)) - 1;
	pg_last = stop_address >> PAGE_SHIFT;

	/* Check whether the buffer range wraps. */
	if (start_address > stop_address) {
		/* sync the first part to the end of ring buffer. */
		for (i = pg_first; i < fw_ring_buf->num_pages; i++) {
			struct page *pg = as_page(fw_ring_buf->phys[i]);
			if (for_cpu) {
				kbase_sync_single_for_cpu(fw_ctx->kbdev,
							  kbase_dma_addr(pg),
							  PAGE_SIZE,
							  DMA_BIDIRECTIONAL);
			} else {
				kbase_sync_single_for_device(fw_ctx->kbdev,
							     kbase_dma_addr(pg),
							     PAGE_SIZE,
							     DMA_BIDIRECTIONAL);
			}
		}

		/* second part starts from page 0. */
		pg_first = 0;
	}

	for (i = pg_first; i <= pg_last; i++) {
		struct page *pg = as_page(fw_ring_buf->phys[i]);
		if (for_cpu) {
			kbase_sync_single_for_cpu(fw_ctx->kbdev,
						  kbase_dma_addr(pg), PAGE_SIZE,
						  DMA_BIDIRECTIONAL);
		} else {
			kbase_sync_single_for_device(fw_ctx->kbdev,
						     kbase_dma_addr(pg),
						     PAGE_SIZE,
						     DMA_BIDIRECTIONAL);
		}
	}
}

static u64 kbasep_hwcnt_backend_csf_if_fw_timestamp_ns(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx)
{
	CSTD_UNUSED(ctx);
	return ktime_get_raw_ns();
}

static void kbasep_hwcnt_backend_csf_if_fw_ring_buf_free(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx,
	struct kbase_hwcnt_backend_csf_if_ring_buf *ring_buf)
{
	struct kbase_hwcnt_backend_csf_if_fw_ring_buf *fw_ring_buf =
		(struct kbase_hwcnt_backend_csf_if_fw_ring_buf *)ring_buf;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	if (!fw_ring_buf)
		return;

	if (fw_ring_buf->phys) {
		u64 gpu_va_base = KBASE_HWC_CSF_RING_BUFFER_VA_START;

		WARN_ON(kbase_mmu_teardown_pages(
			fw_ctx->kbdev, &fw_ctx->kbdev->csf.mcu_mmu,
			gpu_va_base >> PAGE_SHIFT, fw_ring_buf->num_pages,
			MCU_AS_NR));

		vunmap(fw_ring_buf->cpu_dump_base);

		kbase_mem_pool_free_pages(
			&fw_ctx->kbdev->mem_pools.small[KBASE_MEM_GROUP_CSF_FW],
			fw_ring_buf->num_pages, fw_ring_buf->phys, false,
			false);

		kfree(fw_ring_buf->phys);

		kfree(fw_ring_buf);
	}
}

static void kbasep_hwcnt_backend_csf_if_fw_dump_enable_nolock(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx,
	struct kbase_hwcnt_backend_csf_if_ring_buf *ring_buf,
	struct kbase_hwcnt_backend_csf_if_enable *enable)
{
	unsigned long flags;
	u32 prfcnt_config;
	struct kbase_device *kbdev;
	struct kbase_csf_global_iface *global_iface;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;
	struct kbase_hwcnt_backend_csf_if_fw_ring_buf *fw_ring_buf =
		(struct kbase_hwcnt_backend_csf_if_fw_ring_buf *)ring_buf;

	WARN_ON(!ctx);
	WARN_ON(!ring_buf);
	WARN_ON(!enable);

	kbdev = fw_ctx->kbdev;
	global_iface = &kbdev->csf.global_iface;

	lockdep_assert_held(&kbdev->hwaccess_lock);

	/* Configure */
	prfcnt_config = fw_ring_buf->buf_count;
	prfcnt_config |= enable->counter_set << PRFCNT_CONFIG_SETSELECT_SHIFT;

	kbase_csf_scheduler_spin_lock(kbdev, &flags);

	/* Configure the ring buffer base address */
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_JASID,
					fw_ring_buf->as_nr);
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_BASE_LO,
					fw_ring_buf->gpu_dump_base & U32_MAX);
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_BASE_HI,
					fw_ring_buf->gpu_dump_base >> 32);

	/* Set extract position to 0 */
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_EXTRACT, 0);

	/* Configure the enable bitmap */
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_CSF_EN,
					enable->fe_bm);
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_SHADER_EN,
					enable->shader_bm);
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_MMU_L2_EN,
					enable->mmu_l2_bm);
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_TILER_EN,
					enable->tiler_bm);

	/* Configure the HWC set and buffer size */
	kbase_csf_firmware_global_input(global_iface, GLB_PRFCNT_CONFIG,
					prfcnt_config);

	kbdev->csf.hwcnt.enable_pending = true;

	/* Unmask the interrupts */
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_SAMPLE_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_SAMPLE_MASK);
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_THRESHOLD_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_THRESHOLD_MASK);
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_OVERFLOW_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_OVERFLOW_MASK);
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_ENABLE_MASK,
		GLB_ACK_IRQ_MASK_PRFCNT_ENABLE_MASK);

	/* Enable the HWC */
	kbase_csf_firmware_global_input_mask(global_iface, GLB_REQ,
					     (1 << GLB_REQ_PRFCNT_ENABLE_SHIFT),
					     GLB_REQ_PRFCNT_ENABLE_MASK);
	kbase_csf_ring_doorbell(kbdev, CSF_KERNEL_DOORBELL_NR);

	prfcnt_config = kbase_csf_firmware_global_input_read(global_iface,
							     GLB_PRFCNT_CONFIG);

	kbase_csf_scheduler_spin_unlock(kbdev, flags);

	kbasep_hwcnt_backend_csf_if_fw_cc_enable(fw_ctx,
						 enable->clk_enable_map);
}

static void kbasep_hwcnt_backend_csf_if_fw_dump_enable(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx,
	struct kbase_hwcnt_backend_csf_if_ring_buf *ring_buf,
	struct kbase_hwcnt_backend_csf_if_enable *enable)
{
	unsigned long flags;
	struct kbase_device *kbdev;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);
	WARN_ON(!ring_buf);
	WARN_ON(!enable);

	kbdev = fw_ctx->kbdev;

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);

	kbasep_hwcnt_backend_csf_if_fw_dump_enable_nolock(ctx, ring_buf,
							  enable);

	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
}

static void kbasep_hwcnt_backend_csf_if_fw_dump_disable(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx)
{
	unsigned long flags;
	struct kbase_device *kbdev;
	struct kbase_csf_global_iface *global_iface;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);

	kbdev = fw_ctx->kbdev;
	global_iface = &kbdev->csf.global_iface;

	/* Disable the HWC */
	kbase_csf_scheduler_spin_lock(kbdev, &flags);
	kbdev->csf.hwcnt.enable_pending = true;
	kbase_csf_firmware_global_input_mask(global_iface, GLB_REQ, 0,
					     GLB_REQ_PRFCNT_ENABLE_MASK);
	kbase_csf_ring_doorbell(kbdev, CSF_KERNEL_DOORBELL_NR);

	/* mask the interrupts */
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK, 0,
		GLB_ACK_IRQ_MASK_PRFCNT_SAMPLE_MASK);
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK, 0,
		GLB_ACK_IRQ_MASK_PRFCNT_THRESHOLD_MASK);
	kbase_csf_firmware_global_input_mask(
		global_iface, GLB_ACK_IRQ_MASK, 0,
		GLB_ACK_IRQ_MASK_PRFCNT_OVERFLOW_MASK);

	/* In case we have a previous request in flight when the disable
	 * happens.
	 */
	kbdev->csf.hwcnt.request_pending = false;
	kbase_csf_scheduler_spin_unlock(kbdev, flags);

	kbasep_hwcnt_backend_csf_if_fw_cc_disable(fw_ctx);
}

static void kbasep_hwcnt_backend_csf_if_fw_dump_request(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx)
{
	unsigned long flags;
	u32 glb_req;
	struct kbase_device *kbdev;
	struct kbase_csf_global_iface *global_iface;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);

	kbdev = fw_ctx->kbdev;
	global_iface = &kbdev->csf.global_iface;

	/* Trigger dumping */
	kbase_csf_scheduler_spin_lock(kbdev, &flags);
	kbdev->csf.hwcnt.request_pending = true;
	glb_req = kbase_csf_firmware_global_input_read(global_iface, GLB_REQ);
	glb_req ^= GLB_REQ_PRFCNT_SAMPLE_MASK;
	kbase_csf_firmware_global_input_mask(global_iface, GLB_REQ, glb_req,
					     GLB_REQ_PRFCNT_SAMPLE_MASK);
	kbase_csf_ring_doorbell(kbdev, CSF_KERNEL_DOORBELL_NR);
	kbase_csf_scheduler_spin_unlock(kbdev, flags);
}

static void kbasep_hwcnt_backend_csf_if_fw_get_indexes(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx, u32 *extract_index,
	u32 *insert_index)
{
	unsigned long flags;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);
	WARN_ON(!extract_index);
	WARN_ON(!insert_index);

	kbase_csf_scheduler_spin_lock(fw_ctx->kbdev, &flags);
	*extract_index = kbase_csf_firmware_global_input_read(
		&fw_ctx->kbdev->csf.global_iface, GLB_PRFCNT_EXTRACT);
	*insert_index = kbase_csf_firmware_global_output(
		&fw_ctx->kbdev->csf.global_iface, GLB_PRFCNT_INSERT);
	kbase_csf_scheduler_spin_unlock(fw_ctx->kbdev, flags);
}

static void kbasep_hwcnt_backend_csf_if_fw_set_extract_index(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx, u32 extract_idx)
{
	unsigned long flags;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;

	WARN_ON(!ctx);

	/* Set the raw extract index to release the buffer back to the ring
	 * buffer.
	 */
	kbase_csf_scheduler_spin_lock(fw_ctx->kbdev, &flags);
	kbase_csf_firmware_global_input(&fw_ctx->kbdev->csf.global_iface,
					GLB_PRFCNT_EXTRACT, extract_idx);
	kbase_csf_scheduler_spin_unlock(fw_ctx->kbdev, flags);
}

static void kbasep_hwcnt_backend_csf_if_fw_get_gpu_cycle_count(
	struct kbase_hwcnt_backend_csf_if_ctx *ctx, u64 *cycle_counts,
	u64 clk_enable_map)
{
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx =
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)ctx;
	u8 clk;
	u64 timestamp_ns = ktime_get_raw_ns();

	WARN_ON(!ctx);
	WARN_ON(!cycle_counts);

	for (clk = 0; clk < fw_ctx->clk_cnt; clk++) {
		if (!(clk_enable_map & (1ull << clk)))
			continue;

		if (clk == KBASE_CLOCK_DOMAIN_TOP) {
			/* Read cycle count for top clock domain. */
			kbase_backend_get_gpu_time_norequest(
				fw_ctx->kbdev, &cycle_counts[clk], NULL, NULL);
		} else {
			/* Estimate cycle count for non-top clock domain. */
			cycle_counts[clk] = kbase_ccswe_cycle_at(
				&fw_ctx->ccswe_shader_cores, timestamp_ns);
		}
	}
}

/**
 * @brief Destroy a CSF FW interface context.
 *
 * @param[in,out] fw_ctx Pointer to context to destroy.
 */
static void kbasep_hwcnt_backend_csf_if_fw_ctx_destroy(
	struct kbase_hwcnt_backend_csf_if_fw_ctx *fw_ctx)
{
	if (!fw_ctx)
		return;

	kfree(fw_ctx);
}

/**
 * kbasep_hwcnt_backend_csf_if_fw_ctx_create() - Create a CSF Firmware context.
 *
 * @kbdev:   Non_NULL pointer to kbase device.
 * @out_ctx: Non-NULL pointer to where info is stored on success.
 * Return: 0 on success, else error code.
 */
static int kbasep_hwcnt_backend_csf_if_fw_ctx_create(
	struct kbase_device *kbdev,
	struct kbase_hwcnt_backend_csf_if_fw_ctx **out_ctx)
{
	u8 clk;
	int errcode = -ENOMEM;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *ctx = NULL;

	WARN_ON(!kbdev);
	WARN_ON(!out_ctx);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		goto error;

	ctx->kbdev = kbdev;

	/* Determine the number of available clock domains. */
	for (clk = 0; clk < BASE_MAX_NR_CLOCKS_REGULATORS; clk++) {
		if (kbdev->pm.clk_rtm.clks[clk] == NULL)
			break;
	}
	ctx->clk_cnt = clk;

	ctx->clk_enable_map = 0;
	kbase_ccswe_init(&ctx->ccswe_shader_cores);
	ctx->rate_listener.notify =
		kbasep_hwcnt_backend_csf_if_fw_on_freq_change;

	*out_ctx = ctx;

	return 0;
error:
	kbasep_hwcnt_backend_csf_if_fw_ctx_destroy(ctx);
	return errcode;
}

void kbase_hwcnt_backend_csf_if_fw_destroy(
	struct kbase_hwcnt_backend_csf_if *if_fw)
{
	if (!if_fw)
		return;

	kbasep_hwcnt_backend_csf_if_fw_ctx_destroy(
		(struct kbase_hwcnt_backend_csf_if_fw_ctx *)if_fw->ctx);
	memset(if_fw, 0, sizeof(*if_fw));
}

int kbase_hwcnt_backend_csf_if_fw_create(
	struct kbase_device *kbdev, struct kbase_hwcnt_backend_csf_if *if_fw)
{
	int errcode;
	struct kbase_hwcnt_backend_csf_if_fw_ctx *ctx = NULL;

	if (!kbdev || !if_fw)
		return -EINVAL;

	errcode = kbasep_hwcnt_backend_csf_if_fw_ctx_create(kbdev, &ctx);
	if (errcode)
		return errcode;

	if_fw->ctx = (struct kbase_hwcnt_backend_csf_if_ctx *)ctx;
	if_fw->get_gpu_info = kbasep_hwcnt_backend_csf_if_fw_get_gpu_info;
	if_fw->ring_buf_alloc = kbasep_hwcnt_backend_csf_if_fw_ring_buf_alloc;
	if_fw->ring_buf_sync = kbasep_hwcnt_backend_csf_if_fw_ring_buf_sync;
	if_fw->ring_buf_free = kbasep_hwcnt_backend_csf_if_fw_ring_buf_free;
	if_fw->timestamp_ns = kbasep_hwcnt_backend_csf_if_fw_timestamp_ns;
	if_fw->dump_enable = kbasep_hwcnt_backend_csf_if_fw_dump_enable;
	if_fw->dump_enable_nolock =
		kbasep_hwcnt_backend_csf_if_fw_dump_enable_nolock;
	if_fw->dump_disable = kbasep_hwcnt_backend_csf_if_fw_dump_disable;
	if_fw->dump_request = kbasep_hwcnt_backend_csf_if_fw_dump_request;
	if_fw->get_gpu_cycle_count =
		kbasep_hwcnt_backend_csf_if_fw_get_gpu_cycle_count;
	if_fw->get_indexes = kbasep_hwcnt_backend_csf_if_fw_get_indexes;
	if_fw->set_extract_index =
		kbasep_hwcnt_backend_csf_if_fw_set_extract_index;

	return 0;
}
