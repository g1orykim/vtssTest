/*

 Vitesse Switch Application software.

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

#ifndef __VTSS_ERPS_H__
#define __VTSS_ERPS_H__

#include "vtss_erps_api.h"

#define ERPS_VLAN_MAX                        4095

#define   ERPS_STATE_MAX                     5

/* 
 * event_id's local to FSM, need to convert event_id's to these
 * before calling FSM handlers. Upon addition of new events same
 * need to be defined here. These events are arranged in priority
 * order. Hence don't change the order 
 */
enum {
    FSM_EVENT_LOCAL_CLEAR = 0,
    FSM_EVENT_LOCAL_FS,
    FSM_EVENT_REMOTE_FS,
    FSM_EVENT_LOCAL_SF,
    FSM_EVENT_LOCAL_CLEAR_SF,
    FSM_EVENT_REMOTE_SF,
    FSM_EVENT_REMOTE_MS,
    FSM_EVENT_LOCAL_MS,
    FSM_EVENT_LOCAL_WTR_EXPIRES,
    FSM_EVENT_LOCAL_WTR_RUNNING,
    FSM_EVENT_LOCAL_WTB_EXPIRES,
    FSM_EVENT_LOCAL_WTB_RUNNING,
    FSM_EVENT_REMOTE_NR_RB,
    FSM_EVENT_REMOTE_NR,
    FSM_EVENT_REMOTE_EVENT,
    FSM_EVENT_LOCAL_HOLDOFF_EXPIRES,
    FSM_EVENTS_MAX,
    FSM_EVENT_INVALID = 0xFF    /* Invalid event */
};
#define ERPS_CHECK_FLAG(V,F)                 (((V) & (F)) ?1 : 0 )
#define ERPS_SET_FLAG(V,F)                   (V) |= (F)
#define ERPS_UNSET_FLAG(V,F)                 (V) &= ~(F)

#define ERPS_PDU_REQ_STATE_MASK   0xf0       /*  4 bits */
#define ERPS_PDU_RESERVED_MASK    0x0f       /*  4 bits */

#define GET_REQUEST_STATE(RS) \
        (((u8)(RS) & ERPS_PDU_REQ_STATE_MASK) >> 4)

#define GET_RESERVED(RS) \
        ((u8)(RS) & ERPS_PDU_RESERVED_MASK)

#define SET_REQUEST_STATE(type, res) \
        ((((type) << 4) & ERPS_PDU_REQ_STATE_MASK) \
        | ((res)        & ERPS_PDU_RESERVED_MASK))

#define ERPS_MAX_NODE_ID_LEN     6

/* as per ITUT-G.8032(V2), Section 10.1.10 */
typedef struct vtss_erps_flush_logic
{
    u8 node_id[ERPS_MAX_NODE_ID_LEN];
    u8 bpr;
}vtss_erps_flush_logic_t;
/*
 * This data strcture contains all the run time FSM details
 * i.e current state, current event and even maintains R-APS
 * PDU statistics
 */
