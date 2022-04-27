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
#include "main.h"
#include "port_api.h"
#include "phy_api.h"
#include "phy.h"
#include "conf_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "phy_icfg.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

/* Global configuration for the module */
static phy_global_t phy;

/*lint -sem(mmd_read,   thread_protected) ... We're protected  */
/*lint -sem(mmd_write,  thread_protected) ... We're protected */
/*lint -sem(miim_read,  thread_protected) ... We're protected */
/*lint -sem(miim_write, thread_protected) ... We're protected */
/*lint -sem(mmd_read_inc, thread_protected) ... We're protected */
/*lint -sem(phy_mgmt_failover_set, thread_protected) ... We're protected */
/*lint -sem(phy_mgmt_failover_get, thread_protected) ... We're protected */
/*lint -sem(phy_mgmt_inst_activate_default, thread_protected) ... We're protected */
/*lint -sem(phy_channel_failover, thread_protected) ... We're protected */


/* All PHY MIIM reads/writes comes through the following functions: */
static vtss_rc mmd_read(const vtss_inst_t     inst,
                        const vtss_port_no_t  port_no,
                        const u8              mmd,
                        const u16             addr,
                        u16                   *const value)
{
    /* Must use the MMD functions from the default API instance (and update the API pointer to the default state) */
    if (vtss_init_conf_get(NULL, &phy.init_conf_default) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    if (phy.init_conf_default.mmd_read(NULL, port_no,  mmd,  addr, value) != VTSS_RC_OK) {
        T_I("MMD read Failed");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc mmd_write(const vtss_inst_t     inst,
                         const vtss_port_no_t  port_no,
                         const u8              mmd,
                         const u16             addr,
                         const u16             value)

{
    /* Must use the MMD functions from the default API instance (and update the API pointer to the default state) */
    if (vtss_init_conf_get(NULL, &phy.init_conf_default) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    if (phy.init_conf_default.mmd_write(NULL, port_no, mmd, addr, value) != VTSS_RC_OK) {
        T_I("MMD write Failed");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc mmd_read_inc(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            const u8              mmd,
                            const u16             addr,
                            u16                   *buf,
                            u8                    count)
{
   /* Must use the MMD functions from the default API instance (and update the API pointer to the default state) */
    if (vtss_init_conf_get(NULL, &phy.init_conf_default) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    if (phy.init_conf_default.mmd_read_inc(NULL, port_no, mmd, addr, buf, count) != VTSS_RC_OK) {
        T_I("MMD read inc Failed");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}


static vtss_rc miim_read(const vtss_inst_t     inst,
                         const vtss_port_no_t  port_no,
                         const u8              addr,
                         u16                   *const value)
{
    /* Must use the MMD functions from the default API instance (and update the API pointer to the default state) */
    if (vtss_init_conf_get(NULL, &phy.init_conf_default) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    if (phy.init_conf_default.miim_read(NULL, port_no, addr, value) != VTSS_RC_OK) {
        T_E("MIIM read Failed");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}


static vtss_rc miim_write(const vtss_inst_t     inst,
                          const vtss_port_no_t  port_no,
                          const u8              addr,
                          const u16             value)

{
    /* Must use the MMD functions from the default API instance (and update the API pointer to the default state) */
    if (vtss_init_conf_get(NULL, &phy.init_conf_default) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    if (phy.init_conf_default.miim_write(NULL, port_no, addr, value) != VTSS_RC_OK) {
        T_E("MIIM write Failed");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}


static vtss_rc phy_inst_create(void)
{
    vtss_inst_create_t       create;
    vtss_init_conf_t         init_conf;
    vtss_port_no_t           port_no;
    port_cap_t               cap;
    vtss_rc                  rc;

    if (phy.start_inst == PHY_INST_NONE) {
        phy.current_inst = NULL;
        return VTSS_RC_OK; /* The Phys are included in the Default API instance */
    }

    /* Find a Phy port for storing Warm start info  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (port_cap_get(port_no, &cap) != VTSS_OK) {
            T_E("Could not get port CAP info p:%u",port_no);
            continue;
        }
        if (phy.start_inst == PHY_INST_1G_PHY) {
            if (cap & PORT_CAP_1G_PHY) {
                break;
            }
        } else {
            if (cap & PORT_CAP_VTSS_10G_PHY) {
                break;
            }
        }
    }
   
    if (port_no == VTSS_PORT_NO_END) {
        T_E("Could not find a Phy to store Warm start info");
        return VTSS_RC_ERROR;
    }

    /* Create a 1G or 10G API instance */
    if ((rc = vtss_inst_get((phy.start_inst == PHY_INST_10G_PHY) ? VTSS_TARGET_10G_PHY : VTSS_TARGET_CU_PHY, &create)) != VTSS_RC_OK) {
        T_E("vtss_inst_get() Failed");
        return rc;
    }
    if ((rc = vtss_inst_create(&create, &phy.current_inst)) != VTSS_RC_OK) {
        T_E("vtss_inst_create() Failed");
        return rc;
    }
    /* Get the defaults */
    if ((rc = vtss_init_conf_get(phy.current_inst, &init_conf)) != VTSS_RC_OK) {
        T_E("vtss_init_conf_get() Failed");
        return rc;
    }

    /* Hook up our local call out functions */
    init_conf.mmd_read = mmd_read;
    init_conf.mmd_write = mmd_write;
    init_conf.mmd_read_inc = mmd_read_inc;
    init_conf.miim_read = miim_read;
    init_conf.miim_write = miim_write;

    /* Apply Warm start info */
    init_conf.warm_start_enable = 1;
    init_conf.restart_info_port = port_no;
    init_conf.restart_info_src = (phy.start_inst == PHY_INST_10G_PHY) ?  VTSS_RESTART_INFO_SRC_10G_PHY : VTSS_RESTART_INFO_SRC_CU_PHY;
    /* Save the phy instance */
    if ((rc = vtss_init_conf_set(phy.current_inst, &init_conf)) != VTSS_RC_OK) {
        T_E("vtss_init_conf_set() Failed");
        return rc;
    }
    /* Unlock the VTSS_API  */
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static BOOL phy_ts_eng_used_info[VTSS_PORT_NO_END - VTSS_PORT_NO_START][2][4];
static vtss_rc phy_ts_conf_get(const vtss_inst_t inst, const vtss_port_no_t port_no)
{
    inst_store_ts_t             *port_ts_conf = &phy.store_ts[port_no];
    vtss_phy_ts_init_conf_t     ts_init_conf;
    vtss_phy_ts_eng_init_conf_t ts_eng_init_conf;
    vtss_phy_ts_engine_t        eng_id;

    memset(port_ts_conf, 0, sizeof(inst_store_ts_t));
    memset(phy_ts_eng_used_info, 0, sizeof(phy_ts_eng_used_info));
    (void)vtss_phy_ts_init_conf_get(inst, port_no, &port_ts_conf->port_ts_init_done, &ts_init_conf);
    if (port_ts_conf->port_ts_init_done == FALSE) {
    /* TS block is not initialized */
        return VTSS_RC_OK;
    }
    port_ts_conf->clk_freq = ts_init_conf.clk_freq;
    port_ts_conf->clk_src = ts_init_conf.clk_src;
    port_ts_conf->rx_ts_pos = ts_init_conf.rx_ts_pos;
    port_ts_conf->tx_fifo_mode = ts_init_conf.tx_fifo_mode;
    port_ts_conf->tx_ts_len = ts_init_conf.tx_ts_len;
#if defined(VTSS_FEATURE_10G)
    port_ts_conf->xaui_sel_8487 = ts_init_conf.xaui_sel_8487;
#endif
#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    if (ts_init_conf.tx_fifo_mode == VTSS_PHY_TS_FIFO_MODE_SPI) {
        (void)(vtss_phy_ts_new_spi_mode_get(inst, port_no, &port_ts_conf->new_spi_mode));
    }
#endif

    VTSS_RC(vtss_phy_ts_mode_get(inst, port_no, &port_ts_conf->port_ena));

    VTSS_RC(vtss_phy_ts_fifo_sig_get(inst, port_no, &port_ts_conf->sig_mask));
    VTSS_RC(vtss_phy_ts_ingress_latency_get(inst, port_no, &port_ts_conf->ingress_latency));
    VTSS_RC(vtss_phy_ts_egress_latency_get(inst, port_no, &port_ts_conf->egress_latency));
    VTSS_RC(vtss_phy_ts_path_delay_get(inst, port_no, &port_ts_conf->path_delay));
    VTSS_RC(vtss_phy_ts_clock_rateadj_get(inst, port_no, &port_ts_conf->rate_adj));
    eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    while (eng_id != VTSS_PHY_TS_ENGINE_ID_INVALID) {
        (void)(vtss_phy_ts_ingress_engine_init_conf_get(inst, port_no, eng_id, &ts_eng_init_conf));
        port_ts_conf->ingress_eng_conf[eng_id].eng_used = ts_eng_init_conf.eng_used;
        if (port_ts_conf->ingress_eng_conf[eng_id].eng_used) {
            port_ts_conf->ingress_eng_conf[eng_id].encap_type = ts_eng_init_conf.encap_type;
            port_ts_conf->ingress_eng_conf[eng_id].flow_match_mode = ts_eng_init_conf.flow_match_mode;
            port_ts_conf->ingress_eng_conf[eng_id].flow_st_index = ts_eng_init_conf.flow_st_index;
            port_ts_conf->ingress_eng_conf[eng_id].flow_end_index = ts_eng_init_conf.flow_end_index;
            VTSS_RC(vtss_phy_ts_ingress_engine_conf_get(inst, port_no, eng_id, &port_ts_conf->ingress_eng_conf[eng_id].flow_conf));
            VTSS_RC(vtss_phy_ts_ingress_engine_action_get(inst, port_no, eng_id, &port_ts_conf->ingress_eng_conf[eng_id].action_conf));
        }
        eng_id++;
    }
    eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    while (eng_id != VTSS_PHY_TS_ENGINE_ID_INVALID) {
        (void)(vtss_phy_ts_egress_engine_init_conf_get(inst, port_no, eng_id, &ts_eng_init_conf));
        port_ts_conf->egress_eng_conf[eng_id].eng_used = ts_eng_init_conf.eng_used;
        if (port_ts_conf->egress_eng_conf[eng_id].eng_used) {
            port_ts_conf->egress_eng_conf[eng_id].encap_type = ts_eng_init_conf.encap_type;
            port_ts_conf->egress_eng_conf[eng_id].flow_match_mode = ts_eng_init_conf.flow_match_mode;
            port_ts_conf->egress_eng_conf[eng_id].flow_st_index = ts_eng_init_conf.flow_st_index;
            port_ts_conf->egress_eng_conf[eng_id].flow_end_index = ts_eng_init_conf.flow_end_index;
            VTSS_RC(vtss_phy_ts_egress_engine_conf_get(inst, port_no, eng_id, &port_ts_conf->egress_eng_conf[eng_id].flow_conf));
            VTSS_RC(vtss_phy_ts_egress_engine_action_get(inst, port_no, eng_id, &port_ts_conf->egress_eng_conf[eng_id].action_conf));
        }
        eng_id++;
    }
    (void)(vtss_phy_ts_event_enable_get(inst, port_no, &port_ts_conf->event_mask));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_base_port_get(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           vtss_port_no_t     *const base_port_no)
{
#if defined(VTSS_FEATURE_10G)
    vtss_phy_10g_id_t phy_id_10g;
#endif
    vtss_phy_type_t   phy_id_1g;

    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
#if defined(VTSS_FEATURE_10G)
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get(inst, port_no, &phy_id_10g) == VTSS_RC_OK) {
        /* Get base port_no for 10G */
        *base_port_no = phy_id_10g.phy_api_base_no;
    } else if (vtss_phy_id_get(inst, port_no, &phy_id_1g) == VTSS_RC_OK) {
#else
    if (vtss_phy_id_get(inst, port_no, &phy_id_1g) == VTSS_RC_OK) {
#endif
        *base_port_no = phy_id_1g.phy_api_base_no;
    } else {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc phy_ts_conf_set(const vtss_inst_t inst, const vtss_port_no_t port_no)
{
    inst_store_ts_t             *stored_ts_conf = &phy.store_ts[port_no];
    vtss_phy_ts_init_conf_t     ts_init_conf;
    vtss_phy_ts_engine_t        eng_id;
    vtss_port_no_t              base_port = 0;

    VTSS_RC(vtss_phy_ts_base_port_get(inst, port_no, &base_port));
    memset(&ts_init_conf, 0, sizeof(vtss_phy_ts_init_conf_t));
    if (stored_ts_conf->port_ts_init_done == FALSE) {
    /* TS block was not initialized */
        return VTSS_RC_OK;
    }
    /*Initialize the TS block*/
    ts_init_conf.clk_freq = stored_ts_conf->clk_freq;
    ts_init_conf.clk_src = stored_ts_conf->clk_src;
    ts_init_conf.rx_ts_pos = stored_ts_conf->rx_ts_pos;
    ts_init_conf.rx_ts_len = stored_ts_conf->rx_ts_len;
    ts_init_conf.tx_fifo_mode = stored_ts_conf->tx_fifo_mode;
    ts_init_conf.tx_ts_len = stored_ts_conf->tx_ts_len;
#if defined(VTSS_FEATURE_10G)
    ts_init_conf.xaui_sel_8487 = stored_ts_conf->xaui_sel_8487;
#endif
#if 0 /* TODO: is not stored in state */
    ts_init_conf.remote_phy = stored_ts_conf->remote_phy;
#endif
    VTSS_RC(vtss_phy_ts_init(inst, port_no, &ts_init_conf));
    VTSS_RC(vtss_phy_ts_mode_set(inst, port_no, stored_ts_conf->port_ena));
    VTSS_RC(vtss_phy_ts_ingress_latency_set(inst, port_no, &stored_ts_conf->ingress_latency));
    VTSS_RC(vtss_phy_ts_egress_latency_set(inst, port_no, &stored_ts_conf->egress_latency));
    VTSS_RC(vtss_phy_ts_path_delay_set(inst, port_no, &stored_ts_conf->path_delay));
    VTSS_RC(vtss_phy_ts_delay_asymmetry_set(inst, port_no, &stored_ts_conf->delay_asym));
    /*Configuring the Ingress engines*/
    eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    while (eng_id != VTSS_PHY_TS_ENGINE_ID_INVALID) {
        if (stored_ts_conf->ingress_eng_conf[eng_id].eng_used) {
            /*Ingress Engine Initialization*/
            if (phy_ts_eng_used_info[base_port][0][eng_id] == FALSE) { 
                VTSS_RC(vtss_phy_ts_ingress_engine_init(inst,
                                port_no, eng_id,
                                stored_ts_conf->ingress_eng_conf[eng_id].encap_type,
                                stored_ts_conf->ingress_eng_conf[eng_id].flow_st_index,
                                stored_ts_conf->ingress_eng_conf[eng_id].flow_end_index,
                                stored_ts_conf->ingress_eng_conf[eng_id].flow_match_mode));
                phy_ts_eng_used_info[base_port][0][eng_id] = TRUE; 
            }
            /*Programming the Comparators/flows of the engine*/
            VTSS_RC(vtss_phy_ts_ingress_engine_conf_set(inst, port_no, eng_id, &stored_ts_conf->ingress_eng_conf[eng_id].flow_conf));
            /*Programming the Engine Action*/
            VTSS_RC(vtss_phy_ts_ingress_engine_action_set(inst, port_no, eng_id, &stored_ts_conf->ingress_eng_conf[eng_id].action_conf));
        }
        eng_id++;
    }
    /*Configuring the egress engines*/
    eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    while (eng_id != VTSS_PHY_TS_ENGINE_ID_INVALID) {
        if (stored_ts_conf->egress_eng_conf[eng_id].eng_used) {
            /*Ingress Engine Initialization*/
            if (phy_ts_eng_used_info[base_port][1][eng_id] == FALSE) { 
                VTSS_RC(vtss_phy_ts_egress_engine_init(inst,
                                port_no, eng_id,
                                stored_ts_conf->egress_eng_conf[eng_id].encap_type,
                                stored_ts_conf->egress_eng_conf[eng_id].flow_st_index,
                                stored_ts_conf->egress_eng_conf[eng_id].flow_end_index,
                                stored_ts_conf->egress_eng_conf[eng_id].flow_match_mode));
                phy_ts_eng_used_info[base_port][1][eng_id] = TRUE; 
            }
            /*Programming the Comparators/flows of the engine*/
            VTSS_RC(vtss_phy_ts_egress_engine_conf_set(inst, port_no, eng_id, &stored_ts_conf->egress_eng_conf[eng_id].flow_conf));
            /*Programming the Engine Action*/
            VTSS_RC(vtss_phy_ts_egress_engine_action_set(inst, port_no, eng_id, &stored_ts_conf->egress_eng_conf[eng_id].action_conf));
        }
        eng_id++;
    }
    VTSS_RC(vtss_phy_ts_fifo_sig_set(inst, port_no, stored_ts_conf->sig_mask));
    VTSS_RC(vtss_phy_ts_clock_rateadj_set(inst, port_no, &stored_ts_conf->rate_adj));
    VTSS_RC(vtss_phy_ts_event_enable_set(inst, port_no, TRUE, stored_ts_conf->event_mask));
#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    if (stored_ts_conf->new_spi_mode) {
        VTSS_RC(vtss_phy_ts_new_spi_mode_set(inst, port_no, stored_ts_conf->new_spi_mode));
    }
#endif
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

/* Store or Apply API CONF */
static vtss_rc phy_inst_conf(BOOL store)
{
    vtss_phy_type_t         id1g;
    vtss_port_no_t          port_no;
    inst_store_1g           *p1g;
    u32                     port_count = port_count_max();
    u32                     i;
#if defined(VTSS_CHIP_10G_PHY)
    inst_store_10g          *p10g;
    vtss_phy_10g_id_t       phy_id_10g;
#endif

    VTSS_RC(vtss_phy_detect_base_ports(phy.current_inst));
    if (store) {
        /* Store the PHY API instance configuration locally  */
        /* ************************************************* */
        memset(phy.store_1g, 0, sizeof(inst_store_1g) * VTSS_PORTS);
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        T_D("Storing 1G Phy, vtss_phy_ts_fifo_read_cb_get");
        (void)vtss_phy_ts_fifo_read_cb_get(phy.current_inst, &phy.ts_fifo_cb, &phy.cb_cntxt);
#endif
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if ((vtss_phy_id_get(phy.current_inst, port_no, &id1g) == VTSS_RC_OK) && (id1g.part_number != 0)) {
                T_I("Storing 1G Phy, port:%u",port_no);
                p1g = &phy.store_1g[port_no];
                p1g->active = 1;        
                /* Store standard 1G stuff */    
                VTSS_RC(vtss_phy_reset_get(phy.current_inst, port_no, &p1g->reset));
                VTSS_RC(vtss_phy_conf_get(phy.current_inst, port_no, &p1g->conf));
                VTSS_RC(vtss_phy_conf_1g_get(phy.current_inst, port_no, &p1g->conf_1g));
                VTSS_RC(vtss_phy_power_conf_get(phy.current_inst, port_no, &p1g->power));
                VTSS_RC(vtss_phy_loopback_get(phy.current_inst, port_no, &p1g->loopback));
                
#if defined(VTSS_FEATURE_EEE)
                VTSS_RC(vtss_phy_eee_conf_get(phy.current_inst, port_no, &p1g->eee_conf));
#endif
#if defined(VTSS_FEATURE_LED_POW_REDUC)
                VTSS_RC(vtss_phy_led_intensity_get(phy.current_inst, port_no, &p1g->led_intensity));
                VTSS_RC(vtss_phy_enhanced_led_control_init_get(phy.current_inst, port_no, &p1g->enhanced_led_control));
#endif
                T_D("put_2:%d, put_1:%d rate:%d", p1g->enhanced_led_control.ser_led_output_2, p1g->enhanced_led_control.ser_led_output_1, p1g->enhanced_led_control.ser_led_frame_rate);
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
                T_I("Storing 1G Phy, phy_ts_conf_get");
                VTSS_RC(phy_ts_conf_get(phy.current_inst, port_no));
#endif
                T_I("Storing 1G Phy, vtss_phy_event_enable_get");
                VTSS_RC(vtss_phy_event_enable_get(phy.current_inst, port_no, &p1g->ev_mask));                
                VTSS_RC(vtss_phy_event_enable_get(phy.current_inst, port_no, &p1g->ev_mask));                

                T_I("Storing 1G Phy, vtss_phy_clock_conf_get");
                for (i=0; i<VTSS_PHY_RECOV_CLK_NUM; ++i)
                    VTSS_RC(vtss_phy_clock_conf_get(phy.current_inst, port_no, i, &p1g->recov_clk[i].conf, &p1g->recov_clk[i].source));
                T_I("1g Read done");
                /* More 1G get functions here... */
            }
        }
#if defined(VTSS_CHIP_10G_PHY)            
        memset(phy.store_10g, 0, sizeof(inst_store_10g) * VTSS_PORTS);
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
         if (vtss_phy_10G_is_valid(phy.current_inst, port_no)) {
                (void)vtss_phy_10g_id_get(phy.current_inst, port_no, &phy_id_10g);
                T_I("Storing 10G phy, port:%u",port_no);
                p10g = &phy.store_10g[port_no];
                p10g->active = 1;            
                /* Store standard 10G stuff */
                VTSS_RC(vtss_phy_10g_mode_get(phy.current_inst, port_no, &p10g->mode));
                VTSS_RC(vtss_phy_10g_loopback_get(phy.current_inst, port_no, &p10g->loopback));
                VTSS_RC(vtss_phy_10g_power_get(phy.current_inst, port_no, &p10g->power));
                if (phy_id_10g.part_number != 0x8486) {
                    VTSS_RC(vtss_phy_10g_failover_get(phy.current_inst, port_no, &p10g->failover));
                }
                VTSS_RC(vtss_phy_10g_event_enable_get(phy.current_inst, port_no, &p10g->ev_mask));
                for (i=0; i<VTSS_10G_PHY_GPIO_MAX; ++i) {
                    if (phy_id_10g.part_number != 0x8486)
                        VTSS_RC(vtss_phy_10g_gpio_mode_get(phy.current_inst, port_no, i, &p10g->gpio_mode[i]));
                }
                VTSS_RC(vtss_phy_10g_synce_clkout_get(phy.current_inst, port_no, &p10g->synce_clkout));
                VTSS_RC(vtss_phy_10g_xfp_clkout_get(phy.current_inst, port_no, &p10g->xfp_clkout));

                /* More 10g get functions here... */
#if defined(VTSS_FEATURE_WIS)
                /* EWIS */
//                VTSS_RC(vtss_ewis_static_conf_get(phy.current_inst, port_no, &p10g->ewis_mode.static_conf));
                VTSS_RC(vtss_ewis_force_conf_get(phy.current_inst, port_no, &p10g->ewis_mode.force_mode));
                VTSS_RC(vtss_ewis_tx_oh_get(phy.current_inst, port_no, &p10g->ewis_mode.tx_oh));
                VTSS_RC(vtss_ewis_tx_oh_passthru_get(phy.current_inst, port_no, &p10g->ewis_mode.tx_oh_passthru));
                VTSS_RC(vtss_ewis_mode_get(phy.current_inst, port_no, &p10g->ewis_mode.ewis_mode));
                VTSS_RC(vtss_ewis_cons_act_get(phy.current_inst, port_no, &p10g->ewis_mode.section_cons_act));
                VTSS_RC(vtss_ewis_section_txti_get(phy.current_inst, port_no, &p10g->ewis_mode.section_txti));
                VTSS_RC(vtss_ewis_path_txti_get(phy.current_inst, port_no, &p10g->ewis_mode.path_txti));
                VTSS_RC(vtss_ewis_test_mode_get(phy.current_inst, port_no, &p10g->ewis_mode.test_conf));
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
                VTSS_RC(phy_ts_conf_get(phy.current_inst, port_no));
#endif
            }
        }
#endif
    } else {            
        /* Apply the stored PHY API instance configuration */
        /* *********************************************** */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        (void)vtss_phy_ts_fifo_read_install(phy.current_inst, phy.ts_fifo_cb, phy.cb_cntxt);
#endif
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            p1g = &phy.store_1g[port_no];
            if (p1g->active) {
                T_D("Applying 1g phy conf port:%u",port_no);
                /* Apply standard 1G stuff */    
                VTSS_RC(vtss_phy_reset(phy.current_inst, port_no, &p1g->reset));
                VTSS_RC(vtss_phy_conf_set(phy.current_inst, port_no, &p1g->conf));
                VTSS_RC(vtss_phy_conf_1g_set(phy.current_inst, port_no, &p1g->conf_1g));
                VTSS_RC(vtss_phy_power_conf_set(phy.current_inst, port_no, &p1g->power));
                VTSS_RC(vtss_phy_loopback_set(phy.current_inst, port_no, p1g->loopback));            
#if defined(VTSS_FEATURE_EEE)
                VTSS_RC(vtss_phy_eee_conf_set(phy.current_inst, port_no, p1g->eee_conf));
#endif
#if defined(VTSS_FEATURE_LED_POW_REDUC)
                VTSS_RC(vtss_phy_led_intensity_set(phy.current_inst, port_no, p1g->led_intensity));
                VTSS_RC(vtss_phy_enhanced_led_control_init(phy.current_inst, port_no, p1g->enhanced_led_control));
#endif
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
                VTSS_RC(phy_ts_conf_set(phy.current_inst, port_no));
#endif
                VTSS_RC(vtss_phy_event_enable_set(phy.current_inst, port_no, p1g->ev_mask, TRUE));
                for (i=0; i<VTSS_PHY_RECOV_CLK_NUM; ++i)
                    if ((p1g->recov_clk[i].conf.src == VTSS_PHY_CLK_DISABLED) || (p1g->recov_clk[i].source == port_no))
                        VTSS_RC(vtss_phy_clock_conf_set(phy.current_inst, port_no, i, &p1g->recov_clk[i].conf));
                /* More 1g set functions here... */
                T_D("1g write done");
            }
        }
#if defined(VTSS_CHIP_10G_PHY)            
        /* Must initilize the 10G chip in reversed order because channel 0 must get registered first */
        /* Normally this is done by the Port module */
        for (port_no = port_count-1;;port_no--) {
            p10g = &phy.store_10g[port_no];
            if (p10g->active) { 
                VTSS_RC(vtss_phy_10g_mode_set(phy.current_inst, port_no, &p10g->mode));                
            }            
            if (port_no == VTSS_PORT_NO_START) { break; }
        }
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            p10g = &phy.store_10g[port_no];
            if (p10g->active) {
                T_D("Applying 10g phy conf port:%u",port_no);
                /* Apply standard 10G stuff */
                VTSS_RC(vtss_phy_10g_id_get(phy.current_inst, port_no, &phy_id_10g));
                
                T_D("vtss_phy_10g_loopback_set, port:%u",port_no);
                VTSS_RC(vtss_phy_10g_loopback_set(phy.current_inst, port_no, &p10g->loopback));
                T_D("vtss_phy_10g_power_set, port:%u",port_no);
                VTSS_RC(vtss_phy_10g_power_set(phy.current_inst, port_no, &p10g->power));
                if (phy_id_10g.part_number != 0x8486) {
                    VTSS_RC(vtss_phy_10g_failover_set(phy.current_inst, port_no, &p10g->failover)); 
                }
                T_D("vtss_phy_10g_event_enable_set, port:%u",port_no);
                VTSS_RC(vtss_phy_10g_event_enable_set(phy.current_inst, port_no, p10g->ev_mask, TRUE));
                for (i=0; i<VTSS_10G_PHY_GPIO_MAX; ++i) {
                    if (phy_id_10g.part_number != 0x8486)
                        VTSS_RC(vtss_phy_10g_gpio_mode_set(phy.current_inst, port_no, i, &p10g->gpio_mode[i]));
                }
                T_D("vtss_phy_10g_synce_clkout_set, port:%u",port_no);
                VTSS_RC(vtss_phy_10g_synce_clkout_set(phy.current_inst, port_no, p10g->synce_clkout));
                VTSS_RC(vtss_phy_10g_xfp_clkout_set(phy.current_inst, port_no, p10g->xfp_clkout));

                /* More 10g set functions here... */
#if defined(VTSS_FEATURE_WIS)
                T_D("vtss_ewis_force_conf_set, port:%u",port_no);
                VTSS_RC(vtss_ewis_force_conf_set(phy.current_inst, port_no, &p10g->ewis_mode.force_mode));
                VTSS_RC(vtss_ewis_tx_oh_set(phy.current_inst, port_no, &p10g->ewis_mode.tx_oh));
                VTSS_RC(vtss_ewis_mode_set(phy.current_inst, port_no, &p10g->ewis_mode.ewis_mode));
                VTSS_RC(vtss_ewis_cons_act_set(phy.current_inst, port_no, &p10g->ewis_mode.section_cons_act));
                VTSS_RC(vtss_ewis_section_txti_set(phy.current_inst, port_no, &p10g->ewis_mode.section_txti));
                VTSS_RC(vtss_ewis_path_txti_set(phy.current_inst, port_no, &p10g->ewis_mode.path_txti));
                VTSS_RC(vtss_ewis_test_mode_set(phy.current_inst, port_no, &p10g->ewis_mode.test_conf));
#endif /* VTSS_FEATURE_WIS */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
                T_D("phy_ts_conf_set, port:%u",port_no);
                VTSS_RC(phy_ts_conf_set(phy.current_inst, port_no));
#endif
            }
        }
#endif         
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_CHIP_10G_PHY)            
static void phy_channel_failover(void)
{
    vtss_phy_10g_failover_mode_t current;
    
    if (phy.failover_port == VTSS_PORT_NO_NONE)
        return;
    
    if (vtss_phy_10g_failover_get(phy.current_inst, phy.failover_port, &current) != VTSS_RC_OK) {
        T_E("Could not complete vtss_phy_10g_failover_get() operation port:%u",phy.failover_port);
    }
    if (current == phy.failover)
        return;

    if (vtss_phy_10g_failover_set(phy.current_inst, phy.failover_port, &phy.failover) != VTSS_RC_OK) {
        T_E("Could not complete vtss_phy_10g_failover_set() operation port:%u",phy.failover_port);
    }
    T_I("Completed failover to port:%u (channel: %d)",phy.failover_port+1,phy.failover==VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL?0:1);
}
#endif

/****************************************************************************/
/*  Management functions                                                                                                           */
/****************************************************************************/
vtss_inst_t phy_mgmt_inst_get(void)
{
    vtss_inst_t inst;
    
    PHY_CRIT_ENTER();
    inst = phy.current_inst;
    PHY_CRIT_EXIT();
    return inst;
}

phy_inst_start_t phy_mgmt_start_inst_get(void)
{
    phy_inst_start_t inst;

    PHY_CRIT_ENTER();
    inst = phy.start_inst;
    PHY_CRIT_EXIT();
    return inst;
}

vtss_rc phy_mgmt_inst_restart(vtss_inst_t inst, phy_inst_restart_t restart)
{
    vtss_rc rc=VTSS_RC_OK;

    PHY_CRIT_ENTER();
    /* Store the current API Conf */
    if ((rc = phy_inst_conf(1)) != VTSS_RC_OK) {
        T_E("Problem in storing API, rc:%d", rc);
        PHY_CRIT_EXIT();
        return rc;
    }

    /* Set restart mode */
    T_I("Set the restart mode to %s",restart==WARM ? "WARM":"COOL");
    if ((rc = vtss_restart_conf_set(phy.current_inst, restart==WARM ? VTSS_RESTART_WARM : VTSS_RESTART_COOL) != VTSS_RC_OK)) {
        T_E("Could not complete vtss_restart_conf_set()");
        PHY_CRIT_EXIT();
        return rc;
    }
    /* Lock the VTSS API */
    VTSS_API_ENTER();
    /* Destroy current instance */
    T_I("Destroy the current Phy instance");
    if ((rc = vtss_inst_destroy(phy.current_inst)) != VTSS_RC_OK) {
        T_E("Could not complete vtss_inst_destroy()");
        VTSS_API_EXIT();
        PHY_CRIT_EXIT();
        return rc;
    }

    /* Create a new instance */
    T_I("Create an new Phy instance");
    if ((rc = phy_inst_create()) != VTSS_RC_OK) {
        T_E("Could not create PHY instance");
        VTSS_API_EXIT();
        PHY_CRIT_EXIT();
        return rc;

    }
    /* UnLock the VTSS API */
    VTSS_API_EXIT();

    /* Apply the stored conf to the new instance */
    if ((rc = phy_inst_conf(0)) != VTSS_RC_OK) {
        T_E("Problem in applying stored API conf, rc:%d", rc);
        PHY_CRIT_EXIT();
        return rc;
    }

    PHY_CRIT_EXIT();

    /* Synchronize API and chip */
    T_I("Synchronize API and Phys");
    if ((rc = vtss_restart_conf_end(phy.current_inst)) != VTSS_RC_OK) {
        T_E("Could not complete vtss_restart_conf_end()");
    }

    /* The API Phy instance restart is now completed. */

#if defined(VTSS_CHIP_10G_PHY)            
    /* Perform a failover if requested */
    (void)phy_channel_failover();
#endif

    return rc;
}

#if defined(VTSS_CHIP_10G_PHY)            
vtss_rc phy_mgmt_failover_set(vtss_port_no_t port_no, vtss_phy_10g_failover_mode_t *failover)
{
    PHY_CRIT_ENTER();
    phy.failover = *failover;    
    phy.failover_port = port_no;    
    PHY_CRIT_EXIT();
    return VTSS_RC_OK;
}


vtss_rc phy_mgmt_failover_get(vtss_port_no_t port_no, vtss_phy_10g_failover_mode_t *failover)
{
    PHY_CRIT_ENTER();
    *failover = phy.failover;
    PHY_CRIT_EXIT();
    return VTSS_RC_OK;
}

vtss_rc phy_mgmt_inst_activate_default(void)
{
    vtss_phy_10g_mode_t      mode;
    vtss_port_no_t           port_no;
    u32                      port_count = port_count_max();

    for (port_no = port_count-1;;port_no--) {
        if (vtss_phy_10G_is_valid(phy.current_inst, port_no)) {
            VTSS_RC(vtss_phy_10g_mode_get(phy.current_inst, port_no, &mode));
            VTSS_RC(vtss_phy_10g_mode_set(NULL, port_no, &mode));                
        }
        if (port_no == VTSS_PORT_NO_START) { break; }
    }
    return VTSS_RC_OK;
}
#endif

vtss_rc phy_mgmt_inst_create(phy_inst_start_t inst_create)
{
    T_D("Enter phy_mgmt_inst_create\n");
    phy_conf_t *phy_blk;

    PHY_CRIT_ENTER();
    phy.start_inst = inst_create;
    PHY_CRIT_EXIT();
    /* Save to flash */
    if ((phy_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PHY_CONF, NULL)) == NULL) {
        T_E("Failed to open phy conf");
    } else {    
        PHY_CRIT_ENTER();
        phy_blk->inst = phy.start_inst; 
        phy_blk->version = PHY_CONF_VERSION; 
        PHY_CRIT_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PHY_CONF);
    }

    return VTSS_RC_OK;
}

/* Read/create and activate configuration */
static void phy_conf_read(vtss_isid_t isid_add, BOOL force_default)
{
    phy_conf_t             *phy_blk;
    ulong                  size;
    BOOL                   create; 

    T_D("enter, isid_add: %d", isid_add);
    /* Restore. Open or create configuration block */
    if ((phy_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PHY_CONF, &size)) == NULL) {
        create = 1;
        T_D("Could not open conf block. Defaulting.");
    } else {
        /* If the configuration size have changed then create a block defaults */
        if (size != sizeof(*phy_blk)) {
            create = 1;
            T_W("Configuration size have changed, creating defaults");
        } else if (phy_blk->version != PHY_CONF_VERSION) {
            T_W("Version mismatch, creating defaults");
            create = 1;
        } else {
            create = (isid_add != VTSS_ISID_GLOBAL);
        }
    }
    /* Defaulting */
    if (create || force_default) {
        T_D("Defaulting configuration.");
        phy_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_PHY_CONF, sizeof(phy_conf_t));

        /* Write to the global structure */
        phy.start_inst = PHY_INST_NONE;

        /* Write to flash */
        if (phy_blk != NULL) {
            phy_blk->inst = phy.start_inst;
            phy_blk->version = PHY_CONF_VERSION;
        } 
    } else {
        /* Use stored configuration */
        T_D("Using stored config");

        /* Move entries from flash to local structure */
        if (phy_blk != NULL) {
            phy.start_inst = phy_blk->inst;
        }
    }

    if (phy_blk == NULL) {
        T_E("Failed to open phy config table");
    } else {
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PHY_CONF);
    }

    T_D("exit");
}

vtss_rc phy_init(vtss_init_data_t *data)
{

    if (data->cmd == INIT_CMD_INIT) {
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        phy.current_inst = NULL;
        phy.failover_port = VTSS_PORT_NO_NONE;
        /* Create API semaphore (initially locked) */
        critd_init(&phy.crit, "phy_crit", VTSS_TRACE_MODULE_ID, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        /* Initialize API */
        phy_conf_read(VTSS_ISID_GLOBAL,0);
#if defined(VTSS_SW_OPTION_ICFG)
        vtss_rc rc = phy_icfg_init();
        if (rc != VTSS_OK) {
            T_E("%% Error fail to init phy icfg registration, rc = %s", error_txt(rc));
        }
#endif
        VTSS_API_ENTER();
        (void)phy_inst_create();
        VTSS_API_EXIT();
        PHY_CRIT_EXIT();
        break;
    case INIT_CMD_START:
        break;
    case INIT_CMD_CONF_DEF:
        phy_conf_read(VTSS_ISID_GLOBAL,1);
        break;
    case INIT_CMD_MASTER_UP:
        break;
    case INIT_CMD_MASTER_DOWN:
        break;
    case INIT_CMD_SWITCH_ADD:
        break;
    case INIT_CMD_SWITCH_DEL:
        break;
    default:
        break;
    }

    T_D("exit");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/


