
#include <trace/iotrace.h>
#include "huawei_platform/log/imonitor.h"
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/export.h>
#include <linux/time.h>
#include <linux/writeback.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/task_io_accounting_ops.h>
#include <linux/trace_clock.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/bootdevice.h>
#include <linux/namei.h>
#include <linux/f2fs_fs.h>
#include <scsi/ufs/ufs.h>
#include <scsi/ufs/ufshcd.h>
#include <scsi/ufs/hufs_plat.h>

typedef u32 block_t;
typedef u32 nid_t;

#include <trace/events/f2fs.h>

/*  data type for block offset of block group */
typedef int ext4_grpblk_t;
/*  data type for filesystem-wide blocks number */
typedef unsigned long long ext4_fsblk_t;
/*  data type for file logical block number */
typedef __u32 ext4_lblk_t;
/*  data type for block group number */
typedef unsigned int ext4_group_t;

#include <trace/events/ext4.h>
#include <trace/events/block.h>
#include <trace/events/mmc.h>
#include <trace/events/scsi.h>

/* ****************************************************************** */

#define io_trace_print(fmt, arg...) printk("[[IO_TRACE] %s:%d]"fmt, __FUNCTION__, __LINE__, ##arg)
#define io_trace_assert(cond) \
do { if (unlikely(!(cond))) \
		io_trace_print("[IO_TRACE] %s\n", #cond); \
} while (0)

enum IO_TRACE_ACTION {
	READ_BEGIN_TAG = 0x1,
	WRITE_BEGIN_TAG,
	EXT4_DA_WRITE_BEGIN_TAG,
	BLOCK_WRITE_BEGIN_TAG,
	EXT4_SYNC_ENTER_TAG,
	F2FS_SYNC_ENTER_TAG,
	/* FIXME: according it alloc pid log entry */
	IO_TRACE_PARENT_END = F2FS_SYNC_ENTER_TAG,

	READ_END_TAG,
	BLOCK_WRITE_END_TAG,
	EXT4_DA_WRITE_END_TAG,
	WRITE_END_TAG,
	WRITE_WAIT_END_TAG, /* in sync file */
	EXT4_SYNC_EXIT_TAG,
	F2FS_SYNC_EXIT_TAG,

	/* lint -save -e849 -e488 */
	IO_TRACE_JANK_TAG = F2FS_SYNC_EXIT_TAG,
	EXT4_MAP_SUBMIT_TAG,

	EXT4_WRITE_PAGES_TAG,
	EXT4_WRITE_PAGE_TAG,
	EXT4_WRITE_PAGES_END_TAG,

	F2FS_SYNC_FS_TAG,
	F2FS_WRITE_CP_TAG,

	BLK_THRO_WEIGHT_TAG,
	BLK_THRO_DISP_TAG,
	BLK_THRO_IOCOST_TAG,
	BLK_THRO_LIMIT_START_TAG,
	BLK_THRO_LIMIT_END_TAG,
	BLK_THRO_BIO_IN_TAG,
	BLK_THRO_BIO_OUT_TAG,

	BLOCK_BIO_DM_TAG,
	BLOCK_BIO_CRYPT_MAP,
	BLOCK_DM_CRYPT_TAG,
	BLOCK_DM_CRYPT_DEC_TAG,

	BLOCK_BIO_REMAP_TAG,
	BLOCK_BIO_QUEUE_TAG,
	BLOCK_BIO_BACKMERGE_TAG,
	BLOCK_BIO_FRONTMERGE_TAG,
	BLOCK_BIO_WBT_TAG,
	BLOCK_GETRQ_TAG,
	BLOCK_SLEEPRQ_TAG,
	BLOCK_RQ_INSERT_TAG,
	BLOCK_RQ_ISSUE_TAG,
	BLOCK_RQ_COMPLETE_TAG,
	BLOCK_RQ_ABORT_TAG,
	BLOCK_RQ_REQUEUE_TAG,
	BLOCK_RQ_SLEEP_TAG,
	BLOCK_EXT4_END_BIO_TAG,
	MMC_BLK_ISSUE_RQ_TAG,
	MMC_BLK_RW_START_TAG,
	MMC_BLK_RW_END_TAG,

	MMC_BLK_CMDQ_RW_START_TAG,
	MMC_BLK_CMDQ_RW_END_TAG,

	SCSI_BLK_CMD_RW_START_TAG,
	SCSI_BLK_CMD_RW_END_TAG,

	F2FS_QUASI_DETECTED_TAG,

	/* lint -restore */
	IO_TRACE_TAG_END,
};

struct io_trace_action {
	unsigned int action;
	unsigned char *name;
};

/* lint -save -e64 */
static struct io_trace_action all_trace_action[] = {
	{0, "0"},
	{READ_BEGIN_TAG, "r_begin"},
	{WRITE_BEGIN_TAG, "w_begin"},
	{EXT4_DA_WRITE_BEGIN_TAG, "e4_w_begin"},
	{BLOCK_WRITE_BEGIN_TAG, "b_w_begin"},
	{EXT4_SYNC_ENTER_TAG, "e4_sy_begin"},
	{F2FS_SYNC_ENTER_TAG, "f2fs_sy_begin"},

	{READ_END_TAG, "r_end"},
	{BLOCK_WRITE_END_TAG, "b_w_end"},
	{EXT4_DA_WRITE_END_TAG, "e4_w_end"},
	{WRITE_END_TAG, "w_end"},
	{WRITE_WAIT_END_TAG, "w_w_end"},
	{EXT4_SYNC_EXIT_TAG, "e4_sy_end"},
	{F2FS_SYNC_EXIT_TAG, "f2fs_sy_end"},

	{EXT4_MAP_SUBMIT_TAG, "e4_map_blk"},

	{EXT4_WRITE_PAGES_TAG, "e4_w_pages"},
	{EXT4_WRITE_PAGE_TAG, "e4_w_page"},
	{EXT4_WRITE_PAGES_END_TAG, "e4_w_pages_e"},

	{F2FS_SYNC_FS_TAG, "f2fs_sync_fs"},
	{F2FS_WRITE_CP_TAG, "f2fs_w_cp"},

	{BLK_THRO_WEIGHT_TAG, "b_t_wgt"},
	{BLK_THRO_DISP_TAG, "b_t_disp"},
	{BLK_THRO_IOCOST_TAG, "b_t_cost"},
	{BLK_THRO_LIMIT_START_TAG, "b_t_lim_s"},
	{BLK_THRO_LIMIT_END_TAG, "b_t_lim_e"},
	{BLK_THRO_BIO_IN_TAG, "b_t_bio_i"},
	{BLK_THRO_BIO_OUT_TAG, "b_t_bio_o"},

	{BLOCK_BIO_DM_TAG, "b_b_dm"},
	{BLOCK_BIO_CRYPT_MAP, "b_crypt_map"},
	{BLOCK_DM_CRYPT_TAG, "b_crypt_s"},
	{BLOCK_DM_CRYPT_DEC_TAG, "b_crypt_e"},

	{BLOCK_BIO_REMAP_TAG, "b_b_remap"},
	{BLOCK_BIO_QUEUE_TAG, "b_b_queue"},
	{BLOCK_BIO_BACKMERGE_TAG, "b_b_back"},
	{BLOCK_BIO_FRONTMERGE_TAG, "b_b_front"},
	{BLOCK_BIO_WBT_TAG, "b_b_wbt"},
	{BLOCK_GETRQ_TAG, "b_getq"},
	{BLOCK_SLEEPRQ_TAG, "b_slq"},
	{BLOCK_RQ_INSERT_TAG, "b_q_insert"},
	{BLOCK_RQ_ISSUE_TAG, "b_q_issue"},
	{BLOCK_RQ_COMPLETE_TAG, "b_q_comp"},
	{BLOCK_RQ_ABORT_TAG, "b_q_abort"},
	{BLOCK_RQ_REQUEUE_TAG, "b_q_requeue"},
	{BLOCK_RQ_SLEEP_TAG, "b_q_sleep"},
	{BLOCK_EXT4_END_BIO_TAG, "b_i_comp"},
	{MMC_BLK_ISSUE_RQ_TAG, "mmc_issue"},
	{MMC_BLK_RW_START_TAG, "mmc_start"},
	{MMC_BLK_RW_END_TAG, "mmc_end"},

	{MMC_BLK_CMDQ_RW_START_TAG, "mmc_cmdq_s"},
	{MMC_BLK_CMDQ_RW_END_TAG, "mmc_cmdq_e"},

	{SCSI_BLK_CMD_RW_START_TAG, "scsi_cmd_s"},
	{SCSI_BLK_CMD_RW_END_TAG, "scsi_cmd_e"},

	{F2FS_QUASI_DETECTED_TAG, "f2fs_d_qu"},
};
/* lint -restore */

static inline int action_is_read(unsigned char action)
{
	if (action == READ_END_TAG ||
			action == READ_BEGIN_TAG)
		return 1;

	return 0;
}

static inline int action_is_write(unsigned char action)
{
	if (action == WRITE_END_TAG ||
			action == EXT4_SYNC_EXIT_TAG ||
				action == WRITE_BEGIN_TAG ||
					action == EXT4_SYNC_ENTER_TAG ||
						action == F2FS_SYNC_ENTER_TAG ||
							action == F2FS_SYNC_EXIT_TAG)
		return 1;

	return 0;
}
/* lint -save -e64 */
enum IO_TRACE_TYPE {
	__IO_TRACE_WRITE,

	__IO_TRACE_SYNC = 4,
	__IO_TRACE_META, /*  metadata io request  */
	__IO_TRACE_PRIO, /*  boost priority in cfq  */
	__IO_TRACE_DISCARD, /*  request to discard sectors  */
	__IO_TRACE_SECURE_ERASE,
	__IO_TRACE_WRITE_SAME,
	__IO_TRACE_FLUSH,
	__IO_TRACE_FUA,
	__IO_TRACE_FG,
};
/* lint -restore */

#define IO_TRACE_READ	 0
#define IO_TRACE_WRITE	(1 << __IO_TRACE_WRITE)
#define IO_TRACE_SYNC	(1 << __IO_TRACE_SYNC)
#define IO_TRACE_META	(1 << __IO_TRACE_META)
#define IO_TRACE_DISCARD	(1 << __IO_TRACE_DISCARD)
#define IO_TRACE_SECURE_ERASE	(1 << __IO_TRACE_SECURE_ERASE)
#define IO_TRACE_WRITE_SAME	(1 <<  __IO_TRACE_WRITE_SAME)
#define IO_TRACE_FLUSH	(1 << __IO_TRACE_FLUSH)
#define IO_TRACE_FUA	(1 << __IO_TRACE_FUA)
#define IO_TRACE_FG	(1 << __IO_TRACE_FG)
/* lint -save -e64 */
static unsigned char *rw_name[] = {
	"R", "W", "N", "N", "S", "M", "P", "D", "E", "A", "F", "U", "G"
};
/* lint -restore */
#define IO_F_NAME_LEN   32
#define IO_P_NAME_LEN   16
struct io_trace_log_entry {
	pid_t pid;
	pid_t tgid;
	unsigned char action;
	unsigned char rw;
	unsigned char cpu;
	unsigned char valid; /* whether it is running */
	unsigned int  nr_bytes;
	unsigned long inode;
	unsigned long time;
	unsigned long delay;
	unsigned long f_time;
	char comm[IO_P_NAME_LEN];
	char f_name[IO_F_NAME_LEN];
};

#define IO_MAX_PROCESS	32768
#define IO_RUNNING_MAX_PROCESS	32

#define IO_MAX_BUF_ENTRY	(4 * 1024)

#define IO_MAX_GLOBAL_ENTRY	(4 * 1024 * 1024)
#define IO_MAX_OUTPUT_ENTRY	(4 * 1024)

#define IO_MAX_OP_LOG_PER_BUF	((IO_MAX_BUF_ENTRY) / sizeof(struct io_trace_output_entry))
#define IO_MAX_LOG_PER_BUF	((IO_MAX_BUF_ENTRY) / sizeof(struct io_trace_log_entry))
#define IO_MAX_LOG_PER_GLOBAL \
	(((IO_MAX_GLOBAL_ENTRY) / sizeof(struct io_trace_log_entry)) - 1)
#define IO_TRACE_LIMIT_LOG	50000000
#define IO_MS_IN_NS			1000000

static unsigned int io_ns_to_ms(unsigned int ns)
{
	return ns / IO_MS_IN_NS;
}

#define UFS_DELAY_DMD_CODE	914009000
static struct task_struct *io_count_thread = NULL;
static int io_count_thread_fn(void *data);
#define UFS_IO_COUNT_UPLOAD_INTER	(2 * 3600)
#define KB			  1024
static struct ufs_hba *fsr_hba = NULL;
#define UFS_VENDOR_HI1861	0x8B6
static unsigned int ufs_manufacture;
static unsigned int dm_ufs_manfid;

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
static int iotrace_ufs_fsr_get(void);
static int io_fsr_log_get(struct file *filp);

#ifdef HI18VV_FSR_GET_THREAD
static struct task_struct *apr_work_thread = NULL;
static int apr_work_thread_fn(void *data);
#endif

#define UFS_HI1861_FSR_DMD_CODE	938008000
#define UFS_HI1861_FSR_UPLOAD_INTER	(1 * 12 * 3600) /* in second  */
#define UFS_HI1861_FSR_UPLOAD_FIRST_INTER	(5 * 60)
#endif

struct io_trace_pid {
	pid_t pid;
	unsigned short running;   /* pid already begin trace?? */
	unsigned long  log_count;
	struct io_trace_log_entry pid_entry[IO_TRACE_PARENT_END + 1];
};

struct io_trace_mem_mngt {
	int index;
	unsigned char *buff[IO_RUNNING_MAX_PROCESS];
	unsigned char *all_trace_buff;
};

struct io_trace_ctrl {
	struct platform_device *pdev;
	unsigned int enable;
	unsigned int first;

	unsigned int   all_log_index;
	unsigned int   all_log_count; /* not more than IO_MAX_LOG_PER_GLOBAL  */

	unsigned int   out_log_index;
	unsigned int   out_log_count;
	unsigned int   out_valid;
	unsigned int   out_test;

	struct file *filp;

	struct io_trace_pid *all_trace[IO_MAX_PROCESS];
	struct io_trace_mem_mngt *mem_mngt;
};

static struct io_trace_ctrl *io_trace_this = NULL;
static DEFINE_RAW_SPINLOCK(trace_buff_spinlock);
static spinlock_t global_spinlock;
static struct mutex output_mutex;

static int trace_mm_init(struct io_trace_ctrl *trace_ctrl);
static void io_trace_setup(struct io_trace_ctrl *trace_ctrl);

static unsigned char *io_mem_mngt_alloc(struct io_trace_mem_mngt *mem_mngt);
static void io_mem_mngt_free(struct io_trace_mem_mngt *mem_mngt, unsigned char *buf);

static ssize_t sysfs_io_trace_attr_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf);
static ssize_t sysfs_io_trace_attr_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t count);

