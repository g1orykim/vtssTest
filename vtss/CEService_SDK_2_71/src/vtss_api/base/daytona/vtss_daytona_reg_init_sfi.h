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
#ifndef _VTSS_DAYTONA_REG_INIT_SFI_H
#define _VTSS_DAYTONA_REG_INIT_SFI_H

// Settings for mode SDR

#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA_SDR                                         VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL_SDR                                      VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX_SDR                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX(0x3f)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA_SDR                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS_SDR                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA_SDR                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA_SDR                                VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA_SDR                                           VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL_SDR                                           VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA_SDR                                          VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL_SDR                                          VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA_SDR                                    VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA_SDR                                      VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA(0x1)


// Settings for mode DDR

#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA_DDR                                         VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL_DDR                                      VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX_DDR                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX(0x3f)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA_DDR                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS_DDR                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA_DDR                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA_DDR                                VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA_DDR                                           VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL_DDR                                           VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA_DDR                                          VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL_DDR                                          VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA_DDR                                    VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA_DDR                                      VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA(0x1)


// Settings for mode UNUSED

#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA_UNUSED                                      VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL_UNUSED                                   VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX_UNUSED                                    VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX(0x3f)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA_UNUSED                                    VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS_UNUSED                                     VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA_UNUSED                                     VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA_UNUSED                             VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA(0x0)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA_UNUSED                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL_UNUSED                                        VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SD_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA_UNUSED                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL_UNUSED                                       VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_LOS_POL(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA_UNUSED                                 VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_FAST_SYNC_ENA(0x1)
#define  VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA_UNUSED                                   VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CL49_ENA(0x1)




typedef enum {
    BM_SFI_SDR,
    BM_SFI_DDR,
    BM_SFI_UNUSED,
    BM_SFI_LAST
} block_sfi_mode_t;

#endif /* _VTSS_DAYTONA_REG_INIT_SFI_H */
