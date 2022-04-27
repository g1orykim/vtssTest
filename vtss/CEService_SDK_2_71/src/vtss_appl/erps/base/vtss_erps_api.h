/*

 Vitesse Switch Application software.

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

#ifndef __VTSS_ERPS_API_H__
#define __VTSS_ERPS_API_H__

#include "vtss_types.h"

#define ERPS_MAX_PROTECTION_GROUPS           64
#define PROTECTED_VLANS_MAX                  63
#define ERPS_MIN_PROTECTION_GROUPS           1

#define RAPS_RPL_BLOCKED                      1
#define RAPS_RPL_NON_BLOCKED                  2

#define ERPS_PDU_SIZE                        12

#define SECS_TO_MS(sec) (sec*1000LLU)
#define MIN_TO_MS(sec) (sec*60000LLU)
#define SECS_TO_MINS(sec) (sec/60LLU)
#define MS_TO_MIN(ms) (ms/60000LLU)

/* WTR,Hold-Off and Guard Timeout max/min values */
#define  RAPS_WTR_TIMEOUT_MIN_MINUTES        1
#define  RAPS_WTR_TIMEOUT_MAX_MINUTES        12
#define  RAPS_WTR_TIMEOUT_DEFAULT_MINITES    RAPS_WTR_TIMEOUT_MIN_MINUTES

#define RAPS_GUARD_TIMEOUT_MIN_MILLISECONDS       10
#define RAPS_GUARD_TIMEOUT_MAX_SECONDS            2
#define RAPS_GUARDTIMEOUT_DEFAULT_MILLISECONDS    500

/* 
 * WTB is identified to be 5 seconds longer than guard timer, thus 
 * adding 5 seconds to the guard timeout values 
 *
 * section 10.1.4 ITUT-G.8032/Y.1344 (03/2010)
 */
#define RAPS_WTB_TIMEOUT_MIN_MILLISECONDS       (SECS_TO_MS(5)+10)
#define RAPS_WTB_TIMEOUT_MAX_SECONDS            7
#define RAPS_WTBTIMEOUT_DEFAULT_MILLISECONDS    (SECS_TO_MS(5)+500)

#define RAPS_HOLD_OFF_TIMEOUT_MIN_SECONDS         0
#define RAPS_HOLD_OFF_TIMEOUT_MAX_SECONDS         10
#define RAPS_HOLD_OFF_DEFAULT_TIMEOUT        RAPS_HOLD_OFF_TIMEOUT_MIN_SECONDS
#define RAPS_WTB_DEFAULT_TIMEOUT             RAPS_WTB_TIMEOUT_MIN_SECONDS

#define ERPS_TOPOLOGY_CHANGE_TIMEOUT         10
#define ERPS_FOP_TIMEOUT                     5000

#define ERPS_PROTECTION_GROUP_INACTIVE        0
#define ERPS_PROTECTION_GROUP_RESERVED        1
#define ERPS_PROTECTION_GROUP_ACTIVE          2

#define ERPS_STATE_NONE              0
#define ERPS_STATE_IDLE              1
#define ERPS_STATE_PROTECTED         2
#define ERPS_STATE_FORCED_SWITCH     3
#define ERPS_STATE_MANUAL_SWITCH     4
#define ERPS_STATE_PENDING           5

#ifndef API2ERPS_HWINSTANCE
#define API2ERPS_HWINSTANCE(i) (i)
#endif

#define CONV_MGMTTOERPS_INSTANCE(i) (i-1)
#define CONV_ERPSTOMGMT_INSTANCE(i) (i+1)

#define ERPS_CRITD_ENTER vtss_erps_crit_lock
#define ERPS_CRITD_EXIT  vtss_erps_crit_unlock

#define ERPS_MAX_NODE_ID_LEN     6

