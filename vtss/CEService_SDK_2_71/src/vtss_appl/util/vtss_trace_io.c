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
 
 $Id$
 $Revision$

*/


#if defined(VTSS_OPSYS_ECOS)
#include <cyg/infra/diag.h>
#else
#include <stdio.h>
#define diag_sprintf		sprintf
#define diag_printf		printf
#define diag_vprintf		vprintf
#define diag_write_string(s)	fputs(s, stdout)
#define diag_write_char(c)	putchar(c)
#endif

#include "main.h"
#include "vtss_trace.h"
#include "cli.h" /* To get access to cli_printf() */


#define TRACE_MAX_IO_REGS 8 /* 4 for Telnet (CLI_TELNET_MAX_CLIENT) and 4 for SSH (DROPBEAR_MAX_SOCK_NUM) */
typedef struct _trace_io_reg_t {
    vtss_trace_io_t  *io;
    vtss_module_id_t module_id;
} trace_io_reg_t;

static int trace_io_reg_cnt = 0;
static trace_io_reg_t trace_io_regs[TRACE_MAX_IO_REGS];

/* Trace ring buffer */
#define TRACE_RB_SIZE           100000
#define TRACE_RB_MAX_ENTRY_SIZE 1000 /* Max size of a single entry in rb */
typedef struct _trace_rb_t {
    char buf[TRACE_RB_SIZE+1];
    int  wr_pos;
    BOOL dis; // Disable field instead of an enable field, so that we can avoid initializing it
} trace_rb_t;
static trace_rb_t trace_rb;

#if VTSS_TRACE_MULTI_THREAD
  #if VTSS_OPSYS_ECOS
    /* Don't wait for a semaphore, because the caller of trace
       might be marked as a caller from IRQ context. */
    #define WAIT_RINGBUF() cyg_scheduler_lock()
  #else
    /* Other operating systems may wish to change this. */
    #warning "Inserting waiting point from ISR/DSR context"
    #define WAIT_RINGBUF VTSS_OS_SEM_WAIT(&trace_rb_crit)
  #endif /* VTSS_OPSYS_ECOS */
#else
  #define WAIT_RINGBUF() do {} while(0);
#endif /* VTSS_TRACE_MULTI_THREAD */

#if VTSS_TRACE_MULTI_THREAD
  #if VTSS_OPSYS_ECOS
    #define POST_RINGBUF() cyg_scheduler_unlock()
  #else
    /* Other operating systems may wish to change this. */
    #define POST_RINGBUF VTSS_OS_SEM_POST(&trace_rb_crit)
  #endif /* VTSS_OPSYS_ECOS */
#else
  #define POST_RINGBUF() do {} while(0);
#endif /* VTSS_TRACE_MULTI_THREAD */

/* ###########################################################################
 * Internal functions
 * 
 * The "err_buf" argument is used to store a copy of the trace message,
 * used for writing error messages to flash.
 * If err_buf is NULL, then such copy is generated.
 * ------------------------------------------------------------------------ */

/* ===========================================================================
 * Console functions
 * ------------------------------------------------------------------------ */

int  trace_sprintf(char *buf, const char *fmt, ...)
{
    int rv;
    va_list ap;
    va_start(ap, fmt);

    rv = diag_sprintf(buf, fmt, ap);

    va_end(ap);

    return rv;
} /* trace_sprintf */


int  trace_vprintf(char *err_buf, const char *fmt, va_list ap)
{
    int rv;
    rv = diag_vprintf(fmt, ap);

    if (trace_io_reg_cnt) {
        int i = 0;
        for (i = 0; i < TRACE_MAX_IO_REGS; i++) {
            if (trace_io_regs[i].io != NULL) {
                trace_io_regs[i].io->trace_vprintf(trace_io_regs[i].io, fmt, ap);
            }
        }
    }

    if (err_buf) {
        vsnprintf(&err_buf[strlen(err_buf)], TRACE_ERR_BUF_SIZE-strlen(err_buf), fmt, ap);
    }

    return rv;
} /* trace_vprintf */


int  trace_printf(char *err_buf,  const char *fmt, ...)
{
    int rv = 0;

    va_list ap;
    va_start(ap, fmt);

    rv = trace_vprintf(err_buf, fmt, ap);

    va_end(ap);

    return rv;
} /* trace_printf */


