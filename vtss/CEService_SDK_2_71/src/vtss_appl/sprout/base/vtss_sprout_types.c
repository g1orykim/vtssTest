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

 $Id$
 $Revision$


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/








#include "vtss_sprout.h"
#include <string.h>


void vtss_sprout__ri_init(
    vtss_sprout__ri_t *ri_p)
{
    memset(ri_p, 0, sizeof(vtss_sprout__ri_t));

    ri_p->upsid[0]           = VTSS_VSTAX_UPSID_UNDEF;
    ri_p->upsid[1]           = VTSS_VSTAX_UPSID_UNDEF;

    ri_p->stack_port_dist[0] = VTSS_SPROUT_DIST_INFINITY;
    ri_p->stack_port_dist[1] = VTSS_SPROUT_DIST_INFINITY;
} 


void vtss_sprout__rit_init(vtss_sprout__rit_t  *rit_p)
{
    vtss_sprout__ri_t *ri_p = NULL;

    rit_p->changed = 0;

    while ((ri_p = vtss_sprout__ri_get_nxt(rit_p, ri_p))) {
        vtss_sprout__ri_init(ri_p);
    }
} 



void vtss_sprout__ui_init(
    vtss_sprout__ui_t *ui_p)
{
    uint g;
    const vtss_sprout_switch_addr_t switch_addr_null = VTSS_SPROUT_SWITCH_ADDR_NULL;

    memset(ui_p, 0, sizeof(vtss_sprout__ui_t));

    ui_p->vld            = 0;
    ui_p->found          = 0;

    ui_p->switch_addr    = switch_addr_null;
    ui_p->unit_idx       = 0;

    ui_p->upsid[0]       = VTSS_VSTAX_UPSID_UNDEF;
    ui_p->upsid[1]       = VTSS_VSTAX_UPSID_UNDEF;
    ui_p->primary_unit   = 0;

    for (g = 0; g < VTSS_GLAGS; g++) {
        ui_p->glag_mbr_cnt[g] = 0;
    }

    ui_p->have_mirror    = 0;

    ui_p->mst_capable    = 0;
    ui_p->mst_elect_prio = VTSS_SPROUT_MST_ELECT_PRIO_DEFAULT - 1;
    ui_p->mst_time       = 0;

    ui_p->ip_addr = 0;

    ui_p->sp_idx = VTSS_SPROUT__SP_UNDEF;
} 


void vtss_sprout__uit_init(
    vtss_sprout__uit_t *uit_p)
{
    uint i;

    uit_p->changed          = 0;
    uit_p->mst_time_changed = 0;
    uit_p->change_mask      = 0;

    for (i = 0; i < VTSS_SPROUT_UIT_SIZE; i++) {
        vtss_sprout__ui_init(&uit_p->ui[i]);
    }
} 


void vtss_sprout__stack_port_state_init(vtss_sprout__stack_port_state_t *const sps_p)
{
    memset(sps_p, 0, sizeof(vtss_sprout__stack_port_state_t));

    sps_p->adm_up     = 0;
    sps_p->link_up    = 0;
    sps_p->port_no    = 0;

    sps_p->update_age_time     = VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT;
    sps_p->update_interval_slv = VTSS_SPROUT_UPDATE_INTERVAL_SLV_DEFAULT;
    sps_p->update_interval_mst = VTSS_SPROUT_UPDATE_INTERVAL_MST_DEFAULT;
    sps_p->update_limit        = VTSS_SPROUT_UPDATE_LIMIT_DEFAULT;
    sps_p->alert_limit         = VTSS_SPROUT_ALERT_LIMIT_DEFAULT;

    sps_p->proto_up   = 0;
    vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);
    vtss_sprout__uit_init(&sps_p->uit);
    sps_p->ttl        = 0;
    sps_p->ttl_new    = 0;
    sps_p->mirror_fwd = 0;
    sps_p->cfg_change = 0;

    sps_p->deci_secs_since_last_tx_update = 0;
    sps_p->deci_secs_since_last_rx_update = 0;

    sps_p->update_bucket                              = 0;
    sps_p->alert_bucket                               = 0;
    sps_p->deci_secs_since_last_update_bucket_filling = 0;
    sps_p->deci_secs_since_last_alert_bucket_filling  = 0;

    sps_p->sprout_update_rx_cnt           = 0;
    sps_p->sprout_update_periodic_tx_cnt  = 0;
    sps_p->sprout_update_triggered_tx_cnt = 0;
    sps_p->sprout_update_rx_err_cnt       = 0;
    sps_p->sprout_update_tx_err_cnt       = 0;

    sps_p->sprout_alert_rx_cnt              = 0;
    sps_p->sprout_alert_tx_cnt              = 0;
    sps_p->sprout_alert_tx_policer_drop_cnt = 0;
    sps_p->sprout_alert_rx_err_cnt          = 0;
    sps_p->sprout_alert_tx_err_cnt          = 0;
} 


