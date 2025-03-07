// SPDX-License-Identifier: GPL-2.0
/*
 * ION Memory Allocator system heap exporter
 *
 * Copyright (C) 2011 Google, Inc.
 */

#include <asm/page.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/kthread.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/platform_drivers/mm_ion.h>
#include <linux/seq_file.h>

#include "ion_page_pool.h"
#include "../ion.h"

#define NUM_ORDERS ARRAY_SIZE(orders)

static gfp_t high_order_gfp_flags = (GFP_HIGHUSER | __GFP_ZERO | __GFP_NOWARN |
				     __GFP_NORETRY) & ~__GFP_RECLAIM;
static gfp_t low_order_gfp_flags  = GFP_HIGHUSER | __GFP_ZERO;
static const unsigned int orders[] = {8, 4, 0};
static bool install;

static int order_to_index(unsigned int order)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (order == orders[i])
			return i;
	BUG();
	return -1;
}

static inline unsigned int order_to_size(int order)
{
	return PAGE_SIZE << order;
}

struct ion_system_heap {
	struct ion_heap heap;
	struct ion_page_pool *pools[NUM_ORDERS];
	unsigned long pool_watermark;
	struct task_struct *sys_pool_thread;
	wait_queue_head_t sys_pool_wait;
	atomic_t wait_flag;
	struct mutex pool_lock;
#ifdef CONFIG_ZONE_MEDIA_OPT
	atomic64_t cma_page_num;
#endif
};

static struct ion_system_heap *ion_sys_heap;

static int ion_sys_pool_count(struct ion_system_heap *heap)
{
	struct ion_page_pool *pool = NULL;
	int nr_pool_total = 0;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = heap->pools[i];

		nr_pool_total += ion_page_pool_total(pool, false);
	}

	return nr_pool_total;
}

static int fill_pool_once(struct ion_page_pool *pool)
{
	struct page *page = NULL;

	page = alloc_pages(pool->gfp_mask, pool->order);
	if (!page)
		return -ENOMEM;

	ion_page_pool_free(pool, page);

	return 0;
}

static void fill_pool_watermark(struct ion_page_pool **pools,
				unsigned long watermark)
{
	unsigned int i;
	unsigned long count;

	for (i = 0; i < NUM_ORDERS; i++) {
		while (watermark) {
			if (fill_pool_once(pools[i]))
				break;

			count = 1UL << pools[i]->order;
			if (watermark >= count)
				watermark -= count;
			else
				watermark = 0;
		}
	}
}

static void ion_sys_pool_wakeup(void)
{
	struct ion_system_heap *heap = ion_sys_heap;

	atomic_set(&heap->wait_flag, 1);
	wake_up_interruptible(&heap->sys_pool_wait);
}

static int ion_sys_pool_kthread(void *p)
{
	struct ion_system_heap *heap = NULL;
	int ret;
#ifdef CONFIG_ZONE_MEDIA_OPT
	long nr_fill_count = 0;
#endif

	if (!p)
		return -EINVAL;

	heap = (struct ion_system_heap *)p;
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(heap->sys_pool_wait,
						atomic_read(&heap->wait_flag));
		if (ret)
			continue;

		atomic_set(&heap->wait_flag, 0);

#ifdef CONFIG_ZONE_MEDIA_OPT
		nr_fill_count = heap->pool_watermark -
				(unsigned long)ion_sys_pool_count(heap);
		if (nr_fill_count <= 0)
			continue;
		mutex_lock(&heap->pool_lock);
		fill_pool_watermark(heap->pools, nr_fill_count);
		mutex_unlock(&heap->pool_lock);
#else
		mutex_lock(&heap->pool_lock);
		if (heap->pool_watermark)
			fill_pool_watermark(heap->pools,
					    heap->pool_watermark);

		heap->pool_watermark = 0;
		mutex_unlock(&heap->pool_lock);
#endif
	}

	return 0;
}

