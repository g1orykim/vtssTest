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

 $Id: topo.c,v 1.213 2011/04/13 13:44:50 toe Exp $
 $Revision: 1.213 $

*/

#include "topo.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "sprout_cli.h"
#endif

#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TOPO





#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "topo",
    .descr     = "Topology module"
};


#ifndef TOPO_DEFAULT_TRACE_LVL
#define TOPO_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CFG] = {
        .name      = "cfg",
        .descr     = "Configuration",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_RXPKT_DUMP] = {
        .name      = "rxpktdump",
        .descr     = "Hex dump of received packets (lvl=noise)",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_TXPKT_DUMP] = {
        .name      = "txpktdump",
        .descr     = "Hex dump of transmitted packets (lvl=noise)",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_UPSID] = {
        .name      = "upsid",
        .descr     = "UPSID handling",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_FAILOVER] = {
        .name      = "failover",
        .descr     = "Selected failover related debug output with usec timestamp",
        .lvl       = TOPO_DEFAULT_TRACE_LVL,
        .timestamp = 1,
        .usec      = 1,
        .ringbuf   = 1,
    },
};
#endif 
















static critd_t  crit_topo_cfg_wr;

static critd_t  crit_topo_cfg_rd;

static critd_t  crit_topo_callback_regs;

static critd_t  crit_topo_generic_req_msg;
static critd_t  crit_topo_generic_rsp_msg;

static critd_t  crit_topo_generic_res;



#if VTSS_TRACE_ENABLED
#define TOPO_CFG_RD_CRIT_ENTER()               critd_enter(        &crit_topo_cfg_rd,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_CFG_RD_CRIT_EXIT()                critd_exit(         &crit_topo_cfg_rd,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_CFG_RD_CRIT_ENTER_RACKET()        critd_enter(        &crit_topo_cfg_rd,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define TOPO_CFG_RD_CRIT_EXIT_RACKET()         critd_exit(         &crit_topo_cfg_rd,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define TOPO_CFG_RD_CRIT_ASSERT_LOCKED()       critd_assert_locked(&crit_topo_cfg_rd,          TRACE_GRP_CRIT,                        __FILE__, __LINE__)
#define _TOPO_CFG_WR_CRIT_ENTER()              critd_enter(        &crit_topo_cfg_wr,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define _TOPO_CFG_WR_CRIT_EXIT()               critd_exit(         &crit_topo_cfg_wr,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define _TOPO_CFG_WR_CRIT_ENTER_RACKET()       critd_enter(        &crit_topo_cfg_wr,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define _TOPO_CFG_WR_CRIT_EXIT_RACKET()        critd_exit(         &crit_topo_cfg_wr,          TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define TOPO_CFG_WR_CRIT_ASSERT_LOCKED()       critd_assert_locked(&crit_topo_cfg_wr,          TRACE_GRP_CRIT,                        __FILE__, __LINE__)
#define TOPO_CALLBACK_REGS_CRIT_ENTER()        critd_enter(        &crit_topo_callback_regs,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_CALLBACK_REGS_CRIT_EXIT()         critd_exit(         &crit_topo_callback_regs,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_REQ_MSG_CRIT_ENTER()      critd_enter(        &crit_topo_generic_req_msg, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_REQ_MSG_CRIT_EXIT()       critd_exit(         &crit_topo_generic_req_msg, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_RSP_MSG_CRIT_ENTER()      critd_enter(        &crit_topo_generic_rsp_msg, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_RSP_MSG_CRIT_EXIT()       critd_exit(         &crit_topo_generic_rsp_msg, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_RES_CRIT_ENTER()          critd_enter(        &crit_topo_generic_res,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_RES_CRIT_EXIT()           critd_exit(         &crit_topo_generic_res,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define TOPO_GENERIC_RES_CRIT_ASSERT_LOCKED()  critd_assert_locked(&crit_topo_generic_res,     TRACE_GRP_CRIT,                        __FILE__, __LINE__)
#else

#define TOPO_CFG_RD_CRIT_ENTER()              critd_enter(        &crit_topo_cfg_rd         )
#define TOPO_CFG_RD_CRIT_EXIT()               critd_exit(         &crit_topo_cfg_rd         )
#define TOPO_CFG_RD_CRIT_ENTER_RACKET()       critd_enter(        &crit_topo_cfg_rd         )
#define TOPO_CFG_RD_CRIT_EXIT_RACKET()        critd_exit(         &crit_topo_cfg_rd         )
#define TOPO_CFG_RD_CRIT_ASSERT_LOCKED()      critd_assert_locked(&crit_topo_cfg_rd         )
#define _TOPO_CFG_WR_CRIT_ENTER()             critd_enter(        &crit_topo_cfg_wr         )
#define _TOPO_CFG_WR_CRIT_EXIT()              critd_exit(         &crit_topo_cfg_wr         )
#define _TOPO_CFG_WR_CRIT_ENTER_RACKET()      critd_enter(        &crit_topo_cfg_wr         )
#define _TOPO_CFG_WR_CRIT_EXIT_RACKET()       critd_exit(         &crit_topo_cfg_wr         )
#define TOPO_CFG_WR_CRIT_ASSERT_LOCKED()      critd_assert_locked(&crit_topo_cfg_wr         )
#define TOPO_CALLBACK_REGS_CRIT_ENTER()       critd_enter(        &crit_topo_callback_regs  )
#define TOPO_CALLBACK_REGS_CRIT_EXIT()        critd_exit(         &crit_topo_callback_regs  )
#define TOPO_GENERIC_REQ_MSG_CRIT_ENTER()     critd_enter(        &crit_topo_generic_req_msg)
#define TOPO_GENERIC_REQ_MSG_CRIT_EXIT()      critd_exit(         &crit_topo_generic_req_msg)
#define TOPO_GENERIC_RSP_MSG_CRIT_ENTER()     critd_enter(        &crit_topo_generic_rsp_msg)
#define TOPO_GENERIC_RSP_MSG_CRIT_EXIT()      critd_exit(         &crit_topo_generic_rsp_msg)
#define TOPO_GENERIC_RES_CRIT_ENTER()         critd_enter(        &crit_topo_generic_res    )
#define TOPO_GENERIC_RES_CRIT_EXIT()          critd_exit(         &crit_topo_generic_res    )
#define TOPO_GENERIC_RES_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit_topo_generic_res    )
#endif


#define TOPO_CFG_WR_CRIT_ENTER()         { _TOPO_CFG_WR_CRIT_ENTER();        TOPO_CFG_RD_CRIT_ENTER();        }
#define TOPO_CFG_WR_CRIT_ENTER_RACKET()  { _TOPO_CFG_WR_CRIT_ENTER_RACKET(); TOPO_CFG_RD_CRIT_ENTER_RACKET(); }
#define TOPO_CFG_WR_CRIT_EXIT()          {  TOPO_CFG_RD_CRIT_EXIT();        _TOPO_CFG_WR_CRIT_EXIT();         }
#define TOPO_CFG_WR_CRIT_EXIT_RACKET()   {  TOPO_CFG_RD_CRIT_EXIT_RACKET(); _TOPO_CFG_WR_CRIT_EXIT_RACKET();  }


#define TOPO_CFG_RD_CRIT_EXIT_TMP(cmd)   {  TOPO_CFG_RD_CRIT_EXIT(); {cmd;}; TOPO_CFG_RD_CRIT_ENTER(); }






static void topo_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc);

static BOOL topo_msg_rx(
    void                   *contxt,
    const void            *const msg,
    const size_t           len,
    const vtss_module_id_t modid,
    const ulong            isid);






static const uchar zeros[VTSS_SPROUT_FW_VER_LEN]; 


typedef struct {
    topo_state_change_callback_t callback;
    vtss_module_id_t             module_id;
} topo_state_change_reg_t;


typedef struct {
    topo_upsid_change_callback_t callback;
    vtss_module_id_t             module_id;
} topo_upsid_change_reg_t;



#define RETURN_RC(rc) \
    { \
        if (rc < 0) { \
            T_W("exit, rc.module_id=%d, rc.code=%d (%s)", \
                VTSS_RC_GET_MODULE_ID(rc), VTSS_RC_GET_MODULE_CODE(rc), error_txt(rc)); \
        } else { \
            T_N("exit, rc.module_id=%d, rc.code=%d (%s)", \
                VTSS_RC_GET_MODULE_ID(rc), VTSS_RC_GET_MODULE_CODE(rc), error_txt(rc)); \
        } \
        return rc; \
    }


#define RETURN_RC_NEG(func) \
    { \
      if ((rc = func) < 0) { \
        T_W("exit, rc.module_id=%d, rc.code=%d (%s)", \
            VTSS_RC_GET_MODULE_ID(rc), VTSS_RC_GET_MODULE_CODE(rc), error_txt(rc)); \
        return rc; \
      } \
    }


#define T_W_RC_NEG(func) \
    { \
      if ((rc = func) < 0) { \
        T_W("rc.module_id=%d, rc.code=%d (%s)", \
            VTSS_RC_GET_MODULE_ID(rc), VTSS_RC_GET_MODULE_CODE(rc), error_txt(rc)); \
      } \
    }


#define T_E_RC_NEG(func) \
    { \
      if ((rc = func) < 0) { \
        T_E("rc.module_id=%d, rc.code=%d (%s)", \
            VTSS_RC_GET_MODULE_ID(rc), VTSS_RC_GET_MODULE_CODE(rc), error_txt(rc)); \
      } \
    }

void *rx_pkt_contxt = NULL;

static msg_rx_filter_t topo_msg_rx_filter = {
    .contxt = NULL,
    .cb     = topo_msg_rx,
    .modid  = VTSS_MODULE_ID_TOPO
};


typedef enum {
    TOPO_MSG_TYPE_MST_ELECT_PRIO_SET,
    TOPO_MSG_TYPE_SLAVE_USID_SET,
    TOPO_MSG_TYPE_SWITCH_STAT_REQ,
    TOPO_MSG_TYPE_SWITCH_STAT_RSP,
    TOPO_MSG_TYPE_FW_VER_MODE_SET,
    TOPO_MSG_TYPE_CMEF_MODE_SET,
    TOPO_MSG_TYPE_STACK_CONF_SET_REQ,
    TOPO_MSG_TYPE_STACK_CONF_GET_REQ,
    TOPO_MSG_TYPE_STACK_CONF_GET_RSP,
    TOPO_MSG_TYPE_NONE,
} topo_msg_type_t;




typedef struct {
    topo_msg_type_t       msg_type;
} topo_msg_base_t;


typedef struct {
    topo_msg_type_t       msg_type;
    topo_mst_elect_prio_t mst_elect_prio;
} topo_msg_mst_prio_elect_set_t;


typedef struct {
    topo_msg_type_t       msg_type;
    vtss_usid_t           usid; 
} topo_msg_slave_usid_set_t;


typedef struct {
    topo_msg_type_t       msg_type;
    
    uint                  in_use_id;
    
    uint                  req_id;
    topo_switch_stat_t    stat;
} topo_msg_switch_stat_get_t;

#if defined(VTSS_SPROUT_FW_VER_CHK)

typedef struct {
    topo_msg_type_t msg_type;

    
    union {
        vtss_sprout_fw_ver_mode_t fw_ver_mode;
        BOOL                      cmef_ena;
    } msg_val;
} topo_msg_config_t;
#endif


typedef struct {
    topo_msg_type_t msg_type;
    stack_config_t  conf;
    BOOL            dirty;
} topo_msg_stack_conf_t;





#define SLAVE_USID_TX_TIMER   5 
#define SLAVE_USID_RX_TIMOUT 16 


typedef struct {
    stack_config_t  conf;
    BOOL            dirty;
    vtss_mtimer_t   timer;
} topo_isid_stack_conf_t;









static cyg_handle_t topo_thread_handle;
static cyg_thread   topo_thread_block;
static char         topo_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define TOPO_SWITCH_STAT_REQ_CNT (8 * sizeof(cyg_flag_value_t)) 





static cyg_flag_t topo_switch_stat_get_flag;



#define INTERRUPT_FLAG_LINK_DOWN 0x01
static cyg_flag_t interrupt_flag;
static BOOL       link_down_interrupt[2]; 




static union {
    topo_msg_switch_stat_get_t switch_stat_get;
} topo_generic_req_msg, topo_generic_rsp_msg;




static uint topo_nxt_req_id;






static uint topo_rxed_req_id[TOPO_SWITCH_STAT_REQ_CNT];






static void *topo_generic_res_ptr[TOPO_SWITCH_STAT_REQ_CNT];



#define MAX_STATE_CHANGE_CALLBACK 10
static int state_change_callback_cnt = 0;
static topo_state_change_reg_t state_change_callback_regs[MAX_STATE_CHANGE_CALLBACK];



#define MAX_UPSID_CHANGE_CALLBACK 10
static int upsid_change_callback_cnt = 0;
static topo_upsid_change_reg_t upsid_change_callback_regs[MAX_UPSID_CHANGE_CALLBACK];

static vtss_sprout_switch_init_t sprout_switch_init;


static topo_sit_t  sit_copy;
static BOOL        sit_copy_vld; 
static BOOL        me_mst      = 0;
static vtss_isid_t me_mst_isid = 0;

const mac_addr_t mac_addr_null = {0, 0, 0, 0, 0, 0};





static BOOL isid_delete_pending[VTSS_ISID_END];
const  BOOL isid_delete_pending_null[VTSS_ISID_END];

static topo_msg_slave_usid_set_t periodic_msg_slave_usid[VTSS_ISID_END];

#if defined(VTSS_SPROUT_FW_VER_CHK)

static vtss_sprout_fw_ver_mode_t fw_ver_mode = VTSS_SPROUT_FW_VER_MODE_NORMAL;
#endif


static topo_isid_stack_conf_t topo_stack_conf[VTSS_ISID_END];


#define TOPO_STACK_CONF_TIMER       100
#define TOPO_STACK_CONF_REQ_TIMEOUT 5000


cyg_flag_t topo_stack_conf_flags;



typedef enum {
    TOPO_SWITCH_MODE_UNKNOWN,
    TOPO_SWITCH_MODE_STANDALONE,
    TOPO_SWITCH_MODE_STACKABLE,
} topo_switch_mode_t;

static topo_switch_mode_t topo_switch_mode = TOPO_SWITCH_MODE_UNKNOWN;








#define TOPO_SID_TBL_SIZE (16+1) 

typedef struct _topo_isid_info_t {
    BOOL        assigned;
    mac_addr_t  mac_addr; 
    vtss_usid_t usid;
} topo_isid_info_t;



#define VTSS_TOPO_UID_UNDEF 0
#define UID2UPSID(uid)   (((uid)   == VTSS_TOPO_UID_UNDEF)    ? (VTSS_VSTAX_UPSID_UNDEF) : ((uid)-1))
#define UPSID2UID(upsid) (((upsid) == VTSS_VSTAX_UPSID_UNDEF) ? (VTSS_TOPO_UID_UNDEF)     : ((upsid)+1))

typedef struct _topo_cfg_t {
    uchar                 ver;

    
    
    topo_mst_elect_prio_t mst_elect_prio;

    
    
    BOOL deci_secs_mst_time_ignore;

    
    
    uchar sprout_update_interval_slv;
    uchar sprout_update_interval_mst;
    
    
    uchar sprout_update_age_time;
    
    uchar sprout_update_limit;

    
    uchar fast_mac_age_time;
    uchar fast_mac_age_count;

    
    uchar uid_pref[2][2];

    
    
    topo_isid_info_t isid_tbl[TOPO_SID_TBL_SIZE];

    
    
    vtss_isid_t usid_tbl[TOPO_SID_TBL_SIZE];

    
    
    int deci_secs_slave_usid_tx;

    
    
    int deci_secs_slave_usid_rx;

    
    
    
    
    BOOL led_update_ena;
} topo_cfg_t;






#define CFG_L_SIZE_VER                        1 
#define CFG_L_SIZE_FLAGS                      1 
#define CFG_L_SIZE_MST_ELECT_PRIO             1
#define CFG_L_SIZE_SPROUT_UPDATE_INTERVAL_SLV 1
#define CFG_L_SIZE_SPROUT_UPDATE_INTERVAL_MST 1
#define CFG_L_SIZE_SPROUT_UPDATE_AGE_TIME     1
#define CFG_L_SIZE_SPROUT_UPDATE_LIMIT        1
#define CFG_L_SIZE_FAST_MAC_AGE_TIME          1
#define CFG_L_SIZE_FAST_MAC_AGE_COUNT         1
#define CFG_L_SIZE_UID_PREF                   4
#define CFG_L_SIZE_RSV1                       3 




#define CFG_L_OFFSET_VER                        0
#define CFG_L_OFFSET_FLAGS                      (CFG_L_OFFSET_VER                        + CFG_L_SIZE_VER)
#define CFG_L_OFFSET_MST_ELECT_PRIO             (CFG_L_OFFSET_FLAGS                      + CFG_L_SIZE_FLAGS)
#define CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_SLV (CFG_L_OFFSET_MST_ELECT_PRIO             + CFG_L_SIZE_MST_ELECT_PRIO)
#define CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_MST (CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_SLV + CFG_L_SIZE_SPROUT_UPDATE_INTERVAL_SLV)
#define CFG_L_OFFSET_SPROUT_UPDATE_AGE_TIME     (CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_MST + CFG_L_SIZE_SPROUT_UPDATE_INTERVAL_MST)
#define CFG_L_OFFSET_SPROUT_UPDATE_LIMIT        (CFG_L_OFFSET_SPROUT_UPDATE_AGE_TIME     + CFG_L_SIZE_SPROUT_UPDATE_AGE_TIME)
#define CFG_L_OFFSET_FAST_MAC_AGE_TIME          (CFG_L_OFFSET_SPROUT_UPDATE_LIMIT        + CFG_L_SIZE_SPROUT_UPDATE_LIMIT)
#define CFG_L_OFFSET_FAST_MAC_AGE_COUNT         (CFG_L_OFFSET_FAST_MAC_AGE_TIME          + CFG_L_SIZE_FAST_MAC_AGE_TIME)
#define CFG_L_OFFSET_UID_PREF                   (CFG_L_OFFSET_FAST_MAC_AGE_COUNT         + CFG_L_SIZE_FAST_MAC_AGE_COUNT)
#define CFG_L_OFFSET_RSV1                       (CFG_L_OFFSET_UID_PREF                   + CFG_L_SIZE_UID_PREF)
#define CFG_L_SIZE                              (CFG_L_OFFSET_RSV1                       + CFG_L_SIZE_RSV1)









