/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: Sensor Hub Channel driver
 * Author: qindiwen
 * Create: 2012-05-29
 */

#ifndef __LINUX_INPUTHUB_ROUTE_H__
#define __LINUX_INPUTHUB_ROUTE_H__

#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#include <huawei_platform/log/hw_log.h>

#include <platform_include/smart/linux/base/ap/protocol.h>

#define HWLOG_TAG sensorhub
HWLOG_REGIST();

#define IOM3_ST_NORMAL 0
#define IOM3_ST_RECOVERY 1
#define IOM3_ST_REPEAT 2
/* max gap is 600ms */
#define MAX_SYSCOUNT_TIME_GAP 600000000LL
/* limit is 1 hour */
#define SYSCOUNT_DMD_LIMIT (3600 * 1000000000LL)
#define LENGTH_SIZE sizeof(unsigned int)
#define HEAD_SIZE (LENGTH_SIZE + TIMESTAMP_SIZE)
/* k3v5 report stairs level,record_count base on 1000 */
#define EXT_PEDO_VERSION_2 2000
#define ALS_RESET_COUNT 1
#define PS_RESET_COUNT 1
#define RESET_REFRESH_PERIOD (3600 * 8)
#define READ_CURRENT_INTERVAL 500
#define LOG_LEVEL_FATAL 0
/* Number of z-axis samples required by FingerSense at 1.6KHz ODR */
#define FINGERSENSE_DATA_NSAMPLES 128
#define MAX_SEND_LEN 32

struct link_package {
	int partial_order;
	char link_buffer[MAX_PKT_LENGTH_AP];
	int total_pkg_len;
	int offset;
};

struct mcu_notifier_work {
	struct work_struct worker;
	void *data;
};

struct mcu_notifier_node {
	int tag;
	int cmd;
	int (*notify) (const struct pkt_header *data);
	struct list_head entry;
};

struct mcu_notifier {
	struct list_head head;
	spinlock_t lock;
	struct workqueue_struct *mcu_notifier_wq;
};

#define offset(struct_t, member)                    offsetof(struct_t, member)
#define offset_of_end_mem(struct_t, member)         ((unsigned long)(&(((struct_t *)0)->member) + 1))
#define offset_interval(struct_t, member1, member2) (offset_of_end_mem(struct_t, member2) - offset_of_end_mem(struct_t, member1))

typedef enum {
	DMD_CASE_ALS_NEED_RESET_POWER,
	DMD_CASE_PS_NEED_RESET_POWER,
}sensor_reset_power;

enum port {
	ROUTE_SHB_PORT = 0x01,
	ROUTE_MOTION_PORT = 0x02,
	ROUTE_CA_PORT = 0x03,
	ROUTE_FHB_PORT = 0x04,
	ROUTE_FHB_UD_PORT = 0x05,
};

/* value lenght increase to 16 */
struct sensor_data {
	unsigned short type;
	unsigned short length;
	union {
		int value[16]; /* xyza... */
		struct {
			int serial;
			int data_type;
			int sensor_type;
			int info[13];
		};
	};
};

struct t_sensor_get_data {
	atomic_t reading;
	struct completion complete;
	struct sensor_data data;
};

struct sensor_status {
	/* record whether sensor is in activate status, already opened and setdelay */
	int status[TAG_SENSOR_END];
	/* record sensor delay time */
	int delay[TAG_SENSOR_END];
	/* record whether sensor was opened */
	int opened[TAG_SENSOR_END];
	int batch_cnt[TAG_SENSOR_END];
	char gyro_selftest_result[5];
	char mag_selftest_result[5];
	char accel_selftest_result[5];
	char gps_4774_i2c_selftest_result[5];
	char handpress_selftest_result[5];
	char selftest_result[TAG_SENSOR_END][5];
	int gyro_ois_status;
	struct t_sensor_get_data get_data[TAG_SENSOR_END];
};

typedef struct {
	int for_alignment;
	union {
		char effect_addr[sizeof(int)];
		int pkg_length;
	};
	int64_t timestamp;
} t_head;

struct type_record {
	const struct pkt_header *pkt_info;
	struct read_info *rd;
	struct completion resp_complete;
	struct mutex lock_mutex;
	spinlock_t lock_spin;
};

struct iom7_log_work {
	void *log_p;
	struct delayed_work log_work;
};

struct inputhub_buffer_pos {
	char *pos;
	unsigned int buffer_size;
};

/* Every route item can be used by one reader and one writer */
struct inputhub_route_table {
	unsigned short port;
	/* point to the head of buffer */
	struct inputhub_buffer_pos phead;
	/* point to the read position of buffer */
	struct inputhub_buffer_pos pread;
	/* point to the write position of buffer */
	struct inputhub_buffer_pos pwrite;
	/* to block read when no data in buffer */
	wait_queue_head_t read_wait;
	atomic_t data_ready;
	/* for read write buffer */
	spinlock_t buffer_spin_lock;
};

struct write_info {
	int tag;
	int cmd;
	const void *wr_buf; /* maybe NULL */
	int wr_len; /* maybe zero */
};

