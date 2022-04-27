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

 $Id$
 $Revision$

*/

#include "cli.h"
#include "cli_trace_def.h"
#include "cli_grp_help.h"
#include "cli_custom_api.h"
#include "led_api.h"
#include "msg_api.h"
#include "mgmt_api.h"
#include "topo_api.h"
#include "vlan_api.h"

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif /* VTSS_SW_OPTION_SYSLOG */

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#endif

#ifdef CYGPKG_THREADLOAD
#include <cyg/threadload/threadload.h>
#endif /* CYGPKG_THREADLOAD */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_CLI

/*lint -sem( cli_cmd_debug_prompt, thread_protected ) ... its safe to access global var 'cli' */
/*lint -sem( cli_cmd_help, thread_protected ) ... its safe to call help function */
/*lint -sem( vcli_cmd_exec, thread_protected ) ... its safe to call this function */
/*lint -sem( prompt, thread_protected ) ... its safe to call this function */

/* CLI cmd table boundaries definition */
/*lint -e{19} ...it's need to declare cli_cmd_tab_end, so skip lint error */
CYG_HAL_TABLE_BEGIN(cli_cmd_tab_start, cli_cmd_table);
/*lint -e{19} ...it's need to declare cli_cmd_tab_end, so skip lint error */
CYG_HAL_TABLE_END(cli_cmd_tab_end, cli_cmd_table);

extern struct cli_cmd_t cli_cmd_tab_start[], cli_cmd_tab_end;

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

static char vcli_prompt[CLI_PARM_MAX];
static char vcli_name[CLI_PARM_MAX];
static cyg_uint32 cli_max_req_size = 0;

/****************************************************************************/
/*  Command I/O                                                             */
/****************************************************************************/

static void cursor_left(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    if (pIO->cmd.cmd_cursor > 0) {
        pIO->cmd.cmd_cursor--;

        cli_putchar(ESC);
        cli_putchar(0x5b);
        cli_putchar(CURSOR_LEFT);
        cli_flush();
    }
}

static void cursor_right(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    if (pIO->cmd.cmd_cursor < pIO->cmd.cmd_len) {
        pIO->cmd.cmd_cursor++;

        cli_putchar(ESC);
        cli_putchar(0x5b);
        cli_putchar(CURSOR_RIGHT);
        cli_flush();
    }
}

static void cursor_home(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    while (pIO->cmd.cmd_cursor > 0) {
        cursor_left();
    }
}

static void delete_to_eol(void)
{
    cli_putchar(ESC);
    cli_putchar(0x5b);
    cli_putchar(CURSOR_DELETE_TO_EOL);
    cli_flush();
}

static void rewrite_to_eol(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    while (pIO->cmd.cmd_cursor < pIO->cmd.cmd_len) {
        cli_putchar(pIO->cmd.cmd_buf[pIO->cmd.cmd_cursor++]);
    }
    cli_flush();
}

static void delete_line(void)
{
    cursor_home();
    delete_to_eol();
}

static void append_line(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    uint cursor_save = pIO->cmd.cmd_cursor;
    rewrite_to_eol();
    while (pIO->cmd.cmd_cursor > cursor_save) {
        cursor_left();
    }
}

static void delete_char(BOOL backspace)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    uint j;

    if ((pIO->cmd.cmd_len == 0) ||
        (backspace && pIO->cmd.cmd_cursor == 0) ||
        (!backspace && pIO->cmd.cmd_cursor == pIO->cmd.cmd_len)) {
        // Cannot SP or DEL on empty line, or backspace on start of line, or delete on end of line
        return;
    }

    if (pIO->io.bEcho) {
        if (backspace) {
            cursor_left();
        }
        delete_to_eol();
    }

    /* Concatenate command string */
    pIO->cmd.cmd_len--;
    for (j = pIO->cmd.cmd_cursor; j < pIO->cmd.cmd_len; j++) {
        pIO->cmd.cmd_buf[j] = pIO->cmd.cmd_buf[j + 1];
    }

    /* Rewrite command part to the right of cursor */
    if (pIO->io.bEcho) {
        append_line();
    }
}

static void insert_char(char ch)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    uint j;

    delete_to_eol();
    for (j = pIO->cmd.cmd_len; j > pIO->cmd.cmd_cursor; j--) {
        pIO->cmd.cmd_buf[j] = pIO->cmd.cmd_buf[j - 1];
    }
    pIO->cmd.cmd_len++;
    pIO->cmd.cmd_buf[pIO->cmd.cmd_cursor++] = ch;

    append_line();
}

static void cmd_history_put(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    memcpy(pIO->cmd.cmd_history.buf[pIO->cmd.cmd_history.idx].cmd, pIO->cmd.cmd_buf, pIO->cmd.cmd_len);
    pIO->cmd.cmd_history.buf[pIO->cmd.cmd_history.idx].cmd_len = pIO->cmd.cmd_len - 1; /* don't include CR */
    if (pIO->cmd.cmd_history.len < MAX_CMD_HISTORY_LEN) {
        pIO->cmd.cmd_history.len++;
    }
    if (++pIO->cmd.cmd_history.idx >= MAX_CMD_HISTORY_LEN) {
        pIO->cmd.cmd_history.idx = 0;
    }
    pIO->cmd.cmd_history.scroll = 0;
}

static void cmd_history_get(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    uint idx;

    if (pIO->cmd.cmd_history.idx >= pIO->cmd.cmd_history.scroll) {
        idx = pIO->cmd.cmd_history.idx - pIO->cmd.cmd_history.scroll;
    } else {
        idx = MAX_CMD_HISTORY_LEN - (pIO->cmd.cmd_history.scroll - pIO->cmd.cmd_history.idx);
    }

    pIO->cmd.cmd_len = pIO->cmd.cmd_history.buf[idx].cmd_len;
    memcpy(pIO->cmd.cmd_buf, pIO->cmd.cmd_history.buf[idx].cmd, pIO->cmd.cmd_len);

}

static void get_old_cmd(void)
{
    delete_line();
    cmd_history_get();
    rewrite_to_eol();
}

typedef enum {
    ESC_INCOMPLETE,
    ESC_UNSUPPORTED,
    ESC_UP,
    ESC_DOWN,
    ESC_RIGHT,
    ESC_LEFT,
    ESC_HOME,
    ESC_END,
} cli_escape_keys_t;

static cli_escape_keys_t escape_seq_complete(char *chars, int len)
{
    // len must be >= 1 and chars[0] contain the char after the
    // initial ESC (which is 0x1b).
    // See e.g.
    // http://www.packetstormsecurity.org/UNIX/security/kernel.keylogger.txt
    if (chars[0] != 0x5B) {
        return ESC_UNSUPPORTED;    // Unsupported escape sequence. Stop it.
    }

    if (len == 1) {
        return ESC_INCOMPLETE;    // Not done yet.
    }

    switch (chars[1]) {
    case 0x31: // HOME or F6-F8
        if (len == 2) {
            return ESC_INCOMPLETE;
        }

        switch (chars[2]) {
        case 0x37: // Potential F6
        case 0x38: // Potential F7
        case 0x39: // Potential F8
            return len == 3 ? ESC_INCOMPLETE : ESC_UNSUPPORTED;

        case 0x7E:
            return ESC_HOME;

        default:
            return ESC_UNSUPPORTED;
        }

    case 0x32: // INS or F9-F12
        if (len == 2) {
            return ESC_INCOMPLETE;
        }

        switch (chars[2]) {
        case 0x30: // Potential F9
        case 0x31: // Potential F10
        case 0x33: // Potential F11
        case 0x34: // Potential F12
            return len == 3 ? ESC_INCOMPLETE : ESC_UNSUPPORTED;

        default:
            return ESC_UNSUPPORTED;
        }

    case 0x34: // Potential END
        if (len == 2) {
            return ESC_INCOMPLETE;
        }

        if (chars[2] == 0x7e) {
            return  ESC_END;
        }

        return ESC_UNSUPPORTED;

    case 0x33: // Potential DEL
    case 0x35: // Potential PGUP
    case 0x36: // Potential PGDN
        if (len == 2) {
            return ESC_INCOMPLETE;    // Eat the last char also.
        }

        return ESC_UNSUPPORTED;

    case 0x41:
        return ESC_UP;

    case 0x42:
        return ESC_DOWN;

    case 0x43:
        return ESC_RIGHT;

    case 0x44:
        return ESC_LEFT;

    case 0x5B: // Potential F1-F5
        return len == 2 ? ESC_INCOMPLETE : ESC_UNSUPPORTED;

    default:
        return ESC_UNSUPPORTED;
    }
}

static void process_escape_seq(cli_escape_keys_t escape_key)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    switch (escape_key) {
    case ESC_UP:
        if (pIO->cmd.cmd_history.scroll < pIO->cmd.cmd_history.len) {
            pIO->cmd.cmd_history.scroll++;
            get_old_cmd();
        } else {
            cli_putchar(BEL);
            cli_flush();
        }
        break;

    case ESC_DOWN:
        if (pIO->cmd.cmd_history.scroll > 0) {
            pIO->cmd.cmd_history.scroll--;

            if (pIO->cmd.cmd_history.scroll > 0) {
                get_old_cmd();
            } else {
                delete_line();
                pIO->cmd.cmd_len = 0;
                pIO->cmd.cmd_cursor = 0;
            }
        } else {
            cli_putchar(BEL);
            cli_flush();
        }
        break;

    case ESC_RIGHT:
        cursor_right();
        break;

    case ESC_LEFT:
        cursor_left();
        break;

    case ESC_HOME:
        cursor_home();
        break;

    case ESC_END:
        while (pIO->cmd.cmd_cursor < pIO->cmd.cmd_len) {
            cursor_right();
        }
        break;

    default:
        break;
    }
}

