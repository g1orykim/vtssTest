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
//#include "cli_trace_def.h"
#include "loop_protect_api.h"

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_loop_protect_conf(cli_req_t *req,
                                  BOOL enable,
                                  BOOL txtime, 
                                  BOOL shuttime)
{
    loop_protect_conf_t sc;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    (void) loop_protect_conf_get(&sc);

    if (req->set) {
        if (enable)
            sc.enabled = req->enable;
        if (txtime)
            sc.transmission_time = req->value;
        if (shuttime)
            sc.shutdown_time = req->value;
        (void) loop_protect_conf_set(&sc);
    } else {
        if (enable)
            CPRINTF("Loop Protection  : %sabled\n", sc.enabled ? "En" : "Dis");
        if (txtime)
            CPRINTF("Transmission Time: %u\n", sc.transmission_time);
        if (shuttime)
            CPRINTF("Shutdown Time    : %u\n", sc.shutdown_time);
    }
}

static void cli_loop_protect_port_conf_unit(cli_req_t *req,
                                            switch_iter_t *sit,
                                            BOOL enable, 
                                            BOOL action,
                                            BOOL txmode)
{
    port_iter_t pit;
    char buf[80], *p;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    (void) port_iter_init(&pit, NULL, sit->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (cli_port_iter_getnext(&pit, req)) {
        loop_protect_port_conf_t pconf;
        if ((loop_protect_conf_port_get(sit->isid, pit.iport, &pconf) != VTSS_OK))
            continue;
        if (req->set) {
            if(enable)
                pconf.enabled = req->enable;
            if(action)
                pconf.action = req->value;
            if(txmode)
                pconf.transmit = req->enable;
            (void) loop_protect_conf_port_set(sit->isid, pit.iport, &pconf);
        } else {
            if (pit.first) {
                cli_cmd_usid_print(sit->usid, req, 1);
                p = &buf[0];
                p += sprintf(p, "Port  ");
                if (enable)
                    p += sprintf(p, "Mode      ");
                if (action)
                    p += sprintf(p, "Action        ");
                if (txmode)
                    p += sprintf(p, "Transmit  ");
                cli_table_header(buf);
            }
            CPRINTF("%-2u    ", pit.uport);
            if (enable)
                CPRINTF("%s  ", cli_bool_txt(pconf.enabled));
            if (action)
                CPRINTF("%-12.12s  ", loop_protect_action2string(pconf.action));
            if (txmode)
                CPRINTF("%s  ", cli_bool_txt(pconf.transmit));
            CPRINTF("\n");
        }
    }
}

static void cli_cmd_loop_protect_conf(cli_req_t *req)
{
    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    if(!req->set) {
        cli_header("Loop Protection Configuration", 1);
    }
    cli_loop_protect_conf(req, 1, 1, 1);
}

static void cli_cmd_loop_protect_enable(cli_req_t *req)
{
    cli_loop_protect_conf(req, 1, 0, 0);
}

static void cli_cmd_loop_protect_txtime(cli_req_t *req)
{
    cli_loop_protect_conf(req, 0, 1, 0);
}

static void cli_cmd_loop_protect_shuttime(cli_req_t *req)
{
    cli_loop_protect_conf(req, 0, 0, 1);
}

static void cli_loop_protect_port_conf(cli_req_t *req,
                                       BOOL enable, 
                                       BOOL action,
                                       BOOL txmode)
{
    switch_iter_t sit;
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (cli_switch_iter_getnext(&sit, req)) {
        cli_loop_protect_port_conf_unit(req, &sit, enable, action, txmode);
    }
}

static void cli_cmd_loop_protect_port_conf(cli_req_t *req)
{
    cli_loop_protect_port_conf(req, 1, 1, 1);
}

static void cli_cmd_loop_protect_port_mode(cli_req_t *req)
{
    cli_loop_protect_port_conf(req, 1, 0, 0);
}

static void cli_cmd_loop_protect_port_action(cli_req_t *req)
{
    cli_loop_protect_port_conf(req, 0, 1, 0);
}

static void cli_cmd_loop_protect_port_txmode(cli_req_t *req)
{
    cli_loop_protect_port_conf(req, 0, 0, 1);
}

static void cli_cmd_loop_protect_status_unit(cli_req_t *req, switch_iter_t *sit)
{
    port_iter_t pit;

    cli_cmd_usid_print(sit->usid, req, 1);
    cli_table_header("Port  Action        Transmit  Loops     Status    Loop  Time of Last Loop");

    (void) port_iter_init(&pit, NULL, sit->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (cli_port_iter_getnext(&pit, req)) {
        loop_protect_port_info_t linfo;
        loop_protect_port_conf_t pconf;
        port_status_t pinfo;
        if((loop_protect_port_info_get(sit->isid, pit.iport, &linfo) == VTSS_OK) &&
           (loop_protect_conf_port_get(sit->isid, pit.iport, &pconf) == VTSS_OK)) {
            CPRINTF("%-4d  %-12s  %-8s  %8d  %-8s  %-4s  %s\n",
                    pit.uport,
                    loop_protect_action2string(pconf.action),
                    pconf.transmit ? "Enabled" : "Disabled",
                    linfo.loops,
                    linfo.disabled ? "Disabled" : 
                    ((port_mgmt_status_get(sit->isid, pit.iport, &pinfo) == VTSS_OK) && pinfo.status.link) ? "Up" : "Down",
                    linfo.loop_detect ? "Loop" : "-",
                    linfo.loops ? misc_time2str(linfo.last_loop) : "-");
        }
    }
}

static void cli_cmd_loop_protect_status(cli_req_t *req)
{
    switch_iter_t sit;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (cli_switch_iter_getnext(&sit, req)) {
        cli_cmd_loop_protect_status_unit(req, &sit);
    }
}


/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int loop_cli_parm_parse_txtime(char *cmd, char *cmd2,
                                      char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 1, 10);
    return(error);
}

static int loop_cli_parm_parse_shuttime(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 0, 60 * 60 * 24 * 7);
    return(error);
}

