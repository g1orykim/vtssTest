/*

 Vitesse ETH Link OAM software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "cli_trace_def.h"
#include "eth_link_oam_api.h"
#include "eth_link_oam_cli.h"
#include "netdb.h"

typedef struct {
    /* Keywords */
    vtss_eth_link_oam_control_t    enable;
    vtss_eth_link_oam_mode_t       mode;
    u8                             variable_choice;
    u8                             flags;
    u8                             flags_cntrl;
    u8                             link_monitoring_event;
    u64                            window;
    u64                            threshold;
    u64                            rxpacket_threshold;
} eth_link_oam_cli_req_t;

void eth_link_oam_cli_req_init(void)
{
    /* register the size required for OAM req. structure */
    cli_req_size_register(sizeof(eth_link_oam_cli_req_t));
}

static void loam_print_error(vtss_isid_t    isid,
                             vtss_port_no_t iport,
                             const vtss_rc rc)
{
    iport++;

    switch (rc) {

    case ETH_LINK_OAM_RC_INVALID_PARAMETER:
        CPRINTF("Invalid parameter error returned from LOAM on port(%u/%u)\n", isid,
                iport);
        break;
    case ETH_LINK_OAM_RC_NOT_ENABLED:
        CPRINTF("Link OAM is not enabled on the port(%u/%u)\n", isid, iport);
        break;
    case ETH_LINK_OAM_RC_ALREADY_CONFIGURED:
        CPRINTF("Requested configuration is already configured on port(%u/%u)\n", isid,
                iport);
        break;
    case ETH_LINK_OAM_RC_NOT_SUPPORTED:
        CPRINTF("Requested configuration is not supported with the current OAM mode on port(%u/%u)\n",
                isid, iport);
        break;
    case ETH_LINK_OAM_RC_INVALID_STATE:
        CPRINTF("Requested configuration is not supported with the current OAM state on port(%u/%u)\n",
                isid, iport);
        break;
    case ETH_LINK_OAM_RC_TIMED_OUT:
        CPRINTF("Requested operation gets timed out on port(%u/%u)\n", isid, iport);
        break;
    default:
        CPRINTF("Error returned from OAM on port(%u/%u)\n", isid, iport);
        break;
    }
    return;
}