struct read_info {
	int errno;
	int data_length;
	char data[MAX_PKT_LENGTH];
};

typedef struct pkt_subcmd_para {
	struct pkt_header_resp hd;
	uint32_t subcmd;
	char data[SUBCMD_LEN];
} __packed pkt_subcmd_para_t;

typedef struct sensor_operation {
	int (*enable) (bool enable);
	int (*setdelay) (int ms);
} sensor_operation_t;

typedef struct ap_sensor_ops_record {
	sensor_operation_t ops;
	bool work_on_ap;
} t_ap_sensor_ops_record;

typedef bool(*t_match) (void *priv, const struct pkt_header *pkt);
struct mcu_event_waiter {
	struct completion complete;
	t_match match;
	void *out_data;
	int out_data_len;
	void *priv;
	struct list_head entry;
};

struct iom7_charging_work {
	uint32_t event;
	struct work_struct charging_work;
};

static inline bool match_tag_cmd(void *priv, const struct pkt_header *recv)
{
	struct pkt_header *send = (struct pkt_header *) priv;
	return (send->tag == recv->tag) && ((send->cmd + 1) == recv->cmd) && (send->tranid == recv->tranid);
}

#define wait_for_mcu_resp(trigger, match, wait_ms, out_data, out_data_len, priv) \
({\
	int __ret = 0;\
	struct mcu_event_waiter waiter;\
	init_wait_node_add_list(&waiter, match, out_data, out_data_len, (void *)priv);\
	reinit_completion(&waiter.complete);\
	trigger;\
	__ret = wait_for_completion_timeout(&waiter.complete, msecs_to_jiffies(wait_ms));\
	list_del_mcu_event_waiter(&waiter);\
	__ret;\
})

#define wait_for_mcu_resp_data_after_send(send_pkt, trigger, wait_ms, out_data, out_data_len) \
wait_for_mcu_resp(trigger, match_tag_cmd, wait_ms, out_data, out_data_len, send_pkt)

#define wait_for_mcu_resp_after_send(send_pkt, trigger, wait_ms) \
wait_for_mcu_resp_data_after_send(send_pkt, trigger, wait_ms, NULL, 0)

extern char *obj_tag_str[];
extern struct sensor_status sensor_status;

/* called by sensorhub or tp modules */
extern int inputhub_route_open(unsigned short port);
extern void inputhub_route_close(unsigned short port);
extern ssize_t inputhub_route_read(unsigned short port, char __user *buf, size_t count);
/* called by inputhub_mcu module or test module. */
extern void inputhub_route_init(void);
extern void inputhub_route_exit(void);
extern ssize_t inputhub_route_write(unsigned short port, char *buf, size_t count);

extern int inputhub_route_recv_mcu_data(const char *buf, unsigned int length);

extern int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);
extern int register_mcu_event_notifier(int tag, int cmd, int (*notify) (const struct pkt_header *head));
extern int unregister_mcu_event_notifier(int tag, int cmd, int (*notify) (const struct pkt_header *head));
extern int inputhub_sensor_enable(int tag, bool enable);
extern int inputhub_sensor_enable_stepcounter(bool enable, type_step_counter_t steptype);
extern int inputhub_sensor_setdelay(int tag, interval_param_t *interval_param);
extern int inputhub_sensor_enable_nolock(int tag, bool enable);
extern int inputhub_sensor_setdelay_nolock(int tag, interval_param_t *interval_param);
extern int inputhub_mcu_write_cmd(const void *buf, unsigned int length);
extern int register_ap_sensor_operations(int tag, sensor_operation_t *ops);
extern int unregister_ap_sensor_operations(int tag);
extern ssize_t inputhub_route_write_batch(unsigned short port, char *buf, size_t count, int64_t timestamp);

extern bool ap_sensor_enable(int tag, bool enable);
extern bool ap_sensor_setdelay(int tag, int ms);
extern int report_sensor_event(int tag, int value[], int length);
extern void init_wait_node_add_list(struct mcu_event_waiter *waiter, t_match match, void *out_data,
	int out_data_len, void *priv);
extern void list_del_mcu_event_waiter(struct mcu_event_waiter *self);
int send_app_config_cmd(int tag, void *app_config, bool use_lock);
int inputhub_get_airpress_data(void);
int inputhub_get_temperature_data(void);
spinlock_t *get_fsdata_lock(void);
int *get_step_ref_cnt(void);
bool *get_fingersense_data_ready(void);
bool *get_fingersense_data_intrans(void);
s16 *get_fingersense_data(void);
unsigned int get_sensor_read_number(enum obj_tag tag);
t_ap_sensor_ops_record *get_all_ap_sensor_operations(void);
#ifdef CONFIG_INPUTHUB_20
extern int ap_hall_report(int value);
#else /* CONFIG_INPUTHUB_20 = 0 */
static inline int ap_hall_report(int value)
{
	return 0;
}
#endif /* CONFIG_INPUTHUB_20 */
#endif /* __LINUX_INPUTHUB_ROUTE_H__ */
