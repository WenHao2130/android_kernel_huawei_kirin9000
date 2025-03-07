/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Sensor Hub Channel Bridge
 */

#include "contexthub_recovery.h"

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/of_address.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/regulator/consumer.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include <asm/cacheflush.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "contexthub_boot.h"
#include "contexthub_route.h"
#include "motion_detect.h"
#include <platform_include/smart/linux/base/ap/protocol.h>
#include "sensor_config.h"
#include "sensor_detect.h"

BLOCKING_NOTIFIER_HEAD(iom3_recovery_notifier_list);

wait_queue_head_t iom3_rec_waitq;
atomic_t iom3_rec_state = ATOMIC_INIT(IOM3_RECOVERY_UNINIT);
struct rdr_register_module_result current_sh_info;
struct completion sensorhub_rdr_completion;
void *g_sensorhub_extend_dump_buff;
uint8_t *g_sensorhub_dump_buff;
uint32_t g_enable_dump = 1;
uint32_t g_dump_extend_size;
uint32_t g_dump_region_list[DL_BOTTOM][DE_BOTTOM];

static struct workqueue_struct *iom3_rec_wq;
static struct delayed_work iom3_rec_work;
int g_iom3_state = IOM3_ST_NORMAL;
static struct completion iom3_rec_done;
static struct wakeup_source *iom3_rec_wl;
static void __iomem *iomcu_cfg_base;
static struct mutex mutex_recovery_cmd;
static u64 current_sh_core_id = RDR_IOM3; /* const */
static struct semaphore rdr_sh_sem;
static struct semaphore rdr_exce_sem;
static void __iomem *sysctrl_base;
static void __iomem *watchdog_base;
static unsigned int *dump_vir_addr;
static struct wakeup_source *rdr_wl;
static int nmi_reg;
static char g_dump_dir[PATH_MAXLEN] = SH_DMP_DIR;
static char g_dump_fs[PATH_MAXLEN] = SH_DMP_FS;
static uint32_t g_dump_index = -1;

extern void *g_sensorhub_extend_dump_buff;
extern uint32_t g_dump_extend_size;
extern uint32_t g_enable_dump;
extern struct completion sensorhub_rdr_completion;
extern uint32_t need_reset_io_power;
extern uint32_t need_set_3v_io_power;
extern struct regulator *sensorhub_vddio;
extern char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
extern struct config_on_ddr *g_p_config_on_ddr;
extern int key_state;
extern struct notifier_block charge_recovery_notify;
extern rproc_id_t ipc_ap_to_iom_mbx;
extern rproc_id_t ipc_ap_to_lpm_mbx;

extern void disable_fingerprint_when_sysreboot(void);
extern void disable_fingerprint_ud_when_sysreboot(void);
extern void rdr_system_error(u32 modid, u32 arg1, u32 arg2);
extern void emg_flush_logbuff(void);
extern void reset_logbuff(void);
extern void disable_motions_when_sysreboot(void);
extern void disable_cas_when_sysreboot(void);
extern void __disable_irq(struct irq_desc *desc, unsigned int irq, bool suspend);
extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

static const char *sh_reset_reasons[] = {
	"SH_FAULT_HARDFAULT",
	"SH_FAULT_BUSFAULT",
	"SH_FAULT_USAGEFAULT",
	"SH_FAULT_MEMFAULT",
	"SH_FAULT_NMIFAULT",
	"SH_FAULT_ASSERT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"SH_FAULT_INTERNELFAULT",
	"SH_FAULT_IPC_RX_TIMEOUT",
	"SH_FAULT_IPC_TX_TIMEOUT",
	"SH_FAULT_RESET",
	"SH_FAULT_USER_DUMP",
	"SH_FAULT_RESUME",
	"SH_FAULT_REDETECT",
	"SH_FAULT_PANIC",
	"SH_FAULT_NOC",
	"SH_FAULT_EXP_BOTTOM", /* also use as unknow dump */
};

static int get_watchdog_base(void)
{
	struct device_node *np = NULL;

	if (!watchdog_base) {
		np = of_find_compatible_node(NULL, NULL,
			"hisilicon,iomcu-watchdog");
		if (!np) {
			hwlog_err("can not find  watchdog node\n");
			return -1;
		}

		watchdog_base = of_iomap(np, 0);
		if (!watchdog_base) {
			hwlog_err("get watchdog_base  error\n");
			return -1;
		}
	}
	return 0;
}

static int get_sysctrl_base(void)
{
	struct device_node *np = NULL;

	if (!sysctrl_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
		if (!np) {
			hwlog_err("can not find  sysctrl node\n");
			return -1;
		}

		sysctrl_base = of_iomap(np, 0);
		if (!sysctrl_base) {
			hwlog_err("get sysctrl_base  error\n");
			return -1;
		}
	}
	return 0;
}

