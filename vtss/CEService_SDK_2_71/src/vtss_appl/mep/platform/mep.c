
/*

 Vitesse MEP software.

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
#include "main.h"
#include "conf_api.h"
#include "critd_api.h"
#include "port_api.h"
#include "../base/vtss_mep_api.h"
#include "../base/vtss_mep_supp_api.h"
#include "mep_api.h"
#include "packet_api.h"
#include "vtss_tod_api.h"
#include "vtss_tod_phy_engine.h"
#include "vlan_api.h"
#include "misc_api.h"
#include "../base/vtss_mep.h" /* For internal base/platform API */
#if defined(VTSS_ARCH_SERVAL)
#include "qos_api.h"
#endif
#if defined(VTSS_SW_OPTION_PTP)
#include "vtss_ptp_os.h"
#endif
#include "interrupt_api.h"
#if defined(VTSS_SW_OPTION_ACL)
#include "acl_api.h"
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "mep_cli.h"
#endif
#if defined(VTSS_SW_OPTION_EVC)
#include "evc_api.h"
#endif

#if defined(VTSS_SW_OPTION_EPS)
#include "eps_api.h"
#endif

#if defined(VTSS_SW_OPTION_ERPS)
#include "erps_api.h"
#endif

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
#include "vtss_ts_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "mep_icli_functions.h"
#endif

//#undef VTSS_SW_OPTION_ACL

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <sys/time.h>
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"
#include "vtss_tod_mod_man.h"
#include "tod_api.h"
#include "phy_api.h"
#define API_INST_DEFAULT        PHY_INST       
#endif //defined(VTSS_FEATURE_PHY_TIMESTAMP)

#undef VTSS_TRACE_MODULE_ID
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MEP

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_APS_TX       2
#define TRACE_GRP_APS_RX       3
#define TRACE_GRP_TX_FRAME     4
#define TRACE_GRP_RX_FRAME     5
#define TRACE_GRP_CNT          6

#include <vtss_trace_api.h>

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "MEP",
    .descr     = "MEP module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_APS_TX] = { 
        .name      = "txAPS",
        .descr     = "Tx APS print out ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_APS_RX] = { 
        .name      = "rxAPS",
        .descr     = "Rx APS print out ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_TX_FRAME] = { 
        .name      = "txFRAME",
        .descr     = "Tx frame print out ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_RX_FRAME] = { 
        .name      = "rxFRAME",
        .descr     = "Rx frame print out ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};
#define CRIT_ENTER(crit) critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT(crit)  critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CRIT_ENTER(crit) critd_enter(&crit)
#define CRIT_EXIT(crit)  critd_exit( &crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
#define FLAG_TIMER          0x0001
#define APS_INST_INV        0xFFFF
#define SF_EPS_MAX          (MEP_EPS_MAX-1)
#define TX_TIME_STAMP_F_O   18  /* Refer to DMR PDU Format */
#define TX_TIME_STAMP_B_O   34  /* Refer to DMR PDU Format */
#define TAG_LEN             4
#define TX_BUF_MAX          3

//#define MEP_DATA_DUMP              
#undef  MEP_DATA_DUMP
#define DEBUG_WO_PHY            0   
#define VTSS_FLOW_NUM_PER_PORT  4   
#define DEBUG_MORE_INFO         0   

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MEP

typedef struct
{
    ulong                       version;                          /* Block version */
    BOOL                        up_mep_enabled;
    vtss_mep_mgmt_conf_t        conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_cc_conf_t     cc_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_lm_conf_t     lm_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_dm_conf_t     dm_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_aps_conf_t    aps_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_ais_conf_t    ais_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_lck_conf_t    lck_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_client_conf_t client_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_pm_conf_t     pm_conf[MEP_INSTANCE_MAX];
} mep_conf_t;

static cyg_handle_t        timer_thread_handle;
static cyg_thread          timer_thread_block;
static char                timer_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_handle_t        run_thread_handle;
static cyg_thread          run_thread_block;
static char                run_thread_stack[THREAD_DEFAULT_STACK_SIZE];

static critd_t             crit_p;      /* Platform critd */
static critd_t             crit;        /* Base MEP critd */
static critd_t             crit_supp;   /* Base MEP Support critd */

static cyg_sem_t           run_wait_sem;
static cyg_flag_t          timer_wait_flag;

typedef struct
{
    BOOL                       port[VTSS_PORT_ARRAY_SIZE];       /* Port to be used by MEP */
    u32                        ssf_count;                        /* Number of related EPS (SF_EPS_MAX)            */
    u16                        ssf_inst[SF_EPS_MAX];             /* SF/SD state to all    */
    u16                        aps_inst;
    vtss_mep_mgmt_aps_type_t   aps_type;
    mep_eps_type_t             eps_type;
    vtss_mep_mgmt_domain_t     domain;
    u32                        flow;
    u32                        res_port;
    BOOL                       enable;

    u8                         aps_txdata[VTSS_MEP_APS_DATA_LENGTH];
    BOOL                       raps_tx;
    BOOL                       raps_forward;

    u8                         *tx_buffer[TX_BUF_MAX][2];       /* Extra transmit buffers for auto CCM/TST transmission on protection port or into major ring */
    vtss_ace_id_t              ccm_ace_id[2];                   /* The ACE id of the HW CCM ACE rule */
    vtss_ace_id_t              tst_ace_id[2];                   /* The ACE id of the TST ACE rule */
    vtss_ace_id_t              mip_lbm_ace_id;
} instance_data_t;

static void vlan_custom_ethertype_callback(void);


#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
typedef struct 
{
    vtss_timeinterval_t max;
    vtss_timeinterval_t min;
    vtss_timeinterval_t mean;
    u32 cnt;
} mep_observed_egr_lat_t;
/* latency observed in onestep tx timestamping */
static mep_observed_egr_lat_t observed_egr_lat = {0,0,0,0};
#endif

static instance_data_t  instance_data[MEP_INSTANCE_MAX];
static BOOL             los_state[VTSS_PORT_ARRAY_SIZE];
static BOOL             conv_los_state[VTSS_PORT_ARRAY_SIZE];
static u32              in_port_conv[VTSS_PORT_ARRAY_SIZE];     
static u32              out_port_conv[VTSS_PORT_ARRAY_SIZE];
static BOOL             out_port_1p1[VTSS_PORT_ARRAY_SIZE];
static vtss_ace_id_t    up_mep_tst_ace_id=ACE_ID_NONE;                      /* The ACE id of the Up-MEP TST ACE rule */
#ifdef VTSS_SW_OPTION_MEP_LOOP_PORT
static u32              voe_up_mep_loop_port = (VTSS_PORT_ARRAY_SIZE-1);
#else
static u32              voe_up_mep_loop_port = VTSS_PORT_ARRAY_SIZE;
#endif

#define EVC_VLAN_VID_SIZE 4096
static u32              evc_vlan_vid[EVC_VLAN_VID_SIZE];

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static struct {
    u16 channel_id;                             /* identifies the channel id in the PHY
                                                   (needed to access the timestamping feature) */
    BOOL port_shared;                           /* port shares engine with another port */
    vtss_port_no_t shared_port_no;              /* the port this engine is shared with. */
    BOOL port_phy_ts;                           /* PHY timestamping type PHY            */
} port_data [VTSS_PORTS];

typedef struct {
    BOOL                        phy_ts_port;  /* TRUE if this port is a PTP port and has the PHY timestamp feature */
   
    vtss_phy_ts_engine_t        engine_id;   /* VTSS_PHY_TS_ENGINE_ID_INVALID indicates that no engine is allocated */
    u8                          ing_mac[VTSS_FLOW_NUM_PER_PORT][VTSS_MEP_MAC_LENGTH]; /* MAC address of the flow to set to PHY */
    u8                          ing_shr_no[VTSS_FLOW_NUM_PER_PORT]; /* the number of mep to share this flow (MAC and tag_no) */
    u8                          ing_mac_cnt; /* count of the used flow*/
    //vtss_phy_ts_engine_t        egress_id;    /* VTSS_PHY_TS_ENGINE_ID_INVALID indicates that no engine is allocated */
    u8                          eg_mac[VTSS_FLOW_NUM_PER_PORT][VTSS_MEP_MAC_LENGTH];
    u8                          eg_shr_no[VTSS_FLOW_NUM_PER_PORT]; /* the number of mep to share this flow (MAC and tag_no) */
    u8                          eg_mac_cnt; /* count of the used flow*/
    u8                          flow_id_low;  /* identifies the flows allocated for this port */
    u8                          flow_id_high; /* identifies the flows allocated for this port */
} mep_phy_ts_engine_alloc_t;

mep_phy_ts_engine_alloc_t mep_phy_ts_port[VTSS_PORTS];


#ifdef MEP_DATA_DUMP
void phy_ts_dump(u8 port)
{
    int i,p;
    int y, z;
   
    if (port == 0xFF){
        /* dump all ports */
        y = 0;
        z = VTSS_PORTS;
    } else {
        y = port;
        z = port + 1;
    }    
           
   
    for (i = y ; i < z ; i++) {
        printf("port %d: is_ts_port: %d, igrid %d, egrid %d f_lo: %d f_hi: %d\n", i, 
                       mep_phy_ts_port[i].phy_ts_port, 
                       mep_phy_ts_port[i].engine_id, 
                       mep_phy_ts_port[i].engine_id, 
                       mep_phy_ts_port[i].flow_id_low,
                       mep_phy_ts_port[i].flow_id_high);
        printf("ing_mac_cnt: %d\n", mep_phy_ts_port[i].ing_mac_cnt);
        for (p = 0 ; p < mep_phy_ts_port[i].ing_mac_cnt ; p++) {
            printf(" mac address %s, share_no: %d\n",
            misc_mac2str(&mep_phy_ts_port[i].ing_mac[p][0]),
            mep_phy_ts_port[i].ing_shr_no[p]);
        }
        
        printf("eg_mac_cnt: %d\n", mep_phy_ts_port[i].eg_mac_cnt);
        for (p = 0 ; p < mep_phy_ts_port[i].eg_mac_cnt ; p++) {
            printf(" mac address %s, share_no: %d\n",
            misc_mac2str(&mep_phy_ts_port[i].eg_mac[p][0]),
            mep_phy_ts_port[i].eg_shr_no[p]);
        }  
            
    }
}

static void dump_conf(vtss_phy_ts_engine_flow_conf_t *flow_conf)
{
    int i;
    printf("flow conf: eng_mode %d ", flow_conf->eng_mode);
    printf("mep comm: ppb_en %d, etype %x, tpid %x\n", flow_conf->flow_conf.oam.eth1_opt.comm_opt.pbb_en,
           flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype,
           flow_conf->flow_conf.oam.eth1_opt.comm_opt.tpid);
    for (i = 0; i < 8; i++) {
        printf(" channel_map[%d] %d ", i, flow_conf->channel_map[i]);
        printf(" mep flow: flow_en %d, match_mode %d, match_select %d\n", flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].flow_en,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].addr_match_mode,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].addr_match_select);
        printf(" mac address %s\n",misc_mac2str(flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].mac_addr));
        printf(" vlan_check %d, num_tag %d, outer_tag_type %d, inner_tag_type %d, tag_range_mode %d\n", 
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].vlan_check,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].num_tag,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].outer_tag_type,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].inner_tag_type,
               flow_conf->flow_conf.oam.eth1_opt.flow_opt[i].tag_range_mode);
    }
}

static void dump_mep_action(vtss_phy_ts_engine_action_t *oam_action)
{
    int i;
    printf("action_ptp %d\n", oam_action->action_ptp);
    for (i = 0; i < 2; i++) {
        printf(" action[%d]: enable %d, channel_map %d\n", i, oam_action->action.ptp_conf[i].enable, oam_action->action.oam_conf[i].channel_map);
#if 0        
        printf("  ptpconf: range_en %d, val/upper %d, mask/lower %d\n", oam_action->action.oam_conf[i].oam_conf.range_en,
               oam_action->action.ptp_conf[i].oam_conf.domain.value.val,
               oam_action->action.ptp_conf[i].oam_conf.domain.value.mask);
        printf("   clk__mode %d, delaym_type %d\n",oam_action->action.oam_conf[i].clk_mode,
              oam_action->action.ptp_conf[i].delaym_type);
#endif    
    }
}

#endif //#ifdef MEP_DATA_DUMP

static void mep_phy_config_mac(const u8  is_del,
                               const u8  port,
                               const u8  is_ing,
                               const u8  mac[VTSS_MEP_SUPP_MAC_LENGTH])
{
    /* 
     *  If is_del is 0, it means to add an entry. Otherwise the
     *  value of (is_del - 1) is the index to delete. Add 1 to avoid
     *  0 used for addition.
     */
    if (is_ing) {
        /* ingress */
        if (is_del == 0) {
#if DEBUG_MORE_INFO             
            printf(" add : is_del: %d  mep_phy_ts_port[port].ing_mac_cnt:%d\n", is_del, mep_phy_ts_port[port].ing_mac_cnt);
#endif            
            memcpy(mep_phy_ts_port[port].ing_mac[mep_phy_ts_port[port].ing_mac_cnt], mac, sizeof(mep_phy_ts_port[port].ing_mac[mep_phy_ts_port[port].ing_mac_cnt]));
            mep_phy_ts_port[port].ing_shr_no[mep_phy_ts_port[port].ing_mac_cnt] = 0;
            mep_phy_ts_port[port].ing_mac_cnt++;
        } else {
            /* delete */
#if DEBUG_MORE_INFO             
            printf("delete : is_del: %d  mep_phy_ts_port[port].ing_mac_cnt:%d\n", is_del, mep_phy_ts_port[port].ing_mac_cnt);
#endif            
            if (mep_phy_ts_port[port].ing_mac_cnt) {
                if (is_del != mep_phy_ts_port[port].ing_mac_cnt) {
                    /* means not to delete the last one
                       move the last one to the position deleted */
                    memcpy(mep_phy_ts_port[port].ing_mac[is_del - 1], mep_phy_ts_port[port].ing_mac[mep_phy_ts_port[port].ing_mac_cnt - 1], sizeof(mep_phy_ts_port[port].ing_mac[is_del - 1]));
                }
                mep_phy_ts_port[port].ing_mac_cnt--;    
            }         
        }        
            
    } else {
        /* egress */ 
        if (is_del == 0) {
#if DEBUG_MORE_INFO             
            printf(" add : is_del: %d  mep_phy_ts_port[port].eg_mac_cnt:%d\n", is_del, mep_phy_ts_port[port].eg_mac_cnt);
#endif            
            memcpy(mep_phy_ts_port[port].eg_mac[mep_phy_ts_port[port].eg_mac_cnt], mac, sizeof(mep_phy_ts_port[port].eg_mac[mep_phy_ts_port[port].eg_mac_cnt]));
            mep_phy_ts_port[port].eg_shr_no[mep_phy_ts_port[port].eg_mac_cnt] = 0;
            mep_phy_ts_port[port].eg_mac_cnt++;
        } else {
            /* delete */
#if DEBUG_MORE_INFO             
            printf("delete : is_del: %d  mep_phy_ts_port[port].eg_mac_cnt:%d\n", is_del, mep_phy_ts_port[port].eg_mac_cnt);
#endif            
            if (mep_phy_ts_port[port].eg_mac_cnt) {
                if (is_del != mep_phy_ts_port[port].eg_mac_cnt) {
                    /* means not to delete the last one
                       move the last one to the position deleted */
                    memcpy(mep_phy_ts_port[port].eg_mac[is_del - 1], mep_phy_ts_port[port].eg_mac[mep_phy_ts_port[port].eg_mac_cnt - 1], sizeof(mep_phy_ts_port[port].eg_mac[is_del - 1]));
                }
                mep_phy_ts_port[port].eg_mac_cnt--;    
            }         
        } 
    }        
                              
}

static vtss_rc mep_phy_ts_update(u8 port)
{
    vtss_rc rc = VTSS_RC_OK;
    u8  mac_count;
    vtss_phy_ts_encap_t encap_type;
    vtss_port_no_t shared_port;
    u8  flow_id;
    BOOL engine_free = FALSE;

    encap_type = VTSS_PHY_TS_ENCAP_ETH_OAM;
    T_D("setting ts rules for port %d", port);
#if DEBUG_MORE_INFO     
    printf("mep_phy_ts_update 1 \n");
#endif
    
    /* We only support unicast solution.
     * We don't have the issue of h/w resource not enough so we just use the
     * easy way to allocate and configure it in the initial time and not to free
     * even some are really not used. The software architeture still reserves the 
     * flexibility to free h/w resource for use of implementing multicast soulution
     * later.
     */
    if (mep_phy_ts_port[port].phy_ts_port) {  
	/**/
        T_D("allocate engines for port %d", port);
        /* Allocate engines for this instance */
        if (mep_phy_ts_port[port].engine_id == VTSS_PHY_TS_ENGINE_ID_INVALID) { /* if not already allocated */
            /* allocate the engine only if PHY time stamping support is available*/
            mep_phy_ts_port[port].engine_id = tod_phy_eng_alloc(port, encap_type);
            T_I("allocated engine %d for port %d", mep_phy_ts_port[port].engine_id, port);
            
            rc = vtss_phy_ts_ingress_engine_init(API_INST_DEFAULT,
                    port,
                    mep_phy_ts_port[port].engine_id,
                    encap_type,
                    0, 3, /* 4 flows are always available (engine 2 can be shared between 2 API engines), we can allocate maximum up to 8 flows  */
                    VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT);
                    
             if (rc != VTSS_RC_OK) {
                T_E("mep_phy_ts_update - fail to allocate ingress engine");
                return rc;
             }
#if DEBUG_MORE_INFO                    
            T_E("ing eng alloc id: %d for port %d, encap_type %d", mep_phy_ts_port[port].engine_id, port, encap_type);
#endif     
            if (port_data[port].port_shared) {
                /* In dual PHY, the first port uses the first two of the 4 allocated flows
                   and the second one uses the last two */
                shared_port = port_data[port].shared_port_no;
                mep_phy_ts_port[shared_port].engine_id = mep_phy_ts_port[port].engine_id;
#if DEBUG_MORE_INFO
                T_E("shared engine: port %d, sharedPort %ld, ing_id %d", 
                     port, shared_port, mep_phy_ts_port[port].engine_id);
#endif                
                if (port_data[port].channel_id == 0) {
                    mep_phy_ts_port[port].flow_id_low = 0;
                    mep_phy_ts_port[port].flow_id_high = 1;
                    mep_phy_ts_port[shared_port].flow_id_low = 2;
                    mep_phy_ts_port[shared_port].flow_id_high = 3;
                } else {
                    mep_phy_ts_port[port].flow_id_low = 2;
                    mep_phy_ts_port[port].flow_id_high = 3;
                    mep_phy_ts_port[shared_port].flow_id_low = 0;
                    mep_phy_ts_port[shared_port].flow_id_high = 1;
                }
            } else {
                /* In single PHY, the port uses 4 flows */
#if DEBUG_MORE_INFO
                T_E("non shared engine: port %d, ing_id %d", 
                     port, mep_phy_ts_port[port].engine_id);
#endif                
                mep_phy_ts_port[port].flow_id_low = 0;
                mep_phy_ts_port[port].flow_id_high = 3;
            }
        
            rc = vtss_phy_ts_egress_engine_init(API_INST_DEFAULT,
              port,
               mep_phy_ts_port[port].engine_id,
              encap_type,
              0, 3, /* 4 flows are always available (engine 2 can be shared between 2 API engines)  */
              VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT);
              
            if (rc != VTSS_RC_OK) {
                 T_E("mep_phy_ts_update - fail to allocate egress engine");
                 return rc;
            }   
#if DEBUG_MORE_INFO                                    
        T_E("eg eng alloc id: %d for port %d, encap_type %d", mep_phy_ts_port[port].engine_id, port, encap_type);
#endif  
        }
                   
        /* Set up flow comparators */
        vtss_phy_ts_engine_flow_conf_t flow_conf;
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &flow_conf);
        
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_ingress_engine_conf_get fail ");
            return rc;
        }                                    
                
#ifdef MEP_DATA_DUMP
        T_E("conf dump before:");
        dump_conf(&flow_conf);
#endif
#if DEBUG_MORE_INFO
        T_E("get ing engine conf: %d", mep_phy_ts_port[port].engine_id);
#endif        
        /* modify flow configuration */
        flow_conf.eng_mode = TRUE;
        flow_conf.flow_conf.oam.eth1_opt.comm_opt.etype = 0x8902;

        mac_count = mep_phy_ts_port[port].ing_mac_cnt;
        for (flow_id = mep_phy_ts_port[port].flow_id_low; flow_id <= mep_phy_ts_port[port].flow_id_high; flow_id++) {
            if (mac_count == 0) {
                /* all MACs are in already */
                break;
            } else {
               mac_count--; 
            }
                    
            flow_conf.channel_map[flow_id] = port_data[port].channel_id == 0 ?
                                          VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].flow_en = TRUE;
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
            memcpy(flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].mac_addr,  /* match DA unicast address */
                   mep_phy_ts_port[port].ing_mac[flow_id-mep_phy_ts_port[port].flow_id_low], sizeof(flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].mac_addr));
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].vlan_check = FALSE;
#if DEBUG_MORE_INFO
            T_E("eth: flow_opt[%d]: channel_map %d, flow_en %d, match_mode %d, match:_sel %d", 
                 flow_id, 
                 flow_conf.channel_map[flow_id],
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].flow_en, 
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_mode,
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_select);
#endif        
        }
        /* the same configuration is applied to both ingress and egress */
#ifdef MEP_DATA_DUMP
        T_E("conf dump after:");
        dump_conf(&flow_conf);
#endif
        rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &flow_conf);

        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_ingress_engine_conf_set fail");
            return rc;
        }

        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &flow_conf);
        
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_egress_engine_conf_get fail ");
            return rc;
        }                                    
                     
#if DEBUG_MORE_INFO
        T_E("set ing engine conf: port %d, id %d", port, mep_phy_ts_port[port].engine_id);
#endif    
        /* modify flow configuration */
        flow_conf.eng_mode = TRUE;
        flow_conf.flow_conf.oam.eth1_opt.comm_opt.etype = 0x8902;
            
        mac_count = mep_phy_ts_port[port].eg_mac_cnt;
        for (flow_id = mep_phy_ts_port[port].flow_id_low; flow_id <= mep_phy_ts_port[port].flow_id_high; flow_id++) {
            if (mac_count == 0) {
                /* all MACs are in already */
                break;
            } else {
               mac_count--; 
            }
            flow_conf.channel_map[flow_id] = port_data[port].channel_id == 0 ?
                                          VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].flow_en = TRUE;
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;           
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_ADDR;
            memcpy(flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].mac_addr,  /* match SA unicast address */
                   mep_phy_ts_port[port].eg_mac[flow_id-mep_phy_ts_port[port].flow_id_low], sizeof(flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].mac_addr));
            flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].vlan_check = FALSE;
#if DEBUG_MORE_INFO
            T_E("eth: flow_opt[%d]: channel_map %d, flow_en %d, match_mode %d, match:_sel %d", 
                 flow_id, 
                 flow_conf.channel_map[flow_id],
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].flow_en, 
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_mode,
                 flow_conf.flow_conf.oam.eth1_opt.flow_opt[flow_id].addr_match_select);
#endif        
        }
        
        rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &flow_conf);
        
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_egress_engine_conf_set failure");
            return rc;
        }                    
#if DEBUG_MORE_INFO
        T_E("set eg engine conf: %d", mep_phy_ts_port[port].engine_id);
#endif   
        /* Configure the Actions */
        /* set up actions */
        vtss_phy_ts_engine_action_t oam_action;
        /* Get the default actions which the API gives */
        rc = vtss_phy_ts_ingress_engine_action_get(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &oam_action);
        if (rc != VTSS_RC_OK) {
            T_E("get engine action fail");
            return rc;
        }
#if DEBUG_MORE_INFO
        T_E("get ing action: %d", mep_phy_ts_port[port].engine_id);
#endif        
        oam_action.action_ptp = FALSE;
        /* Maximum of 6 actions are possible for each engine  */
        oam_action.action.oam_conf[0].enable   = TRUE;
        oam_action.action.oam_conf[0].y1731_en = TRUE;
        oam_action.action.oam_conf[0].channel_map |= port_data[port].channel_id == 0 ?
                                         VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
    
        oam_action.action.oam_conf[0].oam_conf.y1731_oam_conf.range_en = FALSE;
        oam_action.action.oam_conf[0].oam_conf.y1731_oam_conf.meg_level.value.val = 0;
        oam_action.action.oam_conf[0].oam_conf.y1731_oam_conf.meg_level.value.mask = 0; /* Don't care */
         /* The action can be set to 1DM or DMM or DMR */
        oam_action.action.oam_conf[0].oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM;
    
        oam_action.action.oam_conf[1].enable   = TRUE;
        oam_action.action.oam_conf[1].y1731_en = TRUE;
        oam_action.action.oam_conf[1].channel_map |= port_data[port].channel_id == 0 ?
                                         VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
    
        oam_action.action.oam_conf[1].oam_conf.y1731_oam_conf.range_en = FALSE;
        oam_action.action.oam_conf[1].oam_conf.y1731_oam_conf.meg_level.value.val = 0;
        oam_action.action.oam_conf[1].oam_conf.y1731_oam_conf.meg_level.value.mask = 0; /* Don't care */
         /* The action can be set to 1DM or DMM or DMR */
        oam_action.action.oam_conf[1].oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM;
        
        oam_action.action.oam_conf[2].enable = FALSE;
        
#ifdef MEP_DATA_DUMP
        dump_mep_action(&oam_action);
#endif
        rc = vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &oam_action);
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_ingress_engine_action_set: failure");
            return rc;
        }                        
#if DEBUG_MORE_INFO
        T_E("set ing action: %d", mep_phy_ts_port[port].engine_id);
#endif
        rc = vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT,
                port,
                mep_phy_ts_port[port].engine_id,
                &oam_action);
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_egress_engine_action_set: failure");
            return rc;
        }
#if DEBUG_MORE_INFO
        T_E("set eg action: %d", mep_phy_ts_port[port].engine_id);
#endif        
        /* Enable phy_ts mode */
        rc = vtss_phy_ts_mode_set(API_INST_DEFAULT, port ,TRUE);
        if (rc != VTSS_RC_OK) {
            T_E("vtss_phy_ts_mode_set: failure");
            return rc;
        }
        
    } else {
        /* not used by this port: is it used by the shared port ? */
        engine_free = TRUE;
        if (port_data[port].port_shared) {
            shared_port = port_data[port].shared_port_no;
            if (mep_phy_ts_port[shared_port].phy_ts_port) {
                engine_free = FALSE;
            }
        } else {
            shared_port = 0;
        }
        if (engine_free && mep_phy_ts_port[port].engine_id != VTSS_PHY_TS_ENGINE_ID_INVALID) { /* if not used any more */
            rc = vtss_phy_ts_ingress_engine_clear(API_INST_DEFAULT, port, mep_phy_ts_port[port].engine_id);
            if (rc != VTSS_RC_OK) {
                T_E("vtss_phy_ts_ingress_engine_clear fail");
                return rc;
            }
#if DEBUG_MORE_INFO
            T_E("ing eng free id: %d for port %d", mep_phy_ts_port[port].engine_id, port);
#endif            
            tod_phy_eng_free(port, mep_phy_ts_port[port].engine_id);
            mep_phy_ts_port[port].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
        }
        if (engine_free && mep_phy_ts_port[port].engine_id != VTSS_PHY_TS_ENGINE_ID_INVALID) {
            rc = vtss_phy_ts_egress_engine_clear(API_INST_DEFAULT, port, mep_phy_ts_port[port].engine_id);
            if (rc != VTSS_RC_OK) {
                T_E("vtss_phy_ts_egress_engine_free fail");
                return rc;
            }
#if DEBUG_MORE_INFO            
            T_E("eg eng free id: %d for port %d", mep_phy_ts_port[port].engine_id, port);
#endif            
            tod_phy_eng_free(port, mep_phy_ts_port[port].engine_id);
            mep_phy_ts_port[port].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
        }
        if (engine_free && port_data[port].port_shared) {
            mep_phy_ts_port[shared_port].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
        }
    }
