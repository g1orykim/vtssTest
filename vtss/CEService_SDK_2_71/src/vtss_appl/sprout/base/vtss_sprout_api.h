/*

 Vitesse Switch Software.

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

 $Id: vtss_sprout_api.h,v 1.97 2011/03/17 14:40:01 fj Exp $
 $Revision: 1.97 $


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/


/*
 * API for SPROUT module.
 *
 * The SPROUT module implements the topology protocol for Vitesse stacking.
 *
 * "vtss_sprout_" is used as common prefix for definitions in this API.
 */

/*
 * Introduction to SPROUT API
 * ==========================
 * SPROUT provides the following functionality:
 * + Topology control
 *   Discover topology (and topology changes) within a stack.
 *   Configure the switch according the to the discovered topology.
 *   Notify application when topology changes have occurred.
 * + Firmware interoperability check (SPROUT v2 only)
 * + Mirroring (optional)
 *   Exchange information on the location of the mirror port among
 *   the switches in the stack.
 *   Configure the mirror-related forwarding masks for forwarding
 *   of mirrored frames to the mirror port.
 * + IP address exchange (optional)
 *   Exchange switch IP addresses among the switches in the stack.
 * + Master election (optional)
 *   Election of a master switch within the stack.
 *
 * VTSS_SPROUT_V1:
 * + Global Link Aggregation Group - GLAG (optional)
 *   Exchange information on the location of GLAG member ports among
 *   the switches in the stack.
 *   Configuring aggregation masks corresponding to GLAG member
 *   port location is the responsibility of the application.
 *
 * The topology control part of SPROUT is mandatory. Utilizing the other parts
 * of SPROUT (Mirroring, GLAG, IP address exchange and master election) is optional.
 * If some of the optional SPROUT functionality is not utilized, then the
 * corresponding API calls shall not be used.
 *
 * The following outlines how an application using SPROUT must use this API.
 *
 *
 * Master Election (optional feature)
 * ===============
 * SPROUT itself does not need to elect a master switch (to SPROUT all
 * switches are equal). The master election is thus purely supported as
 * a service to the upper level application, which may require election of
 * a master within the stack.
 * The following parameters control the master election algorithm:
 * + Master capable bit (mst_capable)
 *   Only switches which have mst_capable=1 are candidates for becoming master.
 *   mst_capable should be used to ensure that switches, which do not have the
 *   required master hardware configuration, become master. E.g. if the master
 *   must have more RAM than the slave switches.
 * + Master election priority (mst_elect_prio)
 *   A number 1-4 controlling the probability of a given switch becoming master.
 *   Smaller priority => Higher probability for becoming master.
 * + Switch address
 *   Switch address (which must be unique among the switches in the stack) is
 *   used as the final tie-breaker in the master election algorithm.
 * + Master time
 *   The number of seconds a switch has been master. This parameter is
 *   used to avoid unnecessary master reelection.
 * + Ignore master time (mst_time_ignore)
 *   When set, the master time is ignored in the election algorithm.
 *   This can be used to enforce a master reelection (e.g. after changing
 *   master election priorities.
 *
 * The master election algorithm works as follows:
 * 1) If no switch has set mst_time_ignore:
 *      if any switch(es) has been master for more than than
 *      VTSS_SPROUT_MST_TIME_MIN, then choose the one, which
 *      has been master for the longest period of time.
 * 2) Pick the switch with the smallest mst_elect_prio.
 * 3) Pick the switch with the smallest switch_addr.
 *
 * At startup, SPROUT assumes that the switch is slave. If it becomes
 * master, VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST event will be generated
 * (callback to vtss_sprout_callback.state_change() ).
 *
 *
 * Initialization
 * ==============
 * Call vtss_sprout_init().
 * Call vtss_sprout_switch_init().
 * Call vtss_sprout_stack_port_adm_state_set() for each stack port.
 *
 *
 * Link up/down, packet reception & periodic activation
 * ====================================================
 * Call vtss_sprout_stack_port_link_state_set() whenever link goes up or
 * down on a stack port.
 *
 * Call vtss_sprout_rx_pkt() when a SPROUT PDU is received.
 *
 * Call vtss_sprout_service_100msec() every ~100msec.
 *
 *
 * Configuration (optional features)
 * =============
 * The following describes how to use the optional parts of SPROUT.
 *
 * Mirroring
 * ---------
 * Call vtss_sprout_have_mirror_port_set() to inform SPROUT whether the mirror port
 * is located on this switch. In addition to informing other switches about
 * the mirror port location, SPROUT will also configure the mirror forwarding
 * registers accordingly.
 * Only one mirror port is supported within a stack, but multiple probe
 * ports may mirror to the mirror port.
 *
 * Global Link Aggregation Groups (only for VTSS_SPROUT_V1)
 * ------------------------------
 * Call vtss_sprout_glag_mbr_cnt_set() whenever the number of local ports in a
 * GLAG changes.
 * Only local ports with link up must be considered part of a GLAG.
 * SPROUT does not configure GLAG forwarding masks, it only conveys
 * information about GLAG member port location between the switches.
 * The application is responsible for configuring the relevant forwarding
 * masks appropriately (when the state_change callback function is called, see
 * below).
 *
 * IP Address
 * ----------
 * Call vtss_sprout_ipv4_addr_set() to inform SPROUT of the switch IPv4 address.
 *
 * Master Election
 * ---------------
 * Call vtss_sprout_parm_set() to:
 * a) Configure master election priority (VTSS_SPROUT_PARM_MST_ELECT_PRIO)
 * b) Set VTSS_SPROUT_PARM_MST_ELECT_PRIO for a short period of time to
 *    enforce a master reelection.
 *
 * Semaphores
 * ==========
 * To support multi-threaded environments, SPROUT implements two semaphores:
 * state_data & tbl_rd. These two semaphores protects the
 * internal state information within SPROUT against concurrent
 * access from multiple threads.
 *
 * Any SPROUT API function needing to read and write to the internal SPROUT
 * state information acquires both semaphores.
 *
 * Any SPROUT API function only needing to read internal SPROUT state
 * information acquires only the tbl_rd semaphore.
 *
 * The semaphores are created by vtss_sprout_init() and a token is posted to the
 * semaphores by vtss_sprout_switch_init().
 *
 * When the state_change callback function is called, the state_data semaphore
 * is acquired, but the tbl_rd semaphore is not acquired.
 * The callback function is thus allowed to read additional data from SPROUT
 * but must otherwise not call SPROUT API functions.
 *
 * The semaphores are implemented using vtss_os.h.
 *
 * To enable semaphores, VTSS_SPROUT_MULTI_THREAD must be set.
 * If the SPROUT API is only called from one thread, then VTSS_SPROUT_MULTI_THREAD
 * should not be set, thus disabling use of semaphores.
 *
 *
 * Reacting on state changes
 * =========================
 * The state_change callback functions must be implemented to react
 * on the state changes detected by SPROUT.
 * When the state_change function is called, vtss_sprout_sit_get() must
 * be called to retrieve details on the current SPROUT state.
 *
 * TTL Change => Fast MAC aging (only for VTSS_SPROUT_V1)
 * ----------------------------
 * If the state_change function signals that SPROUT has changed the
 * TTL of the stack ports, then the application should set the MAC table
 * age timer to a value of VTSS_SPROUT_FAST_MAC_AGING_TIMER for a period of
 * VTSS_SPROUT_FAST_MAC_AGING_PERIOD.
 *
 * GLAG change (only for VTSS_SPROUT_V1)
 * -----------
 * If SPROUT is used to convey information on GLAG member port location
 * the application must use the GLAG member port distribution returned by
 * vtss_sprout_sit_get() to calculate and configure GLAG forwarding masks.
 *
 * The calculation of GLAG forwarding masks can be viewed as "assigning"
 * AC values to each of the GLAG member ports. Every switch in the stack
 * must assign the same AC values to each GLAG member port.
 * To do so, the member ports must thus be "sorted" in a consistent order
 * across the stack. This sorting can be based on the switch addresses
 * (which are unique across the stack).
 * Thus if a GLAG has "glag_mbr_cnt" member ports, then the member port number
 * (as per the sorted member list), which a given AC value forwards to, can be
 * calculated as:
 *     ac % glag_mbr_cnt
 * This way GLAG forwarding can be setup consistently across the stack
 * without exchanging any other information than the number of GLAG member
 * ports located each switch (identified by its switch address) in the stack.
 *
 *
 * Dependencies
 * ============
 * SPROUT requires the following modules:
 * - Trace module (vtss_appl/util/vtss_trace*)
 * - Switch API (vtss_api/...)
 *
 *
 * Switch Setup
 * ============
 * SPROUT will call the following switch API functions to setup the switch
 * silicon:
 * + vtss_vstax_setup_set()
 *   Setup basic stacking parameters.
 *   This function should not be called from the application.
 * + vtss_vstax_port_setup_set()
 *   This function should not be called from the application.
 * + vtss_mac_table_port_flush() (only for VTSS_SPROUT_V1)
 *   Called to flush MAC addresses learned on stack port.
 * + vtss_vstax_topology_set() (only for VTSS_SPROUT_V2)
 *   Setup UPSID routing table and other topology related setttings.
 *
 *
 * Compilation Directives
 * ======================
 * The following defines are used to control the compilation of sprout:
 * + VTSS_SPROUT_V1, VTSS_SPROUT_V2
 *   Version 1 and version 2 of SPROUT. Exactly one of these two defines
 *   must be defined.
 * + VTSS_SPROUT_CRIT_CHK
 *   If set additional checks on semaphore handling is made.
 * + VTSS_OPSYS_ECOS
 *   To be set when using eCos.
 * + VTSS_SPROUT_DEFAULT_TRACE_LVL
 *   Default trace level for all SPROUTs trace groups.
 *   If unspecified, VTSS_TRACE_LVL_ERROR is used.
 *   It is recommended always to have error trace enabled.
 * + VTSS_SPROUT_MULTI_THREAD
 *   Must be set to 1 for multi-thread environments, i.e. SPROUT API
 *   may be called from different threads.
 *   Should be set to 0 for single-thread environments.
 *   If 1, semaphores are instantiated and used.
 * + VTSS_TRACE_LVL_MIN
 *   Through vtss_trace_api.h this controls the amount of trace compiled
 *   into SPROUT.
 * + VTSS_SWITCH
 *   Must be set to 1 when compiling SPROUT for Vitesse turnkey SW solution
 *   (managed as well as unmanaged).
 *   If not defined, 0 is assumed.
 * + VTSS_SPROUT_FW_VER_CHK
 *   Only allow stack connectivity between switches with same firmware version.
 *   It is recommended to set VTSS_SPROUT_FW_VER_CHK.
 *
 * Obsoleted Directives
 * --------------------
 * + VTSS_SPROUT_UNMGD (not supported for VTSS_SPROUT_V2) - OBSOLETED!
 *   Unmanaged switch.
 *   When set to 1, code for local GLAG members, local mirror port, IP address
 *   and master election is removed.
 *   If not defined, 0 is assumed.
 */

