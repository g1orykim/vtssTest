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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "dot1x_api.h"
#include "dot1x_trace.h" // For trace

/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/
// Enum for selecting which status or configuration to access
typedef enum {
    STATISTICS,      // Displaying statistics
    STATUS,          // Displaying status
    GLOBAL_CONF,     // Configuring global configuration
    INTERFACE_CONF   // Configuring interfaces
} dot1x_cmd_t;

// Enum for selecting which statistics, status or configuration command to execute
typedef enum {
    STATIS_CLEAR,         // Clear statistics
    STATIS_EAPOL,         // Display eapol statistics
    STATIS_RADIUS,        // Display radius statistics
    STATIS_ALL,           // Display all statistics
    STATUS_BRIEF,         // Display status in brief format
    STATUS_FULL,          // Display status
    CONF_REAUTH_TIMER,    // Configure reauthentication timer
    CONF_TX_PERIOD,       // Configure TX-period timer
    CONF_INACTIVITY,      // Configure inactivity timer
    CONF_QUIET_PERIOD,    // Configure quite time (hold time)
    CONF_REAUTHENTICATION,// Configure reauthentication
    CONF_SYSTEM_AUTH_CONTROL,// Configure mode global enabled
    CONF_RE_AUTHENTICATE, // Restart re-authentication
    CONF_PORT_CONTROL,    // Configure admin state
    CONF_INTERFACE_GUEST_VLAN,   // Configure guest vlan for an interface
    CONF_INTERFACE_RADIUS_QOS,   // Configure guest radius-qos
    CONF_INTERFACE_RADIUS_VLAN,   // Configure guest radius-vlan
    CONF_GUEST_VLAN_SUPPLICANT,   // Configure guest vlan supplicant for
    CONF_GLOBAL,                  // Configure global enable / disable of features
    CONF_GUEST_VLAN,      // Configure guest vlan
    CONF_REAUTH_MAX,      // Configure maximum reauth.
    CONF_INITALIZE        // Force reauthentication immediately
} dot1x_sub_cmd_t;

// Used to passed the configuration to be changed.or status information to display
typedef struct {
    dot1x_cmd_t type;          // Which configuration to access
    dot1x_sub_cmd_t sub_type;  // More information about which configuration to access

    BOOL no; // "invert" the command (User no command)

    // New values
    union {
        u16 reauth_period_secs;
        u16 eapol_timeout_secs;
#ifdef NAS_USES_PSEC
        u32 psec_aging_period_secs;
        u32 psec_hold_time_secs;
#endif
        nas_port_control_t admin_state;
        vtss_vid_t guest_vid;
        u32 reauth_max;
        struct {
            BOOL radius_qos;
            BOOL radius_vlan;
            BOOL guest_vlan;
        } global;
    } value;
} dot1x_icli_cmd_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/

