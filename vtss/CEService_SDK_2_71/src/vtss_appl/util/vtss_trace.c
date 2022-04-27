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

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "vtss_trace.h"

#if VTSS_SWITCH
#include <conf_api.h>
#include <led_api.h>

#ifdef VTSS_SW_OPTION_SYSUTIL
#include <sysutil_api.h>
#endif

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif

#if VTSS_SWITCH_MANAGED && defined(VTSS_SW_OPTION_SYSLOG)
#include <syslog_api.h>
#endif
#endif /* VTSS_SWITCH */

#include "vtss_trace_cli.h"

#if VTSS_OPSYS_ECOS
#include <cyg/kernel/diag.h>
#ifdef VTSS_SW_OPTION_CLI
#include <cli_api.h> /* For cli_print_thread_statuc() */
#endif
#endif

#include "misc_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TRACE

/* ===========================================================================
 * Trace for the trace module itself
 *  ----------------------------------------------------------------------- */

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "trace",
    .descr     = "Trace module."
};

#ifndef TRACE_DEFAULT_TRACE_LVL
#define TRACE_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_WARNING
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = TRACE_DEFAULT_TRACE_LVL,
        .timestamp = 0,
        .usec      = 0,
        .ringbuf   = 0,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/* ======================================================================== */


/* ===========================================================================
 * Global variables
 *  ----------------------------------------------------------------------- */

static BOOL trace_init_done = 0;

/* Max number of module index (inherited from module_id.h) */
#define MODULE_ID_MAX VTSS_MODULE_ID_NONE

/* Array with pointers to registrations */
static vtss_trace_reg_t* trace_regs[MODULE_ID_MAX+1];

#if VTSS_TRACE_MULTI_THREAD
/* Semaphore for IO registrations */
vtss_os_sem_t trace_io_crit;

#if !VTSS_OPSYS_ECOS
/* Semaphore for ring buffer access. Not needed for eCos. */
vtss_os_sem_t trace_rb_crit;
#endif
#endif

BOOL vtss_trace_port_list[100]; // List that enables/disables trace per port for the T_*_PORT commands. *MUST* be global.

const vtss_trace_thread_t trace_thread_default = {
    .lvl      = VTSS_TRACE_LVL_RACKET,
    .stackuse = 0,
    .lvl_prv  = VTSS_TRACE_LVL_RACKET,
};

static vtss_trace_thread_t trace_threads[VTSS_TRACE_MAX_THREAD_ID+1];

// In diag.cxx, diag_check_string(), eCos enforces a maximum length of 2048.
#if VTSS_OPSYS_ECOS
#define VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN 2048
#endif

// Global level for all modules and threads
static int global_lvl = VTSS_TRACE_LVL_RACKET;

/* ======================================================================== */



/* ###########################################################################
 * Internal functions
 * ------------------------------------------------------------------------ */

/* ----- Wrappers of vtss_trace_io functions to check for ring buf ----- */
static int grp_trace_printf(const vtss_trace_grp_t* const trace_grp_p,
                            char* const err_buf,
                            const char *fmt, ...)
{
    int rv = 0;
    va_list ap;

    va_start(ap, fmt);
    if (trace_grp_p->ringbuf) {
        rv = trace_rb_vprintf(err_buf, fmt, ap);
    } else if (!trace_grp_p->irq) {
        rv = trace_vprintf(err_buf, fmt, ap);
    }
    va_end(ap);

    return rv;
} /* grp_trace_printf */

static int grp_trace_vprintf(const vtss_trace_grp_t* const trace_grp_p,
                             char* const err_buf,
                             const char *fmt, va_list ap)
{
    int rv = 0;

    if (trace_grp_p->ringbuf) {
        rv = trace_rb_vprintf(err_buf, fmt, ap);
    } else if(!trace_grp_p->irq) {
        rv = trace_vprintf(err_buf, fmt, ap);
    }

    return rv;
} /* grp_trace_vprintf */


static void grp_trace_write_string(const vtss_trace_grp_t* const trace_grp_p,
                                   char* const err_buf,
                                   const char *str)
{
    if (trace_grp_p->ringbuf) {
        trace_rb_write_string(err_buf, str);
    } else if (!trace_grp_p->irq) {
        trace_write_string(err_buf, str);
    }
} /* grp_trace_write_string */


static void grp_trace_write_char(const vtss_trace_grp_t* const trace_grp_p,
                                 char* const err_buf,
                                 const char c)
{
    if (trace_grp_p->ringbuf) {
        trace_rb_write_char(err_buf, c);
    } else if (!trace_grp_p->irq) {
        trace_write_char(err_buf, c);
    }
} /* grp_trace_write_string */


static void grp_trace_flush(const vtss_trace_grp_t* const trace_grp_p)
{
    if (!trace_grp_p->ringbuf && !trace_grp_p->irq) {
        trace_flush();
    }
} /* grp_trace_flush */


static char* trace_lvl_to_char(int lvl)
{
    static char txt[8];
    txt[1] = 0;

    switch (lvl) {
    case VTSS_TRACE_LVL_ERROR:
        txt[0] = 'E';
        break;
    case VTSS_TRACE_LVL_WARNING:
        txt[0] = 'W';
        break;
    case VTSS_TRACE_LVL_INFO:
        txt[0] = 'I';
        break;
    case VTSS_TRACE_LVL_DEBUG:
        txt[0] = 'D';
        break;
    case VTSS_TRACE_LVL_NOISE:
        txt[0] = 'N';
        break;
    case VTSS_TRACE_LVL_RACKET:
        txt[0] = 'R';
        break;
    default:
        txt[0] = '?';
    }
    return txt;
} /* trace_lvl_to_char */


static void time_str(char *buf)
{
    time_t t = time(NULL);
    struct tm *timeinfo_p;

#if VTSS_SWITCH && defined(VTSS_SW_OPTION_SYSUTIL)
    /* Correct for timezone */
    t += (system_get_tz_off() * 60);
#endif
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
    /* Correct for DST */
    t += (time_dst_get_offset() * 60);
#endif

    timeinfo_p = localtime(&t);

    sprintf(buf, "%02d:%02d:%02d",
            timeinfo_p->tm_hour,
            timeinfo_p->tm_min,
            timeinfo_p->tm_sec);
} /* time_str */


static void trace_grp_init(vtss_trace_grp_t* trace_grp_p)
{
    if (trace_grp_p->lvl == 0) {
        trace_grp_p->lvl    = VTSS_TRACE_LVL_ERROR;
    }
    trace_grp_p->cookie = VTSS_TRACE_GRP_T_COOKIE;
} /* trace_grp_init */


static void  trace_grps_init(vtss_trace_grp_t* trace_grp_p, int cnt)
{
    int i;

    for (i = 0; i < cnt; i++) {
        trace_grp_init(&trace_grp_p[i]);
    }
} /* trace_grps_init */


