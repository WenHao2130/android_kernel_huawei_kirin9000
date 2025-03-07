// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/segment.h
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/blkdev.h>
#include <linux/backing-dev.h>
#include <trace/events/hmfs.h>

/* constant macro */
#define NULL_SEGNO			((unsigned int)(~0))
#define NULL_SECNO			((unsigned int)(~0))

#define DEF_RECLAIM_PREFREE_SEGMENTS	5	/* 5% over total segments */
#define DEF_MAX_RECLAIM_PREFREE_SEGMENTS	4096	/* 8GB in maximum */

#define F2FS_MIN_SEGMENTS	9 /* SB + 2 (CP + SIT + NAT) + SSA + MAIN */

/* L: Logical segment # in volume, R: Relative segment # in main area */
#define GET_L2R_SEGNO(free_i, segno)	((segno) - (free_i)->start_segno)
#define GET_R2L_SEGNO(free_i, segno)	((segno) + (free_i)->start_segno)

#define IS_DATASEG(t)	((t) <= CURSEG_COLD_DATA)
#define IS_NODESEG(t)	((t) >= CURSEG_HOT_NODE && (t) <= CURSEG_COLD_NODE)
#define IS_DMGCSEG(t)	((t) >= CURSEG_DATA_MOVE1 && (t) <= CURSEG_DATA_MOVE2)

#define IS_HOT(t)	((t) == CURSEG_HOT_NODE || (t) == CURSEG_HOT_DATA)
#define IS_WARM(t)	((t) == CURSEG_WARM_NODE || (t) == CURSEG_WARM_DATA)
#define IS_COLD(t)	((t) == CURSEG_COLD_NODE || (t) == CURSEG_COLD_DATA)

#define IS_CURSEG(sbi, seg)						\
	(((seg) == CURSEG_I(sbi, CURSEG_HOT_DATA)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_WARM_DATA)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_COLD_DATA)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_HOT_NODE)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_WARM_NODE)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_COLD_NODE)->segno) ||	\
	 ((seg) == CURSEG_I(sbi, CURSEG_DATA_MOVE1)->segno) ||  \
	 ((seg) == CURSEG_I(sbi, CURSEG_DATA_MOVE2)->segno) || \
	 ((seg) == CURSEG_I(sbi, CURSEG_FRAGMENT_DATA)->segno))

#define IS_CURSEC(sbi, secno)						\
	(((secno) == CURSEG_I(sbi, CURSEG_HOT_DATA)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_WARM_DATA)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_COLD_DATA)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_HOT_NODE)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_WARM_NODE)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_COLD_NODE)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_DATA_MOVE1)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_DATA_MOVE2)->segno /		\
	  (sbi)->segs_per_sec) ||	\
	 ((secno) == CURSEG_I(sbi, CURSEG_FRAGMENT_DATA)->segno /	\
	  (sbi)->segs_per_sec))	\

#define MAIN_BLKADDR(sbi)						\
	(SM_I(sbi) ? SM_I(sbi)->main_blkaddr :				\
		le32_to_cpu(F2FS_RAW_SUPER(sbi)->main_blkaddr))
#define SEG0_BLKADDR(sbi)						\
	(SM_I(sbi) ? SM_I(sbi)->seg0_blkaddr :				\
		le32_to_cpu(F2FS_RAW_SUPER(sbi)->segment0_blkaddr))

#define MAIN_SEGS(sbi)	(SM_I(sbi)->main_segments)
#define MAIN_SECS(sbi)	((sbi)->total_sections)

#define TOTAL_SEGS(sbi)							\
	(SM_I(sbi) ? SM_I(sbi)->segment_count :					\
		le32_to_cpu(F2FS_RAW_SUPER(sbi)->segment_count))
#define TOTAL_BLKS(sbi)	(TOTAL_SEGS(sbi) << (sbi)->log_blocks_per_seg)

#define MAX_BLKADDR(sbi)	(SEG0_BLKADDR(sbi) + TOTAL_BLKS(sbi))
#define SEGMENT_SIZE(sbi)	(1ULL << ((sbi)->log_blocksize +	\
					(sbi)->log_blocks_per_seg))

#define START_BLOCK(sbi, segno)	(SEG0_BLKADDR(sbi) +			\
	 (GET_R2L_SEGNO(FREE_I(sbi), segno) << (sbi)->log_blocks_per_seg))

#define NEXT_FREE_BLKADDR(sbi, curseg)					\
	(START_BLOCK(sbi, (curseg)->segno) + (curseg)->next_blkoff)

#define GET_SEGOFF_FROM_SEG0(sbi, blk_addr)	((blk_addr) - SEG0_BLKADDR(sbi))
#define GET_SEGNO_FROM_SEG0(sbi, blk_addr)				\
	(GET_SEGOFF_FROM_SEG0(sbi, blk_addr) >> (sbi)->log_blocks_per_seg)
#define GET_BLKOFF_FROM_SEG0(sbi, blk_addr)				\
	(GET_SEGOFF_FROM_SEG0(sbi, blk_addr) & ((sbi)->blocks_per_seg - 1))

#define GET_SEGNO(sbi, blk_addr)					\
	((!is_valid_data_blkaddr(sbi, blk_addr)) ?			\
	NULL_SEGNO : GET_L2R_SEGNO(FREE_I(sbi),			\
		GET_SEGNO_FROM_SEG0(sbi, blk_addr)))
#define BLKS_PER_SEC(sbi)					\
	((sbi)->segs_per_sec * (sbi)->blocks_per_seg)
