/*

 Vitesse sFlow software.

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
#include "sflow_api.h"
#include "msg_api.h"
#include "port_api.h"    /* For port_iter_t */
#include "mgmt_api.h"    /* For mgmt_txt2ipv4_ext() and mgmt_txt2ipv6() */
#include "sflow_trace.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#if SFLOW_RECEIVER_CNT != 1 || SFLOW_INSTANCE_CNT != 1
// If a customer wants it, we can make the Web-pages more sophisticated,
// but for simplicity, we keep it to one receiver and one per-port instance for now.
#error "The sFlow Web-handler only supports one receiver and one port-instance"
#endif

// Well, SFLOW_web_err_msg is indeed not protected, but I find it very unlikely
// that the error message from one Web-browser instance gets forwarded to another
// Web-browser instance.
/*lint -esym(459,SFLOW_web_err_msg)*/
static const char *SFLOW_web_err_msg = "";

/******************************************************************************/
// SFLOW_web_err_msg_compose()
/******************************************************************************/
static void SFLOW_web_err_msg_compose(CYG_HTTPD_STATE *p, int errors, vtss_rc rc)
{
    if (errors) {
        // There are two types of errors. Those where a form variable was invalid,
        // and those whee an sflow_mgmt_XXX() function failed.
        if (rc == VTSS_RC_OK) {
            SFLOW_web_err_msg = "Invalid form";
        } else {
            SFLOW_web_err_msg = error_txt(rc);
        }
    } else {
        SFLOW_web_err_msg = "";
    }
}

/******************************************************************************/
// SFLOW_web_handler_config()
/******************************************************************************/
static cyg_int32 SFLOW_web_handler_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t       isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    sflow_rcvr_t      rcvr_new_cfg, rcvr_cur_cfg;
    sflow_rcvr_info_t rcvr_info;
    sflow_agent_t     agent_cfg;
    sflow_fs_t        fs;
    sflow_cp_t        cp;
    vtss_rc           rc;
    const char        *inbuf;
    int               var_value, timeout_orig, timeout_new;
    BOOL              timeout_chg = FALSE;
    int               cnt, errors = 0, release;
    port_iter_t       pit;
    size_t            len;
    char              encoded_hostname[3 * SFLOW_HOSTNAME_LEN];
    i8                ipstr[40];

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SFLOW)) {
        return -1;
    }