static ssize_t io_log_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t io_log_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

static struct file * open_log_file(char *name);
static int write_log_kernel(struct file *filp, struct io_trace_log_entry *entry);
static int write_io_monlog_kernel(struct file *filp, unsigned char *buf, int len);
static int write_head_kernel(struct file *filp);

#define IO_TRACE_KOBJ_ATTR(_name) { \
.attr = {.name = __stringify(_name), .mode = S_IRUGO | S_IWUSR}, \
.show = sysfs_io_trace_attr_show, \
.store = sysfs_io_trace_attr_store, \
};

static struct kobj_attribute g_kobjAttrEnable = IO_TRACE_KOBJ_ATTR(enable);
static struct kobj_attribute g_kobjAttrPid = IO_TRACE_KOBJ_ATTR(pid);
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
static struct kobj_attribute g_kobjAttrFsr = IO_TRACE_KOBJ_ATTR(fsr);
#endif

static struct kobj_attribute io_log_data_attr = {
	.attr = {.name = "io_data", .mode= 0660},
	.show = io_log_data_show,
	.store = io_log_data_store
};

static struct attribute *io_trace_attrs[] = {
	&g_kobjAttrEnable.attr, /* this attr is just for debug (&g_kobjAttrPid.attr,) */
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	&g_kobjAttrFsr.attr,
#endif
	&io_log_data_attr.attr,
	NULL,
};

struct attribute_group io_trace_attr_group = {
	.name  = "iotrace",
	.attrs = io_trace_attrs,
};

struct io_trace_output_entry {
	unsigned short rw;
	unsigned short action;
	pid_t pid;
	unsigned int  delay;
	unsigned long inode;
	unsigned long time_stamp;
	char comm[IO_P_NAME_LEN];
};

struct io_trace_output_head {
	unsigned char version[4];
	unsigned int data_len;
	unsigned int r_max_delay;
	unsigned int r_aver_delay;
	unsigned int r_total_delay;
	unsigned int w_max_delay;
	unsigned int w_aver_delay;
	unsigned int w_total_delay;
};

struct io_stat_output_entry {
	pid_t pid;
	int   stat_ret;
	unsigned long rchar;
	unsigned long wchar;
	unsigned long syscr;
	unsigned long syscw;
	unsigned long read_bytes;
	unsigned long write_bytes;
	unsigned long cancel_write_bytes;
};

static void io_trace_version(unsigned char *v)
{
	*(v) = 'V';
	*(v + 1) = '2';
	*(v + 2) = '0';
	*(v + 3) = '0';

	return;
}

struct io_trace_interface {
	unsigned int  magic;
	unsigned int  circum;
	unsigned long time_stamp;
	unsigned long time_span;
	unsigned int  buff_len;
	unsigned int  pid_num;
	pid_t pid[0];
};

enum {
	SYS_IO_DATA,
	SYS_IO_STAT,
	SYS_IO_NUM,
};

enum IO_PART_INDEX {
	IO_PART_USER_INDEX = 0,
	IO_PART_SYSTEM_INDEX = 1,
	IO_PART_VENDOR_INDEX = 2,
	IO_PART_OTHER_INDEX = 3,
	IO_PART_INDEX_MAX_NUM = 4
};

struct io_part_volume_index {
	unsigned char *volume;
	unsigned int index;
};

enum IO_OP_TYPE {
	IO_OP_READ,
	IO_OP_WRITE,
	IO_OP_END
};

struct io_blk_stats {
	unsigned int io_latency_max[IO_OP_END];
	unsigned int io_latency_min[IO_OP_END];
	unsigned int io_latency_avg[IO_OP_END];
	unsigned int io_cnt[IO_OP_END];
};

static struct io_part_volume_index partitions_volume_index[] = {
	{ "userdata", IO_PART_USER_INDEX },
	{ "system", IO_PART_SYSTEM_INDEX },
	{ "system_a", IO_PART_SYSTEM_INDEX },
	{ "system_b", IO_PART_SYSTEM_INDEX },
	{ "vendor", IO_PART_VENDOR_INDEX },
	{ "vendor_a", IO_PART_VENDOR_INDEX },
	{ "vendor_b", IO_PART_VENDOR_INDEX },
	{ NULL, IO_PART_OTHER_INDEX }
};

static unsigned char *partitions_name[IO_PART_INDEX_MAX_NUM] = {
	"user",
	"system",
	"vendor",
	"other"
};

/*  add for io delay monitor  */
static struct io_blk_stats io_4k_stats[IO_PART_INDEX_MAX_NUM];
static struct io_blk_stats io_128k_stats[IO_PART_INDEX_MAX_NUM];
static struct io_blk_stats io_1024k_stats[IO_PART_INDEX_MAX_NUM];
static unsigned int all_read_cnt;
static unsigned int all_write_cnt;
static unsigned int ufs_tag_cnt_one;
static unsigned int ufs_tag_cnt_two;
static unsigned int ufs_tag_cnt_three;
static unsigned int ufs_tag_cnt_four;

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
#define IO_UNISTORE_DMD_CODE 914009001
static unsigned int io_unistore_stats[UNISTORE_TYPE_MAX];
#endif

static struct file *open_log_file(char *name)
{
	mm_segment_t oldfs;
	struct file *filp = NULL;

	if (name == NULL)
		return NULL;

	oldfs = get_fs();
	set_fs(get_ds());

	filp = filp_open(name, O_CREAT | O_RDWR, 0644);

	set_fs(oldfs);

	if (IS_ERR(filp)) {
		io_trace_print("failed to open %s: %ld \n", name, PTR_ERR(filp));
		return NULL;
	}

	return filp;
}

static int write_file(struct file *filp, struct iovec *vec)
{
	int ret;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(get_ds());
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
	ret = vfs_writev(filp, vec, 1, &filp->f_pos, 0);
#else
	ret = vfs_writev(filp, vec, 1, &filp->f_pos);
#endif
	set_fs(oldfs);
	if (unlikely(ret < 0))
		io_trace_print("failed to write\n");

	return ret;
}
/* lint -save -e732 -e501 -e747 -e712 -e838 -e730 */
static int write_io_monlog_kernel(struct file *filp, unsigned char *buf, int len)
{
	int ret = 0;
	struct iovec vec[1];
	mm_segment_t oldfs;

	if (filp == NULL || buf == NULL)
		return -1;

	if (IS_ERR(filp)) {
		io_trace_print("file descriptor to is invalid: %ld\n", PTR_ERR(filp));
		return -1;
	}

	vec[0].iov_base = buf;
	vec[0].iov_len  = len;

	oldfs = get_fs();
	set_fs(get_ds());

#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
	ret = vfs_writev(filp, vec, 1, &filp->f_pos, 0);
#else
	ret = vfs_writev(filp, vec, 1, &filp->f_pos);
#endif

	set_fs(oldfs);

	if (unlikely(ret < 0))
		io_trace_print("failed to write!\n");

	if (ret != len) {
		io_trace_print("want write %d, real write %d\n", len, ret);
		return -1;
	}

	return ret;
}

static void io_trace_clear_account(void)
{
	memset(io_4k_stats, 0, sizeof(io_4k_stats));
	memset(io_128k_stats, 0, sizeof(io_128k_stats));
	memset(io_1024k_stats, 0, sizeof(io_1024k_stats));
	all_read_cnt = 0;
	all_write_cnt = 0;
	ufs_tag_cnt_one = 0;
	ufs_tag_cnt_two = 0;
	ufs_tag_cnt_three = 0;
	ufs_tag_cnt_four = 0;
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	memset(io_unistore_stats, 0, sizeof(io_unistore_stats));
#endif
}

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
static unsigned int io_trace_unistore_stream(struct imonitor_eventobj *obj)
{
	unsigned int ret = 0;

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM0_TOTAL_WRITE_INT,
		io_unistore_stats[UNISTORE_STREAM0_TOTAL_WRITE]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM1_TOTAL_WRITE_INT,
		io_unistore_stats[UNISTORE_STREAM1_TOTAL_WRITE]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM2_TOTAL_WRITE_INT,
		io_unistore_stats[UNISTORE_STREAM2_TOTAL_WRITE]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM3_TOTAL_WRITE_INT,
		io_unistore_stats[UNISTORE_STREAM3_TOTAL_WRITE]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM4_TOTAL_WRITE_INT,
		io_unistore_stats[UNISTORE_STREAM4_TOTAL_WRITE]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM0_WRITE_DELAY_4K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM0_WRITE_DELAY_4K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM1_WRITE_DELAY_4K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM1_WRITE_DELAY_4K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM2_WRITE_DELAY_4K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM2_WRITE_DELAY_4K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM3_WRITE_DELAY_4K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM3_WRITE_DELAY_4K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM4_WRITE_DELAY_4K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM4_WRITE_DELAY_4K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM0_WRITE_DELAY_128K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM0_WRITE_DELAY_128K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM1_WRITE_DELAY_128K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM1_WRITE_DELAY_128K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM2_WRITE_DELAY_128K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM2_WRITE_DELAY_128K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM3_WRITE_DELAY_128K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM3_WRITE_DELAY_128K_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM4_WRITE_DELAY_128K_AVG_INT,
		io_unistore_stats[UNISTORE_STREAM4_WRITE_DELAY_128K_AVG]);

	return ret;
}

static unsigned int io_trace_unistore_interface(struct imonitor_eventobj *obj)
{
	unsigned int ret = 0;

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_PON_SYNC_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_PON_SYNC_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_PON_SYNC_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_PON_SYNC_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_GET_STREAM_OOB_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_GET_STREAM_OOB_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_GET_STREAM_OOB_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_GET_STREAM_OOB_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_RESET_FTL_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_RESET_FTL_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_RESET_FTL_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_RESET_FTL_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_DATA_MOVE_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_DATA_MOVE_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_DATA_MOVE_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_DATA_MOVE_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_SLC_MODE_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_SLC_MODE_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_SLC_MODE_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_SLC_MODE_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_FS_SYNC_DONE_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_FS_SYNC_DONE_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_FS_SYNC_DONE_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_FS_SYNC_DONE_FAIL_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_SYNC_READ_VERIFY_SEND_CNT_INT,
		io_unistore_stats[UNISTORE_SYNC_READ_VERIFY_SEND_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_SYNC_READ_VERIFY_FAIL_CNT_INT,
		io_unistore_stats[UNISTORE_SYNC_READ_VERIFY_FAIL_CNT]);

	return ret;
}

