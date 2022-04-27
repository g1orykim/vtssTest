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

*/
#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "vtss_module_id.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "mgmt_api.h"
#include "phy_api.h"
#include "vtss_tod_phy_engine.h"
#include "vtss_tod_mod_man.h"



#include "tod_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TOD

#define API_INST_DEFAULT PHY_INST

/******************************************************************************/
// Lint stuff.
/******************************************************************************/
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_init) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_clear) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_mode) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_channel_map) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_eth1_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_eth1_flow_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_eth2_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_eth2_flow_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_ip1_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_ip1_flow_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_ip2_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_ip2_flow_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_mpls_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_mpls_flow_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_ach_comm_conf) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_action_add) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_action_delete) */
/*lint -esym(459, cli_cmd_debug_phy_1588_engine_action_show) */
/*lint -esym(459, cli_cmd_debug_phy_1588_latency) */
/*lint -esym(459, cli_cmd_debug_phy_1588_delay) */
/*lint -esym(459, cli_cmd_debug_phy_1588_delay_asym) */
/*lint -esym(459, cli_cmd_phy_1588_block_init) */

typedef struct {
    BOOL        ingress;
    BOOL        enable;
    BOOL        disable;
    u8          port_no;
    u8          eng_id;
    u8          encap_type;
    u8          flow_st_index;
    u8          flow_end_index;
    u8          flow_match_mode;
    u8          channel_map;
    BOOL        flow_id_spec;
    u8          flow_id;
    BOOL        pbb_spec;
    BOOL        pbb_en;
    u16         etype;
    u16         tpid;
    BOOL        mac_match_mode_spec;
    u8          mac_match_mode;
    BOOL        ptp_mac_spec;
    u8          ptp_mac[6];
    BOOL        vlan_chk_spec;
    BOOL        vlan_chk;
    BOOL        num_tag_spec;
    u8          num_tag;
    BOOL        tag_rng_mode_spec;
    u8          tag_rng_mode;
    u8          tag1_type;
    u8          tag2_type;
    u16         tag1_lower;
    u16         tag1_upper;
    u16         tag2_lower;
    u16         tag2_upper;
    BOOL        ip_mode_spec;
    u8          ip_mode;
    BOOL        sport_spec;
    u16         sport_val;
    u16         sport_mask;
    BOOL        dport_spec;
    u16         dport_val;
    u16         dport_mask;
    BOOL        addr_match_select_spec;
    u8          addr_match_select;
    BOOL        ipv4_addr_spec;
    vtss_ipv4_t ipv4_addr;
    BOOL        ipv4_mask_spec;
    vtss_ipv4_t ipv4_mask;
    u32         ipv6_addr[4];
    u32         ipv6_mask[4];
    BOOL        stk_depth_spec;
    u8          stk_depth;
    BOOL        stk_ref_point_spec;
    u8          stk_ref_point;
    BOOL        stk_lvl_0;
    u32         stk_lvl_0_lower;
    u32         stk_lvl_0_upper;
    BOOL        stk_lvl_1;
    u32         stk_lvl_1_lower;
    u32         stk_lvl_1_upper;
    BOOL        stk_lvl_2;
    u32         stk_lvl_2_lower;
    u32         stk_lvl_2_upper;
    BOOL        stk_lvl_3;
    u32         stk_lvl_3_lower;
    u32         stk_lvl_3_upper;
    BOOL        cw_en;
    BOOL        ach_ver_spec;
    u8          ach_ver;
    BOOL        channel_type_spec;
    u16         channel_type;
    BOOL        proto_id_spec;
    u16         proto_id;
    u8          action_id;
    BOOL        ptp_spec;
    u8          clk_mode;
    u8          delaym;
    u8          domain_meg_lower;
    u8          domain_meg_upper;
    BOOL        y1731_oam_spec;
    BOOL        ietf_oam_spec;
    u8          ietf_tf;
    u8          ietf_ds;
    u8          sig_mask;
    u8          version;
    u8          time_sec;
    vtss_timeinterval_t  latency_val;
    vtss_timeinterval_t  delay_val; 
    vtss_timeinterval_t  asym_val;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    vtss_timeinterval_t  ing_delay_val;
    vtss_timeinterval_t  egr_delay_val;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
    vtss_tod_internal_tc_mode_t tc_int_mode;
    u8          clk_src;
    u8          rx_ts_pos;
    u8          tx_fifo_mode;
    u8          clkfreq;
    u8          clk_freq_spec;
    BOOL        clk_src_spec;
    BOOL        rx_ts_pos_spec;
    BOOL        tx_fifo_mode_spec;
    BOOL        modify_frame_spec;
    BOOL        modify_frame;
    vtss_phy_ts_encap_t encap_mode;
    vtss_phy_ts_nphase_sampler_t nphase_sampler;
} tod_cli_req_t;
              
typedef enum  {
    VTSS_PTP_ONE_PPS_DISABLE, /* 1 pps not used */
    VTSS_PTP_ONE_PPS_OUTPUT,  /* 1 pps output */
    VTSS_PTP_ONE_PPS_INPUT,   /* 1 pps input */
} ptp_ext_clock_1pps_t;


/* external clock output configuration */
typedef struct vtss_ptp_ext_clock_mode_t {
    ptp_ext_clock_1pps_t   one_pps_mode;    /* Select 1pps mode:
                                input : lock clock to 1pps input
                                output: enable external sync pulse output
                                disable: disable 1 pps */
    BOOL clock_out_enable;  /* Enable programmable clock output
                                clock frequency = 'freq' */
    BOOL vcxo_enable;       /* Enable use of external VCXO for rate adjustment */
    u32  freq;              /* clock output frequency (hz [1..25.000.000]). */
} vtss_ptp_ext_clock_mode_t;


void tod_cli_init(void)
{
    /* register the size required for tod req. structure */
    cli_req_size_register(sizeof(tod_cli_req_t));
}

static void tod_cli_req_default_set(cli_req_t * req)
{

    tod_cli_req_t *tod_req = req->module_req;
    memset(tod_req, 0, sizeof(tod_cli_req_t));
}

struct {
    BOOL  init;
    u8    encap_type;
    u8    flow_match_mode;
    u16   flow_st_index;
    u16   flow_end_index;
} engine_init_info[VTSS_PORTS][2][4]; /* 2->ingress/egress, 4->num_engine */
//} engine_init_info[28][2][4]; /* 2->ingress/egress, 4->num_engine */

