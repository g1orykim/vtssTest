/*

 Vitesse Switch Software.

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
#ifndef _VTSS_DAYTONA_REG_INIT_PCS_H
#define _VTSS_DAYTONA_REG_INIT_PCS_H

// Settings for mode INTR_MON

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_INTR_MON                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_INTR_MON                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_INTR_MON                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_INTR_MON                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_INTR_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_INTR_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_INTR_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_INTR_MON                                  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_INTR_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_INTR_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0xc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_INTR_MON                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x20)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_INTR_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0xdc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_INTR_MON                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xf2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_INTR_MON                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x25d8)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_INTR_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_INTR_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode INTR_MON_LOS

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_INTR_MON_LOS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_INTR_MON_LOS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_INTR_MON_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_INTR_MON_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_INTR_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_INTR_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_INTR_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_INTR_MON_LOS                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_INTR_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_INTR_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0xc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_INTR_MON_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x20)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_INTR_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0xdc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_INTR_MON_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xf2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_INTR_MON_LOS                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x25d8)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_INTR_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_INTR_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode PASS_MON

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_PASS_MON                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_PASS_MON                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_PASS_MON                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_PASS_MON                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_PASS_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_PASS_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_PASS_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_PASS_MON                                  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_PASS_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_PASS_MON                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_PASS_MON                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_PASS_MON                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0x40)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_PASS_MON                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xc0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_PASS_MON                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x28c6)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_PASS_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_PASS_MON                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode PASS_MON_WIS

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_PASS_MON_WIS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_PASS_MON_WIS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_PASS_MON_WIS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_PASS_MON_WIS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_PASS_MON_WIS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_PASS_MON_WIS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_PASS_MON_WIS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_PASS_MON_WIS                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_PASS_MON_WIS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_PASS_MON_WIS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_PASS_MON_WIS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_PASS_MON_WIS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0xdc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_PASS_MON_WIS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xf2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_PASS_MON_WIS                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x28c6)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_PASS_MON_WIS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_PASS_MON_WIS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode PASS_MON_LOS

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_PASS_MON_LOS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_PASS_MON_LOS                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_PASS_MON_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_PASS_MON_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_PASS_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_PASS_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_PASS_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_PASS_MON_LOS                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_PASS_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_PASS_MON_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_PASS_MON_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_PASS_MON_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0x40)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_PASS_MON_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xc0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_PASS_MON_LOS                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x28c6)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_PASS_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_PASS_MON_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode BYP

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_BYP                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_BYP                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_BYP                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_BYP                              VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_BYP                                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_BYP                                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_BYP                                  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_BYP                                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_BYP                                  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_BYP                                  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_BYP                               VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_BYP                                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_BYP                               VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_BYP                                   VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_BYP                                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_BYP                                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode MIN_IFG_1

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_MIN_IFG_1                    VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_MIN_IFG_1                    VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_MIN_IFG_1                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_MIN_IFG_1                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_MIN_IFG_1                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_MIN_IFG_1                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_MIN_IFG_1                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_MIN_IFG_1                                 VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_MIN_IFG_1                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_MIN_IFG_1                            VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0xc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_MIN_IFG_1                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x20)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_MIN_IFG_1                          VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0xdc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_MIN_IFG_1                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xf2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_MIN_IFG_1                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x25d8)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_MIN_IFG_1                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_MIN_IFG_1                           VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)


// Settings for mode MIN_IFG_1_LOS

#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE_MIN_IFG_1_LOS                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE_MIN_IFG_1_LOS                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA_MIN_IFG_1_LOS                    VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA_MIN_IFG_1_LOS                    VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS_MIN_IFG_1_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS_MIN_IFG_1_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON_MIN_IFG_1_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(0x0)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS_MIN_IFG_1_LOS                             VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP_MIN_IFG_1_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP_MIN_IFG_1_LOS                        VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(0xc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL_MIN_IFG_1_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(0x20)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL_MIN_IFG_1_LOS                      VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(0xdc)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL_MIN_IFG_1_LOS                     VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(0xf2)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125_MIN_IFG_1_LOS                         VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(0x25d8)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH_MIN_IFG_1_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(0x1)
#define  VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH_MIN_IFG_1_LOS                       VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(0x1)




typedef enum {
    BM_PCS_INTR_MON,
    BM_PCS_INTR_MON_LOS,
    BM_PCS_PASS_MON,
    BM_PCS_PASS_MON_WIS,
    BM_PCS_PASS_MON_LOS,
    BM_PCS_BYP,
    BM_PCS_MIN_IFG_1,
    BM_PCS_MIN_IFG_1_LOS,
    BM_PCS_LAST
} block_pcs_mode_t;

#endif /* _VTSS_DAYTONA_REG_INIT_PCS_H */