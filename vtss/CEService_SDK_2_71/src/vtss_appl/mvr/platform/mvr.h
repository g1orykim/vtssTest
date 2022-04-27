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

#ifndef _MVR_H_
#define _MVR_H_

#include "mvr_conf.h"
#include "vtss_mvr.h"


#define VTSS_IPMCMVR_DISABLED       VTSS_IPMC_DISABLE
#define VTSS_IPMCMVR_ENABLED        VTSS_IPMC_ENABLE

#define MVR_DEF_INTF_STATE_VALUE    VTSS_IPMCMVR_ENABLED
#define MVR_DEF_INTF_QUERIER_VALUE  VTSS_IPMCMVR_DISABLED


/* ================================================================= *
 *  MVR stack messages
 * ================================================================= */
/* MVR request message timeout */
#define MVR_REQ_TIMEOUT                     12345   /* in msec */

/* MVR messages IDs */
typedef enum {
    MVR_MSG_ID_GLOBAL_SET_REQ = 0,          /* MVR MODE SET request (no reply) */
    MVR_MSG_ID_SYS_MGMT_SET_REQ,            /* MVR System IPv4 Address set request (no reply) */
    MVR_MSG_ID_GLOBAL_PURGE_REQ,            /* Purge MVR Internal request (no reply) */

    MVR_MSG_ID_VLAN_ENTRY_SET_REQ,          /* MVR VLAN Entry SET request (no reply) */
    MVR_MSG_ID_VLAN_ENTRY_GET_REQ,          /* MVR VLAN Entry GET request (wait reply) */
    MVR_MSG_ID_VLAN_ENTRY_GET_REP,          /* MVR VLAN Entry GET-REPLY */
    MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ,   /* MVR GROUP Entry GET request (wait reply) */
    MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP,   /* MVR GROUP Entry GET-REPLY */
    MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ,      /* MVR SRCLIST Entry GET request (wait reply) */
    MVR_MSG_ID_GROUP_SRCLIST_WALK_REP,      /* MVR SRCLIST Entry GET-REPLY */

    MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ,     /* MVR FAST LEAVE set request (no reply) */

    MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ,      /* MVR clear counter request (no reply) */
    MVR_MSG_ID_STP_PORT_CHANGE_REQ,         /* MVR STP-PSC call-baack handler (no reply) */

    MVR_MSG_MAX_ID
} mvr_msg_id_t;

/* MVR_MSG_ID_GLOBAL_SET_REQ:12 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    mvr_conf_global_t               global_setting;
} mvr_msg_global_set_req_t;

/* MVR_MSG_ID_SYS_MGMT_SET_REQ */
typedef struct {
    mvr_msg_id_t                    msg_id;
    u8                              mgmt_mac[6];
    u32                             intf_cnt;
    ipmc_mgmt_ipif_t                ip_addr[VTSS_IPMC_MGMT_IPIF_MAX_CNT];
} mvr_msg_sys_mgmt_set_req_t;

/* MVR_MSG_ID_GLOBAL_PURGE_REQ:8 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
} mvr_msg_purge_req_t;

/* MVR_MSG_ID_VLAN_ENTRY_SET_REQ:312 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    ipmc_operation_action_t         action;
    ipmc_ip_version_t               version;
    mvr_mgmt_interface_t            vlan_entry;
} mvr_msg_vlan_set_req_t;

/*
    MVR_MSG_ID_VLAN_ENTRY_GET_REQ
    MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ
    MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ
    :52
*/
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;

    vtss_vid_t                      vid;
    ipmc_ip_version_t               version;
    vtss_ipv6_t                     group_addr;

    vtss_ipv6_t                     srclist_addr;
    BOOL                            srclist_type;
} mvr_msg_vlan_entry_get_req_t;

/* MVR_MSG_ID_VLAN_ENTRY_GET_REP:136 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    ipmc_prot_intf_entry_param_t    interface_entry;
} mvr_msg_vlan_entry_get_rep_t;

/* MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:300 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    ipmc_prot_intf_group_entry_t    intf_group_entry;
} mvr_msg_vlan_group_entry_get_rep_t;

/* MVR_MSG_ID_GROUP_SRCLIST_WALK_REP:256 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    ipmc_prot_group_srclist_t       group_srclist_entry;
} mvr_msg_group_srclist_get_rep_t;

/* MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ:16 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    mvr_conf_fast_leave_t           fast_leave;
} mvr_msg_port_fast_leave_set_req_t;

/* MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ:12 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    vtss_vid_t                      vid;
} mvr_msg_stat_counter_clear_req_t;

/* MVR_MSG_ID_STP_PORT_CHANGE_REQ:16 */
typedef struct {
    mvr_msg_id_t                    msg_id;
    vtss_isid_t                     isid;
    vtss_port_no_t                  port;
    vtss_common_stpstate_t          new_state;
} mvr_msg_stp_port_change_set_req_t;

/* MVR Message Buffer */
typedef struct {
    vtss_os_sem_t                   *sem;   /* Semaphore */
    u8                              *msg;   /* Message */
} mvr_msg_buf_t;

#endif /* _MVR_H_ */
