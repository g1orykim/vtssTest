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

 $Id$
 $Revision$

*/

#ifndef _TOPO_H_
#define _TOPO_H_

#include "main.h"
#include <vtss_module_id.h>




#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_TOPO

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CFG          1
#define TRACE_GRP_RXPKT_DUMP   2
#define TRACE_GRP_TXPKT_DUMP   3
#define TRACE_GRP_CRIT         4
#define TRACE_GRP_UPSID        5
#define TRACE_GRP_FAILOVER     6
#define TRACE_GRP_CNT          7

#include <vtss_trace_api.h>


#include "version.h"
#include "critd_api.h"
#include "conf_api.h"
#include "port_api.h"
#include "topo_api.h"
#ifdef VTSS_SW_OPTION_MAC
#include "mac_api.h"
#endif
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "vtss_sprout_api.h"
#include "misc_api.h"
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif
#include "led_api.h"
#include "msg_api.h"
#include "interrupt_api.h"
#include "vtss_api_if_api.h"

#define TOPO_ASSERT(expr, fmt, ...) { \
    if (!(expr)) { \
        T_E("ASSERTION FAILED"); \
        T_E(fmt, ##__VA_ARGS__); \
        VTSS_ASSERT(expr); \
    } \
}


#define TOPO_ASSERTR(expr, fmt, ...) { \
    if (!(expr)) { \
        TOPO_ASSERT(expr, fmt, ##__VA_ARGS__); \
        return TOPO_ASSERT_FAILURE; \
    } \
}

#define TOPO_ASSERT_RC_OK(rc, fmt, ...) { \
     TOPO_ASSERT((rc) >= 0, fmt, ##__VA_ARGS__)

#undef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)    ((a) > (b) ? (b) : (a))



#define TOPO_SPROUT_MST_ELECT_PRIO_DEFAULT  VTSS_SPROUT_MST_ELECT_PRIO_DEFAULT
#define TOPO_SPROUT_UPDATE_INTERVAL_DEFAULT VTSS_SPROUT_UPDATE_INTERVAL_DEFAULT
#define TOPO_SPROUT_AGE_TIME_DEFAULT        VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT
#define TOPO_SPROUT_LIMIT_DEFAULT           VTSS_SPROUT_UPDATE_LIMIT_DEFAULT


#define TOPO_FAST_MAC_AGE_TIME       VTSS_SPROUT_FAST_MAC_AGING_TIMER

#define TOPO_FAST_MAC_AGE_COUNT      (VTSS_SPROUT_FAST_MAC_AGING_PERIOD/\
                                      VTSS_SPROUT_FAST_MAC_AGING_TIMER)

#endif 