static void trace_threads_init(void)
{
    int i;

    for (i = 0; i <= VTSS_TRACE_MAX_THREAD_ID; i++) {
        trace_threads[i] = trace_thread_default;
    }
} /* trace_threads_init */


/* strip leading path from file */
static const char *trace_filename(const char *fn)
{
    int i, start;
    if(!fn)
        return NULL;
    for (start = 0, i = strlen(fn); i > 0; i--) {
        if (fn[i-1] == '/') {
            start = i;
            break;
        }
    }
    return fn+start;
} /* trace_filename */


static void trace_msg_prefix(
    /* Generate copy of trace message for storing errors in flash */
    char              *err_buf,
    vtss_trace_reg_t* trace_reg_p,
    vtss_trace_grp_t* trace_grp_p,
    int               grp_idx,
    int               lvl,
    const char        *location,
    uint              line_no)
{
    grp_trace_write_string(trace_grp_p, err_buf, trace_lvl_to_char(lvl)); /* Trace type */
    grp_trace_write_char(trace_grp_p, err_buf, ' ');
    grp_trace_write_string(trace_grp_p, err_buf, trace_reg_p->name);      /* Registration name */

    if (grp_idx > 0) {
        /* Not default group => print group name */
        grp_trace_write_char(trace_grp_p, err_buf, '/');
        grp_trace_write_string(trace_grp_p, err_buf, trace_grp_p->name);
    }
    grp_trace_write_char(trace_grp_p, err_buf, ' ');

    /* Wall clock time stamp hh:mm:ss */
    if (trace_grp_p->timestamp && !trace_grp_p->irq) {
        /* Calculate current time. This won't work if called from an interrupt handler
         * because time_str() calls time(), which inserts a waiting point. */
        char buf[strlen("hh:mm:ss") + 2];
        time_str(buf);

        grp_trace_printf(trace_grp_p, err_buf, "%s ", buf);
    }

#if VTSS_OPSYS_ECOS
    /* usec time stamp. Format: ss.mmm,uuu */
    /* Output a usec timestamp if either requested to or users wants a
     * normal timestamp while being marked as a caller from interrupt context */
    if (trace_grp_p->usec || (trace_grp_p->irq && trace_grp_p->timestamp)) {
        cyg_uint64 usecs;
        cyg_uint64 s, m, u;

#ifdef VTSS_ARCH_LUTON28
        uint clks;
        u32  dsr_count = cyg_interrupt_pending_count(CYGNUM_HAL_INTERRUPT_RTC);

        /* HAL_CLOCK_READ() may be called from ISR/DSR context. */
        HAL_CLOCK_READ(&clks);

        /* cyg_current_time() may be called from ISR/DSR context. */
        usecs =
            1000 * ((cyg_current_time() + dsr_count) * ECOS_MSECS_PER_HWTICK) +
            (1000 * clks) / ECOS_HWCLOCKS_PER_MSEC;
#else
        usecs = hal_time_get();
#endif

        s = (usecs / 1000000ULL);
        m = (usecs - 1000000ULL * s) / 1000ULL;
        u = (usecs - 1000000ULL * s - 1000ULL * m);
        s %= 100LLU;

        grp_trace_printf(trace_grp_p, err_buf, "%02u.%03u,%03u ", (u32)s, (u32)m, (u32)u);
    }
#endif

#if VTSS_OPSYS_ECOS
    {
        int thread_id;
        cyg_handle_t thread;
        cyg_thread_info info;

        if (!trace_grp_p->irq) {
            /* cyg_thread_self() may only be called from thread context */
            thread = cyg_thread_self();
            thread_id = cyg_thread_get_id(thread);
            /* Stack used ("su=123") */
            if (thread_id < ARRSZ(trace_threads) && trace_threads[thread_id].stackuse) {
                cyg_thread_get_info(thread, thread_id, &info);
                grp_trace_printf(trace_grp_p, err_buf, "su=%d ", info.stack_used);
            }
        } else {
          thread_id = 0;
        }

        /* Thread ID, function, line */
        grp_trace_printf(trace_grp_p, err_buf, "%d/%s#%d: ", thread_id, trace_filename(location), line_no);
    }
#else
    grp_trace_printf(trace_grp_p, err_buf, "%s: ", location);
#endif
} /* trace_msg_prefix */


static void strn_tolower(char *str, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
} /* str_tolower */


static BOOL str_contains_spaces(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') return 1;
    }

    return 0;
} /* str_contains_spaces */


static void trace_store_prv_lvl(void)
{
    int mid, gidx;
    int i;

    /* Copy current trace levels to previous */
    for (mid = 0; mid <= MODULE_ID_MAX; mid++) {
        if (trace_regs[mid]) {
            for (gidx = 0; gidx < trace_regs[mid]->grp_cnt; gidx++) {
                trace_regs[mid]->grps[gidx].lvl_prv = trace_regs[mid]->grps[gidx].lvl;
            }
        }
    }

    for (i = 0; i <= VTSS_TRACE_MAX_THREAD_ID; i++) {
        trace_threads[i].lvl_prv = trace_threads[i].lvl;
    }
} /* trace_store_prv_lvl */

/* ===========================================================================
 * Configuration
 * ------------------------------------------------------------------------ */

#if VTSS_SWITCH
/*
   Configuration layout

   Header:
     version:      1 byte
     global_flags: 1 byte
     thread_cnt:   1 byte - default: 0 = No configuration saved
     module_cnt:   1 byte - default: 0 = No configuration saved

     Per thread:
       thread_id    1 byte
       thread_flags 1 byte
       thread_rsv   1 byte
       lvl          1 byte

     Per module:
       module_id    1 byte
       module_flags 1 byte
       module_rsv   1 byte
       grp_cnt      1 byte

       Per group:
         lvl        1 byte
         grp_flags  1 byte
*/

/* Size of each field in bytes */
/* Header */
#define TRACE_CFG_VER_SIZE             1
#define TRACE_CFG_GLOBAL_FLAGS_SIZE    1
#define TRACE_CFG_THREAD_CNT_SIZE      1
#define TRACE_CFG_MODULE_CNT_SIZE      1
#define TRACE_CFG_HDR_SIZE             4 /* Sum */

/* Thread */
#define TRACE_CFG_THREAD_ID_SIZE       1
#define TRACE_CFG_THREAD_FLAGS_SIZE    1
#define TRACE_CFG_THREAD_FLAG_STACKUSE 0x02
#define TRACE_CFG_THREAD_RSV_SIZE      1
#define TRACE_CFG_THREAD_LVL_SIZE      1
#define TRACE_CFG_THREAD_SIZE          4 /* Sum */

/* Module */
#define TRACE_CFG_MODULE_ID_SIZE       1
#define TRACE_CFG_MODULE_FLAGS_SIZE    1
#define TRACE_CFG_MODULE_RSV_SIZE      1
#define TRACE_CFG_MODULE_GRP_CNT_SIZE  1
#define TRACE_CFG_MODULE_SIZE          4 /* Sum */

