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

 $Id: vtss_sprout_xit.c,v 1.38 2010/11/25 11:42:04 toe Exp $
 $Revision: 1.38 $


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/







#include "vtss_sprout.h"










static void rit_shift_up(
    vtss_sprout__rit_t  *rit_p,
    uint        ri_idx)
{
    uint i = 0;

    VTSS_SPROUT_ASSERT(rit_p->ri[ri_idx].vld == 0,
                       ("ri_idx=%d, i=%d\n%s",
                        ri_idx, i, vtss_sprout__rit_to_str(rit_p)));

    for (i = ri_idx; i < VTSS_SPROUT_RIT_SIZE - 1; i++) {
        if (rit_p->ri[i + 1].vld) {
            rit_p->ri[i]       = rit_p->ri[i + 1];
            rit_p->ri[i + 1].vld = 0;
        } else {
            
            break;
        }
    }
} 


static void uit_shift_up(
    vtss_sprout__uit_t  *uit_p,
    uint              ui_idx)
{
    uint i = 0;

    VTSS_SPROUT_ASSERT(uit_p->ui[ui_idx].vld == 0,
                       ("ui_idx=%d, i=%d\nUIT:\n%s",
                        ui_idx, i,
                        vtss_sprout__uit_to_str(uit_p)));

    for (i = ui_idx; i < VTSS_SPROUT_UIT_SIZE - 1; i++) {
        if (uit_p->ui[i + 1].vld) {
            uit_p->ui[i]       = uit_p->ui[i + 1];
            uit_p->ui[i + 1].vld = 0;
        } else {
            
            break;
        }
    }
} 






static void rit_shift_dn(
    vtss_sprout__rit_t  *rit_p,
    uint        ri_idx)
{
    uint i = 0;

    VTSS_SPROUT_ASSERT(ri_idx < VTSS_SPROUT_RIT_SIZE - 1,
                       ("ri_idx=%d, i=%d\n%s",
                        ri_idx, i, vtss_sprout__rit_to_str(rit_p)));

    VTSS_SPROUT_ASSERT(rit_p->ri[ri_idx].vld == 1,
                       ("ri_idx=%d, i=%d\n%s",
                        ri_idx, i, vtss_sprout__rit_to_str(rit_p)));

    for (i = VTSS_SPROUT_RIT_SIZE - 1; i > ri_idx; i--) {
        VTSS_SPROUT_ASSERT(rit_p->ri[i].vld == 0,
                           ("ri_idx=%d, i=%d\n%s",
                            ri_idx, i, vtss_sprout__rit_to_str(rit_p)));

        
        if (rit_p->ri[i - 1].vld) {
            rit_p->ri[i]       = rit_p->ri[i - 1];
            rit_p->ri[i - 1].vld = 0;
        }
    }
} 


static void uit_shift_dn(
    vtss_sprout__uit_t  *uit_p,
    uint              ui_idx)
{
    uint i = 0;

    VTSS_SPROUT_ASSERT(ui_idx < VTSS_SPROUT_UIT_SIZE - 1,
                       ("ui_idx=%d, i=%d\nUIT:\n%s",
                        ui_idx, i,
                        vtss_sprout__uit_to_str(uit_p)));

    VTSS_SPROUT_ASSERT(uit_p->ui[ui_idx].vld == 1,
                       ("ui_idx=%d, i=%d\nUIT:\n%s",
                        ui_idx, i,
                        vtss_sprout__uit_to_str(uit_p)));

    for (i = VTSS_SPROUT_UIT_SIZE - 1; i > ui_idx; i--) {
        VTSS_SPROUT_ASSERT(uit_p->ui[i].vld == 0,
                           ("ui_idx=%d, i=%d\nUIT:\n%s",
                            ui_idx, i,
                            vtss_sprout__uit_to_str(uit_p)));

        
        if (uit_p->ui[i - 1].vld) {
            uit_p->ui[i]       = uit_p->ui[i - 1];
            uit_p->ui[i - 1].vld = 0;
        }
    }
} 








