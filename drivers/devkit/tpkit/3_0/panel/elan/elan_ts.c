#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <asm/unaligned.h>
#include <linux/firmware.h>
#include <linux/ctype.h>
#include <linux/atomic.h>
#include <linux/of_gpio.h>
#include "elan_ts.h"
#include "elan_mmi.h"
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

static int elan_ktf_chip_detect(struct ts_kit_platform_data *platform_data);
static int elan_ktf_init_chip(void);
static int elan_ktf_input_config(struct input_dev *input_dev);
static int elan_ktf_irq_top_half(struct ts_cmd_node *cmd);
static int elan_ktf_irq_bottom_half(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd);
static int elan_ktf_fw_update_boot(char *file_name);
static int elan_ktf_fw_update_sd(void);
static int elan_ktf_hw_reset(void);
static int elan_ktf_core_suspend(void);
static int elan_ktf_core_resume(void);
static void elan_chip_shutdown(void);
static int elan_ktf_parse_dts(struct device_node *device, struct ts_kit_device_data *chip_data);
static int elan_config_gpio(void);
static int elan_before_suspend(void);
static int elan_ktf_pen_config(struct input_dev *input_dev);
static int elan_read_tx_rx(void);
static int elan_ktf_get_rawdata(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd);
static int i2c_communicate_check(void);
static int elan_ktf_get_info(struct ts_chip_info_param *info);
static int elan_read_fw_info(void);
static int elan_chip_get_capacitance_test_type(struct ts_test_type_info *info);
static int rawdata_proc_elan_printf(struct seq_file *m, struct ts_rawdata_info *info,
		int range_size, int row_size);
static void elan_chip_touch_switch(void);
static int elan_ktf_ts_calibrate(void);
static atomic_t g_last_pen_inrange_status = ATOMIC_INIT(TS_PEN_OUT_RANGE); // remember the last pen inrange status
extern u8 cypress_ts_kit_color[TP_COLOR_SIZE];
struct elan_ktf_ts_data *g_elan_ts = NULL;

extern struct ts_kit_platform_data g_ts_kit_platform_data;
enum TP_MODE
{
	TP_NORMAL, // normal mode
	TP_RECOVER, // update fw fail mode
	TP_MODULETEST, // mmi test mode
	TP_FWUPDATA, // update fw mode
};

/* fw debug tool use */
#ifdef ELAN_IAP
#define ELAN_IOCTLID    0xD0
#define IOCTL_RESET  _IOR(ELAN_IOCTLID, 4, int)
#define IOCTL_IAP_MODE_LOCK  _IOR(ELAN_IOCTLID, 5, int)
#define IOCTL_FW_VER  _IOR(ELAN_IOCTLID, 7, int)
#define IOCTL_X_RESOLUTION  _IOR(ELAN_IOCTLID, 8, int)
#define IOCTL_Y_RESOLUTION  _IOR(ELAN_IOCTLID, 9, int)
#define IOCTL_FW_ID  _IOR(ELAN_IOCTLID, 10, int)
#define IOCTL_IAP_MODE_UNLOCK  _IOR(ELAN_IOCTLID, 12, int)
#define IOCTL_I2C_INT  _IOR(ELAN_IOCTLID, 13, int)
#endif

#define ELAN_WATER_MODE_INFO_MASK 0x18
#define ELAN_TRACE_MODE_INFO_MASK 0x20
#define ELAN_PALM_MODE_INFO_MASK 0x4
#define ELAN_FREQ_HOP_INFO_MASK 0x2
#define ELAN_OBL_MODE_INFO_MASK 0x1
#define ELAN_IC_DEBUG_INFO 0x3
#define TRACE_MODE_OFFSET 0x5
#define WATER_MODE_OFFSET 0x3
#define ELAN_DOZE_MAX_INPUT_SEPARATE_NUM 2
#define ELAN_CALIBRATE_STATUS_LBYTE 7
#define ELAN_CALIBRATE_STATUS_HBYTE 6
#define ELAN_NOT_CALIBRATE 0xFF

struct ts_device_ops ts_kit_elan_ops = {
	.chip_parse_config = elan_ktf_parse_dts,
	.chip_detect = elan_ktf_chip_detect,
	.chip_init = elan_ktf_init_chip,
	.chip_input_config = elan_ktf_input_config,
	.chip_input_pen_config = elan_ktf_pen_config,
	.chip_irq_top_half = elan_ktf_irq_top_half,
	.chip_irq_bottom_half = elan_ktf_irq_bottom_half,
	.chip_suspend = elan_ktf_core_suspend,
	.chip_resume = elan_ktf_core_resume,
	.chip_hw_reset = elan_ktf_hw_reset,
	.chip_fw_update_boot = elan_ktf_fw_update_boot,
	.chip_fw_update_sd = elan_ktf_fw_update_sd,
	.chip_before_suspend = elan_before_suspend,
	.chip_get_rawdata = elan_ktf_get_rawdata,
	.chip_get_info = elan_ktf_get_info,
	.chip_get_capacitance_test_type = elan_chip_get_capacitance_test_type,
	.chip_special_rawdata_proc_printf = rawdata_proc_elan_printf,
	.chip_shutdown = elan_chip_shutdown,
	.chip_touch_switch = elan_chip_touch_switch,
};

static void elan_scene_switch(unsigned int scene, unsigned int oper)
{
	int error;
	u8 enter_scene_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03,
		0x00, 0x04, 0x54, 0xcf, 0x00, (u8)scene };
	u8 exit_scene_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03,
		0x00, 0x04, 0x54, 0xcf, 0x00, 0x00 };

	if ((g_elan_ts->elan_chip_data->touch_switch_flag &
		TS_SWITCH_TYPE_SCENE) != TS_SWITCH_TYPE_SCENE) {
		TS_LOG_ERR("%s, scene switch does not supported by this chip\n",
			__func__);
		goto out;
	}

	switch (oper) {
	case TS_SWITCH_SCENE_ENTER:
		TS_LOG_INFO("%s, enter scene %d\n", __func__, scene);
		error = elan_i2c_write(enter_scene_cmd,
			sizeof(enter_scene_cmd));
		if (error) {
			TS_LOG_ERR("%s: Switch to scene %d mode Failed: error:%d\n",
				__func__, scene, error);
		}
		break;
	case TS_SWITCH_SCENE_EXIT:
		TS_LOG_INFO("%s, enter normal scene\n", __func__);
		error = elan_i2c_write(exit_scene_cmd,
			sizeof(exit_scene_cmd));
		if (error) {
			TS_LOG_ERR("%s: exit scene %d mode Failed: error:%d\n",
				__func__, scene, error);
		}
		break;
	default:
		TS_LOG_ERR("%s: oper unknown:%d, invalid\n",
			__func__, oper);
		break;
	}
out:
	return;
}

static void elan_chip_touch_switch(void)
{
	char in_data[MAX_STR_LEN] = {0};
	unsigned int stype = 0;
	unsigned int soper = 0;
	unsigned int time = 0;
	int error;
	unsigned int i;
	unsigned int cnt = 0;

	TS_LOG_INFO("%s enter\n", __func__);
	if (!g_elan_ts || !g_elan_ts->elan_chip_data ||
		!g_elan_ts->elan_chip_data->ts_platform_data){
		TS_LOG_ERR("%s, error chip data\n", __func__);
		goto out;
	}

	/* SWITCH_OPER,ENABLE_DISABLE,PARAM */
	memcpy(in_data, g_elan_ts->elan_chip_data->touch_switch_info,
		MAX_STR_LEN - 1);
	TS_LOG_INFO("%s, in_data:%s\n", __func__, in_data);
	for (i = 0; i < strlen(in_data) && (in_data[i] != '\n'); i++) {
		if (in_data[i] == ',') {
			cnt++;
		} else if (!isdigit(in_data[i])) {
			TS_LOG_ERR("%s: input format error!\n", __func__);
			goto out;
		}
	}
	if (cnt != ELAN_DOZE_MAX_INPUT_SEPARATE_NUM) {
		TS_LOG_ERR("%s: input format error[separation_cnt=%d]!\n",
			__func__, cnt);
		goto out;
	}

	error = sscanf(in_data, "%u,%u,%u", &stype, &soper, &time);
	if (error <= 0) {
		TS_LOG_ERR("%s: sscanf error\n", __func__);
		goto out;
	}
	TS_LOG_DEBUG("stype=%u,soper=%u,param=%u\n", stype, soper, time);

	if (atomic_read(&g_elan_ts->elan_chip_data->ts_platform_data->state) ==
		TS_SLEEP) {
		TS_LOG_ERR("%s, TP in sleep\n", __func__);
		goto out;
	}

	switch (stype) {
	case TS_SWITCH_TYPE_DOZE:
		break;
	case TS_SWITCH_TYPE_GAME:
		break;
	case TS_SWITCH_SCENE_3:
	case TS_SWITCH_SCENE_4:
	case TS_SWITCH_SCENE_5:
	case TS_SWITCH_SCENE_6:
	case TS_SWITCH_SCENE_7:
	case TS_SWITCH_SCENE_8:
	case TS_SWITCH_SCENE_9:
	case TS_SWITCH_SCENE_10:
	case TS_SWITCH_SCENE_11:
	case TS_SWITCH_SCENE_12:
	case TS_SWITCH_SCENE_13:
	case TS_SWITCH_SCENE_14:
	case TS_SWITCH_SCENE_15:
	case TS_SWITCH_SCENE_16:
	case TS_SWITCH_SCENE_17:
	case TS_SWITCH_SCENE_18:
	case TS_SWITCH_SCENE_19:
	case TS_SWITCH_SCENE_20:
		elan_scene_switch(stype, soper);
		break;
	default:
		TS_LOG_ERR("%s: stype unknown:%u, invalid\n",
			__func__, stype);
		break;
	}
out:
	return;
}