static vtss_rc vtss_phy_ts_base_port_get(const vtss_port_no_t port_no,
                                         vtss_port_no_t     *const base_port_no)
{
#if defined(VTSS_FEATURE_10G)
    vtss_phy_10g_id_t phy_id_10g;
#endif /* VTSS_FEATURE_10G */
    vtss_phy_type_t   phy_id_1g;

    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
#if defined(VTSS_FEATURE_10G)
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get(API_INST_DEFAULT, port_no, &phy_id_10g) == VTSS_RC_OK) {
        /* Get base port_no for 10G */
        *base_port_no = phy_id_10g.phy_api_base_no;
    } else if (vtss_phy_id_get(API_INST_DEFAULT, port_no, &phy_id_1g) == VTSS_RC_OK) {
#else
    if (vtss_phy_id_get(API_INST_DEFAULT, port_no, &phy_id_1g) == VTSS_RC_OK) {
#endif /* VTSS_FEATURE_10G */
        *base_port_no = phy_id_1g.phy_api_base_no;
    } else {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static void cli_cmd_debug_phy_1588_engine_init(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no, base_port;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    u8 flow_match_mode;
    char *flow_match = NULL; 
    vtss_rc rc;

    port_no = uport2iport(tod_req->port_no);
    if ((rc = vtss_phy_ts_base_port_get(port_no, &base_port)) != VTSS_RC_OK) {
        CPRINTF("Engine initialized failed!!!\n");
        return;
    }

    if (req->set == FALSE) {
        if(engine_init_info[base_port][dir][eng_id].init == FALSE) {
            CPRINTF("Engine not initialized!\n");
        } else {
            flow_match_mode = engine_init_info[base_port][dir][eng_id].flow_match_mode;
            if (flow_match_mode == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                flow_match = "strict";
            } else {
                flow_match = "non-strict";
            }
            CPRINTF("encap_type = %d flow_start = %d flow_end = %d flow_match = %s\n",
                    engine_init_info[base_port][dir][eng_id].encap_type,
                    engine_init_info[base_port][dir][eng_id].flow_st_index,
                    engine_init_info[base_port][dir][eng_id].flow_end_index,
                    flow_match);
        }
    } else {
        flow_match_mode = tod_req->flow_match_mode;
                
        CPRINTF("port_no = %d, ingress = %d, eng_id = %d, encap = %d, st_ind = %d, end_ind = %d, match_mode = %d\n",
                (int)port_no, tod_req->ingress, eng_id, tod_req->encap_type,
                tod_req->flow_st_index, tod_req->flow_end_index, flow_match_mode);
        if (tod_req->ingress) {
            rc = vtss_phy_ts_ingress_engine_init(API_INST_DEFAULT, port_no, eng_id,
                             tod_req->encap_type,
                             tod_req->flow_st_index,
                             tod_req->flow_end_index,
                             flow_match_mode);
        } else {
            rc = vtss_phy_ts_egress_engine_init(API_INST_DEFAULT, port_no, eng_id,
                             tod_req->encap_type,
                             tod_req->flow_st_index,
                             tod_req->flow_end_index,
                             flow_match_mode);
        }
        if (rc == VTSS_RC_OK) {
            engine_init_info[base_port][dir][eng_id].init = TRUE;
            engine_init_info[base_port][dir][eng_id].encap_type = tod_req->encap_type;
            engine_init_info[base_port][dir][eng_id].flow_st_index = tod_req->flow_st_index;
            engine_init_info[base_port][dir][eng_id].flow_end_index = tod_req->flow_end_index;
            engine_init_info[base_port][dir][eng_id].flow_match_mode = tod_req->flow_match_mode;
            engine_init_info[port_no][dir][eng_id] = engine_init_info[base_port][dir][eng_id];
            CPRINTF("Engine init success\n");
        } else {
            CPRINTF("Engine init Failed!\n");
        }
    }
}

static void cli_cmd_debug_phy_1588_engine_clear(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no, base_port;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;

    port_no = uport2iport(tod_req->port_no);

    if ((rc = vtss_phy_ts_base_port_get(port_no, &base_port)) != VTSS_RC_OK) {
        CPRINTF("Engine initialized failed!!!\n");
        return;
    }
    if(engine_init_info[base_port][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
    } else {
        if (tod_req->ingress) {
            rc = vtss_phy_ts_ingress_engine_clear(API_INST_DEFAULT, port_no, eng_id);
        } else {
            rc = vtss_phy_ts_egress_engine_clear(API_INST_DEFAULT, port_no, eng_id);
        }
        if (rc == VTSS_RC_OK) {
            engine_init_info[base_port][dir][eng_id].init = FALSE;
            engine_init_info[base_port][dir][eng_id].encap_type = 0;
            engine_init_info[base_port][dir][eng_id].flow_st_index = 0;
            engine_init_info[base_port][dir][eng_id].flow_end_index = 0;
            engine_init_info[base_port][dir][eng_id].flow_match_mode = 0;
            engine_init_info[port_no][dir][eng_id] = engine_init_info[base_port][dir][eng_id];
            CPRINTF("Engine clear success\n");
        } else {
            CPRINTF("Engine init Failed!\n");
        }
    }
}

static void cli_cmd_debug_phy_1588_engine_mode(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL, *conf_ptr;
    vtss_rc rc;

    port_no = uport2iport(tod_req->port_no);
    if(engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }
    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            flow_conf->eng_mode = tod_req->enable;
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("Engine Mode:%d\n", flow_conf->eng_mode);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_channel_map(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;
    vtss_rc rc;
    int i;

    port_no = uport2iport(tod_req->port_no);
    if(engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }
    if (tod_req->flow_id_spec &&
        (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index)) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (tod_req->flow_id_spec) {
                flow_conf->channel_map[tod_req->flow_id] = tod_req->channel_map;
            } else {
                for (i = engine_init_info[port_no][dir][eng_id].flow_st_index;
                     i < engine_init_info[port_no][dir][eng_id].flow_end_index; i++) {
                    flow_conf->channel_map[i] = tod_req->channel_map;
                }
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("Channel_map: ");
            for (i = engine_init_info[port_no][dir][eng_id].flow_st_index;
                 i <= engine_init_info[port_no][dir][eng_id].flow_end_index; i++) {
                CPRINTF("%x ", flow_conf->channel_map[i]);
            }
            CPRINTF("\n");
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_eth1_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (tod_req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth1_conf = &flow_conf->flow_conf.ptp.eth1_opt;
    } else {
        eth1_conf = &flow_conf->flow_conf.oam.eth1_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (tod_req->pbb_spec) eth1_conf->comm_opt.pbb_en = tod_req->pbb_en;
            eth1_conf->comm_opt.etype = (tod_req->etype ? tod_req->etype : eth1_conf->comm_opt.etype);
            eth1_conf->comm_opt.tpid = (tod_req->tpid ? tod_req->tpid : eth1_conf->comm_opt.tpid);
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("ETH1 conf: pbb = %d, etype = 0x%x, tpid = 0x%x\n",
                    eth1_conf->comm_opt.pbb_en, eth1_conf->comm_opt.etype,
                    eth1_conf->comm_opt.tpid);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_eth1_flow_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    if (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (tod_req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth1_conf = &flow_conf->flow_conf.ptp.eth1_opt;
    } else {
        eth1_conf = &flow_conf->flow_conf.oam.eth1_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth1_conf->flow_opt[tod_req->flow_id].flow_en = tod_req->enable;
            if (tod_req->mac_match_mode_spec) {
                eth1_conf->flow_opt[tod_req->flow_id].addr_match_mode = tod_req->mac_match_mode;
            }
            if (tod_req->ptp_mac_spec) {
                memcpy(eth1_conf->flow_opt[tod_req->flow_id].mac_addr, tod_req->ptp_mac, 6);
            }
            if (tod_req->addr_match_select_spec) {
                if (tod_req->addr_match_select == 0) {
                    eth1_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_ADDR;
                } else if (tod_req->addr_match_select == 1) {
                    eth1_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
                } else {
                    eth1_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_OR_DEST;
                }
            }
            if (tod_req->vlan_chk_spec) {
                eth1_conf->flow_opt[tod_req->flow_id].vlan_check = tod_req->vlan_chk;
            }
            if (tod_req->num_tag_spec) {
                eth1_conf->flow_opt[tod_req->flow_id].num_tag = tod_req->num_tag;
            }
            if (tod_req->tag_rng_mode_spec) {
                eth1_conf->flow_opt[tod_req->flow_id].tag_range_mode = tod_req->tag_rng_mode;
            }
            if (tod_req->tag1_type > 0 && tod_req->tag1_type < 5) {
                eth1_conf->flow_opt[tod_req->flow_id].outer_tag_type = tod_req->tag1_type;
            }
            if (tod_req->tag2_type > 0 && tod_req->tag2_type < 5) {
                eth1_conf->flow_opt[tod_req->flow_id].inner_tag_type = tod_req->tag2_type;
            }
            if (tod_req->tag_rng_mode_spec && tod_req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                if (tod_req->tag1_lower) {
                    eth1_conf->flow_opt[tod_req->flow_id].outer_tag.range.lower = tod_req->tag1_lower;
                }
                if (tod_req->tag1_upper) {
                    eth1_conf->flow_opt[tod_req->flow_id].outer_tag.range.upper = tod_req->tag1_upper;
                }
            } else {
                eth1_conf->flow_opt[tod_req->flow_id].outer_tag.value.val = tod_req->tag1_lower;
                eth1_conf->flow_opt[tod_req->flow_id].outer_tag.value.mask = tod_req->tag1_upper;
            }
            if (tod_req->tag_rng_mode_spec && tod_req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                if ((tod_req->num_tag_spec) && (tod_req->num_tag == 1))
                {
                    if (tod_req->tag1_lower) {
                        eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.lower = tod_req->tag1_lower;
                    }
                    if (tod_req->tag1_upper) {
                        eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.upper = tod_req->tag1_upper;
                    }
                }
                if (tod_req->tag2_lower) {
                    eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.lower = tod_req->tag2_lower;
                }
                if (tod_req->tag2_upper) {
                    eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.upper = tod_req->tag2_upper;
                }
            } else {
                eth1_conf->flow_opt[tod_req->flow_id].inner_tag.value.val = tod_req->tag2_lower;
                eth1_conf->flow_opt[tod_req->flow_id].inner_tag.value.mask = tod_req->tag2_upper;
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("ETH1 flow conf: enable = %d, match_mode = %d, mac = 0x%x-%x-%x-%x-%x-%x\n",
                    eth1_conf->flow_opt[tod_req->flow_id].flow_en,
                    eth1_conf->flow_opt[tod_req->flow_id].addr_match_mode,
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[0],
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[1],
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[2],
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[3],
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[4],
                    eth1_conf->flow_opt[tod_req->flow_id].mac_addr[5]);
            CPRINTF("match_select = %d, vlan_chk = %d, num_tag = %d, range_mode = %d\n",
                     eth1_conf->flow_opt[tod_req->flow_id].addr_match_select,
                     eth1_conf->flow_opt[tod_req->flow_id].vlan_check,
                     eth1_conf->flow_opt[tod_req->flow_id].num_tag,
                     eth1_conf->flow_opt[tod_req->flow_id].tag_range_mode);
            CPRINTF("tag1_type = %d, tag2_type = %d, tag1_lower = %d, upper = %d, tag2_lower = %d, upper = %d\n",
                    eth1_conf->flow_opt[tod_req->flow_id].outer_tag_type,
                    eth1_conf->flow_opt[tod_req->flow_id].inner_tag_type,
                    eth1_conf->flow_opt[tod_req->flow_id].outer_tag.range.lower,
                    eth1_conf->flow_opt[tod_req->flow_id].outer_tag.range.upper,
                    eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.lower,
                    eth1_conf->flow_opt[tod_req->flow_id].inner_tag.range.upper);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_eth2_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (tod_req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth2_conf = &flow_conf->flow_conf.ptp.eth2_opt;
    } else {
        eth2_conf = &flow_conf->flow_conf.oam.eth2_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth2_conf->comm_opt.etype = (tod_req->etype ? tod_req->etype : eth2_conf->comm_opt.etype);
            eth2_conf->comm_opt.tpid = (tod_req->tpid ? tod_req->tpid : eth2_conf->comm_opt.tpid);
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("ETH2 conf: etype = 0x%x, tpid = 0x%x\n",
                    eth2_conf->comm_opt.etype,
                    eth2_conf->comm_opt.tpid);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_eth2_flow_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    if (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (tod_req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth2_conf = &flow_conf->flow_conf.ptp.eth2_opt;
    } else {
        eth2_conf = &flow_conf->flow_conf.oam.eth2_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth2_conf->flow_opt[tod_req->flow_id].flow_en = tod_req->enable;
            if (tod_req->mac_match_mode_spec) {
                eth2_conf->flow_opt[tod_req->flow_id].addr_match_mode = tod_req->mac_match_mode;
            }
            if (tod_req->ptp_mac_spec) {
                memcpy(eth2_conf->flow_opt[tod_req->flow_id].mac_addr, tod_req->ptp_mac, 6);
            }
            if (tod_req->addr_match_select_spec) {
                if (tod_req->addr_match_select == 0) {
                    eth2_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_ADDR;
                } else if (tod_req->addr_match_select == 1) {
                    eth2_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
                } else {
                    eth2_conf->flow_opt[tod_req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_OR_DEST;
                }
            }
            if (tod_req->vlan_chk_spec) {
                eth2_conf->flow_opt[tod_req->flow_id].vlan_check = tod_req->vlan_chk;
            }
            if (tod_req->num_tag_spec) {
                eth2_conf->flow_opt[tod_req->flow_id].num_tag = tod_req->num_tag;
            }
            if (tod_req->tag_rng_mode_spec) {
                eth2_conf->flow_opt[tod_req->flow_id].tag_range_mode = tod_req->tag_rng_mode;
            }
            if (tod_req->tag1_type > 0 && tod_req->tag1_type < 5) {
                eth2_conf->flow_opt[tod_req->flow_id].outer_tag_type = tod_req->tag1_type;
            }
            if (tod_req->tag2_type > 0 && tod_req->tag2_type < 5) {
                eth2_conf->flow_opt[tod_req->flow_id].inner_tag_type = tod_req->tag2_type;
            }

            if (tod_req->tag_rng_mode_spec && tod_req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                if (tod_req->tag1_lower) {
                    eth2_conf->flow_opt[tod_req->flow_id].outer_tag.range.lower = tod_req->tag1_lower;
                }
                if (tod_req->tag1_upper) {
                    eth2_conf->flow_opt[tod_req->flow_id].outer_tag.range.upper = tod_req->tag1_upper;
                }
            } else {
                eth2_conf->flow_opt[tod_req->flow_id].outer_tag.value.val = tod_req->tag1_lower;
                eth2_conf->flow_opt[tod_req->flow_id].outer_tag.value.mask = tod_req->tag1_upper;
            }
            if (tod_req->tag_rng_mode_spec && tod_req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                if (tod_req->tag2_lower) {
                    eth2_conf->flow_opt[tod_req->flow_id].inner_tag.range.lower = tod_req->tag2_lower;
                }
                if (tod_req->tag2_upper) {
                    eth2_conf->flow_opt[tod_req->flow_id].inner_tag.range.upper = tod_req->tag2_upper;
                }
            } else {
                eth2_conf->flow_opt[tod_req->flow_id].inner_tag.value.val = tod_req->tag2_lower;
                eth2_conf->flow_opt[tod_req->flow_id].inner_tag.value.mask = tod_req->tag2_upper;
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("ETH2 flow conf: enable = %d, match_mode = %d, mac = 0x%x-%x-%x-%x-%x-%x\n",
                    eth2_conf->flow_opt[tod_req->flow_id].flow_en,
                    eth2_conf->flow_opt[tod_req->flow_id].addr_match_mode,
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[0],
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[1],
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[2],
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[3],
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[4],
                    eth2_conf->flow_opt[tod_req->flow_id].mac_addr[5]);
            CPRINTF("match_select = %d, vlan_chk = %d, num_tag = %d, range_mode = %d\n",
                     eth2_conf->flow_opt[tod_req->flow_id].addr_match_select,
                     eth2_conf->flow_opt[tod_req->flow_id].vlan_check,
                     eth2_conf->flow_opt[tod_req->flow_id].num_tag,
                     eth2_conf->flow_opt[tod_req->flow_id].tag_range_mode);
            CPRINTF("tag1_type = %d, tag2_type = %d, tag1_lower = %d, upper = %d, tag2_lower = %d, upper = %d\n",
                    eth2_conf->flow_opt[tod_req->flow_id].outer_tag_type,
                    eth2_conf->flow_opt[tod_req->flow_id].inner_tag_type,
                    eth2_conf->flow_opt[tod_req->flow_id].outer_tag.range.lower,
                    eth2_conf->flow_opt[tod_req->flow_id].outer_tag.range.upper,
                    eth2_conf->flow_opt[tod_req->flow_id].inner_tag.range.lower,
                    eth2_conf->flow_opt[tod_req->flow_id].inner_tag.range.upper);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_ip1_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        tod_req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    ip1_conf = &flow_conf->flow_conf.ptp.ip1_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (tod_req->ip_mode_spec) {
                ip1_conf->comm_opt.ip_mode = tod_req->ip_mode;
            }
            if (tod_req->sport_spec) {
                ip1_conf->comm_opt.sport_val = tod_req->sport_val;
                ip1_conf->comm_opt.sport_mask = tod_req->sport_mask;
            }
            if (tod_req->dport_spec) {
                ip1_conf->comm_opt.dport_val = tod_req->dport_val;
                ip1_conf->comm_opt.dport_mask = tod_req->dport_mask;
            }
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("IP1 conf: mode = %d, sport_val = %d, mask = 0x%x, dport_val = %d, mask = 0x%x\n",
                    ip1_conf->comm_opt.ip_mode,
                    ip1_conf->comm_opt.sport_val, ip1_conf->comm_opt.sport_mask,
                    ip1_conf->comm_opt.dport_val, ip1_conf->comm_opt.dport_mask);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_ip1_flow_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        tod_req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    if (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    ip1_conf = &flow_conf->flow_conf.ptp.ip1_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if ((ip1_conf->comm_opt.ip_mode != tod_req->ip_mode)&& tod_req->enable) {
                VTSS_FREE(flow_conf);
                CPRINTF("IP mode mismatch!\n");
                return;
            }

            ip1_conf->flow_opt[tod_req->flow_id].flow_en = tod_req->enable;
            if (tod_req->addr_match_select_spec) {
                if (tod_req->addr_match_select == 0) {
                    ip1_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                } else if (tod_req->addr_match_select == 1) {
                    ip1_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                } else {
                    ip1_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC_OR_DEST;
                }
            }
            if (ip1_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                if (tod_req->ipv4_addr_spec) {
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.addr = tod_req->ipv4_addr;
                }
                if (tod_req->ipv4_mask_spec) {
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.mask = tod_req->ipv4_mask;
                }
            } else {         
                if (tod_req->ipv4_addr_spec) {
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]= tod_req->ipv6_addr[0];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1]= tod_req->ipv6_addr[1];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2]= tod_req->ipv6_addr[2];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3]= tod_req->ipv6_addr[3];
                }
                if (tod_req->ipv4_mask_spec) {
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]= tod_req->ipv6_mask[0];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1]= tod_req->ipv6_mask[1];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2]= tod_req->ipv6_mask[2];
                    ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3]= tod_req->ipv6_mask[3];
                }
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            if (ip1_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {  
                CPRINTF("IP1 flow_conf: enable = %d, match_mode = %d, addr = 0x%x, mask = 0x%x\n",
                    ip1_conf->flow_opt[tod_req->flow_id].flow_en,
                    ip1_conf->flow_opt[tod_req->flow_id].match_mode,
                    (unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.addr,
                    (unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.mask);
            } else {
                CPRINTF("IP1 flow_conf: enable = %d, match_mode = %d, addr = %x:%x:%x:%x:%x:%x:%x:%x, mask = %x:%x:%x:%x:%x:%x:%x:%x\n",
                    ip1_conf->flow_opt[tod_req->flow_id].flow_en,
                    ip1_conf->flow_opt[tod_req->flow_id].match_mode,
                    (unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]&0xFFFF,
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3]>>16) & 0xFFFF),
                    (unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]&0xFFFF,
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3]>>16) & 0xFFFF));
            }
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_ip2_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        tod_req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    ip2_conf = &flow_conf->flow_conf.ptp.ip2_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (tod_req->ip_mode_spec) {
                ip2_conf->comm_opt.ip_mode = tod_req->ip_mode;
            }
            if (tod_req->sport_spec) {
                ip2_conf->comm_opt.sport_val = tod_req->sport_val;
                ip2_conf->comm_opt.sport_mask = tod_req->sport_mask;
            }
            if (tod_req->dport_spec) {
                ip2_conf->comm_opt.dport_val = tod_req->dport_val;
                ip2_conf->comm_opt.dport_mask = tod_req->dport_mask;
            }
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("IP2 conf: mode = %d, sport_val = %d, mask = 0x%x, dport_val = %d, mask = 0x%x\n",
                    ip2_conf->comm_opt.ip_mode,
                    ip2_conf->comm_opt.sport_val, ip2_conf->comm_opt.sport_mask,
                    ip2_conf->comm_opt.dport_val, ip2_conf->comm_opt.dport_mask);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_ip2_flow_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        tod_req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    if (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    ip2_conf = &flow_conf->flow_conf.ptp.ip2_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if ((ip2_conf->comm_opt.ip_mode != tod_req->ip_mode)&& tod_req->enable) {
                VTSS_FREE(flow_conf);
                CPRINTF("IP mode mismatch!\n");
                return;
            }

            ip2_conf->flow_opt[tod_req->flow_id].flow_en = tod_req->enable;
            if (tod_req->addr_match_select_spec) {
                if (tod_req->addr_match_select == 0) {
                    ip2_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                } else if (tod_req->addr_match_select == 1) {
                    ip2_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                } else {
                    ip2_conf->flow_opt[tod_req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC_OR_DEST;
                }
            }
        if (ip2_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                if (tod_req->ipv4_addr_spec) {
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.addr = tod_req->ipv4_addr;
                }
                if (tod_req->ipv4_mask_spec) {
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.mask = tod_req->ipv4_mask;
                }
            } else {

                if (tod_req->ipv4_addr_spec) {
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]= tod_req->ipv6_addr[0];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1]= tod_req->ipv6_addr[1];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2]= tod_req->ipv6_addr[2];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3]= tod_req->ipv6_addr[3];
                }
                if (tod_req->ipv4_mask_spec) {
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]= tod_req->ipv6_mask[0];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1]= tod_req->ipv6_mask[1];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2]= tod_req->ipv6_mask[2];
                    ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3]= tod_req->ipv6_mask[3];
                }
            }
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            if (ip2_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {  
                CPRINTF("IP2 flow_conf: enable = %d, match_mode = %d, addr = 0x%x, mask = 0x%x\n",
                    ip2_conf->flow_opt[tod_req->flow_id].flow_en,
                    ip2_conf->flow_opt[tod_req->flow_id].match_mode,
                    (unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.addr,
                    (unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv4.mask);
            } else {
                 CPRINTF("IP2 flow_conf: enable = %d, match_mode = %d, addr = %x:%x:%x:%x:%x:%x:%x:%x, mask = %x:%x:%x:%x:%x:%x:%x:%x\n",
                        ip2_conf->flow_opt[tod_req->flow_id].flow_en,
                        ip2_conf->flow_opt[tod_req->flow_id].match_mode,
                        (unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]&0xFFFF,
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[0]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[1]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[2]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.addr[3]>>16) & 0xFFFF),
                        (unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]&0xFFFF,
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[0]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[1]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[2]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[tod_req->flow_id].ip_addr.ipv6.mask[3]>>16) & 0xFFFF));
            }
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_mpls_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_mpls_conf_t *mpls_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ){ 
        mpls_conf = &flow_conf->flow_conf.oam.mpls_opt;
    } else if((engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP) || 
        (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP)) {
        mpls_conf = &flow_conf->flow_conf.ptp.mpls_opt;
    } else { 
        CPRINTF("Invalid Operation: Engine encapsulation type !\n");
        VTSS_FREE(flow_conf);
        return;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (mpls_conf->comm_opt.cw_en == tod_req->cw_en){
                VTSS_FREE(flow_conf);
                CPRINTF("Success\n");
                return;
            }
            mpls_conf->comm_opt.cw_en = tod_req->cw_en;
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("MPLS Control Word: enable = %d\n", mpls_conf->comm_opt.cw_en);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_mpls_flow_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_mpls_conf_t *mpls_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    if (tod_req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        tod_req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        CPRINTF("Invalid flow ID!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM || 
        engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        mpls_conf = &flow_conf->flow_conf.oam.mpls_opt;
    } else {
        mpls_conf = &flow_conf->flow_conf.ptp.mpls_opt;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            mpls_conf->flow_opt[tod_req->flow_id].flow_en = tod_req->enable;
            if (tod_req->stk_depth_spec) {
                mpls_conf->flow_opt[tod_req->flow_id].stack_depth = tod_req->stk_depth;
            }
            if (tod_req->stk_ref_point_spec) {
                mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point = tod_req->stk_ref_point;
            }
            if (tod_req->stk_lvl_0) {
                if (mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.top.lower = tod_req->stk_lvl_0_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.top.upper = tod_req->stk_lvl_0_upper;
                } else {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.end.lower = tod_req->stk_lvl_0_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.end.upper = tod_req->stk_lvl_0_upper;
                }
            }
            if (tod_req->stk_lvl_1) {
                if (mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.frst_lvl_after_top.lower = tod_req->stk_lvl_1_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.frst_lvl_after_top.upper = tod_req->stk_lvl_1_upper;
                } else {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.frst_lvl_before_end.lower = tod_req->stk_lvl_1_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.frst_lvl_before_end.upper = tod_req->stk_lvl_1_upper;
                }
            }
            if (tod_req->stk_lvl_2) {
                if (mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.snd_lvl_after_top.lower = tod_req->stk_lvl_2_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.snd_lvl_after_top.upper = tod_req->stk_lvl_2_upper;
                } else {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.snd_lvl_before_end.lower = tod_req->stk_lvl_2_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.snd_lvl_before_end.upper = tod_req->stk_lvl_2_upper;
                }
            }
            if (tod_req->stk_lvl_3) {
                if (mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.thrd_lvl_after_top.lower = tod_req->stk_lvl_3_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.thrd_lvl_after_top.upper = tod_req->stk_lvl_3_upper;
                } else {
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.lower = tod_req->stk_lvl_3_lower;
                    mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.upper = tod_req->stk_lvl_3_upper;
                }
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("MPLS flow_conf: enable = %d, stack_depth = %d, ref_point = %d\n",
                     mpls_conf->flow_opt[tod_req->flow_id].flow_en,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_depth,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point);
            if (mpls_conf->flow_opt[tod_req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                CPRINTF("level_0 = %u - %u, level_1 = %u - %u, level_2 = %u - %u, level_3 = %u - %u\n",
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.top.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.top.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.frst_lvl_after_top.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.frst_lvl_after_top.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.snd_lvl_after_top.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.snd_lvl_after_top.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.thrd_lvl_after_top.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.top_down.thrd_lvl_after_top.upper);
            } else {
                CPRINTF("level_0 = %u - %u, level_1 = %u - %u, level_2 = %u - %u, level_3 = %u - %u\n",
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.end.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.end.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.frst_lvl_before_end.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.frst_lvl_before_end.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.snd_lvl_before_end.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.snd_lvl_before_end.upper,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.lower,
                     mpls_conf->flow_opt[tod_req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.upper);
            }
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_ach_comm_conf(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ach_conf_t *ach_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        tod_req->eng_id > VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    flow_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }

    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        ach_conf = &flow_conf->flow_conf.oam.ach_opt;
    } else {
        ach_conf = &flow_conf->flow_conf.ptp.ach_opt;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (tod_req->ach_ver_spec) {
                ach_conf->comm_opt.version.value = tod_req->ach_ver;
                ach_conf->comm_opt.version.mask = 0xF;
            }
            if (tod_req->channel_type_spec) {
                ach_conf->comm_opt.channel_type.value = tod_req->channel_type;
                ach_conf->comm_opt.channel_type.mask = 0xFFFF;
            }
            if (tod_req->proto_id_spec) {
                ach_conf->comm_opt.proto_id.value = tod_req->proto_id;
                ach_conf->comm_opt.proto_id.mask = 0xFFFF;
            }

            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            CPRINTF("ACH conf: ver = %d, chan_type = %d, proto_id = %d\n",
                    ach_conf->comm_opt.version.value,
                    ach_conf->comm_opt.channel_type.value,
                    ach_conf->comm_opt.proto_id.value);
        }
    }

    VTSS_FREE(flow_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_action_add(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    action_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp == TRUE && tod_req->ptp_spec == TRUE) {
            if (tod_req->action_id < 2) {
                ptp_action = &action_conf->action.ptp_conf[tod_req->action_id];
                ptp_action->enable = TRUE;
                ptp_action->channel_map = tod_req->channel_map;
                ptp_action->clk_mode = tod_req->clk_mode;
                ptp_action->delaym_type = tod_req->delaym;
                ptp_action->ptp_conf.range_en = TRUE;
                ptp_action->ptp_conf.domain.range.lower = tod_req->domain_meg_lower;
                ptp_action->ptp_conf.domain.range.upper = tod_req->domain_meg_upper;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else if (action_conf->action_ptp == FALSE && tod_req->y1731_oam_spec == TRUE) {
            if (tod_req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[tod_req->action_id];
                oam_action->enable = TRUE;
                oam_action->channel_map = tod_req->channel_map;
                oam_action->version     = tod_req->version;
                oam_action->y1731_en    = TRUE;
                oam_action->oam_conf.y1731_oam_conf.delaym_type = tod_req->delaym;
                oam_action->oam_conf.y1731_oam_conf.range_en = TRUE;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower = tod_req->domain_meg_lower;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper = tod_req->domain_meg_upper;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else if (action_conf->action_ptp == FALSE && tod_req->ietf_oam_spec == TRUE) {
            if (tod_req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[tod_req->action_id];
                oam_action->enable = TRUE;
                oam_action->channel_map = tod_req->channel_map;
                oam_action->version     = tod_req->version;
                oam_action->y1731_en    = FALSE;
                oam_action->oam_conf.ietf_oam_conf.delaym_type = tod_req->delaym;
                oam_action->oam_conf.ietf_oam_conf.ts_format = tod_req->ietf_tf;
                oam_action->oam_conf.ietf_oam_conf.ds = tod_req->ietf_ds;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else {
            rc = VTSS_RC_ERROR;
        }

        if (rc == VTSS_RC_OK) {
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            }
        }
    }

    VTSS_FREE(action_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_action_delete(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    action_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp) {
            if (tod_req->action_id < 2) {
                ptp_action = &action_conf->action.ptp_conf[tod_req->action_id];
                ptp_action->enable = FALSE;
                ptp_action->channel_map = 0;
                ptp_action->clk_mode = 0;
                ptp_action->delaym_type = 0;
                ptp_action->ptp_conf.range_en = FALSE;
                ptp_action->ptp_conf.domain.range.lower = 0;
                ptp_action->ptp_conf.domain.range.upper = 0;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else {
            if (tod_req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[tod_req->action_id];
                oam_action->enable = FALSE;
                oam_action->channel_map = 0;
                oam_action->oam_conf.y1731_oam_conf.delaym_type = 0;
                oam_action->oam_conf.y1731_oam_conf.range_en = FALSE;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower = 0;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper = 0;
            } else {
                rc = VTSS_RC_ERROR;
            }
        }

        if (rc == VTSS_RC_OK) {
            if (tod_req->ingress) {
                rc = vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            }
        }
    }

    VTSS_FREE(action_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

static void cli_cmd_debug_phy_1588_engine_action_show(cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    vtss_port_no_t port_no;
    BOOL dir = (tod_req->ingress ? 0 : 1);
    u8 eng_id = tod_req->eng_id;
    vtss_rc rc = VTSS_RC_OK;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;
    int i;


    port_no = uport2iport(tod_req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        CPRINTF("Engine not initialized!\n");
        return;
    }

    action_conf = VTSS_MALLOC(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        CPRINTF("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (tod_req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(API_INST_DEFAULT, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp) {
            CPRINTF("***PTP action***\n");
            for (i = 0; i < 2; i++) {
                ptp_action = &action_conf->action.ptp_conf[i];
                CPRINTF("id = %d, enable = %d, channel_map = %d, clk_mode = %d, delaym = %d\n",
                         i, ptp_action->enable, ptp_action->channel_map,
                         ptp_action->clk_mode, ptp_action->delaym_type);
                CPRINTF("domain range_en = %d, domain range lower = %d, upper = %d\n\n",
                        ptp_action->ptp_conf.range_en,
                        ptp_action->ptp_conf.domain.range.lower,
                        ptp_action->ptp_conf.domain.range.upper);
            }
        } else {
            CPRINTF("***OAM action***\n");
            for (i = 0; i < 6; i++) {
                oam_action = &action_conf->action.oam_conf[i];
                CPRINTF("id = %d, enable = %d, channel_map = %d, delaym = %d\n",
                        i, oam_action->enable,
                        oam_action->channel_map,
                        oam_action->oam_conf.y1731_oam_conf.delaym_type);
                CPRINTF("MEG range_en = %d, MEG range lower = %d, upper = %d\n\n",
                        oam_action->oam_conf.y1731_oam_conf.range_en,
                        oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower,
                        oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper);
            }
        }
    }

    VTSS_FREE(action_conf);
    if (rc == VTSS_RC_OK) {
        CPRINTF("Success\n");
     } else {
        CPRINTF("Failed\n");
     }
}

#define MISC_CLI_PHY_TS_SIG_MSG_TYPE_LEN       1
#define MISC_CLI_PHY_TS_SIG_DOMAIN_NUM_LEN     1
#define MISC_CLI_PHY_TS_SIG_SOURCE_PORT_ID_LEN 10
#define MISC_CLI_PHY_TS_SIG_SEQUENCE_ID_LEN    2
#define MISC_CLI_PHY_TS_SIG_DEST_IP_LEN        4
#define MISC_CLI_PHY_TS_SIG_SRC_IP_LEN         4
#define MISC_CLI_PHY_TS_SIG_DEST_MAC_LEN       6

static void cli_cmd_debug_phy_1588_signature_conf(cli_req_t *req)
{
    u16 sig_mask = 0, len = 0;
    vtss_port_no_t port_no;
    tod_cli_req_t *tod_req = req->module_req;

    port_no = uport2iport(tod_req->port_no);
    sig_mask = tod_req->sig_mask;
    if (req->set) {

        if ( tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) {
            len += MISC_CLI_PHY_TS_SIG_MSG_TYPE_LEN;
            CPRINTF("Msg Type |");
        }
        if ( tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) {
            len += MISC_CLI_PHY_TS_SIG_DOMAIN_NUM_LEN;
            CPRINTF("Domain Num |");
        }
        if (tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) {
            len += MISC_CLI_PHY_TS_SIG_SOURCE_PORT_ID_LEN;
            CPRINTF("Source Port ID |");
        }
        if (tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) {
            len += MISC_CLI_PHY_TS_SIG_SEQUENCE_ID_LEN ;
            CPRINTF("Sequence ID |");
        }
        if ( tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
            len += MISC_CLI_PHY_TS_SIG_DEST_IP_LEN;
            CPRINTF("Dest IP |");
        }
        if (tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
            len += MISC_CLI_PHY_TS_SIG_SRC_IP_LEN;
            CPRINTF("Src IP |");
        }
        if (tod_req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
            len += MISC_CLI_PHY_TS_SIG_DEST_MAC_LEN;
            CPRINTF("Dest MAC |");
        }
        CPRINTF("\nLength : %d \n",len);
        if (vtss_phy_ts_fifo_sig_set(API_INST_DEFAULT, port_no, tod_req->sig_mask) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_ts_fifo_sig_set() operation \n");
            return;
        }
    } else {
        if (vtss_phy_ts_fifo_sig_get(API_INST_DEFAULT, port_no, (vtss_phy_ts_fifo_sig_mask_t  *)&sig_mask) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_ts_fifo_sig_get() operation \n");
            return;
        }
        CPRINTF("Port : %d Signature : %x \n", port_no, sig_mask);
    }
}

static void cli_cmd_debug_phy_1588_ts_stats_show(cli_req_t *req)
{
    u16 i =0, j=0;
    vtss_port_no_t port_no,cfg_port;
    vtss_port_no_t ts_port_no[6] = {0};
    tod_cli_req_t *tod_req = req->module_req;

    vtss_phy_ts_stats_t ts_stats[6];
    u16   num_ports=6, num_sec=5;
    memset(ts_stats, 0, sizeof(vtss_phy_ts_stats_t)*6);
    port_no = uport2iport(tod_req->port_no);
    if (tod_req->time_sec){
        num_sec = tod_req->time_sec;
    }
    
    for (j = 0; j<6; j++) {
        ts_port_no[j] = 0;
    }
    for (j=0;j<num_sec; j++) {
        for (port_no = 21, i=0; port_no <= 26; port_no++) {
            cfg_port = uport2iport(port_no);
            if ( vtss_phy_ts_stats_get(API_INST_DEFAULT, cfg_port, &ts_stats[i]) == VTSS_RC_OK) {
                 ts_port_no[i++] = port_no;
            }
        }
        num_ports = i;
        for (i=0;i<num_ports; i++) {
             CPRINTF("Port : %u\n", ts_port_no[i]);
             CPRINTF("Frames with preambles too short to shrink  Ingress        : %u\n",ts_stats[i].ingr_pream_shrink_err);
             CPRINTF("Frames with preambles too short to shrink  Egress         : %u\n",ts_stats[i].egr_pream_shrink_err );
             CPRINTF("Timestamp block received frame with FCS error in ingress  : %u\n",ts_stats[i].ingr_fcs_err );
             CPRINTF("Timestamp block received frame with FCS error in egress   : %u\n",ts_stats[i].egr_fcs_err );
             CPRINTF("No of frames modified by timestamp block in ingress       : %u\n",ts_stats[i].ingr_frm_mod_cnt );
             CPRINTF("No of frames modified by timestamp block in egress        : %u\n",ts_stats[i].egr_frm_mod_cnt );
             CPRINTF("The number of timestamps transmitted to the SPI interface : %u\n",ts_stats[i].ts_fifo_tx_cnt );
             CPRINTF("Count of dropped Timestamps not enqueued to the Tx TSFIFO : %u\n",ts_stats[i].ts_fifo_drop_cnt );
        }
        VTSS_OS_MSLEEP(1000);
    }
}
static void cli_cmd_debug_phy_1588_latency(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    BOOL                       error = FALSE;
    vtss_timeinterval_t        latency_val;
    u32                        temp;

    port_no = uport2iport(tod_req->port_no);
    if (req->set) {
        if (tod_req->ingress) {
            if(vtss_phy_ts_ingress_latency_set(API_INST_DEFAULT, port_no, &(tod_req->latency_val)) != VTSS_RC_OK) {
                error = TRUE;
            }        
        } else {
            if (vtss_phy_ts_egress_latency_set(API_INST_DEFAULT, port_no, &(tod_req->latency_val)) != VTSS_RC_OK) {
                error = TRUE;
            }
        }
        if (!error) {
            CPRINTF("Set the Latency Successfully\n");
        } else {
            CPRINTF("Failed...! Set Operation is not processed\n");
        }
    } else {
        if(tod_req->ingress) {
            if(vtss_phy_ts_ingress_latency_get(API_INST_DEFAULT, port_no, &latency_val) != VTSS_RC_OK) {
                error = TRUE;
                CPRINTF("Failed...! Get Operation is not processed\n");
            }
        } else {
            if(vtss_phy_ts_egress_latency_get(API_INST_DEFAULT, port_no, &latency_val) != VTSS_RC_OK) {
                error = TRUE;
                CPRINTF("Failed...! Get Operation is not processed\n");
            }
        }
        if(!error) {
            latency_val = latency_val >> 16;
            temp = (u32) latency_val;
            CPRINTF("Port  Ingress/Egress  Latency\n----  --------------  -------\n");
            CPRINTF("%-4d  %-14s  %u\n", tod_req->port_no, tod_req->ingress ? "Ingress" : "Egress", temp);
        }
    }
}

static void cli_cmd_debug_phy_1588_delay(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    vtss_timeinterval_t        delay_val;
    u32                        temp;

    port_no = uport2iport(tod_req->port_no);
    if(req->set) {
         if(vtss_phy_ts_path_delay_set(API_INST_DEFAULT, port_no, &(tod_req->delay_val)) != VTSS_RC_OK)  {
             CPRINTF("Failed to set the port delay for the port %d\n", tod_req->port_no);
         } else {
             CPRINTF("Successfully set the port delay..\n");
         }
     } else {
         if(vtss_phy_ts_path_delay_get(API_INST_DEFAULT, port_no, &delay_val) != VTSS_RC_OK)  {
             CPRINTF("Failed to get the port delay for the port %d\n", tod_req->port_no);
         } else {
             temp = (u32)(delay_val >> 16);
             CPRINTF("Port  PathDelay\n----  ---------\n");
             CPRINTF("%-4d  %-u\n", tod_req->port_no, temp);
         }
     }
}
static void cli_cmd_debug_phy_1588_delay_asym(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    i32                        temp;
    vtss_timeinterval_t        asym_val;

    port_no = uport2iport(tod_req->port_no);
    if(req->set) {
        if(vtss_phy_ts_delay_asymmetry_set(API_INST_DEFAULT, port_no, &(tod_req->asym_val)) != VTSS_RC_OK) {
            CPRINTF("Failed to set the Delay Asymmetry for the port %d\n", tod_req->port_no);
        } else {
            CPRINTF("Successfully set the delay asymmetry...\n");
        }
    } else {
        if(vtss_phy_ts_delay_asymmetry_get(API_INST_DEFAULT, port_no, &asym_val) != VTSS_RC_OK) {
            CPRINTF("Failed to get the asymmetry delay for the port %d\n", tod_req->port_no);
        } else {
            asym_val = asym_val >> 16;
            temp = (i32) asym_val;
            CPRINTF("Port  DelayAsym\n----  ---------\n");
            CPRINTF("%-4d  %d\n", tod_req->port_no, temp);
        }
    }
}

static void cli_cmd_debug_phy_1588_nphase(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req;
    vtss_phy_ts_nphase_status_t status;
    char sampler_str[5][10]={"PPS_O","PPS_RI","EGR_SOF","ING_SOF","LS"};
    u8 icount=0;
    if(req != NULL) {
        tod_req = (tod_cli_req_t *)req->module_req;
        port_no = uport2iport(tod_req->port_no);
        CPRINTF("Port  Sampler  %-12s\n","Calibration");
        CPRINTF("%-4s  %-7s  %-5s  %-5s\n","","","Done","Error");
        CPRINTF("%-4s  %-7s  %-5s  %-5s\n","---","-------","-----","-----");
        if(tod_req->nphase_sampler == 0 ) {
            for(icount = VTSS_PHY_TS_NPHASE_PPS_O;icount<VTSS_PHY_TS_NPHASE_MAX;icount++) {
                if(vtss_phy_ts_nphase_status_get(API_INST_DEFAULT,port_no,icount,&status)==VTSS_RC_OK){
                    CPRINTF("%-4u  %-7s  %-5s  %-5s\n",tod_req->port_no,sampler_str[icount],status.CALIB_DONE ? "YES" : "FALSE",status.CALIB_ERR ? "YES" : "FALSE");
                }
            }
        }else {

            if(vtss_phy_ts_nphase_status_get(API_INST_DEFAULT,port_no,(tod_req->nphase_sampler) - 1,&status)==VTSS_RC_OK){
                CPRINTF("%-4u  %-7s  %-5s  %-5s\n",tod_req->port_no,sampler_str[(tod_req->nphase_sampler) - 1],status.CALIB_DONE ? "YES" : "FALSE",status.CALIB_ERR ? "YES" : "FALSE");
            }
        }

    }
}

#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
static void cli_cmd_debug_phy_1588_ing_delay(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    i32                        temp;
    vtss_timeinterval_t        ing_delay_val;

    port_no = uport2iport(tod_req->port_no);
    if(req->set) {
        if(vtss_phy_ts_ingress_delay_comp_set(API_INST_DEFAULT, port_no, &(tod_req->ing_delay_val)) != VTSS_RC_OK) {
            CPRINTF("Failed to set the Ingress Delay for the port %d\n", tod_req->port_no);
        } else {
            CPRINTF("Successfully set the ingress delay ...\n");
        }
    } else {
        if(vtss_phy_ts_delay_asymmetry_get(API_INST_DEFAULT, port_no, &ing_delay_val) != VTSS_RC_OK) {
            CPRINTF("Failed to get the ingress delay for the port %d\n", tod_req->port_no);
        } else {
            ing_delay_val = ing_delay_val >> 16;
            temp = (i32) ing_delay_val;
            CPRINTF("Port  IngDelay\n----  ---------\n");
            CPRINTF("%-4d  %d\n", tod_req->port_no, temp);
        }
    }
}
static void cli_cmd_debug_phy_1588_egr_delay(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    i32                        temp;
    vtss_timeinterval_t        egr_delay_val;

    port_no = uport2iport(tod_req->port_no);
    if(req->set) {
        if(vtss_phy_ts_egress_delay_comp_set(API_INST_DEFAULT, port_no, &(tod_req->egr_delay_val)) != VTSS_RC_OK) {
            CPRINTF("Failed to set the Ingress Delay for the port %d\n", tod_req->port_no);
        } else {
            CPRINTF("Successfully set the egress delay ...\n");
        }
    } else {
        if(vtss_phy_ts_delay_asymmetry_get(API_INST_DEFAULT, port_no, &egr_delay_val) != VTSS_RC_OK) {
            CPRINTF("Failed to get the egress delay for the port %d\n", tod_req->port_no);
        } else {
            egr_delay_val = egr_delay_val >> 16;
            temp = (i32) egr_delay_val;
            CPRINTF("Port  EgrDelay\n----  ---------\n");
            CPRINTF("%-4d  %d\n", tod_req->port_no, temp);
        }
    }
}


#endif

static char *tc_int_mode_disp(vtss_tod_internal_tc_mode_t m)
{
    switch (m) {
        case VTSS_TOD_INTERNAL_TC_MODE_30BIT: return "MODE_30BIT";
        case VTSS_TOD_INTERNAL_TC_MODE_32BIT: return "MODE_32BIT";
        case VTSS_TOD_INTERNAL_TC_MODE_44BIT: return "MODE_44BIT";
        case VTSS_TOD_INTERNAL_TC_MODE_48BIT: return "MODE_48BIT";
        default: return "unknown";
    }
}

static void cli_cmd_debug_tod_tc_internal_mode(cli_req_t *req)
{
    tod_cli_req_t              *tod_req = req->module_req;

    if(req->set) {
        if(!tod_tc_mode_set(&tod_req->tc_int_mode)) {
            CPRINTF("Failed to set the TC internal mode to %d\n", tod_req->tc_int_mode);
        } else {
            CPRINTF("Successfully set the TC internal mode...\n");
			CPRINTF("Internal TC mode Configuration has been set, you need to reboot to activate the changed conf.\n");
        }
    } else {
        if(!tod_tc_mode_get(&tod_req->tc_int_mode)) {
            CPRINTF("Failed to get the TC internal mode to %s\n", tc_int_mode_disp(tod_req->tc_int_mode));
        } else {
            CPRINTF("TC internal mode %s\n", tc_int_mode_disp(tod_req->tc_int_mode));
        }
    }
}

static void cli_cmd_debug_phy_ts_enable(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;

    port_no = uport2iport(tod_req->port_no);
    if(req->set) {
        if(!tod_port_phy_ts_set(&tod_req->enable, port_no)) {
            CPRINTF("Failed to set the phy ts mode for port %d\n", tod_req->port_no);
        } else {
            CPRINTF("Successfully set the phy ts mode...\n");
			CPRINTF("PHY timestamp mode Configuration has been set, you need to reboot to activate the changed conf.\n");
        }
    } else {
        if(!tod_port_phy_ts_get(&tod_req->enable, port_no)) {
            CPRINTF("Failed to get the the phy ts mode for port %d\n", tod_req->port_no);
        } else {
            CPRINTF("Phy ts mode port %d = %s\n", tod_req->port_no, tod_req->enable ? "enable" : "disable");
        }
    }
}

static void cli_cmd_debug_phy_ptptime(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    tod_cli_req_t              *tod_req = req->module_req;
    vtss_phy_timestamp_t       ts;

    port_no = uport2iport(tod_req->port_no);
    memset(&ts, 0, sizeof(vtss_phy_timestamp_t));
    if (req->set) {
        ts.seconds.high = 0;
        ts.seconds.low = tod_req->time_sec;
        ts.nanoseconds = 0;
        printf("Setting the LTC\n");
        if (vtss_phy_ts_ptptime_set(API_INST_DEFAULT, port_no, &ts) != VTSS_RC_OK) {
            printf("Error..! vtss_phy_ts_ptptime_set\n");
        }
        cyg_thread_delay(100);
        if (vtss_phy_ts_ptptime_set_done(API_INST_DEFAULT, port_no) != VTSS_RC_OK) {
            printf("Error..! vtss_phy_ts_ptptime_set_done\n");
        }
    } else {
        if (vtss_phy_ts_ptptime_arm(API_INST_DEFAULT, port_no) != VTSS_RC_ERROR) {
            printf("Error..! vtss_phy_ts_ptptime_arm\n");
        }
        cyg_thread_delay(100);
        if (vtss_phy_ts_ptptime_get(API_INST_DEFAULT, port_no, &ts) != VTSS_RC_ERROR)
            printf("Error..! vtss_phy_ts_ptptime_get\n");
        printf("PTP Time Get::\n");
        printf("    Seconds High::%u\n", ts.seconds.high);
        printf("    Seconds Low::%u\n", ts.seconds.low);
        printf("    Nano Seconds:: %u\n", ts.nanoseconds);
    }
}


static void cli_cmd_phy_1588_block_init(cli_req_t *req)
{
    vtss_port_no_t port_no;
    static BOOL ts_init_info[4] = {FALSE, FALSE, FALSE, FALSE};
    vtss_rc rc;
    tod_cli_req_t *const tod_req = req->module_req;
#ifndef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    vtss_phy_ts_init_conf_t tod_phy_init = {VTSS_PHY_TS_CLOCK_FREQ_250M, VTSS_PHY_TS_CLOCK_SRC_LINE_TX, VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP, VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT, VTSS_PHY_TS_FIFO_MODE_SPI, VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_10BYTE, 0};

    if(tod_req->clk_freq_spec)
        tod_phy_init.clk_freq = tod_req->clkfreq;
    if(tod_req->clk_src_spec)
        tod_phy_init.clk_src = tod_req->clk_src;
    if(tod_req->rx_ts_pos_spec)
        tod_phy_init.rx_ts_pos = tod_req->rx_ts_pos;
    if(tod_req->tx_fifo_mode_spec)
        tod_phy_init.tx_fifo_mode = tod_req->tx_fifo_mode;
    if(tod_req->modify_frame_spec)
        tod_phy_init.chk_ing_modified = tod_req->modify_frame;
#endif
    port_no = uport2iport(tod_req->port_no);
    if (ts_init_info[port_no] == FALSE) {
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
        rc = vtss_phy_dce_init(API_INST_DEFAULT, port_no);
#else
        rc = vtss_phy_ts_init(API_INST_DEFAULT, port_no, &tod_phy_init);
#endif
        if (rc == VTSS_RC_OK) {
            ts_init_info[port_no] = TRUE;
            printf("PHY TS Init Success\n");
        } else {
            printf("PHY TS Init Failed!\n");
            return;
        }
        rc = vtss_phy_ts_mode_set(API_INST_DEFAULT, port_no, TRUE);
        if (rc != VTSS_RC_OK) {
            printf("PHY TS Block Enable Failed\n");
        }
    }
}

static void cli_cmd_ext_pps_out(cli_req_t *req)
{
    vtss_ts_ext_clock_mode_t clock_mode;

        /* update the local clock to the system clock */
       clock_mode.one_pps_mode = TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT;
       clock_mode.enable = TRUE;
       clock_mode.freq = 1;
       (void)vtss_ts_external_clock_mode_set(NULL, &clock_mode);
}

static const char* engine_name(vtss_phy_ts_engine_t eng_id) 
{
    switch (eng_id) {
        case    VTSS_PHY_TS_PTP_ENGINE_ID_0:  return " ID_0";
        case    VTSS_PHY_TS_PTP_ENGINE_ID_1:  return " ID_01";
        case    VTSS_PHY_TS_OAM_ENGINE_ID_2A: return " ID_2A";
        case    VTSS_PHY_TS_OAM_ENGINE_ID_2B: return " ID_2B";
        default                             : return "INVALID";
    }
}
        
static void cli_cmd_phy_eng_alloc(cli_req_t *req)
{
    vtss_phy_ts_engine_t eng_id;
    BOOL engine_list[4];
    vtss_port_no_t     port_no;
    tod_cli_req_t *const tod_req = req->module_req;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            eng_id = tod_phy_eng_alloc(port_no, tod_req->encap_mode);
            if (eng_id <= VTSS_PHY_TS_ENGINE_ID_INVALID) {
                CPRINTF("Allocated engine %s for port %d\n", engine_name(eng_id), iport2uport(port_no));
            }
        } else {
            tod_phy_eng_alloc_get(port_no, engine_list);
            CPRINTF("Port %d Engines allocated: %s%s%s%s\n", iport2uport(port_no), 
                    engine_list[0] ? engine_name(VTSS_PHY_TS_PTP_ENGINE_ID_0) : "", engine_list[1] ? engine_name(VTSS_PHY_TS_PTP_ENGINE_ID_1) : "", 
                    engine_list[2] ? engine_name(VTSS_PHY_TS_OAM_ENGINE_ID_2A) : "", engine_list[3] ? engine_name(VTSS_PHY_TS_OAM_ENGINE_ID_2B) : "" );
        }
    }
}

static void cli_cmd_phy_eng_free(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    tod_cli_req_t *const tod_req = req->module_req;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            if (tod_req->eng_id <= VTSS_PHY_TS_ENGINE_ID_INVALID) {
                tod_phy_eng_free(port_no, tod_req->eng_id);
                CPRINTF("Free'ed engine %s for port %d\n", engine_name(tod_req->eng_id), iport2uport(port_no));
            }
        } else {
            CPRINTF("nothing to free\n");
        }
    }
}

static void cli_cmd_phy_monitor(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    tod_cli_req_t *const tod_req = req->module_req;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            if (VTSS_RC_OK == ptp_module_man_time_slave_timecounter_enable_disable(port_no, tod_req->enable)) {
                CPRINTF("%s monitoring of port %d\n", tod_req->enable ? "Enable" : "Disable", iport2uport(port_no));
            } else {
                CPRINTF("Enable/disable failed\n");
            }
        } else {
            CPRINTF("nothing to change\n");
        }
    }
}

static int32_t cli_parm_1588_api_latency_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
   int                   error;
   u32                   val;
   tod_cli_req_t         *tod_req = req->module_req;
  
   error = cli_parse_ulong(cmd, &val, 0, 65536);
   if (!error) {
       tod_req->latency_val = (vtss_timeinterval_t)val;
       tod_req->latency_val = tod_req->latency_val << 16;
   }
   return error;
}

static int32_t cli_parm_1588_api_delay_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
   int                   error;
   ulong                 val;
   tod_cli_req_t         *tod_req = req->module_req;

   error = cli_parse_ulong(cmd, &val, 0, 4294967295u);
   if (!error) {
       tod_req->delay_val = (vtss_timeinterval_t)val;
       tod_req->delay_val = tod_req->delay_val << 16;
   }
   return error;
}
static int32_t cli_parm_1588_api_delay_asym_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int                   error;
    long                   val;
    tod_cli_req_t         *tod_req = req->module_req;

    error = cli_parse_long(cmd, &val, -32768, 32767);
    if (!error) {
        tod_req->asym_val = (vtss_timeinterval_t)val;
        tod_req->asym_val = tod_req->asym_val << 16;
    }
    return error;
}

static int32_t cli_parm_1588_api_nphase_sampler_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int error=0;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 4);
    if(!error) {
        tod_req->nphase_sampler = (u8)value + 1;
    }
    return error;
}


#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
static int32_t cli_parm_1588_api_ing_delay_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int                   error;
    long                   val;
    tod_cli_req_t         *tod_req = req->module_req;

    error = cli_parse_long(cmd, &val, -32768, 32767);
    if (!error) {
        tod_req->ing_delay_val = (vtss_timeinterval_t)val;
        tod_req->ing_delay_val = tod_req->ing_delay_val << 16;
    }
    return error;
}