void trace_write_string(char *err_buf, const char *str)
{
    diag_write_string(str);

    if (trace_io_reg_cnt) {
        int i = 0;
        for (i = 0; i < TRACE_MAX_IO_REGS; i++) {
            if (trace_io_regs[i].io != NULL) {
                trace_io_regs[i].io->trace_write_string(trace_io_regs[i].io, str);
            }
        }
    }

    if (err_buf) {
        snprintf(&err_buf[strlen(err_buf)], TRACE_ERR_BUF_SIZE-strlen(err_buf), str);
    }
    
} /* trace_write_string */


void trace_write_char(char *err_buf, char c)
{
    diag_write_char(c);

    if (trace_io_reg_cnt) {
        int i = 0;
        for (i = 0; i < TRACE_MAX_IO_REGS; i++) {
            if (trace_io_regs[i].io != NULL) {
                trace_io_regs[i].io->trace_putchar(trace_io_regs[i].io, c);
            }
        }
    }

    if (err_buf) {
        snprintf(&err_buf[strlen(err_buf)], TRACE_ERR_BUF_SIZE-strlen(err_buf), "%c", c);
    }

} /* trace_write_char */


void trace_flush(void)
{
    if (trace_io_reg_cnt) {
        int i = 0;
        for (i = 0; i < TRACE_MAX_IO_REGS; i++) {
            if (trace_io_regs[i].io != NULL) {
                trace_io_regs[i].io->trace_flush(trace_io_regs[i].io);
            }
        }
    }
} /* trace_flush */

/* ---------------------------------------------------------------------------
 * Console functions
 * ======================================================================== */


/* ===========================================================================
 * Ring buffer functions
 * ------------------------------------------------------------------------ */

static void rb_write_string(char *err_buf, const char *str)
{
    uint str_len = MIN(strlen(str), TRACE_RB_SIZE);

    if (trace_rb.dis != 0) return;

    /* Copy into ring buffer */
    if (trace_rb.wr_pos + str_len <= TRACE_RB_SIZE) {
        /* Fast path */
        memcpy(&trace_rb.buf[trace_rb.wr_pos], str, str_len);
        trace_rb.wr_pos += str_len;
        if (trace_rb.wr_pos >= TRACE_RB_SIZE) {
            trace_rb.wr_pos = 0;
        }
    } else {
        /* Wrap-around */
        memcpy(&trace_rb.buf[trace_rb.wr_pos], str,                                     TRACE_RB_SIZE - trace_rb.wr_pos);
        memcpy(&trace_rb.buf[0],               &str[(TRACE_RB_SIZE - trace_rb.wr_pos)], str_len - (TRACE_RB_SIZE - trace_rb.wr_pos));
        trace_rb.wr_pos = str_len - (TRACE_RB_SIZE - trace_rb.wr_pos);
    }

    /* Make sure to zero-terminate current rb content */
    trace_rb.buf[trace_rb.wr_pos] = 0;

    if (err_buf) {
        snprintf(&err_buf[strlen(err_buf)], TRACE_ERR_BUF_SIZE-strlen(err_buf), str);
    }

    TRACE_ASSERT(trace_rb.wr_pos               <= TRACE_RB_SIZE,       ("trace_rb.wr_pos=%d",               trace_rb.wr_pos));
} /* rb_write_string */


int trace_rb_vprintf(char *err_buf, const char *fmt, va_list ap)
{
    int rv;
    static char entry[TRACE_RB_MAX_ENTRY_SIZE];

    WAIT_RINGBUF();

    /* Build entry */
    rv = vsnprintf(entry, TRACE_RB_MAX_ENTRY_SIZE, fmt, ap);

    /* Write to ring buffer */
    rb_write_string(err_buf, entry);

    POST_RINGBUF();

    return rv;
} /* trace_rb_vprintf */


void trace_rb_write_string(char *err_buf, const char *str)
{
    WAIT_RINGBUF();

    rb_write_string(err_buf, str);

    POST_RINGBUF();
} /* trace_rb_write_string */