#ifdef MEP_DATA_DUMP
    phy_ts_dump(port);
#endif
    return rc;
}

static void port_data_initialize(void)
{
#if defined(VTSS_FEATURE_10G)
    vtss_phy_10g_id_t          phy_id;
    vtss_rc                    rc = VTSS_RC_OK;
    vtss_phy_10g_mode_t  phy_10g_mode;
#endif
    int                        i;

    CRIT_ENTER(crit_p);        
    for (i = 0; i < VTSS_PORTS; i++) {
        /* is this port a PHY TS port ? */
#if defined(VTSS_FEATURE_10G)
#if !DEBUG_WO_PHY
        phy_10g_mode.oper_mode = VTSS_PHY_1G_MODE;
        rc = vtss_phy_10g_id_get(API_INST_DEFAULT, i, &phy_id);
        if (rc == VTSS_RC_OK && (phy_id.part_number == 0x8488 || phy_id.part_number == 0x8487) &&
                                phy_id.revision >= 4) {
            rc = vtss_phy_10g_mode_get (API_INST_DEFAULT, i, &phy_10g_mode);
        }
        if (rc == VTSS_RC_OK && (phy_id.part_number == 0x8488 || phy_id.part_number == 0x8487) &&
                phy_id.revision >= 4 && phy_10g_mode.oper_mode != VTSS_PHY_1G_MODE) {

#else
        BOOL  test = TRUE;
        if (test){
#endif            
            mep_phy_ts_port[i].phy_ts_port = TRUE; 
            mep_phy_ts_port[i].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
            mep_phy_ts_port[i].ing_mac_cnt = 0;
            mep_phy_ts_port[i].eg_mac_cnt = 0; 
            
            /* This information needs to be maintained in the application to know which
             * ports are shared as the PHY has only one analyzer that needs to be shared
             * between the two channels
             */
            port_data[i].channel_id  = phy_id.channel_id;
            port_data[i].port_phy_ts = TRUE;
            
            if (phy_id.phy_api_base_no != i) {
                port_data[i].port_shared = TRUE;
                port_data[i].shared_port_no = phy_id.phy_api_base_no;
                port_data[phy_id.phy_api_base_no].port_shared = TRUE;
                port_data[phy_id.phy_api_base_no].shared_port_no = i;
            }
            /* Initialize the TS Functionality: is moved to the tod module */
#if DEBUG_MORE_INFO
            T_E("xxx Port_no = %d, uses 1588 PHY, channel %d, phy_api_base_no %ld, rc = %x ", i, phy_id.channel_id, phy_id.phy_api_base_no, rc);
#endif            

        } else {
            port_data[i].port_phy_ts = FALSE;
        }
#else
        port_data[i].port_phy_ts = FALSE;
#endif /* (VTSS_FEATURE_10G) */
    }
    CRIT_EXIT(crit_p);            
}

#endif //#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
BOOL vtss_mep_phy_config_mac(const u8  port,
                             const u8  is_add,
                             const u8  is_ing,
                             const u8  mac[VTSS_MEP_MAC_LENGTH])
{
    u8      i, changed = 0;
    BOOL    ret;
    
    CRIT_ENTER(crit_p);
#if !DEBUG_WO_PHY
    /* for debug only*/    
    if (!mep_phy_ts_port[port].phy_ts_port) {
        CRIT_EXIT(crit_p);
        return TRUE; /* Do nothing for port not support PHY timestamp */
    }
#endif
#if DEBUG_MORE_INFO    
    printf("port: %d, is_add: %d, is_ing: %d xxx\n", port,is_add, is_ing);
#endif    
    if (is_ing) {
        if (mep_phy_ts_port[port].ing_mac_cnt == 0) {
            if (is_add) {
                mep_phy_config_mac(0, port, is_ing, mac);
#if DEBUG_MORE_INFO
                T_E("1\n");
#endif                
                changed = 1;
                ret = TRUE;
            } else {
                /* delete */
                /* databse is empty, nothing to do */
#if DEBUG_MORE_INFO
                T_E("2\n");
#endif                
                ret =  TRUE;
            }        
        } else {
            for (i = 0; i < VTSS_FLOW_NUM_PER_PORT; i++) {
                if (memcmp(&mep_phy_ts_port[port].ing_mac[i][0],
                    mac, VTSS_MEP_SUPP_MAC_LENGTH) == 0) {
                    /* the MAC address is alreay existed, 
                     * just update share number
                     */
#if DEBUG_MORE_INFO
                    T_E("2-1"); 
#endif                   
                    break;
                }
            }
            if (i == VTSS_FLOW_NUM_PER_PORT) {
                /* entry not existed */
                if (is_add) { 
                    if (mep_phy_ts_port[port].ing_mac_cnt < VTSS_FLOW_NUM_PER_PORT) {
                        /* with room to add */
                        mep_phy_config_mac(0, port, is_ing, mac);
                        changed = 1;
#if DEBUG_MORE_INFO
                        T_E("3\n");
#endif                   
                        ret =  TRUE;        
                    }
                    else {
                        /* without room to add */
                        T_D("vtss_mep_phy_add_mac:ingrss no room to addd\n");
#if DEBUG_MORE_INFO
                        T_E("4\n");
#endif                   
                        ret =  FALSE;    
                    } 
                } else {
                    /* delete */
                    /* entry not existed, nothing to do */
#if DEBUG_MORE_INFO
                    T_E("5\n");
#endif                   
                    ret =  TRUE;
                }               
            } else {
                if (is_add) {
                    /* share with the existing entry */
                    /* add 1 to share number */
                    mep_phy_ts_port[port].ing_shr_no[i]++;
                    changed = 1;
#if DEBUG_MORE_INFO
                    T_E("6\n");
#endif                   
                    ret =  TRUE;
                } else {
                    /* delete */
                    if (mep_phy_ts_port[port].ing_shr_no[i]) {
                        /* the entry is still used by others */
                        /* subtract 1 from share number */
                        mep_phy_ts_port[port].ing_shr_no[i]--;
#if DEBUG_MORE_INFO
                        T_E("7\n");
#endif                   
                        ret =  TRUE; 
                    } else {
                        /* remove entry */
                        mep_phy_config_mac((i+1), port, is_ing, mac);
                        changed = 1;
#if DEBUG_MORE_INFO
                        T_E("8\n");
#endif                   
                        ret =  TRUE;
                    }        
                }        
            }         
                   
        }
    } else {
        /* egress */
        if (mep_phy_ts_port[port].eg_mac_cnt == 0) {
            if (is_add) {
                mep_phy_config_mac(0, port, is_ing, mac);
#if DEBUG_MORE_INFO
                T_E("1\n");
#endif                
                changed = 1;
                ret =  TRUE;
            } else {
                /* delete */
                /* databse is empty, nothing to do */
#if DEBUG_MORE_INFO
                T_E("2\n");
#endif                
                ret =  TRUE;
            }        
        } else {
            for (i = 0; i < VTSS_FLOW_NUM_PER_PORT; i++) {
                if (memcmp(&mep_phy_ts_port[port].eg_mac[i][0],
                    mac, VTSS_MEP_SUPP_MAC_LENGTH) == 0) {
                    /* the MAC address is alreay existed, 
                     * just update share number
                     */
#if DEBUG_MORE_INFO
                     T_E("2-1");
#endif                   
                     break;
                }
            }
            if (i == VTSS_FLOW_NUM_PER_PORT) {
                /* entry not existed */
                if (is_add) { 
                    if (mep_phy_ts_port[port].eg_mac_cnt < VTSS_FLOW_NUM_PER_PORT) {
                        /* with room to add */
                        mep_phy_config_mac(0, port, is_ing, mac);
                        changed = 1;
#if DEBUG_MORE_INFO
                        T_E("3\n");
#endif                   
                        ret =  TRUE;        
                    }
                    else {
                        /* without room to add */
                        T_D("vtss_mep_phy_add_mac:ingrss no room to addd\n");
#if DEBUG_MORE_INFO
                        T_E("4\n");
#endif                   
                        ret =  FALSE;    
                    } 
                } else {
                    /* delete */
                    /* entry not existed, nothing to do */
#if DEBUG_MORE_INFO
                    T_E("5\n");
#endif                   
                    ret =  TRUE;
                }               
            } else {
                if (is_add) {
                    /* share with the existing entry */
                    /* add 1 to share number */
                    mep_phy_ts_port[port].eg_shr_no[i]++;
#if DEBUG_MORE_INFO
                    T_E("6\n");
#endif                   
                    ret =  TRUE;
                } else {
                    /* delete */
                    if (mep_phy_ts_port[port].eg_shr_no[i]) {
                        /* the entry is still used by others */
                        /* subtract 1 from share number */
                        mep_phy_ts_port[port].eg_shr_no[i]--;
#if DEBUG_MORE_INFO
                        T_E("7\n");
#endif                   
                        ret =  TRUE; 
                    } else {
                        /* remove entry */
                        mep_phy_config_mac((i+1), port, is_ing, mac);
                        changed = 1;
#if DEBUG_MORE_INFO                        
                        T_E("8\n");
#endif                   
                        ret =  TRUE;
                    }        
                }        
            }         
                   
        }
    }
#if DEBUG_MORE_INFO      
    T_E("changed: %d\n", changed);
#endif     
    if (changed) {
        /* do nothing */
        /* mep_phy_ts_update(port); */
    }
    
    CRIT_EXIT(crit_p); 
    return ret;   
}
#endif //#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

static void los_state_set(const u32 port,     const BOOL state)
{
    u32 i, j, los=FALSE;

//printf("los_state_set  port %u  state %u\n", port, state);
    CRIT_ENTER(crit_p);
    if (los_state[port] != state)
    {/* Only do this if state changed */
        los_state[port] = state;

        if (in_port_conv[port] != 0xFFFF)       /* If we don't discard on this port due to port protection */
            conv_los_state[in_port_conv[port]] = state;
        conv_los_state[port] = state;
//printf("conv_los_state  %u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", conv_los_state[0], conv_los_state[1], conv_los_state[2], conv_los_state[3], conv_los_state[4], conv_los_state[5], conv_los_state[6], conv_los_state[7], conv_los_state[8], conv_los_state[9], conv_los_state[10]);
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if (!instance_data[i].enable)   continue;
            if (instance_data[i].domain == VTSS_MEP_MGMT_PORT)
            {/* Port Domain */
                for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j)  /* Calculate SSF for this mep instance based on los state on all related ports */
                    if ((instance_data[i].port[j]) && (!los_state[j]))      break;
                los = (j == VTSS_PORT_ARRAY_SIZE);      /* LOS is detected if no related port without LOS active was found */
                CRIT_EXIT(crit_p);
                vtss_mep_new_ssf_state(i, los);
                CRIT_ENTER(crit_p);
            }
            if (((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN)) && (in_port_conv[port] != 0xFFFF) && (instance_data[i].port[in_port_conv[port]]))
            {/* EVC domain and NOT discarding due to port protection and port is related to this MEP */
                for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j)  /* Calculate SSF for this mep instance based on los state on all related ports */
                    if ((instance_data[i].port[j]) && (!conv_los_state[j]))      break;
                los = (j == VTSS_PORT_ARRAY_SIZE);      /* LOS is detected if no related port without LOS active was found */
                CRIT_EXIT(crit_p);
                vtss_mep_new_ssf_state(i, los);
                CRIT_ENTER(crit_p);
            }
        }
    }
    CRIT_EXIT(crit_p);
}

/*lint -e{454, 455, 456} ... The mutex is locked so it is ok to unlock */
static void prot_state_change(const u32 port)   /* This port is now active in the port protection */
{
    u32 i, j, los;

/*T_D("port %lu", port);*/
    conv_los_state[in_port_conv[port]] = los_state[port];	/* Calculate the port state on the active port */
//printf("conv_los_state  %u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", conv_los_state[0], conv_los_state[1], conv_los_state[2], conv_los_state[3], conv_los_state[4], conv_los_state[5], conv_los_state[6], conv_los_state[7], conv_los_state[8], conv_los_state[9], conv_los_state[10], conv_los_state[11]);

    for (i=0; i<MEP_INSTANCE_MAX; ++i)	/* For all EVC MEP 'behind' this port protection */
        if ((instance_data[i].enable) && ((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN)) && (instance_data[i].port[in_port_conv[port]]))
        {/* MEP is enabled in EVC domaine and port is related to this MEP */
            for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j)  /* Calculate SSF for this mep instance based on los state on all related ports */
                if ((instance_data[i].port[j]) && (!conv_los_state[j]))      break;
            los = (j == VTSS_PORT_ARRAY_SIZE);      /* LOS is detected if no related port without LOS active was found */
            CRIT_EXIT(crit_p);
            vtss_mep_new_ssf_state(i,  los);
            CRIT_ENTER(crit_p);
        }
}

static void set_conf_to_default(mep_conf_t  *blk)
{
    uint i;
    vtss_mep_mgmt_def_conf_t  def_conf;

    vtss_mep_mgmt_def_conf_get(&def_conf);

    CRIT_ENTER(crit_p);
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        memset(blk, 0, sizeof(mep_conf_t));
        blk->conf[i] = def_conf.config;
        blk->cc_conf[i] = def_conf.cc_conf;
        blk->lm_conf[i] = def_conf.lm_conf;
        blk->dm_conf[i] = def_conf.dm_conf;
        blk->aps_conf[i] = def_conf.aps_conf;
        blk->ais_conf[i] = def_conf.ais_conf;
        blk->lck_conf[i] = def_conf.lck_conf;
        blk->client_conf[i] = def_conf.client_conf;

        memset(&instance_data[i], 0, sizeof(instance_data_t));
        instance_data[i].aps_inst = APS_INST_INV;
    }

    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {
        in_port_conv[i] = out_port_conv[i] = i;
        conv_los_state[i] = FALSE;
        out_port_1p1[i] = FALSE;
    }
    CRIT_EXIT(crit_p);
}

static void apply_configuration(mep_conf_t  *blk)
{
    u32           base_rc = VTSS_MEP_RC_OK, i;
    vtss_mep_mgmt_def_conf_t  def_conf;

    vtss_mep_mgmt_def_conf_get(&def_conf);

#if defined(VTSS_SW_OPTION_UP_MEP)
    if (blk->up_mep_enabled)    vtss_mep_mgmt_up_mep_enable();
#endif
    for (i=0; i<MEP_INSTANCE_MAX; ++i) {
        base_rc += vtss_mep_mgmt_conf_set(i, &blk->conf[i]);
        if (blk->conf[i].enable) {
            base_rc += vtss_mep_mgmt_cc_conf_set(i, &blk->cc_conf[i]);
            base_rc += vtss_mep_mgmt_lm_conf_set(i, &blk->lm_conf[i]);
            base_rc += vtss_mep_mgmt_dm_conf_set(i, &blk->dm_conf[i]);
            base_rc += vtss_mep_mgmt_aps_conf_set(i, &blk->aps_conf[i]);
            base_rc += vtss_mep_mgmt_ais_conf_set(i, &blk->ais_conf[i]);
            base_rc += vtss_mep_mgmt_lck_conf_set(i, &blk->lck_conf[i]);
            base_rc += vtss_mep_mgmt_client_conf_set(i, &blk->client_conf[i]);
            base_rc += vtss_mep_mgmt_lb_conf_set(i, &def_conf.lb_conf);
            base_rc += vtss_mep_mgmt_lt_conf_set(i, &def_conf.lt_conf);
            base_rc += vtss_mep_mgmt_tst_conf_set(i, &def_conf.tst_conf);
        }

        instance_data[i].enable = blk->conf[i].enable;
        if (blk->conf[i].enable) {
            instance_data[i].domain = blk->conf[i].domain;
            instance_data[i].flow = blk->conf[i].flow;
            instance_data[i].res_port = blk->conf[i].port;
            instance_data[i].aps_type = blk->aps_conf[i].type;
        } else {
            instance_data[i].domain = VTSS_MEP_MGMT_PORT;
        }
    }

    if (base_rc) {
        T_D("Error during configuration");
    }
}

static void restore_to_default(void)
{
    mep_conf_t *blk = (mep_conf_t *)VTSS_MALLOC(sizeof(*blk));
    if (!blk) {
        T_W("Out of memory for MEP restore-to-default");
    } else {
        set_conf_to_default(blk);
        apply_configuration(blk);
        VTSS_FREE(blk);
    }
}


static void mep_timer_thread(cyg_addrword_t data)
{
    BOOL               stop, supp_stop;
    cyg_flag_value_t   flag_value;
    vtss_mtimer_t      onesec_timer;
    vtss_vlan_conf_t   vlan_conf;
    vtss_etype_t       s_etype;

    stop = supp_stop = FALSE;
    s_etype = 0;

    VTSS_MTIMER_START(&onesec_timer, 1000);

    for (;;)
    {
        VTSS_OS_MSLEEP(10);

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_JAGUAR_1)
        vtss_mep_ccm_thread();
#endif
        flag_value = FLAG_TIMER;
        flag_value = cyg_flag_poll(&timer_wait_flag, flag_value, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (!(stop && supp_stop) || flag_value)
        {
            vtss_mep_supp_timer_thread(&supp_stop);
            vtss_mep_timer_thread(&stop);
        }

        if (VTSS_MTIMER_TIMEOUT(&onesec_timer)) {
            /* Polling of the Custom S-Tag Ether Type is done once a second - as there are currently no call back on this */
            VTSS_MTIMER_START(&onesec_timer, 1000);

            if (vtss_vlan_conf_get(NULL, &vlan_conf) == VTSS_MEP_RC_OK) { /* Get the custom S-Port EtherType */
                if (s_etype != vlan_conf.s_etype) { /* The custom S-Port EtherType has changed */
                    s_etype = vlan_conf.s_etype;
                    vlan_custom_ethertype_callback();
                }
            }
        }
    }
}

static void mep_run_thread(cyg_addrword_t data)
{
    cyg_bool_t rc;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    u32 i;
    cyg_tick_count_t wakeup = cyg_current_time() + (100/ECOS_MSECS_PER_HWTICK);
	cyg_flag_value_t flags;
    static cyg_flag_t control_flags;   /* thread control */

    cyg_flag_init( &control_flags );
	while (!tod_ready()) {
		T_I("wait until TOD module is ready");
		flags = cyg_flag_timed_wait(&control_flags, 0xffff, 0, wakeup);
		T_I("wait until TOD module is ready (flags %x)", flags);
		wakeup += (100/ECOS_MSECS_PER_HWTICK);
	}
    port_data_initialize();
    /* Initial related rule for PHY TS */
#if 1
    u8 mac[VTSS_MEP_MAC_LENGTH];
    for (i = 0; i < VTSS_PORTS; i++) {
        if (mep_phy_ts_port[i].phy_ts_port == TRUE) {
            vtss_mep_mac_get(i, mac);  /* Get MAC of recidence port */
            (void)vtss_mep_phy_config_mac(i, 1, 1, mac); /* Add port mac to ingress side */
            (void)vtss_mep_phy_config_mac(i, 1, 0, mac); /* Add port mac to egress side */
#ifdef MEP_DATA_DUMP
            phy_ts_dump(i);
#endif
            rc = mep_phy_ts_update(i);
            if (rc != VTSS_RC_OK)        T_E("Error during config PHY timeStamp");
        }
    }
#endif
#endif

    cyg_thread_resume(timer_thread_handle);

    for (;;)
    {
        rc = cyg_semaphore_wait(&run_wait_sem);
        if (!rc)        T_D("Thread was released");
        vtss_mep_supp_run_thread();
        vtss_mep_run_thread();
    }
}


static vtss_rc rc_conv(u32 base_rc)
{
    switch (base_rc) {
    case VTSS_MEP_RC_OK:                return VTSS_RC_OK;
    case VTSS_MEP_RC_INVALID_PARAMETER: return MEP_RC_INVALID_PARAMETER;
    case VTSS_MEP_RC_NOT_ENABLED:       return MEP_RC_NOT_ENABLED;
    case VTSS_MEP_RC_CAST:              return MEP_RC_CAST;
    case VTSS_MEP_RC_PERIOD:            return MEP_RC_PERIOD;
    case VTSS_MEP_RC_PEER_CNT:          return MEP_RC_PEER_CNT;
    case VTSS_MEP_RC_PEER_ID:           return MEP_RC_PEER_ID;
    case VTSS_MEP_RC_MIP:               return MEP_RC_MIP;
    case VTSS_MEP_RC_INVALID_EVC:       return MEP_RC_INVALID_EVC;
    case VTSS_MEP_RC_APS_UP:            return MEP_RC_APS_UP;
    case VTSS_MEP_RC_APS_DOMAIN:        return MEP_RC_APS_DOMAIN;
    case VTSS_MEP_RC_INVALID_VID:       return MEP_RC_INVALID_VID;
    case VTSS_MEP_RC_INVALID_COS_ID:    return MEP_RC_INVALID_COS_ID;
    case VTSS_MEP_RC_NO_VOE:            return MEP_RC_NO_VOE;
    case VTSS_MEP_RC_NO_TIMESTAMP_DATA: return MEP_RC_NO_TIMESTAMP_DATA;
    case VTSS_MEP_RC_PEER_MAC:          return MEP_RC_PEER_MAC;
    case VTSS_MEP_RC_INVALID_INSTANCE:  return MEP_RC_INVALID_INSTANCE;
    case VTSS_MEP_RC_INVALID_MEG:       return MEP_RC_INVALID_MEG;
    case VTSS_MEP_RC_PROP_SUPPORT:      return MEP_RC_PROP_SUPPORT;
    case VTSS_MEP_RC_VOLATILE:          return MEP_RC_VOLATILE;
    case VTSS_MEP_RC_VLAN_SUPPORT:      return MEP_RC_VLAN_SUPPORT;
    case VTSS_MEP_RC_CLIENT_MAX_LEVEL:  return MEP_RC_CLIENT_MAX_LEVEL;
    case VTSS_MEP_RC_MIP_SUPPORT:       return MEP_RC_MIP_SUPPORT;
    default:                            return MEP_RC_INVALID_PARAMETER;
    }
}

/****************************************************************************/
/*  MEP management interface                                                */
/****************************************************************************/
void mep_mgmt_def_conf_get(vtss_mep_mgmt_def_conf_t *const def_conf)
{
    vtss_mep_mgmt_def_conf_get(def_conf);
}

vtss_rc mep_mgmt_conf_set(const u32                  instance,
                          const vtss_mep_mgmt_conf_t *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)      return MEP_RC_INVALID_INSTANCE;

    if ((rc = rc_conv(vtss_mep_mgmt_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            if (conf->enable)        blk->conf[instance] = *conf;
            else
            {
                blk->conf[instance].enable = FALSE;
                blk->cc_conf[instance].enable = FALSE;
                blk->lm_conf[instance].enable = FALSE;
                blk->dm_conf[instance].enable = FALSE;
                blk->aps_conf[instance].enable = FALSE;
            }
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        CRIT_ENTER(crit_p);
        instance_data[instance].enable = conf->enable;
        if (conf->enable)
        {
            instance_data[instance].domain = conf->domain;
            instance_data[instance].flow = conf->flow;
            instance_data[instance].res_port = conf->port;
        }
        else
            instance_data[instance].aps_type = VTSS_MEP_MGMT_INV_APS;
        CRIT_EXIT(crit_p);
    }

    return rc;
}

vtss_rc mep_mgmt_volatile_conf_set(const u32                  instance,
                                   const vtss_mep_mgmt_conf_t *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)      return MEP_RC_INVALID_INSTANCE;

    if ((rc = rc_conv(vtss_mep_mgmt_volatile_conf_set(instance, conf))) == VTSS_RC_OK)
    {
        CRIT_ENTER(crit_p);
        instance_data[instance].enable = conf->enable;
        if (conf->enable)
        {
            instance_data[instance].domain = conf->domain;
            instance_data[instance].flow = conf->flow;
            instance_data[instance].res_port = conf->port;
        }
        else
            instance_data[instance].aps_type = VTSS_MEP_MGMT_INV_APS;
        CRIT_EXIT(crit_p);
    }

    return rc;
}

vtss_rc mep_mgmt_conf_get(const u32               instance,
                          u8                      *const mac,
                          u32                     *const eps_count,
                          u16                     *const eps_inst,
                          vtss_mep_mgmt_conf_t    *const conf)
{
    u32 i, inx, aps_count = 0;

    if (instance >= MEP_INSTANCE_MAX)      return(MEP_RC_INVALID_INSTANCE);

    if (eps_count || eps_inst) {
        CRIT_ENTER(crit_p);
        if (instance_data[instance].aps_inst != APS_INST_INV) {
            if (eps_inst) {
                eps_inst[0] = instance_data[instance].aps_inst;   /* APS EPS instance number is first */
            }
            aps_count = 1;
        }
        for (i = 0, inx = aps_count; i < instance_data[instance].ssf_count; i++) /* SSF EPS instance numbers - but not if also APS instance */ {
            if ((aps_count == 0) || (instance_data[instance].ssf_inst[i] != instance_data[instance].aps_inst)) {
                if (eps_inst) {
                    eps_inst[inx] = instance_data[instance].ssf_inst[i];
                }
                inx++;
            }
        }
        if (eps_count) {
            *eps_count = inx;
        }
        CRIT_EXIT(crit_p);
    }

    return rc_conv(vtss_mep_mgmt_conf_get(instance, mac, conf));
}

vtss_rc mep_mgmt_pm_conf_set(const u32                     instance,
                             const vtss_mep_mgmt_pm_conf_t *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)      return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_pm_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->pm_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_pm_conf_get(const u32                 instance,
                             vtss_mep_mgmt_pm_conf_t   *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_pm_conf_get(instance, conf));
}

vtss_rc mep_mgmt_cc_conf_set(const u32                        instance,
                             const vtss_mep_mgmt_cc_conf_t    *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)      return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_cc_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->cc_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_cc_conf_get(const u32                 instance,
                         vtss_mep_mgmt_cc_conf_t   *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_cc_conf_get(instance, conf));
}

vtss_rc mep_mgmt_lm_conf_set(const u32                      instance,
                             const vtss_mep_mgmt_lm_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_lm_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->lm_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_lm_conf_get(const u32                instance,
                             vtss_mep_mgmt_lm_conf_t  *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lm_conf_get(instance, conf));
}

