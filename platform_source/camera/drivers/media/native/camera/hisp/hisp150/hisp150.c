/*
 * Copyright (c) Huawei Technologies Co., Ltd.2014-2020.All rights reserved.
 * Description:Implement of hisp150
 * Create: 2014-11-11
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <platform_include/camera/native/camera.h>
#include <platform_include/camera/native/hisp150_cfg.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/pm_qos.h>
#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include "cam_log.h"
#include "hisp_intf.h"
#include "platform/sensor_commom.h"
#include <linux/pm_wakeup.h>
#include <linux/platform_drivers/mm_ion.h>
#include <linux/mm_iommu.h>
#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <linux/iommu.h>
#include <linux/mutex.h>

DEFINE_MUTEX(kernel_rpmsg_service_mutex);

static struct pm_qos_request qos_request_ddr_down_record;
static int current_ddr_bandwidth = 0;
static struct wakeup_source hisp_power_wakelock;
static struct mutex hisp_wake_lock_mutex;

extern void hisp_boot_stat_dump(void);
typedef enum _timestamp_state_t {
	TIMESTAMP_UNINTIAL = 0,
	TIMESTAMP_INTIAL,
} timestamp_state_t;
static timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static u32 s_system_couter_rate;
static u64 s_system_counter;

enum hisp150_rpmsg_state {
	RPMSG_UNCONNECTED,
	RPMSG_CONNECTED,
	RPMSG_FAIL,
};

/*
 * These are used for distinguish the rpmsg_msg status
 * The process in hisp150_rpmsg_ept_cb are different
 * for the first receive and later.
 */
enum {
	HISP_SERV_FIRST_RECV,
	HISP_SERV_NOT_FIRST_RECV,
};

/*
 * brief the instance for rpmsg service
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to send/recv rpmsg and store the rpmsg data
 */
struct rpmsg_hisp150_service {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	struct rpmsg_device *rpdev;
#else
	struct rpmsg_channel *rpdev;
#endif
	struct mutex send_lock;
	struct mutex recv_lock;
	struct completion *comp;
	struct sk_buff_head queue;
	wait_queue_head_t readq;
	struct rpmsg_endpoint *ept;
	u32 dst;
	int state;
	char recv_count;
};

/*
 * brief the instance to talk to hisp driver
 * When Histar ISP is probed, this sturcture will be initialized,
 * the object is used to notify hisp driver when needed.
 */
typedef struct _tag_hisp150 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device *pdev; /* by used to get dts node */
	hisp_dt_data_t dt;
	struct iommu_domain *domain;
	struct ion_client *ion_client;
} hisp150_t;

struct rpmsg_service_info {
	struct rpmsg_hisp150_service *kernel_isp_serv;
	struct completion isp_comp;
	int isp_minor;
};

extern void a7_mmu_unmap(unsigned int va, unsigned int size);

/* Store the only rpmsg_hisp150_service pointer to local static rpmsg_local */
static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up = false;

#define I2HI(i) container_of(i, hisp150_t, intf)

static void hisp150_notify_rpmsg_cb(void);

char const *hisp150_get_name(hisp_intf_t *i);

static int hisp150_config(hisp_intf_t *i, void *cfg);

static int hisp150_power_on(hisp_intf_t *i);

static int hisp150_power_off(hisp_intf_t *i);

static int hisp150_send_rpmsg(hisp_intf_t *i, hisp_msg_t *m, size_t len);

static int hisp150_recv_rpmsg(hisp_intf_t *i,
	hisp_msg_t *user_addr, size_t len);

static void hisp150_set_ddrfreq(int ddr_bandwidth);

static void hisp150_release_ddrfreq(void);

static void hisp150_update_ddrfreq(unsigned int ddr_bandwidth);

void hisp150_init_timestamp(void);
void hisp150_destroy_timestamp(void);
void hisp150_set_timestamp(unsigned int *timestampH, unsigned int *timestampL);
void hisp150_handle_msg(hisp_msg_t *msg);

static bool hisp150_is_secure_supported(void)
{
#ifdef CONFIG_KERNEL_CAMERA_ISP_SECURE
	return true;
#else
	return false;
#endif
}

void hisp150_init_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_INTIAL;
	s_system_counter = arch_counter_get_cntvct();
	s_system_couter_rate = arch_timer_get_rate();
	do_gettimeofday(&s_timeval);

	cam_info("%s state=%u system_counter=%llu rate=%u"
		" time_second=%ld time_usecond=%ld size=%lu",
		__func__,
		(unsigned int)s_timestamp_state,
		s_system_counter,
		s_system_couter_rate,
		s_timeval.tv_sec,
		s_timeval.tv_usec,
		sizeof(s_timeval) / sizeof(u32));
}

