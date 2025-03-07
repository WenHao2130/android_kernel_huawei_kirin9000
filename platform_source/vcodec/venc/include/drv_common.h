/*
 * drv_common.h
 *
 * This is for VENC Driver Common Structure.
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <linux/platform_device.h>
#include "securec.h"
#include "vcodec_type.h"
#include "drv_venc_osal.h"
#include "drv_venc_ioctl.h"
#include "power_manager.h"

#ifdef PCIE_LINK
#define INTERRUPT_TIMEOUT_MS 100000
#define ENCODE_DONE_TIMEOUT_MS 110000
#define WAIT_CORE_IDLE_TIMEOUT_MS 110000
#define FPGA_WAIT_EVENT_TIME_MS 110000
#else
#define INTERRUPT_TIMEOUT_MS 300
#define ENCODE_DONE_TIMEOUT_MS 2000
#define WAIT_CORE_IDLE_TIMEOUT_MS 1000
#define FPGA_WAIT_EVENT_TIME_MS 1000000
#endif

#define MAX_OPEN_COUNT 4
#define MAX_CHANNEL_NUM 16
#define MAX_RING_BUFFER_SIZE (MAX_CHANNEL_NUM * MAX_SUPPORT_CORE_NUM)
#define align_up(val, align) (((val) + ((align)-1)) & ~((align)-1))
#ifndef unused
#define unused(x) (void)(x)
#endif

enum {
	VEDU_H265 = 0,
	VEDU_H264 = 1
};

typedef enum {
	VENC_CORE_0,
	VENC_CORE_1,
	MAX_SUPPORT_CORE_NUM,
} venc_core_id_t;

typedef enum {
	VENC_POWER_OFF,
	VENC_IDLE,
	VENC_BUSY,
	VENC_TIME_OUT,
} venc_ip_status_t;

enum {
	CMDLIST_DISABLE = 0,
	CMDLIST_DUMP_DATA,
	PRINT_ENC_COST_TIME,
	PRINT_HARDWARE_USAGE,
	LOW_POWER_DISABLE,
	MCORE_ENABLE,
};

#ifdef VENC_DPM_ENABLE
enum {
	DP_MONIT_MOUDLE,
	SMMU_PRE_MODULE,
	MAX_INNER_MODULE,
};
#endif

struct venc_fifo_buffer {
	spinlock_t *lock;
	vedu_osal_event_t event;
	DECLARE_KFIFO_PTR(fifo, struct encode_done_info);
};

struct venc_irq_num {
	uint32_t  normal;
	uint32_t  protect;
	uint32_t  safe;
	uint32_t  mcore_normal;
	uint32_t  mcore_safe;
};

/* VENC IP Context */
struct venc_context {
	spinlock_t lock;
	venc_ip_status_t status;
	struct venc_fifo_buffer *buffer;
	struct channel_info  channel;
	uint32_t  *reg_base;
	uint8_t *intr_hub_reg_base;
	struct venc_irq_num  irq_num;
	bool first_cfg_flag;
	bool is_block;
	bool is_protected;
	venc_timer_t timer;
	struct power_manager pm;
	uint64_t start_time;
	int32_t core_id;
	uint64_t min_time;
	uint64_t max_time;
	uint64_t total_time;
	uint64_t total_number;
};

struct encode_ops {
	int32_t (*init)(void);
	void (*deinit)(void);
	int32_t (*encode)(struct encode_info*, struct venc_fifo_buffer*);
	void (*encode_done)(struct venc_context*);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	void (*encode_timeout)(struct timer_list *);
#else
	void (*encode_timeout)(unsigned long);
#endif
};

struct clock_backup_info {
	struct mutex lock;
	struct clock_info info;
};

typedef struct {
	dev_t dev;
	struct device *venc_device;
	struct cdev cdev;
	struct class *venc_class;
	struct semaphore hw_sem; /* hardware lock */
	struct venc_context ctx[MAX_SUPPORT_CORE_NUM];
	vedu_osal_event_t event;
	struct encode_ops ops;
	struct clock_backup_info backup_info;
	uint32_t debug_flag;
	uint32_t popf_switch; /* switch power off per frame */
	uint32_t *intr_hub_reg_base;
	uint32_t *mcore_image;
} venc_entry_t;

struct platform_device *venc_get_device(void);
int32_t venc_check_coreid(int32_t core_id);
void venc_set_device(struct platform_device *pdev);
irqreturn_t venc_drv_encode_done(int32_t irq, void *dev_id);
irqreturn_t venc_drv_mcore_irq(int32_t irq, void *dev_id);
int32_t venc_drv_get_encode_done_info(struct venc_fifo_buffer *buffer, struct encode_done_info *encode_done_info);

#endif
