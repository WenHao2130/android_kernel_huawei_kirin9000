/*
 *  SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Date:	  2018-05-17
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
 * Foundation, Inc., 59 Temple Place, Suite 330, MA  02111-1307  USA
 */

#ifndef __HW_KERNEL_CAM_HISP300_CFG_H__
#define __HW_KERNEL_CAM_HISP300_CFG_H__

#include "hisp300_msg.h"
#include "hisp_cfg_base.h"

#define HISP_IOCTL_SEND_RPMSG _IOW('A', BASE_VIDIOC_PRIVATE + 0x03, hisp_msg_t)
#define HISP_IOCTL_RECV_RPMSG _IOR('A', BASE_VIDIOC_PRIVATE + 0x04, hisp_msg_t)

#endif /* __HW_KERNEL_CAM_HISP300_CFG_H__ */
