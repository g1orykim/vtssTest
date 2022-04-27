/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_QOS_H_
#define _VTSS_QOS_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_QOS

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

#if defined(VTSS_FEATURE_QCL_V2)
/* =================================================================
 *  QCE ID definitions
 *  we will use a 16-bit users ID and a 16-bit QCE ID to identify an QCE.
 *  The VTSS API used a 32-bits QCE ID. These two 16-bit values could
 *  be combined when accessing the switch API. This would give each QCL
 *  user a separate ID space used for its own entries.
 * ================================================================= */
#define QCL_USER_ID_GET(qce_id)               ((qce_id) >> 16)
#define QCL_QCE_ID_GET(qce_id)                ((qce_id) & 0xFFFF)
#define QCL_COMBINED_ID_SET(user_id, qce_id)  (((user_id) << 16) + ((qce_id) & 0xFFFF))
#endif

/* ================================================================= *
 *  QOS configuration blocks
 * ================================================================= */

/* Block versions */
#define QOS_CONF_BLK_VERSION            0x1b2b3c4d
#define QOS_PORT_CONF_BLK_VERSION       0x1b2b3c4f
#define QOS_QCL_QCE_BLK_VERSION         0x1b2b3c4e

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#define QOS_LPORT_CONF_BLK_VERSION      0x1b2b3c5d //LPORT-TODO: ? 
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

/* QOS configuration block */
typedef struct {
    ulong                version;
    qos_conf_t           conf;
} qos_conf_blk_t;

/* QOS port configuration block */
typedef struct {
    ulong                version;
    qos_port_conf_t      conf[VTSS_ISID_CNT][VTSS_PORTS]; // Indexed with (isid - VTSS_ISID_START)
} qos_port_conf_blk_t;

/* QOS QCL QCE configuration block */
typedef struct {
    ulong                version;
    ulong                count[QCL_MAX + RESERVED_QCL_CNT];             /* Number of entries */
    ulong                size[QCL_MAX + RESERVED_QCL_CNT];              /* Size of each entry */
    qos_qce_entry_conf_t table[(QCL_MAX + RESERVED_QCL_CNT) * QCE_ID_END]; /* Entries */
} qcl_qce_blk_t;

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
/* QOS lport configuration block */
typedef struct {
    ulong            version;
    BOOL             mode_change;
    qos_lport_conf_t conf[VTSS_LPORTS];
} qos_lport_conf_blk_t;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */



/* QCL entry */
typedef struct qos_qce_t {
    struct qos_qce_t    *next; /* Next in list */
    qos_qce_entry_conf_t conf; /* Configuration */
} qcl_qce_t;

#if defined(VTSS_FEATURE_QCL_V1)
typedef struct {
    qcl_qce_t *qce_used_list;     /* a link list for used QCEs */
    qcl_qce_t *qce_free_list;     /* a link list for unused QCEs */
    qcl_qce_t  qce_list[QCE_MAX]; /* the body of whole QCE table */
} qcl_qce_list_table_t;
#endif
#if defined(VTSS_FEATURE_QCL_V2)
typedef struct {
    qcl_qce_t *qce_used_list;     /* a link list for used QCEs */
    qcl_qce_t *qce_free_list;     /* a link list for unused QCEs */
    qcl_qce_t *qce_list;          /* the body of whole QCE table: since the size
                                     of stack and switch qce table are different
                                     we will create the space at sys init time */
} qcl_qce_list_table_t;
#endif


/* ================================================================= *
 *  QOS stack messages
 * ================================================================= */

