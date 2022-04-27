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

#include "cli.h"
#include "cli_api.h"
#include "sflow_api.h"
#include "misc_api.h"
#include "mgmt_api.h"  /* For mgmt_txt2ipv6() */

#if SFLOW_RECEIVER_CNT > 1 && SFLOW_INSTANCE_CNT == 1
// The opposite case where the number of instances is greater than the number of
// receivers is already covered in sflow_api.h
#error "sFlow CLI only supports SFLOW_RECEIVER_CNT == 1, when SFLOW_INSTANCE_CNT == 1"
#endif

/******************************************************************************/
// This module's special parameters
/******************************************************************************/
typedef struct {
    vtss_ip_addr_t agent_ip_addr;

    u32  rcvr_idx;
    BOOL rcvr_all; // Only used in 'sFlow Receiver' commands.
    u32  rcvr_timeout;
    BOOL rcvr_timeout_specified;
    u8   rcvr_hostname[SFLOW_HOSTNAME_LEN];
    BOOL rcvr_hostname_specified;
    u16  rcvr_udp_port;
    BOOL rcvr_udp_port_specified;
    u32  rcvr_datagram_size;
    BOOL rcvr_datagram_size_specified;
    BOOL rcvr_release;

    BOOL port_specified;
    BOOL inst_all;
    u16  inst;

    u32  fs_sampling_rate;
    BOOL fs_sampling_rate_specified;
    u32  fs_max_header_size;
    BOOL fs_max_header_size_specified;

    u32  cp_interval;
    BOOL cp_interval_specified;
} sflow_cli_req_t;

/******************************************************************************/
// SFLOW_cli_default_set()
/******************************************************************************/
static void SFLOW_cli_default_set(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    // sflow_req already memset() to zeros by CLI module.
#if SFLOW_RECEIVER_CNT > 1
    sflow_req->rcvr_all = TRUE;
#else
    sflow_req->rcvr_idx = SFLOW_RECEIVER_IDX;
#endif

#if SFLOW_INSTANCE_CNT > 1
    sflow_req->inst_all = TRUE;
#else
    sflow_req->inst = SFLOW_INSTANCE_IDX;
#endif
}

/******************************************************************************/
// SFLOW_cli_error()
/******************************************************************************/
static void SFLOW_cli_error(vtss_rc rc)
{
    cli_printf("Error: %s\n", error_txt(rc));
}

/******************************************************************************/
// SFLOW_cli_init()
/******************************************************************************/
void sflow_cli_init(void)
{
    // Register the size required for the sFlow CLI request structure
    cli_req_size_register(sizeof(sflow_cli_req_t));
}

