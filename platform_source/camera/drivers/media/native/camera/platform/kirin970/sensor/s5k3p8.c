/*
 *  SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Date:      2017-06-26
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


#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define POWER_DELAY_0        0//delay 0 ms
#define POWER_DELAY_1        1//delay 1 ms

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64
//lint -save -e826 -e838 -e715 -e747 -e778 -e774 -e732
//lint -save -e650 -e31 -e731 -e528 -e753 -e737

static bool s_s5k3p8_power_on = false;

struct mutex s5k3p8_power_lock;

char const* s5k3p8_get_name(hwsensor_intf_t* si);
int s5k3p8_config(hwsensor_intf_t* si, void  *argp);
int s5k3p8_power_up(hwsensor_intf_t* si);
int s5k3p8_power_down(hwsensor_intf_t* si);
int s5k3p8_match_id(hwsensor_intf_t* si, void * data);
int s5k3p8_csi_enable(hwsensor_intf_t* si);
int s5k3p8_csi_disable(hwsensor_intf_t* si);

static hwsensor_vtbl_t s_s5k3p8_vtbl =
{
    .get_name = s5k3p8_get_name,
    .config = s5k3p8_config,
    .power_up = s5k3p8_power_up,
    .power_down = s5k3p8_power_down,
    .match_id = s5k3p8_match_id,
    .csi_enable = s5k3p8_csi_enable,
    .csi_disable = s5k3p8_csi_disable,
};

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting hw_s5k3p8_power_up_setting[] = {
    //disable MCAM1 reset [GPIO136]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_0,
    },

    //disable MCAM1 PWDN [GPIO227]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_0,
    },

    //SCAM IOVDD 1.80V [LDO21]
    {
        .seq_type     = SENSOR_IOVDD,
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM AVDD 2.80V [LDO13]
    {
        .seq_type     = SENSOR_AVDD,
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_0,
    },

    //SCAM DVDD 1.2V [LDO32]
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM CLK
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //SCAM RESET [GPIO32]
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,//pull up
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
};

atomic_t volatile s5k3p8_powered = ATOMIC_INIT(0);
sensor_t s_s5k3p8 =
{
    .intf = { .vtbl = &s_s5k3p8_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hw_s5k3p8_power_up_setting),
            .power_setting = hw_s5k3p8_power_up_setting,
     },
     .p_atpowercnt = &s5k3p8_powered,
};

const struct of_device_id s_s5k3p8_dt_match[] =
{
    {
        .compatible = "huawei,s5k3p8",
        .data = &s_s5k3p8.intf,
    },

    {
    },
};

MODULE_DEVICE_TABLE(of, s_s5k3p8_dt_match);

struct platform_driver
s_s5k3p8_driver =
{
    .driver =
    {
        .name = "huawei,s5k3p8",
        .owner = THIS_MODULE,
        .of_match_table = s_s5k3p8_dt_match,
    },
};

char const* s5k3p8_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int s5k3p8_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
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

int s5k3p8_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
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

int s5k3p8_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int s5k3p8_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

int s5k3p8_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;

    cam_info("%s enter.", __func__);

    if ((NULL == si) || (NULL == data)) {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -EINVAL;
    }
    cdata  = (struct sensor_cfg_data *)data;
    cdata->data = sensor->board_info->sensor_index;
    cam_info("%s name:%s", __func__, sensor->board_info->name);

    return 0;
}

int s5k3p8_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if (NULL == si || NULL == argp || NULL == si->vtbl) {
        cam_err("%s : si or argp or si->vtbl is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("s5k3p8 cfgtype = %d",data->cfgtype);
    switch (data->cfgtype) {
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&s5k3p8_power_lock);
            if (NULL == si->vtbl->power_up)
            {
                cam_err("%s. si power_up is null", __func__);
                /*lint -e455 -esym(455,*)*/
                mutex_unlock(&s5k3p8_power_lock);
                /*lint -e455 +esym(455,*)*/
                return -EINVAL;
            }
            if (!s_s5k3p8_power_on){
                ret = si->vtbl->power_up(si);
                if (0 == ret) {
                    s_s5k3p8_power_on = true;
                } else {
                    cam_err("%s. power up fail.", __func__);
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&s5k3p8_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&s5k3p8_power_lock);
            if (NULL == si->vtbl->power_down)
            {
                cam_err("%s. si power_up is null", __func__);
                /*lint -e455 -esym(455,*)*/
                mutex_unlock(&s5k3p8_power_lock);
                /*lint -e455 +esym(455,*)*/
                return -EINVAL;
            }
            if (s_s5k3p8_power_on)
            {
                ret = si->vtbl->power_down(si);
                if (0 == ret)
                {
                    s_s5k3p8_power_on = false;
                }else{
                    cam_err("%s. power down fail.", __func__);
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&s5k3p8_power_lock);
            /*lint -e455 +esym(455,*)*/
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
            if(NULL == si->vtbl->match_id){
                cam_err("%s. si->vtbl->match_id is null.", __func__);
                ret=-EINVAL;
            }else{
                ret = si->vtbl->match_id(si,argp);
            }
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }
    return ret;
}

int32_t s5k3p8_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_notice("enter %s",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&s5k3p8_power_lock);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_s5k3p8_dt_match, np);
    if (NULL == id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    if (NULL == intf) {
        cam_err("%s intf is NULL", __func__);
        return -ENODEV;
    }

    sensor = I2S(intf);
    if(NULL == sensor){
        cam_err("%s sensor is NULL rc %d", __func__, rc);
        return -ENODEV;
    }
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data rc %d", __func__, rc);
        return -ENODEV;
    }

    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_pdev = pdev;

    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

int __init
s5k3p8_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_s5k3p8_driver,
            s5k3p8_platform_probe);
}

void __exit
s5k3p8_exit_module(void)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_pdev) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
    }
    platform_driver_unregister(&s_s5k3p8_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(s5k3p8_init_module);
module_exit(s5k3p8_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("s5k3p8");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