static unsigned int io_trace_unistore_account(struct imonitor_eventobj *obj)
{
	unsigned int ret = 0;

	ret |= io_trace_unistore_stream(obj);

	ret |= io_trace_unistore_interface(obj);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_FSYNC_INC_ORDER_CNT_INT,
		io_unistore_stats[UNISTORE_FSYNC_INC_ORDER_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_STREAM_INC_ORDER_CNT_INT,
		io_unistore_stats[UNISTORE_STREAM_INC_ORDER_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_RPMB_INC_ORDER_CNT_INT,
		io_unistore_stats[UNISTORE_RPMB_INC_ORDER_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_EXP_LBA_SECOND_MATCH_CNT_INT,
		io_unistore_stats[UNISTORE_EXP_LBA_SECOND_MATCH_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_EXP_LBA_SECOND_MATCH_DELAY_AVG_INT,
		io_unistore_stats[UNISTORE_EXP_LBA_SECOND_MATCH_DELAY_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_REQUEUE_SEARCH_CNT_AVG_INT,
		io_unistore_stats[UNISTORE_REQUEUE_SEARCH_CNT_AVG]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_SPLIT_OVER_SECTION_CNT_INT,
		io_unistore_stats[UNISTORE_SPLIT_OVER_SECTION_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_BKOPS_FS_QUERY_CNT_INT,
		io_unistore_stats[UNISTORE_BKOPS_FS_QUERY_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_BKOPS_FS_START_CNT_INT,
		io_unistore_stats[UNISTORE_BKOPS_FS_START_CNT]);

	ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
		V914009001_BKOPS_FS_STOP_CNT_INT,
		io_unistore_stats[UNISTORE_BKOPS_FS_STOP_CNT]);

	return ret;
}

static void io_trace_unistore_count_upload(void)
{
	unsigned int ret = 0;
	struct imonitor_eventobj *obj = NULL;

	obj = imonitor_create_eventobj(IO_UNISTORE_DMD_CODE);
	if (!obj) {
		io_trace_print("obj create failed! %d \n", IO_UNISTORE_DMD_CODE);
		return;
	}

	ret |= io_trace_unistore_account(obj);

	if (ret) {
		imonitor_destroy_eventobj(obj);
		io_trace_clear_account();
		return;
	} else {
		imonitor_send_event(obj);
		io_trace_print(
			"%d, upload unistore count succeed!\n",
			IO_UNISTORE_DMD_CODE);
	}

	imonitor_destroy_eventobj(obj);
}
#endif

static int io_trace_count_upload(void)
{
	unsigned int ret = 0;
	unsigned int vol_index;
	struct io_blk_stats *io_stats_element = NULL;
	struct imonitor_eventobj *obj = NULL;

	/*  DMD upload io delay  */
	for (vol_index = IO_PART_USER_INDEX; vol_index < IO_PART_INDEX_MAX_NUM;
		vol_index++) {
		obj = imonitor_create_eventobj(UFS_DELAY_DMD_CODE);
		if (!obj) {
			io_trace_print("obj create failed! %d vol_index=%d\n",
				UFS_DELAY_DMD_CODE, vol_index);
			return -1;
		}
		ret |= (unsigned int)imonitor_set_param_string_v2(obj,
			V914009000_PARTITION_INT, partitions_name[vol_index]);

		io_stats_element = &io_4k_stats[vol_index];
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_4K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_4K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_4K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_4K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_4K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_4K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_4K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_4K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_WRITE]);

		io_stats_element = &io_128k_stats[vol_index];
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_128K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_128K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_128K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_128K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_128K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_128K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_128K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_128K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_WRITE]);

		io_stats_element = &io_1024k_stats[vol_index];
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_1024K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_1024K_FIVE_INT,
			io_stats_element->io_cnt[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_1024K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_1024K_TWO_INT,
			io_stats_element->io_latency_max[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_1024K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_1024K_ONE_INT,
			io_stats_element->io_latency_min[IO_OP_WRITE]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_DELAY_1024K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_READ]);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_DELAY_1024K_AVERAGE_INT,
			io_stats_element->io_latency_avg[IO_OP_WRITE]);

		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_READ_RUN_IO_NUM_INT, all_read_cnt);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_WRITE_RUN_IO_NUM_INT, all_write_cnt);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_STROAGE_MANFID_INT, dm_ufs_manfid);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_UFS_QUEUE_TAG_ONE_INT, ufs_tag_cnt_one);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_UFS_QUEUE_TAG_TWO_INT, ufs_tag_cnt_two);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_UFS_QUEUE_TAG_THREE_INT, ufs_tag_cnt_three);
		ret |= (unsigned int)imonitor_set_param_integer_v2(obj,
			V914009000_UFS_QUEUE_TAG_FOUR_INT, ufs_tag_cnt_four);

		if (ret) {
			imonitor_destroy_eventobj(obj);
			io_trace_clear_account();
			return -1;
		} else {
			imonitor_send_event(obj);
			io_trace_print(
				"%d, upload iocount and io delay succeed!\n",
				UFS_DELAY_DMD_CODE);
		}

		imonitor_destroy_eventobj(obj);
	}

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	io_trace_unistore_count_upload();
#endif
	io_trace_clear_account();

	return 0;
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#ifdef HI18VV_FSR_GET_THREAD
static int io_trace_fsr_upload(void)
{
	struct imonitor_eventobj *obj = imonitor_create_eventobj(UFS_HI1861_FSR_DMD_CODE);

	if (!obj) {
		io_trace_print("obj_fsr create failed!\n");
		return -1;
	}

	io_trace_print("obj_fsr upload ing \n");

	imonitor_send_event(obj);

	imonitor_destroy_eventobj(obj);

	return 0;
}
#endif
#endif

static struct io_trace_interface *my_itface;
/* lint restore */
static int io_copy_interface_data(struct io_trace_interface *in_itface,
		struct io_trace_interface *out_itface)
{
	int i;
	int len = -1;

	in_itface->magic = out_itface->magic;
	in_itface->buff_len = out_itface->buff_len;
	in_itface->circum = out_itface->circum;
	in_itface->pid_num = out_itface->pid_num;
	in_itface->time_stamp = out_itface->time_stamp;
	in_itface->time_span = out_itface->time_span;

	if (in_itface->buff_len > IO_MAX_BUF_ENTRY) {
		io_trace_print("in_itface buff_len: %d\n", in_itface->buff_len);
		return len;
	}

	len = sizeof(struct io_trace_interface);

	if (in_itface->pid_num > 0) {
		for (i = 0; i < in_itface->pid_num; i++) {
			len += sizeof(pid_t);
			if (len >= IO_MAX_BUF_ENTRY) {
				io_trace_print("limit buffer, real pid num: %d\n", i);
				in_itface->pid_num = i;
				len = len - sizeof(pid_t);
				break;
			}
			in_itface->pid[i] = out_itface->pid[i];
			io_trace_print("recv pid[%d]: %d\n", i, in_itface->pid[i]);
		}
	} else {
		io_trace_print("print all the process!\n");
	}

	return len;
}

static inline int io_log_allow_pid(unsigned int pid_num, pid_t *o_pid, pid_t pid)
{
	unsigned int i;
	if (pid_num == 0)
		return 1;

	for (i = 0; i < pid_num; i++) {
		if (o_pid[i] == pid)
			return 1;
	}

	return 0;
}

static inline int io_log_allow_time(unsigned long time, unsigned long o_time, unsigned long time_span)
{
	if (o_time == 0 || time_span == 0)
		return 1;

	if (o_time > io_ns_to_ms(time)) {
		if ((o_time - io_ns_to_ms(time)) < time_span)
			return 1;
		else
			return 0;
	} else {
		return 2;
	}
}

static inline int io_log_allow_action(unsigned int action)
{
	/* only the vfs log */
	if (action <= IO_TRACE_JANK_TAG)
		return 1;

	return 0;
}

static struct io_trace_log_entry *io_trace_find_log(struct io_trace_interface *itface,
		struct io_trace_ctrl *trace_ctrl)
{
	unsigned int index = trace_ctrl->out_log_index;
	unsigned int find_flag = 0;
	unsigned int total_count = io_trace_this->all_log_count;
	int ret = 0;
	struct io_trace_log_entry *entry = NULL;
	while (trace_ctrl->out_log_count < total_count) {
		trace_ctrl->out_log_count++;
		entry = (((struct io_trace_log_entry *)(trace_ctrl->mem_mngt->all_trace_buff)) +
				index);
		/* next find log index */
		index = (index == 0) ? (total_count - 1) : (index - 1);
		ret = io_log_allow_time(entry->time, itface->time_stamp, itface->time_span);
		if (!ret)
			return NULL;

		if (ret == 2)
			continue;

		if (io_log_allow_pid(itface->pid_num, itface->pid, entry->tgid) &&
				io_log_allow_action(entry->action)) {
			trace_ctrl->out_log_index = index;
			find_flag = 1;
			break;
		}
	}
	if (find_flag == 1)
		return entry;
	else
		/* can't find */
		return NULL;
}

static ssize_t io_log_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i = 0;
	int j = 0;
	int ret = -1;
	int ret_e = 0;
	unsigned int out_count = 0;
	struct io_trace_interface *itface = my_itface;
	char file_name[48];
	unsigned int data_len = 0;
	unsigned int head_len = 0;
	char *head_buf = NULL;
	unsigned int r_max_delay = 0;
	unsigned int r_total_delay = 0;
	unsigned int w_max_delay = 0;
	unsigned int w_total_delay = 0;
	unsigned int r_cnt = 0;
	unsigned int w_cnt = 0;
	unsigned int count = 0;

	struct io_trace_log_entry *blk_entry = NULL;

	mutex_lock(&output_mutex);
	if (!io_trace_this->enable) {
		io_trace_print("io trace enable failed!\n");
		mutex_unlock(&output_mutex);
		return ret;
	}

	io_trace_this->enable = 0;

	if (!io_trace_this->out_valid) {
		io_trace_print("out valid error: %d\n", io_trace_this->out_valid);
		goto end;
	}

	if (!itface) {
		io_trace_print("itface is NULL\n");
		goto end;
	}

	if (buf == NULL) {
		io_trace_print("itface buff NULL\n");
		goto end;
	}
	if (itface->buff_len == 0 || itface->buff_len > 4096) {
		io_trace_print("itface buff_len error: %d\n", itface->buff_len);
		goto end;
	}

	sprintf(file_name, "/data/log/jank/iotrace/%s", "iotrace.txt");
	io_trace_this->filp = open_log_file(file_name);
	if (io_trace_this->filp == NULL) {
		io_trace_print("io_trace open file %s failed!\n", file_name);
		goto end;
	}

	/* end */
	data_len = itface->buff_len;
	head_len = sizeof(struct io_trace_output_head); /* reserved how many bytes */
	/* skip the head buffer */
	head_buf = buf;
	data_len -= head_len;
	buf += head_len;
	/* return value */
	ret = head_len;

	out_count = data_len / (sizeof(struct io_trace_output_entry));

	io_trace_this->out_log_index = (io_trace_this->all_log_index == 0) ? (io_trace_this->all_log_count - 1) :
		(io_trace_this->all_log_index - 1);

	io_trace_this->out_log_count = 0;

	for (i = 0; i < out_count; i++) {
		struct io_trace_output_entry *out_entry = NULL;
		struct io_trace_log_entry *log_entry;

		out_entry = ((struct io_trace_output_entry *)buf) + i;
		log_entry = io_trace_find_log(itface, io_trace_this);
		if (log_entry == NULL) {
			io_trace_print("no such log entry output!,ret:%d\n", ret);
			goto log_end;
		}
		out_entry->action = log_entry->action;
		out_entry->pid = log_entry->tgid;
		out_entry->inode = log_entry->inode;
		out_entry->rw = log_entry->rw;
		out_entry->time_stamp = log_entry->time;
		out_entry->delay = io_ns_to_ms(log_entry->delay);
		memcpy(out_entry->comm, log_entry->comm, IO_P_NAME_LEN);

		if (action_is_read(log_entry->action)) {
			r_max_delay = (r_max_delay >= log_entry->delay) ? r_max_delay :
				log_entry->delay;
			r_total_delay += log_entry->delay;
			r_cnt++;
		}

		if (action_is_write(log_entry->action)) {
			w_max_delay = (w_max_delay >= log_entry->delay) ? w_max_delay :
				log_entry->delay;
			w_total_delay += log_entry->delay;
			w_cnt++;
		}

		ret += sizeof(struct io_trace_output_entry);
	}

log_end:
	/* store all 4mb buff log entry */
	/* force dump iotrace every jankexc */
	/* lint -save -e713 -e574 -e737 -e747 */
	if (io_trace_this->all_log_count > 0) {
/* get hi1861 fsr log when blocked */
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
		if (ufs_manufacture == UFS_VENDOR_HI1861) {
			ret_e = io_fsr_log_get(io_trace_this->filp);

			if (ret_e < 0)
				io_trace_print("fsr write log kernel failed:[%d]\n", ret_e);
		}
#endif

		count = io_trace_this->all_log_count;

		for (i = io_trace_this->all_log_index, j = 0; j < count; j++) {
			i = (i == 0) ? (count - 1) : (i - 1);
			io_trace_assert(i >= 0);
			blk_entry = (((struct io_trace_log_entry *)
					(io_trace_this->mem_mngt->all_trace_buff)) + i);
			ret_e = write_io_monlog_kernel(io_trace_this->filp, (unsigned char *)blk_entry, sizeof(struct io_trace_log_entry));
			if (ret_e < 0) {
				io_trace_print("write log kernel failed:[%d]\n", ret_e);
				/*  FREE BUFF release file & unlock mutex when err happen  */
				io_trace_this->out_valid = 0;
				if (my_itface) {
					io_mem_mngt_free(io_trace_this->mem_mngt, (unsigned char *)my_itface);
					my_itface = NULL;
				}

				if (io_trace_this->filp)	filp_close(io_trace_this->filp, NULL);

				mutex_unlock(&output_mutex);
				return -1;
			}
		}
	}

	/* lint restore */
	if (head_buf) {
		struct io_trace_output_head *head = (struct io_trace_output_head *)head_buf;
		memset(head_buf, 0, sizeof(struct io_trace_output_head));
		io_trace_version(head->version);
		if (r_cnt > 0) {
			head->r_max_delay = io_ns_to_ms(r_max_delay);
			head->r_aver_delay = io_ns_to_ms(r_total_delay / r_cnt);
			head->r_total_delay = io_ns_to_ms(r_total_delay);
		}
		if (w_cnt > 0) {
			head->w_max_delay = io_ns_to_ms(w_max_delay);
			head->w_aver_delay = io_ns_to_ms(w_total_delay / w_cnt);
			head->w_total_delay =  io_ns_to_ms(w_total_delay);
		}
		/* used for data check */
		head->data_len = (unsigned int)ret;
	}