#define CFG_G_SIZE_VER                        1 
#define CFG_G_SIZE_RSV1                       7 
#define CFG_G_SIZE_ISID_TBL_ENTRY             8 
#define CFG_G_SIZE_ISID_TBL                   (TOPO_SID_TBL_SIZE*CFG_G_SIZE_ISID_TBL_ENTRY)


#define CFG_G_OFFSET_VER                      0
#define CFG_G_OFFSET_RSV1                     (CFG_G_OFFSET_VER                        + CFG_G_SIZE_VER)
#define CFG_G_OFFSET_ISID_TBL                 (CFG_G_OFFSET_RSV1                       + CFG_G_SIZE_RSV1)
#define CFG_G_SIZE                            (CFG_G_OFFSET_ISID_TBL                   + CFG_G_SIZE_ISID_TBL)


static const topo_cfg_t CFG_DEFAULT = {
    .ver = 6, 

    .mst_elect_prio              = TOPO_SPROUT_MST_ELECT_PRIO_DEFAULT,
    .deci_secs_mst_time_ignore   = 0,

    .sprout_update_interval_slv  = VTSS_SPROUT_UPDATE_INTERVAL_SLV_DEFAULT,
    .sprout_update_interval_mst  = VTSS_SPROUT_UPDATE_INTERVAL_MST_DEFAULT,
    .sprout_update_age_time      = VTSS_SPROUT_UDATE_AGE_TIME_DEFAULT,
    .sprout_update_limit         = VTSS_SPROUT_UPDATE_LIMIT_DEFAULT,

    .fast_mac_age_time           = TOPO_FAST_MAC_AGE_TIME,
    .fast_mac_age_count          = TOPO_FAST_MAC_AGE_COUNT,

    .uid_pref[0][0] = VTSS_TOPO_UID_UNDEF,
    .uid_pref[0][1] = VTSS_TOPO_UID_UNDEF,
    .uid_pref[1][0] = VTSS_TOPO_UID_UNDEF,
    .uid_pref[1][1] = VTSS_TOPO_UID_UNDEF,

    .isid_tbl[ 0].assigned = 0,
    .isid_tbl[ 1].assigned = 0,
    .isid_tbl[ 2].assigned = 0,
    .isid_tbl[ 3].assigned = 0,
    .isid_tbl[ 4].assigned = 0,
    .isid_tbl[ 5].assigned = 0,
    .isid_tbl[ 6].assigned = 0,
    .isid_tbl[ 7].assigned = 0,
    .isid_tbl[ 8].assigned = 0,
    .isid_tbl[ 9].assigned = 0,
    .isid_tbl[10].assigned = 0,
    .isid_tbl[11].assigned = 0,
    .isid_tbl[12].assigned = 0,
    .isid_tbl[13].assigned = 0,
    .isid_tbl[14].assigned = 0,
    .isid_tbl[15].assigned = 0,
    .isid_tbl[16].assigned = 0,

    .isid_tbl[ 0].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 1].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 2].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 3].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 4].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 5].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 6].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 7].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 8].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[ 9].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[10].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[11].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[12].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[13].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[14].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[15].mac_addr = {0, 0, 0, 0, 0, 0},
    .isid_tbl[16].mac_addr = {0, 0, 0, 0, 0, 0},

    .isid_tbl[ 0].usid =  0,
    .isid_tbl[ 1].usid =  1,
    .isid_tbl[ 2].usid =  2,
    .isid_tbl[ 3].usid =  3,
    .isid_tbl[ 4].usid =  4,
    .isid_tbl[ 5].usid =  5,
    .isid_tbl[ 6].usid =  6,
    .isid_tbl[ 7].usid =  7,
    .isid_tbl[ 8].usid =  8,
    .isid_tbl[ 9].usid =  9,
    .isid_tbl[10].usid = 10,
    .isid_tbl[11].usid = 11,
    .isid_tbl[12].usid = 12,
    .isid_tbl[13].usid = 13,
    .isid_tbl[14].usid = 14,
    .isid_tbl[15].usid = 15,
    .isid_tbl[16].usid = 16,

    .usid_tbl[ 0] =   0,
    .usid_tbl[ 1] =   1,
    .usid_tbl[ 2] =   2,
    .usid_tbl[ 3] =   3,
    .usid_tbl[ 4] =   4,
    .usid_tbl[ 5] =   5,
    .usid_tbl[ 6] =   6,
    .usid_tbl[ 7] =   7,
    .usid_tbl[ 8] =   8,
    .usid_tbl[ 9] =   9,
    .usid_tbl[10] =  10,
    .usid_tbl[11] =  11,
    .usid_tbl[12] =  12,
    .usid_tbl[13] =  13,
    .usid_tbl[14] =  14,
    .usid_tbl[15] =  15,
    .usid_tbl[16] =  16,

    .deci_secs_slave_usid_tx = SLAVE_USID_TX_TIMER * 10,
    .deci_secs_slave_usid_rx = SLAVE_USID_RX_TIMOUT * 10,

    .led_update_ena = TRUE,
};


static topo_cfg_t topo_cfg;














static vtss_rc cfg_activate(BOOL init_phase)
{
    vtss_rc                  rc = VTSS_OK;

    T_D("enter");

    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_MST_ELECT_PRIO,             topo_cfg.mst_elect_prio));
    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_MST_TIME_IGNORE,            (topo_cfg.deci_secs_mst_time_ignore > 0)));
    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_SLV, topo_cfg.sprout_update_interval_slv));
    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_SPROUT_UPDATE_INTERVAL_MST, topo_cfg.sprout_update_interval_mst));
    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_SPROUT_UPDATE_AGE_TIME,     topo_cfg.sprout_update_age_time));
    T_E_RC_NEG(vtss_sprout_parm_set(init_phase, VTSS_SPROUT_PARM_SPROUT_UPDATE_LIMIT,        topo_cfg.sprout_update_limit));

    RETURN_RC(rc);
} 



static vtss_rc cfg_wr(
    BOOL        recreate_local,  
    BOOL        recreate_global, 
    topo_parm_t parm)
{
    vtss_rc rc = VTSS_OK;
    uchar        *conf_p;
    ulong        conf_size;
    vtss_isid_t  isid;
    uint         offset;

    T_D("enter. recreate_local=%d, recreate_global=%d, parm=%d",
        recreate_local, recreate_global, parm);

    TOPO_CFG_WR_CRIT_ASSERT_LOCKED();

    
    
    if (recreate_local || parm != TOPO_PARM_ISID_TBL) {
        if (recreate_local) {
            
            TOPO_CFG_RD_CRIT_EXIT_TMP(
                conf_p = conf_create(CONF_BLK_TOPO, CFG_L_SIZE));
            if (!conf_p) {
                T_E("Creation of local CONF_BLK_TOPO failed");
                RETURN_RC(TOPO_ERROR_GEN);
            }
            recreate_local = 1;
            conf_size = CFG_L_SIZE;
        } else {
            TOPO_CFG_RD_CRIT_EXIT_TMP(
                conf_p = conf_open(CONF_BLK_TOPO, &conf_size));
            if (!conf_p) {
                T_E("conf_open failed, creating new local CONF_BLK_TOPO block.");
                TOPO_CFG_RD_CRIT_EXIT_TMP(
                    conf_p = conf_create(CONF_BLK_TOPO, CFG_L_SIZE));
                if (!conf_p) {
                    T_E("Creation of local CONF_BLK_TOPO failed");
                    RETURN_RC(TOPO_ERROR_GEN);
                } else {
                    recreate_local = 1;
                    conf_size = CFG_L_SIZE;
                }
            }
        }

        TOPO_ASSERTR(conf_size == CFG_L_SIZE,
                     "conf_size=%u, expected %d", conf_size, CFG_L_SIZE);

        
        if (recreate_local) {
            memset(conf_p, 0, CFG_L_SIZE);
            conf_p[CFG_L_OFFSET_VER] = CFG_DEFAULT.ver;
        }

        if (recreate_local || parm == TOPO_PARM_MST_ELECT_PRIO) {
            conf_p[CFG_L_OFFSET_MST_ELECT_PRIO] = (topo_cfg.mst_elect_prio == CFG_DEFAULT.mst_elect_prio) ? 0 : topo_cfg.mst_elect_prio;
        }
        if (recreate_local || parm == TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV) {
            conf_p[CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_SLV] = (topo_cfg.sprout_update_interval_slv == CFG_DEFAULT.sprout_update_interval_slv) ? 0 : topo_cfg.sprout_update_interval_slv;
        }
        if (recreate_local || parm == TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST) {
            conf_p[CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_MST] = (topo_cfg.sprout_update_interval_mst == CFG_DEFAULT.sprout_update_interval_mst) ? 0 : topo_cfg.sprout_update_interval_mst;
        }
        if (recreate_local || parm == TOPO_PARM_SPROUT_UPDATE_AGE_TIME) {
            conf_p[CFG_L_OFFSET_SPROUT_UPDATE_AGE_TIME] = (topo_cfg.sprout_update_age_time == CFG_DEFAULT.sprout_update_age_time) ? 0 : topo_cfg.sprout_update_age_time;
        }
        if (recreate_local || parm == TOPO_PARM_SPROUT_UPDATE_LIMIT) {
            conf_p[CFG_L_OFFSET_SPROUT_UPDATE_LIMIT] = (topo_cfg.sprout_update_limit == CFG_DEFAULT.sprout_update_limit) ? 0 : topo_cfg.sprout_update_limit;
        }
        if (recreate_local || parm == TOPO_PARM_FAST_MAC_AGE_TIME) {
            conf_p[CFG_L_OFFSET_FAST_MAC_AGE_TIME] = (topo_cfg.fast_mac_age_time == CFG_DEFAULT.fast_mac_age_time) ? 0 : topo_cfg.fast_mac_age_time;
        }
        if (recreate_local || parm == TOPO_PARM_FAST_MAC_AGE_COUNT) {
            conf_p[CFG_L_OFFSET_FAST_MAC_AGE_COUNT] = (topo_cfg.fast_mac_age_count == CFG_DEFAULT.fast_mac_age_count) ? 0 : topo_cfg.fast_mac_age_count;
        }
        if (recreate_local || parm == TOPO_PARM_UID_0_0) {
            conf_p[CFG_L_OFFSET_UID_PREF + 0] = topo_cfg.uid_pref[0][0];
        }
        if (recreate_local || parm == TOPO_PARM_UID_0_1) {
            conf_p[CFG_L_OFFSET_UID_PREF + 1] = topo_cfg.uid_pref[0][1];
        }
        if (recreate_local || parm == TOPO_PARM_UID_1_0) {
            conf_p[CFG_L_OFFSET_UID_PREF + 2] = topo_cfg.uid_pref[1][0];
        }
        if (recreate_local || parm == TOPO_PARM_UID_1_1) {
            conf_p[CFG_L_OFFSET_UID_PREF + 3] = topo_cfg.uid_pref[1][1];
        }

        T_DG(TRACE_GRP_UPSID, "cfg_wr(): %u %u %u %u", conf_p[CFG_L_OFFSET_UID_PREF + 0], conf_p[CFG_L_OFFSET_UID_PREF + 1], conf_p[CFG_L_OFFSET_UID_PREF + 2], conf_p[CFG_L_OFFSET_UID_PREF + 3]);

        TOPO_CFG_RD_CRIT_EXIT_TMP(conf_close(CONF_BLK_TOPO));

        T_NG(TRACE_GRP_CFG, "Wrote local conf. size=%d", CFG_L_SIZE);
    }

    
    
    if (recreate_global || parm == TOPO_PARM_ISID_TBL) {
        if (recreate_global) {
            
            TOPO_CFG_RD_CRIT_EXIT_TMP(
                conf_p = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_TOPO, CFG_G_SIZE));
            if (!conf_p) {
                T_E("Creation of global CONF_BLK_TOPO failed");
                RETURN_RC(TOPO_ERROR_GEN);
            }
            recreate_global = 1;
            conf_size = CFG_G_SIZE;
        } else {
            TOPO_CFG_RD_CRIT_EXIT_TMP(
                conf_p = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOPO, &conf_size));
            if (!conf_p) {
                T_E("conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOPO, &conf_size) failed, creating new global CONF_BLK_TOPO block.");
                TOPO_CFG_RD_CRIT_EXIT_TMP(
                    conf_p = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_TOPO, CFG_G_SIZE));
                if (!conf_p) {
                    T_E("Creation of global CONF_BLK_TOPO failed");
                    RETURN_RC(TOPO_ERROR_GEN);
                } else {
                    recreate_global = 1;
                    conf_size = CFG_G_SIZE;
                }
            }
        }

        TOPO_ASSERTR(conf_size == CFG_G_SIZE,
                     "conf_size=%u, expected %d", conf_size, CFG_G_SIZE);

        
        if (recreate_global) {
            memset(conf_p, 0, CFG_L_SIZE);
            conf_p[CFG_L_OFFSET_VER] = CFG_DEFAULT.ver;
        }

        if (recreate_global || parm == TOPO_PARM_ISID_TBL) {
            for (isid = 1; isid < VTSS_ISID_END; isid++) {
                offset = CFG_G_OFFSET_ISID_TBL + isid * CFG_G_SIZE_ISID_TBL_ENTRY;
                memcpy(conf_p + offset, topo_cfg.isid_tbl[isid].mac_addr, 6);
                conf_p[offset + 6] = topo_cfg.isid_tbl[isid].usid;
            }
        }

        TOPO_CFG_RD_CRIT_EXIT_TMP(conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOPO));

        T_NG(TRACE_GRP_CFG, "Wrote global conf. size=%d", CFG_G_SIZE);
    }


    RETURN_RC(rc);
} 






#define CFG_RD_SEC_MASK_LOCAL  1
#define CFG_RD_SEC_MASK_GLOBAL 2
static vtss_rc cfg_rd(
    uint sec_mask)  
{
    vtss_rc rc = VTSS_OK;
    uchar        *conf_p;
    ulong        conf_size;
    vtss_isid_t  isid;
    uint         offset;

    T_D("enter, sec_mask=0x%x", sec_mask);

    TOPO_CFG_WR_CRIT_ASSERT_LOCKED();

    if (sec_mask & CFG_RD_SEC_MASK_LOCAL) {
        TOPO_CFG_RD_CRIT_EXIT_TMP(
            conf_p = conf_open(CONF_BLK_TOPO, &conf_size));
        if (!conf_p) {
            T_I("conf_open, local section, failed");
            
            topo_cfg = CFG_DEFAULT;
            RETURN_RC_NEG(cfg_wr(1, 0, 0));
        } else {
            if (conf_size != CFG_L_SIZE) {
                
                T_E("conf_size=%u, expected %d, local section => Recreating w. default values", conf_size, CFG_L_SIZE);

                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_close(CONF_BLK_TOPO));
                topo_cfg = CFG_DEFAULT;
                RETURN_RC_NEG(cfg_wr(1, 0, 0));
            } else if (*conf_p != CFG_DEFAULT.ver) {
                T_W("version=%d, expected %d, local section => Recreating w. default values", *conf_p, CFG_DEFAULT.ver);
                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_close(CONF_BLK_TOPO));
                topo_cfg = CFG_DEFAULT;

                RETURN_RC_NEG(cfg_wr(1, 0, 0));
            } else {
                
                topo_cfg.ver                        = conf_p[CFG_L_OFFSET_VER];
                topo_cfg.mst_elect_prio             = conf_p[CFG_L_OFFSET_MST_ELECT_PRIO];
                topo_cfg.sprout_update_interval_slv = conf_p[CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_SLV];
                topo_cfg.sprout_update_interval_mst = conf_p[CFG_L_OFFSET_SPROUT_UPDATE_INTERVAL_MST];
                topo_cfg.sprout_update_age_time     = conf_p[CFG_L_OFFSET_SPROUT_UPDATE_AGE_TIME];
                topo_cfg.sprout_update_limit        = conf_p[CFG_L_OFFSET_SPROUT_UPDATE_LIMIT];
                topo_cfg.fast_mac_age_time          = conf_p[CFG_L_OFFSET_FAST_MAC_AGE_TIME];
                topo_cfg.fast_mac_age_count         = conf_p[CFG_L_OFFSET_FAST_MAC_AGE_COUNT];

                
                topo_cfg.mst_elect_prio             = topo_cfg.mst_elect_prio             ? topo_cfg.mst_elect_prio             : CFG_DEFAULT.mst_elect_prio;
                topo_cfg.sprout_update_interval_slv = topo_cfg.sprout_update_interval_slv ? topo_cfg.sprout_update_interval_slv : CFG_DEFAULT.sprout_update_interval_slv;
                topo_cfg.sprout_update_interval_mst = topo_cfg.sprout_update_interval_mst ? topo_cfg.sprout_update_interval_mst : CFG_DEFAULT.sprout_update_interval_mst;
                topo_cfg.sprout_update_age_time     = topo_cfg.sprout_update_age_time     ? topo_cfg.sprout_update_age_time     : CFG_DEFAULT.sprout_update_age_time;
                topo_cfg.sprout_update_limit        = topo_cfg.sprout_update_limit        ? topo_cfg.sprout_update_limit        : CFG_DEFAULT.sprout_update_limit;
                topo_cfg.fast_mac_age_time          = topo_cfg.fast_mac_age_time          ? topo_cfg.fast_mac_age_time          : CFG_DEFAULT.fast_mac_age_time;
                topo_cfg.fast_mac_age_count         = topo_cfg.fast_mac_age_count         ? topo_cfg.fast_mac_age_count         : CFG_DEFAULT.fast_mac_age_count;

                topo_cfg.uid_pref[0][0]             = conf_p[CFG_L_OFFSET_UID_PREF + 0];
                topo_cfg.uid_pref[0][1]             = conf_p[CFG_L_OFFSET_UID_PREF + 1];
                topo_cfg.uid_pref[1][0]             = conf_p[CFG_L_OFFSET_UID_PREF + 2];
                topo_cfg.uid_pref[1][1]             = conf_p[CFG_L_OFFSET_UID_PREF + 3];

                T_DG(TRACE_GRP_UPSID, "UPSIDs: %u %u %u %u", topo_cfg.uid_pref[0][0], topo_cfg.uid_pref[0][1], topo_cfg.uid_pref[1][0], topo_cfg.uid_pref[1][1]);

                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_close(CONF_BLK_TOPO));
            }
        }
    }

    if (sec_mask & CFG_RD_SEC_MASK_GLOBAL) {
        TOPO_CFG_RD_CRIT_EXIT_TMP(
            conf_p = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOPO, &conf_size));
        if (!conf_p) {
            T_I("conf_sec_open, global section, failed");
            
            topo_cfg = CFG_DEFAULT;
            RETURN_RC_NEG(cfg_wr(0, 1, 0));
        } else {
            if (conf_size != CFG_G_SIZE) {
                
                T_E("conf_size=%u, expected %d, global section => Recreating w. default values",
                    conf_size, CFG_G_SIZE);

                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOPO));
                topo_cfg = CFG_DEFAULT;
                RETURN_RC_NEG(cfg_wr(0, 1, 0));
            } else if (*conf_p != CFG_DEFAULT.ver) {
                T_W("version=%d, expected %d, global section => Recreating w. default values",
                    *conf_p, CFG_DEFAULT.ver);
                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOPO));
                topo_cfg = CFG_DEFAULT;
                RETURN_RC_NEG(cfg_wr(0, 1, 0));
            } else {
                
                BOOL recreate_global = 0;
                topo_cfg.ver         = conf_p[CFG_G_OFFSET_VER];

                topo_cfg.isid_tbl[0] = CFG_DEFAULT.isid_tbl[0];
                for (isid = 1; isid < VTSS_ISID_END; isid++) {
                    offset = CFG_G_OFFSET_ISID_TBL + isid * CFG_G_SIZE_ISID_TBL_ENTRY;
                    memcpy(topo_cfg.isid_tbl[isid].mac_addr, conf_p + offset, 6);
                    topo_cfg.isid_tbl[isid].usid = conf_p[offset + 6];
                    if (topo_cfg.isid_tbl[isid].usid > VTSS_USID_CNT) {
                        
                        recreate_global = 1;
                        break;
                    }

                    topo_cfg.usid_tbl[topo_cfg.isid_tbl[isid].usid] = isid;

                    if (memcmp(topo_cfg.isid_tbl[isid].mac_addr, mac_addr_null, 6) != 0) {
                        topo_cfg.isid_tbl[isid].assigned = 1;
                    } else {
                        topo_cfg.isid_tbl[isid].assigned = 0;
                    }
                }

                TOPO_CFG_RD_CRIT_EXIT_TMP(conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOPO));

                if (recreate_global) {
                    topo_cfg = CFG_DEFAULT;
                    RETURN_RC_NEG(cfg_wr(0, 1, 0));
                }
            }
        }
    }

    RETURN_RC(rc);
} 


