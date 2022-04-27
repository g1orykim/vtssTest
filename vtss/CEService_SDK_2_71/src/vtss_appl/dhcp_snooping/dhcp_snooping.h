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

#ifndef _DHCP_SNOOPING_H_
#define _DHCP_SNOOPING_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "dhcp_snooping_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_SNOOPING

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* DHCP snooping configuration timer */
#define DHCP_SNOOPING_CONF_TIMER     1000


/* ================================================================= *
 *  DHCP snooping configuration blocks
 * ================================================================= */

/* Block versions */
#define DHCP_SNOOPING_CONF_BLK_VERSION          1
#define DHCP_SNOOPING_PORT_CONF_BLK_VERSION     2

/* DHCP snooping configuration block */
typedef struct {
    u32                         version;    /* Block version */
    dhcp_snooping_conf_t        conf;       /* DHCP snooping configuration */
} dhcp_snooping_conf_blk_t;

/* DHCP snooping port configuration block */
typedef struct {
    u32                         version;                    /* Block version */
    dhcp_snooping_port_conf_t   port_conf[VTSS_ISID_CNT];   /* DHCP snooping port configuration */
} dhcp_snooping_port_conf_blk_t;


/* ================================================================= *
 *  DHCP snooping global structure
 * ================================================================= */

/* DHCP snooping global structure */
typedef struct {
    critd_t                     crit;
    dhcp_snooping_conf_t        conf;
    dhcp_snooping_port_conf_t   port_conf[VTSS_ISID_END];
    u32                         frame_info_cnt; /* Frame information counter */
} dhcp_snooping_global_t;

#endif /* _DHCP_SNOOPING_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
