/*

 Vitesse API software.

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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_MPLS
#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"

#if defined(VTSS_FEATURE_MPLS)

vtss_rc vtss_mpls_tc_conf_get(vtss_inst_t                 inst,
                              vtss_mpls_tc_conf_t * const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *conf = vtss_state->mpls.tc_conf;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_mpls_tc_conf_set(vtss_inst_t                       inst,
                              const vtss_mpls_tc_conf_t * const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.tc_conf_set, conf);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_mpls_l2_segment_attach(vtss_inst_t                   inst,
                                    const vtss_mpls_l2_idx_t      idx,
                                    const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.l2_segment_attach, idx, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_l2_segment_detach(vtss_inst_t                   inst,
                                    const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.l2_segment_detach, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_l2_alloc(vtss_inst_t                inst,
                           vtss_mpls_l2_idx_t * const idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.l2_alloc, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_l2_free(vtss_inst_t              inst,
                          const vtss_mpls_l2_idx_t idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.l2_free, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_l2_get(vtss_inst_t              inst,
                         const vtss_mpls_l2_idx_t idx,
                         vtss_mpls_l2_t * const   l2)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_MPLS_IDX_CHECK_L2(idx);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *l2 = vtss_state->mpls.l2_conf[idx].pub;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_l2_set(vtss_inst_t                  inst,
                         const vtss_mpls_l2_idx_t     idx,
                         const vtss_mpls_l2_t * const l2)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.l2_set, idx, l2);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_alloc(vtss_inst_t                     inst,
                                const BOOL                      in_seg,
                                vtss_mpls_segment_idx_t * const idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_alloc, in_seg, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_free(vtss_inst_t                   inst,
                               const vtss_mpls_segment_idx_t idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_free, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_alloc(vtss_inst_t                inst,
                           const vtss_mpls_xc_type_t  type,
                           vtss_mpls_xc_idx_t * const idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_alloc, type, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_free(vtss_inst_t              inst,
                          const vtss_mpls_xc_idx_t idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_free, idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_get(vtss_inst_t                   inst,
                              const vtss_mpls_segment_idx_t idx,
                              vtss_mpls_segment_t * const   seg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *seg = vtss_state->mpls.segment_conf[idx].pub;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_set(vtss_inst_t                       inst,
                              const vtss_mpls_segment_idx_t     idx,
                              const vtss_mpls_segment_t * const seg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_set, idx, seg);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_state_get(vtss_inst_t                       inst,
                                    const vtss_mpls_segment_idx_t     idx,
                                    vtss_mpls_segment_state_t * const state)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_state_get, idx, state);
    }
    VTSS_EXIT();
    return rc;

}

vtss_rc vtss_mpls_xc_get(vtss_inst_t              inst,
                         const vtss_mpls_xc_idx_t idx,
                         vtss_mpls_xc_t * const   xc)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_MPLS_IDX_CHECK_XC(idx);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *xc = vtss_state->mpls.xc_conf[idx].pub;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_set(vtss_inst_t                  inst,
                         const vtss_mpls_xc_idx_t     idx,
                         const vtss_mpls_xc_t * const xc)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_set, idx, xc);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_segment_attach(vtss_inst_t                   inst,
                                    const vtss_mpls_xc_idx_t      xc_idx,
                                    const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_segment_attach, xc_idx, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_segment_detach(vtss_inst_t                   inst,
                                    const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_segment_detach, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_mc_segment_attach(vtss_inst_t                   inst,
                                       const vtss_mpls_xc_idx_t      xc_idx,
                                       const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_mc_segment_attach, xc_idx, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_xc_mc_segment_detach(vtss_inst_t                   inst,
                                       const vtss_mpls_xc_idx_t      xc_idx,
                                       const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.xc_mc_segment_detach, xc_idx, seg_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_server_attach(vtss_inst_t                   inst,
                                        const vtss_mpls_segment_idx_t idx,
                                        const vtss_mpls_segment_idx_t srv_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_server_attach, idx, srv_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_mpls_segment_server_detach(vtss_inst_t                   inst,
                                        const vtss_mpls_segment_idx_t idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(mpls.segment_server_detach, idx);
    }
    VTSS_EXIT();
    return rc;
}

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_mpls_inst_create(vtss_state_t *vtss_state)
{
    u32 i;

    /* TC mapping and Vprofiles are configured by (chip)_init_conf_set(). */
    
    vtss_state->mpls.vprofile_free_list = VTSS_MPLS_IDX_UNDEFINED;
    
    vtss_mpls_idxchain_init(vtss_state);
    
    for (i = 0; i < VTSS_MPLS_L2_CNT; i++) {
        vtss_state->mpls.l2_conf[i].pub.port  = VTSS_PORT_NO_NONE;
        vtss_state->mpls.l2_conf[i].next_free = i + 1;
        VTSS_MPLS_IDXCHAIN_UNDEF(vtss_state->mpls.l2_conf[i].users);
    }
    vtss_state->mpls.l2_conf[VTSS_MPLS_L2_CNT - 1].next_free = VTSS_MPLS_IDX_UNDEFINED;
    vtss_state->mpls.l2_free_list = 0;
    
    for (i = 0; i < VTSS_MPLS_SEGMENT_CNT; i++) {
        vtss_state->mpls.segment_conf[i].pub.label.value = VTSS_MPLS_LABEL_VALUE_DONTCARE;
        vtss_state->mpls.segment_conf[i].next_free       = i + 1;
    }
    vtss_state->mpls.segment_conf[VTSS_MPLS_SEGMENT_CNT - 1].next_free = VTSS_MPLS_IDX_UNDEFINED;
    vtss_state->mpls.segment_free_list = 0;
    
    for (i = 0; i < VTSS_MPLS_XC_CNT; i++) {
        vtss_state->mpls.xc_conf[i].next_free = i + 1;
    }
    vtss_state->mpls.xc_conf[VTSS_MPLS_XC_CNT - 1].next_free = VTSS_MPLS_IDX_UNDEFINED;
    vtss_state->mpls.xc_free_list = 0;
    
    VTSS_MPLS_IDXCHAIN_UNDEF(vtss_state->mpls.is0_mll_free_chain);
    VTSS_MPLS_IDXCHAIN_UNDEF(vtss_state->mpls.encap_free_chain);

    // Index chains are last-in-first-out, so we insert in reverse order; this doesn't confuse
    // people so much when they're debugging... (We could have done append-to-tail, but that's
    // inefficient.)
    for (i = VTSS_MPLS_IN_ENCAP_CNT; i > 0; i--) {
        (void) vtss_mpls_idxchain_insert_at_head(vtss_state, &vtss_state->mpls.is0_mll_free_chain, i - 1);
    }

    // Out encap; valid values are [1..VTSS_MPLS_OUT_ENCAP_CNT[, as index 0 tells ES0 to not push encap.
    for (i = VTSS_MPLS_OUT_ENCAP_CNT; i > 1; i--) {
        (void) vtss_mpls_idxchain_insert_at_head(vtss_state, &vtss_state->mpls.encap_free_chain, i - 1);
    }
    
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

