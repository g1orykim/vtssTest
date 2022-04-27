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



#ifndef _VTSS_MIRROR_H_
#define _VTSS_MIRROR_H_

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

/* ================================================================= *
 * Trace definitions
 * ================================================================= */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MIRROR
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CLI          2
#define TRACE_GRP_CNT          3

#include <vtss_trace_api.h>
/* ============== */
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MIRROR

#include "critd_api.h"
/* messages IDs */
typedef enum {
    MIRROR_MSG_ID_CONF_SET_REQ,     /* Configuration set request (no reply) */
} mirror_msg_id_t;


/* Mac message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    uchar         *msg; /* Message */
} mirror_msg_buf_t;

typedef struct {
    vtss_port_no_t mirror_port;                 /* Mirroring port. Port 0 is disable mirroring */
    BOOL           src_enable[VTSS_PORTS];    // Enable for source mirroring
    BOOL           dst_enable[VTSS_PORTS];    // Enable for detination mirroring
#ifdef VTSS_FEATURE_MIRROR_CPU
    BOOL           cpu_dst_enable;            // Enable for CPU source mirroring
    BOOL           cpu_src_enable;            // Enable for CPU destination mirroring
#endif
}  mirror_local_switch_conf_t;

// Message for configuration
typedef struct {
    mirror_msg_id_t             msg_id;        /* Message ID: MAC_AGE_MSG_ID_CONFIG_SET_REQ */
    mirror_local_switch_conf_t  conf;          /* Configuration */
} mirror_conf_set_req_t;

/* Mirror configuration */
typedef struct {
    ulong                version; /* Block version */
    vtss_isid_t    mirror_switch;              // Switch for the switch with the active mirror port.
    vtss_port_no_t mirror_port;                 /* Mirroring port. Setting port to  VTSS_PORT_NO_NONE disables mirroring */
    BOOL           src_enable[VTSS_ISID_CNT][VTSS_PORTS];    // Enable for source mirroring
    BOOL           dst_enable[VTSS_ISID_CNT][VTSS_PORTS];    // Enable for detination mirroring
#ifdef VTSS_FEATURE_MIRROR_CPU
    BOOL           cpu_src_enable[VTSS_ISID_CNT];    // Enable for CPU source mirroring
    BOOL           cpu_dst_enable[VTSS_ISID_CNT];    // Enable for CPU destination mirroring
#endif
} mirror_stack_conf_t;

#define MIRROR_CONF_VERSION 1

typedef struct {
    mirror_stack_conf_t stack_conf; // configuration for all switches in the stack

    /* Request buffer and semaphore. Using the largest of the lot. */
    struct {
        vtss_os_sem_t    sem;
        uchar            msg[sizeof(mirror_conf_set_req_t)];
    } request;

    /* Reply buffer and semaphore. Using the largest of the lot. */
    struct {
        vtss_os_sem_t    sem;
        uchar            msg[sizeof(mirror_conf_set_req_t)];
    } reply;

} mirror_global_t;
#endif /* _VTSS_MIRROR_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
