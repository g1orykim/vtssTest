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

 $Id$
 $Revision$ 

*/

#ifndef _PHY_H_
#define _PHY_H_

/* ================================================================= *
 * Trace definitions
 * ================================================================= */
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PHY
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_ICLI         2
#define TRACE_GRP_CNT          3

#include <vtss_trace_api.h>
#include "critd_api.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"
#endif  /* VTSS_FEATURE_PHY_TIMESTAMP */

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "phy",
    .descr     = "PHY"
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
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
//static critd_t   phy_crit; 
#define PHY_CRIT_ENTER() critd_enter(&phy.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PHY_CRIT_EXIT()  critd_exit( &phy.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#endif /* VTSS_TRACE_ENABLED */

/* ================================================================= *
 *  Aggr local definitions
 * ================================================================= */
#define PHY_CONF_VERSION     0

/* Flash/Mem structs */
typedef struct {      
    phy_inst_start_t inst;
    ulong            version; /* Block version     */
} phy_conf_t;

typedef struct {      
    BOOL                          active;
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t           mode;
    BOOL                          synce_clkout;
    BOOL                          xfp_clkout;
    vtss_phy_10g_loopback_t       loopback;
    vtss_phy_10g_power_t          power;
    vtss_phy_10g_failover_mode_t  failover;    
    vtss_phy_10g_event_t          ev_mask;
    vtss_gpio_10g_gpio_mode_t     gpio_mode[VTSS_10G_PHY_GPIO_MAX];
#if defined(VTSS_FEATURE_WIS)
    vtss_ewis_conf_t              ewis_mode;
#endif
#endif
} inst_store_10g;


typedef struct {
    vtss_phy_clock_conf_t  conf;
    vtss_port_no_t         source;
} clock_conf_t;

typedef struct {      
    BOOL                     active;
    vtss_phy_conf_t          conf;
    vtss_phy_reset_conf_t    reset;
    vtss_phy_conf_1g_t       conf_1g;
    vtss_phy_power_conf_t    power;
    clock_conf_t             recov_clk[VTSS_PHY_RECOV_CLK_NUM];
    vtss_phy_clock_conf_t    clk_conf;
    vtss_phy_loopback_t	     loopback;
#if defined(VTSS_FEATURE_EEE)
    vtss_phy_eee_conf_t      eee_conf;
#endif
#if defined(VTSS_FEATURE_LED_POW_REDUC)
    vtss_phy_led_intensity   led_intensity;
    vtss_phy_enhanced_led_control_t enhanced_led_control;
#endif
    vtss_phy_event_t         ev_mask;
} inst_store_1g;

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

typedef struct {
    BOOL                            eng_used; /* allocated the engine to application */
    vtss_phy_ts_encap_t             encap_type; /* engine encapsulation */
    vtss_phy_ts_engine_flow_match_t flow_match_mode; /* strict/non-strict flow match */
    u8                              flow_st_index; /* start index of flow */
    u8                              flow_end_index; /* end index of flow */
    vtss_phy_ts_engine_flow_conf_t  flow_conf; /* engine flow config */
    vtss_phy_ts_engine_action_t     action_conf; /* engine action */
    u8                              action_flow_map[6]; /* action map to flow */
} vtss_phy_ts_eng_conf_t;

typedef struct {
    BOOL                            eng_used; /* allocated the engine to application */
    vtss_phy_ts_encap_t             encap_type; /* engine encapsulation */
    vtss_phy_ts_engine_flow_match_t flow_match_mode; /* strict/non-strict flow match */
    u8                              flow_st_index; /* start index of flow */
    u8                              flow_end_index; /* end index of flow */
    vtss_phy_ts_engine_flow_conf_t  flow_conf; /* engine flow config */
    vtss_phy_ts_engine_action_t     action_conf; /* engine action */
} vtss_phy_ts_eng_conf_stored_t;

typedef struct {
    BOOL                             port_ts_init_done; /* PHY TS init done */
    BOOL                             port_ena;
    vtss_phy_ts_clockfreq_t          clk_freq;  /* reference clock frequency */
    vtss_phy_ts_clock_src_t          clk_src;   /* reference clock source */
    vtss_phy_ts_rxtimestamp_pos_t    rx_ts_pos; /* Rx timestamp position */
    vtss_phy_ts_rxtimestamp_len_t    rx_ts_len; /* Rx timestamp length */
    vtss_phy_ts_fifo_mode_t          tx_fifo_mode; /* Tx TSFIFO access mode */
    vtss_phy_ts_fifo_timestamp_len_t tx_ts_len; /* timestamp size in Tx TSFIFO */
    vtss_phy_ts_8487_xaui_sel_t      xaui_sel_8487; /* 8487 XAUI Lane selection */
    vtss_phy_ts_fifo_sig_mask_t      sig_mask;  /* FIFO signature */
    u32                              fifo_age;  /* SW TSFIFO age in milli-sec */
    vtss_timeinterval_t              ingress_latency;
    vtss_timeinterval_t              egress_latency;
    vtss_timeinterval_t              path_delay;
    vtss_timeinterval_t              delay_asym;
    vtss_phy_ts_scaled_ppb_t         rate_adj;  /* clock rate adjustment */
    vtss_phy_ts_eng_conf_stored_t    ingress_eng_conf[4]; /*port ingress engine configuration including encapsulation, comparator configuration and action  */
    vtss_phy_ts_eng_conf_stored_t    egress_eng_conf[4]; /*port egress engine configuration including encapsulation, comparator configuration and action  */
    vtss_phy_ts_event_t              event_mask; /* interrupt mask config */
#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    BOOL                             new_spi_mode;
#endif
} inst_store_ts_t;
#endif  /* VTSS_FEATURE_PHY_TIMESTAMP */


typedef struct {  
    phy_inst_start_t             start_inst;
    vtss_inst_t                  current_inst;       
    vtss_init_conf_t             init_conf_default;
    phy_conf_t                   conf;
    critd_t                      crit; 
    inst_store_1g                store_1g[VTSS_PORTS];
    inst_store_10g               store_10g[VTSS_PORTS];
    vtss_port_no_t               failover_port;
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_failover_mode_t failover;
#endif
    
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    vtss_phy_ts_fifo_read        ts_fifo_cb;
    void                         *cb_cntxt;
    inst_store_ts_t              store_ts[VTSS_PORTS];
#endif  /* VTSS_FEATURE_PHY_TIMESTAMP */
} phy_global_t;

#endif /* _PHY_H_ */
