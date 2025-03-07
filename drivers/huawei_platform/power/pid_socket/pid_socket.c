/*copyright (c) Huawei Technologies Co., Ltd. 1998-2014. All rights reserved.
 *
 * File name: pid_socket.c
 * Description: This file use to record pid and socket
 * Author: xishiying@huawei.com
 * Version: 0.1
 * Date:  2014/11/27
 */

#include <net/tcp.h>

#include <linux/sched.h>
#include <linux/rcupdate.h>

#include "pid_socket.h"
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14,0 )
#include <linux/sched/task.h>
#endif
#include <net/sock.h>

#include <log/log_usertype.h>

void print_process_pid_name(struct inet_sock *inet)
{
	int pid = 0;
	int uid = 0;
	unsigned short source_port = 0;
#ifdef CONFIG_LOG_EXCEPTION
	unsigned int user_type = get_logusertype_flag();

	if (user_type != BETA_USER && user_type != OVERSEA_USER)
		return;
#endif

#if defined(CONFIG_HUAWEI_KSTATE)
	if (NULL == inet || NULL == inet->sk.sk_socket) {
		return;
	}

	pid = inet->sk.sk_socket->pid;
#else
	if (NULL == inet) {
		return;
	}

	pid = task_tgid_vnr(current);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	uid = sock_i_uid(&inet->sk).val;
#else
	uid = sock_i_uid(&inet->sk);
#endif

	source_port = inet->inet_sport;

	source_port = htons(source_port);
	printk("task=NULL, %s: uid:%d,pid:(%d),port:(%d)\n",__func__, uid, pid, source_port);
}

/*20150114 add,get pid and process name of the app who used connect function.*/
