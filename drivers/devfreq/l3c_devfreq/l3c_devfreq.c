/*
 * l3c_devfreq.c
 *
 * L3cache dvfs driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#include <linux/devfreq.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "governor.h"
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <platform_include/basicplatform/linux/ipc_msg.h>
#include <linux/perf_event.h>
#include <linux/cpufreq.h>
#include <trace/events/power.h>
#include <securec.h>

#ifdef CONFIG_LPCPU_CPUFREQ_DT
#include <linux/platform_drivers/lpcpu_cpufreq_dt.h>
#endif

#ifdef CONFIG_DRG
#include <linux/drg.h>
#endif

#ifdef CONFIG_HW_VOTE_L3C_FREQ
#include <linux/platform_drivers/hw_vote.h>
#endif

#define CREATE_TRACE_POINTS
#include <trace/events/l3c_devfreq.h>

#define L3C_DEVFREQ_PLATFORM_DEVICE_NAME		"l3c_devfreq"
#define L3C_DEVFREQ_GOVERNOR_NAME			"l3c_governor"
#define L3C_DEVFREQ_DEFAULT_POLLING_MS			60
#define FREQ_MHZ					1000000
#define FREQ_KHZ					1000
#define L3D_EV						0x2B
#define BUS_ACCESS_EV					0x19

enum ev_index {
	L3D_IDX,
	BUS_ACCESS_IDX,
	NUM_EVENTS
};

enum cluster_idx {
	LITTLE_CLUSTER = 0,
	MID_CLUSTER,
	BIG_CLUSTER,
	NUM_CLUSTERS
};

struct evt_count {
	unsigned long l3_count;
	unsigned long bus_access_count;
};

struct event_data {
	struct perf_event *pevent;
	unsigned long prev_count;
};

struct l3c_hwmon_data {
	struct event_data events[NUM_EVENTS];
	bool active;
	/* to protect operation of hw monitor */
	struct mutex active_lock;
};

struct l3c_hwmon {
	struct evt_count count;
	struct l3c_hwmon_data l3c_hw_data;
};

struct l3c_devfreq_data {
	u32 freq_min;
	u32 freq_max;
};

struct l3c_devfreq {
	struct l3c_devfreq_data l3c_data;
	struct devfreq *devfreq;
	struct platform_device *pdev;
	struct devfreq_dev_profile *devfreq_profile;
#ifdef CONFIG_HW_VOTE_L3C_FREQ
	struct hvdev *l3c_hvdev;
#endif
	u32 polling_ms;
	unsigned long initial_freq;
	u32 hv_supported;

	u32 *load_map;
	int load_map_len;

	unsigned long cur_freq;
	unsigned long load_map_max;
	struct l3c_hwmon *hw;
	unsigned long l3c_bw;

	/* Contains state for the algorithm. It is clean during resume */
	struct {
		unsigned long usec_delta;
		unsigned long last_update;
	} alg;
};

static const struct of_device_id l3c_devfreq_id[] = {
	{.compatible = "hisilicon,l3c_devfreq"},
	{}
};

static int l3c_devfreq_set_target_freq_ipc(struct device *dev,
					   unsigned long freq)
{
	u32 msg[8] = {0};
	int rproc_id = IPC_ACPU_LPM3_MBX_1;
	int ret;

	msg[0] = (OBJ_AP << 24) |
		 (OBJ_L3 << 16) |
		 (CMD_SETTING << 8) |
		 TYPE_FREQ;
	msg[1] = freq;
	msg[1] = msg[1] / FREQ_MHZ;

	ret = RPROC_ASYNC_SEND((rproc_id_t)rproc_id, (mbox_msg_t *)msg, 8);
	if (ret != 0) {
		dev_err(dev, "mailbox send error\n");
		return ret;
	}

	return 0;
}

#ifdef CONFIG_HW_VOTE_L3C_FREQ
static int l3c_devfreq_set_target_freq_hv(struct device *dev,
					  unsigned long freq)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);
	int ret;

	ret = hv_set_freq(l3c->l3c_hvdev, (freq / FREQ_KHZ));
	if (ret != 0) {
		dev_err(dev, "failed to set freq by hw vote\n");
		return ret;
	}

	return 0;
}
#endif

