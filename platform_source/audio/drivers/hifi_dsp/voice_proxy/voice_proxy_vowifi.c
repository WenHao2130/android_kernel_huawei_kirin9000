/*
 * voice_proxy_vowifi.c - HW voice proxy vowifi in kernel, it is used for pass
 * through voice data between audio hal and hifi for vowifi.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, MA 02111-1307, USA.
 *
 */

#include "voice_proxy.h"
#include <linux/miscdevice.h>

/* lint -e528 -e753 */
#define LOG_TAG "voice_proxy_vowifi"
#define WIFI_RX_DATA_SIZE sizeof(struct voice_proxy_wifi_rx_notify)
#define WIFI_TX_DATA_SIZE sizeof(struct voice_proxy_wifi_tx_notify)
#define DTS_COMP_VOICE_PROXY_VOWIFI_NAME "hisilicon,voice_proxy_vowifi"

#define VOICE_PROXY_WAKE_UP_VOWIFI_READ  _IO('P',  0x1)

enum STATUS {
	STATUS_CLOSE,
	STATUS_OPEN,
	STATUS_BUTT
};

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

LIST_HEAD(recv_vowifi_rx_queue);
LIST_HEAD(send_vowifi_tx_queue);

struct vowifi_priv {
	spinlock_t vowifi_read_lock;

	spinlock_t vowifi_write_lock;

	wait_queue_head_t vowifi_read_waitq;

	int32_t vowifi_read_wait_flag;

	/* this is used for counting the size of recv_vowifi_rx_queue */
	int32_t vowifi_rx_cnt;

	/* this is used for counting the size of send_vowifi_tx_queue */
	int32_t vowifi_tx_cnt;

	/* vowifi rx voice data confirm */
	bool vowifi_rx_cnf;

	/* vowifi rx first voice data */
	bool first_vowifi_rx;

	/* vowifi receive tx open msg from hifi */
	bool vowifi_is_tx_open;

	/* vowifi rx voice data time stamp */
	int64_t vowifi_rx_stamp;

	struct mutex ioctl_mutex;
};

static struct vowifi_priv priv;

struct voice_proxy_tx_notify {
	unsigned short msg_id;
	unsigned short modem_no;
};

static void vowifi_sign_init(void)
{
	priv.vowifi_rx_cnf = false;
	priv.first_vowifi_rx = true;
	priv.vowifi_is_tx_open = false;
}

static void vowifi_clear_queue(struct list_head *queue)
{
	uint32_t cnt = 0;
	struct voice_proxy_data_node *node = NULL;

	while (!list_empty_careful(queue)) {
		cnt++;
		if (cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
			AUDIO_LOGE("vowifi:clear queue abnormal, cnt is %u", cnt);
			break;
		}
		/* lint !e826 */
		node = list_first_entry(queue, struct voice_proxy_data_node, list_node);
		list_del_init(&node->list_node);
		kfree(node);
		node = NULL;
	}

	AUDIO_LOGI("vowifi:clear queue cnt is %u", cnt);
}

static int32_t vowifi_add_tx_data(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&priv.vowifi_read_lock);
	if (priv.vowifi_tx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		priv.vowifi_read_wait_flag++;
		spin_unlock_bh(&priv.vowifi_read_lock);
		wake_up(&priv.vowifi_read_waitq);

		return -ENOMEM;
	}
	spin_unlock_bh(&priv.vowifi_read_lock);

	ret = voice_proxy_create_data_node(&node, rev_buf, (int)buf_size);
	if (ret) {
		AUDIO_LOGE("data_node kzalloc failed");
		return ret;
	}

	spin_lock_bh(&priv.vowifi_read_lock);
	list_add_tail(&node->list_node, &send_vowifi_tx_queue);
	priv.vowifi_tx_cnt++;
	priv.vowifi_read_wait_flag++;
	spin_unlock_bh(&priv.vowifi_read_lock);
	wake_up(&priv.vowifi_read_waitq);

	return 0;
}

