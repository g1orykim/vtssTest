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
#include "dumpbuffer_api.h"
#include "control_api.h"
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif  /* VTSS_SW_OPTION_SYSLOG */
#include "misc_api.h"

#include <cyg/hal/hal_arch.h>   /* HAL_* */
#include <cyg/hal/hal_if.h>     /* For HAL vectors */
#include <cyg/infra/diag.h>     /* diag_* */
#include <cyg/infra/cyg_ass.h>  /* For cyg_assert_msg */

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MAIN

// Dump buffer structure
typedef struct dumpbuffer {
    cyg_uint32 magic;           /* magic cookie */
#define DUMPBUF_MAGIC_EMPTY     0xBEADFAC1
#define DUMPBUF_MAGIC_VALID     0xBEDAFA2C

    cyg_uint32 type;            /* type field */
#define DUMPBUF_TYPE_NONE       0xde42ff02
#define DUMPBUF_TYPE_EXCEPTION  (DUMPBUF_TYPE_NONE+1)
#define DUMPBUF_TYPE_STRING     (DUMPBUF_TYPE_NONE+2)

    size_t length;

    union {
        struct {
            cyg_code_t code;
            HAL_SavedRegisters regs;
            char version[128];
        } exception;

        struct {
            char buffer[4096 - 128]; /* Persist buffer - overhead */
        } string;

    } data;
} dumpbuffer_t;

static dumpbuffer_t *dumpbuffer;

// "Memory" printing
static struct {
    size_t length, left;
    char buffer[4096];
} memprint;

void memprint_init(void)
{
    memprint.length = 0;
    memprint.left = sizeof(memprint.buffer);
}

int memprint_printf(const char *fmt, ...)
{
    int ret = 0;
    cyg_scheduler_lock();
    if (memprint.left > 1) {    /* Reserve NULL termination */
        va_list ap = NULL;
        va_start(ap, fmt);
        ret = diag_vsnprintf(&memprint.buffer[memprint.length], memprint.left, fmt, ap);
        memprint.length += ret;
        memprint.left -= ret;
        va_end(ap);
    }
    cyg_scheduler_unlock();
    return ret;
}

void cyg_assert_fail(const char *fn, const char *file, cyg_uint32 lineno, const char *msg) __THROW {
    const char *fmt = "eCos assertion:%s:%d: %s, version %s";
    const char *rev = misc_software_code_revision_txt();
    cyg_uint32 old_ints /*lint --e{529} */;

    HAL_DISABLE_INTERRUPTS(old_ints); /* No restore */

    if (dumpbuffer)
    {
        dumpbuffer->magic = DUMPBUF_MAGIC_VALID;
        dumpbuffer->type = DUMPBUF_TYPE_STRING;
        dumpbuffer->length = diag_snprintf(dumpbuffer->data.string.buffer, sizeof(dumpbuffer->data.string.buffer), fmt, file, lineno, msg, rev);
        diag_write_string(dumpbuffer->data.string.buffer);
        diag_write_char('\n');
    } else {
        (void) diag_printf(fmt, file, lineno, msg, rev);
    }

#ifdef CYGHWR_TEST_PROGRAM_EXIT
    CYGHWR_TEST_PROGRAM_EXIT();
#endif

    /* NOTREACHED */
    HAL_PLATFORM_RESET();

    /* This function does *NOT* return */
    for (;;)
    {
        /* NOP */
        ;
    }
}

void dump_buffer_initialize(void *persisted, size_t len)
{
    if (persisted && len >= sizeof(dumpbuffer_t)) {
        dumpbuffer = persisted;
        if (dumpbuffer->magic != DUMPBUF_MAGIC_EMPTY) {
            if (dumpbuffer->magic == DUMPBUF_MAGIC_VALID) {
                if (dumpbuffer->type == DUMPBUF_TYPE_EXCEPTION) {
#ifdef VTSS_SW_OPTION_SYSLOG
                    memprint_init();
                    (void) memprint_printf("Dump record - version %s\n", dumpbuffer->data.exception.version);
                    dump_exception_data(memprint_printf, dumpbuffer->data.exception.code, &dumpbuffer->data.exception.regs, FALSE);
                    syslog_ram_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, memprint.buffer);
                    syslog_flash_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, memprint.buffer);
                    (void) diag_printf("Notice: Exception record saved to syslog.\n");
#else
                    (void)diag_printf("No syslog, exception record ignored\n");
#endif  /* VTSS_SW_OPTION_SYSLOG */
                } else if (dumpbuffer->type == DUMPBUF_TYPE_STRING &&
                           dumpbuffer->length > 0 && dumpbuffer->length < sizeof(dumpbuffer->data.string)) {
#ifdef VTSS_SW_OPTION_SYSLOG
                    syslog_ram_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, dumpbuffer->data.string.buffer);
                    syslog_flash_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, dumpbuffer->data.string.buffer);
                    (void) diag_printf("Notice: Assertion record saved to syslog.\n");
#else
                    (void)diag_printf("No syslog, assertion record ignored\n");
#endif  /* VTSS_SW_OPTION_SYSLOG */
                } else {
                    T_W("Undefined dump buffer type, %08x", dumpbuffer->type);
                }
            } else {
                T_W("Undefined magic, resetting: %08x", dumpbuffer->magic);
            }
        }

        /* Be sure to init */
        dumpbuffer->magic = DUMPBUF_MAGIC_EMPTY;
        dumpbuffer->type = DUMPBUF_TYPE_NONE;
    } else {
        T_E("Inadequate dumpbuffer, have %zd bytes but need %zd. Adjust CYGNUM_REDBOOT_PERSIST_DATA_LENGTH", len, sizeof(dumpbuffer_t));
    }
}

void dump_buffer_save_exeption(cyg_code_t exception_number,
                               const HAL_SavedRegisters *regs)
{
    if (dumpbuffer) {
        dumpbuffer->magic = DUMPBUF_MAGIC_VALID;
        dumpbuffer->type = DUMPBUF_TYPE_EXCEPTION;
        dumpbuffer->length = sizeof(dumpbuffer->data.exception);
        dumpbuffer->data.exception.code = exception_number;
        dumpbuffer->data.exception.regs = *regs;
        strncpy(dumpbuffer->data.exception.version, misc_software_code_revision_txt(), sizeof(dumpbuffer->data.exception.version));
        dumpbuffer->data.exception.version[sizeof(dumpbuffer->data.exception.version) - 1] = '\0'; /* Zero-terminate */
    }
}

vtss_rc dumpbuffer_init(vtss_init_data_t *data)
{
#if !defined(VTSS_ARCH_LUTON28)
    if (data->cmd == INIT_CMD_START) {
#ifdef CYGACC_CALL_IF_GET_PERSIST_DATA
        cyg_persist_data_t persist_data;

        if (CYGACC_CALL_IF_GET_PERSIST_DATA(&persist_data) == persist_data.length &&
            persist_data.length >= sizeof(dumpbuffer_t)) {
            dump_buffer_initialize((void *) persist_data.buffer, persist_data.length);
        } else {
            T_E("Bootloader unable to keep fault data. Please upgrade bootloader!");
        }
#endif
    }
#endif
    return VTSS_RC_OK;
}

