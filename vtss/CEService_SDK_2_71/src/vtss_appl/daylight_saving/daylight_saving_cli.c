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
#include "vtss_module_id.h"
#include "daylight_saving_api.h"

#include "cli_trace_def.h"

typedef struct {
    long                val;
    time_dst_cfg_t      time;

    BOOL                disable;
    BOOL                recurring;
    BOOL                non_recurring;

    BOOL                start;
    BOOL                end;
} time_cli_req_t;

/* ==================================================================*/
/* parse functions */
/* ==================================================================*/

static int32_t cli_time_tz_offset_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_long(cmd, &time_req->val, -7200, 7201);

    return error;
}

static int32_t cli_time_tz_acronym_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, 17);

    return error;
}

static int32_t cli_time_dst_mode_keyword_parse(char *cmd, char *cmd2, char *stx,
                                               char *cmd_org, cli_req_t *req)
{
    time_cli_req_t *time_req;
    char *found = cli_parse_find(cmd, stx);

    time_req = req->module_req;
    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "disable", 7)) {
            time_req->disable = 1;
        } else if (!strncmp(found, "recurring", 9)) {
            time_req->recurring = 1;
        } else if (!strncmp(found, "non-recurring", 13)) {
            time_req->non_recurring = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_time_dst_time_parse_week(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 5);

    if (!error) {
        time_req->time.week = value;
    }

    return (error);
}

static int32_t cli_time_dst_time_parse_day(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 7);

    if (!error) {
        time_req->time.day = value;
    }

    return (error);
}

static int32_t cli_time_dst_time_parse_month(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 12);

    if (!error) {
        time_req->time.month = value;
    }

    return (error);
}

static int32_t cli_time_dst_time_parse_date(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 31);

    if (!error) {
        time_req->time.date = value;
    }

    return (error);
}


static int32_t cli_time_dst_time_parse_year(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 2000, 2097);

    if (!error) {
        time_req->time.year = value;
    }

    return (error);
}

static int32_t cli_time_dst_time_parse_hour(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 23);

    if (!error) {
        time_req->time.hour = value;
    }

    return (error);
}

static int32_t cli_time_dst_time_parse_minute(char *cmd, char *cmd2, char *stx,
                                              char *cmd_org, cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, 59);

    if (!error) {
        time_req->time.minute = value;
    }

    return (error);
}

static int32_t cli_time_dst_offset_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    time_cli_req_t *time_req;

    time_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_long(cmd, &time_req->val, 1, 1440);

    return error;
}

/* ==================================================================*/
/* CLI command table entries */
/* ==================================================================*/