#ifndef _VTSS_SPROUT_API_H_
#define _VTSS_SPROUT_API_H_

#include "vtss_api.h"

/* ###########################################################################
 * Defines and typedefs
 * ------------------------------------------------------------------------ */

/* Set version depending on chip archtecture */
#if defined(VTSS_ARCH_LUTON28)
#define VTSS_SPROUT_V1
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_SPROUT_V2
#endif

/* Check that version has been properly selected */
#if !defined(VTSS_SPROUT_V1) && !defined(VTSS_SPROUT_V2)
#error Neither of VTSS_SPROUT_V1 and VTSS_SPROUT_V2 defined
#endif

#if defined(VTSS_SPROUT_V1) && defined(VTSS_SPROUT_V2)
#error VTSS_SPROUT_V1 and VTSS_SPROUT_V2 both defined
#endif

#if defined(VTSS_SPROUT_V2) && defined(VTSS_SPROUT_UNMGD) && VTSS_SPROUT_UNMGD==1
#error VTSS_SPROUT_UNMGD not supported for VTSS_SPROUT_V2
#endif

/* Maximum number of VStaX chips in switch */
#if defined(VTSS_SPROUT_V1)
#define VTSS_SPROUT_MAX_LOCAL_CHIP_CNT 1
#elif defined(VTSS_SPROUT_V2)
#define VTSS_SPROUT_MAX_LOCAL_CHIP_CNT 2
#else
#error Unknown SPROUT version
#endif

