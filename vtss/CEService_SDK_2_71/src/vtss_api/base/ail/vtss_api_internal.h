/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#ifndef _VTSS_API_INTERNAL_H_
#define _VTSS_API_INTERNAL_H_

#include "vtss_api.h"

/* PHY Interrupt status/control - should we share this ? */
#define PHY_INTERRUPT_MASK_REG        25
#define PHY_INTERRUPT_PENDING_REG     26

#define PHY_INT_MASK              0x8000
#define PHY_LINK_MASK             0x2000
#define PHY_FAST_LINK_MASK        0x0080
#define PHY_AMS_MASK              0x0010

#endif /* _VTSS_API_INTERNAL_H_ */
