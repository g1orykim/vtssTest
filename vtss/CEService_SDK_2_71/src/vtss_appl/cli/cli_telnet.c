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

#include <stdio.h>
#include <stdlib.h>
#include <network.h>

#include "main.h"
#include "cli.h"
#include "cli_trace_def.h"
#include "sysutil_api.h"
#include "control_api.h"
#include "vtss_trace_api.h"
#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif /* VTSS_SW_OPTION_AUTH */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_CLI

#define CLI_TELNET_MAX_CLIENT       4
#define CLI_TELNET_PORT             23       /* Default telnet port */
#define CLI_TELNET_THREAD_NAME_MAX  16       /* Maximum thread name */

static char cli_telnet_parent_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_thread cli_telnet_parent_thread_data;
static cyg_handle_t cli_telnet_parent_thread_handle;

static char cli_telnet_child_name[CLI_TELNET_MAX_CLIENT][CLI_TELNET_THREAD_NAME_MAX];
static char cli_telnet_child_stack[CLI_TELNET_MAX_CLIENT][CLI_STACK_SIZE];
static cyg_thread cli_telnet_child_data[CLI_TELNET_MAX_CLIENT];
static cyg_handle_t cli_telnet_child_handle[CLI_TELNET_MAX_CLIENT];

// Special characters used by Telnet - must be interpretted here
#define TELNET_IAC    0xFF // Interpret as command (escape)
#define TELNET_IP     0xF4 // Interrupt process
#define TELNET_WILL   0xFB // I Will do XXX
#define TELNET_WONT   0xFC // I Won't do XXX
#define TELNET_DO     0xFD // Will you XXX
#define TELNET_DONT   0xFE // Don't you XXX

#define TELNET_OPT_ECHO                     1
#define TELNET_OPT_SUPPRESS_GO_AHEAD        3
#define TELNET_OPT_STATUS                   5
#define TELNET_OPT_TIMING_MARK              6
#define TELNET_OPT_TERMINAL_TYPE            24
#define TELNET_OPT_WINDOW_SIZE              31
#define TELNET_OPT_TERMINAL_SPEED           32
#define TELNET_OPT_REMOTE_FLOW_CONTROL      33
#define TELNET_OPT_LINEMODE                 34
#define TELNET_OPT_ENVIRONMENT_VARIABLES    36

#ifdef VTSS_SW_OPTION_IPV6
# define PEXIT(m)                         \
    do {                                  \
        if (s       >= 0) close(s);       \
        if (conn    >= 0) close(conn);    \
        if (s6      >= 0) close(s6);      \
        if (conn_v6 >= 0) close(conn_v6); \
        T_EG(VTSS_TRACE_GRP_TELNET, m);   \
        return;                           \
    } while(0)
#else
# define PEXIT(m)                         \
    do {                                  \
        if (s    >= 0) close(s);          \
        if (conn >= 0) close(conn);       \
        T_EG(VTSS_TRACE_GRP_TELNET, m);   \
        return;                           \
    } while(0)
#endif /* VTSS_SW_OPTION_IPV6 */

static const u8 telnet_opts[] = { TELNET_IAC, TELNET_WILL, TELNET_OPT_ECHO };
static unsigned long telnet_security_mode = 0;

/*lint -sem(telnet_trace_flush, thread_protected) */
/*lint -sem(telnet_trace_putchar, thread_protected) */
/*lint -sem(telnet_trace_vprintf, thread_protected) */
/*lint -sem(telnet_trace_write_string, thread_protected) */
/*lint -sem(cli_telnet_do_close, thread_protected) */
/*lint -sem(cli_telnet, thread_protected) */
/*lint -sem(cli_telnet_create_child_thread, thread_protected) */

/* CLI Telnet IO layer */

typedef struct cli_io_telnet {
    cli_io_t                base;
    int                     listen_fd;
    BOOL                    trace_registered;
    vtss_trace_io_t         *trace_layer;
    uint                    trace_reg;
    struct {
        int                 out_buflen;
        unsigned char       out_buf[1024];
        unsigned char       *out_bufp;
    } io;
    BOOL                    valid;
    char                    prev_ch;
} cli_io_telnet_t;

