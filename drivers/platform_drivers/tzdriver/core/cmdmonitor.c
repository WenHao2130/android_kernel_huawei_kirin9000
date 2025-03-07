/*
 * cmd_monitor.c
 *
 * cmdmonitor function, monitor every cmd which is sent to TEE.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "cmdmonitor.h"
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <securec.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif

#ifdef CONFIG_TEE_LOG_EXCEPTION
#include <huawei_platform/log/imonitor.h>
#define IMONITOR_TA_CRASH_EVENT_ID 901002003
#define IMONITOR_MEMSTAT_EVENT_ID 940007001
#define IMONITOR_TAMEMSTAT_EVENT_ID 940007002
#endif

#include "tc_ns_log.h"
#include "smc_smp.h"
#include "tui.h"
#include "mailbox_mempool.h"
#include "tlogger.h"
#include "log_cfg_api.h"
#include "tz_kthread_affinity.h"

static int g_cmd_need_archivelog;
static LIST_HEAD(g_cmd_monitor_list);
static int g_cmd_monitor_list_size;
/* report 2 hours */
static const long long g_memstat_report_freq = 2 * 60 * 60 * 1000;
#define MAX_CMD_MONITOR_LIST 200
#define MAX_AGENT_CALL_COUNT 250
static DEFINE_MUTEX(g_cmd_monitor_lock);

/* independent wq to avoid block system_wq */
static struct workqueue_struct *g_cmd_monitor_wq;
static struct delayed_work g_cmd_monitor_work;
static struct delayed_work g_cmd_monitor_work_archive;
static struct delayed_work g_mem_stat;
static int g_tee_detect_ta_crash;

enum {
	TYPE_CRASH_TA = 1,
	TYPE_CRASH_TEE = 2,
};

static void get_time_spec(struct time_spec *time)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	time->ts = current_kernel_time();
#else
	ktime_get_coarse_ts64(&time->ts);
#endif
}

static void schedule_memstat_work(struct delayed_work *work,
	unsigned long delay)
{
	schedule_delayed_work(work, delay);
}
static void schedule_cmd_monitor_work(struct delayed_work *work,
	unsigned long delay)
{
	if (g_cmd_monitor_wq)
		queue_delayed_work(g_cmd_monitor_wq, work, delay);
	else
		schedule_delayed_work(work, delay);
}

void tzdebug_memstat(void)
{
	schedule_memstat_work(&g_mem_stat, usecs_to_jiffies(S_TO_MS));
}

void tzdebug_archivelog(void)
{
	schedule_cmd_monitor_work(&g_cmd_monitor_work_archive,
		usecs_to_jiffies(0));
}

void cmd_monitor_ta_crash(int32_t type)
{
	g_tee_detect_ta_crash = ((type == TYPE_CRASH_TEE) ?
		TYPE_CRASH_TEE : TYPE_CRASH_TA);
	tzdebug_archivelog();
}

static int get_pid_name(pid_t pid, char *comm, size_t size)
{
	struct task_struct *task = NULL;
	int sret;

	if (size <= TASK_COMM_LEN - 1 || !comm)
		return -1;

	rcu_read_lock();

	task = find_task_by_vpid(pid);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task) {
		tloge("get task failed\n");
		return -1;
	}

	sret = strncpy_s(comm, size, task->comm, strlen(task->comm));
	if (sret)
		tloge("strncpy faild: errno = %d\n", sret);
	put_task_struct(task);

	return sret;
}

bool is_thread_reported(unsigned int tid)
{
	bool ret = false;
	struct cmd_monitor *monitor = NULL;

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
		if (monitor->tid == tid && !is_tui_in_use(monitor->tid)) {
			ret = (monitor->is_reported ||
				monitor->agent_call_count >
				MAX_AGENT_CALL_COUNT);
			break;
		}
	}
	mutex_unlock(&g_cmd_monitor_lock);
	return ret;
}

#ifdef CONFIG_TEE_LOG_EXCEPTION
#define FAIL_RET (-1)
#define SUCC_RET 0

static int send_memstat_packet(const struct tee_mem *meminfo)
{
	struct imonitor_eventobj *memstat = NULL;
	uint32_t result = 0;
	struct time_spec nowtime;
	int ret;
	get_time_spec(&nowtime);

	memstat = imonitor_create_eventobj(IMONITOR_MEMSTAT_EVENT_ID);
	if (!memstat) {
		tloge("create eventobj failed\n");
		return FAIL_RET;
	}

	result |= (uint32_t)imonitor_set_param_integer_v2(memstat,
		"totalmem", meminfo->total_mem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat,
		"mem", meminfo->pmem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat,
		"freemem", meminfo->free_mem);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat,
		"freememmin", meminfo->free_mem_min);
	result |= (uint32_t)imonitor_set_param_integer_v2(memstat,
		"tanum", meminfo->ta_num);
	result |= (uint32_t)imonitor_set_time(memstat, nowtime.ts.tv_sec);
	if (result) {
		tloge("set param integer1 failed ret=%u\n", result);
		imonitor_destroy_eventobj(memstat);
		return FAIL_RET;
	}

	ret = imonitor_send_event(memstat);
	imonitor_destroy_eventobj(memstat);
	if (ret <= 0) {
		tloge("imonitor send memstat packet failed\n");
		return FAIL_RET;
	}

	return SUCC_RET;
}