static int tp_module_test_init(struct ts_rawdata_info *info)
{
	int ret = 0;
	if (!info) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	ret = i2c_communicate_check();
	if (ret) {
		TS_LOG_ERR("[elan]:i2c Connet Test Fail!\n");
		strncat(info->result, "0F-", strlen("0F-"));
		return ret;
	} else {
		TS_LOG_INFO("[elan]:i2c Connet Test Pass!\n");
		strncat(info->result, "0P-", strlen("0P-"));
	}

	ret = alloc_data_buf();
	if (ret) {
		TS_LOG_ERR("[elan]:alloc data buf fail!\n");
		return ret;
	}

	if (g_elan_ts->rx_num <= 1 || g_elan_ts->tx_num <= 1) { // avoid rx-1 or tx -1
		TS_LOG_ERR("%s, tx, rx invalid\n", __func__);
		return -EINVAL;
	}
	info->buff[0] = g_elan_ts->rx_num;
	info->buff[1] = g_elan_ts->tx_num;
	info->used_size += 2;
	ret = disable_finger_report();
	if (ret) {
		TS_LOG_ERR("[elan]:disable_finger_report fail!\n");
		return ret;
	}
	ret = elan_get_set_opcmd();
	if (ret) {
		TS_LOG_ERR("[elan]:set op cmd Fail!\n");
		return ret;
	}
	ret = elan_read_ph();
	if (ret) {
		TS_LOG_ERR("[elan]:read ph value Fail!\n");
		return ret;
	}
	ret = elan_calibration_base();
	if (ret) {
		TS_LOG_ERR("[elan]:elan_calibration_base Fail!\n");
		return ret;
	}
	return NO_ERR;
}

static int elan_ktf_get_rawdata(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd)
{
	int ret = 0;
	int res = NO_ERR;
	bool txrx_open_test = true;
	char noise_test_result[RESULT_MAX_LEN] = {0};
	char txrx_open_test_result[RESULT_MAX_LEN] = {0};
	char txrx_short_test_result[RESULT_MAX_LEN] = {0};
	char adc_mean_low_boundary_test_result[RESULT_MAX_LEN] = {0};

	TS_LOG_INFO("--->>elan module test start<<---\n");
	if ((!info) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]:%s,info is null\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&g_elan_ts->tp_mode) != TP_NORMAL) {
		TS_LOG_ERR("[elan]:tp is not in normal mode\n");
		return -EINVAL;
	} else {
		atomic_set(&g_elan_ts->tp_mode, TP_MODULETEST);
	}
	__pm_stay_awake(g_elan_ts->wake_lock);
	memset(info->result, 0, sizeof(info->result));

	/* elan mmi step 1 i2c test and alloc data buf */
	ret = tp_module_test_init(info);
	if (ret) {
		TS_LOG_ERR("[elan]:tp_module_test_init Fail!\n");
		goto test_exit;
	}

	/* elan mmi step 2 get noise and test */
	ret = get_noise_test_data(info);
	if (ret < 0) {
		TS_LOG_ERR("[elan]:Noise read Fail!\n");
		strncat(noise_test_result, "3F-", (RESULT_MAX_LEN - 1));
		goto test_exit;
	} else if (ret == 1) { // 1, noise test fail
		TS_LOG_ERR("[elan]:Noise Test Fail!\n");
		strncat(noise_test_result, "3F-", (RESULT_MAX_LEN - 1));
	} else {
		TS_LOG_INFO("[elan]:Noise Test Pass!\n");
		strncat(noise_test_result, "3P-", (RESULT_MAX_LEN - 1));
	}

	/* elan mmi step 3 rx open test */
	ret = elan_rx_open_check(info);
	if (ret < 0) {
		TS_LOG_ERR("[elan]:elan_rx_open_check fail\n");
		goto test_exit;
	} else if (ret == 1) {    // 1, rx open test fail
		txrx_open_test = false;
	}

	/* elan mmi step 4 normal adc mean test */
	ret = elan_mean_value_check(adc_mean_low_boundary_test_result, info);
	if (ret) {
		TS_LOG_ERR("[elan]:elan_mean_value_check fail\n");
		goto test_exit;
	}

	/* elan mmi step 5 tx open test */
	ret = elan_tx_open_check();
	if (ret) {
		TS_LOG_ERR("[elan]:tx open test fail!\n");
		txrx_open_test = false;
	} else {
		TS_LOG_INFO("[elan]:tx open test pass!\n");
	}
	if (txrx_open_test) {
		strncat(txrx_open_test_result, "2P-", (RESULT_MAX_LEN - 1));
	} else {
		strncat(txrx_open_test_result, "2F-", (RESULT_MAX_LEN - 1));
	}

	/* elan mmi step 6 txrx short test */
	ret = elan_txrx_short_check(txrx_short_test_result, info);
	if (ret) {
		TS_LOG_ERR("[elan]:get tx rx short data fail\n");
		goto test_exit;
	}

test_exit:
	strncat(info->result, adc_mean_low_boundary_test_result, strlen(adc_mean_low_boundary_test_result));
	strncat(info->result, txrx_open_test_result, strlen(txrx_open_test_result));
	strncat(info->result, noise_test_result, strlen(noise_test_result));
	strncat(info->result, txrx_short_test_result, strlen(txrx_short_test_result));
	strncat(info->result, ";", sizeof(";"));
	strncat(info->result, g_elan_ts->elan_chip_data->chip_name, sizeof(g_elan_ts->elan_chip_data->chip_name));
	strncat(info->result, "_", sizeof("_"));
	strncat(info->result, g_elan_ts->project_id, sizeof(g_elan_ts->project_id));
	free_data_buf();
	__pm_relax(g_elan_ts->wake_lock);
	atomic_set(&g_elan_ts->tp_mode, TP_NORMAL);
	res = elan_ktf_hw_reset();
	if (res != NO_ERR) {
		TS_LOG_ERR("%s hw reset fail\n", __func__);
	}
	TS_LOG_ERR("[elan]%s,end!,%s\n", __func__, info->result);
	return ret;
}

static int rawdata_proc_elan_printf(struct seq_file *m, struct ts_rawdata_info *info,
		int range_size, int row_size)
{
	int index = 0;
	int index1 = 0;

	if ((range_size == 0) || (row_size == 0) || (!info)) {
		TS_LOG_ERR("%s  range_size OR row_size is 0 OR info is NULL\n", __func__);
		return -EINVAL;
	}
	/* buf[0] is rx_num, buf[1] is tx_num, so offset 2, need print 3 items */
	for (index = 0; row_size * index + 2 < (2 + row_size * range_size * 3); index++) {
		if (index == 0) {
			seq_printf(m, "noisedata begin\n");    /* print the title */
		}
		for (index1 = 0; index1 < row_size; index1++) {
			seq_printf(m, "%d,", info->buff[2 + row_size * index + index1]);    /* print oneline */
		}
		/* index1 = 0; */
		seq_printf(m, "\n ");

		if (index == (range_size - 1)) {
			seq_printf(m, "noisedata end\n");
			seq_printf(m, "normal adc begin\n");
		} else if (index == (range_size * 2 - 1)) {
			seq_printf(m, "normal adc end\n");
			seq_printf(m, "normal adc2 begin\n");
		}
	}
	seq_printf(m, "normal adc2 end\n");
	/* buf[0] is rx_num, buf[1] is tx_num, so offset 2, need print 3 items */
	if (info->used_size > (2 + row_size * range_size * 3)) {
		seq_printf(m, "rx short begin\n");
		for (index1 = 0; index1 < (row_size + range_size) * 2; index1++) {
			seq_printf(m, "%d,", info->buff[2 + row_size * range_size * 3 + index1]);
			if (index1 == (row_size - 1)) {
				seq_printf(m, "\n ");
				seq_printf(m, "rx short end\n");
				seq_printf(m, "tx short begin\n");
			} else if (index1 == (row_size + range_size - 1)) {
				seq_printf(m, "\n ");
				seq_printf(m, "tx short end\n");
				seq_printf(m, "rx short2 begin\n");
			} else if (index1 == (row_size * 2 + range_size - 1)) {
				seq_printf(m, "\n ");
				seq_printf(m, "rx short2 end\n");
				seq_printf(m, "tx short2 begin\n");
			}
		}
		seq_printf(m, "\n ");
		seq_printf(m, "tx short2 end\n");
	}
	return NO_ERR;
}

static int elan_chip_get_capacitance_test_type(struct ts_test_type_info *info)
{
	int error = NO_ERR;
	struct elan_ktf_ts_data *data = g_elan_ts;

	if (!info || !data || !data->elan_chip_data) {
		TS_LOG_ERR("[elan],%s:info=%ld\n", __func__, PTR_ERR(info));
		error = -ENOMEM;
		return error;
	}
	switch (info->op_action) {
		case TS_ACTION_READ:
			memcpy(info->tp_test_type, data->elan_chip_data->tp_test_type,
					TS_CAP_TEST_TYPE_LEN - 1);
			TS_LOG_INFO("read_chip_get_test_type=%s, \n",
					info->tp_test_type);
			break;
		case TS_ACTION_WRITE:
			break;
		default:
			TS_LOG_ERR("invalid status: %s", info->tp_test_type);
			error = -EINVAL;
			break;
	}
	return error;
}

#ifdef ELAN_IAP
static int elan_iap_open(struct inode *inode, struct file *filp)
{
	TS_LOG_INFO("[elan]into elan_iap_open\n");
	if (!g_elan_ts) {
		TS_LOG_INFO("g_elan_ts is NULL\n");
	}
	return NO_ERR;
}

static ssize_t elan_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	int ret = 0;
	char *tmp = NULL;
	if (!buff) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("[elan]into elan_iap_write\n");
	if (count > MAX_WRITE_LEN) {
		return -ENOMEM;
	}
	tmp = kmalloc(count, GFP_KERNEL);
	if (tmp == NULL) {
		TS_LOG_ERR("[elan]%s:fail to kmalloc\n", __func__);
		return -ENOMEM;
	}
	if (copy_from_user(tmp, buff, count)) {
		TS_LOG_ERR("[elan]%s:fail to copy_from_user\n", __func__);
		kfree(tmp);
		tmp = NULL;
		return -EFAULT;
	}

	ret = elan_i2c_write(tmp, ELAN_SEND_DATA_LEN);
	if (ret) {
		TS_LOG_ERR("[elan]%s:fail to i2c_write,ret=%d\n", __func__, ret);
		kfree(tmp);
		tmp = NULL;
		return -EINVAL;
	}
	kfree(tmp);
	tmp = NULL;
	return count;
}

static ssize_t elan_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	char *tmp = NULL;
	int ret = 0;
	if (!buff) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("[elan]into elan_iap_read\n");
	if (count > MAX_WRITE_LEN) {
		count = MAX_WRITE_LEN;
	}
	tmp = kmalloc(count, GFP_KERNEL);
	if (tmp == NULL) {
		TS_LOG_ERR("[elan]%s:fail to kmalloc\n", __func__);
		return -ENOMEM;
	}
	ret = elan_i2c_read(NULL, 0, tmp, count);
	if (ret >= 0) {
		ret = copy_to_user(buff, tmp, count);
		if (ret) {
			TS_LOG_ERR("[elan]%s:copy_to_user fail\n", __func__);
			kfree(tmp);
			tmp = NULL;
			return -ENOMEM;
		}
	}
	kfree(tmp);
	tmp = NULL;
	return (ret < 0) ? ret : count;
}

