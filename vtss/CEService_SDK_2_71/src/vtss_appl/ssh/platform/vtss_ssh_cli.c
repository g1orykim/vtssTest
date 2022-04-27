/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "cli_grp_help.h"
#include "vtss_ssh_api.h"
#include "cli_trace_def.h"

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void SSH_cli_cmd_conf(cli_req_t *req, BOOL mode)
{
    ssh_conf_t conf;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        ssh_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        conf.mode = req->enable;
        (void) ssh_mgmt_conf_set(&conf);
    } else {
        if (mode) {
            CPRINTF("SSH Mode : %s\n", cli_bool_txt(conf.mode));
        }
    }
}

static void SSH_cli_cmd_conf_disp(cli_req_t *req)
{

    if (!req->set) {
        cli_header("SSH Configuration", 1);
    }

    SSH_cli_cmd_conf(req, 1);
    return;
}

static void SSH_cli_cmd_mode(cli_req_t *req)
{
    SSH_cli_cmd_conf(req, 1);
    return;
}

static void SSH_cli_cmd_debug_public_key(cli_req_t *req)
{
    u8 str_buff[64];


    ssh_mgmt_publickey_get(0, str_buff);
    CPRINTF("RSA : %s\n", str_buff);
    ssh_mgmt_publickey_get(1, str_buff);
    CPRINTF("DSA : %s\n", str_buff);

    return;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t ssh_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable SSH\n"
        "disable: Disable SSH\n"
        "(default: Show SSH mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        SSH_cli_cmd_mode
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    SSH_PRIO_CONF,
    SSH_PRIO_MODE,
    SSH_PRIO_DEBUG_PUBLIC_KEY_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SSH Configuration",
    NULL,
    "Show SSH configuration",
    SSH_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    SSH_cli_cmd_conf_disp,
    NULL,
    ssh_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SSH Mode",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SSH Mode [enable|disable]",
    "Set or show the SSH mode",
    SSH_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    SSH_cli_cmd_mode,
    NULL,
    ssh_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug SSH Public Key",
    NULL,
    "Show SSH public key fingerprint",
    SSH_PRIO_DEBUG_PUBLIC_KEY_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    SSH_cli_cmd_debug_public_key,
    NULL,
    ssh_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
