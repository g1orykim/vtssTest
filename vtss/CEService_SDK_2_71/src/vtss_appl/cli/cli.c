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
#include "mgmt_api.h"

#include <sys/param.h>
#include <network.h>
#include <fcntl.h>

#ifdef CYGPKG_THREADLOAD
#include <cyg/threadload/threadload.h>
#endif /* CYGPKG_THREADLOAD */





#ifdef VTSS_SW_OPTION_ICLI

/* Somewhere in eCos, printf is defined as diag_printf,
 * which causes __attribute__ format(printf) to fail
 * when including icli_api.h
 */
#undef printf

#include "icli_api.h"

/* Verify some constants until iCLI uses common header files */
#if ICLI_USERNAME_MAX_LEN != VTSS_SYS_USERNAME_LEN
#error ICLI_USERNAME_MAX_LEN != VTSS_SYS_USERNAME_LEN
#endif
#if ICLI_PASSWORD_MAX_LEN != VTSS_SYS_PASSWD_LEN
#error ICLI_PASSWORD_MAX_LEN != VTSS_SYS_PASSWD_LEN
#endif
#endif /* VTSS_SW_OPTION_ICLI */

#ifdef VTSS_SW_OPTION_ICFG /* CP, 06/24/2013 13:57, Bugzilla#12076 - slient upgrade */
#include "icfg_api.h"
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global variable struct */
typedef struct {
    cyg_handle_t thread_handle;
    cyg_thread   thread_block;
    char         thread_stack[CLI_STACK_SIZE];
    cyg_ucount32 ioindex;  /* Index for the per thread data */
    critd_t      crit;     /* Shared data critical region protection */
} cli_global_t;

static cli_global_t cli;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "cli",
    .descr     = "Command line interface"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_VCLI] = {
        .name      = "vcli",
        .descr     = "Vitesse CLI",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_TELNET] = {
        .name      = "telnet",
        .descr     = "Telnet",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_POE] = {
        .name      = "poe",
        .descr     = "PoE",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 0,
    },

    [VTSS_TRACE_GRP_LLDP] = {
        .name      = "lldp",
        .descr     = "LLDP",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 0,
    },
    [VTSS_TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
};
#define CLI_CRIT_ENTER() critd_enter(&cli.crit, VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CLI_CRIT_EXIT()  critd_exit( &cli.crit, VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CLI_CRIT_ENTER() critd_enter(&cli.crit)
#define CLI_CRIT_EXIT()  critd_exit( &cli.crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/* CLI Serial IO layer                                                      */
/****************************************************************************/
static void cli_io_serial_init(cli_iolayer_t *pIO)
{
    CLI_CRIT_ENTER();

    pIO->bIOerr = FALSE;
    pIO->authenticated = FALSE;

    if (pIO->fd < 0) {
        pIO->fd = open("/dev/ser0", O_RDWR | O_SYNC);
        if (pIO->fd < 0) {
            pIO->fd = open("/dev/ser1", O_RDWR | O_SYNC);
            if ( pIO->fd < 0 ) {
                T_E("CLI: Error opening /dev/ser[01]");
            }
        }
    }

    CLI_CRIT_EXIT();
}

static int cli_io_serial_getch(cli_iolayer_t *pIO, int timeout, char *ch)
{
    int rc;
    T_N("Serial timeout %d", timeout);
    rc = cli_io_getch(pIO, timeout, ch);
    if (rc == VTSS_OK) {
        T_N("Serial got char '%c' (%u)", ((*ch > 31) && (*ch < 127)) ? *ch : '?', (u8)*ch);
    } else {
        T_N("Serial err %s (%d)", error_txt(rc), rc);
    }
    return rc;
}

static int cli_io_serial_vprintf(cli_iolayer_t *pIO, const char *fmt, va_list ap)
{
    int l = vprintf(fmt, ap);
    (void) fflush(stdout);
    return l;
}

static void cli_io_serial_putchar(cli_iolayer_t *pIO, char ch)
{
    putchar(ch);
    (void) fflush(stdout);
}

static void cli_io_serial_puts(cli_iolayer_t *pIO, const char *str)
{
    while (*str) {
        pIO->cli_putchar(pIO, *str++);
    }
}

static void cli_io_serial_flush(cli_iolayer_t *pIO)
{
    (void) fflush(stdout);
}

static void cli_io_serial_close(cli_iolayer_t *pIO)
{
    pIO->bIOerr = TRUE;
}

#ifdef VTSS_AUTH_ENABLE_CONSOLE
static BOOL cli_io_serial_login(cli_iolayer_t *pIO)
{
    return cli_io_login(pIO, VTSS_AUTH_AGENT_CONSOLE, CLI_NO_CHAR_TIMEOUT);
}
#endif /*  VTSS_AUTH_ENABLE_CONSOLE */

static cli_io_t cli_io_serial = {
    .io = {
        .cli_init = cli_io_serial_init,
        .cli_getch = cli_io_serial_getch,
        .cli_vprintf = cli_io_serial_vprintf,
        .cli_putchar = cli_io_serial_putchar,
        .cli_puts = cli_io_serial_puts,
        .cli_flush = cli_io_serial_flush,
        .cli_close = cli_io_serial_close,
#ifdef VTSS_AUTH_ENABLE_CONSOLE
        .cli_login = cli_io_serial_login,
#endif /*  VTSS_AUTH_ENABLE_CONSOLE */
        .fd = -1,
        .char_timeout = CLI_NO_CHAR_TIMEOUT,
        .bEcho = TRUE,
        .cDEL = CLI_DEL_KEY_WINDOWS, /* Assume serial from Windows */
        .cBS  = CLI_BS_KEY_WINDOWS,  /* Assume serial from Windows */
        .current_privilege_level = 15,
#if defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI)
        .parser = CLI_PARSER_DEFAULT,
#endif /* defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI) */
#ifdef VTSS_SW_OPTION_ICLI
        .session_way = CLI_WAY_CONSOLE,
#endif /* VTSS_SW_OPTION_ICLI */
    },
};

