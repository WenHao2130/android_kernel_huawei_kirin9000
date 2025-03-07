/*
 * ds28el16.c
 *
 * Implementation of communication protocol for ds28el16
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

#include <linux/types.h>
#include <linux/pm_qos.h>
#include <linux/smp.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>

#include "ds28el16.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG HW_ONEWIRE_DS28EL16
HWLOG_REGIST();

#define TMP_BUF_LEN 64
#define DS28EL16_PAGE_SIZE 16
#define DS28EL16_MSECRET_BYTES 16
#define DS28EL16_PAGE_SIZE 16
#define DS28EL16_TMP_CHAR_LEN 3

static struct platform_device *g_st_pdev;
static struct pm_qos_request g_pm_qos;

/* before receive cmd responce we should wait Xms */
static unsigned char get_waittime(unsigned char cmd,
	struct maxim_onewire_drv_data *drv_data)
{
	struct maxim_onewire_time_request *t = &drv_data->ic_des.trq;
	unsigned char waittime = 0;

	switch (cmd) {
	case DS28EL16_WRITE_MEMORY:
		waittime = t->t_write_memory;
		break;
	case DS28EL16_READ_MEMORY:
		waittime = t->t_read;
		break;
	case DS28EL16_READ_STATUS:
		waittime = t->t_read;
		break;
	case DS28EL16_SET_PROTECTION:
		waittime = t->t_write_status;
		break;
	case DS28EL16_GET_MAC:
		waittime = t->t_read + t->t_compute;
		break;
	case DS28EL16_S_SECRET:
		waittime = t->t_read + t->t_compute;
		break;
	default:
		hwlog_err("cmd %02X not support in %s\n", cmd, __func__);
		break;
	}

	return waittime;
}

/* not include result byte & Length Byte */
static unsigned char get_rx_length(unsigned char cmd)
{
	unsigned char length = 0;

	switch (cmd) {
	case DS28EL16_WRITE_MEMORY:
		/* 1 : write memory cmd length */
		length = 1;
		break;
	case DS28EL16_READ_MEMORY:
		/* 33 : read memory cmd length */
		length = 33;
		break;
	case DS28EL16_READ_STATUS:
		/* 7 : read status cmd length */
		length = 7;
		break;
	case DS28EL16_SET_PROTECTION:
		/* 1 : set protection cmd length */
		length = 1;
		break;
	case DS28EL16_GET_MAC:
		/* 33 : get mac cmd length */
		length = 33;
		break;
	case DS28EL16_S_SECRET:
		/* 1 : serect cmd length */
		length = 1;
		break;
	default:
		hwlog_err("cmd %02X not support in %s\n", cmd, __func__);
		break;
	}

	return length;
}

/* not include result byte & Length Byte */
static unsigned char get_tx_length(unsigned char cmd)
{
	unsigned char length = 0;

	switch (cmd) {
	case DS28EL16_WRITE_MEMORY:
		/* 18 : write memory cmd length */
		length = 18;
		break;
	case DS28EL16_READ_MEMORY:
		/* 2 : read memory cmd length */
		length = 2;
		break;
	case DS28EL16_READ_STATUS:
		/* 1 : read status cmd length */
		length = 1;
		break;
	case DS28EL16_SET_PROTECTION:
		/* 3 : set protection cmd length */
		length = 3;
		break;
	case DS28EL16_GET_MAC:
		/* 35 : get mac cmd length */
		length = 35;
		break;
	case DS28EL16_S_SECRET:
		/* 35 : serect cmd length */
		length = 35;
		break;
	default:
		hwlog_err("cmd %02X not support in %s\n", cmd, __func__);
		break;
	}

	return length;
}