static bool sys_pool_watermark_check(struct ion_system_heap *heap,
				     unsigned long nr_watermark)
{
	unsigned long nr_pool_count = 0;

	if (!nr_watermark)
		return false;

	if (heap->pool_watermark >= nr_watermark)
		return false;

	nr_pool_count = (unsigned long)ion_sys_pool_count(heap);
	if (nr_pool_count >= nr_watermark)
		return false;

	return true;
}

void set_sys_pool_watermark(unsigned long watermark)
{
	struct ion_system_heap *heap = ion_sys_heap;
	unsigned long nr_watermark = watermark / PAGE_SIZE;
	bool pool_wakeup = true;

	if (!wq_has_sleeper(&heap->sys_pool_wait))
		goto drain_pages;

	pool_wakeup = sys_pool_watermark_check(heap, nr_watermark);
	mutex_lock(&heap->pool_lock);
	/*
	 * Maximization principle.
	 */
	if (!nr_watermark || heap->pool_watermark < nr_watermark)
		heap->pool_watermark = nr_watermark;
	mutex_unlock(&heap->pool_lock);

	if (pool_wakeup)
		ion_sys_pool_wakeup();

drain_pages:
	/*
	 * Setting ZERO to watermark,
	 * Meaning that APP does not need the buffer
	 * So we drain all pages.
	 */
	if (!nr_watermark)
		ion_heap_freelist_drain(&heap->heap, 0);
}
static struct page *alloc_buffer_page(struct ion_system_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long order)
{
	struct ion_page_pool *pool = heap->pools[order_to_index(order)];
	struct page *page = NULL;
#ifdef CONFIG_ZONE_MEDIA_OPT
	unsigned long cam_flag = buffer->flags & ION_FLAG_CAM_CMA_BUFFER;
	gfp_t gfp_mask = 0;

	if (cam_flag)
		gfp_mask = ___GFP_CMA;
	page = ion_page_pool_alloc_with_gfp(pool, gfp_mask);
	if (page && page_is_cma(page))
		atomic64_add(1 << compound_order(page), &heap->cma_page_num);
#else
	page = ion_page_pool_alloc(pool);
#endif
#ifdef CONFIG_DFX_KERNELDUMP
	if (page)
		SetPageMemDump(page);
#endif
	return page;
}

static void free_buffer_page(struct ion_system_heap *heap,
			     struct ion_buffer *buffer, struct page *page)
{
	struct ion_page_pool *pool;
	unsigned int order = compound_order(page);
	pgprot_t pgprot;

#ifdef CONFIG_ZONE_MEDIA_OPT
	if (page_is_cma(page))
		atomic64_sub(1 << compound_order(page), &heap->cma_page_num);
#endif
	/* go to system */
	if (buffer->private_flags & ION_PRIV_FLAG_SHRINKER_FREE) {
		__free_pages(page, order);
		return;
	}

	pool = heap->pools[order_to_index(order)];

	if (order == 0 || page_is_cma(page)) {
		ion_page_pool_free_immediate(pool, page);
		return;
	}

	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);
	ion_heap_pages_zero(page, PAGE_SIZE << compound_order(page), pgprot);

	ion_page_pool_free(pool, page);
}

static struct page *alloc_largest_available(struct ion_system_heap *heap,
					    struct ion_buffer *buffer,
					    unsigned long size,
					    unsigned int max_order)
{
	struct page *page;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		page = alloc_buffer_page(heap, buffer, orders[i]);
		if (!page)
			continue;

		return page;
	}

	return NULL;
}

static int ion_system_heap_allocate(struct ion_heap *heap,
				    struct ion_buffer *buffer,
				    unsigned long size,
				    unsigned long flags)
{
	struct ion_system_heap *sys_heap = container_of(heap,
							struct ion_system_heap,
							heap);
	struct sg_table *table = NULL;
	struct scatterlist *sg;
	struct list_head pages;
	struct page *page, *tmp_page;
	int i = 0;
	unsigned long size_remaining = PAGE_ALIGN(size);
	unsigned int max_order = orders[0];

	if (size / PAGE_SIZE > totalram_pages() / 2)
		return -ENOMEM;

	INIT_LIST_HEAD(&pages);
	while (size_remaining > 0) {
		page = alloc_largest_available(sys_heap, buffer, size_remaining,
					       max_order);
		if (!page)
			goto free_pages;
		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		max_order = compound_order(page);
		i++;
	}
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		goto free_pages;

	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	buffer->sg_table = table;

	return 0;

free_table:
	kfree(table);
free_pages:
	list_for_each_entry_safe(page, tmp_page, &pages, lru)
		free_buffer_page(sys_heap, buffer, page);
	return -ENOMEM;
}

