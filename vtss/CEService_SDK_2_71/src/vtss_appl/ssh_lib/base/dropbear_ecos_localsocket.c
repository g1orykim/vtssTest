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
#include "dropbear_ecos.h"
#ifdef DROPBEAR_NO_SHELL

#include <stdio.h>
#include <stdlib.h>
#include <network.h>

#include "main.h"
#include "cli.h"
#include "cli_trace_def.h"
#include "sysutil_api.h"
#include "control_api.h"
#include "vtss_trace_api.h"
#include "ssh_lib_callout.h"

#define DROPBEAR_ECOS_LOCAL_SOCKET_THREAD_NAME_MAX  16       /* Maximum thread name */
#define PEXIT(s) do { T_E(s); return; } while(0)

#define CYG_DROPBEAR_MSEC2TICK(msec) (msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000))

static int dropbear_local_socket_fd = -1;

static char local_socket_parent_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_thread local_socket_parent_thread_data;
static cyg_handle_t local_socket_parent_thread_handle;

static char local_socket_child_name[DROPBEAR_MAX_SOCK_NUM][DROPBEAR_ECOS_LOCAL_SOCKET_THREAD_NAME_MAX];
static char local_socket_child_stack[DROPBEAR_MAX_SOCK_NUM][CLI_STACK_SIZE];
static cyg_thread local_socket_child_data[DROPBEAR_MAX_SOCK_NUM];
static cyg_handle_t local_socket_child_handle[DROPBEAR_MAX_SOCK_NUM];

static int cli_get_child_index(void);

/* CLI SSH IO layer */

typedef struct cli_io_ssh {
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
} cli_io_ssh_t;

static void __raw_flush(cli_io_ssh_t *pTIO)
{
    char *bp = pTIO->io.out_buf;
    if (pTIO->base.io.fd < 0) {
        return;
    }
    while(pTIO->io.out_buflen && !pTIO->base.io.bIOerr) {
        int n = write(pTIO->base.io.fd, bp, pTIO->io.out_buflen);
        if(n < 0) {
            pTIO->base.io.bIOerr = TRUE;
        } else {
            pTIO->io.out_buflen -= n;
            bp += n;
        }
    }
    pTIO->io.out_bufp = &pTIO->io.out_buf[0];
    pTIO->io.out_buflen = 0;
}

static
void __raw_putch(cli_io_ssh_t *pTIO, unsigned char c)
{
    *(pTIO->io.out_bufp++) = c;
    if(++pTIO->io.out_buflen == sizeof(pTIO->io.out_buf))
        __raw_flush(pTIO);
}

/* Real SSH IO layer */

static
void cli_io_ssh_init(cli_iolayer_t *pIO)
{
    cli_io_ssh_t *pTIO = (cli_io_ssh_t *) pIO;
    pIO->bIOerr = FALSE;
    pIO->bEcho = TRUE,
    pIO->authenticated = FALSE;
    pTIO->io.out_bufp = &pTIO->io.out_buf[0];
    pTIO->io.out_buflen = 0;
    if (!pTIO->trace_registered) {
        if (vtss_trace_io_register(pTIO->trace_layer, VTSS_MODULE_ID_CLI, &pTIO->trace_reg) == VTSS_OK) {
            pTIO->trace_registered = TRUE;
        } else {
            T_E("Unable to register trace!");
        }
    } else {
        T_E("Trace already registered!");
    }
}

static
void cli_io_ssh_putchar(cli_iolayer_t *pIO, char ch)
{
    __raw_putch((cli_io_ssh_t *) pIO, ch);
    if(ch == '\n') {
        __raw_putch((cli_io_ssh_t *) pIO, '\r');
    }
    __raw_flush((cli_io_ssh_t *) pIO);
}

static
void cli_io_ssh_puts(cli_iolayer_t *pIO, const char *str)
{
    while( *str ) {
        cli_io_ssh_putchar(pIO, *str++);
    }
}

static
int cli_io_ssh_vprintf(cli_iolayer_t *pIO, const char *fmt, va_list ap)
{
    char buf[1024];
    int l, i;
    l = vsnprintf(buf, sizeof(buf)-1, fmt, ap);
    for(i = 0; i < l; i++)
        cli_io_ssh_putchar(pIO, buf[i]);
    return l;
}

static
void cli_io_ssh_flush(cli_iolayer_t *pIO)
{
    __raw_flush((cli_io_ssh_t *) pIO);
}