static int ds28el16_cmds(struct maxim_onewire_drv_data *drv_data,
	unsigned char *tx_data, unsigned char *rx_data)
{
	int ret = -1;
	unsigned char buff[TMP_BUF_LEN] = { 0 };
	unsigned char buff_cnt;
	struct ds28el16_drv_data *ds_drv =
		container_of(drv_data, struct ds28el16_drv_data, mdrv);
	struct ow_phy_ops *phy_ops = &drv_data->ic_des.phy_ops;
	unsigned char *bug = &ds_drv->status_crc_bug;
	unsigned char i;
	unsigned char tx_len =
		get_tx_length(tx_data[DS28EL16_TX_DATA_CMD_INDEX]);
	unsigned char rx_len =
		get_rx_length(tx_data[DS28EL16_TX_DATA_CMD_INDEX]);
	unsigned char ic_rx_len;
	unsigned char ic_ret_byte;

	if (!phy_ops->reset) {
		hwlog_err("NULL:phy_ops->reset found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->read_byte) {
		hwlog_err("NULL:phy_ops->read_byte found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->write_byte) {
		hwlog_err("NULL:phy_ops->write_byte found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->set_time_request) {
		hwlog_err("NULL:phy_ops->set_time_request found in %s\n",
			__func__);
		return -1;
	}
	if (!phy_ops->wait_for_ic) {
		hwlog_err("NULL:phy_ops->wait_for_ic found in %s\n", __func__);
		return -1;
	}

	if ((tx_len == 0) || (rx_len == 0)) {
		hwlog_err("rx len %u or tx len %u == 0\n", rx_len, tx_len);
		return ret;
	}

	/* Add the QoS request and set the latency to 0 */
	cpu_latency_qos_add_request(&g_pm_qos, 0);
	kick_all_cpus_sync();

	buff_cnt = 0;
	/* start reset */
	if (phy_ops->reset()) {
		hwlog_err("No slave responce\n");
		goto final_reset;
	}
	phy_ops->write_byte(SKIP_ROM);

	/* send start byte */
	phy_ops->write_byte(DS28EL16_COMMAND_START);
	buff[buff_cnt++] = DS28EL16_COMMAND_START;
	/* send cmd&parameter length */
	phy_ops->write_byte(tx_len);
	buff[buff_cnt++] = tx_len;
	/* send cmd&parameter length */
	for (i = 0; i < tx_len; i++) {
		phy_ops->write_byte(tx_data[i]);
		buff[buff_cnt++] = tx_data[i];
	}
	/* TX Receive CRC */
	buff[buff_cnt++] = phy_ops->read_byte();
	buff[buff_cnt++] = phy_ops->read_byte();
	if ((check_crc16(buff, buff_cnt) != MAXIM_CRC16_RESULT) &&
		((*bug != 0) ||
		(tx_data[DS28EL16_TX_DATA_CMD_INDEX] !=
		DS28EL16_READ_STATUS))) {
		hwlog_err("TX CRC16 failed in %s for cmd %02X\n", __func__,
			tx_data[DS28EL16_TX_DATA_CMD_INDEX]);
		goto final_reset;
	}

	/* Release cmd */
	phy_ops->write_byte(DS28EL16_RELEASE_BYTE);
	if (tx_data[DS28EL16_TX_DATA_CMD_INDEX] == DS28EL16_READ_STATUS)
		*bug = 1;

	/* wait for cmd complete */
	phy_ops->wait_for_ic(get_waittime(tx_data[DS28EL16_TX_DATA_CMD_INDEX],
		drv_data));

	/* read out dummy byte */
	phy_ops->read_byte();
	/* read out rx data length */
	ic_rx_len = phy_ops->read_byte();
	if (ic_rx_len != rx_len) {
		hwlog_err("unexpect rx length %u while %u expected in %s\n",
			ic_rx_len, rx_len, __func__);
		goto final_reset;
	}
	buff_cnt = 0;
	buff[buff_cnt++] = rx_len;

	/* read cmd excute result */
	ic_ret_byte = phy_ops->read_byte();
	if (ic_ret_byte != MAXIM_ONEWIRE_COMMAND_SUCCESS) {
		hwlog_err("cmd %02X excute failed %02X\n",
			tx_data[DS28EL16_TX_DATA_CMD_INDEX], ic_ret_byte);
		goto final_reset;
	}
	buff[buff_cnt++] = ic_ret_byte;
	/* read out rx data if any rx_len not include crc_16 */
	for (i = 0; i < rx_len - 1; i++) {
		rx_data[i] = phy_ops->read_byte();
		buff[buff_cnt++] = rx_data[i];
	}
	/* read crc 16 */
	buff[buff_cnt++] = phy_ops->read_byte();
	buff[buff_cnt++] = phy_ops->read_byte();

	/* RX Receive CRC */
	if (check_crc16(buff, buff_cnt) != MAXIM_CRC16_RESULT) {
		hwlog_err("RX CRC16 failed in %s for cmd %02X\n", __func__,
			tx_data[DS28EL16_TX_DATA_CMD_INDEX]);
		goto final_reset;
	}
	ret = 0;
final_reset:
	/* final reset */
	phy_ops->reset();
	cpu_latency_qos_remove_request(&g_pm_qos);
	return ret;
}

#ifdef ONEWIRE_STABILITY_DEBUG
/* write page data just use for page0 & page1 */
static int ds28el16_write_memory(struct maxim_onewire_drv_data *drv_data,
	unsigned char page, const unsigned char *data)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_TX_BUF_LEN] = { 0 };

	if (page > DS28EL16_MAX_USABLE_PAGE) {
		hwlog_err("page %u data should not be set\n", page);
		return -1;
	}

	incr_total_com_stat(drv_data, SET_PAGE_DATA_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_WRITE_MEMORY;
	tx_data[DS28EL16_TX_DATA_PARM1_INDEX] = page;
	memcpy(tx_data + DS28EL16_TX_DATA_PARM1_INDEX + 1,
		data, get_page_size(drv_data));

	clear_page_data_sign(drv_data, page);
	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, SET_PAGE_DATA_INDEX);
		return -1;
	}
	put_page_data(drv_data, page, data);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}
#endif

/* read data for page that's not read protection */
static int ds28el16_read_memory(struct maxim_onewire_drv_data *drv_data,
	unsigned char page)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };

	if (page >= get_page_num(drv_data))
		return -1;

	incr_total_com_stat(drv_data, GET_PAGE_DATA_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_READ_MEMORY;
	tx_data[DS28EL16_TX_DATA_PARM1_INDEX] = page;
	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, GET_PAGE_DATA_INDEX);
		return -1;
	}
	put_page_data(drv_data, page, rx_data);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}

/* read ic status include four page status & main id & version */
static int ds28el16_read_status(struct maxim_onewire_drv_data *drv_data)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char status_num = get_page_num(drv_data);

	incr_total_com_stat(drv_data, GET_PAGE_STATUS_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_READ_STATUS;

	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, GET_PAGE_STATUS_INDEX);
		return -1;
	}
	put_page_status(drv_data, rx_data);
	put_man_id(drv_data, rx_data[status_num]);
	put_version(drv_data, rx_data[status_num + 1]);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}

static int ds28el16_set_status(struct maxim_onewire_drv_data *drv_data,
	unsigned char page, unsigned char status)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };

	if (page > DS28EL16_MAX_USABLE_PAGE) {
		hwlog_err("page %u status should not be set\n", page);
		return -1;
	}

	incr_total_com_stat(drv_data, SET_PAGE_STATUS_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_SET_PROTECTION;
	tx_data[DS28EL16_TX_DATA_PARM1_INDEX] = page;
	tx_data[DS28EL16_TX_DATA_PARM2_INDEX] = status;

	clear_page_status_sign(drv_data);
	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, SET_PAGE_STATUS_INDEX);
		return -1;
	}

	put_one_block_status(drv_data, page, status);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}

static int ds28el16_get_mac(struct maxim_onewire_drv_data *drv_data,
	unsigned char anon, unsigned char page)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };

	if (page > DS28EL16_MAX_USABLE_PAGE) {
		hwlog_err("page %u should not be used to get mac\n", page);
		return -1;
	}

	incr_total_com_stat(drv_data, GET_MAC_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	clear_mac_data_sign(drv_data);
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_GET_MAC;
	anon = anon ? DS28EL16_ANON_MASK : 0;
	tx_data[DS28EL16_TX_DATA_PARM1_INDEX] = anon | page;
	tx_data[DS28EL16_TX_DATA_PARM2_INDEX] = DS28EL16_AUTH_PARAM;
	memcpy(tx_data + DS28EL16_TX_DATA_PARM3_INDEX,
		drv_data->random_bytes, RANDOM_NUM_BYTES);

	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, GET_MAC_INDEX);
		return -1;
	}

	put_mac_data(drv_data, rx_data);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}