static int32_t cli_parm_1588_api_egr_delay_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int                   error;
    long                   val;
    tod_cli_req_t         *tod_req = req->module_req;

    error = cli_parse_long(cmd, &val, -32768, 32767);
    if (!error) {
        tod_req->egr_delay_val = (vtss_timeinterval_t)val;
        tod_req->egr_delay_val = tod_req->egr_delay_val << 16;
    }
    return error;
}
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */


static int32_t cli_parm_tc_internal_mode_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int                   error;
    long                   val;
    tod_cli_req_t         *tod_req = req->module_req;

    error = cli_parse_long(cmd, &val, 0, VTSS_TOD_INTERNAL_TC_MODE_MAX);
    if (!error) {
        tod_req->tc_int_mode = (vtss_tod_internal_tc_mode_t) val;
    }
    return error;
}



#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
static void cli_cmd_debug_phy_1588_new_spi_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    tod_cli_req_t *tod_req = req->module_req;

    port_no = uport2iport(tod_req->port_no);
    if (req->set) {
        if (vtss_phy_ts_new_spi_mode_set(API_INST_DEFAULT, port_no, (tod_req->enable ? TRUE : FALSE)) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_ts_spi_mode_set() operation \n");
            return;
        }
    } else {
        CPRINTF("Port : %d New SPI mode : to be retrieved!!!\n", port_no);
    }

    CPRINTF("Success \n");
}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */


static int32_t cli_parm_1588_api_port_no_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, port_isid_port_count(VTSS_ISID_LOCAL));
    if (!error) {
       tod_req->port_no = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_eng_id_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 3);
    if (!error) {
       tod_req->eng_id = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_channel_map_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0x1, 0x3);
    if (!error) {
       tod_req->channel_map = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_flow_id_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    if (!error) {
       tod_req->flow_id_spec = TRUE;
       tod_req->flow_id = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_encap_type_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, VTSS_PHY_TS_ENCAP_ETH_PTP, VTSS_PHY_TS_ENCAP_ANY);
    if (!error) {
       tod_req->encap_type = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_flow_st_index_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    if (!error) {
       tod_req->flow_st_index = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_flow_end_index_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    if (!error) {
       tod_req->flow_end_index = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_ip_flow_keyword(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ipv4", 4)) {
            tod_req->ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (!strncmp(found, "ipv6", 4)) {
            tod_req->ip_mode = VTSS_PHY_TS_IP_VER_6;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_parm_1588_api_parse_keyword(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ingress", 7)) {
            tod_req->ingress = TRUE;
        } else if (!strncmp(found, "egress", 6)) {
            tod_req->ingress = FALSE;
        } else if (!strncmp(found, "disable", 7)) {
            tod_req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            tod_req->enable = 1;
        } else if (!strncmp(found, "strict", 6)) {
            tod_req->flow_match_mode = VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT;
        } else if (!strncmp(found, "non-strict", 10)) {
            tod_req->flow_match_mode = VTSS_PHY_TS_ENG_FLOW_MATCH_ANY;
        } else if (!strncmp(found, "pbb_en", 6)) {
            tod_req->pbb_spec = TRUE;
            tod_req->pbb_en = TRUE;
        } else if (!strncmp(found, "pbb_dis", 7)) {
            tod_req->pbb_spec = TRUE;
            tod_req->pbb_en = FALSE;
        } else if (!strncmp(found, "match_full", 10)) {
            tod_req->mac_match_mode_spec = TRUE;
            tod_req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
        } else if (!strncmp(found, "any_uc", 6)) {
            tod_req->mac_match_mode_spec = TRUE;
            tod_req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST;
        } else if (!strncmp(found, "any_mc", 6)) {
            tod_req->mac_match_mode_spec = TRUE;
            tod_req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST;
        } else if (!strncmp(found, "vlan_chk_en", 11)) {
            tod_req->vlan_chk_spec = TRUE;
            tod_req->vlan_chk = TRUE;
        } else if (!strncmp(found, "vlan_chk_dis", 12)) {
            tod_req->vlan_chk_spec = TRUE;
            tod_req->vlan_chk = FALSE;
        } else if (!strncmp(found, "tag_rng_none", 12)) {
            tod_req->tag_rng_mode_spec = TRUE;
            tod_req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_NONE;
        } else if (!strncmp(found, "tag_rng_outer", 13)) {
            tod_req->tag_rng_mode_spec = TRUE;
            tod_req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_OUTER;
        } else if (!strncmp(found, "tag_rng_inner", 13)) {
            tod_req->tag_rng_mode_spec = TRUE;
            tod_req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_INNER;
        } else if (!strncmp(found, "ipv4", 4)) {
            tod_req->ip_mode_spec = TRUE;
            tod_req->ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (!strncmp(found, "ipv6", 4)) {
            tod_req->ip_mode_spec = TRUE;
            tod_req->ip_mode = VTSS_PHY_TS_IP_VER_6;
        } else if (!strncmp(found, "match_src", 9)) {
            tod_req->addr_match_select_spec = TRUE;
            tod_req->addr_match_select = 0;
        } else if (!strncmp(found, "match_dest", 10)) {
            tod_req->addr_match_select_spec = TRUE;
            tod_req->addr_match_select = 1;
        } else if (!strncmp(found, "match_src_dest", 14)) {
            tod_req->addr_match_select_spec = TRUE;
            tod_req->addr_match_select = 2;
        } else if (!strncmp(found, "stk_ref_top", 11)) {
            tod_req->stk_ref_point_spec = TRUE;
            tod_req->stk_ref_point = VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP;
        } else if (!strncmp(found, "stk_ref_bottom", 14)) {
            tod_req->stk_ref_point_spec = TRUE;
            tod_req->stk_ref_point = VTSS_PHY_TS_MPLS_STACK_REF_POINT_END;
        } else if (!strncmp(found, "ptp", 3)) {
            tod_req->ptp_spec = TRUE;
        } else if (!strncmp(found, "y1731_oam", 9)) {
            tod_req->y1731_oam_spec = TRUE;
        } else if (!strncmp(found, "ietf_oam", 8)) {
            tod_req->ietf_oam_spec = TRUE;
        }

    }

    return (found == NULL ? 1 : 0);

}

static int32_t cli_parm_1588_api_tag1_type_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 4);
    if (!error) {
        tod_req->tag1_type = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tag2_type_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 4);
    if (!error) {
        tod_req->tag2_type = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tag1_lower_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 4094);
    if (!error) {
        tod_req->tag1_lower = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tag1_upper_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
        tod_req->tag1_upper = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tag2_lower_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 4094);
    if (!error) {
        tod_req->tag2_lower = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tag2_upper_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
        tod_req->tag2_upper = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_etype_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0x600, 0xFFFF);
    if (!error) {
        tod_req->etype = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tpid_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0x1, 0xFFFF);
    if (!error) {
        tod_req->tpid = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_mac_addr_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    tod_cli_req_t *tod_req = req->module_req;
    cli_spec_t spec;

    error = cli_parse_mac(cmd, tod_req->ptp_mac, &spec, 0);
    if (!error) tod_req->ptp_mac_spec = TRUE;

    return error;
}

static int32_t cli_parm_1588_api_vlan_num_tag_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 2);
    if (!error) {
       tod_req->num_tag = (u8)value;
       tod_req->num_tag_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_sport_val_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    if (!error) {
       tod_req->sport_val = (u16)value;
       tod_req->sport_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_sport_mask_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
       tod_req->sport_mask = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_dport_val_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    if (!error) {
       tod_req->dport_val = (u16)value;
       tod_req->dport_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_dport_mask_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
       tod_req->dport_mask = (u16)value;
    }
    return error;
}

/* Validate and convert IPv4 address string a.b.c.d
 * a legal unicast/multicast address or 0.0.0.0 is accepted.
 */
static int txt2_ipv4(char *buf, ulong *ip)
{
    uint  a, b, c, d;
    ulong data;
    int   items;

    items = sscanf(buf, "%u.%u.%u.%u", &a, &b, &c, &d);
    if ((items != 4) || (a > 255) || (b > 255) || (c > 255) || (d > 255)) {
        /* ERROR: Invalid IP address format */
        return VTSS_UNSPECIFIED_ERROR;
    }

    data = ((a<<24) + (b<<16) + (c<<8) + d);
    /* Check that it is 0.0.0.0 or a legal address */
    if ((a == 0) && (data != 0)) {
        /* ERROR: Not a legal address or 0.0.0.0 */
        return 1;
    }

    *ip = data;
    return 0;
}

static int32_t cli_parm_1588_api_ipv4_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error;
    tod_cli_req_t *tod_req = req->module_req;
    error = txt2_ipv4(cmd, &tod_req->ipv4_addr);
    if (!error) {
        tod_req->ipv4_addr_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_ipv4_mask_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error;
    tod_cli_req_t *tod_req = req->module_req;

    error = txt2_ipv4(cmd, &tod_req->ipv4_mask);
    if (!error) {
        tod_req->ipv4_mask_spec = TRUE;
    }
    return error;
}


static int32_t cli_parm_1588_api_ipv6_addr_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    vtss_ipv6_t addr;
    tod_cli_req_t *tod_req = req->module_req;
    error = mgmt_txt2ipv6(cmd, &addr);
    if (!error) {
        tod_req->ipv6_addr[3] = addr.addr[ 0] << 24 | addr.addr[ 1] << 16 | addr.addr[ 2] << 8 | addr.addr[ 3];
        tod_req->ipv6_addr[2] = addr.addr[ 4] << 24 | addr.addr[ 5] << 16 | addr.addr[ 6] << 8 | addr.addr[ 7];
        tod_req->ipv6_addr[1] = addr.addr[ 8] << 24 | addr.addr[ 9] << 16 | addr.addr[10] << 8 | addr.addr[11];
        tod_req->ipv6_addr[0] = addr.addr[12] << 24 | addr.addr[13] << 16 | addr.addr[14] << 8 | addr.addr[15];
        tod_req->ipv4_addr_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_ipv6_mask_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    vtss_ipv6_t addr;

    tod_cli_req_t *tod_req = req->module_req;
    error = mgmt_txt2ipv6(cmd, &addr);
    if (!error) {
        tod_req->ipv6_mask[3] = addr.addr[ 0] << 24 | addr.addr[ 1] << 16 | addr.addr[ 2] << 8 | addr.addr[ 3];
        tod_req->ipv6_mask[2] = addr.addr[ 4] << 24 | addr.addr[ 5] << 16 | addr.addr[ 6] << 8 | addr.addr[ 7];
        tod_req->ipv6_mask[1] = addr.addr[ 8] << 24 | addr.addr[ 9] << 16 | addr.addr[10] << 8 | addr.addr[11];
        tod_req->ipv6_mask[0] = addr.addr[12] << 24 | addr.addr[13] << 16 | addr.addr[14] << 8 | addr.addr[15];
        tod_req->ipv4_mask_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_1588_api_stk_depth_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 4);
    if (!error) {
        tod_req->stk_depth_spec = TRUE;
        switch(value) {
        case 1:
            tod_req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_1;
            break;
        case 2:
            tod_req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_2;
            break;
        case 3:
            tod_req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_3;
            break;
        case 4:
            tod_req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_4;
            break;
        default:
            break; 
        }
    }
    return error;
}

static int32_t cli_parm_1588_api_stk_lvl_0_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong low, high;
    tod_cli_req_t *tod_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "stk_lvl_0"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        tod_req->stk_lvl_0 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = cli_parse_range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                tod_req->stk_lvl_0_lower = low;
                tod_req->stk_lvl_0_upper = high;
            }
        } else {
                tod_req->stk_lvl_0_lower = low;
                tod_req->stk_lvl_0_upper = low;
        }
    }

    return error;
}

