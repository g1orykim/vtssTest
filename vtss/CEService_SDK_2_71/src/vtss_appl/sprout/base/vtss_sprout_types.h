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

 $Id: vtss_sprout_types.h,v 1.58 2010/11/25 11:42:04 toe Exp $
 $Revision: 1.58 $


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/









#ifndef _VTSS_SPROUT_TYPES_H_
#define _VTSS_SPROUT_TYPES_H_

#include <time.h>

#define VTSS_SPROUT_UIT_SIZE           VTSS_SPROUT_SIT_SIZE


typedef uint vtss_sprout__glagid_t;


typedef uint vtss_sprout__mst_elect_prio_t;


typedef uint vtss_sprout__cpu_rx_qu_t;



typedef uint vtss_sprout__unit_idx_t;


typedef uint vtss_sprout__upsid_idx_t;


char *vtss_sprout_topology_type_to_str(vtss_sprout_topology_type_t topology_type);
void vtss_sprout_topology_type_print(vtss_sprout_topology_type_t topology_type);






#define VTSS_SPROUT__SP_UNDEF (-1)
#define VTSS_SPROUT__SP_A     0
#define VTSS_SPROUT__SP_B     1
typedef int vtss_sprout__stack_port_idx_t;






enum {
    



    SPROUT_RC_UPSID_DEPLETION = MODULE_ERROR_START(VTSS_MODULE_ID_SPROUT),
};




typedef struct vtss_sprout__ri_t {
    
    BOOL                    vld;
    BOOL                    found;   
    BOOL                    is_top_switch;

    
    vtss_sprout_switch_addr_t switch_addr;
    vtss_sprout__unit_idx_t   unit_idx;

    
    
    
    vtss_vstax_upsid_t        upsid[2];

    vtss_sprout_dist_t        stack_port_dist[2];

    BOOL                      tightly_coupled;
} vtss_sprout__ri_t;


typedef struct _vtss_sprout__rit_t {
    BOOL      changed; 
    vtss_sprout__ri_t ri[VTSS_SPROUT_RIT_SIZE];
} vtss_sprout__rit_t;


void vtss_sprout__ri_init(    vtss_sprout__ri_t  *ri_p);
void vtss_sprout__rit_init(   vtss_sprout__rit_t *rit_p);
char *vtss_sprout__ri_to_str(const vtss_sprout__ri_t *ri_p);
void vtss_sprout__ri_print(   vtss_sprout__ri_t  *ri_p);
char *vtss_sprout__rit_to_str(vtss_sprout__rit_t *rit_p);
void vtss_sprout__rit_print(  vtss_sprout_dbg_printf_t dbg_printf,
                              vtss_sprout__rit_t *rit_p);








typedef struct _vtss_sprout__ui_t {
    
    BOOL                          vld;
    BOOL                          found;   

    
    vtss_sprout_switch_addr_t     switch_addr;
    
    
    vtss_sprout__unit_idx_t       unit_idx;

    
    
    vtss_vstax_upsid_t            upsid[2];

    BOOL                          primary_unit;
    
    uint                          glag_mbr_cnt[VTSS_GLAGS];
    BOOL                          have_mirror;

    
    BOOL                          mst_capable; 
    vtss_sprout__mst_elect_prio_t mst_elect_prio;
    BOOL                          mst_time_ignore;
    ulong                         mst_time;

    
    ulong ip_addr;

    
    BOOL                          tightly_coupled;

    
    vtss_sprout_switch_appl_info_t switch_appl_info;

    
    uchar unit_base_info_rsv;                
    uchar unit_glag_mbr_cnt_rsv[VTSS_GLAGS]; 
    uchar ups_base_info_rsv[2];              
    uchar switch_mst_elect_rsv;              
    uchar switch_base_info_rsv;              

    
    
    
    vtss_sprout__stack_port_idx_t sp_idx;

    
    u64 ups_port_mask[2];
} vtss_sprout__ui_t;


typedef struct _vtss_sprout__uit_t {
    BOOL      changed;
    BOOL      mst_time_changed;
    u32       change_mask; 

    vtss_sprout__ui_t ui[VTSS_SPROUT_UIT_SIZE];
} vtss_sprout__uit_t;


void vtss_sprout__ui_init(  vtss_sprout__ui_t  *ui_p);
void vtss_sprout__uit_init( vtss_sprout__uit_t *uit_p);
char *vtss_sprout__ui_to_str(const vtss_sprout__ui_t  *ui_p);
void vtss_sprout__ui_print( vtss_sprout__ui_t  *ui_p);
char *vtss_sprout__uit_to_str(vtss_sprout__uit_t *uit_p);
void vtss_sprout__uit_print(vtss_sprout_dbg_printf_t dbg_printf, vtss_sprout__uit_t *uit_p);