/* Raw Telnet IO layer */

static void __raw_flush(cli_io_telnet_t *pTIO)
{
    u8 *bp = pTIO->io.out_buf;
    if (pTIO->base.io.fd < 0) {
        return;
    }
    while (pTIO->io.out_buflen && !pTIO->base.io.bIOerr) {
        int n = write(pTIO->base.io.fd, bp, pTIO->io.out_buflen);
        if (n < 0) {
            pTIO->base.io.bIOerr = TRUE;
        } else {
            pTIO->io.out_buflen -= n;
            bp += n;
        }
    }
    pTIO->io.out_bufp = &pTIO->io.out_buf[0];
    pTIO->io.out_buflen = 0;
}

static void __raw_putch(cli_io_telnet_t *pTIO, unsigned char c)
{
    *(pTIO->io.out_bufp++) = c;
    if ( ++pTIO->io.out_buflen == sizeof(pTIO->io.out_buf) ) {
        __raw_flush(pTIO);
    }
}

#define __raw_getch(_pIO_, _timeout_, _ch_) do {                                                \
    vtss_rc _rc_;                                                                               \
    T_NG(VTSS_TRACE_GRP_TELNET, "Telnet timeout %d", _timeout_);                                                        \
    _rc_ = cli_io_getch(_pIO_, _timeout_, _ch_);                                                \
    if (_rc_ != VTSS_OK) {                                                                      \
        T_NG(VTSS_TRACE_GRP_TELNET, "Telnet err %s (%d)", error_txt(_rc_), _rc_);                                       \
        return _rc_;                                                                            \
    }                                                                                           \
    T_NG(VTSS_TRACE_GRP_TELNET, "Telnet got char '%c' (%u)", ((*_ch_ > 31) && (*_ch_ < 127)) ? *_ch_ : '?', (u8)*_ch_); \
} while (0)

/* Real Telnet IO layer */
static void cli_io_telnet_init(cli_iolayer_t *pIO)
{
    cli_io_telnet_t *pTIO = (cli_io_telnet_t *) pIO;
    pIO->bIOerr = FALSE;
    pIO->bEcho = FALSE;
    pIO->authenticated = FALSE;
    pTIO->io.out_bufp = &pTIO->io.out_buf[0];
    pTIO->io.out_buflen = 0;
    if (!pTIO->trace_registered) {
        if (vtss_trace_io_register(pTIO->trace_layer, VTSS_MODULE_ID_CLI, &pTIO->trace_reg) == VTSS_OK) {
            pTIO->trace_registered = TRUE;
        } else {
            T_EG(VTSS_TRACE_GRP_TELNET, "Unable to register trace!");
        }
    } else {
        T_EG(VTSS_TRACE_GRP_TELNET, "Trace already registered!");
    }
}

static void cli_io_telnet_putchar(cli_iolayer_t *pIO, char ch)
{
    __raw_putch((cli_io_telnet_t *) pIO, ch);
    if (ch == '\n') {
        __raw_putch((cli_io_telnet_t *) pIO, '\r');
    }
    __raw_flush((cli_io_telnet_t *) pIO);
}

static void cli_io_telnet_puts(cli_iolayer_t *pIO, const char *str)
{
    while ( *str ) {
        cli_io_telnet_putchar(pIO, *str++);
    }
}

static int cli_io_telnet_vprintf(cli_iolayer_t *pIO, const char *fmt, va_list ap)
{
    char buf[1024];
    int l, i;
    l = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    for (i = 0; i < l; i++) {
        cli_io_telnet_putchar(pIO, buf[i]);
    }
    return l;
}

