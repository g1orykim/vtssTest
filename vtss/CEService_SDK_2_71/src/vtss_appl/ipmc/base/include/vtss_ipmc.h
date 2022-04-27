/*

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

#ifndef _VTSS_IPMC_H_
#define _VTSS_IPMC_H_

#include "vtss_common_os.h"
#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"

/* Parameter Values */
#define IPMC_QUERIER_ADDRESS4               IPMC_PARAM_DEF_QUERIER_ADRS4
#define IPMC_QUERIER_QUERY_INTERVAL         IPMC_PARAM_DEF_QI
#define IPMC_QUERIER_ROBUST_VARIABLE        IPMC_PARAM_DEF_RV
#define IPMC_QUERIER_MAX_RESP_TIME          IPMC_PARAM_DEF_QRI
#define IPMC_QUERIER_LAST_Q_INTERVAL        IPMC_PARAM_DEF_LLQI
#define IPMC_QUERIER_UNSOLICIT_REPORT       IPMC_PARAM_DEF_URI

typedef struct {
    ipmc_ip_version_t   ipmc_version;
    vtss_vid_t          vid;
    vtss_ipv6_t         group_address;

    BOOL                leave;
    ipmc_compat_mode_t  compat;
} ipmc_proxy_report_entry_t;


/*
 * Functions provided by the vtss_ipmc protocol module.
 */

/**
 * vtss_ipmc_init - Initialize internal data.
 */
void vtss_ipmc_init (void);

/*
    vtss_ipmc_upd_unknown_fwdmsk - Update unknown flooding mask.
*/
void vtss_ipmc_upd_unknown_fwdmsk(ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_mode - Set mode of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_mode(BOOL mode, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_leave_proxy - Set the leave proxy status of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_leave_proxy(BOOL mode, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_proxy - Set the proxy status of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_proxy(BOOL mode, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_ssm_range - Set the SSM range of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *prefix);

/**
 * vtss_ipmc_set_unreg_flood - Set flooding flag of un-registered multicast traffic.
 */
void vtss_ipmc_set_unreg_flood(BOOL enabled, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_static_router_ports - Set static router ports of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_static_router_ports(ipmc_port_bfs_t *port_mask, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_static_fast_leave_ports - Set static fast leave ports of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_static_fast_leave_ports(ipmc_port_bfs_t *port_mask, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_get_static_fast_leave_ports - Get static fast leave of a port in vtss_ipmc protocol module.
 */
BOOL vtss_ipmc_get_static_fast_leave_ports(u32 port, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_static_port_throttling_max_no - Set static group throttling max number for specific port of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_static_port_throttling_max_no(ipmc_port_throttling_t *ipmc_port_throttling, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_set_static_port_group_filtering - Set static port multicast group filtering for specific port of vtss_ipmc protocol module.
 */
void vtss_ipmc_set_static_port_group_filtering(ipmc_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version);

/* vtss_ipmc_tick_xxx - maintain timer-driven-event of vtss_ipmc protocol module */
void vtss_ipmc_tick_gen(void);
void vtss_ipmc_tick_intf_tmr(u32 i);
void vtss_ipmc_tick_intf_rxmt(void);
void vtss_ipmc_tick_group_tmr(void);
void vtss_ipmc_tick_proxy(BOOL local_service);

/**
 * vtss_ipmc_get_intf_entry - get vlan entry in vtss_ipmc protocol module.
 */
ipmc_intf_entry_t *vtss_ipmc_get_intf_entry(vtss_vid_t vid, ipmc_ip_version_t version);

/**
 * vtss_ipmc_set_intf_entry - add/update vlan entry to vtss_ipmc protocol module.
 */
ipmc_intf_entry_t *vtss_ipmc_set_intf_entry(vtss_vid_t vid, BOOL state, BOOL querier, ipmc_port_bfs_t *vlan_ports, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_upd_intf_entry - set interface parameters to vtss_ipmc protocol module.
 */
BOOL vtss_ipmc_upd_intf_entry(ipmc_prot_intf_basic_t *intf_entry, ipmc_ip_version_t ipmc_version);

/**
 * vtss_ipmc_del_intf_entry - delete vlan entry in vtss_ipmc protocol module.
 */
void vtss_ipmc_del_intf_entry(vtss_vid_t vid, ipmc_ip_version_t ipmc_version);

ipmc_intf_entry_t *vtss_ipmc_get_next_intf_entry(vtss_vid_t vid, ipmc_ip_version_t version);

void vtss_ipmc_clear_stat_counter(ipmc_ip_version_t ipmc_version, vtss_vid_t vid);

void vtss_ipmc_port_state_change_handle(vtss_port_no_t port_no, port_info_t *info);

BOOL vtss_ipmc_get_intf_group_entry(vtss_vid_t vid, ipmc_group_entry_t *grp, ipmc_ip_version_t version);
BOOL vtss_ipmc_get_next_intf_group_entry(vtss_vid_t vid, ipmc_group_entry_t *grp, ipmc_ip_version_t version);

void vtss_ipmc_stp_port_state_change_handle(ipmc_ip_version_t version, vtss_port_no_t port_no, vtss_common_stpstate_t new_state);

#ifdef VTSS_SW_OPTION_PACKET
vtss_rc RX_ipmcsnp(ipmc_ip_version_t version, void *contxt, const u8 *const frame, const vtss_packet_rx_info_t *const rx_info, ipmc_port_bfs_t *ret_fwd);
#endif /* VTSS_SW_OPTION_PACKET */

void vtss_ipmc_process_glag(ulong port, vtss_vid_t vid, const uchar *const frame, ulong frame_len, ipmc_ip_version_t ipmc_version);
void vtss_ipmc_calculate_dst_ports(vtss_vid_t vid, u8 port_no, ipmc_port_bfs_t *port_mask, ipmc_ip_version_t version);

BOOL vtss_ipmc_debug_pkt_tx(ipmc_intf_entry_t *entry, ipmc_ctrl_pkt_t type, vtss_ipv6_t *dst, u8 idx, BOOL untag);

#endif /* _VTSS_IPMC_H_ */