static void __get_dump_region_list(struct device_node *dump_node,
	char *prop_name, enum dump_loc loc)
{
	int i;
	int cnt;
	struct property *prop = NULL;

	prop = of_find_property(dump_node, prop_name, NULL);
	hwlog_info("=========get dump region %s config from dts\n", prop_name);
	if (!prop || !prop->value) {
		hwlog_err("%s prop %s invalid\n", __func__, prop_name);
	} else {
		cnt = prop->length / sizeof(uint32_t);
		if (of_property_read_u32_array(dump_node, prop_name,
			g_dump_region_list[loc - 1], cnt)) {
			hwlog_err("%s read %s from dts fail\n", __func__,
				prop_name);
		} else {
			for (i = 0; i < cnt; i++) {
				g_p_config_on_ddr->dump_config.elements[g_dump_region_list[loc - 1][i]].loc = loc;
				hwlog_info("region [%d]\n",
					g_dump_region_list[loc - 1][i]);
			}
		}
	}
}

static void get_dump_config_from_dts(void)
{
	char *pstr = NULL;
	struct device_node *dump_node = NULL;

	dump_node = of_find_node_by_name(NULL, "sensorhub_dump");
	if (!dump_node) {
		hwlog_err("%s find node sensorhub_dump fail\n", __func__);
		goto OUT;
	}

	if (of_property_read_u32(dump_node, "enable", &g_enable_dump))
		hwlog_err("%s failed to find property enable\n", __func__);

	if (!g_enable_dump)
		goto OUT;

	__get_dump_region_list(dump_node, "tcm_regions", DL_TCM);

	if (of_property_read_u32(dump_node, "extend_region_size",
		&g_dump_extend_size))
		hwlog_err("%s find node extend_region_size fail\n", __func__);

	hwlog_info("%s sensorhub extend_region_size is 0x%x\n", __func__,
		g_dump_extend_size);

	if (g_dump_extend_size)
		__get_dump_region_list(dump_node, "ext_regions", DL_EXT);

	if (of_property_read_string(dump_node, "dump_dir",
		(const char **)&pstr)) {
		hwlog_err("%s find node dump_dir err,use default\n", __func__);
		memset(g_dump_dir, 0, sizeof(g_dump_dir));
		strncpy(g_dump_dir, SH_DMP_DIR, PATH_MAXLEN - 1);
	}

	if (of_property_read_string(dump_node, "dump_fs",
		(const char **)&pstr)) {
		hwlog_err("%s find node dump_fs err, use default\n", __func__);
		memset(g_dump_fs, 0, sizeof(g_dump_fs));
		strncpy(g_dump_fs, SH_DMP_FS, PATH_MAXLEN - 1);
	}

OUT:
	hwlog_info("%s sensorhub dump enabled is %d,logDir:%s,parentDir:%s\n",
		__func__, g_enable_dump, g_dump_dir, g_dump_dir);
	g_p_config_on_ddr->dump_config.enable = g_enable_dump;
}

static int write_ramdump_info_to_sharemem(void)
{
	if (g_p_config_on_ddr == NULL) {
		hwlog_err("g_p_config_on_ddr is NULL, maybe ioremap %x failed\n",
			IOMCU_CONFIG_START);
		return -1;
	}

	g_p_config_on_ddr->dump_config.finish = SH_DUMP_INIT;
	g_p_config_on_ddr->dump_config.dump_addr = SENSORHUB_DUMP_BUFF_ADDR;
	g_p_config_on_ddr->dump_config.dump_size = SENSORHUB_DUMP_BUFF_SIZE;
	g_p_config_on_ddr->dump_config.ext_dump_addr = 0;
	g_p_config_on_ddr->dump_config.ext_dump_size = 0;
	g_p_config_on_ddr->dump_config.reason = 0;

	hwlog_warn("%s dumpaddr low:%x, dumpaddr high:%x, dumplen:%x, end\n",
		__func__, (u32)(g_p_config_on_ddr->dump_config.dump_addr),
		(u32)((g_p_config_on_ddr->dump_config.dump_addr) >> 32),
		g_p_config_on_ddr->dump_config.dump_size);
	return 0;
}

static void __send_nmi(void)
{
	if (sysctrl_base && nmi_reg != 0)
		writel(0x2, sysctrl_base + nmi_reg);
	else
		hwlog_err("sysctrl_base is %pK, nmi_reg is %d\n",
			sysctrl_base, nmi_reg);
}

static void notify_rdr_thread(void)
{
	up(&rdr_sh_sem);
}

#ifdef CONFIG_DFX_BB
static void sh_dump(u32 modid, u32 etype, u64 coreid, char *pathname,
	pfn_cb_dump_done pfn_cb)
{
	hwlog_info("%s\n", __func__);
	if (pfn_cb)
		pfn_cb(modid, current_sh_core_id);
}

static void sh_reset(u32 modid, u32 etype, u64 coreid)
{
	hwlog_info("%s\n", __func__);
	if (dump_vir_addr)
		memset(dump_vir_addr, 0, current_sh_info.log_len);
}

static int clean_rdr_memory(struct rdr_register_module_result rdr_info)
{
	dump_vir_addr = (unsigned int *)dfx_bbox_map(rdr_info.log_addr,
		rdr_info.log_len);
	if (!dump_vir_addr) {
		hwlog_err("dfx_bbox_map %llx failed in %s\n",
			rdr_info.log_addr, __func__);
		return -1;
	}
	memset(dump_vir_addr, 0, rdr_info.log_len);
	return 0;
}

