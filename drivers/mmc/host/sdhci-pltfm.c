// SPDX-License-Identifier: GPL-2.0-only
/*
 * sdhci-pltfm.c Support for SDHCI platform devices
 * Copyright (c) 2009 Intel Corporation
 *
 * Copyright (c) 2007, 2011 Freescale Semiconductor, Inc.
 * Copyright (c) 2009 MontaVista Software, Inc.
 *
 * Authors: Xiaobo Xie <X.Xie@freescale.com>
 *	    Anton Vorontsov <avorontsov@ru.mvista.com>
 */

/* Supports:
 * SDHCI platform devices
 *
 * Inspired by sdhci-pci.c, by Pierre Ossman
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of.h>
#ifdef CONFIG_PPC
#include <asm/machdep.h>
#endif
#include "sdhci-pltfm.h"
#ifdef CONFIG_COUL_DRV
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h> /* for is_zodiac_battery_exist */
#endif
#include <platform_include/basicplatform/linux/hw_cmdline_parse.h> /* for runmode_is_factory */

unsigned int sdhci_pltfm_clk_get_max_clock(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
#ifdef CONFIG_ZODIAC_MMC
	if (host->quirks2 & SDHCI_QUIRK2_ZODIAC_FPGA)
		return MMC_HS200_MAX_DTR;
#endif

	return (unsigned int)clk_get_rate(pltfm_host->clk);
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_clk_get_max_clock);

static const struct sdhci_ops sdhci_pltfm_ops = {
	.set_clock = sdhci_set_clock,
	.set_bus_width = sdhci_set_bus_width,
	.reset = sdhci_reset,
	.set_uhs_signaling = sdhci_set_uhs_signaling,
};

static bool sdhci_wp_inverted(struct device_node *np)
{
	if (of_get_property(np, "sdhci,wp-inverted", NULL) ||
	    of_get_property(np, "wp-inverted", NULL))
		return true;

	/* Old device trees don't have the wp-inverted property. */
#ifdef CONFIG_PPC
	return machine_is(mpc837x_rdb) || machine_is(mpc837x_mds);
#else
	return false;
#endif /* CONFIG_PPC */
}

#ifdef CONFIG_OF
static void sdhci_get_compatibility(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct device_node *np = pdev->dev.of_node;

	if (!np)
		return;

	if (of_device_is_compatible(np, "fsl,p2020-rev1-esdhc"))
		host->quirks |= SDHCI_QUIRK_BROKEN_DMA;

	if (of_device_is_compatible(np, "fsl,p2020-esdhc") ||
	    of_device_is_compatible(np, "fsl,p1010-esdhc") ||
	    of_device_is_compatible(np, "fsl,t4240-esdhc") ||
	    of_device_is_compatible(np, "fsl,mpc8536-esdhc"))
		host->quirks |= SDHCI_QUIRK_BROKEN_TIMEOUT_VAL;
}
#else
void sdhci_get_compatibility(struct platform_device *pdev) {}
#endif /* CONFIG_OF */

