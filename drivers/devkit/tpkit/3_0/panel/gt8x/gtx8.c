#include "gtx8.h"
#include "gtx8_dts.h"
#include "tpkit_platform_adapter.h"
#include "huawei_ts_kit_misc.h"

#if defined (CONFIG_TEE_TUI)
#include "tui.h"
#define GTX8_DEVICE_NAME "gtx8"
#define IC_GTX8_6861 "6861_gtx8"
#endif

static struct mutex wrong_touch_lock;

extern u8 cypress_ts_kit_color[TP_COLOR_SIZE];

static DEFINE_MUTEX(wrong_touch_lock);

static struct completion roi_data_done;
struct gtx8_ts_data *gtx8_ts = NULL;
static u16 pre_index = 0;
static u16 cur_index = 0;

#if defined (CONFIG_TEE_TUI)
extern struct ts_tui_data tee_tui_data;
#endif

static void gtx8_touch_switch(void);
int stylus3_connect_state(unsigned int stylus3_status);
void gtx8_pen_respons_mode_change(int soper);

/* stylus type information */
static int stylus3_type_info_handle(u32 stylus3_type_info);
struct styus3_type_info_table_entry {
	u8 vendor;
	u8 checksum;
	u8 info[GT73X_STYLUS3_TYPE_DATA_LEN];
};
static const struct styus3_type_info_table_entry stylus3_type_info_table[] = {
	{
		.vendor = GT73X_STYLUS3_TYPE1,
		.checksum = GT73X_STYLUS3_TYPE1_CHECK_DATA,
		.info = { GT73X_STYLU3_TYPE_DATA0,
			GT73X_STYLU3_TYPE1_DATA1, GT73X_STYLU3_TYPE1_DATA2 }
	}, {
		.vendor = GT73X_STYLUS3_TYPE2,
		.checksum = GT73X_STYLUS3_TYPE2_CHECK_DATA,
		.info = { GT73X_STYLU3_TYPE_DATA0,
			GT73X_STYLU3_TYPE2_DATA1, GT73X_STYLU3_TYPE2_DATA2 }
	}, {
		.vendor = GT73X_STYLUS3_TYPE3,
		.checksum = GT73X_STYLUS3_TYPE3_CHECK_DATA,
		.info = { GT73X_STYLU3_TYPE_DATA0,
			GT73X_STYLU3_TYPE3_DATA1, GT73X_STYLU3_TYPE3_DATA2 }
	}
};

u32 getUint(u8 *buffer, int len)
{
	u32 num = 0;
	int i = 0;
	for (i = 0; i < len; i++) {
		num <<= 8;
		num += buffer[i];
	}
	return num;
}

/**
 * gtx8_i2c_read_trans - i2c read in normal mode.
 * @addr: register address.
 * @buffer: data buffer.
 * @len: the bytes of data to write.
 * Return: 0: success, otherwise: failed
 */
int gtx8_i2c_read_trans(u16 addr, u8 *buffer, u32 len)
{
	struct ts_kit_device_data *dev_data = gtx8_ts->dev_data;
	u8 data[2] = {0};
	int ret = 0;

	data[0] = (u8)((addr >> 8) & 0xFF);
	data[1] = (u8)(addr & 0xFF);
	ret = dev_data->ts_platform_data->bops->bus_read(data, 2, buffer, len);
	if (ret < 0)
		TS_LOG_ERR("%s:i2c read error,addr:%04x bytes:%d\n",
				__func__, addr, len);

	return ret;
}

/**
 * gtx8_i2c_write_trans - i2c write in normal mode.
 * @addr: register address.
 * @buffer: data buffer.
 * @len: the bytes of data to write.
 * Return: 0: success, otherwise: failed
 */
int gtx8_i2c_write_trans(u16 addr, u8 *buffer, u32 len)
{
	struct ts_kit_device_data *dev_data =  gtx8_ts->dev_data;
	u8 stack_mem[64] = {0};
	u8 *data = NULL;
	int ret = 0;
	if(!buffer){
		TS_LOG_ERR("%s: invalid input.\n", __func__);
		return -EINVAL;
	}
	if (len + 2 > sizeof(stack_mem)) {
		data = kzalloc(len + 2, GFP_KERNEL);
		if (!data) {
			TS_LOG_ERR("%s: No memory,len =%d\n", __func__,len);
			return -ENOMEM;
		}
	} else {
		data = &stack_mem[0];
	}

	data[0] = (u8)((addr >> 8) & 0xFF);
	data[1] = (u8)(addr & 0xFF);
	memcpy(&data[2], buffer, len);
	ret = dev_data->ts_platform_data->bops->bus_write(data, len + 2);
	if (ret < 0)
		TS_LOG_ERR("%s:i2c write error,addr:%04x bytes:%d\n",
				__func__, addr, len);

	if (data != &stack_mem[0]) {
		kfree(data);
		data = NULL;
	}

	return ret;
}

/**
 * gtx8_set_i2c_doze_mode - disable or enable doze mode
 * @enable: true/flase
 * return: 0 - ok; < 0 - error.
 * This func must be used in pairs, when you disable doze
 * mode, then you must enable it again.
 * Between set_doze_false and set_doze_true, do not reset
 * IC!
*/
int gtx8_set_i2c_doze_mode(int enable)
{
	int result = -EINVAL;
	int i = 0;
	u8 w_data = 0, r_data = 0;

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		return NO_ERR;

	mutex_lock(&gtx8_ts->doze_mode_lock);

	if (enable) {
		if (gtx8_ts->doze_mode_set_count != 0)
			gtx8_ts->doze_mode_set_count--;

		/*when count equal 0, allow ic enter doze mode*/
		if (gtx8_ts->doze_mode_set_count == 0) {
			w_data = GTX8_DOZE_ENABLE_DATA;
			for (i = 0; i < GTX8_RETRY_NUM_3; i++) {
				if (!gtx8_i2c_write_trans(GTP_REG_DOZE_CTRL, &w_data, 1)) {
					result = NO_ERR;
					goto exit;
				}
				usleep_range(1000, 1100);
			}
			if (i >= GTX8_RETRY_NUM_3)
				TS_LOG_ERR("i2c doze mode enable failed, i2c write fail\n");
		} else {
			result = NO_ERR;
			goto exit;
		}
	} else {
		gtx8_ts->doze_mode_set_count++;

		if (gtx8_ts->doze_mode_set_count == 1) {
			w_data = GTX8_DOZE_DISABLE_DATA;
			if (gtx8_i2c_write_trans(GTP_REG_DOZE_CTRL, &w_data, 1)) {
				TS_LOG_ERR("doze mode comunition disable FAILED\n");
				goto exit;
			}
			usleep_range(1000, 1100);
			for (i = 0; i < 9; i++) {
				if (gtx8_i2c_read_trans(GTP_REG_DOZE_STAT, &r_data, 1)) {
					TS_LOG_ERR("doze mode comunition disable FAILED\n");
					goto exit;
				}
				if (GTX8_DOZE_CLOSE_OK_DATA == r_data) {
					result = NO_ERR;
					goto exit;
				} else if (0xAA != r_data){
					w_data = GTX8_DOZE_DISABLE_DATA;
					if (gtx8_i2c_write_trans(GTP_REG_DOZE_CTRL, &w_data, 1)) {
						TS_LOG_ERR("doze mode comunition disable FAILED\n");
						goto exit;
					}
				}
				usleep_range(10000, 11000);
			}
			TS_LOG_ERR("doze mode disable FAILED\n");
		} else {
			result = NO_ERR;
			goto exit;
		}
	}

exit:
	mutex_unlock(&gtx8_ts->doze_mode_lock);
	return result;
}

/**
 * gtx8_i2c_read - i2c read in normal & doze mode.
 * @addr: register address.
 * @buffer: data buffer.
 * @len: the bytes of data to write.
 * Return: 0: success, otherwise: failed
 */
int gtx8_i2c_read(u16 addr, u8 *buffer, u32 len)
{
	int ret = -EINVAL;

	if (gtx8_set_i2c_doze_mode(false) != 0){
		TS_LOG_ERR("gtx8 i2c read:0x%04x ERROR, disable doze mode FAILED\n", addr);
		goto exit;
	}
	ret = gtx8_i2c_read_trans(addr, buffer, len);
exit:
	if (gtx8_set_i2c_doze_mode(true) != 0)
		TS_LOG_ERR("gtx8 i2c read:0x%04x ERROR, enable doze mode FAILED\n", addr);

	return ret;
}

/**
 * gtx8_i2c_write - i2c write in normal & doze mode.
 * @addr: register address.
 * @buffer: data buffer.
 * @len: the bytes of data to write.
 * Return: 0: success, otherwise: failed
 */
int gtx8_i2c_write(u16 addr, u8 *buffer, u32 len)
{
	int ret = -EINVAL;

	if (gtx8_set_i2c_doze_mode(false) != 0){
		TS_LOG_ERR("gtx8 i2c write:0x%04x ERROR, disable doze mode FAILED\n",addr);
		goto exit;
	}
	ret = gtx8_i2c_write_trans(addr, buffer, len);

exit:
	if (gtx8_set_i2c_doze_mode(true) != 0)
		TS_LOG_ERR("gtx8 i2c write:0x%04x ERROR, enable doze mode FAILED\n",addr);

	return ret;
}


/**
 * gtx8_send_cmd - seng cmd to firmware
 * @cmd: pointer to command struct which cotain command data
 * Returns 0 - succeed,<0 - failed
 */
static int gtx8_send_cmd(u8 cmd, u8 data, u8 issleep)
{
	s32 ret = 0;
	static DEFINE_MUTEX(cmd_mutex);
	u8 buffer[3] = { cmd, data, 0 };

	TS_LOG_DEBUG("%s: Send command:%u\n", __func__, cmd);
	mutex_lock(&cmd_mutex);
	buffer[2] = (u8) ((0 - cmd - data) & 0xFF);
	ret = gtx8_i2c_write(gtx8_ts->reg.command, &buffer[0], 3);
	if (ret < 0)
		TS_LOG_ERR("%s: i2c_write command fail\n", __func__);

	if (issleep)
		msleep(50);

	mutex_unlock(&cmd_mutex);
	return ret;
}

/**
* gtx8_check_cfg_valid_gt7382 - check config valid.
* @cfg : config data pointer.
* @length : config length.
* Returns 0 - succeed,<0 - failed
*/
static int gtx8_check_cfg_valid_gt7382(u8 *cfg, u32 length)
{
	int ret;
	u8 data_sum;

	TS_LOG_INFO("%s run\n", __func__);

	if (!cfg || (length == 0)) {
		TS_LOG_ERR("%s: cfg is invalid ,length:%d\n", __func__, length);
		ret = -EINVAL;
		goto exit;
	}
	data_sum = checksum_u8(cfg, length);
	if (data_sum == 0) {
		ret = 0;
	} else {
		TS_LOG_ERR("cfg sum:0x%x\n", data_sum);
		ret = -EINVAL;
	}

exit:
	TS_LOG_INFO("%s exit,ret:%d\n", __func__, ret);
	return ret;
}

/**
* gtx8_check_cfg_valid - check config valid.
* @cfg : config data pointer.
* @length : config length.
* Returns 0 - succeed,<0 - failed
*/
static int gtx8_check_cfg_valid(u8 *cfg, u32 length)
{
	int ret = NO_ERR;
	u8 bag_num = 0;
	u8 checksum = 0;
	int i = 0, j = 0;
	int bag_start = 0;
	int bag_end = 0;

	if (!cfg || length < GTX8_CFG_HEAD_LEN) {
		TS_LOG_ERR("cfg is INVALID, len:%d\n", length);
		ret = -EINVAL;
		goto exit;
	}

	for (i = 0; i < GTX8_CFG_HEAD_LEN; i++)
		checksum += cfg[i];
	if (checksum != 0) {
		TS_LOG_ERR("cfg head checksum ERROR, ic type:normandy, checksum:0x%02x\n",
						checksum);
		ret = -EINVAL;
		goto exit;
	}
	bag_num = cfg[GTX8_CFG_BAG_NUM_INDEX];
	bag_start = GTX8_CFG_BAG_START_INDEX;

	TS_LOG_INFO("cfg bag_num:%d, cfg length:%d\n", bag_num, length);

	/*check each bag's checksum*/
	for (j = 0; j < bag_num; j++) {
		if (bag_start >= length - 1) {
			TS_LOG_ERR("ERROR, overflow!!bag_start:%d, cfg_len:%d\n", bag_start, length);
			ret = -EINVAL;
			goto exit;
		}

		bag_end = bag_start + cfg[bag_start + 1] + 3;

		checksum = 0;
		if (bag_end > length) {
			TS_LOG_ERR("ERROR, overflow!!bag:%d, bag_start:%d,  bag_end:%d, cfg length:%d\n",
					j, bag_start, bag_end, length);
			ret = -EINVAL;
			goto exit;
		}
		for (i = bag_start; i < bag_end; i++)
			checksum += cfg[i];
		if (checksum != 0) {
			TS_LOG_ERR("cfg INVALID, bag:%d checksum ERROR:0x%02x\n", j, checksum);
			ret = -EINVAL;
			goto exit;
		}
		bag_start = bag_end;
	}

	ret = NO_ERR;
	TS_LOG_INFO("configuration check SUCCESS\n");

exit:
	return ret;
}

/**
* gtx8_send_large_config - send large config.
* config : config pointer to send.
* Returns 0 - succeed,<0 - failed.
*/
static int gtx8_send_large_config(struct gtx8_ts_config *config)
{
	int ret = NO_ERR;
	int try_times = 0;
	u8 buf = 0;
	u16 command_reg = gtx8_ts->reg.command;
	u16 cfg_reg = gtx8_ts->reg.cfg_addr;
	int read_try_times;

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		read_try_times = GTX8_RETRY_NUM_30;
	else
		read_try_times = GTX8_RETRY_NUM_10;
	/*1. Inquire command_reg until it's free*/
	for (try_times = 0; try_times < read_try_times; try_times++) {
		if (!gtx8_i2c_read(command_reg, &buf, 1) && buf == GTX8_CMD_COMPLETED)
			break;
		usleep_range(10000, 11000);
	}
	if (try_times >= read_try_times) {
		TS_LOG_ERR("Send large cfg FAILED, before send, reg:0x%04x is not 0xff\n", command_reg);
		ret = -EINVAL;
		goto exit;
	}

	/*2. send "start write cfg" command*/
	if (gtx8_send_cmd(GTX8_CMD_START_SEND_CFG, 0, GTX8_NOT_NEED_SLEEP)) {
		TS_LOG_ERR("Send large cfg FAILED, send COMMAND_START_SEND_CFG ERROR\n");
		ret = -EINVAL;
		goto exit;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		usleep_range(3000, 3100); /* ic need delay */
		read_try_times = GTX8_RETRY_NUM_20;
	}
	/*3. wait ic set command_reg to 0x82*/
	for (try_times = 0; try_times < read_try_times; try_times++) {
		if (!gtx8_i2c_read(command_reg, &buf, 1) && buf == 0x82)
			break;
		if (gtx8_ts->ic_type == IC_TYPE_7382)
			usleep_range(3000, 3100);
		else
			usleep_range(10000, 11000);
	}
	if (try_times >= read_try_times) {
		TS_LOG_ERR("Send large cfg FAILED, reg:0x%04x is not 0x82\n", command_reg);
		ret = -EINVAL;
		goto exit;
	}

	/*4. write cfg*/
	if (gtx8_i2c_write(cfg_reg, config->data, config->length)) {
		TS_LOG_ERR("Send large cfg FAILED, write cfg to fw ERROR\n");
		ret = -EINVAL;
		goto exit;
	}

	/*5. send "end send cfg" command*/
	if (gtx8_send_cmd(GTX8_CMD_END_SEND_CFG, 0, GTX8_NOT_NEED_SLEEP)) {
		TS_LOG_ERR("Send large cfg FAILED, send COMMAND_END_SEND_CFG ERROR\n");
		ret = -EINVAL;
		goto exit;
	}

	/*6. wait ic set command_reg to 0xff*/
	for (try_times = 0; try_times < GTX8_RETRY_NUM_10; try_times++) {
		if (!gtx8_i2c_read(command_reg, &buf, 1) && buf == GTX8_CMD_COMPLETED)
			break;
		usleep_range(10000, 11000);
	}
	if (try_times >= GTX8_RETRY_NUM_10) {
		TS_LOG_ERR("Send large cfg FAILED, after send, reg:0x%04x is not 0xff\n", command_reg);
		ret = -EINVAL;
		goto exit;
	}

	TS_LOG_INFO("Send large cfg SUCCESS\n");
	ret = NO_ERR;

exit:
	return ret;
}

/**
* gtx8_send_small_config - send small config.
* config : config pointer to send.
* Returns 0 - succeed,<0 - failed.
*/
static int gtx8_send_small_config(struct gtx8_ts_config *config)
{
	int ret = NO_ERR;
	int try_times = 0;
	u8 buf = 0;
	u16 command_reg = gtx8_ts->reg.command;
	u16 cfg_reg = gtx8_ts->reg.cfg_addr;

	/*1. Inquire command_reg until it's free*/
	for (try_times = 0; try_times < GTX8_RETRY_NUM_10; try_times++) {
		if (!gtx8_i2c_read(command_reg, &buf, 1) && buf == GTX8_CMD_COMPLETED)
			break;
		usleep_range(10000, 11000);
	}
	if (try_times >= GTX8_RETRY_NUM_10) {
		TS_LOG_ERR("Send small cfg FAILED, before send, reg:0x%04x is not 0xff\n", command_reg);
		ret = -EINVAL;
		goto exit;
	}

	/*2. write cfg data*/
	if (gtx8_i2c_write(cfg_reg, config->data, config->length)) {
		TS_LOG_ERR("Send small cfg FAILED, write cfg to fw ERROR\n");
		ret = -EINVAL;
		goto exit;
	}

	/*3. send 0x81 command*/
	if (gtx8_send_cmd(GTX8_CMD_SEND_SMALL_CFG, 0, GTX8_NOT_NEED_SLEEP)) {
		TS_LOG_ERR("Send small cfg FAILED, send COMMAND_SEND_SMALL_CFG ERROR");
		ret = -EINVAL;
		goto exit;
	}

	ret = NO_ERR;
	TS_LOG_INFO("send small cfg SUCCESS\n");

exit:
	return ret;
}

/**
* gtx8_send_cfg - send config.
* config : config pointer to send.
* Returns 0 - succeed,<0 - failed.
*/
static int gtx8_send_cfg(struct gtx8_ts_config *config)
{
	int ret = -EINVAL;

	if (!config || !config->initialized) {
		TS_LOG_ERR("invalied config data\n");
		return -EINVAL;
	}

	/*check configuration valid*/
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		ret = gtx8_check_cfg_valid_gt7382(config->data, config->length);
		if (ret != 0) {
			TS_LOG_ERR("cfg check failed\n");
			return -EINVAL;
		}
		TS_LOG_INFO("ver:%02xh,size:%d\n", config->data[0],
			config->length);
		mutex_lock(&config->lock);
		ret = gtx8_send_large_config(config);
		if (ret != 0)
			TS_LOG_ERR("%s: send_cfg fail.\n", __func__);
		mutex_unlock(&config->lock);
		return ret;
	}
	ret = gtx8_check_cfg_valid(config->data, config->length);
	if (ret != 0) {
		TS_LOG_ERR("cfg check FAILED\n");
		return -EINVAL;
	}

	TS_LOG_INFO("ver:%02xh,size:%d\n", config->data[0],
					config->length);

	mutex_lock(&config->lock);

	/*disable doze mode*/
	ret = gtx8_set_i2c_doze_mode(false);
	if (ret) {
		TS_LOG_ERR("%s: failed disabled doze mode before send config[ignore]\n", __func__);
	}
	/* gt6861 need not to send cfg when doze mode failed */
	if (!(gtx8_ts->support_doze_failed_not_send_cfg && ret)) {
		if (config->length > GTX8_SMALL_OR_LARGE_CFG_LEN)
			ret = gtx8_send_large_config(config);
		else
			ret = gtx8_send_small_config(config);
	}

	if (ret != 0)
		TS_LOG_ERR("%s: send_cfg fail.\n", __func__);
	/*enable doze mode*/
	if (gtx8_set_i2c_doze_mode(true))
		TS_LOG_ERR("%s: failed enable doze mode\n", __func__);
	mutex_unlock(&config->lock);

	return ret;
}

/* success return config length else return -1 */
static int _gtx8_do_read_config(u32 base_addr, u8 *buf)
{
	int sub_bags = 0;
	int offset = 0;
	int subbag_len = 0;
	u8 checksum = 0;
	int i = 0;
	int ret = NO_ERR;

	ret = gtx8_i2c_read(base_addr, buf, 4);
	if (ret)
		goto err_out;

	offset = 4;
	sub_bags = buf[2];
	checksum = checksum_u8(buf, 4);
	if (checksum) {
		TS_LOG_ERR("Config head checksum err:0x%x,data:%*ph\n",
				checksum, 4, buf);
		ret = -EINVAL;
		goto err_out;
	}

	TS_LOG_INFO("config_version:%u, vub_bags:%u\n",
					buf[0], sub_bags);

	for (i = 0; i < sub_bags; i++) {
		/* read sub head [0]: sub bag num, [1]: sub bag length */
		ret = gtx8_i2c_read(base_addr + offset, buf + offset, 2);
		if (ret)
			goto err_out;

		/* read sub bag data */
		subbag_len = buf[offset + 1];

		TS_LOG_INFO("sub bag num:%u,sub bag length:%u\n",
						buf[offset], subbag_len);
		ret = gtx8_i2c_read(base_addr + offset + 2,
							buf + offset + 2,
							subbag_len + 1);
		if (ret)
			goto err_out;

		checksum = checksum_u8(buf + offset, subbag_len + 3);
		if (checksum) {
			TS_LOG_ERR("sub bag checksum err:0x%x\n", checksum);
			ret = -EINVAL;
			goto err_out;
		}

		offset += subbag_len + 3;
		TS_LOG_DEBUG("sub bag %d, data:%*ph\n",
						buf[offset], buf[offset + 1] + 3,
						buf + offset);
	}

	ret = offset;

err_out:
	return ret;
}