/* Group */
#define TRACE_CFG_GRP_LVL_SIZE         1
#define TRACE_CFG_GRP_FLAGS_SIZE       1
#define TRACE_CFG_GRP_FLAG_TIMESTAMP   0x01
#define TRACE_CFG_GRP_FLAG_USEC        0x04
#define TRACE_CFG_GRP_FLAG_RINGBUF     0x08
#define TRACE_CFG_GRP_SIZE             2 /* Sum */

/* Offset within header/module/group */
#define TRACE_CFG_VER_OFFSET            0
#define TRACE_CFG_GLOBAL_FLAGS_OFFSET   1
#define TRACE_CFG_THREAD_CNT_OFFSET     2
#define TRACE_CFG_MODULE_CNT_OFFSET     3

#define TRACE_CFG_THREAD_ID_OFFSET      0
#define TRACE_CFG_THREAD_FLAGS_OFFSET   1
#define TRACE_CFG_THREAD_RSV_OFFSET     2
#define TRACE_CFG_THREAD_LVL_OFFSET     3

#define TRACE_CFG_MODULE_ID_OFFSET      0
#define TRACE_CFG_MODULE_FLAGS_OFFSET   1
#define TRACE_CFG_MODULE_RSV_OFFSET     2
#define TRACE_CFG_MODULE_GRP_CNT_OFFSET 3

#define TRACE_CFG_GRP_LVL_OFFSET        0
#define TRACE_CFG_GRP_FLAGS_OFFSET      1


/* Default values */
#define TRACE_CFG_VER_DEFAULT         2

vtss_rc vtss_trace_cfg_erase(void)
{
    /* Create conf. block with size=0 => Erase block */
    conf_create(CONF_BLK_TRACE, 0);
    conf_close(CONF_BLK_TRACE);

    return VTSS_OK;
} /* vtss_trace_cfg_erase */


vtss_rc vtss_trace_cfg_wr(void)
{
    uchar   *conf_p;

    T_D("enter");

    int cfg_size = TRACE_CFG_HDR_SIZE;
    int module_cnt = 0;
    int m, g, t;
    int pos;

    /* Calculate size */
    for (m = 0; m < MODULE_ID_MAX+1; m++) {
        if (trace_regs[m] != NULL) {
            module_cnt++;
            cfg_size += TRACE_CFG_MODULE_SIZE;
            cfg_size += trace_regs[m]->grp_cnt*TRACE_CFG_GRP_SIZE;
        }
    }
    cfg_size += VTSS_TRACE_MAX_THREAD_ID*TRACE_CFG_THREAD_SIZE;

    T_D("Creating configuration block CONF_BLK_TRACE=%d of size %d bytes",
        CONF_BLK_TRACE, cfg_size);
    if (!(conf_p = conf_create(CONF_BLK_TRACE, cfg_size))) {
        T_E("Creation of CONF_BLK_TRACE failed, cfg_size=%d", cfg_size);
        return VTSS_UNSPECIFIED_ERROR;
    }

    /* Write configuration */
    /* Header */
    memset(conf_p, 0, cfg_size);
    conf_p[TRACE_CFG_VER_OFFSET]        = TRACE_CFG_VER_DEFAULT;
    conf_p[TRACE_CFG_THREAD_CNT_OFFSET] = VTSS_TRACE_MAX_THREAD_ID;
    conf_p[TRACE_CFG_MODULE_CNT_OFFSET] = module_cnt;
    pos = TRACE_CFG_HDR_SIZE;

    /* Threads */
    for (t = 1; t <= VTSS_TRACE_MAX_THREAD_ID; t++) {
        if (pos + TRACE_CFG_THREAD_SIZE > cfg_size) {
            /* We are about to write beyond the cfg_size?! */
            T_E("pos=%d, cfg_size=%d, t=%d", pos, cfg_size, t);
            memset(conf_p, 0, cfg_size);
            conf_p[TRACE_CFG_VER_OFFSET] = TRACE_CFG_VER_DEFAULT;
            conf_close(CONF_BLK_TRACE);
            return VTSS_UNSPECIFIED_ERROR;
        }

        T_D("Writing trace settings for thread_id=%d, pos=%d", t, pos);
        conf_p[pos + TRACE_CFG_THREAD_ID_OFFSET] = t;
        conf_p[pos + TRACE_CFG_THREAD_FLAGS_OFFSET] =
            (trace_threads[t].stackuse ? TRACE_CFG_THREAD_FLAG_STACKUSE : 0);
        conf_p[pos + TRACE_CFG_THREAD_LVL_OFFSET] =
            trace_threads[t].lvl;

        pos += TRACE_CFG_THREAD_SIZE;
    }

    /* Modules */
    for (m = 0; m < MODULE_ID_MAX+1; m++) {
        if (trace_regs[m] != NULL) {
            if (pos + TRACE_CFG_MODULE_SIZE > cfg_size) {
                /* We are about to write beyond the cfg_size?! */
                T_E("pos=%d, cfg_size=%d, m=%d", pos, cfg_size, m);
                memset(conf_p, 0, cfg_size);
                conf_p[TRACE_CFG_VER_OFFSET] = TRACE_CFG_VER_DEFAULT;
                conf_close(CONF_BLK_TRACE);
                return VTSS_UNSPECIFIED_ERROR;
            }

            conf_p[pos + TRACE_CFG_MODULE_ID_OFFSET] =
                trace_regs[m]->module_id;
            conf_p[pos + TRACE_CFG_MODULE_GRP_CNT_OFFSET] =
                trace_regs[m]->grp_cnt;
            pos += TRACE_CFG_MODULE_SIZE;

            T_D("Writing trace settings for module_id=%d (grp_cnt=%d), pos=%d",
                m, trace_regs[m]->grp_cnt, pos);

            for (g = 0; g < trace_regs[m]->grp_cnt; g++) {
                if (pos + TRACE_CFG_GRP_SIZE > cfg_size) {
                    T_E("pos=%d, cfg_size=%d, m=%d, g=%d", pos, cfg_size, m, g);
                    memset(conf_p, 0, cfg_size);
                    conf_p[TRACE_CFG_VER_OFFSET] = TRACE_CFG_VER_DEFAULT;
                    conf_close(CONF_BLK_TRACE);
                    return VTSS_UNSPECIFIED_ERROR;
                }

                conf_p[pos + TRACE_CFG_GRP_LVL_OFFSET]   = trace_regs[m]->grps[g].lvl;
                conf_p[pos + TRACE_CFG_GRP_FLAGS_OFFSET] |=
                    ((trace_regs[m]->grps[g].timestamp ? TRACE_CFG_GRP_FLAG_TIMESTAMP : 0) |
                     (trace_regs[m]->grps[g].usec      ? TRACE_CFG_GRP_FLAG_USEC      : 0) |
                     (trace_regs[m]->grps[g].ringbuf   ? TRACE_CFG_GRP_FLAG_RINGBUF   : 0));

                pos += TRACE_CFG_GRP_SIZE;
            }
        }
    }

    TRACE_ASSERT(pos == cfg_size, ("pos=%d, cfg_size=%d", pos, cfg_size));
    T_D("Completed writing trace configuration. pos=%d, cfg_size=%d",
        pos, cfg_size);

    conf_close(CONF_BLK_TRACE);
    return VTSS_OK;
} /* vtss_trace_cfg_wr */


