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
#ifdef VTSS_SW_OPTION_SFLOW

#include "icli_api.h"
#include "icli_porting_util.h"

#include "cli.h" // For cli_port_iter_init

#include "sflow_api.h"
#include "sflow_trace.h"

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

/***************************************************************************/
/*  Type defines                                                           */
/***************************************************************************/
// Enum for selecting which sflow receiver configuration to perform.
typedef enum {
    RELEASE,          // Release - restore to default
    TIMEOUT,          // Configuration timeout.
    COLLECTOR,     // Configuration IP/HOSTNAME
    COLLECTOR_PORT,     // Configuration port
    MAX_DATAGRAM_SIZE // Configuration datagram
} sflow_rcvr_conf_type_t;

// Used to passed the sflow receiver configuration value.
typedef union {
    u32  timeout;
    u8   hostname[SFLOW_HOSTNAME_LEN];
    u16  collector_port;
    u32  max_datagram_size;
    u16  inst;
} sflow_rcvr_conf_value_t;

// Enum for selecting which sflow samplers configuration to perform.
typedef enum {
    SAMPLING_RATE,// Sampler configuration sampling rate
    MAX_HDR_SIZE, // Sampler configuration of Max bytes to transmuit
    ENABLE        // Sampler configuration of enable/disable
} sflow_samplers_conf_type_t;

// Enum for selecting which command to perform.
typedef enum {
    SAMPLER,      // Sampler configuration
    COUNTER_POLL, // counter poll configuration
    STATISTICS_CMD// Statistics printing
} sflow_command_type_t;

// Used to passed the sflow samplers configuration value.
typedef union {
    // Sampler
    u32  sampling_rate;
    u32  max_hdr_size;
    BOOL enable;

    // Counter poller
    u32  poll_interval;

    // Statistics
    BOOL clear; // TRUE to clear statistics
} sflow_conf_value_t;

// Enum for selecting which port status to display.
typedef enum {
    STATISTICS,        // Displaying statistics
    CAPABILITIES,      // Displaying capabilities
    STATUS,            // Displaying status.
} port_status_type_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
// Function for clearing or printing receiver statistics
// In - session_id - For iCLI printing
//      has_clear  - TRUE if statistics shall be cleared
//      print_header - TRUE to print the statistics header
static vtss_rc sflow_icli_statistics_receiver(i32 session_id, u32 rcvr_idx, BOOL has_clear, BOOL print_header)
{
    sflow_rcvr_statistics_t rcvr;
    i8 str_buf[100];
    VTSS_RC(sflow_mgmt_rcvr_statistics_get(rcvr_idx, &rcvr, has_clear)); // Read or clear static depending upon has_clear

    // If the statistics should not be clear the they shall be printed
    if (!has_clear) {
        if (print_header) {

            // Revr header
#if SFLOW_RECEIVER_CNT > 1
            sprintf(str_buf, "%-4s ", Rcvr);
#else
            strcpy(str_buf, "");
#endif
            // Add rest of header
            sprintf(str_buf, "%s%-15s  %-15s  %-15s  %-15s", str_buf, "Tx Successes", "Tx Errors   ", "Flow Samples", "Counter Samples");

            // Print header
            icli_table_header(session_id, str_buf);
        }

        // Print counters
#if SFLOW_RECEIVER_CNT > 1
        ICLI_PRINTF("%4d", rcvr_idx);
#endif

        ICLI_PRINTF("%15llu  %15llu  %15llu  %15llu\n", rcvr.dgrams_ok, rcvr.dgrams_err, rcvr.fs, rcvr.cp);
    }
    return VTSS_RC_OK;
}

// Function for looping through all receivers and calling the statistics function
// In - session_id - For ICLI PRINT
//    - rcvr_idx_list - List of receiver indexes to print
//    - has_clear - TRUE if receiver statistics shall be cleared.
static vtss_rc sflow_icli_statistics_recevier_loop(i32 session_id, icli_range_t *rcvr_idx_list, BOOL has_clear)
{
    u8 rcvr_idx;
    BOOL print_header = TRUE;

    if (rcvr_idx_list == NULL) {
        for (rcvr_idx = 1; rcvr_idx <= SFLOW_RECEIVER_CNT; rcvr_idx++) {
            VTSS_RC(sflow_icli_statistics_receiver(session_id, rcvr_idx, has_clear, print_header));
            print_header = FALSE;
        }
    } else {
        // Print only the indexes specified by user.
        u8 cnt_index;
        for (cnt_index = 0; cnt_index < rcvr_idx_list->u.sr.cnt; cnt_index++) {
            for (rcvr_idx = rcvr_idx_list->u.sr.range[cnt_index].min; rcvr_idx <= rcvr_idx_list->u.sr.range[cnt_index].max; rcvr_idx++) {
                VTSS_RC(sflow_icli_statistics_receiver(session_id, rcvr_idx, has_clear, print_header));
                print_header = FALSE;
            }
        }
    }
    return VTSS_RC_OK;
}