typedef struct _vtss_sprout__stack_port_state_t {
    
    BOOL                       adm_up;
    BOOL                       link_up;
    vtss_port_no_t             port_no;

    uint                       update_age_time;
    
    uint                       update_interval_slv;
    uint                       update_interval_mst;

    
    uint                       update_limit;
    uint                       alert_limit;


    
    BOOL                       proto_up;
    vtss_sprout_switch_addr_t    nbr_switch_addr;
    vtss_sprout__uit_t           uit;
    uint                       ttl;
    uint                       ttl_new;
    
    BOOL                       mirror_fwd;

    
    BOOL                       cfg_change;

    uint                       deci_secs_since_last_tx_update;
    uint                       deci_secs_since_last_rx_update;

    uint                       update_bucket;
    uint                       alert_bucket;
    uint                       deci_secs_since_last_update_bucket_filling;
    uint                       deci_secs_since_last_alert_bucket_filling;

    
    uint                       sprout_update_rx_cnt;              
    uint                       sprout_update_triggered_tx_cnt;    
    uint                       sprout_update_periodic_tx_cnt;     
    uint                       sprout_update_tx_policer_drop_cnt; 
    uint                       sprout_update_rx_err_cnt;          
    uint                       sprout_update_tx_err_cnt;          

    
    uint                       sprout_alert_rx_cnt;              
    uint                       sprout_alert_tx_cnt;              
    uint                       sprout_alert_tx_policer_drop_cnt; 
    uint                       sprout_alert_rx_err_cnt;          
    uint                       sprout_alert_tx_err_cnt;          
} vtss_sprout__stack_port_state_t;

void vtss_sprout__stack_port_state_init(vtss_sprout__stack_port_state_t *const sps_p);
char *vtss_sprout__stack_port_states_to_str(
    const vtss_sprout__stack_port_state_t *sps0_p,
    const vtss_sprout__stack_port_state_t *sps1_p, 
    const vtss_sprout__stack_port_state_t *sps2_p, 
    const vtss_sprout__stack_port_state_t *sps3_p, 
    const char *prefix);

typedef struct _vtss_sprout__unit_state_t {
    
    
    vtss_vstax_upsid_t              upsid;         
    u64                             ups_port_mask;
    uint                            glag_mbr_cnt[VTSS_GLAGS];
    BOOL                            have_mirror_port;

    
    
    
    
    
    
    
    
    vtss_sprout__stack_port_state_t stack_port[2];

    
} vtss_sprout__unit_state_t;

void vtss_sprout__unit_state_init(   vtss_sprout__unit_state_t *const us_p);

typedef struct _vtss_sprout__switch_state_t {
    
    vtss_sprout_switch_addr_t     switch_addr;

    BOOL                          mst_capable;
    vtss_sprout__mst_elect_prio_t mst_elect_prio;
    BOOL                          mst_time_ignore;
    vtss_sprout_switch_addr_t     mst_switch_addr; 
    vtss_vstax_upsid_t            mst_upsid;       
    BOOL                          mst;             
    ulong                         mst_start_time;  
    vtss_sprout__cpu_rx_qu_t      cpu_qu;

    
    
    vtss_sprout__unit_state_t     unit[VTSS_SPROUT_MAX_LOCAL_CHIP_CNT];

    
    vtss_sprout_topology_type_t   topology_type;
    vtss_sprout_topology_type_t   topology_type_new;
    uint                          topology_n; 
#if defined(VTSS_SPROUT_V1)
    vtss_sprout__ui_t             *ui_top_p;
    vtss_sprout__stack_port_idx_t top_link;
    vtss_sprout_dist_t            top_dist;
#endif

    
    BOOL                        virgin;

    time_t                      topology_change_time; 
    time_t                      mst_change_time;      

    
    ulong                       ip_addr;

    
    uchar                       switch_appl_info[8];

#if defined(VTSS_SPROUT_FW_VER_CHK)
    uchar                       my_fw_ver[VTSS_SPROUT_FW_VER_LEN];
    vtss_sprout_fw_ver_mode_t   fw_ver_mode;
#endif
} vtss_sprout__switch_state_t;

char *vtss_sprout__unit_state_to_str(vtss_sprout__switch_state_t *ss_p,
                                     vtss_sprout__unit_state_t *const specific_us_p, 
                                     const char *prefix);
void vtss_sprout__switch_state_init (vtss_sprout__switch_state_t *const ss_p);
char *vtss_sprout__switch_state_to_str(vtss_sprout__switch_state_t *ss_p);
void vtss_sprout__switch_state_print (vtss_sprout_dbg_printf_t dbg_printf,
                                      vtss_sprout__switch_state_t *ss_p);









void vtss_sprout_switch_addr_init(vtss_sprout_switch_addr_t *sa_p);
char *vtss_sprout_switch_addr_to_str(const vtss_sprout_switch_addr_t *sa_p);
void vtss_sprout_switch_addr_print(vtss_sprout_switch_addr_t *sa_p);





#endif 