static vtss_rc topo_stack_conf_rd(stack_config_t *conf, BOOL *dirty)
{
    void *p;
    u32  size;
    stack_config_t *old = &topo_stack_conf[VTSS_ISID_LOCAL].conf;

    if ((p = conf_sec_open(CONF_SEC_LOCAL, CONF_BLK_STACKING, &size)) != NULL &&
        size == sizeof(*conf)) {
        memcpy(conf, p, size);
    } else {
        memset(conf, 0, sizeof(*conf));
        conf->stacking = TRUE;
        conf->port_0 = PORT_NO_STACK_0;
        conf->port_1 = PORT_NO_STACK_1;
    }
    *dirty = (conf->stacking == old->stacking &&
              conf->port_0 == old->port_0 &&
              conf->port_1 == old->port_1 ? 0 : 1);

    return VTSS_OK;
}


static vtss_rc topo_stack_conf_wr(stack_config_t *conf)
{
    vtss_rc rc = VTSS_UNSPECIFIED_ERROR;
    void    *p;
    u32     size = sizeof(*conf);

    if ((p = conf_sec_create(CONF_SEC_LOCAL, CONF_BLK_STACKING, size)) != NULL) {
        memcpy(p, conf, size);
        conf_sec_close(CONF_SEC_LOCAL, CONF_BLK_STACKING);
        rc = VTSS_OK;
    }
    return rc;
}









static char *mac_addr_to_str(const mac_addr_t mac_addr)
{
    static char s[18];

    sprintf(s, "%02x-%02x-%02x-%02x-%02x-%02x",
            mac_addr[0],
            mac_addr[1],
            mac_addr[2],
            mac_addr[3],
            mac_addr[4],
            mac_addr[5]);

    TOPO_ASSERT(strlen(s) <= 17, "!");

    return s;
} 








#if VTSS_SWITCH_MANAGED
static void sit2tsl(
    const vtss_sprout_topology_type_t topology_type,
    vtss_sprout_sit_entry_t *const    si_p,
    topo_switch_t           *const    ts_p)
{
    uint i;

    TOPO_CFG_RD_CRIT_ASSERT_LOCKED();
    TOPO_ASSERT(si_p->vld, "!");

    memset(ts_p, 0, sizeof(topo_switch_t));

    ts_p->vld             = 1;
    if (memcmp(sprout_switch_init.switch_addr.addr,
               si_p->switch_addr.addr, 6) == 0) {
        ts_p->me = 1;
    }

    memcpy(ts_p->mac_addr, si_p->switch_addr.addr, 6);
    if (me_mst) {
        ts_p->isid            = si_p->id;
        ts_p->usid            = topo_cfg.isid_tbl[ts_p->isid].usid;
    } else {
        ts_p->isid            = 0;
        ts_p->usid            = 0;
    }
    ts_p->present         = 1;
    ts_p->ip_addr         = si_p->ip_addr;
    ts_p->mst_capable     = si_p->mst_capable;
    ts_p->mst_elect_prio  = si_p->mst_elect_prio;
    ts_p->mst_time_ignore = si_p->mst_time_ignore;

    
    ts_p->mst_time = si_p->mst_time;

    ts_p->chip_cnt = si_p->chip_cnt;

    for (i = 0; i < si_p->chip_cnt; i++) {
        
        ts_p->chip[i].dist_pri[0] = si_p->chip[i].dist[0];
        ts_p->chip[i].dist_pri[1] = si_p->chip[i].dist[1];

        
        if (si_p->chip[i].dist[1] > 0) {
            ts_p->chip[i].dist_sec[0] = si_p->chip[i].dist[1] - 1;
        } else {
            ts_p->chip[i].dist_sec[0] = si_p->chip[i].dist[1];
        }

        if (si_p->chip[i].dist[0] > 0) {
            ts_p->chip[i].dist_sec[1] = si_p->chip[i].dist[0] + 1;
        } else {
            ts_p->chip[i].dist_sec[1] = si_p->chip[i].dist[0];
        }

        if (vtss_board_chipcount() == 2) {
            
            TOPO_ASSERT((ts_p->chip[i].dist_pri[0] != -1 &&
                         ts_p->chip[i].dist_sec[1] != -1) ||
                        (ts_p->chip[i].dist_pri[0] == -1 &&
                         ts_p->chip[i].dist_sec[1] == -1),
                        "!");
            TOPO_ASSERT((ts_p->chip[i].dist_pri[1] != -1 &&
                         ts_p->chip[i].dist_sec[0] != -1) ||
                        (ts_p->chip[i].dist_pri[1] == -1 &&
                         ts_p->chip[i].dist_sec[0] == -1),
                        "!");
        }

        
        if (ts_p->me) {
            
            sprintf(ts_p->chip[i].dist_str[0], "%d", 0);
            sprintf(ts_p->chip[i].dist_str[1], "%d", 0);
        } else {
            int j;
            if (vtss_board_chipcount() == 1) {
                
                for (j = 0; j < 2; j++) {
                    if (ts_p->chip[i].dist_pri[j] == -1) {
                        sprintf(ts_p->chip[i].dist_str[j], "-");
                    } else {
                        sprintf(ts_p->chip[i].dist_str[j], "%d", ts_p->chip[i].dist_pri[j]);
                    }
                }
            } else {
                
                if (ts_p->chip[i].dist_pri[0] == -1) {
                    sprintf(ts_p->chip[i].dist_str[0], "-");
                } else {
                    sprintf(ts_p->chip[i].dist_str[0], "%d-%d",
                            MIN(ts_p->chip[i].dist_pri[0],
                                ts_p->chip[i].dist_sec[1]),
                            MAX(ts_p->chip[i].dist_pri[0],
                                ts_p->chip[i].dist_sec[1]));
                }
                if (ts_p->chip[i].dist_pri[1] == -1) {
                    sprintf(ts_p->chip[i].dist_str[1], "-");
                } else {
                    sprintf(ts_p->chip[i].dist_str[1], "%d-%d",
                            MIN(ts_p->chip[i].dist_pri[1],
                                ts_p->chip[i].dist_sec[0]),
                            MAX(ts_p->chip[i].dist_pri[1],
                                ts_p->chip[i].dist_sec[0]));
                }
            }
        }

        ts_p->chip[i].shortest_path = si_p->chip[i].shortest_path;

        
        if (ts_p->me) {
            ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_LOCAL;
            ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_LOCAL;
        } else {

#if defined(VTSS_SPROUT_V1)
            BOOL stack_0;

            stack_0 = (ts_p->chip[i].shortest_path == PORT_NO_STACK_0);

            ts_p->chip[i].stack_port_fwd_mode[0] =
                (topology_type == TopoClosedLoop && !stack_0) ?
                TOPO_STACK_PORT_FWD_MODE_BACKUP :
                (topology_type == TopoOpenLoop && !stack_0) ?
                TOPO_STACK_PORT_FWD_MODE_NONE : TOPO_STACK_PORT_FWD_MODE_ACTIVE;

            ts_p->chip[i].stack_port_fwd_mode[1] =
                (topology_type == TopoClosedLoop && stack_0) ?
                TOPO_STACK_PORT_FWD_MODE_BACKUP :
                (topology_type == TopoOpenLoop && stack_0) ?
                TOPO_STACK_PORT_FWD_MODE_NONE : TOPO_STACK_PORT_FWD_MODE_ACTIVE;
#endif

#if defined(VTSS_SPROUT_V2)
            ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_NONE;
            ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_NONE;

            if (topology_type == TopoClosedLoop) {
                if (vtss_board_chipcount() == 1) {
                    if (ts_p->chip[i].dist_pri[0] < ts_p->chip[i].dist_pri[1]) {
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_BACKUP;
                    } else if (ts_p->chip[i].dist_pri[0] > ts_p->chip[i].dist_pri[1]) {
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_BACKUP;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                    } else {
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                    }
                } else {
                    
                    if (ts_p->chip[i].dist_pri[0] < ts_p->chip[i].dist_pri[1] &&
                        ts_p->chip[i].dist_sec[1] < ts_p->chip[i].dist_sec[0]) {
                        
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_BACKUP;
                    } else if (ts_p->chip[i].dist_pri[0] > ts_p->chip[i].dist_pri[1] &&
                               ts_p->chip[i].dist_sec[1] > ts_p->chip[i].dist_sec[0]) {
                        
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_BACKUP;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                    } else {
                        
                        ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                        ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                    }
                }
            } else if (topology_type == TopoOpenLoop) {
                if (ts_p->chip[i].dist_pri[0] != -1) {
                    ts_p->chip[i].stack_port_fwd_mode[0] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                }
                if (ts_p->chip[i].dist_pri[1] != -1) {
                    ts_p->chip[i].stack_port_fwd_mode[1] = TOPO_STACK_PORT_FWD_MODE_ACTIVE;
                }
            }
#endif
        }

        
        ts_p->chip[i].ups_cnt  = 1;

        ts_p->chip[i].upsid[0] = si_p->chip[i].upsid[0];
        ts_p->chip[i].upsid[1] = si_p->chip[i].upsid[1];

        ts_p->chip[i].ups_port_mask[0] = si_p->chip[i].ups_port_mask[0];
        ts_p->chip[i].ups_port_mask[1] = si_p->chip[i].ups_port_mask[1];
    }
} 
#endif








static BOOL topo_led_usid_set(
    const vtss_isid_t isid,
    const vtss_usid_t usid)
{
    TOPO_CFG_WR_CRIT_ASSERT_LOCKED();

    T_D("isid=%d, usid=%d", isid, usid);

    if (!(vtss_switch_mgd() && vtss_switch_stackable())) {
        
        return 1;
    }

    if (isid == 0) {
        
        led_usid_set(usid);
        topo_cfg.deci_secs_slave_usid_rx = CFG_DEFAULT.deci_secs_slave_usid_rx;
        return 1;
    } else if (me_mst) {
        if (isid == me_mst_isid) {
            
            led_usid_set(usid);
            return 1;
        } else {
            
            topo_msg_slave_usid_set_t *topo_msg_p;

            if ((topo_msg_p = VTSS_MALLOC(sizeof(*topo_msg_p)))) {
                topo_msg_p->msg_type = TOPO_MSG_TYPE_SLAVE_USID_SET;
                topo_msg_p->usid     = usid;

                T_D("TOPO_MSG_TYPE_SLAVE_USID_SET->isid=%d: usid=%d msg_p=%p",
                    isid, usid, topo_msg_p);
                TOPO_CFG_RD_CRIT_EXIT();
                msg_tx_adv((void *)isid,
                           &topo_msg_tx_done,
                           MSG_TX_OPT_DONT_FREE,
                           VTSS_MODULE_ID_TOPO,
                           isid,
                           (void *)topo_msg_p,
                           sizeof(*topo_msg_p));
                TOPO_CFG_RD_CRIT_ENTER();
                return 0;
            } else {
                T_E("VTSS_MALLOC() failed, size=%zu", sizeof(*topo_msg_p));
                return 1;
            }
        }
    } else {
        
        T_E("topo_led_usid_set(%d, %d), but me_mst=%d",
            isid, usid, me_mst);
        return 1;
    }
} 

void topo_led_update_set(BOOL enable)
{
    TOPO_CFG_RD_CRIT_ENTER();
    topo_cfg.led_update_ena = enable;
    if (topo_cfg.led_update_ena) {
        
        topo_cfg.deci_secs_slave_usid_rx = CFG_DEFAULT.deci_secs_slave_usid_rx;
    }
    TOPO_CFG_RD_CRIT_EXIT();
}

static vtss_isid_t topo_mac2isid_int(
    const BOOL get_semaphore,
    const mac_addr_t mac_addr)
{
    vtss_isid_t        isid = 0;

    if (vtss_switch_unmgd()) {
        return isid;
    }

    if (get_semaphore) {
        TOPO_CFG_RD_CRIT_ENTER();
    } else {
        TOPO_CFG_RD_CRIT_ASSERT_LOCKED();
    }

    for (isid = 1; isid < VTSS_ISID_END; isid++) {
        if (memcmp(mac_addr, topo_cfg.isid_tbl[isid].mac_addr, 6) == 0) {
            break;
        }
    }

    if (get_semaphore) {
        TOPO_CFG_RD_CRIT_EXIT();
    }

    isid = (isid >= VTSS_ISID_END) ? 0 : isid;

    return isid;
} 


static void sit_isid_set(topo_sit_t *const sit_p)
{
    uint i;

    if (vtss_switch_unmgd()) {
        return;
    }

    TOPO_CFG_RD_CRIT_ASSERT_LOCKED();

    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_p->si[i].vld) {
            sit_p->si[i].id = topo_mac2isid_int(0, sit_p->si[i].switch_addr.addr);
        }
    }
} 




static vtss_isid_t isid_assign(
    const mac_addr_t mac_addr)
{
    vtss_isid_t isid = 0;

    if (vtss_switch_unmgd()) {
        return isid;
    }

    T_D("enter, mac_addr=%s",
        mac_addr_to_str(mac_addr));

    TOPO_CFG_WR_CRIT_ASSERT_LOCKED();

    
    for (isid = 1; isid < VTSS_ISID_END; isid++) {
        if (topo_cfg.isid_tbl[isid].assigned) {
            if (memcmp(topo_cfg.isid_tbl[isid].mac_addr, mac_addr, 6) == 0) {
                T_D("Assigned existing isid=%d", isid);
                return isid;
            }
        }
    }

    
    for (isid = 1; isid < VTSS_ISID_END; isid++) {
        if (!topo_cfg.isid_tbl[isid].assigned) {
            memcpy(topo_cfg.isid_tbl[isid].mac_addr, mac_addr, 6);
            topo_cfg.isid_tbl[isid].assigned = 1;
            cfg_wr(0, 0, TOPO_PARM_ISID_TBL); 
            T_D("Assigned new isid=%d", isid);
            return isid;
        }
    }

    return 0;
} 



static void isid_free(vtss_isid_t isid)
{
    int i;

    if (vtss_switch_unmgd()) {
        return;
    }

    T_D("enter, isid=%d", isid);

    TOPO_CFG_WR_CRIT_ASSERT_LOCKED();

    topo_cfg.isid_tbl[isid].assigned = 0;
    memset(&topo_cfg.isid_tbl[isid].mac_addr, 0, sizeof(mac_addr_t));
    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_copy.si[i].vld && sit_copy.si[i].id == isid) {
            sit_copy.si[i].id = 0;
            break;
        }
    }
    cfg_wr(0, 0, TOPO_PARM_ISID_TBL); 
    
    TOPO_CFG_RD_CRIT_EXIT_TMP(msg_topo_event(MSG_TOPO_EVENT_CONF_DEF, isid));
} 


static void sit_flush(topo_sit_t *sit_p)
{
    memset(sit_p, 0, sizeof(topo_sit_t));
} 


#if defined(VTSS_SPROUT_V2)

static void cmef_mode_set(BOOL enable)
{
    vtss_rc           rc;
    vtss_vstax_conf_t vstax_conf;

    vtss_appl_api_lock();
    if ((rc = vtss_vstax_conf_get(NULL, &vstax_conf)) != VTSS_OK) {
        T_E("vtss_vstax_conf_get() returned %d (%s)\n", rc, error_txt(rc));
    } else {
        vstax_conf.cmef_disable = !enable;
        if ((rc = vtss_vstax_conf_set(NULL, &vstax_conf)) != VTSS_OK) {
            T_E("vtss_vstax_conf_set() returned %d (%s)\n", rc, error_txt(rc));
        }
    }
    vtss_appl_api_unlock();
} 
#endif








#ifdef VTSS_SW_OPTION_PACKET

static BOOL rx_pkt(void  *contxt,
                   const uchar *const frm_p,
                   const vtss_packet_rx_info_t *const rx_info)
{
    T_DG(TRACE_GRP_RXPKT_DUMP,
         "enter, context=0x%p, frm_p=0x%p len=%u src_port=%u",
         contxt, frm_p, rx_info->length, rx_info->port_no);

    TOPO_ASSERT(frm_p != NULL, " ");

    T_NG(    TRACE_GRP_RXPKT_DUMP, "src_port=%u, len=%u", rx_info->port_no, rx_info->length);
    T_NG_HEX(TRACE_GRP_RXPKT_DUMP, frm_p, rx_info->length);

    TOPO_ASSERT(contxt == rx_pkt_contxt,
                "contxt=0x%p, expected 0x%p", contxt, rx_pkt_contxt);

    TOPO_ASSERT(rx_info->port_no == PORT_NO_STACK_0 || rx_info->port_no == PORT_NO_STACK_1,
                "src_port=%u", rx_info->port_no);

    vtss_sprout_rx_pkt(rx_info->port_no, frm_p, rx_info->length);

    return 0; 
} 
#endif