static long elan_iap_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int __user *ip = (int __user *)arg;
	int ret = NO_ERR;
	TS_LOG_INFO("[elan]:%s\n", __func__);
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_client)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EFAULT;
	}
	switch (cmd) {
		case IOCTL_RESET:
			ret = elan_ktf_hw_reset();
			if (ret != NO_ERR) {
				TS_LOG_ERR("%s hw reset fail\n", __func__);
			}
			break;
		case IOCTL_IAP_MODE_LOCK:
			atomic_set(&g_elan_ts->tp_mode, TP_FWUPDATA);
			TS_LOG_INFO("[elan]:disable irq=%d\n", g_elan_ts->irq_id);
			disable_irq(g_elan_ts->irq_id);
			break;
		case IOCTL_FW_VER:
			return g_elan_ts->fw_ver;
			break;
		case IOCTL_X_RESOLUTION:
			return g_elan_ts->finger_x_resolution;
			break;
		case IOCTL_Y_RESOLUTION:
			return g_elan_ts->finger_y_resolution;
			break;
		case IOCTL_FW_ID:
			return g_elan_ts->fw_id;
			break;
		case IOCTL_IAP_MODE_UNLOCK:
			atomic_set(&g_elan_ts->tp_mode, TP_NORMAL);
			TS_LOG_INFO("[elan]:enable irq\n");
			enable_irq(g_elan_ts->irq_id);
			break;
		case IOCTL_I2C_INT:
			put_user(gpio_get_value(g_elan_ts->int_gpio), ip);
			break;
		default:
			TS_LOG_INFO("[elan]:unknown ioctl cmd!\n");
			break;
	}
	return NO_ERR;
}

static const struct file_operations elan_touch_fops = {
	.owner = THIS_MODULE,
	.open = elan_iap_open,
	.write = elan_iap_write,
	.read = elan_iap_read,
	.compat_ioctl = elan_iap_ioctl,
	.unlocked_ioctl = elan_iap_ioctl,
};
#endif

static void elants_reset_pin_low(void)
{
	gpio_set_value(g_elan_ts->reset_gpio, 0);
	return;
}

static void elants_reset_pin_high(void)
{
	gpio_set_value(g_elan_ts->reset_gpio, 1);
	return;
}

static int elan_ktf_get_info(struct ts_chip_info_param *info)
{
	struct elan_ktf_ts_data *ts = g_elan_ts;
	if (!info || !ts || !ts->elan_chip_data || !ts->elan_chip_client) {
		TS_LOG_ERR("[elan]arg is NULL,%s\n", __func__);
		return -EINVAL;
	}
	if (ts->elan_chip_client->hide_plain_id)
		snprintf(info->ic_vendor, sizeof(ts->project_id),
			ts->project_id);
	else
		snprintf(info->ic_vendor, sizeof(info->chip_name),
			ts->elan_chip_data->chip_name);
	snprintf(info->fw_vendor, sizeof(info->fw_vendor), "0x%04x", ts->fw_ver);
	snprintf(info->mod_vendor, sizeof(info->mod_vendor), ts->project_id);
	return NO_ERR;
}

static int elan_config_gpio(void)
{
	int err = NO_ERR;
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]arg is NULL,%s\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("[elan]:%s enter\n", __func__);
	if (gpio_is_valid(g_elan_ts->int_gpio)) {
		err = gpio_direction_input(g_elan_ts->int_gpio);
		if (err) {
			TS_LOG_ERR("[elan]:unable to set direction for gpio [%d]\n", g_elan_ts->int_gpio);
			return -EINVAL;
		}
	} else {
		TS_LOG_INFO("[elan]:int gpio not provided\n");
	}

	if (gpio_is_valid(g_elan_ts->reset_gpio)) {
		err = gpio_direction_output(g_elan_ts->reset_gpio, 0);
		if (err) {
			TS_LOG_ERR("[elan]:unable to set direction for gpio [%d]\n", g_elan_ts->reset_gpio);
			return -EINVAL;
		}
	} else {
		TS_LOG_ERR("[elan]:int gpio not provided\n");
	}
	return NO_ERR;
}

static inline void elan_ktf_finger_parse_xy(uint8_t *data)
{
	g_elan_ts->wx = data[FINGERX_WIDTH_BYTE];
	g_elan_ts->wy = data[FINGERY_WIDTH_BYTE];
	g_elan_ts->ewx = data[FINGERX_EDGE_WIDTH_BYTE];
	g_elan_ts->ewy = data[FINGERY_EDGE_WIDTH_BYTE];
	g_elan_ts->xer = data[FINGERX_XEDGE_RATIO_BYTE];
	g_elan_ts->yer = data[FINGERY_YEDGE_RATIO_BYTE];
	g_elan_ts->finger_x = ((data[FINGERX_POINT_HBYTE] << 8) |
		data[FINGERX_POINT_LBYTE]);
	g_elan_ts->finger_y = ((data[FINGERY_POINT_HBYTE] << 8) |
		data[FINGERY_POINT_LBYTE]);

	return;
}

static inline void elan_ktf_pen_parse_xy(uint8_t *data)
{
	g_elan_ts->pen_x = ((data[PENX_POINT_HBYTE] << 8) |
		data[PENX_POINT_LBYTE]);
	g_elan_ts->pen_y = ((data[PENY_POINT_HBYTE] << 8) |
		data[PENY_POINT_LBYTE]);
	g_elan_ts->pen_pressure = ((data[PEN_PRESS_HBYTE] << 8) |
		data[PEN_PRESS_LBYTE]);

	return;
}