/****************************************************************************/
/* Thread debug functions                                                   */
/****************************************************************************/
static int cli_print_stack_backtrace(cyg_thread_info *info, int (*print_function)(const char *fmt, ...), BOOL compiled_with_O2, BOOL self_thread)
{
    HAL_SavedRegisters *regs;
    int                cnt = 0;

#ifdef __arm__
    cyg_uint32 *inner_fp, *outer_fp;

    // Show stack backtrace.
    // NOTE: This may not work, if you use another compiler than arm-elf-gcc, or
    //       perhaps even another version (though I doubt that the GCC/ARM folks
    //       will change the current layout of a stack frame in future version
    //       of the compiler). Furthermore, this may not work if you compile it
    //       for another CPU than the ARM9.

    // First get the saved registers, given the thread's current stack pointer:
    HAL_THREAD_GET_SAVED_REGISTERS(info->stack_ptr, regs);

    if (print_function) {
        (void)print_function("#%-2d 0x%08x\n", cnt, regs->lr); // Print value of link register. This is the closest we get to where the thread was interrupted.
    }
    cnt++;

    // Then start with the innermost stack frame and work the way out.
    inner_fp = 0x0;
    outer_fp = (cyg_uint32 *)regs->fp; // fp == "(stack) frame pointer"
    // Since the stack is growing downwards, outer functions must have
    // a stack frame located on higher memory addresses. Also make a
    // check that we don't get out of the stack bounds.
    /*lint -e{413} ...null pointer 'inner_fp' in right
      argument to operator: it's ok to have NULL in '>' */
    while (outer_fp > inner_fp && (cyg_uint32)outer_fp >= info->stack_base && (cyg_uint32)outer_fp < info->stack_base + info->stack_size) {
        // A stack frame looks like (example)
        // Stack offset | Contents
        // -------------|---------------------------------------------------------
        // 0xA00        | FP number 0 (innermost). Points to PC number 1 (0xC58)
        // 0xA04        | IP number 0
        // 0xA08        | LR number 0
        // 0xA0C        | PC number 0. Value of regs->fp points to this location.
        // 0xA10        | <other data>
        // ...
        // 0xC4C        | FP number 1. Points to PC number 2 (0xD84).
        // 0xC50        | IP number 1
        // 0xC54        | LR number 1
        // 0xC58        | PC number 1
        // 0xC5C        | <other data>
        // ...
        // 0xD78        | FP number 2 (outermost). Points out of stack.
        // 0xD7C        | IP number 2
        // 0xD80        | LR number 2
        // 0xD84        | PC number 2
        // 0xD88        | <other data>
        // ...

        // regs->pc should not be used (not assigned, eCos says).
        // The stack frame's PC entry is 8 bytes higher than the current instruction.
        // The stack frame's LR entry is the value of the "Link Register", i.e. it
        //   contains the return address of a branch (points to the instruction
        //   following the branch statement).
        // The stack frame's FP is used to traverse the stack frames.
        inner_fp = outer_fp;
        if (print_function) {
            (void)print_function("#%-2d 0x%08x\n", cnt, *(inner_fp - 1)); // Print value of LR
        }
        cnt++;
        outer_fp = (cyg_uint32 *) * (inner_fp - 3); // Get pointer to next (outer) stack frame.
    }
#endif /* __arm__ */

#ifdef __mips__
    cyg_uint32          *x = (cyg_uint32 *)1 /* Initial value not used for anything, but Lint can't see the asm volatile assignment */, *top_of_stack;
    extern cyg_uint32   _ftext, _etext;

    // On MIPS, it's not really possible to backtrace the stack, because the location of
    // return address and stack pointer on the stack for a given function depends on the
    // number of auto-variables that a given function uses, and that number is not available
    // to anyone but the function itself.
    // What we can do is provide a plausible stack back trace.
    // Here, another problem arises, namely the fact that when the code is compiled with -O0,
    // the compiler saves a frame pointer, which can be used to trace the stack with more
    // likely results (as shall be seen below) than when the code is compiled with -O2.
    // When compiled with -O2:
    //   Every function keeps track of its own stack usage, and the unfortunate thing is that
    //   the prologue of every function first subtracts its stack usage from the current stack
    //   pointer and then saves the return address one location ahead of the old stack pointer.
    //   Had it only stored it with a fixed offset from the current stack pointer, would things
    //   have been a lot easier.
    // When compiled with -O0:
    //   Also here, every function keeps track of its own stack usage, and saves the calling
    //   function's return address one location prior to the old stack pointer (as is the case
    //   with the -O2 compiler option). But in addition, it saves the calling funtion's
    //   frame pointer one location before the saved return address. This pointer points to the
    //   calling function's frame, so this can be used to filter out a number of the false hits
    //   by checking that stack[x] == x + 2.
    //
    // Since we don't know how the project is compiled (with -O0 or -O2), it's pretty tough to devise
    // a method for tracing the stack. However, we can improve a bit by first running through
    // the stack of one module that we know will produce a number of hits if assuming -O0 compilation.
    // Therefore, the caller of this function will first call it with the print_function set to NULL
    // in order to count the number of hits when assuming a stack frame. Later, it will re-invoke
    // this function with a non-NULL print_function to enforce the printing with the given
    // @compiled_with_O2 detection.
    //
    // Furthermore, in order to filter out definitely false hits, we compare the stack locations
    // with &_ftext and &_etext, which are the absolute start and end addresses of the .text
    // segment. Only if a given entry is within this range will it be a candidate for return
    // address.

    // First get the saved registers, given the thread's current stack pointer:
    HAL_THREAD_GET_SAVED_REGISTERS(info->stack_ptr, regs);

    top_of_stack = (cyg_uint32 *)((cyg_uint32)info->stack_base + (cyg_uint32)info->stack_size);
    if (compiled_with_O2) {
        // Check one more address, since there's no frame pointer to match on.
        top_of_stack++;
    }

    if (print_function) {
        register CYG_WORD32 ra = 0 /* Initial value not used for anything, but Lint can't see the asm volatile assignment */;

        if (self_thread) {
            // The saved registers can't be used when we're printing info of the currently running thread.
            asm volatile ("move %0,$31;" : "=r" (ra));
        } else {
            ra = regs->d[31];
        }
        (void)print_function("#%-2d 0x%08x\n", cnt, ra); // Print value of RA register. This is the closest we get to where the thread was interrupted.
    }
    cnt++;

    // Then start with the innermost stack frame and work the way out. d[29] == sp = "stack pointer"
    if (self_thread) {
        // The saved registers can't be used when we're printing info of the currently running thread.
        asm volatile ("move %0,$29;" : "=r" (x));
    } else {
        x = (cyg_uint32 *)regs->d[29];
    }

    for (; x >= (cyg_uint32 *)info->stack_base && x < top_of_stack; x++) {
        cyg_uint32 *possible_return_address;

        if (compiled_with_O2) {
            possible_return_address = (cyg_uint32 *)(*x);
        } else {
            possible_return_address = (cyg_uint32 *)(*(x + 1));
        }

        if (compiled_with_O2 || (cyg_uint32 *)(*x) == x + 2) {
            // Check that the possible return address is within the .text segment.
            if (possible_return_address >= &_ftext && possible_return_address < &_etext) {
                if (print_function) {
                    // This is the printing iteration.
                    (void)print_function("#%-2d 0x%08x\n", cnt, possible_return_address);
                }
                cnt++;
            }
        }
    }
#endif /* __mips__ */
    return cnt;
}

