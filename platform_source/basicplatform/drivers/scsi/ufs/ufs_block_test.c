/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt"\n"

#include <linux/async.h>
#include <linux/atomic.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/test-iosched.h>
#include <scsi/scsi.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <linux/devfreq.h>
#include "ufshci.h"
#include "ufshcd.h"
#include "ufs.h"

#define MODULE_NAME "ufs_block_test"
#define UFS_TEST_BLK_DEV_TYPE_PREFIX "sd"

#define TEST_MAX_BIOS_PER_REQ		128
#define TEST_DEFAULT_SECTOR_RANGE		(1024*1024)	/* 512MB */
#define LARGE_PRIME_1	1103515367
#define LARGE_PRIME_2	35757
#define MAGIC_SEED	7
#define DEFAULT_NUM_OF_BIOS		2
#define LONG_SEQUENTIAL_MIXED_TIMOUT_MS 100000
#define THREADS_COMPLETION_TIMOUT msecs_to_jiffies(10000)	/* 10 sec */
#define MAX_PARALLEL_QUERIES 33
#define RANDOM_REQUEST_THREADS 4
#define LUN_DEPTH_TEST_SIZE 9
#define SECTOR_SIZE	512
#define NUM_UNLUCKY_RETRIES	10

extern struct gendisk *scsi_gendisk_get_from_dev(struct device *dev);
/*
 * this defines the density of random requests in the address space, and
 * it represents the ratio between accessed sectors and non-accessed sectors
 */
#define LONG_RAND_TEST_REQ_RATIO	64
/* request queue limitation is 128 requests, and we leave 10 spare requests */
#define QUEUE_MAX_REQUESTS 118
#define MB_MSEC_RATIO_APPROXIMATION ((1024 * 1024) / 1000)
/* actual number of MiB in test multiplied by 10, for single digit precision */
#define BYTE_TO_MB_x_10(x) ((x * 10) / (1024 * 1024))
/* extract integer value */
#define LONG_TEST_SIZE_INTEGER(x) (BYTE_TO_MB_x_10(x) / 10)
/* and calculate the MiB value fraction */
#define LONG_TEST_SIZE_FRACTION(x) (BYTE_TO_MB_x_10(x) - \
		(LONG_TEST_SIZE_INTEGER(x) * 10))
/* translation mask from sectors to block */
#define SECTOR_TO_BLOCK_MASK 0x7