// Function for printing statistics
// In - session_id - For iCLI printing
//      sit        - Switch information.
//      pit        - Port information
//      cmd        - Information about what to print.
static vtss_rc dot1x_icli_cmd_print_stat(i32 session_id, const switch_iter_t *sit, const port_iter_t *pit, const dot1x_icli_cmd_t *cmd, const dot1x_switch_status_t *switch_status)
{
    i8 interface_str_buf[50];
    if (cmd->sub_type == STATIS_CLEAR) {
        VTSS_RC(dot1x_mgmt_statistics_clear(sit->isid, pit->iport, NULL));
        return VTSS_RC_OK;
    }

    dot1x_statistics_t    stat;
    VTSS_RC(dot1x_mgmt_statistics_get(sit->isid, pit->iport, NULL, &stat));

#ifdef NAS_MULTI_CLIENT
    BOOL client_found = TRUE, client_first = TRUE;
    while (client_found) {
        if (switch_status->status[pit->iport - VTSS_PORT_NO_START] >= NAS_PORT_STATUS_CNT) {
            if (dot1x_mgmt_multi_client_statistics_get(sit->isid, pit->iport, &stat, &client_found, client_first) != VTSS_RC_OK) {
                continue;
            }
            if (!client_found) {
                if (client_first) {
                    memset(&stat, 0, sizeof(stat)); // Gotta display something anyway, since it's the first time
                } else {
                    continue;
                }
            }
            client_first = FALSE;
        } else {
            client_found = FALSE; // Gotta stop after this iteration when not MAC-based
        }
#else
    {
#endif
        if (cmd->sub_type == STATIS_EAPOL) {
            if (pit->first) {
                ICLI_PRINTF(                  "                       Rx      Tx      Rx      Tx      Rx      Tx      Rx      Rx      Rx\n");
                icli_table_header(session_id, "Interface              Total   Total   RespId  ReqId   Resp    Req     Start   Logoff  Error");
            }
            ICLI_PRINTF("%-23s%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u\n",
                        icli_port_info_txt(sit->usid, pit->uport, interface_str_buf),
                        stat.eapol_counters.dot1xAuthEapolFramesRx,
                        stat.eapol_counters.dot1xAuthEapolFramesTx,
                        stat.eapol_counters.dot1xAuthEapolRespIdFramesRx,
                        stat.eapol_counters.dot1xAuthEapolReqIdFramesTx,
                        stat.eapol_counters.dot1xAuthEapolRespFramesRx,
                        stat.eapol_counters.dot1xAuthEapolReqFramesTx,
                        stat.eapol_counters.dot1xAuthEapolStartFramesRx,
                        stat.eapol_counters.dot1xAuthEapolLogoffFramesRx,
                        stat.eapol_counters.dot1xAuthInvalidEapolFramesRx +
                        stat.eapol_counters.dot1xAuthEapLengthErrorFramesRx);
        } else if (cmd->sub_type == STATIS_RADIUS) {
#ifdef NAS_MULTI_CLIENT
            if (pit->first) {
                ICLI_PRINTF(                  "                       Rx Access   Rx Other    Rx Auth.    Rx Auth.    Tx          MAC\n");
                icli_table_header(session_id, "Interface              Challenges  Requests    Successes   Failures    Responses   Address");
            }
#else
            if (pit->first) {
                ICLI_PRINTF(                  "                       Rx Access   Rx Other    Rx Auth.    Rx Auth.    Tx\n");
                icli_table_header(session_id, "Interface              Challenges  Requests    Successes   Failures    Responses");

            }
#endif
            ICLI_PRINTF("%-23s%-12u%-12u%-12u%-12u%-12u",
                        icli_port_info_txt(sit->usid, pit->uport, interface_str_buf),
                        stat.backend_counters.backendAccessChallenges,
                        stat.backend_counters.backendOtherRequestsToSupplicant,
                        stat.backend_counters.backendAuthSuccesses,
                        stat.backend_counters.backendAuthFails,
                        stat.backend_counters.backendResponses);

#ifdef NAS_MULTI_CLIENT
            ICLI_PRINTF("%s", strlen(stat.client_info.mac_addr_str) ? (char *)stat.client_info.mac_addr_str : "-");
#endif
            ICLI_PRINTF("\n");

        } else {
#ifdef NAS_MULTI_CLIENT
            if (switch_status->status[pit->iport - VTSS_PORT_NO_START] < NAS_PORT_STATUS_CNT)
#endif
            {
                ICLI_PRINTF("%s%s EAPOL Statistics:\n\n", pit->first ? "" : "\n", icli_port_info_txt(sit->usid, pit->uport, interface_str_buf));
                icli_cmd_stati(session_id, "Total",          "Total",      stat.eapol_counters.dot1xAuthEapolFramesRx,          stat.eapol_counters.dot1xAuthEapolFramesTx);
                icli_cmd_stati(session_id, "Response/Id",    "Request/Id", stat.eapol_counters.dot1xAuthEapolRespIdFramesRx,    stat.eapol_counters.dot1xAuthEapolReqIdFramesTx);
                icli_cmd_stati(session_id, "Response",       "Request",    stat.eapol_counters.dot1xAuthEapolRespFramesRx,      stat.eapol_counters.dot1xAuthEapolReqFramesTx);
                icli_cmd_stati(session_id, "Start",          NULL,         stat.eapol_counters.dot1xAuthEapolStartFramesRx,     0);
                icli_cmd_stati(session_id, "Logoff",         NULL,         stat.eapol_counters.dot1xAuthEapolLogoffFramesRx,    0);
                icli_cmd_stati(session_id, "Invalid Type",   NULL,         stat.eapol_counters.dot1xAuthInvalidEapolFramesRx,   0);
                icli_cmd_stati(session_id, "Invalid Length", NULL,         stat.eapol_counters.dot1xAuthEapLengthErrorFramesRx, 0);
            }
#ifdef NAS_MULTI_CLIENT
            if (switch_status->status[pit->iport - VTSS_PORT_NO_START] >= NAS_PORT_STATUS_CNT && strlen(stat.client_info.mac_addr_str)) {
                ICLI_PRINTF("\n%s Backend Server Statistics (MAC Address %s):\n\n", icli_port_info_txt(sit->usid, pit->uport, interface_str_buf), stat.client_info.mac_addr_str);
            } else
#endif
            {
                ICLI_PRINTF("\n%s Backend Server Statistics:\n\n", icli_port_info_txt(sit->usid, pit->uport, interface_str_buf));
            }

            icli_cmd_stati(session_id, "Access Challenges", "Responses", stat.backend_counters.backendAccessChallenges,          stat.backend_counters.backendResponses);
            icli_cmd_stati(session_id, "Other Requests",    NULL,        stat.backend_counters.backendOtherRequestsToSupplicant, 0);
            icli_cmd_stati(session_id, "Auth. Successes",   NULL,        stat.backend_counters.backendAuthSuccesses,             0);
            icli_cmd_stati(session_id, "Auth. Failures",    NULL,        stat.backend_counters.backendAuthFails,                 0);
        } /* !eapol && !radius only */
#ifdef NAS_MULTI_CLIENT
        //      first = FALSE; // Needed in case we're doing MAC-based Authentication with several clients on one port.
#endif
    } /* while (client_found) */
    return VTSS_RC_OK;
}

// Function for printing status in brief format
// In - session_id - For iCLI printing
//      sit        - Switch information.
//      pit        - Port information
//      cmd        - Information about what to print.
static vtss_rc dot1x_icli_cmd_print_status_brief(i32 session_id, const switch_iter_t *sit, const port_iter_t *pit, const dot1x_icli_cmd_t *cmd, const dot1x_switch_status_t *switch_status)
{
    i8 str_buf[85], interface_str_buf[20];
    nas_port_status_t  port_status;
    char               *buf_ptr;
    char               buf[50];
    dot1x_switch_cfg_t switch_cfg;
    dot1x_statistics_t stat;

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN) || defined(VTSS_SW_OPTION_NAS_GUEST_VLAN)
    vtss_vid_t vid;
#endif

    strcpy(&buf[0], "");

    VTSS_RC(dot1x_mgmt_switch_cfg_get(sit->isid, &switch_cfg));


    if (sit->isid == VTSS_ISID_LOCAL) {
        memset(&stat, 0, sizeof(stat));
    } else if (dot1x_mgmt_statistics_get(sit->isid, pit->iport, NULL, &stat) != VTSS_RC_OK) {
        T_I_PORT(pit->iport, "For MAC-based ports, the statistics counters are not valid. See below when statistics is printed");
        return VTSS_RC_OK;
    }

    if (pit->first) {
        if (!sit->first) {
            ICLI_PRINTF("\n");
        }

        (void) snprintf (&str_buf[0], sizeof(str_buf), "%-10s%-7s%-12s%-19s%-19s", "Inf", "Admin", "Port State", "Last Src", "Last ID");

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-6s", &str_buf[0], "QOS");
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-6s", &str_buf[0], "VLAN");
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-6s", &str_buf[0], "Guest");
#endif

        icli_table_header(session_id, &str_buf[0]);
    }


    port_status = switch_status->status[pit->iport - VTSS_PORT_NO_START];

    buf_ptr = port_status == NAS_PORT_STATUS_LINK_DOWN    ? "Down"     :
              port_status == NAS_PORT_STATUS_AUTHORIZED   ? "Auth"     :
              port_status == NAS_PORT_STATUS_UNAUTHORIZED ? "UnAuth"   :
              port_status == NAS_PORT_STATUS_DISABLED     ? "Disabled" : "?";

#ifdef NAS_MULTI_CLIENT
    dot1x_port_cfg_t  *port_cfg = &switch_cfg.port_cfg[pit->iport - VTSS_PORT_NO_START];
    if (port_status >= NAS_PORT_STATUS_CNT) {
        u32 auth_cnt, unauth_cnt;
        dot1x_mgmt_decode_auth_unauth(port_status, &auth_cnt, &unauth_cnt);

        // Avoid compiler-warning when neither is defined (in which case we wouldn't get here)
        buf[0] = '\0';

        if (NAS_PORT_CONTROL_IS_MULTI_CLIENT(port_cfg->admin_state)) {
            sprintf(buf, "%u A/%u unA", auth_cnt, unauth_cnt);
        }
        buf_ptr = buf;
    }
#endif

    ICLI_PRINTF("%-10s%-7s%-12s%-19s%-19s",
                icli_port_info_txt_short(sit->usid, pit->uport, interface_str_buf),
                sit->isid == VTSS_ISID_LOCAL ? "-" :
                dot1x_port_control_to_str(port_cfg->admin_state, TRUE),
                buf_ptr,
                strlen(stat.client_info.mac_addr_str) ? (char *)stat.client_info.mac_addr_str : "-",
                strlen(stat.client_info.identity)     ? (char *)stat.client_info.identity     : "-");


#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    dot1x_qos_class_to_str(switch_status->qos_class[pit->iport - VTSS_PORT_NO_START], buf);
    ICLI_PRINTF("%-6s", buf);
#endif

#ifdef NAS_USES_VLAN
    T_NG_PORT(TRACE_GRP_ICLI, pit->iport, "switch_status->vid[pit->iport - VTSS_PORT_NO_START]:%d", switch_status->vid[pit->iport - VTSS_PORT_NO_START]);
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
    if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
        vid = 0;
    }
    dot1x_vlan_to_str(vid, buf);
    ICLI_PRINTF("%-6s", buf);
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
    if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_GUEST_VLAN) {
        vid = 0;
    }

    T_IG_PORT(TRACE_GRP_ICLI, pit->iport, "vlan_type:%d, vid:%d" , switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START], vid);
    dot1x_vlan_to_str(vid, buf);
    ICLI_PRINTF("%-6s", buf);