#ifdef CYGPKG_THREADLOAD
static void cli_threadload2buf(cyg_bool threadload_started, char *threadload_buf, cyg_uint16 threadload, cyg_uint16 id)
{
    if (threadload_started && id < CYGNUM_THREADLOAD_MAX_ID) {
        mgmt_long2str_float(threadload_buf, threadload, 2);
        strcat(threadload_buf, "%");
    } else {
        strcpy(threadload_buf, "N/A");
    }
}
#endif

void cli_print_thread_status(int (*print_function)(const char *fmt, ...), BOOL backtrace, BOOL running_thread_only)
{
    cyg_handle_t       thread = 0;
    cyg_uint16         id = 0;
    cyg_thread_info    info;
    cyg_uint32         state;
    char               buf[23];
    cyg_uint32         len;
    BOOL               compiled_with_O2 = FALSE;
    int                this_thread_id = cyg_thread_get_id(cyg_thread_self());
#ifdef CYGPKG_THREADLOAD
    char               threadload_1sec_buf[10];
    char               threadload_10sec_buf[10];
    cyg_uint16         threadload_1sec[CYGNUM_THREADLOAD_MAX_ID];
    cyg_uint16         threadload_10sec[CYGNUM_THREADLOAD_MAX_ID];
    cyg_bool           threadload_started = cyg_threadload_get(threadload_1sec, threadload_10sec);
#endif

    if (backtrace) {
        // Nice to also get some version and revision output should this function
        // be invoked due to an exception.
        const char *code_rev = misc_software_code_revision_txt();
        (void)print_function("Version      : %s\n", misc_software_version_txt());
        (void)print_function("Build Date   : %s\n", misc_software_date_txt());
        if (strlen(code_rev)) {
            (void)print_function("Code Revision: %s\n", code_rev);
        }
    }

#ifdef __mips__
    if (backtrace) {
        // We need to figure out whether the project is compiled with or without stack frames
        // (i.e. with -O0 or -O2) for the backtrace printing to work. This requires
        // us to traverse the stack of a "known" thread. See details in the cli_print_stack_backtrace() function.
        cyg_scheduler_lock();
        while (cyg_thread_get_next(&thread, &id) != 0) {
            (void)cyg_thread_get_info(thread, id, &info);
            if (info.name && strcasecmp(info.name, "Critd") == 0) {
                // If the number of hits when assuming stack frame is less than 4, then
                // it's probably because the compilation is made with -O2.
                compiled_with_O2 = cli_print_stack_backtrace(&info, NULL, FALSE, FALSE) < 4;
                break;
            }
        }
        cyg_scheduler_unlock();

        // Potentially either some false hits or a lot of false hits, depending on whether the code is found to be compiled with -O0 or -O2, respectively.
        (void)print_function("Warning: Return addresses are %sunreliable (code seems to be compiled with -O%c)\n", compiled_with_O2 ? "highly "  : "", compiled_with_O2 ? '2' : '0');

        // Prepare for main loop below
        thread = 0;
        id = 0;
    }
#endif

#ifdef CYGPKG_THREADLOAD
    (void)print_function("ID  State SetPrio CurPrio Name                   1sec Load 10sec Load Stack Base Size  Used \n");
    (void)print_function("--- ----- ------- ------- ---------------------- --------- ---------- ---------- ----- -----\n");
#else
    (void)print_function("ID  State SetPrio CurPrio Name                   Stack Base Size  Used \n");
    (void)print_function("--- ----- ------- ------- ---------------------- ---------- ----- -----\n");
#endif

    cyg_scheduler_lock();
#ifdef CYGPKG_THREADLOAD
    // Print the DSR context thread load (index 0).
    cli_threadload2buf(threadload_started, threadload_1sec_buf,  threadload_1sec[0],  0);
    cli_threadload2buf(threadload_started, threadload_10sec_buf, threadload_10sec[0], 0);
    (void)print_function("DSR N/A       N/A     N/A DSR Context            %9s %10s        N/A   N/A   N/A\n", threadload_1sec_buf, threadload_10sec_buf);
#endif

    while (cyg_thread_get_next(&thread, &id) != 0) {
        BOOL self_thread = id == this_thread_id;

        if (running_thread_only && !self_thread) {
            continue;
        }

        (void)cyg_thread_get_info(thread, id, &info);
        state = (info.state & 0x1b);
        memset(buf, ' ', sizeof(buf));
        len = (info.name == NULL ? 0 : strlen(info.name));
        memcpy(buf, info.name, len > sizeof(buf) ? sizeof(buf) : len);
        buf[sizeof(buf) - 1] = '\0';
#ifdef CYGPKG_THREADLOAD
        cli_threadload2buf(threadload_started, threadload_1sec_buf,  threadload_1sec[id],  id);
        cli_threadload2buf(threadload_started, threadload_10sec_buf, threadload_10sec[id], id);

        (void)print_function("%3d %-5s %7d %7d %s %9s %10s 0x%08x %5d",
#else
        (void)print_function("%3d %-5s %7d %7d %s 0x%08x %5d",
#endif
                             id,
                             info.state == 0 ? "Run" :
                             (info.state & 0x04) ? "Susp" :
                             state == 0x01 ? "Sleep" :
                             state == 0x02 ? "CSleep" :
                             state == 0x08 ? "Create" :
                             state == 0x10 ? "Exit" : "?\?\?\?",
                             info.set_pri,
                             info.cur_pri,
                             buf,
#ifdef CYGPKG_THREADLOAD
                             threadload_1sec_buf,
                             threadload_10sec_buf,
#endif
                             info.stack_base,
                             info.stack_size
#ifdef CYGPKG_THREADLOAD
                            );
#else
                            );
#endif

#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
        (void)print_function(" %5d%s\n", info.stack_used, self_thread ? "*" : "");
#else
        (void)print_function("   N/A%s\n", self_thread ? "*" : "");
#endif

        if (backtrace) {
            (void)cli_print_stack_backtrace(&info, print_function, compiled_with_O2, self_thread);
        }
    }
    cyg_scheduler_unlock();
}