vtss_rc mep_mgmt_dm_conf_set(const u32                      instance,
                             const vtss_mep_mgmt_dm_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_dm_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->dm_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_dm_conf_get(const u32                instance,
                             vtss_mep_mgmt_dm_conf_t  *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_dm_conf_get(instance, conf));
}

vtss_rc mep_mgmt_aps_conf_set(const u32                         instance,
                              const vtss_mep_mgmt_aps_conf_t    *const conf)
{
    vtss_rc rc;
    BOOL    invalid = FALSE;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (conf->enable && (((conf->type == VTSS_MEP_MGMT_L_APS) && (instance_data[instance].eps_type == MEP_EPS_TYPE_ERPS)) ||
                         ((conf->type == VTSS_MEP_MGMT_R_APS) && (instance_data[instance].eps_type == MEP_EPS_TYPE_ELPS))))
        invalid = TRUE;
    CRIT_EXIT(crit_p);

    if (invalid)        return MEP_RC_APS_PROT_CONNECTED;

    if ((rc = rc_conv(vtss_mep_mgmt_aps_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->aps_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        CRIT_ENTER(crit_p);
        instance_data[instance].aps_type = (conf->enable) ? conf->type : VTSS_MEP_MGMT_INV_APS;
        CRIT_EXIT(crit_p);
    } else {
        T_D("inst %u rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_aps_conf_get(const u32                  instance,
                              vtss_mep_mgmt_aps_conf_t   *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    rc = rc_conv(vtss_mep_mgmt_aps_conf_get(instance, conf));

    if (!conf->enable)  conf->raps_octet = 1;   /* This is just to give the value '1' as a default value to the "manager" */

    return rc;
}

vtss_rc mep_mgmt_lt_conf_set(const u32                      instance,
                             const vtss_mep_mgmt_lt_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_lt_conf_set(instance, conf))) != VTSS_RC_OK) {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_lt_conf_get(const u32                 instance,
                             vtss_mep_mgmt_lt_conf_t   *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lt_conf_get(instance, conf));
}

vtss_rc mep_mgmt_lb_conf_set(const u32                      instance,
                             const vtss_mep_mgmt_lb_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_lb_conf_set(instance, conf))) != VTSS_RC_OK) {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_lb_conf_get(const u32                 instance,
                             vtss_mep_mgmt_lb_conf_t   *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lb_conf_get(instance, conf));
}

vtss_rc mep_mgmt_ais_conf_set(const u32                      instance,
                              const vtss_mep_mgmt_ais_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_ais_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->ais_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    return rc;
}

vtss_rc mep_mgmt_ais_conf_get(const u32                 instance,
                              vtss_mep_mgmt_ais_conf_t  *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_ais_conf_get(instance,conf));
}

vtss_rc mep_mgmt_lck_conf_set(const u32                      instance,
                              const vtss_mep_mgmt_lck_conf_t  *const conf)
{
    vtss_rc rc;


    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_lck_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->lck_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    return rc;
}

vtss_rc mep_mgmt_lck_conf_get(const u32                 instance,
                              vtss_mep_mgmt_lck_conf_t  *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lck_conf_get(instance,conf));
}


vtss_rc mep_mgmt_tst_conf_set(const u32                       instance,
                              const vtss_mep_mgmt_tst_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_tst_conf_set(instance, conf))) != VTSS_RC_OK) {
        T_D("inst %u  rc %u", instance, rc);
    }

    return rc;
}

vtss_rc mep_mgmt_tst_conf_get(const u32                  instance,
                              vtss_mep_mgmt_tst_conf_t   *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_tst_conf_get(instance, conf));
}

vtss_rc mep_mgmt_client_conf_set(const u32                          instance,
                                 const vtss_mep_mgmt_client_conf_t  *const conf)
{
    vtss_rc rc;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    if ((rc = rc_conv(vtss_mep_mgmt_client_conf_set(instance, conf))) == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->client_conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    return rc;
}

vtss_rc mep_mgmt_client_conf_get(const u32                    instance,
                                 vtss_mep_mgmt_client_conf_t  *const conf)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_client_conf_get(instance, conf));
}

vtss_rc mep_mgmt_lt_state_get(const u32                  instance,
                              vtss_mep_mgmt_lt_state_t   *const state)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lt_state_get(instance, state));
}

vtss_rc mep_mgmt_lb_state_get(const u32                  instance,
                          vtss_mep_mgmt_lb_state_t   *const state)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lb_state_get(instance, state));
}

vtss_rc mep_mgmt_state_get(const u32              instance,
                           vtss_mep_mgmt_state_t  *const state)
{
    if (instance >= MEP_INSTANCE_MAX)       return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_state_get(instance, state));
}

vtss_rc mep_mgmt_lm_state_get(const u32                   instance,
                              vtss_mep_mgmt_lm_state_t    *const state)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lm_state_get(instance, state));
}

vtss_rc mep_mgmt_lm_state_clear_set(const u32    instance)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_lm_state_clear_set(instance));
}

vtss_rc mep_mgmt_dm_state_get(const u32                   instance,
                              vtss_mep_mgmt_dm_state_t    *const dmr_state,
                              vtss_mep_mgmt_dm_state_t    *const dm1_state_far_to_near,
                              vtss_mep_mgmt_dm_state_t    *const dm1_state_near_to_far)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_dm_state_get(instance, dmr_state, dm1_state_far_to_near, dm1_state_near_to_far));
}

vtss_rc mep_mgmt_dm_db_state_get(const u32                   instance,
                                 u32                         *delay,
                                 u32                         *delay_var)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_dm_db_state_get(instance, delay, delay_var));
}

vtss_rc mep_mgmt_dm_timestamp_get(const u32                    instance,
                                  vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_far_to_near,
                                  vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_near_to_far)
{
    return rc_conv(vtss_mep_mgmt_dm_timestamp_get(instance, dm1_timestamp_far_to_near, dm1_timestamp_near_to_far));
}

vtss_rc mep_mgmt_dm_state_clear_set(const u32 instance)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_dm_state_clear_set(instance));
}

vtss_rc mep_mgmt_tst_state_get(const u32                    instance,
                               vtss_mep_mgmt_tst_state_t    *const state)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_tst_state_get(instance, state));
}

vtss_rc mep_mgmt_tst_state_clear_set(const u32 instance)
{
    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    return rc_conv(vtss_mep_mgmt_tst_state_clear_set(instance));
}

#if defined(VTSS_SW_OPTION_UP_MEP)
void mep_mgmt_up_mep_enable(BOOL  enable)
{

    if (enable)     vtss_mep_mgmt_up_mep_enable();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        ulong       size;
        mep_conf_t  *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
            blk->up_mep_enabled = enable;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}
#endif
/****************************************************************************/
/*  MEP module interface - call out                                         */
/****************************************************************************/
void vtss_mep_rx_aps_info_set(const u32                      instance,
                              const vtss_mep_mgmt_domain_t   domain,
                              const u8                       *aps)
{
#if defined(VTSS_SW_OPTION_EPS) || defined(VTSS_SW_OPTION_ERPS)
    u32            eps = 0;
    mep_eps_type_t eps_type = MEP_EPS_TYPE_INV;
    vtss_rc        rc = VTSS_RC_OK;

    if (instance >= MEP_INSTANCE_MAX)        return;

    CRIT_ENTER(crit_p);
    if (instance_data[instance].eps_type != MEP_EPS_TYPE_INV) {
        eps_type = instance_data[instance].eps_type;
    } else {
        rc = MEP_RC_INVALID_PARAMETER;
    }
    if (instance_data[instance].aps_inst != APS_INST_INV) {
        eps = instance_data[instance].aps_inst;
    } else  {
        rc = MEP_RC_INVALID_PARAMETER;
    }
    CRIT_EXIT(crit_p);

//T_D("rx aps  instance %u  %X %X\n",instance, aps[0], aps[1]);
    if (rc == VTSS_RC_OK) {
#if defined(VTSS_SW_OPTION_EPS)
        if (eps_type == MEP_EPS_TYPE_ELPS) {
            if (eps_rx_aps_info_set(eps, instance, aps) != EPS_RC_OK) {
                T_D("Error during APS rx set %u", instance);
            }
        }
#endif
#if defined(VTSS_SW_OPTION_ERPS)
        if (eps_type == MEP_EPS_TYPE_ERPS) {
            erps_handover_raps_pdu(aps, instance, VTSS_MEP_SUPP_RAPS_DATA_LENGTH, eps);
        }
#endif
    }
#endif
}

void vtss_mep_signal_out(const u32                       instance,
                         const vtss_mep_mgmt_domain_t    domain)
{
    u32  i;
    BOOL los=FALSE;

    CRIT_ENTER(crit_p);
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)  /* Calculate SSF for this mep instance based on los state */
        if (instance_data[instance].port[i])    /* Check if this port(i) is related to this MEP */
        {
            los = ((domain == VTSS_MEP_MGMT_EVC) || (domain == VTSS_MEP_MGMT_VLAN)) ? conv_los_state[i] : los_state[i];
            if (!los)   break;      /* Only report los active (SSF) if all related ports are failing */
        }

#if defined(VTSS_SW_OPTION_EPS)
    u32              eps=0, forward, raps_tx;
    u8               aps_txdata[VTSS_MEP_APS_DATA_LENGTH];
    mep_eps_type_t   eps_type=MEP_EPS_TYPE_INV;
    vtss_rc          rc = VTSS_RC_OK;

    memcpy(aps_txdata, instance_data[instance].aps_txdata, sizeof(aps_txdata));
    if (instance_data[instance].eps_type == MEP_EPS_TYPE_ERPS)
    {
        forward = instance_data[instance].raps_forward ? 2 : 1;
        raps_tx = instance_data[instance].raps_tx ? 2 : 1;
    }
    else
    {
        forward = 0;
        raps_tx = 0;
    }

    if (instance_data[instance].eps_type != MEP_EPS_TYPE_INV)    eps_type = instance_data[instance].eps_type;
    else    rc = MEP_RC_INVALID_PARAMETER;
    if (instance_data[instance].aps_inst != APS_INST_INV)        eps = instance_data[instance].aps_inst;
    else    rc = MEP_RC_INVALID_PARAMETER;
#endif
    CRIT_EXIT(crit_p);

    vtss_mep_new_ssf_state(instance,  los);

#if defined(VTSS_SW_OPTION_EPS)
    if (raps_tx)
    {
        if (vtss_mep_tx_aps_info_set(instance, aps_txdata, FALSE))    T_D("Error during configuration");
        if (vtss_mep_raps_transmission(instance, (raps_tx == 2)))     T_D("Error during configuration");
    }

    if (forward)        if (vtss_mep_raps_forwarding(instance, (forward == 2)))     T_D("Error during configuration");

    if (rc == VTSS_RC_OK) {
        if (eps_type == MEP_EPS_TYPE_ELPS) {
            if (eps_signal_in(eps, instance) != EPS_RC_OK) {
                T_D("Error during EPS signal %u", instance);
            }
        }

        if (eps_type == MEP_EPS_TYPE_ERPS) {
        }
    }
#endif
}

void vtss_mep_sf_sd_state_set(const u32                      instance,
                              const vtss_mep_mgmt_domain_t   domain,
                              const BOOL                     sf_state,
                              const BOOL                     sd_state)
{
#if defined(VTSS_SW_OPTION_EPS) || defined(VTSS_SW_OPTION_ERPS)
    u32              i;
    u32              eps_rc = EPS_RC_OK;
    u32              eps_count;
    u16              eps_inst[SF_EPS_MAX];
    mep_eps_type_t   eps_type=MEP_EPS_TYPE_INV;
    vtss_rc          erps_rc = VTSS_RC_OK;

    CRIT_ENTER(crit_p);
    eps_type = instance_data[instance].eps_type;
    eps_count = instance_data[instance].ssf_count;
    memcpy(eps_inst, instance_data[instance].ssf_inst, sizeof(eps_inst));
    CRIT_EXIT(crit_p);

#if defined(VTSS_SW_OPTION_EPS)
    if (eps_type == MEP_EPS_TYPE_ELPS)
    {
        for (i=0; i<eps_count; ++i)
            eps_rc += eps_sf_sd_state_set(eps_inst[i], instance, sf_state, sd_state);
    }
#endif
#if defined(VTSS_SW_OPTION_ERPS)
    if (eps_type == MEP_EPS_TYPE_ERPS) {
        for (i=0; i<eps_count; ++i) {
            erps_rc += erps_sf_sd_state_set(eps_inst[i], instance, sf_state, sd_state);
        }
    }
#endif
    if (eps_rc != EPS_RC_OK || erps_rc != VTSS_RC_OK) {
        T_D("Error during SF/SD set %u", instance);
    }
#endif
}

/****************************************************************************/
/*  MEP module interface - call in                                          */
/****************************************************************************/
vtss_rc mep_eps_aps_register(const u32            instance,
                             const u32            eps_inst,
                             const mep_eps_type_t eps_type,
                             const BOOL           enable)
{
    BOOL invalid = FALSE;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (enable && instance_data[instance].aps_inst != APS_INST_INV)
        invalid = TRUE;
    if (!enable && instance_data[instance].aps_inst != eps_inst)
        invalid = TRUE;
    if (enable && (((instance_data[instance].aps_type == VTSS_MEP_MGMT_L_APS) && (eps_type == MEP_EPS_TYPE_ERPS)) ||
                   ((instance_data[instance].aps_type == VTSS_MEP_MGMT_R_APS) && (eps_type == MEP_EPS_TYPE_ELPS))))
        invalid = TRUE;

    if (!invalid)
    {
        if (enable)
        {/* Add EPS as APS instance */
            instance_data[instance].aps_inst = eps_inst;
            instance_data[instance].eps_type = eps_type;
        }
        else
        {/* Remove EPS as APS instance */
            instance_data[instance].aps_inst = APS_INST_INV;
            if (instance_data[instance].ssf_count == 0)
                instance_data[instance].eps_type = MEP_EPS_TYPE_INV;    /* No EPS Related */
        }
    }
    CRIT_EXIT(crit_p);

    return (invalid ? (vtss_rc)MEP_RC_INVALID_PARAMETER : VTSS_RC_OK);
}


vtss_rc mep_eps_sf_register(const u32            instance,
                            const u32            eps_inst,
                            const mep_eps_type_t eps_type,
                            const BOOL           enable)
{
    u32 i;
    BOOL invalid = FALSE;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (enable && (((instance_data[instance].aps_type == VTSS_MEP_MGMT_L_APS) && (eps_type == MEP_EPS_TYPE_ERPS)) ||
                   ((instance_data[instance].aps_type == VTSS_MEP_MGMT_R_APS) && (eps_type == MEP_EPS_TYPE_ELPS))))
        invalid = TRUE;
    if (enable && instance_data[instance].ssf_count >= SF_EPS_MAX)
        invalid = TRUE;
    for (i=0; i<instance_data[instance].ssf_count; ++i)     if (instance_data[instance].ssf_inst[i] == eps_inst)     break;
    if (enable && (i < instance_data[instance].ssf_count))      invalid = TRUE;
    if (!enable && (i == instance_data[instance].ssf_count))    invalid = TRUE;

    if (!invalid)
    {
        if (enable)
        {/* Add EPS as SSF instance */
            instance_data[instance].ssf_inst[i] = eps_inst;
            instance_data[instance].ssf_count++;
            instance_data[instance].eps_type = eps_type;
        }
        else
        {/* Remove EPS as SSF instance */
            for (i=i; i<(instance_data[instance].ssf_count-1); ++i)
                instance_data[instance].ssf_inst[i] = instance_data[instance].ssf_inst[i+1];
            if (instance_data[instance].ssf_count)  instance_data[instance].ssf_count--;

            if ((instance_data[instance].ssf_count == 0) && (instance_data[instance].aps_inst == APS_INST_INV))
                instance_data[instance].eps_type = MEP_EPS_TYPE_INV;    /* No EPS Related */
        }
    }
    CRIT_EXIT(crit_p);

    return (invalid ? (vtss_rc)MEP_RC_INVALID_PARAMETER : VTSS_RC_OK);
}

vtss_rc mep_tx_aps_info_set(const u32  instance,
                            const u32  eps_inst,
                            const u8   *aps,
                            const BOOL event)
{
    vtss_rc rc = VTSS_RC_OK;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);
    CRIT_ENTER(crit_p);
    if (instance_data[instance].eps_type == MEP_EPS_TYPE_INV)    rc = MEP_RC_INVALID_PARAMETER;
    if ((instance_data[instance].aps_inst == APS_INST_INV) || (instance_data[instance].aps_inst != eps_inst))    rc = MEP_RC_INVALID_PARAMETER;
//    if (rc==VTSS_RC_OK)   memcpy(instance_data[instance].aps_txdata, aps, sizeof(aps));
    if (rc == VTSS_RC_OK)   memcpy(instance_data[instance].aps_txdata, aps, (instance_data[instance].eps_type == MEP_EPS_TYPE_ERPS) ? VTSS_MEP_SUPP_RAPS_DATA_LENGTH : VTSS_MEP_SUPP_LAPS_DATA_LENGTH);
    CRIT_EXIT(crit_p);
//T_D("tx aps  instance %lu  %X %X\n",instance, aps[0], aps[1]);

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return rc_conv(vtss_mep_tx_aps_info_set(instance, aps, event));
}


vtss_rc mep_signal_in(const u32 instance,
                      const u32 eps_inst)
{
    u32     i;
    vtss_rc rc = VTSS_RC_OK;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (instance_data[instance].eps_type == MEP_EPS_TYPE_INV)    rc = MEP_RC_INVALID_PARAMETER;
    for (i=0; i<instance_data[instance].ssf_count; ++i)
        if (instance_data[instance].ssf_inst[i] == eps_inst)     break;
    if ((i==instance_data[instance].ssf_count) &&
        ((instance_data[instance].aps_inst == APS_INST_INV) || (instance_data[instance].aps_inst != eps_inst)))    rc = MEP_RC_INVALID_PARAMETER;
    CRIT_EXIT(crit_p);

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return rc_conv(vtss_mep_signal_in(instance));
}


vtss_rc mep_raps_forwarding(const u32  instance,
                            const u32  eps_inst,
                            const BOOL enable)
{
    vtss_rc rc = VTSS_RC_OK;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (instance_data[instance].eps_type != MEP_EPS_TYPE_ERPS)    rc = MEP_RC_INVALID_PARAMETER;
    if ((instance_data[instance].aps_inst == APS_INST_INV) || (instance_data[instance].aps_inst != eps_inst))    rc = MEP_RC_INVALID_PARAMETER;
    if (rc == VTSS_RC_OK)   instance_data[instance].raps_forward = enable;
    CRIT_EXIT(crit_p);

//T_D("forward enable  instance %lu  enable %u\n", instance, enable);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return rc_conv(vtss_mep_raps_forwarding(instance, enable));
}


vtss_rc mep_raps_transmission(const u32  instance,
                              const u32  eps_inst,
                              const BOOL enable)
{
    vtss_rc rc = VTSS_RC_OK;

    if (instance >= MEP_INSTANCE_MAX)        return(MEP_RC_INVALID_INSTANCE);

    CRIT_ENTER(crit_p);
    if (instance_data[instance].eps_type != MEP_EPS_TYPE_ERPS)    rc = MEP_RC_INVALID_PARAMETER;
    if ((instance_data[instance].aps_inst == APS_INST_INV) || (instance_data[instance].aps_inst != eps_inst))    rc = MEP_RC_INVALID_PARAMETER;
    if (rc == VTSS_RC_OK)   instance_data[instance].raps_tx = enable;
    CRIT_EXIT(crit_p);

//T_D("tx enable  instance %lu  enable %u\n", instance, enable);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return rc_conv(vtss_mep_raps_transmission(instance, enable));
}

/*lint -e{454, 455, 456} ... The mutex is locked so it is ok to unlock */
void mep_port_protection_changed_evc_mep(u32 port, BOOL sw)
{
    u32 i, rc;

    for(i=0; i<MEP_INSTANCE_MAX; ++i) {
        if ((instance_data[i].enable) && ((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN)) && (instance_data[i].port[port])) { /* Check if an EVC or VLAN on this port  */
            CRIT_EXIT(crit_p);
            if (sw)
                rc = vtss_mep_mgmt_protection_change_set(i);    /* This MEP needs to reconfigure depending on port protection */
            else
                rc = vtss_mep_mgmt_change_set(i);   /* This MEP needs to reconfigure depending on port protection */
            CRIT_ENTER(crit_p);
            if (rc != VTSS_MEP_RC_OK)   continue;
        }
    }
}

static u32 working_port_get(const u32 port)    /* Port can be either working or protecting - or a port not part of port protection */
{
    u32 i;

    if (in_port_conv[port] == port)   /* This port is either a working port or a port not part of port protection */
        return(port);
    if (out_port_conv[port] != port)  /* This port is a protected port (working) */
        return(port);
    if (in_port_conv[port] != 0xFFFF) /* This port is a active protecting port */
        return(in_port_conv[port]);
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)  /* This is a NOT active protecting port - find working */
        if ((i != port) && (out_port_conv[i] == port))      break;
    if (i < VTSS_PORT_ARRAY_SIZE)
        return(i);                    /* The protected port was found */
    return(port);   /* This should not happen */
}

static u32 port_protection_mask_calc(u32 port)
{
    u32 mask, w_port, p_port;

    w_port = working_port_get(port);
    p_port = out_port_conv[w_port];
    mask = 0;

    if (p_port != w_port) {  /* This is a protected port */
        if (out_port_1p1[w_port])  /* This is 1+1 - mask is both working and protecting port */
            mask = (1<<w_port) | (1<<p_port);
        else { /* This is 1:1 - mask is only active port */
            if (in_port_conv[w_port] == w_port) /* Working is active */
                mask = (1<<w_port);
            else if (in_port_conv[p_port] == w_port) /* protecting is active */
                mask = (1<<p_port);
        }
    }
    else    /* This is NOT a protected port */
        mask = 1<<w_port;

    return(mask);
}

void mep_port_protection_change(const u32         w_port,
                                const u32         p_port,
                                const BOOL        active)
{
    BOOL change = FALSE;

    CRIT_ENTER(crit_p);
    /* Change in port protection selector state */
    if ((active && (in_port_conv[w_port] == w_port)) || (!active && (in_port_conv[w_port] == 0xFFFF)))  /* Protection state changed */
        change = TRUE;

    if (!active)
    {/* Port protection is no more active */
/*T_D("1 port %lu", port);*/
        in_port_conv[w_port] = w_port;
        if (in_port_conv[p_port] == w_port)     in_port_conv[p_port] = 0xFFFF;        /* p_port is protecting this w_port - Discard on p_port port */
        prot_state_change(w_port);
    }
    else
    {/* Port protection is active */
/*T_D("2 port %lu", port);*/
        if (in_port_conv[p_port] == 0xFFFF)
        {/* p_port is not already protecting other w_port */
            in_port_conv[w_port] = 0xFFFF;        /* Discard on w_port port */
            in_port_conv[p_port] = w_port;
            prot_state_change(p_port);
        }
    }

    if (change)     /* Signal any change to the MEP base part - it might be necessary to do configuration change */
        mep_port_protection_changed_evc_mep(w_port, TRUE);
//printf("mep_port_protection_change  in_port_conv %lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX\n", in_port_conv[0], in_port_conv[1], in_port_conv[2], in_port_conv[3], in_port_conv[4], in_port_conv[5], in_port_conv[6], in_port_conv[7], in_port_conv[8], in_port_conv[9], in_port_conv[10], in_port_conv[11]);
    CRIT_EXIT(crit_p);
}

void mep_port_protection_create(mep_eps_architecture_t architecture,  const u32 w_port,  const u32 p_port)
{
    CRIT_ENTER(crit_p);
    in_port_conv[w_port] = w_port;
    if (in_port_conv[p_port] == p_port)            in_port_conv[p_port] = 0xFFFF;       /* Not already protecting some w_port - discard on protecting port */
    if (architecture == MEP_EPS_ARCHITECTURE_1P1)  out_port_1p1[w_port] = TRUE;         /* This is a 1+1 port protection */

    if (out_port_conv[w_port] != p_port) { /* Protection changed */
        out_port_conv[w_port] = p_port;
        mep_port_protection_changed_evc_mep(w_port, FALSE);
    }
//printf("mep_port_protection_create  in_port_conv %lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX-%lX\n", in_port_conv[0], in_port_conv[1], in_port_conv[2], in_port_conv[3], in_port_conv[4], in_port_conv[5], in_port_conv[6], in_port_conv[7], in_port_conv[8], in_port_conv[9], in_port_conv[10], in_port_conv[11]);
    CRIT_EXIT(crit_p);
}

void mep_port_protection_delete(const u32 w_port,  const u32 p_port)
{
    u32 i;

    CRIT_ENTER(crit_p);
    /* Check for other that got the same p_port */
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)
        if ((i != w_port) && (out_port_conv[i] == p_port))      break;
    if (i == VTSS_PORT_ARRAY_SIZE)
    {/* No Other ESP got this p_port as protection port */
        in_port_conv[p_port] = p_port;
        conv_los_state[p_port] = los_state[p_port];
    }

    prot_state_change(w_port);      /* Working is now active - New SF state to MEP */

    in_port_conv[w_port] = w_port;
    out_port_conv[w_port] = w_port;
    conv_los_state[w_port] = los_state[w_port];

    mep_port_protection_changed_evc_mep(w_port, FALSE);
    CRIT_EXIT(crit_p);
}

void mep_ring_protection_block(const u32 port,  BOOL block)
{
    T_D("mep_ring_protection_block port %u  block %u\n", port, block);

    u32 i, rc;

    CRIT_ENTER(crit_p);

    for(i=0; i<MEP_INSTANCE_MAX; ++i) {
        if ((instance_data[i].enable) && ((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN)) && (instance_data[i].port[port])) { /* Check if an EVC or VLAN on this port  */
            CRIT_EXIT(crit_p);
            rc = vtss_mep_mgmt_protection_change_set(i);    /* This MEP needs to reconfigure depending on ring protection */
            CRIT_ENTER(crit_p);
            if (rc != VTSS_MEP_RC_OK)   continue;
        }
    }

    CRIT_EXIT(crit_p);
}





/****************************************************************************/
/*  MEP platform interface                                                  */
/****************************************************************************/
void vtss_mep_crit_lock(void)
{
    CRIT_ENTER(crit);
}

void vtss_mep_crit_unlock(void)
{
    CRIT_EXIT(crit);
}

void vtss_mep_run(void)
{
    cyg_semaphore_post(&run_wait_sem);
}

void vtss_mep_timer_start(void)
{
    cyg_flag_setbits(&timer_wait_flag, FLAG_TIMER);
}

void vtss_mep_trace(const char  *string,
                    const u32   param1,
                    const u32   param2,
                    const u32   param3,
                    const u32   param4)
{
    T_D("%s - %d, %d, %d, %d", string, param1, param2, param3, param4);
}


void vtss_mep_mac_get(u32 port,  u8 mac[VTSS_MEP_MAC_LENGTH])
{
    if (conf_mgmt_mac_addr_get(mac, port+1) < 0)
        T_W("Error getting own MAC:\n");
}

