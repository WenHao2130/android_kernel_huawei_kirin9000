/*
 *
 * Based on the RDR framework, adapt to the AP side to implement resource
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __RDR_DFX_AP_ADAPTER_H__
#define __RDR_DFX_AP_ADAPTER_H__

#include <linux/thread_info.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <platform_include/basicplatform/linux/rdr_ap_hook.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_sctrl_interface.h>

#define REG_NAME_LEN 12
#define PRODUCT_VERSION_LEN 32
#define PRODUCT_DEVICE_LEN 32
#define REGS_DUMP_MAX_NUM   10
#define AP_DUMP_MAGIC   0x19283746
#define BBOX_VERSION    0x1002C /* v1.2.12 */
#define AP_DUMP_END_MAGIC   0x1F2E3D4C
#define SIZE_1K         0x400
#define SYSTEM_BUILD_POP    "/vendor/build.prop"
#define AMNTN_MODULE_NAME_LEN 12
#define AMNTN_MODULE_COMP_LEN 48
#define NMI_NOTIFY_LPM3_ADDR SOC_SCTRL_SCLPMCUCTRL_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#define WDT_KICK_SLICE_TIMES    3
#define FPGA 1
#define KDUMP_SKP_SUCCESS_FLAG    0xAA55
#define KDUMP_SKP_DATASAVE_OFFSET 0x1000

#define PSTORE_PATH            "/proc/rdr/pstore/"
#define FASTBOOT_LOG_FILE      "/proc/rdr/log/fastboot_log"
#define LAST_FASTBOOT_LOG_FILE "/proc/rdr/log/last_fastboot_log"
#define REGS_INFO_NAME_LEN 12

#define SRC_KERNELDUMP  "/proc/rdr/memory/kernel_dump"
#define SRC_LPMCU_DDR_MEMORY  "/proc/rdr/memory/lpmcu_ddr_mem"
#define SRC_BL31_MNTN_MEMORY  "/proc/rdr/memory/bl31_mntn_mem"
#define SRC_ETR_DUMP  "/proc/rdr/memory/etr_dump"
#define SRC_LOGBUFFER_MEMORY  "/proc/rdr/memory/kernel_logbuff"
#define SRC_LOGRINGBUFFER_MEMORY  "/proc/rdr/memory/prb_log_buf"
#define SRC_LOGRINGBUFFER_INFO_MEMORY  "/proc/rdr/memory/prb_log_info"
#define SRC_LOGRINGBUFFER_DESC_MEMORY  "/proc/rdr/memory/prb_log_desc"
#ifdef CONFIG_HHEE
#define SRC_HHEE_MNTN_MEMORY  "/proc/rdr/memory/hhee_mntn_mem"
#endif
#ifdef CONFIG_ARM64_HKRR
#define SRC_HKRR_MNTN_MEMORY  "/proc/rdr/memory/hkrr_obj_mem"
#endif
#define REG_MAP_SIZE 0x4
#define MODID_FAIL 1
#define RDR_CONSOLE_LOGLEVEL_DEFAULT 7
#define RDR_REG_BITS 32
#define SCTRL_ADDR_INDEX 0
#define SCTRL_SIZE_INDEX 1
#define L32BIT_ADDR_INDEX 2
#define H32BIT_ADDR_INDEX 3
#define DTS_32K_TIMER_SIZE 4
#define SRC_LOG_BUFF_LEN (1 << CONFIG_LOG_BUF_SHIFT)
#define DST_LOG_BUFF_LEN (SRC_LOG_BUFF_LEN + (SRC_LOG_BUFF_LEN >> 2))

struct module_dump_mem_info {
	rdr_ap_dump_func_ptr dump_funcptr;
	unsigned char *dump_addr;
	unsigned int dump_size;
	char module_name[AMNTN_MODULE_NAME_LEN];
};

struct regs_info {
	char reg_name[REGS_INFO_NAME_LEN];
	u32 reg_size;
	u64 reg_base;
	void __iomem *reg_map_addr;
	unsigned char *reg_dump_addr;
};

struct ap_eh_root {
	unsigned int dump_magic;
	unsigned char version[PRODUCT_VERSION_LEN];
	u32 modid;
	u32 e_exce_type;
	u32 e_exce_subtype;
	u64 coreid;
	u64 slice;
	struct rdr_register_module_result ap_rdr_info;
	unsigned int enter_times; /* Reentrant count,The initial value is 0,Each entry++ */

	unsigned int num_reg_regions;
	struct regs_info dump_regs_info[REGS_DUMP_MAX_NUM];

	unsigned char *hook_buffer_addr[HK_MAX];

	struct percpu_buffer_info hook_percpu_buffer[HK_PERCPU_TAG];

	unsigned char *last_task_struct_dump_addr[NR_CPUS];
	unsigned char *last_task_stack_dump_addr[NR_CPUS];

	char log_path[LOG_PATH_LEN];
	unsigned char *rdr_ap_area_map_addr; /* Mapping address allocated by the rdr to the AP memory */
	struct module_dump_mem_info module_dump_info[MODU_MAX];
	u64 wdt_kick_slice[WDT_KICK_SLICE_TIMES];
	unsigned char device_id[PRODUCT_DEVICE_LEN];
	unsigned int cpu_count;
	u64 bbox_version; /* Indicates the BBox version */
	unsigned int end_magic; /* End of the tag structure. It is used to determine the scope of the structure */
	char reserved[1];
}; /* The ap_eh_root occupies 2K space and is reserved by using the get_rdr_ap_dump_addr function */

/* Variables that can be used by external */
void get_product_version(char *version, size_t count);
void print_debug_info(void);
int rdr_ap_dump_init(const struct rdr_register_module_result *retinfo);
void rdr_ap_dump(u32 modid, u32 etype, u64 coreid,
				char *log_path, pfn_cb_dump_done pfn_cb);
unsigned int is_reboot_reason_from_modem(void);
u64 get_dfx_ap_addr(void);

#ifdef CONFIG_PM
void rdr_ap_root_backup(void);
void rdr_ap_root_restore(void);
#endif

/* Test function declaration */
void ap_exch_task_stack_dump(int task_pid);
void ap_exch_buf_show(unsigned int offset, unsigned int size);
void ap_exch_hex_dump(unsigned char *buf, unsigned int size,
				unsigned char per_row);
int ap_exch_undef(void);
int ap_exch_swi(void);
int ap_exch_pabt(void);
int ap_exch_dabt(void);
int save_mntndump_log(void *arg);

u32 bbox_test_get_exce_size(void);
int bbox_test_registe_exception(void);
void bbox_test_enable(int enable);
void bbox_test_print_exc(void);
int bbox_test_unregiste_exception(void);
int rdr_ap_cleartext_print(const char *dir_path, u64 log_addr, u32 log_len);
#endif
