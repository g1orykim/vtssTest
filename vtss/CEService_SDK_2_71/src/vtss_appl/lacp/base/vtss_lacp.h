/*

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

/*
 * This file contains the API definitions used between the LACP protocol
 * module and the operating environment.
 */
#ifndef _VTSS_LACP_H_
#define _VTSS_LACP_H_ 1

typedef unsigned short vtss_lacp_agid_t; /* Aggregator ids counted from 1 VTSS_LACP_MAX_AGGR */

typedef unsigned long vtss_lacp_time_interval_t; /* Time intervals in 1 sec */
#if defined(HP_PROCURVE_HW)
#define VTSS_LACP_TICKS_PER_SEC         (1)
#else
#define VTSS_LACP_TICKS_PER_SEC         (10)
#endif


typedef unsigned short vtss_lacp_key_t; /* Port key */

typedef unsigned char vtss_lacp_fs_mode_t; /* Fast or slow transmit mode */
#define VTSS_LACP_FSMODE_SLOW           ((vtss_lacp_fs_mode_t)0)
#define VTSS_LACP_FSMODE_FAST           ((vtss_lacp_fs_mode_t)1)
#define VTSS_LACP_DEFAULT_FSMODE        VTSS_LACP_FSMODE_FAST

typedef unsigned char vtss_lacp_activity_mode_t; /* LACP activity mode */
#define VTSS_LACP_ACTMODE_PASSIVE       ((vtss_lacp_activity_mode_t)0)
#define VTSS_LACP_ACTMODE_ACTIVE        ((vtss_lacp_activity_mode_t)1)
#define VTSS_LACP_DEFAULT_ACTIVITY_MODE VTSS_LACP_ACTMODE_ACTIVE

typedef unsigned short vtss_lacp_prio_t; /* Priority as defined by LACP */
#define VTSS_LACP_DEFAULT_SYSTEMPRIO    ((vtss_lacp_prio_t)0x8000)
#define VTSS_LACP_DEFAULT_PORTPRIO      ((vtss_lacp_prio_t)0x8000)

#include "vtss_lacp_os.h"

#if defined(AGGR_MGMT_LAG_PORTS_MAX)
#define VTSS_LACP_MAX_PORTS_IN_AGGR     (AGGR_MGMT_LAG_PORTS_MAX) /* Max number of ports in a Aggregation group */
#else
#define VTSS_LACP_MAX_PORTS_IN_AGGR     (VTSS_LACP_MAX_PORTS)   /* No limit */
#endif

#define VTSS_LACP_MAX_AGGR              (VTSS_LACP_MAX_PORTS) /* Max number of Aggregation groups */
#define VTSS_LACP_MAX_FRAME_SIZE        (124) /* Max frame size recv/xmit (w/o CRC) */
#ifndef VTSS_PORT_NO_START
#define VTSS_PORT_NO_START              (1) /* To support all sw builds  */
#endif

typedef struct {
    vtss_common_counter_t lacp_frame_xmits; /* LACP frames transmitted */
    vtss_common_counter_t lacp_frame_recvs; /* LACP frames received */
    vtss_common_counter_t markreq_frame_xmits; /* MARKER frames transmitted */
    vtss_common_counter_t markreq_frame_recvs; /* MARKER frames received */
    vtss_common_counter_t markresp_frame_xmits; /* MARKER respnse frames transmitted */
    vtss_common_counter_t markresp_frame_recvs; /* MARKER respnse frames received */
    vtss_common_counter_t unknown_frame_recvs; /* Unknown frames received and discarded in error */
    vtss_common_counter_t illegal_frame_recvs; /* Illegal frames received and discarded in error */
} vtss_lacp_port_statistcs_t;

typedef enum {
    LACP_PORT_NOT_ACTIVE,
    LACP_PORT_ACTIVE,
    LACP_PORT_STANDBY,
} vtss_lacp_port_state_t;

/* Status information - used by cli/web/snmp */
typedef VTSS_COMMON_DATA_ATTRIB struct {
    vtss_lacp_agid_t aggrid;
    vtss_common_macaddr_t partner_oper_system;
    vtss_lacp_prio_t partner_oper_system_priority;
    vtss_lacp_key_t partner_oper_key;
    vtss_lacp_time_interval_t secs_since_last_change;
    vtss_common_bool_t port_list[VTSS_LACP_MAX_PORTS]; /* True for each port on this aggr */
} vtss_lacp_aggregatorstatus_t;

typedef VTSS_COMMON_DATA_ATTRIB struct {
    vtss_common_port_t port_number;
    vtss_common_bool_t port_enabled;
    vtss_common_stpstate_t port_forwarding;
    vtss_lacp_key_t actor_oper_port_key;
    vtss_lacp_key_t actor_admin_port_key;
    vtss_lacp_agid_t actor_port_aggregator_identifier;
    vtss_lacp_prio_t partner_oper_port_priority;
    vtss_common_port_t partner_oper_port_number;
    vtss_lacp_port_statistcs_t port_stats;
    vtss_lacp_port_state_t port_state;
} vtss_lacp_portstatus_t;

/* ------------------------------------------------------------------------------------------- */

/*
 * Functions provided by the vtss_lacp protocol module.
 */

