

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"

int32_t oal_pcie_device_auxclk_init_hi1103(oal_pcie_res *pst_pci_res);
int32_t oal_pcie_device_aspm_init_hi1103(oal_pcie_res *pst_pci_res);
void oal_pcie_device_aspm_ctrl_hi1103(oal_pcie_res *pst_pci_res, oal_bool_enum clear);

int32_t oal_pcie_device_phy_config_bisheng(oal_pcie_res *pst_pci_res)
{
    /* ASPM L1sub power_on_time bias */
    oal_pcie_config_l1ss_poweron_time(pcie_res_to_dev(pst_pci_res));
    return OAL_SUCC;
}

int32_t pcie_pm_chip_init_bisheng(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_init = oal_pcie_device_aspm_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_auxclk_init = oal_pcie_device_auxclk_init_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_aspm_ctrl = oal_pcie_device_aspm_ctrl_hi1103;
    pst_pci_res->chip_info.cb.pm_cb.pcie_device_phy_config = oal_pcie_device_phy_config_bisheng;
    return OAL_SUCC;
}
#endif