end:
	io_trace_this->out_valid = 0;
	if (my_itface) {
		io_mem_mngt_free(io_trace_this->mem_mngt, (unsigned char *)my_itface);
		my_itface = NULL;
	}

	if (io_trace_this->filp)
		filp_close(io_trace_this->filp, NULL);

	io_trace_this->enable = 1;
	mutex_unlock(&output_mutex);

	return ret;
}

static ssize_t io_log_data_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct io_trace_interface *out_itface = (struct io_trace_interface *)buf;
	int ret = -1;

	mutex_lock(&output_mutex);
	if (!io_trace_this->enable) {
		io_trace_print("io trace enable failed!\n");
		mutex_unlock(&output_mutex);
		return ret;
	}

	if (out_itface->magic == 0x5A || out_itface->magic == 0x5B) {
		unsigned char *mem = io_mem_mngt_alloc(io_trace_this->mem_mngt);
		if (mem == NULL) {
			io_trace_print("mem alloc failed!\n");
			goto end;
		}
		my_itface = (struct io_trace_interface *)mem;
		ret = io_copy_interface_data(my_itface, out_itface);
		if (ret < 0) {
			io_mem_mngt_free(io_trace_this->mem_mngt, mem);
			goto end;
		}

		io_trace_this->out_valid = 1;
	} else {
		io_trace_print("io trace magic error: 0x%x\n", out_itface->magic);
	}

end:
	mutex_unlock(&output_mutex);

	return ret;
}


static ssize_t sysfs_io_trace_attr_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	int ret_e = 0;
	ssize_t ret = 0;
	unsigned long log_time, log_time_end;

	if (attr == &g_kobjAttrEnable)
		ret = sprintf(buf, "%d\n", io_trace_this->enable);

	if (attr == &g_kobjAttrPid) {
		int i, j;

		struct io_trace_log_entry *entry = NULL;
		char file_name[128];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
		struct timespec64 realts;
#else
		struct timespec realts;
#endif

		do_posix_clock_monotonic_gettime(&realts);
		log_time = (unsigned long)realts.tv_sec * 1000000000 + realts.tv_nsec;

		io_trace_this->enable = 0;
		sprintf(file_name, "/sdcard/log_%s", "end");
		io_trace_this->filp = open_log_file(file_name);
		if (io_trace_this->filp == NULL) {
			io_trace_print("io_trace open file %s failed!\n", file_name);
			io_trace_this->enable = 1;
			return ret;
		}
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
		if (ufs_manufacture == UFS_VENDOR_HI1861) {
			ret_e = io_fsr_log_get(io_trace_this->filp);

			if (ret_e < 0)
				io_trace_print("fsr write log kernel failed:[%d]\n", ret_e);
		}
#endif
		write_head_kernel(io_trace_this->filp);
		if (io_trace_this->all_log_count > 0) {
			unsigned int count = io_trace_this->all_log_count;

			for (i = io_trace_this->all_log_index, j = 0; j < count; j++) {
				i = (i == 0) ? (count - 1) : (i - 1);
				io_trace_assert(i >= 0);
				entry = (((struct io_trace_log_entry *)
						(io_trace_this->mem_mngt->all_trace_buff)) + i);
				write_log_kernel(io_trace_this->filp, entry);
			}
		}

		do_posix_clock_monotonic_gettime(&realts);
		/* lint -save -e747 */
		log_time_end = (unsigned long)realts.tv_sec * 1000000000 + realts.tv_nsec;
		/* lint restore */
		ret = sprintf(buf, "%ld\n", log_time_end - log_time);

		io_trace_this->enable = 1;
	}
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	if (attr == &g_kobjAttrFsr) {
		io_trace_this->enable = 0;
		ret = iotrace_ufs_fsr_get();

		if (ret >= 0) {
			ret = sprintf(buf, "Succeed\n");
		} else {
			ret = sprintf(buf, "Failed\n");
			return -1;
		}
		io_trace_this->enable = 1;
	}

#endif
	return ret;
}
static ssize_t sysfs_io_trace_attr_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int value;
	int ret = 0;

	if (attr == &g_kobjAttrEnable) {
		mutex_lock(&output_mutex);
		sscanf(buf, "%u", &value);
		if (value == 1) {
			if (io_trace_this->first == 0) {
				ret = trace_mm_init(io_trace_this);
				if (ret < 0) {
					io_trace_print("trace mm init failed!\n");
					io_trace_this->enable = 0;
					mutex_unlock(&output_mutex);
					return count;
				}
				io_trace_setup(io_trace_this);
				io_trace_this->first = 1;
			}
			io_trace_this->enable = value;
		}
		if (value == 0)
			io_trace_this->enable = value;

		mutex_unlock(&output_mutex);
	}

	if (attr == &g_kobjAttrPid) {
		unsigned char *mem = io_mem_mngt_alloc(io_trace_this->mem_mngt);
		if (mem == NULL) {
			io_trace_print("mem alloc failed!\n");
			return count;
		}
		my_itface = (struct io_trace_interface *)mem;
		my_itface->buff_len = 4096;
		my_itface->circum = 0;
		my_itface->time_stamp = 0;
		my_itface->pid_num = 0;
		if (value) {
			my_itface->pid_num = 1;
			my_itface->pid[0] = value;
		}
		io_trace_print("out pid_num:%d, buff_len:%d, time: %ld\n",
				my_itface->pid_num, my_itface->buff_len, my_itface->time_stamp);
		io_trace_this->out_valid = 1;
		io_trace_this->out_test = value;
	}
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	if (attr == &g_kobjAttrFsr)
		io_trace_print("load hi1861 Fsr log\n");

#endif
	return count;
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
extern int ufshcd_get_fsr_command(struct ufs_hba *hba, unsigned char *buf, unsigned int size);
static int iotrace_ufs_fsr_get(void)
{
	struct hufs_host *host_kirin = NULL;
	static struct file *fsr_filp = NULL;
	int ret = 0;
	char file_name[128];

	if (!fsr_hba) {
		io_trace_print("%s shost_priv host failed\n", __func__);
		return -1;
	}

	if ((fsr_hba->manufacturer_id == 0) || (fsr_hba->manufacturer_id != UFS_VENDOR_HI1861)) {
		io_trace_print("fsr is only for hi1861!\n");
		return -1;
	}

	host_kirin = fsr_hba->priv;

	if (host_kirin->in_suspend) {
		io_trace_print("hiVV device is in suspend!\n");
		return -1;
	}

	sprintf(file_name, "/sdcard/log_%s", "fsr");
	fsr_filp = open_log_file(file_name);

	if (fsr_filp == NULL) {
		io_trace_print("io_trace open file %s failed!\n", file_name);
		return 0;
	}

	ret = io_fsr_log_get(fsr_filp);

	if (ret < 0)
		io_trace_print("store fsr log kernel failed !\n");

	if (fsr_filp)
		filp_close(fsr_filp, NULL);

	return ret;
}

static int io_fsr_log_get(struct file *filp)
{
	struct hufs_host *host_kirin = NULL;
	unsigned char *mem = NULL;
	int ret = 0;

	if (!fsr_hba) {
		io_trace_print("%s shost_priv host failed\n", __func__);
		return -1;
	}

	if ((fsr_hba->manufacturer_id == 0) || (fsr_hba->manufacturer_id != UFS_VENDOR_HI1861)) {
		io_trace_print("fsr is only for hi1861!\n");
		return -1;
	}

	host_kirin = fsr_hba->priv;

	if (host_kirin->in_suspend) {
		io_trace_print("hiVV is in suspend!\n");
		return -1;
	}

	mem = io_mem_mngt_alloc(io_trace_this->mem_mngt);

	if (filp == NULL) {
		io_trace_print("io_trace open file failed!\n");
		ret = -1;
		goto end;
	}

	if (mem == NULL) {
		io_trace_print("fsr mem alloc failed !\n");
		ret = -1;
		goto end;
	}

	ret = ufshcd_get_fsr_command(fsr_hba, mem, IO_MAX_BUF_ENTRY);

	if (ret < 0) {
		io_trace_print("get fsr log kernel failed !\n");
		goto end;
	}

	ret = write_io_monlog_kernel(filp, (unsigned char *)mem, IO_MAX_BUF_ENTRY);
	if (ret < 0) {
		io_trace_print("write fsr log kernel failed:[%d]\n", ret);
		goto end;
	}

end:
	if (mem) {
		io_mem_mngt_free(io_trace_this->mem_mngt, (unsigned char *)mem);
		mem = NULL;
	}
	return ret;
}
#endif

static int write_head_kernel(struct file *filp)
{
	struct iovec vec[1];
	unsigned char buf[164];
	int ret;

	unsigned len;

	len = sprintf(buf, "\n%-5s%-11s%-11s%-21s%-20s%-14s%-10s%-15s%-15s%-8s%-15s%-15s\n", "CPU", "TGID",
			"PID", "PName", "Time", "Inode/Sector", "Nr_Types", "Action", "Delay", "RW", "F_NAME", "F_TIME");

	vec[0].iov_base = buf;
	vec[0].iov_len  = len;

	ret = write_file(filp, &vec[0]);

	return ret;
}


/* lint restore */
static void io_entry_rw(unsigned char rw, char *buf)
{
	unsigned int len = 0;
	len += sprintf(buf + len, "%s", rw_name[rw & IO_TRACE_WRITE]);
	if (rw & IO_TRACE_SYNC)
		len += sprintf(buf + len, "%s", rw_name[__IO_TRACE_SYNC]);

	if (rw & IO_TRACE_META)
		len += sprintf(buf + len, "%s", rw_name[__IO_TRACE_META]);

	if (rw & IO_TRACE_DISCARD)
		len += sprintf(buf + len, "%s", rw_name[__IO_TRACE_DISCARD]);

	return;
}

static int write_log_kernel(struct file *filp, struct io_trace_log_entry *entry)
{
	struct iovec vec[1];
	int ret;
	char rw_type[32];
	unsigned char buf[200];
	unsigned len;

	if (IS_ERR(filp)) {
		io_trace_print("file descriptor to is invalid: %ld\n", PTR_ERR(filp));
		return -1;
	}
	// format: <priority:1><message:N>\0
	io_entry_rw(entry->rw, rw_type);
	/* lint -save -e705 */
	if (entry->action != SCSI_BLK_CMD_RW_END_TAG) {
		len = sprintf(buf, "%-5d&%-10d&%-10d&%-20s&%-20ld&%-12ld&%-10d&%-12s&%-15ld&%-5s&%-10s&%-10ld\n", entry->cpu,
				entry->tgid, entry->pid, entry->comm, entry->time, entry->inode, entry->nr_bytes,
				all_trace_action[entry->action].name, entry->delay,
				rw_type, entry->f_name, entry->f_time);
	} else {
		 len = sprintf(buf, "%-5d&%-10d&%-10d&%-20s&%-20ld&%-12ld&%-10d&%-12s&0x%-13lx&%-5s&%-10s&%-10ld\n", entry->cpu,
				entry->tgid, entry->pid, entry->comm, entry->time, entry->inode, entry->nr_bytes,
				all_trace_action[entry->action].name, entry->delay,
				rw_type, entry->f_name, entry->f_time);
	}
	/* lint restore */
	vec[0].iov_base = buf;
	vec[0].iov_len  = len;

	ret = write_file(filp, &vec[0]);

	return ret;
}

static struct kobject *kobject_ts;
static int io_trace_probe(struct platform_device *pdev)
{
	int ret;
	kobject_ts = kobject_create_and_add("io_log_data", NULL);
	if (!kobject_ts) {
		io_trace_print("create object failed!\n");
		return -1;
	}

	ret = sysfs_create_group(kobject_ts, &io_trace_attr_group);
	if (ret) {
		kobject_put(kobject_ts);
		io_trace_print("create group failed!\n");
		return -1;
	}
	return ret;
}

static struct platform_driver io_trace_driver = {
	.probe = io_trace_probe,
	.driver = {
		.owner = THIS_MODULE,
		.name = "io_trace",
	},
};

static struct io_trace_log_entry *io_get_pid_buf(struct io_trace_pid *trace_pid,
		unsigned int action)
{
	struct io_trace_log_entry *ret_trace = NULL;

	if (action > IO_TRACE_PARENT_END)
		return NULL;

	ret_trace = &trace_pid->pid_entry[action];

	return ret_trace;
}

static struct io_trace_log_entry *io_get_all_buf(struct io_trace_ctrl *trace_ctrl)
{
	struct io_trace_log_entry *ret_trace = NULL;
	unsigned long flags;