static void ion_system_heap_free(struct ion_buffer *buffer)
{
	struct ion_system_heap *sys_heap = container_of(buffer->heap,
							struct ion_system_heap,
							heap);
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	int i;

	for_each_sg(table->sgl, sg, table->nents, i)
		free_buffer_page(sys_heap, buffer, sg_page(sg));
	sg_free_table(table);
	kfree(table);
}

static int ion_system_heap_shrink(struct ion_heap *heap, gfp_t gfp_mask,
				  int nr_to_scan)
{
	struct ion_page_pool *pool;
	struct ion_system_heap *sys_heap;
	int nr_total = 0;
	int i, nr_freed;
	int only_scan = 0;
#ifdef CONFIG_ZONE_MEDIA_OPT
	unsigned long pool_pages = 0;
	long gt_watermark_count = 0; /* greater than watermark count */
#endif

	sys_heap = container_of(heap, struct ion_system_heap, heap);

	if (!nr_to_scan) {
		only_scan = 1;
	} else {
#ifdef CONFIG_ZONE_MEDIA_OPT
		gt_watermark_count = (unsigned long)ion_sys_pool_count(sys_heap) -
				    sys_heap->pool_watermark;
		if (gt_watermark_count <= 0)
			return 0;
		if (nr_to_scan > gt_watermark_count)
			nr_to_scan = (int)gt_watermark_count;
#endif
	}

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = sys_heap->pools[i];

		if (only_scan) {
			nr_total += ion_page_pool_shrink(pool,
							 gfp_mask,
							 nr_to_scan);

		} else {
			nr_freed = ion_page_pool_shrink(pool,
							gfp_mask,
							nr_to_scan);
			nr_to_scan -= nr_freed;
			nr_total += nr_freed;
			if (nr_to_scan <= 0)
				break;
		}
	}
#ifdef CONFIG_ZONE_MEDIA_OPT
	if (only_scan) {
		pool_pages = sys_heap->pool_watermark;
		nr_total -= min(nr_total, pool_pages);
	}
#endif
	return nr_total;
}

static struct ion_heap_ops system_heap_ops = {
	.allocate = ion_system_heap_allocate,
	.free = ion_system_heap_free,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
	.map_user = ion_heap_map_user,
	.shrink = ion_system_heap_shrink,
};

static int ion_system_heap_debug_show(struct ion_heap *heap, struct seq_file *s,
				      void *unused)
{
	struct ion_system_heap *sys_heap = container_of(heap,
							struct ion_system_heap,
							heap);
	int i;
	struct ion_page_pool *pool;

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = sys_heap->pools[i];

		seq_printf(s, "%d order %u highmem pages %lu total\n",
			   pool->high_count, pool->order,
			   (PAGE_SIZE << pool->order) * pool->high_count);
		seq_printf(s, "%d order %u lowmem pages %lu total\n",
			   pool->low_count, pool->order,
			   (PAGE_SIZE << pool->order) * pool->low_count);
	}

#ifdef CONFIG_ZONE_MEDIA_OPT
	seq_printf(s, "%ld cma pages allocated by camera\n",
		   atomic64_read(&sys_heap->cma_page_num));
#endif

	return 0;
}

static void ion_system_heap_destroy_pools(struct ion_page_pool **pools)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (pools[i])
			ion_page_pool_destroy(pools[i]);
}

static int ion_system_heap_create_pools(struct ion_page_pool **pools)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		struct ion_page_pool *pool;
		gfp_t gfp_flags = low_order_gfp_flags;

		if (orders[i] >= 4)
			gfp_flags = high_order_gfp_flags;

		if (orders[i] == 8)
			gfp_flags = high_order_gfp_flags & ~__GFP_RECLAIMABLE;
		pool = ion_page_pool_create(gfp_flags, orders[i]);
		if (!pool)
			goto err_create_pool;
		pools[i] = pool;
	}

	return 0;