/**
 * vtss_lacp_init - Initialize internal data, obtain HW MAC adresses and start the LACP protocol.
 */
extern void vtss_lacp_init(void);

/**
 * vtss_lacp_deinit - Close down LACP.
 */
extern void vtss_lacp_deinit(void);

/* Global parameters */
typedef struct {
    vtss_lacp_prio_t system_prio;
    vtss_common_macaddr_t system_id;
} vtss_lacp_system_config_t;

/**
 * vtss_lacp_set_config - Set global parameters.
 * Note: Can be called before vtss_lacp_init().
 */
extern void vtss_lacp_set_config(const vtss_lacp_system_config_t *system_config);

/**
 * vtss_lacp_get_config - Get global parameters.
 * Note: Can be called before vtss_lacp_init().
 */
extern void vtss_lacp_get_config(vtss_lacp_system_config_t *system_config);

/* Port parameters */
typedef struct {
    vtss_lacp_prio_t port_prio;
    vtss_lacp_key_t port_key;
    vtss_common_bool_t enable_lacp;
    vtss_lacp_fs_mode_t xmit_mode;
    vtss_lacp_activity_mode_t active_or_passive;
} vtss_lacp_port_config_t;

/**
 * vtss_lacp_set_portconfig - Set port-specific parameters.
 * Note: Can be called before vtss_lacp_init().
 */
extern void vtss_lacp_set_portconfig(vtss_common_port_t portno,
                                     const vtss_lacp_port_config_t *port_config);

/**
 * vtss_lacp_get_portconfig - Get port-specific parameters.
 * Note: Can be called before vtss_lacp_init().
 */
extern void vtss_lacp_get_portconfig(vtss_common_port_t portno,
                                     vtss_lacp_port_config_t *port_config);

/**
 * vtss_lacp_get_aggr_status - Get status for a specific aggregation.
 * Note: Can be called before vtss_lacp_init().
 */
extern vtss_common_bool_t vtss_lacp_get_aggr_status(vtss_lacp_agid_t aggrid, vtss_lacp_aggregatorstatus_t *aggr_status);

/**
 * vtss_lacp_get_port_status - Get status for a specific port.
 * Note: Can be called before vtss_lacp_init().
 */
extern void vtss_lacp_get_port_status(vtss_common_port_t portno, vtss_lacp_portstatus_t *port_status);

#if !defined(HP_PROCURVE_HW)
/* Clear the statistics  */
extern void vtss_lacp_clear_statistics(vtss_common_port_t port);
#endif

/* ------------------------------------------------------------------------------------------- */

/*
 * Event callback functions provided by the vtss_lacp protocol module.
 */

/**
 * vtss_lacp_linkstate_changed() - OS indicates to LACP that a port link state changed.
 * Must be called from OS when link status changes.
 */
extern void vtss_lacp_linkstate_changed(vtss_common_port_t portno, vtss_common_linkstate_t new_state);

/**
 * vtss_lacp_tick() - Timer tick event.
 * Must be called from OS every 1/10th of a second.
 */
extern void vtss_lacp_tick(void);

/**
 * vtss_lacp_more_work() - Called when idle.
 * Must be called to perform remaining tasks.
 */
extern void vtss_lacp_more_work(void);

/**
 * vtss_lacp_receive - Deliver a frame from the OS to the LACP protocol.
 * Must be called from OS when a frame with destination
 * MAC address VTSS_LACP_MULTICAST_MACADDR and ethertype
 * of VTSS_LACP_ETHTYPE is received
 */
extern void vtss_lacp_receive(vtss_common_port_t from_port,
                              const vtss_common_octet_t VTSS_COMMON_BUFMEM_ATTRIB *frame,
                              vtss_common_framelen_t len);

/* Addresses used on LACP frames */
#define VTSS_LACP_MULTICAST_MACADDR     { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x02 }
#define VTSS_LACP_ETHTYPE               (0x8809)

/* ------------------------------------------------------------------------------------------- */

/*
 * Functions to be provided by OS specifically for the vtss_lacp module.
 * There are other functions defined in vtss_common_os.h
 */

/**
 * vtss_os_make_key - Generate a key for a port that reflects it's relationship
 */
extern vtss_lacp_key_t vtss_os_make_key(vtss_common_port_t portno, vtss_lacp_key_t new_key);

/**
 * vtss_os_set_hwaggr - Added a specific port to a aggregation group.
 */
extern void vtss_os_set_hwaggr(vtss_lacp_agid_t aid,
                               vtss_common_port_t new_port);

/**
 * vtss_os_clear_hwaggr - Remove a specific port from a aggregation group.
 */
extern void vtss_os_clear_hwaggr(vtss_lacp_agid_t aid,
                                 vtss_common_port_t old_port);

/**
 * vtss_os_translate_port - Swap the advertised port id with internal port id (to/from core).
 */
extern vtss_common_port_t  vtss_os_translate_port(vtss_common_port_t l2port, BOOL from_core);

/* ------------------------------------------------------------------------------------------- */

#ifndef VTSS_LACP_NDEBUG
extern void vtss_lacp_dump(void);
#endif /* !VTSS_LACP_NDEBUG */

#endif /* _VTSS_LACP_H_ */