	/* must be irq save lock, because mmc end is done in irq context! */
	spin_lock_irqsave(&global_spinlock, flags);

	ret_trace = ((struct io_trace_log_entry *)(trace_ctrl->mem_mngt->all_trace_buff) +
			trace_ctrl->all_log_index);

	trace_ctrl->all_log_index++;

	if (trace_ctrl->all_log_index >= IO_MAX_LOG_PER_GLOBAL)
		trace_ctrl->all_log_index = 0;

	/* point to next valid entry */

	if (trace_ctrl->all_log_count < IO_MAX_LOG_PER_GLOBAL)
		trace_ctrl->all_log_count++;

	spin_unlock_irqrestore(&global_spinlock, flags);

	return ret_trace;
}

struct io_trace_tag_attr {
	unsigned int curr_attr;
	unsigned int parent;
	unsigned int rw_flags;
	unsigned int fname_flag;
	unsigned int nr_bytes;
	char f_name[IO_F_NAME_LEN];

	unsigned long (*func)(struct io_trace_pid *trace_pid, unsigned int action,
		unsigned int parent, unsigned long log_time, unsigned long inode); /* handler from the begin tag to end */
	void (*handler)(struct io_trace_pid *trace_pid, unsigned int action,
			unsigned int parent, unsigned long inode);
	int (*func_get_fname)(struct io_trace_tag_attr *tag, struct inode *inode);
};

static unsigned int io_trace_find_index(struct io_trace_log_entry *begin_entry,
		unsigned int count_all, unsigned int log_index, unsigned int log_count, unsigned int action,
		unsigned int parent, unsigned long inode, unsigned int nr_bytes)
{
	unsigned int index;
	struct io_trace_log_entry *entry = NULL;
	unsigned int count = (count_all >= 5000) ? 5000 : count_all;
	io_trace_assert(action > parent);

	index = log_index;
	while (log_count > 0) {
		index = (index == 0) ? (count_all - 1) : (index - 1);
		entry = (((struct io_trace_log_entry *)(begin_entry)) + index);
		if (entry != NULL) {
			if ((entry->action == parent) && (inode == entry->inode) && (entry->nr_bytes == nr_bytes)) {
				break;
			} else if (entry->action == parent) {
				log_count--;
			}
		} else {
			break;
		}
		/* add for when can't find start on entry */
		count--;
		if (count <= 0)
			break;
	}

	if ((!log_count) || (count <= 0))
		return (unsigned int)-1;

	return index;
}

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
static void calc_approximate_avg(
	unsigned int* value, unsigned int add_value)
{
	if (!(*value))
		*value = add_value;
	else
		/* average of 4 data stream */
		*value = (*value + add_value) >> 2;
}

static void io_trace_unistore_count_io(
	unsigned int nr_bytes, unsigned long delay, unsigned char stream_type)
{
	if (!io_trace_this)
		return;

	if (!io_trace_this->enable)
		return;

	io_unistore_stats[UNISTORE_STREAM0_TOTAL_WRITE + stream_type]++;
	if (nr_bytes == (4 * KB)) /* completed io length = 4KB */
		calc_approximate_avg(&(
		io_unistore_stats[UNISTORE_STREAM0_WRITE_DELAY_4K_AVG +
		stream_type]), delay);
	else if (nr_bytes == (128 * KB)) /* completed io length = 128KB */
		calc_approximate_avg(&(
		io_unistore_stats[UNISTORE_STREAM0_WRITE_DELAY_128K_AVG +
		stream_type]), delay);
}

void io_trace_unistore_count(enum unistore_account_type type, unsigned int count)
{
	if (!io_trace_this)
		return;

	if (!io_trace_this->enable)
		return;

	if (type == UNISTORE_REQUEUE_SEARCH_CNT_AVG ||
		type == UNISTORE_EXP_LBA_SECOND_MATCH_DELAY_AVG)
		calc_approximate_avg(&(io_unistore_stats[type]), count);
	else
		io_unistore_stats[type] += count;
}
#endif

static unsigned long io_trace_direct_delay(struct io_trace_pid *trace_pid,
		unsigned int action, unsigned int parent, unsigned long log_time,
		unsigned long inode)
{
	unsigned long ret_delay;
	struct io_trace_log_entry *ret_trace = NULL;

	io_trace_assert(action > parent);
	ret_trace = &trace_pid->pid_entry[parent];
	/* normal not happen, but only the enable and disable pair */
	if (ret_trace->valid == 0)
		return 0;

	if (ret_trace->inode != inode)
		return (unsigned long)-1;

	ret_trace->valid = 0;
	ret_delay = log_time - ret_trace->time;

	return ret_delay;
}

static int io_trace_calu_ufs_tag_set_cnt(unsigned long lrb_tag)
{
	unsigned int tag_cnt = 0;

	while (lrb_tag != 0) {
		if (lrb_tag & 0x1)
			tag_cnt++;

		lrb_tag = lrb_tag >> 1;
	}
	return tag_cnt;
}

static void io_trace_ufs_tag_cnt(unsigned long delay)
{
	unsigned int tag_cnt = 0;
	tag_cnt =  io_trace_calu_ufs_tag_set_cnt(delay);
	if ((tag_cnt >= 0) && (tag_cnt < 7)) {
		ufs_tag_cnt_one++;
	} else if ((tag_cnt >= 8) && (tag_cnt < 15)) {
		ufs_tag_cnt_two++;
	} else if ((tag_cnt >= 16) && (tag_cnt < 23)) {
		ufs_tag_cnt_three++;
	} else {
		ufs_tag_cnt_four++;
	}
}

static unsigned int io_find_partition_volume_index(unsigned char *volname)
{
	unsigned int i;

	if (volname == NULL)
		return IO_PART_OTHER_INDEX;

	for (i = 0; partitions_volume_index[i].volume; i++) {
		if (!strncmp(volname, partitions_volume_index[i].volume,
			strlen(partitions_volume_index[i].volume) + 1))
			return partitions_volume_index[i].index;
	}

	return IO_PART_OTHER_INDEX;
}

static void io_trace_blk_do_account_io_completion(struct io_blk_stats *io_stats_element,
	unsigned int rw, unsigned long delay)
{
	if ((io_stats_element->io_latency_max[rw] == 0) &&
		(io_stats_element->io_latency_min[rw] == 0) &&
		(io_stats_element->io_latency_avg[rw] == 0)) {
		io_stats_element->io_latency_max[rw] = delay;
		io_stats_element->io_latency_min[rw] = delay;
		io_stats_element->io_latency_avg[rw] = delay;
	} else {
		if (delay > io_stats_element->io_latency_max[rw])
			io_stats_element->io_latency_max[rw] = delay;

		if (delay < io_stats_element->io_latency_min[rw])
			io_stats_element->io_latency_min[rw] = delay;

		io_stats_element->io_latency_avg[rw] =
			(io_stats_element->io_latency_avg[rw] + delay) >> 1;
	}
	io_stats_element->io_cnt[rw]++;
}

static void io_trace_blk_account_io_completion(unsigned int nr_bytes,
	unsigned long delay, unsigned int rw_flags, unsigned char *volname)
{
	unsigned int vol_index;
	unsigned int rw;
	struct io_blk_stats *io_stats_element = NULL;

	vol_index = io_find_partition_volume_index(volname);
	rw = rw_flags & IO_TRACE_WRITE;
	/*  rw_op 0 is read 1 is write  */
	if ((rw == 0) || (rw == 1)) {
		if (nr_bytes == (4 * KB))
			io_stats_element = &io_4k_stats[vol_index];
		else if (nr_bytes == (128 * KB))
			io_stats_element = &io_128k_stats[vol_index];
		else if (nr_bytes == (1024 * KB))
			io_stats_element = &io_1024k_stats[vol_index];

		if (io_stats_element)
			io_trace_blk_do_account_io_completion(
				io_stats_element, rw, delay);
	}
}

static void io_trace_global_log(unsigned int action, unsigned long sector,
	unsigned int nr_bytes, unsigned int parent, unsigned long own_time,
	unsigned long delay, unsigned int rw_flags, const char *f_name,
	unsigned long ftrace_time, unsigned char *volname)
{
	struct io_trace_ctrl *trace_ctrl = io_trace_this;
	struct io_trace_log_entry *all_log_entry = NULL;
	struct task_struct *tsk = current;
	pid_t pid;
	pid_t tgid;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	struct timespec64 realts;
#else
	struct timespec realts;
#endif
	unsigned long log_time;
	unsigned long ft_time;

	pid = tsk->pid;
	tgid = tsk->tgid;
	io_trace_assert(pid < IO_MAX_PROCESS);
	/* dont handle max process */
	if (pid >= IO_MAX_PROCESS || tgid >= IO_MAX_PROCESS)
		return;

	log_time = own_time;
	if (!log_time) {
		do_posix_clock_monotonic_gettime(&realts);
		log_time = (unsigned long)realts.tv_sec * 1000000000 + realts.tv_nsec;
	}
	ft_time = ftrace_time;
	if (!ft_time) {
		ft_time = trace_clock_local();
		ft_time = (ft_time / 1000) + 1 + (((ft_time % 1000) / 100) >= 5 ? 1 : 0);
	}

	if (sector == ((unsigned long)-1))
		sector = (unsigned long)0;

	if (action == BLOCK_RQ_COMPLETE_TAG) {
		if ((rw_flags & IO_TRACE_WRITE) == 0)
			all_read_cnt++;
		else if ((rw_flags & IO_TRACE_WRITE) == 1)
			all_write_cnt++;
	}

	if (action == SCSI_BLK_CMD_RW_END_TAG)
		io_trace_ufs_tag_cnt(delay);

	if (parent && (sector != 0) && (action == BLOCK_RQ_COMPLETE_TAG) &&
		((nr_bytes == (4 * KB)) || (nr_bytes == (128 * KB)) ||
		(nr_bytes == (1024 * KB))) &&
		(io_trace_this->all_log_count >= IO_MAX_LOG_PER_GLOBAL)) {
		struct io_trace_log_entry *begin =
			(struct io_trace_log_entry *)(trace_ctrl->mem_mngt->all_trace_buff);
		unsigned int count = io_trace_this->all_log_count;
		struct io_trace_log_entry *index_entry = NULL;
		unsigned int index;

		index = io_trace_find_index(begin, count, trace_ctrl->all_log_index, 32, action, parent,
				sector, nr_bytes);

		if (index != (unsigned int)(-1)) {
			index_entry = (begin + index);
			delay = log_time - index_entry->time;
		} else {
			delay = 0xFFFFFFFF;
		}

		if (delay != 0xFFFFFFFF)
			io_trace_blk_account_io_completion(nr_bytes, delay,
				rw_flags, volname);
		else
			delay = 0;
	}

	all_log_entry = io_get_all_buf(trace_ctrl);
	all_log_entry->action = action;
	all_log_entry->rw = (unsigned char)rw_flags;
	all_log_entry->cpu = raw_smp_processor_id();
	all_log_entry->pid = pid;
	all_log_entry->tgid = tgid;
	all_log_entry->nr_bytes = nr_bytes;
	all_log_entry->inode = sector;
	all_log_entry->time = log_time;
	all_log_entry->f_time = ft_time;
	all_log_entry->delay = delay;
	memcpy(all_log_entry->comm, tsk->comm, IO_P_NAME_LEN);
	memset(all_log_entry->f_name, '\0', IO_F_NAME_LEN);

	if (f_name)
		memcpy(all_log_entry->f_name, f_name, IO_F_NAME_LEN);

	return;
}

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
static unsigned long io_trace_get_ft_time(unsigned long ftrace_time)
{
	unsigned long ft_time;

	ft_time = ftrace_time;
	if (!ft_time) {
		ft_time = trace_clock_local();
		ft_time = (ft_time / 1000) + 1 +
		(((ft_time % 1000) / 100) >= 5 ? 1 : 0); /* over half (500) */
	}

	return ft_time;
}

static void io_trace_all_cnt_update(unsigned int action,
	unsigned int rw_flags)
{
	if (action == BLOCK_RQ_COMPLETE_TAG) {
		if ((rw_flags & IO_TRACE_WRITE) == 0)
			all_read_cnt++;
		else if ((rw_flags & IO_TRACE_WRITE) == 1)
			all_write_cnt++;
	}
}

static unsigned long io_trace_get_delay(struct io_trace_ctrl *trace_ctrl,
	unsigned int action, unsigned int parent, unsigned long sector,
	unsigned int nr_bytes, unsigned long log_time)
{
	struct io_trace_log_entry *begin =
		(struct io_trace_log_entry *)(trace_ctrl->mem_mngt->all_trace_buff);
	unsigned int count = io_trace_this->all_log_count;
	struct io_trace_log_entry *index_entry = NULL;
	unsigned int index;
	unsigned long delay;

	index = io_trace_find_index(begin, count, trace_ctrl->all_log_index, 32,
			action, parent, sector, nr_bytes);

	if (index != (unsigned int)(-1)) {
		index_entry = (begin + index);
		delay = log_time - index_entry->time;
	} else {
		delay = 0xFFFFFFFF;
	}

	return delay;
}

