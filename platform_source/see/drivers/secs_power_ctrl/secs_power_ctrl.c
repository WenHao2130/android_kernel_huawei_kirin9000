/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: secs power ctrl driver
 * Create : 2017/09/19
 */
#include <linux/arm-smccc.h>
#include <linux/version.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/cpumask.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/regulator/consumer.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include "platform_include/see/access_register_smc_id.h"

#define secs_unused(x)        ((void)(x))
#define SECS_POWER_UP         0xAA55A5A5
#define SECS_POWER_DOWN       0x55AA5A5A
#define SECS_SECCLK_EN        0x5A5A55AA
#define SECS_SECCLK_DIS       0xA5A5AA55
#define SECS_CPU0             0
#define SECS_CPU_INDEX_MAX    7
#define SECS_ONLY_CPU0_ONLINE 0xE5A5
#define SECS_OTHER_CPU_ONLINE 0xA5E5
#define SECS_SUSPEND_STATUS   0xA5A5
#define SECS_TRIGGER_TIMER    (jiffies + HZ) /* trigger time 1s */

/* defined for freq control */
enum {
	SECS_HIGH_FREQ_INDEX = 0,
	SECS_LOW_TEMP_FREQ_INDEX,
	SECS_LOW_VOLT_FREQ_INDEX,
	SECS_FREQ_MAX,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void hisi_secs_timer_func(struct timer_list *unused_value);
#else
static void hisi_secs_timer_func(unsigned long unused_value);
#endif

static DEFINE_MUTEX(g_secs_timer_lock);
static DEFINE_MUTEX(g_secs_count_lock);
static DEFINE_MUTEX(g_secs_freq_lock);
static DEFINE_MUTEX(g_secs_mutex_lock);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static DEFINE_TIMER(g_secs_timer, hisi_secs_timer_func);
#else
static DEFINE_TIMER(g_secs_timer, hisi_secs_timer_func, 0, 0);
#endif

static long g_secs_power_ctrl_count;
unsigned long g_secs_suspend_status;
static struct regulator_bulk_data g_regu_burning;
static struct clk *g_secs_clk;
static struct device_node *g_secs_np;
static struct device *g_secs_dev;

unsigned long get_secs_suspend_status(void)
{
	return g_secs_suspend_status;
}

static void hisi_secs_timer_init(void)
{
	int cpu;
	unsigned int flag = SECS_ONLY_CPU0_ONLINE;

	mutex_lock(&g_secs_timer_lock);
	for_each_online_cpu(cpu) {
		if (!timer_pending(&g_secs_timer)) {
			if (cpu == SECS_CPU0) {
				flag = SECS_ONLY_CPU0_ONLINE;
				continue;
			}
			if (cpu <= SECS_CPU_INDEX_MAX) {
				g_secs_timer.expires = SECS_TRIGGER_TIMER;
				add_timer_on(&g_secs_timer, cpu);
				flag = SECS_OTHER_CPU_ONLINE;
				break;
			}
			pr_err("Array overrun\n");
			break;
		}
	}
	if (flag == SECS_ONLY_CPU0_ONLINE)
		pr_err("only cpu0 online secure clk feature disable\n");
	mutex_unlock(&g_secs_timer_lock);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void hisi_secs_timer_func(struct timer_list *unused_value)
#else
static void hisi_secs_timer_func(unsigned long unused_value)
#endif
{
	struct arm_smccc_res res = {0};

	secs_unused(unused_value);
	arm_smccc_smc(ACCESS_REGISTER_FN_MAIN_ID,
		(u64)SECS_SECCLK_EN,
		(u64)0,
		ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL,
		0, 0, 0, 0, &res);
	if (res.a0) {
		pr_err("failed to enable secs secclk 0x%x\n", (int)res.a0);
		return;
	}
	mod_timer(&g_secs_timer, SECS_TRIGGER_TIMER);
}

static int hisi_secs_adjust_freq(void)
{
	u32 secs_freq_info[SECS_FREQ_MAX] = {0};
	int ret = -1;

	if (of_get_property(g_secs_np, "secs-clk-freq-adapt", NULL) == 0)
		return 0;

	if (of_property_read_u32_array(g_secs_np, "secs_clk_rate",
				       secs_freq_info, SECS_FREQ_MAX)) {
		pr_err("can not find secs_freq by dts\n");
		goto err;
	}

	mutex_lock(&g_secs_freq_lock);
	if (clk_set_rate(g_secs_clk, secs_freq_info[SECS_HIGH_FREQ_INDEX])) {
		pr_err("clk_set_rate high freq failed!\n");
		if (clk_set_rate(g_secs_clk,
				 secs_freq_info[SECS_LOW_TEMP_FREQ_INDEX])) {
			pr_err("clk_set_rate low temp freq failed!\n");
			if (clk_set_rate(g_secs_clk,
				secs_freq_info[SECS_LOW_VOLT_FREQ_INDEX])) {
				pr_err("clk_set_rate low volt freq failed!\n");
				goto unlock_err;
			}
			pr_err("clk_set_rate to low volt freq!\n");
		} else {
			pr_err("clk_set_rate to low temp freq!\n");
		}
	}

	if (clk_prepare_enable(g_secs_clk)) {
		pr_err("%s: clk_prepare_enable failed, try low temp freq\n", __func__);
		if (clk_set_rate(g_secs_clk,
				 secs_freq_info[SECS_LOW_TEMP_FREQ_INDEX])) {
			pr_err("clk_set_rate low temp freq failed!\n");
			if (clk_set_rate(g_secs_clk,
				secs_freq_info[SECS_LOW_VOLT_FREQ_INDEX])) {
				pr_err("clk_set_rate low volt freq failed!\n");
				goto unlock_err;
			}
		}

		if (clk_prepare_enable(g_secs_clk)) {
			pr_err("%s: clk_prepare_enable low temp freq failed!\n",
			       __func__);
			if (clk_set_rate(g_secs_clk,
				secs_freq_info[SECS_LOW_VOLT_FREQ_INDEX])) {
				pr_err("clk_set_rate low volt freq failed!\n");
				goto unlock_err;
			}
			if (clk_prepare_enable(g_secs_clk)) {
				pr_err("clk_prepare low volt freq failed!\n");
				goto unlock_err;
			}
		}
	}
	ret = 0;
unlock_err:
	mutex_unlock(&g_secs_freq_lock);
err:
	return ret;
}

int hisi_secs_power_on(void)
{
	int ret = 0;
	struct arm_smccc_res res = {0};

	mutex_lock(&g_secs_count_lock);
	if (g_secs_suspend_status == SECS_SUSPEND_STATUS)
		pr_err("system is suspend, failed to power on secs\n");
	mutex_unlock(&g_secs_count_lock);

	if (of_get_property(g_secs_np, "secs-atfd-power-ctrl", NULL)) {
		ret = hisi_secs_adjust_freq();
		if (ret)
			return ret;
		mutex_lock(&g_secs_count_lock);
		if (g_secs_power_ctrl_count) {
			g_secs_power_ctrl_count++;
			mutex_unlock(&g_secs_count_lock);
			return ret;
		}
		g_secs_power_ctrl_count++;
		hisi_secs_timer_init();

		arm_smccc_smc(ACCESS_REGISTER_FN_MAIN_ID,
			(u64)SECS_POWER_UP,
			(u64)0,
			ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL,
			0, 0, 0, 0, &res);
		ret = (int)res.a0;
		if (ret) {
			pr_err("failed to powerup secs 0x%x\n", ret);
			mutex_unlock(&g_secs_count_lock);
			return ret;
		}

		arm_smccc_smc(ACCESS_REGISTER_FN_MAIN_ID,
			(u64)SECS_SECCLK_EN,
			(u64)0,
			ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL,
			0, 0, 0, 0, &res);
		ret = (int)res.a0;
		if (ret)
			pr_err("failed to enable secs secclk 0x%x\n", ret);
		mutex_unlock(&g_secs_count_lock);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_secs_power_on);

int hisi_secs_power_down(void)
{
	int ret = 0;
	struct arm_smccc_res res = {0};

	mutex_lock(&g_secs_count_lock);
	if (g_secs_suspend_status == SECS_SUSPEND_STATUS)
		pr_err("system is suspend, failed to power down secs\n");
	mutex_unlock(&g_secs_count_lock);

	if (of_get_property(g_secs_np, "secs-clk-freq-adapt", NULL))
		clk_disable_unprepare(g_secs_clk);

	if (of_get_property(g_secs_np, "secs-atfd-power-ctrl", NULL)) {
		mutex_lock(&g_secs_count_lock);
		g_secs_power_ctrl_count--;
		if (g_secs_power_ctrl_count) {
			mutex_unlock(&g_secs_count_lock);
			return ret;
		}
		del_timer_sync(&g_secs_timer);

		arm_smccc_smc(ACCESS_REGISTER_FN_MAIN_ID,
			(u64)SECS_SECCLK_DIS,
			(u64)0,
			ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL,
			0, 0, 0, 0, &res);
		ret = (int)res.a0;
		if (ret) {
			pr_err("failed to disable secs secclk 0x%x\n", ret);
			mutex_unlock(&g_secs_count_lock);
			return ret;
		}

		arm_smccc_smc(ACCESS_REGISTER_FN_MAIN_ID,
			(u64)SECS_POWER_DOWN,
			(u64)0,
			ACCESS_REGISTER_FN_SUB_ID_SECS_POWER_CTRL,
			0, 0, 0, 0, &res);
		ret = (int)res.a0;
		if (ret)
			pr_err("failed to powerdown secs 0x%x\n", ret);
		mutex_unlock(&g_secs_count_lock);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(hisi_secs_power_down);

static int hisi_secs_power_ctrl_probe(struct platform_device *pdev)
{
	int ret = 0;

	g_secs_dev = &pdev->dev;
	g_secs_np = pdev->dev.of_node;

	dev_info(g_secs_dev, "hisi secs power ctrl probe\n");
	if (of_get_property(g_secs_np, "secs-clk-freq-adapt", NULL)) {
		g_secs_clk = devm_clk_get(g_secs_dev, "clk_secs");
		if (IS_ERR_OR_NULL(g_secs_clk)) {
			dev_err(g_secs_dev, "fail get secs clk!\n");
			ret = -ENODEV;
		}
	}
	dev_info(g_secs_dev, "hisi secs power ctrl probe done\n");
	return ret;
}

static int hisi_secs_power_ctrl_remove(struct platform_device *pdev)
{
	regulator_bulk_free(1, &g_regu_burning);
	secs_unused(pdev);
	return 0;
}

static const struct of_device_id g_secs_ctrl_of_match[] = {
	{.compatible = "hisilicon,secs_power_ctrl"},
	{},
};

#ifdef CONFIG_PM
static int secs_power_ctrl_suspend(struct device *dev)
{
	dev_info(dev, "%s: suspend +\n", __func__);
	mutex_lock(&g_secs_count_lock);
	g_secs_suspend_status = SECS_SUSPEND_STATUS;
	mutex_unlock(&g_secs_count_lock);
	dev_info(dev, "%s: secs_power_ctrl_count=%lx\n", __func__,
		 g_secs_power_ctrl_count);
	dev_info(dev, "%s: suspend -\n", __func__);
	return 0;
}

static int secs_power_ctrl_resume(struct device *dev)
{
	dev_info(dev, "%s: resume +\n", __func__);
	mutex_lock(&g_secs_count_lock);
	g_secs_suspend_status = 0;
	mutex_unlock(&g_secs_count_lock);
	dev_info(dev, "%s: secs_power_ctrl_count=%lx\n", __func__,
		 g_secs_power_ctrl_count);
	dev_info(dev, "%s: resume -\n", __func__);
	return 0;
}

static SIMPLE_DEV_PM_OPS(secs_power_ctrl_pm_ops, secs_power_ctrl_suspend,
			 secs_power_ctrl_resume);
#endif

static struct platform_driver g_hisi_secs_ctrl_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "hisi-secs-power-ctrl",
		.of_match_table = of_match_ptr(g_secs_ctrl_of_match),
#ifdef CONFIG_PM
		.pm = &secs_power_ctrl_pm_ops,
#endif
	},
	.probe = hisi_secs_power_ctrl_probe,
	.remove = hisi_secs_power_ctrl_remove,
};

static int __init hisi_secs_power_ctrl_init(void)
{
	int ret;

	pr_info("hisi secs power ctrl init\n");
	ret = platform_driver_register(&g_hisi_secs_ctrl_driver);
	if (ret)
		pr_err("register secs power ctrl driver fail\n");

	return ret;
}
fs_initcall(hisi_secs_power_ctrl_init);
MODULE_DESCRIPTION("Hisilicon secs power ctrl driver");
MODULE_ALIAS("hisi secs power-ctrl module");
MODULE_LICENSE("GPL");