BOOL vtss_mep_pvid_get(u32 port, u32 *pvid, vtss_mep_tag_type_t *ptype)
{
    vlan_port_conf_t  vlan_conf;

    if (port >= VTSS_PORT_ARRAY_SIZE) {
        // You cannot call vlan_mgmt_port_conf_get() with port >= VTSS_PORT_ARRAY_SIZE.
        T_E("Invalid port number (%u)", port);
        return FALSE;
    }

   if (vlan_mgmt_port_conf_get(VTSS_ISID_START,  port,  &vlan_conf, VLAN_USER_ALL) != VTSS_RC_OK)    return FALSE;

   switch (vlan_conf.port_type) {
        case VLAN_PORT_TYPE_C:              *ptype = VTSS_MEP_TAG_C; break;
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        case VLAN_PORT_TYPE_S:              *ptype = VTSS_MEP_TAG_S; break;
        case VLAN_PORT_TYPE_S_CUSTOM:       *ptype = VTSS_MEP_TAG_S_CUSTOM; break;
#endif
        default:                            *ptype = VTSS_MEP_TAG_NONE;
    }
    *pvid = vlan_conf.pvid;

    return(TRUE);
}

BOOL vtss_mep_vlan_get(const u32 vid,   BOOL nni[VTSS_PORT_ARRAY_SIZE])
{
    vlan_mgmt_entry_t conf;
    vtss_port_no_t    port_no;

    if (vlan_mgmt_vlan_get(VTSS_ISID_START, vid, &conf, FALSE, VLAN_USER_ALL) != VTSS_RC_OK)     return(FALSE);

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        nni[port_no] = (conf.ports[port_no] == 1) ? TRUE : FALSE;
    }

    return(TRUE);
}

BOOL vtss_mep_port_counters_get(u32 instance,  u32 port,  vtss_port_counters_t *counters)
{
    if (((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) || (instance_data[instance].domain == VTSS_MEP_MGMT_VLAN)) && (in_port_conv[port] == 0xFFFF))
        return (vtss_port_counters_get(NULL, out_port_conv[port], counters) != VTSS_RC_OK);
    return (vtss_port_counters_get(NULL, port, counters) != VTSS_RC_OK);
}

u32 vtss_mep_port_count(void)
{
    return(port_isid_port_count(VTSS_ISID_LOCAL));
}

#if defined(VTSS_ARCH_JAGUAR_1)
BOOL vtss_mep_evc_counters_get(u32 instance,  u32 evc,  u32 port,  vtss_evc_counters_t *counters)
{
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) && (in_port_conv[port] == 0xFFFF))
        return (vtss_evc_counters_get(NULL, evc, out_port_conv[port], counters) != VTSS_RC_OK);
    return (vtss_evc_counters_get(NULL, evc, port, counters) != VTSS_RC_OK);
}
#endif

#if defined(VTSS_SW_OPTION_EVC)
BOOL vtss_mep_evc_flow_info_get(const u32                  evc_inst,
                                BOOL                       nni[VTSS_PORT_ARRAY_SIZE],
                                BOOL                       uni[VTSS_PORT_ARRAY_SIZE],
                                u32                        *const vid,
                                u32                        *const evid,
                                vtss_mep_tag_type_t        *const ttype,
                                u32                        *const tvid,
                                BOOL                       *const tunnel,
                                u32                        *const tag_cnt)
{
    vtss_evc_id_t       evc_id;
    evc_mgmt_conf_t     conf;
    evc_mgmt_ece_conf_t ece_conf;
    u32                 i;

    evc_id = evc_inst;

    if (evc_mgmt_get(&evc_id, &conf, FALSE) != VTSS_RC_OK) return(FALSE);
#if defined(VTSS_FEATURE_MPLS)
    if (VTSS_MPLS_IDX_IS_DEF(conf.conf.network.mpls_tp.pw_egress_xc) || VTSS_MPLS_IDX_IS_DEF(conf.conf.network.mpls_tp.pw_ingress_xc))
        return(FALSE);
#endif

    *vid = conf.conf.network.pb.ivid; /* EVC internal VID - classified VID after IS1 */
    *evid = conf.conf.network.pb.vid; /* EVC outer VID - port classification VID */
    *ttype = VTSS_MEP_TAG_NONE;
    *tvid = 0;
    *tunnel = FALSE;
    *tag_cnt = 1;
#if defined(VTSS_ARCH_LUTON26)
    *tag_cnt = (conf.conf.network.pb.inner_tag.type == VTSS_EVC_INNER_TAG_NONE) ? 1 : 2;
    *tunnel = conf.conf.network.pb.inner_tag.vid_mode == VTSS_EVC_VID_MODE_TUNNEL;
    *tvid = conf.conf.network.pb.inner_tag.vid;
    switch (conf.conf.network.pb.inner_tag.type)
    {
        case VTSS_EVC_INNER_TAG_NONE:     *ttype = VTSS_MEP_TAG_NONE; break;
        case VTSS_EVC_INNER_TAG_C:        *ttype = VTSS_MEP_TAG_C; break;
        case VTSS_EVC_INNER_TAG_S:        *ttype = VTSS_MEP_TAG_S; break;
        case VTSS_EVC_INNER_TAG_S_CUSTOM: *ttype = VTSS_MEP_TAG_S_CUSTOM; break;
        default:                          *ttype = VTSS_MEP_TAG_NONE;
    }
#endif

    memset(uni, 0, (VTSS_PORT_ARRAY_SIZE*sizeof(BOOL)));
    memcpy(nni, conf.conf.network.pb.nni, (VTSS_PORT_ARRAY_SIZE*sizeof(BOOL)));

    ece_conf.conf.id = EVC_ECE_ID_FIRST;    /* UNI for all ECE is collected */
    while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, 1) == VTSS_RC_OK) {
        if (ece_conf.conf.action.evc_id == evc_id) {
            for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)
                if (ece_conf.conf.key.port_list[i])     uni[i] = TRUE;
        }
    }

    return(TRUE);
}

#if defined(VTSS_ARCH_SERVAL)
void vtss_mep_evc_cos_id_get(const u32  evc_inst,
                             BOOL       cos_id[VTSS_MEP_SUPP_COS_ID_SIZE])
{
    vtss_evc_id_t       evc_id;
    evc_mgmt_ece_conf_t ece_conf;

    evc_id = evc_inst;

    memset(cos_id, 0, (VTSS_MEP_SUPP_COS_ID_SIZE*sizeof(BOOL)));

    ece_conf.conf.id = EVC_ECE_ID_FIRST;    /* COS-ID for all ECE is collected */
    while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, 1) == VTSS_RC_OK) {
        if ((ece_conf.conf.action.evc_id == evc_id) && (ece_conf.conf.action.dir != VTSS_ECE_DIR_NNI_TO_UNI) && (ece_conf.conf.action.prio_enable))
                if (ece_conf.conf.action.prio < VTSS_MEP_SUPP_COS_ID_SIZE)
                    cos_id[ece_conf.conf.action.prio] = TRUE;
    }
}

void vtss_mep_supp_evc_mce_info_get(const u32                          evc_inst,
                                    vtss_mep_supp_evc_nni_type_t       *nni_type,
                                    vtss_mep_supp_evc_mce_outer_tag_t  nni_outer_tag[VTSS_MEP_SUPP_COS_ID_SIZE],
                                    vtss_mep_supp_evc_mce_key_t        nni_key[VTSS_MEP_SUPP_COS_ID_SIZE])
{
    vtss_evc_id_t       evc_id;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_action_t   *action;

    evc_id = evc_inst;

    memset(nni_outer_tag, 0, (VTSS_MEP_SUPP_COS_ID_SIZE*sizeof(vtss_mep_supp_evc_mce_outer_tag_t)));
    memset(nni_key, 0, (VTSS_MEP_SUPP_COS_ID_SIZE*sizeof(vtss_mep_supp_evc_mce_key_t)));

    *nni_type = VTSS_MEP_SUPP_EVC_NNI_TYPE_E_NNI;
    ece_conf.conf.id = EVC_ECE_ID_FIRST;    /* COS-ID for all ECE is collected */
    while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, 1) == VTSS_RC_OK) {
        action = &ece_conf.conf.action;

        if ((action->evc_id == evc_id) && (action->dir != VTSS_ECE_DIR_NNI_TO_UNI) && (action->prio_enable)) {
        /* This ECE has priority classification enabled and it is NOT unidirectional from NNI to UNI. This is specifying PCP/DEI mapping from UNI to NNI on this COS-ID (prio) */
            if ((action->tx_lookup == VTSS_ECE_TX_LOOKUP_VID) && (action->outer_tag.pcp_mode == VTSS_ECE_PCP_MODE_MAPPED))
            /* If ECE TX lookup is VID based and PCP is mapped then it is a TN1135 I-NNI - NNI PCP mapping is based on port mapping */
                *nni_type = VTSS_MEP_SUPP_EVC_NNI_TYPE_I_NNI;
            if (action->prio < VTSS_MEP_SUPP_COS_ID_SIZE) {
                nni_outer_tag[action->prio].enable = TRUE;
                nni_outer_tag[action->prio].pcp_mode = (action->outer_tag.pcp_mode == VTSS_ECE_PCP_MODE_FIXED) ? VTSS_MCE_PCP_MODE_FIXED : VTSS_MCE_PCP_MODE_MAPPED;
                nni_outer_tag[action->prio].pcp = action->outer_tag.pcp;
                nni_outer_tag[action->prio].dei_mode = (action->outer_tag.dei_mode == VTSS_ECE_DEI_MODE_FIXED) ? VTSS_MCE_DEI_MODE_FIXED : VTSS_MCE_DEI_MODE_DP;
                nni_outer_tag[action->prio].dei = action->outer_tag.dei;
            }
        }
        if ((action->evc_id == evc_id) && (action->dir == VTSS_ECE_DIR_NNI_TO_UNI) && (action->prio_enable)) {
        /* This ECE has priority classification enabled and it is unidirectional from NNI to UNI. This is specifying PCP/DEI mapping from NNI to UNI on this COS-ID */
            if (action->prio < VTSS_MEP_SUPP_COS_ID_SIZE) {
                nni_key[action->prio].enable = TRUE;
                nni_key[action->prio].pcp = ece_conf.conf.key.tag.pcp.value;
            }
        }
        if ((action->evc_id == evc_id) && (action->dir == VTSS_ECE_DIR_BOTH) && (action->prio_enable)) {
        /* This ECE has priority classification enabled and it is bidirectional. This is specifying PCP/DEI mapping from NNI to UNI on this COS-ID */
            if (action->prio < VTSS_MEP_SUPP_COS_ID_SIZE) {
                nni_key[action->prio].enable = TRUE;
                nni_key[action->prio].pcp = action->outer_tag.pcp;
            }
        }
    }
}
#endif
#endif /* VTSS_SW_OPTION_EVC */

void vtss_mep_port_register(const u32         instance,
                            const BOOL        port[VTSS_PORT_ARRAY_SIZE])
{
    CRIT_ENTER(crit_p);
    memcpy(instance_data[instance].port, port, sizeof(instance_data[instance].port));
    CRIT_EXIT(crit_p);
}

vtss_packet_rx_queue_t vtss_mep_oam_cpu_queue_get(void)
{
    return PACKET_XTR_QU_OAM;
}

BOOL vtss_mep_vlan_member(u32 vid,  BOOL *ports,  BOOL enable)
{
    vtss_rc                 rc;
    vlan_mgmt_entry_t       vlan_entry;
    vtss_port_no_t          port_no;

    rc = vlan_mgmt_vlan_get(VTSS_ISID_START, vid, &vlan_entry, FALSE, VLAN_USER_MEP);
    if (rc != VTSS_RC_OK && rc != VLAN_ERROR_ENTRY_NOT_FOUND) {
        return FALSE;
    }

    vlan_entry.vid = vid;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vlan_entry.ports[port_no] = (ports[port_no] ? 1 : 0);
    }
    if (enable)     {if (vlan_mgmt_vlan_add(VTSS_ISID_START,  &vlan_entry,  VLAN_USER_MEP) != VTSS_RC_OK)     return(FALSE);}
    else            {if (vlan_mgmt_vlan_del(VTSS_ISID_GLOBAL, vid, VLAN_USER_MEP) != VTSS_RC_OK)    return(FALSE);}

    return(TRUE);
}

#if defined(VTSS_SW_OPTION_ACL)
#if defined(VTSS_ARCH_SERVAL)
void vtss_mep_acl_del(u32    instance)
{
    if (instance_data[instance].mip_lbm_ace_id != ACE_ID_NONE)
    {
        (void)acl_mgmt_ace_del(ACL_USER_MEP, instance_data[instance].mip_lbm_ace_id);
        instance_data[instance].mip_lbm_ace_id = ACE_ID_NONE;
    }
    if (instance_data[instance].ccm_ace_id[0] != ACE_ID_NONE)
    {
        (void)acl_mgmt_ace_del(ACL_USER_MEP, instance_data[instance].ccm_ace_id[0]);
        instance_data[instance].ccm_ace_id[0] = ACE_ID_NONE;
    }
    if (instance_data[instance].tst_ace_id[0] != ACE_ID_NONE)
    {
        (void)acl_mgmt_ace_del(ACL_USER_MEP, instance_data[instance].tst_ace_id[0]);
        instance_data[instance].tst_ace_id[0] = ACE_ID_NONE;
    }
}
#else
void vtss_mep_acl_del(u32    vid)
{
    acl_entry_conf_t     conf;
    vtss_ace_id_t        id;
    vtss_ace_counter_t   counter;
    vtss_rc              rc;
    u32                  i;

//T_D("vtss_mep_acl_del  vid %lu\n", vid);

    id = ACE_ID_NONE;
    do
    {
        if ((rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  id,  &conf,  &counter,  TRUE)) == VTSS_RC_OK)
        {
            if (conf.vid.value == vid)
            {
                rc = acl_mgmt_ace_del(ACL_USER_MEP, conf.id);   /* Entry on this VID so delete - next get is on same 'id' as last time */
                for (i=0; i<MEP_INSTANCE_MAX; ++i)
                { /* Check if this entry is a HW based CCM/TST frame rule */
                    if (conf.id == instance_data[i].ccm_ace_id[0])  instance_data[i].ccm_ace_id[0] = ACE_ID_NONE;
                    if (conf.id == instance_data[i].ccm_ace_id[1])  instance_data[i].ccm_ace_id[1] = ACE_ID_NONE;
                    if (conf.id == instance_data[i].tst_ace_id[0])  instance_data[i].tst_ace_id[0] = ACE_ID_NONE;
                    if (conf.id == instance_data[i].tst_ace_id[1])  instance_data[i].tst_ace_id[1] = ACE_ID_NONE;
                }
            }
            else   id = conf.id;    /* Entry NOT on this VID - next get on this new 'id' */
        }
    }
    while (rc == VTSS_RC_OK);
}
#endif

#if defined(VTSS_FEATURE_ACL_V2)
BOOL vtss_mep_acl_add(vtss_mep_acl_entry    *acl)
{
    acl_entry_conf_t     conf;
#if defined(VTSS_SW_OPTION_EVC)
    u32 i;
#endif

//    T_D("vtss_mep_acl_add\n");
//    T_D("vid = %lu   oam = %lX\n", acl->vid, acl->oam);
//    T_D("level = %lX   mask = %lX\n", acl->level, acl->mask);
//    T_D("ing_port = %u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", acl->ing_port[0],acl->ing_port[1],acl->ing_port[2],acl->ing_port[3],acl->ing_port[4],acl->ing_port[5],acl->ing_port[6],acl->ing_port[7],acl->ing_port[8],acl->ing_port[9],acl->ing_port[10],acl->ing_port[11],acl->ing_port[12],acl->ing_port[13],acl->ing_port[14],acl->ing_port[15],acl->ing_port[16],acl->ing_port[17],acl->ing_port[18],acl->ing_port[19],acl->ing_port[20],acl->ing_port[21],acl->ing_port[22],acl->ing_port[23]);
//    T_D("eg_port = %u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", acl->eg_port[0],acl->eg_port[1],acl->eg_port[2],acl->eg_port[3],acl->eg_port[4],acl->eg_port[5],acl->eg_port[6],acl->eg_port[7],acl->eg_port[8],acl->eg_port[9],acl->eg_port[10],acl->eg_port[11],acl->eg_port[12],acl->eg_port[13],acl->eg_port[14],acl->eg_port[15],acl->eg_port[16],acl->eg_port[17],acl->eg_port[18],acl->eg_port[19],acl->eg_port[20],acl->eg_port[21],acl->eg_port[22],acl->eg_port[23]);
//    T_D("ing_port = %u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", acl->ing_port[0],acl->ing_port[1],acl->ing_port[2],acl->ing_port[3],acl->ing_port[4],acl->ing_port[5],acl->ing_port[6],acl->ing_port[7],acl->ing_port[8],acl->ing_port[9]);
//    T_D("eg_port = %u-%u-%u-%u-%u-%u-%u-%u-%u-%u\n", acl->eg_port[0],acl->eg_port[1],acl->eg_port[2],acl->eg_port[3],acl->eg_port[4],acl->eg_port[5],acl->eg_port[6],acl->eg_port[7],acl->eg_port[8],acl->eg_port[9]);
//    T_D("cpu = %u  hw_ccm %u\n", acl->cpu, acl->hw_ccm);
//    T_D("\n");

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return FALSE;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;

    conf.isid = VTSS_ISID_LOCAL;
    conf.vid.value = acl->vid;                          /* Look for this VID */
    conf.vid.mask = 0xFFFF;
    conf.frame.etype.etype.mask[0] = 0xFF;              /* Look for OAM PDU */ 
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.frame.etype.data.mask[0] |= acl->mask<<5;      /* Look for this level(s) */
    conf.frame.etype.data.value[0] |= acl->level<<5;
/*
    if (acl->hw_ccm)
    {
        memcpy(conf.frame.etype.smac.value, acl->mac, sizeof(conf.frame.etype.smac.value));
        memset(conf.frame.etype.smac.mask, 0xFF, sizeof(conf.frame.etype.smac.mask));
    }
*/
    if (acl->oam != VTSS_OAM_ANY_TYPE)                  /* Look for a specific OAM PDU type */
    {
        conf.frame.etype.data.mask[1] = 0xFF;
        conf.frame.etype.data.value[1] = acl->oam;
    }

#if defined(VTSS_SW_OPTION_EVC)
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {
        if (acl->ing_port[i]) {
            if ((evc_vlan_vid[acl->vid] != 0xFFFFFFFF)) { /* Check if this VID is EVC */
                if ((out_port_conv[i] != i) && (in_port_conv[i] == 0xFFFF)){ /* This port is protected and not active */
                    acl->ing_port[i] = FALSE;
                    acl->ing_port[out_port_conv[i]] = TRUE;     /* The protecting port must hit the rule */
                }
            }
            break;
        }
    }
#endif

    memcpy(conf.port_list, acl->ing_port, sizeof(conf.port_list));
    conf.action.force_cpu = acl->cpu;
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memcpy(conf.action.port_list, acl->eg_port, sizeof(conf.action.port_list));

#if defined(VTSS_ARCH_LUTON26)
    conf.action.ptp_action = (acl->ts) ? VTSS_ACL_PTP_ACTION_TWO_STEP : VTSS_ACL_PTP_ACTION_NONE;
#endif
    return(acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) == VTSS_RC_OK);
}


BOOL vtss_up_mep_acl_add(u32 pag_oam, u32 pag_port, u32 customer, u32 first_egress_loop)
{
    acl_entry_conf_t     conf;
    u32 i;

    /* Ingress UNI -> NNI */
    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return FALSE;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;

    memset(conf.port_list, TRUE, sizeof(conf.port_list));
    conf.policy.value = pag_oam;
    conf.policy.mask = 0xFF;
    conf.type = VTSS_ACE_TYPE_ETYPE;
    conf.isid = VTSS_ISID_LOCAL;
    conf.frame.etype.etype.mask[0] = 0xFF;  /* OAM PDU */ 
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
    conf.action.policer = ACL_POLICER_NONE;
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memset(conf.action.port_list, TRUE, sizeof(conf.action.port_list));
    conf.action.force_cpu = FALSE;

    /* Any OAM rule for not hitting EVC IS2 entry */
    conf.id = ACE_ID_NONE;
    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);

    /* NNI -> Egress UNI */
    /* Rules for generating port mask for OAM to egress loop ports */
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
    for (i=0; i<4; ++i)
    {
        conf.id = ACE_ID_NONE;
        conf.policy.value = pag_port+i;
        conf.action.port_list[first_egress_loop+i] = TRUE;
        if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);

        conf.action.port_list[first_egress_loop+i] = FALSE;
    }

    /* Egress UNI termination */
    conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
    conf.policy.mask = 0;
    memset(conf.port_list, 0, sizeof(conf.port_list));
    for (i=0; i<4; ++i)        conf.port_list[first_egress_loop+i] = TRUE;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));

    conf.id = ACE_ID_NONE;
    conf.frame.etype.data.mask[1] = 0xFC;
    conf.frame.etype.data.value[1] = 0x2C;  /* DM type PDU */
    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
    conf.action.force_cpu = TRUE;
    conf.action.policer = ACL_POLICER_NONE;
    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);

    conf.id = ACE_ID_NONE;
    conf.frame.etype.data.mask[1] = 0xFF;
    conf.frame.etype.data.value[1] = 37;  /* TST type PDU */
    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
    conf.action.force_cpu = FALSE;
    conf.action.cpu_once = TRUE;
    conf.action.policer = ACL_POLICER_NONE;
    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);
    up_mep_tst_ace_id = conf.id;

    conf.id = ACE_ID_NONE;
    conf.frame.etype.data.mask[1] = 0x00;   /* Any type PDU */
    conf.action.force_cpu = TRUE;
    conf.action.policer = ACL_POLICER_NONE;
    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);

    conf.id = ACE_ID_NONE;
    conf.frame.etype.etype.mask[0] = 0;     /* Any frame */
    conf.frame.etype.etype.mask[1] = 0;
    conf.action.force_cpu = FALSE;
    conf.action.policer = ACL_POLICER_NONE;
    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    return(FALSE);

    return(TRUE);
}


void vtss_mep_acl_raps_add(u32 vid,  BOOL *ports,  u8 *mac,  u32 *id)
{
    vtss_ace_id_t       next_id;
    acl_entry_conf_t    conf;
    vtss_ace_counter_t  counter;

    if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  ACE_ID_NONE,  &conf,  &counter,  TRUE) != VTSS_RC_OK)    return;

    next_id = conf.id;
    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;

#if defined(VTSS_FEATURE_ACL_V2)
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
#else
    conf.action.permit = FALSE;
#endif /* VTSS_FEATURE_ACL_V2 */
    conf.isid = VTSS_ISID_LOCAL;
    conf.vid.value = vid;                          /* Look for this VID */
    conf.vid.mask = 0xFFFF;
    memcpy(conf.frame.etype.dmac.value, mac, sizeof(conf.frame.etype.dmac.value));
    memset(conf.frame.etype.dmac.mask, 0xFF, sizeof(conf.frame.etype.dmac.mask));
    conf.action.force_cpu = TRUE;
    memcpy(conf.port_list, ports, sizeof(conf.port_list));
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memcpy(conf.action.port_list, ports, sizeof(conf.action.port_list));

    if (acl_mgmt_ace_add(ACL_USER_MEP,  next_id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);

    *id = conf.id;
}

void vtss_mep_acl_raps_del(u32 id)
{
    if (acl_mgmt_ace_del(ACL_USER_MEP, id) != VTSS_RC_OK)      T_W("acl_mgmt_ace_del() failed for this ID  %u", id);
}
#endif

#if defined(VTSS_ARCH_JAGUAR_1) && !defined(VTSS_FEATURE_ACL_V2)
/****************************************************************
THIS IS TEMPORARY IN ORDER TO GET DM PDU TIMESTAMPTING ON JAGUAR
*****************************************************************/
void vtss_mep_acl_dm_add(vtss_mep_mgmt_domain_t domain,  u32 vid,  u32 pag,  u8 level)
{
    acl_entry_conf_t    conf;

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;
    conf.action.force_cpu = TRUE;
    conf.action.permit = TRUE;
    conf.action.port_no = VTSS_PORT_NO_NONE;
    conf.isid = VTSS_ISID_LOCAL;
    conf.frame.etype.etype.mask[0] = 0xFF;         /* Look for OAM PDU */ 
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.frame.etype.data.mask[0] = 0xE0;          /* Look for this level(s) */
    conf.frame.etype.data.value[0] = level<<5;
    conf.frame.etype.data.mask[1] = 0xFC;          /* Look for any DM PDU */
    conf.frame.etype.data.value[1] = 0x2C;

    if (domain == VTSS_MEP_MGMT_EVC) {
        conf.vid.value = vid;                          /* Save this VID */
        conf.vid.mask = 0;
        conf.policy.value = pag;
        conf.policy.mask = 0xFF;
        if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);
        conf.id = ACE_ID_NONE;
        conf.policy.mask = 0;
    }

    conf.vid.value = vid;                          /* Look for this VID */
    conf.vid.mask = 0xFFFF;

    if (acl_mgmt_ace_add(ACL_USER_MEP,  ACE_ID_NONE,  &conf) != VTSS_RC_OK)    T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);
}
#endif

void vtss_mep_acl_ccm_add(u32 instance,  u32 vid,  BOOL *port,  u32 level,  u8 *smac,  u32 pag)
{
    u32 i;
    vtss_ace_id_t       next_id;
    acl_entry_conf_t    conf;
    vtss_ace_counter_t  counter;
    vtss_rc             rc;

    rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  ACE_ID_NONE,  &conf,  &counter,  TRUE);  /* Get first entry for this user - MEP */

    if (rc == VTSS_RC_OK)
        next_id = conf.id;
    else
    if (rc == ACL_ERROR_ACE_NOT_FOUND)
        next_id = ACE_ID_NONE;
    else
        return;

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;
    conf.isid = VTSS_ISID_LOCAL;

#if defined(VTSS_ARCH_JAGUAR_1)
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) && (pag != 0)) {
        conf.vid.value = vid;                          /* Save this VID */
        conf.vid.mask = 0;
        conf.policy.value = pag;                       /* Look for this PAG */
        conf.policy.mask = 0xFF;
    }
    else {
        conf.vid.value = vid;                       /* Look for this VID */
        conf.vid.mask = 0xFFFF;
    }
#elif defined(VTSS_ARCH_SERVAL)
    conf.vid.value = vid;                          /* Look for this VID/ISDX */
    conf.vid.mask = 0xFFFF;
    conf.policy.value = pag;                       /* Look for this PAG */
    conf.policy.mask = 0x3F;
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) && (pag != 0))
        conf.isdx_enable = TRUE;
#else
    conf.vid.value = vid;                       /* Look for this VID */
    conf.vid.mask = 0xFFFF;
#endif

    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    if ((i < VTSS_PORT_ARRAY_SIZE) && (instance_data[instance].domain == VTSS_MEP_MGMT_EVC)) { /* Check if this VID is EVC */
        if ((out_port_conv[i] != i) && (in_port_conv[i] == 0xFFFF)){ /* This port is protected and not active */
            port[i] = FALSE;
            port[out_port_conv[i]] = TRUE;     /* The protecting port must hit the rule */
        }
    }