/****************************************************************************/
/* iCLI/vCLI generalization functions                                       */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICLI
static BOOL cli_is_icli(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_VCLI
    return (pIO->parser == CLI_PARSER_ICLI);
#else
    return TRUE;
#endif /* VTSS_SW_OPTION_VCLI */
}
#endif /* VTSS_SW_OPTION_ICLI */

#ifdef VTSS_SW_OPTION_VCLI
static BOOL cli_is_vcli(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_ICLI
    return (pIO->parser == CLI_PARSER_VCLI);
#else
    return TRUE;
#endif /* VTSS_SW_OPTION_ICLI */
}
#endif /* VTSS_SW_OPTION_VCLI */

static void cli_banner_motd(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_ICLI
    if (cli_is_icli(pIO)) {
        icli_session_data_t data;
        data.session_id = pIO->icli_session_id;
        if (icli_session_data_get(&data) == VTSS_OK) {
            if (data.b_motd_banner) {
                char banner[ICLI_BANNER_MAX_LEN + 1];
                if (icli_banner_motd_get(banner) == VTSS_OK) {
                    if (banner[0]) {
                        cli_putchar('\n');
                        cli_puts(banner);
                        cli_putchar('\n');
                    }
                } else {
                    T_E("Unable to get motd banner");
                }
            }
        } else {
            T_E("Unable to get iCLI session data");
        }
    }
#endif /* VTSS_SW_OPTION_ICLI */
}

static void cli_pre_login(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_ICLI
    if (cli_is_icli(pIO)) {
        char ch;
        cli_puts("\nPress ENTER to get started");
        while (cli_io_getkey(pIO, 0)) {
            /* Empty input buffer */
        }
        while (pIO->cli_getch(pIO, CLI_NO_CHAR_TIMEOUT, &ch) == VTSS_OK) {
            if (ch == CR) {
                cli_putchar('\n');
                break;
            }
        }
    }
#endif /* VTSS_SW_OPTION_ICLI */
}

#ifdef VTSS_SW_OPTION_AUTH
static void cli_banner_login(cli_iolayer_t *pIO, vtss_auth_agent_t agent)
{
#ifdef VTSS_SW_OPTION_ICLI
    if (cli_is_icli(pIO)) {
        char banner[ICLI_BANNER_MAX_LEN + 1];
        if (agent == VTSS_AUTH_AGENT_CONSOLE) {
            cli_pre_login(pIO);
        }
        if (icli_banner_login_get(banner) == VTSS_OK) {
            if (banner[0]) {
                cli_putchar('\n');
                cli_puts(banner);
                cli_putchar('\n');
            }
        } else {
            T_E("Unable to get login banner");
        }
    }
#endif /* VTSS_SW_OPTION_ICLI */
}
#endif /* VTSS_SW_OPTION_AUTH */

