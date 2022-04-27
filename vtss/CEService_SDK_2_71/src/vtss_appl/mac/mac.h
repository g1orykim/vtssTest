/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MAC_H_
#define _VTSS_MAC_H_

#include "vtss_api.h"

/* ================================================================= *
 * Trace definitions
 * ================================================================= */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MAC
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  Mac age local definitions
 * ================================================================= */
#define MAC_CONF_VERSION 0
#define MAC_STATIC_VERSION 2
#define MAC_LEARN_MODE_VERSION 0
/* Structure for aging time in conf blk */
typedef struct {
    ulong       version; /* Block version */
    mac_age_conf_t conf;
} mac_conf_blk_t;

/* Structure for temporary age time change */
typedef struct {
    mac_age_conf_t temp_conf;
    mac_age_conf_t stored_conf;
    ulong age_period;
} age_time_temp_t;

/* ================================================================= *
 *  Static Mac Addresses local definitions
 * ================================================================= */

#define MAC_ADDR_NOT_INCL_STACK ((char)1<<0)
#define MAC_ADDR_DYNAMIC ((char)1<<1)

/* MAC address properties for flash and memory */
typedef struct {
    vtss_vid_mac_t  vid_mac;                           /* VLAN ID and MAC addr */
    char            destination[VTSS_PORT_BF_SIZE];    /* Bit port mask */
    vtss_isid_t     isid;
    char            mode;
    BOOL            copy_to_cpu;
} mac_entry_conf_t;

/* MAC static table - kept in flash*/
typedef  struct {
    ulong              version;                    /* Block version */
    ulong              size;                       /* Size of each entry */
    ulong              count;                            /* Number of entries */
    mac_entry_conf_t   table[MAC_ADDR_NON_VOLATILE_MAX]; /* Entries */
} mac_static_table_t;


/* MAC static entry pointer list - kept in memory */
typedef struct mac_static_t {
    struct mac_static_t   *next;    /* Next in list */
    mac_entry_conf_t      conf;     /* Configuration */
} mac_static_t;

/* ================================================================= *
 *  Mac learn mode definition
 * ================================================================= */
/* Bitmask structure to save space */
typedef struct {
    ulong              version;                                     /* Block version                         */
    char               learn_mode[VTSS_ISID_END][VTSS_PORTS + 1][1]; /* Bit port mask                         */
} mac_conf_learn_mode_t;

/* Bit placements */
#define LEARN_AUTOMATIC 0
#define LEARN_CPU       1
#define LEARN_DISCARD   2

#define MAC_ALL_PORTS 99

/* ================================================================= *
 *  Mac age stack definitions (used with msg module)
 * ================================================================ */

/* MAC messages IDs */
typedef enum {
    MAC_MSG_ID_AGE_SET_REQ,           /* request */
    MAC_MSG_ID_GET_NEXT_REQ,          /* request */
    MAC_MSG_ID_GET_NEXT_REP,          /* reply   */
    MAC_MSG_ID_GET_NEXT_STACK_REQ,    /* request */
    MAC_MSG_ID_GET_STATS_REQ,         /* request */
    MAC_MSG_ID_GET_STATS_REP,         /* reply   */
    MAC_MSG_ID_ADD_REQ,               /* request */
    MAC_MSG_ID_DEL_REQ,               /* request */
    MAC_MSG_ID_LEARN_REQ,             /* request */
    MAC_MSG_ID_FLUSH_REQ,             /* request */
    MAC_MSG_ID_LOCKED_DEL_REQ,        /* request */
    MAC_MSG_ID_UPSID_FLUSH_REQ,       /* request */
    MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ, /* request */
} mac_msg_id_t;

/* MAC message request timer in sec */
#define MAC_REQ_TIMEOUT 20

/* Request message */
typedef struct {
    /* Message ID */
    mac_msg_id_t msg_id;

    union {
        /* MAC_MSG_ID_AGE_SET_REQ */
        struct {
            mac_age_conf_t conf;               /* Configuration */
        } age_set;


        /* MAC_MSG_ID_GET_NEXT_REQ */
        struct {
            vtss_vid_mac_t vid_mac;            /* Mac addr query */
            BOOL           next;               /* Get next or lookup */
        } get_next;

        /* MAC_MSG_ID_GET_STATS_REQ */
        struct {
            vtss_vid_t             vlan;
        } stats;

        /* MAC_MSG_ID_ADD_REQ */
        struct {
            mac_mgmt_addr_entry_t  entry;      /* Address to add */
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
            vtss_vstax_upsid_t     upsid;
            vtss_vstax_upspn_t     upspn;
#endif
        } add;

        /* MAC_MSG_ID_DEL_REQ */
        struct {
            vtss_vid_mac_t         vid_mac;    /* Mac addr to del */
            BOOL                   vol;        /* Volatile? */
        } del;

        /* MAC_MSG_ID_LEARN_REQ */
        struct {
            vtss_port_no_t         port_no;    /* Port to set */
            vtss_learn_mode_t      learn_mode; /* Learn mode  */
        } learn;

        /* MAC_MSG_ID_FLUSH_REQ */
        /* No data */

        /* MAC_MSG_ID_LOCKED_DEL_REQ */
        /* No data */

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        /* MAC_MSG_ID_UPSID_FLUSH_REQ */
        struct {
            vtss_vstax_upsid_t     upsid;
        } upsid_flush;

        /* MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ */
        struct {
            vtss_vstax_upsid_t     upsid;
            vtss_vstax_upspn_t     upspn;
        } upspn_flush;
#endif
    } req;
} mac_msg_req_t;

/* Reply message */
typedef struct {
    /* Message ID */
    mac_msg_id_t msg_id;

    union {
        /* MAC_MSG_ID_GET_NEXT_REP */
        struct {
            vtss_mac_table_entry_t entry;
            vtss_rc                rc;         /* Return code */
        } get_next;

        /* MAC_MSG_ID_GET_STATS_REP */
        struct {
            mac_table_stats_t      stats;
            vtss_rc                rc;         /* Return code */
        } stats;
    } rep;
} mac_msg_rep_t;

/* ================================================================= *
 *  Mac global struct
 * ================================================================= */
typedef struct {
    /* Thread variables */
    cyg_handle_t         thread_handle;
    cyg_thread           thread_block;
    char                 thread_stack[THREAD_DEFAULT_STACK_SIZE];

    /* Critical region mac_ram_table */
    critd_t              ram_crit;

    /* Critical region protection protecting the following block of variables */
    critd_t              crit;

    /* Mac age configuration */
    mac_age_conf_t         conf;

    /* Get Next reply from slave */
    struct {
        vtss_mac_table_entry_t entry;
        vtss_rc                rc;
    } get_next;

    /* Get stats reply from slave */
    struct {
        mac_table_stats_t      stats;
        vtss_rc                rc;
    } get_stats;

    /* Request buffer message pool */
    void *request;

    /* Reply buffer message pool */
    void *reply;

} mac_global_t;


#endif /* _VTSS_MAC_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
