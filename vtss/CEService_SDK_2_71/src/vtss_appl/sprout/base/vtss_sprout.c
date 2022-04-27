/*

 Vitesse Switch Software.

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

 $Id: vtss_sprout.c,v 1.199 2011/01/10 19:58:58 fj Exp $
 $Revision: 1.199 $


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/



























































































#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#if !(VTSS_OPSYS_ECOS)
#include <time.h>
#endif

#include "vtss_sprout.h"






#ifndef VTSS_SPROUT_SPROUT_PERIODIC_UPDATES
#define VTSS_SPROUT_SPROUT_PERIODIC_UPDATES 1
#endif

#ifndef VTSS_SPROUT_SPROUT_TRIGGERED_UPDATES
#define VTSS_SPROUT_SPROUT_TRIGGERED_UPDATES 1
#endif

#ifndef VTSS_SPROUT_SPROUT_AGING
#define VTSS_SPROUT_SPROUT_AGING 1
#endif



#ifndef VTSS_SPROUT_DEBUG_DMAC_INCR
#define VTSS_SPROUT_DEBUG_DMAC_INCR 0
#endif













#if VTSS_SPROUT_DEBUG_DMAC_INCR
static       vtss_mac_t sprout_dmac = {{0x01, 0x01, 0xC1, 0x00, 0x00, 0x02}};
#else
static const vtss_mac_t sprout_dmac = {{0x01, 0x01, 0xC1, 0x00, 0x00, 0x02}};
#endif

static       vtss_mac_t sprout_smac = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static const vtss_sprout_switch_addr_t null_switch_addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static const uchar exbit_protocol_ssp[4]  = {0x88, 0x80, 0x00, 0x02};
static const uchar ssp_sprout[4]          = {0x00, 0x01, 0x00, 0x00};
#define            SPROUT_PDU_TYPE_UPDATE   1
static const uchar sprout_pdu_type_update = SPROUT_PDU_TYPE_UPDATE;

#if defined(VTSS_SPROUT_V2)
#define            SPROUT_PDU_TYPE_ALERT    2
static const uchar sprout_pdu_type_alert  = SPROUT_PDU_TYPE_ALERT;
#define            SPROUT_VER 2
static const uchar sprout_ver             = SPROUT_VER;
#else

#if defined(VTSS_SPROUT_FW_VER_CHK)


#define            SPROUT_VER 2
#else

#define            SPROUT_VER 0
#endif

static const uchar sprout_ver             = SPROUT_VER;
#endif

static const uchar zeros[VTSS_SPROUT_FW_VER_LEN]; 

#define SPROUT_ALERT_TYPE_PROTODOWN 0x1


#define SPROUT_SECTION_TYPE_END    0x0 
#define SPROUT_SECTION_TYPE_SWITCH 0x1


#define SPROUT_SWITCH_TLV_TYPE_UNIT_BASE_INFO      0x1
#if defined(VTSS_SPROUT_V1)
#define SPROUT_SWITCH_TLV_TYPE_UNIT_GLAG_MBR_CNT   0x2
#endif
#define SPROUT_SWITCH_TLV_TYPE_UPS_BASE_INFO       0x3
#define SPROUT_SWITCH_TLV_TYPE_SWITCH_BASE_INFO    0x4
#define SPROUT_SWITCH_TLV_TYPE_SWITCH_IP_ADDR      0x20 
#define SPROUT_SWITCH_TLV_TYPE_SWITCH_MST_ELECT    0x21 
#define SPROUT_SWITCH_TLV_TYPE_SWITCH_APPL_INFO    0x22 
#define SPROUT_SWITCH_TLV_TYPE_UPS_PORT_MASK       0x23 


#define SPROUT_SWITCH_TLV_UNIT_BASE_INFO_RSV_MASK         0x80 
#if defined(VTSS_SPROUT_V1)
#define SPROUT_SWITCH_TLV_UNIT_GLAG_MBR_CNT_RSV_MASK      0xe0 
#endif
#define SPROUT_SWITCH_TLV_UPS_BASE_INFO_RSV_MASK          0xe0 
#define SPROUT_SWITCH_TLV_SWITCH_BASE_INFO_RSV_MASK       0xfe 
#define SPROUT_SWITCH_TLV_SWITCH_MST_ELECT_BYTE4_RSV_MASK 0x78 

#define SPROUT_MAX_PDU_SIZE 1024 


static vtss_sprout_init_t sprout_init;
static BOOL               sprout_init_done = 0;
static BOOL               switch_init_done = 0;


typedef struct _internal_cfg_t {
    BOOL sprout_periodic_updates;
    BOOL sprout_triggered_updates;
    BOOL sprout_aging;
} internal_cfg_t;

static internal_cfg_t internal_cfg;


vtss_sprout__rit_t rit;
vtss_sprout__uit_t uit;


vtss_sprout__switch_state_t switch_state;

#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "sprout",
    .descr     = "SPROUT - Topology protocol (vtss_sprout.c)"
};

#ifndef VTSS_SPROUT_DEFAULT_TRACE_LVL
#define VTSS_SPROUT_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_PKT_DUMP] = {
        .name      = "pktdump",
        .descr     = "Hex dump of tx'ed & rx'ed SPROUT packets (lvl=noise)",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_TOP_CHG] = {
        .name      = "top_chg",
        .descr     = "Topology change (lvl=info)",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_MST_ELECT] = {
        .name      = "mstelect",
        .descr     = "Master election",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_UPSID] = {
        .name      = "upsid",
        .descr     = "upsid assignment",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical region enter/exit trace (lvl=racket)",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_FAILOVER] = {
        .name      = "failover",
        .descr     = "Selected failover related debug output with usec timestamp",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
        .usec      = 1,
        .ringbuf   = 1,
    },
    [TRACE_GRP_STARTUP] = {
        .name      = "startup",
        .descr     = "Selected startup related debug output",
        .lvl       = VTSS_SPROUT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
        .usec      = 0,
        .ringbuf   = 0,
    },
};
#endif 



static BOOL ignore_semaphores = 0;

#define CRIT_IGNORE(cmd) {ignore_semaphores = 1; cmd; ignore_semaphores = 1;}

#if defined(VTSS_SPROUT_V2)


#define SPROUT_ALERT_PROTODOWN_PDU_LEN (24+8) 
static vtss_vstax_tx_header_t sprout_alert_protodown_vs2_hdr;
static uchar                  sprout_alert_protodown_pkt[SPROUT_ALERT_PROTODOWN_PDU_LEN];
#endif










static vtss_rc process_xit_changes(
    BOOL allow_sprout_stack_port0,
    BOOL allow_sprout_stack_port1);

static char *dbg_info(void);

static void update_local_uis(void);

static vtss_rc stack_setup(void);



static vtss_rc topo_protocol_error(
    const char *fmt, ...)
{
    va_list ap;
    char s[200] = "SPROUT protocol error: ";
    int len;

    len = strlen(s);

    va_start(ap, fmt);

    vsprintf(s + len, fmt, ap);
    T_E("%s\n", s);

    if (sprout_init.callback.log_msg) {
        sprout_init.callback.log_msg(s);
    }

    va_end(ap);

    return VTSS_OK;
} 


static BOOL is_odd(uint n)
{
    if ((n % 2) == 1) {
        return 1;
    } else {
        return 0;
    }
}


#if defined(VTSS_SPROUT_V1)
static BOOL is_even(uint n)
{
    return !(is_odd(n));
}
#endif





static vtss_rc cfg_save(void)
{
    vtss_sprout_cfg_save_t  topo_cfg;
    vtss_sprout__unit_idx_t unit_idx;

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        vtss_vstax_upsid_t upsid = switch_state.unit[unit_idx].upsid;
        VTSS_SPROUT_ASSERT_DBG_INFO(VTSS_VSTAX_UPSID_LEGAL(upsid) ||
                                    upsid == VTSS_VSTAX_UPSID_UNDEF,
                                    (" "));
        topo_cfg.upsid[unit_idx] = upsid;
    }

    return sprout_init.callback.cfg_save(&topo_cfg);
} 




















static uint get_shortest_path(
    const vtss_sprout__unit_idx_t          src_unit_idx,
    const vtss_sprout_switch_addr_t *const dst_switch_addr_p,
    const vtss_sprout__unit_idx_t          dst_unit_idx)
{
    vtss_sprout__ri_t *ri_p;
    vtss_sprout_dist_t sp_dist[2];

#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT == 1
    VTSS_SPROUT_ASSERT(src_unit_idx == 0, ("Impossible"));
#endif

    if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                     dst_switch_addr_p) == 0) {
        
        if (src_unit_idx == dst_unit_idx) {
            
            return 2;
        } else {
            
            return 1;
        }
    }

    
    
    ri_p = vtss_sprout__ri_find(&rit,
                                dst_switch_addr_p,
                                dst_unit_idx);
    VTSS_SPROUT_ASSERT_DBG_INFO(ri_p != NULL, (" "));

    sp_dist[0] = ri_p->stack_port_dist[0];
    sp_dist[1] = ri_p->stack_port_dist[1];

    VTSS_SPROUT_ASSERT_DBG_INFO(
        !(sp_dist[0] == VTSS_SPROUT_DIST_INFINITY &&
          sp_dist[1] == VTSS_SPROUT_DIST_INFINITY),
        ("get_shortest_path: RI dist is INFINITY for both stack ports?!\nri=%s",
         vtss_sprout__ri_to_str(ri_p)));

    



    if (src_unit_idx == 1) {
        
        sp_dist[0] =
            (ri_p->stack_port_dist[1] == VTSS_SPROUT_DIST_INFINITY) ?
            VTSS_SPROUT_DIST_INFINITY :
            ri_p->stack_port_dist[1] - 1;
        
        sp_dist[1] =
            (ri_p->stack_port_dist[0] == VTSS_SPROUT_DIST_INFINITY) ?
            VTSS_SPROUT_DIST_INFINITY :
            ri_p->stack_port_dist[0] + 1;
    }


    if (sp_dist[0] != VTSS_SPROUT_DIST_INFINITY &&
        sp_dist[1] == VTSS_SPROUT_DIST_INFINITY) {
        
        return 0;
    } else if (sp_dist[0] == VTSS_SPROUT_DIST_INFINITY &&
               sp_dist[1] != VTSS_SPROUT_DIST_INFINITY) {
        
        return 1;
    } else if (switch_state.unit[src_unit_idx].stack_port[0].ttl_new >=
               sp_dist[0]) {
        
        return 0;
    } else if (switch_state.unit[src_unit_idx].stack_port[1].ttl_new >=
               sp_dist[1]) {
        
        return 1;
    } else {
        T_E("No shortest path found (src_u_idx=%u, dst_sa=%s, dst_u_idx=%u)?!\n%s",
            src_unit_idx,
            vtss_sprout_switch_addr_to_str(dst_switch_addr_p),
            dst_unit_idx,
            dbg_info());
#if 0
        T_E(vtss_sprout__rit_to_str(&rit));
        T_E("sp_dist[0/1]=%d/%d", sp_dist[0], sp_dist[1]);
        T_E("ri=%s", vtss_sprout__ri_to_str(ri_p));
#endif
        return 0;
    }
} 





static void topology_determination(void)
{
    




























    vtss_sprout__ri_t *ri_p;
    BOOL found_myself_unit0 = 0;
    BOOL found_myself_unit1 = 0;

    T_N("topology_determination");

    
    switch_state.topology_n = 0;
    ri_p = NULL;
    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
        switch_state.topology_n++;

        if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                         &ri_p->switch_addr) == 0) {
            if (ri_p->unit_idx == 0) {
                found_myself_unit0 = 1;
            } else {
                found_myself_unit1 = 1;
            }
        }
    }

    if (!found_myself_unit0) {
        switch_state.topology_n++;;
    }
    if (!found_myself_unit1 && vtss_board_chipcount() == 2) {
        switch_state.topology_n++;
    }


    if (switch_state.topology_n == 1) {
        switch_state.topology_type_new = VtssTopoOpenLoop;
        return;
    }
    if (switch_state.topology_n == 2) {
        while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
            if (ri_p->stack_port_dist[0] == 1 &&
                ri_p->stack_port_dist[1] == 1) {
                switch_state.topology_type_new = VtssTopoBack2Back;
#if defined(VTSS_SPROUT_V2)
                switch_state.topology_type_new = VtssTopoClosedLoop;
#endif
                return;
            }
        }

        
        
        switch_state.topology_type_new = VtssTopoOpenLoop;
        return;
    }
    
    if (switch_state.topology_n == 3) {
        vtss_sprout__ri_t *ri_p[2];

        ri_p[0] = vtss_sprout__ri_find_at_dist(&rit, 0, 1);
        ri_p[1] = vtss_sprout__ri_find_at_dist(&rit, 1, 1);

        if (ri_p[0] && ri_p[1] &&
            (vtss_sprout__switch_addr_cmp(&ri_p[0]->switch_addr,
                                          &ri_p[1]->switch_addr) == 0) &&
            ri_p[0]->tightly_coupled == 1 &&
            ri_p[1]->tightly_coupled == 1) {
            
            switch_state.topology_type_new = VtssTopoBack2Back;
#if defined(VTSS_SPROUT_V2)
            switch_state.topology_type_new = VtssTopoClosedLoop;
#endif
            return;
        }
    }

    
    VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.topology_n > 2, (" "));

    if (!found_myself_unit0) {
        switch_state.topology_type_new = VtssTopoOpenLoop;
        return;
    }
    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
        if (ri_p->stack_port_dist[0] == VTSS_SPROUT_DIST_INFINITY ||
            ri_p->stack_port_dist[1] == VTSS_SPROUT_DIST_INFINITY) {
            switch_state.topology_type_new = VtssTopoOpenLoop;
            return;
        }
    }
    

    switch_state.topology_type_new = VtssTopoClosedLoop;

    return;
} 


#if defined(VTSS_SPROUT_V1)







static void top_switch_calc(void)
{
    vtss_sprout__ri_t             *ri_p     = NULL;
    vtss_sprout__ui_t             *ui_p     = NULL;
    vtss_sprout__ui_t             *ui_top_p = NULL;
    vtss_sprout__stack_port_idx_t sp_idx;

    T_N("top_switch_calc: topology_type_new=%d, N=%d",
        switch_state.topology_type_new,
        switch_state.topology_n);

    
    VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.topology_n != 0,
                                (" "));

    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        if ((ui_p->unit_idx == 0) &&
            (ui_top_p == NULL ||
             vtss_sprout__switch_addr_cmp(&ui_top_p->switch_addr,
                                          &ui_p->switch_addr) == 1)) {
            
            ui_top_p = ui_p;
        }
    }

    
    VTSS_SPROUT_ASSERT_DBG_INFO(ui_top_p, (" "));

    
    switch_state.ui_top_p = ui_top_p;

    if (switch_state.topology_type_new == VtssTopoClosedLoop) {
        

        if (vtss_sprout__switch_addr_cmp(&ui_top_p->switch_addr,
                                         &switch_state.switch_addr) == 0) {
            
            vtss_sprout_switch_addr_t *nbr_sa_p[2];
            T_N("We are top!");
            nbr_sa_p[VTSS_SPROUT__SP_A] = &switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].nbr_switch_addr;
            nbr_sa_p[VTSS_SPROUT__SP_B] = &switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].nbr_switch_addr;

            if (vtss_sprout__switch_addr_cmp(nbr_sa_p[VTSS_SPROUT__SP_A], nbr_sa_p[VTSS_SPROUT__SP_B]) > 0) {
                
                switch_state.top_link = VTSS_SPROUT__SP_A;
            } else {
                switch_state.top_link = VTSS_SPROUT__SP_B;
            }
            switch_state.top_dist = switch_state.topology_n;
        } else {
            
            vtss_sprout_switch_addr_t *nbr_sa_p[2] = {NULL, NULL};
            vtss_sprout_switch_addr_t *top_nbr_sa_p = NULL;

            
            for (sp_idx = 0; sp_idx < 2; sp_idx++) {

                if (vtss_sprout__switch_addr_cmp(&switch_state.unit[0].stack_port[sp_idx].nbr_switch_addr,
                                                 &switch_state.ui_top_p->switch_addr) == 0) {
                    T_N("On sp_idx=%d, we are neighbour to top_switch!", sp_idx);
                    nbr_sa_p[sp_idx] = &switch_state.switch_addr;
                } else {
                    
                    vtss_sprout_dist_t dist_to_top =
                        vtss_sprout__get_dist(&rit,
                                              sp_idx,
                                              &switch_state.ui_top_p->switch_addr,
                                              0);
                    vtss_sprout_dist_t dist_nbr = VTSS_SPROUT_DIST_INFINITY;

                    VTSS_SPROUT_ASSERT_DBG_INFO(dist_to_top != VTSS_SPROUT_DIST_INFINITY, (" "));

                    
                    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
                        if (ri_p->unit_idx == 0) {
                            
                            vtss_sprout_dist_t dist;
                            dist = ri_p->stack_port_dist[sp_idx];

                            if (dist != VTSS_SPROUT_DIST_INFINITY &&
                                dist < dist_to_top) {
                                
                                
                                if (dist_nbr == VTSS_SPROUT_DIST_INFINITY ||
                                    dist_nbr < dist) {
                                    
                                    dist_nbr = dist;
                                    nbr_sa_p[sp_idx] = &ri_p->switch_addr;
                                }
                            }

                        }
                    } 

                    
                    VTSS_SPROUT_ASSERT_DBG_INFO(dist_nbr == dist_to_top - 1 ||
                                                dist_nbr == dist_to_top - 2,
                                                ("dist_nbr=%d, dist_to_top=%d",
                                                 dist_nbr, dist_to_top));
                }
            } 

            VTSS_SPROUT_ASSERT_DBG_INFO(nbr_sa_p[0] != NULL && nbr_sa_p[1] != NULL, (" "));

            
            VTSS_SPROUT_ASSERT_DBG_INFO(vtss_sprout__switch_addr_cmp(nbr_sa_p[0], nbr_sa_p[1]) != 0,
                                        ("nbr_sa_p[0]=%s",
                                         vtss_sprout_switch_addr_to_str(nbr_sa_p[0])));

            
            

            

            
            if (vtss_sprout__switch_addr_cmp(nbr_sa_p[VTSS_SPROUT__SP_A], nbr_sa_p[VTSS_SPROUT__SP_B]) < 0) {
                
                top_nbr_sa_p = nbr_sa_p[VTSS_SPROUT__SP_A];
            } else {
                
                top_nbr_sa_p = nbr_sa_p[VTSS_SPROUT__SP_B];
            }
            VTSS_SPROUT_ASSERT_DBG_INFO(top_nbr_sa_p != NULL, (" "));

            
            


            if (vtss_sprout__switch_addr_cmp(top_nbr_sa_p, &switch_state.switch_addr) == 0) {
                
                if (vtss_sprout__switch_addr_cmp(&switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].nbr_switch_addr,
                                                 &switch_state.ui_top_p->switch_addr) == 0) {
                    switch_state.top_link = VTSS_SPROUT__SP_A;
                } else {
                    VTSS_SPROUT_ASSERT_DBG_INFO(
                        vtss_sprout__switch_addr_cmp(&switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].nbr_switch_addr,
                                                     &switch_state.ui_top_p->switch_addr) == 0,
                        (" "));

                    switch_state.top_link = VTSS_SPROUT__SP_B;
                }
            } else {
                if (vtss_sprout__get_dist(&rit,
                                          VTSS_SPROUT__SP_A,
                                          &switch_state.ui_top_p->switch_addr,
                                          0)
                    >
                    vtss_sprout__get_dist(&rit,
                                          VTSS_SPROUT__SP_A,
                                          top_nbr_sa_p,
                                          0)) {
                    switch_state.top_link = VTSS_SPROUT__SP_A;
                } else {
                    switch_state.top_link = VTSS_SPROUT__SP_B;
                }
            }

            
            switch_state.top_dist =
                vtss_sprout__get_dist(&rit,
                                      switch_state.top_link,
                                      &switch_state.ui_top_p->switch_addr,
                                      0);
        }
    } else {
        
        switch_state.top_link     = 0;
        switch_state.top_dist     = VTSS_SPROUT_DIST_INFINITY;
    }
} 
#endif









static void ttl_calc(void)
{
    vtss_sprout__ri_t             *ri_p = NULL;
    vtss_sprout_dist_t            dist = 0;
#if defined(VTSS_SPROUT_V1)
    vtss_sprout_dist_t            top_dist = switch_state.top_dist;
#endif
    uint                          N = switch_state.topology_n;
    vtss_sprout__stack_port_idx_t sp_idx;
    uint                          ttl[2][2]; 
    vtss_sprout__unit_idx_t       unit_idx;

    VTSS_SPROUT_ASSERT_DBG_INFO(N != 0, (" "));

    T_N("ttl_calc: topology_type_new=%d, N=%d",
        switch_state.topology_type_new, N);

#if defined(VTSS_SPROUT_V1)
    VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.ui_top_p != NULL, (" "));
    T_N("ttl_calc: top_dist=%d", top_dist);
#endif

    switch (switch_state.topology_type_new) {
    case VtssTopoBack2Back:
#if defined(VTSS_SPROUT_V2)
        
        VTSS_SPROUT_ASSERT_DBG_INFO(0, (" "));
#endif

        if (switch_state.topology_n == 2) {
            ttl[0][VTSS_SPROUT__SP_A] = 1;
            ttl[0][VTSS_SPROUT__SP_B] = 1;
        } else {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.topology_n == 3, (" "));

            ttl[0][VTSS_SPROUT__SP_A] = 2;
            ttl[0][VTSS_SPROUT__SP_B] = 2;
        }
        break;

    case VtssTopoOpenLoop:
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            dist = 0;
            while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
                if (ri_p->stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY &&
                    vtss_sprout__switch_addr_cmp(&ri_p->switch_addr, &switch_state.switch_addr) != 0 &&
                    ri_p->stack_port_dist[sp_idx] > dist) {
                    dist = ri_p->stack_port_dist[sp_idx];
                }
            }
            ttl[0][sp_idx] = dist;

            if (vtss_board_chipcount() == 2 && sp_idx == VTSS_SPROUT__SP_B) {
                
                ttl[0][sp_idx] = MAX(dist, 1);
            }

            VTSS_SPROUT_ASSERT_DBG_INFO(dist < switch_state.topology_n,
                                        ("dist=%d, topology_n=%d, sp_idx=%d",
                                         dist, switch_state.topology_n, sp_idx));
        }

        
        
        ttl[1][VTSS_SPROUT__SP_A] = ttl[0][VTSS_SPROUT__SP_B] - 1;
        
        ttl[1][VTSS_SPROUT__SP_B] = ttl[0][VTSS_SPROUT__SP_A] + 1;

#if defined(VTSS_SPROUT_V2)
        if (ttl[0][VTSS_SPROUT__SP_A] + ttl[0][VTSS_SPROUT__SP_B] >= switch_state.topology_n) {
            
            
            
            

            T_I("Appears to be an OpenLoop becoming a ClosedLoop => Adjusting TTLs");

            if (is_odd(N)) {
                
                ttl[0][VTSS_SPROUT__SP_A] = (N - 1) / 2;
                ttl[0][VTSS_SPROUT__SP_B] = (N - 1) / 2;
                ttl[1][VTSS_SPROUT__SP_A] = (N - 1) / 2;
                ttl[1][VTSS_SPROUT__SP_B] = (N - 1) / 2;
            } else {
                ttl[0][VTSS_SPROUT__SP_A] = N / 2 - 1;
                ttl[0][VTSS_SPROUT__SP_B] = N / 2;

                ttl[1][VTSS_SPROUT__SP_A] = N / 2 - 1;
                ttl[1][VTSS_SPROUT__SP_B] = N / 2;
            }
        }
#else
        
        
        
        
        
        
#endif

        break;

    case VtssTopoClosedLoop:
        if (is_odd(N)) {
            
            ttl[0][VTSS_SPROUT__SP_A] = (N - 1) / 2;
            ttl[0][VTSS_SPROUT__SP_B] = (N - 1) / 2;
            ttl[1][VTSS_SPROUT__SP_A] = (N - 1) / 2;
            ttl[1][VTSS_SPROUT__SP_B] = (N - 1) / 2;
        } else {
            
            
#if defined(VTSS_SPROUT_V1)
            
            if (top_dist == N) {
                ttl[0][switch_state.top_link]  = N / 2;
                ttl[0][!switch_state.top_link] = N / 2 - 1;
            } else if (top_dist < N / 2 && is_even(top_dist)) {
                ttl[0][switch_state.top_link]  = N / 2;
                ttl[0][!switch_state.top_link] = N / 2 - 1;
            } else if (top_dist < N / 2 && is_odd(top_dist)) {
                ttl[0][switch_state.top_link]  = N / 2 - 1;
                ttl[0][!switch_state.top_link] = N / 2;
            } else if (top_dist >= N / 2 && is_even(top_dist) && is_even(N / 2)) {
                ttl[0][switch_state.top_link]  = N / 2 - 1;
                ttl[0][!switch_state.top_link] = N / 2;
            } else if (top_dist >= N / 2 && is_odd(top_dist) && is_even(N / 2)) {
                ttl[0][switch_state.top_link]  = N / 2;
                ttl[0][!switch_state.top_link] = N / 2 - 1;
            } else if (top_dist >= N / 2 && is_even(top_dist) && is_odd(N / 2)) {
                ttl[0][switch_state.top_link]  = N / 2;
                ttl[0][!switch_state.top_link] = N / 2 - 1;
            } else if (top_dist >= N / 2 && is_odd(top_dist) && is_odd(N / 2)) {
                ttl[0][switch_state.top_link]  = N / 2 - 1;
                ttl[0][!switch_state.top_link] = N / 2;
            } else {
                VTSS_SPROUT_ASSERT_DBG_INFO(0, (" "));
            }
#else
            
            
            
            ttl[0][VTSS_SPROUT__SP_A] = N / 2 - 1;
            ttl[0][VTSS_SPROUT__SP_B] = N / 2;

            ttl[1][VTSS_SPROUT__SP_A] = N / 2 - 1;
            ttl[1][VTSS_SPROUT__SP_B] = N / 2;
#endif
        }

#if defined(VTSS_SPROUT_V1)
        
        if (switch_state.topology_n > 3) {
            vtss_sprout__ri_t *ri_p[2];

            ri_p[0] = vtss_sprout__ri_find_at_dist(&rit, 0, ttl[0][0]);
            ri_p[1] = vtss_sprout__ri_find_at_dist(&rit, 1, ttl[0][1]);

            if (ri_p[0] && ri_p[1] &&
                (vtss_sprout__switch_addr_cmp(&ri_p[0]->switch_addr,
                                              &ri_p[1]->switch_addr) == 0) &&
                ri_p[0]->tightly_coupled == 1 &&
                ri_p[1]->tightly_coupled == 1) {
                
                if (ttl[0][0] < ttl[0][1]) {
                    ttl[0][0]++;
                    ttl[0][1]--;
                } else if (ttl[0][0] > ttl[0][1]) {
                    ttl[0][0]--;
                    ttl[0][1]++;
                } else {
                    
                    ttl[0][switch_state.top_link]--;
                    ttl[0][!switch_state.top_link]++;
                }
            }
        }
#endif

        break;
    default:
        
        VTSS_SPROUT_ASSERT_DBG_INFO(0, (" "));
    }

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        if (switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_A].ttl != ttl[unit_idx][VTSS_SPROUT__SP_A]) {
            switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_A].cfg_change = 1;
            switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_A].ttl_new    = ttl[unit_idx][VTSS_SPROUT__SP_A];
        }
        if (switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_B].ttl != ttl[unit_idx][VTSS_SPROUT__SP_B]) {
            switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_B].cfg_change = 1;
            switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_B].ttl_new    = ttl[unit_idx][VTSS_SPROUT__SP_B];
        }
    }
} 






static void mirror_calc(void)
{
    vtss_sprout__stack_port_state_t *sps_pa[VTSS_SPROUT_MAX_LOCAL_CHIP_CNT][2];
    vtss_sprout__ui_t               *ui_p     = NULL;
    BOOL                            mirror_fwd_new[VTSS_SPROUT_MAX_LOCAL_CHIP_CNT][2];
    uint                            sp_idx;

    vtss_sprout__unit_idx_t unit_idx;
    memset(mirror_fwd_new, 0, sizeof(mirror_fwd_new));

    
    sps_pa[0][0] = &switch_state.unit[0].stack_port[0];
    sps_pa[0][1] = &switch_state.unit[0].stack_port[1];
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
    sps_pa[1][0] = &switch_state.unit[1].stack_port[0];
    sps_pa[1][1] = &switch_state.unit[1].stack_port[1];
#endif

    

    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        if (ui_p->have_mirror) {
            uint sp_path; 

            switch (switch_state.topology_type_new) {
            case VtssTopoBack2Back:
                VTSS_SPROUT_ASSERT_DBG_INFO(vtss_board_chipcount() == 1, (" "));
                sp_path =
                    get_shortest_path(
                        0,
                        &ui_p->switch_addr,
                        ui_p->unit_idx);
                if (sp_path != 2) {
                    
                    mirror_fwd_new[0][0] = 1;
                    mirror_fwd_new[0][1] = 1;
                }
                break;

            case VtssTopoOpenLoop:
            case VtssTopoClosedLoop:
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
                for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
#else
                for (unit_idx = 0; unit_idx < 1; unit_idx++) {
#endif
                    sp_path =
                        get_shortest_path(
                            unit_idx,
                            &ui_p->switch_addr,
                            ui_p->unit_idx);
                    if (sp_path != 2) {
                        mirror_fwd_new[unit_idx][sp_path] = 1;
                    }
                }
                break;

            default:
                
                VTSS_SPROUT_ASSERT_DBG_INFO(0, (" "));
            }
        }
    }



    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            if (sps_pa[unit_idx][sp_idx]->mirror_fwd != mirror_fwd_new[unit_idx][sp_idx]) {
                sps_pa[unit_idx][sp_idx]->cfg_change = 1;
                sps_pa[unit_idx][sp_idx]->mirror_fwd = mirror_fwd_new[unit_idx][sp_idx];
            }
        }
    }
} 










static ulong my_mst_time(void)
{
    if (switch_state.mst) {
        return (sprout_init.callback.secs_since_boot() - switch_state.mst_start_time);
    } else {
        return 0;
    }
} 






static BOOL mst_calc(void)
{
    
    vtss_sprout__ui_t             *best_mst_ui_p = NULL;
    
    ulong                         best_mst_time  = 0;

    vtss_sprout__ui_t             *ui_p          = NULL;
    BOOL                          mst_time_ignore = 0;
    int                           n = 0;

#if VTSS_SPROUT_UNMGD
    return 0;
#endif

    T_DG(TRACE_GRP_MST_ELECT, "enter");
    T_DG(TRACE_GRP_MST_ELECT, "Current master: %s",
         vtss_sprout_switch_addr_to_str(&switch_state.mst_switch_addr));

    
    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        mst_time_ignore |= ui_p->mst_time_ignore;
        n++;
    }

    T_DG(TRACE_GRP_MST_ELECT, "%d unit(s) in stack. mst_time_ignore=%d",
         n, mst_time_ignore);

    
    ui_p = NULL;
    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        ulong mst_time   = 0;
        BOOL  local_unit = 0;

        
        if (ui_p->unit_idx != 0) {
            continue;
        }

        
        if (!ui_p->mst_capable) {
            continue;
        }

        local_unit =
            vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                         &ui_p->switch_addr) == 0;

        if (local_unit) {
            
            mst_time = my_mst_time();
        } else {
            
            mst_time = ui_p->mst_time;
        }

        if (best_mst_ui_p == NULL) {
            best_mst_ui_p = ui_p;
            best_mst_time = mst_time;
            T_DG(TRACE_GRP_MST_ELECT,
                 "1st master candidate: %s (mst_time=%u, elect_prio=%d)",
                 vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr),
                 best_mst_time,
                 best_mst_ui_p->mst_elect_prio);
            continue;
        }

        if (!mst_time_ignore) {
            
            if (mst_time >= VTSS_SPROUT_MST_TIME_MIN &&
                mst_time > best_mst_time + VTSS_SPROUT_MST_TIME_DIFF_MIN) {
                
                best_mst_ui_p = ui_p;
                best_mst_time = mst_time;
                T_DG(TRACE_GRP_MST_ELECT,
                     "New master candidate due to mst_time: %s (mst_time=%u, elect_prio=%d)",
                     vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr),
                     mst_time,
                     ui_p->mst_elect_prio);

                continue;
            }
            if (best_mst_time >= VTSS_SPROUT_MST_TIME_MIN) {
                if (mst_time < VTSS_SPROUT_MST_TIME_MIN) {
                    T_DG(TRACE_GRP_MST_ELECT,
                         "Skipping master candidate due to mst_time <min: %s (mst_time=%u)",
                         vtss_sprout_switch_addr_to_str(&ui_p->switch_addr), mst_time);
                    continue;
                }
                if (best_mst_time > mst_time + VTSS_SPROUT_MST_TIME_DIFF_MIN) {
                    T_DG(TRACE_GRP_MST_ELECT,
                         "Skipping master candidate due to mst_time: %s (mst_time=%u)",
                         vtss_sprout_switch_addr_to_str(&ui_p->switch_addr), mst_time);
                    continue;
                }
            }

            if (best_mst_time > 0 &&
                ui_p->mst_time > 0) {
                
                
                T_EXPLICIT(VTSS_TRACE_MODULE_ID,
                           TRACE_GRP_MST_ELECT,
                           VTSS_TRACE_LVL_INFO,
                           __FUNCTION__,
                           __LINE__,

                           "Two units have master time larger than min(!)\n"
                           "Switch addr=%s\n"
                           "  Master time: %u\n"
                           "  Master elect prio: %d\n"
                           "Switch addr=%s\n"
                           "  Master time: %u\n"
                           "  Master elect prio: %d\n"
                           "Current master addr=%s",

                           vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr),
                           best_mst_time,
                           best_mst_ui_p->mst_elect_prio,

                           vtss_sprout_switch_addr_to_str(&ui_p->switch_addr),
                           mst_time,
                           ui_p->mst_elect_prio,

                           vtss_sprout_switch_addr_to_str(&switch_state.mst_switch_addr));
            }
        }

        
        if (ui_p->mst_elect_prio < best_mst_ui_p->mst_elect_prio) {
            
            best_mst_ui_p = ui_p;
            best_mst_time = mst_time;
            T_DG(TRACE_GRP_MST_ELECT,
                 "New master candidate due to elect_prio: %s (mst_time=%u, elect_prio=%d)",
                 vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr),
                 mst_time,
                 ui_p->mst_elect_prio);
            continue;
        } else if (ui_p->mst_elect_prio > best_mst_ui_p->mst_elect_prio) {
            T_DG(TRACE_GRP_MST_ELECT,
                 "Skipping master candidate due to elect_prio: %s",
                 vtss_sprout_switch_addr_to_str(&ui_p->switch_addr));
            continue;
        }

        
        if (vtss_sprout__switch_addr_cmp(&ui_p->switch_addr,
                                         &best_mst_ui_p->switch_addr) < 0) {
            
            best_mst_ui_p = ui_p;
            best_mst_time = mst_time;
            T_DG(TRACE_GRP_MST_ELECT,
                 "New master candidate due to switch_addr: %s",
                 vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr));
            continue;
        }
    }

    if (!best_mst_ui_p) {
        
        if (vtss_sprout__switch_addr_cmp(&switch_state.mst_switch_addr,
                                         &switch_addr_undef) == 0) {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(
                (switch_state.mst_upsid == VTSS_VSTAX_UPSID_UNDEF),
                ("%s", vtss_sprout__switch_state_to_str(&switch_state)));
            return 0;
        } else {
            
            T_IG(TRACE_GRP_MST_ELECT,
                 "No master found\n"
                 "Previuos master was: %s\n"
                 "Prevoius master UPSID was: %d",
                 vtss_sprout_switch_addr_to_str(&switch_state.mst_switch_addr),
                 switch_state.mst_upsid);
            switch_state.mst_change_time = sprout_init.callback.secs_since_boot();
            switch_state.mst_switch_addr = switch_addr_undef;
            switch_state.mst_upsid       = VTSS_VSTAX_UPSID_UNDEF;
            switch_state.mst = 0;

            vtss_vstax_master_upsid_set(NULL, VTSS_VSTAX_UPSID_UNDEF);

            return 1;
        }
    } else if (vtss_sprout__switch_addr_cmp(&best_mst_ui_p->switch_addr,
                                            &switch_state.mst_switch_addr)) {
        
        T_IG(TRACE_GRP_MST_ELECT,
             "New master found: %s\n"
             "Master UPSID: %d\n"
             "Master time: %u\n"
             "Master elect prio: %d",
             vtss_sprout_switch_addr_to_str(&best_mst_ui_p->switch_addr),
             best_mst_ui_p->upsid[0],
             best_mst_time,
             best_mst_ui_p->mst_elect_prio);

        switch_state.mst_change_time = sprout_init.callback.secs_since_boot();
        switch_state.mst_switch_addr = best_mst_ui_p->switch_addr;
        switch_state.mst_upsid       = best_mst_ui_p->upsid[0];

        if (vtss_sprout__switch_addr_cmp(&switch_state.mst_switch_addr,
                                         &switch_state.switch_addr) == 0) {
            
            switch_state.mst = 1;
            switch_state.mst_start_time = sprout_init.callback.secs_since_boot();
            T_IG(TRACE_GRP_MST_ELECT,
                 "We have become master!\n"
                 "switch_state.mst_start_time=%u",
                 switch_state.mst_start_time);
        } else {
            if (switch_state.mst) {
                T_IG(TRACE_GRP_MST_ELECT, "We are no longer master!");
                switch_state.mst = 0;
            }
        }

        vtss_vstax_master_upsid_set(NULL, switch_state.mst_upsid);

        return 1; 
    } else {
        

        
        if (switch_state.mst_upsid != best_mst_ui_p->upsid[0]) {
            T_IG(TRACE_GRP_MST_ELECT,
                 "No master change, but master's UPSID has changed:\n"
                 "%d -> %d",
                 switch_state.mst_upsid, best_mst_ui_p->upsid[0]);

            switch_state.mst_upsid = best_mst_ui_p->upsid[0];
            vtss_vstax_master_upsid_set(NULL, switch_state.mst_upsid);
        }

        return 0;
    }
} 















static vtss_vstax_upsid_t upsid_get_random_unused(
    BOOL                           *upsid_inuse_mask,
    const BOOL                     require_even_upsid)
{
    static BOOL        srand_called = 0;
    vtss_vstax_upsid_t upsid_start, upsid;
    int                i;

    if (!srand_called) {
        




        srand((switch_state.switch_addr.addr[0] <<  8 | switch_state.switch_addr.addr[1] <<  0) ^
              (switch_state.switch_addr.addr[2] << 24 | switch_state.switch_addr.addr[3] << 16 |
               switch_state.switch_addr.addr[4] <<  8 | switch_state.switch_addr.addr[5] <<  0));
    }

    
    upsid_start = rand() % (VTSS_VSTAX_UPSID_MAX + 1);

    
    i = 0;
    for (i = 0; i <= VTSS_VSTAX_UPSID_MAX; i++) {
        upsid = (upsid_start + i) % (VTSS_VSTAX_UPSID_MAX + 1);
        if (upsid_inuse_mask[upsid]) {
            continue;
        }
        if (require_even_upsid && is_odd(upsid)) {
            continue;
        }

        upsid_inuse_mask[upsid] = 1;
        return upsid;
    };

    return VTSS_VSTAX_UPSID_UNDEF;
} 









static vtss_rc upsids_set_random_unused(
    BOOL *upsid_inuse_mask)
{
    vtss_vstax_upsid_t upsid;

    
    upsid = upsid_get_random_unused(upsid_inuse_mask, 1);
    if (upsid == VTSS_VSTAX_UPSID_UNDEF) {
        T_I("UPSID depletion encountered");
        return SPROUT_RC_UPSID_DEPLETION;
    }
    switch_state.unit[0].upsid = upsid;

#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
    
    if (vtss_board_chipcount() == 2) {
        switch_state.unit[1].upsid = upsid + 1;
    }
#endif

    return VTSS_OK;
} 







static vtss_rc upsids_initial_calc(
    
    BOOL *upsid_inuse_mask
)
{
    vtss_sprout__unit_idx_t unit_idx;
    vtss_rc                 rc = VTSS_OK;
    BOOL                    upsids_changed = 0;

    if (upsid_inuse_mask[switch_state.unit[0].upsid]) {
        if (upsids_set_random_unused(upsid_inuse_mask) == SPROUT_RC_UPSID_DEPLETION) {
            T_I("UPSID depletion encountered");
            return SPROUT_RC_UPSID_DEPLETION;
        } else {
            upsids_changed = 1;
        }
    } else {
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
        if (vtss_board_chipcount() == 2) {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(
                !upsid_inuse_mask[switch_state.unit[1].upsid], (" "));
        }
#endif
    }


    
    if ((rc = stack_setup()) < 0) {
        return rc;
    }

    update_local_uis();

    
    if (upsids_changed) {

        uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_LOCAL;
        uit.changed      = 1;
        rit.changed      = 1;

        T_DG(TRACE_GRP_UPSID, "Calling cfg_save()");

        if ((rc = cfg_save()) < 0) {
            return rc;
        }
    }

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        T_DG(TRACE_GRP_UPSID, "unit[%d].upsid=%d",
             unit_idx, switch_state.unit[unit_idx].upsid);
    }

    return VTSS_OK;
} 







static vtss_rc upsids_chk_and_recalc(
    BOOL *upsids_changed_p)
{
    vtss_sprout__ui_t               *ui_p = NULL;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_sprout__unit_idx_t         unit_idx;
    vtss_vstax_upsid_t              upsid;
    vtss_sprout__stack_port_idx_t   sp_idx;
    vtss_sprout__uit_t              *uit_p;
    BOOL                            upsid_inuse_mask[VTSS_SPROUT_MAX_UNITS_IN_STACK];
    BOOL                            upsid_recalc_needed = 0;
    vtss_rc                         rc = VTSS_OK;

    *upsids_changed_p = 0;

    memset(upsid_inuse_mask, 0, sizeof(upsid_inuse_mask));

    
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        VTSS_SPROUT_ASSERT_DBG_INFO(
            switch_state.unit[unit_idx].upsid != VTSS_VSTAX_UPSID_UNDEF, (" "));
        upsid_inuse_mask[switch_state.unit[unit_idx].upsid] = 1;
    }

    
    upsid_recalc_needed = 0;
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        upsid = switch_state.unit[unit_idx].upsid;

        
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

            if (sps_p->proto_up) {
                uit_p = &switch_state.unit[unit_idx].stack_port[sp_idx].uit;
                while ((ui_p = vtss_sprout__ui_get_nxt(uit_p, ui_p))) {
                    if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                                     &ui_p->switch_addr) == 0) {
                        continue;
                    }

                    if (ui_p->upsid[0] != VTSS_VSTAX_UPSID_UNDEF) {
                        upsid_inuse_mask[ui_p->upsid[0]] = 1;
                    }
                    if (ui_p->upsid[1] != VTSS_VSTAX_UPSID_UNDEF) {
                        upsid_inuse_mask[ui_p->upsid[1]] = 1;
                    }
                    if (upsid == ui_p->upsid[0] || upsid == ui_p->upsid[1]) {
                        
                        T_DG(TRACE_GRP_UPSID, "Conflict with own UPSID=%d found", upsid);
                        if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                                         &ui_p->switch_addr) > 0) {
                            
                            T_DG(TRACE_GRP_UPSID, "Lost UPSID=%d", upsid);
                            upsid_recalc_needed = 1;
                        } else {
                            T_DG(TRACE_GRP_UPSID, "We won. Will keep UPSID=%d", upsid);
                        }
                    }
                }
            }
        }
    }

    if (upsid_recalc_needed) {
        
        if (upsids_set_random_unused(upsid_inuse_mask) == SPROUT_RC_UPSID_DEPLETION) {
            T_I("UPSID depletion encountered");
            return SPROUT_RC_UPSID_DEPLETION;
        }

        *upsids_changed_p = 1;
    }

    if (*upsids_changed_p) {
        rc = stack_setup(); 

        uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_LOCAL;
        uit.changed      = 1;
        rit.changed      = 1;

        T_DG(TRACE_GRP_UPSID, "Calling cfg_save()");

        
        if ((rc = cfg_save()) < 0) {
            return rc;
        }
    }

    return rc;
} 











static void pkt_add_byte(
    uchar            *pkt_p,
    uint             *i_p,
    const uchar       byte)
{
    pkt_p[*i_p] = byte;
    *i_p += 1;

    VTSS_SPROUT_ASSERT_DBG_INFO(*i_p < SPROUT_MAX_PDU_SIZE, (" "));
} 



static void pkt_add_bytes(
    uchar            *pkt_p,
    uint             *i_p,
    const uchar      *bytes,
    uint              byte_cnt)
{
    uint i = 0;

    for (i = 0; i < byte_cnt; i++) {
        pkt_add_byte(pkt_p, i_p, bytes[i]);
    }
} 


static void pkt_add_1byte_tlv(
    uchar                        *pkt_p,
    uint                         *i_p,
    const uchar                  type,
    const vtss_sprout__unit_idx_t  unit_idx,
    const vtss_sprout__upsid_idx_t ups_idx,
    const uchar                  val)
{
    uchar byte;

    VTSS_SPROUT_ASSERT(type < 32, ("type=%d", type));
    VTSS_SPROUT_ASSERT(unit_idx <= 1, ("unit_idx=%d", unit_idx));
    VTSS_SPROUT_ASSERT(ups_idx  <= 1, ("ups_idx=%d", ups_idx));

    byte = ((type << 2) | (unit_idx << 1) | ups_idx);
    pkt_add_byte(pkt_p, i_p, byte);
    pkt_add_byte(pkt_p, i_p, val);
} 


static void pkt_add_nbyte_tlv(
    uchar                          *pkt_p,
    uint                           *i_p,
    const uchar                    type,
    const vtss_sprout__unit_idx_t  unit_idx,
    const vtss_sprout__upsid_idx_t ups_idx,
    const uchar                    len,
    const uchar                    *val)
{
    uchar byte;
    int i;

    VTSS_SPROUT_ASSERT(type >= 32, ("type=%d", type));
    VTSS_SPROUT_ASSERT(unit_idx <= 1, ("unit_idx=%d", unit_idx));
    VTSS_SPROUT_ASSERT(ups_idx  <= 1, ("ups_idx=%d", ups_idx));
    VTSS_SPROUT_ASSERT(len > 1, ("len=%d", len));

    byte = ((type << 2) | (unit_idx << 1) | ups_idx);
    pkt_add_byte(pkt_p, i_p, byte);
    pkt_add_byte(pkt_p, i_p, len);
    for (i = 0; i < len; i++) {
        pkt_add_byte(pkt_p, i_p, val[i]);
    }
} 



static void pkt_add_mac(
    uchar            *pkt_p,
    uint             *i_p,
    const vtss_mac_t *mac_p)
{
    pkt_add_bytes(pkt_p, i_p, mac_p->addr, 6);
} 



static void pkt_init_sprout_hdr(
    uchar            *pkt_p,
    uint             *i_p,
    uchar            sprout_pdu_type)

{
    
    pkt_add_mac(pkt_p, i_p, &sprout_dmac);
#if VTSS_SPROUT_DEBUG_DMAC_INCR
    sprout_dmac.addr[5]++;
#endif
    pkt_add_mac(pkt_p, i_p, &sprout_smac);

    
    VTSS_SPROUT_ASSERT((sprout_smac.addr[0] & 0x01) == 0,
                       ("sprout_smac.addr[0]=%02x", sprout_smac.addr[0]));

    
    pkt_add_bytes(pkt_p, i_p, exbit_protocol_ssp, 4);
    pkt_add_bytes(pkt_p, i_p, ssp_sprout,         4);

    
    pkt_add_bytes(pkt_p, i_p, &sprout_pdu_type,   1);
    pkt_add_bytes(pkt_p, i_p, &sprout_ver,        1);
    pkt_add_bytes(pkt_p, i_p, zeros,              2);
} 


static void pkt_init_vs2_hdr(
    vtss_vstax_tx_header_t *vs2_hdr_p,
    vtss_vstax_ttl_t        ttl,
    BOOL                    keep_ttl)
{
    memset(vs2_hdr_p, 0, sizeof(vtss_vstax_tx_header_t));
    vs2_hdr_p->fwd_mode    = VTSS_VSTAX_FWD_MODE_CPU_ALL;
    vs2_hdr_p->ttl         = ttl;
    vs2_hdr_p->prio        = VTSS_PRIO_SUPER;
    vs2_hdr_p->upsid       = 1;
    vs2_hdr_p->tci.vid     = 1;
    vs2_hdr_p->tci.cfi     = 0;
    vs2_hdr_p->tci.tagprio = 0;
    vs2_hdr_p->port_no     = 0;
    vs2_hdr_p->glag_no     = VTSS_GLAG_NO_NONE;
    vs2_hdr_p->queue_no    = switch_state.cpu_qu;
#if defined(VTSS_SPROUT_V2)
    vs2_hdr_p->keep_ttl    = keep_ttl;
#endif
} 


#if defined(VTSS_SPROUT_V2)

static void pkt_change_ttl(
    vtss_vstax_tx_header_t *vs2_hdr_p,
    vtss_vstax_ttl_t        ttl)
{
    vs2_hdr_p->ttl = ttl;
} 
#endif







static void pkt_add_switch_section(
    const vtss_sprout_switch_addr_t     *switch_addr_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    uchar                             *pkt_p,
    uint                              *i_p)
{
    vtss_sprout_dist_t       dist[2] = { VTSS_SPROUT_DIST_INFINITY, VTSS_SPROUT_DIST_INFINITY };
    uint                     unit_cnt;
    vtss_sprout__ri_t        *ri_p;
    vtss_sprout__ui_t        *ui_p[2] = { NULL, NULL };
    vtss_sprout__unit_idx_t  unit_idx;
    vtss_sprout__upsid_idx_t ups_idx;
    BOOL                     local_switch = 0;
    vtss_sprout__uit_t       *uit_p;
    uchar                    val;
    BOOL                     sp_int = 0;
    int                      j;
    uint  len_i; 

    sp_int = (vtss_board_chipcount() == 2) && (sp_idx == VTSS_SPROUT__SP_B);

    local_switch = (vtss_sprout__switch_addr_cmp(switch_addr_p,
                                                 &switch_state.switch_addr) == 0);

    
    pkt_add_byte(pkt_p, i_p, SPROUT_SECTION_TYPE_SWITCH);

    
    len_i = *i_p;
    *i_p += 1;

    
    pkt_add_bytes(pkt_p, i_p, switch_addr_p->addr, 6);

    
    if (vtss_sprout__switch_addr_cmp(switch_addr_p,
                                     &switch_state.switch_addr) == 0) {
        unit_cnt = vtss_board_chipcount();
    } else {
        if (vtss_sprout__ri_find(&rit, switch_addr_p, 1)) {
            
            unit_cnt = 2;
        } else {
            unit_cnt = 1;
        }
    }

    
    if (local_switch) {
        if (unit_cnt == 1) {
            
            dist[0] = 0;
        } else {
            VTSS_SPROUT_ASSERT_DBG_INFO(unit_cnt == 2, (" "));

            if (!sp_int) {
                
                dist[0] = 0;
                dist[1] = 1;
            } else {
                
                dist[0] = 1;
                dist[1] = 0;
            }
        }
    } else {
        
        for (unit_idx = 0; unit_idx < 2; unit_idx++) {
            ri_p = vtss_sprout__ri_find(&rit, switch_addr_p, unit_idx);
            VTSS_SPROUT_ASSERT_DBG_INFO(unit_idx == 1 || ri_p != NULL, (" "));

            if (ri_p) {
                dist[unit_idx] = ri_p->stack_port_dist[(sp_idx + 1) % 2];

                if (sp_int) {
                    
                    dist[unit_idx]++;
                }
            }
        }
    }

    
    if (local_switch) {
        uit_p = &uit;
    } else {
        uit_p = &switch_state.unit[0].stack_port[(sp_idx + 1) % 2].uit;
    }

    
    for (unit_idx = 0; unit_idx < 2; unit_idx++) {
        if (dist[unit_idx] != VTSS_SPROUT_DIST_INFINITY) {
            ui_p[unit_idx] = vtss_sprout__ui_find(uit_p, switch_addr_p, unit_idx);
            VTSS_SPROUT_ASSERT_DBG_INFO(ui_p[unit_idx] != NULL, ("switch_addr=%s\n%s",
                                                                 vtss_sprout_switch_addr_to_str(switch_addr_p),
                                                                 " "));
        } else {
            ui_p[unit_idx] = NULL;
        }
    }

    
    for (unit_idx = 0; unit_idx < 2; unit_idx++) {
        if (dist[unit_idx] != VTSS_SPROUT_DIST_INFINITY) {
            val = ((ui_p[unit_idx]->primary_unit << 6) |
                   (ui_p[unit_idx]->have_mirror << 5) | dist[unit_idx] |
                   ui_p[unit_idx]->unit_base_info_rsv);
            pkt_add_1byte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_UNIT_BASE_INFO,
                              unit_idx, 0, val);
        }
    }

#if defined(VTSS_SPROUT_V1)
    {
        uint glag_id;
        
        for (unit_idx = 0; unit_idx < 2; unit_idx++) {
            if (dist[unit_idx] != VTSS_SPROUT_DIST_INFINITY) {
                for (glag_id = 0; glag_id < VTSS_GLAGS; glag_id++) {
                    if (ui_p[unit_idx]->glag_mbr_cnt[glag_id] > 0) {
                        val = ((glag_id << 4) | (ui_p[unit_idx]->glag_mbr_cnt[glag_id] - 1) |
                               ui_p[unit_idx]->unit_glag_mbr_cnt_rsv[glag_id]);
                        pkt_add_1byte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_UNIT_GLAG_MBR_CNT,
                                          unit_idx, 0, val);
                    }
                }
            }
        }
    }
#endif

    
    for (unit_idx = 0; unit_idx < 2; unit_idx++) {
        if (dist[unit_idx] != VTSS_SPROUT_DIST_INFINITY) {
            for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                if (ui_p[unit_idx]->upsid[ups_idx] != VTSS_VSTAX_UPSID_UNDEF) {
                    val = (ui_p[unit_idx]->upsid[ups_idx] |
                           ui_p[unit_idx]->ups_base_info_rsv[ups_idx]);
                    pkt_add_1byte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_UPS_BASE_INFO,
                                      unit_idx, ups_idx, val);
                }
            }
        }
    }

    
    {
        u64            ups_port_mask;
        uchar          val[8];

        
        VTSS_SPROUT_ASSERT(VTSS_PORT_ARRAY_SIZE <= 64, ("%d", VTSS_PORT_ARRAY_SIZE));

        for (unit_idx = 0; unit_idx < 2; unit_idx++) {
            if (dist[unit_idx] != VTSS_SPROUT_DIST_INFINITY) {
                for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                    if (ui_p[unit_idx]->upsid[ups_idx] != VTSS_VSTAX_UPSID_UNDEF) {
                        
                        ups_port_mask = ui_p[unit_idx]->ups_port_mask[ups_idx];

                        
                        for (j = 0; j < 8; j++) {
                            val[7 - j] = (ups_port_mask >> (j * 8)) & 0xff;
                        }
                        pkt_add_nbyte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_UPS_PORT_MASK,
                                          unit_idx, ups_idx, 8, val);
                    }
                }
            }
        }
    }

    
    {
        val = (ui_p[0]->tightly_coupled |
               ui_p[0]->switch_base_info_rsv);
        pkt_add_1byte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_SWITCH_BASE_INFO,
                          0, 0, val);
    }

    
    {
        if (ui_p[0]->ip_addr != 0) {
            uchar ip_addr[4]; 
            ip_addr[0] = (ui_p[0]->ip_addr & 0xff000000) >> 24;
            ip_addr[1] = (ui_p[0]->ip_addr & 0x00ff0000) >> 16;
            ip_addr[2] = (ui_p[0]->ip_addr & 0x0000ff00) >>  8;
            ip_addr[3] = (ui_p[0]->ip_addr & 0x000000ff) >>  0;

            pkt_add_nbyte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_SWITCH_IP_ADDR, 0, 0, 4, ip_addr);
        }
    }

    
    {
        uchar val[5];
        ulong mst_time;

        if (local_switch) {
            
            if (switch_state.mst_capable) {
                mst_time = my_mst_time();
                val[4] = ((switch_state.mst_time_ignore << 7) |
                          (switch_state.mst_capable << 2) |
                          switch_state.mst_elect_prio);
            } else {
                mst_time = 0;
                val[4] = ((switch_state.mst_time_ignore << 7) |
                          (switch_state.mst_capable << 2) |
                          0);
            }
            T_N("mst_time=%u", mst_time);
        } else {
            mst_time = ui_p[0]->mst_time;
            val[4] = ((ui_p[0]->mst_time_ignore << 7) |
                      (ui_p[0]->mst_capable << 2) |
                      ui_p[0]->mst_elect_prio |
                      ui_p[0]->switch_mst_elect_rsv);
        }
        val[0] = (mst_time & 0xff000000) >> 24;
        val[1] = (mst_time & 0x00ff0000) >> 16;
        val[2] = (mst_time & 0x0000ff00) >> 8;
        val[3] = (mst_time & 0x000000ff) >> 0;
        T_N("val=%02x%02x%02x%02x%02x",
            val[0], val[1], val[2], val[3], val[4]);
        pkt_add_nbyte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_SWITCH_MST_ELECT,
                          0, 0, 5, val);
    }

    
    
    if (memcmp(ui_p[0]->switch_appl_info, zeros, VTSS_SPROUT_SWITCH_APPL_INFO_LEN) != 0) {
        pkt_add_nbyte_tlv(pkt_p, i_p, SPROUT_SWITCH_TLV_TYPE_SWITCH_APPL_INFO, 0, 0,
                          VTSS_SPROUT_SWITCH_APPL_INFO_LEN, ui_p[0]->switch_appl_info);
    }

    
    pkt_p[len_i] = (*i_p - len_i - 1);
} 





static void tx_sprout_update(
    const vtss_sprout__stack_port_idx_t sp_idx,   
    const BOOL                          periodic
)
{
    uchar pkt_buf[SPROUT_MAX_PDU_SIZE];
    uchar *pkt_p;
    uint  byte_idx = 0;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_sprout__ri_t               *ri_p = NULL;
    vtss_vstax_tx_header_t          vs2_hdr;
    vtss_rc                         rc = VTSS_OK;

    pkt_p = pkt_buf;

    sps_p = &switch_state.unit[0].stack_port[sp_idx];

    T_N("tx_sprout_update: sp_idx=%d, adm_up=%d, link_up=%d, proto_up=%d",
        sp_idx,
        sps_p->adm_up, sps_p->link_up, sps_p->proto_up);

    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->link_up, (" "));

    pkt_init_vs2_hdr(&vs2_hdr, 1, 1);
    pkt_init_sprout_hdr(pkt_p, &byte_idx, sprout_pdu_type_update);

    
    
    pkt_add_bytes(pkt_p, &byte_idx, sps_p->nbr_switch_addr.addr,   6);
    pkt_add_bytes(pkt_p, &byte_idx, switch_state.switch_addr.addr, 6);
#if defined(VTSS_SPROUT_FW_VER_CHK)
    if (switch_state.fw_ver_mode == VTSS_SPROUT_FW_VER_MODE_NORMAL) {
        pkt_add_bytes(pkt_p, &byte_idx, switch_state.my_fw_ver,         80);
    } else {
        
        pkt_add_bytes(pkt_p, &byte_idx, zeros,                          80);
    }
    pkt_add_bytes(pkt_p, &byte_idx, zeros,                          4);
#endif

    
    



    pkt_add_switch_section(&switch_state.switch_addr, sp_idx, pkt_p, &byte_idx);
    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
        if (vtss_sprout__switch_addr_cmp(&ri_p->switch_addr,
                                         &switch_state.switch_addr)) {
            
            if (ri_p->stack_port_dist[(sp_idx + 1) % 2] != VTSS_SPROUT_DIST_INFINITY &&
                ri_p->stack_port_dist[(sp_idx + 1) % 2] != VTSS_SPROUT_MAX_UNITS_IN_STACK) {
                
                if (ri_p->unit_idx == 0) {
                    pkt_add_switch_section(&ri_p->switch_addr, sp_idx, pkt_p, &byte_idx);
                }
            }
        }
    }

    
    pkt_add_byte(pkt_p, &byte_idx, SPROUT_SECTION_TYPE_END);
    pkt_add_byte(pkt_p, &byte_idx, 0);

    T_NG(    TRACE_GRP_PKT_DUMP, "Tx port: %u, size: %d", sps_p->port_no, byte_idx);
    T_NG_HEX(TRACE_GRP_PKT_DUMP, pkt_p, byte_idx);

    if (sps_p->update_bucket > 0) {
        T_N("Sending SPROUT Update on port=%u", sps_p->port_no);
        rc = sprout_init.callback.tx_vstax2_pkt(sps_p->port_no,
                                                &vs2_hdr,
                                                pkt_p,
                                                byte_idx);
        if (rc < 0) {
            sps_p->sprout_update_tx_err_cnt++;
        } else {
            if (periodic) {
                sps_p->sprout_update_periodic_tx_cnt++;
            } else {
                sps_p->sprout_update_triggered_tx_cnt++;
            }
        }

        T_IG(TRACE_GRP_STARTUP, "Sent SPROUT Update #%d (%s) on port=%u, rc=%d",
             sps_p->sprout_update_periodic_tx_cnt + sps_p->sprout_update_triggered_tx_cnt,
             periodic ? "periodic" : "triggered",
             sps_p->port_no, rc);

        sps_p->update_bucket--;
    } else {
        sps_p->sprout_update_tx_policer_drop_cnt++;
        T_N("tx_sprout_update: update_bucket empty");
    }

    if (rc != VTSS_OK) {
        T_N("tx_sprout_update: Unexpected return code: %d", rc);
    }

    sps_p->deci_secs_since_last_tx_update = 0;

    
    return;
} 





#if defined(VTSS_SPROUT_V2)
static void tx_sprout_alert_protodown(
    const vtss_sprout__stack_port_idx_t sp_idx)
{
    vtss_vstax_tx_header_t          *vs2_hdr_p;
    uchar                           *pkt_p;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_rc                          rc = VTSS_OK;
    vtss_sprout__ri_t               *ri_p;
    uint                             dist;

    sps_p = &switch_state.unit[0].stack_port[sp_idx];

    vs2_hdr_p = &sprout_alert_protodown_vs2_hdr;
    pkt_p     = sprout_alert_protodown_pkt;

    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->link_up, (" "));

    
    
    
    dist = 0;
    ri_p = NULL;
    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
        if (ri_p->stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY &&
            vtss_sprout__switch_addr_cmp(&ri_p->switch_addr, &switch_state.switch_addr) != 0 &&
            ri_p->stack_port_dist[sp_idx] > dist) {
            dist = ri_p->stack_port_dist[sp_idx];
        }
    }

    
    pkt_change_ttl(vs2_hdr_p, dist);

    T_D("sp_idx=%d, ttl=%d, adm_up=%d, link_up=%d, proto_up=%d",
        sp_idx,
        dist, sps_p->adm_up, sps_p->link_up, sps_p->proto_up);


    T_DG(    TRACE_GRP_PKT_DUMP, "Tx port: %u, size: %d", sps_p->port_no, SPROUT_ALERT_PROTODOWN_PDU_LEN);
    T_DG_HEX(TRACE_GRP_PKT_DUMP, pkt_p, SPROUT_ALERT_PROTODOWN_PDU_LEN);

    if (sps_p->alert_bucket > 0) {
        rc = sprout_init.callback.tx_vstax2_pkt(sps_p->port_no,
                                                vs2_hdr_p,
                                                pkt_p,
                                                SPROUT_ALERT_PROTODOWN_PDU_LEN);
        if (rc < 0) {
            sps_p->sprout_alert_tx_err_cnt++;
        } else {
            sps_p->sprout_alert_tx_cnt++;
        }

        sps_p->alert_bucket--;
    } else {
        sps_p->sprout_alert_tx_policer_drop_cnt++;
        T_D("alert_bucket empty");
    }

    if (rc != VTSS_OK) {
        T_D("Unexpected return code: %d", rc);
    }

    T_DG(TRACE_GRP_FAILOVER, "alert tx done");

    
    return;
} 
#endif











static uchar pkt_get_byte(
    const uchar *pkt_p,
    uint *const i_p)
{
    uchar byte;

    byte = pkt_p[*i_p];
    *i_p += 1;

    return byte;
} 


static vtss_rc pkt_get_switch_addr(
    const uchar             *pkt_p,
    uint                    *i_p,
    const uint              len,
    vtss_sprout_switch_addr_t *switch_addr_p)
{
    uint i;

    if ((len - *i_p) < 6) {
        return VTSS_RC_ERROR;
    } else {
        for (i = 0; i < 6; i++) {
            switch_addr_p->addr[i] = pkt_get_byte(pkt_p, i_p);
        }
    }

    return VTSS_OK;
} 


typedef struct _switch_section_t {
    
    vtss_sprout_switch_addr_t     switch_addr;
    BOOL                          mst_capable;
    vtss_sprout__mst_elect_prio_t mst_elect_prio;
    ulong                         mst_time;
    BOOL                          mst_time_ignore;

    
    BOOL               have_mirror[2];
    vtss_sprout_dist_t dist[2];
    BOOL               primary_unit[2];
#if defined(VTSS_SPROUT_V1)
    uint               glag_mbr_cnt[2][VTSS_GLAGS];
#endif

    
    vtss_vstax_upsid_t upsid[2][2];
    u64                ups_port_mask[2][2];

    
    BOOL                 tightly_coupled;

    
    ulong ip_addr;

    vtss_sprout_switch_appl_info_t switch_appl_info;

    
    uchar unit_base_info_rsv[2];               
#if defined(VTSS_SPROUT_V1)
    uchar unit_glag_mbr_cnt_rsv[2][VTSS_GLAGS]; 
#endif
    uchar ups_base_info_rsv[2][2];              
    uchar switch_mst_elect_rsv;                 
    uchar switch_base_info_rsv;                 
} switch_section_t;


static void switch_section_init(
    switch_section_t *switch_section_p)
{
    vtss_sprout__unit_idx_t  unit_idx;
    vtss_sprout__upsid_idx_t ups_idx;

    memset(switch_section_p, 0, sizeof(switch_section_t));

    vtss_sprout_switch_addr_init(&switch_section_p->switch_addr);
    switch_section_p->mst_elect_prio = 0;

    for (unit_idx = 0; unit_idx < 2; unit_idx++) {

        switch_section_p->have_mirror[unit_idx] = 0;
        switch_section_p->dist[unit_idx] = VTSS_SPROUT_DIST_INFINITY;

#if defined(VTSS_SPROUT_V1)
        {
            vtss_sprout__glagid_t glagid;
            for (glagid = 0; glagid < VTSS_GLAGS; glagid++) {
                switch_section_p->glag_mbr_cnt[unit_idx][glagid] = 0;
            }
        }
#endif

        for (ups_idx = 0; ups_idx < 2; ups_idx++) {
            switch_section_p->upsid[unit_idx][ups_idx] = VTSS_VSTAX_UPSID_UNDEF;
        }
    }

    switch_section_p->ip_addr = 0;
} 





static vtss_rc pkt_get_switch_section(
    const vtss_port_no_t    port_no,
    const uchar             *pkt_p,
    uint                    *i_p,
    const uint              sect_len,
    switch_section_t        *switch_section_p)
{
    uchar byte;
    uint section_len;
    uint section_end; 
    uchar tlv_cnt[64];

    memset(tlv_cnt, 0, sizeof(tlv_cnt));

    if (sect_len - *i_p < 8) {
        
        topo_protocol_error(
            "Port %d: Incomplete section header",
            port_no);

        return VTSS_RC_ERROR;
    }

    section_len = pkt_get_byte(pkt_p, i_p);
    section_end = *i_p + section_len;

    if (sect_len - *i_p < section_len) {
        
        topo_protocol_error(
            "Port %d: Packet length (%d) shorter than section length (%d)",
            port_no, sect_len - *i_p, section_len);

        return VTSS_RC_ERROR;
    }

    pkt_get_switch_addr(pkt_p, i_p, sect_len, &switch_section_p->switch_addr);

    
    while (*i_p <= section_end - 2) {
        uint tlv_type;
        vtss_sprout__unit_idx_t  unit_idx;
        vtss_sprout__upsid_idx_t ups_idx;
        BOOL                     len;  

        byte = pkt_get_byte(pkt_p, i_p);

        tlv_type = (byte >> 2);
        unit_idx = ((byte & 0x2) >> 1);
        ups_idx  = (byte & 0x1);
        if (tlv_type < 32) {
            
            len = 1;
        } else {
            
            len = pkt_get_byte(pkt_p, i_p);
        }

        switch (tlv_type) {
        case SPROUT_SWITCH_TLV_TYPE_UNIT_BASE_INFO:
            
            byte = pkt_get_byte(pkt_p, i_p);
            switch_section_p->primary_unit[unit_idx]       = ((byte & 0x40) >> 6);
            switch_section_p->have_mirror[unit_idx]        = ((byte & 0x20) >> 5);
            switch_section_p->dist[unit_idx]               = (byte & 0x1f);
            switch_section_p->unit_base_info_rsv[unit_idx] = (byte & SPROUT_SWITCH_TLV_UNIT_BASE_INFO_RSV_MASK);

            
            VTSS_SPROUT_ASSERT_DBG_INFO(!unit_idx == switch_section_p->primary_unit[unit_idx],
                                        (" "));
            break;

#if defined(VTSS_SPROUT_V1)
        case SPROUT_SWITCH_TLV_TYPE_UNIT_GLAG_MBR_CNT: {
            uint  glag_id;

            
            byte = pkt_get_byte(pkt_p, i_p);
            glag_id = (byte & 0x10) >> 4;
            switch_section_p->glag_mbr_cnt[unit_idx][glag_id] =
                (byte & 0xf) + 1;
            switch_section_p->unit_glag_mbr_cnt_rsv[unit_idx][glag_id] =
                (byte & SPROUT_SWITCH_TLV_UNIT_GLAG_MBR_CNT_RSV_MASK);
        }
        break;
#endif

        case SPROUT_SWITCH_TLV_TYPE_UPS_BASE_INFO:
            byte = pkt_get_byte(pkt_p, i_p);
            switch_section_p->upsid[unit_idx][ups_idx] = (byte & 0x1f);
            switch_section_p->ups_base_info_rsv[unit_idx][ups_idx] =
                (byte & SPROUT_SWITCH_TLV_UPS_BASE_INFO_RSV_MASK);
            break;

        case SPROUT_SWITCH_TLV_TYPE_UPS_PORT_MASK:
            if (len == 8) {
                int i;
                for (i = 7; i >= 0; i--) {
                    switch_section_p->ups_port_mask[unit_idx][ups_idx] |= ((u64)pkt_get_byte(pkt_p, i_p) << (i * 8));
                }
            } else {
                topo_protocol_error(
                    "Port %d: Syntax error: Length of switch info TLV is %d, expected 8",
                    port_no, len);
                return VTSS_RC_ERROR;
            }
            break;

        case SPROUT_SWITCH_TLV_TYPE_SWITCH_BASE_INFO:
            byte = pkt_get_byte(pkt_p, i_p);
            switch_section_p->tightly_coupled = (byte & 0x01);
            switch_section_p->switch_base_info_rsv =
                (byte & SPROUT_SWITCH_TLV_SWITCH_BASE_INFO_RSV_MASK);
            break;

        case SPROUT_SWITCH_TLV_TYPE_SWITCH_IP_ADDR:
            if (len == 4) {
                switch_section_p->ip_addr =
                    (pkt_get_byte(pkt_p, i_p) << 24 |
                     pkt_get_byte(pkt_p, i_p) << 16 |
                     pkt_get_byte(pkt_p, i_p) <<  8 |
                     pkt_get_byte(pkt_p, i_p) <<  0);
            } else {
                topo_protocol_error(
                    "Port %d: Syntax error: Length of IP Addr. TLV is %d, expected 4",
                    port_no, len);
                return VTSS_RC_ERROR;
            }
            break;

        case SPROUT_SWITCH_TLV_TYPE_SWITCH_APPL_INFO:
            if (len == 8) {
                int i;
                for (i = 0; i < VTSS_SPROUT_SWITCH_APPL_INFO_LEN; i++) {
                    switch_section_p->switch_appl_info[i] = pkt_get_byte(pkt_p, i_p);
                }
            } else {
                topo_protocol_error(
                    "Port %d: Syntax error: Length of switch info TLV is %d, expected 8",
                    port_no, len);
                return VTSS_RC_ERROR;
            }
            break;

        case SPROUT_SWITCH_TLV_TYPE_SWITCH_MST_ELECT:
            if (len == 5) {
                uchar byte;
                switch_section_p->mst_time =
                    (pkt_get_byte(pkt_p, i_p) << 24 |
                     pkt_get_byte(pkt_p, i_p) << 16 |
                     pkt_get_byte(pkt_p, i_p) <<  8 |
                     pkt_get_byte(pkt_p, i_p) <<  0);
                byte = pkt_get_byte(pkt_p, i_p);
                switch_section_p->mst_time_ignore = (byte & 0x80) >> 7;
                switch_section_p->mst_elect_prio  = (byte & 0x03);
                switch_section_p->mst_capable     = (byte & 0x04) >> 2;
                switch_section_p->switch_mst_elect_rsv =
                    (byte & SPROUT_SWITCH_TLV_SWITCH_MST_ELECT_BYTE4_RSV_MASK);
            } else {
                topo_protocol_error(
                    "Port %d: Syntax error: Length of Master Elect is %d, expected 5",
                    port_no, len);
                return VTSS_RC_ERROR;
            }
            break;

        default:
            
            T_W("Port %u: Unknown TLV type: 0x%02x", port_no, tlv_type);

            *i_p = *i_p + len;
        }
        tlv_cnt[tlv_type]++;
    }

    if (*i_p != section_end) {
        topo_protocol_error(
            "Port %d: Syntax error in switch section.",
            port_no);
        T_N("*i_p=%d, section_end=%d", *i_p, section_end);

        return VTSS_RC_ERROR;
    }

    if (
#if defined(VTSS_SPROUT_V1)
        tlv_cnt[SPROUT_SWITCH_TLV_TYPE_UNIT_GLAG_MBR_CNT] > 0 ||
#endif
        tlv_cnt[SPROUT_SWITCH_TLV_TYPE_UPS_BASE_INFO]     > 0) {
        if (tlv_cnt[SPROUT_SWITCH_TLV_TYPE_UNIT_BASE_INFO] == 0) {
            topo_protocol_error(
                "Port %d: UnitBaseInfo TLV missing", port_no);
        }
    }

    {
        
        vtss_sprout__unit_idx_t  unit_idx;
        vtss_sprout__upsid_idx_t ups_idx;
        u64                      ups_port_mask = 0;
        for (unit_idx = 0; unit_idx < 2; unit_idx++) {
            for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                if (ups_port_mask &
                    switch_section_p->ups_port_mask[unit_idx][ups_idx]) {
                    topo_protocol_error("Port %d: Overlapping ups_port_mask", port_no);
                    break;
                }
                ups_port_mask |= switch_section_p->ups_port_mask[unit_idx][ups_idx];
            }
        }
    }

    return VTSS_OK;
} 



static vtss_rc rx_sprout_update(
    const vtss_port_no_t                port_no,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const uchar                         *pkt_p,
    const uint                          len)
{
    
    switch_section_t              switch_section[VTSS_SPROUT_MAX_UNITS_IN_STACK];
    vtss_sprout_switch_addr_t     sa_mine;
    vtss_sprout_switch_addr_t     sa_nbr;
    vtss_sprout_switch_addr_t     sa;
    uint                          ss_cnt = 0; 
    uint                          byte_idx;
    vtss_rc                       rc = VTSS_OK;
    uint                          i, j;
    uchar                         section_type;
    BOOL                          found_end_section;
    BOOL                          found;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_sprout__ui_t               ui;
    vtss_sprout__ri_t               ri;
    vtss_sprout__unit_idx_t         u;
    uint                          cnt;
    BOOL                          upsids_changed = 0; 
    

    BOOL                          force_tx_sprout_update[2] = {0, 0};
    uchar                         byte;
    BOOL                          org_proto_up;

    
    sps_p = &switch_state.unit[0].stack_port[sp_idx];
    org_proto_up = sps_p->proto_up;

    T_IG(TRACE_GRP_STARTUP, "rx_sprout_update: sp_idx=%d, len=%d, adm_up=%d, link_up=%d",
         sp_idx, len, sps_p->adm_up, sps_p->link_up);
    T_N("rx_sprout_update: port_no=%u, sp_idx=%d, len=%d, adm_up=%d, link_up=%d",
        port_no, sp_idx, len, sps_p->adm_up, sps_p->link_up);
    T_NG(TRACE_GRP_PKT_DUMP, "rx_sprout_update: port_no=%u, sp_idx=%d, len=%d, adm_up=%d, link_up=%d",
         port_no, sp_idx, len, sps_p->adm_up, sps_p->link_up);
    T_NG_HEX(TRACE_GRP_PKT_DUMP, pkt_p, len);

    
    VTSS_SPROUT_ASSERT_DBG_INFO(!rit.changed &&
                                !uit.changed &&
                                !switch_state.unit[0].stack_port[sp_idx].uit.changed,
                                (" "));

    sps_p->sprout_update_rx_cnt++;

    if (sps_p->adm_up == 0) {
        
        return VTSS_OK;
    }

    if (sps_p->link_up == 0) {
        
        return VTSS_OK;
    }


    for (i = 0; i < VTSS_SPROUT_MAX_UNITS_IN_STACK; i++) {
        switch_section_init(&switch_section[i]);
    }

    
    byte_idx = 20;

    
    byte = pkt_get_byte(pkt_p, &byte_idx);
    VTSS_SPROUT_ASSERT(byte == SPROUT_PDU_TYPE_UPDATE, ("PDU Type=%d", byte));

    
    if ((byte = pkt_get_byte(pkt_p, &byte_idx)) != sprout_ver) {
        
        sps_p->sprout_update_rx_err_cnt++;
        topo_protocol_error(
            "Port %d: Unknown SPROUT version: 0x%02x",
            port_no, byte);
        return VTSS_OK;
    }

    
    byte_idx += 2;

    
    if ((rc = pkt_get_switch_addr(pkt_p, &byte_idx, len, &sa_mine)) < 0) {
        sps_p->sprout_update_rx_err_cnt++;
        topo_protocol_error(
            "Port %d: Packet too short while parsing switch address in Update Header",
            port_no);
        return rc;
    }

    
    if ((rc = pkt_get_switch_addr(pkt_p, &byte_idx, len, &sa_nbr)) < 0)  {
        sps_p->sprout_update_rx_err_cnt++;
        topo_protocol_error(
            "Port %d: Packet too short, while parsing neighbour switch address in Update Header",
            port_no);
        return rc;
    }

#if defined(VTSS_SPROUT_FW_VER_CHK)
    
    
    
    
    if (!sps_p->proto_up) {
        if (sprout_init.callback.fw_ver_chk(
                sps_p->port_no,
                1,
                (pkt_p + byte_idx)) != VTSS_RC_OK) {
            T_I("Ignoring SPROUT Update with not interoperable FW version.");
            return VTSS_OK;
        }
    }
    byte_idx += 84;
#endif

    
    found_end_section = 0;
    while (!found_end_section &&
           byte_idx < len &&
           ss_cnt < VTSS_SPROUT_MAX_UNITS_IN_STACK) {
        section_type = pkt_get_byte(pkt_p, &byte_idx);

        if (section_type == SPROUT_SECTION_TYPE_SWITCH) {
            rc = pkt_get_switch_section(port_no, pkt_p, &byte_idx, len, &switch_section[ss_cnt++]);
            if (rc < 0) {
                sps_p->sprout_update_rx_err_cnt++;
                return rc;
            }
        } else if (section_type == SPROUT_SECTION_TYPE_END) {
            found_end_section = 1;
        } else {
            sps_p->sprout_update_rx_err_cnt++;
            T_W("Port %u: Unknown section type: %d", port_no, section_type);
            return VTSS_RC_ERROR;
        }
    }

    if (!found_end_section) {
        sps_p->sprout_update_rx_err_cnt++;
        topo_protocol_error(
            "Port %d: No end section before end-of-packet",
            port_no);
    }

    
    
    found = 0;
    for (i = 0; i < ss_cnt && !found; i++) {
        if (vtss_sprout__switch_addr_cmp(&sa_nbr, &switch_section[i].switch_addr) == 0 &&
            switch_section[i].dist[0] != VTSS_SPROUT_DIST_INFINITY) {
            found = 1;
        }
    }
    if (!found) {
        sps_p->sprout_update_rx_err_cnt++;
        topo_protocol_error("Port %d: No switch section found for neighbour", port_no);
        return VTSS_RC_ERROR;
    }

    
    for (i = 0; i < ss_cnt; i++) {
        sa = switch_section[i].switch_addr;
        for (j = i + 1; j < ss_cnt; j++) {
            if (vtss_sprout__switch_addr_cmp(&sa, &switch_section[j].switch_addr) == 0) {
                sps_p->sprout_update_rx_err_cnt++;
                topo_protocol_error("Port %d: Two switch sections for same switch address.", port_no);
                return VTSS_RC_ERROR;
            }
        }
    }

    
    
    if (sps_p->proto_up) {
        VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->link_up, (" "));

        
        if (vtss_sprout__switch_addr_cmp(&sps_p->nbr_switch_addr, &sa_nbr) != 0) {
            
            T_I("Port %u: ProtoUp->ProtoDown, neighbour address changed (old: %s, new: %s)",
                port_no,
                vtss_sprout_switch_addr_to_str(&sps_p->nbr_switch_addr),
                vtss_sprout_switch_addr_to_str(&sa_nbr));
            sps_p->proto_up = 0;
        }

        
        if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr, &sa_mine) != 0) {
            
            T_I("Port %u: ProtoUp->ProtoDown, neighbour no longer knows me (he says I am %s)!",
                port_no, vtss_sprout_switch_addr_to_str(&sa_mine));
            sps_p->proto_up = 0;
        }

        if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr, &sa_nbr) == 0) {
            





            T_I("Port %u: ProtoUp->ProtoDown, neighbour is myself!",
                port_no);
            sps_p->proto_up = 0;
        }

        if (!sps_p->proto_up) {
            
            vtss_sprout__uit_init(&sps_p->uit);
            sps_p->uit.changed = 1;

            
            vtss_sprout__rit_infinity_all(&rit, sp_idx);

            if (vtss_sprout__switch_addr_cmp(&sps_p->nbr_switch_addr, &null_switch_addr) != 0) {
                T_N("Send update to neighbour to try to reestablish neighbourship");
                force_tx_sprout_update[sp_idx] = 1;
            }
        }
    } else {
        

        if (switch_state.virgin) {
            
            BOOL upsid_mask[VTSS_SPROUT_MAX_UNITS_IN_STACK];

            if (switch_state.virgin) {
                T_I("Virgin");
            }

            
            memset(upsid_mask, 0, sizeof(upsid_mask));
            for (i = 0; i < ss_cnt; i++) {
                for (u = 0; u < 2; u++) {
                    vtss_sprout__upsid_idx_t ups_idx;
                    for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                        vtss_vstax_upsid_t upsid;
                        upsid = switch_section[i].upsid[u][ups_idx];
                        if (upsid != VTSS_VSTAX_UPSID_UNDEF) {
                            upsid_mask[upsid] = 1;
                        }
                    }
                }
            }

            VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.unit[0].stack_port[0].proto_up == 0,
                                        (" "));
            VTSS_SPROUT_ASSERT_DBG_INFO(switch_state.unit[0].stack_port[1].proto_up == 0,
                                        (" "));

            rc = upsids_initial_calc(upsid_mask);
            if (rc == SPROUT_RC_UPSID_DEPLETION) {
                T_I("UPSID depletion encountered in first update reception");
                return VTSS_OK;
            } else if (rc < 0) {
                T_IG(TRACE_GRP_STARTUP, "upsids_initial_calc=%d", rc);
                return rc;
            }

            
            if (switch_state.unit[0].stack_port[!sp_idx].link_up &&
                switch_state.unit[0].stack_port[!sp_idx].adm_up) {
                T_D("We now have an UPSID, so tx update on other stack port (sp_idx=%d)", !sp_idx);
                force_tx_sprout_update[!sp_idx] = 1;
            }
        }


        
        if (vtss_sprout__switch_addr_cmp(&sa_nbr,  &switch_state.switch_addr) != 0) {
            if (vtss_sprout__switch_addr_cmp(&sa_nbr, &null_switch_addr) != 0 &&
                vtss_sprout__switch_addr_cmp(&sa_mine, &switch_state.switch_addr) == 0 &&
                sps_p->link_up == 1 &&
                sps_p->adm_up == 1) {
                
                T_I("Port %u: ProtoDown->ProtoUp (except if UPSID depletion)", port_no);

                sps_p->nbr_switch_addr = sa_nbr;
                force_tx_sprout_update[sp_idx] = 1;
                sps_p->proto_up = 1;
            } else if (sps_p->link_up == 1 && sps_p->adm_up == 1) {
                
                T_I("Neighbour does not know me. tx update sp_idx=%d\n"
                    "sa_nbr=%s sa_mine=%s sps_p->nbr_switch_addr=%s",
                    sp_idx,
                    vtss_sprout_switch_addr_to_str(&sa_nbr),
                    vtss_sprout_switch_addr_to_str(&sa_mine),
                    vtss_sprout_switch_addr_to_str(&sps_p->nbr_switch_addr));
                sps_p->nbr_switch_addr = sa_nbr;
                force_tx_sprout_update[sp_idx] = 1;
            }
        }
    } 

    
    sps_p->nbr_switch_addr = sa_nbr;

    if (sps_p->proto_up) {
        
        uint foreign_unit_cnt = 0;

        for (i = 0; i < ss_cnt; i++) {
            for (u = 0; u < 2; u++) {
                if (switch_section[i].dist[u] != VTSS_SPROUT_DIST_INFINITY &&
                    vtss_sprout__switch_addr_cmp(&switch_section[i].switch_addr,
                                                 &switch_state.switch_addr) != 0) {
                    foreign_unit_cnt++;
                }
            }
        }
        T_N("foreign_unit_cnt=%d (ss_cnt=%d)", foreign_unit_cnt, ss_cnt);

        
        for (i = 0; i < ss_cnt; i++) {
            BOOL switch_section_me =
                (vtss_sprout__switch_addr_cmp(&switch_section[i].switch_addr,
                                              &switch_state.switch_addr) == 0);

            for (u = 0; u < 2; u++) {
                switch_section_me = (vtss_sprout__switch_addr_cmp(&switch_section[i].switch_addr,
                                                                  &switch_state.switch_addr) == 0);
                if (switch_section[i].dist[u] != VTSS_SPROUT_DIST_INFINITY &&

                    
                    !(switch_section[i].dist[u] + 1 >= foreign_unit_cnt + 1 &&
                      !switch_section_me)) {
                    





                    vtss_sprout__ui_init(&ui);
                    ui.switch_addr = switch_section[i].switch_addr;
                    ui.unit_idx = u;
                    ui.upsid[0]         = switch_section[i].upsid[u][0];
                    ui.upsid[1]         = switch_section[i].upsid[u][1];
                    ui.ups_port_mask[0] = switch_section[i].ups_port_mask[u][0];
                    ui.ups_port_mask[1] = switch_section[i].ups_port_mask[u][1];

#if defined(VTSS_SPROUT_V1)
                    {
                        vtss_sprout__glagid_t           glagid;
                        for (glagid = 0; glagid < VTSS_GLAGS; glagid++) {
                            ui.glag_mbr_cnt[glagid] =
                                switch_section[i].glag_mbr_cnt[u][glagid];
                        }
                    }
#endif

                    ui.have_mirror  = switch_section[i].have_mirror[u];
#if defined(VTSS_SPROUT_V2)
                    ui.primary_unit = switch_section[i].primary_unit[u];
#else
                    ui.primary_unit = 1;
#endif
                    
                    ui.mst_capable     = switch_section[i].mst_capable;
                    ui.mst_elect_prio  = switch_section[i].mst_elect_prio;
                    ui.mst_time        = switch_section[i].mst_time;
                    ui.mst_time_ignore = switch_section[i].mst_time_ignore;

                    ui.tightly_coupled = switch_section[i].tightly_coupled;

                    ui.ip_addr = switch_section[i].ip_addr;

                    memcpy(ui.switch_appl_info, switch_section[i].switch_appl_info,
                           VTSS_SPROUT_SWITCH_APPL_INFO_LEN);

                    
                    ui.unit_base_info_rsv = switch_section[i].unit_base_info_rsv[u];
#if defined(VTSS_SPROUT_V1)
                    memcpy(ui.unit_glag_mbr_cnt_rsv, switch_section[i].unit_glag_mbr_cnt_rsv[u], VTSS_GLAGS);
#endif
                    memcpy(ui.ups_base_info_rsv,     switch_section[i].ups_base_info_rsv[u],     2);
                    ui.switch_mst_elect_rsv = switch_section[i].switch_mst_elect_rsv;
                    ui.switch_base_info_rsv = switch_section[i].switch_base_info_rsv;

                    T_N("dist=%d\n%s",
                        switch_section[i].dist[u],
                        vtss_sprout__ui_to_str(&ui));
                    vtss_sprout__uit_update(&sps_p->uit, &ui, 1);

                    vtss_sprout__ri_init(&ri);
                    ri.switch_addr     = switch_section[i].switch_addr;
                    ri.unit_idx        = u;
                    ri.upsid[0]        = switch_section[i].upsid[u][0];
                    ri.upsid[1]        = switch_section[i].upsid[u][1];
                    
                    if (vtss_board_chipcount() == 1) {
                        ri.stack_port_dist[sp_idx] = switch_section[i].dist[u] + 1;
                    } else {
                        if (sp_idx == VTSS_SPROUT__SP_A) {
                            ri.stack_port_dist[sp_idx] = switch_section[i].dist[u] + 1;
                        } else {
                            
                            if (!switch_section_me) {
                                
                                ri.stack_port_dist[sp_idx] = switch_section[i].dist[u] + 2;
                            } else {
                                
                                if (u == 0) {
                                    
                                    ri.stack_port_dist[sp_idx] = switch_section[i].dist[u] + 2;
                                } else {
                                    
                                    
                                    ri.stack_port_dist[sp_idx] = 1;
                                }
                            }
                        }

                    }
                    ri.tightly_coupled         = switch_section[i].tightly_coupled;

                    vtss_sprout__rit_update(&rit, &ri, sp_idx);
                }
            }
        } 

        
        vtss_sprout__uit_del_unfound(&sps_p->uit);
        cnt = vtss_sprout__rit_infinity_if_not_found(&rit, sp_idx);

        if (cnt > 0) {
            T_N("Dist changed to infinity for %d RIT entries", cnt);
        }
    }

    if (sps_p->proto_up && sps_p->uit.changed) {
        
        rc = upsids_chk_and_recalc(&upsids_changed);

        T_DG(TRACE_GRP_UPSID, "upsids_chk_and_recalc rc=%d sp_idx=%u", rc, sp_idx);

        if (rc == SPROUT_RC_UPSID_DEPLETION) {
            
            sps_p->proto_up = 0;

            
            vtss_sprout__uit_init(&sps_p->uit);

            
            vtss_sprout__rit_infinity_all(&rit, sp_idx);

            
            vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);

            if (org_proto_up) {
                
                T_I("Going down due to UPSID depletion");
                sps_p->uit.changed = 1;

                force_tx_sprout_update[sp_idx] = 1;
                force_tx_sprout_update[!sp_idx] =
                    (switch_state.unit[0].stack_port[!sp_idx].link_up &&
                     switch_state.unit[0].stack_port[!sp_idx].adm_up);
            } else {
                



                T_I("Cancelling ProtoUp, due to UPSID depletion");
                force_tx_sprout_update[sp_idx]  = 0;
                force_tx_sprout_update[!sp_idx] = 0;
            }
        }
    }

    
    {
        BOOL allow_tx_sprout_update[2];

        if (switch_state.virgin && sps_p->proto_up) {
            switch_state.virgin = 0;
        }

        allow_tx_sprout_update[0] = ((sp_idx == 1 || upsids_changed) && !force_tx_sprout_update[0]);
        allow_tx_sprout_update[1] = ((sp_idx == 0 || upsids_changed) && !force_tx_sprout_update[1]);

        rc = process_xit_changes(allow_tx_sprout_update[0],
                                 allow_tx_sprout_update[1]);
        if (rc < 0) {
            return rc;
        }

        if (force_tx_sprout_update[0]) {
            T_N("Forced tx_sprout_update, sp_idx=0");
            tx_sprout_update(0, 0);
        }
        if (force_tx_sprout_update[1]) {
            T_N("Forced tx_sprout_update, sp_idx=1");
            tx_sprout_update(1, 0);
        }
        T_N("rx_sprout_update done");
    }


    
    sps_p->deci_secs_since_last_rx_update = 0;

    return rc;
} 


#if defined(VTSS_SPROUT_V2)
static vtss_rc rx_sprout_alert(
    const vtss_port_no_t                port_no,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const uchar                         *pkt_p,
    const uint                          len)
{
    vtss_sprout__stack_port_state_t *sps_p;
    uint                             byte_idx;
    uchar                            byte;
    vtss_sprout_switch_addr_t        sa_sender;
    uchar                            alert_type;
    uchar                            alert_data;
    vtss_sprout__ri_t               *ri_p;
    vtss_sprout_dist_t               max_dist_to_sender;

    T_DG(TRACE_GRP_FAILOVER, "alert rx start %u", port_no);

    sps_p = &switch_state.unit[0].stack_port[sp_idx];

    T_NG(TRACE_GRP_PKT_DUMP, "sp_idx=%d, len=%d, adm_up=%d, link_up=%d",
         sp_idx, len, sps_p->adm_up, sps_p->link_up);
    T_NG_HEX(TRACE_GRP_PKT_DUMP, pkt_p, len);

    
    VTSS_SPROUT_ASSERT_DBG_INFO(!rit.changed &&
                                !uit.changed &&
                                !switch_state.unit[0].stack_port[sp_idx].uit.changed,
                                (" "));

    sps_p->sprout_alert_rx_cnt++;

    if (sps_p->adm_up == 0) {
        
        return VTSS_OK;
    }

    
    byte_idx = 20;

    
    byte = pkt_get_byte(pkt_p, &byte_idx);
    VTSS_SPROUT_ASSERT(byte == SPROUT_PDU_TYPE_ALERT, ("PDU Type=%d", byte));

    
    if ((byte = pkt_get_byte(pkt_p, &byte_idx)) != SPROUT_VER) {
        
        topo_protocol_error("Port %d: Unknown SPROUT version: 0x%02x",
                            port_no, byte);
        sps_p->sprout_alert_rx_err_cnt++;
        return VTSS_OK;
    }

    
    byte_idx += 2;

    
    if (len < byte_idx + 8 + 1) {
        
        topo_protocol_error(
            "Port %d: Alert PDU too short, length=%d",
            len);
        sps_p->sprout_alert_rx_err_cnt++;
        return VTSS_OK;
    }

    
    pkt_get_switch_addr(pkt_p, &byte_idx, len, &sa_sender);

    
    alert_type = pkt_get_byte(pkt_p, &byte_idx);
    alert_data = pkt_get_byte(pkt_p, &byte_idx);

    switch (alert_type) {
    case SPROUT_ALERT_TYPE_PROTODOWN:
        
        T_I("sp_idx=%d, len=%d, adm_up=%d, link_up=%d, sender=%s",
            sp_idx, len, sps_p->adm_up, sps_p->link_up,
            vtss_sprout_switch_addr_to_str(&sa_sender));

        if (sps_p->proto_up == 0) {
            return VTSS_OK;
        }

        
        if (vtss_sprout__switch_addr_cmp(&sa_sender,
                                         &switch_state.switch_addr) == 0) {
            T_W("Received own ProtoDown Alert!?, sp_idx=%d", sp_idx);
            return VTSS_OK;
        }

        
        max_dist_to_sender = VTSS_SPROUT_DIST_INFINITY;
        ri_p = NULL;
        while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
            if (vtss_sprout__switch_addr_cmp(&sa_sender,
                                             &ri_p->switch_addr) == 0) {
                
                if (ri_p->stack_port_dist[sp_idx] != VTSS_SPROUT_DIST_INFINITY) {
                    
                    VTSS_SPROUT_ASSERT_DBG_INFO(max_dist_to_sender != ri_p->stack_port_dist[sp_idx],
                                                ("Two units with same distance=%d?!",
                                                 max_dist_to_sender));
                    if (max_dist_to_sender == VTSS_SPROUT_DIST_INFINITY ||
                        max_dist_to_sender < ri_p->stack_port_dist[sp_idx]) {
                        max_dist_to_sender = ri_p->stack_port_dist[sp_idx];
                    }
                }
            }
        }

        if (max_dist_to_sender == VTSS_SPROUT_DIST_INFINITY) {
            
            T_I("Ignoring ProtoDown Alert on sp_idx=%d, since sender unknown",
                sp_idx);
            return VTSS_OK;
        }

        
        vtss_sprout__rit_infinity_beyond_dist(&rit, sp_idx, max_dist_to_sender);

        
        process_xit_changes(0, 0);

        T_DG(TRACE_GRP_FAILOVER,
             "rx_sprout_alert done sp_idx=%d",
             sp_idx);
        T_N("Done");
        break;

    default:
        topo_protocol_error("Unknown Alert type: 0x%02x", alert_type);
        return VTSS_OK;
        break;
    }

    T_DG(TRACE_GRP_FAILOVER, "alert rx end");

    return VTSS_OK;
} 
#endif













static void update_local_uis(void)
{
    uint unit_idx;
    vtss_sprout__ui_t ui;

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        vtss_sprout__ui_init(&ui);

        ui.switch_addr     = switch_state.switch_addr;
        ui.unit_idx        = unit_idx;
#if defined(VTSS_SPROUT_V1)
        {
            uint g;
            for (g = 0; g < VTSS_GLAGS; g++) {
                ui.glag_mbr_cnt[g] = switch_state.unit[unit_idx].glag_mbr_cnt[g];
            }
        }
#endif
        ui.mst_capable     = switch_state.mst_capable;
        ui.mst_elect_prio  = switch_state.mst_elect_prio;
        ui.mst_time_ignore = switch_state.mst_time_ignore;

        ui.have_mirror       = switch_state.unit[unit_idx].have_mirror_port;
        
        ui.upsid[0]          = switch_state.unit[unit_idx].upsid;
        ui.ups_port_mask[0]  = switch_state.unit[unit_idx].ups_port_mask;
        if (unit_idx == 0) {
            
            ui.primary_unit    = 1;
        }
        if (unit_idx == 1) {
            
            
            ui.sp_idx = VTSS_SPROUT__SP_B;
        }


        ui.ip_addr = switch_state.ip_addr;

        memcpy(ui.switch_appl_info, switch_state.switch_appl_info,
               VTSS_SPROUT_SWITCH_APPL_INFO_LEN);

        
        vtss_sprout__uit_update(&uit, &ui, 0);
    }
} 





static void merge_uits(void)
{
    vtss_sprout__uit_t            *uit_sp_p[2];
    vtss_sprout__ui_t             *ui_p;
    vtss_sprout__ri_t             *ri_p;
    vtss_sprout__stack_port_idx_t sp_idx;
    u32                           change_mask;

    T_D("enter");

    
    
    uit_sp_p[0] = &switch_state.unit[0].stack_port[0].uit;
    uit_sp_p[1] = &switch_state.unit[0].stack_port[1].uit;

    
    if (!rit.changed &&
        !uit_sp_p[0]->changed &&
        !uit_sp_p[1]->changed) {
        if (uit_sp_p[0]->mst_time_changed ||
            uit_sp_p[1]->mst_time_changed) {
            
            vtss_sprout__ui_t *cur_ui_p = NULL;
            vtss_sprout__ui_t *sp_ui_p = NULL;

            T_D("Only mst_time to be merged");

            
            while ((cur_ui_p = vtss_sprout__ui_get_nxt(&uit, cur_ui_p))) {
                if (vtss_sprout__switch_addr_cmp(&cur_ui_p->switch_addr,
                                                 &switch_state.switch_addr) != 0) {
                    
                    VTSS_SPROUT_ASSERT_DBG_INFO(cur_ui_p->sp_idx != VTSS_SPROUT__SP_UNDEF,
                                                (" "));
                    sp_ui_p = vtss_sprout__ui_find(uit_sp_p[cur_ui_p->sp_idx],
                                                   &cur_ui_p->switch_addr,
                                                   cur_ui_p->unit_idx);
                    VTSS_SPROUT_ASSERT_DBG_INFO(sp_ui_p != NULL, (" "));

                    cur_ui_p->mst_time = sp_ui_p->mst_time;
                }
            }
            uit.mst_time_changed = 1;
            return;
        } else {
            
            T_D("Nothing to merge");
            return;
        }
    }
    

    
    change_mask = uit.change_mask; 
    vtss_sprout__uit_init(&uit);
    uit.changed = 1;
    uit.change_mask = change_mask;

    
    ui_p = NULL;
    ri_p = NULL;
    while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
        if (vtss_sprout__switch_addr_cmp(&ri_p->switch_addr,
                                         &switch_state.switch_addr) != 0) {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(!(ri_p->stack_port_dist[VTSS_SPROUT__SP_A] == VTSS_SPROUT_DIST_INFINITY &&
                                          ri_p->stack_port_dist[VTSS_SPROUT__SP_B] == VTSS_SPROUT_DIST_INFINITY),
                                        (" "));

            
            if (ri_p->stack_port_dist[VTSS_SPROUT__SP_A] == VTSS_SPROUT_DIST_INFINITY) {
                sp_idx = VTSS_SPROUT__SP_B;
            } else if (ri_p->stack_port_dist[VTSS_SPROUT__SP_B] == VTSS_SPROUT_DIST_INFINITY) {
                sp_idx = VTSS_SPROUT__SP_A;
            } else if (ri_p->stack_port_dist[VTSS_SPROUT__SP_A] ==
                       ri_p->stack_port_dist[VTSS_SPROUT__SP_B]) {
                
                if (switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].port_no <
                    switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].port_no) {
                    sp_idx = VTSS_SPROUT__SP_A;
                } else {
                    sp_idx = VTSS_SPROUT__SP_B;
                }
            } else if (ri_p->stack_port_dist[VTSS_SPROUT__SP_A] <
                       ri_p->stack_port_dist[VTSS_SPROUT__SP_B]) {
                sp_idx = VTSS_SPROUT__SP_A;
            } else {
                sp_idx = VTSS_SPROUT__SP_B;
            }

            
            ui_p = vtss_sprout__ui_find(uit_sp_p[sp_idx],
                                        &ri_p->switch_addr,
                                        ri_p->unit_idx);
            VTSS_SPROUT_ASSERT_DBG_INFO(ui_p != NULL, (" "));
            

            
            ui_p->sp_idx = sp_idx;
            vtss_sprout__uit_update(&uit, ui_p, 0);
        }
    }

    update_local_uis();
} 




static vtss_rc force_proto_down(
    vtss_sprout__stack_port_idx_t sp_idx)
{
    vtss_rc                       rc = VTSS_OK;
    vtss_sprout__stack_port_state_t *sps_p;

    sps_p = &switch_state.unit[0].stack_port[sp_idx];

    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->proto_up, (" "));
    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->adm_up,   (" "));
    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->link_up,  (" "));

    
    vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);

    
    vtss_sprout__uit_init(&sps_p->uit);
    sps_p->uit.changed = 1;

    
    vtss_sprout__rit_infinity_all(&rit, sp_idx);

    
    rc = process_xit_changes(1, 1);

    sps_p->proto_up = 0;

#if defined(VTSS_SPROUT_FW_VER_CHK)
    
    sprout_init.callback.fw_ver_chk(sps_p->port_no, 0, NULL);
#endif

    return rc;
} 


static vtss_rc stack_port_adm_state_change(
    vtss_sprout__stack_port_idx_t sp_idx,
    BOOL                          adm_up)
{
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_rc                       rc = VTSS_OK;
    sps_p = &switch_state.unit[0].stack_port[sp_idx];

    T_N("stack_port_adm_state_change: sp_idx=%d, adm_up=%d",
        sp_idx, adm_up);

    if (sps_p->adm_up == adm_up) {
        
        return VTSS_OK;
    }

    if (!adm_up && sps_p->proto_up) {
        
        rc = force_proto_down(sp_idx);

#if defined(VTSS_SPROUT_FW_VER_CHK)
        
        sprout_init.callback.fw_ver_chk(sps_p->port_no, 0, NULL);
#endif
    } else {
        
        VTSS_SPROUT_ASSERT_DBG_INFO(!sps_p->proto_up, (" "));

        if (sps_p->link_up) {
            
            vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);
            tx_sprout_update(sp_idx, 0);
        }
    }

    switch_state.unit[0].stack_port[sp_idx].adm_up = adm_up;

    return rc;
} 





static vtss_rc stack_port_link_state_change(
    vtss_sprout__stack_port_idx_t sp_idx,
    BOOL                          link_up)
{
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_sprout__stack_port_state_t *sps_other_p;
    vtss_rc                          rc = VTSS_OK;

    T_DG(TRACE_GRP_FAILOVER, "link state change, sp_idx=%d", sp_idx);

    sps_p       = &switch_state.unit[0].stack_port[sp_idx];
    sps_other_p = &switch_state.unit[0].stack_port[!sp_idx];

    T_DG(TRACE_GRP_FAILOVER,
         "stack_port_link_state_change:\n"
         "Args: sp_idx=%d, link_up=%d\n"
         "Current state: sps.adm_up=%d sps.link_up=%d",
         sp_idx, link_up,
         sps_p->adm_up, sps_p->link_up);

    T_I("stack_port_link_state_change:\n"
        "Args: sp_idx=%d, port_no=%u, link_up=%d\n"
        "Current state: sps.adm_up=%d sps.link_up=%d",
        sp_idx, sps_p->port_no, link_up,
        sps_p->adm_up, sps_p->link_up);

    if (sps_p->link_up == link_up) {
        
        T_N("Ignoring link state change - no change.\n%s", dbg_info());
        return VTSS_OK;
    }

    if (link_up) {
        
        T_I("Link coming up, sp_idx=%d, port_no=%u",
            sp_idx, sps_p->port_no);
        switch_state.unit[0].stack_port[sp_idx].link_up = link_up;

        VTSS_SPROUT_ASSERT_DBG_INFO(!sps_p->proto_up, (" "));

        if (sps_p->adm_up) {
            
            vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);
            tx_sprout_update(sp_idx, 0);
        }
    } else if (!link_up) {
        
        T_I("Link going down, sp_idx=%d, port_no=%u",
            sp_idx, sps_p->port_no);
        switch_state.unit[0].stack_port[sp_idx].link_up = link_up;

        if (sps_p->proto_up) {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->adm_up, (" "));

#if defined(VTSS_SPROUT_V2)
            if (sps_other_p->proto_up) {
                
                tx_sprout_alert_protodown(!sp_idx);
            }
#endif
            sprout_init.callback.thread_set_priority_normal();

            
            vtss_sprout__uit_init(&sps_p->uit);
            sps_p->uit.changed = 1;

            
            vtss_sprout__rit_infinity_all(&rit, sp_idx);

            
            rc = process_xit_changes((sp_idx != 0), (sp_idx != 1));

            sps_p->proto_up = 0;

            if (!sps_other_p->link_up) {
                
                
                
                switch_state.virgin = 1;
            }

            
            vtss_sprout_switch_addr_init(&sps_p->nbr_switch_addr);
        }
#if defined(VTSS_SPROUT_FW_VER_CHK)
        
        sprout_init.callback.fw_ver_chk(sps_p->port_no, 0, NULL);
#endif
    }

    return VTSS_OK;
} 





static void clr_xit_flags(void)
{
    vtss_sprout__unit_idx_t       unit_idx;
    vtss_sprout__stack_port_idx_t sp_idx;

    T_R("clr_xit_flags: uit.changed=%d uit.change_mask=0x%x rit.changed=0x%x",
        uit.changed, uit.change_mask, rit.changed);

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            vtss_sprout__uit_clr_flags(&switch_state.unit[unit_idx].stack_port[sp_idx].uit);
        }
    }

    vtss_sprout__uit_clr_flags(&uit);
    vtss_sprout__rit_clr_flags(&rit);

    return;
} 





static vtss_rc stack_setup(void)
{
    vtss_sprout__unit_idx_t unit_idx;
    vtss_vstax_conf_t       vstax_setup;
    vtss_rc                 rc;

    vtss_appl_api_lock();
    (void)vtss_vstax_conf_get(NULL, &vstax_setup);
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        if (unit_idx == 0) {
            vstax_setup.upsid_0 = switch_state.unit[unit_idx].upsid;
        } else {
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
            vstax_setup.upsid_1 = switch_state.unit[unit_idx].upsid;
#else
            VTSS_SPROUT_ASSERT(FALSE, ("Unreachable"));
#endif
        }
    }
    rc = vtss_vstax_conf_set(NULL, &vstax_setup);
    vtss_appl_api_unlock();
    return rc;
} 





static vtss_rc stack_port_setup(
    vtss_sprout__unit_idx_t       unit_idx,
    vtss_sprout__stack_port_idx_t sp_idx)
{
    vtss_vstax_port_conf_t          vstax_port_setup;
    vtss_rc                         rc = VTSS_OK;
    vtss_sprout__stack_port_state_t *sps_p;
    BOOL                            stack_port_a = (sp_idx == VTSS_SPROUT__SP_A);
    vtss_chip_no_t                  chip_no = unit_idx;

    sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

    T_D("enter, unit_idx=%d, sp_idx=%d, ttl_current=%d, ttl_new=%d, mirror_fwd=%d",
        unit_idx, sp_idx, sps_p->ttl, sps_p->ttl_new, sps_p->mirror_fwd);

    rc = vtss_vstax_port_conf_get(NULL, chip_no, stack_port_a, &vstax_port_setup);
    VTSS_SPROUT_ASSERT_DBG_INFO(rc >= 0, ("rc=%d", rc));

    vstax_port_setup.ttl    = sps_p->ttl_new;
    vstax_port_setup.mirror = sps_p->mirror_fwd;


    sps_p->cfg_change = 0;
    sps_p->ttl        = sps_p->ttl_new;

    rc = vtss_vstax_port_conf_set(NULL, chip_no, stack_port_a, &vstax_port_setup);
    VTSS_SPROUT_ASSERT_DBG_INFO(rc >= 0, ("stack_port_setup(%d, %d): rc=%d",
                                          unit_idx, sp_idx,
                                          rc));

    if (unit_idx == 0) {
        
        vtss_vstax_conf_t vstax_setup;
        vtss_appl_api_lock();
        if (vtss_vstax_conf_get(NULL, &vstax_setup) == VTSS_RC_OK) {
            vstax_setup.port_0 = switch_state.unit[0].stack_port[0].port_no;
            vstax_setup.port_1 = switch_state.unit[0].stack_port[1].port_no;
            vtss_vstax_conf_set(NULL, &vstax_setup);
        }
        vtss_appl_api_unlock();
    }

    return rc;
} 





static vtss_rc process_xit_changes(
    BOOL allow_sprout_stack_port0, 
    BOOL allow_sprout_stack_port1  
)
{
    vtss_sprout__unit_idx_t          unit_idx;
    vtss_sprout__stack_port_idx_t    sp_idx;
    vtss_rc                          rc = VTSS_OK;
    vtss_sprout__stack_port_state_t *sps_pa[2];
    BOOL                             ttl_change = 0;

    sps_pa[0] = &switch_state.unit[0].stack_port[0];
    sps_pa[1] = &switch_state.unit[0].stack_port[1];

    T_N("process_xit_changes: allows=%d%d link_up=%d/%d "
        "uit_l.changed=%d/%d uit_l.mst_time_changed=%d/%d "
        "uit.changed/mask=%d/0x%x rit.changed=%d "
        "uit0.changed/mask=%d0x%x uit1.changed/mask=%d0x%x",
        allow_sprout_stack_port0, allow_sprout_stack_port1,
        sps_pa[0]->link_up, sps_pa[1]->link_up,
        sps_pa[0]->uit.changed, sps_pa[1]->uit.changed,
        sps_pa[0]->uit.mst_time_changed, sps_pa[1]->uit.mst_time_changed,
        uit.changed, uit.change_mask, rit.changed,
        sps_pa[0]->uit.changed, sps_pa[0]->uit.change_mask,
        sps_pa[1]->uit.changed, sps_pa[1]->uit.change_mask
       );

    
    merge_uits();

    T_N("rit.change=%d, sps_pa[0/1]->uit.changed=%d/%d uit.changed=%d uit.change_mask=0x%x",
        rit.changed, sps_pa[0]->uit.changed, sps_pa[1]->uit.changed, uit.changed, uit.change_mask);

    
    if (uit.changed ||
        rit.changed ||
        uit.mst_time_changed) {
        BOOL mst = switch_state.mst;
        if (mst_calc()) {
            
            uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_NEW_MST;
            if (mst != switch_state.mst) {
                if (switch_state.mst) {
                    
                    uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST;
                } else {
                    
                    uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_ME_SLV;
                }
            }
        }
    }

    T_N("rit.change=%d, sps_pa[0/1]->uit.changed=%d/%d uit.changed=%d uit.change_mask=0x%x",
        rit.changed, sps_pa[0]->uit.changed, sps_pa[1]->uit.changed, uit.changed, uit.change_mask);

    
    if (!rit.changed &&
        !sps_pa[0]->uit.changed &&
        !sps_pa[1]->uit.changed &&
        !uit.changed &&
        !uit.change_mask 
       ) {
        

        if (sps_pa[0]->uit.mst_time_changed ||
            sps_pa[1]->uit.mst_time_changed ||
            uit.mst_time_changed) {
            T_N("process_xit_changes: Only mst_time change");
            
            clr_xit_flags();

            
            if (allow_sprout_stack_port0 && sps_pa[0]->proto_up) {
                VTSS_SPROUT_ASSERT_DBG_INFO(sps_pa[0]->adm_up && sps_pa[0]->link_up, (" "));
                if (internal_cfg.sprout_triggered_updates) {
                    tx_sprout_update(0, 0);
                }
            }
            if (allow_sprout_stack_port1 && sps_pa[1]->proto_up) {
                VTSS_SPROUT_ASSERT_DBG_INFO(sps_pa[1]->adm_up && sps_pa[1]->link_up, (" "));
                if (internal_cfg.sprout_triggered_updates) {
                    tx_sprout_update(1, 0);
                }
            }
        } else {
            T_N("process_xit_changes: No changes found, not even mst_time");
            clr_xit_flags();
        }
        return VTSS_OK;
    }
    

    if (rit.changed) {
        topology_determination();
#if defined(VTSS_SPROUT_V1)
        top_switch_calc();
#endif
        ttl_calc();
    }

    if (rit.changed || uit.changed) {
        mirror_calc();
    }

    
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            vtss_sprout__stack_port_state_t *sps_p;
            sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

            if (sps_p->cfg_change &&
                sps_p->ttl_new < sps_p->ttl) {
                rc = stack_port_setup(unit_idx, sp_idx);
                ttl_change = 1;
                if (rc < 0) {
                    VTSS_SPROUT_ASSERT_DBG_INFO(rc < 0, ("rc=%d", rc));
                    return rc;
                }
            }
        }
    }

    
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            vtss_sprout__stack_port_state_t *sps_p;
            sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

            if (sps_p->cfg_change &&
                sps_p->ttl_new >= sps_p->ttl) {
                ttl_change |= (sps_p->ttl_new != sps_p->ttl);
                rc = stack_port_setup(unit_idx, sp_idx);
                if (rc < 0) {
                    VTSS_SPROUT_ASSERT_DBG_INFO(rc < 0, ("rc=%d", rc));
                    return rc;
                }
            }
        }
    }

    
    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            vtss_sprout__stack_port_state_t *sps_p;
            sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

            if (sps_p->cfg_change) {
                rc = stack_port_setup(unit_idx, sp_idx);
                if (rc < 0) {
                    return rc;
                }
            }
        }
    }

#if defined(VTSS_SPROUT_V2)
    if (rit.changed) {
        
        vtss_vstax_upsid_t       upsid;
        vtss_sprout__unit_idx_t  unit_idx;
        vtss_sprout__upsid_idx_t ups_idx;
        vtss_sprout__ri_t        *ri_p = NULL;
        vtss_vstax_route_table_t upsid_route;

        
        for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
            
            memset(&upsid_route, 0, sizeof(upsid_route));

            
            while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
                
                BOOL use_stack_port_a = 0;
                BOOL use_stack_port_b = 0;

                
                vtss_sprout_dist_t dist_a, dist_b;

                BOOL switch_me =
                    (vtss_sprout__switch_addr_cmp(&ri_p->switch_addr,
                                                  &switch_state.switch_addr) == 0);

                BOOL unit_me = (switch_me && ri_p->unit_idx == unit_idx);

                if (unit_idx == 0) {
                    
                    dist_a = ri_p->stack_port_dist[VTSS_SPROUT__SP_A];
                    dist_b = ri_p->stack_port_dist[VTSS_SPROUT__SP_B];
                } else {
                    
                    
                    
                    if (!switch_me) {
                        
                        dist_b = (ri_p->stack_port_dist[VTSS_SPROUT__SP_A] == VTSS_SPROUT_DIST_INFINITY) ?
                                 VTSS_SPROUT_DIST_INFINITY :
                                 ri_p->stack_port_dist[VTSS_SPROUT__SP_A] + 1;
                        dist_a = (ri_p->stack_port_dist[VTSS_SPROUT__SP_B] == VTSS_SPROUT_DIST_INFINITY) ?
                                 VTSS_SPROUT_DIST_INFINITY :
                                 ri_p->stack_port_dist[VTSS_SPROUT__SP_B] - 1;
                    } else {
                        
                        dist_b = 1;
                        dist_a = (ri_p->stack_port_dist[VTSS_SPROUT__SP_B] == VTSS_SPROUT_DIST_INFINITY) ?
                                 VTSS_SPROUT_DIST_INFINITY :
                                 ri_p->stack_port_dist[VTSS_SPROUT__SP_B] - 1;
                        T_I("dist_a=%d, dist_b=%d", dist_a, dist_b);
                    }
                }

                VTSS_SPROUT_ASSERT_DBG_INFO(dist_a != VTSS_SPROUT_DIST_INFINITY ||
                                            dist_b != VTSS_SPROUT_DIST_INFINITY, (" "));

                
                if (dist_b == VTSS_SPROUT_DIST_INFINITY) {
                    use_stack_port_a = 1;
                } else if (dist_a == VTSS_SPROUT_DIST_INFINITY) {
                    use_stack_port_b = 1;
                } else if (dist_a < dist_b) {
                    use_stack_port_a = 1;
                } else if (dist_a > dist_b) {
                    use_stack_port_b = 1;
                } else if (dist_a == dist_b) {
                    if (switch_state.topology_type_new == VtssTopoOpenLoop) {
                        
                        
                        
                        
                        T_I("OpenLoop, but UPSID %d is reachable with same distance through both stack ports. "
                            "Using stack port with largest TTL.", ri_p->upsid[0]);
                        if (switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_A].ttl_new >
                            switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_B].ttl_new) {
                            use_stack_port_a = 1;
                        } else if (switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_A].ttl_new <
                                   switch_state.unit[unit_idx].stack_port[VTSS_SPROUT__SP_B].ttl_new) {
                            use_stack_port_b = 1;
                        } else {
                            
                            T_E("unit_idx=%d, dist_a=%d, dist_b=%d, rit=%s",
                                unit_idx,
                                dist_a, dist_b,
                                vtss_sprout__rit_to_str(&rit));
                            VTSS_SPROUT_ASSERT_DBG_INFO(0, (" "));
                        }
                    } else {
                        
                        use_stack_port_a = 1;
                        use_stack_port_b = 1;
                    }
                } else {
                    T_E("What?! dist_a/b=%d/%d", dist_a, dist_b);
                }

                
                
                for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                    upsid = ri_p->upsid[ups_idx];

                    
                    if (unit_me) {
                        continue;
                    }

                    if (upsid != VTSS_VSTAX_UPSID_UNDEF) {
                        upsid_route.table[upsid].stack_port_a = use_stack_port_a;
                        upsid_route.table[upsid].stack_port_b = use_stack_port_b;
                    }
                }
            }
            upsid_route.topology_type =
                (switch_state.topology_type_new == VtssTopoOpenLoop) ?
                VTSS_VSTAX_TOPOLOGY_CHAIN : VTSS_VSTAX_TOPOLOGY_RING;

            if (vtss_board_chipcount() == 2) {
                
                
                upsid_route.table[switch_state.unit[!unit_idx].upsid].stack_port_a = 0;
                upsid_route.table[switch_state.unit[!unit_idx].upsid].stack_port_b = 1;
            }

            vtss_vstax_topology_set(NULL,
                                    unit_idx,
                                    &upsid_route);
        }
    }
#endif
    if (ttl_change) {
#if defined(VTSS_SPROUT_V1)
        
        vtss_mac_table_port_flush(NULL, sps_pa[VTSS_SPROUT__SP_A]->port_no);
        vtss_mac_table_port_flush(NULL, sps_pa[VTSS_SPROUT__SP_B]->port_no);
#endif

        uit.change_mask |= VTSS_SPROUT_STATE_CHANGE_MASK_TTL;
    }

#if defined(VTSS_SPROUT_V1)
    
    if (switch_state.topology_type_new == VtssTopoBack2Back ||
        switch_state.topology_type     == VtssTopoBack2Back) {
        
        BOOL member[VTSS_PORT_ARRAY_SIZE];

        memset(member, 0, sizeof(member));

        member[sps_pa[VTSS_SPROUT__SP_A]->port_no] = (switch_state.topology_type_new == VtssTopoBack2Back);
        member[sps_pa[VTSS_SPROUT__SP_B]->port_no] = (switch_state.topology_type_new == VtssTopoBack2Back);
        rc = vtss_aggr_port_members_set(NULL, VTSS_AGGR_NO_END - 1, member);
        if (rc < 0) {
            return rc;
        }
    }
#endif
    switch_state.topology_type = switch_state.topology_type_new;

    
    if (allow_sprout_stack_port0 && sps_pa[0]->proto_up) {
        VTSS_SPROUT_ASSERT_DBG_INFO(sps_pa[0]->adm_up && sps_pa[0]->link_up, (" "));
        if (internal_cfg.sprout_triggered_updates) {
            tx_sprout_update(0, 0);
        }
    }
    if (allow_sprout_stack_port1 && sps_pa[1]->proto_up) {
        VTSS_SPROUT_ASSERT_DBG_INFO(sps_pa[1]->adm_up && sps_pa[1]->link_up, (" "));
        if (internal_cfg.sprout_triggered_updates) {
            tx_sprout_update(1, 0);
        }
    }

    
    T_IG(VTSS_TRACE_GRP_TOP_CHG,
         "UIT/RIT has changed (change_mask=0x%x). New topology information:\n"
         "topology_type=%s\n"
         "topology_n=%d\n"
#if defined(VTSS_SPROUT_V1)
         "top_switch=%s\n"
#endif
         "master_switch=%s\n"
         "ttl[0]=%d\n"
         "ttl[1]=%d\n"
         "%s",
         uit.change_mask,
         vtss_sprout_topology_type_to_str(switch_state.topology_type),
         switch_state.topology_n,
#if defined(VTSS_SPROUT_V1)
         switch_state.ui_top_p ?
         vtss_sprout_switch_addr_to_str(&switch_state.ui_top_p->switch_addr) :
         "-",
#endif
         vtss_sprout_switch_addr_to_str(&switch_state.mst_switch_addr),
         switch_state.unit[0].stack_port[0].ttl,
         switch_state.unit[0].stack_port[1].ttl,
         vtss_sprout__rit_to_str(&rit));

    return VTSS_OK;
} 


static vtss_rc state_change_callback(void)
{
    vtss_rc               rc = VTSS_OK;
    BOOL                  glag_mbrs_found = 0;
    uchar                 topo_change_mask = 0;

#if defined(VTSS_SPROUT_V1)
    if (rit.changed) {
        

        vtss_sprout__ui_t     *ui_p     = NULL;
        while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
            uint mbr_cnt = 0;
            vtss_sprout__glagid_t glagid;
            for (glagid = 0; glagid < VTSS_GLAGS; glagid++) {
                mbr_cnt += ui_p->glag_mbr_cnt[glagid];
                if (mbr_cnt) {
                    glag_mbrs_found = 1;
                    break;
                };
            }
            if (mbr_cnt) {
                glag_mbrs_found = 1;
                break;
            };
        }
    }
#else
    glag_mbrs_found = 0;
#endif

    
    if (uit.changed ||
        (rit.changed && glag_mbrs_found) ||
        (uit.change_mask)) {
        topo_change_mask =
            (rit.changed * VTSS_SPROUT_STATE_CHANGE_MASK_STACK_MBR) |
            uit.change_mask;
#if defined(VTSS_SPROUT_V1)
        topo_change_mask |=
            


            ((rit.changed && glag_mbrs_found) * VTSS_SPROUT_STATE_CHANGE_MASK_GLAG);
#endif

        if (topo_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_TTL) {
            
            switch_state.topology_change_time = sprout_init.callback.secs_since_boot();
        }
    }

    clr_xit_flags();

    
    if (topo_change_mask) {
        T_D("sprout_init.callback.state_change(%x)", topo_change_mask);

        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        rc = sprout_init.callback.state_change(topo_change_mask);
        VTSS_SPROUT_CRIT_ENTER_TBL_RD();
    }

    return rc;
} 










#if defined(VTSS_SPROUT_MAIN)
int main(void)
{
    return 0;
}
#endif 






vtss_rc vtss_sprout_init(
    const vtss_sprout_init_t *const init)
{
    
    VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
    VTSS_TRACE_REGISTER(&trace_reg);

    T_D("enter");

    VTSS_SPROUT_ASSERT(sprout_init_done == 0, ("?"));
    VTSS_SPROUT_ASSERT(switch_init_done == 0, ("?"));

    VTSS_SPROUT_ASSERT(init != NULL, (" "));

    VTSS_SPROUT_ASSERT(init->callback.log_msg               != NULL, ("?"));
    VTSS_SPROUT_ASSERT(init->callback.cfg_save              != NULL, ("?"));
    VTSS_SPROUT_ASSERT(init->callback.state_change          != NULL, ("?"));
    VTSS_SPROUT_ASSERT(init->callback.tx_vstax2_pkt         != NULL, ("?"));

    sprout_init      = *init;
    sprout_init_done = 1;

    vtss_sprout__switch_state_init(&switch_state);

#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
    if (vtss_board_chipcount() == 2) {
        
        switch_state.unit[0].stack_port[1].ttl     = 1;
        switch_state.unit[0].stack_port[1].ttl_new = 1;
        switch_state.unit[1].stack_port[1].ttl     = 1;
        switch_state.unit[1].stack_port[1].ttl_new = 1;
    }
#endif

    
    switch_state.topology_change_time = sprout_init.callback.secs_since_boot();

    vtss_sprout__rit_init(&rit);
    vtss_sprout__uit_init(&uit);

    internal_cfg.sprout_periodic_updates  = VTSS_SPROUT_SPROUT_PERIODIC_UPDATES;
    internal_cfg.sprout_triggered_updates = VTSS_SPROUT_SPROUT_TRIGGERED_UPDATES;
    internal_cfg.sprout_aging             = VTSS_SPROUT_SPROUT_AGING;

    



    VTSS_SPROUT_CRIT_INIT_TBL_RD();
    VTSS_SPROUT_CRIT_INIT_STATE_DATA();
    VTSS_SPROUT_CRIT_INIT_DBG();

    return VTSS_OK;
} 





vtss_rc vtss_sprout_switch_init(
    const vtss_sprout_switch_init_t *const setup)
{
    uint i = 0;
    uint j = 0;
    vtss_rc                       rc = VTSS_OK;
    vtss_sprout__unit_idx_t         unit_idx;
    vtss_sprout__stack_port_idx_t   sp_idx;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_port_no_t                   port;
    vtss_vstax_upsid_t               upsids_pref[2];

    
    vtss_sprout_switch_addr_t vtss_none = {{0x00, 0x01, 0xc1, 0x00, 0x00, 0x00}};
    vtss_sprout_switch_addr_t null_addr = {{0, 0, 0, 0, 0, 0}};

    BOOL upsids_changed = 0;
    BOOL upsid_recalc_needed = 0;

    T_D("enter");

    VTSS_SPROUT_CRIT_ASSERT_TBL_RD(1);
    VTSS_SPROUT_ASSERT(setup != NULL, (" "));

    VTSS_SPROUT_ASSERT_DBG_INFO(sprout_init_done == 1, (" "));
    VTSS_SPROUT_ASSERT_DBG_INFO(switch_init_done == 0, (" "));

    switch_state.switch_addr    = setup->switch_addr;

    if (vtss_sprout__switch_addr_cmp(&switch_state.switch_addr, &vtss_none) == 0 ||
        vtss_sprout__switch_addr_cmp(&switch_state.switch_addr, &null_addr) == 0) {
        T_E("Illegal switch address: %s. Appears not to be unique. Unique switch address required.",
            vtss_sprout_switch_addr_to_str(&switch_state.switch_addr));
        T_E("setup->switch_addr=%s",
            vtss_sprout_switch_addr_to_str(&setup->switch_addr));

        
        
        
        
    }

    switch_state.cpu_qu         = setup->cpu_qu;
    switch_state.mst_capable    = setup->mst_capable;
#if VTSS_SPROUT_UNMGD
    VTSS_SPROUT_ASSERT_DBG_INFO(setup->mst_capable == 0, (" "));
#endif
    memcpy(switch_state.switch_appl_info, setup->switch_appl_info, VTSS_SPROUT_SWITCH_APPL_INFO_LEN);

#if defined(VTSS_SPROUT_FW_VER_CHK)
    memcpy(switch_state.my_fw_ver, setup->my_fw_ver, VTSS_SPROUT_FW_VER_LEN);
#endif

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        for (sp_idx = 0; sp_idx < 2; sp_idx++) {
            sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];

            
            sps_p->update_bucket = sps_p->update_limit;
            sps_p->alert_bucket  = sps_p->alert_limit;
        }
    }

    
    
    
    for (i = 0; i < ARRSZ(upsids_pref); i++) {
        upsids_pref[i] = VTSS_VSTAX_UPSID_UNDEF;
    }
    upsids_pref[0] = setup->chip[0].upsid_pref;
    upsids_pref[1] = setup->chip[1].upsid_pref;

    T_DG(TRACE_GRP_UPSID, "upsids_pref[0] = %d, upsids_pref[1] = %d", upsids_pref[0], upsids_pref[1]);

    
    
    
    
    
    
    
    
    
    
    if (upsids_pref[0] != VTSS_VSTAX_UPSID_UNDEF && is_odd(upsids_pref[0])) {
        
        T_DG(TRACE_GRP_UPSID, "First UPSID (%d) is not even", upsids_pref[0]);
        for (i = 0; i < ARRSZ(upsids_pref); i++) {
            upsids_pref[i] = VTSS_VSTAX_UPSID_UNDEF;
        }
    }
    for (i = 1; i < ARRSZ(upsids_pref); i++) {
        if (upsids_pref[i] != VTSS_VSTAX_UPSID_UNDEF &&
            upsids_pref[i] != upsids_pref[i - 1] + 1) {
            T_DG(TRACE_GRP_UPSID, "UPSIDs are not consecutive (upsids_prev[%d - 0] = %d, upsids_prev[%d - 1] = %d)", i, upsids_pref[i], i, upsids_pref[i - 1]);
            
            for (i = 0; i < ARRSZ(upsids_pref); i++) {
                upsids_pref[i] = VTSS_VSTAX_UPSID_UNDEF;
            }
            break;
        }
    }

    
    for (i = 0; i < ARRSZ(upsids_pref); i++) {
        if (upsids_pref[i] == VTSS_VSTAX_UPSID_UNDEF) {
            continue;
        }

        for (j = 0; j < ARRSZ(upsids_pref); j++) {
            if (i == j) {
                continue;
            }

            if (upsids_pref[i] == upsids_pref[j]) {
                T_W("Switch has same preferred UPSID %d for different local UPS?!", upsids_pref[i]);
                T_DG(TRACE_GRP_UPSID, "Same preferred UPSIDs (i = %d, j = %d, upsid = %d", i, j, upsids_pref[i]);
                upsids_pref[j] = VTSS_VSTAX_UPSID_UNDEF;
            }
        }
    }

    for (i = 0; i < ARRSZ(upsids_pref); i++) {
        
        VTSS_SPROUT_ASSERT_DBG_INFO(VTSS_VSTAX_UPSID_LEGAL(upsids_pref[i]) ||
                                    upsids_pref[i] == VTSS_VSTAX_UPSID_UNDEF,
                                    (" "));
    }

    for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
        const vtss_sprout_chip_setup_t *chip;
        chip = &setup->chip[unit_idx];

        
        for (port = 0; port < VTSS_PORT_ARRAY_SIZE; port++) {
            
            if (port_custom_table[port].map.chip_port != CHIP_PORT_UNUSED &&
                port_custom_table[port].map.chip_no == unit_idx &&
                port != setup->chip[0].stack_port[0].port_no &&
                port != setup->chip[0].stack_port[1].port_no) {
                switch_state.unit[unit_idx].ups_port_mask |= ((u64)1 << port);
            }
        }

        if (upsids_pref[unit_idx] == VTSS_VSTAX_UPSID_UNDEF) {
            




            T_DG(TRACE_GRP_UPSID, "Recalc_needed[%d]", unit_idx);
            upsid_recalc_needed = 1;
        } else {
            switch_state.unit[unit_idx].upsid = upsids_pref[unit_idx];
        }

        for (j = 0; j < 2; j++) {
            
            VTSS_SPROUT_ASSERT_DBG_INFO(switch_init_done == 0 ||
                                        (switch_state.unit[unit_idx].stack_port[j].port_no ==
                                         chip->stack_port[j].port_no), (" "));

            switch_state.unit[unit_idx].stack_port[j].port_no = chip->stack_port[j].port_no;
        }
    }

    if (upsid_recalc_needed) {
        
        BOOL upsid_inuse_mask[VTSS_SPROUT_MAX_UNITS_IN_STACK];
        memset(upsid_inuse_mask, 0, sizeof(upsid_inuse_mask));

        rc = upsids_set_random_unused(upsid_inuse_mask);

        
        VTSS_SPROUT_ASSERT_DBG_INFO(rc == VTSS_OK, (" "));

        upsids_changed = 1;
    }

    

    for (i = 0; i < 6; i++) {
        sprout_smac.addr[i] = setup->switch_addr.addr[i];
    }
    sprout_smac.addr[0] = sprout_smac.addr[0] & 0xfe;

#if defined(VTSS_SPROUT_V2)
    {
        uchar *pkt_p;
        uint  byte_idx = 0;


        
        
        pkt_init_vs2_hdr(&sprout_alert_protodown_vs2_hdr, 0, 0);
        pkt_p = sprout_alert_protodown_pkt;
        pkt_init_sprout_hdr(pkt_p, &byte_idx, sprout_pdu_type_alert);
        pkt_add_bytes(pkt_p, &byte_idx, switch_state.switch_addr.addr, 6);
        pkt_add_byte(pkt_p, &byte_idx, SPROUT_ALERT_TYPE_PROTODOWN);
        pkt_add_byte(pkt_p, &byte_idx, 0x0);

        VTSS_SPROUT_ASSERT(byte_idx == SPROUT_ALERT_PROTODOWN_PDU_LEN,
                           ("byte_idx=%d, len=%d", byte_idx, SPROUT_ALERT_PROTODOWN_PDU_LEN));
    }
#endif

    if (!switch_init_done) {
        
        
        for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
            for (sp_idx = 0; sp_idx < 2; sp_idx++) {
                stack_port_setup(unit_idx, sp_idx);
            }
        }

        switch_init_done = 1;
    }

    if (upsids_changed) {
        T_DG(TRACE_GRP_UPSID, "Calling cfg_save()");
        cfg_save();
    }

    
    
    stack_setup();

#if defined(VTSS_SPROUT_V2)
    if (vtss_board_chipcount() == 2) {
        
        vtss_vstax_route_table_t upsid_route;
        vtss_sprout__unit_idx_t  unit_idx;
        for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
            memset(&upsid_route, 0, sizeof(upsid_route));
            upsid_route.topology_type = VTSS_VSTAX_TOPOLOGY_CHAIN;

            
            upsid_route.table[switch_state.unit[!unit_idx].upsid].stack_port_b = 1;

            vtss_vstax_topology_set(NULL,
                                    unit_idx,
                                    &upsid_route);
        }
    }
#endif

    update_local_uis();

    
    
    rc = process_xit_changes(1, 1);
    if (rc < 0) {
        clr_xit_flags();
        return rc;
    }

    rc = state_change_callback();

    
    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    VTSS_SPROUT_CRIT_EXIT_DBG();

    return rc;
} 





vtss_rc vtss_sprout_stack_port_adm_state_set(
    const vtss_port_no_t port_no,
    const BOOL           adm_up)
{
    vtss_sprout__stack_port_idx_t sp_idx = 0;
    BOOL                        found = 0;
    vtss_rc                     rc = VTSS_OK;

    T_D("enter, port_no=%u, adm_up=%d", port_no, adm_up);

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    
    while (!found && (sp_idx < 2)) {
        if (switch_state.unit[0].stack_port[sp_idx].port_no == port_no) {
            found = 1;
        }

        if (!found) {
            sp_idx++;
        }
    }

    if (!found) {
        T_E("Unknown port_no: %u", port_no);
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return VTSS_INVALID_PARAMETER;
    }

    if (switch_state.unit[0].stack_port[sp_idx].adm_up == adm_up) {
        
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return VTSS_OK;
    }

    rc = stack_port_adm_state_change(sp_idx, adm_up);
    if (rc < 0) {
        clr_xit_flags();
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }

    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();

    return rc;
} 





vtss_rc vtss_sprout_stack_port_link_state_set(
    const vtss_port_no_t port_no,
    const BOOL           link_up)
{
    vtss_sprout__stack_port_idx_t sp_idx = 0;
    BOOL                        found = 0;
    vtss_rc                     rc = VTSS_OK;

    T_I("enter, port_no=%u, link_up=%d", port_no, link_up);

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    
    
    
    
    for (sp_idx = 0; sp_idx < 2; sp_idx++) {
        if (switch_state.unit[0].stack_port[sp_idx].port_no ==
            port_no) {
            
            found = 1;
            break;
        }
    }

    T_D("enter, port_no=%u, link_up=%d, sp_idx=%d", port_no, link_up, sp_idx);

    if (!found) {
        T_E("Unknown port_no: %u\n%s", port_no, dbg_info());
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return VTSS_INVALID_PARAMETER;
    }

    if (switch_state.unit[0].stack_port[sp_idx].link_up == link_up) {
        
        T_D("No link state change, weird. port_no=%u link_up=%d\n%s", port_no, link_up, dbg_info());
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return VTSS_OK;
    }

    rc = stack_port_link_state_change(sp_idx, link_up);
    if (rc < 0) {
        clr_xit_flags();
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }

    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();

    return rc;
} 


#if defined(VTSS_SPROUT_V1)
vtss_rc vtss_sprout_glag_mbr_cnt_set(
    const vtss_glag_no_t       glag_no,
    const uint                 glag_mbr_cnt)
{
    vtss_rc               rc = VTSS_OK;
    vtss_sprout__unit_idx_t unit_idx = 0;

#if VTSS_SPROUT_UNMGD
    return VTSS_UNSPECIFIED_ERROR;
#endif

    T_D("enter, glag_no=%u, glag_mbr_cnt=%d", glag_no, glag_mbr_cnt);

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    VTSS_SPROUT_ASSERT(glag_no <= VTSS_GLAGS,         ("glag_no=%u", glag_no));

    switch_state.unit[unit_idx].glag_mbr_cnt[glag_no - 1] = glag_mbr_cnt;

    update_local_uis();
    rc = process_xit_changes(1, 1);
    if (rc < 0) {
        T_E("rc=%d\n%s", rc, dbg_info());
        clr_xit_flags();

        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }

    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    return rc;
} 
#endif 


vtss_rc vtss_sprout_have_mirror_port_set(
    const vtss_sprout_chip_idx_t chip_idx,
    const BOOL                   have_mirror_port)
{
    vtss_sprout__unit_idx_t unit_idx;
    vtss_rc                 rc = VTSS_OK;

#if VTSS_SPROUT_UNMGD
    return VTSS_UNSPECIFIED_ERROR;
#endif

    T_D("enter, have_mirror_port=%d", have_mirror_port);

    VTSS_SPROUT_ASSERT_DBG_INFO(chip_idx <= vtss_board_chipcount(), (" "));

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    unit_idx = chip_idx - 1;

    switch_state.unit[unit_idx].have_mirror_port = have_mirror_port;

    update_local_uis();

    
    
    rc = process_xit_changes(1, 1);
    if (rc < 0) {
        T_E("rc=%d\n%s", rc, dbg_info());
        clr_xit_flags();
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }

    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    return rc;
} 


vtss_rc vtss_sprout_ipv4_addr_set(
    
    const vtss_ipv4_t ipv4_addr)
{
    vtss_rc               rc = VTSS_OK;

#if VTSS_SPROUT_UNMGD
    return VTSS_UNSPECIFIED_ERROR;
#endif

    T_D("enter, ipv4_addr=%u.%u.%u.%u",
        (ipv4_addr >> 24) & 0xff,
        (ipv4_addr >> 16) & 0xff,
        (ipv4_addr >>  8) & 0xff,
        (ipv4_addr >>  0) & 0xff);


    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    switch_state.ip_addr = ipv4_addr;

    update_local_uis();

    
    
    rc = process_xit_changes(1, 1);
    if (rc < 0) {
        T_E("rc=%d\n%s", rc, dbg_info());
        clr_xit_flags();
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }


    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    return rc;
} 


vtss_rc vtss_sprout_switch_appl_info_set(
    const vtss_sprout_switch_appl_info_t switch_appl_info_val,
    const vtss_sprout_switch_appl_info_t switch_appl_info_mask)
{
    vtss_rc               rc = VTSS_OK;
    uint byte_idx;

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    for (byte_idx = 0; byte_idx < VTSS_SPROUT_SWITCH_APPL_INFO_LEN; byte_idx++) {
        if (switch_appl_info_mask[byte_idx] != 0) {
            switch_state.switch_appl_info[byte_idx] =
                ((switch_state.switch_appl_info[byte_idx] & ~switch_appl_info_mask[byte_idx]) |
                 switch_appl_info_val[byte_idx]);
        }
    }

    update_local_uis();

    
    
    rc = process_xit_changes(1, 1);
    if (rc < 0) {
        T_E("rc=%d\n%s", rc, dbg_info());
        clr_xit_flags();
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return rc;
    }


    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    return rc;
} 



vtss_rc vtss_sprout_service_100msec(void)
{
    uint sp_idx;
    uint update_bucket_add = 0;
    uint alert_bucket_add  = 0;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_rc                       rc = VTSS_OK;

    T_R("enter");

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    for (sp_idx = 0; sp_idx < 2; sp_idx++) {
        sps_p = &switch_state.unit[0].stack_port[sp_idx];

        sps_p->deci_secs_since_last_tx_update++;

        if (internal_cfg.sprout_aging) {
            
            sps_p->deci_secs_since_last_rx_update++;
            if (sps_p->deci_secs_since_last_rx_update >= sps_p->update_age_time * 10) {
                if (sps_p->proto_up) {
                    




                    T_I("No updates seen on sp_idx=%d => Taking proto down", sp_idx);
                    rc = force_proto_down(sp_idx);
                    if (rc < 0) {
                        clr_xit_flags();
                        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
                        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
                        return rc;
                    }
                    VTSS_SPROUT_ASSERT_DBG_INFO(sps_p->proto_up == 0, (" "));
                }
            }
        }

        if (internal_cfg.sprout_periodic_updates) {
            
            if ((switch_state.mst == 0 &&
                 sps_p->deci_secs_since_last_tx_update >= sps_p->update_interval_slv * 10)
                ||
                (switch_state.mst == 1 &&
                 sps_p->deci_secs_since_last_tx_update >= sps_p->update_interval_mst * 10)) {
                if (sps_p->link_up) {
                    
                    T_N("Periodic SPROUT update on sp_idx=%d (secs since last update: %d, mst=%d)",
                        sp_idx, sps_p->deci_secs_since_last_tx_update, switch_state.mst);
                    tx_sprout_update(sp_idx, 1);
                }
            }
        }

        
        if (sps_p->update_bucket < sps_p->update_limit) {
            sps_p->deci_secs_since_last_update_bucket_filling++;

            update_bucket_add =
                (sps_p->deci_secs_since_last_update_bucket_filling * sps_p->update_limit) / 10;
            if (update_bucket_add > 0) {
                sps_p->update_bucket += update_bucket_add;
                sps_p->deci_secs_since_last_update_bucket_filling =
                    (sps_p->deci_secs_since_last_update_bucket_filling * sps_p->update_limit) % 10;
            }
            if (sps_p->update_bucket >= sps_p->update_limit) {
                sps_p->update_bucket = sps_p->update_limit;
                sps_p->deci_secs_since_last_update_bucket_filling = 0;
            }
        } else {
            sps_p->deci_secs_since_last_update_bucket_filling = 0;
        }

        
        if (sps_p->alert_bucket < sps_p->alert_limit) {
            sps_p->deci_secs_since_last_alert_bucket_filling++;

            alert_bucket_add =
                (sps_p->deci_secs_since_last_alert_bucket_filling * sps_p->alert_limit) / 10;
            if (alert_bucket_add > 0) {
                sps_p->alert_bucket += alert_bucket_add;
                sps_p->deci_secs_since_last_alert_bucket_filling =
                    (sps_p->deci_secs_since_last_alert_bucket_filling * sps_p->alert_limit) % 10;
            }
            if (sps_p->alert_bucket >= sps_p->alert_limit) {
                sps_p->alert_bucket = sps_p->alert_limit;
                sps_p->deci_secs_since_last_alert_bucket_filling = 0;
            }
        } else {
            sps_p->deci_secs_since_last_alert_bucket_filling = 0;
        }
    }

    rc = state_change_callback();

    VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    return rc;
} 











vtss_rc vtss_sprout_rx_pkt(
    const vtss_port_no_t  port_no,
    const uchar *const    frame,
    const uint            length)
{
    vtss_rc                       rc;
    vtss_sprout__stack_port_idx_t sp_idx;
    BOOL                          found = 0;
    uchar                         pdu_type;
    uint                          byte_idx;

    T_N("vtss_sprout_rx_pkt: port_no=%u, length=%d", port_no, length);

    VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
    VTSS_SPROUT_CRIT_ENTER_TBL_RD();

    
    {
        ushort word16;
        word16 = ((frame[12] << 8) | (frame[13] << 0));
        VTSS_SPROUT_ASSERT(word16 == 0x8880,
                           ("Unexpected EtherType=0x%08x, expected 0x8880", word16));
        word16 = ((frame[14] << 8) | (frame[15] << 0));
        VTSS_SPROUT_ASSERT(word16 == 0x0002,
                           ("Unexpected EPID=0x%08x, expected 0x0002", word16));
        word16 = ((frame[16] << 8) | (frame[17] << 0));
        VTSS_SPROUT_ASSERT(word16 == 0x0001,
                           ("Unexpected SSPID=0x%08x, expected 0x0001", word16));
        
        
    }

    
    
    
    
    for (sp_idx = 0; sp_idx < 2; sp_idx++) {
        if (switch_state.unit[0].stack_port[sp_idx].port_no ==
            port_no) {
            
            found = 1;
            break;
        }
    }

    if (found) {
        
        byte_idx = 20;
        pdu_type = pkt_get_byte(frame, &byte_idx);

        switch (pdu_type) {
        case SPROUT_PDU_TYPE_UPDATE:
            rc = rx_sprout_update(
                     port_no,
                     sp_idx,
                     frame,
                     length);
            break;
#if defined(VTSS_SPROUT_V2)
        case SPROUT_PDU_TYPE_ALERT:
            rc = rx_sprout_alert(
                     port_no,
                     sp_idx,
                     frame,
                     length);
            break;
#endif
        default:
            T_W("Unknown PDU type: 0x%02x", pdu_type);
            T_W_HEX(frame, length);
            VTSS_SPROUT_CRIT_EXIT_TBL_RD();
            VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
            return VTSS_OK;
        }

        if (rc < 0) {
            clr_xit_flags();
            VTSS_SPROUT_CRIT_EXIT_TBL_RD();
            VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
            return rc;
        }

        rc = state_change_callback();

        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();

        return rc;
    } else {
        
        T_N("Frame received on unexpected port: %u", port_no);
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
        return VTSS_UNSPECIFIED_ERROR;
    }
} 


void vtss_sprout_sit_get(
    vtss_sprout_sit_t *const sit)
{
    vtss_sprout__ui_t *ui_p;
    vtss_sprout__ri_t *ri_p;
    uint i = 0;
    uint sp_path; 
    vtss_sprout_switch_addr_t *prv_switch_addr_p = NULL;
    uint chip_idx;
    uint switch_cnt = 0;

    T_D("enter");

    if (!ignore_semaphores) {
        VTSS_SPROUT_CRIT_ENTER_TBL_RD();
    }

    VTSS_SPROUT_ASSERT(sit != NULL, (" "));

    memset(sit, 0, sizeof(vtss_sprout_sit_t));

    sit->mst_switch_addr = switch_state.mst_switch_addr;
    sit->mst_change_time = switch_state.mst_change_time;
    sit->topology_type   = switch_state.topology_type;

    ui_p = NULL;
    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        BOOL local_unit =
            vtss_sprout__switch_addr_cmp(&switch_state.switch_addr,
                                         &ui_p->switch_addr) == 0;
        BOOL same_switch_as_prv_ui =
            (prv_switch_addr_p != NULL &&
             (vtss_sprout__switch_addr_cmp(prv_switch_addr_p,
                                           &ui_p->switch_addr) == 0));

        if (same_switch_as_prv_ui) {
            
            i--;

            VTSS_SPROUT_ASSERT(sit->si[i].chip_cnt == 1,
                               ("sit->si[%d].chip_cnt=%d",
                                i, sit->si[i].chip_cnt));

            VTSS_SPROUT_ASSERT(ui_p->unit_idx == 1,
                               ("ui_p->unit_idx=%d", ui_p->unit_idx));
        }

        
        
        sit->si[i].vld = 1;

        sit->si[i].switch_addr = ui_p->switch_addr;

        
        if (ui_p->primary_unit) {
            
            sit->si[i].mst_capable     = ui_p->mst_capable;
            sit->si[i].mst_elect_prio  = ui_p->mst_elect_prio + 1;
            sit->si[i].mst_time_ignore = ui_p->mst_time_ignore;
            if (local_unit) {
                if (switch_state.mst) {
                    sit->si[i].mst_time =
                        sprout_init.callback.secs_since_boot() -
                        switch_state.mst_start_time;
                } else {
                    sit->si[i].mst_time = 0;
                }
            } else {
                sit->si[i].mst_time     = ui_p->mst_time;
            }

            
            memcpy(sit->si[i].switch_appl_info, ui_p->switch_appl_info,
                   VTSS_SPROUT_SWITCH_APPL_INFO_LEN);

            
            sit->si[i].ip_addr         = ui_p->ip_addr;
        }

        sit->si[i].chip_cnt++;

#if defined(VTSS_SPROUT_V1)
        {
            uint g = 0;
            for (g = 0; g < VTSS_GLAGS; g++) {
                sit->si[i].glag_mbr_cnt[g + 1] = ui_p->glag_mbr_cnt[g];
            }
        }
#endif

        
        
        
        if (ui_p->primary_unit) {
            chip_idx = 0;
        } else {
            chip_idx = 1;
        }

        sit->si[i].chip[chip_idx].upsid[0] = ui_p->upsid[0];
        sit->si[i].chip[chip_idx].upsid[1] = ui_p->upsid[1];

        sit->si[i].chip[chip_idx].ups_port_mask[0] = ui_p->ups_port_mask[0];
        sit->si[i].chip[chip_idx].ups_port_mask[1] = ui_p->ups_port_mask[1];

        sp_path = get_shortest_path(0, &ui_p->switch_addr, ui_p->unit_idx);
        if (sp_path == 2) {
            
            sit->si[i].chip[chip_idx].shortest_path = 0;
            sit->si[i].chip[chip_idx].dist[0]       = 0;
            sit->si[i].chip[chip_idx].dist[1]       = 0;
        } else {
            sit->si[i].chip[chip_idx].shortest_path =
                switch_state.unit[0].stack_port[sp_path].port_no;

            if ((ri_p = vtss_sprout__ri_find(&rit,
                                             &ui_p->switch_addr,
                                             chip_idx))) {

                sit->si[i].chip[chip_idx].dist[0] = ri_p->stack_port_dist[0];
                sit->si[i].chip[chip_idx].dist[1] = ri_p->stack_port_dist[1];
            } else {
                sit->si[i].chip[chip_idx].dist[0] = -1;
                sit->si[i].chip[chip_idx].dist[1] = -1;
            }
        }

        sit->si[i].chip[chip_idx].have_mirror     = ui_p->have_mirror;

        prv_switch_addr_p = &ui_p->switch_addr;

        i++;
    }
    switch_cnt = i;

    for (; i < VTSS_SPROUT_SIT_SIZE; i++) {
        sit->si[i].vld = 0;
    }

    if (!ignore_semaphores) {
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    }
    T_D("exit, switch_cnt=%d", switch_cnt);
} 


vtss_rc vtss_sprout_stat_get(
    vtss_sprout_switch_stat_t *const stat)
{
    
    vtss_sprout_switch_addr_t addr[VTSS_SPROUT_UIT_SIZE];
    uint                    addr_cnt = 0;
    vtss_sprout__ui_t         *ui_p = NULL;
    vtss_sprout__stack_port_idx_t sp_idx;

    T_D("enter");

    if (!ignore_semaphores) {
        VTSS_SPROUT_CRIT_ENTER_TBL_RD();
    }

    VTSS_SPROUT_ASSERT(stat != NULL, (" "));

    
    while ((ui_p = vtss_sprout__ui_get_nxt(&uit, ui_p))) {
        BOOL found = 0;
        uint i     = 0;
        while (i < addr_cnt) {
            if (vtss_sprout__switch_addr_cmp(&ui_p->switch_addr, &addr[i]) == 0) {
                found = 1;
                break;
            }
            i++;
        }
        if (!found) {
            
            addr[addr_cnt++] = ui_p->switch_addr;
        }
    }

    stat->switch_cnt           = addr_cnt;
    stat->topology_type        = switch_state.topology_type;
    stat->topology_change_time = switch_state.topology_change_time;

    for (sp_idx = 0; sp_idx < 2; sp_idx++) {
        
        stat->stack_port[sp_idx].proto_up                          = switch_state.unit[0].stack_port[sp_idx].proto_up;
        stat->stack_port[sp_idx].sprout_update_rx_cnt              = switch_state.unit[0].stack_port[sp_idx].sprout_update_rx_cnt;
        stat->stack_port[sp_idx].sprout_update_periodic_tx_cnt     = switch_state.unit[0].stack_port[sp_idx].sprout_update_periodic_tx_cnt;
        stat->stack_port[sp_idx].sprout_update_triggered_tx_cnt    = switch_state.unit[0].stack_port[sp_idx].sprout_update_triggered_tx_cnt;
        stat->stack_port[sp_idx].sprout_update_tx_policer_drop_cnt = switch_state.unit[0].stack_port[sp_idx].sprout_update_tx_policer_drop_cnt;
        stat->stack_port[sp_idx].sprout_update_rx_err_cnt          = switch_state.unit[0].stack_port[sp_idx].sprout_update_rx_err_cnt;
        stat->stack_port[sp_idx].sprout_update_tx_err_cnt          = switch_state.unit[0].stack_port[sp_idx].sprout_update_tx_err_cnt;

#if defined(VTSS_SPROUT_V2)
        stat->stack_port[sp_idx].sprout_alert_rx_cnt              = switch_state.unit[0].stack_port[sp_idx].sprout_alert_rx_cnt;
        stat->stack_port[sp_idx].sprout_alert_tx_cnt              = switch_state.unit[0].stack_port[sp_idx].sprout_alert_tx_cnt;
        stat->stack_port[sp_idx].sprout_alert_tx_policer_drop_cnt = switch_state.unit[0].stack_port[sp_idx].sprout_alert_tx_policer_drop_cnt;
        stat->stack_port[sp_idx].sprout_alert_rx_err_cnt          = switch_state.unit[0].stack_port[sp_idx].sprout_alert_rx_err_cnt;
        stat->stack_port[sp_idx].sprout_alert_tx_err_cnt          = switch_state.unit[0].stack_port[sp_idx].sprout_alert_tx_err_cnt;
#endif
    }

    if (!ignore_semaphores) {
        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
    }

    return VTSS_OK;
} 


vtss_rc vtss_sprout_parm_set(
    const BOOL             init_phase,
    const vtss_sprout_parm_t parm,
    const int              val)
{
    vtss_rc rc = VTSS_OK;
    vtss_sprout__stack_port_state_t *sps_p;
    vtss_sprout__stack_port_idx_t   sp_idx;
    vtss_sprout__unit_idx_t         unit_idx;

    T_D("enter");

    if (!init_phase) {
        VTSS_SPROUT_CRIT_ENTER_STATE_DATA();
        VTSS_SPROUT_CRIT_ENTER_TBL_RD();
    }

    switch (parm) {
    case VTSS_SPROUT_PARM_MST_ELECT_PRIO:
        if (!(VTSS_SPROUT_MST_ELECT_PRIO_START <= val &&
              val < VTSS_SPROUT_MST_ELECT_PRIO_END)) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            switch_state.mst_elect_prio = val - 1;
        }
        break;

    case VTSS_SPROUT_PARM_MST_TIME_IGNORE:
        if (val != 0 && val != 1) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            switch_state.mst_time_ignore = val;
        }
        break;

    case VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_SLV:
        if (val <= 0) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
                for (sp_idx = 0; sp_idx < 2; sp_idx++) {
                    sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];
                    sps_p->update_interval_slv = val;
                }
            }
        }
        break;

    case VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_MST:
        if (val <= 0) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
                for (sp_idx = 0; sp_idx < 2; sp_idx++) {
                    sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];
                    sps_p->update_interval_mst = val;
                }
            }
        }
        break;

    case VTSS_SPROUT_PARM_SPROUT_UPDATE_AGE_TIME:
        if (val <= 0) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
                for (sp_idx = 0; sp_idx < 2; sp_idx++) {
                    sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];
                    sps_p->update_age_time = val;
                }
            }
        }
        break;
    case VTSS_SPROUT_PARM_SPROUT_UPDATE_LIMIT:
        if (val <= 0) {
            T_E("Invalid val: %d", val);
            rc = VTSS_INVALID_PARAMETER;
        } else {
            for (unit_idx = 0; unit_idx < vtss_board_chipcount(); unit_idx++) {
                for (sp_idx = 0; sp_idx < 2; sp_idx++) {
                    sps_p = &switch_state.unit[unit_idx].stack_port[sp_idx];
                    sps_p->update_limit = val;
                }
            }
        }
        break;
    default:
        T_E("Unknown parm: %d", parm);
        rc = VTSS_INVALID_PARAMETER;
    }

    if (!init_phase) {
        update_local_uis();

        
        rc = process_xit_changes(1, 1);
        if (rc < 0) {
            clr_xit_flags();
            VTSS_SPROUT_CRIT_EXIT_TBL_RD();
            VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
            return rc;
        }

        rc = state_change_callback();

        VTSS_SPROUT_CRIT_EXIT_TBL_RD();
        VTSS_SPROUT_CRIT_EXIT_STATE_DATA();
    }
    return rc;
} 













#if defined(VTSS_SPROUT_FW_VER_CHK)
void vtss_sprout_fw_ver_mode_set(
    vtss_sprout_fw_ver_mode_t mode)
{
    switch_state.fw_ver_mode = mode;
} 
#endif

static char *dbg_info(void)
{
    static char s[5000];
    const  int  size = 5000; 

    s[0] = 0;
    vtss_sprout__str_append(s, size, "Debug info:\n");
    vtss_sprout__str_append(s, size, "Switch state:\n%s", vtss_sprout__switch_state_to_str(&switch_state));



#if 0
    vtss_sprout__str_append(s, size, vtss_sprout__rit_to_str(&rit));
    vtss_sprout__str_append(s, size, "UIT:\n%s", vtss_sprout__uit_to_str(&uit));
#endif

    VTSS_SPROUT_ASSERT(strlen(s) < size,
                       ("strlen(s)=%d", strlen(s)));

    return s;
} 


static void dbg_cmd_syntax_error(
    vtss_sprout_dbg_printf_t dbg_printf,
    const char *fmt, ...)
{
    va_list ap;
    char s[200] = "Command syntax error: ";
    int len;

    len = strlen(s);

    va_start(ap, fmt);

    vsprintf(s + len, fmt, ap);
    dbg_printf("%s\n", s);

    va_end(ap);
} 


#define TOPO_DBG_CMD_RIT_PRINT               3
#define TOPO_DBG_CMD_UIT_PRINT               4
#define TOPO_DBG_CMD_UI_UPDATE               5
#define TOPO_DBG_CMD_UIT_CLR_FLAGS           6
#define TOPO_DBG_CMD_UIT_DEL_UNFOUND         7
#define TOPO_DBG_CMD_SWITCH_STATE_PRINT      8
#define TOPO_DBG_CMD_RIT_CLR                 9
#define TOPO_DBG_CMD_SET_NBR                 10
#define TOPO_DBG_CMD_UIT_CLR                 11
#define TOPO_DBG_CMD_ADM_UP                  12
#define TOPO_DBG_CMD_LINK_UP                 13
#define TOPO_DBG_CMD_TX_SPROUT_UPDATE        14
#define TOPO_DBG_CMD_SUMMARY                 15
#define TOPO_DBG_CMD_STACK_PORT_MIRROR       16
#define TOPO_DBG_CMD_LINK_DOWN_1WAY          17
#define TOPO_DBG_CMD_SPROUT_UPDATE_PERIODIC  18
#define TOPO_DBG_CMD_SPROUT_UPDATE_TRIGGERED 19
#define TOPO_DBG_CMD_SWITCH_STAT             20
#define TOPO_DBG_CMD_TRACE_LEVEL             23
#define TOPO_DBG_CMD_SET_MST_TIME            24
#define TOPO_DBG_CMD_SET_MST_TIME_IGNORE     25
#define TOPO_DBG_CMD_SET_MST_ELECT_PRIO      26

#define TOPO_DBG_CMD_TMP_TEST                80

#if defined(VTSS_SPROUT_V1)
#define TOPO_DBG_CMD_SET_GLAG_MBR_CNT        92
#endif
#define TOPO_DBG_CMD_SET_UPSID               93
#define TOPO_DBG_CMD_SET_SWITCH_ADDR         94
#define TOPO_DBG_CMD_MERGE_UITS              96
#define TOPO_DBG_CMD_TEST_FUNC99             99
#define TOPO_DBG_CMD_END                     -1

typedef struct {
    int  cmd_num;
    char cmd_txt[80];
    char arg_syntax[80];
    uint arg_cnt;
} vtss_sprout_dbg_cmd_t;


void vtss_sprout_dbg(
    vtss_sprout_dbg_printf_t dbg_printf,
    const uint         parms_cnt,
    const ulong *const parms)
{
    vtss_sprout__ui_t             ui;
    vtss_sprout__ri_t             *ri_p = NULL;
    uint                          i;
    int                           sel;
    int                           cmd_num;
    vtss_sprout__stack_port_idx_t sp_idx;
    vtss_port_no_t                port_no;
    static vtss_sprout_dbg_cmd_t  cmds[] = {
        {
            TOPO_DBG_CMD_RIT_PRINT,
            "Print RIT",
            "",
            0
        },
        {
            TOPO_DBG_CMD_UIT_PRINT,
            "Print UIT",
            "<uit select> - 0: stack port a   1: stack port b   2: UIT",
            1
        },
        {
            TOPO_DBG_CMD_UI_UPDATE,
            "Update/Insert UI on stack port UIT",
            "<stack_port_idx> <sa byte> <unit idx> <upsid> <glag_mbr_cnt> <have_mirror>",
            6
        },
        {
            TOPO_DBG_CMD_UIT_CLR_FLAGS,
            "Clear UIT flags",
            "<stack_port>",
            1,
        },
        {
            TOPO_DBG_CMD_UIT_DEL_UNFOUND,
            "Delete UIs not found in SPROUT update",
            "<stack_port>",
            1,
        },
        {
            TOPO_DBG_CMD_SWITCH_STATE_PRINT,
            "Print switch state information",
            "",
            0,
        },
        {
            TOPO_DBG_CMD_RIT_CLR,
            "Clear RIT",
            "",
            0,
        },
        {
            TOPO_DBG_CMD_SET_NBR,
            "Set neighbour address",
            "<stack port> <sa byte>",
            2,
        },
        {
            TOPO_DBG_CMD_UIT_CLR,
            "Clear UIT",
            "<uit select>",
            1,
        },
        {
            TOPO_DBG_CMD_ADM_UP,
            "Adm up/down",
            "<stack port no (SA/SB)> <up>",
            2,
        },
        {
            TOPO_DBG_CMD_LINK_UP,
            "Link up/down",
            "<stack port no (SA/SB)> <up>",
            2,
        },
        {
            TOPO_DBG_CMD_TX_SPROUT_UPDATE,
            "Send SPROUT Update",
            "<stack port idx (0/1)>",
            1,
        },
        {
            TOPO_DBG_CMD_SUMMARY,
            "Print state summary",
            "",
            0,
        },
        {
            TOPO_DBG_CMD_STACK_PORT_MIRROR,
            "Mirror stack port to front port 1",
            "<stack port no (SA/SB)>",
            1,
        },
        {
            TOPO_DBG_CMD_LINK_DOWN_1WAY,
            "Set LANE_ENA=0, causing peer to see link as down",
            "<stack port no (SA/SB)>",
            1,
        },
        {
            TOPO_DBG_CMD_SPROUT_UPDATE_PERIODIC,
            "Enable/disable periodic SPROUT updates and SPROUT aging",
            "<0=disable, 1=enable>",
            1,
        },
        {
            TOPO_DBG_CMD_SPROUT_UPDATE_TRIGGERED,
            "Enable/disable triggered SPROUT updates",
            "<0=disable, 1=enable>",
            1,
        },
        {
            TOPO_DBG_CMD_SWITCH_STAT,
            "Print SPROUT switch statistics",
            "",
            0,
        },
        {
            TOPO_DBG_CMD_TRACE_LEVEL,
            "Set trace level minimum. <grp>=-1 => All groups.",
            "<grp> <lvl: 10=None, 9=Error, 8=Warning, 6=Info, 4=Debug, 2=Noise 1=Racket>",
            2,
        },
        {
            TOPO_DBG_CMD_SET_MST_TIME,
            "Set master time (and mst=1)",
            "<master_time>",
            1,
        },
        {
            TOPO_DBG_CMD_SET_MST_TIME_IGNORE,
            "Set master time ignore",
            "<1/0>",
            1,
        },
        {
            TOPO_DBG_CMD_SET_MST_ELECT_PRIO,
            "Set master elect prio",
            "<prio (0-3)>",
            1,
        },
        

#if defined(VTSS_SPROUT_V1)
        {
            TOPO_DBG_CMD_SET_GLAG_MBR_CNT,
            "Set GLAG member count",
            "<glag id (1/2)> <count>",
            2,
        },
#endif
        {
            TOPO_DBG_CMD_TMP_TEST,
            "Tempoary test",
            "",
            2,
        },
        {
            TOPO_DBG_CMD_SET_UPSID,
            "Set UPSID (of primary unit) and process",
            "<upsid (0-30)>",
            1,
        },
        {
            TOPO_DBG_CMD_SET_SWITCH_ADDR,
            "Set address of switch and tx updates",
            "<32 bit switch addr>",
            1,
        },
        {
            TOPO_DBG_CMD_MERGE_UITS,
            "Merge stack port UITs into common UIT",
            "",
            0,
        },
        {
            TOPO_DBG_CMD_TEST_FUNC99,
            "Test function",
            "",
            2,
        },
        {
            TOPO_DBG_CMD_END,
            "",
            "",
            0,
        }
    };

    if (parms_cnt == 0) {
        dbg_printf("Usage: D <cmd idx>\n");
        dbg_printf("\n");
        dbg_printf("Commands:\n");
        i = 0;
        while (cmds[i].cmd_num != TOPO_DBG_CMD_END) {
            dbg_printf("  %2d: %s\n", cmds[i].cmd_num, cmds[i].cmd_txt);
            if (cmds[i].arg_syntax[0]) {
                dbg_printf("      Arguments: %s.\n", cmds[i].arg_syntax);
            }
            i++;
        }
        char str[20];
        sprintf(str, "\nSA=%u, SB=%u\n",
                switch_state.unit[0].stack_port[0].port_no,
                switch_state.unit[0].stack_port[1].port_no);
        dbg_printf(str);
        return;
    }
    cmd_num = parms[0];

    
    i = 0;
    while (cmds[i].cmd_num != TOPO_DBG_CMD_END) {
        if (cmds[i].cmd_num == cmd_num) {
            break;
        }

        i++;
    }
    if (cmds[i].cmd_num == TOPO_DBG_CMD_END) {
        dbg_cmd_syntax_error(dbg_printf, "Unknown command number: %d", cmd_num);
        return;
    }
    if (cmds[i].arg_cnt + 1 != parms_cnt) {
        dbg_cmd_syntax_error(dbg_printf, "Incorrect number of arguments (%d).\n"
                             "Arguments: %s",
                             parms_cnt - 1,
                             cmds[i].arg_syntax);
        return;
    }

    switch (parms[0]) {
    case TOPO_DBG_CMD_RIT_PRINT:
        vtss_sprout__rit_print(dbg_printf, &rit);
        break;

    case TOPO_DBG_CMD_UIT_PRINT:
        sel = parms[1];

        switch (sel) {
        case 0:
            dbg_printf("UIT for stack port 0\n");
            vtss_sprout__uit_print(dbg_printf, &switch_state.unit[0].stack_port[0].uit);
            break;
        case 1:
            dbg_printf("UIT for stack port 1\n");
            vtss_sprout__uit_print(dbg_printf, &switch_state.unit[0].stack_port[1].uit);
            break;
        case 2:
            dbg_printf("UIT\n");
            vtss_sprout__uit_print(dbg_printf, &uit);
            break;
        default:
            dbg_cmd_syntax_error(dbg_printf, "Unexpected UIT selector: %d", sel);
        }
        break;

    case TOPO_DBG_CMD_UI_UPDATE:
        sp_idx = parms[1];
        if (sp_idx > 1) {
            dbg_cmd_syntax_error(dbg_printf, "stack_port_idx==%d >1", sp_idx);
            return;
        }

        vtss_sprout__ui_init(&ui);
        ui.vld = 1;
        ui.switch_addr.addr[0]              = parms[2];
        ui.unit_idx                         = parms[3];
        ui.upsid[0]                         = parms[4];
#if defined(VTSS_SPROUT_V1)
        ui.glag_mbr_cnt[0]                  = parms[5];
        ui.glag_mbr_cnt[0]                  = parms[5];
#endif
        ui.have_mirror                      = parms[6];
        vtss_sprout__uit_update(&switch_state.unit[0].stack_port[sp_idx].uit, &ui, 0);

        vtss_sprout__uit_print(dbg_printf, &switch_state.unit[0].stack_port[sp_idx].uit);
        break;

    case TOPO_DBG_CMD_UIT_CLR_FLAGS:
        sel = parms[1];

        switch (sel) {
        case 0:
            vtss_sprout__uit_clr_flags(&switch_state.unit[0].stack_port[0].uit);
            break;
        case 1:
            vtss_sprout__uit_clr_flags(&switch_state.unit[0].stack_port[1].uit);
            break;
        case 2:
            vtss_sprout__uit_clr_flags(&uit);
            break;
        default:
            dbg_cmd_syntax_error(dbg_printf, "Unexpected UIT selector: %d", sel);
        }
        break;

    case TOPO_DBG_CMD_UIT_DEL_UNFOUND:
        sel = parms[1];

        if (sel > 1) {
            dbg_cmd_syntax_error(dbg_printf, "Unexpected UIT selector: %d", sel);
            return;
        }

        vtss_sprout__uit_del_unfound(&switch_state.unit[0].stack_port[sel].uit);
        break;

    case TOPO_DBG_CMD_SWITCH_STATE_PRINT:
        vtss_sprout__switch_state_print(dbg_printf, &switch_state);
        break;

    case TOPO_DBG_CMD_RIT_CLR:
        while ((ri_p = vtss_sprout__ri_get_nxt(&rit, ri_p))) {
            vtss_sprout__ri_init(ri_p);
        }
        break;

    case TOPO_DBG_CMD_SET_NBR:
        sp_idx = parms[1];
        if (sp_idx > 1) {
            dbg_cmd_syntax_error(dbg_printf, "stack_port_idx==%d >1", sp_idx);
            return;
        }
        vtss_sprout_switch_addr_init(&switch_state.unit[0].stack_port[sp_idx].nbr_switch_addr);
        switch_state.unit[0].stack_port[sp_idx].nbr_switch_addr.addr[0] = parms[2];
        break;

    case TOPO_DBG_CMD_UIT_CLR:
        sel = parms[1];
        if (sel > 2) {
            dbg_cmd_syntax_error(dbg_printf, "Unexpected UIT selector: %d", sel);
            return;
        }
        if (sel < 2) {
            vtss_sprout__uit_init(&switch_state.unit[0].stack_port[sel].uit);
        } else {
            vtss_sprout__uit_init(&uit);
        }
        break;

    case TOPO_DBG_CMD_MERGE_UITS:
        merge_uits();
        break;

    case TOPO_DBG_CMD_ADM_UP:
        vtss_sprout_stack_port_adm_state_set(parms[1], parms[2]);
        break;

    case TOPO_DBG_CMD_LINK_UP:
        
    {
        vtss_port_conf_t conf;
        port_no = parms[1];

        if (vtss_port_conf_get(NULL, port_no, &conf) == VTSS_RC_OK) {
            conf.power_down = ((parms[2] == 0) ? 1 : 0);
            vtss_port_conf_set(NULL, port_no, &conf);
        }
    }
    break;

    case TOPO_DBG_CMD_LINK_DOWN_1WAY:
        







        vtss_reg_write(NULL,
                       0,
                       (3 << 12) | (4 << 8) | ((parms[1] == 25) ? 0x04 : 0x0c),
                       0);
        vtss_reg_write(NULL,
                       0,
                       (3 << 12) | (4 << 8) | ((parms[1] == 25) ? 0x08 : 0x10),
                       0);
        break;
    case TOPO_DBG_CMD_SPROUT_UPDATE_PERIODIC:
        internal_cfg.sprout_periodic_updates = parms[1];
        internal_cfg.sprout_aging            = parms[1];
        break;

    case TOPO_DBG_CMD_SPROUT_UPDATE_TRIGGERED:
        internal_cfg.sprout_triggered_updates = parms[1];
        break;

    case TOPO_DBG_CMD_SWITCH_STAT: {
        int i;
        vtss_sprout_switch_stat_t stat;
        CRIT_IGNORE(vtss_sprout_stat_get(&stat));
        dbg_printf("switch_cnt=%d\n", stat.switch_cnt);
        dbg_printf("topology_type=%d\n", stat.topology_type);
        for (i = 0; i < 2; i++) {
            dbg_printf("Stack port %d: proto_up=%d\n",                i, stat.stack_port[i].proto_up);
            dbg_printf("Stack port %d: sprout_update_rx_cnt=%d\n",           i, stat.stack_port[i].sprout_update_rx_cnt);
            dbg_printf("Stack port %d: sprout_update_tx_periodic_cnt=%d\n",  i, stat.stack_port[i].sprout_update_periodic_tx_cnt);
            dbg_printf("Stack port %d: sprout_update_tx_triggered_cnt=%d\n", i, stat.stack_port[i].sprout_update_triggered_tx_cnt);
            dbg_printf("Stack port %d: sprout_update_rx_err_cnt=%d\n",       i, stat.stack_port[i].sprout_update_rx_err_cnt);
            dbg_printf("Stack port %d: sprout_update_tx_err_cnt=%d\n",       i, stat.stack_port[i].sprout_update_tx_err_cnt);
        }
    }
    break;

    case TOPO_DBG_CMD_TRACE_LEVEL:
#if (VTSS_TRACE_ENABLED)
        if ((int)parms[1] >= TRACE_GRP_CNT) {
            dbg_cmd_syntax_error(dbg_printf, "grp==%d >=%d", parms[1], TRACE_GRP_CNT);
            return;
        }

        if (parms[1] == -1) {
            
            int i;
            for (i = 0; i < TRACE_GRP_CNT; i++) {
                trace_grps[i].lvl = parms[2];
            }
        } else {
            trace_grps[parms[1]].lvl = parms[2];
        }
#endif
        break;

    case TOPO_DBG_CMD_SET_MST_TIME:
        switch_state.mst = 1;
        switch_state.mst_start_time = sprout_init.callback.secs_since_boot() - parms[1];
        switch_state.mst_switch_addr = switch_state.switch_addr;
        break;

    case TOPO_DBG_CMD_SET_MST_TIME_IGNORE:
        vtss_sprout_parm_set(0, VTSS_SPROUT_PARM_MST_TIME_IGNORE, parms[1]);
        break;

    case TOPO_DBG_CMD_SET_MST_ELECT_PRIO:
        vtss_sprout_parm_set(0, VTSS_SPROUT_PARM_MST_ELECT_PRIO, parms[1]);
        break;

    case TOPO_DBG_CMD_TX_SPROUT_UPDATE:
        tx_sprout_update(parms[1], 0);
        break;

    case TOPO_DBG_CMD_SUMMARY:
        dbg_printf("topology_type=%s\n", vtss_sprout_topology_type_to_str(switch_state.topology_type));
        dbg_printf("topology_n=%d\n", switch_state.topology_n);
#if defined(VTSS_SPROUT_V1)
        if (switch_state.ui_top_p) {
            dbg_printf("top_switch=%s\n",
                       vtss_sprout_switch_addr_to_str(&switch_state.ui_top_p->switch_addr));
        }
#endif
        dbg_printf("ttl[0]=%d\n", switch_state.unit[0].stack_port[0].ttl);
        dbg_printf("ttl[1]=%d\n", switch_state.unit[0].stack_port[1].ttl);
        vtss_sprout__rit_print(dbg_printf, &rit);
        break;

    case TOPO_DBG_CMD_STACK_PORT_MIRROR:
        
        vtss_mirror_monitor_port_set(NULL, 1);

        
        





        vtss_reg_write(NULL, 0, (1 << 12) | (0 << 8) | 0x25, (1 << 7));

        {
            BOOL member[VTSS_PORT_ARRAY_SIZE];

            memset(member, 0, sizeof(member));
            member[parms[1]] = 1;

            vtss_mirror_ingress_ports_set(NULL, member);
            vtss_mirror_egress_ports_set(NULL, member);
        }
        break;

#if defined(VTSS_SPROUT_V1)
    case TOPO_DBG_CMD_SET_GLAG_MBR_CNT:
        vtss_sprout_glag_mbr_cnt_set(parms[1], parms[2]);
        break;
#endif

    case TOPO_DBG_CMD_TMP_TEST:
        dbg_printf(dbg_info());

#if 0
        {
            static int i = 0;
            static char s[1000];

            for (i = 0; i < parms[1]; i++) {
                s[i] = 'a';
            }
            s[i] = 0;
            T_I(s);
        }
#endif
        break;

    case TOPO_DBG_CMD_SET_UPSID:
        switch_state.unit[0].upsid = parms[1] & 0x1E; 
#if VTSS_SPROUT_MAX_LOCAL_CHIP_CNT > 1
        switch_state.unit[1].upsid = switch_state.unit[0].upsid + 1;
#endif
        T_DG(TRACE_GRP_UPSID, "Calling cfg_save()");
        cfg_save();
        stack_setup();
        update_local_uis();
        clr_xit_flags();
        
        
        
        
        break;

    case TOPO_DBG_CMD_SET_SWITCH_ADDR:
        memset(&switch_state.switch_addr, 0, sizeof(switch_state.switch_addr));
        switch_state.switch_addr.addr[2] = (parms[1] & 0xff000000) > 24;
        switch_state.switch_addr.addr[3] = (parms[1] & 0x00ff0000) > 16;
        switch_state.switch_addr.addr[4] = (parms[1] & 0x0000ff00) >  8;
        switch_state.switch_addr.addr[5] = (parms[1] & 0x000000ff);
        switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].proto_up = 0;
        switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].proto_up = 0;
        vtss_sprout__uit_init(&switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].uit);
        vtss_sprout__uit_init(&switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].uit);
        vtss_sprout__uit_init(&uit);

        vtss_sprout__uit_print(dbg_printf, &uit);
        T_N("--1--");
        update_local_uis();
        T_N("--2--");
        vtss_sprout__uit_print(dbg_printf, &uit);
        T_N("--3--");
        uit.changed = 0;

        vtss_sprout__rit_init(&rit);

#if 1
        if (switch_state.unit[0].stack_port[VTSS_SPROUT__SP_A].link_up) {
            tx_sprout_update(VTSS_SPROUT__SP_A, 0);
        }
        if (switch_state.unit[0].stack_port[VTSS_SPROUT__SP_B].link_up) {
            tx_sprout_update(VTSS_SPROUT__SP_B, 0);
        }
#endif
        break;

    case TOPO_DBG_CMD_TEST_FUNC99: {


        T_I("%s", vtss_sprout__uit_to_str(&uit));
    }
    break;

    default:
        dbg_cmd_syntax_error(dbg_printf, "Unknown command index: %d", (int)parms[0]);
        break;
    }
} 








