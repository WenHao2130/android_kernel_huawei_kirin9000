/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM ipa_thermal_trace

#if !defined(_IPA_THERMAL_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _IPA_THERMAL_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(IPA_boost,
	TP_PROTO(pid_t pid, struct thermal_zone_device *tz, int boost,
		 unsigned int boost_timeout),

	TP_ARGS(pid, tz, boost, boost_timeout),

	TP_STRUCT__entry(
		__field(pid_t, pid)
		__field(int, tz_id)
		__field(int, boost)
		__field(unsigned int, boost_timeout)
	),

	TP_fast_assign(
		__entry->pid = pid;
		__entry->tz_id = tz->id;
		__entry->boost = boost;
		__entry->boost_timeout = boost_timeout;
	),

	TP_printk("pid=%d tz_id=%d boost=%d timeout=%u",
		__entry->pid,
		__entry->tz_id,
		__entry->boost,
		__entry->boost_timeout)
);

TRACE_EVENT(thermal_power_actor_cpu_limit,
	TP_PROTO(const struct cpumask *cpus, unsigned int freq,
		unsigned long cdev_state, u32 power),

	TP_ARGS(cpus, freq, cdev_state, power),

	TP_STRUCT__entry(
		__bitmask(cpumask, num_possible_cpus())
		__field(unsigned int,  freq      )
		__field(unsigned long, cdev_state)
		__field(u32,           power     )
	),

	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				num_possible_cpus());
		__entry->freq = freq;
		__entry->cdev_state = cdev_state;
		__entry->power = power;
	),

	TP_printk("cpus=%s freq=%u cdev_state=%lu power=%u",
		__get_bitmask(cpumask), __entry->freq, __entry->cdev_state,
		__entry->power)
);

TRACE_EVENT(thermal_power_actor_gpu_get_power,
	TP_PROTO(unsigned long freq, unsigned long load,
		 unsigned long dynamic_power, unsigned long static_power, unsigned long req_power),

	TP_ARGS(freq, load, dynamic_power, static_power, req_power),

	TP_STRUCT__entry(
		__field(unsigned long, freq          )
		__field(unsigned long, load          )
		__field(unsigned long, dynamic_power )
		__field(unsigned long, static_power  )
		__field(unsigned long, req_power  )
	),

	TP_fast_assign(
		__entry->freq = freq;
		__entry->load = load;
		__entry->dynamic_power = dynamic_power;
		__entry->static_power = static_power;
		__entry->req_power = req_power;
	),

	TP_printk("gpu freq=%lu load=%lu dynamic_power=%lu static_power=%lu req_power=%lu",
		 __entry->freq, __entry->load,
		 __entry->dynamic_power, __entry->static_power, __entry->req_power)
);

TRACE_EVENT(IPA_allocator,
	TP_PROTO(unsigned long current_temp, unsigned long control_temp, unsigned long switch_temp, s32 delta_temp,
			 u32 num_actors,
			 u32 power_range,
			 u32 *req_power, u32 total_req_power,
			 u32 *max_power, u32 max_allocatable_power,
			 u32 *granted_power, u32 total_granted_power
		),
	TP_ARGS(current_temp, control_temp, switch_temp, delta_temp, num_actors,
			power_range,
			req_power, total_req_power,
			max_power, max_allocatable_power,
			granted_power, total_granted_power
		),
	TP_STRUCT__entry(
		__field(unsigned long, current_temp             )
		__field(unsigned long, control_temp             )
		__field(unsigned long, switch_temp             )
		__field(s32,           delta_temp               )
		__field(u32,           num_actors               )
		__field(u32,           power_range              )
		__dynamic_array(u32,   req_power, num_actors    )
		__field(u32,           total_req_power          )
		__dynamic_array(u32,   granted_power, num_actors)
		__field(u32,           total_granted_power      )
		__dynamic_array(u32,   max_power, num_actors    )
		__field(u32,           max_allocatable_power    )

	),
	TP_fast_assign(
		__entry->current_temp = current_temp;
		__entry->control_temp = control_temp;
		__entry->switch_temp = switch_temp;
		__entry->delta_temp = delta_temp;
		__entry->num_actors = num_actors;
		__entry->power_range = power_range;
		memcpy(__get_dynamic_array(req_power), req_power, num_actors * sizeof(*req_power));
		__entry->total_req_power = total_req_power;
		memcpy(__get_dynamic_array(granted_power), granted_power, num_actors * sizeof(*granted_power));
		__entry->total_granted_power = total_granted_power;
		memcpy(__get_dynamic_array(max_power), max_power, num_actors * sizeof(*max_power));
		__entry->max_allocatable_power = max_allocatable_power;
	),

	TP_printk("%lu,%lu,%lu,%d,%u,%s,%u,%s,%u,%s,%u",
			__entry->current_temp,__entry->control_temp,__entry->switch_temp,
			__entry->delta_temp,
			__entry->power_range,
		__print_array(__get_dynamic_array(req_power), __entry->num_actors, sizeof(u32)),
		__entry->total_req_power,
		__print_array(__get_dynamic_array(max_power), __entry->num_actors, sizeof(u32)),
		__entry->max_allocatable_power,
		__print_array(__get_dynamic_array(granted_power), __entry->num_actors, sizeof(u32)),
		__entry->total_granted_power
		)

);

