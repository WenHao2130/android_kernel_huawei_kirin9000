/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM block

#if !defined(_TRACE_BLOCK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_BLOCK_H

#include <linux/blktrace_api.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/tracepoint.h>

#define RWBS_LEN	8

DECLARE_EVENT_CLASS(block_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh),

	TP_STRUCT__entry (
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  size_t,	size			)
	),

	TP_fast_assign(
		__entry->dev		= bh->b_bdev->bd_dev;
		__entry->sector		= bh->b_blocknr;
		__entry->size		= bh->b_size;
	),

	TP_printk("%d,%d sector=%llu size=%zu",
		MAJOR(__entry->dev), MINOR(__entry->dev),
		(unsigned long long)__entry->sector, __entry->size
	)
);

/**
 * block_touch_buffer - mark a buffer accessed
 * @bh: buffer_head being touched
 *
 * Called from touch_buffer().
 */
DEFINE_EVENT(block_buffer, block_touch_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh)
);

/**
 * block_dirty_buffer - mark a buffer dirty
 * @bh: buffer_head being dirtied
 *
 * Called from mark_buffer_dirty().
 */
DEFINE_EVENT(block_buffer, block_dirty_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh)
);

/**
 * block_rq_requeue - place block IO request back on a queue
 * @q: queue holding operation
 * @rq: block IO operation request
 *
 * The block operation request @rq is being placed back into queue
 * @q.  For some reason the request was not completed and needs to be
 * put back in the queue.
 */
TRACE_EVENT(block_rq_requeue,

	TP_PROTO(struct request_queue *q, struct request *rq),

	TP_ARGS(q, rq),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__array(  char,		rwbs,	RWBS_LEN	)
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->rq_disk ? disk_devt(rq->rq_disk) : 0;
		__entry->sector    = blk_rq_trace_sector(rq);
		__entry->nr_sector = blk_rq_trace_nr_sectors(rq);

#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, rq->mas_cmd_flags, blk_rq_bytes(rq));
#else
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, blk_rq_bytes(rq));
#endif
		__get_str(cmd)[0] = '\0';
	),

	TP_printk("%d,%d %s (%s) %llu + %u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __get_str(cmd),
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, 0)
);

/**
 * block_rq_complete - block IO operation completed by device driver
 * @rq: block operations request
 * @error: status code
 * @nr_bytes: number of completed bytes
 *
 * The block_rq_complete tracepoint event indicates that some portion
 * of operation request has been completed by the device driver.  If
 * the @rq->bio is %NULL, then there is absolutely no additional work to
 * do for the request. If @rq->bio is non-NULL then there is
 * additional work required to complete the request.
 */
TRACE_EVENT(block_rq_complete,

	TP_PROTO(struct request *rq, int error, unsigned int nr_bytes),

	TP_ARGS(rq, error, nr_bytes),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__field(  int,		error			)
		__array(  char,		rwbs,	RWBS_LEN	)
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->rq_disk ? disk_devt(rq->rq_disk) : 0;
		__entry->sector    = blk_rq_pos(rq);
		__entry->nr_sector = nr_bytes >> 9;
		__entry->error     = error;

#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, rq->mas_cmd_flags, nr_bytes);
#else
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, nr_bytes);
#endif
		__get_str(cmd)[0] = '\0';
	),

	TP_printk("%d,%d %s (%s) %llu + %u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __get_str(cmd),
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->error)
);

DECLARE_EVENT_CLASS(block_rq,

	TP_PROTO(struct request_queue *q, struct request *rq),

	TP_ARGS(q, rq),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__field(  unsigned int,	bytes			)
		__array(  char,		rwbs,	RWBS_LEN	)
		__array(  char,         comm,   TASK_COMM_LEN   )
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->rq_disk ? disk_devt(rq->rq_disk) : 0;
		__entry->sector    = blk_rq_trace_sector(rq);
		__entry->nr_sector = blk_rq_trace_nr_sectors(rq);
		__entry->bytes     = blk_rq_bytes(rq);

#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, rq->mas_cmd_flags, blk_rq_bytes(rq));
#else
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, blk_rq_bytes(rq));
#endif
		__get_str(cmd)[0] = '\0';