void vtss_mpls_debug_print(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info)
{
    static const char * const    tagtype_to_str[] = { "Not", "C", "S" };
    static const char * const    xctype_to_str[]  = { "LSR", "LER" };
    static const char * const    tunnelmode_to_str[]  = { "Pipe", "Short-Pipe", "Uniform" };
    vtss_mpls_xc_idx_t           xc_idx;
    vtss_mpls_segment_idx_t      seg_idx;
    vtss_mpls_l2_idx_t           l2_idx;
    vtss_mpls_xc_internal_t      *xc;
    vtss_mpls_segment_internal_t *seg;
    vtss_mpls_l2_internal_t      *l2;
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;
    const char                   *comma;

    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_MPLS)) {

        pr("L2 Entries In Use\n");
        pr("  Idx Uprt Peer MAC          Self MAC          Tag  VID PCP DEI LL-up LL-dn User segments\n");
        for (l2_idx = 0; l2_idx < VTSS_MPLS_L2_CNT; l2_idx++) {
            l2 = &vtss_state->mpls.l2_conf[l2_idx];
            if (VTSS_MPLS_IDX_IS_UNDEF(l2->next_free)  &&  l2->pub.port != VTSS_PORT_NO_NONE) {
                pr("%5d %4d %02x:%02x:%02x:%02x:%02x:%02x %02x:%02x:%02x:%02x:%02x:%02x %3s %4d %3d %3d %5d %5d ",
                   l2_idx, l2->pub.port,
                   l2->pub.peer_mac.addr[0], l2->pub.peer_mac.addr[1],
                   l2->pub.peer_mac.addr[2], l2->pub.peer_mac.addr[3],
                   l2->pub.peer_mac.addr[4], l2->pub.peer_mac.addr[5],
                   l2->pub.self_mac.addr[0], l2->pub.self_mac.addr[1],
                   l2->pub.self_mac.addr[2], l2->pub.self_mac.addr[3],
                   l2->pub.self_mac.addr[4], l2->pub.self_mac.addr[5],
                   tagtype_to_str[l2->pub.tag_type], l2->pub.vid,
                   l2->pub.pcp, l2->pub.dei,
                   l2->ll_idx_upstream, l2->ll_idx_downstream);

                if (! VTSS_MPLS_IDXCHAIN_END(l2->users)) {
                    comma = "";
                    more = vtss_mpls_idxchain_get_first(vtss_state, &l2->users, &iter, &user);
                    while (more) {
                        pr("%s%d", comma, user);
                        comma = ",";
                        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
                    }
                    pr("\n");
                }
                else {
                    pr("-\n");
                }
            }
        }

        pr("\n\nSegments In Use\n");
        pr("  Idx Type E/L L-CoS TC-map         Label Assign  L2  XC Server PW  CntrlWord Alloc MLL MLBS VPr ES0 Encap ELen ESDX Clients\n");
        for (seg_idx = 0; seg_idx < VTSS_MPLS_SEGMENT_CNT; seg_idx++) {
            seg = &vtss_state->mpls.segment_conf[seg_idx];
            if (VTSS_MPLS_IDX_IS_UNDEF(seg->next_free)  &&  seg->pub.label.value != VTSS_MPLS_LABEL_VALUE_DONTCARE) {
                pr("  %3d %4s %3s %5d %3s/%2d %5d/%3d/%3d %6s %3d %3d %6d %2s ",
                   seg_idx,
                   (seg->pub.is_in ? "in" : "out"),
                   (seg->pub.e_lsp ? "E" : "L"),
                   seg->pub.l_lsp_cos,
                   (seg->pub.is_in ? "T2Q" : "Q2T"), seg->pub.tc_qos_map_idx,
                   seg->pub.label.value, seg->pub.label.tc, seg->pub.label.ttl,
                   (seg->pub.upstream ? "upstr" : "downst"),
                   seg->pub.l2_idx, seg->pub.xc_idx, seg->pub.server_idx,
                   (seg->pub.pw_conf.is_pw ? "Y" : "N"));

                if (seg->pub.pw_conf.is_pw && seg->pub.pw_conf.process_cw) {
                    pr("0x%08x ", seg->pub.pw_conf.cw);
                }
                else {
                    pr("%10s ", "-");
                }

                pr("%5s ", seg->need_hw_alloc ? "Y" : "N");
                if (seg->pub.is_in) {
                    pr("%3s %4s %3d "
                        "%3s %5s %4s %4s ",
                       (seg->u.in.has_mll  ? "Y" : "N"),
                       (seg->u.in.has_mlbs ? "Y" : "N"),
                       seg->u.in.vprofile_idx,
                       "-", "-", "-", "-");
                }
                else {
                    pr("%3s %4s %3s "
                       "%3s %5d %4d %4d ",
                       "-", "-", "-",
                       (seg->u.out.has_es0   ? "Y" : "N"),
                       seg->u.out.encap_idx, seg->u.out.encap_bytes,
                       (seg->u.out.esdx ? seg->u.out.esdx->sdx : -1));
                }

                if (! VTSS_MPLS_IDXCHAIN_END(seg->clients)) {
                    comma = "";
                    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
                    while (more) {
                        pr("%s%d", comma, user);
                        comma = ",";
                        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
                    }
                    pr("\n");
                }
                else {
                    pr("-\n");
                }
            }
        }

        pr("\n\nXCs In Use\n");
        pr("  Idx Type  In-seg Out-seg    TC-mode   TTL-mode ISDX MC chain\n");
        for (xc_idx = 0; xc_idx < VTSS_MPLS_XC_CNT; xc_idx++) {
            xc = &vtss_state->mpls.xc_conf[xc_idx];
            if (VTSS_MPLS_IDX_IS_UNDEF(xc->next_free)  &&  xc->pub.in_seg_idx != xc->pub.out_seg_idx) {
                pr("  %3d %4s %7d %7d %10s %10s %4d ",
                   xc_idx,
                   xctype_to_str[xc->pub.type],
                   xc->pub.in_seg_idx, xc->pub.out_seg_idx,
                   tunnelmode_to_str[xc->pub.tc_tunnel_mode],
                   tunnelmode_to_str[xc->pub.ttl_tunnel_mode],
                   (xc->isdx ? xc->isdx->sdx : -1));

                if (! VTSS_MPLS_IDXCHAIN_END(xc->mc_chain)) {
                    comma = "";
                    more = vtss_mpls_idxchain_get_first(vtss_state, &xc->mc_chain, &iter, &user);
                    while (more) {
                        pr("%s%d", comma, user);
                        comma = ",";
                        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
                    }
                    pr("\n");
                }
                else {
                    pr("-\n");
                }
            }
        }
    }

    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_MPLS_OAM)) {
        pr("(Placeholder for MPLS-OAM debug output)\n");
    }
}

#endif /* VTSS_FEATURE_MPLS */
