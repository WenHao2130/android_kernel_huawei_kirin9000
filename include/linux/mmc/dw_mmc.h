 /*
 * Synopsys DesignWare Multimedia Card Interface driver
 *  (Based on NXP driver for lpc 31xx)
 *
 * Copyright (C) 2009 NXP Semiconductors
 * Copyright (C) 2009, 2010 Imagination Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef LINUX_MMC_DW_MMC_H
#define LINUX_MMC_DW_MMC_H

#include <linux/scatterlist.h>
#include <linux/mmc/core.h>
#include <linux/dmaengine.h>
#include <linux/reset.h>

#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/stringify.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/ratelimit.h>

/* kv3_5 platform dw emmc controller use as wifi usage with id = 0 */
#ifdef CONFIG_HUAWEI_EMMC_DSM
#undef CONFIG_HUAWEI_EMMC_DSM
#endif

#ifdef CONFIG_HUAWEI_EMMC_DSM
#include <linux/mmc/dsm_emmc.h>
#endif

/* kv3_5 platform dw emmc controller use as wifi usage with id = 0 */
#ifdef CONFIG_HUAWEI_EMMC_DSM
#undef CONFIG_HUAWEI_EMMC_DSM
#endif

#ifdef CONFIG_HUAWEI_EMMC_DSM
#include <linux/mmc/dsm_emmc.h>
#endif

#define MAX_MCI_SLOTS	2
#define TUNING_INIT_CONFIG_NUM 7
#define TUNING_INIT_TIMING_MODE 10

enum dw_mci_state {
	STATE_IDLE = 0,
	STATE_SENDING_CMD,
	STATE_SENDING_DATA,
	STATE_DATA_BUSY,
	STATE_SENDING_STOP,
	STATE_DATA_ERROR,
	STATE_SENDING_CMD11,
	STATE_WAITING_CMD11_DONE,
};

enum {
	EVENT_CMD_COMPLETE = 0,
	EVENT_XFER_COMPLETE,
	EVENT_DATA_COMPLETE,
	EVENT_DATA_ERROR,
};

struct mmc_data;

enum {
	TRANS_MODE_PIO = 0,
	TRANS_MODE_IDMAC,
	TRANS_MODE_EDMAC
};

struct dw_mci_dma_slave {
	struct dma_chan *ch;
	enum dma_transfer_direction direction;
};