enum {
    /* ERPS_SUCCESS = MODULE_ERROR_START(VTSS_MODULE_ID_ERPS), */
    ERPS_RC_OK = VTSS_RC_OK,
    ERPS_ERROR_PG_CREATION_FAILED,
    ERPS_ERROR_GROUP_NOT_EXISTS,
    ERPS_ERROR_GROUP_ALREADY_EXISTS,
    ERPS_ERROR_INVALID_PGID,
    ERPS_ERROR_CANNOT_SET_RPL_OWNER_FOR_ACTIVE_GROUP,
    ERPS_ERROR_PG_IN_INACTIVE_STATE,
    ERPS_ERROR_NODE_ALREADY_RPL_OWNER,
    ERPS_ERROR_NODE_NON_RPL,
    ERPS_ERROR_SETTING_RPL_BLOCK,
    ERPS_ERROR_VLANS_CANNOT_BE_ADDED,
    ERPS_ERROR_VLAN_ALREADY_PARTOF_GROUP,
    ERPS_ERROR_VLANS_CANNOT_BE_DELETED,
    ERPS_ERROR_CONTROL_VID_PART_OF_ANOTHER_GROUP,
    ERPS_ERROR_INVALID_REMOTE_EVENT,
    ERPS_ERROR_INVALID_LOCAL_EVENT,
    ERPS_ERROR_EAST_AND_WEST_ARE_SAME,
    ERPS_ERROR_INCORRECT_ERPS_PDU_SIZE,
    ERPS_ERROR_RAPS_PARSING_FAILED,
    ERPS_ERROR_FAILED_IN_SETTING_VLANMAP,
    ERPS_ERROR_MOVING_TO_NEXT_STATE,
    ERPS_ERROR_CANNOT_ASSOCIATE_GROUP,
    ERPS_ERROR_ALREADY_NEIGHBOUR_FOR_THISGROUP,
    ERPS_ERROR_GROUP_IN_ACTIVE_MODE,
    ERPS_ERROR_GIVEN_PORT_NOT_EAST_OR_WEST,
    ERPS_ERROR_MAXIMUM_PVIDS_REACHED,
    ERPS_ERROR_RAPS_ENABLE_FORWARDING_FAILED,
    ERPS_ERROR_RAPS_DISABLE_FORWARDING_FAILED,
    ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED,
    ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED,
    ERPS_ERROR_SF_DERIGISTER_FAILED,
    ERPS_ERROR_SF_RIGISTER_FAILED,
    ERPS_ERROR_MOVING_TO_FORWARDING,
    ERPS_ERROR_MOVING_TO_DISCARDING,
    ERPS_ERROR_CANTBE_IC_NODE,
    ERPS_ERROR_NOT_AN_INTERCONNECTED_NODE,
    ERPS_ERROR_CANNOTBE_INTERCONNECTED_SUB_NODE,
    ERPS_PRIORITY_LOGIC_FAILED,
    ERPS_ERROR
};

typedef enum vtss_erps_ring_type
{
    ERPS_RING_TYPE_MAJOR,
    ERPS_RING_TYPE_SUB
} vtss_erps_ring_type_t;

typedef enum vtss_erps_admin_cmd
{
    ERPS_ADMIN_CMD_MANUAL_SWITCH,
    ERPS_ADMIN_CMD_FORCED_SWITCH,
    ERPS_ADMIN_CMD_CLEAR
} vtss_erps_admin_cmd_t;

typedef enum vtss_erps_version
{
    ERPS_VERSION_V1 = 1,
    ERPS_VERSION_V2
}vtss_erps_version_t;

typedef struct vtss_erps_statistics {
    u64 raps_sent;
    u64 raps_rcvd;
    u64 raps_rx_dropped;
    u64 local_sf;
    u64 local_sf_cleared;
    u64 remote_sf;
    u64 event_nr;
    u64 remote_ms;
    u64 local_ms;
    u64 remote_fs;
    u64 local_fs;
    u64 admin_cleared;
} vtss_erps_statistics_t;

/*
 * vtss_erps_config_erpg_t gets stored into the flash.
 */
