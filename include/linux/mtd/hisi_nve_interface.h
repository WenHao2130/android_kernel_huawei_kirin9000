/* Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: define struct hisi_nve_info_user and declear function
 * hisi_nve_direct_access that will be used by other functions or files.
 * if you want to visit NV partition, i.e. read NV items or write NV
 * items in other files, you should include this .h file.
 * Author: hisilicon
 * Create: 2012-06-20
 * Revision history:2019-06-20 NVE CSEC
 */

#ifndef __HISI_NVE_INTERFACE_H__
#define __HISI_NVE_INTERFACE_H__

#include <linux/types.h>
#include "platform_include/basicplatform/linux/nve/nve_ap_kernel_interface.h"
#define NV_NAME_LENGTH 8     /* NV name maximum length */
#define NVE_NV_DATA_SIZE 104 /* NV data maximum length */

#define NV_WRITE 0 /* NV write operation */
#define NV_READ 1  /* NV read  operation */

struct hisi_nve_info_user {
	uint32_t nv_operation;
	uint32_t nv_number;
	char nv_name[NV_NAME_LENGTH];
	uint32_t valid_size;
	u_char nv_data[NVE_NV_DATA_SIZE];
};

int hisi_nve_direct_access(struct hisi_nve_info_user *user_info);

#endif