void hisp150_destroy_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_UNINTIAL;
	s_system_counter = 0;
	s_system_couter_rate = 0;
	memset_s(&s_timeval, sizeof(s_timeval), 0x00, sizeof(s_timeval));
}

/* Function declaration */
/**********************************************
 * |-----pow-on------->||<----  fw-SOF ---->|
 * timeval(got) ----------------->fw_timeval=?
 * system_counter(got)----------------->fw_sys_counter(got)
 *
 * fw_timeval = timeval + (fw_sys_counter - system_counter)
 *
 * With a base position(<timeval, system_counter>, we get it at same time),
 * we can calculate fw_timeval with fw syscounter
 * and deliver it to hal.Hal then gets second and microsecond
 *********************************************/
void hisp150_set_timestamp(unsigned int *timestampH, unsigned int *timestampL)
{
	#define MICROSECOND_PER_SECOND 1000000
	u64 fw_micro_second = 0;
	u64 fw_sys_counter = 0;
	u64 micro_second = 0;

	if (s_timestamp_state == TIMESTAMP_UNINTIAL) {
		cam_err("%s wouldn't enter this branch", __func__);
		hisp150_init_timestamp();
	}

	if (timestampH == NULL || timestampL == NULL) {
		cam_err("%s timestampH or timestampL is null", __func__);
		return;
	}

	cam_debug("%s ack_high:0x%x ack_low:0x%x", __func__,
		*timestampH, *timestampL);

	if (*timestampH == 0 && *timestampL == 0)
		return;

	fw_sys_counter = ((u64)(*timestampH) << 32) | (u64)(*timestampL);
	micro_second = (fw_sys_counter - s_system_counter) *
		MICROSECOND_PER_SECOND / s_system_couter_rate;

	/* chang nano second to micro second */
	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND + s_timeval.tv_sec) *
		MICROSECOND_PER_SECOND +
		((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);

	*timestampH = (u32)(fw_micro_second >> 32 & 0xFFFFFFFF);
	*timestampL = (u32)(fw_micro_second & 0xFFFFFFFF);

	cam_debug("%s h:0x%x l:0x%x", __func__, *timestampH, *timestampL);
}

void hisp150_handle_msg(hisp_msg_t *msg)
{
	if (msg == NULL)
		return;
	switch (msg->api_name) {
	case REQUEST_RESPONSE:
		hisp150_set_timestamp(&(msg->u.ack_request.timestampH),
			&(msg->u.ack_request.timestampL));
		break;
	case MSG_EVENT_SENT:
		hisp150_set_timestamp(&(msg->u.event_sent.timestampH),
			&(msg->u.event_sent.timestampL));
		break;

	default:
		break;
	}
}

static hisp_vtbl_t s_vtbl_hisp150 = {
	.get_name = hisp150_get_name,
	.config = hisp150_config,
	.power_on = hisp150_power_on,
	.power_off = hisp150_power_off,
	.send_rpmsg = hisp150_send_rpmsg,
	.recv_rpmsg = hisp150_recv_rpmsg,
};

static hisp150_t s_hisp150 = {
	.intf = {.vtbl = &s_vtbl_hisp150,},
	.name = "hisp150",
};

static void hisp150_notify_rpmsg_cb(void)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB;
	cam_debug("%s, %d", __func__, __LINE__);
	hisp_notify_intf_rpmsg_cb(s_hisp150.notify, &isp_ev);
}


/* Function declaration */
/**********************************************
 * Save the rpmsg from isp to locally skb queue.
 * Only called by hisp150_rpmsg_ept_cb when api_name
 * is NOT POWER_REQ, will notify user space through HISP
 *********************************************/
static void hisp150_save_rpmsg_data(void *data, int len)
{
	struct rpmsg_hisp150_service *kernel_serv = rpmsg_local.kernel_isp_serv;
	struct sk_buff *skb = NULL;
	unsigned char *skbdata = NULL;

	cam_debug("Enter %s ", __func__);
	if (kernel_serv == NULL) {
		cam_err("func %s: kernel_serv is NULL", __func__);
		return;
	}
	hisp_assert(data != NULL);
	if (data == NULL)
		return;
	hisp_assert(len > 0);

	skb = alloc_skb(len, GFP_KERNEL);
	if (!skb) {
		cam_err("%s() %d failed: alloc_skb len is %u", __func__,
			__LINE__, len);
		return;
	}

	skbdata = skb_put(skb, len);
	memcpy_s(skbdata, len, data, len);

	/* add skb to skb queue */
	mutex_lock(&kernel_serv->recv_lock);
	skb_queue_tail(&kernel_serv->queue, skb);
	mutex_unlock(&kernel_serv->recv_lock);

	wake_up_interruptible(&kernel_serv->readq);
	hisp150_notify_rpmsg_cb();
}