static BOOL empty_cmd_line(void)
{
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();
    uint j;

    for (j = 0; j < pIO->cmd.cmd_len; j++) {
        if ((pIO->cmd.cmd_buf[j] != ' ') && (pIO->cmd.cmd_buf[j] != CR)) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Get current stack state */
static void cli_stack_info_get(cli_io_t *pIO)
{
    vtss_usid_t       usid;
    vtss_isid_t       isid;
    cli_stack_state_t *stack = &pIO->cmd.stack;

    stack->master = msg_switch_is_master();
    stack->count = 0;
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if (stack->master) {
            /* MASTER: Include selected and active switches */
            isid = topo_usid2isid(usid);
            if ((pIO->cmd.usid != VTSS_USID_ALL && pIO->cmd.usid != usid) || !msg_switch_exists(isid)) {
                isid = VTSS_ISID_END;
            }
        } else {
            /* SLAVE: Include local switch only */
            isid = (usid == VTSS_USID_START ? VTSS_ISID_LOCAL : VTSS_ISID_END);
        }
        stack->isid[usid] = isid;
        if (isid != VTSS_ISID_END) {
            stack->isid_debug = isid;
            stack->count++;
        }
    }
    if (stack->count > 1) { /* Debug local switch if multiple selected */
        stack->isid_debug = VTSS_ISID_LOCAL;
    }
}

static void prompt(cli_io_t *pIO)
{
    int               i, len, standalone = 0;
    vtss_usid_t       usid;
    cli_stack_state_t *stack = &pIO->cmd.stack;

    /* Get current stack state */
    cli_stack_info_get(pIO);

    if (stack->master) {
        /* Master switch */
        if (vcli_name[0]) {
            cli_printf("%s:/", vcli_name);
        }
        if (pIO->cmd.usid == VTSS_USID_ALL) {
            if (stack->count > 1) {
                cli_printf("Master");
            } else {
                standalone = 1;
            }
        } else {
            cli_printf("Switch_%d", pIO->cmd.usid);
        }
    } else {
        /* Slave switch */
        cli_printf("Slave");
        usid = led_usid_get();
        if (VTSS_USID_LEGAL(usid)) {
            cli_printf("_%d", usid);
        }
    }
    len = strlen(pIO->cmd.cmd_group);
    if (len && !standalone) {
        cli_printf(":/");
    }
    for (i = 0; i < len; i++) {
        cli_putchar(pIO->cmd.cmd_group[i] == ' ' ? '/' : pIO->cmd.cmd_group[i]);
    }
    cli_printf(vcli_prompt);
    cli_flush();
}

static char *cmd_get(cli_io_t *pIO)
{
    pIO->cmd.cmd_len = 0;
    pIO->cmd.cmd_cursor = 0;

    return &pIO->cmd.cmd_buf[0];
}

static vtss_rc cmd_ready(cli_io_t *pIO)
{
    vtss_rc rc;
    char ch;
    static uint escape_seq_flag = FALSE;
    static uint escape_seq_count = 0;
    static char escape_seq[4];
    cli_escape_keys_t escape_key;

    if ((rc = pIO->io.cli_getch(&pIO->io, pIO->io.char_timeout, &ch)) != VTSS_OK) {
        return rc;
    }

    // Gotta process any escape seqs before ruling out specific chars.
    if (escape_seq_flag) {
        escape_seq[escape_seq_count++] = ch;
        escape_key = escape_seq_complete(escape_seq, escape_seq_count);
        if (escape_key != ESC_INCOMPLETE) {
            process_escape_seq(escape_key);
            escape_seq_flag = FALSE;
        }
        return VTSS_INCOMPLETE; // Don't store command in history buffer
    }

    if (ch == 0x01) {
        // HOME key
        process_escape_seq(ESC_HOME);
    } else if (ch == 0x03) {
        // Ctrl+C key. Clear current line - don't buffer it.
        cli_putchar(CR);
        cli_putchar(LF);
        cli_flush();
        (void)cmd_get(pIO); // Flushes the input buffer
        prompt(pIO);
    } else if (ch == 0x04 || ch == pIO->io.cDEL) {
        // DEL key
        delete_char(FALSE);
    } else if (ch == 0x05) {
        // END key
        process_escape_seq(ESC_END);
    } else if (ch == 0x08 || ch == pIO->io.cBS) {
        // Backspace key
        delete_char(TRUE);
    } else if (ch == ESC) {
        // Start of escape/control sequence
        escape_seq_flag = TRUE;
        escape_seq_count = 0;
    } else if ((/* ch >= 0x00 && */ ch <= 0x0c) || (ch >= 0x0e && ch <= 0x1f)) {
        // Discard special non-used, non-printable chars, since - if stored in the cmd and don't give rise
        // to a visible char - will allow the cursor to go left of the prompt when hitting BS.
    } else {
        if ((pIO->cmd.cmd_len < MAX_CMD_LEN - 1) && pIO->io.bEcho) {
            /* echo */
            cli_putchar(ch);
            cli_flush();
        }

        if (ch != CR) {
            if (pIO->cmd.cmd_len < MAX_CMD_LEN - 1) { /* Subtract 1 to allow for trailing '\0' */
                if (pIO->cmd.cmd_cursor < pIO->cmd.cmd_len) {
                    insert_char(ch);
                } else {
                    pIO->cmd.cmd_buf[pIO->cmd.cmd_cursor++] = ch;
                    if (pIO->cmd.cmd_len < pIO->cmd.cmd_cursor) {
                        pIO->cmd.cmd_len++;
                    }
                }
            }
        } else {
            cli_putchar(LF);
            cli_flush();

            /* Error-handling: ensure that CR is present in buffer in case of buffer overflow */
            if (pIO->cmd.cmd_len == MAX_CMD_LEN - 1) {
                pIO->cmd.cmd_buf[MAX_CMD_LEN - 2] = CR; // Also make room for trailing '\0'
            } else {
                pIO->cmd.cmd_buf[pIO->cmd.cmd_len++] = CR;
            }

            if (!empty_cmd_line()) {
                cmd_history_put();
            }

            pIO->cmd.cmd_buf[pIO->cmd.cmd_len++] = '\0';
            return VTSS_OK;
        }
    }

    return VTSS_INCOMPLETE;
}

/****************************************************************************/
/*  Public functions used for implementing vCLI commands.                   */
/****************************************************************************/
/* Check that the selected switch is active */
BOOL cli_cmd_switch_none(cli_req_t *req)
{
    if (req->stack.count) {
        return 0;
    }

    cli_puts("The selected switch is not active\n");
    return 1;
}

/* Check that configuration is allowed on switch */
BOOL cli_cmd_conf_slave(cli_req_t *req)
{
    /* SET operation allowed on master switch only */
    if (!req->set || req->stack.master) {
        return 0;
    }

    cli_puts("Configuration can not be changed on slave switch\n");
    return 1;
}

/* Checks if user is trying to do not allowed configuration at slave or a non active switch.Prints out error message is that is the case */
BOOL cli_cmd_slave_do_not_set(cli_req_t *req)
{

    // Switch must be active
    if (cli_cmd_switch_none(req)) {
        return 1;
    }

    // Check if changing configuration from a slave switch.
    if  (req->set) {
        return cli_cmd_slave(req);
    }
    return 0;
}

/* Check if management is allowed on switch */
BOOL cli_cmd_slave(cli_req_t *req)
{
    if (req->stack.master) {
        return 0;
    }

    cli_puts("Management of slave switch is not supported\n");
    return 1;
}

/* Print a switch header */
void cli_cmd_usid_print(vtss_usid_t usid, cli_req_t *req, BOOL nl)
{
    char buf[20];

    if (req->stack.count > 1) {
        sprintf(buf, "Switch %d", usid);
        cli_header_nl(buf, 1, 1);
    } else if (nl) {
        cli_puts("\n");
    }
}

#if defined(VTSS_SW_OPTION_L2PROTO)
const char *cli_l2port2uport_str(l2_port_no_t l2port)
{
    const char *s;

    s = l2port2str(l2port);
    while (*s == ' ') {
        s++;
    }
    return s;
}
#endif

/* Port numbers */
void cli_cmd_port_numbers(void)
{
#if defined(CLI_CUSTOM_PORT_TXT)
    cli_puts("Port Numbers:\n\n");
    cli_puts(CLI_CUSTOM_PORT_TXT);
#else
    // Print nothing to satisfy Lint:
    // error 522: (Warning -- Highest operation,
    // function 'cli_cmd_port_numbers', lacks side-effects)
    cli_printf("%s", "");
#endif /* CLI_CUSTOM_PORT_TXT */
}

/* Port list string */
char *cli_iport_list_txt(BOOL iport_list[VTSS_PORT_ARRAY_SIZE], char *buf)
{
    char *ret_buf;

    ret_buf = mgmt_iport_list2txt(iport_list, buf);
    if (strlen(ret_buf) == 0) {
        strcpy(buf, "None");
    }
    return buf;
}

/* Print two counters in columns */
void cli_cmd_stati(char *col1, char *col2, ulong c1, ulong c2)
{
    char buf[80];

    sprintf(buf, "%s:", col1);
    cli_printf("%-28s%10u   ", buf, c1);
    if (col2 != NULL) {
        sprintf(buf, "%s:", col2);
        cli_printf("%-28s%10u", buf, c2);
    }
    cli_puts("\n");
}

void cli_name_set(const char *name)
{
    strncpy(vcli_name, name, CLI_PARM_MAX - 1);
    vcli_name[CLI_PARM_MAX - 1] = '\0';
}

/****************************************************************************/
/* vCLI parameter parsing                                                   */
/****************************************************************************/

/* Parse comma/dash seperated list. for example '1,2,6-8' */
int cli_parm_parse_list(char *cmd, BOOL *list, ulong min, ulong max, BOOL def)
{
    return (mgmt_txt2list(cmd, list, min, max, def) == VTSS_OK ? 0 : 1);
}

/* Parse 'any' wildcard keyword */
int cli_parse_wc(char *cmd, cli_spec_t *spec)
{
    int  error;
    char *stx = "any";

    error = (strstr(stx, cmd) != stx);
    if (!error && spec != NULL) {
        *spec = CLI_SPEC_ANY;
    }
    return error;
}

/* Parse syntax word */
int cli_parse_word(char *cmd, char *stx)
{
    return (strstr(stx, cmd) != stx);
}

/* Parse 'all' keyword */
int cli_parse_all(char *cmd)
{
    return cli_parse_word(cmd, "all");
}

/* Parse 'CPU' keyword */
int cli_parse_cpu(char *cmd)
{
    return cli_parse_word(cmd, "cpu");
}

/* Parse 'none' keyword */
int cli_parse_none(char *cmd)
{
    return cli_parse_word(cmd, "none");
}

/* Parse 'disable' keyword */
int cli_parse_disable(char *cmd)
{
    return cli_parse_word(cmd, "disable");
}

/* MAC address. The legal format is "xx-xx-xx-xx-xx-xx" or "xx.xx.xx.xx.xx.xx" or "xxxxxxxxxxxx" (x is a hexadecimal digit). */
int cli_parse_mac(char *cmd, uchar *mac_addr, cli_spec_t *spec, BOOL wildcard)
{
    uint i, mac[6];
    int error = 1;

    if ((sscanf(cmd, "%2x-%2x-%2x-%2x-%2x-%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) ||
        sscanf(cmd, "%2x.%2x.%2x.%2x.%2x.%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
        for (i = 0; i < 6; i++) {
            mac_addr[i] = (mac[i] & 0xff);
        }
        *spec = CLI_SPEC_VAL;
        error = 0;

        for (i = 0; i < strlen(cmd); i++) {
            if (!isxdigit(cmd[i]) && (cmd[i] != '-') && (cmd[i] != '.')) {
                error = 1;
            }
        }
    } else if (strlen(cmd) == 12) {
        char mac_string[3];
        error = 0;
        for (i = 0; i < strlen(cmd); i++) {
            if (!isxdigit(cmd[i])) {
                error = 1;
                break;
            } else if (i % 2) {
                mac_string[1] = cmd[i];
                mac_string[2] = '\0';

                if (sscanf(mac_string, "%2x", &mac[0]) == 1) {
                    mac_addr[i / 2] = (mac[0] & 0xff);
                } else {
                    error = 1;
                    break;
                }
            } else {
                mac_string[0] = cmd[i];
            }
        }
    } else if (wildcard) {
        error = cli_parse_wc(cmd, spec);
    }
    return error;
}

/* Parse unsigned long integer */
int cli_parse_ulong(char *cmd, ulong *req, ulong min, ulong max)
{
    return (mgmt_txt2ulong(cmd, req, min, max) != VTSS_OK);
}

/* Parse signed long integer */
int cli_parse_long(char *cmd, long *req, long min, long max)
{
    long n;
    char  *end;

    n = strtol(cmd, &end, 0);
    if (*end != '\0') {
        return 1;
    }

    if (n < min || n > max) {
        return 1;
    }

    *req = n;
    return 0;
}

/* Parse signed long long integer */
longlong cli_parse_longlong(char *cmd, longlong *req, longlong min, longlong max)
{
    longlong n;
    char  *end;

    n = strtoll(cmd, &end, 0);
    if (*end != '\0') {
        return 1;
    }

    if (n < min || n > max) {
        return 1;
    }

    *req = n;
    return 0;
}

/* Parse unsigned long long integer */
ulonglong cli_parse_ulonglong(char *cmd, ulonglong *req, ulonglong min, ulonglong max)
{
    ulonglong n;
    char  *end;

    n = strtoull(cmd, &end, 0);
    if (*end != '\0') {
        return 1;
    }

    if (n < min || n > max) {
        return 1;
    }

    *req = n;
    return 0;
}


/* Parse unsigned long integer or 'any' */
int cli_parse_ulong_wc(char *cmd, ulong *req, ulong min, ulong max, cli_spec_t *spec)
{
    int error;

    error = cli_parse_ulong(cmd, req, min, max);
    if (error) {
        error = cli_parse_wc(cmd, spec);
    } else {
        *spec = CLI_SPEC_VAL;
    }
    return error;
}

/* Parse range, x or x-y */
int cli_parse_range(char *cmd, ulong *req_min, ulong *req_max, ulong min, ulong max)
{
    return (mgmt_txt2range(cmd, req_min, req_max, min, max) != VTSS_OK);
}

#if defined(VTSS_SW_OPTION_IP2)
/* IPv4 address/mask/router (a.b.c.d[/n]) */
int cli_parse_ipv4(char *cmd, vtss_ipv4_t *ipv4, vtss_ipv4_t *mask, cli_spec_t *spec, BOOL is_mask)
{
    int error;

    error = (int)mgmt_txt2ipv4(cmd, ipv4, mask, is_mask);
    if (!error) {
        *spec = CLI_SPEC_VAL;
    } else if (mask != NULL) {
        error = cli_parse_wc(cmd, spec);
    }
    return error;
}

int cli_parse_ipmcv4_addr(char *cmd, vtss_ipv4_t *ipv4, cli_spec_t *spec)
{
    int error;

    error = (int)mgmt_txt2ipmcv4(cmd, ipv4);
    if (!error) {
        *spec = CLI_SPEC_VAL;
    }
    return error;
}

/* IPV6 address/mask/router */
int cli_parse_ipv6(char *cmd, vtss_ipv6_t *ipv6, cli_spec_t *spec)
{
    int error;

    error = (int)mgmt_txt2ipv6(cmd, ipv6);
    if (!error) {
        *spec = CLI_SPEC_VAL;
    }
    return error;
}

int cli_parse_ip(char *cmd, vtss_ip_addr_t *ip, cli_spec_t *spec)
{
    if (cli_parse_ipv4(cmd, &ip->addr.ipv4, NULL, spec, 0) == 0) {
        ip->type = VTSS_IP_TYPE_IPV4;
        return 0;
    }

#ifdef VTSS_SW_OPTION_IPV6
    // Failed to parse it as an IPv4 address. Try IPv6.
    if (cli_parse_ipv6(cmd, &ip->addr.ipv6, spec) == 0) {
        ip->type = VTSS_IP_TYPE_IPV6;
        return 0;
    }
#endif
    *spec = CLI_SPEC_NONE;
    ip->type = VTSS_IP_TYPE_NONE;
    return 1; // 1 == error
}

int cli_parse_ipmcv6_addr(char *cmd, vtss_ipv6_t *ipv6, cli_spec_t *spec)
{
    int error;

    error = (int)mgmt_txt2ipmcv6(cmd, ipv6);
    if (!error) {
        *spec = CLI_SPEC_VAL;
    }
    return error;
}

static int cli_check_hostname(char *hostname)
{
    if (misc_str_is_hostname(hostname) != VTSS_OK) {
        return 1;
    } else {
        return 0;
    }
}
#endif /* defined(VTSS_SW_OPTION_IP2) */

/* Raw parameter */
int cli_parse_raw(char *cmd, cli_req_t *req)
{
    strncpy(req->parm, cmd, CLI_PARM_MAX - 1);
    return 0;
}

#define _STR_CMP_STX_TERM   ( (*stx) == 0 || (*stx) == ']' || (*stx) == '|' || (*stx) == ')' || (*stx) == ' ')
#define _STR_CMP_CMD_TERM   ( (*cmd) == 0 )
#define _STR_CMP_TERM       ( (! _STR_CMP_CMD_TERM) && (! _STR_CMP_STX_TERM) )
#define _SKIP_SPACE         for ( stx++; *stx == ' '; stx++ )

/*
    RETURN
        -1 : not match at all
         0 : cmd == stx, exactly match
         1 : cmd is a substring of stx
*/
/*
    RETURN
        -1 : not match at all
         0 : cmd == stx, exactly match
         1 : cmd is a substring of stx
*/
static int _parse_str_cmp(
    char    *cmd,
    char    **s
)
{
    int     i;
    char    *stx;

    for ( stx = *s; _STR_CMP_TERM; cmd++, stx++ ) {
        i = (*cmd) - (*stx);
        if ( i ) {
            /* output */
            *s = stx;
            return -1;
        }
    }

    /* output */
    *s = stx;

    /* check cmd is end or not */
    if ( ! _STR_CMP_CMD_TERM ) {
        //cmd is too long
        return -1;
    }

    if ( _STR_CMP_STX_TERM ) {
        //exactly match
        return 0;
    }

    //cmd is a substring
    return 1;
}

// Look for ambiguities and return NULL in that case, otherwise
// return a pointer to the location in stx (syntax) that match
// the command (keyword) held in cmd. If cmd doesn't fit
// anywhere within the stx, return NULL.
char *cli_parse_find(char *cmd, char *stx)
{
    char    *p,
            *sub,
            *exact;
    int     i;

    if ( cmd == NULL ) {
        //T_EG(VTSS_TRACE_GRP_VCLI, "cmd == NULL\n");
        return NULL;
    }

    if ( stx == NULL ) {
        //T_EG(VTSS_TRACE_GRP_VCLI, "stx == NULL\n");
        return NULL;
    }


    p = NULL;
    sub = NULL;
    exact = NULL;
    while ( 1 ) {
        switch (*stx) {
        case '[':
        case ']':
        case '(':
        case ')':
        case '|':
        case ' ':
            //skip space
            _SKIP_SPACE;
            continue;

        case '<':
            for ( ; *stx != '>' && *stx != 0; stx++ ) {
                ;
            }
            if ( *stx == 0 ) {
                T_EG(VTSS_TRACE_GRP_VCLI, "command syntax error\n");
                return NULL;
            }
            //skip space
            _SKIP_SPACE;
            continue;

        case 0: //EOS
            return exact ? exact : sub;

        default:
            break;
        }

        p = stx;
        i = _parse_str_cmp(cmd, &stx);
        switch (i) {
        case 0:
            if ( exact ) {
                return NULL;
            } else {
                exact = p;
            }
            break;

        case 1:
            if ( sub ) {
                return NULL;
            } else {
                sub = p;
                for ( ; ! _STR_CMP_STX_TERM; stx++ ) {
                    ;
                }
            }
            break;

        case -1:
            for ( ; ! _STR_CMP_STX_TERM; stx++ ) {
                ;
            }
            break;
        }

    }
}

/* Parse integer */
int cli_parse_integer(char *cmd, cli_req_t *req, char *stx)
{
    ulong i;
    for (i = 0; i < strlen(cmd); i++) {
        if (i == 0 &&
            (cmd[i] == '-' || cmd[i] == '+')) {
            continue;
        }

        if (cmd[i] < '0' || cmd[i] > '9' ) {
            return 1; /* Error! */
        }
    }

    req->int_values[req->int_value_cnt++] = atoi(cmd);

    return 0;
} /* cli_parse_integer */

/******************************************************************************/
// Parses an optionally quoted string, whose result is stored in @result.
// The result string is at most @len characters long including the terminating
// NULL character. Due to the initial parsing (cli_parse_command(), it is
// impossible to have quotes embedded in the string.
// Examples:
//   @cmd == "abcd",  @len == 30 => result = abcd\0
//   @cmd == abcd,    @len == 30 => result = abcd\0
//   @cmd == abcd",   @len == 30 => impossible
//   @cmd == "abcd,   @len == 30 => impossible
//   @cmd == ab"cd,   @len == 30 => impossible
//   @cmd == ""abcd", @len == 30 => impossible
//   @cmd == "ab cd", @len == 30 => result = ab cd\0
//   @cmd == ab cd,   @len == 30 => impossible
//   @cmd == "",      @len == 30 => result = \0
//   @cmd == abcd,    @len ==  4 => error = 1
//
// Returns 1 on error, 0 on success.
/******************************************************************************/
int cli_parse_quoted_string(const char *cmd, char *result, int len)
{
    int slen;

    if ((cmd == NULL) || (result == NULL)) {
        return 1;
    }

    slen = strlen(cmd);
    if (cmd[0] == '\"' && cmd[slen - 1] == '\"') {
        if (slen == 1) {
            return 1;
        }
        cmd++;
        slen -= 2;
    }

    if (slen >= len) {
        cli_printf("The maximum string length is %d.\n", len - 1);
        return 1;
    }

    strncpy(result, cmd, slen);
    result[slen] = '\0';
    return 0;
}

/* Parse raw text admin string */
int cli_parse_string(const char *cmd, char *admin_string, size_t min, size_t max)
{
    int error;
    ulong idx;

    if ((error = cli_parse_text(cmd, admin_string, max + 1) != 0)) {
        return error;
    }

    for (idx = 0; idx < strlen(admin_string); idx++) {
        if (admin_string[idx] < 33 || admin_string[idx] > 126) {
            cli_puts("The format is restricted to the ASCII character set (33 - 126)\n");
            return 1;
        }
    }

    if (strlen(admin_string) < min || strlen(admin_string) > max) {
        cli_printf("The length is restricted to %zu - %zu\n", min, max);
        return 1;
    }

    return 0;
}

/* Parse raw text string */
int cli_parse_text(const char *cmd, char *parm, int max)
{
    if ((cmd == NULL) || (parm == NULL)) {
        return 1;
    }

    if (cli_parse_quoted_string(cmd, parm, max)) {
        return 1;
    }

    if (!strncmp(cmd, "clear", 5)) { /* Use 'clear' to clear string */
        parm[0] = '\0';
    }

    return 0;
}

/* Parses raw text string and checks that it only contains numbers
   The int returned is error indicator. 0 = No error (Cmd only contained numbers), 1 = Error
*/
vtss_rc cli_parse_text_numbers_only(const char *cmd, char *parm, int max)
{
    if (cli_parse_quoted_string(cmd, parm, max)) {
        return VTSS_INVALID_PARAMETER;
    }

    return misc_str_chk_numbers_only(cmd);
}

/* Parse keyword string, for example 'disable' */
int cli_parm_parse_keyword(char *cmd, char *cmd2,
                           char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    if (found != NULL) {

        if (!strncmp(found, "all", 3)) {
            req->all = 1;
#ifdef VTSS_SW_OPTION_ICLI
        } else if (!strncmp(found, "toggle", 6)) {
            req->toggle = 1;
#endif
        } else if (!strncmp(found, "backtrace", 9)) {
            req->all = 1;
        } else if (!strncmp(found, "clear", 5)) {
            req->clear = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "port", 4)) {
            req->port = 1;
        } else if (!strncmp(found, "smac", 4)) {
            req->smac = 1;
        } else if (!strncmp(found, "dmac", 4)) {
            req->dmac = 1;
        } else if (!strncmp(found, "ip", 2)) {
            req->aggr_ip = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

int cli_parm_parse_port_list(char *cmd, char *cmd2,
                             char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong min = 1, max = VTSS_PORTS;

    req->parm_parsed = 1;
    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, req->uport_list, min, max, 1));
    return (error);
}

// Function for parsing a list that also allow CPU port
int cli_parm_parse_port_cpu_list(char *cmd, char *cmd2,
                                 char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong min = 1, max = VTSS_PORTS;

    req->parm_parsed = 1;

    BOOL cpu_key_word_found = FALSE; // Indicating if cpu or part of cpu is found within the command

    // Take a copy of the cmd that we can work with.
    char parse_cmd[MAX_CMD_LEN];
    strcpy(&parse_cmd[0], cmd); //


    char new_cmd[MAX_CMD_LEN];
    strcpy(&new_cmd[0], ""); // Use for generating a new cmd with out the cpu keyword.


    // Splitting the comma separated list into single word and determine if the cpu keyword is in the list.
    char *pch = strtok (&parse_cmd[0], ",");
    BOOL first = TRUE; // use to determine if a comma shall be inserted in the new command.
    while (pch != NULL) {
        T_DG(VTSS_TRACE_GRP_VCLI, "new_cmd:%s pch:%s cmd:%s cpu_key_word_found:%d\n", &new_cmd[0], pch, cmd, cpu_key_word_found);
        if (!cli_parse_cpu(pch)) {
            cpu_key_word_found = TRUE; // Ok - cpu found. Shall be excluded from the new command
        } else {
            // Make a new command with out "cpu". Insert comma in case of multiple ports is found.
            if (first) {
                first = FALSE;
            } else {
                strcat(&new_cmd[0], ",");
            }
            strcat(&new_cmd[0], pch);
        }
        pch = strtok (NULL, ",");
    }

    // Parse the result
    if (cpu_key_word_found) {
        // CPU keyword found
        req->cpu_port = TRUE;                                   // Signal CPU port shall be set.
        error = cli_parm_parse_list(&new_cmd[0], req->uport_list, min, max, 0); // Clear the port list
        T_DG(VTSS_TRACE_GRP_VCLI, "Error:%d", error);
    } else if (!cli_parse_all(cmd)) {
        req->cpu_port = TRUE;                                    // Signal CPU port shall be set.
        error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, req->uport_list, min, max, 1));
        T_DG(VTSS_TRACE_GRP_VCLI, "Error:%d", error);
    } else {
        error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, req->uport_list, min, max, 1));
        // If the parse went well then the command contained numbers only and CPU shall not be updated, else use default for cpu_port.
        if (error == VTSS_RC_OK) {
            req->cpu_port = FALSE;                                    // Signal CPU port shall not be set.
        }

        T_DG(VTSS_TRACE_GRP_VCLI, "Error:%d", error);
    }

    T_DG(VTSS_TRACE_GRP_VCLI, "req->cpu_port:%d, cli_parse_cpu(cmd):%d cli_parse_all(cmd):%d, error:%d, min:%u, max:%u",
         req->cpu_port, cli_parse_cpu(cmd), cli_parse_all(cmd), error, min, max);
    return (error);
}