void report_imonitor(const struct tee_mem *meminfo)
{
	int ret;
	uint32_t result = 0;
	uint32_t i;
	struct imonitor_eventobj *pamemobj = NULL;
	struct time_spec nowtime;
	get_time_spec(&nowtime);

	if (!meminfo)
		return;

	if (meminfo->ta_num > MEMINFO_TA_MAX)
		return;

	if (send_memstat_packet(meminfo))
		return;

	for (i = 0; i < meminfo->ta_num; i++) {
		pamemobj =
			imonitor_create_eventobj(IMONITOR_TAMEMSTAT_EVENT_ID);
		if (!pamemobj) {
			tloge("create obj failed\n");
			break;
		}

		result |= (uint32_t)imonitor_set_param_string_v2(pamemobj,
			"NAME", meminfo->ta_mem_info[i].ta_name);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj,
			"MEM", meminfo->ta_mem_info[i].pmem);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj,
			"MEMMAX", meminfo->ta_mem_info[i].pmem_max);
		result |= (uint32_t)imonitor_set_param_integer_v2(pamemobj,
			"MEMLIMIT", meminfo->ta_mem_info[i].pmem_limit);
		result |= (uint32_t)imonitor_set_time(pamemobj,
			nowtime.ts.tv_sec);
		if (result) {
			tloge("set param integer2 failed ret=%d\n", result);
			imonitor_destroy_eventobj(pamemobj);
			return;
		}
		ret = imonitor_send_event(pamemobj);
		imonitor_destroy_eventobj(pamemobj);
		if (ret <= 0) {
			tloge("imonitor send pamem packet failed\n");
			break;
		}
	}
}
#endif

void memstat_report(void)
{
	int ret;
	struct tee_mem *meminfo = NULL;

	meminfo = mailbox_alloc(sizeof(*meminfo), MB_FLAG_ZERO);
	if (!meminfo) {
		tloge("mailbox alloc failed\n");
		return;
	}

	ret = get_tee_meminfo(meminfo);
#ifdef CONFIG_TEE_LOG_EXCEPTION
	if (!ret) {
		tlogd("report imonitor\n");
		report_imonitor(meminfo);
	}
#else
	if (!ret)
		tlogd("get meminfo failed\n");
#endif
	mailbox_free(meminfo);
}

static void memstat_work(struct work_struct *work)
{
	(void)(work);
	memstat_report();
}

void cmd_monitor_reset_context(void)
{
	struct cmd_monitor *monitor = NULL;
	pid_t pid = current->tgid;
	pid_t tid = current->pid;

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
		if (monitor->pid == pid && monitor->tid == tid) {
			get_time_spec(&monitor->sendtime);
			if (monitor->agent_call_count + 1 < 0)
				tloge("agent call count add overflow\n");
			else
				monitor->agent_call_count++;
			break;
		}
	}
	mutex_unlock(&g_cmd_monitor_lock);
}

#ifdef CONFIG_TEE_LOG_EXCEPTION
static struct time_spec g_memstat_check_time;
static bool g_after_loader = false;

static void auto_report_memstat(void)
{
	long long timedif;
	struct time_spec nowtime;
	get_time_spec(&nowtime);

	/*
	 * get time value D (timedif=nowtime-sendtime),
	 * we do not care about overflow
	 * 1 year means 1000 * (60*60*24*365) = 0x757B12C00
	 * only 5bytes, will not overflow
	 */
	timedif = S_TO_MS * (nowtime.ts.tv_sec - g_memstat_check_time.ts.tv_sec) +
		(nowtime.ts.tv_nsec - g_memstat_check_time.ts.tv_nsec) / S_TO_US;
	if (timedif > g_memstat_report_freq && g_after_loader) {
		tlogi("cmdmonitor auto report memstat\n");
		memstat_report();
		g_memstat_check_time = nowtime;
	}

	if (!g_after_loader) {
		g_memstat_check_time = nowtime;
		g_after_loader = true;
	}
}
#endif

/*
 * if one session timeout, monitor will print timedifs every step[n] in secends,
 * if lasted more then 360s, monitor will print timedifs every 360s.
 */
