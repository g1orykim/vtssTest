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

#ifndef _VTSS_LACP_PRIVATE_H_
#define _VTSS_LACP_PRIVATE_H_ 1

#include "vtss_lacp_os.h"

/* Feature definitions */
#undef VTSS_LACP_USE_MARKER                 /* Use the marker protocol during aggr reconfig */

/* General definitions */
#define VTSS_LACP_SHORT_TIMEOUT             ((vtss_lacp_time_interval_t)1)
#define VTSS_LACP_LONG_TIMEOUT              ((vtss_lacp_time_interval_t)0)
#define VTSS_LACP_MAX_TX_IN_SECOND          3
#define VTSS_LACP_COLLECTOR_MAX_DELAY       0 /* What we transmit on the wire */
#define VTSS_LACP_COLLMAXDELAY_MAX          (VTSS_LACP_TICKS_PER_SEC / 2)
typedef unsigned short vtss_lacp_colldelay_t;

/* Timer definitions (43.4.4 in the 802.3ad standard) */
#define VTSS_LACP_FAST_PERIODIC_TIME        ((vtss_lacp_time_interval_t)1)
#define VTSS_LACP_SLOW_PERIODIC_TIME        ((vtss_lacp_time_interval_t)30)
#define VTSS_LACP_SHORT_TIMEOUT_TIME        ((vtss_lacp_time_interval_t)(3 * VTSS_LACP_FAST_PERIODIC_TIME))
#define VTSS_LACP_LONG_TIMEOUT_TIME         ((vtss_lacp_time_interval_t)(3 * VTSS_LACP_SLOW_PERIODIC_TIME))
#define VTSS_LACP_CHURN_DETECTION_TIME      ((vtss_lacp_time_interval_t)60)
#define VTSS_LACP_AGGREGATE_WAIT_TIME       ((vtss_lacp_time_interval_t)2)

/* Port state definitions (43.4.2.2 in the 802.3ad standard) */
typedef unsigned char vtss_lacp_portstate_t;
#define VTSS_LACP_PORTSTATE_LACP_ACTIVITY   ((vtss_lacp_portstate_t)0x01)
#define VTSS_LACP_PORTSTATE_LACP_TIMEOUT    ((vtss_lacp_portstate_t)0x02)
#define VTSS_LACP_PORTSTATE_AGGREGATION     ((vtss_lacp_portstate_t)0x04)
#define VTSS_LACP_PORTSTATE_SYNCHRONIZATION ((vtss_lacp_portstate_t)0x08)
#define VTSS_LACP_PORTSTATE_COLLECTING      ((vtss_lacp_portstate_t)0x10)
#define VTSS_LACP_PORTSTATE_DISTRIBUTING    ((vtss_lacp_portstate_t)0x20)
#define VTSS_LACP_PORTSTATE_DEFAULTED       ((vtss_lacp_portstate_t)0x40)
#define VTSS_LACP_PORTSTATE_EXPIRED         ((vtss_lacp_portstate_t)0x80)

/* Port Variables definitions used by the State Machines (43.4.7 in the 802.3ad standard) */
typedef unsigned short vtss_lacp_sm_t;
#define VTSS_LACP_PORT_BEGIN                ((vtss_lacp_sm_t)0x0001)
#define VTSS_LACP_PORT_LACP_ENABLED         ((vtss_lacp_sm_t)0x0002)
#define VTSS_LACP_PORT_ACTOR_CHURN          ((vtss_lacp_sm_t)0x0004)
#define VTSS_LACP_PORT_PARTNER_CHURN        ((vtss_lacp_sm_t)0x0008)
#define VTSS_LACP_PORT_READY                ((vtss_lacp_sm_t)0x0010)
#define VTSS_LACP_PORT_READY_N              ((vtss_lacp_sm_t)0x0020)
#define VTSS_LACP_PORT_MATCHED              ((vtss_lacp_sm_t)0x0040)
#define VTSS_LACP_PORT_STANDBY              ((vtss_lacp_sm_t)0x0080)
#define VTSS_LACP_PORT_SELECTED             ((vtss_lacp_sm_t)0x0100)
#define VTSS_LACP_PORT_MOVED                ((vtss_lacp_sm_t)0x0200)