#define TEST_OPS(test_name, upper_case_name)				\
static int ufs_test_ ## test_name ## _show(struct seq_file *file,	\
		void *data)						\
{ return ufs_test_show(file, UFS_TEST_ ## upper_case_name); }		\
static int ufs_test_ ## test_name ## _open(struct inode *inode,		\
		struct file *file)					\
{ return single_open(file, ufs_test_ ## test_name ## _show,		\
		inode->i_private); }					\
static ssize_t ufs_test_ ## test_name ## _write(struct file *file,	\
		const char __user *buf, size_t count, loff_t *ppos)	\
{ return ufs_test_write(file, buf, count, ppos,				\
			UFS_TEST_ ## upper_case_name); }		\
static const struct file_operations ufs_test_ ## test_name ## _ops = {	\
	.open = ufs_test_ ## test_name ## _open,			\
	.read = seq_read,						\
	.write = ufs_test_ ## test_name ## _write,			\
};

#define add_test(utd, test_name, upper_case_name)			\
ufs_test_add_test(utd, UFS_TEST_ ## upper_case_name, "ufs_test_"#test_name,\
				&(ufs_test_ ## test_name ## _ops));	\

enum ufs_test_testcases {
	UFS_TEST_WRITE_READ_TEST,
	UFS_TEST_MULTI_QUERY,
	UFS_TEST_DATA_INTEGRITY,

	UFS_TEST_LONG_SEQUENTIAL_READ,
	UFS_TEST_LONG_SEQUENTIAL_WRITE,
	UFS_TEST_LONG_SEQUENTIAL_MIXED,

	UFS_TEST_LONG_RANDOM_READ,
	UFS_TEST_LONG_RANDOM_WRITE,

	UFS_TEST_PARALLEL_READ_AND_WRITE,
	UFS_TEST_LUN_DEPTH,

	UFS_TEST_READ,
	NUM_TESTS,
};

enum ufs_test_stage {
	DEFAULT,
	UFS_TEST_ERROR,

	UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE1,
	UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE2,

	UFS_TEST_LUN_DEPTH_TEST_RUNNING,
	UFS_TEST_LUN_DEPTH_DONE_ISSUING_REQ,
};

/* device test */
static struct blk_dev_test_type *ufs_bdt;

struct ufs_test_data {
	/* Data structure for debugfs dentrys */
	struct dentry **test_list;
	/*
	 * Data structure containing individual test information, including
	 * self-defined specific data
	 */
	struct test_info test_info;

	/* A wait queue for OPs to complete */
	wait_queue_head_t wait_q;
	/* a flag for write compleation */
	bool queue_complete;
	/*
	 * To determine the number of r/w bios. When seed = 0, random is
	 * disabled and 2 BIOs are written.
	 */
	unsigned int random_test_seed;
	struct dentry *random_test_seed_dentry;

	/* A counter for the number of test requests completed */
	unsigned int completed_req_count;
	/* Test stage */
	enum ufs_test_stage test_stage;

	/* Parameters for maintaining multiple threads */
	int fail_threads;
	atomic_t outstanding_threads;
	struct completion outstanding_complete;

	/* user-defined size of address space in which to perform I/O */
	u32 sector_range;
	/* total number of requests to be submitted in long test */
	u32 long_test_num_reqs;

	struct test_iosched *test_iosched;
};

static int ufs_test_add_test(struct ufs_test_data *utd,
			     enum ufs_test_testcases test_id, char *test_str,
			     const struct file_operations *test_fops)
{
	int ret = 0;
	struct dentry *tests_root;

	if (test_id >= NUM_TESTS)
		return -EINVAL;

	tests_root = utd->test_iosched->debug.debug_tests_root;
	if (!tests_root) {
		pr_err("%s: Failed to create debugfs root", __func__);
		return -EINVAL;
	}

	utd->test_list[test_id] = debugfs_create_file(test_str,
						      S_IRUGO | S_IWUGO,
						      tests_root, utd,
						      test_fops);
	if (!utd->test_list[test_id]) {
		pr_err("%s: Could not create the test %s", test_str, __func__);
		ret = -ENOMEM;
	}
	return ret;
}

/**
 * struct test_scenario - keeps scenario data that creates unique pattern
 * @td: per test reference
 * @direction: pattern initial direction
 * @toggle_direction: every toggle_direction requests switch direction for one
 *			request
 * @total_req: number of request to issue
 * @rnd_req: should request issue to random LBA with random size
 * @run_q: the maximum number of request to hold in queue (before run_queue())
 */
struct test_scenario {
	struct test_iosched *test_iosched;
	int direction;
	int toggle_direction;
	int total_req;
	bool rnd_req;
	int run_q;
};

enum scenario_id {
	/* scenarios for parallel read and write test */
	SCEN_RANDOM_READ_50,
	SCEN_RANDOM_WRITE_50,

	SCEN_RANDOM_READ_32_NO_FLUSH,
	SCEN_RANDOM_WRITE_32_NO_FLUSH,

	SCEN_RANDOM_MAX,
};

static struct test_scenario test_scenario[SCEN_RANDOM_MAX] = {
	{NULL, READ, 0, 50, true, 5},	/* SCEN_RANDOM_READ_50 */
	{NULL, WRITE, 0, 50, true, 5},	/* SCEN_RANDOM_WRITE_50 */

	/* SCEN_RANDOM_READ_32_NO_FLUSH */
	{NULL, READ, 0, 32, true, 64},
	/* SCEN_RANDOM_WRITE_32_NO_FLUSH */
	{NULL, WRITE, 0, 32, true, 64},
};

static
struct test_scenario *get_scenario(struct test_iosched *test_iosched,
				   enum scenario_id id)
{
	struct test_scenario *ret = &test_scenario[id];

	ret->test_iosched = test_iosched;
	return ret;
}

static char *ufs_test_get_test_case_str(int testcase)
{
	switch (testcase) {
	case UFS_TEST_WRITE_READ_TEST:
		return "UFS write read test";
	case UFS_TEST_MULTI_QUERY:
		return "Test multiple queries at the same time";
	case UFS_TEST_LONG_RANDOM_READ:
		return "UFS long random read test";
	case UFS_TEST_LONG_RANDOM_WRITE:
		return "UFS long random write test";
	case UFS_TEST_DATA_INTEGRITY:
		return "UFS random data integrity test";
	case UFS_TEST_LONG_SEQUENTIAL_READ:
		return "UFS long sequential read test";
	case UFS_TEST_LONG_SEQUENTIAL_WRITE:
		return "UFS long sequential write test";
	case UFS_TEST_LONG_SEQUENTIAL_MIXED:
		return "UFS long sequential mixed test";
	case UFS_TEST_PARALLEL_READ_AND_WRITE:
		return "UFS parallel read and write test";
	case UFS_TEST_LUN_DEPTH:
		return "UFS LUN depth test";
	case UFS_TEST_READ:
		return "UFS test READ";
	}
	return "Unknown test";
}

static unsigned int ufs_test_pseudo_random_seed(unsigned int *seed_number,
						unsigned int min_val,
						unsigned int max_val)
{
	int ret = 0;

	if (!seed_number)
		return 0;

	*seed_number = ((unsigned int)(((unsigned long)*seed_number
					* (unsigned long)LARGE_PRIME_1) +
				       LARGE_PRIME_2));
	ret = (unsigned int)((*seed_number) % max_val);

	return (ret > min_val ? ret : min_val);
}

/**
 * pseudo_rnd_sector_and_size - provides random sector and size for test request
 * @seed: random seed
 * @min_start_sector: minimum lba
 * @start_sector: pointer for output start sector
 * @num_of_bios: pointer for output number of bios
 *
 * Note that for UFS sector number has to be aligned with block size. Since
 * scsi will send the block number as the LBA.
 */
static void pseudo_rnd_sector_and_size(struct ufs_test_data *utd,
				       unsigned int *start_sector,
				       unsigned int *num_of_bios)
{
	struct test_iosched *tios = utd->test_iosched;
	u32 min_start_sector = tios->start_sector;
	unsigned int max_sec = min_start_sector + utd->sector_range;

	do {
		*start_sector =
		    ufs_test_pseudo_random_seed(&utd->random_test_seed, 1,
						max_sec);
		*num_of_bios =
		    ufs_test_pseudo_random_seed(&utd->random_test_seed, 1,
						TEST_MAX_BIOS_PER_REQ);
		if (!(*num_of_bios))
			*num_of_bios = 1;
	} while ((*start_sector < min_start_sector) ||
		 (*start_sector + (*num_of_bios * TEST_BIO_SIZE)) > max_sec);
	/*
	 * The test-iosched API is working with sectors 512b, while UFS LBA
	 * is in blocks (4096). Thus the last 3 bits has to be cleared.
	 */
	*start_sector &= ~SECTOR_TO_BLOCK_MASK;
}

static void ufs_test_pseudo_rnd_size(unsigned int *seed,
				     unsigned int *num_of_bios)
{
	*num_of_bios = ufs_test_pseudo_random_seed(seed, 1,
						   TEST_MAX_BIOS_PER_REQ);
	if (!(*num_of_bios))
		*num_of_bios = DEFAULT_NUM_OF_BIOS;
}

static int ufs_test_pm_runtime_cfg_sync(struct test_iosched *tios,
					       bool enable)
{
	struct scsi_device *sdev;
	struct ufs_hba *hba;
	int ret;

	BUG_ON(!tios || !tios->req_q || !tios->req_q->queuedata);
	sdev = (struct scsi_device *)tios->req_q->queuedata;
	BUG_ON(!sdev->host);
	hba = shost_priv(sdev->host);
	BUG_ON(!hba);

	if (enable) {
		ret = pm_runtime_get_sync(hba->dev);
		/* Positive non-zero return values are not errors */
		if (ret < 0) {
			pr_err("%s: pm_runtime_get_sync failed, ret=%d\n",
			       __func__, ret);
			return ret;
		}
		return 0;
	}
	pm_runtime_put_sync(hba->dev);
	return 0;
}

static int ufs_test_show(struct seq_file *file, int test_case)
{
	char *test_description;

	switch (test_case) {
	case UFS_TEST_WRITE_READ_TEST:
		test_description = "\nufs_write_read_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test write once a random block and than reads it to "
		    "verify its content. Used to debug first time transactions.\n";
		break;
	case UFS_TEST_MULTI_QUERY:
		test_description = "Test multiple queries at the same time.\n";
		break;
	case UFS_TEST_DATA_INTEGRITY:
		test_description = "\nufs_data_integrity_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test writes 118 requests of size 4KB to randomly chosen LBAs.\n"
		    "The test then reads from these LBAs and checks that the\n"
		    "correct buffer has been read.\n";
		break;
	case UFS_TEST_LONG_SEQUENTIAL_READ:
		test_description = "\nufs_long_sequential_read_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test runs the following scenarios\n"
		    "- Long Sequential Read Test: this test measures read "
		    "throughput at the driver level by sequentially reading many "
		    "large requests.\n";
		break;
	case UFS_TEST_LONG_RANDOM_READ:
		test_description = "\nufs_long_random_read_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test runs the following scenarios\n"
		    "- Long Random Read Test: this test measures read "
		    "IOPS at the driver level by reading many 4KB requests"
		    "with random LBAs\n";
		break;
	case UFS_TEST_LONG_SEQUENTIAL_WRITE:
		test_description = "\nufs_long_sequential_write_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test runs the following scenarios\n"
		    "- Long Sequential Write Test: this test measures write "
		    "throughput at the driver level by sequentially writing many "
		    "large requests\n";
		break;
	case UFS_TEST_LONG_RANDOM_WRITE:
		test_description = "\nufs_long_random_write_test\n"
		    "=========\n"
		    "Description:\n"
		    "This test runs the following scenarios\n"
		    "- Long Random Write Test: this test measures write "
		    "IOPS at the driver level by writing many 4KB requests"
		    "with random LBAs\n";
		break;
	case UFS_TEST_LONG_SEQUENTIAL_MIXED:
		test_description = "\nufs_long_sequential_mixed_test_read\n"
		    "=========\n"
		    "Description:\n"
		    "The test will verify correctness of sequential data pattern "
		    "written to the device while new data (with same pattern) is "
		    "written simultaneously.\n"
		    "First this test will run a long sequential write scenario."
		    "This first stage will write the pattern that will be read "
		    "later. Second, sequential read requests will read and "
		    "compare the same data. The second stage reads, will issue in "
		    "Parallel to write requests with the same LBA and size.\n"
		    "NOTE: The test requires a long timeout.\n";
		break;
	case UFS_TEST_PARALLEL_READ_AND_WRITE:
		test_description = "\nufs_test_parallel_read_and_write\n"
		    "=========\n"
		    "Description:\n"
		    "This test initiate two threads. Each thread is issuing "
		    "multiple random requests. One thread will issue only read "
		    "requests, while the other will only issue write requests.\n";
		break;
	case UFS_TEST_LUN_DEPTH:
		test_description = "\nufs_test_lun_depth\n"
		    "=========\n"
		    "Description:\n"
		    "This test is trying to stress the edge cases of the UFS "
		    "device queue. This queue has two such edges, the total queue "
		    "depth and the command per LU. To test those edges properly, "
		    "two deviations from the edge in addition to the edge are "
		    "tested as well. One deviation will be fixed (1), and the "
		    "second will be picked randomly.\n"
		    "The test will fill a request queue with random read "
		    "requests. The amount of request will vary each iteration and "
		    "will be either the one of the edges or the sum of this edge "
		    "with one deviations.\n"
		    "The test will test for each iteration once only reads and "
		    "once only writes.\n";
		break;
	case UFS_TEST_READ:
		test_description = "\nufs_test_read\n";
		break;
	default:
		test_description = "Unknown test";
	}

	seq_puts(file, test_description);
	return 0;
}

static struct gendisk *ufs_test_get_rq_disk(struct test_iosched *test_iosched)
{
	struct request_queue *req_q = test_iosched->req_q;
	struct scsi_device *sd;

	if (!req_q) {
		pr_info("%s: Could not fetch request_queue", __func__);
		goto exit;
	}

	sd = (struct scsi_device *)req_q->queuedata;
	if (!sd) {
		pr_info("%s: req_q is missing required queuedata", __func__);
		goto exit;
	}

	return scsi_gendisk_get_from_dev(&sd->sdev_gendev);

exit:
	return NULL;
}

static int ufs_test_put_gendisk(struct test_iosched *test_iosched)
{
	struct request_queue *req_q = test_iosched->req_q;
	struct scsi_device *sd;
	int ret = 0;

	if (!req_q) {
		pr_info("%s: Could not fetch request_queue", __func__);
		ret = -EINVAL;
		goto exit;
	}

	sd = (struct scsi_device *)req_q->queuedata;
	if (!sd) {
		pr_info("%s: req_q is missing required queuedata", __func__);
		ret = -EINVAL;
		goto exit;
	}

	scsi_gendisk_put(&sd->sdev_gendev);

exit:
	return ret;
}

static int ufs_test_prepare(struct test_iosched *tios)
{
	return ufs_test_pm_runtime_cfg_sync(tios, true);
}

static int ufs_test_post(struct test_iosched *tios)
{
	int ret;

	ret = ufs_test_pm_runtime_cfg_sync(tios, false);
	if (!ret)
		ret = ufs_test_put_gendisk(tios);

	return ret;
}

static int ufs_test_check_result(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	if (utd->test_stage == UFS_TEST_ERROR) {
		pr_err("%s: An error occurred during the test", __func__);
		return TEST_FAILED;
	}

	if (utd->fail_threads != 0) {
		pr_err("%s: About %d threads failed during execution",
		       __func__, utd->fail_threads);
		return utd->fail_threads;
	}

	return 0;
}

static bool ufs_write_read_completion(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	if (!utd->queue_complete) {
		utd->queue_complete = true;
		wake_up(&utd->wait_q);
		return false;
	}
	return true;
}

static int ufs_test_run_write_read_test(struct test_iosched *test_iosched)
{
	int ret = 0;
	unsigned int start_sec;
	unsigned int num_bios;
	struct request_queue *q = test_iosched->req_q;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	start_sec = test_iosched->start_sector + sizeof(int) * BIO_U32_SIZE
	    * test_iosched->num_of_write_bios;
	if (utd->random_test_seed != 0)
		ufs_test_pseudo_rnd_size(&utd->random_test_seed, &num_bios);
	else
		num_bios = DEFAULT_NUM_OF_BIOS;

	/* Adding a write request */
	pr_info("%s: Adding a write request with %d bios to Q, req_id=%d",
		__func__, num_bios, test_iosched->wr_rd_next_req_id);

	utd->queue_complete = false;
	ret = test_iosched_add_wr_rd_test_req(test_iosched, 0, WRITE, start_sec,
					      num_bios, TEST_PATTERN_5A, NULL);
	if (ret) {
		pr_err("%s: failed to add a write request", __func__);
		return ret;
	}

	/* waiting for the write request to finish */
	blk_post_runtime_resume(q, 0);
	wait_event(utd->wait_q, utd->queue_complete);

	/* Adding a read request */
	pr_info("%s: Adding a read request to Q", __func__);

	ret = test_iosched_add_wr_rd_test_req(test_iosched, 0, READ, start_sec,
					      num_bios, TEST_PATTERN_5A, NULL);
	if (ret) {
		pr_err("%s: failed to add a read request", __func__);
		return ret;
	}

	blk_post_runtime_resume(q, 0);
	return ret;
}

static void ufs_test_thread_complete(struct ufs_test_data *utd, int result)
{
	if (result)
		utd->fail_threads++;
	atomic_dec(&utd->outstanding_threads);
	if (!atomic_read(&utd->outstanding_threads))
		complete(&utd->outstanding_complete);
}

#if 1
static void ufs_test_random_async_query(void *data, async_cookie_t cookie)
{
	int op;
	struct test_iosched *test_iosched = data;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;
	struct scsi_device *sdev;
	struct ufs_hba *hba;
	int buff_len = QUERY_DESC_UNIT_MAX_SIZE;
	u8 desc_buf[QUERY_DESC_UNIT_MAX_SIZE];
	bool flag;
	u32 att;
	int ret = 0;

	sdev = (struct scsi_device *)test_iosched->req_q->queuedata;
	BUG_ON(!sdev->host);
	hba = shost_priv(sdev->host);
	BUG_ON(!hba);

	op = ufs_test_pseudo_random_seed(&utd->random_test_seed, 1, 8);
	/*
	 * When write data (descriptor/attribute/flag) queries are issued,
	 * regular work and functionality must be kept. The data is read
	 * first to make sure the original state is restored.
	 */
	switch (op) {
	case UPIU_QUERY_OPCODE_READ_DESC:
	case UPIU_QUERY_OPCODE_WRITE_DESC:
		ret =
		    ufshcd_query_descriptor_retry(hba,
						  UPIU_QUERY_OPCODE_READ_DESC,
						  QUERY_DESC_IDN_UNIT, 0, 0,
						  desc_buf, &buff_len);
		break;
	case UPIU_QUERY_OPCODE_WRITE_ATTR:
	case UPIU_QUERY_OPCODE_READ_ATTR:
		ret = ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_READ_ATTR,
					QUERY_ATTR_IDN_EE_CONTROL, 0, 0, &att);
		if (ret || op == UPIU_QUERY_OPCODE_READ_ATTR)
			break;

		ret = ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
					QUERY_ATTR_IDN_EE_CONTROL, 0, 0, &att);
		break;
	case UPIU_QUERY_OPCODE_READ_FLAG:
	case UPIU_QUERY_OPCODE_SET_FLAG:
	case UPIU_QUERY_OPCODE_CLEAR_FLAG:
	case UPIU_QUERY_OPCODE_TOGGLE_FLAG:
		/* We read the QUERY_FLAG_IDN_BKOPS_EN and restore it later */
		ret = ufshcd_query_flag(hba, UPIU_QUERY_OPCODE_READ_FLAG,
					QUERY_FLAG_IDN_BKOPS_EN, &flag);
		if (ret || op == UPIU_QUERY_OPCODE_READ_FLAG)
			break;

		/* After changing the flag we have to change it back */
		ret = ufshcd_query_flag(hba, op, QUERY_FLAG_IDN_BKOPS_EN, NULL);
		if ((op == UPIU_QUERY_OPCODE_SET_FLAG && flag) ||
		    (op == UPIU_QUERY_OPCODE_CLEAR_FLAG && !flag))
			/* No need to change it back */
			break;

		if (flag)
			ret |= ufshcd_query_flag(hba,
						 UPIU_QUERY_OPCODE_SET_FLAG,
						 QUERY_FLAG_IDN_BKOPS_EN, NULL);
		else
			ret |= ufshcd_query_flag(hba,
						 UPIU_QUERY_OPCODE_CLEAR_FLAG,
						 QUERY_FLAG_IDN_BKOPS_EN, NULL);
		break;
	default:
		pr_err("%s: Random error unknown op %d", __func__, op);
	}

	if (ret)
		pr_err("%s: Query thread with op %d, failed with err %d",
		       __func__, op, ret);

	ufs_test_thread_complete(utd, ret);
}
#endif
static void scenario_free_end_io_fn(struct request *rq, int err)
{
	struct test_request *test_rq;
	struct test_iosched *test_iosched = rq->q->elevator->elevator_data;

	BUG_ON(!rq);
	test_rq = (struct test_request *)rq->elv.priv[0];
	BUG_ON(!test_rq);

	spin_lock_irq(&test_iosched->lock);
	test_iosched->dispatched_count--;
	list_del_init(&test_rq->queuelist);
	__blk_put_request(test_iosched->req_q, test_rq->rq);
	spin_unlock_irq(&test_iosched->lock);

	test_iosched_free_test_req_data_buffer(test_rq);
	kfree(test_rq);

	if (err)
		pr_err("%s: request %d completed, err=%d\n", __func__,
		       test_rq->req_id, err);

	check_test_completion(test_iosched);
}

static bool ufs_test_multi_thread_completion(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;
	return atomic_read(&utd->outstanding_threads) <= 0 &&
	    utd->test_stage != UFS_TEST_LUN_DEPTH_TEST_RUNNING;
}

static bool long_rand_test_check_completion(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	if (utd->completed_req_count > utd->long_test_num_reqs) {
		pr_err
		    ("%s: Error: Completed more requests than total test requests,Terminating test\n",
		     __func__);
		return true;
	}
	return utd->completed_req_count == utd->long_test_num_reqs;
}

static bool long_seq_test_check_completion(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	if (utd->completed_req_count > utd->long_test_num_reqs) {
		pr_err
		    ("%s: Error: Completed more requests than total test requests\n",
		     __func__);
		pr_err("%s: Terminating test\n", __func__);
		return true;
	}
	return utd->completed_req_count == utd->long_test_num_reqs;
}

/**
 * ufs_test_toggle_direction() - decides whether toggling is
 * needed. Toggle factor zero means no toggling.
 *
 * toggle_factor - iteration to toggle = toggling frequency
 * iteration - the current request iteration
 *
 * Returns nonzero if toggling is needed, and 0 when toggling is
 * not needed.
 */
static inline int ufs_test_toggle_direction(int toggle_factor, int iteration)
{
	if (!toggle_factor)
		return 0;

	return !(iteration % toggle_factor);
}

static void ufs_test_run_scenario(void *data, async_cookie_t cookie)
{
	struct test_scenario *ts = (struct test_scenario *)data;
	struct test_iosched *test_iosched = ts->test_iosched;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;
	int start_sec;
	int i;
	int ret = 0;

	BUG_ON(!ts);
	start_sec = ts->test_iosched->start_sector;

	for (i = 0; i < ts->total_req; i++) {
		int num_bios = DEFAULT_NUM_OF_BIOS;
		int direction;

		if (ufs_test_toggle_direction(ts->toggle_direction, i))
			direction = (ts->direction == WRITE) ? READ : WRITE;
		else
			direction = ts->direction;

		/* use randomly generated requests */
		if (ts->rnd_req && utd->random_test_seed != 0)
			pseudo_rnd_sector_and_size(utd, &start_sec, &num_bios);

		ret = test_iosched_add_wr_rd_test_req(test_iosched, 0,
						      direction, start_sec,
						      num_bios, TEST_PATTERN_5A,
						      scenario_free_end_io_fn);
		if (ret) {
			pr_err("%s: failed to create request", __func__);
			break;
		}

		/*
		 * We want to run the queue every run_q requests, or,
		 * when the requests pool is exhausted
		 */

		if (test_iosched->dispatched_count >= QUEUE_MAX_REQUESTS ||
		    (ts->run_q && !(i % ts->run_q)))
			blk_post_runtime_resume(test_iosched->req_q, 0);
	}

	blk_post_runtime_resume(test_iosched->req_q, 0);
	ufs_test_thread_complete(utd, ret);
}

static int ufs_test_run_multi_query_test(struct test_iosched *test_iosched)
{
	int i;
	struct ufs_test_data *utd;
	struct scsi_device *sdev;
	struct ufs_hba *hba;

	BUG_ON(!test_iosched || !test_iosched->req_q ||
	       !test_iosched->req_q->queuedata);
	sdev = (struct scsi_device *)test_iosched->req_q->queuedata;
	BUG_ON(!sdev->host);
	hba = shost_priv(sdev->host);
	BUG_ON(!hba);

	utd = test_iosched->blk_dev_test_data;
	atomic_set(&utd->outstanding_threads, 0);
	utd->fail_threads = 0;
	init_completion(&utd->outstanding_complete);
	for (i = 0; i < MAX_PARALLEL_QUERIES; ++i) {
		atomic_inc(&utd->outstanding_threads);
		async_schedule(ufs_test_random_async_query, test_iosched);
	}

	if (!wait_for_completion_timeout(&utd->outstanding_complete,
					 THREADS_COMPLETION_TIMOUT))
		pr_err("%s: Multi-query test timed-out %d threads left",
		       __func__, atomic_read(&utd->outstanding_threads));

	test_iosched_mark_test_completion(test_iosched);
	return 0;
}

static int ufs_test_run_parallel_read_and_write_test(struct test_iosched
						     *test_iosched)
{
	struct test_scenario *read_data, *write_data;
	int i;
	bool changed_seed = false;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	read_data = get_scenario(test_iosched, SCEN_RANDOM_READ_50);
	write_data = get_scenario(test_iosched, SCEN_RANDOM_WRITE_50);

	/* allow randomness even if user forgot */
	if (utd->random_test_seed <= 0) {
		changed_seed = true;
		utd->random_test_seed = 1;
	}

	atomic_set(&utd->outstanding_threads, 0);
	utd->fail_threads = 0;
	init_completion(&utd->outstanding_complete);

	for (i = 0; i < (RANDOM_REQUEST_THREADS / 2); i++) {
		async_schedule(ufs_test_run_scenario, read_data);
		async_schedule(ufs_test_run_scenario, write_data);
		atomic_add(2, &utd->outstanding_threads);
	}

	if (!wait_for_completion_timeout(&utd->outstanding_complete,
					 THREADS_COMPLETION_TIMOUT))
		pr_err("%s: Multi-thread test timed-out %d threads left",
		       __func__, atomic_read(&utd->outstanding_threads));

	check_test_completion(test_iosched);

	/* clear random seed if changed */
	if (changed_seed)
		utd->random_test_seed = 0;

	return 0;
}

static void ufs_test_run_synchronous_scenario(struct test_scenario *read_data)
{
	struct ufs_test_data *utd = read_data->test_iosched->blk_dev_test_data;
	init_completion(&utd->outstanding_complete);
	atomic_set(&utd->outstanding_threads, 1);
	async_schedule(ufs_test_run_scenario, read_data);
	if (!wait_for_completion_timeout(&utd->outstanding_complete,
					 THREADS_COMPLETION_TIMOUT))
		pr_err("%s: Multi-thread test timed-out %d threads left",
		       __func__, atomic_read(&utd->outstanding_threads));
}

static int ufs_test_run_lun_depth_test(struct test_iosched *test_iosched)
{
	struct test_scenario *read_data, *write_data;
	struct scsi_device *sdev;
	bool changed_seed = false;
	int i = 0, num_req[LUN_DEPTH_TEST_SIZE];
	int lun_qdepth, nutrs, num_scenarios;
	struct ufs_test_data *utd;

	BUG_ON(!test_iosched || !test_iosched->req_q ||
	       !test_iosched->req_q->queuedata);
	sdev = (struct scsi_device *)test_iosched->req_q->queuedata;
	lun_qdepth = sdev->max_queue_depth;
	nutrs = sdev->host->can_queue;
	utd = test_iosched->blk_dev_test_data;

	/* allow randomness even if user forgot */
	if (utd->random_test_seed <= 0) {
		changed_seed = true;
		utd->random_test_seed = 1;
	}

	/* initialize the number of request for each iteration */
	num_req[i++] =
	    ufs_test_pseudo_random_seed(&utd->random_test_seed, 1,
					lun_qdepth - 2);
	num_req[i++] = lun_qdepth - 1;
	num_req[i++] = lun_qdepth;
	num_req[i++] = lun_qdepth + 1;
	/* if (nutrs-lun_qdepth-2 <= 0), do not run this scenario */
	if (nutrs - lun_qdepth - 2 > 0)
		num_req[i++] =
		    lun_qdepth + 1 +
		    ufs_test_pseudo_random_seed(&utd->random_test_seed, 1,
						nutrs - lun_qdepth - 2);

	/* if nutrs == lun_qdepth, do not run these three scenarios */
	if (nutrs != lun_qdepth) {
		num_req[i++] = nutrs - 1;
		num_req[i++] = nutrs;
		num_req[i++] = nutrs + 1;
	}

	/* a random number up to 10, not to cause overflow or timeout */
	num_req[i++] =
	    nutrs + 1 + ufs_test_pseudo_random_seed(&utd->random_test_seed, 1,
						    10);

	num_scenarios = i;
	utd->test_stage = UFS_TEST_LUN_DEPTH_TEST_RUNNING;
	utd->fail_threads = 0;
	read_data = get_scenario(test_iosched, SCEN_RANDOM_READ_32_NO_FLUSH);
	write_data = get_scenario(test_iosched, SCEN_RANDOM_WRITE_32_NO_FLUSH);

	for (i = 0; i < num_scenarios; i++) {
		int reqs = num_req[i];

		read_data->total_req = reqs;
		write_data->total_req = reqs;

		ufs_test_run_synchronous_scenario(read_data);
		ufs_test_run_synchronous_scenario(write_data);
	}

	utd->test_stage = UFS_TEST_LUN_DEPTH_DONE_ISSUING_REQ;
	check_test_completion(test_iosched);

	/* clear random seed if changed */
	if (changed_seed)
		utd->random_test_seed = 0;

	return 0;
}

static void long_test_free_end_io_fn(struct request *rq, int err)
{
	struct test_request *test_rq;
	struct test_iosched *test_iosched = rq->q->elevator->elevator_data;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	if (!rq) {
		pr_err("%s: error: NULL request", __func__);
		return;
	}

	test_rq = (struct test_request *)rq->elv.priv[0];

	BUG_ON(!test_rq);

	spin_lock_irq(&test_iosched->lock);
	test_iosched->dispatched_count--;
	list_del_init(&test_rq->queuelist);
	__blk_put_request(test_iosched->req_q, test_rq->rq);
	spin_unlock_irq(&test_iosched->lock);

	if (utd->test_stage == UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE2 &&
	    rq_data_dir(rq) == READ && compare_buffer_to_pattern(test_rq)) {
		/* if the pattern does not match */
		pr_err("%s: read pattern not as expected", __func__);
		utd->test_stage = UFS_TEST_ERROR;
		check_test_completion(test_iosched);
		return;
	}

	test_iosched_free_test_req_data_buffer(test_rq);
	kfree(test_rq);
	utd->completed_req_count++;

	if (err)
		pr_err("%s: request %d completed, err=%d", __func__,
		       test_rq->req_id, err);

	check_test_completion(test_iosched);
}

/**
 * run_long_test - main function for long sequential test
 * @td - test specific data
 *
 * This function is used to fill up (and keep full) the test queue with
 * requests. There are two scenarios this function works with:
 * 1. Only read/write (STAGE_1 or no stage)
 * 2. Simultaneous read and write to the same LBAs (STAGE_2)
 */
static int run_long_test(struct test_iosched *test_iosched)
{
	int ret = 0;
	int direction, num_bios_per_request;
	static unsigned int inserted_requests;
	u32 sector, seed, num_bios, seq_sector_delta;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	BUG_ON(!test_iosched);
	sector = test_iosched->start_sector;
	if (test_iosched->sector_range)
		utd->sector_range = test_iosched->sector_range;
	else
		utd->sector_range = TEST_DEFAULT_SECTOR_RANGE;

	if (utd->test_stage != UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE2) {
		test_iosched->test_count = 0;
		utd->completed_req_count = 0;
		inserted_requests = 0;
	}

	/* Set test parameters */
	switch (test_iosched->test_info.testcase) {
	case UFS_TEST_LONG_RANDOM_READ:
		num_bios_per_request = 1;
		utd->long_test_num_reqs = (utd->sector_range * SECTOR_SIZE) /
		    (LONG_RAND_TEST_REQ_RATIO * TEST_BIO_SIZE *
		     num_bios_per_request);
		direction = READ;
		break;
	case UFS_TEST_LONG_RANDOM_WRITE:
		num_bios_per_request = 1;
		utd->long_test_num_reqs = (utd->sector_range * SECTOR_SIZE) /
		    (LONG_RAND_TEST_REQ_RATIO * TEST_BIO_SIZE *
		     num_bios_per_request);
		direction = WRITE;
		break;
	case UFS_TEST_LONG_SEQUENTIAL_READ:
		num_bios_per_request = TEST_MAX_BIOS_PER_REQ;
		utd->long_test_num_reqs = (utd->sector_range * SECTOR_SIZE) /
		    (num_bios_per_request * TEST_BIO_SIZE);
		direction = READ;
		break;
	case UFS_TEST_LONG_SEQUENTIAL_WRITE:
	case UFS_TEST_LONG_SEQUENTIAL_MIXED:
		num_bios_per_request = TEST_MAX_BIOS_PER_REQ;
		utd->long_test_num_reqs = (utd->sector_range * SECTOR_SIZE) /
		    (num_bios_per_request * TEST_BIO_SIZE);
		direction = WRITE;
		break;
	default:
		direction = WRITE;
	}

	seq_sector_delta = num_bios_per_request * (TEST_BIO_SIZE / SECTOR_SIZE);

	seed = utd->random_test_seed ? utd->random_test_seed : MAGIC_SEED;

	pr_info("%s: Adding %d requests, first req_id=%d", __func__,
		utd->long_test_num_reqs, test_iosched->wr_rd_next_req_id);

	do {
		/*
		 * since our requests come from a pool containing 128
		 * requests, we don't want to exhaust this quantity,
		 * therefore we add up to QUEUE_MAX_REQUESTS (which
		 * includes a safety margin) and then call the block layer
		 * to fetch them
		 */
		if (test_iosched->test_count >= QUEUE_MAX_REQUESTS) {
			blk_post_runtime_resume(test_iosched->req_q, 0);
			continue;
		}

		switch (test_iosched->test_info.testcase) {
		case UFS_TEST_LONG_SEQUENTIAL_READ:
		case UFS_TEST_LONG_SEQUENTIAL_WRITE:
		case UFS_TEST_LONG_SEQUENTIAL_MIXED:
			/* don't need to increment on the first iteration */
			if (inserted_requests)
				sector += seq_sector_delta;
			break;
		case UFS_TEST_LONG_RANDOM_READ:
		case UFS_TEST_LONG_RANDOM_WRITE:
			pseudo_rnd_sector_and_size(utd, &sector, &num_bios);
			break;
		default:
			break;
		}

		ret = test_iosched_add_wr_rd_test_req(test_iosched, 0,
						      direction, sector,
						      num_bios_per_request,
						      TEST_PATTERN_5A,
						      long_test_free_end_io_fn);
		if (ret) {
			pr_err("%s: failed to create request", __func__);
			break;
		}
		inserted_requests++;
		if (utd->test_stage == UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE2) {
			ret = test_iosched_add_wr_rd_test_req(test_iosched, 0,
							      READ, sector,
							      num_bios_per_request,
							      TEST_PATTERN_5A,
							      long_test_free_end_io_fn);
			if (ret) {
				pr_err("%s: failed to create request",
				       __func__);
				break;
			}
			inserted_requests++;
		}
	} while (inserted_requests < utd->long_test_num_reqs);

	/* in this case the queue will not run in the above loop */
	if (utd->long_test_num_reqs < QUEUE_MAX_REQUESTS)
		blk_post_runtime_resume(test_iosched->req_q, 0);

	return ret;
}

static int run_mixed_long_seq_test(struct test_iosched *test_iosched)
{
	int ret;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	utd->test_stage = UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE1;
	ret = run_long_test(test_iosched);
	if (ret)
		goto out;

	pr_info("%s: First write iteration completed", __func__);
	pr_info("%s: Starting mixed write and reads sequence", __func__);
	utd->test_stage = UFS_TEST_LONG_SEQUENTIAL_MIXED_STAGE2;
	ret = run_long_test(test_iosched);
out:
	return ret;
}

static int long_rand_test_calc_iops(struct test_iosched *test_iosched)
{
	unsigned long mtime, num_ios, iops;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	mtime = ktime_to_ms(utd->test_info.test_duration);
	num_ios = utd->completed_req_count;

	pr_info("%s: time is %lu msec, IOS count is %lu", __func__, mtime,
		num_ios);

	/* preserve some precision */
	num_ios *= 1000;
	/* calculate those iops */
	iops = num_ios / mtime;

	pr_info("%s: IOPS: %lu IOP/sec\n", __func__, iops);

	return ufs_test_post(test_iosched);
}

static int long_seq_test_calc_throughput(struct test_iosched *test_iosched)
{
	unsigned long fraction, integer;
	unsigned long mtime, byte_count;
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	mtime = ktime_to_ms(utd->test_info.test_duration);
	byte_count = utd->test_info.test_byte_count;

	pr_info("%s: time is %lu msec, size is %lu.%lu MiB", __func__, mtime,
		LONG_TEST_SIZE_INTEGER(byte_count),
		LONG_TEST_SIZE_FRACTION(byte_count));

	/* we first multiply in order not to lose precision */
	mtime *= MB_MSEC_RATIO_APPROXIMATION;
	/* divide values to get a MiB/sec integer value with one
	   digit of precision
	 */
	fraction = integer = (byte_count * 10) / mtime;
	integer /= 10;
	/* and calculate the MiB value fraction */
	fraction -= integer * 10;

	pr_info("%s: Throughput: %lu.%lu MiB/sec\n", __func__, integer,
		fraction);

	return ufs_test_post(test_iosched);
}

static bool ufs_data_integrity_completion(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;
	bool ret = false;

	if (!test_iosched->dispatched_count) {
		/* q is empty in this case */
		if (!utd->queue_complete) {
			utd->queue_complete = true;
			wake_up(&utd->wait_q);
		} else {
			/* declare completion only on second time q is empty */
			ret = true;
		}
	}

	return ret;
}

static int ufs_test_run_data_integrity_test(struct test_iosched *test_iosched)
{
	int ret = 0;
	int i, j;
	unsigned int start_sec, num_bios, retries = NUM_UNLUCKY_RETRIES;
	struct request_queue *q = test_iosched->req_q;
	int sectors[QUEUE_MAX_REQUESTS] = { 0 };
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	start_sec = test_iosched->start_sector;
	utd->queue_complete = false;

	if (utd->random_test_seed != 0) {
		ufs_test_pseudo_rnd_size(&utd->random_test_seed, &num_bios);
	} else {
		num_bios = DEFAULT_NUM_OF_BIOS;
		utd->random_test_seed = MAGIC_SEED;
	}

	/* Adding write requests */
	pr_info("%s: Adding %d write requests, first req_id=%d", __func__,
		QUEUE_MAX_REQUESTS, test_iosched->wr_rd_next_req_id);

	for (i = 0; i < QUEUE_MAX_REQUESTS; i++) {
		/* make sure that we didn't draw the same start_sector twice */
		while (retries--) {
			pseudo_rnd_sector_and_size(utd, &start_sec, &num_bios);
			sectors[i] = start_sec;
			/* just increment j */
			for (j = 0; (j < i) && (sectors[i] != sectors[j]); j++) {
				if (j == i)
					break;
			}
		}
		if (!retries) {
			pr_err("%s: too many unlucky start_sector draw retries",
			       __func__);
			ret = -EINVAL;
			return ret;
		}
		retries = NUM_UNLUCKY_RETRIES;

		ret = test_iosched_add_wr_rd_test_req(test_iosched, 0, WRITE,
						      start_sec, 1, i,
						      long_test_free_end_io_fn);
		if (ret) {
			pr_err("%s: failed to add a write request", __func__);
			return ret;
		}
	}

	/* waiting for the write request to finish */
	blk_post_runtime_resume(q, 0);
	wait_event(utd->wait_q, utd->queue_complete);

	/* Adding read requests */
	pr_info("%s: Adding %d read requests, first req_id=%d", __func__,
		QUEUE_MAX_REQUESTS, test_iosched->wr_rd_next_req_id);

	for (i = 0; i < QUEUE_MAX_REQUESTS; i++) {
		ret = test_iosched_add_wr_rd_test_req(test_iosched, 0, READ,
						      sectors[i], 1, i,
						      long_test_free_end_io_fn);
		if (ret) {
			pr_err("%s: failed to add a read request", __func__);
			return ret;
		}
	}

	blk_post_runtime_resume(q, 0);
	return ret;
}

static ssize_t ufs_test_write(struct file *file, const char __user *buf,
			      size_t count, loff_t *ppos, int test_case)
{
	int ret = 0;
	int i;
	int number;
	struct seq_file *seq_f = file->private_data;
	struct ufs_test_data *utd = seq_f->private;

	ret = kstrtoint_from_user(buf, count, 0, &number);
	if (ret < 0) {
		pr_err("%s: Error while reading test parameter value %d",
		       __func__, ret);
		return ret;
	}

	if (number <= 0)
		number = 1;

	pr_info("%s:the test will run for %d iterations.", __func__, number);
	memset(&utd->test_info, 0, sizeof(struct test_info)); /* unsafe_function_ignore: memset */

	/* Initializing test */
	utd->test_info.data = utd;
	utd->test_info.get_test_case_str_fn = ufs_test_get_test_case_str;
	utd->test_info.testcase = test_case;
	utd->test_info.get_rq_disk_fn = ufs_test_get_rq_disk;
	utd->test_info.check_test_result_fn = ufs_test_check_result;
	utd->test_info.post_test_fn = ufs_test_post;
	utd->test_info.prepare_test_fn = ufs_test_prepare;
	utd->test_stage = DEFAULT;

	pr_info("%s:test_case is %d.", __func__, test_case);
	switch (test_case) {
	case UFS_TEST_WRITE_READ_TEST:
		utd->test_info.run_test_fn = ufs_test_run_write_read_test;
		utd->test_info.check_test_completion_fn =
		    ufs_write_read_completion;
		break;
	case UFS_TEST_MULTI_QUERY:
		utd->test_info.run_test_fn = ufs_test_run_multi_query_test;
		utd->test_info.check_test_result_fn = ufs_test_check_result;
		break;
	case UFS_TEST_DATA_INTEGRITY:
		utd->test_info.run_test_fn = ufs_test_run_data_integrity_test;
		utd->test_info.check_test_completion_fn =
		    ufs_data_integrity_completion;
		break;
	case UFS_TEST_LONG_RANDOM_READ:
	case UFS_TEST_LONG_RANDOM_WRITE:
		utd->test_info.run_test_fn = run_long_test;
		utd->test_info.post_test_fn = long_rand_test_calc_iops;
		utd->test_info.check_test_result_fn = ufs_test_check_result;
		utd->test_info.check_test_completion_fn =
		    long_rand_test_check_completion;
		break;
	case UFS_TEST_LONG_SEQUENTIAL_READ:
	case UFS_TEST_LONG_SEQUENTIAL_WRITE:
		utd->test_info.run_test_fn = run_long_test;
		utd->test_info.post_test_fn = long_seq_test_calc_throughput;
		utd->test_info.check_test_result_fn = ufs_test_check_result;
		utd->test_info.check_test_completion_fn =
		    long_seq_test_check_completion;
		break;
	case UFS_TEST_LONG_SEQUENTIAL_MIXED:
		utd->test_info.timeout_msec = LONG_SEQUENTIAL_MIXED_TIMOUT_MS;
		utd->test_info.run_test_fn = run_mixed_long_seq_test;
		utd->test_info.post_test_fn = long_seq_test_calc_throughput;
		utd->test_info.check_test_result_fn = ufs_test_check_result;
		break;
	case UFS_TEST_PARALLEL_READ_AND_WRITE:
		utd->test_info.run_test_fn =
		    ufs_test_run_parallel_read_and_write_test;
		utd->test_info.check_test_completion_fn =
		    ufs_test_multi_thread_completion;
		break;
	case UFS_TEST_LUN_DEPTH:
		utd->test_info.run_test_fn = ufs_test_run_lun_depth_test;
		break;
	case UFS_TEST_READ:
		utd->test_info.run_test_fn = NULL;
		break;
	default:
		pr_err("%s: Unknown test-case: %d", __func__, test_case);
		WARN_ON(true);
	}

	/* Running the test multiple times */
	for (i = 0; i < number; ++i) {
		pr_info("%s: Cycle # %d / %d", __func__, i + 1, number);
		pr_info("%s: ====================", __func__);

		utd->test_info.test_byte_count = 0;
		ret = test_iosched_start_test(utd->test_iosched,
					      &utd->test_info);
		if (ret) {
			pr_err("%s: Test failed, err=%d.", __func__, ret);
			return ret;
		}

		/* Allow FS requests to be dispatched */
		msleep(1000);
	}

	pr_info("%s: Completed all the ufs test iterations.", __func__);

	return count;
}

TEST_OPS(write_read_test, WRITE_READ_TEST);
TEST_OPS(multi_query, MULTI_QUERY);
TEST_OPS(data_integrity, DATA_INTEGRITY);
TEST_OPS(long_random_read, LONG_RANDOM_READ);
TEST_OPS(long_random_write, LONG_RANDOM_WRITE);
TEST_OPS(long_sequential_read, LONG_SEQUENTIAL_READ);
TEST_OPS(long_sequential_write, LONG_SEQUENTIAL_WRITE);
TEST_OPS(long_sequential_mixed, LONG_SEQUENTIAL_MIXED);
TEST_OPS(parallel_read_and_write, PARALLEL_READ_AND_WRITE);
TEST_OPS(lun_depth, LUN_DEPTH);
TEST_OPS(read, READ);

static void ufs_test_debugfs_cleanup(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;
	debugfs_remove_recursive(test_iosched->debug.debug_root);
	kfree(utd->test_list);
}

static int ufs_test_debugfs_init(struct ufs_test_data *utd)
{
	struct dentry *utils_root, *tests_root;
	int ret = 0;
	struct test_iosched *ts = utd->test_iosched;

	utils_root = ts->debug.debug_utils_root;
	tests_root = ts->debug.debug_tests_root;

	utd->test_list = kmalloc(sizeof(struct dentry *) * NUM_TESTS,
				 GFP_KERNEL);
	if (!utd->test_list) {
		pr_err("%s: failed to allocate tests dentrys", __func__);
		return -ENODEV;
	}

	if (!utils_root || !tests_root) {
		pr_err("%s: Failed to create debugfs root", __func__);
		ret = -EINVAL;
		goto exit_err;
	}

	utd->random_test_seed_dentry = debugfs_create_u32("random_test_seed",
							  S_IRUGO | S_IWUGO,
							  utils_root,
							  &utd->
							  random_test_seed);

	if (!utd->random_test_seed_dentry) {
		pr_err("%s: Could not create debugfs random_test_seed",
		       __func__);
		ret = -ENOMEM;
		goto exit_err;
	}

	ret = add_test(utd, write_read_test, WRITE_READ_TEST);
	if (ret)
		goto exit_err;
	ret = add_test(utd, data_integrity, DATA_INTEGRITY);
	if (ret)
		goto exit_err;
	ret = add_test(utd, long_random_read, LONG_RANDOM_READ);
	if (ret)
		goto exit_err;
	ret = add_test(utd, long_random_write, LONG_RANDOM_WRITE);
	if (ret)
		goto exit_err;
	ret = add_test(utd, long_sequential_read, LONG_SEQUENTIAL_READ);
	if (ret)
		goto exit_err;
	ret = add_test(utd, long_sequential_write, LONG_SEQUENTIAL_WRITE);
	if (ret)
		goto exit_err;
	ret = add_test(utd, long_sequential_mixed, LONG_SEQUENTIAL_MIXED);
	if (ret)
		goto exit_err;
	add_test(utd, multi_query, MULTI_QUERY);
	if (ret)
		goto exit_err;
	add_test(utd, parallel_read_and_write, PARALLEL_READ_AND_WRITE);
	if (ret)
		goto exit_err;
	add_test(utd, lun_depth, LUN_DEPTH);
	if (ret)
		goto exit_err;
	add_test(utd, read, READ);
	if (ret)
		goto exit_err;

	goto exit;

exit_err:
	ufs_test_debugfs_cleanup(ts);
exit:
	return ret;
}

static int ufs_test_probe(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd;
	int ret;

	utd = kzalloc(sizeof(*utd), GFP_KERNEL);
	if (!utd) {
		pr_err("%s: failed to allocate ufs test data\n", __func__);
		return -ENOMEM;
	}

	init_waitqueue_head(&utd->wait_q);
	utd->test_iosched = test_iosched;
	test_iosched->blk_dev_test_data = utd;

	ret = ufs_test_debugfs_init(utd);
	if (ret) {
		pr_err("%s: failed to init debug-fs entries, ret=%d\n",
		       __func__, ret);
		kfree(utd);
	}

	return ret;
}

static void ufs_test_remove(struct test_iosched *test_iosched)
{
	struct ufs_test_data *utd = test_iosched->blk_dev_test_data;

	ufs_test_debugfs_cleanup(test_iosched);
	test_iosched->blk_dev_test_data = NULL;
	kfree(utd);
}

static int __init ufs_test_init(void)
{
	ufs_bdt = kzalloc(sizeof(*ufs_bdt), GFP_KERNEL);
	if (!ufs_bdt)
		return -ENOMEM;

	ufs_bdt->type_prefix = UFS_TEST_BLK_DEV_TYPE_PREFIX;
	ufs_bdt->init_fn = ufs_test_probe;
	ufs_bdt->exit_fn = ufs_test_remove;
	INIT_LIST_HEAD(&ufs_bdt->list);

	test_iosched_register(ufs_bdt);

	return 0;
}

EXPORT_SYMBOL_GPL(ufs_test_init);

static void __exit ufs_test_exit(void)
{
	test_iosched_unregister(ufs_bdt);
	kfree(ufs_bdt);
}

module_init(ufs_test_init);
module_exit(ufs_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("UFC test");
