

#ifndef __OAL_LINUX_PCI_IF_H__
#define __OAL_LINUX_PCI_IF_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_util.h"
#include "oal_hardware.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_PCI_IF_H

/* 宏定义 */
#ifndef PCI_EXP_DEVCTL2_LTR_EN
#define PCI_EXP_DEVCTL2_LTR_EN 0x0400 /* Enable LTR mechanism */
#endif

#ifndef PCI_EXT_CAP_ID_L1SS
#define PCI_EXT_CAP_ID_L1SS 0x1E    /* L1 PM Substates */
#endif

#ifndef PCI_L1SS_CTL2
#define PCI_L1SS_CTL2 0x0C  /* Aspm L1sub power on time */
#endif

#ifndef PCI_GEN2_CTRL
#define PCI_GEN2_CTRL 0x80C /* Enable link training, equalization */
#endif

/* indicate shenkuo pci0&pci1
 * 0xFFC~0xFFF */
#define PCI_EXP_DEVICE2_DUAL_FLAG   0xFFC

/* 函数声明 */
extern void (*arm_pm_restart)(char str, const char *cmd);

/*
 * 函 数 名  : oal_pci_register_driver
 * 功能描述  : PCI驱动注册
 * 输入参数  : pst_drv: 指向要注册的驱动
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_register_driver(oal_pci_driver_stru *pst_drv)
{
    return pci_register_driver(pst_drv);
}

/*
 * 函 数 名  : oal_pci_register_driver
 * 功能描述  : PCI驱动去注册
 * 输入参数  : pst_drv: 指向要去注册的驱动
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE void oal_pci_unregister_driver(oal_pci_driver_stru *pst_drv)
{
    pci_unregister_driver(pst_drv);
}

/*
 * 函 数 名  : oal_pci_enable_device
 * 功能描述  : 使能PCI设备
 * 输入参数  : pst_dev: PCI设备
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_enable_device(oal_pci_dev_stru *pst_dev)
{
    return pci_enable_device(pst_dev);
}

/*
 * 函 数 名  : oal_pci_disable_device
 * 功能描述  : 禁止PCI设备
 */
OAL_STATIC OAL_INLINE void oal_pci_disable_device(oal_pci_dev_stru *pst_dev)
{
    pci_disable_device(pst_dev);
}

/*
 * 函 数 名  : oal_pci_read_config_byte
 * 功能描述  : 从配置空间读取一个字节
 * 输入参数  : pst_dev : 指向PCI设备
 *             ul_where: 配置寄存器地址
 * 输出参数  : puc_val: 读取的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_read_config_byte(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                       uint8_t *puc_val)
{
    return pci_read_config_byte(pst_dev, ul_where, puc_val);
}

/*
 * 函 数 名  : oal_pci_read_config_word
 * 功能描述  : 从配置空间读取一个字
 * 输入参数  : pst_dev : 指向PCI设备
 *             ul_where: 配置寄存器地址
 * 输出参数  : pus_val : 读取的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_read_config_word(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                       uint16_t *pus_val)
{
    return pci_read_config_word(pst_dev, ul_where, pus_val);
}

/*
 * 函 数 名  : oal_pci_read_config_dword
 * 功能描述  : 从配置空间读取双字
 * 输入参数  : pst_dev : 指向PCI设备
 *             ul_where: 配置寄存器地址
 * 输出参数  : pul_val : 读取的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_read_config_dword(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                        uint32_t *pul_val)
{
    return pci_read_config_dword(pst_dev, ul_where, pul_val);
}

/*
 * 函 数 名  : oal_pci_write_config_byte
 * 功能描述  : 向配置空间写一个字节
 * 输入参数  : pst_dev : 指向PCI设备
 *             ul_where: 配置寄存器地址
 *             uc_val  : 要写的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_write_config_byte(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                        uint8_t uc_val)
{
    return pci_write_config_byte(pst_dev, ul_where, uc_val);
}

/*
 * 函 数 名  : oal_pci_write_config_word
 * 功能描述  : 向配置空间读写一个字
 * 输入参数  : pst_dev : 指向PCI设备
 *             ul_where: 配置寄存器地址
 *             us_val  : 要写的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_write_config_word(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                        uint16_t us_val)
{
    return pci_write_config_word(pst_dev, ul_where, us_val);
}

/*
 * 函 数 名  : oal_pci_write_config_dword
 * 功能描述  : 向配置空间写一个双字
 *             ul_where: 配置寄存器地址
 *             ul_val  : 要写的值
 * 返 回 值  : Linux错误码
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_write_config_dword(oal_pci_dev_stru *pst_dev, int32_t ul_where,
                                                         uint32_t ul_val)
{
    return pci_write_config_dword(pst_dev, ul_where, ul_val);
}

/*
 * 函 数 名  : oal_pci_resource_start
 * 功能描述  : 读取PCI I/O空间首地址
 * 输入参数  : pst_dev: PCI设备
 *             l_bar  : PCI设备基地址寄存器编号，取值0~5
 */