static int32_t cli_parm_1588_api_stk_lvl_1_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong low, high;
    tod_cli_req_t *tod_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "stk_lvl_1"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        tod_req->stk_lvl_1 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = cli_parse_range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                tod_req->stk_lvl_1_lower = low;
                tod_req->stk_lvl_1_upper = high;
            }
        } else {
                tod_req->stk_lvl_1_lower = low;
                tod_req->stk_lvl_1_upper = low;
        }
    }

    return error;
}

static int32_t cli_parm_1588_api_stk_lvl_2_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong low, high;
    tod_cli_req_t *tod_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "stk_lvl_2"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        tod_req->stk_lvl_2 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = cli_parse_range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                tod_req->stk_lvl_2_lower = low;
                tod_req->stk_lvl_2_upper = high;
            }
        } else {
                tod_req->stk_lvl_2_lower = low;
                tod_req->stk_lvl_2_upper = low;
        }
    }

    return error;
}

static int32_t cli_parm_1588_api_stk_lvl_3_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong low, high;
    tod_cli_req_t *tod_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "stk_lvl_3"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        tod_req->stk_lvl_3 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = cli_parse_range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                tod_req->stk_lvl_3_lower = low;
                tod_req->stk_lvl_3_upper = high;
            }
        } else {
                tod_req->stk_lvl_3_lower = low;
                tod_req->stk_lvl_3_upper = low;
        }
    }

    return error;
}