static void vowifi_get_rx_data(int8_t *data, uint32_t *size)
{
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&priv.vowifi_write_lock);
	if (!list_empty_careful(&recv_vowifi_rx_queue)) {
		node = list_first_entry(&recv_vowifi_rx_queue, struct voice_proxy_data_node, list_node); /* lint !e826 */

		list_del_init(&node->list_node);

		if (priv.vowifi_rx_cnt > 0)
			priv.vowifi_rx_cnt--;

		if (*size < (uint32_t)node->list_data.size) {
			AUDIO_LOGE("Size is invalid, size = %d, list_data.size = %d", *size, node->list_data.size);
			kfree(node);
			*size = 0;
			spin_unlock_bh(&priv.vowifi_write_lock);
			return;
		}

		*size = (uint32_t)node->list_data.size;
		memcpy(data, node->list_data.data, (size_t)*size); /* unsafe_function_ignore: memcpy */

		kfree(node);
		spin_unlock_bh(&priv.vowifi_write_lock);

		priv.first_vowifi_rx = false;
		priv.vowifi_rx_cnf = false;
	} else {
		spin_unlock_bh(&priv.vowifi_write_lock);
		*size = 0;
	}
} /* lint !e438 */

static void vowifi_receive_rctp_om_tx_ntf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;

	if (!rev_buf) {
		AUDIO_LOGE("receive_rctp_om_tx_ntf fail, param rev_buf is NULL");
		return;
	}

	ret = vowifi_add_tx_data(rev_buf, buf_size);
	if (ret) {
		AUDIO_LOGE("send vowifi tx data to read func failed");
		return;
	}

}

static void vowifi_receive_ajb_om_tx_ntf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;

	if (!rev_buf) {
		AUDIO_LOGE("receive_ajb_om_tx_ntf fail, param rev_buf is NULL");
		return;
	}

	ret = vowifi_add_tx_data(rev_buf, buf_size);
	if (ret) {
		AUDIO_LOGE("send vowifi tx data to read func failed");
		return;
	}

}

static void vowifi_receive_status_ntf(int8_t *rev_buf, uint32_t buf_size)
{
	struct dsp_proxy_wifi_status_ind *status_ind = NULL;

	if (!rev_buf) {
		AUDIO_LOGE("vowifi:status notify param rev_buf is NULL");
		return;
	}

	if (buf_size < sizeof(struct dsp_proxy_wifi_status_ind)) {
		AUDIO_LOGE("vowifi:status indication msg size is error,actually is %u, the expection not less than %ld",
			buf_size, sizeof(struct dsp_proxy_wifi_status_ind));
		return;
	}

	status_ind = (struct dsp_proxy_wifi_status_ind *)rev_buf;

	spin_lock_bh(&priv.vowifi_read_lock);
	if (status_ind->status == STATUS_OPEN) {
		if (!priv.vowifi_is_tx_open) {
			vowifi_clear_queue(&send_vowifi_tx_queue);
			priv.vowifi_tx_cnt = 0;
			priv.vowifi_read_wait_flag = 0;
		}
		priv.vowifi_is_tx_open = true;
	} else if (status_ind->status == STATUS_CLOSE) {
		priv.vowifi_is_tx_open = false;
	}
	spin_unlock_bh(&priv.vowifi_read_lock);

	AUDIO_LOGI("vowifi:status indication, status is %u(1-open,0-close)",
		status_ind->status);
}

static void vowifi_receive_tx_ntf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret;
	struct voice_proxy_wifi_tx_notify *tx_msg = NULL;

	if (!rev_buf) {
		AUDIO_LOGE("receive_tx_ntf fail, param rev_buf is NULL");
		return;
	}

	tx_msg = (struct voice_proxy_wifi_tx_notify *)rev_buf;

	ret = vowifi_add_tx_data(rev_buf, buf_size);
	if (ret) {
		AUDIO_LOGE("send vowifi tx data to read func failed");
		return;
	}

	/* hifi only need channel id */
	voice_proxy_add_work_queue_cmd(ID_PROXY_VOICE_WIFI_TX_CNF, tx_msg->modem_no, tx_msg->channel_id);

}