/**
 * struct dw_mci - MMC controller state shared between all slots
 * @lock: Spinlock protecting the queue and associated data.
 * @irq_lock: Spinlock protecting the INTMASK setting.
 * @regs: Pointer to MMIO registers.
 * @fifo_reg: Pointer to MMIO registers for data FIFO
 * @sg: Scatterlist entry currently being processed by PIO code, if any.
 * @sg_miter: PIO mapping scatterlist iterator.
 * @cur_slot: The slot which is currently using the controller.
 * @mrq: The request currently being processed on @cur_slot,
 *	or NULL if the controller is idle.
 * @cmd: The command currently being sent to the card, or NULL.
 * @data: The data currently being transferred, or NULL if no data
 *	transfer is in progress.
 * @stop_abort: The command currently prepared for stoping transfer.
 * @prev_blksz: The former transfer blksz record.
 * @timing: Record of current ios timing.
 * @use_dma: Which DMA channel is in use for the current transfer, zero
 *  denotes PIO mode.
 * @using_dma: Whether DMA is in use for the current transfer.
 * @dma_64bit_address: Whether DMA supports 64-bit address mode or not.
 * @sg_dma: Bus address of DMA buffer.
 * @sg_cpu: Virtual address of DMA buffer.
 * @dma_ops: Pointer to platform-specific DMA callbacks.
 * @cmd_status: Snapshot of SR taken upon completion of the current
 * @ring_size: Buffer size for idma descriptors.
 *	command. Only valid when EVENT_CMD_COMPLETE is pending.
 * @dms: structure of slave-dma private data.
 * @phy_regs: physical address of controller's register map
 * @data_status: Snapshot of SR taken upon completion of the current
 *	data transfer. Only valid when EVENT_DATA_COMPLETE or
 *	EVENT_DATA_ERROR is pending.
 * @stop_cmdr: Value to be loaded into CMDR when the stop command is
 *	to be sent.
 * @dir_status: Direction of current transfer.
 * @tasklet: Tasklet running the request state machine.
 * @pending_events: Bitmask of events flagged by the interrupt handler
 *	to be processed by the tasklet.
 * @completed_events: Bitmask of events which the state machine has
 *	processed.
 * @state: Tasklet state.
 * @queue: List of slots waiting for access to the controller.
 * @bus_hz: The rate of @mck in Hz. This forms the basis for MMC bus
 *	rate and timeout calculations.
 * @current_speed: Configured rate of the controller.
 * @num_slots: Number of slots available.
 * @fifoth_val: The value of FIFOTH register.
 * @verid: Denote Version ID.
 * @dev: Device associated with the MMC controller.
 * @pdata: Platform data associated with the MMC controller.
 * @drv_data: Driver specific data for identified variant of the controller
 * @priv: Implementation defined private data.
 * @biu_clk: Pointer to bus interface unit clock instance.
 * @ciu_clk: Pointer to card interface unit clock instance.
 * @slot: Slots sharing this MMC controller.
 * @fifo_depth: depth of FIFO.
 * @data_shift: log2 of FIFO item size.
 * @part_buf_start: Start index in part_buf.
 * @part_buf_count: Bytes of partial data in part_buf.
 * @part_buf: Simple buffer for partial fifo reads/writes.
 * @push_data: Pointer to FIFO push function.
 * @pull_data: Pointer to FIFO pull function.
 * @vqmmc_enabled: Status of vqmmc, should be true or false.
 * @irq_flags: The flags to be passed to request_irq.
 * @irq: The irq value to be passed to request_irq.
 * @sdio_id0: Number of slot0 in the SDIO interrupt registers.
 * @cmd11_timer: Timer for SD3.0 voltage switch over scheme.
 * @dto_timer: Timer for broken data transfer over scheme.
 *
 * Locking
 * =======
 *
 * @lock is a softirq-safe spinlock protecting @queue as well as
 * @cur_slot, @mrq and @state. These must always be updated
 * at the same time while holding @lock.
 *
 * The @mrq field of struct dw_mci_slot is also protected by @lock,
 * and must always be written at the same time as the slot is added to
 * @queue.
 *
 * @irq_lock is an irq-safe spinlock protecting the INTMASK register
 * to allow the interrupt handler to modify it directly.  Held for only long
 * enough to read-modify-write INTMASK and no other locks are grabbed when
 * holding this one.
 *
 * @pending_events and @completed_events are accessed using atomic bit
 * operations, so they don't need any locking.
 *
 * None of the fields touched by the interrupt handler need any
 * locking. However, ordering is important: Before EVENT_DATA_ERROR or
 * EVENT_DATA_COMPLETE is set in @pending_events, all data-related
 * interrupts must be disabled and @data_status updated with a
 * snapshot of SR. Similarly, before EVENT_CMD_COMPLETE is set, the
 * CMDRDY interrupt must be disabled and @cmd_status updated with a
 * snapshot of SR, and before EVENT_XFER_COMPLETE can be set, the
 * bytes_xfered field of @data must be written. This is ensured by
 * using barriers.
 */
struct dw_mci {
	bool			wm_aligned;
	unsigned int sdio_rst;
        unsigned int bit_sdcard_o_sel18;
        unsigned int scperctrls;
        unsigned int odio_sd_mask_bit;
	int wifi_sdio_sdr104_160M;
	int wifi_sdio_sdr104_177M;
	spinlock_t		lock;
	spinlock_t		irq_lock;
	void __iomem		*regs;
	void __iomem		*fifo_reg;

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
	int	clk_change;
	int	retuning_flag;
	int	need_clk_change;
#endif
	int	downshift;
	int  use_samdly_range[2];
	int  enable_shift_range[2];
	bool use_samdly_flag;
	bool enable_shift_flag;

	int  is_cs_timing_config;

	struct scatterlist	*sg;
	struct sg_mapping_iter	sg_miter;

	struct dw_mci_slot	*cur_slot;
	struct mmc_request	*mrq;
	struct mmc_command	*cmd;
	struct mmc_data		*data;
	struct mmc_command	stop_abort;
	unsigned int		prev_blksz;
	unsigned char		timing;
	struct mmc_command	stop;
	bool			stop_snd;

	struct workqueue_struct	*card_workqueue;