int elan_i2c_read(u8 *reg_addr, u16 reg_len, u8 *buf, u16 len)
{
	int rc = 0;
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_data) || (!g_elan_ts->elan_chip_client) ||
			(!g_elan_ts->elan_chip_client->bops)) {
		TS_LOG_ERR("[elan]arg is NULL,%s\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&g_elan_ts->ktf_mutex);
	if (!reg_addr) {
		g_elan_ts->elan_chip_data->is_i2c_one_byte = true;
	} else {
		g_elan_ts->elan_chip_data->is_i2c_one_byte = false;
	}
	rc = g_elan_ts->elan_chip_client->bops->bus_read(reg_addr, reg_len, buf, len);
	if (rc) {
		TS_LOG_ERR("[elan]:%s,fail read    rc=%d\n", __func__, rc);
	}
	mutex_unlock(&g_elan_ts->ktf_mutex);
	return rc;
}

int elan_i2c_write(u8 *buf, u16 length)
{
	int rc = 0;
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_client) || (!g_elan_ts->elan_chip_client->bops)) {
		TS_LOG_ERR("[elan]arg is NULL,%s\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&g_elan_ts->ktf_mutex);
	rc = g_elan_ts->elan_chip_client->bops->bus_write(buf, length);
	if (rc) {
		TS_LOG_ERR("[elan]:%s,fail write rc=%d\n", __func__, rc);
	}
	mutex_unlock(&g_elan_ts->ktf_mutex);
	return rc;
}

static int i2c_communicate_check(void)
{
	int rc = NO_ERR;
	int i = 0;
	// check i2c communicate regs
	u8 cmd_id[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0xf0, 0x00, 0x01 };
	u8 buf[ELAN_RECV_DATA_LEN] = {0};
	for (i = 0; i < I2C_RW_TRIES; i++) {
		rc = elan_i2c_read(cmd_id, sizeof(cmd_id), buf, sizeof(buf));
		if (rc) {
			TS_LOG_ERR("[elan]:%s, Failed to read tp info,i = %d, rc = %d\n", __func__, i, rc);
			msleep(50); // IC need
		} else {
			TS_LOG_INFO("[elan]:%s, i2c communicate check success,buf[0]=%x\n", \
					__func__, buf[0]); // hid head recv data len
			return NO_ERR;
		}
	}

	return rc;
}

static int elan_ktf_ts_recv_data(u8 *pbuf)
{
	int ret = 0;
	int fingernum = 0;
	int recv_count = 0;
	int recv_count_max = 0;
	u8 *buf = pbuf;
	u8 data_buf[REPORT_DATA_LEN] = {0};
	if ((!buf) || (!g_elan_ts)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_read(NULL, 0, buf, REPORT_DATA_LEN);
	if (ret) {
		TS_LOG_ERR("[elan]elan_i2c_read Fail!ret=%d\n", ret);
		return ret;
	}
	if (buf[REPORT_ID_BYTE] == FINGER_REPORT_ID) {
		fingernum = buf[CUR_FINGER_NUM_BYTE];
		if (fingernum > MAX_FINGER_SIZE) {
			TS_LOG_ERR("[elan]:invalid finger num\n");
			return -EINVAL;
		}
		g_elan_ts->cur_finger_num = fingernum;
		recv_count_max = (fingernum / 2) + ((fingernum % 2) != 0); // 2, one recv_package include 2 fingers touch mesg
		for (recv_count = 1; recv_count < recv_count_max; recv_count++) {
			ret = elan_i2c_read(NULL, 0, data_buf, REPORT_DATA_LEN);
			if (ret) {
				TS_LOG_ERR("[elan]elan_i2c_read Fail!ret=%d\n", ret);
				return ret;
			}
			memcpy(buf + (REPORT_DATA_LEN - 1)*recv_count - POINT_HEAD_LEN * (recv_count - 1),
					data_buf + POINT_HEAD_LEN, REPORT_DATA_LEN - POINT_HEAD_LEN); // not Out of bounds
		}
	}
	return NO_ERR;
}

static void elan_mt_process_touch(uint16_t *x, uint16_t *y, int x_resolution, int y_resolution)
{
	uint16_t temp_value = 0;
	uint16_t temp_x = *x;
	uint16_t temp_y = *y;
	int lcm_max_x = 0;
	int lcm_max_y = 0;

	if (g_elan_ts->mt_flags & ELAN_MT_FLAG_FLIP) {
		temp_value = temp_x;
		temp_x = temp_y;
		temp_y = temp_value;

	}
	lcm_max_x = g_elan_ts->elan_chip_data->x_max - 1;
	lcm_max_y = g_elan_ts->elan_chip_data->y_max - 1;
	if ((x_resolution > 0) && (y_resolution > 0)) {
		temp_x = (uint16_t)((int)temp_x * lcm_max_x / x_resolution);
		temp_y = (uint16_t)((int)temp_y * lcm_max_y / y_resolution);
	}

	if (g_elan_ts->mt_flags & ELAN_MT_FLAG_INV_X) {
		temp_x = lcm_max_x - temp_x;
	}
	if (g_elan_ts->mt_flags & ELAN_MT_FLAG_INV_Y) {
		temp_y = lcm_max_y - temp_y;
	}

	*x = temp_x;
	*y = temp_y;
	return;
}

static void parse_fingers_point(struct ts_fingers *pointinfo, u8 *pbuf)
{
	int i = 0;
	int fid = 0;
	int idx = 3; // point  start byte

	if ((!pointinfo) || (!pbuf) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return ;
	}

	memset(pointinfo, 0, sizeof(struct ts_fingers));
	for (i = 0; i < g_elan_ts->cur_finger_num; i++) {
		if ((pbuf[idx] & 0x3) != 0x0) {    // bit0 tip bit1 range
			fid = (pbuf[idx] >> 2) & 0x3f;    // fingerid bit 2-7
			elan_ktf_finger_parse_xy(pbuf + idx);
			elan_mt_process_touch(&(g_elan_ts->finger_x),
				&(g_elan_ts->finger_y),
				g_elan_ts->finger_x_resolution,
				g_elan_ts->finger_y_resolution);
			pointinfo->fingers[fid].status = TS_FINGER_PRESS;
			pointinfo->fingers[fid].x = (int)(g_elan_ts->finger_x);
			pointinfo->fingers[fid].y = (int)(g_elan_ts->finger_y);
			pointinfo->fingers[fid].major = FINGER_MAJOR;
			pointinfo->fingers[fid].minor = FINGER_MINOR;
			pointinfo->fingers[fid].pressure = FINGER_PRESSURE;
			pointinfo->fingers[fid].wx = (int)(g_elan_ts->wx);
			pointinfo->fingers[fid].wy = (int)(g_elan_ts->wy);
			pointinfo->fingers[fid].ewx = (int)(g_elan_ts->ewx);
			pointinfo->fingers[fid].ewy = (int)(g_elan_ts->ewy);
			pointinfo->fingers[fid].xer = (int)(g_elan_ts->xer);
			pointinfo->fingers[fid].yer = (int)(g_elan_ts->yer);
		}
		idx += VALUE_OFFSET;
	}
	pointinfo->cur_finger_number = g_elan_ts->cur_finger_num;
}

static void parse_pen_point(struct ts_pens *pointinfo, u8 *pbuf, struct ts_cmd_node *out_cmd)
{
	unsigned int pen_down = 0;

	if ((!pointinfo) || (!pbuf) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return ;
	}
	pen_down = pbuf[3] & 0x03;   // pbuf[3] bit 0,1 tip and inrange
	if (pen_down) {
		elan_ktf_pen_parse_xy(pbuf);
		elan_mt_process_touch(&(g_elan_ts->pen_x), &(g_elan_ts->pen_y),
			g_elan_ts->pen_x_resolution,
			g_elan_ts->pen_y_resolution);
		pointinfo->tool.tip_status = (int)(pen_down >> 1);
		pointinfo->tool.x = (int)(g_elan_ts->pen_x);
		pointinfo->tool.y = (int)(g_elan_ts->pen_y);
		pointinfo->tool.pressure = (int)(g_elan_ts->pen_pressure);
		pointinfo->tool.pen_inrange_status = (int)(pen_down & 0x1);
		TS_LOG_DEBUG("[elan]:report pen pressure = %d, pen_inrange_status = %d\n", \
			g_elan_ts->pen_pressure, (int)(pen_down & 0x1));
		if (!g_elan_ts->pen_detected) {
			TS_LOG_INFO("[elan]:pen is detected!\n");
			g_elan_ts->pen_detected = true;
		}
	} else {
		pointinfo->tool.tip_status = 0;
		pointinfo->tool.pen_inrange_status = 0;
		TS_LOG_INFO("[elan]:pen is exit!\n");
		g_elan_ts->pen_detected = false;
	}
	return;
}

static int check_fw_status(void)
{
	int ret = 0;
	u8 checkstatus[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x18 };
	u8 check_rek_count[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00,
		0x03, 0x00, 0x04, 0x53, 0xd0, 0x00, 0x01 };
	u8 buff[ELAN_RECV_DATA_LEN] = {0};
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s,elan ts is null\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_read(checkstatus, sizeof(checkstatus), buff, sizeof(buff));
	if (ret) {
		TS_LOG_ERR("[elan]:i2c read tp mode fail!ret=%d\n", ret);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:Tp mode check:%x,%x,%x,%x,%x\n", \
				buff[0], buff[1], buff[2], buff[3], buff[4]); // 0-3 hid head 4 th fw status buff
	}

	if (buff[TP_NORMAL_DATA_BYTE] == TP_NORMAL_DATA) {
		ret = elan_i2c_read(check_rek_count, sizeof(check_rek_count),
			buff, sizeof(buff));
		if (ret) {
			TS_LOG_ERR("[elan]:i2c read tp mode fail!ret=%d\n",
				ret);
			return -EINVAL;
		}
		/* 0-5th hid head info, 6th & 7th calibration status */
		TS_LOG_INFO("[elan]:Tp rek_count check:%x,%x,%x,%x,%x,%x,%x,%x\n",
			buff[0], buff[1], buff[2], buff[3], buff[4],
			buff[5], buff[ELAN_CALIBRATE_STATUS_HBYTE],
			buff[ELAN_CALIBRATE_STATUS_LBYTE]);
		/*
		 * calibrate status description:
		 *    0xFFFF: TP IC do not calibrate after last update firmware.
		 *    0x0: TP IC need not calibrate.
		 *    0x1: TP IC has been calibrated.
		 */
		if (buff[ELAN_CALIBRATE_STATUS_LBYTE] == ELAN_NOT_CALIBRATE) {
			ret = elan_ktf_ts_calibrate();
			if (ret) {
				TS_LOG_ERR("[elan]:%s,calibrate fail\n",
					__func__);
				return -EINVAL;
			}
			TS_LOG_INFO("%s,elan_ktf_ts_calibrate success\n",
				__func__);
		}
		atomic_set(&g_elan_ts->tp_mode, TP_NORMAL);
		return 1;
	} else if (buff[TP_RECOVER_DATA_BYTE] == TP_RECOVER_DATA) {
		atomic_set(&g_elan_ts->tp_mode, TP_RECOVER);
		return 0;
	} else {
		TS_LOG_ERR("[elan]:unknown mode!\n");
		return -EINVAL;
	}
	return NO_ERR;
}

/* enter updata fw mode */
static int enter_iap_mode(void)
{
	int ret = 0;
	u8 flash_key[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xc0, 0xe1, 0x5a };
	u8 isp_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x00, 0x12, 0x34 };
	u8 check_addr[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x01, 0x10 };
	u8 buff[ELAN_RECV_DATA_LEN] = {0};
	TS_LOG_INFO("[elan]:%s!!\n", __func__);
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s g_elan_ts is null\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_write(flash_key, sizeof(flash_key));
	if (ret) {
		TS_LOG_ERR("[elan]:%s,send flash key fail!ret=%d\n", __func__, ret);
		return -EINVAL;
	}
	if (atomic_read(&g_elan_ts->tp_mode) == TP_NORMAL) {
		ret = elan_i2c_write(isp_cmd, sizeof(isp_cmd));
		if (ret) {
			TS_LOG_ERR("[elan]:%s,send iap cmd fail!ret=%d\n", __func__, ret);
			return -EINVAL;
		}
	}

	ret = elan_i2c_read(check_addr, sizeof(check_addr), buff, sizeof(buff));
	if (ret) {
		TS_LOG_ERR("[elan]:%s,i2c read data fail!ret=%d\n", __func__, ret);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:addr data:%x,%x,%x,%x,%x,%x,%x\n", \
				buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6]); // 0-3th hid head 4-6th fw status buff
	}
	return NO_ERR;
}

static int get_ack_data(void)
{
	int res = 0;
	u8 buff[ELAN_RECV_DATA_LEN] = {0};
	u8 ack_buf[2] = {IC_ACK, IC_ACK};
	res = elan_i2c_read(NULL, 0, buff, sizeof(buff));
	if (res) {
		TS_LOG_ERR("[elan]:read ack data fail!res=%d\n", res);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:ack data:%x,%x,%x,%x,%x,%x\n", \
				buff[0], buff[1], buff[2], buff[3], buff[4], buff[5]); // 0-3 hid head data 4&5th ic ack
	}
	res = memcmp(buff + 4, ack_buf, sizeof(ack_buf));  // 4-5th ack (0xaa)
	if (res) {
		TS_LOG_ERR("[elan]:ack not right!\n");
		return -EINVAL;
	}
	return NO_ERR;
}

static int SendFinishCmd(void)
{
	int len = 0;
	u8 finish_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x1A };
	len = elan_i2c_write(finish_cmd, sizeof(finish_cmd));
	if (len) {
		TS_LOG_ERR("[elan]:send updata fw finish cmd fail!len=%d\n", len);
	}
	return len;
}