static void cli_cmd_eth_link_oam_mode_conf(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    BOOL                                        first = TRUE;
    char                                        buf[80], *p;
    vtss_eth_link_oam_mode_t                    oam_mode_flag;
    eth_link_oam_cli_req_t                      *module_req;
    vtss_rc                                     rc;

    module_req = req->module_req;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    rc = eth_link_oam_mgmt_port_mode_conf_set (
                             isid, pit.iport, module_req->mode);
                    if (rc != VTSS_RC_OK) {
                        loam_print_error(isid, pit.iport, rc);
                        continue;
                    }
                } else {
                    if (first == TRUE) {
                        first = FALSE;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "Port Mode    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    if (eth_link_oam_mgmt_port_mode_conf_get (isid, pit.iport, &oam_mode_flag) == VTSS_RC_OK) {
                        switch (oam_mode_flag) {
                        case VTSS_ETH_LINK_OAM_MODE_ACTIVE:
                            CPRINTF("%s     ", "active");
                            break;
                        case VTSS_ETH_LINK_OAM_MODE_PASSIVE:
                            CPRINTF("%s     ", "passive");
                            break;
                        default:
                            CPRINTF("%s     ", "passive");
                        }

                    } else {
                        CPRINTF("%s     ", "passive");
                    }
                    CPRINTF("\n");
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_control_conf(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    BOOL                                        first = TRUE;
    char                                        buf[80], *p;
    vtss_eth_link_oam_control_t                 oam_control_flag;
    vtss_rc                                     rc;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    rc = eth_link_oam_mgmt_port_control_conf_set(isid, pit.iport, req->enable);
                    if (rc != VTSS_RC_OK) {
                        loam_print_error(isid, pit.iport, rc);
                        continue;
                    }
                } else {
                    if (first == TRUE) {
                        first = FALSE;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "Port Mode    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    if (eth_link_oam_mgmt_port_control_conf_get (isid, pit.iport, &oam_control_flag) == VTSS_RC_OK) {
                        switch (oam_control_flag) {
                        case VTSS_ETH_LINK_OAM_CONTROL_ENABLE:
                            CPRINTF("%s     ", "enabled");
                            break;
                        case VTSS_ETH_LINK_OAM_CONTROL_DISABLE:
                            CPRINTF("%s     ", "disabled");
                            break;
                        default:
                            CPRINTF("%s     ", "disabled");
                        }
                    } else {
                        CPRINTF("%s     ", "disabled");
                    }
                    CPRINTF("\n");
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_mib_retrival_conf(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    BOOL                                        first = TRUE;
    char                                        buf[80], *p;
    BOOL                                        conf;
    vtss_rc                                     rc;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    rc = eth_link_oam_mgmt_port_mib_retrival_conf_set (isid, pit.iport, req->enable);
                    if (rc != VTSS_RC_OK) {
                        loam_print_error(isid, pit.iport, rc);
                    }
                } else {
                    if (first == TRUE) {
                        first = FALSE;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "MIB retrival support    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    if (eth_link_oam_mgmt_port_mib_retrival_conf_get (isid, pit.iport, &conf) == VTSS_RC_OK) {
                        switch (conf) {
                        case TRUE:
                            CPRINTF("%s     ", "enabled");
                            break;
                        case FALSE:
                            CPRINTF("%s     ", "disabled");
                            break;
                        default:
                            CPRINTF("%s     ", "disabled");
                        }
                    } else {
                        CPRINTF("%s     ", "disabled");
                    }
                    CPRINTF("\n");
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_remote_loopback_conf(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    BOOL                                        first = TRUE;
    char                                        buf[80], *p;
    BOOL                                        conf;
    vtss_rc                                     rc;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    rc = eth_link_oam_mgmt_port_remote_loopback_conf_set(isid, pit.iport, req->enable);
                    if (rc == ETH_LINK_OAM_RC_NOT_SUPPORTED) {
                        CPRINTF("The configuration is invalid as remote loopback is on.\n");
                        break;
                    }
                } else {
                    if (first == TRUE) {
                        first = FALSE;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "Remote LoopBack support    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    if (eth_link_oam_mgmt_port_remote_loopback_conf_get (isid, pit.iport, &conf) == VTSS_RC_OK) {
                        switch (conf) {
                        case TRUE:
                            CPRINTF("%s     ", "enabled");
                            break;
                        case FALSE:
                            CPRINTF("%s     ", "disabled");
                            break;
                        default:
                            CPRINTF("%s     ", "disabled");
                        }
                    } else {
                        CPRINTF("%s     ", "disabled");
                    }
                    CPRINTF("\n");
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_link_monitoring_conf(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    BOOL                                        first = TRUE;
    char                                        buf[80], *p;
    BOOL                                        conf;
    vtss_rc                                     rc;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    rc = eth_link_oam_mgmt_port_link_monitoring_conf_set(isid, pit.iport, req->enable);
                    if (rc != VTSS_RC_OK) {
                        loam_print_error(isid, pit.iport, rc);
                        continue;
                    }
                } else {
                    if (first == TRUE) {
                        first = FALSE;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "Link Monitoring support    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    if (eth_link_oam_mgmt_port_link_monitoring_conf_get (isid, pit.iport, &conf) == VTSS_RC_OK) {
                        switch (conf) {
                        case TRUE:
                            CPRINTF("%s     ", "enabled");
                            break;
                        case FALSE:
                            CPRINTF("%s     ", "disabled");
                            break;
                        default:
                            CPRINTF("%s     ", "disabled");
                        }
                    } else {
                        CPRINTF("%s     ", "disabled");
                    }
                    CPRINTF("\n");
                }
            }
        }
    }
}
static void cli_cmd_eth_link_oam_link_monitoring_event_conf(BOOL frame_error,
                                                            BOOL symbol_period_error,
                                                            BOOL frame_period_error,
                                                            BOOL frame_secs_summary_error,
                                                            cli_req_t *req)
{
    eth_link_oam_cli_req_t            *oam_req = NULL;
    vtss_usid_t                       usid;
    vtss_isid_t                       isid;
    port_iter_t                       pit;
    BOOL                              display_header = TRUE;
    char                              buf[80];
    u16                               error_frame_window;
    u32                               error_frame_threshold;
    u64                               symbol_period_error_window;
    u64                               symbol_period_error_threshold;
    u64                               rxpacket_threshold;
    u32                               frame_period_error_window;
    u32                               frame_period_error_threshold;
    u16                               error_frame_secs_summary_window;
    u16                               error_frame_secs_summary_threshold;
    vtss_rc                           rc = 0;

    oam_req = req->module_req;
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->set) {
                    if (frame_error) {
                        (eth_link_oam_mgmt_port_link_error_frame_window_set (isid, pit.iport, (u16)oam_req->window)
                         != VTSS_RC_OK ) ? (rc += 1) : (rc += 0) ;
                        (eth_link_oam_mgmt_port_link_error_frame_threshold_set (isid, pit.iport, (u32)oam_req->threshold)
                         != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                    } else if (symbol_period_error) {
                        (eth_link_oam_mgmt_port_link_symbol_period_error_window_set(isid, pit.iport, oam_req->window)
                         != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set (isid, pit.iport,
                                                                                        oam_req->threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set(isid, pit.iport,
                                                                                                 oam_req->rxpacket_threshold) != VTSS_RC_OK ) ? (rc += 1) : (rc += 0) ;
                    } else if (frame_period_error) {
                        (eth_link_oam_mgmt_port_link_frame_period_error_window_set(isid, pit.iport, oam_req->window)
                         != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_frame_period_error_threshold_set(isid, pit.iport,
                                                                                      oam_req->threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set(isid, pit.iport,
                                                                                                oam_req->rxpacket_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                    } else {
                        (eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set (isid, pit.iport,
                                                                                          (u16)oam_req->window) != VTSS_RC_OK ) ? (rc += 1) : (rc += 0) ;
                        (eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set(isid, pit.iport,
                                                                                            (u16)oam_req->threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                    }
                    if (rc) {
                        CPRINTF("Error In Configuration \n");
                        continue;
                    }
                } else {
                    /* Place For Display */
                    if (display_header == TRUE) {
                        strcpy(buf, "Port-No      Error_Window  Error_Threshold ");
#if defined(RX_THRESHOLD_SUPPORT)
                        if (!frame_error && !frame_secs_summary_error) {
                            strcat(buf, "  Rx_threshold");
                        }
#endif /* RX_THRESHOLD_SUPPORT */
                        cli_table_header(buf);
                        display_header = FALSE;
                    }
                    if (frame_error) {
                        (eth_link_oam_mgmt_port_link_error_frame_window_get (isid, pit.iport, &error_frame_window)
                         != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_error_frame_threshold_get (isid, pit.iport, &error_frame_threshold)
                         != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        CPRINTF(" %-10d     %-10u       %-10u\n", pit.uport, error_frame_window,
                                error_frame_threshold);
                    } else if (symbol_period_error) {
                        (eth_link_oam_mgmt_port_link_symbol_period_error_window_get(isid, pit.iport,
                                                                                    &symbol_period_error_window) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get (isid, pit.iport,
                                                                                        &symbol_period_error_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get (isid, pit.iport,
                                                                                                  &rxpacket_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
#if defined(RX_THRESHOLD_SUPPORT)
                        CPRINTF(" %-10d     %-10llu        %-10llu        %-10llu\n", (int)uport,
                                symbol_period_error_window, symbol_period_error_threshold, rxpacket_threshold);
#else
                        CPRINTF(" %-10d     %-10llu        %-10llu\n", pit.uport,
                                symbol_period_error_window, symbol_period_error_threshold);
#endif
                    } else if (frame_period_error) {
                        (eth_link_oam_mgmt_port_link_frame_period_error_window_get(isid, pit.iport,
                                                                                   &frame_period_error_window) != VTSS_RC_OK ) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_frame_period_error_threshold_get (isid, pit.iport,
                                                                                       &frame_period_error_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get (isid,
                                                                                                 pit.iport, &rxpacket_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        CPRINTF(" %-10d     %-10u        %-10u        %-10llu\n", pit.uport,
                                (unsigned int)frame_period_error_window,
                                (unsigned int)frame_period_error_threshold, rxpacket_threshold);
                    } else {
                        (eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get (isid, pit.iport,
                                                                                          &error_frame_secs_summary_window) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        (eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get (isid, pit.iport,
                                                                                             &error_frame_secs_summary_threshold) != VTSS_RC_OK) ? (rc += 1) : (rc += 0);
                        CPRINTF(" %-10d     %-10u      %-10u\n", pit.uport,
                                error_frame_secs_summary_window, error_frame_secs_summary_threshold);
                    }
                    if (rc) {
                        CPRINTF("Error In Getting the Configuration \n");
                    }
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_link_monitoring_error_frame_event_conf(cli_req_t *req)
{
    cli_cmd_eth_link_oam_link_monitoring_event_conf(1, 0, 0, 0, req);
}

static void cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf(cli_req_t *req)
{
    cli_cmd_eth_link_oam_link_monitoring_event_conf(0, 1, 0, 0, req);
}

#if defined(RX_THRESHOLD_SUPPORT)
static void cli_cmd_eth_link_oam_link_monitoring_frame_period_error_event_conf(cli_req_t *req)
{
    cli_cmd_eth_link_oam_link_monitoring_event_conf(0, 0, 1, 0, req);
}
#endif /* RX_THRESHOLD_SUPPORT */

static void cli_cmd_eth_link_oam_link_monitoring_error_frame_secs_summary_event_conf(cli_req_t *req)
{
    cli_cmd_eth_link_oam_link_monitoring_event_conf(0, 0, 0, 1, req);
}

static void cli_cmd_eth_link_oam_mib_retrival_oper_conf_set(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    eth_link_oam_cli_req_t                      *module_req;
    vtss_rc                                     rc;
    BOOL                                        opr_lock = FALSE;
    i8                                          buf[VTSS_ETH_LINK_OAM_RESPONSE_BUF] = {0};


    module_req = req->module_req;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                vtss_eth_link_oam_mgmt_client_register_cb(pit.iport, vtss_eth_link_oam_send_response_to_cli, buf,
                                                          VTSS_ETH_LINK_OAM_RESPONSE_BUF);
                rc = eth_link_oam_mgmt_port_mib_retrival_oper_set(isid, pit.iport,
                                                                  module_req->variable_choice);

                if (rc == ETH_LINK_OAM_RC_INVALID_STATE) {
                    CPRINTF("Requested operation is already in progress\n");
                    break;
                } else if (rc != VTSS_RC_OK) {
                    CPRINTF("Invalid request on this port\n");
                    break;
                }

                opr_lock = vtss_eth_link_oam_mib_retrival_opr_lock();
                if (opr_lock == FALSE) {
                    CPRINTF("Requested operation got timed out\n");
                } else {
                    CPRINTF("\n ********* Response Received************\n");
                    CPRINTF("%s\n", buf);
                }
                vtss_eth_link_oam_mgmt_client_deregister_cb(pit.iport);
            }
        }
    }
}

static void cli_cmd_eth_link_oam_remote_loopback_oper_conf_set(cli_req_t *req)
{

    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    vtss_rc                                     rc;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                rc = eth_link_oam_mgmt_port_remote_loopback_oper_conf_set(isid, pit.iport, req->enable);
                if (rc == ETH_LINK_OAM_RC_INVALID_STATE) {
                    CPRINTF("Requested operation is already in progress\n");
                    continue;
                } else if (rc != VTSS_RC_OK) {
                    loam_print_error(isid, pit.iport, rc);
                    continue;
                }
            }
        }
    }
}

static void eth_link_oam_client_display_info(vtss_eth_link_oam_info_tlv_t local_info,
                                             vtss_eth_link_oam_info_tlv_t remote_info, BOOL remote_active)
{
    u16               temp, remote_temp;
    i8                buf[80] = {0}, remote_buf[80] = { 0 };

    /*Print Local configuration mode*/
    CPRINTF("Mode:                                ");
    if (IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE)) {
        CPRINTF("%-24s", "active");
    } else {
        CPRINTF("%-24s", "passive");
    }
    /*Remote configuration mode info*/
    if (remote_active) {
        if (IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE)) {
            CPRINTF("%8s", "active\n");
        } else {
            CPRINTF("%8s", "passive\n");
        }
    } else {
        CPRINTF("%8s", "-------\n");
    }
    /* Local */
    CPRINTF("Unidirectional operation support:    ");
    if (IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT)) {
        CPRINTF("%-24s", "enabled");
    } else {
        CPRINTF("%-24s", "disabled");
    }
    /* Remote */
    if (remote_active) {
        if (IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT)) {
            CPRINTF("%8s", "enabled\n");
        } else if (remote_active) {
            CPRINTF("%8s", "disabled\n");
        }
    } else {
        CPRINTF("%8s", "-------\n");
    }
    /*Local */
    CPRINTF("Remote loopback support:             ");
    if (IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT)) {
        CPRINTF("%-24s", "enabled");
    } else {
        CPRINTF("%-24s", "disabled");
    }
    /*Remote*/
    if (remote_active) {
        if (IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT)) {
            CPRINTF("%8s", "enabled\n");
        } else {
            CPRINTF("%8s", "disabled\n");
        }
    } else {
        CPRINTF("%8s", "-------\n");
    }

    /*Local */
    CPRINTF("Link monitoring support:             ");
    if (IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT)) {
        CPRINTF("%-24s", "enabled");
    } else {
        CPRINTF("%-24s", "disabled");
    }
    /*Remote*/
    if (remote_active) {
        if (IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT)) {
            CPRINTF("%8s", "enabled\n");
        } else {
            CPRINTF("%8s", "disabled\n");
        }
    } else {
        CPRINTF("%8s", "-------\n");
    }
    /*Local*/
    CPRINTF("MIB retrival support:                ");
    if (IS_CONF_ACTIVE(local_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT)) {
        CPRINTF("%-24s", "enabled");
    } else {
        CPRINTF("%-24s", "disabled");
    }
    /*Remote*/
    if (remote_active) {
        if (IS_CONF_ACTIVE(remote_info.oam_conf, VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT)) {
            CPRINTF("%8s", "enabled\n");
        } else {
            CPRINTF("%8s", "disabled\n");
        }
    } else {
        CPRINTF("%8s", "-------\n");
    }

    memcpy(&temp, local_info.oampdu_conf, sizeof(temp));
    temp = NET2HOSTS(temp);
    memcpy(&remote_temp, remote_info.oampdu_conf, sizeof(remote_temp));
    remote_temp = NET2HOSTS(remote_temp);
    if (remote_active) {
        CPRINTF("MTU Size:                            %-4d%24d\n", temp, remote_temp);
        CPRINTF("Multiplexer state:                   %-14s%21s\n", mux_state_to_str((local_info.state & 4) >> 2), mux_state_to_str((remote_info.state & 4) >> 2));
        CPRINTF("Parser state:                        %-14s%21s\n", parser_state_to_str(local_info.state & 3), parser_state_to_str(remote_info.state & 3));
        sprintf(buf, "%02x:%02x:%02x", local_info.oui[0], local_info.oui[1], local_info.oui[2]);
        sprintf(remote_buf, "%02x:%02x:%02x", (i8)remote_info.oui[0], remote_info.oui[1], remote_info.oui[2]);
        CPRINTF("OUI:                                 %-10s%22s\n", buf, remote_buf);
    } else {
        CPRINTF("MTU Size:                            %-4d%27s\n", temp, "-------");
        CPRINTF("Multiplexer state:                   %-14s%17s\n", mux_state_to_str((local_info.state & 4) >> 2), "-------");
        CPRINTF("Parser state:                        %-14s%17s\n", parser_state_to_str(local_info.state & 3), "-------");
        sprintf(buf, "%02x-%02x-%02x", local_info.oui[0], local_info.oui[1], local_info.oui[2]);
        CPRINTF("OUI:                                 %-10s%21s\n", buf, "-------");
    }
    memcpy(&temp, local_info.revision, sizeof(temp));
    temp = NET2HOSTS(temp);
    CPRINTF("PDU revision :                           %-4d%23s\n", temp, "-------");
    return;
}

static void  port_flags_to_str(u8 flags, char *buf)
{
    strcpy(buf, "");
    if (flags & VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT) {
        strcpy(buf, strcat(buf, " link_fault "));
    }
    if (flags & VTSS_ETH_LINK_OAM_FLAG_DYING_GASP) {
        strcpy(buf, strcat(buf, " dying_gasp "));
    }
    if (flags & VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT) {
        strcpy(buf, strcat(buf, " crit_event "));
    }
    return ;
}

static void cli_cmd_eth_link_oam_status(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    char                                        buf[80], *p;
    vtss_eth_link_oam_info_tlv_t                local_info, remote_info;
    u8                                          oper_status;
    u16                                         temp;
    BOOL                                        remote_active = FALSE;
    vtss_eth_link_oam_discovery_state_t         state;
    vtss_eth_link_oam_pdu_control_t             tx_control;
    vtss_rc                                     rc, rc1;
    u8                                          remote_mac_addr[VTSS_COMMON_MACADDR_SIZE];


    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                memset(&local_info, 0, sizeof(local_info));
                memset(&remote_info, 0, sizeof(remote_info));
                rc  = eth_link_oam_client_port_local_info_get (isid, pit.iport, &local_info);
                rc1 = eth_link_oam_client_port_remote_info_get (isid, pit.iport, &remote_info);
                if ( (rc  != VTSS_RC_OK) ||
                     ( (rc1 != VTSS_RC_OK) &&
                       (rc1 != ETH_LINK_OAM_RC_NOT_ENABLED)
                     ) ) {
                    continue;

                }
                cli_cmd_usid_print(usid, req, 1);
                CPRINTF("Port :                                 %u    \n", pit.uport);
                if (eth_link_oam_control_layer_port_pdu_control_status_get(isid, pit.iport,
                                                                           &tx_control) == VTSS_RC_OK) {
                    CPRINTF("PDU permission:                        %s\n", pdu_tx_control_to_str(tx_control));
                }
                if (eth_link_oam_control_layer_port_discovery_state_get(isid, pit.iport, &state) == VTSS_RC_OK) {
                    CPRINTF("Discovery state:                       %s\n", discovery_state_to_str(state));
                }
                memset(buf, '\0', sizeof(buf));
                if (eth_link_oam_client_port_remote_mac_addr_info_get(isid, pit.iport, remote_mac_addr) == VTSS_RC_OK) {
                    p = &buf[0];
                    for (temp = 0; temp < VTSS_COMMON_MACADDR_SIZE; temp++) {
                        if ((temp + 1) == VTSS_COMMON_MACADDR_SIZE) {
                            sprintf(p, "%02x", remote_mac_addr[temp]);
                        } else {
                            sprintf(p, "%02x:", remote_mac_addr[temp]);
                        }
                        p = p + 3;
                    }
                } else {
                    sprintf(buf, "%s", "-");
                }
                CPRINTF("Remote MAC Address:                    %s\n\n", buf);
                memset(buf, '\0', sizeof(buf));

                p = &buf[0];
                p += sprintf(p, "                                     Local client          Remote Client");
                cli_table_header(buf);

                if (rc == VTSS_RC_OK) {
                    oper_status = VTSS_ETH_LINK_OAM_NULL;
                    if (rc1 == VTSS_RC_OK) {
                        remote_active = TRUE;
                        CPRINTF("\nOperational status: \n");
                        if (eth_link_oam_client_port_control_conf_get(isid, pit.iport, &oper_status) == VTSS_RC_OK) {
                            if (oper_status) {
                                CPRINTF("port status:                         operational             operational\n");
                            } else {
                                CPRINTF("port status:                         non operational         operational\n");
                            }
                        }

                    } else {
                        remote_active = FALSE;
                        if (oper_status) {
                            CPRINTF("port status:                            operational             -------\n");
                        } else {
                            CPRINTF("port status:                         non operational         -------\n");
                        }
                    }
                    eth_link_oam_client_display_info(local_info, remote_info, remote_active);
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_link_monitor_status(cli_req_t *req)
{
    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    vtss_eth_link_oam_error_frame_event_tlv_t   error_frame_tlv;
    vtss_eth_link_oam_error_frame_event_tlv_t   remote_error_frame_tlv;
    u16                                         temp16;
    u32                                         temp32;
    u64                                         temp64;
    vtss_rc                                     rc;

    vtss_eth_link_oam_error_frame_period_event_tlv_t  error_frame_period_tlv;
    vtss_eth_link_oam_error_frame_period_event_tlv_t  remote_error_frame_period_tlv;

    vtss_eth_link_oam_error_symbol_period_event_tlv_t  error_symbol_period_tlv;
    vtss_eth_link_oam_error_symbol_period_event_tlv_t  remote_error_symbol_period_tlv;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t error_frame_secs_summary_tlv;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t remote_frame_secs_summary_tlv;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                cli_cmd_usid_print(usid, req, 1);
                temp16 = 0;
                rc = eth_link_oam_client_port_remote_seq_num_get(isid, pit.iport, &temp16);
                if (rc != VTSS_RC_OK) {
                    continue;
                }
                CPRINTF("Port :                                          %-2u\n", pit.uport);
                CPRINTF("Sequence number :                               %-2u\n", temp16);
                temp16 = 0;
                memset(&error_frame_tlv, '\0', sizeof(error_frame_tlv));
                memset(&remote_error_frame_tlv, '\0', sizeof(remote_error_frame_tlv));
                memset(&error_frame_period_tlv, '\0', sizeof(error_frame_period_tlv));
                memset(&remote_error_frame_period_tlv, '\0', sizeof(remote_error_frame_period_tlv));
                memset(&error_symbol_period_tlv, '\0', sizeof(error_symbol_period_tlv));
                memset(&remote_error_symbol_period_tlv, '\0', sizeof(remote_error_symbol_period_tlv));

                rc = eth_link_oam_client_port_symbol_period_error_info_get(isid, pit.iport,
                                                                           &error_symbol_period_tlv,
                                                                           &remote_error_symbol_period_tlv);

                if (rc == VTSS_RC_OK) {

                    /*memcpy(&temp16,remote_error_symbol_period_tlv.sequence_number,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_SEQUENCE_NUMBER_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Symbol period error event sequece number:       %u\n",temp16);*/

                    memcpy(&temp16, remote_error_symbol_period_tlv.event_time_stamp,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Symbol period error event Timestamp:            %u\n", temp16);

                    memcpy(&temp64, remote_error_symbol_period_tlv.error_symbol_window,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN);
                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Symbol period error event window:               %llu\n", temp64);

                    memcpy(&temp64, remote_error_symbol_period_tlv.error_symbol_threshold,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN);

                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Symbol period error event threshold:            %llu\n", temp64);

                    memcpy(&temp64, remote_error_symbol_period_tlv.error_symbols,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN);
                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Symbol period errors:                           %llu\n", temp64);

                    memcpy(&temp64, remote_error_symbol_period_tlv.error_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN);
                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Total symbol period errors:                     %llu\n", temp64);

                    memcpy(&temp32, remote_error_symbol_period_tlv.event_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Total symbol period error events:               %u\n\n", temp32);

                } else {
                    continue;
                }

                rc = eth_link_oam_client_port_frame_error_info_get(isid, pit.iport,
                                                                   &error_frame_tlv,
                                                                   &remote_error_frame_tlv);
                if (rc == VTSS_RC_OK) {

                    /*memcpy(&temp16,remote_error_frame_tlv.sequence_number,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_SEQUENCE_NUMBER_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Frame error event sequece number:               %u\n",temp16);*/

                    memcpy(&temp16, remote_error_frame_tlv.event_time_stamp,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TIME_STAMP_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Frame error event Timestamp:                    %u\n", temp16);

                    memcpy(&temp16, remote_error_frame_tlv.error_frame_window,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN);
                    temp16 = NET2HOSTS(temp16);
                    CPRINTF("Frame error event window:                       %u\n", temp16);

                    memcpy(&temp32, remote_error_frame_tlv.error_frame_threshold,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN);

                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Frame error event threshold:                    %u\n", temp32);

                    memcpy(&temp32, remote_error_frame_tlv.error_frames,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Frame errors:                                   %u\n", temp32);

                    memcpy(&temp64, remote_error_frame_tlv.error_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN);
                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Total frame errors:                             %llu\n", temp64);

                    memcpy(&temp32, remote_error_frame_tlv.event_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Total frame error events:                       %u\n\n", temp32);

                } else {
                    continue;
                }

                rc = eth_link_oam_client_port_frame_period_error_info_get(isid, pit.iport,
                                                                          &error_frame_period_tlv,
                                                                          &remote_error_frame_period_tlv);

                if (rc == VTSS_RC_OK) {

                    /*memcpy(&temp16,remote_error_frame_period_tlv.sequence_number,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_SEQUENCE_NUMBER_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Frame period error event sequece number:        %u\n",temp16);*/

                    memcpy(&temp16, remote_error_frame_period_tlv.event_time_stamp,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TIME_STAMP_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Frame period error event Timestamp:             %u\n", temp16);

                    memcpy(&temp32, remote_error_frame_period_tlv.error_frame_period_window,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Frame period error event window:                %u\n", temp32);

                    memcpy(&temp32, remote_error_frame_period_tlv.error_frame_threshold,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN);

                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Frame period error event threshold:             %u\n", temp32);

                    memcpy(&temp32, remote_error_frame_period_tlv.error_frames,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Frame period errors:                            %u\n", temp32);

                    memcpy(&temp64, remote_error_frame_period_tlv.error_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN);
                    temp64 = vtss_eth_link_oam_swap64(temp64);
                    CPRINTF("Total frame period errors:                      %llu\n", temp64);

                    memcpy(&temp32, remote_error_frame_period_tlv.event_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Total frame period error events:                %u\n\n", temp32);

                } else {
                    continue;
                }
                rc = eth_link_oam_client_port_error_frame_secs_summary_info_get(isid, pit.iport,
                                                                                &error_frame_secs_summary_tlv,
                                                                                &remote_frame_secs_summary_tlv);

                if (rc == VTSS_RC_OK) {
                    memcpy(&temp16, remote_frame_secs_summary_tlv.event_time_stamp,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TIME_STAMP_LEN);
                    temp16 = NET2HOSTS(temp16);

                    CPRINTF("Error Frame Seconds Summary Event Timestamp:    %u\n", temp16);

                    memcpy(&temp16, remote_frame_secs_summary_tlv.secs_summary_window,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN);
                    temp16 = NET2HOSTS(temp16);
                    CPRINTF("Error Frame Seconds Summary Event window:       %u\n", temp16);

                    memcpy(&temp16, remote_frame_secs_summary_tlv.secs_summary_threshold,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN);
                    temp16 = NET2HOSTS(temp16);
                    CPRINTF("Error Frame Seconds Summary  Event Threshold:   %u\n", temp16);

                    memcpy(&temp16, remote_frame_secs_summary_tlv.secs_summary_events,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN);
                    temp16 = NET2HOSTS(temp16);
                    CPRINTF("Error Frame Seconds Summary Errors:             %u\n", temp16);

                    memcpy(&temp32, remote_frame_secs_summary_tlv.error_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Total Error Frame Seconds Summary Errors:       %u\n", temp32);

                    memcpy(&temp32, remote_frame_secs_summary_tlv.event_running_total,
                           VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN);
                    temp32 = NET2HOSTL(temp32);
                    CPRINTF("Total Error Frame Seconds Summary Events:       %u\n", temp32);

                } else {
                    continue;
                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_stats(cli_req_t *req)
{

    vtss_usid_t                                 usid;
    vtss_isid_t                                 isid;
    port_iter_t                                 pit;
    char                                        buf[80], *p;
    vtss_eth_link_oam_pdu_stats_t               port_stats;
    vtss_rc                                     rc;

    vtss_eth_link_oam_critical_event_pdu_stats_t  port_ce_stats;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                if (req->clear) {
                    rc = eth_link_oam_clear_statistics(pit.iport);
                    if (rc != VTSS_RC_OK) {
                        loam_print_error(isid, pit.iport, rc);
                    }
                    continue;
                }
                CPRINTF("Port : %-2u\n", pit.uport);
                p = &buf[0];
                p += sprintf(p, "PDU stats  ");
                cli_table_header(buf);

                if (eth_link_oam_control_layer_port_pdu_stats_get(isid, pit.iport, &port_stats) == VTSS_RC_OK) {
                    CPRINTF("Information PDU TX:                        %u\n", port_stats.information_tx);
                    CPRINTF("Information PDU RX:                        %u\n", port_stats.information_rx);
                    CPRINTF("Variable request PDU RX:                   %u\n", port_stats.variable_request_rx);
                    CPRINTF("Variable request PDU TX:                   %u\n", port_stats.variable_request_tx);
                    CPRINTF("Variable response PDU RX:                  %u\n", port_stats.variable_response_rx);
                    CPRINTF("Variable response PDU TX:                  %u\n", port_stats.variable_response_tx);
                    CPRINTF("Loopback PDU RX:                           %u\n", port_stats.loopback_control_rx);
                    CPRINTF("Loopback PDU TX:                           %u\n", port_stats.loopback_control_tx);
                    CPRINTF("Link Unique event notification PDU TX:     %u\n",
                            port_stats.unique_event_notification_tx);
                    CPRINTF("Link Unique event notification PDU RX:     %u\n",
                            port_stats.unique_event_notification_rx);
                    CPRINTF("Link Duplicate event notification PDU TX:  %u\n",
                            port_stats.duplicate_event_notification_tx);
                    CPRINTF("Link Duplicate event notification PDU RX:  %u\n",
                            port_stats.duplicate_event_notification_rx);
                    CPRINTF("Unsupported PDU RX:                        %u\n", port_stats.unsupported_codes_rx);
                    CPRINTF("Unsupported PDU TX:                        %u\n", port_stats.unsupported_codes_tx);
                }
                if (eth_link_oam_control_layer_port_critical_event_pdu_stats_get(isid,
                                                                                 pit.iport, &port_ce_stats) == VTSS_RC_OK) {

                    CPRINTF("Link Fault PDU TX:                         %u\n", port_ce_stats.link_fault_tx);
                    CPRINTF("Link Fault PDU RX:                         %u\n", port_ce_stats.link_fault_rx);
                    CPRINTF("Dying gasp PDU TX:                         %u\n", port_ce_stats.dying_gasp_tx);
                    CPRINTF("Dying gasp PDU RX:                         %u\n", port_ce_stats.dying_gasp_rx);
                    CPRINTF("Critical event PDU TX:                     %u\n", port_ce_stats.critical_event_tx);
                    CPRINTF("Critical event PDU RX:                     %u\n", port_ce_stats.critical_event_rx);

                }
            }
        }
    }
}

static void cli_cmd_eth_link_oam_port_flags_control(cli_req_t *req)
{

    eth_link_oam_cli_req_t            *oam_req = NULL;
    vtss_usid_t                       usid;
    vtss_isid_t                       isid;
    port_iter_t                       pit;
    char                              buf[50];
    u8                                flags = 0;
    BOOL                              hdr_display = TRUE;


    oam_req = req->module_req;
    T_D("oam_req->flags-[%x], flags_cntrl-[%d]", oam_req->flags, oam_req->flags_cntrl);


    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Loop through all front and NPI ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                /* If the control reaches here the port is set/configured */
                switch ( oam_req->flags_cntrl) {
                case VTSS_LINK_OAM_FLAG_CONTROL_ENABLE :
                    if (!req->set) {
                        return;
                    }
                    if (eth_link_oam_control_port_flags_conf_set(isid, pit.iport,
                                                                 oam_req->flags,
                                                                 TRUE) != VTSS_RC_OK) {
                        port_flags_to_str(oam_req->flags, buf);
                        CPRINTF("Flag-[%s] is not set\n", buf);
                    }
                    break;
                case VTSS_LINK_OAM_FLAG_CONTROL_DISABLE:
                    if (!req->set) {
                        return;
                    }
                    if (eth_link_oam_control_port_flags_conf_set(isid, pit.iport,
                                                                 oam_req->flags,
                                                                 FALSE) != VTSS_RC_OK) {
                        port_flags_to_str(oam_req->flags, buf);
                        CPRINTF("Flag-[%s] is not Cleared\n", buf);
                    }
                    break;
                default :
                    /* If Control Reaches Here, it is for the display */
                    if (eth_link_oam_control_port_flags_conf_get(isid, pit.iport,
                                                                 &flags) != VTSS_RC_OK) {
                        CPRINTF("Error in Getting the Flags on the Port\n");
                    }
                    T_D("Flags are -[%x]\n", flags);
                    switch (oam_req->flags) {
                    case  VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT :
                        CPRINTF("link_fault flag on port is %s \n",
                                (flags & VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT)
                                ? "Set" : "Not Set");
                        break;
                    case  VTSS_ETH_LINK_OAM_FLAG_DYING_GASP :
                        CPRINTF("dying_gasp flag on port is %s \n",
                                (flags & VTSS_ETH_LINK_OAM_FLAG_DYING_GASP)
                                ? "Set" : "Not Set");
                        break;
                    case  VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT :
                        CPRINTF("crit_event flag on port is %s \n",
                                (flags & VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT)
                                ? "Set" : "Not Set");
                        break;
                    default :
                        /* If Control reaches Here then display all the flags */
                        if (hdr_display == TRUE) {
                            strcpy(buf, "Port-No      Flags   ");
                            cli_table_header(buf);
                            hdr_display = FALSE;
                        }
                        if (!(flags & (VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT |
                                       VTSS_ETH_LINK_OAM_FLAG_DYING_GASP |
                                       VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT))) {
                            CPRINTF(" %-10d  Disabled\n", pit.uport);
                        } else {
                            port_flags_to_str(flags, buf);
                            CPRINTF(" %-10d  %s\n", pit.uport, buf);
                        }
                        break;
                    }
                }
            }
        }
    }
}

static int32_t cli_eth_link_oam_parse_window (char *cmd, char *cmd2, char *stx,
                                              char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    eth_link_oam_cli_req_t *oam_req = NULL;

    req->parm_parsed = 1;
    error = (cli_parse_ulong(cmd, &value, 1, 900)
             &&
             cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req));
    oam_req = req->module_req;
    oam_req->window = value;

    return error;
}

static int32_t cli_eth_link_oam_parse_error_threshold (char *cmd, char *cmd2, char *stx,
                                                       char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    longlong value = 0;
    eth_link_oam_cli_req_t *oam_req = NULL;

    req->parm_parsed = 1;
    error = (cli_parse_longlong(cmd, &value, 0, 0xffffffff)
             &&
             cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req));
    oam_req = req->module_req;
    oam_req->threshold = (ulong)value;

    return error;
}

#if defined(RX_THRESHOLD_SUPPORT)
static int32_t cli_eth_link_oam_parse_rxpacket_threshold (char *cmd, char *cmd2, char *stx,
                                                          char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    eth_link_oam_cli_req_t *oam_req = NULL;

    req->parm_parsed = 1;
    error = (cli_parse_ulong(cmd, &value, 0, 100000)
             &&
             cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req));
    oam_req = req->module_req;
    oam_req->rxpacket_threshold = value;

    return error;
}
#endif /* RX_THRESHOLD_SUPPORT */

static int32_t cli_eth_link_oam_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                              cli_req_t *req)
{
    eth_link_oam_cli_req_t *oam_req = NULL;

    char *found = cli_parse_find(cmd, stx);
    T_I("ALL %s", found);

    req->parm_parsed = 1;

    oam_req = req->module_req;
    oam_req->flags_cntrl = 0;

    if (found != NULL) {
        if (!strncmp(found, "active", 6)) {
            oam_req->mode = 1;
        } else if (!strncmp(found, "passive", 7)) {
            oam_req->mode = 0;
        } else if (!strncmp(found, "local-info", 10)) {
            oam_req->variable_choice = 1;
        } else if (!strncmp(found, "remote-info", 11)) {
            oam_req->variable_choice = 2;
        } else if (!strncmp(found, "link_fault", 10)) {
            oam_req->flags = VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT;
        } else if (!strncmp(found, "dying_gasp", 10)) {
            oam_req->flags = VTSS_ETH_LINK_OAM_FLAG_DYING_GASP;
        } else if (!strncmp(found, "crit_event", 10)) {
            oam_req->flags = VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT;
        } else if (!strncmp(found, "enable", 6)) {
            oam_req->flags_cntrl = VTSS_LINK_OAM_FLAG_CONTROL_ENABLE;
        } else if (!strncmp(found, "disable", 7)) {
            oam_req->flags_cntrl = VTSS_LINK_OAM_FLAG_CONTROL_DISABLE;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_eth_link_oam_parse_mode (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                            cli_req_t *req)
{
    //eth_link_oam_cli_req_t *oam_req = NULL;
    ulong error = 0;

    req->parm_parsed = 1;
    //oam_req = req->module_req;
    error =  cli_eth_link_oam_parse_keyword(cmd, cmd2, stx, cmd_org, req);
    return (error);
}

static void cli_eth_link_oam_link_monitoring_error_frame_event_default_set ( cli_req_t *req )
{
    eth_link_oam_cli_req_t *oam_req = NULL;

    oam_req = req->module_req;
    oam_req->link_monitoring_event = VTSS_LINK_OAM_LINK_MONITORING_FRAME_ERROR_EVENT ;
}

static void cli_eth_link_oam_link_monitoring_symbol_period_error_event_default_set ( cli_req_t *req )
{
    eth_link_oam_cli_req_t *oam_req = NULL;

    oam_req = req->module_req;
    oam_req->link_monitoring_event = VTSS_LINK_OAM_LINK_MONITORING_SYMBOL_PERIOD_ERROR_EVENT;
}

#if defined(RX_THRESHOLD_SUPPORT)
static void cli_eth_link_oam_link_monitoring_frame_period_error_event_default_set ( cli_req_t *req )
{
    eth_link_oam_cli_req_t *oam_req = NULL;

    oam_req = req->module_req;
    oam_req->link_monitoring_event = VTSS_LINK_OAM_LINK_MONITORING_FRAME_PERIOD_ERROR_EVENT;
}
#endif

static void cli_eth_link_oam_link_monitoring_error_frame_secs_summary_event_default_set ( cli_req_t *req )
{
    eth_link_oam_cli_req_t *oam_req = NULL;

    oam_req = req->module_req;
    oam_req->link_monitoring_event = VTSS_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUMMARY_EVENT;
}

static cli_parm_t eth_link_oam_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable Link OAM\n"
        "disable: Disable Link OAM",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_eth_link_oam_control_conf,
    },

    {
        "enable|disable",
        "enable : Enable MIB retrival support\n"
        "disable: Disable MIB retrival support",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_eth_link_oam_mib_retrival_conf,
    },

    {
        "enable|disable",
        "enable : Enable remote loopback support\n"
        "disable: Disable remote loopback support",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_eth_link_oam_remote_loopback_conf,
    },

    {
        "enable|disable",
        "enable : Enable remote loopback operation\n"
        "disable: Disable remote loopback operation",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_eth_link_oam_remote_loopback_oper_conf_set,
    },

    {
        "enable|disable",
        "enable : Enable link monitoring support\n"
        "disable: Disable link monitoring support",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_eth_link_oam_link_monitoring_conf,
    },

    {
        "local-info|remote-info",
        "local-info : Enable MIB retrival support\n"
        "remote-info: Disable MIB retrival support",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_mode,
        cli_cmd_eth_link_oam_mib_retrival_oper_conf_set,
    },


    {
        "active|passive",
        "active : Enable Link OAM Active mode\n"
        "passive: Disable Link OAM passive mode",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_mode,
        cli_cmd_eth_link_oam_mode_conf,
    },
    {
        "link_fault|dying_gasp|crit_event",
        "link_fault : Simulate Link fault event\n"
        "dying_gasp : Simulate Dying Gasp event\n"
        "crit_event : Simulate Critical event",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_mode,
        cli_cmd_eth_link_oam_port_flags_control,
    },
    {
        "enable|disable",
        "enable : Enable the flag\n"
        "disable: Disable the flag\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_mode,
        cli_cmd_eth_link_oam_port_flags_control,
    },
    {
        "<error_window>",
        "error_window: Duration of the monitoring period in terms of seconds.\n"
        "              Range (1 - 60 seconds). Default 1 second.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_window,
        cli_cmd_eth_link_oam_link_monitoring_error_frame_event_conf,
    },
    {
        "<error_window>",
        "error_window: Duration of the monitoring in terms of seconds.\n"
        "              Range (1 - 60 seconds). Default 1 second.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_window,
        cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf,
    },
#if defined(RX_THRESHOLD_SUPPORT)
    {
        "<period_threshold>",
        "period_threshold: Duration of the period in terms of ms intervals",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_window,
        cli_cmd_eth_link_oam_link_monitoring_frame_period_error_event_conf,
    },
#endif /* RX_THRESHOLD_SUPPORT */
    {
        "<error_window>",
        "error_window: Duration of the monitoring period in terms of seconds.\n"
        "              Range (10 - 900 seconds). Default 10 seconds.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_window,
        cli_cmd_eth_link_oam_link_monitoring_error_frame_secs_summary_event_conf,
    },
    {
        "<error_threshold>",
        "error_threshold: Number of permissible errors frames in the period defined by error_window.\n"
        "                 Range (0 - 0xffffffff) frames. Default 0 frames.\n"
        "                 Upbound value is not specified in the standard.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_error_threshold,
        cli_cmd_eth_link_oam_link_monitoring_error_frame_event_conf,
    },
    {
        "<error_threshold>",
        "error_threshold: Number of permissible error symbols in the period defined by error_window.\n"
        "                 Range (0 - 0xffffffff) symbols. Default 0 symbols.\n"
        "                 Upbound value is not specified in the standard.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_error_threshold,
        cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf,
    },
#if defined(RX_THRESHOLD_SUPPORT)
    {
        "<error_threshold>",
        "error_threshold: Number of permissible error frames in the period defined by period_threshold and rx_threshold",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_error_threshold,
        cli_cmd_eth_link_oam_link_monitoring_frame_period_error_event_conf,
    },
#endif /* RX_THRESHOLD_SUPPORT */
    {
        "<error_threshold>",
        "error_threshold: Number of permissible Error Frame Seconds in the period defined by error_window\n"
        "                 Range (0 - 0xffff) errored seconds. Default 1 errored second.\n"
        "                 Upbound value is not specified in the standard.",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_error_threshold,
        cli_cmd_eth_link_oam_link_monitoring_error_frame_secs_summary_event_conf,
    },
#if defined(RX_THRESHOLD_SUPPORT)
    {
        "<rx_threshold>",
        "rx_threshold : Number of received symbols in 'error_window' period\n"
        "               Range (0 - )"
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_rxpacket_threshold,
        cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf,
    },
    {
        "<rx_threshold>",
        "rx_threshold : Number of allowed 'error_threshold' for received frames (Range: 0 - )",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eth_link_oam_parse_rxpacket_threshold,
        cli_cmd_eth_link_oam_link_monitoring_frame_period_error_event_conf,
    },
#endif /* RX_THRESHOLD_SUPPORT */

    {NULL, NULL, 0, 0, NULL}
};

enum {
    PRIO_ETH_LINK_OAM_CONTROL,
    PRIO_ETH_LINK_OAM_MODE,
    PRIO_ETH_LINK_OAM_MIB_RETRIVAL,
    PRIO_ETH_LINK_OAM_VAR_RETRIVAL_OPR,
    PRIO_ETH_LINK_OAM_REMOTE_LOOPBACK,
    PRIO_ETH_LINK_OAM_REMOTE_LOOPBACK_OPER,
    PRIO_ETH_LINK_OAM_LINK_MONITOR,
    PRIO_ETH_LINK_OAM_PORT_FLAGS_CONTROL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_ETH_LINK_OAM_ERROR_FRAME_CONTROL,
    PRIO_ETH_LINK_OAM_SYMBOL_PERIOD_ERROR_EVENT_CONTROL,
    PRIO_ETH_LINK_OAM_FRAME_PERIOD_ERROR_EVENT_CONTROL,
    PRIO_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_CONTROL,
    PRIO_ETH_LINK_OAM_STATUS,
    PRIO_ETH_LINK_OAM_LINK_MONITOR_STATUS,
    PRIO_ETH_LINK_OAM_STATS,
};

cli_cmd_tab_entry (
    "LOAM Control [<port_list>] [enable|disable]",
    NULL,
    "Set or show Link OAM Control capability",
    PRIO_ETH_LINK_OAM_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_control_conf,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LOAM Mib-retrival-support [<port_list>] [enable|disable]",
    NULL,
    "Set or show MIB retrival support",
    PRIO_ETH_LINK_OAM_MIB_RETRIVAL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_mib_retrival_conf,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LOAM Remote_loopback_support [<port_list>] [enable|disable]",
    NULL,
    "Set or show remote loopback support",
    PRIO_ETH_LINK_OAM_REMOTE_LOOPBACK,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_remote_loopback_conf,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LOAM Link_monitoring_support [<port_list>] [enable|disable]",
    NULL,
    "Set or show link monitoring support",
    PRIO_ETH_LINK_OAM_LINK_MONITOR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_conf,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    NULL,
    "LOAM Frame_error_event [<port_list>] [<error_window>] [<error_threshold>]",
    "Set (or) Show Parameters for Frame Error Event",
    PRIO_ETH_LINK_OAM_ERROR_FRAME_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_error_frame_event_conf,
    cli_eth_link_oam_link_monitoring_error_frame_event_default_set,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

#if defined(RX_THRESHOLD_SUPPORT)
cli_cmd_tab_entry (
    NULL,
    "LOAM Symbol_period_error_event [<port_list>] [<error_window>] [<error_threshold>] [<rx_threshold>]",
    "Set (or) Show Parameters for Symbol Period Error Event",
    PRIO_ETH_LINK_OAM_SYMBOL_PERIOD_ERROR_EVENT_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf,
    cli_eth_link_oam_link_monitoring_symbol_period_error_event_default_set,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    NULL,
    "LOAM Frame_period_error_event [<port_list>] [<period_threshold>] [<error_threshold>] [<rx_threshold>]",
    "Set (or) Show Parameters for Frame Period Error Event",
    PRIO_ETH_LINK_OAM_FRAME_PERIOD_ERROR_EVENT_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_frame_period_error_event_conf,
    cli_eth_link_oam_link_monitoring_frame_period_error_event_default_set,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);
#else
cli_cmd_tab_entry (
    NULL,
    "LOAM Symbol_period_error_event [<port_list>] [<error_window>] [<error_threshold>]",
    "Set (or) Show Parameters for Symbol Period Error Event. We treat CRC erros as symbol errors",
    PRIO_ETH_LINK_OAM_SYMBOL_PERIOD_ERROR_EVENT_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_symbol_period_error_event_conf,
    cli_eth_link_oam_link_monitoring_symbol_period_error_event_default_set,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);
#endif /* RX_THRESHOLD_SUPPORT */

cli_cmd_tab_entry (
    NULL,
    "LOAM Frame_error_seconds_summary_event [<port_list>] [<error_window>] [<error_threshold>] ",
    "Set (or) Show Parameters for Frame Error Seconds Summary Event",
    PRIO_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitoring_error_frame_secs_summary_event_conf,
    cli_eth_link_oam_link_monitoring_error_frame_secs_summary_event_default_set,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);


cli_cmd_tab_entry (
    "LOAM Variable-retrieve [<port_list>] [local-info|remote-info]",
    NULL,
    "Set or show MIB retrieval support",
    PRIO_ETH_LINK_OAM_VAR_RETRIVAL_OPR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_mib_retrival_oper_conf_set,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LOAM Remote_loopback_oper [<port_list>] [enable|disable]",
    NULL,
    "Set or show remote loopback operation",
    PRIO_ETH_LINK_OAM_REMOTE_LOOPBACK_OPER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_remote_loopback_oper_conf_set,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LOAM Mode [<port_list>] [active|passive]",
    NULL,
    "Set or show Link OAM mode",
    PRIO_ETH_LINK_OAM_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_mode_conf,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "LOAM Status [<port_list>]",
    NULL,
    "Show Link OAM port status",
    PRIO_ETH_LINK_OAM_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_status,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "LOAM Link_monitor_status [<port_list>]",
    NULL,
    "Show Link OAM port link monitoring status",
    PRIO_ETH_LINK_OAM_LINK_MONITOR_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_link_monitor_status,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    "LOAM Statistics [<port_list>] [clear]",
    NULL,
    "Show Link OAM port statistics",
    PRIO_ETH_LINK_OAM_STATS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    cli_cmd_eth_link_oam_stats,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug LOAM [<port_list>] [link_fault|dying_gasp|crit_event] [enable|disable]",
    NULL,
    "Enable/Disable or show the Link OAM port flags",
    PRIO_ETH_LINK_OAM_PORT_FLAGS_CONTROL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_eth_link_oam_port_flags_control,
    NULL,
    eth_link_oam_cli_parm_table,
    CLI_CMD_FLAG_NONE
);