typedef struct vtss_erps_config_erpg
{
    BOOL            enable;
    u64             hold_off_time;
    u64             wtr_time;
    u64             guard_time;
    /* 
     * if ring-type is sub-ring, then east_port contains sub-ring link
     */
    vtss_port_no_t         east_port;
    vtss_port_no_t         west_port;
    u32                    group_id;
    BOOL                   rpl_owner;
    vtss_port_no_t         rpl_owner_port;
    BOOL                   rpl_neighbour;
    vtss_port_no_t         rpl_neighbour_port;
    vtss_vid_t             protected_vlans[PROTECTED_VLANS_MAX];
    u32                    wtb_time;
    vtss_erps_ring_type_t  ring_type;
    BOOL                   inter_connected_node;
    u32                    major_ring_id;
    BOOL                   revertive;
    vtss_erps_version_t    version;
    BOOL                   topology_change; 
    BOOL                   virtual_channel;
}vtss_erps_config_erpg_t;

/*
 * used to get ERPS FSM related information
 */
typedef struct vtss_erps_fsm_stat
{
    u16    state;
    u64    wtr_remaining_time;
    u16    active;
#define ERPS_PORT_STATE_OK  1
#define ERPS_PORT_STATE_SF  2
    u16    east_port_state;
    u16    west_port_state;
    u16    east_blocked;
    u16    west_blocked;
    u16    rpl_blocked;
    u8     admin_cmd;
    BOOL   fop_alarm;

    u16    tx;
    u16    tx_req;
    u16    tx_rb;
    u16    tx_dnf;
    u16    tx_bpr;

    u16    rx[2];
    u16    rx_req[2];
    u16    rx_rb[2];
    u16    rx_dnf[2];
    u16    rx_bpr[2];
    u8     rx_node_id[2][ERPS_MAX_NODE_ID_LEN];
} vtss_erps_fsm_stat_t;

/*
 *  below structure is used for getting R-APS Group related information
 *  from core part of ERPS Protocol
 */
typedef struct vtss_erps_mgmt_conf
{
    u32 group_id;
    vtss_erps_config_erpg_t erpg;
    vtss_erps_fsm_stat_t    stats;
    vtss_erps_statistics_t  raps_stats;
}vtss_erps_base_conf_t;


/******************************************************************************
 *    ERPS Management call in Functions  
 ******************************************************************************/
/* erps_group_id :     ERPS protection group id                               */
/* east_port     :     east port of the ring instance (for interconnection
                       node, east_port contains Sub-Ring link                 */
/* west_port     :     west port of the ring instance (for interconnection 
                       node west_port contains Virtual Port                   */
/* ring_type     :     ring_type can be major-ring or sub-ring                */
/* interconnected :    TRUE for interconnected ring                           */
/* major_group_id :    For interconnected sub-ring, major ring group Id       */
/* virtual_channel:    TRUE for sub-rings with virtual channel                */
i32 vtss_erps_create_protection_group(u32                    erps_group_id,
                                      vtss_port_no_t         east_port,
                                      vtss_port_no_t         west_port,
                                      vtss_erps_ring_type_t  ring_type,
                                      BOOL                   interconnected,
                                      u32                    major_group_id,
                                      BOOL                   virtual_channel);

/* erps_group_id :     ERPS protection group id                               */
i32 vtss_erps_delete_protection_group(u32 erps_group_id);

/*================================== ERPS V2 Start ===========================*/
/* erps_group_id     :     ERPS protection group id                           */
/* major_ring_id     :     major ring id of sub ring                          */
i32 vtss_erps_enable_interconnected(u32 erps_group_id,
                                    u32 major_ring_id);

/* erps_group_id     :     ERPS protection group id                           */
i32 vtss_erps_disable_interconnected(u32 erps_group_id,
                                     u32 major_ring_id);

/* erps_group_id :     ERPS protection group id                               */
i32 vtss_erps_enable_non_reversion(u32 erps_group_id);

/* erps_group_id :     ERPS protection group id                               */
i32 vtss_erps_disable_non_reversion(u32 erps_group_id);

/* erps_group_id    : ERPS protection group id                                */
/* cmd              : Administrative command destined to a given 
                      protection group                                        */
/* port             : contains the requested ring port to be blocked */
i32 vtss_erps_admin_command(u32 erps_group_id, vtss_erps_admin_cmd_t cmd, vtss_port_no_t port);

/* erps_group_id :  ERPS protection group id                                  */
/* enable        :  TRUE represents topology propogate enable and FALSE 
                    represents topology propogate disable                     */