static int ds28el16_s_secret(struct ds28el16_drv_data *ds_drv)
{
	struct maxim_onewire_drv_data *drv_data = &ds_drv->mdrv;
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };

	incr_total_com_stat(drv_data, SET_S_SECRET_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);
	ds_drv->s_secret_sign = 0;
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_S_SECRET;
	tx_data[DS28EL16_TX_DATA_PARM1_INDEX] = DS28EL16_S_SECRET_PAGE_PARA;
	tx_data[DS28EL16_TX_DATA_PARM2_INDEX] = DS28EL16_SEC_PARAM;
	memcpy(tx_data + DS28EL16_TX_DATA_PARM3_INDEX,
		ds_drv->secret_seed, DS28EL16_SECRET_SEED_BYTES);

	if (ds28el16_cmds(drv_data, tx_data, rx_data)) {
		hwlog_err("%s failed\n", __func__);
		mutex_unlock(&drv_data->ic_des.memory.lock);
		incr_err_com_stat(drv_data, SET_S_SECRET_INDEX);
		return -1;
	}
	ds_drv->s_secret_sign = 1;
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;
}

static int ds28el16_get_rom_id(struct maxim_onewire_drv_data *drv_data)
{
	unsigned char rx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	unsigned char tx_data[DS28EL16_RX_BUF_LEN] = { 0 };
	struct ow_phy_ops *phy_ops = &drv_data->ic_des.phy_ops;
	unsigned char i;
	unsigned char crc8;
	int retval;

	if (!phy_ops->reset) {
		hwlog_err("NULL:phy_ops->reset found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->read_byte) {
		hwlog_err("NULL:phy_ops->read_byte found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->write_byte) {
		hwlog_err("NULL:phy_ops->write_byte found in %s\n", __func__);
		return -1;
	}
	if (!phy_ops->set_time_request) {
		hwlog_err("NULL:phy_ops->set_time_request found in %s\n",
			__func__);
		return -1;
	}
	if (!phy_ops->wait_for_ic) {
		hwlog_err("NULL:phy_ops->wait_for_ic found in %s\n", __func__);
		return -1;
	}

	incr_total_com_stat(drv_data, GET_ROM_ID_INDEX);
	mutex_lock(&drv_data->ic_des.memory.lock);

	/* Reset */
	if (no_slave_response(phy_ops->reset())) {
		hwlog_err("1st reset: no slave response in %s\n", __func__);
		retval = MAXIM_ONEWIRE_NO_SLAVE;
		goto err_com_stat;
	}

	/* Skip ROM first */
	phy_ops->write_byte(SKIP_ROM);

	/* read status ignore */
	tx_data[DS28EL16_TX_DATA_CMD_INDEX] = DS28EL16_READ_STATUS;
	if (ds28el16_cmds(drv_data, tx_data, rx_data))
		hwlog_err("command run failed in %s\n", __func__);

	/* Reset */
	if (no_slave_response(phy_ops->reset())) {
		hwlog_err("2nd reset:: no slave response in %s\n", __func__);
		retval = MAXIM_ONEWIRE_NO_SLAVE;
		goto err_com_stat;
	}
	/* Read ROM */
	phy_ops->write_byte(READ_ROM);

	/*
	 * Read 8 bytes ROM = 8-bit family code, unique 48-bit serial number,
	 * and 8-bit CRC
	 */
	for (i = 0; i < get_rom_id_len(drv_data); i++)
		rx_data[i] = phy_ops->read_byte();

	/* CRC8 Check */
	crc8 = check_crc8(rx_data, get_rom_id_len(drv_data));
	if (crc8 != MAXIM_CRC8_RESULT) {
		hwlog_err("maxim: CRC8 failed in %s\n", __func__);
		retval = MAXIN_ONEWIRE_CRC8_ERROR;
		goto err_com_stat;
	}

	if (*rx_data != DS28EL16_ROM_ID_VALID_SIGN) {
		hwlog_err("rom id sign is  0x%x, not 0x9F\n", *rx_data);
		retval = MAXIM_ONEWIRE_ILLEGAL_PAR;
		goto err_com_stat;
	}
	put_rom_id(drv_data, rx_data);
	mutex_unlock(&drv_data->ic_des.memory.lock);

	return 0;

err_com_stat:
	mutex_unlock(&drv_data->ic_des.memory.lock);
	incr_err_com_stat(drv_data, GET_ROM_ID_INDEX);
	return retval;
}

static int ds28el16_get_rom_id_retry(struct maxim_onewire_drv_data *drv_data)
{
	int i, ret;

	if (check_rom_id_sign(drv_data))
		return MAXIM_SUCCESS;

	set_sched_affinity_to_current();
	for (i = 0; i < GET_ROM_ID_RETRY; i++) {
		hwlog_info("read rom id communication start\n");
		ret = ds28el16_get_rom_id(drv_data);
		maxim_onewire_communication_info(ret, "get_rom_id");

		if (check_rom_id_sign(drv_data)) {
			set_sched_affinity_to_all();
			return MAXIM_SUCCESS;
		}
	}
	set_sched_affinity_to_all();

	return MAXIM_FAIL;
}

static int ds28el16_get_pstatus_retry(struct maxim_onewire_drv_data *drv_data)
{
	int i, ret;

	if (check_page_status_sign(drv_data))
		return MAXIM_SUCCESS;

	set_sched_affinity_to_current();
	for (i = 0; i < GET_BLOCK_STATUS_RETRY; i++) {
		hwlog_info("read page status communication start\n");
		ret = ds28el16_read_status(drv_data);
		maxim_onewire_communication_info(ret, "read_status");

		if (check_page_status_sign(drv_data)) {
			set_sched_affinity_to_all();
			return MAXIM_SUCCESS;
		}
	}
	set_sched_affinity_to_all();

	return MAXIM_FAIL;
}

static int ds28el16_set_pstatus_retry(struct maxim_onewire_drv_data *drv_data,
	unsigned char page, unsigned char status)
{
	int i, ret;

	set_sched_affinity_to_current();
	for (i = 0; i < SET_BLOCK_STATUS_RETRY; i++) {
		hwlog_info("set page status communication start\n");
		ret = ds28el16_set_status(drv_data, page, status);
		maxim_onewire_communication_info(ret, "set_page_status");

		if (check_page_status_sign(drv_data)) {
			set_sched_affinity_to_all();
			return MAXIM_SUCCESS;
		}
	}
	set_sched_affinity_to_all();

	return MAXIM_FAIL;
}

static int ds28el16_get_page_data_retry(struct maxim_onewire_drv_data *drv_data,
	int page)
{
	int i, ret;

	if (page >= get_page_num(drv_data))
		return MAXIM_FAIL;

	if (check_page_data_sign(drv_data, page))
		return MAXIM_SUCCESS;

	set_sched_affinity_to_current();
	for (i = 0; i < GET_USER_MEMORY_RETRY; i++) {
		hwlog_info("read page data communication start\n");
		ret = ds28el16_read_memory(drv_data, page);
		maxim_onewire_communication_info(ret, "read_page_data");
		if (check_page_data_sign(drv_data, page)) {
			set_sched_affinity_to_all();
			return MAXIM_SUCCESS;
		}
	}
	set_sched_affinity_to_all();

	return MAXIM_FAIL;
}

#ifdef ONEWIRE_STABILITY_DEBUG
static int ds28el16_get_id(struct ds28el16_drv_data *ds_drv,
	const unsigned char **id, unsigned int *id_len)
{
	struct maxim_onewire_drv_data *drv_data = NULL;

	if (!ds_drv)
		return MAXIM_FAIL;
	drv_data = &ds_drv->mdrv;

	if (id_len)
		*id_len = get_rom_id_len(drv_data);

	if (id) {
		ds28el16_get_rom_id_retry(drv_data);
		if (check_rom_id_sign(drv_data)) {
			*id = get_rom_id(drv_data);
			return MAXIM_SUCCESS;
		}
	}

	return MAXIM_FAIL;
}
#endif

static int ds28el16_check_page_status(struct ds28el16_drv_data *ds_drv)
{
	struct maxim_onewire_drv_data *drv_data = &ds_drv->mdrv;
	unsigned char *status = get_page_status(drv_data);
	unsigned char page_sn_status;
	unsigned char page_msecret_status;
	char err = MAXIM_SUCCESS;
	int sn_page_no = drv_data->ic_des.memory.sn_page;

	if (sn_page_no >= get_page_num(drv_data))
		return MAXIM_FAIL;
	/* sn page */
	page_sn_status = status[sn_page_no] & DS28EL16_PROTECTION_MASK;
	if ((page_sn_status != DS28EL16_NO_PROTECTION) &&
		(page_sn_status != DS28EL16_WR_PROTECTION)) {
		err = MAXIM_FAIL;
		hwlog_err("Info page sn status was wrong %02X\n",
			status[sn_page_no]);
	} else {
		hwlog_info("Info page sn status was %02X\n",
			status[sn_page_no]);
	}

	if (!ds_drv->check_key_page_status)
		return err;

	/* master secret page */
	page_msecret_status = status[DS28EL16_MASTER_PAGE_NO] &
		DS28EL16_PROTECTION_MASK;
	if (page_msecret_status != DS28EL16_RW_PROTECTION) {
		err = MAXIM_FAIL;
		hwlog_err("Info page secret status was wrong %02X\n",
			status[DS28EL16_MASTER_PAGE_NO]);
	} else {
		hwlog_info("Info page secret status was %02X\n",
			status[DS28EL16_MASTER_PAGE_NO]);
	}

	return err;
}

static int ds28el16_check_ic_status(struct platform_device *pdev)
{
	struct ds28el16_drv_data *ds_drv = platform_get_drvdata(pdev);
	struct maxim_onewire_drv_data *drv_data = NULL;

	if (!ds_drv)
		return MAXIM_FAIL;
	drv_data = &ds_drv->mdrv;

	/* rom id */
	(void)ds28el16_get_rom_id_retry(drv_data);
	if (!check_rom_id_sign(drv_data)) {
		hwlog_err("get rom id failed\n");
		return MAXIM_FAIL;
	}

	if (maxim_check_rom_id_format(drv_data))
		return MAXIM_FAIL;

	/* page status */
	(void)ds28el16_get_pstatus_retry(drv_data);
	if (!check_page_status_sign(drv_data)) {
		hwlog_err("get page status failed\n");
		return MAXIM_FAIL;
	}

	if (ds28el16_check_page_status(ds_drv))
		return MAXIM_FAIL;

	return MAXIM_SUCCESS;
}

static int ds28el16_get_sn(struct platform_device *pdev, struct power_genl_attr *res,
	const unsigned char **sn, unsigned int *sn_size)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned int sn_page;
	unsigned char *sn_page_data = NULL;
	unsigned char *printable_sn = NULL;

	(void)res;
	if (!pdev)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;
	drv_data = &ds_drv->mdrv;
	sn_page = drv_data->ic_des.memory.sn_page;
	printable_sn = drv_data->ic_des.memory.sn;

	if ((sn_page >= get_page_num(drv_data)) || !printable_sn)
		return MAXIM_FAIL;

	if (sn) {
		/* sn page data */
		ds28el16_get_page_data_retry(drv_data, sn_page);
		if (!check_page_data_sign(drv_data, sn_page)) {
			hwlog_err("get sn page data failed\n");
			return MAXIM_FAIL;
		}

		sn_page_data = get_page_data(drv_data, sn_page);
		if (sn) {
			/* sn offset (1byte version) */
			maxim_parise_printable_sn(sn_page_data,
				drv_data->ic_des.memory.sn_offset_bits,
				printable_sn);
			*sn = printable_sn;
		}
	}
	if (sn_size)
		*sn_size = PRINTABLE_SN_SIZE - 1;

	return MAXIM_SUCCESS;
}

static int ds28el16_get_batt_type(struct platform_device *pdev,
	const unsigned char **type, unsigned int *type_len)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	const unsigned char *sn = NULL;
	unsigned int sn_size;
	unsigned char *batt_type = NULL;
	unsigned char *printable_sn = NULL;

	if (!pdev || !type || !type_len)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;
	drv_data = &ds_drv->mdrv;
	batt_type = drv_data->ic_des.memory.batt_type;
	printable_sn = drv_data->ic_des.memory.sn;

	if (ds28el16_get_sn(pdev, NULL, &sn, &sn_size)) {
		hwlog_err("get type failed because of getting sn failed\n");
		return MAXIM_FAIL;
	}

	batt_type[0] = printable_sn[BATTERY_PACK_FACTORY];
	batt_type[1] = printable_sn[BATTERY_CELL_FACTORY];
	*type = batt_type;
	/* Effective battery type length */
	*type_len = 2;

	return MAXIM_SUCCESS;
}

