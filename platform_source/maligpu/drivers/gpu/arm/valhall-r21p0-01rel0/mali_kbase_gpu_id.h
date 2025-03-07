/*
 *
 * (C) COPYRIGHT 2015-2019 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */
#ifndef _KBASE_GPU_ID_H_
#define _KBASE_GPU_ID_H_

/* GPU_ID register */
#define GPU_ID_VERSION_STATUS_SHIFT       0
#define GPU_ID_VERSION_MINOR_SHIFT        4
#define GPU_ID_VERSION_MAJOR_SHIFT        12
#define GPU_ID_VERSION_PRODUCT_ID_SHIFT   16
#define GPU_ID_VERSION_STATUS             (0xFu  << GPU_ID_VERSION_STATUS_SHIFT)
#define GPU_ID_VERSION_MINOR              (0xFFu << GPU_ID_VERSION_MINOR_SHIFT)
#define GPU_ID_VERSION_MAJOR              (0xFu  << GPU_ID_VERSION_MAJOR_SHIFT)
#define GPU_ID_VERSION_PRODUCT_ID  (0xFFFFu << GPU_ID_VERSION_PRODUCT_ID_SHIFT)
#define TRYM_GPU_CORE_NUM                 6

#define GPU_ID2_VERSION_STATUS_SHIFT      0
#define GPU_ID2_VERSION_MINOR_SHIFT       4
#define GPU_ID2_VERSION_MAJOR_SHIFT       12
#define GPU_ID2_PRODUCT_MAJOR_SHIFT       16
#define GPU_ID2_ARCH_REV_SHIFT            20
#define GPU_ID2_ARCH_MINOR_SHIFT          24
#define GPU_ID2_ARCH_MAJOR_SHIFT          28
#define GPU_ID2_VERSION_STATUS            (0xFu << GPU_ID2_VERSION_STATUS_SHIFT)
#define GPU_ID2_VERSION_MINOR             (0xFFu << GPU_ID2_VERSION_MINOR_SHIFT)
#define GPU_ID2_VERSION_MAJOR             (0xFu << GPU_ID2_VERSION_MAJOR_SHIFT)
#define GPU_ID2_PRODUCT_MAJOR             (0xFu << GPU_ID2_PRODUCT_MAJOR_SHIFT)
#define GPU_ID2_ARCH_REV                  (0xFu << GPU_ID2_ARCH_REV_SHIFT)
#define GPU_ID2_ARCH_MINOR                (0xFu << GPU_ID2_ARCH_MINOR_SHIFT)
#define GPU_ID2_ARCH_MAJOR                (0xFu << GPU_ID2_ARCH_MAJOR_SHIFT)
#define GPU_ID2_PRODUCT_MODEL  (GPU_ID2_ARCH_MAJOR | GPU_ID2_PRODUCT_MAJOR)
#define GPU_ID2_VERSION        (GPU_ID2_VERSION_MAJOR | \
								GPU_ID2_VERSION_MINOR | \
								GPU_ID2_VERSION_STATUS)

/* Helper macro to create a partial GPU_ID (new format) that defines
   a product ignoring its version. */
#define GPU_ID2_PRODUCT_MAKE(arch_major, arch_minor, arch_rev, product_major) \
		((((u32)arch_major) << GPU_ID2_ARCH_MAJOR_SHIFT)  | \
		 (((u32)arch_minor) << GPU_ID2_ARCH_MINOR_SHIFT)  | \
		 (((u32)arch_rev) << GPU_ID2_ARCH_REV_SHIFT)      | \
		 (((u32)product_major) << GPU_ID2_PRODUCT_MAJOR_SHIFT))

/* Helper macro to create a partial GPU_ID (new format) that specifies the
   revision (major, minor, status) of a product */
