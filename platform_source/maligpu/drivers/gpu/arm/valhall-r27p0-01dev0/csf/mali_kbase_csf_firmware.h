/*
 *
 * (C) COPYRIGHT 2018-2020 ARM Limited. All rights reserved.
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

#ifndef _KBASE_CSF_FIRMWARE_H_
#define _KBASE_CSF_FIRMWARE_H_

#include "device/mali_kbase_device.h"
#include "mali_gpu_csf_registers.h"

/*
 * PAGE_KERNEL_RO was only defined on 32bit ARM in 4.19 in:
 * Commit a3266bd49c721e2e0a71f352d83713fbd60caadb
 * Author: Luis R. Rodriguez <mcgrof@kernel.org>
 * Date:   Fri Aug 17 15:46:29 2018 -0700
 *
 * mm: provide a fallback for PAGE_KERNEL_RO for architectures
 *
 * Some architectures do not define certain PAGE_KERNEL_* flags, this is
 * either because:
 *
 * a) The way to implement some of these flags is *not yet ported*, or
 * b) The architecture *has no way* to describe them
 *
 * [snip]
 *
 * This can be removed once support of 32bit ARM kernels predating 4.19 is no
 * longer required.
 */
#ifndef PAGE_KERNEL_RO
#define PAGE_KERNEL_RO PAGE_KERNEL
#endif

/* Address space number to claim for the firmware. */
#define MCU_AS_NR 0
#define MCU_AS_BITMASK (1 << MCU_AS_NR)

/* Number of available Doorbells */
#define CSF_NUM_DOORBELL ((u8)24)

/* Offset to the first HW doorbell page */
#define CSF_HW_DOORBELL_PAGE_OFFSET ((u32)0x80000)

/* Size of HW Doorbell page, used to calculate the offset to subsequent pages */
#define CSF_HW_DOORBELL_PAGE_SIZE ((u32)0x10000)

/* Doorbell 0 is used by the driver. */
#define CSF_KERNEL_DOORBELL_NR ((u32)0)

/* Offset of name inside a trace buffer entry in the firmware image */
#define TRACE_BUFFER_ENTRY_NAME_OFFSET (0x1C)

/* All implementations of the host interface with major version 0 must comply
 * with these restrictions:
 */
/* GLB_GROUP_NUM: At least 3 command stream groups, but no more than 31 */
#define MIN_SUPPORTED_CSGS 3
#define MAX_SUPPORTED_CSGS 31
/* GROUP_STREAM_NUM: At least 8 command streams per CSG, but no more than 32 */
#define MIN_SUPPORTED_STREAMS_PER_GROUP 8
/* Maximum command streams per csg. */
#define MAX_SUPPORTED_STREAMS_PER_GROUP 32

struct kbase_device;


/**
 * struct kbase_csf_mapping - Memory mapping for CSF memory.
 * @phys:      Physical memory allocation used by the mapping.
 * @cpu_addr:  Starting CPU address for the mapping.
 * @va_reg:    GPU virtual address region for the mapping.
 * @num_pages: Size of the mapping, in memory pages.
 */
struct kbase_csf_mapping {
	struct tagged_addr *phys;
	void *cpu_addr;
	struct kbase_va_region *va_reg;
	unsigned int num_pages;
};

/**
 * struct kbase_csf_trace_buffers - List and state of firmware trace buffers.
 * @list:       List of trace buffers descriptors.
 * @mcu_rw:     Metadata for the MCU shared memory mapping used for
 *              GPU-readable,writable/CPU-writable variables.
 * @mcu_write:  Metadata for the MCU shared memory mapping used for
 *              GPU-writable/CPU-readable variables.
 */
struct kbase_csf_trace_buffers {
	struct list_head list;
	struct kbase_csf_mapping mcu_rw;
	struct kbase_csf_mapping mcu_write;
};

/**
 * struct kbase_csf_cmd_stream_info - Command stream interface provided by the
 *                                    firmware.
 *
 * @kbdev: Address of the instance of a GPU platform device that implements
 *         this interface.
 * @features: Bit field of command stream features (e.g. which types of jobs
 *            are supported). Bits 7:0 specify the number of work registers(-1).
 *            Bits 11:8 specify the number of scoreboard entries(-1).
 * @input: Address of command stream interface input page.
 * @output: Address of command stream interface output page.
 */