static int ds28el16_certification(struct platform_device *pdev,
	struct power_genl_attr *res, enum key_cr *result)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *remote_mac = NULL;
	int ret, j;
	int page = DS28EL16_MAXIM_PAGE1;
	const unsigned char *data = NULL;
	unsigned int data_len;

	if (!pdev || !res || !result)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;

	drv_data = &ds_drv->mdrv;
	data = res->data;
	data_len = res->len;

	if (data_len != get_mac_data_len(drv_data)) {
		hwlog_err("certification data length %u not correct\n",
			data_len);
		return MAXIM_FAIL;
	}

	set_sched_affinity_to_current();
	for (j = 0; j < GET_MAC_RETRY; j++) {
		/* s-secret */
		ds_drv->s_secret_sign = 0;
		if (!ds_drv->s_secret_sign)
			ds28el16_s_secret(ds_drv);

		if (!ds_drv->s_secret_sign) {
			hwlog_err("s_secret compute failed in %s\n", __func__);
			continue;
		}
		/* hmac */
		if (!check_mac_data_sign(drv_data)) {
			ret = ds28el16_get_mac(drv_data, 0, page);
			maxim_onewire_communication_info(ret, "get_mac");
		}

		if (check_mac_data_sign(drv_data)) {
			remote_mac = get_mac_data(drv_data);
			if (memcmp(remote_mac, data, data_len)) {
				*result = KEY_FAIL_UNMATCH;
				hwlog_err("mac is unmatch\n");
			} else {
				*result = KEY_PASS;
				hwlog_err("mac is match\n");
			}

			set_sched_affinity_to_all();
			return MAXIM_SUCCESS;
		}
	}
	set_sched_affinity_to_all();
	*result = KEY_FAIL_TIMEOUT;

	return MAXIM_SUCCESS;
}

