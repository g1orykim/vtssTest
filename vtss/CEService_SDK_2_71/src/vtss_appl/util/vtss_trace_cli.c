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

#include "main.h"
#include "cli.h"
#include "vtss_trace_cli.h"
#include "vtss_trace_vtss_switch_api.h"
#include "cli_trace_def.h"

typedef struct {
    /* Keywords */
    BOOL    read;
    BOOL    write;
    BOOL    erase;
    BOOL    none;

    int    trace_module_id; /* -1 = All modules                  */
    int    trace_grp_idx;   /* -1 = All groups within module(s)  */
    int    trace_lvl;       /* Trace level (ref trace_lvl_api.h) */
    int    thread_id;       /* -1 = All threads                  */
} trace_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void trace_cli_init(void)
{
    /* register the size required for Trace req. structure */
    cli_req_size_register(sizeof(trace_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* Debug trace port */
static void cli_cmd_debug_trace_module_port(cli_req_t *req)
{
    vtss_uport_no_t uport;
    vtss_port_no_t  iport;
    char            enable_str[20];

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;

        if (req->set) {
            vtss_trace_port_set(iport, !req->enable);
        } else {
            if (iport == 0)
                cli_table_header("Port Trace");
            if (vtss_trace_port_get(iport)) {
                strcpy(enable_str,"Disable");
            } else {
                strcpy(enable_str,"Enable");
            }
            CPRINTF("%-3u %s \n", uport, &enable_str[0]) ;
        }
    }
} /* cli_cmd_debug_trace_module_port */

static void cli_cmd_debug_trace_reverse(cli_req_t *req)
{
    vtss_trace_lvl_reverse();
    vtss_api_trace_update();
    CPRINTF("Previous trace level change reversed\n");
} /* cli_cmd_debug_trace_reverse */

static void cli_cmd_debug_trace_config(cli_req_t *req)
{
    vtss_rc rc;
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->write) {
        CPRINTF("Writing trace settings to flash ...\n");
        rc = vtss_trace_cfg_wr();
        if (rc < 0) CPRINTF("Failed writing trace settings to flash, rc=%d\n", rc);
    } else if (trace_req->read) {
        CPRINTF("Reading trace settings from flash ...\n");
        rc = vtss_trace_cfg_rd();
        if (rc < 0) CPRINTF("Failed reading trace settings from flash\n");
    } else if (trace_req->erase) {
        CPRINTF("Erasing trace settings from flash ...\n");
        rc = vtss_trace_cfg_erase();
        if (rc < 0) CPRINTF("Failed erasing trace settings from flash, rc=%d\n", rc);
    }
} /* cli_cmd_debug_trace_config */

static void cli_cmd_debug_trace_print_info(cli_req_t *req)
{
    /* No level to be set => Print trace information */
    int mid_start,  mid_stop;
    int gidx_start, gidx_stop;
    trace_cli_req_t *trace_req = req->module_req;
    
    int mid=-1, gidx=-1;
    char *module_name;
    char *grp_name;
    
    /* Work-around for problem with printf("%*s", ...) */
    char name_format[10];
    char lvl_format[10];
    sprintf(name_format, "%%-%ds", VTSS_TRACE_MAX_NAME_LEN);
    sprintf(lvl_format, "%%-%ds",  VTSS_TRACE_MAX_LVL_LEN);
    
    if (trace_req->trace_module_id == TRACE_MODULE_ID_UNSPECIFIED) {
        mid_start = -1;
        mid_stop  = -1;
    } else {
        mid_start = MAX(trace_req->trace_module_id-1, -1);
        mid_stop  = trace_req->trace_module_id;
    }
    if (trace_req->trace_grp_idx == TRACE_GRP_IDX_UNSPECIFIED) {
        gidx_start = -1;
        gidx_stop  = -1;
    } else {
        gidx_start = MAX(trace_req->trace_grp_idx-1, -1);
        gidx_stop  = trace_req->trace_grp_idx;
    }
    
    if (vtss_trace_global_lvl_get() != VTSS_TRACE_LVL_RACKET) {
        CPRINTF("Global trace level: %s\n",
                vtss_trace_lvl_to_str(vtss_trace_global_lvl_get()));
        CPRINTF("\n");
    }
    
    // Output format:
    //
    // Module      Group    Level  Timestamp  Usec  Ring Buf  IRQ  Description
    // ----------  -----    -----  ---------  ----  --------  ---  -----------
    // mirror      default         yes        no    no        no   Bla bla bla
    //             g1       error  no         no    yes       yes  didadida
    //             g2       error  no         no    no        no   didadida
    /* Header */
    CPRINTF(name_format, "Module");
    CPRINTF("  ");
    CPRINTF(name_format, "Group");
    CPRINTF("  ");
    CPRINTF(lvl_format, "Level");
    CPRINTF("  ");
    CPRINTF("Timestamp  ");
    CPRINTF("Usec  ");
    CPRINTF("Ring Buf  ");
    CPRINTF("IRQ  ");
    CPRINTF("Description\n");
    
    cprintf_repeat_char('-', VTSS_TRACE_MAX_NAME_LEN);
    CPRINTF("  ");
    cprintf_repeat_char('-', VTSS_TRACE_MAX_NAME_LEN);
    CPRINTF("  ");
    cprintf_repeat_char('-', VTSS_TRACE_MAX_LVL_LEN);
    CPRINTF("  ");
    cprintf_repeat_char('-', strlen("Timestamp"));
    CPRINTF("  ");
    cprintf_repeat_char('-', strlen("Usec"));
    CPRINTF("  ");
    cprintf_repeat_char('-', strlen("Ring Buf"));
    CPRINTF("  ");
    cprintf_repeat_char('-', strlen("IRQ"));
    CPRINTF("  ");
    cprintf_repeat_char('-', strlen("Description"));
    CPRINTF("\n");
    
    /* Modules */
    mid = mid_start;
    while ((module_name = vtss_trace_module_name_get_nxt(&mid))) {
        BOOL one_line_format = 0;
        
        gidx = gidx_start;
        if (strcmp(vtss_trace_grp_name_get_nxt(mid, &gidx), "default") == 0) {
            /* First group to print is named "default"
             * =>
             * print group name on same line as module name and
             * skip printing group description */
            one_line_format = 1;
        }
        
        CPRINTF(name_format, module_name);
        CPRINTF("  ");
        if (!one_line_format) {
            CPRINTF(name_format, "");
            CPRINTF("  ");
            CPRINTF(lvl_format,  "");
            CPRINTF("  ");
            cprintf_repeat_char(' ', strlen("Timestamp"));
            CPRINTF("  ");
            cprintf_repeat_char(' ', strlen("Usec"));
            CPRINTF("  ");
            cprintf_repeat_char(' ', strlen("Ring Buf"));
            CPRINTF("  ");
            cprintf_repeat_char(' ', strlen("IRQ"));
            CPRINTF("  ");
            CPRINTF("%s\n", vtss_trace_module_get_descr(mid));
        }
        
        /* Groups */
        gidx = gidx_start;
        while ((grp_name = vtss_trace_grp_name_get_nxt(mid, &gidx))) {
            if (!one_line_format) {
                CPRINTF(name_format, "");
                CPRINTF("  ");
            }
            CPRINTF(name_format, grp_name);
            CPRINTF("  ");
            CPRINTF(lvl_format, vtss_trace_lvl_to_str(vtss_trace_module_lvl_get(mid, gidx)));
            CPRINTF("  ");
            CPRINTF("%-9s  ", vtss_trace_grp_get_parm(VTSS_TRACE_MODULE_PARM_TIMESTAMP, mid, gidx) ? "yes" : "no");
            CPRINTF("%-4s  ", vtss_trace_grp_get_parm(VTSS_TRACE_MODULE_PARM_USEC, mid, gidx)      ? "yes" : "no");
            CPRINTF("%-8s  ", vtss_trace_grp_get_parm(VTSS_TRACE_MODULE_PARM_RINGBUF, mid, gidx)   ? "yes" : "no");
            CPRINTF("%-3s  ", vtss_trace_grp_get_parm(VTSS_TRACE_MODULE_PARM_IRQ,     mid, gidx)   ? "yes" : "no");
            if (!one_line_format) {
                CPRINTF("%s\n", vtss_trace_grp_get_descr(mid, gidx));
            } else {
                CPRINTF("%s\n", vtss_trace_module_get_descr(mid));
            }
            one_line_format = 0; /* one_line_format only applicable to first group */
            
            if (gidx == gidx_stop) break;
        }
        
        if (mid == mid_stop) break;
    }
}

/* Debug trace level */
static void cli_cmd_debug_trace_module_level(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_module_id != TRACE_MODULE_ID_UNSPECIFIED &&
        trace_req->trace_grp_idx   != TRACE_GRP_IDX_UNSPECIFIED &&
        trace_req->trace_lvl       != VTSS_TRACE_LVL_UNSPECIFIED) {
        vtss_trace_module_lvl_set(trace_req->trace_module_id, trace_req->trace_grp_idx, trace_req->trace_lvl);
        vtss_api_trace_update();
        return;
    }

    cli_cmd_debug_trace_print_info(req);
} /* cli_cmd_debug_trace_module_level */