static
void cli_io_ssh_close(cli_iolayer_t *pIO)
{
    int child_index = cli_get_child_index();
    cli_io_ssh_t *pTIO = (cli_io_ssh_t *) pIO;

    if (child_index >= 0 && pIO->fd >= 0) {
        __raw_flush(pTIO);
        close(pIO->fd);
        pIO->fd = -1;
        pIO->bIOerr = TRUE;
        pTIO->valid = FALSE;
        if (pTIO->trace_registered) {
            (void)vtss_trace_io_unregister(&pTIO->trace_reg);
            pTIO->trace_registered = FALSE;
        }
        dropbear_ecos_clean_child_thread(child_index);
        /* Terminate client thread */
        cyg_thread_exit();
    }
}

#ifdef VTSS_SW_OPTION_AUTH
static BOOL cli_io_ssh_login(cli_iolayer_t *pIO)
{
    return cli_io_login(pIO, VTSS_AUTH_AGENT_SSH, CLI_PASSWD_CHAR_TIMEOUT);
}
#endif /* VTSS_SW_OPTION_AUTH */

static int cli_io_ssh_getch(cli_iolayer_t *pIO, int timeout, char *ch)
{
    int rc;
    T_N("SSH timeout %d", timeout);
    rc = cli_io_getch(pIO, timeout, ch);
    if (rc == VTSS_OK) {
        T_N("SSH got char '%c' (%u)", ((*ch > 31) && (*ch < 127)) ? *ch : '?', (u8)*ch);
    } else {
        T_N("SSH err %s (%d)", error_txt(rc), rc);
    }
    return rc;
}

static cli_io_ssh_t cli_io_ssh[DROPBEAR_MAX_SOCK_NUM];

static cli_io_ssh_t cli_io_ssh_default =
{
    .base = {
        .io = {
            .cli_init = cli_io_ssh_init,
            .cli_getch = cli_io_ssh_getch,
            .cli_vprintf = cli_io_ssh_vprintf,
            .cli_putchar = cli_io_ssh_putchar,
            .cli_puts = cli_io_ssh_puts,
            .cli_flush = cli_io_ssh_flush,
            .cli_close = cli_io_ssh_close,
#ifdef VTSS_SW_OPTION_AUTH
            .cli_login = cli_io_ssh_login,
#endif /* VTSS_SW_OPTION_AUTH */
            .fd = -1,
            .char_timeout = CLI_COMMAND_CHAR_TIMEOUT,
            .cDEL = CLI_DEL_KEY_LINUX, /* Assume from Linux */
            .cBS  = CLI_BS_KEY_LINUX,  /* Assume from Linux */
            .current_privilege_level = 15,
#if defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI)
            .parser = CLI_PARSER_DEFAULT,
#endif /* defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI) */
#ifdef VTSS_SW_OPTION_ICLI
            .session_way = CLI_WAY_SSH,
#endif /* VTSS_SW_OPTION_ICLI */
        },
    },
};

/*
 * SSH trace layer
 */

static int cli_get_child_index(void)
{
    int child_index;
    cyg_handle_t handle;

    handle = cyg_thread_self();
    for (child_index = 0; child_index < DROPBEAR_MAX_SOCK_NUM; child_index++) {
        if (handle == local_socket_child_handle[child_index])
            return child_index;
    }
    return -1;
}

static void vtss_trace_putchar(struct _vtss_trace_io_t *pIO, char ch)
{
    int child_index = cli_get_child_index();
    if (child_index >= 0) {
        cli_io_ssh_putchar(&cli_io_ssh[child_index].base.io, ch);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < DROPBEAR_MAX_SOCK_NUM; child_index++) {
            if (cli_io_ssh[child_index].base.io.fd >= 0) {
                (void) cli_io_ssh_putchar(&cli_io_ssh[child_index].base.io, ch);
            }
        }
    }
}

static int vtss_trace_vprintf(struct _vtss_trace_io_t *pIO, const char *fmt, va_list ap)
{
    int child_index = cli_get_child_index();
    if (child_index >= 0) {
        return cli_io_ssh_vprintf(&cli_io_ssh[child_index].base.io, fmt, ap);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < DROPBEAR_MAX_SOCK_NUM; child_index++) {
            if (cli_io_ssh[child_index].base.io.fd >= 0) {
                (void) cli_io_ssh_vprintf(&cli_io_ssh[child_index].base.io, fmt, ap);
            }
        }
    }
    return 1;
}

static void vtss_trace_write_string(struct _vtss_trace_io_t *pIO, const char *str)
{
    int child_index = cli_get_child_index();
    if (child_index >= 0) {
        cli_io_ssh_puts(&cli_io_ssh[child_index].base.io, str);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < DROPBEAR_MAX_SOCK_NUM; child_index++) {
            if (cli_io_ssh[child_index].base.io.fd >= 0) {
                (void) cli_io_ssh_puts(&cli_io_ssh[child_index].base.io, str);
            }
        }
    }
}

