/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2021. All rights reserved.
 * Description: the hw_rscan_scanner.c for kernel space root scan
 * Create: 2016-06-18
 */

#include <chipset_common/security/hw_kernel_stp_interface.h>
#include "./include/hw_rscan_scanner.h"
#include "./include/hw_rscan_utils.h"
#include "./include/hw_rscan_whitelist.h"

#define KCODE_OFFSET 0
#define SYSCALL_OFFSET 1
#define SEHOOKS_OFFSET 2
#define SESTATUS_OFFSET 3

#define RSCAN_LOG_TAG "hw_rscan_scanner"
#define DEFAULT_PROC "/init"

static DEFINE_MUTEX(scanner_lock);      /* lint -save -e64 -e785 -e708 -e570 */
static DEFINE_MUTEX(whitelist_lock);    /* lint -save -e64 -e785 -e708 -e570 */
static char *g_trustlist_proc = RPROC_WHITE_LIST_STR;
static int g_rs_data_init = RSCAN_UNINIT;
static int g_root_scan_hot_fix;

static struct item_bits itembits[MAX_NUM_OF_ITEM] = {
	// kcode
	{
		RS_KCODE,
		KERNELCODEBIT,
		D_RSOPID_KCODE,
	},
	// syscall
	{
		RS_SYS_CALL,
		SYSTEMCALLBIT,
		D_RSOPID_SYS_CALL,
	},
	// selinux
	{
		RS_SE_STATUS,
		SESTATUSBIT,
		D_RSOPID_SE_STATUS,
	},
	// se_hook
	{
		RS_SE_HOOKS,
		SEHOOKBIT,
		D_RSOPID_SE_HOOKS,
	},
	// root_proc
	{
		RS_RRPOCS,
		ROOTPROCBIT,
		D_RSOPID_RRPOCS,
	},
	// set_id
	{
		RS_SETID,
		SETIDBIT,
		D_RSOPID_SETID,
	},
};

struct rscan_skip_flags g_rscan_skip_flag = {
	.skip_kcode = NOT_SKIP,
	.skip_syscall = NOT_SKIP,
	.skip_se_hooks = NOT_SKIP,
	.skip_se_status = NOT_SKIP,
	.skip_rprocs = NOT_SKIP,
	.skip_setid = NOT_SKIP,
};

static struct rscan_result_dynamic g_rscan_clean_scan_result;
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static int g_r_p_flag;
static int g_test_count;
static struct rscan_result_dynamic g_rscan_orig_result;
#define FAULT_PRIVATE_LENTH 500
struct fault_private {
	size_t len;
	char buf[FAULT_PRIVATE_LENTH];
};
#endif

bool get_xom_enable(void)
{
#ifdef CONFIG_HKIP_XOM_CODE
	return true;
#else
	return false;
#endif
}

static uint rscan_trigger_by_stp(char *upload_rootproc, int upload_rootproc_len)
{
	int scan_err_code = 0;
	uint root_masks;
	int dynamic_ops;
	int root_proc_length;
	int ret;
	struct rscan_result_dynamic *scan_result_buf = NULL;

	scan_result_buf = vmalloc(sizeof(struct rscan_result_dynamic));
	if (scan_result_buf == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "no enough space for scan_result_buf");
		return -ENOSPC;
	}

	ret = memset_s(scan_result_buf, sizeof(struct rscan_result_dynamic),
			0, sizeof(struct rscan_result_dynamic));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		vfree(scan_result_buf);
		return -ENOMEM;
	}

	dynamic_ops = RSOPID_ALL;
	mutex_lock(&scanner_lock);
	root_masks = rscan_dynamic(dynamic_ops, scan_result_buf, &scan_err_code);
	mutex_unlock(&scanner_lock);
	if (root_masks != 0)
		rs_log_debug(RSCAN_LOG_TAG, "root status trigger by stp is %u", root_masks);

	if ((upload_rootproc != NULL) &&
		(strlen(scan_result_buf->rprocs) > 0)) {
		root_proc_length = strnlen(scan_result_buf->rprocs,
					sizeof(scan_result_buf->rprocs));
		if (root_proc_length >= RPROC_VALUE_LEN_MAX) {
			root_proc_length = RPROC_VALUE_LEN_MAX - 1;
			scan_result_buf->rprocs[root_proc_length] = '\0';
		}

		ret = strncpy_s(upload_rootproc, upload_rootproc_len,
				scan_result_buf->rprocs, RPROC_VALUE_LEN_MAX);
		if (ret != EOK) {
			rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
			vfree(scan_result_buf);
			return -ENOMEM;
		}
	}

	vfree(scan_result_buf);

	return root_masks;
}

static int get_credible_of_item(int item_ree_status, int item_tee_status)
{
	if ((item_ree_status == 0) && (item_tee_status == 1)) {
		return STP_REFERENCE;
	} else {
		return STP_CREDIBLE;
	}
}

static int need_to_upload(unsigned int masks, unsigned int mask,
			int ree_status, int tee_status, int flag)
{
	if (flag == 1)
		return 1;

	if ((masks & mask) && ((ree_status != 0) || (tee_status != 0)))
		return 1;

	return 0;
}

