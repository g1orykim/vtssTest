/**********************************************-*- mode: C; c-indent: 2  -*-*/
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

#include "msg_test_api.h"
#include "msg_api.h"

/*lint -esym(459, msg_test_jumbo) */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MSG_TEST

/****************************************************************************/
// Useful macros
/****************************************************************************/
#undef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))

/****************************************************************************/
// Trace definitions
/****************************************************************************/
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MSG_TEST
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CNT          1
#include <vtss_trace_api.h>

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by msg_test_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "msg_test",
    .descr     = "Message Test"
};

#ifndef MSG_TEST_DEFAULT_TRACE_LVL
#define MSG_TEST_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_NOISE
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = MSG_TEST_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
//
//                      MESSAGE PROTOCOL TEST SUITE
//
/****************************************************************************/

// Message Test Thread variables
static cyg_handle_t msg_test_thread_handle;
static cyg_thread   msg_test_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         msg_test_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// Event Flag that is used to wake up the msg_thread.
static cyg_flag_t msg_test_flag;

static BOOL msg_test_suite_in_progress;
static u16  msg_test_idx = 0; // Current test in progress
static u32  msg_test_suite_args[3];

static void msg_test_basic(void);
static void msg_test_jumbo(void);

typedef struct {
    char *dscr;
    void (*func)(void);
} msg_test_suite_t;

#define MSG_TEST_IDX_NULL  0
#define MSG_TEST_IDX_BASIC 1
#define MSG_TEST_IDX_JUMBO 2

msg_test_suite_t msg_test_suites[] = {
    [MSG_TEST_IDX_NULL] = {
        .dscr = "<Dummy. Do Not Remove>",
        .func = NULL,
    },
    [MSG_TEST_IDX_BASIC] = {
        .dscr = "Basic transmission to master with dynamic and static memory allocation",
        .func = msg_test_basic,
    },
    [MSG_TEST_IDX_JUMBO] = {
        .dscr = "Jumbo-message transmission to up to 4 slaves in stack.\n"
        "   Arguments: <isid_start> <isid_end> [<message size measured in kBytes>].",
        .func = msg_test_jumbo,
    },
};

static vtss_os_crit_t msg_test_crit; // Used to test that a critical region may be used to protect internal states while sending and receiving messages.
#define MSG_TEST_STATIC_MSG  "xStatic test message\0"
#define MSG_TEST_DYNAMIC_MSG "xDynamic test message\0"
static char *static_msg = MSG_TEST_STATIC_MSG;

#define MSG_TEST_CONTXT_FLAG_DYNAMIC       0x1
#define MSG_TEST_CONTXT_FLAG_EXP_CB        0x2
#define MSG_TEST_CONTXT_FLAG_MSG_MOD_FREES 0x4
#define MSG_TEST_CONTXT_FLAG_HIGH_PRIO     0x8
#define MSG_TEST_CREATE_CONTXT(d, e, f, p) (((d) ? MSG_TEST_CONTXT_FLAG_DYNAMIC       : 0) | \
                                            ((e) ? MSG_TEST_CONTXT_FLAG_EXP_CB        : 0) | \
                                            ((f) ? MSG_TEST_CONTXT_FLAG_MSG_MOD_FREES : 0) | \
                                            ((p) ? MSG_TEST_CONTXT_FLAG_HIGH_PRIO     : 0))
#define MSG_TEST_CREATE_OPT(f, p) (((f) ? 0 : MSG_TX_OPT_DONT_FREE) | ((p) ? MSG_TX_OPT_PRIO_HIGH : 0))

