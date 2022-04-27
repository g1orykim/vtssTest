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
// register structures for mode BYP

#define PTP_TBL_SIZE   1
 static static_cfg_t ptp_byp_tbl[1] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL(0), (VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS_BYP | VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL_BYP), (VTSS_M_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS | VTSS_M_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL)} };


// register structures for mode UNUSED

static static_cfg_t ptp_unused_tbl[1] = { {DAYTONA_BLOCK_IP_1588, VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL(0), (VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS_UNUSED | VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL_UNUSED), (VTSS_M_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS | VTSS_M_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL)} };


static const static_cfg_t *ptp_config_table[BM_PTP_LAST] = {
    ptp_byp_tbl,
    ptp_unused_tbl,
};