static int unit_addr_cmp(
    const vtss_sprout_switch_addr_t   *switch_addr_x_p,
    const vtss_sprout__unit_idx_t     unit_idx_x,
    const vtss_sprout_switch_addr_t   *switch_addr_y_p,
    const vtss_sprout__unit_idx_t     unit_idx_y)
{
    int switch_addr_cmp;

    switch_addr_cmp =
        vtss_sprout__switch_addr_cmp(switch_addr_x_p, switch_addr_y_p);

    if (switch_addr_cmp != 0) {
        return switch_addr_cmp;
    } else {
        
        if (unit_idx_x < unit_idx_y) {
            return -1;
        } else if (unit_idx_x > unit_idx_y) {
            return 1;
        } else {
            
            return 0;
        }
    }
} 
















BOOL vtss_sprout__rit_update(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__ri_t             *ri_p,
    const vtss_sprout__stack_port_idx_t sp_idx)
{
    uint i = 0;
    int  cmp;
    vtss_sprout__ri_t       *cur = NULL;
    const vtss_sprout__ri_t *new = NULL;

    for (i = 0; i < VTSS_SPROUT_RIT_SIZE; i++) {
        cmp = -1234;
        if (rit_p->ri[i].vld) {
            cmp = unit_addr_cmp(&ri_p->switch_addr,        ri_p->unit_idx,
                                &rit_p->ri[i].switch_addr, rit_p->ri[i].unit_idx);
            if (cmp == 0) {
                break;
            } else if (cmp < 0) {
                
                rit_shift_dn(rit_p, i);
                break;
            } else {
                
            }
        } else {
            
            break;
        }
    }

    if (i == VTSS_SPROUT_UIT_SIZE) {
        T_WG(TRACE_GRP_UPSID, "RIT size exceeded");
        return 0;
    }

    if (cmp != 0) {
        
        VTSS_SPROUT_ASSERT(rit_p->ri[i].vld == 0,
                           ("sp_idx=%d i=%d cmp=%d\nri=%s\n%s",
                            sp_idx, i, cmp,
                            vtss_sprout__ri_to_str(ri_p), vtss_sprout__rit_to_str(rit_p)))

        vtss_sprout__ri_init(&rit_p->ri[i]);
        rit_p->ri[i].vld             = 1;
        rit_p->changed               = 1;
        rit_p->ri[i].switch_addr     = ri_p->switch_addr;
        rit_p->ri[i].unit_idx        = ri_p->unit_idx;
        rit_p->ri[i].upsid[0]        = ri_p->upsid[0];
        rit_p->ri[i].upsid[1]        = ri_p->upsid[1];
        rit_p->ri[i].tightly_coupled = ri_p->tightly_coupled;
    }

    VTSS_SPROUT_ASSERT(rit_p->ri[i].vld == 1,
                       ("sp_idx=%d i=%d\nri=%s\n%s",
                        sp_idx, i, vtss_sprout__ri_to_str(ri_p), vtss_sprout__rit_to_str(rit_p)))

    rit_p->ri[i].found = 1;

    
    cur = &rit_p->ri[i];
    new = ri_p;
    if (new->stack_port_dist[sp_idx] !=
        cur->stack_port_dist[sp_idx]) {
        
        cur->stack_port_dist[sp_idx] = new->stack_port_dist[sp_idx];
        rit_p->changed = 1;
    } else if (new->tightly_coupled !=
               cur->tightly_coupled) {
        
        cur->tightly_coupled = new->tightly_coupled;
        rit_p->changed = 1;
    } else if (new->upsid[0] !=
               cur->upsid[0]) {
        
        cur->upsid[0] = new->upsid[0];
        rit_p->changed = 1;
    } else if (new->upsid[1] !=
               cur->upsid[1]) {
        
        cur->upsid[1] = new->upsid[1];
        rit_p->changed = 1;
    }

    return rit_p->changed;
} 






