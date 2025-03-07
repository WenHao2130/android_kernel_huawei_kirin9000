// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/checkpoint.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/mpage.h>
#include <linux/writeback.h>
#include <linux/blkdev.h>
#include <linux/hmfs_fs.h>
#include <linux/pagevec.h>
#include <linux/swap.h>
#include <linux/part_stat.h>

#ifdef CONFIG_FSCK_BOOST
#include <linux/fsck_boost.h>
#endif

#include "hmfs.h"
#include "node.h"
#include "segment.h"
#include "trace.h"
#include <trace/events/hmfs.h>

static struct kmem_cache *ino_entry_slab;
struct kmem_cache *hmfs_inode_entry_slab;

void hmfs_stop_checkpoint(struct f2fs_sb_info *sbi, bool end_io)
{
	f2fs_build_fault_attr(sbi, 0, 0);
	set_ckpt_flags(sbi, CP_ERROR_FLAG);
	if (!end_io)
		hmfs_flush_merged_writes(sbi);
}

/*
 * We guarantee no failure on the returned page.
 */
struct page *hmfs_grab_meta_page(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct address_space *mapping = META_MAPPING(sbi);
	struct page *page = NULL;
repeat:
	page = f2fs_grab_cache_page(mapping, index, false);
	if (!page) {
		cond_resched();
		goto repeat;
	}
	hmfs_wait_on_page_writeback(page, META, true);
	if (!PageUptodate(page))
		SetPageUptodate(page);
	return page;
}

/*
 * We guarantee no failure on the returned page if cache_only is false.
 */
struct page *hmfs_get_meta_page_ex(struct f2fs_sb_info *sbi, pgoff_t index,
			bool is_meta, bool cache_only)
{
	struct address_space *mapping = NULL;
	struct page *page;
	struct f2fs_io_info fio = {
		.sbi = sbi,
		.type = META,
		.op = REQ_OP_READ,
		.op_flags = REQ_META | REQ_PRIO,
		.old_blkaddr = index,
		.new_blkaddr = index,
		.encrypted_page = NULL,
		.is_meta = is_meta,
	};
	int err;

#ifdef CONFIG_FSCK_BOOST
	if (!sbi->inited) {
		mapping = BDEV_MAPPING(sbi);
		page = find_lock_page(mapping, index);
		if (page && PageUptodate(page))
			return page;
	}
#endif

	mapping = META_MAPPING(sbi);

	if (unlikely(!is_meta))
		fio.op_flags &= ~REQ_META;
repeat:
#ifdef CONFIG_HUAWEI_PAGECACHE_HELPER
	/* Just try to find but not create a page */
	if (cache_only)
		page = pch_get_page(mapping, index, FGP_LOCK|FGP_ACCESSED,
					mapping_gfp_mask(NODE_MAPPING(sbi)));
	else
		page = f2fs_grab_cache_page(mapping, index, false);
#else
	page = f2fs_grab_cache_page(mapping, index, false);
#endif
	if (!page) {
		if (cache_only)
			return page;
		cond_resched();
		goto repeat;
	}
	if (PageUptodate(page))
		goto out;

	fio.page = page;

	err = hmfs_submit_page_bio(&fio);
	if (err) {
		memset(page_address(page), 0, PAGE_SIZE);
		hmfs_stop_checkpoint(sbi, false);
		f2fs_bug_on(sbi, 1);
		/*lint -save -e527*/
		goto out;
		/*lint -restore*/
	}

	lock_page(page);
	if (unlikely(page->mapping != mapping)) {
		f2fs_put_page(page, 1);
		goto repeat;
	}

	if (unlikely(!PageUptodate(page))) {
		hmfs_stop_checkpoint(sbi, false);
		hmfs_msg(sbi->sb, KERN_ERR,"hmfs readed meta page is not uptodate!");
#ifdef CONFIG_HUAWEI_HMFS_DSM
		if (hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) {
			dsm_client_record(hmfs_dclient, "HMFS reboot: %s:%d\n",
				__func__, __LINE__);
			dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK);
		}
#endif
		f2fs_restart(); /* force restarting */
	}
out:
	return page;
}

static inline struct page *__get_meta_page(struct f2fs_sb_info *sbi,
			       pgoff_t index, bool is_meta)
{
	return hmfs_get_meta_page_ex(sbi, index, is_meta, false);
}

struct page *hmfs_get_meta_page(struct f2fs_sb_info *sbi, pgoff_t index)
{
	return __get_meta_page(sbi, index, true);
}

struct page *hmfs_get_meta_page_retry(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct page *page;
	int count = 0;

retry:
	page = __get_meta_page(sbi, index, true);
	if (IS_ERR(page)) {
		if (PTR_ERR(page) == -EIO &&
				++count <= DEFAULT_RETRY_IO_COUNT)
			goto retry;
		hmfs_stop_checkpoint(sbi, false);
	}
	return page;
}

/* for POR only */
struct page *hmfs_get_tmp_page(struct f2fs_sb_info *sbi, pgoff_t index)
{
	return __get_meta_page(sbi, index, false);
}

bool hmfs_is_valid_blkaddr(struct f2fs_sb_info *sbi,
					block_t blkaddr, int type)
{
	switch (type) {
	case META_NAT:
		break;
	case META_SIT:
		if (unlikely(blkaddr >= SIT_BLK_CNT(sbi)))
			return false;
		break;
	case META_SSA:
		if (unlikely(blkaddr >= MAIN_BLKADDR(sbi) ||
			blkaddr < SM_I(sbi)->ssa_blkaddr))
			return false;
		break;
	case META_CP:
		if (unlikely(blkaddr >= SIT_I(sbi)->sit_base_addr ||
			blkaddr < __start_cp_addr(sbi)))
			return false;
		break;
	case META_POR:
	case DATA_GENERIC:
		if (unlikely(blkaddr >= MAX_BLKADDR(sbi) && !sbi->resized ||
			blkaddr < MAIN_BLKADDR(sbi))) {
			if (type == DATA_GENERIC) {
				hmfs_msg(sbi->sb, KERN_WARNING,
					"access invalid blkaddr:%u", blkaddr);
				WARN_ON(1);
			}
			return false;
		}
		break;
	case META_GENERIC:
		if (unlikely(blkaddr < SEG0_BLKADDR(sbi) ||
			blkaddr >= MAIN_BLKADDR(sbi)))
			return false;
		break;
	default:
		BUG();
	}

	return true;
}

/*
 * Readahead CP/NAT/SIT/SSA pages
 */
int hmfs_ra_meta_pages(struct f2fs_sb_info *sbi, block_t start, int nrpages,
							int type, bool sync)
{
	struct page *page;
	block_t blkno = start;
	struct f2fs_io_info fio = {
		.sbi = sbi,
		.type = META,
		.op = REQ_OP_READ,
		.op_flags = sync ? (REQ_META | REQ_PRIO) : REQ_RAHEAD,
		.encrypted_page = NULL,
		.in_list = false,
		.is_meta = (type != META_POR),
	};
	struct blk_plug plug;

	if (unlikely(type == META_POR))
		fio.op_flags &= ~REQ_META;

	blk_start_plug(&plug);
	for (; nrpages-- > 0; blkno++) {

		if (!hmfs_is_valid_blkaddr(sbi, blkno, type))
			goto out;

		switch (type) {
		case META_NAT:
			if (unlikely(blkno >=
					NAT_BLOCK_OFFSET(NM_I(sbi)->max_nid)))
				blkno = 0;
			/* get nat block addr */
			fio.new_blkaddr = current_nat_addr(sbi,
					blkno * NAT_ENTRY_PER_BLOCK);
			break;
		case META_SIT:
			/* get sit block addr */
			fio.new_blkaddr = current_sit_addr(sbi,
					blkno * SIT_ENTRY_PER_BLOCK);
			break;
		case META_SSA:
		case META_CP:
		case META_POR:
			fio.new_blkaddr = blkno;
			break;
		default:
			BUG();
		}

#ifdef CONFIG_FSCK_BOOST
		if (!sbi->inited && is_fsck_boost_done(sbi->sb->s_bdev)) {
			page = find_get_page(BDEV_MAPPING(sbi),
					fio.new_blkaddr);
			if (page) {
				if (PageUptodate(page)) {
					put_page(page);
					continue;
				} else {
					put_page(page);
				}
			}
		}
#endif

		page = f2fs_grab_cache_page(META_MAPPING(sbi),
						fio.new_blkaddr, false);
		if (!page)
			continue;
		if (PageUptodate(page)) {
			f2fs_put_page(page, 1);
			continue;
		}

		fio.page = page;
		hmfs_submit_page_bio(&fio);
		f2fs_put_page(page, 0);
	}
out:
	blk_finish_plug(&plug);
	return blkno - start;
}