static void topo_link_down_interrupt(vtss_interrupt_source_t source_id,
                                     vtss_port_no_t          port_no)
{
    T_DG(TRACE_GRP_FAILOVER, "topo_link_down_interrupt port_no=%u", port_no);

    
    
    
    if (vtss_interrupt_source_hook_set(topo_link_down_interrupt, source_id, INTERRUPT_PRIORITY_NORMAL) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_FAILOVER, "Unable to hook interrupts (id = %d)", source_id);
    }

    if (port_no != PORT_NO_STACK_0 && port_no != PORT_NO_STACK_1) {
        
        return;
    }

    switch (source_id) {
    case INTERRUPT_SOURCE_LOS:
        
        link_down_interrupt[(port_no == PORT_NO_STACK_0) ? 0 : 1] = true;

        
        
        cyg_thread_set_priority(topo_thread_handle, THREAD_ABOVE_NORMAL_PRIO);

        
        cyg_flag_setbits(&interrupt_flag, INTERRUPT_FLAG_LINK_DOWN);
        break;

    default:
        T_E("Unexpected interrupt source: %d", source_id);
        break;
    }
} 



static void topo_port_state_change(
    vtss_port_no_t port_no,
    port_info_t *info_p)
{
    
    TOPO_ASSERT(info_p != NULL, " ");
    TOPO_ASSERT(info_p->link == 0 || info_p->link == 1, " ");

    if (port_no == PORT_NO_STACK_0 || port_no == PORT_NO_STACK_1) {
        T_D("enter, port_no=%u, link=%d", port_no, info_p->link);

        vtss_sprout_stack_port_link_state_set(port_no, info_p->link);
    }
} 


static void topo_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    vtss_isid_t      isid;
    topo_msg_base_t *topo_msg_base_p;

    topo_msg_base_p = (topo_msg_base_t *)msg;

    switch (topo_msg_base_p->msg_type) {
    case TOPO_MSG_TYPE_MST_ELECT_PRIO_SET:
        
        T_E("Unexpected msg_type=%d, msg=%p", topo_msg_base_p->msg_type, msg);
        break;

#if defined(VTSS_SPROUT_FW_VER_CHK)
    case TOPO_MSG_TYPE_FW_VER_MODE_SET:
#endif
#if defined(VTSS_SPROUT_V2)
    case TOPO_MSG_TYPE_CMEF_MODE_SET:
        
        T_E("Unexpected msg_type=%d, msg=%p", topo_msg_base_p->msg_type, msg);
        break;
#endif

    case TOPO_MSG_TYPE_SLAVE_USID_SET:
        if (contxt != NULL) {
            
            isid = (int)contxt;

            TOPO_ASSERT(VTSS_ISID_START <= isid && isid < VTSS_ISID_END, "contxt=%d", isid);

            TOPO_CFG_WR_CRIT_ENTER();
            if (isid_delete_pending[isid]) {

                TOPO_CFG_RD_CRIT_EXIT();
                T_D("tx_done: SWITCH_DEL, isid=%d ...", isid);
                msg_topo_event(MSG_TOPO_EVENT_SWITCH_DEL, isid);
                T_D("tx_done: SWITCH_DEL, isid=%d: Done", isid);
                TOPO_CFG_RD_CRIT_ENTER();

                isid_delete_pending[isid] = 0;
                isid_free(isid);

            }
            VTSS_FREE(msg);
            TOPO_CFG_WR_CRIT_EXIT();
        } else {
            
        }
        break;

    case TOPO_MSG_TYPE_SWITCH_STAT_REQ:
        TOPO_GENERIC_REQ_MSG_CRIT_EXIT(); 
        break;

    case TOPO_MSG_TYPE_SWITCH_STAT_RSP:
        TOPO_GENERIC_RSP_MSG_CRIT_EXIT(); 
        break;

    default:
        T_E("Unexpected msg_type=%d contxt=%p msg=%p", topo_msg_base_p->msg_type, contxt, msg);
        break;
    }
} 








static void topo_thread(cyg_addrword_t data)
{
    vtss_rc rc;
    
    
#ifdef VTSS_SW_OPTION_PACKET
    void *filter_id;
    packet_rx_filter_t rx_filter;
#endif
    ushort vtss_int_ver;
    int i;
    vtss_mtimer_t loop_timer;
    cyg_flag_value_t flag_value;

    
    T_D("enter, data: %d", data);

    
    
    T_W_RC_NEG(cfg_rd(CFG_RD_SEC_MASK_LOCAL));
    T_W_RC_NEG(cfg_activate(1));

    
    msg_rx_filter_register(&topo_msg_rx_filter);

#ifdef VTSS_SW_OPTION_PACKET
    
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid                 = VTSS_MODULE_ID_TOPO;
    rx_filter.match                 = PACKET_RX_FILTER_MATCH_SSPID | PACKET_RX_FILTER_MATCH_SRC_PORT;
    rx_filter.prio                  = 0;
    rx_filter.contxt                = rx_pkt_contxt;
    rx_filter.cb                    = rx_pkt;
    rx_filter.sspid                 = 0x0001;
    rx_filter.prio                  = PACKET_RX_FILTER_PRIO_SUPER;
    VTSS_PORT_BF_SET(rx_filter.src_port_mask, PORT_NO_STACK_0, 1);
    VTSS_PORT_BF_SET(rx_filter.src_port_mask, PORT_NO_STACK_1, 1);
    rc = packet_rx_filter_register(&rx_filter, &filter_id);
    TOPO_ASSERT(rc >= 0, "rc=%d", rc);
#endif

    
    vtss_int_ver = VTSS_INTERNAL_VERSION;

    
    T_W_RC_NEG(conf_mgmt_mac_addr_get(sprout_switch_init.switch_addr.addr, 0));
    
    if (sprout_switch_init.switch_addr.addr[0] == 0 &&
        sprout_switch_init.switch_addr.addr[1] == 0 &&
        sprout_switch_init.switch_addr.addr[2] == 0 &&
        sprout_switch_init.switch_addr.addr[3] == 0 &&
        sprout_switch_init.switch_addr.addr[4] == 0 &&
        sprout_switch_init.switch_addr.addr[5] == 0) {
        T_E("conf_mgmt_mac_addr_get returned 00-00-00-00-00-00");
        return;
    }
    if (vtss_switch_mgd()) {
        sprout_switch_init.mst_capable            = 1;
    } else {
        sprout_switch_init.mst_capable            = 0;
    }
#ifdef VTSS_SW_OPTION_PACKET
    sprout_switch_init.cpu_qu                     = PACKET_XTR_QU_SPROUT;
#endif
    for (i = 0; i < 2; i++) {
        sprout_switch_init.chip[i].upsid_pref            = VTSS_VSTAX_UPSID_UNDEF;
    }

    for (i = 0; i < vtss_board_chipcount(); i++) {
        sprout_switch_init.chip[i].upsid_pref            = UID2UPSID(topo_cfg.uid_pref[i][0]);
    }

    TOPO_ASSERT(vtss_board_chipcount() == 1 ||
                (port_custom_table[PORT_NO_STACK_0].map.chip_no == 0 &&
                 port_custom_table[PORT_NO_STACK_1].map.chip_no == 1),
                "Invalid stack port number configuration: %u/%u (chip: %u/%u)",
                PORT_NO_STACK_0, PORT_NO_STACK_1,
                port_custom_table[PORT_NO_STACK_0].map.chip_no,
                port_custom_table[PORT_NO_STACK_1].map.chip_no);

    
    sprout_switch_init.chip[0].stack_port[0].port_no = PORT_NO_STACK_0;
    sprout_switch_init.chip[0].stack_port[1].port_no = PORT_NO_STACK_1;
    sprout_switch_init.chip[1].stack_port[0].port_no = VTSS_PORT_NO_NONE;
    sprout_switch_init.chip[1].stack_port[1].port_no = VTSS_PORT_NO_NONE;

#if defined(VTSS_SPROUT_FW_VER_CHK)
    
    memcpy(sprout_switch_init.my_fw_ver, misc_software_version_txt(), MIN(strlen(misc_software_version_txt()), VTSS_SPROUT_FW_VER_LEN));
#endif

    
    TOPO_GENERIC_REQ_MSG_CRIT_EXIT();
    TOPO_GENERIC_RSP_MSG_CRIT_EXIT();
    TOPO_GENERIC_RES_CRIT_EXIT();
    TOPO_CFG_WR_CRIT_EXIT();

    T_W_RC_NEG(vtss_sprout_switch_init(&sprout_switch_init));

    
    T_W_RC_NEG(port_change_register(VTSS_MODULE_ID_TOPO, topo_port_state_change));

    
    T_W_RC_NEG(vtss_sprout_stack_port_adm_state_set(PORT_NO_STACK_0, 1));
    T_W_RC_NEG(vtss_sprout_stack_port_adm_state_set(PORT_NO_STACK_1, 1));

    if (vtss_board_features() & VTSS_BOARD_FEATURE_LOS) {
        if (vtss_interrupt_source_hook_set(topo_link_down_interrupt, INTERRUPT_SOURCE_LOS, INTERRUPT_PRIORITY_NORMAL) != VTSS_RC_OK) {
            T_EG(TRACE_GRP_FAILOVER, "Unable to hook interrupts");
        }
    }


    while (1) {
        
        T_W_RC_NEG(vtss_sprout_service_100msec());

        
        TOPO_CFG_WR_CRIT_ENTER_RACKET();
        if (topo_cfg.deci_secs_mst_time_ignore > 0) {
            if (--topo_cfg.deci_secs_mst_time_ignore == 0) {
                TOPO_CFG_WR_CRIT_EXIT_RACKET();
                T_E_RC_NEG(vtss_sprout_parm_set(0, VTSS_SPROUT_PARM_MST_TIME_IGNORE, (topo_cfg.deci_secs_mst_time_ignore > 0)));
                TOPO_CFG_WR_CRIT_ENTER_RACKET();
            }
        }

        if (me_mst) {
            topo_cfg.deci_secs_slave_usid_tx--;
            if (topo_cfg.deci_secs_slave_usid_tx <= 0) {
                
                
                vtss_isid_t isid;
                for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                    topo_msg_slave_usid_set_t *topo_msg_p;
                    BOOL managed_switch;

                    
                    TOPO_CFG_RD_CRIT_EXIT();
                    managed_switch = msg_switch_exists(isid);
                    TOPO_CFG_RD_CRIT_ENTER();
                    if (managed_switch) {
                        topo_msg_p = &periodic_msg_slave_usid[isid];
                        topo_msg_p->usid = topo_cfg.isid_tbl[isid].usid;
                        T_D("msg_tx_adv, isid=%d, msg=%p", isid, topo_msg_p);
                        TOPO_CFG_RD_CRIT_EXIT();
                        msg_tx_adv(NULL,
                                   &topo_msg_tx_done,
                                   MSG_TX_OPT_DONT_FREE,
                                   VTSS_MODULE_ID_TOPO,
                                   isid,
                                   (void *)topo_msg_p,
                                   sizeof(*topo_msg_p));
                        TOPO_CFG_RD_CRIT_ENTER();
                    }
                }
                topo_cfg.deci_secs_slave_usid_tx = SLAVE_USID_TX_TIMER * 10;
            }
        } else {
            
            topo_cfg.deci_secs_slave_usid_rx--;

            if (topo_cfg.led_update_ena && topo_cfg.deci_secs_slave_usid_rx <= 0) {
                topo_led_usid_set(0, 0);
            }
        }
        TOPO_CFG_WR_CRIT_EXIT_RACKET();

        VTSS_MTIMER_START(&loop_timer, 100);
        flag_value = cyg_flag_timed_wait(&interrupt_flag, INTERRUPT_FLAG_LINK_DOWN,
                                         CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
                                         loop_timer);

        if (flag_value == INTERRUPT_FLAG_LINK_DOWN) {
            T_DG(TRACE_GRP_FAILOVER, "topo_thread, link down");

            if (link_down_interrupt[0]) {
                link_down_interrupt[0] = false;
                vtss_sprout_stack_port_link_state_set(PORT_NO_STACK_0, 0);
            }
            if (link_down_interrupt[1]) {
                link_down_interrupt[1] = false;
                vtss_sprout_stack_port_link_state_set(PORT_NO_STACK_1, 0);
            }
        }
    }
} 







static vtss_rc topo_sprout_log_msg(char *msg)
{
    
    return VTSS_OK;
} 


static vtss_rc topo_sprout_cfg_save(vtss_sprout_cfg_save_t *sprout_cfg)
{
    vtss_rc rc = VTSS_OK;

    TOPO_CFG_WR_CRIT_ENTER();

    
    topo_cfg.uid_pref[0][0] = UPSID2UID(sprout_cfg->upsid[0]);
    if (vtss_board_chipcount() > 1) {
        topo_cfg.uid_pref[1][0] = UPSID2UID(sprout_cfg->upsid[1]);
    }

    T_W_RC_NEG(cfg_wr(0, 0, TOPO_PARM_UID_0_0));
    T_W_RC_NEG(cfg_wr(0, 0, TOPO_PARM_UID_0_1));
    T_W_RC_NEG(cfg_wr(0, 0, TOPO_PARM_UID_1_0));
    T_W_RC_NEG(cfg_wr(0, 0, TOPO_PARM_UID_1_1));

    TOPO_CFG_WR_CRIT_EXIT();

    RETURN_RC(rc);
} 


