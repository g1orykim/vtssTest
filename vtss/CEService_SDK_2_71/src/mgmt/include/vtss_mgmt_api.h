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

#ifndef _VTSS_MGMT_API_H_
#define _VTSS_MGMT_API_H_

/* First module specific error code. Can be used as first entry in error type enumeration */
#define MODULE_ERROR_START(module_id) (-((module_id<<16) | 0xffff))

/* Decompose return code into module_id and error code */
#define VTSS_RC_GET_MODULE_ID(rc)     (((-rc) >> 16) & 0xffff)
#define VTSS_RC_GET_MODULE_CODE(rc)   (-(rc & 0xffff))

#include "vtss_module_id.h"
#include "vtss_api.h"

#ifdef VTSS_SW_OPTION_PORT
#include "vtss_mgmt_port_api.h"
#endif  /* VTSS_SW_OPTION_PORT */

#endif // _VTSS_MGMT_API_H_


// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