static void cli_banner_exec(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_ICLI
    if (cli_is_icli(pIO)) {
        icli_session_data_t data;
        data.session_id = pIO->icli_session_id;
        if (icli_session_data_get(&data) == VTSS_OK) {
            if (data.b_exec_banner) {
                char banner[ICLI_BANNER_MAX_LEN + 1];
                if (icli_banner_exec_get(banner) == VTSS_OK) {
                    if (banner[0]) {
                        cli_putchar('\n');
                        cli_puts(banner);
                        cli_putchar('\n');
                    }
                } else {
                    T_E("Unable to get exec banner");
                }
            }
        } else {
            T_E("Unable to get iCLI session data");
        }
    }
#endif /* VTSS_SW_OPTION_ICLI */

#ifdef VTSS_SW_OPTION_VCLI
    if (cli_is_vcli(pIO)) {
        vcli_banner_exec(pIO);
    }
#endif /* VTSS_SW_OPTION_VCLI */
}

static void cli_parser_init(cli_iolayer_t *pIO)
{
#ifdef VTSS_SW_OPTION_VCLI
    vcli_cmd_parse_init(pIO);
#endif
}

static void cli_parser_loop(cli_iolayer_t *pIO)
{
    vtss_rc rc;

    while (!pIO->bIOerr) {
#ifdef VTSS_SW_OPTION_ICLI
        if (cli_is_icli(pIO)) {
            if ((rc = icli_session_engine(pIO->icli_session_id)) != ICLI_RC_OK) {
                T_I("iCLI session terminated (%d)", rc);
                break;
            }
            T_D("iCLI session continue");
        }
#endif /* VTSS_SW_OPTION_ICLI */

#ifdef VTSS_SW_OPTION_VCLI
        if (cli_is_vcli(pIO)) {
            if ((rc = vcli_cmd_parse_and_exec(pIO)) != VTSS_OK) {
                T_I("vCLI session terminated (%d)", rc);
                break;
            }
            T_D("vCLI session continue");
        }
#endif /* VTSS_SW_OPTION_VCLI */
    }
}

#ifdef VTSS_SW_OPTION_ICLI
/*
    get session input by char

    INPUT
        app_id  : application ID
        timeout : in millisecond
                  = 0 - no wait
                  < 0 - forever

    OUTPUT
        c : char inputted

    RETURN
        TRUE  : successful
        FALSE : failed due to timeout
*/
static BOOL _icli_char_get(
    IN  u32     app_id,
    IN  i32     timeout,
    OUT i32     *c
)
{
    cli_iolayer_t   *pIO = (cli_iolayer_t *)app_id;
    char            ch;
    i32             rc;

    /* get char */
    rc = pIO->cli_getch(pIO, timeout, &ch);
    if ( rc != VTSS_OK ) {
#if 1 /* Bugzilla#11486, 04/08/2013 15:02 */
        if ( pIO->bIOerr == TRUE ) {
            if ( pIO->icli_session_id != ICLI_SESSION_ID_NONE ) {
                (void)icli_session_close( pIO->icli_session_id );
            }
        }
#endif
        return FALSE;
    }

    *c = ch;
    return TRUE;
}

/*
    output one char on session
*/
static BOOL _icli_char_put(
    IN  u32     app_id,
    IN  char    c
)
{
    cli_iolayer_t   *pIO = (cli_iolayer_t *)app_id;

    pIO->cli_putchar(pIO, c);

#if 1 /* Bugzilla#11486, 04/08/2013 15:02 */
    if ( pIO->bIOerr == TRUE ) {
        if ( pIO->icli_session_id != ICLI_SESSION_ID_NONE ) {
            (void)icli_session_close( pIO->icli_session_id );
        }
        return FALSE;
    }
#endif

    return TRUE;
}

/*
    output string on session
*/
static BOOL _icli_str_put(
    IN  u32     app_id,
    IN  char    *str
)
{
    cli_iolayer_t   *pIO = (cli_iolayer_t *)app_id;

    pIO->cli_puts(pIO, str);

#if 1 /* Bugzilla#11486, 04/08/2013 15:02 */
    if ( pIO->bIOerr == TRUE ) {
        if ( pIO->icli_session_id != ICLI_SESSION_ID_NONE ) {
            (void)icli_session_close( pIO->icli_session_id );
        }
        return FALSE;
    }
#endif

    return TRUE;
}

static void _get_client_addr(
    IN  cli_iolayer_t               *pIO,
    OUT icli_session_open_data_t    *open_data
)
{
    struct sockaddr_storage     *client_addr;

    client_addr = (struct sockaddr_storage *)(pIO->client_addr);
    if ( client_addr->ss_len == 0 ) {
        return;
    }

    switch (client_addr->ss_family) {
    case AF_INET: {
        struct sockaddr_in *sa = (struct sockaddr_in *)client_addr;

        open_data->client_ip.type   = ICLI_IP_ADDR_TYPE_IPV4;
        open_data->client_ip.u.ipv4 = ntohl(sa->sin_addr.s_addr);
        open_data->client_port      = ntohs(sa->sin_port);
        break;
    }
    case AF_INET6: {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)client_addr;

        open_data->client_ip.type   = ICLI_IP_ADDR_TYPE_IPV6;
        memcpy(open_data->client_ip.u.ipv6.addr, sa->sin6_addr.s6_addr, sizeof(vtss_ipv6_t));
        open_data->client_port      = ntohs(sa->sin6_port);
        break;
    }
    default:
        T_E("Unknown address family: %d!", client_addr->ss_family);
        break;
    }
}