static vtss_rc topo_sprout_state_change(uchar state_change_mask)
{
    vtss_rc     rc = VTSS_OK;
    int         i, j;
    topo_sit_t *sit_old_p = NULL;
    topo_sit_t *sit_new_p = NULL;
    vtss_isid_t switches_gone_isid[VTSS_SPROUT_SIT_SIZE];
    uint        gone_cnt = 0;
    vtss_isid_t switches_new_isid[VTSS_SPROUT_SIT_SIZE];
    BOOL        switches_new_mgd[VTSS_SPROUT_SIT_SIZE];
    uint        new_cnt = 0;
    vtss_sprout_dist_t dist;
    BOOL        isids_with_changed_upsid[VTSS_ISID_END];
    vtss_isid_t isid;

    memset(switches_gone_isid, 0, sizeof(switches_gone_isid));
    memset(switches_new_isid,  0, sizeof(switches_new_isid));
    memset(isids_with_changed_upsid, 0, sizeof(isids_with_changed_upsid));

    T_D("enter, change_mask=0x%x", state_change_mask);

    TOPO_CFG_WR_CRIT_ENTER();

    TOPO_ASSERT(
        !((state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST) &&
          (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_SLV)),
        "Invalid state_change_mask: 0x%x", state_change_mask);

#if defined(VTSS_SPROUT_V1) && defined(VTSS_SW_OPTION_MAC)
    
    if (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_TTL) {
        
        TOPO_CFG_RD_CRIT_EXIT_TMP(mac_age_time_set(topo_cfg.fast_mac_age_time, topo_cfg.fast_mac_age_time * topo_cfg.fast_mac_age_count));
    }
#endif


    
    if (vtss_switch_mgd()) {
        if (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_SLV) {
            T_D("We have become slave!");

            TOPO_ASSERT(me_mst == 1, "change_mask=0x%x master=%s",
                        state_change_mask,
                        mac_addr_to_str(sit_copy.mst_switch_addr.addr));

            memset(isid_delete_pending, 0, VTSS_ISID_END);

            me_mst      = 0;
            me_mst_isid = 0;

            
            T_I("MASTER_DOWN ...");
            TOPO_CFG_RD_CRIT_EXIT();
            topo_led_usid_set(0, 0); 
            msg_topo_event(MSG_TOPO_EVENT_MASTER_DOWN, 0);
            TOPO_CFG_RD_CRIT_ENTER();
            T_D("MASTER_DOWN: Done");
        }
    }


    if (vtss_switch_mgd()) {
        if (me_mst ||
            (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST)) {
            
            if (!(sit_old_p = VTSS_MALLOC(sizeof(topo_sit_t)))) {
                T_E("VTSS_MALLOC() failed, size=%zu", sizeof(topo_sit_t));
                TOPO_CFG_WR_CRIT_EXIT();
                return TOPO_ALLOC_FAILED;
            }

            if (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST) {
                
                sit_flush(sit_old_p);
            } else {
                memcpy(sit_old_p, &sit_copy, sizeof(topo_sit_t));
            }
        }
    }


    
    
    vtss_sprout_sit_get(&sit_copy);
    sit_isid_set(&sit_copy);

    sit_copy_vld = 1;
    sit_new_p = &sit_copy;


    
    if (vtss_switch_mgd()) {
        if (me_mst ||
            (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST)) {
            

            TOPO_ASSERT(sit_old_p, "!");

            
            if (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST) {
                T_D("We have become master!");

                TOPO_ASSERT(me_mst == 0, "me_mst=%d, me_mst_isid=%d",
                            me_mst, me_mst_isid);

                TOPO_ASSERT(memcmp(isid_delete_pending, isid_delete_pending_null, VTSS_ISID_END) == 0, " ");

                
                T_W_RC_NEG(cfg_rd(CFG_RD_SEC_MASK_GLOBAL));

                me_mst = 1;
                me_mst_isid = isid_assign(sit_copy.mst_switch_addr.addr);

                TOPO_ASSERT(me_mst_isid != 0,
                            "No ISID could be assigned to me (%s) as master?!",
                            mac_addr_to_str(sit_copy.mst_switch_addr.addr));

                
                T_I("MASTER_UP, isid=%d ...", me_mst_isid);
                TOPO_CFG_RD_CRIT_EXIT();
                msg_topo_event(MSG_TOPO_EVENT_MASTER_UP, me_mst_isid);
                topo_led_usid_set(0, topo_cfg.isid_tbl[me_mst_isid].usid);
                TOPO_CFG_RD_CRIT_ENTER();
                T_D("MASTER_UP, isid=%d: Done", me_mst_isid);
            }

            
            if ((state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_STACK_MBR) ||
                (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_ME_MST)) {
                
                for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
                    if (sit_old_p->si[i].vld) {
                        BOOL found = 0;
                        for (j = 0; j < VTSS_SPROUT_SIT_SIZE; j++) {
                            if (sit_new_p->si[j].vld) {
                                if (memcmp(sit_old_p->si[i].switch_addr.addr,
                                           sit_new_p->si[j].switch_addr.addr,
                                           sizeof(vtss_sprout_switch_addr_t)) == 0) {
                                    
                                    found = 1;
                                    break;
                                }
                            }
                        }

                        if (!found) {
                            
                            T_D("Switch gone: %s",
                                mac_addr_to_str(sit_old_p->si[i].switch_addr.addr));

                            if (sit_old_p->si[i].id != 0) {
                                
                                switches_gone_isid[gone_cnt] = sit_old_p->si[i].id;
                                gone_cnt++;
                            }
                        }
                    }
                }

                
                
                for (dist = 0; dist < VTSS_SPROUT_MAX_UNITS_IN_STACK; dist++) {
                    for (j = 0; j < VTSS_SPROUT_SIT_SIZE; j++) {
                        if (sit_new_p->si[j].vld &&
                            MIN(sit_new_p->si[j].chip[0].dist[0] == VTSS_SPROUT_DIST_INFINITY ?
                                0xffff : sit_new_p->si[j].chip[0].dist[0],
                                sit_new_p->si[j].chip[0].dist[1] == VTSS_SPROUT_DIST_INFINITY ?
                                0xffff : sit_new_p->si[j].chip[0].dist[1]) == dist) {
                            BOOL found = 0;
                            for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
                                if (sit_old_p->si[i].vld) {
                                    if (memcmp(sit_old_p->si[i].switch_addr.addr,
                                               sit_new_p->si[j].switch_addr.addr,
                                               sizeof(vtss_sprout_switch_addr_t)) == 0) {
                                        
                                        found = 1;
                                        break;
                                    }
                                }
                            }

                            if (!found) {
                                
                                T_D("Switch new: %s, dist=%d",
                                    mac_addr_to_str(sit_new_p->si[j].switch_addr.addr), dist);

                                
                                sit_new_p->si[j].id =
                                    isid_assign(sit_new_p->si[j].switch_addr.addr);

                                if (sit_new_p->si[j].id) {
                                    
                                    switches_new_isid[new_cnt] = sit_new_p->si[j].id;

                                    switches_new_mgd[new_cnt] = 1;
                                    new_cnt++;
                                } else {
                                    T_D("New switch %s, but no free ISID => Not announced to msg",
                                        mac_addr_to_str(sit_new_p->si[j].switch_addr.addr));
                                }
                            }
                        }
                    }
                }
            }

            TOPO_ASSERT(me_mst == 1, "!");

            TOPO_CFG_RD_CRIT_EXIT();
            
            for (i = 0; i < gone_cnt; i++) {
                vtss_isid_t isid;
                isid = switches_gone_isid[i];

                isid_delete_pending[isid] = 0;

                T_D("sprout_state_change: SWITCH_DEL, isid=%d ...", isid);
                msg_topo_event(MSG_TOPO_EVENT_SWITCH_DEL, isid);
                T_D("SWITCH_DEL, isid=%d: Done", isid);
            }

            
            for (i = 0; i < new_cnt; i++) {
                vtss_isid_t isid;
                isid = switches_new_isid[i];

                TOPO_ASSERT(isid_delete_pending[isid] == 0, "!");

                T_D("SWITCH_ADD, isid=%d ...", isid);
                msg_topo_event(MSG_TOPO_EVENT_SWITCH_ADD, isid);
                T_D("SWITCH_ADD, isid=%d: Done", isid);
            }
            TOPO_CFG_RD_CRIT_ENTER();

            
            if ((state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_REMOTE) ||
                (state_change_mask & VTSS_SPROUT_STATE_CHANGE_MASK_UPSID_LOCAL)) {
                
                for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
                    if (sit_old_p->si[i].vld) {
                        for (j = 0; j < VTSS_SPROUT_SIT_SIZE; j++) {
                            if (sit_new_p->si[j].vld) {
                                if (memcmp(sit_old_p->si[i].switch_addr.addr,
                                           sit_new_p->si[j].switch_addr.addr,
                                           sizeof(vtss_sprout_switch_addr_t)) == 0) {
                                    
                                    if (sit_new_p->si[j].chip[0].upsid[0] !=
                                        sit_old_p->si[i].chip[0].upsid[0]
                                        ||
                                        sit_new_p->si[j].chip[0].upsid[1] !=
                                        sit_old_p->si[i].chip[0].upsid[1]
                                        ||
                                        sit_new_p->si[j].chip[1].upsid[0] !=
                                        sit_old_p->si[i].chip[1].upsid[0]
                                        ||
                                        sit_new_p->si[j].chip[1].upsid[1] !=
                                        sit_old_p->si[i].chip[1].upsid[1]) {
                                        
                                        TOPO_ASSERT(VTSS_ISID_LEGAL(sit_old_p->si[i].id),
                                                    "isid=%d", sit_old_p->si[i].id);
                                        isids_with_changed_upsid[sit_old_p->si[i].id] = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (sit_old_p) {
        VTSS_FREE(sit_old_p);
    }

    TOPO_CFG_RD_CRIT_EXIT();
    for (i = 0; i < state_change_callback_cnt; i++) {
        T_D("State change: Calling module_id=%d", state_change_callback_regs[i].module_id);
        state_change_callback_regs[i].callback(state_change_mask);
        T_D("State change: Called module_id=%d", state_change_callback_regs[i].module_id);
    }
    if (me_mst) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (isids_with_changed_upsid[isid] == 1) {
                T_DG(TRACE_GRP_UPSID,
                     "upsid_change_callback(isid=%d) to %d modules",
                     isid, upsid_change_callback_cnt);
                for (i = 0; i < upsid_change_callback_cnt; i++) {
                    T_DG(TRACE_GRP_UPSID,
                         "UPSID change: Calling module_id=%d",
                         upsid_change_callback_regs[i].module_id);
                    upsid_change_callback_regs[i].callback(isid);
                    T_DG(TRACE_GRP_UPSID,
                         "UPSID change: Called module_id=%d",
                         upsid_change_callback_regs[i].module_id);
                }
            }
        }
    }
    TOPO_CFG_RD_CRIT_ENTER();

    TOPO_CFG_WR_CRIT_EXIT();

    T_D("exit, rc=%d", rc);
    return rc;
} 


#define TOPO_PACKET_TX_DBG 0
#if TOPO_PACKET_TX_DBG
static void topo_packet_tx_done(void                   *context, 
                                packet_tx_done_props_t *props)  
{
    T_DG(TRACE_GRP_FAILOVER, "topo tx done");
} 

static void topo_packet_pre_tx(void   *tx_pre_contxt, 
                               u8     *frm,           
                               size_t len)           
{
    T_DG(TRACE_GRP_FAILOVER, "topo pre tx");
} 
#endif


static vtss_rc topo_sprout_tx_vstax2_pkt(
    vtss_port_no_t         port_no,
    vtss_vstax_tx_header_t *vstax2_hdr_p,
    uchar                  *pkt_p,
    uint                   len)
{
    vtss_rc rc = VTSS_OK;
#ifdef VTSS_SW_OPTION_PACKET
    uchar             *frm_p;
    packet_tx_props_t tx_props;

    
    
    
    
    
    
    

    T_N("enter, port_no=%u, len=%d", port_no, len);

    
    if ((frm_p = packet_tx_alloc(len)) == NULL) {
        T_W("packet_tx_alloc failed.");
        return TOPO_ALLOC_FAILED;
    }

    
    
    memcpy(frm_p, pkt_p, len);

    T_NG(    TRACE_GRP_TXPKT_DUMP, "port_no=%u, len=%d", port_no, len);
    T_NG_HEX(TRACE_GRP_TXPKT_DUMP, frm_p, len);

    
    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid      = VTSS_MODULE_ID_TOPO;
    tx_props.packet_info.frm[0]     = frm_p;
    tx_props.packet_info.len[0]     = len;
    tx_props.tx_info.dst_port_mask  = VTSS_BIT64(port_no);
    tx_props.tx_info.cos            = 8; 
    tx_props.tx_info.tx_vstax_hdr   = VTSS_PACKET_TX_VSTAX_SYM;
    tx_props.tx_info.vstax.sym      = *vstax2_hdr_p;
#if TOPO_PACKET_TX_DBG
    tx_props.packet_info.tx_done_cb = topo_packet_tx_done;
    tx_props.packet_info.tx_pre_cb  = topo_packet_pre_tx;
#endif

    if (packet_tx(&tx_props) != VTSS_RC_OK) {
        T_W("packet_tx failed");
        rc = TOPO_TX_FAILED;
    } else {
        T_N("packet_tx successful");
        rc = VTSS_OK;
    }
#endif

    RETURN_RC(rc);
} 


static ulong topo_sprout_secs_since_boot(void)
{
    ulong secs;

    secs = (cyg_current_time() * ECOS_MSECS_PER_HWTICK) / 1000;

    return secs;
} 


#if defined(VTSS_SPROUT_FW_VER_CHK)
static vtss_rc topo_sprout_fw_ver_chk(
    const vtss_port_no_t port_no,
    const BOOL           link_up,
    const uchar          *nbr_fw_ver)
{
    
    
    
    
    static BOOL fw_ver_error_port_state[2];
    
    static BOOL fw_ver_error_switch_state;

    vtss_rc        rc = VTSS_RC_OK;
    BOOL           error_set = 0;
    BOOL           error_clr = 0;
    BOOL           fw_ver_error_switch_state_new = fw_ver_error_switch_state;

    uint           sp_idx; 

    TOPO_ASSERT(port_no == PORT_NO_STACK_0 ||
                port_no == PORT_NO_STACK_1,
                "port_no=%u", port_no);

    sp_idx = (port_no == PORT_NO_STACK_0) ? 0 : 1;

    T_D("topo_sprout_fw_ver_chk args:\n"
        "port_no=%u\n"
        "link_up:=%d",
        port_no, link_up);

    T_D("topo_sprout_fw_ver_chk state:\n"
        "port_state=%d,%d\n"
        "switch_state=%d",
        fw_ver_error_port_state[0], fw_ver_error_port_state[1],
        fw_ver_error_switch_state);

    if (!link_up) {
        
        error_clr = fw_ver_error_port_state[sp_idx];
        fw_ver_error_port_state[sp_idx] = 0;
        rc = VTSS_RC_OK;
    } else {
        
        TOPO_ASSERT(nbr_fw_ver != NULL,
                    "Link up, but nbr_fw_ver=NULL");

        
        
        uchar my_fw_ver[VTSS_SPROUT_FW_VER_LEN + 1];
        memset(my_fw_ver, 0, VTSS_SPROUT_FW_VER_LEN + 1);
        memcpy(my_fw_ver, misc_software_version_txt(), MIN(strlen(misc_software_version_txt()), VTSS_SPROUT_FW_VER_LEN));

        if ((memcmp(my_fw_ver, nbr_fw_ver, VTSS_SPROUT_FW_VER_LEN) != 0) &&
            (memcmp(zeros,     nbr_fw_ver, VTSS_SPROUT_FW_VER_LEN) != 0) &&
            (fw_ver_mode != VTSS_SPROUT_FW_VER_MODE_NULL)) {
            
            if (!fw_ver_error_port_state[sp_idx]) {
                
                
                uchar nbr_fw_ver_str[VTSS_SPROUT_FW_VER_LEN + 1];
                memset(nbr_fw_ver_str, 0, VTSS_SPROUT_FW_VER_LEN + 1);
                memcpy(nbr_fw_ver_str, nbr_fw_ver, VTSS_SPROUT_FW_VER_LEN);

                T_W("Neighbour has incompatible firmware version, "
                    "thus stack connectivity cannot be established. "
                    "Port: %u, Neighbour version: %s, Local version: %s",
                    (uint)port_no, nbr_fw_ver_str, my_fw_ver);

#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("Neighbour has incompatible firmware version, "
                    "thus stack connectivity cannot be established. "
                    "Port: %u, Neighbour version: %s, Local version: %s",
                    (uint)port_no, nbr_fw_ver_str, my_fw_ver);
#endif

                error_set = 1;
                fw_ver_error_port_state[sp_idx] = 1;
            }
            rc = VTSS_RC_ERROR;
        } else {
            
            error_clr = fw_ver_error_port_state[sp_idx];
            fw_ver_error_port_state[sp_idx] = 0;

            rc = VTSS_RC_OK;
        }
    }

    if (error_clr || error_set) {
        if (error_set && !fw_ver_error_switch_state) {
            
            fw_ver_error_switch_state_new = 1;
        } else if (error_clr && fw_ver_error_switch_state) {
            
            fw_ver_error_switch_state_new =
                fw_ver_error_port_state[0] ||
                fw_ver_error_port_state[1];
        }
    }

    if (fw_ver_error_switch_state_new != fw_ver_error_switch_state) {
        fw_ver_error_switch_state = fw_ver_error_switch_state_new;
        if (fw_ver_error_switch_state) {
            T_I("Setting LED FW error (triggered by event on port %u)", port_no);
            led_front_led_state(LED_FRONT_LED_STACK_FW_CHK_ERROR);
        } else {
            T_I("Clearing LED FW error (triggered by event on port %u)", port_no);
            led_front_led_state(LED_FRONT_LED_NORMAL);
        }
    }

    return rc;
} 
#endif


#if VTSS_SWITCH_MANAGED






static vtss_rc topo_switch_stat_get__static(
    const vtss_isid_t   isid,
    topo_switch_stat_t *const stat_p,
    BOOL                time_convert)
{
    vtss_rc rc = VTSS_OK;
    uint req_id;
    BOOL wait = 1;

    memset(stat_p, 0, sizeof(*stat_p));

    if (isid == VTSS_ISID_LOCAL) {
        T_W_RC_NEG(vtss_sprout_stat_get(stat_p));
    } else if (isid == VTSS_ISID_GLOBAL) {
        T_E("Illegal isid: %d", isid);
    } else {
        if (!me_mst) {
            T_W("Not master");
        } else {
            topo_msg_switch_stat_get_t *topo_msg_p;
            u32 in_use_id, in_use_mask, i;
            BOOL found = FALSE;

            
            TOPO_GENERIC_REQ_MSG_CRIT_ENTER();
            TOPO_GENERIC_RES_CRIT_ENTER();
            req_id = topo_nxt_req_id++;

            for (i = 0; i < TOPO_SWITCH_STAT_REQ_CNT; i++) {
                if (topo_generic_res_ptr[i] == NULL) {
                    
                    in_use_id = i;
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                T_E("Too many threads are requesting the switch status at the same time");
                TOPO_GENERIC_RES_CRIT_EXIT();
                TOPO_GENERIC_REQ_MSG_CRIT_EXIT();
                return VTSS_RC_ERROR;
            }

            in_use_mask = 1U << in_use_id;

            topo_msg_p = &topo_generic_req_msg.switch_stat_get;
            memset(topo_msg_p, 0, sizeof(*topo_msg_p));
            topo_msg_p->msg_type  = TOPO_MSG_TYPE_SWITCH_STAT_REQ;
            topo_msg_p->in_use_id = in_use_id;
            topo_msg_p->req_id    = req_id;

            topo_generic_res_ptr[in_use_id] = stat_p;

            cyg_flag_maskbits(&topo_switch_stat_get_flag, ~in_use_mask);
            TOPO_GENERIC_RES_CRIT_EXIT();

            
            msg_tx_adv(NULL,
                       &topo_msg_tx_done,
                       MSG_TX_OPT_DONT_FREE,
                       VTSS_MODULE_ID_TOPO,
                       isid,
                       (void *)topo_msg_p,
                       sizeof(*topo_msg_p));
            while (wait) {
                if (!cyg_flag_timed_wait(&topo_switch_stat_get_flag, in_use_mask, CYG_FLAG_WAITMODE_OR, cyg_current_time() + 2000 / ECOS_MSECS_PER_HWTICK)) {
                    T_W("Timed out waiting for stat_get response (isid=%d)", isid);
                    TOPO_GENERIC_RES_CRIT_ENTER();
                    cyg_flag_maskbits(&topo_switch_stat_get_flag, ~in_use_mask);
                    topo_generic_res_ptr[in_use_id] = NULL;
                    TOPO_GENERIC_RES_CRIT_EXIT();
                    wait = 0;
                } else {
                    
                    TOPO_GENERIC_RES_CRIT_ENTER();

                    if (topo_rxed_req_id[in_use_id] == req_id) {
                        
                        topo_generic_res_ptr[in_use_id] = NULL;
                        wait = 0;
                    } else {
                        
                        
                        T_W("req_id=%d, waiting for %d", topo_rxed_req_id[in_use_id], req_id);
                    }
                    cyg_flag_maskbits(&topo_switch_stat_get_flag, ~in_use_mask);
                    TOPO_GENERIC_RES_CRIT_EXIT();
                }
            }
        }
    }

    if (time_convert) {
        
        
        
        
        stat_p->topology_change_time = msg_abstime_get(isid, stat_p->topology_change_time);
    }

    return rc;
}
#endif 





static BOOL topo_msg_rx(
    void                   *contxt,
    const void              *const msg,
    const size_t           len,
    const vtss_module_id_t modid,
    const ulong            id)
{
    vtss_isid_t     isid = id;
    topo_msg_base_t *topo_msg_base_p;

    T_D("Received msg: len=%u, modid=%d, id=%u",
        (unsigned int)len, modid, id);

    TOPO_ASSERT(modid == VTSS_MODULE_ID_TOPO, "modid=%d", modid);

    topo_msg_base_p = (topo_msg_base_t *)msg;

    switch (topo_msg_base_p->msg_type) {
    case TOPO_MSG_TYPE_MST_ELECT_PRIO_SET: {
        topo_msg_mst_prio_elect_set_t *topo_msg_p;
        topo_msg_p = (topo_msg_mst_prio_elect_set_t *)msg;

        if (TOPO_MST_ELECT_PRIO_LEGAL(topo_msg_p->mst_elect_prio)) {
            (void) topo_parm_set(0, TOPO_PARM_MST_ELECT_PRIO,
                                 topo_msg_p->mst_elect_prio);
        } else {
            T_W("Illegal mst_elect_prio: %d", topo_msg_p->mst_elect_prio);
        }
    }
    break;

#if defined(VTSS_SPROUT_FW_VER_CHK)
    case TOPO_MSG_TYPE_FW_VER_MODE_SET: {
        topo_msg_config_t *topo_msg_p;
        topo_msg_p = (topo_msg_config_t *)msg;

        if (TOPO_FW_VER_MODE_LEGAL(topo_msg_p->msg_val.fw_ver_mode)) {
            (void) topo_parm_set(0, TOPO_PARM_FW_VER_MODE,
                                 topo_msg_p->msg_val.fw_ver_mode);
        } else {
            T_W("Illegal fw_ver_mode: %d", topo_msg_p->msg_val.fw_ver_mode);
        }
    }
    break;

    case TOPO_MSG_TYPE_CMEF_MODE_SET: {
        topo_msg_config_t *topo_msg_p;
        topo_msg_p = (topo_msg_config_t *)msg;

        (void) topo_parm_set(0, TOPO_PARM_CMEF_MODE,
                             (topo_msg_p->msg_val.cmef_ena == 0) ? 0 : 1);
    }
    break;
#endif

    case TOPO_MSG_TYPE_SLAVE_USID_SET: {
        
        
        topo_msg_slave_usid_set_t *topo_msg_p;
        topo_msg_p = (topo_msg_slave_usid_set_t *)msg;

        if (!me_mst) {
            TOPO_CFG_WR_CRIT_ENTER();
            topo_led_usid_set(0, topo_msg_p->usid);
            TOPO_CFG_WR_CRIT_EXIT();
        }
#ifdef VTSS_SW_OPTION_LLDP
        
        lldp_local_usid_set_get(TRUE, topo_msg_p->usid);
#endif
    }
    break;

    case TOPO_MSG_TYPE_SWITCH_STAT_REQ: {
#if VTSS_SWITCH_MANAGED
        topo_msg_switch_stat_get_t *topo_req_msg_p;
        topo_msg_switch_stat_get_t *topo_rsp_msg_p;

        topo_req_msg_p = (topo_msg_switch_stat_get_t *)msg;

        TOPO_GENERIC_RSP_MSG_CRIT_ENTER(); 
        topo_rsp_msg_p            = &topo_generic_rsp_msg.switch_stat_get;
        topo_rsp_msg_p->msg_type  = TOPO_MSG_TYPE_SWITCH_STAT_RSP;
        topo_rsp_msg_p->in_use_id = topo_req_msg_p->in_use_id;
        topo_rsp_msg_p->req_id    = topo_req_msg_p->req_id;
        topo_switch_stat_get__static(VTSS_ISID_LOCAL, &topo_rsp_msg_p->stat, FALSE); 
        msg_tx_adv(NULL,
                   &topo_msg_tx_done,
                   MSG_TX_OPT_DONT_FREE,
                   VTSS_MODULE_ID_TOPO,
                   id, 
                   (void *)topo_rsp_msg_p,
                   sizeof(*topo_rsp_msg_p));
#endif
    }
    break;

    case TOPO_MSG_TYPE_SWITCH_STAT_RSP: {
        topo_msg_switch_stat_get_t *topo_rsp_msg_p;
        uint req_in_use_id;

        topo_rsp_msg_p = (topo_msg_switch_stat_get_t *)msg;
        req_in_use_id = topo_rsp_msg_p->in_use_id;

        if (req_in_use_id >= TOPO_SWITCH_STAT_REQ_CNT) {
            T_W("Illegal in_use id: %d", req_in_use_id);
        } else if (me_mst) {
            TOPO_GENERIC_RES_CRIT_ENTER();
            if (topo_generic_res_ptr[req_in_use_id] != NULL) {
                
                
                memcpy(topo_generic_res_ptr[req_in_use_id],
                       &topo_rsp_msg_p->stat,
                       sizeof(topo_rsp_msg_p->stat));

                
                topo_rxed_req_id[req_in_use_id] = topo_rsp_msg_p->req_id;

                
                cyg_flag_setbits(&topo_switch_stat_get_flag, 1U << req_in_use_id);
            } else {
                T_W("Unexpected response - must have timed out waiting: res_ptr:%p",
                    topo_generic_res_ptr[req_in_use_id]);
            }
            TOPO_GENERIC_RES_CRIT_EXIT();
        } else {
            T_W("STAT_RSP received by slave");
        }
    }
    break;

    case TOPO_MSG_TYPE_STACK_CONF_SET_REQ: {
        topo_msg_stack_conf_t *conf_msg = (topo_msg_stack_conf_t *)msg;

        
        T_D("STACK_CONF_SET_REQ");
        if (topo_stack_conf_wr(&conf_msg->conf) != VTSS_OK) {
            T_W("topo_stack_conf_wr failed");
        }
    }
    break;

    case TOPO_MSG_TYPE_STACK_CONF_GET_REQ: {
        topo_msg_stack_conf_t *conf_msg;

        
        T_D("STACK_CONF_GET_REQ");
        if ((conf_msg = VTSS_MALLOC(sizeof(*conf_msg))) != NULL) {
            conf_msg->msg_type = TOPO_MSG_TYPE_STACK_CONF_GET_RSP;
            if (topo_stack_conf_rd(&conf_msg->conf, &conf_msg->dirty) != VTSS_OK) {
                T_W("topo_stack_conf_rd failed");
                VTSS_FREE(conf_msg);
            } else {
                msg_tx(VTSS_MODULE_ID_TOPO, isid, (uchar *)conf_msg, sizeof(*conf_msg));
            }
        }
    }
    break;

    case TOPO_MSG_TYPE_STACK_CONF_GET_RSP: {
        topo_msg_stack_conf_t  *conf_msg = (topo_msg_stack_conf_t *)msg;
        topo_isid_stack_conf_t *conf;

        
        T_D("STACK_CONF_GET_RSP");
        if (VTSS_ISID_LEGAL(isid)) {
            conf = &topo_stack_conf[isid];
            conf->conf = conf_msg->conf;
            conf->dirty = conf_msg->dirty;
            VTSS_MTIMER_START(&conf->timer, TOPO_STACK_CONF_TIMER);
            cyg_flag_setbits(&topo_stack_conf_flags, 1 << isid);
        }
    }
    break;

    default:
        T_W("Unknown msg type: %d", topo_msg_base_p->msg_type);
    }

    return TRUE;
} 


static void topo_thread_set_priority_normal(void)
{
    cyg_thread_set_priority(topo_thread_handle, THREAD_DEFAULT_PRIO);
} 
















const char *topo_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case TOPO_ERROR_PARM:
        txt = "Illegal parameter";
        break;
    case TOPO_ASSERT_FAILURE:
        txt = "Assertion failed";
        break;
    case TOPO_ALLOC_FAILED:
        txt = "Ressource allocation failed";
        break;
    case TOPO_TX_FAILED:
        txt = "SPROUT Update tx failed";
        break;
    case TOPO_ERROR_SWITCH_NOT_PRESENT:
        txt = "Switch not present in stack";
        break;
    case TOPO_ERROR_SID_NOT_ASSIGNED:
        txt = "SID not assigned to any switch";
        break;
    case TOPO_ERROR_REMOTE_PARM_ONLY_FROM_MST:
        txt = "Remote parameter can only be set from master";
        break;
    case TOPO_ERROR_REMOTE_PARM_NOT_SUPPORTED:
        txt = "Remote assignment not supported";
        break;
    case TOPO_ERROR_NO_SWITCH_SELECTED:
        txt = "No switch selected";
        break;
    case TOPO_ERROR_NOT_MASTER:
        txt = "Operation only supported on master";
        break;
    case TOPO_ERROR_SID_IN_USE:
        txt = "SID is in use by another switch";
        break;
    case TOPO_ERROR_SWITCH_HAS_SID:
        txt = "Switch has already been assigned to a SID";
        break;
    case TOPO_ERROR_ISID_DELETE_PENDING:
        txt = "Operation not allowed, deletion is pending.";
        break;
    case TOPO_ERROR_CONFIG_ILLEGAL_STACK_PORT:
        txt = "Illegal stack port given";
        break;
    default:
        txt = "TOPO unknown error";
    }
    return txt;
} 


vtss_rc topo_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;
    vtss_isid_t isid = data->isid;
    uint i;
    uchar *conf, my_mac[6];
    ulong conf_size;
    BOOL dirty;

    switch (data->cmd) {
    case INIT_CMD_INIT: {
        vtss_sprout_init_t sprout_init;

        
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        T_D("enter, cmd=INIT");

        
        (void)topo_stack_conf_rd(&topo_stack_conf[VTSS_ISID_LOCAL].conf, &dirty);

        
        critd_init(&crit_topo_cfg_rd, "crit_topo_cfg_rd", VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        if (PORT_NO_STACK_0 == VTSS_PORT_NO_NONE) {
            
            topo_switch_mode = TOPO_SWITCH_MODE_STANDALONE;

            
            TOPO_ASSERT(PORT_NO_STACK_1 == VTSS_PORT_NO_NONE, "PORT_NO_STACK_1=%u", PORT_NO_STACK_1);

            TOPO_CFG_RD_CRIT_EXIT();

            
            break;
        } else {
            topo_switch_mode = TOPO_SWITCH_MODE_STACKABLE;

            
            TOPO_ASSERT(PORT_NO_STACK_1 != VTSS_PORT_NO_NONE, "PORT_NO_STACK_1=%u", PORT_NO_STACK_1);
        }

        
        topo_generic_req_msg = topo_generic_rsp_msg;
        topo_nxt_req_id      = 0;

        cyg_flag_init(&interrupt_flag);

#ifdef VTSS_SW_OPTION_VCLI
        sprout_cli_req_init();
#endif

        
        cyg_flag_init(&topo_switch_stat_get_flag);

        
        
        
        
        critd_init(&crit_topo_generic_req_msg, "crit_topo_generic_req_msg", VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_SEMAPHORE);
        critd_init(&crit_topo_generic_rsp_msg, "crit_topo_generic_rsp_msg", VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_SEMAPHORE);
        critd_init(&crit_topo_generic_res,     "crit_topo_generic_res",     VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        critd_init(&crit_topo_cfg_wr,          "crit_topo_cfg_wr",          VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        critd_init(&crit_topo_callback_regs,   "crit_topo_callback_regs",   VTSS_MODULE_ID_TOPO, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        
        TOPO_CALLBACK_REGS_CRIT_EXIT();

        
        for (isid = 1; isid < VTSS_ISID_END; isid++) {
            periodic_msg_slave_usid[isid].msg_type = TOPO_MSG_TYPE_SLAVE_USID_SET;
        }

        
        sprout_init.callback.log_msg                    = topo_sprout_log_msg;
        sprout_init.callback.cfg_save                   = topo_sprout_cfg_save;
        sprout_init.callback.state_change               = topo_sprout_state_change;
        sprout_init.callback.tx_vstax2_pkt              = topo_sprout_tx_vstax2_pkt;
        sprout_init.callback.secs_since_boot            = topo_sprout_secs_since_boot;
        sprout_init.callback.thread_set_priority_normal = topo_thread_set_priority_normal;

#if defined(VTSS_SPROUT_FW_VER_CHK)
        sprout_init.callback.fw_ver_chk = topo_sprout_fw_ver_chk;
#endif

        vtss_sprout_init(&sprout_init);

        
        for (isid = 1; isid < VTSS_ISID_END; isid++) {
            VTSS_MTIMER_START(&topo_stack_conf[isid].timer, 1);
        }

        cyg_flag_init(&topo_stack_conf_flags);

        
        
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          topo_thread,
                          0,
                          "Stack Topology",
                          topo_thread_stack,
                          sizeof(topo_thread_stack),
                          &topo_thread_handle,
                          &topo_thread_block);
    }
    break;

    case INIT_CMD_START:
        T_D("enter, cmd=START");

        if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
            

            
            topo_cfg = CFG_DEFAULT;

            isid = VTSS_ISID_START;
            if (conf_mgmt_mac_addr_get(my_mac, 0) == 0 && (conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOPO, &conf_size)) != NULL && *conf == CFG_DEFAULT.ver && conf_size == CFG_G_SIZE) {
                for (i = VTSS_ISID_START; i < VTSS_ISID_END; i++) {
                    uint offset = CFG_G_OFFSET_ISID_TBL + i * CFG_G_SIZE_ISID_TBL_ENTRY;

                    memcpy(topo_cfg.isid_tbl[i].mac_addr, conf + offset, 6);
                    topo_cfg.isid_tbl[i].usid = conf[offset + 6];
                    topo_cfg.usid_tbl[topo_cfg.isid_tbl[i].usid] = i;

                    if (memcmp(topo_cfg.isid_tbl[i].mac_addr, mac_addr_null, 6) != 0) {
                        topo_cfg.isid_tbl[i].assigned = 1;
                    }

                    if (memcmp(my_mac, topo_cfg.isid_tbl[i].mac_addr, 6) == 0) {
                        
                        
                        isid = i;
                    }
                }
            }
            msg_topo_event(MSG_TOPO_EVENT_MASTER_UP, isid);
            msg_topo_event(MSG_TOPO_EVENT_SWITCH_ADD, isid);
        } else {
            
            cyg_thread_resume(topo_thread_handle);
#if defined(VTSS_SPROUT_FW_VER_CHK)
            if (strlen(misc_software_version_txt()) > VTSS_SPROUT_FW_VER_LEN) {
#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("version_string too long for SPROUT: Length=%d, Max=%d. Firmware interoperability check will be incomplete.", strlen(misc_software_version_txt()), VTSS_SPROUT_FW_VER_LEN);
#endif
                T_W("version_string too long for SPROUT: Length=%d, Max=%d. Firmware interoperability check will be incomplete.", strlen(misc_software_version_txt()), VTSS_SPROUT_FW_VER_LEN);
            }
#endif
        }
        break;

    case INIT_CMD_CONF_DEF: {
        BOOL activate = 0;
        int  i;

        if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
            
            break;
        }

        T_D("enter, cmd=CONF_DEF, isid: %u", isid);

        TOPO_CFG_WR_CRIT_ENTER();

        if (isid == VTSS_ISID_LOCAL ||
            (me_mst && me_mst_isid == isid)) {
            
            
            topo_cfg_t cfg_new;
            cfg_new = CFG_DEFAULT;
            cfg_new.deci_secs_mst_time_ignore = topo_cfg.deci_secs_mst_time_ignore;
            cfg_new.mst_elect_prio            = topo_cfg.mst_elect_prio;
            cfg_new.uid_pref[0][0]            = topo_cfg.uid_pref[0][0];
            cfg_new.uid_pref[0][1]            = topo_cfg.uid_pref[0][1];
            cfg_new.uid_pref[1][0]            = topo_cfg.uid_pref[1][0];
            cfg_new.uid_pref[1][1]            = topo_cfg.uid_pref[1][1];
            memcpy(cfg_new.isid_tbl, topo_cfg.isid_tbl, sizeof(cfg_new.isid_tbl));
            memcpy(cfg_new.usid_tbl, topo_cfg.usid_tbl, sizeof(cfg_new.usid_tbl));
            if (data->flags & INIT_CMD_PARM2_FLAGS_ME_PRIO) {
                
                cfg_new.mst_elect_prio = CFG_DEFAULT.mst_elect_prio;
            }

            topo_cfg = cfg_new;
            rc = cfg_wr(1, 0, 0);
            activate = 1;
        } else if (isid == VTSS_ISID_GLOBAL) {
            if (data->flags & INIT_CMD_PARM2_FLAGS_SID) {
                if (me_mst) {
                    
                    for (i = 0; i < TOPO_SID_TBL_SIZE; i++) {
                        topo_cfg.isid_tbl[i].usid = i;
                        topo_cfg.usid_tbl[i] = i;
                    }

                    cfg_wr(0, 0, TOPO_PARM_ISID_TBL);
                }
            }
        } else {
            
            if (me_mst) {
                if (data->flags & INIT_CMD_PARM2_FLAGS_ME_PRIO) {
                    TOPO_CFG_WR_CRIT_EXIT();
                    (void) topo_parm_set(isid, TOPO_PARM_MST_ELECT_PRIO,
                                         CFG_DEFAULT.mst_elect_prio);
                    TOPO_CFG_WR_CRIT_ENTER();
                }
            }
        }

        TOPO_CFG_WR_CRIT_EXIT();

        if (activate) {
            rc = cfg_activate(0);
        }
    }
    break;

    case INIT_CMD_MASTER_UP:
    case INIT_CMD_MASTER_DOWN:
        break;

    case INIT_CMD_SWITCH_ADD:
        if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
            
            break;
        }

        if (me_mst) {
            if (me_mst_isid != isid) {
                TOPO_CFG_WR_CRIT_ENTER();
                topo_led_usid_set(isid, topo_cfg.isid_tbl[isid].usid);
                TOPO_CFG_WR_CRIT_EXIT();
            }
        }
        break;

    case INIT_CMD_SWITCH_DEL:
        
        break;

    default:
        T_W("topo_init called with unknown cmd=%d", data->cmd);
        break;
    }

    RETURN_RC(rc);
} 

