/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: eagle_eye
 * Create: 2021-03-29
 */
#ifndef __DFX_EAGLE_EYE_H
#define __DFX_EAGLE_EYE_H

/* unit:nanosecond, keep with (watchdog_thresh*2)/5 in kernel/watchdog.c */
#define ALARM_DETECT_TIMEOUT	(4L*1000000000L)

#define EAGLE_EYE_RT_PRIORITY	97

struct alarm_detect_info {
	struct list_head  list;
	u32 reason; /* warning cause or type */
	u32 alarm_cpu;
	void *data;

	/* Constraints: This function is in the atomic context. Do not use functions that can sleep. */
	int (*detect)(struct alarm_detect_info *info);
	int (*process)(struct alarm_detect_info *info); /* running in non-atomic contexts */
	char *desc;
};

enum early_alarm_type {
	EARLY_ALARM_NORMAL     = 0x0,
	EARLY_ALARM_AWDT_NO_KICK   = 0x01,
	EARLY_ALARM_TASK_NO_SCHED     = 0x02,
	EARLY_ALARM_TEST      = 0x03,
	EARLY_ALARM_MAX
};

enum alarm_id {
	EARLY_ALARM_AWDT_NO_KICK_ID	= 0x0,
#ifdef CONFIG_SCHEDSTATS
	EARLY_ALARM_TASK_NO_SCHED_ID,
#endif
	EARLY_ALARM_TEST_ID,
	EARLY_ALARM_ID_MAX
};

#ifdef CONFIG_DFX_EAGLE_EYE
extern int eeye_register_alarm_detect_function(struct alarm_detect_info *info);
extern int  eeye_early_alarm(u32 reason, u32 cpu);
extern int  eeye_alarm_detect(void);
bool eeye_comm_init(void);
int  eeye_is_alarm_detecting(int cpu);
#else
static inline int eeye_register_alarm_detect_function(
	struct alarm_detect_info *info) { return 0; }
static inline int  eeye_early_alarm(u32 reason, u32 cpu) { return 0; }
static inline bool eeye_comm_init(void) { return false; }
static inline int  eeye_alarm_detect(void) { return 0; }
static inline int  eeye_is_alarm_detecting(int cpu) { return 0; }
#endif

#endif /* __DFX_EAGLE_EYE_H */