#define GET_SEC_FROM_SEG(sbi, segno)				\
	((segno) / (sbi)->segs_per_sec)
#define GET_SEC_FROM_LBA(sbi, blkaddr)				\
	(GET_SEC_FROM_SEG((sbi), GET_SEGNO((sbi), (blkaddr))))
#define GET_SEG_FROM_SEC(sbi, secno)				\
	((secno) * (sbi)->segs_per_sec)
#define GET_ZONE_FROM_SEC(sbi, secno)				\
	((secno) / (sbi)->secs_per_zone)
#define GET_ZONE_FROM_SEG(sbi, segno)				\
	GET_ZONE_FROM_SEC(sbi, GET_SEC_FROM_SEG(sbi, segno))

#define GET_SUM_BLOCK(sbi, segno)				\
	((sbi)->sm_info->ssa_blkaddr + (segno))

bool hmfs_is_last_addr_in_section(struct f2fs_sb_info *sbi,
		block_t blkaddr, enum page_type type);
#define IS_LAST_DATA_BLOCK_IN_SEC(sbi, blkaddr, type)	\
	hmfs_is_last_addr_in_section(sbi, blkaddr, type)

#define IS_FIRST_DATA_BLOCK_IN_SEC(sbi, blkaddr)	\
	((blkaddr < MAIN_BLKADDR(sbi)) ? false :	\
	((blkaddr - MAIN_BLKADDR(sbi)) % BLKS_PER_SEC(sbi) == 0))

#define DATA_BLOCK_IN_SAME_SEC(sbi, blkaddr1, blkaddr2) \
	(GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, (blkaddr1))) == \
	 GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, (blkaddr2))))

#define GET_WR_SEGS_LIMIT(sbi)	((sbi)->segs_per_sec / 3 - 1)
/*
 * only record oob start and end position
 * 1) start: record in CP
 * 2) end: record by UFS
 * so write 2 section at most to trigger CP
 */
#define OOB_WR_LIMIT	(MAX_RECOVER_EXT_CNT)

#define GET_SUM_TYPE(footer) ((footer)->entry_type)
#define SET_SUM_TYPE(footer, type) ((footer)->entry_type = (type))

#define SIT_ENTRY_OFFSET(sit_i, segno)					\
	((segno) % (sit_i)->sents_per_block)
#define SIT_BLOCK_OFFSET(segno)					\
	((segno) / SIT_ENTRY_PER_BLOCK)
#define	START_SEGNO(segno)		\
	(SIT_BLOCK_OFFSET(segno) * SIT_ENTRY_PER_BLOCK)
#define SIT_BLK_CNT(sbi)			\
	((MAIN_SEGS(sbi) + SIT_ENTRY_PER_BLOCK - 1) / SIT_ENTRY_PER_BLOCK)
#define f2fs_bitmap_size(nr)			\
	(BITS_TO_LONGS(nr) * sizeof(unsigned long))

#define SECTOR_FROM_BLOCK(blk_addr)					\
	(((sector_t)blk_addr) << F2FS_LOG_SECTORS_PER_BLOCK)
#define SECTOR_TO_BLOCK(sectors)					\
	((sectors) >> F2FS_LOG_SECTORS_PER_BLOCK)
#ifdef CONFIG_HMFS_GRADING_SSR
#define KBS_PER_SEGMENT 2048
#endif

#define SSR_CONTIG_DIRTY_NUMS	32	/*Dirty pages for LFS alloction in grading ssr . */
#define SSR_CONTIG_LARGE	256	/*Larege files */

enum {
	SEQ_NONE,
	SEQ_32BLKS,
	SEQ_256BLKS
};

/*
 * indicate a block allocation direction: RIGHT and LEFT.
 * RIGHT means allocating new sections towards the end of volume.
 * LEFT means the opposite direction.
 */
enum {
	ALLOC_RIGHT = 0,
	ALLOC_LEFT,
	ALLOC_SPREAD,	/* for subdivision allocation only */
};

/*
 * In the victim_sel_policy->alloc_mode, there are two block allocation modes.
 * LFS writes data sequentially with cleaning operations.
 * SSR (Slack Space Recycle) reuses obsolete space without cleaning operations.
 * ASSR (Age based Slack Space Recycle) merges fragments into fragmented segment
 * which has similar aging degree.
 */
enum {
	LFS = 0,
	SSR,
	ASSR,
};

/*
 * In the victim_sel_policy->gc_mode, there are two gc, aka cleaning, modes.
 * GC_CB is based on cost-benefit algorithm.
 * GC_GREEDY is based on greedy algorithm.
 * GC_AT is based on age-threshold algorithm.
 */
enum {
	GC_CB = 0,
	GC_GREEDY,
	GC_AT,
	ALLOC_NEXT,
	FLUSH_DEVICE,
	MAX_GC_POLICY,
};

/*
 * BG_GC means the background cleaning job.
 * FG_GC means the on-demand cleaning job.
 * FORCE_FG_GC means on-demand cleaning job in background.
 */
enum {
	BG_GC = 0,
	FG_GC,
	FORCE_FG_GC,
};

#ifdef CONFIG_HMFS_GRADING_SSR
enum {
	GRADING_SSR_OFF = 0,
	GRADING_SSR_ON
};
#endif