int cli_parm_parse_mac_addr(char *cmd, char *cmd2,
                            char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_mac(cmd, req->mac_addr, &req->mac_addr_spec, 0);
    return (error);
}

int cli_parm_parse_sid(char *cmd, char *cmd2,
                       char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong value = 0;

    req->parm_parsed = 1;

    error = (cli_parse_ulong(cmd, &value, VTSS_USID_START, VTSS_USID_END - 1) &&
             cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req));
    if (req->usid[0] == USID_UNSPECIFIED) {
        req->usid[0] = value;
    } else if (req->usid[1] == USID_UNSPECIFIED) {
        req->usid[1] = value;
    } else {
        error = 1;
    }

    return (error);
}

#if defined(VTSS_SW_OPTION_IP2)
int cli_parm_parse_ipaddr_str(char *cmd, char *cmd2,
                              char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_quoted_string(cmd, req->host_name, VTSS_SYS_HOSTNAME_LEN);
    if (!error) {
        error = cli_check_hostname(req->host_name);
        if (!error) {
            req->host_name_spec = CLI_SPEC_VAL;
        }
    }
    T_DG(VTSS_TRACE_GRP_VCLI, "error = %d", error);
    return (error);
}

int cli_parm_parse_ipaddr(char *cmd, char *cmd2,
                          char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &req->ipv4_addr, NULL, &req->ipv4_addr_spec, 0);
    return (error);
}
#endif /* defined(VTSS_SW_OPTION_IP2) */