typedef struct erps_fsm {

    u8                 current_state;
    u8                 admin_cmd;

#define RING_BLOCKED_EAST            1
#define RING_BLOCKED_WEST            2
#define RING_BLOCKED_NONE            3
#define RING_BLOCKED_BOTH            4
  /* what if two ports are blocked */
    vtss_port_no_t     current_blocked;

   /* 
    * port where local SF occured, these represents the exact status of the 
    * ports( while hold-off is running and not handed over the same SF to ERPS
    * state machine.  ERPS state machine needs to know this information as well.
    * once hold-off is expired and SF still persists, east_blocked 
    * and east_blocked gets updated with the actual status.
    */
    vtss_port_no_t     lsf_port_east;
    vtss_port_no_t     lsf_port_west;

    vtss_port_no_t     l_event_port;

    /* 
     * these two represents the current blocked status of the 
     * east,west ports, incase if ERPS wants to check weather a given port is
     * blocked or unblocked, a check can be made against these two for fidning
     * out exact status 
     */
    vtss_port_no_t     east_blocked;
    vtss_port_no_t     west_blocked;

    /* indication of receiving higher node_id's */
    BOOL               remote_higher_nodeid;
    BOOL               fop_alarm;

#define ERPS_START_RAPS_TX        1
#define ERPS_STOP_RAPS_TX         0
   /* a flag indicating whether to send R-APS PDUS or not */
    u8                 raps_tx;

   /* WTR, Guard & Hold-Off timeout */
    u64                wtr_timeout;
    u64                hold_off_timeout;
    u64                guard_timeout;
    u64                wtb_timeout;
    u64                fop_timeout;
    u8                 wtr_running;
    u8                 hold_off_running;
    u8                 guard_timer_running;
    u8                 wtb_running;
    u8                 fop_running;
   
    /* fileds copied from incoming R-APS PDU */
    /* rpl blocked */
    u16                raps_rx[2];
    u16                req[2];
    u8                 rb[2];
    u8                 dnf[2];
    u8                 bpr[2];
    u8                 node_id[2][ERPS_MAX_NODE_ID_LEN];

    /* flush logic */
    vtss_erps_flush_logic_t  rcvd_fl[2];
    vtss_erps_flush_logic_t  stored_fl[2];

    /* protection group R-APS statistics */
    vtss_erps_statistics_t   erps_stats;

    /* if dnf == 1, sending DNF sent as 0 otherwise 
       DNF is 1 */
#define ERPS_SET_DNF        1
#define ERPS_UNSET_DNF      2
    u8                 dnf_out;
    BOOL               wtr_sf_on_rpl;   /* Remember if SF was on RPL */
    
    u16                tx_req;
    u16                tx_rb;
    u16                tx_dnf;
    u16                tx_bpr;

    u64                rx_timestamp;
} erps_fsm_t;


#define RAPS_DIRECTION_EAST    1
#define RAPS_DIRECTION_WEST    2
/*
 * This strcture contains all the configured protection group
 * information.
 */
typedef struct erps_protection_group {

#define ERPS_NODE_RPL_OWNER                   1
#define ERPS_NODE_NON_RPL_OWNER               0
    u16                   rpl_owner;
    u8                    rpl_blocked;
                     
    u8                    erps_status;
    vtss_port_no_t        blocked_port;
                     
    u32                   group_id;
    vtss_port_no_t        east_port;
    vtss_port_no_t        west_port;
                     
    u32                   p_vids_configured;
    vtss_vid_t            protected_vlans[PROTECTED_VLANS_MAX];
                     
    u64                   wtr_time;
    u64                   guard_time;
    u64                   holdoff_time;
    u64                   tc_timeout;  /* Topology change time to disable after enable */
    u8                    tc_running;
    BOOL                  topology_change;

    /* revertive is supported as per ITUT G.8032 */
    u16                   revertive;
                     
    u8                    node_id_e[ERPS_MAX_NODE_ID_LEN];
    u8                    node_id_w[ERPS_MAX_NODE_ID_LEN];
                     
    BOOL                  rpl_neighbour;
    vtss_port_no_t        rpl_neighbour_port;

    /* erpsv2 related stuff start */
    vtss_erps_ring_type_t ring_type; 
    BOOL                  raps_virt_channel;
    BOOL                  interconnected_node;
    u32                   major_ring_id;
    BOOL                  topology_propogate;
    vtss_erps_version_t   erps_version; 
    u64                   wtb_time;
    /* erpsv2 related stuff end */

    /* ERPS Finite State Machine information */
    struct erps_fsm       erps_instance;

} erps_protection_group_t;

/*
 * newly received R-APS PDU being copied into erps_pdu_t strcture
 * and handovers to FSM for further processing.
 */
typedef struct erps_pdu
{
#define ERPS_REQ_NR         0x0 /* (0000) */
#define ERPS_REQ_SF         0xb /* (1011) */
#define ERPS_REQ_FS         0xd /* (1101) */
#define ERPS_REQ_EVENT      0xe /* (1110) */
#define ERPS_REQ_MS         0x7 /* (0111) */

#define ERPS_PDU_SIZE                        12
    u8  req_state;

#define ERPS_PDU_RESERVED             0
    u8  reserved;

#define ERPS_PDU_NOREQUEST_RB_MASK    ( 1 << 7 )
    u8  rb;

#define ERPS_PDU_DNF_MASK             ( 1 << 6 )
    u8  dnf;

#define ERPS_BPR_EAST                 0
#define ERPS_BPR_WEST                 1
#define ERPS_PDU_BPR_MASK             ( 1 << 5 )
    u8  bpr;

    u8  node_id[ERPS_MAX_NODE_ID_LEN];
} erps_pdu_t;