/* Function declaration */
/**********************************************
 * Power up CSI/DPHY/sensor according to isp req
 * Only called by hisp150_rpmsg_ept_cb when api_name
 * is POWER_REQ, and will send a POWER_RSP to isp
 * after power request done.
 *********************************************/

static int hisp150_rpmsg_ept_cb(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 src)

{
	struct rpmsg_hisp150_service *kernel_serv = rpmsg_local.kernel_isp_serv;
	hisp_msg_t *msg = NULL;
	struct rpmsg_hdr *rpmsg_msg = NULL;

	cam_debug("Enter %s\n", __func__);
	hisp_recvin((void*)data);
	if (kernel_serv == NULL) {
		cam_err("func %s: kernel_serv is NULL", __func__);
		return -EINVAL;
	}
	if (data == NULL) {
		cam_err("func %s: data is NULL", __func__);
		return -EINVAL;
	}

	hisp_assert(len > 0);

	if (kernel_serv->state != RPMSG_CONNECTED) {
		hisp_assert(RPMSG_UNCONNECTED == kernel_serv->state);
		rpmsg_msg = container_of(data, struct rpmsg_hdr, data);
		cam_info("msg src.%u, msg dst.%u ", rpmsg_msg->src,
			rpmsg_msg->dst);

		/* add instance dst and modify the instance state */
		kernel_serv->dst = rpmsg_msg->src;
		kernel_serv->state = RPMSG_CONNECTED;
	}

	msg = (hisp_msg_t*)(data);
	/* save the data and wait for hisp150_recv_rpmsg to get the data */
	hisp_recvx(data);
	hisp150_save_rpmsg_data(data, len);
	return 0;
}

char const *hisp150_get_name(hisp_intf_t *i)
{
	hisp150_t *hi = NULL;
	hisp_assert(i != NULL);
	hi = I2HI(i);
	if (hi == NULL) {
		cam_err("func %s: hi is NULL", __func__);
		return NULL;
	}
	return hi->name;
}

static int hisp150_unmap_a7isp_addr(void *cfg)
{
#ifdef CONFIG_KERNEL_CAMERA_ISP_SECURE
	struct hisp_cfg_data *pcfg = NULL;

	if (hisp150_is_secure_supported() == false)
		return -ENODEV;

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}

	pcfg = (struct hisp_cfg_data*)cfg;

	cam_info("func %s: a7 %x, size %x",
		__func__, pcfg->param.moduleAddr, pcfg->param.size);
	a7_mmu_unmap(pcfg->param.moduleAddr, pcfg->param.size);
	return 0;
#else
	return -ENODEV;
#endif
}

static int hisp150_get_a7isp_addr(void *cfg)
{
	int ret = -ENODEV;
#ifdef CONFIG_KERNEL_CAMERA_ISP_SECURE
	struct hisp_cfg_data *pcfg = NULL;
	struct scatterlist *sg;
	struct sg_table *table;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;
#else
	struct ion_handle *hdl = NULL;
#endif
	cam_info("func %s: enter", __func__);
	if (hisp150_is_secure_supported() == false)
		return -ENODEV;

	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data*)cfg;

	mutex_lock(&kernel_rpmsg_service_mutex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	ret = hisp_get_sg_table(pcfg->param.sharedFd,
		&(s_hisp150.pdev->dev), &buf, &attach, &table);
	if (ret < 0) {
		cam_err("func %s: get_sg_table failed", __func__);
		goto err_ion_client;
	}
#else
	if (IS_ERR_OR_NULL(s_hisp150.ion_client)) {
		cam_err("func %s: s_hisp150.ion_client is NULL or ERR",
			__func__);
		goto err_ion_client;
	}
	hdl = ion_import_dma_buf(s_hisp150.ion_client, pcfg->param.sharedFd);
	if (IS_ERR_OR_NULL(hdl)) {
		cam_err("failed to create ion handle");
		goto err_ion_client;
	}
	cam_info("func %s: import ok", __func__);

	table = ion_sg_table(s_hisp150.ion_client, hdl);
	if (IS_ERR_OR_NULL(table)) {
		cam_err("%s Failed : ion_sg_table.%lu ", __func__,
			PTR_ERR(table));
		goto err_ion_sg_table;
	}
	cam_info("func %s: ion_sg_table ok", __func__);
#endif

	sg = table->sgl;

	pcfg->param.moduleAddr = a7_mmu_map(sg, pcfg->param.size,
		pcfg->param.prot, pcfg->param.type);
	cam_info("func %s: a7 %x", __func__, pcfg->param.moduleAddr);

	ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	hisp_free_dma_buf(&buf,&attach,&table);
#else
err_ion_sg_table:
	ion_free(s_hisp150.ion_client, hdl);
#endif
err_ion_client:
	mutex_unlock(&kernel_rpmsg_service_mutex);
#endif
	return ret;
}