struct kbase_csf_cmd_stream_info {
	struct kbase_device *kbdev;
	u32 features;
	void *input;
	void *output;
};

/**
 * kbase_csf_firmware_cs_input() - Set a word in a command stream's input page
 *
 * @info: Command stream interface provided by the firmware.
 * @offset: Offset of the word to be written, in bytes.
 * @value: Value to be written.
 */
void kbase_csf_firmware_cs_input(
	const struct kbase_csf_cmd_stream_info *info, u32 offset, u32 value);

/**
 * kbase_csf_firmware_cs_input_read() - Read a word in a command stream's input
 *                                      page
 *
 * Return: Value of the word read from the command stream's input page.
 *
 * @info: Command stream interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_cs_input_read(
	const struct kbase_csf_cmd_stream_info *const info, const u32 offset);

/**
 * kbase_csf_firmware_cs_input_mask() - Set part of a word in a command stream's
 *                                      input page
 *
 * @info: Command stream interface provided by the firmware.
 * @offset: Offset of the word to be modified, in bytes.
 * @value: Value to be written.
 * @mask: Bitmask with the bits to be modified set.
 */
void kbase_csf_firmware_cs_input_mask(
	const struct kbase_csf_cmd_stream_info *info, u32 offset,
	u32 value, u32 mask);

/**
 * kbase_csf_firmware_cs_output() - Read a word in a command stream's output
 *                                  page
 *
 * Return: Value of the word read from the command stream's output page.
 *
 * @info: Command stream interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_cs_output(
	const struct kbase_csf_cmd_stream_info *info, u32 offset);
/**
 * struct kbase_csf_cmd_stream_group_info - Command stream group interface
 *                                          provided by the firmware.
 *
 * @kbdev: Address of the instance of a GPU platform device that implements
 *         this interface.
 * @features: Bit mask of features. Reserved bits should be 0, and should
 *            be ignored.
 * @input: Address of global interface input page.
 * @output: Address of global interface output page.
 * @suspend_size: Size in bytes for normal suspend buffer for the command
 *                stream group.
 * @protm_suspend_size: Size in bytes for protected mode suspend buffer
 *                      for the command stream group.
 * @stream_num: Number of command streams in the command stream group.
 * @stream_stride: Stride in bytes in JASID0 virtual address between
 *                 command stream capability structures.
 * @streams: Address of an array of command stream capability structures.
 */
struct kbase_csf_cmd_stream_group_info {
	struct kbase_device *kbdev;
	u32 features;
	void *input;
	void *output;
	u32 suspend_size;
	u32 protm_suspend_size;
	u32 stream_num;
	u32 stream_stride;
	struct kbase_csf_cmd_stream_info *streams;
};

/**
 * kbase_csf_firmware_csg_input() - Set a word in a command stream group's
 *                                  input page
 *
 * @info: Command stream group interface provided by the firmware.
 * @offset: Offset of the word to be written, in bytes.
 * @value: Value to be written.
 */
void kbase_csf_firmware_csg_input(
	const struct kbase_csf_cmd_stream_group_info *info, u32 offset,
	u32 value);

/**
 * kbase_csf_firmware_csg_input_read() - Read a word in a command stream group's
 *                                       input page
 *
 * Return: Value of the word read from the command stream group's input page.
 *
 * @info: Command stream group interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_csg_input_read(
	const struct kbase_csf_cmd_stream_group_info *info, u32 offset);

/**
 * kbase_csf_firmware_csg_input_mask() - Set part of a word in a command stream
 *                                       group's input page
 *
 * @info: Command stream group interface provided by the firmware.
 * @offset: Offset of the word to be modified, in bytes.
 * @value: Value to be written.
 * @mask: Bitmask with the bits to be modified set.
 */
void kbase_csf_firmware_csg_input_mask(
	const struct kbase_csf_cmd_stream_group_info *info, u32 offset,
	u32 value, u32 mask);

