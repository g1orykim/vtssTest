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

#include "cli_api.h"   /* For cli_xxx()                                           */
#include "cli.h"       /* For cli_req_t (sadly enough)                            */
#include "cli_grp_help.h" /* For Security group definition access                 */
#include "dot1x_api.h" /* For dot1x_mgmt_xxx(), dot1x_port_control_to_str(), etc. */
#include "dot1x_cli.h" /* Check that public function decls. and defs. are in sync */

#define DOT1X_CLI_PATH "NAS "

/******************************************************************************/
// The order that the commands shall appear.
/******************************************************************************/
typedef enum {
    DOT1X_CLI_PRIO_CONF,
    DOT1X_CLI_PRIO_MODE,
    DOT1X_CLI_PRIO_STATE,
    DOT1X_CLI_PRIO_REAUTH,
    DOT1X_CLI_PRIO_REAUTH_PERIOD,
    DOT1X_CLI_PRIO_EAPOL_TIMEOUT,
#ifdef NAS_USES_PSEC
    DOT1X_CLI_PRIO_AGE_TIME,
    DOT1X_CLI_PRIO_HOLD_TIME,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    DOT1X_CLI_PRIO_BACKEND_QOS,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    DOT1X_CLI_PRIO_BACKEND_VLAN,
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    DOT1X_CLI_PRIO_GUEST_VLAN,
#endif
    DOT1X_CLI_PRIO_AUTHENTICATE,
    DOT1X_CLI_PRIO_STATISTICS,
} dot1x_cli_prio_t;

/******************************************************************************/
// This defines the things that this module can parse.
// The fields are filled in by the dedicated parsers.
/******************************************************************************/
typedef struct {
    nas_port_control_t port_control;
    BOOL               now;
    BOOL               eapol;
    BOOL               radius;
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
    BOOL               global;
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    u32                reauth_max;
    BOOL               reauth_max_set;
    BOOL               allow_if_eapol_seen;
    BOOL               allow_if_eapol_seen_set;
#endif
} dot1x_cli_req_t;

/******************************************************************************/
//
// Static variables
//
/******************************************************************************/

