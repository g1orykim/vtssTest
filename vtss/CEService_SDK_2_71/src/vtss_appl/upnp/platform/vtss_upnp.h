/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_UPNP_H_
#define _VTSS_UPNP_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */
#include "critd_api.h"
#include "vtss_upnp_api.h"
#include "vtss_module_id.h"
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>

#ifndef VTSS_BOOL
#define VTSS_BOOL
#endif

/* We have two devices on hand, D_Link DIR-635 and Accton ES3526XA, supporting UPnP.
 * Those two devices use source port 1900 to repsonse M-search messages from control
 * points. It does not make sense but we keep this flag to have the backward
 * compatibility to old control points.
 */
#define VTSS_BACKWARD_COMPATIBLE    0
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_UPNP

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

/* UPnP ACE IDs */
#define UPNP_SSDP_ACE_ID                1
#define IGMP_QUERY_ACE_ID               2

#ifdef VTSS_SW_OPTION_IP2
#define UPNP_IP_INTF_IFID_GET_NEXT(x)   (vtss_ip2_if_id_next((x), &(x)) == VTSS_OK)
#define UPNP_IP_INTF_MAX_OPST           4
#define UPNP_IP_INTF_OPST_UP(x)         ((x) ? (((x)->type == VTSS_IF_STATUS_TYPE_LINK) ? (((x)->u.link.flags&VTSS_IF_LINK_FLAG_UP) && ((x)->u.link.flags&VTSS_IF_LINK_FLAG_RUNNING)) : FALSE) : FALSE)
#define UPNP_IP_INTF_OPST_VID(x)        ((x) ? (((x)->if_id.type == VTSS_ID_IF_TYPE_VLAN) ? (x)->if_id.u.vlan : 0) : 0)
#define UPNP_IP_INTF_OPST_ADR4(x)       ((x) ? (((x)->type == VTSS_IF_STATUS_TYPE_IPV4) ? ((x)->u.ipv4.net.address) : 0) : 0)
#define UPNP_IP_INTF_OPST_GET(x, y, z)  (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_ANY, (x), UPNP_IP_INTF_MAX_OPST, &(z), (y)) == VTSS_OK)
#endif /* VTSS_SW_OPTION_IP2 */

/* ================================================================= *
 *  UPNP configuration blocks
 * ================================================================= */

/* Block versions */
#define UPNP_CONF_BLK_VERSION      1

/* UPNP configuration block */
typedef struct {
    unsigned long   version;    /* Block version */
    upnp_conf_t     upnp_conf;   /* UPNP configuration */
} upnp_conf_blk_t;

/* ================================================================= *
 *  UPNP stack messages
 * ================================================================= */

/* UPNP messages IDs */
typedef enum {
    UPNP_MSG_ID_UPNP_CONF_SET_REQ,          /* UPNP configuration set request (no reply) */
    UPNP_MSG_ID_FRAME_RX_IND,               /* Frame receive indication */
} upnp_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    upnp_msg_id_t    msg_id;

    /* Request data, depending on message ID */
    union {
        /* UPNP_MSG_ID_UPNP_CONF_SET_REQ */
        struct {
            upnp_conf_t  conf;
        } conf_set;

        /* UPNP_MSG_ID_FRAME_RX_IND */
        struct {
            size_t              len;
            unsigned long       vid;
            unsigned long       port_no;
        } rx_ind;

    } req;
} upnp_msg_req_t;

/* UPNP message buffer */
typedef struct {
    vtss_os_sem_t   *sem; /* Semaphore */
    void            *msg; /* Message */
} upnp_msg_buf_t;

/* ================================================================= *
 *  UPNP global structure
 * ================================================================= */

/* UPNP global structure */
typedef struct {
    critd_t     crit;
    upnp_conf_t  upnp_conf;

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t   sem;
        upnp_msg_req_t   msg;
    } request;

} upnp_global_t;

/* Get the location string */
void vtss_upnp_get_location(char *location);

#endif /* _VTSS_UPNP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