#if defined(VTSS_SPROUT_V1)

vtss_rc topo_glag_mbr_cnt_set(
    const topo_chip_idx_t chip_idx,
    const vtss_glag_no_t  glag_no,
    const uint            glag_mbr_cnt)
{
    vtss_rc rc = VTSS_OK;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, chip_idx=%d, glag_no=%u, glag_mbr_cnt=%d",
        chip_idx, glag_no, glag_mbr_cnt);

    rc = vtss_sprout_glag_mbr_cnt_set(glag_no, glag_mbr_cnt);

    RETURN_RC(rc);
} 
#endif



vtss_rc topo_have_mirror_port_set(
    const topo_chip_idx_t chip_idx,
    const BOOL            have_mirror_port)
{
    vtss_rc rc = VTSS_OK;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, chip_idx=%d, have_mirror_port=%d",
        chip_idx, have_mirror_port);

    rc = vtss_sprout_have_mirror_port_set(chip_idx, have_mirror_port);

    RETURN_RC(rc);
} 



vtss_rc topo_stack_port_adm_state_set(
    const vtss_port_no_t port_no,
    const BOOL           adm_up)
{
    vtss_rc rc = VTSS_OK;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, port_no=%u, adm_up=%d",
        port_no, adm_up);

    rc = vtss_sprout_stack_port_adm_state_set(port_no, adm_up);

    RETURN_RC(rc);
} 



vtss_rc topo_ipv4_addr_set(
    
    const vtss_ipv4_t ipv4_addr)
{
    vtss_rc rc = VTSS_OK;
    char buf[20];

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    misc_ipv4_txt(ipv4_addr, buf);
    T_D("enter, ipv4_addr=%s", buf);

    rc = vtss_sprout_ipv4_addr_set(ipv4_addr);

    RETURN_RC(rc);
} 




vtss_rc topo_sit_get(
    topo_sit_t *const sit_p)
{
    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, sit_p=0x%p", sit_p);

    vtss_sprout_sit_get(sit_p);

    
    
    sit_p->mst_change_time = msg_abstime_get(VTSS_ISID_LOCAL, sit_p->mst_change_time);

    TOPO_CFG_RD_CRIT_ENTER();
    
    sit_isid_set(sit_p);
    TOPO_CFG_RD_CRIT_EXIT();

    return VTSS_OK;
} 