#ifdef VTSS_SW_OPTION_QOS
int cli_parm_parse_class(char *cmd, char *cmd2,
                         char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = (mgmt_txt2prio(cmd, &req->class_) != VTSS_OK);
    if (!error) {
        req->class_spec = CLI_SPEC_VAL;
    }
    return (error);
}
#endif /* VTSS_SW_OPTION_QOS */

int cli_parm_parse_vid(char *cmd, char *cmd2,
                       char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong value = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX);
    if (!error) {
        req->vid_spec = CLI_SPEC_VAL;
        req->vid = value;
    }

    return (error);
}

int cli_parm_parse_port(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong value = 0;
    ulong min = 1, max = VTSS_PORTS;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, min, max);
    req->uport = value;
    return (error);
}

int cli_parm_parse_addr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->addr, 0, 0xffffffff);
    return (error);
}

int cli_parm_parse_fill_val(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->fill_val, 0, 0xffffffff);
    return (error);
}

int cli_parm_parse_item_cnt(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->item_cnt, 1, 0x7fffffff);
    return (error);
}

int cli_parm_parse_item_size(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->item_size, 1, 4);
    if (req->item_size != 1 && req->item_size != 2 && req->item_size != 4) {
        error = 1;
    }
    return (error);
}


int cli_parm_parse_integer(char *cmd, char *cmd2,
                           char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_integer(cmd, req, stx);
    return (error);
}

int cli_parm_parse_prompt(char *cmd, char *cmd2,
                          char *stx, char *cmd_org, cli_req_t *req)
{
    int error;

    req->parm_parsed = 1;
    error = cli_parse_raw(cmd_org, req);
    return (error);
}

int cli_parm_parse_thread_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 0, 65535);
    return error;
}

int cli_parm_parse_thread_prio(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->fill_val, 0, 31);
    return error;
}