static int cli_io_telnet_getch(cli_iolayer_t *pIO, int timeout, char *ch)
{
    cli_io_telnet_t *pTIO = (cli_io_telnet_t *) pIO;
    char prev_ch, command, option;
    u8 resp;

    while (TRUE) {
        __raw_getch(pIO, timeout, ch);

        prev_ch = pTIO->prev_ch;
        pTIO->prev_ch = *ch;

        if ((prev_ch == CR) && (*ch == 0)) { /* RFC 854 specifies that a NUL received after a CR should be stripped out */
            T_NG(VTSS_TRACE_GRP_TELNET, "Telnet NUL stripped");
            continue;  /* Get next char */
        }

        if ((u8)*ch != TELNET_IAC) {
            return VTSS_OK;
        }

        // Telnet escape - get the command
        __raw_getch(pIO, timeout, &command);
        T_NG(VTSS_TRACE_GRP_TELNET, "Telnet got command %u", (u8)command);

        switch ((u8)command) {
        case TELNET_IAC:
            // The other special case - escaped escape
            *ch = command;
            return VTSS_OK;
        case TELNET_IP:
            // Just in case the other end needs synchronizing
            __raw_putch(pTIO, TELNET_IAC);
            __raw_putch(pTIO, TELNET_WONT);
            __raw_putch(pTIO, TELNET_OPT_TIMING_MARK);
            __raw_flush(pTIO);
            *ch = 0x03; // Special case for ^C == Interrupt Process
            return VTSS_OK;
        case TELNET_DO:
            // Telnet DO option
            __raw_getch(pIO, timeout, &option);
            T_NG(VTSS_TRACE_GRP_TELNET, "Telnet got option %u", (u8)option);
            resp = TELNET_WONT;     /* Default to WONT */
            switch ((u8)option) {
            case TELNET_OPT_ECHO:      /* Will echo */
                pTIO->base.io.bEcho = TRUE;
                T_DG(VTSS_TRACE_GRP_TELNET, "Telnet ECHO ON");
                /* we send "will echo" in initial state (telnet_opts) - no response required here */
                continue; /* Get next char */
            case TELNET_OPT_SUPPRESS_GO_AHEAD: /* Will Suppress */
                resp = TELNET_WILL;
                break;
            }
            T_DG(VTSS_TRACE_GRP_TELNET, "Telnet: DO %d -> %s", option, resp == TELNET_WILL ? "WILL" : "WONT");
            // Respond
            __raw_putch(pTIO, TELNET_IAC);
            __raw_putch(pTIO, resp);
            __raw_putch(pTIO, option);
            __raw_flush(pTIO);
            continue; /* Get next char */
        case TELNET_WILL:
            // Telnet WILL option
            __raw_getch(pIO, timeout, &option);
            T_NG(VTSS_TRACE_GRP_TELNET, "Telnet got option %u", (u8)option);
            resp = TELNET_DONT;     /* Default to WONT */
            switch ((u8)option) {
            case TELNET_OPT_SUPPRESS_GO_AHEAD: /* Do Suppress */
                resp = TELNET_DO;
                break;
            }
            T_DG(VTSS_TRACE_GRP_TELNET, "Telnet: WILL %d -> %s", option, resp == TELNET_DO ? "DO" : "DONT");
            // Respond
            __raw_putch(pTIO, TELNET_IAC);
            __raw_putch(pTIO, resp);
            __raw_putch(pTIO, option);
            __raw_flush(pTIO);
            continue; /* Get next char */
        case TELNET_WONT:
        case TELNET_DONT:
            __raw_getch(pIO, timeout, &option);
            T_NG(VTSS_TRACE_GRP_TELNET, "Telnet got option %u", (u8)option);
            continue; /* Get next char */
        default:
            continue; /* Get next char */
        }
    }
}

static void cli_io_telnet_flush(cli_iolayer_t *pIO)
{
    __raw_flush((cli_io_telnet_t *) pIO);
}

static void cli_io_telnet_close(cli_iolayer_t *pIO)
{
    cli_io_telnet_t *pTIO = (cli_io_telnet_t *) pIO;

    if (pIO->fd >= 0) {
        __raw_flush(pTIO);
        close(pIO->fd);
        pIO->fd = -1;
        pIO->bIOerr = TRUE;
        pTIO->valid = FALSE;
        if (pTIO->trace_registered) {
            (void)vtss_trace_io_unregister(&pTIO->trace_reg);
            pTIO->trace_registered = FALSE;
        }
        /* Terminate client thread */
        cyg_thread_exit();
    }
}