#if defined(VTSS_FEATURE_ACL_V1)
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    conf.port_no = i;
#endif
#if defined(VTSS_FEATURE_ACL_V2)
#ifdef VTSS_ARCH_SERVAL
    if (!conf.isdx_enable)
#endif
    memcpy(conf.port_list, port, sizeof(conf.port_list));
#endif
    conf.frame.etype.etype.mask[0] = 0xFF;     /* Look for OAM Ether Type */ 
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.frame.etype.data.mask[0] = 7<<5;      /* Look for this level(s) */
    conf.frame.etype.data.value[0] = level<<5;
    conf.frame.etype.data.mask[1] = 0xFF;      /* Look for CCM */
    conf.frame.etype.data.value[1] = 0x01;
    memcpy(conf.frame.etype.smac.value, smac, sizeof(conf.frame.etype.smac.value));
    memset(conf.frame.etype.smac.mask, 0xFF, sizeof(conf.frame.etype.smac.mask));
    conf.action.cpu_once = TRUE;
    conf.action.force_cpu = FALSE;
#if defined(VTSS_FEATURE_ACL_V1)
    conf.action.port_no = VTSS_PORT_NO_NONE;
    conf.action.permit = TRUE;
#endif
#if defined(VTSS_FEATURE_ACL_V2)
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memcpy(conf.action.port_list, port, sizeof(conf.action.port_list));
#endif

    if (acl_mgmt_ace_add(ACL_USER_MEP,  next_id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);

    instance_data[instance].ccm_ace_id[0] = conf.id;

#if defined(VTSS_FEATURE_ACL_V1)
    /* Search to see if one more port is wanted */
    for (i=conf.port_no+1; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    if (i < VTSS_PORT_ARRAY_SIZE) { /* One more entry is needed */
        conf.id = ACE_ID_NONE;
        conf.port_no = i;
        if (acl_mgmt_ace_add(ACL_USER_MEP,  conf.id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);
        instance_data[instance].ccm_ace_id[1] = conf.id;
    } else
        instance_data[instance].ccm_ace_id[1] = ACE_ID_NONE;
#endif
}

#ifdef VTSS_ARCH_SERVAL
void vtss_mep_acl_mip_lbm_add(u32 instance,  u32 dmac_port,  BOOL *ports,  u32 mip_pag,  BOOL isdx_to_zero)
{
#if defined(VTSS_SW_OPTION_EVC)
    u32               i;
#endif
    acl_entry_conf_t  conf;
    u8                mac[VTSS_MEP_MAC_LENGTH];

    vtss_mep_mac_get(dmac_port,  mac);

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK)     return;

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;
    conf.isid = VTSS_ISID_LOCAL;
    conf.policy.value = mip_pag;
    conf.policy.mask = 0X3F;
    conf.frame.etype.etype.mask[0] = 0xFF;
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.frame.etype.data.mask[1] = 0xFF;
    conf.frame.etype.data.value[1] = 3;
    memcpy(conf.frame.etype.dmac.value, mac, sizeof(conf.frame.etype.smac.value));
    memset(conf.frame.etype.dmac.mask, 0xFF, sizeof(conf.frame.etype.smac.mask));
#if defined(VTSS_SW_OPTION_EVC)
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {
        if (ports[i]) {
            if ((out_port_conv[i] != i) && (in_port_conv[i] == 0xFFFF)){ /* This port is protected and not active */
                ports[i] = FALSE;
                ports[out_port_conv[i]] = TRUE;     /* The protecting port must hit the rule */
            }
            break;
        }
    }
#endif
    memcpy(conf.port_list, ports, sizeof(conf.port_list));

    conf.action.force_cpu = TRUE;
    conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
    conf.action.lm_cnt_disable = (isdx_to_zero) ? TRUE : FALSE;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
/*  This is not implemented yet  
    if (isdx_to_zero) {
        conf.action.isdx_enable = TRUE;
        conf.action.isdx = 0;
    }
*/
    if (acl_mgmt_ace_add(ACL_USER_MEP, ACE_ID_NONE, &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
    instance_data[instance].mip_lbm_ace_id = conf.id;
}

static vtss_ace_id_t              service_mep_ace_id;
static vtss_ace_id_t              service_mip_ccm_ace_id;
static vtss_ace_id_t              service_mip_uni_ltm_ace_id;
static vtss_ace_id_t              service_mip_nni_ltm_ace_id;

void vtss_mep_acl_evc_add(BOOL mep,  BOOL *mip_nni,  BOOL *mip_uni,  BOOL *mip_egress,  u32 mep_pag,  u32 mip_pag)
{
#if defined(VTSS_SW_OPTION_EVC)
    u32               i;
#endif
    acl_entry_conf_t  conf, nxt_conf;
    vtss_rc           rc;
    BOOL              ports[VTSS_PORT_ARRAY_SIZE];

    memset(ports, 0, sizeof(ports));

#if defined(VTSS_SW_OPTION_EVC)
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {
        if (mip_nni[i]) {
            if ((out_port_conv[i] != i) && (in_port_conv[i] == 0xFFFF)){ /* This port is protected and not active */
                mip_nni[i] = FALSE;
                mip_nni[out_port_conv[i]] = TRUE;     /* The protecting port must hit the rule */
            }
        }
        ports[i] = (mip_nni[i] || mip_uni[i]) ? TRUE : FALSE;
    }
#endif
    if (!mep) {
        (void)acl_mgmt_ace_del(ACL_USER_MEP, service_mep_ace_id);
        service_mep_ace_id = ACE_ID_NONE;
    }

    if (mep && (service_mep_ace_id == ACE_ID_NONE)) { /* Add the MEP ACL */
        if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK)       return;

        conf.action.cpu_queue = PACKET_XTR_QU_OAM;
        conf.isid = VTSS_ISID_LOCAL;
        conf.policy.value = mep_pag;
        conf.policy.mask = 0X3F;
        conf.action.force_cpu = TRUE;
        conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
        memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
        if (acl_mgmt_ace_add(ACL_USER_MEP, ACE_ID_NONE, &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
        service_mep_ace_id = conf.id;
    }


    conf.id = service_mip_ccm_ace_id;
    nxt_conf.id = ACE_ID_NONE;
    if (service_mip_ccm_ace_id != ACE_ID_NONE)
    {
        /* Get conf for id */
        if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_ccm_ace_id,  &conf,  NULL,  FALSE) != VTSS_RC_OK)    return;
    
        /* Get next config for this id */
        rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_ccm_ace_id,  &nxt_conf,  NULL,  TRUE);
        if (rc == ACL_ERROR_ACE_NOT_FOUND)
            nxt_conf.id = ACE_ID_NONE;
        else
        if (rc != VTSS_RC_OK)
            return;
    }
    else {
        if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK)      return;
        conf.action.cpu_queue = PACKET_XTR_QU_OAM;
        conf.isid = VTSS_ISID_LOCAL;
        conf.policy.value = mip_pag;
        conf.policy.mask = 0X3F;
        conf.frame.etype.data.mask[1] = 0xFF;
        conf.frame.etype.data.value[1] = 1;
        conf.action.cpu_once = TRUE;
        conf.action.force_cpu = FALSE;
        conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    }

    memcpy(conf.port_list, ports, sizeof(conf.port_list));
    memcpy(conf.action.port_list, mip_egress, sizeof(conf.action.port_list));

    if (acl_mgmt_ace_add(ACL_USER_MEP, nxt_conf.id, &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
    service_mip_ccm_ace_id = conf.id;


    conf.id = service_mip_uni_ltm_ace_id;
    nxt_conf.id = ACE_ID_NONE;
    if (service_mip_uni_ltm_ace_id != ACE_ID_NONE)
    {
        /* Get conf for id */
        if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_uni_ltm_ace_id,  &conf,  NULL,  FALSE) != VTSS_RC_OK)    return;
    
        /* Get next config for this id */
        rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_uni_ltm_ace_id,  &nxt_conf,  NULL,  TRUE);
        if (rc == ACL_ERROR_ACE_NOT_FOUND)
            nxt_conf.id = ACE_ID_NONE;
        else
        if (rc != VTSS_RC_OK)
            return;
    }
    else {
        if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK)      return;
        conf.action.cpu_queue = PACKET_XTR_QU_OAM;
        conf.isid = VTSS_ISID_LOCAL;
        conf.policy.value = mip_pag;
        conf.policy.mask = 0X3F;
        conf.frame.etype.data.mask[1] = 0xFF;
        conf.frame.etype.data.value[1] = 5;
        conf.action.force_cpu = TRUE;
        conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
        conf.action.lm_cnt_disable = TRUE;
/*  This is not implemented yet  
        conf.action.isdx_enable = TRUE;
        conf.action.isdx = 0;
*/
    }

    memcpy(conf.port_list, mip_uni, sizeof(conf.port_list));
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));

    if (acl_mgmt_ace_add(ACL_USER_MEP, nxt_conf.id, &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
    service_mip_uni_ltm_ace_id = conf.id;


    conf.id = service_mip_nni_ltm_ace_id;
    nxt_conf.id = ACE_ID_NONE;
    if (service_mip_nni_ltm_ace_id != ACE_ID_NONE)
    {
        /* Get conf for id */
        if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_nni_ltm_ace_id,  &conf,  NULL,  FALSE) != VTSS_RC_OK)    return;
    
        /* Get next config for this id */
        rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  service_mip_nni_ltm_ace_id,  &nxt_conf,  NULL,  TRUE);
        if (rc == ACL_ERROR_ACE_NOT_FOUND)
            nxt_conf.id = ACE_ID_NONE;
        else
        if (rc != VTSS_RC_OK)
            return;
    }
    else {
        if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK)       return;
        conf.action.cpu_queue = PACKET_XTR_QU_OAM;
        conf.isid = VTSS_ISID_LOCAL;
        conf.policy.value = mip_pag;
        conf.policy.mask = 0X3F;
        conf.frame.etype.data.mask[1] = 0xFF;
        conf.frame.etype.data.value[1] = 5;
        conf.action.force_cpu = TRUE;
        conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
    }

    memcpy(conf.port_list, mip_nni, sizeof(conf.port_list));
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));

    if (acl_mgmt_ace_add(ACL_USER_MEP, nxt_conf.id, &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
    service_mip_nni_ltm_ace_id = conf.id;
}
#endif

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_JAGUAR_1)
void vtss_mep_acl_ccm_hit(u32 instance)
{
    vtss_rc  rc;

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
    if (instance_data[instance].ccm_ace_id[0] != ACE_ID_NONE)
    {
        rc = vtss_ace_counter_clear(NULL, instance_data[instance].ccm_ace_id[0] | (ACL_USER_MEP<<16));
        if (rc != VTSS_RC_OK)      return;
    }
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
    u32                  i;
    acl_entry_conf_t     conf, nxt_conf;
    vtss_ace_counter_t   counter;

    for (i=0; i<2; ++i) {
        if (instance_data[instance].ccm_ace_id[i] != ACE_ID_NONE)
        {
            /* Get conf for id */
            if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  instance_data[instance].ccm_ace_id[i],  &conf,  &counter,  FALSE) != VTSS_RC_OK)    return;
            conf.action.cpu_once = TRUE;
    
            /* Get next config for this id */
            rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  instance_data[instance].ccm_ace_id[i],  &nxt_conf,  &counter,  TRUE);
            if (rc == ACL_ERROR_ACE_NOT_FOUND)
                nxt_conf.id = ACE_ID_NONE;
            else
            if (rc != VTSS_RC_OK)
                return;
    
            /* Add config for this id in front of next id */
            if (acl_mgmt_ace_add(ACL_USER_MEP,  nxt_conf.id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
        }
    }
#endif
}

void vtss_mep_acl_ccm_count(u32 instance,  u32 *count)
{
    u32 i;
    vtss_ace_counter_t   counter=0;

    *count = 0;
    for (i=0; i<2; ++i) {
        if (instance_data[instance].ccm_ace_id[i] != ACE_ID_NONE)
        {
            if (vtss_ace_counter_get(NULL, instance_data[instance].ccm_ace_id[i] | (ACL_USER_MEP<<16), &counter) != VTSS_RC_OK)      T_W("vtss_ace_counter_get() failed for this id  %u", instance_data[instance].ccm_ace_id[i]);
            *count += (u32)counter;
        }
    }
}
#endif /* Luton26 or Jaguar */

void vtss_mep_acl_tst_add(u32 instance,  u32 vid,  BOOL *port,  u32 level,  u32 pag)
{
    u32 i;
    vtss_ace_id_t       next_id;
    acl_entry_conf_t    conf;
    vtss_ace_counter_t  counter;
    vtss_rc             rc;

    rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  ACE_ID_NONE,  &conf,  &counter,  TRUE);

    if (rc == VTSS_RC_OK)
        next_id = conf.id;
    else
    if (rc == ACL_ERROR_ACE_NOT_FOUND)
        next_id = ACE_ID_NONE;
    else
        return;

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf) != VTSS_RC_OK) {
        return;
    }

    conf.action.cpu_queue = PACKET_XTR_QU_OAM;
    conf.isid = VTSS_ISID_LOCAL;

#if defined(VTSS_ARCH_JAGUAR_1)
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) && (pag != 0)) {
        conf.vid.value = vid;                          /* Save this VID */
        conf.vid.mask = 0;
        conf.policy.value = pag;                       /* Look for this PAG */
        conf.policy.mask = 0xFF;
    }
    else {
        conf.vid.value = vid;                       /* Look for this VID */
        conf.vid.mask = 0xFFFF;
    }
#elif defined(VTSS_ARCH_SERVAL)
    conf.vid.value = vid;                          /* Look for this VID/ISDX */
    conf.vid.mask = 0xFFFF;
    conf.policy.value = pag;                       /* Look for this PAG */
    conf.policy.mask = 0x3F;
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) && (pag != 0))
        conf.isdx_enable = TRUE;
#else
    conf.vid.value = vid;                       /* Look for this VID */
    conf.vid.mask = 0xFFFF;
#endif

    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    if ((i < VTSS_PORT_ARRAY_SIZE) && (instance_data[instance].domain == VTSS_MEP_MGMT_EVC)) { /* Check if this VID is EVC */
        if ((out_port_conv[i] != i) && (in_port_conv[i] == 0xFFFF)){ /* This port is protected and not active */
            port[i] = FALSE;
            port[out_port_conv[i]] = TRUE;     /* The protecting port must hit the rule */
        }
    }

#if defined(VTSS_FEATURE_ACL_V1)
    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    conf.port_no = i;
#endif
#if defined(VTSS_FEATURE_ACL_V2)
#ifdef VTSS_ARCH_SERVAL
    if (!conf.isdx_enable)
#endif
    memcpy(conf.port_list, port, sizeof(conf.port_list));
#endif
    conf.frame.etype.etype.mask[0] = 0xFF;     /* Look for OAM Ether Type */ 
    conf.frame.etype.etype.mask[1] = 0xFF;
    conf.frame.etype.etype.value[0] = 0x89;
    conf.frame.etype.etype.value[1] = 0x02;
    conf.frame.etype.data.mask[0] = 7<<5;      /* Look for this level(s) */
    conf.frame.etype.data.value[0] = level<<5;
    conf.frame.etype.data.mask[1] = 0xFF;      /* Look for TST */
    conf.frame.etype.data.value[1] = 37;
    conf.action.cpu_once = TRUE;
    conf.action.force_cpu = FALSE;
#if defined(VTSS_FEATURE_ACL_V1)
    conf.action.port_no = VTSS_PORT_NO_NONE;
    conf.action.permit = FALSE;
#endif
#if defined(VTSS_FEATURE_ACL_V2)
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memcpy(conf.action.port_list, port, sizeof(conf.action.port_list));
#endif

    if (acl_mgmt_ace_add(ACL_USER_MEP,  next_id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);

    instance_data[instance].tst_ace_id[0] = conf.id;

#if defined(VTSS_FEATURE_ACL_V1)
    /* Search to see if one more port is wanted - in case of Port domain Egress MEP */
    for (i=conf.port_no+1; i<VTSS_PORT_ARRAY_SIZE; ++i)     if (port[i])    break;
    if (i < VTSS_PORT_ARRAY_SIZE) { /* One more entry is needed */
        conf.id = ACE_ID_NONE;
        conf.port_no = i;
        if (acl_mgmt_ace_add(ACL_USER_MEP,  conf.id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this VID  %u", vid);
        instance_data[instance].tst_ace_id[1] = conf.id;
    } else
        instance_data[instance].tst_ace_id[1] = ACE_ID_NONE;
#endif
}

void vtss_mep_acl_tst_count(u32 instance,  u32 *count)
{
    u32 i;
    vtss_ace_counter_t   counter=0;
    BOOL ingress = FALSE;

    *count = 0;
    for (i=0; i<2; ++i) {
        if (instance_data[instance].tst_ace_id[i] != ACE_ID_NONE)
        {
            if (vtss_ace_counter_get(NULL, instance_data[instance].tst_ace_id[i] | (ACL_USER_MEP<<16), &counter) != VTSS_RC_OK)      T_W("vtss_ace_counter_get() failed for this id  %u", instance_data[instance].ccm_ace_id[i]);
            *count += (u32)counter;
            ingress = TRUE;
        }
    }

    if (!ingress && (up_mep_tst_ace_id != ACE_ID_NONE)) {
        if (vtss_ace_counter_get(NULL, up_mep_tst_ace_id | (ACL_USER_MEP<<16), &counter) != VTSS_RC_OK)      T_W("vtss_ace_counter_get() failed for this id  %u", up_mep_tst_ace_id);
        *count = (u32)counter;
    }
}

void vtss_mep_acl_tst_clear(u32 instance)
{
    u32     i;
    BOOL    ingress = FALSE;

    for (i=0; i<2; ++i) {
        if (instance_data[instance].tst_ace_id[i] != ACE_ID_NONE) {
            if (vtss_ace_counter_clear(NULL, instance_data[instance].tst_ace_id[i] | (ACL_USER_MEP<<16)) != VTSS_RC_OK)      T_W("vtss_ace_counter_clear() failed for this id  %u", instance_data[instance].tst_ace_id[i]);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON28)
            acl_entry_conf_t    conf, nxt_conf;
            vtss_ace_counter_t  counter;
            vtss_rc             rc;
        
            /* Get conf for id */
            if (acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  instance_data[instance].tst_ace_id[i],  &conf,  &counter,  FALSE) != VTSS_RC_OK)    continue;
            conf.action.cpu_once = TRUE;
        
            /* Get next config for this id */
            rc = acl_mgmt_ace_get(ACL_USER_MEP,  VTSS_ISID_LOCAL,  instance_data[instance].tst_ace_id[i],  &nxt_conf,  &counter,  TRUE);
            if (rc == ACL_ERROR_ACE_NOT_FOUND)
                nxt_conf.id = ACE_ID_NONE;
            else
            if (rc != VTSS_RC_OK)
                return;
        
            /* Add config for this id in front of next id */
            if (acl_mgmt_ace_add(ACL_USER_MEP,  nxt_conf.id,  &conf) != VTSS_RC_OK)      T_W("acl_mgmt_ace_add() failed for this id  %u", conf.id);
#endif
            ingress = TRUE;
        }
    }
    if (!ingress && (up_mep_tst_ace_id != ACE_ID_NONE)) {
        if (vtss_ace_counter_clear(NULL, up_mep_tst_ace_id | (ACL_USER_MEP<<16)) != VTSS_RC_OK)      T_W("vtss_ace_counter_clear() failed for this id  %u", up_mep_tst_ace_id);
    }
}

#endif /* VTSS_SW_OPTION_ACL */

void vtss_mep_rule_update_failed(u32 vid)
{
    T_W("Updating rules (MAC/ACL) failed for this VID  %u", vid);
//T_D("vtss_mep_update_failed  vid %u\n", vid);
}

BOOL vtss_mep_protection_port_get(const u32 port,  u32 *p_port)
{
    u32 i;

    if (out_port_conv[port] != port) { /* This port is a protected port (working) */
        *p_port = out_port_conv[port];
        return(TRUE);
    }

    for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)  /* If this is a protecting port - find working */
        if ((i != port) && (out_port_conv[i] == port))      break;

    if (i != VTSS_PORT_ARRAY_SIZE) {    /* A working port was found for this protecting port */
        *p_port = out_port_conv[i];
        return(TRUE);
    }

    *p_port = 0;
    return(FALSE);
}





/****************************************************************************/
/*  MEP lower support platform interface                                    */
/****************************************************************************/

void vtss_mep_supp_crit_lock(void)
{
    CRIT_ENTER(crit_supp);
}

void vtss_mep_supp_crit_unlock(void)
{
    CRIT_EXIT(crit_supp);
}

void vtss_mep_supp_run(void)
{
    cyg_semaphore_post(&run_wait_sem);
}

void vtss_mep_supp_timer_start(void)
{
    cyg_flag_setbits(&timer_wait_flag, FLAG_TIMER);
}

void vtss_mep_supp_trace(const char  *const string,
                         const u32   param1,
                         const u32   param2,
                         const u32   param3,
                         const u32   param4)
{
    T_D("%s - %u, %u, %u, %u", string, param1, param2, param3, param4);
}

void vtss_mep_supp_port_conf(const u32       instance,
                             const u32       port,
                             const BOOL      up_mep,
                             const BOOL      out_of_service)
{
    port_vol_conf_t conf;

    conf.disable = FALSE;
    conf.loop = out_of_service ? VTSS_PORT_LOOP_PCS_HOST : VTSS_PORT_LOOP_DISABLE;
    conf.oper_up = out_of_service || up_mep;

    (void)port_vol_conf_set(PORT_USER_MEP, VTSS_ISID_START, port, &conf);
}

void vtss_mep_supp_packet_tx_frm_cnt(u8 *frm, u64 *frm_cnt)
{
#if defined(VTSS_FEATURE_AFI_FDMA)
    if (packet_tx_afi_frm_cnt(frm, frm_cnt) != VTSS_RC_OK)
#endif
    {
        *frm_cnt = 0;
    }
}

u8 *vtss_mep_supp_packet_tx_alloc(u32 size)
{
    u8 *buffer = 0;
    buffer = packet_tx_alloc(size);
    memset(buffer, 0, size);
    return buffer;
}

void vtss_mep_supp_packet_tx_free(u8 *buffer)
{
    packet_tx_free(buffer);
}

void vtss_mep_supp_packet_tx_cancel(u32 instance,  u8 *buffer)
{
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    u32 tx_idx;

    for (tx_idx=0; tx_idx<TX_BUF_MAX; ++tx_idx)  /* Find buffer in a tx_buffer element */
        if (instance_data[instance].tx_buffer[tx_idx][0] == buffer)   break;

    if (tx_idx < TX_BUF_MAX) { /* buffer was found */
        if (packet_tx_afi_cancel(instance_data[instance].tx_buffer[tx_idx][0]) != VTSS_RC_OK)   /* Cancel buffer */
            T_D("Error during packet_tx_afi_cancel()");
    
        if (instance_data[instance].tx_buffer[tx_idx][1] != NULL)
            if (packet_tx_afi_cancel(instance_data[instance].tx_buffer[tx_idx][1]) != VTSS_RC_OK)   /* Cancel extra buffer - if any */
                T_D("Error during packet_tx_afi_cancel()");

        instance_data[instance].tx_buffer[tx_idx][0] = NULL;    /* Free tx_buffer element */
        instance_data[instance].tx_buffer[tx_idx][1] = NULL;
    }
#endif
}

static void packet_tx_done(void *context, packet_tx_done_props_t *props)
{
    vtss_mep_supp_tx_done_t *done;
    vtss_timestamp_t        tx_time;

    memset(&tx_time, 0, sizeof(tx_time));

    if (!context)    return;
    done = (vtss_mep_supp_tx_done_t*)(context);
    if (!done->cb)   return;

    vtss_tod_ts_to_time(props->sw_tstamp.hw_cnt, &tx_time);
    T_D("packet_tx_done: Sec- %u, NanoSec - %u", tx_time.seconds,tx_time.nanoseconds);

    done->cb(done->instance, props->frm_ptr[0], tx_time, props->frm_cnt);
}

vtss_packet_oam_type_t oam_type_calc(vtss_mep_supp_oam_type_t   oam_type)
{
    switch(oam_type) {
        case VTSS_MEP_SUPP_OAM_TYPE_NONE:     return(VTSS_PACKET_OAM_TYPE_NONE);
        case VTSS_MEP_SUPP_OAM_TYPE_CCM:      return(VTSS_PACKET_OAM_TYPE_CCM);
        case VTSS_MEP_SUPP_OAM_TYPE_CCM_LM:   return(VTSS_PACKET_OAM_TYPE_CCM_LM);
        case VTSS_MEP_SUPP_OAM_TYPE_LBM:      return(VTSS_PACKET_OAM_TYPE_LBM);
        case VTSS_MEP_SUPP_OAM_TYPE_LBR:      return(VTSS_PACKET_OAM_TYPE_LBR);
        case VTSS_MEP_SUPP_OAM_TYPE_LMM:      return(VTSS_PACKET_OAM_TYPE_LMM);
        case VTSS_MEP_SUPP_OAM_TYPE_LMR:      return(VTSS_PACKET_OAM_TYPE_LMR);
        case VTSS_MEP_SUPP_OAM_TYPE_DMM:      return(VTSS_PACKET_OAM_TYPE_DMM);
        case VTSS_MEP_SUPP_OAM_TYPE_DMR:      return(VTSS_PACKET_OAM_TYPE_DMR);
        case VTSS_MEP_SUPP_OAM_TYPE_1DM:      return(VTSS_PACKET_OAM_TYPE_1DM);
        case VTSS_MEP_SUPP_OAM_TYPE_LTM:      return(VTSS_PACKET_OAM_TYPE_LTM);
        case VTSS_MEP_SUPP_OAM_TYPE_LTR:      return(VTSS_PACKET_OAM_TYPE_LTR);
        case VTSS_MEP_SUPP_OAM_TYPE_GENERIC:  return(VTSS_PACKET_OAM_TYPE_GENERIC);
        default:                              return(VTSS_PACKET_OAM_TYPE_NONE);
    }
}