/**
 * kbase_csf_firmware_csg_output()- Read a word in a command stream group's
 *                                  output page
 *
 * Return: Value of the word read from the command stream group's output page.
 *
 * @info: Command stream group interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_csg_output(
	const struct kbase_csf_cmd_stream_group_info *info, u32 offset);

/**
 * struct kbase_csf_global_iface - Global command stream front-end interface
 *                                 provided by the firmware.
 *
 * @kbdev: Address of the instance of a GPU platform device that implements
 *         this interface.
 * @version: Bits 31:16 hold the major version number and 15:0 hold the minor
 *           version number. A higher minor version is backwards-compatible
 *           with a lower minor version for the same major version.
 * @features: Bit mask of features (e.g. whether certain types of job can
 *            be suspended). Reserved bits should be 0, and should be ignored.
 * @input: Address of global interface input page.
 * @output: Address of global interface output page.
 * @group_num: Number of command stream groups supported.
 * @group_stride: Stride in bytes in JASID0 virtual address between
 *                command stream group capability structures.
 * @prfcnt_size: Performance counters size.
 * @groups: Address of an array of command stream group capability structures.
 */
struct kbase_csf_global_iface {
	struct kbase_device *kbdev;
	u32 version;
	u32 features;
	void *input;
	void *output;
	u32 group_num;
	u32 group_stride;
	u32 prfcnt_size;
	struct kbase_csf_cmd_stream_group_info *groups;
};

/**
 * kbase_csf_firmware_global_input() - Set a word in the global input page
 *
 * @iface: Command stream front-end interface provided by the firmware.
 * @offset: Offset of the word to be written, in bytes.
 * @value: Value to be written.
 */
void kbase_csf_firmware_global_input(
	const struct kbase_csf_global_iface *iface, u32 offset, u32 value);

/**
 * kbase_csf_firmware_global_input_mask() - Set part of a word in the global
 *                                          input page
 *
 * @iface: Command stream front-end interface provided by the firmware.
 * @offset: Offset of the word to be modified, in bytes.
 * @value: Value to be written.
 * @mask: Bitmask with the bits to be modified set.
 */
void kbase_csf_firmware_global_input_mask(
	const struct kbase_csf_global_iface *iface, u32 offset,
	u32 value, u32 mask);

/**
 * kbase_csf_firmware_global_input_read() - Read a word in a global input page
 *
 * Return: Value of the word read from the global input page.
 *
 * @info: Command stream group interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_global_input_read(
	const struct kbase_csf_global_iface *info, u32 offset);

/**
 * kbase_csf_firmware_global_output() - Read a word in the global output page
 *
 * Return: Value of the word read from the global output page.
 *
 * @iface: Command stream front-end interface provided by the firmware.
 * @offset: Offset of the word to be read, in bytes.
 */
u32 kbase_csf_firmware_global_output(
	const struct kbase_csf_global_iface *iface, u32 offset);

/* Calculate the offset to the Hw doorbell page corresponding to the
 * doorbell number.
 */
static u32 csf_doorbell_offset(int doorbell_nr)
{
	WARN_ON(doorbell_nr >= CSF_NUM_DOORBELL);

	return CSF_HW_DOORBELL_PAGE_OFFSET +
		(doorbell_nr * CSF_HW_DOORBELL_PAGE_SIZE);
}

static inline void kbase_csf_ring_doorbell(struct kbase_device *kbdev,
					   int doorbell_nr)
{
	WARN_ON(doorbell_nr >= CSF_NUM_DOORBELL);

	kbase_reg_write(kbdev, csf_doorbell_offset(doorbell_nr), (u32)1);
}

/**
 * kbase_csf_read_firmware_memory - Read a value in a GPU address
 *
 * This function read a value in a GPU address that belongs to
 * a private firmware memory region. The function assumes that the location
 * is not permanently mapped on the CPU address space, therefore it maps it
 * and then unmaps it to access it independently.
 *
 * @kbdev:     Device pointer
 * @gpu_addr:  GPU address to read
 * @value:     output pointer to which the read value will be written.
 */
void kbase_csf_read_firmware_memory(struct kbase_device *kbdev,
	u32 gpu_addr, u32 *value);

