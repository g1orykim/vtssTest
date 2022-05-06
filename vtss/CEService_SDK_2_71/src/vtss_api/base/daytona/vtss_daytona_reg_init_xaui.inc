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
// register structures for mode SDR

#define XAUI_TBL_SIZE   2
 static static_cfg_t xaui_sdr_tbl[2] = { {DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA_SDR), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA)},
				{DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_SDR | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA_SDR | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA_SDR | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS_SDR | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS_SDR), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS)} };


// register structures for mode DDR_A

static static_cfg_t xaui_ddr_a_tbl[2] = { {DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA_DDR_A), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA)},
				{DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_DDR_A | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA_DDR_A | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA_DDR_A | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS_DDR_A | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS_DDR_A), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS)} };


// register structures for mode DDR_K

static static_cfg_t xaui_ddr_k_tbl[2] = { {DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA_DDR_K), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA)},
				{DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_DDR_K | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA_DDR_K | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA_DDR_K | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS_DDR_K | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS_DDR_K), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS)} };


// register structures for mode DDR_M

static static_cfg_t xaui_ddr_m_tbl[2] = { {DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA_DDR_M), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA)},
				{DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_DDR_M | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA_DDR_M | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA_DDR_M | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS_DDR_M | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS_DDR_M), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS)} };


// register structures for mode UNUSED

static static_cfg_t xaui_unused_tbl[2] = { {DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA_UNUSED), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA)},
				{DAYTONA_BLOCK_PCS_XAUI, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(0), (VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_UNUSED | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA_UNUSED | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA_UNUSED | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS_UNUSED | VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS_UNUSED), (VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS | VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS)} };


static const static_cfg_t *xaui_config_table[BM_XAUI_LAST] = {
    xaui_sdr_tbl,
    xaui_ddr_a_tbl,
    xaui_ddr_k_tbl,
    xaui_ddr_m_tbl,
    xaui_unused_tbl,
};
