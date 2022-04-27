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
#ifndef _VTSS_DAYTONA_REG_INIT_BLOCK_STATES_H
#define _VTSS_DAYTONA_REG_INIT_BLOCK_STATES_H

typedef enum {
    BLOCK_GCB,
    BLOCK_GCB_CLIENT_PLL,
    BLOCK_UPI,
    BLOCK_UPI_HSS6G,
    BLOCK_XAUI,
    BLOCK_TFI,
    BLOCK_SFI,
    BLOCK_XFI_1,
    BLOCK_XFI_HSS11G_1,
    BLOCK_OTU_1,
    BLOCK_OTU_FEC_1,
    BLOCK_GFP_1,
    BLOCK_WIS_1,
    BLOCK_PCS_1,
    BLOCK_MAC_1,
    BLOCK_RAB,
    BLOCK_MAC_2,
    BLOCK_PCS_2,
    BLOCK_PTP,
    BLOCK_WIS_2,
    BLOCK_GFP_2,
    BLOCK_OTU_2,
    BLOCK_OTU_FEC_2,
    BLOCK_GCB_LINE_PLL,
    BLOCK_XFI_2,
    BLOCK_XFI_HSS11G_2,
    BLOCK_PREDICTOR,
    BLOCK_LAST
} block_name_t;

typedef enum {
    MODE_PSS,
    MODE_PSS_W,
    MODE_PSOC_A,
    MODE_PSOC_WA,
    MODE_PSOC_S,
    MODE_PSOC_WS,
    MODE_PEO_WMWS,
    MODE_PEO_WMWA,
    MODE_PEE_W,
    MODE_PEE_MW,
    MODE_XAUI_2M_PEE_MW,
    MODE_PEE_P,
    MODE_PEE_MP,
    MODE_XAUI_2M_PEE_MP,
    MODE_PFF_P,
    MODE_PEO_WA,
    MODE_PEO_MWA,
    MODE_PEO_WS,
    MODE_PEO_MWS,
    MODE_PEO_P2E_20,
    MODE_PEO_MP2E_20,
    MODE_PEO_P2E_100,
    MODE_PEO_P1E_100,
    MODE_XAUI_2M_PEO_P1E_100,
    MODE_PFO_P2F,
    MODE_PGO_2,
    MODE_PGO_2SUB,
    MODE_PGO_2SUBSMALL,
    MODE_PGO_2GEN,
    MODE_BOO_2,
    MODE_BOO_1E,
    MODE_BOO_2E,
    MODE_BOO_2F,
    MODE_BMX_2,
    MODE_BMX_2E,
    MODE_BMX_1E,
    MODE_BMX_2F,
    MODE_BOE_WMWA,
    MODE_BOE_WMWS,
    MODE_BOE_PMP2E_20,
    MODE_BOE_PMP1E_20,
    MODE_BOEC_2E,
    MODE_BOEC_1E,
    MODE_BOG_2,
    MODE_BOG_2SUB,
    MODE_BOG_W2,
    MODE_BOSC_A,
    MODE_BOSC_WA,
    MODE_BOSC_S,
    MODE_BOSC_WS,
    MODE_BOFC_2F,
    MODE_BO8F,
    MODE_TSOC_A,
    MODE_TSOC_WA,
    MODE_TSOC_S,
    MODE_TSOC_WS,
    MODE_TEO_PMP2E,
    MODE_TEO_PMP1E,
    MODE_TEO_PP2E,
    MODE_TFO_PP2F,
    MODE_TEOC_2E,
    MODE_TEOC_1E,
    MODE_TFOC_2F,
    MODE_TEO_WMWA,
    MODE_TEO_WMWS,
    MODE_TGO_2,
    MODE_TGO_2SUB,
    MODE_TGO_2SUBSMALL,
    MODE_TGO_W2,
    MODE_T8FO,
    MODE_TOO_2,
    MODE_TOO_2E2,
    MODE_TOO_1E2,
    MODE_TOO_2F2,
    MODE_TEE_L,
    MODE_TEE_PMP_S,
    MODE_TEE_PMP_A,
    MODE_TEE_PMW,
    MODE_TEE_PP,
    MODE_TEE_W,
    MODE_TFF,
    MODE_TSS,
    MODE_XAUI_2M_PEO_MWA,
    MODE_DISABLED,
    MODE_LAST
} mode_name_t;

#endif /* _VTSS_DAYTONA_REG_INIT_BLOCK_STATES_H */
