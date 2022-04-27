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

#ifndef _VTSS_GVRP_H_
#define _VTSS_GVRP_H_


#ifdef VTSS_SW_OPTION_GVRP

#include "vtss_garp.h"
#include <vtss_xxrp_api.h>
#include <vlan_api.h>

#define GVRP_CRIT_ENTER() vtss_mrp_crit_enter(VTSS_GARP_APPL_GVRP)
#define GVRP_CRIT_EXIT()  vtss_mrp_crit_exit(VTSS_GARP_APPL_GVRP)

extern vtss_rc  vtss_gvrp_construct(int max_participants, int max_vlans);
extern void vtss_gvrp_destruct(BOOL set_default);

extern int vtss_gvrp_rx_pdu(int port_no, const u8 *pdu, int len);

extern vtss_rc vtss_gvrp_gid(int port_no, vtss_vid_t vid, struct garp_gid_instance **gid);
extern void    vtss_gvrp_gid_done(struct garp_gid_instance **gid);

extern vtss_rc vtss_gvrp_participant(int port_no, struct garp_participant **p);

//extern u32 vtss_GVRP_txPDU_timeout(void);
extern u32 vtss_gvrp_txPDU_timeout(struct garp_gid_instance* gid);
extern int vtss_gvrp_leave_timeout(struct garp_gid_instance* gid);
extern u32 vtss_gvrp_leaveall_timeout(int port_no);

// Management function for Join, Leave, setting timer values and administrative control for registrar.
extern vtss_rc vtss_gvrp_join_request(int port_no, vtss_vid_t vid);
extern vtss_rc vtss_gvrp_leave_request(int port_no, vtss_vid_t vid);
extern vtss_rc vtss_gvrp_set_timer(enum timer_context tc, u32 T);
extern u32     vtss_gvrp_get_timer(enum timer_context tc);
extern vtss_rc vtss_gvrp_registrar_administrative_control(int port_no, vtss_vid_t vid, vlan_registration_type_t S);

// Enabling of ports
extern vtss_rc vtss_gvrp_port_control_conf_get(u32 port_no, BOOL *const status);
extern vtss_rc vtss_gvrp_port_control_conf_set(u32 port_no, BOOL enable);

extern void vtss_gvrp_update_vlan_to_msti_mapping(void);
extern vtss_rc vtss_gvrp_mstp_port_state_change_handler(u32 port_no, u8 msti, vtss_mrp_mstp_port_state_change_type_t  port_state_type);

extern u32 vtss_gvrp_global_control_conf_create(vtss_mrp_t **mrp_app);

extern uint vtss_gvrp_timer_tick(uint);

extern int vtss_gvrp_is_enabled(void);
extern int vtss_gvrp_max_vlans(void);
extern void vtss_gvrp_max_vlans_set(int m);
extern int vtss_gvrp_gip_context(struct garp_gid_instance *gid);

extern void vtss_gvrp_internal_statistic(void);

#endif

#endif