/**
* gtx8_read_cfg - read config.
* config_data : config pointer to save.
* config_len : config read length.
* Success return config_len, <= 0 failed.
*/
static int gtx8_read_cfg(u8 *config_data, u32 config_len)
{
	u8 cmd_flag = 0;
	u16 cmd_reg = gtx8_ts->reg.command;
	int ret = NO_ERR;
	int i = 0;
	int send_cmd_ret;
	u16 cfg_reg = gtx8_ts->reg.command + 16; /* reg offset */
	int try_times;

	if (!config_data || config_len > GTX8_CFG_MAX_LEN) {
		TS_LOG_ERR("Illegal params\n");
		return -EINVAL;
	}
	if (!cmd_reg) {
		TS_LOG_ERR("command register ERROR:0x%04x", cmd_reg);
		return -EINVAL;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		if (config_len == 0) {
			TS_LOG_ERR("%s, config len invalid\n", __func__);
			return -EINVAL;
		}
		cfg_reg = gtx8_ts->reg.cfg_addr;
	}

	/*disable doze mode*/
	ret = gtx8_set_i2c_doze_mode(false);
	if (ret) {
		TS_LOG_ERR("%s: failed disabled doze mode\n", __func__);
		goto exit;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		try_times = GTX8_RETRY_NUM_30;
	else
		try_times = GTX8_RETRY_NUM_20;
	/* wait for IC in IDLE state */
	for (i = 0; i < try_times; i++) {
		cmd_flag = 0;
		ret = gtx8_i2c_read(cmd_reg, &cmd_flag, 1);
		if (ret < 0 || cmd_flag == GTX8_CMD_COMPLETED)
			break;
		usleep_range(5000, 5200);
	}
	if (cmd_flag != GTX8_CMD_COMPLETED) {
		TS_LOG_ERR("Wait for IC ready IDEL state timeout:flag 0x%x\n", cmd_flag);
		ret = -EAGAIN;
		goto exit;
	}

	/* 0x86 read config command */
	ret = gtx8_send_cmd(GTX8_CMD_READ_CFG, 0, GTX8_NOT_NEED_SLEEP);
	if (ret) {
		TS_LOG_ERR("Failed send read config commandn");
		goto exit;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		usleep_range(3000, 3100); /* ic need delay */
	/* wait for config data ready */
	for (i = 0; i < GTX8_RETRY_NUM_20; i++) {
		cmd_flag = 0;
		ret = gtx8_i2c_read(cmd_reg, &cmd_flag, 1);
		if (ret < 0 || cmd_flag == GTX8_CMD_CFG_READY)
			break;
		if (gtx8_ts->ic_type == IC_TYPE_7382)
			usleep_range(3000, 3100);
		else
			usleep_range(5000, 5200);
	}
	if (cmd_flag != GTX8_CMD_CFG_READY) {
		TS_LOG_ERR("Wait for config data ready timeout\n");
		ret = -EAGAIN;
		goto exit;
	}

	if (config_len) {
		ret = gtx8_i2c_read(cfg_reg, config_data, config_len);
		if (ret)
			TS_LOG_ERR("Failed read config data\n");
		else
			ret = config_len;
	} else {
		ret = _gtx8_do_read_config(cfg_reg, config_data);
		if (ret < 0)
			TS_LOG_ERR("Failed read config data\n");
	}
	if (ret > 0)
		TS_LOG_INFO("success read config, len:%d\n", ret);

	/* clear command */
	send_cmd_ret = gtx8_send_cmd(GTX8_CMD_COMPLETED, 0, GTX8_NOT_NEED_SLEEP);
	if (send_cmd_ret)
		TS_LOG_ERR("Failed send read config commandn");

exit:
	/*enable doze mode*/
	if (gtx8_set_i2c_doze_mode(true))
		TS_LOG_ERR("%s: failed enabled doze mode\n", __func__);

	return ret;

}

static int gtx8_read_version(struct gtx8_ts_version *version)
{
	u8 buffer[12] = {0};
	u8 temp_buf[256] = {0};
	u8 checksum = 0;
	int ret = NO_ERR;
	u8 pid_read_len = gtx8_ts->reg.pid_len;
	u8 vid_read_len = gtx8_ts->reg.vid_len;
	u8 sensor_id_mask = gtx8_ts->reg.sensor_id_mask;

	version->valid = false;
	strncpy(gtx8_ts->dev_data->chip_name, GTX8_OF_NAME, MAX_STR_LEN - 1);

	/*check reg info valid*/
	if (!gtx8_ts->reg.pid || !gtx8_ts->reg.sensor_id || !gtx8_ts->reg.vid) {
		TS_LOG_ERR("reg is NULL, pid:0x%04x, vid:0x%04x, sensor_id:0x%04x\n",
				gtx8_ts->reg.pid, gtx8_ts->reg.vid, gtx8_ts->reg.sensor_id);
		return -EINVAL;
	}
	if (!pid_read_len || pid_read_len > PID_DATA_MAX_LEN || !vid_read_len || vid_read_len > VID_DATA_MAX_LEN) {
		TS_LOG_ERR("pid vid read len ERROR, pid_read_len:%d, vid_read_len:%d\n",
				pid_read_len, vid_read_len);
		return -EINVAL;
	}

	/*disable doze mode*/
	ret = gtx8_set_i2c_doze_mode(false);
	if (ret) {
		TS_LOG_ERR("%s: failed disabled doze mode\n", __func__);
	}

	if (gtx8_ts->support_priority_read_sensorid) {
		/* read sensor_id */
		memset(buffer, 0, sizeof(buffer));
		ret = gtx8_i2c_read(gtx8_ts->reg.sensor_id, buffer, 1);
		if (ret < 0) {
			TS_LOG_ERR("Read sensor_id failed\n");
			goto exit;
		}
		if (sensor_id_mask != 0) {
			version->sensor_id = buffer[0] & sensor_id_mask;
			TS_LOG_INFO("sensor_id_mask:0x%02x, sensor_id:0x%02x\n",
				sensor_id_mask, version->sensor_id);
		} else {
			version->sensor_id = buffer[0];
		}

	}
	/*check checksum*/
	if (gtx8_ts->ic_type != IC_TYPE_7382) {
		if (gtx8_ts->reg.version_base &&
			(gtx8_ts->reg.version_len != 0)) {
			ret = gtx8_i2c_read(gtx8_ts->reg.version_base,
				temp_buf, gtx8_ts->reg.version_len);

			if (ret < 0) {
				TS_LOG_ERR("Read version base failed, reg:0x%02x, len:%d\n",
					gtx8_ts->reg.version_base,
					gtx8_ts->reg.version_len);
				goto exit;
			}

			checksum = checksum_u8(temp_buf,
				gtx8_ts->reg.version_len);
			if (checksum) {
				TS_LOG_ERR("checksum error:0x%02x, base:0x%02x, len:%d\n",
					checksum, gtx8_ts->reg.version_base,
					gtx8_ts->reg.version_len);

			TS_LOG_ERR("%*ph\n",
				(int)(gtx8_ts->reg.version_len / 2), temp_buf);
				TS_LOG_ERR("%*ph\n",
					(int)(gtx8_ts->reg.version_len -
					gtx8_ts->reg.version_len / 2),
					&temp_buf[gtx8_ts->reg.version_len / 2]);
				ret = -EINVAL;
				goto exit;
			}
		} else {
			TS_LOG_ERR("Incomplete parameter for read pid, vid info\n");
			goto exit;
		}
	}
	/*read pid*/
	memset(buffer, 0, sizeof(buffer));
	memset(version->pid, 0, sizeof(version->pid));
	ret = gtx8_i2c_read(gtx8_ts->reg.pid, buffer, pid_read_len);
	if (ret < 0) {
		TS_LOG_ERR("Read pid failed\n");
		goto exit;
	}
	memcpy(version->pid, buffer, pid_read_len);

	/*read vid*/
	memset(buffer, 0, sizeof(buffer));
	memset(version->vid, 0, sizeof(version->vid));
	ret = gtx8_i2c_read(gtx8_ts->reg.vid, buffer, vid_read_len);
	if (ret < 0) {
		TS_LOG_ERR("Read vid failed\n");
		goto exit;
	}
	memcpy(version->vid, buffer, vid_read_len);

	if (!gtx8_ts->support_priority_read_sensorid) {
		/* read sensor_id */
		memset(buffer, 0, sizeof(buffer));
		ret = gtx8_i2c_read(gtx8_ts->reg.sensor_id, buffer, 1);
		if (ret < 0) {
			TS_LOG_ERR("Read sensor_id failed\n");
			goto exit;
		}
		if (sensor_id_mask != 0) {
			version->sensor_id = buffer[0] & sensor_id_mask;
			TS_LOG_INFO("sensor_id_mask:0x%02x, sensor_id:0x%02x\n",
					sensor_id_mask, version->sensor_id);
		} else {
			version->sensor_id = buffer[0];
		}
	}

	/*judge ic work status*/
	if (memcmp(version->pid, gtx8_ts->chip_name, GTX8_PRODUCT_ID_LEN)) {
		TS_LOG_ERR("%s: The firmware is damaged.\n", __func__);
		memset(version->pid, 0, GTX8_PRODUCT_ID_LEN + 1);
		memcpy(version->pid, gtx8_ts->chip_name, GTX8_PRODUCT_ID_LEN);
		goto exit;
	}

	version->valid = true;

	TS_LOG_INFO("PID:%s,SensorID:%d, VID:%*ph\n",
						version->pid,
						version->sensor_id,
						(int)sizeof(version->vid), version->vid);

	snprintf(gtx8_ts->dev_data->version_name, MAX_STR_LEN - 1, "0x%04X", getU32(version->vid));
exit:
	/*enable doze mode*/
	if (gtx8_set_i2c_doze_mode(true))
		TS_LOG_ERR("%s: failed enabled doze mode\n", __func__);

	return ret;
}


/**
 * gtx8_pinctrl_init - pinctrl init
 */
static int gtx8_pinctrl_init(void)
{
	int ret = NO_ERR;
	struct gtx8_ts_data *ts = gtx8_ts;

	ts->pinctrl = devm_pinctrl_get(&ts->pdev->dev);
	if (IS_ERR_OR_NULL(ts->pinctrl)) {
		TS_LOG_INFO("%s : Failed to get pinctrl[ignore]:%ld\n",
				__func__, PTR_ERR(ts->pinctrl));
		ts->pinctrl = NULL;
		/* Ignore this error to support no pinctrl platform*/
		return NO_ERR;
	}
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ts->pins_default = pinctrl_lookup_state(ts->pinctrl, "default");
	if (IS_ERR_OR_NULL(ts->pins_default)) {
		TS_LOG_ERR("%s: Pin state[default] not found\n", __func__);
		ret = PTR_ERR(ts->pins_default);
		goto exit_put;
	}

	ts->pins_suspend = pinctrl_lookup_state(ts->pinctrl, "idle");
	if (IS_ERR_OR_NULL(ts->pins_suspend)) {
		TS_LOG_ERR("%s: Pin state[idle] not found[ignore]:%ld\n",
				__func__, PTR_ERR(ts->pins_suspend));
		ts->pins_suspend = NULL;
		/* permit undefine idle state */
		ret = NO_ERR;
	}
#else
	ts->pinctrl_state_reset_high = pinctrl_lookup_state(ts->pinctrl,
		"state_rst_output1");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_reset_high)) {
		TS_LOG_ERR("%s: Pin state_rst_output1 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_reset_high);
		goto exit_put;
	}

	ts->pinctrl_state_reset_low = pinctrl_lookup_state(ts->pinctrl,
		"state_rst_output0");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_reset_low)) {
		TS_LOG_ERR("%s: Pin state_rst_output0 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_reset_low);
		goto exit_put;
	}

	ts->pinctrl_state_as_int = pinctrl_lookup_state(ts->pinctrl,
		"state_eint_as_int");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_as_int)) {
		TS_LOG_ERR("%s: Pin state_eint_as_int not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_as_int);
		goto exit_put;
	}

	ts->pinctrl_state_int_high = pinctrl_lookup_state(ts->pinctrl,
		"state_eint_output1");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_int_high)) {
		TS_LOG_ERR("%s: Pin state_eint_output1 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_int_high);
		goto exit_put;
	}

	ts->pinctrl_state_int_low = pinctrl_lookup_state(ts->pinctrl,
		"state_eint_output0");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_int_low)) {
		TS_LOG_ERR("%s: Pin state_eint_output0 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_int_low);
		goto exit_put;
	}

	ts->pinctrl_state_iovdd_low = pinctrl_lookup_state(ts->pinctrl,
		"state_iovdd_output0");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_iovdd_low)) {
		TS_LOG_ERR("%s: Pin state_iovdd_output0 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_iovdd_low);
		goto exit_put;
	}

	ts->pinctrl_state_iovdd_high = pinctrl_lookup_state(ts->pinctrl,
		"state_iovdd_output1");
	if (IS_ERR_OR_NULL(ts->pinctrl_state_iovdd_high)) {
		TS_LOG_ERR("%s: Pin state_iovdd_output1 not found\n", __func__);
		ret = PTR_ERR(ts->pinctrl_state_iovdd_high);
		goto exit_put;
	}
#endif
	return ret;
exit_put:
	devm_pinctrl_put(ts->pinctrl);
	ts->pinctrl = NULL;
	ts->pins_suspend = NULL;
	ts->pins_default = NULL;
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ts->pinctrl_state_reset_high = NULL;
	ts->pinctrl_state_int_high = NULL;
	ts->pinctrl_state_int_low = NULL;
	ts->pinctrl_state_reset_low = NULL;
	ts->pinctrl_state_as_int = NULL;
	ts->pinctrl_state_iovdd_low = NULL;
	ts->pinctrl_state_iovdd_high = NULL;
#endif
	return ret;
}


static void gtx8_pinctrl_release(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts->pinctrl)
		devm_pinctrl_put(ts->pinctrl);
	ts->pinctrl = NULL;
	ts->pins_gesture = NULL;
	ts->pins_suspend = NULL;
	ts->pins_default = NULL;
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ts->pinctrl_state_reset_high = NULL;
	ts->pinctrl_state_int_high = NULL;
	ts->pinctrl_state_int_low = NULL;
	ts->pinctrl_state_reset_low = NULL;
	ts->pinctrl_state_as_int = NULL;
	ts->pinctrl_state_iovdd_low = NULL;
	ts->pinctrl_state_iovdd_high = NULL;
#endif
}

/**
 * gtx8_pinctrl_select_normal - set normal pin state
 *  Irq pin *must* be set to *pull-up* state.
 */
static void gtx8_pinctrl_select_normal(void)
{
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int ret = 0;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts->pinctrl && ts->pins_default) {
		ret = pinctrl_select_state(ts->pinctrl, ts->pins_default);
		if (ret < 0)
			TS_LOG_ERR("%s:Set normal pin state error:%d\n", __func__, ret);
	}
#endif
	return;
}

/**
 * gtx8_pinctrl_select_suspend - set suspend pin state
 *  Irq pin *must* be set to *pull-up* state.
 */
static void gtx8_pinctrl_select_suspend(void)
{
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int ret = 0;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts->pinctrl && ts->pins_suspend) {
		ret = pinctrl_select_state(ts->pinctrl, ts->pins_suspend);
		if (ret < 0)
			TS_LOG_ERR("%s:Set suspend pin state error:%d\n", __func__, ret);
	}
#endif
	return;
}

/**
 * gtx8_pinctrl_select_gesture - set gesture pin state
 *  Irq pin *must* be set to *pull-up* state.
 */
static int gtx8_pinctrl_select_gesture(void)
{
	int ret = 0;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts->pinctrl && ts->pins_gesture) {
		ret = pinctrl_select_state(ts->pinctrl, ts->pins_gesture);
		if (ret < 0)
			TS_LOG_ERR("%s:Set gesture pin state error:%d\n", __func__, ret);
	}

	return ret;
}

static void gtx8_power_on_gpio_set(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	unsigned int ret = 0;
	gtx8_pinctrl_select_normal();
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ret = gpio_direction_input(ts->dev_data->ts_platform_data->irq_gpio);
	ret |= gpio_direction_output(ts->dev_data->ts_platform_data->reset_gpio, 1);
#else
	ret = pinctrl_select_state(ts->pinctrl, ts->pinctrl_state_as_int);
	ret |= pinctrl_select_state(ts->pinctrl, ts->pinctrl_state_reset_high);
#endif
	if(ret)
		TS_LOG_INFO("%s:error\n",__func__);
}

static void gtx8_power_off_gpio_set(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	unsigned int ret = 0;
	gtx8_pinctrl_select_suspend();
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ret = gpio_direction_input(ts->dev_data->ts_platform_data->irq_gpio);
	ret |= gpio_direction_output(ts->dev_data->ts_platform_data->reset_gpio, 0);
#else
	ret = pinctrl_select_state(ts->pinctrl, ts->pinctrl_state_as_int);
	ret |= pinctrl_select_state(ts->pinctrl, ts->pinctrl_state_reset_low);
	ret |= pinctrl_select_state(ts->pinctrl, ts->pinctrl_state_iovdd_low);
#endif
	if(ret)
		TS_LOG_INFO("%s:error\n",__func__);
}

/**
 * gtx8_power_switch - power switch
 * @on: 1-switch on, 0-switch off
 * return: 0-succeed, -1-faileds
 */
 static int gtx8_power_init(void)
{
	int ret  = 0;
	ret = ts_kit_power_supply_get(TS_KIT_IOVDD);
	if(ret)
		return ret;
	ret = ts_kit_power_supply_get(TS_KIT_VCC);
	return ret;
}

static int gtx8_power_release(void)
{
	int ret  = 0;
	ret = ts_kit_power_supply_put(TS_KIT_IOVDD);
	if(ret)
		return ret;
	ret = ts_kit_power_supply_put(TS_KIT_VCC);
	return ret;
}


static int gtx8_power_on(void)
{
	int ret  =0; 
	TS_LOG_INFO("gtx8_power_on called\n");
	ret = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_ON, 0);
	if(ret)
		return ret;
	udelay(5);
	ret = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_ON, 15);
	if(ret)
		return ret;
	gtx8_power_on_gpio_set();
	return 0;
}
static int  gtx8_power_off(void)
{
	int ret  =0; 
	ret = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_OFF, 0);
	if(ret)
		return ret;
	udelay(2);
	ret = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_OFF, 0);
	if(ret)
		return ret;
	udelay(2);
	gtx8_power_off_gpio_set();
	return 0;
}
static int gtx8_power_switch(int on)
{
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int reset_gpio = 0;
#endif
	int ret = 0;
	struct gtx8_ts_data *ts = gtx8_ts;

#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	reset_gpio = ts->dev_data->ts_platform_data->reset_gpio;
	ret = gpio_direction_output(reset_gpio, 0);
#else
	ret = pinctrl_select_state(ts->pinctrl,
		ts->pinctrl_state_iovdd_high);
	ret |= pinctrl_select_state(ts->pinctrl,
		ts->pinctrl_state_reset_low);
#endif
	if(ret)
		TS_LOG_INFO("%s:error./\n",__func__);
	udelay(100);

	if (on) {
		ret = gtx8_power_on();
		if (ts->ic_type == IC_TYPE_7382)
			ts->poweron_flag = true;
	} else {
		ret = gtx8_power_off();
	}
	return 0 ;

}

static void gtx8_reg_init(struct gtx8_ts_data *ts)
{
	ts->reg.cfg_addr = GTP_REG_CFG_ADDR;
	ts->reg.command = GTP_REG_CMD;
	ts->reg.coor = GTP_REG_COOR;
	ts->reg.gesture = GTP_REG_GESTURE;
	ts->reg.esd = GTP_REG_ESD_TICK_W;
	ts->reg.pid = GTP_REG_PID;
	ts->reg.pid_len = GTP_PID_LEN;
	ts->reg.sensor_id = GTP_REG_SENSOR_ID;
	ts->reg.sensor_id_mask = GTP_SENSOR_ID_MASK;
	ts->reg.version_base = GTP_REG_VERSION_BASE;
	ts->reg.version_len = GTP_VERSION_LEN;
	ts->reg.vid = GTP_REG_VID;
	ts->reg.vid_len = GTP_VID_LEN;
	ts->reg.fw_request = GTP_REG_FW_REQUEST;
	if (ts->ic_type == IC_TYPE_7382) {
		ts->reg.cfg_addr = GTP_REG_CFG_ADDR_GT7382;
		ts->reg.esd = GTP_REG_ESD_TICK_W_GT7382;
		ts->reg.command = GTP_REG_CMD_GT7382;
		ts->reg.coor = GTP_REG_COOR_GT7382;
		ts->reg.gesture = GTP_REG_GESTURE_GT7382;
		ts->reg.pid = GTP_REG_PID_GT7382;
		ts->reg.vid = GTP_REG_VID_GT7382;
		ts->reg.sensor_id = GTP_REG_SENSOR_ID_GT7382;
	}
	return ;
}



/**
 * gtx8_switch_workmode - Switch working mode.
 * @workmode: GTP_CMD_SLEEP - Sleep mode
 *			  GESTURE_MODE - gesture mode
 * Returns  0--success,non-0--fail.
 */
static int gtx8_switch_workmode(int workmode)
{
	s32 retry = 0;
	u8 cmd = 0;

	switch (workmode) {
	case SLEEP_MODE:
		cmd = GTX8_CMD_SLEEP;
		break;
	case GESTURE_MODE:
		cmd = GTX8_CMD_GESTURE;
		break;
	case GESTURE_EXIT_MODE:
		cmd = GTX8_CMD_GESTURE_EXIT;
		break;
	default:
		TS_LOG_ERR("%s: no supported workmode\n", __func__);
		return -EINVAL;
	}

	TS_LOG_DEBUG("%s: Switch working mode[%02X]\n", __func__, cmd);
	while (retry++ < GTX8_RETRY_NUM_3) {
		if (!gtx8_send_cmd(cmd, 0, GTX8_NOT_NEED_SLEEP))
			return NO_ERR;

		TS_LOG_ERR("%s: send_cmd failed, retry = %d\n", __func__, retry);
	}

	TS_LOG_ERR("%s: Failed to switch working mode\n", __func__);

	return -EINVAL;
}

/**
 * gt8x_feature_switch - Switch touch feature.
 * @ts: gtx8 ts data
 * @fea: gtx8 touch feature
 * @on: SWITCH_ON, SWITCH_OFF
 * Returns  0--success,non-0--fail.
 */
static int gtx8_feature_switch(struct gtx8_ts_data *ts,
		enum gtx8_ts_feature fea, int on)
{
	struct ts_feature_info *info = NULL;
	struct gtx8_ts_config *config = NULL;
	int ret = NO_ERR;
	if (!ts ||!ts->dev_data ||!ts->dev_data->ts_platform_data) {
		TS_LOG_ERR("%s:invalid param\n", __func__);
		return -EINVAL;
	}
	info = &ts->dev_data->ts_platform_data->feature_info;
	if (on == SWITCH_ON) {
		switch (fea) {
		case TS_FEATURE_NONE:
			config = &ts->normal_cfg;
			break;
		case TS_FEATURE_GLOVE:
			config = &ts->glove_cfg;
			break;
		case TS_FEATURE_HOLSTER:
			config = &ts->holster_cfg;
			break;
		case TS_FEATURE_GAME:
			config = &ts->game_cfg;
			break;
		case TS_FEATURE_CHARGER:
				config = &ts->normal_noise_cfg;
			break;
		default:
			TS_LOG_ERR("%s:invalid feature type\n", __func__);
			return -EINVAL;
		}
	} else if (on == SWITCH_OFF) {
		if (info->holster_info.holster_switch)
			config = &ts->holster_cfg;
		else if (info->glove_info.glove_switch)
			config = &ts->glove_cfg;
		else if (info->charger_info.charger_switch)
			config = &ts->normal_noise_cfg;
		else
			config = &ts->normal_cfg;
	} else {
		TS_LOG_ERR("%s:invalid switch status\n", __func__);
		return -EINVAL;
	}

	ts->noise_env = false;
	ret = gtx8_send_cfg(config);
	if (ret)
		TS_LOG_ERR("%s:send_cfg failed\n", __func__);

	return ret;
}
static int gtx8_wakeup_gesture_enable_switch(struct
						  ts_wakeup_gesture_enable_info *info);
/**
 * gtx8_feature_resume - firmware feature resume
 * @ts: pointer to gtx8_ts_data
 * return 0 ok, others error
 */
