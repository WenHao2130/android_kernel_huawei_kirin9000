/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: compat 32 header file.
 * Create: 2016-04-01
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HW_ALAN_KERNEL_COMPAT32_H__
#define __HW_ALAN_KERNEL_COMPAT32_H__

#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <platform_include/camera/native/isp_cfg.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
typedef struct timeval cam_timeval;
typedef struct timespec cam_timespec;
#define cam_access_ok(type, addr, size)     access_ok(type, addr, size)
#define cam_compat_get_timespec(ts, user)   compat_get_timespec(ts, user)
#define cam_compat_put_timespec(ts, user)   compat_put_timespec(ts, user)
#define cam_compat_get_timeval(tv, user)    compat_get_timeval(tv, user)
#define cam_compat_put_timeval(tv, user)    compat_put_timeval(tv, user)
#else
typedef struct timespec64 cam_timeval;
typedef struct timespec64 cam_timespec;
#define cam_access_ok(type, addr, size)     access_ok(addr, size)
#define cam_compat_get_timespec(ts, user)   get_old_timespec32(ts, user)
#define cam_compat_put_timespec(ts, user)   put_old_timespec32(ts, user)
#define cam_compat_get_timeval(tv, user)    get_old_timespec32(tv, user)
#define cam_compat_put_timeval(tv, user)    put_old_timespec32(tv, user)
#endif


struct v4l2_event32 {
	__u32 type;
	union {
		struct v4l2_event_vsync vsync;
		struct v4l2_event_ctrl ctrl;
		struct v4l2_event_frame_sync frame_sync;
		__u8 data[128]; /* 128: array data len */
	} u;
	__u32 pending;
	__u32 sequence;
#ifdef CONFIG_COMPAT
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct compat_timespec timestamp;
#else
	struct old_timespec32 timestamp;
#endif
#else
	struct timespec timestamp;
#endif
	__u32 id;
	__u32 reserved[8]; /* 8,reserved */
};

struct v4l2_clip32 {
	struct v4l2_rect c;
	compat_caddr_t next;
};

struct v4l2_window32 {
	struct v4l2_rect w;
	__u32 field; /* enum v4l2_field */
	__u32 chromakey;
	compat_caddr_t clips; /* actually struct v4l2_clip32 */
	__u32 clipcount;
	compat_caddr_t bitmap;
};

struct v4l2_format32 {
	__u32 type; /* enum v4l2_buf_type */
	union {
		struct v4l2_pix_format pix;
		struct v4l2_pix_format_mplane pix_mp;
		struct v4l2_window32 win;
		struct v4l2_vbi_format vbi;
		struct v4l2_sliced_vbi_format sliced;
		__u8 raw_data[200]; /* 200: user-defined */
	} fmt;
};

typedef struct _tag_hwisp_stream_buf_info32 {
	union {
		void    *user_buffer_handle;
		int64_t _user_buffer_handle;
	};

	uint32_t y_addr_phy;
	uint32_t u_addr_phy;
	uint32_t v_addr_phy;

	uint32_t y_addr_iommu;
	uint32_t u_addr_iommu;
	uint32_t v_addr_iommu;

	int ion_fd;
	union {
		struct ion_handle *ion_vc_hdl;
		int64_t           _ion_vc_hdl;
	};
	union {
		void    *ion_vaddr;
		int64_t _ion_vaddr;
	};

	union {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		struct compat_timeval tv;
#else
		struct old_timespec32 tv;
#endif
		int64_t               _timestamp[2]; /* 2: array len */
	};

	ovisp23_port_info_t port;
} hwisp_stream_buf_info_t32;

typedef struct _tag_cam_buf_status32 {
	int id;
	int buf_status;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct compat_timeval tv;
#else
	struct old_timespec32 tv;
#endif
} cam_buf_status_t32;

#define VIDIOC_G_FMT32 _IOWR('V', 4, struct v4l2_format32)
#define VIDIOC_S_FMT32 _IOWR('V', 5, struct v4l2_format32)

#define VIDIOC_DQEVENT32 _IOR('V', 89, struct v4l2_event32)
#define CAM_V4L2_IOCTL_REQUEST_ACK32 _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x20, struct v4l2_event32)
#define CAM_V4L2_IOCTL_NOTIFY32 _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x21, struct v4l2_event32)
#define CAM_V4L2_IOCTL_THERMAL_GUARD32 _IOWR('A', \
	BASE_VIDIOC_PRIVATE + 0x22, struct v4l2_event32)

#define HWISP_STREAM_IOCTL_ENQUEUE_BUF32 _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x01, hwisp_stream_buf_info_t32)
#define HWISP_STREAM_IOCTL_DEQUEUE_BUF32 _IOR('A', \
	BASE_VIDIOC_PRIVATE + 0x02, hwisp_stream_buf_info_t32)

#define CAM_V4L2_IOCTL_GET_BUF32 _IOR('A', \
	BASE_VIDIOC_PRIVATE + 0x06, cam_buf_status_t32)
#define CAM_V4L2_IOCTL_PUT_BUF32 _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x07, cam_buf_status_t32)
#define CAM_V4L2_IOCTL_BUF_DONE32 _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x08, cam_buf_status_t32)

long compat_get_v4l2_event_data(struct v4l2_event __user *pdata,
	struct v4l2_event32 __user *pdata32);
long compat_put_v4l2_event_data(struct v4l2_event __user *pdata,
	struct v4l2_event32 __user *pdata32);
long compat_get_v4l2_format_data(struct v4l2_format *kp,
	struct v4l2_format32 __user *up);
long compat_put_v4l2_format_data(struct v4l2_format *kp,
	struct v4l2_format32 __user *up);

long compat_get_cam_buf_status_data(cam_buf_status_t __user *pdata,
	cam_buf_status_t32 __user *pdata32);
long compat_put_cam_buf_status_data(cam_buf_status_t __user *pdata,
	cam_buf_status_t32 __user *pdata32);

#endif /* __HW_ALAN_KERNEL_COMPAT32_H__ */