static int ds28el16_prepare(struct platform_device *pdev,
	enum res_type type, struct power_genl_attr *res)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *mac_datum = NULL;
	int page;

	if (!pdev || !res)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;

	drv_data = &ds_drv->mdrv;
	mac_datum = drv_data->ic_des.memory.mac_datum;

	if (mac_datum == 0) {
		hwlog_err("Mac dataum should not be NULL %s\n", __func__);
		return MAXIM_FAIL;
	}

	switch (type) {
	case RES_CT:
		page = DS28EL16_MAXIM_PAGE1;
		/* rom id */
		ds28el16_get_rom_id_retry(drv_data);
		if (!check_rom_id_sign(drv_data)) {
			hwlog_err("get rom id failed in %s\n", __func__);
			return MAXIM_FAIL;
		}
		/* page data */
		ds28el16_get_page_data_retry(drv_data, page);
		if (!check_page_data_sign(drv_data, page)) {
			hwlog_err("get sn page%d data failed in %s\n",
				page, __func__);
			return MAXIM_FAIL;
		}
		/* page status--for man id */
		ds28el16_get_pstatus_retry(drv_data);
		if (!check_page_status_sign(drv_data))
			break;

		memset(mac_datum, 0, MAX_MAC_SOURCE_SIZE);
		/* partical secret */
		memcpy(mac_datum + DS28EL16_HMAC_PSECRET_OFFSET,
			ds_drv->secret_seed, DS28EL16_SECRET_SEED_BYTES);
		/* ROM ID */
		memcpy(mac_datum + DS28EL16_HMAC_ROM_ID_OFFSET,
			get_rom_id(drv_data), get_rom_id_len(drv_data));
		/* PAGE DATA */
		memcpy(mac_datum + DS28EL16_HMAC_PAGE_OFFSET,
			get_page_data(drv_data, page), get_page_size(drv_data));
		/* Challenge */
		memcpy(mac_datum + DS28EL16_HMAC_CHALLENGE_OFFSET,
			drv_data->random_bytes, RANDOM_NUM_BYTES);
		/* page */
		mac_datum[DS28EL16_HMAC_PAGE_NO_OFFSET] = page;
		/* man id */
		mac_datum[DS28EL16_HMAC_MAN_ID_OFFSET] = get_main_id(drv_data);

		res->data = mac_datum;
		res->len = MAX_MAC_SOURCE_SIZE;
		return MAXIM_SUCCESS;
	case RES_SN:
		res->data = NULL;
		res->len = 0;
		return MAXIM_SUCCESS;
	default:
		hwlog_err("Wrong mac resource type %u requred in %s\n",
			type, __func__);
		break;
	}

	return MAXIM_FAIL;
}

static int ds28el16_set_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *status = NULL;
	int ret_val = MAXIM_FAIL;
	int sn_page_no;

	if (!pdev)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;

	drv_data = &ds_drv->mdrv;
	status = get_page_status(drv_data);
	sn_page_no = drv_data->ic_des.memory.sn_page;

	/* sn page */
	if (sn_page_no >= get_page_num(drv_data))
		return MAXIM_FAIL;

	mutex_lock(&drv_data->batt_safe_info_lock);
	/* 5s */
	__pm_wakeup_event(drv_data->write_lock, jiffies_to_msecs(5 * HZ));
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (!value)
			break;
		if (*(enum batt_match_type *)value == BATTERY_REMATCHABLE)
			break;
		if (check_page_status_sign(drv_data) &&
			(status[sn_page_no] & DS28EL16_WR_PROTECTION)) {
			ret_val = MAXIM_SUCCESS;
			hwlog_info("batt was already write protection\n");
			break;
		}
		hwlog_info("set battery to write protection\n");
		if (!ds28el16_set_pstatus_retry(drv_data,
			sn_page_no, DS28EL16_WR_PROTECTION))
			ret_val = MAXIM_SUCCESS;
		break;
	default:
		hwlog_err("unsupported battery safety type in %s\n", __func__);
		break;
	}
	__pm_relax(drv_data->write_lock);
	mutex_unlock(&drv_data->batt_safe_info_lock);

	return ret_val;
}

static int ds28el16_get_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *status = NULL;
	int retval = MAXIM_FAIL;
	int sn_page_no;

	if (!pdev || !value)
		return MAXIM_FAIL;
	ds_drv = platform_get_drvdata(pdev);
	if (!ds_drv)
		return MAXIM_FAIL;

	drv_data = &ds_drv->mdrv;
	status = get_page_status(drv_data);
	sn_page_no = drv_data->ic_des.memory.sn_page;

	/* sn page */
	if (sn_page_no >= get_page_num(drv_data))
		return MAXIM_FAIL;

	mutex_lock(&drv_data->batt_safe_info_lock);
	switch (type) {
	case BATT_MATCH_ABILITY:
		/* page status */
		ds28el16_get_pstatus_retry(drv_data);
		if (!check_page_status_sign(drv_data))
			break;

		if (status[sn_page_no] & DS28EL16_WR_PROTECTION)
			*(enum batt_match_type *)value = BATTERY_UNREMATCHABLE;
		else
			*(enum batt_match_type *)value = BATTERY_REMATCHABLE;

		retval = MAXIM_SUCCESS;
		break;
	default:
		hwlog_err("unsupported battery safety type found in %s\n",
			__func__);
		break;
	}
	mutex_unlock(&drv_data->batt_safe_info_lock);

	return retval;
}