void hmfs_ra_meta_pages_cond(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct page *page;
	bool readahead = false;

	page = find_get_page(META_MAPPING(sbi), index);
	if (!page || !PageUptodate(page))
		readahead = true;
	f2fs_put_page(page, 0);

	if (readahead)
		hmfs_ra_meta_pages(sbi, index, BIO_MAX_PAGES, META_POR, true);
}

static int __f2fs_write_meta_page(struct page *page,
				struct writeback_control *wbc,
				enum iostat_type io_type)
{
	struct f2fs_sb_info *sbi = F2FS_P_SB(page);

	trace_hmfs_writepage(page, META);

	if (unlikely(f2fs_cp_error(sbi)))
		goto redirty_out;
	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		goto redirty_out;
	if (wbc->for_reclaim && page->index < GET_SUM_BLOCK(sbi, 0))
		goto redirty_out;

	hmfs_do_write_meta_page(sbi, page, io_type);
	dec_page_count(sbi, F2FS_DIRTY_META);

	if (wbc->for_reclaim)
		hmfs_submit_merged_write_cond(sbi, NULL, page, 0, META);

	unlock_page(page);

	if (unlikely(f2fs_cp_error(sbi)))
		hmfs_submit_merged_write(sbi, META);

	return 0;

redirty_out:
	redirty_page_for_writepage(wbc, page);
	return AOP_WRITEPAGE_ACTIVATE;
}

static int f2fs_write_meta_page(struct page *page,
				struct writeback_control *wbc)
{
	return __f2fs_write_meta_page(page, wbc, FS_META_IO);
}

static int f2fs_write_meta_pages(struct address_space *mapping,
				struct writeback_control *wbc)
{
	struct f2fs_sb_info *sbi = F2FS_M_SB(mapping);
	long diff, written;

	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		goto skip_write;

	/* collect a number of dirty meta pages and write together */
	if (wbc->sync_mode != WB_SYNC_ALL &&
			get_pages(sbi, F2FS_DIRTY_META) <
					nr_pages_to_skip(sbi, META))
		goto skip_write;

	/* if locked failed, cp will flush dirty pages instead */
	if (!mutex_trylock(&sbi->cp_mutex))
		goto skip_write;

	trace_hmfs_writepages(mapping->host, wbc, META);
	diff = nr_pages_to_write(sbi, META, wbc);
	written = hmfs_sync_meta_pages(sbi, META, wbc->nr_to_write, FS_META_IO);
	mutex_unlock(&sbi->cp_mutex);
	wbc->nr_to_write = max((long)0, wbc->nr_to_write - written - diff);
	return 0;

skip_write:
	wbc->pages_skipped += get_pages(sbi, F2FS_DIRTY_META);
	trace_hmfs_writepages(mapping->host, wbc, META);
	return 0;
}

long hmfs_sync_meta_pages(struct f2fs_sb_info *sbi, enum page_type type,
				long nr_to_write, enum iostat_type io_type)
{
	struct address_space *mapping = META_MAPPING(sbi);
	pgoff_t index = 0, prev = ULONG_MAX;
	struct pagevec pvec;
	long nwritten = 0;
	int nr_pages;
	struct writeback_control wbc = {
		.for_reclaim = 0,
	};
	struct blk_plug plug;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	pagevec_init(&pvec, 0);
#else
	pagevec_init(&pvec);
#endif

	blk_start_plug(&plug);

	while ((nr_pages = pagevec_lookup_tag(&pvec, mapping, &index,
				PAGECACHE_TAG_DIRTY))) {
		int i;

		for (i = 0; i < nr_pages; i++) {
			struct page *page = pvec.pages[i];

			if (prev == ULONG_MAX)
				prev = page->index - 1;
			if (nr_to_write != LONG_MAX && page->index != prev + 1) {
				pagevec_release(&pvec);
				goto stop;
			}

			lock_page(page);

			if (unlikely(page->mapping != mapping)) {
continue_unlock:
				unlock_page(page);
				continue;
			}
			if (!PageDirty(page)) {
				/* someone wrote it for us */
				goto continue_unlock;
			}

			hmfs_wait_on_page_writeback(page, META, true);

			BUG_ON(PageWriteback(page));
			if (!clear_page_dirty_for_io(page))
				goto continue_unlock;

			if (__f2fs_write_meta_page(page, &wbc, io_type)) {
				unlock_page(page);
				break;
			}
			nwritten++;
			prev = page->index;
			if (unlikely(nwritten >= nr_to_write))
				break;
		}
		pagevec_release(&pvec);
		cond_resched();
	}
stop:
	if (nwritten)
		hmfs_submit_merged_write(sbi, type);

	blk_finish_plug(&plug);

	return nwritten;
}

static int f2fs_set_meta_page_dirty(struct page *page)
{
	trace_hmfs_set_page_dirty(page, META);

	if (!PageUptodate(page))
		SetPageUptodate(page);
	if (!PageDirty(page)) {
		__set_page_dirty_nobuffers(page);
		inc_page_count(F2FS_P_SB(page), F2FS_DIRTY_META);
		f2fs_set_page_private(page, 0);
		f2fs_trace_pid(page);
		return 1;
	}
	return 0;
}

const struct address_space_operations hmfs_meta_aops = {
	.writepage	= f2fs_write_meta_page,
	.writepages	= f2fs_write_meta_pages,
	.set_page_dirty	= f2fs_set_meta_page_dirty,
	.invalidatepage = hmfs_invalidate_page,
	.releasepage	= hmfs_release_page,
#ifdef CONFIG_MIGRATION
	.migratepage	= hmfs_migrate_page,
#endif
};

static void __add_ino_entry(struct f2fs_sb_info *sbi, nid_t ino,
						unsigned int devidx, int type)
{
	struct inode_management *im = &sbi->im[type];
	struct ino_entry *e, *tmp;

	tmp = f2fs_kmem_cache_alloc(ino_entry_slab, GFP_NOFS);

	radix_tree_preload(GFP_NOFS | __GFP_NOFAIL);

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	if (!e) {
		e = tmp;
		if (unlikely(radix_tree_insert(&im->ino_root, ino, e)))
			f2fs_bug_on(sbi, 1);

		memset(e, 0, sizeof(struct ino_entry));
		e->ino = ino;

		list_add_tail(&e->list, &im->ino_list);
		if (type != ORPHAN_INO)
			im->ino_num++;
	}

	if (type == FLUSH_INO)
		f2fs_set_bit(devidx, (char *)&e->dirty_device);

	spin_unlock(&im->ino_lock);
	radix_tree_preload_end();

	if (e != tmp)
		kmem_cache_free(ino_entry_slab, tmp);
}

static void __remove_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	struct inode_management *im = &sbi->im[type];
	struct ino_entry *e;

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	if (e) {
		list_del(&e->list);
		radix_tree_delete(&im->ino_root, ino);
		im->ino_num--;
		spin_unlock(&im->ino_lock);
		kmem_cache_free(ino_entry_slab, e);
		return;
	}
	spin_unlock(&im->ino_lock);
}

void hmfs_add_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	/* add new dirty ino entry into list */
	__add_ino_entry(sbi, ino, 0, type);
}

void hmfs_remove_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	/* remove dirty ino entry from list */
	__remove_ino_entry(sbi, ino, type);
}

/* mode should be APPEND_INO or UPDATE_INO */
bool hmfs_exist_written_data(struct f2fs_sb_info *sbi, nid_t ino, int mode)
{
	struct inode_management *im = &sbi->im[mode];
	struct ino_entry *e;

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	spin_unlock(&im->ino_lock);
	return e ? true : false;
}

void hmfs_release_ino_entry(struct f2fs_sb_info *sbi, bool all)
{
	struct ino_entry *e, *tmp;
	int i;

	for (i = all ? ORPHAN_INO : APPEND_INO; i < MAX_INO_ENTRY; i++) {
		struct inode_management *im = &sbi->im[i];

		spin_lock(&im->ino_lock);
		list_for_each_entry_safe(e, tmp, &im->ino_list, list) {
			list_del(&e->list);
			radix_tree_delete(&im->ino_root, e->ino);
			kmem_cache_free(ino_entry_slab, e);
			im->ino_num--;
		}
		spin_unlock(&im->ino_lock);
	}
}

void hmfs_set_dirty_device(struct f2fs_sb_info *sbi, nid_t ino,
					unsigned int devidx, int type)
{
	__add_ino_entry(sbi, ino, devidx, type);
}