vtss_sprout__ri_t *vtss_sprout__ri_find(
    vtss_sprout__rit_t              *rit_p,
    const vtss_sprout_switch_addr_t *switch_addr_p,
    const vtss_sprout__unit_idx_t   unit_idx)
{
    vtss_sprout__ri_t *ri_p = NULL;
    int cmp;

    while ((ri_p = vtss_sprout__ri_get_nxt(rit_p, ri_p))) {
        cmp = unit_addr_cmp(switch_addr_p,      unit_idx,
                            &ri_p->switch_addr, ri_p->unit_idx);

        if (cmp == 0) {
            return ri_p;
        } else if (cmp < 0) {
            
            return NULL;
        }
    }

    return NULL;
} 






vtss_sprout__ri_t *vtss_sprout__ri_find_at_dist(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const vtss_sprout_dist_t            dist)
{
    vtss_sprout__ri_t *ri_p = NULL;

    while ((ri_p = vtss_sprout__ri_get_nxt(rit_p, ri_p))) {
        if (ri_p->stack_port_dist[sp_idx] == dist) {
            return ri_p;
        }
    }

    return ri_p;
} 






void vtss_sprout__rit_infinity_all(
    vtss_sprout__rit_t                        *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx)
{
    uint i = 0;

    while (i < VTSS_SPROUT_RIT_SIZE) {
        if (!rit_p->ri[i].vld) {
            return;
        }

        if (rit_p->ri[i].stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY) {
            rit_p->changed = 1;
            rit_p->ri[i].stack_port_dist[sp_idx] = VTSS_SPROUT_DIST_INFINITY;
        }

        if (rit_p->ri[i].stack_port_dist[(sp_idx + 1) % 2] ==
            VTSS_SPROUT_DIST_INFINITY) {
            
            rit_p->ri[i].vld = 0;
            rit_shift_up(rit_p, i);
            continue; 
        }
        i++;
    }

    return;
} 







void vtss_sprout__rit_infinity_beyond_dist(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t  sp_idx,
    const vtss_sprout_dist_t             max_dist)
{
    uint i = 0;

    if (max_dist == VTSS_SPROUT_DIST_INFINITY) {
        return;
    }

    while (i < VTSS_SPROUT_RIT_SIZE) {
        if (!rit_p->ri[i].vld) {
            return;
        }
        if (rit_p->ri[i].stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY &&
            rit_p->ri[i].stack_port_dist[sp_idx] > max_dist) {
            rit_p->changed = 1;
            rit_p->ri[i].stack_port_dist[sp_idx] = VTSS_SPROUT_DIST_INFINITY;

            if (rit_p->ri[i].stack_port_dist[(sp_idx + 1) % 2] ==
                VTSS_SPROUT_DIST_INFINITY) {
                
                rit_p->ri[i].vld = 0;
                rit_shift_up(rit_p, i);
                continue; 
            }
        }
        i++;
    }

    return;
} 







uint vtss_sprout__rit_infinity_if_not_found(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx)
{
    uint infinity_cnt = 0;
    uint i = 0;

    while (i < VTSS_SPROUT_RIT_SIZE) {
        if (!rit_p->ri[i].vld) {
            return infinity_cnt;
        }
        if (!rit_p->ri[i].found &&
            rit_p->ri[i].stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY) {
            rit_p->ri[i].stack_port_dist[sp_idx] = VTSS_SPROUT_DIST_INFINITY;
            infinity_cnt++;
            rit_p->changed = 1;

            if (rit_p->ri[i].stack_port_dist[(sp_idx + 1) % 2] ==
                VTSS_SPROUT_DIST_INFINITY) {
                
                rit_p->ri[i].vld = 0;
                rit_shift_up(rit_p, i);
                continue; 
            }
        }
        i++;
    }

    return infinity_cnt;
} 