void vtss_sprout__unit_state_init(vtss_sprout__unit_state_t *const us_p)
{
    vtss_sprout__glagid_t glagid;
    uint                spidx;

    us_p->upsid = -1;

    for (glagid = 0; glagid < VTSS_GLAGS; glagid++) {
        us_p->glag_mbr_cnt[glagid] = 0;
    }

    us_p->have_mirror_port = 0;

    for (spidx = 0; spidx < 2; spidx++) {
        vtss_sprout__stack_port_state_init(&us_p->stack_port[spidx]);
    }
} 


void vtss_sprout__switch_state_init (vtss_sprout__switch_state_t *const ss_p)
{
    uint chipidx;

    memset(ss_p, 0, sizeof(vtss_sprout__switch_state_t));

    vtss_sprout_switch_addr_init(&ss_p->switch_addr);
    ss_p->mst_elect_prio    = VTSS_SPROUT_MST_ELECT_PRIO_DEFAULT - 1;
    ss_p->mst               = 0;
    ss_p->mst_start_time    = 0;
    ss_p->mst_upsid         = VTSS_VSTAX_UPSID_UNDEF;
    ss_p->cpu_qu            = 0;

    ss_p->topology_type     = VtssTopoOpenLoop;
    ss_p->topology_type_new = VtssTopoOpenLoop;
    ss_p->topology_n        = 1;
#if defined(VTSS_SPROUT_V1)
    ss_p->ui_top_p          = NULL;
    ss_p->top_link          = 0;
    ss_p->top_dist          = VTSS_SPROUT_DIST_INFINITY;
#endif

    ss_p->virgin            = 1;
    ss_p->topology_change_time = 0; 

    for (chipidx = 0; chipidx < vtss_board_chipcount(); chipidx++) {
        vtss_sprout__unit_state_init(&ss_p->unit[chipidx]);
    }
#if defined(VTSS_SPROUT_FW_VER_CHK)
    ss_p->fw_ver_mode        = VTSS_SPROUT_FW_VER_MODE_NORMAL;
#endif
} 


void vtss_sprout_switch_addr_init(vtss_sprout_switch_addr_t *sa_p)
{
    sa_p->addr[0] = 0;
    sa_p->addr[1] = 0;
    sa_p->addr[2] = 0;
    sa_p->addr[3] = 0;
    sa_p->addr[4] = 0;
    sa_p->addr[5] = 0;
} 