static int32_t cli_parm_1588_api_ach_ver_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFF);
    if (!error) {
        tod_req->ach_ver_spec = TRUE;
        tod_req->ach_ver = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_channel_type_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
        tod_req->channel_type_spec = TRUE;
        tod_req->channel_type = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_proto_id_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    if (!error) {
         tod_req->proto_id_spec = TRUE;
         tod_req->proto_id = (u16)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_action_id_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 5);
    if (!error) {
         tod_req->action_id = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_clock_mode_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 4);
    if (!error) {
         tod_req->clk_mode = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_delaym_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 1);
    if (!error) {
         tod_req->delaym = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_oam_version_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0);
    if (!error) {
         tod_req->version = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_domain_meg_range_lower_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 255);
    if (!error) {
         tod_req->domain_meg_lower = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_domain_meg_range_upper_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 255);
    if (!error) {
         tod_req->domain_meg_upper = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_tf_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 3, 3);
    if (!error) {
         tod_req->ietf_tf = (u8)value;
    }
    return error;
}

static int32_t cli_parm_1588_api_ds_traffic_class_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    if (!error) {
         tod_req->ietf_ds = (u8)value;
    }
    return error;
}


static int32_t cli_parm_1588_api_cw_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    tod_cli_req_t *tod_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "cw_en", 5)) {
            tod_req->cw_en = TRUE;
        } else if (!strncmp(found, "cw_dis", 6)) {
            tod_req->cw_en = FALSE;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_parm_1588_api_signature_mask_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0x7f);
    if (!error) {
         tod_req->sig_mask = (u8)value;
    }
    return error;
}
static int32_t cli_parm_1588_api_time_sec_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 60);
    if (!error) {
         tod_req->time_sec = (u8)value;
    }
    return error;
}