/* Only allow stack connectivity between switches with same firmware version */
#if defined(VTSS_SPROUT_V2)
/* Must be set for VTSS_SPROUT_V2 */
#define VTSS_SPROUT_FW_VER_CHK
#else
/* Optional, but recommended, for VTSS_SPROUT_V1 */
#define VTSS_SPROUT_FW_VER_CHK
#endif

/* Maximum number of chip/units in a stack */
#define VTSS_SPROUT_MAX_UNITS_IN_STACK 32
/* Maximum number of chip/units in a switch */
#define VTSS_SPROUT_MAX_UNITS_IN_SWITCH 2
/* Set maximum table size slightly larger to handle transitional unit counts during quick removal+insertion */
#define VTSS_SPROUT_SIT_SIZE           (VTSS_SPROUT_MAX_UNITS_IN_STACK + VTSS_SPROUT_MAX_UNITS_IN_STACK/2)

/* Master election priority.
 * Smaller priority => higher probability of becoming master */
#define VTSS_SPROUT_MST_ELECT_PRIO_START 1
#define VTSS_SPROUT_MST_ELECT_PRIOS      4
#define VTSS_SPROUT_MST_ELECT_PRIO_END   (VTSS_SPROUT_MST_ELECT_PRIO_START+VTSS_SPROUT_MST_ELECT_PRIOS)
typedef uint vtss_sprout_mst_elect_prio_t;