bool hmfs_is_dirty_device(struct f2fs_sb_info *sbi, nid_t ino,
					unsigned int devidx, int type)
{
	struct inode_management *im = &sbi->im[type];
	struct ino_entry *e;
	bool is_dirty = false;

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	if (e && f2fs_test_bit(devidx, (char *)&e->dirty_device))
		is_dirty = true;
	spin_unlock(&im->ino_lock);
	return is_dirty;
}

int hmfs_acquire_orphan_inode(struct f2fs_sb_info *sbi)
{
	struct inode_management *im = &sbi->im[ORPHAN_INO];
	int err = 0;

	spin_lock(&im->ino_lock);

	if (time_to_inject(sbi, FAULT_ORPHAN)) {
		spin_unlock(&im->ino_lock);
		f2fs_show_injection_info(FAULT_ORPHAN);
		return -ENOSPC;
	}

	if (unlikely(im->ino_num >= sbi->max_orphans))
		err = -ENOSPC;
	else
		im->ino_num++;
	spin_unlock(&im->ino_lock);

	return err;
}

void hmfs_release_orphan_inode(struct f2fs_sb_info *sbi)
{
	struct inode_management *im = &sbi->im[ORPHAN_INO];

	spin_lock(&im->ino_lock);
	f2fs_bug_on(sbi, im->ino_num == 0);
	im->ino_num--;
	spin_unlock(&im->ino_lock);
}

void hmfs_add_orphan_inode(struct inode *inode)
{
	/* add new orphan ino entry into list */
	__add_ino_entry(F2FS_I_SB(inode), inode->i_ino, 0, ORPHAN_INO);
	hmfs_update_inode_page(inode);
}

void hmfs_remove_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino)
{
	/* remove orphan entry from orphan list */
	__remove_ino_entry(sbi, ino, ORPHAN_INO);
}

static int recover_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino)
{
	struct inode *inode;
	struct node_info ni;
	int err;

	inode = hmfs_iget_retry(sbi->sb, ino);
	if (IS_ERR(inode)) {
		/*
		 * there should be a bug that we can't find the entry
		 * to orphan inode.
		 */
		f2fs_bug_on(sbi, PTR_ERR(inode) == -ENOENT);
		return PTR_ERR(inode);
	}

	err = dquot_initialize(inode);
	if (err) {
		iput(inode);
		goto err_out;
	}

	clear_nlink(inode);

	/* truncate all the data during iput */
	iput(inode);

	err = f2fs_get_node_info(sbi, ino, &ni);
	if (err)
		goto err_out;

	/* ENOMEM was fully retried in hmfs_evict_inode. */
	if (ni.blk_addr != NULL_ADDR) {
		err = -EIO;
		goto err_out;
	}
	return 0;

err_out:
	set_sbi_flag(sbi, SBI_NEED_FSCK);
	hmfs_set_need_fsck_report();
	hmfs_msg(sbi->sb, KERN_WARNING,
			"%s: orphan failed (ino=%x), run fsck to fix.",
			__func__, ino);
	return err;
}

int hmfs_recover_orphan_inodes(struct f2fs_sb_info *sbi)
{
	block_t start_blk, orphan_blocks, i, j;
	unsigned int s_flags = sbi->sb->s_flags;
	int err = 0;
#ifdef CONFIG_QUOTA
	int quota_enabled;
#endif

	if (!is_set_ckpt_flags(sbi, CP_ORPHAN_PRESENT_FLAG))
		return 0;

	if (s_flags & SB_RDONLY) {
		hmfs_msg(sbi->sb, KERN_INFO, "orphan cleanup on readonly fs");
		sbi->sb->s_flags &= ~SB_RDONLY;
	}

#ifdef CONFIG_QUOTA
	/* Needed for iput() to work correctly and not trash data */
	sbi->sb->s_flags |= SB_ACTIVE;

	/*
	 * Turn on quotas which were not enabled for read-only mounts if
	 * filesystem has quota feature, so that they are updated correctly.
	 */
	quota_enabled = hmfs_enable_quota_files(sbi, s_flags & SB_RDONLY);
#endif

	start_blk = __start_cp_addr(sbi) + 1 + __cp_payload(sbi);
	orphan_blocks = __start_sum_addr(sbi) - 1 - __cp_payload(sbi);

	hmfs_ra_meta_pages(sbi, start_blk, orphan_blocks, META_CP, true);

	for (i = 0; i < orphan_blocks; i++) {
		struct page *page;
		struct f2fs_orphan_block *orphan_blk;

		page = hmfs_get_meta_page(sbi, start_blk + i);
		if (IS_ERR(page)) {
			err = PTR_ERR(page);
			goto out;
		}

		orphan_blk = (struct f2fs_orphan_block *)page_address(page);
		for (j = 0; j < le32_to_cpu(orphan_blk->entry_count); j++) {
			nid_t ino = le32_to_cpu(orphan_blk->ino[j]);
			err = recover_orphan_inode(sbi, ino);
			if (err) {
				f2fs_put_page(page, 1);
				goto out;
			}
		}
		f2fs_put_page(page, 1);
	}
	/* clear Orphan Flag */
	clear_ckpt_flags(sbi, CP_ORPHAN_PRESENT_FLAG);
out:
	set_sbi_flag(sbi, SBI_IS_RECOVERED);

#ifdef CONFIG_QUOTA
	/* Turn quotas off */
	if (quota_enabled)
		hmfs_quota_off_umount(sbi->sb);
#endif
	sbi->sb->s_flags = s_flags; /* Restore MS_RDONLY status */

	return err;
}

static void write_orphan_inodes(struct f2fs_sb_info *sbi, block_t start_blk)
{
	struct list_head *head;
	struct f2fs_orphan_block *orphan_blk = NULL;
	unsigned int nentries = 0;
	unsigned short index = 1;
	unsigned short orphan_blocks;
	struct page *page = NULL;
	struct ino_entry *orphan = NULL;
	struct inode_management *im = &sbi->im[ORPHAN_INO];

	orphan_blocks = GET_ORPHAN_BLOCKS(im->ino_num);

	/*
	 * we don't need to do spin_lock(&im->ino_lock) here, since all the
	 * orphan inode operations are covered under f2fs_lock_op().
	 * And, spin_lock should be avoided due to page operations below.
	 */
	head = &im->ino_list;

	/* loop for each orphan inode entry and write them in Jornal block */
	list_for_each_entry(orphan, head, list) {
		if (!page) {
			page = hmfs_grab_meta_page(sbi, start_blk++);
			orphan_blk =
				(struct f2fs_orphan_block *)page_address(page);
			memset(orphan_blk, 0, sizeof(*orphan_blk));
		}

		orphan_blk->ino[nentries++] = cpu_to_le32(orphan->ino);

		if (nentries == F2FS_ORPHANS_PER_BLOCK) {
			/*
			 * an orphan block is full of 1020 entries,
			 * then we need to flush current orphan blocks
			 * and bring another one in memory
			 */
			orphan_blk->blk_addr = cpu_to_le16(index);
			orphan_blk->blk_count = cpu_to_le16(orphan_blocks);
			orphan_blk->entry_count = cpu_to_le32(nentries);
			set_page_dirty(page);
			f2fs_put_page(page, 1);
			index++;
			nentries = 0;
			page = NULL;
		}
	}

	if (page) {
		orphan_blk->blk_addr = cpu_to_le16(index);
		orphan_blk->blk_count = cpu_to_le16(orphan_blocks);
		orphan_blk->entry_count = cpu_to_le32(nentries);
		set_page_dirty(page);
		f2fs_put_page(page, 1);
	}
}

static int get_checkpoint_version(struct f2fs_sb_info *sbi, block_t cp_addr,
		struct f2fs_checkpoint **cp_block, struct page **cp_page,
		unsigned long long *version, __u32 *pre_crc, size_t *crc_offset_ori)
{
	unsigned long blk_size = sbi->blocksize;
	size_t crc_offset = 0;
	__u32 crc = 0;

	*cp_page = hmfs_get_meta_page(sbi, cp_addr);
	if (IS_ERR(*cp_page))
		return PTR_ERR(*cp_page);

	*cp_block = (struct f2fs_checkpoint *)page_address(*cp_page);

	crc_offset = le32_to_cpu((*cp_block)->checksum_offset);
	if (crc_offset > (blk_size - sizeof(__le32))) {
		f2fs_put_page(*cp_page, 1);
		hmfs_msg(sbi->sb, KERN_WARNING,
			"invalid crc_offset: %zu", crc_offset);
		return -EINVAL;
	}

	crc = cur_cp_crc(*cp_block);
	if (!f2fs_crc_valid(sbi, crc, *cp_block, crc_offset)) {
		f2fs_put_page(*cp_page, 1);
		hmfs_msg(sbi->sb, KERN_WARNING, "invalid crc value");
		return -EINVAL;
	}

	*pre_crc = crc;
	*crc_offset_ori = crc_offset;
	*version = cur_cp_version(*cp_block);
	return 0;
}

