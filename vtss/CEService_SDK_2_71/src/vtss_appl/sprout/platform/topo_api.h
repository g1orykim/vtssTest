/*

 Vitesse Switch API software.

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

 $Id$
 $Revision$

*/

#ifndef _TOPO_API_H_
#define _TOPO_API_H_

#if defined(VTSS_SWITCH_STANDALONE) && VTSS_SWITCH_STANDALONE
#include "standalone_api.h" 
#else

#include <time.h>

#include "vtss_sprout_api.h"


typedef enum {
    TOPO_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_TOPO),  
    TOPO_ERROR_PARM,                                           
    TOPO_ASSERT_FAILURE,                                       
    TOPO_ALLOC_FAILED,                                         
    TOPO_TX_FAILED,                                            
    TOPO_ERROR_SWITCH_NOT_PRESENT,                             
    TOPO_ERROR_SID_NOT_ASSIGNED,                               
    TOPO_ERROR_REMOTE_PARM_ONLY_FROM_MST,                      
    TOPO_ERROR_REMOTE_PARM_NOT_SUPPORTED,                      
    TOPO_ERROR_NO_SWITCH_SELECTED,                             
    TOPO_ERROR_NOT_MASTER,                                     
    TOPO_ERROR_MASTER_SID,                                     
    TOPO_ERROR_SID_IN_USE,                                     
    TOPO_ERROR_SWITCH_HAS_SID,                                 
    TOPO_ERROR_ISID_DELETE_PENDING,                            
    TOPO_ERROR_CONFIG_ILLEGAL_STACK_PORT,                      
} topo_error_t;


const char *topo_error_txt(vtss_rc rc);



#define TOPO_MAX_UNITS_IN_STACK 32

#define TOPO_SIT_SIZE           (TOPO_MAX_UNITS_IN_STACK + TOPO_MAX_UNITS_IN_STACK/2)



#define TOPO_MAX_UNITS_IN_STACK 32



#define TOPO_MST_ELECT_PRIO_START    1
#define TOPO_MST_ELECT_PRIOS         4
#define TOPO_MST_ELECT_PRIO_END      (TOPO_MST_ELECT_PRIO_START+TOPO_MST_ELECT_PRIOS)
#define TOPO_MST_ELECT_PRIO_LEGAL(v) (TOPO_MST_ELECT_PRIO_START <= v && v < TOPO_MST_ELECT_PRIO_END)
typedef uint topo_mst_elect_prio_t;



#define TOPO_FW_VER_MODE_LEGAL(v) (VTSS_SPROUT_FW_VER_MODE_NULL == v || v == VTSS_SPROUT_FW_VER_MODE_NORMAL)




#define TOPO_CHIP_IDX_START 1
#define TOPO_CHIP_IDXS      2
#define TOPO_CHIP_IDX_END   (TOPO_CHIP_IDX_START+TOPO_CHIP_IDXS)
typedef uint topo_chip_idx_t;


typedef enum {TopoBack2Back,  
              TopoClosedLoop,
              TopoOpenLoop
             } topo_topology_type_t;


typedef vtss_sprout_stack_port_stat_t topo_stack_port_stat_t;

typedef vtss_sprout_switch_stat_t     topo_switch_stat_t;


typedef struct {
    BOOL stacking;
    
    
    
    vtss_port_no_t port_0, port_1;
} stack_config_t;


#define TOPO_DIST_INFINITY -1
typedef int topo_dist_t;

typedef vtss_sprout_sit_entry_t topo_sit_entry_t;
typedef vtss_sprout_sit_t       topo_sit_t;



#define TOPO_STATE_CHANGE_MASK_TTL           (1 << 0)

#if defined(VTSS_SPROUT_V1)

#define TOPO_STATE_CHANGE_MASK_GLAG          (1 << 1)
#endif


typedef uchar topo_state_change_mask_t;
typedef void (*topo_state_change_callback_t)(topo_state_change_mask_t mask);











typedef void (*topo_upsid_change_callback_t)(vtss_isid_t isid);






vtss_rc topo_init(vtss_init_data_t *data);

#if defined(VTSS_SPROUT_V1)

vtss_rc topo_glag_mbr_cnt_set(
    const topo_chip_idx_t chip_idx,
    const vtss_glag_no_t  glag_no,
    const uint            glag_mbr_cnt);
#endif


vtss_rc topo_have_mirror_port_set(
    const topo_chip_idx_t chip_idx,
    const BOOL             have_mirror_port);


vtss_rc topo_stack_port_adm_state_set(
    const vtss_port_no_t port_no,
    const BOOL           adm_up);


vtss_rc topo_ipv4_addr_set(
    const vtss_ipv4_t ipv4_addr);




vtss_rc topo_sit_get(
    topo_sit_t *const sit_p);


vtss_rc topo_state_change_callback_register(
    const topo_state_change_callback_t callback,
    const vtss_module_id_t             module_id);


vtss_rc topo_upsid_change_callback_register(
    const topo_upsid_change_callback_t callback,
    const vtss_module_id_t             module_id);





typedef enum {
    TOPO_PARM_MST_ELECT_PRIO,
    TOPO_PARM_MST_TIME_IGNORE,
    TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV,
    TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST,
    TOPO_PARM_SPROUT_UPDATE_AGE_TIME,
    TOPO_PARM_SPROUT_UPDATE_LIMIT,
    TOPO_PARM_FAST_MAC_AGE_TIME,
    TOPO_PARM_FAST_MAC_AGE_COUNT,
    TOPO_PARM_UID_0_0, 
    TOPO_PARM_UID_0_1, 
    TOPO_PARM_UID_1_0, 
    TOPO_PARM_UID_1_1, 
    TOPO_PARM_FW_VER_MODE, 
    TOPO_PARM_CMEF_MODE,   
    TOPO_PARM_ISID_TBL 
} topo_parm_t;