static int elan_ktf_ts_calibrate(void)
{
	int ret = NO_ERR;
	u8 flash_key[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0xc0, 0xe1, 0x5a };
	u8 cal_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x29, 0x00, 0x01 };
	ret = elan_i2c_write(flash_key, sizeof(flash_key));
	if (ret) {
		TS_LOG_ERR("[elan]:send flash key cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}
	ret = elan_i2c_write(cal_cmd, sizeof(cal_cmd));
	if (ret) {
		TS_LOG_ERR("[elan]:send calibrate cmd fail!ret=%d\n", ret);
		return -EINVAL;
	}
	return NO_ERR;
}

static int hid_write_page(int pagenum, const u8 *fwdata)
{
	int write_times = 0;
	int ipage = 0;
	unsigned int offset;
	int byte_count = 0;
	int curIndex = 0;
	int res = 0;
	int lastdatalen = 0;
	const u8 *szBuff = NULL;
	u8 write_buf[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x21, 0x00, 0x00, 0x28 };
	u8 cmd_iap_write[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x22 };
	int lastpages = (int)(pagenum / WRITE_PAGES) * WRITE_PAGES + 1;
	if (!fwdata) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	TS_LOG_DEBUG("[elan]:lastpages=%d\n", lastpages);
	for (ipage = 1; ipage <= pagenum; ipage += WRITE_PAGES) {
		offset = 0;
		if (ipage == lastpages) {
			write_times = (int)((pagenum - lastpages + 1) * FW_PAGE_SIZE / WRITE_DATA_VALID_LEN) +
				((pagenum - lastpages + 1) * FW_PAGE_SIZE % WRITE_DATA_VALID_LEN != 0);
			lastdatalen = (pagenum - lastpages + 1) * FW_PAGE_SIZE % WRITE_DATA_VALID_LEN;
		} else {
			write_times = (int)(WRITE_PAGES * FW_PAGE_SIZE / WRITE_DATA_VALID_LEN) +
				(WRITE_PAGES * FW_PAGE_SIZE % WRITE_DATA_VALID_LEN != 0);
			lastdatalen = WRITE_PAGES * FW_PAGE_SIZE % WRITE_DATA_VALID_LEN;
		}
		TS_LOG_DEBUG("[elan]:update fw write_times=%d,lastdatalen=%d\n", write_times, lastdatalen);

		for (byte_count = 1; byte_count <= write_times; byte_count++) {
			szBuff = fwdata + curIndex;
			write_buf[OFFSET_LBYTE] = offset & 0x00ff;
			write_buf[OFFSET_HBYTE] = offset >> 8;

			if (byte_count != write_times) {
				write_buf[WRITE_DATA_VALID_LEN_BYTE] = WRITE_DATA_VALID_LEN;
				offset += WRITE_DATA_VALID_LEN;
				curIndex = curIndex + WRITE_DATA_VALID_LEN;
				memcpy(write_buf + 9, szBuff, WRITE_DATA_VALID_LEN); // 0-8 is write head
			} else {
				write_buf[WRITE_DATA_VALID_LEN_BYTE] = lastdatalen;
				curIndex = curIndex + lastdatalen;
				memcpy(write_buf + 9, szBuff, lastdatalen);  // 0-8 is write head
			}
			res = elan_i2c_write(write_buf, ELAN_SEND_DATA_LEN);
			if (res) {
				TS_LOG_ERR("[elan]:updata fw write page fail!res=%d\n", res);
				return -EINVAL;
			}
		}
		msleep(10); // fw updata need
		res = elan_i2c_write(cmd_iap_write, ELAN_SEND_DATA_LEN);
		if (res) {
			TS_LOG_ERR("[elan]:write cmd_iap_write fail,res=%d\n", res);
			return -EINVAL;
		}
		msleep(200); // fw updata need
		res = get_ack_data();
		if (res) {
			TS_LOG_ERR("[elan]get_ack_data Fail\n");
			return -EINVAL;
		}
		mdelay(10);    // fw updata need
	}
	res = SendFinishCmd();
	if (res) {
		TS_LOG_ERR("[elan]SendFinishCmd Fail\n");
		return -EINVAL;
	}
	return NO_ERR;
}

static int elan_firmware_update(const struct firmware *fw)
{
	int res = 0;
	int retry_num = 0;
	const u8 *fw_data = NULL;
	int fw_size = 0;
	int page_num = 0;
	if (!fw) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	fw_data = fw->data;
	fw_size = fw->size;
	page_num = (fw_size / sizeof(uint8_t) / FW_PAGE_SIZE);
	TS_LOG_INFO("[elan]:fw pagenum=%d\n", page_num);
retry_updata:
	res = elan_ktf_hw_reset();
	if (res != NO_ERR) {
		TS_LOG_ERR("%s hw reset fail\n", __func__);
	}
	res = check_fw_status();
	if (res < 0) {
		TS_LOG_ERR("[elan]:tp is on unknown mode!\n");
		return -EINVAL;
	}
	res = enter_iap_mode();
	if (res) {
		TS_LOG_ERR("[elan]:enter updata fw mode fail!\n");
		return -EINVAL;
	}
	msleep(10);    // fw updata need
	res = hid_write_page(page_num, fw_data);
	if (res) {
		retry_num++;
		if (retry_num >= FW_UPDATE_RETRY) {
			TS_LOG_ERR("[elan]:retry updata fw fail!\n");
			return -EINVAL;
		} else {
			TS_LOG_INFO("[elan]:fw updata fail!,retry num=%d\n", retry_num);
			goto retry_updata;
		}
	}
	msleep(10); // fw updata finish need
	res = elan_ktf_hw_reset();
	if (res != NO_ERR) {
		TS_LOG_ERR("%s hw reset fail\n", __func__);
	}
	res = elan_read_fw_info();
	if (res) {
		TS_LOG_ERR("[elan]:%s,read fw info fail\n", __func__);
	}
	res = elan_read_tx_rx();
	if (res == NO_ERR && g_elan_ts->rx_num - 1 > 0 && g_elan_ts->tx_num - 1 > 0) {
		g_elan_ts->finger_y_resolution = (g_elan_ts->rx_num - 1) * FINGER_OSR;
		g_elan_ts->finger_x_resolution = (g_elan_ts->tx_num - 1) * FINGER_OSR;
		g_elan_ts->pen_y_resolution = (g_elan_ts->rx_num - 1) * PEN_OSR;
		g_elan_ts->pen_x_resolution = (g_elan_ts->tx_num - 1) * PEN_OSR;
	}
	res = elan_ktf_ts_calibrate();
	if (res) {
		TS_LOG_ERR("[elan]:%s,calibrate fail\n", __func__);
	}
	return NO_ERR;
}

static int elan_get_lcd_panel_info(void)
{
	struct device_node *dev_node = NULL;
	char *lcd_type = NULL;

	if (!g_elan_ts) {
		TS_LOG_ERR("%s, elan ts is null ptr\n", __func__);
		return -EINVAL;
	}
	dev_node = of_find_compatible_node(NULL, NULL, LCD_PANEL_TYPE_DEVICE_NODE_NAME);
	if (!dev_node) {
		TS_LOG_ERR("[elan]:%s: NOT found device node[%s]!\n", __func__, LCD_PANEL_TYPE_DEVICE_NODE_NAME);
		return -EINVAL;
	}

	lcd_type = (char *)of_get_property(dev_node, "lcd_panel_type", NULL);
	if (!lcd_type) {
		TS_LOG_ERR("[elan]:%s: Get lcd panel type faile!\n", __func__);
		return -EINVAL ;
	}

	strncpy(g_elan_ts->lcd_panel_info, lcd_type, LCD_PANEL_INFO_MAX_LEN - 1);
	TS_LOG_INFO("[elan]:lcd_panel_info = %s.\n", g_elan_ts->lcd_panel_info);

	return 0;
}

static void  elan_get_lcd_module_name(void)
{
	char temp[LCD_PANEL_INFO_MAX_LEN] = {0};
	int i = 0;

	strncpy(temp, g_elan_ts->lcd_panel_info, LCD_PANEL_INFO_MAX_LEN - 1);
	for (i = 0; i < (LCD_PANEL_INFO_MAX_LEN - 1) && (i < (MAX_STR_LEN - 1)); i++) {
		if (temp[i] == '_' || temp[i] == '-') {
			break;
		}
		g_elan_ts->lcd_module_name[i] = tolower(temp[i]);
	}
	TS_LOG_INFO("[elan]:lcd_module_name = %s.\n", g_elan_ts->lcd_module_name);

	return;
}

static int elan_ktf_fw_update_sd(void)
{
	int err = NO_ERR;
	const  struct firmware *fw_entry = NULL;

	char filename[MAX_NAME_LEN] = {0};
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_client) || (!g_elan_ts->elan_dev)) {
		TS_LOG_ERR("[elan]g_elan_ts is NULL\n");
		return -EINVAL;
	}

	scnprintf(filename, MAX_NAME_LEN, "%s", "ts/touch_screen_firmware.img");
	TS_LOG_INFO("[elan]:%s, enter\n", __func__);
	err = request_firmware(&fw_entry, filename, &g_elan_ts->elan_dev->dev);
	if (err < 0) {
		TS_LOG_ERR("[elan]:%s %d: Fail request firmware %s, retval = %d\n", \
				__func__, __LINE__, filename, err);
		goto EXIT;
	}

	atomic_set(&g_elan_ts->tp_mode, TP_FWUPDATA);
	__pm_stay_awake(g_elan_ts->wake_lock);
	err = elan_firmware_update(fw_entry);
	if (err) {
		TS_LOG_ERR("[elan]:updata fw fail!\n");
		atomic_set(&g_elan_ts->tp_mode, TP_RECOVER);
	} else {
		TS_LOG_DEBUG("[elan]:updata fw success!\n");
		atomic_set(&g_elan_ts->tp_mode, TP_NORMAL);
	}
	__pm_relax(g_elan_ts->wake_lock);
	TS_LOG_INFO("[elan]:%s: end!\n", __func__);
	release_firmware(fw_entry);
EXIT:
	return err;
}