static void get_kcode_info(int *item_credible,
				int *item_status,
				int item_tee_status, int idx)
{
	bool xom_enable = false;

	if ((idx == KCODE) && (*item_credible == STP_REFERENCE) &&
		(g_root_scan_hot_fix != 0))
		*item_credible = STP_CREDIBLE;

	xom_enable = get_xom_enable();
	if ((xom_enable == true) && (idx == KCODE)) {
		*item_status = item_tee_status;
		*item_credible = STP_CREDIBLE;
	}
}

/* flag = 0, just upload the abnormal items; flag = 1, upload all items */
static void upload_to_stp(uint ree_status, uint tee_status,
			const char *rootproc, unsigned int mask, int flag)
{
	int item_status;
	int item_version = 0;
	int item_credible;
	int item_tee_status;
	int need_upload;
	int i;
	int ret;

	const struct stp_item_info *stp_item_info = NULL;
	struct stp_item item = { 0 };

	for (i = 0; i < MAX_NUM_OF_ITEM; i++) {
		item_status = check_status(ree_status,
					itembits[i].item_ree_bit);
		item_tee_status = check_status(tee_status,
					itembits[i].item_tee_bit);
		need_upload = need_to_upload(mask, itembits[i].item_ree_mask,
					item_status, item_tee_status, flag);
		if (need_upload == 0)
			continue;
		stp_item_info = get_item_info_by_idx(i);
		if (stp_item_info == NULL) {
			rs_log_error(RSCAN_LOG_TAG, "idx is %d, get item info by index failed", i);
			return;
		}

		item_credible = get_credible_of_item(item_status, item_tee_status);
		if ((i == ROOT_PROCS) || (i == SE_HOOK))
			item_credible = STP_REFERENCE;

		get_kcode_info(&item_credible, &item_status, item_tee_status, i);

		item.id = stp_item_info->id;
		item.status = item_status;
		item.credible = item_credible;
		item.version = item_version;

		if (strlen(stp_item_info->name) >= STP_ITEM_NAME_LEN) {
			rs_log_error(RSCAN_LOG_TAG,
				"the length of the item name [%s] has exceeded the max allowed value",
				stp_item_info->name);
			return;
		}

		ret = strncpy_s(item.name, STP_ITEM_NAME_LEN - 1,
				stp_item_info->name, STP_ITEM_NAME_LEN - 1);
		if (ret != EOK) {
			rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
			return;
		}

		item.name[STP_ITEM_NAME_LEN - 1] = '\0';

		if (i == ROOT_PROCS)
			(void)kernel_stp_upload(item, rootproc);
		else
			(void)kernel_stp_upload(item, NULL);
	}

	return;
}

int stp_rscan_trigger(void)
{
	uint ree_status;
	uint tee_status;
	char *upload_rootproc = NULL;

	upload_rootproc = kzalloc(RPROC_VALUE_LEN_MAX, GFP_KERNEL);
	if (upload_rootproc == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "failed to alloc upload_rootproc");
		return -ENOSPC;
	}

	ree_status = rscan_trigger_by_stp(upload_rootproc, RPROC_VALUE_LEN_MAX);
	tee_status = get_tee_status();

	/* 1 is flag, 1 mean upload all items */
	upload_to_stp(ree_status, tee_status, upload_rootproc, RSOPID_ALL, 1);

	kfree(upload_rootproc);

	return 0;
}

static void scan_rprocs(struct rscan_result_dynamic *result, uint *error_code)
{
	int ret;
#ifdef CONFIG_RSCAN_SKIP_RPROCS
	ret = strncpy_s(result->rprocs, sizeof(result->rprocs),
			DEFAULT_PROC, strlen(DEFAULT_PROC) + 1);
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
		*error_code |= D_RSOPID_RRPOCS;
		rs_log_error(RSCAN_LOG_TAG, "root processes scan failed!");
		return;
	}
#else
	ret = get_root_procs(result->rprocs, sizeof(result->rprocs));
	if (ret == 0) {
		*error_code |= D_RSOPID_RRPOCS;
		rs_log_error(RSCAN_LOG_TAG, "root processes scan failed!");
	}
#endif
}

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static void print_scan_item_info(void)
{
	if (g_r_p_flag == 1) {
		if (g_rscan_skip_flag.skip_kcode == SKIP)
			rs_log_debug(RSCAN_LOG_TAG, "skip kcode scan");
		if (g_rscan_skip_flag.skip_syscall == SKIP)
			rs_log_debug(RSCAN_LOG_TAG, "skip syscall scan");
		if (g_rscan_skip_flag.skip_se_hooks == SKIP)
			rs_log_debug(RSCAN_LOG_TAG, "skip se hooks scan");
		if (g_rscan_skip_flag.skip_se_status == SKIP)
			rs_log_debug(RSCAN_LOG_TAG, "skip se status scan");
		if (g_rscan_skip_flag.skip_setid == SKIP)
			rs_log_debug(RSCAN_LOG_TAG, "skip setid scan");
	}
}
#endif