static void _open_icli_session(
    IN cli_iolayer_t    *pIO
)
{
    icli_session_open_data_t    open_data;
    i32                         rc;

    /* reset session ID */
    pIO->icli_session_id = ICLI_SESSION_ID_NONE;

    /* prepare open data */
    memset(&open_data, 0, sizeof(open_data));

    open_data.way    = pIO->session_way;
    open_data.app_id = (i32)pIO;

    switch ( pIO->session_way ) {
    case CLI_WAY_CONSOLE:
        open_data.way  = ICLI_SESSION_WAY_THREAD_CONSOLE;
        open_data.name = "CONSOLE";
        break;

    case CLI_WAY_TELNET:
        open_data.way  = ICLI_SESSION_WAY_THREAD_TELNET;
        open_data.name = "TELNET";
        _get_client_addr(pIO, &open_data);
        break;

    case CLI_WAY_SSH:
        open_data.way  = ICLI_SESSION_WAY_THREAD_SSH;
        open_data.name = "SSH";
        _get_client_addr(pIO, &open_data);
        break;

    default:
        T_E("invalid session way = %d\n", pIO->session_way);
        return;
    }

    /* I/O callback */
    open_data.char_get  = _icli_char_get;
    open_data.char_put  = _icli_char_put;
    open_data.str_put   = _icli_str_put;

    /* open ICLI session */
    rc = icli_session_open(&open_data, &(pIO->icli_session_id));
    if ( rc != ICLI_RC_OK ) {
        T_E("Fail to open a session for TELNET, err = %d\n", rc);
        return;
    }

    /* set user name for SSH */
    if ( pIO->session_way == CLI_WAY_SSH ) {
        if (icli_session_privilege_set(pIO->icli_session_id, pIO->current_privilege_level) != ICLI_RC_OK) {
            T_E("Fail to set privilege %d to ICLI session %d\n",
                pIO->current_privilege_level, pIO->icli_session_id);
        }
        if (icli_session_user_name_set(pIO->icli_session_id, pIO->user_name) != ICLI_RC_OK) {
            T_E("Fail to set user name %s to ICLI session %d\n",
                pIO->user_name, pIO->icli_session_id);
        }
    }
}

static void _close_icli_session(
    IN cli_iolayer_t    *pIO
)
{
    if ( pIO->icli_session_id != ICLI_SESSION_ID_NONE ) {
        (void)icli_session_close( pIO->icli_session_id );
        pIO->icli_session_id = ICLI_SESSION_ID_NONE;
    }
}
#endif /* VTSS_SW_OPTION_ICLI */

/****************************************************************************/
/* CLI public functions                                                     */
/****************************************************************************/
/* Generic CLI thread used by serial, Telnet and SSH */
void cli_thread(cyg_addrword_t data)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *)data;

    cyg_thread_set_data(cli.ioindex, data); /* Store the IO layer in my thread data */

    while (1) {

#ifdef VTSS_SW_OPTION_ICLI
        /* open ICLI session */
        _open_icli_session(pIO);
#endif /* VTSS_SW_OPTION_ICLI */

        pIO->cli_init(pIO);

        cli_parser_init(pIO);

        cli_banner_motd(pIO);

        if (pIO->cli_login) {
            if (!pIO->cli_login(pIO)) {
#ifdef VTSS_SW_OPTION_ICLI
                /*
                    close ICLI session
                    put here before cli_close()
                    because cli_close() for Telnet/SSH will exit thread directly.
                */
                _close_icli_session(pIO);
#endif /* VTSS_SW_OPTION_ICLI */
                pIO->cli_close(pIO);
                VTSS_OS_MSLEEP(1000);  /* To stall password cracking */
                continue;
            }
        } else {
            cli_pre_login(pIO); /* Wait for user to press enter (iCLI only) */
        }

        cli_banner_exec(pIO);

#ifdef VTSS_SW_OPTION_ICFG /* CP, 06/24/2013 13:57, Bugzilla#12076 - slient upgrade */
        if ( vtss_icfg_silent_upgrade_active() ) {
            pIO->cli_putchar(pIO, '\n');
            pIO->cli_puts(pIO, "PLEASE NOTE: System configuration is being upgraded. Do not reload or attempt\n");
            pIO->cli_puts(pIO, "             to modify the system until the upgrade process is complete.\n");
            pIO->cli_putchar(pIO, '\n');
        }
#endif

        cli_parser_loop(pIO); /* Main parser loop */

#ifdef VTSS_SW_OPTION_ICLI
        /*
            close ICLI session
            put here before cli_close()
            because cli_close() for Telnet/SSH will exit thread directly.
        */
        _close_icli_session(pIO);
#endif /* VTSS_SW_OPTION_ICLI */

        pIO->cli_close(pIO); /* <- only cli serial returns from this function! */

    } /* while (1) */
}

/* CLI Generic IO layer */

char cli_io_getkey(cli_iolayer_t *pIO, char ch)
{
    char c;
    while (pIO->cli_getch(pIO, 0, &c) == VTSS_OK) {
        if ((ch == 0) || (ch == c)) {
            return c;
        }
    }
    return 0;
}

