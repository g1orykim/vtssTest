/*

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

*/

#ifndef _VTSS_MVR_H_
#define _VTSS_MVR_H_

#include "vtss_common_os.h"
#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"

#define VTSS_TRACE_MODULE_ID            VTSS_MODULE_ID_MVR

/* Parameter Values */
#define MVR_QUERIER_ADDRESS4            IPMC_PARAM_DEF_QUERIER_ADRS4
#define MVR_QUERIER_QUERY_INTERVAL      IPMC_PARAM_DEF_QI
#define MVR_QUERIER_ROBUST_VARIABLE     IPMC_PARAM_DEF_RV
#define MVR_QUERIER_MAX_RESP_TIME       IPMC_PARAM_DEF_QRI
#define MVR_QUERIER_LAST_Q_INTERVAL     IPMC_PARAM_DEF_LLQI
#define MVR_QUERIER_UNSOLICIT_REPORT    IPMC_PARAM_DEF_URI

#define MVR_NO_OF_SUPPORTED_GROUPS      IPMC_LIB_SUPPORTED_MVR_GROUPS


typedef struct {
    BOOL                    state;

    BOOL                    unregistered_flood;
    ipmc_prefix_t           ssm4_fltr;
    ipmc_prefix_t           ssm6_fltr;

    BOOL                    router_port[VTSS_PORT_BF_SIZE];
    int                     router_port_timer[VTSS_PORT_ARRAY_SIZE];

    BOOL                    fast_leave[VTSS_PORT_BF_SIZE];

    ipmc_db_ctrl_hdr_t      *interfaces;
    ipmc_db_ctrl_hdr_t      *groups;
} vtss_mvr_global_t;

typedef struct {
    vtss_vid_t              vid;
    ipmc_intf_vtag_t        vtag;
    u8                      priority;
    ipmc_pkt_src_port_t     src_type;
} vtss_mvr_intf_tx_t;

typedef struct {
    ipmc_intf_entry_t       basic;
    i8                      name[VTSS_IPMC_MVR_NAME_MAX_LEN];

    mvr_intf_mode_t         mode;
    ipmc_intf_vtag_t        vtag;
    u8                      priority;

    u32                     profile_index;

    mvr_port_role_t         ports[VTSS_PORT_ARRAY_SIZE];
} vtss_mvr_interface_t;

/*
    Functions provided by the vtss_mvr application module.
*/

/*
    vtss_mvr_init - Initialize internal database.
*/
void vtss_mvr_init(void);

/*
    vtss_mvr_purge - Purge internal database.
*/
vtss_rc vtss_mvr_purge(vtss_isid_t isid, BOOL mode, BOOL grp_adr_only);

#ifndef VTSS_SW_OPTION_IPMC
/*
    vtss_mvr_upd_unknown_fwdmsk - Update unknown flooding mask.
*/
void vtss_mvr_upd_unknown_fwdmsk(void);
#endif /* VTSS_SW_OPTION_IPMC */

/*
    vtss_mvr_tick_xxx
    Maintain timer-driven-event of vtss_mvr application module
*/
void vtss_mvr_tick_gen(void);
void vtss_mvr_tick_intf_tmr(ipmc_intf_entry_t *intf);
void vtss_mvr_tick_intf_rxmt(void);
void vtss_mvr_tick_group_tmr(void);

void vtss_mvr_set_mode(BOOL mode);
void vtss_mvr_set_unreg_flood(BOOL enabled);
void vtss_mvr_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *prefix);
void vtss_mvr_set_router_ports(u32 idx, BOOL status);
void vtss_mvr_clear_stat_counter(vtss_vid_t vid);
/*
    vtss_mvr_global_set - Set global settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_global_set(vtss_mvr_global_t *global_entry);

/*
    vtss_mvr_global_get - Get global settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_global_get(vtss_mvr_global_t *global_entry);

/*
    vtss_mvr_set_fast_leave_ports - Set fast leave ports of vtss_mvr application module.
*/
void vtss_mvr_set_fast_leave_ports(ipmc_port_bfs_t *port_mask);

/*
    vtss_mvr_get_fast_leave_ports - Get fast leave status of a port in vtss_mvr application module.
*/
BOOL vtss_mvr_get_fast_leave_ports(u32 port);

/*
    vtss_mvr_interface_set - Set interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_set(ipmc_operation_action_t action, vtss_mvr_interface_t *mvr_intf);

/*
    vtss_mvr_interface_get - Get interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_get(vtss_mvr_interface_t *mvr_intf);

/*
    vtss_mvr_interface_get_next - GetNext interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_get_next(vtss_mvr_interface_t *mvr_intf);

BOOL vtss_mvr_intf_group_get(ipmc_group_entry_t *grp);
BOOL vtss_mvr_intf_group_get_next(ipmc_group_entry_t *grp);

void vtss_mvr_port_state_change_handle(vtss_port_no_t port_no, port_info_t *info);
void vtss_mvr_stp_port_state_change_handle(ipmc_ip_version_t version, vtss_port_no_t port_no, vtss_common_stpstate_t new_state);

#ifdef VTSS_SW_OPTION_PACKET
vtss_rc RX_ipmcmvr(ipmc_ip_version_t version,
                   u16 ingress_vid,
                   void *contxt,
                   const u8 *const frame,
                   const vtss_packet_rx_info_t *const rx_info,
                   ipmc_port_bfs_t *ret_fwd,
                   vtss_mvr_intf_tx_t pcp[MVR_NUM_OF_INTF_PER_VERSION]);
#endif /* VTSS_SW_OPTION_PACKET */

void vtss_mvr_process_glag(u32 port, ulong vid, const uchar *const frame, ulong frame_len, ipmc_ip_version_t ipmc_version);
void vtss_mvr_calculate_dst_ports(BOOL compat_bypass,
                                  BOOL inc_rcver,
                                  vtss_vid_t vid,
                                  u8 port_no,
                                  ipmc_port_bfs_t *port_mask,
                                  ipmc_ip_version_t version);

#endif /* _VTSS_MVR_H_ */