// Forward declaration of static functions
static void    DOT1X_cli_cmd_conf         (cli_req_t *req);
static void    DOT1X_cli_cmd_mode         (cli_req_t *req);
static void    DOT1X_cli_cmd_state        (cli_req_t *req);
static void    DOT1X_cli_cmd_reauth       (cli_req_t *req);
static void    DOT1X_cli_cmd_reauth_period(cli_req_t *req);
static void    DOT1X_cli_cmd_eapol_timeout(cli_req_t *req);
#ifdef NAS_USES_PSEC
static void    DOT1X_cli_cmd_age_time     (cli_req_t *req);
static void    DOT1X_cli_cmd_hold_time    (cli_req_t *req);
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
static void    DOT1X_cli_cmd_backend_qos  (cli_req_t *req);
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
static void    DOT1X_cli_cmd_backend_vlan (cli_req_t *req);
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
static void    DOT1X_cli_cmd_guest_vlan   (cli_req_t *req);
#endif
static void    DOT1X_cli_cmd_authenticate (cli_req_t *req);
static void    DOT1X_cli_cmd_statistics   (cli_req_t *req);
static int32_t DOT1X_cli_parse_generic            (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t DOT1X_cli_parse_reauth_period      (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t DOT1X_cli_parse_eapol_timeout      (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
#ifdef NAS_USES_PSEC
static int32_t DOT1X_cli_parse_age_time           (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t DOT1X_cli_parse_hold_time          (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
static int32_t DOT1X_cli_parse_reauth_max         (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t DOT1X_cli_parse_allow_if_eapol_seen(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
#endif
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
static int32_t DOT1X_cli_parse_global_or_port_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
#endif

/******************************************************************************/
// Parameters used in this module
/******************************************************************************/
static cli_parm_t DOT1X_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Globally enable 802.1X\n"
        "disable: Globally disable 802.1X\n"
        "(default: Show current 802.1X global state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DOT1X_cli_cmd_mode,
    },
    {
        "auto|authorized|unauthorized"
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        "|single"
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        "|multi"
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        "|macbased"
#endif
        ,
        "auto        : Port-based 802.1X Authentication\n"
        "authorized  : Port access is allowed\n"
        "unauthorized: Port access is not allowed\n"
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        "single      : Single Host 802.1X Authentication\n"
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        "multi       : Multiple Host 802.1X Authentication\n"
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        "macbased    : Switch authenticates on behalf of the client\n"
#endif
        "(default: Show 802.1X state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_generic,
        DOT1X_cli_cmd_state,
    },
    {
        "enable|disable",
        "enable : Enable reauthentication\n"
        "disable: Disable reauthentication\n"
        "(default: Show current reauthentication mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DOT1X_cli_cmd_reauth,
    },
    {
        "<reauth_period>",
        "Period between reauthentication attempts (" vtss_xstr(DOT1X_REAUTH_PERIOD_SECS_MIN) "-" vtss_xstr(DOT1X_REAUTH_PERIOD_SECS_MAX) " seconds)\n"
        "(default: Show current reauthentication period)",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_reauth_period,
        DOT1X_cli_cmd_reauth_period,
    },
    {
        "<eapol_timeout>",
        "Time between EAPOL retransmissions (" vtss_xstr(DOT1X_EAPOL_TIMEOUT_SECS_MIN) "-" vtss_xstr(DOT1X_EAPOL_TIMEOUT_SECS_MAX) " seconds)\n"
        "(default: Show current EAPOL retransmission timeout)",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_eapol_timeout,
        DOT1X_cli_cmd_eapol_timeout,
    },
#ifdef NAS_USES_PSEC
    {
        "<age_time>",
        "Time between checks (" vtss_xstr(NAS_PSEC_AGING_PERIOD_SECS_MIN) "-" vtss_xstr(NAS_PSEC_AGING_PERIOD_SECS_MAX) " seconds)\n"
        "(default: Show current age time)",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_age_time,
        DOT1X_cli_cmd_age_time,
    },
    {
        "<hold_time>",
        "Time on hold (" vtss_xstr(NAS_PSEC_HOLD_TIME_SECS_MIN) "-" vtss_xstr(NAS_PSEC_HOLD_TIME_SECS_MAX) " seconds)\n"
        "(default: Show current hold time)",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_hold_time,
        DOT1X_cli_cmd_hold_time,
    },
#endif
    {
        "now",
        "Force reauthentication immediately\n"
        "(default: Schedule a reauthentication)",
        CLI_PARM_FLAG_NONE,
        DOT1X_cli_parse_generic,
        DOT1X_cli_cmd_authenticate,
    },
    {
        "clear|eapol|radius",
        "clear   : Clear statistics\n"
        "eapol   : Show EAPOL statistics\n"
        "radius  : Show Backend Server statistics\n"
        "(default: Show all statistics)",
        CLI_PARM_FLAG_NO_TXT,
        DOT1X_cli_parse_generic,
        NULL,
    },
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    {
        "global|<port_list>",
        "global     : Select the global RADIUS-assigned QoS setting\n"
        "<port_list>: Select the per-port RADIUS-assigned QoS setting\n"
        "(default: Show current per-port RADIUS-assigned QoS state)",
        CLI_PARM_FLAG_NO_TXT,
        DOT1X_cli_parse_global_or_port_list,
        DOT1X_cli_cmd_backend_qos,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    {
        "enable|disable",
        "enable : Enable RADIUS-assigned QoS either globally or on one or more ports\n"
        "disable: Disable RADIUS-assigned QoS either globally or on one or more ports\n"
        "(default: Show current RADIUS-assigned QoS state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DOT1X_cli_cmd_backend_qos,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    {
        "global|<port_list>",
        "global     : Select the global RADIUS-assigned VLAN setting\n"
        "<port_list>: Select the per-port RADIUS-assigned VLAN setting\n"
        "(default: Show current per-port RADIUS-assigned VLAN state)",
        CLI_PARM_FLAG_NO_TXT,
        DOT1X_cli_parse_global_or_port_list,
        DOT1X_cli_cmd_backend_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    {
        "enable|disable",
        "enable : Enable RADIUS-assigned VLAN either globally or on one or more ports\n"
        "disable: Disable RADIUS-assigned VLAN either globally or on one or more ports\n"
        "(default: Show current RADIUS-assigned VLAN state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DOT1X_cli_cmd_backend_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        "global|<port_list>",
        "global     : Select the global Guest VLAN setting\n"
        "<port_list>: Select the per-port Guest VLAN setting\n"
        "(default: Show current per-port Guest VLAN state)",
        CLI_PARM_FLAG_NO_TXT,
        DOT1X_cli_parse_global_or_port_list,
        DOT1X_cli_cmd_guest_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        "enable|disable",
        "enable : Enable Guest VLAN either globally or on one or more ports\n"
        "disable: Disable Guest VLAN either globally or on one or more ports\n"
        "(default: Show current Guest VLAN state)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DOT1X_cli_cmd_guest_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        "<vid>",
        "Guest VLAN ID used when entering the Guest VLAN. Use the 'global' keyword to change it\n"
        "(default: Show current Guest VLAN ID)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vid,
        DOT1X_cli_cmd_guest_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        "<reauth_max>",
        "The value can only be set if you use the 'global' keyword in the beginning of the command.\n"
        "The number of times a Request Identity EAPOL frame is sent without response before considering entering the Guest VLAN\n"
        "(default: Show current Maximum Reauth Count value",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_reauth_max,
        DOT1X_cli_cmd_guest_vlan,
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        "<allow_if_eapol_seen>",
        "The value can only be set if you use the 'global'\n"
        "keyword in the beginning of the command.\n"
        "disable:The Guest VLAN can only be entered if no EAPOL frames have\n"
        "        been received on a port for the lifetime of the port\n"
        "enable :The Guest VLAN can be entered even if an EAPOL frame has\n"
        "         been received during the lifetime of the port\n"
        "(default: Show current setting)\n",
        CLI_PARM_FLAG_SET,
        DOT1X_cli_parse_allow_if_eapol_seen,
        DOT1X_cli_cmd_guest_vlan,
    },
#endif
    {NULL, NULL, 0, 0, NULL}
};

/******************************************************************************/
// Commands defined in this module
/******************************************************************************/
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Configuration [<port_list>]",
    NULL,
    "Show 802.1X configuration",
    DOT1X_CLI_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_conf,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Mode",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Mode [enable|disable]",
    "Set or show the global NAS state",
    DOT1X_CLI_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_mode,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

// Use the CLI_CMD_TAB_ENTRY_DECL() macro, so that
// we can use #ifdef/#endif directives when defining
// the command.
// If we only had the cli_cmd_tab_entry() macro,
// we couldn't do that, and with three options
// we would have to do 2^3 = 8 different flavors
// of the command :-(
CLI_CMD_TAB_ENTRY_DECL(DOT1X_cli_cmd_state) = {
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "State [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "State [<port_list>] ["
    "auto|authorized|unauthorized"
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    "|single"
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    "|multi"
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    "|macbased"
#endif
    "]",
    "Set or show the port security state",
    (ulong)DOT1X_CLI_PRIO_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_state,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Reauthentication",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Reauthentication [enable|disable]",
    "Set or show Reauthentication state",
    DOT1X_CLI_PRIO_REAUTH,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_reauth,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "ReauthPeriod",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "ReauthPeriod [<reauth_period>]",
    "Set or show the period between reauthentication attempts",
    DOT1X_CLI_PRIO_REAUTH_PERIOD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_reauth_period,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "EapolTimeout",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "EapolTimeout [<eapol_timeout>]",
    "Set or show the time between EAPOL retransmissions",
    DOT1X_CLI_PRIO_EAPOL_TIMEOUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_eapol_timeout,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef NAS_USES_PSEC
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Agetime",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Agetime [<age_time>]",
    "Time in seconds between check for activity on successfully authenticated MAC addresses",
    DOT1X_CLI_PRIO_AGE_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_age_time,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#ifdef NAS_USES_PSEC
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Holdtime",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Holdtime [<hold_time>]",
    "Time in seconds before a MAC-address that failed authentication gets a new authentication chance",
    DOT1X_CLI_PRIO_HOLD_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_hold_time,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* NAS_USES_PSEC */

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "RADIUS_QoS [global|<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "RADIUS_QoS [global|<port_list>] [enable|disable]",
    "Set or show either global state (use the global keyword) or per-port state of RADIUS-assigned QoS",
    DOT1X_CLI_PRIO_BACKEND_QOS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_backend_qos,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "RADIUS_VLAN [global|<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "RADIUS_VLAN [global|<port_list>] [enable|disable]",
    "Set or show either global state (use the global keyword) or per-port state of RADIUS-assigned VLAN",
    DOT1X_CLI_PRIO_BACKEND_VLAN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_backend_vlan,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Guest_VLAN [global|<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Guest_VLAN [global|<port_list>] [enable|disable] [<vid>] [<reauth_max>] [<allow_if_eapol_seen>]",
    "Set or show either global state and parameters (use the global keyword) or per-port state of Guest VLAN.\n"
    "The <reauth_max> and <allow_if_eapol_seen> parameters will only be used if 'global' is specified",
    DOT1X_CLI_PRIO_GUEST_VLAN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_guest_vlan,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Authenticate [<port_list>] [now]",
    "Refresh (restart) 802.1X authentication process",
    DOT1X_CLI_PRIO_AUTHENTICATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_authenticate,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Statistics [<port_list>] [eapol|radius]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH DOT1X_CLI_PATH "Statistics [<port_list>] [clear|eapol|radius]",
    "Show or clear 802.1X statistics",
    DOT1X_CLI_PRIO_STATISTICS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    DOT1X_cli_cmd_statistics,
    NULL,
    DOT1X_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/******************************************************************************/
//
// Module Private Functions
//
/******************************************************************************/
static inline void DOT1X_cli_cmd_print_stat(cli_req_t *req, switch_iter_t *sit, port_iter_t *pit, dot1x_switch_status_t *switch_status,
                                            dot1x_port_cfg_t *port_cfg, dot1x_statistics_t *stat,
                                            BOOL cmd_state, BOOL cmd_backend_qos_port, BOOL cmd_backend_vlan_port, BOOL cmd_guest_vlan_port, BOOL cmd_statistics)
{
    nas_port_status_t port_status;
    char              *buf_ptr;
    dot1x_cli_req_t   *dot1x_req = req->module_req;
#if defined(NAS_MULTI_CLIENT) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN)
    char              buf[50];
#endif
    BOOL              first = pit->first;

    if (first) {
        cli_cmd_usid_print(sit->usid, req, 1);
    }

    if (cmd_state) {
        if (first) {
            cli_table_header("Port  Admin State         Port State             Last Source        Last ID             ");
        }

        port_status = switch_status->status[pit->iport - VTSS_PORT_NO_START];

        buf_ptr = port_status == NAS_PORT_STATUS_LINK_DOWN    ? "Link Down"         :
                  port_status == NAS_PORT_STATUS_AUTHORIZED   ? "Authorized"        :
                  port_status == NAS_PORT_STATUS_UNAUTHORIZED ? "Unauthorized"      :
                  port_status == NAS_PORT_STATUS_DISABLED     ? "Globally Disabled" : "?";

#ifdef NAS_MULTI_CLIENT
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

        cli_printf("%-6d%-20s%-23s%-19s%s\n",
                   pit->uport,
                   sit->isid == VTSS_ISID_LOCAL ? "-" :
                   dot1x_port_control_to_str(port_cfg->admin_state, FALSE),
                   buf_ptr,
                   strlen(stat->client_info.mac_addr_str) ? (char *)stat->client_info.mac_addr_str : "-",
                   strlen(stat->client_info.identity)     ? (char *)stat->client_info.identity     : "-");
    } else if (cmd_backend_qos_port) {
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        if (first) {
            cli_printf(      "      RADIUS\n");
            cli_table_header("Port  QoS      Current");
        }

        dot1x_qos_class_to_str(switch_status->qos_class[pit->iport - VTSS_PORT_NO_START], buf);
        cli_printf("%-6d%-9s%6s\n", pit->uport, cli_bool_txt(port_cfg->qos_backend_assignment_enabled), buf);
#endif
    } else if (cmd_backend_vlan_port) {
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        vtss_vid_t vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
        if (first) {
            cli_printf(      "      RADIUS\n");
            cli_table_header("Port  VLAN     Current");
        }

        if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
            vid = 0;
        }
        dot1x_vlan_to_str(vid, buf);
        cli_printf("%-6d%-9s%6s\n", pit->uport, cli_bool_txt(port_cfg->vlan_backend_assignment_enabled), buf);
#endif
    } else if (cmd_guest_vlan_port) {
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        vtss_vid_t vid = switch_status->vid[pit->iport - VTSS_PORT_NO_START];
        if (first) {
            cli_printf(      "      Guest\n");
            cli_table_header("Port  VLAN     Current");
        }

        if (switch_status->vlan_type[pit->iport - VTSS_PORT_NO_START] != NAS_VLAN_TYPE_GUEST_VLAN) {
            vid = 0;
        }

        dot1x_vlan_to_str(vid, buf);
        cli_printf("%-6d%-9s%6s\n", pit->uport, cli_bool_txt(port_cfg->guest_vlan_enabled), buf);
#endif
    }

    if (cmd_statistics) {
#ifdef NAS_MULTI_CLIENT
        BOOL client_found = TRUE, client_first = TRUE;
        while (client_found) {
            if (switch_status->status[pit->iport - VTSS_PORT_NO_START] >= NAS_PORT_STATUS_CNT) {
                if (dot1x_mgmt_multi_client_statistics_get(sit->isid, pit->iport, stat, &client_found, client_first) != VTSS_RC_OK) {
                    continue;
                }
                if (!client_found) {
                    if (client_first) {
                        memset(stat, 0, sizeof(*stat)); // Gotta display something anyway, since it's the first time
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
            if (dot1x_req->eapol) {
                if (first) {
                    cli_printf(      "      Rx      Tx      Rx      Tx      Rx      Tx      Rx      Rx      Rx\n");
                    cli_table_header("Port  Total   Total   RespId  ReqId   Resp    Req     Start   Logoff  Error");
                }
                cli_printf("%-6d%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u\n",
                           pit->uport,
                           stat->eapol_counters.dot1xAuthEapolFramesRx,
                           stat->eapol_counters.dot1xAuthEapolFramesTx,
                           stat->eapol_counters.dot1xAuthEapolRespIdFramesRx,
                           stat->eapol_counters.dot1xAuthEapolReqIdFramesTx,
                           stat->eapol_counters.dot1xAuthEapolRespFramesRx,
                           stat->eapol_counters.dot1xAuthEapolReqFramesTx,
                           stat->eapol_counters.dot1xAuthEapolStartFramesRx,
                           stat->eapol_counters.dot1xAuthEapolLogoffFramesRx,
                           stat->eapol_counters.dot1xAuthInvalidEapolFramesRx +
                           stat->eapol_counters.dot1xAuthEapLengthErrorFramesRx);
            } else if (dot1x_req->radius) {
#ifdef NAS_MULTI_CLIENT
                if (first) {
                    cli_printf(      "      Rx Access   Rx Other    Rx Auth.    Rx Auth.    Tx          MAC\n");
                    cli_table_header("Port  Challenges  Requests    Successes   Failures    Responses   Address          ");
                }
#else
                if (first) {
                    cli_printf(      "      Rx Access   Rx Other    Rx Auth.    Rx Auth.    Tx\n");
                    cli_table_header("Port  Challenges  Requests    Successes   Failures    Responses   ");
                }
#endif
                cli_printf("%-6d%-12u%-12u%-12u%-12u%-12u"
#ifdef NAS_MULTI_CLIENT
                           "%s"
#endif
                           "\n",
                           pit->uport,
                           stat->backend_counters.backendAccessChallenges,
                           stat->backend_counters.backendOtherRequestsToSupplicant,
                           stat->backend_counters.backendAuthSuccesses,
                           stat->backend_counters.backendAuthFails,
                           stat->backend_counters.backendResponses
#ifdef NAS_MULTI_CLIENT
                           , strlen(stat->client_info.mac_addr_str) ? (char *)stat->client_info.mac_addr_str : "-"
#endif
                          );
            } else {
#ifdef NAS_MULTI_CLIENT
                if (switch_status->status[pit->iport - VTSS_PORT_NO_START] < NAS_PORT_STATUS_CNT)
#endif
                {
                    cli_printf("%sPort %d EAPOL Statistics:\n\n", first ? "" : "\n", pit->uport);
                    cli_cmd_stati("Rx Total",          "Tx Total",      stat->eapol_counters.dot1xAuthEapolFramesRx,          stat->eapol_counters.dot1xAuthEapolFramesTx);
                    cli_cmd_stati("Rx Response/Id",    "Tx Request/Id", stat->eapol_counters.dot1xAuthEapolRespIdFramesRx,    stat->eapol_counters.dot1xAuthEapolReqIdFramesTx);
                    cli_cmd_stati("Rx Response",       "Tx Request",    stat->eapol_counters.dot1xAuthEapolRespFramesRx,      stat->eapol_counters.dot1xAuthEapolReqFramesTx);
                    cli_cmd_stati("Rx Start",          NULL,            stat->eapol_counters.dot1xAuthEapolStartFramesRx,     0);
                    cli_cmd_stati("Rx Logoff",         NULL,            stat->eapol_counters.dot1xAuthEapolLogoffFramesRx,    0);
                    cli_cmd_stati("Rx Invalid Type",   NULL,            stat->eapol_counters.dot1xAuthInvalidEapolFramesRx,   0);
                    cli_cmd_stati("Rx Invalid Length", NULL,            stat->eapol_counters.dot1xAuthEapLengthErrorFramesRx, 0);
                }
#ifdef NAS_MULTI_CLIENT
                if (switch_status->status[pit->iport - VTSS_PORT_NO_START] >= NAS_PORT_STATUS_CNT && strlen(stat->client_info.mac_addr_str)) {
                    cli_printf("\nPort %d Backend Server Statistics (MAC Address %s):\n\n", pit->uport, stat->client_info.mac_addr_str);
                } else
#endif
                {
                    cli_printf("\nPort %d Backend Server Statistics:\n\n", pit->uport);
                }

                cli_cmd_stati("Rx Access Challenges", "Tx Responses", stat->backend_counters.backendAccessChallenges,          stat->backend_counters.backendResponses);
                cli_cmd_stati("Rx Other Requests",    NULL,           stat->backend_counters.backendOtherRequestsToSupplicant, 0);
                cli_cmd_stati("Rx Auth. Successes",   NULL,           stat->backend_counters.backendAuthSuccesses,             0);
                cli_cmd_stati("Rx Auth. Failures",    NULL,           stat->backend_counters.backendAuthFails,                 0);
            } /* !eapol && !radius only */
#ifdef NAS_MULTI_CLIENT
            first = FALSE; // Needed in case we're doing MAC-based Authentication with several clients on one port.
#endif
        } /* while (client_found) */
    } /* if (cmd_statistics) */
}

/******************************************************************************/
// DOT1X_cli_cmd()
// The master of all CLI functions. Takes care of all status and configuration.
/******************************************************************************/
static void DOT1X_cli_cmd(cli_req_t *req,
                          BOOL cmd_mode,              BOOL cmd_state,           BOOL cmd_reauth,           BOOL cmd_reauth_period,    BOOL cmd_eapol_timeout,
                          BOOL cmd_age_time,          BOOL cmd_hold_time,       BOOL cmd_backend_qos_glbl, BOOL cmd_backend_qos_port, BOOL cmd_backend_vlan_glbl,
                          BOOL cmd_backend_vlan_port, BOOL cmd_guest_vlan_glbl, BOOL cmd_guest_vlan_port,  BOOL cmd_auth,             BOOL cmd_statistics)
{
    dot1x_glbl_cfg_t      glbl_cfg;
    dot1x_switch_cfg_t    switch_cfg;
    dot1x_port_cfg_t      *port_cfg;
    dot1x_switch_status_t switch_status;
    dot1x_statistics_t    stat;
    vtss_rc               rc;
    dot1x_cli_req_t       *dot1x_req = req->module_req;
    switch_iter_t         sit;

    if (cli_cmd_switch_none(req) || ((cmd_mode || !cmd_state) && cli_cmd_slave(req)) || cli_cmd_conf_slave((req))) {
        return;
    }

    if (cmd_mode || cmd_reauth || cmd_reauth_period || cmd_eapol_timeout || cmd_age_time || cmd_hold_time || cmd_backend_qos_glbl || cmd_backend_vlan_glbl || cmd_guest_vlan_glbl) {
        if ((rc = dot1x_mgmt_glbl_cfg_get(&glbl_cfg)) != VTSS_RC_OK) {
            cli_printf("%s\n", error_txt(rc));
            return;
        }

        if (req->set) {
            if (cmd_mode) {
                glbl_cfg.enabled = req->enable;
            }
            if (cmd_reauth) {
                glbl_cfg.reauth_enabled = req->enable;
            }
            if (cmd_reauth_period) {
                glbl_cfg.reauth_period_secs = req->value;
            }
            if (cmd_eapol_timeout) {
                glbl_cfg.eapol_timeout_secs = req->value;
            }
#ifdef NAS_USES_PSEC
            if (cmd_age_time) {
                glbl_cfg.psec_aging_period_secs = req->value;
            }
            if (cmd_hold_time) {
                glbl_cfg.psec_hold_time_secs = req->value;
            }
#endif /* NAS_USES_PSEC */
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
            if (cmd_backend_qos_glbl) {
                glbl_cfg.qos_backend_assignment_enabled = req->enable;
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
            if (cmd_backend_vlan_glbl) {
                glbl_cfg.vlan_backend_assignment_enabled = req->enable;
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
            if (cmd_guest_vlan_glbl) {
                glbl_cfg.guest_vlan_enabled = req->enable;

                if (req->vid_spec == CLI_SPEC_VAL) {
                    glbl_cfg.guest_vid = req->vid;
                }
                if (dot1x_req->reauth_max_set) {
                    glbl_cfg.reauth_max = dot1x_req->reauth_max;
                }
                if (dot1x_req->allow_if_eapol_seen_set) {
                    glbl_cfg.guest_vlan_allow_eapols = dot1x_req->allow_if_eapol_seen;
                }
            }
#endif
            if ((rc = dot1x_mgmt_glbl_cfg_set(&glbl_cfg)) != VTSS_RC_OK) {
                cli_printf("%s\n", error_txt(rc));
            }
        } else {
            // Display current settings
            if (cmd_mode) {
                cli_printf("Mode             : %s\n", cli_bool_txt(glbl_cfg.enabled));
            }
            if (cmd_reauth) {
                cli_printf("Reauth.          : %s\n", cli_bool_txt(glbl_cfg.reauth_enabled));
            }
            if (cmd_reauth_period) {
                cli_printf("Reauth. Period   : %d\n", glbl_cfg.reauth_period_secs);
            }
            if (cmd_eapol_timeout) {
                cli_printf("EAPOL Timeout    : %d\n", glbl_cfg.eapol_timeout_secs);
            }
#ifdef NAS_USES_PSEC
            if (cmd_age_time) {
                cli_printf("Age Period       : %u\n", glbl_cfg.psec_aging_period_secs);
            }
            if (cmd_hold_time) {
                cli_printf("Hold Time        : %u\n", glbl_cfg.psec_hold_time_secs);
            }
#endif /* NAS_USES_PSEC */

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
            if (cmd_backend_qos_glbl) {
                cli_printf("RADIUS QoS       : %s\n", cli_bool_txt(glbl_cfg.qos_backend_assignment_enabled));
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
            if (cmd_backend_vlan_glbl) {
                cli_printf("RADIUS VLAN      : %s\n", cli_bool_txt(glbl_cfg.vlan_backend_assignment_enabled));
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
            if (cmd_guest_vlan_glbl) {
                cli_printf("Guest VLAN       : %s\n", cli_bool_txt(glbl_cfg.guest_vlan_enabled));
                cli_printf("Guest VLAN ID    : %d\n", glbl_cfg.guest_vid);
                cli_printf("Max. Reauth Count: %u\n", glbl_cfg.reauth_max);
                cli_printf("Allow Guest VLAN if EAPOL Frame Seen: %s\n", cli_bool_txt(glbl_cfg.guest_vlan_allow_eapols));
            }
#endif
        }
    }

    if (!(cmd_state || cmd_backend_qos_port || cmd_backend_vlan_port || cmd_guest_vlan_port || cmd_auth || cmd_statistics)) {
        // Nothing more to do.
        return;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;

        if ((cmd_state || cmd_backend_qos_port || cmd_backend_vlan_port || cmd_guest_vlan_port || cmd_statistics) && (dot1x_mgmt_switch_status_get(sit.isid, &switch_status) != VTSS_RC_OK ||
                (sit.isid != VTSS_ISID_LOCAL && dot1x_mgmt_switch_cfg_get(sit.isid, &switch_cfg) != VTSS_RC_OK))) {
            continue;
        }

        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            if (cmd_auth) {
                (void)dot1x_mgmt_reauth(sit.isid, pit.iport, dot1x_req->now);
                continue;
            }

            if (req->clear) {
                (void)dot1x_mgmt_statistics_clear(sit.isid, pit.iport, NULL);
                continue;
            }

            port_cfg = &switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START];
            if (req->set && cmd_state) {
                port_cfg->admin_state = dot1x_req->port_control;
                continue;
            }
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
            if (req->set && cmd_backend_qos_port) {
                port_cfg->qos_backend_assignment_enabled = req->enable;
                continue;
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
            if (req->set && cmd_backend_vlan_port) {
                port_cfg->vlan_backend_assignment_enabled = req->enable;
                continue;
            }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
            if (req->set && cmd_guest_vlan_port) {
                port_cfg->guest_vlan_enabled = req->enable;
                continue;
            }
#endif
            if (sit.isid == VTSS_ISID_LOCAL) {
                memset(&stat, 0, sizeof(stat));
            } else if (dot1x_mgmt_statistics_get(sit.isid, pit.iport, NULL, &stat) != VTSS_RC_OK) {
                // For MAC-based ports, the statistics counters are not valid. See below when statistics is printed
                continue;
            }

            DOT1X_cli_cmd_print_stat(req, &sit, &pit, &switch_status, port_cfg, &stat, cmd_state, cmd_backend_qos_port, cmd_backend_vlan_port, cmd_guest_vlan_port, cmd_statistics);
        } /* while (cli_port_iter_getnext()) */

        if (req->set && (cmd_state || cmd_backend_qos_port || cmd_backend_vlan_port || cmd_guest_vlan_port)) {
            if ((rc = dot1x_mgmt_switch_cfg_set(sit.isid, &switch_cfg)) != VTSS_RC_OK) {
                cli_printf("Error: %s\n", error_txt(rc));
            }
        }
    } /* while (cli_switch_iter_getnext() */
}

/******************************************************************************/
// DOT1X_cli_cmd_conf()
/******************************************************************************/
static void DOT1X_cli_cmd_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("802.1X Configuration", 1);
    }

    DOT1X_cli_cmd(req, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_mode()
/******************************************************************************/
static void DOT1X_cli_cmd_mode(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_state()
/******************************************************************************/
static void DOT1X_cli_cmd_state(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_reauth()
/******************************************************************************/
static void DOT1X_cli_cmd_reauth(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_reauth_period()
/******************************************************************************/
static void DOT1X_cli_cmd_reauth_period(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_eapol_timeout()
/******************************************************************************/
static void DOT1X_cli_cmd_eapol_timeout(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_cli_cmd_age_time()
/******************************************************************************/
static void DOT1X_cli_cmd_age_time(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
#endif

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_cli_cmd_hold_time()
/******************************************************************************/
static void DOT1X_cli_cmd_hold_time(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
/******************************************************************************/
// DOT1X_cli_cmd_backend_qos()
/******************************************************************************/
static void DOT1X_cli_cmd_backend_qos(cli_req_t *req)
{
    dot1x_cli_req_t *dot1x_req = req->module_req;

    if (dot1x_req->global) {
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    } else {
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
    }
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
/******************************************************************************/
// DOT1X_cli_cmd_backend_vlan()
/******************************************************************************/
static void DOT1X_cli_cmd_backend_vlan(cli_req_t *req)
{
    dot1x_cli_req_t *dot1x_req = req->module_req;

    if (dot1x_req->global) {
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    } else {
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
    }
}
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_cli_cmd_guest_vlan()
/******************************************************************************/
static void DOT1X_cli_cmd_guest_vlan(cli_req_t *req)
{
    dot1x_cli_req_t *dot1x_req = req->module_req;

    if (dot1x_req->global) {
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
    } else {
        if (req->set && dot1x_req->reauth_max_set) {
            cli_printf("Error: You cannot specify <reauth_max> unless you use the 'global' keyword\n");
            return;
        }
        if (req->set && dot1x_req->allow_if_eapol_seen_set) {
            cli_printf("Error: You cannot specify <allow_if_eapol_seen> unless you use the 'global' keyword\n");
            return;
        }
        DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0);
    }
}
#endif

/******************************************************************************/
// DOT1X_cli_cmd_authenticate()
/******************************************************************************/
static void DOT1X_cli_cmd_authenticate(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
}

/******************************************************************************/
// DOT1X_cli_cmd_statistics()
/******************************************************************************/
static void DOT1X_cli_cmd_statistics(cli_req_t *req)
{
    DOT1X_cli_cmd(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
}

/******************************************************************************/
// DOT1X_cli_parse_generic()
/******************************************************************************/
static int32_t DOT1X_cli_parse_generic(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char            *found     = cli_parse_find(cmd, stx);
    dot1x_cli_req_t *dot1x_req = req->module_req;

    req->parm_parsed = 1;

    if (!found) {
        return 1;
    } else if (!strncmp(found, "authorized", 10)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_FORCE_AUTHORIZED;
    } else if (!strncmp(found, "auto", 4)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_AUTO;
    } else if (!strncmp(found, "unauthorized", 12)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_FORCE_UNAUTHORIZED;
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    } else if (!strncmp(found, "single", 6)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_DOT1X_SINGLE;
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    } else if (!strncmp(found, "multi", 5)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_DOT1X_MULTI;
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    } else if (!strncmp(found, "macbased", 8)) {
        dot1x_req->port_control = NAS_PORT_CONTROL_MAC_BASED;
#endif
    } else if (!strncmp(found, "now", 3)) {
        dot1x_req->now = 1;
    } else if (!strncmp(found, "eapol", 5)) {
        dot1x_req->eapol = 1;
    } else if (!strncmp(found, "radius", 6)) {
        dot1x_req->radius = 1;
    } else if (!strncmp(found, "clear", 5)) {
        req->clear = TRUE;
    } else {
        return 1;
    }

    return 0;
}

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
/******************************************************************************/
// DOT1X_cli_parse_global_or_port_list()
/******************************************************************************/
static int32_t DOT1X_cli_parse_global_or_port_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char            *found     = cli_parse_find(cmd, stx);
    dot1x_cli_req_t *dot1x_req = req->module_req;

    req->parm_parsed = 1;

    if (!found) {
        return cli_parm_parse_port_list(cmd, cmd2, stx, cmd_org, req);
    } else if (!strncmp(found, "global", 6)) {
        dot1x_req->global = TRUE;
    } else {
        return 1;
    }
    return 0;
}
#endif

/******************************************************************************/
// DOT1X_cli_parse_reauth_period()
/******************************************************************************/
static int32_t DOT1X_cli_parse_reauth_period(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, DOT1X_REAUTH_PERIOD_SECS_MIN, DOT1X_REAUTH_PERIOD_SECS_MAX);
}

/******************************************************************************/
// DOT1X_cli_parse_eapol_timeout()
/******************************************************************************/
static int32_t DOT1X_cli_parse_eapol_timeout(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, DOT1X_EAPOL_TIMEOUT_SECS_MIN, DOT1X_EAPOL_TIMEOUT_SECS_MAX);
}

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_cli_parse_age_time()
/******************************************************************************/
static int32_t DOT1X_cli_parse_age_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, NAS_PSEC_AGING_PERIOD_SECS_MIN, NAS_PSEC_AGING_PERIOD_SECS_MAX);
}
#endif /* NAS_USES_PSEC */

/******************************************************************************/
// DOT1X_cli_parse_hold_time()
/******************************************************************************/
#ifdef NAS_USES_PSEC
static int32_t DOT1X_cli_parse_hold_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, NAS_PSEC_HOLD_TIME_SECS_MIN, NAS_PSEC_HOLD_TIME_SECS_MAX);
}
#endif /* NAS_USES_PSEC */

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_cli_parse_reauth_max()
/******************************************************************************/
static int32_t DOT1X_cli_parse_reauth_max(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    dot1x_cli_req_t *dot1x_req = req->module_req;
    dot1x_req->reauth_max_set = TRUE;
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &dot1x_req->reauth_max, 1, 255);
}
#endif /* VTSS_SW_OPTION_NAS_GUEST_VLAN */

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_cli_parse_allow_if_eapol_seen()
/******************************************************************************/
static int32_t DOT1X_cli_parse_allow_if_eapol_seen(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    dot1x_cli_req_t *dot1x_req = req->module_req;
    char *found = cli_parse_find(cmd, "[enable|disable]");

    req->parm_parsed = 1;
    if (found) {
        if (!strncmp(found, "disable", 7)) {
            dot1x_req->allow_if_eapol_seen = FALSE;
            dot1x_req->allow_if_eapol_seen_set = TRUE;
            return 0;
        } else if (!strncmp(found, "enable", 6)) {
            dot1x_req->allow_if_eapol_seen = TRUE;
            dot1x_req->allow_if_eapol_seen_set = TRUE;
            return 0;
        }
    }
    return 1;
}
#endif /* VTSS_SW_OPTION_NAS_GUEST_VLAN */

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// dot1x_cli_init()
/******************************************************************************/
void dot1x_cli_init(void)
{
    // Register the size required for this module's structure
    cli_req_size_register(sizeof(dot1x_cli_req_t));
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