#endif

    ICLI_PRINTF("\n");
    return VTSS_RC_OK;
}

// Function for printing status
// In - session_id - For iCLI printing
//      sit        - Switch information.
//      pit        - Port information
//      cmd        - Information about what to print.
static vtss_rc dot1x_icli_cmd_print_status(i32 session_id, const switch_iter_t *sit, const port_iter_t *pit, const dot1x_icli_cmd_t *cmd, const dot1x_switch_status_t *switch_status)
{
    i8 str_buf[200], interface_str_buf[50];
    nas_port_status_t  port_status;
    char               *buf_ptr;
    char               buf[50];
    dot1x_switch_cfg_t switch_cfg;
    dot1x_statistics_t stat;

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN) || defined(VTSS_SW_OPTION_NAS_GUEST_VLAN)
    char       vid_qos_buf[10];
    vtss_vid_t vid;
#endif

    strcpy(&buf[0], "");

    VTSS_RC(dot1x_mgmt_switch_cfg_get(sit->isid, &switch_cfg));


    if (sit->isid == VTSS_ISID_LOCAL) {
        memset(&stat, 0, sizeof(stat));
    } else if (dot1x_mgmt_statistics_get(sit->isid, pit->iport, NULL, &stat) != VTSS_RC_OK) {
        T_I_PORT(pit->iport, "For MAC-based ports, the statistics counters are not valid. See below when statistics is printed");
        return VTSS_RC_OK;
    }

    if (!sit->first) {
        ICLI_PRINTF("\n");
    }

    (void) snprintf (&interface_str_buf[0], sizeof(interface_str_buf), "%s :", icli_port_info_txt(sit->usid, pit->uport, interface_str_buf));
    icli_table_header(session_id, &interface_str_buf[0]);

    (void) snprintf (&str_buf[0], sizeof(str_buf), "%-20s%-23s%-19s%-15s", "Admin State", "Port State", "Last Source", "Last ID");

    icli_table_header(session_id, &str_buf[0]);

    port_status = switch_status->status[pit->iport - VTSS_PORT_NO_START];

    buf_ptr = port_status == NAS_PORT_STATUS_LINK_DOWN    ? "Link Down"         :
              port_status == NAS_PORT_STATUS_AUTHORIZED   ? "Authorized"        :
              port_status == NAS_PORT_STATUS_UNAUTHORIZED ? "Unauthorized"      :
              port_status == NAS_PORT_STATUS_DISABLED     ? "Globally Disabled" : "?";