// Function for printing sflow samplers statistics
// In - session_id - For ICLI PRINT
//    - inst - Samplers instance
//    - has_clear - TRUE if receiver statistics shall be cleared.
//    - print_header - TRUE if header shall be printed.
//    - sit - containing switch id
//    - pit - containing port id
static vtss_rc sflow_icli_statistics_samplers(i32 session_id, u32 inst, BOOL has_clear, BOOL print_header, const switch_iter_t *sit, const port_iter_t *pit)
{
    i8 str_buf[100];
    if (has_clear) {
        VTSS_RC(sflow_mgmt_instance_statistics_clear(sit->isid, pit->iport, inst));
    } else {
        if (print_header) {
#if SFLOW_INSTANCE_CNT > 1
            icli_header(session_id, "Per-Instance Statistics", TRUE);
#else
            icli_header(session_id, "Per-Port Statistics", TRUE);
#endif
        }

        sflow_switch_statistics_t statistics;
        u64 fs_rx, fs_tx, cp;
        VTSS_RC(sflow_mgmt_switch_statistics_get(sit->isid, &statistics));
        // Instance is from a user point start from 1, so we subtract 1 here
        fs_rx = statistics.port[pit->iport].fs_rx[inst - 1];
        fs_tx = statistics.port[pit->iport].fs_tx[inst - 1];
        cp    = statistics.port[pit->iport].cp   [inst - 1];

        if (print_header) {
            sprintf(str_buf, "%-23s", "Interface");

#if SFLOW_INSTANCE_CNT > 1
            sprintf(str_buf, "%s %-15s", str_buf, "Inst");
#endif
            sprintf(str_buf, "%s %-16s %-16s %-16s", str_buf, "Rx Flow Samples", "Tx Flow Samples", "Counter Samples");
            icli_table_header(session_id, str_buf);
        }

        ICLI_PRINTF("%-23s ", icli_port_info_txt(sit->usid, pit->uport, str_buf)); // Print interface
#if SFLOW_INSTANCE_CNT > 1
        ICLI_PRINTF("%15llu ", inst);
#endif
        ICLI_PRINTF("%16llu %16llu %16llu\n",  fs_rx, fs_tx, cp);
    }
    return VTSS_RC_OK;
}

// function for printing current sflow agent configuration
// In  - Session_Id - For ICLI print
static vtss_rc sflow_print_agent_conf(i32 session_id)
{
    sflow_agent_t agent;
    char          buf[50];

    icli_header(session_id, "Agent Configuration", TRUE);

    VTSS_RC(sflow_mgmt_agent_cfg_get(&agent));

    ICLI_PRINTF("Agent Address: %s\n", misc_ip_txt(&agent.agent_ip_addr, buf));

    return VTSS_RC_OK;
}

// function for print current sflow  receiver "configuration"
// In  - Session_Id - For ICLI print
// Out - no_rcvr_enabled - Set to TRUE when at least one receiver instance is enabled
static vtss_rc sflow_print_rcvr_conf(i32 session_id, BOOL *no_rcvr_enabled)
{
    sflow_rcvr_t      rcvr;
    sflow_rcvr_info_t info;
    BOOL              res_eq_host;
    int               rcvr_idx, rcvr_idx_min, rcvr_idx_max;

    rcvr_idx_min = 1;
    rcvr_idx_max = SFLOW_RECEIVER_CNT;

    icli_header(session_id, "Receiver Configuration", TRUE);

    // First print the receivers.
    for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
        VTSS_RC(sflow_mgmt_rcvr_cfg_get(rcvr_idx, &rcvr, &info));

        // req_eq_host is TRUE if the resolved IP address equals the hostname the user has input.
        res_eq_host = strcmp((char *)rcvr.hostname, info.ip_addr_str) == 0 ? TRUE : FALSE;

        // Unfortunately, it's impractical to have a table header and one row per receiver, because
        // the IP address may be up to 50 chars long (IPv6) , and the owner string up to 127 chars.
#if SFLOW_RECEIVER_CNT > 1
        ICLI_PRINTF("Collector (Receiver) ID  : %d\n", rcvr_idx);
#endif
        ICLI_PRINTF("Owner        : %s\n", rcvr.owner[0] == '\0' ? "<none>" : rcvr.owner);
        if (res_eq_host) {
            ICLI_PRINTF("Receiver     : %s\n", rcvr.hostname);
        } else {
            ICLI_PRINTF("Receiver     : %s (%s)\n", rcvr.hostname, info.ip_addr_str);
        }
        ICLI_PRINTF("UDP Port     : %u\n", rcvr.udp_port);
        ICLI_PRINTF("Max. Datagram: %u bytes\n", rcvr.max_datagram_size);
        ICLI_PRINTF("Time left    : %u seconds\n\n", info.timeout_left);

        if (rcvr.owner[0] != '\0') {
            *no_rcvr_enabled = FALSE;
            T_NG(TRACE_GRP_ICLI, "rcvr.owner found");
        }
        T_NG(TRACE_GRP_ICLI, "no_rcvr_enabled:%d", *no_rcvr_enabled);
    }
    return VTSS_RC_OK;
}