static void vowifi_receive_rx_cnf(int8_t *rev_buf, uint32_t buf_size)
{
	int32_t ret = voice_proxy_add_cmd(ID_VOICE_PROXY_WIFI_RX_CNF);

	if (ret)
		AUDIO_LOGE("send wifi rx data cnf failed");
}

static void vowifi_handle_rx_ntf(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	if (!data || !size || !msg_id) {
		AUDIO_LOGE("handle_rx_ntf fail, param is NULL");
		return;
	}

	voice_proxy_set_send_sign(priv.first_vowifi_rx, &priv.vowifi_rx_cnf, &priv.vowifi_rx_stamp);

	if (priv.first_vowifi_rx || priv.vowifi_rx_cnf)
		vowifi_get_rx_data(data, size);
	else
		*size = 0;

	*msg_id = ID_PROXY_VOICE_WIFI_RX_NTF;
}

static void vowifi_handle_rx_cnf(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	if (!data || !size || !msg_id) {
		AUDIO_LOGE("handle_rx_cnf fail, param is NULL");
		return;
	}

	priv.vowifi_rx_cnf = true;
	priv.vowifi_rx_stamp = voice_proxy_get_time_ms();

	vowifi_get_rx_data(data, size);
	*msg_id = ID_PROXY_VOICE_WIFI_RX_NTF;
}

static bool is_input_param_valid(struct file *file, size_t size)
{
	if (!file) {
		AUDIO_LOGE("file is null");
		return false;
	}

	if (file->f_flags & O_NONBLOCK) {
		AUDIO_LOGE("file->f_flags & O_NONBLOCK(%d) fail", file->f_flags & O_NONBLOCK);
		return false;
	}

	if (size < WIFI_TX_DATA_SIZE) {
		AUDIO_LOGE("param err, size(%zd) < WIFI_TX_DATA_SIZE(%ld)", size, WIFI_TX_DATA_SIZE);
		return false;
	}

	return true;
}

static ssize_t vowifi_read(struct file *file, char __user *user_buf, size_t size, loff_t *ppos)
{
	struct voice_proxy_data_node *node = NULL;
	int ret = 0;

	if (!is_input_param_valid(file, size)) {
		AUDIO_LOGE("invalid input params");
		return -EINVAL;
	}

	spin_lock_bh(&priv.vowifi_read_lock);
	if (list_empty_careful(&send_vowifi_tx_queue)) {
		spin_unlock_bh(&priv.vowifi_read_lock);
		ret = wait_event_interruptible(priv.vowifi_read_waitq,
			priv.vowifi_read_wait_flag > 0); /* lint !e40 !e578 !e774 !e845 !e712 */
		if (ret) {
			if (ret != -ERESTARTSYS)
				AUDIO_LOGE("wait event interruptible fail, 0x%x", ret);
			return -EBUSY;
		}
		spin_lock_bh(&priv.vowifi_read_lock);
	}

	priv.vowifi_read_wait_flag = 0;

	if (!list_empty_careful(&send_vowifi_tx_queue)) {
		node = list_first_entry(&send_vowifi_tx_queue,
		struct voice_proxy_data_node, list_node); /* lint !e826 */

		list_del_init(&node->list_node);
		if (priv.vowifi_tx_cnt > 0)
			priv.vowifi_tx_cnt--;

		if (user_buf == NULL || size < node->list_data.size) { /*lint !e574 !e737*/
			AUDIO_LOGE("user_buf == NULL or size(%zd) < node->list_data.size(%d)", size, node->list_data.size);
			kfree(node);
			spin_unlock_bh(&priv.vowifi_read_lock);
			return -EAGAIN; /* lint !e438 */
		}

		if (copy_to_user(user_buf, node->list_data.data, node->list_data.size)) { /* lint !e732 !e747 */
			AUDIO_LOGE("copy_to_user fail");
			ret = -EFAULT;
		} else {
			ret = node->list_data.size;
		}
		kfree(node);
		spin_unlock_bh(&priv.vowifi_read_lock);
	} else {
		spin_unlock_bh(&priv.vowifi_read_lock);
		ret = -EAGAIN;
		AUDIO_LOGE("list is empty, read again");
	}

	return ret; /* lint !e438 */
}