#ifdef NAS_MULTI_CLIENT
    dot1x_port_cfg_t  *port_cfg = &switch_cfg.port_cfg[pit->iport - VTSS_PORT_NO_START];
    if (port_status >= NAS_PORT_STATUS_CNT) {
        u32 auth_cnt, unauth_cnt;
        dot1x_mgmt_decode_auth_unauth(port_status, &auth_cnt, &unauth_cnt);

        // Avoid compiler-warning when neither is defined (in which case we wouldn't get here)
        buf[0] = '\0';

        if (NAS_PORT_CONTROL_IS_MULTI_CLIENT(port_cfg->admin_state)) {
            sprintf(buf, "%u Auth/%u Unauth", auth_cnt, unauth_cnt);
        }
        buf_ptr = buf;
    }
#endif

    ICLI_PRINTF("%-20s%-23s%-19s%-15s\n",
                sit->isid == VTSS_ISID_LOCAL ? "-" :
                dot1x_port_control_to_str(port_cfg->admin_state, FALSE),
                buf_ptr,
                strlen(stat.client_info.mac_addr_str) ? (char *)stat.client_info.mac_addr_str : "-",
                strlen(stat.client_info.identity)     ? (char *)stat.client_info.identity     : "-");


    strcpy(&str_buf[0], "");
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-22s", &str_buf[0], "Current Radius QOS");
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-22s", &str_buf[0], "Current Radius VLAN");
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    (void) snprintf(&str_buf[0], sizeof(str_buf), "%s%-22s", &str_buf[0], "Current Guest VLAN");
#endif

    strcpy(buf, "");

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    dot1x_qos_class_to_str(switch_status->qos_class[pit->iport - VTSS_PORT_NO_START], vid_qos_buf);
    (void) snprintf(&buf[0], sizeof(buf), "%s%-22s", &buf[0], &vid_qos_buf[0]);
#endif


#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
    if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
        vid = 0;
    }
    dot1x_vlan_to_str(vid, vid_qos_buf);
    (void) snprintf(&buf[0], sizeof(buf), "%s%-22s", &buf[0], &vid_qos_buf[0]);
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
    if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_GUEST_VLAN) {
        vid = 0;
    }
    dot1x_vlan_to_str(vid, vid_qos_buf);
    (void) snprintf(&buf[0], sizeof(buf), "%s%-22s", &buf[0], &vid_qos_buf[0]);
#endif

#if defined(VTSS_SW_OPTION_NAS_GUEST_VLAN) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS)
    ICLI_PRINTF("\n");
    icli_table_header(session_id, &str_buf[0]);
    ICLI_PRINTF("%-22s\n", buf);
#endif

    ICLI_PRINTF("\n");
    return VTSS_RC_OK;
}