static int32_t cli_parm_parse_rx_ts_pos(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 1);
    if (!error) {
        tod_req->rx_ts_pos = (u8)value;
        tod_req->rx_ts_pos_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_parse_clk_src(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 5);
    if (!error) {
        tod_req->clk_src = (u8)value;
        tod_req->clk_src_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_parse_tx_fifo_mode(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 1);
    if (!error) {
        tod_req->tx_fifo_mode = (u8)value;
        tod_req->tx_fifo_mode_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_parse_clkfrq(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 4);
    if (!error) {
        tod_req->clkfreq= (u8)value;
        tod_req->clk_freq_spec = TRUE;
    }
    return error;
}

static int32_t cli_parm_parse_encap_mode(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int error;
    ulong value;
    tod_cli_req_t *tod_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 12);
    if (!error) {
        tod_req->encap_mode = value;
    }
    return error;
}

static int32_t cli_parm_parse_modify_bit(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)

{
      int error;
      ulong value;
      tod_cli_req_t *tod_req = req->module_req;

      error = cli_parse_ulong(cmd, &value, 0, 1);
      if (!error) {
          tod_req->modify_frame = (u8)value;
          tod_req->modify_frame_spec = TRUE;
      }
      return error;
}

static cli_parm_t tod_cli_parm_table[] = {
    {
        "ingress|egress",
        "ingress or egress engine",
        CLI_PARM_FLAG_NONE,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<port_no>",
        "port number",
        CLI_PARM_FLAG_NONE,
        cli_parm_1588_api_port_no_parse,
        NULL
    },
    {
        "<engine_id>",
        "Engine Identifier (values are 0,1,2 and 3)",
        CLI_PARM_FLAG_NONE,
        cli_parm_1588_api_eng_id_parse,
        NULL
    },
    {
        "<channel_map>",
        "flow associated to channel",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_channel_map_parse,
        cli_cmd_debug_phy_1588_engine_channel_map
    },
    {
        "<channel_map>",
        "action associated to channel",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_channel_map_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },

    {
        "<flow_id>",
        "flow Identifier (0 to 7)",
        CLI_PARM_FLAG_NONE,
        cli_parm_1588_api_flow_id_parse,
        NULL
    },
    {
        "<encap_type>",
        "encapsulation type, Following are the encapsulation types supported\n"
        "\t\t\t0 -> ETH/PTP \n"
        "\t\t\t1 -> ETH/IP/PTP \n"
        "\t\t\t2 -> ETH/IP/IP/PTP \n"
        "\t\t\t3 -> ETH/ETH/PTP \n"
        "\t\t\t4 -> ETH/ETH/IP/PTP \n"
        "\t\t\t5 -> ETH/MPLS/IP/PTP \n"
        "\t\t\t6 -> ETH/MPLS/ETH/PTP \n"
        "\t\t\t7 -> ETH/MPLS/ETH/IP/PTP \n"
        "\t\t\t8 -> ETH/MPLS/ACH/PTP \n"
        "\t\t\t9 -> ETH/OAM \n"
        "\t\t\t10-> ETH/ETH/OAM \n"
        "\t\t\t11-> ETH/MPLS/ETH/OAM\n"
        "\t\t\t12-> ETH/MPLS/ACH/OAM\n"
        "\t\t\t13-> TS ANY PKT",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_encap_type_parse,
        cli_cmd_debug_phy_1588_engine_init
    },
    {
        "<flow_st_index>",
        "Start index of the flow range, i.e., start flow_id",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_flow_st_index_parse,
        cli_cmd_debug_phy_1588_engine_init
    },
    {
        "<flow_end_index>",
        "Last index of the flow range, i.e., Last flow_id",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_flow_end_index_parse,
        cli_cmd_debug_phy_1588_engine_init
    },
    {
        "strict|non-strict",
        "Flow Match configuration \n"
        "\t\t\tstrict:Packet should match against Same flow id in all comparators, except MPLS and PTP\n"
        "\t\t\tnon-strict: Packet can match any flow with in a comparator",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        cli_cmd_debug_phy_1588_engine_init
    },
    {
        "pbb_en|pbb_dis",
        "PBB match configuration \n"
        "\t\t\tpbb_en: To enable PBB encapsulation classification \n"
        "\t\t\tpbb_dis: To disable PBB encapsulation classification",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<etype>",
        "Ethernet protocol Number\n"
        "\t\t\tMost used values:IPv4-> 0x800, IPv6 -> 0x86DD, MPLS -> 0x8847, PTP -> 0x88F7",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_etype_parse,
        NULL
    },
    {
        "<tpid>",
        "Tag Protocoal identifier value(0x1 ~ 0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tpid_parse,
        NULL
    },
    {
        "match_full|any_uc|any_mc",
        "MAC address match mode\n"
        "match_full: Exact Match\n"
        "any_uc    : any unicact MAC\n"
        "any_mc    : any multicast MAC",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<mac_addr>",
        "MAC Address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_mac_addr_parse,
        NULL
    },
    {
        "match_src|match_dest|match_src_dest",
        "Address selection: \n"
        "\tmatch_src      : Source Address\n"
        "\tmatch_dest     : Destination Address\n"
        "\tmatch_src_dest : Source Address  or Destination Address",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "vlan_chk_en|vlan_chk_dis",
        "VLAN check configuration \n"
        "\tvlan_chk_en : To Enable VLAN header classification\n"
        "\tvlan_chk_dis: To Disable VLAN header classification",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<num_tag>",
        "Number of VLAN tags in the packet(possible values: 0,1 or 2)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_vlan_num_tag_parse,
        NULL
    },
    {
        "tag_rng_none|tag_rng_outer|tag_rng_inner",
        "VLAN tag range check\n"
        "\ttag_rng_none : Tag range check disable\n"
        "\ttag_rng_outer: Outer Tag range check enable\n"
        "\ttag_rng_inner: Inner Tag range check enable",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<tag1_type>",
        "Supported tag1 types are Customer tag, Service provider tag, Internal tag, Bridge tag\n"
        "Possible Values for tag types are(1 to 4), 1->Customer tag, 2->Service provider tag, 3->Internal tag, 4->Bridge tag",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag1_type_parse,
        NULL
    },
    {
        "<tag2_type>",
        "Supported tag2 types are Customer tag, Service provider tag, Internal tag, Bridge tag\n"
        "Possible Values for tag types are(1 to 4), 1->Customer tag, 2->Service provider tag, 3->Internal tag, 4->Bridge tag",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag2_type_parse,
        NULL
    },
    {
        "<tag1_lower>",
        "For specifying the range of tag values for tag1,tag1_lower:Start index/tag in the tag range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag1_lower_parse,
        NULL
    },
    {
        "<tag1_upper>",
        "For specifying the range of tag values for tag1,tag1_upper:Last index/tag in the tag range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag1_upper_parse,
        NULL
    },
    {
        "<tag2_lower>",
        "For specifying the range of tag values for tag2,tag2_lower:Start index/tag in the tag range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag2_lower_parse,
        NULL
    },
    {
        "<tag2_upper>",
        "For specifying the range of tag values for tag2,tag2_upper:Last index/tag in the tag range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tag2_upper_parse,
        NULL
    },
    {
        "ipv4|ipv6",
        "IP version ",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<sport_val>",
        "source port value( 1 ~ 0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_sport_val_parse,
        NULL
    },
    {
        "<sport_mask>",
        "source port mask to specify the source port range, value ( 0~0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_sport_mask_parse,
        NULL
    },
    {
        "<dport_val>",
        "destination port value (1 ~ 0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_dport_val_parse,
        NULL
    },
    {
        "<dport_mask>",
        "destination port mask to specify the destination port range, value ( 0~0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_dport_mask_parse,
        NULL
    },
    {
        "<ip_addr>",
        "IPv4 address (in x.x.x.x format, where x is decimal value 0~255)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ipv4_addr_parse,
        NULL
    },
    {
        "<ip_mask>",
        "IPv4 address mask  (in x.x.x.x format, where x is decimal value 0~255)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ipv4_mask_parse,
        NULL
    },
    {
        "ipv4",
        "Keyword to identify ipv4 parameters",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_ip_flow_keyword,
        NULL
    },
    {
        "ipv6",
        "Keyword to identify ipv6 parameters",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_ip_flow_keyword,
        NULL
    },
    {
        "<ipv6_addr>",
        "IPv6 address (in x:x:x:x:x:x:x:x format, where x is hexa-decimal value 0~0xFFFF) ",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ipv6_addr_parse,
        NULL
    },
    {
        "<ipv6_mask>",
        "IPv6 address mask in x:x:x:x:x:x:x:x format, where x is hexa-decimal value 0~0xFFFF",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ipv6_mask_parse,
        NULL
    },
    {
        "cw_en|cw_dis",
        "Control Word configuration\n"
        "\tcw_en : MPLS packet with control word\n"
        "\tcw_dis: MPLS packet with out control word.",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_cw_parse,
        NULL
    },
    {
        "<stk_depth>",
        "Configure stack depth",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_stk_depth_parse,
        NULL
    },
    {
        "stk_ref_top|stk_ref_bottom",
        "stack ref point",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<stk_lvl_0>",
        "syntax: \"stk_lvl_0 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_stk_lvl_0_value_parse,
        NULL
    },
    {
        "<stk_lvl_1>",
        "syntax: \"stk_lvl_1 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_stk_lvl_1_value_parse,
        NULL
    },
    {
        "<stk_lvl_2>",
        "syntax: \"stk_lvl_2 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_stk_lvl_2_value_parse,
        NULL
    },
    {
        "<stk_lvl_3>",
        "syntax: \"stk_lvl_3 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_stk_lvl_3_value_parse,
        NULL
    },
    {
        "<ach_ver>",
        "ACH version",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ach_ver_parse,
        NULL
    },
    {
        "<channel_type>",
        "ACH channel type (0~0xFFFF)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_channel_type_parse,
        NULL
    },
    {
        "<proto_id>",
        "Protocol Identifier",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_proto_id_parse,
        NULL
    },
    {
        "<action_id>",
        "Action ID; range: 0-2 for PTP, 0-5 for OAM",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_action_id_parse,
        NULL
    },
    {
        "ptp",
        "Keyword for PTP action",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<clock_mode>",
        "PTP clock mode: \n"
        "\t0 -> BC One Step\n"
        "\t1 -> BC Two Step\n"
        "\t2 -> TC one Step\n"
        "\t3 -> TC Two Step\n"
        "\t4 -> Delay compensation",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_clock_mode_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<ptp_delaym>",
        "Delay method (0 or 1)\n"
        "\t0 -> Peer to Peer\n"
        "\t1 -> End to End",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_delaym_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<domain_lower>",
        "lower value of domain range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_domain_meg_range_lower_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<domain_upper>",
        "upper value of domain range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_domain_meg_range_upper_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "y1731_oam",
        "keyword for Y1731_OAM action",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "ietf_oam",
        "keyword for IETF_OAM action",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<version>",
        "OAM Version : only 0 is supported",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_oam_version_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<y1731_oam_delaym>",
        "Delay method for y1731_oam \n"
        "\t0 -> One-way Delay Measurement Method(1DM)\n"
        "\t1 -> Two-way Delay Measurement Method(DMM)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_delaym_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<ietf_oam_delaym>",
        "Delay method for ietf_oam \n"
        "\t0 -> Delay Measurement Method(DM)\n"
        "\t1 -> Loss and Delay Measurement(LDM), In LDM :: We only update the TimeStamp Fields \n",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_delaym_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },

    {
        "<meg_lower>",
        "lower value of Maintenence Entity Group (MEG) range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_domain_meg_range_lower_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<meg_upper>",
        "upper value of Maintenence Entity Group (MEG) range",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_domain_meg_range_upper_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<tf>",
        "TimeStamp Format \n"
        "Currently only 3 (PTP TimeStamp Format) is used",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_tf_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "<tc>",
        "Traffic Class : DSCP Field in the Header",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_ds_traffic_class_parse,
        cli_cmd_debug_phy_1588_engine_action_add
    },
    {
        "enable|disable",
        "enable: To Enable,disable: To Disable",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_parse_keyword,
        NULL
    },
    {
        "<signature_mask>",
        "Signature Mask : Signature Mask for extracting the signature bytes from the Packet",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_signature_mask_parse,
        cli_cmd_debug_phy_1588_signature_conf

    },
    {
        "<time_sec>",
        "How many seconds to collect stats, max 60. default 10 seconds.,collects at every second.",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_time_sec_parse,
        NULL
    },
    {
       "<latency_val>",
       "Value of the ingress/egress latency for the particular port in Nano seconds of the range(0-65536)",
       CLI_PARM_FLAG_SET,
       cli_parm_1588_api_latency_parse,
       NULL
    },
    {
       "<delay_val>",
       "Value of the path delay for the particular port in Nano seconds of the range(0-4294967295)",
       CLI_PARM_FLAG_SET,
       cli_parm_1588_api_delay_parse,
       NULL
    },
    {
       "<asym_val>",
       "Value of the delay aymmetry for the particular port in Nano seconds of the range(-32768 to 32767)",
       CLI_PARM_FLAG_SET,
       cli_parm_1588_api_delay_asym_parse,
       NULL
    },
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    {
       "<ing_delay_val>",
       "Value of the ingress delay for the particular port in Nano seconds of the range(-32768 to 32767)",
       CLI_PARM_FLAG_SET,
       cli_parm_1588_api_ing_delay_parse,
       NULL
    },
    {
       "<egr_delay_val>",
       "Value of the egress delay for the particular port in Nano seconds of the range(-32768 to 32767)",
       CLI_PARM_FLAG_SET,
       cli_parm_1588_api_egr_delay_parse,
       NULL
    },
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
    {
       "<int_tc_mode>",
       "Value of the internal tc timesamppong mode:\n"
       "0 = MODE_30BIT, 1 = MODE_32BIT, 2 = MODE_44BIT, 3 = MODE_48BIT",
       CLI_PARM_FLAG_SET,
       cli_parm_tc_internal_mode_parse,
       NULL
    },
    {
        "<rx_ts_pos>",
        "RX Time Stamp Position: 0-1\n"
        "\t\t\t 0 -> 4 Reserved bytes in PTP header\n"
        "\t\t\t 1 -> 4 Bytes appended at the end",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_rx_ts_pos,
        NULL
    },
    {
        "<clk_src>",
        "Clock Source; range: 0-5\n"
        "\t\t\t 0 -> External source\n"
        "\t\t\t 1 -> 10G: XAUI lane 0 recovered clock. 1G: MAC RX clock\n"
        "\t\t\t 2 -> 10G: XAUI lane 0 recovered clock, 1G: MAC TX clock\n"
        "\t\t\t 3 -> Received line clock\n"
        "\t\t\t 4 -> Transmitted line clock\n"
        "\t\t\t 5 -> 10G: Invalid, 1G: Internal 250 MHz Clock",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_clk_src,
        NULL
    },
    {
        "<modify_frm>",
        "modify bit :0 aand 1\n"
        "\t\t\t 0 -> modify bit different PHY\n"
        "\t\t\t 1 -> modify bit  same PHY\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_modify_bit,
        NULL
     },
    {
        "<clkfreq>",
        "Clock Frequency; range: 0-4\n"
        "\t\t\t 0 -> 125 MHz\n"
        "\t\t\t 1 -> 156.25 MHz\n"
        "\t\t\t 2 -> 200 MHz\n"
        "\t\t\t 3 -> 250 MHz\n"
        "\t\t\t 4 -> 500 MHz\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_clkfrq,
        NULL
    },
    {
        "<tx_fifo_mode>",
        "Tx FIFO mode; range: 0-1"
        "\t\t\t 0 -> Read TS from normal CPU interface\n"
        "\t\t\t 1 -> Push TS on the SPI interface",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_tx_fifo_mode,
        NULL
    },
    {
        "<encap>",
        "encine encaplulation mode\n"
        "0 = ETH_PTP, 1 = ETH_IP_PTP, 2 = ETH_IP_IP_PTP, 3 = ETH_ETH_PTP, 4 = ETH_ETH_IP_PTP\n"
        "5 = ETH_MPLS_IP_PTP, 6 = ETH_MPLS_ETH_PTP, 7 = ETH_MPLS_ETH_IP_PTP, 8 = ETH_MPLS_ACH_PTP\n"
        "9 = ETH_OAM, 10 = ETH_ETH_OAM, 11 = ETH_MPLS_ETH_OAM, 12 = ETH_MPLS_ACH_OAM",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_encap_mode,
        NULL
    },
    {
        "<engine>",
        "Engine Identifier (values are 0,1,2 and 3)",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_eng_id_parse,
        cli_cmd_phy_eng_free
    },
    {
        "<nphase_sampler>",
        "nphase sampler values are \n\t\t 0 -> VTSS_PHY_TS_NPHASE_PPS_O,\n\t\t 1 -> VTSS_PHY_TS_NPHASE_PPS_RI,\n\t\t 2 -> VTSS_PHY_TS_NPHASE_EGR_SOF,\n\t\t 3 -> VTSS_PHY_TS_NPHASE_ING_SOF and \n\t\t 4 -> VTSS_PHY_TS_NPHASE_LS",
        CLI_PARM_FLAG_SET,
        cli_parm_1588_api_nphase_sampler_parse,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL,
    },
};

enum {
  PRIO_DEBUG_PHY_1588_ENGINE_INIT = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_CLEAR = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_MODE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_CHANNEL_MAP = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ETH1_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ETH1_FLOW_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ETH2_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ETH2_FLOW_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_IP1_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_IP1_FLOW_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_IP2_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_IP2_FLOW_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_MPLS_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_MPLS_FLOW_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ACH_COMM_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_ADD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_DELETE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_SHOW = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_SIGNATURE_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_NEW_SPI_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_STATS_SHOW = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_LATENCY = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_DELAY = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_DELAY_ASYM = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_NPHASE = CLI_CMD_SORT_KEY_DEFAULT,
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
  PRIO_DEBUG_PHY_1588_ING_DELAY = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_EGR_DELAY = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
  PRIO_DEBUG_TOD_TC_INTERNAL_MODE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_TOD_PHY_TS_MODE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_TOD_PHY_ENG_ALLOC = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_TOD_PHY_ENG_FREE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_TOD_PHY_MON = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Init <port_no> (ingress|egress)",
  "Debug PHY 1588_API Engine Init <port_no> (ingress|egress) <engine_id> <encap_type> <flow_st_index> <flow_end_index> [strict|non-strict]",
  "To initialize the engine for a specific encapsulation type,\n"
  "number of flows, and flow match mode.",
  PRIO_DEBUG_PHY_1588_ENGINE_INIT,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_init,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug PHY 1588_API Engine Clear <port_no> (ingress|egress) <engine_id>",
  "To clear the initialized engine. (To re-use the engine with different encapsulation type or number of flows, engine must be cleared first)",
  PRIO_DEBUG_PHY_1588_ENGINE_CLEAR,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_clear,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Mode <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Mode <port_no> (ingress|egress) <engine_id> [enable|disable]",
  "Enable or Disable the engine. By default an initialised engine is disabled.",
  PRIO_DEBUG_PHY_1588_ENGINE_MODE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_mode,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Channel_Map <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Channel_Map <port_no> (ingress|egress) <engine_id> [<flow_id>] [<channel_map>]",
  "Channel association for each flow in an Engine. Engine configuration is shared by both the channels",
  PRIO_DEBUG_PHY_1588_ENGINE_CHANNEL_MAP,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_channel_map,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf eth1 <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf eth1 <port_no> (ingress|egress) <engine_id> [pbb_en|pbb_dis] [<etype>] [<tpid>]",
  "Configure the Ethernet 1 comparator common parameters (common for all 8 flows)",
  PRIO_DEBUG_PHY_1588_ENGINE_ETH1_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_eth1_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Flow_Conf eth1 <port_no> (ingress|egress) <engine_id> <flow_id>",
  "Debug PHY 1588_API Engine Flow_Conf eth1 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
  "       [enable|disable] [match_full|any_uc|any_mc] [<mac_addr>]\n"
  "       [match_src|match_dest|match_src_dest] [vlan_chk_en|vlan_chk_dis] [<num_tag>]\n"
  "       [tag_rng_none|tag_rng_outer|tag_rng_inner] [<tag1_type>] [<tag2_type>]\n"
  "       [<tag1_lower>] [<tag1_upper>] [<tag2_lower>] [<tag2_upper>]",
  "Configure (flow parameters, enable/disable) the individual flows for \n"
  "Ethernet 1 comparator. (ETH1 classification parameters)",
  PRIO_DEBUG_PHY_1588_ENGINE_ETH1_FLOW_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_eth1_flow_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf eth2 <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf eth2 <port_no> (ingress|egress) <engine_id> [<etype>] [<tpid>]",
  "Configure the Ethernet 2 comparator common parameters (common for all 8 flows)",
  PRIO_DEBUG_PHY_1588_ENGINE_ETH2_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_eth2_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Flow_Conf eth2 <port_no> (ingress|egress) <engine_id> <flow_id>",
  "Debug PHY 1588_API Engine Flow_Conf eth2 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
  "       [enable|disable] [match_full|any_uc|any_mc] [<mac_addr>]\n"
  "       [match_src|match_dest|match_src_dest] [vlan_chk_en|vlan_chk_dis] [<num_tag>]\n"
  "       [tag_rng_none|tag_rng_outer|tag_rng_inner] [<tag1_type>] [<tag2_type>]\n"
  "       [<tag1_lower>] [<tag1_upper>] [<tag2_lower>] [<tag2_upper>]",
  "Configure (flow parameters, enable/disable) the individual flows for \n"
  "Ethernet 2 comparator. (ETH2 classification parameters)",
  PRIO_DEBUG_PHY_1588_ENGINE_ETH2_FLOW_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_eth2_flow_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf ip1 <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf ip1 <port_no> (ingress|egress) <engine_id>\n"
  "       [ipv4|ipv6] [<sport_val>] [<sport_mask>] [<dport_val>] [<dport_mask>]",
  "Configure IP comparator 1 for the common part. (common for all 8 flows)\n"
  "The common part of flows includes IP version, Src port, dest port",
  PRIO_DEBUG_PHY_1588_ENGINE_IP1_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_ip1_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Flow_Conf ip1 <port_no> (ingress|egress) <engine_id> <flow_id>",
  "Debug PHY 1588_API Engine Flow_Conf ip1 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
  "       [enable|disable] [match_src|match_dest|match_src_dest]\n"
  "       [(ipv4 <ip_addr> <ip_mask>) | (ipv6 <ipv6_addr> <ipv6_mask>)]",
  "Configure IP comparator 1 individual flows of supported 8 flows.\n"
  "And enable or disable the configured flows.",
  PRIO_DEBUG_PHY_1588_ENGINE_IP1_FLOW_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_ip1_flow_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf ip2 <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf ip2 <port_no> (ingress|egress) <engine_id>\n"
  "       [ipv4|ipv6] [<sport_val>] [<sport_mask>] [<dport_val>] [<dport_mask>]",
  "Configure IP comparator 2 for the common part (common for all 8 flows)\n"
  "The common part of flows includes IP version, Src port, dest port",
  PRIO_DEBUG_PHY_1588_ENGINE_IP2_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_ip2_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Flow_Conf ip2 <port_no> (ingress|egress) <engine_id> <flow_id>",
  "Debug PHY 1588_API Engine Flow_Conf ip2 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
  "       [enable|disable] [match_src|match_dest|match_src_dest]\n"
  "       [(ipv4 <ip_addr> <ip_mask>) | (ipv6 <ipv6_addr> <ipv6_mask>)]",
  "Configure IP comparator 2 individual flows of supported 8 flows.\n"
  "And enable or disable the configured flows.",
  PRIO_DEBUG_PHY_1588_ENGINE_IP2_FLOW_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_ip2_flow_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf mpls <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf mpls <port_no> (ingress|egress) <engine_id> [cw_en|cw_dis]",
  "Configure the MPLS Comparator common part (common for supported 8 MPLS flows)",
  PRIO_DEBUG_PHY_1588_ENGINE_MPLS_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_mpls_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Flow_Conf mpls <port_no> (ingress|egress) <engine_id> <flow_id>",
  "Debug PHY 1588_API Engine Flow_Conf mpls <port_no> (ingress|egress) <engine_id> <flow_id>\n"
  "       [enable|disable] [<stk_depth>] [stk_ref_top|stk_ref_bottom]\n"
  "       [<stk_lvl_0>] [<stk_lvl_1>] [<stk_lvl_2>] [<stk_lvl_3>]",
  "Configure the MPLS Comparator,the supported 8 MPLS flows individually.\n"
  "Enable or disable the configured flows",
  PRIO_DEBUG_PHY_1588_ENGINE_MPLS_FLOW_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_mpls_flow_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Comm_Conf ACH <port_no> (ingress|egress) <engine_id>",
  "Debug PHY 1588_API Engine Comm_Conf ACH <port_no> (ingress|egress) <engine_id>\n"
  "       [<ach_ver>] [<channel_type>] [<proto_id>]",
  "Configure the IP1/ACH Comparator, for common part of the supported 8 ACH flows",
  PRIO_DEBUG_PHY_1588_ENGINE_ACH_COMM_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_ach_comm_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  NULL,
  "Debug PHY 1588_API Engine Action Add <port_no> (ingress|egress) <engine_id>\n"
  "       <action_id> <channel_map>\n"
  "       [(ptp <clock_mode> <ptp_delaym> <domain_lower> <domain_upper>) |\n"
  "        (y1731_oam <version> <y1731_oam_delaym> <meg_lower> <meg_upper>) |\n"
  "        (ietf_oam <version> <ietf_oam_delaym> <tf> <tc>)]",
  "Configure the action for PTP/OAM comparator ( Action for identified PTP flows)",
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_ADD,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_action_add,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug PHY 1588_API Engine Action Delete <port_no> (ingress|egress) <engine_id> <action_id>",
  "Delete the actions configured.",
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_DELETE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_action_delete,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Engine Action Show <port_no> (ingress|egress) <engine_id>",
  NULL,
  "Display the Actions configured for the engine",
  PRIO_DEBUG_PHY_1588_ENGINE_ACTION_SHOW,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_engine_action_show,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Signature <port_no>",
  "Debug PHY 1588_API Signature <port_no> [<signature_mask>]",
  "Configure the signature mask for egress TX FIFO, to capture the tx timestamps\n"
  "used in two step synchronization",
  PRIO_DEBUG_PHY_1588_SIGNATURE_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_signature_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
cli_cmd_tab_entry (
  "Debug PHY 1588_API New_SPI <port_no>",
  "Debug PHY 1588_API New_SPI <port_no> [enable|disable]",
  "Enable or Disable the New SPI mode, New Tx Timestamp capturing method",
  PRIO_DEBUG_PHY_1588_NEW_SPI_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_new_spi_conf,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */


cli_cmd_tab_entry (
  "Debug PHY 1588_API Stats [<time_sec>]",
  NULL,
  "PHY TS stats show",
  PRIO_DEBUG_PHY_1588_STATS_SHOW,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_ts_stats_show,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug PHY 1588_API Latency <port_no> (ingress|egress) [<latency_val>]",
  NULL,
  "Get/Set the ingress/egress latency for a port",
  PRIO_DEBUG_PHY_1588_LATENCY,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_latency,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_API Path_Delay <port_no> [<delay_val>]",
  NULL,
  "Get/Set the Path delay for a port",
  PRIO_DEBUG_PHY_1588_DELAY,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_debug_phy_1588_delay,
  tod_cli_req_default_set,
  tod_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PHY 1588_API Delay_Asym <port_no> [<asym_val>]",
    NULL,
    "Get/Set the Delay Asymmetry for a port",
    PRIO_DEBUG_PHY_1588_DELAY_ASYM,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_1588_delay_asym,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
cli_cmd_tab_entry (
    "Debug PHY 1588_API Ing_Delay <port_no> [<ing_delay_val>]",
    NULL,
    "Get/Set the Ingress Delay for a port",
    PRIO_DEBUG_PHY_1588_ING_DELAY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_1588_ing_delay,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug PHY 1588_API Egr_Delay <port_no> [<egr_delay_val>]",
    NULL,
    "Get/Set the Egress Delay for a port",
    PRIO_DEBUG_PHY_1588_EGR_DELAY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_1588_egr_delay,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */

cli_cmd_tab_entry (
    "Debug TOD TC Internal [<int_tc_mode>]",
    NULL,
    "Get/Set the Internal TC mode\n"
    "When the mode is changed, the node must be rebooted before the change takes place",
    PRIO_DEBUG_TOD_TC_INTERNAL_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_tod_tc_internal_mode,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug TOD PHY TS <port_no> [enable|disable]",
    NULL,
    "Get/Set the PHY Timestamping mode.\n"
    "On ports equipped with a timestamp phy, the use of the timestamp feature can be enabled or disabled\n"
    "On ports not equipped with a timestamp phy, this mode sets the backplane mode used for internal ports in a distributed system\n"
    "When the mode is changed, the node must be rebooted before the change takes place",
    PRIO_DEBUG_TOD_PHY_TS_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_ts_enable,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug TOD Time <port_no> [<time_sec>]",
    NULL,
    "Set the PHY Time\n",
    PRIO_DEBUG_TOD_PHY_TS_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_ptptime,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug phy 1588 Block Init <port_no> [<clkfreq>] [<clk_src>] [<rx_ts_pos>] [<tx_fifo_mode>] [<modify_frm>]",
    NULL,
    "Initialize the 1588 Block\n",
    PRIO_DEBUG_TOD_PHY_TS_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_phy_1588_block_init,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PTP ExtClockMode",
    NULL,
    "Update or show the 1PPS and External clock output configuration\n"
    "and vcxo frequency rate adjustment option.\n"
    "(if vcxo mode is changed, the node must be restarted)",
    PRIO_DEBUG_TOD_PHY_TS_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_ext_pps_out,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug TOD EngAlloc <port_list> [<encap>]",
    NULL,
    "Allocate or show allocated engines for a port",
    PRIO_DEBUG_TOD_PHY_ENG_ALLOC,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_phy_eng_alloc,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug TOD EngFree <port_list> <engine>",
    NULL,
    "Free an allocated engine for a port",
    PRIO_DEBUG_TOD_PHY_ENG_FREE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_phy_eng_free,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug TOD Monitor <port_list> [enable|disable]",
    NULL,
    "Enable or disable timeofday monitoring for a port",
    PRIO_DEBUG_TOD_PHY_MON,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_phy_monitor,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PHY 1588_API nphase <port_no> [<nphase_sampler>]",
    NULL,
    "Get N-phase status on a port",
    PRIO_DEBUG_PHY_1588_NPHASE,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_phy_1588_nphase,
    tod_cli_req_default_set,
    tod_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