static int rdr_sensorhub_register_core(void)
{
	struct rdr_module_ops_pub s_module_ops;
	int ret;

	s_module_ops.ops_dump = sh_dump;
	s_module_ops.ops_reset = sh_reset;

	ret = rdr_register_module_ops(current_sh_core_id, &s_module_ops,
		&current_sh_info);
	if (ret != 0)
		return ret;

	ret = clean_rdr_memory(current_sh_info);
	if (ret != 0)
		return ret;
	return ret;
}

static const char sh_module_name[] = "RDR_IOM7";
static const char sh_module_desc[] = "RDR_IOM7 for watchdog timeout issue.";
static const char sh_module_user_desc[] = "RDR_IOM7 for user trigger dump.";
static int sh_register_exception(void)
{
	struct rdr_exception_info_s einfo;
	int ret;

	memset(&einfo, 0, sizeof(struct rdr_exception_info_s));
	einfo.e_modid = SENSORHUB_MODID;
	einfo.e_modid_end = SENSORHUB_MODID;
	einfo.e_process_priority = RDR_WARN;
	einfo.e_reboot_priority = RDR_REBOOT_NO;
	einfo.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3;
	einfo.e_reentrant = RDR_REENTRANT_ALLOW;
	einfo.e_exce_type = IOM3_S_EXCEPTION;
	einfo.e_from_core = RDR_IOM3;
	einfo.e_upload_flag = RDR_UPLOAD_YES;
	memcpy(einfo.e_from_module, sh_module_name,
		(sizeof(sh_module_name) > MODULE_NAME_LEN) ? MODULE_NAME_LEN :
		sizeof(sh_module_name));
	memcpy(einfo.e_desc, sh_module_desc,
		(sizeof(sh_module_desc) > STR_EXCEPTIONDESC_MAXLEN) ?
		STR_EXCEPTIONDESC_MAXLEN : sizeof(sh_module_desc));
	ret = rdr_register_exception(&einfo);
	if (!ret)
		return ret;

	einfo.e_modid = SENSORHUB_USER_MODID;
	einfo.e_modid_end = SENSORHUB_USER_MODID;
	einfo.e_exce_type = IOM3_S_USER_EXCEPTION;
	memcpy(einfo.e_desc, sh_module_user_desc,
		(sizeof(sh_module_user_desc) > STR_EXCEPTIONDESC_MAXLEN) ?
		STR_EXCEPTIONDESC_MAXLEN : sizeof(sh_module_user_desc));
	ret = rdr_register_exception(&einfo);
	return ret;
}
#endif

static void wdt_stop(void)
{
	writel(REG_UNLOCK_KEY, watchdog_base + WDOGLOCK);
	writel(0x0, watchdog_base + WDOGCTRL);
	writel(0x1, watchdog_base + WDOGINTCLR);
	writel(0x01, watchdog_base + WDOGLOCK);
}

static irqreturn_t watchdog_handler(int irq, void *data)
{
	wdt_stop();
	hwlog_warn("%s start\n", __func__);
	peri_used_request();
	__pm_stay_awake(rdr_wl);
	/* release exception sem */
	up(&rdr_exce_sem);
	return IRQ_HANDLED;
}

static int __sh_create_dir(char *path)
{
	int fd;
	mm_segment_t old_fs;

	if (!path) {
		hwlog_err("invalid  parameter. path:%pK.\n", path);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fd = sys_access(path, 0);
	if (fd != 0) {
		hwlog_info("sh: need create dir %s\n", path);
		fd = sys_mkdir(path, DIR_LIMIT);
		if (fd < 0) {
			hwlog_err("sh:create dir %s err,ret = %d\n", path, fd);
			set_fs(old_fs);
			return fd;
		}

		hwlog_info("sh: create dir %s successed [%d]\n", path, fd);
	}
	set_fs(old_fs);
	return 0;
}

static void sh_wait_fs(const char *path)
{
	int fd;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	do {
		fd = sys_access(path, 0);
		if (fd) {
			msleep(10);
			hwlog_info("%s wait ...\n", __func__);
		}
	} while (fd);
	set_fs(old_fs);
}

static int sh_savebuf2fs(char *logpath, char *filename, void *buf, u32 len,
	u32 is_append)
{
	int ret;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];

	if (!logpath || !filename || !buf || len <= 0) {
		hwlog_err("invalid para path:%pK name:%pK buf:%pK len:0x%x\n",
			logpath, filename, buf, len);
		ret = -1;
		goto param_err;
	}

	snprintf(path, PATH_MAXLEN, "%s/%s", logpath, filename);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		set_fs(old_fs);
		hwlog_err("%s():create file %s err.\n", __func__, path);
		ret = -1;
		goto param_err;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	ret = vfs_write(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		hwlog_err("%s:write file %s exception with ret %d\n", __func__,
			path, ret);
		goto write_err;
	}

	vfs_fsync(fp, 0);
write_err:
	filp_close(fp, NULL);
	ret = (int)sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID);
	if (ret)
		hwlog_err("[%s], chown %s uid [%d] gid [%d] failed err [%d]\n",
			__func__, path, ROOT_UID, SYSTEM_GID, ret);
	set_fs(old_fs);
param_err:
	return ret;
}