static int loop_cli_parm_parse_action (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                       cli_req_t *req)
{
    int error = 1;
    char *found = cli_parse_find(cmd, stx);
    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "shutdown", 8)) {
            req->value = LOOP_PROTECT_ACTION_SHUTDOWN;
            error = 0;
        } else if (!strncmp(found, "shut_log", 8)) {
            req->value = LOOP_PROTECT_ACTION_SHUT_LOG;
            error = 0;
        } else if (!strncmp(found, "log", 3)) {
            req->value = LOOP_PROTECT_ACTION_LOG_ONLY;
            error = 0;
        }
    }
    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t loop_protect_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable Loop Protection\n"
        "disable: Disable Loop Protection",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL
    },
    {
        "<transmit-time>",
        "Transmit time interval (1-10 seconds)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        loop_cli_parm_parse_txtime,
        cli_cmd_loop_protect_txtime
    },
    {
        "<shutdown-time>",
        "Shutdown time interval (0-604800 seconds)\n"
        "A value of zero disables re-enabling the port",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        loop_cli_parm_parse_shuttime,
        cli_cmd_loop_protect_shuttime
    },
    {
        "shutdown|shut_log|log",
        "shutdown : Shutdown the port\n"
        "shut_log : Shutdown the port and Log event\n"
        "log      : (Only) Log the event",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        loop_cli_parm_parse_action,
        cli_cmd_loop_protect_port_action
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* LOOP_PROT CLI Command Sorting Order */
enum {
    CLI_CMD_LOOP_PROTECT_CONF_PRIO = 0,
    CLI_CMD_LOOP_PROTECT_ENABLE_PRIO,
    CLI_CMD_LOOP_PROTECT_TXTIME_PRIO,
    CLI_CMD_LOOP_PROTECT_SHUTTIME_PRIO,
    CLI_CMD_LOOP_PROTECT_PORT_CONF_PRIO,
    CLI_CMD_LOOP_PROTECT_PORT_MODE_PRIO,
    CLI_CMD_LOOP_PROTECT_PORT_ACTION_PRIO,
    CLI_CMD_LOOP_PROTECT_PORT_TXMODE_PRIO,
    CLI_CMD_LOOP_PROTECT_STATUS_PRIO,
};

/* Command table entries */
cli_cmd_tab_entry(
    "Loop Protect Configuration",
    NULL,
    "Show Loop Protection configuration",
    CLI_CMD_LOOP_PROTECT_CONF_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_conf,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
    );


cli_cmd_tab_entry (
    "Loop Protect Mode",
    "Loop Protect Mode [enable|disable]",
    "Set or show the Loop Protection mode",
    CLI_CMD_LOOP_PROTECT_ENABLE_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_enable,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "Loop Protect Transmit",
    "Loop Protect Transmit [<transmit-time>]",
    "Set or show the Loop Protection transmit interval",
    CLI_CMD_LOOP_PROTECT_TXTIME_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_txtime,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "Loop Protect Shutdown",
    "Loop Protect Shutdown [<shutdown-time>]",
    "Set or show the Loop Protection shutdown time",
    CLI_CMD_LOOP_PROTECT_SHUTTIME_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_shuttime,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "Loop Protect Port Configuration [<port_list>]",
    NULL,
    "Show Loop Protection port configuration",
    CLI_CMD_LOOP_PROTECT_PORT_CONF_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_port_conf,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
    "Loop Protect Port Mode",
    "Loop Protect Port Mode [<port_list>] [enable|disable]",
    "Set or show the Loop Protection port mode",
    CLI_CMD_LOOP_PROTECT_PORT_MODE_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_port_mode,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
    "Loop Protect Port Action",
    "Loop Protect Port Action [<port_list>] [shutdown|shut_log|log]",
    "Set or show the Loop Protection port action",
    CLI_CMD_LOOP_PROTECT_PORT_ACTION_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_port_action,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
    "Loop Protect Port Transmit",
    "Loop Protect Port Transmit [<port_list>] [enable|disable]",
    "Set or show the Loop Protection port transmit mode",
    CLI_CMD_LOOP_PROTECT_PORT_TXMODE_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_port_txmode,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
    "Loop Protect Status [<port_list>]",
    NULL,
    "Show the Loop Protection status",
    CLI_CMD_LOOP_PROTECT_STATUS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LOOP_PROTECT,
    cli_cmd_loop_protect_status,
    NULL,
    loop_protect_cli_parm_table,
    CLI_CMD_FLAG_NONE
    );

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