u32 vtss_mep_supp_packet_tx(u32                            instance,
                            vtss_mep_supp_tx_done_t        *done,
                            BOOL                           port[VTSS_PORT_ARRAY_SIZE],
                            u32                            len,
                            unsigned char                  *frm,
                            vtss_mep_supp_tx_frame_info_t  *frame_info,
                            u32                            rate,
                            BOOL                           count_enable,
                            u32                            seq_offset,
                            vtss_mep_supp_oam_type_t       oam_type)
{
    int rc = 0;
    u32  i, port_mask=0, mask, tx_idx;
    BOOL port_down_mep;
    packet_tx_props_t         tx_props;
    vtss_packet_port_info_t   info;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];

    if (frm == NULL)     return(1);

    /* This is in order to inter-operate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    /* In case of rate <> 0 the frame is either CCM (>64) or a TST PDU. TST generator need to transmit frames smaller that 64 byte */ 
    if ((!rate) && (len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    if (vtss_packet_port_info_init(&info) != VTSS_RC_OK)
        return(1);

    info.vid = frame_info->vid;
    if (vtss_packet_port_filter_get(NULL, &info, filter) != VTSS_RC_OK) /* Get the filter info for this VID */
        return(1);

    CRIT_ENTER(crit_p);
    port_down_mep = ((instance_data[instance].domain == VTSS_MEP_MGMT_PORT) && port[instance_data[instance].res_port]);
    for (i=0, mask=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)    /* All ports are checked. Injection must be through analyser or on the loop port or no filter discarded. Injection must be using AFI or the port must be active */
        if (port[i] && (!frame_info->bypass || (i == voe_up_mep_loop_port) || port_down_mep || (filter[i].filter != VTSS_PACKET_FILTER_DISCARD)) && (!los_state[i] || (i == voe_up_mep_loop_port)))
            port_mask |= mask;

    if (port_mask)
    {
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
        tx_props.packet_info.frm[0]             = frm;
        tx_props.packet_info.len[0]             = len;
        tx_props.tx_info.dst_port_mask          = port_mask;
        tx_props.packet_info.tx_done_cb         = packet_tx_done;
        tx_props.packet_info.tx_done_cb_context = done;
        tx_props.tx_info.switch_frm             = !frame_info->bypass;
        tx_props.tx_info.cos                    = frame_info->qos;
        tx_props.tx_info.dp                     = frame_info->dp;
        tx_props.tx_info.tag.pcp                = frame_info->pcp;
        tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
        tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
        tx_props.tx_info.tag.vid                = frame_info->vid_inj ? frame_info->vid : 0;
#if defined(VTSS_ARCH_SERVAL)
        tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif

        if (!rate) {    /* This is SW generated frame - NO AFI */
            rc = (packet_tx(&tx_props) == VTSS_RC_OK);   /* Transmit */
            T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                 instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                 tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                 frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
        }
        else
        { /* This is HW based CCM or TST/LBM - transmission on more ports demands more buffers */
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
            u8 *tx_buffer = NULL;

            tx_props.fdma_info.afi_fps = rate;
#if defined(VTSS_FEATURE_AFI_FDMA)
            tx_props.fdma_info.afi_enable_counting = count_enable;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

            for (i=0, mask=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)
                if (port_mask & mask)    break;         /* Transmit frame on the first port in mask */
            if (i == VTSS_PORT_ARRAY_SIZE)  {CRIT_EXIT(crit_p); return(1);} /* Check if any port was found */
            tx_props.tx_info.dst_port_mask = VTSS_BIT64(i);

            if (done == NULL)   /* This is a AFI transmission so only CB if requested. */
                tx_props.packet_info.tx_done_cb = 0;

            if (count_enable && (in_port_conv[i] == 0xFFFF)) { /* No counting and call back on not active port */
                tx_props.packet_info.tx_done_cb = 0;
#if defined(VTSS_FEATURE_AFI_FDMA)
                tx_props.fdma_info.afi_enable_counting = FALSE;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
            }

#if defined(VTSS_FEATURE_AFI_FDMA)
            if (seq_offset != 0) { /* Offset for sequence numbers is indicating active */
                tx_props.fdma_info.afi_enable_sequence_numbering = TRUE;
                tx_props.fdma_info.afi_sequence_number_offset = seq_offset;
            }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

            for (tx_idx=0; tx_idx<TX_BUF_MAX; ++tx_idx)  /* Find a free tx_buffer element */
                if (instance_data[instance].tx_buffer[tx_idx][0] == NULL)   break;
            if (tx_idx == TX_BUF_MAX)  {CRIT_EXIT(crit_p); return(1);}

            instance_data[instance].tx_buffer[tx_idx][0] = frm;
            instance_data[instance].tx_buffer[tx_idx][1] = NULL;

            rc = (packet_tx(&tx_props) == VTSS_RC_OK);  /* Transmit */
            T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                 instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                 tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                 frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);

            for (i=i+1, mask<<=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)    /* Search for a second port in mask */
                if (port_mask & mask)    break;
            if (i == VTSS_PORT_ARRAY_SIZE)  {CRIT_EXIT(crit_p); return(0);} /* Check if any port was found */

            /* Second port found - Allocate buffer for second port */
            tx_buffer = instance_data[instance].tx_buffer[tx_idx][1] = packet_tx_alloc(len);
            if (tx_buffer == NULL)  {CRIT_EXIT(crit_p); return(0);}

            /* Transmit copy of frame on the second port */
            memcpy(tx_buffer, frm, len);
            tx_props.tx_info.dst_port_mask = VTSS_BIT64(i);
            tx_props.packet_info.tx_done_cb = packet_tx_done;

#if defined(VTSS_FEATURE_AFI_FDMA)
            tx_props.fdma_info.afi_enable_counting = count_enable;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
            tx_props.packet_info.frm[0] = tx_buffer;

            if (done == NULL)   /* This is a AFI transmission so only CB if requested. */
                tx_props.packet_info.tx_done_cb = 0;

            if (count_enable && (in_port_conv[i] == 0xFFFF)) { /* No counting and call back on not active port */
                tx_props.packet_info.tx_done_cb = 0;
#if defined(VTSS_FEATURE_AFI_FDMA)
                tx_props.fdma_info.afi_enable_counting = FALSE;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
            }

            rc = (packet_tx(&tx_props) == VTSS_RC_OK);  /* Transmit */
            T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                 instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                 tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                 frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
        }
    }

    CRIT_EXIT(crit_p);

    return (rc != 0);
}

u32 vtss_mep_supp_packet_tx_1(u32                            instance,
                              vtss_mep_supp_tx_done_t        *done,
                              u32                            port,
                              u32                            len,
                              unsigned char                  *frm,
                              vtss_mep_supp_tx_frame_info_t  *frame_info,
                              vtss_mep_supp_oam_type_t       oam_type)
{
    int rc = 0;
    u32 i, mask;
    packet_tx_props_t tx_props;

    if (frm == NULL)     return(1);

    /* This is in order to inter-orperate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    if ((len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    CRIT_ENTER(crit_p);
    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
    tx_props.packet_info.frm[0]             = frm;
    tx_props.packet_info.len[0]             = len;
    tx_props.packet_info.tx_done_cb         = packet_tx_done;
    tx_props.packet_info.tx_done_cb_context = done;
    tx_props.tx_info.switch_frm             = !frame_info->bypass;
    tx_props.tx_info.cos                    = frame_info->qos;
    tx_props.tx_info.dp                     = frame_info->dp;
    tx_props.tx_info.tag.pcp                = frame_info->pcp;
    tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
    tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
    tx_props.tx_info.tag.vid                = frame_info->vid_inj ? frame_info->vid : 0;
#if defined(VTSS_ARCH_SERVAL)
    tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif


    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) || (instance_data[instance].domain == VTSS_MEP_MGMT_VLAN)) {
        tx_props.tx_info.dst_port_mask = port_protection_mask_calc(port);
        for (i=0, mask=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)    /* Clear any port with active LOS */
            if (los_state[i])   tx_props.tx_info.dst_port_mask &= ~(u64)mask;
        if (tx_props.tx_info.dst_port_mask) { /* Do not transmit if mask is all zero */
            rc = packet_tx(&tx_props) == VTSS_RC_OK;
            T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                 instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                 tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                 frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
        }
    }
    else if (instance_data[instance].domain == VTSS_MEP_MGMT_PORT) {
        if (!los_state[port]) {
            tx_props.tx_info.dst_port_mask = VTSS_BIT64(port);
            rc = packet_tx(&tx_props) == VTSS_RC_OK;
            T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                 instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                 tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                 frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
        }
    }

    CRIT_EXIT(crit_p);

    return (rc != 0);
}

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
static void timestamp_two_step_cb (void *context, u32 port_no, vtss_ts_timestamp_t *ts)
{ 
    vtss_mep_supp_tx_done_t *done;
    vtss_timestamp_t        tx_time;

    T_D("timestamp_two_step_cb: port_no: %d",port_no);
    
    if (!context)    return;
    done = (vtss_mep_supp_tx_done_t*)(context);
    if (!done->cb)   return;
    vtss_tod_ts_to_time(ts->ts, &tx_time);
    T_D("timestamp_two_step_cb: Sec:%u  NanoSec:%u", tx_time.seconds, tx_time.nanoseconds);
    
    done->cb(done->instance, NULL, tx_time, 0);
}

static vtss_rc mep_ts_two_step_cb_reg(packet_tx_props_t *tx_props,
                                u64  port_mask,
                                void *context)
{
    vtss_rc rc;
    vtss_ts_timestamp_alloc_t alloc_parm;
    
    alloc_parm.port_mask = port_mask;
    alloc_parm.context = context;
    alloc_parm.cb = timestamp_two_step_cb;
    vtss_ts_id_t ts_id;
    rc = vtss_tx_timestamp_idx_alloc(0, &alloc_parm, &ts_id); /* allocate id for transmission*/
    T_D("Timestamp Id (%u)allocated  rc - %d", ts_id.ts_id, rc);
#if defined(VTSS_ARCH_LUTON26)
    tx_props->tx_info.ptp_action    = VTSS_PACKET_PTP_ACTION_TWO_STEP; /* twostep action */
    tx_props->tx_info.ptp_id        = ts_id.ts_id;
    tx_props->tx_info.ptp_timestamp = 0; /* subtracted from current time in the tx-fifo */
#endif        
#if defined(VTSS_ARCH_JAGUAR_1)
    /* latch register no+1: see Jaguar Datasheet, table 7, fwd.capture_tstamp */
    tx_props->tx_info.latch_timestamp = ts_id.ts_id + 1;
#endif 
    return (rc);
}   
#endif 

u32 vtss_mep_supp_packet_tx_two_step_ts(u32                         instance,
                                        vtss_mep_supp_tx_done_t     *done,
                                        BOOL                        port[VTSS_PORT_ARRAY_SIZE],
                                        u32                         len,
                                        unsigned char               *frm)
{
    int rc = TRUE;
    u32  i, port_mask=0, mask;
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)    
    u64  port_mask_ts;
#endif    
    packet_tx_props_t tx_props;
    vtss_rc rc_reg = VTSS_RC_OK;

    if (frm == NULL)     return(1);

    /* This is in order to inter-orperate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    if ((len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    CRIT_ENTER(crit_p);
    if (instance_data[instance].domain == VTSS_MEP_MGMT_EVC)
    {
        for (i=0, mask = 1 << (0 + VTSS_PORT_NO_START); i<VTSS_PORT_ARRAY_SIZE; ++i, mask <<= 1)
            if (port[i])
            {
                port_mask |= mask;
                if ((out_port_conv[i] != i) && (!los_state[out_port_conv[i]]))   port_mask |= 1 << (out_port_conv[i] + VTSS_PORT_NO_START);     /* Transmit packet on protecting port if any */
            }
        if (port_mask)
        {
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = port_mask;
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = 0;
            if (done)
            { 
#if defined(VTSS_ARCH_LUTON28)                  
                tx_props.packet_info.tx_done_cb         = packet_tx_done;
                tx_props.packet_info.tx_done_cb_context = done;
#endif                
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
                /* tx call back is replaced with ts call back 
                 * but a except issue without registering tx
                 * call back function. The workaourd to set
                 * tx_done_cb_context to 0 which causes nothing
                 * to do when tx call back function executes
                 */
                
                port_mask_ts = port_mask;
                rc_reg = mep_ts_two_step_cb_reg(&tx_props, port_mask_ts, done);
#endif 
            }
                 
            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
            } 
        }
    }
    if (instance_data[instance].domain == VTSS_MEP_MGMT_PORT)
    {
        for (i=0, mask = 1 << (0 + VTSS_PORT_NO_START); i<VTSS_PORT_ARRAY_SIZE; ++i, mask <<= 1)
            if (port[i])     {port_mask |= mask; break;}
        if (port_mask)
        {
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = VTSS_BIT64(i);
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = 0;
            if (done)
            {  
#if defined(VTSS_ARCH_LUTON28)
               T_D("VTSS_ARCH_LUTON28");                   
                tx_props.packet_info.tx_done_cb         = packet_tx_done;
                tx_props.packet_info.tx_done_cb_context = done;
#endif                
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
                /* tx call back is replaced with ts call back 
                 * but a except issue without registering tx
                 * call back function. The workaourd to set
                 * tx_done_cb_context to 0 which causes nothing
                 * to do when tx call back function executes
                 */
                
                port_mask_ts = 1; 
                port_mask_ts = port_mask_ts << (i + VTSS_PORT_NO_START);
                T_D("port_mask_ts: %llu",port_mask_ts);
                rc_reg = mep_ts_two_step_cb_reg(&tx_props, port_mask_ts, done);
#endif 
            }

            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
            } 
        }
    }
    CRIT_EXIT(crit_p);

    return (rc != 0);
}

u32 vtss_mep_supp_packet_tx_1_two_step_ts(u32                       instance,
                                          vtss_mep_supp_tx_done_t   *done,
                                          u32                       port,
                                          u32                       len,
                                          unsigned char             *frm)
{
    int rc = TRUE;
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)    
    u64  port_mask_ts;
#endif    
    u32  port_mask=0;
    packet_tx_props_t tx_props;
    vtss_rc rc_reg = VTSS_RC_OK;

    if (frm == NULL)     return(1);

    /* This is in order to inter-orperate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    if ((len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    CRIT_ENTER(crit_p);
    if (instance_data[instance].domain == VTSS_MEP_MGMT_EVC)
    {
        port_mask = (1 << (port + VTSS_PORT_NO_START));
        if ((out_port_conv[port] != port) && (!los_state[out_port_conv[port]]))    port_mask |= (1 << (out_port_conv[port] + VTSS_PORT_NO_START));     /* Transmit packet on protecting port if any */
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
        tx_props.packet_info.frm[0]             = frm;
        tx_props.packet_info.len[0]             = len;
        tx_props.tx_info.dst_port_mask          = port_mask;
        tx_props.packet_info.tx_done_cb         = packet_tx_done;
        tx_props.packet_info.tx_done_cb_context = 0;  
        if (done)
        {
#if defined(VTSS_ARCH_LUTON28)                    
                    tx_props.packet_info.tx_done_cb         = packet_tx_done;
                    tx_props.packet_info.tx_done_cb_context = done;
#endif                    
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
                    /* tx call back is replaced with ts call back 
                     * but a except issue without registering tx
                     * call back function. The workaourd to set
                     * tx_done_cb_context to 0 which causes nothing
                     * to do when tx call back function executes
                     */
                
                    port_mask_ts = port_mask;
                    rc_reg = mep_ts_two_step_cb_reg(&tx_props, port_mask_ts, done);
#endif      
        }
        if ((rc_reg == VTSS_RC_OK)) {      
            rc = packet_tx(&tx_props) == VTSS_RC_OK;
        } 
    }
    if (instance_data[instance].domain == VTSS_MEP_MGMT_PORT)
    {
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
        tx_props.packet_info.frm[0]             = frm;
        tx_props.packet_info.len[0]             = len;
        tx_props.tx_info.dst_port_mask          = VTSS_BIT64(port);
        tx_props.packet_info.tx_done_cb_context = 0;
        tx_props.packet_info.tx_done_cb         = packet_tx_done;
        if (done)
        {
#if defined(VTSS_ARCH_LUTON28)                   
               tx_props.packet_info.tx_done_cb         = packet_tx_done;
               tx_props.packet_info.tx_done_cb_context = done;
#endif                
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
               /* tx call back is replaced with ts call back 
                 * but a except issue without registering tx
                 * call back function. The workaourd to set
                 * tx_done_cb_context to 0 which causes nothing
                 * to do when tx call back function executes
                 */
               
                port_mask_ts = 1; 
                port_mask_ts = port_mask_ts << (port + VTSS_PORT_NO_START);
                T_D("port_mask_ts: %llu",port_mask_ts);
                rc_reg = mep_ts_two_step_cb_reg(&tx_props, port_mask_ts, done);
#endif 
        }
        
        if ((rc_reg == VTSS_RC_OK)) {      
            rc = packet_tx(&tx_props) == VTSS_RC_OK;
        } 
    }
    CRIT_EXIT(crit_p);
    return (rc != 0);
}

#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
static void packet_tx_one_step_pre_done(void          *tx_pre_contxt, /* Defined by user module                                      */
                                        unsigned char *frm,           /* Pointer to the frame to be transmitted (points to the DMAC) */
                                        size_t        len)            /* Frame length (excluding IFH, CMD, and FCS).  */

{
    vtss_mep_supp_onestep_extra_t *onestep_extra = (vtss_mep_supp_onestep_extra_t *)tx_pre_contxt;
    u32 tc, diff, diff_ns;

    tc = vtss_tod_get_ns_cnt();
    vtss_tod_ts_cnt_sub(&diff, tc, onestep_extra->tc);
    diff_ns = vtss_tod_ts_cnt_to_ns(diff);

    onestep_extra->tx_time = tc;
    onestep_extra->ts.nanoseconds += diff_ns;
    while (onestep_extra->ts.nanoseconds > 1000000000) {
        onestep_extra->ts.seconds++;
        onestep_extra->ts.nanoseconds -= 1000000000;
    }

    if (onestep_extra->sw_egress_delay)
        vtss_tod_add_TimeInterval(&onestep_extra->ts, &onestep_extra->ts, &onestep_extra->sw_egress_delay);

    vtss_tod_pack32(onestep_extra->ts.seconds, (frm + onestep_extra->corr_offset));
    vtss_tod_pack32(onestep_extra->ts.nanoseconds, (frm + onestep_extra->corr_offset + 4));
}


static void vtss_mep_ts_one_step_pre_cb_reg(vtss_mep_supp_onestep_extra_t *buf_handle, u32 hw_time)
{
    vtss_mep_supp_onestep_extra_t *onestep_extra;
    onestep_extra = (vtss_mep_supp_onestep_extra_t *)buf_handle;
    
    vtss_mep_supp_timestamp_get(&onestep_extra->ts, &onestep_extra->tc);

    /* egress latency compensation in SW */
    onestep_extra->sw_egress_delay = observed_egr_lat.min;
}                                    
#endif

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
    
static void one_step_timestamped (void *context, u32 port_no, vtss_ts_timestamp_t *ts)
{
    vtss_mep_supp_onestep_extra_t *onestep_extra = (vtss_mep_supp_onestep_extra_t *)context;
    u32 now = 0;
    u32 lat;
    vtss_timeinterval_t egr_lat;
    char buf1 [25];

    if (ts->ts_valid) {
        now = ts->ts;
        vtss_tod_ts_cnt_sub(&lat, now, onestep_extra->tx_time);
        vtss_tod_ts_cnt_to_timeinterval(&egr_lat, lat);
        T_D("MEP: Onestep data: hw_time %d, tx_time %d, now %d, xxx_lat: %d, lat %d, egr_lat %s ", 
             onestep_extra->tc, onestep_extra->tx_time, now,(onestep_extra->tx_time-onestep_extra->tc), lat, vtss_tod_TimeInterval_To_String(&egr_lat,buf1,'.'));
        
        CRIT_ENTER(crit_p);
        if (observed_egr_lat.cnt == 0) {
            observed_egr_lat.max = egr_lat;
            observed_egr_lat.min = egr_lat;
            observed_egr_lat.mean = egr_lat;
        } else {
            if (observed_egr_lat.max < egr_lat) observed_egr_lat.max = egr_lat;
            if (observed_egr_lat.min > egr_lat) observed_egr_lat.min = egr_lat;
            observed_egr_lat.mean = (observed_egr_lat.mean*observed_egr_lat.cnt + egr_lat)/(observed_egr_lat.cnt+1);
        }
        ++observed_egr_lat.cnt;
        
        //T_D("observed_egr_lat.min: %llu", observed_egr_lat.min);
        
        CRIT_EXIT(crit_p);
    } else {
        T_E("TX time stamp invalid  port %u", port_no);
    }
    
}

static vtss_rc mep_ts_one_step_timed_cb_reg(packet_tx_props_t  *tx_props,
                                            u64                port_mask,
                                            void               *context)
{
    vtss_ts_id_t              ts_id;
    vtss_rc                   rc_reg = VTSS_RC_OK;
    vtss_ts_timestamp_alloc_t alloc_parm;

    if (tx_props->tx_info.switch_frm)    return(VTSS_RC_OK);     /* This is only working for frames not switched - bypassed */

    alloc_parm.port_mask = port_mask;
    alloc_parm.context = context;
    alloc_parm.cb = one_step_timestamped;
    rc_reg = vtss_tx_timestamp_idx_alloc(0, &alloc_parm, &ts_id); /* allocate id for transmission*/

#if defined(VTSS_ARCH_LUTON26)
    /*
     * It is triky here. We don't have a real 1-step timestamp for DM in Lu26
     * so we use 2-step mechanism to simulate 1-step here. 
     *
     */
    tx_props->tx_info.ptp_id             = ts_id.ts_id;
    tx_props->tx_info.ptp_action         = VTSS_PACKET_PTP_ACTION_TWO_STEP; /* twostep action */
    tx_props->tx_info.ptp_timestamp      = 0; /* used for correction field update */
#endif


#if defined(VTSS_ARCH_JAGUAR_1)   
    /* latch register no+1: see Jaguar Datasheet, table 7, fwd.capture_tstamp */
    tx_props->tx_info.latch_timestamp = ts_id.ts_id + 1;
    //tx_props.tx_info.cos = 8; /* it is not possible to timestamp super priority queue packets */
#endif    
    return rc_reg;
}
#endif 

u32 vtss_mep_supp_packet_tx_1_one_step_ts(u32                            instance,
                                          vtss_mep_supp_tx_done_t        *done,
                                          u32                            port,
                                          u32                            len,
                                          unsigned char                  *frm,
                                          vtss_mep_supp_tx_frame_info_t  *frame_info,
                                          u32                            hw_time,
                                          vtss_mep_supp_onestep_extra_t  *buf_handle,
                                          vtss_mep_supp_oam_type_t       oam_type)
{
    /*
     *  Both JR and Lu26 don't support real 1-step timestamp. We use 
     *  2-step timestamp mechanism to simulate 1-step ts. Two call back functions
     *  are used to reach the purpose. The first one call back function is called
     *  to update the correct time when packet module will leave the s/w part.
     *  The second one is callede with the real h/w sending time, which is a 2-step 
     *  mechanism.  In the second one, we can calculate the time offset between 
     *  packet to leave s/w part(get in the first callback function) to packet
     *  to really sendout. We input this offset to the first call back function to
     *  make the timestamp more acurate.   
     */
    
    int rc = 0;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
    u64  port_mask_ts;
#endif    
    u32  port_mask=0, i, mask;
    packet_tx_props_t tx_props;
    packet_tx_pre_cb_t pre_cb=0;
    vtss_rc rc_reg = VTSS_RC_OK;

    if (frm == NULL)     return(1);

    /* This is in order to inter-orperate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    if ((len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    CRIT_ENTER(crit_p);
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) || (instance_data[instance].domain == VTSS_MEP_MGMT_VLAN))
    {
        port_mask = port_protection_mask_calc(port);
        for (i=0, mask=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)    /* Clear any port with active LOS */
            if (los_state[i])   port_mask &= ~mask;

        if (port_mask) { /* Do not transmit if mask is all zero */
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            if (oam_type == VTSS_MEP_SUPP_OAM_TYPE_NONE) {  /* Only do CB insert timestamp if NOT VOE based MEP */
                pre_cb = packet_tx_one_step_pre_done;
                buf_handle->corr_offset = TX_TIME_STAMP_B_O + buf_handle->tag_cnt * TAG_LEN;
                vtss_mep_ts_one_step_pre_cb_reg((vtss_mep_supp_onestep_extra_t *)buf_handle, hw_time);
            }
#endif
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = port_mask;
            tx_props.tx_info.switch_frm             = !frame_info->bypass;
            tx_props.tx_info.cos                    = frame_info->qos;
            tx_props.tx_info.dp                     = frame_info->dp;
            tx_props.tx_info.tag.pcp                = frame_info->pcp;
            tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
            tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
            tx_props.tx_info.tag.vid                = frame_info->vid_inj ? frame_info->vid : 0;
#if defined(VTSS_ARCH_SERVAL)
            tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif
            /* set tx_done_cb and tx_done_cb_context to 0 to prevent buffer
            released by the packet module */
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = done;
            tx_props.packet_info.tx_pre_cb          = pre_cb; /* Pre-Tx callback function */
            tx_props.packet_info.tx_pre_cb_context  = buf_handle;/* User-defined - used in pre-tx callback */

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
            port_mask_ts = port_mask;
            rc_reg = mep_ts_one_step_timed_cb_reg(&tx_props, port_mask_ts, (void *)buf_handle);
#endif  
            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
                T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                    instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                    tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                    frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
            } 
        }
    }
    if (instance_data[instance].domain == VTSS_MEP_MGMT_PORT)
    {
        if (!los_state[port]) { /* Do not transmit if LOS active on port */
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            if (oam_type == VTSS_MEP_SUPP_OAM_TYPE_NONE) {  /* Only do CB insert timestamp if NOT VOE based MEP */
                pre_cb = packet_tx_one_step_pre_done;
                buf_handle->corr_offset = TX_TIME_STAMP_B_O + buf_handle->tag_cnt * TAG_LEN;
                vtss_mep_ts_one_step_pre_cb_reg((vtss_mep_supp_onestep_extra_t *)buf_handle, hw_time);
            }
#endif
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = VTSS_BIT64(port);
            tx_props.tx_info.switch_frm             = !frame_info->bypass;
            tx_props.tx_info.cos                    = frame_info->qos;
            tx_props.tx_info.dp                     = frame_info->dp;
            tx_props.tx_info.tag.pcp                = frame_info->pcp;
            tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
            tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
#if defined(VTSS_ARCH_SERVAL)
            tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif
            /* set tx_done_cb and tx_done_cb_context to 0 to prevent buffer
            released by the packet module */
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = done;  
            tx_props.packet_info.tx_pre_cb          = pre_cb; /* Pre-Tx callback function */
            tx_props.packet_info.tx_pre_cb_context  = buf_handle;/* User-defined - used in pre-tx callback */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
            port_mask_ts = 1;
            port_mask_ts = port_mask_ts << (port + VTSS_PORT_NO_START);
            rc_reg = mep_ts_one_step_timed_cb_reg(&tx_props, port_mask_ts, (void *)buf_handle);
#endif            
            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
                T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                    instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                    tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                    frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
            } 
        }
    }
    CRIT_EXIT(crit_p);
    return (rc != 0);
}