static int sh_readfs2buf(char *logpath, char *filename, void *buf, u32 len)
{
	int ret;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];

	if (!logpath || !filename || !buf || len <= 0) {
		hwlog_err("invalid para path:%pK, name:%pK buf:%pK len:0x%x\n",
			logpath, filename, buf, len);
		goto param_err;
	}

	snprintf(path, PATH_MAXLEN, "%s/%s", logpath, filename);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if (sys_access(path, 0) != 0)
		goto file_err;

	flags = O_RDWR;
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		hwlog_err("%s():open file %s err.\n", __func__, path);
		goto file_err;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	ret = vfs_read(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		hwlog_err("%s:read file %s exception with ret %d.\n",
			__func__, path, ret);
		ret = -1;
	}
	filp_close(fp, NULL);
file_err:
	set_fs(old_fs);
param_err:
	return ret;
}

static int sh_create_dir(const char *path)
{
	char cur_path[64];
	int index = 0;

	if (!path) {
		hwlog_err("invalid  parameter. path:%pK\n", path);
		return -1;
	}
	memset(cur_path, 0, 64);
	if (*path != '/')
		return -1;
	cur_path[index++] = *path++;
	while (*path != '\0') {
		if (*path == '/')
			__sh_create_dir(cur_path);

		cur_path[index] = *path;
		path++;
		index++;
	}
	return 0;
}

extern char *rdr_get_timestamp(void);
#ifdef CONFIG_DFX_BB
extern u64 rdr_get_tick(void);
#endif

static int get_dump_reason_idx(void)
{
	if (g_p_config_on_ddr->dump_config.reason >= ARRAY_SIZE(sh_reset_reasons))
		return ARRAY_SIZE(sh_reset_reasons) - 1;
	else
		return g_p_config_on_ddr->dump_config.reason;
}

static int write_sh_dump_history(void)
{
	int ret = 0;
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	mm_segment_t old_fs;
	char local_path[PATH_MAXLEN];
	char date[DATATIME_MAXLEN];

	hwlog_info("%s: write sensorhub dump history file\n", __func__);
	memset(date, 0, DATATIME_MAXLEN);
#ifdef CONFIG_DFX_BB
	snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(),
		rdr_get_tick());
#endif

	memset(local_path, 0, PATH_MAXLEN);
	snprintf(local_path, PATH_MAXLEN, "%s/%s", g_dump_dir, "history.log");

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* check file size */
	if (vfs_stat(local_path, &historylog_stat) == 0 &&
		historylog_stat.size > HISTORY_LOG_MAX) {
		hwlog_info("truncate dump history log\n");
		sys_unlink(local_path); /* delete history.log */
	}

	set_fs(old_fs);
	/* write history file */
	memset(buf, 0, HISTORY_LOG_SIZE);
	snprintf(buf, HISTORY_LOG_SIZE, "reason [%s], [%02d], time [%s]\n",
		sh_reset_reasons[get_dump_reason_idx()], g_dump_index, date);
	sh_savebuf2fs(g_dump_dir, "history.log", buf, strlen(buf), 1);
	return ret;
}

static void get_max_dump_cnt(void)
{
	int ret;
	uint32_t index;

	/* find max index */
	ret = sh_readfs2buf(g_dump_dir, "dump_max", &index, sizeof(index));
	if (ret < 0)
		g_dump_index = -1;
	else
		g_dump_index = index;

	g_dump_index++;
	if (g_dump_index == MAX_DUMP_CNT)
		g_dump_index = 0;
	sh_savebuf2fs(g_dump_dir, "dump_max", &g_dump_index,
		sizeof(g_dump_index), 0);
}

static int write_sh_dump_file(void)
{
	char date[DATATIME_MAXLEN];
	char path[PATH_MAXLEN];
	struct dump_zone_head *dzh = NULL;

	memset(date, 0, DATATIME_MAXLEN);
#ifdef CONFIG_DFX_BB
	snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(),
		rdr_get_tick());
#endif
	memset(path, 0, PATH_MAXLEN);
	snprintf(path, PATH_MAXLEN, "sensorhub-%02d.dmp", g_dump_index);
	hwlog_info("%s: write sensorhub dump  file %s\n", __func__, path);
	hwlog_err("sensorhub recovery source is %s\n",
		sh_reset_reasons[get_dump_reason_idx()]);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	flush_cache_all();
#endif
	/* write share part */
	if (g_sensorhub_dump_buff) {
		dzh = (struct dump_zone_head *)g_sensorhub_dump_buff;
		sh_savebuf2fs(g_dump_dir, path, g_sensorhub_dump_buff,
			min(g_p_config_on_ddr->dump_config.dump_size, dzh->len), 0);
	}
	/* write extend part */
	if (g_sensorhub_extend_dump_buff) {
		dzh = g_sensorhub_extend_dump_buff;
		sh_savebuf2fs(g_dump_dir, path, g_sensorhub_extend_dump_buff,
			min(g_p_config_on_ddr->dump_config.ext_dump_size, dzh->len),
			1);
	}
	return 0;
}