/*
   Read configuration from flash.
   If number of modules or groups do not match, then no configuration
   is read and an error issued.

   If silent is set, then no errors is issued.
*/
vtss_rc vtss_trace_cfg_rd(void)
{
    vtss_rc rc = VTSS_OK;
    uchar   *conf_p;
    ulong   cfg_size;
    int     thread_cnt = 0;
    int     module_cnt = 0;
    int     thread_id;
    int     module_id;
    int     grp_cnt;
    int     m, g, t;
    int     pos = 0;
    BOOL    conf_ok = TRUE;

    T_D("enter");

    if (!(conf_p = conf_open(CONF_BLK_TRACE, &cfg_size))) {
        T_D("No configuration found");
        return VTSS_UNSPECIFIED_ERROR;
    }

    if (conf_p[TRACE_CFG_MODULE_CNT_OFFSET]) {
        T_I("Reading trace configuration from flash (cfg_size=%u)...", cfg_size);

        /* Check version */
        if (conf_p[TRACE_CFG_VER_OFFSET] != TRACE_CFG_VER_DEFAULT) {
            T_W("Unexpected version: %d", conf_p[TRACE_CFG_VER_OFFSET]);
            conf_ok = FALSE;
        }

        if (conf_ok) {
            /* Copy settings from flash */
            module_cnt = conf_p[TRACE_CFG_MODULE_CNT_OFFSET];
            thread_cnt = conf_p[TRACE_CFG_THREAD_CNT_OFFSET];

            pos = TRACE_CFG_HDR_SIZE;

            for (t = 0; t < MIN(thread_cnt, VTSS_TRACE_MAX_THREAD_ID); t++) {
                thread_id = conf_p[pos + TRACE_CFG_THREAD_ID_OFFSET];

                T_D("Reading trace settings for thread_id=%d, pos=%d",
                    thread_id, pos);

                if (thread_id > 0 && thread_id < VTSS_TRACE_MAX_THREAD_ID) {
                    trace_threads[thread_id].stackuse =
                        ((conf_p[pos + TRACE_CFG_THREAD_FLAGS_OFFSET] &
                          TRACE_CFG_THREAD_FLAG_STACKUSE) != 0);
                    trace_threads[thread_id].lvl = conf_p[pos + TRACE_CFG_THREAD_LVL_OFFSET];
                }

                pos += TRACE_CFG_THREAD_SIZE;
            }


            for (m = 0; m < module_cnt; m++) {
                module_id = conf_p[pos + TRACE_CFG_MODULE_ID_OFFSET];
                grp_cnt   = conf_p[pos + TRACE_CFG_MODULE_GRP_CNT_OFFSET];
                pos += TRACE_CFG_MODULE_SIZE;

                T_D("Reading trace settings for module_id=%d (grp_cnt=%d), pos=%d",
                    module_id, grp_cnt, pos);

                if (module_id >= 0                &&
                    module_id < ARRSZ(trace_regs) &&
                    trace_regs[module_id] != NULL &&
                    trace_regs[module_id]->grp_cnt == grp_cnt) {
                    /* Module registered and group count matches */
                    for (g = 0; g < grp_cnt; g++) {
                        trace_regs[module_id]->grps[g].lvl =
                            conf_p[pos + TRACE_CFG_GRP_LVL_OFFSET];
                        trace_regs[module_id]->grps[g].timestamp =
                            ((conf_p[pos + TRACE_CFG_GRP_FLAGS_OFFSET] &
                              TRACE_CFG_GRP_FLAG_TIMESTAMP) != 0);
                        trace_regs[module_id]->grps[g].usec =
                            ((conf_p[pos + TRACE_CFG_GRP_FLAGS_OFFSET] &
                              TRACE_CFG_GRP_FLAG_USEC) != 0);
                        trace_regs[module_id]->grps[g].ringbuf =
                            ((conf_p[pos + TRACE_CFG_GRP_FLAGS_OFFSET] &
                              TRACE_CFG_GRP_FLAG_RINGBUF) != 0);

                        pos += TRACE_CFG_GRP_SIZE;
                    }
                } else {
                    T_W("Unknown module or missing group count match: "
                        "module_id=%d, grp_cnt=%d", module_id, grp_cnt);
                    pos += grp_cnt*TRACE_CFG_GRP_SIZE;
                }
            }

            if (pos != cfg_size) {
                T_E("pos=%d, cfg_size=%u", pos, cfg_size);
                rc = VTSS_UNSPECIFIED_ERROR;
            }

            T_D("Completed reading configuration, pos=%d, cfg_size=%u",
                pos, cfg_size);
        }

    }

    conf_close(CONF_BLK_TRACE);
    return rc;
} /* vtss_trace_cfg_rd */

#endif /* VTSS_SWITCH */

static void trace_to_flash(vtss_trace_grp_t *trace_grp_p, char *err_buf)
{
#if VTSS_SWITCH && VTSS_SWITCH_MANAGED
    if (err_buf) {
        if(vtss_switch_mgd()) {
            if (trace_grp_p->irq || trace_grp_p->ringbuf) {
#if VTSS_OPSYS_ECOS
                // The error might never be seen if it goes into the ring buffer
                // Use a function that will print on the UART even in DSR/IRQ context.
                diag_printf(err_buf);
#endif
            } else if (!trace_grp_p->irq) {
                /* Don't write to flash if caller is in ISR/DSR context. */
#ifdef VTSS_SW_OPTION_SYSLOG
                syslog_flash_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, err_buf);
#endif
            }

#if VTSS_OPSYS_ECOS && defined(VTSS_SW_OPTION_CLI)
            // It's nice to also get a stack backtrace of the running thread
            // when a trace error occurs.
            cli_print_thread_status(diag_printf, TRUE, TRUE);
#endif
        }
    }
#endif
}

/* ======================================================================== */

/* ---------------------------------------------------------------------------
 * Internal functions
 * ######################################################################## */



/* ###########################################################################
 * API functions
 * ------------------------------------------------------------------------ */

void vtss_trace_port_set(int port_index,BOOL trace_disabled) {
    vtss_trace_port_list[port_index] = trace_disabled;
}

BOOL vtss_trace_port_get(int port_index) {
    return vtss_trace_port_list[port_index];
}