static int l3c_devfreq_set_target_freq(struct device *dev, unsigned long freq)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);

#ifdef CONFIG_HW_VOTE_L3C_FREQ
	if (l3c->hv_supported != 0)
		return l3c_devfreq_set_target_freq_hv(dev, freq);
	else
		return l3c_devfreq_set_target_freq_ipc(dev, freq);
#else
	return l3c_devfreq_set_target_freq_ipc(dev, freq);
#endif
}

static inline void l3c_devfreq_opp_put(struct dev_pm_opp *opp)
{
	dev_pm_opp_put(opp);
}

static int l3c_devfreq_target(struct device *dev,
			      unsigned long *_freq, u32 flags)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);
	struct dev_pm_opp *opp = NULL;
	unsigned long freq;
	int ret;

	opp = devfreq_recommended_opp(dev, _freq, flags);
	if (IS_ERR(opp)) {
		dev_err(dev, "failed to get Operating Performance Point\n");
		return PTR_ERR(opp);
	}
	freq = dev_pm_opp_get_freq(opp);
	l3c_devfreq_opp_put(opp);

	if (freq == l3c->cur_freq)
		return 0;

	trace_l3c_devfreq_target(freq);
	trace_clock_set_rate("l3cache", freq, raw_smp_processor_id());

	/* Set requested freq */
	ret = l3c_devfreq_set_target_freq(dev, freq);
	if (ret == 0)
		l3c->cur_freq = freq;

	*_freq = freq;
	return ret;
}

/* perf event counter */
#define MAX_COUNT_LIM		    0xFFFFFFFFFFFFFFFF
static unsigned long l3c_devfreq_read_event(struct event_data *event)
{
	unsigned long ev_count;
	u64 total, enabled, running;

	if (IS_ERR_OR_NULL(event->pevent))
		return 0;

	total = perf_event_read_value(event->pevent, &enabled, &running);
	if (total >= event->prev_count)
		ev_count = total - event->prev_count;
	else
		ev_count = (MAX_COUNT_LIM - event->prev_count) + total;

	event->prev_count = total;

	return ev_count;
}

static void l3c_devfreq_read_perf_counters(struct l3c_hwmon *hw)
{
	struct l3c_hwmon_data *hw_data = &hw->l3c_hw_data;

	mutex_lock(&hw_data->active_lock);
	if (!hw_data->active) {
		mutex_unlock(&hw_data->active_lock);
		return;
	}

	hw->count.l3_count =
		l3c_devfreq_read_event(&hw_data->events[L3D_IDX]);

	hw->count.bus_access_count =
		l3c_devfreq_read_event(&hw_data->events[BUS_ACCESS_IDX]);

	mutex_unlock(&hw_data->active_lock);
}

static void l3c_devfreq_delete_events(struct l3c_hwmon_data *hw_data)
{
	int i;

	for (i = 0; i < NUM_EVENTS; i++) {
		hw_data->events[i].prev_count = 0;
		if (hw_data->events[i].pevent) {
			perf_event_release_kernel(hw_data->events[i].pevent);
			hw_data->events[i].pevent = NULL;
		}
	}
}

static void l3c_devfreq_stop_hwmon(struct l3c_hwmon *hw)
{
	struct l3c_hwmon_data *hw_data = &hw->l3c_hw_data;

	mutex_lock(&hw_data->active_lock);
	if (!hw_data->active) {
		mutex_unlock(&hw_data->active_lock);
		return;
	}
	hw_data->active = false;

	l3c_devfreq_delete_events(hw_data);
	hw->count.l3_count = 0;
	hw->count.bus_access_count = 0;

	mutex_unlock(&hw_data->active_lock);
}

static struct perf_event_attr *l3c_devfreq_alloc_attr(void)
{
	struct perf_event_attr *attr = NULL;

	attr = kzalloc(sizeof(*attr), GFP_KERNEL);
	if (attr == NULL)
		return ERR_PTR(-ENOMEM);
#ifdef CONFIG_NAMTSO_PMU
	attr->type = PERF_TYPE_NAMTSO;
#else
	attr->type = PERF_TYPE_DSU;
#endif
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;
	return attr;
}