TRACE_EVENT(IPA_allocator_pid,
	TP_PROTO(s32 err, s32 err_integral, s64 p, s64 i, s64 d, s32 output),
	TP_ARGS(err, err_integral, p, i, d, output),
	TP_STRUCT__entry(
		__field(s32, err         )
		__field(s32, err_integral)
		__field(s64, p           )
		__field(s64, i           )
		__field(s64, d           )
		__field(s32, output      )
	),
	TP_fast_assign(
		__entry->err = err;
		__entry->err_integral = err_integral;
		__entry->p = p;
		__entry->i = i;
		__entry->d = d;
		__entry->output = output;
	),

	TP_printk("%d,%d,%lld,%lld,%lld,%d",
		__entry->err, __entry->err_integral,
		__entry->p, __entry->i, __entry->d, __entry->output)
);

TRACE_EVENT(IPA_actor_cpu_get_power,
	TP_PROTO(const struct cpumask *cpus, unsigned long freq, u32 *load,
		size_t load_len, u32 dynamic_power, u32 static_power, u32 req_power),

	TP_ARGS(cpus, freq, load, load_len, dynamic_power, static_power, req_power),

	TP_STRUCT__entry(
		__bitmask(cpumask, num_possible_cpus())
		__field(unsigned long, freq          )
		__dynamic_array(u32,   load, load_len)
		__field(size_t,        load_len      )
		__field(u32,           dynamic_power )
		__field(u32,           static_power  )
		__field(u32,           req_power     )
	),

	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				num_possible_cpus());
		__entry->freq = freq;
		memcpy(__get_dynamic_array(load), load, load_len * sizeof(*load));
		__entry->load_len = load_len;
		__entry->dynamic_power = dynamic_power;
		__entry->static_power = static_power;
		__entry->req_power = req_power;
	),

	TP_printk("%s,%lu,%s,%u,%u,%u",
		__get_bitmask(cpumask),
		__entry->freq,
		__print_array(__get_dynamic_array(load), __entry->load_len, sizeof(u32)),
		__entry->dynamic_power,
		__entry->static_power,
		__entry->req_power)
);

TRACE_EVENT(IPA_actor_cpu_limit,
	TP_PROTO(const struct cpumask *cpus, unsigned int freq,
		unsigned long cdev_state, u32 power),

	TP_ARGS(cpus, freq, cdev_state, power),

	TP_STRUCT__entry(
		__bitmask(cpumask, num_possible_cpus())
		__field(unsigned int,  freq      )
		__field(unsigned long, cdev_state)
		__field(u32,           power     )
	),

	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				num_possible_cpus());
		__entry->freq = freq;
		__entry->cdev_state = cdev_state;
		__entry->power = power;
	),

	TP_printk("%s,%u,%lu,%u",
		__get_bitmask(cpumask), __entry->freq, __entry->cdev_state,
		__entry->power)
);

TRACE_EVENT(IPA_actor_cpu_pdata_to_freq,
		TP_PROTO(int level, unsigned int freq_mhz,
			unsigned int voltage, unsigned long long calc_power,
			u32 power_table, unsigned long normalized_pdata),

		TP_ARGS(level, freq_mhz, voltage, calc_power, power_table,
			normalized_pdata),

		TP_STRUCT__entry(
			__field(int, level)
			__field(unsigned int, freq_mhz)
			__field(unsigned int, voltage)
			__field(unsigned long long, calc_power)
			__field(u32, power_table)
			__field(unsigned long, normalized_pdata)
			),

		TP_fast_assign(
				__entry->level = level;
				__entry->freq_mhz = freq_mhz;
				__entry->voltage = voltage;
				__entry->calc_power = calc_power;
				__entry->power_table = power_table;
				__entry->normalized_pdata = normalized_pdata;
			      ),

		TP_printk("%d,%u,%u,%llu,%u,%lu",
				__entry->level,
				__entry->freq_mhz,
				__entry->voltage,
				__entry->calc_power,
				__entry->power_table,
				__entry->normalized_pdata)
);