static uint rscan_dynamic_raw_unlock(uint op_mask,
				struct rscan_result_dynamic *result)
{
	int ret = 0;
	uint error_code = 0;

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	print_scan_item_info();
#endif

	if (op_mask & D_RSOPID_KCODE) {
		ret = kcode_scan(result->kcode, sizeof(result->kcode));
		if (ret != 0) {
			error_code |= D_RSOPID_KCODE;
			rs_log_error(RSCAN_LOG_TAG, "kcode_scan failed");
		}
	}

	if (op_mask & D_RSOPID_SYS_CALL) {
		ret = kcode_syscall_scan(result->syscalls,
					sizeof(result->syscalls));
		if (ret != 0) {
			error_code |= D_RSOPID_SYS_CALL;
			rs_log_error(RSCAN_LOG_TAG, "kcode system call scan failed");
		}
	}

	if (op_mask & D_RSOPID_SE_HOOKS) {
		ret = sescan_hookhash(result->sehooks,
				sizeof(result->sehooks));
		if (ret != 0) {
			error_code |= D_RSOPID_SE_HOOKS;
			rs_log_error(RSCAN_LOG_TAG, "sescan_hookhash scan failed");
		}
	}

	if (op_mask & D_RSOPID_SE_STATUS)
		result->seenforcing = get_selinux_enforcing();

	if (op_mask & D_RSOPID_RRPOCS)
		scan_rprocs(result, &error_code);

	if (op_mask & D_RSOPID_SETID)
		result->setid = get_setids();

	return error_code;
}

static uint rscan_get_dynamic_bad_mask(uint op_mask, struct rscan_result_dynamic *result)
{
	uint bad_mask = 0;

	if ((op_mask & D_RSOPID_KCODE) &&
		(g_rscan_skip_flag.skip_kcode == NOT_SKIP) &&
		(memcmp(result->kcode, g_rscan_clean_scan_result.kcode,
					sizeof(result->kcode)) != 0)) {
		bad_mask |= D_RSOPID_KCODE;
		rs_log_debug(RSCAN_LOG_TAG, "kernel code is abnormal");
	}

	if ((op_mask & D_RSOPID_SYS_CALL) &&
		(g_rscan_skip_flag.skip_syscall == NOT_SKIP) &&
		(memcmp(result->syscalls, g_rscan_clean_scan_result.syscalls,
			sizeof(result->syscalls)) != 0)) {
		bad_mask |= D_RSOPID_SYS_CALL;
		rs_log_debug(RSCAN_LOG_TAG, "kernel system call is abnormal");
	}

	if ((op_mask & D_RSOPID_SE_HOOKS) &&
		(g_rscan_skip_flag.skip_se_hooks == NOT_SKIP) &&
		(memcmp(result->sehooks, g_rscan_clean_scan_result.sehooks,
			sizeof(result->sehooks)) != 0)) {
		bad_mask |= D_RSOPID_SE_HOOKS;
		rs_log_debug(RSCAN_LOG_TAG, "SeLinux hooks is abnormal");
	}

	if ((op_mask & D_RSOPID_SE_STATUS) &&
			(g_rscan_skip_flag.skip_se_status == NOT_SKIP) &&
			(result->seenforcing !=
			 g_rscan_clean_scan_result.seenforcing)) {
		bad_mask |= D_RSOPID_SE_STATUS;
		rs_log_debug(RSCAN_LOG_TAG, "SeLinux enforcing status is abnormal");
	}

	if ((op_mask & D_RSOPID_RRPOCS) &&
			(g_rscan_skip_flag.skip_rprocs == NOT_SKIP)) {
		rprocs_strip_trustlist(result->rprocs,
				(ssize_t)sizeof(result->rprocs));
		if (result->rprocs[0]) {
			bad_mask |= D_RSOPID_RRPOCS;
			rs_log_debug(RSCAN_LOG_TAG, "root processes are abnormal");
		}
	}

	if ((op_mask & D_RSOPID_SETID) &&
			(g_rscan_skip_flag.skip_se_status == NOT_SKIP) &&
			(result->setid != g_rscan_clean_scan_result.setid)) {
		bad_mask |= D_RSOPID_SETID;
		rs_log_debug(RSCAN_LOG_TAG, "setid status is abnormal");
	}

	return bad_mask;
}

/* return: mask of abnormal scans items result */
uint rscan_dynamic(uint op_mask, struct rscan_result_dynamic *result,
		uint *error_code)
{
	uint bad_mask;

	if ((result == NULL) || (error_code == NULL)) {
		rs_log_error(RSCAN_LOG_TAG, "input parameters error!");
		return 0;
	}

	*error_code = rscan_dynamic_raw_unlock(op_mask, result);
	if (*error_code != 0)
		rs_log_warning(RSCAN_LOG_TAG, "some item of root scan failed");

	bad_mask = rscan_get_dynamic_bad_mask(op_mask, result);
	rs_log_trace(RSCAN_LOG_TAG, "root scan finished");
	return bad_mask;
}

