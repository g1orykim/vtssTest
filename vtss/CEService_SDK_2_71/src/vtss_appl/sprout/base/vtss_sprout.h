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

 $Id$
 $Revision$


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/

#ifndef _VTSS_SPROUT_H_
#define _VTSS_SPROUT_H_

#include <vtss_module_id.h>
#include <vtss_os.h>
#include <port_custom_api.h>
#include <vtss_api_if_api.h>


#if !defined(VTSS_SWITCH)
#define VTSS_SWITCH 0
#elif (VTSS_SWITCH != 0 && VTSS_SWITCH != 1)
#error VTSS_SWITCH must be set to 0 or 1
#endif

#if !defined(VTSS_SPROUT_UNMGD)
#define VTSS_SPROUT_UNMGD 0
#elif (VTSS_SPROUT_UNMGD != 0 && VTSS_SPROUT_UNMGD != 1)
#error VTSS_SPROUT_UNMGD must be set to 0 or 1
#endif

#if !defined(VTSS_SPROUT_MULTI_THREAD) || (VTSS_SPROUT_MULTI_THREAD != 0 && VTSS_SPROUT_MULTI_THREAD != 1)
#error VTSS_SPROUT_MULTI_THREAD must be set to 0 or 1.
#endif
#if !defined(VTSS_SPROUT_CRIT_CHK) || (VTSS_SPROUT_CRIT_CHK != 0 && VTSS_SPROUT_CRIT_CHK != 1)
#error VTSS_SPROUT_CRIT_CHK must be set to 0 or 1.
#endif

#include <main.h>




#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SPROUT

#define VTSS_TRACE_GRP_DEFAULT          0
#define TRACE_GRP_PKT_DUMP              1
#define VTSS_TRACE_GRP_TOP_CHG          2
#define TRACE_GRP_MST_ELECT             3
#define TRACE_GRP_UPSID                 4
#define TRACE_GRP_CRIT                  5
#define TRACE_GRP_FAILOVER              6
#define TRACE_GRP_STARTUP               7
#define TRACE_GRP_CNT                   8

#include <vtss_trace_api.h>


#include "vtss_sprout_api.h"


#define VTSS_SPROUT_RIT_SIZE           (VTSS_SPROUT_MAX_UNITS_IN_STACK + VTSS_SPROUT_MAX_UNITS_IN_STACK/2)

#include "vtss_sprout_types.h"
#include "vtss_sprout_crit.h"
#include "vtss_sprout_util.h"
#include "vtss_sprout_xit.h"

#ifndef VTSS_SPROUT_ASSERT
#define VTSS_SPROUT_ASSERT(expr, msg) { \
    if (!(expr)) { \
        T_E msg; \
        VTSS_ASSERT(expr); \
    } \
}
#endif

#ifndef VTSS_SPROUT_ASSERT_DBG_INFO
#define VTSS_SPROUT_ASSERT_DBG_INFO(expr, msg) { \
    if (!(expr)) { \
        T_E msg; \
        T_E("Switch state:\n%s\n", \
            vtss_sprout__switch_state_to_str(&switch_state));   \
        T_E("RIT:\n%s\n", \
            vtss_sprout__rit_to_str(&rit)); \
        T_E("UIT:\n%s\n", \
            vtss_sprout__uit_to_str(&uit)); \
        VTSS_ASSERT(expr); \
    } \
}
#endif




#define VTSS_SPROUT_MST_TIME_DIFF_MIN 10 

extern vtss_sprout__switch_state_t switch_state;
extern vtss_sprout__uit_t uit;
extern vtss_sprout__rit_t rit;

#endif 