/* Debug trace timestamp enable/disable */
static void cli_cmd_debug_trace_module_timestamp(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_module_id     != TRACE_MODULE_ID_UNSPECIFIED &&
        trace_req->trace_grp_idx != TRACE_GRP_IDX_UNSPECIFIED &&
        (req->enable || req->disable)) {
        vtss_trace_module_parm_set(
            VTSS_TRACE_MODULE_PARM_TIMESTAMP,
            trace_req->trace_module_id, trace_req->trace_grp_idx, req->enable);
        return;
    }

    cli_cmd_debug_trace_print_info(req);
} /* cli_cmd_debug_trace_module_timestamp */ 

/* Debug trace timestamp enable/disable */
static void cli_cmd_debug_trace_module_usec(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_module_id     != TRACE_MODULE_ID_UNSPECIFIED &&
        trace_req->trace_grp_idx != TRACE_GRP_IDX_UNSPECIFIED &&
        (req->enable || req->disable)) {
        vtss_trace_module_parm_set(
            VTSS_TRACE_MODULE_PARM_USEC,
            trace_req->trace_module_id, trace_req->trace_grp_idx, req->enable);
        return;
    }

    cli_cmd_debug_trace_print_info(req);
} /* cli_cmd_debug_trace_module_usec */ 

/* Ringbuf redirect enable/disable */
static void cli_cmd_debug_trace_module_ringbuf(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_module_id     != TRACE_MODULE_ID_UNSPECIFIED &&
        trace_req->trace_grp_idx != TRACE_GRP_IDX_UNSPECIFIED &&
        (req->enable || req->disable)) {
        vtss_trace_module_parm_set(
            VTSS_TRACE_MODULE_PARM_RINGBUF,
            trace_req->trace_module_id, trace_req->trace_grp_idx, req->enable);
        return;
    }

    cli_cmd_debug_trace_print_info(req);
} /* cli_cmd_debug_trace_module_ringbuf */ 