// Function for looping over all switches and all ports a the port list, and then calling a configuration or status/statistics function.
// In : session_id - For printing
//      plist      - Containing information about which switches and ports to "access"
//      cmd        - Containing information about which function to call.
static vtss_rc dot1x_icli_sit_port_loop(i32 session_id, icli_stack_port_range_t *plist, const dot1x_icli_cmd_t *cmd)
{
    switch_iter_t sit;

    if (cmd->type == STATISTICS || cmd->type == STATUS) {
        // Only existing switches, in USID order.
        VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    } else {
        // Configurable switches, in USID order.
        VTSS_RC(icli_switch_iter_init(&sit));
    }
    T_NG(TRACE_GRP_ICLI, "type:%d", cmd->type);
    while (icli_switch_iter_getnext(&sit, plist)) {
        dot1x_switch_status_t switch_status;
        T_DG(TRACE_GRP_ICLI, "isid:%d", sit.isid);
        VTSS_RC(dot1x_mgmt_switch_status_get(sit.isid, &switch_status))

        // Loop through all ports
        port_iter_t pit;
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
        while (icli_port_iter_getnext(&pit, plist)) {
            T_DG_PORT(TRACE_GRP_ICLI, pit.iport, "type:%d", cmd->type);
            dot1x_switch_cfg_t    switch_cfg, switch_cfg_default;
            switch (cmd->type) {
            case STATISTICS:
                VTSS_RC(dot1x_icli_cmd_print_stat(session_id, &sit, &pit, cmd, &switch_status));
                break;

            case STATUS:
                if (cmd->sub_type == STATUS_BRIEF) {
                    VTSS_RC(dot1x_icli_cmd_print_status_brief(session_id, &sit, &pit, cmd, &switch_status));
                } else {
                    VTSS_RC(dot1x_icli_cmd_print_status(session_id, &sit, &pit, cmd, &switch_status));
                }
                break;

            case INTERFACE_CONF:
                // Get current configuration
                VTSS_RC(dot1x_mgmt_switch_cfg_get(sit.isid, &switch_cfg));

                // Get default configuration
                DOT1X_cfg_default_switch(&switch_cfg_default);

                switch (cmd->sub_type) {
                case CONF_RE_AUTHENTICATE:
                    VTSS_RC(dot1x_mgmt_reauth(sit.isid, pit.iport, FALSE));
                    break;

                case CONF_INITALIZE:
                    VTSS_RC(dot1x_mgmt_reauth(sit.isid, pit.iport, FALSE));
                    break;

                case CONF_PORT_CONTROL:
                    if (cmd->no) {
                        switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state = switch_cfg_default.port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state;
                    } else {
                        switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state = cmd->value.admin_state;
                    }
                    break;
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                case CONF_INTERFACE_GUEST_VLAN:
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].guest_vlan_enabled = !cmd->no;
                    break;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                case CONF_INTERFACE_RADIUS_QOS:
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].qos_backend_assignment_enabled = !cmd->no;
                    break;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                case CONF_INTERFACE_RADIUS_VLAN:
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].vlan_backend_assignment_enabled = !cmd->no;
                    break;
#endif
                default:
                    T_E("Unexpected subtype:%d", cmd->sub_type);
                }

                // Update with new configuration
                VTSS_RC(dot1x_mgmt_switch_cfg_set(sit.isid, &switch_cfg));
                break;
            default:
                T_E("Unexpected cmd type:%d", cmd->type);
                return VTSS_RC_ERROR;
            }
        }
    }
    return VTSS_RC_OK;
}

// Function for configuring global enable / disable of features
// IN - Session_Id       - For printing
//     cmd               - Containing which configuration to set, and the value.
//     glbl_cfg          - New configuration
//     glbl_cfg_default  - Default configuration
// Return - Vitsse error code

static void conf_global(const dot1x_icli_cmd_t *cmd, dot1x_glbl_cfg_t *glbl_cfg, dot1x_glbl_cfg_t *glbl_cfg_default)
{
    // all globals are BOOLEANs
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    if (cmd->value.global.guest_vlan) {
        if (cmd->no) {
            glbl_cfg->guest_vlan_enabled = glbl_cfg_default->guest_vlan_enabled;
        } else {
            glbl_cfg->guest_vlan_enabled = !glbl_cfg_default->guest_vlan_enabled;
        }
    }
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    if (cmd->value.global.radius_vlan) {
        if (cmd->no) {
            glbl_cfg->vlan_backend_assignment_enabled = glbl_cfg_default->vlan_backend_assignment_enabled;
        } else {
            glbl_cfg->vlan_backend_assignment_enabled = !glbl_cfg_default->vlan_backend_assignment_enabled;
        }
    }
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    if (cmd->value.global.radius_qos) {
        if (cmd->no) {
            glbl_cfg->qos_backend_assignment_enabled = glbl_cfg_default->qos_backend_assignment_enabled;
        } else {
            glbl_cfg->qos_backend_assignment_enabled = !glbl_cfg_default->qos_backend_assignment_enabled;
        }
    }
#endif
}

