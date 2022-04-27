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

#ifndef __ICLI_PORTING_TRACE_H__
#define __ICLI_PORTING_TRACE_H__

#ifdef ICLI_TARGET

#include "vtss_trace_api.h"
#include "vtss_module_id.h"
#include "vtss_trace_lvl_api.h"

#ifdef VTSS_TRACE_MODULE_ID
#undef VTSS_TRACE_MODULE_ID
#endif

#ifdef VTSS_TRACE_GRP_DEFAULT
#undef VTSS_TRACE_GRP_DEFAULT
#endif

#ifdef TRACE_GRP_CRIT
#undef TRACE_GRP_CRIT
#endif

#ifdef TRACE_GRP_CNT
#undef TRACE_GRP_CNT
#endif

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_ICLI
#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2


#include <vtss_trace_api.h>

#else // ICLI_TARGET

#ifndef T_E
#define T_E(...) \
    printf("Error: %s, %s, %d, ", __FILE__, __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__);
#endif

#ifndef T_W
#define T_W(...) \
    printf("Warning: %s, %s, %d, ", __FILE__, __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__);
#endif

#endif // ICLI_TARGET

#endif  /* __ICLI_PORTING_TRACE_H__ */