const int32_t g_timer_step[] = {1, 1, 1, 2, 5, 10, 40, 120, 180, 360};
const int32_t g_timer_nums = sizeof(g_timer_step) / sizeof(int32_t);
static void show_timeout_cmd_info(struct cmd_monitor *monitor)
{
	long long timedif, timedif2;
	struct time_spec nowtime;
	get_time_spec(&nowtime);

	/*
	 * 1 year means 1000 * (60*60*24*365) = 0x757B12C00
	 * only 5bytes, so timedif (timedif=nowtime-sendtime) will not overflow
	 */
	timedif = S_TO_MS * (nowtime.ts.tv_sec - monitor->sendtime.ts.tv_sec) +
		(nowtime.ts.tv_nsec - monitor->sendtime.ts.tv_nsec) / S_TO_US;

	/* timeout to 10s, we log the teeos log, and report */
	if ((timedif > CMD_MAX_EXECUTE_TIME * S_TO_MS) && (!monitor->is_reported)) {
		monitor->is_reported = true;
		tlogw("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d, "
			"tname=%s, lastcmdid=%u, agent call count:%d, "
			"running with timedif=%lld ms and report\n",
			monitor->pid, monitor->pname, monitor->tid,
			monitor->tname, monitor->lastcmdid,
			monitor->agent_call_count, timedif);
		/* threads out of white table need info dump */
		tlogw("monitor: pid-%d", monitor->pid);
		if (!is_tui_in_use(monitor->tid)) {
			show_cmd_bitmap();
			g_cmd_need_archivelog = 1;
			wakeup_tc_siq(SIQ_DUMP_TIMEOUT);
		}
	}

	timedif2 = S_TO_MS * (nowtime.ts.tv_sec - monitor->lasttime.ts.tv_sec) +
		(nowtime.ts.tv_nsec - monitor->lasttime.ts.tv_nsec) / S_TO_US;
	int32_t time_in_sec = monitor->timer_index >= g_timer_nums ?
		g_timer_step[g_timer_nums - 1] : g_timer_step[monitor->timer_index];
	if (timedif2 > time_in_sec * S_TO_MS) {
		monitor->lasttime = nowtime;
		monitor->timer_index = monitor->timer_index >= sizeof(g_timer_step) ?
			sizeof(g_timer_step) : (monitor->timer_index + 1);
		tlogw("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d, "
			"lastcmdid=%u,agent call count:%d,timedif=%lld ms\n",
			monitor->pid, monitor->pname, monitor->tid,
			monitor->lastcmdid, monitor->agent_call_count,
			timedif);
	}
}

static void cmd_monitor_tick(void)
{
	struct cmd_monitor *monitor = NULL;
	struct cmd_monitor *tmp = NULL;

	mutex_lock(&g_cmd_monitor_lock);
	list_for_each_entry_safe(monitor, tmp, &g_cmd_monitor_list, list) {
		if (monitor->returned) {
			g_cmd_monitor_list_size--;
			tlogi("[cmd_monitor_tick] pid=%d,pname=%s,tid=%d, "
				"tname=%s,lastcmdid=%u,count=%d,agent call count=%d, "
				"timetotal=%lld us returned, remained command(s)=%d\n",
				monitor->pid, monitor->pname, monitor->tid, monitor->tname,
				monitor->lastcmdid, monitor->count, monitor->agent_call_count,
				monitor->timetotal, g_cmd_monitor_list_size);
			list_del(&monitor->list);
			kfree(monitor);
			continue;
		}
		show_timeout_cmd_info(monitor);
	}

	/* if have cmd in monitor list, we need tick */
	if (g_cmd_monitor_list_size > 0)
		schedule_cmd_monitor_work(&g_cmd_monitor_work, usecs_to_jiffies(S_TO_US));
	mutex_unlock(&g_cmd_monitor_lock);
#ifdef CONFIG_TEE_LOG_EXCEPTION
	auto_report_memstat();
#endif
}

static void cmd_monitor_tickfn(struct work_struct *work)
{
	(void)(work);
	cmd_monitor_tick();
	/* check tlogcat if have new log */
	tz_log_write();
}

static void cmd_monitor_archivefn(struct work_struct *work)
{
	(void)(work);
	if (tlogger_store_msg(CONFIG_TEE_LOG_ACHIVE_PATH,
		sizeof(CONFIG_TEE_LOG_ACHIVE_PATH)) < 0)
		tloge("[cmd_monitor_tick]tlogger store lastmsg failed\n");

	if (g_tee_detect_ta_crash == TYPE_CRASH_TA) {
#ifdef CONFIG_TEE_LOG_EXCEPTION
		if (teeos_log_exception_archive(IMONITOR_TA_CRASH_EVENT_ID,
			"ta crash") < 0)
			tloge("log exception archive failed\n");
#endif
	}

	if (g_tee_detect_ta_crash == TYPE_CRASH_TEE) {
		tloge("detect teeos crash, panic\n");
		report_log_system_panic();
	}

	g_tee_detect_ta_crash = 0;
}