/* just get the measurement, return the error mask */
uint rscan_dynamic_raw(uint op_mask, struct rscan_result_dynamic *result)
{
	uint error_code;

	if (result == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "input parameter is invalid");
		return -EINVAL;
	}

	mutex_lock(&scanner_lock);
	error_code = rscan_dynamic_raw_unlock(op_mask, result);
	mutex_unlock(&scanner_lock);

	return error_code;
}

/* call by CA to send dynamic measurement and upload abnormal item */
int rscan_dynamic_raw_and_upload(uint op_mask,
				struct rscan_result_dynamic *result)
{
	uint ree_status;
	uint tee_status;
	uint error_code = 0;

	if (result == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "input parameter is invalid");
		return -EINVAL;
	}

	mutex_lock(&scanner_lock);
	ree_status = rscan_dynamic(op_mask, result, &error_code);
	mutex_unlock(&scanner_lock);

	tee_status = get_tee_status();
	/* 0 in upload_to_stp mean just upload abnormal items */
	if ((ree_status != 0) || (tee_status != 0))
		upload_to_stp(ree_status, tee_status, NULL, op_mask, 0);

	return error_code;
}

#ifdef CONFIG_ARCH_MSM
int get_battery_status(int *is_charging, int *percentage)
{
	union power_supply_propval status;
	union power_supply_propval capacity;
	struct power_supply *psy = power_supply_get_by_name(BATTERY_NAME);

	if (psy == NULL)
		return -EINVAL;

	if (is_charging &&
		!psy->get_property(psy, POWER_SUPPLY_PROP_STATUS, &status))
		*is_charging = (status.intval == POWER_SUPPLY_STATUS_CHARGING) ||
				(status.intval == POWER_SUPPLY_STATUS_FULL);

	if (percentage &&
		!psy->get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &capacity))
		*percentage = capacity.intval;

	return 0;
}
#else
int get_battery_status(int *is_charging, int *percentage)
{
	union power_supply_propval status;
	union power_supply_propval capacity;
	struct power_supply *psy = NULL;

	if ((is_charging == NULL) || (percentage == NULL)) {
		rs_log_error(RSCAN_LOG_TAG, "input parameters invalid");
		return -EINVAL;
	}

	psy = power_supply_get_by_name(BATTERY_NAME);
	if (psy == NULL)
		return -1;

	/* is_charging never be NULL because of input parameters check */
	if (!power_supply_get_property(psy, POWER_SUPPLY_PROP_STATUS, &status))
		*is_charging = (status.intval == POWER_SUPPLY_STATUS_CHARGING) ||
				(status.intval == POWER_SUPPLY_STATUS_FULL);

	/* percentage never be NULL because of input parameters check */
	if (!power_supply_get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &capacity))
		*percentage = capacity.intval;

	return 0;
}
#endif

int rscan_get_status(struct rscan_status *status)
{
	int is_charging = 0;
	int percentage = 0;
	int result = 0;
	struct timespec64 ts;

	if (status == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "input parameter is invalid");
		return -1;
	}

	status->cpuload = 0;

	if (get_battery_status(&is_charging, &percentage) == 0) {
		status->battery  = (uint32_t)percentage;
		status->charging = (uint32_t)is_charging;
	} else {
		rs_log_warning(RSCAN_LOG_TAG, "rootscan: get_battery_status failed");
		status->battery  = 0;
		status->charging = 0;
		result = 1;
	}

	ktime_get_real_ts64(&ts);
	status->time = (uint32_t)ts.tv_sec;
	status->timezone = (uint32_t)sys_tz.tz_minuteswest;

	return result;
}

int load_rproc_trustlist(char *trustlist, size_t len)
{
	size_t min_len = strlen(g_trustlist_proc);
	int ret;

	if (trustlist == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "input parameter is invalid");
		return -EINVAL;
	}

	if (min_len >= len) {
		rs_log_warning(RSCAN_LOG_TAG, "The g_trustlist_proc lenth is too long");
		return -1;
	} else if (min_len <= 0) {
		rs_log_warning(RSCAN_LOG_TAG, "g_trustlist_proc is null");
		return -1;
	} else {
		ret = strncpy_s(trustlist, len, g_trustlist_proc, min_len);
		if (ret != EOK) {
			rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
			return -1;
		}
		trustlist[min_len] = '\0';
	}

	return 0;
}

