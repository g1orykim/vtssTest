/* Switch API software.

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

 $Id$
 $Revision$
*/

#ifndef VTSS_ICLI_MEP_H
#define VTSS_ICLI_MEP_H

#include "icli_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

void mep_show_mep(i32 session_id, icli_range_t *inst,
                  BOOL has_peer, BOOL has_cc, BOOL has_lm, BOOL has_dm, BOOL has_lt, BOOL has_lb, BOOL has_tst, BOOL has_aps, BOOL has_client, BOOL has_ais, BOOL has_lck, BOOL has_detail);

void mep_clear_mep(i32 session_id, u32 inst,
                  BOOL has_lm, BOOL has_dm, BOOL has_tst);

void mep_mep(i32 session_id, u32 inst,
             BOOL has_mip, BOOL has_up, BOOL has_down, BOOL has_port, BOOL has_evc, BOOL has_vlan, BOOL has_vid, u32 vid, u32 flow, u32 level, icli_switch_port_range_t port);

void mep_no_mep(i32 session_id, u32 inst);

void mep_mep_meg_id(i32 session_id, u32 inst,
                    char *megid, BOOL has_itu, BOOL has_itu_cc, BOOL has_ieee, BOOL has_name, char *name);

void mep_mep_mep_id(i32 session_id, u32 inst,
                    u32 mepid);

void mep_mep_pm(i32 session_id, u32 inst);

void mep_no_mep_pm(i32 session_id, u32 inst);

void mep_mep_level(i32 session_id, u32 inst,
                   u32 level);

void mep_mep_vid(i32 session_id, u32 inst,
                 u32 vid);

void mep_no_mep_vid(i32 session_id, u32 inst);

void mep_mep_voe(i32 session_id, u32 inst);

void mep_no_mep_voe(i32 session_id, u32 inst);

void mep_mep_peer_mep_id(i32 session_id, u32 inst,
                         u32 mepid, BOOL has_mac, vtss_mac_t mac);

void mep_no_mep_peer_mep_id(i32 session_id, u32 inst,
                            u32 mepid, BOOL has_all);

void mep_mep_cc(i32 session_id, u32 inst,
                u32 prio, BOOL has_fr300s, BOOL has_fr100s, BOOL has_fr10s, BOOL has_fr1s, BOOL has_fr6m, BOOL has_fr1m, BOOL has_fr6h);

void mep_no_mep_cc(i32 session_id, u32 inst);

void mep_mep_lm(i32 session_id, u32 inst,
                u32 prio, BOOL has_uni, BOOL has_multi, BOOL has_single, BOOL has_dual, BOOL has_fr10s, BOOL has_fr1s, BOOL has_fr6m, BOOL has_fr1m, BOOL has_fr6h, BOOL has_flr, u32 flr);

void mep_no_mep_lm(i32 session_id, u32 inst);

void mep_mep_dm(i32 session_id, u32 inst,
                u32 prio, BOOL has_uni, BOOL has_multi, u32 mepid, BOOL has_dual, BOOL has_single, BOOL has_rdtrp, BOOL has_flow, u32 interval, u32 lastn);

void mep_no_mep_dm(i32 session_id, u32 inst);

void mep_mep_dm_overflow_reset(i32 session_id, u32 inst);

void mep_no_mep_dm_overflow_reset(i32 session_id, u32 inst);

void mep_mep_dm_ns(i32 session_id, u32 inst);

void mep_no_mep_dm_ns(i32 session_id, u32 inst);

void mep_mep_dm_syncronized(i32 session_id, u32 inst);

void mep_no_mep_dm_syncronized(i32 session_id, u32 inst);

void mep_mep_dm_proprietary(i32 session_id, u32 inst);

void mep_no_mep_dm_proprietary(i32 session_id, u32 inst);

void mep_mep_lt(i32 session_id, u32 inst,
                u32 prio, BOOL has_mep_id, u32 mepid, BOOL has_mac, vtss_mac_t mac, u32 ttl);

void mep_no_mep_lt(i32 session_id, u32 inst);

void mep_mep_lb(i32 session_id, u32 inst,
                u32 prio, BOOL has_dei, BOOL has_multi, BOOL has_uni, BOOL has_mep_id, u32 mepid, BOOL has_mac, vtss_mac_t mac, u32 count, u32 size, u32 interval);

void mep_no_mep_lb(i32 session_id, u32 inst);

void mep_mep_tst(i32 session_id, u32 inst,
                 u32 prio, BOOL has_dei, u32 mepid, BOOL has_sequence, BOOL has_all_zero, BOOL has_all_one, BOOL has_one_zero, u32 rate, u32 size);

void mep_mep_tst_tx(i32 session_id, u32 inst);

void mep_no_mep_tst_tx(i32 session_id, u32 inst);

void mep_mep_tst_rx(i32 session_id, u32 inst);

void mep_no_mep_tst_rx(i32 session_id, u32 inst);

void mep_mep_aps(i32 session_id, u32 inst,
                 u32 prio, BOOL has_multi, BOOL has_uni, BOOL has_laps, BOOL has_raps, BOOL has_octet, u32 octet);

void mep_no_mep_aps(i32 session_id, u32 inst);

void mep_mep_client(i32 session_id, u32 inst,
                    BOOL has_evc, BOOL has_vlan);

void mep_mep_client_flow(i32 session_id, u32 inst,
                         u32 cflow, u32 level, BOOL has_ais_prio, u32 aisprio, BOOL has_ais_highest, BOOL has_lck_prio, u32 lckprio, BOOL has_lck_highest);

void mep_no_mep_client_flow(i32 session_id, u32 inst,
                            u32 cflow, BOOL has_all);

void mep_mep_ais(i32 session_id, u32 inst,
                 BOOL has_fr1s, BOOL has_fr1m, BOOL has_protect);

void mep_no_mep_ais(i32 session_id, u32 inst);

void mep_mep_lck(i32 session_id, u32 inst,
                 BOOL has_fr1s, BOOL has_fr1m);

void mep_no_mep_lck(i32 session_id, u32 inst);

void mep_mep_volatile(i32 session_id, u32 inst);

void mep_no_mep_volatile(i32 session_id, u32 inst);

vtss_rc mep_icfg_init(void);

#endif /* VTSS_ICLI_MEP_H */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
