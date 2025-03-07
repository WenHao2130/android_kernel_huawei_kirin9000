/*
 * hisi_usb_vbus.c
 *
 * Hisi usb vbus irq driver.
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>

#include "hisi_usb_vbus.h"

struct hisi_usb_vbus {
	int vbus_connect_irq;
	int vbus_disconnect_irq;
	int vbus_status;
	bool initialized;
	void *tcpc_data;
};
struct hisi_usb_vbus _vbus = {
	.vbus_connect_irq = 0,
	.vbus_disconnect_irq = 0,
	.vbus_status = 0,
	.initialized = false,
};

struct hisi_usb_vbus *hisi_vbus = &_vbus;

static irqreturn_t hisi_usb_vbus_intr(int irq, void *data)
{
	struct hisi_usb_vbus *vbus = (struct hisi_usb_vbus *)data;

	if (irq == vbus->vbus_connect_irq) {
		pr_info("%s: vbus_connect_irq\n", __func__);
		vbus->vbus_status = 1;
	} else {
		pr_info("%s: vbus_disconnect_irq\n", __func__);
		vbus->vbus_status = 0;
	}

	hisi_tcpc_vbus_irq_handler(vbus->tcpc_data, vbus->vbus_status);

	return IRQ_HANDLED;
}

int hisi_usb_vbus_status(void)
{
	return hisi_vbus->vbus_status;
}

void hisi_usb_vbus_init(void *tcpc_data)
{
	struct hisi_usb_vbus *vbus = hisi_vbus;
	int ret;

	if (vbus->initialized) {
		pr_info("%s: hisi usb vbus already initialized!\n", __func__);
		return;
	}

	if (!tcpc_data) {
		pr_err("%s: tcpc_data NULL\n", __func__);
		return;
	}

	vbus->tcpc_data = tcpc_data;
	vbus->vbus_status = pmic_get_vbus_status();
	vbus->vbus_connect_irq = pmic_get_irq_byname(VBUS_CONNECT);
	vbus->vbus_disconnect_irq = pmic_get_irq_byname(VBUS_DISCONNECT);
	pr_info("%s: vbus_status %d\n", __func__, vbus->vbus_status);

	ret = request_irq(vbus->vbus_connect_irq,
			hisi_usb_vbus_intr,
			IRQF_SHARED | IRQF_NO_SUSPEND,
			"hisi_vbus_connect",
			vbus);
	if (ret) {
		pr_err("%s: request vbus_connect irq err\n", __func__);
		return;
	}

	ret = request_irq(vbus->vbus_disconnect_irq,
			hisi_usb_vbus_intr,
			IRQF_SHARED | IRQF_NO_SUSPEND,
			"hisi_vbus_disconnect",
			vbus);
	if (ret) {
		pr_err("%s: request vbus_disconnect irq err\n", __func__);
		free_irq(vbus->vbus_connect_irq, vbus);
	}
}

void hisi_usb_vbus_exit(void)
{
	struct hisi_usb_vbus *vbus = hisi_vbus;

	free_irq(vbus->vbus_connect_irq, vbus);
	free_irq(vbus->vbus_disconnect_irq, vbus);
	vbus->initialized = false;
}