vtss_rc cli_io_getch(cli_iolayer_t *pIO, int timeout, char *ch)
{
    struct timeval tv;
    int            rounds, num, len, wakeup = CLI_GETCH_WAKEUP; // Wake from select each 'wakeup' mS
    fd_set         set;
    char           c;

    if (timeout == 0) {
        rounds = 1; // One round
        wakeup = 0; // Wakeup immediately
    } else if (timeout < 0) {
        rounds = -1; // No timeout
    } else {
        rounds = timeout / CLI_GETCH_WAKEUP;
        if (timeout % CLI_GETCH_WAKEUP) {
            rounds++;
        }
    }

    FD_ZERO(&set);
    FD_SET((unsigned int)pIO->fd, &set);

    while (rounds && !pIO->bIOerr) {
        tv.tv_sec  = wakeup / 1000;
        tv.tv_usec = (wakeup % 1000) * 1000;

        num = select(pIO->fd + 1, &set, NULL, NULL, &tv);
        switch (num) {
        case 1: // There is something to read
            len = read(pIO->fd, &c, 1);
            if (len != 1) {
                pIO->bIOerr = TRUE;
                return VTSS_UNSPECIFIED_ERROR;
            }
            *ch = c;
            return VTSS_OK;
        case 0: // Timeout
            if (rounds > 0) {
                rounds--;
            }
            break;
        default: // Error
            pIO->bIOerr = TRUE;
            return VTSS_UNSPECIFIED_ERROR;
        }
    }
    if (pIO->bIOerr) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    return CLI_ERROR_IO_TIMEOUT;
}

int cli_io_printf(cli_iolayer_t *pIO, const char *fmt, ...)
{
    int     rc;
    va_list ap;    /*lint -e{530} ... 'ap' is initialized by va_start() */

    va_start(ap, fmt);
    rc = pIO->cli_vprintf(pIO, fmt, ap);
    va_end(ap);

    return rc;
}

int cli_printf(const char *fmt, ...)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *) cyg_thread_get_data(cli.ioindex);
    int           rc;
    va_list       ap; /*lint -e{530} ... 'ap' is initialized by va_start() */

    va_start(ap, fmt);
    rc = pIO->cli_vprintf(pIO, fmt, ap);
    va_end(ap);

    return rc;
}

void cli_puts(char *str)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *) cyg_thread_get_data(cli.ioindex);
    pIO->cli_puts(pIO, str);
}

void cli_putchar(char ch)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *) cyg_thread_get_data(cli.ioindex);
    pIO->cli_putchar(pIO, ch);
}

void cli_flush(void)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *) cyg_thread_get_data(cli.ioindex);
    pIO->cli_flush(pIO);
}

char cli_getkey(char ch)
{
    cli_iolayer_t *pIO = (cli_iolayer_t *) cyg_thread_get_data(cli.ioindex);
    return cli_io_getkey(pIO, ch);
}

#ifdef VTSS_SW_OPTION_AUTH
BOOL cli_io_login(cli_iolayer_t *pIO, vtss_auth_agent_t agent, int timeout)
{
    char ch, username[VTSS_SYS_USERNAME_LEN], passwd[VTSS_SYS_PASSWD_LEN];
    int  ct, userlevel, auth_cnt = 5;

    if (agent == VTSS_AUTH_AGENT_SSH) {
        pIO->authenticated = TRUE;
        return TRUE; /* Nothing more to do. SSH uses its own authentication mechanism */
    }

    cli_banner_login(pIO, agent);

    while (auth_cnt--) {
        cli_puts("\nUsername: ");

        username[ct = 0] = '\0';
        while (!pIO->bIOerr && ct < VTSS_SYS_USERNAME_LEN) {
            if (pIO->cli_getch(pIO, timeout, &ch) != VTSS_OK) {
                return FALSE;
            }
            if ((ch == CTLD) || (ch == CTLH) || (ch == DEL)) {
                if (ct) {
                    ct--;
                    username[ct] = '\0';
                    cli_putchar(ESC);
                    cli_putchar(0x5b);
                    cli_putchar(CURSOR_LEFT);
                    cli_putchar(ESC);
                    cli_putchar(0x5b);
                    cli_putchar(CURSOR_DELETE_TO_EOL);
                    cli_flush();
                }
                continue;
            } else if (ch == CR) {
                break;              /* End of username */
            } else if (ch >= 32 && ch <= 126) {  /* Rack up chars */
                cli_putchar(ch);
                username[ct++] = ch;
                if (ct < VTSS_SYS_USERNAME_LEN) {
                    username[ct] = '\0';
                } else {
                    username[VTSS_SYS_USERNAME_LEN - 1] = '\0';
                }
            }
        }

        if (pIO->bIOerr) {
            return FALSE;
        }

        cli_puts("\nPassword: ");

        passwd[ct = 0] = '\0';
        while (!pIO->bIOerr && ct < VTSS_SYS_PASSWD_LEN) {
            if (pIO->cli_getch(pIO, timeout, &ch) != VTSS_OK) {
                return FALSE;
            }
            if ((ch == CTLD) || (ch == CTLH) || (ch == DEL)) {
                if (ct) {
                    ct--;
                    passwd[ct] = '\0';
                }
                continue;
            } else if (ch == CR) {
                break;              /* End of passwd */
            } else if (ch >= 32 && ch <= 126) {  /* Rack up chars */
                passwd[ct++] = ch;
                if (ct < VTSS_SYS_PASSWD_LEN) {
                    passwd[ct] = '\0';
                } else {
                    passwd[VTSS_SYS_PASSWD_LEN - 1] = '\0';
                }
            }
        }

        if (pIO->bIOerr) {
            return FALSE;
        }

#if 1 /* CP, 04/09/2013 14:01, consume redundant NEWLINE */
        if ( agent == VTSS_AUTH_AGENT_TELNET ) {
            (void)pIO->cli_getch( pIO, 100, &ch );
        }
#endif

        if (vtss_authenticate(agent, username, passwd, &userlevel) == VTSS_OK) {
            CLI_CRIT_ENTER();
            pIO->authenticated = TRUE;
            pIO->current_privilege_level = userlevel; // Update the current privilege level
            CLI_CRIT_EXIT();
#ifdef VTSS_SW_OPTION_ICLI
            if (icli_session_privilege_set(pIO->icli_session_id, userlevel) != ICLI_RC_OK) {
                T_E("Fail to set privilege %d to ICLI session %d\n", userlevel, pIO->icli_session_id);
            }
            if (icli_session_user_name_set(pIO->icli_session_id, username) != ICLI_RC_OK) {
                T_E("Fail to set user name %s to ICLI session %d\n", username, pIO->icli_session_id);
            }
#endif /* VTSS_SW_OPTION_ICLI */
            cli_putchar('\n');
            return TRUE; /* Success */
        }

        cli_puts("\nWrong username or password!");
    }
    return FALSE;
}
#endif /* VTSS_SW_OPTION_AUTH */