TRACE_EVENT(IPA_actor_gpu_pdata_to_freq,
		TP_PROTO(int level, unsigned int freq_mhz,
			unsigned int voltage, unsigned long long calc_power,
			u32 power_table, unsigned long normalized_pdata),

		TP_ARGS(level, freq_mhz, voltage, calc_power, power_table,
			normalized_pdata),

		TP_STRUCT__entry(
			__field(int, level)
			__field(unsigned int, freq_mhz)
			__field(unsigned int, voltage)
			__field(unsigned long long, calc_power)
			__field(u32, power_table)
			__field(unsigned long, normalized_pdata)
			),

		TP_fast_assign(
				__entry->level = level;
				__entry->freq_mhz = freq_mhz;
				__entry->voltage = voltage;
				__entry->calc_power = calc_power;
				__entry->power_table = power_table;
				__entry->normalized_pdata = normalized_pdata;
			      ),

		TP_printk("%d,%u,%u,%llu,%u,%lu",
				__entry->level,
				__entry->freq_mhz,
				__entry->voltage,
				__entry->calc_power,
				__entry->power_table,
				__entry->normalized_pdata)
);

TRACE_EVENT(IPA_actor_cpu_normalize_power,
		TP_PROTO(unsigned long normalized_pdata,
			unsigned int dynamic_power,
			unsigned int freq_mhz,
			unsigned int voltage),

		 TP_ARGS(normalized_pdata, dynamic_power, freq_mhz, voltage),

		 TP_STRUCT__entry(
			__field(unsigned long, normalized_pdata)
			__field(unsigned int, dynamic_power)
			__field(unsigned int, freq_mhz)
			__field(unsigned int, voltage)
			),

		 TP_fast_assign(
			 __entry->normalized_pdata = normalized_pdata;
			 __entry->dynamic_power = dynamic_power;
			 __entry->freq_mhz = freq_mhz;
			 __entry->voltage = voltage;
			 ),
		  TP_printk("%lu,%u,%u,%u",
				  __entry->normalized_pdata,
				  __entry->dynamic_power,
				  __entry->freq_mhz,
				  __entry->voltage)
);

TRACE_EVENT(IPA_actor_gpu_normalize_power,
		TP_PROTO(unsigned long normalized_pdata,
			unsigned int dynamic_power,
			unsigned int freq_mhz,
			unsigned int voltage),

		 TP_ARGS(normalized_pdata, dynamic_power, freq_mhz, voltage),

		 TP_STRUCT__entry(
			__field(unsigned long, normalized_pdata)
			__field(unsigned int, dynamic_power)
			__field(unsigned int, freq_mhz)
			__field(unsigned int, voltage)
			),

		 TP_fast_assign(
			 __entry->normalized_pdata = normalized_pdata;
			 __entry->dynamic_power = dynamic_power;
			 __entry->freq_mhz = freq_mhz;
			 __entry->voltage = voltage;
			 ),
		  TP_printk("%lu,%u,%u,%u",
				  __entry->normalized_pdata,
				  __entry->dynamic_power,
				  __entry->freq_mhz,
				  __entry->voltage)
);

TRACE_EVENT(IPA_actor_cpu_cooling,
	TP_PROTO(const struct cpumask *cpus, unsigned int freq,
		unsigned long cdev_state),

	TP_ARGS(cpus, freq, cdev_state),

	TP_STRUCT__entry(
		__bitmask(cpumask, num_possible_cpus())
		__field(unsigned int,  freq      )
		__field(unsigned long, cdev_state)
	),

	TP_fast_assign(
		__assign_bitmask(cpumask, cpumask_bits(cpus),
				num_possible_cpus());
		__entry->freq = freq;
		__entry->cdev_state = cdev_state;
	),

	TP_printk("%s,%u,%lu",
		__get_bitmask(cpumask), __entry->freq, __entry->cdev_state)
);

TRACE_EVENT(IPA_actor_gpu_limit,
	TP_PROTO(unsigned long freq, unsigned long cdev_state, u32 power),

	TP_ARGS(freq, cdev_state, power),

	TP_STRUCT__entry(
		__field(unsigned long,  freq      )
		__field(unsigned long, cdev_state)
		__field(u32,           power     )
	),

	TP_fast_assign(
		__entry->freq = freq;
		__entry->cdev_state = cdev_state;
		__entry->power = power;
	),

	TP_printk("%lu,%lu,%u",
		__entry->freq, __entry->cdev_state,
		__entry->power)
);