// Function for configuring global settings
// IN - Session_Id - For printing
//     cmd         - Containing which configuration to set, and the value.
// Return - Vitsse error code
static vtss_rc dot1x_icli_global_conf(i32 session_id, const dot1x_icli_cmd_t *cmd)
{
    dot1x_glbl_cfg_t      glbl_cfg, glbl_cfg_default;

    // Get current configuration
    VTSS_RC(dot1x_mgmt_glbl_cfg_get(&glbl_cfg));
    DOT1X_cfg_default_glbl(&glbl_cfg_default); // Get default settings

    T_IG(TRACE_GRP_ICLI, "type:%d, sub_type:%d, no:%d", cmd->type, cmd->sub_type, cmd->no); ;
    switch (cmd->type) {
    case GLOBAL_CONF:
        switch (cmd->sub_type) {
        case CONF_REAUTH_TIMER:
            if (cmd->no) {
                glbl_cfg.reauth_period_secs = glbl_cfg_default.reauth_period_secs;
            } else {
                glbl_cfg.reauth_period_secs = cmd->value.reauth_period_secs;
            }
            T_IG(TRACE_GRP_ICLI, "glbl_cfg.reauth_period_secs:%d", glbl_cfg.reauth_period_secs);
            break;
#ifdef NAS_USES_PSEC
        case CONF_INACTIVITY:
            if (cmd->no) {
                glbl_cfg.psec_aging_period_secs = glbl_cfg_default.psec_aging_period_secs;
            } else {
                glbl_cfg.psec_aging_period_secs = cmd->value.psec_aging_period_secs;
            }
            T_IG(TRACE_GRP_ICLI, "psec_aging_period_secs:%d", glbl_cfg.psec_aging_period_secs);
            break;
#endif
        case CONF_QUIET_PERIOD:
            if (cmd->no) {
                glbl_cfg.psec_hold_time_secs = glbl_cfg_default.psec_hold_time_secs;
            } else {
                glbl_cfg.psec_hold_time_secs = cmd->value.psec_hold_time_secs;
            }
            break;
        case CONF_TX_PERIOD:
            if (cmd->no) {
                glbl_cfg.eapol_timeout_secs = glbl_cfg_default.eapol_timeout_secs;
            } else {
                glbl_cfg.eapol_timeout_secs = cmd->value.eapol_timeout_secs;
            }
            break;
        case CONF_REAUTHENTICATION:
            if (cmd->no) {
                glbl_cfg.reauth_enabled = glbl_cfg_default.reauth_enabled;
            } else {
                glbl_cfg.reauth_enabled = !glbl_cfg_default.reauth_enabled;
            }
            break;
        case CONF_SYSTEM_AUTH_CONTROL:
            // enable is a BOOLEAN
            if (cmd->no) {
                glbl_cfg.enabled = glbl_cfg_default.enabled;
            } else {
                glbl_cfg.enabled = !glbl_cfg_default.enabled;
            }
            break;
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        case CONF_GUEST_VLAN:
            if (cmd->no) {
                glbl_cfg.guest_vid = glbl_cfg_default.guest_vid;
            } else {
                glbl_cfg.guest_vid = cmd->value.guest_vid;
            }
            break;
        case CONF_GUEST_VLAN_SUPPLICANT:
            // Supplicant is a BOOLEAN
            if (cmd->no) {
                glbl_cfg.guest_vlan_allow_eapols = glbl_cfg_default.guest_vlan_allow_eapols;
            } else {
                glbl_cfg.guest_vlan_allow_eapols = !glbl_cfg_default.guest_vlan_allow_eapols;
            }
            break;
        case CONF_REAUTH_MAX:
            if (cmd->no) {
                glbl_cfg.reauth_max = glbl_cfg_default.reauth_max;
            } else {
                glbl_cfg.reauth_max = cmd->value.reauth_max;
            }
            T_IG(TRACE_GRP_ICLI, "reauth_max:%d", glbl_cfg.reauth_max);
            break;
#endif

        case CONF_GLOBAL:
            conf_global(cmd, &glbl_cfg, &glbl_cfg_default);
            break;

        default:
            T_E("Unexpected sub type:%d", cmd->sub_type);
        }
        break;
    default:
        T_E("cmd type should always be GLOBAL_CONF here, was :%d", cmd->type);
        return VTSS_RC_ERROR;
    }

    // Update with new configuration
    VTSS_RC(dot1x_mgmt_glbl_cfg_set(&glbl_cfg));

    return VTSS_RC_OK;
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
//  see port_icli_functions.h
vtss_rc dot1x_icli_statistics(i32 session_id, BOOL has_eapol, BOOL has_radius, BOOL has_all, icli_stack_port_range_t *plist)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = STATISTICS;
    if (has_all) {
        cmd.sub_type = STATIS_ALL;
    } else if (has_eapol) {
        cmd.sub_type = STATIS_EAPOL;
    } else if (has_radius) {
        cmd.sub_type = STATIS_RADIUS;
    } else {
        T_E("Sorry, don't know what to do, showing all");
        return VTSS_RC_ERROR;
    }

    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_status(i32 session_id, icli_stack_port_range_t *plist, BOOL has_brief)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = STATUS;
    if (has_brief) {
        cmd.sub_type = STATUS_BRIEF;
    } else {
        cmd.sub_type = STATUS_FULL;
    }
    T_NG(TRACE_GRP_ICLI, "Enter");
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_statistics_clr(i32 session_id, icli_stack_port_range_t *plist)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = STATISTICS;
    cmd.sub_type = STATIS_CLEAR;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_reauthenticate(i32 session_id, i32 value, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_REAUTH_TIMER;
    cmd.value.reauth_period_secs = value;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_tx_period(i32 session_id, i32 value, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_TX_PERIOD;
    cmd.value.eapol_timeout_secs = value;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

#ifdef NAS_USES_PSEC
//  see port_icli_functions.h
vtss_rc dot1x_icli_inactivity(i32 session_id, i32 value, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_INACTIVITY;
    cmd.value.psec_aging_period_secs = value;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}
#endif

//  see port_icli_functions.h
vtss_rc dot1x_icli_reauthentication(i32 session_id, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_REAUTHENTICATION;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_quiet_period(i32 session_id, i32 value, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_QUIET_PERIOD;
    cmd.value.psec_hold_time_secs = value;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_re_authenticate(i32 session_id, icli_stack_port_range_t *plist)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_RE_AUTHENTICATE;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_initialize(i32 session_id, icli_stack_port_range_t *plist)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_INITALIZE;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_system_auth_control(i32 session_id, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_SYSTEM_AUTH_CONTROL;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
//  see port_icli_functions.h
vtss_rc dot1x_icli_interface_guest_vlan(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_INTERFACE_GUEST_VLAN;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_max_reauth_req(i32 session_id, i32 value, BOOL no)
{
    T_IG(TRACE_GRP_ICLI, "no:%d, value:%d", no, value);
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_REAUTH_MAX;
    cmd.no = no;
    cmd.value.reauth_max = value;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_guest_vlan(i32 session_id, i32 value, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_GUEST_VLAN;
    cmd.no = no;
    cmd.value.guest_vid = value;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_guest_vlan_supplicant(i32 session_id, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_GUEST_VLAN_SUPPLICANT;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}
#endif

//  see port_icli_functions.h
vtss_rc dot1x_icli_radius_qos(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_INTERFACE_RADIUS_QOS;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_radius_vlan(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_INTERFACE_RADIUS_VLAN;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_global(i32 session_id, BOOL has_guest_vlan, BOOL has_radius_qos, BOOL has_radius_vlan, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = GLOBAL_CONF;
    cmd.sub_type = CONF_GLOBAL;
    cmd.no = no;

    cmd.value.global.guest_vlan  = has_guest_vlan;
    cmd.value.global.radius_qos  = has_radius_qos;
    cmd.value.global.radius_vlan = has_radius_vlan;

    VTSS_ICLI_ERR_PRINT(dot1x_icli_global_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc dot1x_icli_port_contol(i32 session_id, BOOL has_force_unauthorized, BOOL has_force_authorized, BOOL has_auto, BOOL has_single, BOOL has_multi, BOOL has_macbased, icli_stack_port_range_t *plist, BOOL no)
{
    dot1x_icli_cmd_t  cmd;
    cmd.type = INTERFACE_CONF;
    cmd.sub_type = CONF_PORT_CONTROL;
    if (has_force_authorized) {
        cmd.value.admin_state = NAS_PORT_CONTROL_FORCE_AUTHORIZED;
    } else if (has_force_unauthorized) {
        cmd.value.admin_state = NAS_PORT_CONTROL_FORCE_UNAUTHORIZED;
    } else if (has_auto) {
        cmd.value.admin_state = NAS_PORT_CONTROL_AUTO;
    } else if (has_single) {
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        cmd.value.admin_state = NAS_PORT_CONTROL_DOT1X_SINGLE;
#endif
    } else if (has_macbased) {
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        cmd.value.admin_state = NAS_PORT_CONTROL_MAC_BASED;
#endif
    } else if (has_multi) {
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        cmd.value.admin_state = NAS_PORT_CONTROL_DOT1X_MULTI;
#endif
    }

    cmd.no = no;

    VTSS_ICLI_ERR_PRINT(dot1x_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see port_icli_functions.h
BOOL dot1x_icli_runtime_single(u32                session_id,
                               icli_runtime_ask_t ask,
                               icli_runtime_t     *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

//  see port_icli_functions.h
BOOL dot1x_icli_runtime_multi(u32                session_id,
                              icli_runtime_ask_t ask,
                              icli_runtime_t     *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

//  see port_icli_functions.h
BOOL dot1x_icli_runtime_macbased(u32                session_id,
                                 icli_runtime_ask_t ask,
                                 icli_runtime_t     *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc dot1x_icfg_conf(const vtss_icfg_query_request_t *req,
                               vtss_icfg_query_result_t *result)
{
    dot1x_glbl_cfg_t  glbl_cfg, glbl_cfg_default;
    vtss_icfg_conf_print_t conf_print;
    vtss_icfg_conf_print_init(&conf_print);

    T_NG(TRACE_GRP_ICLI, "mode:%d", req->cmd_mode);
    switch (req->cmd_mode) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        // Get current configuration for this switch
        VTSS_RC(dot1x_mgmt_glbl_cfg_get(&glbl_cfg));
        DOT1X_cfg_default_glbl(&glbl_cfg_default); // Get default settings
        // Reauth_Period_Secs
        conf_print.is_default = glbl_cfg.reauth_period_secs == glbl_cfg_default.reauth_period_secs;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x authentication timer re-authenticate", "%d", glbl_cfg.reauth_period_secs));

        //
        // EAPOL timeout
        //
        conf_print.is_default = glbl_cfg.eapol_timeout_secs == glbl_cfg_default.eapol_timeout_secs;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x timeout tx-period", "%d", glbl_cfg.eapol_timeout_secs));

        //
        // EAPOL age period
        //
#ifdef NAS_USES_PSEC
        conf_print.is_default = glbl_cfg.psec_aging_period_secs == glbl_cfg_default.psec_aging_period_secs;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x authentication timer inactivity", "%d", glbl_cfg.psec_aging_period_secs));
#endif

        //
        // reauth_enabled
        //
        conf_print.is_default = glbl_cfg.reauth_enabled == glbl_cfg_default.reauth_enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x re-authentication", ""));

        //
        // mode enabled
        //
        conf_print.is_default = glbl_cfg.enabled == glbl_cfg_default.enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x system-auth-control",   ""));

        //
        // Hold time
        //
        conf_print.is_default = glbl_cfg.psec_hold_time_secs == glbl_cfg_default.psec_hold_time_secs;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x timeout quiet-period", "%d", glbl_cfg.psec_hold_time_secs));

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        //
        // Guest Vlan
        //
        conf_print.is_default = glbl_cfg.guest_vid == glbl_cfg_default.guest_vid;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x guest-vlan",   "%d", glbl_cfg.guest_vid));

        //
        // reauth max
        //
        conf_print.is_default = glbl_cfg.reauth_max == glbl_cfg_default.reauth_max;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x max-reauth-req",  "%d", glbl_cfg.reauth_max));

        //
        // guest vlan supplicant (allow_if_eapol_seen)
        //
        conf_print.is_default = glbl_cfg.guest_vlan_allow_eapols == glbl_cfg_default.guest_vlan_allow_eapols;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x guest-vlan supplicant",   ""));
#endif

        //
        // Global enable
        //
#if defined(VTSS_SW_OPTION_NAS_GUEST_VLAN) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS)
        i8 param_default_str[100];
        i8 param_non_default_str[100];
        strcpy(param_default_str, "");
        strcpy(param_non_default_str, "");

        BOOL default_val_found     = FALSE;
        BOOL non_default_val_found = FALSE;



        BOOL guest_vlan_is_default = TRUE;
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        guest_vlan_is_default = glbl_cfg.guest_vlan_enabled == glbl_cfg_default.guest_vlan_enabled;
        if (guest_vlan_is_default) {
            default_val_found = TRUE;
            strcat(param_default_str, "guest-vlan ");
        } else {
            non_default_val_found = TRUE;
            strcat(param_non_default_str, "guest-vlan ");
        }
#endif

        BOOL radius_vlan_is_default = TRUE;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        radius_vlan_is_default = glbl_cfg.vlan_backend_assignment_enabled == glbl_cfg_default.vlan_backend_assignment_enabled;
        if (radius_vlan_is_default) {
            default_val_found = TRUE;
            strcat(param_default_str, "radius-vlan ");
        } else {
            non_default_val_found = TRUE;
            strcat(param_non_default_str, "radius-vlan ");
        }
#endif

        BOOL radius_qos_is_default = TRUE;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        radius_qos_is_default = glbl_cfg.qos_backend_assignment_enabled == glbl_cfg_default.qos_backend_assignment_enabled;
        if (radius_qos_is_default) {
            default_val_found = TRUE;
            strcat(param_default_str, " radius-qos ");
        } else {
            non_default_val_found = TRUE;
            strcat(param_non_default_str, " radius-qos ");
        }
#endif

        conf_print.is_default = TRUE;
        conf_print.print_no_arguments = TRUE;

        if (default_val_found) {
            VTSS_RC(vtss_icfg_conf_print(req, result,
                                         conf_print, "dot1x feature", "%s", param_default_str));
        }

        conf_print.is_default = FALSE;
        if (non_default_val_found) {
            VTSS_RC(vtss_icfg_conf_print(req, result,
                                         conf_print, "dot1x feature", "%s", param_non_default_str));
        }
#endif
        break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST: {
        vtss_isid_t isid = req->instance_id.port.isid;
        vtss_port_no_t iport = req->instance_id.port.begin_iport;
        dot1x_switch_cfg_t    switch_cfg, switch_cfg_default;

        // Get current configuration
        VTSS_RC(dot1x_mgmt_switch_cfg_get(isid, &switch_cfg));

        // Get default configuration
        DOT1X_cfg_default_switch(&switch_cfg_default);

        //
        // Admin state
        //
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].admin_state;
        /*lint -e(436) */ // Ignore this - Lint Warning 436: Apparent preprocessor directive in invocation of macro 'VTSS_RC'
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x port-control", "%s",
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_FORCE_AUTHORIZED ? "force-authorized" :
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_FORCE_UNAUTHORIZED ? "force-unauthorized" :
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_AUTO ? "auto" :
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_MAC_BASED ? "mac-based" :
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_DOT1X_MULTI ? "multi" :
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
                                     switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE ? "single" :
#endif
                                     ""));

        //
        // Guest vlan
        //
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].guest_vlan_enabled == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].guest_vlan_enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x guest-vlan", ""));
#endif

        //
        // Radius QOS
        //
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].qos_backend_assignment_enabled == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].qos_backend_assignment_enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x radius-qos", ""));
#endif

        //
        // Radius VLAN
        //
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].vlan_backend_assignment_enabled == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].vlan_backend_assignment_enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "dot1x radius-vlan", ""));
#endif
        break;
    }
    default:
        //Not needed
        break;
    }

    return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc dot1x_icfg_init(void)
{
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_DOT1X_GLOBAL_CONF, "dot1x", dot1x_icfg_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_DOT1X_INTERFACE_CONF, "dot1x", dot1x_icfg_conf));
    return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