static int l3c_devfreq_set_events(struct l3c_hwmon_data *hw_data, int cpu)
{
	struct perf_event *pevent = NULL;
	struct perf_event_attr *attr = NULL;
	int err;

	/* Allocate an attribute for event initialization */
	attr = l3c_devfreq_alloc_attr();
	if (IS_ERR(attr)) {
		pr_debug("l3c_devfreq:alloc attr failed\n");
		return PTR_ERR(attr);
	}

	attr->config = L3D_EV;
	pevent = perf_event_create_kernel_counter(attr, cpu, NULL, NULL, NULL);
	if (IS_ERR_OR_NULL(pevent)) {
		pr_debug("perf event create failed, config = 0x%x\n",
			 (unsigned int)attr->config);
		goto err_out;
	}
	hw_data->events[L3D_IDX].pevent = pevent;
	perf_event_enable(hw_data->events[L3D_IDX].pevent);

	attr->config = BUS_ACCESS_EV;
	pevent = perf_event_create_kernel_counter(attr, cpu, NULL, NULL, NULL);
	if (IS_ERR_OR_NULL(pevent)) {
		pr_debug("perf event create failed, config = 0x%x\n",
			 (unsigned int)attr->config);
		goto err_l3d;
	}
	hw_data->events[BUS_ACCESS_IDX].pevent = pevent;
	perf_event_enable(hw_data->events[BUS_ACCESS_IDX].pevent);

	kfree(attr);
	return 0;

err_l3d:
	perf_event_disable(hw_data->events[L3D_IDX].pevent);
	perf_event_release_kernel(hw_data->events[L3D_IDX].pevent);
	hw_data->events[L3D_IDX].pevent = NULL;
err_out:
	err = PTR_ERR(pevent);
	kfree(attr);
	return err;
}

static int l3c_devfreq_start_hwmon(struct l3c_hwmon *hw)
{
	int ret = 0;
	struct l3c_hwmon_data *hw_data = &hw->l3c_hw_data;
	/* cpu must be 0 */
	int cpu = 0;

	mutex_lock(&hw_data->active_lock);
	if (hw_data->active)
		goto exit;

	ret = l3c_devfreq_set_events(hw_data, cpu);
	if (ret != 0) {
		pr_info("l3c_devfreq:perf event init failed on CPU%d\n", cpu);
		WARN_ON_ONCE(1);
		goto exit;
	}

	hw_data->active = true;

exit:
	mutex_unlock(&hw_data->active_lock);
	return ret;
}

static int l3c_devfreq_get_dev_status(struct device *dev,
				      struct devfreq_dev_status *stat)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);
	unsigned long const usec = ktime_to_us((unsigned int)ktime_get());
	unsigned long delta;

	l3c_devfreq_read_perf_counters(l3c->hw);

	delta = usec - l3c->alg.last_update;
	l3c->alg.last_update = usec;
	l3c->alg.usec_delta = delta;

	stat->current_frequency = l3c->cur_freq;

	/*
	 * Although this is not yet suitable for use with the simple ondemand
	 * governor we'll fill these usage statistics.
	 */
	stat->total_time = delta;

	return 0;
}

static int l3c_get_freq_from_load(struct l3c_devfreq *l3c)
{
	struct devfreq_dev_profile *dev_profile = l3c->devfreq_profile;
	int i;

	for (i = 0; i < l3c->load_map_len; i++) {
		if (l3c->l3c_bw < l3c->load_map[i])
			break;
	}

	if (i == l3c->load_map_len)
		return l3c->l3c_data.freq_max;

	if ((unsigned int)i < dev_profile->max_state)
		return dev_profile->freq_table[i];
	else
		return l3c->l3c_data.freq_max;
}

