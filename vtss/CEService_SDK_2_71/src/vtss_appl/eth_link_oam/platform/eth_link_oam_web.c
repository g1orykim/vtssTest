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

*/
#include "web_api.h"
#include "vtss_types.h"
#include "msg_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#include "vtss_eth_link_oam_api.h"
#include "eth_link_oam_api.h"
#include "vtss_eth_link_oam_control_api.h"
#include "cli_trace_def.h"
#endif
#define LOAM_WEB_BUF_LEN 1024
//extern struct cli_io_mem cli_io_mem;
cyg_int32 handler_config_oam_ports(CYG_HTTPD_STATE *p)
{
    vtss_isid_t              isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                      ct;
    vtss_eth_link_oam_conf_t conf;
    char                     *err = NULL;
    vtss_rc                  rc = VTSS_RC_OK;
    vtss_eth_link_oam_info_tlv_t local_info;
    int temp;
    port_iter_t              pit;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_ETH_LINK_OAM)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int val;
        memset(&conf, 0, sizeof(conf));
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (eth_link_oam_mgmt_port_conf_get(isid, pit.iport, &conf) == VTSS_OK ) {
                    vtss_eth_link_oam_conf_t new, *old = &conf;
                    BOOL                     oper_lb = FALSE, current_oper_lb = FALSE;
                    memset(&new, 0, sizeof(new));
                    new = *old;
                    new.oam_control = cyg_httpd_form_variable_check_fmt(p, "enable_%d", pit.uport);
                    new.oam_remote_loop_back_support = cyg_httpd_form_variable_check_fmt(p, "loop_%d", pit.uport);
                    if (cyg_httpd_form_variable_int_fmt(p, &val, "role_%d", pit.uport)) {
                        new.oam_mode = val;
                    }
                    new.oam_link_monitoring_support = cyg_httpd_form_variable_check_fmt(p, "monitor_%d", pit.uport);
                    new.oam_mib_retrival_support = cyg_httpd_form_variable_check_fmt(p, "mib_%d", pit.uport);
                    oper_lb = cyg_httpd_form_variable_check_fmt(p, "olb_%d", pit.uport);
                    if (eth_link_oam_client_port_local_info_get (isid, pit.iport, &local_info) == VTSS_RC_OK) {
                        current_oper_lb = (local_info.state & 3) ? 1 : 0;
                    }

                    if ((memcmp(&new, old, sizeof(*old)) != 0)) {
                        *old = new;
                        rc = eth_link_oam_mgmt_port_control_conf_set(isid, pit.iport, conf.oam_control);

                        if ( rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                            rc = eth_link_oam_mgmt_port_mode_conf_set(isid, pit.iport, conf.oam_mode);
                        }

                        if ( rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                            rc = eth_link_oam_mgmt_port_remote_loopback_conf_set(isid, pit.iport, conf.oam_remote_loop_back_support);
                        }
                        if ( rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                            rc = eth_link_oam_mgmt_port_link_monitoring_conf_set(isid, pit.iport, conf.oam_link_monitoring_support);
                        }

                        if ( rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                            rc = eth_link_oam_mgmt_port_mib_retrival_conf_set(isid, pit.iport, conf.oam_mib_retrival_support);
                        }

                        if ( rc != VTSS_RC_OK  && rc != ETH_LINK_OAM_RC_ALREADY_CONFIGURED ) {
                            err = "Error while configuring the OAM";
                        }

                    }
                    if (current_oper_lb != oper_lb) {
                        rc = eth_link_oam_mgmt_port_remote_loopback_oper_conf_set(isid, pit.iport, oper_lb);
                        if ( rc != VTSS_RC_OK  && rc != ETH_LINK_OAM_RC_ALREADY_CONFIGURED ) {
                            switch (rc) {
                            case ETH_LINK_OAM_RC_NOT_SUPPORTED:
                                err = "Error requested configuration is not supported with the current OAM mode";
                                break;
                            case ETH_LINK_OAM_RC_TIMED_OUT:
                                err = "Error requested operation gets timed out";
                                break;
                            default:
                                err = "Error while configuring the OAM loopback";
                            }
                        }
                    }
                }
            }
        }

        if (err != NULL) {
            send_custom_error(p, "OAM Error", err, strlen(err));
        } else {
            redirect(p, "/eth_link_oam_port_config.htm");
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        memset(&conf, 0, sizeof(conf));
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (eth_link_oam_mgmt_port_conf_get(isid, pit.iport, &conf) == VTSS_RC_OK
                    && eth_link_oam_client_port_local_info_get (isid, pit.iport, &local_info) == VTSS_RC_OK ) {
                    const vtss_eth_link_oam_conf_t *pp = &conf;
                    temp = (local_info.state & 3);

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%d/%d|",
                                  pit.uport,
                                  pp->oam_control,
                                  pp->oam_remote_loop_back_support,
                                  pp->oam_mode,
                                  pp->oam_link_monitoring_support,
                                  pp->oam_mib_retrival_support,
                                  temp ? 1 : 0
                                 );
                    if (ct <= 0) {
                        T_D("Failed to do snprintf\n");
                    }
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

cyg_int32 handler_stat_link_oam_statistics(CYG_HTTPD_STATE *p)
{
    vtss_isid_t isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                    ct;
    port_iter_t            pit;
    vtss_eth_link_oam_control_port_conf_t pp;
    vtss_rc                rc;

    //if(redirectUnmanagedOrInvalid(p, isid)) /* Redirect unmanaged/invalid access to handler */
    //  return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_ETH_LINK_OAM)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    memset(&pp, 0, sizeof(pp));

    // Loop through all front ports
    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {

            if (var_clear[0]) {      /* Clear? */
                if (eth_link_oam_clear_statistics(L2PORT2PORT(isid, pit.iport)) != VTSS_RC_OK) {
                    return -1;
                }
            }

            rc = eth_link_oam_control_layer_port_pdu_stats_get(isid, L2PORT2PORT(isid, pit.iport), &pp.oam_stats);
            if (rc != VTSS_RC_OK) {
                return -1;
            } else {
                rc = eth_link_oam_control_layer_port_critical_event_pdu_stats_get(isid, L2PORT2PORT(isid, pit.iport),
                                                                                  &pp.oam_ce_stats);
            }
            if (rc != VTSS_RC_OK) {
                return -1;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%u/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d|",
                          pit.uport,
                          (int)pp.oam_stats.information_rx,
                          (int)pp.oam_stats.information_tx,
                          (int)pp.oam_stats.unique_event_notification_rx,
                          (int)pp.oam_stats.unique_event_notification_tx,
                          (int)pp.oam_stats.duplicate_event_notification_rx,
                          (int)pp.oam_stats.duplicate_event_notification_tx,
                          (int)pp.oam_stats.loopback_control_rx,
                          (int)pp.oam_stats.loopback_control_tx,
                          (int)pp.oam_stats.variable_request_rx,
                          (int)pp.oam_stats.variable_request_tx,
                          (int)pp.oam_stats.variable_response_rx,
                          (int)pp.oam_stats.variable_response_tx,
                          (int)pp.oam_stats.org_specific_rx,
                          (int)pp.oam_stats.org_specific_tx,
                          (int)pp.oam_stats.unsupported_codes_rx,
                          (int)pp.oam_stats.unsupported_codes_tx,
                          (int)pp.oam_ce_stats.link_fault_rx,
                          (int)pp.oam_ce_stats.link_fault_tx,
                          (int)pp.oam_ce_stats.dying_gasp_rx,
                          (int)pp.oam_ce_stats.dying_gasp_tx,
                          (int)pp.oam_ce_stats.critical_event_rx,
                          (int)pp.oam_ce_stats.critical_event_tx);
            if (ct <= 0) {
                T_D("Failed to do snprintf\n");
            }
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    //cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_oam_port_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                                 isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                                 pit;
    int                                         ct = 0;
    char                                        buf[32] = {0}, disc_buf[32] = {0};
    vtss_eth_link_oam_info_tlv_t                local_info, remote_info;
    u16                                         temp = 0, remote_temp = 0;
    vtss_eth_link_oam_discovery_state_t         state;
    vtss_eth_link_oam_pdu_control_t             tx_control;
    BOOL                                        rc = TRUE, rc_peer = TRUE;
    u16                                         mtu_size = 0, remote_mtu_size = 0;
    u8                                          oui_buf[80] = {0}, remote_oui_buf[80] = {0};
    int char_cnt                                = 0;
    u8                                          remote_mac[6] = {0}, temp_remote_mac[80] = {0};

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    // Loop through all front ports
    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {

            rc_peer = TRUE;
            rc = TRUE;

            /*Retrieve Local Information first*/
            if (eth_link_oam_client_port_local_info_get (isid, pit.iport, &local_info) == VTSS_RC_OK) {
                memcpy(&temp, local_info.revision, sizeof(temp));
                temp = NET2HOSTS(temp);
                memcpy(&mtu_size, local_info.oampdu_conf, sizeof(u16));
                mtu_size = NET2HOSTS(mtu_size);
                memset(oui_buf, 0, sizeof(oui_buf));
                char_cnt = snprintf((i8 *)oui_buf, sizeof(oui_buf), "%02x-%02x-%02x", local_info.oui[0], local_info.oui[1], local_info.oui[2]);
                if (char_cnt <= 0) {
                    T_D("Failed to do snprintf\n");
                }
            } else {
                rc = FALSE;
            }

            /*Retrieve peer information */
            if (eth_link_oam_client_port_remote_info_get(isid, pit.iport, &remote_info) == VTSS_RC_OK) {
                memcpy(&remote_temp, remote_info.revision, sizeof(temp));
                remote_temp = NET2HOSTS(remote_temp);
                memcpy(&remote_mtu_size, remote_info.oampdu_conf, sizeof(u16));
                remote_mtu_size = NET2HOSTS(remote_mtu_size);
                memset(remote_oui_buf, 0, sizeof(remote_oui_buf));
                char_cnt = snprintf((i8 *)remote_oui_buf, sizeof(remote_oui_buf), "%02x-%02x-%02x", remote_info.oui[0], remote_info.oui[1], remote_info.oui[2]);
                if (char_cnt <= 0) {
                    T_D("Failed to do snprintf\n");
                }
            } else {
                rc_peer = FALSE;
            }

            if (eth_link_oam_control_layer_port_pdu_control_status_get(isid, pit.iport,
                                                                       &tx_control) != VTSS_RC_OK) {
                rc = FALSE;
            } else {
                memset(buf, 0, sizeof(buf));
                strncpy(buf, (i8 *)pdu_tx_control_to_str(tx_control), sizeof(buf));
            }

            if (eth_link_oam_control_layer_port_discovery_state_get(isid, pit.iport, &state) != VTSS_RC_OK) {
                rc = FALSE;
            } else {
                memset(disc_buf, 0, sizeof(disc_buf));
                strncpy(disc_buf, (i8 *)discovery_state_to_str(state), sizeof(disc_buf));
            }
            if (eth_link_oam_client_port_remote_mac_addr_info_get(isid, pit.iport,
                                                                  remote_mac) != VTSS_RC_OK) {
                rc_peer = FALSE;
            } else {
                ct = snprintf((i8 *)temp_remote_mac, sizeof(temp_remote_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                              remote_mac[0], remote_mac[1], remote_mac[2], remote_mac[3], remote_mac[4], remote_mac[5]);
                if (ct <= 0) {
                    T_D("Failed to do snprintf\n");
                }
            }
            if (rc == TRUE && rc_peer == FALSE) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%s/%s/%s/%s/%d/%s/%s/%s/%d/%s/%s/%s/%s/%s/%s/%s/%s/%s/%s/%s/%s/%s|",
                              pit.uport,
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE) ? "Active" : "Passive",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT) ? "Enabled" : "Disabled",
                              mtu_size,
                              mux_state_to_str((local_info.state & 4) >> 2),
                              parser_state_to_str(local_info.state & 3),
                              oui_buf,
                              temp,
                              buf,
                              disc_buf,
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------",
                              "------"
                             );
                if (ct <= 0) {
                    T_D("Failed to do snprintf\n");
                }
            } else if (rc == TRUE) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%s/%s/%s/%s/%d/%s/%s/%s/%d/%s/%s/%s/%s/%s/%s/%s/%d/%s/%s/%s/%d/%s|",
                              pit.uport,
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE) ? "Active" : "Passive",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT) ? "Enabled" : "Disabled",
                              mtu_size,
                              mux_state_to_str((local_info.state & 4) >> 2),
                              parser_state_to_str(local_info.state & 3),
                              oui_buf,
                              temp,
                              buf,
                              disc_buf,
                              IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE) ? "Active" : "Passive",
                              IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT) ? "Enabled" : "Disabled",
                              IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT) ? "Enabled" : "Disabled",
                              remote_mtu_size,
                              mux_state_to_str((remote_info.state & 4) >> 2),
                              parser_state_to_str(remote_info.state & 3),
                              remote_oui_buf,
                              remote_temp,
                              temp_remote_mac
                             );
                if (ct <= 0) {
                    T_D("Failed to do snprintf\n");
                }
            }
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }
    //cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_diag_mib_retrieve(CYG_HTTPD_STATE *p)
{
    vtss_isid_t  isid = 0;
    char         *err = NULL;
    BOOL         opr_lock = FALSE;
    static i8           buf[VTSS_ETH_LINK_OAM_RESPONSE_BUF] = {0};
    vtss_rc      rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_ETH_LINK_OAM)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        size_t len;
        int port;
        BOOL port_rc = TRUE;
        const char *sel = cyg_httpd_form_varable_string(p, "select", &len);
        port_rc = cyg_httpd_form_varable_int(p, "port_no", &port);
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (msg_switch_exists (isid) == FALSE) {
                continue;
            } /* end of if */
            break;
        } /* end of for */
        memset(buf, 0, sizeof(buf));
        if (!port_rc) {
            return -1;
        } else {
            vtss_eth_link_oam_mgmt_client_register_cb(uport2iport(port), vtss_eth_link_oam_send_response_to_cli, buf, VTSS_ETH_LINK_OAM_RESPONSE_BUF);
        }
        if (!memcmp(sel, "1", len)) {
            rc = eth_link_oam_mgmt_port_mib_retrival_oper_set(isid, uport2iport(port), 1);
        } else {
            rc = eth_link_oam_mgmt_port_mib_retrival_oper_set(isid, uport2iport(port), 2);
        }
        if (rc == VTSS_RC_INV_STATE) {
            err = "Requested operation is already in progress";
        } else if (rc != VTSS_RC_OK) {
            err = "Invalid request on this port";
        } else {
            opr_lock = vtss_eth_link_oam_mib_retrival_opr_lock();
            if (opr_lock == FALSE) {
                err = "Lock got timed out";
            }
            vtss_eth_link_oam_mgmt_client_deregister_cb(uport2iport(port));
        }
        if (err != NULL) {
            send_custom_error(p, "OAM Error", err, strlen(err));
        } else {
            redirect(p, "/eth_link_oam_mib_support_result.htm");
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        cyg_httpd_write_chunked((i8 *)buf, VTSS_ETH_LINK_OAM_RESPONSE_BUF);
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}