static int save_sh_dump_file(void)
{
	if (!g_enable_dump) {
		hwlog_info("%s skipped\n", __func__);
		return 0;
	}
	sh_wait_fs(g_dump_fs);
	hwlog_info("%s fs ready\n", __func__);
	/* check and create dump dir */
	if (sh_create_dir(g_dump_dir)) {
		hwlog_err("%s fail to create dir %s\n", __func__, g_dump_dir);
		return -1;
	}
	get_max_dump_cnt();
	/* write history file */
	write_sh_dump_history();
	/* write dump file */
	write_sh_dump_file();
	return 0;
}

static int rdr_sh_thread(void *arg)
{
	int timeout;

	while (1) {
		timeout = 2000;
		down(&rdr_sh_sem);

		if (g_enable_dump) {
			hwlog_warn("======dump sensorhub log start=======\n");
			while (g_p_config_on_ddr->dump_config.finish !=
				SH_DUMP_FINISH && timeout--)
				mdelay(1);
			hwlog_warn("======sensorhub dump finished=======\n");
			hwlog_warn("dump reason idx %d\n",
				g_p_config_on_ddr->dump_config.reason);
			/* write to fs */
			save_sh_dump_file();
			/* free buff */
			if (g_sensorhub_extend_dump_buff) {
				hwlog_info("%s free alloc\n", __func__);
				kfree(g_sensorhub_extend_dump_buff);
				g_sensorhub_extend_dump_buff = NULL;
			}
			hwlog_warn("=======dump sensorhub log end========\n");
		}
		__pm_relax(rdr_wl);
		peri_used_release();
		complete_all(&sensorhub_rdr_completion);
	}

	return 0;
}

static int rdr_exce_thread(void *arg)
{
	while (1) {
		down(&rdr_exce_sem);
		hwlog_warn(" ==============trigger exception==============\n");
		iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_INTERNELFAULT);
	}

	return 0;
}

int register_iom3_recovery_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&iom3_recovery_notifier_list,
		nb);
}

int get_iom3_state(void)
{
	return g_iom3_state;
}

int iom3_rec_sys_callback(const struct pkt_header *head)
{
	int ret;

	if (atomic_read(&iom3_rec_state) == IOM3_RECOVERY_MINISYS) {
		if (((pkt_sys_statuschange_req_t *)head)->status ==
			ST_MINSYSREADY) {
			hwlog_info("REC sys ready mini\n");
			ret = send_fileid_to_mcu();
			if (ret)
				hwlog_err("REC get sen cfg err,ret=%d\n", ret);
			else
				hwlog_info("REC get sensors cfg data succ\n");
		} else if (((pkt_sys_statuschange_req_t *)head)->status ==
			ST_MCUREADY) {
			hwlog_info("REC mcu all ready\n");
			ret = motion_set_cfg_data();
			if (ret < 0)
				hwlog_err("set cfg data err ret=%d\n", ret);
			ret = sensor_set_cfg_data();
			if (ret < 0)
				hwlog_err("REC sensor detect ret=%d\n", ret);
			else
				complete(&iom3_rec_done);
		}
	}

	return 0;
}

static void notify_modem_when_iom3_recovery_finish(void)
{
	uint16_t status = ST_RECOVERY_FINISH;
	struct write_info pkg_ap;

	hwlog_info("%s\n", __func__);

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_STATUSCHANGE_REQ;
	pkg_ap.wr_buf = &status;
	pkg_ap.wr_len = sizeof(status);
	write_customize_cmd(&pkg_ap, NULL, false);
}

static void disable_key_when_sysreboot(void)
{
	int ret;
	struct write_info winfo;

	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return;
	}

	winfo.tag = TAG_KEY;
	winfo.cmd = CMD_CMN_CLOSE_REQ;
	winfo.wr_len = 0;
	winfo.wr_buf = NULL;
	ret = write_customize_cmd(&winfo, NULL, false);
	if (ret < 0)
		hwlog_err("write close cmd err.\n");

	hwlog_info("close key when reboot\n");
}

static void enable_key_when_recovery_iom3(void)
{
	int ret;
	interval_param_t interval_param;

	memset(&interval_param, 0, sizeof(interval_param));
	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return;
	}

	hwlog_info("%s ++\n", __func__);
	if (key_state) { /* open */
		ret = inputhub_sensor_enable_nolock(TAG_KEY, true);
		if (ret) {
			hwlog_err("write open cmd err.\n");
			return;
		}
		memset(&interval_param, 0, sizeof(interval_param));
		interval_param.period = 20;
		interval_param.mode = AUTO_MODE;
		interval_param.batch_count = 1;
		ret = inputhub_sensor_setdelay_nolock(TAG_KEY, &interval_param);
		if (ret)
			hwlog_err("write interval cmd err.\n");
	} else { /* close */
		ret = inputhub_sensor_enable_nolock(TAG_KEY, false);
		if (ret < 0)
			hwlog_err("write close cmd err.\n");
	}
	hwlog_info("%s --\n", __func__);
}

static void disable_sensors_when_sysreboot(void)
{
	int tag;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (sensor_status.status[tag]) {
			if (tag == TAG_STEP_COUNTER)
				inputhub_sensor_enable_stepcounter(false,
					TYPE_STANDARD);
			else
				inputhub_sensor_enable(tag, false);
			msleep(50);
			hwlog_info("disable sensor - %d before reboot\n", tag);
		}
	}
}