#ifdef VTSS_SW_OPTION_AUTH
BOOL cli_io_telnet_login(struct cli_iolayer *pIO)
{
    return cli_io_login(pIO, VTSS_AUTH_AGENT_TELNET, CLI_PASSWD_CHAR_TIMEOUT);
}
#endif /* VTSS_SW_OPTION_AUTH */

static cli_io_telnet_t cli_io_telnet[CLI_TELNET_MAX_CLIENT];

static cli_io_telnet_t cli_io_telnet_default = {
    .base = {
        .io = {
            .cli_init = cli_io_telnet_init,
            .cli_getch = cli_io_telnet_getch,
            .cli_vprintf = cli_io_telnet_vprintf,
            .cli_putchar = cli_io_telnet_putchar,
            .cli_puts = cli_io_telnet_puts,
            .cli_flush = cli_io_telnet_flush,
            .cli_close = cli_io_telnet_close,
#ifdef VTSS_SW_OPTION_AUTH
            .cli_login = cli_io_telnet_login,
#endif /* VTSS_SW_OPTION_AUTH */
            .fd = -1,
            .char_timeout = CLI_COMMAND_CHAR_TIMEOUT,
            .cDEL = CLI_DEL_KEY_LINUX, /* Assume telnet from Linux */
            .cBS  = CLI_BS_KEY_LINUX,  /* Assume telnet from Linux */
            .current_privilege_level = 15,
#if defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI)
            .parser = CLI_PARSER_DEFAULT,
#endif /* defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI) */
#ifdef VTSS_SW_OPTION_ICLI
            .session_way = CLI_WAY_TELNET,
#endif /* VTSS_SW_OPTION_ICLI */
        },
    },
};

/*
 * Telnet trace layer
 */

static int telnet_trace_get_child_index(void)
{
    int child_index;
    cyg_handle_t handle;

    handle = cyg_thread_self();
    for (child_index = 0; child_index < CLI_TELNET_MAX_CLIENT; child_index++) {
        if (handle == cli_telnet_child_handle[child_index]) {
            return child_index;
        }
    }
    return -1;
}

static void telnet_trace_putchar(struct _vtss_trace_io_t *pIO, char ch)
{
    int child_index = telnet_trace_get_child_index();
    if (child_index >= 0) {
        cli_io_telnet_putchar(&cli_io_telnet[child_index].base.io, ch);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < CLI_TELNET_MAX_CLIENT; child_index++) {
            if (cli_io_telnet[child_index].base.io.fd >= 0) {
                (void) cli_io_telnet_putchar(&cli_io_telnet[child_index].base.io, ch);
            }
        }
    }
}

static int telnet_trace_vprintf(struct _vtss_trace_io_t *pIO, const char *fmt, va_list ap)
{
    int child_index = telnet_trace_get_child_index();
    if (child_index >= 0) {
        return cli_io_telnet_vprintf(&cli_io_telnet[child_index].base.io, fmt, ap);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < CLI_TELNET_MAX_CLIENT; child_index++) {
            if (cli_io_telnet[child_index].base.io.fd >= 0) {
                (void) cli_io_telnet_vprintf(&cli_io_telnet[child_index].base.io, fmt, ap);
            }
        }
    }
    return 1;
}

static void telnet_trace_write_string(struct _vtss_trace_io_t *pIO, const char *str)
{
    int child_index = telnet_trace_get_child_index();
    if (child_index >= 0) {
        cli_io_telnet_puts(&cli_io_telnet[child_index].base.io, str);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < CLI_TELNET_MAX_CLIENT; child_index++) {
            if (cli_io_telnet[child_index].base.io.fd >= 0) {
                (void) cli_io_telnet_puts(&cli_io_telnet[child_index].base.io, str);
            }
        }
    }
}