/* for a function parameter to select a victim segment */
struct victim_sel_policy {
	int alloc_mode;			/* LFS or SSR */
	int gc_mode;			/* GC_CB or GC_GREEDY */
	unsigned long *dirty_bitmap;	/* dirty segment/section bitmap */
	unsigned int max_search;	/* maximum # of segments to search */
	unsigned int offset;		/* last scanned bitmap offset */
	unsigned int ofs_unit;		/* bitmap search unit */
	unsigned int min_cost;		/* minimum cost */
	unsigned long long oldest_age;	/* oldest age of segments having the same min cost */
	unsigned int min_segno;		/* segment # having min. cost */
	unsigned long long age;		/* mtime of GCed section*/
	unsigned long long age_threshold;/* age threshold */
};

struct seg_entry {
	unsigned int type:6;		/* segment type like CURSEG_XXX_TYPE */
	unsigned int valid_blocks:10;	/* # of valid blocks */
	unsigned int ckpt_valid_blocks:10;	/* # of valid blocks last cp */
	unsigned int padding:6;		/* padding */
	unsigned char *cur_valid_map;	/* validity bitmap of blocks */
#ifdef CONFIG_HMFS_CHECK_FS
	unsigned char *cur_valid_map_mir;	/* mirror of current valid bitmap */
#endif
	/*
	 * # of valid blocks and the validity bitmap stored in the the last
	 * checkpoint pack. This information is used by the SSR mode.
	 */
	unsigned char *ckpt_valid_map;	/* validity bitmap of blocks last cp */
	unsigned char *discard_map;
	unsigned long long mtime;	/* modification time of the segment */
};

struct sec_entry {
	unsigned int valid_blocks;	/* # of valid blocks in a section */
	int flash_mode;
};

struct segment_allocation {
	void (*allocate_segment)(struct f2fs_sb_info *, struct curseg_info *,
								int, bool, int);
	void (*get_new_segment)(struct f2fs_sb_info *,
					unsigned int *, bool , int);
	void (*new_curseg)(struct f2fs_sb_info *, struct curseg_info *,
								int, bool);
};

#define BGGC_NODE_PAGE			((unsigned long)-3)
#define IS_BGGC_NODE_PAGE(page)				\
		(page_private(page) == (unsigned long)BGGC_NODE_PAGE)

#define MAX_SKIP_GC_COUNT			16

struct inmem_pages {
	struct list_head list;
	struct page *page;
	block_t old_addr;		/* for revoking when fail to commit */
};

#ifdef CONFIG_HMFS_CHECK_FS
#define SIT_VBLOCK_MAP_NUM 4
#else
#define SIT_VBLOCK_MAP_NUM 3
#endif

struct sit_info {
	const struct segment_allocation *s_ops;

	block_t sit_base_addr;		/* start block address of SIT area */
	block_t sit_blocks;		/* # of blocks used by SIT area */
	block_t written_valid_blocks;	/* # of valid blocks in main area */
	char *bitmap;			/* all bitmaps pointer */
	char *sit_bitmap;		/* SIT bitmap pointer */
#ifdef CONFIG_HMFS_CHECK_FS
	char *sit_bitmap_mir;		/* SIT bitmap mirror */
#endif
	unsigned int bitmap_size;	/* SIT bitmap size */

	unsigned long *tmp_map;			/* bitmap for temporal use */
	unsigned long *dirty_sentries_bitmap;	/* bitmap for dirty sentries */
	unsigned int dirty_sentries;		/* # of dirty sentries */
	unsigned int sents_per_block;		/* # of SIT entries per block */
	struct rw_semaphore sentry_lock;	/* to protect SIT cache */
	struct seg_entry *sentries;		/* SIT segment-level cache */
	struct sec_entry *sec_entries;		/* SIT section-level cache */

	/* for cost-benefit algorithm in cleaning procedure */
	unsigned long long elapsed_time;	/* elapsed time after mount */
	time64_t mounted_time;			/* mount time */
	unsigned long long min_mtime;		/* min. modification time */
	unsigned long long dirty_min_mtime;	/* rerange candidates in GC_AT */
	unsigned long long max_mtime;		/* max. modification time */
	unsigned long long dirty_max_mtime;	/* rerange candidates in GC_AT */

	unsigned int last_victim[MAX_GC_POLICY]; /* last victim segment # */
};

struct free_segmap_info {
	unsigned int start_segno;	/* start segment number logically */
	unsigned int free_segments;	/* # of free segments */
	unsigned int free_sections;	/* # of free sections */
	spinlock_t segmap_lock;		/* free segmap lock */
	unsigned long *free_segmap;	/* free segment bitmap */
	unsigned long *free_secmap;	/* free section bitmap */
};

/* Notice: The order of dirty type is same with CURSEG_XXX in hmfs.h */
enum dirty_type {
	DIRTY_HOT_DATA,		/* dirty segments assigned as hot data logs */
	DIRTY_WARM_DATA,	/* dirty segments assigned as warm data logs */
	DIRTY_COLD_DATA,	/* dirty segments assigned as cold data logs */
	DIRTY_HOT_NODE,		/* dirty segments assigned as hot node logs */
	DIRTY_WARM_NODE,	/* dirty segments assigned as warm node logs */
	DIRTY_COLD_NODE,	/* dirty segments assigned as cold node logs */
	DIRTY_DATA_MOVE1,   /* dirty segments assigned as data move1 logs */
	DIRTY_DATA_MOVE2,   /* dirty segments assigned as data move2 logs */
	DIRTY,			/* to count # of dirty segments */
	PRE,			/* to count # of entirely obsolete segments */
	PRE_SEC,		/* to count # of entirely obsolete sections */
	NR_DIRTY_TYPE
};