/* Chip index (within switch) */
#define VTSS_SPROUT_CHIP_IDX_START 1
#define VTSS_SPROUT_CHIP_IDXS      2
#define VTSS_SPROUT_CHIP_IDX_END   (VTSS_SPROUT_CHIP_IDX_START+VTSS_SPROUT_CHIP_IDXS)
typedef uint vtss_sprout_chip_idx_t;

/* Distance (hop count) to unit. 0..  -1 => Infinity */
#define VTSS_SPROUT_DIST_INFINITY -1
typedef int vtss_sprout_dist_t;

/* Parameters to be saved in non-volatile memory by application */
typedef struct _vtss_sprout_cfg_save_t {
    vtss_vstax_upsid_t upsid[VTSS_SPROUT_MAX_LOCAL_CHIP_CNT]; /* UPSIDs */
} vtss_sprout_cfg_save_t;


/* Use of bits in state_change_mask-argument for topo_state_change */
/* TTL changed (including link up/down) */
#define VTSS_SPROUT_STATE_CHANGE_MASK_TTL           (1 << 0)
#if defined(VTSS_SPROUT_V1)
/* GLAG member count on unit(s) have changed */
/* Note: Only supported for SPROUT v1 */
#define VTSS_SPROUT_STATE_CHANGE_MASK_GLAG          (1 << 1)
#endif
/* Switch(es) added/removed from stack and/or forwarding path changed */
#define VTSS_SPROUT_STATE_CHANGE_MASK_STACK_MBR     (1 << 2)
/* New master elected */
#define VTSS_SPROUT_STATE_CHANGE_MASK_NEW_MST       (1 << 3)
/* We have become master! */
#define VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST        (1 << 4)
/* We have become slave! */
#define VTSS_SPROUT_STATE_CHANGE_MASK_ME_SLV        (1 << 5)
/* One or more UPSID(s) of remote switch have changed */
#define VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_REMOTE  (1 << 6)
/* One or more UPSID(s) of local switch have changed */
#define VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_LOCAL   (1 << 7)


/* Fast MAC aging upon TTL change (see API description) */
#define VTSS_SPROUT_FAST_MAC_AGING_TIMER  8 /* 8 seconds */
#define VTSS_SPROUT_FAST_MAC_AGING_PERIOD 8 /* 8 seconds */

/* Firmware version length */
#define VTSS_SPROUT_FW_VER_LEN 80