static void telnet_trace_flush(struct _vtss_trace_io_t *pIO)
{
    int child_index = telnet_trace_get_child_index();
    if (child_index >= 0) {
        cli_io_telnet_flush(&cli_io_telnet[child_index].base.io);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < CLI_TELNET_MAX_CLIENT; child_index++) {
            if (cli_io_telnet[child_index].base.io.fd >= 0) {
                (void) cli_io_telnet_flush(&cli_io_telnet[child_index].base.io);
            }
        }
    }
}

static vtss_trace_io_t telnet_trace_layer = {
    .trace_putchar = telnet_trace_putchar,
    .trace_vprintf = telnet_trace_vprintf,
    .trace_write_string = telnet_trace_write_string,
    .trace_flush = telnet_trace_flush,
};

/*
 * CLI Initialization
 */

static void cli_telnet_create_child_thread(int ix)
{
    sprintf(cli_telnet_child_name[ix], "Telnet CLI %01d", ix + 1);

    // Create a child thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(THREAD_DEFAULT_PRIO,                       // Priority
                      cli_thread,                                // entry
                      (cyg_addrword_t) &cli_io_telnet[ix].base,  // entry parameter
                      cli_telnet_child_name[ix],                 // Name
                      &cli_telnet_child_stack[ix][0],            // Stack
                      sizeof(cli_telnet_child_stack[ix]),        // Size
                      &cli_telnet_child_handle[ix],              // Handle
                      &cli_telnet_child_data[ix]                 // Thread data structure
                     );
    cyg_thread_resume(cli_telnet_child_handle[ix]);              // Start it
}

#define CLI_TELNET_BUF_SIZE     64

static void cli_telnet(cyg_addrword_t data)
{
    int                     s = -1;
    int                     conn = -1;
    socklen_t               client_len;
    struct sockaddr_in      client_addr, local;
    int                     i,
                            one = 1;
    int                     found_empty;
    fd_set                  rfds;
    cyg_int32               rc;
    cyg_int32               fdmax;
#ifdef VTSS_SW_OPTION_IPV6
    int                     s6 = -1;
    int                     conn_v6 = -1;
    socklen_t               client_len_v6;
    struct sockaddr_in6     client_addr_v6, local_v6;
#endif /* VTSS_SW_OPTION_IPV6 */

    /* for IPv4 */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        PEXIT("IPv4 CLI telnet stream socket");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        PEXIT("IPv4 CLI telnet setsockopt SO_REUSEADDR");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        PEXIT("IPv4 CLI telnet setsockopt SO_REUSEPORT");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(CLI_TELNET_PORT);
    local.sin_addr.s_addr = INADDR_ANY;
    while (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
        //diag_printf("bind error, sleeping\n");
        VTSS_OS_MSLEEP(3000);
    }
    (void)listen(s, CLI_TELNET_MAX_CLIENT);
    FD_ZERO(&rfds);
    fdmax = s;

#ifdef VTSS_SW_OPTION_IPV6
    /* for IPv6 */
    s6 = socket(AF_INET6, SOCK_STREAM, 0);
    if (s6 < 0) {
        PEXIT("IPv4 CLI telnet stream socket");
    }
    if (setsockopt(s6, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one))) {
        PEXIT("IPv4 CLI telnet setsockopt IPV6_V6ONLY");
    }
    memset(&local_v6, 0, sizeof(local_v6));
    local_v6.sin6_family = AF_INET6;
    local_v6.sin6_len = sizeof(local_v6);
    local_v6.sin6_port = htons(CLI_TELNET_PORT);
    local_v6.sin6_addr.__u6_addr.__u6_addr32[0] = INADDR_ANY;
    local_v6.sin6_addr.__u6_addr.__u6_addr32[1] = INADDR_ANY;
    local_v6.sin6_addr.__u6_addr.__u6_addr32[2] = INADDR_ANY;
    local_v6.sin6_addr.__u6_addr.__u6_addr32[3] = INADDR_ANY;
    while (bind(s6, (struct sockaddr *) &local_v6, sizeof(struct sockaddr_in6)) < 0) {
        //diag_printf("bind error, sleeping\n");
        VTSS_OS_MSLEEP(3000);
    }
    (void)listen(s6, CLI_TELNET_MAX_CLIENT);
    fdmax = MAX(s, s6);