/**
 * kbase_csf_update_firmware_memory - Write a value in a GPU address
 *
 * This function writes a given value in a GPU address that belongs to
 * a private firmware memory region. The function assumes that the destination
 * is not permanently mapped on the CPU address space, therefore it maps it
 * and then unmaps it to access it independently.
 *
 * @kbdev:     Device pointer
 * @gpu_addr:  GPU address to write
 * @value:     Value to write
 */
void kbase_csf_update_firmware_memory(struct kbase_device *kbdev,
	u32 gpu_addr, u32 value);

/**
 * kbase_csf_firmware_init() - Load the firmware for the CSF MCU
 *
 * Request the firmware from user space and load it into memory.
 *
 * Return: 0 if successful, negative error code on failure
 *
 * @kbdev: Kbase device
 */
int kbase_csf_firmware_init(struct kbase_device *kbdev);

/**
 * kbase_csf_firmware_term() - Unload the firmware
 *
 * Frees the memory allocated by kbase_csf_firmware_init()
 *
 * @kbdev: Kbase device
 */
void kbase_csf_firmware_term(struct kbase_device *kbdev);

/**
 * kbase_csf_firmware_ping - Send the ping request to firmware.
 *
 * The function sends the ping request to firmware to confirm it is alive.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_firmware_ping(struct kbase_device *kbdev);

/**
 * kbase_csf_firmware_set_timeout - Set a hardware endpoint progress timeout.
 *
 * @kbdev:   Instance of a GPU platform device that implements a command
 *           stream front-end interface.
 * @timeout: The maximum number of GPU cycles that is allowed to elapse
 *           without forward progress before the driver terminates a GPU
 *           command queue group.
 *
 * Configures the progress timeout value used by the firmware to decide
 * when to report that a task is not making progress on an endpoint.
 *
 * Return: 0 on success, or negative on failure.
 */
int kbase_csf_firmware_set_timeout(struct kbase_device *kbdev, u64 timeout);

/**
 * kbase_csf_enter_protected_mode - Send the Global request to firmware to
 *                                  enter protected mode.
 *
 * @kbdev: Instance of a GPU platform device that implements a command
 *         stream front-end interface.
 */
void kbase_csf_enter_protected_mode(struct kbase_device *kbdev);

/**
 * kbase_csf_firmware_reinit() - Reload the CSF firmware on MCU
 *
 * This helper function will reload the firmware image and make MCU restart the
 * execution. It is supposed to be called after MCU(GPU) has been reset.
 * Unlike the initial boot the firmware binary image is not parsed completely.
 * Only the data sections, which were loaded in memory during the initial boot,
 * are re-initialized either by zeroing them or copying their data from the
 * firmware binary image. The memory allocation for the firmware pages and
 * MMU programming is not needed for the reboot, presuming the firmware binary
 * file on the filesystem would not change.
 *
 * Return: 0 if successful, negative error code on failure
 *
 * @kbdev: Kbase device
 */
int kbase_csf_firmware_reinit(struct kbase_device *kbdev);

/**
 * Request the global control block of CSF interface capabilities
 *
 * Return: Total number of command streams, summed across all groups.
 *
 * @kbdev:                 Kbase device.
 * @group_data:            Pointer where to store all the group data
 *                         (sequentially).
 * @max_group_num:         The maximum number of groups to be read.
 *                         Can be 0, in which case group_data is unused.
 * @stream_data:           Pointer where to store all the stream data
 *                         (sequentially).
 * @max_total_stream_num:  The maximum number of streams to be read.
 *                         Can be 0, in which case stream_data is unused.
 * @glb_version:           Where to store the global interface version.
 *                         Bits 31:16 hold the major version number and
 *                         15:0 hold the minor version number.
 *                         A higher minor version is backwards-compatible
 *                         with a lower minor version for the same major
 *                         version.
 * @features:              Where to store a bit mask of features (e.g.
 *                         whether certain types of job can be suspended).
 * @group_num:             Where to store the number of command stream groups
 *                         supported.
 * @prfcnt_size:           Where to store the size of CSF performance counters,
 *                         in bytes. Bits 31:16 hold the size of firmware
 *                         performance counter data and 15:0 hold the size of
 *                         hardware performance counter data.
 */