static void vtss_trace_flush(struct _vtss_trace_io_t *pIO)
{
    int child_index = cli_get_child_index();
    if (child_index >= 0) {
        cli_io_ssh_flush(&cli_io_ssh[child_index].base.io);
    } else {
        /* Calling trace macro from other threads (not telnet or ssh) */
        for (child_index = 0; child_index < DROPBEAR_MAX_SOCK_NUM; child_index++) {
            if (cli_io_ssh[child_index].base.io.fd >= 0) {
                (void) cli_io_ssh_flush(&cli_io_ssh[child_index].base.io);
            }
        }
    }
}

static vtss_trace_io_t vtss_trace_layer = {
    .trace_putchar = vtss_trace_putchar,
    .trace_vprintf = vtss_trace_vprintf,
    .trace_write_string = vtss_trace_write_string,
    .trace_flush = vtss_trace_flush,
};

static void cli_child_create(int index)
{
    sprintf(local_socket_child_name[index], "SSH CLI %01d", index + 1);

    // Create a child thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(THREAD_DEFAULT_PRIO,                          // Priority
                      cli_thread,                                   // entry
                      (cyg_addrword_t) &cli_io_ssh[index].base,     // entry parameter
                      local_socket_child_name[index],               // Name
                      &local_socket_child_stack[index][0],          // Stack
                      sizeof(local_socket_child_stack[index]),      // Size
                      &local_socket_child_handle[index],            // Handle
                      &local_socket_child_data[index]               // Thread data structure
                     );
    cyg_thread_resume(local_socket_child_handle[index]);            // Start it
}