/* timers types (43.4.9 in the 802.3ad standard) */
typedef unsigned char vtss_lacp_timer_id_t;
#define VTSS_LACP_TID_CURRENT_WHILE_TIMER   ((vtss_lacp_timer_id_t)0)
#define VTSS_LACP_TID_ACTOR_CHURN_TIMER     ((vtss_lacp_timer_id_t)1)
#define VTSS_LACP_TID_PERIODIC_TIMER        ((vtss_lacp_timer_id_t)2)
#define VTSS_LACP_TID_PARTNER_CHURN_TIMER   ((vtss_lacp_timer_id_t)3)
#define VTSS_LACP_TID_WAIT_WHILE_TIMER      ((vtss_lacp_timer_id_t)4)

typedef unsigned short vtss_lacp_protoport_t;

typedef VTSS_COMMON_BUFMEM_ATTRIB struct {
    vtss_common_octet_t         system_priority[sizeof(vtss_lacp_prio_t)];
    vtss_common_octet_t         system_macaddr[VTSS_COMMON_MACADDR_SIZE];
    vtss_common_octet_t         key[sizeof(vtss_lacp_key_t)];
    vtss_common_octet_t         port_priority[sizeof(vtss_lacp_prio_t)];
    vtss_common_octet_t         port[sizeof(vtss_lacp_protoport_t)];
    vtss_lacp_portstate_t       state;
    vtss_common_octet_t         reserved[3];
} vtss_lacp_info_t;
#define SIZEOF_VTSS_LACP_INFO   18 /* Alignment Alert: sizeof(vtss_lacp_info_t) */

#define VTSS_LACP_SUBTYPE_LACP              ((vtss_common_octet_t)0x01)
#define VTSS_LACP_SUBTYPE_MARK              ((vtss_common_octet_t)0x02)
#define VTSS_LACP_VERSION_NO                ((vtss_common_octet_t)1)
#define VTSS_LACP_TVLTYPE_ACTOR_INFO        ((vtss_common_octet_t)1)
#define VTSS_LACP_TVLLEN_ACTOR_INFO         ((vtss_common_octet_t)20)
#define VTSS_LACP_TVLTYPE_PARTNER_INFO      ((vtss_common_octet_t)2)
#define VTSS_LACP_TVLLEN_PARTNER_INFO       ((vtss_common_octet_t)20)
#define VTSS_LACP_TVLTYPE_COLLECTOR         ((vtss_common_octet_t)3)
#define VTSS_LACP_TVLLEN_COLLECTOR          ((vtss_common_octet_t)16)
#define VTSS_LACP_TVLTYPE_TERMINATOR        ((vtss_common_octet_t)0)
#define VTSS_LACP_TVLLEN_TERMINATOR         ((vtss_common_octet_t)0)
#define VTSS_LACP_TVLTYPE_MARKER_INFO       ((vtss_common_octet_t)1)
#define VTSS_LACP_TVLTYPE_MARKER_RESPONS_INFO ((vtss_common_octet_t)2)
#define VTSS_LACP_TVLLEN_MARKER_INFO        ((vtss_common_octet_t)16)

typedef VTSS_COMMON_BUFMEM_ATTRIB struct {
    vtss_common_octet_t         dst_mac[VTSS_COMMON_MACADDR_SIZE];
    vtss_common_octet_t         src_mac[VTSS_COMMON_MACADDR_SIZE];
    vtss_common_octet_t         eth_type[sizeof(vtss_common_ethtype_t)];
    vtss_common_octet_t         subtype;
    vtss_common_octet_t         version;
} vtss_lacp_frame_header_t;
#define SIZEOF_VTSS_LACP_FRAME_HEADER   16 /* Alignment alert: sizeof(vtss_lacp_frame_header_t) */

typedef VTSS_COMMON_BUFMEM_ATTRIB struct {
    vtss_common_octet_t         frame_header[SIZEOF_VTSS_LACP_FRAME_HEADER];
    vtss_common_octet_t         tvl_type_actor;
    vtss_common_octet_t         tvl_length_actor;
    vtss_common_octet_t         actor_info[SIZEOF_VTSS_LACP_INFO];
    vtss_common_octet_t         tvl_type_partner;
    vtss_common_octet_t         tvl_length_partner;
    vtss_common_octet_t         partner_info[SIZEOF_VTSS_LACP_INFO];
    vtss_common_octet_t         tvl_type_collector;
    vtss_common_octet_t         tvl_length_collector;
    vtss_common_octet_t         collector_max_delay[sizeof(vtss_lacp_colldelay_t)];
    vtss_common_octet_t         reserved[12];
    vtss_common_octet_t         tvl_type_terminator;
    vtss_common_octet_t         tvl_length_terminator;
    vtss_common_octet_t         reserved2[50];
} vtss_lacp_lacpdu_t;
#define SIZEOF_VTSS_LACP_LACPDU 124 /* Alignment Alert: sizeof(vtss_lacp_lacpdu_t) */