u32 kbase_csf_firmware_get_glb_iface(struct kbase_device *kbdev,
	struct basep_cs_group_control *group_data, u32 max_group_num,
	struct basep_cs_stream_control *stream_data, u32 max_total_stream_num,
	u32 *glb_version, u32 *features, u32 *group_num, u32 *prfcnt_size);


/**
 * Get CSF firmware header timeline metadata content
 *
 * Return: The firmware timeline metadata content which match @p name.
 *
 * @kbdev:        Kbase device.
 * @name:         Name of the metadata which metadata content to be returned.
 * @size:         Metadata size if specified metadata found.
 */
const char *kbase_csf_firmware_get_timeline_metadata(struct kbase_device *kbdev,
	const char *name, size_t *size);

/**
 * kbase_csf_firmware_mcu_shared_mapping_init -
 * Allocate and map MCU shared memory.
 *
 * This helper function allocates memory and maps it on both the CPU
 * and the GPU address spaces. Most of the properties of the mapping
 * are implicit and will be automatically determined by the function,
 * e.g. whether memory is cacheable.
 *
 * The client is only expected to specify whether the mapping is readable
 * or writable in the CPU and the GPU address spaces; any other flag
 * will be ignored by the function.
 *
 * Return: 0 if success, or an error code on failure.
 *
 * @kbdev:              Kbase device the memory mapping shall belong to.
 * @num_pages:          Number of memory pages to map.
 * @cpu_map_properties: Either PROT_READ or PROT_WRITE.
 * @gpu_map_properties: Either KBASE_REG_GPU_RD or KBASE_REG_GPU_WR.
 * @csf_mapping:        Object where to write metadata for the memory mapping.
 */
int kbase_csf_firmware_mcu_shared_mapping_init(
		struct kbase_device *kbdev,
		unsigned int num_pages,
		unsigned long cpu_map_properties,
		unsigned long gpu_map_properties,
		struct kbase_csf_mapping *csf_mapping);

/**
 * kbase_csf_firmware_mcu_shared_mapping_term - Unmap and free MCU shared memory.
 *
 * @kbdev:       Device pointer.
 * @csf_mapping: Metadata of the memory mapping to terminate.
 */
void kbase_csf_firmware_mcu_shared_mapping_term(
		struct kbase_device *kbdev, struct kbase_csf_mapping *csf_mapping);

#ifndef MALI_KBASE_BUILD
/**
 * mali_kutf_process_fw_utf_entry() - Process the "Firmware UTF tests" section
 *
 * Read "Firmware UTF tests" section from the firmware image and create
 * necessary kutf app+suite+tests.
 *
 * Return: 0 if successful, negative error code on failure. In both cases
 * caller will have to invoke mali_kutf_fw_utf_entry_cleanup for the cleanup
 *
 * @kbdev: Kbase device structure
 * @fw_data: Pointer to the start of firmware binary image loaded from disk
 * @fw_size: Size (in bytes) of the firmware image
 * @entry: Pointer to the start of the section
 */
int mali_kutf_process_fw_utf_entry(struct kbase_device *kbdev,
	const void *fw_data, size_t fw_size, const u32 *entry);

/**
 * mali_kutf_fw_utf_entry_cleanup() - Remove the Fw UTF tests debugfs entries
 *
 * Destroy the kutf apps+suites+tests created on parsing "Firmware UTF tests"
 * section from the firmware image.
 *
 * @kbdev: Kbase device structure
 */
void mali_kutf_fw_utf_entry_cleanup(struct kbase_device *kbdev);
#endif

#ifdef CONFIG_MALI_DEBUG
extern bool fw_debug;
#endif

static inline long kbase_csf_timeout_in_jiffies(const unsigned int msecs)
{
#ifdef CONFIG_MALI_DEBUG
	return (fw_debug ? MAX_SCHEDULE_TIMEOUT : msecs_to_jiffies(msecs));
#else
	return msecs_to_jiffies(msecs);
#endif
}

#endif