void vtss_trace_reg_init(vtss_trace_reg_t* trace_reg_p, vtss_trace_grp_t* trace_grp_p, int grp_cnt)
{
    if (!trace_init_done) {
        /* Initialize and register trace ressources */
        trace_init_done = 1;
        trace_threads_init();
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

#if VTSS_TRACE_MULTI_THREAD
        /* Create critical region variables */
        VTSS_OS_SEM_CREATE(&trace_io_crit, 1);
#if !VTSS_OPSYS_ECOS
        /* Ring-buf protection is handled through cyg_scheduler_lock/unlock()
           calls on eCos because the caller may be calling from ISR/DSR context. */
        VTSS_OS_SEM_CREATE(&trace_rb_crit, 1);
#endif
#endif
    }

    trace_reg_p->cookie = VTSS_TRACE_REG_T_COOKIE;

    trace_grps_init(trace_grp_p, grp_cnt);
    trace_reg_p->grp_cnt = grp_cnt;
    trace_reg_p->grps    = trace_grp_p;
} /* vtss_trace_reg_init */


vtss_rc vtss_trace_register(vtss_trace_reg_t* trace_reg_p)
{
    int i;

    TRACE_ASSERT(trace_reg_p != NULL, ("trace_reg_p=NULL"));
    TRACE_ASSERT(trace_reg_p->module_id <= MODULE_ID_MAX,
                 ("module_id too large: %d", trace_reg_p->module_id));
    TRACE_ASSERT(trace_reg_p->module_id >= 0,
                 ("module_id too small: %d", trace_reg_p->module_id));


    /* Make sure to zero-terminate name and description string */
    trace_reg_p->name[VTSS_TRACE_MAX_NAME_LEN]   = 0;
    trace_reg_p->descr[VTSS_TRACE_MAX_DESCR_LEN] = 0;

    TRACE_ASSERT(trace_regs[trace_reg_p->module_id] == NULL,
                 ("module_id %d already registered:\n"
                  "Name of 1st registration: %s\n"
                  "Name of 2nd registration: %s",
                  trace_reg_p->module_id,
                  trace_regs[trace_reg_p->module_id]->name,
                  trace_reg_p->name));
    TRACE_ASSERT(trace_reg_p->grps != NULL,
                 ("No groups defined (at least one required), module_id=%d",
                  trace_reg_p->module_id));
    TRACE_ASSERT(trace_init_done,
                 ("trace_init_done=%d", trace_init_done));

    /* Check that registration name contains no spaces */
    TRACE_ASSERT(!str_contains_spaces(trace_reg_p->name),
                 ("Registration name contains spaces.\n"
                  "name: \"%s\"", trace_reg_p->name));

    /* Check that registration name is unique */
    for (i = 0; i <= MODULE_ID_MAX; i++) {
        if (trace_regs[i]) {
            TRACE_ASSERT(strcmp(trace_regs[i]->name, trace_reg_p->name),
                         ("Registration name is not unique. "
                          "Registrations %d and %d are both named \"%s\".",
                          i, trace_reg_p->module_id, trace_reg_p->name));
        }
    }

    /* Check group definitions */
    for (i = 0; i < trace_reg_p->grp_cnt; i++) {
        int j;

        /* Make sure to zero-terminate name and description string */
        trace_reg_p->grps[i].name[VTSS_TRACE_MAX_NAME_LEN]   = 0;
        trace_reg_p->grps[i].descr[VTSS_TRACE_MAX_DESCR_LEN] = 0;

        /* Check that group name contains no spaces */
        TRACE_ASSERT(!str_contains_spaces(trace_reg_p->grps[i].name),
                     ("Group name contains spaces.\n"
                      "module: %s\n"
                      "group: \"%s\"",
                      trace_reg_p->name,
                      trace_reg_p->grps[i].name));

        /* Check that group names within registration are unique */
        for (j = 0; j < trace_reg_p->grp_cnt; j++) {
            if (j != i) {
                TRACE_ASSERT(strcmp(trace_reg_p->grps[i].name,
                                    trace_reg_p->grps[j].name),
                             ("Group names are not unique for registration #%d (\"%s\"): "
                              "Two groups are named \"%s\".",
                              trace_reg_p->module_id, trace_reg_p->name,
                              trace_reg_p->grps[i].name));
            }
        }
    }

    /* Set lvl_prv to initial value */
    for (i = 0; i < trace_reg_p->grp_cnt; i++) {
        trace_reg_p->grps[i].lvl_prv = trace_reg_p->grps[i].lvl;
    }

    /* Convert module and group name to lowercase */
    str_tolower(trace_reg_p->name);
    for (i = 0; i < trace_reg_p->grp_cnt; i++) {
        str_tolower(trace_reg_p->grps[i].name);
    }

    trace_regs[trace_reg_p->module_id] = trace_reg_p;

    return VTSS_OK;
} /* vtss_trace_register */

