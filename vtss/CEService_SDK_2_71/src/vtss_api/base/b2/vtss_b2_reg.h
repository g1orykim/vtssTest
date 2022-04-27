/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/

/* This file is auto-generated using vml/reglist_generator/barrington2/reglist.pl. */

#ifndef _VTSS_H_BARRINGTON2_
#define _VTSS_H_BARRINGTON2_

/* Target IDs */
#define VTSS_TGT_DEVCPU_ORG    0x00
#define VTSS_TGT_DEVCPU_GCB    0x01
#define VTSS_TGT_ASM           0x02
#define VTSS_TGT_DSM           0x03
#define VTSS_TGT_ANA_CL        0x04
#define VTSS_TGT_ANA_AC        0x05
#define VTSS_TGT_SCH           0x06
#define VTSS_TGT_QSS           0x07
#define VTSS_TGT_DEVSPI        0x0c
#define VTSS_TGT_DEV1G         0x20
#define VTSS_TGT_DEV10G        0x38
#define VTSS_TGT_XAUI_PHY_STAT 0x39
#define VTSS_TGT_FAST_REGS     0x3f

/*********************************************************************** 
 * Target DEVCPU_ORG
 * Origin In Cpu Device
 ***********************************************************************/
#define VTSS_ADDR_DEVCPU_ORG_ERR_ACCESS_DROP_1                            0x0000
#define VTSS_OFF_DEVCPU_ORG_ERR_ACCESS_DROP_1_SIM_REPLY_STICKY                 9
#define VTSS_LEN_DEVCPU_ORG_ERR_ACCESS_DROP_1_SIM_REPLY_STICKY                 1
#define VTSS_OFF_DEVCPU_ORG_ERR_ACCESS_DROP_1_UTM_STICKY                       8
#define VTSS_LEN_DEVCPU_ORG_ERR_ACCESS_DROP_1_UTM_STICKY                       1
#define VTSS_OFF_DEVCPU_ORG_ERR_ACCESS_DROP_1_TGT_MODULE_UTM_STICKY            0
#define VTSS_LEN_DEVCPU_ORG_ERR_ACCESS_DROP_1_TGT_MODULE_UTM_STICKY            8

#define VTSS_ADDR_DEVCPU_ORG_ERR_ACCESS_DROP_2                            0x0001
#define VTSS_OFF_DEVCPU_ORG_ERR_ACCESS_DROP_2_NO_ACTION_STICKY                 9
#define VTSS_LEN_DEVCPU_ORG_ERR_ACCESS_DROP_2_NO_ACTION_STICKY                 1
#define VTSS_OFF_DEVCPU_ORG_ERR_ACCESS_DROP_2_TGT_MODULE_NO_ACTION_STICKY      0
#define VTSS_LEN_DEVCPU_ORG_ERR_ACCESS_DROP_2_TGT_MODULE_NO_ACTION_STICKY      8

#define VTSS_ADDR_DEVCPU_ORG_ACCESS_TABLE                                 0x0002
#define VTSS_OFF_DEVCPU_ORG_ACCESS_TABLE_TGT                                   0
#define VTSS_LEN_DEVCPU_ORG_ACCESS_TABLE_TGT                                   8

#define VTSS_ADDR_DEVCPU_ORG_ERR_TGT_MSB                                  0x0003
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_MSB_ORG_RST                                9
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_MSB_ORG_RST                                1
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_MSB_BSY_STICKY                             8
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_MSB_BSY_STICKY                             1
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_MSB_TGT_MODULE_BSY                         0
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_MSB_TGT_MODULE_BSY                         8

#define VTSS_ADDR_DEVCPU_ORG_ERR_TGT_LSB                                  0x0004
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_LSB_WD_STICKY                              9
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_LSB_WD_STICKY                              1
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_LSB_TGT_MODULE_WD_DROP                     0
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_LSB_TGT_MODULE_WD_DROP                     8

#define VTSS_ADDR_DEVCPU_ORG_ERR_TGT_INT_MASK                             0x0005
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_INT_MASK_BSY_STICKY_INT_MASK               1
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_INT_MASK_BSY_STICKY_INT_MASK               1
#define VTSS_OFF_DEVCPU_ORG_ERR_TGT_INT_MASK_WD_STICKY_INT_MASK                0
#define VTSS_LEN_DEVCPU_ORG_ERR_TGT_INT_MASK_WD_STICKY_INT_MASK                1

#define VTSS_ADDR_DEVCPU_ORG_ERR_CNTS_1                                   0x0006
#define VTSS_OFF_DEVCPU_ORG_ERR_CNTS_1_BUSY_CNT                                8
#define VTSS_LEN_DEVCPU_ORG_ERR_CNTS_1_BUSY_CNT                                8
#define VTSS_OFF_DEVCPU_ORG_ERR_CNTS_1_WD_DROP_CNT                             0
#define VTSS_LEN_DEVCPU_ORG_ERR_CNTS_1_WD_DROP_CNT                             8

#define VTSS_ADDR_DEVCPU_ORG_ERR_CNTS_2                                   0x0007
#define VTSS_OFF_DEVCPU_ORG_ERR_CNTS_2_NO_ACTION_CNT                           8
#define VTSS_LEN_DEVCPU_ORG_ERR_CNTS_2_NO_ACTION_CNT                           8
#define VTSS_OFF_DEVCPU_ORG_ERR_CNTS_2_UTM_CNT                                 0
#define VTSS_LEN_DEVCPU_ORG_ERR_CNTS_2_UTM_CNT                                 8

#define VTSS_ADDR_DEVCPU_ORG_LAST_REPLY_0                                 0x0008
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_0_LAST_STATUS                           2
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_0_LAST_STATUS                           3
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_0_LAST_RD                               1
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_0_LAST_RD                               1
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_0_LAST_REG_GRP_VLD                      0
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_0_LAST_REG_GRP_VLD                      1

#define VTSS_ADDR_DEVCPU_ORG_LAST_REPLY_1_MSB                             0x0009
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_1_MSB_LAST_TGT_ID                       8
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_1_MSB_LAST_TGT_ID                       8
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_1_MSB_LAST_TGT_ADDR_MSB                 0
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_1_MSB_LAST_TGT_ADDR_MSB                 2

#define VTSS_ADDR_DEVCPU_ORG_LAST_REPLY_1_LSB                             0x000a
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_1_LSB_LAST_TGT_ADDR_LSB                 0
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_1_LSB_LAST_TGT_ADDR_LSB                16

#define VTSS_ADDR_DEVCPU_ORG_LAST_REPLY_2_MSB                             0x000b
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_2_MSB_LAST_TGT_DATA_MSB                 0
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_2_MSB_LAST_TGT_DATA_MSB                16

#define VTSS_ADDR_DEVCPU_ORG_LAST_REPLY_2_LSB                             0x000c
#define VTSS_OFF_DEVCPU_ORG_LAST_REPLY_2_LSB_LAST_TGT_DATA_LSB                 0
#define VTSS_LEN_DEVCPU_ORG_LAST_REPLY_2_LSB_LAST_TGT_DATA_LSB                16

#define VTSS_ADDR_DEVCPU_ORG_CFG_STATUS                                   0x000d
#define VTSS_OFF_DEVCPU_ORG_CFG_STATUS_RD_ERR_STICKY_INT_MASK                  8
#define VTSS_LEN_DEVCPU_ORG_CFG_STATUS_RD_ERR_STICKY_INT_MASK                  1
#define VTSS_OFF_DEVCPU_ORG_CFG_STATUS_BUSY                                    2
#define VTSS_LEN_DEVCPU_ORG_CFG_STATUS_BUSY                                    1
#define VTSS_OFF_DEVCPU_ORG_CFG_STATUS_RD_ERR_STICKY                           1
#define VTSS_LEN_DEVCPU_ORG_CFG_STATUS_RD_ERR_STICKY                           1
#define VTSS_OFF_DEVCPU_ORG_CFG_STATUS_REQUEST_STATUS                          0
#define VTSS_LEN_DEVCPU_ORG_CFG_STATUS_REQUEST_STATUS                          1

/*********************************************************************** 
 * Target DEVCPU_GCB
 * General Configuration Block.
 ***********************************************************************/
#define VTSS_ADDR_DEVCPU_GCB_GENERAL_PURPOSE_0                            0x0000

#define VTSS_ADDR_DEVCPU_GCB_GENERAL_PURPOSE_1                            0x0001

#define VTSS_ADDR_DEVCPU_GCB_GENERAL_PURPOSE_2                            0x0002

#define VTSS_ADDR_DEVCPU_GCB_GENERAL_PURPOSE_3                            0x0003

#define VTSS_ADDR_DEVCPU_GCB_CHIP_ID                                      0x0004
#define VTSS_OFF_DEVCPU_GCB_CHIP_ID_REV_ID                                    28
#define VTSS_LEN_DEVCPU_GCB_CHIP_ID_REV_ID                                     4
#define VTSS_OFF_DEVCPU_GCB_CHIP_ID_PART_ID                                   12
#define VTSS_LEN_DEVCPU_GCB_CHIP_ID_PART_ID                                   16
#define VTSS_OFF_DEVCPU_GCB_CHIP_ID_MFG_ID                                     1
#define VTSS_LEN_DEVCPU_GCB_CHIP_ID_MFG_ID                                    11
#define VTSS_OFF_DEVCPU_GCB_CHIP_ID_ONE                                        0
#define VTSS_LEN_DEVCPU_GCB_CHIP_ID_ONE                                        1

#define VTSS_ADDR_DEVCPU_GCB_CHIP_MODE                                    0x0005
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_ID_SEL                                  28
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_ID_SEL                                   4
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_STATUS_CHANNEL_SEL                 24
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_XAUI_STATUS_CHANNEL_SEL                  1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_SRC_SEL                    20
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_SRC_SEL                     2
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_SRC_SEL                    18
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_SRC_SEL                     2
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_DRIVE_EN                   17
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_DRIVE_EN                    1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_DRIVE_EN                   16
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_DRIVE_EN                    1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_STD_PREAMBLE_ENA                         6
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_STD_PREAMBLE_ENA                         1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SPI4_INTERLEAVE_MODE                     5
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_SPI4_INTERLEAVE_MODE                     1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_TAG_FORM                            4
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_XAUI_TAG_FORM                            1
#define VTSS_OFF_DEVCPU_GCB_CHIP_MODE_HOST_MODE                                0
#define VTSS_LEN_DEVCPU_GCB_CHIP_MODE_HOST_MODE                                4

#define VTSS_ADDR_DEVCPU_GCB_GPIO_OUTPUT_ENA                              0x0020
#define VTSS_OFF_DEVCPU_GCB_GPIO_OUTPUT_ENA_OE                                 0
#define VTSS_LEN_DEVCPU_GCB_GPIO_OUTPUT_ENA_OE                                16

#define VTSS_ADDR_DEVCPU_GCB_GPIO_O                                       0x0021
#define VTSS_OFF_DEVCPU_GCB_GPIO_O_G_OUT                                       0
#define VTSS_LEN_DEVCPU_GCB_GPIO_O_G_OUT                                      16

#define VTSS_ADDR_DEVCPU_GCB_GPIO_STATUS                                  0x0022
#define VTSS_OFF_DEVCPU_GCB_GPIO_STATUS_STATUS_STICKY                          0
#define VTSS_LEN_DEVCPU_GCB_GPIO_STATUS_STATUS_STICKY                         16

#define VTSS_ADDR_DEVCPU_GCB_GPIO_INT_MASK                                0x0023
#define VTSS_OFF_DEVCPU_GCB_GPIO_INT_MASK_GPIO_INT_MASK                        0
#define VTSS_LEN_DEVCPU_GCB_GPIO_INT_MASK_GPIO_INT_MASK                       16

#define VTSS_ADDR_DEVCPU_GCB_GPIO_I                                       0x0024
#define VTSS_OFF_DEVCPU_GCB_GPIO_I_G_IN                                        0
#define VTSS_LEN_DEVCPU_GCB_GPIO_I_G_IN                                       16

#define VTSS_ADDR_DEVCPU_GCB_DEVCPU_RST_REGS                              0x0040
#define VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_AUTO_BIST_DISABLE                  9
#define VTSS_LEN_DEVCPU_GCB_DEVCPU_RST_REGS_AUTO_BIST_DISABLE                  1
#define VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_MEMLOCK_ENABLE                     8
#define VTSS_LEN_DEVCPU_GCB_DEVCPU_RST_REGS_MEMLOCK_ENABLE                     1
#define VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST                      1
#define VTSS_LEN_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST                      1
#define VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_NON_CFG_RST                   0
#define VTSS_LEN_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_NON_CFG_RST                   1

#define VTSS_ADDR_DEVCPU_GCB_SOFT_DEVCPU_RST                              0x0041
#define VTSS_OFF_DEVCPU_GCB_SOFT_DEVCPU_RST_SOFT_MISC_RST                      3
#define VTSS_LEN_DEVCPU_GCB_SOFT_DEVCPU_RST_SOFT_MISC_RST                      1

#define VTSS_ADDR_DEVCPU_GCB_DBG                                          0x0050
#define VTSS_OFF_DEVCPU_GCB_DBG_ORG_DROPPED                                    8
#define VTSS_LEN_DEVCPU_GCB_DBG_ORG_DROPPED                                    5
#define VTSS_OFF_DEVCPU_GCB_DBG_TGT_DROPPED                                    0
#define VTSS_LEN_DEVCPU_GCB_DBG_TGT_DROPPED                                    8

#define VTSS_ADDR_DEVCPU_GCB_WD_SETUP                                     0x0051
#define VTSS_OFF_DEVCPU_GCB_WD_SETUP_TIME_STAMP_VALUE                          0
#define VTSS_LEN_DEVCPU_GCB_WD_SETUP_TIME_STAMP_VALUE                          8

#define VTSS_ADDR_DEVCPU_GCB_MIIM                                         0x0080
#define VTSS_WIDTH_DEVCPU_GCB_MIIM                                        0x0040
#define VTSS_ADDX_DEVCPU_GCB_MIIM(x)                                      (VTSS_ADDR_DEVCPU_GCB_MIIM + (x)*VTSS_WIDTH_DEVCPU_GCB_MIIM)

#define VTSS_ADDR_DEVCPU_GCB_MIIM_STATUS                                  0x0000
#define VTSS_ADDX_DEVCPU_GCB_MIIM_STATUS(x)                               (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_STATUS)
#define VTSS_OFF_DEVCPU_GCB_MIIM_STATUS_MIIM_SCAN_COMPLETE                     4
#define VTSS_LEN_DEVCPU_GCB_MIIM_STATUS_MIIM_SCAN_COMPLETE                     1
#define VTSS_OFF_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_BUSY                         3
#define VTSS_LEN_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_BUSY                         1
#define VTSS_OFF_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_OPR_PEND                     2
#define VTSS_LEN_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_OPR_PEND                     1
#define VTSS_OFF_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_PENDING_RD                   1
#define VTSS_LEN_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_PENDING_RD                   1
#define VTSS_OFF_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_PENDING_WR                   0
#define VTSS_LEN_DEVCPU_GCB_MIIM_STATUS_MIIM_STAT_PENDING_WR                   1

#define VTSS_ADDR_DEVCPU_GCB_MIIM_CFG_7226                                0x0001
#define VTSS_ADDX_DEVCPU_GCB_MIIM_CFG_7226(x)                             (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_CFG_7226)
#define VTSS_OFF_DEVCPU_GCB_MIIM_CFG_7226_MIIM_7226_CFG_FIELD                  9
#define VTSS_LEN_DEVCPU_GCB_MIIM_CFG_7226_MIIM_7226_CFG_FIELD                  1

#define VTSS_ADDR_DEVCPU_GCB_MIIM_CMD                                     0x0002
#define VTSS_ADDX_DEVCPU_GCB_MIIM_CMD(x)                                  (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_CMD)
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_VLD                             31
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_VLD                              1
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_SINGLE_SCAN                     29
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_SINGLE_SCAN                      1
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_PHYAD                           24
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_PHYAD                            5
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_REGAD                           19
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_REGAD                            5
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_SCAN                            18
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_SCAN                             1
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_WRDATA                           2
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_WRDATA                          16
#define VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_OPR_FIELD                        0
#define VTSS_LEN_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_OPR_FIELD                        2

#define VTSS_ADDR_DEVCPU_GCB_MIIM_DATA                                    0x0003
#define VTSS_ADDX_DEVCPU_GCB_MIIM_DATA(x)                                 (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_DATA)
#define VTSS_OFF_DEVCPU_GCB_MIIM_DATA_MIIM_DATA_SUCCESS                       16
#define VTSS_LEN_DEVCPU_GCB_MIIM_DATA_MIIM_DATA_SUCCESS                        2
#define VTSS_OFF_DEVCPU_GCB_MIIM_DATA_MIIM_DATA_RDDATA                         0
#define VTSS_LEN_DEVCPU_GCB_MIIM_DATA_MIIM_DATA_RDDATA                        16

#define VTSS_ADDR_DEVCPU_GCB_MIIM_CFG                                     0x0004
#define VTSS_ADDX_DEVCPU_GCB_MIIM_CFG(x)                                  (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_CFG)
#define VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_DBG                             10
#define VTSS_LEN_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_DBG                              1
#define VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_ST_CFG_FIELD                         8
#define VTSS_LEN_DEVCPU_GCB_MIIM_CFG_MIIM_ST_CFG_FIELD                         2
#define VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_PRESCALE                         0
#define VTSS_LEN_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_PRESCALE                         8

#define VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_0                                  0x0005
#define VTSS_ADDX_DEVCPU_GCB_MIIM_SCAN_0(x)                               (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_0)
#define VTSS_OFF_DEVCPU_GCB_MIIM_SCAN_0_MIIM_SCAN_PHYADHI                      5
#define VTSS_LEN_DEVCPU_GCB_MIIM_SCAN_0_MIIM_SCAN_PHYADHI                      5
#define VTSS_OFF_DEVCPU_GCB_MIIM_SCAN_0_MIIM_SCAN_PHYADLO                      0
#define VTSS_LEN_DEVCPU_GCB_MIIM_SCAN_0_MIIM_SCAN_PHYADLO                      5

#define VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_1                                  0x0006
#define VTSS_ADDX_DEVCPU_GCB_MIIM_SCAN_1(x)                               (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_1)
#define VTSS_OFF_DEVCPU_GCB_MIIM_SCAN_1_MIIM_SCAN_MASK                        16
#define VTSS_LEN_DEVCPU_GCB_MIIM_SCAN_1_MIIM_SCAN_MASK                        16
#define VTSS_OFF_DEVCPU_GCB_MIIM_SCAN_1_MIIM_SCAN_EXPECT                       0
#define VTSS_LEN_DEVCPU_GCB_MIIM_SCAN_1_MIIM_SCAN_EXPECT                      16

#define VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS                         0x0007
#define VTSS_ADDX_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS(x)                      (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS)

#define VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS_VLD                     0x0008
#define VTSS_ADDX_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS_VLD(x)                  (VTSS_ADDX_DEVCPU_GCB_MIIM(x) + VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_LAST_RSLTS_VLD)

#define VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_RESULTS_STICKY                     0x0100
#define VTSS_ADDX_DEVCPU_GCB_MIIM_SCAN_RESULTS_STICKY(x)                  (VTSS_ADDR_DEVCPU_GCB_MIIM_SCAN_RESULTS_STICKY + (x))

#define VTSS_ADDR_DEVCPU_GCB_MWR_ACCESS                                   0x0110
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_START_ACCESS                       28
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_START_ACCESS                        1
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_WR_BUSY                            27
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_WR_BUSY                             1
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_WR_ERR                             26
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_WR_ERR                              1
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_WR_ERR_STICKY                      25
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_WR_ERR_STICKY                       1
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_ACCESS_TYPE                        24
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_ACCESS_TYPE                         1
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_REG_ADDR                           16
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_REG_ADDR                            8
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_REG_WR_DATA                         8
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_REG_WR_DATA                         8
#define VTSS_OFF_DEVCPU_GCB_MWR_ACCESS_MWR_MODULE_ID                           0
#define VTSS_LEN_DEVCPU_GCB_MWR_ACCESS_MWR_MODULE_ID                           8

#define VTSS_ADDR_DEVCPU_GCB_MWR_RD_RSLT                                  0x0111
#define VTSS_OFF_DEVCPU_GCB_MWR_RD_RSLT_MWR_RD_BUSY                            9
#define VTSS_LEN_DEVCPU_GCB_MWR_RD_RSLT_MWR_RD_BUSY                            1
#define VTSS_OFF_DEVCPU_GCB_MWR_RD_RSLT_MWR_RD_ERR                             8
#define VTSS_LEN_DEVCPU_GCB_MWR_RD_RSLT_MWR_RD_ERR                             1
#define VTSS_OFF_DEVCPU_GCB_MWR_RD_RSLT_MWR_REG_RD_DATA                        0
#define VTSS_LEN_DEVCPU_GCB_MWR_RD_RSLT_MWR_REG_RD_DATA                        8

#define VTSS_ADDR_DEVCPU_GCB_SGMII_IDDQ                                   0x0120
#define VTSS_OFF_DEVCPU_GCB_SGMII_IDDQ_SGMII_IDDQ                              0
#define VTSS_LEN_DEVCPU_GCB_SGMII_IDDQ_SGMII_IDDQ                              6

#define VTSS_ADDR_DEVCPU_GCB_SGMII_PLL_BYP                                0x0121
#define VTSS_OFF_DEVCPU_GCB_SGMII_PLL_BYP_SGMII_TEST_BUS_ENABLE               16
#define VTSS_LEN_DEVCPU_GCB_SGMII_PLL_BYP_SGMII_TEST_BUS_ENABLE                1
#define VTSS_OFF_DEVCPU_GCB_SGMII_PLL_BYP_SGMII_PLL_BYP                        0
#define VTSS_LEN_DEVCPU_GCB_SGMII_PLL_BYP_SGMII_PLL_BYP                        6

/*********************************************************************** 
 * Target ASM
 * Assembler
 ***********************************************************************/
#define VTSS_ADDR_ASM_DEV_STATISTICS                                      0x0000
#define VTSS_WIDTH_ASM_DEV_STATISTICS                                     0x0040
#define VTSS_ADDX_ASM_DEV_STATISTICS(x)                                   (VTSS_ADDR_ASM_DEV_STATISTICS + (x)*VTSS_WIDTH_ASM_DEV_STATISTICS)

#define VTSS_ADDR_ASM_RX_IN_BYTES_CNT                                     0x0000
#define VTSS_ADDX_ASM_RX_IN_BYTES_CNT(x)                                  (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_IN_BYTES_CNT)

#define VTSS_ADDR_ASM_RX_SYMBOL_ERR_CNT                                   0x0001
#define VTSS_ADDX_ASM_RX_SYMBOL_ERR_CNT(x)                                (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SYMBOL_ERR_CNT)

#define VTSS_ADDR_ASM_RX_PAUSE_CNT                                        0x0002
#define VTSS_ADDX_ASM_RX_PAUSE_CNT(x)                                     (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_PAUSE_CNT)

#define VTSS_ADDR_ASM_RX_UNSUP_OPCODE_CNT                                 0x0003
#define VTSS_ADDX_ASM_RX_UNSUP_OPCODE_CNT(x)                              (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_UNSUP_OPCODE_CNT)

#define VTSS_ADDR_ASM_RX_OK_BYTES_CNT                                     0x0004
#define VTSS_ADDX_ASM_RX_OK_BYTES_CNT(x)                                  (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_OK_BYTES_CNT)

#define VTSS_ADDR_ASM_RX_BAD_BYTES_CNT                                    0x0005
#define VTSS_ADDX_ASM_RX_BAD_BYTES_CNT(x)                                 (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_BAD_BYTES_CNT)

#define VTSS_ADDR_ASM_RX_UC_CNT                                           0x0006
#define VTSS_ADDX_ASM_RX_UC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_UC_CNT)

#define VTSS_ADDR_ASM_RX_MC_CNT                                           0x0007
#define VTSS_ADDX_ASM_RX_MC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_MC_CNT)

#define VTSS_ADDR_ASM_RX_BC_CNT                                           0x0008
#define VTSS_ADDX_ASM_RX_BC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_BC_CNT)

#define VTSS_ADDR_ASM_RX_CRC_ERR_CNT                                      0x0009
#define VTSS_ADDX_ASM_RX_CRC_ERR_CNT(x)                                   (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_CRC_ERR_CNT)

#define VTSS_ADDR_ASM_RX_UNDERSIZE_CNT                                    0x000a
#define VTSS_ADDX_ASM_RX_UNDERSIZE_CNT(x)                                 (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_UNDERSIZE_CNT)

#define VTSS_ADDR_ASM_RX_FRAGMENTS_CNT                                    0x000b
#define VTSS_ADDX_ASM_RX_FRAGMENTS_CNT(x)                                 (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_FRAGMENTS_CNT)

#define VTSS_ADDR_ASM_RX_IN_RANGE_LEN_ERR_CNT                             0x000c
#define VTSS_ADDX_ASM_RX_IN_RANGE_LEN_ERR_CNT(x)                          (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_IN_RANGE_LEN_ERR_CNT)

#define VTSS_ADDR_ASM_RX_OUT_OF_RANGE_LEN_ERR_CNT                         0x000d
#define VTSS_ADDX_ASM_RX_OUT_OF_RANGE_LEN_ERR_CNT(x)                      (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_OUT_OF_RANGE_LEN_ERR_CNT)

#define VTSS_ADDR_ASM_RX_OVERSIZE_CNT                                     0x000e
#define VTSS_ADDX_ASM_RX_OVERSIZE_CNT(x)                                  (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_OVERSIZE_CNT)

#define VTSS_ADDR_ASM_RX_JABBERS_CNT                                      0x000f
#define VTSS_ADDX_ASM_RX_JABBERS_CNT(x)                                   (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_JABBERS_CNT)

#define VTSS_ADDR_ASM_RX_SIZE64_CNT                                       0x0010
#define VTSS_ADDX_ASM_RX_SIZE64_CNT(x)                                    (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE64_CNT)

#define VTSS_ADDR_ASM_RX_SIZE65TO127_CNT                                  0x0011
#define VTSS_ADDX_ASM_RX_SIZE65TO127_CNT(x)                               (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE65TO127_CNT)

#define VTSS_ADDR_ASM_RX_SIZE128TO255_CNT                                 0x0012
#define VTSS_ADDX_ASM_RX_SIZE128TO255_CNT(x)                              (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE128TO255_CNT)

#define VTSS_ADDR_ASM_RX_SIZE256TO511_CNT                                 0x0013
#define VTSS_ADDX_ASM_RX_SIZE256TO511_CNT(x)                              (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE256TO511_CNT)

#define VTSS_ADDR_ASM_RX_SIZE512TO1023_CNT                                0x0014
#define VTSS_ADDX_ASM_RX_SIZE512TO1023_CNT(x)                             (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE512TO1023_CNT)

#define VTSS_ADDR_ASM_RX_SIZE1024TO1518_CNT                               0x0015
#define VTSS_ADDX_ASM_RX_SIZE1024TO1518_CNT(x)                            (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE1024TO1518_CNT)

#define VTSS_ADDR_ASM_RX_SIZE1519TOMAX_CNT                                0x0016
#define VTSS_ADDX_ASM_RX_SIZE1519TOMAX_CNT(x)                             (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_SIZE1519TOMAX_CNT)

#define VTSS_ADDR_ASM_RX_IPG_SHRINK_CNT                                   0x0017
#define VTSS_ADDX_ASM_RX_IPG_SHRINK_CNT(x)                                (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_RX_IPG_SHRINK_CNT)

#define VTSS_ADDR_ASM_TX_OUT_BYTES_CNT                                    0x0018
#define VTSS_ADDX_ASM_TX_OUT_BYTES_CNT(x)                                 (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_OUT_BYTES_CNT)

#define VTSS_ADDR_ASM_TX_PAUSE_CNT                                        0x0019
#define VTSS_ADDX_ASM_TX_PAUSE_CNT(x)                                     (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_PAUSE_CNT)

#define VTSS_ADDR_ASM_TX_OK_BYTES_CNT                                     0x001a
#define VTSS_ADDX_ASM_TX_OK_BYTES_CNT(x)                                  (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_OK_BYTES_CNT)

#define VTSS_ADDR_ASM_TX_UC_CNT                                           0x001b
#define VTSS_ADDX_ASM_TX_UC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_UC_CNT)

#define VTSS_ADDR_ASM_TX_MC_CNT                                           0x001c
#define VTSS_ADDX_ASM_TX_MC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_MC_CNT)

#define VTSS_ADDR_ASM_TX_BC_CNT                                           0x001d
#define VTSS_ADDX_ASM_TX_BC_CNT(x)                                        (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_BC_CNT)

#define VTSS_ADDR_ASM_TX_SIZE64_CNT                                       0x001e
#define VTSS_ADDX_ASM_TX_SIZE64_CNT(x)                                    (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE64_CNT)

#define VTSS_ADDR_ASM_TX_SIZE65TO127_CNT                                  0x001f
#define VTSS_ADDX_ASM_TX_SIZE65TO127_CNT(x)                               (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE65TO127_CNT)

#define VTSS_ADDR_ASM_TX_SIZE128TO255_CNT                                 0x0020
#define VTSS_ADDX_ASM_TX_SIZE128TO255_CNT(x)                              (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE128TO255_CNT)

#define VTSS_ADDR_ASM_TX_SIZE256TO511_CNT                                 0x0021
#define VTSS_ADDX_ASM_TX_SIZE256TO511_CNT(x)                              (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE256TO511_CNT)

#define VTSS_ADDR_ASM_TX_SIZE512TO1023_CNT                                0x0022
#define VTSS_ADDX_ASM_TX_SIZE512TO1023_CNT(x)                             (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE512TO1023_CNT)

#define VTSS_ADDR_ASM_TX_SIZE1024TO1518_CNT                               0x0023
#define VTSS_ADDX_ASM_TX_SIZE1024TO1518_CNT(x)                            (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE1024TO1518_CNT)

#define VTSS_ADDR_ASM_TX_SIZE1519TOMAX_CNT                                0x0024
#define VTSS_ADDX_ASM_TX_SIZE1519TOMAX_CNT(x)                             (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_SIZE1519TOMAX_CNT)

#define VTSS_ADDR_ASM_TX_MULTI_COLL_CNT                                   0x0025
#define VTSS_ADDX_ASM_TX_MULTI_COLL_CNT(x)                                (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_MULTI_COLL_CNT)

#define VTSS_ADDR_ASM_TX_LATE_COLL_CNT                                    0x0026
#define VTSS_ADDX_ASM_TX_LATE_COLL_CNT(x)                                 (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_LATE_COLL_CNT)

#define VTSS_ADDR_ASM_TX_XCOLL_CNT                                        0x0027
#define VTSS_ADDX_ASM_TX_XCOLL_CNT(x)                                     (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_XCOLL_CNT)

#define VTSS_ADDR_ASM_TX_DEFER_CNT                                        0x0028
#define VTSS_ADDX_ASM_TX_DEFER_CNT(x)                                     (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_DEFER_CNT)

#define VTSS_ADDR_ASM_TX_XDEFER_CNT                                       0x0029
#define VTSS_ADDX_ASM_TX_XDEFER_CNT(x)                                    (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_XDEFER_CNT)

#define VTSS_ADDR_ASM_TX_BACKOFF1_CNT                                     0x002a
#define VTSS_ADDX_ASM_TX_BACKOFF1_CNT(x)                                  (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_BACKOFF1_CNT)

#define VTSS_ADDR_ASM_TX_CSENSE_CNT                                       0x002b
#define VTSS_ADDX_ASM_TX_CSENSE_CNT(x)                                    (VTSS_ADDX_ASM_DEV_STATISTICS(x) + VTSS_ADDR_ASM_TX_CSENSE_CNT)

#define VTSS_ADDR_ASM_STAT_CFG                                            0x0600
#define VTSS_OFF_ASM_STAT_CFG_STAT_CNT_CLR_SHOT                                0
#define VTSS_LEN_ASM_STAT_CFG_STAT_CNT_CLR_SHOT                                1

#define VTSS_ADDR_ASM_CBC_CFG                                             0x0601
#define VTSS_ADDX_ASM_CBC_CFG(x)                                          (VTSS_ADDR_ASM_CBC_CFG + (x))
#define VTSS_OFF_ASM_CBC_CFG_DEV_GRP_NUM                                       0
#define VTSS_LEN_ASM_CBC_CFG_DEV_GRP_NUM                                       5

#define VTSS_ADDR_ASM_MAC_ADDR_HIGH_CFG                                   0x0641
#define VTSS_ADDX_ASM_MAC_ADDR_HIGH_CFG(x)                                (VTSS_ADDR_ASM_MAC_ADDR_HIGH_CFG + (x))
#define VTSS_OFF_ASM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH                           0
#define VTSS_LEN_ASM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH                          24

#define VTSS_ADDR_ASM_MAC_ADDR_LOW_CFG                                    0x065b
#define VTSS_ADDX_ASM_MAC_ADDR_LOW_CFG(x)                                 (VTSS_ADDR_ASM_MAC_ADDR_LOW_CFG + (x))
#define VTSS_OFF_ASM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW                             0
#define VTSS_LEN_ASM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW                            24

#define VTSS_ADDR_ASM_PAUSE_CFG                                           0x0675
#define VTSS_ADDX_ASM_PAUSE_CFG(x)                                        (VTSS_ADDR_ASM_PAUSE_CFG + (x))
#define VTSS_OFF_ASM_PAUSE_CFG_ABORT_PAUSE_ENA                                 0
#define VTSS_LEN_ASM_PAUSE_CFG_ABORT_PAUSE_ENA                                 1

#define VTSS_ADDR_ASM_HIH_CFG                                             0x068f
#define VTSS_ADDX_ASM_HIH_CFG(x)                                          (VTSS_ADDR_ASM_HIH_CFG + (x))
#define VTSS_OFF_ASM_HIH_CFG_HIH_CHK_ENA                                       0
#define VTSS_LEN_ASM_HIH_CFG_HIH_CHK_ENA                                       1

#define VTSS_ADDR_ASM_LPORT_MAP_CFG                                       0x0691
#define VTSS_ADDX_ASM_LPORT_MAP_CFG(x)                                    (VTSS_ADDR_ASM_LPORT_MAP_CFG + (x))
#define VTSS_OFF_ASM_LPORT_MAP_CFG_LPORT_MAP_MODE                              0
#define VTSS_LEN_ASM_LPORT_MAP_CFG_LPORT_MAP_MODE                              1

#define VTSS_ADDR_ASM_LUT_INIT_CFG                                        0x0693
#define VTSS_OFF_ASM_LUT_INIT_CFG_LPORT_MAP_INIT                               0
#define VTSS_LEN_ASM_LUT_INIT_CFG_LPORT_MAP_INIT                               1

#define VTSS_ADDR_ASM_FC_FRAME_CFG                                        0x0694
#define VTSS_ADDX_ASM_FC_FRAME_CFG(x)                                     (VTSS_ADDR_ASM_FC_FRAME_CFG + (x))
#define VTSS_OFF_ASM_FC_FRAME_CFG_DSTN_SLOT_ID                                16
#define VTSS_LEN_ASM_FC_FRAME_CFG_DSTN_SLOT_ID                                 5
#define VTSS_OFF_ASM_FC_FRAME_CFG_FRAME_MATCH_ENA                              0
#define VTSS_LEN_ASM_FC_FRAME_CFG_FRAME_MATCH_ENA                              2

#define VTSS_ADDR_ASM_FC_DMAC_HIGH_CFG                                    0x0696
#define VTSS_ADDX_ASM_FC_DMAC_HIGH_CFG(x)                                 (VTSS_ADDR_ASM_FC_DMAC_HIGH_CFG + (x))
#define VTSS_OFF_ASM_FC_DMAC_HIGH_CFG_FC_DMAC_HIGH                             0
#define VTSS_LEN_ASM_FC_DMAC_HIGH_CFG_FC_DMAC_HIGH                            24

#define VTSS_ADDR_ASM_FC_DMAC_LOW_CFG                                     0x0698
#define VTSS_ADDX_ASM_FC_DMAC_LOW_CFG(x)                                  (VTSS_ADDR_ASM_FC_DMAC_LOW_CFG + (x))
#define VTSS_OFF_ASM_FC_DMAC_LOW_CFG_FC_DMAC_LOW                               0
#define VTSS_LEN_ASM_FC_DMAC_LOW_CFG_FC_DMAC_LOW                              24

#define VTSS_ADDR_ASM_FC_ETYPE_CFG                                        0x069a
#define VTSS_ADDX_ASM_FC_ETYPE_CFG(x)                                     (VTSS_ADDR_ASM_FC_ETYPE_CFG + (x))
#define VTSS_OFF_ASM_FC_ETYPE_CFG_FC_ETYPE                                     0
#define VTSS_LEN_ASM_FC_ETYPE_CFG_FC_ETYPE                                    16

#define VTSS_ADDR_ASM_PORT_LUT_CFG                                        0x069c
#define VTSS_ADDX_ASM_PORT_LUT_CFG(x)                                     (VTSS_ADDR_ASM_PORT_LUT_CFG + (x))
#define VTSS_OFF_ASM_PORT_LUT_CFG_CELLBUS_PORT_OFFSET                          8
#define VTSS_LEN_ASM_PORT_LUT_CFG_CELLBUS_PORT_OFFSET                          5
#define VTSS_OFF_ASM_PORT_LUT_CFG_HOST_PORT_NUM_ENA                            4
#define VTSS_LEN_ASM_PORT_LUT_CFG_HOST_PORT_NUM_ENA                            1
#define VTSS_OFF_ASM_PORT_LUT_CFG_OFFSET_ENA                                   0
#define VTSS_LEN_ASM_PORT_LUT_CFG_OFFSET_ENA                                   1

#define VTSS_ADDR_ASM_DBG_CFG                                             0x169c
#define VTSS_OFF_ASM_DBG_CFG_FIFO_RST_1G                                       8
#define VTSS_LEN_ASM_DBG_CFG_FIFO_RST_1G                                      24
#define VTSS_OFF_ASM_DBG_CFG_FIFO_RST_10G                                      3
#define VTSS_LEN_ASM_DBG_CFG_FIFO_RST_10G                                      2
#define VTSS_OFF_ASM_DBG_CFG_FIFO_RST_SPI                                      2
#define VTSS_LEN_ASM_DBG_CFG_FIFO_RST_SPI                                      1

#define VTSS_ADDR_ASM_HIH_CNT                                             0x169d
#define VTSS_ADDX_ASM_HIH_CNT(x)                                          (VTSS_ADDR_ASM_HIH_CNT + (x))

#define VTSS_ADDR_ASM_SPI_ERR_STICKY                                      0x169f
#define VTSS_OFF_ASM_SPI_ERR_STICKY_FC_UFLW_STICKY                            12
#define VTSS_LEN_ASM_SPI_ERR_STICKY_FC_UFLW_STICKY                             1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_FC_OFLW_STICKY                            11
#define VTSS_LEN_ASM_SPI_ERR_STICKY_FC_OFLW_STICKY                             1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_CC_INTRN_ERR_STICKY                       10
#define VTSS_LEN_ASM_SPI_ERR_STICKY_CC_INTRN_ERR_STICKY                        1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_CC_UFLW_STICKY                             9
#define VTSS_LEN_ASM_SPI_ERR_STICKY_CC_UFLW_STICKY                             1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_CC_OFLW_STICKY                             8
#define VTSS_LEN_ASM_SPI_ERR_STICKY_CC_OFLW_STICKY                             1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_MAIN_SM_INTRN_ERR_STICKY                   1
#define VTSS_LEN_ASM_SPI_ERR_STICKY_MAIN_SM_INTRN_ERR_STICKY                   1
#define VTSS_OFF_ASM_SPI_ERR_STICKY_MAIN_SM_OFLW_STICKY                        0
#define VTSS_LEN_ASM_SPI_ERR_STICKY_MAIN_SM_OFLW_STICKY                        1

#define VTSS_ADDR_ASM_NON_SPI_ERR_STICKY                                  0x16a0
#define VTSS_OFF_ASM_NON_SPI_ERR_STICKY_HIH_STICKY                            16
#define VTSS_LEN_ASM_NON_SPI_ERR_STICKY_HIH_STICKY                             1
#define VTSS_OFF_ASM_NON_SPI_ERR_STICKY_FIFO_OFLW_STICKY                       0
#define VTSS_LEN_ASM_NON_SPI_ERR_STICKY_FIFO_OFLW_STICKY                       4

#define VTSS_ADDR_ASM_ALL_ERR_STICKY                                      0x16a1
#define VTSS_ADDX_ASM_ALL_ERR_STICKY(x)                                   (VTSS_ADDR_ASM_ALL_ERR_STICKY + (x))
#define VTSS_OFF_ASM_ALL_ERR_STICKY_CALENDAR_STICKY                            7
#define VTSS_LEN_ASM_ALL_ERR_STICKY_CALENDAR_STICKY                            1
#define VTSS_OFF_ASM_ALL_ERR_STICKY_UNUSED_BYTES_STICKY                        6
#define VTSS_LEN_ASM_ALL_ERR_STICKY_UNUSED_BYTES_STICKY                        1
#define VTSS_OFF_ASM_ALL_ERR_STICKY_FRAGMENT_STICKY                            5
#define VTSS_LEN_ASM_ALL_ERR_STICKY_FRAGMENT_STICKY                            1
#define VTSS_OFF_ASM_ALL_ERR_STICKY_MISSING_EOF_STICKY                         4
#define VTSS_LEN_ASM_ALL_ERR_STICKY_MISSING_EOF_STICKY                         1
#define VTSS_OFF_ASM_ALL_ERR_STICKY_INVLD_ABORT_STICKY                         3
#define VTSS_LEN_ASM_ALL_ERR_STICKY_INVLD_ABORT_STICKY                         1
#define VTSS_OFF_ASM_ALL_ERR_STICKY_MISSING_SOF_STICKY                         2
#define VTSS_LEN_ASM_ALL_ERR_STICKY_MISSING_SOF_STICKY                         1

#define VTSS_ADDR_ASM_LUT_STICKY                                          0x16a6
#define VTSS_OFF_ASM_LUT_STICKY_LUT_WRONG_PORT_STICKY                          8
#define VTSS_LEN_ASM_LUT_STICKY_LUT_WRONG_PORT_STICKY                          1
#define VTSS_OFF_ASM_LUT_STICKY_LUT_SOFT_STICKY                                0
#define VTSS_LEN_ASM_LUT_STICKY_LUT_SOFT_STICKY                                1

#define VTSS_ADDR_ASM_CSC_STICKY                                          0x16a7
#define VTSS_OFF_ASM_CSC_STICKY_CSC_SOFT_STICKY                                2
#define VTSS_LEN_ASM_CSC_STICKY_CSC_SOFT_STICKY                                1
#define VTSS_OFF_ASM_CSC_STICKY_UNSUP_OPCODE_PRE_CNT_OFLW_STICKY               1
#define VTSS_LEN_ASM_CSC_STICKY_UNSUP_OPCODE_PRE_CNT_OFLW_STICKY               1
#define VTSS_OFF_ASM_CSC_STICKY_PAUSE_FRM_PRE_CNT_OFLW_STICKY                  0
#define VTSS_LEN_ASM_CSC_STICKY_PAUSE_FRM_PRE_CNT_OFLW_STICKY                  1

#define VTSS_ADDR_ASM_SRC_SLOT_FC_STICKY                                  0x16a8

#define VTSS_ADDR_ASM_SPI_ERR_MASK                                        0x16a9
#define VTSS_OFF_ASM_SPI_ERR_MASK_SPI4_OFLW_MASK                               0
#define VTSS_LEN_ASM_SPI_ERR_MASK_SPI4_OFLW_MASK                               1

#define VTSS_ADDR_ASM_NON_SPI_ERR_MASK                                    0x16aa
#define VTSS_OFF_ASM_NON_SPI_ERR_MASK_HIH_MASK                                16
#define VTSS_LEN_ASM_NON_SPI_ERR_MASK_HIH_MASK                                 1
#define VTSS_OFF_ASM_NON_SPI_ERR_MASK_FIFO_OFLW_MASK                           0
#define VTSS_LEN_ASM_NON_SPI_ERR_MASK_FIFO_OFLW_MASK                           4

#define VTSS_ADDR_ASM_ALL_ERR_MASK                                        0x16ab
#define VTSS_ADDX_ASM_ALL_ERR_MASK(x)                                     (VTSS_ADDR_ASM_ALL_ERR_MASK + (x))
#define VTSS_OFF_ASM_ALL_ERR_MASK_CALENDAR_MASK                                7
#define VTSS_LEN_ASM_ALL_ERR_MASK_CALENDAR_MASK                                1

#define VTSS_ADDR_ASM_LUT_MASK                                            0x16b0
#define VTSS_OFF_ASM_LUT_MASK_LUT_SOFT_MASK                                    0
#define VTSS_LEN_ASM_LUT_MASK_LUT_SOFT_MASK                                    1

#define VTSS_ADDR_ASM_CSC_MASK                                            0x16b1
#define VTSS_OFF_ASM_CSC_MASK_CSC_SOFT_MASK                                    2
#define VTSS_LEN_ASM_CSC_MASK_CSC_SOFT_MASK                                    1

#define VTSS_ADDR_ASM_SPARE_RW_REG                                        0x16b2

#define VTSS_ADDR_ASM_SPARE_RO_REG                                        0x16b3

#define VTSS_ADDR_ASM_SPI4_CH_CFG                                         0x16b4
#define VTSS_OFF_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_MODE_SEL                         3
#define VTSS_LEN_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_MODE_SEL                         2
#define VTSS_OFF_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_ENA                              2
#define VTSS_LEN_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_ENA                              1

#define VTSS_ADDR_ASM_SPI4_FC_CFG                                         0x16b5
#define VTSS_OFF_ASM_SPI4_FC_CFG_SPI4_FC_WM                                    1
#define VTSS_LEN_ASM_SPI4_FC_CFG_SPI4_FC_WM                                    8
#define VTSS_OFF_ASM_SPI4_FC_CFG_SPI4_FC_ENA                                   0
#define VTSS_LEN_ASM_SPI4_FC_CFG_SPI4_FC_ENA                                   1

#define VTSS_ADDR_ASM_SPI4_STATUS                                         0x16b6
#define VTSS_ADDX_ASM_SPI4_STATUS(x)                                      (VTSS_ADDR_ASM_SPI4_STATUS + (x))

#define VTSS_ADDR_ASM_FC_FORCE                                            0x16e6
#define VTSS_ADDX_ASM_FC_FORCE(x)                                         (VTSS_ADDR_ASM_FC_FORCE + (x))
#define VTSS_OFF_ASM_FC_FORCE_FC_FORCE_ENA                                     1
#define VTSS_LEN_ASM_FC_FORCE_FC_FORCE_ENA                                     1
#define VTSS_OFF_ASM_FC_FORCE_FC_FORCE_VAL                                     0
#define VTSS_LEN_ASM_FC_FORCE_FC_FORCE_VAL                                     1

#define VTSS_ADDR_ASM_FC_STATE_STICKY                                     0x1716
#define VTSS_ADDX_ASM_FC_STATE_STICKY(x)                                  (VTSS_ADDR_ASM_FC_STATE_STICKY + (x))
#define VTSS_OFF_ASM_FC_STATE_STICKY_FC_STATE_STICKY                           0
#define VTSS_LEN_ASM_FC_STATE_STICKY_FC_STATE_STICKY                           1

/*********************************************************************** 
 * Target DSM
 * Disassembler
 ***********************************************************************/
#define VTSS_ADDR_DSM_RATE_CTRL_WM                                        0x0000
#define VTSS_OFF_DSM_RATE_CTRL_WM_TAXI_32_RATE_CTRL_WM                         8
#define VTSS_LEN_DSM_RATE_CTRL_WM_TAXI_32_RATE_CTRL_WM                         8
#define VTSS_OFF_DSM_RATE_CTRL_WM_TAXI_128_RATE_CTRL_WM                        0
#define VTSS_LEN_DSM_RATE_CTRL_WM_TAXI_128_RATE_CTRL_WM                        8

#define VTSS_ADDR_DSM_RATE_CTRL                                           0x0001
#define VTSS_ADDX_DSM_RATE_CTRL(x)                                        (VTSS_ADDR_DSM_RATE_CTRL + (x))
#define VTSS_OFF_DSM_RATE_CTRL_FRM_GAP_COMP                                   28
#define VTSS_LEN_DSM_RATE_CTRL_FRM_GAP_COMP                                    4
#define VTSS_OFF_DSM_RATE_CTRL_TAXI_RATE_HIGH                                 16
#define VTSS_LEN_DSM_RATE_CTRL_TAXI_RATE_HIGH                                 12
#define VTSS_OFF_DSM_RATE_CTRL_TAXI_RATE_LOW                                   0
#define VTSS_LEN_DSM_RATE_CTRL_TAXI_RATE_LOW                                  12

#define VTSS_ADDR_DSM_CLR_BUF                                             0x001b
#define VTSS_OFF_DSM_CLR_BUF_CLR_BUF                                           0
#define VTSS_LEN_DSM_CLR_BUF_CLR_BUF                                          27

#define VTSS_ADDR_DSM_SCH_STOP_WM_CFG                                     0x001c
#define VTSS_ADDX_DSM_SCH_STOP_WM_CFG(x)                                  (VTSS_ADDR_DSM_SCH_STOP_WM_CFG + (x))
#define VTSS_OFF_DSM_SCH_STOP_WM_CFG_SCH_STOP_WM                               0
#define VTSS_LEN_DSM_SCH_STOP_WM_CFG_SCH_STOP_WM                               4

#define VTSS_ADDR_DSM_RX_PAUSE_CFG                                        0x0036
#define VTSS_ADDX_DSM_RX_PAUSE_CFG(x)                                     (VTSS_ADDR_DSM_RX_PAUSE_CFG + (x))
#define VTSS_OFF_DSM_RX_PAUSE_CFG_RX_PAUSE_EN                                  1
#define VTSS_LEN_DSM_RX_PAUSE_CFG_RX_PAUSE_EN                                  1

#define VTSS_ADDR_DSM_ETH_FC_GEN                                          0x0050
#define VTSS_ADDX_DSM_ETH_FC_GEN(x)                                       (VTSS_ADDR_DSM_ETH_FC_GEN + (x))
#define VTSS_OFF_DSM_ETH_FC_GEN_ETH_PORT_FC_GEN                                0
#define VTSS_LEN_DSM_ETH_FC_GEN_ETH_PORT_FC_GEN                                1

#define VTSS_ADDR_DSM_MAC_CFG                                             0x006a
#define VTSS_ADDX_DSM_MAC_CFG(x)                                          (VTSS_ADDR_DSM_MAC_CFG + (x))
#define VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_VAL                                     16
#define VTSS_LEN_DSM_MAC_CFG_TX_PAUSE_VAL                                     16
#define VTSS_OFF_DSM_MAC_CFG_HDX_BACKPRESSURE                                  2
#define VTSS_LEN_DSM_MAC_CFG_HDX_BACKPRESSURE                                  1
#define VTSS_OFF_DSM_MAC_CFG_SEND_PAUSE_FRM_TWICE                              1
#define VTSS_LEN_DSM_MAC_CFG_SEND_PAUSE_FRM_TWICE                              1
#define VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_XON_XOFF                                 0
#define VTSS_LEN_DSM_MAC_CFG_TX_PAUSE_XON_XOFF                                 1

#define VTSS_ADDR_DSM_MAC_ADDR_HIGH_CFG                                   0x0084
#define VTSS_ADDX_DSM_MAC_ADDR_HIGH_CFG(x)                                (VTSS_ADDR_DSM_MAC_ADDR_HIGH_CFG + (x))
#define VTSS_OFF_DSM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH                           0
#define VTSS_LEN_DSM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH                          24

#define VTSS_ADDR_DSM_MAC_ADDR_LOW_CFG                                    0x009e
#define VTSS_ADDX_DSM_MAC_ADDR_LOW_CFG(x)                                 (VTSS_ADDR_DSM_MAC_ADDR_LOW_CFG + (x))
#define VTSS_OFF_DSM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW                             0
#define VTSS_LEN_DSM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW                            24

#define VTSS_ADDR_DSM_SPI4_CFG                                            0x00b8
#define VTSS_OFF_DSM_SPI4_CFG_MAXBURST2                                       24
#define VTSS_LEN_DSM_SPI4_CFG_MAXBURST2                                        8
#define VTSS_OFF_DSM_SPI4_CFG_MAXBURST1                                       16
#define VTSS_LEN_DSM_SPI4_CFG_MAXBURST1                                        8
#define VTSS_OFF_DSM_SPI4_CFG_SPI4_BURST_SIZE                                  8
#define VTSS_LEN_DSM_SPI4_CFG_SPI4_BURST_SIZE                                  4
#define VTSS_OFF_DSM_SPI4_CFG_SPI4_ALTER_HIH_PLI_VALUE                         3
#define VTSS_LEN_DSM_SPI4_CFG_SPI4_ALTER_HIH_PLI_VALUE                         1
#define VTSS_OFF_DSM_SPI4_CFG_SPI4_STRIP_FCS                                   2
#define VTSS_LEN_DSM_SPI4_CFG_SPI4_STRIP_FCS                                   1
#define VTSS_OFF_DSM_SPI4_CFG_SPI4_HDR_CFG                                     0
#define VTSS_LEN_DSM_SPI4_CFG_SPI4_HDR_CFG                                     2

#define VTSS_ADDR_DSM_SPI4_STOP_SCH_WM                                    0x00b9
#define VTSS_OFF_DSM_SPI4_STOP_SCH_WM_SPI4_STOP_SCH_WM                         0
#define VTSS_LEN_DSM_SPI4_STOP_SCH_WM_SPI4_STOP_SCH_WM                         6

#define VTSS_ADDR_DSM_SPI4_HI_CH_CLR_CREDITS                              0x00ba
#define VTSS_OFF_DSM_SPI4_HI_CH_CLR_CREDITS_SPI4_HI_CH_CLR_CREDITS             0
#define VTSS_LEN_DSM_SPI4_HI_CH_CLR_CREDITS_SPI4_HI_CH_CLR_CREDITS            24

#define VTSS_ADDR_DSM_SPI4_LO_CH_CLR_CREDITS                              0x00bb
#define VTSS_OFF_DSM_SPI4_LO_CH_CLR_CREDITS_SPI4_LO_CH_CLR_CREDITS             0
#define VTSS_LEN_DSM_SPI4_LO_CH_CLR_CREDITS_SPI4_LO_CH_CLR_CREDITS            24

#define VTSS_ADDR_DSM_SPI4_HI_CH_BUF_FLUSH                                0x00bc
#define VTSS_OFF_DSM_SPI4_HI_CH_BUF_FLUSH_SPI4_HI_CH_BUF_FLUSH                 0
#define VTSS_LEN_DSM_SPI4_HI_CH_BUF_FLUSH_SPI4_HI_CH_BUF_FLUSH                24

#define VTSS_ADDR_DSM_SPI4_LO_CH_BUF_FLUSH                                0x00bd
#define VTSS_OFF_DSM_SPI4_LO_CH_BUF_FLUSH_SPI4_LO_CH_BUF_FLUSH                 0
#define VTSS_LEN_DSM_SPI4_LO_CH_BUF_FLUSH_SPI4_LO_CH_BUF_FLUSH                24

#define VTSS_ADDR_DSM_DBG_CTRL                                            0x00be
#define VTSS_OFF_DSM_DBG_CTRL_DBG_EVENT_CTRL                                   0
#define VTSS_LEN_DSM_DBG_CTRL_DBG_EVENT_CTRL                                   3

#define VTSS_ADDR_DSM_IFH_CFG                                             0x00bf
#define VTSS_OFF_DSM_IFH_CFG_KEEP_INVLD_IFH_CHKSUM                             0
#define VTSS_LEN_DSM_IFH_CFG_KEEP_INVLD_IFH_CHKSUM                             1

#define VTSS_ADDR_DSM_FCS_CFG                                             0x00c0
#define VTSS_ADDX_DSM_FCS_CFG(x)                                          (VTSS_ADDR_DSM_FCS_CFG + (x))
#define VTSS_OFF_DSM_FCS_CFG_PORT_FCS_CHK_ENA                                  0
#define VTSS_LEN_DSM_FCS_CFG_PORT_FCS_CHK_ENA                                  1

#define VTSS_ADDR_DSM_TST_FRM_CNT_ENA                                     0x0120

#define VTSS_ADDR_DSM_TST_FRM_CNT_STRT                                    0x0121
#define VTSS_OFF_DSM_TST_FRM_CNT_STRT_TST_FRM_CNT_STRT                         0
#define VTSS_LEN_DSM_TST_FRM_CNT_STRT_TST_FRM_CNT_STRT                         1

#define VTSS_ADDR_DSM_TST_FRM_CNT_RST                                     0x0122
#define VTSS_OFF_DSM_TST_FRM_CNT_RST_TST_FRM_CNT_RST                           0
#define VTSS_LEN_DSM_TST_FRM_CNT_RST_TST_FRM_CNT_RST                           1

#define VTSS_ADDR_DSM_TST_FRM_CNT_VAL_LSB                                 0x0123
#define VTSS_ADDX_DSM_TST_FRM_CNT_VAL_LSB(x)                              (VTSS_ADDR_DSM_TST_FRM_CNT_VAL_LSB + (x))

#define VTSS_ADDR_DSM_TST_FRM_CNT_VAL_MSB                                 0x013e
#define VTSS_ADDX_DSM_TST_FRM_CNT_VAL_MSB(x)                              (VTSS_ADDR_DSM_TST_FRM_CNT_VAL_MSB + (x))
#define VTSS_OFF_DSM_TST_FRM_CNT_VAL_MSB_TST_FRM_CNT_VAL_MSB                   0
#define VTSS_LEN_DSM_TST_FRM_CNT_VAL_MSB_TST_FRM_CNT_VAL_MSB                   8

#define VTSS_ADDR_DSM_SPARE_CTRL                                          0x0159

#define VTSS_ADDR_DSM_FCS_ERR_MASK                                        0x015a
#define VTSS_OFF_DSM_FCS_ERR_MASK_FCS_ERR_MASK                                 0
#define VTSS_LEN_DSM_FCS_ERR_MASK_FCS_ERR_MASK                                 1

#define VTSS_ADDR_DSM_IFH_ERR_MASK                                        0x015b
#define VTSS_OFF_DSM_IFH_ERR_MASK_IFH_BIP8_ERR_MASK                            0
#define VTSS_LEN_DSM_IFH_ERR_MASK_IFH_BIP8_ERR_MASK                            1

#define VTSS_ADDR_DSM_CELL_BUS_ERR_MASK                                   0x015c
#define VTSS_OFF_DSM_CELL_BUS_ERR_MASK_CELL_BUS_ILLEGAL_PORT_NUM_MASK          3
#define VTSS_LEN_DSM_CELL_BUS_ERR_MASK_CELL_BUS_ILLEGAL_PORT_NUM_MASK          1
#define VTSS_OFF_DSM_CELL_BUS_ERR_MASK_CONS_CELL_FOR_SAME_TAXI_MASK            2
#define VTSS_LEN_DSM_CELL_BUS_ERR_MASK_CONS_CELL_FOR_SAME_TAXI_MASK            1
#define VTSS_OFF_DSM_CELL_BUS_ERR_MASK_CELL_BUS_MISSING_SOF_MASK               1
#define VTSS_LEN_DSM_CELL_BUS_ERR_MASK_CELL_BUS_MISSING_SOF_MASK               1
#define VTSS_OFF_DSM_CELL_BUS_ERR_MASK_CELL_BUS_MISSING_EOF_MASK               0
#define VTSS_LEN_DSM_CELL_BUS_ERR_MASK_CELL_BUS_MISSING_EOF_MASK               1

#define VTSS_ADDR_DSM_DEMUX_CELL_ERR_MASK                                 0x015d
#define VTSS_OFF_DSM_DEMUX_CELL_ERR_MASK_DEMUX_CELL_ERR_MASK                   0
#define VTSS_LEN_DSM_DEMUX_CELL_ERR_MASK_DEMUX_CELL_ERR_MASK                   1

#define VTSS_ADDR_DSM_PRE_CNT_OFLW_MASK                                   0x015e
#define VTSS_OFF_DSM_PRE_CNT_OFLW_MASK_PRE_CNT_OFLW_MASK                      31
#define VTSS_LEN_DSM_PRE_CNT_OFLW_MASK_PRE_CNT_OFLW_MASK                       1

#define VTSS_ADDR_DSM_BUF_OFLW_MASK                                       0x015f

#define VTSS_ADDR_DSM_BUF_UFLW_MASK                                       0x0160

#define VTSS_ADDR_DSM_SPI4_LO_CH_BUF_OFLW_MASK                            0x0161
#define VTSS_OFF_DSM_SPI4_LO_CH_BUF_OFLW_MASK_SPI4_LO_CH_BUF_OFLW_MASK         0
#define VTSS_LEN_DSM_SPI4_LO_CH_BUF_OFLW_MASK_SPI4_LO_CH_BUF_OFLW_MASK        24

#define VTSS_ADDR_DSM_SPI4_HI_CH_BUF_OFLW_MASK                            0x0162
#define VTSS_OFF_DSM_SPI4_HI_CH_BUF_OFLW_MASK_SPI4_HI_CH_BUF_OFLW_MASK         0
#define VTSS_LEN_DSM_SPI4_HI_CH_BUF_OFLW_MASK_SPI4_HI_CH_BUF_OFLW_MASK        24

#define VTSS_ADDR_DSM_SPI4_LO_CH_BUF_UFLW_MASK                            0x0163
#define VTSS_OFF_DSM_SPI4_LO_CH_BUF_UFLW_MASK_SPI4_LO_CH_BUF_UFLW_MASK         0
#define VTSS_LEN_DSM_SPI4_LO_CH_BUF_UFLW_MASK_SPI4_LO_CH_BUF_UFLW_MASK        24

#define VTSS_ADDR_DSM_SPI4_HI_CH_BUF_UFLW_MASK                            0x0164
#define VTSS_OFF_DSM_SPI4_HI_CH_BUF_UFLW_MASK_SPI4_HI_CH_BUF_UFLW_MASK         0
#define VTSS_LEN_DSM_SPI4_HI_CH_BUF_UFLW_MASK_SPI4_HI_CH_BUF_UFLW_MASK        24

#define VTSS_ADDR_DSM_SPI4_LO_CTXT_BUF_OFLW_MASK                          0x0165
#define VTSS_OFF_DSM_SPI4_LO_CTXT_BUF_OFLW_MASK_SPI4_LO_CTXT_BUF_OFLW_MASK      0
#define VTSS_LEN_DSM_SPI4_LO_CTXT_BUF_OFLW_MASK_SPI4_LO_CTXT_BUF_OFLW_MASK     24

#define VTSS_ADDR_DSM_SPI4_HI_CTXT_BUF_OFLW_MASK                          0x0166
#define VTSS_OFF_DSM_SPI4_HI_CTXT_BUF_OFLW_MASK_SPI4_HI_CTXT_BUF_OFLW_MASK      0
#define VTSS_LEN_DSM_SPI4_HI_CTXT_BUF_OFLW_MASK_SPI4_HI_CTXT_BUF_OFLW_MASK     24

#define VTSS_ADDR_DSM_RAM_PARITY_ERR_MASK                                 0x0167
#define VTSS_OFF_DSM_RAM_PARITY_ERR_MASK_RAM_PARITY_ERR_MASK                   0
#define VTSS_LEN_DSM_RAM_PARITY_ERR_MASK_RAM_PARITY_ERR_MASK                   7

#define VTSS_ADDR_DSM_FCS_STATUS                                          0x0168
#define VTSS_OFF_DSM_FCS_STATUS_FCS_ERR_STICKY                                 0
#define VTSS_LEN_DSM_FCS_STATUS_FCS_ERR_STICKY                                 1

#define VTSS_ADDR_DSM_IFH_STATUS                                          0x0169
#define VTSS_OFF_DSM_IFH_STATUS_IFH_BIP8_ERR_STICKY                            0
#define VTSS_LEN_DSM_IFH_STATUS_IFH_BIP8_ERR_STICKY                            1

#define VTSS_ADDR_DSM_TX_PAUSE_CNT                                        0x016a
#define VTSS_ADDX_DSM_TX_PAUSE_CNT(x)                                     (VTSS_ADDR_DSM_TX_PAUSE_CNT + (x))

#define VTSS_ADDR_DSM_DBG_CNT                                             0x016c
#define VTSS_ADDX_DSM_DBG_CNT(x)                                          (VTSS_ADDR_DSM_DBG_CNT + (x))

#define VTSS_ADDR_DSM_CELL_BUS_STICKY                                     0x01b7
#define VTSS_OFF_DSM_CELL_BUS_STICKY_CELL_BUS_ILLEGAL_PORT_NUM_STICKY          3
#define VTSS_LEN_DSM_CELL_BUS_STICKY_CELL_BUS_ILLEGAL_PORT_NUM_STICKY          1
#define VTSS_OFF_DSM_CELL_BUS_STICKY_CONS_CELL_FOR_SAME_TAXI_STICKY            2
#define VTSS_LEN_DSM_CELL_BUS_STICKY_CONS_CELL_FOR_SAME_TAXI_STICKY            1
#define VTSS_OFF_DSM_CELL_BUS_STICKY_CELL_BUS_MISSING_SOF_STICKY               1
#define VTSS_LEN_DSM_CELL_BUS_STICKY_CELL_BUS_MISSING_SOF_STICKY               1
#define VTSS_OFF_DSM_CELL_BUS_STICKY_CELL_BUS_MISSING_EOF_STICKY               0
#define VTSS_LEN_DSM_CELL_BUS_STICKY_CELL_BUS_MISSING_EOF_STICKY               1

#define VTSS_ADDR_DSM_DEMUX_CELL_ERR_STICKY                               0x01b8
#define VTSS_OFF_DSM_DEMUX_CELL_ERR_STICKY_DEMUX_CELL_ERR_STICKY               0
#define VTSS_LEN_DSM_DEMUX_CELL_ERR_STICKY_DEMUX_CELL_ERR_STICKY               1

#define VTSS_ADDR_DSM_PRE_CNT_OFLW_STICKY                                 0x01b9
#define VTSS_OFF_DSM_PRE_CNT_OFLW_STICKY_PRE_CNT_OFLW_STICKY                  31
#define VTSS_LEN_DSM_PRE_CNT_OFLW_STICKY_PRE_CNT_OFLW_STICKY                   1

#define VTSS_ADDR_DSM_BUF_OFLW_STICKY                                     0x01ba

#define VTSS_ADDR_DSM_BUF_UFLW_STICKY                                     0x01bb

#define VTSS_ADDR_DSM_SPI4_LO_CH_BUF_OFLW_STICKY                          0x01bc
#define VTSS_OFF_DSM_SPI4_LO_CH_BUF_OFLW_STICKY_SPI4_LO_CH_BUF_OFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_LO_CH_BUF_OFLW_STICKY_SPI4_LO_CH_BUF_OFLW_STICKY     24

#define VTSS_ADDR_DSM_SPI4_HI_CH_BUF_OFLW_STICKY                          0x01bd
#define VTSS_OFF_DSM_SPI4_HI_CH_BUF_OFLW_STICKY_SPI4_HI_CH_BUF_OFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_HI_CH_BUF_OFLW_STICKY_SPI4_HI_CH_BUF_OFLW_STICKY     24

#define VTSS_ADDR_DSM_SPI4_LO_CH_BUF_UFLW_STICKY                          0x01be
#define VTSS_OFF_DSM_SPI4_LO_CH_BUF_UFLW_STICKY_SPI4_LO_CH_BUF_UFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_LO_CH_BUF_UFLW_STICKY_SPI4_LO_CH_BUF_UFLW_STICKY     24

#define VTSS_ADDR_DSM_SPI4_HI_CH_BUF_UFLW_STICKY                          0x01bf
#define VTSS_OFF_DSM_SPI4_HI_CH_BUF_UFLW_STICKY_SPI4_HI_CH_BUF_UFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_HI_CH_BUF_UFLW_STICKY_SPI4_HI_CH_BUF_UFLW_STICKY     24

#define VTSS_ADDR_DSM_SPI4_LO_CTXT_BUF_OFLW_STICKY                        0x01c0
#define VTSS_OFF_DSM_SPI4_LO_CTXT_BUF_OFLW_STICKY_SPI4_LO_CTXT_BUF_OFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_LO_CTXT_BUF_OFLW_STICKY_SPI4_LO_CTXT_BUF_OFLW_STICKY     24

#define VTSS_ADDR_DSM_SPI4_HI_CTXT_BUF_OFLW_STICKY                        0x01c1
#define VTSS_OFF_DSM_SPI4_HI_CTXT_BUF_OFLW_STICKY_SPI4_HI_CTXT_BUF_OFLW_STICKY      0
#define VTSS_LEN_DSM_SPI4_HI_CTXT_BUF_OFLW_STICKY_SPI4_HI_CTXT_BUF_OFLW_STICKY     24

#define VTSS_ADDR_DSM_RAM_PARITY_ERR_STICKY                               0x01c2
#define VTSS_OFF_DSM_RAM_PARITY_ERR_STICKY_RAM_PARITY_ERR_STICKY               0
#define VTSS_LEN_DSM_RAM_PARITY_ERR_STICKY_RAM_PARITY_ERR_STICKY               7

#define VTSS_ADDR_DSM_SPARE_STATUS                                        0x01c3

#define VTSS_ADDR_DSM_XAUI0_TX_FC_FRAME_EN                                0x01c4
#define VTSS_OFF_DSM_XAUI0_TX_FC_FRAME_EN_XAUI0_TX_FC_FRAME_EN                 0
#define VTSS_LEN_DSM_XAUI0_TX_FC_FRAME_EN_XAUI0_TX_FC_FRAME_EN                 1

#define VTSS_ADDR_DSM_XAUI0_SOURCE_SLOT_ID                                0x01c5
#define VTSS_OFF_DSM_XAUI0_SOURCE_SLOT_ID_XAUI0_SOURCE_SLOT_ID                 0
#define VTSS_LEN_DSM_XAUI0_SOURCE_SLOT_ID_XAUI0_SOURCE_SLOT_ID                 5

#define VTSS_ADDR_DSM_XAUI0_FC_FRAME_DMAC_HI                              0x01c6
#define VTSS_OFF_DSM_XAUI0_FC_FRAME_DMAC_HI_XAUI0_FC_FRAME_DMAC_HI             0
#define VTSS_LEN_DSM_XAUI0_FC_FRAME_DMAC_HI_XAUI0_FC_FRAME_DMAC_HI            24

#define VTSS_ADDR_DSM_XAUI0_FC_FRAME_DMAC_LO                              0x01c7
#define VTSS_OFF_DSM_XAUI0_FC_FRAME_DMAC_LO_XAUI0_FC_FRAME_DMAC_LO             0
#define VTSS_LEN_DSM_XAUI0_FC_FRAME_DMAC_LO_XAUI0_FC_FRAME_DMAC_LO            24

#define VTSS_ADDR_DSM_XAUI0_FC_FRAME_ETYPE                                0x01c8
#define VTSS_OFF_DSM_XAUI0_FC_FRAME_ETYPE_XAUI0_FC_FRAME_ETYPE                 0
#define VTSS_LEN_DSM_XAUI0_FC_FRAME_ETYPE_XAUI0_FC_FRAME_ETYPE                16

#define VTSS_ADDR_DSM_XAUI0_INB_LPORT_CFG                                 0x01c9
#define VTSS_ADDX_DSM_XAUI0_INB_LPORT_CFG(x)                              (VTSS_ADDR_DSM_XAUI0_INB_LPORT_CFG + (x))
#define VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_FC_ID                     16
#define VTSS_LEN_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_FC_ID                      6
#define VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_SLOT_ID                    0
#define VTSS_LEN_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_SLOT_ID                    5

#define VTSS_ADDR_DSM_XAUI1_TX_FC_FRAME_EN                                0x01e1
#define VTSS_OFF_DSM_XAUI1_TX_FC_FRAME_EN_XAUI1_TX_FC_FRAME_EN                 0
#define VTSS_LEN_DSM_XAUI1_TX_FC_FRAME_EN_XAUI1_TX_FC_FRAME_EN                 1

#define VTSS_ADDR_DSM_XAUI1_SOURCE_SLOT_ID                                0x01e2
#define VTSS_OFF_DSM_XAUI1_SOURCE_SLOT_ID_XAUI1_SOURCE_SLOT_ID                 0
#define VTSS_LEN_DSM_XAUI1_SOURCE_SLOT_ID_XAUI1_SOURCE_SLOT_ID                 5

#define VTSS_ADDR_DSM_XAUI1_FC_FRAME_DMAC_HI                              0x01e3
#define VTSS_OFF_DSM_XAUI1_FC_FRAME_DMAC_HI_XAUI1_FC_FRAME_DMAC_HI             0
#define VTSS_LEN_DSM_XAUI1_FC_FRAME_DMAC_HI_XAUI1_FC_FRAME_DMAC_HI            24

#define VTSS_ADDR_DSM_XAUI1_FC_FRAME_DMAC_LO                              0x01e4
#define VTSS_OFF_DSM_XAUI1_FC_FRAME_DMAC_LO_XAUI1_FC_FRAME_DMAC_LO             0
#define VTSS_LEN_DSM_XAUI1_FC_FRAME_DMAC_LO_XAUI1_FC_FRAME_DMAC_LO            24

#define VTSS_ADDR_DSM_XAUI1_FC_FRAME_ETYPE                                0x01e5
#define VTSS_OFF_DSM_XAUI1_FC_FRAME_ETYPE_XAUI1_FC_FRAME_ETYPE                 0
#define VTSS_LEN_DSM_XAUI1_FC_FRAME_ETYPE_XAUI1_FC_FRAME_ETYPE                16

#define VTSS_ADDR_DSM_XAUI1_INB_LPORT_CFG                                 0x01e6
#define VTSS_ADDX_DSM_XAUI1_INB_LPORT_CFG(x)                              (VTSS_ADDR_DSM_XAUI1_INB_LPORT_CFG + (x))
#define VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_FC_ID                     16
#define VTSS_LEN_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_FC_ID                      6
#define VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_SLOT_ID                    0
#define VTSS_LEN_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_SLOT_ID                    5

#define VTSS_ADDR_DSM_SPI4_BS_ACTIVE                                      0x01fe
#define VTSS_OFF_DSM_SPI4_BS_ACTIVE_SPI4_BS_ACTIVE                             0
#define VTSS_LEN_DSM_SPI4_BS_ACTIVE_SPI4_BS_ACTIVE                             1

#define VTSS_ADDR_DSM_SPI4_BS_WEIGHT                                      0x02fe
#define VTSS_ADDX_DSM_SPI4_BS_WEIGHT(x)                                   (VTSS_ADDR_DSM_SPI4_BS_WEIGHT + (x))
#define VTSS_OFF_DSM_SPI4_BS_WEIGHT_SPI4_BS_WEIGHT                             0
#define VTSS_LEN_DSM_SPI4_BS_WEIGHT_SPI4_BS_WEIGHT                            21

#define VTSS_ADDR_DSM_SPI4_BS_DEFICIT                                     0x03fe
#define VTSS_ADDX_DSM_SPI4_BS_DEFICIT(x)                                  (VTSS_ADDR_DSM_SPI4_BS_DEFICIT + (x))
#define VTSS_OFF_DSM_SPI4_BS_DEFICIT_SPI4_BS_DEFICIT                           0
#define VTSS_LEN_DSM_SPI4_BS_DEFICIT_SPI4_BS_DEFICIT                          22

#define VTSS_ADDR_DSM_SPI4_BS_DWELL_COUNT_ENABLE                          0x04fe
#define VTSS_OFF_DSM_SPI4_BS_DWELL_COUNT_ENABLE_SPI4_BS_DWELL_COUNT_ENABLE      0
#define VTSS_LEN_DSM_SPI4_BS_DWELL_COUNT_ENABLE_SPI4_BS_DWELL_COUNT_ENABLE      1

#define VTSS_ADDR_DSM_SPI4_BS_DWELL_COUNT_PRESET                          0x05fe
#define VTSS_ADDX_DSM_SPI4_BS_DWELL_COUNT_PRESET(x)                       (VTSS_ADDR_DSM_SPI4_BS_DWELL_COUNT_PRESET + (x))
#define VTSS_OFF_DSM_SPI4_BS_DWELL_COUNT_PRESET_SPI4_BS_DWELL_COUNT_PRESET      0
#define VTSS_LEN_DSM_SPI4_BS_DWELL_COUNT_PRESET_SPI4_BS_DWELL_COUNT_PRESET      7

#define VTSS_ADDR_DSM_SPI4_BS_DWELL_COUNT_OUTPUT                          0x06fe
#define VTSS_OFF_DSM_SPI4_BS_DWELL_COUNT_OUTPUT_SPI4_BS_DWELL_COUNT_OUTPUT      0
#define VTSS_LEN_DSM_SPI4_BS_DWELL_COUNT_OUTPUT_SPI4_BS_DWELL_COUNT_OUTPUT      7

#define VTSS_ADDR_DSM_SPI4_BS_ELIGIBLE_FLAG_MS                            0x07fe
#define VTSS_OFF_DSM_SPI4_BS_ELIGIBLE_FLAG_MS_SPI4_BS_ELIGIBLE_FLAG_MS         0
#define VTSS_LEN_DSM_SPI4_BS_ELIGIBLE_FLAG_MS_SPI4_BS_ELIGIBLE_FLAG_MS        16

#define VTSS_ADDR_DSM_SPI4_BS_ELIGIBLE_FLAG_LS                            0x07ff

#define VTSS_ADDR_DSM_SPI4_BS_SCHED_STATE                                 0x08fe
#define VTSS_OFF_DSM_SPI4_BS_SCHED_STATE_SPI4_BS_SCHED_STATE                   0
#define VTSS_LEN_DSM_SPI4_BS_SCHED_STATE_SPI4_BS_SCHED_STATE                   6

/*********************************************************************** 
 * Target ANA_CL
 * Classifier sub block of the Analyzer
 ***********************************************************************/
#define VTSS_ADDR_ANA_CL_TAG_CTRL                                         0x0000
#define VTSS_WIDTH_ANA_CL_TAG_CTRL                                        0x0001
#define VTSS_ADDX_ANA_CL_TAG_CTRL(x)                                      (VTSS_ADDR_ANA_CL_TAG_CTRL + (x)*VTSS_WIDTH_ANA_CL_TAG_CTRL)

#define VTSS_ADDR_ANA_CL_TAG_AND_LBL_CFG                                  0x0000
#define VTSS_ADDX_ANA_CL_TAG_AND_LBL_CFG(x)                               (VTSS_ADDX_ANA_CL_TAG_CTRL(x) + VTSS_ADDR_ANA_CL_TAG_AND_LBL_CFG)
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_MAX_VLAN_TAGS                         16
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_MAX_VLAN_TAGS                          2
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_TAG_SEL                               12
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_TAG_SEL                                1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_STOP_ENA                          8
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_CTAG_STOP_ENA                          1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_CFI_STOP_ENA                      7
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_CTAG_CFI_STOP_ENA                      1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_STAGGED_DIS                       5
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_VLAN_STAGGED_DIS                       1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_TAGGED_DIS                        3
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_VLAN_TAGGED_DIS                        1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_PRIO_TAGGED_DIS                   2
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_VLAN_PRIO_TAGGED_DIS                   1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_UNTAGGED_DIS                      1
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_VLAN_UNTAGGED_DIS                      1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_AWARE_ENA                         0
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_CFG_VLAN_AWARE_ENA                         1

#define VTSS_ADDR_ANA_CL_ENDPT_DSCP_CFG                                   0x0018
#define VTSS_ADDX_ANA_CL_ENDPT_DSCP_CFG(x)                                (VTSS_ADDR_ANA_CL_ENDPT_DSCP_CFG + (x))
#define VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_TRUST_ENA                      8
#define VTSS_LEN_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_TRUST_ENA                      1
#define VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_RED                            4
#define VTSS_LEN_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_RED                            2
#define VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_PRIO                           0
#define VTSS_LEN_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_PRIO                           3

#define VTSS_ADDR_ANA_CL_ENDPT                                            0x0098
#define VTSS_WIDTH_ANA_CL_ENDPT                                           0x0080
#define VTSS_ADDX_ANA_CL_ENDPT(x)                                         (VTSS_ADDR_ANA_CL_ENDPT + (x)*VTSS_WIDTH_ANA_CL_ENDPT)

#define VTSS_ADDR_ANA_CL_ENDPT_REMAP_PRIO_CFG                             0x0000
#define VTSS_ADDX_ANA_CL_ENDPT_REMAP_PRIO_CFG(x)                          (VTSS_ADDR_ANA_CL_ENDPT_REMAP_PRIO_CFG + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_REMAP_PRIO_CFG(x, y)                      (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_REMAP_PRIO_CFG(y))
#define VTSS_OFF_ANA_CL_ENDPT_REMAP_PRIO_CFG_PRIO_ENDPT_REMAP                  0
#define VTSS_LEN_ANA_CL_ENDPT_REMAP_PRIO_CFG_PRIO_ENDPT_REMAP                  5

#define VTSS_ADDR_ANA_CL_ENDPT_REMAP_ALT_CFG                              0x0011
#define VTSS_ADDX_ANA_CL_ENDPT_REMAP_ALT_CFG(x)                           (VTSS_ADDR_ANA_CL_ENDPT_REMAP_ALT_CFG + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_REMAP_ALT_CFG(x, y)                       (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_REMAP_ALT_CFG(y))
#define VTSS_OFF_ANA_CL_ENDPT_REMAP_ALT_CFG_ALT_ENDPT_REMAP                    0
#define VTSS_LEN_ANA_CL_ENDPT_REMAP_ALT_CFG_ALT_ENDPT_REMAP                    5

#define VTSS_ADDR_ANA_CL_ENDPT_L2_CTRL_CFG                                0x0022
#define VTSS_ADDX_ANA_CL_ENDPT_L2_CTRL_CFG(x)                             (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_L2_CTRL_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_RED                      4
#define VTSS_LEN_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_RED                      2
#define VTSS_OFF_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_PRIO                     0
#define VTSS_LEN_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_PRIO                     3

#define VTSS_ADDR_ANA_CL_ENDPT_L3_CTRL_CFG                                0x0023
#define VTSS_ADDX_ANA_CL_ENDPT_L3_CTRL_CFG(x)                             (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_L3_CTRL_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_RED                      4
#define VTSS_LEN_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_RED                      2
#define VTSS_OFF_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_PRIO                     0
#define VTSS_LEN_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_PRIO                     3

#define VTSS_ADDR_ANA_CL_ENDPT_CFI_CFG                                    0x0024
#define VTSS_ADDX_ANA_CL_ENDPT_CFI_CFG(x)                                 (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_CFI_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_RED                         4
#define VTSS_LEN_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_RED                         2
#define VTSS_OFF_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_PRIO                        0
#define VTSS_LEN_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_PRIO                        3

#define VTSS_ADDR_ANA_CL_ENDPT_TCPUDP_CFG_0                               0x0025
#define VTSS_ADDX_ANA_CL_ENDPT_TCPUDP_CFG_0(x)                            (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_TCPUDP_CFG_0)
#define VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_GLOBAL_TCPUDP_PORT_ENA          4
#define VTSS_LEN_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_GLOBAL_TCPUDP_PORT_ENA          8
#define VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_LOCAL_TCPUDP_RNG_ENA            0
#define VTSS_LEN_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_LOCAL_TCPUDP_RNG_ENA            1

#define VTSS_ADDR_ANA_CL_ENDPT_TCPUDP_CFG_1                               0x0026
#define VTSS_ADDX_ANA_CL_ENDPT_TCPUDP_CFG_1(x)                            (VTSS_ADDR_ANA_CL_ENDPT_TCPUDP_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_TCPUDP_CFG_1(x, y)                        (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_TCPUDP_CFG_1(y))
#define VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_VAL           8
#define VTSS_LEN_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_VAL          16
#define VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_RED           4
#define VTSS_LEN_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_RED           2
#define VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_PRIO          0
#define VTSS_LEN_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_PRIO          3

#define VTSS_ADDR_ANA_CL_ENDPT_IP_PROTO_CFG                               0x0028
#define VTSS_ADDX_ANA_CL_ENDPT_IP_PROTO_CFG(x)                            (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_IP_PROTO_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_VAL                    8
#define VTSS_LEN_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_VAL                    8
#define VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_RED                    4
#define VTSS_LEN_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_RED                    2
#define VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_PRIO                   0
#define VTSS_LEN_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_PRIO                   3

#define VTSS_ADDR_ANA_CL_ENDPT_IP4_CFG                                    0x0029
#define VTSS_ADDX_ANA_CL_ENDPT_IP4_CFG(x)                                 (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_IP4_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_RED                              4
#define VTSS_LEN_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_RED                              2
#define VTSS_OFF_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_PRIO                             0
#define VTSS_LEN_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_PRIO                             3

#define VTSS_ADDR_ANA_CL_ENDPT_IP6_CFG                                    0x002a
#define VTSS_ADDX_ANA_CL_ENDPT_IP6_CFG(x)                                 (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_IP6_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_RED                              4
#define VTSS_LEN_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_RED                              2
#define VTSS_OFF_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_PRIO                             0
#define VTSS_LEN_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_PRIO                             3

#define VTSS_ADDR_ANA_CL_ENDPT_ETYPE_CFG                                  0x002b
#define VTSS_ADDX_ANA_CL_ENDPT_ETYPE_CFG(x)                               (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_ETYPE_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_VAL                         16
#define VTSS_LEN_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_VAL                         16
#define VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_RED                          4
#define VTSS_LEN_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_RED                          2
#define VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_PRIO                         0
#define VTSS_LEN_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_PRIO                         3

#define VTSS_ADDR_ANA_CL_ENDPT_MPLS_CFG                                   0x002c
#define VTSS_ADDX_ANA_CL_ENDPT_MPLS_CFG(x)                                (VTSS_ADDR_ANA_CL_ENDPT_MPLS_CFG + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_MPLS_CFG(x, y)                            (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_MPLS_CFG(y))
#define VTSS_OFF_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_RED                            4
#define VTSS_LEN_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_RED                            2
#define VTSS_OFF_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_PRIO                           0
#define VTSS_LEN_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_PRIO                           3

#define VTSS_ADDR_ANA_CL_ENDPT_VID_CFG                                    0x0034
#define VTSS_ADDX_ANA_CL_ENDPT_VID_CFG(x)                                 (VTSS_ADDR_ANA_CL_ENDPT_VID_CFG + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_VID_CFG(x, y)                             (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_VID_CFG(y))
#define VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_VAL                              8
#define VTSS_LEN_ANA_CL_ENDPT_VID_CFG_QOS_VID_VAL                             12
#define VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_RED                              4
#define VTSS_LEN_ANA_CL_ENDPT_VID_CFG_QOS_VID_RED                              2
#define VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_PRIO                             0
#define VTSS_LEN_ANA_CL_ENDPT_VID_CFG_QOS_VID_PRIO                             3

#define VTSS_ADDR_ANA_CL_ENDPT_UPRIO_CFG                                  0x0036
#define VTSS_ADDX_ANA_CL_ENDPT_UPRIO_CFG(x)                               (VTSS_ADDR_ANA_CL_ENDPT_UPRIO_CFG + (x))
#define VTSS_ADDXY_ANA_CL_ENDPT_UPRIO_CFG(x, y)                           (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDX_ANA_CL_ENDPT_UPRIO_CFG(y))
#define VTSS_OFF_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_RED                          4
#define VTSS_LEN_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_RED                          2
#define VTSS_OFF_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_PRIO                         0
#define VTSS_LEN_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_PRIO                         3

#define VTSS_ADDR_ANA_CL_ENDPT_SNAP_CFG                                   0x003e
#define VTSS_ADDX_ANA_CL_ENDPT_SNAP_CFG(x)                                (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_SNAP_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_RED                            4
#define VTSS_LEN_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_RED                            2
#define VTSS_OFF_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_PRIO                           0
#define VTSS_LEN_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_PRIO                           3

#define VTSS_ADDR_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG                    0x003f
#define VTSS_ADDX_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG(x)                 (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_RED      4
#define VTSS_LEN_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_RED      2
#define VTSS_OFF_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_PRIO      0
#define VTSS_LEN_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_PRIO      3

#define VTSS_ADDR_ANA_CL_ENDPT_DEFAULT_CFG                                0x0040
#define VTSS_ADDX_ANA_CL_ENDPT_DEFAULT_CFG(x)                             (VTSS_ADDX_ANA_CL_ENDPT(x) + VTSS_ADDR_ANA_CL_ENDPT_DEFAULT_CFG)
#define VTSS_OFF_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_RED                      4
#define VTSS_LEN_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_RED                      2
#define VTSS_OFF_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_PRIO                     0
#define VTSS_LEN_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_PRIO                     3

#define VTSS_ADDR_ANA_CL_PORT                                             0x0c98
#define VTSS_WIDTH_ANA_CL_PORT                                            0x0040
#define VTSS_ADDX_ANA_CL_PORT(x)                                          (VTSS_ADDR_ANA_CL_PORT + (x)*VTSS_WIDTH_ANA_CL_PORT)

#define VTSS_ADDR_ANA_CL_FILTER_CTRL                                      0x0000
#define VTSS_ADDX_ANA_CL_FILTER_CTRL(x)                                   (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDR_ANA_CL_FILTER_CTRL)
#define VTSS_OFF_ANA_CL_FILTER_CTRL_ALT_ORDER_ENA                             24
#define VTSS_LEN_ANA_CL_FILTER_CTRL_ALT_ORDER_ENA                              1
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_VID_ENA                            20
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_VID_ENA                             2
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_ETYPE_ENA                          18
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_ETYPE_ENA                           2
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_DMAC_ENA                           12
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_DMAC_ENA                            2
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_MAC_CTRL_ENA                       11
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_MAC_CTRL_ENA                        1
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_NULL_MAC_ENA                       10
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_NULL_MAC_ENA                        1
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_BC_ENA                              8
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_BC_ENA                              1
#define VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_SMAC_MC_ENA                         2
#define VTSS_LEN_ANA_CL_FILTER_CTRL_FILTER_SMAC_MC_ENA                         1

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0                         0x0001
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0(x)                      (VTSS_ADDR_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0(x, y)                  (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0_FILTER_ETYPE                  0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_ETYPE_CFG_0_FILTER_ETYPE                 16

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1                         0x0003
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1(x)                      (VTSS_ADDR_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1(x, y)                  (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1_FILTER_ETYPE_MASK             0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_ETYPE_CFG_1_FILTER_ETYPE_MASK            16

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_0                               0x0005
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_0(x)                            (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_0 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CFG_0(x, y)                        (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_0(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CFG_0_FILTER_DMAC_MSB_PATTERN             0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CFG_0_FILTER_DMAC_MSB_PATTERN            16

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_1                               0x0007
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_1(x)                            (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CFG_1(x, y)                        (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_1(y))

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_2                               0x0009
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_2(x)                            (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_2 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CFG_2(x, y)                        (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_2(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CFG_2_FILTER_DMAC_MSB_MASK                0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CFG_2_FILTER_DMAC_MSB_MASK               16

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_3                               0x000b
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_3(x)                            (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CFG_3 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CFG_3(x, y)                        (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CFG_3(y))

#define VTSS_ADDR_ANA_CL_FILTER_VID_CFG_0                                 0x000d
#define VTSS_ADDX_ANA_CL_FILTER_VID_CFG_0(x)                              (VTSS_ADDR_ANA_CL_FILTER_VID_CFG_0 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_VID_CFG_0(x, y)                          (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_VID_CFG_0(y))
#define VTSS_OFF_ANA_CL_FILTER_VID_CFG_0_FILTER_VID_PATTERN                    0
#define VTSS_LEN_ANA_CL_FILTER_VID_CFG_0_FILTER_VID_PATTERN                   12

#define VTSS_ADDR_ANA_CL_FILTER_VID_CFG_1                                 0x000f
#define VTSS_ADDX_ANA_CL_FILTER_VID_CFG_1(x)                              (VTSS_ADDR_ANA_CL_FILTER_VID_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_VID_CFG_1(x, y)                          (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_VID_CFG_1(y))
#define VTSS_OFF_ANA_CL_FILTER_VID_CFG_1_FILTER_VID_MASK                       0
#define VTSS_LEN_ANA_CL_FILTER_VID_CFG_1_FILTER_VID_MASK                      12

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0                        0x0011
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0(x)                     (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0(x, y)                 (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_POS      4
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_POS      6
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_TYPE      0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_TYPE      2

#define VTSS_ADDR_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1                        0x0018
#define VTSS_ADDX_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1(x)                     (VTSS_ADDR_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1(x, y)                 (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1(y))
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_MASK     16
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_MASK     16
#define VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_PATTERN      0
#define VTSS_LEN_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_PATTERN     16

#define VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG                         0x001f
#define VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG(x)                      (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG)
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_FILTER_GLOBAL_CUSTOM_PORTMASK      0
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_FILTER_GLOBAL_CUSTOM_PORTMASK      8

#define VTSS_ADDR_ANA_CL_MISC_PORT_CFG                                    0x0020
#define VTSS_ADDX_ANA_CL_MISC_PORT_CFG(x)                                 (VTSS_ADDX_ANA_CL_PORT(x) + VTSS_ADDR_ANA_CL_MISC_PORT_CFG)
#define VTSS_OFF_ANA_CL_MISC_PORT_CFG_ALIGN_IP6_HOP_BY_HOP_ENA                 7
#define VTSS_LEN_ANA_CL_MISC_PORT_CFG_ALIGN_IP6_HOP_BY_HOP_ENA                 1
#define VTSS_OFF_ANA_CL_MISC_PORT_CFG_FILTER_LOCAL_CUSTOM_ENA                  0
#define VTSS_LEN_ANA_CL_MISC_PORT_CFG_FILTER_LOCAL_CUSTOM_ENA                  1

#define VTSS_ADDR_ANA_CL_COMMON_PORT                                      0x1298
#define VTSS_WIDTH_ANA_CL_COMMON_PORT                                     0x000f
#define VTSS_ADDX_ANA_CL_COMMON_PORT(x)                                   (VTSS_ADDR_ANA_CL_COMMON_PORT + (x)*VTSS_WIDTH_ANA_CL_COMMON_PORT)

#define VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0                       0x0000
#define VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0(x)                    (VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0(x, y)                (VTSS_ADDX_ANA_CL_COMMON_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0(y))
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_POS      4
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_POS      7
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_TYPE      0
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_TYPE      2

#define VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1                       0x0007
#define VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1(x)                    (VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1 + (x))
#define VTSS_ADDXY_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1(x, y)                (VTSS_ADDX_ANA_CL_COMMON_PORT(x) + VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1(y))
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_MASK     16
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_MASK     16
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_PATTERN      0
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_PATTERN     16

#define VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_2                       0x000e
#define VTSS_ADDX_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_2(x)                    (VTSS_ADDX_ANA_CL_COMMON_PORT(x) + VTSS_ADDR_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_2)
#define VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_2_FILTER_GLOBAL_CUSTOM_ENA      0
#define VTSS_LEN_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_2_FILTER_GLOBAL_CUSTOM_ENA      1

#define VTSS_ADDR_ANA_CL_IFH_CHKSUM_CTRL                                  0x1310
#define VTSS_ADDX_ANA_CL_IFH_CHKSUM_CTRL(x)                               (VTSS_ADDR_ANA_CL_IFH_CHKSUM_CTRL + (x))
#define VTSS_OFF_ANA_CL_IFH_CHKSUM_CTRL_IFH_CHKSUM_CHK_ENA                     0
#define VTSS_LEN_ANA_CL_IFH_CHKSUM_CTRL_IFH_CHKSUM_CHK_ENA                     1

#define VTSS_ADDR_ANA_CL_DSCP_REMAP_IDX_CFG                               0x1328
#define VTSS_OFF_ANA_CL_DSCP_REMAP_IDX_CFG_DSCP_REMAP_IDX                      0
#define VTSS_LEN_ANA_CL_DSCP_REMAP_IDX_CFG_DSCP_REMAP_IDX                     24

#define VTSS_ADDR_ANA_CL_STAG_ETYPE_CFG                                   0x1329
#define VTSS_OFF_ANA_CL_STAG_ETYPE_CFG_STAG_ETYPE_VAL                          0
#define VTSS_LEN_ANA_CL_STAG_ETYPE_CFG_STAG_ETYPE_VAL                         16

#define VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG                          0x132a
#define VTSS_ADDX_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG(x)                       (VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG + (x))
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_VAL          8
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_VAL         16
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_RED      4
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_RED      2
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_PRIO      0
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_PRIO      3

#define VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_TCPUDP_RNG_CFG                      0x1332
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_RNG_CFG_QOS_GLOBAL_TCPUDP_RNG_ENA      0
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_TCPUDP_RNG_CFG_QOS_GLOBAL_TCPUDP_RNG_ENA      4

#define VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG                   0x1333
#define VTSS_ADDX_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG(x)                (VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG + (x))
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_RED      4
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_RED      2
#define VTSS_OFF_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_PRIO      0
#define VTSS_LEN_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_PRIO      3

#define VTSS_ADDR_ANA_CL_DEBUG_CFG                                        0x133b
#define VTSS_OFF_ANA_CL_DEBUG_CFG_CFG_RAM_INIT                                 0
#define VTSS_LEN_ANA_CL_DEBUG_CFG_CFG_RAM_INIT                                 1

#define VTSS_ADDR_ANA_CL_SOFT_STICKY                                      0x133c
#define VTSS_OFF_ANA_CL_SOFT_STICKY_PORT_CFG_SOFT_STICKY                       1
#define VTSS_LEN_ANA_CL_SOFT_STICKY_PORT_CFG_SOFT_STICKY                       1
#define VTSS_OFF_ANA_CL_SOFT_STICKY_ENDPT_CFG_SOFT_STICKY                      0
#define VTSS_LEN_ANA_CL_SOFT_STICKY_ENDPT_CFG_SOFT_STICKY                      1

#define VTSS_ADDR_ANA_CL_SOFT_MASK                                        0x133d
#define VTSS_OFF_ANA_CL_SOFT_MASK_PORT_CFG_SOFT_MASK                           1
#define VTSS_LEN_ANA_CL_SOFT_MASK_PORT_CFG_SOFT_MASK                           1
#define VTSS_OFF_ANA_CL_SOFT_MASK_ENDPT_CFG_SOFT_MASK                          0
#define VTSS_LEN_ANA_CL_SOFT_MASK_ENDPT_CFG_SOFT_MASK                          1

#define VTSS_ADDR_ANA_CL_SPARE_RW_REG                                     0x133e

#define VTSS_ADDR_ANA_CL_SPARE_RO_REG                                     0x133f

#define VTSS_ADDR_ANA_CL_TAG_AND_LBL_STICKY                               0x1340
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_IFH_CHK_FAILED_STICKY              13
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_IFH_CHK_FAILED_STICKY               1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_FILTER_MAX_TAG_STICKY              10
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_FILTER_MAX_TAG_STICKY               1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_FILTER_SINGLE_SERVICE_STICKY        9
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_FILTER_SINGLE_SERVICE_STICKY        1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_FILTER_PRIO_TAGGED_STICKY           8
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_FILTER_PRIO_TAGGED_STICKY           1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_FILTER_TAGGED_STICKY                7
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_FILTER_TAGGED_STICKY                1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_STICKY_FILTER_UNTAGGED_STICKY              6
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_STICKY_FILTER_UNTAGGED_STICKY              1

#define VTSS_ADDR_ANA_CL_DETECTION_STICKY                                 0x1341
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_FILTER_GLOBAL_CUSTOM_STICKY           6
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_FILTER_GLOBAL_CUSTOM_STICKY           1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_MAC_CTRL_STICKY                       5
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_MAC_CTRL_STICKY                       1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_FILTER_L2_BC_STICKY                   4
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_FILTER_L2_BC_STICKY                   1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_FILTER_ETYPE_OR_MAC_STICKY            3
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_FILTER_ETYPE_OR_MAC_STICKY            1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_BAD_MACS_STICKY                       2
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_BAD_MACS_STICKY                       1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_SMAC_MC_STICKY                        1
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_SMAC_MC_STICKY                        1
#define VTSS_OFF_ANA_CL_DETECTION_STICKY_FILTER_LOCAL_CUSTOM_STICKY            0
#define VTSS_LEN_ANA_CL_DETECTION_STICKY_FILTER_LOCAL_CUSTOM_STICKY            1

#define VTSS_ADDR_ANA_CL_STICKY_MASK                                      0x1342
#define VTSS_WIDTH_ANA_CL_STICKY_MASK                                     0x0010
#define VTSS_ADDX_ANA_CL_STICKY_MASK(x)                                   (VTSS_ADDR_ANA_CL_STICKY_MASK + (x)*VTSS_WIDTH_ANA_CL_STICKY_MASK)

#define VTSS_ADDR_ANA_CL_TAG_AND_LBL_MASK                                 0x0000
#define VTSS_ADDX_ANA_CL_TAG_AND_LBL_MASK(x)                              (VTSS_ADDX_ANA_CL_STICKY_MASK(x) + VTSS_ADDR_ANA_CL_TAG_AND_LBL_MASK)
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_IFH_CHK_FAILED_STICKY_MASK           13
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_IFH_CHK_FAILED_STICKY_MASK            1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_FILTER_MAX_TAG_STICKY_MASK           10
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_FILTER_MAX_TAG_STICKY_MASK            1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_FILTER_SINGLE_SERVICE_STICKY_MASK      9
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_FILTER_SINGLE_SERVICE_STICKY_MASK      1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_FILTER_PRIO_TAGGED_STICKY_MASK        8
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_FILTER_PRIO_TAGGED_STICKY_MASK        1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_FILTER_TAGGED_STICKY_MASK             7
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_FILTER_TAGGED_STICKY_MASK             1
#define VTSS_OFF_ANA_CL_TAG_AND_LBL_MASK_FILTER_UNTAGGED_STICKY_MASK           6
#define VTSS_LEN_ANA_CL_TAG_AND_LBL_MASK_FILTER_UNTAGGED_STICKY_MASK           1

#define VTSS_ADDR_ANA_CL_DETECTION_MASK                                   0x0001
#define VTSS_ADDX_ANA_CL_DETECTION_MASK(x)                                (VTSS_ADDX_ANA_CL_STICKY_MASK(x) + VTSS_ADDR_ANA_CL_DETECTION_MASK)
#define VTSS_OFF_ANA_CL_DETECTION_MASK_FILTER_GLOBAL_CUSTOM_STICKY_MASK        6
#define VTSS_LEN_ANA_CL_DETECTION_MASK_FILTER_GLOBAL_CUSTOM_STICKY_MASK        1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_MAC_CTRL_STICKY_MASK                    5
#define VTSS_LEN_ANA_CL_DETECTION_MASK_MAC_CTRL_STICKY_MASK                    1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_FILTER_L2_BC_STICKY_MASK                4
#define VTSS_LEN_ANA_CL_DETECTION_MASK_FILTER_L2_BC_STICKY_MASK                1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_FILTER_ETYPE_OR_MAC_STICKY_MASK         3
#define VTSS_LEN_ANA_CL_DETECTION_MASK_FILTER_ETYPE_OR_MAC_STICKY_MASK         1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_BAD_MACS_STICKY_MASK                    2
#define VTSS_LEN_ANA_CL_DETECTION_MASK_BAD_MACS_STICKY_MASK                    1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_SMAC_MC_STICKY_MASK                     1
#define VTSS_LEN_ANA_CL_DETECTION_MASK_SMAC_MC_STICKY_MASK                     1
#define VTSS_OFF_ANA_CL_DETECTION_MASK_FILTER_LOCAL_CUSTOM_STICKY_MASK         0
#define VTSS_LEN_ANA_CL_DETECTION_MASK_FILTER_LOCAL_CUSTOM_STICKY_MASK         1

/*********************************************************************** 
 * Target ANA_AC
 * Access Control sub block of the Analyzer
 ***********************************************************************/
#define VTSS_ADDR_ANA_AC_POL_ALL_CFG                                      0x0000
#define VTSS_OFF_ANA_AC_POL_ALL_CFG_FORCE_CLOSE                                2
#define VTSS_LEN_ANA_AC_POL_ALL_CFG_FORCE_CLOSE                                1
#define VTSS_OFF_ANA_AC_POL_ALL_CFG_FORCE_OPEN                                 1
#define VTSS_LEN_ANA_AC_POL_ALL_CFG_FORCE_OPEN                                 1
#define VTSS_OFF_ANA_AC_POL_ALL_CFG_FORCE_INIT                                 0
#define VTSS_LEN_ANA_AC_POL_ALL_CFG_FORCE_INIT                                 1

#define VTSS_ADDR_ANA_AC_POL_PORT_GAP                                     0x0001
#define VTSS_ADDX_ANA_AC_POL_PORT_GAP(x)                                  (VTSS_ADDR_ANA_AC_POL_PORT_GAP + (x))
#define VTSS_OFF_ANA_AC_POL_PORT_GAP_GAP_VALUE                                 0
#define VTSS_LEN_ANA_AC_POL_PORT_GAP_GAP_VALUE                                 8

#define VTSS_ADDR_ANA_AC_POL_PORT_CFG                                     0x0019
#define VTSS_ADDX_ANA_AC_POL_PORT_CFG(x)                                  (VTSS_ADDR_ANA_AC_POL_PORT_CFG + (x))
#define VTSS_OFF_ANA_AC_POL_PORT_CFG_PORT_CFG                                  1
#define VTSS_LEN_ANA_AC_POL_PORT_CFG_PORT_CFG                                  3

#define VTSS_ADDR_ANA_AC_POL_PORT_RATE_CFG                                0x0049
#define VTSS_ADDX_ANA_AC_POL_PORT_RATE_CFG(x)                             (VTSS_ADDR_ANA_AC_POL_PORT_RATE_CFG + (x))
#define VTSS_OFF_ANA_AC_POL_PORT_RATE_CFG_PORT_RATE                            0
#define VTSS_LEN_ANA_AC_POL_PORT_RATE_CFG_PORT_RATE                           23

#define VTSS_ADDR_ANA_AC_POL_PORT_THRES_CFG_0                             0x0079
#define VTSS_ADDX_ANA_AC_POL_PORT_THRES_CFG_0(x)                          (VTSS_ADDR_ANA_AC_POL_PORT_THRES_CFG_0 + (x))
#define VTSS_OFF_ANA_AC_POL_PORT_THRES_CFG_0_PORT_THRES0                       0
#define VTSS_LEN_ANA_AC_POL_PORT_THRES_CFG_0_PORT_THRES0                       8

#define VTSS_ADDR_ANA_AC_POL_PRIO_CFG                                     0x0101
#define VTSS_ADDX_ANA_AC_POL_PRIO_CFG(x)                                  (VTSS_ADDR_ANA_AC_POL_PRIO_CFG + (x))
#define VTSS_OFF_ANA_AC_POL_PRIO_CFG_PRIO_POL_CFG                              1
#define VTSS_LEN_ANA_AC_POL_PRIO_CFG_PRIO_POL_CFG                              3

#define VTSS_ADDR_ANA_AC_POL_PRIO_RATE_CFG                                0x01c1
#define VTSS_ADDX_ANA_AC_POL_PRIO_RATE_CFG(x)                             (VTSS_ADDR_ANA_AC_POL_PRIO_RATE_CFG + (x))
#define VTSS_OFF_ANA_AC_POL_PRIO_RATE_CFG_PRIO_RATE                            0
#define VTSS_LEN_ANA_AC_POL_PRIO_RATE_CFG_PRIO_RATE                           23

#define VTSS_ADDR_ANA_AC_POL_PRIO_THRES_CFG_0                             0x0281
#define VTSS_ADDX_ANA_AC_POL_PRIO_THRES_CFG_0(x)                          (VTSS_ADDR_ANA_AC_POL_PRIO_THRES_CFG_0 + (x))
#define VTSS_OFF_ANA_AC_POL_PRIO_THRES_CFG_0_PRIO_THRES0                       0
#define VTSS_LEN_ANA_AC_POL_PRIO_THRES_CFG_0_PRIO_THRES0                       8

#define VTSS_ADDR_ANA_AC_SOFT_STICKY                                      0x0501
#define VTSS_OFF_ANA_AC_SOFT_STICKY_LB_CFG_SOFT_STICKY                         8
#define VTSS_LEN_ANA_AC_SOFT_STICKY_LB_CFG_SOFT_STICKY                         1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PRIO_STAT_INFO_SOFT_STICKY                 7
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PRIO_STAT_INFO_SOFT_STICKY                 1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PORT_STAT_INFO_SOFT_STICKY                 6
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PORT_STAT_INFO_SOFT_STICKY                 1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PRIO_STAT_CNT_SOFT_STICKY                  5
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PRIO_STAT_CNT_SOFT_STICKY                  1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PORT_STAT_CNT_SOFT_STICKY                  4
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PORT_STAT_CNT_SOFT_STICKY                  1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PORT_LB_CNT_SOFT_STICKY                    3
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PORT_LB_CNT_SOFT_STICKY                    1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PORT_LB_CFG_SOFT_STICKY                    2
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PORT_LB_CFG_SOFT_STICKY                    1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PRIO_LB_CNT_SOFT_STICKY                    1
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PRIO_LB_CNT_SOFT_STICKY                    1
#define VTSS_OFF_ANA_AC_SOFT_STICKY_PRIO_LB_CFG_SOFT_STICKY                    0
#define VTSS_LEN_ANA_AC_SOFT_STICKY_PRIO_LB_CFG_SOFT_STICKY                    1

#define VTSS_ADDR_ANA_AC_SOFT_MASK                                        0x0502
#define VTSS_OFF_ANA_AC_SOFT_MASK_POL_CFG_SOFT_MASK                            8
#define VTSS_LEN_ANA_AC_SOFT_MASK_POL_CFG_SOFT_MASK                            1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PRIO_STAT_INFO_SOFT_MASK                     7
#define VTSS_LEN_ANA_AC_SOFT_MASK_PRIO_STAT_INFO_SOFT_MASK                     1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PORT_STAT_INFO_SOFT_MASK                     6
#define VTSS_LEN_ANA_AC_SOFT_MASK_PORT_STAT_INFO_SOFT_MASK                     1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PRIO_STAT_CNT_SOFT_MASK                      5
#define VTSS_LEN_ANA_AC_SOFT_MASK_PRIO_STAT_CNT_SOFT_MASK                      1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PORT_STAT_CNT_SOFT_MASK                      4
#define VTSS_LEN_ANA_AC_SOFT_MASK_PORT_STAT_CNT_SOFT_MASK                      1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PRIO_LB_CNT_SOFT_MASK                        3
#define VTSS_LEN_ANA_AC_SOFT_MASK_PRIO_LB_CNT_SOFT_MASK                        1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PRIO_LB_CFG_SOFT_MASK                        2
#define VTSS_LEN_ANA_AC_SOFT_MASK_PRIO_LB_CFG_SOFT_MASK                        1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PORT_LB_CNT_SOFT_MASK                        1
#define VTSS_LEN_ANA_AC_SOFT_MASK_PORT_LB_CNT_SOFT_MASK                        1
#define VTSS_OFF_ANA_AC_SOFT_MASK_PORT_LB_CFG_SOFT_MASK                        0
#define VTSS_LEN_ANA_AC_SOFT_MASK_PORT_LB_CFG_SOFT_MASK                        1

#define VTSS_ADDR_ANA_AC_SPARE_RW_REG                                     0x0503

#define VTSS_ADDR_ANA_AC_SPARE_RO_REG                                     0x0504

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_0                    0x0505
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_1                    0x0506
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_1_GLOBAL_EVENT_MASK_1      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_1_GLOBAL_EVENT_MASK_1      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_2                    0x0507
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_2_GLOBAL_EVENT_MASK_2      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_2_GLOBAL_EVENT_MASK_2      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_3                    0x0508
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_3_GLOBAL_EVENT_MASK_3      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_3_GLOBAL_EVENT_MASK_3      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_4                    0x0509
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_4_GLOBAL_EVENT_MASK_4      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_4_GLOBAL_EVENT_MASK_4      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_5                    0x050a
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_5_GLOBAL_EVENT_MASK_5      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_5_GLOBAL_EVENT_MASK_5      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_6                    0x050b
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_6_GLOBAL_EVENT_MASK_6      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_6_GLOBAL_EVENT_MASK_6      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_7                    0x050c
#define VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_7_GLOBAL_EVENT_MASK_7      0
#define VTSS_LEN_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_7_GLOBAL_EVENT_MASK_7      1

#define VTSS_ADDR_ANA_AC_PORT_STAT_RESET                                  0x050d
#define VTSS_OFF_ANA_AC_PORT_STAT_RESET_RESET                                  0
#define VTSS_LEN_ANA_AC_PORT_STAT_RESET_RESET                                  1

#define VTSS_ADDR_ANA_AC_PORT_STAT_CNT_CFG_PORT                           0x050e
#define VTSS_WIDTH_ANA_AC_PORT_STAT_CNT_CFG_PORT                          0x0020
#define VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x)                        (VTSS_ADDR_ANA_AC_PORT_STAT_CNT_CFG_PORT + (x)*VTSS_WIDTH_ANA_AC_PORT_STAT_CNT_CFG_PORT)

#define VTSS_ADDR_ANA_AC_PORT_STAT_EVENTS_STICKY                          0x0000
#define VTSS_ADDX_ANA_AC_PORT_STAT_EVENTS_STICKY(x)                       (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_EVENTS_STICKY)
#define VTSS_OFF_ANA_AC_PORT_STAT_EVENTS_STICKY_STICKY_BITS                    0
#define VTSS_LEN_ANA_AC_PORT_STAT_EVENTS_STICKY_STICKY_BITS                    8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_0                                  0x0001
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_0(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_0)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_0_CFG_CNT_BYTE_0                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_0_CFG_CNT_BYTE_0                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_0                              0x0002
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_0(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_0)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_0                              0x0003
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_0(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_0)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_0_MSB_CNT_0                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_0_MSB_CNT_0                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_1                                  0x0004
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_1(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_1)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_1_CFG_CNT_BYTE_1                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_1_CFG_CNT_BYTE_1                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_1                              0x0005
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_1(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_1)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_1                              0x0006
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_1(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_1)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_1_MSB_CNT_1                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_1_MSB_CNT_1                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_2                                  0x0007
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_2(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_2)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_2_CFG_CNT_BYTE_2                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_2_CFG_CNT_BYTE_2                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_2                              0x0008
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_2(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_2)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_2                              0x0009
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_2(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_2)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_2_MSB_CNT_2                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_2_MSB_CNT_2                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_3                                  0x000a
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_3(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_3)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_3_CFG_CNT_BYTE_3                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_3_CFG_CNT_BYTE_3                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_3                              0x000b
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_3(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_3)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_3                              0x000c
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_3(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_3)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_3_MSB_CNT_3                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_3_MSB_CNT_3                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_4                                  0x000d
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_4(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_4)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_4_CFG_CNT_BYTE_4                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_4_CFG_CNT_BYTE_4                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_4                              0x000e
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_4(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_4)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_4                              0x000f
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_4(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_4)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_4_MSB_CNT_4                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_4_MSB_CNT_4                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_5                                  0x0010
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_5(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_5)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_5_CFG_CNT_BYTE_5                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_5_CFG_CNT_BYTE_5                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_5                              0x0011
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_5(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_5)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_5                              0x0012
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_5(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_5)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_5_MSB_CNT_5                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_5_MSB_CNT_5                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_6                                  0x0013
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_6(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_6)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_6_CFG_CNT_BYTE_6                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_6_CFG_CNT_BYTE_6                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_6                              0x0014
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_6(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_6)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_6                              0x0015
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_6(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_6)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_6_MSB_CNT_6                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_6_MSB_CNT_6                          8

#define VTSS_ADDR_ANA_AC_PORT_STAT_CFG_7                                  0x0016
#define VTSS_ADDX_ANA_AC_PORT_STAT_CFG_7(x)                               (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_CFG_7)
#define VTSS_OFF_ANA_AC_PORT_STAT_CFG_7_CFG_CNT_BYTE_7                         0
#define VTSS_LEN_ANA_AC_PORT_STAT_CFG_7_CFG_CNT_BYTE_7                         1

#define VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_7                              0x0017
#define VTSS_ADDX_ANA_AC_PORT_STAT_LSB_CNT_7(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_LSB_CNT_7)

#define VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_7                              0x0018
#define VTSS_ADDX_ANA_AC_PORT_STAT_MSB_CNT_7(x)                           (VTSS_ADDX_ANA_AC_PORT_STAT_CNT_CFG_PORT(x) + VTSS_ADDR_ANA_AC_PORT_STAT_MSB_CNT_7)
#define VTSS_OFF_ANA_AC_PORT_STAT_MSB_CNT_7_MSB_CNT_7                          0
#define VTSS_LEN_ANA_AC_PORT_STAT_MSB_CNT_7_MSB_CNT_7                          8

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_0                   0x080e
#define VTSS_OFF_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0      0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0      1

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_1                   0x080f
#define VTSS_OFF_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_1_GLOBAL_EVENT_MASK_1      0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_1_GLOBAL_EVENT_MASK_1      1

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_RESET                                 0x0810
#define VTSS_OFF_ANA_AC_QUEUE_STAT_RESET_RESET                                 0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_RESET_RESET                                 1

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE                         0x0811
#define VTSS_WIDTH_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE                        0x0008
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x)                      (VTSS_ADDR_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE + (x)*VTSS_WIDTH_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE)

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_EVENTS_STICKY                         0x0000
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_EVENTS_STICKY(x)                      (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_EVENTS_STICKY)
#define VTSS_OFF_ANA_AC_QUEUE_STAT_EVENTS_STICKY_STICKY_BITS                   0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_EVENTS_STICKY_STICKY_BITS                   2

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_CFG_0                                 0x0001
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_CFG_0(x)                              (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_CFG_0)
#define VTSS_OFF_ANA_AC_QUEUE_STAT_CFG_0_CFG_CNT_BYTE_0                        0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_CFG_0_CFG_CNT_BYTE_0                        1

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_LSB_CNT_0                             0x0002
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_LSB_CNT_0(x)                          (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_LSB_CNT_0)

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_MSB_CNT_0                             0x0003
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_MSB_CNT_0(x)                          (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_MSB_CNT_0)
#define VTSS_OFF_ANA_AC_QUEUE_STAT_MSB_CNT_0_MSB_CNT_0                         0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_MSB_CNT_0_MSB_CNT_0                         8

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_CFG_1                                 0x0004
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_CFG_1(x)                              (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_CFG_1)
#define VTSS_OFF_ANA_AC_QUEUE_STAT_CFG_1_CFG_CNT_BYTE_1                        0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_CFG_1_CFG_CNT_BYTE_1                        1

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_LSB_CNT_1                             0x0005
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_LSB_CNT_1(x)                          (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_LSB_CNT_1)

#define VTSS_ADDR_ANA_AC_QUEUE_STAT_MSB_CNT_1                             0x0006
#define VTSS_ADDX_ANA_AC_QUEUE_STAT_MSB_CNT_1(x)                          (VTSS_ADDX_ANA_AC_QUEUE_STAT_CNT_CFG_QUEUE(x) + VTSS_ADDR_ANA_AC_QUEUE_STAT_MSB_CNT_1)
#define VTSS_OFF_ANA_AC_QUEUE_STAT_MSB_CNT_1_MSB_CNT_1                         0
#define VTSS_LEN_ANA_AC_QUEUE_STAT_MSB_CNT_1_MSB_CNT_1                         8

/*********************************************************************** 
 * Target SCH
 * Scheduler and Shaper 
 ***********************************************************************/
#define VTSS_ADDR_SCH_SCH_SPR_ENABLE                                      0x0000
#define VTSS_OFF_SCH_SCH_SPR_ENABLE_SCH_SPR_ENABLE                             0
#define VTSS_LEN_SCH_SCH_SPR_ENABLE_SCH_SPR_ENABLE                             1

#define VTSS_ADDR_SCH_HOST_ENABLE                                         0x0001
#define VTSS_OFF_SCH_HOST_ENABLE_SPI4_HOST_EN                                  2
#define VTSS_LEN_SCH_HOST_ENABLE_SPI4_HOST_EN                                  1
#define VTSS_OFF_SCH_HOST_ENABLE_XAUI0_HOST_EN                                 1
#define VTSS_LEN_SCH_HOST_ENABLE_XAUI0_HOST_EN                                 1
#define VTSS_OFF_SCH_HOST_ENABLE_XAUI1_HOST_EN                                 0
#define VTSS_LEN_SCH_HOST_ENABLE_XAUI1_HOST_EN                                 1

#define VTSS_ADDR_SCH_LINE_PORT_ENABLE                                    0x0002
#define VTSS_OFF_SCH_LINE_PORT_ENABLE_LINE_PORT_ENABLE                         0
#define VTSS_LEN_SCH_LINE_PORT_ENABLE_LINE_PORT_ENABLE                        24

#define VTSS_ADDR_SCH_RATE_CONVERSION                                     0x0003
#define VTSS_ADDX_SCH_RATE_CONVERSION(x)                                  (VTSS_ADDR_SCH_RATE_CONVERSION + (x))
#define VTSS_OFF_SCH_RATE_CONVERSION_RATE_CONVERSION                           0
#define VTSS_LEN_SCH_RATE_CONVERSION_RATE_CONVERSION                           9

#define VTSS_ADDR_SCH_FLOW_CONTROL_ENABLE                                 0x001e
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_EN                           5
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_EN                           1
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_EN                           4
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_EN                           1
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_SELECT                       3
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_SELECT                       1
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_SELECT                       2
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_SELECT                       1
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_SPI4_STATUS_FC_EN                     1
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_SPI4_STATUS_FC_EN                     1
#define VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_SPI4_PS_BM_STATUS_FC_EN               0
#define VTSS_LEN_SCH_FLOW_CONTROL_ENABLE_SPI4_PS_BM_STATUS_FC_EN               1

#define VTSS_ADDR_SCH_SPR_PORT_ENABLE                                     0x001f
#define VTSS_OFF_SCH_SPR_PORT_ENABLE_SPR_PORT_ENABLE                           0
#define VTSS_LEN_SCH_SPR_PORT_ENABLE_SPR_PORT_ENABLE                          27

#define VTSS_ADDR_SCH_SPR_INIT                                            0x0020
#define VTSS_OFF_SCH_SPR_INIT_SPR_INIT                                         0
#define VTSS_LEN_SCH_SPR_INIT_SPR_INIT                                         1

#define VTSS_ADDR_SCH_SPR_FORCE_OPEN                                      0x0021
#define VTSS_OFF_SCH_SPR_FORCE_OPEN_SPR_FORCE_OPEN                             0
#define VTSS_LEN_SCH_SPR_FORCE_OPEN_SPR_FORCE_OPEN                             1

#define VTSS_ADDR_SCH_SPR_FORCE_CLOSED                                    0x0022
#define VTSS_OFF_SCH_SPR_FORCE_CLOSED_SPR_FORCE_CLOSED                         0
#define VTSS_LEN_SCH_SPR_FORCE_CLOSED_SPR_FORCE_CLOSED                         1

#define VTSS_ADDR_SCH_SPR_MEMORY_ERROR                                    0x0023
#define VTSS_OFF_SCH_SPR_MEMORY_ERROR_LB_MEM_ERROR                             1
#define VTSS_LEN_SCH_SPR_MEMORY_ERROR_LB_MEM_ERROR                             1
#define VTSS_OFF_SCH_SPR_MEMORY_ERROR_CONFIG_MEM_ERROR                         0
#define VTSS_LEN_SCH_SPR_MEMORY_ERROR_CONFIG_MEM_ERROR                         1

#define VTSS_ADDR_SCH_INTERRUPT_MASK                                      0x0024
#define VTSS_OFF_SCH_INTERRUPT_MASK_LB_MEM_ERROR_MASK                          1
#define VTSS_LEN_SCH_INTERRUPT_MASK_LB_MEM_ERROR_MASK                          1
#define VTSS_OFF_SCH_INTERRUPT_MASK_CONFIG_MEM_ERROR_MASK                      0
#define VTSS_LEN_SCH_INTERRUPT_MASK_CONFIG_MEM_ERROR_MASK                      1

#define VTSS_ADDR_SCH_SHAPER_THRESHOLD                                    0x0100
#define VTSS_ADDX_SCH_SHAPER_THRESHOLD(x)                                 (VTSS_ADDR_SCH_SHAPER_THRESHOLD + (x))
#define VTSS_OFF_SCH_SHAPER_THRESHOLD_SHAPER_THRESHOLD                         0
#define VTSS_LEN_SCH_SHAPER_THRESHOLD_SHAPER_THRESHOLD                         8

#define VTSS_ADDR_SCH_SHAPER_RATE                                         0x0200
#define VTSS_ADDX_SCH_SHAPER_RATE(x)                                      (VTSS_ADDR_SCH_SHAPER_RATE + (x))
#define VTSS_OFF_SCH_SHAPER_RATE_SHAPER_RATE                                   0
#define VTSS_LEN_SCH_SHAPER_RATE_SHAPER_RATE                                  23

#define VTSS_ADDR_SCH_SPI4_PS_WEIGHT                                      0x0300
#define VTSS_ADDX_SCH_SPI4_PS_WEIGHT(x)                                   (VTSS_ADDR_SCH_SPI4_PS_WEIGHT + (x))
#define VTSS_OFF_SCH_SPI4_PS_WEIGHT_SPI4_PS_WEIGHT                             0
#define VTSS_LEN_SCH_SPI4_PS_WEIGHT_SPI4_PS_WEIGHT                            21

#define VTSS_ADDR_SCH_XAUI0_PS_WEIGHT                                     0x0330
#define VTSS_ADDX_SCH_XAUI0_PS_WEIGHT(x)                                  (VTSS_ADDR_SCH_XAUI0_PS_WEIGHT + (x))
#define VTSS_OFF_SCH_XAUI0_PS_WEIGHT_XAUI0_PS_WEIGHT                           0
#define VTSS_LEN_SCH_XAUI0_PS_WEIGHT_XAUI0_PS_WEIGHT                          21

#define VTSS_ADDR_SCH_XAUI1_PS_WEIGHT                                     0x0348
#define VTSS_ADDX_SCH_XAUI1_PS_WEIGHT(x)                                  (VTSS_ADDR_SCH_XAUI1_PS_WEIGHT + (x))
#define VTSS_OFF_SCH_XAUI1_PS_WEIGHT_XAUI1_PS_WEIGHT                           0
#define VTSS_LEN_SCH_XAUI1_PS_WEIGHT_XAUI1_PS_WEIGHT                          21

#define VTSS_ADDR_SCH_QS0_WEIGHT                                          0x0360
#define VTSS_ADDX_SCH_QS0_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS0_WEIGHT + (x))
#define VTSS_OFF_SCH_QS0_WEIGHT_QS0_WEIGHT                                     0
#define VTSS_LEN_SCH_QS0_WEIGHT_QS0_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS1_WEIGHT                                          0x0366
#define VTSS_ADDX_SCH_QS1_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS1_WEIGHT + (x))
#define VTSS_OFF_SCH_QS1_WEIGHT_QS1_WEIGHT                                     0
#define VTSS_LEN_SCH_QS1_WEIGHT_QS1_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS2_WEIGHT                                          0x036c
#define VTSS_ADDX_SCH_QS2_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS2_WEIGHT + (x))
#define VTSS_OFF_SCH_QS2_WEIGHT_QS2_WEIGHT                                     0
#define VTSS_LEN_SCH_QS2_WEIGHT_QS2_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS3_WEIGHT                                          0x0372
#define VTSS_ADDX_SCH_QS3_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS3_WEIGHT + (x))
#define VTSS_OFF_SCH_QS3_WEIGHT_QS3_WEIGHT                                     0
#define VTSS_LEN_SCH_QS3_WEIGHT_QS3_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS4_WEIGHT                                          0x0378
#define VTSS_ADDX_SCH_QS4_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS4_WEIGHT + (x))
#define VTSS_OFF_SCH_QS4_WEIGHT_QS4_WEIGHT                                     0
#define VTSS_LEN_SCH_QS4_WEIGHT_QS4_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS5_WEIGHT                                          0x037e
#define VTSS_ADDX_SCH_QS5_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS5_WEIGHT + (x))
#define VTSS_OFF_SCH_QS5_WEIGHT_QS5_WEIGHT                                     0
#define VTSS_LEN_SCH_QS5_WEIGHT_QS5_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS6_WEIGHT                                          0x0384
#define VTSS_ADDX_SCH_QS6_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS6_WEIGHT + (x))
#define VTSS_OFF_SCH_QS6_WEIGHT_QS6_WEIGHT                                     0
#define VTSS_LEN_SCH_QS6_WEIGHT_QS6_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS7_WEIGHT                                          0x038a
#define VTSS_ADDX_SCH_QS7_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS7_WEIGHT + (x))
#define VTSS_OFF_SCH_QS7_WEIGHT_QS7_WEIGHT                                     0
#define VTSS_LEN_SCH_QS7_WEIGHT_QS7_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS8_WEIGHT                                          0x0390
#define VTSS_ADDX_SCH_QS8_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS8_WEIGHT + (x))
#define VTSS_OFF_SCH_QS8_WEIGHT_QS8_WEIGHT                                     0
#define VTSS_LEN_SCH_QS8_WEIGHT_QS8_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS9_WEIGHT                                          0x0396
#define VTSS_ADDX_SCH_QS9_WEIGHT(x)                                       (VTSS_ADDR_SCH_QS9_WEIGHT + (x))
#define VTSS_OFF_SCH_QS9_WEIGHT_QS9_WEIGHT                                     0
#define VTSS_LEN_SCH_QS9_WEIGHT_QS9_WEIGHT                                    21

#define VTSS_ADDR_SCH_QS10_WEIGHT                                         0x039c
#define VTSS_ADDX_SCH_QS10_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS10_WEIGHT + (x))
#define VTSS_OFF_SCH_QS10_WEIGHT_QS10_WEIGHT                                   0
#define VTSS_LEN_SCH_QS10_WEIGHT_QS10_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS11_WEIGHT                                         0x03a2
#define VTSS_ADDX_SCH_QS11_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS11_WEIGHT + (x))
#define VTSS_OFF_SCH_QS11_WEIGHT_QS11_WEIGHT                                   0
#define VTSS_LEN_SCH_QS11_WEIGHT_QS11_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS12_WEIGHT                                         0x03a8
#define VTSS_ADDX_SCH_QS12_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS12_WEIGHT + (x))
#define VTSS_OFF_SCH_QS12_WEIGHT_QS12_WEIGHT                                   0
#define VTSS_LEN_SCH_QS12_WEIGHT_QS12_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS13_WEIGHT                                         0x03ae
#define VTSS_ADDX_SCH_QS13_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS13_WEIGHT + (x))
#define VTSS_OFF_SCH_QS13_WEIGHT_QS13_WEIGHT                                   0
#define VTSS_LEN_SCH_QS13_WEIGHT_QS13_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS14_WEIGHT                                         0x03b4
#define VTSS_ADDX_SCH_QS14_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS14_WEIGHT + (x))
#define VTSS_OFF_SCH_QS14_WEIGHT_QS14_WEIGHT                                   0
#define VTSS_LEN_SCH_QS14_WEIGHT_QS14_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS15_WEIGHT                                         0x03ba
#define VTSS_ADDX_SCH_QS15_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS15_WEIGHT + (x))
#define VTSS_OFF_SCH_QS15_WEIGHT_QS15_WEIGHT                                   0
#define VTSS_LEN_SCH_QS15_WEIGHT_QS15_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS16_WEIGHT                                         0x03c0
#define VTSS_ADDX_SCH_QS16_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS16_WEIGHT + (x))
#define VTSS_OFF_SCH_QS16_WEIGHT_QS16_WEIGHT                                   0
#define VTSS_LEN_SCH_QS16_WEIGHT_QS16_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS17_WEIGHT                                         0x03c6
#define VTSS_ADDX_SCH_QS17_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS17_WEIGHT + (x))
#define VTSS_OFF_SCH_QS17_WEIGHT_QS17_WEIGHT                                   0
#define VTSS_LEN_SCH_QS17_WEIGHT_QS17_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS18_WEIGHT                                         0x03cc
#define VTSS_ADDX_SCH_QS18_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS18_WEIGHT + (x))
#define VTSS_OFF_SCH_QS18_WEIGHT_QS18_WEIGHT                                   0
#define VTSS_LEN_SCH_QS18_WEIGHT_QS18_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS19_WEIGHT                                         0x03d2
#define VTSS_ADDX_SCH_QS19_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS19_WEIGHT + (x))
#define VTSS_OFF_SCH_QS19_WEIGHT_QS19_WEIGHT                                   0
#define VTSS_LEN_SCH_QS19_WEIGHT_QS19_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS20_WEIGHT                                         0x03d8
#define VTSS_ADDX_SCH_QS20_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS20_WEIGHT + (x))
#define VTSS_OFF_SCH_QS20_WEIGHT_QS20_WEIGHT                                   0
#define VTSS_LEN_SCH_QS20_WEIGHT_QS20_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS21_WEIGHT                                         0x03de
#define VTSS_ADDX_SCH_QS21_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS21_WEIGHT + (x))
#define VTSS_OFF_SCH_QS21_WEIGHT_QS21_WEIGHT                                   0
#define VTSS_LEN_SCH_QS21_WEIGHT_QS21_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS22_WEIGHT                                         0x03e4
#define VTSS_ADDX_SCH_QS22_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS22_WEIGHT + (x))
#define VTSS_OFF_SCH_QS22_WEIGHT_QS22_WEIGHT                                   0
#define VTSS_LEN_SCH_QS22_WEIGHT_QS22_WEIGHT                                  21

#define VTSS_ADDR_SCH_QS23_WEIGHT                                         0x03ea
#define VTSS_ADDX_SCH_QS23_WEIGHT(x)                                      (VTSS_ADDR_SCH_QS23_WEIGHT + (x))
#define VTSS_OFF_SCH_QS23_WEIGHT_QS23_WEIGHT                                   0
#define VTSS_LEN_SCH_QS23_WEIGHT_QS23_WEIGHT                                  21

#define VTSS_ADDR_SCH_SPI4_PS_DEFICIT                                     0x0400
#define VTSS_ADDX_SCH_SPI4_PS_DEFICIT(x)                                  (VTSS_ADDR_SCH_SPI4_PS_DEFICIT + (x))
#define VTSS_OFF_SCH_SPI4_PS_DEFICIT_SPI4_PS_DEFICIT                           0
#define VTSS_LEN_SCH_SPI4_PS_DEFICIT_SPI4_PS_DEFICIT                          22

#define VTSS_ADDR_SCH_XAUI0_PS_DEFICIT                                    0x0430
#define VTSS_ADDX_SCH_XAUI0_PS_DEFICIT(x)                                 (VTSS_ADDR_SCH_XAUI0_PS_DEFICIT + (x))
#define VTSS_OFF_SCH_XAUI0_PS_DEFICIT_XAUI0_PS_DEFICIT                         0
#define VTSS_LEN_SCH_XAUI0_PS_DEFICIT_XAUI0_PS_DEFICIT                        22

#define VTSS_ADDR_SCH_XAUI1_PS_DEFICIT                                    0x0448
#define VTSS_ADDX_SCH_XAUI1_PS_DEFICIT(x)                                 (VTSS_ADDR_SCH_XAUI1_PS_DEFICIT + (x))
#define VTSS_OFF_SCH_XAUI1_PS_DEFICIT_XAUI1_PS_DEFICIT                         0
#define VTSS_LEN_SCH_XAUI1_PS_DEFICIT_XAUI1_PS_DEFICIT                        22

#define VTSS_ADDR_SCH_QS0_DEFICIT                                         0x0460
#define VTSS_ADDX_SCH_QS0_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS0_DEFICIT + (x))
#define VTSS_OFF_SCH_QS0_DEFICIT_QS0_DEFICIT                                   0
#define VTSS_LEN_SCH_QS0_DEFICIT_QS0_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS1_DEFICIT                                         0x0466
#define VTSS_ADDX_SCH_QS1_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS1_DEFICIT + (x))
#define VTSS_OFF_SCH_QS1_DEFICIT_QS1_DEFICIT                                   0
#define VTSS_LEN_SCH_QS1_DEFICIT_QS1_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS2_DEFICIT                                         0x046c
#define VTSS_ADDX_SCH_QS2_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS2_DEFICIT + (x))
#define VTSS_OFF_SCH_QS2_DEFICIT_QS2_DEFICIT                                   0
#define VTSS_LEN_SCH_QS2_DEFICIT_QS2_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS3_DEFICIT                                         0x0472
#define VTSS_ADDX_SCH_QS3_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS3_DEFICIT + (x))
#define VTSS_OFF_SCH_QS3_DEFICIT_QS3_DEFICIT                                   0
#define VTSS_LEN_SCH_QS3_DEFICIT_QS3_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS4_DEFICIT                                         0x0478
#define VTSS_ADDX_SCH_QS4_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS4_DEFICIT + (x))
#define VTSS_OFF_SCH_QS4_DEFICIT_QS4_DEFICIT                                   0
#define VTSS_LEN_SCH_QS4_DEFICIT_QS4_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS5_DEFICIT                                         0x047e
#define VTSS_ADDX_SCH_QS5_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS5_DEFICIT + (x))
#define VTSS_OFF_SCH_QS5_DEFICIT_QS5_DEFICIT                                   0
#define VTSS_LEN_SCH_QS5_DEFICIT_QS5_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS6_DEFICIT                                         0x0484
#define VTSS_ADDX_SCH_QS6_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS6_DEFICIT + (x))
#define VTSS_OFF_SCH_QS6_DEFICIT_QS6_DEFICIT                                   0
#define VTSS_LEN_SCH_QS6_DEFICIT_QS6_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS7_DEFICIT                                         0x048a
#define VTSS_ADDX_SCH_QS7_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS7_DEFICIT + (x))
#define VTSS_OFF_SCH_QS7_DEFICIT_QS7_DEFICIT                                   0
#define VTSS_LEN_SCH_QS7_DEFICIT_QS7_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS8_DEFICIT                                         0x0490
#define VTSS_ADDX_SCH_QS8_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS8_DEFICIT + (x))
#define VTSS_OFF_SCH_QS8_DEFICIT_QS8_DEFICIT                                   0
#define VTSS_LEN_SCH_QS8_DEFICIT_QS8_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS9_DEFICIT                                         0x0496
#define VTSS_ADDX_SCH_QS9_DEFICIT(x)                                      (VTSS_ADDR_SCH_QS9_DEFICIT + (x))
#define VTSS_OFF_SCH_QS9_DEFICIT_QS9_DEFICIT                                   0
#define VTSS_LEN_SCH_QS9_DEFICIT_QS9_DEFICIT                                  22

#define VTSS_ADDR_SCH_QS10_DEFICIT                                        0x049c
#define VTSS_ADDX_SCH_QS10_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS10_DEFICIT + (x))
#define VTSS_OFF_SCH_QS10_DEFICIT_QS10_DEFICIT                                 0
#define VTSS_LEN_SCH_QS10_DEFICIT_QS10_DEFICIT                                22

#define VTSS_ADDR_SCH_QS11_DEFICIT                                        0x04a2
#define VTSS_ADDX_SCH_QS11_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS11_DEFICIT + (x))
#define VTSS_OFF_SCH_QS11_DEFICIT_QS11_DEFICIT                                 0
#define VTSS_LEN_SCH_QS11_DEFICIT_QS11_DEFICIT                                22

#define VTSS_ADDR_SCH_QS12_DEFICIT                                        0x04a8
#define VTSS_ADDX_SCH_QS12_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS12_DEFICIT + (x))
#define VTSS_OFF_SCH_QS12_DEFICIT_QS12_DEFICIT                                 0
#define VTSS_LEN_SCH_QS12_DEFICIT_QS12_DEFICIT                                22

#define VTSS_ADDR_SCH_QS13_DEFICIT                                        0x04ae
#define VTSS_ADDX_SCH_QS13_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS13_DEFICIT + (x))
#define VTSS_OFF_SCH_QS13_DEFICIT_QS13_DEFICIT                                 0
#define VTSS_LEN_SCH_QS13_DEFICIT_QS13_DEFICIT                                22

#define VTSS_ADDR_SCH_QS14_DEFICIT                                        0x04b4
#define VTSS_ADDX_SCH_QS14_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS14_DEFICIT + (x))
#define VTSS_OFF_SCH_QS14_DEFICIT_QS14_DEFICIT                                 0
#define VTSS_LEN_SCH_QS14_DEFICIT_QS14_DEFICIT                                22

#define VTSS_ADDR_SCH_QS15_DEFICIT                                        0x04ba
#define VTSS_ADDX_SCH_QS15_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS15_DEFICIT + (x))
#define VTSS_OFF_SCH_QS15_DEFICIT_QS15_DEFICIT                                 0
#define VTSS_LEN_SCH_QS15_DEFICIT_QS15_DEFICIT                                22

#define VTSS_ADDR_SCH_QS16_DEFICIT                                        0x04c0
#define VTSS_ADDX_SCH_QS16_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS16_DEFICIT + (x))
#define VTSS_OFF_SCH_QS16_DEFICIT_QS16_DEFICIT                                 0
#define VTSS_LEN_SCH_QS16_DEFICIT_QS16_DEFICIT                                22

#define VTSS_ADDR_SCH_QS17_DEFICIT                                        0x04c6
#define VTSS_ADDX_SCH_QS17_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS17_DEFICIT + (x))
#define VTSS_OFF_SCH_QS17_DEFICIT_QS17_DEFICIT                                 0
#define VTSS_LEN_SCH_QS17_DEFICIT_QS17_DEFICIT                                22

#define VTSS_ADDR_SCH_QS18_DEFICIT                                        0x04cc
#define VTSS_ADDX_SCH_QS18_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS18_DEFICIT + (x))
#define VTSS_OFF_SCH_QS18_DEFICIT_QS18_DEFICIT                                 0
#define VTSS_LEN_SCH_QS18_DEFICIT_QS18_DEFICIT                                22

#define VTSS_ADDR_SCH_QS19_DEFICIT                                        0x04d2
#define VTSS_ADDX_SCH_QS19_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS19_DEFICIT + (x))
#define VTSS_OFF_SCH_QS19_DEFICIT_QS19_DEFICIT                                 0
#define VTSS_LEN_SCH_QS19_DEFICIT_QS19_DEFICIT                                22

#define VTSS_ADDR_SCH_QS20_DEFICIT                                        0x04d8
#define VTSS_ADDX_SCH_QS20_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS20_DEFICIT + (x))
#define VTSS_OFF_SCH_QS20_DEFICIT_QS20_DEFICIT                                 0
#define VTSS_LEN_SCH_QS20_DEFICIT_QS20_DEFICIT                                22

#define VTSS_ADDR_SCH_QS21_DEFICIT                                        0x04de
#define VTSS_ADDX_SCH_QS21_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS21_DEFICIT + (x))
#define VTSS_OFF_SCH_QS21_DEFICIT_QS21_DEFICIT                                 0
#define VTSS_LEN_SCH_QS21_DEFICIT_QS21_DEFICIT                                22

#define VTSS_ADDR_SCH_QS22_DEFICIT                                        0x04e4
#define VTSS_ADDX_SCH_QS22_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS22_DEFICIT + (x))
#define VTSS_OFF_SCH_QS22_DEFICIT_QS22_DEFICIT                                 0
#define VTSS_LEN_SCH_QS22_DEFICIT_QS22_DEFICIT                                22

#define VTSS_ADDR_SCH_QS23_DEFICIT                                        0x04ea
#define VTSS_ADDX_SCH_QS23_DEFICIT(x)                                     (VTSS_ADDR_SCH_QS23_DEFICIT + (x))
#define VTSS_OFF_SCH_QS23_DEFICIT_QS23_DEFICIT                                 0
#define VTSS_LEN_SCH_QS23_DEFICIT_QS23_DEFICIT                                22

#define VTSS_ADDR_SCH_DWELL_COUNT_ENABLE                                  0x0500
#define VTSS_OFF_SCH_DWELL_COUNT_ENABLE_SPI4_PS_DWELL_COUNT_ENABLE             2
#define VTSS_LEN_SCH_DWELL_COUNT_ENABLE_SPI4_PS_DWELL_COUNT_ENABLE             1
#define VTSS_OFF_SCH_DWELL_COUNT_ENABLE_XAUI0_PS_DWELL_COUNT_ENABLE            1
#define VTSS_LEN_SCH_DWELL_COUNT_ENABLE_XAUI0_PS_DWELL_COUNT_ENABLE            1
#define VTSS_OFF_SCH_DWELL_COUNT_ENABLE_XAUI1_PS_DWELL_COUNT_ENABLE            0
#define VTSS_LEN_SCH_DWELL_COUNT_ENABLE_XAUI1_PS_DWELL_COUNT_ENABLE            1

#define VTSS_ADDR_SCH_SPI4_PS_DWELL_COUNT_PRESET                          0x0600
#define VTSS_ADDX_SCH_SPI4_PS_DWELL_COUNT_PRESET(x)                       (VTSS_ADDR_SCH_SPI4_PS_DWELL_COUNT_PRESET + (x))
#define VTSS_OFF_SCH_SPI4_PS_DWELL_COUNT_PRESET_SPI4_PS_DWELL_COUNT_PRESET      0
#define VTSS_LEN_SCH_SPI4_PS_DWELL_COUNT_PRESET_SPI4_PS_DWELL_COUNT_PRESET      7

#define VTSS_ADDR_SCH_XAUI0_PS_DWELL_COUNT_PRESET                         0x0630
#define VTSS_ADDX_SCH_XAUI0_PS_DWELL_COUNT_PRESET(x)                      (VTSS_ADDR_SCH_XAUI0_PS_DWELL_COUNT_PRESET + (x))
#define VTSS_OFF_SCH_XAUI0_PS_DWELL_COUNT_PRESET_XAUI0_PS_DWELL_COUNT_PRESET      0
#define VTSS_LEN_SCH_XAUI0_PS_DWELL_COUNT_PRESET_XAUI0_PS_DWELL_COUNT_PRESET      7

#define VTSS_ADDR_SCH_XAUI1_PS_DWELL_COUNT_PRESET                         0x0648
#define VTSS_ADDX_SCH_XAUI1_PS_DWELL_COUNT_PRESET(x)                      (VTSS_ADDR_SCH_XAUI1_PS_DWELL_COUNT_PRESET + (x))
#define VTSS_OFF_SCH_XAUI1_PS_DWELL_COUNT_PRESET_XAUI1_PS_DWELL_COUNT_PRESET      0
#define VTSS_LEN_SCH_XAUI1_PS_DWELL_COUNT_PRESET_XAUI1_PS_DWELL_COUNT_PRESET      7

#define VTSS_ADDR_SCH_XAUI0_PS_DWELL_COUNT_PRESET_EXT                     0x0660
#define VTSS_ADDX_SCH_XAUI0_PS_DWELL_COUNT_PRESET_EXT(x)                  (VTSS_ADDR_SCH_XAUI0_PS_DWELL_COUNT_PRESET_EXT + (x))
#define VTSS_OFF_SCH_XAUI0_PS_DWELL_COUNT_PRESET_EXT_XAUI0_PS_DWELL_COUNT_PRESET_EXT      0
#define VTSS_LEN_SCH_XAUI0_PS_DWELL_COUNT_PRESET_EXT_XAUI0_PS_DWELL_COUNT_PRESET_EXT      7

#define VTSS_ADDR_SCH_XAUI1_PS_DWELL_COUNT_PRESET_EXT                     0x0678
#define VTSS_ADDX_SCH_XAUI1_PS_DWELL_COUNT_PRESET_EXT(x)                  (VTSS_ADDR_SCH_XAUI1_PS_DWELL_COUNT_PRESET_EXT + (x))
#define VTSS_OFF_SCH_XAUI1_PS_DWELL_COUNT_PRESET_EXT_XAUI1_PS_DWELL_COUNT_PRESET_EXT      0
#define VTSS_LEN_SCH_XAUI1_PS_DWELL_COUNT_PRESET_EXT_XAUI1_PS_DWELL_COUNT_PRESET_EXT      7

#define VTSS_ADDR_SCH_SPI4_PS_DWELL_COUNT_OUTPUT                          0x0700
#define VTSS_OFF_SCH_SPI4_PS_DWELL_COUNT_OUTPUT_SPI4_PS_DWELL_COUNT_OUTPUT      0
#define VTSS_LEN_SCH_SPI4_PS_DWELL_COUNT_OUTPUT_SPI4_PS_DWELL_COUNT_OUTPUT      7

#define VTSS_ADDR_SCH_XAUI0_PS_DWELL_COUNT_OUTPUT                         0x0701
#define VTSS_OFF_SCH_XAUI0_PS_DWELL_COUNT_OUTPUT_XAUI0_PS_DWELL_COUNT_OUTPUT      0
#define VTSS_LEN_SCH_XAUI0_PS_DWELL_COUNT_OUTPUT_XAUI0_PS_DWELL_COUNT_OUTPUT      7

#define VTSS_ADDR_SCH_XAUI1_PS_DWELL_COUNT_OUTPUT                         0x0702
#define VTSS_OFF_SCH_XAUI1_PS_DWELL_COUNT_OUTPUT_XAUI1_PS_DWELL_COUNT_OUTPUT      0
#define VTSS_LEN_SCH_XAUI1_PS_DWELL_COUNT_OUTPUT_XAUI1_PS_DWELL_COUNT_OUTPUT      7

#define VTSS_ADDR_SCH_SPI4_PS_ELIGIBLE_FLAG_MS                            0x0800
#define VTSS_OFF_SCH_SPI4_PS_ELIGIBLE_FLAG_MS_SPI4_PS_ELIGIBLE_FLAG_MS         0
#define VTSS_LEN_SCH_SPI4_PS_ELIGIBLE_FLAG_MS_SPI4_PS_ELIGIBLE_FLAG_MS        16

#define VTSS_ADDR_SCH_SPI4_PS_ELIGIBLE_FLAG_LS                            0x0801

#define VTSS_ADDR_SCH_XAUI0_PS_ELIGIBLE_FLAG                              0x0802
#define VTSS_OFF_SCH_XAUI0_PS_ELIGIBLE_FLAG_XAUI0_PS_ELIGIBLE_FLAG             0
#define VTSS_LEN_SCH_XAUI0_PS_ELIGIBLE_FLAG_XAUI0_PS_ELIGIBLE_FLAG            24

#define VTSS_ADDR_SCH_XAUI1_PS_ELIGIBLE_FLAG                              0x0803
#define VTSS_OFF_SCH_XAUI1_PS_ELIGIBLE_FLAG_XAUI1_PS_ELIGIBLE_FLAG             0
#define VTSS_LEN_SCH_XAUI1_PS_ELIGIBLE_FLAG_XAUI1_PS_ELIGIBLE_FLAG            24

#define VTSS_ADDR_SCH_QS_ELIGIBLE_FLAG                                    0x0804
#define VTSS_ADDX_SCH_QS_ELIGIBLE_FLAG(x)                                 (VTSS_ADDR_SCH_QS_ELIGIBLE_FLAG + (x))
#define VTSS_OFF_SCH_QS_ELIGIBLE_FLAG_QS_ELIGIBLE_FLAG                         0
#define VTSS_LEN_SCH_QS_ELIGIBLE_FLAG_QS_ELIGIBLE_FLAG                         8

#define VTSS_ADDR_SCH_XAUI0_PS_ELIGIBLE_FLAG_EXT                          0x081c
#define VTSS_OFF_SCH_XAUI0_PS_ELIGIBLE_FLAG_EXT_XAUI0_PS_ELIGIBLE_FLAG_EXT      0
#define VTSS_LEN_SCH_XAUI0_PS_ELIGIBLE_FLAG_EXT_XAUI0_PS_ELIGIBLE_FLAG_EXT     24

#define VTSS_ADDR_SCH_XAUI1_PS_ELIGIBLE_FLAG_EXT                          0x081d
#define VTSS_OFF_SCH_XAUI1_PS_ELIGIBLE_FLAG_EXT_XAUI1_PS_ELIGIBLE_FLAG_EXT      0
#define VTSS_LEN_SCH_XAUI1_PS_ELIGIBLE_FLAG_EXT_XAUI1_PS_ELIGIBLE_FLAG_EXT     24

#define VTSS_ADDR_SCH_SPI4_PS_SCHED_STATE                                 0x0900
#define VTSS_OFF_SCH_SPI4_PS_SCHED_STATE_SPI4_PS_SCHED_STATE                   0
#define VTSS_LEN_SCH_SPI4_PS_SCHED_STATE_SPI4_PS_SCHED_STATE                   6

#define VTSS_ADDR_SCH_XAUI0_PS_SCHED_STATE                                0x0901
#define VTSS_OFF_SCH_XAUI0_PS_SCHED_STATE_XAUI0_PS_SCHED_STATE                 0
#define VTSS_LEN_SCH_XAUI0_PS_SCHED_STATE_XAUI0_PS_SCHED_STATE                 6

#define VTSS_ADDR_SCH_XAUI1_PS_SCHED_STATE                                0x0902
#define VTSS_OFF_SCH_XAUI1_PS_SCHED_STATE_XAUI1_PS_SCHED_STATE                 0
#define VTSS_LEN_SCH_XAUI1_PS_SCHED_STATE_XAUI1_PS_SCHED_STATE                 6

#define VTSS_ADDR_SCH_QS_SCHED_STATE                                      0x0903
#define VTSS_ADDX_SCH_QS_SCHED_STATE(x)                                   (VTSS_ADDR_SCH_QS_SCHED_STATE + (x))
#define VTSS_OFF_SCH_QS_SCHED_STATE_QS_SCHED_STATE                             0
#define VTSS_LEN_SCH_QS_SCHED_STATE_QS_SCHED_STATE                             6

#define VTSS_ADDR_SCH_SPARE_CONFIG                                        0x0a00

#define VTSS_ADDR_SCH_SPARE_STATUS                                        0x0a01

#define VTSS_ADDR_SCH_XAUI0_PS_WEIGHT_EXT                                 0x0b00
#define VTSS_ADDX_SCH_XAUI0_PS_WEIGHT_EXT(x)                              (VTSS_ADDR_SCH_XAUI0_PS_WEIGHT_EXT + (x))
#define VTSS_OFF_SCH_XAUI0_PS_WEIGHT_EXT_XAUI0_PS_WEIGHT_EXT                   0
#define VTSS_LEN_SCH_XAUI0_PS_WEIGHT_EXT_XAUI0_PS_WEIGHT_EXT                  21

#define VTSS_ADDR_SCH_XAUI1_PS_WEIGHT_EXT                                 0x0b18
#define VTSS_ADDX_SCH_XAUI1_PS_WEIGHT_EXT(x)                              (VTSS_ADDR_SCH_XAUI1_PS_WEIGHT_EXT + (x))
#define VTSS_OFF_SCH_XAUI1_PS_WEIGHT_EXT_XAUI1_PS_WEIGHT_EXT                   0
#define VTSS_LEN_SCH_XAUI1_PS_WEIGHT_EXT_XAUI1_PS_WEIGHT_EXT                  21

#define VTSS_ADDR_SCH_XAUI0_PS_DEFICIT_EXT                                0x0c00
#define VTSS_ADDX_SCH_XAUI0_PS_DEFICIT_EXT(x)                             (VTSS_ADDR_SCH_XAUI0_PS_DEFICIT_EXT + (x))
#define VTSS_OFF_SCH_XAUI0_PS_DEFICIT_EXT_XAUI0_PS_DEFICIT_EXT                 0
#define VTSS_LEN_SCH_XAUI0_PS_DEFICIT_EXT_XAUI0_PS_DEFICIT_EXT                22

#define VTSS_ADDR_SCH_XAUI1_PS_DEFICIT_EXT                                0x0c18
#define VTSS_ADDX_SCH_XAUI1_PS_DEFICIT_EXT(x)                             (VTSS_ADDR_SCH_XAUI1_PS_DEFICIT_EXT + (x))
#define VTSS_OFF_SCH_XAUI1_PS_DEFICIT_EXT_XAUI1_PS_DEFICIT_EXT                 0
#define VTSS_LEN_SCH_XAUI1_PS_DEFICIT_EXT_XAUI1_PS_DEFICIT_EXT                22

#define VTSS_ADDR_SCH_PP_SPR_ENABLE                                       0x0d00
#define VTSS_OFF_SCH_PP_SPR_ENABLE_PP_SPR_ENABLE                               0
#define VTSS_LEN_SCH_PP_SPR_ENABLE_PP_SPR_ENABLE                               1

#define VTSS_ADDR_SCH_PP_SPR_PORT_ENABLE_MS                               0x0d01
#define VTSS_OFF_SCH_PP_SPR_PORT_ENABLE_MS_PP_SPR_PORT_ENABLE_MS               0
#define VTSS_LEN_SCH_PP_SPR_PORT_ENABLE_MS_PP_SPR_PORT_ENABLE_MS              16

#define VTSS_ADDR_SCH_PP_SPR_PORT_ENABLE_LS                               0x0d02

#define VTSS_ADDR_SCH_PP_SPR_INIT                                         0x0d03
#define VTSS_OFF_SCH_PP_SPR_INIT_PP_SPR_INIT                                   0
#define VTSS_LEN_SCH_PP_SPR_INIT_PP_SPR_INIT                                   1

#define VTSS_ADDR_SCH_PP_SPR_FORCE_OPEN                                   0x0d04
#define VTSS_OFF_SCH_PP_SPR_FORCE_OPEN_PP_SPR_FORCE_OPEN                       0
#define VTSS_LEN_SCH_PP_SPR_FORCE_OPEN_PP_SPR_FORCE_OPEN                       1

#define VTSS_ADDR_SCH_PP_SPR_FORCE_CLOSED                                 0x0d05
#define VTSS_OFF_SCH_PP_SPR_FORCE_CLOSED_PP_SPR_FORCE_CLOSED                   0
#define VTSS_LEN_SCH_PP_SPR_FORCE_CLOSED_PP_SPR_FORCE_CLOSED                   1

#define VTSS_ADDR_SCH_PP_SPR_MEMORY_ERROR                                 0x0d06
#define VTSS_OFF_SCH_PP_SPR_MEMORY_ERROR_PP_LB_MEM_ERROR                       1
#define VTSS_LEN_SCH_PP_SPR_MEMORY_ERROR_PP_LB_MEM_ERROR                       1
#define VTSS_OFF_SCH_PP_SPR_MEMORY_ERROR_PP_CONFIG_MEM_ERROR                   0
#define VTSS_LEN_SCH_PP_SPR_MEMORY_ERROR_PP_CONFIG_MEM_ERROR                   1

#define VTSS_ADDR_SCH_PP_INTERRUPT_MASK                                   0x0d07
#define VTSS_OFF_SCH_PP_INTERRUPT_MASK_PP_LB_MEM_ERROR_MASK                    1
#define VTSS_LEN_SCH_PP_INTERRUPT_MASK_PP_LB_MEM_ERROR_MASK                    1
#define VTSS_OFF_SCH_PP_INTERRUPT_MASK_PP_CONFIG_MEM_ERROR_MASK                0
#define VTSS_LEN_SCH_PP_INTERRUPT_MASK_PP_CONFIG_MEM_ERROR_MASK                1

#define VTSS_ADDR_SCH_PP_SHAPER_THRESHOLD                                 0x0e00
#define VTSS_ADDX_SCH_PP_SHAPER_THRESHOLD(x)                              (VTSS_ADDR_SCH_PP_SHAPER_THRESHOLD + (x))
#define VTSS_OFF_SCH_PP_SHAPER_THRESHOLD_PP_SHAPER_THRESHOLD                   0
#define VTSS_LEN_SCH_PP_SHAPER_THRESHOLD_PP_SHAPER_THRESHOLD                   8

#define VTSS_ADDR_SCH_PP_SHAPER_RATE                                      0x0f00
#define VTSS_ADDX_SCH_PP_SHAPER_RATE(x)                                   (VTSS_ADDR_SCH_PP_SHAPER_RATE + (x))
#define VTSS_OFF_SCH_PP_SHAPER_RATE_PP_SHAPER_RATE                             0
#define VTSS_LEN_SCH_PP_SHAPER_RATE_PP_SHAPER_RATE                            23

/*********************************************************************** 
 * Target QSS
 * Queue Sub System Configuration and Status Registers
 ***********************************************************************/
#define VTSS_ADDR_QSS_QSS_INIT                                            0x0000
#define VTSS_OFF_QSS_QSS_INIT_QSS_INIT                                         0
#define VTSS_LEN_QSS_QSS_INIT_QSS_INIT                                         1

#define VTSS_ADDR_QSS_QSS_CTL                                             0x0001
#define VTSS_OFF_QSS_QSS_CTL_DISABLE_SHUTDOWN_ON_MERR                          3
#define VTSS_LEN_QSS_QSS_CTL_DISABLE_SHUTDOWN_ON_MERR                          1
#define VTSS_OFF_QSS_QSS_CTL_XAUI_PAUSE_FC_ENA                                 2
#define VTSS_LEN_QSS_QSS_CTL_XAUI_PAUSE_FC_ENA                                 1
#define VTSS_OFF_QSS_QSS_CTL_ARIV_FAILED_FRM_DSCRD_ENA                         0
#define VTSS_LEN_QSS_QSS_CTL_ARIV_FAILED_FRM_DSCRD_ENA                         1

#define VTSS_ADDR_QSS_QSS_STICKY                                          0x0002
#define VTSS_OFF_QSS_QSS_STICKY_ARIV_DISABLED_BRM_STICKY                      11
#define VTSS_LEN_QSS_QSS_STICKY_ARIV_DISABLED_BRM_STICKY                       1
#define VTSS_OFF_QSS_QSS_STICKY_RX_DISCARD_CLASS_1_STICKY                     10
#define VTSS_LEN_QSS_QSS_STICKY_RX_DISCARD_CLASS_1_STICKY                      1
#define VTSS_OFF_QSS_QSS_STICKY_RX_DISCARD_CLASS_0_STICKY                      9
#define VTSS_LEN_QSS_QSS_STICKY_RX_DISCARD_CLASS_0_STICKY                      1
#define VTSS_OFF_QSS_QSS_STICKY_TX_DISCARD_CLASS_1_STICKY                      8
#define VTSS_LEN_QSS_QSS_STICKY_TX_DISCARD_CLASS_1_STICKY                      1
#define VTSS_OFF_QSS_QSS_STICKY_TX_DISCARD_CLASS_0_STICKY                      7
#define VTSS_LEN_QSS_QSS_STICKY_TX_DISCARD_CLASS_0_STICKY                      1
#define VTSS_OFF_QSS_QSS_STICKY_FLIST_UFLW_STICKY                              6
#define VTSS_LEN_QSS_QSS_STICKY_FLIST_UFLW_STICKY                              1
#define VTSS_OFF_QSS_QSS_STICKY_FLIST_EMPTY_STICKY                             5
#define VTSS_LEN_QSS_QSS_STICKY_FLIST_EMPTY_STICKY                             1
#define VTSS_OFF_QSS_QSS_STICKY_SCH_REQ_ERR_STICKY                             4
#define VTSS_LEN_QSS_QSS_STICKY_SCH_REQ_ERR_STICKY                             1
#define VTSS_OFF_QSS_QSS_STICKY_FRM_TRUNC_STICKY                               3
#define VTSS_LEN_QSS_QSS_STICKY_FRM_TRUNC_STICKY                               1
#define VTSS_OFF_QSS_QSS_STICKY_ARIV_MAX_FRM_LEN_STICKY                        2
#define VTSS_LEN_QSS_QSS_STICKY_ARIV_MAX_FRM_LEN_STICKY                        1
#define VTSS_OFF_QSS_QSS_STICKY_ARIV_CELL_SEQ_ERR_STICKY                       1
#define VTSS_LEN_QSS_QSS_STICKY_ARIV_CELL_SEQ_ERR_STICKY                       1
#define VTSS_OFF_QSS_QSS_STICKY_APORT_DISABLED_STICKY                          0
#define VTSS_LEN_QSS_QSS_STICKY_APORT_DISABLED_STICKY                          1

#define VTSS_ADDR_QSS_FRM_UFLW_PORTS_0_31_STICKY                          0x0003

#define VTSS_ADDR_QSS_FRM_UFLW_PORTS_32_63_STICKY                         0x0004

#define VTSS_ADDR_QSS_FRM_UFLW_PORTS_64_95_STICKY                         0x0005

#define VTSS_ADDR_QSS_RAM_PTY_ERR_STICKY                                  0x0006
#define VTSS_OFF_QSS_RAM_PTY_ERR_STICKY_RAM_PTY_ERR_STICKY                     0
#define VTSS_LEN_QSS_RAM_PTY_ERR_STICKY_RAM_PTY_ERR_STICKY                    16

#define VTSS_ADDR_QSS_INTERRUPT_MASK                                      0x0007
#define VTSS_OFF_QSS_INTERRUPT_MASK_FRM_TRUNC_MASK                            21
#define VTSS_LEN_QSS_INTERRUPT_MASK_FRM_TRUNC_MASK                             1
#define VTSS_OFF_QSS_INTERRUPT_MASK_FLIST_UFLW_MASK                           20
#define VTSS_LEN_QSS_INTERRUPT_MASK_FLIST_UFLW_MASK                            1
#define VTSS_OFF_QSS_INTERRUPT_MASK_RX_DISCARD_CLASS_1_MASK                   19
#define VTSS_LEN_QSS_INTERRUPT_MASK_RX_DISCARD_CLASS_1_MASK                    1
#define VTSS_OFF_QSS_INTERRUPT_MASK_RX_DISCARD_CLASS_0_MASK                   18
#define VTSS_LEN_QSS_INTERRUPT_MASK_RX_DISCARD_CLASS_0_MASK                    1
#define VTSS_OFF_QSS_INTERRUPT_MASK_TX_DISCARD_CLASS_1_MASK                   17
#define VTSS_LEN_QSS_INTERRUPT_MASK_TX_DISCARD_CLASS_1_MASK                    1
#define VTSS_OFF_QSS_INTERRUPT_MASK_TX_DISCARD_CLASS_0_MASK                   16
#define VTSS_LEN_QSS_INTERRUPT_MASK_TX_DISCARD_CLASS_0_MASK                    1
#define VTSS_OFF_QSS_INTERRUPT_MASK_RAM_PTY_ERR_MASK                           0
#define VTSS_LEN_QSS_INTERRUPT_MASK_RAM_PTY_ERR_MASK                          16

#define VTSS_ADDR_QSS_Q_FLUSH                                             0x0008
#define VTSS_OFF_QSS_Q_FLUSH_Q_FLUSH_PRIO                                      8
#define VTSS_LEN_QSS_Q_FLUSH_Q_FLUSH_PRIO                                      3
#define VTSS_OFF_QSS_Q_FLUSH_Q_FLUSH_PORT                                      0
#define VTSS_LEN_QSS_Q_FLUSH_Q_FLUSH_PORT                                      7

#define VTSS_ADDR_QSS_Q_FLUSH_REQ                                         0x0009
#define VTSS_OFF_QSS_Q_FLUSH_REQ_Q_FLUSH_REQ                                   0
#define VTSS_LEN_QSS_Q_FLUSH_REQ_Q_FLUSH_REQ                                   1

#define VTSS_ADDR_QSS_EXT_TX_PAUSE_1GBE_ENA                               0x000a
#define VTSS_OFF_QSS_EXT_TX_PAUSE_1GBE_ENA_EXT_TX_PAUSE_1GBE_ENA               0
#define VTSS_LEN_QSS_EXT_TX_PAUSE_1GBE_ENA_EXT_TX_PAUSE_1GBE_ENA               1

#define VTSS_ADDR_QSS_EXT_TX_PAUSE_10GBE_ENA                              0x000b
#define VTSS_OFF_QSS_EXT_TX_PAUSE_10GBE_ENA_EXT_TX_PAUSE_10GBE_ENA             0
#define VTSS_LEN_QSS_EXT_TX_PAUSE_10GBE_ENA_EXT_TX_PAUSE_10GBE_ENA             1

#define VTSS_ADDR_QSS_APORT_ENA_CLASS_MAP                                 0x0100
#define VTSS_ADDX_QSS_APORT_ENA_CLASS_MAP(x)                              (VTSS_ADDR_QSS_APORT_ENA_CLASS_MAP + (x))
#define VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_ENA                             8
#define VTSS_LEN_QSS_APORT_ENA_CLASS_MAP_APORT_ENA                             1
#define VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_CLASS                           7
#define VTSS_LEN_QSS_APORT_ENA_CLASS_MAP_APORT_CLASS                           1
#define VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_MAP                             0
#define VTSS_LEN_QSS_APORT_ENA_CLASS_MAP_APORT_MAP                             7

#define VTSS_ADDR_QSS_APORT_DSCRPT_H                                      0x0160
#define VTSS_ADDX_QSS_APORT_DSCRPT_H(x)                                   (VTSS_ADDR_QSS_APORT_DSCRPT_H + (x))
#define VTSS_OFF_QSS_APORT_DSCRPT_H_APORT_DSCRPT_H                             0
#define VTSS_LEN_QSS_APORT_DSCRPT_H_APORT_DSCRPT_H                            14

#define VTSS_ADDR_QSS_APORT_DSCRPT_M                                      0x01c0
#define VTSS_ADDX_QSS_APORT_DSCRPT_M(x)                                   (VTSS_ADDR_QSS_APORT_DSCRPT_M + (x))

#define VTSS_ADDR_QSS_APORT_DSCRPT_L                                      0x0220
#define VTSS_ADDX_QSS_APORT_DSCRPT_L(x)                                   (VTSS_ADDR_QSS_APORT_DSCRPT_L + (x))

#define VTSS_ADDR_QSS_DPORT_DSCRPT                                        0x0280
#define VTSS_ADDX_QSS_DPORT_DSCRPT(x)                                     (VTSS_ADDR_QSS_DPORT_DSCRPT + (x))
#define VTSS_OFF_QSS_DPORT_DSCRPT_DPORT_DSCRPT                                 0
#define VTSS_LEN_QSS_DPORT_DSCRPT_DPORT_DSCRPT                                 6

#define VTSS_ADDR_QSS_RX_TOTAL_SIZE                                       0x0300
#define VTSS_OFF_QSS_RX_TOTAL_SIZE_RX_TOTAL_SIZE                               0
#define VTSS_LEN_QSS_RX_TOTAL_SIZE_RX_TOTAL_SIZE                              13

#define VTSS_ADDR_QSS_TX_TOTAL_SIZE                                       0x0301
#define VTSS_OFF_QSS_TX_TOTAL_SIZE_TX_TOTAL_SIZE                               0
#define VTSS_LEN_QSS_TX_TOTAL_SIZE_TX_TOTAL_SIZE                              13

#define VTSS_ADDR_QSS_FC_3LVL                                             0x0302
#define VTSS_OFF_QSS_FC_3LVL_DISABLED_PORT_FC                                  3
#define VTSS_LEN_QSS_FC_3LVL_DISABLED_PORT_FC                                  2
#define VTSS_OFF_QSS_FC_3LVL_SPI4_FC_3LVL                                      2
#define VTSS_LEN_QSS_FC_3LVL_SPI4_FC_3LVL                                      1
#define VTSS_OFF_QSS_FC_3LVL_XAUI1_FC_3LVL                                     1
#define VTSS_LEN_QSS_FC_3LVL_XAUI1_FC_3LVL                                     1
#define VTSS_OFF_QSS_FC_3LVL_XAUI0_FC_3LVL                                     0
#define VTSS_LEN_QSS_FC_3LVL_XAUI0_FC_3LVL                                     1

#define VTSS_ADDR_QSS_RX_SHCNT                                            0x0303
#define VTSS_OFF_QSS_RX_SHCNT_RX_SHCNT                                         0
#define VTSS_LEN_QSS_RX_SHCNT_RX_SHCNT                                        13

#define VTSS_ADDR_QSS_RX_SHCNT_WM                                         0x0304
#define VTSS_OFF_QSS_RX_SHCNT_WM_RX_SHCNT_WM                                   0
#define VTSS_LEN_QSS_RX_SHCNT_WM_RX_SHCNT_WM                                  13

#define VTSS_ADDR_QSS_TX_SHCNT                                            0x0305
#define VTSS_OFF_QSS_TX_SHCNT_TX_SHCNT                                         0
#define VTSS_LEN_QSS_TX_SHCNT_TX_SHCNT                                        13

#define VTSS_ADDR_QSS_TX_SHCNT_WM                                         0x0306
#define VTSS_OFF_QSS_TX_SHCNT_WM_TX_SHCNT_WM                                   0
#define VTSS_LEN_QSS_TX_SHCNT_WM_TX_SHCNT_WM                                  13

#define VTSS_ADDR_QSS_RX_GBL_ENA                                          0x0400
#define VTSS_ADDX_QSS_RX_GBL_ENA(x)                                       (VTSS_ADDR_QSS_RX_GBL_ENA + (x))
#define VTSS_OFF_QSS_RX_GBL_ENA_RX_GBL_ENA                                     0
#define VTSS_LEN_QSS_RX_GBL_ENA_RX_GBL_ENA                                     1

#define VTSS_ADDR_QSS_RX_GBL_SHMAX                                        0x0402
#define VTSS_ADDX_QSS_RX_GBL_SHMAX(x)                                     (VTSS_ADDR_QSS_RX_GBL_SHMAX + (x))
#define VTSS_OFF_QSS_RX_GBL_SHMAX_RX_GBL_SHMAX                                 0
#define VTSS_LEN_QSS_RX_GBL_SHMAX_RX_GBL_SHMAX                                13

#define VTSS_ADDR_QSS_RX_GBL_DROP_HLTH                                    0x0404
#define VTSS_ADDX_QSS_RX_GBL_DROP_HLTH(x)                                 (VTSS_ADDR_QSS_RX_GBL_DROP_HLTH + (x))
#define VTSS_OFF_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_HTH                         16
#define VTSS_LEN_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_HTH                         13
#define VTSS_OFF_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_LTH                          0
#define VTSS_LEN_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_LTH                         13

#define VTSS_ADDR_QSS_RX_GBL_FC_HLTH                                      0x0406
#define VTSS_ADDX_QSS_RX_GBL_FC_HLTH(x)                                   (VTSS_ADDR_QSS_RX_GBL_FC_HLTH + (x))
#define VTSS_OFF_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_HTH                             16
#define VTSS_LEN_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_HTH                             13
#define VTSS_OFF_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_LTH                              0
#define VTSS_LEN_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_LTH                             13

#define VTSS_ADDR_QSS_RX_CNT                                              0x0408
#define VTSS_ADDX_QSS_RX_CNT(x)                                           (VTSS_ADDR_QSS_RX_CNT + (x))
#define VTSS_OFF_QSS_RX_CNT_RX_CNT                                             0
#define VTSS_LEN_QSS_RX_CNT_RX_CNT                                            13

#define VTSS_ADDR_QSS_RX_CNT_WM                                           0x040a
#define VTSS_ADDX_QSS_RX_CNT_WM(x)                                        (VTSS_ADDR_QSS_RX_CNT_WM + (x))
#define VTSS_OFF_QSS_RX_CNT_WM_RX_CNT_WM                                       0
#define VTSS_LEN_QSS_RX_CNT_WM_RX_CNT_WM                                      13

#define VTSS_ADDR_QSS_RX_DROP_FC_STATE                                    0x040c
#define VTSS_ADDX_QSS_RX_DROP_FC_STATE(x)                                 (VTSS_ADDR_QSS_RX_DROP_FC_STATE + (x))
#define VTSS_OFF_QSS_RX_DROP_FC_STATE_RX_FC_STATE                              4
#define VTSS_LEN_QSS_RX_DROP_FC_STATE_RX_FC_STATE                              3
#define VTSS_OFF_QSS_RX_DROP_FC_STATE_RX_DROP_STATE                            0
#define VTSS_LEN_QSS_RX_DROP_FC_STATE_RX_DROP_STATE                            3

#define VTSS_ADDR_QSS_RX_DROP_FC_WM                                       0x040e
#define VTSS_ADDX_QSS_RX_DROP_FC_WM(x)                                    (VTSS_ADDR_QSS_RX_DROP_FC_WM + (x))
#define VTSS_OFF_QSS_RX_DROP_FC_WM_RX_FC_WM                                    2
#define VTSS_LEN_QSS_RX_DROP_FC_WM_RX_FC_WM                                    2
#define VTSS_OFF_QSS_RX_DROP_FC_WM_RX_DROP_WM                                  0
#define VTSS_LEN_QSS_RX_DROP_FC_WM_RX_DROP_WM                                  2

#define VTSS_ADDR_QSS_RX_DROP_CNT                                         0x0410
#define VTSS_ADDX_QSS_RX_DROP_CNT(x)                                      (VTSS_ADDR_QSS_RX_DROP_CNT + (x))

#define VTSS_ADDR_QSS_TX_GBL_ENA                                          0x0500
#define VTSS_ADDX_QSS_TX_GBL_ENA(x)                                       (VTSS_ADDR_QSS_TX_GBL_ENA + (x))
#define VTSS_OFF_QSS_TX_GBL_ENA_TX_GBL_ENA                                     0
#define VTSS_LEN_QSS_TX_GBL_ENA_TX_GBL_ENA                                     1

#define VTSS_ADDR_QSS_TX_GBL_SHMAX                                        0x0502
#define VTSS_ADDX_QSS_TX_GBL_SHMAX(x)                                     (VTSS_ADDR_QSS_TX_GBL_SHMAX + (x))
#define VTSS_OFF_QSS_TX_GBL_SHMAX_TX_GBL_SHMAX                                 0
#define VTSS_LEN_QSS_TX_GBL_SHMAX_TX_GBL_SHMAX                                13

#define VTSS_ADDR_QSS_TX_GBL_DROP_HLTH                                    0x0504
#define VTSS_ADDX_QSS_TX_GBL_DROP_HLTH(x)                                 (VTSS_ADDR_QSS_TX_GBL_DROP_HLTH + (x))
#define VTSS_OFF_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_HTH                         16
#define VTSS_LEN_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_HTH                         13
#define VTSS_OFF_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_LTH                          0
#define VTSS_LEN_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_LTH                         13

#define VTSS_ADDR_QSS_TX_GBL_FC_HLTH                                      0x0506
#define VTSS_ADDX_QSS_TX_GBL_FC_HLTH(x)                                   (VTSS_ADDR_QSS_TX_GBL_FC_HLTH + (x))
#define VTSS_OFF_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_HTH                             16
#define VTSS_LEN_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_HTH                             13
#define VTSS_OFF_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_LTH                              0
#define VTSS_LEN_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_LTH                             13

#define VTSS_ADDR_QSS_TX_CNT                                              0x0508
#define VTSS_ADDX_QSS_TX_CNT(x)                                           (VTSS_ADDR_QSS_TX_CNT + (x))
#define VTSS_OFF_QSS_TX_CNT_TX_CNT                                             0
#define VTSS_LEN_QSS_TX_CNT_TX_CNT                                            13

#define VTSS_ADDR_QSS_TX_CNT_WM                                           0x050a
#define VTSS_ADDX_QSS_TX_CNT_WM(x)                                        (VTSS_ADDR_QSS_TX_CNT_WM + (x))
#define VTSS_OFF_QSS_TX_CNT_WM_TX_CNT_WM                                       0
#define VTSS_LEN_QSS_TX_CNT_WM_TX_CNT_WM                                      13

#define VTSS_ADDR_QSS_TX_DROP_FC_STATE                                    0x050c
#define VTSS_ADDX_QSS_TX_DROP_FC_STATE(x)                                 (VTSS_ADDR_QSS_TX_DROP_FC_STATE + (x))
#define VTSS_OFF_QSS_TX_DROP_FC_STATE_TX_FC_STATE                              4
#define VTSS_LEN_QSS_TX_DROP_FC_STATE_TX_FC_STATE                              3
#define VTSS_OFF_QSS_TX_DROP_FC_STATE_TX_DROP_STATE                            0
#define VTSS_LEN_QSS_TX_DROP_FC_STATE_TX_DROP_STATE                            3

#define VTSS_ADDR_QSS_TX_DROP_FC_WM                                       0x050e
#define VTSS_ADDX_QSS_TX_DROP_FC_WM(x)                                    (VTSS_ADDR_QSS_TX_DROP_FC_WM + (x))
#define VTSS_OFF_QSS_TX_DROP_FC_WM_TX_FC_WM                                    2
#define VTSS_LEN_QSS_TX_DROP_FC_WM_TX_FC_WM                                    2
#define VTSS_OFF_QSS_TX_DROP_FC_WM_TX_DROP_WM                                  0
#define VTSS_LEN_QSS_TX_DROP_FC_WM_TX_DROP_WM                                  2

#define VTSS_ADDR_QSS_TX_DROP_CNT                                         0x0510
#define VTSS_ADDX_QSS_TX_DROP_CNT(x)                                      (VTSS_ADDR_QSS_TX_DROP_CNT + (x))

#define VTSS_ADDR_QSS_PORT_ENA                                            0x0600
#define VTSS_ADDX_QSS_PORT_ENA(x)                                         (VTSS_ADDR_QSS_PORT_ENA + (x))
#define VTSS_OFF_QSS_PORT_ENA_PORT_ENA                                         0
#define VTSS_LEN_QSS_PORT_ENA_PORT_ENA                                         1

#define VTSS_ADDR_QSS_PORT_RSVD_GFC_LTH                                   0x0660
#define VTSS_ADDX_QSS_PORT_RSVD_GFC_LTH(x)                                (VTSS_ADDR_QSS_PORT_RSVD_GFC_LTH + (x))
#define VTSS_OFF_QSS_PORT_RSVD_GFC_LTH_PORT_RSVD                              16
#define VTSS_LEN_QSS_PORT_RSVD_GFC_LTH_PORT_RSVD                              13
#define VTSS_OFF_QSS_PORT_RSVD_GFC_LTH_PORT_GFC_LTH                            0
#define VTSS_LEN_QSS_PORT_RSVD_GFC_LTH_PORT_GFC_LTH                           13

#define VTSS_ADDR_QSS_PORT_SHMAX                                          0x06c0
#define VTSS_ADDX_QSS_PORT_SHMAX(x)                                       (VTSS_ADDR_QSS_PORT_SHMAX + (x))
#define VTSS_OFF_QSS_PORT_SHMAX_PORT_SHMAX                                     0
#define VTSS_LEN_QSS_PORT_SHMAX_PORT_SHMAX                                    13

#define VTSS_ADDR_QSS_PORT_DROP_HLTH                                      0x0720
#define VTSS_ADDX_QSS_PORT_DROP_HLTH(x)                                   (VTSS_ADDR_QSS_PORT_DROP_HLTH + (x))
#define VTSS_OFF_QSS_PORT_DROP_HLTH_PORT_DROP_HTH                             16
#define VTSS_LEN_QSS_PORT_DROP_HLTH_PORT_DROP_HTH                             13
#define VTSS_OFF_QSS_PORT_DROP_HLTH_PORT_DROP_LTH                              0
#define VTSS_LEN_QSS_PORT_DROP_HLTH_PORT_DROP_LTH                             13

#define VTSS_ADDR_QSS_PORT_FC_HLTH                                        0x0780
#define VTSS_ADDX_QSS_PORT_FC_HLTH(x)                                     (VTSS_ADDR_QSS_PORT_FC_HLTH + (x))
#define VTSS_OFF_QSS_PORT_FC_HLTH_PORT_FC_HTH                                 16
#define VTSS_LEN_QSS_PORT_FC_HLTH_PORT_FC_HTH                                 13
#define VTSS_OFF_QSS_PORT_FC_HLTH_PORT_FC_LTH                                  0
#define VTSS_LEN_QSS_PORT_FC_HLTH_PORT_FC_LTH                                 13

#define VTSS_ADDR_QSS_PORT_PRE_ALLOC                                      0x07e0
#define VTSS_ADDX_QSS_PORT_PRE_ALLOC(x)                                   (VTSS_ADDR_QSS_PORT_PRE_ALLOC + (x))
#define VTSS_OFF_QSS_PORT_PRE_ALLOC_PORT_PRE_ALLOC                             0
#define VTSS_LEN_QSS_PORT_PRE_ALLOC_PORT_PRE_ALLOC                             7

#define VTSS_ADDR_QSS_PORT_Q_CT_TH                                        0x0840
#define VTSS_ADDX_QSS_PORT_Q_CT_TH(x)                                     (VTSS_ADDR_QSS_PORT_Q_CT_TH + (x))
#define VTSS_OFF_QSS_PORT_Q_CT_TH_PORT_Q_CT_TH                                 0
#define VTSS_LEN_QSS_PORT_Q_CT_TH_PORT_Q_CT_TH                                13

#define VTSS_ADDR_QSS_PORT_CNT                                            0x08a0
#define VTSS_ADDX_QSS_PORT_CNT(x)                                         (VTSS_ADDR_QSS_PORT_CNT + (x))
#define VTSS_OFF_QSS_PORT_CNT_PORT_CNT                                         0
#define VTSS_LEN_QSS_PORT_CNT_PORT_CNT                                        13

#define VTSS_ADDR_QSS_PORT_SHCNT                                          0x0900
#define VTSS_ADDX_QSS_PORT_SHCNT(x)                                       (VTSS_ADDR_QSS_PORT_SHCNT + (x))
#define VTSS_OFF_QSS_PORT_SHCNT_PORT_FR_SHCNT                                 24
#define VTSS_LEN_QSS_PORT_SHCNT_PORT_FR_SHCNT                                  7
#define VTSS_OFF_QSS_PORT_SHCNT_PORT_PA_SHCNT                                 16
#define VTSS_LEN_QSS_PORT_SHCNT_PORT_PA_SHCNT                                  7
#define VTSS_OFF_QSS_PORT_SHCNT_PORT_SHCNT                                     0
#define VTSS_LEN_QSS_PORT_SHCNT_PORT_SHCNT                                    13

#define VTSS_ADDR_QSS_PORT_DROP_FC_GFC_STATE                              0x0960
#define VTSS_ADDX_QSS_PORT_DROP_FC_GFC_STATE(x)                           (VTSS_ADDR_QSS_PORT_DROP_FC_GFC_STATE + (x))
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_STATE_PORT_GFC_STATE                     8
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_STATE_PORT_GFC_STATE                     3
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_STATE_PORT_FC_STATE                      4
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_STATE_PORT_FC_STATE                      3
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_STATE_PORT_DROP_STATE                    0
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_STATE_PORT_DROP_STATE                    3

#define VTSS_ADDR_QSS_PORT_DROP_FC_GFC_WM                                 0x09c0
#define VTSS_ADDX_QSS_PORT_DROP_FC_GFC_WM(x)                              (VTSS_ADDR_QSS_PORT_DROP_FC_GFC_WM + (x))
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_WM_PORT_GFC_WM                           8
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_WM_PORT_GFC_WM                           3
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_WM_PORT_FC_WM                            4
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_WM_PORT_FC_WM                            2
#define VTSS_OFF_QSS_PORT_DROP_FC_GFC_WM_PORT_DROP_WM                          0
#define VTSS_LEN_QSS_PORT_DROP_FC_GFC_WM_PORT_DROP_WM                          2

#define VTSS_ADDR_QSS_PORT_DROP_CNT                                       0x0a20
#define VTSS_ADDX_QSS_PORT_DROP_CNT(x)                                    (VTSS_ADDR_QSS_PORT_DROP_CNT + (x))

#define VTSS_ADDR_QSS_Q_ENA                                               0x0e00
#define VTSS_ADDX_QSS_Q_ENA(x)                                            (VTSS_ADDR_QSS_Q_ENA + (x))
#define VTSS_OFF_QSS_Q_ENA_Q_ENA                                               0
#define VTSS_LEN_QSS_Q_ENA_Q_ENA                                               1

#define VTSS_ADDR_QSS_Q_RSVD                                              0x0ef0
#define VTSS_ADDX_QSS_Q_RSVD(x)                                           (VTSS_ADDR_QSS_Q_RSVD + (x))
#define VTSS_OFF_QSS_Q_RSVD_Q_RSVD                                             0
#define VTSS_LEN_QSS_Q_RSVD_Q_RSVD                                            13

#define VTSS_ADDR_QSS_Q_MAX                                               0x0fe0
#define VTSS_ADDX_QSS_Q_MAX(x)                                            (VTSS_ADDR_QSS_Q_MAX + (x))
#define VTSS_OFF_QSS_Q_MAX_Q_MAX                                               0
#define VTSS_LEN_QSS_Q_MAX_Q_MAX                                              13

#define VTSS_ADDR_QSS_Q_DROP_HLTH                                         0x10d0
#define VTSS_ADDX_QSS_Q_DROP_HLTH(x)                                      (VTSS_ADDR_QSS_Q_DROP_HLTH + (x))
#define VTSS_OFF_QSS_Q_DROP_HLTH_Q_DROP_HTH                                   16
#define VTSS_LEN_QSS_Q_DROP_HLTH_Q_DROP_HTH                                   13
#define VTSS_OFF_QSS_Q_DROP_HLTH_Q_DROP_LTH                                    0
#define VTSS_LEN_QSS_Q_DROP_HLTH_Q_DROP_LTH                                   13

#define VTSS_ADDR_QSS_Q_CNT                                               0x11c0
#define VTSS_ADDX_QSS_Q_CNT(x)                                            (VTSS_ADDR_QSS_Q_CNT + (x))
#define VTSS_OFF_QSS_Q_CNT_Q_FR_SHCNT                                         16
#define VTSS_LEN_QSS_Q_CNT_Q_FR_SHCNT                                          7
#define VTSS_OFF_QSS_Q_CNT_Q_CNT                                               0
#define VTSS_LEN_QSS_Q_CNT_Q_CNT                                              13

#define VTSS_ADDR_QSS_Q_DROP_STATE                                        0x12b0
#define VTSS_ADDX_QSS_Q_DROP_STATE(x)                                     (VTSS_ADDR_QSS_Q_DROP_STATE + (x))
#define VTSS_OFF_QSS_Q_DROP_STATE_Q_DROP_STATE                                 0
#define VTSS_LEN_QSS_Q_DROP_STATE_Q_DROP_STATE                                 3

#define VTSS_ADDR_QSS_Q_DROP_WM                                           0x13a0
#define VTSS_ADDX_QSS_Q_DROP_WM(x)                                        (VTSS_ADDR_QSS_Q_DROP_WM + (x))
#define VTSS_OFF_QSS_Q_DROP_WM_Q_DROP_WM                                       0
#define VTSS_LEN_QSS_Q_DROP_WM_Q_DROP_WM                                       2

#define VTSS_ADDR_QSS_Q_DROP_CNT                                          0x1490
#define VTSS_ADDX_QSS_Q_DROP_CNT(x)                                       (VTSS_ADDR_QSS_Q_DROP_CNT + (x))

#define VTSS_ADDR_QSS_Q_RED_AVG_CNT                                       0x1580
#define VTSS_ADDX_QSS_Q_RED_AVG_CNT(x)                                    (VTSS_ADDR_QSS_Q_RED_AVG_CNT + (x))
#define VTSS_OFF_QSS_Q_RED_AVG_CNT_Q_RED_AVG_CNT                               0
#define VTSS_LEN_QSS_Q_RED_AVG_CNT_Q_RED_AVG_CNT                              31

#define VTSS_ADDR_QSS_Q_RED_LAST_CNT                                      0x1670
#define VTSS_ADDX_QSS_Q_RED_LAST_CNT(x)                                   (VTSS_ADDR_QSS_Q_RED_LAST_CNT + (x))
#define VTSS_OFF_QSS_Q_RED_LAST_CNT_Q_RED_LAST_CNT                             0
#define VTSS_LEN_QSS_Q_RED_LAST_CNT_Q_RED_LAST_CNT                            10

#define VTSS_ADDR_QSS_Q_RED_MISC_CFG                                      0x1760
#define VTSS_ADDX_QSS_Q_RED_MISC_CFG(x)                                   (VTSS_ADDR_QSS_Q_RED_MISC_CFG + (x))
#define VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_3                              20
#define VTSS_LEN_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_3                               8
#define VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_2                              12
#define VTSS_LEN_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_2                               8
#define VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_1                               4
#define VTSS_LEN_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_1                               8
#define VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_TH_GAIN                              1
#define VTSS_LEN_QSS_Q_RED_MISC_CFG_Q_RED_TH_GAIN                              3
#define VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_ENA                                  0
#define VTSS_LEN_QSS_Q_RED_MISC_CFG_Q_RED_ENA                                  1

#define VTSS_ADDR_QSS_Q_RED_MIN_MAX_TH                                    0x1850
#define VTSS_ADDX_QSS_Q_RED_MIN_MAX_TH(x)                                 (VTSS_ADDR_QSS_Q_RED_MIN_MAX_TH + (x))
#define VTSS_OFF_QSS_Q_RED_MIN_MAX_TH_Q_RED_MAXMIN_TH                          8
#define VTSS_LEN_QSS_Q_RED_MIN_MAX_TH_Q_RED_MAXMIN_TH                          8
#define VTSS_OFF_QSS_Q_RED_MIN_MAX_TH_Q_RED_MIN_TH                             0
#define VTSS_LEN_QSS_Q_RED_MIN_MAX_TH_Q_RED_MIN_TH                             8

#define VTSS_ADDR_QSS_Q_RED_WQ                                            0x1940
#define VTSS_ADDX_QSS_Q_RED_WQ(x)                                         (VTSS_ADDR_QSS_Q_RED_WQ + (x))
#define VTSS_OFF_QSS_Q_RED_WQ_Q_RED_WQ                                         0
#define VTSS_LEN_QSS_Q_RED_WQ_Q_RED_WQ                                         5

#define VTSS_ADDR_QSS_Q_BCNT_H                                            0x1e00
#define VTSS_ADDX_QSS_Q_BCNT_H(x)                                         (VTSS_ADDR_QSS_Q_BCNT_H + (x))
#define VTSS_OFF_QSS_Q_BCNT_H_Q_BCNT_H                                         0
#define VTSS_LEN_QSS_Q_BCNT_H_Q_BCNT_H                                         8

#define VTSS_ADDR_QSS_Q_BCNT_L                                            0x1ef0
#define VTSS_ADDX_QSS_Q_BCNT_L(x)                                         (VTSS_ADDR_QSS_Q_BCNT_L + (x))

#define VTSS_ADDR_QSS_Q_FCNT                                              0x1fe0
#define VTSS_ADDX_QSS_Q_FCNT(x)                                           (VTSS_ADDR_QSS_Q_FCNT + (x))

#define VTSS_ADDR_QSS_Q_BRM_DROP_BCNT                                     0x20d0
#define VTSS_ADDX_QSS_Q_BRM_DROP_BCNT(x)                                  (VTSS_ADDR_QSS_Q_BRM_DROP_BCNT + (x))

#define VTSS_ADDR_QSS_Q_BRM_DROP_FCNT                                     0x21c0
#define VTSS_ADDX_QSS_Q_BRM_DROP_FCNT(x)                                  (VTSS_ADDR_QSS_Q_BRM_DROP_FCNT + (x))

#define VTSS_ADDR_QSS_Q_RED_DROP_BCNT                                     0x22b0
#define VTSS_ADDX_QSS_Q_RED_DROP_BCNT(x)                                  (VTSS_ADDR_QSS_Q_RED_DROP_BCNT + (x))

#define VTSS_ADDR_QSS_Q_RED_DROP_FCNT                                     0x23a0
#define VTSS_ADDX_QSS_Q_RED_DROP_FCNT(x)                                  (VTSS_ADDR_QSS_Q_RED_DROP_FCNT + (x))

#define VTSS_ADDR_QSS_Q_ERR_DROP_BCNT                                     0x2490
#define VTSS_ADDX_QSS_Q_ERR_DROP_BCNT(x)                                  (VTSS_ADDR_QSS_Q_ERR_DROP_BCNT + (x))

#define VTSS_ADDR_QSS_Q_ERR_DROP_FCNT                                     0x2580
#define VTSS_ADDX_QSS_Q_ERR_DROP_FCNT(x)                                  (VTSS_ADDR_QSS_Q_ERR_DROP_FCNT + (x))

#define VTSS_ADDR_QSS_RX_LP_IPV4_BCNT_H                                   0x2e00
#define VTSS_ADDX_QSS_RX_LP_IPV4_BCNT_H(x)                                (VTSS_ADDR_QSS_RX_LP_IPV4_BCNT_H + (x))
#define VTSS_OFF_QSS_RX_LP_IPV4_BCNT_H_RX_LP_IPV4_BCNT_H                       0
#define VTSS_LEN_QSS_RX_LP_IPV4_BCNT_H_RX_LP_IPV4_BCNT_H                       8

#define VTSS_ADDR_QSS_RX_LP_IPV4_BCNT_L                                   0x2e18
#define VTSS_ADDX_QSS_RX_LP_IPV4_BCNT_L(x)                                (VTSS_ADDR_QSS_RX_LP_IPV4_BCNT_L + (x))

#define VTSS_ADDR_QSS_RX_LP_IPV4_FCNT                                     0x2e30
#define VTSS_ADDX_QSS_RX_LP_IPV4_FCNT(x)                                  (VTSS_ADDR_QSS_RX_LP_IPV4_FCNT + (x))

#define VTSS_ADDR_QSS_RX_LP_IPV6_BCNT_H                                   0x2e48
#define VTSS_ADDX_QSS_RX_LP_IPV6_BCNT_H(x)                                (VTSS_ADDR_QSS_RX_LP_IPV6_BCNT_H + (x))
#define VTSS_OFF_QSS_RX_LP_IPV6_BCNT_H_RX_LP_IPV6_BCNT_H                       0
#define VTSS_LEN_QSS_RX_LP_IPV6_BCNT_H_RX_LP_IPV6_BCNT_H                       8

#define VTSS_ADDR_QSS_RX_LP_IPV6_BCNT_L                                   0x2e60
#define VTSS_ADDX_QSS_RX_LP_IPV6_BCNT_L(x)                                (VTSS_ADDR_QSS_RX_LP_IPV6_BCNT_L + (x))

#define VTSS_ADDR_QSS_RX_LP_IPV6_FCNT                                     0x2e78
#define VTSS_ADDX_QSS_RX_LP_IPV6_FCNT(x)                                  (VTSS_ADDR_QSS_RX_LP_IPV6_FCNT + (x))

#define VTSS_ADDR_QSS_RX_LP_MPLS_BCNT_H                                   0x2e90
#define VTSS_ADDX_QSS_RX_LP_MPLS_BCNT_H(x)                                (VTSS_ADDR_QSS_RX_LP_MPLS_BCNT_H + (x))
#define VTSS_OFF_QSS_RX_LP_MPLS_BCNT_H_RX_LP_MPLS_BCNT_H                       0
#define VTSS_LEN_QSS_RX_LP_MPLS_BCNT_H_RX_LP_MPLS_BCNT_H                       8

#define VTSS_ADDR_QSS_RX_LP_MPLS_BCNT_L                                   0x2ea8
#define VTSS_ADDX_QSS_RX_LP_MPLS_BCNT_L(x)                                (VTSS_ADDR_QSS_RX_LP_MPLS_BCNT_L + (x))

#define VTSS_ADDR_QSS_RX_LP_MPLS_FCNT                                     0x2ec0
#define VTSS_ADDX_QSS_RX_LP_MPLS_FCNT(x)                                  (VTSS_ADDR_QSS_RX_LP_MPLS_FCNT + (x))

#define VTSS_ADDR_QSS_RX_LP_TE_BCNT_H                                     0x2ed8
#define VTSS_ADDX_QSS_RX_LP_TE_BCNT_H(x)                                  (VTSS_ADDR_QSS_RX_LP_TE_BCNT_H + (x))
#define VTSS_OFF_QSS_RX_LP_TE_BCNT_H_RX_LP_TE_BCNT_H                           0
#define VTSS_LEN_QSS_RX_LP_TE_BCNT_H_RX_LP_TE_BCNT_H                           8

#define VTSS_ADDR_QSS_RX_LP_TE_BCNT_L                                     0x2ef0
#define VTSS_ADDX_QSS_RX_LP_TE_BCNT_L(x)                                  (VTSS_ADDR_QSS_RX_LP_TE_BCNT_L + (x))

#define VTSS_ADDR_QSS_RX_LP_TE_FCNT                                       0x2f08
#define VTSS_ADDX_QSS_RX_LP_TE_FCNT(x)                                    (VTSS_ADDR_QSS_RX_LP_TE_FCNT + (x))

#define VTSS_ADDR_QSS_RX_LP_UTE_BCNT_H                                    0x2f20
#define VTSS_ADDX_QSS_RX_LP_UTE_BCNT_H(x)                                 (VTSS_ADDR_QSS_RX_LP_UTE_BCNT_H + (x))
#define VTSS_OFF_QSS_RX_LP_UTE_BCNT_H_RX_LP_UTE_BCNT_H                         0
#define VTSS_LEN_QSS_RX_LP_UTE_BCNT_H_RX_LP_UTE_BCNT_H                         8

#define VTSS_ADDR_QSS_RX_LP_UTE_BCNT_L                                    0x2f38
#define VTSS_ADDX_QSS_RX_LP_UTE_BCNT_L(x)                                 (VTSS_ADDR_QSS_RX_LP_UTE_BCNT_L + (x))

#define VTSS_ADDR_QSS_RX_LP_UTE_FCNT                                      0x2f50
#define VTSS_ADDX_QSS_RX_LP_UTE_FCNT(x)                                   (VTSS_ADDR_QSS_RX_LP_UTE_FCNT + (x))

#define VTSS_ADDR_QSS_TX_LP_IPV4_BCNT_H                                   0x3200
#define VTSS_ADDX_QSS_TX_LP_IPV4_BCNT_H(x)                                (VTSS_ADDR_QSS_TX_LP_IPV4_BCNT_H + (x))
#define VTSS_OFF_QSS_TX_LP_IPV4_BCNT_H_TX_LP_IPV4_BCNT_H                       0
#define VTSS_LEN_QSS_TX_LP_IPV4_BCNT_H_TX_LP_IPV4_BCNT_H                       8

#define VTSS_ADDR_QSS_TX_LP_IPV4_BCNT_L                                   0x3218
#define VTSS_ADDX_QSS_TX_LP_IPV4_BCNT_L(x)                                (VTSS_ADDR_QSS_TX_LP_IPV4_BCNT_L + (x))

#define VTSS_ADDR_QSS_TX_LP_IPV4_FCNT                                     0x3230
#define VTSS_ADDX_QSS_TX_LP_IPV4_FCNT(x)                                  (VTSS_ADDR_QSS_TX_LP_IPV4_FCNT + (x))

#define VTSS_ADDR_QSS_TX_LP_IPV6_BCNT_H                                   0x3248
#define VTSS_ADDX_QSS_TX_LP_IPV6_BCNT_H(x)                                (VTSS_ADDR_QSS_TX_LP_IPV6_BCNT_H + (x))
#define VTSS_OFF_QSS_TX_LP_IPV6_BCNT_H_TX_LP_IPV6_BCNT_H                       0
#define VTSS_LEN_QSS_TX_LP_IPV6_BCNT_H_TX_LP_IPV6_BCNT_H                       8

#define VTSS_ADDR_QSS_TX_LP_IPV6_BCNT_L                                   0x3260
#define VTSS_ADDX_QSS_TX_LP_IPV6_BCNT_L(x)                                (VTSS_ADDR_QSS_TX_LP_IPV6_BCNT_L + (x))

#define VTSS_ADDR_QSS_TX_LP_IPV6_FCNT                                     0x3278
#define VTSS_ADDX_QSS_TX_LP_IPV6_FCNT(x)                                  (VTSS_ADDR_QSS_TX_LP_IPV6_FCNT + (x))

#define VTSS_ADDR_QSS_TX_LP_MPLS_BCNT_H                                   0x3290
#define VTSS_ADDX_QSS_TX_LP_MPLS_BCNT_H(x)                                (VTSS_ADDR_QSS_TX_LP_MPLS_BCNT_H + (x))
#define VTSS_OFF_QSS_TX_LP_MPLS_BCNT_H_TX_LP_MPLS_BCNT_H                       0
#define VTSS_LEN_QSS_TX_LP_MPLS_BCNT_H_TX_LP_MPLS_BCNT_H                       8

#define VTSS_ADDR_QSS_TX_LP_MPLS_BCNT_L                                   0x32a8
#define VTSS_ADDX_QSS_TX_LP_MPLS_BCNT_L(x)                                (VTSS_ADDR_QSS_TX_LP_MPLS_BCNT_L + (x))

#define VTSS_ADDR_QSS_TX_LP_MPLS_FCNT                                     0x32c0
#define VTSS_ADDX_QSS_TX_LP_MPLS_FCNT(x)                                  (VTSS_ADDR_QSS_TX_LP_MPLS_FCNT + (x))

#define VTSS_ADDR_QSS_TX_LP_TE_BCNT_H                                     0x32d8
#define VTSS_ADDX_QSS_TX_LP_TE_BCNT_H(x)                                  (VTSS_ADDR_QSS_TX_LP_TE_BCNT_H + (x))
#define VTSS_OFF_QSS_TX_LP_TE_BCNT_H_TX_LP_TE_BCNT_H                           0
#define VTSS_LEN_QSS_TX_LP_TE_BCNT_H_TX_LP_TE_BCNT_H                           8

#define VTSS_ADDR_QSS_TX_LP_TE_BCNT_L                                     0x32f0
#define VTSS_ADDX_QSS_TX_LP_TE_BCNT_L(x)                                  (VTSS_ADDR_QSS_TX_LP_TE_BCNT_L + (x))

#define VTSS_ADDR_QSS_TX_LP_TE_FCNT                                       0x3308
#define VTSS_ADDX_QSS_TX_LP_TE_FCNT(x)                                    (VTSS_ADDR_QSS_TX_LP_TE_FCNT + (x))

#define VTSS_ADDR_QSS_TX_LP_UTE_BCNT_H                                    0x3320
#define VTSS_ADDX_QSS_TX_LP_UTE_BCNT_H(x)                                 (VTSS_ADDR_QSS_TX_LP_UTE_BCNT_H + (x))
#define VTSS_OFF_QSS_TX_LP_UTE_BCNT_H_TX_LP_UTE_BCNT_H                         0
#define VTSS_LEN_QSS_TX_LP_UTE_BCNT_H_TX_LP_UTE_BCNT_H                         8

#define VTSS_ADDR_QSS_TX_LP_UTE_BCNT_L                                    0x3338
#define VTSS_ADDX_QSS_TX_LP_UTE_BCNT_L(x)                                 (VTSS_ADDR_QSS_TX_LP_UTE_BCNT_L + (x))

#define VTSS_ADDR_QSS_TX_LP_UTE_FCNT                                      0x3350
#define VTSS_ADDX_QSS_TX_LP_UTE_FCNT(x)                                   (VTSS_ADDR_QSS_TX_LP_UTE_FCNT + (x))

#define VTSS_ADDR_QSS_FLIST_ELEM_DETECT_PTR                               0x3600
#define VTSS_OFF_QSS_FLIST_ELEM_DETECT_PTR_FLIST_ELEM_DETECT_PTR               0
#define VTSS_LEN_QSS_FLIST_ELEM_DETECT_PTR_FLIST_ELEM_DETECT_PTR              13

#define VTSS_ADDR_QSS_FLIST_ELEM_RECOV_ENA                                0x3601
#define VTSS_OFF_QSS_FLIST_ELEM_RECOV_ENA_FLIST_ELEM_RECOV_ENA                 0
#define VTSS_LEN_QSS_FLIST_ELEM_RECOV_ENA_FLIST_ELEM_RECOV_ENA                 1

#define VTSS_ADDR_QSS_FLIST_ALLOC_CNT_OVFL_STICKY                         0x3602
#define VTSS_OFF_QSS_FLIST_ALLOC_CNT_OVFL_STICKY_FLIST_ALLOC_CNT_OVFL_STICKY      0
#define VTSS_LEN_QSS_FLIST_ALLOC_CNT_OVFL_STICKY_FLIST_ALLOC_CNT_OVFL_STICKY      1

#define VTSS_ADDR_QSS_FLIST_ELEM_DETECT_STICKY                            0x3603
#define VTSS_OFF_QSS_FLIST_ELEM_DETECT_STICKY_FLIST_ELEM_DETECT_STICKY         0
#define VTSS_LEN_QSS_FLIST_ELEM_DETECT_STICKY_FLIST_ELEM_DETECT_STICKY         1

#define VTSS_ADDR_QSS_FLIST_ELEM_RECOV_ERR_STICKY                         0x3604
#define VTSS_OFF_QSS_FLIST_ELEM_RECOV_ERR_STICKY_FLIST_ELEM_RECOV_ERR_STICKY      0
#define VTSS_LEN_QSS_FLIST_ELEM_RECOV_ERR_STICKY_FLIST_ELEM_RECOV_ERR_STICKY      1

#define VTSS_ADDR_QSS_FL_DSCRPT_BUF_CNT                                   0x3605
#define VTSS_OFF_QSS_FL_DSCRPT_BUF_CNT_FL_DSCRPT_BUF_CNT                       0
#define VTSS_LEN_QSS_FL_DSCRPT_BUF_CNT_FL_DSCRPT_BUF_CNT                      14

#define VTSS_ADDR_QSS_FL_DSCRPT_PTRS                                      0x3606
#define VTSS_OFF_QSS_FL_DSCRPT_PTRS_FL_DSCRPT_HB                              16
#define VTSS_LEN_QSS_FL_DSCRPT_PTRS_FL_DSCRPT_HB                              13
#define VTSS_OFF_QSS_FL_DSCRPT_PTRS_FL_DSCRPT_TB                               0
#define VTSS_LEN_QSS_FL_DSCRPT_PTRS_FL_DSCRPT_TB                              13

#define VTSS_ADDR_QSS_FL_TOP_PTR                                          0x3607
#define VTSS_OFF_QSS_FL_TOP_PTR_FL_TOP_PTR                                     0
#define VTSS_LEN_QSS_FL_TOP_PTR_FL_TOP_PTR                                    13

#define VTSS_ADDR_QSS_QSS_DEBUG_INIT                                      0x3700
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_QSP_COUNTER_INIT                           5
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_QSP_COUNTER_INIT                           1
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_BRM_COUNTER_INIT                           4
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_BRM_COUNTER_INIT                           1
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_BRM_DISABLE                                3
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_BRM_DISABLE                                1
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_FLM_LLM_INIT                               2
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_FLM_LLM_INIT                               1
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_Q_DSCRPT_INIT                              1
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_Q_DSCRPT_INIT                              1
#define VTSS_OFF_QSS_QSS_DEBUG_INIT_PORT_DSCRPT_INIT                           0
#define VTSS_LEN_QSS_QSS_DEBUG_INIT_PORT_DSCRPT_INIT                           1

#define VTSS_ADDR_QSS_QSS_OFFLINE_ENA                                     0x3701
#define VTSS_OFF_QSS_QSS_OFFLINE_ENA_QSS_OFFLINE_ENA                           0
#define VTSS_LEN_QSS_QSS_OFFLINE_ENA_QSS_OFFLINE_ENA                           1

#define VTSS_ADDR_QSS_QSS_SPARE                                           0x3800

#define VTSS_ADDR_QSS_PFCI_LPORT_ENA                                      0x3801
#define VTSS_OFF_QSS_PFCI_LPORT_ENA_PFCI_LPORT_ENA                             0
#define VTSS_LEN_QSS_PFCI_LPORT_ENA_PFCI_LPORT_ENA                            24

#define VTSS_ADDR_QSS_PFCI_LPORT_STATE                                    0x3802
#define VTSS_OFF_QSS_PFCI_LPORT_STATE_PFCI_LPORT_STATE                         0
#define VTSS_LEN_QSS_PFCI_LPORT_STATE_PFCI_LPORT_STATE                        24

/*********************************************************************** 
 * Target DEVSPI
 * Device SPI4
 ***********************************************************************/
#define VTSS_ADDR_DEVSPI_SPI4_IB_CONFIG                                   0x0000
#define VTSS_OFF_DEVSPI_SPI4_IB_CONFIG_CONF_IB_BUS_DISABLE                     1
#define VTSS_LEN_DEVSPI_SPI4_IB_CONFIG_CONF_IB_BUS_DISABLE                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_CONFIG_CONF_IB_TRANSM_USIZE_PKT                0
#define VTSS_LEN_DEVSPI_SPI4_IB_CONFIG_CONF_IB_TRANSM_USIZE_PKT                1

#define VTSS_ADDR_DEVSPI_SPI4_IB_DIP4_CONFIG                              0x0001
#define VTSS_OFF_DEVSPI_SPI4_IB_DIP4_CONFIG_CONF_IB_DIP4_THRES                 8
#define VTSS_LEN_DEVSPI_SPI4_IB_DIP4_CONFIG_CONF_IB_DIP4_THRES                 4
#define VTSS_OFF_DEVSPI_SPI4_IB_DIP4_CONFIG_CONF_IB_DIP4_WINDOW                0
#define VTSS_LEN_DEVSPI_SPI4_IB_DIP4_CONFIG_CONF_IB_DIP4_WINDOW                6

#define VTSS_ADDR_DEVSPI_SPI4_IB_SYNC_CONFIG                              0x0002
#define VTSS_OFF_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_DIP4_RESYNC_EN             1
#define VTSS_LEN_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_DIP4_RESYNC_EN             1
#define VTSS_OFF_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_FRC_DSKW_RESYNC            0
#define VTSS_LEN_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_FRC_DSKW_RESYNC            1

#define VTSS_ADDR_DEVSPI_SPI4_IB_DIP4_ERR_CNT                             0x0003

#define VTSS_ADDR_DEVSPI_SPI4_IB_PKT_CNT                                  0x0004

#define VTSS_ADDR_DEVSPI_SPI4_IB_EOPA_CNT                                 0x0005

#define VTSS_ADDR_DEVSPI_SPI4_IB_BYTE_CNT                                 0x0006

#define VTSS_ADDR_DEVSPI_SPI4_IB_USIZE_PKT_CNT                            0x0007

#define VTSS_ADDR_DEVSPI_SPI4_IB_STICKY                                   0x0008
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_INVALID_PORT_NUM_STICKY             25
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_INVALID_PORT_NUM_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_PUSH_BACK_STICKY                    24
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_PUSH_BACK_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_USIZE_PKT_STICKY                    23
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_USIZE_PKT_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SYNC_FAIL_STICKY                    22
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SYNC_FAIL_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_DIP4_ERR_STICKY                     21
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_DIP4_ERR_STICKY                      1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_RAM_ERROR_STICKY                    20
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_RAM_ERROR_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_BUF_OVFL_STICKY                     19
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_BUF_OVFL_STICKY                      1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_4_PLUS_IDLE_STICKY          18
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_4_PLUS_IDLE_STICKY           1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_4_IDLE_STICKY               17
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_4_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_3_IDLE_STICKY               16
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_3_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_2_IDLE_STICKY               15
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_2_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_1_IDLE_STICKY               14
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_1_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_N_IDLE_STICKY               13
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_N_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_C_IDLE_STICKY               12
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOB_EOB_C_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_SOP_ALL_STICKY                      11
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_SOP_ALL_STICKY                       1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_EOP_ALL_STICKY                      10
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_EOP_ALL_STICKY                       1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_EOPA_WORD_STICKY                     9
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_EOPA_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_EOP2_WORD_STICKY                     8
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_EOP2_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_EOP1_WORD_STICKY                     7
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_EOP1_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_TRAINING_WORD_STICKY                 6
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_TRAINING_WORD_STICKY                 1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_UNKN_DATA_WORD_STICKY                5
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_UNKN_DATA_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_DATA_WORD_STICKY                     4
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_DATA_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_UNKN_CTRL_WORD_STICKY                3
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_UNKN_CTRL_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_EXT_CTRL_WORD_STICKY                 2
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_EXT_CTRL_WORD_STICKY                 1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_IDLE_CTRL_WORD_STICKY                1
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_IDLE_CTRL_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_IB_STICKY_IB_CTRL_WORD_STICKY                     0
#define VTSS_LEN_DEVSPI_SPI4_IB_STICKY_IB_CTRL_WORD_STICKY                     1

#define VTSS_ADDR_DEVSPI_SPI4_OB_CONFIG                                   0x0009
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_TAXI_PHY_LPBK                  30
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_TAXI_PHY_LPBK                   1
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_COMB_EOP_ENA                   29
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_COMB_EOP_ENA                    1
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_COMB_FILL_LVL                  25
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_COMB_FILL_LVL                   4
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_PACKET_EXTEND                  20
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_PACKET_EXTEND                   5
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_BUS_DISABLE                    17
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_BUS_DISABLE                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_BURST_EXTEND                   11
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_BURST_EXTEND                    5
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_FIFO_RESET                      8
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_FIFO_RESET                      1
#define VTSS_OFF_DEVSPI_SPI4_OB_CONFIG_CONF_OB_EFF_FIFO_SIZE                   0
#define VTSS_LEN_DEVSPI_SPI4_OB_CONFIG_CONF_OB_EFF_FIFO_SIZE                   5

#define VTSS_ADDR_DEVSPI_SPI4_OB_TRAIN_CONFIG                             0x000a
#define VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ENA_AUTO_TRAINING        25
#define VTSS_LEN_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ENA_AUTO_TRAINING         1
#define VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_SEND_TRAINING            24
#define VTSS_LEN_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_SEND_TRAINING             1
#define VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ALPHA_MAX_T              16
#define VTSS_LEN_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ALPHA_MAX_T               8
#define VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_DATA_MAX_T                0
#define VTSS_LEN_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_DATA_MAX_T               10

#define VTSS_ADDR_DEVSPI_SPI4_OB_STATUS                                   0x000b
#define VTSS_OFF_DEVSPI_SPI4_OB_STATUS_STAT_OB_FIFO_EMPTY                      0
#define VTSS_LEN_DEVSPI_SPI4_OB_STATUS_STAT_OB_FIFO_EMPTY                      1

#define VTSS_ADDR_DEVSPI_SPI4_OB_PKT_CNT                                  0x000c

#define VTSS_ADDR_DEVSPI_SPI4_OB_EOPA_CNT                                 0x000d

#define VTSS_ADDR_DEVSPI_SPI4_OB_BYTE_CNT                                 0x000e

#define VTSS_ADDR_DEVSPI_SPI4_OB_STICKY                                   0x000f
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_RAM_ERROR_STICKY                    20
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_RAM_ERROR_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_BUF_OVFL_STICKY                     19
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_BUF_OVFL_STICKY                      1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_4_PLUS_IDLE_STICKY          18
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_4_PLUS_IDLE_STICKY           1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_4_IDLE_STICKY               17
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_4_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_3_IDLE_STICKY               16
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_3_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_2_IDLE_STICKY               15
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_2_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_1_IDLE_STICKY               14
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_1_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_N_IDLE_STICKY               13
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_N_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_C_IDLE_STICKY               12
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOB_EOB_C_IDLE_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_SOP_ALL_STICKY                      11
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_SOP_ALL_STICKY                       1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_EOP_ALL_STICKY                      10
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_EOP_ALL_STICKY                       1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_EOPA_WORD_STICKY                     9
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_EOPA_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_EOP2_WORD_STICKY                     8
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_EOP2_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_EOP1_WORD_STICKY                     7
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_EOP1_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_TRAINING_WORD_STICKY                 6
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_TRAINING_WORD_STICKY                 1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_UNKN_DATA_WORD_STICKY                5
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_UNKN_DATA_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_DATA_WORD_STICKY                     4
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_DATA_WORD_STICKY                     1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_UNKN_CTRL_WORD_STICKY                3
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_UNKN_CTRL_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_EXT_CTRL_WORD_STICKY                 2
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_EXT_CTRL_WORD_STICKY                 1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_IDLE_CTRL_WORD_STICKY                1
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_IDLE_CTRL_WORD_STICKY                1
#define VTSS_OFF_DEVSPI_SPI4_OB_STICKY_OB_CTRL_WORD_STICKY                     0
#define VTSS_LEN_DEVSPI_SPI4_OB_STICKY_OB_CTRL_WORD_STICKY                     1

#define VTSS_ADDR_DEVSPI_SPI4_IBS_CONFIG                                  0x0010
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_EM_STOP_ENA                  20
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_EM_STOP_ENA                   1
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CLK_SHIFT                    19
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CLK_SHIFT                     1
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_FORCE_IDLE                   18
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_FORCE_IDLE                    1
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DSKW_GENS_IDLE               17
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DSKW_GENS_IDLE                1
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DIP4_GENS_IDLE               16
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DIP4_GENS_IDLE                1
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_M                         8
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_M                         4
#define VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_LEN                       0
#define VTSS_LEN_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_LEN                       6

#define VTSS_ADDR_DEVSPI_SPI4_IBS_DIAG                                    0x0011
#define VTSS_OFF_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_OVWR_ENA                       10
#define VTSS_LEN_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_OVWR_ENA                        1
#define VTSS_OFF_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_OVWR_VAL                        8
#define VTSS_LEN_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_OVWR_VAL                        2
#define VTSS_OFF_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_STAT_CAPTURE                    7
#define VTSS_LEN_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_STAT_CAPTURE                    1
#define VTSS_OFF_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_STAT_PORT_NUM                   0
#define VTSS_LEN_DEVSPI_SPI4_IBS_DIAG_CONF_IBS_STAT_PORT_NUM                   7

#define VTSS_ADDR_DEVSPI_SPI4_IBS_CALENDAR                                0x0012
#define VTSS_ADDX_DEVSPI_SPI4_IBS_CALENDAR(x)                             (VTSS_ADDR_DEVSPI_SPI4_IBS_CALENDAR + (x))
#define VTSS_OFF_DEVSPI_SPI4_IBS_CALENDAR_CONF_IBS_CAL_VAL                     0
#define VTSS_LEN_DEVSPI_SPI4_IBS_CALENDAR_CONF_IBS_CAL_VAL                     7

#define VTSS_ADDR_DEVSPI_SPI4_IBS_STATUS                                  0x0052
#define VTSS_OFF_DEVSPI_SPI4_IBS_STATUS_STAT_IB_LINK_STATE                     2
#define VTSS_LEN_DEVSPI_SPI4_IBS_STATUS_STAT_IB_LINK_STATE                     1
#define VTSS_OFF_DEVSPI_SPI4_IBS_STATUS_STAT_IBS_VALUE                         0
#define VTSS_LEN_DEVSPI_SPI4_IBS_STATUS_STAT_IBS_VALUE                         2

#define VTSS_ADDR_DEVSPI_SPI4_OBS_CONFIG                                  0x0053
#define VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CLK_SHIFT                    20
#define VTSS_LEN_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CLK_SHIFT                     1
#define VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_M                         8
#define VTSS_LEN_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_M                         4
#define VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_LEN                       0
#define VTSS_LEN_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_LEN                       6

#define VTSS_ADDR_DEVSPI_SPI4_OBS_LINK_CONFIG                             0x0054
#define VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_IDLE_THRESH             12
#define VTSS_LEN_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_IDLE_THRESH              4
#define VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINK_DOWN_DIS            8
#define VTSS_LEN_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINK_DOWN_DIS            1
#define VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKUP_LIM               4
#define VTSS_LEN_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKUP_LIM               4
#define VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKDN_LIM               0
#define VTSS_LEN_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKDN_LIM               4

#define VTSS_ADDR_DEVSPI_SPI4_OBS_DIAG_CONFIG                             0x0055
#define VTSS_OFF_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_OVWR_ENA                10
#define VTSS_LEN_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_OVWR_ENA                 1
#define VTSS_OFF_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_OVWR_VAL                 8
#define VTSS_LEN_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_OVWR_VAL                 2
#define VTSS_OFF_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_STAT_CAPTURE             7
#define VTSS_LEN_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_STAT_CAPTURE             1
#define VTSS_OFF_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_STAT_PORT_NUM            0
#define VTSS_LEN_DEVSPI_SPI4_OBS_DIAG_CONFIG_CONF_OBS_STAT_PORT_NUM            7

#define VTSS_ADDR_DEVSPI_SPI4_OBS_CALENDAR                                0x0056
#define VTSS_ADDX_DEVSPI_SPI4_OBS_CALENDAR(x)                             (VTSS_ADDR_DEVSPI_SPI4_OBS_CALENDAR + (x))
#define VTSS_OFF_DEVSPI_SPI4_OBS_CALENDAR_CONF_OBS_CAL_VAL                     0
#define VTSS_LEN_DEVSPI_SPI4_OBS_CALENDAR_CONF_OBS_CAL_VAL                     7

#define VTSS_ADDR_DEVSPI_SPI4_OBS_STATUS                                  0x0096
#define VTSS_OFF_DEVSPI_SPI4_OBS_STATUS_STAT_OBS_LINK_STATUS                   2
#define VTSS_LEN_DEVSPI_SPI4_OBS_STATUS_STAT_OBS_LINK_STATUS                   1
#define VTSS_OFF_DEVSPI_SPI4_OBS_STATUS_STAT_OBS_VALUE                         0
#define VTSS_LEN_DEVSPI_SPI4_OBS_STATUS_STAT_OBS_VALUE                         2

#define VTSS_ADDR_DEVSPI_SPI4_OBS_DIP2_ERR_CNT                            0x0097

#define VTSS_ADDR_DEVSPI_SPI4_OBS_STICKY                                  0x0098
#define VTSS_OFF_DEVSPI_SPI4_OBS_STICKY_OBS_LINK_DOWN_STICKY                   2
#define VTSS_LEN_DEVSPI_SPI4_OBS_STICKY_OBS_LINK_DOWN_STICKY                   1
#define VTSS_OFF_DEVSPI_SPI4_OBS_STICKY_OBS_DIP2_ERR_STICKY                    1
#define VTSS_LEN_DEVSPI_SPI4_OBS_STICKY_OBS_DIP2_ERR_STICKY                    1
#define VTSS_OFF_DEVSPI_SPI4_OBS_STICKY_OBS_IDLE_MODE_STICKY                   0
#define VTSS_LEN_DEVSPI_SPI4_OBS_STICKY_OBS_IDLE_MODE_STICKY                   1

#define VTSS_ADDR_DEVSPI_SPI4_INT_MASK                                    0x0099
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_IB_USIZE_PKT_MASK                        8
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_IB_USIZE_PKT_MASK                        1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_IB_SYNC_FAIL_MASK                        7
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_IB_SYNC_FAIL_MASK                        1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_IB_DIP4_ERR_MASK                         6
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_IB_DIP4_ERR_MASK                         1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_IB_RAM_ERROR_MASK                        5
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_IB_RAM_ERROR_MASK                        1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_IB_BUF_OVFL_MASK                         4
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_IB_BUF_OVFL_MASK                         1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_OB_RAM_ERROR_MASK                        3
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_OB_RAM_ERROR_MASK                        1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_OB_BUF_OVFL_MASK                         2
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_OB_BUF_OVFL_MASK                         1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_OBS_DIP2_ERR_MASK                        1
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_OBS_DIP2_ERR_MASK                        1
#define VTSS_OFF_DEVSPI_SPI4_INT_MASK_OBS_LINK_DOWN_MASK                       0
#define VTSS_LEN_DEVSPI_SPI4_INT_MASK_OBS_LINK_DOWN_MASK                       1

#define VTSS_ADDR_DEVSPI_SPI4_OB_CNTR_CONFIG                              0x009a
#define VTSS_OFF_DEVSPI_SPI4_OB_CNTR_CONFIG_CONF_OB_CNTR_MASK                  0
#define VTSS_LEN_DEVSPI_SPI4_OB_CNTR_CONFIG_CONF_OB_CNTR_MASK                  7

#define VTSS_ADDR_DEVSPI_SPI4_IB_CNTR_CONFIG                              0x009b
#define VTSS_OFF_DEVSPI_SPI4_IB_CNTR_CONFIG_CONF_IB_CNTR_MASK                  0
#define VTSS_LEN_DEVSPI_SPI4_IB_CNTR_CONFIG_CONF_IB_CNTR_MASK                  7

#define VTSS_ADDR_DEVSPI_SPI4_OB_TG_CONFIG                                0x009c
#define VTSS_OFF_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_PAT_SEL                   2
#define VTSS_LEN_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_PAT_SEL                   1
#define VTSS_OFF_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_USR_SEL                   1
#define VTSS_LEN_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_USR_SEL                   1
#define VTSS_OFF_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_ENA                       0
#define VTSS_LEN_DEVSPI_SPI4_OB_TG_CONFIG_CONF_OB_TG_ENA                       1

#define VTSS_ADDR_DEVSPI_SPI4_OB_TG_UDP0                                  0x009d

#define VTSS_ADDR_DEVSPI_SPI4_OB_TG_UDP1                                  0x009e

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_CONFIG                                0x009f
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_NO_LOCK                  15
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_NO_LOCK                   1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_LANE_SEL                  8
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_LANE_SEL                  5
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_SAMP                      3
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_SAMP                      1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_PAT_SEL                   2
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_PAT_SEL                   1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_USR_SEL                   1
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_USR_SEL                   1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_ENA                       0
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_ENA                       1

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_UDP0                                  0x00a0

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_UDP1                                  0x00a1

#define VTSS_ADDR_DEVSPI_SPI4_DBG_RW                                      0x00a2

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_SAMP0                                 0x00a3

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_SAMP1                                 0x00a4

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_ERR_CNT                               0x00a5

#define VTSS_ADDR_DEVSPI_SPI4_DBG_RO                                      0x00a6

#define VTSS_ADDR_DEVSPI_SPI4_IB_TC_STICKY                                0x00a7
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_PRBS_LOCKED_STICKY            18
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_PRBS_LOCKED_STICKY             1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_UDP_LOCKED_STICKY             17
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_UDP_LOCKED_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE16_ERR_STICKY             16
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE16_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE15_ERR_STICKY             15
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE15_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE14_ERR_STICKY             14
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE14_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE13_ERR_STICKY             13
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE13_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE12_ERR_STICKY             12
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE12_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE11_ERR_STICKY             11
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE11_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE10_ERR_STICKY             10
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE10_ERR_STICKY              1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE9_ERR_STICKY               9
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE9_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE8_ERR_STICKY               8
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE8_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE7_ERR_STICKY               7
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE7_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE6_ERR_STICKY               6
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE6_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE5_ERR_STICKY               5
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE5_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE4_ERR_STICKY               4
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE4_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE3_ERR_STICKY               3
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE3_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE2_ERR_STICKY               2
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE2_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE1_ERR_STICKY               1
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE1_ERR_STICKY               1
#define VTSS_OFF_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE0_ERR_STICKY               0
#define VTSS_LEN_DEVSPI_SPI4_IB_TC_STICKY_IB_TC_LANE0_ERR_STICKY               1

#define VTSS_ADDR_DEVSPI_SPI4_DDS_CONFIG                                  0x00a8
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_OB_RESET                          6
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_OB_RESET                          1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_IB_RESET                          5
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_IB_RESET                          1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_POWEROFF                          4
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_POWEROFF                          1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_EXT_STATUS_PLOOP                  3
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_EXT_STATUS_PLOOP                  1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_INT_STATUS_PLOOP                  2
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_INT_STATUS_PLOOP                  1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_EXT_DATA_PLOOP                    1
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_EXT_DATA_PLOOP                    1
#define VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_INT_DATA_PLOOP                    0
#define VTSS_LEN_DEVSPI_SPI4_DDS_CONFIG_CONF_INT_DATA_PLOOP                    1

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_CONFIG                               0x00a9
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_CMU_BYP                    25
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_CMU_BYP                     1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DDS_BYP                    24
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DDS_BYP                     1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DRU_FAST                   23
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DRU_FAST                    1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_VCO_RANGE                  21
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_VCO_RANGE                   2
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_FORCEMIN                   20
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_FORCEMIN                    1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_QUARTER_RATE               19
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_QUARTER_RATE                1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_HALF_RATE                  18
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_HALF_RATE                   1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DATA_SWAP                  17
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DATA_SWAP                   1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_INV_DATA                    0
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_INV_DATA                   17

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_PHASE                                0x00aa
#define VTSS_ADDX_DEVSPI_SPI4_DDS_IB_PHASE(x)                             (VTSS_ADDR_DEVSPI_SPI4_DDS_IB_PHASE + (x))
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_PHASE_CONF_IB_DDS_ALIGN_OFFSET             0
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_PHASE_CONF_IB_DDS_ALIGN_OFFSET             3

#define VTSS_ADDR_DEVSPI_SPI4_DDS_OB_CONFIG                               0x00bb
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_VCO_RANGE                  27
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_VCO_RANGE                   2
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_FORCEMIN                   26
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_FORCEMIN                    1
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CLK_PHASE                  24
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CLK_PHASE                   2
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CMU_RATIO                  20
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CMU_RATIO                   4
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_QUARTER_RATE               19
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_QUARTER_RATE                1
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_HALF_RATE                  18
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_HALF_RATE                   1
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_DATA_SWAP                  17
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_DATA_SWAP                   1
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_INV_DATA                    0
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_INV_DATA                   17

#define VTSS_ADDR_DEVSPI_SPI4_DDS_OB_SYNC_STATUS                          0x00bc
#define VTSS_OFF_DEVSPI_SPI4_DDS_OB_SYNC_STATUS_STAT_OB_PLOCK                  0
#define VTSS_LEN_DEVSPI_SPI4_DDS_OB_SYNC_STATUS_STAT_OB_PLOCK                  1

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_SYNC_STATUS                          0x00bd
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_SYNC_STATUS_STAT_IB_SYNC_VALID             1
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_SYNC_STATUS_STAT_IB_SYNC_VALID             1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_SYNC_STATUS_STAT_IB_PLOCK                  0
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_SYNC_STATUS_STAT_IB_PLOCK                  1

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_STATUS                               0x00be
#define VTSS_ADDX_DEVSPI_SPI4_DDS_IB_STATUS(x)                            (VTSS_ADDR_DEVSPI_SPI4_DDS_IB_STATUS + (x))
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_STATUS_STAT_IB_FIFO                        8
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_STATUS_STAT_IB_FIFO                        4
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_STATUS_STAT_IB_DRU_PHASE                   0
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_STATUS_STAT_IB_DRU_PHASE                   5

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_TRAIN_CNT                            0x00cf

#define VTSS_ADDR_DEVSPI_SPI4_DDS_IB_STICKY                               0x00d0
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_STICKY_IB_SYNC_DET_STICKY                  1
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_STICKY_IB_SYNC_DET_STICKY                  1
#define VTSS_OFF_DEVSPI_SPI4_DDS_IB_STICKY_IB_TRAIN_DET_STICKY                 0
#define VTSS_LEN_DEVSPI_SPI4_DDS_IB_STICKY_IB_TRAIN_DET_STICKY                 1

/*********************************************************************** 
 * Target DEV1G
 * Device 1G
 ***********************************************************************/
#define VTSS_ADDR_DEV1G_DEV_RST_CTRL                                      0x0000
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_CLK_DIVIDE_SEL                            24
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_CLK_DIVIDE_SEL                             2
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_SPEED_SEL                                 20
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_SPEED_SEL                                  3
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_CLK_B_DRIVE_EN                            18
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_CLK_B_DRIVE_EN                             1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_CLK_A_DRIVE_EN                            17
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_CLK_A_DRIVE_EN                             1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_ENABLE                              16
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_FX100_ENABLE                               1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_SGMII_MACRO_RST                           15
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_SGMII_MACRO_RST                            1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_PCS_TX_RST                          14
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_FX100_PCS_TX_RST                           1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_PCS_RX_RST                          13
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_FX100_PCS_RX_RST                           1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_PCS_TX_RST                                12
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_PCS_TX_RST                                 1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_PCS_RX_RST                                 8
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_PCS_RX_RST                                 1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_TX_RST                                 4
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_MAC_TX_RST                                 1
#define VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_RX_RST                                 0
#define VTSS_LEN_DEV1G_DEV_RST_CTRL_MAC_RX_RST                                 1

#define VTSS_ADDR_DEV1G_DEV_STICKY                                        0x0001
#define VTSS_OFF_DEV1G_DEV_STICKY_SD_UP_STICKY                                15
#define VTSS_LEN_DEV1G_DEV_STICKY_SD_UP_STICKY                                 1
#define VTSS_OFF_DEV1G_DEV_STICKY_SD_DOWN_STICKY                              14
#define VTSS_LEN_DEV1G_DEV_STICKY_SD_DOWN_STICKY                               1
#define VTSS_OFF_DEV1G_DEV_STICKY_TX_TAXI_PROT_ERR_STICKY                     12
#define VTSS_LEN_DEV1G_DEV_STICKY_TX_TAXI_PROT_ERR_STICKY                      1
#define VTSS_OFF_DEV1G_DEV_STICKY_PRE_CNT_OFLW_STICKY                         11
#define VTSS_LEN_DEV1G_DEV_STICKY_PRE_CNT_OFLW_STICKY                          1
#define VTSS_OFF_DEV1G_DEV_STICKY_RX_OFLW_STICKY                              10
#define VTSS_LEN_DEV1G_DEV_STICKY_RX_OFLW_STICKY                               1
#define VTSS_OFF_DEV1G_DEV_STICKY_TX_EOF_STICKY                                3
#define VTSS_LEN_DEV1G_DEV_STICKY_TX_EOF_STICKY                                1
#define VTSS_OFF_DEV1G_DEV_STICKY_TX_SOF_STICKY                                2
#define VTSS_LEN_DEV1G_DEV_STICKY_TX_SOF_STICKY                                1
#define VTSS_OFF_DEV1G_DEV_STICKY_TX_OFLW_STICKY                               1
#define VTSS_LEN_DEV1G_DEV_STICKY_TX_OFLW_STICKY                               1
#define VTSS_OFF_DEV1G_DEV_STICKY_TX_UFLW_STICKY                               0
#define VTSS_LEN_DEV1G_DEV_STICKY_TX_UFLW_STICKY                               1

#define VTSS_ADDR_DEV1G_DEV_INT_MASK                                      0x0002
#define VTSS_OFF_DEV1G_DEV_INT_MASK_SD_UP_INT_MASK                            15
#define VTSS_LEN_DEV1G_DEV_INT_MASK_SD_UP_INT_MASK                             1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_SD_DOWN_INT_MASK                          14
#define VTSS_LEN_DEV1G_DEV_INT_MASK_SD_DOWN_INT_MASK                           1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_TX_TAXI_PROT_ERR_INT_MASK                 12
#define VTSS_LEN_DEV1G_DEV_INT_MASK_TX_TAXI_PROT_ERR_INT_MASK                  1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_PRE_CNT_OFLW_INT_MASK                     11
#define VTSS_LEN_DEV1G_DEV_INT_MASK_PRE_CNT_OFLW_INT_MASK                      1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_RX_OFLW_INT_MASK                          10
#define VTSS_LEN_DEV1G_DEV_INT_MASK_RX_OFLW_INT_MASK                           1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_TX_EOF_INT_MASK                            3
#define VTSS_LEN_DEV1G_DEV_INT_MASK_TX_EOF_INT_MASK                            1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_TX_SOF_INT_MASK                            2
#define VTSS_LEN_DEV1G_DEV_INT_MASK_TX_SOF_INT_MASK                            1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_TX_OFLW_INT_MASK                           1
#define VTSS_LEN_DEV1G_DEV_INT_MASK_TX_OFLW_INT_MASK                           1
#define VTSS_OFF_DEV1G_DEV_INT_MASK_TX_UFLW_INT_MASK                           0
#define VTSS_LEN_DEV1G_DEV_INT_MASK_TX_UFLW_INT_MASK                           1

#define VTSS_ADDR_DEV1G_DEV_LB_CFG                                        0x0003
#define VTSS_OFF_DEV1G_DEV_LB_CFG_INV_RX_CLOCK                                12
#define VTSS_LEN_DEV1G_DEV_LB_CFG_INV_RX_CLOCK                                 1
#define VTSS_OFF_DEV1G_DEV_LB_CFG_LINE_LB_ENA                                  6
#define VTSS_LEN_DEV1G_DEV_LB_CFG_LINE_LB_ENA                                  1
#define VTSS_OFF_DEV1G_DEV_LB_CFG_TAXI_PHY_LB_ENA                              4
#define VTSS_LEN_DEV1G_DEV_LB_CFG_TAXI_PHY_LB_ENA                              1
#define VTSS_OFF_DEV1G_DEV_LB_CFG_TAXI_HOST_LB_ENA                             0
#define VTSS_LEN_DEV1G_DEV_LB_CFG_TAXI_HOST_LB_ENA                             1

#define VTSS_ADDR_DEV1G_DEV_DBG_CFG                                       0x0004
#define VTSS_OFF_DEV1G_DEV_DBG_CFG_PRE_CNT_OFLW_ID                            24
#define VTSS_LEN_DEV1G_DEV_DBG_CFG_PRE_CNT_OFLW_ID                             6
#define VTSS_OFF_DEV1G_DEV_DBG_CFG_TX_MAX_FILL_LVL                            16
#define VTSS_LEN_DEV1G_DEV_DBG_CFG_TX_MAX_FILL_LVL                             5
#define VTSS_OFF_DEV1G_DEV_DBG_CFG_TX_MAX_FILL_LVL_CLR                        12
#define VTSS_LEN_DEV1G_DEV_DBG_CFG_TX_MAX_FILL_LVL_CLR                         1
#define VTSS_OFF_DEV1G_DEV_DBG_CFG_BACKOFF_CNT_ENA                             8
#define VTSS_LEN_DEV1G_DEV_DBG_CFG_BACKOFF_CNT_ENA                             1
#define VTSS_OFF_DEV1G_DEV_DBG_CFG_TX_BUF_HIGH_WM                              0
#define VTSS_LEN_DEV1G_DEV_DBG_CFG_TX_BUF_HIGH_WM                              5

#define VTSS_ADDR_DEV1G_DEV_PORT_PROTECT                                  0x0005
#define VTSS_OFF_DEV1G_DEV_PORT_PROTECT_PORT_PROTECT_ID                        4
#define VTSS_LEN_DEV1G_DEV_PORT_PROTECT_PORT_PROTECT_ID                        3
#define VTSS_OFF_DEV1G_DEV_PORT_PROTECT_PORT_PROTECT_ENA                       0
#define VTSS_LEN_DEV1G_DEV_PORT_PROTECT_PORT_PROTECT_ENA                       1

#define VTSS_ADDR_DEV1G_DEV_SPARE                                         0x0006

#define VTSS_ADDR_DEV1G_FX100_RX_STAT                                     0x0007
#define VTSS_OFF_DEV1G_FX100_RX_STAT_ESD_ERR_STICKY                            9
#define VTSS_LEN_DEV1G_FX100_RX_STAT_ESD_ERR_STICKY                            1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_SSD_ERR_STICKY                            8
#define VTSS_LEN_DEV1G_FX100_RX_STAT_SSD_ERR_STICKY                            1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_RCV_ERR_STICKY                            7
#define VTSS_LEN_DEV1G_FX100_RX_STAT_RCV_ERR_STICKY                            1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_REMOTE_XMIT_ERR_STICKY                    6
#define VTSS_LEN_DEV1G_FX100_RX_STAT_REMOTE_XMIT_ERR_STICKY                    1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_LINK_DOWN                                 4
#define VTSS_LEN_DEV1G_FX100_RX_STAT_LINK_DOWN                                 1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_LOCK_ERR_STICKY                           2
#define VTSS_LEN_DEV1G_FX100_RX_STAT_LOCK_ERR_STICKY                           1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_SYMBOL_LOCK_DETECT                        1
#define VTSS_LEN_DEV1G_FX100_RX_STAT_SYMBOL_LOCK_DETECT                        1
#define VTSS_OFF_DEV1G_FX100_RX_STAT_SYMBOL_LOCK_ERR_DETECT                    0
#define VTSS_LEN_DEV1G_FX100_RX_STAT_SYMBOL_LOCK_ERR_DETECT                    1

#define VTSS_ADDR_DEV1G_FX100_CTL                                         0x0008
#define VTSS_OFF_DEV1G_FX100_CTL_LINKDOWN_TIMER_MSB                           12
#define VTSS_LEN_DEV1G_FX100_CTL_LINKDOWN_TIMER_MSB                            3
#define VTSS_OFF_DEV1G_FX100_CTL_SLOW_DOWN_LINKDOWN_TIMER                     10
#define VTSS_LEN_DEV1G_FX100_CTL_SLOW_DOWN_LINKDOWN_TIMER                      2
#define VTSS_OFF_DEV1G_FX100_CTL_SHORTEN_RX_LINKUP_COUNTER                     8
#define VTSS_LEN_DEV1G_FX100_CTL_SHORTEN_RX_LINKUP_COUNTER                     1
#define VTSS_OFF_DEV1G_FX100_CTL_LINK_CONTROL                                  5
#define VTSS_LEN_DEV1G_FX100_CTL_LINK_CONTROL                                  1
#define VTSS_OFF_DEV1G_FX100_CTL_LOOPBACK                                      4
#define VTSS_LEN_DEV1G_FX100_CTL_LOOPBACK                                      1
#define VTSS_OFF_DEV1G_FX100_CTL_LINK_HYST_TIMER                               0
#define VTSS_LEN_DEV1G_FX100_CTL_LINK_HYST_TIMER                               4

#define VTSS_ADDR_DEV1G_FX100_RX_INT                                      0x0009
#define VTSS_OFF_DEV1G_FX100_RX_INT_FALSE_CARRIER_STICKY                       2
#define VTSS_LEN_DEV1G_FX100_RX_INT_FALSE_CARRIER_STICKY                       1
#define VTSS_OFF_DEV1G_FX100_RX_INT_RXER_STICKY                                1
#define VTSS_LEN_DEV1G_FX100_RX_INT_RXER_STICKY                                1
#define VTSS_OFF_DEV1G_FX100_RX_INT_SYMBOL_ERR_STICKY                          0
#define VTSS_LEN_DEV1G_FX100_RX_INT_SYMBOL_ERR_STICKY                          1

#define VTSS_ADDR_DEV1G_FX100_RX_INT_MASK                                 0x000a
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_LOWER_BOUNDS_EXCEEDED_INT_MASK       17
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_LOWER_BOUNDS_EXCEEDED_INT_MASK        1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_UPPER_BOUNDS_EXCEEDED_INT_MASK       16
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_UPPER_BOUNDS_EXCEEDED_INT_MASK        1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_ESD_ERR_INT_MASK                      9
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_ESD_ERR_INT_MASK                      1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_SSD_ERR_INT_MASK                      8
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_SSD_ERR_INT_MASK                      1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_RCV_ERR_INT_MASK                      7
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_RCV_ERR_INT_MASK                      1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_REMOTE_XMIT_ERR_INT_MASK              6
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_REMOTE_XMIT_ERR_INT_MASK              1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_LOCK_ERR_INT_MASK                     5
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_LOCK_ERR_INT_MASK                     1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_FALSE_CARRIER_INT_MASK                2
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_FALSE_CARRIER_INT_MASK                1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_RXER_INT_MASK                         1
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_RXER_INT_MASK                         1
#define VTSS_OFF_DEV1G_FX100_RX_INT_MASK_SYMBOL_ERR_INT_MASK                   0
#define VTSS_LEN_DEV1G_FX100_RX_INT_MASK_SYMBOL_ERR_INT_MASK                   1

#define VTSS_ADDR_DEV1G_FX100_FALSE_CARRIER_CNT                           0x000b

#define VTSS_ADDR_DEV1G_FX100_RCV_ERR_CNT                                 0x000c

#define VTSS_ADDR_DEV1G_FX100_STICKY                                      0x000d
#define VTSS_OFF_DEV1G_FX100_STICKY_FX100_LINK_UP_STICKY                       1
#define VTSS_LEN_DEV1G_FX100_STICKY_FX100_LINK_UP_STICKY                       1
#define VTSS_OFF_DEV1G_FX100_STICKY_FX100_LINK_DOWN_STICKY                     0
#define VTSS_LEN_DEV1G_FX100_STICKY_FX100_LINK_DOWN_STICKY                     1

#define VTSS_ADDR_DEV1G_FX100_INT_MASK                                    0x000e
#define VTSS_OFF_DEV1G_FX100_INT_MASK_FX100_LINK_UP_INT_MASK                   1
#define VTSS_LEN_DEV1G_FX100_INT_MASK_FX100_LINK_UP_INT_MASK                   1
#define VTSS_OFF_DEV1G_FX100_INT_MASK_FX100_LINK_DOWN_INT_MASK                 0
#define VTSS_LEN_DEV1G_FX100_INT_MASK_FX100_LINK_DOWN_INT_MASK                 1

#define VTSS_ADDR_DEV1G_FX100_EYE_WINDOW_CFG                              0x000f
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_SERDES_FX100_MODE                 25
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_SERDES_FX100_MODE                  1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_MANUAL_ENABLE_SERDES_FX100_MODE     24
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_MANUAL_ENABLE_SERDES_FX100_MODE      1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_USE_DK_SHIM_DATA                  22
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_USE_DK_SHIM_DATA                   1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_KICK_MODE                         20
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_KICK_MODE                          2
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_SWAP_CP_MD                        19
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_SWAP_CP_MD                         1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_SIMPLE_MODE                       18
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_SIMPLE_MODE                        1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_MD_BYP                            17
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_MD_BYP                             1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_CP_BYP                            16
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_CP_BYP                             1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_FORCED_EYE_CENTER_POSITION         5
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_FORCED_EYE_CENTER_POSITION         6
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_FORCE_EYE_CENTER                   4
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_FORCE_EYE_CENTER                   1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_EYE_CENTER_HYST_CFG                2
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_EYE_CENTER_HYST_CFG                2
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_AUTO_RESET_DISABLE                 1
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_AUTO_RESET_DISABLE                 1
#define VTSS_OFF_DEV1G_FX100_EYE_WINDOW_CFG_RESET_EYE_WINDOW_CENTER            0
#define VTSS_LEN_DEV1G_FX100_EYE_WINDOW_CFG_RESET_EYE_WINDOW_CENTER            1

#define VTSS_ADDR_DEV1G_FX100_EYE_LOGIC_STATUS                            0x0010
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_TRANSITIONS_CROSSED_STICKY      20
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_TRANSITIONS_CROSSED_STICKY       1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_TRANSITIONS_TOO_CLOSE_STICKY     19
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_TRANSITIONS_TOO_CLOSE_STICKY      1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION_TOO_HIGH_STICKY     18
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION_TOO_HIGH_STICKY      1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION                11
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION                 6
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_BOUNDS_EXCEEDED_STICKY     10
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_BOUNDS_EXCEEDED_STICKY      1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION_VALID           9
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_LOWER_TRANSITION_VALID           1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION_TOO_LOW_STICKY      8
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION_TOO_LOW_STICKY      1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION                 2
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION                 6
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_BOUNDS_EXCEEDED_STICKY      1
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_BOUNDS_EXCEEDED_STICKY      1
#define VTSS_OFF_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION_VALID           0
#define VTSS_LEN_DEV1G_FX100_EYE_LOGIC_STATUS_UPPER_TRANSITION_VALID           1

#define VTSS_ADDR_DEV1G_FX100_BIT_TRANS                                   0x0011

#define VTSS_ADDR_DEV1G_FX100_BIT_TRANS_MSB                               0x0012
#define VTSS_OFF_DEV1G_FX100_BIT_TRANS_MSB_BIT_TRANS_MSB                       0
#define VTSS_LEN_DEV1G_FX100_BIT_TRANS_MSB_BIT_TRANS_MSB                       8

#define VTSS_ADDR_DEV1G_MAC_ENA_CFG                                       0x0013
#define VTSS_OFF_DEV1G_MAC_ENA_CFG_RX_ENA                                      4
#define VTSS_LEN_DEV1G_MAC_ENA_CFG_RX_ENA                                      1
#define VTSS_OFF_DEV1G_MAC_ENA_CFG_TX_ENA                                      0
#define VTSS_LEN_DEV1G_MAC_ENA_CFG_TX_ENA                                      1

#define VTSS_ADDR_DEV1G_MAC_MODE_CFG                                      0x0014
#define VTSS_OFF_DEV1G_MAC_MODE_CFG_GIGA_MODE_ENA                              4
#define VTSS_LEN_DEV1G_MAC_MODE_CFG_GIGA_MODE_ENA                              1
#define VTSS_OFF_DEV1G_MAC_MODE_CFG_FDX_ENA                                    0
#define VTSS_LEN_DEV1G_MAC_MODE_CFG_FDX_ENA                                    1

#define VTSS_ADDR_DEV1G_MAC_MAXLEN_CFG                                    0x0015
#define VTSS_OFF_DEV1G_MAC_MAXLEN_CFG_MAX_LEN                                  0
#define VTSS_LEN_DEV1G_MAC_MAXLEN_CFG_MAX_LEN                                 16

#define VTSS_ADDR_DEV1G_MAC_TAGS_CFG                                      0x0016
#define VTSS_OFF_DEV1G_MAC_TAGS_CFG_TAG_ID                                    16
#define VTSS_LEN_DEV1G_MAC_TAGS_CFG_TAG_ID                                    16
#define VTSS_OFF_DEV1G_MAC_TAGS_CFG_PB_ENA                                     1
#define VTSS_LEN_DEV1G_MAC_TAGS_CFG_PB_ENA                                     1
#define VTSS_OFF_DEV1G_MAC_TAGS_CFG_VLAN_AWR_ENA                               0
#define VTSS_LEN_DEV1G_MAC_TAGS_CFG_VLAN_AWR_ENA                               1

#define VTSS_ADDR_DEV1G_MAC_ADV_CHK_CFG                                   0x0017
#define VTSS_OFF_DEV1G_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS                  28
#define VTSS_LEN_DEV1G_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS                   1
#define VTSS_OFF_DEV1G_MAC_ADV_CHK_CFG_LEN_DROP_ENA                            0
#define VTSS_LEN_DEV1G_MAC_ADV_CHK_CFG_LEN_DROP_ENA                            1

#define VTSS_ADDR_DEV1G_MAC_IFG_CFG                                       0x0018
#define VTSS_OFF_DEV1G_MAC_IFG_CFG_TX_IFG                                      8
#define VTSS_LEN_DEV1G_MAC_IFG_CFG_TX_IFG                                      5
#define VTSS_OFF_DEV1G_MAC_IFG_CFG_RX_IFG2                                     4
#define VTSS_LEN_DEV1G_MAC_IFG_CFG_RX_IFG2                                     4
#define VTSS_OFF_DEV1G_MAC_IFG_CFG_RX_IFG1                                     0
#define VTSS_LEN_DEV1G_MAC_IFG_CFG_RX_IFG1                                     4

#define VTSS_ADDR_DEV1G_MAC_HDX_CFG                                       0x0019
#define VTSS_OFF_DEV1G_MAC_HDX_CFG_SEED                                       16
#define VTSS_LEN_DEV1G_MAC_HDX_CFG_SEED                                        8
#define VTSS_OFF_DEV1G_MAC_HDX_CFG_SEED_LOAD                                  12
#define VTSS_LEN_DEV1G_MAC_HDX_CFG_SEED_LOAD                                   1
#define VTSS_OFF_DEV1G_MAC_HDX_CFG_TX_FAST_HDX_FIX_ENA                         9
#define VTSS_LEN_DEV1G_MAC_HDX_CFG_TX_FAST_HDX_FIX_ENA                         1
#define VTSS_OFF_DEV1G_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA                     8
#define VTSS_LEN_DEV1G_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA                     1
#define VTSS_OFF_DEV1G_MAC_HDX_CFG_LATE_COL_POS                                0
#define VTSS_LEN_DEV1G_MAC_HDX_CFG_LATE_COL_POS                                7

#define VTSS_ADDR_DEV1G_MAC_STICKY                                        0x001a
#define VTSS_OFF_DEV1G_MAC_STICKY_RX_IPG_SHRINK_STICKY                         9
#define VTSS_LEN_DEV1G_MAC_STICKY_RX_IPG_SHRINK_STICKY                         1
#define VTSS_OFF_DEV1G_MAC_STICKY_RX_PREAM_SHRINK_STICKY                       8
#define VTSS_LEN_DEV1G_MAC_STICKY_RX_PREAM_SHRINK_STICKY                       1
#define VTSS_OFF_DEV1G_MAC_STICKY_RX_JUNK_STICKY                               5
#define VTSS_LEN_DEV1G_MAC_STICKY_RX_JUNK_STICKY                               1
#define VTSS_OFF_DEV1G_MAC_STICKY_TX_RETRANSMIT_STICKY                         4
#define VTSS_LEN_DEV1G_MAC_STICKY_TX_RETRANSMIT_STICKY                         1
#define VTSS_OFF_DEV1G_MAC_STICKY_TX_JAM_STICKY                                3
#define VTSS_LEN_DEV1G_MAC_STICKY_TX_JAM_STICKY                                1
#define VTSS_OFF_DEV1G_MAC_STICKY_TX_ABORT_STICKY                              0
#define VTSS_LEN_DEV1G_MAC_STICKY_TX_ABORT_STICKY                              1

#define VTSS_ADDR_DEV1G_MAC_INT_MASK                                      0x001b
#define VTSS_OFF_DEV1G_MAC_INT_MASK_RX_IPG_SHRINK_INT_MASK                     9
#define VTSS_LEN_DEV1G_MAC_INT_MASK_RX_IPG_SHRINK_INT_MASK                     1
#define VTSS_OFF_DEV1G_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK                   8
#define VTSS_LEN_DEV1G_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK                   1
#define VTSS_OFF_DEV1G_MAC_INT_MASK_RX_JUNK_INT_MASK                           5
#define VTSS_LEN_DEV1G_MAC_INT_MASK_RX_JUNK_INT_MASK                           1
#define VTSS_OFF_DEV1G_MAC_INT_MASK_TX_RETRANSMIT_INT_MASK                     4
#define VTSS_LEN_DEV1G_MAC_INT_MASK_TX_RETRANSMIT_INT_MASK                     1
#define VTSS_OFF_DEV1G_MAC_INT_MASK_TX_JAM_INT_MASK                            3
#define VTSS_LEN_DEV1G_MAC_INT_MASK_TX_JAM_INT_MASK                            1
#define VTSS_OFF_DEV1G_MAC_INT_MASK_TX_ABORT_INT_MASK                          0
#define VTSS_LEN_DEV1G_MAC_INT_MASK_TX_ABORT_INT_MASK                          1

#define VTSS_ADDR_DEV1G_PCS1G_MODE_CFG                                    0x001c
#define VTSS_OFF_DEV1G_PCS1G_MODE_CFG_DISABLE_IPGFIX                           1
#define VTSS_LEN_DEV1G_PCS1G_MODE_CFG_DISABLE_IPGFIX                           1
#define VTSS_OFF_DEV1G_PCS1G_MODE_CFG_SGMII_MODE_ENA                           0
#define VTSS_LEN_DEV1G_PCS1G_MODE_CFG_SGMII_MODE_ENA                           1

#define VTSS_ADDR_DEV1G_PCS1G_SD_CFG                                      0x001d
#define VTSS_OFF_DEV1G_PCS1G_SD_CFG_SD_POL                                     4
#define VTSS_LEN_DEV1G_PCS1G_SD_CFG_SD_POL                                     1
#define VTSS_OFF_DEV1G_PCS1G_SD_CFG_SD_ENA                                     0
#define VTSS_LEN_DEV1G_PCS1G_SD_CFG_SD_ENA                                     1

#define VTSS_ADDR_DEV1G_PCS1G_ANEG_CFG                                    0x001e
#define VTSS_OFF_DEV1G_PCS1G_ANEG_CFG_DAR                                     16
#define VTSS_LEN_DEV1G_PCS1G_ANEG_CFG_DAR                                     16
#define VTSS_OFF_DEV1G_PCS1G_ANEG_CFG_SW_RESOLVE_ENA                           8
#define VTSS_LEN_DEV1G_PCS1G_ANEG_CFG_SW_RESOLVE_ENA                           1
#define VTSS_OFF_DEV1G_PCS1G_ANEG_CFG_ANEG_ENA                                 4
#define VTSS_LEN_DEV1G_PCS1G_ANEG_CFG_ANEG_ENA                                 1
#define VTSS_OFF_DEV1G_PCS1G_ANEG_CFG_ANEG_RESTART                             0
#define VTSS_LEN_DEV1G_PCS1G_ANEG_CFG_ANEG_RESTART                             1

#define VTSS_ADDR_DEV1G_PCS1G_LB_CFG                                      0x001f
#define VTSS_OFF_DEV1G_PCS1G_LB_CFG_TBI_HOST_LB_ENA                            0
#define VTSS_LEN_DEV1G_PCS1G_LB_CFG_TBI_HOST_LB_ENA                            1

#define VTSS_ADDR_DEV1G_PCS1G_DBG_CFG                                     0x0020
#define VTSS_OFF_DEV1G_PCS1G_DBG_CFG_UDLT                                      0
#define VTSS_LEN_DEV1G_PCS1G_DBG_CFG_UDLT                                      1

#define VTSS_ADDR_DEV1G_PCS1G_ANEG_STATUS                                 0x0021
#define VTSS_OFF_DEV1G_PCS1G_ANEG_STATUS_LPA                                  16
#define VTSS_LEN_DEV1G_PCS1G_ANEG_STATUS_LPA                                  16
#define VTSS_OFF_DEV1G_PCS1G_ANEG_STATUS_PR                                    4
#define VTSS_LEN_DEV1G_PCS1G_ANEG_STATUS_PR                                    1
#define VTSS_OFF_DEV1G_PCS1G_ANEG_STATUS_ANC                                   0
#define VTSS_LEN_DEV1G_PCS1G_ANEG_STATUS_ANC                                   1

#define VTSS_ADDR_DEV1G_PCS1G_LINK_STATUS                                 0x0022
#define VTSS_OFF_DEV1G_PCS1G_LINK_STATUS_LINK_DOWN                             1
#define VTSS_LEN_DEV1G_PCS1G_LINK_STATUS_LINK_DOWN                             1
#define VTSS_OFF_DEV1G_PCS1G_LINK_STATUS_SIGNAL_DETECT                         0
#define VTSS_LEN_DEV1G_PCS1G_LINK_STATUS_SIGNAL_DETECT                         1

#define VTSS_ADDR_DEV1G_PCS1G_LINK_DOWN_CNT                               0x0023
#define VTSS_OFF_DEV1G_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT                       0
#define VTSS_LEN_DEV1G_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT                       8

#define VTSS_ADDR_DEV1G_PCS1G_STICKY                                      0x0024
#define VTSS_OFF_DEV1G_PCS1G_STICKY_LINK_UP_STICKY                             5
#define VTSS_LEN_DEV1G_PCS1G_STICKY_LINK_UP_STICKY                             1
#define VTSS_OFF_DEV1G_PCS1G_STICKY_LINK_DOWN_STICKY                           4
#define VTSS_LEN_DEV1G_PCS1G_STICKY_LINK_DOWN_STICKY                           1
#define VTSS_OFF_DEV1G_PCS1G_STICKY_OUT_OF_SYNC_STICKY                         0
#define VTSS_LEN_DEV1G_PCS1G_STICKY_OUT_OF_SYNC_STICKY                         1

#define VTSS_ADDR_DEV1G_PCS1G_INT_MASK                                    0x0025
#define VTSS_OFF_DEV1G_PCS1G_INT_MASK_LINK_UP_INT_MASK                         5
#define VTSS_LEN_DEV1G_PCS1G_INT_MASK_LINK_UP_INT_MASK                         1
#define VTSS_OFF_DEV1G_PCS1G_INT_MASK_LINK_DOWN_INT_MASK                       4
#define VTSS_LEN_DEV1G_PCS1G_INT_MASK_LINK_DOWN_INT_MASK                       1
#define VTSS_OFF_DEV1G_PCS1G_INT_MASK_OUT_OF_SYNC_INT_MASK                     0
#define VTSS_LEN_DEV1G_PCS1G_INT_MASK_OUT_OF_SYNC_INT_MASK                     1

#define VTSS_ADDR_DEV1G_PCS1G_DBG_STATUS                                  0x0026
#define VTSS_OFF_DEV1G_PCS1G_DBG_STATUS_XMIT_MODE                             12
#define VTSS_LEN_DEV1G_PCS1G_DBG_STATUS_XMIT_MODE                              2

#define VTSS_ADDR_DEV1G_PCS1G_TSTPAT_MODE_CFG                             0x0027
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_MODE_CFG_JTP_SEL                           0
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_MODE_CFG_JTP_SEL                           2

#define VTSS_ADDR_DEV1G_PCS1G_TSTPAT_STATUS                               0x0028
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_STATUS_JTP_ERR                             4
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_STATUS_JTP_ERR                             1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_STATUS_JTP_LOCK                            0
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_STATUS_JTP_LOCK                            1

#define VTSS_ADDR_DEV1G_CMU_CFG                                           0x0029
#define VTSS_OFF_DEV1G_CMU_CFG_LOWPASS                                        12
#define VTSS_LEN_DEV1G_CMU_CFG_LOWPASS                                         2
#define VTSS_OFF_DEV1G_CMU_CFG_CHARGEPUMP                                     10
#define VTSS_LEN_DEV1G_CMU_CFG_CHARGEPUMP                                      2
#define VTSS_OFF_DEV1G_CMU_CFG_FREQ                                            8
#define VTSS_LEN_DEV1G_CMU_CFG_FREQ                                            2

#define VTSS_ADDR_DEV1G_CMU_TEST_CFG                                      0x002a
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_TESTSEL                                   24
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_TESTSEL                                    4
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_LD_FRC_ZERO                               15
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_LD_FRC_ZERO                                1
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_LD_FRC_ONE                                14
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_LD_FRC_ONE                                 1
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_LD_PPM_SEL                                12
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_LD_PPM_SEL                                 2
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_BIT_ZERO                                   1
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_BIT_ZERO                                   1
#define VTSS_OFF_DEV1G_CMU_TEST_CFG_BIT_NINE                                   0
#define VTSS_LEN_DEV1G_CMU_TEST_CFG_BIT_NINE                                   1

#define VTSS_ADDR_DEV1G_LANE_CFG                                          0x002b
#define VTSS_OFF_DEV1G_LANE_CFG_FASTLOCK_ENA                                  24
#define VTSS_LEN_DEV1G_LANE_CFG_FASTLOCK_ENA                                   1
#define VTSS_OFF_DEV1G_LANE_CFG_FASTLOCKON                                    10
#define VTSS_LEN_DEV1G_LANE_CFG_FASTLOCKON                                     3
#define VTSS_OFF_DEV1G_LANE_CFG_FASTLOCKOFF                                    7
#define VTSS_LEN_DEV1G_LANE_CFG_FASTLOCKOFF                                    3
#define VTSS_OFF_DEV1G_LANE_CFG_LANE_ENA                                       0
#define VTSS_LEN_DEV1G_LANE_CFG_LANE_ENA                                       1

#define VTSS_ADDR_DEV1G_LANE_TEST_CFG                                     0x002c
#define VTSS_OFF_DEV1G_LANE_TEST_CFG_CDR_DIS                                   6
#define VTSS_LEN_DEV1G_LANE_TEST_CFG_CDR_DIS                                   1
#define VTSS_OFF_DEV1G_LANE_TEST_CFG_EQLOOP_ENA                                2
#define VTSS_LEN_DEV1G_LANE_TEST_CFG_EQLOOP_ENA                                1

#define VTSS_ADDR_DEV1G_OB_CFG                                            0x002d
#define VTSS_OFF_DEV1G_OB_CFG_LVPECL_DRIVE_DIS                                 0
#define VTSS_LEN_DEV1G_OB_CFG_LVPECL_DRIVE_DIS                                 1

#define VTSS_ADDR_DEV1G_IB_CFG                                            0x002e
#define VTSS_OFF_DEV1G_IB_CFG_CMT_ENA                                         20
#define VTSS_LEN_DEV1G_IB_CFG_CMT_ENA                                          1
#define VTSS_OFF_DEV1G_IB_CFG_LOS_THRES_HIGH                                  16
#define VTSS_LEN_DEV1G_IB_CFG_LOS_THRES_HIGH                                   2
#define VTSS_OFF_DEV1G_IB_CFG_LOS_THRES_LOW                                   14
#define VTSS_LEN_DEV1G_IB_CFG_LOS_THRES_LOW                                    2
#define VTSS_OFF_DEV1G_IB_CFG_GAIN_CTRL                                        8
#define VTSS_LEN_DEV1G_IB_CFG_GAIN_CTRL                                        4
#define VTSS_OFF_DEV1G_IB_CFG_DES_FRC_LOW                                      5
#define VTSS_LEN_DEV1G_IB_CFG_DES_FRC_LOW                                      1
#define VTSS_OFF_DEV1G_IB_CFG_IB_DES_DIS                                       4
#define VTSS_LEN_DEV1G_IB_CFG_IB_DES_DIS                                       1
#define VTSS_OFF_DEV1G_IB_CFG_HF_COMP_CTRL                                     0
#define VTSS_LEN_DEV1G_IB_CFG_HF_COMP_CTRL                                     3

#define VTSS_ADDR_DEV1G_LANE_STATUS                                       0x002f
#define VTSS_OFF_DEV1G_LANE_STATUS_FASTLOCKACTIVE                              7
#define VTSS_LEN_DEV1G_LANE_STATUS_FASTLOCKACTIVE                              1

#define VTSS_ADDR_DEV1G_IB_STATUS                                         0x0030
#define VTSS_OFF_DEV1G_IB_STATUS_IS_LOW_LVL_STATUS                             2
#define VTSS_LEN_DEV1G_IB_STATUS_IS_LOW_LVL_STATUS                             1
#define VTSS_OFF_DEV1G_IB_STATUS_IS_HIGH_LVL_STATUS                            1
#define VTSS_LEN_DEV1G_IB_STATUS_IS_HIGH_LVL_STATUS                            1

#define VTSS_ADDR_DEV1G_PCS1G_TSTPAT_CTRL_CFG                             0x0031
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PATTERN_MATCH                    16
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PATTERN_MATCH                     1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP                    13
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP                     1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PRBS_POLY_INV                    12
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_PRBS_POLY_INV                     1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_CHK_ENA                        9
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_CHK_ENA                        1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_CHK_SEL                        5
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_CHK_SEL                        4
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_GEN_ENA                        4
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_GEN_ENA                        1
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_GEN_SEL                        0
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_CTRL_CFG_VT_GEN_SEL                        4

#define VTSS_ADDR_DEV1G_PCS1G_TSTPAT_PRPAT_CFG                            0x0032
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_PRPAT_CFG_CHK_PAT                         16
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_PRPAT_CFG_CHK_PAT                         10
#define VTSS_OFF_DEV1G_PCS1G_TSTPAT_PRPAT_CFG_GEN_PAT                          0
#define VTSS_LEN_DEV1G_PCS1G_TSTPAT_PRPAT_CFG_GEN_PAT                         10

#define VTSS_ADDR_DEV1G_PCS1G_TSTPAT_ERR_CNT                              0x0033

/*********************************************************************** 
 * Target DEV10G
 * Device 10G
 ***********************************************************************/
#define VTSS_ADDR_DEV10G_PCS1G_MODE_CFG                                   0x0000
#define VTSS_OFF_DEV10G_PCS1G_MODE_CFG_DISABLE_IPGFIX                          1
#define VTSS_LEN_DEV10G_PCS1G_MODE_CFG_DISABLE_IPGFIX                          1
#define VTSS_OFF_DEV10G_PCS1G_MODE_CFG_SGMII_MODE_ENA                          0
#define VTSS_LEN_DEV10G_PCS1G_MODE_CFG_SGMII_MODE_ENA                          1

#define VTSS_ADDR_DEV10G_PCS1G_SD_CFG                                     0x0001
#define VTSS_OFF_DEV10G_PCS1G_SD_CFG_SD_POL                                    4
#define VTSS_LEN_DEV10G_PCS1G_SD_CFG_SD_POL                                    1
#define VTSS_OFF_DEV10G_PCS1G_SD_CFG_SD_ENA                                    0
#define VTSS_LEN_DEV10G_PCS1G_SD_CFG_SD_ENA                                    1

#define VTSS_ADDR_DEV10G_PCS1G_ANEG_CFG                                   0x0002
#define VTSS_OFF_DEV10G_PCS1G_ANEG_CFG_DAR                                    16
#define VTSS_LEN_DEV10G_PCS1G_ANEG_CFG_DAR                                    16
#define VTSS_OFF_DEV10G_PCS1G_ANEG_CFG_SW_RESOLVE_ENA                          8
#define VTSS_LEN_DEV10G_PCS1G_ANEG_CFG_SW_RESOLVE_ENA                          1
#define VTSS_OFF_DEV10G_PCS1G_ANEG_CFG_ANEG_ENA                                4
#define VTSS_LEN_DEV10G_PCS1G_ANEG_CFG_ANEG_ENA                                1
#define VTSS_OFF_DEV10G_PCS1G_ANEG_CFG_ANEG_RESTART                            0
#define VTSS_LEN_DEV10G_PCS1G_ANEG_CFG_ANEG_RESTART                            1

#define VTSS_ADDR_DEV10G_PCS1G_PMA_CFG                                    0x0003
#define VTSS_OFF_DEV10G_PCS1G_PMA_CFG_PMA_LOCK_FORCE_VAL                       4
#define VTSS_LEN_DEV10G_PCS1G_PMA_CFG_PMA_LOCK_FORCE_VAL                       1
#define VTSS_OFF_DEV10G_PCS1G_PMA_CFG_AUTO_PMA_LOCK_ENA                        0
#define VTSS_LEN_DEV10G_PCS1G_PMA_CFG_AUTO_PMA_LOCK_ENA                        1

#define VTSS_ADDR_DEV10G_PCS1G_LB_CFG                                     0x0004
#define VTSS_OFF_DEV10G_PCS1G_LB_CFG_TBI_HOST_LB_ENA                           0
#define VTSS_LEN_DEV10G_PCS1G_LB_CFG_TBI_HOST_LB_ENA                           1

#define VTSS_ADDR_DEV10G_PCS1G_DBG_CFG                                    0x0005
#define VTSS_OFF_DEV10G_PCS1G_DBG_CFG_UDLT                                     0
#define VTSS_LEN_DEV10G_PCS1G_DBG_CFG_UDLT                                     1

#define VTSS_ADDR_DEV10G_PCS1G_ANEG_STATUS                                0x0006
#define VTSS_OFF_DEV10G_PCS1G_ANEG_STATUS_LPA                                 16
#define VTSS_LEN_DEV10G_PCS1G_ANEG_STATUS_LPA                                 16
#define VTSS_OFF_DEV10G_PCS1G_ANEG_STATUS_PR                                   4
#define VTSS_LEN_DEV10G_PCS1G_ANEG_STATUS_PR                                   1
#define VTSS_OFF_DEV10G_PCS1G_ANEG_STATUS_ANC                                  0
#define VTSS_LEN_DEV10G_PCS1G_ANEG_STATUS_ANC                                  1

#define VTSS_ADDR_DEV10G_PCS1G_LINK_STATUS                                0x0007
#define VTSS_OFF_DEV10G_PCS1G_LINK_STATUS_SIGNAL_DETECT                        0
#define VTSS_LEN_DEV10G_PCS1G_LINK_STATUS_SIGNAL_DETECT                        1

#define VTSS_ADDR_DEV10G_PCS1G_LINK_DOWN_CNT                              0x0008
#define VTSS_OFF_DEV10G_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT                      0
#define VTSS_LEN_DEV10G_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT                      8

#define VTSS_ADDR_DEV10G_PCS1G_STICKY                                     0x0009
#define VTSS_OFF_DEV10G_PCS1G_STICKY_LINK_DOWN_STICKY                          4
#define VTSS_LEN_DEV10G_PCS1G_STICKY_LINK_DOWN_STICKY                          1
#define VTSS_OFF_DEV10G_PCS1G_STICKY_OUT_OF_SYNC_STICKY                        0
#define VTSS_LEN_DEV10G_PCS1G_STICKY_OUT_OF_SYNC_STICKY                        1

#define VTSS_ADDR_DEV10G_PCS1G_DBG_STATUS                                 0x000a
#define VTSS_OFF_DEV10G_PCS1G_DBG_STATUS_XMIT_MODE                            12
#define VTSS_LEN_DEV10G_PCS1G_DBG_STATUS_XMIT_MODE                             2

#define VTSS_ADDR_DEV10G_PCS1G_TSTPAT_MODE_CFG                            0x000b
#define VTSS_OFF_DEV10G_PCS1G_TSTPAT_MODE_CFG_JTP_SEL                          0
#define VTSS_LEN_DEV10G_PCS1G_TSTPAT_MODE_CFG_JTP_SEL                          2

#define VTSS_ADDR_DEV10G_PCS1G_TSTPAT_STATUS                              0x000c
#define VTSS_OFF_DEV10G_PCS1G_TSTPAT_STATUS_JTP_ERR                            4
#define VTSS_LEN_DEV10G_PCS1G_TSTPAT_STATUS_JTP_ERR                            1
#define VTSS_OFF_DEV10G_PCS1G_TSTPAT_STATUS_JTP_LOCK                           0
#define VTSS_LEN_DEV10G_PCS1G_TSTPAT_STATUS_JTP_LOCK                           1

#define VTSS_ADDR_DEV10G_MAC_ENA_CFG                                      0x000d
#define VTSS_OFF_DEV10G_MAC_ENA_CFG_USE_LEADING_EDGE_DETECT                   28
#define VTSS_LEN_DEV10G_MAC_ENA_CFG_USE_LEADING_EDGE_DETECT                    1
#define VTSS_OFF_DEV10G_MAC_ENA_CFG_RX_ENA                                     4
#define VTSS_LEN_DEV10G_MAC_ENA_CFG_RX_ENA                                     1
#define VTSS_OFF_DEV10G_MAC_ENA_CFG_TX_ENA                                     0
#define VTSS_LEN_DEV10G_MAC_ENA_CFG_TX_ENA                                     1

#define VTSS_ADDR_DEV10G_MAC_MODE_CFG                                     0x000e
#define VTSS_OFF_DEV10G_MAC_MODE_CFG_XGMII_GEN_MODE_ENA                        4
#define VTSS_LEN_DEV10G_MAC_MODE_CFG_XGMII_GEN_MODE_ENA                        1
#define VTSS_OFF_DEV10G_MAC_MODE_CFG_PACE_MODE_ENA                             0
#define VTSS_LEN_DEV10G_MAC_MODE_CFG_PACE_MODE_ENA                             1

#define VTSS_ADDR_DEV10G_MAC_MAXLEN_CFG                                   0x000f
#define VTSS_OFF_DEV10G_MAC_MAXLEN_CFG_MAX_LEN                                 0
#define VTSS_LEN_DEV10G_MAC_MAXLEN_CFG_MAX_LEN                                16

#define VTSS_ADDR_DEV10G_MAC_TAGS_CFG                                     0x0010
#define VTSS_OFF_DEV10G_MAC_TAGS_CFG_TAG_ID                                   16
#define VTSS_LEN_DEV10G_MAC_TAGS_CFG_TAG_ID                                   16
#define VTSS_OFF_DEV10G_MAC_TAGS_CFG_PB_ENA                                    4
#define VTSS_LEN_DEV10G_MAC_TAGS_CFG_PB_ENA                                    1
#define VTSS_OFF_DEV10G_MAC_TAGS_CFG_VLAN_AWR_ENA                              0
#define VTSS_LEN_DEV10G_MAC_TAGS_CFG_VLAN_AWR_ENA                              1

#define VTSS_ADDR_DEV10G_MAC_ADV_CHK_CFG                                  0x0011
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS                 28
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS                  1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA                       24
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA                        1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA                       20
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA                        1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_SFD_CHK_ENA                           16
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_SFD_CHK_ENA                            1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS                       12
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS                        1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_PRM_CHK_ENA                            8
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_PRM_CHK_ENA                            1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_OOR_ERR_ENA                            4
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_OOR_ERR_ENA                            1
#define VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_INR_ERR_ENA                            0
#define VTSS_LEN_DEV10G_MAC_ADV_CHK_CFG_INR_ERR_ENA                            1

#define VTSS_ADDR_DEV10G_MAC_LFS_CFG                                      0x0012
#define VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_INH_TX                                 8
#define VTSS_LEN_DEV10G_MAC_LFS_CFG_LFS_INH_TX                                 1
#define VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_DIS_TX                                 4
#define VTSS_LEN_DEV10G_MAC_LFS_CFG_LFS_DIS_TX                                 1
#define VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_MODE_ENA                               0
#define VTSS_LEN_DEV10G_MAC_LFS_CFG_LFS_MODE_ENA                               1

#define VTSS_ADDR_DEV10G_MAC_LB_CFG                                       0x0013
#define VTSS_OFF_DEV10G_MAC_LB_CFG_XGMII_HOST_LB_ENA                           4
#define VTSS_LEN_DEV10G_MAC_LB_CFG_XGMII_HOST_LB_ENA                           1
#define VTSS_OFF_DEV10G_MAC_LB_CFG_XGMII_PHY_LB_ENA                            0
#define VTSS_LEN_DEV10G_MAC_LB_CFG_XGMII_PHY_LB_ENA                            1

#define VTSS_ADDR_DEV10G_MAC_VSTAX2_CFG                                   0x0014
#define VTSS_OFF_DEV10G_MAC_VSTAX2_CFG_VSTAX2_AWR_ENA                          0
#define VTSS_LEN_DEV10G_MAC_VSTAX2_CFG_VSTAX2_AWR_ENA                          1

#define VTSS_ADDR_DEV10G_MAC_TX_MONITOR                                   0x0015
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_LFS_LOCAL_FAULT                         1
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_LFS_LOCAL_FAULT                         1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_LFS_REMOTE_FAULT                        0
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_LFS_REMOTE_FAULT                        1

#define VTSS_ADDR_DEV10G_MAC_RX_LANE_STICKY_0                             0x0016
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_0_LANE3_STICKY                     24
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_0_LANE3_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_0_LANE2_STICKY                     16
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_0_LANE2_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_0_LANE1_STICKY                      8
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_0_LANE1_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_0_LANE0_STICKY                      0
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_0_LANE0_STICKY                      7

#define VTSS_ADDR_DEV10G_MAC_RX_LANE_STICKY_1                             0x0017
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_1_LANE7_STICKY                     24
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_1_LANE7_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_1_LANE6_STICKY                     16
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_1_LANE6_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_1_LANE5_STICKY                      8
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_1_LANE5_STICKY                      7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_STICKY_1_LANE4_STICKY                      0
#define VTSS_LEN_DEV10G_MAC_RX_LANE_STICKY_1_LANE4_STICKY                      7

#define VTSS_ADDR_DEV10G_MAC_TX_MONITOR_STICKY                            0x0018
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_STICKY_LOCAL_ERR_STATE_STICKY           4
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_STICKY_LOCAL_ERR_STATE_STICKY           1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_STICKY_REMOTE_ERR_STATE_STICKY          3
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_STICKY_REMOTE_ERR_STATE_STICKY          1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_STICKY_PAUSE_STATE_STICKY               2
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_STICKY_PAUSE_STATE_STICKY               1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_STICKY_IDLE_STATE_STICKY                1
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_STICKY_IDLE_STATE_STICKY                1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_STICKY_DIS_STATE_STICKY                 0
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_STICKY_DIS_STATE_STICKY                 1

#define VTSS_ADDR_DEV10G_MAC_STICKY                                       0x0019
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_IPG_SHRINK_STICKY                        9
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_IPG_SHRINK_STICKY                        1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_PREAM_SHRINK_STICKY                      8
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_PREAM_SHRINK_STICKY                      1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_PREAM_MISMATCH_STICKY                    7
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_PREAM_MISMATCH_STICKY                    1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_PREAM_ERR_STICKY                         6
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_PREAM_ERR_STICKY                         1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_NON_STD_PREAM_STICKY                     5
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_NON_STD_PREAM_STICKY                     1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_MPLS_MC_STICKY                           4
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_MPLS_MC_STICKY                           1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_MPLS_UC_STICKY                           3
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_MPLS_UC_STICKY                           1
#define VTSS_OFF_DEV10G_MAC_STICKY_RX_TAG_STICKY                               2
#define VTSS_LEN_DEV10G_MAC_STICKY_RX_TAG_STICKY                               1
#define VTSS_OFF_DEV10G_MAC_STICKY_TX_UFLW_STICKY                              1
#define VTSS_LEN_DEV10G_MAC_STICKY_TX_UFLW_STICKY                              1
#define VTSS_OFF_DEV10G_MAC_STICKY_TX_ABORT_STICKY                             0
#define VTSS_LEN_DEV10G_MAC_STICKY_TX_ABORT_STICKY                             1

#define VTSS_ADDR_DEV10G_MAC_RX_LANE_INT_MASK_0                           0x001a
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE3_INT_MASK                 24
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE3_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE2_INT_MASK                 16
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE2_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE1_INT_MASK                  8
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE1_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE0_INT_MASK                  0
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_0_LANE0_INT_MASK                  7

#define VTSS_ADDR_DEV10G_MAC_RX_LANE_INT_MASK_1                           0x001b
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE7_INT_MASK                 24
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE7_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE6_INT_MASK                 16
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE6_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE5_INT_MASK                  8
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE5_INT_MASK                  7
#define VTSS_OFF_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE4_INT_MASK                  0
#define VTSS_LEN_DEV10G_MAC_RX_LANE_INT_MASK_1_LANE4_INT_MASK                  7

#define VTSS_ADDR_DEV10G_MAC_TX_MONITOR_INT_MASK                          0x001c
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_INT_MASK_LOCAL_ERR_STATE_INT_MASK       4
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_INT_MASK_LOCAL_ERR_STATE_INT_MASK       1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_INT_MASK_REMOTE_ERR_STATE_INT_MASK      3
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_INT_MASK_REMOTE_ERR_STATE_INT_MASK      1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_INT_MASK_PAUSE_STATE_INT_MASK           2
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_INT_MASK_PAUSE_STATE_INT_MASK           1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_INT_MASK_IDLE_STATE_INT_MASK            1
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_INT_MASK_IDLE_STATE_INT_MASK            1
#define VTSS_OFF_DEV10G_MAC_TX_MONITOR_INT_MASK_DIS_STATE_INT_MASK             0
#define VTSS_LEN_DEV10G_MAC_TX_MONITOR_INT_MASK_DIS_STATE_INT_MASK             1

#define VTSS_ADDR_DEV10G_MAC_INT_MASK                                     0x001d
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_IPG_SHRINK_INT_MASK                    9
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_IPG_SHRINK_INT_MASK                    1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK                  8
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK                  1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK                7
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK                1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK                     6
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK                     1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK                 5
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK                 1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_MPLS_MC_INT_MASK                       4
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_MPLS_MC_INT_MASK                       1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_MPLS_UC_INT_MASK                       3
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_MPLS_UC_INT_MASK                       1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_RX_TAG_INT_MASK                           2
#define VTSS_LEN_DEV10G_MAC_INT_MASK_RX_TAG_INT_MASK                           1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_TX_UFLW_INT_MASK                          1
#define VTSS_LEN_DEV10G_MAC_INT_MASK_TX_UFLW_INT_MASK                          1
#define VTSS_OFF_DEV10G_MAC_INT_MASK_TX_ABORT_INT_MASK                         0
#define VTSS_LEN_DEV10G_MAC_INT_MASK_TX_ABORT_INT_MASK                         1

#define VTSS_ADDR_DEV10G_MAC_TX_SYNC_FIFO_CTL                             0x001e
#define VTSS_OFF_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_MAX_FILL_LEVEL_RESET     31
#define VTSS_LEN_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_MAX_FILL_LEVEL_RESET      1
#define VTSS_OFF_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_MAX_FILL_LEVEL      24
#define VTSS_LEN_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_MAX_FILL_LEVEL       6
#define VTSS_OFF_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_READ_START_LB        8
#define VTSS_LEN_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_READ_START_LB        2
#define VTSS_OFF_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_READ_START           0
#define VTSS_LEN_DEV10G_MAC_TX_SYNC_FIFO_CTL_TX_SYNC_FIFO_READ_START           2

#define VTSS_ADDR_DEV10G_RX_SYMBOL_ERR_CNT                                0x001f

#define VTSS_ADDR_DEV10G_RX_PAUSE_CNT                                     0x0020

#define VTSS_ADDR_DEV10G_RX_UNSUP_OPCODE_CNT                              0x0021

#define VTSS_ADDR_DEV10G_RX_BAD_BYTES_CNT                                 0x0022

#define VTSS_ADDR_DEV10G_RX_UC_CNT                                        0x0023

#define VTSS_ADDR_DEV10G_RX_MC_CNT                                        0x0024

#define VTSS_ADDR_DEV10G_RX_BC_CNT                                        0x0025

#define VTSS_ADDR_DEV10G_RX_CRC_ERR_CNT                                   0x0026

#define VTSS_ADDR_DEV10G_RX_UNDERSIZE_CNT                                 0x0027

#define VTSS_ADDR_DEV10G_RX_FRAGMENTS_CNT                                 0x0028

#define VTSS_ADDR_DEV10G_RX_IN_RANGE_LEN_ERR_CNT                          0x0029

#define VTSS_ADDR_DEV10G_RX_OUT_OF_RANGE_LEN_ERR_CNT                      0x002a

#define VTSS_ADDR_DEV10G_RX_OVERSIZE_CNT                                  0x002b

#define VTSS_ADDR_DEV10G_RX_JABBERS_CNT                                   0x002c

#define VTSS_ADDR_DEV10G_RX_SIZE64_CNT                                    0x002d

#define VTSS_ADDR_DEV10G_RX_SIZE65TO127_CNT                               0x002e

#define VTSS_ADDR_DEV10G_RX_SIZE128TO255_CNT                              0x002f

#define VTSS_ADDR_DEV10G_RX_SIZE256TO511_CNT                              0x0030

#define VTSS_ADDR_DEV10G_RX_SIZE512TO1023_CNT                             0x0031

#define VTSS_ADDR_DEV10G_RX_SIZE1024TO1518_CNT                            0x0032

#define VTSS_ADDR_DEV10G_RX_SIZE1519TOMAX_CNT                             0x0033

#define VTSS_ADDR_DEV10G_RX_IPG_SHRINK_CNT                                0x0034

#define VTSS_ADDR_DEV10G_TX_PAUSE_CNT                                     0x0035

#define VTSS_ADDR_DEV10G_TX_UC_CNT                                        0x0036

#define VTSS_ADDR_DEV10G_TX_MC_CNT                                        0x0037

#define VTSS_ADDR_DEV10G_TX_BC_CNT                                        0x0038

#define VTSS_ADDR_DEV10G_TX_SIZE64_CNT                                    0x0039

#define VTSS_ADDR_DEV10G_TX_SIZE65TO127_CNT                               0x003a

#define VTSS_ADDR_DEV10G_TX_SIZE128TO255_CNT                              0x003b

#define VTSS_ADDR_DEV10G_TX_SIZE256TO511_CNT                              0x003c

#define VTSS_ADDR_DEV10G_TX_SIZE512TO1023_CNT                             0x003d

#define VTSS_ADDR_DEV10G_TX_SIZE1024TO1518_CNT                            0x003e

#define VTSS_ADDR_DEV10G_TX_SIZE1519TOMAX_CNT                             0x003f

#define VTSS_ADDR_DEV10G_RX_IN_BYTES_CNT                                  0x0040

#define VTSS_ADDR_DEV10G_RX_IN_BYTES_CNT_MSB                              0x0041
#define VTSS_OFF_DEV10G_RX_IN_BYTES_CNT_MSB_RX_IN_BYTES_CNT_MSB                0
#define VTSS_LEN_DEV10G_RX_IN_BYTES_CNT_MSB_RX_IN_BYTES_CNT_MSB                8

#define VTSS_ADDR_DEV10G_RX_OK_BYTES_CNT                                  0x0042

#define VTSS_ADDR_DEV10G_RX_OK_BYTES_CNT_MSB                              0x0043
#define VTSS_OFF_DEV10G_RX_OK_BYTES_CNT_MSB_RX_OK_BYTES_CNT_MSB                0
#define VTSS_LEN_DEV10G_RX_OK_BYTES_CNT_MSB_RX_OK_BYTES_CNT_MSB                8

#define VTSS_ADDR_DEV10G_TX_OUT_BYTES_CNT                                 0x0044

#define VTSS_ADDR_DEV10G_TX_OUT_BYTES_CNT_MSB                             0x0045
#define VTSS_OFF_DEV10G_TX_OUT_BYTES_CNT_MSB_TX_OUT_BYTES_CNT_MSB              0
#define VTSS_LEN_DEV10G_TX_OUT_BYTES_CNT_MSB_TX_OUT_BYTES_CNT_MSB              8

#define VTSS_ADDR_DEV10G_TX_OK_BYTES_CNT                                  0x0046

#define VTSS_ADDR_DEV10G_TX_OK_BYTES_CNT_MSB                              0x0047
#define VTSS_OFF_DEV10G_TX_OK_BYTES_CNT_MSB_TX_OK_BYTES_CNT_MSB                0
#define VTSS_LEN_DEV10G_TX_OK_BYTES_CNT_MSB_TX_OK_BYTES_CNT_MSB                8

#define VTSS_ADDR_DEV10G_RX_XGMII_PROT_ERR_CNT                            0x0048

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_0_CFG                    0x0049
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_0_CFG_S0_SYM_B                16
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_0_CFG_S0_SYM_B                10
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_0_CFG_S0_SYM_A                 0
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_0_CFG_S0_SYM_A                10

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_1_CFG                    0x004a
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_1_CFG_S1_SYM_B                16
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_1_CFG_S1_SYM_B                10
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_1_CFG_S1_SYM_A                 0
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_1_CFG_S1_SYM_A                10

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_2_CFG                    0x004b
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_2_CFG_S2_SYM_B                16
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_2_CFG_S2_SYM_B                10
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_2_CFG_S2_SYM_A                 0
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_2_CFG_S2_SYM_A                10

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_3_CFG                    0x004c
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_3_CFG_S3_SYM_B                16
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_3_CFG_S3_SYM_B                10
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_3_CFG_S3_SYM_A                 0
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_SKIP_SYM_3_CFG_S3_SYM_A                10

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_ERR_SYM_CFG                       0x004d
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_ERR_SYM_CFG_E_SYM_B                    16
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_ERR_SYM_CFG_E_SYM_B                    10
#define VTSS_OFF_DEV10G_PCS10G_RX_FIFO_ERR_SYM_CFG_E_SYM_A                     0
#define VTSS_LEN_DEV10G_PCS10G_RX_FIFO_ERR_SYM_CFG_E_SYM_A                    10

#define VTSS_ADDR_DEV10G_PCS10G_RX_ALIGN_SYM_CFG                          0x004e
#define VTSS_OFF_DEV10G_PCS10G_RX_ALIGN_SYM_CFG_A_SYM_B                       16
#define VTSS_LEN_DEV10G_PCS10G_RX_ALIGN_SYM_CFG_A_SYM_B                       10
#define VTSS_OFF_DEV10G_PCS10G_RX_ALIGN_SYM_CFG_A_SYM_A                        0
#define VTSS_LEN_DEV10G_PCS10G_RX_ALIGN_SYM_CFG_A_SYM_A                       10

#define VTSS_ADDR_DEV10G_PCS10G_RX_MAN_ALIGN_CFG                          0x004f
#define VTSS_OFF_DEV10G_PCS10G_RX_MAN_ALIGN_CFG_MAN_ALIGN_SEL                  0
#define VTSS_LEN_DEV10G_PCS10G_RX_MAN_ALIGN_CFG_MAN_ALIGN_SEL                 16

#define VTSS_ADDR_DEV10G_PCS10G_RX_ERR_CNT_CFG                            0x0050
#define VTSS_OFF_DEV10G_PCS10G_RX_ERR_CNT_CFG_D_ERR_MASK                      12
#define VTSS_LEN_DEV10G_PCS10G_RX_ERR_CNT_CFG_D_ERR_MASK                       4
#define VTSS_OFF_DEV10G_PCS10G_RX_ERR_CNT_CFG_C_ERR_MASK                       8
#define VTSS_LEN_DEV10G_PCS10G_RX_ERR_CNT_CFG_C_ERR_MASK                       4
#define VTSS_OFF_DEV10G_PCS10G_RX_ERR_CNT_CFG_UFLW_ERR_MASK                    4
#define VTSS_LEN_DEV10G_PCS10G_RX_ERR_CNT_CFG_UFLW_ERR_MASK                    4
#define VTSS_OFF_DEV10G_PCS10G_RX_ERR_CNT_CFG_OFLW_ERR_MASK                    0
#define VTSS_LEN_DEV10G_PCS10G_RX_ERR_CNT_CFG_OFLW_ERR_MASK                    4

#define VTSS_ADDR_DEV10G_PCS10G_RX_PAD_TRUNCATE_CFG                       0x0051
#define VTSS_OFF_DEV10G_PCS10G_RX_PAD_TRUNCATE_CFG_PT_IPG_SIZE                 3
#define VTSS_LEN_DEV10G_PCS10G_RX_PAD_TRUNCATE_CFG_PT_IPG_SIZE                 3
#define VTSS_OFF_DEV10G_PCS10G_RX_PAD_TRUNCATE_CFG_PT_MODE_SEL                 0
#define VTSS_LEN_DEV10G_PCS10G_RX_PAD_TRUNCATE_CFG_PT_MODE_SEL                 3

#define VTSS_ADDR_DEV10G_PCS10G_RX_LANE_CFG                               0x0052
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_CODE_BLOCK_SIZE                    24
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_CODE_BLOCK_SIZE                     3
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_NO_VALIDS                          22
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_NO_VALIDS                           2
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_SYNC_ACQ_CNT                       18
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_SYNC_ACQ_CNT                        4
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_LSSM_MODE                          17
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_LSSM_MODE                           1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_VSC7226_SYNC_ENA                   16
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_VSC7226_SYNC_ENA                    1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_SHORT_FIFO_PIPE                    12
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_SHORT_FIFO_PIPE                     1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_FIFO_RPT_RES                       11
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_FIFO_RPT_RES                        1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_EN_10B8B_ERR_GEN                    6
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_EN_10B8B_ERR_GEN                    1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_MAN_ALIGN                           5
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_MAN_ALIGN                           1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_CHECK_10B8B                         4
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_CHECK_10B8B                         1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_BYPASS_10B8B                        3
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_BYPASS_10B8B                        1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_BYPASS_DEMAP                        2
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_BYPASS_DEMAP                        1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_CFG_CHAN_ALIGN_NO                       0
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_CFG_CHAN_ALIGN_NO                       2

#define VTSS_ADDR_DEV10G_PCS10G_TX_LANE_CFG                               0x0053
#define VTSS_OFF_DEV10G_PCS10G_TX_LANE_CFG_TX_Q                                8
#define VTSS_LEN_DEV10G_PCS10G_TX_LANE_CFG_TX_Q                               24
#define VTSS_OFF_DEV10G_PCS10G_TX_LANE_CFG_TX_Q_DIS                            7
#define VTSS_LEN_DEV10G_PCS10G_TX_LANE_CFG_TX_Q_DIS                            1
#define VTSS_OFF_DEV10G_PCS10G_TX_LANE_CFG_BYPASS_8B10B                        3
#define VTSS_LEN_DEV10G_PCS10G_TX_LANE_CFG_BYPASS_8B10B                        1
#define VTSS_OFF_DEV10G_PCS10G_TX_LANE_CFG_BYPASS_PREMAP                       2
#define VTSS_LEN_DEV10G_PCS10G_TX_LANE_CFG_BYPASS_PREMAP                       1

#define VTSS_ADDR_DEV10G_PCS10G_LB_CFG                                    0x0054
#define VTSS_OFF_DEV10G_PCS10G_LB_CFG_XAUI_LB_ENA                              0
#define VTSS_LEN_DEV10G_PCS10G_LB_CFG_XAUI_LB_ENA                              1

#define VTSS_ADDR_DEV10G_PCS10G_FLIP_HMBUS_CFG                            0x0055
#define VTSS_OFF_DEV10G_PCS10G_FLIP_HMBUS_CFG_FLIP_HMBUS_ENA                   0
#define VTSS_LEN_DEV10G_PCS10G_FLIP_HMBUS_CFG_FLIP_HMBUS_ENA                   1

#define VTSS_ADDR_DEV10G_PCS10G_RX_SEQ_REC_STAT                           0x0056
#define VTSS_OFF_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q                            8
#define VTSS_LEN_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q                           24
#define VTSS_OFF_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q_CHANGED_INT_MASK           1
#define VTSS_LEN_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q_CHANGED_INT_MASK           1
#define VTSS_OFF_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q_CHANGED_STICKY             0
#define VTSS_LEN_DEV10G_PCS10G_RX_SEQ_REC_STAT_RX_Q_CHANGED_STICKY             1

#define VTSS_ADDR_DEV10G_PCS10G_RX_LANE_STAT                              0x0057
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_FRM_ALIGNED_INT_MASK              17
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_FRM_ALIGNED_INT_MASK               1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_FRM_ALIGNED_STICKY                16
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_FRM_ALIGNED_STICKY                 1
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_LOCAL_FAULT_INT_MASK              12
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_LOCAL_FAULT_INT_MASK               4
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_LOCAL_FAULT_STICKY                 8
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_LOCAL_FAULT_STICKY                 4
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_ALIGN                              4
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_ALIGN                              4
#define VTSS_OFF_DEV10G_PCS10G_RX_LANE_STAT_LANE_SYNC                          0
#define VTSS_LEN_DEV10G_PCS10G_RX_LANE_STAT_LANE_SYNC                          4

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_L0_OFLW_ERR_CNT                   0x0058

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_L1_UFLW_ERR_CNT                   0x0059

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_L2_D_ERR_CNT                      0x005a

#define VTSS_ADDR_DEV10G_PCS10G_RX_FIFO_L3_CG_ERR_CNT                     0x005b

#define VTSS_ADDR_DEV10G_PCS10G_RX_VTSS_UNCOD_ERR_SYM_CFG                 0x005c
#define VTSS_OFF_DEV10G_PCS10G_RX_VTSS_UNCOD_ERR_SYM_CFG_ERR_SYM_DEC           0
#define VTSS_LEN_DEV10G_PCS10G_RX_VTSS_UNCOD_ERR_SYM_CFG_ERR_SYM_DEC          10

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_CTRL_CFG                           0x005d
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PATTERN_MATCH                  16
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PATTERN_MATCH                   4
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_FCJ_DISP_INIT                  14
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_FCJ_DISP_INIT                   1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP                  13
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP                   1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PRBS_POLY_INV                  12
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_PRBS_POLY_INV                   1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_FREEZE_ERR_CNT_ENA             10
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_FREEZE_ERR_CNT_ENA              1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_ENA                      9
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_ENA                      1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_SEL                      5
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_SEL                      4
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_ENA                      4
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_ENA                      1
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_SEL                      0
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_SEL                      4

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_PRPAT_L0_CFG                       0x005e
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L0_CFG_CHK_PAT_L0                 16
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L0_CFG_CHK_PAT_L0                 10
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L0_CFG_GEN_PAT_L0                  0
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L0_CFG_GEN_PAT_L0                 10

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_PRPAT_L1_CFG                       0x005f
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L1_CFG_CHK_PAT_L1                 16
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L1_CFG_CHK_PAT_L1                 10
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L1_CFG_GEN_PAT_L1                  0
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L1_CFG_GEN_PAT_L1                 10

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_PRPAT_L2_CFG                       0x0060
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L2_CFG_CHK_PAT_L2                 16
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L2_CFG_CHK_PAT_L2                 10
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L2_CFG_GEN_PAT_L2                  0
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L2_CFG_GEN_PAT_L2                 10

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_PRPAT_L3_CFG                       0x0061
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L3_CFG_CHK_PAT_L3                 16
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L3_CFG_CHK_PAT_L3                 10
#define VTSS_OFF_DEV10G_PCS10G_TSTPAT_PRPAT_L3_CFG_GEN_PAT_L3                  0
#define VTSS_LEN_DEV10G_PCS10G_TSTPAT_PRPAT_L3_CFG_GEN_PAT_L3                 10

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_RND_SEQ_CNT                        0x0062

#define VTSS_ADDR_DEV10G_PCS10G_TSTPAT_TX_SEQ_CNT                         0x0063

#define VTSS_ADDR_DEV10G_PCS12G_CFG                                       0x0064
#define VTSS_OFF_DEV10G_PCS12G_CFG_RESYNC_ENA                                  8
#define VTSS_LEN_DEV10G_PCS12G_CFG_RESYNC_ENA                                  1
#define VTSS_OFF_DEV10G_PCS12G_CFG_SCRAM_DIS                                   7
#define VTSS_LEN_DEV10G_PCS12G_CFG_SCRAM_DIS                                   1
#define VTSS_OFF_DEV10G_PCS12G_CFG_SH_CNT_MAX                                  1
#define VTSS_LEN_DEV10G_PCS12G_CFG_SH_CNT_MAX                                  6
#define VTSS_OFF_DEV10G_PCS12G_CFG_SYNC_TYPE_SEL                               0
#define VTSS_LEN_DEV10G_PCS12G_CFG_SYNC_TYPE_SEL                               1

#define VTSS_ADDR_DEV10G_PCS12G_STATUS                                    0x0065
#define VTSS_OFF_DEV10G_PCS12G_STATUS_RX_ALIGNMENT_STATUS                      4
#define VTSS_LEN_DEV10G_PCS12G_STATUS_RX_ALIGNMENT_STATUS                      1
#define VTSS_OFF_DEV10G_PCS12G_STATUS_RX_SYNC_STATUS                           0
#define VTSS_LEN_DEV10G_PCS12G_STATUS_RX_SYNC_STATUS                           4

#define VTSS_ADDR_DEV10G_PCS12G_ERR_STATUS                                0x0066
#define VTSS_OFF_DEV10G_PCS12G_ERR_STATUS_XGMII_ERR_STICKY                     6
#define VTSS_LEN_DEV10G_PCS12G_ERR_STATUS_XGMII_ERR_STICKY                     1
#define VTSS_OFF_DEV10G_PCS12G_ERR_STATUS_C64B66B_ERR_STICKY                   5
#define VTSS_LEN_DEV10G_PCS12G_ERR_STATUS_C64B66B_ERR_STICKY                   1
#define VTSS_OFF_DEV10G_PCS12G_ERR_STATUS_ALIGNMENT_LOST_STICKY                4
#define VTSS_LEN_DEV10G_PCS12G_ERR_STATUS_ALIGNMENT_LOST_STICKY                1
#define VTSS_OFF_DEV10G_PCS12G_ERR_STATUS_SYNC_LOST_STICKY                     0
#define VTSS_LEN_DEV10G_PCS12G_ERR_STATUS_SYNC_LOST_STICKY                     4

#define VTSS_ADDR_DEV10G_DEV_RST_CTRL                                     0x0067
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_LINK_STATE                               28
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_LINK_STATE                                1
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DIVIDE_SEL                           24
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_CLK_DIVIDE_SEL                            2
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_SPEED_SEL                                20
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_SPEED_SEL                                 3
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DRIVE_EN                             16
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_CLK_DRIVE_EN                              1
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_TX_RST                               12
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_PCS_TX_RST                                1
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_RX_RST                                8
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_PCS_RX_RST                                1
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_TX_RST                                4
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_MAC_TX_RST                                1
#define VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_RX_RST                                0
#define VTSS_LEN_DEV10G_DEV_RST_CTRL_MAC_RX_RST                                1

#define VTSS_ADDR_DEV10G_DEV_PORT_PROTECT                                 0x0068
#define VTSS_OFF_DEV10G_DEV_PORT_PROTECT_PORT_PROTECT_ID                       4
#define VTSS_LEN_DEV10G_DEV_PORT_PROTECT_PORT_PROTECT_ID                       2
#define VTSS_OFF_DEV10G_DEV_PORT_PROTECT_PORT_PROTECT_ENA                      0
#define VTSS_LEN_DEV10G_DEV_PORT_PROTECT_PORT_PROTECT_ENA                      1

#define VTSS_ADDR_DEV10G_DEV_LB_CFG                                       0x0069
#define VTSS_OFF_DEV10G_DEV_LB_CFG_LINE_LB_ENA                                 6
#define VTSS_LEN_DEV10G_DEV_LB_CFG_LINE_LB_ENA                                 1
#define VTSS_OFF_DEV10G_DEV_LB_CFG_TAXI_PHY_LB_ENA                             4
#define VTSS_LEN_DEV10G_DEV_LB_CFG_TAXI_PHY_LB_ENA                             1
#define VTSS_OFF_DEV10G_DEV_LB_CFG_TAXI_HOST_LB_ENA                            0
#define VTSS_LEN_DEV10G_DEV_LB_CFG_TAXI_HOST_LB_ENA                            1

#define VTSS_ADDR_DEV10G_DEV_DBG_CFG                                      0x006a
#define VTSS_OFF_DEV10G_DEV_DBG_CFG_TX_BUF_HIGH_WM                             0
#define VTSS_LEN_DEV10G_DEV_DBG_CFG_TX_BUF_HIGH_WM                             6

#define VTSS_ADDR_DEV10G_DEV_STICKY                                       0x006b
#define VTSS_OFF_DEV10G_DEV_STICKY_LINK_DOWN_STICKY                           25
#define VTSS_LEN_DEV10G_DEV_STICKY_LINK_DOWN_STICKY                            1
#define VTSS_OFF_DEV10G_DEV_STICKY_LINK_UP_STICKY                             24
#define VTSS_LEN_DEV10G_DEV_STICKY_LINK_UP_STICKY                              1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_RESYNC_FIFO_OFLW_STICKY                 10
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_RESYNC_FIFO_OFLW_STICKY                  1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_TAXI_FIFO_OFLW_STICKY                    9
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_TAXI_FIFO_OFLW_STICKY                    1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_EOF_STICKY                               8
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_EOF_STICKY                               1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_SOF_STICKY                               7
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_SOF_STICKY                               1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_GMII_IPG_SHRINK_STICKY                   6
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_GMII_IPG_SHRINK_STICKY                   1
#define VTSS_OFF_DEV10G_DEV_STICKY_RX_GMII_JUNK_DATA_STICKY                    5
#define VTSS_LEN_DEV10G_DEV_STICKY_RX_GMII_JUNK_DATA_STICKY                    1
#define VTSS_OFF_DEV10G_DEV_STICKY_TX_PROTO_ERR_STICKY                         4
#define VTSS_LEN_DEV10G_DEV_STICKY_TX_PROTO_ERR_STICKY                         1
#define VTSS_OFF_DEV10G_DEV_STICKY_TX_EOF_STICKY                               3
#define VTSS_LEN_DEV10G_DEV_STICKY_TX_EOF_STICKY                               1
#define VTSS_OFF_DEV10G_DEV_STICKY_TX_SOF_STICKY                               2
#define VTSS_LEN_DEV10G_DEV_STICKY_TX_SOF_STICKY                               1
#define VTSS_OFF_DEV10G_DEV_STICKY_TX_FIFO_OFLW_STICKY                         1
#define VTSS_LEN_DEV10G_DEV_STICKY_TX_FIFO_OFLW_STICKY                         1
#define VTSS_OFF_DEV10G_DEV_STICKY_TX_FIFO_UFLW_STICKY                         0
#define VTSS_LEN_DEV10G_DEV_STICKY_TX_FIFO_UFLW_STICKY                         1

#define VTSS_ADDR_DEV10G_DEV_INT_MASK                                     0x006c
#define VTSS_OFF_DEV10G_DEV_INT_MASK_LINK_DOWN_MASK                           25
#define VTSS_LEN_DEV10G_DEV_INT_MASK_LINK_DOWN_MASK                            1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_LINK_UP_MASK                             24
#define VTSS_LEN_DEV10G_DEV_INT_MASK_LINK_UP_MASK                              1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_RESYNC_FIFO_OFLW_INT_MASK             10
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_RESYNC_FIFO_OFLW_INT_MASK              1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_TAXI_FIFO_OFLW_INT_MASK                9
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_TAXI_FIFO_OFLW_INT_MASK                1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_EOF_INT_MASK                           8
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_EOF_INT_MASK                           1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_SOF_INT_MASK                           7
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_SOF_INT_MASK                           1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_GMII_IPG_SHRINK_INT_MASK               6
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_GMII_IPG_SHRINK_INT_MASK               1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_RX_GMII_JUNK_DATA_INT_MASK                5
#define VTSS_LEN_DEV10G_DEV_INT_MASK_RX_GMII_JUNK_DATA_INT_MASK                1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_TX_PROTO_ERR_INT_MASK                     4
#define VTSS_LEN_DEV10G_DEV_INT_MASK_TX_PROTO_ERR_INT_MASK                     1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_TX_EOF_INT_MASK                           3
#define VTSS_LEN_DEV10G_DEV_INT_MASK_TX_EOF_INT_MASK                           1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_TX_SOF_INT_MASK                           2
#define VTSS_LEN_DEV10G_DEV_INT_MASK_TX_SOF_INT_MASK                           1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_TX_FIFO_OFLW_INT_MASK                     1
#define VTSS_LEN_DEV10G_DEV_INT_MASK_TX_FIFO_OFLW_INT_MASK                     1
#define VTSS_OFF_DEV10G_DEV_INT_MASK_TX_FIFO_UFLW_INT_MASK                     0
#define VTSS_LEN_DEV10G_DEV_INT_MASK_TX_FIFO_UFLW_INT_MASK                     1

#define VTSS_ADDR_DEV10G_DEV_SPARE                                        0x006d

/*********************************************************************** 
 * Target XAUI_PHY_STAT
 * XAUI PHY Configuration And Status Target
 ***********************************************************************/
#define VTSS_ADDR_XAUI_PHY_STAT_DVR_CTRL                                  0x0000
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TX_SWAP                               24
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TX_SWAP                                1
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_3                        22
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_3                         2
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_2                        20
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_2                         2
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_1                        18
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_1                         2
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_0                        16
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXDRIVE_LANE_0                         2
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_3                           13
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_3                            3
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_2                           10
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_2                            3
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_1                            7
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_1                            3
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_0                            4
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_TXEQ_LANE_0                            3
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_3                           3
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_3                           1
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_2                           2
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_2                           1
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_1                           1
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_1                           1
#define VTSS_OFF_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_0                           0
#define VTSS_LEN_XAUI_PHY_STAT_DVR_CTRL_DRVCM_LANE_0                           1

#define VTSS_ADDR_XAUI_PHY_STAT_CMU_CTRL                                  0x0001
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLBW                                  8
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_TXLBW                                  4
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLDPPM                                5
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_TXLDPPM                                2
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REF_SRC                                4
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_REF_SRC                                1
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REFSEL                                 2
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_REFSEL                                 2
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_CMUPOWEROFF                            1
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_CMUPOWEROFF                            1
#define VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_SETUP_CP                               0
#define VTSS_LEN_XAUI_PHY_STAT_CMU_CTRL_SETUP_CP                               1

#define VTSS_ADDR_XAUI_PHY_STAT_CORE_CLK_OUTPUT_CTRL                      0x0002
#define VTSS_OFF_XAUI_PHY_STAT_CORE_CLK_OUTPUT_CTRL_CLK10POWEROFF              1
#define VTSS_LEN_XAUI_PHY_STAT_CORE_CLK_OUTPUT_CTRL_CLK10POWEROFF              1
#define VTSS_OFF_XAUI_PHY_STAT_CORE_CLK_OUTPUT_CTRL_MR_CLK10                   0
#define VTSS_LEN_XAUI_PHY_STAT_CORE_CLK_OUTPUT_CTRL_MR_CLK10                   1

#define VTSS_ADDR_XAUI_PHY_STAT_TX_PWR_CTRL                               0x0003
#define VTSS_OFF_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_3                   3
#define VTSS_LEN_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_3                   1
#define VTSS_OFF_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_2                   2
#define VTSS_LEN_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_2                   1
#define VTSS_OFF_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_1                   1
#define VTSS_LEN_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_1                   1
#define VTSS_OFF_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_0                   0
#define VTSS_LEN_XAUI_PHY_STAT_TX_PWR_CTRL_TXPOWEROFF_LANE_0                   1

#define VTSS_ADDR_XAUI_PHY_STAT_RX_CELL_CTL                               0x0004
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RX_LANE_SWAP                       31
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RX_LANE_SWAP                        1
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_3                     22
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_3                      2
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_2                     20
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_2                      2
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_1                     18
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_1                      2
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_0                     16
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXLOSTH_LANE_0                      2
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_3                        12
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_3                         4
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_2                         8
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_2                         4
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_1                         4
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_1                         4
#define VTSS_OFF_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_0                         0
#define VTSS_LEN_XAUI_PHY_STAT_RX_CELL_CTL_RXEQ_LANE_0                         4

#define VTSS_ADDR_XAUI_PHY_STAT_CRU_CTRL                                  0x0005
#define VTSS_OFF_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_3                           8
#define VTSS_LEN_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_3                           2
#define VTSS_OFF_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_2                           6
#define VTSS_LEN_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_2                           2
#define VTSS_OFF_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_1                           4
#define VTSS_LEN_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_1                           2
#define VTSS_OFF_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_0                           2
#define VTSS_LEN_XAUI_PHY_STAT_CRU_CTRL_RXLBW_LANE_0                           2
#define VTSS_OFF_XAUI_PHY_STAT_CRU_CTRL_RXLDPPM                                0
#define VTSS_LEN_XAUI_PHY_STAT_CRU_CTRL_RXLDPPM                                2

#define VTSS_ADDR_XAUI_PHY_STAT_RX_PWR_CTRL                               0x0006
#define VTSS_OFF_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_3                   3
#define VTSS_LEN_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_3                   1
#define VTSS_OFF_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_2                   2
#define VTSS_LEN_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_2                   1
#define VTSS_OFF_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_1                   1
#define VTSS_LEN_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_1                   1
#define VTSS_OFF_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_0                   0
#define VTSS_LEN_XAUI_PHY_STAT_RX_PWR_CTRL_RXPOWEROFF_LANE_0                   1

#define VTSS_ADDR_XAUI_PHY_STAT_RESET_CTRL                                0x0007
#define VTSS_OFF_XAUI_PHY_STAT_RESET_CTRL_MR_RX                                4
#define VTSS_LEN_XAUI_PHY_STAT_RESET_CTRL_MR_RX                                1
#define VTSS_OFF_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_3                         3
#define VTSS_LEN_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_3                         1
#define VTSS_OFF_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_2                         2
#define VTSS_LEN_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_2                         1
#define VTSS_OFF_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_1                         1
#define VTSS_LEN_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_1                         1
#define VTSS_OFF_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_0                         0
#define VTSS_LEN_XAUI_PHY_STAT_RESET_CTRL_MR_TX_LANE_0                         1

#define VTSS_ADDR_XAUI_PHY_STAT_LOOPBACK_CTRL                             0x0008
#define VTSS_OFF_XAUI_PHY_STAT_LOOPBACK_CTRL_EQUIPLOOP                         0
#define VTSS_LEN_XAUI_PHY_STAT_LOOPBACK_CTRL_EQUIPLOOP                         1

#define VTSS_ADDR_XAUI_PHY_STAT_OP_DISABLE                                0x0009
#define VTSS_OFF_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_3                    3
#define VTSS_LEN_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_3                    1
#define VTSS_OFF_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_2                    2
#define VTSS_LEN_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_2                    1
#define VTSS_OFF_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_1                    1
#define VTSS_LEN_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_1                    1
#define VTSS_OFF_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_0                    0
#define VTSS_LEN_XAUI_PHY_STAT_OP_DISABLE_OP_DISABLE_LANE_0                    1

#define VTSS_ADDR_XAUI_PHY_STAT_CMU_STAT                                  0x0010
#define VTSS_OFF_XAUI_PHY_STAT_CMU_STAT_NOREF                                  3
#define VTSS_LEN_XAUI_PHY_STAT_CMU_STAT_NOREF                                  1
#define VTSS_OFF_XAUI_PHY_STAT_CMU_STAT_TXLOLLO                                2
#define VTSS_LEN_XAUI_PHY_STAT_CMU_STAT_TXLOLLO                                1
#define VTSS_OFF_XAUI_PHY_STAT_CMU_STAT_TXLOLHI                                1
#define VTSS_LEN_XAUI_PHY_STAT_CMU_STAT_TXLOLHI                                1
#define VTSS_OFF_XAUI_PHY_STAT_CMU_STAT_TXLOL                                  0
#define VTSS_LEN_XAUI_PHY_STAT_CMU_STAT_TXLOL                                  1

#define VTSS_ADDR_XAUI_PHY_STAT_CRU_STAT                                  0x0011
#define VTSS_OFF_XAUI_PHY_STAT_CRU_STAT_RXLOLLO                                2
#define VTSS_LEN_XAUI_PHY_STAT_CRU_STAT_RXLOLLO                                1
#define VTSS_OFF_XAUI_PHY_STAT_CRU_STAT_RXLOLHI                                1
#define VTSS_LEN_XAUI_PHY_STAT_CRU_STAT_RXLOLHI                                1
#define VTSS_OFF_XAUI_PHY_STAT_CRU_STAT_RXLOL                                  0
#define VTSS_LEN_XAUI_PHY_STAT_CRU_STAT_RXLOL                                  1

#define VTSS_ADDR_XAUI_PHY_STAT_RX_LOS                                    0x0012
#define VTSS_OFF_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_3                             3
#define VTSS_LEN_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_3                             1
#define VTSS_OFF_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_2                             2
#define VTSS_LEN_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_2                             1
#define VTSS_OFF_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_1                             1
#define VTSS_LEN_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_1                             1
#define VTSS_OFF_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_0                             0
#define VTSS_LEN_XAUI_PHY_STAT_RX_LOS_RXLOS_LANE_0                             1

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_SB_CONFIG                            0x0020
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_SB_INTERPRET_01                 10
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_SB_CONFIG_SB_INTERPRET_01                  1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_DIP_2_ERROR_INT_MASK             8
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_SB_CONFIG_DIP_2_ERROR_INT_MASK             1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_STATUS_CHANNEL_AGGREGATION_MODE      0
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_SB_CONFIG_STATUS_CHANNEL_AGGREGATION_MODE      1

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_IBS_CFG                              0x0030
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SOURCE               28
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SOURCE                1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SHIFT                20
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SHIFT                 1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE               19
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE                1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_M                     8
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_M                     4
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_LEN                   0
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_LEN                   6

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_IBS_DIAG                             0x0031
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_DIAG_CONF_IBS_OVWR_ENA                10
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_DIAG_CONF_IBS_OVWR_ENA                 1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_DIAG_CONF_IBS_OVWR_VAL                 8
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_DIAG_CONF_IBS_OVWR_VAL                 2

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_IBS_CAL                              0x0032
#define VTSS_ADDX_XAUI_PHY_STAT_XAUI_IBS_CAL(x)                           (VTSS_ADDR_XAUI_PHY_STAT_XAUI_IBS_CAL + (x))
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CAL_CONF_IBS_CAL_VAL                   0
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_IBS_CAL_CONF_IBS_CAL_VAL                   7

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_CFG                              0x0072
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CLK_SHIFT                20
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CLK_SHIFT                 1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_M                     8
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_M                     4
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_LEN                   0
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_LEN                   6

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_DIAG_CONFIG                      0x0073
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_DIAG_CONFIG_CONF_OBS_OVWR_ENA         10
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_DIAG_CONFIG_CONF_OBS_OVWR_ENA          1
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_DIAG_CONFIG_CONF_OBS_OVWR_VAL          8
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_DIAG_CONFIG_CONF_OBS_OVWR_VAL          2

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_CAL                              0x0074
#define VTSS_ADDX_XAUI_PHY_STAT_XAUI_OBS_CAL(x)                           (VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_CAL + (x))
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CAL_CONF_OBS_CAL_VAL                   0
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_CAL_CONF_OBS_CAL_VAL                   7

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_DIP2_ERR_CNT                     0x00b4

#define VTSS_ADDR_XAUI_PHY_STAT_XAUI_OBS_STICKY                           0x00b5
#define VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_STICKY_OBS_DIP2_ERR_STICKY             1
#define VTSS_LEN_XAUI_PHY_STAT_XAUI_OBS_STICKY_OBS_DIP2_ERR_STICKY             1

/*********************************************************************** 
 * Target FAST_REGS
 * Fast Register Block
 ***********************************************************************/
#define VTSS_ADDR_FAST_REGS_SLOWDATA_MSB                                  0x0000
#define VTSS_OFF_FAST_REGS_SLOWDATA_MSB_SLOW_DATA_MSB                          0
#define VTSS_LEN_FAST_REGS_SLOWDATA_MSB_SLOW_DATA_MSB                         16

#define VTSS_ADDR_FAST_REGS_SLOWDATA_LSB                                  0x0001
#define VTSS_OFF_FAST_REGS_SLOWDATA_LSB_SLOW_DATA_LSB                          0
#define VTSS_LEN_FAST_REGS_SLOWDATA_LSB_SLOW_DATA_LSB                         16

#define VTSS_ADDR_FAST_REGS_CFG_STATUS_1                                  0x0002
#define VTSS_OFF_FAST_REGS_CFG_STATUS_1_PI_DONE_DRIVE_DIS                     12
#define VTSS_LEN_FAST_REGS_CFG_STATUS_1_PI_DONE_DRIVE_DIS                      3
#define VTSS_OFF_FAST_REGS_CFG_STATUS_1_FORCE_PI_RDY_N                         8
#define VTSS_LEN_FAST_REGS_CFG_STATUS_1_FORCE_PI_RDY_N                         1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_1_ENDIAN_SEL                             1
#define VTSS_LEN_FAST_REGS_CFG_STATUS_1_ENDIAN_SEL                             1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_1_INVERSE_INTR_N_POLAR                   0
#define VTSS_LEN_FAST_REGS_CFG_STATUS_1_INVERSE_INTR_N_POLAR                   1

#define VTSS_ADDR_FAST_REGS_CFG_STATUS_2                                  0x0003
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_RD_IN_PROGRESS                        15
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_RD_IN_PROGRESS                         1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_WR_IN_PROGRESS                        14
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_WR_IN_PROGRESS                         1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_PI_RDY                                13
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_PI_RDY                                 1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_PI_HOLD                                8
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_PI_HOLD                                3
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_REQUEST_ACK                            7
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_REQUEST_ACK                            1
#define VTSS_OFF_FAST_REGS_CFG_STATUS_2_PI_WAIT                                3
#define VTSS_LEN_FAST_REGS_CFG_STATUS_2_PI_WAIT                                4

#define VTSS_ADDR_FAST_REGS_INT_STATUS_1                                  0x0004
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_0                        15
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_0                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_1                        14
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_1                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_2                        13
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_2                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_3                        12
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_3                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_4                        11
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_4                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_5                        10
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_5                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_6                         9
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_6                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_7                         8
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_7                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_8                         7
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_8                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_9                         6
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_9                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_10                        5
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_10                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_11                        4
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_11                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_12                        3
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_12                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_13                        2
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_13                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_14                        1
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_14                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_15                        0
#define VTSS_LEN_FAST_REGS_INT_STATUS_1_GBE_INT_STAT_15                        1

#define VTSS_ADDR_FAST_REGS_INT_STATUS_2                                  0x0005
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_16                        7
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_16                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_17                        6
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_17                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_18                        5
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_18                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_19                        4
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_19                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_20                        3
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_20                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_21                        2
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_21                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_22                        1
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_22                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_23                        0
#define VTSS_LEN_FAST_REGS_INT_STATUS_2_GBE_INT_STAT_23                        1

#define VTSS_ADDR_FAST_REGS_INT_STATUS_3                                  0x0006
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_CPU_IF_INT_STAT                       15
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_CPU_IF_INT_STAT                        1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_GPIO_INT_STAT                         14
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_GPIO_INT_STAT                          1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_CLASSIFIER_INT_STAT                   13
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_CLASSIFIER_INT_STAT                    1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_POLICER_INT_STAT                      12
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_POLICER_INT_STAT                       1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_QUEUE_INT_STAT                        11
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_QUEUE_INT_STAT                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_SCHED_INT_STAT                        10
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_SCHED_INT_STAT                         1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_DSM_INT_STAT                           9
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_DSM_INT_STAT                           1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_ASM_INT_STAT                           8
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_ASM_INT_STAT                           1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_SPI4_INT_STAT                          6
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_SPI4_INT_STAT                          1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_TEN_GBE_INT_STAT_0                     1
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_TEN_GBE_INT_STAT_0                     1
#define VTSS_OFF_FAST_REGS_INT_STATUS_3_TEN_GBE_INT_STAT_1                     0
#define VTSS_LEN_FAST_REGS_INT_STATUS_3_TEN_GBE_INT_STAT_1                     1

#define VTSS_ADDR_FAST_REGS_INT_ENABLE_1                                  0x0007
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_0                      15
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_0                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_1                      14
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_1                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_2                      13
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_2                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_3                      12
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_3                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_4                      11
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_4                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_5                      10
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_5                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_6                       9
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_6                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_7                       8
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_7                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_8                       7
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_8                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_9                       6
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_9                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_10                      5
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_10                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_11                      4
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_11                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_12                      3
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_12                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_13                      2
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_13                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_14                      1
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_14                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_15                      0
#define VTSS_LEN_FAST_REGS_INT_ENABLE_1_GBE_INT_ENABLE_15                      1

#define VTSS_ADDR_FAST_REGS_INT_ENABLE_2                                  0x0008
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_16                      7
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_16                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_17                      6
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_17                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_18                      5
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_18                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_19                      4
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_19                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_20                      3
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_20                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_21                      2
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_21                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_22                      1
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_22                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_23                      0
#define VTSS_LEN_FAST_REGS_INT_ENABLE_2_GBE_INT_ENABLE_23                      1

#define VTSS_ADDR_FAST_REGS_INT_ENABLE_3                                  0x0009
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_CPU_IF_INT_ENABLE                     15
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_CPU_IF_INT_ENABLE                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_GPIO_INT_ENABLE                       14
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_GPIO_INT_ENABLE                        1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_CLASSIFIER_INT_ENABLE                 13
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_CLASSIFIER_INT_ENABLE                  1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_POLICER_INT_ENABLE                    12
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_POLICER_INT_ENABLE                     1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_QUEUE_INT_ENABLE                      11
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_QUEUE_INT_ENABLE                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_SCHED_INT_ENABLE                      10
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_SCHED_INT_ENABLE                       1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_DSM_INT_ENABLE                         9
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_DSM_INT_ENABLE                         1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_ASM_INT_ENABLE                         8
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_ASM_INT_ENABLE                         1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_SPI4_INT_ENABLE                        6
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_SPI4_INT_ENABLE                        1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_0                      1
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_0                      1
#define VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_1                      0
#define VTSS_LEN_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_1                      1

#define VTSS_ADDR_FAST_REGS_PSI_CFG                                       0x000a
#define VTSS_OFF_FAST_REGS_PSI_CFG_PSI_CFG_PRESCALE                            8
#define VTSS_LEN_FAST_REGS_PSI_CFG_PSI_CFG_PRESCALE                            8
#define VTSS_OFF_FAST_REGS_PSI_CFG_PSI_ENABLE                                  0
#define VTSS_LEN_FAST_REGS_PSI_CFG_PSI_ENABLE                                  1

#define VTSS_ADDR_FAST_REGS_DEV_RESET_1                                   0x000b
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_0                            15
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_0                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_1                            14
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_1                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_2                            13
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_2                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_3                            12
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_3                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_4                            11
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_4                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_5                            10
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_5                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_6                             9
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_6                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_7                             8
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_7                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_8                             7
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_8                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_9                             6
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_9                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_10                            5
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_10                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_11                            4
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_11                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_12                            3
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_12                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_13                            2
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_13                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_14                            1
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_14                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_1_GBE_RESET_15                            0
#define VTSS_LEN_FAST_REGS_DEV_RESET_1_GBE_RESET_15                            1

#define VTSS_ADDR_FAST_REGS_DEV_RESET_2                                   0x000c
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_16                            7
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_16                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_17                            6
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_17                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_18                            5
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_18                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_19                            4
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_19                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_20                            3
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_20                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_21                            2
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_21                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_22                            1
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_22                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_2_GBE_RESET_23                            0
#define VTSS_LEN_FAST_REGS_DEV_RESET_2_GBE_RESET_23                            1

#define VTSS_ADDR_FAST_REGS_DEV_RESET_3                                   0x000d
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_CLASSIFIER_RESET                       13
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_CLASSIFIER_RESET                        1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_QUEUE_RESET                            11
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_QUEUE_RESET                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_SCHED_RESET                            10
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_SCHED_RESET                             1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_DSM_RESET                               9
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_DSM_RESET                               1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_ASM_RESET                               8
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_ASM_RESET                               1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_SPI4_RESET                              6
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_SPI4_RESET                              1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_XAUI_RESET_0                            1
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_XAUI_RESET_0                            1
#define VTSS_OFF_FAST_REGS_DEV_RESET_3_XAUI_RESET_1                            0
#define VTSS_LEN_FAST_REGS_DEV_RESET_3_XAUI_RESET_1                            1

#define VTSS_ADDR_FAST_REGS_REV_ID                                        0x000e
#define VTSS_OFF_FAST_REGS_REV_ID_REV_ID                                       0
#define VTSS_LEN_FAST_REGS_REV_ID_REV_ID                                       4

#define VTSS_ADDR_FAST_REGS_PART_ID                                       0x000f
#define VTSS_OFF_FAST_REGS_PART_ID_PART_ID                                     0
#define VTSS_LEN_FAST_REGS_PART_ID_PART_ID                                    16

#define VTSS_ADDR_FAST_REGS_MFG_ID                                        0x0010
#define VTSS_OFF_FAST_REGS_MFG_ID_MFG_ID                                       0
#define VTSS_LEN_FAST_REGS_MFG_ID_MFG_ID                                      11

#define VTSS_ADDR_FAST_REGS_ONE                                           0x0011
#define VTSS_OFF_FAST_REGS_ONE_ONE                                             0
#define VTSS_LEN_FAST_REGS_ONE_ONE                                             1

#define VTSS_ADDR_FAST_REGS_TMON_ENABLE                                   0x0012
#define VTSS_OFF_FAST_REGS_TMON_ENABLE_TMON_ENABLE                             0
#define VTSS_LEN_FAST_REGS_TMON_ENABLE_TMON_ENABLE                             1

#define VTSS_ADDR_FAST_REGS_TMON_RUN                                      0x0013
#define VTSS_OFF_FAST_REGS_TMON_RUN_TMON_RUN                                   0
#define VTSS_LEN_FAST_REGS_TMON_RUN_TMON_RUN                                   1

#define VTSS_ADDR_FAST_REGS_TMON_DATA                                     0x0014
#define VTSS_OFF_FAST_REGS_TMON_DATA_TMON_DATA                                 0
#define VTSS_LEN_FAST_REGS_TMON_DATA_TMON_DATA                                 6

#define VTSS_ADDR_FAST_REGS_TMON_READY                                    0x0015
#define VTSS_OFF_FAST_REGS_TMON_READY_TMON_READY                               0
#define VTSS_LEN_FAST_REGS_TMON_READY_TMON_READY                               1

#endif /* _VTSS_H_BARRINGTON2_ */