static struct page *validate_checkpoint(struct f2fs_sb_info *sbi,
				block_t cp_addr, unsigned long long *version)
{
	struct page *cp_page_1 = NULL, *cp_page_2 = NULL;
	struct f2fs_checkpoint *cp_block = NULL;
	struct super_block *sb = sbi->sb;
	unsigned long long cur_version = 0, pre_version = 0;
	int err;
	__u32 pre_crc_1 = 0, pre_crc_2 = 0;
	size_t crc_offset;

	err = get_checkpoint_version(sbi, cp_addr, &cp_block,
					&cp_page_1, version, &pre_crc_1, &crc_offset);
	if (err)
		return NULL;

	if (le32_to_cpu(cp_block->cp_pack_total_block_count) >
					sbi->blocks_per_seg) {
		hmfs_msg(sbi->sb, KERN_WARNING,
			"invalid cp_pack_total_block_count:%u",
			le32_to_cpu(cp_block->cp_pack_total_block_count));
		goto invalid_cp;
	}
	pre_version = *version;

	cp_addr += le32_to_cpu(cp_block->cp_pack_total_block_count) - 1;
	err = get_checkpoint_version(sbi, cp_addr, &cp_block,
					&cp_page_2, version, &pre_crc_2, &crc_offset);
	if (err)
		goto invalid_cp;
	cur_version = *version;

	/* For write statistics */
	if (sb->s_bdev->bd_part)
		sbi->sectors_written_start =
			(u64)part_stat_read(sb->s_bdev->bd_part, sectors[1]);

	/* This part is a little bit tricky. Above all, cp1 and cp2 are both
	 * valid if we reach here, which means the whole cp pack is valid.
	 * Then 1) if cp2 is different from cp1, accumulated write IO stat is
	 * stored right before CRC of cp2; 2) if cp2 is the same as cp1,
	 * there is no stat stored in cp2.
	 */
	if (pre_crc_1 != pre_crc_2) {
		sbi->kbytes_written = le64_to_cpu(*(__le64 *)((unsigned char *)cp_block
			+ crc_offset - SIZEOF_KBYTES_WRITTEN));
		sbi->blocks_gc_written = le64_to_cpu(*(__le64 *)((unsigned char *)cp_block
			+ crc_offset - SIZEOF_KBYTES_WRITTEN - sizeof(__le64)));
	}

	if (cur_version == pre_version) {
		*version = cur_version;
		f2fs_put_page(cp_page_2, 1);
		return cp_page_1;
	}
	f2fs_put_page(cp_page_2, 1);
invalid_cp:
	f2fs_put_page(cp_page_1, 1);
	return NULL;
}

int hmfs_get_valid_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp_block;
	struct f2fs_super_block *fsb = sbi->raw_super;
	struct page *cp1, *cp2, *cur_page;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cp1_version = 0, cp2_version = 0;
	unsigned long long cp_start_blk_no;
	unsigned int cp_blks = 1 + __cp_payload(sbi);
	block_t cp_blk_no;
	int i;

	sbi->ckpt = f2fs_kzalloc(sbi, array_size(blk_size, cp_blks),
				 GFP_KERNEL);
	if (!sbi->ckpt)
		return -ENOMEM;
	/*
	 * Finding out valid cp block involves read both
	 * sets( cp pack1 and cp pack 2)
	 */
	cp_start_blk_no = le32_to_cpu(fsb->cp_blkaddr);
	cp1 = validate_checkpoint(sbi, cp_start_blk_no, &cp1_version);

	/* The second checkpoint pack should start at the next segment */
	cp_start_blk_no += ((unsigned long long)1) <<
				le32_to_cpu(fsb->log_blocks_per_seg);
	cp2 = validate_checkpoint(sbi, cp_start_blk_no, &cp2_version);

	if (cp1 && cp2) {
		if (ver_after(cp2_version, cp1_version))
			cur_page = cp2;
		else
			cur_page = cp1;
	} else if (cp1) {
		cur_page = cp1;
	} else if (cp2) {
		cur_page = cp2;
	} else {
		goto fail_no_cp;
	}

	cp_block = (struct f2fs_checkpoint *)page_address(cur_page);
	memcpy(sbi->ckpt, cp_block, blk_size);
	sbi->ckpt_crc = cur_cp_crc(cp_block);

	if (cur_page == cp1)
		sbi->cur_cp_pack = 1;
	else
		sbi->cur_cp_pack = 2;

	/* Sanity checking of checkpoint */
	if (hmfs_sanity_check_ckpt(sbi))
		goto free_fail_no_cp;

	if (cp_blks <= 1)
		goto done;

	cp_blk_no = le32_to_cpu(fsb->cp_blkaddr);
	if (cur_page == cp2)
		cp_blk_no += 1 << le32_to_cpu(fsb->log_blocks_per_seg);

	for (i = 1; i < cp_blks; i++) {
		void *sit_bitmap_ptr;
		unsigned char *ckpt = (unsigned char *)sbi->ckpt;

		cur_page = hmfs_get_meta_page(sbi, cp_blk_no + i);
		if (IS_ERR(cur_page))
			goto free_fail_no_cp;
		sit_bitmap_ptr = page_address(cur_page);
		memcpy(ckpt + i * blk_size, sit_bitmap_ptr, blk_size);
		f2fs_put_page(cur_page, 1);
	}
done:
	f2fs_put_page(cp1, 1);
	f2fs_put_page(cp2, 1);
	return 0;

free_fail_no_cp:
	f2fs_put_page(cp1, 1);
	f2fs_put_page(cp2, 1);
fail_no_cp:
	kfree(sbi->ckpt);
	return -EINVAL;
}

static void __add_dirty_inode(struct inode *inode, enum inode_type type)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int flag = (type == DIR_INODE) ? FI_DIRTY_DIR : FI_DIRTY_FILE;

	if (is_inode_flag_set(inode, flag))
		return;

	set_inode_flag(inode, flag);
	if (!f2fs_is_volatile_file(inode))
		list_add_tail(&F2FS_I(inode)->dirty_list,
						&sbi->inode_list[type]);
	stat_inc_dirty_inode(sbi, type);
}

static void __remove_dirty_inode(struct inode *inode, enum inode_type type)
{
	int flag = (type == DIR_INODE) ? FI_DIRTY_DIR : FI_DIRTY_FILE;

	if (get_dirty_pages(inode) || !is_inode_flag_set(inode, flag))
		return;

	list_del_init(&F2FS_I(inode)->dirty_list);
	clear_inode_flag(inode, flag);
	stat_dec_dirty_inode(F2FS_I_SB(inode), type);
}

void hmfs_update_dirty_page(struct inode *inode, struct page *page)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	enum inode_type type = S_ISDIR(inode->i_mode) ? DIR_INODE : FILE_INODE;

	if (!S_ISDIR(inode->i_mode) && !S_ISREG(inode->i_mode) &&
			!S_ISLNK(inode->i_mode))
		return;

	spin_lock(&sbi->inode_lock[type]);
	if (type != FILE_INODE || test_opt(sbi, DATA_FLUSH))
		__add_dirty_inode(inode, type);
	inode_inc_dirty_pages(inode);
	spin_unlock(&sbi->inode_lock[type]);

	f2fs_set_page_private(page, 0);
	f2fs_trace_pid(page);
}

void hmfs_remove_dirty_inode(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	enum inode_type type = S_ISDIR(inode->i_mode) ? DIR_INODE : FILE_INODE;

	if (!S_ISDIR(inode->i_mode) && !S_ISREG(inode->i_mode) &&
			!S_ISLNK(inode->i_mode))
		return;

	if (type == FILE_INODE && !test_opt(sbi, DATA_FLUSH))
		return;

	spin_lock(&sbi->inode_lock[type]);
	__remove_dirty_inode(inode, type);
	spin_unlock(&sbi->inode_lock[type]);
}