static void cli_cmd_debug_trace_thread_info(cli_req_t *req)
{
    cyg_handle_t        thread = 0;
    cyg_uint16          id = 0;
    cyg_thread_info     info;
    char                buf[16];
    size_t              len;
    vtss_trace_thread_t trace_thread;
    BOOL                print_extra = FALSE;

    if (vtss_trace_global_lvl_get() != VTSS_TRACE_LVL_RACKET) {
        CPRINTF("Global trace level: %s\n",
                vtss_trace_lvl_to_str(vtss_trace_global_lvl_get()));
        CPRINTF("\n");
    }
    cli_table_header("ID   Name             Level    Stack used");
    cyg_scheduler_lock();
    while (cyg_thread_get_next(&thread, &id) != 0 && cyg_thread_get_info(thread, id, &info)) {
        if (id <= VTSS_TRACE_MAX_THREAD_ID) {
            vtss_trace_thread_get(id, &trace_thread);
            memset(buf, ' ', sizeof(buf));
            len = (info.name == NULL ? 0 : strlen(info.name));
            memcpy(buf, info.name, len > sizeof(buf) ? sizeof(buf) : len);
            buf[sizeof(buf) - 1] = '\0';
            CPRINTF("%-3d  %s  %-7s  %s\n",
                    id,
                    buf,
                    vtss_trace_lvl_to_str(trace_thread.lvl),
                    trace_thread.stackuse ? "Yes" : "No");
        } else {
            print_extra = TRUE;
        }
    }
    cyg_scheduler_unlock();

    if (print_extra) {
        CPRINTF("Threads with ID > %u exist, but these can't be trace controlled\n", VTSS_TRACE_MAX_THREAD_ID);
    }
} /* cli_cmd_debug_trace_thread_info */