void vtss_sprout__rit_clr_flags(
    vtss_sprout__rit_t                  *rit_p)
{
    uint i = 0;

    for (i = 0; i < VTSS_SPROUT_RIT_SIZE; i++) {
        if (!rit_p->ri[i].vld) {
            break;
        }
        rit_p->ri[i].found   = 0;
    }

    rit_p->changed = 0;
} 







vtss_sprout__ri_t *vtss_sprout__ri_get_nxt(
    vtss_sprout__rit_t       *rit_p,
    vtss_sprout__ri_t  *ri_p)
{
    if (ri_p == NULL) {
        
        if (rit_p->ri[0].vld) {
            return &rit_p->ri[0];
        } else {
            
            return NULL;
        }
    } else {
        if (ri_p[1].vld) {
            return &(ri_p[1]);
        } else {
            return NULL;
        }
    }
} 






vtss_sprout_dist_t vtss_sprout__get_dist(
    vtss_sprout__rit_t                        *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const vtss_sprout_switch_addr_t     *switch_addr_p,
    const vtss_sprout__unit_idx_t       unit_idx)
{
    vtss_sprout__ri_t *ri_p;

    ri_p = vtss_sprout__ri_find(rit_p,
                                switch_addr_p,
                                unit_idx);

    if (ri_p != NULL) {
        return ri_p->stack_port_dist[sp_idx];
    } else {
        return VTSS_SPROUT_DIST_INFINITY;
    }
} 