int rscan_init_data(void)
{
	int ret;
	uint error_code;

	/* initialize g_rscan_clean_scan_result */
	ret = memset_s(&g_rscan_clean_scan_result, sizeof(struct rscan_result_dynamic),
			0, sizeof(struct rscan_result_dynamic));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		return -1;
	}

	/* 1:selinux closed  0:selinux opened */
	g_rscan_clean_scan_result.seenforcing = 1;
	g_rscan_clean_scan_result.setid = get_setids();

	ret = load_rproc_trustlist(g_rscan_clean_scan_result.rprocs,
				sizeof(g_rscan_clean_scan_result.rprocs));
	if ((ret != 0) || (!init_rprocs_trustlist(g_rscan_clean_scan_result.rprocs))) {
		rs_log_error(RSCAN_LOG_TAG, "load root trustlist failed, rproc will skip");
		if (sizeof(g_rscan_clean_scan_result.rprocs) < strlen(DEFAULT_PROC)) {
			rs_log_error(RSCAN_LOG_TAG, "%s length is overlong", DEFAULT_PROC);
			return -1;
		}

		ret = strncpy_s(g_rscan_clean_scan_result.rprocs,
				MAX_RPROC_SIZE, DEFAULT_PROC,
				strlen(DEFAULT_PROC) + 1);
		if (ret != EOK) {
			rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
			return -1;
		}

		g_rscan_skip_flag.skip_rprocs = SKIP;
	}

	error_code = rscan_dynamic_raw(D_RSOPID_KCODE |
				D_RSOPID_SYS_CALL |
				D_RSOPID_SE_HOOKS,
				&g_rscan_clean_scan_result);
	if (error_code != 0) {
		if (error_code & D_RSOPID_KCODE) {
			rs_log_error(RSCAN_LOG_TAG, "rscan D_RSOPID_KCODE init failed");
			g_rscan_skip_flag.skip_kcode = SKIP;
		}

		if (error_code & D_RSOPID_SYS_CALL) {
			rs_log_error(RSCAN_LOG_TAG, "rscan D_RSOPID_SYS_CALL init failed");
			g_rscan_skip_flag.skip_syscall = SKIP;
		}

		if (error_code & D_RSOPID_SE_HOOKS) {
			rs_log_error(RSCAN_LOG_TAG, "rscan D_RSOPID_SE_HOOKS init failed");
			g_rscan_skip_flag.skip_se_hooks = SKIP;
		}
	}

	g_rs_data_init = RSCAN_INIT;
	return 0;
}

int rscan_trigger(void)
{
	int result = stp_rscan_trigger();

	rs_log_trace(RSCAN_LOG_TAG, "scan and upload finished. result: %d", result);
	return result;
}

static uint dynamic_call(unsigned int mask)
{
	uint root_status;

	if (mask & D_RSOPID_KCODE)
		g_rscan_skip_flag.skip_kcode = NOT_SKIP;

	if (mask & D_RSOPID_SYS_CALL)
		g_rscan_skip_flag.skip_syscall = NOT_SKIP;

	if (mask & D_RSOPID_SE_HOOKS)
		g_rscan_skip_flag.skip_se_hooks = NOT_SKIP;

	if (mask & D_RSOPID_SE_STATUS)
		g_rscan_skip_flag.skip_se_status = NOT_SKIP;

	root_status = rscan_dynamic_raw_unlock(mask,
						&g_rscan_clean_scan_result);

	rs_log_debug(RSCAN_LOG_TAG, "set %u scan resume", mask);

	return root_status;
}

/* @reserved is reserved parameters for external module */
static int __root_scan_pause(unsigned int op_mask, const void *reserved)
{
	var_not_used(reserved);

	g_rscan_skip_flag.skip_kcode     = SKIP &
			((op_mask & D_RSOPID_KCODE) >> KCODE_OFFSET);
	g_rscan_skip_flag.skip_syscall   = SKIP &
			((op_mask & D_RSOPID_SYS_CALL) >> SYSCALL_OFFSET);
	g_rscan_skip_flag.skip_se_hooks  = SKIP & ((op_mask &
			D_RSOPID_SE_HOOKS) >> SEHOOKS_OFFSET);
	g_rscan_skip_flag.skip_se_status = SKIP &
			((op_mask & D_RSOPID_SE_STATUS) >> SESTATUS_OFFSET);

	rs_log_debug(RSCAN_LOG_TAG, "set scan pause, pause mask %u", op_mask);

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_r_p_flag = 1;
#endif

	return 0;
}

/* @reserved is reserved parameters for external module */
static int __root_scan_resume(unsigned int op_mask, void *reserved)
{
	unsigned int resume_mask = 0;

	var_not_used(reserved);
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_r_p_flag = 0;
#endif

	if ((op_mask & D_RSOPID_KCODE) &&
			(g_rscan_skip_flag.skip_kcode == SKIP))
		resume_mask |= D_RSOPID_KCODE;

	if ((op_mask & D_RSOPID_SYS_CALL) &&
			(g_rscan_skip_flag.skip_syscall == SKIP))
		resume_mask |= D_RSOPID_SYS_CALL;

	if ((op_mask & D_RSOPID_SE_HOOKS) &&
			(g_rscan_skip_flag.skip_se_hooks == SKIP))
		resume_mask |= D_RSOPID_SE_HOOKS;

	if ((op_mask & D_RSOPID_SE_STATUS) &&
			(g_rscan_skip_flag.skip_se_status == SKIP))
		resume_mask |= D_RSOPID_SE_STATUS;

	return dynamic_call(resume_mask);
}