typedef struct _vtss_sprout_callback_t {
    /* Log messages to event log */
    vtss_rc (*log_msg)(
        char *); /* Text string */

    /*
     * Saving configuration information generated by SPROUT.
     * E.g. UPSIDs
     */
    vtss_rc (*cfg_save)(
        vtss_sprout_cfg_save_t *);

    /*
     * SPROUT state change
     * Mask specifies what has changed (ref. VTSS_SPROUT_STATE_CHANGE...)
     */
    vtss_rc (*state_change)(
        /* Mask with changes. More than one bit may be set */
        uchar state_change_mask
    );

    /*
     * Transmit packet with VStaX2 header on super priority queue
     *
     * In front of pkt_p, 12 bytes is reserved for insertion of
     * VStaX2 header. I.e. to insert a VStaX2 header, tx_vstax2_pkt
     * must
     * - move DMAC+SMAC to position [pkt_p-12 .. pkt_p-1]
     * - insert VStaX2 header at position [pkt_p .. pkt_p+11]
     * - transmit the final packet starting at position pkt_p-12
     *
     * len does NOT include the VStaX2 header.
     */
    vtss_rc (*tx_vstax2_pkt)(
        vtss_port_no_t         port_no,
        vtss_vstax_tx_header_t *vstax2_hdr_p,
        uchar                  *pkt_p,
        uint                   len);

    /*
     * Set thread priority (back) to normal.
     * The function is called upon transmission of a SPROUT Alert,
     * such that the calling module may increase the thread priority
     * on link down and have it set back to normal upon Alert TX.
    */
    void (*thread_set_priority_normal)(void);

    /* Seconds since switch booted */
    ulong (*secs_since_boot)(void);

#if defined(VTSS_SPROUT_FW_VER_CHK)
    // Firmware interoperability check
    //
    // The main purpose of this function is to allow the application to
    // control whether connectivity should be established with a neighbor
    // with a different firmware version.
    //
    // Must return VTSS_RC_OK if interoperable. Otherwise VTSS_RC_ERROR.
    //
    // As an addititional service to the application, the port_no and
    // link_up parameters are also included and the function is also
    // called on SPROUT link-down events (whether administrative or
    // physical link down). This can be used by the application to
    // maintain a state telling the user whether there is currently
    // an interoperability issue to be fixed (e.g. through signalling
    // with an LED).
    vtss_rc (*fw_ver_chk)(
        // Port number where SPROUT update with fw_ver was received
        const vtss_port_no_t port_no,
        // Link up.
        const BOOL           link_up,
        // fw_ver to check
        // Valid only if link_up==1
        const uchar          *nbr_fw_ver);
#endif
} vtss_sprout_callback_t;


/* Application specific information exchanged by SPROUT */
#define VTSS_SPROUT_SWITCH_APPL_INFO_LEN 8
typedef uchar vtss_sprout_switch_appl_info_t[VTSS_SPROUT_SWITCH_APPL_INFO_LEN];

/* SPROUT initialization record */
typedef struct _vtss_sprout_init_t {
    vtss_sprout_callback_t   callback;
} vtss_sprout_init_t;


/*
 * Switch address. Must be unique within stack.
 * It is recommended to use the MAC address of the switch.
 */
#define VTSS_SPROUT_SWITCH_ADDR_NULL {{0,0,0,0,0,0}}
typedef struct _vtss_sprout_switch_addr_t {
    uchar addr[6];
} vtss_sprout_switch_addr_t;


/* Stack port setup */
typedef struct _vtss_sprout_stack_port_setup_t {
    /* Stack port number */
    vtss_port_no_t port_no;
} vtss_sprout_stack_port_setup_t;


/* Setup stacking parameters for chip */
typedef struct _vtss_sprout_chip_setup_t {
    /*
     * Preferred UPSID
     * Should be set to value saved due to call to cfg_save.
     * If no value has been saved, use VTSS_SPROUT_UPSID_UNDEF
     * (= no preference).
     */
    vtss_vstax_upsid_t             upsid_pref;

    // TOETBD-2U: Maybe move this to switch_init_t
    /*
     * Stack port numbers.
     * For dual unit switches with SPROUT only running on one unit
     * (the "primary unit"), the stack port numbers are stored in
     * chip[0].stack_port[0/1].
     * chip[1].stack_port[0/1] is not used.
     *
     * Values for EStaX34:
     *   [0] = 25
     *   [1] = 26
     */
    vtss_sprout_stack_port_setup_t stack_port[2];
} vtss_sprout_chip_setup_t;


/*
 * SPROUT switch settings
 */
typedef struct _vtss_sprout_switch_init_t {
    vtss_sprout_switch_addr_t    switch_addr;

    /* CPU queue to be used for SPROUT PDUs */
    vtss_packet_rx_queue_t       cpu_qu;

    /* Specify whether this switch may be elected as master */
    BOOL                         mst_capable;

    /* Initial value of switch_appl_info */
    /* May be changed later using vtss_sprout_switch_appl_info_set() */
    uchar switch_appl_info[8];

#if defined(VTSS_SPROUT_FW_VER_CHK)
    /* Firmware version */
    uchar                        my_fw_ver[VTSS_SPROUT_FW_VER_LEN];
#endif

    /*
     * Setup for each VStaX chip in the switch
     *
     * For multi-chip swiches chip[0] is the primary chip,
     * i.e. chip with active CPU.
     */
    vtss_sprout_chip_setup_t     chip[VTSS_SPROUT_MAX_LOCAL_CHIP_CNT];
} vtss_sprout_switch_init_t;


/*
 * SPROUT protocol settings, default values.
 * It is recommended always to use the default values.
 */

/*
 * If no updates are seen for VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT
 * seconds, then protocol is considered down on that stack port.
 */