static int gtx8_feature_resume(struct gtx8_ts_data *ts)
{
	struct ts_feature_info *info = NULL;
	struct gtx8_ts_config *config = NULL;
	int ret = NO_ERR;

	if (!ts ||!ts->dev_data ||!ts->dev_data->ts_platform_data) {
		TS_LOG_ERR("%s:invalid param\n", __func__);
		return -EINVAL;
	}
	info = &ts->dev_data->ts_platform_data->feature_info;
	TS_LOG_INFO("holster_supported:%d,holster_switch_staus:%d,glove_supported:%d,glove_switch_status:%d\n",
		info->holster_info.holster_supported, info->holster_info.holster_switch,
		info->glove_info.glove_supported, info->glove_info.glove_switch);

	if (info->holster_info.holster_supported
		&& info->holster_info.holster_switch){
		config = &ts->holster_cfg;
	}
	else if (info->glove_info.glove_supported
		&& info->glove_info.glove_switch) {
		if (ts->noise_env)
			config = &ts->glove_noise_cfg;
		else
			config = &ts->glove_cfg;
	} else {
		if (ts->noise_env ||(info->charger_info.charger_switch))
			config = &ts->normal_noise_cfg;
		else
			config = &ts->normal_cfg;
	}
	if (config) {
		ret = gtx8_send_cfg(config);
		if (ret < 0)
			TS_LOG_ERR("%s: send_cfg fail\n", __func__);
	} else {
		TS_LOG_ERR("%s: config parm is null!\n", __func__);
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		goto skip_send_cmd;
	//if need to send cmd after send config,it should delay 50ms befor send cmd.
	ret = gtx8_wakeup_gesture_enable_switch(&(info->wakeup_gesture_enable_info));//have delay 50ms before send cmd.
skip_send_cmd:
	if(info->charger_info.charger_switch){
		msleep(SEND_CMD_AFTER_CMD_DELAY);//if need to send cmd after other cmd.it should delay 20ms befor send new cmd.
		TS_LOG_INFO("resume to send  charger cmd:%d\n",GTX8_CMD_CHARGER_ON);
		if (gtx8_send_cmd(GTX8_CMD_CHARGER_ON, 0, GTX8_NOT_NEED_SLEEP)) {
			TS_LOG_ERR("Send charge cmd failed.\n");
			ret |= -EINVAL;
		}
	}

	return ret;
}

/**
 * gtx8_ts_esd_init - initialize esd protection
 */
int gtx8_ts_esd_init(struct gtx8_ts_data *ts)
{
	u8 data = 0xaa;
	int ret = NO_ERR;

	if (ts->reg.esd != 0) {
		if (ts->ic_type == IC_TYPE_7382) {
			/* init static esd */
			ret = ts->ops.write_trans(0x8043, &data, 1);
			if (ret < 0)
				TS_LOG_ERR("static ESD init failed\n");
			goto skip_dynamic_esd_init;
		}
		/*init dynamic esd*/
		ret = ts->ops.write_trans(ts->reg.esd, &data, 1);
		if (ret < 0)
			TS_LOG_ERR("dynamic ESD init ERROR, i2c write failed\n");
	}
skip_dynamic_esd_init:
	if (ts->ic_type == IC_TYPE_7382)
		return ret;
	else
		return NO_ERR;
}

static void gtx8_parse_ic_type(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	struct device_node *device = ts->pdev->dev.of_node;
	int value = 0;
	int ret;

	ret = of_property_read_u32(device, GTP_IC_TYPE, &value);
	if (ret) {
		TS_LOG_ERR("%s:ic_type read failed, Use default value\n",
			__func__);
		value = IC_TYPE_9886;
	}
	ts->ic_type = value;
}

static void gtx8_ts_parse_recovery_res(struct device_node *device)
{
	int ret;
	struct device_node *np = device;

	if (device == NULL) {
		TS_LOG_ERR("%s, param invalid\n", __func__);
		return;
	}
	/*
	 * convert ic resolution x[0,2815] y[0,5119] to real resolution,
	 * fix inaccurate touch in recovery mode,
	 * there is no coordinate conversion in recovery mode.
	 */
	if (get_into_recovery_flag_adapter()) {
		gtx8_ts->in_recovery_mode = GTX8_USE_IN_RECOVERY_MODE;
		TS_LOG_INFO("Now driver in recovery mode\n");
	} else {
		gtx8_ts->in_recovery_mode = GTX8_USE_NOT_IN_RECOVERY_MODE;
	}

	ret = of_property_read_u32(np, "use_ic_res", &gtx8_ts->use_ic_res);
	if (ret) {
		TS_LOG_INFO("%s:get device use_ic_res fail, use default, ret=%d\n",
			__func__, ret);
		gtx8_ts->use_ic_res = GTX8_USE_LCD_RESOLUTION;
	}

	ret = of_property_read_u32(np, "x_max_rc", &gtx8_ts->x_max_rc);
	if (ret) {
		TS_LOG_INFO("%s:get device x_max_rc fail,use default, ret=%d\n",
			__func__, ret);
		gtx8_ts->x_max_rc = 0;
	}

	ret = of_property_read_u32(np, "y_max_rc", &gtx8_ts->y_max_rc);
	if (ret) {
		TS_LOG_INFO("%s:get device y_max_rc fail,use default, ret=%d\n",
			__func__, ret);
		gtx8_ts->y_max_rc = 0;
	}

	TS_LOG_INFO("%s:%s=%d, %s=%d, %s=%d, %s=%d\n", __func__,
		"use_ic_res", gtx8_ts->use_ic_res,
		"x_max_rc", gtx8_ts->x_max_rc,
		"y_max_rc", gtx8_ts->y_max_rc,
		"in_recovery_mode", gtx8_ts->in_recovery_mode);
}

/**
 * gtx8_parse_dts - parse gtx8 private properties
 */
static int gtx8_parse_dts(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	struct device_node *device = ts->pdev->dev.of_node;
	struct ts_kit_device_data *chip_data = ts->dev_data;
	char *tmp_buff = NULL;
	int value = 0;
	int ret = NO_ERR;

	ret = of_property_read_u32(device, GTP_X_MAX_MT, &ts->max_x);
	if (ret) {
		TS_LOG_ERR("%s: Get x_max_mt failed\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	ret = of_property_read_u32(device, GTP_Y_MAX_MT, &ts->max_y);
	if (ret) {
		TS_LOG_ERR("%s: Get y_max_mt failed\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	ret = of_property_read_string(device, GTP_CHIP_NAME, (const char **)&tmp_buff);
	if (ret || tmp_buff == NULL) {
		TS_LOG_ERR("%s: chip_name read failed\n", __func__);
		ret = -EINVAL;
		goto err;
	}
	strncpy(ts->chip_name, tmp_buff, GTX8_PRODUCT_ID_LEN);

	ret = of_property_read_string(device, GTP_MODULE_VENDOR, (const char **)&tmp_buff);
	if (ret || tmp_buff == NULL) {
		TS_LOG_ERR("%s: default module name read failed\n", __func__);
		ret = -EINVAL;
		goto err;
	}
	strncpy(ts->dev_data->module_name, tmp_buff, MAX_STR_LEN - 1);

	ret = of_property_read_u32(device, GTP_IC_TYPE, &value);
	if (ret) {
		TS_LOG_ERR("%s:ic_type read failed, Use default value: 9PT\n", __func__);
		value = IC_TYPE_9886;
	}
	ts->ic_type = value;

	ret = of_property_read_u32(device, GTP_ROI_DATA_ADDR, &value);
	if (ret) {
		TS_LOG_INFO("%s:roi_data address read failed, Use default value.\n", __func__);
		value = 0;
	}
	ts->gtx8_roi_data_add = value;
	TS_LOG_INFO("%s:roi_data address used is: [%x]\n", __func__, ts->gtx8_roi_data_add);

	ret = of_property_read_u32(device, GTP_ROI_FW_SUPPORTED, &value);
	if (ret) {
		TS_LOG_INFO("%s: gtx8_roi_fw_supported use default value\n",
			__func__);
		value = GTX8_NOT_EXIST;
	}
	ts->gtx8_roi_fw_supported = value;

	ret = of_property_read_u32(device, GTP_EASY_WAKE_SUPPORTED, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: easy_wakeup_supported use default value\n", __func__);
	}
	ts->easy_wakeup_supported = (u8)value;

	if (ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported) {
		ret = of_property_read_u32_index(device, GTP_ROI_DATA_SIZE, 0,
				&ts->roi.roi_rows);
		if (ret) {
			TS_LOG_ERR("%s : Get ROI rows failed\n", __func__);
			ret = -EINVAL;
			goto err;
		}
		ret = of_property_read_u32_index(device, GTP_ROI_DATA_SIZE, 1,
				&ts->roi.roi_cols);
		if (ret) {
			TS_LOG_ERR("%s : Get ROI cols failed\n", __func__);
			ret = -EINVAL;
			goto err;
		}
		TS_LOG_INFO("%s : roi_rows = [%d], roi_cols = [%d]\n", __func__,
						ts->roi.roi_rows, ts->roi.roi_cols);
	}

	ret = of_property_read_u32(device, GTP_STATIC_PID_SUPPORT, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: gtx8_static_pid_support use default value\n", __func__);
	}
	ts->gtx8_static_pid_support = value;

	ret = of_property_read_u32(device, GTP_TOOL_SUPPORT, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: tools_support use default value\n", __func__);
	}
	ts->tools_support = value;

	ret = of_property_read_u32(device, GTX8_SUPPORT_TP_COLOR, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: support_get_tp_color use default value\n",
			__func__);
	}
	ts->support_get_tp_color = value;
	TS_LOG_INFO("%s, support_get_tp_color=%d\n", __func__,
		ts->support_get_tp_color);
	ret = of_property_read_u32(device, GTX8_SUPPORT_READ_PROJECTID, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: support_read_projectid use default value\n",
			__func__);
	}
	ts->support_read_projectid = value;
	TS_LOG_INFO("%s, support_read_projectid=%d\n", __func__,
		ts->support_read_projectid);
	ret = of_property_read_u32(device, GTX8_WAKE_LOCK_SUSPEND, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: support_wake_lock_suspend use default value\n",
			__func__);
	}
	ts->support_wake_lock_suspend = value;
	TS_LOG_INFO("%s, support_wake_lock_suspend=%d\n", __func__,
		ts->support_wake_lock_suspend);
	ret = of_property_read_u32(device, GTX8_FILENAME_CONTAIN_PROJECTID,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: support_cfg_named_with_projectid use default value\n",
			__func__);
	}
	ts->support_filename_contain_projectid = value;
	TS_LOG_INFO("%s, support_filename_contain_projectid=%d\n", __func__,
		ts->support_filename_contain_projectid);
	ret = of_property_read_u32(device, GTX8_CHECKED_ESD_RESET_CHIP, &value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: support_checked_esd_reset_chip use default value\n",
			__func__);
	}
	ts->support_checked_esd_reset_chip = value;
	TS_LOG_INFO("%s, support_checked_esd_reset_chip=%d\n", __func__,
		ts->support_checked_esd_reset_chip);
	ret = of_property_read_u32(device, GTX8_DELETE_INSIGNIFICANT_INFO,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_INFO("%s: delete_insignificant_chip_info use default value\n",
			__func__);
	}
	ts->delete_insignificant_chip_info = value;
	TS_LOG_INFO("%s, delete_insignificant_chip_info=%d\n", __func__,
		ts->delete_insignificant_chip_info);
	ret = of_property_read_u32(device, GTX8_SUPPORT_PRIORITY_READ_SENSORID,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_priority_read_sensorid use default value\n",
			__func__);
	}
	ts->support_priority_read_sensorid = value;
	TS_LOG_INFO("%s, support_priority_read_sensorid=%d\n", __func__,
		ts->support_priority_read_sensorid);
	ret = of_property_read_u32(device, GTX8_DOZE_FAILED_NOT_SEND_CFG, &value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_doze_failed_not_send_cfg use default value\n",
			__func__);
	}
	ts->support_doze_failed_not_send_cfg = value;
	TS_LOG_INFO("%s, support_doze_failed_not_send_cfg=%d\n", __func__,
		ts->support_doze_failed_not_send_cfg);
	ret = of_property_read_u32(device, GTX8_SUPPORT_READ_MORE_DEUGMSG,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_read_more_debug_msg use default value\n",
			__func__);
	}
	ts->support_read_more_debug_msg = value;
	TS_LOG_INFO("%s, support_read_more_debug_msg=%d\n", __func__,
		ts->support_read_more_debug_msg);
	ret = of_property_read_u32(device, GTX8_FILENAME_CONTAIN_LCD_MOUDULE,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_filename_contain_lcd_module use default value\n",
			__func__);
	}
	ts->support_filename_contain_lcd_module = value;
	TS_LOG_INFO("%s, support_filename_contain_lcd_module=%d\n", __func__,
		ts->support_filename_contain_lcd_module);
	ret = of_property_read_u32(device, GTX8_SUPPORT_IC_USE_MAX_RESOLUTION,
			&value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_filename_contain_lcd_module use default value\n",
			__func__);
	}
	ts->support_ic_use_max_resolution = value;
	TS_LOG_INFO("%s, support_ic_use_max_resolution=%d\n", __func__,
		ts->support_ic_use_max_resolution);
	ret = of_property_read_u32(device,
		GTX8_SUPPORT_READ_GESTURE_WITHOUT_CHECKSUM, &value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_read_gesture_without_checksum use default value\n",
			__func__);
	}
	ts->support_read_gesture_without_checksum = value;
	TS_LOG_INFO("%s, support_read_gesture_without_checksum=%d\n", __func__,
		ts->support_read_gesture_without_checksum);
	ret = of_property_read_u32(device, GTX8_SUPPORT_PEN_USE_MAX_RESOLUTION,
		&value);
	if (ret) {
		value = 0;
		TS_LOG_ERR("%s: support_pen_use_max_resolution use default value\n",
			__func__);
	}
	ts->support_pen_use_max_resolution = (u32)value;
	TS_LOG_INFO("%s: support_pen_use_max_resolution=%d\n", __func__,
		ts->support_pen_use_max_resolution);
	ret = of_property_read_u32(device, GTX8_SUPPORT_RETRY_READ_GESTURE,
		&ts->support_retry_read_gesture);
	if (ret)
		ts->support_retry_read_gesture = 0;
	TS_LOG_INFO("%s: support_retry_read_gesture=%d\n", __func__,
		ts->support_retry_read_gesture);
	TS_LOG_INFO("%s: irq_config=%d,algo_id=%d,project_id=%s,config_flag=%d,roi_supported=%d,"
		"holster_supported=%d,glove_supported=%d, easy_wakeup_supported=%d ,"
		"tools_support=%d,ic_type=%d,chip_name=%s\n" , __func__,
		chip_data->irq_config, chip_data->algo_id, ts->project_id, ts->config_flag,
		chip_data->ts_platform_data->feature_info.roi_info.roi_supported,
		chip_data->ts_platform_data->feature_info.holster_info.holster_supported,
		chip_data->ts_platform_data->feature_info.glove_info.glove_supported,
		ts->easy_wakeup_supported,  ts->tools_support, ts->ic_type, ts->chip_name);
	if ((ts->support_ic_use_max_resolution) ||
		(ts->support_pen_use_max_resolution))
		gtx8_ts_parse_recovery_res(device);

	TS_LOG_INFO("%s: gtx8_parse_dts end\n", __func__);
	ret = NO_ERR;
err:
	return ret;
}

static int gtx8_parse_specific_dts(void)
{
	struct device_node *device = NULL;
	struct gtx8_ts_data *ts = gtx8_ts;
	char sensor_id_str[20] = {0};
	char *tmp_buff = NULL;
	int ret = NO_ERR;

	snprintf(sensor_id_str, sizeof(sensor_id_str), "gtx8-sensorid-%u", ts->hw_info.sensor_id);
	TS_LOG_INFO("%s: Parse specific dts:%s\n", __func__, sensor_id_str);

	device = of_find_compatible_node(ts->pdev->dev.of_node, NULL, sensor_id_str);
	if (!device) {
		TS_LOG_INFO("%s : No chip specific dts: %s, need to parse\n",
			__func__, sensor_id_str);
		return -EINVAL;
	}

	ret = of_property_read_string(device, GTP_MODULE_VENDOR, (const char **)&tmp_buff);
	if (ret || tmp_buff == NULL) {
		TS_LOG_ERR("%s: vendor_name read failed\n", __func__);
		return -EINVAL;
	}
	memset(ts->dev_data->module_name, 0, MAX_STR_LEN);
	strncpy(ts->dev_data->module_name, tmp_buff, MAX_STR_LEN - 1);

	ret = of_property_read_string(device, GTP_PROJECT_ID, (const char**)&tmp_buff);
	if (ret || tmp_buff == NULL)
	{
		TS_LOG_ERR("%s: project_id read failed\n", __func__);
		return -EINVAL;
	}
	memset(ts->project_id, 0, MAX_STR_LEN);
	strncpy(ts->project_id, tmp_buff, MAX_STR_LEN - 1);

	TS_LOG_INFO("%s: vendor_name=%s,project_id=%s\n",__func__,  ts->dev_data->module_name,ts->project_id);

	return NO_ERR;
}

/**
 * gtx8_ts_roi_init - initialize roi feature
 * @roi: roi data structure
 * return 0 OK, < 0 fail
 */
static int gtx8_ts_roi_init(struct gtx8_ts_roi *roi)
{
	unsigned int roi_bytes;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (!ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported
			|| !roi ) {
		TS_LOG_ERR("%s: roi is not support or invalid parameter\n", __func__);
		return -EINVAL;
	}

	if (!roi->roi_rows || !roi->roi_cols) {
		TS_LOG_ERR("%s: Invalid roi config,rows:%d,cols:%d\n",
				__func__, roi->roi_rows, roi->roi_cols);
		return -EINVAL;
	}

	mutex_init(&roi->mutex);

	roi_bytes = (roi->roi_rows * roi->roi_cols + 1) * sizeof(*roi->rawdata);
	roi->rawdata = kzalloc(roi_bytes + ROI_HEAD_LEN, GFP_KERNEL);
	if (!roi->rawdata) {
		TS_LOG_ERR("%s: Failed to alloc memory for roi\n", __func__);
		return -ENOMEM;
	}

	TS_LOG_INFO("%s: ROI init,rows:%d,cols:%d\n",
				__func__, roi->roi_rows, roi->roi_cols);

	return NO_ERR;
}

static int gtx8_parse_cfg_data(const struct firmware *cfg_bin,
				char *cfg_type, u8 *cfg, int *cfg_len, u8 sid)
{
	int i = 0, config_status = 0, one_cfg_count = 0;

	u8 bin_group_num = 0, bin_cfg_num = 0;
	u16 cfg_checksum = 0, checksum = 0;
	u8 sid_is_exist = GTX8_NOT_EXIST;

	if (getU32(cfg_bin->data) + BIN_CFG_START_LOCAL != cfg_bin->size) {
		TS_LOG_ERR("%s:Bad firmware!\n", __func__);
		goto exit;
	}

	/* check firmware's checksum */
	cfg_checksum = getU16(&cfg_bin->data[4]);

	for (i = BIN_CFG_START_LOCAL; i < (cfg_bin->size) ; i++)
		checksum += cfg_bin->data[i];

	if ((checksum) != cfg_checksum) {
		TS_LOG_ERR("%s:Bad firmware!(checksum: 0x%04X, header define: 0x%04X)\n",
			__func__, checksum, cfg_checksum);
		goto exit;
	}
	/* check head end  */

	bin_group_num = cfg_bin->data[MODULE_NUM];
	bin_cfg_num = cfg_bin->data[CFG_NUM];
	TS_LOG_INFO("%s:bin_group_num = %d, bin_cfg_num = %d\n",
						__func__, bin_group_num , bin_cfg_num);

	if (!strncmp(cfg_type, GTX8_TEST_CONFIG, strlen(GTX8_TEST_CONFIG)))
		config_status = 0;
	else if (!strncmp(cfg_type, GTX8_NORMAL_CONFIG, strlen(GTX8_NORMAL_CONFIG)))
		config_status = 1;
	else if (!strncmp(cfg_type, GTX8_NORMAL_NOISE_CONFIG, strlen(GTX8_NORMAL_NOISE_CONFIG)))
		config_status = 2;
	else if (!strncmp(cfg_type, GTX8_GLOVE_CONFIG, strlen(GTX8_GLOVE_CONFIG)))
		config_status = 3;
	else if (!strncmp(cfg_type, GTX8_GLOVE_NOISE_CONFIG, strlen(GTX8_GLOVE_NOISE_CONFIG)))
		config_status = 4;
	else if (!strncmp(cfg_type, GTX8_HOLSTER_CONFIG, strlen(GTX8_HOLSTER_CONFIG)))
		config_status = 5;
	else if (!strncmp(cfg_type, GTX8_HOLSTER_NOISE_CONFIG, strlen(GTX8_HOLSTER_NOISE_CONFIG)))
		config_status = 6;
	else if (!strncmp(cfg_type, GTX8_NOISE_TEST_CONFIG, strlen(GTX8_NOISE_TEST_CONFIG)))
		config_status = 7;
	else if (!strncmp(cfg_type, GTX8_GAME_CONFIG, strlen(GTX8_GAME_CONFIG)))
		config_status = 8;
	else if (!strncmp(cfg_type, GTX8_GAME_NOISE_CONFIG, strlen(GTX8_GAME_NOISE_CONFIG)))
		config_status = 9;
	else {
		TS_LOG_ERR("%s: invalid config text field\n", __func__);
		goto exit;
	}

	for (i = 0 ; i < bin_group_num*bin_cfg_num; i++) {
		/* find cfg's sid in cfg.bin */
		if (sid == (cfg_bin->data[CFG_HEAD_BYTES + i*CFG_INFO_BLOCK_BYTES])) {
			sid_is_exist = GTX8_EXIST;
			if (config_status == (cfg_bin->data[CFG_HEAD_BYTES+1 + i*CFG_INFO_BLOCK_BYTES])) {
				one_cfg_count = getU16(&cfg_bin->data[CFG_HEAD_BYTES + 2 + i*CFG_INFO_BLOCK_BYTES]);
				memcpy(cfg, &cfg_bin->data[CFG_HEAD_BYTES + bin_group_num*bin_cfg_num*
						CFG_INFO_BLOCK_BYTES + i*one_cfg_count], one_cfg_count);
				*cfg_len = one_cfg_count;
				TS_LOG_INFO("%s:one_cfg_count = %d, cfg_data1 = 0x%02x, cfg_data2 = 0x%02x\n",
							__func__, one_cfg_count , cfg[0], cfg[1]);
				break;
			}
		}
	}
#if defined (CONFIG_HUAWEI_DSM)
	if (GTX8_NOT_EXIST == sid_is_exist) {
		if (!dsm_client_ocuppy(ts_dclient)) {
			TS_LOG_ERR("%s: DSM_TP_FWUPDATE_ERROR_NO %d\n",
				__func__, DSM_TP_FWUPDATE_ERROR_NO);
			dsm_client_record(ts_dclient,
				"config update result: sensor_id  is not exist\n");
			dsm_client_notify(ts_dclient, DSM_TP_FWUPDATE_ERROR_NO);
		}
	}
#endif
	if (i >= bin_group_num*bin_cfg_num) {
		TS_LOG_ERR("%s:(not find config ,config_status: %d)\n", __func__, config_status);
		goto exit;
	}

	return NO_ERR;
exit:
	return RESULT_ERR;
}

static int gtx8_get_cfg_data(const struct firmware *cfg_bin , char *config_name, struct gtx8_ts_config *config)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	u8 *cfg_data = NULL;
	int cfg_len = 0;
	int ret = NO_ERR;

	cfg_data = kzalloc(GOODIX_CFG_MAX_SIZE, GFP_KERNEL);
	if (cfg_data == NULL) {
		TS_LOG_ERR("Memory allco err\n");
		goto exit;
	}

	config->initialized = false;
	mutex_init(&config->lock);
	config->reg_base = GTP_REG_CFG_ADDR;

	/* parse config data */
	ret = gtx8_parse_cfg_data(cfg_bin, config_name, cfg_data,
				&cfg_len, ts->hw_info.sensor_id);
	if (ret < 0) {
		TS_LOG_ERR("%s: parse %s data failed\n", __func__, config_name);
		ret = -EINVAL;
		goto exit;
	}

	TS_LOG_INFO("%s: %s  version:%d , size:%d\n",
			__func__, config_name, cfg_data[0], cfg_len);

	if ((cfg_len <= 0) || (cfg_len > GOODIX_CFG_MAX_SIZE)) {
		TS_LOG_ERR("%s: cfg_len invalid\n", __func__);
		ret = -EINVAL;
		goto exit;
	}
	memcpy(config->data, cfg_data, cfg_len);
	config->length = cfg_len;

	strncpy(config->name, config_name, MAX_STR_LEN);
	config->initialized = true;

exit:
	if (cfg_data) {
		kfree(cfg_data);
		cfg_data = NULL;
	}

	return ret;
}