static void operations_when_recovery_iom3(void)
{
	hwlog_info("%s\n", __func__);
	/* recovery key */
	enable_key_when_recovery_iom3();
	notify_modem_when_iom3_recovery_finish();
}
#ifdef CONFIG_DFX_BB
static int rdr_sensorhub_init_early(void)
{
	int ret;

	/* register module. */
	ret = rdr_sensorhub_register_core();
	if (ret != 0)
		return ret;
	/* write ramdump info to share mem */
	ret = write_ramdump_info_to_sharemem();
	if (ret != 0)
		return ret;
	get_dump_config_from_dts();
	g_sensorhub_dump_buff = (uint8_t *)ioremap_wc(SENSORHUB_DUMP_BUFF_ADDR,
		SENSORHUB_DUMP_BUFF_SIZE);
	if (!g_sensorhub_dump_buff) {
		hwlog_err("%s failed remap dump buff\n", __func__);
		return -EINVAL;
	}
	/* regitster exception. */
	ret = sh_register_exception(); /* 0:error */
	if (ret == 0)
		ret = -EINVAL;
	else
		ret = 0;
	return ret;
}
#endif

static int sensorhub_panic_notify(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int timeout;

	hwlog_warn("%s start\n", __func__);
	timeout = 100;
	__send_nmi();
	hwlog_warn("%s\n", __func__);

	while (g_p_config_on_ddr->dump_config.finish != SH_DUMP_FINISH && timeout--)
		mdelay(1);

	hwlog_warn("%s done\n", __func__);
	return NOTIFY_OK;
}

int sensorhub_noc_notify(int value)
{
	if (is_sensorhub_disabled()) {
		hwlog_err("[%s]do not handle noc as sensorhub is disabled\n", __func__);
		return 0;
	}

	if (value <= 1) {
		hwlog_warn("%s :read err,no need recovery iomcu\n", __func__);
		return 0;
	}

	hwlog_warn("%s start\n", __func__);
	iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_NOC);
	wait_for_completion(&sensorhub_rdr_completion);
	hwlog_warn("%s done\n", __func__);

	return 0;
}

static struct notifier_block sensorhub_panic_block = {
	.notifier_call = sensorhub_panic_notify,
};

static int get_nmi_offset(void)
{
	struct device_node *sh_node = NULL;

	sh_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub_nmi");
	if (!sh_node) {
		hwlog_err("%s, can not find node  sensorhub_nmi\n", __func__);
		return -1;
	}

	if (of_property_read_u32(sh_node, "nmi_reg", &nmi_reg)) {
		hwlog_err("%s:read nmi reg err:value is %d\n", __func__,
			nmi_reg);
		return -1;
	}

	hwlog_info("%s arch nmi reg is 0x%x\n", __func__, nmi_reg);
	return 0;
}

static int get_iomcu_cfg_base(void)
{
	struct device_node *np = NULL;

	if (!iomcu_cfg_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,iomcuctrl");
		if (!np) {
			hwlog_err("can not find iomcuctrl node\n");
			return -1;
		}

		iomcu_cfg_base = of_iomap(np, 0);
		if (!iomcu_cfg_base) {
			hwlog_err("get iomcu_cfg_base  error\n");
			return -1;
		}
	}

	return 0;
}

static inline void show_iom3_stat(void)
{
	hwlog_err("CLK_SEL:0x%x,DIV0:0x%x,DIV1:0x%x,CLKSTAT0:0x%x, RSTSTAT0:0x%x\n",
		readl(iomcu_cfg_base + IOMCU_CLK_SEL),
		readl(iomcu_cfg_base + IOMCU_CFG_DIV0),
		readl(iomcu_cfg_base + IOMCU_CFG_DIV1),
		readl(iomcu_cfg_base + CLKSTAT0_OFFSET),
		readl(iomcu_cfg_base + RSTSTAT0_OFFSET));
}

static void reset_i2c_0_controller(void)
{
	unsigned long flags;

	local_irq_save(flags);
	writel(I2C_0_RST_VAL, iomcu_cfg_base + RSTEN0_OFFSET);
	udelay(5);
	writel(I2C_0_RST_VAL, iomcu_cfg_base + RSTDIS0_OFFSET);
	local_irq_restore(flags);
}

static void reset_sensor_power(void)
{
	int ret;

	if (!need_reset_io_power) {
		hwlog_warn("%s: no need to reset sensor power\n", __func__);
		return;
	}

	if (IS_ERR(sensorhub_vddio)) {
		hwlog_err("%s: regulator_get fail\n", __func__);
		return;
	}
	ret = regulator_disable(sensorhub_vddio);
	if (ret < 0) {
		hwlog_err("failed to disable regulator sensorhub_vddio\n");
		return;
	}
	msleep(10);
	if (need_set_3v_io_power) {
		ret = regulator_set_voltage(sensorhub_vddio, SENSOR_VOLTAGE_3V,
			SENSOR_VOLTAGE_3V);
		if (ret < 0) {
			hwlog_err("set sensorhub_vddio voltage to 3V fail\n");
			return;
		}
	}
	ret = regulator_enable(sensorhub_vddio);
	msleep(5);
	if (ret < 0) {
		hwlog_err("failed to enable regulator sensorhub_vddio\n");
		return;
	}
	hwlog_info("%s done\n", __func__);
}