/****************************************************************************/
// msg_test_rx()
// Called back when a message destined for VTSS_MODULE_ID_MSG_TEST arrives.
/****************************************************************************/
static BOOL msg_test_rx(void *contxt, const void *const msg, const size_t len, const vtss_module_id_t modid, const u32 id)
{
    int i;
    u8  *msg_ptr = (u8 *)msg;
    u8  test_idx = msg_ptr[0];
    u8  disid;
    u32 sz;

    T_I("msg_test_rx(): test_idx=%d, msg_ptr=%p, len=%zu, id=%u", test_idx, msg, len, id);

    (void)VTSS_OS_CRIT_ENTER(&msg_test_crit);
    switch (test_idx) {
    case MSG_TEST_IDX_BASIC:
        T_N("RX(basic): m=\"%s\", l=%zu, SISID=%u", ((u8 *)msg) + 1, len, (u32)id);
        break;

    case MSG_TEST_IDX_JUMBO:
        disid = msg_ptr[1];
        sz  = msg_ptr[2] << 24;
        sz |= msg_ptr[3] << 16;
        sz |= msg_ptr[4] <<  8;
        sz |= msg_ptr[5] <<  0;
        T_N("Rx(jumbo): l=%zu, disid=%d", len, disid);
        if (len != sz + disid) {
            T_E("Rx(jumbo): Unexpected length. Expected %u, got %zu", sz + disid, len);
            break;
        }

        msg_ptr += 6; // Real data starts at offset 6
        for (i = 6; i < (int)sz; i++) {
            if (*msg_ptr != (u8)i) {
                T_E("Rx(jumbo): Unexpected value at offset = %zu. Expected %u, got %u", (size_t)(msg_ptr - (u8 *)msg), i, *msg_ptr);
                break;
            }
            msg_ptr++;
        }
        break;

    default:
        T_E("Unknown test (%u)", msg_test_idx);
        break;

    }

    VTSS_OS_CRIT_EXIT(&msg_test_crit);
    return TRUE;
}

/****************************************************************************/
/****************************************************************************/
static void msg_test_basic_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    BOOL dynamic       = ((u32)contxt & MSG_TEST_CONTXT_FLAG_DYNAMIC)       != 0;
    BOOL exp_cb        = ((u32)contxt & MSG_TEST_CONTXT_FLAG_EXP_CB)        != 0;
    BOOL msg_mod_frees = ((u32)contxt & MSG_TEST_CONTXT_FLAG_MSG_MOD_FREES) != 0;
    BOOL high_prio     = ((u32)contxt & MSG_TEST_CONTXT_FLAG_HIGH_PRIO)     != 0;

    (void)VTSS_OS_CRIT_ENTER(&msg_test_crit);
    T_N("TX DONE: rc=%d, m=\"%s\", d=%d, cb=%d, f=%d, p=%d", rc, (u8 *)msg, dynamic, exp_cb, msg_mod_frees, high_prio);

    if (exp_cb == FALSE) {
        T_E("msg_test_basic_tx_done was called back unexpectedly");
    }
    if (dynamic && !msg_mod_frees) {
        VTSS_FREE(msg);
    }

    VTSS_OS_CRIT_EXIT(&msg_test_crit);
}

/****************************************************************************/
/****************************************************************************/
static void msg_test_basic_tx(BOOL dynamic, BOOL callback, BOOL msg_mod_frees, BOOL high_prio)
{
    char *msg;
    u32 len;

    if (dynamic) {
        len = strlen(MSG_TEST_DYNAMIC_MSG) + 1; // One for the trailing NULL
        msg = VTSS_MALLOC(len);
        if (msg == NULL) {
            T_E("Out of memory");
            return;
        }
        strncpy(msg, MSG_TEST_DYNAMIC_MSG, len);
    } else {
        len = strlen(MSG_TEST_STATIC_MSG) + 1; // One for the trailing NULL
        msg = static_msg;

        // Don't let the Message Module deallocate the message, since it's statically declared.
        if (msg_mod_frees) {
            T_E("Internal error");
            return;
        }
    }
    msg[0] = MSG_TEST_IDX_BASIC;

    T_N("TX: m=\"%s\", d=%d, cb=%d, f=%d, p=%d", &msg[1], dynamic, callback, msg_mod_frees, high_prio);

    // Select the flavor of msg_tx() to call.
    if (dynamic && !callback && msg_mod_frees) {
        msg_tx(VTSS_MODULE_ID_MSG_TEST, 0, msg, len);
    } else {
        u32 contxt = MSG_TEST_CREATE_CONTXT(dynamic, callback, msg_mod_frees, high_prio);
        contxt |= (u32)MSG_TEST_IDX_BASIC << 16;
        msg_tx_opt_t opt = MSG_TEST_CREATE_OPT(msg_mod_frees, high_prio);
        msg_tx_adv((void *)contxt, callback ? msg_test_basic_tx_done : NULL, opt , VTSS_MODULE_ID_MSG_TEST, 0, msg, len);
    }
}