static enum batt_ic_type ds28el16_get_ic_type(void)
{
	return MAXIM_DS28EL16_TYPE;
}

/* Battery information initialization */
static int ds28el16_valid(struct maxim_onewire_drv_data *drv_data,
	struct platform_device *pdev)
{
	int ret;
	int i;
	unsigned int myid;
	struct ow_phy_ops *opo = &drv_data->ic_des.phy_ops;
	struct list_head *ow_phy_reg_head = get_owi_phy_list_head();
	struct ow_phy_list *pos = NULL;

	ret = of_property_read_u32(pdev->dev.of_node, "phy-ctrl", &myid);
	if (ret) {
		hwlog_err("No phy controller specified in device tree for %s\n",
			pdev->name);
		return ret;
	}

	/* find the physical controller by matching phanlde number */
	list_for_each_entry(pos, ow_phy_reg_head, node) {
		ret = MAXIM_FAIL;
		if ((myid == pos->dev_phandle) &&
			pos->onewire_phy_register)
			ret = pos->onewire_phy_register(opo);
		if (!ret) {
			hwlog_info("Found phy-ctrl for %s\n", pdev->name);
			break;
		}
	}
	if (ret) {
		hwlog_err("Can't find phy-ctrl for %s\n", pdev->name);
		return ret;
	}

	/* check all onewire physical operations are valid */
	if (!opo->reset) {
		hwlog_err("NULL:opo->reset found in %s\n", __func__);
		return -1;
	}
	if (!opo->read_byte) {
		hwlog_err("NULL:opo->read_byte found in %s\n", __func__);
		return -1;
	}
	if (!opo->write_byte) {
		hwlog_err("NULL:opo->write_byte found in %s\n", __func__);
		return -1;
	}
	if (!opo->set_time_request) {
		hwlog_err("NULL:opo->set_time_request found in %s\n", __func__);
		return -1;
	}
	if (!opo->wait_for_ic) {
		hwlog_err("NULL:opo->wait_for_ic found in %s\n", __func__);
		return -1;
	}

	/* set time request for physic controller first */
	opo->set_time_request(&drv_data->ic_des.trq.ow_trq);

	/* try to get batt_id */
	for (i = 0; i < FIND_IC_RETRY_NUM; i++) {
		ret = ds28el16_get_rom_id(drv_data);
		if (!ret)
			break;
	}
	if (ret) {
		hwlog_err("%s try to get rom id failed in %s\n",
			pdev->name, __func__);
		return ret;
	}

	return 0;
}

int ds28el16_ct_ops_register(struct platform_device *pdev,
	struct batt_ct_ops *bco)
{
	struct ds28el16_drv_data *ds_drv = platform_get_drvdata(g_st_pdev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	const unsigned char partical_secret[DS28EL16_SECRET_SEED_BYTES] = {
		0x11, 0x65, 0xF1, 0x87, 0x32, 0x33, 0x23, 0x71,
		0xF5, 0x41, 0x25, 0x22, 0xAB, 0xD1, 0x0A, 0x01,
		0xA1, 0xB5, 0x32, 0x77, 0x23, 0x31, 0x13, 0x72,
		0xF6, 0x43, 0x35, 0x23, 0xAC, 0x11, 0x31, 0x03,
	};

	(void)pdev;
	if (!ds_drv) {
		hwlog_err("get driver data failed in %s\n", __func__);
		return MAXIM_FAIL;
	}
	drv_data = &ds_drv->mdrv;

	if (ds28el16_valid(drv_data, g_st_pdev)) {
		hwlog_err("%s can't find phy ctl for ic\n", g_st_pdev->name);
		return MAXIM_FAIL;
	}

	if (ds28el16_check_ic_status(g_st_pdev)) {
		hwlog_err("%s ic status was not fine\n", g_st_pdev->name);
		return MAXIM_FAIL;
	}

	get_random_bytes(drv_data->random_bytes, RANDOM_NUM_BYTES);

	memcpy(ds_drv->secret_seed, partical_secret,
		DS28EL16_SECRET_SEED_BYTES);

	bco->get_ic_type = ds28el16_get_ic_type;
	bco->get_batt_type = ds28el16_get_batt_type;
	bco->get_batt_sn = ds28el16_get_sn;
	bco->certification = ds28el16_certification;
	bco->prepare = ds28el16_prepare;
	bco->set_batt_safe_info = ds28el16_set_batt_safe_info;
	bco->get_batt_safe_info = ds28el16_get_batt_safe_info;
	bco->power_down = NULL;

	return MAXIM_SUCCESS;
}

#ifdef ONEWIRE_STABILITY_DEBUG
static ssize_t get_rom_id_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	const unsigned char *rom = NULL;
	unsigned int rom_len;
	int val;
	int ret;

	if (!ds_drv)
		return 0;
	drv_data = &ds_drv->mdrv;

	clear_rom_id_sign(drv_data);
	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_get_id(ds_drv, &rom, &rom_len);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	val = snprintf(buff, PAGE_SIZE, "OK ");
	val += snprintf_array(buff + val, PAGE_SIZE - val,
		(unsigned char *)rom, rom_len);

	return val;
}

static ssize_t get_page_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *page0 = NULL;
	int val;
	int ret;

	if (!ds_drv)
		return 0;
	drv_data = &ds_drv->mdrv;
	page0 = get_page_data(drv_data, 0);

	clear_page_data_sign(drv_data, 0);
	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_get_page_data_retry(drv_data, 0);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	val = snprintf(buff, PAGE_SIZE, "OK ");
	val += snprintf_array(buff + val, PAGE_SIZE - val, page0 - 1,
		get_page_size(drv_data) + 1);

	return val;
}

static ssize_t set_page_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	/* write bind data to page 0 */
	const unsigned char bind_data[DS28EL16_PAGE_SIZE] = {
		0x32, 0x37, 0x36, 0x32, 0x54, 0x59, 0x49, 0x42,
		0x30, 0x38, 0x58, 0x07, 0xF7, 0x60, 0x00, 0x00,
	};
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	int ret;

	if (!ds_drv)
		return 0;
	drv_data = &ds_drv->mdrv;

	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_write_memory(drv_data, 0, bind_data);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	return snprintf(buff, PAGE_SIZE, "OK");
}