i32 vtss_erps_set_topology_change_propogation(u32 erps_group_id, BOOL enable);

/* erps_group_id :  ERPS protection group id                                  */
i32 vtss_erps_disable_virtual_channel(u32 erps_group_id);

/* erps_group_id :  ERPS protection group id                                  */
i32 vtss_erps_enable_virtual_channel(u32 erps_group_id);

/* erps_group_id :  ERPS protection group id                                  */
/* version       :  ERPS version to be set for a particular protection group  */
i32 vtss_erps_set_protocol_version(u32 erps_group_id, vtss_erps_version_t version);
/*================================== ERPS V2 End =============================*/

/* erps_group_id :  ERPS protection group id                                  */
/* protected_vid :  protected vlan_id                                         */
i32 vtss_erps_associate_protected_vlans (u32 erps_group_id, u16 protected_vid);

/* erps_group_id: ERPS protection group id for setting rpl block              */
/* port         : RPL Blocked port                                            */
/* rpl_replace  : to replace rpl block or not                                 */
i32 vtss_erps_set_rpl_owner(u32 erps_group_id, 
                            vtss_port_no_t port, u16 rpl_replace);

/* erps_group_id: ERPS protection group id for setting rpl block              */
/* rpl_block    : RPL Blocked port                                            */
i32 vtss_erps_unset_rpl_owner(u32 erps_group_id);

/* erps_group_id :     ERPS protection group id                               */
/* port          :     RPL Neighbour port                                     */
i32 vtss_erps_set_rpl_neighbour (u32 erps_group_id, vtss_port_no_t port);

/* erps_group_id :     ERPS protection group id                               */
i32 vtss_erps_unset_rpl_neighbour (u32 erps_group_id);

/* erps_group_id: ERPS protection group id for configuring guard timeout      */
/* timeout      : guard timeout                                               */
i32 vtss_erps_set_guard_timeout(u32 erps_group_id, u16 timeout);

/* erps_group_id: ERPS protection group id for configuring wtr timeout        */
/* timeout      : WTR timeout                                                 */
i32 vtss_erps_set_wtr_timeout(u32 erps_group_id, u64 timeout);

/* erps_group_id: ERPS protection group id for configuring guard timeout      */
/* timeout      : guard timeout                                               */
i32 vtss_erps_set_holdoff_timeout(u32 erps_group_id, u64 timeout );

/* erps_group_id: ERPS protection group id for configuring guard timeout      */
/* timeout      : WTB timeout                                                 */
i32 vtss_erps_set_wtb_timeout(u32 erps_group_id, u32 timeout );

/* erps_group_id :     ERPS protection group id                               */
/* num_vids      :     number of protected vlans                              */
/* protected_vid :     protected vlan_id                                      */
i32 vtss_erps_remove_protected_vlan (u32 erps_group_id, 
                                     u8  num_vids, u16 protected_vid);

/* erps_group_id :   ERPS Protection group id                                */
i32 vtss_erps_associate_group ( u32 erps_group_id);

/* erps_group_id :   ERPS Protection group id                                */
i32 vtss_erps_clear_statistics(u32 erps_group_id);

/******************************************************************************
 *    ERPS callout functions
 ******************************************************************************/
/* vid         :     vlan id part of the protection group, this is the vlan
                     on which protection is applicable for */
/* inst        :     each protection group is mapped with an ERPS Instance in
                     the Switch API. All vlan's related to a single protection
                     group need to be mapped to it                            */
vtss_rc vtss_erps_ctrl_set_vlanmap (vtss_vid_t vid, vtss_erpi_t inst,BOOL member);

/* l2port      :     layer 2 port                                             */
/* enable      :     enable or disable ingress filtering                      */
vtss_rc vtss_erps_vlan_ingress_filter (vtss_port_no_t l2port, BOOL enable);

/* port           :  port number related an ERPS group                        */
/* erps_instance  :  protection group instance number                         */
/* aps            :  R-APS protocol data to be transmitted                    */
/* event          :  R-APS event message or not                               */
vtss_rc vtss_erps_raps_tx (vtss_port_no_t port, u32 eps_instance, u8 *aps, BOOL event);