void sdhci_get_property(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	const __be32 *clk = NULL;
	u32 bus_width = 0;
	unsigned int size;
	int runmode_normal;
	int batterystate_exist;

	if (of_device_is_available(np)) {
		if (of_get_property(np, "sdhci,auto-cmd12", NULL))
			host->quirks |= SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12;

		if (of_get_property(np, "sdhci,1-bit-only", NULL) ||
			(of_property_read_u32(np, "bus-width", &bus_width) == 0 &&
				bus_width == 1))
			host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;

		if (bus_width == 4)
			host->mmc->caps |= MMC_CAP_4_BIT_DATA;
		else if (bus_width == 8)
			host->mmc->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;

		if (sdhci_wp_inverted(np))
			host->quirks |= SDHCI_QUIRK_INVERTED_WRITE_PROTECT;

		if (of_get_property(np, "broken-cd", NULL))
			host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;

		if (of_find_property(np, "non-removable", NULL))
			host->mmc->caps |= MMC_CAP_NONREMOVABLE;

		if (of_get_property(np, "no-1-8-v", NULL))
			host->quirks2 |= SDHCI_QUIRK2_NO_1_8_V;

		if (of_device_is_compatible(np, "fsl,p2020-rev1-esdhc"))
			host->quirks |= SDHCI_QUIRK_BROKEN_DMA;

		if (of_find_property(np, "use-pio", NULL)) {
				host->quirks |= SDHCI_QUIRK_BROKEN_DMA;
				host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
		}

		if (of_find_property(np, "use-dma", NULL))
			host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;

		if (!of_find_property(np, "sdhci-adma-64bit", NULL))
			host->quirks2 |= SDHCI_QUIRK2_BROKEN_64_BIT_DMA;

		if (of_device_is_compatible(np, "fsl,p2020-esdhc") ||
			of_device_is_compatible(np, "fsl,p1010-esdhc") ||
			of_device_is_compatible(np, "fsl,t4240-esdhc") ||
			of_device_is_compatible(np, "fsl,mpc8536-esdhc"))
			host->quirks |= SDHCI_QUIRK_BROKEN_TIMEOUT_VAL;

		clk = of_get_property(np, "clock-frequency", &size);
		if (clk && size == sizeof(*clk) && *clk)
			pltfm_host->clock = be32_to_cpup(clk);

		host->quirks2 |= SDHCI_QUIRK2_BROKEN_HS200;
		if (of_find_property(np, "caps2-mmc-ddr50-1_8v", NULL))
			host->mmc->caps |= MMC_CAP_1_8V_DDR;

		if (of_find_property(np, "caps2-mmc-ddr50-1_2v", NULL))
			host->mmc->caps |= MMC_CAP_1_2V_DDR;

		if (of_find_property(np, "caps2-mmc-hs200-1_8v", NULL)) {
			host->mmc->caps2 |= MMC_CAP2_HS200_1_8V_SDR;
			host->quirks2 &= ~SDHCI_QUIRK2_BROKEN_HS200;
		}

		if (of_find_property(np, "caps2-mmc-hs200-1_2v", NULL)) {
			host->mmc->caps2 |= MMC_CAP2_HS200_1_2V_SDR;
			host->quirks2 &= ~SDHCI_QUIRK2_BROKEN_HS200;
		}

		if (of_find_property(np, "caps2-mmc-hs400-1_8v", NULL)) {
			host->mmc->caps2 |= MMC_CAP2_HS200_1_8V_SDR;
			host->mmc->caps2 |= MMC_CAP2_HS400_1_8V;
			host->quirks2 &= ~SDHCI_QUIRK2_BROKEN_HS200;
		}

		if (of_find_property(np, "caps2-mmc-hs400-1_2v", NULL)) {
			host->mmc->caps2 |= MMC_CAP2_HS200_1_2V_SDR;
			host->mmc->caps2 |= MMC_CAP2_HS400_1_2V;
			host->quirks2 &= ~SDHCI_QUIRK2_BROKEN_HS200;
		}

		if (of_find_property(np, "delaymeas_code_1_3T", NULL)) {
			dev_info(&pdev->dev, "set strobe clk 1/3T\n");
			host->flags |= SDHCI_SET_TX_CLK_1_3T;
		}

		if (of_find_property(np, "mmc_disable_tuning_move", NULL)) {
			dev_info(&pdev->dev, "mmc_disable_tuning_move\n");
			host->flags |= SDHCI_WITHOUT_TUNING_MOVE;
		}

		runmode_normal = !runmode_is_factory();

#ifdef CONFIG_COUL_DRV
		batterystate_exist = coul_drv_is_battery_exist();
#else
		batterystate_exist = 0;
#endif
		dev_info(&pdev->dev, "runmode_normal = %d batterystate_exist = %d\n", runmode_normal, batterystate_exist);

		if (of_find_property(np, "caps2-mmc-cache-ctrl", NULL)) {
			dev_info(&pdev->dev, "caps2-mmc-cache-ctrl is set in dts.\n");
			if (runmode_normal || batterystate_exist) {
				dev_info(&pdev->dev, "cache ctrl on\n");
				host->mmc->caps2 |= MMC_CAP2_CACHE_CTRL;
			} else {
				dev_info(&pdev->dev, "cache ctrl off\n");
			}
		}

		if (of_find_property(np, "full-pwr-cycle", NULL))
			host->mmc->caps2 |= MMC_CAP2_FULL_PWR_CYCLE;

		if (of_find_property(np, "caps2-mmc-cmd-queue", NULL)) {
			dev_info(&pdev->dev, "caps2-mmc-cmd-queue is set in dts.\n");
			if (runmode_normal || batterystate_exist) {
				dev_info(&pdev->dev, "caps2-mmc-cmd-queue on\n");
				host->mmc->caps2 |= MMC_CAP2_CMD_QUEUE;
			} else {
				dev_info(&pdev->dev, "caps2-mmc-cmd-queue off\n");
			}
		}

		if (of_find_property(np, "keep-power-in-suspend", NULL))
			host->mmc->pm_caps |= MMC_PM_KEEP_POWER;

		if (of_property_read_bool(np, "wakeup-source") ||
			of_property_read_bool(np, "enable-sdio-wakeup")) /* legacy */
			host->mmc->pm_caps |= MMC_PM_WAKE_SDIO_IRQ;

		if (of_find_property(np, "caps3-mmc-enhanced_strobe-ctrl", NULL))
			host->mmc->caps3 |= MMC_CAP3_ENHANCED_STROBE;

		if (of_find_property(np, "caps3-mmc-cache_flush_barrier-ctrl", NULL))
			host->mmc->caps3 |= MMC_CAP3_CACHE_FLUSH_BARRIER;

		if (of_find_property(np, "caps3-mmc-bkops_auto-ctrl", NULL))
			host->mmc->caps3 |= MMC_CAP3_BKOPS_AUTO_CTRL;

		if (of_find_property(np, "caps2-mmc-HC-erase-size", NULL))
			host->mmc->caps2 |= MMC_CAP2_HC_ERASE_SZ;
	}
}
EXPORT_SYMBOL_GPL(sdhci_get_property);