int hmfs_sync_dirty_inodes(struct f2fs_sb_info *sbi, enum inode_type type)
{
	struct list_head *head;
	struct inode *inode;
	struct f2fs_inode_info *fi;
	bool is_dir = (type == DIR_INODE);
	unsigned long ino = 0;
	unsigned long cycles = 0;

	trace_hmfs_sync_dirty_inodes_enter(sbi->sb, is_dir,
				get_pages(sbi, is_dir ?
				F2FS_DIRTY_DENTS : F2FS_DIRTY_DATA));
retry:
	cycles++;
	if (cycles % 10000 == 0)
		hmfs_msg(sbi->sb, KERN_ALERT,
			"%s:%d: too many cycles(%lu), maybe an infinite loop!",
				__func__, __LINE__, cycles);
	if (unlikely(f2fs_cp_error(sbi))) {
		trace_hmfs_sync_dirty_inodes_exit(sbi->sb, is_dir,
				get_pages(sbi, is_dir ?
				F2FS_DIRTY_DENTS : F2FS_DIRTY_DATA));
		return -EIO;
	}

	spin_lock(&sbi->inode_lock[type]);

	head = &sbi->inode_list[type];
	if (list_empty(head)) {
		spin_unlock(&sbi->inode_lock[type]);
		trace_hmfs_sync_dirty_inodes_exit(sbi->sb, is_dir,
				get_pages(sbi, is_dir ?
				F2FS_DIRTY_DENTS : F2FS_DIRTY_DATA));
		return 0;
	}
	fi = list_first_entry(head, struct f2fs_inode_info, dirty_list);
	inode = igrab(&fi->vfs_inode);
	spin_unlock(&sbi->inode_lock[type]);
	if (inode) {
		unsigned long cur_ino = inode->i_ino;

		if (is_dir)
			F2FS_I(inode)->cp_task = current;

		filemap_fdatawrite(inode->i_mapping);

		if (is_dir)
			F2FS_I(inode)->cp_task = NULL;

		iput(inode);
		/* We need to give cpu to another writers. */
		if (ino == cur_ino)
			cond_resched();
		else
			ino = cur_ino;
	} else {
		/*
		 * We should submit bio, since it exists several
		 * wribacking dentry pages in the freeing inode.
		 */
		hmfs_submit_merged_write(sbi, DATA);
		cond_resched();
	}
	goto retry;
}

int hmfs_sync_inode_meta(struct f2fs_sb_info *sbi)
{
	struct list_head *head = &sbi->inode_list[DIRTY_META];
	struct inode *inode;
	struct f2fs_inode_info *fi;
	s64 total = get_pages(sbi, F2FS_DIRTY_IMETA);

	while (total--) {
		if (unlikely(f2fs_cp_error(sbi)))
			return -EIO;

		spin_lock(&sbi->inode_lock[DIRTY_META]);
		if (list_empty(head)) {
			spin_unlock(&sbi->inode_lock[DIRTY_META]);
			return 0;
		}
		fi = list_first_entry(head, struct f2fs_inode_info,
							gdirty_list);
		inode = igrab(&fi->vfs_inode);
		spin_unlock(&sbi->inode_lock[DIRTY_META]);
		if (inode) {
			sync_inode_metadata(inode, 0);

			/* it's on eviction */
			if (is_inode_flag_set(inode, FI_DIRTY_INODE))
				hmfs_update_inode_page(inode);
			iput(inode);
		}
	}
	return 0;
}

static void __prepare_cp_block(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	nid_t last_nid = nm_i->next_scan_nid;

	next_free_nid(sbi, &last_nid);
	ckpt->valid_block_count = cpu_to_le64(valid_user_blocks(sbi));
	ckpt->valid_node_count = cpu_to_le32(valid_node_count(sbi));
	ckpt->valid_inode_count = cpu_to_le32(valid_inode_count(sbi));
	ckpt->next_free_nid = cpu_to_le32(last_nid);
}

static bool __need_flush_quota(struct f2fs_sb_info *sbi)
{
	if (!is_journalled_quota(sbi))
		return false;
	if (is_sbi_flag_set(sbi, SBI_QUOTA_SKIP_FLUSH))
		return false;
	if (is_sbi_flag_set(sbi, SBI_QUOTA_NEED_REPAIR))
		return false;
	if (is_sbi_flag_set(sbi, SBI_QUOTA_NEED_FLUSH))
		return true;
	if (get_pages(sbi, F2FS_DIRTY_QDATA))
		return true;
	return false;
}

/*
 * Freeze all the FS-operations for checkpoint.
 */
static int block_operations(struct f2fs_sb_info *sbi)
{
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = LONG_MAX,
		.for_reclaim = 0,
	};
	struct blk_plug plug;
	int err = 0, cnt = 0;
	unsigned long sync_inode_cycles = 0;
	unsigned long sync_imeta_cycles = 0;
	unsigned long sync_nodes_cycles = 0;

	blk_start_plug(&plug);

	/*
	 * Let's flush inline_data in dirty node pages.
	 */
	hmfs_flush_inline_data(sbi);

retry_flush_quotas:
	if (__need_flush_quota(sbi)) {
		int locked;

		if (++cnt > DEFAULT_RETRY_QUOTA_FLUSH_COUNT) {
			set_sbi_flag(sbi, SBI_QUOTA_SKIP_FLUSH);
			f2fs_lock_all(sbi);
			goto retry_flush_dents;
		}
		clear_sbi_flag(sbi, SBI_QUOTA_NEED_FLUSH);

		/* only failed during mount/umount/freeze/quotactl */
		locked = down_read_trylock(&sbi->sb->s_umount);
		hmfs_quota_sync(sbi->sb, -1);
		if (locked)
			up_read(&sbi->sb->s_umount);
	}

	f2fs_lock_all(sbi);
	if (__need_flush_quota(sbi)) {
		f2fs_unlock_all(sbi);
		cond_resched();
		goto retry_flush_quotas;
	}

retry_flush_dents:
	/* write all the dirty dentry pages */
	if (get_pages(sbi, F2FS_DIRTY_DENTS)) {
		sync_inode_cycles++;
		if (sync_inode_cycles % 10000 == 0)
			hmfs_msg(sbi->sb, KERN_ALERT,
				"%s:%d: too many sync inode cycles(%lu), maybe an infinite loop!",
					__func__, __LINE__, sync_inode_cycles);

		f2fs_unlock_all(sbi);
		err = hmfs_sync_dirty_inodes(sbi, DIR_INODE);
		if (err)
			goto out;
		cond_resched();
		goto retry_flush_quotas;
	}

	/*
	 * POR: we should ensure that there are no dirty node pages
	 * until finishing nat/sit flush. inode->i_blocks can be updated.
	 */
	down_write(&sbi->node_change);

	if (__need_flush_quota(sbi)) {
		up_write(&sbi->node_change);
		f2fs_unlock_all(sbi);
		goto retry_flush_quotas;
	}

	if (get_pages(sbi, F2FS_DIRTY_IMETA)) {
		sync_imeta_cycles++;
		if (sync_imeta_cycles % 10000 == 0)
			hmfs_msg(sbi->sb, KERN_ALERT,
				"%s:%d: too many sync imeta cycles(%lu), maybe an infinite loop!",
					__func__, __LINE__, sync_imeta_cycles);
		up_write(&sbi->node_change);
		f2fs_unlock_all(sbi);
		err = hmfs_sync_inode_meta(sbi);
		if (err)
			goto out;
		cond_resched();
		goto retry_flush_quotas;
	}

retry_flush_nodes:
	down_write(&sbi->node_write);

	if (get_pages(sbi, F2FS_DIRTY_NODES)) {
		sync_nodes_cycles++;
		if (sync_nodes_cycles % 10000 == 0)
			hmfs_msg(sbi->sb, KERN_ALERT,
				"%s:%d: too many sync nodes cycles(%lu), maybe an infinite loop!",
					__func__, __LINE__, sync_nodes_cycles);
		up_write(&sbi->node_write);
		atomic_inc(&sbi->wb_sync_req[NODE]);
		err = hmfs_sync_node_pages(sbi, &wbc, false, FS_CP_NODE_IO);
		atomic_dec(&sbi->wb_sync_req[NODE]);
		if (err) {
			up_write(&sbi->node_change);
			f2fs_unlock_all(sbi);
			goto out;
		}
		cond_resched();
		goto retry_flush_nodes;
	}

	/*
	 * sbi->node_change is used only for AIO write_begin path which produces
	 * dirty node blocks and some checkpoint values by block allocation.
	 */
#ifdef CONFIG_HMFS_CHECK_FS
	atomic_set(&sbi->in_cp, 1);
#endif
	__prepare_cp_block(sbi);
	up_write(&sbi->node_change);
out:
	blk_finish_plug(&plug);
	return err;
}

static void unblock_operations(struct f2fs_sb_info *sbi)
{
#ifdef CONFIG_HMFS_CHECK_FS
	atomic_set(&sbi->in_cp, 0);
#endif
	up_write(&sbi->node_write);
	f2fs_unlock_all(sbi);
}