static int hisp150_config(hisp_intf_t *i, void *cfg)
{
	int rc = 0;
	hisp150_t *hi = NULL;
	struct hisp_cfg_data *pcfg = NULL;

	hisp_assert(i != NULL);
	cam_info("%s enter ", __func__);
	if (cfg == NULL) {
		cam_err("func %s: cfg is NULL", __func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data*)cfg;
	hi = I2HI(i);
	hisp_assert(hi != NULL);

	switch (pcfg->cfgtype) {
	case HISP_CONFIG_POWER_ON:
		if (!remote_processor_up) {
			if (pcfg->isSecure == 0) {
				hisp_set_boot_mode(NONSEC_CASE);
			} else if (pcfg->isSecure == 1) {
				cam_info("%s secure mode ", __func__);
				hisp_set_boot_mode(SEC_CASE);
			} else {
				cam_info("%s invalid mode ", __func__);
			}
			cam_notice("%s power on the hisp150", __func__);
			rc = hisp150_power_on(i);
		} else {
			cam_warn("%s hisp150 is still on power-on state, power off it",
				__func__);
			rc = hisp150_power_off(i);
			if (rc != 0) {
				break;
			}

			cam_warn("%s begin to power on the hisp150",
				__func__);
			rc = hisp150_power_on(i);
		}
		break;
	case HISP_CONFIG_POWER_OFF:
		if (remote_processor_up) {
			cam_notice("%s power off the hisp150", __func__);
			rc = hisp150_power_off(i);
		}
		break;
	case HISP_CONFIG_GET_MAP_ADDR:
		rc = hisp150_get_a7isp_addr(cfg);
		cam_info("%s get a7 map address 0x%x ", __func__,
			pcfg->param.moduleAddr);
		break;
	case HISP_CONFIG_UNMAP_ADDR:
		cam_info("%s unmap a7 address from isp atf ", __func__);
		rc = hisp150_unmap_a7isp_addr(cfg);
		break;
	case HISP_CONFIG_PROC_TIMEOUT:
		cam_info("%s message_id.0x%x", __func__, pcfg->cfgdata[0]);
		hisp_dump_rpmsg_with_id(pcfg->cfgdata[0]);
		break;
	default:
		break;
	}
	return rc;
}

static int hisp150_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
	bool ion_client_created = false;
	hisp150_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ? 30000 : 15000;

	struct rpmsg_hisp150_service *kernel_serv = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	struct rpmsg_channel_info chinfo = {
		.src = RPMSG_ADDR_ANY,
	};
#endif
	if (i == NULL)
		return -1;
	hi = I2HI(i);

	cam_info("%s enter ... ", __func__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!hisp_power_wakelock.active) {
		__pm_stay_awake(&hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock ", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);

	mutex_lock(&kernel_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_default)) {
				rc = pinctrl_select_state(hi->dt.pinctrl,
					hi->dt.pinctrl_default);
				if (rc != 0)
					goto FAILED_RET;
			}
		}

		hisp_rpmsgrefs_reset();
		rc = hisp_rproc_enable();
		if (rc != 0)
			goto FAILED_RET;
		rproc_enabled = true;

		rc = wait_for_completion_timeout(&rpmsg_local.isp_comp,
			msecs_to_jiffies(timeout));
		if (rc == 0) {
			rc = -ETIME;
			hisp_boot_stat_dump();
			goto FAILED_RET;
		} else {
			cam_info("%s() %d after wait completion, rc = %d",
				__func__, __LINE__, rc);
			rc = 0;
		}

		atomic_inc(&hi->opened);
		hi_opened = true;
	} else {
		cam_notice("%s isp has been opened", __func__);
	}
	remote_processor_up = true;
	kernel_serv = rpmsg_local.kernel_isp_serv;
	if (!kernel_serv) {
		rc = -ENODEV;
		goto FAILED_RET;
	}

	/* assign a new, unique, local address and associate instance with it */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	kernel_serv->ept = rpmsg_create_ept(kernel_serv->rpdev,
		hisp150_rpmsg_ept_cb, kernel_serv, chinfo);