static void cli_cmd_debug_trace_thread_level(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->thread_id != THREAD_ID_UNSPECIFIED &&
        trace_req->trace_lvl != VTSS_TRACE_LVL_UNSPECIFIED) {
        vtss_trace_thread_lvl_set(trace_req->thread_id, trace_req->trace_lvl);
        return;
    }

    cli_cmd_debug_trace_thread_info(req);
} /* cli_cmd_debug_trace_thread_level */ 

static void cli_cmd_debug_trace_thread_stackuse(cli_req_t *req)
{
    trace_cli_req_t *trace_req = (trace_cli_req_t*)req->module_req;

    if (trace_req->thread_id != THREAD_ID_UNSPECIFIED &&
        (req->enable || req->disable)) {
        vtss_trace_thread_stackuse_set(trace_req->thread_id, req->enable);
        return;
    }

    cli_cmd_debug_trace_thread_info(req);
} /* cli_cmd_debug_trace_thread_stackuse */


static void cli_cmd_debug_trace_ringbuffer_print(cli_req_t *req)
{
    vtss_trace_rb_output();
} /* cli_cmd_debug_trace_ringbuffer_print */


static void cli_cmd_debug_trace_ringbuffer_flush(cli_req_t *req)
{
    vtss_trace_rb_flush();
} /* cli_cmd_debug_trace_ringbuffer_flush */


static void cli_cmd_debug_trace_ringbuffer_start(cli_req_t *req)
{
    vtss_trace_rb_ena(1);
} /* cli_cmd_debug_trace_ringbuffer_start */


static void cli_cmd_debug_trace_ringbuffer_stop(cli_req_t *req)
{
    vtss_trace_rb_ena(0);
} /* cli_cmd_debug_trace_ringbuffer_stop */


static void cli_cmd_debug_trace_global_level(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_lvl != VTSS_TRACE_LVL_UNSPECIFIED) {
        vtss_trace_global_lvl_set(trace_req->trace_lvl);
        vtss_api_trace_update();
        return;
    }

    CPRINTF("Global trace level: %s\n",
            vtss_trace_lvl_to_str(vtss_trace_global_lvl_get()));
} /* cli_cmd_debug_trace_global_level */


/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static void cli_cmd_trace_default_func(cli_req_t *req)
{
    trace_cli_req_t *trace_req = req->module_req;

    trace_req->trace_module_id     = TRACE_MODULE_ID_UNSPECIFIED;
    trace_req->trace_grp_idx = TRACE_GRP_IDX_UNSPECIFIED;
    trace_req->trace_lvl     = VTSS_TRACE_LVL_UNSPECIFIED;
    trace_req->thread_id     = THREAD_ID_UNSPECIFIED;
}/*cli_cmd_trace_default_func*/


static int cli_parse_trace_module(char *cmd, cli_req_t *req)
{
    int           error = 1;
    trace_cli_req_t *trace_req = req->module_req;

    if (vtss_trace_module_to_val(cmd, &trace_req->trace_module_id)) {
        error = 0;
    } else if ((cmd[0] == '*' || cmd[0] == '.') && strlen(cmd) == 1) {
        /* Wildcard */
        trace_req->trace_module_id = -1;
        error = 0;
    }

    return error;
}

static int cli_parse_trace_group(char *cmd, cli_req_t *req)
{
    int error = 1;
    trace_cli_req_t *trace_req = req->module_req;

    if (trace_req->trace_module_id == -1) {
        /* trace_module_id was wildcarded => wildcard group as well */
        trace_req->trace_grp_idx = -1;
        error = 0;
    } else if (vtss_trace_grp_to_val(cmd, trace_req->trace_module_id, &trace_req->trace_grp_idx)) {
        error = 0;
    } else if ((cmd[0] == '*' || cmd[0] == '.') && strlen(cmd) == 1) {
        /* Wildcard */
        trace_req->trace_grp_idx = -1;
        error = 0;
    }

    return error;
} /* cli_parse_trace_group */

