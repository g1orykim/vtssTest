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

#include "cli.h"            /* Should've been called cli_api.h!                        */
#include "cli_grp_help.h"  /* For Security group definition access */
#include "psec_limit_api.h" /* Interface to the module that this file supports         */
#include "psec_limit_cli.h" /* Check that public function decls. and defs. are in sync */
#include "cli_trace_def.h"  /* Import the CLI trace definitions                        */

#define PSEC_LIMIT_CLI_PATH "Limit "

/******************************************************************************/
// The order that the commands shall appear.
/******************************************************************************/
typedef enum {
    PSEC_LIMIT_CLI_PRIO_CONF,
    PSEC_LIMIT_CLI_PRIO_MODE,
    PSEC_LIMIT_CLI_PRIO_AGING,
    PSEC_LIMIT_CLI_PRIO_AGETIME,
    PSEC_LIMIT_CLI_PRIO_PORT,
    PSEC_LIMIT_CLI_PRIO_LIMIT,
    PSEC_LIMIT_CLI_PRIO_ACTION,
    PSEC_LIMIT_CLI_PRIO_REOPEN,
} psec_limit_cli_prio_t;

/******************************************************************************/
// This defines the things that this module can parse.
// The fields are filled in by the dedicated parsers, and used by the
// PSEC_LIMIT_cli_cmd() function.
/******************************************************************************/
typedef struct {
    psec_limit_action_t action;
} psec_limit_cli_req_t;

/******************************************************************************/
//
// Static variables
//
/******************************************************************************/