/* Set CLI request default values */
static int cli_req_default_set(cli_req_t *req)
{
    int ret;
    cli_io_t *pIO = (cli_io_t *)cli_get_io_handle();

    memset(req, 0, sizeof(*req));

    req->usid_sel = pIO->cmd.usid; /* Current USID */
    req->stack = pIO->cmd.stack; /* Current stack information */

    req->usid[0]  = USID_UNSPECIFIED;
    req->usid[1]  = USID_UNSPECIFIED;

    ret = cli_parm_parse_list(NULL, req->uport_list, 0, VTSS_PORT_NO_END, 1);
    req->cpu_port = TRUE;
    req->vid = VLAN_ID_DEFAULT;
    req->item_cnt = 1;
    req->item_size = 1;
    return ret;
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_cmd_help(cli_req_t *req)
{
    cli_header_nl("General Commands", 0, 0);
    cli_puts("Help/?: Get help on a group or a specific command\n");
    cli_puts("Up    : Move one command level up\n");
    cli_puts("Logout: Exit CLI\n");

    cli_header_nl("Command Groups", 1, 0);
    cli_cmd_grp_disp();

    cli_puts("\nType '<group>' to enter command group, e.g. 'port'.\n");
    cli_puts("Type '<group> ?' to get list of group commands, e.g. 'port ?'.\n");
    cli_puts("Type '<command> ?' to get help on a command, e.g. 'port mode ?'.\n");
    cli_puts("Commands may be abbreviated, e.g. 'por co' instead of 'port configuration'.\n");

}

static void cli_cmd_question(cli_req_t *req)
{
    cli_cmd_help(req);
}

static void cli_cmd_up(cli_req_t *req)
{
    return;
}

static void cli_cmd_logout(cli_req_t *req)
{
    cli_get_io_handle()->bIOerr = TRUE;
}

/* Debug thread */
static void cli_cmd_debug_thread(cli_req_t *req)
{
#ifdef CYGPKG_THREADLOAD
    if (req->set) {
        cli_printf("%sabling thread-load monitoring\n", req->enable ? "En" : "Dis");
        if (req->enable) {
            cyg_threadload_start();
        } else {
            cyg_threadload_stop();
        }
    } else
#endif
    {
        cli_print_thread_status(cli_printf, req->all, FALSE);
    }
}

static void cli_cmd_debug_thread_prio(cli_req_t *req)
{
    u16             requested_thread_id   = req->value, thread_id = 0;
    u8              requested_thread_prio = req->fill_val;
    cyg_handle_t    thread_handle = 0;
    cyg_thread_info thread_info;
    int             cnt = 0;
    char            buf[24];
    u32             len;

    if (req->set && requested_thread_id < 1) {
        cli_puts("Error: Cannot set priority for all threads in one go\n");
        return;
    }

    while (cyg_thread_get_next(&thread_handle, &thread_id) != 0) {
        if (requested_thread_id != 0 && requested_thread_id != thread_id) {
            // requested_thread_id == 0 means: Handle all threads.
            continue;
        }

        cnt++;

        if (req->set) {
            cyg_thread_set_priority(thread_handle, requested_thread_prio);
        } else {
            if (cnt == 1) {
                cli_table_header("ID  Name                    SetPrio  CurPrio");
            }
            (void)cyg_thread_get_info(thread_handle, thread_id, &thread_info);
            memset(buf, ' ', sizeof(buf));
            len = (thread_info.name == NULL ? 0 : strlen(thread_info.name));
            memcpy(buf, thread_info.name, len > sizeof(buf) ? sizeof(buf) : len);
            buf[sizeof(buf) - 1] = '\0';
            cli_printf("%-3d %s %7d  %7d\n", thread_id, buf, thread_info.set_pri, thread_info.cur_pri);
        }
    }

    if (cnt == 0) {
        cli_puts("Error: No such thread\n");
    }
}


static void cli_cmd_debug_prompt(cli_req_t *req)
{
    strcpy(vcli_prompt, req->parm);
}

#ifdef VTSS_SW_OPTION_ICLI
static void cli_cmd_debug_cli(cli_req_t *req)
{
    cli_iolayer_t *pIO = cli_get_io_handle();

    if (req->set && req->toggle) {
        pIO->parser = (pIO->parser == CLI_PARSER_ICLI) ? CLI_PARSER_VCLI : CLI_PARSER_ICLI;
    } else {
        cli_printf("Current CLI is %s\n", (pIO->parser == CLI_PARSER_ICLI) ? "iCLI" : "vCLI");
    }
}
#endif

/* Print memory array */
/* start_pos included, end_pos not included. */
static void mem_print(u32 addr, u8 *arr, int start_pos, int end_pos)
{
    int i;
    addr &= ~0xF;
    cli_printf("%08x: ", addr);
    for (i = 0; i < start_pos; i++) {
        cli_puts("   ");
    }
    for (i = start_pos; i < end_pos; i++) {
        cli_printf("%02x%c", arr[i], i == 7 ? '-' : ' ');
    }
    for (i = end_pos; i < 16; i++) {
        cli_puts("   ");
    }
    for (i = 0; i < start_pos; i++) {
        cli_puts(" ");
    }
    for (i = start_pos; i < end_pos; i++) {
        cli_printf("%c", isprint(arr[i]) ? arr[i] : '.');
    }
    cli_puts("\n");
}

/* Debug memory dump */
static void cli_cmd_debug_mem_dump(cli_req_t *req)
{
    u8  arr[16];
    int start_pos, end_pos, i;

    if ((req->addr & (req->item_size - 1)) != 0) {
        cli_puts("Error: <addr> misaligned with <item_size>\n");
        return;
    }

    // start_pos included, end_pos not included.
    switch (req->item_size) {
    case 1: {
        u8 *p = (u8 *)req->addr;
        while (req->item_cnt > 0) {
            start_pos = (u32)p & 0xF;
            end_pos   = start_pos + req->item_cnt < 16 ? start_pos + req->item_cnt : 16;
            for (i = start_pos; i < end_pos; i++) {
                arr[i] = *p++;
            }
            mem_print(req->addr, arr, start_pos, end_pos);
            u32 cnt = end_pos - start_pos;
            req->addr     += cnt;
            req->item_cnt -= cnt;
        }
        break;
    }

    case 2: {
        u16 *p = (u16 *)req->addr;
        while (req->item_cnt > 0) {
            start_pos = (u32)p & 0xF;
            end_pos   = start_pos + 2 * req->item_cnt < 16 ? start_pos + 2 * req->item_cnt : 16;
            for (i = start_pos; i < end_pos; i += 2) {
                *((u16 *)&arr[i]) = *p++;
            }
            mem_print(req->addr, arr, start_pos, end_pos);
            u32 cnt = end_pos - start_pos;
            req->addr     += cnt;
            req->item_cnt -= cnt / 2;
        }
        break;
    }

    case 4: {
        u32 *p = (u32 *)req->addr;
        while (req->item_cnt > 0) {
            start_pos = (u32)p & 0xF;
            end_pos   = start_pos + 4 * req->item_cnt < 16 ? start_pos + 4 * req->item_cnt : 16;
            for (i = start_pos; i < end_pos; i += 4) {
                *((u32 *)&arr[i]) = *p++;
            }
            mem_print(req->addr, arr, start_pos, end_pos);
            u32 cnt = end_pos - start_pos;
            req->addr     += cnt;
            req->item_cnt -= cnt / 4;
        }
        break;
    }

    default:
        cli_puts("Not supported\n");
        break;
    }
}

/* Debug memory fill */
static void cli_cmd_debug_mem_fill(cli_req_t *req)
{
    ulong i;

    if ((req->addr & (req->item_size - 1)) != 0) {
        cli_puts("Error: <addr> misaligned with <item_size>\n");
        return;
    }

    if ((req->item_size == 1 && (req->fill_val & 0xFFFFFF00) != 0) ||
        (req->item_size == 2 && (req->fill_val & 0xFFFF0000) != 0)) {
        cli_puts("Error: <fill_val> to big for <item_size>\n");
        return;
    }

    // start_pos included, end_pos not included.
    switch (req->item_size) {
    case 1: {
        u8 *p = (u8 *)req->addr;
        for (i = 0; i < req->item_cnt; i++) {
            *p++ = req->fill_val;
        }
        break;
    }
    case 2: {
        u16 *p = (u16 *)req->addr;
        for (i = 0; i < req->item_cnt; i++) {
            *p++ = req->fill_val;
        }
        break;
    }

    case 4: {
        u32 *p = (u32 *)req->addr;
        for (i = 0; i < req->item_cnt; i++) {
            *p++ = req->fill_val;
        }
        break;

    }

    default:
        cli_puts("Not supported\n");
        break;
    }
}

/* Debug Initialization */
static void cli_cmd_debug_init(cli_req_t *req)
{
    init_cmd_t       cmd;
    vtss_init_data_t data;
    vtss_isid_t      isid;

    switch (req->int_values[0]) {
    case INIT_CMD_INIT:
        cmd = INIT_CMD_INIT;
        break;
    case INIT_CMD_START:
        cmd = INIT_CMD_START;
        break;
    case INIT_CMD_CONF_DEF:
        cmd = INIT_CMD_CONF_DEF;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            data.switch_info[isid].configurable = TRUE;
        }
        break;
    case INIT_CMD_MASTER_UP:
        cmd = INIT_CMD_MASTER_UP;
        break;
    case INIT_CMD_MASTER_DOWN:
        cmd = INIT_CMD_MASTER_DOWN;
        break;
    case INIT_CMD_SWITCH_ADD:
        cmd = INIT_CMD_SWITCH_ADD;
        // Hmm. This will probably confuse the port module, which looks at switch_info[].
        break;
    case INIT_CMD_SWITCH_DEL:
        cmd = INIT_CMD_SWITCH_DEL;
        break;
    case INIT_CMD_SUSPEND_RESUME:
        cmd = INIT_CMD_SUSPEND_RESUME;
        break;
    default:
        cli_puts("Wrong Init Command\n");
        return;
    }

    memset(&data, 0, sizeof(data));
    data.cmd = cmd;
    data.isid = req->int_values[1];
    data.flags = req->int_values[2];
    data.resume = req->int_values[1];
    (void)init_modules(&data);
}

/* System configuration all modules */
void cli_system_conf_disp(cli_req_t *req)
{
    // This function *is* thread-safe, because cli_cmd_tab_start/end are never changing */
    /*lint -esym(459, cli_cmd_tab_start, cli_cmd_tab_end) */

#if VTSS_SWITCH_STACKABLE
    msg_switch_info_t info;

    if (req->stack.master) {
        vtss_usid_t usid;
        vtss_isid_t isid;
        int         first = 1;

        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }

            if (first) {
                cli_puts("\n");
                cli_table_header("SID  Chip ID   Software Version");
                first = 0;
            }
            if (msg_switch_info_get(isid, &info) == VTSS_RC_OK) {
                cli_printf("%-2d   VSC%-5x  %s\n", usid, info.info.api_inst_id, info.version_string);
            }
        }
    }
#endif /* VTSS_SWITCH_STACKABLE */

    if (req->all || req->port) {
        int i, tab_size = (&cli_cmd_tab_end - cli_cmd_tab_start);

        for (i = 0; i < tab_size; i++) {
            if (cli_getkey(CTLC)) {
                cli_puts("Interrupted by ctrl-c\n");
                return;
            }
            if (cli_cmd_tab_start[i].flags & CLI_CMD_FLAG_SYS_CONF) {
                if (cli_cmd_tab_start[i].cmd_fun) {
                    memset(req->module_req, 0, cli_max_req_size);
                    if (cli_cmd_tab_start[i].def_fun) {
                        cli_cmd_tab_start[i].def_fun(req);
                    }
                    cli_cmd_tab_start[i].cmd_fun(req);
                } else {
                    T_EG(VTSS_TRACE_GRP_VCLI, "Internal error: Command with no action routine: %s",
                         cli_cmd_tab_start[i].ro_syntax ?
                         cli_cmd_tab_start[i].ro_syntax :
                         cli_cmd_tab_start[i].rw_syntax);
                    return;
                }
            }
        }
    }
}

/****************************************************************************/
/*  Generic Parameter Table                                                 */
/****************************************************************************/
static char cli_sid_help_str[20];
static cli_parm_t cli_parm_table[] = {
    {
        "<port_list>",
        "Port list or 'all', default: All ports",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_port_list,
        NULL
    },
    {
        "<port_cpu_list>",
        "Port list or CPU or 'all', default: All ports and CPU",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_port_cpu_list,
        NULL
    },
    {
        "<mac_addr>",
        "MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_mac_addr,
        NULL
    },
    {
        "<sid>",
        cli_sid_help_str,
        CLI_PARM_FLAG_SET,
        cli_parm_parse_sid,
        NULL
    },
#if defined(VTSS_SW_OPTION_IP2)
    {
        "<ip_addr_string>",
#ifdef VTSS_SW_OPTION_DNS
        "IP host address (a.b.c.d) or a host name string",
#else
        "IP host address (a.b.c.d)",
#endif
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ipaddr_str,
        NULL
    },
    {
        "<ip_addr>",
        "IP address (a.b.c.d), default: Show IP address",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ipaddr,
        NULL
    },
#endif /* defined(VTSS_SW_OPTION_IP2) */
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vid,
        NULL
    },
    {
        "<port>",
        "Port number",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_port,
        NULL
    },
    {
        "<addr>",
        "Address",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_addr,
        NULL
    },
    {
        "<fill_val>",
        "Value to write",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_fill_val,
        NULL
    },
    {
        "<item_cnt>",
        "Number of locations. "
        "Default: 1",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_item_cnt,
        NULL,
    },
    {
        "<item_size>",
        "Size - in bytes - of each memory location. "
        "Valid values: 1, 2, 4 "
        "Default: 1",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_item_size,
        NULL,
    },
    {
        "<integer>",
        "Integer value",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_integer,
        NULL,
    },
    {
        "clear",
        "clear   : Clear log",
        CLI_PARM_FLAG_NO_TXT,
        cli_parm_parse_keyword,
        NULL,
    },
    {
        "backtrace",
        "If specified, a backtrace of the thread's stack will be shown.\n"
        "The topmost is the innermost function (closest to the current execution point).\n"
        "Use gdb's \"info line *<addr>\" to figure out the source file and line number of a given address.\n"
#ifdef __mips__
        "NOTE: SOME ENTRIES MAY BE FALSE\n"
#endif
        ,
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_debug_thread,
    },
#ifdef CYGPKG_THREADLOAD
    {
        "enable|disable",
        "Turns on/off monitoring of per-thread CPU load.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_thread,
    },
