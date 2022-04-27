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

#ifndef _VTSS_PVLAN_H_
#define _VTSS_PVLAN_H_

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "pvlan_api.h"
#include "port_api.h"

// This module supports two kinds of private VLANs, srcmask-based PVLANs
// and port-isolation.
//
// srcmask-based PVLANs:
//   These only work within one single switch, and are implemented via
//   a port's srcmask: When a frame arrives on the port, the set of
//   destination ports are looked up, and afterwards masked with the
//   srcmask. The frame can only be forwarded to ports that have a '1'
//   in the corresponding bit in the ingress port's srcmask.
//
// port-isolation:
//   If a port on a given switch is isolated, it can only transmit
//   to non-isolated ports, and can only receive from non-isolated
//   ports. This works across the stack because the stack header contains
//   a bit that tells whether the frame was originally received on an
//   isolated frontport.
//   Here is how it goes: A frame arrives on a front port. Its
//   classified VID is looked up in the VLAN table. A bit in this entry
//   indicates if the port is a private VLAN. If so, the ANA::PRIV_VLAN_MASK
//   register is consulted. If ANA::PRIV_VLAN_MASK[ingress_port] is 1,
//   the normal VLAN_PORT_MASK (also looked up in the VLAN table) is used.
//   In that case the ingress port is "promiscuous", which means that it can send
//   to all ports in the classified VLAN. If ANA::PRIV_VLAN_MASK[ingress_port]
//   is 0, the ANA::PRIV_VLAN_MASK is ANDed with VLAN_PORT_MASK to form the
//   final set of ports that are member of this VID. This feature works
//   across a stack, because the stack header contains a bit (let's call it
//   "isolated ingress port", indicating if the original ingress port was
//   an isolated port or a promiscuous port (according to the above, a port is
//   isolated when the VLAN table's private VLAN bit is 1 and
//   ANA::PRIV_VLAN_MASK[ingress_port]==0).
//   On the neigboring switch, the VLAN table is consulted again to find that
//   switch's VLAN_PORT_MASK, which in turn is ANDed with ANA::PRIV_VLAN_MASK if
//   the stack header's "isolated ingress port" is set, otherwise it's not
//   ANDed. So ANA::PRIV_VLAN_MASK[stack_port] is never used.
//
// To bring the confusion to a higher level, the datasheet talks about
// port-isolation as private VLANs, while this source code (for legacy
// reasons) uses the term private VLANs for srcmask-based VLANs.
//
// It has been decided to only include the srcmask-based PVLANs in
// the standalone version of the software, but it could "easily" be
// implemented for the stacking solution, but it would probably confuse
// the users that it only works for a single switch in the stack.
// In order to be able to enable srcmask-based PVLANs in this module,
// even for the stacking image, a new #define is made, which currently
// is based on VTSS_SWITCH_STANDALONE, but could be changed to a hard-
// coded 1 if needed. This would require changes to web.c and cli.c
// as well, so that this module's API functions are called.
// In pvlan_api.h, PVLAN_SRC_MASK_ENA is defined if this is included

// In the following, structs prefixed with SM concern the srcmask-based
// PVLANs, and structs prefixed by PI are about the port-isolation.

/* ========================================================= *
 * Trace definitions
 * ========================================================= */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PVLAN

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 * VLAN configuration blocks
 * ================================================================= */

#define PVLAN_ISOLATE_BLK_VERSION    1
#define PVLAN_MEMBERSHIP_BLK_VERSION 1

#define PVLAN_DELETE   1
#define PVLAN_ADD      0

#if defined(PVLAN_SRC_MASK_ENA)
typedef struct {
    vtss_pvlan_no_t privatevid;                      /* Private VLAN ID    */
    uchar ports[VTSS_ISID_END][VTSS_PORT_BF_SIZE];   /* Port mask          */
} SM_entry_conf_t;

/* Private VLAN configuration table, as located in flash */
typedef struct {
    ulong           version;                         /* Block version      */
    ulong           count;                           /* Number of entries  */
    ulong           size;                            /* Size of each entry */
    SM_entry_conf_t table[VTSS_PVLANS];              /* Entries            */
} SM_blk_t;