void trace_rb_write_char(char *err_buf, char c)
{
    WAIT_RINGBUF();

    static char str[2];
    str[0] = c;
    str[1] = 0;
    
    rb_write_string(err_buf, str);

    POST_RINGBUF();
} /* trace_rb_write_char */


/* ---------------------------------------------------------------------------
 * Ring buffer functions
 * ======================================================================== */


/* ---------------------------------------------------------------------------
 * Internal functions
 * ######################################################################## */

/* ###########################################################################
 * API functions
 * ------------------------------------------------------------------------ */

vtss_rc vtss_trace_io_register(
    vtss_trace_io_t  *trace_io,
    vtss_module_id_t module_id,
    uint        *io_reg_id)
{
    vtss_rc rc = VTSS_OK;
    uint         i;

    T_D("enter, module_id=%d", module_id);
    
#if VTSS_TRACE_MULTI_THREAD
    VTSS_OS_SEM_WAIT(&trace_io_crit);
#endif

    /* Find an unused entry in registration table */
    for (i = 0; i < TRACE_MAX_IO_REGS; i++) {
        if (trace_io_regs[i].io == NULL) {
            break;
        }
    }
    
    if (i < TRACE_MAX_IO_REGS) {
        trace_io_regs[i].io = trace_io;
        trace_io_regs[i].module_id = module_id;
        *io_reg_id = i;
        trace_io_reg_cnt++;
    } else {
        T_E("All io registrations in use(!), TRACE_MAX_IO_REGS=%d, trace_io_reg_cnt=%d",
            TRACE_MAX_IO_REGS, trace_io_reg_cnt);
        rc = VTSS_UNSPECIFIED_ERROR;
    }
    
#if VTSS_TRACE_MULTI_THREAD
    VTSS_OS_SEM_POST(&trace_io_crit);
#endif

    T_D("exit, module_id=%d, rc=%d, io_reg_id=%u", module_id, rc, *io_reg_id);
    
    return rc;
} /* vtss_trace_io_register */


vtss_rc vtss_trace_io_unregister(
    uint *io_reg_id)
{
    vtss_rc rc = VTSS_OK;
    
    T_D("enter, *io_reg_id=%u", *io_reg_id);
    
#if VTSS_TRACE_MULTI_THREAD
    VTSS_OS_SEM_WAIT(&trace_io_crit);
#endif

    TRACE_ASSERT(*io_reg_id < TRACE_MAX_IO_REGS, ("*io_reg_id=%d", *io_reg_id));

    trace_io_regs[*io_reg_id].io = NULL;
    trace_io_reg_cnt--;
    
#if VTSS_TRACE_MULTI_THREAD
    VTSS_OS_SEM_POST(&trace_io_crit);
#endif

    T_D("exit, module_id=%d, rc=%d, io_reg_id=%u", trace_io_regs[*io_reg_id].module_id, rc, *io_reg_id);
    
    return rc;
} /* vtss_trace_io_unregister */


/* ===========================================================================
 * Ring buffer functions used by CLI
 * ------------------------------------------------------------------------ */

void vtss_trace_rb_output(void)
{
    WAIT_RINGBUF();

    trace_rb.buf[TRACE_RB_SIZE] = 0;

    if (trace_rb.wr_pos+1 < TRACE_RB_SIZE) {
        cli_printf("%s", &trace_rb.buf[trace_rb.wr_pos+1]);
    }
    if (trace_rb.wr_pos != 0) {
        cli_printf("%s", trace_rb.buf);
    }
    
    POST_RINGBUF();
} /* vtss_trace_rb_output */


/* Flush current content ring buffer */
void vtss_trace_rb_flush(void)
{
    WAIT_RINGBUF();

    memset(trace_rb.buf, 0, TRACE_RB_SIZE);
    trace_rb.wr_pos = 0;

    POST_RINGBUF();
} /* vtss_trace_rb_flush */


void vtss_trace_rb_ena(BOOL ena) 
{
    trace_rb.dis = !ena;
} /* vtss_trace_rb_ena */


/* ---------------------------------------------------------------------------
 * Ring buffer functions used by CLI
 * ======================================================================== */

/* ---------------------------------------------------------------------------
 * API functions
 * ######################################################################## */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