#endif
    {
        "<thread_id>",
        "The Thread ID to set or get the current priority for.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_thread_id,
        cli_cmd_debug_thread_prio,
    },
    {
        "<thread_prio>",
        "The Thread Priority to set the chosen thread ID to. Use with care.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_thread_prio,
        cli_cmd_debug_thread_prio,
    },
    {
        "<prompt>",
        "CLI prompt string",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_prompt,
        NULL,
    },
#ifdef VTSS_SW_OPTION_ICLI
    {
        "toggle",
        "Switch to other CLI (vCLI -> iCLI or vice versa).",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_cli,
    },
#endif
    {NULL, NULL, 0, 0, NULL}
};  /* cli_parm_table */

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    CLI_CMD_HELP_PRIO = 0,
    CLI_CMD_UP_PRIO,
    CLI_CMD_LOGOUT_PRIO,
    CLI_CMD_DEBUG_THREAD_PRIO      = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_THREAD_PRIO_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_MEM_DUMP_PRIO    = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_MEM_FILL_PRIO    = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_INIT_PRIO        = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PROMPT_PRIO      = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_CLI              = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
    "Help",
    NULL,
    "Show CLI general help text",
    CLI_CMD_HELP_PRIO,
    CLI_CMD_TYPE_NONE,
    VTSS_MODULE_ID_NONE,
    cli_cmd_help,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "?",
    NULL,
    "Show CLI general help text",
    CLI_CMD_HELP_PRIO,
    CLI_CMD_TYPE_NONE,
    VTSS_MODULE_ID_NONE,
    cli_cmd_question,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Up",
    NULL,
    "Move one command level up",
    CLI_CMD_UP_PRIO,
    CLI_CMD_TYPE_NONE,
    VTSS_MODULE_ID_NONE,
    cli_cmd_up,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Logout",
    NULL,
    "Logout from this CLI session",
    CLI_CMD_LOGOUT_PRIO,
    CLI_CMD_TYPE_NONE,
    VTSS_MODULE_ID_NONE,
    cli_cmd_logout,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