char *vtss_sprout__ri_to_str(const vtss_sprout__ri_t *ri_p)
{
    static char s[100];
    const  int  size = sizeof(s);

    s[0] = 0;

    vtss_sprout__str_append(
        s, size,
        "%02x%02x%02x.%02x%02x%02x/%d: found=%d dist[0]=%d dist[1]=%d tight=%d upsids=%d,%d",
        ri_p->switch_addr.addr[0],
        ri_p->switch_addr.addr[1],
        ri_p->switch_addr.addr[2],
        ri_p->switch_addr.addr[3],
        ri_p->switch_addr.addr[4],
        ri_p->switch_addr.addr[5],
        ri_p->unit_idx,

        ri_p->found,

        ri_p->stack_port_dist[0],
        ri_p->stack_port_dist[1],
        ri_p->tightly_coupled,
        ri_p->upsid[0],
        ri_p->upsid[1]
    );

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout__ri_print(vtss_sprout__ri_t *ri_p)
{
    printf("%s\n", vtss_sprout__ri_to_str(ri_p));
} 


char *vtss_sprout__rit_to_str(vtss_sprout__rit_t *rit_p)
{
    static char s[VTSS_SPROUT_RIT_SIZE * 150];
    const  int  size = sizeof(s);

    vtss_sprout__ri_t *ri_p = NULL;
    uint i = 0;

    s[0] = 0;

    vtss_sprout__str_append(s, size, "RIT:\n");

    if (rit_p->changed) {
        vtss_sprout__str_append(s, size, "Changed!\n");
    }
    while ((ri_p = vtss_sprout__ri_get_nxt(rit_p, ri_p))) {
        vtss_sprout__str_append(s, size, "%2d: ", i);
        vtss_sprout__str_append(s, size, "%s\n", vtss_sprout__ri_to_str(ri_p));

        i++;
    }

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout__rit_print(vtss_sprout_dbg_printf_t dbg_printf,
                            vtss_sprout__rit_t *rit_p)
{
    dbg_printf("%s", vtss_sprout__rit_to_str(rit_p));
} 


char *vtss_sprout__ui_to_str(const vtss_sprout__ui_t  *ui_p)
{
    static char              s[500];
    const  int               size = sizeof(s);
    vtss_sprout__upsid_idx_t ups_idx;
#if defined(VTSS_SPROUT_V1)
    uint                     glag_id;
#endif

    s[0] = 0;

    vtss_sprout__str_append(s, size, "%02x%02x%02x.%02x%02x%02x/%d: found=%d upsid=%2d,%2d "
#if defined(VTSS_SPROUT_V1)
                            "glag_mbr_cnt=%d,%d "
#endif
                            "have_mirror=%d\n"
                            "    mst:(elect_prio=%d time=%lu time_ignore=%d capable=%d)\n"
                            "    ip=%d.%d.%d.%d sp_idx=%d tight=%d",
                            ui_p->switch_addr.addr[0],
                            ui_p->switch_addr.addr[1],
                            ui_p->switch_addr.addr[2],
                            ui_p->switch_addr.addr[3],
                            ui_p->switch_addr.addr[4],
                            ui_p->switch_addr.addr[5],
                            ui_p->unit_idx,
                            ui_p->found,
                            ui_p->upsid[0],
                            ui_p->upsid[1],
#if defined(VTSS_SPROUT_V1)
                            ui_p->glag_mbr_cnt[0],
                            ui_p->glag_mbr_cnt[1],
#endif
                            ui_p->have_mirror,

                            ui_p->mst_elect_prio,
                            ui_p->mst_time,
                            ui_p->mst_time_ignore,
                            ui_p->mst_capable,

                            (int)((ui_p->ip_addr >> 24) & 0xff),
                            (int)((ui_p->ip_addr >> 16) & 0xff),
                            (int)((ui_p->ip_addr >>  8) & 0xff),
                            (int)((ui_p->ip_addr)       & 0xff),
                            ui_p->sp_idx,
                            ui_p->tightly_coupled);

#if 0
    {
        int                      i;

        vtss_sprout__str_append(s, size, " appl_info=0x");
        for (i = 0; i < VTSS_SPROUT_SWITCH_APPL_INFO_LEN; i++) {
            vtss_sprout__str_append(s, size, "%02x", ui_p->switch_appl_info[i]);
        }
    }
#endif

    vtss_sprout__str_append(s, size, "\n    ups_port_masks=%s/%s",
                            vtss_sprout_port_mask_to_str(ui_p->ups_port_mask[0]),
                            vtss_sprout_port_mask_to_str(ui_p->ups_port_mask[1]));

#if 0
    vtss_sprout__str_append(s, size, " (0x%016llx/0x%016llx)",
                            ui_p->ups_port_mask[0], ui_p->ups_port_mask[1]);
#endif

    vtss_sprout__str_append(s, size, "\n");

    
    vtss_sprout__str_append(s, size, "    rsv=");
    vtss_sprout__str_append(s, size, "%02x.", ui_p->unit_base_info_rsv);
#if defined(VTSS_SPROUT_V1)
    for (glag_id = 0; glag_id < VTSS_GLAGS; glag_id++) {
        vtss_sprout__str_append(s, size, "%02x", ui_p->unit_glag_mbr_cnt_rsv[glag_id]);
    }
    vtss_sprout__str_append(s, size, ".");
#endif
    for (ups_idx = 0; ups_idx < 2; ups_idx++) {
        vtss_sprout__str_append(s, size, "%02x", ui_p->ups_base_info_rsv[ups_idx]);
    }
    vtss_sprout__str_append(s, size, ".%02x", ui_p->switch_mst_elect_rsv);
    vtss_sprout__str_append(s, size, ".%02x", ui_p->switch_base_info_rsv);

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout__ui_print( vtss_sprout__ui_t  *ui_p)
{
    printf("%s\n", vtss_sprout__ui_to_str(ui_p));
} 


char *vtss_sprout__uit_to_str(vtss_sprout__uit_t *uit_p)
{
    static char s[VTSS_SPROUT_RIT_SIZE * 200];
    const  int  size = sizeof(s);
    vtss_sprout__ui_t *ui_p = NULL;
    uint i = 0;

    s[0] = 0;
    vtss_sprout__str_append(s, size, "changed=%d\n", uit_p->changed);
    vtss_sprout__str_append(s, size, "mst_time_changed=%d\n", uit_p->mst_time_changed);
    vtss_sprout__str_append(s, size, "change_mask=0x%lx\n", uit_p->change_mask);
    while ((ui_p = vtss_sprout__ui_get_nxt(uit_p, ui_p))) {
        vtss_sprout__str_append(s, size, "%2d: ", i);
        vtss_sprout__str_append(s, size, "%s\n", vtss_sprout__ui_to_str(ui_p));
        i++;
    }

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout__uit_print(vtss_sprout_dbg_printf_t dbg_printf,
                            vtss_sprout__uit_t *uit_p)
{
    vtss_sprout__ui_t *ui_p = NULL;
    uint i = 0;

    dbg_printf("changed=%d\n", uit_p->changed);
    dbg_printf("change_mask=%x\n", uit_p->change_mask);
    while ((ui_p = vtss_sprout__ui_get_nxt(uit_p, ui_p))) {
        dbg_printf("%2d: ", i);
        dbg_printf("%s\n", vtss_sprout__ui_to_str(ui_p));
        i++;
    }
} 


char *vtss_sprout_switch_addr_to_str(const vtss_sprout_switch_addr_t *sa_p)
{
    static char s[8][14];
    static uint i = 0;

    i = (i + 1) % 8;

    sprintf(s[i], "%02x%02x%02x.%02x%02x%02x",
            sa_p->addr[0],
            sa_p->addr[1],
            sa_p->addr[2],
            sa_p->addr[3],
            sa_p->addr[4],
            sa_p->addr[5]);

    VTSS_ASSERT(strlen(s[i]) < 14);

    return s[i];
} 


void vtss_sprout_switch_addr_print(vtss_sprout_switch_addr_t *sa_p)
{
    printf("%s", vtss_sprout_switch_addr_to_str(sa_p));
} 


char *vtss_sprout_topology_type_to_str(vtss_sprout_topology_type_t topology_type)
{
    static char s[20];
    const  int  size = sizeof(s);

    s[0] = 0;

    switch (topology_type) {
    case VtssTopoBack2Back:
        vtss_sprout__str_append(s, size, "Back2Back");
        break;
    case VtssTopoClosedLoop:
        vtss_sprout__str_append(s, size, "ClosedLoop");
        break;
    case VtssTopoOpenLoop:
        vtss_sprout__str_append(s, size, "OpenLoop");
        break;
    default:
        vtss_sprout__str_append(s, size, "Unknown!");
        break;
    }

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout_topology_type_print(vtss_sprout_topology_type_t topology_type)
{
    printf("%s", vtss_sprout_topology_type_to_str(topology_type));
} 


char *vtss_sprout__stack_port_states_to_str(
    const vtss_sprout__stack_port_state_t *sps0_p,
    const vtss_sprout__stack_port_state_t *sps1_p, 
    const vtss_sprout__stack_port_state_t *sps2_p, 
    const vtss_sprout__stack_port_state_t *sps3_p, 
    const char *prefix)
{
    static uint i = 0;
    static char s[2][800];
    const  int  size = sizeof(s) / 2;

    i = (i + 1) % 2;

    s[i][0] = 0;

    vtss_sprout__str_append(s[i], size, "%sadm_up=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->adm_up,
                            sps1_p ? sps1_p->adm_up : 0,
                            sps2_p ? sps2_p->adm_up : 0,
                            sps3_p ? sps3_p->adm_up : 0
                           );
    vtss_sprout__str_append(s[i], size, "%slink_up=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->link_up,
                            sps1_p ? sps1_p->link_up : 0,
                            sps2_p ? sps2_p->link_up : 0,
                            sps3_p ? sps3_p->link_up : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sproto_up=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->proto_up,
                            sps1_p ? sps1_p->proto_up : 0,
                            sps2_p ? sps2_p->proto_up : 0,
                            sps3_p ? sps3_p->proto_up : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sport_no=%lu/%lu & %lu/%lu\n",
                            prefix,
                            sps0_p->port_no,
                            sps1_p ? sps1_p->port_no : 0,
                            sps2_p ? sps2_p->port_no : 0,
                            sps3_p ? sps3_p->port_no : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sttl=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->ttl,
                            sps1_p ? sps1_p->ttl : 0,
                            sps2_p ? sps2_p->ttl : 0,
                            sps3_p ? sps3_p->ttl : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sttl_new=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->ttl_new,
                            sps1_p ? sps1_p->ttl_new : 0,
                            sps2_p ? sps2_p->ttl_new : 0,
                            sps3_p ? sps3_p->ttl_new : 0
                           );
    vtss_sprout__str_append(s[i], size, "%smirror_fwd=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->mirror_fwd,
                            sps1_p ? sps1_p->mirror_fwd : 0,
                            sps2_p ? sps2_p->mirror_fwd : 0,
                            sps3_p ? sps3_p->mirror_fwd : 0
                           );
    vtss_sprout__str_append(s[i], size, "%scfg_change=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->cfg_change,
                            sps1_p ? sps1_p->cfg_change : 0,
                            sps2_p ? sps2_p->cfg_change : 0,
                            sps3_p ? sps3_p->cfg_change : 0
                           );
    vtss_sprout__str_append(s[i], size, "%snbr_switch_addr=%s/%s & %s/%s\n",
                            prefix,
                            vtss_sprout_switch_addr_to_str(&sps0_p->nbr_switch_addr),
                            sps1_p ? vtss_sprout_switch_addr_to_str(&sps1_p->nbr_switch_addr) : "",
                            sps2_p ? vtss_sprout_switch_addr_to_str(&sps2_p->nbr_switch_addr) : "",
                            sps3_p ? vtss_sprout_switch_addr_to_str(&sps3_p->nbr_switch_addr) : ""
                           );
    vtss_sprout__str_append(s[i], size, "%supdate_interval (m/s): %d/%d / %d/%d  &  %d/%d / %d/%d\n",
                            prefix,
                            sps0_p->update_interval_mst,
                            sps0_p->update_interval_slv,
                            sps1_p ? sps1_p->update_interval_mst : 0,
                            sps2_p ? sps2_p->update_interval_mst : 0,
                            sps3_p ? sps3_p->update_interval_mst : 0,
                            sps1_p ? sps1_p->update_interval_slv : 0,
                            sps2_p ? sps2_p->update_interval_slv : 0,
                            sps3_p ? sps3_p->update_interval_slv : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sdeci_secs_since_last_tx_update=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->deci_secs_since_last_tx_update,
                            sps1_p ? sps1_p->deci_secs_since_last_tx_update : 0,
                            sps2_p ? sps2_p->deci_secs_since_last_tx_update : 0,
                            sps3_p ? sps3_p->deci_secs_since_last_tx_update : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sdeci_secs_since_last_rx_update=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->deci_secs_since_last_rx_update,
                            sps1_p ? sps1_p->deci_secs_since_last_rx_update : 0,
                            sps2_p ? sps2_p->deci_secs_since_last_rx_update : 0,
                            sps3_p ? sps3_p->deci_secs_since_last_rx_update : 0
                           );
    vtss_sprout__str_append(s[i], size, "%supdate_bucket=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->update_bucket,
                            sps1_p ? sps1_p->update_bucket : 0,
                            sps2_p ? sps2_p->update_bucket : 0,
                            sps3_p ? sps3_p->update_bucket : 0
                           );
    vtss_sprout__str_append(s[i], size, "%salert_bucket=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->alert_bucket,
                            sps1_p ? sps1_p->alert_bucket : 0,
                            sps2_p ? sps2_p->alert_bucket : 0,
                            sps3_p ? sps3_p->alert_bucket : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sdeci_secs_since_last_update_bucket_filling=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->deci_secs_since_last_update_bucket_filling,
                            sps1_p ? sps1_p->deci_secs_since_last_update_bucket_filling : 0,
                            sps2_p ? sps2_p->deci_secs_since_last_update_bucket_filling : 0,
                            sps3_p ? sps3_p->deci_secs_since_last_update_bucket_filling : 0
                           );
    vtss_sprout__str_append(s[i], size, "%sdeci_secs_since_last_alert_bucket_filling=%d/%d & %d/%d\n",
                            prefix,
                            sps0_p->deci_secs_since_last_alert_bucket_filling,
                            sps1_p ? sps1_p->deci_secs_since_last_alert_bucket_filling : 0,
                            sps2_p ? sps2_p->deci_secs_since_last_alert_bucket_filling : 0,
                            sps3_p ? sps3_p->deci_secs_since_last_alert_bucket_filling : 0
                           );
    vtss_sprout__str_append(s[i], size, "%srx/tx cnt=%d/%d+%d / %d/%d+%d & %d/%d+%d / %d/%d+%d\n",
                            prefix,
                            sps0_p->sprout_update_rx_cnt,
                            sps0_p->sprout_update_periodic_tx_cnt,
                            sps0_p->sprout_update_triggered_tx_cnt,
                            sps1_p ? sps1_p->sprout_update_rx_cnt : 0,
                            sps1_p ? sps1_p->sprout_update_periodic_tx_cnt : 0,
                            sps1_p ? sps1_p->sprout_update_triggered_tx_cnt : 0,
                            sps2_p ? sps2_p->sprout_update_rx_cnt : 0,
                            sps2_p ? sps2_p->sprout_update_periodic_tx_cnt : 0,
                            sps2_p ? sps2_p->sprout_update_triggered_tx_cnt : 0,
                            sps3_p ? sps3_p->sprout_update_rx_cnt : 0,
                            sps3_p ? sps3_p->sprout_update_periodic_tx_cnt : 0,
                            sps3_p ? sps3_p->sprout_update_triggered_tx_cnt : 0
                           );
    vtss_sprout__str_append(s[i], size, "%srx/tx err cnt=%d/%d / %d/%d & %d/%d / %d/%d\n",
                            prefix,
                            sps0_p->sprout_update_rx_err_cnt,
                            sps0_p->sprout_update_tx_err_cnt,
                            sps1_p ? sps1_p->sprout_update_rx_err_cnt : 0,
                            sps1_p ? sps1_p->sprout_update_tx_err_cnt : 0,
                            sps2_p ? sps2_p->sprout_update_rx_err_cnt : 0,
                            sps2_p ? sps2_p->sprout_update_tx_err_cnt : 0,
                            sps3_p ? sps3_p->sprout_update_rx_err_cnt : 0,
                            sps3_p ? sps3_p->sprout_update_tx_err_cnt : 0
                           );

    VTSS_ASSERT(strlen(s[i]) < size);

    return s[i];
} 


char *vtss_sprout__unit_state_to_str(vtss_sprout__switch_state_t *ss_p,
                                     vtss_sprout__unit_state_t *const specific_us_p, 
                                     const char *prefix)
{
    static uint i = 0;
    static char s[2][2000];
    const  int  size = sizeof(s) / 2;
    char   prefix2[10];
    vtss_sprout__unit_state_t *us0_p;
    vtss_sprout__unit_state_t *us1_p;

    sprintf(prefix2, "%s  ", prefix);

    i = (i + 1) % 2;

    s[i][0] = 0;

    us0_p = specific_us_p;
    us1_p = NULL;
    if (specific_us_p == NULL) {
        
        VTSS_SPROUT_ASSERT(ss_p != NULL,
                           ("Invalid arguments"));

        us0_p = &ss_p->unit[0];
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
        us1_p = &ss_p->unit[1];
#endif
    }

    vtss_sprout__str_append(s[i], size, "%sglag_mbr_cnt=%d,%d & %d,%d\n",
                            prefix,
                            us0_p->glag_mbr_cnt[0],
                            us0_p->glag_mbr_cnt[1],
                            us1_p ? us1_p->glag_mbr_cnt[0] : -1,
                            us1_p ? us1_p->glag_mbr_cnt[1] : -1);
    vtss_sprout__str_append(s[i], size, "%shave_mirror_port=%d & %d\n",
                            prefix,
                            us0_p->have_mirror_port,
                            us1_p ? us1_p->have_mirror_port : -1);
    vtss_sprout__str_append(s[i], size, "%supsid=%d & %d\n",
                            prefix,
                            us0_p->upsid,
                            us1_p ? us1_p->upsid : -1);
    vtss_sprout__str_append(s[i], size, "%sups_port_mask=%s & %s\n",
                            prefix,
                            vtss_sprout_port_mask_to_str(us0_p->ups_port_mask),
                            us1_p ? vtss_sprout_port_mask_to_str(us1_p->ups_port_mask) : "-");
    vtss_sprout__str_append(s[i], size,
                            "%sStack port unit0:0/1 & unit1:0/1:\n",
                            prefix);
    vtss_sprout__str_append(s[i], size, "%s\n",
                            vtss_sprout__stack_port_states_to_str(&us0_p->stack_port[0],
                                                                  &us0_p->stack_port[1],
                                                                  us1_p ? &us1_p->stack_port[0] : NULL,
                                                                  us1_p ? &us1_p->stack_port[1] : NULL,
                                                                  prefix2));

    VTSS_ASSERT(strlen(s[i]) < size);

    return s[i];
} 


char *vtss_sprout__switch_state_to_str(vtss_sprout__switch_state_t *ss_p)
{
    static char s[2800];
    const  int  size = sizeof(s);

    s[0] = 0;

    vtss_sprout__str_append(s, size, "switch_addr=%s\n", vtss_sprout_switch_addr_to_str(&ss_p->switch_addr));
    vtss_sprout__str_append(s, size, "mst_elect_prio=%d\n", ss_p->mst_elect_prio);
    vtss_sprout__str_append(s, size, "mst_time_ignore=%d\n", ss_p->mst_time_ignore);
    vtss_sprout__str_append(s, size, "mst_switch_addr=%s\n", vtss_sprout_switch_addr_to_str(&ss_p->mst_switch_addr));
    vtss_sprout__str_append(s, size, "mst_upsid=%d\n", ss_p->mst_upsid);
    vtss_sprout__str_append(s, size, "mst=%d\n", ss_p->mst);
    vtss_sprout__str_append(s, size, "mst_start_time=%lu\n", ss_p->mst_start_time);
    vtss_sprout__str_append(s, size, "cpu_qu=%d\n", ss_p->cpu_qu);
    vtss_sprout__str_append(s, size, "topology_type=%s\n", vtss_sprout_topology_type_to_str(ss_p->topology_type));
    vtss_sprout__str_append(s, size, "topology_type_new=%s\n", vtss_sprout_topology_type_to_str(ss_p->topology_type_new));
    vtss_sprout__str_append(s, size, "topology_n=%d\n", ss_p->topology_n);
#if defined(VTSS_SPROUT_V1)
    if (!ss_p->ui_top_p) {
        vtss_sprout__str_append(s, size, "ui_top_p=NULL\n");
    } else {
        vtss_sprout__str_append(s, size, "top switch_addr=%s\n",
                                vtss_sprout_switch_addr_to_str(&ss_p->ui_top_p->switch_addr));
    }
    vtss_sprout__str_append(s, size, "top_link=%d\n", ss_p->top_link);
    vtss_sprout__str_append(s, size, "top_dist=%d\n", ss_p->top_dist);
#endif

    vtss_sprout__str_append(s, size, "virgin=%d\n",  ss_p->virgin);

#if defined(VTSS_SPROUT_FW_VER_CHK)
    vtss_sprout__str_append(s, size, "fw_ver_mode=%s\n",
                            (ss_p->fw_ver_mode == VTSS_SPROUT_FW_VER_MODE_NORMAL) ?
                            "normal" : "null");
#endif

    if (vtss_board_chipcount() == 1) {
        vtss_sprout__str_append(s, size, "Unit 0\n");
        vtss_sprout__str_append(s, size, "%s", vtss_sprout__unit_state_to_str(ss_p, &ss_p->unit[0], "  "));
    } else {
        vtss_sprout__str_append(s, size, "Unit 0 & 1\n");
        vtss_sprout__str_append(s, size, "%s", vtss_sprout__unit_state_to_str(ss_p, NULL, "  "));
    }

    VTSS_ASSERT(strlen(s) < size);

    return s;
} 


void vtss_sprout__switch_state_print(vtss_sprout_dbg_printf_t dbg_printf,
                                     vtss_sprout__switch_state_t *ss_p)
{
    int i = 0;
    char *s;

    s = vtss_sprout__switch_state_to_str(ss_p);
    
    while (i < strlen(s)) {
        dbg_printf("%c", s[i]);
        i++;
    }
} 