#ifdef CONFIG_MAS_BLK
		memcpy(__entry->comm, rq->mas_req.task_comm, TASK_COMM_LEN);
#else
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
#endif
	),

	TP_printk("%d,%d %s %u (%s) %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __entry->bytes, __get_str(cmd),
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

/**
 * block_rq_insert - insert block operation request into queue
 * @q: target queue
 * @rq: block IO operation request
 *
 * Called immediately before block operation request @rq is inserted
 * into queue @q.  The fields in the operation request @rq struct can
 * be examined to determine which device and sectors the pending
 * operation would access.
 */
DEFINE_EVENT(block_rq, block_rq_insert,

	TP_PROTO(struct request_queue *q, struct request *rq),

	TP_ARGS(q, rq)
);

/**
 * block_rq_issue - issue pending block IO request operation to device driver
 * @q: queue holding operation
 * @rq: block IO operation operation request
 *
 * Called when block operation request @rq from queue @q is sent to a
 * device driver for processing.
 */
DEFINE_EVENT(block_rq, block_rq_issue,

	TP_PROTO(struct request_queue *q, struct request *rq),

	TP_ARGS(q, rq)
);

/**
 * block_rq_merge - merge request with another one in the elevator
 * @q: queue holding operation
 * @rq: block IO operation operation request
 *
 * Called when block operation request @rq from queue @q is merged to another
 * request queued in the elevator.
 */
DEFINE_EVENT(block_rq, block_rq_merge,

	TP_PROTO(struct request_queue *q, struct request *rq),

	TP_ARGS(q, rq)
);

/**
 * block_bio_bounce - used bounce buffer when processing block operation
 * @q: queue holding the block operation
 * @bio: block operation
 *
 * A bounce buffer was used to handle the block operation @bio in @q.
 * This occurs when hardware limitations prevent a direct transfer of
 * data between the @bio data memory area and the IO device.  Use of a
 * bounce buffer requires extra copying of data and decreases
 * performance.
 */
TRACE_EVENT(block_bio_bounce,

	TP_PROTO(struct request_queue *q, struct bio *bio),

	TP_ARGS(q, bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

/**
 * block_bio_complete - completed all work on the block operation
 * @q: queue holding the block operation
 * @bio: block operation completed
 *
 * This tracepoint indicates there is no further work to do on this
 * block IO operation @bio.
 */
TRACE_EVENT(block_bio_complete,

	TP_PROTO(struct request_queue *q, struct bio *bio),

	TP_ARGS(q, bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned,	nr_sector	)
		__field( int,		error		)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		__entry->error		= blk_status_to_errno(bio->bi_status);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
	),

	TP_printk("%d,%d %s %llu + %u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->error)
);

DECLARE_EVENT_CLASS(block_bio_merge,

	TP_PROTO(struct request_queue *q, struct request *rq, struct bio *bio),

	TP_ARGS(q, rq, bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

/**
 * block_bio_backmerge - merging block operation to the end of an existing operation
 * @q: queue holding operation
 * @rq: request bio is being merged into
 * @bio: new block operation to merge
 *
 * Merging block request @bio to the end of an existing block request
 * in queue @q.
 */
DEFINE_EVENT(block_bio_merge, block_bio_backmerge,

	TP_PROTO(struct request_queue *q, struct request *rq, struct bio *bio),

	TP_ARGS(q, rq, bio)
);

/**
 * block_bio_frontmerge - merging block operation to the beginning of an existing operation
 * @q: queue holding operation
 * @rq: request bio is being merged into
 * @bio: new block operation to merge
 *
 * Merging block IO operation @bio to the beginning of an existing block
 * operation in queue @q.
 */
DEFINE_EVENT(block_bio_merge, block_bio_frontmerge,

	TP_PROTO(struct request_queue *q, struct request *rq, struct bio *bio),

	TP_ARGS(q, rq, bio)
);

/**
 * block_bio_queue - putting new block IO operation in queue
 * @q: queue holding operation
 * @bio: new block operation
 *
 * About to place the block IO operation @bio into queue @q.
 */
TRACE_EVENT(block_bio_queue,

	TP_PROTO(struct request_queue *q, struct bio *bio),

	TP_ARGS(q, bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

DECLARE_EVENT_CLASS(block_get_rq,

	TP_PROTO(struct request_queue *q, struct bio *bio, int rw),

	TP_ARGS(q, bio, rw),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
        ),

	TP_fast_assign(
		__entry->dev		= bio ? bio_dev(bio) : 0;
		__entry->sector		= bio ? bio->bi_iter.bi_sector : 0;
		__entry->nr_sector	= bio ? bio_sectors(bio) : 0;
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs,
				  bio ? bio->bi_opf : 0, bio ? bio->mas_bi_opf : 0, __entry->nr_sector);
#else
		blk_fill_rwbs(__entry->rwbs,
			      bio ? bio->bi_opf : 0, __entry->nr_sector);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
        ),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);

/**
 * block_getrq - get a free request entry in queue for block IO operations
 * @q: queue for operations
 * @bio: pending block IO operation (can be %NULL)
 * @rw: low bit indicates a read (%0) or a write (%1)
 *
 * A request struct for queue @q has been allocated to handle the
 * block IO operation @bio.
 */
DEFINE_EVENT(block_get_rq, block_getrq,

	TP_PROTO(struct request_queue *q, struct bio *bio, int rw),

	TP_ARGS(q, bio, rw)
);

/**
 * block_sleeprq - waiting to get a free request entry in queue for block IO operation
 * @q: queue for operation
 * @bio: pending block IO operation (can be %NULL)
 * @rw: low bit indicates a read (%0) or a write (%1)
 *
 * In the case where a request struct cannot be provided for queue @q
 * the process needs to wait for an request struct to become
 * available.  This tracepoint event is generated each time the
 * process goes to sleep waiting for request struct become available.
 */
DEFINE_EVENT(block_get_rq, block_sleeprq,

	TP_PROTO(struct request_queue *q, struct bio *bio, int rw),

	TP_ARGS(q, bio, rw)
);

/**
 * block_plug - keep operations requests in request queue
 * @q: request queue to plug
 *
 * Plug the request queue @q.  Do not allow block operation requests
 * to be sent to the device driver. Instead, accumulate requests in
 * the queue to improve throughput performance of the block device.
 */
TRACE_EVENT(block_plug,

	TP_PROTO(struct request_queue *q),

	TP_ARGS(q),

	TP_STRUCT__entry(
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("[%s]", __entry->comm)
);

DECLARE_EVENT_CLASS(block_unplug,

	TP_PROTO(struct request_queue *q, unsigned int depth, bool explicit),

	TP_ARGS(q, depth, explicit),

	TP_STRUCT__entry(
		__field( int,		nr_rq			)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->nr_rq = depth;
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("[%s] %d", __entry->comm, __entry->nr_rq)
);

/**
 * block_unplug - release of operations requests in request queue
 * @q: request queue to unplug
 * @depth: number of requests just added to the queue
 * @explicit: whether this was an explicit unplug, or one from schedule()
 *
 * Unplug request queue @q because device driver is scheduled to work
 * on elements in the request queue.
 */
DEFINE_EVENT(block_unplug, block_unplug,

	TP_PROTO(struct request_queue *q, unsigned int depth, bool explicit),

	TP_ARGS(q, depth, explicit)
);

/**
 * block_split - split a single bio struct into two bio structs
 * @q: queue containing the bio
 * @bio: block operation being split
 * @new_sector: The starting sector for the new bio
 *
 * The bio request @bio in request queue @q needs to be split into two
 * bio requests. The newly created @bio request starts at
 * @new_sector. This split may be required due to hardware limitation
 * such as operation crossing device boundaries in a RAID system.
 */
TRACE_EVENT(block_split,

	TP_PROTO(struct request_queue *q, struct bio *bio,
		 unsigned int new_sector),

	TP_ARGS(q, bio, new_sector),

	TP_STRUCT__entry(
		__field( dev_t,		dev				)
		__field( sector_t,	sector				)
		__field( sector_t,	new_sector			)
		__array( char,		rwbs,		RWBS_LEN	)
		__array( char,		comm,		TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->new_sector	= new_sector;
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu / %llu [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  (unsigned long long)__entry->new_sector,
		  __entry->comm)
);

/**
 * block_bio_remap - map request for a logical device to the raw device
 * @q: queue holding the operation
 * @bio: revised operation
 * @dev: device for the operation
 * @from: original sector for the operation
 *
 * An operation for a logical device has been mapped to the
 * raw block device.
 */
TRACE_EVENT(block_bio_remap,

	TP_PROTO(struct request_queue *q, struct bio *bio, dev_t dev,
		 sector_t from),

	TP_ARGS(q, bio, dev, from),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned int,	nr_sector	)
		__field( dev_t,		old_dev		)
		__field( sector_t,	old_sector	)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		__entry->old_dev	= dev;
		__entry->old_sector	= from;
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
	),

	TP_printk("%d,%d %s %llu + %u <- (%d,%d) %llu",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector,
		  MAJOR(__entry->old_dev), MINOR(__entry->old_dev),
		  (unsigned long long)__entry->old_sector)
);

/**
 * block_rq_remap - map request for a block operation request
 * @q: queue holding the operation
 * @rq: block IO operation request
 * @dev: device for the operation
 * @from: original sector for the operation
 *
 * The block operation request @rq in @q has been remapped.  The block
 * operation request @rq holds the current information and @from hold
 * the original sector.
 */
TRACE_EVENT(block_rq_remap,

	TP_PROTO(struct request_queue *q, struct request *rq, dev_t dev,
		 sector_t from),

	TP_ARGS(q, rq, dev, from),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned int,	nr_sector	)
		__field( dev_t,		old_dev		)
		__field( sector_t,	old_sector	)
		__field( unsigned int,	nr_bios		)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= disk_devt(rq->rq_disk);
		__entry->sector		= blk_rq_pos(rq);
		__entry->nr_sector	= blk_rq_sectors(rq);
		__entry->old_dev	= dev;
		__entry->old_sector	= from;
		__entry->nr_bios	= blk_rq_count_bios(rq);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, rq->mas_cmd_flags, blk_rq_bytes(rq));
#else
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags, blk_rq_bytes(rq));
#endif
	),

	TP_printk("%d,%d %s %llu + %u <- (%d,%d) %llu %u",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector,
		  MAJOR(__entry->old_dev), MINOR(__entry->old_dev),
		  (unsigned long long)__entry->old_sector, __entry->nr_bios)
);

#ifdef CONFIG_MAS_IO_TRACE
#define HISI_IO_TRACE_LEN   128
TRACE_EVENT(hisi_io,

	TP_PROTO(const char *func, const char *str_info, const unsigned int len),

	TP_ARGS(func, str_info),

	TP_STRUCT__entry(
		__dynamic_array(char, info_func, strlen(func) + 1);
		__dynamic_array(char, info_string, strlen(str_info) + 1);),

	TP_fast_assign(
		memset(__get_str(info_func), 0x00, strlen(func) + 1);
		memcpy(__get_str(info_func), func, strlen(func));
		memset(__get_str(info_string), 0x00, strlen(str_info) + 1);
		memcpy(__get_str(info_string), str_info, strlen(str_info));),

	TP_printk("[%s]:%s", __get_str(info_func), __get_str(info_string)));

TRACE_EVENT(block_submit_bio,

	TP_PROTO(struct bio *bio, int enter),

	TP_ARGS(bio, enter),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
        __field( unsigned int,	is_enter		)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
#ifdef CONFIG_MAS_BLK
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->mas_bi_opf, bio->bi_iter.bi_size);
#else
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf, bio->bi_iter.bi_size);
#endif
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
        __entry->is_enter = enter
	),

	TP_printk("%d,%d %s %llu + %u %d [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->is_enter, __entry->comm)
);
#endif /* CONFIG_MAS_IO_TRACE */
#endif /* _TRACE_BLOCK_H */

/* This part must be outside protection */
#include <trace/define_trace.h>