	/* DMA interface members*/
	int			use_dma;
	int			using_dma;
	int			saved_tuning_phase;
	int			tuning_result_flag;
	int			dma_64bit_address;

	dma_addr_t		sg_dma;
	void			*sg_cpu;
	const struct dw_mci_dma_ops	*dma_ops;
	unsigned int		ring_size;
	unsigned int		desc_sz;
	u64			dma_mask;		/* custom DMA mask */
	/* For edmac */
	struct dw_mci_dma_slave *dms;
	/* Registers's physical base address */
	resource_size_t		phy_regs;
	u32			cmd_status;
	u32			data_status;
	u32			stop_cmdr;
	u32			dir_status;
	struct tasklet_struct	tasklet;
	struct work_struct	card_work;
#ifdef CONFIG_SD_TIMEOUT_RESET
	struct work_struct	work_volt_mmc;
#endif
	unsigned long		pending_events;
	unsigned long		completed_events;
	enum dw_mci_state	state;
	struct list_head	queue;

	u32			bus_hz;
	u32			current_speed;
	u32			num_slots;
	u32			fifoth_val;
	u16			verid;
	u16			data_offset;
	struct device		*dev;
	struct dw_mci_board	*pdata;
	const struct dw_mci_drv_data	*drv_data;
	void			*priv;
#ifdef CONFIG_SD_TIMEOUT_RESET
	int			volt_hold_clk_sd;
	int			volt_hold_clk_sdio;
	int			set_sd_data_tras_timeout;
	int			set_sdio_data_tras_timeout;
	struct clk		*volt_hold_sd_clk;
#endif
	struct clk		*volt_hold_sdio_clk;
	struct clk		*biu_clk;
	struct clk		*ciu_clk;
	struct clk 		*parent_clk;
	struct dw_mci_slot	*slot[MAX_MCI_SLOTS];

	/* FIFO push and pull */
	int			fifo_depth;
	unsigned int			data_shift;
	u8			part_buf_start;
	u8			part_buf_count;
	union {
		u16		part_buf16;
		u32		part_buf32;
		u64		part_buf;
	};
	void (*push_data)(struct dw_mci *host, void *buf, int cnt);
	void (*pull_data)(struct dw_mci *host, void *buf, int cnt);

	/* Workaround flags */
        u32                     quirks;

	bool			vqmmc_enabled;
	/* S/W reset timer */
	struct timer_list       timer;
#ifdef CONFIG_HUAWEI_EMMC_DSM
	struct timer_list       rw_to_timer;
	struct work_struct   dmd_work;
	u32 para;
	u32			dmd_cmd_status;
#endif

	/* pinctrl handles */
	struct pinctrl		*pinctrl;
	struct pinctrl_state	*pins_default;
	struct pinctrl_state	*pins_idle;

	struct regulator	*vmmc;	 /* Power regulator */
	struct regulator	*vqmmc;	 /* Signaling regulator (vccq) */
	struct regulator	*vmmcmosen;	 /* Power regulator mosen*/
	u8	sd_vmmcmosen_switch;

	unsigned long		irq_flags; /* IRQ flags */
	int			irq;

	unsigned int			flags;		/* Host attributes */
#define PINS_DETECT_ENABLE	(1 << 4)	/* NM card 4-pin detect control */
#define DWMMC_IN_TUNING		(1 << 5)	/* Host is doing tuning */
#define DWMMC_TUNING_DONE	(1 << 6)	/* Host initialization tuning done */
#define DWMMC_HPD_IRQ		(1 << 7)	/* pheonix enable hpd irq */
#define NANO_GPIO_POWER		(1 << 8)	/* cancel cw io and device power by gpio */
#define SLOW_IOCTL		(1 << 9)	/* host support slow ioctl */
#define SD_SUPPORT_1135		(1 << 10) /* for libra sd support 1135 */
#define TIMEOUT_ENABLE_PIO	(1 << 11)	/* for sdio data timeout and can reset restore */
#define OUTLDO_GPIO_POWER	(1 << 12)	/* TETON io and device power by gpio */

#define WIFI_ONLY_GPIO_NP		(1 << 13) /* WIFI only when plug out set gpio to sd idle state(GPIO TO NP) */
#define SIMCARD_CMD_PD			(1 << 14) /* levershift no sd insert set cmd gpio to Pull down */

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
	int						tuning_flag_all_pass;
#endif
	int						current_div;				/* record current div */
	int						tuning_current_sample;		/* record current sample */
	int						tuning_init_sample;			/* record the inital sample */
	int						tuning_move_sample;			/* record the move sample */
	int						tuning_move_count;			/* record the move count */
	unsigned int			tuning_sample_flag;			/* record the sample OK or NOT */
#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
	unsigned int			all_pass_flag;			/* tuning all pass flag */
	unsigned int			data_crc_flag;			/* tuning all pass then data crc flag */
#endif
	int						tuning_move_start;			/* tuning move start flag */
#define DWMMC_EMMC_ID		0
#define DWMMC_SD_ID			1
#define DWMMC_SDIO_ID		2
	int						hw_mmc_id;					/* Hardware mmc id */
#ifdef CONFIG_MMC_SIM_GPIO_EXCHANGE
	int						need_get_mmc_regulator;
#endif
	int						sd_reinit;
	int						sd_hw_timeout;