static unsigned long l3c_devfreq_calc_next_freq(struct l3c_devfreq *l3c)
{
	unsigned long target_freq;
	struct l3c_devfreq_data *data = &l3c->l3c_data;
	unsigned long tmp_target = data->freq_min;
	unsigned long l3c_bw;
	unsigned long l3c_hit_bw = 0;

	/*
	 * bw_rate = total_access_count / cycle / 2
	 * curr_freq = cycle / time
	 * normalized bw_rate = bw_rate X curr_freq / max_freq
	 */
	if (l3c->hw->count.l3_count > 0 &&
	    l3c->hw->count.bus_access_count > 0 &&
	    l3c->alg.usec_delta > 0 &&
	    data->freq_max > 0)
		l3c_bw = ((l3c->hw->count.l3_count +
			  l3c->hw->count.bus_access_count) >> 1) * 1000 /
			 l3c->alg.usec_delta * 1000 /
			 (data->freq_max / 1000);
	else
		l3c_bw = 0;

	l3c->l3c_bw = l3c_bw;

	if (l3c->alg.usec_delta > 0 && data->freq_max > 0)
		l3c_hit_bw = (l3c->hw->count.l3_count >> 1) * 1000 /
			     l3c->alg.usec_delta * 1000 /
			     (data->freq_max / 1000);

	trace_l3c_devfreq_counter_info(l3c->hw->count.l3_count,
				       l3c->hw->count.bus_access_count);

	trace_l3c_devfreq_bw_info(l3c->alg.usec_delta,
				  l3c->cur_freq, l3c_bw, l3c_hit_bw);

	target_freq = (unsigned int)l3c_get_freq_from_load(l3c);
	target_freq = max(target_freq, tmp_target);

	/* bw is invalid by FCM idle, assign min freq to L3 */
	if (l3c_bw > l3c->load_map_max || l3c_hit_bw > l3c->load_map_max)
		target_freq = data->freq_min;

	return target_freq;
}

static int l3c_devfreq_governor_get_target_freq(struct devfreq *df,
						unsigned long *freq)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(df->dev.parent);
	struct l3c_devfreq_data *data = &l3c->l3c_data;
	int err;

	err = devfreq_update_stats(df);
	if (err != 0)
		return err;

	*freq = l3c_devfreq_calc_next_freq(l3c);

	if (*freq < data->freq_min)
		*freq = data->freq_min;
	else if (*freq > data->freq_max)
		*freq = data->freq_max;

	return 0;
}

static int l3c_devfreq_governor_event_handler(struct devfreq *devfreq,
					      unsigned int event, void *data)
{
	int ret = 0;

	switch (event) {
	case DEVFREQ_GOV_START:
		devfreq_monitor_start(devfreq);
		break;

	case DEVFREQ_GOV_STOP:
		devfreq_monitor_stop(devfreq);
		break;

	case DEVFREQ_GOV_SUSPEND:
		devfreq_monitor_suspend(devfreq);
		break;

	case DEVFREQ_GOV_RESUME:
		devfreq_monitor_resume(devfreq);
		break;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	case DEVFREQ_GOV_UPDATE_INTERVAL:
		devfreq_update_interval(devfreq, (unsigned int *)data);
#else
	case DEVFREQ_GOV_INTERVAL:
		devfreq_interval_update(devfreq, (unsigned int *)data);
#endif
		break;

	default:
		break;
	}

	return ret;
}

static struct devfreq_governor l3c_devfreq_governor = {
	.name = L3C_DEVFREQ_GOVERNOR_NAME,
	.immutable = 1,
	.get_target_freq = l3c_devfreq_governor_get_target_freq,
	.event_handler = l3c_devfreq_governor_event_handler,
};

static int l3c_devfreq_reinit_device(struct device *dev)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);

	/* Clean the algorithm statistics and start from scrach */
	(void)memset_s(&l3c->alg, sizeof(l3c->alg), 0, sizeof(l3c->alg));
	l3c->alg.last_update = (unsigned int)ktime_to_us(ktime_get());

	return 0;
}

static int l3c_devfreq_setup_devfreq_profile(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	struct devfreq_dev_profile *df_profile = NULL;

	l3c->devfreq_profile = devm_kzalloc(&pdev->dev,
					    sizeof(struct devfreq_dev_profile),
					    GFP_KERNEL);
	if (IS_ERR_OR_NULL(l3c->devfreq_profile)) {
		dev_err(&pdev->dev, "no memory.\n");
		return PTR_ERR(l3c->devfreq_profile);
	}

	df_profile = l3c->devfreq_profile;

	df_profile->target = l3c_devfreq_target;
	df_profile->get_dev_status = l3c_devfreq_get_dev_status;
	df_profile->polling_ms = l3c->polling_ms;
	df_profile->initial_freq = l3c->initial_freq;

	return 0;
}