#define VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT  13

/*
 * Seconds between periodic SPROUT update transmission for slave switches
 */
#define VTSS_SPROUT_UPDATE_INTERVAL_SLV_DEFAULT  4

/*
 * Seconds between periodic SPROUT update transmission for master switches
 */
#define VTSS_SPROUT_UPDATE_INTERVAL_MST_DEFAULT  3

/*
 * Maximum number of SPROUT Updates transmitted per second.
 */
#define VTSS_SPROUT_UPDATE_LIMIT_DEFAULT    10


/*
 * Maximum number of SPROUT Alerts transmitted per second.
 */
#define VTSS_SPROUT_ALERT_LIMIT_DEFAULT 2

/*
 * Master election priority
 * Smaller priority => Higher probability of becoming master
 */
#define VTSS_SPROUT_MST_ELECT_PRIO_DEFAULT 3 /* 2nd lowest probability */

/* Minimum value of master time to be considered */
#define VTSS_SPROUT_MST_TIME_MIN 30 /* seconds */


/* Period for which a mst_time ignore should be set */
#define VTSS_SPROUT_MST_TIME_IGNORE_PERIOD VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT


/* Various SPROUT parameters, set through vtss_sprout_parm_set() */
typedef enum {
    VTSS_SPROUT_PARM_MST_ELECT_PRIO,
    VTSS_SPROUT_PARM_MST_TIME_IGNORE,

    VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_SLV,
    VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_MST,
    VTSS_SPROUT_PARM_SPROUT_UPDATE_AGE_TIME,
    VTSS_SPROUT_PARM_SPROUT_UPDATE_LIMIT,
} vtss_sprout_parm_t;


/*
 * Topology
 *
 * VtssTopoBack2Back:  Two units connected with two stacking cables
 * VtssTopoClosedLoop: Ring topology with more than two units.
 * VtssTopoOpenLoop:   Chain topology.
 */
typedef enum {
    VtssTopoBack2Back, // Not used for VTSS_SPROUT_V2
    VtssTopoClosedLoop,
    VtssTopoOpenLoop
}
vtss_sprout_topology_type_t;

/* SPROUT port statistics */
typedef struct _vtss_sprout_stack_port_stat_t {
    /* SPROUT stack port state */
    BOOL proto_up;

    /* SPROUT Update Counters */
    uint sprout_update_rx_cnt;              /* Rx'ed SPROUT Updates, including bad ones */
    uint sprout_update_periodic_tx_cnt;     /* Tx'ed SPROUT Updates, periodic (tx OK)   */
    uint sprout_update_triggered_tx_cnt;    /* Tx'ed SPROUT Updates, triggered (tx OK)  */
    uint sprout_update_tx_policer_drop_cnt; /* Tx-drops by SPROUT Tx-policer            */
    uint sprout_update_rx_err_cnt;          /* Rx'ed errornuous SPROUT Updates          */
    uint sprout_update_tx_err_cnt;          /* Tx of SPROUT Update failed               */

#if defined(VTSS_SPROUT_V2)
    /* SPROUT Alert Counters */
    uint sprout_alert_rx_cnt;              /* Rx'ed SPROUT Alerts, including bad ones */
    uint sprout_alert_tx_cnt;              /* Tx'ed SPROUT Alerts (tx OK) */
    uint sprout_alert_tx_policer_drop_cnt; /* Tx-drops by SPROUT Tx-policer */
    uint sprout_alert_rx_err_cnt;          /* Rx'ed errornuous SPROUT Alerts */
    uint sprout_alert_tx_err_cnt;          /* Tx of SPROUT Alert failed */
#endif
} vtss_sprout_stack_port_stat_t;


/* SPROUT switch statistics */
typedef struct _vtss_sprout_switch_stat_t {
    /* Topology */
    uint                          switch_cnt; /* Number of switches in stack */
    vtss_sprout_topology_type_t   topology_type;

    /* Time of last topology change. Seconds */
    ulong                         topology_change_time;

    vtss_sprout_stack_port_stat_t stack_port[2];
} vtss_sprout_switch_stat_t;


/*
 * Chip information (part of SIT)
 */
