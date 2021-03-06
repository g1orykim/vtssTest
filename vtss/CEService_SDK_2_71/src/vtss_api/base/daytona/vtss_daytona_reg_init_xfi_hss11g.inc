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
// register structures for mode OTU2

#define XFI_HSS11G_TBL_SIZE   11
 static static_cfg_t xfi_hss11g_otu2_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU2_X4

static static_cfg_t xfi_hss11g_otu2_x4_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2_X4 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2_X4), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU2E

static static_cfg_t xfi_hss11g_otu2e_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2E | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2E), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2E), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2E), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU2E_X4

static static_cfg_t xfi_hss11g_otu2e_x4_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2E_X4 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2E_X4), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU1E

static static_cfg_t xfi_hss11g_otu1e_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU1E | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU1E), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU1E), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU1E), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU1E), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU1E_X4

static static_cfg_t xfi_hss11g_otu1e_x4_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU1E_X4 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU1E_X4), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU1E_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU2F

static static_cfg_t xfi_hss11g_otu2f_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2F | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2F), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2F), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2F), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2F), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode OTU2F_X4

static static_cfg_t xfi_hss11g_otu2f_x4_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_OTU2F_X4 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_OTU2F_X4), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_OTU2F_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode SONET

static static_cfg_t xfi_hss11g_sonet_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_SONET | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_SONET), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_SONET), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_SONET), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_SONET), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode SONET_X4

static static_cfg_t xfi_hss11g_sonet_x4_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_SONET_X4 | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_SONET_X4), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_SONET_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_SONET_X4), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_SONET_X4), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode 10GELAN

static static_cfg_t xfi_hss11g_10gelan_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_10GELAN | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_10GELAN), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_10GELAN), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_10GELAN), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GELAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode 10GEWAN

static static_cfg_t xfi_hss11g_10gewan_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_10GEWAN | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_10GEWAN), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_10GEWAN), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_10GEWAN), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GEWAN), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode 10GFC

static static_cfg_t xfi_hss11g_10gfc_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_10GFC | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_10GFC), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_10GFC), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_10GFC), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_10GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode 8GFC

static static_cfg_t xfi_hss11g_8gfc_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_8GFC | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_8GFC), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_8GFC), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_8GFC), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_8GFC), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


// register structures for mode UNUSED

static static_cfg_t xfi_hss11g_unused_tbl[11] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(0), (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL_UNUSED | VTSS_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL_UNUSED), (VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL | VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL_UNUSED), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(0, 2), (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL_UNUSED), (VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(0, 2), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 0), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)},
				{DAYTONA_BLOCK_XFI, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(0, 1), (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL_UNUSED), (VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL)} };


static const static_cfg_t *xfi_hss11g_config_table[BM_XFI_HSS11G_LAST] = {
    xfi_hss11g_otu2_tbl,
    xfi_hss11g_otu2_x4_tbl,
    xfi_hss11g_otu2e_tbl,
    xfi_hss11g_otu2e_x4_tbl,
    xfi_hss11g_otu1e_tbl,
    xfi_hss11g_otu1e_x4_tbl,
    xfi_hss11g_otu2f_tbl,
    xfi_hss11g_otu2f_x4_tbl,
    xfi_hss11g_sonet_tbl,
    xfi_hss11g_sonet_x4_tbl,
    xfi_hss11g_10gelan_tbl,
    xfi_hss11g_10gewan_tbl,
    xfi_hss11g_10gfc_tbl,
    xfi_hss11g_8gfc_tbl,
    xfi_hss11g_unused_tbl,
};

