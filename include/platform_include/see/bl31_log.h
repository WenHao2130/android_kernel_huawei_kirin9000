/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: bl31_log.h.
 * Create: 2021-06-26
 */

#ifndef __BL31_LOG_H__
#define __BL31_LOG_H__

#ifdef CONFIG_ATF_KERNEL_BL31_LOG
extern int bl31_logbuf_read(char *read_buf, u64 buf_size);
#else
static inline int bl31_logbuf_read(char *read_buf, u64 buf_size)
{
	return -1;
}
#endif

#endif
