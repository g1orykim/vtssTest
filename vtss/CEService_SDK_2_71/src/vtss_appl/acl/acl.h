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

#ifndef _VTSS_ACL_H_
#define _VTSS_ACL_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ACL

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>


/* ================================================================= *
 *  ACE ID definitions
 *  we¡¦ll use a 16-bit users ID and a 16-bit ACE ID to identify an ACE.
 *  The VTSS API used a 32-bits ACE ID. These two 16-bit values could
 *  be combined when accessing the switch API. This would give each ACL
 *  user a separate ID space used for its own entries.
 * ================================================================= */

#define ACL_USER_ID_GET(ace_id)                 ((ace_id) >> 16)
#define ACL_ACE_ID_GET(ace_id)                  ((ace_id) & 0xFFFF)
#define ACL_COMBINED_ID_SET(user_id, ace_id)    (((user_id) << 16) + ((ace_id) & 0xFFFF))

/* In stackable architecture, when a new switch is added,
   ACL module will apply related ACEs (using message of
   ACL_MSG_ID_ACE_CONF_SET_REQ) to the new switch.
   The total ACEs count is dynamic and it maybe great ACE_MAX.
   We defined a variable here for the purpose.
   Only ACL_USER_IP_SOURCE_GUARD uses the ACL_USER_REG_MODE_GLOBAL now.
   If there are two module use the global registration,
   We need set this value as multiple of ACE_MAX. */
#define ACL_MAX_EXTEND_ACE_CNT                  (ACE_MAX * (ACL_USER_GLOBAL_MODE_CNT - 1))

/* ================================================================= *
 *  ACL configuration blocks
 * ================================================================= */

/* Block versions */
#define ACL_PORT_BLK_VERSION    1
#define ACL_POLICER_BLK_VERSION 1
#define ACL_ACE_BLK_VERSION     2

/* Port configuration block */
typedef struct {
    ulong           version;
    acl_port_conf_t conf[VTSS_ISID_CNT *VTSS_PORTS];
} acl_port_blk_t;

/* Policer configuration block */
typedef struct {
    ulong              version;
    acl_policer_conf_t conf[VTSS_ACL_POLICERS];
} acl_policer_blk_t;

/* ACE configuration block */
typedef struct {
    ulong            version;
    ulong            count;             /* Number of entries */
    ulong            size;              /* Size of each entry */
    acl_entry_conf_t table[ACE_ID_END]; /* Entries */
} acl_ace_blk_t;

/* ================================================================= *
 *  ACL table and list
 * ================================================================= */

/* ACL entry */
typedef struct acl_ace_t {
    struct acl_ace_t *next; /* Next in list */
    acl_entry_conf_t conf;  /* Configuration */
} acl_ace_t;

/* ACL lists */
typedef struct {
    acl_ace_t *used; /* Active list */
    acl_ace_t *free; /* Free list */
} acl_list_t;

/* ACL counters */
typedef struct {
    vtss_ace_id_t      id;      /* ACE ID */
    vtss_ace_counter_t counter; /* ACE counter */
} ace_counter_t;

/* ================================================================= *
 *  ACL stack messages
 * ================================================================= */

