/*
 *
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_NOC_H
#define __DFX_NOC_H

#include <linux/types.h>

#define MODULE_NAME	"DFX_NOC"
#define MAX_NOC_NODE_NAME_LEN	20
#define GET_NODE_FROM_ARRAY(node, idx)    { node = noc_nodes_array[idx]; }

#define NOC_MAX_IRQ_NR        64
#define MAX_NOC_NODES_NR      33
#define DFX_NOC_CLOCK_MAX     6
#define DFX_NOC_CLOCK_REG_DEFAULT 0xFFFFFFFF
#define MAX_BUSID_VALE         5
#define MAX_FILTER_INITFLOW 20
#define MAX_NAME_LEN 64
#define MAX_DUMP_REG 32
#define MAX_INITFLOW_ARRAY_SIZE         64
#define MAX_TARGETFLOW_ARRAY_SIZE       64
#define MAX_ROUTEID_TBL_SIZE 140
#define NOC_ROUTEID_TBL_FLAG 0x1
#define NOC_ROUTEID_TBL2_FLAG 0x2
#define MAX_SEC_MODE 8
#define MAX_MID_NUM 150
#define MAX_NODE_NAME 256
#define MAX_MODID_NUM 3
#define NOC_MODID_MATCH 0x12345678

#define NOC_PTYPE_UART 1
#define NOC_PTYPE_PSTORE 0

#define NOC_ACPU_INIT_FLOW_ARRY_SIZE 2

#define MAX_NOC_LIST_TARGETS 0x20
#define MAX_NOC_TARGET_NAME_LEN 0x20

#define MASK_PMCTRL_POWER_IDLE 0x000000000000FFFF
#define MASK_PMCTRL_POWER_IDLE1 0x00000000FFFF0000
#define MASK_PMCTRL_POWER_IDLE2 0x0000FFFF00000000
#define PMCTRL_POWER_IDLE_POISION 0
#define PMCTRL_POWER_IDLE_POISION1 1
#define PMCTRL_POWER_IDLE_POISION2 2
#define GET_PMCTRL_POWER_IDLE1 16
#define GET_PMCTRL_POWER_IDLE2 32
#define HIGH_ENABLE_MASK 16
#define MAX_GIVEUP_IDLE_NUM 5
#define PERI_CRG_NUM 2
#define PERI_CRG_TIMEOUT_MASK 0XFFFFFFFF
#define MAX_NOC_REG_BITS 32

/* NOTE: Must the same order with DTS. */
enum NOC_REG_DUMP_NAME {
	NOC_SCTRL_BASE = 0,
	NOC_PCTRL_BASE,
	NOC_PCRGCTRL_BASE,
	NOC_PMCTRL_BASE,
	NOC_MEDIA1_CRG_BASE,
	NOC_MEDIA2_CRG_BASE,
	NOC_MAX_BASE
};

enum NOC_IRQ_TPYE {
	NOC_ERR_PROBE_IRQ,
	NOC_PACKET_PROBE_IRQ,
	NOC_TRANS_PROBE_IRQ,
};

enum IRQ_NUMBER {
	NOC_ERR_PROBE_IRQ_NUM,
	NOC_TIMEOUT_IRQ_NUM,
	QIC_IRQ_NUM,
};

struct dfx_noc_irqd {
	enum NOC_IRQ_TPYE type;
	struct noc_node *node;
};

struct dfx_noc_reg_list {
	unsigned int pctrl_stat0_offset;
	unsigned int pctrl_stat2_offset;
	unsigned int pctrl_stat3_offset;
	unsigned int pctrl_ctrl19_offset;
	unsigned int sctrl_scperstatus6_offset;
	unsigned int pmctrl_int0_stat_offset;
	unsigned int pmctrl_int0_mask_offset;
	unsigned int pmctrl_power_idlereq_offset;
	unsigned int pmctrl_power_idleack_offset;
	unsigned int pmctrl_power_idle_offset;
	unsigned int pmctrl_power_idlereq1_offset;
	unsigned int pmctrl_power_idleack1_offset;
	unsigned int pmctrl_power_idle1_offset;
	unsigned int pmctrl_power_idlereq2_offset;
	unsigned int pmctrl_power_idleack2_offset;
	unsigned int pmctrl_power_idle2_offset;
	unsigned int pmctrl_int1_stat_offset;
	unsigned int pmctrl_int1_mask_offset;
	unsigned int pmctrl_int2_stat_offset;
	unsigned int pmctrl_int2_mask_offset;
};