/* Private VLAN entry */
typedef struct SM_pvlan_t {
    struct SM_pvlan_t *next;                         /* Next in list       */
    SM_entry_conf_t conf;                            /* Configuration      */
} SM_pvlan_t;

/* Private VLAN lists */
typedef struct {
    SM_pvlan_t *used;                                /* Active list        */
    SM_pvlan_t *free;                                /* Free list          */
} SM_pvlan_list_t;
#endif /* PVLAN_SRC_MASK_ENA */

/* VLAN port isolation configuration table, as located in flash */
typedef struct {
    ulong version;                                   /* Block version      */
    BOOL  conf[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE]; /* Entries            */
} PI_blk_t;

/* ================================================================= *
 *  VLAN stack messages
 * ================================================================= */

/* Private VLAN messages IDs */
typedef enum {
    PVLAN_MSG_ID_CONF_SET_REQ,        /* PVLAN memberships configuration set request (no reply) */
    PVLAN_MSG_ID_CONF_ADD_REQ,        /* PVLAN memberships add request                          */
    PVLAN_MSG_ID_CONF_DEL_REQ,        /* PVLAN memberships delete request                       */
    PVLAN_MSG_ID_ISOLATE_CONF_SET_REQ /* Port isolation configuration set request (no reply)    */
} PVLAN_msg_id_t;

#if defined(PVLAN_SRC_MASK_ENA)
/* PVLAN_MSG_ID_CONF_SET_REQ */
typedef struct {
    PVLAN_msg_id_t  msg_id;                /* Message ID    */
    ulong           count;                 /* Entry count   */
    SM_entry_conf_t table[VTSS_PVLANS];    /* Configuration */
} SM_msg_conf_set_req_t;

/* PVLAN_MSG_ID_CONF_ADD_REQ */
typedef struct {
    PVLAN_msg_id_t  msg_id;                /* Message ID    */
    SM_entry_conf_t conf;                  /* Configuration */
} SM_msg_conf_add_req_t;

/* PVLAN_MSG_ID_CONF_DEL_REQ */
typedef struct {
    PVLAN_msg_id_t  msg_id;                /* Message ID    */
    SM_entry_conf_t conf;                  /* Configuration */
} SM_msg_conf_del_req_t;
#endif /* PVLAN_SRC_MASK_ENA */

/* PVLAN_MSG_ID_CONF_ISOLATE_SET_REQ */
typedef struct {
    PVLAN_msg_id_t  msg_id;                /* Message ID */
    BOOL conf[VTSS_PORT_ARRAY_SIZE];       /* Configuration */
} PI_msg_conf_set_req_t;

/* Private VLAN message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    uchar         *msg; /* Message */
} PVLAN_msg_buf_t;

/* Request message. The union is solely used to find */
/* the largest message we use.                       */
typedef struct {
    union {
#if defined(PVLAN_SRC_MASK_ENA)
        SM_msg_conf_set_req_t a;
        SM_msg_conf_add_req_t b;
        SM_msg_conf_del_req_t c;
#endif /* PVLAN_SRC_MASK_ENA */
        PI_msg_conf_set_req_t d;
    } data;
} PVLAN_msg_req_t;

/* ==========================================================================
 *
 * Private VLAN Global Structure
 * =========================================================================*/

typedef struct {
    critd_t           crit;
    BOOL              PI_conf[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE]; // Entries [VTSS_ISID_START; VTSS_ISID_END] hold stack conf, VTSS_ISID_LOCAL this switch's.

#if defined(PVLAN_SRC_MASK_ENA)
    SM_pvlan_list_t   switch_pvlan; // Holds this switch's actual configuration (entry VTSS_ISID_LOCAL used, only).
    SM_pvlan_list_t   stack_pvlan;  // Holds the whole stack's configuration (entries [VTSS_ISID_START; VTSS_ISID_END[ used).

    SM_pvlan_t        pvlan_switch_table[VTSS_PVLANS];
    SM_pvlan_t        pvlan_stack_table[VTSS_PVLANS];
#endif /* PVLAN_SRC_MASK_ENA */

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t sem;
        uchar         msg[sizeof(PVLAN_msg_req_t)];
    } request;

} pvlan_global_t;

#endif /* _VTSS_PVLAN_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