#endif /* VTSS_SW_OPTION_IPV6 */

    while (TRUE) {
        /* Disable wrong lint warnings for FD_SET() and FD_ISSET() in this block */
        /*lint --e{573,661,662} */
        FD_SET(s, &rfds);
#ifdef VTSS_SW_OPTION_IPV6
        FD_SET(s6, &rfds);
#endif /* VTSS_SW_OPTION_IPV6 */
        struct timeval tv = {300, 0};
        rc = select(fdmax + 1, &rfds, NULL, NULL, &tv);
        if ((rc > 0) && (FD_ISSET(s, &rfds))) {
            /* for IPv4 */
            client_len = sizeof(client_addr);
            if ((conn = accept(s, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                PEXIT("accept");
            }
            T_IG(VTSS_TRACE_GRP_TELNET, "connection(%d) from %s:%d", conn, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            /* Notice if connection break */
            if (setsockopt(conn, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one))) {
                PEXIT("setsockopt");
            }

            if (telnet_security_mode) {
                char *buf = "This device's SSH mode is enabled, please use the SSH instead of Telnet.\r\n";
                T_IG(VTSS_TRACE_GRP_TELNET, "Extra connection(%d) from %s:%d", conn,
                     inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write(conn, buf, strlen(buf));
                VTSS_OS_MSLEEP(50);  /* Allow to drain */
                close(conn);
                conn = -1;
            } else {
                /* Found empty entry */
                found_empty = 0;
                for (i = 0; i < CLI_TELNET_MAX_CLIENT; i++) {
                    if (!cli_io_telnet[i].valid) {

                        found_empty = 1;
                        cli_io_telnet[i].valid = TRUE;
                        cli_io_telnet[i].base.io.fd = conn;
                        cli_io_telnet[i].listen_fd = s;

                        /* cli_io_telnet_init() will register trace */
                        cli_io_telnet[i].trace_registered = FALSE;
                        cli_io_telnet[i].trace_layer = &telnet_trace_layer;

                        /* Send initial options */
                        write(conn, telnet_opts, sizeof(telnet_opts));

#ifdef VTSS_SW_OPTION_ICLI
                        /* Save client address */
                        memcpy(cli_io_telnet[i].base.io.client_addr, &client_addr, client_len);
#endif /* VTSS_SW_OPTION_ICLI */

                        /* Borrow thread - code only */
                        cli_telnet_create_child_thread(i);
                        break;
                    }
                }

                if (found_empty == 0) {
                    static char *buf;
                    T_IG(VTSS_TRACE_GRP_TELNET, "Extra connection(%d) from %s:%d", conn,
                         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    if ((buf = VTSS_MALLOC(CLI_TELNET_BUF_SIZE + 1))) {
                        int len = snprintf( buf, CLI_TELNET_BUF_SIZE,
                                            "Only %d connections allowed.\r\n",
                                            CLI_TELNET_MAX_CLIENT);
                        write(conn, buf, len);
                        VTSS_OS_MSLEEP(50);  /* Allow to drain */
                        VTSS_FREE(buf);
                    }
                    close(conn);
                    conn = -1;
                }
            }
        }
#ifdef VTSS_SW_OPTION_IPV6
        else if ((rc > 0) && (FD_ISSET(s6, &rfds))) {
            /* for IPv6 */
            client_len_v6 = sizeof(client_addr_v6);
            if ((conn_v6 = accept(s6, (struct sockaddr *)&client_addr_v6, &client_len_v6)) < 0) {
                PEXIT("accept");
            }

            if (telnet_security_mode) {
                char *buf = "This device's SSH mode is enabled, please use the SSH instead of Telnet.\r\n";
                write(conn_v6, buf, strlen(buf));
                VTSS_OS_MSLEEP(50);  /* Allow to drain */
                close(conn_v6);
                conn_v6 = -1;
            } else {
                /* Found empty entry */
                found_empty = 0;
                for (i = 0; i < CLI_TELNET_MAX_CLIENT; i++) {
                    if (!cli_io_telnet[i].valid) {
                        found_empty = 1;
                        cli_io_telnet[i].valid = TRUE;
                        cli_io_telnet[i].base.io.fd = conn_v6;
                        cli_io_telnet[i].listen_fd = s6;

                        /* cli_io_telnet_init() will register trace */
                        cli_io_telnet[i].trace_registered = FALSE;
                        cli_io_telnet[i].trace_layer = &telnet_trace_layer;

                        /* Send initial options */
                        write(conn_v6, telnet_opts, sizeof(telnet_opts));

#ifdef VTSS_SW_OPTION_ICLI
                        /* Save client address */
                        memcpy(cli_io_telnet[i].base.io.client_addr, &client_addr_v6, client_len_v6);
#endif /* VTSS_SW_OPTION_ICLI */

                        /* Borrow thread - code only */
                        cli_telnet_create_child_thread(i);
                        break;
                    }
                }

                if (!found_empty) {
                    static char *buf;
                    if ((buf = VTSS_MALLOC(CLI_TELNET_BUF_SIZE + 1))) {
                        int len = snprintf(buf, CLI_TELNET_BUF_SIZE,
                                           "Only %d connections allowed.\r\n",
                                           CLI_TELNET_MAX_CLIENT);
                        write(conn_v6, buf, len);
                        VTSS_OS_MSLEEP(50);  /* Allow to drain */
                        VTSS_FREE(buf);
                    }
                    close(conn_v6);
                    conn_v6 = -1;
                }
            }
        }
        fdmax = MAX(s , s6);
#endif /* VTSS_SW_OPTION_IPV6 */
    }
}

static void cli_telnet_do_close(vtss_restart_t restart)
{
    int i, active = 0;
    for (i = 0; i < CLI_TELNET_MAX_CLIENT; i++) {
        if (cli_io_telnet[i].valid && cli_io_telnet[i].base.io.authenticated) {
            cli_io_telnet[i].base.io.bIOerr = TRUE; /* Force all sessions to terminate themselves */
            active++;
        }
    }
    if (active) {
        T_IG(VTSS_TRACE_GRP_TELNET, "%d Telnet session%s terminated!", active, (active > 1) ? "s" : "");
        VTSS_OS_MSLEEP(CLI_GETCH_WAKEUP * 2); /* Give the sessions a little time to terminate */
    }
    /*
     * We have previously called cyg_thread_delete() when we want to force a thread to be closed.
     * This will remove the thread on the scheduler list (and make it impossible to query the thread for e.g stack usage).
     * It will NOT free any memory that is assigned to the thread, as it is statically allocated.
     * It is perfectly ok to recreate a cli thread using the existing handle.
     */
}

void cli_telnet_close(void)
{
    cli_telnet_do_close(VTSS_RESTART_COLD /* Unused */);
}

void cli_telnet_init(void)
{
    int i;

    // Set default configuration
    for (i = 0; i < CLI_TELNET_MAX_CLIENT; i++) {
        cli_io_telnet[i] = cli_io_telnet_default;
    }

    // Get warning about system resets
    control_system_reset_register(cli_telnet_do_close);
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(THREAD_DEFAULT_PRIO,                  // Priority
                      cli_telnet,                           // entry
                      0,                                    // entry parameter
                      "Telnet CLI Main",                    // Name
                      &cli_telnet_parent_stack[0],          // Stack
                      sizeof(cli_telnet_parent_stack),      // Size
                      &cli_telnet_parent_thread_handle,     // Handle
                      &cli_telnet_parent_thread_data        // Thread data structure
                     );
    cyg_thread_resume(cli_telnet_parent_thread_handle);  // Start it
}

#ifdef TELNET_SECURITY_SUPPORTED
/* Set TELNET security mode. When secrity mode is enabled,
   we should use the SSH instead of Telnet and disconnect all existing telnet sessions */
void telnet_set_security_mode(unsigned long security_mode)
{
    telnet_security_mode = security_mode;
    if (security_mode) {
        cli_telnet_close(); //disconnect all existing telnet sessions
    }
}
#endif /* TELNET_SECURITY_SUPPORTED */
