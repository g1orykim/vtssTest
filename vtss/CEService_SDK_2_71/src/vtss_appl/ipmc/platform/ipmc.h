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

#ifndef _IPMC_H_
#define _IPMC_H_

#include "ipmc_conf.h"
#include "vtss_ipmc.h"


/* ================================================================= *
 *  IPMC stack messages
 * ================================================================= */
/* IPMC request message timeout */
#define IPMC_REQ_TIMEOUT                        12345   /* in msec */

/* IPMC messages IDs */
typedef enum {
    IPMC_MSG_ID_MODE_SET_REQ = 0,               /* IPMC MODE set request (no reply) */
    IPMC_MSG_ID_SYS_MGMT_SET_REQ,               /* IPMC System IPv4 Address set request (no reply) */
    IPMC_MSG_ID_LEAVE_PROXY_SET_REQ,            /* IPMC LEAVE PROXY set request (no reply) */
    IPMC_MSG_ID_PROXY_SET_REQ,                  /* IPMC PROXY set request (no reply) */
    IPMC_MSG_ID_SSM_RANGE_SET_REQ,              /* IPMC SSM RANGE set request (no reply) */
    IPMC_MSG_ID_UNREG_FLOOD_SET_REQ,            /* IPMC UNREG FLOOD set request (no reply) */

    IPMC_MSG_ID_ROUTER_PORT_SET_REQ,            /* IPMC ROUTER PORT set request (no reply) */
    IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ,        /* IPMC FAST LEAVE PORT set request (no reply) */
    IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ,      /* IPMC Throllting Max number set request (no reply) */
    IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ,   /* IPMC Port Group Filterinig set request (no reply) */
    IPMC_MSG_ID_VLAN_SET_REQ,                   /* IPMC VLAN SET request (no reply) */
    IPMC_MSG_ID_VLAN_ENTRY_SET_REQ,             /* IPMC VLAN Parameter SET request (no reply) */

    IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ,         /* IPMC clear counter request (no reply) */
    IPMC_MSG_ID_STP_PORT_CHANGE_REQ,            /* IPMC STP-PSC call-baack handler (no reply) */

    IPMC_MSG_ID_VLAN_ENTRY_GET_REQ,
    IPMC_MSG_ID_VLAN_ENTRY_GET_REP,
    IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ,
    IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP,
    IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ,
    IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP,
    IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ,
    IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP,

    IPMC_MSG_MAX_ID
} ipmc_msg_id_t;


/* IPMC_MSG_ID_MODE_SET_REQ:12 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    BOOL                    ipmc_mode_enabled;
    ipmc_ip_version_t       version;
} ipmc_msg_mode_set_req_t;

/* IPMC_MSG_ID_SYS_MGMT_SET_REQ:12 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    u8                      mgmt_mac[6];
    u32                     intf_cnt;
    ipmc_mgmt_ipif_t        ip_addr[VTSS_IPMC_MGMT_IPIF_MAX_CNT];
} ipmc_msg_sys_mgmt_set_req_t;

/* IPMC_MSG_ID_LEAVE_PROXY_SET_REQ:12 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    BOOL                    ipmc_leave_proxy_enabled;
    ipmc_ip_version_t       version;
} ipmc_msg_leave_proxy_set_req_t;

/* IPMC_MSG_ID_PROXY_SET_REQ:12 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    BOOL                    ipmc_proxy_enabled;
    ipmc_ip_version_t       version;
} ipmc_msg_proxy_set_req_t;

/* IPMC_MSG_ID_SSM_RANGE_SET_REQ:28 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    ipmc_prefix_t           prefix;
    ipmc_ip_version_t       version;
} ipmc_msg_ssm_range_set_req_t;

/* IPMC_MSG_ID_UNREG_FLOOD_SET_REQ:12 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    BOOL                    ipmc_unreg_flood_enabled;
    ipmc_ip_version_t       version;
} ipmc_msg_unreg_flood_set_req_t;

/* IPMC_MSG_ID_ROUTER_PORT_SET_REQ:68 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    vtss_isid_t             isid;
    BOOL                    ipmc_router_ports[VTSS_PORT_ARRAY_SIZE];
    ipmc_ip_version_t       version;
} ipmc_msg_router_port_set_req_t;

/* IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ:68 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    vtss_isid_t             isid;
    BOOL                    ipmc_fast_leave_ports[VTSS_PORT_ARRAY_SIZE];
    ipmc_ip_version_t       version;
} ipmc_msg_fast_leave_port_set_req_t;

/* IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ:224 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    vtss_isid_t             isid;
    int                     ipmc_throttling_max_no[VTSS_PORT_ARRAY_SIZE];
    ipmc_ip_version_t       version;
} ipmc_msg_throllting_max_no_set_req_t;

/* IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ:224 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    vtss_isid_t             isid;
    u32                     ipmc_port_group_filtering[VTSS_PORT_ARRAY_SIZE];
    ipmc_ip_version_t       version;
} ipmc_msg_port_group_filtering_set_req_t;

/* IPMC_MSG_ID_VLAN_SET_REQ & IPMC_MSG_ID_VLAN_ENTRY_SET_REQ:52 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    ipmc_operation_action_t action;
    ipmc_prot_intf_basic_t  vlan_entry;
    ipmc_port_bfs_t         vlan_member;
    ipmc_ip_version_t       version;
} ipmc_msg_vlan_set_req_t;

/* IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ:10 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    ipmc_ip_version_t       version;
    vtss_vid_t              vid;
} ipmc_msg_stat_counter_clear_req_t;

/* IPMC_MSG_ID_STP_PORT_CHANGE_REQ:16 */
typedef struct {
    ipmc_msg_id_t           msg_id;
    vtss_port_no_t          port;
    vtss_common_stpstate_t  new_state;
    ipmc_ip_version_t       version;
} ipmc_msg_stp_port_change_set_req_t;

