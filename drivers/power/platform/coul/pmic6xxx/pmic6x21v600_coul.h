/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: hi6421v600 coulometer driver headfile
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _PMIC_21V600_COUL_H_
#define _PMIC_21V600_COUL_H_

#include <pmic_interface.h>

#ifndef BIT
#define BIT(x)                              (1 << (x))
#endif

#define COUL_HARDWARE_I_OUT_GATE                   PMIC_I_OUT_GATE0_ADDR(0)
#define COUL_HARDWARE_I_IN_GATE                    PMIC_I_IN_GATE0_ADDR(0)

#define COUL_HARDWARE_VOL_OFFSET_B_ADDR            PMIC_OTP_34_R_ADDR(0)
#define COUL_HARDWARE_VOL_OFFSET_A_ADDR_0          PMIC_OTP_34_R_ADDR(0)
#define COUL_HARDWARE_VOL_OFFSET_A_ADDR_1          PMIC_OTP_35_R_ADDR(0)

/* bit uah */
#define BIT_FOR_UAH_DCXO_586                143142

#define VOL_OFFSET_A_STEP                   100
#define VOL_OFFSET_B_STEP                   1000
#define VOL_OFFSET_A_BASE                   974400
#define VOL_OFFSET_B_BASE                   (-31500)
#define VOL_OFFSET_B_VALID_MASK             0x3F
#define VOL_OFFSET_A_HIGH_VALID_MASK        0x1fc
#define VOL_OFFSET_A_LOW_VALID_MASK         0x3
#define VOL_OFFSET_A_VALID_MASK             0x3FF

#define COUL_HARDWARE_DEBUG_REG                    PMIC_CLJ_DEBUG_ADDR(0)
#define COUL_HARDWARE_DEBUG_REG2                   PMIC_CLJ_DEBUG_2_ADDR(0)

#define COUL_HARDWARE_CURRENT                      PMIC_CURRENT_0_ADDR(0)
#define COUL_HARDWARE_V_OUT                        PMIC_V_OUT_0_ADDR(0)
#define COUL_HARDWARE_CL_OUT_BASE                  PMIC_CL_OUT0_ADDR(0)
#define COUL_HARDWARE_CL_IN_BASE                   PMIC_CL_IN0_ADDR(0)
#define COUL_HARDWARE_CHG_TIMER_BASE               PMIC_CHG_TIMER0_ADDR(0)
#define COUL_HARDWARE_LOAD_TIMER_BASE              PMIC_LOAD_TIMER0_ADDR(0)
#define COUL_HARDWARE_CL_INT_BASE                  PMIC_CL_INT0_ADDR(0)
#define COUL_HARDWARE_VOL_INT_BASE                 PMIC_V_INT0_ADDR(0)
#define COUL_HARDWARE_OFFSET_CURRENT               PMIC_OFFSET_CURRENT0_ADDR(0)
#define COUL_HARDWARE_OFFSET_VOLTAGE               PMIC_OFFSET_VOLTAGE0_ADDR(0)
#define COUL_HARDWARE_OCV_VOLTAGE_BASE             PMIC_OCV_VOLTAGE0_ADDR(0)
#define COUL_HARDWARE_OCV_CURRENT_BASE             PMIC_OCV_CURRENT0_ADDR(0)
#define COUL_HARDWARE_ECO_OUT_CLIN_REG_BASE        PMIC_ECO_OUT_CLIN_0_ADDR(0)
#define COUL_HARDWARE_ECO_OUT_CLOUT_REG_BASE       PMIC_ECO_OUT_CLOUT_0_ADDR(0)

#define COUL_HARDWARE_VOL_FIFO_BASE                PMIC_V_OUT0_PRE0_ADDR(0)
#define COUL_HARDWARE_CUR_FIFO_BASE                PMIC_CURRENT0_PRE0_ADDR(0)

#define COUL_HARDWARE_COUL_ECO_MASK                PMIC_COUL_ECO_MASK_ADDR(0)

#define COUL_HARDWARE_FIFO_CLEAR                   PMIC_CLJ_CTRL_REGS2_ADDR(0) /* use bit 1 */