struct dirty_seglist_info {
	const struct victim_selection *v_ops;	/* victim selction operation */
	unsigned long *dirty_segmap[NR_DIRTY_TYPE];
	unsigned long *dirty_secmap;
	struct mutex seglist_lock;		/* lock for segment bitmaps */
	int nr_dirty[NR_DIRTY_TYPE];		/* # of dirty segments */
	unsigned long *victim_secmap;		/* background GC victims */
};

/* victim selection function for cleaning and SSR */
struct victim_selection {
	int (*get_victim)(struct f2fs_sb_info *, unsigned int *,
					int, int, char, unsigned long long);
};

/* for active log information */
struct curseg_info {
	struct mutex curseg_mutex;		/* lock for consistency */
	struct f2fs_summary_block *sum_blk;	/* cached summary block */
	struct rw_semaphore journal_rwsem;	/* protect journal area */
	struct f2fs_journal *journal;		/* cached journal info */
	unsigned char alloc_type;		/* current allocation type */
	unsigned int segno;			/* current segment number */
	unsigned short next_blkoff;		/* next block offset to write */
	unsigned int zone;			/* current zone number */
	unsigned int next_segno;		/* preallocated segment */
	bool inited;				/* indicate inmem log is inited */
	char type;
};

struct sit_entry_set {
	struct list_head set_list;	/* link with all sit sets */
	unsigned int start_segno;	/* start segno of sits in set */
	unsigned int entry_cnt;		/* the # of sit entries in set */
};

/*
 * inline functions
 */

static inline struct curseg_info *CURSEG_I(struct f2fs_sb_info *sbi, int type)
{
	return (struct curseg_info *)(SM_I(sbi)->curseg_array + type);
}

static inline int CURSEG_T(int stream_id)
{
	/* update when stream id change */
	static const int stream_id_to_cur_type[5] = {
		NO_CHECK_TYPE,
		CURSEG_COLD_NODE, CURSEG_COLD_DATA,
		CURSEG_HOT_NODE, CURSEG_HOT_DATA
	};

	return stream_id_to_cur_type[stream_id];
}

static inline int get_stream_id_by_type_temp(enum page_type type,
		enum temp_type temp)
{
	int stream_id = STREAM_NR;

	if (type != DATA && type != NODE) {
		stream_id = STREAM_META;
	} else {
		switch (type) {
		case DATA:
			if (temp == HOT)
				stream_id = STREAM_HOT_DATA;
			else if (temp == COLD)
				stream_id = STREAM_COLD_DATA;
			break;

		case NODE:
			if (temp == HOT)
				stream_id = STREAM_HOT_NODE;
			else if (temp == COLD)
				stream_id = STREAM_COLD_NODE;
			break;
		default:
			break;
		}
	}
	return stream_id;
}

static inline struct seg_entry *get_seg_entry(struct f2fs_sb_info *sbi,
						unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->sentries[segno];
}

static inline struct sec_entry *get_sec_entry(struct f2fs_sb_info *sbi,
						unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->sec_entries[GET_SEC_FROM_SEG(sbi, segno)];
}

static inline void hmfs_dm_check_blkaddr(struct f2fs_sb_info *sbi, block_t blkaddr)
{
	if ((blkaddr < MAIN_BLKADDR(sbi)) || (blkaddr > MAX_BLKADDR(sbi))) {
		hmfs_msg(sbi->sb, KERN_ERR, "dm illegal blkaddr %u", blkaddr);
		f2fs_bug_on(sbi, 1);
	}
}

static inline int hmfs_get_flash_mode(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	if (!IS_MULTI_SEGS_IN_SEC(sbi)) {
		return TLC_MODE;
	}

	return get_sec_entry(sbi, segno)->flash_mode;
}

static inline bool hmfs_is_file_switch_stream(struct inode *inode)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	return fi->is_switch;
}

static inline bool hmfs_is_exceed_oob_cnt(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < NR_CURSEG_TYPE; ++i) {
		if (i != CURSEG_HOT_DATA && i != CURSEG_COLD_DATA)
			continue;

		if (atomic_read(&sbi->oob_wr_cnt[i]) >= OOB_WR_LIMIT)
			return true;
	}

	return false;
}

static inline bool hmfs_is_file_atomic_switch(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	return (fi->cp_ver[FSYNC_CP_VER] == cur_cp_version(F2FS_CKPT(sbi)) &&
			fi->last_atomic && !fi->fsync_atomic);
}

static inline bool hmfs_is_file_truncate_write(struct inode *inode, enum cp_ver_record item)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);

	return (fi->cp_ver[item] == cur_cp_version(F2FS_CKPT(sbi)));
}


static inline bool hmfs_is_file_fsync_after_wb(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	unsigned long long cur_ver = cur_cp_version(F2FS_CKPT(sbi));
	/*
	 * Data has been flushed by wb, so oob w/o fsync mark.
	 * In this case, it should flush inode or CP.
	 * It's difficult to hit this kind of scene, so choose CP.
	 */
	if (!fi->fsync_dirty_pages && fi->has_wb &&
			fi->cp_ver[FSYNC_CP_VER] != cur_ver)
		return true;

	return false;
}