static int32_t vowifi_add_rx_data(int8_t *data, uint32_t size)
{
	int32_t ret;
	struct voice_proxy_data_node *node = NULL;

	spin_lock_bh(&priv.vowifi_write_lock);
	if (priv.vowifi_rx_cnt > VOICE_PROXY_QUEUE_SIZE_MAX) {
		spin_unlock_bh(&priv.vowifi_write_lock);
		AUDIO_LOGE("out of queue, rx cnt(%d)>(%d)", priv.vowifi_rx_cnt, VOICE_PROXY_QUEUE_SIZE_MAX);
		return -ENOMEM;
	}
	spin_unlock_bh(&priv.vowifi_write_lock);

	ret = voice_proxy_create_data_node(&node, data, (int)size);
	if (ret) {
		AUDIO_LOGE("node kzalloc failed");
		return -EFAULT;
	}
	spin_lock_bh(&priv.vowifi_write_lock);
	list_add_tail(&node->list_node, &recv_vowifi_rx_queue);
	priv.vowifi_rx_cnt++;
	spin_unlock_bh(&priv.vowifi_write_lock);

	return (int)size;
}

static ssize_t vowifi_write(struct file *filp, const char __user *buff, size_t size, loff_t *offp)
{
	int32_t ret;
	int8_t data[WIFI_RX_DATA_SIZE] = {0};

	UNUSED_PARAMETER(filp);
	UNUSED_PARAMETER(offp);

	if (buff == NULL || size > WIFI_RX_DATA_SIZE) {
		AUDIO_LOGE("buff == NULL or para error, size:%zd(>%ld)", size, WIFI_RX_DATA_SIZE);
		return -EINVAL;
	}

	if (copy_from_user(data, buff, size)) {
		AUDIO_LOGE("copy_from_user fail");
		return -EFAULT;
	}

	ret = voice_proxy_add_data(vowifi_add_rx_data, data, (uint32_t)size, ID_PROXY_VOICE_WIFI_RX_NTF);
	if (ret <= 0) {
		AUDIO_LOGE("call voice_proxy_add_data fail");
		return -EFAULT;
	}

	return (int)size;
}

static void vowifi_wake_up_read(void)
{
	spin_lock_bh(&priv.vowifi_read_lock);
	priv.vowifi_read_wait_flag++;
	spin_unlock_bh(&priv.vowifi_read_lock);
	wake_up(&priv.vowifi_read_waitq);
}

static long vowifi_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	UNUSED_PARAMETER(fd);
	UNUSED_PARAMETER(arg);

	mutex_lock(&priv.ioctl_mutex);
	switch (cmd) {
	case VOICE_PROXY_WAKE_UP_VOWIFI_READ: /* lint !e845 */
		vowifi_wake_up_read();
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&priv.ioctl_mutex);

	return (long)ret;
}

static int vowifi_open(struct inode *finode, struct file *fd)
{
	UNUSED_PARAMETER(finode);
	UNUSED_PARAMETER(fd);

	spin_lock_bh(&priv.vowifi_write_lock);
	vowifi_clear_queue(&recv_vowifi_rx_queue);
	priv.vowifi_rx_cnt = 0;
	spin_unlock_bh(&priv.vowifi_write_lock);

	return 0;
}

static int vowifi_close(struct inode *node, struct file *filp)
{
	UNUSED_PARAMETER(node);
	UNUSED_PARAMETER(filp);

	spin_lock_bh(&priv.vowifi_read_lock);
	priv.vowifi_read_wait_flag++;
	priv.vowifi_is_tx_open = false;
	spin_unlock_bh(&priv.vowifi_read_lock);
	wake_up(&priv.vowifi_read_waitq);
	return 0;
}

static const struct file_operations vowifi_misc_fops = {
	.owner = THIS_MODULE, /* lint !e64 */
	.open = vowifi_open,
	.read = vowifi_read,
	.write = vowifi_write,
	.release = vowifi_close,
	.unlocked_ioctl = vowifi_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = vowifi_ioctl,
#endif
}; /* lint !e785 */

