#ifndef _VTSS_LUTON26_REGS_DEV_CMN_H_
#define _VTSS_LUTON26_REGS_DEV_CMN_H_

/*

 Vitesse Switch API software.

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

#include "vtss_luton26_regs_dev_gmii.h"
#include "vtss_luton26_regs_dev.h"

/*
 * Abstraction macros for functionally identical registers 
 * in the DEV and DEV_GMII targets.
 *
 * Caution: These macros may not work a lvalues, depending
 * on compiler and platform.
 *
 */

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_MODE_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_IFG_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_IFG_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_IFG_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_TAGS_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_TAGS_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_HDX_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_HDX_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_ENA_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_ENA_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_FC_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_FC_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MAXLEN_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_MAXLEN_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_MAXLEN_CFG(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_STICKY(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_STICKY(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_STICKY(t) )

#define VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(t) ( (t) >= VTSS_TO_DEV_10 ? VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(t) : VTSS_DEV_GMII_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(t) )


#endif /* _VTSS_LUTON26_REGS_DEV_CMN_H_ */