typedef struct _vtss_sprout_sit_entry_chip_t {
    // UPSIDs of chip
    vtss_vstax_upsid_t           upsid[2];

    // Front port mask of UPSes of chip
    u64                          ups_port_mask[2];

    // Distance from primary unit to chip via stack port A and B.
    // -1 = Unreachable. 0 = local unit
    vtss_sprout_dist_t           dist[2];

    // Stack port used to reach unit from primary unit.
    // Local unit => 0 (for both primary and secondary unit)
    vtss_port_no_t               shortest_path;

    // This chip holds the global mirror port
    BOOL                         have_mirror;
} vtss_sprout_sit_entry_chip_t;


/*
 * Switch Information Table (SIT)
 */
typedef struct _vtss_sprout_sit_entry_t {
    BOOL                         vld;

    // Switch Information
    // ------------------
    vtss_sprout_switch_addr_t    switch_addr;

    // Master election information
    BOOL                         mst_capable;
    vtss_sprout_mst_elect_prio_t mst_elect_prio;
    ulong                        mst_time;
    BOOL                         mst_time_ignore;

    // Application specific information, exchanged by SPROUT
    vtss_sprout_switch_appl_info_t switch_appl_info;

    // IPv4 address. Host order. 0.0.0.0 = Unknown
    ulong                        ip_addr;

    /* Additional application specific information, not used by SPROUT */
    uint id;

    // Number of chips in switch, 1 or 2
    // SPROUT V1 only supports chip_cnt=1
    uint                         chip_cnt;

#if defined(VTSS_SPROUT_V1)
    uint                         glag_mbr_cnt[VTSS_GLAGS + 1];
#endif

    // Chip Information
    // ----------------
    // [0] is primary chip (chip with CPU active)
    // [1] is secondary chip (if present)
    vtss_sprout_sit_entry_chip_t chip[2];
} vtss_sprout_sit_entry_t;

typedef struct _vtss_sprout_sit_t {
    vtss_sprout_switch_addr_t   mst_switch_addr;
    ulong                       mst_change_time;
    vtss_sprout_topology_type_t topology_type;

    vtss_sprout_sit_entry_t si[VTSS_SPROUT_SIT_SIZE];
} vtss_sprout_sit_t;


/* ######################################################################## */


/* ###########################################################################
 * API Functions
 * ------------------------------------------------------------------------ */

/* ===========================================================================
 * Initialization
 * ------------------------------------------------------------------------ */

/*
 * Initialization of Topo.
 *
 * Handles initialization of data structures, creation of semaphores, etc.
 * Does NOT make any calls to setup hardware (i.e. no calls to switch API).
 * Must be called only once.
 */
vtss_rc vtss_sprout_init(
    const vtss_sprout_init_t *const init);


/*
 * Setup switch, including link state information.
 *
 * Initial setup of switch. SPROUT protocol started.
 * Must be called only once, after vtss_sprout_init() has been called.
 * Upon completion, tokens are posted to semaphores.
 *
 * Any master election priority should be set before calling
 * vtss_sprout_switch_init(). Ref. vtss_sprout_parm_set().
 */
vtss_rc vtss_sprout_switch_init(
    const vtss_sprout_switch_init_t *const setup);

/* ======================================================================== */


/* ===========================================================================
 * Configuration
 *
 * These functions acquire both state_data and tbl_rd semaphores.
 * ------------------------------------------------------------------------ */

#if defined(VTSS_SPROUT_V1)
/*
 * Set number of member ports of a given GLAG.
 * Default is no GLAG members.
 */
vtss_rc vtss_sprout_glag_mbr_cnt_set(
    const vtss_glag_no_t       glag_no,
    const uint                 glag_mbr_cnt);
#endif


/*
 * Set whether mirror port is on local switch.
 * Default is no local mirror port.
 */
vtss_rc vtss_sprout_have_mirror_port_set(
    const vtss_sprout_chip_idx_t chip_idx,
    const BOOL                 have_mirror_port);


/*
 * Set IPv4 address of switch
 */
vtss_rc vtss_sprout_ipv4_addr_set(
    /* IPv4 address. Host order */
    const vtss_ipv4_t ipv4_addr);


/*
 * Set content of switch info TLV (8 bytes)
 */
vtss_rc vtss_sprout_switch_appl_info_set(
    const vtss_sprout_switch_appl_info_t switch_appl_info_val,
    const vtss_sprout_switch_appl_info_t switch_appl_info_mask);


/*
 * Set adm state of stack port
 * Initial state is down.
 */
vtss_rc vtss_sprout_stack_port_adm_state_set(
    const vtss_port_no_t port_no,
    const BOOL           adm_up);


/*
 * Configure various SPROUT parameters
 * init_phase must be set, when function is called before calling
 * vtss_sprout_switch_init(), i.e. during initialization phase.
 */
