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

#define TFI_TBL_SIZE   2
 static static_cfg_t tfi_sdr_tbl[2] = { {DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(0), (VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN_SDR | VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN_SDR), (VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN | VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN)},
				{DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_RX_RX_CONTROL(0), (VTSS_TFI_5_RX_RX_CONTROL_SYNC_SEL_SDR), (VTSS_M_TFI_5_RX_RX_CONTROL_SYNC_SEL)} };


// register structures for mode DDR

static static_cfg_t tfi_ddr_tbl[2] = { {DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(0), (VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN_DDR | VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN_DDR), (VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN | VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN)},
				{DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_RX_RX_CONTROL(0), (VTSS_TFI_5_RX_RX_CONTROL_SYNC_SEL_DDR), (VTSS_M_TFI_5_RX_RX_CONTROL_SYNC_SEL)} };


// register structures for mode UNUSED

static static_cfg_t tfi_unused_tbl[2] = { {DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(0), (VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN_UNUSED | VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN_UNUSED), (VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN | VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN)},
				{DAYTONA_BLOCK_TFI_5, VTSS_TFI_5_RX_RX_CONTROL(0), (VTSS_TFI_5_RX_RX_CONTROL_SYNC_SEL_UNUSED), (VTSS_M_TFI_5_RX_RX_CONTROL_SYNC_SEL)} };


static const static_cfg_t *tfi_config_table[BM_TFI_LAST] = {
    tfi_sdr_tbl,
    tfi_ddr_tbl,
    tfi_unused_tbl,
};

