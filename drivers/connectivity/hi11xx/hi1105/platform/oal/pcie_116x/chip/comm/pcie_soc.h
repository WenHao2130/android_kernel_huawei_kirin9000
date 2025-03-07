

#ifndef __PCIE_SOC_H__
#define __PCIE_SOC_H__

#if (defined _PRE_PRODUCT_ID_HI1161_HOST)
#define DEV_PCIE_SDIO_SEL_REG   0x000008d8
#define PCIE_SDIO_SEL_REG_VALUE 0xbeaf
#define PCIE_INBOUND_REGIONS_MAX 32
#else
#define DEV_PCIE_SDIO_SEL_REG   0x00000204
#define PCIE_SDIO_SEL_REG_VALUE 0x7070
#define PCIE_INBOUND_REGIONS_MAX 16
#endif

#endif