#define TOPO_PARM_MST_ELECT_PRIO_MIN 1
#define TOPO_PARM_MST_ELECT_PRIO_MAX 4

#define TOPO_PARM_SPROUT_UPDATE_INTERVAL_MIN 1
#define TOPO_PARM_SPROUT_UPDATE_AGE_TIME_MIN 1
#define TOPO_PARM_SPROUT_UPDATE_LIMIT_MIN    1
#define TOPO_PARM_FAST_MAC_AGE_TIME_MIN      1
#define TOPO_PARM_FAST_MAC_AGE_COUNT_MIN     1


int topo_parm_get(const topo_parm_t parm);





topo_error_t topo_parm_set(
    vtss_isid_t       isid,
    const topo_parm_t parm,
    const int         val);









vtss_port_no_t topo_isid2port(const vtss_isid_t isid);










vtss_port_no_t topo_mac2port(const mac_addr_t mac_addr);









topo_error_t topo_isid_delete(const vtss_isid_t isid);








topo_error_t topo_isid_assign(
    const vtss_isid_t isid,
    const mac_addr_t  mac_addr);




topo_error_t topo_usid_swap(
    const vtss_usid_t usida,
    const vtss_usid_t usidb);





vtss_isid_t topo_usid2isid(const vtss_usid_t usid);


vtss_usid_t topo_isid2usid(const vtss_isid_t isid);



vtss_rc topo_isid2mac(
    const vtss_isid_t isid,
    mac_addr_t mac_addr);



vtss_isid_t topo_mac2isid(const mac_addr_t mac_addr);
















vtss_vstax_upsid_t topo_isid_port2upsid(
    const vtss_isid_t    isid,
    const vtss_port_no_t port_no);








vtss_isid_t topo_upsid2isid(const vtss_vstax_upsid_t upsid);

#if defined(VTSS_FEATURE_VSTAX_V2)
typedef struct {
    vtss_isid_t    isid;
    vtss_usid_t    usid;
    vtss_port_no_t port_no;
} topo_sid_port_t;


vtss_rc topo_upsid_upspn2sid_port(const vtss_vstax_upsid_t upsid,
                                  const vtss_vstax_upspn_t upspn,
                                  topo_sid_port_t *sid_port);
#endif 



vtss_rc topo_stack_config_get(vtss_isid_t isid, stack_config_t *stack_config, BOOL *dirty);
vtss_rc topo_stack_config_set(vtss_isid_t isid, stack_config_t *stack_config);



#if VTSS_SWITCH_MANAGED



typedef enum {
    TOPO_STACK_PORT_FWD_MODE_NONE,   
    TOPO_STACK_PORT_FWD_MODE_LOCAL,  
    TOPO_STACK_PORT_FWD_MODE_ACTIVE, 
    TOPO_STACK_PORT_FWD_MODE_BACKUP, 
} topo_stack_port_fwd_mode_t;

typedef struct {
    
    
    
    
    
    
    
    
    vtss_sprout_dist_t           dist_pri[2];

    
    
    
    
    
    
    
    
    vtss_sprout_dist_t           dist_sec[2];

    
    char dist_str[2][sizeof("11-12")];

    
    topo_stack_port_fwd_mode_t stack_port_fwd_mode[2];

    
    
    
    
    
    vtss_port_no_t               shortest_path;

    
    ushort             ups_cnt;

    
    vtss_vstax_upsid_t upsid[2];

    
    u64                ups_port_mask[2];
} topo_chip_t;


typedef struct {
    BOOL               vld;

    
    
    BOOL               me;

    mac_addr_t         mac_addr;
    vtss_usid_t        usid; 
    vtss_isid_t        isid; 

    BOOL               present;

    BOOL                         mst_capable;
    vtss_sprout_mst_elect_prio_t mst_elect_prio;
    time_t                       mst_time;
    BOOL                         mst_time_ignore;

    vtss_ipv4_t                  ip_addr;

    uint                         chip_cnt;

    
    
    
    
    topo_chip_t                  chip[2];
} topo_switch_t;





typedef struct {
    mac_addr_t  mst_switch_mac_addr;
    ulong       mst_change_time;

    topo_switch_t ts[VTSS_SPROUT_SIT_SIZE + 15];
} topo_switch_list_t;


vtss_rc topo_switch_list_get(topo_switch_list_t *const tsl_p);

const char *topo_stack_port_fwd_mode_to_str(
    const topo_stack_port_fwd_mode_t stack_port_fwd_mode);


vtss_rc topo_switch_stat_get(
    const vtss_isid_t   isid,
    topo_switch_stat_t *const stat_p);
#endif




typedef int (*topo_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));

#if VTSS_SWITCH_MANAGED

void topo_dbg_isid_tbl_print(
    const topo_dbg_printf_t  dbg_printf);



void topo_parm_set_default(void);
#endif

void topo_dbg_sprout(
    const topo_dbg_printf_t  dbg_printf,
    const uint               parms_cnt,
    const ulong *const       parms);

void topo_dbg_test(
    const topo_dbg_printf_t  dbg_printf,
    const int arg1,
    const int arg2,
    const int arg3);







void topo_led_update_set(BOOL enable);



#endif 
#endif 







