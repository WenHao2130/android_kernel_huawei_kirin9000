

#include "host_tx_bisheng.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_TX_BISHENG_C

#define BISHENG_DEVICE_LARGE_TX_RING_NUM 21
#define BISHENG_DEVICE_SMALL_TX_RING_NUM 91
#define BISHENG_DEVICE_TX_RING_TOTAL_NUM (BISHENG_DEVICE_LARGE_TX_RING_NUM + BISHENG_DEVICE_SMALL_TX_RING_NUM)
uint8_t bisheng_host_get_device_tx_ring_num(void)
{
    return BISHENG_DEVICE_TX_RING_TOTAL_NUM;
}
