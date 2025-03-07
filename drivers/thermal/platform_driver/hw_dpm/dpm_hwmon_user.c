/*
 * dpm_hwmon_user.c
 *
 * dpm interface for user
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_drivers/dpm_hwmon_user.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <securec.h>

#if defined(CONFIG_DPM_HWMON_DEBUG) && defined(CONFIG_DEBUG_FS)
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/delay.h>
#endif

#if defined(CONFIG_DPM_HWMON_V1)
#include "dpm_hwmon_v1.h"
#elif defined(CONFIG_DPM_HWMON_V2)
#include "dpm_hwmon_v2.h"
#elif defined(CONFIG_DPM_HWMON_V3)
#include "dpm_hwmon_v3.h"
#endif

#define DPM_SWITCH_MASK		0x80000000
bool g_dpm_report_enabled = true;

static struct dpm_hwmon_ops *search_dpm_module(int dpm_id)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	mutex_lock(&g_dpm_hwmon_ops_list_lock);
	list_for_each_entry(tmp, &g_dpm_hwmon_ops_list, ops_list) {
		if (tmp->dpm_module_id == dpm_id) {
			pos = tmp;
			break;
		}
	}
	mutex_unlock(&g_dpm_hwmon_ops_list_lock);
	return pos;
}

unsigned int dpm_peri_num(void)
{
	struct dpm_hwmon_ops *pos = NULL;
	unsigned int count  = 0;

	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list)
		if (pos->dpm_type == DPM_PERI_MODULE)
			count++;

	return count;
}

unsigned long long get_dpm_chdmod_power(int dpm_id)
{
	struct dpm_hwmon_ops *pos = NULL;
	unsigned long long dpm_power;

	pos = search_dpm_module(dpm_id);
	dpm_power = get_dpm_power(pos);
	return dpm_power;
}
#ifdef CONFIG_DPM_HWMON_V3
unsigned long long get_dpm_part_power(int dpm_id, unsigned int start,
				      unsigned int end)
{
	struct dpm_hwmon_ops *pos = NULL;
	unsigned long long dpm_power;

	pos = search_dpm_module(dpm_id);
	dpm_power = get_part_power(pos, start, end);
	return dpm_power;
}
#endif

bool check_dpm_enabled(int dpm_type)
{
	struct dpm_hwmon_ops *pos = NULL;
	bool enabled = false;

	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		if (pos->dpm_type == dpm_type) {
			enabled = get_dpm_enabled(pos);
			if (enabled)
				pr_debug("%s:%d:dpm is true.\n", __func__, __LINE__);
			else
				pr_debug("%s:%d:dpm is false.\n", __func__, __LINE__);
			return enabled;
		}
	}
	return false;
}

void dpm_parse_switch_cmd(unsigned int cmd)
{
	struct dpm_hwmon_ops *pos = NULL;
	bool dpm_switch = ((cmd & DPM_SWITCH_MASK) != 0);
	bool peri_enabled = ((cmd & BIT(DPM_PERI_MODULE)) != 0);
	bool gpu_enabled = ((cmd & BIT(DPM_GPU_MODULE)) != 0);
	bool npu_enabled = ((cmd & BIT(DPM_NPU_MODULE)) != 0);
	bool cpu_enabled = ((cmd & BIT(DPM_CPU_MODULE)) != 0);

	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		switch (pos->dpm_type) {
		case DPM_PERI_MODULE:
			if (peri_enabled)
				dpm_enable_module(pos, dpm_switch);
			break;
		case DPM_GPU_MODULE:
			if (gpu_enabled)
				dpm_enable_module(pos, dpm_switch);
			break;
		case DPM_NPU_MODULE:
			if (npu_enabled)
				dpm_enable_module(pos, dpm_switch);
			break;
		case DPM_CPU_MODULE:
			if (cpu_enabled)
				dpm_enable_module(pos, dpm_switch);
			break;
		default:
			break;
		}
	}
}

int get_dpm_peri_data(struct dubai_transmit_t *peri_data, unsigned int peri_num)
{
	int ret;
	unsigned int count = 0;
	struct dpm_hwmon_ops *pos = NULL;
	struct ip_energy *dpm_ip = NULL;

	if (peri_data == NULL) {
		pr_err("%s peri data is null\n", __func__);
		return -EFAULT;
	}
	if (peri_num <= 0 || peri_num > DPM_MODULE_NUM) {
		pr_err("%s:%u, invalid peri_num!", __func__, peri_num);
		return -EFAULT;
	}

	list_for_each_entry(pos, &g_dpm_hwmon_ops_list, ops_list) {
		if (pos->dpm_type == DPM_PERI_MODULE && count < peri_num) {
			dpm_ip = (struct ip_energy *)(peri_data->data) + count;
			ret = strcpy_s(dpm_ip->name, DPM_NAME_SIZE,
				       dpm_module_table[pos->dpm_module_id]);
			if (ret != EOK) {
				pr_err("%s strcpy_s fail, ret is %d\n",
				       __func__, ret);
				return -EFAULT;
			}
			dpm_ip->energy = get_dpm_power(pos);
			count++;
		}
	}

	if (count != peri_num) {
		pr_err("peri num is not consistent %u vs %u!\n", count, peri_num);
		return -EFAULT;
	}

	peri_data->length = count * sizeof(struct ip_energy);
	return 0;
}

void update_dpm_power(int dpm_id)
{
	struct dpm_hwmon_ops *pos = NULL;

	pos = search_dpm_module(dpm_id);
	if (pos != NULL)
		(void)dpm_sample(pos);
}

#if defined(CONFIG_DPM_HWMON_DEBUG) && defined(CONFIG_DEBUG_FS)
#define MAX_DPM_LOG_SIZE	4096
#define DPM_WRITE_PARANUM	4
#define DPM_MAX_COUNT		1000000
#define DEBUGFS_BUF_LEN		32

unsigned long long g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];
static struct dentry *g_dpm_hwmon_debugfs_root;

static unsigned long long get_timer_value(void)
{
	unsigned long long total_us;
	struct timespec64 ts = (struct timespec64){ 0, 0 };

	ktime_get_real_ts64(&ts);
	total_us = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

	return total_us;
}

static void print_time_array(unsigned long long *array, int len)
{
	char buf[MAX_DPM_LOG_SIZE] = {'\0'};
	unsigned long long total_us;
	int i, ret;
	int offset = 0;

	if (array == NULL) {
		pr_err("%s: null pointer in dpm\n", __func__);
		return;
	}
	total_us = get_timer_value();
	ret = sprintf_s(buf, MAX_DPM_LOG_SIZE,
			"[DPM] time : %llu us reg: ", total_us);
	if (ret < 0) {
		pr_err("%s sprintf_s fail!\n", __func__);
		return;
	}

	offset += ret;
	for (i = 0; i < len; i++) {
		ret = sprintf_s(buf + offset, MAX_DPM_LOG_SIZE - offset,
				"%llu,", array[i]);
		if (ret < 0) {
			pr_err("%s sprintf_s fail!\n", __func__);
			return;
		}
		offset += ret;
	}
	if (offset > 0)
		pr_err("%s\n", buf);
}

static int dpm_debug_show(struct seq_file *m, void *v)
{
	unsigned int dpm_id;
	unsigned long long dpm_power;

	seq_printf(m, "g_dpm_report_enabled : %d\n", g_dpm_report_enabled);
	for (dpm_id = 0; dpm_id < ARRAY_SIZE(dpm_module_table); dpm_id++) {
		dpm_power = get_dpm_chdmod_power(dpm_id);
		seq_printf(m, "%s : %llu\n", dpm_module_table[dpm_id], dpm_power);
	}

	return 0;
}

static int dpm_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, dpm_debug_show, NULL);
}

static ssize_t dpm_debug_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *f_pos)
{
	int i;
	char buf[DEBUGFS_BUF_LEN] = {'\0'};
	int dpm_id = 0;
	int timer_span_ms = 0;
	int total_count = 0;
	int mode = 0;
	struct dpm_hwmon_ops *pos = NULL;

	if (user_buf == NULL) {
		pr_err("%s:user_buf is NULL!\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, min_t(size_t, sizeof(buf) - 1, count)) != 0) {
		pr_err("%s copy error!\n", __func__);
		return -EFAULT;
	}
	pr_err("%s: buf = %s\n", __func__, buf);

	if (sscanf_s(buf, "%d %d %d %d", &dpm_id, &timer_span_ms, &total_count, &mode) !=
	    DPM_WRITE_PARANUM) {
		pr_err("%s the num of enter is wrong!\n", __func__);
		return -EFAULT;
	}

	pr_err("%s: dpm_id = %d, timer_span_ms = %d, total_count = %d, mode = %d!\n",
	       __func__, dpm_id, timer_span_ms, total_count, mode);

	if (dpm_id >= (int)ARRAY_SIZE(dpm_module_table) || dpm_id < 0 ||
	    timer_span_ms <= 0 || total_count <= 0 ||
	    total_count > DPM_MAX_COUNT) {
		pr_err("illegal input!\n");
		return count;
	}

	pos = search_dpm_module(dpm_id);
	if (pos == NULL) {
		pr_err("dpm %d is NULL!\n", dpm_id);
		return count;
	}
#ifdef CONFIG_DPM_HWMON_PWRSTAT
	powerstat_config(pos, timer_span_ms, total_count, mode);
#else
	for (i = 0; i < total_count; i++) {
		mdelay(timer_span_ms);
		if (pos->hi_dpm_get_counter_for_fitting(mode) > 0) {
			print_time_array(pos->dpm_counter_table, pos->dpm_cnt_len);
#if defined(CONFIG_DPM_HWMON_V2)
			print_time_array(pos->dpm_power_table, pos->dpm_power_len);
#elif defined(CONFIG_DPM_HWMON_V3)
			print_time_array(pos->total_power_table, pos->dpm_power_len);
			print_time_array(pos->dyn_power_table, pos->dpm_power_len);
#endif
		}
	}
#ifdef CONFIG_DPM_HWMON_V2
	pr_err("dpm [%d] enabled is %d\n", dpm_id, pos->module_enabled);
#endif
#endif
	return count;
}

static const struct file_operations dpm_debug_fops = {
	.owner = THIS_MODULE,
	.open = dpm_debug_open,
	.read = seq_read,
	.write = dpm_debug_write,
	.llseek = seq_lseek,
	.release = single_release
};

static int dpm_hwmon_debugfs_init(void)
{
	struct dentry *dpm_hwmon_debug = NULL;

	pr_err("dpm %s\n", __func__);
	g_dpm_hwmon_debugfs_root = debugfs_create_dir("dpm_hwmon", NULL);
	if (g_dpm_hwmon_debugfs_root == NULL)
		return -ENOENT;

	dpm_hwmon_debug = debugfs_create_file("dpm_debug", S_IRUGO | S_IWUSR,
					      g_dpm_hwmon_debugfs_root,
					      NULL, &dpm_debug_fops);
	if (dpm_hwmon_debug == NULL) {
		debugfs_remove_recursive(g_dpm_hwmon_debugfs_root);
		pr_err("%s LINE %d fail!\n", __func__, __LINE__);
		return -ENOENT;
	}
	return 0;
}

static void dpm_hwmon_debugfs_exit(void)
{
	pr_err("dpm %s\n", __func__);
	debugfs_remove_recursive(g_dpm_hwmon_debugfs_root);
	pr_err("dpm_hwmon_debugfs removed!\n");
}
#endif

static int __init dpm_hwmon_init(void)
{
	g_dpm_report_enabled = true;

	if (dpm_hwmon_prepare() < 0)
		return -EFAULT;

#if defined(CONFIG_DPM_HWMON_DEBUG) && defined(CONFIG_DEBUG_FS)
	(void)dpm_hwmon_debugfs_init();
#endif
	return 0;
}

static void __exit dpm_hwmon_exit(void)
{
	dpm_hwmon_end();

#if defined(CONFIG_DPM_HWMON_DEBUG) && defined(CONFIG_DEBUG_FS)
	dpm_hwmon_debugfs_exit();
#endif
}

module_init(dpm_hwmon_init);
module_exit(dpm_hwmon_exit);

#ifdef CONFIG_DEBUG_FS
module_param_named(dpm_report_enable, g_dpm_report_enabled,
		   bool, S_IRUGO | S_IWUSR);
#endif
