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

// Avoid "cli_grp_help.h not used in module system_cli.c"
/*lint --e{766} */

#include "main.h"
#include "cli.h"
#include "cli_grp_help.h"
#include "cli_api.h"
#include "conf_api.h"
#include "msg_api.h"
#include "vtss_api_if_api.h" /* For vtss_api_chipid() */
#include "sys/time.h"        /* For struct timeval */
#include "misc_api.h"        /* For misc_strncpyz() */
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif
#include "system_cli.h"
#include "vtss_module_id.h"
#include "system_cli.h"
#include "control_api.h"
#include "cli_trace_def.h"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#include "vtss_eth_link_oam_api.h"
#endif

#ifdef VTSS_CLI_SEC_GRP
#define GRP_CLI_PATH    VTSS_CLI_GRP_SEC_SWITCH_PATH
#else
#define GRP_CLI_PATH    "System "
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SYSTEM

typedef struct {
    long    val;
#ifdef VTSS_SW_OPTION_SYSLOG
    syslog_lvl_t syslog_lvl;
#endif
    ulong   log_min;
    ulong   log_max;

    struct tm tm;

    /* Keywords */
    BOOL    keep_ip;
    vtss_restart_t restart;
} system_cli_req_t;

void system_cli_req_init(void)
{
    /* register the size required for system req. structure */
    cli_req_size_register(sizeof(system_cli_req_t));
}
static void cli_cmd_system(cli_req_t *req, BOOL contact, BOOL name, BOOL location, BOOL password, BOOL timezone)
{
    system_conf_t conf;
#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
    system_cli_req_t *system_req;
#endif

    if (system_get_config(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (contact) {
            misc_strncpyz(conf.sys_contact, req->parm, VTSS_SYS_STRING_LEN);
        }
        if (name) {
            misc_strncpyz(conf.sys_name, req->parm, VTSS_SYS_STRING_LEN);
        }
        if (location) {
            misc_strncpyz(conf.sys_location, req->parm, VTSS_SYS_STRING_LEN);
        }
#ifndef VTSS_SW_OPTION_USERS
        if (password) {
             misc_strncpyz(conf.sys_passwd, req->parm, VTSS_SYS_PASSWD_LEN);
        }
#endif /* VTSS_SW_OPTION_USERS */

#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
        if (timezone) {
            system_req = (system_cli_req_t *)req->module_req;
            conf.tz_off = system_req->val;
        }
#endif
        (void) system_set_config(&conf);
    } else {
        if (contact) {
            CPRINTF("System Contact  : %s\n", conf.sys_contact);
        }
        if (name) {
            CPRINTF("System Name     : %s\n", conf.sys_name);
        }
        if (location) {
            CPRINTF("System Location : %s\n", conf.sys_location);
        }
        /* We should not show the password for security considered
        if (password)
            CPRINTF("System Password : %s\n", conf.sys_passwd); */
#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
        if (timezone) {
            CPRINTF("Timezone Offset : %d\n", conf.tz_off);
        }
#endif
    }
}

static void cli_cmd_system_restart_show(BOOL debug)
{
    vtss_restart_status_t status;
    char                  buf[32];

    if (vtss_restart_status_get(NULL, &status) == VTSS_RC_OK) {
        strcpy(buf, control_system_restart_to_str(status.restart));
        buf[0] = toupper(buf[0]);
        CPRINTF("Previous Restart: %s\n", buf);
        if (debug) {
            CPRINTF("Current Version : %u\n", status.cur_version);
            CPRINTF("Previous Version: %u\n", status.prev_version);
        }
    }
}

/* System version */
static void cli_cmd_system_version(cli_req_t *req)
{
    const char *code_rev = misc_software_code_revision_txt();
    CPRINTF("Version      : %s\n", misc_software_version_txt());
    CPRINTF("Build Date   : %s\n", misc_software_date_txt());
    if (strlen(code_rev)) {
        // version.c is always compiled, this file is not, so we must
        // check for whether there's something in the code revision
        // string or not. Only version.c knows about the CODE_REVISION
        // environment variable.
        CPRINTF("Code Revision: %s\n", code_rev);
    }
}

/* System configuration */
static void cli_cmd_system_conf(cli_req_t *req)
{
    char     buf[MSG_MAX_VERSION_STRING_LEN];
    uchar    mac[6];

    if (req->all) {
        cli_header("System Configuration", 1);
    }
    cli_cmd_system(req, 1, 1, 1, 1, 1);
    if (conf_mgmt_mac_addr_get(mac, 0) >= 0) {
        CPRINTF("MAC Address     : %s\n", misc_mac_txt(mac, buf));
    }
#if VTSS_SWITCH_STACKABLE
    if (!req->stack.master)
#endif
    {
        // This is displayed by cli_system_conf_disp() for stackable switches that are masters.
        CPRINTF("Chip ID         : VSC%x\n", vtss_api_chipid());
    }
    CPRINTF("System Time     : %s\n", misc_time2str(time(NULL)));
    CPRINTF("System Uptime   : %s\n", cli_time_txt(cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR));
#if VTSS_SWITCH_STACKABLE
    if (!req->stack.master)
#endif
    {
        // This is displayed by cli_system_conf_disp() for stackable switches that are masters.
        CPRINTF("Software Version: %s\n", misc_software_version_txt());
    }
    CPRINTF("Software Date   : %s\n", misc_software_date_txt());
    cli_cmd_system_restart_show(0);

    /* Display individual module configuration */
    cli_system_conf_disp(req);
}

static void cli_cmd_do_reboot(cli_req_t *req)
{
    system_cli_req_t *system_req = req->module_req;
    int rc;

    if (!req->set) {
        cli_cmd_system_restart_show(1);
        return;
    }

#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    /* This notification allows Link OAM to send out the dyning gasp events */
    /* This action handler makes sure that after PDU exit only the sys reboot happens */
    vtss_eth_link_oam_mgmt_sys_reboot_action_handler();
#endif

    if (vtss_switch_mgd() && vtss_switch_stackable() && msg_switch_is_master() && vtss_stacking_enabled()) {
        if (req->usid_sel == VTSS_USID_ALL) {
            CPRINTF("Rebooting all switches in the stack\n");
        } else {
            CPRINTF("Rebooting switch %d\n", req->usid_sel);
        }
        rc = control_system_reset(FALSE, req->usid_sel, system_req->restart);
    } else {
        CPRINTF("System will reboot in a few seconds\n");
        rc = control_system_reset(TRUE, VTSS_USID_ALL, system_req->restart);
    }

    if (rc) {
        CPRINTF("Restart failed! System is updating by another process.\n");
    }
}

static void cli_cmd_system_reboot(cli_req_t *req)
{
    system_cli_req_t *system_req  = req->module_req;

    system_req->restart = VTSS_RESTART_COOL;






    req->set = 1;
    cli_cmd_do_reboot(req);
}

static void cli_cmd_system_contact ( cli_req_t *req )
{
    cli_cmd_system(req, 1, 0, 0, 0, 0);
}

static void cli_cmd_system_name ( cli_req_t *req )
{
    cli_cmd_system(req, 0, 1, 0, 0, 0);
}

static void cli_cmd_system_location ( cli_req_t *req )
{
    cli_cmd_system(req, 0, 0, 1, 0, 0);
}

static void cli_cmd_system_password ( cli_req_t *req )
{
    cli_cmd_system(req, 0, 0, 0, 1, 0);
}

#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
static void cli_cmd_system_tz ( cli_req_t *req )
{
    cli_cmd_system(req, 0, 0, 0, 0, 1);
}
#endif

static void system_cmd_system_restore_default(cli_req_t *req )
{
    system_cli_req_t *system_req;

    system_req = req->module_req;
    ulong flags = system_req->keep_ip ? INIT_CMD_PARM2_FLAGS_IP : 0;
    if (vtss_switch_stackable()) {
        /* Non-local operation */
        control_config_reset(req->usid_sel, flags);
    } else {
        /* Local operation only */
        control_config_reset(VTSS_USID_ALL, flags);
    }
}

#ifdef VTSS_SW_OPTION_SYSLOG
void system_cmd_syslog_ram ( cli_req_t *req )
{

    vtss_usid_t        usid;
    vtss_isid_t        isid;
    syslog_ram_entry_t *entry;
    syslog_ram_stat_t  stat;
    ulong              total;
    syslog_lvl_t       lvl;
    int                i;
    BOOL               first;
    BOOL               debug = 0;
    system_cli_req_t    *sys_req = NULL;

    if ((entry = VTSS_MALLOC(sizeof(*entry))) == NULL) {
        return;
    }

    sys_req = req->module_req;

    /* Log all entries by default */
    if (sys_req->log_min == 0) {
        sys_req->log_min = 1;
    }
    if (sys_req->log_max == 0) {
        sys_req->log_max = 0xffffffff;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        isid = (debug ? (usid == VTSS_USID_START ? VTSS_ISID_LOCAL : VTSS_ISID_END) :
                    req->stack.isid[usid]);
        if (isid == VTSS_ISID_END) {
            continue;
        }

        if (req->clear) {
            syslog_ram_clear(isid, SYSLOG_LVL_ALL);
            continue;
        }

        for (entry->id = (sys_req->log_min - 1), first = 1;
                syslog_ram_get(isid, 1, entry->id, sys_req->syslog_lvl, VTSS_MODULE_ID_NONE, entry) &&
                entry->id <= sys_req->log_max; ) {
            if (first && !debug) {
                cli_cmd_usid_print(usid, req, 0);
            }

            if (sys_req->log_min == sys_req->log_max) {
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
                for (i = 0; i < 80; i++)
                    if (entry->msg[i] == '\n') {
                        entry->msg[i] = '\0';
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

#endif /* VTSS_SW_OPTION_SYSLOG */

static void cli_cmd_dbg_time(cli_req_t *req)
{
    system_cli_req_t *system_req = req->module_req;

    if (req->set) {
        struct timeval tv;
        time_t secs = mktime(&system_req->tm);
        // Adjust for timezone (system_get_tz_off() returns number of minutes).
        secs -= (system_get_tz_off() * 60);
        tv.tv_sec  = secs;
        tv.tv_usec = 0;
        (void)settimeofday(&tv, NULL);
    } else {
        CPRINTF("%s\n", misc_time2str(time(NULL)));
    }
}

static int32_t cli_system_generic_keyword_parse(char *cmd, cli_req_t *req, char *stx)
{
    char *found = cli_parse_find(cmd, stx);
    system_cli_req_t *system_req;

    T_I("ALL %s", found);

    system_req = req->module_req;
    if (found != NULL) {
        if (!strncmp(found, "keep_ip", 7)) {
            system_req->keep_ip = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_system_keep_ip_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_system_generic_keyword_parse(cmd, req, stx);

    return error;
}

static int32_t cli_system_contact_location_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    if (strncmp(cmd_org, "clear", 5)) {
        error = cli_parse_text(cmd_org, req->parm, VTSS_SYS_STRING_LEN);
    } else {
        strcpy(req->parm, cmd_org);
    }

    return error;
}

static int32_t cli_system_name_parse(char *cmd, char *cmd2, char *stx,
                                     char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    if (strncmp(cmd_org, "clear", 5)) {
        error = cli_parse_text(cmd_org, req->parm, VTSS_SYS_STRING_LEN);
    } else {
        strcpy(req->parm, cmd_org);
    }
    if (!error && strcmp(cmd_org, "\"\"") && !system_name_is_administratively(cmd_org)) {
        return 1;
    }
    return error;
}

static int32_t cli_system_password_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, VTSS_SYS_PASSWD_LEN);

    return error;
}

static int32_t cli_system_offset_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    system_cli_req_t *system_req;

    system_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_long(cmd, &system_req->val, -720, 720);

    return error;
}

static int32_t cli_system_dbg_reboot_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char             *found      = cli_parse_find(cmd, stx);
    system_cli_req_t *system_req = req->module_req;
    req->parm_parsed = 1;

    if (!found) {
        return 1;
    } else if (!strncmp(found, "cold", 4)) {
        system_req->restart = VTSS_RESTART_COLD;
    } else  if (!strncmp(found, "cool", 4)) {
        system_req->restart = VTSS_RESTART_COOL;
    } else  if (!strncmp(found, "warm", 4)) {
        system_req->restart = VTSS_RESTART_WARM;
    } else {
        return 1;
    }

    return 0;
}

static int32_t cli_system_date_time_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    system_cli_req_t *system_req = req->module_req;
    char *res;

    // strptime() returns NULL if the input was erroneous, otherwise a pointer to the
    // last character parsed, so we check that this last char points to the NULL-terminating byte,
    // and if not, the user input more date than required for the format.
    if ((res = strptime(cmd_org, "%Y-%m-%dT%H:%M:%S", &system_req->tm)) == NULL || *res != '\0') {
        return 1;
    }

    return 0;
}

static cli_parm_t system_cli_parm_table[] = {
    {
        "all",
        "Show all switch configuration, default: Show system configuration",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_system_conf
    },
    {
        "port",
        "Show switch port configuration",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_system_conf
    },
    {
        "keep_ip",
        "Keep IP configuration, default: Restore full configuration",
        CLI_PARM_FLAG_NONE,
        cli_system_keep_ip_parse,
        NULL
    },
    {
        "<contact>",
        "System contact string. (1-"vtss_xstr(VTSS_SYS_INPUT_STRING_LEN)")\n"
        "           Use \"\" to clear the string\n"
        "           In CLI, No blank or space characters are permitted as part of a\n"
        "           contact.",
        CLI_PARM_FLAG_SET,
        cli_system_contact_location_parse,
        cli_cmd_system_contact
    },
    {
        "<name>",
        "System name string. (1-"vtss_xstr(VTSS_SYS_INPUT_STRING_LEN)")\n"
        "        Use \"\" to clear the string\n"
        "        System name is a text string drawn from the alphabet (A-Za-z),\n"
        "        digits (0-9), minus sign (-).\n"
        "        No blank or space characters are permitted as part of a name.\n"
        "        The first character must be an alpha character, and the first or\n"
        "        last character must not be a minus sign.",
        CLI_PARM_FLAG_SET,
        cli_system_name_parse,
        NULL,
    },
    {
        "<location>",
        "System location string. (1-"vtss_xstr(VTSS_SYS_INPUT_STRING_LEN)")\n"
        "            Use \"\" to clear the string\n"
        "            In CLI, no blank or space characters are permitted as part of a\n"
        "            location.",
        CLI_PARM_FLAG_SET,
        cli_system_contact_location_parse,
        cli_cmd_system_location
    },
    {
        "<password>",
        "System password string.Use 'clear' or \"\" to clear the string",
        CLI_PARM_FLAG_SET,
        cli_system_password_parse,
        cli_cmd_system_password
    },
    {
        "<password>",
        "The password for this user name. Use 'clear' or \"\" as null string",
        CLI_PARM_FLAG_NONE,
        cli_system_password_parse,
        NULL
    },
    {
        "<offset>",
        "Time zone offset in minutes (-720 to 720) relative to UTC",
        CLI_PARM_FLAG_SET,
        cli_system_offset_parse,
        NULL
    },
    {






        "cold|cool",
        "cold: Perform a cold reboot\n"
        "cool: Perform a cool reboot\n"

        "(default: Show previous boot type)",
        CLI_PARM_FLAG_SET | CLI_PARM_FLAG_NO_TXT,
        cli_system_dbg_reboot_parse,
        cli_cmd_do_reboot
    },









    {
        "<date_time>",
        "Date and time using following syntax: YYYY-MM-DDTHH:MM:SS (example: 2011-03-17T15:44:23)",
        CLI_PARM_FLAG_SET,
        cli_system_date_time_parse,
        NULL
    },
    {
        "clear",
        "Clear system name",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_system_name
    },
    {
        "clear",
        "Clear system location",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_system_location
    },
    {
        "clear",
        "Clear system contact",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_system_contact
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

enum {
    PRIO_SYSTEM_CONF = 0,
    PRIO_SYSTEM_NAME,
    PRIO_SYSTEM_CONTACT,
    PRIO_SYSTEM_LOCATION,
    PRIO_SYSTEM_TZ,
    PRIO_SYSTEM_PASSWORD,
    PRIO_SYSTEM_REBOOT,
    PRIO_SYSTEM_DEF,
    PRIO_SYSTEM_DBG_REBOOT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_SYSTEM_DBG_TIME   = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
    "System Configuration [all | (port <port_list>)]",
    NULL,
    "Show system configuration",
    PRIO_SYSTEM_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_conf,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "System Version",
    NULL,
    "Show system version information",
    PRIO_SYSTEM_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_version,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* Reboot commands, depending on warm start option */




#define SYSTEM_REBOOT_CMD       "System Reboot"
#define SYSTEM_DEBUG_REBOOT_CMD "Debug System Reboot [cold|cool]"


/* Command table entries */
cli_cmd_tab_entry (
    NULL,
    SYSTEM_REBOOT_CMD,
    "Reboot the system",
    PRIO_SYSTEM_REBOOT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_system_reboot,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "System Restore Default [keep_ip]",
    "Restore factory default configuration",
    PRIO_SYSTEM_DEF,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    system_cmd_system_restore_default,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Contact",
    "System Contact [<contact>]",
    "Set or show the system contact",
    PRIO_SYSTEM_CONTACT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_contact,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Name",
    "System Name [<name>]",
    "Set or show the system name",
    PRIO_SYSTEM_NAME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_name,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "System Location",
    "System Location [<location>]",
    "Set or show the system location",
    PRIO_SYSTEM_LOCATION,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_location,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


#ifndef VTSS_SW_OPTION_USERS
cli_cmd_tab_entry (
    NULL,
    GRP_CLI_PATH "Password <password>",
    "Set the system password",
    PRIO_SYSTEM_PASSWORD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_system_password,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
cli_cmd_tab_entry (
    "System Timezone",
    "System Timezone [<offset>]",
    "Set or show the system timezone offset",
    PRIO_SYSTEM_TZ,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_system_tz,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    SYSTEM_DEBUG_REBOOT_CMD,
    NULL,
    "Show how the system was rebooted last time or reboot with a given method",
    PRIO_SYSTEM_DBG_REBOOT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_do_reboot,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Time",
    "Debug Time [<date_time>]",
    "Set the current date and time (non-persistent)",
    PRIO_SYSTEM_DBG_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_dbg_time,
    NULL,
    system_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


