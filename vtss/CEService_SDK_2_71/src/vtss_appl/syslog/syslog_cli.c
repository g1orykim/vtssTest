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
#include "cli_api.h"
#include "syslog_api.h"
#include "syslog_cli.h"
#include "misc_api.h"   //misc_strncpyz()

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SYSLOG

typedef struct {
    syslog_cat_t    syslog_cat;
    syslog_lvl_t    syslog_lvl;
    BOOL            syslog_store_type;
    ulong           log_min;
    ulong           log_max;
    cli_spec_t      udp_port_spec;
    ushort          udp_port;
    ulong           repeat_cnt;
} syslog_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void syslog_cli_req_init(void)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(syslog_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_cmd_debug_syslog_show(cli_req_t *req )
{
    syslog_cli_req_t *syslog_req = req->module_req;

    syslog_flash_print(syslog_req->syslog_cat, syslog_req->syslog_lvl, cli_printf);
}

/* Debug syslog erase */
static void cli_cmd_debug_syslog_erase(cli_req_t *req )
{
    CPRINTF("Deleting syslog from flash...");
    cli_flush();
    if (syslog_flash_erase()) {
        CPRINTF("Done!\n");
    } else {
        CPRINTF("Flash is in use by another process. Please try again later...\n");
    }
}

/* RAM syslog */
static void cli_cmd_syslog_ram(cli_req_t *req, BOOL debug)
{
    vtss_usid_t        usid;
    vtss_isid_t        isid;
    syslog_ram_entry_t *entry;
    syslog_ram_stat_t  stat;
    ulong              total;
    syslog_lvl_t       lvl;
    int                i;
    BOOL               first;
    syslog_cli_req_t   *syslog_req = req->module_req;

    if ((entry = VTSS_MALLOC(sizeof(*entry))) == NULL) {
        return;
    }

    /* Log all entries by default */
    if (syslog_req->log_min == 0) {
        syslog_req->log_min = 1;
    }
    if (syslog_req->log_max == 0) {
        syslog_req->log_max = 0xffffffff;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        isid = (debug ? (usid == VTSS_USID_START ? VTSS_ISID_LOCAL : VTSS_ISID_END) :
                req->stack.isid[usid]);
        if (isid == VTSS_ISID_END) {
            continue;
        }

        for (entry->id = (syslog_req->log_min - 1), first = 1;
             syslog_ram_get(isid, 1, entry->id, syslog_req->syslog_lvl, VTSS_MODULE_ID_NONE, entry) &&
             entry->id <= syslog_req->log_max; ) {
            if (first && !debug) {
                cli_cmd_usid_print(usid, req, 0);
            }

            if (syslog_req->log_min == syslog_req->log_max) {
                /* Detailed view */
                CPRINTF("ID     : %d\n", entry->id);
                CPRINTF("Level  : %s\n", syslog_lvl_to_string(entry->lvl, FALSE));
                CPRINTF("Time   : %s\n", misc_time2str(entry->time));
                CPRINTF("Message:\n\n");
                cli_puts(entry->msg);
                CPRINTF("\n");
            } else {
                if (first) {
                    if (syslog_ram_stat_get(isid, &stat) == VTSS_OK) {
                        CPRINTF("Number of entries:\n");
                        for (total = 0, lvl = 0; lvl < SYSLOG_LVL_ALL; lvl++) {
                            total += stat.count[lvl];
                            CPRINTF("%-7s: %d\n", syslog_lvl_to_string(lvl, FALSE), stat.count[lvl]);
                        }
                        CPRINTF("%-7s: %d\n\n", "All", total);
                    }
                    cli_table_header("ID    Level   Time                       Message");
                }

                /* The summary message length is limited to 35 characters or one line */
                for (i = 0; i < 35; i++) {
                    if (entry->msg[i] == '\n') {
                        entry->msg[i] = '\0';
                        break;
                    }
                }
                if (i == 35) {
                    strcpy(&entry->msg[i], " ...");
                }
                CPRINTF("%4d  %-7s %s  %s\n",
                        entry->id,
                        syslog_lvl_to_string(entry->lvl, FALSE),
                        misc_time2str(entry->time),
                        entry->msg);
            }
            first = 0;
        }
    }
    VTSS_FREE(entry);
}

static void cli_cmd_debug_syslog_ram_debug(cli_req_t *req)
{
    cli_cmd_syslog_ram(req, 1);
}

static void cli_cmd_debug_syslog_ram_nodebug(cli_req_t *req)
{
    cli_cmd_syslog_ram(req, 0);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_syslog_category_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    if (!found) {
        return 1;
    }

    req->parm_parsed = 1;

    if (!strncmp(found, "all", 3)) {
        syslog_req->syslog_cat = SYSLOG_CAT_ALL;
    } else if (!strncmp(found, "debug", 5)) {
        syslog_req->syslog_cat = SYSLOG_CAT_DEBUG;
    } else if (!strncmp(found, "system", 6)) {
        syslog_req->syslog_cat = SYSLOG_CAT_SYSTEM;
    } else if (!strncmp(found, "application", 11)) {
        syslog_req->syslog_cat = SYSLOG_CAT_APP;
    } else {
        return 1; // Unreachable
    }

    return 0;
}

static int32_t cli_syslog_level_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    if (!found) {
        return 1;
    }

    req->parm_parsed = 1;

    if (!strncmp(found, "info", 4)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_INFO;
    } else if (!strncmp(found, "warning", 7)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_WARNING;
    } else if (!strncmp(found, "error", 5)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_ERROR;
    } else {
        return 1; // Unreachable
    }

    return 0;
}