static void cli_cmd_time_tz(cli_req_t *req, BOOL tz_offset, BOOL acronym)
{
    time_conf_t conf;
    time_cli_req_t *time_req;

    if (time_dst_get_config(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {

        if (tz_offset) {
            time_req = (time_cli_req_t *)req->module_req;
            conf.tz = time_req->val;
            conf.tz_offset = time_req->val / 10;
        }
        if (acronym) {
            strcpy(conf.tz_acronym, req->parm);
        }

        (void) time_dst_set_config(&conf);
    } else {

        if (tz_offset) {
            CPRINTF("Timezone Offset : %d ( %d minutes)\n", conf.tz, conf.tz_offset);
        }
        if (acronym) {
            CPRINTF("Timezone Acronym : %s\n", conf.tz_acronym);
        }
    }
}

static void cli_cmd_time_dst(cli_req_t *req, BOOL mode, BOOL start_end, BOOL dst_offset)
{
    time_conf_t conf;
    time_cli_req_t *time_req;

    if (time_dst_get_config(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {

        if (mode) {
            time_req = (time_cli_req_t *)req->module_req;
            if (time_req->disable) {
                conf.dst_mode = TIME_DST_DISABLED;
            } else if (time_req->recurring) {
                conf.dst_mode = TIME_DST_RECURRING;
            } else if (time_req->non_recurring) {
                conf.dst_mode = TIME_DST_NON_RECURRING;
            }
        }

        if (start_end) {
            T_N("enter");
            time_req = (time_cli_req_t *)req->module_req;
            if (time_req->start) {
                conf.dst_start_time.week = time_req->time.week;
                conf.dst_start_time.day = time_req->time.day;
                conf.dst_start_time.month = time_req->time.month;
                conf.dst_start_time.date = time_req->time.date;
                conf.dst_start_time.year = time_req->time.year;
                conf.dst_start_time.hour = time_req->time.hour;
                conf.dst_start_time.minute = time_req->time.minute;
                T_N("start : %d, %d, %d, %d, %d, %d, %d", time_req->time.week, time_req->time.day, time_req->time.month, time_req->time.date, time_req->time.year, time_req->time.hour, time_req->time.minute);
            } else if (time_req->end) {
                conf.dst_end_time.week = time_req->time.week;
                conf.dst_end_time.day = time_req->time.day;
                conf.dst_end_time.month = time_req->time.month;
                conf.dst_end_time.date = time_req->time.date;
                conf.dst_end_time.year = time_req->time.year;
                conf.dst_end_time.hour = time_req->time.hour;
                conf.dst_end_time.minute = time_req->time.minute;
                T_N("end : %d, %d, %d, %d, %d, %d, %d", time_req->time.week, time_req->time.day, time_req->time.month, time_req->time.date, time_req->time.year, time_req->time.hour, time_req->time.minute);
            }
        }

        if (dst_offset) {
            time_req = (time_cli_req_t *)req->module_req;
            conf.dst_offset = time_req->val;
        }

        (void) time_dst_set_config(&conf);
    } else {

        if (mode) {
            CPRINTF("Daylight Saving Time Mode : ");

            switch (conf.dst_mode) {
            case TIME_DST_DISABLED:
                CPRINTF("Disabled.\n");
                break;
            case TIME_DST_RECURRING:
                CPRINTF("Recurring.\n");
                break;
            case TIME_DST_NON_RECURRING:
                CPRINTF("Non-Recurring.\n");
                break;
            }
        }

        if (start_end) {
            switch (conf.dst_mode) {
            case TIME_DST_DISABLED:
                CPRINTF("Daylight Saving Time Start Time Settings : \n");
                CPRINTF("        Week: %d\n", conf.dst_start_time.week);
                CPRINTF("        Day: %d\n", conf.dst_start_time.day);
                CPRINTF("        Month: %d\n", conf.dst_start_time.month);
                CPRINTF("        Date: %d\n", conf.dst_start_time.date);
                CPRINTF("        Year: %d\n", conf.dst_start_time.year);
                CPRINTF("        Hour: %d\n", conf.dst_start_time.hour);
                CPRINTF("        Minute: %d\n", conf.dst_start_time.minute);
                CPRINTF("Daylight Saving Time End Time Settings : \n");
                CPRINTF("        Week: %d\n", conf.dst_end_time.week);
                CPRINTF("        Day: %d\n", conf.dst_end_time.day);
                CPRINTF("        Month: %d\n", conf.dst_end_time.month);
                CPRINTF("        Date: %d\n", conf.dst_end_time.date);
                CPRINTF("        Year: %d\n", conf.dst_end_time.year);
                CPRINTF("        Hour: %d\n", conf.dst_end_time.hour);
                CPRINTF("        Minute: %d\n", conf.dst_end_time.minute);
                break;
            case TIME_DST_RECURRING:
                CPRINTF("Daylight Saving Time Start Time Settings : \n");
                CPRINTF("      * Week: %d\n", conf.dst_start_time.week);
                CPRINTF("      * Day: %d\n", conf.dst_start_time.day);
                CPRINTF("      * Month: %d\n", conf.dst_start_time.month);
                CPRINTF("        Date: %d\n", conf.dst_start_time.date);
                CPRINTF("        Year: %d\n", conf.dst_start_time.year);
                CPRINTF("      * Hour: %d\n", conf.dst_start_time.hour);
                CPRINTF("      * Minute: %d\n", conf.dst_start_time.minute);
                CPRINTF("Daylight Saving Time End Time Settings : \n");
                CPRINTF("      * Week: %d\n", conf.dst_end_time.week);
                CPRINTF("      * Day: %d\n", conf.dst_end_time.day);
                CPRINTF("      * Month: %d\n", conf.dst_end_time.month);
                CPRINTF("        Date: %d\n", conf.dst_end_time.date);
                CPRINTF("        Year: %d\n", conf.dst_end_time.year);
                CPRINTF("      * Hour: %d\n", conf.dst_end_time.hour);
                CPRINTF("      * Minute: %d\n", conf.dst_end_time.minute);
                break;
            case TIME_DST_NON_RECURRING:
                CPRINTF("Daylight Saving Time Start Time Settings : \n");
                CPRINTF("        Week: %d\n", conf.dst_start_time.week);
                CPRINTF("        Day: %d\n", conf.dst_start_time.day);
                CPRINTF("      * Month: %d\n", conf.dst_start_time.month);
                CPRINTF("      * Date: %d\n", conf.dst_start_time.date);
                CPRINTF("      * Year: %d\n", conf.dst_start_time.year);
                CPRINTF("      * Hour: %d\n", conf.dst_start_time.hour);
                CPRINTF("      * Minute: %d\n", conf.dst_start_time.minute);
                CPRINTF("Daylight Saving Time End Time Settings : \n");
                CPRINTF("        Week: %d\n", conf.dst_end_time.week);
                CPRINTF("        Day: %d\n", conf.dst_end_time.day);
                CPRINTF("      * Month: %d\n", conf.dst_end_time.month);
                CPRINTF("      * Date: %d\n", conf.dst_end_time.date);
                CPRINTF("      * Year: %d\n", conf.dst_end_time.year);
                CPRINTF("      * Hour: %d\n", conf.dst_end_time.hour);
                CPRINTF("      * Minute: %d\n", conf.dst_end_time.minute);
                break;
            }
        }

        if (dst_offset) {
            CPRINTF("Daylight Saving Time Offset : %u (minutes)\n", conf.dst_offset);
        }

        if (start_end) {
            CPRINTF("\n * : This symbol indicates the parameter needs to be set the reasonable value.\n");
        }
    }
}

static void cli_cmd_time_tz_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("System Timezone Configuration", 1);
    }
    cli_cmd_time_tz(req, 1, 1);
}

static void cli_cmd_time_tz_offset ( cli_req_t *req )
{
    cli_cmd_time_tz(req, 1, 0);
}

static void cli_cmd_time_tz_acronym ( cli_req_t *req )
{
    cli_cmd_time_tz(req, 0, 1);
}

static void cli_cmd_time_dst_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("System Daylight Saving Time(DST) Configuration", 1);
    }
    cli_cmd_time_dst(req, 1, 1, 1);
}

