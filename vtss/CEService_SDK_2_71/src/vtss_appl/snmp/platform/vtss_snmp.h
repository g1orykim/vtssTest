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

#ifndef _VTSS_SNMP_H_
#define _VTSS_SNMP_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_snmp_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SNMP

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  SNMP configuration blocks
 * ================================================================= */

/* Block versions */
#define SNMP_CONF_BLK_VERSION                   2
#define SNMP_PORT_CONF_BLK_VERSION              1
#define SNMP_RMON_STAT_CONF_BLK_VERSION         1
#define SNMP_RMON_HISTORY_CONF_BLK_VERSION      1
#define SNMP_RMON_ALARM_CONF_BLK_VERSION        1
#define SNMP_RMON_EVENT_CONF_BLK_VERSION        1
#ifdef SNMP_SUPPORT_V3
#define SNMPV3_COMMUNITIES_CONF_BLK_VERSION     1
#define SNMPV3_USERS_CONF_BLK_VERSION           1
#define SNMPV3_GROUPS_CONF_BLK_VERSION          1
#define SNMPV3_VIEWS_CONF_BLK_VERSION           1
#define SNMPV3_ACCESSES_CONF_BLK_VERSION        1
#endif /* SNMP_SUPPORT_V3 */

#define TRAP_CONF_BLK_VERSION                   1

#define SNMP_SMON_STAT_MAX_ROW_SIZE         128
/* SNMP configuration block */
typedef struct {
    unsigned long    version;       /* Block version */
    snmp_conf_t      snmp_conf;     /* SNMP configuration */
} snmp_conf_blk_t;

/* SNMP Port configuration block */
typedef struct {
    unsigned long    version;           /* Block version */
    snmp_port_conf_t snmp_port_conf[VTSS_ISID_CNT *VTSS_PORTS]; /* SNMP port configuration */
} snmp_port_conf_blk_t;

typedef struct {
    unsigned long          version;                                           /* Block version */
    unsigned long          snmp_smon_stat_entry_num;                          /* SNMP RMON statistics row entries number */
    snmp_rmon_stat_entry_t snmp_smon_stat_entry[SNMP_SMON_STAT_MAX_ROW_SIZE]; /* SNMP RMON statistics row entries */
} smon_stat_conf_blk_t;

typedef struct {
    unsigned long          version;                                           /* Block version */
    unsigned long          snmp_port_copy_entry_num;                          /* SNMP RMON statistics row entries number */
    snmp_port_copy_entry_t snmp_port_copy_entry[SNMP_SMON_STAT_MAX_ROW_SIZE]; /* SNMP RMON statistics row entries */
} port_copy_conf_blk_t;

#ifdef SNMP_SUPPORT_V3
/* SNMPv3 communities configuration block */
typedef struct {
    unsigned long             version;                                     /* Block version */
    unsigned long             communities_conf_num;                        /* communities configuration number */
    snmpv3_communities_conf_t communities_conf[SNMPV3_MAX_COMMUNITIES];    /* communities configuration */
} snmpv3_communities_conf_blk_t;

/* SNMPv3 users configuration block */
typedef struct {
    unsigned long       version;                        /* Block version */
    unsigned long       users_conf_num;                 /* users configuration number */
    snmpv3_users_conf_t users_conf[SNMPV3_MAX_USERS];   /* users configuration */
} snmpv3_users_conf_blk_t;

/* SNMPv3 groups configuration block */
typedef struct {
    unsigned long        version;                           /* Block version */
    unsigned long        groups_conf_num;                   /* groups configuration number */
    snmpv3_groups_conf_t groups_conf[SNMPV3_MAX_GROUPS];    /* groups configuration */
} snmpv3_groups_conf_blk_t;

/* SNMPv3 views configuration block */
typedef struct {
    unsigned long          version;                             /* Block version */
    unsigned long          views_conf_num;                      /* views configuration number */
    snmpv3_views_conf_t    views_conf[SNMPV3_MAX_VIEWS];        /* views configuration */
} snmpv3_views_conf_blk_t;

/* SNMPv3 accesses configuration block */
typedef struct {
    unsigned long          version;                             /* Block version */
    unsigned long          accesses_conf_num;                   /* accesses configuration number */
    snmpv3_accesses_conf_t accesses_conf[SNMPV3_MAX_ACCESSES];  /* accesses configuration */
} snmpv3_accesses_conf_blk_t;
#endif /* SNMP_SUPPORT_V3 */

typedef struct {
    unsigned long          version;                             /* Block version */
    vtss_trap_sys_conf_t   trap_sys_conf;                         /* accesses configuration */
} trap_conf_blk_t;


/* ================================================================= *
 *  SNMP stack messages
 * ================================================================= */

/* SNMP messages IDs */
typedef enum {
    SNMP_MSG_ID_SNMP_CONF_SET_REQ,          /* SNMP configuration set request (no reply) */
} snmp_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    snmp_msg_id_t    msg_id;

    /* Request data, depending on message ID */
    union {
        /* SNMP_MSG_ID_SNMP_CONF_SET_REQ */
        struct {
            snmp_conf_t      conf;
        } conf_set;
    } req;
} snmp_msg_req_t;

/* SNMP message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    void          *msg; /* Message */
} snmp_msg_buf_t;

/* ================================================================= *
 *  SNMP global structure
 * ================================================================= */

/* SNMP global structure */
typedef struct {
    critd_t                   crit;
    snmp_conf_t               snmp_conf;
    snmp_port_conf_t          snmp_port_conf[VTSS_ISID_END][VTSS_PORTS];
    unsigned long             snmp_smon_stat_entry_num;
    snmp_rmon_stat_entry_t    snmp_smon_stat_entry[SNMP_SMON_STAT_MAX_ROW_SIZE];
    unsigned long             snmp_port_copy_entry_num;
    snmp_port_copy_entry_t    snmp_port_copy_entry[SNMP_SMON_STAT_MAX_ROW_SIZE];
#ifdef SNMP_SUPPORT_V3
    unsigned long             communities_conf_num;
    snmpv3_communities_conf_t communities_conf[SNMPV3_MAX_COMMUNITIES];
    unsigned long             users_conf_num;
    snmpv3_users_conf_t       users_conf[SNMPV3_MAX_USERS];
    unsigned long             groups_conf_num;
    snmpv3_groups_conf_t      groups_conf[SNMPV3_MAX_GROUPS];
    unsigned long             views_conf_num;
    snmpv3_views_conf_t       views_conf[SNMPV3_MAX_ACCESSES];
    unsigned long             accesses_conf_num;
    snmpv3_accesses_conf_t    accesses_conf[SNMPV3_MAX_ACCESSES];
#endif /* SNMP_SUPPORT_V3 */

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t sem;
        snmp_msg_req_t msg;
    } request;

} snmp_global_t;

#endif /* _VTSS_SNMP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