/****************************************************************************/
/****************************************************************************/
static void msg_test_basic(void)
{
    // Try to send messages to current master.
    // We take the mutex to test that the callback functions (msg_test_basic_tx_done()
    // and msg_test_rx()) are able to take it as well without a deadlock.
    (void)VTSS_OS_CRIT_ENTER(&msg_test_crit);
    BOOL high_prio = FALSE;
    while (1) {
        // Parameters to msg_test_basic_tx():
        //   dynamic, callback msg_test_basic_tx_done, Message Module frees memory, high_prio
        msg_test_basic_tx(TRUE,  FALSE, TRUE,  high_prio); // Dynamic memory, Tx Done is not called back, Message Module frees
        msg_test_basic_tx(TRUE,  TRUE,  TRUE,  high_prio); // Dynamic memory, Tx Done is called back,     Message Module frees
        msg_test_basic_tx(TRUE,  TRUE,  FALSE, high_prio); // Dynamic memory, Tx Done is called back,     Message Module doesn't free
        msg_test_basic_tx(FALSE, TRUE,  FALSE, high_prio); // Static memory,  Tx Done is called back,     Message Module doesn't free

        // Switch prio
        if (high_prio) {
            break;
        }

        high_prio = TRUE;
    }
    VTSS_OS_CRIT_EXIT(&msg_test_crit);
}

/****************************************************************************/
/****************************************************************************/
static void msg_test_jumbo_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    u16         test_idx = (u32)contxt >> 16;
    vtss_isid_t isid     = (u32)contxt & 0xFFFF;

    T_I("TxDone(jumbo): msg=%p, rc=%u, isid=%d", msg, rc, isid);

    if (test_idx != MSG_TEST_IDX_JUMBO) {
        T_E("TxDone(jumbo): Invalid test index. Expected %u, got %u", MSG_TEST_IDX_JUMBO, test_idx);
    }

    if (rc != MSG_TX_RC_OK) {
        T_E("TxDone(jumbo): RC indicates error: %u", rc);
    }
}

#if VTSS_SWITCH_STACKABLE
#define JUMBO_BUF_CNT 4
#else
#define JUMBO_BUF_CNT 1
#endif

/****************************************************************************/
/****************************************************************************/
static void msg_test_jumbo(void)
{
#define DEFAULT_JUMBO_MSG_SZ_BYTES 2000000

    u8          *long_msg[JUMBO_BUF_CNT], *msg_ptr[JUMBO_BUF_CNT];
    int         i, j, used_bufs = 0;
    u32         contxt = (MSG_TEST_IDX_JUMBO << 16);
    u32         sz = DEFAULT_JUMBO_MSG_SZ_BYTES;
    vtss_isid_t isid_start, isid_end;

    isid_start = msg_test_suite_args[0];
    isid_end   = msg_test_suite_args[1];

    if (!VTSS_ISID_LEGAL(isid_start) ||
        !VTSS_ISID_LEGAL(isid_end)   ||
        !(isid_start <= isid_end)    ||
        (isid_end - isid_end + 1 > JUMBO_BUF_CNT)) {
        T_E("Illegal ISID range: %d-%d", isid_start, isid_end);
        return;
    }

    if (msg_test_suite_args[2] != 0) {
        sz = msg_test_suite_args[2] * 1000; // Small kilobytes.
    }

    sz = MIN(sz, MSG_MAX_LEN_BYTES - VTSS_ISID_CNT);

    for (i = 0; i < JUMBO_BUF_CNT; i++) {
        msg_ptr[i] = long_msg[i] = VTSS_MALLOC(sz);
        if (long_msg[i] == NULL) {
            T_E("Out of memory. This module may not have cleaned up after itself, so please reboot");
        }
        msg_ptr[i] += 6; // Skip across msg[0] reserved for test idx, msg[1] reserved for disid, and msg[2-5] reserved for length.

        for (j = 6; j < (int)sz; j++) {
            *(msg_ptr[i])++ = (u8)j;
        }

        long_msg[i][0] = MSG_TEST_IDX_JUMBO;
        long_msg[i][2] = (sz >> 24) & 0xFF;
        long_msg[i][3] = (sz >> 16) & 0xFF;
        long_msg[i][4] = (sz >>  8) & 0xFF;
        long_msg[i][5] = (sz >>  0) & 0xFF;
    }

    // Send to all connected slaves.
    for (i = (int)isid_start; i <= (int)isid_end; i++) {
        if (msg_switch_exists(i)) {
            long_msg[used_bufs][1] = i;
            T_N("msg_test_jumbo(): Tx %u bytes message to isid=%d", sz + i, i);
            msg_tx_adv((void *)(contxt | i), msg_test_jumbo_tx_done, 0, VTSS_MODULE_ID_MSG_TEST, i, long_msg[used_bufs], sz + i);

            used_bufs++;
            if (used_bufs == JUMBO_BUF_CNT) {
                break;
            }
        }
    }

    for (i = used_bufs; i < JUMBO_BUF_CNT; i++) {
        VTSS_FREE(long_msg[i]);
    }
}