vtss_rc topo_state_change_callback_register(
    const topo_state_change_callback_t callback,
    const vtss_module_id_t             module_id)
{
    int i;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, module_id=%d, state_change_callback_cnt=%d",
        module_id, state_change_callback_cnt);

    TOPO_ASSERTR(callback != NULL,
                 "callback=NULL. module_id=%d", module_id);

    TOPO_ASSERTR(state_change_callback_cnt < MAX_STATE_CHANGE_CALLBACK,
                 "No more room in registration table");

    TOPO_CALLBACK_REGS_CRIT_ENTER();

    
    for (i = 0; i < state_change_callback_cnt; i++) {
        if (state_change_callback_regs[i].module_id == module_id) {
            T_E("module_id=%d already registrered", module_id);
            break;
        }
    }

    state_change_callback_regs[state_change_callback_cnt].callback  = callback;
    state_change_callback_regs[state_change_callback_cnt].module_id = module_id;
    state_change_callback_cnt++;

    TOPO_CALLBACK_REGS_CRIT_EXIT();

    RETURN_RC(VTSS_OK);
} 



vtss_rc topo_upsid_change_callback_register(
    const topo_upsid_change_callback_t callback,
    const vtss_module_id_t             module_id)
{
    int i;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("enter, module_id=%d, upsid_change_callback_cnt=%d",
        module_id, upsid_change_callback_cnt);

    TOPO_ASSERTR(callback != NULL,
                 "callback=NULL. module_id=%d", module_id);

    TOPO_ASSERTR(upsid_change_callback_cnt < MAX_UPSID_CHANGE_CALLBACK,
                 "No more room in registration table");

    TOPO_CALLBACK_REGS_CRIT_ENTER();

    
    for (i = 0; i < upsid_change_callback_cnt; i++) {
        if (upsid_change_callback_regs[i].module_id == module_id) {
            T_E("module_id=%d already registrered", module_id);
            break;
        }
    }

    upsid_change_callback_regs[upsid_change_callback_cnt].callback  = callback;
    upsid_change_callback_regs[upsid_change_callback_cnt].module_id = module_id;
    upsid_change_callback_cnt++;

    TOPO_CALLBACK_REGS_CRIT_EXIT();

    RETURN_RC(VTSS_OK);
} 


int topo_parm_get(const topo_parm_t parm)
{
    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return 0;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");


    switch (parm) {
    case TOPO_PARM_MST_ELECT_PRIO:
        return topo_cfg.mst_elect_prio;

    case TOPO_PARM_MST_TIME_IGNORE:
        return (topo_cfg.deci_secs_mst_time_ignore > 0);

    case TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV:
        return topo_cfg.sprout_update_interval_slv;

    case TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST:
        return topo_cfg.sprout_update_interval_mst;

    case TOPO_PARM_SPROUT_UPDATE_AGE_TIME:
        return topo_cfg.sprout_update_age_time;

    case TOPO_PARM_SPROUT_UPDATE_LIMIT:
        return topo_cfg.sprout_update_limit;

#if defined(VTSS_SPROUT_V1)
    case TOPO_PARM_FAST_MAC_AGE_TIME:
        return topo_cfg.fast_mac_age_time;

    case TOPO_PARM_FAST_MAC_AGE_COUNT:
        return topo_cfg.fast_mac_age_count;
#endif

#if defined(VTSS_SPROUT_FW_VER_CHK)
    case TOPO_PARM_FW_VER_MODE:
        return fw_ver_mode;

    case TOPO_PARM_CMEF_MODE:
        
        return -1;
#endif

    case TOPO_PARM_UID_0_0:
        return topo_cfg.uid_pref[0][0];

    case TOPO_PARM_UID_0_1:
        return topo_cfg.uid_pref[0][1];

    case TOPO_PARM_UID_1_0:
        return topo_cfg.uid_pref[1][0];

    case TOPO_PARM_UID_1_1:
        return topo_cfg.uid_pref[1][1];

    default:
        T_E("Unknown parm: %d", parm);
    }

    return 0;
} 


topo_error_t topo_parm_set(
    vtss_isid_t       isid,
    const topo_parm_t parm,
    const int         val)
{
    int i;
    topo_error_t rc = VTSS_OK;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    T_D("isid=%d, parm=%d, val=%d", isid, parm, val);

    if (me_mst && isid == me_mst_isid) {
        isid = VTSS_ISID_LOCAL;
    }
    if (!me_mst) {
        
        isid = VTSS_ISID_LOCAL;
    }

    TOPO_CFG_WR_CRIT_ENTER();

    if (isid == VTSS_ISID_GLOBAL &&
        parm != TOPO_PARM_FW_VER_MODE &&
        parm != TOPO_PARM_CMEF_MODE) {
        
        rc = TOPO_ERROR_NO_SWITCH_SELECTED;
    } else if (isid != VTSS_ISID_LOCAL) {
        
        if (isid != VTSS_ISID_GLOBAL &&
            !VTSS_ISID_LEGAL(isid)) {
            rc = TOPO_ERROR_SWITCH_NOT_PRESENT;
        } else if (parm != TOPO_PARM_MST_ELECT_PRIO &&
                   parm != TOPO_PARM_FW_VER_MODE &&
                   parm != TOPO_PARM_CMEF_MODE) {
            
            rc = TOPO_ERROR_REMOTE_PARM_NOT_SUPPORTED;
        } else if (!me_mst) {
            rc = TOPO_ERROR_REMOTE_PARM_ONLY_FROM_MST;
        }
        if (rc == VTSS_OK && isid != VTSS_ISID_GLOBAL) {
            
            if (!topo_cfg.isid_tbl[isid].assigned) {
                rc = TOPO_ERROR_SID_NOT_ASSIGNED;
            } else {
                
                BOOL found = 0;
                for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
                    if (sit_copy.si[i].vld) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    rc = TOPO_ERROR_SWITCH_NOT_PRESENT;
                }
            }
        }
    }

    if (rc == VTSS_OK) {
        switch (parm) {
        case TOPO_PARM_MST_ELECT_PRIO:
            if (!TOPO_MST_ELECT_PRIO_LEGAL(val)) {
                T_E("Illegal value for mst_elect_prio: %d", val);
            } else {
                if (isid == 0) {
                    topo_cfg.mst_elect_prio = val;
                } else {
                    topo_msg_mst_prio_elect_set_t *topo_msg_p;

                    if ((topo_msg_p = VTSS_MALLOC(sizeof(*topo_msg_p)))) {
                        topo_msg_p->msg_type = TOPO_MSG_TYPE_MST_ELECT_PRIO_SET;
                        topo_msg_p->mst_elect_prio = val;

                        TOPO_CFG_RD_CRIT_EXIT_TMP(
                            msg_tx(VTSS_MODULE_ID_TOPO, isid, (uchar *)topo_msg_p,
                                   sizeof(*topo_msg_p)));
                    } else {
                        T_E("VTSS_MALLOC() failed, size=%zu", sizeof(*topo_msg_p));
                    }

                }
            }
            break;

        case TOPO_PARM_MST_TIME_IGNORE:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val != 0 && val != 1) {
                T_E("Illegal value for mst_time_ignore: %d", val);
            } else {
                topo_cfg.deci_secs_mst_time_ignore = (val ? VTSS_SPROUT_MST_TIME_IGNORE_PERIOD * 10 : 0);
            }
            break;

        case TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_SPROUT_UPDATE_INTERVAL_MIN) {
                T_E("Illegal value for sprout_update_interval: %d", val);
            } else {
                topo_cfg.sprout_update_interval_slv = val;
            }
            break;

        case TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_SPROUT_UPDATE_INTERVAL_MIN) {
                T_E("Illegal value for sprout_update_interval: %d", val);
            } else {
                topo_cfg.sprout_update_interval_mst = val;
            }
            break;

        case TOPO_PARM_SPROUT_UPDATE_AGE_TIME:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_SPROUT_UPDATE_AGE_TIME_MIN) {
                T_E("Illegal value for sprout_update_age_time: %d", val);
            } else {
                topo_cfg.sprout_update_age_time = val;
            }
            break;

        case TOPO_PARM_SPROUT_UPDATE_LIMIT:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_SPROUT_UPDATE_LIMIT_MIN) {
                T_E("Illegal value for sprout_update_limit: %d", val);
            } else {
                topo_cfg.sprout_update_limit = val;
            }
            break;

#if defined(VTSS_SPROUT_V1)
        case TOPO_PARM_FAST_MAC_AGE_TIME:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_FAST_MAC_AGE_TIME_MIN) {
                T_E("Illegal value for sprout_fast_mac_age_time: %d", val);
            } else {
                topo_cfg.fast_mac_age_time = val;
            }
            break;

        case TOPO_PARM_FAST_MAC_AGE_COUNT:
            TOPO_ASSERT(isid == 0, "isid=%d", isid);
            if (val < TOPO_PARM_FAST_MAC_AGE_COUNT_MIN) {
                T_E("Illegal value for sprout_fast_mac_age_count: %d", val);
            } else {
                topo_cfg.fast_mac_age_count = val;
            }
            break;
#endif
#if defined(VTSS_SPROUT_FW_VER_CHK)
        case TOPO_PARM_FW_VER_MODE:
            if (!TOPO_FW_VER_MODE_LEGAL(val)) {
                T_E("Illegal value for fw_ver_mode: %d", val);
            } else {
                if (isid == VTSS_ISID_LOCAL) {
                    
                    fw_ver_mode = val;
                    vtss_sprout_fw_ver_mode_set(val);
                } else {
                    
                    vtss_isid_t i;
                    for (i = VTSS_ISID_START; i < VTSS_ISID_END; i++) {
                        if (!msg_switch_exists(i)) {
                            continue;
                        }

                        if (i == isid || isid == VTSS_ISID_GLOBAL) {
                            
                            topo_msg_config_t *topo_msg_p;

                            if ((topo_msg_p = VTSS_MALLOC(sizeof(*topo_msg_p)))) {
                                topo_msg_p->msg_type = TOPO_MSG_TYPE_FW_VER_MODE_SET;
                                topo_msg_p->msg_val.fw_ver_mode = val;

                                TOPO_CFG_RD_CRIT_EXIT_TMP(
                                    msg_tx(VTSS_MODULE_ID_TOPO, i, (uchar *)topo_msg_p,
                                           sizeof(*topo_msg_p)));
                            } else {
                                T_E("VTSS_MALLOC() failed, size=%zu", sizeof(*topo_msg_p));
                            }
                        }
                    }
                }
            }
            break;
#endif

#if defined(VTSS_SPROUT_V2)
        case TOPO_PARM_CMEF_MODE:
            if (isid == VTSS_ISID_LOCAL) {
                
                cmef_mode_set(val);
            } else {
                
                vtss_isid_t i;
                for (i = VTSS_ISID_START; i < VTSS_ISID_END; i++) {
                    if (!msg_switch_exists(i)) {
                        continue;
                    }

                    if (i == isid || isid == VTSS_ISID_GLOBAL) {
                        
                        topo_msg_config_t *topo_msg_p;

                        if ((topo_msg_p = VTSS_MALLOC(sizeof(*topo_msg_p)))) {
                            topo_msg_p->msg_type = TOPO_MSG_TYPE_CMEF_MODE_SET;
                            topo_msg_p->msg_val.cmef_ena = (val == 0) ? 0 : 1;

                            TOPO_CFG_RD_CRIT_EXIT_TMP(
                                msg_tx(VTSS_MODULE_ID_TOPO, i, (uchar *)topo_msg_p,
                                       sizeof(*topo_msg_p)));
                        } else {
                            T_E("VTSS_MALLOC() failed, size=%zu", sizeof(*topo_msg_p));
                        }
                    }
                }
            }
            break;
#endif
        default:
            T_E("Parameter not supported by topo_parm_set: %d", parm);
        }

        cfg_wr(0, 0, parm);
    }

    TOPO_CFG_WR_CRIT_EXIT();

    if (rc == VTSS_OK) {
        cfg_activate(0);
    }

    return rc;
} 


vtss_port_no_t topo_isid2port(const vtss_isid_t isid)
{
    uint i;
    vtss_port_no_t port_no = 0;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return port_no;
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    TOPO_ASSERT(1 <= isid && isid < VTSS_ISID_END, "Invalid isid: %d", isid);

    TOPO_CFG_RD_CRIT_ENTER();

    
    if (me_mst) {
        for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
            if (sit_copy.si[i].vld &&
                sit_copy.si[i].id == isid) {
                TOPO_ASSERT(topo_cfg.isid_tbl[sit_copy.si[i].id].assigned, "isid=%d", isid);

                
                port_no = sit_copy.si[i].chip[0].shortest_path;

                TOPO_ASSERT(port_no == PORT_NO_STACK_0 ||
                            port_no == PORT_NO_STACK_1 ||
                            port_no == 0, 
                            "port_no=%u", port_no);
                break;
            }
        }
    }

    TOPO_CFG_RD_CRIT_EXIT();

    return port_no;
} 


vtss_port_no_t topo_mac2port(const mac_addr_t mac_addr)
{
    uint        i;
    static uint i_latest_match = 0;
    vtss_port_no_t port_no = 0;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return 0;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    TOPO_CFG_RD_CRIT_ENTER();

    if (sit_copy.si[i_latest_match].vld &&
        (memcmp(sit_copy.si[i_latest_match].switch_addr.addr, mac_addr, sizeof(mac_addr_t)) == 0)) {
        
        
        

        
        port_no = sit_copy.si[i_latest_match].chip[0].shortest_path;
    } else {
        
        for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
            if (sit_copy.si[i].vld &&
                (memcmp(sit_copy.si[i].switch_addr.addr, mac_addr, sizeof(mac_addr_t)) == 0)) {
                
                port_no = sit_copy.si[i].chip[0].shortest_path;

                i_latest_match = i;
                break;
            }
        }
    }

    TOPO_ASSERT(port_no == PORT_NO_STACK_0 ||
                port_no == PORT_NO_STACK_1 ||
                port_no == 0,
                "port_no=%u", port_no);

    TOPO_CFG_RD_CRIT_EXIT();

    return port_no;
} 





topo_error_t topo_isid_delete(
    const vtss_isid_t isid)
{
    BOOL active_switch = 0;
    int i = 0;
    topo_error_t rc = VTSS_OK;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_RC_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    TOPO_CFG_WR_CRIT_ENTER();

    if (!me_mst) {
        rc = TOPO_ERROR_NOT_MASTER;
        goto end;
    } else if (isid == me_mst_isid) {
        rc = TOPO_ERROR_MASTER_SID;
        goto end;
    } else if (!topo_cfg.isid_tbl[isid].assigned) {
        rc = TOPO_ERROR_SID_NOT_ASSIGNED;
        goto end;
    } else if (isid_delete_pending[isid]) {
        
        rc = VTSS_OK;
        goto end;
    } else {
        for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
            if (sit_copy.si[i].vld && sit_copy.si[i].id == isid) {
                active_switch = 1;
                break;
            }
        }
    }

    if (active_switch) {
        
        if (topo_led_usid_set(isid, 0)) {
            TOPO_CFG_RD_CRIT_EXIT();
            T_D("topo_isid_delete: SWITCH_DEL, isid=%d ...", isid);
            msg_topo_event(MSG_TOPO_EVENT_SWITCH_DEL, isid);
            T_D("SWITCH_DEL, isid=%d: Done", isid);
            TOPO_CFG_RD_CRIT_ENTER();
        } else {
            
            
            isid_delete_pending[isid] = 1;
        }
    }

    if (!isid_delete_pending[isid]) {
        isid_free(isid);
    }

end:
    TOPO_CFG_WR_CRIT_EXIT();

    return rc;
} 


topo_error_t topo_isid_assign(
    const vtss_isid_t isid,
    const mac_addr_t  mac_addr)
{
    uint i;
    topo_error_t rc = VTSS_OK;
    BOOL found = 0;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_RC_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    TOPO_CFG_WR_CRIT_ENTER();


    if (!me_mst) {
        rc = TOPO_ERROR_NOT_MASTER;
        goto end;
    } else if (isid_delete_pending[isid]) {
        rc = TOPO_ERROR_ISID_DELETE_PENDING;
        goto end;
    } else if (topo_cfg.isid_tbl[isid].assigned) {
        
        
        for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
            if (sit_copy.si[i].vld &&
                sit_copy.si[i].id == isid) {
                rc = TOPO_ERROR_SID_IN_USE;
                goto end;
            }
        }
    }

    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_copy.si[i].vld &&
            memcmp(sit_copy.si[i].switch_addr.addr, mac_addr, 6) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        rc = TOPO_ERROR_SWITCH_NOT_PRESENT;
    } else {
        if (sit_copy.si[i].id != 0) {
            rc = TOPO_ERROR_SWITCH_HAS_SID;
        } else {
            
            memcpy(topo_cfg.isid_tbl[isid].mac_addr, mac_addr, 6);
            sit_copy.si[i].id = isid;
            topo_cfg.isid_tbl[isid].assigned = 1;
            cfg_wr(0, 0, TOPO_PARM_ISID_TBL); 

            TOPO_CFG_RD_CRIT_EXIT();
            T_D("SWITCH_ADD, isid=%d ...", isid);
            msg_topo_event(MSG_TOPO_EVENT_SWITCH_ADD, isid);
            T_D("SWITCH_ADD, isid=%d: Done", isid);
            TOPO_CFG_RD_CRIT_ENTER();
        }
    }

end:
    TOPO_CFG_WR_CRIT_EXIT();

    return rc;
} 


topo_error_t topo_usid_swap(const vtss_usid_t usida, const vtss_usid_t usidb)
{
    vtss_isid_t  isid;
    topo_error_t rc = VTSS_OK;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_RC_ERROR;
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    BOOL usida_found = 0;
    BOOL usidb_found = 0;

    TOPO_CFG_WR_CRIT_ENTER();

    if (!me_mst) {
        rc = TOPO_ERROR_NOT_MASTER;
    } else {
        for (isid = 1; isid < VTSS_ISID_END; isid++) {
            if (topo_cfg.isid_tbl[isid].usid == usida) {
                TOPO_ASSERT(!usida_found, "!");
                usida_found = 1;
                topo_cfg.isid_tbl[isid].usid = usidb;
            } else if (topo_cfg.isid_tbl[isid].usid == usidb) {
                TOPO_ASSERT(!usidb_found, "!");
                usidb_found = 1;
                topo_cfg.isid_tbl[isid].usid = usida;
            }
        }
        TOPO_ASSERT(usida_found && usidb_found, "usida=%d, usidb=%d, usida_found=%d, usidb_found=%d", usida, usidb, usida_found, usidb_found);

        isid = topo_cfg.usid_tbl[usida];
        topo_cfg.usid_tbl[usida] = topo_cfg.usid_tbl[usidb];
        topo_cfg.usid_tbl[usidb] = isid;

        cfg_wr(0, 0, TOPO_PARM_ISID_TBL);

        topo_led_usid_set(topo_cfg.usid_tbl[usida], usida);
        topo_led_usid_set(topo_cfg.usid_tbl[usidb], usidb);
    }

    TOPO_CFG_WR_CRIT_EXIT();

    return rc;
} 