static int l3c_devfreq_load_map(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	struct device_node *opp_np = NULL;
	struct device_node *np = NULL;
	struct dev_pm_opp *opp = NULL;
	int ret;
	u64 rate;
	int i = 0;

	l3c->load_map_len = dev_pm_opp_get_opp_count(&pdev->dev);
	if (l3c->load_map_len <= 0) {
		dev_err(&pdev->dev, "Fail to get OPP number\n");
		ret = -ENODEV;
		goto err;
	}

	l3c->load_map = devm_kcalloc(&pdev->dev, l3c->load_map_len,
				     sizeof(*l3c->load_map),
				     GFP_KERNEL);

	if (IS_ERR_OR_NULL(l3c->load_map)) {
		dev_err(&pdev->dev, "load_map alloc fail\n");
		ret = -ENOMEM;
		goto err;
	}

	opp_np = dev_pm_opp_of_get_opp_desc_node(&pdev->dev);
	for_each_available_child_of_node(opp_np, np) {
		ret = of_property_read_u64(np, "opp-hz", &rate);
		if (ret != 0) {
			dev_err(&pdev->dev, "get opp-hz fail\n");
			goto err;
		}

		opp = dev_pm_opp_find_freq_exact(&pdev->dev, rate, true);
		if (!IS_ERR(opp)) {
			ret = of_property_read_u32(np, "load-map",
						   &l3c->load_map[i]);
			if (ret != 0) {
				dev_err(&pdev->dev, "get load-map fail\n");
				goto err;
			}
			i++;
			if (i >= l3c->load_map_len)
				break;
		} else {
			continue;
		}
	}

	l3c->load_map_max = l3c->load_map[l3c->load_map_len - 1];

	dev_dbg(&pdev->dev, "load_map_max = %lu\n",
		(unsigned long)l3c->load_map_max);

	for (i = 0; i < l3c->load_map_len; i++)
		dev_dbg(&pdev->dev, "load_map[%d] = %lu\n",
			i, (unsigned long)l3c->load_map[i]);

	ret = 0;
err:
	return ret;
}

