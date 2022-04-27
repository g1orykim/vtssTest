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

*/

#include "icfg_api.h"
#include "icli_api.h"
#include "misc_api.h"
#include "eth_link_oam_api.h"
#include "eth_link_oam_icfg.h"
#include "topo_api.h"

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_ETH_LINK_OAM

//******************************************************************************
// ICFG callback functions
//******************************************************************************

static vtss_rc eth_link_oam_icfg_intf_conf ( const vtss_icfg_query_request_t *req,
                                             vtss_icfg_query_result_t *result
                                           )
{
    vtss_isid_t                    isid;
    vtss_port_no_t                 iport;
    vtss_eth_link_oam_conf_t       conf;
    u16                            secs_summary_threshold;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    /* Interface level: link-oam control, link-oam mode active|passive, link-oam remote loopback, link-oam link-monitor,
       link-oam mib retrieval
    */

    isid = topo_usid2isid(req->instance_id.port.usid);
    iport = uport2iport(req->instance_id.port.begin_uport);

    memset(&conf, 0, sizeof(conf));
    if (eth_link_oam_mgmt_port_conf_get (isid, iport, &conf) != VTSS_OK) {
        T_E("Could not get Link OAM port conf\n");
        return VTSS_RC_ERROR;
    }
    if (eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get(isid, iport, &secs_summary_threshold) != VTSS_OK) {
        T_E("Could not get Link OAM sec summary threshold conf\n");
        return VTSS_RC_ERROR;
    }
    if (conf.oam_control != VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " %slink-oam\n", conf.oam_control ? "" : "no "));
    }
    if (conf.oam_mode != VTSS_ETH_LINK_OAM_DEFAULT_PORT_MODE || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam mode %s\n", conf.oam_mode ? "active" : "passive"));
    }
    if ((conf.oam_remote_loop_back_support != VTSS_ETH_LINK_OAM_DEFAULT_PORT_REMOTE_LOOPBACK_SUPPORT ) &&
        (conf.oam_control != VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL)) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam remote-loopback supported\n"));
    } else if (req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " %slink-oam remote-loopback supported\n", conf.oam_remote_loop_back_support ? "" : "no "));
    }
    if ((conf.oam_link_monitoring_support != VTSS_ETH_LINK_OAM_DEFAULT_PORT_LINK_MONITORING_SUPPORT) &&
        (conf.oam_control != VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL)) {
        VTSS_RC(vtss_icfg_printf(result, " no link-oam link-monitor supported\n"));
    } else if (req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " %slink-oam link-monitor supported\n", conf.oam_link_monitoring_support ? "" : "no "));
    }
    if ((conf.oam_mib_retrival_support != VTSS_ETH_LINK_OAM_DEFAULT_PORT_MIB_RETRIVAL_SUPPORT) &&
        (conf.oam_control != VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL)) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam mib-retrieval supported\n"));
    } else if (req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " %slink-oam mib-retrieval supported\n", conf.oam_mib_retrival_support ? "" : "no "));
    }
    if (conf.oam_error_frame_event_conf.window != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_WINDOW_CONF
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor frame window %d\n", conf.oam_error_frame_event_conf.window));
    }
    if (conf.oam_error_frame_event_conf.threshold != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_THRESHOLD_CONF
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor frame threshold %u\n", conf.oam_error_frame_event_conf.threshold));
    }
    if (conf.oam_symbol_period_event_conf.window != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_WINDOW_CONF
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor symbol-period window %llu\n", conf.oam_symbol_period_event_conf.window));
    }
    if (conf.oam_symbol_period_event_conf.threshold != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_THRESHOLD_CONF
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor symbol-period threshold %llu\n",
                                 conf.oam_symbol_period_event_conf.threshold));
    }
    if (conf.oam_error_frame_secs_summary_event_conf.window != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_SECS_SUMMARY_WINDOW_MIN
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor frame-seconds window %u\n", conf.oam_error_frame_secs_summary_event_conf.window));
    }
    if (secs_summary_threshold != VTSS_ETH_LINK_OAM_DEFAULT_EVENT_SECS_SUMMARY_THRESHOLD_MIN
        || req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, " link-oam link-monitor frame-seconds threshold %u\n", secs_summary_threshold));
    }
    return VTSS_OK;
}

//******************************************************************************
//   Public functions
//******************************************************************************

vtss_rc eth_link_oam_icfg_init(void)
{
    return vtss_icfg_query_register ( VTSS_ICFG_ETH_LINK_OAM_INTERFACE_CONF,
                                      "link-oam", eth_link_oam_icfg_intf_conf );
}