err_create_pool:
	ion_system_heap_destroy_pools(pools);
	return -ENOMEM;
}

struct ion_heap *ion_system_heap_create(struct ion_platform_heap *unused_data)
{
	struct ion_system_heap *heap;

	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);
	heap->heap.ops = &system_heap_ops;
	heap->heap.type = ION_HEAP_TYPE_SYSTEM;
	heap->heap.flags = ION_HEAP_FLAG_DEFER_FREE;

	if (ion_system_heap_create_pools(heap->pools))
		goto free_heap;

	atomic_set(&heap->wait_flag, 0);
#ifdef CONFIG_ZONE_MEDIA_OPT
	atomic64_set(&heap->cma_page_num, 0);
#endif
	init_waitqueue_head(&heap->sys_pool_wait);
	heap->sys_pool_thread = kthread_run(ion_sys_pool_kthread, heap,
						"%s", "sys_pool");
	if (IS_ERR(heap->sys_pool_thread)) {
		pr_err("%s: kthread_create failed!\n", __func__);
		goto destroy_pools;
	}
	mutex_init(&heap->pool_lock);

	heap->heap.debug_show = ion_system_heap_debug_show;
	ion_sys_heap = heap;

	return &heap->heap;

destroy_pools:
	ion_system_heap_destroy_pools(heap->pools);

free_heap:
	kfree(heap);
	return ERR_PTR(-ENOMEM);
}

static int ion_system_contig_heap_allocate(struct ion_heap *heap,
					   struct ion_buffer *buffer,
					   unsigned long len,
					   unsigned long flags)
{
	int order = get_order(len);
	struct page *page;
	struct sg_table *table = NULL;
	unsigned long i;
	int ret;

	page = alloc_pages(low_order_gfp_flags | __GFP_NOWARN, order);
	if (!page)
		return -ENOMEM;

	split_page(page, order);

	len = PAGE_ALIGN(len);
	for (i = len >> PAGE_SHIFT; i < (1 << order); i++)
		__free_page(page + i);

	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		ret = -ENOMEM;
		goto free_pages;
	}

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret)
		goto free_table;

	sg_set_page(table->sgl, page, len, 0);

	buffer->sg_table = table;

	return 0;

free_table:
	kfree(table);
free_pages:
	for (i = 0; i < len >> PAGE_SHIFT; i++)
		__free_page(page + i);

	return ret;
}

static void ion_system_contig_heap_free(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct page *page = sg_page(table->sgl);
	unsigned long pages = PAGE_ALIGN(buffer->size) >> PAGE_SHIFT;
	unsigned long i;

	for (i = 0; i < pages; i++)
		__free_page(page + i);
	sg_free_table(table);
	kfree(table);
}


static struct ion_heap_ops kmalloc_ops = {
	.allocate = ion_system_contig_heap_allocate,
	.free = ion_system_contig_heap_free,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
	.map_user = ion_heap_map_user,
};

static struct ion_heap *__ion_system_contig_heap_create(void)
{
	struct ion_heap *heap;

	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);
	heap->ops = &kmalloc_ops;
	heap->type = ION_HEAP_TYPE_SYSTEM_CONTIG;
	heap->name = "ion_system_contig_heap";
	return heap;
}

static int __init ion_system_contig_heap_cfg(char *param)
{
	pr_err("%s: param is =%s\n", __func__, param);
	if (strcmp(param, "y") == 0)
		install = true;
	else
		install = false;
	return 0;
}

early_param("contigheap", ion_system_contig_heap_cfg);

static int ion_system_contig_heap_create(void)
{
	struct ion_heap *heap;

	if (!install) {
		pr_err("%s: skip contig heap create.\n", __func__);
		return 0;
	}

	heap = __ion_system_contig_heap_create();
	if (IS_ERR(heap))
		return PTR_ERR(heap);

	ion_device_add_heap(heap);
	return 0;
}
device_initcall(ion_system_contig_heap_create);