static int elan_ktf_fw_update_boot(char *file_name)
{
	int err = NO_ERR;
	int New_Fw_Ver = 0;
	char file_path[MAX_NAME_LEN] = {0};
	const  struct firmware *fw_entry = NULL;
	const u8 *fw_data = NULL;
	if ((!file_name) || (!g_elan_ts) || (!g_elan_ts->elan_dev)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("[elan]:%s: enter!\n", __func__);
	err = elan_get_lcd_panel_info();
	if (err < 0) {
		TS_LOG_ERR("[elan]:elan_get_lcd_panel_info fail\n");
		return -EINVAL;
	}
	elan_get_lcd_module_name();
	strncat(file_name, g_elan_ts->project_id, sizeof(g_elan_ts->project_id));
	strncat(file_name, "_", strlen("_"));
	strncat(file_name, g_elan_ts->lcd_module_name, strlen(g_elan_ts->lcd_module_name));
	strncat(file_name, FW_SUFFIX, strlen(FW_SUFFIX));
	snprintf(file_path, strlen(file_name) + strlen("ts/") + 1, "ts/%s", file_name);
	TS_LOG_INFO("[elan]:start to request firmware %s,%s\n", file_name, file_path);

	err = request_firmware(&fw_entry, file_path, &g_elan_ts->elan_dev->dev);
	if (err < 0) {
		TS_LOG_ERR("[elan]:%s %d: Fail request firmware %s, retval = %d\n", \
				__func__, __LINE__, file_path, err);
		goto exit;
	}
	fw_data = fw_entry->data;
	New_Fw_Ver = fw_data[FWVER_HIGH_BYTE_IN_EKT] << 8 | fw_data[FWVER_LOW_BYTE_IN_EKT];
	TS_LOG_INFO("[elan]:new fw ver=%x\n", New_Fw_Ver);

	if ((New_Fw_Ver != (g_elan_ts->fw_ver)) || g_elan_ts->sd_fw_updata) {
		atomic_set(&g_elan_ts->tp_mode, TP_FWUPDATA);
		__pm_stay_awake(g_elan_ts->wake_lock);
		err = elan_firmware_update(fw_entry);
		if (err) {
			TS_LOG_ERR("[elan]:updata fw fail!\n");
			atomic_set(&g_elan_ts->tp_mode, TP_RECOVER);
		} else {
			TS_LOG_DEBUG("[elan]:updata fw success!\n");
			atomic_set(&g_elan_ts->tp_mode, TP_NORMAL);
		}
		__pm_relax(g_elan_ts->wake_lock);
	} else {
		TS_LOG_INFO("[elan]:fw ver is new don't need updata!\n");
	}
	TS_LOG_INFO("[elan]:%s: end!\n", __func__);
	release_firmware(fw_entry);
exit:
	return NO_ERR;
}

static int elan_ktf_parse_dts_power(struct device_node *device, struct ts_kit_device_data *chip_data)
{
	int slave_addr = 0;
	int ret = NO_ERR;
	if ((!device) || (!chip_data) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data) ||
			(!g_elan_ts->elan_chip_client->client)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	ret = of_property_read_u32(device, ELAN_SLAVE_ADDR, &slave_addr);
	if (ret) {
		g_elan_ts->elan_chip_client->client->addr = ELAN_I2C_ADRR;
	} else {
		g_elan_ts->elan_chip_client->client->addr = slave_addr;
	}
	TS_LOG_INFO("[elan]:tp ic slave_addr=%x\n", slave_addr);

	return NO_ERR;
}

static int elan_ktf_parse_dts(struct device_node *device, struct ts_kit_device_data *chip_data)
{
	int ret = NO_ERR;
	unsigned int value = 0;
	TS_LOG_INFO("[elan]:%s: call!\n", __func__);
	if ((!device) || (!chip_data) || (!g_elan_ts) || (!g_elan_ts->elan_chip_client)) {
		TS_LOG_ERR("[elan]:%s,null point\n", __func__);
		return -EINVAL;
	}
	mutex_init(&chip_data->device_call_lock);
	g_elan_ts->elan_chip_data->is_direct_proc_cmd = false;
	ret = of_property_read_u32(device, ELAN_MT_FLAGS_CONFIG, &value);
	if (ret) {
		g_elan_ts->mt_flags = 0;
	} else {
		g_elan_ts->mt_flags = value;
	}
	TS_LOG_INFO("[elan]:g_elan_ts->mt_flags = 0x%x\n", g_elan_ts->mt_flags);

	ret = of_property_read_u32(device, ELAN_SUPPORT_GET_TP_COLOR, &value);
	if (ret)
		g_elan_ts->support_get_tp_color = 0;
	else
		g_elan_ts->support_get_tp_color = value;

	TS_LOG_INFO("[elan]:g_elan_ts->support_get_tp_color = 0x%x\n",
		g_elan_ts->support_get_tp_color);

	return NO_ERR;
}

static int elan_power_init(void)
{
	int ret = 0;

	ret = ts_kit_power_supply_get(TS_KIT_IOVDD);
	if (ret) {
		TS_LOG_ERR("%s, get iovdd supply fail, %d\n", __func__, ret);
		return ret;
	}
	ret = ts_kit_power_supply_get(TS_KIT_VCC);
	if (ret) {
		TS_LOG_ERR("%s, get vcc supply fail, %d\n", __func__, ret);
		goto fail_get_vcc;
	}
	return 0;

fail_get_vcc:
	ret = ts_kit_power_supply_put(TS_KIT_IOVDD);
	if (ret) {
		TS_LOG_ERR("%s, put iovdd failed\n", __func__);
	}
	return ret;
}
static int elan_power_on(void)
{
	int rc = 0;
	TS_LOG_INFO("[elan]:%s, power control by touch ic\n", __func__);
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]%s:g_elan_ts is NULL\n", __func__);
		return -EINVAL;
	}
	atomic_set(&g_last_pen_inrange_status, TS_PEN_OUT_RANGE);
	(void)ts_event_notify(TS_PEN_OUT_RANGE); // notify pen out of range
	/* set voltage for vddd and vdda */
	rc = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_ON, 0);  /* power on vcc, delay 0ms */
	if (rc) {
		TS_LOG_ERR("%s, power on vcc fail, %d\n", __func__, rc);
		goto exit;
	}
	rc = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_ON, 0);    /* power on iovdd, delay 0ms */
	if (rc) {
		TS_LOG_ERR("%s, power on iovdd fail, %d\n", __func__, rc);
		goto power_off_vcc;
	}
	return NO_ERR;

power_off_vcc:
	rc = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_OFF, 0);
	if (rc) {
		TS_LOG_ERR("%s, power off vci failed\n", __func__);
	}
exit:
	return rc;
}

static void elan_power_off(void)
{
	int rc = 0;
	TS_LOG_INFO("[elan]:%s, power control by touch ic\n", __func__);

	rc = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_OFF, 0); /* power off vcc, delay 0ms */
	if (rc) {
		TS_LOG_ERR("%s, power on vcc fail, %d\n", __func__, rc);
	}
	rc = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_OFF, 0); /* power off iovdd, delay 0ms */
	if (rc) {
		TS_LOG_ERR("%s, power on iovdd fail, %d\n", __func__, rc);
	}

	atomic_set(&g_last_pen_inrange_status, TS_PEN_OUT_RANGE);
	(void)ts_event_notify(TS_PEN_OUT_RANGE); /* notify pen out of range */
	TS_LOG_INFO("report pen exit\n");
	return;
}

static void elan_power_release(void)
{
	int rc = 0;
	TS_LOG_INFO("[elan]:%s, power control by touch ic\n", __func__);

	rc = ts_kit_power_supply_put(TS_KIT_VCC);
	if (rc) {
		TS_LOG_ERR("%s, release vcc fail, %d\n", __func__, rc);
	}
	rc = ts_kit_power_supply_put(TS_KIT_IOVDD);
	if (rc) {
		TS_LOG_ERR("%s, release iovdd fail, %d\n", __func__, rc);
	}
	return;
}

static int elan_before_suspend(void)
{
	return NO_ERR;
}

static int elan_ktf_chip_detect(struct ts_kit_platform_data *platform_data)
{
	int ret = NO_ERR;
	TS_LOG_INFO("[elan]:%s enter!\n", __func__);

	if (((!platform_data) || (!platform_data->ts_dev)) ||
			(!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]:%s device, ts_kit_platform_data *data or data->ts_dev is NULL \n", __func__);
		ret = -ENOMEM;
		goto exit;
	}
	mutex_init(&g_elan_ts->ktf_mutex);
	g_elan_ts->elan_dev = platform_data->ts_dev;
	g_elan_ts->elan_dev->dev.of_node = g_elan_ts->elan_chip_data->cnode;
	g_elan_ts->elan_chip_client = platform_data;
	g_elan_ts->elan_chip_data->ts_platform_data = platform_data;

	g_elan_ts->elan_chip_data->is_new_oem_structure = 0;
	g_elan_ts->elan_chip_data->is_parade_solution = 0;
	g_elan_ts->elan_chip_data->is_ic_rawdata_proc_printf = 1; // need config
	g_elan_ts->reset_gpio = platform_data->reset_gpio;
	g_elan_ts->int_gpio = platform_data->irq_gpio;
	g_elan_ts->irq_id = gpio_to_irq(g_elan_ts->int_gpio);

	ret = elan_ktf_parse_dts_power(g_elan_ts->elan_chip_data->cnode, g_elan_ts->elan_chip_data);
	if (ret) {
		TS_LOG_ERR("[elan]:parse_dts for DT err\n");
		goto exit;
	}

	ret = elan_power_init();
	if (ret) {
		TS_LOG_ERR("[elan]:power init fail!ret=%d\n", ret);
		goto exit;
	}

	ret = elan_config_gpio();
	if (ret) {
		TS_LOG_ERR("[elan]:gpio config fail!ret=%d\n", ret);
		goto free_power_gpio;
	}

	ret = elan_power_on();
	if (ret) {
		TS_LOG_ERR("[elan]:%s, failed to enable power, rc = %d\n", __func__, ret);
		goto power_off;
	}
	msleep(1); // spec need
	ret = elan_ktf_hw_reset();
	if (ret != NO_ERR) {
		TS_LOG_ERR("%s hw reset fail\n", __func__);
	}
	ret = i2c_communicate_check();
	if (ret) {
		TS_LOG_ERR("[elan]:%s:not find elan tp device, ret=%d\n", __func__, ret);
		goto power_off;
	} else {
		TS_LOG_INFO("[elan]:%s:find elan tp device\n", __func__);
	}
	TS_LOG_INFO("[elan]:%s:elan chip detect success\n", __func__);
	ret = elan_ktf_parse_dts(g_elan_ts->elan_chip_data->cnode, g_elan_ts->elan_chip_data);
	if (ret) {
		TS_LOG_ERR("[elan]:elan_ktf_parse_dts fail\n");
		goto power_off;
	}

	return NO_ERR;
power_off:
	elants_reset_pin_low();
	mdelay(2); // spec need
	elan_power_off();
free_power_gpio:
	elan_power_release();
exit:
	TS_LOG_INFO("[elan]:%s:elan chip detect fail\n", __func__);
	if (g_elan_ts && g_elan_ts->elan_chip_data) {
		kfree(g_elan_ts->elan_chip_data);
		g_elan_ts->elan_chip_data = NULL;
	}

	if (g_elan_ts) {
		kfree(g_elan_ts);
		g_elan_ts = NULL;
	}
	return ret;
}

static int wait_int_low(void)
{
	int i = 0;
	for (i = 0; i < PROJECT_ID_POLL; i++) {
		if (gpio_get_value(g_elan_ts->int_gpio) == 0) {
			TS_LOG_INFO("[elan]:int is low!i=%d", i);
			return NO_ERR;
		} else {
			msleep(10); // IC need
		}
	}
	TS_LOG_ERR("[elan]:no data,int is always high!");
	return -EINVAL;
}