#define COUL_HARDWARE_OFFSET_CUR_MODIFY_BASE       PMIC_OFFSET_CURRENT_MOD_0_ADDR(0)
#define COUL_HARDWARE_OFFSET_VOL_MODIFY_BASE       PMIC_OFFSET_VOLTAGE_MOD_0_ADDR(0)

/* coul reserverd regs use */
#define COUL_HARDWARE_BATTERY_MOVE_ADDR            PMIC_HRST_REG0_ADDR(0)
#define BATTERY_MOVE_MAGIC_NUM              0xc3
#define BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM  0x18

#define COUL_HARDWARE_OCV_CHOOSE                   PMIC_HRST_REG1_ADDR(0) /* use bit 5 */
#define COUL_HARDWARE_TEMP_PROTECT            PMIC_HRST_REG1_ADDR(0) /* use bit 4 */
#define COUL_HARDWARE_DELTA_RC_SCENE               PMIC_HRST_REG1_ADDR(0) /* use bit 3 */
#define COUL_HARDWARE_PD_BY_OCV_WRONG              PMIC_HRST_REG1_ADDR(0) /* use bit 2 */
#define COUL_HARDWARE_NV_READ_SUCCESS              PMIC_HRST_REG1_ADDR(0) /* use bit 1 */
#define COUL_HARDWARE_NV_SAVE_SUCCESS              PMIC_HRST_REG1_ADDR(0) /* use bit 0 */
#define USE_SAVED_OCV_FLAG                  BIT(5)
#define TEMP_PROTECT_BITMASK                BIT(4)
#define DELTA_RC_SCENE_BITMASK              BIT(3)
#define PD_BY_OCV_WRONG_BIT                 BIT(2)
#define NV_READ_BITMASK                     BIT(1)
#define NV_SAVE_BITMASK                     BIT(0)

#define COUL_HARDWARE_SAVE_OCV_ADDR                PMIC_HRST_REG2_ADDR(0) /* use 2byte,reserved3 and reserved4 */
#define COUL_HARDWARE_SAVE_OCV_RESERVED            PMIC_HRST_REG3_ADDR(0)
#define INVALID_TO_UPDATE_FCC               0x8000

#define COUL_HARDWARE_SAVE_OCV_TEMP_ADDR           PMIC_HRST_REG4_ADDR(0) /* OCV TEMP saved use 2bytes */
#define COUL_HARDWARE_SAVE_OCV_TEMP_RESERVED       PMIC_HRST_REG5_ADDR(0)

/* record last soc */
#define COUL_HARDWARE_SAVE_LAST_SOC                PMIC_HRST_REG6_ADDR(0) /* last soc 0-6bit */
#define COUL_HARDWARE_SAVE_LAST_SOC_VAILD          PMIC_HRST_REG6_ADDR(0) /* last soc vaild 7bit */
#define SAVE_LAST_SOC (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SAVE_LAST_SOC_FLAG                  BIT(7)
#define CLEAR_LAST_SOC_FLAG                 0x7F

#define COUL_HARDWARE_OCV_LEVEL_ADDR               PMIC_HRST_REG7_ADDR(0) /* last soc 2-5bit */
#define SAVE_OCV_LEVEL                      (BIT(5) | BIT(4) | BIT(3) | BIT(2))
#define OCV_LEVEL_SHIFT                     2

#define DRAINED_BATTERY_FLAG_ADDR           PMIC_HRST_REG12_ADDR(0)
#define DRAINED_BATTERY_FLAG_BIT            BIT(0)
#define BOOT_OCV_ADDR                       PMIC_HRST_REG12_ADDR(0) /* bit 1 */
#define EN_BOOT_OCV_SAMPLE                  BIT(1)

/* coul register of smartstar */
#define COUL_HARDWARE_CTRL_REG                PMIC_CLJ_CTRL_REG_ADDR(0)
#define COUL_CALI_ENABLE                    BIT(7)

#define REG_NUM                             2
#define CC_REG_NUM                          4