static inline void hmfs_inode_set_fofs(struct f2fs_sb_info *sbi,
		struct f2fs_inode_info *fi, loff_t fofs)
{
	if (fi->cp_ver[WRITE_CP_VER] != cur_cp_version(F2FS_CKPT(sbi))) {
		fi->cp_ver[WRITE_CP_VER] = cur_cp_version(F2FS_CKPT(sbi));
		fi->fofs = fofs;
	} else {
		fi->fofs = max(fofs, fi->fofs);
	}
}

static inline unsigned int get_valid_blocks(struct f2fs_sb_info *sbi,
				unsigned int segno, bool use_section)
{
	/*
	 * In order to get # of valid blocks in a section instantly from many
	 * segments, f2fs manages two counting structures separately.
	 */
	if (use_section && IS_MULTI_SEGS_IN_SEC(sbi))
		return get_sec_entry(sbi, segno)->valid_blocks;
	else
		return get_seg_entry(sbi, segno)->valid_blocks;
}

static inline unsigned int get_ckpt_valid_blocks(struct f2fs_sb_info *sbi,
				unsigned int segno, bool use_section)
{
	if (use_section && IS_MULTI_SEGS_IN_SEC(sbi)) {
		unsigned int start_segno = START_SEGNO(segno);
		unsigned int blocks = 0;
		int i;

		for (i = 0; i < sbi->segs_per_sec; i++, start_segno++) {
			struct seg_entry *se = get_seg_entry(sbi, start_segno);

			blocks += se->ckpt_valid_blocks;
		}
		return blocks;
	}
	return get_seg_entry(sbi, segno)->ckpt_valid_blocks;
}

static inline void seg_info_from_raw_sit(struct seg_entry *se,
					struct f2fs_sit_entry *rs)
{
	se->valid_blocks = GET_SIT_VBLOCKS(rs);
	se->ckpt_valid_blocks = GET_SIT_VBLOCKS(rs);
	memcpy(se->cur_valid_map, rs->valid_map, SIT_VBLOCK_MAP_SIZE);
	memcpy(se->ckpt_valid_map, rs->valid_map, SIT_VBLOCK_MAP_SIZE);
#ifdef CONFIG_HMFS_CHECK_FS
	memcpy(se->cur_valid_map_mir, rs->valid_map, SIT_VBLOCK_MAP_SIZE);
#endif
	se->type = GET_SIT_TYPE(rs);
	se->mtime = le64_to_cpu(rs->mtime);
}

static inline void __seg_info_to_raw_sit(struct seg_entry *se,
					struct f2fs_sit_entry *rs)
{
	unsigned short raw_vblocks = (se->type << SIT_VBLOCKS_SHIFT) |
					se->valid_blocks;
	rs->vblocks = cpu_to_le16(raw_vblocks);
	memcpy(rs->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
	rs->mtime = cpu_to_le64(se->mtime);
}

static inline void seg_info_to_sit_page(struct f2fs_sb_info *sbi,
				struct page *page, unsigned int start)
{
	struct f2fs_sit_block *raw_sit;
	struct seg_entry *se;
	struct f2fs_sit_entry *rs;
	unsigned int end = min(start + SIT_ENTRY_PER_BLOCK,
					(unsigned long)MAIN_SEGS(sbi));
	int i;

	raw_sit = (struct f2fs_sit_block *)page_address(page);
	memset(raw_sit, 0, PAGE_SIZE);
	for (i = 0; i < end - start; i++) {
		rs = &raw_sit->entries[i];
		se = get_seg_entry(sbi, start + i);
		__seg_info_to_raw_sit(se, rs);
	}
}

static inline void seg_info_to_raw_sit(struct seg_entry *se,
					struct f2fs_sit_entry *rs)
{
	__seg_info_to_raw_sit(se, rs);

	memcpy(se->ckpt_valid_map, rs->valid_map, SIT_VBLOCK_MAP_SIZE);
	se->ckpt_valid_blocks = se->valid_blocks;
}

static inline unsigned int find_next_inuse(struct free_segmap_info *free_i,
		unsigned int max, unsigned int segno)
{
	unsigned int ret;
	spin_lock(&free_i->segmap_lock);
	ret = find_next_bit(free_i->free_segmap, max, segno);
	spin_unlock(&free_i->segmap_lock);
	return ret;
}

static inline void __set_free(struct f2fs_sb_info *sbi, unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);
	unsigned int start_segno = GET_SEG_FROM_SEC(sbi, secno);
	unsigned int next;

	if (hmfs_datamove_check_discard(sbi, segno,
						segno + 1, NULL))
		return;

	spin_lock(&free_i->segmap_lock);
	clear_bit(segno, free_i->free_segmap);
	free_i->free_segments++;

	next = find_next_bit(free_i->free_segmap,
			start_segno + sbi->segs_per_sec, start_segno);
	if (next >= start_segno + sbi->segs_per_sec) {
		clear_bit(secno, free_i->free_secmap);
		free_i->free_sections++;
	}
	spin_unlock(&free_i->segmap_lock);
}

static inline void __set_inuse(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);

	set_bit(segno, free_i->free_segmap);
	free_i->free_segments--;
	if (!test_and_set_bit(secno, free_i->free_secmap))
		free_i->free_sections--;
}

static inline void __set_test_and_free(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);
	unsigned int start_segno = GET_SEG_FROM_SEC(sbi, secno);
	unsigned int next;

	if (hmfs_datamove_check_discard(sbi, segno,
						segno + 1, NULL))
		return;

	spin_lock(&free_i->segmap_lock);
	if (test_and_clear_bit(segno, free_i->free_segmap)) {
		free_i->free_segments++;

		if (IS_CURSEC(sbi, secno))
			goto skip_free;
		next = find_next_bit(free_i->free_segmap,
				start_segno + sbi->segs_per_sec, start_segno);
		if (next >= start_segno + sbi->segs_per_sec) {
			if (test_and_clear_bit(secno, free_i->free_secmap))
				free_i->free_sections++;
		}
	}