u32 vtss_mep_supp_packet_tx_one_step_ts(u32                           instance,
                                        vtss_mep_supp_tx_done_t       *done,
                                        BOOL                          port[VTSS_PORT_ARRAY_SIZE],
                                        u32                           len,
                                        unsigned char                 *frm,
                                        vtss_mep_supp_tx_frame_info_t *frame_info,
                                        u32                           hw_time,
                                        vtss_mep_supp_onestep_extra_t *buf_handle,
                                        vtss_mep_supp_oam_type_t      oam_type)
{
    /*
     *  Both JR and Lu26 don't support real 1-step timestamp. We use 
     *  2-step timestamp mechanism to simulate 1-step ts. Two call back functions
     *  are used to reach the purpose. The first one call back function is called
     *  to update the correct time when packet module will leave the s/w part.
     *  The second one is callede with the real h/w sending time, which is a 2-step 
     *  mechanism.  In the second one, we can calculate the time offset between 
     *  packet to leave s/w part(get in the first callback function) to packet
     *  to really sendout. We input this offset to the first call back function to
     *  make the timestamp more acurate.   
     */
    
    int rc = 0;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
    u64  port_mask_ts;
#endif    
    u32  i, port_mask=0, mask;
    packet_tx_props_t tx_props;
    packet_tx_pre_cb_t pre_cb=0;
    vtss_rc rc_reg = VTSS_RC_OK;
    vtss_packet_port_info_t   info;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];

    if (frm == NULL)     return(1);

    /* This is in order to inter-orperate with vendors that require tagged frames to be transmitted with minimum length of 68 bytes - four bytes will be added by packet_tx() */
    if ((len < 64) && ((frm[12] == 0x81) || (frm[12] == 0x88)))   len = 64;

    if (vtss_packet_port_info_init(&info) != VTSS_RC_OK)
        return(1);

    info.vid = frame_info->vid;
    if (vtss_packet_port_filter_get(NULL, &info, filter) != VTSS_RC_OK)
        return(1);

    CRIT_ENTER(crit_p);
    if ((instance_data[instance].domain == VTSS_MEP_MGMT_EVC) || (instance_data[instance].domain == VTSS_MEP_MGMT_VLAN))
    {
        for (i=0, mask=1; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1)
            if (port[i] && !los_state[i] && (!frame_info->bypass || (filter[i].filter != VTSS_PACKET_FILTER_DISCARD)))
                port_mask |= mask;

        if (port_mask) { /* Do not transmit if mask is all zero */
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            if (oam_type == VTSS_MEP_SUPP_OAM_TYPE_NONE) {  /* Only do CB insert timestamp if NOT VOE based MEP */
                pre_cb = packet_tx_one_step_pre_done;
                buf_handle->corr_offset = TX_TIME_STAMP_F_O + buf_handle->tag_cnt * TAG_LEN;
                vtss_mep_ts_one_step_pre_cb_reg((vtss_mep_supp_onestep_extra_t *)buf_handle, hw_time);
            }
#endif
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = port_mask;
            tx_props.tx_info.switch_frm             = !frame_info->bypass;
            tx_props.tx_info.cos                    = frame_info->qos;
            tx_props.tx_info.dp                     = frame_info->dp;
            tx_props.tx_info.tag.pcp                = frame_info->pcp;
            tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
            tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
            tx_props.tx_info.tag.vid                = frame_info->vid_inj ? frame_info->vid : 0;
#if defined(VTSS_ARCH_SERVAL)
            tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif
           /* set tx_done_cb and tx_done_cb_context to 0 to prevent buffer
              released by the packet module */
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = done;  

            /* add 1-step call back to upper-layer */
            tx_props.packet_info.tx_pre_cb          = pre_cb; /* Pre-Tx callback function */
            tx_props.packet_info.tx_pre_cb_context  = buf_handle;/* User-defined - used in pre-tx callback */
               
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
            port_mask_ts = port_mask;
            rc_reg = mep_ts_one_step_timed_cb_reg(&tx_props, port_mask_ts, (void *)buf_handle);
#endif  
            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
                T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                    instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                    tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                    frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
            }
        }
    }
    if (instance_data[instance].domain == VTSS_MEP_MGMT_PORT)
    {
        for (i=0, mask = 1 << (0 + VTSS_PORT_NO_START); i<VTSS_PORT_ARRAY_SIZE; ++i, mask <<= 1)
            if (port[i] && !los_state[i])    {port_mask |= mask; break;}

        if (port_mask)
        {
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            if (oam_type == VTSS_MEP_SUPP_OAM_TYPE_NONE) {  /* Only do CB insert timestamp if NOT VOE based MEP */
                pre_cb = packet_tx_one_step_pre_done;
                buf_handle->corr_offset = TX_TIME_STAMP_F_O + buf_handle->tag_cnt * TAG_LEN;
                vtss_mep_ts_one_step_pre_cb_reg((vtss_mep_supp_onestep_extra_t *)buf_handle, hw_time);
            }
#endif
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid              = VTSS_MODULE_ID_MEP;
            tx_props.packet_info.frm[0]             = frm;
            tx_props.packet_info.len[0]             = len;
            tx_props.tx_info.dst_port_mask          = VTSS_BIT64(i);
            tx_props.tx_info.switch_frm             = !frame_info->bypass;
            tx_props.tx_info.cos                    = frame_info->qos;
            tx_props.tx_info.dp                     = frame_info->dp;
            tx_props.tx_info.tag.pcp                = frame_info->pcp;
            tx_props.tx_info.isdx                   = (frame_info->isdx != VTSS_MEP_SUPP_INDX_INVALID) ? frame_info->isdx : VTSS_ISDX_NONE;
            tx_props.tx_info.masquerade_port        = frame_info->maskerade_port;
#if defined(VTSS_ARCH_SERVAL)
            tx_props.tx_info.oam_type               = oam_type_calc(oam_type);
#endif
            /* set tx_done_cb and tx_done_cb_context to 0 to prevent buffer
               released by the packet module */
            tx_props.packet_info.tx_done_cb         = packet_tx_done;
            tx_props.packet_info.tx_done_cb_context = done;  
            tx_props.packet_info.tx_pre_cb          = pre_cb; /* Pre-Tx callback function */
            tx_props.packet_info.tx_pre_cb_context  = buf_handle;/* User-defined - used in pre-tx callback */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
            port_mask_ts = 1;
            port_mask_ts = port_mask_ts << (i + VTSS_PORT_NO_START);
            //T_D("port_mask_ts: %llu",port_mask_ts);
            rc_reg = mep_ts_one_step_timed_cb_reg(&tx_props, port_mask_ts, (void *)buf_handle);
#endif 
            if ((rc_reg == VTSS_RC_OK)) {      
                rc = packet_tx(&tx_props) == VTSS_RC_OK;
                T_DG(TRACE_GRP_TX_FRAME, "TX frame:  instance %u  port_mask 0x%llX  isdx %u  vid %u  qos %u  pcp %u  dp %u  len %u  maskarade %u  switched %u  oam_type %u  frame %X-%X-%X-%X-%X-%X-%X-%X-%X",
                    instance, tx_props.tx_info.dst_port_mask, tx_props.tx_info.isdx, tx_props.tx_info.tag.vid, tx_props.tx_info.cos, tx_props.tx_info.tag.pcp,
                    tx_props.tx_info.dp, tx_props.packet_info.len[0], tx_props.tx_info.masquerade_port, tx_props.tx_info.switch_frm, tx_props.tx_info.oam_type,
                    frm[12+0], frm[12+1], frm[12+2], frm[12+3], frm[12+4], frm[12+5], frm[12+6], frm[12+7], frm[12+8]);
            }
        }
    }
 
    CRIT_EXIT(crit_p);

    return (rc != 0);
}


void vtss_mep_supp_timestamp_get(vtss_timestamp_t *timestamp, u32 *tc)
{
#if 1
    vtss_tod_gettimeofday(timestamp, tc); 
#else
    vtss_tod_ts_to_time(vtss_tod_get_ts_cnt(), (vtss_timestamp_t *)timestamp);
#endif
}

BOOL vtss_mep_supp_check_hw_timestamp(void)
{
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
    T_D("vtss_mep_supp_check_hw_timestamp: TRUE");
    return TRUE;  
#else    
    return FALSE;
#endif    
}    

BOOL vtss_mep_supp_check_phy_timestamp(u32 port, u8 *mac)
{
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    /* Only unicast is supported */
    if ((mac[0] & 0x1) == 0) {
        return (mep_phy_ts_port[port].phy_ts_port);
    } else {
        return FALSE;    
    }              
#else
    return FALSE;
#endif //#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
}


/* The unit of the return value is nanosecond or microsecond */
i32 vtss_mep_supp_delay_calc(const vtss_timestamp_t *x,
                             const vtss_timestamp_t *y,
                             const BOOL             is_ns)
{
    i32          diff=0;
    vtss_timeinterval_t deltat;

    T_D("is_ns: %d", is_ns);

    vtss_tod_sub_TimeInterval(&deltat, x, y);
    
    diff = deltat / (1<< 16);
    if (diff < 0)
    {
        T_D("vtss_mep_supp_delay_calc: diff < 0\n");
        T_D("rx_time:(%u, %u)  tx_time(%u, %u)\n",
            x->seconds, x->nanoseconds, y->seconds, y->nanoseconds);
    }

    if (is_ns) 
    {    
        return diff;
    }
    else
    {    
        diff = diff / 1000;
        if ((diff % 1000) > 500)
            diff++; /*round up*/
        return diff;
    }  
}

BOOL vtss_mep_supp_check_forwarding(u32 i_port,  u32 *f_port,  u8 *mac,  u32 vid)
{
    u32                     i, dest;
    vtss_vid_mac_t          vid_mac;
    vtss_mac_table_entry_t  entry;

    memset(&entry, 0, sizeof(entry));
    vid_mac.vid = vid;
    memcpy(vid_mac.mac.addr, mac, sizeof(vid_mac.mac.addr));

    if (vtss_mac_table_get(NULL,  &vid_mac,  &entry) != VTSS_RC_OK)   return(FALSE);

    /* 'mac' found in MAC table */
    if (entry.locked)     /* It must be an dynamic entry */
        if (vtss_mac_table_get_next(NULL,  &vid_mac,  &entry) != VTSS_RC_OK)   return(FALSE);

    if (entry.locked)   return(FALSE);
    /* 'mac' found in MAC table and it is dynamic */

    /* Find and count destination ports */
    for (i=0, dest=0; i<VTSS_PORT_ARRAY_SIZE; ++i)
    {
        if ((entry.destination[i]) && (in_port_conv[i] != 0xFFFF))
        {
            dest++;
            *f_port = i;
        }
    }

    if (dest != 1)            return(FALSE);    /* Only forward if one destination port is found */
    if (*f_port == i_port)    return(FALSE);    /* Only forward if destination port is different from ingress port */

    return(TRUE);
}