vtss_isid_t topo_usid2isid(const vtss_usid_t usid)
{
    vtss_isid_t isid = 0;

    if (vtss_switch_unmgd()) {
        return usid;    
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    if (usid == 0) {
        T_E("Invalid usid: %d", usid);
        return 0;
    }

    TOPO_ASSERT(usid < VTSS_USID_END, "Invalid usid: %d", usid);

    TOPO_CFG_RD_CRIT_ENTER();
    isid = topo_cfg.usid_tbl[usid];
    TOPO_CFG_RD_CRIT_EXIT();

    return isid;
} 

vtss_usid_t topo_isid2usid(const vtss_isid_t isid)
{
    vtss_usid_t usid = 0;

    if (vtss_switch_unmgd()) {
        return isid;    
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    TOPO_ASSERT(1 <= isid && isid < VTSS_ISID_END, "Invalid isid: %d", isid);

    TOPO_CFG_RD_CRIT_ENTER();
    usid = topo_cfg.isid_tbl[isid].usid;
    TOPO_CFG_RD_CRIT_EXIT();

    return usid;
} 

vtss_rc topo_isid2mac(
    const vtss_isid_t isid,
    mac_addr_t mac_addr)
{
    vtss_rc    rc = VTSS_OK;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        (void)conf_mgmt_mac_addr_get(mac_addr, 0);
        return VTSS_OK;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    TOPO_ASSERT(VTSS_ISID_LEGAL(isid), "Invalid isid: %d", isid);

    TOPO_CFG_RD_CRIT_ENTER();

    if (topo_cfg.isid_tbl[isid].assigned) {
        memcpy(mac_addr, topo_cfg.isid_tbl[isid].mac_addr, sizeof(mac_addr_t));
    } else {
        rc = TOPO_ERROR_GEN;
    }

    TOPO_CFG_RD_CRIT_EXIT();

    return rc;
} 



vtss_isid_t topo_mac2isid(const mac_addr_t mac_addr)
{
    vtss_isid_t isid = 0;

    if (vtss_switch_unmgd() || topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return isid;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    isid = topo_mac2isid_int(1, mac_addr);

    return isid;
} 



vtss_vstax_upsid_t topo_isid_port2upsid(vtss_isid_t isid, const vtss_port_no_t port_no)
{
    int                i;
    vtss_vstax_upsid_t upsid = VTSS_VSTAX_UPSID_UNDEF;
    topo_chip_idx_t    chip_idx;
    int                ups_idx;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        vtss_vstax_conf_t vstax_conf;
        if (isid != VTSS_ISID_LOCAL && !msg_switch_is_local(isid)) {
            T_E("isid must be local isid in standalone mode (%u)", isid);
        } else if (vtss_vstax_conf_get(NULL, &vstax_conf) != VTSS_RC_OK) {
            T_E("Unable to obtain VStaX conf from API");
        } else if (port_no == VTSS_PORT_NO_NONE) {
            upsid = vstax_conf.upsid_0; 
        } else if (port_no < port_isid_port_count(isid)) {
            upsid = port_custom_table[port_no].map.chip_no == 0 ? vstax_conf.upsid_0 : vstax_conf.upsid_1;
        } else {
            T_E("Illegal port number or isid (%u:%u)", isid, port_no);
        }
        return upsid;
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    
    
    
    if (!me_mst) {
        T_W("!me_mst");
        return VTSS_VSTAX_UPSID_UNDEF;
    }

    TOPO_CFG_RD_CRIT_ENTER();
    TOPO_ASSERT(sit_copy_vld, "!sit_copy_vld");

    if (isid == VTSS_ISID_LOCAL) {
        isid = me_mst_isid;
    }
    TOPO_ASSERT(VTSS_ISID_LEGAL(isid), "Invalid isid: %d", isid);

    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_copy.si[i].vld && sit_copy.si[i].id == isid) {
            

            if (port_no == VTSS_PORT_NO_NONE) {
                
                upsid = sit_copy.si[i].chip[0].upsid[0];
            } else {
                
                for (chip_idx = 0; chip_idx < sit_copy.si[i].chip_cnt; chip_idx++) {
                    for (ups_idx = 0; ups_idx < 2; ups_idx++) {
                        if (sit_copy.si[i].chip[chip_idx].upsid[ups_idx] != VTSS_VSTAX_UPSID_UNDEF &&
                            sit_copy.si[i].chip[chip_idx].ups_port_mask[ups_idx] & ((u64)1 << (port_no))) {
                            TOPO_ASSERT(upsid == VTSS_VSTAX_UPSID_UNDEF, "Multiple UPSes in sit_copy with isid=%u, port_no=%u?! (i=%d)", isid, port_no, i);
                            upsid = sit_copy.si[i].chip[chip_idx].upsid[ups_idx];
                        }
                    }
                }
            }
            break;
        }
    }

    TOPO_CFG_RD_CRIT_EXIT();

    if (upsid == VTSS_VSTAX_UPSID_UNDEF) {
        T_W("UPSID undefined for isid=%u, port_no=%u", isid, port_no);
    }

    return upsid;
} 



vtss_isid_t topo_upsid2isid(const vtss_vstax_upsid_t upsid)
{
    int         i;
    vtss_isid_t isid = VTSS_ISID_UNKNOWN;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return isid;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    TOPO_ASSERT(me_mst, "!me_mst");
    TOPO_ASSERT(VTSS_VSTAX_UPSID_LEGAL(upsid), "upsid=%d", upsid);

    TOPO_CFG_RD_CRIT_ENTER();
    TOPO_ASSERT(sit_copy_vld, "!sit_copy_vld");

    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_copy.si[i].vld &&
            (sit_copy.si[i].chip[0].upsid[0] == upsid ||
             sit_copy.si[i].chip[0].upsid[1] == upsid ||
             sit_copy.si[i].chip[1].upsid[0] == upsid ||
             sit_copy.si[i].chip[1].upsid[1] == upsid)) {
            isid = sit_copy.si[i].id;
            break;
        }
    }

    TOPO_CFG_RD_CRIT_EXIT();

    if (isid == VTSS_ISID_UNKNOWN) {
        T_W("ISID for UPSID=%d unknown", upsid);
    }

    return isid;
} 

#if defined(VTSS_FEATURE_VSTAX_V2)
vtss_rc topo_upsid_upspn2sid_port(const vtss_vstax_upsid_t upsid, const vtss_vstax_upspn_t upspn, topo_sid_port_t *sid_port)
{
    vtss_rc                      rc = VTSS_UNSPECIFIED_ERROR;
    vtss_sprout_sit_entry_t      *si;
    vtss_sprout_sit_entry_chip_t *ci;
    int                          i;
    vtss_chip_no_t               chip_no;
    vtss_port_no_t               port_no;
    u32                          port_count;
    port_isid_port_info_t        info;

    if (topo_switch_mode != TOPO_SWITCH_MODE_STACKABLE ||
        !me_mst ||
        !VTSS_VSTAX_UPSID_LEGAL(upsid) ||
        !sit_copy_vld) {
        return rc;
    }

    
    TOPO_CFG_RD_CRIT_ENTER();
    for (i = 0; rc != VTSS_OK && i < VTSS_SPROUT_SIT_SIZE; i++) {
        si = &sit_copy.si[i];
        for (chip_no = 0; si->vld && chip_no < 2; chip_no++) {
            ci = &si->chip[chip_no];
            if (ci->upsid[0] == upsid || ci->upsid[1] == upsid) {
                sid_port->isid = si->id;
                sid_port->usid = topo_cfg.isid_tbl[sid_port->isid].usid;
                rc = VTSS_OK;
                break;
            }
        }
    }
    TOPO_CFG_RD_CRIT_EXIT();

    if (rc == VTSS_OK) {
        
        rc = VTSS_UNSPECIFIED_ERROR;
        port_count = port_isid_port_count(sid_port->isid);
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (port_isid_port_info_get(sid_port->isid, port_no, &info) == VTSS_OK &&
                info.chip_port == upspn &&
                info.chip_no == chip_no) {
                sid_port->port_no = port_no;
                rc = VTSS_OK;
                break;
            }
        }
    }
    return rc;
}
#endif 

#if VTSS_SWITCH_MANAGED


static topo_sit_t topo_switch_list_get__sit; 
vtss_rc topo_switch_list_get(topo_switch_list_t *const tsl_p)
{
    vtss_rc rc = VTSS_OK;
    topo_sit_t *sit_p;
    vtss_isid_t isid;
    vtss_isid_t usid;
    uint        i;
    uint        ts_idx = 0;

    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN,   "Unknown switch mode");

    TOPO_ASSERT(tsl_p != NULL, "!");
    memset(tsl_p, 0, sizeof(topo_switch_list_t));

    
    sit_p = &topo_switch_list_get__sit;
    T_W_RC_NEG(topo_sit_get(sit_p));
    if (rc < 0) {
        return rc;
    }

    memcpy(tsl_p->mst_switch_mac_addr, sit_p->mst_switch_addr.addr, 6);
    tsl_p->mst_change_time = sit_p->mst_change_time;

    TOPO_CFG_RD_CRIT_ENTER();

    
    
    
    

    
    for (usid = 1; usid < VTSS_USID_END; usid++) {
        isid = topo_cfg.usid_tbl[usid];

        for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
            if (sit_p->si[i].vld &&
                sit_p->si[i].id == isid) {
                
                sit2tsl(sit_p->topology_type, &sit_p->si[i], &tsl_p->ts[ts_idx++]);
                break;
            }
        }
    }

    
    for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
        if (sit_p->si[i].vld &&
            sit_p->si[i].id == 0) {
            
            sit2tsl(sit_p->topology_type, &sit_p->si[i], &tsl_p->ts[ts_idx++]);
        }
    }

    
    if (me_mst) {
        for (usid = 1; usid < VTSS_USID_END; usid++) {
            isid = topo_cfg.usid_tbl[usid];

            if (topo_cfg.isid_tbl[isid].assigned) {
                BOOL found = 0;
                for (i = 0; i < VTSS_SPROUT_SIT_SIZE; i++) {
                    if (sit_p->si[i].vld &&
                        sit_p->si[i].id == isid) {
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    
                    tsl_p->ts[ts_idx].vld      = 1;
                    tsl_p->ts[ts_idx].isid     = isid;
                    tsl_p->ts[ts_idx].usid     = usid;
                    tsl_p->ts[ts_idx].chip_cnt = 1; 
                    memcpy(tsl_p->ts[ts_idx].mac_addr, topo_cfg.isid_tbl[isid].mac_addr, 6);
                    ts_idx++;
                }
            }
        }
    }

    TOPO_CFG_RD_CRIT_EXIT();

    return rc;
} 

const char *topo_stack_port_fwd_mode_to_str(
    const topo_stack_port_fwd_mode_t stack_port_fwd_mode)
{
    switch (stack_port_fwd_mode) {
    case TOPO_STACK_PORT_FWD_MODE_NONE:
        return "-";
    case TOPO_STACK_PORT_FWD_MODE_LOCAL:
        return "Local";
    case TOPO_STACK_PORT_FWD_MODE_ACTIVE:
        return "Active";
    case TOPO_STACK_PORT_FWD_MODE_BACKUP:
        return "Backup";
    default:
        T_E("?");
        break;
    }

    return "?";
} 



vtss_rc topo_switch_stat_get(
    const vtss_isid_t   isid,
    topo_switch_stat_t *const stat_p)
{
    if (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    TOPO_ASSERT(topo_switch_mode != TOPO_SWITCH_MODE_UNKNOWN, "Unknown switch mode");

    return topo_switch_stat_get__static(isid, stat_p, TRUE); 
} 

static vtss_rc topo_stack_conf_get_req(vtss_isid_t isid, stack_config_t *conf, BOOL *dirty)
{
    topo_msg_stack_conf_t  *conf_msg;
    cyg_flag_value_t       flag = (1 << isid);
    cyg_flag_t             *flags = &topo_stack_conf_flags;
    cyg_tick_count_t       time;
    topo_isid_stack_conf_t *isid_conf = &topo_stack_conf[isid];

    if (VTSS_MTIMER_TIMEOUT(&isid_conf->timer)) {
        
        if ((conf_msg = VTSS_MALLOC(sizeof(*conf_msg))) == NULL) {
            return TOPO_ALLOC_FAILED;
        }

        cyg_flag_maskbits(flags, ~flag);
        conf_msg->msg_type = TOPO_MSG_TYPE_STACK_CONF_GET_REQ;
        msg_tx(VTSS_MODULE_ID_TOPO, isid, (uchar *)conf_msg, sizeof(*conf_msg));
        time = (cyg_current_time() + VTSS_OS_MSEC2TICK(TOPO_STACK_CONF_REQ_TIMEOUT));
        if (!(cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time) & flag)) {
            return TOPO_ERROR_GEN;
        }
    }

    *conf = isid_conf->conf;
    *dirty = isid_conf->dirty;
    return VTSS_OK;
}

static vtss_rc topo_stack_conf_set_req(vtss_isid_t isid, stack_config_t *conf)
{
    topo_msg_stack_conf_t *msg;

    if ((msg = VTSS_MALLOC(sizeof(*msg))) == NULL) {
        return TOPO_ALLOC_FAILED;
    }

    msg->msg_type = TOPO_MSG_TYPE_STACK_CONF_SET_REQ;
    msg->conf = *conf;
    msg_tx(VTSS_MODULE_ID_TOPO, isid, (uchar *)msg, sizeof(*msg));

    return VTSS_OK;
}

static vtss_rc topo_isid_valid(vtss_isid_t isid)
{
    if (!VTSS_ISID_LEGAL(isid)) {
        return TOPO_ERROR_PARM;
    }

    if (!msg_switch_is_master()) {
        return TOPO_ERROR_NOT_MASTER;
    }

    if (!msg_switch_exists(isid)) {
        return TOPO_ERROR_SWITCH_NOT_PRESENT;
    }

    return VTSS_OK;
}

vtss_rc topo_stack_config_get(vtss_isid_t isid, stack_config_t *conf, BOOL *dirty)
{
    vtss_rc rc;

    if ((rc = topo_isid_valid(isid)) != VTSS_OK) {
        return rc;
    }

    return (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE ?
            topo_stack_conf_rd(conf, dirty) : topo_stack_conf_get_req(isid, conf, dirty));
}

vtss_rc topo_stack_config_set(vtss_isid_t isid, stack_config_t *conf)
{
    vtss_rc               rc;
    port_isid_port_info_t info;
    u32                   i, port_count;
    vtss_port_no_t        port_no;
    vtss_chip_no_t        chip_no = 0;

    if ((rc = topo_isid_valid(isid)) != VTSS_OK) {
        return rc;
    }

    port_count = port_isid_port_count(isid);
    for (i = 0; i < 2; i++) {
        port_no = (i == 0 ? conf->port_0 : conf->port_1);
        T_D("isid: %u, port_no: %u", isid, port_no);
        if (port_no >= port_count) {
            T_W("isid: %u, port_no: %u, port_count: %u", isid, port_no, port_count);
            return TOPO_ERROR_CONFIG_ILLEGAL_STACK_PORT;
        }
        if ((rc = port_isid_port_info_get(isid, port_no, &info)) != VTSS_OK) {
            return rc;
        }
        if ((info.cap & PORT_CAP_STACKING) == 0) {
            T_W("isid: %u, port_no: %u, cap: 0x%08x", isid, port_no, info.cap);
            return TOPO_ERROR_CONFIG_ILLEGAL_STACK_PORT;
        }
        if (i == 0) {
            
            chip_no = info.chip_no;
        } else {
            if (port_count > 32 && chip_no == info.chip_no) {
                
                T_W("isid: %u, both on chip_no: %u", isid, chip_no);
                return TOPO_ERROR_CONFIG_ILLEGAL_STACK_PORT;
            }
            if (chip_no == 1) {
                
                conf->port_1 = conf->port_0;
                conf->port_0 = port_no;
            }
        }
    }
    return (topo_switch_mode == TOPO_SWITCH_MODE_STANDALONE ?
            topo_stack_conf_wr(conf) : topo_stack_conf_set_req(isid, conf));
}
#endif 




#if VTSS_SWITCH_MANAGED

void topo_dbg_isid_tbl_print(
    const topo_dbg_printf_t  dbg_printf)
{
    vtss_isid_t isid;

    
    
    

    dbg_printf("In use  ISID  MAC Addr           USID  Port\n");
    dbg_printf("------  ----  -----------------  ----  ----\n");
    for (isid = 1; isid < VTSS_ISID_END; isid++) {
        dbg_printf("%6d  %4d  %s  %4d  %4u\n",
                   topo_cfg.isid_tbl[isid].assigned,
                   isid,
                   mac_addr_to_str(topo_cfg.isid_tbl[isid].mac_addr),
                   topo_cfg.isid_tbl[isid].usid,
                   topo_isid2port(isid));

        if (topo_usid2isid(topo_cfg.isid_tbl[isid].usid) != isid) {
            T_E("topo_usid2isid(%d) = %d  != %d",
                topo_cfg.isid_tbl[isid].usid,
                topo_usid2isid(topo_cfg.isid_tbl[isid].usid),
                isid);
        }
    }
    dbg_printf("\n");
    dbg_printf("me_mst=%d, me_mst_isid=%d\n", me_mst, me_mst_isid);
    dbg_printf("topo_cfg.deci_secs_mst_time_ignore=%d\n", topo_cfg.deci_secs_mst_time_ignore);
} 

void topo_parm_set_default(void)
{
    TOPO_CFG_WR_CRIT_ENTER();
    topo_cfg = CFG_DEFAULT;
    cfg_wr(1, 1, 0);
    TOPO_CFG_WR_CRIT_EXIT();

    cfg_activate(0);

} 
#endif


void topo_dbg_sprout(
    const topo_dbg_printf_t  dbg_printf,
    const uint               parms_cnt,
    const ulong *const       parms)
{
    vtss_sprout_dbg(dbg_printf, parms_cnt, parms);
} 


void topo_dbg_test(
    const topo_dbg_printf_t  dbg_printf,
    const int arg1,
    const int arg2,
    const int arg3)
{
    switch (arg1) {
    case 0:
        dbg_printf("Local UPSID: %d\n", topo_upsid2isid(arg2));
        break;
    case 1:
        dbg_printf("me_mst_isid=%d\n", me_mst_isid);
        break;
    }
} 