#define COUL_ECO_FLT_50MS                   0
#define COUL_ECO_FLT_100MS                  BIT(4)
#define COUL_ECO_FLT_150MS                  BIT(5)
#define COUL_ECO_FLT_200MS                  (BIT(4) | BIT(5))
#define COUL_ECO_FLT_250MS                  BIT(6)
#define COUL_ECO_FLT_300MS                  (BIT(4) | BIT(6))
#define COUL_ECO_FLT_350MS                  (BIT(5) | BIT(6))
#define COUL_ECO_FLT_400MS                  (BIT(4) | BIT(5) | BIT(6))

#define COUL_ALL_REFLASH                    0
#define COUL_ECO_REFLASH                    BIT(3)
#define COUL_ECO_ENABLE                     BIT(2)
#define COUL_ECO_PMU_EN                     (BIT(0) | BIT(1))
#define COUL_ECO_DISABLE                    0
#define COUL_FIFO_CLEAR                     BIT(1)
#define DEFAULT_COUL_CTRL_VAL (COUL_ECO_FLT_100MS | COUL_ALL_REFLASH | \
	COUL_ECO_DISABLE)
#define ECO_COUL_CTRL_VAL     (COUL_ECO_FLT_200MS | COUL_ECO_REFLASH | \
	COUL_ECO_PMU_EN)

#define CALI_AUTO_ONOFF_CTRL                BIT(5)
#define CALI_AUTO_TIME_15S                  0
#define CALI_AUTO_TIME_30S                  BIT(2)
#define CALI_AUTO_TIME_60S                  BIT(3)
#define CALI_AUTO_TIME_2MIN                 (BIT(2) | BIT(3))
#define CALI_AUTO_TIME_4MIN                 BIT(4)
#define CALI_AUTO_TIME_8MIN                 (BIT(2) | BIT(4))
#define CALI_AUTO_TIME_16MIN                (BIT(4) | BIT(3))
#define CALI_AUTO_TIME_32MIN                (BIT(2) | BIT(3) | BIT(4))

#define COUL_HARDWARE_CLJ_CTRL                     PMIC_CLJ_CTRL_REGS2_ADDR(0)
/* Warning: bit change */
#define COUL_CTRL_ONOFF_REG                 BIT(0)
#define CALI_CLJ_DEFAULT_VALUE (CALI_AUTO_TIME_4MIN | CALI_AUTO_ONOFF_CTRL | \
	COUL_CTRL_ONOFF_REG)

#define MASK_CALI_AUTO_OFF                  (CALI_AUTO_ONOFF_CTRL)

#define COUL_CLK_MODE_ADDR                  PMIC_STATUS1_ADDR(0)
#define XO32K_MODE_MSK                      BIT(2) /* tells 32k or others */
#define COUL_32K_CLK_MODE                   BIT(2)
#define NO_32K_MODE                         0

#define COUL_HARDWARE_IRQ_REG                 PMIC_COUL_IRQ_ADDR(0)
#define COUL_HARDWARE_IRQ_MASK_REG            PMIC_COUL_IRQ_MASK_ADDR(0)
#define COUL_CLEAR_IRQ                      0x0F


#define COUL_I_OUT_MASK                     BIT(5)
#define COUL_I_IN_MASK                      BIT(4)
#define COUL_VBAT_INT_MASK                  BIT(3)
#define COUL_CL_IN_MASK                     BIT(2)
#define COUL_CL_OUT_MASK                    BIT(1)
#define COUL_CL_INT_MASK                    BIT(0)
#define COUL_INT_MASK_ALL (COUL_VBAT_INT_MASK | COUL_CL_IN_MASK | \
	COUL_CL_OUT_MASK | COUL_CL_INT_MASK)

#define COUL_HARDWARE_VERSION_ADDR            PMIC_VERSION0_ADDR(0)

#define COUL_HARDWARE_STATE_REG               PMIC_STATE_TEST_ADDR(0) /* Warning: bit change */

#define COUL_CALI_ING                       (BIT(0) | BIT(1))

#define FIFO_DEPTH                          10

/* register write lock/unlock */
#define COUL_HARDWARE_DEBUG_WRITE_PRO              PMIC_DEBUG_WRITE_PRO_ADDR(0)
#define COUL_WRITE_LOCK                     0x56
#define COUL_WRITE_UNLOCK                   0xA9

#endif