// function for print current sflow  samplers "configuration"
// In  - Session_Id - For ICLI print
//     - sit - Containing switch information
static vtss_rc sflow_print_samplers_conf(i32 session_id, const switch_iter_t *sit)
{
    i8 str_buf[100];
    port_iter_t pit;
    int         display_cnt = 0;

    icli_header(session_id, "Flow Sampler Configuration", TRUE);

    VTSS_RC(cli_port_iter_init(&pit, sit->isid, PORT_ITER_FLAGS_NORMAL));
    while (icli_port_iter_getnext(&pit, NULL)) {
        sflow_fs_t fs;
        u16        inst, inst_min, inst_max;

        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;

        for (inst = inst_min; inst <= inst_max; inst++) {
            VTSS_RC(sflow_mgmt_flow_sampler_cfg_get(sit->isid, pit.iport, inst, &fs));

            if ((fs.receiver == 0 || fs.enabled == FALSE)) {
                // Skip this port instance, since it's not connected to any receivers.
                continue;
            }

            // We cannot use pit.first, because we don't necessarily show all selected ports.
            if (display_cnt++ == 0) {
                sprintf(str_buf, "%-23s", "Interface");
#if SFLOW_RECEIVER_CNT > 1
                sprintf(str_buf, "%s %-4s", str_buf, "Inst");
#endif
                sprintf(str_buf, "%s %-14s %-8s", str_buf,  "Sampling Rate", "Max Hdr");
                icli_table_header(session_id, str_buf);
            }

            ICLI_PRINTF("%-23s ", icli_port_info_txt(sit->usid, pit.uport, str_buf)); // Print interface
#if SFLOW_RECEIVER_CNT > 1
            ICLI_PRINTF("%4d", inst);
#endif
            ICLI_PRINTF("%14u %8u\n", fs.sampling_rate, fs.max_header_size);
        }
    }
    if (display_cnt == 0) {
#if SFLOW_RECEIVER_CNT > 1
        ICLI_PRINTF("No active flow sample receivers.\n");
#else
        ICLI_PRINTF("No active flow samplers.\n");
#endif
    }
    return VTSS_RC_OK;
}

// function for print current sflow  counter poller "configuration"
// In  - Session_Id - For ICLI print
//     - sit - Containing switch information
static vtss_rc sflow_print_counter_poll_conf(i32 session_id, const switch_iter_t *sit)
{
    int         display_cnt = 0;
    port_iter_t pit;
    i8 str_buf[100];

    icli_header(session_id, "Counter Poller Configuration", 1);

    VTSS_RC(cli_port_iter_init(&pit, sit->isid, PORT_ITER_FLAGS_NORMAL));
    while (icli_port_iter_getnext(&pit, NULL)) {
        sflow_cp_t cp;
        u16        inst, inst_min, inst_max;

        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;

        for (inst = inst_min; inst <= inst_max; inst++) {
            T_IG_PORT(TRACE_GRP_ICLI, pit.iport, "usid:%d, uport:%d, inst:%d", sit->usid, pit.uport, inst);
            VTSS_RC((sflow_mgmt_counter_poller_cfg_get(sit->isid, pit.iport, inst, &cp)));

            if ((cp.receiver == 0 || cp.enabled == FALSE)) {
                // Skip this port instance, since it's not connected to any receivers.
                continue;
            }

            if (display_cnt++ == 0) {
                sprintf(str_buf, "%-23s", "Interface");
#if SFLOW_RECEIVER_CNT > 1
                sprintf(str_buf, "%s %-4s", str_buf, "Inst");
#endif
                sprintf(str_buf, "%s %-8s", str_buf,  "Interval");
                icli_table_header(session_id, str_buf);
            }

            T_IG_PORT(TRACE_GRP_ICLI, pit.iport, "usid:%d, uport:%d", sit->usid, pit.uport);
            ICLI_PRINTF("%-23s ", icli_port_info_txt(sit->usid, pit.uport, str_buf)); // Print interface
#if SFLOW_RECEIVER_CNT > 1
            ICLI_PRINTF("%-4d ", inst);
#endif
            ICLI_PRINTF("%8u\n", cp.interval);

        }
    }
    if (display_cnt == 0) {
#if SFLOW_RECEIVER_CNT > 1
        ICLI_PRINTF("No active counter polling receivers.\n");
#else
        ICLI_PRINTF("No active counter pollers.\n");
#endif
    }
    return VTSS_RC_OK;
}

