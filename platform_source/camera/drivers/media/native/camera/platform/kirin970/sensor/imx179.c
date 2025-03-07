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

static char const* imx179_get_name(hwsensor_intf_t* si);
static int imx179_config(hwsensor_intf_t* si, void  *argp);
static int imx179_power_up(hwsensor_intf_t* si);
static int imx179_power_down(hwsensor_intf_t* si);
static int imx179_match_id(hwsensor_intf_t* si, void * data);
static int imx179_csi_enable(hwsensor_intf_t* si);
static int imx179_csi_disable(hwsensor_intf_t* si);

static struct sensor_power_setting imx179_power_setting[] = {
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
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

    //MCAM1 OIS LDO25 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM AVDD LDO19 2.8V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD LDO20 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .data = (void*)"main-sensor-dvdd",
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //enable gpio22 output vcmvdd 2.85v
    {
        .seq_type = SENSOR_VCM_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD LDO13 2.85V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD LDO32 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx179_ml_power_setting[] = {
    //SENSOR IO
    {
        .seq_type = SENSOR_PMIC,
        .seq_val  = VOUT_LDO_1,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 AVDD 2.8V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"front-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_V1P25V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //VCM [2.85v]
    {
        .seq_type = SENSOR_VCM_AVDD,
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

static struct sensor_power_setting imx179_fpga_power_setting[] = {
	//SENSOR IO
	{
		.seq_type = SENSOR_PMIC,
		.seq_val  = VOUT_LDO_1,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//MCAM1 AVDD 2.8V
	{
		.seq_type = SENSOR_AVDD,
		.data = (void*)"front-sensor-avdd",
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//MCAM1 DVDD 1.2V
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_V1P25V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	//VCM [2.85v]
	{
		.seq_type = SENSOR_VCM_AVDD,
		.config_val = LDO_VOLTAGE_V2P85V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

static hwsensor_vtbl_t
s_imx179_vtbl =
{
    .get_name = imx179_get_name,
    .config = imx179_config,
    .power_up = imx179_power_up,
    .power_down = imx179_power_down,
    .match_id = imx179_match_id,
    .csi_enable = imx179_csi_enable,
    .csi_disable = imx179_csi_disable,
};

static sensor_t s_imx179 =
{
    .intf = { .vtbl = &s_imx179_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(imx179_power_setting),
            .power_setting = imx179_power_setting,
     },
};

static sensor_t s_imx179_ml =
{
    .intf = { .vtbl = &s_imx179_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx179_ml_power_setting),
        .power_setting = imx179_ml_power_setting,
    },
};

static sensor_t s_imx179_fpga =
{
    .intf = { .vtbl = &s_imx179_vtbl, },
    .power_setting_array = {
        .size = ARRAY_SIZE(imx179_fpga_power_setting),
        .power_setting = imx179_fpga_power_setting,
    },
};

static const struct of_device_id
s_imx179_dt_match[] =
{
    {
        .compatible = "huawei,imx179",
        .data = &s_imx179.intf,
    },
    {
        .compatible = "huawei,imx179_ml",
        .data = &s_imx179_ml.intf,
    },
    {
        .compatible = "huawei,imx179_fpga",
        .data = &s_imx179_fpga.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_imx179_dt_match);

static struct platform_driver
s_imx179_driver =
{
    .driver =
    {
        .name = "huawei,imx179",
        .owner = THIS_MODULE,
        .of_match_table = s_imx179_dt_match,
    },
};

static char const*
imx179_get_name(
        hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

static int
imx179_power_up(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()){
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
imx179_power_down(
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

static int imx179_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx179_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int
imx179_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = I2S(si);
    struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;

    char * sensor_name[] = {"IMX179_4L_FOXCONN"};

    cam_info("%s enter.", __func__);

    cdata->data = SENSOR_INDEX_INVALID; /*lint !e569 */
    memset(cdata->cfg.name, 0, DEVICE_NAME_SIZE);

    if (!strncmp(sensor->board_info->name, "IMX179_VENDOR", strlen("IMX179_VENDOR"))) {
    // Fixme
        strncpy(cdata->cfg.name, sensor_name[0], DEVICE_NAME_SIZE-1);
        cdata->data = sensor->board_info->sensor_index;
    } else {
        strncpy(cdata->cfg.name, sensor->board_info->name, DEVICE_NAME_SIZE-1);
        cdata->data = sensor->board_info->sensor_index;
    }
    cam_info("%s TODO. cdata->data=%d,cdata->cfg.name = %s", __func__, cdata->data,cdata->cfg.name);
    return 0;
}

static int
imx179_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;
    static bool imx179_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx179 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            if (!imx179_power_on) {
                ret = si->vtbl->power_up(si);
                imx179_power_on = true;
            }
            break;
        case SEN_CONFIG_POWER_OFF:
            if (imx179_power_on) {
                ret = si->vtbl->power_down(si);
                imx179_power_on = false;
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

static int32_t
imx179_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    struct device_node *np = pdev->dev.of_node;
    const struct of_device_id *id;
    hwsensor_intf_t *intf;
    sensor_t *sensor;

    cam_notice("enter %s",__func__);
    if (!np)
    {
        cam_err("%s imx179 of_node is NULL.\n", __func__);
        goto imx179_sensor_probe_fail;
    }

    id = of_match_node(s_imx179_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    sensor = I2S(intf);
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto imx179_sensor_probe_fail;
    }
    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto imx179_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        cam_err("%s failed line %d\n", __func__, __LINE__);
        goto imx179_sensor_probe_fail;
    }

imx179_sensor_probe_fail:
    return rc;
}

static int __init
imx179_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_imx179_driver,
            imx179_platform_probe);
}

static void __exit
imx179_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_imx179);
    hwsensor_unregister(Sensor2Pdev(s_imx179));
    platform_driver_unregister(&s_imx179_driver);
}

module_init(imx179_init_module);
module_exit(imx179_exit_module);
MODULE_DESCRIPTION("imx179");
MODULE_LICENSE("GPL v2");