BOOL vtss_sprout__uit_update(
    vtss_sprout__uit_t      *uit_p,
    const vtss_sprout__ui_t *ui_p,
    BOOL                    pdu_rx) 
{
    uint i = 0;
    uint g = 0;
    int                     cmp;
    vtss_sprout__ui_t       *cur = NULL;
    const vtss_sprout__ui_t *new = NULL;
    BOOL                    glag_mbr_cnt_changed = 0;
    vtss_sprout__glagid_t   glagid;
    BOOL                    local_unit;
    BOOL                    ui_changed = 0;

    local_unit =
        (vtss_sprout__switch_addr_cmp(&ui_p->switch_addr,
                                      &switch_state.switch_addr) == 0);

    for (i = 0; i < VTSS_SPROUT_UIT_SIZE; i++) {
        cmp = -1234;
        if (uit_p->ui[i].vld) {
            cmp = unit_addr_cmp(&ui_p->switch_addr,        ui_p->unit_idx,
                                &uit_p->ui[i].switch_addr, uit_p->ui[i].unit_idx);
            if (cmp == 0) {
                break;
            } else if (cmp < 0) {
                
                uit_shift_dn(uit_p, i);
                break;
            } else {
                
            }
        } else {
            
            break;
        }
    }

    if (i == VTSS_SPROUT_UIT_SIZE) {
        T_WG(TRACE_GRP_UPSID, "RIT size exceeded");
        return 0;
    }

    if (cmp != 0) {
        
        VTSS_SPROUT_ASSERT(uit_p->ui[i].vld == 0,
                           ("i=%d cmp=%d\nri=%s\n%s",
                            i, cmp,
                            vtss_sprout__ui_to_str(ui_p), vtss_sprout__uit_to_str(uit_p)));
        vtss_sprout__ui_init(&uit_p->ui[i]);
        uit_p->ui[i].vld         = 1;
        uit_p->ui[i].switch_addr = ui_p->switch_addr;
        uit_p->ui[i].unit_idx    = ui_p->unit_idx;
        ui_changed               = 1;
    }

    VTSS_SPROUT_ASSERT(uit_p->ui[i].vld == 1,
                       ("i=%d cmp=%d\nri=%s\n%s",
                        i, cmp,
                        vtss_sprout__ui_to_str(ui_p), vtss_sprout__uit_to_str(uit_p)));

    uit_p->ui[i].found = 1;

    
    cur = &uit_p->ui[i];
    new = ui_p;

    glag_mbr_cnt_changed = 0;
    for (glagid = 0; glagid < VTSS_GLAGS; glagid++) {
        if (new->glag_mbr_cnt[glagid] != cur->glag_mbr_cnt[glagid]) {
            glag_mbr_cnt_changed = 1;
        }
    }

    if (cmp == 0 &&    
        (new->upsid[0]        != cur->upsid[0] ||
         new->upsid[1]        != cur->upsid[1])) {
        T_DG(TRACE_GRP_UPSID, "Found remote UPSID change: Old=%d/%d, New=%d/%d Switch addr=%s, pdu_rx=%d",
             cur->upsid[0], cur->upsid[1], new->upsid[0], new->upsid[1],
             vtss_sprout_switch_addr_to_str(&new->switch_addr), pdu_rx);
        ui_changed = 1;
        T_N("ui_changed=1 - upsid");

        if (!local_unit) {
            
            
            
            
            
            
            uit.changed      = 1;
            uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_REMOTE;
        }
    }
    if (new->primary_unit    != cur->primary_unit) {
        ui_changed = 1;
        T_N("ui_changed=1 - primary unit");
    }
#if defined(VTSS_SPROUT_V1)
    if (glag_mbr_cnt_changed) {
        ui_changed = 1;
        if (!(local_unit && pdu_rx)) {
            uit_p->change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_GLAG;
        }
    }
#endif
    if (new->have_mirror     != cur->have_mirror) {
        ui_changed = 1;
        T_N("ui_changed=1 - mirror");
    }
    if (new->mst_capable  != cur->mst_capable) {
        ui_changed = 1;
        T_N("ui_changed=1 - mst_capable");
    }
    if (new->mst_elect_prio  != cur->mst_elect_prio) {
        ui_changed = 1;
        T_N("ui_changed=1 - mst_elect_prio");
    }
    if (new->mst_time_ignore  != cur->mst_time_ignore) {
        ui_changed = 1;
        T_N("ui_changed=1 - mst_time_ignore");
    }
    if (new->mst_time != cur->mst_time) {
        








        uit_p->mst_time_changed |= !(local_unit && pdu_rx);
        cur->mst_time            = new->mst_time;
    }
    if (new->tightly_coupled != cur->tightly_coupled) {
        ui_changed = 1;
        T_N("ui_changed=1 - tightly_coupled");
    }
    if (new->ip_addr != cur->ip_addr) {
        ui_changed = 1;
        T_N("ui_changed=1 - ip_addr");
    }
    if (memcmp(new->switch_appl_info, cur->switch_appl_info, VTSS_SPROUT_SWITCH_APPL_INFO_LEN) != 0) {
        ui_changed = 1;
        T_N("ui_changed=1 - appl_info");
    }

    
    if ((new->unit_base_info_rsv != cur->unit_base_info_rsv) ||
        (memcmp(new->unit_glag_mbr_cnt_rsv, cur->unit_glag_mbr_cnt_rsv, VTSS_GLAGS) != 0) ||
        (memcmp(new->ups_base_info_rsv,     cur->ups_base_info_rsv,     2) != 0) ||
        (new->switch_mst_elect_rsv != cur->switch_mst_elect_rsv) ||
        (new->switch_base_info_rsv != cur->switch_base_info_rsv)) {
        T_I("Rsv fields changed for %s",
            vtss_sprout_switch_addr_to_str(&ui_p->switch_addr));
        ui_changed = 1;
        T_N("ui_changed=1 - resv bits");
    }

    if (ui_changed) {
        

        
        
        if (!(local_unit && pdu_rx)) {
            uit_p->changed      = 1;
            T_N("uit_p->changed=1");
        }

        cur->upsid[0]         = new->upsid[0];
        cur->upsid[1]         = new->upsid[1];

        
        cur->ups_port_mask[0] = new->ups_port_mask[0];
        cur->ups_port_mask[1] = new->ups_port_mask[1];

        cur->primary_unit   = new->primary_unit;
        for (g = 0; g < VTSS_GLAGS; g++) {
            cur->glag_mbr_cnt[g]   = new->glag_mbr_cnt[g];
        }
        cur->have_mirror     = new->have_mirror;
        cur->mst_capable     = new->mst_capable;
        cur->mst_elect_prio  = new->mst_elect_prio;
        cur->mst_time_ignore = new->mst_time_ignore;
        cur->tightly_coupled = new->tightly_coupled;

        cur->ip_addr         = new->ip_addr;
        memcpy(cur->switch_appl_info, new->switch_appl_info, VTSS_SPROUT_SWITCH_APPL_INFO_LEN);

        
        cur->unit_base_info_rsv = new->unit_base_info_rsv;
        memcpy(cur->unit_glag_mbr_cnt_rsv, new->unit_glag_mbr_cnt_rsv, VTSS_GLAGS);
        memcpy(cur->ups_base_info_rsv,     new->ups_base_info_rsv,     2);
        cur->switch_mst_elect_rsv = new->switch_mst_elect_rsv;
        cur->switch_base_info_rsv = new->switch_base_info_rsv;

        cur->sp_idx          = new->sp_idx;
    }

    return 0;
} 