struct dfx_noc_err_probe_reg {
	unsigned int err_probe_coreid_offset;
	unsigned int err_probe_revisionid_offset;
	unsigned int err_probe_faulten_offset;
	unsigned int err_probe_errvld_offset;
	unsigned int err_probe_errclr_offset;
	unsigned int err_probe_errlog0_offset;
	unsigned int err_probe_errlog1_offset;
	unsigned int err_probe_errlog3_offset;
	unsigned int err_probe_errlog4_offset;
	unsigned int err_probe_errlog5_offset;
	unsigned int err_probe_errlog7_offset;
};


struct dfx_noc_property {
	unsigned long long pctrl_irq_mask;
	unsigned long long smp_stop_cpu_bit_mask;
	const char *stop_cpu_bus_node_name;
	unsigned int apcu_init_flow_array[NOC_ACPU_INIT_FLOW_ARRY_SIZE];
	unsigned int pctrl_peri_stat0_off;
	unsigned int pctrl_peri_stat3_off;
	bool faulten_default_enable;
	unsigned int platform_id;
	unsigned int noc_aobus_timeout_irq;
	unsigned int noc_aobus_second_int_mask;
	unsigned int sctrl_second_int_org_offset;
	unsigned int sctrl_second_int_mask_offset;
	bool noc_aobus_int_enable;
	bool packet_enable;
	bool transcation_enable;
	bool error_enable;
	bool noc_debug;
	bool noc_timeout_enable;
	bool noc_fama_enable;
	unsigned int noc_list_target_nums;
	unsigned int noc_fama_mask;
	u8 __iomem *bus_node_base_errlog1;
	unsigned int peri_int0_mask;
	unsigned int peri_int1_mask;
	unsigned int masterid_mask;
	bool noc_npu_easc3_flag;
};

struct dfx_noc_target_name_list {
	char name[MAX_NOC_TARGET_NAME_LEN];
	u32 base;
	u32 end;
};

struct dfx_noc_device {
	struct device *dev;
	void __iomem *sys_base;
	void __iomem *media1_crg_base;
	void __iomem *media2_crg_base;
	void __iomem *pctrl_base;
	void __iomem *pmctrl_base;
	void __iomem *sctrl_base;
	void __iomem *pwrctrl_reg;
	void __iomem *pcrgctrl_base;
	unsigned int irq;
	unsigned int timeout_irq;
	struct dfx_noc_err_probe_reg *perr_probe_reg;
	struct dfx_noc_reg_list *preg_list;
	struct dfx_noc_property *noc_property;
	struct dfx_noc_target_name_list *ptarget_list;
};

struct datapath_routid_addr {
	int init_flow;
	int targ_flow;
	int targ_subrange;
	u64 init_localaddr;
};

struct packet_configration {
	unsigned int statperiod;
	unsigned int statalarmmax;

	unsigned int packet_counters_0_src;
	unsigned int packet_counters_1_src;
	unsigned int packet_counters_2_src;
	unsigned int packet_counters_3_src;

	unsigned int packet_counters_0_alarmmode;
	unsigned int packet_counters_1_alarmmode;
	unsigned int packet_counters_2_alarmmode;
	unsigned int packet_counters_3_alarmmode;

	unsigned int packet_filterlut;
	unsigned int packet_f0_routeidbase;
	unsigned int packet_f0_routeidmask;
	unsigned int packet_f0_addrbase;
	unsigned int packet_f0_windowsize;
	unsigned int packet_f0_securitymask;
	unsigned int packet_f0_opcode;
	unsigned int packet_f0_status;
	unsigned int packet_f0_length;
	unsigned int packet_f0_urgency;
	unsigned int packet_f0_usermask;