/* @reserved is reserved parameters for external module */
int root_scan_pause(unsigned int op_mask, const void *reserved)
{
	int scan_err_code = 0;
	int result = 0;
	int root_status;
	int dynamic_ops;
	struct rscan_result_dynamic *scan_result_buf = NULL;
	struct timespec64 ts;

	var_not_used(reserved);

	ktime_get_real_ts64(&ts);
	rs_log_trace(RSCAN_LOG_TAG, "pause item:%u, time:%ld:%ld",
		op_mask, ts.tv_sec, ts.tv_nsec / 1000);

	mutex_lock(&scanner_lock);
	scan_result_buf = vmalloc(sizeof(struct rscan_result_dynamic));
	if (scan_result_buf == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "no enough space for scan_result_buf");
		mutex_unlock(&scanner_lock);
		return -ENOSPC;
	}

	result = memset_s(scan_result_buf, sizeof(struct rscan_result_dynamic),
			0, sizeof(struct rscan_result_dynamic));
	if (result != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		mutex_unlock(&scanner_lock);
		vfree(scan_result_buf);
		return -1;
	}

	/* do scan before pause rootscan */
	dynamic_ops = D_RSOPID_KCODE | D_RSOPID_SYS_CALL |
			D_RSOPID_SE_HOOKS | D_RSOPID_SE_STATUS;
	root_status = rscan_dynamic(dynamic_ops, scan_result_buf, &scan_err_code);
	if (root_status == 0) {
		rs_log_trace(RSCAN_LOG_TAG, "environment clean, pause root scan go");
		result = __root_scan_pause(op_mask, reserved);
	} else {
		rs_log_trace(RSCAN_LOG_TAG, "already rooted, skip pause");
		result = root_status;
	}
	mutex_unlock(&scanner_lock);

	/* scan_result_buf never evaluates to NULL */
	vfree(scan_result_buf);

	return result;
}

/* @reserved is reserved parameters for external module */
int root_scan_resume(unsigned int op_mask, void *reserved)
{
	struct timespec64 ts;
	int result;

	var_not_used(reserved);

	g_root_scan_hot_fix = 1;    /* have been done HotFix */

	ktime_get_real_ts64(&ts);
	rs_log_trace(RSCAN_LOG_TAG, "resume item:%u, time:%ld:%ld",
		op_mask, ts.tv_sec, ts.tv_nsec / 1000);

	mutex_lock(&scanner_lock);
	result = __root_scan_resume(op_mask, reserved);
	mutex_unlock(&scanner_lock);
	return result;
}

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static ssize_t copy_private_data_to_user(struct file *file,
				char __user *buf, size_t count, loff_t *offp)
{
	struct fault_private *priv = file->private_data;
	size_t tocopy = 0;

	if (priv == NULL) {
		rs_log_error(RSCAN_LOG_TAG, "priv is NULL");
		return -ENOMEM;
	}

	if (*offp < priv->len) {
		tocopy = ((priv->len - *offp) < count) ? (priv->len - *offp) : count;

		if (copy_to_user(buf, priv->buf + *offp, tocopy)) {
			rs_log_error(RSCAN_LOG_TAG, "copy_to_user failed");
			return -ENOMEM;
		}

		*offp += tocopy;
	}
	return tocopy;
}

static int open_private_file(struct file *file, const char *str_buf)
{
	struct fault_private *priv = NULL;
	int ret;

	WARN_ON(file->private_data);
	priv = vmalloc(sizeof(struct fault_private));
	if (priv == NULL)
		return -ENOMEM;
	file->private_data = priv;

	ret = memset_s(priv, sizeof(struct fault_private),
			0, sizeof(struct fault_private));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		vfree(priv);
		priv = NULL;
		file->private_data = NULL;
		return -ENOMEM;
	}

	priv->len = strlen(str_buf);
	if (priv->len >= FAULT_PRIVATE_LENTH) {
		vfree(priv);
		priv = NULL;
		file->private_data = NULL;
		return -EINVAL;
	}

	ret = strncpy_s(priv->buf, sizeof(priv->buf), str_buf, priv->len);
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "strncpy_s fail\n");
		vfree(priv);
		priv = NULL;
		file->private_data = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int kcode_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "kernel code scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan kernel code result:%d", ret);
	return ret;
}

static ssize_t kcode_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	int ret;

	ret = memset_s(g_rscan_clean_scan_result.kcode,
			sizeof(g_rscan_clean_scan_result.kcode),
			0, sizeof(g_rscan_clean_scan_result.kcode));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "rootscan: dead_code injected\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int kcode_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "xxx. kcode_release succ!");
	return 0;
}

static int rs_pause_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rs_pause_test_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int test = 0;
	int pause_result = root_scan_pause(D_RSOPID_KCODE | D_RSOPID_SYS_CALL |
				D_RSOPID_SE_HOOKS | D_RSOPID_SE_STATUS, &test);
	g_test_count++;
	rs_log_debug(RSCAN_LOG_TAG, "root scan pause check, count:%d, result:%d",
			g_test_count, pause_result);

	return 0;
}

static int rs_pause_test_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "xxx. pause release succ!");
	return 0;
}