skip_free:
	spin_unlock(&free_i->segmap_lock);
}

static inline void __set_test_and_inuse(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = GET_SEC_FROM_SEG(sbi, segno);

	spin_lock(&free_i->segmap_lock);
	if (!test_and_set_bit(segno, free_i->free_segmap)) {
		free_i->free_segments--;
		if (!test_and_set_bit(secno, free_i->free_secmap))
			free_i->free_sections--;
	}
	spin_unlock(&free_i->segmap_lock);
}

static inline void get_sit_bitmap(struct f2fs_sb_info *sbi,
		void *dst_addr)
{
	struct sit_info *sit_i = SIT_I(sbi);

#ifdef CONFIG_HMFS_CHECK_FS
	if (memcmp(sit_i->sit_bitmap, sit_i->sit_bitmap_mir,
						sit_i->bitmap_size))
		f2fs_bug_on(sbi, 1);
#endif
	memcpy(dst_addr, sit_i->sit_bitmap, sit_i->bitmap_size);
}

static inline block_t written_block_count(struct f2fs_sb_info *sbi)
{
	return SIT_I(sbi)->written_valid_blocks;
}

static inline unsigned int free_segments(struct f2fs_sb_info *sbi)
{
	return FREE_I(sbi)->free_segments;
}

static inline int reserved_segments(struct f2fs_sb_info *sbi)
{
	return SM_I(sbi)->reserved_segments;
}

static inline unsigned int free_sections(struct f2fs_sb_info *sbi)
{
	return FREE_I(sbi)->free_sections;
}

static inline unsigned int prefree_segments(struct f2fs_sb_info *sbi)
{
	return DIRTY_I(sbi)->nr_dirty[PRE];
}

static inline unsigned int prefree_sections(struct f2fs_sb_info *sbi)
{
	return DIRTY_I(sbi)->nr_dirty[PRE_SEC];
}

static inline unsigned int dirty_segments(struct f2fs_sb_info *sbi)
{
	return DIRTY_I(sbi)->nr_dirty[DIRTY_HOT_DATA] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_WARM_DATA] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_COLD_DATA] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_HOT_NODE] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_WARM_NODE] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_COLD_NODE] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_DATA_MOVE1] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_DATA_MOVE2];
}

static inline unsigned int dirty_data_segments(struct f2fs_sb_info *sbi)
{
	return DIRTY_I(sbi)->nr_dirty[DIRTY_HOT_DATA] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_WARM_DATA] +
		DIRTY_I(sbi)->nr_dirty[DIRTY_COLD_DATA];
}

static inline int overprovision_segments(struct f2fs_sb_info *sbi)
{
	return SM_I(sbi)->ovp_segments;
}

static inline int extra_op_segments(struct f2fs_sb_info *sbi)
{
	return SM_I(sbi)->extra_op_segments;
}

static inline int reserved_sections(struct f2fs_sb_info *sbi)
{
	return GET_SEC_FROM_SEG(sbi, (unsigned int)reserved_segments(sbi));
}

static inline int hmfs_overprovision_sections(struct f2fs_sb_info *sbi)
{
	return GET_SEC_FROM_SEG(sbi, (unsigned int)overprovision_segments(sbi));
}

static inline bool has_curseg_enough_space(struct f2fs_sb_info *sbi)
{
	unsigned int node_blocks = get_pages(sbi, F2FS_DIRTY_NODES) +
					get_pages(sbi, F2FS_DIRTY_DENTS);
	unsigned int dent_blocks = get_pages(sbi, F2FS_DIRTY_DENTS);
	unsigned int segno, left_blocks;
	int i;

	/* check current node segment */
	for (i = CURSEG_HOT_NODE; i <= CURSEG_COLD_NODE; i++) {
		if (F2FS_OPTION(sbi).active_logs == 4 && i == CURSEG_WARM_NODE)
			continue;

		segno = CURSEG_I(sbi, i)->segno;
		left_blocks = sbi->blocks_per_seg -
			get_seg_entry(sbi, segno)->ckpt_valid_blocks;

		if (node_blocks > left_blocks)
			return false;
	}

	/* check current data segment */
	segno = CURSEG_I(sbi, CURSEG_HOT_DATA)->segno;
	left_blocks = sbi->blocks_per_seg -
			get_seg_entry(sbi, segno)->ckpt_valid_blocks;
	if (dent_blocks > left_blocks)
		return false;
	return true;
}

static inline bool has_not_enough_free_secs(struct f2fs_sb_info *sbi,
					int freed, int needed)
{
	int unavailabe_secs = 0;

	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		return false;

	if (free_sections(sbi) - unavailabe_secs + freed
				== reserved_sections(sbi) + needed &&
			has_curseg_enough_space(sbi))
		return false;
	return (free_sections(sbi) - unavailabe_secs + freed) <=
				(reserved_sections(sbi) + needed);
}

static inline int f2fs_is_checkpoint_ready(struct f2fs_sb_info *sbi)
{
	if (likely(!is_sbi_flag_set(sbi, SBI_CP_DISABLED)))
		return 0;
	if (likely(!has_not_enough_free_secs(sbi, 0, 0)))
		return 0;
	return -ENOSPC;
}