void vtss_trace_printf(int module_id,
                       int grp_idx,
                       int lvl,
                       const char *location,
                       uint line_no,
                       const char *fmt,
                       ...)
{
    va_list          args;
    vtss_trace_grp_t *trace_grp_p;
    vtss_trace_reg_t *trace_reg_p;
    char             *err_buf = NULL;

    /* Get module's registration */
    trace_reg_p = trace_regs[module_id];
    TRACE_ASSERT(trace_reg_p != NULL, ("module_id=%d %s#%d", module_id, location, line_no));

    /* Get group pointer */
    trace_grp_p = &trace_reg_p->grps[grp_idx];
    TRACE_ASSERT(trace_grp_p != NULL, ("module_id=%d", module_id));

#if defined(_notdef_)           /* Check performed at outer level - see T_xxx() macros */
    /* Return if trace not enabled at this level for the group */
    if (lvl < trace_grp_p->lvl || lvl >= VTSS_TRACE_LVL_NONE) {
        return;
    }

    if (lvl < global_lvl) {
        return;
    }
#endif

#if VTSS_OPSYS_ECOS
    {
        int thread_id = cyg_thread_get_id(cyg_thread_self());
        if (thread_id < ARRSZ(trace_threads) && lvl < trace_threads[thread_id].lvl) {
            return;
        }
    }
#endif

    /* Check that level appears to be one of the well-defined ones */
    TRACE_ASSERT(
        lvl == VTSS_TRACE_LVL_NONE    ||
        lvl == VTSS_TRACE_LVL_ERROR   ||
        lvl == VTSS_TRACE_LVL_WARNING ||
        lvl == VTSS_TRACE_LVL_INFO    ||
        lvl == VTSS_TRACE_LVL_DEBUG   ||
        lvl == VTSS_TRACE_LVL_NOISE   ||
        lvl == VTSS_TRACE_LVL_RACKET,
        ("Unknown trace level used in %s#%d: lvl=%d",
         location, line_no, lvl));

#if VTSS_SWITCH
    if (lvl == VTSS_TRACE_LVL_ERROR) {
        if (!trace_grp_p->irq) {
            /* Don't set LED from ISR/DSR context. */
            led_front_led_state(LED_FRONT_LED_ERROR);
        }

        /* Allocate buffer in which to build error message for flash */
        if ((err_buf = VTSS_MALLOC(TRACE_ERR_BUF_SIZE))) {
            err_buf[0] = 0;
        }
    }
#endif

    /* Print prefix */
    trace_msg_prefix(err_buf,
                     trace_reg_p,
                     trace_grp_p,
                     grp_idx,
                     lvl,
                     location,
                     line_no);

    /* If error or warning add prefix which runtc can identify */
    if (lvl == VTSS_TRACE_LVL_ERROR) {
        grp_trace_printf(trace_grp_p, err_buf, "Error: ");
    }
    if (lvl == VTSS_TRACE_LVL_WARNING) {
        grp_trace_printf(trace_grp_p, err_buf, "Warning: ");
    }

#if VTSS_OPSYS_ECOS
    // In diag.cxx, diag_check_string(), eCos enforces a maximum length of
    // VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN.
    //
    // Check length of format string.
    // Errors from diag_check_string() can still occur, if any of the
    // argument strings exceed VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN.
    // This will result in
    // "<Not a string: 0x81234567>"
    if (strlen(fmt) > VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN) {
        char *fmt_truncated;
        if ((fmt_truncated = VTSS_MALLOC(VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN))) {
            memcpy(fmt_truncated, fmt, VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN-1);
            fmt_truncated[VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN-1] = 0;
        }
        grp_trace_printf(trace_grp_p, err_buf,
                         "Trace format string too long (length=%d, max=%d): \n%s",
                         strlen(fmt),
                         VTSS_TRACE_ECOS_MAX_VPRINTF_STRLEN,
                         fmt_truncated ? fmt_truncated : "");
        if (fmt_truncated) VTSS_FREE(fmt_truncated);
    } else {
#endif
        /* Print message */
        va_start(args, fmt);
        grp_trace_vprintf(trace_grp_p, err_buf, fmt, args);
        va_end(args);
#if VTSS_OPSYS_ECOS
    }
#endif
    grp_trace_write_char(trace_grp_p, err_buf, '\n');
    grp_trace_flush(trace_grp_p);

    trace_to_flash(trace_grp_p, err_buf);
    if (err_buf) {
        VTSS_FREE(err_buf);
    }
} /* vtss_trace_printf */

void vtss_trace_hex_dump(int module_id,
                         int grp_idx,
                         int lvl,
                         const char *location,
                         uint line_no,
                         const uchar *byte_p,
                         int  byte_cnt)
{
    int i = 0;

    vtss_trace_grp_t*  trace_grp_p;
    vtss_trace_reg_t*  trace_reg_p;
    char          *err_buf = NULL;

    trace_reg_p = trace_regs[module_id];
    TRACE_ASSERT(trace_reg_p != NULL, ("module_id=%d", module_id));

    trace_grp_p = &trace_reg_p->grps[grp_idx];
    TRACE_ASSERT(trace_grp_p != NULL, ("module_id=%d", module_id));

    if (lvl < trace_grp_p->lvl ||
        lvl >= VTSS_TRACE_LVL_NONE) {
        return;
    }

#if VTSS_OPSYS_ECOS
    {
        int thread_id = cyg_thread_get_id(cyg_thread_self());
        if (thread_id < ARRSZ(trace_threads) && lvl < trace_threads[thread_id].lvl) {
            return;
        }
    }
#endif

    /* Check that level appears to be one of the well-defined ones */
    TRACE_ASSERT(
        lvl == VTSS_TRACE_LVL_NONE    ||
        lvl == VTSS_TRACE_LVL_ERROR   ||
        lvl == VTSS_TRACE_LVL_WARNING ||
        lvl == VTSS_TRACE_LVL_INFO    ||
        lvl == VTSS_TRACE_LVL_DEBUG   ||
        lvl == VTSS_TRACE_LVL_NOISE   ||
        lvl == VTSS_TRACE_LVL_RACKET,
        ("Unknown trace level used in %s#%d: lvl=%d",
         location, line_no, lvl));

#if VTSS_SWITCH
    if (lvl == VTSS_TRACE_LVL_ERROR) {
        if (!trace_grp_p->irq) {
            /* Don't set LED from ISR/DSR context. */
            led_front_led_state(LED_FRONT_LED_ERROR);
        }

        /* Allocate buffer in which to build error message for flash */
        if ((err_buf = VTSS_MALLOC(TRACE_ERR_BUF_SIZE))) {
            err_buf[0] = 0;
        }
    }
#endif

    trace_msg_prefix(err_buf,
                     trace_reg_p,
                     trace_grp_p,
                     grp_idx,
                     lvl,
                     location,
                     line_no);
    if (byte_cnt > 16) { grp_trace_write_char(trace_grp_p, err_buf, '\n'); }

    i = 0;
    while (i < byte_cnt) {
        int j = 0;
        grp_trace_printf(trace_grp_p, err_buf, "%08x/%04x:", (unsigned int)byte_p+i, i);
        while (j+i < byte_cnt && j < 16) {
            if (j % 2 == 0) {
                grp_trace_write_char(trace_grp_p, err_buf, ' ');
            }
            grp_trace_printf(trace_grp_p, err_buf, "%02x", byte_p[i+j]);
            j++;
        }
        grp_trace_write_char(trace_grp_p, err_buf, '\n');
        i += 16;
    }
    grp_trace_flush(trace_grp_p);

    trace_to_flash(trace_grp_p, err_buf);
    if (err_buf) {
        VTSS_FREE(err_buf);
    }
} /* vtss_trace_hex_dump */

/* ===========================================================================
 * Management functions
 * ------------------------------------------------------------------------ */

#if VTSS_SWITCH

/* Initialization function for managed build */
vtss_rc vtss_trace_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;

#ifdef VTSS_SW_OPTION_VCLI
    trace_cli_init();
#endif
    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("enter, cmd=INIT");
        break;

    case INIT_CMD_START:
        /* Resume thread */
        T_D("enter, cmd=START");
        /* Read configuration from flash (if present) */
        vtss_trace_cfg_rd();
        break;

    case INIT_CMD_CONF_DEF:
        /* Create, save and activate default configuration */
        T_D("enter, cmd=CONF_DEF, isid: %u, flags: 0x%08x", data->isid, data->flags);
        vtss_trace_cfg_erase();
        break;

    default:
        break;
    }

    return rc;
} /* vtss_trace_init */

#endif /* VTSS_SWITCH */