static void dropbear_local_socket_create(void)
{
    int one = 1;
    struct sockaddr_in local;

restart_local_socket_create:
    dropbear_local_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dropbear_local_socket_fd < 0) {
        PEXIT("stream socket");

        VTSS_OS_MSLEEP(1000);
        goto restart_local_socket_create;
    }
    if (setsockopt(dropbear_local_socket_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        PEXIT("setsockopt SO_REUSEADDR");
    }
    if (setsockopt(dropbear_local_socket_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        PEXIT("setsockopt SO_REUSEPORT");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(DROPBEAR_LOCAL_PORT);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    while (bind(dropbear_local_socket_fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        //diag_printf("bind error, sleeping\n");
        VTSS_OS_MSLEEP(3000);
        if (dropbear_local_socket_fd < 0) {
            goto restart_local_socket_create;
        }
    }
    if (dropbear_local_socket_fd >= 0) {
        listen(dropbear_local_socket_fd, 4);
    } else {
        // The bind() maybe fail due to master/slave status change
        goto restart_local_socket_create;
    }
}

#define CLI_SSH_BUF_SIZE     64

static void dropbear_local_socket_daemon(cyg_addrword_t data)
{
    int conn, i;
    socklen_t client_len;
    struct sockaddr_in client_addr;
    int one = 1;
    int found_empty;
    unsigned long ssh_mode;
    fd_set rfds;
    struct timeval tv = {300, 0};
    cyg_int32 rc, fdmax;

restart_local_deamon:
    do {
        dropbear_get_ssh_mode(&ssh_mode);
        if (!ssh_mode) {
            cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(1000));
        }
    } while (!ssh_mode);

    dropbear_local_socket_create();
    fdmax = dropbear_local_socket_fd;
    FD_ZERO(&rfds);
    FD_SET(dropbear_local_socket_fd, &rfds);

    while (TRUE) {
        rc = select(fdmax + 1, &rfds, NULL, NULL, &tv);
        if ((rc > 0) && dropbear_local_socket_fd >= 0 && (FD_ISSET(dropbear_local_socket_fd, &rfds))) {
            client_len = sizeof(client_addr);
            if ((conn = accept(dropbear_local_socket_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                PEXIT("accept");
            }
            T_I("connection(%d) from %s:%d", conn, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            /* Notice if connection break */
            setsockopt(conn, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));

            /* Found empty entry */
            found_empty = 0;
            for (i = 0; i < DROPBEAR_MAX_SOCK_NUM; i++) {
                if (!cli_io_ssh[i].valid) {
                    found_empty = 1;
                    cli_io_ssh[i].valid = TRUE;
                    cli_io_ssh[i].base.io.fd = conn;
                    cli_io_ssh[i].listen_fd = dropbear_local_socket_fd;

                    /* Login will register trace */
                    cli_io_ssh[i].trace_registered = FALSE;
                    cli_io_ssh[i].trace_layer = &vtss_trace_layer;
            
                    /* Borrow thread - code only */
                    cli_child_create(i);
                    break;
                }
            }

            if (!found_empty) {
                char *buf;
                if((buf = ssh_lib_callout_malloc(CLI_SSH_BUF_SIZE + 1))) {
                    int len = snprintf(buf, CLI_SSH_BUF_SIZE,
                                       "Only %d SSH connections allowed.\r\n",
                                       DROPBEAR_MAX_SOCK_NUM);
                    write(conn, buf, len);
                    ssh_lib_callout_free(buf);
                    cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(500));
                    dropbear_ecos_clean_child_thread(DROPBEAR_MAX_SOCK_NUM);
                }
                close(conn);
            }
        } else {
            dropbear_get_ssh_mode(&ssh_mode);
            if (!ssh_mode || rc < 0 || dropbear_local_socket_fd == -1) {
                goto restart_local_deamon;
            }
        }
    }
}

static void dropbear_cli_child_socket_do_close(vtss_restart_t restart)
{
    int i, active = 0;
    for (i = 0; i < DROPBEAR_MAX_SOCK_NUM; i++) {
        if (cli_io_ssh[i].valid && cli_io_ssh[i].base.io.authenticated) {
            cli_io_ssh[i].base.io.bIOerr = TRUE; /* Force all sessions to terminate themselves */
            active++;
        }
    }
    if (active) {
        T_I("%d SSH session%s terminated!", active, (active > 1) ? "s" : "");
        VTSS_OS_MSLEEP(CLI_GETCH_WAKEUP * 2); /* Give the sessions a little time to terminate */
    }
    /* 
     * We have previously called cyg_thread_delete() when we want to force a thread to be closed.
     * This will remove the thread on the scheduler list (and make it impossible to query the thread for e.g stack usage).
     * It will NOT free any memory that is assigned to the thread, as it is statically allocated.
     * It is perfectly ok to recreate a cli thread using the existing handle.
     */
}

void dropbear_cli_child_socket_close(void)
{
  dropbear_cli_child_socket_do_close(VTSS_RESTART_COLD /* Unused */);
  if (dropbear_local_socket_fd >= 0) {
    close(dropbear_local_socket_fd);
    dropbear_local_socket_fd = -1;
  }
}

void dropbear_local_socket_init(void)
{
    int i;

    // Set default configuration
    for (i = 0; i < DROPBEAR_MAX_SOCK_NUM; i++) {
        cli_io_ssh[i] = cli_io_ssh_default;
    }

    // Get warning about system resets
    control_system_reset_register(dropbear_cli_child_socket_do_close);
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(THREAD_DEFAULT_PRIO,                  // Priority
                      dropbear_local_socket_daemon,         // entry
                      0,                                    // entry parameter
                      "SSH CLI Main",                       // Name
                      &local_socket_parent_stack[0],        // Stack
                      sizeof(local_socket_parent_stack),    // Size
                      &local_socket_parent_thread_handle,   // Handle
                      &local_socket_parent_thread_data      // Thread data structure
            );
    cyg_thread_resume(local_socket_parent_thread_handle);   // Start it
}

/* Set user name and his privilege level */
void dropbear_current_priv_lvl_set(int child_index, char *user_name, int privilege_level)
{
    if (child_index >= 0 && child_index < DROPBEAR_MAX_SOCK_NUM) {
#ifdef VTSS_SW_OPTION_ICLI
        if ( user_name ) {
            // Update the current user name
            (void)strncpy(cli_io_ssh[child_index].base.io.user_name, user_name, VTSS_SYS_USERNAME_LEN);
            cli_io_ssh[child_index].base.io.user_name[VTSS_SYS_USERNAME_LEN - 1] = '\0';
        }
#endif /* VTSS_SW_OPTION_ICLI */
        // Update the current privilege level
        cli_io_ssh[child_index].base.io.current_privilege_level = privilege_level;
    }
}

/* Set client IP and port */
void dropbear_client_ip_port_set(int child_index, struct sockaddr *client_addr, socklen_t client_len)
{
    if (child_index >= 0 && child_index < DROPBEAR_MAX_SOCK_NUM) {
#ifdef VTSS_SW_OPTION_ICLI
        memcpy(cli_io_ssh[child_index].base.io.client_addr, client_addr, client_len);
#endif /* VTSS_SW_OPTION_ICLI */
    }
}

#endif /* DROPBEAR_NO_SHELL */