#endif

    // Special case: Releasing the current owner is a GET operation with
    // URL containing the string "clear=1". Check for that first.
    if (p->method == CYG_HTTPD_METHOD_GET) {
        if (cyg_httpd_form_varable_int(p, "clear", &release) && release == 1) {
            memset(&rcvr_new_cfg, 0, sizeof(rcvr_new_cfg));
            if ((rc = sflow_mgmt_rcvr_cfg_set(SFLOW_RECEIVER_IDX, &rcvr_new_cfg)) != VTSS_RC_OK) {
                SFLOW_web_err_msg_compose(p, 1, rc);
            }
        }
    }

    if ((rc = sflow_mgmt_rcvr_cfg_get(SFLOW_RECEIVER_IDX, &rcvr_cur_cfg, &rcvr_info)) != VTSS_RC_OK) {
        errors++;
    }

    if ((rc = sflow_mgmt_agent_cfg_get(&agent_cfg)) != VTSS_RC_OK) {
        errors++;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {

        // When we get here, we know that either 1) we are the current owner of the receiver
        // or 2) the receiver is not owned. Either way, we will become the new owner.
        rcvr_new_cfg = rcvr_cur_cfg;

        inbuf = cyg_httpd_form_variable_str_fmt(p, &len, "hostname");
        if (inbuf) {
            if (!cgi_unescape(inbuf, (char *)rcvr_new_cfg.hostname, len, sizeof(rcvr_new_cfg.hostname))) {
                errors++;
            }
        } else {
            errors++;
        }

        if (cyg_httpd_form_varable_int(p, "udp_port", &var_value)) {
            rcvr_new_cfg.udp_port = var_value;
        } else {
            errors++;
        }

        if (cyg_httpd_form_varable_long_int(p, "max_datagram_size", &var_value)) {
            rcvr_new_cfg.max_datagram_size = var_value;
        } else {
            errors++;
        }

        inbuf = cyg_httpd_form_variable_str_fmt(p, &len, "agent_ip_addr");
        if (inbuf && cgi_unescape(inbuf, ipstr, len, sizeof(ipstr))) {
            if (mgmt_txt2ipv4_ext(ipstr, &agent_cfg.agent_ip_addr.addr.ipv4, 0, FALSE, TRUE, TRUE) == VTSS_RC_OK) {
                agent_cfg.agent_ip_addr.type = VTSS_IP_TYPE_IPV4;
            }
#ifdef VTSS_SW_OPTION_IPV6
            else if (mgmt_txt2ipv6(ipstr, &agent_cfg.agent_ip_addr.addr.ipv6) == VTSS_RC_OK) {
                agent_cfg.agent_ip_addr.type = VTSS_IP_TYPE_IPV6;
            }
#endif
            else {
                rc = SFLOW_ERROR_AGENT_IP;
                errors++;
            }
        } else {
            errors++;
        }

        // If the receiver config hasn't changed, we can continue right away
        // with setting up flow and counter samplers.
        // If the receiver config *has* changed, we should halt current receiver
        // (if any), apply flow and counter samplers, and finally set-up the
        // new receiver.

        // It's not that easy to figure out whether the config has changed, because
        // it contains a highly volatile parameter - the timeout. Luckily the JavaScript
        // has co-operated and saved a copy of the value it received last time in a
        // hidden field. Let's go get both that and the new value
        if (!cyg_httpd_form_varable_int(p, "timeout_orig", &timeout_orig)) {
            errors++;
        }

        if (!cyg_httpd_form_varable_int(p, "timeout", &timeout_new)) {
            errors++;
        }

        if (timeout_orig != timeout_new) {
            // User has changed the timeout. Remember that for a subsequent receiver configuration.
            timeout_chg = TRUE;
        }

        if (!errors && memcmp(&rcvr_cur_cfg, &rcvr_new_cfg, sizeof(rcvr_cur_cfg)) != 0) {
            // It's not just a simple timeout change, so we gotta set a new owner - us.
            strcpy(rcvr_new_cfg.owner, SFLOW_OWNER_LOCAL_MANAGEMENT_STRING);

            rcvr_new_cfg.timeout = timeout_new;

            // Receiver config has changed (not just possibly the current timeout).
            if ((rc = sflow_mgmt_rcvr_cfg_set(SFLOW_RECEIVER_IDX, &rcvr_new_cfg)) != VTSS_RC_OK) {
                errors++;
            }
            // Prevent the function from being called again below.
            timeout_chg = FALSE;
        }

        // Time for the per-port properties.
        if (!errors && (rc = port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL)) != VTSS_RC_OK) {
            errors++;
        }
        while (!errors && port_iter_getnext(&pit)) {
            if ((rc = sflow_mgmt_flow_sampler_cfg_get(  isid, pit.iport, SFLOW_INSTANCE_IDX, &fs)) != VTSS_RC_OK ||
                (rc = sflow_mgmt_counter_poller_cfg_get(isid, pit.iport, SFLOW_INSTANCE_IDX, &cp)) != VTSS_RC_OK) {
                errors++;
                break;
            }

            fs.enabled = cyg_httpd_form_variable_check_fmt(p, "fs_enable_%u", pit.uport);
            if (fs.enabled) {
                fs.receiver = SFLOW_RECEIVER_IDX;
                if (!cyg_httpd_form_variable_long_int_fmt(p, &fs.sampling_rate, "fs_sampling_rate_%u", pit.uport)) {
                    errors++;
                }
                if (!cyg_httpd_form_variable_long_int_fmt(p, &fs.max_header_size, "fs_max_header_size_%u", pit.uport)) {
                    errors++;
                }
            }

            cp.enabled = cyg_httpd_form_variable_check_fmt(p, "cp_enable_%u", pit.uport);
            if (cp.enabled) {
                cp.receiver = SFLOW_RECEIVER_IDX;
                if (!cyg_httpd_form_variable_long_int_fmt(p, &cp.interval, "cp_interval_%u", pit.uport)) {
                    errors++;
                }
            }

            if ((rc = sflow_mgmt_flow_sampler_cfg_set(  isid, pit.iport, SFLOW_INSTANCE_IDX, &fs)) != VTSS_RC_OK ||
                (rc = sflow_mgmt_counter_poller_cfg_set(isid, pit.iport, SFLOW_INSTANCE_IDX, &cp)) != VTSS_RC_OK) {
                errors++;
            }
        }

        if (!errors && timeout_chg) {
            rcvr_new_cfg.timeout = timeout_new;
            if ((rc = sflow_mgmt_rcvr_cfg_set(SFLOW_RECEIVER_IDX, &rcvr_new_cfg)) != VTSS_RC_OK) {
                errors++;
            }
        }

        if (!errors) {
            if ((rc = sflow_mgmt_agent_cfg_set(&agent_cfg)) != VTSS_RC_OK) {
                errors++;
            }
        }

        SFLOW_web_err_msg_compose(p, errors, rc);
        redirect(p, "/sflow.htm");
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        char encoded_owner[3 * SFLOW_OWNER_LEN];
        char encoded_ipstr[3 * sizeof(ipstr)];
        BOOL allow_changes;

        cyg_httpd_start_chunked("html");

        // Format: err_msg#agent_ip_addr#[RcvrConfig]#[PortConfigs]
        //         [RcvrConfig]  = owner/allow_changes/timeout/max_datagram_size/hostname/udp_port/datagram_version
        //         [PortConfigs] = [PortConfig1]#[PortConfig2]#...#[PortConfigN]
        //         [PortConfig]  = port_number/fs_enabled/fs_sampling_rate/fs_max_header_size/cp_enabled/cp_interval
        (void)misc_ip_txt(&agent_cfg.agent_ip_addr, ipstr);
        (void)cgi_escape(ipstr, encoded_ipstr);
        (void)cgi_escape((char *)rcvr_cur_cfg.owner,    encoded_owner);
        (void)cgi_escape((char *)rcvr_cur_cfg.hostname, encoded_hostname);
        allow_changes = strcmp(rcvr_cur_cfg.owner, SFLOW_OWNER_LOCAL_MANAGEMENT_STRING) == 0 || rcvr_cur_cfg.owner[0] == '\0';

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s#%s#%s/%d/%u/%u/%s/%u/%d",
                       SFLOW_web_err_msg,
                       encoded_ipstr,
                       encoded_owner,
                       allow_changes,
                       rcvr_info.timeout_left, // Not the previously configured timeout, but what is left now.
                       rcvr_cur_cfg.max_datagram_size,
                       encoded_hostname,
                       rcvr_cur_cfg.udp_port,
                       rcvr_cur_cfg.datagram_version);
        cyg_httpd_write_chunked(p->outbuffer, cnt);

        SFLOW_web_err_msg = "";

        // Time for the per-port properties.
        if (!errors && port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL) != VTSS_RC_OK) {
            errors++;
        }
        while (!errors && port_iter_getnext(&pit)) {
            if ((sflow_mgmt_flow_sampler_cfg_get(  isid, pit.iport, SFLOW_INSTANCE_IDX, &fs)) != VTSS_RC_OK ||
                (sflow_mgmt_counter_poller_cfg_get(isid, pit.iport, SFLOW_INSTANCE_IDX, &cp)) != VTSS_RC_OK) {
                break;
            }

            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%u/%d/%u/%u/%d/%u",
                           pit.uport,
                           fs.enabled ? 1 : 0,
                           fs.sampling_rate,
                           fs.max_header_size,
                           cp.enabled ? 1 : 0,
                           cp.interval);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// SFLOW_web_handler_status()