static int l3c_devfreq_parse_dt(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	u32 tmp_init_freq;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,l3c_devfreq");
	if (np == NULL) {
		dev_err(&pdev->dev,
			"l3c_devfreq No compatible node found\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "init-freq", &tmp_init_freq);
	if (ret != 0) {
		dev_err(&pdev->dev, "no init-freq\n");
		ret = -ENODEV;
		goto err;
	}
	l3c->initial_freq = (unsigned long)tmp_init_freq * FREQ_KHZ;
	l3c->cur_freq = l3c->initial_freq;
	dev_dbg(&pdev->dev, "initial_freq = %lu\n", l3c->initial_freq);

	ret = of_property_read_u32(np, "hv-supported",
				   &l3c->hv_supported);
	if (ret != 0) {
		dev_err(&pdev->dev, "no hv-supported\n");
		ret = -ENODEV;
		goto err;
	}
	dev_err(&pdev->dev, "hv_supported = %u\n", l3c->hv_supported);

	ret = of_property_read_u32(np, "polling", &l3c->polling_ms);
	if (ret != 0)
		l3c->polling_ms = L3C_DEVFREQ_DEFAULT_POLLING_MS;
	dev_dbg(&pdev->dev, "polling_ms = %d\n", l3c->polling_ms);

err:
	of_node_put(np);
	return ret;
}

#define VERSION_ELEMENTS	1
static int l3c_devfreq_setup(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	struct devfreq_dev_profile *df_profile = NULL;
	struct l3c_devfreq_data *data = NULL;
	int ret;
#ifdef CONFIG_LPCPU_CPUFREQ_DT
	struct opp_table *opp_table = NULL;
	unsigned int cpu_version;

	/* get support cpu target from cpu dtsi */
	ret = lpcpu_get_cpu_version(&cpu_version);
	if (ret != 0) {
		dev_err(&pdev->dev, "%s: failed to get cpu version: %d\n",
			__func__, ret);
		return -ENODEV;
	}

	opp_table = dev_pm_opp_set_supported_hw(&pdev->dev, &cpu_version,
						VERSION_ELEMENTS);
	if (IS_ERR(opp_table)) {
		ret = PTR_ERR(opp_table);
		dev_err(&pdev->dev, "%s: failed to set supported hw: %d\n",
			__func__, ret);
		return ret;
	}
#endif

	if (dev_pm_opp_of_add_table(&pdev->dev)) {
		dev_err(&pdev->dev, "device add opp table failed.\n");
		ret = -ENODEV;
#ifdef CONFIG_LPCPU_CPUFREQ_DT
		goto out_put_hw;
#else
		return ret;
#endif
	}

	ret = l3c_devfreq_setup_devfreq_profile(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "device devfreq_profile setup failed.\n");
		goto free_opp_table;
	}
	df_profile = l3c->devfreq_profile;

	l3c->alg.last_update = (unsigned int)ktime_to_us(ktime_get());

	l3c->devfreq = devm_devfreq_add_device(&pdev->dev,
					       l3c->devfreq_profile,
					       L3C_DEVFREQ_GOVERNOR_NAME, NULL);

	if (IS_ERR_OR_NULL(l3c->devfreq)) {
		dev_err(&pdev->dev, "registering to devfreq failed.\n");
		ret = PTR_ERR(l3c->devfreq);
		goto free_opp_table;
	}

	if (df_profile->max_state == 0 || df_profile->freq_table == NULL) {
		dev_err(&pdev->dev, "max_state or freq_table is invalid\n");
		ret = -ENOMEM;
		goto remove_dev;
	}

	ret = l3c_devfreq_load_map(pdev);
	if (ret != 0)
		goto remove_dev;

	if (df_profile->max_state != (unsigned int)l3c->load_map_len) {
		dev_err(&pdev->dev, "max_state not equal load_map_len\n");
		ret = -EINVAL;
		goto remove_dev;
	}

	data = &l3c->l3c_data;
	data->freq_min = df_profile->freq_table[0];
	data->freq_max = df_profile->freq_table[df_profile->max_state - 1];

#ifdef CONFIG_DRG
	drg_devfreq_register(l3c->devfreq);
#endif
	mutex_lock(&l3c->devfreq->lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	l3c->devfreq->min_freq = data->freq_min;
	l3c->devfreq->max_freq = data->freq_max;
#else
	l3c->devfreq->scaling_min_freq = data->freq_min;
	l3c->devfreq->scaling_max_freq = data->freq_max;
#endif
	mutex_unlock(&l3c->devfreq->lock);

	return 0;

remove_dev:
	devm_devfreq_remove_device(&pdev->dev, l3c->devfreq);
free_opp_table:
	dev_pm_opp_of_remove_table(&pdev->dev);
#ifdef CONFIG_LPCPU_CPUFREQ_DT
out_put_hw:
	dev_pm_opp_put_supported_hw(opp_table);
#endif
	return ret;
}

static void l3c_devfreq_unsetup(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);

#ifdef CONFIG_DRG
	drg_devfreq_unregister(l3c->devfreq);
#endif

	devm_devfreq_remove_device(&pdev->dev, l3c->devfreq);
	dev_pm_opp_of_remove_table(&pdev->dev);
}

static int l3c_devfreq_init_device(struct platform_device *pdev)
{
	l3c_devfreq_reinit_device(&pdev->dev);
	return 0;
}

static int l3c_devfreq_hwmon_setup(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	struct l3c_hwmon *hw = NULL;
	int ret;

	hw = devm_kzalloc(&pdev->dev, sizeof(*hw), GFP_KERNEL);
	if (IS_ERR_OR_NULL(hw))
		return -ENOMEM;

	mutex_init(&hw->l3c_hw_data.active_lock);

	ret = l3c_devfreq_start_hwmon(hw);
	if (ret != 0) {
		dev_dbg(&pdev->dev, "start hwmon failed in setup.\n");
		return -ENODEV;
	}

	l3c->hw = hw;

	return ret;
}

static void l3c_devfreq_hwmon_destroy(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);

	l3c_devfreq_stop_hwmon(l3c->hw);
}

static int l3c_devfreq_suspend(struct device *dev)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);
	int ret;

	ret = devfreq_suspend_device(l3c->devfreq);
	if (ret < 0)
		dev_err(dev, "failed to suspend devfreq device\n");

	return ret;
}