static int gtx8_get_cfg_parms(struct gtx8_ts_data *ts, const char *filename)
{
	int ret = 0;
	const struct firmware *cfg_bin = NULL;

	TS_LOG_INFO("%s: Enter\n", __func__);
	ret = request_firmware(&cfg_bin, filename, &gtx8_ts->pdev->dev);
	if (ret < 0) {
		TS_LOG_ERR("%s:Request config firmware failed - %s (%d)\n",
					__func__, filename, ret);
#if defined(CONFIG_HUAWEI_DSM)
		ts_dmd_report(DSM_TP_FWUPDATE_ERROR_NO,
			"request config file failed. project id:%s, sensor id:0x%x\n",
			ts->project_id, ts->hw_info.sensor_id);
#endif
		goto exit;
	}

	if (cfg_bin->data == NULL) {
		TS_LOG_ERR("%s:Bad firmware!(config firmware size: )\n", __func__);
		goto exit;
	}

	/* parse normal config data */
	ret = gtx8_get_cfg_data(cfg_bin, GTX8_NORMAL_CONFIG, &ts->normal_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse normal_config data:%d\n", __func__, ret);

	ret = gtx8_get_cfg_data(cfg_bin, GTX8_TEST_CONFIG, &ts->test_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse test_config data:%d\n", __func__, ret);

	/* parse normal noise config data */
	ret = gtx8_get_cfg_data(cfg_bin, GTX8_NORMAL_NOISE_CONFIG, &ts->normal_noise_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse normal_noise_config data\n", __func__);

	/* parse noise test config data :use for noise test*/
	ret = gtx8_get_cfg_data(cfg_bin, GTX8_NOISE_TEST_CONFIG, &ts->noise_test_cfg);
	if (ret < 0) {
		memcpy(&ts->noise_test_cfg, &ts->normal_cfg, sizeof(ts->noise_test_cfg));
		TS_LOG_ERR("%s: Failed to parse noise_test_config data,use normal_config data\n", __func__);
	}

	/* parse glove config data */
	if (ts->dev_data->ts_platform_data->feature_info.glove_info.glove_supported) {
		ret = gtx8_get_cfg_data(cfg_bin, GTX8_GLOVE_CONFIG, &ts->glove_cfg);
		if (ret < 0)
			TS_LOG_ERR("%s: Failed to parse glove_config data:%d\n", __func__, ret);

		/* parse glove noise  config data */
		ret = gtx8_get_cfg_data(cfg_bin, GTX8_GLOVE_NOISE_CONFIG, &ts->glove_noise_cfg);
		if (ret < 0)
			TS_LOG_ERR("%s: Failed to parse glove_noise config data:%d\n", __func__, ret);
	}

	/* parse holster config data */
	if (ts->dev_data->ts_platform_data->feature_info.holster_info.holster_supported) {
		ret = gtx8_get_cfg_data(cfg_bin, GTX8_HOLSTER_CONFIG, &ts->holster_cfg);
		if (ret < 0)
			TS_LOG_ERR("%s: Failed to parse holster_config data:%d\n", __func__, ret);

		ret = gtx8_get_cfg_data(cfg_bin, GTX8_HOLSTER_NOISE_CONFIG, &ts->holster_noise_cfg);
		if (ret < 0)
			TS_LOG_ERR("%s: Failed to parse holster_noise_config data:%d\n", __func__, ret);
	}

	/* parse game config data */
	ret = gtx8_get_cfg_data(cfg_bin, GTX8_GAME_CONFIG, &ts->game_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse game_config data:%d\n", __func__, ret);

	/* parse game noise config data */
	ret = gtx8_get_cfg_data(cfg_bin, GTX8_GAME_NOISE_CONFIG, &ts->game_noise_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse game_noise_config data:%d\n", __func__, ret);

exit:
	if (cfg_bin != NULL) {
		release_firmware(cfg_bin);
		cfg_bin = NULL;
	}
	return NO_ERR;
}

static int gtx8_init_configs(struct gtx8_ts_data *ts)
{
	char filename[GTX8_FW_NAME_LEN + 1] = {0};

	snprintf(filename, GTX8_FW_NAME_LEN, "ts/%s_%s_%s",
			ts->dev_data->ts_platform_data->product_name,
			GTX8_OF_NAME, ts->hw_info.pid);
	if (ts->support_filename_contain_projectid ==
			GT8X_FILENAME_CONAIN_PROJECTID_SUPPORT) {
		strncat(filename, "_", strlen("_"));
		strncat(filename, gtx8_ts->project_id,
				strlen(gtx8_ts->project_id));
	}
	if (ts->support_filename_contain_lcd_module) {
		strncat(filename, "_", strlen("_"));
		strncat(filename, gtx8_ts->lcd_module_name,
				strlen(gtx8_ts->lcd_module_name));
	}
	strncat(filename, ".bin", GTX8_FW_NAME_LEN - strlen(filename));
	TS_LOG_INFO("%s: filename =%s\n", __func__, filename);
	return gtx8_get_cfg_parms(ts, filename);
}

static int gtx8_i2c_communicate_check(void)
{
	u8 buf =0;
	int retry = GTX8_RETRY_NUM_3;
	int ret = NO_ERR;
	u16 i2c_detect_addr;

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		i2c_detect_addr = GTP_I2C_DETECT_ADDR_GT7382;
	else
		i2c_detect_addr = GTP_REG_DOZE_STAT;

	while (retry) {
		ret = gtx8_i2c_read_trans(i2c_detect_addr, &buf, 1);
		if (!ret)
			break;
		retry--;
	}

	if (ret) {
		TS_LOG_ERR("%s, Failed check I2C state\n", __func__);
	} else {
		TS_LOG_INFO("I2C state check success:%08X\n", buf);
	}
	return ret;
}

static int gtx8_chip_detect(struct ts_kit_platform_data *pdata)
{
	int ret = NO_ERR;

	TS_LOG_INFO("%s : Chip detect.\n", __func__);

	gtx8_ts->pdev = pdata->ts_dev;
	gtx8_ts->dev_data->ts_platform_data = pdata;
	gtx8_ts->pdev->dev.of_node = pdata->chip_data->cnode;
	gtx8_ts->ops.i2c_read = gtx8_i2c_read;
	gtx8_ts->ops.i2c_write = gtx8_i2c_write;
	gtx8_ts->ops.read_trans = gtx8_i2c_read_trans,
	gtx8_ts->ops.write_trans = gtx8_i2c_write_trans,
	gtx8_ts->ops.chip_reset = gtx8_chip_reset;
	gtx8_ts->ops.send_cmd = gtx8_send_cmd;
	gtx8_ts->ops.send_cfg = gtx8_send_cfg;
	gtx8_ts->ops.read_cfg = gtx8_read_cfg,
	gtx8_ts->ops.read_version = gtx8_read_version;
	gtx8_ts->ops.feature_resume = gtx8_feature_resume;
	gtx8_ts->tools_support = true;
	gtx8_ts->poweron_flag = false;

	/* Do *NOT* remove these logs */
	TS_LOG_INFO("%s:Driver Version: %s\n", __func__, GTP_DRIVER_VERSION);
	gtx8_parse_ic_type();

	ret = gtx8_power_init();
	if (ret< 0) {
		TS_LOG_ERR("gtx8_power_init error %d \n",ret);
		goto out;
	}
	pdata->client->addr = gtx8_ts->dev_data->slave_addr;
	ret = gtx8_pinctrl_init();
	if (ret < 0) {
		TS_LOG_ERR("%s: pinctrl_init fail\n", __func__);
		goto err_regulator;
	}

	/* power on */
	ret = gtx8_power_switch(SWITCH_ON);
	if (ret < 0) {
		TS_LOG_ERR("%s, failed to enable power, ret = %d\n", __func__, ret);
		goto err_pinctrl_init;
	}

	/* reset chip */
	ret = gtx8_chip_reset();
	if (ret < 0) {
		TS_LOG_ERR("%s, failed to chip_reset, ret = %d\n", __func__, ret);
		goto err_power_on;
	}

	ret = gtx8_i2c_communicate_check();
	if (ret < 0) {
		TS_LOG_ERR("%s, failed to i2c_test, ret = %d\n", __func__, ret);
		goto err_power_on;
	}
	strncpy(gtx8_ts->dev_data->chip_name, GOODIX_VENDER_NAME, MAX_STR_LEN -1);

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		gtx8_ts->dev_data->fold_fingers_supported = 1; /* use 20 fingers */
	return ret;
err_power_on:
	gtx8_power_switch(SWITCH_OFF);
err_pinctrl_init:
	gtx8_pinctrl_release();
err_regulator:
	gtx8_power_release();
out:
	return ret;
}

static int  gtx8_read_projectid_tpclolor(void)
{
	u8 read_buf[GT8X_PROJECTID_BUF_LEN] = {0};
	int ret;
	u16 project_id_addr;

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		project_id_addr = GT8X_PROJECTID_ADDR_GT7382;
	else
		project_id_addr = GT8X_PROJECTID_ADDR;

	ret = gtx8_i2c_read_trans(project_id_addr, read_buf,
		GT8X_PROJECTID_LEN);
	if (ret) {
		TS_LOG_ERR("%s: read projectid tp_color error, ret=%d\n",
			__func__, ret);
		return ret;
	}
	memcpy(gtx8_ts->color_id, read_buf + GT8X_COLOR_INDEX,
		sizeof(gtx8_ts->color_id));
	if (gtx8_ts->support_get_tp_color) {
		cypress_ts_kit_color[0] = gtx8_ts->color_id[0];
		TS_LOG_INFO("support_get_tp_color, tp color:0x%x\n",
			cypress_ts_kit_color[0]); /* 1th tpcolor */
	}
	if (gtx8_ts->support_read_projectid) {
		memcpy(gtx8_ts->project_id, read_buf + GT8X_PROJECTID_INDEX,
			GT8X_ACTUAL_PROJECTID_LEN);
		gtx8_ts->project_id[GT8X_ACTUAL_PROJECTID_LEN] = '\0';
		TS_LOG_INFO("support_read_projectid, projectid:%s\n",
			gtx8_ts->project_id);
	}
	return NO_ERR;
}

static int gtx8_get_lcd_panel_info(void)
{
	struct device_node *dev_node = NULL;
	char *lcd_type = NULL;

	if (!gtx8_ts) {
		TS_LOG_ERR("%s: gtx8 ts is null ptr\n", __func__);
		return -EINVAL;
	}
	dev_node = of_find_compatible_node(NULL, NULL,
			LCD_PANEL_TYPE_DEVICE_NODE_NAME);
	if (!dev_node) {
		TS_LOG_ERR("%s: NOT found device node[%s]!\n", __func__,
			LCD_PANEL_TYPE_DEVICE_NODE_NAME);
		return -EINVAL;
	}

	lcd_type = (char *)of_get_property(dev_node, "lcd_panel_type", NULL);
	if (!lcd_type) {
		TS_LOG_ERR("%s: Get lcd panel type faile!\n", __func__);
		return -EINVAL;
	}

	strncpy(gtx8_ts->lcd_panel_info, lcd_type, LCD_PANEL_INFO_MAX_LEN - 1);
	TS_LOG_INFO("lcd_panel_info = %s.\n", gtx8_ts->lcd_panel_info);

	return 0;
}

static void  gtx8_get_lcd_module_name(void)
{
	char temp[LCD_PANEL_INFO_MAX_LEN] = {0};
	int i = 0;

	strncpy(temp, gtx8_ts->lcd_panel_info, LCD_PANEL_INFO_MAX_LEN - 1);
	for (i = 0; i < (LCD_PANEL_INFO_MAX_LEN - 1) && (i < (MAX_STR_LEN - 1)); i++) {
		if (temp[i] == '_' || temp[i] == '-')
			break;
		gtx8_ts->lcd_module_name[i] = tolower(temp[i]);
	}
	TS_LOG_INFO("lcd_module_name = %s.\n", gtx8_ts->lcd_module_name);
}

static int gtx8_chip_init(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	int ret = -1;

#if defined (CONFIG_TEE_TUI)
	int name_len = 0;
	name_len = sizeof(tee_tui_data.device_name);
	memset(tee_tui_data.device_name, 0, name_len);
	if (strlen(GTX8_DEVICE_NAME) >= name_len){
		TS_LOG_ERR("TUI_DEVICE_NAME_LEN limitted !\n");
	}else{
		strncpy(tee_tui_data.device_name, GTX8_DEVICE_NAME, name_len-1);
	}
#endif

	/*init i2c_set_doze_mode para*/
	gtx8_ts->doze_mode_set_count = 0;
	mutex_init(&gtx8_ts->doze_mode_lock);
	init_completion(&roi_data_done);

	/* obtain gtx8 dt properties */
	ret = gtx8_parse_dts();
	if (ret < 0) {
		TS_LOG_ERR("%s: parse_dts fail!\n", __func__);
		return ret;
	}
#if defined CONFIG_TEE_TUI
	if (ts->support_ic_use_max_resolution) {
		strncpy(tee_tui_data.device_name, IC_GTX8_6861,
			strlen(IC_GTX8_6861));
		tee_tui_data.device_name[strlen(IC_GTX8_6861)] = '\0';
	}
#endif

	ts->pen_supported =
		ts->dev_data->ts_platform_data->feature_info.pen_info.pen_supported;
	if (ts->support_wake_lock_suspend == GT8X_WAKE_LOCK_SUSPEND_SUPPORT) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		gtx8_ts->wake_lock = wakeup_source_register(
				&gtx8_ts->pdev->dev, "gtx8tp_wake_lock");
#else
		gtx8_ts->wake_lock = wakeup_source_register("gtx8tp_wake_lock");
#endif
	}
	if (!gtx8_ts->wake_lock) {
		TS_LOG_ERR("%s wakeup source register failed\n", __func__);
		return -EINVAL;
	}
	/* init register addr*/
	gtx8_reg_init(ts);

	/* read version information. pid/vid/sensor id */
	ret = gtx8_read_version(&ts->hw_info);
	if (ret < 0){
		TS_LOG_ERR("%s: read_version fail!\n", __func__);
		if(ts->gtx8_static_pid_support){
			memset(ts->hw_info.pid, 0, PID_DATA_MAX_LEN);
			strncpy(ts->hw_info.pid, ts->chip_name,
				PID_DATA_MAX_LEN - 1);
			TS_LOG_INFO("%s: used static pid:%s!\n", __func__, ts->hw_info.pid);
		}else{
			TS_LOG_INFO("gtx8_static_pid do not support!\n");
		}
	}

	/* obtain specific dt properties */
	ret = gtx8_parse_specific_dts();
	if (ret < 0) {
		TS_LOG_ERR("%s: parse_specific_dts fail!\n", __func__);
		return ret;
	}

	if (ts->tools_support) {
		ret = gtx8_init_tool_node();
		if (ret < 0)
			TS_LOG_ERR("%s : init_tool_node fail!\n", __func__);
	}

	if (ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported) {
		ret = gtx8_ts_roi_init(&gtx8_ts->roi);
		if (ret < 0) {
			TS_LOG_ERR("%s: gtx8_roi_init fail!\n", __func__);
			return ret;
		}
	}

	if ((gtx8_ts->support_get_tp_color == GT8X_READ_TPCOLOR_SUPPORT) ||
			(gtx8_ts->support_read_projectid ==
				GT8X_READ_PROJECTID_SUPPORT)) {
		ret = gtx8_read_projectid_tpclolor();
		if (ret)
			TS_LOG_ERR("%s, read projectid tpcolor error\n",
				__func__);
		TS_LOG_INFO("%s, projectid:%s, tpcolor:0x%x\n", __func__,
				gtx8_ts->project_id,
				gtx8_ts->color_id[0]); /* 1th tpcolor */
	}

	if (gtx8_ts->support_filename_contain_lcd_module) {
		ret = gtx8_get_lcd_panel_info();
		if (ret)
			TS_LOG_ERR("%s, get lcd panel info error\n", __func__);
		gtx8_get_lcd_module_name();
	}
	/* init config data, normal/glove/hoslter config data */
	ret = gtx8_init_configs(ts);
	if (ret < 0) {
		TS_LOG_ERR("%s: init config failed\n", __func__);
		return ret;
	}

	ret = gtx8_feature_resume(ts);
	if (ret < 0) {
		TS_LOG_ERR("%s: gtx8_feature_resume fail!\n", __func__);
		return ret;
	}

	/* esd protector */
	gtx8_ts_esd_init(ts);

	TS_LOG_INFO("%s: end\n", __func__);
	return NO_ERR;
}

static int gtx8_input_config(struct input_dev *input_dev)
{
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts == NULL)
		return -ENODEV;

	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER, input_dev->keybit);

	__set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	__set_bit(TS_SLIDE_L2R, input_dev->keybit);
	__set_bit(TS_SLIDE_R2L, input_dev->keybit);
	__set_bit(TS_SLIDE_T2B, input_dev->keybit);
	__set_bit(TS_SLIDE_B2T, input_dev->keybit);
	__set_bit(TS_CIRCLE_SLIDE, input_dev->keybit);
	__set_bit(TS_LETTER_C, input_dev->keybit);
	__set_bit(TS_LETTER_E, input_dev->keybit);
	__set_bit(TS_LETTER_M, input_dev->keybit);
	__set_bit(TS_LETTER_W, input_dev->keybit);
	__set_bit(TS_PALM_COVERED, input_dev->keybit);
	__set_bit(TS_STYLUS_WAKEUP_TO_MEMO, input_dev->keybit);
	__set_bit(TS_STYLUS_WAKEUP_SCREEN_ON, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);


#ifdef INPUT_TYPE_B_PROTOCOL
	input_mt_init_slots(input_dev,GTP_MAX_TOUCH * 2 + 1,INPUT_MT_DIRECT);
#endif

	/*set ABS_MT_TOOL_TYPE*/
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ts->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ts->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, GTP_MAX_TOUCH, 0, 0);

	return 0;
}

static int gtx8_get_standard_matrix(struct roi_matrix *src_mat, struct roi_matrix *dst_mat)
{
	int i = 0, j = 0;
	short *temp_buf = NULL;
	short diff_max = 0x8000;
	int peak_row = 0, peak_col = 0;
	int src_row_start = 0, src_row_end = 0;
	int src_col_start = 0, src_col_end = 0;
	int dst_row_start = 0, dst_col_start = 0;

	/* transport src_mat matrix */
	temp_buf = kzalloc((src_mat->row * src_mat->col + 1) * sizeof(*temp_buf), GFP_KERNEL);
	if (!temp_buf) {
		TS_LOG_ERR("Failed transport the ROI matrix\n");
		return -EINVAL;
	}

	for (i = 0; i < src_mat->row; i++) {
		for (j = 0; j < src_mat->col; j++) {
			temp_buf[(src_mat->row - i -1) + (src_mat->col -j -1) * src_mat->row] =
				src_mat->data[i * src_mat->col + j];
		}
	}

	i = src_mat->row;
	src_mat->row = src_mat->col;
	src_mat->col = i;
	memcpy(src_mat->data, temp_buf, src_mat->row * src_mat->col * sizeof(*temp_buf));
	kfree(temp_buf);
	temp_buf = NULL;

	/* get peak value postion */
	for (i = 0; i< src_mat->row; i++) {
		for (j = 0; j < src_mat->col; j++) {
			if (src_mat->data[i * src_mat->col + j] > diff_max) {
				diff_max = src_mat->data[i * src_mat->col + j];
				peak_row = i;
				peak_col = j;
			}
		}
	}
	TS_LOG_DEBUG("DEBUG Peak pos[%d][%d] = %d\n", peak_row, peak_col, diff_max);
	src_row_start = 0;
	dst_row_start = ((dst_mat->row - 1) >> 1) - peak_row;
	if (peak_row >= ((dst_mat->row - 1) >> 1)) {
		src_row_start = peak_row - ((dst_mat->row - 1) >> 1);
		dst_row_start = 0;
	}

	src_row_end = src_mat->row -1;
	if (peak_row <= (src_mat->row - 1 - (dst_mat->row >> 1))) {
		src_row_end = peak_row + (dst_mat->row >> 1);
	}

	src_col_start = 0;
	dst_col_start = ((dst_mat->col -1) >> 1) - peak_col;
	if (peak_col >= ((dst_mat->col -1) >> 1)) {
		src_col_start = peak_col - ((dst_mat->col -1) >> 1);
		dst_col_start = 0;
	}

	src_col_end = src_mat->col -1;
	if (peak_col <= (src_mat->col -1 - (dst_mat->col >> 1))) {
		src_col_end = peak_col + (dst_mat->col >> 1);
	}

	/* copy peak value area to the center of ROI matrix */
	memset(dst_mat->data, 0, sizeof(short) * dst_mat->row * dst_mat->col);
	for (i = src_row_start; i <= src_row_end; i++) {
		memcpy(&dst_mat->data[dst_col_start + dst_mat->col * (dst_row_start + i - src_row_start)],
			&src_mat->data[i * src_mat->col + src_col_start],
			(src_col_end -src_col_start + 1) * sizeof(short));
	}

	return NO_ERR;
}

/*
 * gtx8_cache_roidata_fw - caching roi data
 * @roi: roi data structure
 * status[0]: roi state flag
 */
static int gtx8_cache_roidata_fw(struct gtx8_ts_roi *roi)
{
	unsigned int roi_bytes;
	u8 status[ROI_HEAD_LEN + 1] = {0};
	u16 checksum = 0;
	int i;
	unsigned int j;
	int ret;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (!ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported
			|| !roi || !roi->enabled) {
		TS_LOG_ERR("%s:roi is not support or invalid parameter\n",
			__func__);
		return -EINVAL;
	}

	ret = gtx8_i2c_read((u16)ts->gtx8_roi_data_add, status, ROI_HEAD_LEN);
	if (unlikely(ret < 0))
		return ret;

	for (i = 0; i < ROI_HEAD_LEN; i++)
		checksum += status[i];

	TS_LOG_INFO("ROI head{%02x %02x %02x %02x}\n",
		status[0], status[1], status[2], status[3]);

	if (unlikely((u8)checksum != 0)) { /* cast to 8bit checksum */
		TS_LOG_ERR("%s:roi status checksum error\n", __func__);
		return -EINVAL;
	}

	if (likely(status[0] & ROI_READY_MASK)) { /* roi data ready */
		roi->track_id = status[0] & ROI_TRACKID_MASK;
	} else {
		TS_LOG_ERR("%s:not read\n", __func__);
		return -EINVAL; /* not ready */
	}

	mutex_lock(&roi->mutex);
	roi->data_ready = false;
	/*
	 * The data read from the IC is 16-bit.
	 * One piece of data occupies two bytes.
	 */
	roi_bytes = (roi->roi_rows * roi->roi_cols + 1) * ROI_DATA_BYTES;

	ret = gtx8_i2c_read((u16)ts->gtx8_roi_data_add + ROI_HEAD_LEN,
				(u8 *)(roi->rawdata) + ROI_HEAD_LEN, roi_bytes);
	if (unlikely(ret < 0)) {
		mutex_unlock(&roi->mutex);
		TS_LOG_ERR("%s:i2c_read failed!\n", __func__);
		return ret;
	}

	/*
	 * The two adjacent bytes of the read data are combined into a complete
	 * data 16-bit, because the I2C can read only eight bits at a time.
	 */
	for (j = 0, checksum = 0; j < roi_bytes / ROI_DATA_BYTES ; j++) {
		roi->rawdata[j + ROI_HEAD_LEN / ROI_DATA_BYTES] =
			le16_to_cpu(roi->rawdata[j + ROI_HEAD_LEN /
				ROI_DATA_BYTES]);
		checksum += roi->rawdata[j + ROI_HEAD_LEN / ROI_DATA_BYTES];
	}

	memcpy(&roi->rawdata[0], &status[0], ROI_HEAD_LEN);

	if (unlikely(checksum != 0))
		TS_LOG_ERR("%s:roi data checksum error\n", __func__);
	else
		roi->data_ready = true;

	mutex_unlock(&roi->mutex);

	status[0] = 0x00;
	ret = gtx8_i2c_write(ts->gtx8_roi_data_add, status, 1);

	return ret;
}

/*
 * gtx8_cache_roidata_device - caching roi data
 * @roi: roi data structure
 * return 0 ok, < 0 fail
 */
static int gtx8_cache_roidata_device(struct gtx8_ts_roi *roi)
{
	unsigned char status[ROI_HEAD_LEN] = {0};
	struct roi_matrix src_mat, dst_mat;
	u16 checksum = 0;
	int i = 0, res_write = 0, ret = NO_ERR;
	struct gtx8_ts_data *ts = gtx8_ts;
	memset(&src_mat, 0, sizeof(src_mat));
	memset(&dst_mat, 0, sizeof(dst_mat));

	if (!ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported
			|| !roi || !roi->enabled) {
		TS_LOG_ERR("%s:roi is not support or invalid parameter\n", __func__);
		return -EINVAL;
	}

	roi->data_ready = false;

	ret = gtx8_i2c_read_trans(ts->gtx8_roi_data_add, status, ROI_HEAD_LEN);
	if (ret) {
		TS_LOG_ERR("Failed read ROI HEAD\n");
		return -EINVAL;
	}
	TS_LOG_DEBUG("ROI head{%02x %02x %02x %02x}\n",
			status[0], status[1], status[2], status[3]);

	for (i = 0; i < ROI_HEAD_LEN; i++) {
		checksum += status[i];
	}

	if (unlikely((u8)checksum != 0)) {
		TS_LOG_ERR("roi status checksum error{%02x %02x %02x %02x}\n",
				status[0], status[1], status[2], status[3]);
		return -EINVAL;
	}

	if (status[0] & ROI_READY_MASK) {
		roi->track_id = status[0] & ROI_TRACKID_MASK;
	} else {
		TS_LOG_ERR("ROI data not ready\n");
		return -EINVAL;
	}

	dst_mat.row = roi->roi_rows;
	dst_mat.col = roi->roi_cols;
	src_mat.row = status[GTX8_ROI_SRC_STATUS_INDEX] & GTX8_BIT_AND_0x0F;
	src_mat.col = status[GTX8_ROI_SRC_STATUS_INDEX] >> 4;

	src_mat.data = kzalloc((src_mat.row * src_mat.col + 1) * sizeof(src_mat.data[0]), GFP_KERNEL);
	if (!src_mat.data) {
		TS_LOG_ERR("failed alloc src_roi memory\n");
		return -ENOMEM;
	}
	dst_mat.data = kzalloc((dst_mat.row * dst_mat.col + 1) * sizeof(dst_mat.data[0]), GFP_KERNEL);
	if (!dst_mat.data) {
		TS_LOG_ERR("failed alloc dst_roi memory\n");
		kfree(src_mat.data);
		src_mat.data = NULL;
		return -ENOMEM;
	}

	/* read ROI rawdata */
	ret = gtx8_i2c_read_trans((ts->gtx8_roi_data_add + ROI_HEAD_LEN),
						(char*)src_mat.data,
						(src_mat.row * src_mat.col + 1) * sizeof(src_mat.data[0]));
	if (ret) {
		TS_LOG_ERR("Failed read ROI rawdata\n");
		ret = -EINVAL;
		goto err_out;
	}

	for (i = 0, checksum = 0; i < src_mat.row * src_mat.col + 1; i++) {
		src_mat.data[i] = be16_to_cpu(src_mat.data[i]);
		checksum += src_mat.data[i];
	}
	if (checksum) {
		TS_LOG_ERR("ROI rawdata checksum error\n");
		goto err_out;
	}

	if (gtx8_get_standard_matrix(&src_mat, &dst_mat)) {
		TS_LOG_ERR("Failed get standard matrix\n");
		goto err_out;
	}

	mutex_lock(&roi->mutex);
	memcpy(&roi->rawdata[0], &status[0], ROI_HEAD_LEN);
	memcpy(((char*)roi->rawdata) + ROI_HEAD_LEN, &dst_mat.data[0],
		   roi->roi_rows * roi->roi_cols * sizeof(*roi->rawdata));

	roi->data_ready = true;
	mutex_unlock(&roi->mutex);
	ret = 0;
err_out:
	kfree(src_mat.data);
	src_mat.data = NULL;
	kfree(dst_mat.data);
	dst_mat.data = NULL;

	/* clear the roi state flag */
	status[0] = 0x00;
	res_write = gtx8_i2c_write_trans(ts->gtx8_roi_data_add, status, 1);
	if(res_write)
		TS_LOG_ERR("%s:clear ROI status failed [%d]\n", __func__, res_write);

	return ret;
}

#define GTX8_SINGLE_RECT_DATA_SIZE 6
#define GTX8_RECT_DATA_SIZE  60
#define GTX8_RECT_DATA_ADDR   0x4154
static void goodix_report_finger(struct gtx8_ts_data  *ts, struct ts_fingers *ts_fingers, u8 *touch_data, int touch_num)
{
	unsigned int id = 0, x = 0, y = 0, w = 0, i = 0;
	unsigned int xer = 0, yer = 0;
	unsigned int ewx = 0, ewy = 0;
	unsigned int wx = 0, wy = 0;
	u8 *coor_data = NULL;
	u8 finger_rect_data[GTX8_RECT_DATA_SIZE] = {0};
	unsigned int rect_data_size = 0;
	cur_index = 0;
	/* finger rect data info
	  * [x + 0] wx
	  * [x + 1] wy
	  * [x + 2] ewx
	  * [x + 3] ewy
	  * [x + 4] xer
	  * [x + 5] yer
	  * ....
	  */
	if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag && touch_num) {
		rect_data_size = touch_num * GTX8_SINGLE_RECT_DATA_SIZE > GTX8_RECT_DATA_SIZE ? GTX8_RECT_DATA_SIZE :
							touch_num * GTX8_SINGLE_RECT_DATA_SIZE; //data len of aft data.
		if (gtx8_i2c_read_trans(GTX8_RECT_DATA_ADDR, finger_rect_data, rect_data_size))
			TS_LOG_INFO("%s: Faied read RECT data\n", __func__);
	}
	memset(ts_fingers, 0x00, sizeof(struct ts_fingers));
	coor_data = &touch_data[2]; // touch_data size = 84 (4 + BYTES_PER_COORD * GTP_MAX_TOUCH )

	for (i = 0; i < touch_num; i++) {
		id = coor_data[i * BYTES_PER_COORD];
		x = coor_data[i * BYTES_PER_COORD + 1] |
				coor_data[i * BYTES_PER_COORD + 2] << 8;
		y = coor_data[i * BYTES_PER_COORD + 3] |
				coor_data[i * BYTES_PER_COORD + 4] << 8;
		w = coor_data[i * BYTES_PER_COORD + 5];
		if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag){
			wx = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE];
			wy = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 1];
			ewx = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 2];
			ewy = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 3];
			xer = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 4];
			yer = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 5];
		}
		if(id >= TS_MAX_FINGER){
			TS_LOG_INFO("%s:invaild finger id =%d.\n", __func__, id);
			break;
		}
		if ((ts->in_recovery_mode == GTX8_USE_IN_RECOVERY_MODE) &&
			(ts->use_ic_res == GTX8_USE_IC_RESOLUTION) &&
			(ts->max_x != 0) && (ts->max_y != 0)) {
			ts_fingers->fingers[id].x = x * ts->x_max_rc /
				(ts->max_x - 1);
			ts_fingers->fingers[id].y = y * ts->y_max_rc /
				(ts->max_y - 1);
		} else {
			ts_fingers->fingers[id].x = x;
			ts_fingers->fingers[id].y = y;
		}
		ts_fingers->fingers[id].pressure = w;
		ts_fingers->fingers[id].status = TP_FINGER;
		cur_index |= 1 << id;
		TS_LOG_DEBUG("%s:[%d] , w %d.\n", __func__, id, w);
		if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag){
			ts_fingers->fingers[id].wx = wx;
			ts_fingers->fingers[id].wy = wy;
			ts_fingers->fingers[id].ewx = ewx;
			ts_fingers->fingers[id].ewy = ewy;
			ts_fingers->fingers[id].xer = xer;
			ts_fingers->fingers[id].yer = yer;
			TS_LOG_DEBUG("%s:[%d] wx %d, wy %d, ewx %d, ewy %d, xer %d, yer %d\n", __func__, id, wx, wy, ewx, ewy, xer, yer);
		}
	}
	ts_fingers->cur_finger_number = touch_num;
	return;
}