/* ACL messages IDs */
typedef enum {
    ACL_MSG_ID_PORT_CONF_SET_REQ,     /* Port configuration set request (no reply) */
    ACL_MSG_ID_PORT_COUNTERS_GET_REQ, /* Port counters get request */
    ACL_MSG_ID_PORT_COUNTERS_GET_REP, /* Port counters get reply */
    ACL_MSG_ID_POLICER_CONF_SET_REQ,  /* Policer configuration set set request (no reply) */
    ACL_MSG_ID_ACE_CONF_SET_REQ,      /* ACE configuration set request (no reply) */
    ACL_MSG_ID_ACE_CONF_ADD_REQ,      /* ACE configuration add request (no reply) */
    ACL_MSG_ID_ACE_CONF_DEL_REQ,      /* ACE configuration delete request (no reply) */
    ACL_MSG_ID_ACE_COUNTERS_GET_REQ,  /* ACE counters get request */
    ACL_MSG_ID_ACE_COUNTERS_GET_REP,  /* ACE counters get reply */
    ACL_MSG_ID_COUNTERS_CLEAR_REQ,    /* ACL counters clear request */
    ACL_MSG_ID_PORT_SHUTDOWN_SET_REQ, /* Port shutdown */
    ACL_MSG_ID_ACE_CONF_GET_REQ,      /* ACE configuration get request */
    ACL_MSG_ID_ACE_CONF_GET_REP       /* ACE configuration get reply */
} acl_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    acl_msg_id_t msg_id;

    /* Request data, depending on message ID */
    union {
        /* ACL_MSG_ID_PORT_CONF_SET_REQ */
        struct {
            acl_port_conf_t conf[VTSS_PORTS];
        } port_conf_set;

        /* ACL_MSG_ID_PORT_COUNTERS_GET_REQ: No data */

        /* ACL_MSG_ID_POLICER_CONF_SET_REQ */
        struct {
            acl_policer_conf_t conf[VTSS_ACL_POLICERS];
        } policer_conf_set;

        /* ACL_MSG_ID_ACE_CONF_SET_REQ */
        struct {
            ulong            count;                                   /* Number of entries */
            acl_entry_conf_t table[ACE_MAX + ACL_MAX_EXTEND_ACE_CNT]; /* Entries */
        } ace_conf_set;

        /* ACL_MSG_ID_ACE_CONF_ADD_REQ */
        struct {
            vtss_ace_id_t    id;     /* Add before this or last */
            acl_entry_conf_t conf;
        } ace_conf_add;

        /* ACL_MSG_ID_ACE_CONF_DEL_REQ */
        struct {
            vtss_ace_id_t id;
        } ace_conf_del;

        /* ACL_MSG_ID_ACE_COUNTERS_GET_REQ: No data */

        /* ACL_MSG_ID_COUNTERS_CLEAR_REQ: No data */

        /* ACL_MSG_ID_PORT_SHUTDOWN_SET_REQ */
        struct {
            vtss_port_no_t port_no;
        } port_shutdown;

        /* ACL_MSG_ID_ACE_CONF_GET_REQ */
        struct {
            vtss_ace_id_t    id;      /* Add before this or last */
            acl_entry_conf_t conf;
            BOOL             counter;
            BOOL             next;
        } ace_conf_get;
    } req;
} acl_msg_req_t;

/* Reply message */
typedef struct {
    /* Message ID */
    acl_msg_id_t msg_id;

    /* Request data, depending on message ID */
    union {
        /* ACL_MSG_ID_PORT_COUNTERS_GET_REP */
        struct {
            vtss_acl_port_counter_t counters[VTSS_PORTS];
        } port_counters_get;

        /* ACL_MSG_ID_ACE_COUNTERS_GET_REP */
        struct {
            ulong         count;             /* Number of entries */
            ace_counter_t counters[ACE_MAX]; /* Entries */
        } ace_counters_get;
        /* ACL_MSG_ID_ACE_CONF_GET_REP */
        struct {
            acl_entry_conf_t   conf;        /* ACE configuration */
            vtss_ace_counter_t counter;     /* ACE counter */
        } ace_conf_get;
    } rep;
} acl_msg_rep_t;

/* ACL request message timeout */
#define ACL_REQ_TIMEOUT 5

/* ACL counters timer */
#define ACL_COUNTERS_TIMER 1000

/* ACL log buffer */
#define ACL_LOG_BUF_SIZE (6*1024)

/* ================================================================= *
 *  ACL global structure
 * ================================================================= */

/* ACL global structure */
typedef struct {
    critd_t                 crit;
    acl_port_conf_t         port_conf[VTSS_ISID_END][VTSS_PORTS];
    acl_policer_conf_t      policer_conf[VTSS_ACL_POLICERS];
    acl_list_t              switch_acl;
    acl_list_t              stack_acl;
    acl_ace_t               switch_ace_table[ACE_MAX];
    acl_ace_t               stack_ace_table[ACE_ID_END];
    ace_counter_t           ace_counters[VTSS_ISID_CNT][ACE_MAX];     /* ACE counters */
    vtss_mtimer_t           ace_counters_timer[VTSS_ISID_CNT];        /* ACE counters timer */
    cyg_flag_t              ace_counters_flags;                       /* ACE counters flags */
    vtss_acl_port_counter_t port_counters[VTSS_ISID_CNT][VTSS_PORTS]; /* Port counters */
    vtss_mtimer_t           port_counters_timer[VTSS_ISID_CNT];       /* Port counters timer */
    cyg_flag_t              port_counters_flags;                      /* Port counters flags */
    char                    log_buf[ACL_LOG_BUF_SIZE];                /* Log buffer */
    acl_entry_conf_t        ace_conf_get_rep_info[VTSS_ISID_END];     /* ACE configuration get reply information */
    vtss_ace_counter_t      ace_conf_get_rep_counter[VTSS_ISID_END];  /* ACE configuration get reply counter */
    cyg_flag_t              ace_conf_get_flags;                       /* ACE configuration get flags */

    /* Request buffer pool */
    void *request;

    /* Reply buffer pool */
    void *reply;

    void          *filter_id; /* Packet filter ID */
    vtss_mtimer_t port_shutdown_timer[VTSS_PORTS];
} acl_global_t;

#endif /* _VTSS_ACL_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