static void iom3_recovery_work(struct work_struct *work)
{
	int rec_nest_count = 0;
	int rc;
	u32 ack_buffer;
	u32 tx_buffer;

	hwlog_err("%s enter\n", __func__);
	__pm_stay_awake(iom3_rec_wl);
	peri_used_request();
	wait_for_completion(&sensorhub_rdr_completion);

recovery_iom3:
	if (rec_nest_count++ > IOM3_REC_NEST_MAX) {
		hwlog_err("unlucky recovery iom3 times exceed limit\n");
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_FAILED);
		blocking_notifier_call_chain(&iom3_recovery_notifier_list,
			IOM3_RECOVERY_FAILED, NULL);
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
		peri_used_release();
		__pm_relax(iom3_rec_wl);
		hwlog_err("%s exit\n", __func__);
		return;
	}

	/*
	 * fix bug nmi can't be clear by iomcu,
	 * or iomcu will not start correctly
	 */
	if (readl(sysctrl_base + nmi_reg) & 0x2)
		hwlog_err("%s nmi remain\n", __func__);
	writel(0, sysctrl_base + nmi_reg);

	show_iom3_stat();   /* only for IOM3 debug */
	reset_sensor_power();
	/* reload iom3 system */
	tx_buffer = RELOAD_IOM3_CMD;
	rc = RPROC_ASYNC_SEND(ipc_ap_to_lpm_mbx, &tx_buffer, 1);
	if (rc) {
		hwlog_err("RPROC reload iom3 failed %d, nest_count %d\n",
			rc, rec_nest_count);
		goto recovery_iom3;
	}

	show_iom3_stat(); /* only for IOM3 debug */
	reset_i2c_0_controller();
	reset_logbuff();
	write_ramdump_info_to_sharemem();
	write_timestamp_base_to_sharemem();

	msleep(5);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_MINISYS);

	/* startup iom3 system */
	reinit_completion(&iom3_rec_done);
	tx_buffer = STARTUP_IOM3_CMD;
	rc = RPROC_SYNC_SEND(ipc_ap_to_iom_mbx, &tx_buffer, 1, &ack_buffer, 1);
	if (rc) {
		hwlog_err("RPROC start iom3 failed %d, nest_count %d\n",
			rc, rec_nest_count);
		goto recovery_iom3;
	}

	hwlog_err("RPROC restart iom3 success\n");
	show_iom3_stat();      /* only for IOM3 debug */

	/* dynamic loading */
	if (!wait_for_completion_timeout(&iom3_rec_done, 5 * HZ)) {
		hwlog_err("wait for iom3 system ready timeout\n");
		msleep(1000);
		goto recovery_iom3;
	}

	/* repeat send cmd */
	msleep(100);     /* wait iom3 finish handle config-data */
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_DOING);
	hwlog_err("%s doing\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_DOING, NULL);
	operations_when_recovery_iom3();
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_3RD_DOING, NULL); /* recovery pdr */
	hwlog_err("%s pdr recovery\n", __func__);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	__pm_relax(iom3_rec_wl);
	hwlog_err("%s finish recovery\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_IDLE, NULL);
	hwlog_err("%s exit\n", __func__);
	peri_used_release();
}

int iom3_need_recovery(int modid, enum exp_source f)
{
	int ret = 0;
	int old_state;

	old_state = atomic_cmpxchg(&iom3_rec_state, IOM3_RECOVERY_IDLE,
		IOM3_RECOVERY_START);
	hwlog_err("recovery prev state %d, modid 0x%x\n", old_state, modid);
	if (old_state == IOM3_RECOVERY_IDLE) {
		__pm_wakeup_event(iom3_rec_wl, jiffies_to_msecs(10 * HZ));
		blocking_notifier_call_chain(&iom3_recovery_notifier_list,
			IOM3_RECOVERY_START, NULL);

		if (f > SH_FAULT_INTERNELFAULT)
			g_p_config_on_ddr->dump_config.reason = (uint8_t)f;

		/* flush old logs */
		emg_flush_logbuff();

		/* write extend dump config */
		if (g_enable_dump && g_dump_extend_size &&
			!g_sensorhub_extend_dump_buff) {
			g_sensorhub_extend_dump_buff =
				kmalloc(g_dump_extend_size, GFP_KERNEL);
			hwlog_warn("%s alloc pages logic %pK phy addr %pK\n",
				__func__, g_sensorhub_extend_dump_buff,
				(void *)virt_to_phys(g_sensorhub_extend_dump_buff));

			if (g_sensorhub_extend_dump_buff) {
				g_p_config_on_ddr->dump_config.ext_dump_addr =
					virt_to_phys(g_sensorhub_extend_dump_buff);
				g_p_config_on_ddr->dump_config.ext_dump_size =
					g_dump_extend_size;
				barrier();
			}
		}

#ifdef CONFIG_DFX_BB
		rdr_system_error(modid, 0, 0);
#endif
		reinit_completion(&sensorhub_rdr_completion);
		__send_nmi();
		notify_rdr_thread();
		queue_delayed_work(iom3_rec_wq, &iom3_rec_work, 0);
	} else if (f == SH_FAULT_INTERNELFAULT &&
		completion_done(&sensorhub_rdr_completion)) {
		__pm_relax(rdr_wl);
		peri_used_release();
	}
	return ret;
}