static void cli_cmd_time_dst_mode ( cli_req_t *req )
{
    cli_cmd_time_dst(req, 1, 0, 0);
}

static void cli_cmd_time_dst_start ( cli_req_t *req )
{
    time_cli_req_t    *time_req = NULL;

    time_req = req->module_req;
    time_req->start = 1;

    cli_cmd_time_dst(req, 0, 1, 0);
}

static void cli_cmd_time_dst_end ( cli_req_t *req )
{
    time_cli_req_t    *time_req = NULL;

    time_req = req->module_req;
    time_req->end = 1;

    cli_cmd_time_dst(req, 0, 1, 0);
}

static void cli_cmd_time_dst_offset ( cli_req_t *req )
{
    cli_cmd_time_dst(req, 0, 0, 1);
}

/* ==================================================================*/
/* Parameter table entries */
/* ==================================================================*/
static cli_parm_t time_cli_parm_table[] = {
    {
        "<offset>",
        "Time zone offset in minutes (-7200 to 7201) relative to UTC",
        CLI_PARM_FLAG_SET,
        cli_time_tz_offset_parse,
        NULL
    },
    {
        "<acronym>",
        "Time zone acronym ( 0 - 16 characters )",
        CLI_PARM_FLAG_SET,
        cli_time_tz_acronym_parse,
        NULL
    },
    {
        "disable|recurring|non-recurring",
        "disable: Disable Daylight Saving Time\n"
        "recurring : Enable Daylight Saving Time as recurring mode\n"
        "non-recurring : Enable Daylight Saving Time as non-recurring mode\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_time_dst_mode_keyword_parse,
        cli_cmd_time_dst_mode,
    },
    {
        "<week>",
        "Week (1-5), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_week,
        cli_cmd_time_dst_start,
    },
    {
        "<day>",
        "Day (1-7), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_day,
        cli_cmd_time_dst_start,
    },
    {
        "<month>",
        "Month (1-12), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_month,
        cli_cmd_time_dst_start,
    },
    {
        "<date>",
        "Date (1-31), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_date,
        cli_cmd_time_dst_start,
    },
    {
        "<year>",
        "Year (2000-2097)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_year,
        cli_cmd_time_dst_start,
    },
    {
        "<hour>",
        "Hour (0-23)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_hour,
        cli_cmd_time_dst_start,
    },
    {
        "<minute>",
        "Minutes (0-59)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_minute,
        cli_cmd_time_dst_start,
    },
    {
        "<week>",
        "Week (1-5), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_week,
        cli_cmd_time_dst_end,
    },
    {
        "<day>",
        "Day (1-7), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_day,
        cli_cmd_time_dst_end,
    },
    {
        "<month>",
        "Month (1-12), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_month,
        cli_cmd_time_dst_end,
    },
    {
        "<date>",
        "Date (1-31), 0: ignored",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_date,
        cli_cmd_time_dst_end,
    },
    {
        "<year>",
        "Year (2000-2097)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_year,
        cli_cmd_time_dst_end,
    },
    {
        "<hour>",
        "Hour (0-23)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_hour,
        cli_cmd_time_dst_end,
    },
    {
        "<minute>",
        "Minutes (0-59)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_time_parse_minute,
        cli_cmd_time_dst_end,
    },
    {
        "<dst_offset>",
        "DST offset in minutes (1 to 1440)",
        CLI_PARM_FLAG_SET,
        cli_time_dst_offset_parse,
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

enum {
    PRIO_TIME_TZ_CONF = 0,
    PRIO_TIME_TZ,
    PRIO_TIME_TZ_ACRONYM,
    PRIO_TIME_DST_CONF,
    PRIO_TIME_DST_MODE,
    PRIO_TIME_DST_START,
    PRIO_TIME_DST_END,
    PRIO_TIME_DST_OFFSET,
};

/* ==================================================================*/
/* Command table entries */
/* ==================================================================*/
/* System Timezone Configuration */
cli_cmd_tab_entry (
    "System Timezone Configuration",
    NULL,
    "Show System Timezone configuration",
    PRIO_TIME_TZ_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_tz_config,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

/* System Timezone offset [<offset>] */
cli_cmd_tab_entry (
    "System Timezone Offset",
    "System Timezone Offset [<offset>]",
    "Set or show the system timezone offset",
    PRIO_TIME_TZ,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_tz_offset,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* System Timezone Acronym [<acronym>] */
cli_cmd_tab_entry (
    "System Timezone Acronym",
    "System Timezone Acronym [<acronym>]",
    "Set or show the system timezone acronym",
    PRIO_TIME_TZ_ACRONYM,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_tz_acronym,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* System DST Configuration */
cli_cmd_tab_entry (
    "System DST Configuration",
    NULL,
    "Show Daylight Saving Time configuration",
    PRIO_TIME_DST_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_dst_config,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

/* System DST mode [disable|recurring|non-recurring] */
cli_cmd_tab_entry (
    "System DST Mode",
    "System DST Mode [disable|recurring|non-recurring]",
    "Set or show the daylight saving time mode",
    PRIO_TIME_DST_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_dst_mode,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* System DST start <week> <day> <month> <date> <year> <hour> <minute> */
cli_cmd_tab_entry (
    NULL,
    "System DST start <week> <day> <month> <date> <year> <hour> <minute>",
    "start: Set or show the daylight saving time start time settings\n",
    PRIO_TIME_DST_START,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_dst_start,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* System DST end <week> <day> <month> <date> <year> <hour> <minute> */
cli_cmd_tab_entry (
    NULL,
    "System DST end <week> <day> <month> <date> <year> <hour> <minute>",
    "end: Set or show the daylight saving time end time settings\n",
    PRIO_TIME_DST_END,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_dst_end,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/* System DST offset [<offset>] */
cli_cmd_tab_entry (
    "System DST Offset",
    "System DST Offset [<dst_offset>]",
    "Set or show the daylight saving time offset",
    PRIO_TIME_DST_OFFSET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SYSTEM,
    cli_cmd_time_dst_offset,
    NULL,
    time_cli_parm_table,
    CLI_CMD_FLAG_NONE
);