struct sdhci_host *sdhci_pltfm_init(struct platform_device *pdev,
				    const struct sdhci_pltfm_data *pdata,
				    size_t priv_size)
{
	struct sdhci_host *host;
	void __iomem *ioaddr;
	int irq, ret;

	ioaddr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(ioaddr)) {
		ret = PTR_ERR(ioaddr);
		goto err;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto err;
	}

	host = sdhci_alloc_host(&pdev->dev,
		sizeof(struct sdhci_pltfm_host) + priv_size);

	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	host->ioaddr = ioaddr;
	host->irq = irq;
	host->hw_name = dev_name(&pdev->dev);
	if (pdata && pdata->ops)
		host->ops = pdata->ops;
	else
		host->ops = &sdhci_pltfm_ops;
	if (pdata) {
		host->quirks = pdata->quirks;
		host->quirks2 = pdata->quirks2;
	}

	platform_set_drvdata(pdev, host);

	return host;
err:
	dev_err(&pdev->dev, "%s failed %d\n", __func__, ret);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_init);

void sdhci_pltfm_free(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);

	sdhci_free_host(host);
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_free);

int sdhci_pltfm_register(struct platform_device *pdev,
			const struct sdhci_pltfm_data *pdata,
			size_t priv_size)
{
	struct sdhci_host *host;
	int ret = 0;

	host = sdhci_pltfm_init(pdev, pdata, priv_size);
	if (IS_ERR(host))
		return PTR_ERR(host);

	sdhci_get_property(pdev);

	ret = sdhci_add_host(host);
	if (ret)
		sdhci_pltfm_free(pdev);

	return ret;
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_register);

int sdhci_pltfm_unregister(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	int dead = (readl(host->ioaddr + SDHCI_INT_STATUS) == 0xffffffff);

	sdhci_remove_host(host, dead);
	clk_disable_unprepare(pltfm_host->clk);
	sdhci_pltfm_free(pdev);

	return 0;
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_unregister);

#ifdef CONFIG_PM_SLEEP
int sdhci_pltfm_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	int ret;

	if (host->tuning_mode != SDHCI_TUNING_MODE_3)
		mmc_retune_needed(host->mmc);

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	clk_disable_unprepare(pltfm_host->clk);

	return 0;
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_suspend);

int sdhci_pltfm_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	int ret;

	ret = clk_prepare_enable(pltfm_host->clk);
	if (ret)
		return ret;

	ret = sdhci_resume_host(host);
	if (ret)
		clk_disable_unprepare(pltfm_host->clk);

	return ret;
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_resume);
#endif

const struct dev_pm_ops sdhci_pltfm_pmops = {
	SET_SYSTEM_SLEEP_PM_OPS(sdhci_pltfm_suspend, sdhci_pltfm_resume)
};
EXPORT_SYMBOL_GPL(sdhci_pltfm_pmops);

static int __init sdhci_pltfm_drv_init(void)
{
	pr_info("sdhci-pltfm: SDHCI platform and OF driver helper\n");

	return 0;
}
module_init(sdhci_pltfm_drv_init);

static void __exit sdhci_pltfm_drv_exit(void)
{
}
module_exit(sdhci_pltfm_drv_exit);

MODULE_DESCRIPTION("SDHCI platform and OF driver helper");
MODULE_AUTHOR("Intel Corporation");
MODULE_LICENSE("GPL v2");