static int elan_project_color(void)
{
	int ret = 0;
	int i = 0;
	u8 rsp_buf[ELAN_RECV_DATA_LEN] = {0};
	u8 reg_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x96, 0x80, 0x80, 0x00, 0x00, 0x11 };
	u8 test_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x55, 0x55, 0x55, 0x55 };
	u8 project_cmd[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x06, 0x59, 0x00, 0x80, 0x80, 0x00, 0x40 };
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]%s:g_elan_ts is NULL\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&g_elan_ts->tp_mode) == TP_NORMAL) {
		for (i = 0; i < READ_PORJECT_ID_WORDS; i++) {
			reg_cmd[SEND_CMD_VALID_INDEX] = 0x80 + i;    // 0x80 project id addr index low byte ,
			ret = elan_i2c_write(reg_cmd, sizeof(reg_cmd));
			if (ret) {
				TS_LOG_ERR("[elan]:set project id reg cmd fail!ret=%d\n", ret);
				return -EINVAL;
			}
			ret = wait_int_low();
			if (ret) {
				TS_LOG_ERR("[elan]:wait_int_low fail!\n");
				return -EINVAL;
			}
			ret = elan_i2c_read(NULL, 0, rsp_buf, ELAN_RECV_DATA_LEN);
			if (ret) {
				TS_LOG_ERR("[elan]:i2c read data fail!\n");
				return -EINVAL;
			}
			TS_LOG_INFO("[elan]:project id:%x,%x,%x\n", rsp_buf[0], rsp_buf[7], rsp_buf[8]); // 0 data length 7 8 valid data
			if (i < (READ_PORJECT_ID_WORDS - 1)) {
				memcpy(g_elan_ts->project_id + i * 2, rsp_buf + PROJECT_ID_INDEX, 2); //  get two bytes once time
			} else {
				memcpy(g_elan_ts->color_id, rsp_buf + PROJECT_ID_INDEX, sizeof(g_elan_ts->color_id));
			}
		}
	} else {
		ret = elan_i2c_write(test_cmd, sizeof(test_cmd));
		if (ret) {
			TS_LOG_ERR("[elan]:set test cmd fail!ret=%d\n", ret);
			return -EINVAL;
		}
		msleep(15); // IC need
		ret = elan_i2c_write(project_cmd, sizeof(project_cmd));
		if (ret) {
			TS_LOG_ERR("[elan]:elan_i2c_write project_cmd fail!ret=%d\n", ret);
			ret = elan_ktf_hw_reset();
			if (ret != NO_ERR) {
				TS_LOG_ERR("%s hw reset fail\n", __func__);
			}
			return -EINVAL;
		}
		ret = wait_int_low();
		if (ret) {
			TS_LOG_ERR("[elan]:wait_int_low fail!\n");
			ret = elan_ktf_hw_reset();
			if (ret != NO_ERR) {
				TS_LOG_ERR("%s hw reset fail\n", __func__);
			}
			return -EINVAL;
		}
		ret = elan_i2c_read(NULL, 0, rsp_buf, ELAN_RECV_DATA_LEN);
		if (ret) {
			TS_LOG_ERR("[elan]:i2c read data fail!\n");
			if (elan_ktf_hw_reset()) {
				TS_LOG_ERR("%s hw reset fail\n", __func__);
			}
			return -EINVAL;
		}
		memcpy(g_elan_ts->project_id, rsp_buf + PROJECT_ID_INDEX, sizeof(g_elan_ts->project_id) - 1); // reserved 1 byte
		memcpy(g_elan_ts->color_id, rsp_buf + COLOR_ID_INDEX, sizeof(g_elan_ts->color_id));
		ret = elan_ktf_hw_reset();
		if (ret != NO_ERR) {
			TS_LOG_ERR("%s hw reset fail\n", __func__);
		}
	}
	g_elan_ts->project_id[sizeof(g_elan_ts->project_id) - 1] = '\0'; // 9 bytes valid,reserved 1 byte
	memcpy(g_elan_ts->elan_chip_data->module_name, g_elan_ts->project_id, sizeof(g_elan_ts->project_id));
	TS_LOG_INFO("[elan]:module_name=%s,color_id=%x\n", g_elan_ts->elan_chip_data->module_name, g_elan_ts->color_id[0]);
	if (g_elan_ts->support_get_tp_color) {
		cypress_ts_kit_color[0] = g_elan_ts->color_id[0];
		TS_LOG_INFO("support_get_tp_color, tp color:0x%x",
			cypress_ts_kit_color[0]);
	}
	return NO_ERR;
}

static int elan_read_fw_info(void)
{
	int ret = 0;
	unsigned int highbyte;
	unsigned int lowbyte;
	char ver_byte[2] = {0};
	/* Get firmware version */
	u8 cmd_ver[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x53, 0x00, 0x00, 0x01 };
	u8 resp_buf[ELAN_RECV_DATA_LEN] = {0};
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]%s:g_elan_ts is NULL\n", __func__);
		return -EINVAL;
	}
	ret = elan_i2c_read(cmd_ver, sizeof(cmd_ver), resp_buf, sizeof(resp_buf));
	if (ret) {
		TS_LOG_ERR("[elan]:get fw ver fail!ret=%d\n", ret);
		return -EINVAL;
	} else {
		TS_LOG_DEBUG("[elan]:buf[0]=%x,buf[1]=%x,buf[2]=%x,buf[3]=%x,buf[4]=%x\n", \
				resp_buf[0], resp_buf[1], resp_buf[2], resp_buf[3], resp_buf[4]);
	}
	highbyte = ((resp_buf[FW_INFO_INDEX] & 0x0f) << 4) | ((resp_buf[FW_INFO_INDEX + 1] & 0xf0) >> 4);
	lowbyte = ((resp_buf[FW_INFO_INDEX + 1] & 0x0f) << 4) | ((resp_buf[FW_INFO_INDEX + 2] & 0xf0) >> 4);
	g_elan_ts->fw_ver = highbyte << 8 | lowbyte;
	ver_byte[0] = (char)highbyte;
	ver_byte[1] = (char)lowbyte;
	snprintf(g_elan_ts->elan_chip_data->version_name, MAX_STR_LEN, "0x%02x%02x", ver_byte[0], ver_byte[1]);
	TS_LOG_INFO("[elan]:FW VER=0x%04x\n", g_elan_ts->fw_ver);
	return NO_ERR;
}

static int elan_read_tx_rx(void)
{
	int ret = 0;
	u8 info_buff[ELAN_SEND_DATA_LEN] = { 0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 resp_buf[ELAN_RECV_DATA_LEN] = {0};
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]%s:g_elan_ts is NULL\n", __func__);
		return -EINVAL;
	}

	ret = elan_i2c_read(info_buff, sizeof(info_buff), resp_buf, sizeof(resp_buf));
	if (ret) {
		TS_LOG_ERR("[elan]:get tp tx rx num fail!ret=%d\n", ret);
		return -EINVAL;
	} else {
		TS_LOG_INFO("[elan]:buf[6]=%x,buf[7]=%x\n", resp_buf[RX_NUM_INDEX],
				resp_buf[TX_NUM_INDEX]); // 6th rx num byte 7th tx num byte
	}
	g_elan_ts->rx_num = resp_buf[RX_NUM_INDEX];
	g_elan_ts->tx_num = resp_buf[TX_NUM_INDEX];
	if (g_elan_ts->rx_num <= 0 || g_elan_ts->tx_num <= 0) {
		TS_LOG_ERR("[elan]:tp tx rx invalid\n");
		return -EINVAL;
	}
	TS_LOG_INFO("[elan]:rx=%d,tx=%d\n", g_elan_ts->rx_num, g_elan_ts->tx_num);
	return NO_ERR;
}

static int elan_ktf_init_chip(void)
{
	int ret = NO_ERR;
	TS_LOG_INFO("[ELAN]:%s\n", __func__);
	if ((!g_elan_ts) || (!g_elan_ts->elan_chip_client)) {
		TS_LOG_ERR("[elan]%s:g_elan_ts is NULL\n", __func__);
		return -EINVAL;
	}
#ifdef ELAN_IAP
	g_elan_ts->elan_device.minor = MISC_DYNAMIC_MINOR;
	g_elan_ts->elan_device.name = "elan-iap";
	g_elan_ts->elan_device.fops = &elan_touch_fops;
	ret = misc_register(&g_elan_ts->elan_device);
	if (ret < 0) {
		TS_LOG_ERR("[elan]:misc_register failed!!\n");
	} else {
		TS_LOG_INFO("[elan]:misc_register finished!!\n");
	}
#endif
	strncpy(g_elan_ts->elan_chip_data->chip_name, ELAN_KTF_NAME, strlen(ELAN_KTF_NAME) + 1);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_elan_ts->wake_lock = wakeup_source_register(
		&g_elan_ts->elan_dev->dev, "elantp_wake_lock");
#else
	g_elan_ts->wake_lock = wakeup_source_register("elantp_wake_lock");
#endif
	if (!g_elan_ts->wake_lock) {
		TS_LOG_ERR("%s wakeup source register failed\n", __func__);
		return -EINVAL;
	}
	ret = check_fw_status();
	if (ret < 0) {
		TS_LOG_ERR("[elan]:ic is unknown mode\n");
		return ret;
	}
	ret = elan_project_color();
	if (ret) {
		TS_LOG_ERR("[elan]:read project id and color fail\n");
		return ret;
	}
	/* provide panel_id for sensor,panel_id is high byte *10 + low byte */
	g_elan_ts->elan_chip_client->panel_id = (g_elan_ts->project_id[ELAN_PANEL_ID_START_BIT] - '0') * 10 +
		g_elan_ts->project_id[ELAN_PANEL_ID_START_BIT + 1] - '0';
	TS_LOG_INFO("%s: panel_id=%d\n", __func__, g_elan_ts->elan_chip_client->panel_id);
	if (atomic_read(&g_elan_ts->tp_mode) == TP_NORMAL) {
		ret = elan_read_fw_info();
		if (ret) {
			TS_LOG_ERR("[elan]:read tp fw fail!\n");
			return -EINVAL;
		}
		ret = elan_read_tx_rx();
		if (ret == NO_ERR) {
			g_elan_ts->finger_y_resolution = (g_elan_ts->rx_num - 1) * FINGER_OSR;
			g_elan_ts->finger_x_resolution = (g_elan_ts->tx_num - 1) * FINGER_OSR;
			g_elan_ts->pen_y_resolution = (g_elan_ts->rx_num - 1) * PEN_OSR;
			g_elan_ts->pen_x_resolution = (g_elan_ts->tx_num - 1) * PEN_OSR;
		} else {
			TS_LOG_ERR("[elan]:read tx rx num fail!\n");
			return -EINVAL;
		}
	}
	g_elan_ts->sd_fw_updata = false;
	g_elan_ts->pen_detected = false;

#if defined(CONFIG_TEE_TUI)
	strncpy(tee_tui_data.device_name, g_elan_ts->project_id,
		sizeof(g_elan_ts->project_id));
	tee_tui_data.device_name[strlen(g_elan_ts->project_id)] = '\0';
#endif
	return NO_ERR;
}