/****************************************************************************/
/*  MEP Initialize module                                                   */
/****************************************************************************/
#if defined(VTSS_SW_OPTION_EVC)
static void evc_change_callback(u16  evc_id)
{
    u32                          i, j, vid, evid, tvid, tag_cnt;
    vtss_mep_tag_type_t          ttype;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    BOOL                         delete_, tunnel;
    vtss_mep_mgmt_conf_t         conf;
    vtss_mep_mgmt_ais_conf_t     ais_conf;
    vtss_mep_mgmt_lck_conf_t     lck_conf;
    vtss_mep_mgmt_client_conf_t  client_conf;
    BOOL                         nni[VTSS_PORT_ARRAY_SIZE];
    BOOL                         uni[VTSS_PORT_ARRAY_SIZE];

    delete_ = !vtss_mep_evc_flow_info_get(evc_id,  nni,  uni,  &vid,  &evid, &ttype, &tvid, &tunnel, &tag_cnt);

    T_D("evc_id %u   delete %u\n", evc_id, delete_);

    CRIT_ENTER(crit_p);
    for(i=0; i<EVC_VLAN_VID_SIZE; ++i)   /* Update the register of what EVC the VID is related to */
        if (evc_vlan_vid[i] == evc_id)   {evc_vlan_vid[i] = 0xFFFFFFFF; break;}
    if (!delete_)    evc_vlan_vid[vid] = evc_id;

    for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
    {
        if (instance_data[i].enable && (instance_data[i].flow == evc_id) && (instance_data[i].domain == VTSS_MEP_MGMT_EVC))
        { /* This is a EVC MEP related to this EVC ID */
            CRIT_EXIT(crit_p);
            if (vtss_mep_mgmt_conf_get(i, mac, &conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");

            if (delete_ || (!nni[conf.port] && !uni[conf.port]))
            {/* This is a delete of an EVC or this MEP is no longer on a EVC port - delete MEP in database */
                conf.enable = FALSE;
                if (mep_mgmt_conf_set(i, &conf) != VTSS_RC_OK)  T_D("Error during call to MEP base");
            }
            else
            {/* This is some change in an EVC - re-create MEP/MIP bypassing database */
                if (vtss_mep_mgmt_change_set(i) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");
            }
            CRIT_ENTER(crit_p);
        }
        if (instance_data[i].enable && (instance_data[i].domain == VTSS_MEP_MGMT_PORT))
        { /* This is a Port MEP */
            CRIT_EXIT(crit_p);
            if (vtss_mep_mgmt_client_conf_get(i, &client_conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");
            if (vtss_mep_mgmt_ais_conf_get(i, &ais_conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");

            if ((ais_conf.enable) && (client_conf.domain == VTSS_MEP_MGMT_EVC))
            { /* AIS is enabled - check for AIS client EVC ID  */
                for (j=0; j<client_conf.flow_count; ++j)    if (client_conf.flows[j] == evc_id)     break;
                if (j < client_conf.flow_count)
                    if (vtss_mep_mgmt_ais_conf_set(i, &ais_conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");
            }

            if (vtss_mep_mgmt_lck_conf_get(i, &lck_conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");

            if ((lck_conf.enable) && (client_conf.domain == VTSS_MEP_MGMT_EVC))
            { /* LCK is enabled - check for LCK client EVC ID  */
                for (j=0; j<client_conf.flow_count; ++j)    if (client_conf.flows[j] == evc_id)     break;
                if (j < client_conf.flow_count)
                    if (vtss_mep_mgmt_lck_conf_set(i, &lck_conf) != VTSS_MEP_RC_OK)  T_D("Error during call to MEP base");
            }
            CRIT_ENTER(crit_p);
        }
    }
    CRIT_EXIT(crit_p);
}
#endif



static void port_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    T_D("Port %u   Los %u\n", port_no, !info->link);

    los_state_set(port_no, (info->link) ? FALSE : TRUE);
}

typedef struct
{
    vlan_port_type_t   port_type;
    vtss_vid_t         pvid;
} mep_vlan_conf_t;
static mep_vlan_conf_t vlan_port_conf[VTSS_PORT_ARRAY_SIZE];

static void vlan_port_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, const vlan_port_conf_t *new_conf)
{
    u32 i;
#if defined(VTSS_ARCH_SERVAL)
    u32      base_rc;
    u8       mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_conf_t  mep_conf;
#endif
    CRIT_ENTER(crit_p);

    T_D("port_no %u  new port_type %u  old port_type %u", port_no, new_conf->port_type, vlan_port_conf[port_no].port_type);

    if (vlan_port_conf[port_no].port_type != new_conf->port_type) {
        vlan_port_conf[port_no].port_type = new_conf->port_type;

        for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
        {
            if (instance_data[i].enable)
            {
                if (instance_data[i].port[port_no]) {
                    CRIT_EXIT(crit_p);
                    /* This is some change in vlan port configuration - re-create MEP/MIP bypassing database */
                    (void)vtss_mep_mgmt_change_set(i);
                    CRIT_ENTER(crit_p);
                }
#if defined(VTSS_ARCH_SERVAL)
                else if ((instance_data[i].res_port == port_no) && ((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN))) {
                    CRIT_EXIT(crit_p);
                    base_rc = vtss_mep_mgmt_conf_get(i, mac, &mep_conf);
                    if ((mep_conf.mode == VTSS_MEP_MGMT_MEP) && (mep_conf.direction == VTSS_MEP_MGMT_UP))  /* This is a Up-MEP on this port */
                        if (base_rc == VTSS_MEP_RC_OK)    base_rc += vtss_mep_mgmt_change_set(i);
                    CRIT_ENTER(crit_p);
                }
#endif
            }
        }
    }

    if (vlan_port_conf[port_no].pvid != new_conf->pvid) {
        vlan_port_conf[port_no].pvid = new_conf->pvid;

        for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
        {
            if (instance_data[i].enable && (instance_data[i].domain == VTSS_MEP_MGMT_PORT))
            {
                if (instance_data[i].port[port_no]) {
                    CRIT_EXIT(crit_p);
                    /* This is some change in vlan port configuration - re-create MEP/MIP bypassing database */
                    (void)vtss_mep_mgmt_change_set(i);
                    CRIT_ENTER(crit_p);
                }
            }
        }
    }
    CRIT_EXIT(crit_p);
}

static void vlan_custom_ethertype_callback(void)
{
    u32                   i, j;
    vtss_vlan_port_conf_t vlan_conf;
#if defined(VTSS_ARCH_SERVAL)
    u8                    mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_conf_t  mep_conf;
#endif

    T_D("Enter");

    CRIT_ENTER(crit_p);
    for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
    {
        if (instance_data[i].enable)
        {
            for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j) { /* Check ports to see if they are 'Custom S Port' */
                if (instance_data[i].port[j]) {
                    if (vtss_vlan_port_conf_get(NULL, j, &vlan_conf) != VTSS_MEP_RC_OK) /* Get the VLAN port type */
                        continue;
                    if (vlan_conf.port_type != VTSS_VLAN_PORT_TYPE_S_CUSTOM)   /* If the port is Custom S the the MEP must be reconfigured */
                        continue;
                    CRIT_EXIT(crit_p);  /* Reconfigure */
                    (void)vtss_mep_mgmt_change_set(i);
                    CRIT_ENTER(crit_p);
                }
#if defined(VTSS_ARCH_SERVAL)
                else if ((instance_data[i].res_port == j) && ((instance_data[i].domain == VTSS_MEP_MGMT_EVC) || (instance_data[i].domain == VTSS_MEP_MGMT_VLAN))) {
                    CRIT_EXIT(crit_p);
                    if (vtss_mep_mgmt_conf_get(i, mac, &mep_conf) == VTSS_MEP_RC_OK) {  /* Get MEP instance configuration */
                        if ((mep_conf.mode == VTSS_MEP_MGMT_MEP) && (mep_conf.direction == VTSS_MEP_MGMT_UP)) {  /* This is a Up-MEP on this port */
                            if (vtss_vlan_port_conf_get(NULL, j, &vlan_conf) == VTSS_MEP_RC_OK) {   /* Get the VLAN port type */
                                if (vlan_conf.port_type == VTSS_VLAN_PORT_TYPE_S_CUSTOM)   /* If the port is Custom S the the MEP must be reconfigured */
                                    (void)vtss_mep_mgmt_change_set(i);
                            }
                        }
                    }
                    CRIT_ENTER(crit_p);
                }
#endif
            }
        }
    }
    CRIT_EXIT(crit_p);
}

#if defined(VTSS_ARCH_SERVAL)
static void vlan_membership_change_callback(vtss_isid_t isid, vtss_vid_t vid, vlan_membership_change_t *changes)
{
    u32                  i, j, res_port;
    BOOL                 nni_found = TRUE;
    u8                   mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_conf_t conf;
    vlan_ports_t         resulting;

    T_D("vid %u static_exists %u", vid, changes->static_vlan_exists);

    // Resulting configuration for static user must be computed based on both the static and the forbidden VLAN users' configuration.
    for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
        // Clear those bits in static ports that are forbidden.
        resulting.ports[i] = changes->static_ports.ports[i] & ~changes->forbidden_ports.ports[i];
    }

    CRIT_ENTER(crit_p);

    T_D("evc_vlan_vid[vid] %X", evc_vlan_vid[vid]);

    if (evc_vlan_vid[vid] >= VTSS_EVCS) {
        // The VID is registered as a "VLAN" VID. This has implications when frames are received through a port protection
        // The static user may or may not have members set, independent of whether the VLAN has been created
        // by him. Therefore, it's not good enough to look at changes->static_ports to check whether it has bits set or not.
        evc_vlan_vid[vid] = changes->static_vlan_exists ? VTSS_EVCS : 0xFFFFFFFF;
    }

    for (i = 0; i < VTSS_MEP_INSTANCE_MAX; i++) {
        if ((instance_data[i].enable) && (instance_data[i].domain == VTSS_MEP_MGMT_VLAN) && (instance_data[i].flow == vid)) {
            /* This is a VLAN MEP in this VID */
            res_port = instance_data[i].res_port;

            CRIT_EXIT(crit_p);
            if (vtss_mep_mgmt_conf_get(i, mac, &conf) == VTSS_RC_OK) {
                if (conf.direction == VTSS_MEP_MGMT_UP) {   /* An Up-MEP requires a "NNI" port*/
                    nni_found = FALSE;
                    for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j) {
                        if ((j != res_port) && VTSS_PORT_BF_GET(resulting.ports, j)) {
                            nni_found = TRUE;     /* 'NNI port was found */
                            break;
                        }
                    }
                }

                if (nni_found && VTSS_PORT_BF_GET(resulting.ports, res_port)) {
                    /* This MEP is in this VLAN */
                    if (vtss_mep_mgmt_change_set(i) != VTSS_RC_OK) {
                        T_D("Error during call to MEP base");
                    }
                } else if (conf.mode == VTSS_MEP_MGMT_MEP) {
                    /* This MEP is no longer on a VLAN port - delete MEP in database */
                    conf.enable = FALSE;
                    if (mep_mgmt_conf_set(i, &conf) != VTSS_RC_OK) {
                        T_D("Error during call to MEP base");
                    }
                }
            }
            CRIT_ENTER(crit_p);
        }
    }

    CRIT_EXIT(crit_p);
}

typedef struct
{
    vtss_prio_t            qos_class_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE]; /* Ingress mapping for tagged frames from PCP and DEI to QOS class  */
    vtss_dp_level_t        dp_level_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE];  /* Ingress mapping for tagged frames from PCP and DEI to DP level */
    vtss_tagprio_t         tag_pcp_map[QOS_PORT_PRIO_CNT][2];                       /* Egress mapping from QOS class and DP level to PCP */
} mep_qos_conf_t;
static mep_qos_conf_t qos_port_conf[VTSS_PORT_ARRAY_SIZE];

static void qos_port_change_callback(const vtss_isid_t isid, const vtss_port_no_t iport, const qos_port_conf_t *const conf)
{
    u32                  i, base_rc;
    u8                   mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_conf_t mep_conf;

    T_D("iport %u", iport);

    CRIT_ENTER(crit_p);
    if (memcmp(conf->qos_class_map, qos_port_conf[iport].qos_class_map, sizeof(conf->qos_class_map)) ||
        memcmp(conf->dp_level_map, qos_port_conf[iport].dp_level_map, sizeof(conf->dp_level_map))) {

        memcpy(qos_port_conf[iport].qos_class_map, conf->qos_class_map, sizeof(qos_port_conf[iport].qos_class_map));
        memcpy(qos_port_conf[iport].dp_level_map, conf->dp_level_map, sizeof(qos_port_conf[iport].dp_level_map));

        for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
        {
            if (instance_data[i].enable && (instance_data[i].res_port == iport) && (instance_data[i].domain == VTSS_MEP_MGMT_EVC))
            {
                CRIT_EXIT(crit_p);
                base_rc = vtss_mep_mgmt_conf_get(i, mac, &mep_conf);
                if ((mep_conf.mode == VTSS_MEP_MGMT_MEP) && (mep_conf.direction == VTSS_MEP_MGMT_UP))  /* This is a Up-MEP on this port */
                    if (base_rc == VTSS_MEP_RC_OK)    base_rc += vtss_mep_mgmt_change_set(i);
                CRIT_ENTER(crit_p);
            }
        }
    }

    if (memcmp(conf->tag_pcp_map, qos_port_conf[iport].tag_pcp_map, sizeof(conf->tag_pcp_map))) {

        memcpy(qos_port_conf[iport].tag_pcp_map, conf->tag_pcp_map, sizeof(qos_port_conf[iport].tag_pcp_map));

        for (i=0; i<VTSS_MEP_INSTANCE_MAX; ++i)
        {
            if (instance_data[i].enable && (instance_data[i].port[iport]) && (instance_data[i].domain == VTSS_MEP_MGMT_EVC))
            {
                CRIT_EXIT(crit_p);
                base_rc = vtss_mep_mgmt_conf_get(i, mac, &mep_conf);
                if (!mep_conf.voe && (mep_conf.mode == VTSS_MEP_MGMT_MEP) && (mep_conf.direction == VTSS_MEP_MGMT_DOWN))   /* This is a SW EVC Down-MEP on this port */
                    if (base_rc == VTSS_MEP_RC_OK)    base_rc += vtss_mep_mgmt_change_set(i);
                CRIT_ENTER(crit_p);
            }
        }
    }

    CRIT_EXIT(crit_p);
}
#endif

char *mep_error_txt(vtss_rc error)
{
    switch (error) {
    case MEP_RC_APS_PROT_CONNECTED:     return "Mismatch between protection type and APS type";
    case MEP_RC_INVALID_PARAMETER:      return "Invalid parameter error returned from MEP";
    case MEP_RC_NOT_ENABLED:            return "MEP instance is not enabled";
    case MEP_RC_CAST:                   return "LM and CC sharing SW generated CCM must have same casting (multi/uni)";
    case MEP_RC_PERIOD:                 return "LM and CC sharing SW generated CCM must have same period";
    case MEP_RC_PEER_CNT:               return "Invalid number of peer's for this configuration";
    case MEP_RC_PEER_ID:                return "Invalid peer MEP ID";
    case MEP_RC_MIP:                    return "Not allowed on a MIP";
    case MEP_RC_INVALID_EVC:            return "EVC flow was found invalid";
    case MEP_RC_APS_UP:                 return "APS not allowed on EVC UP MEP";
    case MEP_RC_APS_DOMAIN:             return "R-APS not allowed in this domain";
    case MEP_RC_INVALID_VID:            return "VLAN is not created for this VID";
    case MEP_RC_INVALID_COS_ID:         return "Invalid COS ID (priority) for this EVC";
    case MEP_RC_NO_VOE:                 return "No VOE available";
    case MEP_RC_NO_TIMESTAMP_DATA:      return "There is no DMR timestamp data available";
    case MEP_RC_PEER_MAC:               return "Peer Unicast MAC must be known to do HW based CCM on SW MEP";
    case MEP_RC_INVALID_INSTANCE:       return "Invalid MEP instance ID";
    case MEP_RC_INVALID_MEG:            return "Invalid MEG-ID or IEEE Name";
    case MEP_RC_PROP_SUPPORT:           return "Propritary DM is not supported";
    case MEP_RC_VOLATILE:               return "Cannot change volatile entry";
    case MEP_RC_VLAN_SUPPORT:           return "VLAN domain is not supported";
    case MEP_RC_CLIENT_MAX_LEVEL:       return "The MEP is on MAX level (7) - it is not possible to have a client on higher level";
    case MEP_RC_MIP_SUPPORT:            return "This MIP is not supported";
    // By not specifying a "default:" we get lint or compilerwarnings when not all are specified.
    }

    return "Unknown error code";
}

static void link_interrupt_function(vtss_interrupt_source_t        source_id,
                                    u32                            instance_id)
{
    vtss_rc rc;

//printf("link_interrupt_function  source_id %u  instance_id %u\n", source_id, instance_id);
    rc = vtss_interrupt_source_hook_set(link_interrupt_function,
                                        source_id,
                                        INTERRUPT_PRIORITY_PROTECT);
    if (rc != VTSS_RC_OK)       T_D("Error during interrupt hook");

    los_state_set(instance_id, TRUE);
}


static BOOL rx_frame(void *contxt, const u8 *const frame, const vtss_packet_rx_info_t *const rx_info)
{
/* This is only for L28 - frame is received with stripped TAG */
    u32                        port_conv;
    u32                        tl_idx;
    vtss_timestamp_t           rx_time;
    vtss_mep_supp_frame_info_t frame_info;
    vtss_timestamp_t           timee;
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_ts_timestamp_t ts;
    BOOL timestamp_ok;
#endif
#if defined(VTSS_ARCH_LUTON26)
    vtss_ts_id_t ts_id;
    vtss_ts_timestamp_t ts;
    BOOL timestamp_ok;
#endif
#if defined(VTSS_ARCH_SERVAL)
    BOOL timestamp_ok;
#endif
    u32                 rx_count=0;
#if defined(VTSS_ARCH_SERVAL)
//    vtss_ts_id_t        ts_id;
//    vtss_ts_timestamp_t ts;
    u32 voe_idx;
    vtss_oam_ts_id_t          oam_ts_id;
    vtss_oam_ts_timestamp_t   oam_ts;
#endif

    tl_idx = 12;
    if ((frame[tl_idx] == 0x81) && (frame[tl_idx+1] == 0x00))   /* If there is a not stripped tag in the frame we only accept C and S tag. Also the Ether type must be 0x8902 */
        tl_idx += 4;
    else
    if ((frame[tl_idx] == 0x88) && (frame[tl_idx+1] == 0xA8))
        tl_idx += 4;
    if ((frame[tl_idx] != 0x89) || (frame[tl_idx+1] != 0x02))     return(FALSE);

    T_DG(TRACE_GRP_RX_FRAME, "Frame:  port %u  vid %u  isdx %u  tagged %u  qos %u  pcp %u  dei %u  len %u  count %u  smac %X-%X  level %u  pdu %u  flags %X",
    rx_info->port_no, rx_info->tag.vid, rx_info->isdx, (rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED) ? TRUE : FALSE, rx_info->cos, rx_info->tag.pcp, rx_info->tag.dei, rx_info->length, rx_count, frame[10], frame[11], frame[tl_idx+2]>>5, frame[tl_idx+3], frame[tl_idx+4]);

    memset(&timee, 0, sizeof(timee));
    memset(&rx_time, 0, sizeof(rx_time));
    memset(&frame_info, 0, sizeof(frame_info));

    //T_D("rx_frame  tag_type %u\n", rx_info->tag_type);
    if ((frame[tl_idx+3] == 45) || (frame[tl_idx+3] == 46) || (frame[tl_idx+3] == 47))
    { /* Only get timestamp if it is a delay measurement frame */
#if defined(VTSS_ARCH_LUTON28)
        vtss_tod_ts_to_time(rx_info->sw_tstamp.hw_cnt, &timee);
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
        ts.ts = rx_info->hw_tstamp;
        timestamp_ok = rx_info->hw_tstamp_decoded;
        T_D("Raw timestamp %u, hw_tstamp_decoded %d", ts.ts, timestamp_ok);
        if (VTSS_RC_OK == vtss_rx_master_timestamp_get(0, rx_info->port_no, &ts) && ts.ts_valid) {
            vtss_tod_ts_to_time(ts.ts, &timee);
        } else {
            T_E("No valid timestamp for port: %d", rx_info->port_no);
        }
#endif
#if defined(VTSS_ARCH_LUTON26)
        ts_id.ts_id = rx_info->tstamp_id;
        timestamp_ok = rx_info->tstamp_id_decoded;
        T_DG(TRACE_GRP_RX_FRAME, "Timestamp_id %d, Timestamp_decoded %d", ts_id.ts_id, timestamp_ok);
        if (VTSS_RC_OK == vtss_rx_timestamp_get(0,&ts_id,&ts) && ts.ts_valid) {
            vtss_tod_ts_to_time(ts.ts, &timee);
        } else {
            //James !!! not fixed yet
            //vtss_tod_ts_to_time(ts.ts, (TimeStamp *)&rx_time);
            //T_E("timestamp_hw: Sec:%lu  NanoSec:%lu", rx_time.seconds, rx_time.nanoseconds);
            vtss_tod_ts_to_time(vtss_tod_get_ts_cnt(), &timee);
            T_DG(TRACE_GRP_RX_FRAME, "No valid timestamp detected for id: %d, ts_valid:%d",ts_id.ts_id, ts.ts_valid);
            //T_E("timestamp_sw: Sec:%lu  NanoSec:%lu", rx_time.seconds, rx_time.nanoseconds);
        }
#endif
#if defined(VTSS_ARCH_SERVAL)
        if (rx_info->hw_tstamp_decoded) { /* This is timestamp captured by hit in S2 - PTP two-step */
            vtss_tod_ts_to_time(rx_info->hw_tstamp, &timee);
        }
        else {
            if (rx_info->oam_info_decoded) { /* VOE - TS is in FIFO or in .oam_info */
                if (!vtss_mep_supp_voe_up(rx_info->isdx, &voe_idx)) { /* This is not an UP-MEP. The .oam_info member contains a timestamp */
                    vtss_tod_ts_to_time(rx_info->oam_info, &timee);
                }
                else { /* This is an UP-MEP. The .oam_info member contains a timestamp FIFO sequence number */
                    oam_ts_id.voe_id = voe_idx;
                    oam_ts_id.voe_sq = rx_info->oam_info>>27;
                    T_DG(TRACE_GRP_RX_FRAME, "UP-MEP voe_id %u, voe_sq %u", oam_ts_id.voe_id, oam_ts_id.voe_sq);
                    oam_ts.ts_valid = FALSE;
                    (void)vtss_oam_timestamp_get(NULL, &oam_ts_id, &oam_ts);

                    if (!oam_ts.ts_valid) {
                        T_DG(TRACE_GRP_RX_FRAME, "UP-MEP Time stamp invalid");
                        return(TRUE);   /* Discard frame if no valid timestamp was found */
                    }
                    vtss_tod_ts_to_time(oam_ts.ts, &timee);
                }
            }
        }
#endif
        rx_time = timee;
        T_DG(TRACE_GRP_RX_FRAME, "sec %u  nsec %u", rx_time.seconds, rx_time.nanoseconds);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        /* According to the standard, rx timestamp for 1dm,dmm and dmr must be 0.
         * If it is not zero, use it as the rx timestamp. Only support ucast 
         * solution at this moment so it is the only way to check if PHY
         * timestamp is used. For multicast DM packets, MAC timestamp is
         * still used  
         */ 
        CRIT_ENTER(crit_p);
        if (mep_phy_ts_port[rx_info->port_no].phy_ts_port == TRUE) {
            u8  ts_offset;
            u32 sec, nanosec;
            
            if ((frame[tl_idx+3] == 45) || (frame[tl_idx+3] == 47)) {
                /* 1DM and DMM(opcode), the rx time offset is 11 bytes after opcode */
                ts_offset = tl_idx + 3 + 11; 
            } else {
                /* DMR(opcode), the rx time offset is 27 bytes after opcode */
                ts_offset = tl_idx + 3 + 27; 
            }        
            sec = vtss_tod_unpack32(&frame[ts_offset]); //tshw
            nanosec = vtss_tod_unpack32(&frame[ts_offset + 4]); //tshw
            
            if (sec != 0 || nanosec != 0) {
                rx_time.seconds = sec;
                rx_time.nanoseconds = nanosec;
                timestamp_ok = TRUE;
                T_D("rx_frame: PHY timestamp get, timestamp_ok:%d, ts_offset%d", timestamp_ok, ts_offset);   
            } else {
#if DEBUG_MORE_INFO                 
                T_E("*** rx_frame: PHY timestamp not get  *** ");         
#endif                
            }     
        } else {
#if DEBUG_MORE_INFO             
            T_E(" *** rx_frame: port %ld not support PHY timestamp   *** ", rx_info->port_no);
#endif             
        }             
        CRIT_EXIT(crit_p);
#endif
    }

    if ((frame[tl_idx+3] == 1) || (frame[tl_idx+3] == 42) || (frame[tl_idx+3] == 43))
    { /* Only get rx frame count if it is a LM frame */
#if defined(VTSS_ARCH_SERVAL)
        if (rx_info->oam_info_decoded) {
            rx_count = rx_info->oam_info;
            frame_info.oam_info = rx_info->oam_info;
        }
#endif
    }

    port_conv = rx_info->port_no;
    if ((rx_info->tag_type == VTSS_TAG_TYPE_S_TAGGED) || (rx_info->tag_type == VTSS_TAG_TYPE_C_TAGGED) || (rx_info->tag_type == VTSS_TAG_TYPE_S_CUSTOM_TAGGED))
    {   /* Received with a C or S TAG. Customer tagged will be treaded as S-tagged */
#if defined(VTSS_SW_OPTION_EVC)
        port_conv = rx_info->port_no;
        BOOL evc=FALSE;
        CRIT_ENTER(crit_p);
        if (evc_vlan_vid[rx_info->tag.vid] != 0xFFFFFFFF)      /* Check if this VID is EVC */
        { /* This VID is a EVC */
            evc = TRUE;
            port_conv = in_port_conv[rx_info->port_no];
        }
        CRIT_EXIT(crit_p);
        if (evc)
        {/* This is EVC VID - check if is should be discarded */
            if (port_conv == 0xFFFF)       return(TRUE);   /* This port is protected by other - discard */
        }
#endif
    }
    else {
    }

    frame_info.vid = rx_info->tag.vid;
    frame_info.qos = rx_info->cos;
    frame_info.pcp = rx_info->tag.pcp;
    frame_info.dei = rx_info->tag.dei;
    frame_info.isdx = rx_info->isdx;
    frame_info.tagged = (rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED) ? TRUE : FALSE;

    vtss_mep_supp_rx_frame(port_conv, &frame_info, (u8 *)frame, rx_info->length, &rx_time, rx_count);

    return(TRUE);
}

typedef struct
{
    BOOL                     enable;
    BOOL                     protection;
    vtss_mep_mgmt_period_t   period;
    vtss_mep_mgmt_domain_t   client_domain;
    BOOL                     client_dei;            /* DEI for the AIS              */
    u8                       client_prio; 
    u8                       client_level;
    u32                      client_flow_count;
    u32                      client_flows[VTSS_MEP_CLIENT_FLOWS_MAX];
} mep_old_mgmt_ais_conf_t;

typedef struct
{
    BOOL                     enable;
    vtss_mep_mgmt_period_t   period;
    vtss_mep_mgmt_domain_t   client_domain;
    BOOL                     client_dei;            /* DEI for the LCK              */
    u8                       client_prio; 
    u8                       client_level;
    u32                      client_flow_count;
    u32                      client_flows[VTSS_MEP_CLIENT_FLOWS_MAX];
} mep_old_mgmt_lck_conf_t;

#define MEP_OLD_MEG_CODE_LENGTH       9  /* Both Domain Name/ICC and MEG-ID can be max 8 charecters plus a NULL termination */
typedef struct
{
    BOOL                        enable;                                           /* Enable/disable                                  */
    vtss_mep_mgmt_mode_t        mode;                                             /* MEP or MIP mode                                 */
    vtss_mep_mgmt_direction_t   direction;                                        /* Ingress or Egress direction                     */
    vtss_mep_mgmt_domain_t      domain;                                           /* Domain                                          */
    u32                         flow;                                             /* Flow instance                                   */
    u32                         port;                                             /* Residence port - same as flow if domain is Port */
    u32                         level;                                            /* MEG level                                       */
    u16                         vid;                                              /* VID used for tagged OAM                         */

    vtss_mep_mgmt_format_t      format;                                           /* MEG-ID format.                                  */
    char                        name[MEP_OLD_MEG_CODE_LENGTH];                   /* Domain Name or ITU Carrier Code (ICC). (string) */
    char                        meg[MEP_OLD_MEG_CODE_LENGTH];                    /* Unique MEG ID Code.                    (string) */
    u32                         mep;                                              /* MEP id of this instance                         */
    u32                         peer_count;                                       /* Number of peer MEP’s  (VTSS_MEP_PEER_MAX)       */
    u16                         peer_mep[VTSS_MEP_PEER_MAX];                      /* MEP id of peer MEP’s                            */
    u8                          peer_mac[VTSS_MEP_PEER_MAX][VTSS_MEP_MAC_LENGTH]; /* Peer unicast MAC                                */

    u32                         evc_pag;                                          /* EVC generated policy PAG value. On Jaguar this is used as IS2 key. For Up-MEP this is used when creating MCE (IS1) entries */
    u32                         evc_qos;                                          /* EVC QoS value. Only used on Caracel for getting queue frame counters. For Up-MEP this is used when creating MCE (IS1) entries */
} mep_old_mgmt_conf_t;

typedef struct
{
    ulong                       version;                          /* Block version */
    BOOL                        up_mep_enabled;
    mep_old_mgmt_conf_t         conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_cc_conf_t     cc_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_lm_conf_t     lm_conf[MEP_INSTANCE_MAX];
    vtss_mep_mgmt_aps_conf_t    aps_conf[MEP_INSTANCE_MAX];
    mep_old_mgmt_ais_conf_t     ais_conf[MEP_INSTANCE_MAX];
    mep_old_mgmt_lck_conf_t     lck_conf[MEP_INSTANCE_MAX];
} mep_old_conf_t;

static mep_old_conf_t       old_blk;
static mep_conf_t           new_blk;

#if defined(VTSS_ARCH_SERVAL)
void mep_voe_interrupt(void)
{
    // mep_voe_interrupt() is just a stepping-stone function
    vtss_mep_supp_voe_interrupt();
}
#endif

vtss_rc mep_init(vtss_init_data_t *data)
{
    u32                  size, base_rc, i, j, new_idx;
    vtss_isid_t          isid = data->isid;
    mep_conf_t           *blk;
    packet_rx_filter_t   filter;
    void                 *filter_id;
    vtss_rc              rc = VTSS_RC_OK;
    vtss_mep_mgmt_def_conf_t  def_conf;

    if (data->cmd == INIT_CMD_INIT)
    {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd)
    {
        case INIT_CMD_INIT:
            T_D("INIT");

            /* initialize critd */
            critd_init(&crit_p, "MEP Platform Crit", VTSS_MODULE_ID_MEP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            critd_init(&crit, "MEP Crit", VTSS_MODULE_ID_MEP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            critd_init(&crit_supp, "MEP supp Crit", VTSS_MODULE_ID_MEP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

            cyg_semaphore_init(&run_wait_sem, 0);
            cyg_flag_init(&timer_wait_flag);
#ifdef VTSS_SW_OPTION_VCLI
            mep_cli_req_init();
#endif
            cyg_thread_create(THREAD_HIGHEST_PRIO,
                              mep_timer_thread,
                              0,
                              "MEP Timer",
                              timer_thread_stack,
                              sizeof(timer_thread_stack),
                              &timer_thread_handle,
                              &timer_thread_block);

            cyg_thread_create(THREAD_HIGHEST_PRIO,
                              mep_run_thread,
                              0,
                              "MEP State Machine",
                              run_thread_stack,
                              sizeof(run_thread_stack),
                              &run_thread_handle,
                              &run_thread_block);

            memset(los_state, 0xFF, sizeof(los_state));
            for (i=0; i<MEP_INSTANCE_MAX; ++i)
            {
                memset(&instance_data[i], 0, sizeof(instance_data_t));
                instance_data[i].aps_inst = APS_INST_INV;
                instance_data[i].mip_lbm_ace_id = ACE_ID_NONE;
                instance_data[i].ccm_ace_id[0] = ACE_ID_NONE;
                instance_data[i].ccm_ace_id[1] = ACE_ID_NONE;
                instance_data[i].tst_ace_id[0] = ACE_ID_NONE;
                instance_data[i].tst_ace_id[1] = ACE_ID_NONE;
            }
            base_rc = vtss_mep_init(10);
            base_rc += vtss_mep_supp_init(10, voe_up_mep_loop_port);
            if (base_rc)        T_D("Error during init");

            for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)
                in_port_conv[i] = out_port_conv[i] = i;
            for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)
                conv_los_state[i] = TRUE;

#if defined(VTSS_ARCH_SERVAL)
            memset(qos_port_conf, 0, sizeof(qos_port_conf));
            memset(vlan_port_conf, 0, sizeof(vlan_port_conf));

            service_mep_ace_id = ACE_ID_NONE;
            service_mip_ccm_ace_id = ACE_ID_NONE;
            service_mip_uni_ltm_ace_id = ACE_ID_NONE;
            service_mip_nni_ltm_ace_id = ACE_ID_NONE;
#endif
            CRIT_EXIT(crit_p);
            CRIT_EXIT(crit);
            CRIT_EXIT(crit_supp);

#ifdef VTSS_SW_OPTION_ICFG
            // Initialize ICLI "show running" configuration
            VTSS_RC(mep_icfg_init());
#endif
            break;
        case INIT_CMD_START:
            T_D("START");
            break;
        case INIT_CMD_CONF_DEF:
            T_D("CONF_DEF");
            if (isid == VTSS_ISID_LOCAL)
            {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                if ((blk = (mep_conf_t *)conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) != NULL) {
                    set_conf_to_default(blk);
                    apply_configuration(blk);
                    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
                }
#else
                restore_to_default();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            }
            break;
        case INIT_CMD_MASTER_UP:
            T_D("MASTER_UP");
            if (misc_conf_read_use()) {
                T_W("Upgrade is enabled");
                if ((blk = (mep_conf_t *)conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, &size)) == NULL || ((size != sizeof(*blk)) && (size != sizeof(old_blk))))
                {
                    T_W("Open conf failed  blk %p  size %u  old_blk %u", blk, size, sizeof(old_blk));
                    blk = (mep_conf_t *)conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE, sizeof(*blk));
                    if (blk != NULL) {
                        blk->version = 0;
                        set_conf_to_default(blk);
                    } else {
                        T_W("conf_sec_create failed");
                        break;
                    }
                }

                if ((blk != NULL) && (size == sizeof(old_blk))) {
                    T_W("Upgrading database");
                    vtss_mep_mgmt_def_conf_get(&def_conf);

                    memcpy(&old_blk, blk, sizeof(old_blk));     /* Save old blk */
                    memset(&new_blk, 0, sizeof(new_blk));
                    new_blk.version = 1;
                    new_blk.up_mep_enabled = FALSE;
                    memcpy(new_blk.cc_conf, old_blk.cc_conf, sizeof(new_blk.cc_conf));
                    memcpy(new_blk.lm_conf, old_blk.lm_conf, sizeof(new_blk.lm_conf));
                    memcpy(new_blk.aps_conf, old_blk.aps_conf, sizeof(new_blk.aps_conf));
                    for (i=0; i<MEP_INSTANCE_MAX; ++i) { /* New configuration is set to default */
                        new_blk.conf[i].enable = old_blk.conf[i].enable;
                        new_blk.conf[i].mode = old_blk.conf[i].mode;
                        new_blk.conf[i].direction = old_blk.conf[i].direction;
                        new_blk.conf[i].domain = old_blk.conf[i].domain;
                        if (old_blk.conf[i].domain == 2)
                            new_blk.conf[i].domain = VTSS_MEP_MGMT_EVC;
                        new_blk.conf[i].flow = old_blk.conf[i].flow;
                        new_blk.conf[i].port = old_blk.conf[i].port;
                        new_blk.conf[i].level = old_blk.conf[i].level;
                        new_blk.conf[i].vid = old_blk.conf[i].vid;
                        new_blk.conf[i].format = old_blk.conf[i].format;
                        if (old_blk.conf[i].format == VTSS_MEP_MGMT_ITU_ICC) {
                            memset(new_blk.conf[i].name, 0, sizeof(new_blk.conf[i].name));
                            for (j=0; j<MEP_OLD_MEG_CODE_LENGTH && (old_blk.conf[i].name[j] != 0); ++j)
                                new_blk.conf[i].meg[j] = old_blk.conf[i].name[j];
                            for (new_idx=j, j=0; j<MEP_OLD_MEG_CODE_LENGTH && (old_blk.conf[i].meg[j] != 0); ++j, ++new_idx)
                                new_blk.conf[i].meg[new_idx] = old_blk.conf[i].meg[j];
                        } else {
                            for (j=0; j<MEP_OLD_MEG_CODE_LENGTH && (old_blk.conf[i].name[j] != 0); ++j)
                                new_blk.conf[i].name[j] = old_blk.conf[i].name[j];
                            for (j=0; j<MEP_OLD_MEG_CODE_LENGTH && (old_blk.conf[i].meg[j] != 0); ++j)
                                new_blk.conf[i].meg[j] = old_blk.conf[i].meg[j];
                        }
                        new_blk.conf[i].mep = old_blk.conf[i].mep;
                        new_blk.conf[i].peer_count = old_blk.conf[i].peer_count;
                        memcpy(new_blk.conf[i].peer_mep, old_blk.conf[i].peer_mep, sizeof(new_blk.conf[i].peer_mep));
                        for (j=0; j<VTSS_MEP_PEER_MAX; ++j)
                            memcpy(new_blk.conf[i].peer_mac[j], old_blk.conf[i].peer_mac[j], sizeof(new_blk.conf[i].peer_mac[j]));
                        new_blk.conf[i].evc_pag = old_blk.conf[i].evc_pag;
                        new_blk.conf[i].evc_qos = old_blk.conf[i].evc_qos;

                        new_blk.ais_conf[i].enable = old_blk.ais_conf[i].enable;
                        new_blk.ais_conf[i].protection = old_blk.ais_conf[i].protection;
                        new_blk.ais_conf[i].period = old_blk.ais_conf[i].period;

                        new_blk.lck_conf[i].enable = old_blk.lck_conf[i].enable;
                        new_blk.lck_conf[i].period = old_blk.lck_conf[i].period;

                        new_blk.client_conf[i].domain = old_blk.ais_conf[i].client_domain;
                        if (old_blk.ais_conf[i].client_domain == 2)
                            new_blk.client_conf[i].domain = VTSS_MEP_MGMT_EVC;
                        new_blk.client_conf[i].flow_count = old_blk.ais_conf[i].client_flow_count;
                        memcpy(new_blk.client_conf[i].flows, old_blk.ais_conf[i].client_flows, sizeof(new_blk.client_conf[i].flows));
                        memset(new_blk.client_conf[i].ais_prio, old_blk.ais_conf[i].client_prio, sizeof(new_blk.client_conf[i].ais_prio));
                        memset(new_blk.client_conf[i].lck_prio, old_blk.lck_conf[i].client_prio, sizeof(new_blk.client_conf[i].lck_prio));
                        memset(new_blk.client_conf[i].level, old_blk.ais_conf[i].client_level, sizeof(new_blk.client_conf[i].level));

                        new_blk.dm_conf[i] = def_conf.dm_conf;
                        new_blk.pm_conf[i] = def_conf.pm_conf;
                    }
                }

                apply_configuration(&new_blk);
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MEP_CONF_TABLE);
            } else {
                restore_to_default();
            }
            break;
        case INIT_CMD_MASTER_DOWN:
            T_D("MASTER_DOWN");
            break;
        case INIT_CMD_SWITCH_ADD:
            T_D("SWITCH_ADD");
            // We resume the thread here because the async config code indirectly expects a valid ISID.
            cyg_thread_resume(run_thread_handle);
            /* MEP need to hook on port state change */
            rc = port_change_register(VTSS_MODULE_ID_MEP, port_change_callback);
            if (rc != VTSS_RC_OK)        T_D("Error during port change register");

            if(vtss_board_features() & (VTSS_BOARD_FEATURE_AMS | VTSS_BOARD_FEATURE_LOS ))
            {
                /* In case of protection we need to hook on fast link fail and LOS detection */
                 rc = vtss_interrupt_source_hook_set(link_interrupt_function,
                                                         INTERRUPT_SOURCE_FLNK,
                                                         INTERRUPT_PRIORITY_PROTECT);
                 rc = vtss_interrupt_source_hook_set(link_interrupt_function,
                                                         INTERRUPT_SOURCE_LOS,
                                                         INTERRUPT_PRIORITY_PROTECT);
                 if (rc != VTSS_RC_OK)        T_D("Error during interrupt source hook");
            }

            memset(evc_vlan_vid, 0xFF, sizeof(evc_vlan_vid));

#if defined(VTSS_SW_OPTION_EVC)
            evc_mgmt_conf_t  conf;
            vtss_evc_id_t    evc_id;
            /* EVC change callback registration */
            rc = evc_change_register(evc_change_callback);
            if (rc != VTSS_RC_OK)        vtss_mep_trace("vtss_mep_init - EVC register failed", 0, 0, 0, 0);

            evc_id = EVC_ID_FIRST;
            do
            {
                if ((rc = evc_mgmt_get(&evc_id, &conf, TRUE)) == VTSS_RC_OK)
                {
#if defined(VTSS_FEATURE_MPLS)
                    if (VTSS_MPLS_IDX_IS_UNDEF(conf.conf.network.mpls_tp.pw_egress_xc) && VTSS_MPLS_IDX_IS_UNDEF(conf.conf.network.mpls_tp.pw_ingress_xc))
                    {
                        evc_vlan_vid[conf.conf.network.pb.ivid] = evc_id;
                    }
#else
                    evc_vlan_vid[conf.conf.network.pb.ivid] = evc_id;
#endif
                }
            } while (rc == VTSS_RC_OK);
#endif

            /* Register for VLAN port configuration changes */
            vlan_port_conf_change_register(VTSS_MODULE_ID_MEP, vlan_port_change_callback, FALSE /* Get called back once the change has occurred */);

#if defined(VTSS_ARCH_SERVAL)
            vlan_mgmt_entry_t    vlan_conf;
            vtss_vid_t           vid;

            vid = VTSS_VID_NULL;
            while (vlan_mgmt_vlan_get(VTSS_ISID_START, vid, &vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) { /* Register all static configured VID */
                vid = vlan_conf.vid; /* Select next entry */
                if (evc_vlan_vid[vid] >= VTSS_EVCS) {    /* This VID is not registered as a EVC VID */
                    evc_vlan_vid[vid] = VTSS_EVCS;
                }
            }

            vlan_membership_change_register(VTSS_MODULE_ID_MEP, vlan_membership_change_callback);
            rc = qos_port_conf_change_register(FALSE, VTSS_MODULE_ID_MEP, qos_port_change_callback);
            if (rc != VTSS_RC_OK)        vtss_mep_trace("vtss_mep_init - QOS register failed", 0, 0, 0, 0);
#endif

            memset(&filter, 0, sizeof(filter));
            filter.modid = VTSS_MODULE_ID_MEP;
            filter.match = PACKET_RX_FILTER_MATCH_ETYPE;
            filter.cb    = rx_frame;
            filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
            filter.etype = 0x8902; // OAM ethertype
            if (packet_rx_filter_register(&filter, &filter_id) != VTSS_RC_OK)   return(FALSE);
            filter.etype = 0x8100; // C-TAG
            if (packet_rx_filter_register(&filter, &filter_id) != VTSS_RC_OK)   return(FALSE);
            filter.etype = 0x88A8; // S-TAG
            if (packet_rx_filter_register(&filter, &filter_id) != VTSS_RC_OK)   return(FALSE);

            break;
        case INIT_CMD_SWITCH_DEL:
            T_D("SWITCH_DEL");
            break;
        case INIT_CMD_SUSPEND_RESUME:
            T_D("SUSPEND_RESUME");
            break;
        default:
            break;
    }

    T_D("exit");
    return VTSS_RC_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
