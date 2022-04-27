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
// register structures for mode WORK

#define XFI_TBL_SIZE   1
 static static_cfg_t xfi_work_tbl[1] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_XFI_MODE(0), (VTSS_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC_WORK), (VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC)} };


// register structures for mode PROT

static static_cfg_t xfi_prot_tbl[1] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_XFI_MODE(0), (VTSS_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC_PROT), (VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC)} };


// register structures for mode W_P

static static_cfg_t xfi_w_p_tbl[1] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_XFI_MODE(0), (VTSS_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC_W_P), (VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC)} };


// register structures for mode UNUSED

static static_cfg_t xfi_unused_tbl[1] = { {DAYTONA_BLOCK_XFI, VTSS_XFI_XFI_CONTROL_XFI_MODE(0), (VTSS_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC_UNUSED), (VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC)} };


static const static_cfg_t *xfi_config_table[BM_XFI_LAST] = {
    xfi_work_tbl,
    xfi_prot_tbl,
    xfi_w_p_tbl,
    xfi_unused_tbl,
};