static unsigned long io_trace_count_io_delay(
	unsigned long delay, unsigned int nr_bytes, unsigned int rw_flags,
	unsigned char *volname, unsigned char stream_type)
{
	if (delay != 0xFFFFFFFF) {
		io_trace_blk_account_io_completion(nr_bytes, delay,
			rw_flags, volname);
		if (rw_flags & IO_TRACE_WRITE)
			io_trace_unistore_count_io(nr_bytes, delay, stream_type);
	} else {
		delay = 0;
	}

	return delay;
}

static void io_trace_global_log_unistore(unsigned int action, unsigned long sector,
	unsigned int nr_bytes, unsigned int parent, unsigned long own_time,
	unsigned long delay, unsigned int rw_flags, const char *f_name,
	unsigned long ftrace_time, unsigned char *volname, unsigned char stream_type)
{
	struct io_trace_ctrl *trace_ctrl = io_trace_this;
	struct io_trace_log_entry *all_log_entry = NULL;
	struct task_struct *tsk = current;
	pid_t pid;
	pid_t tgid;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	struct timespec64 realts;
#else
	struct timespec realts;
#endif
	unsigned long log_time;
	unsigned long ft_time;

	pid = tsk->pid;
	tgid = tsk->tgid;

	io_trace_assert(pid < IO_MAX_PROCESS);

	/* dont handle max process */
	if (pid >= IO_MAX_PROCESS || tgid >= IO_MAX_PROCESS)
		return;

	log_time = own_time;
	if (!log_time) {
		do_posix_clock_monotonic_gettime(&realts);
		log_time = (unsigned long)realts.tv_sec * 1000000000 +
					realts.tv_nsec;
	}

	ft_time = io_trace_get_ft_time(ftrace_time);

	if (sector == ((unsigned long)-1))
		sector = (unsigned long)0;

	io_trace_all_cnt_update(action, rw_flags);

	if (action == SCSI_BLK_CMD_RW_END_TAG)
		io_trace_ufs_tag_cnt(delay);

	if (parent && (sector != 0) && (action == BLOCK_RQ_COMPLETE_TAG) &&
		((nr_bytes == (4 * KB)) || (nr_bytes == (128 * KB)) ||
		(nr_bytes == (1024 * KB))) &&
		(io_trace_this->all_log_count >= IO_MAX_LOG_PER_GLOBAL)) {
		delay = io_trace_get_delay(trace_ctrl, action,
				parent, sector, nr_bytes, log_time);
		delay = io_trace_count_io_delay(delay, nr_bytes,
				rw_flags, volname, stream_type);
	}

	all_log_entry = io_get_all_buf(trace_ctrl);
	all_log_entry->action = action;
	all_log_entry->rw = (unsigned char)rw_flags;
	all_log_entry->cpu = raw_smp_processor_id();
	all_log_entry->pid = pid;
	all_log_entry->tgid = tgid;
	all_log_entry->nr_bytes = nr_bytes;
	all_log_entry->inode = sector;
	all_log_entry->time = log_time;
	all_log_entry->f_time = ft_time;
	all_log_entry->delay = delay;
	memcpy(all_log_entry->comm, tsk->comm, IO_P_NAME_LEN);
	memset(all_log_entry->f_name, '\0', IO_F_NAME_LEN);

	if (f_name)
		memcpy(all_log_entry->f_name, f_name, IO_F_NAME_LEN);

	return;
}
#endif

static int io_trace_need_action(unsigned int action)
{
	/* only parent action */
	if ((action >= READ_BEGIN_TAG) &&
			(action <= IO_TRACE_PARENT_END))
		return 1;

	return 0;
}

static void io_trace_add_log(unsigned int action, unsigned long i_ino, struct inode *inode,
		struct io_trace_tag_attr *tag_attr)
{
	struct io_trace_ctrl *trace_ctrl = io_trace_this;
	struct task_struct *tsk = current;
	pid_t pid;
	pid_t tgid;
	struct io_trace_pid *trace_pid = NULL;
	struct io_trace_log_entry *log_entry = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	struct timespec64 realts;
#else
	struct timespec realts;
#endif
	unsigned long log_time, ft_time;
	unsigned long delay = 0;
	int skip = 0;
	unsigned int rw = tag_attr->rw_flags;
	int ret = 0;

	if (trace_ctrl == NULL) {
		io_trace_print("trace_ctrl is NULL!\n");
		return;
	}
	pid = tsk->pid;
	tgid = tsk->tgid;
	io_trace_assert(pid < IO_MAX_PROCESS);
	/* dont handle max process id */
	if (pid >= IO_MAX_PROCESS || tgid >= IO_MAX_PROCESS)
		return;

	trace_pid = trace_ctrl->all_trace[pid];
	if (trace_pid->running == 0) {
		trace_pid->running = 1;
		trace_pid->pid = pid;
	}

	trace_pid->log_count++;
	/* time based from boot */
	do_posix_clock_monotonic_gettime(&realts);
	log_time = (unsigned long)realts.tv_sec * 1000000000 + realts.tv_nsec;

	ft_time = trace_clock_local();
	ft_time = (ft_time / 1000) + 1 + (((ft_time % 1000) / 100) >= 5 ? 1 : 0);

	if (tag_attr->parent) {
		char *f_name = NULL;
		unsigned int nr_bytes = 0;
		if (tag_attr->func)
			delay = tag_attr->func(trace_pid, action, tag_attr->parent, log_time, i_ino);

		if (delay > IO_TRACE_LIMIT_LOG && (delay != ((unsigned long)-1))) {
			if (tag_attr->func_get_fname) {
				ret = tag_attr->func_get_fname(tag_attr, inode);
				if (ret)
					f_name = tag_attr->f_name;
			}

			if (tag_attr->nr_bytes)
				nr_bytes = tag_attr->nr_bytes;

			io_trace_global_log(action, i_ino, nr_bytes, 0,
				log_time, delay, rw, f_name, ft_time, NULL);
			skip = 1;
		}
	}

	if (io_trace_need_action(action) && (skip == 0)) {
		log_entry = io_get_pid_buf(trace_pid, action);
		if (log_entry == NULL)
			return;

		log_entry->action = action;
		log_entry->rw = (unsigned char)rw;
		log_entry->cpu = raw_smp_processor_id();
		log_entry->pid = pid;
		log_entry->tgid = tgid;
		log_entry->inode = i_ino;
		log_entry->time = log_time;
		log_entry->delay = delay;
		log_entry->f_time = ft_time;
		memcpy(log_entry->comm, tsk->comm, IO_P_NAME_LEN);

		log_entry->valid = 1;
	}

	return;
}

static int io_entry_fname(struct io_trace_tag_attr *tag, struct inode *inode)
{
	unsigned int len = (sizeof(tag->f_name) - 1);
	struct dentry *dir = NULL;

	if (inode == NULL)
		return 0;

	dir = d_find_alias(inode);
	if (dir == NULL)
		return 0;

	if (dir->d_name.name) {
		len = len > dir->d_name.len ? dir->d_name.len : len;
		memcpy(tag->f_name, dir->d_name.name, len);
		tag->f_name[len] = '\0';
		dput(dir);
		return 1;
	} else {
		memset(tag->f_name, '\0', IO_F_NAME_LEN);
		dput(dir);
	}

	return 0;
}

