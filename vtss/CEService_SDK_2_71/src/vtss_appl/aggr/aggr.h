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

#ifndef _VTSS_AGGR_H_
#define _VTSS_AGGR_H_

/* ================================================================= *
 * Trace definitions
 * ================================================================= */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_AGGR
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  Aggr local definitions
 * ================================================================= */
#define AGGR_CONF_VERSION     0
#define AGGR_MGMT_GROUPS      (AGGR_LLAG_CNT + AGGR_GLAG_CNT + 1)
#define AGGR_ALL_GROUPS       999

/* Flash/Mem structs */
typedef struct {
    BOOL member[VTSS_PORT_BF_SIZE];
    vtss_port_speed_t  speed;
} aggr_conf_group_t;

/* Structure for the local switch */
typedef struct {
    aggr_conf_group_t groups[AGGR_MGMT_GROUPS];
    vtss_aggr_mode_t mode;
} aggr_conf_aggr_t;

/* Structure for flash storage for the whole stack */
typedef struct {
    ulong              version; /* Block version     */
    vtss_aggr_mode_t   mode;    /* Aggregation mmode */
    aggr_conf_group_t  switch_id[VTSS_ISID_END][AGGR_MGMT_GROUPS]; /* All groups for all switches */
} aggr_conf_stack_aggr_t;

/* To support upgrade from 2_80: */
typedef struct {
    BOOL member[VTSS_PORT_BF_SIZE];
} aggr_conf_280e_t;

typedef struct {
    ulong              version; /* Block version     */
    vtss_aggr_mode_t   mode;    /* Aggregation mmode */
    aggr_conf_280e_t  switch_id[VTSS_ISID_END][AGGR_MGMT_GROUPS]; /* All groups for all switches */
} aggr_conf_2_80e_t;

typedef struct {
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    vtss_vstax_glag_entry_t  entries[VTSS_GLAG_PORT_ARRAY_SIZE];
#else
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_VSTAX_V2 */
} aggr_glag_members_t;

/* ================================================================= *
 *  Aggr stack definitions (used with msg module)
 * ================================================================ */

/* AGGR messages IDs */
typedef enum {
    AGGR_MSG_ID_ADD_REQ,         /* Request  */
    AGGR_MSG_ID_DEL_REQ,         /* Request  */
    AGGR_MSG_ID_MODE_SET_REQ,    /* Request  */
    AGGR_MSG_ID_GLAG_UPDATE_REQ, /* Request  */
} aggr_msg_id_t;

/* AGGR_MSG_ID_ADD_REQ */
typedef struct {
    aggr_msg_id_t            msg_id;  /* Message ID  */
    aggr_glag_members_t      members;
    aggr_mgmt_group_no_t     aggr_no;
    BOOL                     conf;
} aggr_msg_add_req_t;

/* AGGR_MSG_ID_AGGR_UPDATE_REQ */
typedef struct {
    aggr_msg_id_t            msg_id;  /* Message ID  */
    aggr_mgmt_group_no_t     aggr_no;
    vtss_port_speed_t        grp_speed;
    aggr_mgmt_group_t        members;
} aggr_msg_glag_update_req_t;

/* AGGR_MSG_ID_DEL_REQ */
typedef struct {
    aggr_msg_id_t          msg_id;    /* Message ID  */
    aggr_mgmt_group_no_t   aggr_no;
} aggr_msg_del_req_t;

/* AGGR_MSG_ID_MODE_SET_REQ */
typedef struct {
    aggr_msg_id_t          msg_id;    /* Message ID  */
    vtss_aggr_mode_t       mode;
} aggr_msg_mode_set_req_t;

/* Aggr message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    uchar         *msg; /* Message */
} aggr_msg_buf_t;

/* Request message for size calc */
typedef struct {
    union {
        aggr_msg_add_req_t         add;
        aggr_msg_glag_update_req_t glag;
        aggr_msg_del_req_t         del;
        aggr_msg_mode_set_req_t    set;
    } req;
} aggr_msg_req_t;

/* ================================================================= *
 *  Aggr global struct
 * ================================================================= */
typedef struct {
    /* Request buffer and semaphore. Using the largest of the lot. */
    struct {
        vtss_os_sem_t sem;
        uchar         msg[sizeof(aggr_msg_req_t)];
    } request;

    u32                             aggr_callbacks;
    aggr_change_callback_t          callback_list[5];
    critd_t                         aggr_crit, aggr_cb_crit;

    vtss_glag_no_t                  port_glag_member[VTSS_PORT_ARRAY_SIZE];
    aggr_mgmt_group_no_t            port_aggr_member[VTSS_PORT_ARRAY_SIZE];
    aggr_mgmt_group_no_t            port_active_aggr_member[VTSS_PORT_ARRAY_SIZE];

    aggr_conf_aggr_t                aggr_config;
    vtss_port_speed_t               aggr_group_speed[VTSS_ISID_END][AGGR_MGMT_GROUP_NO_END];
    aggr_conf_stack_aggr_t          aggr_config_stack;


    aggr_mgmt_group_no_t            active_aggr_ports[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE];
    aggr_mgmt_group_no_t            config_aggr_ports[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    init_port_map_t                 port_map_global[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE];
    vtss_glag_no_t                  port_active_glag_member[VTSS_PORT_ARRAY_SIZE];

    struct {
        vtss_vstax_upsid_t upsid[2];
    } upsid_table[VTSS_ISID_END];

    struct {
        vtss_vstax_glag_entry_t  ups;
        vtss_isid_t              isid;
    } glag_ram_entries[VTSS_GLAG_NO_END][VTSS_GLAG_PORT_ARRAY_SIZE];

#endif

#if defined(VTSS_FEATURE_VSTAX_V1) && VTSS_SWITCH_STACKABLE
    uint                            glag_active_count[VTSS_GLAG_NO_END];
    BOOL                            aggr_glag_config_result[VTSS_GLAG_NO_END];
    vtss_glag_no_t                  port_active_glag_member[VTSS_PORT_ARRAY_SIZE];
#endif

#ifdef VTSS_SW_OPTION_LACP
    struct {
        BOOL members[VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START];
        aggr_mgmt_group_no_t aggr_no;
    } aggr_lacp[VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START];
#endif /* VTSS_SW_OPTION_LACP */

} aggr_global_t;

#define LACP_ONLY_ONE_MEMBER 999

#endif /* _VTSS_AGGR_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