// function for printing current sflow "configuration"
// In - Session_Id - For ICLI print
static vtss_rc sflow_print_conf(i32 session_id)
{
    BOOL no_rcvr_enabled = TRUE;
    switch_iter_t sit;

    // Print agent configuration
    VTSS_RC(sflow_print_agent_conf(session_id));

    // Print receiver configuration
    VTSS_RC(sflow_print_rcvr_conf(session_id, &no_rcvr_enabled));

    if (no_rcvr_enabled) {
        // Don't flood the screen with unusable info.
        ICLI_PRINTF("No enabled collectors (receivers). Skipping displaying per-port info.\n");
        return VTSS_RC_OK;
    }

    // Loop through all switches in stack and print samplers and counter poll configuration
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    while (icli_switch_iter_getnext(&sit, NULL)) {
        VTSS_RC(sflow_print_samplers_conf(session_id, &sit));
        VTSS_RC(sflow_print_counter_poll_conf(session_id, &sit));
    }

    return VTSS_RC_OK;
}

// Function for configuring receivers
// In - rcvr_idx - Receiver instance
//    - sflow_rcvr_conf_type - Containing which configuration to configuration
//    - sflow_conf_value - Containing the new configuration.
static vtss_rc sflow_icli_receiver_conf(u32 rcvr_idx, sflow_rcvr_conf_type_t sflow_rcvr_conf_type, const sflow_rcvr_conf_value_t *sflow_conf_value)
{
    sflow_rcvr_t   new_rcvr, cur_rcvr;

    VTSS_RC(sflow_mgmt_rcvr_cfg_get(rcvr_idx, &cur_rcvr, NULL)); // Getting current receiver configuration

    new_rcvr = cur_rcvr;

    // Update the configuration in question.
    switch (sflow_rcvr_conf_type) {
    case RELEASE:
        memset(&new_rcvr, 0, sizeof(new_rcvr));
        VTSS_RC(sflow_mgmt_rcvr_cfg_set(rcvr_idx, &new_rcvr));
        return VTSS_RC_OK;
    case TIMEOUT:
        new_rcvr.timeout = sflow_conf_value->timeout;
        break;
    case MAX_DATAGRAM_SIZE:
        new_rcvr.max_datagram_size = sflow_conf_value->max_datagram_size;
        break;
    case COLLECTOR_PORT:
        new_rcvr.udp_port = sflow_conf_value->collector_port;
        break;
    case COLLECTOR:
        misc_strncpyz((char *)new_rcvr.hostname, (char *)sflow_conf_value->hostname, sizeof(new_rcvr.hostname));
        break;
    }

    // If we get here, CLI/Web will be the owner of the receiver.
    strcpy(new_rcvr.owner, SFLOW_OWNER_LOCAL_MANAGEMENT_STRING);

    // Only call in if we've specified a new timeout or if the configuration has actually changed.
    if (sflow_rcvr_conf_type == TIMEOUT || memcmp(&cur_rcvr, &new_rcvr, sizeof(cur_rcvr)) != 0) {
        VTSS_RC(sflow_mgmt_rcvr_cfg_set(rcvr_idx, &new_rcvr));
    }

    return VTSS_RC_OK;
}