vtss_rc vtss_sprout_parm_set(
    const BOOL               init_phase,
    const vtss_sprout_parm_t parm,
    const int                val);

/* ======================================================================== */


/* ===========================================================================
 * Link up/down, packet reception & periodic activation
 *
 * These functions acquire both state_data and tbl_rd semaphores.
 * ------------------------------------------------------------------------ */

/*
 * Set link state of stack port.
 */
vtss_rc vtss_sprout_stack_port_link_state_set(
    const vtss_port_no_t port_no,
    const BOOL           link_up);


/*
 * SPROUT PDU received
 *
 * Application must call this function when frames of the following
 * type have been extracted from CPU extraction queue:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      DMAC = 0x0101C1000002                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           DMAC, cont.         |            * (SMAC)           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            * (SMAC)                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             0x8880            |1|             *               | (a)
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               *                               | (a)
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               *                               | (a)
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             0x8880            |           0x0002              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             0x0001            |              *                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | * ...                                                         |
 *
 * Legend
 * ------
 * '*': Any value.
 * (a): These three 32-bit words, must be removed from
 *      the frame prior to calling vtss_sprout_rx_pkt().
 */
vtss_rc vtss_sprout_rx_pkt(
    const vtss_port_no_t  port_no,
    const uchar *const    frame,
    const uint            length);


/* Must be called from application every ~100msec */
vtss_rc vtss_sprout_service_100msec(void);

/* ======================================================================== */


/* ===========================================================================
 * Statistics and state information
 *
 * These functions acquire tbl_rd semaphore.
 * ------------------------------------------------------------------------ */

/*
 * Get copy of switch information table
 *
 * Note that vtss_sprout_sit_t is a fairly large struct and it should thus
 * be considered to malloc it, rather than allocating it on the stack.
 */
void vtss_sprout_sit_get(
    vtss_sprout_sit_t *const sit);

/*
 * Get SPROUT switch statistics
 */
vtss_rc vtss_sprout_stat_get(
    vtss_sprout_switch_stat_t *const stat);


/*
 * Convert 64-bit port mask to string of type "25-48,49,51"
 */
char *vtss_sprout_port_mask_to_str(u64 port_mask);


/* ======================================================================== */


/* ===========================================================================
 * Debug functions
 *
 * No semaphores acquired.
 * ------------------------------------------------------------------------ */

#if !(VTSS_OPSYS_ECOS)
typedef int (*vtss_sprout_dbg_printf_t)(const char *fmt, ...);
#else
typedef int (*vtss_sprout_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));
#endif

void vtss_sprout_dbg(
    vtss_sprout_dbg_printf_t dbg_printf,
    const uint             parms_cnt,
    const ulong *const     parms);

#if defined(VTSS_SPROUT_FW_VER_CHK)
// Function for controlling whether to transmit the fw_ver specified in
//   vtss_sprout_switch_init_t.my_fw_ver
// or a null value.
// By having the "fw_ver_chk" callback function to always accept the null
// value, this can be used to attempt to connect switches with non-identical
// FW versions.
//
// Note though that this is only a debug feature, as non-identical FW versions
// are - by definition - are considered NOT interoperable!
typedef enum {
    VTSS_SPROUT_FW_VER_MODE_NULL,   // Tx null FW version
    VTSS_SPROUT_FW_VER_MODE_NORMAL, // Tx normal FW version
} vtss_sprout_fw_ver_mode_t;

void vtss_sprout_fw_ver_mode_set(
    vtss_sprout_fw_ver_mode_t mode);
#endif


// Commands for vtss_sprout_time_keeper()
typedef enum {
    VTSS_SPROUT_TIME_KEEPER_CMD_PRINT,
    VTSS_SPROUT_TIME_KEEPER_CMD_MEASURE,
    VTSS_SPROUT_TIME_KEEPER_CMD_FLUSH,
    VTSS_SPROUT_TIME_KEEPER_CMD_START,
    VTSS_SPROUT_TIME_KEEPER_CMD_STOP,
} vtss_sprout_time_keeper_cmd_t;

// Time keeper for measuring code (debug purposes)
void vtss_sprout_time_keeper(
    vtss_sprout_time_keeper_cmd_t cmd,
    char                          *txt,
    vtss_sprout_dbg_printf_t      dbg_printf);

/* ======================================================================== */


/* ######################################################################## */

#endif /* _VTSS_SPROUT_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