OAL_STATIC OAL_INLINE oal_uint oal_pci_resource_start(oal_pci_dev_stru *pst_dev, int32_t l_bar)
{
    return pci_resource_start(pst_dev, l_bar);
}

/*
 * 函 数 名  : oal_pci_resource_end
 * 功能描述  : 读取PCI I/O空间最后一个可用地址
 * 输入参数  : pst_dev: PCI设备
 *             l_bar  : PCI设备基地址寄存器编号，取值0~5
 */
OAL_STATIC OAL_INLINE oal_uint oal_pci_resource_end(oal_pci_dev_stru *pst_dev, int32_t l_bar)
{
    return pci_resource_end(pst_dev, l_bar);
}

/*
 * 函 数 名  : oal_pci_resource_len
 * 功能描述  : PCI I/O空间大小
 * 输入参数  : pst_dev: PCI设备
 *             l_bar  : PCI设备基地址寄存器编号，取值0~5
 */
OAL_STATIC OAL_INLINE uint32_t oal_pci_resource_len(oal_pci_dev_stru *pst_dev, int32_t l_bar)
{
    return pci_resource_len(pst_dev, l_bar);
}

OAL_STATIC OAL_INLINE uint64_t oal_pci_bus_address(oal_pci_dev_stru *pst_dev, int32_t l_bar)
{
#ifdef CONFIG_PCI
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    return (uint64_t)pci_bus_address(pst_dev, l_bar);
#else
    struct pci_bus_region region;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
    pcibios_resource_to_bus(pst_dev, &region, &pst_dev->resource[l_bar]);
#else
    pcibios_resource_to_bus(pst_dev->bus, &region, &pst_dev->resource[l_bar]);
#endif

    return (uint64_t)region.start;
#endif
#else
    return 0;
#endif
}

OAL_STATIC OAL_INLINE unsigned long oal_pci_resource_flags(oal_pci_dev_stru *pst_dev, int32_t l_bar)
{
    return pci_resource_flags(pst_dev, l_bar);
}

/*
 * 函 数 名  : oal_pci_save_state
 * 功能描述  : PCI配置空间数据保存
 * 输入参数  : pst_dev: PCI设备
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_save_state(oal_pci_dev_stru *pst_dev)
{
    return pci_save_state(pst_dev);
}

/*
 * 函 数 名  : oal_pci_restore_state
 * 功能描述  : PCI配置空间数据恢复
 * 输入参数  : pst_dev: PCI设备
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_restore_state(oal_pci_dev_stru *pst_dev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    pci_restore_state(pst_dev);
    return 0;
#else
    return pci_restore_state(pst_dev);
#endif
}

/*
 * 函 数 名  : oal_pci_set_drvdata
 * 功能描述  : 设置PCI设备私有数据
 * 输入参数  : pst_dev: 指向设备
 *             p_data : 数据
 */
OAL_STATIC OAL_INLINE void oal_pci_set_drvdata(oal_pci_dev_stru *pst_dev, void *p_data)
{
    pci_set_drvdata(pst_dev, p_data);
}

/*
 * 函 数 名  : oal_pci_get_drvdata
 * 功能描述  : 读取PCI设备私有数据
 */
OAL_STATIC OAL_INLINE void *oal_pci_get_drvdata(oal_pci_dev_stru *pst_dev)
{
    return pci_get_drvdata(pst_dev);
}

/*
 * 函 数 名  : oal_pci_find_capability
 * 功能描述  : 根据cap_id获取pci配置空间的指定capability位置
 * 输入参数  : cap id
 * 输出参数  : 配置空间的偏移位置
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_find_capability(oal_pci_dev_stru *pst_dev, int32_t cap)
{
    return pci_find_capability(pst_dev, cap);
}

/*
 * pci_find_ext_capability - Find an extended capability
 * @dev: PCI device to query
 * @cap: capability code
 * Returns the address of the requested extended capability structure
 * within the device's PCI configuration space or 0 if the device does
 * not support it.  Possible values for @cap:
 *  %PCI_EXT_CAP_ID_ERR        Advanced Error Reporting
 *  %PCI_EXT_CAP_ID_VC        Virtual Channel
 *  %PCI_EXT_CAP_ID_DSN        Device Serial Number
 *  %PCI_EXT_CAP_ID_PWR        Power Budgeting
 *  %PCI_EXT_CAP_ID_L1SS    L1 PM Sub state
 */
OAL_STATIC OAL_INLINE int32_t oal_pci_find_ext_capability(oal_pci_dev_stru *pst_dev, int32_t cap)
{
    return pci_find_ext_capability(pst_dev, cap);
}

OAL_STATIC OAL_INLINE int32_t oal_pci_pcie_cap(oal_pci_dev_stru *pst_dev)
{
    int pos = pci_find_capability(pst_dev, PCI_CAP_ID_EXP);
    return pos;
}

#endif /* end of oal_pci_if.h */