static inline bool excess_prefree_segs(struct f2fs_sb_info *sbi)
{
	return prefree_segments(sbi) > SM_I(sbi)->rec_prefree_segments;
}

static inline int utilization(struct f2fs_sb_info *sbi)
{
	return div_u64((u64)valid_user_blocks(sbi) * 100,
					sbi->user_block_count);
}

/*
 * Sometimes f2fs may be better to drop out-of-place update policy.
 * And, users can control the policy through sysfs entries.
 * There are five policies with triggering conditions as follows.
 * F2FS_IPU_FORCE - all the time,
 * F2FS_IPU_SSR - if SSR mode is activated,
 * F2FS_IPU_UTIL - if FS utilization is over threashold,
 * F2FS_IPU_SSR_UTIL - if SSR mode is activated and FS utilization is over
 *                     threashold,
 * F2FS_IPU_FSYNC - activated in fsync path only for high performance flash
 *                     storages. IPU will be triggered only if the # of dirty
 *                     pages over min_fsync_blocks.
 * F2FS_IPUT_DISABLE - disable IPU. (=default option)
 */
#define DEF_MIN_IPU_UTIL	70
#define DEF_MIN_FSYNC_BLOCKS	20
#define DEF_MIN_HOT_BLOCKS	16

#define SMALL_VOLUME_SEGMENTS	(16 * 512)	/* 16GB */

enum {
	F2FS_IPU_FORCE,
	F2FS_IPU_SSR,
	F2FS_IPU_UTIL,
	F2FS_IPU_SSR_UTIL,
	F2FS_IPU_FSYNC,
	F2FS_IPU_ASYNC,
};

static inline bool need_inplace_update_policy(struct inode *inode,
				struct f2fs_io_info *fio)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	unsigned int policy = SM_I(sbi)->ipu_policy;

	if (test_opt(sbi, LFS))
		return false;

	/* if this is cold file, we should overwrite to avoid fragmentation */
	if (file_is_cold(inode)) {
		trace_hmfs_cold_file_should_IPU(inode->i_ino);
		return true;
	}

	if (policy & (0x1 << F2FS_IPU_FORCE))
		return true;
	if (policy & (0x1 << F2FS_IPU_SSR) && hmfs_need_SSR(sbi))
		return true;
	if (policy & (0x1 << F2FS_IPU_UTIL) &&
			utilization(sbi) > SM_I(sbi)->min_ipu_util)
		return true;
	if (policy & (0x1 << F2FS_IPU_SSR_UTIL) && hmfs_need_SSR(sbi) &&
			utilization(sbi) > SM_I(sbi)->min_ipu_util)
		return true;

	/*
	 * IPU for rewrite async pages
	 */
	if (policy & (0x1 << F2FS_IPU_ASYNC) &&
			fio && fio->op == REQ_OP_WRITE &&
			!(fio->op_flags & REQ_SYNC) &&
			!f2fs_encrypted_inode(inode))
		return true;

	/* this is only set during fdatasync */
	if (policy & (0x1 << F2FS_IPU_FSYNC) &&
			is_inode_flag_set(inode, FI_NEED_IPU))
		return true;

	return false;
}

static inline unsigned int curseg_segno(struct f2fs_sb_info *sbi,
		int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	return curseg->segno;
}

static inline unsigned char curseg_alloc_type(struct f2fs_sb_info *sbi,
		int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	return curseg->alloc_type;
}

static inline unsigned short curseg_blkoff(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	return curseg->next_blkoff;
}

static inline void check_seg_range(struct f2fs_sb_info *sbi, unsigned int segno)
{
	f2fs_bug_on(sbi, segno > TOTAL_SEGS(sbi) - 1);
}

static inline void verify_block_addr(struct f2fs_io_info *fio, block_t blk_addr)
{
	struct f2fs_sb_info *sbi = fio->sbi;

	if (__is_meta_io(fio))
		verify_blkaddr(sbi, blk_addr, META_GENERIC);
	else
		verify_blkaddr(sbi, blk_addr, DATA_GENERIC);
}

/*
 * Summary block is always treated as an invalid block
 */
static inline int check_block_count(struct f2fs_sb_info *sbi,
		int segno, struct f2fs_sit_entry *raw_sit)
{
	bool is_valid  = test_bit_le(0, raw_sit->valid_map) ? true : false;
	int valid_blocks = 0;
	int cur_pos = 0, next_pos;

	/* check bitmap with valid block count */
	do {
		if (is_valid) {
			next_pos = find_next_zero_bit_le(&raw_sit->valid_map,
					sbi->blocks_per_seg,
					cur_pos);
			valid_blocks += next_pos - cur_pos;
		} else
			next_pos = find_next_bit_le(&raw_sit->valid_map,
					sbi->blocks_per_seg,
					cur_pos);
		cur_pos = next_pos;
		is_valid = !is_valid;
	} while (cur_pos < sbi->blocks_per_seg);

	if (unlikely(GET_SIT_VBLOCKS(raw_sit) != valid_blocks)) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"Mismatch valid blocks %d vs. %d",
					GET_SIT_VBLOCKS(raw_sit), valid_blocks);
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		hmfs_set_need_fsck_report();
		return -EINVAL;
	}

	/* check segment usage, and check boundary of a given segment number */
	if (unlikely(GET_SIT_VBLOCKS(raw_sit) > sbi->blocks_per_seg
					|| segno > TOTAL_SEGS(sbi) - 1)) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"Wrong valid blocks %d or segno %u",
					GET_SIT_VBLOCKS(raw_sit), segno);
		set_sbi_flag(sbi, SBI_NEED_FSCK);
		hmfs_set_need_fsck_report();
		return -EINVAL;
	}
	return 0;
}