// Function for configuring samplers
// In - session_id - For ICLI_PRINTF
// In - inst - samplers instance
//    - sflow_samplers_conf_type - Containing which configuration to configuration
//    - sflow_conf_value - Containing the new configuration.
static vtss_rc sflow_icli_samplers_conf (i32 session_id, u32 inst, sflow_samplers_conf_type_t sflow_samplers_conf_type, const sflow_conf_value_t *sflow_conf_value,   const switch_iter_t *sit, const port_iter_t *pit, BOOL *note_shown)
{
    sflow_fs_t fs;

    VTSS_RC(sflow_mgmt_flow_sampler_cfg_get(sit->isid, pit->iport, inst, &fs));

    T_DG_PORT(TRACE_GRP_ICLI, pit->iport, "fs.receiver:%d, fs.sampling_rate:%d, fs.max_hdr_size:%d, enable:%d, inst:%d", fs.receiver, fs.sampling_rate, fs.max_header_size, fs.enabled, inst);

    switch (sflow_samplers_conf_type) {
    case SAMPLING_RATE:
        fs.sampling_rate = sflow_conf_value->sampling_rate;
        break;
    case MAX_HDR_SIZE:
        fs.max_header_size = sflow_conf_value->max_hdr_size;
        break;
    case ENABLE:
        if (sflow_conf_value->enable) {
            fs.receiver = inst;
        } else {
            fs.receiver = 0; // Setting receiver to 0 means entry is free
        }
        fs.enabled = sflow_conf_value->enable;
        break;
    }

    T_DG_PORT(TRACE_GRP_ICLI, pit->iport, "fs.receiver:%d, fs.sampling_rate:%d, fs.max_hdr_size:%d, enable:%d, inst:%d",
              fs.receiver, fs.sampling_rate, fs.max_header_size, fs.enabled, inst);

    // Update new configuration
    VTSS_RC(sflow_mgmt_flow_sampler_cfg_set(sit->isid, pit->iport, inst, &fs));

    // There are H/W limitations for which sampling rates we can use, printout note if the sample rate was not set to the requested value.
    if (sflow_samplers_conf_type == SAMPLING_RATE) {
        if (!*note_shown && sflow_conf_value->sampling_rate != fs.sampling_rate) {
            ICLI_PRINTF("Note: Sampling rate modified from %u to %u to cater for H/W limitations\n", sflow_conf_value->sampling_rate, fs.sampling_rate);
            *note_shown = TRUE;
        }
    }

    return VTSS_RC_OK;
}

// Function for configuring counter poll
// In - inst - samplers instance
//    - sflow_conf_value - Containing the new configuration.
//    - sit - Containing switch information.
//    - pit - Containing port information.
static vtss_rc sflow_icli_counter_poll_interval_conf(u32 inst, const sflow_conf_value_t *sflow_conf_value, const switch_iter_t *sit, const port_iter_t *pit)
{
    sflow_cp_t cp;

    VTSS_RC(sflow_mgmt_counter_poller_cfg_get(sit->isid, pit->iport, inst, &cp));

    cp.interval = sflow_conf_value->poll_interval;
    cp.enabled = inst != 0 && cp.interval != 0;
    cp.receiver = inst;

    T_IG(TRACE_GRP_ICLI, "cp.interval:%d, cp.enabled:%d, cp.receiver:%d" , cp.interval, cp.enabled, cp.receiver);
    VTSS_RC(sflow_mgmt_counter_poller_cfg_set(sit->isid, pit->iport, inst, &cp));

    return VTSS_RC_OK;
}

// Function for looping through all samplers and calling the command function
// In - Session_Id - For iCLI printing
//    - plist      - Port list of which interfaces to show
//    - samplers_list - List of samplers to show
//    - command_type - Which function to call
//    - sflow_samplers_conf_type - Which samplers configuration to perform
//    - sflow_conf_value - New configuration value, or command for statistics
static vtss_rc sflow_icli_command(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *samplers_list, sflow_command_type_t command_type, const sflow_samplers_conf_type_t sflow_samplers_conf_type, const sflow_conf_value_t *sflow_conf_value)
{
    switch_iter_t   sit;
    BOOL note_shown = FALSE;

    // Loop through all switches in stack
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop through all ports
        port_iter_t pit;
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
        T_NG(TRACE_GRP_ICLI, "isid:%d", sit.isid);
        while (icli_port_iter_getnext(&pit, plist)) {
            i32 inst;
            if (samplers_list == NULL) {
                for (inst = 1; inst <= SFLOW_INSTANCE_CNT; inst++) {
                    switch (command_type) {
                    case SAMPLER:
                        VTSS_RC(sflow_icli_samplers_conf(session_id, inst, sflow_samplers_conf_type, sflow_conf_value, &sit, &pit, &note_shown));
                        break;
                    case COUNTER_POLL:
                        VTSS_RC(sflow_icli_counter_poll_interval_conf(inst, sflow_conf_value, &sit, &pit));
                        break;
                    case STATISTICS_CMD:
                        VTSS_RC(sflow_icli_statistics_samplers(session_id, inst, sflow_conf_value->clear, pit.first, &sit, &pit));
                        break;
                    }
                }
            } else {
                // Print only the indexes specified by user.
                u8 cnt_index;
                for (cnt_index = 0; cnt_index < samplers_list->u.sr.cnt; cnt_index++) {
                    for (inst = samplers_list->u.sr.range[cnt_index].min; inst <= samplers_list->u.sr.range[cnt_index].max; inst++) {
                        switch (command_type) {
                        case SAMPLER:
                            VTSS_RC(sflow_icli_samplers_conf(session_id, inst, sflow_samplers_conf_type, sflow_conf_value, &sit, &pit, &note_shown));
                            break;
                        case COUNTER_POLL:
                            VTSS_RC(sflow_icli_counter_poll_interval_conf(inst, sflow_conf_value, &sit, &pit));
                            break;
                        case STATISTICS_CMD:
                            VTSS_RC(sflow_icli_statistics_samplers(session_id, inst, sflow_conf_value->clear, pit.first, &sit, &pit));
                            break;
                        }
                    }
                }
            }
        }
    }
    return VTSS_RC_OK;
}