static ssize_t set_master_key_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	/* for test */
	const unsigned char master_secret[DS28EL16_MSECRET_BYTES] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	};
	int ret;

	if (!ds_drv)
		return 0;
	drv_data = &ds_drv->mdrv;

	bat_type_apply_mode(BAT_ID_SN);
	/* page3 */
	ret = ds28el16_write_memory(drv_data, 3, master_secret);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	return snprintf(buff, PAGE_SIZE, "OK");
}

static ssize_t page_status_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char *status = NULL;
	int ret;

	if (!ds_drv)
		return 0;
	drv_data = &ds_drv->mdrv;
	status = get_page_status(drv_data) - 1;

	clear_page_status_sign(drv_data);
	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_get_pstatus_retry(drv_data);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");
	/* array ordinal */
	return snprintf(buff, PAGE_SIZE,
		"OK %02x %02x %02x %02x %02x %02x %02x",
		status[0], status[1], status[2], status[3], status[4],
		get_main_id(drv_data), get_version(drv_data));
}

static ssize_t page_status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char page_no;
	unsigned char protection;
	unsigned char set_value;

	if (!ds_drv)
		return -1;
	drv_data = &ds_drv->mdrv;
	/* page no + protection + '\0' */
	if (count != 3)
		return -1;
	/* 16 : buf size */
	if (kstrtou8(buf, 16, &set_value))
		return -1;
	/* binary shift right 4 bits */
	page_no = (set_value & 0xf0) >> 4;
	protection = set_value & 0x0f;
	if ((page_no > DS28EL16_MASTER_PAGE_NO) ||
		(protection > DS28EL16_RW_PROTECTION))
		return -1;

	bat_type_apply_mode(BAT_ID_SN);
	if (ds28el16_set_pstatus_retry(drv_data, page_no, protection)) {
		bat_type_release_mode(false);
		hwlog_info("set page%u protection %x failed\n",
			page_no, protection);
		return -1;
	}
	bat_type_release_mode(false);

	hwlog_info("set page%u protection %x ok\n", page_no, protection);
	return count;
}

static ssize_t get_mac_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char local_mac[TMP_BUF_LEN] = { 0 };
	enum key_cr result;
	struct power_genl_attr res;
	int ret;

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;
	res.data = local_mac;
	res.len = get_mac_data_len(drv_data);

	ds_drv->s_secret_sign = 0;
	clear_mac_data_sign(drv_data);
	bat_type_apply_mode(BAT_ID_SN);
	/* just check communication */
	ret = ds28el16_certification(pdev, &res, &result);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	return snprintf(buff, PAGE_SIZE, "OK");
}

static ssize_t s_secret_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	int ret;

	if (!ds_drv)
		return 0;

	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_s_secret(ds_drv);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR");

	return snprintf(buff, PAGE_SIZE, "OK");
}

static ssize_t battery_sn_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	const unsigned char *sn = NULL;
	unsigned int sn_size;
	unsigned char sn_page;
	int val;
	int ret;

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;
	sn_page = drv_data->ic_des.memory.sn_page;

	clear_page_data_sign(drv_data, sn_page);
	bat_type_apply_mode(BAT_ID_SN);
	ret = ds28el16_get_sn(pdev, NULL, &sn, &sn_size);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERROR to get sn");

	val = snprintf(buff, PAGE_SIZE, "OK Sn Page Data is: ");
	val += snprintf_array(buff + val, PAGE_SIZE - val,
		get_page_data(drv_data, sn_page) - 1,
		get_page_size(drv_data) + 1);
	val += snprintf(buff + val, PAGE_SIZE - val, "sn is: ");
	memcpy(buff + val, sn, sn_size);
	return (val + sn_size);
}

static ssize_t battery_sn_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int i, j;
	unsigned char page_sn[DS28EL16_PAGE_SIZE];
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	unsigned char tmp[DS28EL16_TMP_CHAR_LEN];

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;

	if (count < PRINTABLE_SN_SIZE)
		return -1;

	page_sn[0] = 0;
	for (i = 0; i < SN_CHAR_PRINT_SIZE; i++)
		page_sn[i + 1] = buf[i];

	i = 0;
	j = 0;
	while (i < SN_HEX_PRINT_SIZE) {
		tmp[0] = buf[i + SN_CHAR_PRINT_SIZE];
		if (i + 1 < SN_HEX_PRINT_SIZE)
			tmp[1] = buf[i + 1 + SN_CHAR_PRINT_SIZE];
		else
			tmp[1] = '0';

		tmp[2] = 0; /* array ordinal */
		/* tmp size */
		if (kstrtou8(tmp, 16, page_sn + j + SN_CHAR_PRINT_SIZE + 1))
			return -1;

		j++;
		i += 2;
	}

	bat_type_apply_mode(BAT_ID_SN);
	if (ds28el16_write_memory(drv_data,
		drv_data->ic_des.memory.sn_page, page_sn)) {
		bat_type_release_mode(false);
		return -1;
	}
	bat_type_release_mode(false);

	return count;
}

static ssize_t reset_one_wire_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	struct ow_phy_ops *phy_ops = NULL;
	int ret;

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;
	phy_ops = &drv_data->ic_des.phy_ops;

	bat_type_apply_mode(BAT_ID_SN);
	ret = phy_ops->reset();
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "ERRORreset one wire");

	return snprintf(buff, PAGE_SIZE, "OK reset one wire");
}
#endif /* ONEWIRE_STABILITY_DEBUG */

static ssize_t com_stat_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	int val = 0;
	int i;

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;

	if (!drv_data->com_stat.totals || !drv_data->com_stat.errs)
		return snprintf(buf, PAGE_SIZE, "ERROR stat info");

	for (i = 0; i < drv_data->com_stat.cmds; i++) {
		if ((drv_data->com_stat.cmd_str != 0) &&
			(drv_data->com_stat.cmd_str[i] != 0))
			val += snprintf(buf + val, PAGE_SIZE,
				"%s--total:%u, failed:%u times\n",
				drv_data->com_stat.cmd_str[i],
				drv_data->com_stat.totals[i],
				drv_data->com_stat.errs[i]);
		else
			val += snprintf(buf + val, PAGE_SIZE,
				"%d--total:%u, failed:%u times\n", i,
				drv_data->com_stat.totals[i],
				drv_data->com_stat.errs[i]);
	}

	return val;
}