#define GPU_ID2_VERSION_MAKE(version_major, version_minor, version_status) \
		((((u32)version_major) << GPU_ID2_VERSION_MAJOR_SHIFT)  | \
		 (((u32)version_minor) << GPU_ID2_VERSION_MINOR_SHIFT)  | \
		 (((u32)version_status) << GPU_ID2_VERSION_STATUS_SHIFT))

/* Helper macro to create a complete GPU_ID (new format) */
#define GPU_ID2_MAKE(arch_major, arch_minor, arch_rev, product_major, \
	version_major, version_minor, version_status) \
		(GPU_ID2_PRODUCT_MAKE(arch_major, arch_minor, arch_rev, \
			product_major) | \
		 GPU_ID2_VERSION_MAKE(version_major, version_minor,     \
			version_status))

/* Helper macro to create a partial GPU_ID (new format) that identifies
   a particular GPU model by its arch_major and product_major. */
#define GPU_ID2_MODEL_MAKE(arch_major, product_major) \
		((((u32)arch_major) << GPU_ID2_ARCH_MAJOR_SHIFT)  | \
		(((u32)product_major) << GPU_ID2_PRODUCT_MAJOR_SHIFT))

/* Strip off the non-relevant bits from a product_id value and make it suitable
   for comparison against the GPU_ID2_PRODUCT_xxx values which identify a GPU
   model. */
#define GPU_ID2_MODEL_MATCH_VALUE(product_id) \
		((((u32)product_id) << GPU_ID2_PRODUCT_MAJOR_SHIFT) & \
		    GPU_ID2_PRODUCT_MODEL)

#define GPU_ID2_PRODUCT_TMIX              GPU_ID2_MODEL_MAKE(6, 0)
#define GPU_ID2_PRODUCT_THEX              GPU_ID2_MODEL_MAKE(6, 1)
#define GPU_ID2_PRODUCT_TSIX              GPU_ID2_MODEL_MAKE(7, 0)
#define GPU_ID2_PRODUCT_TDVX              GPU_ID2_MODEL_MAKE(7, 3)
#define GPU_ID2_PRODUCT_TNOX              GPU_ID2_MODEL_MAKE(7, 1)
#define GPU_ID2_PRODUCT_TGOX              GPU_ID2_MODEL_MAKE(7, 2)
#define GPU_ID2_PRODUCT_TTRX              GPU_ID2_MODEL_MAKE(9, 0)
#define GPU_ID2_PRODUCT_TNAX              GPU_ID2_MODEL_MAKE(9, 1)
#define GPU_ID2_PRODUCT_TBEX              GPU_ID2_MODEL_MAKE(9, 2)
#define GPU_ID2_PRODUCT_LBEX              GPU_ID2_MODEL_MAKE(9, 4)
#define GPU_ID2_PRODUCT_TULX              GPU_ID2_MODEL_MAKE(10, 0)
#define GPU_ID2_PRODUCT_TDUX              GPU_ID2_MODEL_MAKE(10, 1)
#define GPU_ID2_PRODUCT_TODX              GPU_ID2_MODEL_MAKE(10, 2)
#define GPU_ID2_PRODUCT_TIDX              GPU_ID2_MODEL_MAKE(10, 3)
#define GPU_ID2_PRODUCT_TVAX              GPU_ID2_MODEL_MAKE(10, 4)
#define GPU_ID2_PRODUCT_LODX              GPU_ID2_MODEL_MAKE(10, 5)

/* Helper macro to create a GPU_ID assuming valid values for id, major,
   minor, status */
#define GPU_ID_MAKE(id, major, minor, status) \
		((((u32)id) << GPU_ID_VERSION_PRODUCT_ID_SHIFT) | \
		(((u32)major) << GPU_ID_VERSION_MAJOR_SHIFT) |   \
		(((u32)minor) << GPU_ID_VERSION_MINOR_SHIFT) |   \
		(((u32)status) << GPU_ID_VERSION_STATUS_SHIFT))

#endif /* _KBASE_GPU_ID_H_ */