static cyg_int32 handler_config_link_oam_events(CYG_HTTPD_STATE *p)
{
    vtss_isid_t isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                    ct;
    port_iter_t            pit;
    int port;
    u16 error_frame_window, secs_summary_threshold, secs_summary_window;
    u32 err_fr_win_temp, error_frame_threshold/*, frame_period_window, frame_period_threshold*/;
    u32  secs_summary_window_temp, secs_summary_threshold_temp;
    u64 symbol_period_window, symbol_period_threshold, error_frame_threshold_temp/*, symbol_period_rx, frame_period_rx*/;
    vtss_rc rc;
    char                     *err = NULL;
    char                     url[80];
    //if(redirectUnmanagedOrInvalid(p, isid)) /* Redirect unmanaged/invalid access to handler */
    //  return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_ETH_LINK_OAM)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (cyg_httpd_form_varable_int(p, "port_no", &port) &&
            cyg_httpd_form_varable_int(p, "err_frame_window", (int *)&err_fr_win_temp) &&
            cyg_httpd_form_varable_uint64(p, "err_frame_threshold", &error_frame_threshold_temp) &&
            cyg_httpd_form_varable_int(p, "secs_summary_window", (int *)&secs_summary_window_temp) &&
            cyg_httpd_form_varable_int(p, "secs_summary_threshold", (int *)&secs_summary_threshold_temp) &&
            cyg_httpd_form_varable_uint64(p, "symbol_frame_window", &symbol_period_window) &&
            cyg_httpd_form_varable_uint64(p, "symbol_frame_threshold", &symbol_period_threshold)/* &&
                                                                                                       cyg_httpd_form_varable_uint64(p, "symbol_rx_packet_threshold", &symbol_period_rx) &&
                                                                                                       cyg_httpd_form_varable_int(p, "frame_period_window", (int *)&frame_period_window) &&
                                                                                                       cyg_httpd_form_varable_int(p, "frame_period_threshold", (int *)&frame_period_threshold) &&
                                                                                                       cyg_httpd_form_varable_uint64(p, "frame_rx_packet_threshold", &frame_period_rx)*/) {

            error_frame_window = (u16)err_fr_win_temp;
            error_frame_threshold = (u32)error_frame_threshold_temp;
            secs_summary_window = (u16)secs_summary_window_temp;
            secs_summary_threshold = (u16) secs_summary_threshold_temp;

            rc = eth_link_oam_mgmt_port_link_error_frame_window_set(isid, uport2iport(port), error_frame_window);
            if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = eth_link_oam_mgmt_port_link_error_frame_threshold_set(isid, uport2iport(port), error_frame_threshold);
            }
            if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = eth_link_oam_mgmt_port_link_symbol_period_error_window_set(isid, uport2iport(port), symbol_period_window);
            }
            if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set(isid, uport2iport(port), symbol_period_threshold);
            }
            /*          if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                        rc = eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set(isid, uport2iport(port), symbol_period_rx);
                        }
                        if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                        rc = eth_link_oam_mgmt_port_link_frame_period_error_window_set(isid, uport2iport(port), frame_period_window);
                        }
                        if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                        rc = eth_link_oam_mgmt_port_link_frame_period_error_threshold_set(isid, uport2iport(port), frame_period_threshold);
                        }
                        if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                        rc = eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set(isid, uport2iport(port), frame_period_rx);
                        }
             */
            if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set(isid, uport2iport(port), secs_summary_window);
            }
            if (rc == VTSS_RC_OK || rc == ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set(isid, uport2iport(port), secs_summary_threshold);
            }
            if (rc != VTSS_RC_OK && rc != ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                err = "Error While Configuring Link Events";
            }
            if (err != NULL) {
                send_custom_error(p, "OAM Error", err, strlen(err));
            } else {
                memset(url, 0, sizeof(url));
                (void)snprintf(url, sizeof(url), "/eth_link_oam_link_event_config.htm?port=%d", port);
                redirect(p, url);
            }
        }
    } else {
        cyg_httpd_start_chunked("html");

        //memset(&pp,0,sizeof(pp));

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                rc = eth_link_oam_mgmt_port_link_error_frame_window_get(isid, pit.iport, &error_frame_window);
                if (rc != VTSS_RC_OK) {
                    return -1;
                } else {
                    rc = eth_link_oam_mgmt_port_link_error_frame_threshold_get(isid, pit.iport, &error_frame_threshold);
                }
                if (rc != VTSS_RC_OK) {
                    return -1;
                } else {
                    rc = eth_link_oam_mgmt_port_link_symbol_period_error_window_get(isid, pit.iport, &symbol_period_window);
                }
                if (rc != VTSS_RC_OK) {
                    return -1;
                } else {
                    rc = eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get(isid, pit.iport, &symbol_period_threshold);
                }
                /*          if (rc != VTSS_RC_OK) {
                            return -1;
                            } else {
                            rc = eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get(isid, pit.iport, &symbol_period_rx);
                            }
                            if (rc != VTSS_RC_OK) {
                            return -1;
                            } else {
                            rc = eth_link_oam_mgmt_port_link_frame_period_error_window_get(isid, pit.iport, &frame_period_window);
                            }
                            if (rc != VTSS_RC_OK) {
                            return -1;
                            } else {
                            rc = eth_link_oam_mgmt_port_link_frame_period_error_threshold_get(isid, pit.iport, &frame_period_threshold);
                            }
                            if (rc != VTSS_RC_OK) {
                            return -1;
                            } else {
                            rc = eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get(isid, pit.iport, &frame_period_rx);
                            }
                 */
                if (rc != VTSS_RC_OK) {
                    return -1;
                } else {
                    rc = eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get(isid, pit.iport, &secs_summary_window);
                }
                if (rc != VTSS_RC_OK) {
                    return -1;
                } else {
                    rc = eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get(isid, pit.iport, &secs_summary_threshold);
                }
                if (rc != VTSS_RC_OK) {
                    return -1;
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%d/%u/%u/%llu/%llu/%u/%u|",/*/%llu/%d/%d/%llu|*/
                              pit.uport,
                              error_frame_window,
                              error_frame_threshold,
                              symbol_period_window,
                              symbol_period_threshold,
                              secs_summary_window,
                              secs_summary_threshold/*,
                                             symbol_period_rx,
                                             (int)frame_period_window,
                                             (int)frame_period_threshold,
                                             frame_period_rx*/
                             );
                if (ct <= 0) {
                    T_D("Failed to do snprintf\n");
                }
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }

        }
        //cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_link_oam_link_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                  isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                  pit;
    vtss_rc                      rc;
    char                        *err = NULL;
    u16                         seq_num;
    vtss_eth_link_oam_error_frame_event_tlv_t   error_frame_tlv;
    vtss_eth_link_oam_error_frame_event_tlv_t   remote_error_frame_tlv;
    vtss_eth_link_oam_error_frame_period_event_tlv_t  error_frame_period_tlv;
    vtss_eth_link_oam_error_frame_period_event_tlv_t  remote_error_frame_period_tlv;
    vtss_eth_link_oam_error_symbol_period_event_tlv_t  error_symbol_period_tlv;
    vtss_eth_link_oam_error_symbol_period_event_tlv_t  remote_error_symbol_period_tlv;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t  local_secs_info;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t  remote_secs_info;
    int                    ct;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    // Loop through all front ports
    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            memset(&error_symbol_period_tlv, 0, sizeof(error_symbol_period_tlv));
            memset(&remote_error_symbol_period_tlv, 0, sizeof(remote_error_symbol_period_tlv));
            memset(&error_frame_period_tlv, 0, sizeof(error_frame_period_tlv));
            memset(&remote_error_frame_period_tlv, 0, sizeof(remote_error_frame_period_tlv));
            memset(&error_frame_tlv, 0, sizeof(error_frame_tlv));
            memset(&remote_error_frame_tlv, 0, sizeof(remote_error_frame_tlv));

            rc = eth_link_oam_client_port_remote_seq_num_get(isid, pit.iport, &seq_num);
            if (rc != VTSS_RC_OK) {
                err = "Error in Retrieving Sequence Number";
                break;
            }

            rc = eth_link_oam_client_port_symbol_period_error_info_get(isid, pit.iport,
                                                                       &error_symbol_period_tlv,
                                                                       &remote_error_symbol_period_tlv);
            if (rc != VTSS_RC_OK) {
                err = "Error in Retrieving Symbol Period Info";
                break;
            }

            rc = eth_link_oam_client_port_frame_error_info_get(isid, pit.iport,
                                                               &error_frame_tlv,
                                                               &remote_error_frame_tlv);

            if (rc != VTSS_RC_OK) {
                err = "Error in Retrieving Frame Error Info";
                break;
            }
            rc = eth_link_oam_client_port_frame_period_error_info_get(isid, pit.iport,
                                                                      &error_frame_period_tlv,
                                                                      &remote_error_frame_period_tlv);

            if (rc != VTSS_RC_OK) {
                err = "Error in Retrieving Frame Period Info";
                break;
            }
            rc = eth_link_oam_client_port_error_frame_secs_summary_info_get(isid, pit.iport, &local_secs_info, &remote_secs_info);

            if (rc != VTSS_RC_OK) {
                err = "Error in Retrieving Frame Seconds Summary Info";
                break;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%llu/%llu/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%llu/%llu/%u/%u/%u/%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                          pit.uport,
                          seq_num,
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(error_frame_tlv.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_error_frame_tlv.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(error_frame_tlv.error_frame_window),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_error_frame_tlv.error_frame_window),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_tlv.error_frame_threshold),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_tlv.error_frame_threshold),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_tlv.error_frames),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_tlv.error_frames),
                          vtss_eth_link_oam_swap64_from_bytes(error_frame_tlv.error_running_total),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_frame_tlv.error_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_tlv.event_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_tlv.event_running_total),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(error_frame_period_tlv.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_error_frame_period_tlv.event_time_stamp),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_period_tlv.error_frame_period_window),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_period_tlv.error_frame_period_window),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_period_tlv.error_frame_threshold),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_period_tlv.error_frame_threshold),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_period_tlv.error_frames),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_period_tlv.error_frames),
                          vtss_eth_link_oam_swap64_from_bytes(error_frame_period_tlv.error_running_total),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_frame_period_tlv.error_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_frame_period_tlv.event_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_frame_period_tlv.event_running_total),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(error_symbol_period_tlv.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_error_symbol_period_tlv.event_time_stamp),
                          vtss_eth_link_oam_swap64_from_bytes(error_symbol_period_tlv.error_symbol_window),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_symbol_period_tlv.error_symbol_window),
                          vtss_eth_link_oam_swap64_from_bytes(error_symbol_period_tlv.error_symbol_threshold),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_symbol_period_tlv.error_symbol_threshold),
                          vtss_eth_link_oam_swap64_from_bytes(error_symbol_period_tlv.error_symbols),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_symbol_period_tlv.error_symbols),
                          vtss_eth_link_oam_swap64_from_bytes(error_symbol_period_tlv.error_running_total),
                          vtss_eth_link_oam_swap64_from_bytes(remote_error_symbol_period_tlv.error_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(error_symbol_period_tlv.event_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_error_symbol_period_tlv.event_running_total),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(local_secs_info.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_secs_info.event_time_stamp),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(local_secs_info.secs_summary_window),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_secs_info.secs_summary_window),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(local_secs_info.secs_summary_threshold),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_secs_info.secs_summary_threshold),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(local_secs_info.secs_summary_events),
                          (u16)vtss_eth_link_oam_ntohs_from_bytes(remote_secs_info.secs_summary_events),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(local_secs_info.error_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_secs_info.error_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(local_secs_info.event_running_total),
                          (u32)vtss_eth_link_oam_ntohl_from_bytes(remote_secs_info.event_running_total));
            if (ct <= 0) {
                T_D("Failed to do snprintf\n");
            }
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }
    //cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    if (err != NULL) {
        send_custom_error(p, "OAM Link Error", err, strlen(err));
    }

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/
static size_t loam_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{

    char buff[LOAM_WEB_BUF_LEN];
    (void) snprintf(buff, sizeof(buff),
                    "var configLoamErrFrameWindowMin = %d;\n"
                    "var configLoamErrFrameWindowMax = %d;\n"
                    "var configLoamErrFrameThresholdMin = %d;\n"
                    "var configLoamErrFrameThresholdMax = %u;\n"
                    "var configLoamSymbolFrameWindowMin = %d;\n"
                    "var configLoamSymbolFrameWindowMax = %d;\n"
                    "var configLoamSymbolFrameThresholdMin = %d;\n"
                    "var configLoamSymbolFrameThresholdMax = %u;\n"
                    "var configLoamSecsSummaryWindowMin = %d;\n"
                    "var configLoamSecsSummaryWindowMax = %d;\n"
                    "var configLoamSecsSummaryThresholdMin = %d;\n"
                    "var configLoamSecsSummaryThresholdMax = %u;\n",
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MIN_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MAX_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MIN_ERROR_THRESHOLD,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MAX_ERROR_THRESHOLD,
                    VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MIN_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MAX_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MIN_ERROR_THRESHOLD,
                    VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MAX_ERROR_THRESHOLD,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MIN_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MAX_ERROR_WINDOW,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MIN_ERROR_THRESHOLD,
                    VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MAX_ERROR_THRESHOLD
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib_config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(loam_lib_config_js);


CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_link_oam_statistics, "/stat/link_oam_statistics", handler_stat_link_oam_statistics);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_oam_ports, "/config/oam_ports", handler_config_oam_ports);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_link_oam_events, "/config/link_oam_events", handler_config_link_oam_events);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_oam_port_status, "/stat/oam_port_status", handler_stat_oam_port_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_diag_mib_retrieve, "/diag/mib_retrieve", handler_diag_mib_retrieve);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_oam_link_status, "/stat/link_oam_link_status", handler_stat_link_oam_link_status);


