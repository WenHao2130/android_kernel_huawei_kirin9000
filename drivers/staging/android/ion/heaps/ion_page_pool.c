// SPDX-License-Identifier: GPL-2.0
/*
 * ION Memory Allocator page pool helpers
 *
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/list.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/sched/signal.h>

#include "ion_page_pool.h"

#ifdef CONFIG_ZONE_MEDIA_OPT
static inline struct page *ion_page_pool_alloc_pages(struct ion_page_pool *pool,
							gfp_t gfp_mask)
{
	return alloc_pages(pool->gfp_mask | gfp_mask, pool->order);
}
#else
static inline struct page *ion_page_pool_alloc_pages(struct ion_page_pool *pool)
{
	if (fatal_signal_pending(current))
		return NULL;
	return alloc_pages(pool->gfp_mask, pool->order);
}
#endif

static void ion_page_pool_free_pages(struct ion_page_pool *pool,
				     struct page *page)
{
	__free_pages(page, pool->order);
}

static void ion_page_pool_add(struct ion_page_pool *pool, struct page *page)
{
	mutex_lock(&pool->mutex);
	zone_page_state_add(1 << pool->order, page_zone(page),
			    NR_IONCACHE_PAGES);
	if (PageHighMem(page)) {
		list_add_tail(&page->lru, &pool->high_items);
		pool->high_count++;
	} else {
		list_add_tail(&page->lru, &pool->low_items);
		pool->low_count++;
	}

	mutex_unlock(&pool->mutex);
}

static struct page *ion_page_pool_remove(struct ion_page_pool *pool, bool high)
{
	struct page *page;

	if (high) {
		BUG_ON(!pool->high_count);
		page = list_first_entry(&pool->high_items, struct page, lru);
		pool->high_count--;
	} else {
		BUG_ON(!pool->low_count);
		page = list_first_entry(&pool->low_items, struct page, lru);
		pool->low_count--;
	}
	zone_page_state_add(-1 << pool->order, page_zone(page),
			    NR_IONCACHE_PAGES);

	list_del(&page->lru);
	
	return page;
}

struct page *ion_page_pool_alloc(struct ion_page_pool *pool)
{
	struct page *page = NULL;

	BUG_ON(!pool);

	mutex_lock(&pool->mutex);
	if (pool->high_count)
		page = ion_page_pool_remove(pool, true);
	else if (pool->low_count)
		page = ion_page_pool_remove(pool, false);
	mutex_unlock(&pool->mutex);

	if (!page)
#ifdef CONFIG_ZONE_MEDIA_OPT
		page = ion_page_pool_alloc_pages(pool, 0);
#else
		page = ion_page_pool_alloc_pages(pool);
#endif

	return page;
}
EXPORT_SYMBOL_GPL(ion_page_pool_alloc);

#ifdef CONFIG_ZONE_MEDIA_OPT
struct page *ion_page_pool_alloc_with_gfp(struct ion_page_pool *pool,
					  gfp_t gfp_mask)
{
	struct page *page = NULL;

	BUG_ON(!pool);

	mutex_lock(&pool->mutex);
	if (pool->high_count)
		page = ion_page_pool_remove(pool, true);
	else if (pool->low_count)
		page = ion_page_pool_remove(pool, false);
	mutex_unlock(&pool->mutex);

	if (!page)
		page = ion_page_pool_alloc_pages(pool, gfp_mask);

	return page;
}
#endif

void ion_page_pool_free(struct ion_page_pool *pool, struct page *page)
{
	BUG_ON(pool->order != compound_order(page));

	ion_page_pool_add(pool, page);
}
EXPORT_SYMBOL_GPL(ion_page_pool_free);

void ion_page_pool_free_immediate(struct ion_page_pool *pool,
				  struct page *page)
{
	ion_page_pool_free_pages(pool, page);
}

int ion_page_pool_total(struct ion_page_pool *pool, bool high)
{
	int count = pool->low_count;

	if (high)
		count += pool->high_count;

	return count << pool->order;
}

int ion_page_pool_shrink(struct ion_page_pool *pool, gfp_t gfp_mask,
			 int nr_to_scan)
{
	int freed = 0;
	bool high;

	if (current_is_kswapd())
		high = true;
	else
		high = !!(gfp_mask & __GFP_HIGHMEM);

	if (nr_to_scan == 0)
		return ion_page_pool_total(pool, high);

	while (freed < nr_to_scan) {
		struct page *page;

		mutex_lock(&pool->mutex);
		if (pool->low_count) {
			page = ion_page_pool_remove(pool, false);
		} else if (high && pool->high_count) {
			page = ion_page_pool_remove(pool, true);
		} else {
			mutex_unlock(&pool->mutex);
			break;
		}
		mutex_unlock(&pool->mutex);
		ion_page_pool_free_pages(pool, page);
		freed += (1 << pool->order);
	}

	return freed;
}
EXPORT_SYMBOL_GPL(ion_page_pool_shrink);

struct ion_page_pool *ion_page_pool_create(gfp_t gfp_mask, unsigned int order)
{
	struct ion_page_pool *pool = kzalloc(sizeof(*pool), GFP_KERNEL);

	if (!pool)
		return NULL;
	pool->high_count = 0;
	pool->low_count = 0;
	INIT_LIST_HEAD(&pool->low_items);
	INIT_LIST_HEAD(&pool->high_items);
	pool->gfp_mask = gfp_mask | __GFP_COMP;
	pool->order = order;
	mutex_init(&pool->mutex);
	plist_node_init(&pool->list, order);

	return pool;
}
EXPORT_SYMBOL_GPL(ion_page_pool_create);

void ion_page_pool_destroy(struct ion_page_pool *pool)
{
	kfree(pool);
}
EXPORT_SYMBOL_GPL(ion_page_pool_destroy);