void hmfs_wait_on_all_pages_writeback(struct f2fs_sb_info *sbi)
{
	DEFINE_WAIT(wait);
	int ret;

	for (;;) {
		prepare_to_wait(&sbi->cp_wait, &wait, TASK_UNINTERRUPTIBLE);

		if (!get_pages(sbi, F2FS_WB_CP_DATA))
			break;

		if (unlikely(f2fs_cp_error(sbi)))
			break;

		ret = io_schedule_timeout(5*HZ);
		if (ret == 0) {
			struct hd_struct *part = sbi->sb->s_bdev->bd_part;
			unsigned int inflight[2];

			part_in_flight_rw(part, inflight);
			printk(KERN_EMERG "%s timeout writeback pages[%lld, %lld]\r\n",
				__func__, get_pages(sbi, F2FS_WB_CP_DATA),
				get_pages(sbi, F2FS_WB_DATA));
			printk(KERN_EMERG "%s read io in flight %d, write io in flight %d \r\n ",
				__func__, inflight[0], inflight[1]);
		}
	}
	finish_wait(&sbi->cp_wait, &wait);
}

static void update_ckpt_flags(struct f2fs_sb_info *sbi, struct cp_control *cpc)
{
	unsigned long orphan_num = sbi->im[ORPHAN_INO].ino_num;
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	unsigned long flags;

	spin_lock_irqsave(&sbi->cp_lock, flags);

	if ((cpc->reason & CP_UMOUNT) &&
			le32_to_cpu(ckpt->cp_pack_total_block_count) >
			sbi->blocks_per_seg - NM_I(sbi)->nat_bits_blocks)
		disable_nat_bits(sbi, false);

	if (cpc->reason & CP_TRIMMED)
		__set_ckpt_flags(ckpt, CP_TRIMMED_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_TRIMMED_FLAG);

	if (cpc->reason & CP_UMOUNT)
		__set_ckpt_flags(ckpt, CP_UMOUNT_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_UMOUNT_FLAG);

	if (cpc->reason & CP_FASTBOOT)
		__set_ckpt_flags(ckpt, CP_FASTBOOT_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_FASTBOOT_FLAG);

	if (orphan_num)
		__set_ckpt_flags(ckpt, CP_ORPHAN_PRESENT_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_ORPHAN_PRESENT_FLAG);

	if (is_sbi_flag_set(sbi, SBI_NEED_FSCK))
		__set_ckpt_flags(ckpt, CP_FSCK_FLAG);

	if (is_sbi_flag_set(sbi, SBI_IS_RESIZEFS))
		__set_ckpt_flags(ckpt, CP_RESIZEFS_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_RESIZEFS_FLAG);

	if (is_sbi_flag_set(sbi, SBI_CP_DISABLED))
		__set_ckpt_flags(ckpt, CP_DISABLED_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_DISABLED_FLAG);

	if (is_sbi_flag_set(sbi, SBI_QUOTA_SKIP_FLUSH))
		__set_ckpt_flags(ckpt, CP_QUOTA_NEED_FSCK_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_QUOTA_NEED_FSCK_FLAG);

	if (is_sbi_flag_set(sbi, SBI_QUOTA_NEED_REPAIR))
		__set_ckpt_flags(ckpt, CP_QUOTA_NEED_FSCK_FLAG);

	/* set this flag to activate crc|cp_ver for recovery */
	__set_ckpt_flags(ckpt, CP_CRC_RECOVERY_FLAG);
	__clear_ckpt_flags(ckpt, CP_NOCRC_RECOVERY_FLAG);

	spin_unlock_irqrestore(&sbi->cp_lock, flags);
}

static void commit_checkpoint(struct f2fs_sb_info *sbi,
	void *src, block_t blk_addr)
{
	struct writeback_control wbc = {
		.for_reclaim = 0,
	};

	/*
	 * pagevec_lookup_tag and lock_page again will take
	 * some extra time. Therefore, hmfs_update_meta_pages and
	 * hmfs_sync_meta_pages are combined in this function.
	 */
	struct page *page = hmfs_grab_meta_page(sbi, blk_addr);
	int err;

	hmfs_wait_on_page_writeback(page, META, true);
	f2fs_bug_on(sbi, PageWriteback(page));

	memcpy(page_address(page), src, PAGE_SIZE);

	set_page_dirty(page);
	if (unlikely(!clear_page_dirty_for_io(page)))
		f2fs_bug_on(sbi, 1);

	/* writeout cp pack 2 page */
	err = __f2fs_write_meta_page(page, &wbc, FS_CP_META_IO);
	if (unlikely(err && f2fs_cp_error(sbi))) {
		f2fs_put_page(page, 1);
		return;
	}

	f2fs_bug_on(sbi, err);
	f2fs_put_page(page, 0);

	/* submit checkpoint (with barrier if NOBARRIER is not set) */
	hmfs_submit_merged_write(sbi, META_FLUSH);
}

u64 f2fs_cp_flush_meta_time, f2fs_cp_flush_meta_begin, f2fs_cp_flush_meta_end;

static int do_checkpoint(struct f2fs_sb_info *sbi, struct cp_control *cpc)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned long orphan_num = sbi->im[ORPHAN_INO].ino_num, flags;
	block_t start_blk, dm_start_blk;
	unsigned int data_sum_blocks, orphan_blocks;
	__u32 crc32 = 0;
	int i;
	int cp_payload_blks = __cp_payload(sbi);
	struct super_block *sb = sbi->sb;
	struct curseg_info *seg_i = CURSEG_I(sbi, CURSEG_HOT_NODE);
	u64 kbytes_written;
	int err;

	/* Flush all the NAT/SIT pages */
	f2fs_cp_flush_meta_begin = local_clock();
	while (get_pages(sbi, F2FS_DIRTY_META)) {
		hmfs_sync_meta_pages(sbi, META, LONG_MAX, FS_CP_META_IO);
		if (unlikely(f2fs_cp_error(sbi)))
			break;
	}
	f2fs_cp_flush_meta_end = local_clock();
	f2fs_cp_flush_meta_time += f2fs_cp_flush_meta_end - f2fs_cp_flush_meta_begin;

	/*
	 * modify checkpoint
	 * version number is already updated
	 */
	ckpt->elapsed_time = cpu_to_le64(get_mtime(sbi, true));
	ckpt->free_segment_count = cpu_to_le32(free_segments(sbi));
	for (i = 0; i < NR_CURSEG_NODE_TYPE; i++) {
		ckpt->cur_node_segno[i] =
			cpu_to_le32(curseg_segno(sbi, i + CURSEG_HOT_NODE));
		ckpt->cur_node_blkoff[i] =
			cpu_to_le16(curseg_blkoff(sbi, i + CURSEG_HOT_NODE));
		ckpt->alloc_type[i + CURSEG_HOT_NODE] =
				curseg_alloc_type(sbi, i + CURSEG_HOT_NODE);
	}
	for (i = 0; i < NR_CURSEG_DATA_TYPE; i++) {
		ckpt->cur_data_segno[i] =
			cpu_to_le32(curseg_segno(sbi, i + CURSEG_HOT_DATA));
		ckpt->cur_data_blkoff[i] =
			cpu_to_le16(curseg_blkoff(sbi, i + CURSEG_HOT_DATA));
		ckpt->alloc_type[i + CURSEG_HOT_DATA] =
				curseg_alloc_type(sbi, i + CURSEG_HOT_DATA);
	}
	for (i = 0; i < NR_CURSEG_DM_TYPE; i++) {
		if (!is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG))
			continue;
		ckpt->cur_dm_segno[i] =
			cpu_to_le32(curseg_segno(sbi, i + CURSEG_DATA_MOVE1));
		ckpt->cur_dm_blkoff[i] =
			cpu_to_le16(curseg_blkoff(sbi, i + CURSEG_DATA_MOVE1));
		ckpt->alloc_type[i + CURSEG_DATA_MOVE1] =
			curseg_alloc_type(sbi, i + CURSEG_DATA_MOVE1);
	}

	/* 2 cp  + n data seg summary + orphan inode blocks */
	data_sum_blocks = hmfs_npages_for_summary_flush(sbi, false);
	spin_lock_irqsave(&sbi->cp_lock, flags);
	if (data_sum_blocks < NR_CURSEG_DATA_TYPE)
		__set_ckpt_flags(ckpt, CP_COMPACT_SUM_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_COMPACT_SUM_FLAG);
	spin_unlock_irqrestore(&sbi->cp_lock, flags);

	orphan_blocks = GET_ORPHAN_BLOCKS(orphan_num);
	ckpt->cp_pack_start_sum = cpu_to_le32(1 + cp_payload_blks +
			orphan_blocks);

	if (__remain_node_summaries(cpc->reason))
		ckpt->cp_pack_total_block_count = cpu_to_le32(F2FS_CP_PACKS+
				cp_payload_blks + data_sum_blocks +
				orphan_blocks + NR_CURSEG_NODE_TYPE);
	else
		ckpt->cp_pack_total_block_count = cpu_to_le32(F2FS_CP_PACKS +
				cp_payload_blks + data_sum_blocks +
				orphan_blocks);

	if (is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG))
		ckpt->cp_pack_total_block_count =
				cpu_to_le32(NR_CURSEG_DM_TYPE +
				le32_to_cpu(ckpt->cp_pack_total_block_count));