static i32 erps_event_not_applicable (erps_protection_group_t*);
static i32 erps_fsm_event_ignore (erps_protection_group_t *);
static i32 vtss_erps_build_raps_pkt (erps_protection_group_t *erpg,
                                     u8      *pdu,
                                     u8      req_state,
                                     u8      sub_code,
                                     BOOL    rb,
                                     BOOL    dnf,
                                     BOOL    bpr);

static i32 vtss_erps_apply_priority_logic (erps_protection_group_t *, i32 *);

/* event handlers for IDLE State */
static i32 erps_idle_event_handle_local_sf (erps_protection_group_t*);
static i32 erps_idle_event_handle_local_holdoff_expires (erps_protection_group_t*);
static i32 erps_idle_event_handle_local_sf (erps_protection_group_t*);
static i32 erps_idle_event_handle_local_clear_sf(erps_protection_group_t *);
static i32 erps_idle_event_handle_local_fs (erps_protection_group_t *);
static i32 erps_idle_event_handle_local_ms (erps_protection_group_t *);
static i32 erps_idle_event_handle_remote_sf (erps_protection_group_t*);
static i32 erps_idle_event_handle_remote_nr (erps_protection_group_t*);
static i32 erps_idle_event_handle_remote_nr_rb (erps_protection_group_t*);
static i32 erps_idle_event_handle_remote_fs (erps_protection_group_t *);
static i32 erps_idle_event_handle_remote_ms (erps_protection_group_t *);

/* event handlers for PROTECTED State */
static i32 erps_prot_event_handle_local_sf (erps_protection_group_t*);
static i32 erps_prot_event_handle_local_clear_sf (erps_protection_group_t*);
static i32 erps_prot_event_handle_local_fs (erps_protection_group_t *);
//static i32 erps_prot_event_handle_local_clear (erps_protection_group_t *erpg);
static i32 erps_prot_event_handle_local_holdoff_expires (erps_protection_group_t *);
static i32 erps_prot_event_handle_remote_nr (erps_protection_group_t*);
static i32 erps_prot_event_handle_remote_nr_rb (erps_protection_group_t*);
static i32 erps_prot_event_handle_remote_fs (erps_protection_group_t *erpg);

/* event handlers for FORCED_SWITCH State */
static i32 erps_fs_event_handle_local_holdoff_expires (erps_protection_group_t *);
static i32 erps_fs_event_handle_local_fs (erps_protection_group_t *);
static i32 erps_fs_event_handle_local_clear (erps_protection_group_t *);
static i32 erps_fs_event_handle_remote_nr (erps_protection_group_t *);
static i32 erps_fs_event_handle_remote_nr_rb (erps_protection_group_t *);

/* event handlers for MANUAL_SWITCH State */
static i32 erps_ms_event_handle_local_sf (erps_protection_group_t *);
static i32 erps_ms_event_handle_local_clear_sf (erps_protection_group_t *);
static i32 erps_ms_event_handle_local_fs (erps_protection_group_t *);
static i32 erps_ms_event_handle_local_clear (erps_protection_group_t *);
static i32 erps_ms_event_handle_local_holdoff_expires (erps_protection_group_t *);
static i32 erps_ms_event_handle_remote_nr (erps_protection_group_t *);
static i32 erps_ms_event_handle_remote_nr_rb (erps_protection_group_t *);
static i32 erps_ms_event_handle_remote_sf(erps_protection_group_t *);
static i32 erps_ms_event_handle_remote_fs(erps_protection_group_t *);
static i32 erps_ms_event_handle_remote_ms(erps_protection_group_t *);

/* event handlers for PENDING State */
static i32 erps_pend_event_handle_local_sf (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_holdoff_expires (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_fs (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_ms (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_wtb_running (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_wtb_expires (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_wtr_running (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_wtr_expires (erps_protection_group_t *);
static i32 erps_pend_event_handle_local_clear (erps_protection_group_t *);
static i32 erps_pend_event_handle_remote_nr (erps_protection_group_t *);
static i32 erps_pend_event_handle_remote_nr_rb (erps_protection_group_t *);
static i32 erps_pend_event_handle_remote_sf(erps_protection_group_t *);
static i32 erps_pend_event_handle_remote_fs(erps_protection_group_t *);
static i32 erps_pend_event_handle_remote_ms(erps_protection_group_t *);

#endif /* __VTSS_ERPS_H__ */