/* erps_group_id  : Group ID                                                  */
/* grp_status     : pointer to group status                                   */
i32 vtss_erps_get_protection_group_status(u32 erps_group_id, 
                                          u8  *grp_status);

/* inst           : erps instance number in the switch API                    */
/* port           : port related to given erpi instance                       */
/* state          : state of the given port, i.e either DISCARDING 
                       or FORWARDING                                          */
vtss_rc vtss_erps_protection_group_state_set ( vtss_erpi_t inst, 
                                               vtss_port_no_t port, 
                                               vtss_erps_state_t state);

/* inst           : erps instance number related to switch API                */
/* rplport        : rpl port of the ring                                      */
/* By default a given protected vlan is in discarding state on all the ports, 
   vlan state needs to be changed into forwarding soon as a protection group  
   is created                                                                 */
vtss_rc vtss_erps_put_protected_vlans_in_forwarding (vtss_erpi_t inst, 
                                                     vtss_port_no_t rplport);

/* port           : switch port                                               */
/* vid            : vlan id on which FDB flush takes place on the given port  */
vtss_rc vtss_erps_flush_fdb (vtss_port_no_t port, vtss_vid_t vid);

/* port           : ring port on which R-APS PDU forwarding is enabled or disabled */
/* erps_instance  : ERPS Instance Number                                     */
/* enable         : specifies weather R-APS forwarding is enabled or disabled*/
vtss_rc vtss_erps_raps_forwarding ( vtss_port_no_t port, u32 erps_instance,
                                    BOOL enable );

/* port           : one of the ring port on which R-APS transmission to done */
/* erps_instance  : ERPS Instance                                            */
/* enable         : R-APS message transmission is enabled or disabled        */
vtss_rc vtss_erps_raps_transmission ( vtss_port_no_t port,
                                      u32 eps_instance,
                                      BOOL enable );

/* port           : one of the ring port                                     */
/* mac            : mac address of the given ring port                       */
vtss_rc vtss_erps_get_port_mac (u32 port, u8 *mac);

/* supporting functions for critical sections handling */
void vtss_erps_crit_lock(void);
void vtss_erps_crit_unlock(void);
void vtss_erps_fsm_crit_lock(void);
void vtss_erps_fsm_crit_unlock(void);

/* supporting functions for getting current system time */
u64 vtss_erps_current_time (void);

/* supporting function to know whether ERPS is initialized properly or not   */
BOOL vtss_erps_is_initialized(void);

/* supporting function for priting trace messages on the console */
void vtss_erps_trace(const char  *const string,const u32   param);

/* supporting function for priting state change trace messages on the console */
void vtss_erps_trace_state_change(const char  *const string,const u32   param);

/******************************************************************************
 *    ERPS callin functions
 ******************************************************************************/
/*  port    : switch port i.e one of the ring port                           */
/*  pdu_len : R-APS pdu length                                               */
/*  raps_pdu: R-APS PDU                                                      */
/*  erpg_instance: ERPS Instance for which R-APS PDU destined to             */
i32 vtss_erps_rx (vtss_port_no_t port, u16 pdu_len, u8 *raps_pdu,
                  u32 erpg_instance);

/* timer thread to be invoked from platform, this function getting called at 
   10 MS resolutions                                                          */
void vtss_erps_timer_thread(void);

/* instance  : ERPS Instance                                                 */
/* sf_state  : signal failure state on a given ring port                     */
/* sd_state  : signal degrade state of a given ring port                     */
/* lport     : one of the ring port                                          */
vtss_rc vtss_erps_sf_sd_state_set (const u32 instance,
                                   const BOOL sf_state,
                                   const BOOL sd_state,
                                   vtss_port_no_t lport);

/* 
 * function used to get next protection group configaration 
 * from platform part of the code 
 */
vtss_rc vtss_erps_getnext_protection_group_by_id (vtss_erps_base_conf_t*);

vtss_rc vtss_erps_get_protection_group_by_id (vtss_erps_base_conf_t * erpg);

#endif /* __VTSS_ERPS_API_H__ */