#ifdef CONFIG_HMFS_JOURNAL_APPEND
	if (need_append_nat_journal(CURSEG_I(sbi, CURSEG_HOT_DATA)->journal))
		__set_ckpt_flags(ckpt, CP_APPEND_NAT_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_APPEND_NAT_FLAG);

	if (need_append_sit_journal(CURSEG_I(sbi, CURSEG_COLD_DATA)->journal))
		__set_ckpt_flags(ckpt, CP_APPEND_SIT_FLAG);
	else
		__clear_ckpt_flags(ckpt, CP_APPEND_SIT_FLAG);
#endif

	/* update ckpt flag for checkpoint */
	update_ckpt_flags(sbi, cpc);

	/* update SIT/NAT bitmap */
	get_sit_bitmap(sbi, __bitmap_ptr(sbi, SIT_BITMAP));
	get_nat_bitmap(sbi, __bitmap_ptr(sbi, NAT_BITMAP));

	crc32 = f2fs_crc32(sbi, ckpt, le32_to_cpu(ckpt->checksum_offset));
	*((__le32 *)((unsigned char *)ckpt +
				le32_to_cpu(ckpt->checksum_offset)))
				= cpu_to_le32(crc32);
	sbi->ckpt_crc = (u64)crc32;

	start_blk = __start_cp_next_addr(sbi);

	/* write nat bits */
	if (enabled_nat_bits(sbi, cpc)) {
		__u64 cp_ver = cur_cp_version(ckpt);
		block_t blk;

		cp_ver |= ((__u64)crc32 << 32);
		*(__le64 *)nm_i->nat_bits = cpu_to_le64(cp_ver);

		blk = start_blk + sbi->blocks_per_seg - nm_i->nat_bits_blocks;
		for (i = 0; i < nm_i->nat_bits_blocks; i++)
			hmfs_update_meta_page(sbi, nm_i->nat_bits +
					(i << F2FS_BLKSIZE_BITS), blk + i);

		/* Flush all the NAT BITS pages */
		while (get_pages(sbi, F2FS_DIRTY_META)) {
			hmfs_sync_meta_pages(sbi, META, LONG_MAX,
							FS_CP_META_IO);
			if (unlikely(f2fs_cp_error(sbi)))
				break;
		}
	}

	/* write out checkpoint buffer at block 0 */
	hmfs_update_meta_page(sbi, ckpt, start_blk++);

	for (i = 1; i < 1 + cp_payload_blks; i++)
		hmfs_update_meta_page(sbi, (char *)ckpt + i * F2FS_BLKSIZE,
							start_blk++);

	if (orphan_num) {
		write_orphan_inodes(sbi, start_blk);
		start_blk += orphan_blocks;
	}

	hmfs_write_data_summaries(sbi, start_blk);
	start_blk += data_sum_blocks;

	/* Record write statistics in the hot node summary */
	kbytes_written = sbi->kbytes_written;
	if (sb->s_bdev->bd_part)
		kbytes_written += BD_PART_WRITTEN(sbi);

	seg_i->journal->info.kbytes_written = cpu_to_le64(kbytes_written);

	if (__remain_node_summaries(cpc->reason)) {
		hmfs_write_node_summaries(sbi, start_blk);
		start_blk += NR_CURSEG_NODE_TYPE;
	}

	if (is_set_ckpt_flags(sbi, CP_DATAMOVE_FLAG)) {
		hmfs_write_datamove_summaries(sbi, start_blk);
		start_blk += NR_CURSEG_DM_TYPE;
	}

#ifdef CONFIG_HMFS_JOURNAL_APPEND
	write_append_journal(sbi, start_blk + 1);
#endif

	dm_start_blk = start_blk + 1;
#ifdef CONFIG_HMFS_JOURNAL_APPEND
	if (is_set_ckpt_flags(sbi, CP_APPEND_NAT_FLAG))
		dm_start_blk++;
	if (is_set_ckpt_flags(sbi, CP_APPEND_SIT_FLAG))
		dm_start_blk++;
#endif
	/* write datamove unverified entries */
	hmfs_datamove_write_entry(sbi, dm_start_blk);

	/* For write statistics */
	kbytes_written = sbi->kbytes_written;
	if (sb->s_bdev->bd_part)
		kbytes_written += BD_PART_WRITTEN(sbi);

	/* Lifetime write IO stat is stored right before CRC of cp2,
	 * so cp2 is NOT exactly the same as cp1. However, contents
	 * between cp1 and cp2 are considered valid as long as
	 * 1) cp1 and cp2 are both valid, and
	 * 2) the versions of cp1 and cp2 are matched.
	 */
	*(__le64 *)((unsigned char *)ckpt +
		le32_to_cpu(ckpt->checksum_offset) - SIZEOF_KBYTES_WRITTEN)
			= cpu_to_le64(kbytes_written);

	/* For gc write statistics */
	*(__le64 *)((unsigned char *)ckpt + le32_to_cpu(ckpt->checksum_offset) -
		SIZEOF_KBYTES_WRITTEN - sizeof(__le64))
			= cpu_to_le64(sbi->blocks_gc_written);

	/* Calculate CRC of cp2 */
	crc32 = f2fs_crc32(sbi, ckpt, le32_to_cpu(ckpt->checksum_offset));
	*(__le32 *)((unsigned char *)ckpt +
				le32_to_cpu(ckpt->checksum_offset))
				= cpu_to_le32(crc32);

	/* update user_block_counts */
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	percpu_counter_set(&sbi->alloc_valid_block_count, 0);

	/* Here, we have one bio having CP pack except cp pack 2 page */
	hmfs_sync_meta_pages(sbi, META, LONG_MAX, FS_CP_META_IO);

	/* wait for previous submitted meta pages writeback */
	hmfs_wait_on_all_pages_writeback(sbi);

	/* flush all device cache */
	err = hmfs_flush_device_cache(sbi);
	if (err)
		return err;

	/* barrier and flush checkpoint cp pack 2 page if it can */
	commit_checkpoint(sbi, ckpt, start_blk);
	hmfs_wait_on_all_pages_writeback(sbi);

	/*
	 * invalidate intermediate page cache borrowed from meta inode which are
	 * used for migration of encrypted or compressed inode's blocks.
	 */
	if (f2fs_sb_has_encrypt(sbi) || f2fs_sb_has_compression(sbi))
		invalidate_mapping_pages(META_MAPPING(sbi),
				MAIN_BLKADDR(sbi), MAX_BLKADDR(sbi) - 1);

	hmfs_release_ino_entry(sbi, false);

	hmfs_reset_fsync_node_info(sbi);

	clear_sbi_flag(sbi, SBI_IS_DIRTY);
	clear_sbi_flag(sbi, SBI_NEED_CP);
	clear_sbi_flag(sbi, SBI_QUOTA_SKIP_FLUSH);
	sbi->unusable_block_count = 0;
	__set_cp_next_pack(sbi);

	/*
	 * redirty superblock if metadata like node page or inode cache is
	 * updated during writing checkpoint.
	 */
	if (get_pages(sbi, F2FS_DIRTY_NODES) ||
			get_pages(sbi, F2FS_DIRTY_IMETA))
		set_sbi_flag(sbi, SBI_IS_DIRTY);

	f2fs_bug_on(sbi, get_pages(sbi, F2FS_DIRTY_DENTS));

	return unlikely(f2fs_cp_error(sbi)) ? -EIO : 0;
}

