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

#ifndef __MPLS_H__
#define __MPLS_H__

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include "critd_api.h"

#ifndef VTSS_TRACE_MODULE_ID
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MPLS

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2
#endif

#include <vtss_trace_api.h>

extern critd_t vtss_mpls_crit;

#if (VTSS_TRACE_ENABLED)
#define MPLS_CRIT_ENTER() critd_enter(&vtss_mpls_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MPLS_CRIT_EXIT()  critd_exit( &vtss_mpls_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define MPLS_CRIT_ENTER() critd_enter(&vtss_mpls_crit)
#define MPLS_CRIT_EXIT()  critd_exit( &vtss_mpls_crit)
#endif /* VTSS_TRACE_ENABLED */

#endif /* __MPLS_H__ */