static ssize_t reset_com_stat_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct ds28el16_drv_data *ds_drv = dev_get_drvdata(dev);
	struct maxim_onewire_drv_data *drv_data = NULL;
	int i;

	if (!ds_drv)
		return 0;

	drv_data = &ds_drv->mdrv;

	if (!drv_data->com_stat.totals || !drv_data->com_stat.errs)
		return snprintf(buff, PAGE_SIZE, "ERROR stat info");

	for (i = 0; i < drv_data->com_stat.cmds; i++) {
		drv_data->com_stat.totals[i] = 0;
		drv_data->com_stat.errs[i] = 0;
	}

	return snprintf(buff, PAGE_SIZE, "OK to Reset stat info");
}

#ifdef ONEWIRE_STABILITY_DEBUG
static DEVICE_ATTR_RO(get_rom_id);
static DEVICE_ATTR_RO(get_page);
static DEVICE_ATTR_RO(set_page);
static DEVICE_ATTR_RW(page_status);
static DEVICE_ATTR_RO(set_master_key);
static DEVICE_ATTR_RO(get_mac);
static DEVICE_ATTR_RO(s_secret);
static DEVICE_ATTR_RW(battery_sn);
static DEVICE_ATTR_RO(reset_one_wire);
#endif /* ONEWIRE_STABILITY_DEBUG */
static DEVICE_ATTR_RO(com_stat);
static DEVICE_ATTR_RO(reset_com_stat);

static const struct attribute *ds28el16_attrs[] = {
#ifdef ONEWIRE_STABILITY_DEBUG
	&dev_attr_get_rom_id.attr,
	&dev_attr_get_page.attr,
	&dev_attr_set_page.attr,
	&dev_attr_page_status.attr,
	&dev_attr_set_master_key.attr,
	&dev_attr_get_mac.attr,
	&dev_attr_s_secret.attr,
	&dev_attr_battery_sn.attr,
	&dev_attr_reset_one_wire.attr,
#endif /* ONEWIRE_STABILITY_DEBUG */
	&dev_attr_com_stat.attr,
	&dev_attr_reset_com_stat.attr,
	NULL, /* sysfs_create_files need last one be NULL */
};

static void ds28el16_memory_release(struct platform_device *pdev)
{
	struct ds28el16_drv_data *ds_drv = platform_get_drvdata(g_st_pdev);

	(void)pdev;
	if (!ds_drv)
		return;

	hwlog_info("ds28el16: memory release\n");
	maxim_destroy_drv_data(&ds_drv->mdrv, g_st_pdev);
	devm_kfree(&g_st_pdev->dev, ds_drv);
	platform_set_drvdata(g_st_pdev, 0);
}

static const char * const err_str[] = {
	[GET_ROM_ID_INDEX] = "GET_ROM_ID",
	[GET_PAGE_DATA_INDEX] = "GET_PAGE_DATA",
	[SET_PAGE_DATA_INDEX] = "SET_PAGE_DATA",
	[GET_PAGE_STATUS_INDEX] = "GET_PAGE_STATUS",
	[SET_PAGE_STATUS_INDEX] = "SET_PAGE_STATUS",
	[SET_S_SECRET_INDEX] = "SET_S_SECRET",
	[GET_MAC_INDEX] = "GET_MAC",
};

static int ds28el16_driver_probe(struct platform_device *pdev)
{
	struct ds28el16_drv_data *ds_drv = NULL;
	struct maxim_onewire_drv_data *drv_data = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!pdev)
		return MAXIM_FAIL;
	hwlog_info("ds28el16: probing\n");
	g_st_pdev = pdev;

	ds_drv = devm_kzalloc(&pdev->dev,
		sizeof(struct ds28el16_drv_data), GFP_KERNEL);
	if (!ds_drv)
		return MAXIM_FAIL;

	memset(ds_drv, 0, sizeof(struct ds28el16_drv_data));
	drv_data = &ds_drv->mdrv;

	if (maxim_drv_data_init(drv_data, pdev, __MAX_COM_ERR_NUM, err_str)) {
		hwlog_err("driver data init failed in %s\n", __func__);
		goto maxim_drv_data_init_fail;
	}

	if (maxim_dev_sys_node_init(drv_data, pdev, ds28el16_attrs)) {
		hwlog_err("dev_sys_node_init failed in %s\n", __func__);
		goto maxim_dev_node_init_fail;
	}

	if (of_property_read_u32(pdev->dev.of_node, "check-key-page-stauts",
		&(ds_drv->check_key_page_status))) {
		hwlog_err("read check-key-page-stauts from dts error\n");
		ds_drv->check_key_page_status = 1;
	}

	/* add ds28el16_ct_ops_register to struct batt_ct_ops_list */
	drv_data->ct_node.ic_memory_release = ds28el16_memory_release;
	drv_data->ct_node.ct_ops_register = ds28el16_ct_ops_register;
	drv_data->ct_node.ic_dev = pdev;
	INIT_LIST_HEAD(&drv_data->ct_node.node);
	add_to_aut_ic_list(&drv_data->ct_node);

	platform_set_drvdata(pdev, ds_drv);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = pdev->dev.driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("ds28el16: probing success\n");

	return MAXIM_SUCCESS;

maxim_dev_node_init_fail:
	maxim_destroy_drv_data(drv_data, pdev);
maxim_drv_data_init_fail:
	devm_kfree(&pdev->dev, ds_drv);
	return MAXIM_FAIL;
}

static int ds28el16_driver_remove(struct platform_device *pdev)
{
	ds28el16_memory_release(g_st_pdev);

	return 0;
}

static const struct of_device_id ds28el16_match_table[] = {
	{ .compatible = "maxim,onewire-sha3", },
	{ /* end */ },
};

static struct platform_driver ds28el16_driver = {
	.probe = ds28el16_driver_probe,
	.remove = ds28el16_driver_remove,
	.driver = {
		.name = "ds28el16",
		.owner = THIS_MODULE,
		.of_match_table = ds28el16_match_table,
	},
};

int __init ds28el16_driver_init(void)
{
	return platform_driver_register(&ds28el16_driver);
}

void __exit ds28el16_driver_exit(void)
{
	platform_driver_unregister(&ds28el16_driver);
}

subsys_initcall_sync(ds28el16_driver_init);
module_exit(ds28el16_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ds28el16 ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