/* QOS messages IDs */
typedef enum {
    QOS_MSG_ID_CONF_SET_REQ,           /* QOS configuration set request (no reply) */
    QOS_MSG_ID_PORT_CONF_SET_REQ,      /* QOS port configuration set request (no reply) */
    QOS_MSG_ID_PORT_CONF_ALL_SET_REQ,  /* QOS port configuration set all request (no reply) */
    QOS_MSG_ID_QCE_ADD_REQ,            /* QCE configuration add entry request (no reply) */
    QOS_MSG_ID_QCE_DEL_REQ,            /* QCE configuration delete entry request (no reply) */
    QOS_MSG_ID_QCL_CLEAR_REQ,          /* QCL configuration clear request (no reply) */
#if defined(VTSS_FEATURE_QCL_V2)
    QOS_MSG_ID_QCE_GET_REQ,            /* QCE configuration get request (no reply) */
    QOS_MSG_ID_QCE_GET_REP,            /* QCL configuration get response (no reply) */
    QOS_MSG_ID_QCE_CONFLICT_RSLVD_REQ, /* QCE conflict resolve request (no reply) */
#endif
} qos_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    qos_msg_id_t msg_id;

    /* Request data, depending on message ID */
    union {
        /* QOS_MSG_ID_CONF_SET_REQ */
        struct {
            qos_conf_t conf;
        } conf_set;

        /* QOS_MSG_ID_PORT_CONF_SET_REQ */
        struct {
            vtss_port_no_t  port_no;
            qos_port_conf_t conf;
        } port_conf_set;

        /* QOS_MSG_ID_PORT_CONF_ALL_SET_REQ */
        struct {
            qos_port_conf_t conf[VTSS_PORTS];
        } port_conf_all_set;

        /* QOS_MSG_ID_QCE_ADD_REQ */
        struct {
            vtss_qcl_id_t        qcl_id;
            vtss_qce_id_t        qce_id;
            qos_qce_entry_conf_t conf;
        } qce_add;

        /* QOS_MSG_ID_QCE_DEL_REQ */
        struct {
            vtss_qcl_id_t qcl_id;
            vtss_qce_id_t qce_id;
        } qce_del;

        /* QOS_MSG_ID_QCL_CLEAR_REQ (no data) */

#if defined(VTSS_FEATURE_QCL_V2)
        /* QOS_MSG_ID_QCE_GET_REQ/REP */
        struct {
            vtss_qcl_id_t        qcl_id;
            vtss_qce_id_t        qce_id;
            qos_qce_entry_conf_t conf;
            BOOL                 next;
        } qce_get;

        /* QOS_MSG_ID_QCE_CONFLICT_RSLVD_REQ */
        struct {
            vtss_qcl_id_t        qcl_id;
        } qce_conflict;
#endif
    } req;
} qos_msg_req_t;

/* QCL request message timeout */
#define QCL_REQ_TIMEOUT    5

#define QOS_PORT_CONF_CHANGE_REG_MAX 2

/* QoS port configuration change registration table */
typedef struct {
    critd_t                    crit;
    int                        count;
    qos_port_conf_change_reg_t reg[QOS_PORT_CONF_CHANGE_REG_MAX];
} qos_port_conf_change_reg_table_t;

/* ================================================================= *
 *  QOS global structure
 * ================================================================= */

/* QOS global structure */
typedef struct {
    critd_t              qos_crit;
    qos_conf_t           qos_conf;
    qos_conf_t           qos_local_conf; // Local copy saved on the slave. Used by CLI on a slave for getting the configuration
    qos_port_conf_t      qos_port_conf[VTSS_ISID_END][VTSS_PORTS]; // Indexed with isid in order to make room for VTSS_ISID_LOCAL as first entry
    vtss_prio_t          volatile_default_prio[VTSS_ISID_CNT][VTSS_PORTS];
    qcl_qce_list_table_t qcl_qce_stack_list[QCL_MAX + RESERVED_QCL_CNT];
    qcl_qce_list_table_t qcl_qce_switch_list[QCL_MAX + RESERVED_QCL_CNT];

#if defined(VTSS_FEATURE_QCL_V2)
    qos_qce_entry_conf_t qcl_qce_conf_get_rep_info[VTSS_ISID_END]; /* QCE configuration get reply information */
    cyg_flag_t           qcl_qce_conf_get_flags; /* QCE configuration get flags */
#endif

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    qos_lport_conf_t     qos_lport[VTSS_LPORTS]; /* Lport QoS configuration */
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

    /* Request buffer pool */
    void *request;

    qos_port_conf_change_reg_table_t rt;
} qos_global_t;

#endif /* _VTSS_QOS_H_ */

/*********************************************************************/
/*                                                                   */
/*  End of file.                                                     */
/*                                                                   */
/*********************************************************************/