static int shb_recovery_notifier(struct notifier_block *nb, unsigned long foo,
	void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s %lu +\n", __func__, foo);
	mutex_lock(&mutex_recovery_cmd);
	switch (foo) {
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
		g_iom3_state = IOM3_ST_RECOVERY;
		break;
	case IOM3_RECOVERY_DOING:
	case IOM3_RECOVERY_3RD_DOING:
		g_iom3_state = IOM3_ST_REPEAT;
		break;
	case IOM3_RECOVERY_FAILED:
		hwlog_err("%s -recovery failed\n", __func__);
		/* fall-through */
	case IOM3_RECOVERY_IDLE:
		g_iom3_state = IOM3_ST_NORMAL;
		wake_up_all(&iom3_rec_waitq);
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	mutex_unlock(&mutex_recovery_cmd);
	hwlog_info("%s -\n", __func__);
	return 0;
}

static int shb_reboot_notifier(struct notifier_block *nb, unsigned long foo,
	void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("shb:%s: %ld +\n", __func__, foo);
	if (foo == SYS_RESTART) {
		disable_sensors_when_sysreboot();
		disable_motions_when_sysreboot();
		disable_cas_when_sysreboot();
		disable_fingerprint_when_sysreboot();
		disable_fingerprint_ud_when_sysreboot();
		disable_key_when_sysreboot();
	}
	hwlog_info("shb:%s: -\n", __func__);
	return 0;
}

static struct notifier_block reboot_notify = {
	.notifier_call = shb_reboot_notifier,
	.priority = -1,
};

static struct notifier_block recovery_notify = {
	.notifier_call = shb_recovery_notifier,
	.priority = -1,
};

extern int g_sensorhub_wdt_irq;
static int rdr_sensorhub_init(void)
{
	int ret = 0;

#ifdef CONFIG_DFX_BB
	if (rdr_sensorhub_init_early() != 0) {
		hwlog_err("rdr_sensorhub_init_early faild.\n");
		ret = -EINVAL;
	}
#endif
	sema_init(&rdr_sh_sem, 0);
	if (!kthread_run(rdr_sh_thread, NULL, "rdr_sh_thread")) {
		hwlog_err("create thread rdr_sh_main_thread faild.\n");
		ret = -EINVAL;
		return ret;
	}
	sema_init(&rdr_exce_sem, 0);
	if (!kthread_run(rdr_exce_thread, NULL, "rdr_exce_thread")) {
		hwlog_err("create thread rdr_sh_exce_thread faild.\n");
		ret = -EINVAL;
		return ret;
	}
	if (get_sysctrl_base() != 0) {
		hwlog_err("get sysctrl addr faild.\n");
		ret = -EINVAL;
		return ret;
	}
	if (get_watchdog_base() != 0) {
		hwlog_err("get watchdog addr faild.\n");
		ret = -EINVAL;
		return ret;
	}
	if (g_sensorhub_wdt_irq < 0) {
		hwlog_err("%s g_sensorhub_wdt_irq get error\n", __func__);
		return -EINVAL;
	}
	if (request_irq(g_sensorhub_wdt_irq, watchdog_handler, 0, "watchdog",
		NULL)) {
		hwlog_err("%s failure requesting watchdog irq\n", __func__);
		return -EINVAL;
	}
	if (get_nmi_offset() != 0)
		hwlog_err("get_nmi_offset faild.\n");
	if (atomic_notifier_chain_register(&panic_notifier_list,
		&sensorhub_panic_block))
		hwlog_err("%s sensorhub panic register failed\n", __func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	rdr_wl = wakeup_source_register(NULL, "rdr_sensorhub");
#else
	rdr_wl = wakeup_source_register("rdr_sensorhub");
#endif
	init_completion(&sensorhub_rdr_completion);
	return ret;
}

int recovery_init(void)
{
	int ret;

	if (get_iomcu_cfg_base())
		return -1;
	ret = rdr_sensorhub_init();
	if (ret < 0)
		hwlog_err("%s rdr_sensorhub_init ret=%d\n", __func__, ret);
	mutex_init(&mutex_recovery_cmd);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	iom3_rec_wq = create_singlethread_workqueue("iom3_rec_wq");
	if (!iom3_rec_wq) {
		hwlog_err("faild create iom3 recovery workqueue in %s\n",
			__func__);
		return -1;
	}

	INIT_DELAYED_WORK(&iom3_rec_work, iom3_recovery_work);
	init_completion(&iom3_rec_done);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	iom3_rec_wl = wakeup_source_register(NULL, "iom3_rec_wl");
#else
	iom3_rec_wl = wakeup_source_register("iom3_rec_wl");
#endif

	init_waitqueue_head(&iom3_rec_waitq);
	register_iom3_recovery_notifier(&recovery_notify);
	register_reboot_notifier(&reboot_notify);
	return 0;
}