// Forward declaration of static functions
static void    PSEC_LIMIT_cli_cmd_conf    (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_mode    (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_aging   (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_agetime (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_port    (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_limit   (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_action  (cli_req_t *req);
static void    PSEC_LIMIT_cli_cmd_reopen  (cli_req_t *req);
static int32_t PSEC_LIMIT_cli_parse_agetime(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t PSEC_LIMIT_cli_parse_limit  (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t PSEC_LIMIT_cli_parse_action (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);

/******************************************************************************/
// Parameters used in this module
// Note to myself, because I forget it all the time:
// CLI_PARM_FLAG_NO_TXT:
//   If included in flags, then help text will look like:
//      enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   If excluded in flags, then help text will look like:
//     enable|disable: enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   I.e., the parameter name is printed first if excluded. And it looks silly
//   in some cases, but is OK in other.
//   If it's a pipe-separated list of keywords (e.g. enable|disable), then
//   the flag should generally be included.
//   If it's a triangle-parenthesis-enclosed keyword (e.g. <age_time>), then
//   the flag should generally be excluded.
/******************************************************************************/
static cli_parm_t PSEC_LIMIT_cli_parm_table[] = {
    {
        "enable|disable",
        "enable  : Globally enable port security\n"
        "disable : Globally disable port security\n"
        "(default: Show current global state of port security limit control)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        PSEC_LIMIT_cli_cmd_mode
    },
    {
        "enable|disable",
        "enable  : Enable port security on this port\n"
        "disable : Disable port security on this port\n"
        "(default: Show current port state of port security limit control)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        PSEC_LIMIT_cli_cmd_port
    },
    {
        "enable|disable",
        "enable  : Enable aging\n"
        "disable : Disable aging\n"
        "(default: Show current state of aging)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        PSEC_LIMIT_cli_cmd_aging
    },
    {
        "<age_time>",
        "Time in seconds between checks for activity on a MAC address (" vtss_xstr(PSEC_LIMIT_AGING_PERIOD_SECS_MIN) "-" vtss_xstr(PSEC_LIMIT_AGING_PERIOD_SECS_MAX) " seconds)\n"
        "(default: Show current age time)",
        CLI_PARM_FLAG_SET,
        PSEC_LIMIT_cli_parse_agetime,
        PSEC_LIMIT_cli_cmd_agetime
    },
    {
        "<limit>",
        "Max. number of MAC addresses on this port\n"
        "(default: Show current limit)",
        CLI_PARM_FLAG_SET,
        PSEC_LIMIT_cli_parse_limit,
        PSEC_LIMIT_cli_cmd_limit
    },
    {
        "none|trap|shut|trap_shut",
        "Action to be taken in case the number of MAC addresses exceeds the limit\n"
        "none     : Don't do anything\n"
        "trap     : Send an SNMP trap\n"
        "shut     : Shutdown the port\n"
        "trap_shut: Send an SNMP trap and shutdown the port\n"
        "(default: Show current action)",
        CLI_PARM_FLAG_SET,
        PSEC_LIMIT_cli_parse_action,
        PSEC_LIMIT_cli_cmd_action
    },
    {NULL, NULL, 0, 0, NULL}
};

/******************************************************************************/
// Commands defined in this module
/******************************************************************************/
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Configuration [<port_list>]",
    NULL,
    "Show Limit Control configuration",
    PSEC_LIMIT_CLI_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_conf,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Mode",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Mode [enable|disable]",
    "Set or show global state",
    PSEC_LIMIT_CLI_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_mode,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Aging",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Aging [enable|disable]",
    "Set or show aging state",
    PSEC_LIMIT_CLI_PRIO_AGING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_aging,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Agetime",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Agetime [<age_time>]",
    "Time in seconds between check for activity on learned MAC addresses",
    PSEC_LIMIT_CLI_PRIO_AGETIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_agetime,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Port [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Port [<port_list>] [enable|disable]",
    "Set or show per-port state",
    PSEC_LIMIT_CLI_PRIO_PORT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_port,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Limit [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Limit [<port_list>] [<limit>]",
    "Set or show the max. number of MAC addresses that can be learned on this set of ports",
    PSEC_LIMIT_CLI_PRIO_LIMIT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_limit,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Action [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Action [<port_list>] [none|trap|shut|trap_shut]",
    "Set or show the action involved with exceeding the limit",
    PSEC_LIMIT_CLI_PRIO_ACTION,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_action,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_LIMIT_CLI_PATH "Reopen [<port_list>]",
    "Reopen one or more ports whose limit is exceeded and shut down",
    PSEC_LIMIT_CLI_PRIO_REOPEN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    PSEC_LIMIT_cli_cmd_reopen,
    NULL,
    PSEC_LIMIT_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/******************************************************************************/
//
// Module Private Functions
//
/******************************************************************************/

/******************************************************************************/
// PSEC_LIMIT_cli_cmd()
// The master of all CLI functions. Takes care of all status and configuration.
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd(cli_req_t *req, BOOL cmd_mode, BOOL cmd_aging, BOOL cmd_agetime, BOOL cmd_port, BOOL cmd_limit, BOOL cmd_action)
{
    psec_limit_glbl_cfg_t   glbl_cfg;
    psec_limit_switch_cfg_t switch_cfg;
    psec_switch_status_t    switch_status;
    psec_limit_port_cfg_t   *port_cfg;
    vtss_rc                 rc;
    psec_limit_cli_req_t    *psec_limit_req = (psec_limit_cli_req_t *)req->module_req;
    switch_iter_t           sit;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if ((rc = psec_limit_mgmt_glbl_cfg_get(&glbl_cfg)) != VTSS_OK) {
        cli_printf("%s\n", error_txt(rc));
        return;
    }

    // Handle the global configuration
    if (cmd_mode || cmd_aging || cmd_agetime) {

        if (req->set) {
            // Global configuration set request
            if (cmd_mode) {
                glbl_cfg.enabled = req->enable;
            }
            if (cmd_aging) {
                glbl_cfg.enable_aging = req->enable;
            }
            if (cmd_agetime) {
                glbl_cfg.aging_period_secs = req->value;
            }
            if ((rc = psec_limit_mgmt_glbl_cfg_set(&glbl_cfg)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
            }
        } else {
            // Global configuration get request
            if (cmd_mode) {
                cli_printf("Mode      : %s\n", cli_bool_txt(glbl_cfg.enabled));
            }
            if (cmd_aging) {
                cli_printf("Aging     : %s\n", cli_bool_txt(glbl_cfg.enable_aging));
            }
            if (cmd_agetime) {
                cli_printf("Age Period: %u\n", glbl_cfg.aging_period_secs);
            }
        }
    }

    if (!cmd_port && !cmd_limit && !cmd_action) {
        return;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;

        if ((rc = psec_limit_mgmt_switch_cfg_get(sit.isid, &switch_cfg)) != VTSS_OK) {
            cli_printf("%s\n", error_txt(rc));
            continue;
        }

        if (!req->set) {
            if ((rc = psec_mgmt_switch_status_get(sit.isid, &switch_status)) != VTSS_RC_OK) {
                cli_printf("%s\n", error_txt(rc));
                continue;
            }
        }

        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            port_cfg = &switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START];

            if (req->set) {
                // Port configuration set request
                if (cmd_port) {
                    port_cfg->enabled = req->enable;
                }
                if (cmd_limit) {
                    port_cfg->limit = req->value;
                }
                if (cmd_action) {
                    port_cfg->action = psec_limit_req->action;
                }
            } else {
                psec_port_status_t *port_status = &switch_status.port_status[pit.iport - VTSS_PORT_NO_START];
                int                state;

                // Port configuration get request. For simplicity, show all - independent of the cmd
                if (pit.first) {
                    cli_cmd_usid_print(sit.usid, req, 1);
                    cli_table_header("Port  Mode      Limit  Action           State          ");
                }

                if (glbl_cfg.enabled && port_cfg->enabled) {
                    if (port_status->shutdown) {
                        state = 3; // Shutdown
                    } else if (port_status->limit_reached) {
                        state = 2; // Limit reached
                    } else {
                        state = 1; // Ready
                    }
                } else {
                    if (port_status->limit_reached || port_status->shutdown) {
                        // As said, it shouldn't be possible to have a port in its limit_reached or shutdown state
                        // if PSEC LIMIT Control is not enabled on that port.
                        T_E("Internal error.");
                    }
                    state = 0; // Disabled
                }

                cli_printf("%4d  %-8s  %5u  %-15s  %s\n",
                           pit.uport,
                           cli_bool_txt(port_cfg->enabled),
                           port_cfg->limit,
                           port_cfg->action == PSEC_LIMIT_ACTION_NONE              ? "None" :
                           port_cfg->action == PSEC_LIMIT_ACTION_TRAP              ? "Trap" :
                           port_cfg->action == PSEC_LIMIT_ACTION_SHUTDOWN          ? "Shutdown" :
                           port_cfg->action == PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN ? "Trap & Shutdown" : "?",
                           state == 3 ? "Shutdown"      :
                           state == 2 ? "Limit Reached" :
                           state == 1 ? "Ready"         : "Disabled"
                          );
            }
        } // while (cli_port_iter_getnext())

        if (req->set) {
            if ((rc = psec_limit_mgmt_switch_cfg_set(sit.isid, &switch_cfg)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
            }
        }
    } // while (cli_switch_iter_getnext())
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_conf()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Port Security Limit Control Configuration", 1);
    }
    PSEC_LIMIT_cli_cmd(req, 1, 1, 1, 1, 1, 1);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_mode()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_mode(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 1, 0, 0, 0, 0, 0);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_aging()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_aging(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 0, 1, 0, 0, 0, 0);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_agetime()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_agetime(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 0, 0, 1, 0, 0, 0);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_port()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_port(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 0, 0, 0, 1, 0, 0);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_limit()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_limit(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 0, 0, 0, 0, 1, 0);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_action()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_action(cli_req_t *req)
{
    PSEC_LIMIT_cli_cmd(req, 0, 0, 0, 0, 0, 1);
}

/******************************************************************************/
// PSEC_LIMIT_cli_cmd_reopen()
/******************************************************************************/
static void PSEC_LIMIT_cli_cmd_reopen(cli_req_t *req)
{
    vtss_rc         rc;
    switch_iter_t   sit;

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            if ((rc = psec_mgmt_reopen_port(PSEC_USER_PSEC_LIMIT, sit.isid, pit.iport)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                return;
            }
        }
    }
}

/******************************************************************************/
// PSEC_LIMIT_cli_parse_agetime()
/******************************************************************************/
static int32_t PSEC_LIMIT_cli_parse_agetime(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, PSEC_LIMIT_AGING_PERIOD_SECS_MIN, PSEC_LIMIT_AGING_PERIOD_SECS_MAX);
}

/******************************************************************************/
// PSEC_LIMIT_cli_parse_limit()
/******************************************************************************/
static int32_t PSEC_LIMIT_cli_parse_limit(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, PSEC_LIMIT_LIMIT_MIN, PSEC_LIMIT_LIMIT_MAX);
}

/******************************************************************************/
// PSEC_LIMIT_cli_parse_action()
/******************************************************************************/
static int32_t PSEC_LIMIT_cli_parse_action(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char                 *found         = cli_parse_find(cmd, stx);
    psec_limit_cli_req_t *psec_limit_req = (psec_limit_cli_req_t *)req->module_req;

    req->parm_parsed = 1;

    if (!found) {
        return 1;
    } else if (!strncmp(found, "trap_shut", 9)) {
        // Longer identical matches first.
        psec_limit_req->action = PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN;
    } else if (!strncmp(found, "none", 4)) {
        psec_limit_req->action = PSEC_LIMIT_ACTION_NONE;
    } else if (!strncmp(found, "trap", 4)) {
        psec_limit_req->action = PSEC_LIMIT_ACTION_TRAP;
    } else if (!strncmp(found, "shut", 4)) {
        psec_limit_req->action = PSEC_LIMIT_ACTION_SHUTDOWN;
    } else {
        return 1;
    }

    return 0;
}

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// psec_limit_cli_init()
/******************************************************************************/
void psec_limit_cli_init(void)
{
    // Register the size required for this module's structure
    cli_req_size_register(sizeof(psec_limit_cli_req_t));
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