static int32_t cli_syslog_store_type_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    if (!found) {
        return 1;
    }

    req->parm_parsed = 1;

    if (!strncmp(found, "ram", 3)) {
        syslog_req->syslog_store_type = 0;
    } else if (!strncmp(found, "flash", 5)) {
        syslog_req->syslog_store_type = 1;
    } else {
        return 1; // Unreachable
    }

    return 0;
}

static int32_t cli_syslog_show_level_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    if (!found) {
        return 1;
    }

    req->parm_parsed = 1;

    if (!strncmp(found, "all", 3)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_ALL;
    } else if (!strncmp(found, "info", 4)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_INFO;
    } else if (!strncmp(found, "warning", 7)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_WARNING;
    } else if (!strncmp(found, "error", 5)) {
        syslog_req->syslog_lvl = SYSLOG_LVL_ERROR;
    } else {
        return 1; // Unreachable
    }

    return 0;
}

static int32_t cli_syslog_range_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_range(cmd, &syslog_req->log_min, &syslog_req->log_max, 1, 0xffffffff);
}

static int32_t cli_syslog_udp_port_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                         cli_req_t *req)
{
    int32_t             error;
    ulong               value = 0;
    syslog_cli_req_t    *syslog_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 65535);
    if (!error) {
        syslog_req->udp_port_spec = CLI_SPEC_VAL;
        syslog_req->udp_port = (ushort) value;
    }

    return error;
}

static int32_t cli_syslog_cnt_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t             error;
    ulong               value = 0;
    syslog_cli_req_t    *syslog_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 65535);
    if (!error) {
        syslog_req->udp_port_spec = CLI_SPEC_VAL;
        syslog_req->repeat_cnt = value;
    }

    return error;
}

static void syslog_req_default_set(cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;

    syslog_req->syslog_cat    = SYSLOG_CAT_ALL;
    syslog_req->syslog_lvl    = SYSLOG_LVL_ALL;
    syslog_req->repeat_cnt  = 1;
}