static struct cmd_monitor *init_monitor_locked(void)
{
	struct cmd_monitor *newitem = NULL;

	newitem = kzalloc(sizeof(*newitem), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)newitem)) {
		tloge("[cmd_monitor_tick]kzalloc faild\n");
		return NULL;
	}

	get_time_spec(&newitem->sendtime);
	newitem->lasttime = newitem->sendtime;
	newitem->timer_index = 0;
	newitem->count = 1;
	newitem->agent_call_count = 0;
	newitem->returned = false;
	newitem->is_reported = false;
	newitem->pid = current->tgid;
	newitem->tid = current->pid;
	if (get_pid_name(newitem->pid, newitem->pname,
		sizeof(newitem->pname)))
		newitem->pname[0] = '\0';
	if (get_pid_name(newitem->tid, newitem->tname,
		sizeof(newitem->tname)))
		newitem->tname[0] = '\0';
	INIT_LIST_HEAD(&newitem->list);
	list_add_tail(&newitem->list, &g_cmd_monitor_list);
	g_cmd_monitor_list_size++;
	return newitem;
}

struct cmd_monitor *cmd_monitor_log(const struct tc_ns_smc_cmd *cmd)
{
	bool found_flag = false;
	pid_t pid;
	pid_t tid;
	struct cmd_monitor *monitor = NULL;

	if (!cmd)
		return NULL;

	pid = current->tgid;
	tid = current->pid;
	mutex_lock(&g_cmd_monitor_lock);
	do {
		list_for_each_entry(monitor, &g_cmd_monitor_list, list) {
			if (monitor->pid == pid && monitor->tid == tid) {
				found_flag = true;
				/* restart */
				get_time_spec(&monitor->sendtime);
				monitor->lasttime = monitor->sendtime;
				monitor->timer_index = 0;
				monitor->count++;
				monitor->returned = false;
				monitor->is_reported = false;
				monitor->lastcmdid = cmd->cmd_id;
				monitor->agent_call_count = 0;
				monitor->timetotal = 0;
				break;
			}
		}

		if (!found_flag) {
			if (g_cmd_monitor_list_size > MAX_CMD_MONITOR_LIST - 1) {
				tloge("monitor reach max node num\n");
				monitor = NULL;
				break;
			}
			monitor = init_monitor_locked();
			if (!monitor) {
				tloge("init monitor failed\n");
				break;
			}
			monitor->lastcmdid = cmd->cmd_id;
			/* the first cmd will cause timer */
			if (g_cmd_monitor_list_size == 1)
				schedule_cmd_monitor_work(&g_cmd_monitor_work,
					usecs_to_jiffies(S_TO_US));
		}
	} while (0);
	mutex_unlock(&g_cmd_monitor_lock);

	return monitor;
}

void cmd_monitor_logend(struct cmd_monitor *item)
{
	struct time_spec nowtime;
	long long timedif;

	if (!item)
		return;

	get_time_spec(&nowtime);
	/*
	 * get time value D (timedif=nowtime-sendtime),
	 * we do not care about overflow
	 * 1 year means 1000000 * (60*60*24*365) = 0x1CAE8C13E000
	 * only 6bytes, will not overflow
	 */
	timedif = S_TO_US * (nowtime.ts.tv_sec - item->sendtime.ts.tv_sec) +
		(nowtime.ts.tv_nsec - item->sendtime.ts.tv_nsec) / S_TO_MS;
	item->timetotal += timedif;
	item->returned = true;
}

void do_cmd_need_archivelog(void)
{
	if (g_cmd_need_archivelog == 1) {
		g_cmd_need_archivelog = 0;
		schedule_cmd_monitor_work(&g_cmd_monitor_work_archive,
			usecs_to_jiffies(S_TO_US));
	}
}

void init_cmd_monitor(void)
{
	g_cmd_monitor_wq = alloc_workqueue("tz_cmd_monitor_wq",
		WQ_UNBOUND, TZ_WQ_MAX_ACTIVE);
	if (!g_cmd_monitor_wq)
		tloge("alloc cmd monitor wq failed\n");
	else
		tz_workqueue_bind_mask(g_cmd_monitor_wq, 0);

	INIT_DEFERRABLE_WORK((struct delayed_work *)
		(uintptr_t)&g_cmd_monitor_work, cmd_monitor_tickfn);
	INIT_DEFERRABLE_WORK((struct delayed_work *)
		(uintptr_t)&g_cmd_monitor_work_archive, cmd_monitor_archivefn);
	INIT_DEFERRABLE_WORK((struct delayed_work *)
		(uintptr_t)&g_mem_stat, memstat_work);

}