void vtss_trace_module_lvl_set(int module_id, int grp_idx, int lvl)
{
    int i, j;
    int module_id_start=0, module_id_stop=MODULE_ID_MAX;
    int grp_idx_start=0, grp_idx_stop=0;

    if (module_id != -1) {
        module_id_start = module_id;
        module_id_stop  = module_id;

        if (trace_regs[module_id] == NULL) {
            T_E("No registration for module_id=%d", module_id);
            return;
        }
    } else {
        /* Always wildcard group if module is wildcarded */
        grp_idx         = -1;
    }

    if (grp_idx != -1) {
        grp_idx_start = grp_idx;
        grp_idx_stop = grp_idx;

        if (grp_idx > trace_regs[module_id]->grp_cnt-1) {
            T_E("grp_idx=%d too big for module_id=%d (grp_cnt=%d)",
                grp_idx, module_id, trace_regs[module_id]->grp_cnt);
            return;
        }
    }

    /* Store current trace levels before changing */
    trace_store_prv_lvl();

    for (i = module_id_start; i <= module_id_stop; i++) {
        if (trace_regs[i] == NULL)
            continue;
        if (grp_idx == -1) {
            grp_idx_start = 0;
            grp_idx_stop = trace_regs[i]->grp_cnt-1;
        }
        for (j = grp_idx_start; j <= grp_idx_stop; j++) {
            trace_regs[i]->grps[j].lvl = lvl;
        }
    }
} /* vtss_trace_module_lvl_set */


void vtss_trace_module_parm_set(
    vtss_trace_module_parm_t parm,
    int module_id, int grp_idx, BOOL enable)
{
    int i, j;
    int module_id_start=0, module_id_stop=MODULE_ID_MAX;
    int grp_idx_start=0, grp_idx_stop=0;

    if (module_id != -1) {
        module_id_start = module_id;
        module_id_stop  = module_id;

        if (trace_regs[module_id] == NULL) {
            T_E("No registration for module_id=%d", module_id);
            return;
        }
    } else {
        /* Always wildcard group if module is wildcarded */
        grp_idx         = -1;
    }

    if (grp_idx != -1) {
        grp_idx_start = grp_idx;
        grp_idx_stop = grp_idx;

        if (grp_idx > trace_regs[module_id]->grp_cnt-1) {
            T_E("grp_idx=%d too big for module_id=%d (grp_cnt=%d)",
                grp_idx, module_id, trace_regs[module_id]->grp_cnt);
            return;
        }
    }

    for (i = module_id_start; i <= module_id_stop; i++) {
        if (trace_regs[i] == NULL)
            continue;
        if (grp_idx == -1) {
            grp_idx_start = 0;
            grp_idx_stop = trace_regs[i]->grp_cnt-1;
        }
        for (j = grp_idx_start; j <= grp_idx_stop; j++) {
            switch (parm) {
            case VTSS_TRACE_MODULE_PARM_TIMESTAMP:
                trace_regs[i]->grps[j].timestamp = enable;
                break;
            case VTSS_TRACE_MODULE_PARM_USEC:
                trace_regs[i]->grps[j].usec = enable;
                break;
            case VTSS_TRACE_MODULE_PARM_RINGBUF:
                trace_regs[i]->grps[j].ringbuf = enable;
                break;
            default:
                T_E("Unknown parm: %d", parm);
                break;
            }
        }
    }
} /* vtss_trace_module_parm_set */


void vtss_trace_lvl_reverse(void)
{
    int mid, gidx;
    int lvl;
    int i;

    for (mid = 0; mid <= MODULE_ID_MAX; mid++) {
        if (trace_regs[mid]) {
            for (gidx = 0; gidx < trace_regs[mid]->grp_cnt; gidx++) {
                lvl = trace_regs[mid]->grps[gidx].lvl;
                trace_regs[mid]->grps[gidx].lvl     = trace_regs[mid]->grps[gidx].lvl_prv;
                trace_regs[mid]->grps[gidx].lvl_prv = lvl;
            }
        }
    }

    for (i = 0; i <= VTSS_TRACE_MAX_THREAD_ID; i++) {
        lvl = trace_threads[i].lvl;
        trace_threads[i].lvl = trace_threads[i].lvl_prv;
        trace_threads[i].lvl_prv = lvl;
    }
} /* vtss_trace_lvl_reverse */


void vtss_trace_global_lvl_set(int lvl)
{
    global_lvl = lvl;
} // vtss_trace_global_lvl_set


int vtss_trace_global_lvl_get(void)
{
    return global_lvl;
} // vtss_trace_global_lvl_set


int vtss_trace_module_lvl_get(int module_id, int grp_idx)
{
    if (trace_regs[module_id] == NULL) {
        T_E("No registration for module_id=%d", module_id);
        return 0;
    }
    if (grp_idx > trace_regs[module_id]->grp_cnt-1) {
        T_E("grp_idx=%d too big for module_id=%d (grp_cnt=%d)",
            grp_idx, module_id, trace_regs[module_id]->grp_cnt);
        return 0;
    }

    return trace_regs[module_id]->grps[grp_idx].lvl;
} /* vtss_trace_module_lvl_get */


int vtss_trace_global_module_lvl_get(int module_id, int grp_idx)
{
    if (trace_regs[module_id] == NULL) {
        T_E("No registration for module_id=%d", module_id);
        return 0;
    }
    if (grp_idx > trace_regs[module_id]->grp_cnt-1) {
        T_E("grp_idx=%d too big for module %s, (id=%d) (grp_cnt=%d)",
            grp_idx, vtss_module_names[module_id],  module_id, trace_regs[module_id]->grp_cnt);
        return 0;
    }

    return
        (global_lvl > trace_regs[module_id]->grps[grp_idx].lvl) ?
        global_lvl : trace_regs[module_id]->grps[grp_idx].lvl;
} /* vtss_trace_global_module_lvl_get */

char *vtss_trace_module_name_get_nxt(int *module_id_p)
{
    int i;

    TRACE_ASSERT(*module_id_p >= -1, ("*module_id_p=%d", *module_id_p));

    for (i = *module_id_p + 1; i < MODULE_ID_MAX; i++) {
        if (trace_regs[i]) {
            *module_id_p = i;
            return trace_regs[i]->name;
        }
    }

    return NULL;
} /* vtss_trace_module_name_get_nxt */


char *vtss_trace_grp_name_get_nxt(int module_id, int *grp_idx_p)
{
    TRACE_ASSERT(*grp_idx_p >= -1, (" "));

    if (!trace_regs[module_id]) return NULL;
    if (*grp_idx_p + 1 >= trace_regs[module_id]->grp_cnt) return NULL;

    (*grp_idx_p)++;

    /* Check cookies */
    if (trace_regs[module_id]->cookie != VTSS_TRACE_REG_T_COOKIE) {
        T_E("module_id=%d: Cookie=0x%08x, expected 0x%08x",
            module_id,
            trace_regs[module_id]->cookie,
            VTSS_TRACE_REG_T_COOKIE);
    }
    if (trace_regs[module_id]->grps[*grp_idx_p].cookie != VTSS_TRACE_GRP_T_COOKIE) {
        T_E("module_id=%d, grp_idx=%d: Cookie=0x%08x, expected 0x%08x",
            module_id,
            *grp_idx_p,
            trace_regs[module_id]->grps[*grp_idx_p].cookie,
            VTSS_TRACE_GRP_T_COOKIE);
    }

    return trace_regs[module_id]->grps[*grp_idx_p].name;
} /* vtss_trace_grp_name_get_nxt */