/******************************************************************************/
// syslog_cli_cmd_conf()
/******************************************************************************/
static void syslog_cli_cmd_conf(cli_req_t *req, BOOL mode, BOOL addr, BOOL level)
{
    syslog_cli_req_t    *syslog_req = req->module_req;
    syslog_conf_t       conf;
    vtss_rc             rc;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) ||
        syslog_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.server_mode = req->enable;
        }
        if (addr) {
            if (req->host_name_spec == CLI_SPEC_VAL) {
                misc_strncpyz(conf.syslog_server, req->host_name, VTSS_SYS_HOSTNAME_LEN);
            }
            if (syslog_req->udp_port_spec == CLI_SPEC_VAL) {
                conf.udp_port = syslog_req->udp_port;
            }
        }
        if (level) {
            conf.syslog_level = syslog_req->syslog_lvl;
        }
        if ((rc = syslog_mgmt_conf_set(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    } else {
        if (mode) {
            CPRINTF("System Log Server Mode     : %s\n", cli_bool_txt(conf.server_mode));
        }
        if (addr) {
            CPRINTF("System Log Server Address  : %s\n", conf.syslog_server);
            //CPRINTF("System Log Server UDP Port : %d\n", conf.udp_port);
        }
        if (level) {
            CPRINTF("System Log Level           : %s\n", syslog_lvl_to_string(conf.syslog_level, FALSE));
        }
    }
}

/******************************************************************************/
// syslog_cli_cmd_conf_show()
/******************************************************************************/
static void syslog_cli_cmd_conf_show(cli_req_t *req)
{
    cli_header("System Log Configuration", 1);
    syslog_cli_cmd_conf(req, 1, 1, 1);
}

/******************************************************************************/
// syslog_cli_cmd_conf_server_mode()
/******************************************************************************/
static void syslog_cli_cmd_conf_server_mode(cli_req_t *req)
{
    syslog_cli_cmd_conf(req, 1, 0, 0);
}

/******************************************************************************/
// syslog_cli_cmd_conf_server_addr()
/******************************************************************************/
static void syslog_cli_cmd_conf_server_addr(cli_req_t *req)
{
    syslog_cli_cmd_conf(req, 0, 1, 0);
}

/******************************************************************************/
// syslog_cli_cmd_debug_conf_server_addr()
/******************************************************************************/
static void syslog_cli_cmd_debug_conf_server_addr(cli_req_t *req)
{
    syslog_cli_cmd_conf(req, 0, 1, 0);
}

/******************************************************************************/
// syslog_cli_cmd_test_msg()
/******************************************************************************/
static void syslog_cli_cmd_test_msg(cli_req_t *req)
{
    syslog_cli_req_t *syslog_req = req->module_req;

    if (req->set) {
        while (syslog_req->repeat_cnt--) {
            syslog_ram_log(syslog_req->syslog_lvl, SYSLOG_LVL_INFO, "This is a syslog test message.");
        }
        if (syslog_req->syslog_store_type) {
            syslog_flash_log(SYSLOG_CAT_DEBUG, syslog_req->syslog_lvl, "This is a syslog test message.");
        }
    }
}

/******************************************************************************/
// syslog_cli_cmd_conf_level()
/******************************************************************************/
static void syslog_cli_cmd_conf_level(cli_req_t *req)
{
    syslog_cli_cmd_conf(req, 0, 0, 1);
}

/******************************************************************************/
// syslog_cli_cmd_clear()
/******************************************************************************/
static void syslog_cli_cmd_clear(cli_req_t *req)
{
    vtss_usid_t        usid;
    vtss_isid_t        isid;
    syslog_cli_req_t   *syslog_req = req->module_req;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        syslog_ram_clear(isid, syslog_req->syslog_lvl);
    }
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t syslog_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable system log server mode\n"
        "disable: Disable system log server mode\n"
        "(default: Show system Log server mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        syslog_cli_cmd_conf_server_mode
    },
    {
        "all|debug|system|application",
        "all          : Show all categories (default)\n"
        "debug        : Show debug category\n"
        "system       : Show system category\n"
        "application  : Show application category\n",
        CLI_PARM_FLAG_NO_TXT,
        cli_syslog_category_parse,
        cli_cmd_debug_syslog_show
    },
    {
        "<udp_port>",
        "UDP port No. (0-65535)",
        CLI_PARM_FLAG_SET,
        cli_syslog_udp_port_parse,
        syslog_cli_cmd_debug_conf_server_addr
    },
    {
        "info|warning|error",
        "info    : Send informations, warnings and errors\n"
        "warning : Send warnings and errors\n"
        "error   : Send errors",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_syslog_level_parse,
        NULL
    },
    {
        "all|info|warning|error",
        "all     : Show all levels (default)\n"
        "info    : Show informations\n"
        "warning : Show warnings\n"
        "error   : Show errors",
        CLI_PARM_FLAG_NO_TXT,
        cli_syslog_show_level_parse,
        NULL
    },
    {
        "<log_id>",
        "System log ID or range (default: All entries)",
        CLI_PARM_FLAG_NONE,
        cli_syslog_range_parse,
        NULL
    },
    {
        "ram|flash",
        "ram     : Store message to RAM (default)\n"
        "flash   : Store message to Flash (Tip: Don't forget use command 'debug syslog erase' to clean the test message)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_syslog_store_type_parse,
        NULL
    },
    {
        "<repeat_cnt>",
        "Repeat counter",
        CLI_PARM_FLAG_NONE,
        cli_syslog_cnt_parse,
        NULL
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

enum {
    PRIO_SYSTEM_LOG_CONF,
    PRIO_SYSTEM_LOG_SERVER_MODE,
    PRIO_SYSTEM_LOG_SERVER_ADDR,
    PRIO_SYSTEM_LOG_SERVER_LEVEL,
    PRIO_SYSTEM_LOG_TEST_MSG,
    PRIO_SYSTEM_LOG_LOOKUP,
    PRIO_SYSTEM_LOG_CLEAR,
};

cli_cmd_tab_entry (
    "Debug Syslog Show [all|debug|system|application] [all|info|warning|error]",
    NULL,
    "Show the current system log as read from flash",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_syslog_show,
    syslog_req_default_set,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Syslog Erase",
    "Erase the current system log from flash",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_syslog_erase,
    syslog_req_default_set,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Syslog RAM [<log_id>] [all|info|warning|error]",
    NULL,
    "Show the local RAM system log",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_syslog_ram_debug,
    syslog_req_default_set,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Syslog Server Address",
    "Debug Syslog Server Address [<ip_addr_string>] [<udp_port>]",
    "Show or set the system log server address",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    syslog_cli_cmd_debug_conf_server_addr,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Syslog Test [info|warning|error] [ram|flash] [<repeat_cnt>]",
    "Add a syslog message for testing.",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    syslog_cli_cmd_test_msg,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "System Log Configuration",
    NULL,
    "Show system log configuration",
    PRIO_SYSTEM_LOG_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    syslog_cli_cmd_conf_show,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "System Log Server Mode",
    "System Log Server Mode [enable|disable]",
    "Show or set the system log server mode",
    PRIO_SYSTEM_LOG_SERVER_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    syslog_cli_cmd_conf_server_mode,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Log Server Address",
    "System Log Server Address [<ip_addr_string>]",
    "Show or set the system log server address",
    PRIO_SYSTEM_LOG_SERVER_ADDR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    syslog_cli_cmd_conf_server_addr,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Log Level",
    "System Log Level [info|warning|error]",
    "Show or set the system log level.\n"
    "It uses to determine what kind of message will send to syslog server",
    PRIO_SYSTEM_LOG_SERVER_LEVEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    syslog_cli_cmd_conf_level,
    NULL,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Log Lookup [<log_id>] [all|info|warning|error]",
    NULL,
    "Show the system log",
    PRIO_SYSTEM_LOG_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_debug_syslog_ram_nodebug,
    syslog_req_default_set,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "System Log Clear [all|info|warning|error]",
    "Clear the system log",
    PRIO_SYSTEM_LOG_CLEAR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    syslog_cli_cmd_clear,
    syslog_req_default_set,
    syslog_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