/*
    IPMC_MSG_ID_VLAN_ENTRY_GET_REQ &
    IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ &
    IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ
    :44
*/
typedef struct {
    ipmc_msg_id_t           msg_id;

    vtss_vid_t              vid;
    vtss_ipv6_t             group_addr;

    vtss_ipv6_t             srclist_addr;
    BOOL                    srclist_type;

    ipmc_ip_version_t       version;
} ipmc_msg_vlan_entry_get_req_t;

/* IPMC_MSG_ID_VLAN_ENTRY_GET_REP:136 */
typedef struct {
    ipmc_msg_id_t                   msg_id;
    ipmc_prot_intf_entry_param_t    interface_entry;
    ipmc_ip_version_t               version;
} ipmc_msg_vlan_entry_get_rep_t;

/* IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:524 */
typedef struct {
    ipmc_msg_id_t                   msg_id;
    ipmc_prot_intf_group_entry_t    intf_group_entry;
    ipmc_ip_version_t               version;
} ipmc_msg_vlan_group_entry_get_rep_t;

/* IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP:540 */
typedef struct {
    ipmc_msg_id_t                   msg_id;
    ipmc_prot_group_srclist_t       group_srclist_entry;
    ipmc_ip_version_t               version;
} ipmc_msg_group_srclist_get_rep_t;

/* IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ:8 */
typedef struct {
    ipmc_msg_id_t                   msg_id;
    ipmc_ip_version_t               version;
} ipmc_msg_dyn_rtpt_get_req_t;

/* IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP:64 */
typedef struct {
    ipmc_msg_id_t                   msg_id;
    ipmc_dynamic_router_port_t      dynamic_router_ports;
    ipmc_ip_version_t               version;
} ipmc_msg_dynamic_router_ports_get_rep_t;

/* IPMC message buffer */
typedef struct {
    vtss_os_sem_t   *sem; /* Semaphore */
    u8              *msg; /* Message */
} ipmc_msg_buf_t;

typedef struct {
    int                 mem_current_cnt;
    ipmc_memory_info_t  mem_pool_info[IPMC_MEM_TYPE_MAX];
} ipmc_mem_info_t;


/* ================================================================= *
 *  IPMC Configuration Defines
 * ================================================================= */
/**
 * Definition of configurations
 */
#define IPMC_DEF_GLOBAL_STATE_VALUE             VTSS_IPMC_FALSE
#define IPMC_DEF_UNREG_FLOOD_VALUE              VTSS_IPMC_TRUE
#define IPMC_DEF_PROXY_VALUE                    VTSS_IPMC_FALSE
#define IPMC_DEF_LEAVE_PROXY_VALUE              VTSS_IPMC_FALSE
#define IPMC_DEF_INTF_STATE_VALUE               VTSS_IPMC_FALSE
#define IPMC_DEF_SSM_PREFIX4_VALUE              VTSS_IPMC_SSM4_RANGE_PREFIX
#define IPMC_DEF_SSM_PREFIX4_LEN_VALUE          VTSS_IPMC_SSM4_RANGE_LEN
#define IPMC_DEF_SSM_PREFIX6_VALUE              VTSS_IPMC_SSM6_RANGE_PREFIX
#define IPMC_DEF_SSM_PREFIX6_LEN_VALUE          VTSS_IPMC_SSM6_RANGE_LEN
#define IPMC_DEF_INTF_QUERIER_VALUE             VTSS_IPMC_TRUE
#define IPMC_DEF_INTF_QUERIER_ADRS4             IPMC_PARAM_DEF_QUERIER_ADRS4
#define IPMC_DEF_INTF_COMPAT                    IPMC_PARAM_DEF_COMPAT
#define IPMC_DEF_INTF_PRI                       IPMC_PARAM_DEF_PRIORITY
#define IPMC_DEF_INTF_RV                        IPMC_PARAM_DEF_RV
#define IPMC_DEF_INTF_QI                        IPMC_PARAM_DEF_QI
#define IPMC_DEF_INTF_QRI                       IPMC_PARAM_DEF_QRI
#define IPMC_DEF_INTF_LMQI                      IPMC_PARAM_DEF_LLQI
#define IPMC_DEF_INTF_URI                       IPMC_PARAM_DEF_URI
#define IPMC_DEF_RTR_PORT_VALUE                 VTSS_IPMC_FALSE
#define IPMC_DEF_IMMEDIATE_LEAVE_VALUE          VTSS_IPMC_FALSE
#define IPMC_DEF_FLTR_PROFILE_STRLEN_VALUE      0x0
#define IPMC_DEF_THROLLTING_VALUE               0x0


/* This API is used to simulate the IPMC TX. */
BOOL ipmc_debug_pkt_send(ipmc_ctrl_pkt_t type,
                         ipmc_ip_version_t version,
                         vtss_vid_t vid,
                         vtss_ipv6_t *grp,
                         u8 iport,
                         u8 cnt,
                         BOOL untag);

/* Walk IPMC interface database */
vtss_rc ipmc_mgmt_intf_walk(BOOL next, ipmc_intf_map_t *entry);

/* This API is used to get the current count number of IPMC memory usage. */
BOOL ipmc_debug_mem_get_info(ipmc_mem_info_t *info);

#endif /* _IPMC_H_ */