/******************************************************************************/
// SFLOW_cli_display_cfg()
// Display current configuration.
/******************************************************************************/
static void SFLOW_cli_display_cfg(cli_req_t *req, BOOL show_agent, BOOL show_rcvr, BOOL show_fs, BOOL show_cp)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    switch_iter_t   sit;
    vtss_rc         rc;

    if (show_agent) {
        sflow_agent_t agent;
        char          buf[50];

        cli_header("Agent Configuration", 1);

        if ((rc = sflow_mgmt_agent_cfg_get(&agent)) != VTSS_RC_OK) {
            SFLOW_cli_error(rc);
            return;
        }

        cli_printf("Agent Address: %s\n", misc_ip_txt(&agent.agent_ip_addr, buf));
    }

    if (show_rcvr) {
        sflow_rcvr_t      rcvr;
        sflow_rcvr_info_t info;
        BOOL              res_eq_host;
        int               rcvr_idx, rcvr_idx_min, rcvr_idx_max, rcvr_enabled_cnt = 0;

        if (sflow_req->rcvr_all) {
            rcvr_idx_min = 1;
            rcvr_idx_max = SFLOW_RECEIVER_CNT;
        } else {
            rcvr_idx_min = rcvr_idx_max = sflow_req->rcvr_idx;
        }

        cli_header("Receiver Configuration", 1);

        // First print the receivers.
        for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
            if ((rc = sflow_mgmt_rcvr_cfg_get(rcvr_idx, &rcvr, &info)) != VTSS_RC_OK) {
                SFLOW_cli_error(rc);
                return;
            }

            // req_eq_host is TRUE if the resolved IP address equals the hostname the user has input.
            res_eq_host = strcmp((char *)rcvr.hostname, info.ip_addr_str) == 0 ? TRUE : FALSE;

            // Unfortunately, it's impractical to have a table header and one row per receiver, because
            // the IP address may be up to 50 chars long (IPv6) , and the owner string up to 127 chars.
#if SFLOW_RECEIVER_CNT > 1
            cli_printf("Receiver ID  : %d\n", rcvr_idx);
#endif
            cli_printf("Owner        : %s\n", rcvr.owner[0] == '\0' ? "<none>" : rcvr.owner);
            if (res_eq_host) {
                cli_printf("Receiver     : %s\n", rcvr.hostname);
            } else {
                cli_printf("Receiver     : %s (%s)\n", rcvr.hostname, info.ip_addr_str);
            }
            cli_printf("UDP Port     : %u\n", rcvr.udp_port);
            cli_printf("Max. Datagram: %u bytes\n", rcvr.max_datagram_size);
            cli_printf("Time left    : %u seconds\n\n", info.timeout_left);

            if (rcvr.owner[0] != '\0') {
                rcvr_enabled_cnt++;
            }
        }

        if (rcvr_enabled_cnt == 0) {
            if (show_fs || show_cp) {
                // Don't flood the screen with unusable info.
                cli_printf("No enabled receivers. Skipping displaying per-port info.\n");
            }
            return;
        }
    }

    if (show_fs == FALSE && show_cp == FALSE) {
        return;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        cli_cmd_usid_print(sit.usid, req, 1);

        if (show_fs) {
            port_iter_t pit;
            int         display_cnt = 0;

            cli_header("Flow Sampler Configuration", 1);

            (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
            while (cli_port_iter_getnext(&pit, req)) {
                sflow_fs_t fs;
                u16        inst, inst_min, inst_max;

                if (sflow_req->inst_all) {
                    inst_min = 1;
                    inst_max = SFLOW_INSTANCE_CNT;
                } else {
                    inst_min = inst_max = sflow_req->inst;
                }

                for (inst = inst_min; inst <= inst_max; inst++) {
                    if ((rc = sflow_mgmt_flow_sampler_cfg_get(sit.isid, pit.iport, inst, &fs)) != VTSS_RC_OK) {
                        SFLOW_cli_error(rc);
                        return;
                    }

                    if (sflow_req->port_specified == FALSE && (fs.receiver == 0 || fs.enabled == FALSE)) {
                        // Skip this port instance, since it's not connected to any receivers, and not
                        // specifically asked to be displayed (if it was, sflow_req->port_specified would be TRUE).
                        continue;
                    }

                    // We cannot use pit.first, because we don't necessarily show all selected ports.
                    if (display_cnt++ == 0) {
#if SFLOW_RECEIVER_CNT > 1
                        cli_table_header("Port  Inst  Rcvr  Sampling Rate  Max Hdr");
#else
                        // This is where you want to change if you want to support receivers == 1 and instances > 1
                        cli_table_header("Port  Sampling Rate  Max Hdr");
#endif
                    }
#if SFLOW_RECEIVER_CNT > 1
                    cli_printf("%4lu  %4d  %4lu  %13lu  %7lu\n", pit.uport, inst, fs.receiver, fs.sampling_rate, fs.max_header_size);
#else
                    // This is where you want to change if you want to support receivers == 1 and instances > 1
                    cli_printf("%4u  %13u  %7u\n", pit.uport, fs.sampling_rate, fs.max_header_size);
#endif
                }
            }
            if (display_cnt == 0) {
#if SFLOW_RECEIVER_CNT > 1
                cli_printf("No active flow sample receivers.\n");
#else
                cli_printf("No active flow samplers.\n");
#endif
            }
        }

        if (show_cp) {
            port_iter_t pit;
            int         display_cnt = 0;

            cli_header("Counter Poller Configuration", 1);

            (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
            while (cli_port_iter_getnext(&pit, req)) {
                sflow_cp_t cp;
                u16        inst, inst_min, inst_max;

                if (sflow_req->inst_all) {
                    inst_min = 1;
                    inst_max = SFLOW_INSTANCE_CNT;
                } else {
                    inst_min = inst_max = sflow_req->inst;
                }

                for (inst = inst_min; inst <= inst_max; inst++) {
                    if ((rc = sflow_mgmt_counter_poller_cfg_get(sit.isid, pit.iport, inst, &cp)) != VTSS_RC_OK) {
                        SFLOW_cli_error(rc);
                        return;
                    }

                    if (sflow_req->port_specified == FALSE && (cp.receiver == 0 || cp.enabled == FALSE)) {
                        // Skip this port instance, since it's not connected to any receivers, and not
                        // specifically asked to be displayed (if it was, sflow_req->port_specified would be TRUE).
                        continue;
                    }

                    if (display_cnt++ == 0) {
#if SFLOW_RECEIVER_CNT > 1
                        // We cannot use pit.first, because we don't necessarily show all selected ports.
                        cli_table_header("Port  Inst  Rcvr  Interval");
#else
                        // This is where you want to change if you want to support receivers == 1 and instances > 1
                        cli_table_header("Port  Interval");
#endif
                    }
#if SFLOW_RECEIVER_CNT > 1
                    cli_printf("%4lu  %4d  %4lu  %8lu\n", pit.uport, inst, cp.receiver, cp.interval);
#else
                    // This is where you want to change if you want to support receivers == 1 and instances > 1
                    cli_printf("%4u  %8u\n", pit.uport, cp.interval);
#endif
                }
            }
            if (display_cnt == 0) {
#if SFLOW_RECEIVER_CNT > 1
                cli_printf("No active counter polling receivers.\n");
#else
                cli_printf("No active counter pollers.\n");
#endif
            }
        }
    }
}

/******************************************************************************/
// SFLOW_cli_cmd_conf()
// Display current configuration.
/******************************************************************************/
static void SFLOW_cli_cmd_conf(cli_req_t *req)
{
    cli_header("sFlow Configuration", 1);
    SFLOW_cli_display_cfg(req, TRUE, TRUE, TRUE, TRUE);
}

/******************************************************************************/
// SFLOW_cli_cmd_agent()
/******************************************************************************/
static void SFLOW_cli_cmd_agent(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    sflow_agent_t   agent;
    vtss_rc         rc;

    if (!req->set) {
        SFLOW_cli_display_cfg(req, TRUE, FALSE, FALSE, FALSE);
        return;
    }

    if ((rc = sflow_mgmt_agent_cfg_get(&agent)) != VTSS_RC_OK) {
        SFLOW_cli_error(rc);
        return;
    }

    agent.agent_ip_addr = sflow_req->agent_ip_addr;

    if ((rc = sflow_mgmt_agent_cfg_set(&agent)) != VTSS_RC_OK) {
        SFLOW_cli_error(rc);
    }
}

/******************************************************************************/
// SFLOW_cli_cmd_receiver()
// Syntax: "Receiver [<rcvr_idx>] [<timeout>] [<ip_addr_host>] [<udp_port>] [<datagram_size>]",
/******************************************************************************/
static void SFLOW_cli_cmd_receiver(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    sflow_rcvr_t    cur_rcvr, new_rcvr;
    vtss_rc         rc;

    if (req->set && sflow_req->rcvr_all) {
        cli_printf("Error: 'all' keyword in place of <rcvr_idx> is not supported when configuring a receiver.\n");
        return;
    }

    if (!req->set) {
        SFLOW_cli_display_cfg(req, FALSE, TRUE, FALSE, FALSE);
        return;
    }

    if (sflow_req->rcvr_release) {
        memset(&new_rcvr, 0, sizeof(new_rcvr));
        if ((rc = sflow_mgmt_rcvr_cfg_set(sflow_req->rcvr_idx, &new_rcvr)) != VTSS_RC_OK) {
            SFLOW_cli_error(rc);
        }
        // Done.
        return;
    }

    if ((rc = sflow_mgmt_rcvr_cfg_get(sflow_req->rcvr_idx, &cur_rcvr, NULL)) != VTSS_RC_OK) {
        SFLOW_cli_error(rc);
        return;
    }

    new_rcvr = cur_rcvr;

    // If we get here, CLI/Web will be the owner of the receiver.
    strcpy(new_rcvr.owner, SFLOW_OWNER_LOCAL_MANAGEMENT_STRING);

    if (sflow_req->rcvr_hostname_specified) {
        misc_strncpyz((char *)new_rcvr.hostname, (char *)sflow_req->rcvr_hostname, sizeof(new_rcvr.hostname));
    }

    if (sflow_req->rcvr_udp_port_specified) {
        new_rcvr.udp_port = sflow_req->rcvr_udp_port;
    }

    if (sflow_req->rcvr_datagram_size_specified) {
        new_rcvr.max_datagram_size = sflow_req->rcvr_datagram_size;
    }

    if (sflow_req->rcvr_timeout_specified) {
        new_rcvr.timeout = sflow_req->rcvr_timeout;
    }

    // Only call in if we've specified a new timeout or if the configuration has actually changed.
    if (sflow_req->rcvr_timeout_specified || memcmp(&cur_rcvr, &new_rcvr, sizeof(cur_rcvr)) != 0) {
        if ((rc = sflow_mgmt_rcvr_cfg_set(sflow_req->rcvr_idx, &new_rcvr)) != VTSS_RC_OK) {
            SFLOW_cli_error(rc);
        }
    }
}

/******************************************************************************/
// SFLOW_cli_check_params()
/******************************************************************************/
static BOOL SFLOW_cli_check_params(cli_req_t *req, char *str)
{
    sflow_cli_req_t *sflow_req = req->module_req;

    // Don't allow setting configuration if a specific instance is not chosen, unless the receivers is 0 (meaning stop all instances).
    // We do, however, support setting a flow sampler for all ports in one go.
    if (req->set && sflow_req->inst_all && sflow_req->rcvr_idx != 0) {
        cli_printf("Error: Cannot configure a %s to a specific receiver on all instances of a port.\n", str);
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************/
// SFLOW_cli_cmd_flow_sampler()
// Syntax: "FlowSampler [<port_list>] [<instance>] [<rcvr_idx>] [<sampling_rate>] [<max_hdr_size>]"
/******************************************************************************/
static void SFLOW_cli_cmd_flow_sampler(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    switch_iter_t   sit;
    u16             inst, inst_min, inst_max;
    BOOL            note_shown = FALSE;

    if (!SFLOW_cli_check_params(req, "flow sampler")) {
        return;
    }

    if (!req->set) {
        SFLOW_cli_display_cfg(req, FALSE, FALSE, TRUE, FALSE);
        return;
    }

    if (sflow_req->inst_all) {
        // Can only get here when sflow_req->rcvr_idx == 0.
        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;
    } else {
        inst_min = inst_max = sflow_req->inst;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);

        while (cli_port_iter_getnext(&pit, req)) {
            sflow_fs_t fs;
            vtss_rc    rc;

            for (inst = inst_min; inst <= inst_max; inst++) {
                if ((rc = sflow_mgmt_flow_sampler_cfg_get(sit.isid, pit.iport, inst, &fs)) != VTSS_RC_OK) {
                    SFLOW_cli_error(rc);
                    return;
                }

                fs.receiver = sflow_req->rcvr_idx;
                // In CLI, we control the enabled state through the rcvr_idx.
                // In Web, it makes more sense to keep the receiver index as is and control
                // the enabled/disabled state independently.
                // In both cases, we allow setting up flow samplers before configuring a receiver.
                fs.enabled  = sflow_req->rcvr_idx != 0;
                if (sflow_req->fs_sampling_rate_specified) {
                    fs.sampling_rate = sflow_req->fs_sampling_rate;
                }

                if (sflow_req->fs_max_header_size_specified) {
                    fs.max_header_size = sflow_req->fs_max_header_size;
                }

                if ((rc = sflow_mgmt_flow_sampler_cfg_set(sit.isid, pit.iport, inst, &fs)) != VTSS_RC_OK) {
                    SFLOW_cli_error(rc);
                    return;
                }
                if (!note_shown && sflow_req->fs_sampling_rate_specified && sflow_req->fs_sampling_rate != fs.sampling_rate) {
                    cli_printf("Note: Sampling rate modified from %u to %u to cater for H/W limitations\n", sflow_req->fs_sampling_rate, fs.sampling_rate);
                    note_shown = TRUE;
                }
            }
        }
    }
}

/******************************************************************************/
// SFLOW_cli_cmd_counter_poller()
// Syntax: "CounterPoller [<port_list>] [<instance>] [<rcvr_idx>] [<interval>]",
/******************************************************************************/
static void SFLOW_cli_cmd_counter_poller(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    switch_iter_t   sit;
    u16             inst, inst_min, inst_max;

    if (!SFLOW_cli_check_params(req, "counter poller")) {
        return;
    }

    if (!req->set) {
        SFLOW_cli_display_cfg(req, FALSE, FALSE, FALSE, TRUE);
        return;
    }

    if (sflow_req->inst_all) {
        // Can only get here when sflow_req->rcvr_idx == 0.
        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;
    } else {
        inst_min = inst_max = sflow_req->inst;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);

        while (cli_port_iter_getnext(&pit, req)) {
            sflow_cp_t cp;
            vtss_rc    rc;

            for (inst = inst_min; inst <= inst_max; inst++) {
                if ((rc = sflow_mgmt_counter_poller_cfg_get(sit.isid, pit.iport, inst, &cp)) != VTSS_RC_OK) {
                    SFLOW_cli_error(rc);
                    return;
                }
                cp.receiver = sflow_req->rcvr_idx;
                // In CLI, we control the enabled state through the rcvr_idx.
                // In Web, it makes more sense to keep the receiver index as is and control
                // the enabled/disabled state independently.
                // In both cases, we allow setting up counter pollers before configuring a receiver.
                if (sflow_req->cp_interval_specified) {
                    cp.interval = sflow_req->cp_interval;
                }
                cp.enabled = sflow_req->rcvr_idx != 0 && cp.interval != 0;

                if ((rc = sflow_mgmt_counter_poller_cfg_set(sit.isid, pit.iport, inst, &cp)) != VTSS_RC_OK) {
                    SFLOW_cli_error(rc);
                    return;
                }
            }
        }
    }
}

/******************************************************************************/
// SFLOW_cli_cmd_statistics_receiver()
// Syntax: "Statistics Receiver [<rcvr_idx>] [clear]
/******************************************************************************/
static void SFLOW_cli_cmd_statistics_receiver(cli_req_t *req)
{
    sflow_cli_req_t         *sflow_req = req->module_req;
    int                     rcvr_idx, rcvr_idx_min, rcvr_idx_max;
    sflow_rcvr_statistics_t rcvr;
    vtss_rc                 rc;

    if (sflow_req->rcvr_all) {
        rcvr_idx_min = 1;
        rcvr_idx_max = SFLOW_RECEIVER_CNT;
    } else {
        rcvr_idx_min = rcvr_idx_max = sflow_req->rcvr_idx;
    }

    if (req->clear) {
        for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
            if ((rc = sflow_mgmt_rcvr_statistics_get(rcvr_idx, NULL, TRUE)) != VTSS_RC_OK) {
                SFLOW_cli_error(rc);
                return;
            }
        }
        return;
    }

    cli_header("Receiver Statistics", 1);

    for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
        if ((rc = sflow_mgmt_rcvr_statistics_get(rcvr_idx, &rcvr, FALSE)) != VTSS_RC_OK) {
            SFLOW_cli_error(rc);
            return;
        }

        if (rcvr_idx == rcvr_idx_min) {
#if SFLOW_RECEIVER_CNT > 1
            cli_table_header("Rcvr  Tx Successes     Tx Errors        Flow Samples     Counter Samples");
#else
            cli_table_header("Tx Successes     Tx Errors        Flow Samples     Counter Samples");
#endif
        }
#if SFLOW_RECEIVER_CNT > 1
        cli_printf("%4d  %15llu  %15llu  %15llu  %15llu\n", rcvr_idx, rcvr.dgrams_ok, rcvr.dgrams_err, rcvr.fs, rcvr.cp);
#else
        cli_printf("%15llu  %15llu  %15llu  %15llu\n", rcvr.dgrams_ok, rcvr.dgrams_err, rcvr.fs, rcvr.cp);
#endif
    }
}

