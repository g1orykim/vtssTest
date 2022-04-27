/*

 Vitesse Switch Application software.

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
//----------------------------------------------------------------------------
/**
 *  \file
 *      dhcp_server.h
 *
 *  \brief
 *      Platform-dependent definitions
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:48
 */
//----------------------------------------------------------------------------
#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <network.h>

#include "vtss_trace_api.h"
#include "vtss_module_id.h"
#include "vtss_trace_lvl_api.h"

#define TRACE_GRP_CNT           2
#define VTSS_TRACE_GRP_DEFAULT  0
#define TRACE_GRP_CRIT          1

/*
==============================================================================

    Constant

==============================================================================
*/
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_DHCP_SERVER

/*
==============================================================================

    Macro

==============================================================================
*/

/*
==============================================================================

    Type Definition

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/

//----------------------------------------------------------------------------
#endif //__DHCP_SERVER_H__