/******************************************************************************/
static cyg_int32 SFLOW_web_handler_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t               isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    sflow_rcvr_t              rcvr_cfg;
    sflow_rcvr_info_t         rcvr_info;
    sflow_rcvr_statistics_t   rcvr_statistics;
    sflow_switch_statistics_t switch_statistics;
    char                      encoded_owner   [3 * SFLOW_OWNER_LEN];
    char                      encoded_hostname[3 * SFLOW_HOSTNAME_LEN];
    port_iter_t               pit;
    vtss_rc                   rc;
    BOOL                      clear_rcvr = FALSE;  // By default, don't clear receiver counters
    BOOL                      clear_ports = FALSE; // By default, don't clear port counters
    int                       cnt, clear;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SFLOW)) {
        return -1;
    }
#endif

    // This function is also used to clear receiver counters, when the URL contains the string "clear=1"
    if (cyg_httpd_form_varable_int(p, "clear", &clear) && clear == 1) {
        clear_rcvr = TRUE;
    }

    // This function is also used to clear port counters, when the URL contains the string "port=0"
    if (cyg_httpd_form_varable_int(p, "port", &clear) && clear == 0) {
        clear_ports = TRUE;
    }

    if ((rc = sflow_mgmt_rcvr_cfg_get(SFLOW_RECEIVER_IDX, &rcvr_cfg, &rcvr_info)) != VTSS_RC_OK) {
        T_E("rc = %s", error_txt(rc));
    }

    if ((rc = sflow_mgmt_rcvr_statistics_get(SFLOW_RECEIVER_IDX, &rcvr_statistics, clear_rcvr)) != VTSS_RC_OK) {
        T_E("rc = %s", error_txt(rc));
    }

    cyg_httpd_start_chunked("html");

    // Format: [RcvrStatus]#[PortStatuses]
    //         [RcvrStatus]   = owner/timeout/hostname/ok_datagram_cnt/err_datagram_cnt/flow_sample_cnt/counter_sample_cnt
    //         [PortStatuses] = [PortStatus1]#[PortStatus2]#...#[PortStatusN]
    //         [PortStatus]   = port_number/rx_flow_sample_cnt/tx_flow_sample_cnt/counter_sample_cnt

    (void)cgi_escape((char *)rcvr_cfg.owner,    encoded_owner);
    (void)cgi_escape((char *)rcvr_cfg.hostname, encoded_hostname);
    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%s/%llu/%llu/%llu/%llu",
                   encoded_owner,
                   rcvr_info.timeout_left, // Not the previously configured timeout, but what is left now.
                   encoded_hostname,
                   rcvr_statistics.dgrams_ok,
                   rcvr_statistics.dgrams_err,
                   rcvr_statistics.fs,
                   rcvr_statistics.cp);
    cyg_httpd_write_chunked(p->outbuffer, cnt);

    // Per-port statistics
    // First clear if requested to. There's no API to do that in the same go as getting statistics.
    if (clear_ports) {
        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if ((rc = sflow_mgmt_instance_statistics_clear(isid, pit.iport, SFLOW_INSTANCE_IDX)) != VTSS_RC_OK) {
                T_E("rc = %s", error_txt(rc));
            }
        }
        // Since we couldn't clear and get statistics atomically, we cheat and reset the statistics here.
        memset(&switch_statistics, 0, sizeof(switch_statistics));
    } else {
        if ((rc = sflow_mgmt_switch_statistics_get(isid, &switch_statistics)) != VTSS_RC_OK) {
            T_E("rc = %s", error_txt(rc));
        }
    }

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%u/%llu/%llu/%llu",
                       pit.uport,
                       switch_statistics.port[pit.iport].fs_rx[SFLOW_INSTANCE_IDX - 1],
                       switch_statistics.port[pit.iport].fs_tx[SFLOW_INSTANCE_IDX - 1],
                       switch_statistics.port[pit.iport].cp   [SFLOW_INSTANCE_IDX - 1]);
        cyg_httpd_write_chunked(p->outbuffer, cnt);
    }
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_sflow, "/config/sflow",      SFLOW_web_handler_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_sflow, "/stat/sflow_status", SFLOW_web_handler_status);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