#ifdef CONFIG_HMFS_CHECK_FS
static void report_bdev_access_info(struct f2fs_sb_info *sbi)
{
	struct access_timestamp *at, *tmp;
	int open_cnt = atomic_read(&bdev_access_info.open_cnt);
#ifdef CONFIG_HUAWEI_HMFS_DSM
	int size = 0;
	char *buf = NULL;
#endif

	if (!open_cnt)
		return;

	print_bdev_access_info();
	atomic_set(&bdev_access_info.open_cnt, 0);

#ifdef CONFIG_HUAWEI_HMFS_DSM
	if(!hmfs_dclient || dsm_client_ocuppy(hmfs_dclient))
		return;

	buf = kzalloc(1024, GFP_NOFS);
	if (!buf)
		goto do_dmd_report;

	spin_lock(&bdev_access_info.lock);
	list_for_each_entry_safe(at, tmp, &bdev_access_info.access_list, list) {
		list_del(&at->list);
		size += snprintf(buf + size, 1024 - size, "%ld, ",
				at->time.tv_sec);
		kfree(at);
		if (size >= 1024) {
			buf[1023] = '\0';
			break;
		}
	}
	spin_unlock(&bdev_access_info.lock);

do_dmd_report:
        dsm_client_record(hmfs_dclient, "HMFS: open_cnt %d, %s\n",
                                open_cnt, buf ? : "null");
        dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK);
        if (buf)
		kfree(buf);
#endif
	spin_lock(&bdev_access_info.lock);
	list_for_each_entry_safe(at, tmp, &bdev_access_info.access_list, list) {
		list_del(&at->list);
		kfree(at);
	}
	spin_unlock(&bdev_access_info.lock);
}
#endif

/*
 * We guarantee that this checkpoint procedure will not fail.
 */
int hmfs_write_checkpoint(struct f2fs_sb_info *sbi, struct cp_control *cpc)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	unsigned long long ckpt_ver;
	int i;
	int err = 0;
	u64 cp_begin = 0, cp_end, cp_submit_end = 0, discard_begin, discard_end;

	if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED))) {
		if (cpc->reason != CP_PAUSE)
			return 0;
		hmfs_msg(sbi->sb, KERN_WARNING,
				"Start checkpoint disabled!");
	}
	hmfs_wait_all_dm_complete(sbi);
	mutex_lock(&sbi->cp_mutex);

	if (!is_sbi_flag_set(sbi, SBI_IS_DIRTY) &&
		((cpc->reason & CP_FASTBOOT) || (cpc->reason & CP_SYNC) ||
		((cpc->reason & CP_DISCARD) && !sbi->discard_blks)))
		goto out;
	if (unlikely(f2fs_cp_error(sbi))) {
		err = -EIO;
		goto out;
	}
	if (f2fs_readonly(sbi->sb)) {
		err = -EROFS;
		goto out;
	}

	/* try to do force flush for datamove */
	hmfs_datamove_force_flush(sbi, cpc->reason & CP_UMOUNT);

	trace_hmfs_write_checkpoint(sbi->sb, cpc->reason, "start block_ops");
#ifdef CONFIG_HMFS_CHECK_FS
	report_bdev_access_info(sbi);
#endif

	cp_begin = local_clock();
	err = block_operations(sbi);
	if (err)
		goto out;

	trace_hmfs_write_checkpoint(sbi->sb, cpc->reason, "finish block_ops");

	hmfs_flush_merged_writes(sbi);
	cp_submit_end = local_clock();

	/* this is the case of multiple fstrims without any changes */
	if (cpc->reason & CP_DISCARD) {
		if (!hmfs_exist_trim_candidates(sbi, cpc)) {
			unblock_operations(sbi);
			goto out;
		}

		if (NM_I(sbi)->nat_cnt[DIRTY_NAT] == 0 &&
				SIT_I(sbi)->dirty_sentries == 0 &&
				prefree_segments(sbi) == 0) {
			f2fs_cp_flush_meta_time = 0;
			f2fs_cp_flush_meta_begin = local_clock();
			hmfs_wait_all_dm_complete(sbi);
			hmfs_flush_sit_entries(sbi, cpc);
			discard_begin = local_clock();
			hmfs_clear_prefree_segments(sbi, cpc);
			discard_end = local_clock();
			bd_mutex_lock(&sbi->bd_mutex);
			max_bd_val(sbi, max_cp_discard_time,
				   discard_end - discard_begin);
			bd_mutex_unlock(&sbi->bd_mutex);
			unblock_operations(sbi);
			goto out;
		}
	}

	/*
	 * update checkpoint pack index
	 * Increase the version number so that
	 * SIT entries and seg summaries are written at correct place
	 */
	ckpt_ver = cur_cp_version(ckpt);
	ckpt->checkpoint_ver = cpu_to_le64(++ckpt_ver);

	/* write cached NAT/SIT entries to NAT/SIT area */
	f2fs_cp_flush_meta_time = 0;
	f2fs_cp_flush_meta_begin = local_clock();
	err = hmfs_flush_nat_entries(sbi, cpc);
	if (err)
		goto stop;

	hmfs_wait_all_dm_complete(sbi);
	hmfs_flush_sit_entries(sbi, cpc);
	/* flush summary info in virtual log header */
	hmfs_store_virtual_curseg_summary(sbi);
	hmfs_restore_virtual_curseg_status(sbi, true);

	f2fs_cp_flush_meta_end = local_clock();
	f2fs_cp_flush_meta_time = f2fs_cp_flush_meta_end - f2fs_cp_flush_meta_begin;

	/* unlock all the fs_lock[] in do_checkpoint() */
	err = do_checkpoint(sbi, cpc);
	if (err)
		hmfs_release_discard_addrs(sbi);
	else {
		discard_begin = local_clock();
		hmfs_clear_prefree_segments(sbi, cpc);
		discard_end = local_clock();
		bd_mutex_lock(&sbi->bd_mutex);
		max_bd_val(sbi, max_cp_discard_time,
			   discard_end - discard_begin);
		bd_mutex_unlock(&sbi->bd_mutex);
		for (i = 0; i < NR_CURSEG_TYPE; ++i)
			atomic_set(&sbi->oob_wr_cnt[i], 0);
	}
stop:
	hmfs_restore_virtual_curseg_status(sbi, false);
	unblock_operations(sbi);
	stat_inc_cp_count(sbi->stat_info);

	if (cpc->reason & CP_RECOVERY)
		hmfs_msg(sbi->sb, KERN_NOTICE,
			"checkpoint: version = %llx", ckpt_ver);

	/* do checkpoint periodically */
	f2fs_update_time(sbi, CP_TIME);
	trace_hmfs_write_checkpoint(sbi->sb, cpc->reason, "finish checkpoint");
out:
	mutex_unlock(&sbi->cp_mutex);
	if (cp_begin && !err) {
		cp_end = local_clock();
		bd_mutex_lock(&sbi->bd_mutex);
		inc_bd_val(sbi, cp_succ_cnt, 1);
		max_bd_val(sbi, max_cp_submit_time, cp_submit_end - cp_begin);
		inc_bd_val(sbi, cp_time, cp_end - cp_begin);
		max_bd_val(sbi, max_cp_time, cp_end - cp_begin);
		max_bd_val(sbi, max_f2fs_cp_flush_meta_time, f2fs_cp_flush_meta_time);
		bd_mutex_unlock(&sbi->bd_mutex);
	}
	return err;
}

void hmfs_init_ino_entry_info(struct f2fs_sb_info *sbi)
{
	unsigned int append = 0;
	int i;

	for (i = 0; i < MAX_INO_ENTRY; i++) {
		struct inode_management *im = &sbi->im[i];

		INIT_RADIX_TREE(&im->ino_root, GFP_ATOMIC);
		spin_lock_init(&im->ino_lock);
		INIT_LIST_HEAD(&im->ino_list);
		im->ino_num = 0;
	}

#ifdef CONFIG_HMFS_JOURNAL_APPEND
	append = APPEND_BLOCKS;
#endif
	if (enabled_nat_bits(sbi, NULL))
		append += NM_I(sbi)->nat_bits_blocks;

	sbi->max_orphans = (sbi->blocks_per_seg - F2FS_CP_PACKS -
			NR_CURSEG_TYPE - __cp_payload(sbi) - append) *
				F2FS_ORPHANS_PER_BLOCK;
}

int __init hmfs_create_checkpoint_caches(void)
{
	ino_entry_slab = f2fs_kmem_cache_create("f2fs_ino_entry",
			sizeof(struct ino_entry));
	if (!ino_entry_slab)
		return -ENOMEM;
	hmfs_inode_entry_slab = f2fs_kmem_cache_create("f2fs_inode_entry",
			sizeof(struct inode_entry));
	if (!hmfs_inode_entry_slab) {
		kmem_cache_destroy(ino_entry_slab);
		return -ENOMEM;
	}
	return 0;
}

void hmfs_destroy_checkpoint_caches(void)
{
	kmem_cache_destroy(ino_entry_slab);
	kmem_cache_destroy(hmfs_inode_entry_slab);
}
