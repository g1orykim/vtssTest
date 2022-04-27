/*

 Vitesse Switch Software.

==========================================================================

      Vitesse Switch Software

      Register Definitions

==========================================================================
 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####
 ==========================================================================
 #####DESCRIPTIONBEGIN####

 Author(s):    lpovlsen
 Contributors: 
 Date:         2011-10-18
 Purpose:      HAL register definitions
 Description:  This file contains register definitions used by the
               HAL.

 ####DESCRIPTIONEND####

*/
#ifndef _VTSS_LUTON26_REGS_DEV_CMN_H_
#define _VTSS_LUTON26_REGS_DEV_CMN_H_

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