static struct miscdevice vowifi_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "voice_proxy_vowifi",
	.fops = &vowifi_misc_fops,
}; /* lint !e785 */

static int vowifi_probe(struct platform_device *pdev)
{
	int32_t ret;

	memset(&priv, 0, sizeof(priv)); /* unsafe_function_ignore: memset */

	UNUSED_PARAMETER(pdev);

	priv.vowifi_read_wait_flag = 0;

	spin_lock_init(&priv.vowifi_read_lock);
	spin_lock_init(&priv.vowifi_write_lock);
	init_waitqueue_head(&priv.vowifi_read_waitq);
	mutex_init(&priv.ioctl_mutex);

	ret = misc_register(&vowifi_misc_device);
	if (ret) {
		AUDIO_LOGE("vowifi misc register fail");
		return ret;
	}

	vowifi_sign_init();

	voice_proxy_register_msg_callback(ID_VOICE_PROXY_RTCP_OM_INFO_NTF, vowifi_receive_rctp_om_tx_ntf);
	voice_proxy_register_msg_callback(ID_VOICE_PROXY_AJB_OM_INFO_NTF, vowifi_receive_ajb_om_tx_ntf);
	voice_proxy_register_msg_callback(ID_HIFI_PROXY_WIFI_STATUS_NTF, vowifi_receive_status_ntf);
	voice_proxy_register_msg_callback(ID_VOICE_PROXY_WIFI_TX_NTF, vowifi_receive_tx_ntf);
	voice_proxy_register_msg_callback(ID_VOICE_PROXY_WIFI_RX_CNF, vowifi_receive_rx_cnf);
	voice_proxy_register_cmd_callback(ID_PROXY_VOICE_WIFI_RX_NTF, vowifi_handle_rx_ntf);
	voice_proxy_register_cmd_callback(ID_VOICE_PROXY_WIFI_RX_CNF, vowifi_handle_rx_cnf);
	voice_proxy_register_sign_init_callback(vowifi_sign_init);

	return ret;
}

static int vowifi_remove(struct platform_device *pdev)
{

	UNUSED_PARAMETER(pdev);

	mutex_destroy(&priv.ioctl_mutex);
	misc_deregister(&vowifi_misc_device);

	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_RTCP_OM_INFO_NTF);
	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_AJB_OM_INFO_NTF);

	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_WIFI_TX_NTF);
	voice_proxy_deregister_msg_callback(ID_VOICE_PROXY_WIFI_RX_CNF);
	voice_proxy_deregister_msg_callback(ID_HIFI_PROXY_WIFI_STATUS_NTF);

	voice_proxy_deregister_cmd_callback(ID_PROXY_VOICE_WIFI_RX_NTF);
	voice_proxy_deregister_cmd_callback(ID_VOICE_PROXY_WIFI_RX_CNF);

	voice_proxy_deregister_sign_init_callback(vowifi_sign_init);

	return 0;
}


static const struct of_device_id vowifi_match_table[] = {
	{
		.compatible = DTS_COMP_VOICE_PROXY_VOWIFI_NAME,
		.data = NULL,
	},
	{
	} /* lint !e785 */
};

static struct platform_driver vowifi_driver = {
	.driver = {
		.name  = "voice proxy vowifi",
		.owner = THIS_MODULE, /* lint !e64 */
		.of_match_table = of_match_ptr(vowifi_match_table),
	}, /* lint !e785 */
	.probe = vowifi_probe,
	.remove = vowifi_remove,
}; /* lint !e785 */

static int __init vowifi_init(void)
{
	int32_t ret;

	ret = platform_driver_register(&vowifi_driver); /* lint !e64 */
	if (ret)
		AUDIO_LOGE("voice proxy vowifi driver register fail,ERROR is %d", ret);

	return ret;
}

static void __exit vowifi_exit(void)
{
	platform_driver_unregister(&vowifi_driver);
}

module_init(vowifi_init);
module_exit(vowifi_exit);

MODULE_DESCRIPTION("voice proxy vowifi driver");
MODULE_LICENSE("GPL");

