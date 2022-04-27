/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_IP_SOURCE_GUARD_H_
#define _VTSS_IP_SOURCE_GUARD_H_


/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "ip_source_guard_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#include "acl_api.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP_SOURCE_GUARD

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>


/* ================================================================= *
 *  IP_SOURCE_GUARD configuration blocks
 * ================================================================= */

/* Block versions */
#define IP_SOURCE_GUARD_CONF_BLK_VERSION      1

/* IP_SOURCE_GUARD ACE IDs */
#define IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, port_no) (((isid) - VTSS_ISID_START) * VTSS_PORTS + (port_no) - VTSS_PORT_NO_START + 1)

/* IP_SOURCE_GUARD configuration block */
typedef struct {
    unsigned long               version;                /* Block version */
    ip_source_guard_conf_t      ip_source_guard_conf;   /* IP_SOURCE_GUARD configuration */
} ip_source_guard_conf_blk_t;


/* ================================================================= *
 *  IP_SOURCE_GUARD global structure
 * ================================================================= */

/* IP_SOURCE_GUARD global structure */
typedef struct {
    critd_t                         crit;
    ip_source_guard_conf_t          ip_source_guard_conf;
    ip_source_guard_entry_t         ip_source_guard_dynamic_entry[IP_SOURCE_GUARD_MAX_ENTRY_CNT];
    uchar                           id_used[VTSS_BF_SIZE(ACE_ID_END)];
} ip_source_guard_global_t;

#endif /* _VTSS_IP_SOURCE_GUARD_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