/******************************************************************************/
// SFLOW_cli_cmd_statistics_samplers()
// Syntax: "Statistics Samplers [<port_list>] [<instance>] [clear]"
/******************************************************************************/
static void SFLOW_cli_cmd_statistics_samplers(cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    switch_iter_t   sit;
    u16             inst, inst_min, inst_max;
    vtss_rc         rc;

    if (sflow_req->inst_all) {
        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;
    } else {
        inst_min = inst_max = sflow_req->inst;
    }

    if (req->clear) {
        (void)cli_switch_iter_init(&sit);
        while (cli_switch_iter_getnext(&sit, req)) {
            port_iter_t pit;
            (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
            while (cli_port_iter_getnext(&pit, req)) {
                for (inst = inst_min; inst <= inst_max; inst++) {
                    if ((rc = sflow_mgmt_instance_statistics_clear(sit.isid, pit.iport, inst)) != VTSS_RC_OK) {
                        SFLOW_cli_error(rc);
                        return;
                    }
                }
            }
        }
        return;
    }

#if SFLOW_INSTANCE_CNT > 1
    cli_header("Per-Instance Statistics", 1);
#else
    cli_header("Per-Port Statistics", 1);
#endif

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t               pit;
        sflow_switch_statistics_t statistics;
        int                       display_cnt = 0;

        if ((rc = sflow_mgmt_switch_statistics_get(sit.isid, &statistics)) != VTSS_RC_OK) {
            SFLOW_cli_error(rc);
            return;
        }

        cli_cmd_usid_print(sit.usid, req, 1);

        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            // Notice that we index into the port instances with a 0-based index here.
            for (inst = inst_min - 1; inst <= inst_max - 1; inst++) {
                u64 fs_rx, fs_tx, cp;
                fs_rx = statistics.port[pit.iport].fs_rx[inst];
                fs_tx = statistics.port[pit.iport].fs_tx[inst];
                cp    = statistics.port[pit.iport].cp   [inst];
                if (sflow_req->port_specified == FALSE && fs_rx == 0 && fs_tx == 0 && cp == 0) {
                    // Skip this port instance, since there's no statistics for it, and it's not
                    // specifically asked to be displayed (if it was, sflow_req->port_specified would be TRUE).
                    continue;
                }

                // We cannot use pit.first, because we don't necessarily show all selected ports.
                if (display_cnt++ == 0) {
#if SFLOW_INSTANCE_CNT > 1
                    cli_table_header("Port  Inst  Rx Flow Samples  Tx Flow Samples  Counter Samples");
#else
                    cli_table_header("Port  Rx Flow Samples  Tx Flow Samples  Counter Samples");
#endif
                }
#if SFLOW_INSTANCE_CNT > 1
                cli_printf("%4lu  %4d  %15llu  %15llu  %15llu\n", pit.uport, inst + 1, fs_rx, fs_tx, cp);
#else
                cli_printf("%4u  %15llu  %15llu  %15llu\n", pit.uport, fs_rx, fs_tx, cp);
#endif
            }
        }

        if (display_cnt == 0) {
            cli_printf("No non-zero counters.\n");
        }
    }
}

