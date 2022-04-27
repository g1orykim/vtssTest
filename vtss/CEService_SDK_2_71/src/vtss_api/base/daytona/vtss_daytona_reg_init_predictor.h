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
#ifndef _VTSS_DAYTONA_REG_INIT_PREDICTOR_H
#define _VTSS_DAYTONA_REG_INIT_PREDICTOR_H

// Settings for mode UNUSED

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_UNUSED                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_UNUSED                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_UNUSED                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_UNUSED                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_UNUSED                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x18d)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_UNUSED                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_UNUSED                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_UNUSED                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_UNUSED                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_UNUSED                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_UNUSED                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_UNUSED                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_UNUSED                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1f07)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_UNUSED                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_UNUSED                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_UNUSED                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x18d)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_UNUSED                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEE_MW

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_MW                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_MW                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_MW                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_MW                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x337)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_MW                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x19b)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_MW                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_MW                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_MW                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_MW                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_MW                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_MW                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_MW                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_MW                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x2026)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_MW                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x337)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_MW                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_MW                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x19b)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_MW                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEE_W

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_W                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_W                                               VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_W                                               VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_W                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x337)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_W                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x19b)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_W                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_W                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_W                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_W                                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_W                                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_W                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_W                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_W                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x2026)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_W                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x337)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_W                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_W                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x19b)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_W                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEE_MP

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_MP                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_MP                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_MP                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_MP                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_MP                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x18d)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_MP                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_MP                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_MP                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_MP                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_MP                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_MP                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_MP                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_MP                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1f07)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_MP                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_MP                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_MP                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x18d)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_MP                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEE_P

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_P                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_P                                               VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_P                                               VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_P                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_P                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x18d)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_P                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_P                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_P                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_P                                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_P                                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_P                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_P                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_P                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1f07)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_P                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x31a)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_P                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_P                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x18d)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_P                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_MWa

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MWa                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MWa                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MWa                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MWa                                            VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MWa                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x17e)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MWa                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MWa                                      VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MWa                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MWa                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MWa                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MWa                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MWa                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MWa                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1de1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MWa                                             VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MWa                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MWa                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x17e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MWa                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_Wa

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_Wa                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_Wa                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_Wa                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_Wa                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_Wa                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x17e)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_Wa                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_Wa                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_Wa                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_Wa                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_Wa                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_Wa                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_Wa                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_Wa                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1de1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_Wa                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_Wa                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_Wa                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x17e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_Wa                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_MWs

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MWs                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MWs                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MWs                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MWs                                            VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MWs                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x17e)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MWs                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MWs                                      VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MWs                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MWs                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MWs                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MWs                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MWs                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MWs                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1de1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MWs                                             VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MWs                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MWs                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x17e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MWs                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_Ws

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_Ws                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_Ws                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_Ws                                              VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x8)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_Ws                                             VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_Ws                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x17e)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_Ws                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_Ws                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_Ws                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_Ws                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_Ws                                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x1e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_Ws                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x870)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_Ws                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x50)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_Ws                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1de1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_Ws                                              VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2fc)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_Ws                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_Ws                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x17e)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_Ws                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_MP2e_20

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MP2e_20                                   VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MP2e_20                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MP2e_20                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MP2e_20                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MP2e_20                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x171)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MP2e_20                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MP2e_20                                  VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MP2e_20                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MP2e_20                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MP2e_20                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MP2e_20                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MP2e_20                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MP2e_20                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cd7)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MP2e_20                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MP2e_20                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MP2e_20                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x171)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MP2e_20                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_P2e_20

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P2e_20                                    VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P2e_20                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P2e_20                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P2e_20                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P2e_20                                      VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x171)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P2e_20                                      VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P2e_20                                   VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P2e_20                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P2e_20                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P2e_20                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P2e_20                                 VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P2e_20                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P2e_20                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cd7)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P2e_20                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P2e_20                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P2e_20                                       VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x171)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P2e_20                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_P2e_100

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P2e_100                                   VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P2e_100                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P2e_100                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P2e_100                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e5)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P2e_100                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x172)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P2e_100                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P2e_100                                  VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P2e_100                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P2e_100                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P2e_100                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P2e_100                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P2e_100                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P2e_100                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cf6)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P2e_100                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e5)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P2e_100                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P2e_100                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x172)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P2e_100                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode PEO_P1e_100

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P1e_100                                   VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P1e_100                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P1e_100                                         VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P1e_100                                        VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P1e_100                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x171)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P1e_100                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P1e_100                                  VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P1e_100                                    VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P1e_100                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P1e_100                                          VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P1e_100                                VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P1e_100                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P1e_100                               VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cd7)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P1e_100                                         VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P1e_100                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P1e_100                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x171)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P1e_100                                   VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode TEO_PMP2e

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_TEO_PMP2e                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_TEO_PMP2e                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_TEO_PMP2e                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_TEO_PMP2e                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_TEO_PMP2e                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x171)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_TEO_PMP2e                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_TEO_PMP2e                                    VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_TEO_PMP2e                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_TEO_PMP2e                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_TEO_PMP2e                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_TEO_PMP2e                                  VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_TEO_PMP2e                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_TEO_PMP2e                                 VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cd7)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_TEO_PMP2e                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_TEO_PMP2e                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_TEO_PMP2e                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x171)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_TEO_PMP2e                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)


// Settings for mode TEO_PMP1e

#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_TEO_PMP1e                                     VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE(0x1)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_TEO_PMP1e                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_PAF(0x38)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_TEO_PMP1e                                           VTSS_F_IP_1588_INGR_PREDICTOR_IG_CFG_WAF(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_TEO_PMP1e                                          VTSS_F_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_TEO_PMP1e                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB(0x171)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_TEO_PMP1e                                       VTSS_F_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB(0x0)
#define  VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_TEO_PMP1e                                    VTSS_F_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_TEO_PMP1e                                      VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE(0x1)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_TEO_PMP1e                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_PAF(0x18)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_TEO_PMP1e                                            VTSS_F_IP_1588_EGR_PREDICTOR_EG_CFG_WAF(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_TEO_PMP1e                                  VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_TEO_PMP1e                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_TEO_PMP1e                                 VTSS_F_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS(0x1cd7)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_TEO_PMP1e                                           VTSS_F_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA(0x2e2)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_TEO_PMP1e                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB(0x0)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_TEO_PMP1e                                        VTSS_F_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB(0x171)
#define  VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_TEO_PMP1e                                     VTSS_F_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD(0x0)




typedef enum {
    BM_PREDICTOR_UNUSED,
    BM_PREDICTOR_PEE_MW,
    BM_PREDICTOR_PEE_W,
    BM_PREDICTOR_PEE_MP,
    BM_PREDICTOR_PEE_P,
    BM_PREDICTOR_PEO_MWA,
    BM_PREDICTOR_PEO_WA,
    BM_PREDICTOR_PEO_MWS,
    BM_PREDICTOR_PEO_WS,
    BM_PREDICTOR_PEO_MP2E_20,
    BM_PREDICTOR_PEO_P2E_20,
    BM_PREDICTOR_PEO_P2E_100,
    BM_PREDICTOR_PEO_P1E_100,
    BM_PREDICTOR_TEO_PMP2E,
    BM_PREDICTOR_TEO_PMP1E,
    BM_PREDICTOR_LAST
} block_predictor_mode_t;

#endif /* _VTSS_DAYTONA_REG_INIT_PREDICTOR_H */