typedef VTSS_COMMON_BUFMEM_ATTRIB struct {
    vtss_common_octet_t         frame_header[SIZEOF_VTSS_LACP_FRAME_HEADER];
    vtss_common_octet_t         tvl_type_marker_info;
    vtss_common_octet_t         tvl_length_marker_info;
    vtss_common_octet_t         requester_port[sizeof(vtss_common_port_t)];
    vtss_common_octet_t         requester_system_macaddr[VTSS_COMMON_MACADDR_SIZE];
    vtss_common_octet_t         requester_transaction_id[4];
    vtss_common_octet_t         reserved[2];
    vtss_common_octet_t         tvl_type_terminator;
    vtss_common_octet_t         tvl_length_terminator;
    vtss_common_octet_t         reserved2[90];
} vtss_lacp_markerpdu_t;
#define SIZEOF_VTSS_LACP_MARKERPDU 124 /* Alignment Alert: sizeof(vtss_lacp_markerpdu_t) */

typedef unsigned short vtss_lacp_rx_state_t;
#define VTSS_LACP_RXSTATE_INITIALIZE        ((vtss_lacp_rx_state_t)1)
#define VTSS_LACP_RXSTATE_PORT_DISABLED     ((vtss_lacp_rx_state_t)2)
#define VTSS_LACP_RXSTATE_LACP_DISABLED     ((vtss_lacp_rx_state_t)3)
#define VTSS_LACP_RXSTATE_EXPIRED           ((vtss_lacp_rx_state_t)4)
#define VTSS_LACP_RXSTATE_DEFAULTED         ((vtss_lacp_rx_state_t)5)
#define VTSS_LACP_RXSTATE_CURRENT           ((vtss_lacp_rx_state_t)6)

typedef unsigned short vtss_lacp_tx_state_t;
#define VTSS_LACP_TXSTATE_TRANSMIT          ((vtss_lacp_rx_state_t)1)

typedef unsigned short vtss_lacp_mux_state_t;
#define VTSS_LACP_MUXSTATE_DETACHED         ((vtss_lacp_rx_state_t)1)
#define VTSS_LACP_MUXSTATE_WAITING          ((vtss_lacp_rx_state_t)2)
#define VTSS_LACP_MUXSTATE_ATTACHED         ((vtss_lacp_rx_state_t)3)
#define VTSS_LACP_MUXSTATE_COLLDIST         ((vtss_lacp_rx_state_t)4)

typedef unsigned short vtss_lacp_periodic_state_t;
#define VTSS_LACP_PERIODICSTATE_NONE        ((vtss_lacp_rx_state_t)1)
#define VTSS_LACP_PERIODICSTATE_FAST        ((vtss_lacp_rx_state_t)2)
#define VTSS_LACP_PERIODICSTATE_SLOW        ((vtss_lacp_rx_state_t)3)
#define VTSS_LACP_PERIODICSTATE_TX          ((vtss_lacp_rx_state_t)4)

typedef unsigned short vtss_lacp_churn_state_t;
#define VTSS_LACP_CHURNSTATE_NONE           ((vtss_lacp_churn_state_t)1)
#define VTSS_LACP_CHURNSTATE_MONITOR        ((vtss_lacp_churn_state_t)2)
#define VTSS_LACP_CHURNSTATE_CHURN          ((vtss_lacp_churn_state_t)3)

#if defined(HP_PROCURVE_HW)
typedef unsigned char vtss_lacp_tcount_t; /* tick counter */
#else
//typedef unsigned char vtss_lacp_tcount_t; /* tick counter */
typedef unsigned int vtss_lacp_tcount_t; /* tick counter */
#endif