static void io_tag_attr_init(struct io_trace_tag_attr *attr)
{
	attr->curr_attr = 0;
	attr->parent = 0;
	attr->fname_flag = 0;
	attr->nr_bytes = 0;
	attr->func = NULL;
	attr->func_get_fname = NULL;

	return;
}
/* lint save -e747 -e712 -e715 -e732 -epn -eps -epu -epp */
static void io_generic_file_read_begin(void *ignore, struct file *filp, size_t count)
{
	struct inode *inode = filp->f_mapping->host;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_READ;

	if (io_trace_this->enable)
		io_trace_add_log(READ_BEGIN_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_generic_file_read_end(void *ignore, struct file *filp, size_t count)
{
	struct inode *inode = NULL;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.parent = READ_BEGIN_TAG;
	tag_attr.func = io_trace_direct_delay;
	tag_attr.rw_flags = IO_TRACE_READ;
	tag_attr.nr_bytes = count;
	tag_attr.func_get_fname = io_entry_fname;

	if (filp == NULL)
		return;

	inode = filp->f_mapping->host;
	if (inode == NULL)
		return;

	if (io_trace_this->enable)

		io_trace_add_log(READ_END_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_generic_perform_write_enter(void *ignore, struct file *filp,
		size_t count, loff_t pos)
{
	struct inode *inode = NULL;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (filp == NULL)
		return;

	inode = filp->f_mapping->host;
	if (inode == NULL)
		return;

	if (io_trace_this->enable)

		io_trace_add_log(WRITE_BEGIN_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_generic_perform_write_end(void *ignore, struct file *filp, size_t count)
{
	struct inode *inode = NULL;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.parent = WRITE_BEGIN_TAG;
	tag_attr.func = io_trace_direct_delay;
	tag_attr.rw_flags = IO_TRACE_WRITE;
	tag_attr.nr_bytes = count;
	tag_attr.func_get_fname = io_entry_fname;

	if (filp == NULL)
		return;

	inode = filp->f_mapping->host;
	if (inode == NULL)
		return;

	if (io_trace_this->enable)
		io_trace_add_log(WRITE_END_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_ext4_da_write_begin(void *ignore, struct inode *inode,
		loff_t pos, unsigned size, unsigned flags)
{
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(EXT4_DA_WRITE_BEGIN_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_ext4_da_write_begin_end(void *ignore, struct inode *inode,
		loff_t pos, unsigned size, unsigned flags)
{
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;
	tag_attr.nr_bytes = size;

	if (io_trace_this->enable)
		io_trace_add_log(EXT4_DA_WRITE_END_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_ext4_sync_file_enter(void *ignore, struct file *file, int datasync)
{
	struct inode *inode = file->f_mapping->host;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(EXT4_SYNC_ENTER_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_ext4_sync_write_wait_end(void *ignore, struct file *file, int datasync)
{
	return;
}

static void io_ext4_sync_file_end(void *ignore, struct file *filp, int ret)
{
	struct inode *inode = NULL;
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.parent = EXT4_SYNC_ENTER_TAG;
	tag_attr.func = io_trace_direct_delay;
	tag_attr.rw_flags = IO_TRACE_WRITE;
	tag_attr.func_get_fname = io_entry_fname;

	if (filp == NULL)
		return;

	inode = filp->f_mapping->host;
	if (inode == NULL)
		return;

	if (io_trace_this->enable)
		io_trace_add_log(EXT4_SYNC_EXIT_TAG, inode->i_ino, inode, &tag_attr);

	return;
}
static void io_block_write_begin_enter(void *ignore, struct inode *inode,
		struct page *page, loff_t pos, unsigned size)
{
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(BLOCK_WRITE_BEGIN_TAG, inode->i_ino, inode, &tag_attr);
	return;
}

static void io_block_write_begin_end(void *ignore, struct inode *inode,
		struct page *page, int err)
{
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(BLOCK_WRITE_END_TAG, inode->i_ino, inode, &tag_attr);
	return;
}

static void io_mpage_da_map_and_submit(void *ignore, struct inode *inode,
		ext4_fsblk_t fblk, unsigned int len)
{
	if (io_trace_this->enable)
		io_trace_global_log(EXT4_MAP_SUBMIT_TAG,
			inode->i_ino, len << inode->i_blkbits,
			0, 0, fblk << 3, 1, NULL, 0, NULL);
	return;
}

#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
static int iotrace_io_type_parse(unsigned long long op, unsigned long long flag)
{
	unsigned int rw_type = 0;

	switch (op) {
	case REQ_OP_READ:
		rw_type = rw_type | IO_TRACE_READ;
		break;
	case REQ_OP_WRITE:
		rw_type = rw_type | IO_TRACE_WRITE;
		break;
	case REQ_OP_DISCARD:
		rw_type = rw_type | IO_TRACE_DISCARD;
		break;
	case REQ_OP_SECURE_ERASE:
		rw_type = rw_type | IO_TRACE_SECURE_ERASE;
		break;
	case REQ_OP_WRITE_SAME:
		rw_type = rw_type | IO_TRACE_WRITE_SAME;
		break;
	case REQ_OP_FLUSH:
		rw_type = rw_type | IO_TRACE_FLUSH;
		break;
	default:
		break;
	}

	if (flag & REQ_FUA)
		rw_type = rw_type | IO_TRACE_FUA;
	if (flag & REQ_SYNC)
		rw_type = rw_type | IO_TRACE_SYNC;
	if (flag & REQ_META)
		rw_type = rw_type | IO_TRACE_META;
	if (flag & REQ_FG)
		rw_type = rw_type | IO_TRACE_FG;

	return rw_type;
}
#endif

static void blk_add_trace_rq_insert(void *ignore, struct request_queue *q, struct request *rq)
{
	if (io_trace_this->enable) {
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		unsigned int rw_type = iotrace_io_type_parse(req_op(rq), rq->cmd_flags);
		io_trace_global_log(BLOCK_RQ_INSERT_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			0, 0, 0, rw_type, NULL, 0, NULL);
#else
		io_trace_global_log(BLOCK_RQ_INSERT_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			0, 0, 0, rq->cmd_flags, NULL, 0, NULL);
#endif
	}
	return;
}
static void blk_add_trace_rq_issue(void *ignore, struct request_queue *q, struct request *rq)
{
	if (io_trace_this->enable) {
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		unsigned int rw_type = iotrace_io_type_parse(req_op(rq), rq->cmd_flags);
		io_trace_global_log(BLOCK_RQ_ISSUE_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			BLOCK_BIO_QUEUE_TAG, 0, 0, rw_type, NULL, 0, NULL);
#else
		io_trace_global_log(BLOCK_RQ_ISSUE_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			BLOCK_BIO_QUEUE_TAG, 0, 0, rq->cmd_flags,
			NULL, 0, NULL);
#endif
	}
	return;
}

#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,14,0)
static void blk_add_trace_rq_complete(void *ignore, struct request *rq,
		int error, unsigned int nr_bytes)
{
	unsigned char *volname = NULL;
	unsigned int rw_type;
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	unsigned char stream_type;
#endif
	if (io_trace_this->enable) {
		if (rq->part && rq->part->info)
			volname = rq->part->info->volname;
		else
			volname = NULL;

		rw_type = iotrace_io_type_parse(req_op(rq), rq->cmd_flags);
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
		if (blk_queue_query_unistore_enable(rq->q)) {
			stream_type = rq->mas_req.stream_type;
			io_trace_global_log_unistore(BLOCK_RQ_COMPLETE_TAG,
				blk_rq_pos(rq), blk_rq_bytes(rq),
				BLOCK_RQ_ISSUE_TAG, 0, 0, rw_type,
				NULL, 0, volname, stream_type);
			return;
		}
#endif
		io_trace_global_log(BLOCK_RQ_COMPLETE_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			BLOCK_RQ_ISSUE_TAG, 0, 0, rw_type, NULL, 0, volname);
	}
	return;
}

#else
static void blk_add_trace_rq_complete(void *ignore, struct request_queue *q,
		struct request *rq, unsigned int nr_bytes)
{
	unsigned char *volname = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
	unsigned int rw_type;
#endif
	if (io_trace_this->enable) {
		if (rq->part && rq->part->info)
			volname = rq->part->info->volname;
		else
			volname = NULL;

#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		rw_type = iotrace_io_type_parse(
			req_op(rq), rq->cmd_flags);
		io_trace_global_log(BLOCK_RQ_COMPLETE_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			BLOCK_RQ_ISSUE_TAG, 0, 0, rw_type, NULL, 0, volname);
#else
		io_trace_global_log(BLOCK_RQ_COMPLETE_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			BLOCK_RQ_ISSUE_TAG, 0, 0, rq->cmd_flags,
			NULL, 0, volname);
#endif
	}
	return;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
static void blk_add_trace_rq_abort(void *ignore, struct request_queue *q,
		struct request *rq)
{
	if (io_trace_this->enable) {
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		unsigned int rw_type = iotrace_io_type_parse(req_op(rq), rq->cmd_flags);
		io_trace_global_log(BLOCK_RQ_ABORT_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			0, 0, 0, rw_type, NULL, 0, NULL);
#else
		io_trace_global_log(BLOCK_RQ_ABORT_TAG,
			blk_rq_pos(rq), blk_rq_bytes(rq),
			0, 0, 0, rq->cmd_flags, NULL, 0, NULL);
#endif
	}
	return;
}
#endif

static void blk_add_trace_sleeprq(void *ignore, struct request_queue *q,
		struct bio *bio, int rw)
{
	if (io_trace_this->enable && bio) {
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		unsigned int rw_type = iotrace_io_type_parse(bio_op(bio), bio->bi_opf);
		io_trace_global_log(BLOCK_RQ_SLEEP_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, 0, rw_type, NULL, 0, NULL);
#else
		io_trace_global_log(BLOCK_RQ_SLEEP_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, 0, bio->bi_rw, NULL, 0, NULL);
#endif
	}
	return;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
static void blk_add_trace_bio_remap(void *ignore, struct request_queue *q,
		struct bio *bio, dev_t dev, sector_t from)
{
	if (io_trace_this->enable) {
#if LINUX_VERSION_CODE>= KERNEL_VERSION(4,9,0)
		unsigned int rw_type = iotrace_io_type_parse(bio_op(bio), bio->bi_opf);
		io_trace_global_log(BLOCK_BIO_REMAP_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, from, rw_type, NULL, 0, NULL);
#else
		io_trace_global_log(BLOCK_BIO_REMAP_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, from, bio->bi_rw, NULL, 0, NULL);
#endif
	}
	return;
}
#endif

#if LINUX_VERSION_CODE<KERNEL_VERSION(4,14,0)
static void mmc_blk_rw_start(void *ignore, unsigned int cmd,
		unsigned int addr, struct mmc_data *data)
{
	unsigned int rw_flags = 0;

	if (cmd == MMC_WRITE_MULTIPLE_BLOCK || cmd == MMC_WRITE_BLOCK)
		rw_flags |= IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_global_log(MMC_BLK_RW_START_TAG, addr, 0, 0, 0, 0,
			rw_flags, NULL, 0, NULL);
	return;
}

static void mmc_blk_rw_end(void *ignore, unsigned int cmd,
		unsigned int addr, struct mmc_data *data)
{
	unsigned int rw_flags = 0;

	if (cmd == MMC_WRITE_MULTIPLE_BLOCK || cmd == MMC_WRITE_BLOCK)
		rw_flags |= IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_global_log(MMC_BLK_RW_END_TAG, addr, 0,
			MMC_BLK_RW_START_TAG, 0, 0, rw_flags, NULL, 0, NULL);
	return;
}
#endif

static void scsi_dispatch_cmd_start(void *ignore, struct scsi_cmnd *cmd)
{
	unsigned int rw_flags = 0;
	unsigned char c[4];
	unsigned char len[2];
	unsigned int *addr;
	unsigned short *size;

	c[0] = cmd->cmnd[5];
	c[1] = cmd->cmnd[4];
	c[2] = cmd->cmnd[3];
	c[3] = cmd->cmnd[2];

	len[0] = cmd->cmnd[7];
	len[1] = cmd->cmnd[8];

	addr = (int*)c;
	size = (short*)len;

	if (cmd->cmnd[0] == 0x2A)
		rw_flags |= IO_TRACE_WRITE;
	else if (cmd->cmnd[0] == 0x35)
		rw_flags |= (IO_TRACE_SYNC | IO_TRACE_WRITE);

	if (io_trace_this->enable)
		io_trace_global_log(SCSI_BLK_CMD_RW_START_TAG, (*addr) * 8,
			(*size) * 16, 0, 0, 0, rw_flags, NULL, 0, NULL);

	return;
}

static void scsi_dispatch_cmd_done(void *ignore, struct scsi_cmnd *cmd)
{
	unsigned int rw_flags = 0;
	unsigned char c[4];
	unsigned char len[2];
	unsigned int *addr;
	unsigned short *size;
	struct scsi_cmnd *fsr_scsi = NULL;
	struct Scsi_Host *host = NULL;

	fsr_scsi = (struct scsi_cmnd *)cmd;

	if (fsr_scsi->device->host == NULL)
		return;

	host = fsr_scsi->device->host;
	fsr_hba = shost_priv(host);

	if (!fsr_hba) {
		io_trace_print("%s shost_priv host failed\n", __func__);
		return;
	}

	c[0] = cmd->cmnd[5];
	c[1] = cmd->cmnd[4];
	c[2] = cmd->cmnd[3];
	c[3] = cmd->cmnd[2];

	len[0] = cmd->cmnd[7];
	len[1] = cmd->cmnd[8];

	addr = (int*)c;
	size = (short*)len;

	if (cmd->cmnd[0] == 0x2A)
		rw_flags |= IO_TRACE_WRITE;
	else if (cmd->cmnd[0] == 0x35)
		rw_flags |= (IO_TRACE_SYNC | IO_TRACE_WRITE);

	if (io_trace_this->enable)
		io_trace_global_log(SCSI_BLK_CMD_RW_END_TAG, (*addr) * 8,
			(*size) * 16, SCSI_BLK_CMD_RW_START_TAG, 0,
			fsr_hba->lrb_in_use, rw_flags, NULL, 0, NULL);

	return;
}

static void io_f2fs_sync_file_enter(void *ignore, struct inode *inode)
{
	struct io_trace_tag_attr tag_attr;
	io_tag_attr_init(&tag_attr);
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(F2FS_SYNC_ENTER_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_f2fs_sync_file_exit(void *ignore, struct inode *inode, int need_cp,
		int datasync, int ret)
{
	struct io_trace_tag_attr tag_attr;

	io_tag_attr_init(&tag_attr);
	tag_attr.parent = F2FS_SYNC_ENTER_TAG;
	tag_attr.func = io_trace_direct_delay;
	tag_attr.rw_flags = IO_TRACE_WRITE;

	if (io_trace_this->enable)
		io_trace_add_log(F2FS_SYNC_EXIT_TAG, inode->i_ino, inode, &tag_attr);

	return;
}

static void io_f2fs_sync_fs(void *ignore, struct super_block *sb, int sync)
{
	if (io_trace_this->enable)
		io_trace_global_log(F2FS_SYNC_FS_TAG, sync, 0,
			0, 0, 0, IO_TRACE_WRITE, NULL, 0, NULL);

	return;
}

static void io_f2fs_write_checkpoint(void *ignore, struct super_block *sb, int reason,
		char *str_phase)
{
	if (io_trace_this->enable) {
		unsigned int handle = 1;
		if (strncmp(str_phase, "finish block_ops", 15) == 0)
			handle = 2;
		else if (strncmp(str_phase, "finish checkpoint", 15) == 0)
			handle = 3;
		io_trace_global_log(F2FS_WRITE_CP_TAG, reason, handle,
			0, 0, 0, IO_TRACE_WRITE, NULL, 0, NULL);
	}
	return;
}

#if LINUX_VERSION_CODE<KERNEL_VERSION(4,14,0)
#ifdef CONFIG_BLK_DEV_THROTTLING
static void blk_throttle_weight(void *ignore, struct bio *bio,
	unsigned int weight, unsigned int nr_queued)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_WEIGHT_TAG,
			bio->bi_iter.bi_sector, nr_queued,
			0, 0, weight, bio->bi_flags, NULL, 0, NULL);

	return;
}

static void blk_throttle_dispatch(void *ignore, struct bio *bio, unsigned int weight)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_DISP_TAG, bio->bi_iter.bi_sector,
			bio->bi_iter.bi_size,
			0, 0, weight, bio->bi_flags, NULL, 0, NULL);

	return;
}

static void blk_throttle_iocost(void *ignore, uint64_t bps, unsigned int iops, uint64_t bytes_disp,
	unsigned int io_disp)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_IOCOST_TAG, bytes_disp, io_disp,
			0, 0, bps, 0, NULL, iops, NULL);

	return;
}

static void blk_throttle_limit_start(void *ignore, struct bio *bio, int max_inflights, atomic_t inflights)
{
	int inflights_t;
	if (io_trace_this->enable) {
		inflights_t = atomic_dec_return(&inflights);
		io_trace_global_log(BLK_THRO_LIMIT_START_TAG,
			bio->bi_iter.bi_sector, max_inflights,
			0, 0, inflights_t, bio->bi_flags, NULL, 0, NULL);
	}
	return;
}

static void blk_throttle_limit_end(void *ignore, struct bio *bio)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_LIMIT_END_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, 0, bio->bi_flags, NULL, 0, NULL);

	return;
}

static void blk_throttle_bio_in(void *ignore, struct bio *bio)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_BIO_IN_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, 0, bio->bi_flags, NULL, 0, NULL);

	return;
}

static void blk_throttle_bio_out(void *ignore, struct bio *bio, long delay)
{
	if (io_trace_this->enable)
		io_trace_global_log(BLK_THRO_BIO_OUT_TAG,
			bio->bi_iter.bi_sector, bio->bi_iter.bi_size,
			0, 0, delay, bio->bi_flags, NULL, 0, NULL);

	return;
}
#endif
#endif
/* lint -restore */
/* lint save -e730 */
static void iotrace_register_tracepoints(void)
{
	int ret;
	/* vfs */
	ret = register_trace_generic_perform_write_enter(io_generic_perform_write_enter, NULL);
	WARN_ON(ret);
	ret = register_trace_generic_perform_write_end(io_generic_perform_write_end, NULL);
	WARN_ON(ret);
	ret = register_trace_generic_file_read_begin(io_generic_file_read_begin, NULL);
	WARN_ON(ret);
	ret = register_trace_generic_file_read_end(io_generic_file_read_end, NULL);
	WARN_ON(ret);
	ret = register_trace_ext4_da_write_begin(io_ext4_da_write_begin, NULL);
	WARN_ON(ret);
	ret = register_trace_ext4_da_write_begin_end(io_ext4_da_write_begin_end, NULL);
	WARN_ON(ret);
	/* ext4 */
	ret = register_trace_ext4_sync_file_enter(io_ext4_sync_file_enter, NULL);
	WARN_ON(ret);
	ret = register_trace_ext4_sync_write_wait_end(io_ext4_sync_write_wait_end, NULL);
	WARN_ON(ret);
	ret = register_trace_ext4_sync_file_end(io_ext4_sync_file_end, NULL);
	WARN_ON(ret);
	/* F2FS */
	ret = register_trace_f2fs_sync_file_enter(io_f2fs_sync_file_enter, NULL);
	WARN_ON(ret);
	ret = register_trace_f2fs_sync_file_exit(io_f2fs_sync_file_exit, NULL);
	WARN_ON(ret);
	ret = register_trace_f2fs_sync_fs(io_f2fs_sync_fs, NULL);
	WARN_ON(ret);
	ret = register_trace_f2fs_write_checkpoint(io_f2fs_write_checkpoint, NULL);
	WARN_ON(ret);
	/* page cache */
	ret = register_trace_block_write_begin_enter(io_block_write_begin_enter, NULL);
	WARN_ON(ret);
	ret = register_trace_block_write_begin_end(io_block_write_begin_end, NULL);
	WARN_ON(ret);
	ret = register_trace_mpage_da_map_and_submit(io_mpage_da_map_and_submit, NULL);
	WARN_ON(ret);

	/* block layer */
	ret = register_trace_block_rq_insert(blk_add_trace_rq_insert, NULL);
	WARN_ON(ret);
	ret = register_trace_block_rq_issue(blk_add_trace_rq_issue, NULL);
	WARN_ON(ret);
	ret = register_trace_block_rq_complete(blk_add_trace_rq_complete, NULL);
	WARN_ON(ret);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
	ret = register_trace_block_bio_remap(blk_add_trace_bio_remap, NULL);
	WARN_ON(ret);
	ret = register_trace_block_rq_abort(blk_add_trace_rq_abort, NULL);
	WARN_ON(ret);
#endif
	ret = register_trace_block_sleeprq(blk_add_trace_sleeprq, NULL);
	WARN_ON(ret);

#if LINUX_VERSION_CODE<KERNEL_VERSION(4,14,0)
	ret = register_trace_mmc_blk_rw_start(mmc_blk_rw_start, NULL);
	WARN_ON(ret);
	ret = register_trace_mmc_blk_rw_end(mmc_blk_rw_end, NULL);
	WARN_ON(ret);
#endif

#ifdef CONFIG_SCSI
	ret = register_trace_scsi_dispatch_cmd_start(scsi_dispatch_cmd_start, NULL);
	WARN_ON(ret);
	ret = register_trace_scsi_dispatch_cmd_done(scsi_dispatch_cmd_done, NULL);
	WARN_ON(ret);
#endif

#if LINUX_VERSION_CODE<KERNEL_VERSION(4,14,0)
#ifdef CONFIG_BLK_DEV_THROTTLING
	/* block throttle */
	ret = register_trace_block_throttle_weight(blk_throttle_weight, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_dispatch(blk_throttle_dispatch, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_iocost(blk_throttle_iocost, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_limit_start(blk_throttle_limit_start, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_limit_end(blk_throttle_limit_end, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_bio_in(blk_throttle_bio_in, NULL);
	WARN_ON(ret);
	ret = register_trace_block_throttle_bio_out(blk_throttle_bio_out, NULL);
	WARN_ON(ret);

#endif
#endif
	return;
}
/* lint -restore */
static void io_trace_setup(struct io_trace_ctrl *trace_ctrl)
{
	io_trace_print("io_trace_setup!\n");
	iotrace_register_tracepoints();

	return;
}

static unsigned char *io_mem_mngt_alloc(struct io_trace_mem_mngt *mem_mngt)
{
	unsigned char *ret_buf = NULL;

	raw_spin_lock(&trace_buff_spinlock);
	if (mem_mngt->index >= IO_RUNNING_MAX_PROCESS) {
		io_trace_print("mem alloc is full!\n");
		/* TODO: free memory */
		raw_spin_unlock(&trace_buff_spinlock);
		return NULL;
	}
	ret_buf = mem_mngt->buff[mem_mngt->index];
	mem_mngt->index++;
	raw_spin_unlock(&trace_buff_spinlock);

	return ret_buf;
}

static void io_mem_mngt_free(struct io_trace_mem_mngt *mem_mngt, unsigned char *buf)
{
	if (buf == NULL) {
		io_trace_print("free null buffer!\n");
		return;
	}
	raw_spin_lock(&trace_buff_spinlock);
	if ((mem_mngt->index - 1) >= 0) {
		mem_mngt->index--;
		mem_mngt->buff[mem_mngt->index] = buf;
	}
	raw_spin_unlock(&trace_buff_spinlock);

	return;
}

static int io_mem_mngt_init(struct io_trace_mem_mngt *mem_mngt)
{
	int i = 0;

	mem_mngt->index = 0;

	for (i = 0; i < IO_RUNNING_MAX_PROCESS; i++) {
		mem_mngt->buff[i] = kvmalloc(2 * IO_MAX_BUF_ENTRY, GFP_KERNEL);
		if (mem_mngt->buff[i] == NULL) {
			io_trace_print("io_mem_mngt_init kvmalloc failed!\n");
			goto failed;
		}
	}
	/* lint save -e747 -e826 */
	mem_mngt->all_trace_buff = kvmalloc(IO_MAX_GLOBAL_ENTRY, GFP_KERNEL);
	if (mem_mngt->all_trace_buff == NULL) {
		io_trace_print("io_mem_mngt_free all trace buff failed!\n");
		goto failed;
	}
	/* lint restore */
	return 0;

failed:
	i--;
	for (; i >= 0; i--)
		kvfree(mem_mngt->buff[i]);

	return -1;
}


static int trace_mm_init(struct io_trace_ctrl *trace_ctrl)
{
	int i = 0;

	io_trace_print("trace mm init start!\n");
	for (i = 0; i < IO_MAX_PROCESS; i++) {
		trace_ctrl->all_trace[i] = (struct io_trace_pid *)kvmalloc(
				sizeof(struct io_trace_pid), GFP_KERNEL);
		if (trace_ctrl->all_trace[i] == NULL) {
			io_trace_print("all trace kvmalloc failed!\n");
			goto failed;
		}
		memset(trace_ctrl->all_trace[i], 0, sizeof(struct io_trace_pid));
	}

	trace_ctrl->mem_mngt = (struct io_trace_mem_mngt *)kvmalloc(
			sizeof(struct io_trace_mem_mngt), GFP_KERNEL);
	if (trace_ctrl->mem_mngt == NULL) {
		io_trace_print("mem mngt kvmalloc failed!\n");
		goto failed;
	}

	if (io_mem_mngt_init(trace_ctrl->mem_mngt) < 0) {
		kvfree(trace_ctrl->mem_mngt);
		goto failed;
	}

	return 0;

failed:
	i--;
	for (; i >= 0; i--)
		kvfree(trace_ctrl->all_trace[i]);

	return -1;
}

static int trace_ctrl_init(struct io_trace_ctrl *trace_ctrl, unsigned int *total_mem)
{
	trace_ctrl->first = 0;
	trace_ctrl->enable = 0;
	trace_ctrl->pdev = NULL;
	trace_ctrl->filp = NULL;

	return 0;
}

static int io_count_thread_fn(void *data)
{
	int ret = 0;

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		if (kthread_should_stop())
			break;

		schedule_timeout(UFS_IO_COUNT_UPLOAD_INTER * HZ);

		__set_current_state(TASK_RUNNING);

		if (!io_trace_this->enable)
			continue;

		io_trace_this->enable = 0;

		ret = io_trace_count_upload();

		if (ret < 0)
			io_trace_print("io_count upload failed!\n");

		io_trace_this->enable = 1;
	}
	return 0;
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#ifdef HI18VV_FSR_GET_THREAD
static int apr_work_thread_fn(void *data)
{
	unsigned int ret;
	char file_name[128];
	static struct file *fsr_filp = NULL;
	int first_time = 1;

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		if (kthread_should_stop())
			break;

		if (first_time) {
			schedule_timeout(UFS_HI1861_FSR_UPLOAD_FIRST_INTER * HZ);
			first_time = 0;
		} else {
			schedule_timeout(UFS_HI1861_FSR_UPLOAD_INTER * HZ);
		}

		if ((ufs_manufacture != UFS_VENDOR_HI1861) && apr_work_thread) {
			apr_work_thread = NULL;
			break;
		}

		__set_current_state(TASK_RUNNING);
		if ((!io_trace_this->enable) || (fsr_hba == NULL))
			continue;

		io_trace_this->enable = 0;

		sprintf(file_name, "/data/log/jank/iotrace/log_%s", "hi18VV_fsr");
		fsr_filp = open_log_file(file_name);

		ret = io_fsr_log_get(fsr_filp);
		if (ret < 0)
			io_trace_print("apr get fsr failed!\n");

		if (fsr_filp)
			filp_close(fsr_filp, NULL);

		ret = io_trace_fsr_upload();
		if (ret < 0)
			io_trace_print("apr fsr upload failed\n");

		io_trace_this->enable = 1;
	}
	return 0;
}
#endif
#endif

static void ufs_manfid(unsigned int ufs_manufid)
{
	switch (ufs_manufid) {
	case 0x0003:
	case 0x0045:
	case 0x0002:
		dm_ufs_manfid = 3; /* Sandisk */
		break;
	case 0x0011:
	case 0x0098:
	case 0x0198:
		dm_ufs_manfid = 2; /* Toshiba */
		break;
	case 0x0090:
	case 0x00AD:
	case 0x01AD:
		dm_ufs_manfid = 4; /* Hynix */
		break;
	case 0x00FE:
	case 0x002C:
	case 0x0013:
		dm_ufs_manfid = 5; /* Micron */
		break;
	case 0x0015:
	case 0x00CE:
	case 0x01CE:
		dm_ufs_manfid = 1; /* Samsung */
		break;
	case 0x08b6:
		dm_ufs_manfid = 6; /* Hisi */
		break;
	default:
		dm_ufs_manfid = 0;
		break;
	}
}

static int __init io_trace_init(void)
{
	int ret = -1, err = 0;
	int total_mem = 0;

	io_trace_print("Enter %s\n", __FUNCTION__);
	io_trace_clear_account();
	if (io_trace_this == NULL) {
		io_trace_this = (struct io_trace_ctrl *)kzalloc(
				sizeof(struct io_trace_ctrl), GFP_KERNEL);
		total_mem += sizeof(struct io_trace_ctrl);
		if (NULL == io_trace_this) {
			io_trace_print("(%s)kzmalloc failed!", __FUNCTION__);
			return -1;
		}
		io_trace_print("io_trace_this is init!\n");
		if (trace_ctrl_init(io_trace_this, &total_mem) < 0)
			return -1;
	}

	io_trace_print("total mem : %d\n", total_mem);
	io_trace_print("Enter %s:%d\n", __FUNCTION__, __LINE__);
	spin_lock_init(&global_spinlock);
	mutex_init(&output_mutex);

	if (io_count_thread == NULL) {
		io_count_thread = kthread_create(io_count_thread_fn, NULL, "iotrace_delay");

		if (IS_ERR(io_count_thread)) {
			io_trace_print("Unable to start io count upload thread!\n");
			err = PTR_ERR(io_count_thread);
			io_count_thread = NULL;
		}

		if (!err) {
			io_trace_print("io count upload thread start succeed\n");
			wake_up_process(io_count_thread);
		}
	}

	ufs_manufacture = get_bootdevice_manfid();
	/* dm ufs manfid */
	ufs_manfid(ufs_manufacture);
	io_trace_print("manfid : 0x%x\n", dm_ufs_manfid);

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#ifdef HI18VV_FSR_GET_THREAD
	/* start fsr get thead */
	if ((ufs_manufacture == UFS_VENDOR_HI1861) && (apr_work_thread == NULL)) {
		apr_work_thread = kthread_create(apr_work_thread_fn, NULL, "hi18vv-fsr");

		if (IS_ERR(apr_work_thread)) {
			io_trace_print("Unable to start hi18vv FSR get thread!\n");
			err = PTR_ERR(apr_work_thread);
			apr_work_thread = NULL;
		}

		if (!err) {
			io_trace_print("Hi18vv FSR get start succeed\n");
			wake_up_process(apr_work_thread);
		}
	}
#endif
#endif
	io_trace_this->pdev = platform_device_register_simple("io_trace", -1, NULL, 0);
	if (!io_trace_this->pdev) {
		io_trace_print("(%s)register platform_device_register_simple null!", __FUNCTION__);
		return -1;
	}

	ret = platform_driver_register(&io_trace_driver);
	if (ret < 0) {
		io_trace_print("(%s)register platform_driver_register failed!", __FUNCTION__);
		return ret;
	}

	return 0;
}

static void __exit io_trace_exit(void)
{
	if (io_trace_this->pdev)
		platform_device_unregister(io_trace_this->pdev);

	platform_driver_unregister(&io_trace_driver);

	if (io_count_thread) {
		kthread_stop(io_count_thread);
		io_count_thread = NULL;
	}
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#ifdef HI18VV_FSR_GET_THREAD
	if (apr_work_thread) {
		kthread_stop(apr_work_thread);
		apr_work_thread = NULL;
	}
#endif
#endif
}

late_initcall(io_trace_init);
module_exit(io_trace_exit);