#else
	kernel_serv->ept = rpmsg_create_ept(kernel_serv->rpdev,
		hisp150_rpmsg_ept_cb, kernel_serv, RPMSG_ADDR_ANY);
#endif
	if (!kernel_serv->ept) {
		kernel_serv->state = RPMSG_FAIL;
		rc = -ENOMEM;
		goto FAILED_RET;
	}
	cam_info("%s() %d kernel_serv->rpdev:src.%d, dst.%d ",
		__func__, __LINE__,
		kernel_serv->rpdev->src, kernel_serv->rpdev->dst);
	kernel_serv->state = RPMSG_CONNECTED;

	/* set the instance recv_count */
	kernel_serv->recv_count = HISP_SERV_FIRST_RECV;

	hisp150_init_timestamp();

	s_hisp150.ion_client = hisi_ion_client_create("hwcam-hisp150");
	if (IS_ERR_OR_NULL(s_hisp150.ion_client )) {
		cam_err("failed to create ion client! ");
		rc = -ENOMEM;
		goto FAILED_RET;
	}
	ion_client_created = true;
	mutex_unlock(&kernel_rpmsg_service_mutex);
	cam_info("%s exit , power on time:%d... ", __func__,
		jiffies_to_msecs(jiffies - current_jiffies));
	return rc;

FAILED_RET:
	if (hi_opened)
		atomic_dec(&hi->opened);

	if (rproc_enabled) {
		hisp_rproc_disable();
		rproc_set_sync_flag(true);
	}

	if (ion_client_created) {
		ion_client_destroy(s_hisp150.ion_client);
		s_hisp150.ion_client = NULL;
	}
	remote_processor_up = false;

	mutex_unlock(&kernel_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_info("%s hisp power on failed, wake unlock ", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
	return rc;
}

static int hisp150_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp150_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	struct rpmsg_hisp150_service *kernel_serv = NULL;
	if (i == NULL)
		return -1;
	hi = I2HI(i);

	cam_info("%s enter ... ", __func__);

	/* check the remote processor boot flow */
	if (remote_processor_up == false) {
		rc = -EPERM;
		goto RET;
	}

	kernel_serv = rpmsg_local.kernel_isp_serv;
	if (!kernel_serv) {
		rc = -ENODEV;
		goto RET;
	}

	if (kernel_serv->state == RPMSG_FAIL) {
		rc = -EFAULT;
		goto RET;
	}

	mutex_lock(&kernel_rpmsg_service_mutex);

	if (!kernel_serv->ept) {
		rc = -ENODEV;
		goto UNLOCK_RET;
	}
	rpmsg_destroy_ept(kernel_serv->ept);
	kernel_serv->ept = NULL;

	kernel_serv->state = RPMSG_UNCONNECTED;
	kernel_serv->recv_count = HISP_SERV_FIRST_RECV;

	if (atomic_read((&hi->opened))) {
		hisp_rproc_disable();
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_idle)) {
				rc = pinctrl_select_state(hi->dt.pinctrl,
					hi->dt.pinctrl_idle);
				if (rc != 0) {
					// Empty.
				}
			}
		}

		remote_processor_up = false;
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened", __func__);
	}

	hisp150_destroy_timestamp();
UNLOCK_RET:
	if (s_hisp150.ion_client) {
		ion_client_destroy(s_hisp150.ion_client);
		s_hisp150.ion_client = NULL;
	}
	mutex_unlock(&kernel_rpmsg_service_mutex);
RET:
	cam_info("%s exit, power 0ff time:%d... ", __func__,
		jiffies_to_msecs(jiffies - current_jiffies) );

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock ", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex);
	return rc;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static void hisp150_rpmsg_remove(struct rpmsg_device *rpdev)
#else
static void hisp150_rpmsg_remove(struct rpmsg_channel *rpdev)
#endif
{
	struct rpmsg_hisp150_service *kernel_serv = dev_get_drvdata(&rpdev->dev);

	cam_info("%s enter ... ", __func__);

	if (kernel_serv == NULL) {
		cam_err("%s: kernel_serv == NULL", __func__);
		return;
	}

	mutex_destroy(&kernel_serv->send_lock);
	mutex_destroy(&kernel_serv->recv_lock);

	kfree(kernel_serv);
	rpmsg_local.kernel_isp_serv = NULL;
	cam_notice("rpmsg hisi driver is removed ");
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static void hisp150_rpmsg_driver_cb(struct rpmsg_device *rpdev,
	void *data, int len, void *priv, u32 src)
#else
static void hisp150_rpmsg_driver_cb(struct rpmsg_channel *rpdev,
	void *data, int len, void *priv, u32 src)
#endif
{
	cam_info("%s enter ... ", __func__);
	cam_warn("%s() %d uhm, unexpected message", __func__,
		__LINE__);

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
		data, len, true);
}