#define GT738X_RECT_DATA_ADDR   0xF8A4
static void goodix_report_finger_gt7382(struct gtx8_ts_data  *ts,
	struct ts_fingers *ts_fingers, u8 *touch_data, int touch_num)
{
	unsigned int id;
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int i;
	u8 *coor_data = NULL;
	u8 finger_rect_data[GTX8_RECT_DATA_SIZE] = {0};
	unsigned int xer;
	unsigned int yer;
	unsigned int ewx;
	unsigned int ewy;
	unsigned int wx;
	unsigned int wy;
	unsigned int rect_data_size = 0;
	unsigned int revoke_flag;
	cur_index = 0;

	if (touch_num == 0)
		goto exit;
	if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag
		&& touch_num) {
		rect_data_size = touch_num * GTX8_SINGLE_RECT_DATA_SIZE >
			GTX8_RECT_DATA_SIZE ? GTX8_RECT_DATA_SIZE :
			touch_num * GTX8_SINGLE_RECT_DATA_SIZE;
		if (gtx8_i2c_read_trans(GT738X_RECT_DATA_ADDR,
			finger_rect_data, rect_data_size))
			TS_LOG_INFO("%s: Faied read RECT data\n", __func__);
	}
	memset(ts_fingers, 0x00, sizeof(struct ts_fingers));
	coor_data = &touch_data[1]; /* coor_data offset */

	/* finger rect data info
	 * [x + 0] wx
	 * [x + 1] wy
	 * [x + 2] ewx
	 * [x + 3] ewy
	 * [x + 4] xer
	 * [x + 5] yer
	 * ....
	 */
	for (i = 0; i < touch_num; i++) {
		id = coor_data[i * BYTES_PER_COORD];
		x = coor_data[i * BYTES_PER_COORD + 1] |
			(coor_data[i * BYTES_PER_COORD + 2] << 8);
		y = coor_data[i * BYTES_PER_COORD + 3] |
			(coor_data[i * BYTES_PER_COORD + 4] << 8);
		w = coor_data[i * BYTES_PER_COORD + 5] |
			(coor_data[i * BYTES_PER_COORD + 6] << 8);
		if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag) {
			wx = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE];
			wy = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 1];
			ewx = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 2];
			ewy = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 3];
			xer = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 4];
			yer = finger_rect_data[i * GTX8_SINGLE_RECT_DATA_SIZE + 5];
		}
		if (id >= TS_MAX_FINGER) {
			TS_LOG_ERR("%s:invaild finger id =%d.\n",
				__func__, id);
			break;
		}
		ts_fingers->fingers[id].x = x;
		ts_fingers->fingers[id].y = y;
		ts_fingers->fingers[id].pressure = w;
		if (w != 0) { /* pressure is 0 when no touch. */
			ts_fingers->fingers[id].status = TP_FINGER;
			cur_index |= 1 << id;
		} else {
			ts_fingers->fingers[id].status = TP_NONE_FINGER;
		}
		if (ts->dev_data->ts_platform_data->aft_param.aft_enable_flag) {
			ts_fingers->fingers[id].wx = wx;
			ts_fingers->fingers[id].wy = wy;
			ts_fingers->fingers[id].ewx = ewx;
			ts_fingers->fingers[id].ewy = ewy;
			ts_fingers->fingers[id].xer = xer;
			ts_fingers->fingers[id].yer = yer;
			TS_LOG_DEBUG("%s:[%d] wx %d, wy %d, ewx %d, ewy %d, xer %d, yer %d\n",
				__func__, id, wx, wy, ewx, ewy, xer, yer);
		}
		if (g_ts_kit_platform_data.fp_tp_enable) {
			/* 0x824E+i*8+6 revoke flag reg */
			revoke_flag = coor_data[i * BYTES_PER_COORD + 6];
			/* bit29 fwk revoke flag */
			ts_fingers->fingers[id].major =
				((revoke_flag & (1 << 0)) << 29);
			ts_fingers->fingers[id].minor = 0;
			TS_LOG_DEBUG("id=%d revoke flag:%d\n", id,
				revoke_flag);
		}
		TS_LOG_DEBUG("%s:x= %d, y= %d, id = [%d] , w %d\n", __func__,
			x, y, id, w);
	}
exit:
	ts_fingers->cur_finger_number = touch_num;
	return;
}

static void goodix_report_pen(struct gtx8_ts_data  *ts, struct ts_pens *ts_pens, u8 *touch_data, int touch_num){
	int  x = 0, y = 0, w = 0;
	u8 *coor_data = NULL;
	bool have_pen = false;
	static struct ts_pens pre_ts_pens;
	static u8 pre_key_of_pen = 0;

	if(! ts->dev_data->ts_platform_data->feature_info.pen_info.pen_supported){
		return;
	}
	if(touch_num >= 1 && touch_data[8 * (touch_num - 1) + 2] >= 0x80){
		have_pen = true;
	} else{
		have_pen = false;
	}

	memset(ts_pens, 0x00, sizeof(struct ts_pens));

	/*report pen down*/
	if(have_pen){
		/*report coor of pen*/
		coor_data = &touch_data[2 + BYTES_PER_COORD  * (touch_num - 1)];
		x = le16_to_cpup((__le16 *)&coor_data[1]);
		y = le16_to_cpup((__le16 *)&coor_data[3]);
		w = le16_to_cpup((__le16 *)&coor_data[5]);

		ts_pens->tool.x = x;
		ts_pens->tool.y = y;
		ts_pens->tool.pressure = w;
		ts_pens->tool.pen_inrange_status = 1;
		ts_pens->tool.tool_type = BTN_TOOL_PEN;
		TS_LOG_DEBUG("gtx8 :p:%02X\n",ts_pens->tool.pressure);

		if(ts_pens->tool.pressure > 0){
			ts_pens->tool.tip_status = 1;
		} else{
			ts_pens->tool.tip_status = 0;
		}

		TS_LOG_DEBUG("gtx8 : touch_data[1]:%02X, key_value = %02X\n", touch_data[1], touch_data[8 * touch_num + 2]);

		/*report key of pen*/
		if((touch_data[1] & 0x10) == 0x10){
			if((touch_data[8 * touch_num + 2] & 0x10) == 0x10){
				ts_pens->buttons[0].status = TS_PEN_BUTTON_PRESS;
				ts_pens->buttons[0].key = BTN_STYLUS;
				pre_key_of_pen |= 1 << 4;
				TS_LOG_DEBUG("pen buttons down, status:0x%02x, key:0x%02x, pre_key:0x%02x\n", ts_pens->buttons[0].status, ts_pens->buttons[0].key, pre_key_of_pen);
			} else if((pre_key_of_pen & 0x10) == 0x10){
				ts_pens->buttons[0].status = TS_PEN_BUTTON_RELEASE;
				ts_pens->buttons[0].key = BTN_STYLUS;
				pre_key_of_pen &= ~(1 << 4);
				TS_LOG_DEBUG("pen buttons up, status:0x%02x, key:0x%02x, pre_key:0x%02x\n", ts_pens->buttons[0].status, ts_pens->buttons[0].key, pre_key_of_pen);
			}
			TS_LOG_INFO("gtx8 buttons[0].status:0x%02x, key:0x%02x\n", ts_pens->buttons[0].status, ts_pens->buttons[0].key);
		} else{
			if((pre_key_of_pen & 0x10) == 0x10){
				ts_pens->buttons[0].status = TS_PEN_BUTTON_RELEASE;
				ts_pens->buttons[0].key = BTN_STYLUS;
				TS_LOG_INFO("gtx8 buttons up, status:0x%02x, key:0x%02x, pre_key:0x%02x\n", ts_pens->buttons[0].status, ts_pens->buttons[0].key, pre_key_of_pen);
			}
			pre_key_of_pen = 0;
		}
		memcpy(&pre_ts_pens, ts_pens, sizeof(struct ts_pens));
	}else{/*report pen leave*/
		ts_pens->tool.x = pre_ts_pens.tool.x;
		ts_pens->tool.y = pre_ts_pens.tool.y;
		ts_pens->tool.pressure = 0;
		ts_pens->tool.pen_inrange_status = 0;
		ts_pens->tool.tip_status = 0;
		ts_pens->tool.tool_type = BTN_TOOL_PEN;

		if((pre_key_of_pen & 0x10) == 0x10){
			ts_pens->buttons[0].status = TS_PEN_BUTTON_RELEASE;
			ts_pens->buttons[0].key = BTN_STYLUS;
			TS_LOG_INFO("gtx8 buttons up, status:0x%02x, key:0x%02x, pre_key:0x%02x\n", ts_pens->buttons[0].status, ts_pens->buttons[0].key, pre_key_of_pen);
		}

		pre_key_of_pen = 0;
		memset(&pre_ts_pens, 0x00, sizeof(struct ts_pens));
	}
}

static void gtx8_pen_palm_suppression(int soper)
{
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	u8 palm_suppression_in[TS_SWITCH_PEN_CMD_LEN] = {
		GTX8_CMD_STYLUS_RESPONSE_MODE,
		TS_PEN_PALM_SUPPRESSION_CMD,
		TS_PEN_PALM_SUPPRESSION_IN_CHECK
	};
	u8 palm_suppression_out[TS_SWITCH_PEN_CMD_LEN] = {
		GTX8_CMD_PEN_NORMAL_MODE_7382,
		TS_PEN_PALM_SUPPRESSION_CMD,
		TS_PEN_PALM_SUPPRESSION_OUT_CHECK
	};
	int ret;
	u8 *buff = NULL;

	if (gtx8_ts->ic_type != IC_TYPE_7382)
		return;
	if (!pdata->chip_data->support_stylus3_plam_suppression) {
		TS_LOG_ERR("unsupported stylus3 plam suppression\n");
		return;
	}
	if (soper)
		buff = palm_suppression_in;
	else
		buff = palm_suppression_out;
	ret = gtx8_i2c_write(GTP_REG_CMD_GT7382, buff, TS_SWITCH_PEN_CMD_LEN);
	if (ret) {
		TS_LOG_ERR("send palm suppression fail\n");
		return;
	}
	msleep(50); /* ic need 50ms delay */
	TS_LOG_INFO("pen_palm_suppression_mode:%d\n", soper);
}

static void send_suppression_cmd_to_fw(unsigned int suppression_status)
{
	int state;
	struct ts_easy_wakeup_info *easy_wakeup_info =
		&g_ts_kit_platform_data.chip_data->easy_wakeup_info;

	TS_LOG_INFO("%s:enter\n", __func__);
	if (gtx8_ts->ic_type != IC_TYPE_7382) {
		TS_LOG_ERR("%s:ic type not match\n", __func__);
		return;
	}
	if ((easy_wakeup_info->sleep_mode == TS_POWER_OFF_MODE) &&
		(atomic_read(&g_ts_kit_platform_data.state) != TS_WORK)) {
		TS_LOG_DEBUG("power off mode, no need to respond cmd\n");
		return;
	}
	if (suppression_status == STYLUS3_PLAM_SUPPRESSION_ON)
		state = GLOBAL_PALM_SUPPRESSION_ON;
	else
		state = GLOBAL_PALM_SUPPRESSION_OFF;
	gtx8_pen_palm_suppression(state);
	TS_LOG_INFO("%s:over\n", __func__);
}

static int send_stylus3_typeinfo_to_fw(u8 *stylus3_type_info_buf,
	u32 len, u8 check_sum)
{
	int i;
	int ret = NO_ERR;
	u8 check_data_value = GT73X_STYLUS3_TYPE_DEF_CHECK_DATA;

	if (gtx8_ts->ic_type != IC_TYPE_7382) {
		TS_LOG_ERR("%s:ic type not match\n", __func__);
		return -EINVAL;
	}
	if ((!stylus3_type_info_buf) || (len > GT73X_STYLUS3_TYPE_DATA_LEN)) {
		TS_LOG_ERR("%s: stylus3_type_buf addr or buf_len is invalid\n",
			__func__);
		return -EINVAL;
	}
	for (i = 0; i < GT73X_STYLUS3_TYPE_RETRY; i++) {
		ret = gtx8_i2c_write(GTP_REG_CMD_GT7382,
			stylus3_type_info_buf, len);
		if (ret) {
			TS_LOG_ERR("time %d send stylus type to ic failed\n",
				i);
			continue;
		}
		msleep(SYTULS_DELAY);
		ret = gtx8_i2c_read(GT73X_STYLUS3_TYPE_CHECK_REG_ADDR,
			&check_data_value, GT73X_STYLUS3_TYPE_CHECK_DATA_LEN);
		if (ret) {
			TS_LOG_ERR("time %d read check data value frome ic failed\n", i);
			continue;
		}
		if ((check_data_value == check_sum) &&
			(check_data_value != GT73X_STYLUS3_TYPE_DEF_CHECK_DATA))
			break;
	}
	if (i == GT73X_STYLUS3_TYPE_RETRY) {
		TS_LOG_ERR("%s: send stylus type info to ic failed\n",
			__func__);
		return ret;
	}

	TS_LOG_INFO("%s: send stylus type info to ic succ: %u\n",
		__func__, check_data_value);
	return ret;
}

static int stylus3_type_info_handle(u32 stylus3_type_info)
{
	int ret = NO_ERR;
	int list;
	u8 check_data;
	u8 stylus3_type;
	u8 stylus3_type_info_buf[GT73X_STYLUS3_TYPE_DATA_LEN] = { 0 };

	if (gtx8_ts->ic_type != IC_TYPE_7382) {
		TS_LOG_ERR("%s:ic type not match\n", __func__);
		return -EINVAL;
	}
	stylus3_type = (u8)((stylus3_type_info >> MOVE_8_BIT) & GET_8BIT_DATA);
	TS_LOG_INFO("stylus3_type_info = 0x%x, stylus3_type = %u\n",
		stylus3_type_info, stylus3_type);
	for (list = 0; list < ARRAY_SIZE(stylus3_type_info_table); list++) {
		if (stylus3_type == stylus3_type_info_table[list].vendor) {
			memcpy(stylus3_type_info_buf,
				stylus3_type_info_table[list].info,
				GT73X_STYLUS3_TYPE_DATA_LEN);
			check_data = stylus3_type_info_table[list].checksum;
			break;
		}
	}
	if (list == ARRAY_SIZE(stylus3_type_info_table)) {
		TS_LOG_ERR("%s:no type matched,use default stylus param\n",
			__func__);
		/* default cd54 stylus */
		memcpy(stylus3_type_info_buf, stylus3_type_info_table[2].info,
			GT73X_STYLUS3_TYPE_DATA_LEN);
		check_data = stylus3_type_info_table[2].checksum;
	}
	ret = send_stylus3_typeinfo_to_fw(stylus3_type_info_buf,
		GT73X_STYLUS3_TYPE_DATA_LEN, check_data);
	if (ret) {
		TS_LOG_ERR("%s:failed\n", __func__);
		return ret;
	}

	return ret;
}

static int stylus3_command(u8 buffer[], u32 len)
{
	int ret_i2c;

	ret_i2c = gtx8_i2c_write(GTP_REG_CMD_GT7382,
		&buffer[0], len);
	if (ret_i2c) {
		TS_LOG_ERR("i2c_write command fail\n");
		return ret_i2c;
	}
	mdelay(SYTULS_DELAY);

	ret_i2c = gtx8_i2c_read(GTP_REG_CMD_GT7382,
		&buffer[0], len);
	if (ret_i2c) {
		TS_LOG_ERR("i2c_read command fail\n");
		return ret_i2c;
	}

	TS_LOG_INFO("%s,buffer[0] :%x\n", __func__, buffer[0]);

	return ret_i2c;
}

int stylus3_connect_state(unsigned int stylus3_status)
{
	int ret = NO_ERR;

	u8 stylus3_on_buffer[SYTULS_CONNECT_BUFFER_LEN] = {
		SYTULS_CONNECT_ON_COMMAND, SYTULS_CONNECT_ON_VALUE,
		SYTULS_CONNECT_ON_CHECKSUM };
	u8 stylus3_off_buffer[SYTULS_CONNECT_BUFFER_LEN] = {
		SYTULS_CONNECT_OFF_COMMAND, SYTULS_CONNECT_OFF_VALUE,
		SYTULS_CONNECT_OFF_CHECKSUM };
	if (stylus3_status == SYTULS_CONNECT_STATUS) {
		ret = stylus3_command(stylus3_on_buffer,
			SYTULS_CONNECT_BUFFER_LEN);
		if (ret) {
			TS_LOG_ERR("stylus connect command sent fail\n");
			ret = -EINVAL;
		}
	}
	if (stylus3_status == SYTULS_DISCONNECT_STATUS) {
		ret = stylus3_command(stylus3_off_buffer,
			SYTULS_CONNECT_BUFFER_LEN);
		if (ret) {
			TS_LOG_ERR("stylus disconnected command sent fail\n");
			ret = -EINVAL;
		}
	}
	return ret;
}

static void supend_stylus_command_sent(void)
{
	unsigned int stylus3_status;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	if ((gtx8_ts->ic_type == IC_TYPE_7382) && (dev->send_bt_status_to_fw)) {
		stylus3_status = atomic_read(&(g_ts_kit_platform_data.current_stylus3_status));
		stylus3_status = (stylus3_status & SYTULS_VALUE);
		TS_LOG_INFO("%s stylus3_status: %u\n", __func__, stylus3_status);
		if (stylus3_status == SYTULS_DISCONNECT_STATUS)
			stylus3_connect_state(stylus3_status);
	}
}
static void gtx8_stylus_shift_freq_gt7382(const u8 *coor_data)
{
	u8 shift_freq_flag = 0;
	int ret;
	unsigned int stylus3_status;
	static struct shift_freq_info shift_freq_info_tmp;

	ret = gtx8_i2c_read_trans(GTP_REG_STYLUS3_SHIF_FREQ,
		&shift_freq_flag, 1);
	if (ret) {
		TS_LOG_ERR("%s: read stylus3 shif freq flag fail\n",
			__func__);
		return;
	}
	TS_LOG_DEBUG("%s: 0xFF22=%x\n", __func__, shift_freq_flag);
	if (shift_freq_flag == GTP_SHIFT_FREQ_IDLE) { /* idle */
		shift_freq_info_tmp.f1 = *(coor_data + GTP_REG_FREQ_TX1_GT7382);
		shift_freq_info_tmp.f2 = *(coor_data + GTP_REG_FREQ_TX2_GT7382);
		TS_LOG_DEBUG("idle mode f1=%d, f2=%d\n",
			shift_freq_info_tmp.f1, shift_freq_info_tmp.f2);
	} else if (shift_freq_flag == GTP_SHIFT_FREQ_ACTIVE) {
		shift_freq_info_tmp.next_f1 =
			*(coor_data + GTP_REG_FREQ_TX1_GT7382);
		shift_freq_info_tmp.next_f2 =
			*(coor_data + GTP_REG_FREQ_TX2_GT7382);
		TS_LOG_INFO("need shift freq f1=%d, f2=%d, next_f1=%d, next_f2=%d\n",
			shift_freq_info_tmp.f1, shift_freq_info_tmp.f2,
			shift_freq_info_tmp.next_f1,
			shift_freq_info_tmp.next_f2);
		stylus3_status = STYLUS3_FREQ_SHIFT_REQUEST;
		ts_update_stylu3_shif_freq_flag(stylus3_status, true);
		copy_stylus3_shift_freq_to_aft_daemon(&shift_freq_info_tmp);
		TS_LOG_INFO("%s, send shift freq to daemon\n", __func__);
	} else {
		TS_LOG_ERR("invalid status\n");
	}
}

static int gtx8_write_hop_ack_gt7382(void)
{
	u8 write_data;
	int ret;
	int retry_count = GTX8_RETRY_NUM_3;

	if (gtx8_ts->ic_type != IC_TYPE_7382)
		return 0;
	write_data = GTP_SHIFT_FREQ_CONFIRM;
	while (retry_count) {
		ret = gtx8_i2c_write_trans(GTP_REG_STYLUS3_SHIF_FREQ,
			&write_data, 1);
		if (ret) {
			TS_LOG_ERR("%s, write stylus3 shift freq reg err, retry=%d\n",
				__func__, retry_count);
		} else {
			TS_LOG_INFO("%s, write stylus3 shift freq reg ok\n",
				__func__);
			break;
		}
		retry_count--;
	}
	if (!ret)
		ts_update_stylu3_shif_freq_flag(STYLUS3_FREQ_SHIFT_REQUEST,
			false);
	return ret;
}

static void goodix_report_pen_gt7382(struct gtx8_ts_data  *ts,
	struct ts_pens *ts_pens, u8 *touch_data, int touch_num)
{
	static int x;
	static int y;
	int w;
	u8 *coor_data = NULL;
	bool have_pen = false;
	static struct ts_pens pre_ts_pens;
	s16 x_angle;
	s16 y_angle;
	u16 x_angle_u;
	u16 y_angle_u;

	if (!ts->dev_data->ts_platform_data->feature_info.pen_info.pen_supported)
		return;
	if ((touch_num >= 1) &&
		(touch_data[BYTES_PER_COORD * (touch_num - 1) +
		HEAD_LEN] >= 0x80)) /* pen flag */
		have_pen = true;
	else
		have_pen = false;

	memset(ts_pens, 0x00, sizeof(struct ts_pens));

	/* report pen down */
	if (have_pen) {
		/*
		 * report coor of pen
		 * 1th & 2th: pen x
		 * 3th & 4th: pen y
		 * 5th & 6th: pen pressure
		 * 7th & 8th: origin tilt_x
		 * 9th & 10th:origin tilt_y
		 */
		coor_data = &touch_data[HEAD_LEN +
			BYTES_PER_COORD * (touch_num - 1)];
		x = le16_to_cpup((__le16 *)&coor_data[1]);
		y = le16_to_cpup((__le16 *)&coor_data[3]);
		w = le16_to_cpup((__le16 *)&coor_data[5]);
		x_angle_u = le16_to_cpup((__le16 *)&coor_data[7]);
		y_angle_u = le16_to_cpup((__le16 *)&coor_data[9]);
		if ((x_angle_u & 0x8000) != 0) /* x_angle_u is nagetive */
			x_angle = (((~x_angle_u) & 0x7fff) + 1) * (-1);
		else /* x_angle_u is positive */
			x_angle = x_angle_u;
		if ((y_angle_u & 0x8000) != 0) /* y_angle_u is nagetive */
			y_angle = (((~y_angle_u) & 0x7fff) + 1) * (-1);
		else /* y_angle_u is positive */
			y_angle = y_angle_u;

		ts_pens->tool.x = x;
		ts_pens->tool.y = y;
		ts_pens->tool.pressure = w;
		ts_pens->tool.tilt_x = x_angle / 100;
		ts_pens->tool.tilt_y = y_angle / 100;
		ts_pens->tool.pen_inrange_status = 1; /* pen inrange */
		ts_pens->tool.tool_type = BTN_TOOL_PEN;
		TS_LOG_DEBUG("gtx8 :p:%02X\n", ts_pens->tool.pressure);

		if (ts_pens->tool.pressure > 0)
			ts_pens->tool.tip_status = 1; /* pen tip touched tp */
		else
			ts_pens->tool.tip_status = 0; /* pen tip not touch tp */

		TS_LOG_DEBUG("gtx8 : touch_data[1]:%02X, key_value = %02X\n",
			touch_data[1],
			touch_data[BYTES_PER_COORD * touch_num + 2]);
		gtx8_stylus_shift_freq_gt7382(coor_data);
		memcpy(&pre_ts_pens, ts_pens, sizeof(struct ts_pens));
	} else {
		ts_pens->tool.x = x;
		ts_pens->tool.y = y;
		ts_pens->tool.tool_type = BTN_TOOL_PEN;
	}
}

/**
 * gtx8_request_evt_handler - handle firmware request
 *
 * @ts: ts_data
 * Returns 0 - succeed,<0 - failed
 */
static int gtx8_request_evt_handler(struct gtx8_ts_data *ts)
{
	u8 rqst_data = 0;
	int ret = NO_ERR;

	ret = gtx8_i2c_read_trans(ts->reg.fw_request, &rqst_data, 1);/*TS_REG_REQUEST*/
	if (ret < 0)
		return ret;

	switch (rqst_data) {
	case REQUEST_CONFIG:
		TS_LOG_INFO("HW request config\n");
		ret = gtx8_send_cfg(&ts->normal_cfg);
		if (ret < 0)
			TS_LOG_ERR("%s: send_cfg fail\n", __func__);
		goto clear_requ;
		break;
	case REQUEST_BAKREF:
		TS_LOG_INFO("HW request bakref\n");
		goto clear_requ;
		break;
	case REQUEST_RESET:
		TS_LOG_INFO("HW requset reset\n");
		goto clear_requ;
		break;
	case REQUEST_MAINCLK:
		TS_LOG_INFO("HW request mainclk\n");
		goto clear_requ;
		break;
	default:
		TS_LOG_INFO("Unknown hw request:%d\n", rqst_data);
		return 0;
	}

clear_requ:
	rqst_data = 0x00;
	ret = gtx8_i2c_write_trans(ts->reg.fw_request, &rqst_data, 1);/*TS_REG_REQUEST*/
	return ret;
}

