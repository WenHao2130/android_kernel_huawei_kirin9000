/*
 *  SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Date:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"

#include "../pmic/hw_pmic.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)

static struct sensor_power_setting imx286hybird_power_setting[] = {
    //disable front camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0 AVDD0  2.85V  [CAM_PMIC_LDO4]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_4,
        .config_val = PMIC_2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  2.85V  [CAM_PMIC_LDO2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_2,
        .config_val = PMIC_2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  1.80V  [CAM_PMIC_LDO5]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_5,
        .config_val = PMIC_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM0 DVDD 1.1V [LDO]
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.05V [LDO]
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M0  VCM  2.8V  [PMIC BUCK1]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_1,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //M1  VCM  2.8V  [PMIC BUCK2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_2,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM OISVDD 2.80V
	{
		.seq_type = SENSOR_OIS_DRV,
		.config_val = 2800000,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },

    // M0 RESET  [GPIO_051]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    // M1 RESET  [GPIO_013]
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx286hybird_fpga_power_setting[] = {
    //enable gpio51 output iovdd 1.8v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_2,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD0  2.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_3,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M1 AVDD1  1.80V  [CAM_PMIC_LDO3]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_LDO_5,
        .config_val = PMIC_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    // DVDD BUCK_1 1.15v
    {
        .seq_type = SENSOR_PMIC,
        .seq_val = VOUT_BUCK_1,
        .config_val = PMIC_1P15V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //M0  VCM  3V  [PMIC BUCK2]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_BUCK_2,
        .config_val = PMIC_3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //DRVVDD 2.85V [PMIC_LDO4]
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_LDO_4,
        .config_val = PMIC_2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //enable gpio51 output iovdd 1.8v
    {
        .seq_type = SENSOR_LDO_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 2,
        .delay = 1,
    },

    // M0 RESET  [GPIO_052]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
static char const*
imx286hybird_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
imx286hybird_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

static int
imx286hybird_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    return ret;
}

static int imx286hybird_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx286hybird_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
imx286hybird_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    strncpy(cdata->cfg.name, sensor->board_info->name, DEVICE_NAME_SIZE-1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int
imx286hybird_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;
    static bool s_imx286hybird_power_on = false;
    int ret =0;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx286hybird cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            if (!s_imx286hybird_power_on){
                ret = si->vtbl->power_up(si);
                s_imx286hybird_power_on = true;
            }else{
                cam_err("%s camera has powered on",__func__);
            }
            break;
        case SEN_CONFIG_POWER_OFF:
            if (s_imx286hybird_power_on){
                ret = si->vtbl->power_down(si);
                s_imx286hybird_power_on = false;
            }else{
                cam_err("%s camera has powered off",__func__);
            }
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            break;
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = si->vtbl->match_id(si,argp);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

static hwsensor_vtbl_t s_imx286hybird_vtbl =
{
    .get_name = imx286hybird_get_name,
    .config = imx286hybird_config,
    .power_up = imx286hybird_power_up,
    .power_down = imx286hybird_power_down,
    .match_id = imx286hybird_match_id,
    .csi_enable = imx286hybird_csi_enable,
    .csi_disable = imx286hybird_csi_disable,
};
/* individual driver data for each device */

static sensor_t s_imx286hybird =
{
    .intf = { .vtbl = &s_imx286hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx286hybird_power_setting),
        .power_setting = imx286hybird_power_setting,
    },
};

static sensor_t s_imx286hybird_fpga =
{
    .intf = { .vtbl = &s_imx286hybird_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx286hybird_fpga_power_setting),
        .power_setting = imx286hybird_fpga_power_setting,
    },
};

/* support both imx286hybird & imx286legacydual */
static const struct of_device_id
s_imx286hybird_dt_match[] =
{
    {
        .compatible = "huawei,imx286hybird",
        .data = &s_imx286hybird.intf,
    },
    {
        .compatible = "huawei,imx286hybird_fpga",
        .data = &s_imx286hybird_fpga.intf,
    },
    { } /* terminate list */
};

MODULE_DEVICE_TABLE(of, s_imx286hybird_dt_match);
/* platform driver struct */
static int32_t imx286hybird_platform_probe(struct platform_device* pdev);
static int32_t imx286hybird_platform_remove(struct platform_device* pdev);
static struct platform_driver
s_imx286hybird_driver =
{
    .probe = imx286hybird_platform_probe,
    .remove = imx286hybird_platform_remove,
    .driver =
    {
        .name = "huawei,imx286hybird",
        .owner = THIS_MODULE,
        .of_match_table = s_imx286hybird_dt_match,
    },
};


static int32_t
imx286hybird_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s gal",__func__);

    if (!np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx286hybird_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data", __func__);
        return -ENODEV;
    }
    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    rc = rpmsg_sensor_register(pdev, (void*)sensor);

    return rc;
}

static int32_t
imx286hybird_platform_remove(
    struct platform_device * pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_info("enter %s",__func__);

    if (!np) {
        cam_info("%s of_node is NULL", __func__);
        return 0;
    }
    /* don't use dev->p->driver_data
     * we need to search again */
    id = of_match_node(s_imx286hybird_dt_match, np);
    if (!id) {
        cam_info("%s none id matched", __func__);
        return 0;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);

    rpmsg_sensor_unregister((void*)&sensor);
    hwsensor_unregister(pdev);
    return 0;
}
static int __init
imx286hybird_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_imx286hybird_driver,
            imx286hybird_platform_probe);
}

static void __exit
imx286hybird_exit_module(void)
{
    platform_driver_unregister(&s_imx286hybird_driver);
}

module_init(imx286hybird_init_module);
module_exit(imx286hybird_exit_module);
MODULE_DESCRIPTION("imx286hybird");
MODULE_LICENSE("GPL v2");