static int rs_resume_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rs_resume_test_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int test = 0;
	int resume_result = root_scan_resume((D_RSOPID_KCODE |
					D_RSOPID_SYS_CALL |
					D_RSOPID_SE_HOOKS |
					D_RSOPID_SE_STATUS), &test);
	g_test_count++;
	rs_log_debug(RSCAN_LOG_TAG, "root scan resume check, count%d, result:%d",
		g_test_count, resume_result);

	return 0;
}

static int rs_resume_test_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "xxx. resume release succ!");
	return 0;
}

/* revert kcode test items */
static int rev_kcode_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert kernel code scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan revert kernel code result:%d", ret);
	return ret;
}

static ssize_t rev_kcode_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	int ret;

	ret = memcpy_s(g_rscan_clean_scan_result.kcode,
			sizeof(g_rscan_clean_scan_result.kcode),
			g_rscan_orig_result.kcode,
			sizeof(g_rscan_orig_result.kcode));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memcpy_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "revert rootscan: dead_code reverted\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_kcode_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "3. rev_sehooks_release succ!");
	return 0;
}

static int se_enforcing_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "se_enforcing scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan se_enforcing result:%d", ret);
	return ret;
}

static ssize_t se_enforcing_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	g_rscan_clean_scan_result.seenforcing = 0;
	rs_log_debug(RSCAN_LOG_TAG, "root scan check, se_enforcing broken");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int se_enforcing_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "8. se enforcing release succ!");
	return 0;
}

static int syscall_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "system call scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan system call result:%d", ret);
	return ret;
}
static ssize_t syscall_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	int ret;

	ret = memset_s(g_rscan_clean_scan_result.syscalls,
			sizeof(g_rscan_clean_scan_result.syscalls),
			0, sizeof(g_rscan_clean_scan_result.syscalls));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "rootscan: syscall injected\n");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int syscall_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "7. syscall_release succ!");
	return 0;
}

/* set selinux hooks to all 0, for test */
static int sehooks_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "se_hooks scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan se_hooks result:%d", ret);
	return ret;
}

static ssize_t sehooks_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{
	int ret;

	ret = memset_s(g_rscan_clean_scan_result.sehooks,
			sizeof(g_rscan_clean_scan_result.sehooks),
			0, sizeof(g_rscan_clean_scan_result.sehooks));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memset_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "rootscan: sec->sem_semctl injected");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int sehooks_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "5. sehooks_release succ!");
	return 0;
}

/* revert syscall test items */
static int rev_syscall_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert system call scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan revert system call result:%d", ret);
	return ret;
}
static ssize_t rev_syscall_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int ret;

	ret = memcpy_s(g_rscan_clean_scan_result.syscalls,
			sizeof(g_rscan_clean_scan_result.syscalls),
			g_rscan_orig_result.syscalls,
			sizeof(g_rscan_orig_result.syscalls));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memcpy_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "revert rootscan: syscall reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_syscall_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "4. rev_syscall_release succ!");
	return 0;
}

/* revert selinux test items make selinux hooks data original */
static int rev_sehooks_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert se_hooks scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan revert se_hooks result:%d", ret);
	return ret;
}

static ssize_t rev_sehooks_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	int ret;

	ret = memcpy_s(g_rscan_clean_scan_result.sehooks,
			sizeof(g_rscan_clean_scan_result.sehooks),
			g_rscan_orig_result.sehooks,
			sizeof(g_rscan_orig_result.sehooks));
	if (ret != EOK) {
		rs_log_error(RSCAN_LOG_TAG, "memcpy_s fail\n");
		return -1;
	}

	rs_log_debug(RSCAN_LOG_TAG, "revert root scan check, sehooks reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_sehooks_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "2. rev_sehooks_release succ!");
	return 0;
}

/* revert selinux status test items */
static int rev_se_enforcing_open(struct inode *inode, struct file *file)
{
	int ret = open_private_file(file, "revert se_enforcing scan testing\n");

	rs_log_debug(RSCAN_LOG_TAG, "root scan revert se_enforcing result:%d", ret);
	return ret;
}

static ssize_t rev_se_enforcing_read(struct file *file, char __user *buf,
				size_t count, loff_t *offp)
{
	g_rscan_clean_scan_result.seenforcing = 1;
	rs_log_debug(RSCAN_LOG_TAG, "revert root scan check, se_enforcing reverted");
	return copy_private_data_to_user(file, buf, count, offp);
}