	unsigned int packet_f1_routeidbase;
	unsigned int packet_f1_routeidmask;
	unsigned int packet_f1_addrbase;
	unsigned int packet_f1_windowsize;
	unsigned int packet_f1_securitymask;
	unsigned int packet_f1_opcode;
	unsigned int packet_f1_status;
	unsigned int packet_f1_length;
	unsigned int packet_f1_urgency;
	unsigned int packet_f1_usermask;
};
struct transcation_configration {
	unsigned int statperiod;
	unsigned int statalarmmax;

	unsigned int trans_f_mode;
	unsigned int trans_f_addrbase_low;
	unsigned int trans_f_addrwindowsize;
	unsigned int trans_f_opcode;
	unsigned int trans_f_usermask;
	unsigned int trans_f_securitymask;

	unsigned int trans_p_mode;
	unsigned int trans_p_thresholds_0_0;
	unsigned int trans_p_thresholds_0_1;
	unsigned int trans_p_thresholds_0_2;
	unsigned int trans_p_thresholds_0_3;

	unsigned int trans_m_counters_0_src;
	unsigned int trans_m_counters_1_src;
	unsigned int trans_m_counters_2_src;
	unsigned int trans_m_counters_3_src;

	unsigned int trans_m_counters_0_alarmmode;
	unsigned int trans_m_counters_1_alarmmode;
	unsigned int trans_m_counters_2_alarmmode;
	unsigned int trans_m_counters_3_alarmmode;
};

/* Clock information for Noc */
struct noc_clk {
	/* offset of clock status register in PERI_CRG */
	unsigned int offset;
	/* bit to indicate clock status */
	unsigned int mask_bit;
};

struct noc_node {
	char *name;
	void __iomem *base;
	unsigned int bus_id;
	unsigned int interrupt_num;
	unsigned int pwrack_bit;
	unsigned int eprobe_offset;
	bool eprobe_autoenable;
	int eprobe_hwirq;
	int hwirq_type;
	struct transcation_configration tran_cfg;
	struct packet_configration packet_cfg;
	/* Currently 2 clock sources for each noc node */
	struct noc_clk crg_clk[DFX_NOC_CLOCK_MAX];
	u32 noc_modid;
};

struct pctrl_bandwith {
	unsigned int offset;
	unsigned int val;
};

void __iomem *get_errprobe_base(const char *name);
struct noc_node *get_probe_node(const char *name);
void noc_get_bus_nod_info(void **node_array_pptr, unsigned int *idx_ptr);

u64 noc_find_addr_from_routeid(unsigned int idx, int initflow, int targetflow,
			       int targetsubrange);

int get_bus_id_by_base(const void __iomem *base);
int is_noc_node_available(struct noc_node *node);
struct platform_device *noc_get_pt_pdev(void);
unsigned int is_noc_init(void);
int get_coreid_fix_flag(void);
bool get_sctrl_scbakdata27_flag(void);
unsigned int get_sctrl_scbakdata27_offset(void);
/*
 * Function: noc_get_irq_status
 * Description: noc part, get irq status
 * input: void __iomem * pctrl_base: pctrl virtual base address
 * output: none
 * return: irq status
 */
unsigned long long noc_get_irq_status(const void __iomem *pctrl_base);
void enable_noc_transcation_probe(struct device *dev);
void disable_noc_transcation_probe(struct device *dev);
void enable_noc_packet_probe(struct device *dev);
void disable_noc_packet_probe(struct device *dev);
void noc_record_log_pstorememory(const void __iomem *base, int ptype);

extern void mntn_print_to_ramconsole(const char *fmt, ...);
extern void noc_err_probe_hanlder(void __iomem *base, struct noc_node *node);
extern int dfx_noc_get_data_from_dts(const struct platform_device *pdev);
extern void free_noc_bus_source(void);
extern struct dfx_noc_property noc_property_dt;
#endif
