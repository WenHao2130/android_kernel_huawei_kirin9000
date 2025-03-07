/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Implementation of set/get hwdps flag and context.
 * Create: 2022-02-28
 */

#ifndef __HWDPS_CONTEXT_H
#define __HWDPS_CONTEXT_H

#include <linux/fs.h>

s32 f2fs_get_hwdps_attr(struct inode *inode, void *buf, size_t len, u32 flags,
	struct page *page);

s32 f2fs_set_hwdps_attr(struct inode *inode, const void *attr, size_t len,
	void *fs_data);

s32 f2fs_update_hwdps_attr(struct inode *inode, const void *attr,
	size_t len, void *fs_data);

s32 f2fs_get_hwdps_flags(struct inode *inode, void *fs_data, u32 *flags);

s32 f2fs_set_hwdps_flags(struct inode *inode, void *fs_data, u32 *flags);
#endif