static int hisp150_send_rpmsg(hisp_intf_t *i, hisp_msg_t *from_user, size_t len)
{
	int rc = 0;
	hisp150_t *hi = NULL;
	struct rpmsg_hisp150_service *kernel_serv = NULL;
	hisp_msg_t *msg = from_user;
	hisp_assert(i != NULL);
	hisp_assert(from_user != NULL);
	hi = I2HI(i);

	cam_debug("%s enter.api_name(0x%x) ", __func__, msg->api_name);

	kernel_serv = rpmsg_local.kernel_isp_serv;
	if (!kernel_serv) {
		cam_err("%s() %d failed: kernel_serv does not exist",
			__func__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (!kernel_serv->ept) {
		cam_err("%s() %d failed:kernel_serv->ept does not exist",
			__func__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	mutex_lock(&kernel_serv->send_lock);
	/* if the msg is the first msg, let's treat it special */
	if (kernel_serv->state != RPMSG_CONNECTED) {
		if (!kernel_serv->rpdev) {
			cam_err("%s() %d failed:kernel_serv->rpdev does not exist", __func__,
				__LINE__);
			rc = -ENODEV;
			goto UNLOCK_RET;
		}
		hisp_sendin(msg);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
		rc = rpmsg_send_offchannel(kernel_serv->ept,
			kernel_serv->ept->addr,
			kernel_serv->rpdev->dst, (void*)msg,
			len);
#else
		rc = rpmsg_send_offchannel(kernel_serv->rpdev,
			kernel_serv->ept->addr,
			kernel_serv->rpdev->dst, (void*)msg,
			len);
#endif
		if (rc)
			cam_err("%s() %d failed: first rpmsg_send_offchannel ret is %d", __func__,
				__LINE__, rc);
		goto UNLOCK_RET;
	}
	hisp_sendin(msg);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	rc = rpmsg_send_offchannel(kernel_serv->ept, kernel_serv->ept->addr,
		kernel_serv->dst, (void*)msg, len);
#else
	rc = rpmsg_send_offchannel(kernel_serv->rpdev, kernel_serv->ept->addr,
		kernel_serv->dst, (void*)msg, len);
#endif
	if (rc) {
		cam_err("%s() %d failed: rpmsg_send_offchannel ret is %d", __func__,
			__LINE__, rc);
		goto UNLOCK_RET;
	}
UNLOCK_RET:
	mutex_unlock(&kernel_serv->send_lock);
RET:
	return rc;
}

static int hisp150_recv_rpmsg(hisp_intf_t *i, hisp_msg_t *user_addr, size_t len)
{
	int rc = len;
	hisp150_t *hi = NULL;
	struct rpmsg_hisp150_service *kernel_serv = NULL;
	struct sk_buff *skb = NULL;
	hisp_msg_t *msg = NULL;
	hisp_assert(i != NULL);
	if (user_addr == NULL) {
		cam_err("func %s: user_addr is NULL", __func__);
		return -1;
	}
	hi = I2HI(i);

	cam_debug("%s enter. ", __func__);

	kernel_serv = rpmsg_local.kernel_isp_serv;
	if (!kernel_serv) {
		cam_err("%s() %d failed: kernel_serv does not exist",
			__func__, __LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (kernel_serv->recv_count == HISP_SERV_FIRST_RECV)
		kernel_serv->recv_count = HISP_SERV_NOT_FIRST_RECV;

	if (mutex_lock_interruptible(&kernel_serv->recv_lock)) {
		cam_err("%s() %d failed: mutex_lock_interruptible",
			__func__, __LINE__);
		rc = -ERESTARTSYS;
		goto RET;
	}

	if (kernel_serv->state != RPMSG_CONNECTED) {
		cam_err("%s() %d kernel_serv->state != RPMSG_CONNECTED",
			__func__, __LINE__);
		rc = -ENOTCONN;
		goto UNLOCK_RET;
	}

	/* nothing to read ? */
	/* check if skb_queue is NULL ? */
	if (skb_queue_empty(&kernel_serv->queue)) {
		mutex_unlock(&kernel_serv->recv_lock);
		cam_err("%s() %d skb_queue is empty", __func__, __LINE__);

		/* otherwise block, and wait for data */
		if (wait_event_interruptible_timeout(kernel_serv->readq,
			(!skb_queue_empty(&kernel_serv->queue) ||
			kernel_serv->state == RPMSG_FAIL), HISP_WAIT_TIMEOUT)) {
			cam_err("%s() %d kernel_serv->state = %d", __func__,
			__LINE__, kernel_serv->state);
			rc = -ERESTARTSYS;
			goto RET;
		}

		if (mutex_lock_interruptible(&kernel_serv->recv_lock)) {
			cam_err("%s() %d failed: mutex_lock_interruptible",
				__func__, __LINE__);
			rc = -ERESTARTSYS;
			goto RET;
		}
	}

	if (kernel_serv->state == RPMSG_FAIL) {
		cam_err("%s() %d state = RPMSG_FAIL", __func__, __LINE__);
		rc = -ENXIO;
		goto UNLOCK_RET;
	}

	skb = skb_dequeue(&kernel_serv->queue);
	if (!skb) {
		cam_err("%s() %d skb is NULL", __func__, __LINE__);
		rc = -EIO;
		goto UNLOCK_RET;
	}

	rc = min((unsigned int)len, skb->len);
	msg = (hisp_msg_t*) (skb->data);
	hisp_recvdone((void*)msg);
	if (msg->api_name == RELEASE_CAMERA_RESPONSE)
		hisp_rpmsgrefs_dump();
	cam_debug("%s: api_name(0x%x) ", __func__, msg->api_name);

	hisp150_handle_msg(msg);
	if (!memcpy_s(user_addr, rc, msg, rc)) {
		rc = -EFAULT;
		cam_err("Fail: %s()%d ret = %d ", __func__, __LINE__, rc);
	}
	kfree_skb(skb);

UNLOCK_RET:
	mutex_unlock(&kernel_serv->recv_lock);
RET:
	return rc;
}

static void hisp150_set_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%d ", __func__,ddr_bandwidth);
	qos_request_ddr_down_record.pm_qos_class = 0;
	pm_qos_add_request(&qos_request_ddr_down_record,
		pM_QOS_MEMORY_THROUGHPUT, ddr_bandwidth);
	current_ddr_bandwidth = ddr_bandwidth;
}

static void hisp150_release_ddrfreq(void)
{
	cam_info("%s enter ", __func__);
	if (current_ddr_bandwidth == 0)
		return;
	pm_qos_remove_request(&qos_request_ddr_down_record);
	current_ddr_bandwidth = 0;
}

static void hisp150_update_ddrfreq(unsigned int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%u ", __func__,ddr_bandwidth);
	if (!atomic_read(&s_hisp150.opened)) {
		cam_info("%s, cam is not opened,so u can not set ddr bandwidth ", __func__);
		return;
	}

	if (current_ddr_bandwidth == 0) {
		hisp150_set_ddrfreq(ddr_bandwidth);
	} else if (current_ddr_bandwidth > 0) {
		pm_qos_update_request(&qos_request_ddr_down_record,
			ddr_bandwidth);
		current_ddr_bandwidth = ddr_bandwidth;
	} else {
		cam_err("%s,current_ddr_bandwidth is invalid ", __func__);
	}
}

static ssize_t hisp_ddr_freq_ctrl_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
	cam_info("enter %s,current_ddr_bandwidth:%d ", __func__,
		current_ddr_bandwidth);

	return snprintf(buf, PAGE_SIZE, "%d ", current_ddr_bandwidth);
}

static ssize_t hisp_ddr_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ddr_bandwidth = 0;
	if (buf == NULL) {
		cam_err("%s,input buffer is invalid ", __func__);
		return -EINVAL;
	}

	ddr_bandwidth = simple_strtol(buf, NULL, 10);
	cam_info("%s enter,ddr_bandwidth:%d ", __func__, ddr_bandwidth);

	if (ddr_bandwidth < 0) {
		cam_err("%s,ddr_bandwidth is invalid ", __func__);
		return -EINVAL;
	} else if (ddr_bandwidth == 0) {
		hisp150_release_ddrfreq();
	} else if (ddr_bandwidth > 0) {
		hisp150_update_ddrfreq(ddr_bandwidth);
	}

	return count;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
static int32_t hisp150_rpmsg_probe(struct rpmsg_device *rpdev)
#else
static int32_t hisp150_rpmsg_probe(struct rpmsg_channel *rpdev)
#endif
{
	int32_t ret = 0;
	struct rpmsg_hisp150_service *kernel_serv = NULL;
	cam_info("%s enter ", __func__);

	if (rpmsg_local.kernel_isp_serv != NULL) {
		cam_notice("%s kernel_serv is already up", __func__);
		goto SERVER_UP;
	}

	kernel_serv = kzalloc(sizeof(*kernel_serv), GFP_KERNEL);
	if (!kernel_serv) {
		cam_err("%s() %d kzalloc failed", __func__, __LINE__);
		ret = -ENOMEM;
		goto ERROR_RET;
	}
	mutex_init(&kernel_serv->send_lock);
	mutex_init(&kernel_serv->recv_lock);
	skb_queue_head_init(&kernel_serv->queue);
	init_waitqueue_head(&kernel_serv->readq);
	kernel_serv->ept = NULL;
	kernel_serv->comp = &rpmsg_local.isp_comp;

	rpmsg_local.kernel_isp_serv = kernel_serv;
SERVER_UP:
	if (kernel_serv == NULL) {
		cam_err("func %s: kernel_serv is NULL", __func__);
		return -1;
	}
	kernel_serv->rpdev = rpdev;
	kernel_serv->state = RPMSG_UNCONNECTED;
	dev_set_drvdata(&rpdev->dev, kernel_serv);

	complete(kernel_serv->comp);

	cam_info("new HISI connection srv channel: %u -> %u",
		rpdev->src, rpdev->dst);
ERROR_RET:
	return ret;
}

static struct rpmsg_device_id rpmsg_hisp150_id_table[] = {
	{.name = "rpmsg-hisi"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp150_id_table);

static const struct of_device_id s_hisp150_dt_match[] = {
	{
	 .compatible = "vendor,chip_isp150",
	 .data = &s_hisp150.intf,
	},
	{},
};

MODULE_DEVICE_TABLE(of, s_hisp150_dt_match);

static struct rpmsg_driver rpmsg_hisp150_driver = {
	.drv.name = KBUILD_MODNAME,
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_hisp150_id_table,
	.probe = hisp150_rpmsg_probe,
	.callback = hisp150_rpmsg_driver_cb,
	.remove = hisp150_rpmsg_remove,
};


#ifdef CONFIG_DFX_DEBUG_FS
static struct device_attribute hisp_ddr_freq_ctrl_attr =
	__ATTR(ddr_freq_ctrl, 0660, hisp_ddr_freq_ctrl_show,
	hisp_ddr_freq_store);
#endif /* CONFIG_DFX_DEBUG_FS */

static int32_t hisp150_platform_probe(struct platform_device *pdev)
{
	int32_t ret = 0;

	cam_info("%s: enter", __func__);
	wakeup_source_init(&hisp_power_wakelock, "hisp_power_wakelock");
	mutex_init(&hisp_wake_lock_mutex);
	ret = hisp_get_dt_data(pdev, &s_hisp150.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed", __func__);
		goto error;
	}

	init_completion(&rpmsg_local.isp_comp);
	ret = hisp_register(pdev, &s_hisp150.intf, &s_hisp150.notify);
	if (ret == 0) {
		atomic_set(&s_hisp150.opened, 0);
	} else {
		cam_err("%s() %d hisp_register failed with ret %d", __func__,
			__LINE__, ret);
		goto error;
	}

	rpmsg_local.kernel_isp_serv = NULL;

	ret = register_rpmsg_driver(&rpmsg_hisp150_driver);
	if (ret != 0) {
		cam_err("%s() %d register_rpmsg_driver failed with ret %d",
			__func__, __LINE__, ret);
		goto error;
	}

	s_hisp150.pdev = pdev;
	s_hisp150.ion_client = NULL;

#ifdef CONFIG_DFX_DEBUG_FS
	ret = device_create_file(&pdev->dev, &hisp_ddr_freq_ctrl_attr);
	if (ret < 0) {
		cam_err("%s failed to creat hisp ddr freq ctrl attribute",
		__func__);
		unregister_rpmsg_driver(&rpmsg_hisp150_driver);
		hisp_unregister(s_hisp150.pdev);
		goto error;
	}
#endif
	return 0;

error:
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
	cam_notice("%s exit with ret = %d ", __func__, ret);
	return ret;
}

static struct platform_driver
s_hisp150_driver = {
	.probe = hisp150_platform_probe,
	.driver = {
		.name = "vendor,chip_isp150",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp150_dt_match,
	},
};

static int __init hisp150_init_module(void)
{
	cam_notice("%s enter ", __func__);
	return platform_driver_register(&s_hisp150_driver);
}

static void __exit hisp150_exit_module(void)
{
	cam_notice("%s enter ", __func__);
	hisp_unregister(s_hisp150.pdev);
	platform_driver_unregister(&s_hisp150_driver);
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp150_init_module);
module_exit(hisp150_exit_module);
MODULE_DESCRIPTION("hisp150 driver");
MODULE_LICENSE("GPL v2");