static int l3c_devfreq_resume(struct device *dev)
{
	struct l3c_devfreq *l3c = dev_get_drvdata(dev);
	int ret;

	l3c_devfreq_reinit_device(dev);
	ret = devfreq_resume_device(l3c->devfreq);
	if (ret < 0)
		dev_err(dev, "failed to resume devfreq device\n");

	return ret;
}

static SIMPLE_DEV_PM_OPS(l3c_devfreq_pm,
			 l3c_devfreq_suspend, l3c_devfreq_resume);

#ifdef CONFIG_L3C_DEVFREQ_DEBUG
#define LOCAL_BUF_MAX       128
static ssize_t show_load_map(struct device *dev,
			     struct device_attribute *attr __maybe_unused, char *buf)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3c_devfreq *l3c = dev_get_drvdata(devfreq->dev.parent);
	ssize_t count = 0;
	int ret;
	int i;

	for (i = 0; i < l3c->load_map_len; i++) {
		ret = snprintf_s(buf + count,
				 (PAGE_SIZE - count),
				 (PAGE_SIZE - count - 1),
				 "load_map[%d] = %lu\n",
				 i,
				 (unsigned long)l3c->load_map[i]);
		if (ret < 0)
			goto err;
		count += ret;
		if ((unsigned int)count >= PAGE_SIZE)
			goto err;
	}

err:
	return count;
}

static int cmd_parse(char *para_cmd, char *argv[], int max_args)
{
	int para_id = 0;

	while (*para_cmd != '\0') {
		if (para_id >= max_args)
			break;

		while (*para_cmd == ' ')
			para_cmd++;

		if (*para_cmd == '\0')
			break;

		argv[para_id] = para_cmd;
		para_id++;

		while ((*para_cmd != ' ') && (*para_cmd != '\0'))
			para_cmd++;

		if (*para_cmd == '\0')
			break;

		*para_cmd = '\0';
		para_cmd++;
	}

	return para_id;
}

static ssize_t store_load_map(struct device *dev,
			      struct device_attribute *attr __maybe_unused,
			      const char *buf, size_t count)
{
	struct devfreq *devfreq = to_devfreq(dev);
	struct l3c_devfreq *l3c = dev_get_drvdata(devfreq->dev.parent);
	char local_buf[LOCAL_BUF_MAX] = {0};
	char *argv[2] = {0};
	unsigned int idx, value;
	int argc, ret;

	if (count >= sizeof(local_buf))
		return -ENOMEM;

	ret = strncpy_s(local_buf, LOCAL_BUF_MAX, buf,
			min_t(size_t, sizeof(local_buf) - 1, count));
	if (ret != EOK) {
		dev_err(dev, "copy string to local_buf failed\n");
		ret = -EINVAL;
		goto err_handle;
	}

	argc = cmd_parse(local_buf, argv, ARRAY_SIZE(argv));
	if (argc != 2) {
		dev_err(dev, "error, only surport two para\n");
		ret = -EINVAL;
		goto err_handle;
	}

	ret = sscanf_s(argv[0], "%u", &idx);
	if (ret != 1) {
		dev_err(dev, "parse idx error %s\n", argv[0]);
		ret = -EINVAL;
		goto err_handle;
	}

	ret = sscanf_s(argv[1], "%u", &value);
	if (ret != 1) {
		dev_err(dev, "parse value error %s\n", argv[1]);
		ret = -EINVAL;
		goto err_handle;
	}

	mutex_lock(&l3c->devfreq->lock);
	if (idx < (unsigned int)l3c->load_map_len) {
		l3c->load_map[idx] = value;
		ret = (int)count;
	} else {
		dev_err(dev, "invalid idx %u\n", idx);
		ret = -EINVAL;
	}
	mutex_unlock(&l3c->devfreq->lock);

err_handle:
	return ret;
}