static int cli_parse_thread_id(char *cmd, cli_req_t *req)
{
    int           error;
    ulong         value;
    trace_cli_req_t *trace_req = req->module_req;

    if ((cmd[0] == '*' || cmd[0] == '.') && strlen(cmd) == 1) {
        trace_req->thread_id = -1;
        error = 0;
    } else {
        error = cli_parse_ulong(cmd, &value, 0, VTSS_TRACE_MAX_THREAD_ID);
        trace_req->thread_id = value;
    }

    return error;
} /* cli_parse_thread_id */

static int32_t cli_trace_module_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                       cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_trace_module(cmd, req);
}

static int32_t cli_trace_group_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                      cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_trace_group(cmd, req);
}

static int32_t cli_parse_thread_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                          cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_thread_id(cmd, req);
}

static int32_t cli_trace_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                        cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    trace_cli_req_t *trace_req = req->module_req;

    req->parm_parsed = 1;
    T_D("ALLL %s", found);
    if (found != NULL) {
        if (!strncmp(found, "none", 4)) {
            trace_req->trace_lvl = VTSS_TRACE_LVL_NONE;
            trace_req->none = 1;
        }
        else if (!strncmp(found, "error", 5))
            trace_req->trace_lvl = VTSS_TRACE_LVL_ERROR;
        else if (!strncmp(found, "warning", 7))
            trace_req->trace_lvl = VTSS_TRACE_LVL_WARNING;
        else if (!strncmp(found, "info", 4))
            trace_req->trace_lvl = VTSS_TRACE_LVL_INFO;
        else if (!strncmp(found, "debug", 5))
            trace_req->trace_lvl = VTSS_TRACE_LVL_DEBUG;
        else if (!strncmp(found, "noise", 5))
            trace_req->trace_lvl = VTSS_TRACE_LVL_NOISE;
        else if (!strncmp(found, "racket", 6))
            trace_req->trace_lvl = VTSS_TRACE_LVL_RACKET;
        else if (!strncmp(found, "write", 5))
            trace_req->write = 1;
        else if (!strncmp(found, "read", 4))
            trace_req->read = 1;
        else if (!strncmp(found, "erase", 5))
            trace_req->erase = 1;
        else if (!strncmp(found, "enable", 6))
            req->enable = 1;
        else if (!strncmp(found, "disable", 7))
            req->disable = 1;
        else if (!strncmp(found, "yes", 3))
            req->enable = 1;
        else if (!strncmp(found, "no", 2))
            req->disable = 1;
    }

    return (found == NULL ? 1 : 0);
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t trace_cli_parm_table[] = {
    {
        "none|error|warning|info|debug|noise|racket",
        "none         : No trace\n"
        "error        : Error trace level\n"
        "warning      : Warning trace level\n"
        "info         : Information trace level\n"
        "debug        : Debug trace level\n"
        "noise        : Noise trace level\n"
        "racket       : Racket trace level\n"
        "(default     : Show trace level)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        NULL
    },
    {
        "write|read|erase",
        "write        : Write trace settings to flash\n"
        "read         : Read trace settings from flash\n"
        "erase        : Erase trace settings from flash\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        cli_cmd_debug_trace_config
    },
    {
        "enable|disable",
        "enable/disable trace for port\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_trace_module_port
    },
    {
        "enable|disable|yes|no",
        "enable/yes : Enable wall clock time stamp for trace messages"
        "disable/no : Disable wall clock time stamp for trace messages",
        CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        cli_cmd_debug_trace_module_timestamp
    },
    {
        "enable|disable|yes|no",
        "enable/yes : Enable usec time stamp for trace messages"
        "disable/no : Disable usec time stamp for trace messages",
        CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        cli_cmd_debug_trace_module_usec
    },
    {
        "enable|disable|yes|no",
        "enable/yes : Enable redirection of trace into ring buffer"
        "disable/no : Disable redirection of trace into ring buffer",
        CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        cli_cmd_debug_trace_module_ringbuf
    },
    {
        "enable|disable|yes|no",
        "enable/yes : Enable stackuse reporting in trace output"
        "disable/no : Disable stackuse reporting in trace output",
        CLI_PARM_FLAG_SET,
        cli_trace_parse_keyword,
        cli_cmd_debug_trace_thread_stackuse
    },
    {
        "<trace_module>",
        "Trace module or '*', default: Show module names",
        CLI_PARM_FLAG_NONE,
        cli_trace_module_parse,
        NULL
    },
    {
        "<trace_group>",
        "Trace group or '*', default: Show group names",
        CLI_PARM_FLAG_NONE,
        cli_trace_group_parse,
        NULL
    },
    {
        "<thread_id>",
        "Thread ID or '*', default: Show thread settings",
        CLI_PARM_FLAG_NONE,
        cli_parse_thread_id_parse,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_DEBUG_TRACE_MODULE_LEVEL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_MODULE_TIMESTAMP = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_MODULE_USEC = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_MODULE_RINGBUF = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_MODULE_PORT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_CONFIG = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_THREAD_LEVEL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_THREAD_STACKUSE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_REVERSE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_RINGBUFFER_PRINT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_RINGBUFFER_FLUSH = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_RINGBUFFER_START = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_RINGBUFFER_STOP = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TRACE_GLOBAL_LEVEL = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    "Debug Trace Module Port_trace [<port_list>]",
    "Debug Trace Module Port_trace [<port_list>] [enable|disable]",
    "Set or show trace module for port",
    PRIO_DEBUG_TRACE_MODULE_PORT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_module_port,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Trace Module Level [<trace_module>] [<trace_group>] "
    "[none|error|warning|info|debug|noise|racket]",
    "Set or show trace level for module/group",
    PRIO_DEBUG_TRACE_MODULE_LEVEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_module_level,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Module Timestamp [<trace_module>] [<trace_group>]",
    "Debug Trace Module Timestamp [<trace_module>] [<trace_group>] [enable|disable|yes|no]",
    "Enable wall clock time stamp in trace for module/group",
    PRIO_DEBUG_TRACE_MODULE_TIMESTAMP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_module_timestamp,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Module Usec [<trace_module>] [<trace_group>]",
    "Debug Trace Module Usec [<trace_module>] [<trace_group>] [enable|disable|yes|no]",
    "Enable wall usec time stamp in trace for module/group",
    PRIO_DEBUG_TRACE_MODULE_USEC,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_module_usec,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Module RingBuffer [<trace_module>] [<trace_group>]",
    "Debug Trace Module RingBuffer [<trace_module>] [<trace_group>] [enable|disable|yes|no]",
    "Redirect trace into ring buffer for module/group",
    PRIO_DEBUG_TRACE_MODULE_RINGBUF,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_module_ringbuf,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Trace Reverse",
    "Reverse previous trace level change.",
    PRIO_DEBUG_REVERSE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_reverse,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Trace Configuration [write|read|erase]",
    "Save/read trace configuration to/from flash.",
    PRIO_DEBUG_TRACE_CONFIG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_config,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Thread Level [<thread_id>]",
    "Debug Trace Thread Level [<thread_id>] [none|error|warning|info|debug|noise|racket]",
    "Set or show trace level for thread.",
    PRIO_DEBUG_TRACE_THREAD_LEVEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_thread_level,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Thread Stackuse [<thread_id>]",
    "Debug Trace Thread Stackuse [<thread_id>] [enable|disable|yes|no]",
    "Set or show stack size in trace output for thread.",
    PRIO_DEBUG_TRACE_THREAD_STACKUSE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_thread_stackuse,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace RingBuffer Print",
    "Debug Trace RingBuffer Print",
    "Print content of trace ring buffer.",
    PRIO_DEBUG_TRACE_RINGBUFFER_PRINT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_ringbuffer_print,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace RingBuffer Flush",
    "Debug Trace RingBuffer Flush",
    "Delete content of trace ring buffer.",
    PRIO_DEBUG_TRACE_RINGBUFFER_FLUSH,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_ringbuffer_flush,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace RingBuffer Start",
    "Debug Trace RingBuffer Start",
    "Start logging into ring buffer (for enabled groups).",
    PRIO_DEBUG_TRACE_RINGBUFFER_START,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_ringbuffer_start,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace RingBuffer Stop",
    "Debug Trace RingBuffer Stop",
    "Stop all logging into ring buffer.",
    PRIO_DEBUG_TRACE_RINGBUFFER_STOP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_ringbuffer_stop,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Trace Global Level",
    "Debug Trace Global Level [none|error|warning|info|debug|noise|racket]",    
    "Set or show global trace level.",
    PRIO_DEBUG_TRACE_GLOBAL_LEVEL,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_trace_global_level,
    cli_cmd_trace_default_func,
    trace_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
