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
#include "dhcp_relay_cli.h"
#include "dhcp_relay_api.h"
#include "cli_trace_def.h"

typedef struct {
    u32 relay_info_policy;
} dhcp_relay_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void dhcp_relay_cli_init(void)
{
    /* register the size required for DHCP Relay req. structure */
    cli_req_size_register(sizeof(dhcp_relay_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* DHCP relay configuration */
static void DHCP_RELAY_cli_cmd_conf(cli_req_t *req, BOOL relay_mode,
                                    BOOL relay_server_add, BOOL relay_server_del,
                                    BOOL relay_info_mode, BOOL relay_info_policy)
{
    vtss_rc                 rc;
    dhcp_relay_conf_t       conf;
    char                    buf[40];
    int                     idx;
    dhcp_relay_cli_req_t    *dhcp_relay_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        dhcp_relay_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (relay_mode) {
            conf.relay_mode = req->enable;
        }
        if (relay_server_add) {
            conf.relay_server[0] = req->ipv4_addr;
            if (conf.relay_server[0] == 0) {
                conf.relay_server_cnt = 0;
            } else {
                conf.relay_server_cnt = 1;
            }
        }
        if (relay_server_del) {
            for (idx = 0; idx < DHCP_RELAY_MGMT_MAX_DHCP_SERVER; idx++) {
                if (conf.relay_server[idx] == req->ipv4_addr) {
                    conf.relay_server[idx] = 0;
                    conf.relay_server_cnt--;
                    break;
                }
            }
        }
        if (relay_info_mode) {
            conf.relay_info_mode = req->enable;
        }
        if (relay_info_policy) {
            conf.relay_info_policy = dhcp_relay_req->relay_info_policy;
        }
        if (conf.relay_info_mode == DHCP_RELAY_MGMT_DISABLED && conf.relay_info_policy == DHCP_RELAY_INFO_POLICY_REPLACE) {
            CPRINTF("The 'Replace' policy is invalid when relay information mode is disabled\n");
        }
        if ((rc = dhcp_relay_mgmt_conf_set(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    } else {
        if (relay_mode) {
            CPRINTF("DHCP Relay Mode               : %s\n", cli_bool_txt(conf.relay_mode));
        }
        if (relay_server_add) {
            CPRINTF("DHCP Relay Server             : %s", conf.relay_server_cnt ? misc_ipv4_txt(conf.relay_server[0], buf) : "NULL");
#if 0
            for (idx = 1; idx < DHCP_RELAY_MGMT_MAX_DHCP_SERVER; idx++) {
                if (conf.relay_server[idx]) {
                    CPRINTF(", %s", misc_ipv4_txt(conf.relay_server[idx], buf));
                }
            }
#endif
            CPRINTF("\n");
        }
        if (relay_info_mode) {
            CPRINTF("DHCP Relay Information Mode   : %s\n", cli_bool_txt(conf.relay_info_mode));
        }
        if (relay_info_policy) {
            CPRINTF("DHCP Relay Information Policy : %s\n", conf.relay_info_policy == DHCP_RELAY_INFO_POLICY_REPLACE ? "Replace" : conf.relay_info_policy == DHCP_RELAY_INFO_POLICY_KEEP ? "Keep" : "Drop");
        }
    }
}

static void DHCP_RELAY_cli_cmd_conf_show(cli_req_t *req)
{
    if (!req->set) {
        cli_header("DHCP Relay Configuration", 1);
    }
    DHCP_RELAY_cli_cmd_conf(req, 1, 1, 1, 1, 1);
}

static void DHCP_RELAY_cli_cmd_conf_mode(cli_req_t *req)
{
    DHCP_RELAY_cli_cmd_conf(req, 1, 0, 0, 0, 0);
}

static void DHCP_RELAY_cli_cmd_conf_add_server(cli_req_t *req)
{
    DHCP_RELAY_cli_cmd_conf(req, 0, 1, 0, 0, 0);
}

#if 0 /*No command table entry for this function, just keeping it for future use*/
static void DHCP_RELAY_cli_cmd_conf_del_server(cli_req_t *req)
{
    DHCP_RELAY_cli_cmd_conf(req, 0, 0, 1, 0, 0);
}
#endif

static void DHCP_RELAY_cli_cmd_conf_info_mode(cli_req_t *req)
{
    DHCP_RELAY_cli_cmd_conf(req, 0, 0, 0, 1, 0);
}

static void DHCP_RELAY_cli_cmd_conf_info_policy(cli_req_t *req)
{
    DHCP_RELAY_cli_cmd_conf(req, 0, 0, 0, 0, 1);
}

static void DHCP_RELAY_cli_cmd_stats(cli_req_t *req)
{
    dhcp_relay_stats_t stats;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if (req->clear) {
        dhcp_relay_stats_clear();
    } else {
        dhcp_relay_stats_get(&stats);
        CPRINTF("\nServer Statistics:");
        CPRINTF("\n------------------");
        CPRINTF("\nTransmit to Server         : %10d   Transmit Error               : %10d", stats.client_packets_relayed, stats.client_packet_errors);
        CPRINTF("\nReceive from Server        : %10d   Receive Missing Agent Option : %10d", stats.receive_server_packets, stats.missing_agent_option);
        CPRINTF("\nReceive Missing Circuit ID : %10d   Receive Missing Remote ID    : %10d", stats.missing_circuit_id, stats.missing_remote_id);
        CPRINTF("\nReceive Bad Circuit ID     : %10d   Receive Bad Remote ID        : %10d", stats.bad_circuit_id, stats.bad_remote_id);
        CPRINTF("\n\nClient Statistics:");
        CPRINTF("\n--------------------");
        CPRINTF("\nTransmit to Client   : %10d   Transmit Error       : %10d", stats.server_packets_relayed, stats.server_packet_errors);
        CPRINTF("\nReceive from Client  : %10d   Receive Agent Option : %10d", stats.receive_client_packets, stats.receive_client_agent_option);
        CPRINTF("\nReplace Agent Option : %10d   Keep Agent Option    : %10d", stats.replace_agent_option, stats.keep_agent_option);
        CPRINTF("\nDrop Agent Option    : %10d\n", stats.drop_agent_option);
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t DHCP_RELAY_cli_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    dhcp_relay_cli_req_t *dhcp_relay_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "replace", 7)) {
            dhcp_relay_req->relay_info_policy = DHCP_RELAY_INFO_POLICY_REPLACE;
        } else if (!strncmp(found, "keep", 4)) {
            dhcp_relay_req->relay_info_policy = DHCP_RELAY_INFO_POLICY_KEEP;
        } else if (!strncmp(found, "drop", 4)) {
            dhcp_relay_req->relay_info_policy = DHCP_RELAY_INFO_POLICY_DROP;
        }
    }

    return (found == NULL ? 1 : 0);
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t dhcp_relay_cli_parm_table[] = {
    {
        "replace|keep|drop",
        "replace : Replace the original relay information when receive\n"
        "          a DHCP message that already contains it\n"
        "keep    : Keep the original relay information when receive a\n"
        "          DHCP message that already contains it\n"
        "drop    : Drop the package when receive a DHCP message that\n"
        "          already contains relay information\n"
        "(default: Show DHCP relay information policy)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        DHCP_RELAY_cli_parse_keyword,
        NULL
    },
    {
        "clear",
        "Clear DHCP relay statistics",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        DHCP_RELAY_cli_cmd_stats
    },
    {
        "enable|disable",
        "enable : Enable DHCP relaly mode.\n"
        "         When enable DHCP relay mode operation, the agent forward\n"
        "         and to transfer DHCP messages between the clients and the\n"
        "         server when they are not on the same subnet domain. And the\n"
        "         DHCP broadcast message won't flood for security considered.\n"
        "disable: Disable DHCP relaly mode\n"
        "(default: Show flow DHCP relaly mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DHCP_RELAY_cli_cmd_conf_mode
    },
    {
        "enable|disable",
        "enable : Enable DHCP relay agent information option mode\n"
        "disable: Disable DHCP relay agent information option mode\n"
        "(default: Show DHCP relay agent information option mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DHCP_RELAY_cli_cmd_conf_info_mode
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    DHCP_RELAY_PRIO_CONF,
    DHCP_RELAY_PRIO_MODE,
    DHCP_RELAY_PRIO_ADD_SERVER,
    DHCP_RELAY_PRIO_DEL_SERVER,
    DHCP_RELAY_PRIO_INFO_MODE,
    DHCP_RELAY_PRIO_INFO_POLICY,
    DHCP_RELAY_PRIO_STATS
};

cli_cmd_tab_entry(
    "DHCP Relay Configuration",
    NULL,
    "Show DHCP relay configuration",
    DHCP_RELAY_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_conf_show,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "DHCP Relay Mode",
    "DHCP Relay Mode [enable|disable]",
    "Set or show the DHCP relay mode",
    DHCP_RELAY_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_conf_mode,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Relay Server",
    "DHCP Relay Server [<ip_addr>]",
    "Show or set DHCP relay server",
    DHCP_RELAY_PRIO_ADD_SERVER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_conf_add_server,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Relay Information Mode",
    "DHCP Relay Information Mode [enable|disable]",
    "Set or show DHCP relay agent information option mode.\n"
    "When enable DHCP relay information mode operation, the\n"
    "agent insert specific information (option 82) into a DHCP\n"
    "message when forwarding to DHCP server and remote it from\n"
    "a DHCP message when transferring to DHCP client. It only\n"
    "works under DHCP relay operation mode enabled",
    DHCP_RELAY_PRIO_INFO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_conf_info_mode,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Relay Information Policy",
    "DHCP Relay Information Policy [replace|keep|drop]",
    "Set or show the DHCP relay mode.\n"
    "When enable DHCP relay information mode operation, if agent\n"
    "receive a DHCP message that already contains relay agent\n"
    "information. It will enforce the policy",
    DHCP_RELAY_PRIO_INFO_POLICY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_conf_info_policy,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Relay Statistics",
    "DHCP Relay Statistics [clear]",
    "Show or clear DHCP relay statistics",
    DHCP_RELAY_PRIO_STATS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DHCP,
    DHCP_RELAY_cli_cmd_stats,
    NULL,
    dhcp_relay_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