/******************************************************************************/
//
// SFLOW CLI PARSER FUNCTIONS
//
/******************************************************************************/

#if SFLOW_RECEIVER_CNT > 1
/******************************************************************************/
// SFLOW_cli_parse_rcvr_index_1_based()
// For "sFlow Receiver" commands.
// 'all' keyword is allowed in GET commands.
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_index_1_based(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error      = 0;

    req->parm_parsed = 1;
    if (cli_parse_all(cmd) != 0) {
        // Didn't match 'all'
        u32 val;
        if ((error = cli_parse_ulong(cmd, &val, 1, SFLOW_RECEIVER_CNT)) == 0) {
            sflow_req->rcvr_all = FALSE;
            sflow_req->rcvr_idx = val;
        }
    }
    return error;
}
#endif

#if SFLOW_RECEIVER_CNT > 1
/******************************************************************************/
// SFLOW_cli_parse_rcvr_index_0_based()
// For "sFlow FlowSampler" and "sFlow CounterPoller" commands.
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_index_0_based(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error      = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &sflow_req->rcvr_idx, 0, SFLOW_RECEIVER_CNT);
    return error;
}
#endif

/******************************************************************************/
// SFLOW_cli_parse_rcvr_release()
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_release(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    char            *found     = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "release", 7)) {
            sflow_req->rcvr_release = TRUE;
        }
    }
    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// SFLOW_cli_parse_rcvr_timeout()
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_timeout(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &sflow_req->rcvr_timeout, 0, SFLOW_RECEIVER_TIMEOUT_MAX);
    if (!error) {
        sflow_req->rcvr_timeout_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// SFLOW_cli_parse_port_list()
/******************************************************************************/
static int32_t SFLOW_cli_parse_port_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    error = cli_parm_parse_port_list(cmd, cmd2, stx, cmd_org, req);
    if (!error) {
        // Port explicitly specified. Force printing inactive configuration.
        sflow_req->port_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// SFLOW_cli_parse_agent_ipaddr_str()
/******************************************************************************/
static int32_t SFLOW_cli_parse_agent_ipaddr_str(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;

    if (mgmt_txt2ipv4_ext(cmd, &sflow_req->agent_ip_addr.addr.ipv4, 0, FALSE, TRUE, TRUE) == VTSS_RC_OK) {
        sflow_req->agent_ip_addr.type = VTSS_IP_TYPE_IPV4;
        return 0; // OK
    }

#ifdef VTSS_SW_OPTION_IPV6
    if (mgmt_txt2ipv6(cmd, &sflow_req->agent_ip_addr.addr.ipv6) == VTSS_RC_OK) {
        sflow_req->agent_ip_addr.type = VTSS_IP_TYPE_IPV6;
        return 0; // OK
    }
#endif

    return 1; // Error
}

/******************************************************************************/
// SFLOW_cli_parse_rcvr_ipaddr_str()
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_ipaddr_str(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;

    req->parm_parsed = 1;
    if (cli_parse_quoted_string(cmd, (char *)sflow_req->rcvr_hostname, sizeof(sflow_req->rcvr_hostname))) {
        return 1; // Error.
    }

    // Check for IPv4 and real hostname.
    if (misc_str_is_hostname((char *)sflow_req->rcvr_hostname) == VTSS_OK) {
        sflow_req->rcvr_hostname_specified = TRUE;
        return 0; // OK
    }

    // Check for IPv6 (should be embedded in above check but would cause too many incompatibilities with existing code).
#ifdef VTSS_SW_OPTION_IPV6
    {
        vtss_ipv6_t ipv6;
        if (!mgmt_txt2ipv6((char *)sflow_req->rcvr_hostname, &ipv6)) {
            sflow_req->rcvr_hostname_specified = TRUE;
            return 0; // OK.
        }
    }
#endif
    return 1;
}

/******************************************************************************/
// SFLOW_cli_parse_rcvr_udp_port()
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_udp_port(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;
    ulong           val;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &val, 0, SFLOW_RECEIVER_UDP_PORT_MAX);
    if (!error) {
        sflow_req->rcvr_udp_port = val;
        sflow_req->rcvr_udp_port_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// SFLOW_cli_parse_rcvr_datagram_size()
/******************************************************************************/
static int32_t SFLOW_cli_parse_rcvr_datagram_size(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &sflow_req->rcvr_datagram_size, SFLOW_RECEIVER_DATAGRAM_SIZE_MIN, SFLOW_RECEIVER_DATAGRAM_SIZE_MAX);
    if (!error) {
        sflow_req->rcvr_datagram_size_specified = TRUE;
    }
    return error;
}

#if SFLOW_INSTANCE_CNT > 1
/******************************************************************************/
// SFLOW_cli_parse_instance()
/******************************************************************************/
static int32_t SFLOW_cli_parse_instance(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error      = 0;

    req->parm_parsed = 1;
    if (cli_parse_all(cmd) != 0) {
        u32 val;
        // Doesn't contain 'all' keyword.
        if ((error = cli_parse_ulong(cmd, &val, 1, SFLOW_INSTANCE_CNT)) == 0) {
            sflow_req->inst_all = FALSE;
            sflow_req->inst = val;
        }
    }
    return error;
}
#endif

/******************************************************************************/
// SFLOW_cli_parse_fs_sampling_rate()
/******************************************************************************/
static int32_t SFLOW_cli_parse_fs_sampling_rate(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &sflow_req->fs_sampling_rate, 0, 0xFFFFFFFF); // Any sampling rate is allowed. We auto-adjust it.
    if (!error) {
        sflow_req->fs_sampling_rate_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// SFLOW_cli_parse_fs_max_hdr_size()
/******************************************************************************/
static int32_t SFLOW_cli_parse_fs_max_hdr_size(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    req->parm_parsed = 1;
    // Well, the standard says: Auto-adjust, but I think it's nice to see in CLI that something is wrong. For SNMP, any value is accepted
    // and the sflow API will auto-adjust if out-of-bounds.
    error = cli_parse_ulong(cmd, &sflow_req->fs_max_header_size, SFLOW_FLOW_HEADER_SIZE_MIN, SFLOW_FLOW_HEADER_SIZE_MAX);
    if (!error) {
        sflow_req->fs_max_header_size_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// SFLOW_cli_parse_cp_interval()
/******************************************************************************/
static int32_t SFLOW_cli_parse_cp_interval(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sflow_cli_req_t *sflow_req = req->module_req;
    int32_t         error;

    req->parm_parsed = 1;
    // Well, the standard says: Auto-adjust, but I think it's nice to see in CLI that something is wrong. For SNMP, any value is accepted
    // and the sflow API will auto-adjust if out-of-bounds.
    error = cli_parse_ulong(cmd, &sflow_req->cp_interval, 0, SFLOW_POLLING_INTERVAL_MAX);
    if (!error) {
        sflow_req->cp_interval_specified = TRUE;
    }
    return error;
}

/******************************************************************************/
// Parameter table
/******************************************************************************/
static cli_parm_t SFLOW_cli_parm_table[] = {
    {
        "<ip_addr>",
#ifdef VTSS_SW_OPTION_IPV6
        "IPv4/IPv6 address of receiver.",
#else
        "IPv4 address of receiver.",
#endif
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_agent_ipaddr_str,
        SFLOW_cli_cmd_agent
    },

#if SFLOW_RECEIVER_CNT > 1
    {
        "<rcvr_idx>",
        "Receiver index or 'all' ('all' only valid in show). Valid range is 1 - "  vtss_xstr(SFLOW_RECEIVER_CNT) ".",
        CLI_PARM_FLAG_NONE,
        SFLOW_cli_parse_rcvr_index_1_based,
        SFLOW_cli_cmd_receiver
    },
    {
        "<rcvr_idx>",
        "Receiver index or 'all' ('all' only valid in show). Valid range is 1 - "  vtss_xstr(SFLOW_RECEIVER_CNT) ".",
        CLI_PARM_FLAG_NONE,
        SFLOW_cli_parse_rcvr_index_1_based,
        SFLOW_cli_cmd_statistics_receiver
    },
    {
        "<rcvr_idx>",
#if SFLOW_INSTANCE_CNT > 1
        "Valid receiver index or 0 to turn off receiving for this instance. Valid range is 0 - "  vtss_xstr(SFLOW_RECEIVER_CNT) ".",
#else
        "Valid receiver index or 0 to turn off receiving for this port. Valid range is 0 - "  vtss_xstr(SFLOW_RECEIVER_CNT) ".",
#endif
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_index_0_based,
        NULL // SFLOW_cli_cmd_flow_sampler and SFLOW_cli_cmd_counter_poller
    },
#endif /* SFLOW_RECEIVER_CNT > 1 */
    {
        "release",
        "Release the current owner of the receiver.\n"
        "                 The owner can either be \"<none>\" if the receiver is not currently owned by anyone,\n"
        "                 it can be " vtss_xstr(SFLOW_OWNER_LOCAL_MANAGEMENT_STRING) " if it's currently set up\n"
        "                 by CLI or Web, or it can be anything else if is set-up through SNMP.\n"
        "                 You can only (re-)configure the receiver if it is not currently owned by anyone or owned by CLI or Web.\n"
        "                 If this argument is specified, the remaining arguments are ignored.",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_release,
        SFLOW_cli_cmd_receiver
    },
    {
        "<timeout>",
        "Receiver timeout measured in seconds. The switch\n"
        "                 decrements the timeout once per second, and as long as\n"
        "                 it is non-zero, the receiver receives samples. Once the\n"
        "                 timeout reaches 0, the receiver and all its configuration\n"
        "                 is reset to defaults.\n"
        "                 Valid range is 0 - 2147483647 seconds.",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_timeout,
        SFLOW_cli_cmd_receiver
    },
    {
#ifdef VTSS_SW_OPTION_DNS
        "<ip_addr_host>",
#else
        "<ip_addr>",
#endif
#ifdef VTSS_SW_OPTION_DNS
#ifdef VTSS_SW_OPTION_IPV6
        "IPv4/IPv6 address or a hostname identifying the receiver.",
#else
        "IPv4 address or a hostname identifying the receiver.",
#endif
#else
#ifdef VTSS_SW_OPTION_IPV6
        "IPv4/IPv6 address of receiver.",
#else
        "IPv4 address of receiver.",
#endif
#endif
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_ipaddr_str,
        SFLOW_cli_cmd_receiver
    },
    {
        "<udp_port>",
        "Receiver's UDP port. Valid range is 0 - " vtss_xstr(SFLOW_RECEIVER_UDP_PORT_MAX) ".\n"
        "                 Use 0 to get default port (which is " vtss_xstr(SFLOW_RECEIVER_UDP_PORT_DEFAULT) ").",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_udp_port,
        SFLOW_cli_cmd_receiver
    },
    {
        "<datagram_size>",
        "Maximum datagram size. Valid range is " vtss_xstr(SFLOW_RECEIVER_DATAGRAM_SIZE_MIN) " - " vtss_xstr(SFLOW_RECEIVER_DATAGRAM_SIZE_MAX) " bytes.\n"
        "                 Default is " vtss_xstr(SFLOW_RECEIVER_DATAGRAM_SIZE_DEFAULT) " bytes.",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_rcvr_datagram_size,
        SFLOW_cli_cmd_receiver
    },
    {
        "<port_list>",
        "Port list or 'all'. Default: All ports.",
        CLI_PARM_FLAG_NONE,
        SFLOW_cli_parse_port_list,
        NULL
    },
#if SFLOW_INSTANCE_CNT > 1
    {
        "<instance>",
        "Port instance or 'all' ('all' only valid in show). Valid range is 1 - " vtss_xstr(SFLOW_INSTANCE_CNT) ".",
        CLI_PARM_FLAG_NONE,
        SFLOW_cli_parse_instance,
        NULL
    },
#endif
    {
        "<sampling_rate>",
        "Specifies the statistical sampling rate\n"
        "                 The sample rate is specified as N to sample 1/Nth of the packets\n"
        "                 in the monitored flows. There are no restrictions on the value,\n"
        "                 but the switch will adjust it to the closest possible sampling rate.\n"
        "                 0 disables sampling.",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_fs_sampling_rate,
        SFLOW_cli_cmd_flow_sampler
    },
    {
        "<max_hdr_size>",
        "Specifies the maximum number of bytes to transmit per flow sample.\n"
        "                 Valid range is " vtss_xstr(SFLOW_FLOW_HEADER_SIZE_MIN) " - " vtss_xstr(SFLOW_FLOW_HEADER_SIZE_MAX) " bytes. "
        "Default: " vtss_xstr(SFLOW_FLOW_HEADER_SIZE_DEFAULT) " bytes.",
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_fs_max_hdr_size,
        SFLOW_cli_cmd_flow_sampler
    },
    {
        "<interval>",
        "Polling interval in range " vtss_xstr(SFLOW_POLLING_INTERVAL_MIN) " - " vtss_xstr(SFLOW_POLLING_INTERVAL_MAX) ".\n"
#if SFLOW_INSTANCE_CNT > 1
        "             Set to 0 to release this port instance's resources.",
#else
        "             Set to 0 to release this port's resources.",
#endif
        CLI_PARM_FLAG_SET,
        SFLOW_cli_parse_cp_interval,
        SFLOW_cli_cmd_counter_poller
    },
    {
        "clear",
        "Clear statistics.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL
    },
    {
        NULL, NULL, 0, 0, NULL
    }
};

/******************************************************************************/
// Command table
/******************************************************************************/
enum {
    SFLOW_CLI_PRIO_CONF = 0,
    SFLOW_CLI_PRIO_AGENT,
    SFLOW_CLI_PRIO_RECEIVER,
    SFLOW_CLI_PRIO_FLOW_SAMPLER,
    SFLOW_CLI_PRIO_COUNTER_POLLER,
    SFLOW_CLI_PRIO_SFLOW_STATISTICS_RECEIVER,
    SFLOW_CLI_PRIO_SFLOW_STATISTICS_SAMPLERS,
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_conf) = {
    "sFlow Configuration",
    NULL,
#if SFLOW_INSTANCE_CNT > 1
    "Show global and per instance per port sFlow configuration",
#else
    "Show global and per port sFlow configuration",
#endif
    SFLOW_CLI_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_conf,
    SFLOW_cli_default_set,
    NULL,
    CLI_CMD_FLAG_SYS_CONF
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_agent) = {
    "sFlow Agent",
    "sFlow Agent [<ip_addr>]",
    "Set or show the agent IP address used as agent-address in UDP datagrams. Defaults to IPv4 loopback address",
    SFLOW_CLI_PRIO_AGENT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_agent,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_receiver) = {
#if SFLOW_RECEIVER_CNT > 1
    // Multiple receivers
    "sFlow Receiver [<rcvr_idx>]",
#ifdef VTSS_SW_OPTION_DNS
    "sFlow Receiver [<rcvr_idx>] [release] [<timeout>] [<ip_addr_host>] [<udp_port>] [<datagram_size>]",
#else
    "sFlow Receiver [<rcvr_idx>] [release] [<timeout>] [<ip_addr>] [<udp_port>] [<datagram_size>]",
#endif
    "Set or show the sFlow receiver timeout, IP address, and UDP port for a given receiver ID",
#else
    // Single receiver
    "sFlow Receiver",
#ifdef VTSS_SW_OPTION_DNS
    "sFlow Receiver [release] [<timeout>] [<ip_addr_host>] [<udp_port>] [<datagram_size>]",
#else
    "sFlow Receiver [release] [<timeout>] [<ip_addr>] [<udp_port>] [<datagram_size>]",
#endif
    "Set or show the sFlow receiver timeout, IP address, and UDP port",
#endif
    SFLOW_CLI_PRIO_RECEIVER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_receiver,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_flow_sampler) = {
#if SFLOW_RECEIVER_CNT > 1
    "sFlow FlowSampler [<port_list>] [<instance>]",
    "sFlow FlowSampler [<port_list>] [<instance>] [<rcvr_idx>] [<sampling_rate>] [<max_hdr_size>]",
    "Set or show flow sampler configuration per sampler instance per port.\n"
    "<rcvr_idx> may point to a yet-to-be-configured receiver or be set\n"
    "to 0 to release this resource.\n"
    "When operational,the sampling rate 'N' is rounded off to the nearest possible value",
#else
    // From the #error at the top of this file, we know that the number of instances is 1 if the number of receivers is 1.
    // If we really wanted to support SFLOW_INSTANCE_CNT == 1 and SFLOW_RECEIVER_CNT > 1, this is the place to start working.
    "sFlow FlowSampler [<port_list>]",
    "sFlow FlowSampler [<port_list>] [<sampling_rate>] [<max_hdr_size>]",
    "Set or show flow sampler configuration per port.\n"
    "When operational,the sampling rate 'N' is rounded off to the nearest supported value",
#endif
    SFLOW_CLI_PRIO_FLOW_SAMPLER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_flow_sampler,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_counter_poller) = {
#if SFLOW_RECEIVER_CNT > 1
    "sFlow CounterPoller [<port_list>] [<instance>]",
    "sFlow CounterPoller [<port_list>] [<instance>] [<rcvr_idx>] [<interval>]",
    "Set or show counter polling interval configuration per poller instance per port.\n"
    "<rcvr_idx> may point to a yet-to-be-configured receiver or be set\n"
    "to 0 to release this resource",
#else
    // From the #error at the top of this file, we know that the number of instances is 1 if the number of receivers is 1.
    // If we really wanted to support SFLOW_INSTANCE_CNT == 1 and SFLOW_RECEIVER_CNT > 1, this is the place to start working.
    "sFlow CounterPoller [<port_list>]",
    "sFlow CounterPoller [<port_list>] [<interval>]",
    "Set or show counter polling interval configuration per port",
#endif
    SFLOW_CLI_PRIO_COUNTER_POLLER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_counter_poller,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_statistics_receiver) = {
    "sFlow Statistics Receiver",
#if SFLOW_RECEIVER_CNT > 1
    "sFlow Statistics Receiver [<rcvr_idx>] [clear]",
#else
    "sFlow Statistics Receiver [clear]",
#endif
    "Get or clear receiver statistics",
    SFLOW_CLI_PRIO_SFLOW_STATISTICS_RECEIVER,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_statistics_receiver,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(SFLOW_cli_cmd_statistics_samplers) = {
#if SFLOW_INSTANCE_CNT > 1
    "sFlow Statistics Samplers [<port_list>] [<instance>]",
    "sFlow Statistics Samplers [<port_list>] [<instance>] [clear]",
    "Get or clear per-instance per-port statistics",
#else
    "sFlow Statistics Samplers [<port_list>]",
    "sFlow Statistics Samplers [<port_list>] [clear]",
    "Get or clear per-port statistics",
#endif
    SFLOW_CLI_PRIO_SFLOW_STATISTICS_SAMPLERS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SFLOW,
    SFLOW_cli_cmd_statistics_samplers,
    SFLOW_cli_default_set,
    SFLOW_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