#define L3C_DEVFREQ_ATTR_RW(_name) \
	static DEVICE_ATTR(_name, 0640, show_##_name, store_##_name)

L3C_DEVFREQ_ATTR_RW(load_map);

static struct attribute *dev_entries[] = {
	&dev_attr_load_map.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = "l3c_dvfs",
	.attrs = dev_entries,
};
#endif

static int l3c_devfreq_probe(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = NULL;
	int ret;

	dev_err(&pdev->dev, "registering l3c devfreq.\n");

	l3c = devm_kzalloc(&pdev->dev, sizeof(*l3c), GFP_KERNEL);
	if (IS_ERR_OR_NULL(l3c))
		return -ENOMEM;

	platform_set_drvdata(pdev, l3c);
	l3c->pdev = pdev;

	ret = l3c_devfreq_parse_dt(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "devfreq parse dt failed %d\n", ret);
		goto failed;
	}

	ret = l3c_devfreq_hwmon_setup(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "hwmon setup failed %d\n", ret);
		goto failed;
	}

	ret = l3c_devfreq_setup(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "devfreq setup failed %d\n", ret);
		goto err_hwmon;
	}

	ret = l3c_devfreq_init_device(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "devfreq init device failed %d\n", ret);
		goto err_unsetup;
	}

#ifdef CONFIG_HW_VOTE_L3C_FREQ
	if (l3c->hv_supported != 0) {
		l3c->l3c_hvdev = hvdev_register(&pdev->dev, "l3-freq", "vote-src-1");
		if (IS_ERR_OR_NULL(l3c->l3c_hvdev)) {
			dev_err(&pdev->dev, "register hvdev fail!\n");
			ret = -ENODEV;
			goto err_unsetup;
		}
	} else {
		l3c->l3c_hvdev = NULL;
	}
#endif

#ifdef CONFIG_L3C_DEVFREQ_DEBUG
	ret = sysfs_create_group(&l3c->devfreq->dev.kobj, &dev_attr_group);
	if (ret != 0) {
		dev_err(&pdev->dev, "sysfs create err %d\n", ret);
		goto err_unreg_trans;
	}
#endif

	dev_err(&pdev->dev, "probe success\n");
	return 0;

#ifdef CONFIG_L3C_DEVFREQ_DEBUG
err_unreg_trans:
#endif
#ifdef CONFIG_HW_VOTE_L3C_FREQ
	if (l3c->hv_supported != 0)
		(void)hvdev_remove(l3c->l3c_hvdev);
#endif
err_unsetup:
	l3c_devfreq_unsetup(pdev);
err_hwmon:
	l3c_devfreq_hwmon_destroy(pdev);
failed:
	dev_err(&pdev->dev, "failed to register driver, err %d.\n", ret);
	return ret;
}

static int l3c_devfreq_remove(struct platform_device *pdev)
{
	struct l3c_devfreq *l3c = platform_get_drvdata(pdev);
	int ret;

	dev_err(&pdev->dev, "unregistering l3c devfreq device.\n");

	ret = l3c_devfreq_set_target_freq(&pdev->dev, l3c->initial_freq);

#ifdef CONFIG_L3C_DEVFREQ_DEBUG
	sysfs_remove_group(&l3c->devfreq->dev.kobj, &dev_attr_group);
#endif
	l3c_devfreq_unsetup(pdev);
	l3c_devfreq_hwmon_destroy(pdev);

	return ret;
}

MODULE_DEVICE_TABLE(of, l3c_devfreq_id);

static struct platform_driver l3c_devfreq_driver = {
	.probe = l3c_devfreq_probe,
	.remove = l3c_devfreq_remove,
	.driver = {
		.name = L3C_DEVFREQ_PLATFORM_DEVICE_NAME,
		.of_match_table = l3c_devfreq_id,
		.pm = &l3c_devfreq_pm,
		.owner = THIS_MODULE,
	},
};

static int __init l3c_devfreq_init(void)
{
	int ret;

	ret = devfreq_add_governor(&l3c_devfreq_governor);
	if (ret != 0) {
		pr_err("%s: failed to add governor: %d.\n", __func__, ret);
		return ret;
	}

	ret = platform_driver_register(&l3c_devfreq_driver);
	if (ret != 0) {
		ret = devfreq_remove_governor(&l3c_devfreq_governor);
		if (ret != 0)
			pr_err("%s: failed to remove governor: %d.\n",
			       __func__, ret);
	}

	return ret;
}

static void __exit l3c_devfreq_exit(void)
{
	int ret;

	ret = devfreq_remove_governor(&l3c_devfreq_governor);
	if (ret != 0)
		pr_err("%s: failed to remove governor: %d.\n", __func__, ret);

	platform_driver_unregister(&l3c_devfreq_driver);
}

late_initcall(l3c_devfreq_init)

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("L3cache devfreq driver");