vtss_sprout__ui_t *vtss_sprout__ui_find(
    vtss_sprout__uit_t              *uit_p,
    const vtss_sprout_switch_addr_t *switch_addr_p,
    const vtss_sprout__unit_idx_t   unit_idx)
{
    vtss_sprout__ui_t *ui_p = NULL;
    int cmp;

    while ((ui_p = vtss_sprout__ui_get_nxt(uit_p, ui_p))) {
        cmp = unit_addr_cmp(switch_addr_p,      unit_idx,
                            &ui_p->switch_addr, ui_p->unit_idx);

        if (cmp == 0) {
            return ui_p;
        } else if (cmp < 0) {
            
            return NULL;
        }
    }

    return NULL;
} 






void vtss_sprout__uit_clr_flags(
    vtss_sprout__uit_t              *uit_p)
{
    uint i = 0;

    for (i = 0; i < VTSS_SPROUT_UIT_SIZE; i++) {
        if (!uit_p->ui[i].vld) {
            break;
        }
        uit_p->ui[i].found   = 0;
    }

    uit_p->changed          = 0;
    uit_p->mst_time_changed = 0;
    uit_p->change_mask      = 0;
} 








uint vtss_sprout__uit_del_unfound(
    vtss_sprout__uit_t *uit_p)
{
    uint delete_cnt = 0;
    uint i = 0;

    while (i < VTSS_SPROUT_UIT_SIZE) {
        if (!uit_p->ui[i].vld) {
            return delete_cnt;
        }
        if (!uit_p->ui[i].found) {

            uit_p->changed     = 1;

#if defined(VTSS_SPROUT_V1)
            {
                uint glag_idx;
                for (glag_idx = 0; glag_idx < VTSS_GLAGS; glag_idx++) {
                    if (uit_p->ui[i].glag_mbr_cnt[glag_idx] > 0) {
                        uit_p->change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_GLAG;
                    }
                }
            }
#endif

            uit_p->ui[i].vld = 0;
            uit_shift_up(uit_p, i);
            continue; 
        }
        i++;
    }

    return delete_cnt;
} 











vtss_sprout__ui_t *vtss_sprout__ui_get_nxt(
    vtss_sprout__uit_t *uit_p,
    vtss_sprout__ui_t  *ui_p)
{
    if (ui_p == NULL) {
        
        if (uit_p->ui[0].vld) {
            return &uit_p->ui[0];
        } else {
            
            return NULL;
        }
    } else {
        if (ui_p[1].vld) {
            return &(ui_p[1]);
        } else {
            return NULL;
        }
    }
} 

