void cli_set_io_handle(cli_iolayer_t *pIO)
{
    cyg_thread_set_data(cli.ioindex, (CYG_ADDRWORD) pIO);
}

cli_iolayer_t *cli_get_io_handle(void)
{
    return (cli_iolayer_t *)cyg_thread_get_data(cli.ioindex);
}

cli_iolayer_t *cli_get_serial_io_handle(void)
{
    return &cli_io_serial.io;
}

// Close the serial CLI session and force the user to login again
void cli_serial_close(void)
{
    CLI_CRIT_ENTER();
    if (cli_io_serial.io.authenticated) {
        cli_io_serial.io.bIOerr = TRUE; // Force a reauthentication
    }
    CLI_CRIT_EXIT();
}

void cprintf_repeat_char(char c, uint n)
{
    for (; n > 0; n--) {
        cli_printf("%c", c);
    }
}

char *cli_bool_txt(BOOL enabled)
{
    return (enabled ? "Enabled " : "Disabled");
}

const char *cli_time_txt(time_t time_val)
{
    const char *s;

    s = misc_time2interval(time_val);
    while (*s == ' ') {
        s++;
    }
    if (!strncmp(s, "0d ", 3)) {
        s += 3;
    }

    return s;
}

/* Header with optional new line before and after */
void cli_header_nl_char(char *txt, BOOL pre, BOOL post, char c)
{
    int i, len;

    if (pre) {
        cli_puts("\n");
    }
    cli_printf("%s:\n", txt);
    len = (strlen(txt) + 1);
    for (i = 0; i < len; i++) {
        cli_printf("%c", c);
    }
    cli_puts("\n");
    if (post) {
        cli_puts("\n");
    }
}

/* Underlined header with optional new line before and after */
void cli_header_nl(char *txt, BOOL pre, BOOL post)
{
    cli_header_nl_char(txt, pre, post, '-');
}

/* Underlined header with new line before and after */
void cli_header(char *txt, BOOL post)
{
    cli_header_nl_char(txt, 1, post, '=');
}

static void cli_table_header_parm(char *txt, BOOL parm)
{
    int i, j, len, count = 0;

    cli_printf("%s\n", txt);
    while (*txt == ' ') {
        cli_puts(" ");
        txt++;
    }
    len = strlen(txt);
    for (i = 0; i < len; i++) {
        if (txt[i] == ' ') {
            count++;
        } else {
            for (j = 0; j < count; j++) {
                cli_printf("%c", count > 1 && (parm || j >= (count - 2)) ? ' ' : '-');
            }
            cli_puts("-");
            count = 0;
        }
    }
    for (j = 0; j < count; j++) {
        cli_printf("%c", count > 1 && (parm || j >= (count - 2)) ? ' ' : '-');
    }
    cli_puts("\n");
}

void cli_table_header(char *txt)
{
    cli_table_header_parm(txt, 0);
}

void cli_parm_header(char *txt)
{
    cli_table_header_parm(txt, 1);
}

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/
vtss_rc cli_init(vtss_init_data_t *data)
{
    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Create semaphore for critical regions */
        critd_init(&cli.crit, "cli.crit", VTSS_MODULE_ID_CLI, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        CLI_CRIT_EXIT();

        cli.ioindex = cyg_thread_new_data_index();

#ifdef VTSS_SW_OPTION_VCLI
        vcli_init();
#endif

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          cli_thread,
                          (cyg_addrword_t) &cli_io_serial,
                          "CLI Serial",
                          &cli.thread_stack[0],
                          sizeof(cli.thread_stack),
                          &cli.thread_handle,
                          &cli.thread_block);

#ifdef VTSS_SW_OPTION_CLI_TELNET
        cli_telnet_init();
#endif
        break;

    case INIT_CMD_MASTER_UP: {
        static BOOL already_resumed;
        if (!already_resumed) {
            // Well, in fact, this BOOL is not really needed
            // because cyg_thread_resume()'s suspend_counter
            // can't go below 0, but for the sake of support
            // of future OSs, it's better to clarify it.
            cyg_thread_resume(cli.thread_handle);
            already_resumed = TRUE;
        }
        break;
    }

    default:
        return VTSS_UNSPECIFIED_ERROR;
    }

    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