char *vtss_trace_module_get_descr(int module_id)
{
    if (!trace_regs[module_id]) return NULL;

    return trace_regs[module_id]->descr;
} /* vtss_trace_module_get_descr */


char *vtss_trace_grp_get_descr(int module_id, int grp_idx)
{
    if (!trace_regs[module_id]) return NULL;
    if (grp_idx > trace_regs[module_id]->grp_cnt-1) return NULL;

    return trace_regs[module_id]->grps[grp_idx].descr;
} /* vtss_trace_grp_get_descr */


BOOL vtss_trace_grp_get_parm(
    vtss_trace_module_parm_t parm,
    int module_id, int grp_idx)
{
    if (!trace_regs[module_id]) {
        T_E("trace_regs[module_id]=null");
        return 0;
    }
    if (grp_idx > trace_regs[module_id]->grp_cnt-1) {
        T_E("grp_idx=%d, grp_cnt=%d",
            grp_idx, trace_regs[module_id]->grp_cnt);
        return 0;
    }

    switch (parm) {
    case VTSS_TRACE_MODULE_PARM_TIMESTAMP:
        return trace_regs[module_id]->grps[grp_idx].timestamp;
        break;
    case VTSS_TRACE_MODULE_PARM_USEC:
        return trace_regs[module_id]->grps[grp_idx].usec;
        break;
    case VTSS_TRACE_MODULE_PARM_RINGBUF:
        return trace_regs[module_id]->grps[grp_idx].ringbuf;
        break;
    case VTSS_TRACE_MODULE_PARM_IRQ:
        return trace_regs[module_id]->grps[grp_idx].irq;
        break;
    default:
        T_E("Unknown parm: %d", parm);
        return 0;
        break;
    }
} /* vtss_trace_grp_get_parm */


BOOL vtss_trace_module_to_val(const char *name, int *module_id_p)
{
    int i;
    char name_lc[VTSS_TRACE_MAX_NAME_LEN+1];
    int  mid_match = -1;

    /* Convert name to lowercase */
    strncpy(name_lc, name, VTSS_TRACE_MAX_NAME_LEN);
    name_lc[VTSS_TRACE_MAX_NAME_LEN] = 0;
    strn_tolower(name_lc, strlen(name_lc));

    for (i = 0; i < MODULE_ID_MAX; i++) {
        if (trace_regs[i]) {
            if (strncmp(name_lc, trace_regs[i]->name, strlen(name_lc)) == 0) {
                if (strlen(trace_regs[i]->name) == strlen(name_lc)) {
                    /* Exact match found */
                    mid_match = i;
                    break;
                }
                if (mid_match == -1) {
                    /* First match found */
                    mid_match = i;
                } else {
                    /* >1 match found */
                    return 0;
                }
            }
        }
    }

    if (mid_match != -1) {
        *module_id_p = mid_match;
        return 1;
    }

    return 0;
} /* vtss_trace_module_to_val */


BOOL vtss_trace_grp_to_val(const char *name, int  module_id, int *grp_idx_p)
{
    int i;
    char name_lc[VTSS_TRACE_MAX_NAME_LEN+1];
    int gidx_match = -1;

    /* Check for valid module_id, in case CLI should call us with -2 */
    if (module_id < 0 ||
        module_id > MODULE_ID_MAX ||
        !trace_regs[module_id]) return 0;

    /* Convert name to lowercase */
    strncpy(name_lc, name, VTSS_TRACE_MAX_NAME_LEN);
    name_lc[VTSS_TRACE_MAX_NAME_LEN] = 0;
    strn_tolower(name_lc, strlen(name_lc));

    for (i = 0; i < trace_regs[module_id]->grp_cnt; i++) {
        if (strncmp(name_lc, trace_regs[module_id]->grps[i].name, strlen(name_lc)) == 0) {
            if (strlen(trace_regs[module_id]->grps[i].name) == strlen(name_lc)) {
                /* Exact match found */
                gidx_match = i;
                break;
            }
            if (gidx_match == -1) {
                /* First match found */
                gidx_match = i;
            } else {
                /* >1 match found */
                return 0;
            }
        }
    }

    if (gidx_match != -1) {
        *grp_idx_p = gidx_match;
        return 1;
    }

    return 0;
} /* vtss_trace_grp_to_val */

const char *vtss_trace_lvl_to_str(int lvl)
{
    switch (lvl) {
    case VTSS_TRACE_LVL_NONE:
        return "none";
        break;
    case VTSS_TRACE_LVL_ERROR:
        return "error";
        break;
    case VTSS_TRACE_LVL_WARNING:
        return "warning";
        break;
    case VTSS_TRACE_LVL_INFO:
        return "info";
        break;
    case VTSS_TRACE_LVL_DEBUG:
        return "debug";
        break;
    case VTSS_TRACE_LVL_NOISE:
        return "noise";
        break;
    case VTSS_TRACE_LVL_RACKET:
        return "racket";
        break;
    default:
        return "others";
    }
} /* vtss_trace_lvl_to_str */


void vtss_trace_thread_lvl_set(int thread_id, int lvl)
{
    TRACE_ASSERT(thread_id == -1 || thread_id <= VTSS_TRACE_MAX_THREAD_ID,
                 ("thread_id=%d", thread_id));

    /* Store current trace levels before changing */
    trace_store_prv_lvl();

    if (thread_id == -1) {
        int i;
        for (i = 0; i <= VTSS_TRACE_MAX_THREAD_ID; i++) {
            trace_threads[i].lvl = lvl;
        }
    } else {
        trace_threads[thread_id].lvl = lvl;
    }
} /* vtss_trace_thread_lvl_set */


void vtss_trace_thread_stackuse_set(int thread_id, BOOL enable)
{
    TRACE_ASSERT(thread_id == -1 || thread_id <= VTSS_TRACE_MAX_THREAD_ID,
                 ("thread_id=%d", thread_id));

    if (thread_id == -1) {
        int i;
        for (i = 0; i <= VTSS_TRACE_MAX_THREAD_ID; i++) {
            trace_threads[i].stackuse = enable;
        }
    } else {
        trace_threads[thread_id].stackuse = enable;
    }
} /* vtss_trace_thread_stackuse_set */


void vtss_trace_thread_get(int thread_id, vtss_trace_thread_t *trace_thread)
{
    TRACE_ASSERT(thread_id <= VTSS_TRACE_MAX_THREAD_ID, ("thread_id=%d", thread_id));

    *trace_thread = trace_threads[thread_id];
} /* vtss_trace_thread_get */


/* ======================================================================== */


/* ---------------------------------------------------------------------------
 * API functions
 * ######################################################################## */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