/* Variables associated with each Port (43.4.7 in the 802.3ad standard) */
typedef VTSS_COMMON_DATA_ATTRIB struct vtss_lacp_port_vars {
    vtss_lacp_port_config_t     port_config;
    vtss_common_port_t          actor_port_number;
    /* vtss_lacp_prio_t         actor_port_priority; -- port_config.port_prio */
    vtss_common_bool_t          ntt;
    /* vtss_lacp_key_t          actor_admin_port_key; -- port_config.port_key */
    vtss_lacp_key_t             actor_oper_port_key;
    /* vtss_lacp_portstate_t    actor_admin_port_state; -- portconfig.xmit_mode & port_config.active_or_passive */
    vtss_lacp_portstate_t       actor_oper_port_state;
    vtss_common_macaddr_t       partner_admin_system;
    vtss_common_macaddr_t       partner_oper_system;
    vtss_lacp_prio_t            partner_admin_system_priority;
    vtss_lacp_prio_t            partner_oper_system_priority;
    vtss_lacp_key_t             partner_admin_key;
    vtss_lacp_key_t             partner_oper_key;
    vtss_common_port_t          partner_admin_port_number;
    vtss_common_port_t          partner_oper_port_number;
    vtss_lacp_prio_t            partner_admin_port_priority;
    vtss_lacp_prio_t            partner_oper_port_priority;
    vtss_lacp_portstate_t       partner_admin_port_state;
    vtss_lacp_portstate_t       partner_oper_port_state;
    vtss_common_linkstate_t     port_up;
    /* Private fields not covered in standard */
    struct vtss_lacp_aggregator_vars *aggregator; /* Belongs to this aggregator */
    struct vtss_lacp_aggregator_vars *hw_aggregator; /* Last HW aggregator used */
    struct vtss_lacp_port_vars  *next_lag_port;
    vtss_common_macaddr_t       port_macaddr;
    vtss_lacp_colldelay_t       collmaxdelay;
    vtss_lacp_sm_t              sm_vars;
    vtss_lacp_rx_state_t        sm_rx_state;
    vtss_lacp_tx_state_t        sm_tx_state;
    vtss_lacp_mux_state_t       sm_mux_state;
    vtss_lacp_periodic_state_t  sm_periodic_state;
    vtss_lacp_churn_state_t     sm_partner_churn_state;
    vtss_lacp_tcount_t          sm_rx_timer_counter;
    vtss_lacp_tcount_t          sm_tx_timer_counter;
    vtss_lacp_tcount_t          sm_mux_timer_counter;
    vtss_lacp_tcount_t          sm_periodic_timer_counter;
    vtss_lacp_tcount_t          sm_pc_timer_counter;
#ifdef VTSS_LACP_USE_MARKER
    vtss_lacp_tcount_t          mark_reply_timer_counter;
    vtss_common_octet_t         marker_transid;
#endif /* VTSS_LACP_USE_MARKER */
    /* vtss_common_bool_t       lacp_enabled; -- port_config.enable_lacp */
    vtss_common_duplex_t        duplex_mode;
    vtss_common_linkspeed_t     port_speed;
    vtss_lacp_port_statistcs_t  stats;
} vtss_lacp_port_vars_t;

/* Variables associated with each Aggregator (43.4.6 in the 802.3ad standard) */
typedef VTSS_COMMON_PTR_ATTRIB struct vtss_lacp_aggregator_vars {
    vtss_lacp_agid_t            aggregator_identifier;
    vtss_lacp_key_t             actor_admin_aggregator_key;
    vtss_lacp_key_t             actor_oper_aggregator_key;
    vtss_common_macaddr_t       partner_system;
    vtss_lacp_prio_t            partner_system_priority;
    vtss_lacp_key_t             partner_oper_aggregator_key;
    vtss_lacp_port_vars_t       *lag_ports; /* linked list */
    /* Private fields not covered in standard */
    vtss_common_octet_t         num_of_ports;
    vtss_common_bool_t          is_individual;
    vtss_lacp_time_interval_t   last_change;
    vtss_common_linkspeed_t     actor_port_speed;
} vtss_lacp_aggregator_vars_t;

/* Variables associated with the System (43.4.5 in the 802.3ad standard) */
typedef VTSS_COMMON_PTR_ATTRIB struct {
    vtss_common_bool_t          initialized;
    vtss_lacp_system_config_t   system_config;
    /* Private fields not covered in standard */
    vtss_lacp_aggregator_vars_t aggregators[VTSS_LACP_MAX_AGGR];
    vtss_lacp_port_vars_t       ports[VTSS_LACP_MAX_PORTS];
    vtss_lacp_time_interval_t   ticks_since_start;
} vtss_lacp_system_vars_t;

extern vtss_lacp_system_vars_t vtss_lacp_vars; /* The *only* global variable */
#define LACP    (&vtss_lacp_vars) /* Defined as pointer to global area */

#endif /* _VTSS_LACP_PRIVATE_H_ */