// Function for looping through receiver instances and call the configuration function
// In - rcvr_idx_list - List of receiver instances to loop through.
//    - sflow_rcvr_conf_type - Which receiver configuration to do
//    - sflow_conf_value - pointer to new configuration
static vtss_rc sflow_icli_receiver_set(icli_range_t *rcvr_idx_list, sflow_rcvr_conf_type_t sflow_rcvr_conf_type, const sflow_rcvr_conf_value_t *sflow_conf_value)
{
    u8 rcvr_idx;
    if (rcvr_idx_list == NULL) {
        for (rcvr_idx = 1; rcvr_idx <= SFLOW_RECEIVER_CNT; rcvr_idx++) {
            VTSS_RC(sflow_icli_receiver_conf(rcvr_idx, sflow_rcvr_conf_type, sflow_conf_value));
        }
    } else {
        // Print only the indexes specified by user.
        u8 cnt_index;
        for (cnt_index = 0; cnt_index < rcvr_idx_list->u.sr.cnt; cnt_index++) {
            for (rcvr_idx = rcvr_idx_list->u.sr.range[cnt_index].min; rcvr_idx <= rcvr_idx_list->u.sr.range[cnt_index].max; rcvr_idx++) {
                VTSS_RC(sflow_icli_receiver_conf(rcvr_idx, sflow_rcvr_conf_type, sflow_conf_value));
            }
        }
    }
    return VTSS_RC_OK;
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
// See sflow_icli_functions.h
BOOL sflow_icli_run_time_sampler_instances(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if SFLOW_INSTANCE_CNT > 1
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    case ICLI_ASK_BYWORD:
#if SFLOW_INSTANCE_CNT > 1
        icli_sprintf(runtime->byword, "<instance index : %u-%u>", 1, SFLOW_INSTANCE_CNT);
#endif
        return TRUE;
    case ICLI_ASK_RANGE:
        runtime->range.type = ICLI_RANGE_TYPE_UNSIGNED;
        runtime->range.u.sr.cnt = SFLOW_RECEIVER_CNT;
        runtime->range.u.sr.range[0].min = 1;
        runtime->range.u.sr.range[0].max = SFLOW_RECEIVER_CNT;
        return TRUE;
    case ICLI_ASK_HELP:
#if SFLOW_INSTANCE_CNT > 1
        icli_sprintf(runtime->help, "Set or show counter polling interval configuration per poller instance per port.");
#endif
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

// See sflow_icli_functions.h
BOOL sflow_icli_run_time_collector_address(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    i8 help_str[100];
    strcpy(help_str, "IPv4 address");
#ifdef VTSS_SW_OPTION_IPV6
    strcat(help_str, " or IPv6 address");
#endif
#ifdef VTSS_SW_OPTION_DNS
    strcat(help_str, " or hostname");
#endif

    switch (ask) {
    case ICLI_ASK_BYWORD:
        icli_sprintf(runtime->byword, "%s", help_str);
        return TRUE;
    case ICLI_ASK_HELP:
        strcat(help_str, " identifying the collector receiver");
        icli_sprintf(runtime->help, "%s", help_str);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

// See sflow_icli_functions.h
BOOL sflow_icli_run_time_receiver_instances(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if SFLOW_RECEIVER_CNT > 1
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    case ICLI_ASK_BYWORD:
#if SFLOW_RECEIVER_CNT > 1
        icli_sprintf(runtime->byword, "<receiver index : %u-%u>", 1, SFLOW_RECEIVER_CNT);
#endif
        return TRUE;
    case ICLI_ASK_RANGE:
        runtime->range.type = ICLI_RANGE_TYPE_UNSIGNED;
        runtime->range.u.sr.cnt = SFLOW_RECEIVER_CNT;
        runtime->range.u.sr.range[0].min = 1;
        runtime->range.u.sr.range[0].max = SFLOW_RECEIVER_CNT;
        return TRUE;
    case ICLI_ASK_HELP:
#if SFLOW_RECEIVER_CNT > 1
        icli_sprintf(runtime->help, "Receiver index for the receiver(s)");
#endif
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_timeout(i32 session_id, u32 timeout, icli_range_t *rcvr_idx_list)
{
    sflow_rcvr_conf_type_t sflow_rcvr_conf_type = TIMEOUT;
    sflow_rcvr_conf_value_t sflow_conf_value;
    sflow_conf_value.timeout = timeout;
    VTSS_ICLI_ERR_PRINT(sflow_icli_receiver_set(rcvr_idx_list, sflow_rcvr_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_collector_port(i32 session_id, u32 collector_port, icli_range_t *rcvr_idx_list, BOOL no)
{
    sflow_rcvr_conf_type_t sflow_rcvr_conf_type = COLLECTOR_PORT;
    sflow_rcvr_conf_value_t sflow_conf_value;
    if (no) {
        sflow_conf_value.collector_port = SFLOW_RECEIVER_UDP_PORT_DEFAULT;
    } else {
        sflow_conf_value.collector_port = collector_port;
    }

    VTSS_ICLI_ERR_PRINT(sflow_icli_receiver_set(rcvr_idx_list, sflow_rcvr_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_max_datagram_size(i32 session_id, u32 max_datagram_size, icli_range_t *rcvr_idx_list, BOOL no)
{
    sflow_rcvr_conf_type_t sflow_rcvr_conf_type = MAX_DATAGRAM_SIZE;
    sflow_rcvr_conf_value_t sflow_conf_value;
    if (no) {
        sflow_conf_value.max_datagram_size = SFLOW_RECEIVER_DATAGRAM_SIZE_DEFAULT;
    } else {
        sflow_conf_value.max_datagram_size = max_datagram_size;
    }

    VTSS_ICLI_ERR_PRINT(sflow_icli_receiver_set(rcvr_idx_list, sflow_rcvr_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_release(i32 session_id, icli_range_t *rcvr_idx_list)
{
    sflow_rcvr_conf_type_t sflow_rcvr_conf_type = RELEASE;
    sflow_rcvr_conf_value_t sflow_conf_value;
    sflow_conf_value.timeout = 0;
    VTSS_ICLI_ERR_PRINT(sflow_icli_receiver_set(rcvr_idx_list, sflow_rcvr_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_statistics(i32 session_id, BOOL has_receiver, BOOL has_samplers, icli_range_t *rcvr_idx_list, icli_range_t *sampler_idx_list, icli_stack_port_range_t *plist, BOOL has_clear)
{
    sflow_command_type_t command_type = STATISTICS_CMD;
    sflow_conf_value_t sflow_conf_value;
    sflow_samplers_conf_type_t sflow_conf_type = MAX_HDR_SIZE; // Dummy - Not real used in this function.

    sflow_conf_value.clear = has_clear;

    if (has_receiver) {
        VTSS_ICLI_ERR_PRINT(sflow_icli_statistics_recevier_loop(session_id, rcvr_idx_list, has_clear));
    } else if (has_samplers) {
        VTSS_ICLI_ERR_PRINT(sflow_icli_command(session_id, plist, sampler_idx_list, command_type, sflow_conf_type, &sflow_conf_value));
    } else {
        T_E("Sorry don't know what to do");
    }
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_show_flow_conf(i32 session_id)
{
    VTSS_ICLI_ERR_PRINT(sflow_print_conf(session_id));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_collector_address(i32 session_id, icli_range_t *rcvr_idx_list, char *hostname, BOOL no)
{
    sflow_rcvr_conf_type_t sflow_rcvr_conf_type = COLLECTOR;
    sflow_rcvr_conf_value_t sflow_conf_value;
    if (no || hostname == NULL) {
        strcpy((char *)sflow_conf_value.hostname, "0.0.0.0");
    } else {
        memcpy(sflow_conf_value.hostname, hostname, sizeof(sflow_conf_value.hostname));
    }
    VTSS_ICLI_ERR_PRINT(sflow_icli_receiver_set(rcvr_idx_list, sflow_rcvr_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_enable(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *sampler_idx_list, BOOL no)
{
    sflow_command_type_t command_type = SAMPLER;
    sflow_samplers_conf_type_t sflow_conf_type = ENABLE;
    sflow_conf_value_t sflow_conf_value;

    sflow_conf_value.enable = !no;

    VTSS_ICLI_ERR_PRINT(sflow_icli_command(session_id, plist, sampler_idx_list, command_type, sflow_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}


// See sflow_icli_functions.h
vtss_rc sflow_icli_sampling_rate(i32 session_id, icli_stack_port_range_t *plist, u32 sampling_rate, icli_range_t *sampler_idx_list, BOOL no)
{
    sflow_command_type_t command_type = SAMPLER;
    sflow_samplers_conf_type_t sflow_conf_type = SAMPLING_RATE;
    sflow_conf_value_t sflow_conf_value;

    if (no) {
        sflow_conf_value.sampling_rate = SFLOW_FLOW_SAMPLING_RATE_DEFAULT;
    } else {
        sflow_conf_value.sampling_rate = sampling_rate;
    }

    T_IG(TRACE_GRP_ICLI, "sampling_rate:%d", sampling_rate);
    VTSS_ICLI_ERR_PRINT(sflow_icli_command(session_id, plist, sampler_idx_list, command_type, sflow_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_max_hdr_size(i32 session_id, icli_stack_port_range_t *plist, u32 max_hdr_size, icli_range_t *sampler_idx_list, BOOL no)
{
    sflow_command_type_t command_type = SAMPLER;
    sflow_samplers_conf_type_t sflow_conf_type = MAX_HDR_SIZE;
    sflow_conf_value_t sflow_conf_value;

    if (no) {
        sflow_conf_value.max_hdr_size = SFLOW_FLOW_HEADER_SIZE_DEFAULT;
    } else {
        sflow_conf_value.max_hdr_size = max_hdr_size;
    }

    VTSS_ICLI_ERR_PRINT(sflow_icli_command(session_id, plist, sampler_idx_list, command_type, sflow_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_counter_poll_interval(i32 session_id, icli_stack_port_range_t *plist, u32 poll_interval, icli_range_t *sampler_idx_list, BOOL no)
{
    sflow_command_type_t command_type = COUNTER_POLL;
    sflow_conf_value_t sflow_conf_value;
    sflow_samplers_conf_type_t sflow_conf_type = MAX_HDR_SIZE; // Dummy - Not real used in this function.

    if (no) {
        sflow_conf_value.poll_interval = SFLOW_POLLING_INTERVAL_DEFAULT;
    } else {
        sflow_conf_value.poll_interval = poll_interval;
    }

    T_IG(TRACE_GRP_ICLI, "sflow_conf_value.poll_interval:%d", sflow_conf_value.poll_interval);
    VTSS_ICLI_ERR_PRINT(sflow_icli_command(session_id, plist, sampler_idx_list, command_type, sflow_conf_type, &sflow_conf_value));
    return VTSS_RC_OK;
}

// See sflow_icli_functions.h
vtss_rc sflow_icli_agent_ip(i32 session_id, BOOL has_ipv4, vtss_ip_t *v_ipv4_addr, BOOL has_ipv6, vtss_ipv6_t *v_ipv6_addr, BOOL no)
{
    sflow_agent_t agent;
    vtss_rc       rc;

    if ((rc = sflow_mgmt_agent_cfg_get(&agent)) != VTSS_RC_OK) {
        VTSS_ICLI_ERR_PRINT(rc);
        return VTSS_RC_OK;
    }

    if (no) {
        agent.agent_ip_addr.type      = SFLOW_AGENT_IP_TYPE_DEFAULT;
        agent.agent_ip_addr.addr.ipv4 = SFLOW_AGENT_IP_ADDR_DEFAULT;
    } else if (has_ipv4 && v_ipv4_addr != NULL) {
        agent.agent_ip_addr.type      = VTSS_IP_TYPE_IPV4;
        agent.agent_ip_addr.addr.ipv4 = *v_ipv4_addr;
    } else if (has_ipv6 && v_ipv6_addr != NULL) {
        agent.agent_ip_addr.type      = VTSS_IP_TYPE_IPV6;
        memcpy(agent.agent_ip_addr.addr.ipv6.addr, v_ipv6_addr->addr, sizeof(agent.agent_ip_addr.addr.ipv6));
    } else {
        T_E("How did I get here?");
        return VTSS_RC_ERROR;
    }

    VTSS_ICLI_ERR_PRINT(sflow_mgmt_agent_cfg_set(&agent));
    return VTSS_RC_OK;
}

#endif // #ifdef VTSS_SW_OPTION_SFLOW
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

