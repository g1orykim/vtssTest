/*

 Vitesse Switch API software.

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
#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "vtss_module_id.h"
#include "vtss_sntp_api.h"
#include "cli_trace_def.h"

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_cmd_ip_sntp_conf(cli_req_t *req, BOOL mode, BOOL add, BOOL del)
{
    sntp_conf_t     conf;
    int             i;
    vtss_rc         rc;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req)  ||
        sntp_config_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode = req->enable;
            rc = sntp_config_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("SNTP mode configuration failed\n");
            }
        }
        if (add) {
            conf.ip_type = SNTP_IP_TYPE_IPV4;
            strcpy((char *) conf.sntp_server, req->host_name);
            rc = sntp_config_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("SNTP entry addition failed\n");
            }
        }
    } else {
        if (mode) {
            CPRINTF("SNTP Mode : %s\n", cli_bool_txt(conf.mode));
        }
        if (mode && add) {
            CPRINTF("Idx   Server IP host address (a.b.c.d)\n");
            CPRINTF("---   ------------------------------------------------------\n");
            for (i = 0; i < SNTP_MAX_SERVER_COUNT; i++) {
                CPRINTF("%-5d ", i + 1);
                CPRINTF("%-16s \n", conf.sntp_server);
            }
        }
    }

    if (del) {
        if (conf.ip_type == SNTP_IP_TYPE_IPV4) {
            if (strcmp((char *) conf.sntp_server, "") == 0) {
                CPRINTF("Doesn't allowed to delete empty server\n");
                return;
            }
            memset(&conf.sntp_server, 0, sizeof(conf.sntp_server));
            rc = sntp_config_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("SNTP entry deletion failed\n");
            }
        } else {
            return;
        }
    }
}

static void cli_cmd_ip_sntp_config ( cli_req_t *req )
{
    if (!req->set) {
        cli_header("IP SNTP Configuration", 1);
    }
    cli_cmd_ip_sntp_conf(req, 1, 1, 0);
}

static void cli_cmd_ip_sntp_mode ( cli_req_t *req )
{
    cli_cmd_ip_sntp_conf(req, 1, 0, 0);
}

static void cli_cmd_ip_sntp_add ( cli_req_t *req )
{
    cli_cmd_ip_sntp_conf(req, 0, 1, 0);
}

static void cli_cmd_ip_sntp_del ( cli_req_t *req )
{
    cli_cmd_ip_sntp_conf(req, 0, 0, 1);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/
#if 0
static int32_t cli_sntp_server_index_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 1, SNTP_MAX_SERVER_COUNT);

    return error;
}
#endif

static int32_t cli_sntp_server_ip_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &req->ipv4_addr, NULL, &req->ipv4_addr_spec, 0);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t sntp_cli_parm_table[] = {
    {
        "enable|disable",
        "enable       : Enable SNTP mode\n"
        "disable      : Disable SNTP mode\n"
        "(default: Show SNTP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ip_sntp_mode
    },
#if 0
    {
        "<server_index>",
        "The server index (1-" vtss_xstr(SNTP_MAX_SERVER_COUNT) ")",
        CLI_PARM_FLAG_SET,
        cli_sntp_server_index_parse,
        NULL
    },
#endif
    {
        "<server_ip>",
        "Server IP address (a.b.c.d)",
        CLI_PARM_FLAG_SET,
        cli_sntp_server_ip_parse,
        cli_cmd_ip_sntp_add
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
/* SNTP CLI Command Sorting Order */
enum {
    PRIO_IP_SNTP_CONF = 2 * CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_IP_SNTP_MODE,
    PRIO_IP_SNTP_ADD,
    PRIO_IP_SNTP_DEL,
};

cli_cmd_tab_entry (
    "IP SNTP Configuration",
    NULL,
    "Show SNTP configuration",
    PRIO_IP_SNTP_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_config,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IP SNTP Mode",
    "IP SNTP Mode [enable|disable]",
    "Set or show the SNTP mode",
    PRIO_IP_SNTP_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_mode,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if 0
cli_cmd_tab_entry (
    NULL,
    "IP SNTP Server Add <server_index> <ip_addr_string>",
    "Add SNTP server entry",
    PRIO_IP_SNTP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_add,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IP SNTP Server Delete <server_index>",
    "Delete SNTP server entry",
    PRIO_IP_SNTP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_del,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry (
    NULL,
    "IP SNTP Server Add <ip_addr_string>",
    "Add SNTP server entry",
    PRIO_IP_SNTP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_add,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IP SNTP Server Delete",
    "Delete SNTP server entry",
    PRIO_IP_SNTP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_sntp_del,
    NULL,
    sntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