static void gtx8_set_gesture_key(struct ts_fingers *info, struct ts_cmd_node *out_cmd, enum ts_gesture_num gesture)
{
	info->gesture_wakeup_value = gesture;
	out_cmd->command = TS_INPUT_ALGO;
	out_cmd->cmd_param.pub_params.algo_param.algo_order = gtx8_ts->dev_data->algo_id;
	TS_LOG_DEBUG("%s: ts_double_click evevnt report\n", __func__);
}

#define GTX8_GESTURE_PACKAGE_LEN 4
static int gtx8_gesture_evt_handler(struct gtx8_ts_data *ts,
				struct ts_fingers *info, struct ts_cmd_node *out_cmd)
{
	u8 buf[GTX8_GESTURE_PACKAGE_LEN] = {0};
	int ret = 0;
	unsigned char ts_state = 0;
	unsigned int cmd_reg;
	int retry;

	ts_state = atomic_read(&ts->dev_data->ts_platform_data->state);
	TS_LOG_DEBUG("%s: TS_WORK_STATUS :%d\n", __func__, ts_state);
	if(TS_WORK_IN_SLEEP != ts_state){
		TS_LOG_DEBUG("%s: TS_WORK_STATUS[%d] is mismatch.\n", __func__, ts_state);
		return true;
	}
	if (ts->ic_type == IC_TYPE_7382)
		cmd_reg = GTP_REG_CMD_GT7382;
	else
		cmd_reg = GTP_REG_CMD;
	/** package:
		*  buf[0]: event type, bit5 == 1 represent pen wakeup event
		*  buf[1]: event basic info[currently unused]
		*  buf[2]: event type(0x80 represent pen wakeup event)
		*  buf[4]: checksum
		*/
	if (gtx8_ts->support_retry_read_gesture) {
		for (retry = 0; retry < GTX8_RETRY_NUM_3; retry++) {
			ret = gtx8_i2c_read_trans(ts->reg.gesture, buf,
				GTX8_GESTURE_PACKAGE_LEN);
			if (ret < 0) {
				TS_LOG_ERR("%s: get_reg_wakeup_gesture fail,retry:%d\n", __func__, retry);
				mdelay(10); /* delay 10ms */
			} else {
				break;
			}
		}
		if (retry == GTX8_RETRY_NUM_3)
			goto clear_flag;
	} else {
		ret = gtx8_i2c_read_trans(ts->reg.gesture, buf,
			GTX8_GESTURE_PACKAGE_LEN);
		if (ret < 0) {
			TS_LOG_ERR("%s: get_reg_wakeup_gesture faile\n",
				__func__);
			goto clear_flag;
		}
	}

	if (!(buf[0] & 0x20)) {
		TS_LOG_ERR("%s: invalied easy wakeup event, 0x%02x\n", __func__, buf[0]);
		goto clear_flag;
	}

	if (ts->support_read_gesture_without_checksum)
		goto skip_check;
	/* TODO: may need calculate checksum */
	if (checksum_u8(buf, GTX8_GESTURE_PACKAGE_LEN)) {
		TS_LOG_ERR("%s: easy wakeup event checksum error", __func__);
		goto clear_flag;
	}
skip_check:
	TS_LOG_DEBUG("0x%x = 0x%02X,0x%02X,0x%02X,0x%02X\n", cmd_reg,
		buf[0], buf[1], buf[2], buf[3]);

	switch (buf[2]) {
	case GTX8_PEN_WAKEUP_ENTER_MEMO_EVENT_TYPE: //pen wakeup event
		gtx8_set_gesture_key(info, out_cmd, TS_STYLUS_WAKEUP_TO_MEMO);
		break;
	case GTX8_PEN_WAKEUP_ONLI_SCREEN_ON_EVENT_TYPE:
		gtx8_set_gesture_key(info, out_cmd, TS_STYLUS_WAKEUP_SCREEN_ON);
		break;
	case GTX8_FINGER_DOUBLE_CLICK_EVENT_TYPE:
		if (ts->ic_type == IC_TYPE_7382)
			gtx8_set_gesture_key(info, out_cmd,
				TS_DOUBLE_CLICK);
		else
			gtx8_switch_workmode(GESTURE_MODE);
		break;
	case GTX8_PEN_WAKEUP_ENTER_MEMO_EVENT_TYPE_GT7382H:
		if (ts->ic_type == IC_TYPE_7382)
			gtx8_set_gesture_key(info, out_cmd,
				TS_STYLUS_WAKEUP_TO_MEMO);
		else
			gtx8_switch_workmode(GESTURE_MODE);
		break;
	default:
		TS_LOG_ERR("%s: Warning : unsupport wakeup_event class !!\n", __func__);
		gtx8_switch_workmode(GESTURE_MODE);
		break;
	}

clear_flag:
	buf[0] = 0; // clear gesture event flag
	if (gtx8_i2c_write_trans(ts->reg.gesture, buf, 1))
		TS_LOG_INFO("%s:Failed clear gesture event flag\n", __func__);

	TS_LOG_INFO("%s:easy_wakeup_gesture_event report finish!\n", __func__);
	return NO_ERR;
}

static int gtx8_remap_trace_id_gt7382(
		u8 *coor_buf, u32 coor_buf_len, int touch_num)
{
	static u8 remap_array[20] = { 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff };
	int i;
	int j;
	int offset;
	bool need_leave = false;
	bool need_add = false;
	int max_touch_num = GTP_MAX_TOUCH;

	if (touch_num > GTP_MAX_TOUCH) {
		TS_LOG_INFO("touch num error, trace id no remap:%d", touch_num);
		return 0;
	}

	/* 3: touch head data & tail data */
	if (!coor_buf || coor_buf_len >
		(TOUCH_DATA_LEN_4 + BYTES_PER_COORD * max_touch_num + 3)) {
		TS_LOG_INFO("touch data buff error, !coor_buf:%d, len:%d",
			!coor_buf, coor_buf_len);
		return 0;
	}

	/* clear and reset remap_array */
	if (touch_num == 0) {
		for (i = 0; i < sizeof(remap_array); i++)
			remap_array[i] = 0xff;
		return 0;
	}

	/* scan remap_array, find and remove leave point */
	for (i = 0; i < sizeof(remap_array); i++) {
		if (remap_array[i] == 0xff) {
			continue;
		} else {
			need_leave = true;
			offset = 0;
			for (j = 0; j < touch_num; j++) {
				if (remap_array[i] == coor_buf[offset]) {
					need_leave = false;
					break;
				}
				offset += BYTES_PER_COORD;
			}
			if (need_leave == true)
				remap_array[i] = 0xff;
		}
	}

	/* find and add new point */
	offset = 0;
	for (i = 0; i < touch_num; i++) {
		need_add = true;
		for (j = 0; j < sizeof(remap_array); j++) {
			if (coor_buf[offset] == remap_array[j]) {
				need_add = false;
				break;
			}
		}
		if (need_add == true) {
			for (j = 0; j < sizeof(remap_array); j++) {
				if (remap_array[j] == 0xff) {
					remap_array[j] = coor_buf[offset];
					break;
				}
			}
		}
		offset += BYTES_PER_COORD;
	}

	/* remap trace id */
	offset = 0;
	for (i = 0; i < touch_num; i++) {
		/* do not remap pen's trace ID */
		if (coor_buf[offset] >= 0x80) {
			offset += BYTES_PER_COORD;
			continue;
		} else {
			for (j = 0; j < sizeof(remap_array); j++) {
				if (remap_array[j] == coor_buf[offset]) {
					coor_buf[offset] = j;
					break;
				}
			}
			if (j >= sizeof(remap_array)) {
				TS_LOG_INFO("remap ERROR!!trace id:%d",
					coor_buf[offset]);
				TS_LOG_INFO("remap_array:%*ph",
					(int)sizeof(remap_array), remap_array);
			}
			offset += BYTES_PER_COORD;
		}
	}

	return 0;
}

/**
 * gtx8_touch_evt_handler_gt7382 - handle touch event_gt7382
 * (pen event, key event, finger touch envent)
 * Return <0: failed, 0: succeed
 */
static int gtx8_touch_evt_handler_gt7382(struct gtx8_ts_data  *ts,
	struct ts_cmd_node *out_cmd)
{
	/*
	 * gt7382 touch data max size
	 * Head data : 1 byte
	 * finger data : BYTES_PER_COORD * GTP_MAX_TOUCH (no pen)
	 *                   BYTES_PER_COORD * (GTP_MAX_TOUCH -1) +
	 *                   BYTES_PEN_COORD(have pen)
	 * Tail data:2 bytes
	 */
	u8 buffer[1 + BYTES_PER_COORD * GTP_MAX_TOUCH + 4 + 2] = {0};
	u8 touch_num;
	u8 chksum;
	bool have_pen = false;
	int i;
	int ret;
	static u8 pre_finger_num;
	static u8 pre_pen_num;
	struct ts_fingers *ts_fingers = NULL;
	struct ts_fingers temp_ts_fingers;
	struct ts_pens temp_ts_pens;
	struct ts_pens *ts_pens = NULL;
	struct ts_finger_pen_info *ts_finger_pen = NULL;
	int checksum_len;
	int extra_data = HEAD_LEN + TAIL_LEN; /* touch data head & tail */

	TS_LOG_DEBUG("%s interrupt IN\n", __func__);
	memset(&temp_ts_fingers, 0, sizeof(struct ts_fingers));
	memset(&temp_ts_pens, 0, sizeof(struct ts_pens));
	out_cmd->command = TS_INVAILD_CMD;
	ts_fingers = &out_cmd->cmd_param.pub_params.algo_param.info;
	ts_pens = &out_cmd->cmd_param.pub_params.report_pen_info;
	ts_finger_pen = &out_cmd->cmd_param.pub_params.report_touch_info;

	ret = gtx8_i2c_read_trans(ts->reg.coor,
		buffer, BYTES_PEN_COORD + extra_data);
	if (unlikely(ret))
		goto exit;
	checksum_len = extra_data + BYTES_PEN_COORD;

	if (likely((buffer[0] & GTX8_TOUCH_EVENT) != GTX8_TOUCH_EVENT)) {
		ret = -EINVAL;
		TS_LOG_DEBUG("unknown event,coor status:0x%02x\n", buffer[0]);
		return ret;
	}

	/* buffer[0] & 0x0F: touch num */
	touch_num = buffer[0] & 0x0F;

	/* pen flag */
	if ((buffer[0] & PEN_FLAG) == PEN_FLAG)
		have_pen = true;
	else /* only check finger data */
		checksum_len = BYTES_PER_COORD + extra_data;
	TS_LOG_DEBUG("touch num:%d, have pen:%d, coor status:0x%02x!\n",
		touch_num, have_pen, buffer[0]);

	if (unlikely(touch_num > GTP_MAX_TOUCH)) {
		touch_num = -EINVAL;
		TS_LOG_ERR("touch number:%d\n", touch_num);
		goto exit;
	} else if (unlikely(touch_num > 1)) {
		if (have_pen) {
			/* TS_REG_COORDS_BASE */
			if (touch_num >= GTP_MAX_TOUCH) {
				TS_LOG_ERR("touch num is too large\n");
				touch_num = GTP_MAX_TOUCH - 1;
			}
			ret = gtx8_i2c_read_trans(ts->reg.coor +
				extra_data + BYTES_PEN_COORD,
				&buffer[extra_data + BYTES_PEN_COORD],
				(touch_num - 1) * BYTES_PER_COORD);
			if (unlikely(ret < 0))
				goto exit;
			checksum_len = extra_data + BYTES_PEN_COORD +
				(touch_num - 1) * BYTES_PER_COORD;
		} else {
			/* TS_REG_COORDS_BASE */
			ret = gtx8_i2c_read_trans(ts->reg.coor +
				extra_data + BYTES_PEN_COORD,
				&buffer[extra_data + BYTES_PEN_COORD],
				touch_num * BYTES_PER_COORD - BYTES_PEN_COORD);
			if (unlikely(ret < 0))
				goto exit;
			checksum_len = extra_data + touch_num * BYTES_PER_COORD;
		}
	}
	if (touch_num != 0) {
		chksum = checksum_u8(buffer, checksum_len);
		if (unlikely(chksum != 0)) {
			for (i = 0; i < checksum_len; i++)
				TS_LOG_ERR("[0x%04X]:0x%02X\n",
				ts->reg.coor + i, buffer[i]);
			TS_LOG_ERR("Checksum error:%X\n", chksum);
			ret = -EINVAL;
			goto exit;
		}
	}

	TS_LOG_DEBUG("before remap,w:0x%04x\n", buffer[6] + (buffer[7] << 8));
	if (have_pen)
		gtx8_remap_trace_id_gt7382(&buffer[HEAD_LEN],
			BYTES_PER_COORD * GTP_MAX_TOUCH, touch_num - 1);
	else
		gtx8_remap_trace_id_gt7382(&buffer[HEAD_LEN],
			BYTES_PER_COORD * GTP_MAX_TOUCH, touch_num);

	if (have_pen && (touch_num == 1)) { /* only pen event */
		pre_pen_num = 1;
		goodix_report_pen_gt7382(ts, &temp_ts_pens, buffer, touch_num);
		out_cmd->command = TS_ALGO_FINGER_PEN;
		out_cmd->cmd_param.pub_params.report_touch_info.finger_pen_flag = TS_ONLY_PEN;
	} else if (have_pen && (touch_num > 1)) { /* has finger & pen event */
		goodix_report_finger_gt7382(ts, &temp_ts_fingers,
			buffer, touch_num - 1);
		pre_finger_num = touch_num - 1;

		goodix_report_pen_gt7382(ts, &temp_ts_pens,
			&buffer[(touch_num - 1) * BYTES_PER_COORD], 1);
		pre_pen_num = 1;
		out_cmd->command = TS_ALGO_FINGER_PEN;
		out_cmd->cmd_param.pub_params.report_touch_info.finger_pen_flag = TS_FINGER_PEN;
	} else if ((!have_pen) && touch_num > 0) { /* only finger event */
		goodix_report_finger_gt7382(ts, &temp_ts_fingers,
			buffer, touch_num);
		/* if last event has pen, need report pen up */
		if (pre_pen_num != 0)
			goodix_report_pen_gt7382(ts, &temp_ts_pens,
				&buffer[(touch_num - 1) * BYTES_PER_COORD], 0);
		out_cmd->command = TS_ALGO_FINGER_PEN;
		out_cmd->cmd_param.pub_params.report_touch_info.finger_pen_flag = TS_ONLY_FINGER;
		pre_pen_num = 0;
		pre_finger_num = touch_num;
	} else { /* if last event has pen, need report pen up */
		if (pre_pen_num != 0) {
			goodix_report_pen_gt7382(ts, &temp_ts_pens,
				&buffer[(touch_num - 1) * BYTES_PER_COORD], 0);
			out_cmd->command = TS_ALGO_FINGER_PEN;
		}
		out_cmd->cmd_param.pub_params.report_touch_info.finger_pen_flag = TS_ONLY_PEN;
		pre_pen_num = 0;
		pre_finger_num = 0;
	}
	memcpy(&(ts_finger_pen->report_pen_info),
		&temp_ts_pens, sizeof(ts_finger_pen->report_pen_info));
	memcpy(&(ts_finger_pen->report_finger_info),
		&temp_ts_fingers, sizeof(ts_finger_pen->report_finger_info));
	ts_finger_pen->algo_order = ts->dev_data->algo_id;

	if (ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported) {
		if ((pre_index != cur_index) &&
			((cur_index & pre_index) != cur_index))
			goto exit;
	}
exit:
	return ret;
}

/**
 * gtx8_touch_evt_handler - handle touch event
 * (pen event, key event, finger touch envent)
 * Return <0: failed, 0: succeed
 */
static int gtx8_touch_evt_handler(struct gtx8_ts_data  *ts,
				struct ts_cmd_node *out_cmd)
{
	u8 touch_data[TOUCH_DATA_LEN_4 + BYTES_PER_COORD * GTP_MAX_TOUCH] = {0};
	u8 touch_num = 0, chksum = 0;
	bool have_pen = false;
	int i = 0;
	int ret = NO_ERR;
	static u8 pre_finger_num = 0;
	static u8 pre_pen_num = 0;
	struct ts_fingers *ts_fingers = NULL;
	struct ts_pens *ts_pens = NULL;

	TS_LOG_DEBUG("%s interrupt IN\n", __func__);

	out_cmd->command = TS_INVAILD_CMD;

	ts_fingers = &out_cmd->cmd_param.pub_params.algo_param.info;
	out_cmd->cmd_param.pub_params.algo_param.algo_order = ts->dev_data->algo_id;

	ts_pens = &out_cmd->cmd_param.pub_params.report_pen_info;

	ret = gtx8_i2c_read_trans(ts->reg.coor,
			touch_data, TOUCH_DATA_LEN_4 + BYTES_PER_COORD);
	if (unlikely(ret))
		goto exit;

	if (unlikely((touch_data[0] & GTX8_REQUEST_EVENT) == GTX8_REQUEST_EVENT)) {
		/* hw request */
		gtx8_request_evt_handler(ts);
		ret = -EINVAL;
		goto exit;
	}
	if(unlikely((touch_data[0] & GTX8_TOUCH_EVENT) != GTX8_TOUCH_EVENT)){
		TS_LOG_INFO("TOUCH_EVENT is not correct:%X\n", touch_data[0]);
		goto exit;
	}
	/* buffer[1] & 0x0F: touch num */
	touch_num = touch_data[1] & 0x0F;

	if (unlikely(touch_num > GTP_MAX_TOUCH)) {
		touch_num = -EINVAL;
		goto exit;
	} else if (unlikely(touch_num > 1)) {
		ret = gtx8_i2c_read_trans(ts->reg.coor + TOUCH_DATA_LEN_4 + BYTES_PER_COORD,/*TS_REG_COORDS_BASE*/
				&touch_data[TOUCH_DATA_LEN_4 + BYTES_PER_COORD],
				(touch_num - 1) * BYTES_PER_COORD);
		if (unlikely(ret < 0))
			goto exit;
	}

	chksum = checksum_u8(touch_data,
				touch_num * BYTES_PER_COORD + TOUCH_DATA_LEN_4);
	if (unlikely(chksum != 0)) {
		for (i = 0; i < touch_num * BYTES_PER_COORD + TOUCH_DATA_LEN_4; i++)
			TS_LOG_ERR("[0x%04X]:0x%02X\n", GTP_REG_COOR + i, touch_data[i]);
		TS_LOG_ERR("Checksum error:%X\n", chksum);
		ret = -EINVAL;
		goto exit;
	}

	/*if there is a pen, it's must be the last touch data; if trace id >= 0x80, it's a pen*/
	if(touch_num >= 1 && touch_data[BYTES_PER_COORD * (touch_num - 1) + DMNNY_BYTE_LEN_2] >= 0x80){
		have_pen = true;
		TS_LOG_DEBUG("gtx8-%s: have pen!\n", __func__);
	} else {
		have_pen = false;
	}

	if(have_pen){
		/*firstly,report finger leave*/
		if(pre_finger_num > 0){
			goodix_report_finger(ts, ts_fingers, touch_data, 0);
			out_cmd->command = TS_INPUT_ALGO;
			pre_finger_num = 0;
		} else{
			pre_pen_num = 1;
			goodix_report_pen(ts, ts_pens, touch_data, touch_num);
			out_cmd->command = TS_REPORT_PEN;
		}
	} else{
		if(pre_pen_num > 0){
			goodix_report_pen(ts, ts_pens, touch_data, touch_num);
			out_cmd->command = TS_REPORT_PEN;
			pre_pen_num = 0;
		} else{
			pre_finger_num = touch_num;
			goodix_report_finger(ts, ts_fingers, touch_data, touch_num);
			out_cmd->command = TS_INPUT_ALGO;
		}
	}
exit:
	return ret;
}


static int gtx8_input_pen_config(struct input_dev *input_dev)
{
	struct input_dev *input = input_dev;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (ts == NULL)
		return -ENODEV;

	TS_LOG_INFO("%s called\n", __func__);

	input->evbit[0] |= BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	__set_bit(ABS_X, input->absbit);
	__set_bit(ABS_Y, input->absbit);
	__set_bit(ABS_TILT_X, input->absbit);
	__set_bit(ABS_TILT_Y, input->absbit);
	__set_bit(BTN_STYLUS, input->keybit);
	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(BTN_TOOL_PEN, input->keybit);
	__set_bit(INPUT_PROP_DIRECT, input->propbit);
	__set_bit(TS_STYLUS_WAKEUP_SCREEN_ON, input->keybit);
	if (ts->support_pen_use_max_resolution) {
		input_set_abs_params(input, ABS_X, 0, ts->x_max_rc, 0, 0);
		input_set_abs_params(input, ABS_Y, 0, ts->y_max_rc, 0, 0);
	} else {
		input_set_abs_params(input, ABS_X, 0, ts->max_x, 0, 0);
		input_set_abs_params(input, ABS_Y, 0, ts->max_y, 0, 0);
	}
	input_set_abs_params(input, ABS_PRESSURE, 0, GTX8_MAX_PEN_PRESSURE, 0, 0);
	input_set_abs_params(input, ABS_TILT_X,
		-1 * GTX8_MAX_PEN_TILT, GTX8_MAX_PEN_TILT, 0, 0);
	input_set_abs_params(input, ABS_TILT_Y,
		-1 * GTX8_MAX_PEN_TILT, GTX8_MAX_PEN_TILT, 0, 0);
	return NO_ERR;
}

static int gtx8_irq_bottom_half(struct ts_cmd_node *in_cmd,
				struct ts_cmd_node *out_cmd)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	struct ts_fingers *ts_fingers = NULL;
	u8 sync_val = 0;
	int ret = NO_ERR;

	ts_fingers = &out_cmd->cmd_param.pub_params.algo_param.info;
	TS_LOG_DEBUG("%s: wakeup_gesture_enable:%u,sleep_mode:%d,ts_kit_gesture_func:%d,easy_wakeup_gesture:%d\n",
	 __func__, ts->easy_wakeup_supported,ts->dev_data->easy_wakeup_info.sleep_mode,
	 ts_kit_gesture_func,ts->dev_data->easy_wakeup_info.easy_wakeup_gesture);

	if(ts->easy_wakeup_supported && ts_kit_gesture_func
		&& (ts->dev_data->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE)
		&& (ts->dev_data->easy_wakeup_info.easy_wakeup_gesture != 0)) {
		ret = gtx8_gesture_evt_handler(ts, ts_fingers, out_cmd);
		if(!ret){
			return NO_ERR;
		}
	}
	/* handle touch event
	 * return: <0 - error, 0 - touch event handled,
	 * 		       1 - hw request event handledv */
	if (ts->ic_type == IC_TYPE_7382)
		(void)gtx8_touch_evt_handler_gt7382(ts, out_cmd);
	else
		(void)gtx8_touch_evt_handler(ts, out_cmd);

	/*sync_evt:*/
	if (!ts->rawdiff_mode) {
		ret = gtx8_i2c_write_trans(ts->reg.coor, &sync_val, sizeof(sync_val));
		if (ret < 0)
			TS_LOG_ERR("%s: clean irq flag failed\n", __func__);
	} else {
		TS_LOG_DEBUG("%s: Firmware rawdiff mode\n", __func__);
	}
	return NO_ERR;
}

/**
 * gtx8_resume_chip_reset - LCD resume reset chip
 */
static void gtx8_resume_chip_reset(void)
{
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int reset_gpio = 0;

	reset_gpio = gtx8_ts->dev_data->ts_platform_data->reset_gpio;
#endif
	TS_LOG_INFO("%s: Chip reset\n", __func__);

#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	pinctrl_select_state(gtx8_ts->pinctrl,
		gtx8_ts->pinctrl_state_reset_low);
#else
	gpio_direction_output(reset_gpio, 0);
#endif
	//udelay(GTX8_RESET_PIN_D_U_TIME);
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		mdelay(10); /* ic need */
	else
		udelay(2000); /* ic need */
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	pinctrl_select_state(gtx8_ts->pinctrl,
		gtx8_ts->pinctrl_state_reset_high);
#else
	gpio_direction_output(reset_gpio, 1);
#endif
}

/**
 * gtx8_chip_reset - reset chip
 */
int gtx8_chip_reset(void)
{
	u32 stylus3_type_info;
	u32 stylus3_status;
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int reset_gpio = gtx8_ts->dev_data->ts_platform_data->reset_gpio;
#endif
	TS_LOG_INFO("%s: Chip reset\n", __func__);
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	pinctrl_select_state(gtx8_ts->pinctrl,
		gtx8_ts->pinctrl_state_reset_low);
#else
	gpio_direction_output(reset_gpio, 0);
#endif
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		mdelay(10); /* ic need */
	else
		udelay(2000); /* ic need */
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	pinctrl_select_state(gtx8_ts->pinctrl,
		gtx8_ts->pinctrl_state_reset_high);
#else
	gpio_direction_output(reset_gpio, 1);
#endif
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		if (gtx8_ts->poweron_flag == true) {
#ifdef CONFIG_HUAWEI_DEVKIT_QCOM_3_0
			msleep(GT73X_QCOM_AFTER_RESET_DELAY);
#else
			msleep(GT73X_AFTER_REST_DELAY);
#endif
			gtx8_ts->poweron_flag = false;
		} else {
			msleep(GT73X_AFTER_REST_DELAY_440);
		}
	} else {
		msleep(GTX8_AFTER_REST_DELAY);
	}
	gtx8_ts_esd_init(gtx8_ts);
	if ((gtx8_ts->ic_type == IC_TYPE_7382) &&
		gtx8_ts->dev_data->send_stylus_type_to_fw) {
		stylus3_type_info =
			gtx8_ts->dev_data->ts_platform_data->current_stylus3_type_info;
		stylus3_status = atomic_read(&(g_ts_kit_platform_data.current_stylus3_status));
		stylus3_status &= SYTULS_VALUE;
		if (stylus3_status)
			stylus3_type_info_handle(stylus3_type_info);
	}
	return 0;
}