static int rev_se_enforcing_release(struct inode *inode, struct file *file)
{
	if (file->private_data != NULL) {
		vfree(file->private_data);
		file->private_data = NULL;
	}
	rs_log_debug(RSCAN_LOG_TAG, "1. rev_se_enforcing_release succ!");
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
static const struct file_operations rs_pause_test_fops = {
	.owner = THIS_MODULE,
	.open  = rs_pause_test_open,
	.read  = rs_pause_test_read,
	.release = rs_pause_test_release,
};

static const struct file_operations rs_resume_test_fops = {
	.owner = THIS_MODULE,
	.open  = rs_resume_test_open,
	.read  = rs_resume_test_read,
	.release = rs_resume_test_release,
};

static const struct file_operations kcode_fops = {
	.owner = THIS_MODULE,
	.open  = kcode_open,
	.read  = kcode_read,
	.release = kcode_release,
};

static const struct file_operations rev_kcode_fops = {
	.owner = THIS_MODULE,
	.open  = rev_kcode_open,
	.read  = rev_kcode_read,
	.release = rev_kcode_release,
};

static const struct file_operations se_enforcing_fops = {
	.owner = THIS_MODULE,
	.open  = se_enforcing_open,
	.read  = se_enforcing_read,
	.release = se_enforcing_release,
};

static const struct file_operations syscall_fops = {
	.owner = THIS_MODULE,
	.open  = syscall_open,
	.read  = syscall_read,
	.release = syscall_release,
};

static const struct file_operations sehooks_fops = {
	.owner = THIS_MODULE,
	.open  = sehooks_open,
	.read  = sehooks_read,
	.release = sehooks_release,
};

static const struct file_operations rev_syscall_fops = {
	.owner = THIS_MODULE,
	.open  = rev_syscall_open,
	.read  = rev_syscall_read,
	.release = rev_syscall_release,
};

static const struct file_operations rev_sehooks_fops = {
	.owner = THIS_MODULE,
	.open  = rev_sehooks_open,
	.read  = rev_sehooks_read,
	.release = rev_sehooks_release,
};

static const struct file_operations rev_se_enforcing_fops = {
	.owner = THIS_MODULE,
	.open  = rev_se_enforcing_open,
	.read  = rev_se_enforcing_read,
	.release = rev_se_enforcing_release,
};
#else // LINUX_VERSION_CODE
static const struct proc_ops rs_pause_test_fops = {
	.proc_open  = rs_pause_test_open,
	.proc_read  = rs_pause_test_read,
	.proc_release = rs_pause_test_release,
};

static const struct proc_ops rs_resume_test_fops = {
	.proc_open  = rs_resume_test_open,
	.proc_read  = rs_resume_test_read,
	.proc_release = rs_resume_test_release,
};

static const struct proc_ops kcode_fops = {
	.proc_open  = kcode_open,
	.proc_read  = kcode_read,
	.proc_release = kcode_release,
};

static const struct proc_ops rev_kcode_fops = {
	.proc_open  = rev_kcode_open,
	.proc_read  = rev_kcode_read,
	.proc_release = rev_kcode_release,
};

static const struct proc_ops se_enforcing_fops = {
	.proc_open  = se_enforcing_open,
	.proc_read  = se_enforcing_read,
	.proc_release = se_enforcing_release,
};

static const struct proc_ops syscall_fops = {
	.proc_open  = syscall_open,
	.proc_read  = syscall_read,
	.proc_release = syscall_release,
};

static const struct proc_ops sehooks_fops = {
	.proc_open  = sehooks_open,
	.proc_read  = sehooks_read,
	.proc_release = sehooks_release,
};

static const struct proc_ops rev_syscall_fops = {
	.proc_open  = rev_syscall_open,
	.proc_read  = rev_syscall_read,
	.proc_release = rev_syscall_release,
};

static const struct proc_ops rev_sehooks_fops = {
	.proc_open  = rev_sehooks_open,
	.proc_read  = rev_sehooks_read,
	.proc_release = rev_sehooks_release,
};

static const struct proc_ops rev_se_enforcing_fops = {
	.proc_open  = rev_se_enforcing_open,
	.proc_read  = rev_se_enforcing_read,
	.proc_release = rev_se_enforcing_release,
};
#endif // LINUX_VERSION_CODE
#endif // CONFIG_HW_ROOT_SCAN_ENG_DEBUG

int rscan_dynamic_init(void)
{
	if (rscan_init_data() != 0) {
		rs_log_error(RSCAN_LOG_TAG, "rootscan: rscan init data failed");
		return RSCAN_ERR_SCANNER_INIT;
	}
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	g_rscan_orig_result = g_rscan_clean_scan_result;
	/* 0777 is the permissions format of file, Read, write and excute */
	proc_create("rs_pause_test",  0777, NULL, &rs_pause_test_fops);
	proc_create("rs_resume_test", 0777, NULL, &rs_resume_test_fops);

	proc_create("rev_sehooks",      0777, NULL, &rev_sehooks_fops);
	proc_create("rev_kcode",        0777, NULL, &rev_kcode_fops);
	proc_create("rev_syscall",      0777, NULL, &rev_syscall_fops);
	proc_create("rev_se_enforcing", 0777, NULL, &rev_se_enforcing_fops);

	proc_create("rs_se_enforcing", 0777, NULL, &se_enforcing_fops);
	proc_create("rs_sehooks",      0777, NULL, &sehooks_fops);
	proc_create("rs_kcode",        0777, NULL, &kcode_fops);
	proc_create("rs_syscall",      0777, NULL, &syscall_fops);
#endif
	return 0;
}