static inline pgoff_t current_sit_addr(struct f2fs_sb_info *sbi,
						unsigned int start)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(start);
	block_t blk_addr = sit_i->sit_base_addr + offset;

	check_seg_range(sbi, start);

#ifdef CONFIG_HMFS_CHECK_FS
	if (f2fs_test_bit(offset, sit_i->sit_bitmap) !=
			f2fs_test_bit(offset, sit_i->sit_bitmap_mir))
		f2fs_bug_on(sbi, 1);
#endif

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	return blk_addr;
}

static inline pgoff_t next_sit_addr(struct f2fs_sb_info *sbi,
						pgoff_t block_addr)
{
	struct sit_info *sit_i = SIT_I(sbi);
	block_addr -= sit_i->sit_base_addr;
	if (block_addr < sit_i->sit_blocks)
		block_addr += sit_i->sit_blocks;
	else
		block_addr -= sit_i->sit_blocks;

	return block_addr + sit_i->sit_base_addr;
}

static inline void set_to_next_sit(struct sit_info *sit_i, unsigned int start)
{
	unsigned int block_off = SIT_BLOCK_OFFSET(start);

	f2fs_change_bit(block_off, sit_i->sit_bitmap);
#ifdef CONFIG_HMFS_CHECK_FS
	f2fs_change_bit(block_off, sit_i->sit_bitmap_mir);
#endif
}

static inline unsigned long long get_mtime(struct f2fs_sb_info *sbi,
						bool base_time)
{
	struct sit_info *sit_i = SIT_I(sbi);
	time64_t now = ktime_get_real_seconds();
	unsigned long long diff;

	if (now >= sit_i->mounted_time)
		return sit_i->elapsed_time + now - sit_i->mounted_time;

	/* system time is set to the past */
	if (!base_time) {
		diff = sit_i->mounted_time - now;
		if (sit_i->elapsed_time >= diff)
			return sit_i->elapsed_time - diff;
		return 0;
	}
	return sit_i->elapsed_time;
}

static inline void set_summary(struct f2fs_summary *sum, nid_t nid,
			unsigned int ofs_in_node, unsigned char version)
{
	sum->nid = cpu_to_le32(nid);
	sum->ofs_in_node = cpu_to_le16(ofs_in_node);
	sum->version = version;
}

static inline block_t start_sum_block(struct f2fs_sb_info *sbi)
{
	return __start_cp_addr(sbi) +
		le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_start_sum);
}

static inline block_t sum_blk_addr(struct f2fs_sb_info *sbi, int base, int type)
{
	return __start_cp_addr(sbi) +
		le32_to_cpu(F2FS_CKPT(sbi)->cp_pack_total_block_count)
				- (base + 1) + type;
}

static inline bool sec_usage_check(struct f2fs_sb_info *sbi, unsigned int secno)
{
	if (IS_CURSEC(sbi, secno) || (sbi->cur_victim_sec == secno))
		return true;
	return false;
}

/*
 * It is very important to gather dirty pages and write at once, so that we can
 * submit a big bio without interfering other data writes.
 * By default, 512 pages for directory data,
 * 512 pages (2MB) * 8 for nodes, and
 * 256 pages * 8 for meta are set.
 */
static inline int nr_pages_to_skip(struct f2fs_sb_info *sbi, int type)
{
	if (sbi->sb->s_bdi->wb.dirty_exceeded)
		return 0;

	if (type == DATA)
		return sbi->blocks_per_seg;
	else if (type == NODE)
		return 8 * sbi->blocks_per_seg;
	else if (type == META)
		return 8 * BIO_MAX_PAGES;
	else
		return 0;
}

/*
 * When writing pages, it'd better align nr_to_write for segment size.
 */
static inline long nr_pages_to_write(struct f2fs_sb_info *sbi, int type,
					struct writeback_control *wbc)
{
	long nr_to_write, desired;

	if (wbc->sync_mode != WB_SYNC_NONE)
		return 0;

	nr_to_write = wbc->nr_to_write;
	desired = BIO_MAX_PAGES;
	if (type == NODE)
		desired <<= 1;

	wbc->nr_to_write = desired;
	return desired - nr_to_write;
}

static inline void wake_up_discard_thread(struct f2fs_sb_info *sbi, bool force)
{
	struct discard_cmd_control *dcc = SM_I(sbi)->dcc_info;
	bool wakeup = false;
	int i;

	if (force)
		goto wake_up;

	mutex_lock(&dcc->cmd_lock);
	for (i = MAX_PLIST_NUM - 1; i >= 0; i--) {
		if (i + 1 < dcc->discard_granularity)
			break;
		if (!list_empty(&dcc->pend_list[i])) {
			wakeup = true;
			break;
		}
	}
	mutex_unlock(&dcc->cmd_lock);
	if (!wakeup)
		return;
wake_up:
	dcc->discard_wake = 1;
	wake_up_interruptible_all(&dcc->discard_wait_queue);
}

static inline int check_io_seq(int blks)
{
	if (blks >= SSR_CONTIG_LARGE)
		return SEQ_256BLKS;
	else if (blks >= SSR_CONTIG_DIRTY_NUMS)
		return SEQ_32BLKS;
	else
		return SEQ_NONE;
}