/**
 * gtx8_fw_update_boot - update firmware while booting
 */
static int gtx8_fw_update_boot(char *file_name)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	if (!file_name) {
		TS_LOG_ERR("%s: Invalid file name\n", __func__);
		return -ENODEV;
	}

	memset(update_ctrl.fw_name, 0, sizeof(update_ctrl.fw_name));
	if (ts->support_filename_contain_lcd_module)
		snprintf(update_ctrl.fw_name, GTX8_FW_NAME_LEN,
				"ts/%s%s_%s_%s.img",
				file_name, ts->project_id,
				ts->dev_data->module_name,
				gtx8_ts->lcd_module_name);
	else
		snprintf(update_ctrl.fw_name, GTX8_FW_NAME_LEN,
				"ts/%s%s_%s.img",
				file_name, ts->project_id,
				ts->dev_data->module_name);

	TS_LOG_INFO("%s: file_name = %s\n", __func__, update_ctrl.fw_name);
	return gtx8_update_firmware();
}

/**
 * gtx8_fw_update_sd - update firmware from sdcard
 *  firmware path should be '/sdcard/gtx8_fw.bin'
 */
static int gtx8_fw_update_sd(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	int ret = NO_ERR;

	TS_LOG_INFO("%s: start ...\n", __func__);
	gtx8_get_cfg_parms(ts, GTX8_CONFIG_FW_SD_NAME);
	msleep(100);
	memset(update_ctrl.fw_name, 0, sizeof(update_ctrl.fw_name));
	strncpy(update_ctrl.fw_name, GTX8_FW_SD_NAME, GTX8_FW_NAME_LEN);
	update_ctrl.force_update = true;
	TS_LOG_INFO("%s: file_name = %s\n", __func__, update_ctrl.fw_name);
	ret = gtx8_update_firmware();
	update_ctrl.force_update = false;

	return ret;
}

static int gtx8_chip_get_info(struct ts_chip_info_param *info)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	int len = 0;
	if (!info) {
		TS_LOG_ERR("%s: info is null\n", __func__);
		return -EINVAL;
	}

	memset(info->ic_vendor, 0, sizeof(info->ic_vendor));
	memset(info->mod_vendor, 0, sizeof(info->mod_vendor));
	memset(info->fw_vendor, 0, sizeof(info->fw_vendor));

	if (!ts->dev_data->ts_platform_data->hide_plain_id) {
		len = (sizeof(info->ic_vendor) - 1) > sizeof(GTX8_OF_NAME) ?
			sizeof(GTX8_OF_NAME) : (sizeof(info->ic_vendor) - 1);
		strncpy(info->ic_vendor, GTX8_OF_NAME, len);
	} else {
		len = (sizeof(info->ic_vendor) - 1) > sizeof(ts->project_id) ?
			sizeof(ts->project_id) : (sizeof(info->ic_vendor) - 1);
		strncpy(info->ic_vendor, ts->project_id, len);
	}
	len = (sizeof(info->mod_vendor) - 1) > strlen(ts->project_id) ?
		strlen(ts->project_id) : (sizeof(info->mod_vendor) - 1);
	strncpy(info->mod_vendor, ts->project_id, len);

	if (!ts->delete_insignificant_chip_info)
		snprintf(info->fw_vendor, sizeof(info->fw_vendor) - 1,
			"%s-0x%02X--bin_normal-%d", ts->dev_data->module_name,
			getU32(ts->hw_info.vid), ts->normal_cfg.data[0]);
	else
		snprintf(info->fw_vendor, sizeof(info->fw_vendor) - 1,
			"%s-0x%02X-%d", ts->dev_data->module_name,
			getU32(ts->hw_info.vid), ts->normal_cfg.data[0]);

	return NO_ERR;
}

static int gtx8_chip_suspend(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
	int tskit_pt_station_flag = 0;
	int retval = 0;

	if((tp_ops)&&(tp_ops->get_status_by_type)) {
		retval = tp_ops->get_status_by_type(PT_STATION_TYPE, &tskit_pt_station_flag);
		if(retval < 0) {
			TS_LOG_ERR("%s: get tskit_pt_station_flag fail\n", __func__);
			return retval;
		}
	}
	TS_LOG_INFO(" %s enter\n", __func__);

	switch (ts->dev_data->easy_wakeup_info.sleep_mode) {
	case TS_POWER_OFF_MODE:
		gtx8_pinctrl_select_suspend();
		if (!tskit_pt_station_flag) {
			TS_LOG_INFO("%s: enter power_off mode\n", __func__);
			gtx8_power_switch(SWITCH_OFF);
		} else {
			TS_LOG_INFO("%s: enter sleep mode\n", __func__);
			gtx8_switch_workmode(SLEEP_MODE); /*sleep mode*/
		}
		break;
		/*for gesture wake up mode suspend. */
	case TS_GESTURE_MODE:
		if(!ts->easy_wakeup_supported){
			TS_LOG_ERR("%s: disable easy_wakeup, enter deep sleep!!\n", __func__);
			gtx8_pinctrl_select_suspend();
			gtx8_switch_workmode(SLEEP_MODE);
		}else{
			supend_stylus_command_sent();
			TS_LOG_INFO("%s: enter gesture mode\n", __func__);
			gtx8_pinctrl_select_gesture();
			gtx8_switch_workmode(GESTURE_MODE);
		}
		break;
	default:
		TS_LOG_INFO("%s: default enter sleep mode\n", __func__);
		gtx8_pinctrl_select_suspend();
		gtx8_switch_workmode(SLEEP_MODE);
		break;
	}

	TS_LOG_INFO("%s: exit\n", __func__);
	return NO_ERR;
}

static int gtx8_chip_resume(void)
{
	struct gtx8_ts_data *ts = gtx8_ts;
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
	int tskit_pt_station_flag = 0;
	int retval = 0;

	if((tp_ops)&&(tp_ops->get_status_by_type)) {
		retval = tp_ops->get_status_by_type(PT_STATION_TYPE, &tskit_pt_station_flag);
		if(retval < 0) {
			TS_LOG_ERR("%s: get tskit_pt_station_flag fail\n", __func__);
			return retval;
		}
	}

	TS_LOG_INFO("%s: enter\n", __func__);
	switch (ts->dev_data->easy_wakeup_info.sleep_mode) {
	case TS_POWER_OFF_MODE:
		gtx8_pinctrl_select_normal();
		if (!tskit_pt_station_flag)
			gtx8_power_switch(SWITCH_ON);
		gtx8_resume_chip_reset();
		break;
	/* gesture mode need set pinctrl, needn't break */
	case TS_GESTURE_MODE:
		if (gtx8_ts->ic_type == IC_TYPE_7382) {
			gtx8_pinctrl_select_normal();
			gtx8_resume_chip_reset();
			break;
		}
	default:
		gtx8_pinctrl_select_normal();
		gtx8_resume_chip_reset();
		break;
	}

	TS_LOG_INFO(" %s:exit\n", __func__);
	return NO_ERR;
}

static int gtx8_chip_after_resume(void *feature_info)
{
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
	int tskit_pt_station_flag = 0;
	int retval;
	unsigned int stylus3_status;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	u32 stylus3_type_info;

	TS_LOG_INFO("%s: enter\n", __func__);

	//msleep(GTX8_RESET_SLEEP_TIME);
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		if ((tp_ops) && (tp_ops->get_status_by_type)) {
			retval = tp_ops->get_status_by_type(PT_STATION_TYPE,
				&tskit_pt_station_flag);
			if (retval < 0) {
				TS_LOG_ERR("%s: get tskit_pt_station_flag fail\n",
					__func__);
				return retval;
			}
		}
		if ((gtx8_ts->dev_data->easy_wakeup_info.sleep_mode == TS_POWER_OFF_MODE) &&
			(!tskit_pt_station_flag))
			msleep(GT73X_AFTER_REST_DELAY);
		else
			msleep(GT73X_AFTER_REST_DELAY_GESTURE_MODE);
	} else {
		msleep(GTX8_AFTER_REST_DELAY);
	}
	gtx8_ts_esd_init(gtx8_ts);
	gtx8_ts->rawdiff_mode = false;
	gtx8_feature_resume(gtx8_ts);
	TS_LOG_INFO("%s bin_normal_cfg-%d \n", __func__, gtx8_ts->normal_cfg.data[0]);

	if ((gtx8_ts->ic_type == IC_TYPE_7382) && (dev->send_bt_status_to_fw)) {
		stylus3_status = atomic_read(&(g_ts_kit_platform_data.current_stylus3_status));
		stylus3_status = (stylus3_status & SYTULS_VALUE);
		TS_LOG_INFO("%s resume_get_stylus3_status: %u\n", __func__, stylus3_status);
		if (stylus3_status == SYTULS_DISCONNECT_STATUS)
			stylus3_connect_state(stylus3_status);

		mdelay(SYTULS_DELAY);
	}
	if ((gtx8_ts->ic_type == IC_TYPE_7382) &&
		dev->send_stylus_type_to_fw) {
		stylus3_status = atomic_read(&(g_ts_kit_platform_data.current_stylus3_status));
		stylus3_type_info =
			dev->ts_platform_data->current_stylus3_type_info;
		stylus3_status &= SYTULS_VALUE;
		if (stylus3_status)
			stylus3_type_info_handle(stylus3_type_info);
	}
	if ((gtx8_ts->ic_type == IC_TYPE_7382) &&
		(gtx8_ts->dev_data->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE))
		gtx8_touch_switch();
	return NO_ERR;
}

/**
 * gtx8_glove_switch - switch to glove mode
 */
static int gtx8_glove_switch(struct ts_glove_info *info)
{
	static bool glove_en = false;
	int ret = NO_ERR;
	u8 buf = 0;

	if (!info || !info->glove_supported) {
		TS_LOG_ERR("%s: info is Null or no support glove mode\n", __func__);
		return -EINVAL;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		if (glove_en)
			info->glove_switch = 1;
		else
			info->glove_switch = 0;
		break;
	case TS_ACTION_WRITE:
		if (info->glove_switch) {
			/* enable glove feature */
			ret = gtx8_feature_switch(gtx8_ts,
					TS_FEATURE_GLOVE, SWITCH_ON);
			if (!ret)
				glove_en = true;
		} else {
			/* disable glove feature */
			ret = gtx8_feature_switch(gtx8_ts,
					TS_FEATURE_GLOVE, SWITCH_OFF);
			if (!ret)
				glove_en = false;
		}

		if (ret < 0)
			TS_LOG_ERR("%s:set glove switch(%d), failed : %d\n", __func__, buf, ret);
		break;
	default:
		TS_LOG_ERR("%s: invalid switch status: %d\n", __func__, info->glove_switch);
		ret = -EINVAL;
		break;
	}

	return ret;
}

void gtx8_pen_respons_mode_change(int soper)
{
	int ret = -EINVAL;
	switch (soper){
		case TS_SWITCH_PEN_RESPON_FAST:
			ret = gtx8_send_cmd(GTX8_CMD_STYLUS_RESPONSE_MODE,\
								TS_SWITCH_PEN_RESPON_FAST_CMD_DATA,\
								GTX8_NOT_NEED_SLEEP);
			if (gtx8_ts->ic_type == IC_TYPE_7382) {
				ts_update_stylu3_shif_freq_flag(STYLUS3_APP_SWITCH,
					true);
				TS_LOG_INFO("%s pen get in app mode\n",
					__func__);
			}
			if (ret < 0)
				TS_LOG_ERR("pen respon fast mode error \n");
			else
				TS_LOG_INFO("[%s] pen respon fast mode sucess\n",__func__);
			break;
		case TS_SWITCH_PEN_RESPON_NORMAL:
			if (gtx8_ts->ic_type == IC_TYPE_7382) {
				ret = gtx8_send_cmd(
					GTX8_CMD_PEN_NORMAL_MODE_7382,
					TS_SWITCH_PEN_NORMAL_CMD_DATA_7382,
					GTX8_NOT_NEED_SLEEP);
				ts_update_stylu3_shif_freq_flag(STYLUS3_APP_SWITCH,
					false);
				TS_LOG_INFO("%s pen get out app mode\n",
					__func__);
			} else {
				ret = gtx8_send_cmd(
					GTX8_CMD_STYLUS_RESPONSE_MODE,
					TS_SWITCH_PEN_RESPON_NORMAL_CMD_DATA,
					GTX8_NOT_NEED_SLEEP);
			}
			if (ret < 0)
				TS_LOG_ERR("pen respon normal mode	error \n");
			else
				TS_LOG_INFO("[%s] pen respon normal mode sucess\n",__func__);
			break;
		default:
			TS_LOG_ERR("soper unknown:%d, invalid\n", soper);
			break;
	}
	return;
}
void gtx8_game_switch(int soper)
{
	int ret = -EINVAL;

	switch (soper){
		case TS_SWITCH_GAME_ENABLE:
			ret = gtx8_feature_switch(gtx8_ts,TS_FEATURE_GAME, SWITCH_ON);
			if (ret < 0 )
				TS_LOG_ERR("game switch on error \n");
			else{
				gtx8_ts->game_switch = true;
				TS_LOG_INFO("[%s] game switch on:%d \n",__func__,gtx8_ts->game_switch);
			}
			break;
		case TS_SWITCH_GAME_DISABLE:
			ret = gtx8_feature_switch(gtx8_ts,TS_FEATURE_GAME, SWITCH_OFF);
			if (ret < 0 )
				TS_LOG_ERR("game switch off error \n");
			else{
				gtx8_ts->game_switch = false;
				TS_LOG_INFO("[%s] game switch off:%d\n",__func__, gtx8_ts->game_switch);
			}
			break;
		default:
			TS_LOG_ERR("soper unknown:%d, invalid\n", soper);
			break;
	}
	return;
}

/**
 * gtx8_game_mode_switch - switch to game mode
 */
//TODO:ts_game_info to be modified
#define GTX8_SWITCH_TYPE_SCENE  TS_SWITCH_TYPE_SCENE
#define GTX8_SWITCH_TYPE_GAME   5
static void gtx8_touch_switch(void)
{
	unsigned long get_value = 0;
	char *ptr_begin = NULL, *ptr_end = NULL;
	char in_data[MAX_STR_LEN] = {0};
	int len = 0;
	unsigned char stype = 0, soper = 0, param = 0;
	int error = 0;

	TS_LOG_INFO("%s +\n", __func__);
	//follow other old prj touch_switch_flag is function support flag define in overlay files
	//bit0 :doze   bit1:game  bit2:secne (old poj)or pen repons (in this poj)
	if (!(gtx8_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_DOZE) &&
		!(gtx8_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_GAME) &&
		!(gtx8_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_SCENE) ) {//pen respons
		TS_LOG_ERR("touch switch not supprot\n");
		goto out;
	}

	/* SWITCH_OPER,ENABLE_DISABLE,PARAM */
	memcpy(in_data, gtx8_ts->dev_data->touch_switch_info, MAX_STR_LEN -1);
	TS_LOG_INFO("in_data:%s\n", in_data);

	/* get switch type */
	ptr_begin = in_data;
	ptr_end = strstr(ptr_begin, ",");
	if (!ptr_end){
		TS_LOG_ERR("%s get stype fail\n", __func__);
		goto out;
	}
	len = ptr_end - ptr_begin;
	if (len <= 0 || len > 3){
		TS_LOG_ERR("%s stype len error\n", __func__);
		goto out;
	}
	*ptr_end = 0;
	error = kstrtoul(ptr_begin, 0, &get_value);
	if (error) {
		TS_LOG_ERR("kstrtoul return invaild :%d\n", error);
		goto out;
	}else{
		stype = (unsigned char)get_value;
		TS_LOG_INFO("%s get stype:%u\n", __func__, stype);
	}

	/* get switch operate */
	ptr_begin = ptr_end + 1;
	if (!ptr_begin){
		TS_LOG_ERR("%s get soper fail\n", __func__);
		goto out;
	}
	ptr_end = strstr(ptr_begin, ",");
	if (!ptr_end){
		TS_LOG_ERR("%s get soper fail\n", __func__);
		goto out;
	}
	len = ptr_end - ptr_begin;
	if (len <= 0 || len > 3){
		TS_LOG_ERR("%s soper len error\n", __func__);
		goto out;
	}
	*ptr_end = 0;
	error = kstrtoul(ptr_begin, 0, &get_value);
	if (error) {
		TS_LOG_ERR("kstrtoul return invaild :%d\n", error);
		goto out;
	}else{
		soper = (unsigned char)get_value;
		TS_LOG_INFO("%s get soper:%u\n", __func__, soper);
	}

	/* get param */
	ptr_begin = ptr_end + 1;
	if (!ptr_begin){
		TS_LOG_ERR("%s get soper fail\n", __func__);
		goto out;
	}
	error = kstrtoul(ptr_begin, 0, &get_value);
	if (error) {
		TS_LOG_ERR("kstrtoul return invaild :%d\n", error);
		goto out;
	}else{
		param = (unsigned char)get_value;
		TS_LOG_INFO("%s get param:%u\n", __func__, param);
	}

	if(stype == GTX8_SWITCH_TYPE_SCENE){
		gtx8_pen_respons_mode_change(soper);
	}else if (stype == GTX8_SWITCH_TYPE_GAME){
		gtx8_game_switch(soper);
	}else{
		TS_LOG_ERR("stype not GAME or pen respons :%d, invalid\n", stype);
	}

out:
	return ;
}

static void gtx8_chip_shutdown(void)
{
	gtx8_power_switch(SWITCH_OFF);
}