TRACE_EVENT(IPA_actor_gpu_cooling,
	TP_PROTO(unsigned long freq, unsigned long cdev_state),

	TP_ARGS(freq, cdev_state),

	TP_STRUCT__entry(
		__field(unsigned long,  freq      )
		__field(unsigned long, cdev_state)
	),

	TP_fast_assign(
		__entry->freq = freq;
		__entry->cdev_state = cdev_state;
	),

	TP_printk("%lu,%lu",
		__entry->freq, __entry->cdev_state)
);

TRACE_EVENT(IPA_actor_gpu_get_power,
	TP_PROTO(unsigned long freq, unsigned long load,
		 unsigned long dynamic_power, unsigned long static_power, unsigned long req_power),

	TP_ARGS(freq, load, dynamic_power, static_power, req_power),

	TP_STRUCT__entry(
		__field(unsigned long, freq          )
		__field(unsigned long, load          )
		__field(unsigned long, dynamic_power )
		__field(unsigned long, static_power  )
		__field(unsigned long, req_power  )
	),

	TP_fast_assign(
		__entry->freq = freq;
		__entry->load = load;
		__entry->dynamic_power = dynamic_power;
		__entry->static_power = static_power;
		__entry->req_power = req_power;
	),

	TP_printk("%lu,%lu,%lu,%lu,%lu",
		 __entry->freq, __entry->load,
		 __entry->dynamic_power, __entry->static_power, __entry->req_power)
);

TRACE_EVENT(IPA_get_tsens_value,
	TP_PROTO(u32 tsens_num, int *tsens_value, int tsens_value_max),

	TP_ARGS(tsens_num, tsens_value, tsens_value_max),

	TP_STRUCT__entry(
		__field(u32, tsens_num)
		__dynamic_array(int, tsens_value, tsens_num)
		__field(int, tsens_value_max)
	),

	TP_fast_assign(
		__entry->tsens_num = tsens_num;
		memcpy(__get_dynamic_array(tsens_value), tsens_value, tsens_num * sizeof(*tsens_value)); /* unsafe_function_ignore: memcpy */
		__entry->tsens_value_max = tsens_value_max;
	),

	TP_printk("%s,%d",
		 __print_array(__get_dynamic_array(tsens_value), __entry->tsens_num,
		 sizeof(int)), __entry->tsens_value_max)
);

TRACE_EVENT(IPA_cpu_hot_plug,
	TP_PROTO(bool cpu_downed, long sensor_val, int up_threshold,
		int down_threshold),

	TP_ARGS(cpu_downed, sensor_val, up_threshold, down_threshold),

	TP_STRUCT__entry(
		__field(bool, cpu_downed)
		__field(long, sensor_val)
		__field(int, up_threshold)
		__field(int, down_threshold)
	),

	TP_fast_assign(
		__entry->cpu_downed = cpu_downed;
		__entry->sensor_val = sensor_val;
		__entry->up_threshold = up_threshold;
		__entry->down_threshold = down_threshold;
	),

	TP_printk("thermal hotplug: %u,%ld,%d,%d",
		 __entry->cpu_downed, __entry->sensor_val, __entry->up_threshold,
		 __entry->down_threshold)
);

TRACE_EVENT(IPA_gpu_hot_plug,
	TP_PROTO(bool gpu_downed, long sensor_val, int gpu_up_threshold,
		int gpu_down_threshold),

	TP_ARGS(gpu_downed, sensor_val, gpu_up_threshold, gpu_down_threshold),

	TP_STRUCT__entry(
		__field(bool, gpu_downed)
		__field(long, sensor_val)
		__field(int, gpu_up_threshold)
		__field(int, gpu_down_threshold)
	),

	TP_fast_assign(
		__entry->gpu_downed = gpu_downed;
		__entry->sensor_val = sensor_val;
		__entry->gpu_up_threshold = gpu_up_threshold;
		__entry->gpu_down_threshold = gpu_down_threshold;
	),

	TP_printk("thermal gpu hotplug: %u,%ld,%d,%d",
		 __entry->gpu_downed, __entry->sensor_val, __entry->gpu_up_threshold,
		 __entry->gpu_down_threshold)
);
#endif /* _IPA_THERMAL_TRACE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