/****************************************************************************/
/****************************************************************************/
static void msg_test_thread(cyg_addrword_t data)
{
    while (1) {
        // Wait until we get an event
        msg_test_idx = cyg_flag_wait(&msg_test_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        msg_test_suites[msg_test_idx].func();
        msg_test_suite_in_progress = FALSE;
    }
}

/******************************************************************************/
// msg_test_suite()
/******************************************************************************/
void msg_test_suite(u32 num, u32 arg0, u32 arg1, u32 arg2, int (*print_function)(const char *fmt, ...))
{
    int i;
    if (num == 0 || num > sizeof(msg_test_suites) / sizeof(msg_test_suites[0]) - 1) {
        (void)print_function("Defined message test suites:\n");
        for (i = 1; i < (int)(sizeof(msg_test_suites) / sizeof(msg_test_suites[0])); i++) {
            (void)print_function("%d: %s\n", i, msg_test_suites[i].dscr);
        }
        return;
    }

    if (msg_test_suite_in_progress) {
        (void)print_function("A test is already in progress. Hold your horses\n");
        return;
    }

    msg_test_suite_in_progress = TRUE;

    msg_test_suite_args[0] = arg0;
    msg_test_suite_args[1] = arg1;
    msg_test_suite_args[2] = arg2;

    // Send the event to the message test thread.
    cyg_flag_setbits(&msg_test_flag, num);
}

/******************************************************************************/
// Initialize Message Test Module
/******************************************************************************/
vtss_rc msg_test_init(vtss_init_data_t *data)
{
    if (data->cmd == INIT_CMD_INIT) {

        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        VTSS_OS_CRIT_CREATE(&msg_test_crit);
        msg_test_suite_in_progress = FALSE;

        // Create thread.
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          msg_test_thread,
                          0,
                          "Message Test",
                          msg_test_thread_stack,
                          sizeof(msg_test_thread_stack),
                          &msg_test_thread_handle,
                          &msg_test_thread_state);
        cyg_thread_resume(msg_test_thread_handle);

        // Create a flag that can wake up the msg_test_thread.
        cyg_flag_init(&msg_test_flag);

    } else if (data->cmd == INIT_CMD_START) {
        msg_rx_filter_t rx_filter;
        vtss_rc         rc;

        // Subscribe to Messages.
        memset(&rx_filter, 0, sizeof(rx_filter));
        rx_filter.cb    = msg_test_rx;
        rx_filter.modid = VTSS_MODULE_ID_MSG_TEST;
        rc = msg_rx_filter_register(&rx_filter);
        if (rc < 0) {
            T_E("Unable to register for messages");
        }
    }
    return VTSS_OK;
}