#ifdef CYGPKG_THREADLOAD
#define DEBUG_THREAD_STX  "Debug Thread [backtrace] [enable|disable]"
#define DEBUG_THREAD_HELP "Show threads with optional stack backtrace. The command also allows for enabling/disabling monitoring of per-thread CPU load"
#else
#define DEBUG_THREAD_STX "Debug Thread [backtrace]"
#define DEBUG_THREAD_HELP "Show threads with optional stack backtrace"
#endif
cli_cmd_tab_entry(
    DEBUG_THREAD_STX,
    NULL,
    DEBUG_THREAD_HELP,
    CLI_CMD_DEBUG_THREAD_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_thread,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Prio [<thread_id>] [<thread_prio>]",
    NULL,
    "Show or set thread priority",
    CLI_CMD_DEBUG_THREAD_PRIO_PRIO,
    CLI_CMD_TYPE_NONE,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_thread_prio,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Memory Dump <addr> [<item_cnt>] [<item_size>]",
    NULL,
    "Dump memory using either 1, 2, or 4 byte reads",
    CLI_CMD_DEBUG_MEM_DUMP_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mem_dump,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Memory Fill <addr> <fill_val> [<item_cnt>] [<item_size>]",
    NULL,
    "Fill memory using either 1, 2, or 4 byte writes",
    CLI_CMD_DEBUG_MEM_FILL_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mem_fill,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    "Debug Init <integer> [<integer>] [<integer>]",
    "Call init_modules(cmd, p1, p2)",
    CLI_CMD_DEBUG_INIT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_init,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    "Debug Prompt <prompt>",
    "Set the CLI prompt string",
    CLI_CMD_DEBUG_PROMPT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_prompt,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_ICLI
cli_cmd_tab_entry(
    NULL,
    "Debug CLI [toggle]",
    "Show or toggle current CLI",
    CLI_CMD_DEBUG_CLI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_cli,
    NULL,
    cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

/****************************************************************************/
/*  Command Parsing                                                         */
/****************************************************************************/

/* Build array of command/syntax words */
static BOOL cli_build_words(char *str, int *count, char **words, BOOL lower)
{
    int  i, j, len;
    char *p;
    BOOL in_quotes = FALSE;

    len = strlen(str);
    j = 0;
    *count = 0;
    for (i = 0; i < len; i++) {
        p = &str[i];
        if (*p == '\"') {
            in_quotes = !in_quotes;
        }
        if (!in_quotes && isspace(*p)) {
            j = 0;
            *p = '\0';
        } else {
            if (j == 0) {
                if (*count >= MAX_WORD_CNT) {
                    T_IG(VTSS_TRACE_GRP_VCLI, "MAX_WORD_CNT exceeded");
                    return FALSE;
                }
                words[*count] = p;
                (*count)++;
            }
            if (lower) {
                *p = tolower(*p);
            }
            j++;
            if (j >= MAX_WORD_LEN) {
                T_IG(VTSS_TRACE_GRP_VCLI, "MAX_WORD_LEN (%d) exceeded", MAX_WORD_LEN);
                return FALSE;
            }
        }
    }
    return TRUE;
}

static char *cli_lower_word(char *in, char *out)
{
    int i, len;

    len = strlen(in);
    for (i = 0; i <= len; i++) {
        out[i] = tolower(in[i]);
    }
    return out;
}

static cli_parm_t *cli_parm_lookup(char *stx, cli_parm_t *parm_table,
                                   cli_cmd_handl_fun cmd_fun, int *idx, BOOL *is_generic)
{
    int        i;
    cli_parm_t *parm;
    char stx_buf[MAX_WORD_LEN], *stx_start, *stx_end;

    misc_strncpyz(stx_buf, stx, sizeof(stx_buf));
    stx_start = stx_buf;
    /* Remove the brackets/paranthesis from the beginning and end of syntax */
    while ((*stx_start == '(') || (*stx_start == '[')) {
        stx_start++;
    }
    if ((stx_end = strchr(stx_start, ')')) != NULL) {
        *stx_end = '\0';
    }
    if ((stx_end = strchr(stx_start, ']')) != NULL) {
        *stx_end = '\0';
    }

    /* lookup module param table first */
    if (parm_table != NULL) {
        for (i = 0; parm_table[i].txt != NULL; i++) {
            parm = &parm_table[i];
            if ((strcmp(stx_start, parm->txt) == 0) &&
                (parm->cmd_fun == NULL || parm->cmd_fun == cmd_fun)) {
                *idx = i;
                *is_generic = FALSE;
                return parm;
            }
        }
    }

    if (parm_table != cli_parm_table) {
        /* now lookup param table in CLI */
        for (i = 0; cli_parm_table[i].txt != NULL; i++) {
            parm = &cli_parm_table[i];
            if ((strcmp(stx_start, parm->txt) == 0) &&
                (parm->cmd_fun == NULL || parm->cmd_fun == cmd_fun)) {
                *idx = i;
                *is_generic = TRUE;
                return parm;
            }
        }
    }

    return NULL;
}

/* Rule out those that didn't have an exact match */
static inline int cli_match_condense(int match_cmds[], int match_count, BOOL exact_match_for_this_lvl[])
{
    int i, new_match_count = 0;
    for (i = 0; i < match_count; i++) {
        if (exact_match_for_this_lvl[i]) {
            match_cmds[new_match_count++] = match_cmds[i];
        }
    }
    return new_match_count;
}

static char *cli_insuf_priv_lvl = "Insufficient privilege level";
static char *cli_syntax_get(cli_io_t *pIO, struct cli_cmd_t *cli_cmd)
{
    BOOL   rc1 = TRUE, rc2 = TRUE;

    /* check privilege level */
    if (cli_cmd->type != CLI_CMD_TYPE_NONE && cli_cmd->module_id != VTSS_MODULE_ID_NONE) {
#ifdef VTSS_SW_OPTION_PRIV_LVL
        if (cli_cmd->type == CLI_CMD_TYPE_CONF) {
            rc1 = vtss_priv_is_allowed_crw(cli_cmd->module_id, pIO->io.current_privilege_level);
            rc2 = vtss_priv_is_allowed_cro(cli_cmd->module_id, pIO->io.current_privilege_level);
        } else {
            rc1 = vtss_priv_is_allowed_srw(cli_cmd->module_id, pIO->io.current_privilege_level);
            rc2 = vtss_priv_is_allowed_sro(cli_cmd->module_id, pIO->io.current_privilege_level);
        }
#endif
        if (cli_cmd->rw_syntax && rc1) {
            return cli_cmd->rw_syntax;
        } else if (cli_cmd->ro_syntax && rc2) {
            return cli_cmd->ro_syntax;
        } else {
            return cli_insuf_priv_lvl;
        }
    } else {
        return cli_cmd->ro_syntax;
    }
}

/* Check for match between CLI command and syntax */
static BOOL cli_match_check(cli_io_t *pIO, int cmd_tbl_idx, char *stx_buf, char *stx_words[], char *cmd_words[], int cmd_count, int *i_parm, int exact_match_check_lvl, BOOL *exact_match)
{
    char      *stx, *cmd, cmd1_buf[MAX_WORD_LEN];
    int       stx_count, i_cmd, i_stx;
    struct cli_cmd_t *cli_cmd = &cli_cmd_tab_start[cmd_tbl_idx];

    /* Build array of syntax words */
    strcpy(stx_buf, cli_syntax_get(pIO, cli_cmd));
    if (!strcmp(stx_buf, cli_insuf_priv_lvl)) { /* Insufficient privilege level */
        return FALSE;
    }
    if (!cli_build_words(stx_buf, &stx_count, stx_words, 1)) {
        T_EG(VTSS_TRACE_GRP_VCLI, "Internal error - stc_buf = %s", stx_buf);
        return FALSE;
    }

    *exact_match = FALSE;
    for (i_cmd = 0, i_stx = 0; i_stx < stx_count; i_cmd++, i_stx++) {
        stx = stx_words[i_stx];
        /* Ignore specific symbol */
        while (*stx == '(') {
            stx++;
        }
        if (*stx == '<' || *stx == '[') {
            /* Parameters start here */
            *i_parm = i_stx;
            break;
        }

        if (i_cmd >= cmd_count) {
            continue;
        }

        cmd = cli_lower_word(cmd_words[i_cmd], cmd1_buf);
        if (strstr(stx, cmd) != stx) {
            /* Command word mismatch */
            return FALSE;
        } else if (i_cmd == exact_match_check_lvl && strcmp(stx, cmd) == 0) {
            *exact_match = TRUE;
        }
    }

    return TRUE;
}

/* Parse command */
static void cli_parse_command(cli_io_t *pIO)
{
    int32_t    size = &cli_cmd_tab_end - cli_cmd_tab_start;
    char       *cmd, *stx, *cmd2;
    char       c, cmd_buf[MAX_CMD_LEN], stx_buf[MAX_CMD_LEN], *cmd_words[MAX_WORD_CNT], *stx_words[MAX_WORD_CNT];
    char       cmd1_buf[MAX_WORD_LEN], cmd2_buf[MAX_WORD_LEN];
    int        i, i_cmd, i_stx, i_parm = 0, cmd_count, stx_count, max, len, j, error = 0, idx;
    int        match_cmds[size], match_count = 0, exact_match_cnt = 0, exact_match_check_lvl = 0;
    BOOL       exact_match_for_this_lvl[size], in_same_group = TRUE;
    struct cli_cmd_t  *cli_cmd;
    BOOL       help = 0;
    cli_req_t  req;
    cli_parm_t *parm;
    char       *ex_words[MAX_WORD_CNT], ex_str[MAX_CMD_LEN];
    int        ex_count;
    BOOL       root_lvl = FALSE;

    memset(match_cmds, 0, sizeof(match_cmds));
    memset(cmd1_buf, 0, sizeof(cmd1_buf));

    /* Read command and skip leading spaces */
    cmd = cmd_get(pIO);

    /* Remove CR */
    i = strlen(cmd);
    if (i) {
        cmd[i - 1] = '\0';
    }

    if (strcmp(cmd, "up") == 0 || strcmp(cmd, "Up") == 0) {
        for (i = 0, j = 0; ; i++) {
            c = pIO->cmd.cmd_group[i];
            if (c == '\0') {
                break;
            }
            if (isspace(c)) {
                j = i;
            }
        }
        pIO->cmd.cmd_group[j] = '\0';
        return;
    }

    /* Build array of command words */
    if (cmd[0] == '/') {
        if (cmd[1] == '\0') {
            /* Go to root level */
            pIO->cmd.cmd_group[0] = '\0';
            return;
        }
        root_lvl = TRUE;
        /* Do root level command */
        root_lvl = TRUE;
        cmd++;
        len = 0;
    } else {
        /* Prepend group words */
        strcpy(cmd_buf, pIO->cmd.cmd_group);
        len = strlen(cmd_buf);
        if (len) {
            cmd_buf[len++] = ' ';
        }
    }
    strcpy(&cmd_buf[len], cmd);
    if (!cli_build_words(cmd_buf, &cmd_count, cmd_words, 0)) {
        cli_puts("Command too long\n");
        return;
    }

    /* Remove trailing 'help' or '?' command */
    if (cmd_count > 1) {
        cmd = cli_lower_word(cmd_words[cmd_count - 1], cmd1_buf);
        if (strcmp(cmd, "?") == 0 || strcmp(cmd, "help") == 0) {
            cmd_count--;
            help = 1;
        }
    }

    /* Compare entered command with each entry in CLI command table */
    /* This is the first preliminary, coarse sorting */
    for (i = 0; i < size; i++) {
        cli_cmd = &cli_cmd_tab_start[i];
        if (cli_match_check(pIO, i, stx_buf, stx_words, cmd_words, cmd_count, &i_parm, exact_match_check_lvl, &exact_match_for_this_lvl[match_count])) {
            if (exact_match_for_this_lvl[match_count]) {
                exact_match_cnt++;
            }
            match_cmds[match_count++] = i;
        }
    }

    // If there's at least one exact match, rule out those commands that doesn't match exactly.
    while (match_count > 1 && exact_match_check_lvl < cmd_count) {
        if (exact_match_cnt > 0) {
            // At least one command matched exactly. Rule those out that weren't exact.
            match_count = cli_match_condense(match_cmds, match_count, exact_match_for_this_lvl);
            // At this point all commands at exact_match_check_lvl (0-based) are identical.
            if (match_count == 1) {
                break;
            }
        } else {
            // The user has abbreviated the command. This may be OK if all commands at
            // this level are the same.
            strcpy(stx_buf, cli_syntax_get(pIO, &cli_cmd_tab_start[match_cmds[0]]));
            if (!cli_build_words(stx_buf, &stx_count, stx_words, 1)) {
                T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
                return;
            }
            if (exact_match_check_lvl < stx_count) {
                strcpy(cmd1_buf, stx_words[exact_match_check_lvl]);
            } else {
                T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
            }
            for (i = 1; i < match_count; i++) {
                strcpy(stx_buf, cli_syntax_get(pIO, &cli_cmd_tab_start[match_cmds[i]]));
                if (!cli_build_words(stx_buf, &stx_count, stx_words, 1)) {
                    T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
                    return;
                }
                if (exact_match_check_lvl < stx_count) {
                    if (strcasecmp(cmd1_buf, stx_words[exact_match_check_lvl]) != 0) {
                        // Two or more command branches.
                        in_same_group = FALSE;
                        break;
                    }
                } else {
                    T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
                }
            }
            if (!in_same_group) {
                // At least two groups at this level were different. Stop and print.
                break;
            }
        }

        // If we get here, the user has either entered an exact command or an abbreviated
        // command of which there's only one branch in the command hierarchy. Either way,
        // match_count is > 1 and we gotta check the next level in the cmd hierarchy.
        if (++exact_match_check_lvl >= cmd_count) {
            break;
        }
        exact_match_cnt = 0;
        for (i = 0; i < match_count; i++) {
            if (cli_match_check(pIO, match_cmds[i], stx_buf, stx_words, cmd_words, cmd_count, &i_parm, exact_match_check_lvl, &exact_match_for_this_lvl[i])) {
                if (exact_match_for_this_lvl[i]) {
                    exact_match_cnt++;
                }
            } else {
                T_EG(VTSS_TRACE_GRP_VCLI, "Internal Error");
            }
        }
    }

    if (match_count == 0) {
        /* No matching commands */
        cli_puts("Invalid command\n");
    } else if (match_count == 1) {
        /* One matching command */
        cli_cmd = &cli_cmd_tab_start[match_cmds[0]];

        /* Rebuild array of syntax words */
        misc_strncpyz(stx_buf, cli_syntax_get(pIO, cli_cmd), sizeof(stx_buf));
        if (!cli_build_words(stx_buf, &stx_count, stx_words, 1)) {
            T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
            return;
        }

        /* Update i_parm for the matching command */
        i_parm = 0;
        (void)cli_match_check(pIO, match_cmds[0], stx_buf, stx_words, cmd_words, cmd_count,
                              &i_parm, 0, &exact_match_for_this_lvl[0]);

        /* Added additional checks for the syntax matched at some level. Ref. #Bugzilla 5834 */
        strcpy(ex_str, pIO->cmd.cmd_group);
        if (!root_lvl && cli_build_words(ex_str, &ex_count, ex_words, 1)) {
            for (i = 0; i < ex_count; i++) {
                if (strcmp(ex_words[i], stx_words[i])) {
                    cli_puts("Invalid Command\n");
                    return;
                }
            }
        }
        if (help) {
            uchar done[2][stx_count];
            BOOL is_generic;

            memset(done, 0, sizeof(done));
            cli_header_nl("Description", 0, 0);
            cli_printf("%s.\n\n", cli_cmd->descr);
            cli_header_nl("Syntax", 0, 0);
            cli_printf("%s\n\n", cli_syntax_get(pIO, cli_cmd));
            for (max = 0, i = 0; i_parm && i < 2; i++) {
                for (i_stx = i_parm; i_stx < stx_count; i_stx++) {
                    if ((parm = cli_parm_lookup(stx_words[i_stx], cli_cmd->parm_table,
                                                cli_cmd->cmd_fun, &idx, &is_generic)) == NULL) {
                        continue;
                    }
                    len = strlen(parm->txt);
                    if (i == 0) {
                        if (!(parm->flags & CLI_PARM_FLAG_NO_TXT)) {
                            if (len > max) {
                                max = len;
                            }
                        }

                    } else {
                        int indx;
                        for (indx = 0; done[is_generic][indx] != 0; indx++) {
                            if (done[is_generic][indx] == (idx + 1)) {
                                break;
                            }
                        }
                        if (done[is_generic][indx] == 0) {
                            done[is_generic][indx] = idx + 1; //indx can be 0, keep index+1
                            if (i_stx == i_parm) {
                                cli_header_nl("Parameters", 0, 0);
                            }
                            if (!(parm->flags & CLI_PARM_FLAG_NO_TXT)) {
                                cli_printf("%s", parm->txt);
                                for (j = len; j < max; j++) {
                                    cli_puts(" ");
                                }
                                cli_puts(": ");
                            }
                            cli_printf("%s\n", parm->help);
                        }
                    } //else
                } //inner for loop
            } //outer for loop
        } else {
            enum {
                CLI_STATE_IDLE,
                CLI_STATE_PARSING,
                CLI_STATE_DONE,
                CLI_STATE_ERROR
            } state;
            BOOL end = 0, separator, skip_parm;
            BOOL is_generic;
            char *mod_req = NULL;

            /* Create default parameters */
            if (!cli_req_default_set(&req)) {
                T_EG(VTSS_TRACE_GRP_VCLI, "Internal error");
                return;
            }

            mod_req = VTSS_MALLOC(cli_max_req_size);
            if (mod_req == NULL) {
                T_EG(VTSS_TRACE_GRP_VCLI, "Allocation failure");
                return;
            }

            memset(mod_req, 0, cli_max_req_size);
            req.module_req = mod_req;
            if (cli_cmd->def_fun) {
                cli_cmd->def_fun(&req);
            }

            /* Command doesn't need any parameter but user keyin */
            if (i_parm == 0 && cmd_count > stx_count) {
                T_DG(VTSS_TRACE_GRP_VCLI, "stx_count =%d, cmd_count = %d", stx_count, cmd_count);
                cli_printf("Invalid parameter: %s\n\n", cmd_words[stx_count]);
                cli_printf("Syntax:\n%s\n", cli_syntax_get(pIO, cli_cmd));
                if (mod_req) {
                    VTSS_FREE(mod_req);
                }
                return;
            }

            /* Parse arguments */
            state = CLI_STATE_IDLE;
            for (i_cmd = i_parm, i_stx = i_parm; i_parm && i_stx < stx_count; i_stx++) {
                stx = stx_words[i_stx];

                separator = (strcmp(stx, "|") == 0);
                skip_parm = 0;
                switch (state) {
                case CLI_STATE_IDLE:
                    if (stx[0] == '(' || stx[1] == '(') {
                        i = i_cmd;
                        state = CLI_STATE_PARSING;
                    }
                    break;
                case CLI_STATE_PARSING:
                    break;
                case CLI_STATE_ERROR:
                    if (end && separator) {
                        /* Parse next group */
                        i_cmd = i;
                        state = CLI_STATE_PARSING;
                    } else if (strstr(stx, ")]") != NULL) {
                        i_cmd = i;
                        state = CLI_STATE_IDLE;
                    }
                    skip_parm = 1;
                    break;
                case CLI_STATE_DONE:
                    if (stx[0] == '(' || stx[1] == '(') {
                        i = i_cmd;
                        state = CLI_STATE_PARSING;
                    } else if (end && !separator) {
                        state = CLI_STATE_IDLE;
                    } else {
                        skip_parm = 1;
                    }
                    break;
                default:
                    cli_printf("Illegal state: %d\n", state);
                    goto finally;
                }
                end = (strstr(stx, ")") != NULL);

                /* Skip if separator or not in parsing state */
                if (separator || skip_parm) {
                    continue;
                }

                /* Lookup parameter */
                if ((parm = cli_parm_lookup(stx, cli_cmd->parm_table,
                                            cli_cmd->cmd_fun, &idx, &is_generic)) == NULL) {
                    cli_printf("Unknown parameter: %s\n", stx);
                    goto finally;
                }

                if (i_cmd >= cmd_count) {
                    /* No more command words */
                    cmd = NULL;
                    error = 1;
                } else {
                    /* Parse command parameter */
                    do {
                        cmd = cli_lower_word(cmd_words[i_cmd], cmd1_buf);
                        cmd2 = ((i_cmd + 1) < cmd_count ?
                                cli_lower_word(cmd_words[i_cmd + 1], cmd2_buf) : NULL);
                        req.parm_parsed = 1; /* One parameter is parsed by default */
                        if (parm->parm_parse_fun) {
                            error = parm->parm_parse_fun(cmd, cmd2,
                                                         stx, cmd_words[i_cmd], &req);
                        } else {
                            T_EG(VTSS_TRACE_GRP_VCLI, "Internal error: Parameter with no parse func: %s", parm->txt);
                            goto finally;
                        }
                        T_DG(VTSS_TRACE_GRP_VCLI, "stx: %s, cmd: %s, error: %d", stx, cmd, error);
                        if (error) {
                            break;
                        }
                        if (parm->flags & CLI_PARM_FLAG_SET) {
                            req.set = 1;
                        }
                        i_cmd += req.parm_parsed;
                    } while (i_cmd < cmd_count && (parm->flags & CLI_PARM_FLAG_DUAL));
                }

                /* No error or error in optional parameter */
                if (!error ||
                    (stx[0] == '[' && (stx[1] != '(' || stx[2] == '['))) {
                    if (state == CLI_STATE_PARSING && end) {
                        state = CLI_STATE_DONE;
                    }
                    continue;
                }

                /* Error in mandatory parameter of group */
                if (state == CLI_STATE_PARSING) {
                    state = CLI_STATE_ERROR;
                    continue;
                }

                /* Error in mandatory parameter */
                if (cmd == NULL) {
                    cli_printf("Missing %s parameter\n\n", parm->txt);
                } else {
                    cli_printf("Invalid %s parameter: %s\n\n", parm->txt, cmd);
                }
                cli_printf("Syntax:\n%s\n", cli_syntax_get(pIO, cli_cmd));
                /*lint -e{438} ...I don't think 'i' is not used, even if it's ok */
                goto finally;
            } /* for loop */
            if (i_parm) {
                if (i_cmd < cmd_count) {
                    T_DG(VTSS_TRACE_GRP_VCLI, "i_cmd =%d, cmd_count = %d", i_cmd, cmd_count);
                    cli_printf("Invalid parameter: %s\n\n", cmd_words[i_cmd]);
                    cli_printf("Syntax:\n%s\n", cli_syntax_get(pIO, cli_cmd));
                    goto finally;
                }
                if (state == CLI_STATE_ERROR) {
                    cli_printf("Invalid parameter\n\n");
                    cli_printf("Syntax:\n%s\n", cli_syntax_get(pIO, cli_cmd));
                    goto finally;
                }
            } /* Parameter handling */

            /* Handle CLI command */
            if (cli_cmd->cmd_fun) {
                cli_cmd->cmd_fun(&req);
            } else {
                T_EG(VTSS_TRACE_GRP_VCLI, "Internal error: Command with no action routine: %s", cli_cmd->ro_syntax ? cli_cmd->ro_syntax : cli_cmd->rw_syntax);
                goto finally;
            }
finally:
            if (mod_req) {
                VTSS_FREE(mod_req);
            }
        }
    } else {
        /* Multiple matching commands */
        if (cmd_buf[0] == '\0' || match_count == (&cli_cmd_tab_end - cli_cmd_tab_start)) {
            cli_cmd_help(NULL);
        } else {
            stx = NULL; /* Satisfy compiler */
            if (in_same_group) {
                for (len = 0, i = 0, stx = cli_syntax_get(pIO, &cli_cmd_tab_start[match_cmds[0]]); ; i++) {
                    if (stx[i] == '\0') {
                        break;
                    }
                    if (isspace(stx[i])) {
                        len++;
                        if (len == cmd_count) {
                            break;
                        }
                    }
                }
                len = i;
            }

            if (!help && in_same_group && (stx != NULL) && strncmp(pIO->cmd.cmd_group, stx, ((strlen(pIO->cmd.cmd_group) > (u16)len) ? strlen(pIO->cmd.cmd_group) : len))) {
                /* Length of matching syntax of first command */
                cli_puts("Type 'up' to move up one level or '/' to go to root level\n");
                strncpy(pIO->cmd.cmd_group, stx, len);
                pIO->cmd.cmd_group[len] = '\0';
            } else {

#ifdef VTSS_CLI_SEC_GRP
                if (in_same_group) {
                    if ((cmd_count == 1) &&
                        (strncasecmp(VTSS_CLI_GRP_SEC, cmd_words[0], strlen(cmd_words[0])) == 0)) {
                        cli_cmd_sec_grp_disp();
                        return;
                    }
                    if ((cmd_count == 2) &&
                        (strncasecmp(VTSS_CLI_GRP_SEC, cmd_words[0], strlen(cmd_words[0])) == 0) &&
                        (strncasecmp(VTSS_CLI_GRP_SEC_SWITCH, cmd_words[1], strlen(cmd_words[1])) == 0)) {
                        cli_cmd_sec_switch_grp_disp();
                        return;
                    }
                    if ((cmd_count == 2) &&
                        (strncasecmp(VTSS_CLI_GRP_SEC, cmd_words[0], strlen(cmd_words[0])) == 0) &&
                        (strncasecmp(VTSS_CLI_GRP_SEC_NETWORK, cmd_words[1], strlen(cmd_words[1])) == 0)) {
                        cli_cmd_sec_network_grp_disp();
                        return;
                    }
                }
#endif

                cli_puts("Available Commands:\n\n");
                for (i = 0; i < match_count ; i++) {
                    cli_cmd = &cli_cmd_tab_start[match_cmds[i]];
                    // The following two lines were originally made as a cli_puts("%s\n", cli_cmd->syntax).
                    // Unfortunately, cli_telnet uses a stack-allocated buffer of 512 bytes to store the
                    // result of a vprintf(), so only the first 512 bytes of cli_cmd->syntax would end up
                    // on the telnet console (e.g. the ACL help text is longer).
                    // One solution would be to increase the buffer to e.g. 1024 bytes, but that would affect
                    // *all* threads' stacks since trace output also goes to cli_telnet and can happend from
                    // any thread.
                    // A better solution is to split the original cli_puts() into two statements as done below.
                    // The first one bypasses the vprintf() call and outputs the string directly (which is
                    // possible because the cli_cmd->syntax doesn't contain any %'s). The second prints the
                    // newline.
                    cli_puts(cli_syntax_get(pIO, cli_cmd));
                    cli_puts("\n");
                }
            }
        }
    }
} /* cli_parse_command */

static int cmd_tab_cmp_fun(const void *p1, const void *p2)
{
    const struct cli_cmd_t *c1 = (struct cli_cmd_t *)p1;
    const struct cli_cmd_t *c2 = (struct cli_cmd_t *)p2;
    char *s1, *s2;

    if (c1->sort_key != c2->sort_key) {
        return (c1->sort_key - c2->sort_key);
    }

    /* either rw_syntax or ro_syntax must be present, both can not be NULL */
    s1 = (c1->rw_syntax ? c1->rw_syntax : c1->ro_syntax);
    s2 = (c2->rw_syntax ? c2->rw_syntax : c2->ro_syntax);
    return (strcasecmp(s1, s2));
}

/* Sorting cmd group table, cmd table and parameter table */
static void cli_cmd_table_sort(void)
{
    qsort(cli_cmd_tab_start, (&cli_cmd_tab_end - cli_cmd_tab_start),
          sizeof(struct cli_cmd_t), cmd_tab_cmp_fun);
}

/****************************************************************************/
/*  Public functions                                                        */
/****************************************************************************/
void vcli_cmd_parse_init(cli_iolayer_t *pIO)
{
    cli_io_t *p = (cli_io_t *)pIO;

    p->cmd.cmd_len = 0;
    p->cmd.cmd_cursor = 0;
    p->cmd.cmd_history.len = 0;
    p->cmd.cmd_history.idx = 0;
    p->cmd.cmd_history.scroll = 0;
    p->cmd.cmd_group[0] = '\0';
    p->cmd.usid = VTSS_USID_ALL;
}

vtss_rc vcli_cmd_parse_and_exec(cli_iolayer_t *pIO)
{
    cli_io_t *p = (cli_io_t *)pIO;

    prompt(p);
    while (1) {
        vtss_rc rc;
        if ((rc = cmd_ready(p)) == VTSS_OK) {
            cli_stack_info_get(p);
            cli_parse_command(p);
            return rc;
        } else if (rc == VTSS_INCOMPLETE) {
            continue;
        } else {
            return rc;
        }
    }
}

vtss_rc vcli_cmd_exec(char *cmd)
{
    cli_iolayer_t *pIO = cli_get_io_handle();
    cli_io_t      io;
    unsigned int  len;

    if (!cmd || !cmd[0]) {
        T_IG(VTSS_TRACE_GRP_VCLI, "NULL or empty vCLI command!");
        return VTSS_INVALID_PARAMETER;
    }

    len = strlen(cmd);
    if (len > (sizeof(io.cmd.cmd_buf) - 2)) { // Make sure there is room for <cr> and NULL
        T_IG(VTSS_TRACE_GRP_VCLI, "vCLI command too long!");
        return VTSS_INVALID_PARAMETER;
    }
    memset(&io, 0, sizeof(io));
    io.io = *pIO; // Make a copy of iolayer
    strcpy(io.cmd.cmd_buf, cmd); // We have already checked for enough space!
    io.cmd.cmd_buf[len] = CR; // cli_parse_command() expects this

    cli_stack_info_get(&io);
    cli_parse_command(&io);
    return VTSS_OK;
}

void vcli_banner_exec(cli_iolayer_t *pIO)
{
    cli_printf("\nWelcome to %s Command Line Interface (%s).\n", CLI_CUSTOM_VENDOR_NAME, VTSS_CLI_CUSTOM_VERSION);
    cli_puts("Type 'help' or '?' to get help.\n");
    cli_cmd_port_numbers();

#ifdef VTSS_SW_OPTION_SYSLOG
    if (vtss_switch_mgd()) {
        int syslog_entry_cnt;
        if ((syslog_entry_cnt = syslog_flash_entry_cnt(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR)) != 0) {
            if (syslog_entry_cnt == 1) {
                cli_puts("\nThere is 1 error entry in the syslog. Type \"debug syslog show\" to display it.\n");
            } else {
                cli_printf("\nThere are %d error entries in the syslog. Type \"debug syslog show\" to display them.\n", syslog_entry_cnt);
            }
        }
    }
#endif /* VTSS_SW_OPTION_SYSLOG */
}

void cli_req_size_register(cyg_uint32 size)
{
    cli_max_req_size = MAX(cli_max_req_size, size);
}

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void vcli_init(void)
{
    (void)snprintf(cli_sid_help_str, sizeof(cli_sid_help_str), "Switch ID (%d-%d)", VTSS_USID_START, VTSS_USID_END - 1);

    strcpy(vcli_prompt, ">");
    cli_cmd_table_sort();

}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