	u32           		    clock;		      /* Current clock (MHz) */
	u32          		    clock_to_restore; /* Saved clock for dynamic clock gating (MHz) */
	bool                    tuning_done;
	bool					tuning_needed;	  /* tuning move start flag */
	int			sdio_id0;

	struct timer_list       cmd11_timer;
	struct timer_list       cto_timer;
	struct timer_list       dto_timer;
	int                     is_reset_after_retry;
	unsigned int            lever_shift;
	unsigned int            timeout_cnt;
};

/* DMA ops for Internal/External DMAC interface */
struct dw_mci_dma_ops {
	/* DMA Ops */
	int (*init)(struct dw_mci *host);
	int (*start)(struct dw_mci *host, unsigned int sg_len);
	void (*complete)(void *host);
	void (*stop)(struct dw_mci *host);
	void (*reset)(struct dw_mci *host);
	void (*cleanup)(struct dw_mci *host);
	void (*exit)(struct dw_mci *host);
};

/* IP Quirks/flags. */
/* DTO fix for command transmission with IDMAC configured */
#define DW_MCI_QUIRK_IDMAC_DTO                  BIT(0)
/* delay needed between retries on some 2.11a implementations */
#define DW_MCI_QUIRK_RETRY_DELAY                BIT(1)
/* High Speed Capable - Supports HS cards (up to 50MHz) */
#define DW_MCI_QUIRK_HIGHSPEED                  BIT(2)
/* Unreliable card detection */
#define DW_MCI_QUIRK_BROKEN_CARD_DETECTION      BIT(3)
/* Timer for broken data transfer over scheme */
#define DW_MCI_QUIRK_BROKEN_DTO                 BIT(4)

struct dma_pdata;

/* Board platform data */
struct dw_mci_board {
	u32 num_slots;

	u32 quirks; /* Workaround / Quirk flags */
	unsigned int bus_hz; /* Clock speed at the cclk_in pad */

	u32 caps;	/* Capabilities */
	u32 caps2;	/* More capabilities */
	u32 caps3;      /* More capabilities */
	u32 pm_caps;	/* PM capabilities */
	/*
	 * Override fifo depth. If 0, autodetect it from the FIFOTH register,
	 * but note that this may not be reliable after a bootloader has used
	 * it.
	 */
	unsigned int fifo_depth;

	/* delay in mS before detecting cards after interrupt */
	u32 detect_delay_ms;

	struct reset_control *rstc;
	int (*init)(u32 slot_id, irq_handler_t , void *);
	int (*get_ro)(u32 slot_id);
	int (*get_cd)(struct dw_mci *host, u32 slot_id);
	int (*get_ocr)(u32 slot_id);
	int (*get_bus_wd)(u32 slot_id);
	/*
	 * Enable power to selected slot and set voltage to desired level.
	 * Voltage levels are specified using MMC_VDD_xxx defines defined
	 * in linux/mmc/host.h file.
	 */
	void (*setpower)(u32 slot_id, u32 volt);
	void (*exit)(u32 slot_id);
	void (*select_slot)(u32 slot_id);

	struct dw_mci_dma_ops *dma_ops;
	struct dma_pdata *data;
};

#endif /* LINUX_MMC_DW_MMC_H */