static int elan_ktf_input_config(struct input_dev *input_dev)
{
	if ((!input_dev) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]%s:arg is NULL\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("[ELAN]:%s enter\n", __func__);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, FINGER_MAJOR, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MINOR, 0, FINGER_MINOR, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGER_SIZE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
		g_elan_ts->elan_chip_data->y_max - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
		g_elan_ts->elan_chip_data->x_max - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0,
		FINGER_MAJOR, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0,
		FINGER_MINOR, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MAX_FINGER_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_DISTANCE, 0, MAX_FINGER_SIZE, 0, 0);
	return NO_ERR;
}

static int elan_ktf_pen_config(struct input_dev *input_dev)
{
	struct input_dev *input =  input_dev;
	if ((!input_dev) || (!g_elan_ts) || (!g_elan_ts->elan_chip_data)) {
		TS_LOG_ERR("[elan]:%s,date is null\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("[elan]:elan_pen_input_config called\n");

	input->evbit[0] |= BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	__set_bit(ABS_X, input->absbit);
	__set_bit(ABS_Y, input->absbit);
	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(BTN_TOOL_PEN, input->keybit);
	__set_bit(INPUT_PROP_DIRECT, input->propbit);
	input_set_abs_params(input, ABS_X, 0, g_elan_ts->elan_chip_data->x_max - 1, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, g_elan_ts->elan_chip_data->y_max - 1, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, MAX_PEN_PRESSURE, 0, 0);
	return NO_ERR;
}

static int elan_ktf_irq_top_half(struct ts_cmd_node *cmd)
{
	cmd->command = TS_INT_PROCESS;
	return NO_ERR;
}

static void elan_report_debug_info(u8 debug_info)
{
	u8 trace_mode = (debug_info & ELAN_TRACE_MODE_INFO_MASK) >>
		TRACE_MODE_OFFSET;
	u8 water_mode = (debug_info & ELAN_WATER_MODE_INFO_MASK) >>
		WATER_MODE_OFFSET;
	u8 palm_mode = debug_info & ELAN_PALM_MODE_INFO_MASK;
	u8 freq_mode = debug_info & ELAN_FREQ_HOP_INFO_MASK;
	u8 obl_mode = debug_info & ELAN_OBL_MODE_INFO_MASK;

	TS_LOG_INFO("trace=%d,Water=%d,Palm=%d,Freq Hop=%d,OBL=%d\n",
		trace_mode, water_mode, palm_mode, freq_mode, obl_mode);
#if defined(CONFIG_HUAWEI_DSM)
	ts_dmd_report(DSM_TP_CHARGER_NOISE_HOP,
		"try to client record DSM_TP_CHARGER_NOISE_HOP:%d\n"
		"elan tp enter abnormal mode:\n"
		"abnormal mode description:\n"
		"trach mode:0x%02x\n"
		"water mode:0x%02x\n"
		"palm mode:0x%02x\n"
		"freq hop:0x%02x\n"
		"OBL mode:0x%02x\n",
		DSM_TP_CHARGER_NOISE_HOP,
		trace_mode, water_mode, palm_mode, freq_mode, obl_mode);
#endif
}

static int elan_ktf_irq_bottom_half(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int ret = 0;
	u8 buf[TEN_FINGER_DATA_LEN] = { 0 };
	struct algo_param *algo_p = NULL;
	struct ts_fingers *info = NULL;
	struct ts_pens *pens = NULL;
	if (!out_cmd) {
		TS_LOG_ERR("[elan]:%s out cmd is null\n", __func__);
		return -EINVAL;
	}

	algo_p = &out_cmd->cmd_param.pub_params.algo_param;
	info = &algo_p->info;
	ret = elan_ktf_ts_recv_data(buf);
	if (ret) {
		TS_LOG_ERR("[elan]:recv data fail\n");
		return ret;
	}

	switch (buf[REPORT_ID_BYTE])
	{
		case FINGER_REPORT_ID:
			out_cmd->command = TS_INPUT_ALGO;
			parse_fingers_point(info, buf);
			break;
		case PEN_REPORT_ID:
			pens = &out_cmd->cmd_param.pub_params.report_pen_info;
			pens->tool.tool_type = BTN_TOOL_PEN;
			out_cmd->command = TS_REPORT_PEN;
			parse_pen_point(pens, buf, out_cmd);
			if (pens->tool.pen_inrange_status != atomic_read(&g_last_pen_inrange_status)) {
				atomic_set(&g_last_pen_inrange_status, pens->tool.pen_inrange_status);
				if (TS_PEN_OUT_RANGE < pens->tool.pen_inrange_status) {
					(void)ts_event_notify(TS_PEN_IN_RANGE);
				} else {
					(void)ts_event_notify(TS_PEN_OUT_RANGE);
				}
			}
			break;
		case ELAN_DEBUG_MESG:
			elan_report_debug_info(buf[ELAN_IC_DEBUG_INFO]);
			break;
		default:
			// 0th len 2th id 4,5th other
			TS_LOG_INFO("[elan]:unknown report id:%0x,%0x,%0x,%0x\n", buf[0], buf[2], buf[4], buf[5]);
			break;
	}
	return NO_ERR;
}

static int elan_ktf_hw_reset(void)
{
	TS_LOG_INFO("[elan]:%s\n", __func__);
	elants_reset_pin_high();
	msleep(5); // ic reset need
	elants_reset_pin_low();
	msleep(10); // ic reset need
	elants_reset_pin_high();
	msleep(300); // ic reset need
	return NO_ERR;
}

static int elan_ktf_ts_set_power_state(unsigned int state)
{
	int ret = 0;
	u8 cmd[ELAN_SEND_DATA_LEN] = {0x04, 0x00, 0x23, 0x00, 0x03, 0x00, 0x04, 0x54, 0x50, 0x00, 0x01};
	cmd[SUSPEND_OR_REPORT_BYTE] |= (state << 3);
	ret = elan_i2c_write(cmd, sizeof(cmd));
	if (ret) {
		TS_LOG_ERR("[elan] %s: i2c_master_send failed!ret=%d\n", __func__, ret);
		return -EINVAL;
	}
	return NO_ERR;
}

static int elan_ktf_core_suspend(void)
{
	int rc = NO_ERR;
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s,g_elan_ts is null\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&g_elan_ts->tp_mode) == TP_NORMAL) {
		TS_LOG_INFO("[elan] %s: enter\n", __func__);
		rc = elan_ktf_ts_set_power_state(0); // suspend
		if (rc) {
			TS_LOG_ERR("[elan]:suspend fail\n");
		}
	}

	atomic_set(&g_last_pen_inrange_status, TS_PEN_OUT_RANGE);
	(void)ts_event_notify(TS_PEN_OUT_RANGE); /* notify pen out of range */
	TS_LOG_INFO("report pen exit\n");
	return rc;
}

static int elan_ktf_core_resume(void)
{
	int rc = NO_ERR;
	int ret = NO_ERR;
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:%s,g_elan_ts is null\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&g_elan_ts->tp_mode) == TP_NORMAL) {
		TS_LOG_INFO("[elan]:%s: enter\n", __func__);
		rc = elan_ktf_ts_set_power_state(1); // 1: power on
		if (rc) {
			ret = elan_ktf_hw_reset();
			if (ret != NO_ERR) {
				TS_LOG_ERR("%s hw reset fail\n", __func__);
			}
		}
	}

	atomic_set(&g_last_pen_inrange_status, TS_PEN_OUT_RANGE);
	(void)ts_event_notify(TS_PEN_OUT_RANGE); /* notify pen out of range */
	TS_LOG_INFO("report pen exit\n");
	return rc;
}

static void elan_chip_shutdown(void)
{
	TS_LOG_INFO("[elan]:%s: enter\n", __func__);
	elants_reset_pin_low();
	mdelay(2); // spec need
	elan_power_off();
	elan_power_release();
	return;
}
static int __init elan_ktf_ts_init(void)
{
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;
	int error = NO_ERR;

	TS_LOG_INFO("[elan]:%s enter!\n", __func__);
	g_elan_ts = kzalloc(sizeof(struct elan_ktf_ts_data), GFP_KERNEL);
	if (!g_elan_ts) {
		TS_LOG_ERR("[elan]:Failed to alloc mem for struct elan_ktf_ts_data!\n");
		error =  -ENOMEM;
		goto fail_request_elan_ts;
	}
	g_elan_ts->elan_chip_data = kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (!g_elan_ts->elan_chip_data) {
		TS_LOG_ERR("[elan]:Failed to alloc mem for struct elan_chip_data\n");
		error =  -ENOMEM;
		goto free_elan_ts;
	}

	root = of_find_compatible_node(NULL, NULL, "huawei,ts_kit");
	if (!root) {
		TS_LOG_ERR("[elan]:huawei_ts, find_compatible_node huawei,ts_kit error\n");
		error = -EINVAL;
		goto exit;
	}

	for_each_child_of_node(root, child) // find the chip node
	{
		if (of_device_is_compatible(child, ELAN_KTF_NAME)) {
			found = true;
			break;
		}
	}
	if (!found) {
		TS_LOG_ERR("[elan]:not found chip elan child node!\n");
		error = -EINVAL;
		goto exit;
	}

	g_elan_ts->elan_chip_data->cnode = child;
	g_elan_ts->elan_chip_data->ops = &ts_kit_elan_ops;

	error = huawei_ts_chip_register(g_elan_ts->elan_chip_data);
	if (error) {
		TS_LOG_ERR("[elan]: chip register fail !\n");
		goto exit;
	}
	TS_LOG_INFO("[elan]: chip_register! err=%d\n", error);
	return error;
exit:
	if (g_elan_ts && g_elan_ts->elan_chip_data) {
		kfree(g_elan_ts->elan_chip_data);
		g_elan_ts->elan_chip_data = NULL;
	}

free_elan_ts:
	if (g_elan_ts) {
		kfree(g_elan_ts);
		g_elan_ts = NULL;
	}
fail_request_elan_ts:
	return error;
}

static void __exit elan_ktf_ts_exit(void)
{
	return;
}

late_initcall(elan_ktf_ts_init);
module_exit(elan_ktf_ts_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");