static int gtx8_holster_switch(struct ts_holster_info *info)
{
	int ret = NO_ERR;

	if (!info || !info->holster_supported) {
		TS_LOG_ERR("%s: info is Null or no support holster mode\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		if (info->holster_switch)
			ret = gtx8_feature_switch(gtx8_ts,
				TS_FEATURE_HOLSTER, SWITCH_ON);
		else
			ret = gtx8_feature_switch(gtx8_ts,
				TS_FEATURE_HOLSTER, SWITCH_OFF);
		if (ret < 0)
			TS_LOG_ERR("%s:set holster switch(%d), failed: %d\n",
				__func__, info->holster_switch, ret);
		break;
	case TS_ACTION_READ:
		TS_LOG_INFO("%s: invalid holster switch(%d) action: TS_ACTION_READ\n",
				__func__, info->holster_switch);
		break;
	default:
		TS_LOG_INFO("%s: invalid holster switch(%d) action: %d\n",
				__func__, info->holster_switch, info->op_action);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int gtx8_roi_switch(struct ts_roi_info *info)
{
	int ret = 0;

	if (!info || !info->roi_supported ||!gtx8_ts) {
		TS_LOG_ERR("%s: info is Null or not supported roi mode\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		if (info->roi_switch == 1) {
			gtx8_ts->roi.enabled = true;
		} else if (info->roi_switch == 0) {
			gtx8_ts->roi.enabled = false;
		} else {
			TS_LOG_ERR("%s:Invalid roi switch value:%d\n", __func__, info->roi_switch);
			ret = -EINVAL;
		}
		break;
	case TS_ACTION_READ:
		// read
		break;
	default:
		break;
	}
	return ret;
}

static u8 *gtx8_roi_rawdata(void)
{
	u8 *rawdata_ptr = NULL;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (!ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported || !ts->roi.rawdata){
		TS_LOG_ERR("%s : Not supported roi mode\n", __func__);
		return NULL;
	}
	if (wait_for_completion_interruptible_timeout(&roi_data_done, msecs_to_jiffies(30))) {
		mutex_lock(&gtx8_ts->roi.mutex);
		if (gtx8_ts->roi.enabled && gtx8_ts->roi.data_ready) {
			rawdata_ptr = (u8 *)gtx8_ts->roi.rawdata;
		} else {
			TS_LOG_ERR("%s : roi is not enable or roi data not ready\n", __func__);
		}
		mutex_unlock(&gtx8_ts->roi.mutex);
	} else {
		TS_LOG_ERR("%s : wait roi_data_done timeout! \n", __func__);
	}

	return rawdata_ptr;
}

static int gtx8_check_hw_status(void)
{
	u8 esd_state = 0;
	int ret = -1, i = 0;
	static char tmp_reg_buf[GTP_REG_LEN_45] = {0};
	static int tri_cnt = 0;
	int read_reg_ret;
	int read_debug_reg_len;
	int j;
	u16 read_debug_msg_reg;

	tri_cnt += 1;
	if (!gtx8_ts->tool_esd_disable) {
		if (gtx8_ts->ic_type == IC_TYPE_7382) {
			/* static esd check */
			for (j = 0; j < 3; j++) {
				ret = gtx8_ts->ops.read_trans(GTP_REG_ESD_TICK_R_GT7382,
					&esd_state, 1);
				if (ret < 0)
					break;
				/* 0xaa, esd status flag */
				if (esd_state == 0xaa)
					break;
			}
			if (ret || (j >= 3)) {
				TS_LOG_ERR("HW status:Error,static{8043h}=%02X\n",
					esd_state);
				ret = -EIO;
			}
			goto skip_dynamic_esd_check;
		}
		for (i = 0; i < 3; i++) {
			ret = gtx8_ts->ops.read_trans(GTP_REG_ESD_TICK_R, &esd_state, 1);
			if (ret < 0)
				break;
			if (esd_state != 0xaa)
				break;
		}

		if (likely(!ret && i < 3)) {
			TS_LOG_DEBUG(" HW status:OK\n");
			/* IC works normally, feed the watchdog */
			esd_state = 0xaa;
			ret = gtx8_ts->ops.write_trans(GTP_REG_ESD_TICK_W, &esd_state, 1);
		} else {
			TS_LOG_ERR("HW status:Error,reg{3103h}=%02X\n", esd_state);
			ret = -EIO;
		}

skip_dynamic_esd_check:
		if (gtx8_ts->support_read_more_debug_msg)
			read_debug_reg_len = GTP_REG_LEN_9;
		else
			read_debug_reg_len = GTP_REG_LEN_6;
		if (gtx8_ts->ic_type == IC_TYPE_7382)
			read_debug_msg_reg = GTP_GRE_STATUS_GT7382;
		else
			read_debug_msg_reg = GTP_REG_STATUS;
		if (!gtx8_ts->support_checked_esd_reset_chip) {
			ret = gtx8_i2c_read_trans(read_debug_msg_reg,
				tmp_reg_buf +
				(tri_cnt - 1) * read_debug_reg_len,
				read_debug_reg_len);
			if (ret < 0)
				TS_LOG_ERR("Read Reg status ERR!\n");
		} else {
			read_reg_ret = gtx8_i2c_read_trans(read_debug_msg_reg,
				tmp_reg_buf + (tri_cnt - 1) * read_debug_reg_len,
				read_debug_reg_len);
			if (read_reg_ret < 0)
				TS_LOG_ERR("Read Reg status ERR!\n");
		}
		if (WD_TRI_TIMES_5 == tri_cnt){
			for(i = 0; i < WD_TRI_TIMES_5; i++){
				if (gtx8_ts->ic_type == IC_TYPE_7382)
					/* 1,2,3,4,5,6,7,8, debug reg index */
					TS_LOG_INFO("%dS:[FF00]:0x%02X, [FF01]:0x%02X, [FF02]:0x%02X, [FF03]:0x%02X, [FF04]:0x%02X, [FF05]:0x%02X\n",
					i,
					tmp_reg_buf[read_debug_reg_len * i],
					tmp_reg_buf[read_debug_reg_len * i + 1],
					tmp_reg_buf[read_debug_reg_len * i + 2],
					tmp_reg_buf[read_debug_reg_len * i + 3],
					tmp_reg_buf[read_debug_reg_len * i + 4],
					tmp_reg_buf[read_debug_reg_len * i + 5]);
				else
					/* 1,2,3,4,5,6,7,8, debug reg index */
					TS_LOG_INFO("%dS:[4B64]:0x%02X, [4B65]:0x%02X, [4B66]:0x%02X, [4B67]:0x%02X, [4B68]:0x%02X, [4B69]:0x%02X\n",
						i,
						tmp_reg_buf[read_debug_reg_len * i],
						tmp_reg_buf[read_debug_reg_len * i + 1],
						tmp_reg_buf[read_debug_reg_len * i + 2],
						tmp_reg_buf[read_debug_reg_len * i + 3],
						tmp_reg_buf[read_debug_reg_len * i + 4],
						tmp_reg_buf[read_debug_reg_len * i + 5]);
				if (gtx8_ts->support_read_more_debug_msg) {
					if (gtx8_ts->ic_type == IC_TYPE_7382)
						TS_LOG_INFO("%dS:[FF06]:0x%02X, [FF07]:0x%02X, [FF08]:0x%02X\n",
							i,
							tmp_reg_buf[read_debug_reg_len * i + 6],
							tmp_reg_buf[read_debug_reg_len * i + 7],
							tmp_reg_buf[read_debug_reg_len * i + 8]);
					else
						TS_LOG_INFO("%dS:[4B6A]:0x%02X, [4B6B]:0x%02X, [4B6C]:0x%02X\n",
							i,
							tmp_reg_buf[read_debug_reg_len * i + 6],
							tmp_reg_buf[read_debug_reg_len * i + 7],
							tmp_reg_buf[read_debug_reg_len * i + 8]);
				}
			}
			tri_cnt = 0;
			memset(tmp_reg_buf, 0, sizeof(tmp_reg_buf));
		}
	} else {
		TS_LOG_DEBUG("%s: esd check closed by gtp tool\n", __func__);
		ret = NO_ERR;
	}
	return ret;
}

static int gtx8_chip_get_capacitance_test_type(
				struct ts_test_type_info *info)
{
	int ret = NO_ERR;

	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		memcpy(info->tp_test_type,
				gtx8_ts->dev_data->tp_test_type,
				TS_CAP_TEST_TYPE_LEN - 1);
		TS_LOG_INFO("%s: test_type=%s\n", __func__, info->tp_test_type);
		break;
	case TS_ACTION_WRITE:
		break;
	default:
		TS_LOG_ERR("%s: invalid status: %s\n", __func__, info->tp_test_type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*******************************************************************************
* Function Name  : gtx8_get_cfg_value
* Description    : read config data specified by sub-bag number and inside-bag offset.
* config	 : pointer to config data
* buf		 : output buffer
* len		 : data length want to read, if len = 0, get full bag data
* sub_bag_num    : sub-bag number
* offset         : offset inside sub-bag
* Return         : u8(return data len success read from config. < 0 failed)
*******************************************************************************/
extern u8 gtx8_get_cfg_value(u8 *config, u8 *buf, u8 len, u8 sub_bag_num, u8 offset ,u8 buf_size)
{
	u8 *sub_bag_ptr = NULL;
	u8 i = 0;
	if(!config ||!buf ){
		TS_LOG_ERR("%s:invalid input\n", __func__);
		return -EINVAL;
	}
	sub_bag_ptr = &config[4];
	for (i = 0; i < config[2]; i++) {
		if (sub_bag_ptr[0] == sub_bag_num)
			break;
		sub_bag_ptr += sub_bag_ptr[1] + 3;
	}

	if (i >= config[2]) {
		TS_LOG_ERR("Cann't find the specifiled bag num %d\n", sub_bag_num);
		return -EINVAL;
	}

	if (sub_bag_ptr[1] < offset + len) {
		TS_LOG_ERR("Sub bag len less then you want to read: %d < %d\n", sub_bag_ptr[1], offset + len);
		return -EINVAL;
	}

	if (len)
		memcpy(buf, sub_bag_ptr + offset,(len > buf_size ? buf_size : len));
	//else
	//memcpy(buf, sub_bag_ptr, sub_bag_ptr[1] + 3);
	return len;
}

/**
 * Function Name: gt7382_get_cfg_value
 * Description: read config data specified by sub-bag number and
 * inside-bag offset.
 * config : pointer to config data
 * buf : output buffer
 * len : data length want to read, if len = 0, get full bag data
 * addr : data addr
 * Return : u8(return data len success read from config. < 0 failed)
 */
static u8 gt8x_get_cfg_value_gt7382(u8 *config,
	u8 *buf, u8 len, u16 addr)
{
	u8 *ptr = NULL;
	struct gtx8_ts_data *ts = gtx8_ts;

	if (addr < ts->reg.cfg_addr) {
		TS_LOG_ERR("%s, addr invalid\n", __func__);
		return 0;
	}

	ptr = config + addr - ts->reg.cfg_addr;
	if (len)
		(void)memcpy(buf, ptr, len);
	else
		TS_LOG_INFO("get cfg value len is 0\n");
	return len;
}

/*
 * parse driver and sensor num only on screen
 */
 #define GET_CHANNEL_BUF_SIZE   3
int gtx8_get_channel_num_gt7382(u32 *sen_num,
	u32 *drv_num, u8 *cfg_data)
{
	int ret;
	u8 buf[GET_CHANNEL_BUF_SIZE] = {0};
	u8 *temp_cfg_data = cfg_data;

	if (!sen_num || !drv_num || !temp_cfg_data) {
		TS_LOG_ERR("%s: invalid param\n", __func__);
		return -EINVAL;
	}

	ret = gt8x_get_cfg_value_gt7382(temp_cfg_data, buf,
		GET_CHANNEL_BUF_SIZE, GT8X_7382_DRV_ADDR);
	if (ret > 0) {
		*drv_num = buf[0] + buf[2]; /* drv num offset */
	} else {
		TS_LOG_ERR("Failed read drv_num reg\n");
		ret = -EINVAL;
		goto err_out;
	}

	ret = gt8x_get_cfg_value_gt7382(temp_cfg_data, buf,
		GET_CHANNEL_BUF_SIZE, GT8X_7382_SEN_ADDR);
	if (ret > 0) {
		*sen_num = buf[0] + buf[2]; /* sen num offset */
	} else {
		TS_LOG_ERR("Failed read sen_num reg\n");
		ret = -EINVAL;
		goto err_out;
	}

	TS_LOG_INFO("drv_num:%d,sen_num:%d\n", *drv_num, *sen_num);

	if ((*drv_num > MAX_DRV_NUM_7382) || (*drv_num <= 0) ||
		(*sen_num > MAX_SEN_NUM_7382) || (*sen_num <= 0)) {
		TS_LOG_ERR("invalid sensor or driver num\n");
		ret = -EINVAL;
	}

err_out:
	return ret < 0 ? ret : 0;
}

int gtx8_get_channel_num(u32 *sen_num, u32 *drv_num, u8 *cfg_data)
{
	int ret = 0;
	u8 buf[GET_CHANNEL_BUF_SIZE] = {0};
	u8 *temp_cfg_data = cfg_data;
	u8 offset = GT8X_9886_SEN_OFFSET;
	u32 max_sen_num;
	u32 max_drv_num;

	if (!sen_num || !drv_num || !temp_cfg_data) {
		TS_LOG_ERR("%s: invalid param\n", __func__);
		return -EINVAL;
	}

	ret = gtx8_get_cfg_value(temp_cfg_data, buf, 2, 1, 14,GET_CHANNEL_BUF_SIZE );
	if (ret > 0) {
		*drv_num = buf[0] + buf[1];
	} else {
		TS_LOG_ERR("Failed read drv_num reg\n");
		ret = -EINVAL;
		goto err_out;
	}

	if (gtx8_ts->ic_type == IC_TYPE_6861)
		offset = GT8X_6861_SEN_OFFSET;
	ret = gtx8_get_cfg_value(temp_cfg_data, buf, 1, 1, offset,
			GET_CHANNEL_BUF_SIZE);
	if (ret > 0) {
		*sen_num = buf[0];
	} else {
		TS_LOG_ERR("Failed read sen_num reg\n");
		ret = -EINVAL;
		goto err_out;
	}

	TS_LOG_INFO("drv_num:%d,sen_num:%d\n", *drv_num, *sen_num);

	if (gtx8_ts->ic_type == IC_TYPE_6861) {
		max_sen_num = ACTUAL_RX_NUM_6861;
		max_drv_num = ACTUAL_TX_NUM_6861;
	} else {
		max_sen_num = gtx8_ts->max_sen_num;
		max_drv_num = gtx8_ts->max_drv_num;
	}
	if ((*drv_num > max_drv_num) || (*drv_num <= 0) ||
		(*sen_num > max_sen_num) || (*sen_num <= 0)) {
		TS_LOG_ERR("invalid sensor or driver num\n");
		ret = -EINVAL;
	}

err_out:
	return ret < 0 ? ret : 0;
}

static int gtx8_get_debug_data_(u16 *debug_data, u32 sen_num,
		u32 drv_num, int data_type)
{
	int i = 0;
	int ret = 0;
	int data_len = 0;
	u8 buf[1] = {0};
	u8 *ptr = NULL;
	u16 data_start_addr = 0;
	int wait_cnt = 0;
	u16 coor_reg_addr;

	if (READ_RAW_DATA == data_type) {	/*  get rawdata */
		if (IC_TYPE_9886 == gtx8_ts->ic_type)
			data_start_addr = GTP_RAWDATA_ADDR_9886;
		else if (IC_TYPE_6861 == gtx8_ts->ic_type)
			data_start_addr = GTP_RAWDATA_ADDR_6861;
		else if (gtx8_ts->ic_type == IC_TYPE_7382)
			data_start_addr = GTP_RAWDATA_ADDR_7382;
		else
			data_start_addr = GTP_RAWDATA_ADDR_6862;
	} else if (READ_DIFF_DATA == data_type) { /* get noisedata */
		if (IC_TYPE_9886 == gtx8_ts->ic_type)
			data_start_addr = GTP_NOISEDATA_ADDR_9886;
		else if (IC_TYPE_6861 == gtx8_ts->ic_type)
			data_start_addr = GTP_NOISEDATA_ADDR_6861;
		else if (gtx8_ts->ic_type == IC_TYPE_7382)
			data_start_addr = GTP_NOISEDATA_ADDR_7382;
		else
			data_start_addr = GTP_NOISEDATA_ADDR_6862;
	} else {
		TS_LOG_ERR("%s:Not support data_type:%d\n", __func__, data_type);
		return -EINVAL;
	}

	data_len = (sen_num * drv_num) * sizeof(u16); /*  raw data is u16 type */
	if (data_len <= 0) {
		TS_LOG_ERR("%s:Invaliable data len: %d\n", __func__, data_len);
		return -EINVAL;
	}

	ptr = (u8 *)kzalloc(data_len, GFP_KERNEL);
	if (!ptr) {
		TS_LOG_ERR("%s:Failed alloc memory\n", __func__);
		return -ENOMEM;
	}

	/* change to rawdata mode */
	ret = gtx8_send_cmd(GTX8_CMD_RAWDATA, 0x00, GTX8_NEED_SLEEP);
	if (ret) {
		TS_LOG_ERR("%s: Failed send rawdata cmd:ret%d\n", __func__, ret);
		kfree(ptr);
		ptr = NULL;
		return ret;
	}

	for (wait_cnt = 0; wait_cnt < GTX8_RETRY_NUM_3; wait_cnt++) {
		/* waiting for data ready */
		msleep(150);
		ret = gtx8_i2c_read(GTP_REG_COOR, &buf[0], 1);
		if ((ret == 0) && (buf[0] & 0x80))
			break;
		TS_LOG_INFO("%s:Rawdata is not ready,buf[0]=0x%x\n", __func__, buf[0]);
	}

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		coor_reg_addr = GTP_REG_COOR_GT7382;
	else
		coor_reg_addr = GTP_REG_COOR;
	if (wait_cnt < GTX8_RETRY_NUM_3) {
		ret = gtx8_i2c_read(data_start_addr,
				ptr, data_len);
		if (ret) {
			TS_LOG_ERR("%s: Failed to read debug data\n", __func__);
			goto get_data_out;
		} else
			TS_LOG_INFO("%s:Success read debug data:datasize=%d\n", __func__, data_len);


		for (i = 0; i < data_len; i += 2)
			debug_data[i/2] = be16_to_cpup((__be16 *)&ptr[i]);

		/* clear data ready flag */
		buf[0] = 0x00;
		if (gtx8_i2c_write(coor_reg_addr, &buf[0], 1))
			TS_LOG_INFO("%s: Failed clear debug data ready flag\n", __func__);
	} else {
		TS_LOG_ERR("%s:Wait for data ready timeout\n", __func__);
		ret = -EFAULT;
	}

get_data_out:
	if (ptr) {
		kfree(ptr);
		ptr = NULL;
	}
	if (gtx8_send_cmd(GTX8_CMD_NORMAL, 0x00, GTX8_NEED_SLEEP))
		TS_LOG_ERR("%s: Send normal mode command falied\n", __func__);
	return ret;
}

int gtx8_get_debug_data(struct ts_diff_data_info *info,
		struct ts_cmd_node *out_cmd)
{
	int i = 0;
	int ret = 0;
	u32 sen_num = 0;
	u32 drv_num = 0;
	int used_size = 0;
	u8 *temp_cfg_data = NULL;
	u16 *debug_data = NULL;
	size_t data_size = 0;
	int rotate_type;
	u32 cfg_len = 0;

	if (!gtx8_ts) {
		TS_LOG_ERR("%s, tp not init\n", __func__);
		return -EINVAL;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382)
		rotate_type = GT_RAWDATA_CSV_RXTX_ROTATE;
	else
		rotate_type = GT_RAWDATA_CSV_VERTICAL_SCREEN;
	temp_cfg_data = kzalloc(GOODIX_CFG_MAX_SIZE, GFP_KERNEL);
	if (!temp_cfg_data) {
		TS_LOG_ERR("failed alloc memory\n");
		return -EINVAL;
	}

	ret = gtx8_set_i2c_doze_mode(false);
	if (ret) {
		TS_LOG_ERR("%s: failed disabled doze mode\n", __func__);
		goto exit;
	}

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		cfg_len = CFG_LEN_GT7382;
	ret = gtx8_read_cfg(temp_cfg_data, cfg_len);
	if (ret < 0) {
		TS_LOG_ERR("Failed to read config data\n");
		ret = -EINVAL;
		goto exit;
	}

	if (gtx8_ts->ic_type == IC_TYPE_7382)
		ret = gtx8_get_channel_num_gt7382(&sen_num,
			&drv_num, temp_cfg_data);
	else
		ret = gtx8_get_channel_num(&sen_num,
			&drv_num, temp_cfg_data);
	if (ret) {
		TS_LOG_ERR("%s:Failed get channel num, ret =%d\n", __func__, ret);
		goto exit;
	}

	data_size = sen_num * drv_num;
	debug_data = kzalloc(data_size * sizeof(u16), GFP_KERNEL);
	if (!debug_data) {
		ret = -ENOMEM;
		TS_LOG_ERR("%s: alloc mem fail\n", __func__);
		goto exit;
	}

	switch (info->debug_type) {
	case READ_DIFF_DATA:
		TS_LOG_INFO("%s: read diff data\n", __func__);
		ret = gtx8_get_debug_data_(debug_data, sen_num,
				drv_num, READ_DIFF_DATA);
		break;
	case READ_RAW_DATA:
		TS_LOG_ERR("%s: read raw data\n", __func__);
		ret = gtx8_get_debug_data_(debug_data, sen_num,
				drv_num, READ_RAW_DATA);
		break;
	default:
		ret = -EINVAL;
		TS_LOG_ERR("%s: debug_type mis match\n", __func__);
		break;
	}

	if (ret)
		goto free_debug_data;

	info->buff[used_size++] = drv_num;
	info->buff[used_size++] = sen_num;
	for (i = 0; i < data_size && used_size < TS_RAWDATA_BUFF_MAX; i++)
		/* TODO need abs value */
		info->buff[used_size++] = abs(debug_data[i]);

	ts_kit_rotate_rawdata_abcd2cbad(sen_num, drv_num,
		(info->buff + 2), rotate_type); /* 2: offset tx_num & rx_num */
	info->used_size = used_size;

free_debug_data:
	if (debug_data) {
		kfree(debug_data);
		debug_data = NULL;
	}

exit:
	if (temp_cfg_data) {
		kfree(temp_cfg_data);
		temp_cfg_data = NULL;
	}
	if (gtx8_set_i2c_doze_mode(true))
		TS_LOG_ERR("%s: failed enabled doze mode\n", __func__);
	return ret;
}

static void gtx8_work_after_input_kit(void)
{
	if (gtx8_ts->dev_data->ts_platform_data->feature_info.roi_info.roi_supported  || !gtx8_ts->roi.rawdata ) {
		/* We are about to refresh roi_data. To avoid stale output, use a completion to block possible readers. */
		reinit_completion(&roi_data_done);
		TS_LOG_DEBUG("%s: pre[%d]  cur[%d]  (cur_index & pre_index):[%d] .\n",
				__func__, pre_index, cur_index, (cur_index & pre_index));
		/* finger num id diff ,and must be finger done */
		if ((pre_index != cur_index) &&
			((cur_index & pre_index) != cur_index)) {
			if (gtx8_ts->gtx8_roi_fw_supported)
				/* fw has formatted the data */
				gtx8_cache_roidata_fw(&gtx8_ts->roi);
			else
				/* depend on driver to format data */
				gtx8_cache_roidata_device(&gtx8_ts->roi);
		}
		pre_index = cur_index;
		complete_all(&roi_data_done);  /* If anyone has been blocked by us, wake it up. */
	}
	return;
}

static int gtx8_charger_command_gt7382(u8 buffer[], u32 len)
{
	int ret_i2c;

	ret_i2c = gtx8_i2c_write(GTP_REG_CMD_GT7382,
		&buffer[0], len);
	if (ret_i2c) {
		TS_LOG_ERR("i2c_write command fail\n");
		return ret_i2c;
	}
	mdelay(GTX8_CHARGE_MODE_DELAY_TIME);

	ret_i2c = gtx8_i2c_read(GTP_REG_CMD_GT7382,
		&buffer[0], len);
	if (ret_i2c) {
		TS_LOG_ERR("i2c_read command fail\n");
		return ret_i2c;
	}
	if (buffer[0] != GTX8_CHARGE_REG_VALUE)
		TS_LOG_ERR("Charging command sent faile");

	return ret_i2c;
}

static int gtx8_charger_mode_gt7382(u8 charger_switch)
{
	int ret;
	u8 charge_on_buffer[GTX8_CHARGE_BUFEER_LEN] = { GTX8_CHARGE_ON_VALUE_0,
		GTX8_CHARGE_ON_VALUE_1, GTX8_CHARGE_ON_VALUE_2 };
	u8 charge_off_buffer[GTX8_CHARGE_BUFEER_LEN] = { GTX8_CHARGE_OFF_VALUE_0,
		GTX8_CHARGE_OFF_VALUE_1, GTX8_CHARGE_OFF_VALUE_2 };
	if (charger_switch) {
		ret = gtx8_charger_command_gt7382(charge_on_buffer,
			GTX8_CHARGE_BUFEER_LEN);
		if (ret)
			TS_LOG_ERR("charger on failed!\n");
	} else {
		ret = gtx8_charger_command_gt7382(charge_off_buffer,
			GTX8_CHARGE_BUFEER_LEN);
		if (ret)
			TS_LOG_ERR("charger off failed!\n");
	}
	return ret;
}

static int gtx8_charger_switch(struct ts_charger_info *info)
{
	int ret = NO_ERR;
	unsigned char  cmd = 0;
	struct ts_feature_info *feature_info = NULL;
	if (!info || !info->charger_supported || !gtx8_ts || \
		!gtx8_ts->dev_data || !gtx8_ts->dev_data->ts_platform_data) {
		TS_LOG_ERR("%s: info is Null or no support charger mode\n", __func__);
		ret = -ENOMEM;
		return ret;
	}
	feature_info = &gtx8_ts->dev_data->ts_platform_data->feature_info;

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		/*charger config only take effect when TP ic is in normal station*/
		if((!feature_info->holster_info.holster_switch) && \
			(!feature_info->glove_info.glove_switch) && \
			(!gtx8_ts->game_switch)){
			if (gtx8_ts->ic_type == IC_TYPE_7382)
				return gtx8_charger_mode_gt7382(info->charger_switch);
			if (info->charger_switch){
				ret = gtx8_feature_switch(gtx8_ts,TS_FEATURE_CHARGER, SWITCH_ON);
				cmd = GTX8_CMD_CHARGER_ON;//switch on cmd
			} else {
				ret = gtx8_feature_switch(gtx8_ts,TS_FEATURE_CHARGER, SWITCH_OFF);
				cmd = GTX8_CMD_CHARGER_OFF;//switch off  cmd
			}
			TS_LOG_INFO("send charger cmd:%d\n",cmd);
			msleep(SEND_CMD_AFTER_CONFIG_DELAY);//wait config taking effect.
			if (gtx8_send_cmd(cmd, 0, GTX8_NOT_NEED_SLEEP)) {
				TS_LOG_ERR("Send stylus wake up config failed.\n");
				ret = -EINVAL;
			}
			TS_LOG_INFO("%s:set charger_switch(%d),ret = %d\n",__func__, info->charger_switch, ret);
		} else {
			TS_LOG_INFO("%s:no need to send normal_noise_cfg.\n", __func__);
			return 0;
		}
		break;
	case TS_ACTION_READ:
		TS_LOG_INFO("%s:charger_switch read :%d\n",__func__, info->charger_switch);
		break;
	default:
		TS_LOG_INFO("%s: invalid charger_switch action: %d\n",__func__, info->op_action);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int gtx8_wakeup_gesture_enable_switch(struct
						  ts_wakeup_gesture_enable_info
						  *info)
{
	int ret = 0;
	unsigned char  cmd = 0;
	if (!info || !gtx8_ts) {
		TS_LOG_ERR("%s:invalid input.\n",__func__);
		ret = -EINVAL;
		goto exit;
	}
	if (gtx8_ts->ic_type == IC_TYPE_7382) {
		TS_LOG_ERR("%s, not support wakeup_gesture\n", __func__);
		return 0;
	}

	TS_LOG_INFO("%s:switch_value = %d.\n",__func__, info->switch_value);
	if(info->switch_value  == STYLUS_WAKEUP_NORMAL_STATUS){
		cmd = GTP_CMD_STYLUS_NORMAL;
	} else if (info->switch_value  == STYLUS_WAKEUP_LOW_FREQENCY){
		cmd = GTP_CMD_STYLUS_LOW_FREQENCY;
	} else if (info->switch_value  == STYLUS_WAKEUP_DISABLE){
		cmd = GTP_CMD_STYLUS_SWITCH_OFF;
	} else if (info->switch_value  == STYLUS_WAKEUP_TESTMODE){
		cmd = GTP_CMD_STYLUS_NORMAL;
	} else {
		return 0;
	}
	msleep(SEND_CMD_AFTER_CONFIG_DELAY);
	if (gtx8_send_cmd(cmd, 0, GTX8_NOT_NEED_SLEEP)) {
		TS_LOG_ERR("Send stylus wake up config failed.\n");
		ret = -EINVAL;
		goto exit;
	}

exit:
	return 0;
}

struct ts_device_ops ts_gtx8_ops = {
	.chip_detect = gtx8_chip_detect,
	.chip_init = gtx8_chip_init,
	.chip_input_config = gtx8_input_config,
	.chip_input_pen_config = gtx8_input_pen_config,
	.chip_irq_bottom_half = gtx8_irq_bottom_half,
	.chip_reset = gtx8_chip_reset,
	.chip_fw_update_boot = gtx8_fw_update_boot,
	.chip_fw_update_sd = gtx8_fw_update_sd,
	.chip_get_info = gtx8_chip_get_info,
	.chip_suspend = gtx8_chip_suspend,
	.chip_resume = gtx8_chip_resume,
	.chip_after_resume = gtx8_chip_after_resume,
	.chip_get_rawdata = gtx8_get_rawdata,
	.chip_glove_switch = gtx8_glove_switch,
	.chip_shutdown = gtx8_chip_shutdown,
	.chip_holster_switch = gtx8_holster_switch,
	.chip_roi_switch = gtx8_roi_switch,
	.chip_roi_rawdata = gtx8_roi_rawdata,
	.chip_check_status = gtx8_check_hw_status,
	.chip_get_capacitance_test_type =
		gtx8_chip_get_capacitance_test_type,
	.chip_get_debug_data = gtx8_get_debug_data,
	.chip_work_after_input = gtx8_work_after_input_kit,
	.chip_touch_switch = gtx8_touch_switch,
	.chip_charger_switch = gtx8_charger_switch,
	.chip_wakeup_gesture_enable_switch =
		gtx8_wakeup_gesture_enable_switch,
	.chip_write_hop_ack = gtx8_write_hop_ack_gt7382,
	.chip_stylus3_connect_state = stylus3_connect_state,
	.chip_stylus3_type_info_handle = stylus3_type_info_handle,
	.chip_send_supression_cmd_to_fw = send_suppression_cmd_to_fw,
};

static int __init gtx8_ts_module_init(void)
{
	int ret = NO_ERR;
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;

	TS_LOG_INFO("%s: called\n", __func__);
	gtx8_ts = kzalloc(sizeof(struct gtx8_ts_data), GFP_KERNEL);
	if (NULL == gtx8_ts) {
		TS_LOG_ERR("%s:alloc mem for device data fail\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	gtx8_ts->dev_data =
		kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (NULL == gtx8_ts->dev_data) {
		TS_LOG_ERR("%s:alloc mem for ts_kit_device data fail\n", __func__);
		ret = -ENOMEM;
		goto error_exit;
	}

	root = of_find_compatible_node(NULL, NULL, HUAWEI_TS_KIT);
	if (!root) {
		TS_LOG_ERR("%s:find_compatible_node error\n", __func__);
		ret = -EINVAL;
		goto error_exit;
	}

	for_each_child_of_node(root, child) {
		if (of_device_is_compatible(child, GTP_GTX8_CHIP_NAME)) {
			found = true;
			break;
		}
	}

	if (!found) {
		TS_LOG_ERR("%s:device tree node not found, name=%s\n",
			__func__, GTP_GTX8_CHIP_NAME);
		ret = -EINVAL;
		goto error_exit;
	}

	gtx8_ts->dev_data->cnode = child;
	gtx8_ts->dev_data->ops = &ts_gtx8_ops;
	ret = huawei_ts_chip_register(gtx8_ts->dev_data);
	if (ret) {
		TS_LOG_ERR("%s:chip register fail, ret=%d\n", __func__, ret);
		goto error_exit;
	}

	TS_LOG_INFO("%s:success\n", __func__);
	return ret;

error_exit:
	if (gtx8_ts) {
		if (gtx8_ts->dev_data) {
			kfree(gtx8_ts->dev_data);
			gtx8_ts->dev_data = NULL;
		}
		kfree(gtx8_ts);
		gtx8_ts = NULL;
	}

	TS_LOG_INFO("%s:fail\n", __func__);
	return ret;
}

static void __exit gtx8_ts_module_exit(void)
{
	if (gtx8_ts) {
		if (gtx8_ts->dev_data) {
			kfree(gtx8_ts->dev_data);
			gtx8_ts->dev_data = NULL;
		}
		kfree(gtx8_ts);
		gtx8_ts = NULL;
	}

	return;
}

late_initcall(gtx8_ts_module_init);
module_exit(gtx8_ts_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");
