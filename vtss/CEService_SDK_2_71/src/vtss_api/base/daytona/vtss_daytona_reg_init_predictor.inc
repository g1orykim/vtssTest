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
// register structures for mode UNUSED

#define PREDICTOR_TBL_SIZE   10
 static static_cfg_t predictor_unused_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_UNUSED | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_UNUSED | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_UNUSED), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_UNUSED), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_UNUSED | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_UNUSED), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_UNUSED), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_UNUSED | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_UNUSED | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_UNUSED | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_UNUSED | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_UNUSED), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEE_MW

static static_cfg_t predictor_pee_mw_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_MW | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_MW | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_MW), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_MW), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_MW | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_MW), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_MW), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_MW | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_MW | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_MW | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_MW | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_MW), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEE_W

static static_cfg_t predictor_pee_w_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_W | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_W | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_W), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_W), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_W | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_W), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_W), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_W | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_W | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_W | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_W | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_W), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEE_MP

static static_cfg_t predictor_pee_mp_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_MP | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_MP | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_MP), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_MP), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_MP | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_MP), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_MP), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_MP | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_MP | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_MP | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_MP | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_MP), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEE_P

static static_cfg_t predictor_pee_p_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEE_P | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEE_P | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEE_P), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEE_P), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEE_P | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEE_P), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEE_P), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEE_P | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEE_P | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEE_P | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEE_P | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEE_P), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_MWa

static static_cfg_t predictor_peo_mwa_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MWa | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MWa | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MWa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MWa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MWa | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MWa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MWa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MWa | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MWa | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MWa | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MWa | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MWa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_Wa

static static_cfg_t predictor_peo_wa_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_Wa | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_Wa | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_Wa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_Wa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_Wa | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_Wa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_Wa), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_Wa | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_Wa | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_Wa | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_Wa | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_Wa), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_MWs

static static_cfg_t predictor_peo_mws_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MWs | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MWs | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MWs), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MWs), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MWs | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MWs), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MWs), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MWs | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MWs | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MWs | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MWs | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MWs), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_Ws

static static_cfg_t predictor_peo_ws_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_Ws | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_Ws | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_Ws), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_Ws), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_Ws | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_Ws), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_Ws), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_Ws | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_Ws | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_Ws | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_Ws | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_Ws), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_MP2e_20

static static_cfg_t predictor_peo_mp2e_20_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_MP2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_MP2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_MP2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_MP2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_MP2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_MP2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_MP2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_MP2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_MP2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_MP2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_MP2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_MP2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_P2e_20

static static_cfg_t predictor_peo_p2e_20_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P2e_20 | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P2e_20), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P2e_20 | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P2e_20), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_P2e_100

static static_cfg_t predictor_peo_p2e_100_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P2e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P2e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P2e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P2e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P2e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P2e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P2e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P2e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P2e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P2e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P2e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P2e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode PEO_P1e_100

static static_cfg_t predictor_peo_p1e_100_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_PEO_P1e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_PEO_P1e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_PEO_P1e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_PEO_P1e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_PEO_P1e_100 | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_PEO_P1e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_PEO_P1e_100), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_PEO_P1e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_PEO_P1e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_PEO_P1e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_PEO_P1e_100 | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_PEO_P1e_100), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode TEO_PMP2e

static static_cfg_t predictor_teo_pmp2e_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_TEO_PMP2e | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_TEO_PMP2e | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_TEO_PMP2e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_TEO_PMP2e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_TEO_PMP2e | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_TEO_PMP2e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_TEO_PMP2e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_TEO_PMP2e | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_TEO_PMP2e | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_TEO_PMP2e | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_TEO_PMP2e | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_TEO_PMP2e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


// register structures for mode TEO_PMP1e

static static_cfg_t predictor_teo_pmp1e_tbl[10] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_CFG(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE_TEO_PMP1e | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_PAF_TEO_PMP1e | VTSS_IP_1588_INGR_PREDICTOR_IG_CFG_WAF_TEO_PMP1e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_IG_ENABLE | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_PAF | VTSS_M_IP_1588_INGR_PREDICTOR_IG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_PMA(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA_TEO_PMP1e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_XFI(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB_TEO_PMP1e | VTSS_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB_TEO_PMP1e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_LSB | VTSS_M_IP_1588_INGR_PREDICTOR_IG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_INGR_PREDICTOR_IG_OTN(0), (VTSS_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD_TEO_PMP1e), (VTSS_M_IP_1588_INGR_PREDICTOR_IG_OTN_GAP_PERIOD)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_CFG(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE_TEO_PMP1e | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_PAF_TEO_PMP1e | VTSS_IP_1588_EGR_PREDICTOR_EG_CFG_WAF_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_EG_ENABLE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_PAF | VTSS_M_IP_1588_EGR_PREDICTOR_EG_CFG_WAF)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE_TEO_PMP1e | VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_FSIZE | VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_FRAME_W_OH)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_WIS_DELAYS_W_OH_NS)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_PMA(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_PMA_TPMA)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_XFI(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB_TEO_PMP1e | VTSS_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_LSB | VTSS_M_IP_1588_EGR_PREDICTOR_EG_XFI_XFI_MSB)},
				{DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_EGR_PREDICTOR_EG_OTN(0), (VTSS_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD_TEO_PMP1e), (VTSS_M_IP_1588_EGR_PREDICTOR_EG_OTN_GAP_PERIOD)} };


static const static_cfg_t *predictor_config_table[BM_PREDICTOR_LAST] = {
    predictor_unused_tbl,
    predictor_pee_mw_tbl,
    predictor_pee_w_tbl,
    predictor_pee_mp_tbl,
    predictor_pee_p_tbl,
    predictor_peo_mwa_tbl,
    predictor_peo_wa_tbl,
    predictor_peo_mws_tbl,
    predictor_peo_ws_tbl,
    predictor_peo_mp2e_20_tbl,
    predictor_peo_p2e_20_tbl,
    predictor_peo_p2e_100_tbl,
    predictor_peo_p1e_100_tbl,
    predictor_teo_pmp2e_tbl,
    predictor_teo_pmp1e_tbl,
};

