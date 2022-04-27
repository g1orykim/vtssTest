/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "msg.h"
#include "port_api.h"        /* For PORT_NO_STACK_0/1, port_isid_info_get(), etc. */
#include "vtss_api_if_api.h" /* For vtss_api_chipid() */
#include <network.h>         /* For htons() and htonl() */
#if defined(VTSS_SW_OPTION_SNMP)
#include "vtss_snmp_api.h"
#else
#define SNMP_TRAP_STACK_SIZE    0
#endif

#include "packet_api.h"

// If the following is defined, all frame relays cause a new buffer to be allocated.
// If it's not defined, the incoming buffer will be re-used.
// It has only effect if MSG_HOP_BY_HOP is non-zero.
// #define MSG_RELAY_ALLOC

#define MSG_INLINE inline

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MSG

/* Avoid Constant value Boolean Lint warning */
/*lint --e{506} */
/*lint -esym(459, DBG_cmd_cfg_trace_isid_set)  */
/*lint -esym(459, DBG_cmd_cfg_trace_modid_set) */
/*lint -esym(459, DBG_cmd_test_run)            */
/*lint -esym(457, TX_thread)                   */
/*lint -esym(459, this_mac)                    */

/****************************************************************************/
// Trace definitions
/****************************************************************************/
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MSG
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CFG          1
#define TRACE_GRP_RX           2
#define TRACE_GRP_TX           3
#define TRACE_GRP_RELAY        4
#define TRACE_GRP_CRIT         5
#define TRACE_GRP_INIT_MODULES 6
#define TRACE_GRP_TOPO         7
#define TRACE_GRP_CNT          8
#include <vtss_trace_api.h>

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by msg_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "msg",
    .descr     = "Message module"
};

#ifndef MSG_DEFAULT_TRACE_LVL
#define MSG_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = MSG_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CFG] = {
        .name      = "cfg",
        .descr     = "Configuration",
        .lvl       = MSG_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_RX] = {
        // Level Usage:
        //   FAILURE:
        //     Internal fatal error (can never be asserted as the result of a malformed received frame)
        //   ERROR:
        //     The received frame doesn't conform to the message protocol standard
        //   WARNING:
        //     If e.g. receiving an unexpected message protocol frame.
        //   INFO:
        //     Show one-liner when a message gets dispatched.
        //   DEBUG:
        //     Show one-liner when a frame is received and other info
        //   NOISE:
        //     Show message contents when dispatching.
        //   RACKET:
        //     Show frame contents of all received frames destined to this switch.
        .name      = "rx",
        .descr     = "Rx",
        .lvl       = MSG_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_TX] = {
        // Level Usage:
        //   FAILURE:
        //     Internal fatal error
        //   ERROR:
        //     Not used.
        //   WARNING:
        //     If e.g. retransmitting an MD.
        //   INFO:
        //     Show user calls to msg_tx() and when user's tx_done is called back.
        //   DEBUG:
        //     Show one-liner of transmitted frames
        //   NOISE:
        //     Show user's message when he calls msg_tx() and when user's tx_done is called back
        //   RACKET:
        //     Show frame contents of all transmitted frames.
        .name      = "tx",
        .descr     = "Tx",
        .lvl       = MSG_DEFAULT_TRACE_LVL, // VTSS_TRACE_LVL_NOISE,
        .timestamp = 1,
    },
    [TRACE_GRP_RELAY] = {
        .name      = "relay",
        .descr     = "Relay",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = MSG_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_INIT_MODULES] = {
        .name      = "initmods",
        .descr     = "init_modules() calls",
        .lvl       = MSG_DEFAULT_TRACE_LVL, /* Info to get some output */
        .timestamp = 1,
    },
    [TRACE_GRP_TOPO] = {
        .name      = "topo",
        .descr     = "Events from Topo",
        .lvl       = MSG_DEFAULT_TRACE_LVL, /* Info to get some output */
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

// Assert with return value. Useful if compiled without assertions.
#define MSG_ASSERTR(expr, fmt, ...) {         \
    if (!(expr)) {                            \
        MSG_ASSERT(expr, fmt, ##__VA_ARGS__); \
        return MSG_ASSERT_FAILURE;            \
    }                                         \
}

// Allow limitation of trace output per ISID when master.
static BOOL msg_trace_enabled_per_isid[VTSS_ISID_CNT + 1];

// Allow limitation of trace output per Module ID when master or slave
static BOOL msg_trace_enabled_per_modid[VTSS_MODULE_ID_NONE + 1];

// Shape messages subject to shaping.
static struct {
    // Maximum number of unsent messages. 0 to disable.
    u32 limit;

    // Current number of unsent messages.
    u32 current;

    // The number of messages dropped due to shaping.
    u32 drops;
} msg_shaper[VTSS_MODULE_ID_NONE + 1];

// This is the default limit for the shaper.
#define MSG_SHAPER_DEFAULT_LIMIT 50

#define MSG_TRACE_ENABLED(isid, modid) (((isid) == 0 || (isid) > VTSS_ISID_CNT || msg_trace_enabled_per_isid[(isid)]) && ((u32)(modid) >= VTSS_MODULE_ID_NONE || msg_trace_enabled_per_modid[(modid)]))

/****************************************************************************/
/*                                                                          */
/*  NAMING CONVENTION FOR INTERNAL FUNCTIONS:                               */
/*    RX_<function_name> : Functions related to Rx.                         */
/*    TX_<function_name> : Functions related to Tx.                         */
/*    CX_<function_name> : Functions related to both Rx and Tx (common).    */
/*    IM_<function_name> : Functions related to InitModules calls.          */
/*    DBG_<function_name>: Functions related to CLI/debugging.              */
/*                                                                          */
/*  NAMING CONVENTION FOR EXTERNAL (API) FUNCTIONS:                         */
/*    msg_rx_<function_name>: Functions related to Rx.                      */
/*    msg_tx_<function_name>: Functions related to Tx.                      */
/*    msg_<function_name>   : Functions related to both Rx and Tx.          */
/*                                                                          */
/****************************************************************************/

/******************************************************************************/
// MSG semaphores
/******************************************************************************/

// Message semaphore (must be semaphore, since it's created locked in
// one thread, and released in another).
static critd_t crit_msg_state;     // Used to protect global state and mcbs
static critd_t crit_msg_counters;  // Used to protect counters
static critd_t crit_msg_cfg;       // Used to protect subscription list
static critd_t crit_msg_pend_list; // Used to protect pending Rx and Tx done lists.
static critd_t crit_msg_im_fifo;   // Used to protect the IM_fifo.
static critd_t crit_msg_buf;       // Used to pretect message allocations.

// Macros for accessing semaphore functions
// -----------------------------------------
#if VTSS_TRACE_ENABLED
#define MSG_STATE_CRIT_ENTER()             critd_enter(        &crit_msg_state,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_STATE_CRIT_EXIT()              critd_exit(         &crit_msg_state,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_STATE_CRIT_ASSERT_LOCKED()     critd_assert_locked(&crit_msg_state,     TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define MSG_COUNTERS_CRIT_ENTER()          critd_enter(        &crit_msg_counters,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_COUNTERS_CRIT_EXIT()           critd_exit(         &crit_msg_counters,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_COUNTERS_CRIT_ASSERT_LOCKED()  critd_assert_locked(&crit_msg_counters,  TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define MSG_CFG_CRIT_ENTER()               critd_enter(        &crit_msg_cfg,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_CFG_CRIT_EXIT()                critd_exit(         &crit_msg_cfg,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_CFG_CRIT_ASSERT(locked)        critd_assert_locked(&crit_msg_cfg,       TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define MSG_PEND_LIST_CRIT_ENTER()         critd_enter(        &crit_msg_pend_list, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_PEND_LIST_CRIT_EXIT()          critd_exit(         &crit_msg_pend_list, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_PEND_LIST_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit_msg_pend_list, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define MSG_IM_FIFO_CRIT_ENTER()           critd_enter(        &crit_msg_im_fifo,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_IM_FIFO_CRIT_EXIT()            critd_exit(         &crit_msg_im_fifo,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_IM_FIFO_CRIT_ASSERT_LOCKED()   critd_assert_locked(&crit_msg_im_fifo,   TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define MSG_BUF_CRIT_ENTER()               critd_enter(        &crit_msg_buf,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_BUF_CRIT_EXIT()                critd_exit(         &crit_msg_buf,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MSG_BUF_CRIT_ASSERT_LOCKED()       critd_assert_locked(&crit_msg_buf,       TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define MSG_STATE_CRIT_ENTER()             critd_enter(        &crit_msg_state)
#define MSG_STATE_CRIT_EXIT()              critd_exit(         &crit_msg_state)
#define MSG_STATE_CRIT_ASSERT_LOCKED()     critd_assert_locked(&crit_msg_state)
#define MSG_COUNTERS_CRIT_ENTER()          critd_enter(        &crit_msg_counters)
#define MSG_COUNTERS_CRIT_EXIT()           critd_exit(         &crit_msg_counters)
#define MSG_COUNTERS_CRIT_ASSERT_LOCKED()  critd_assert_locked(&crit_msg_counters)
#define MSG_CFG_CRIT_ENTER()               critd_enter(        &crit_msg_cfg)
#define MSG_CFG_CRIT_EXIT()                critd_exit(         &crit_msg_cfg)
#define MSG_CFG_CRIT_ASSERT_LOCKED()       critd_assert_locked(&crit_msg_cfg)
#define MSG_PEND_LIST_CRIT_ENTER()         critd_enter(        &crit_msg_pend_list)
#define MSG_PEND_LIST_CRIT_EXIT()          critd_exit(         &crit_msg_pend_list)
#define MSG_PEND_LIST_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit_msg_pend_list)
#define MSG_IM_FIFO_CRIT_ENTER()           critd_enter(        &crit_msg_im_fifo)
#define MSG_IM_FIFO_CRIT_EXIT()            critd_exit(         &crit_msg_im_fifo)
#define MSG_IM_FIFO_CRIT_ASSERT_LOCKED()   critd_assert_locked(&crit_msg_im_fifo)
#define MSG_BUF_CRIT_ENTER()               critd_enter(        &crit_msg_buf)
#define MSG_BUF_CRIT_EXIT()                critd_exit(         &crit_msg_buf)
#define MSG_BUF_CRIT_ASSERT_LOCKED()       critd_assert_locked(&crit_msg_buf)
#endif

// Statically allocate MCBs. The master can open VTSS_ISID_CNT*MSG_CFG_CONN_CNT
// connections towards the slaves (where master itself is also considered a
// slave), whereas a slave can have MSG_CFG_CONN_CNT connections towards the
// master.
// If we are slave, only the MSG_SLV_ISID_IDX index is used.
// If we are master, the remaining VTSS_ISID_CNT entries are possibly used,
// because the master also has loopback connections towards itself.
// When using an ISID in an API call (when master), they are numbered in the
// interval [VTSS_ISID_START; VTSS_ISID_END[. This translates to an index into
// the following array of [1; VTSS_ISID_END-VTSS_ISID_START+1].
#define MSG_SLV_ISID_IDX       0
#define MSG_MST_ISID_START_IDX 1
#define MSG_MST_ISID_END_IDX   (MSG_MST_ISID_START_IDX + VTSS_ISID_CNT - 1) /* End index included */
msg_mcb_t mcbs[VTSS_ISID_CNT + 1][MSG_CFG_CONN_CNT];

// Statically allocate a global state object.
static msg_glbl_state_t state;

// TX Message Thread variables
static cyg_handle_t TX_msg_thread_handle;
static cyg_thread   TX_msg_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         TX_msg_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// RX Message Thread variables
static cyg_handle_t RX_msg_thread_handle;
static cyg_thread   RX_msg_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         RX_msg_thread_stack[THREAD_DEFAULT_STACK_SIZE + SNMP_TRAP_STACK_SIZE];

// Init Modules Thread variables.
// This thread's sole use is to call init_modules() to overcome deadlock arising if init_modules()
// were called directly from the thread, where a topology change was discovered. For instance, a call
// to init_modules() would be performed from the Packet RX thread when master and a connection was
// negotiated with a slave. This call could potentially cause the Packet RX Thread to die, because some
// modules would attempt to send messages as a result of e.g. an init_modules(Add Switch) command,
// and wait for a semaphore that wouldn't get set before the module's msg_tx_done() callback was invoked.
// This could never be invoked for as long as the Packet RX Thread was waiting for the semaphore - and
// we would have a deadlock. In fact, I think it's bad code to block the calling thread like that (the
// port module currently does), but to initially overcome these problems, we add this thread here.
static cyg_handle_t IM_thread_handle;
static cyg_thread   IM_thread_state;
/* There're many initial procedure need to do, 14K stack size is using in SecureWebStax project,
   we expanded the stack size to 16K for the future */
static char         IM_thread_stack[2 * (THREAD_DEFAULT_STACK_SIZE)];
static cyg_flag_t   IM_flag; // Used to wake up the IM_thread

// For each ISID the FIFO can hold three entries: One Add, one Del, and one Default Conf.
// In addition, it can hold a master down or master up event.
#define MSG_IM_FIFO_SZ (3 * (VTSS_ISID_CNT) + 2)
#define MSG_IM_FIFO_NEXT(idx) ((idx) < ((MSG_IM_FIFO_SZ) - 1) ? (idx) + 1 : 0)
vtss_init_data_t IM_fifo[MSG_IM_FIFO_SZ];
int IM_fifo_cnt;
int IM_fifo_rd_idx;
int IM_fifo_wr_idx;

// Event flag that is used to wake up the TX_thread.
static cyg_flag_t TX_msg_flag;

// Event flag that is used to wake up the RX_thread.
static cyg_flag_t RX_msg_flag;

static msg_rx_filter_item_t *RX_filter_list = NULL;

// When calling back the User Module's Rx or Tx callback, we must not own
// the msg_state mutex. If we did, the User Module callback would not be able
// to call msg_tx() from within the callback function without a deadlock.
// Therefore, all calls to these callback functions are deferred until the
// internal state is completely updated, so that e.g. topo events can occur
// again without affecting the callback mechanism.
// The following two lists hold pending received and transmitted messages.
// The received messages are sent through the Rx dispatch mechanism, whereas
// the pending transmitted messages are sent back to the corresponding Tx Done
// user-callback functions.
// A simple mutex, crit_msg_pend_list, controls the insertion and deletion from
// the lists ready to be released. This mutex must not be held while the actual
// callback function is called, but only while inserting or removing a message
// from the list.
static msg_item_t *pend_rx_list, *pend_rx_list_last;
static msg_item_t *pend_tx_done_list, *pend_tx_done_list_last;

// Debug info
static vtss_module_id_t dbg_latest_rx_modid;
static u32              dbg_latest_rx_len;
static u32              dbg_latest_rx_connid;

#if VTSS_SWITCH_STACKABLE
// The same goes for the message SSP protocol header.
// The Message Protocol has SSPID = 0x0002
static const u8 exbit_protocol_ssp_msg[SSP_HDR_SZ_BYTES]  = {0x88, 0x80, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00};
#endif

// Cached version of this switch's MAC address
static mac_addr_t this_mac;

// The following macro returns TRUE if tick counter wraps around, or if diff in now_ticks and last_ticks
// exceeds the timeout_ms. FALSE otherwise.
#define MSG_TIMEDOUT(last_ticks, now_ticks, timeout_ms) ((now_ticks)<(last_ticks) ? TRUE : ((now_ticks)-(last_ticks)>=((timeout_ms)/(ECOS_MSECS_PER_HWTICK))))

// Compute the difference between a left-most SEQ number and a current,
// taking into account that the window may wrap around. This only works
// if SEQ is 16 bits wide.
#define SEQ_DIFF(left, cur) (((u32)cur >= (u32)left) ? ((u32)cur - (u32)left) : ((MSG_SEQ_CNT - (u32)left) + (u32)cur))

#define PDUTYPE2STR(p)  ((p) == MSG_PDU_TYPE_MSYN ? "MSYN" : (p) == MSG_PDU_TYPE_MSYNACK ? "MSYNACK" : (p) == MSG_PDU_TYPE_MD ? "MD" : (p) == MSG_PDU_TYPE_MACK ? "MACK" : "UNKNOWN")

/******************************************************************************/
//
/******************************************************************************/
struct msg_buf_pool_s;
typedef struct msg_buf_s {
    struct msg_buf_s      *next;
    struct msg_buf_pool_s *pool;
    u32                   ref_cnt;
    void                  *buf;
    /* Here, the message will get located */
} msg_buf_t;

/******************************************************************************/
//
/******************************************************************************/
typedef struct msg_buf_pool_s {
    u32                   magic;
    struct msg_buf_pool_s *next;
    vtss_module_id_t      module_id;
    u32                   buf_cnt_init;
    u32                   buf_cnt_cur;
    u32                   buf_cnt_min;
    u32                   buf_size;
    u32                   allocs;
    i8                    *dscr;
    vtss_os_sem_t         sem;
    msg_buf_t             *free;
    msg_buf_t             *used;
} msg_buf_pool_t;

static msg_buf_pool_t *MSG_buf_pool;

// Note: Use an uintptr_t here to get over size differences
// when converting from address to scalar on 32- and 64-bit machines.
#define MSG_ALIGN64(x) (8 * (((uintptr_t)(x) + 7) / 8))

#define MSG_BUF_POOL_MAGIC 0xbadebabe

/****************************************************************************/
// msg_flash_switch_info_t
// Holds various info about a given switch in the stack. This info is saved
// to flash and loaded at master up event from topo.
// It facilitates transferring info about all previously seen switches in the
// stack to other modules (read: the port module) in master up events.
// In this way, the port module can provide the correct port count and stack
// port numbers to other modules, and the message module can tell whether
// the switch is configurable or not.
/****************************************************************************/
typedef struct {
    u32 version; // Version of this structure in the flash.

    init_switch_info_t info[VTSS_ISID_END]; // VTSS_ISID_LOCAL unused.
} msg_flash_switch_info_t;

static msg_flash_switch_info_t CX_switch_info;

// When a stacking build is configured for standalone,
// we expose CX_switch_info.info[X].configurable as FALSE for
// all switches, but the switch itself.
// However, in order to be able to preserve the switch info
// whenever we write back to flash, we need to save a copy of
// the "real" configurable flag. This is what this array is for.
static BOOL CX_switch_really_configurable[VTSS_ISID_END];

#define MSG_FLASH_SWITCH_INFO_VER 1

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// CX_event_init()
/****************************************************************************/
static void CX_event_init(vtss_init_data_t *event)
{
    memset(event, 0, sizeof(*event));
}

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// CX_create_vs2_hdr()
/******************************************************************************/
static void CX_create_vs2_hdr(vtss_vstax_upsid_t upsid, u8 *bin_hdr)
{
    vtss_vstax_tx_header_t vs2_hdr;
    vtss_rc                rc;

    // Since the VStaX2 header is invariant in the message protocol, have it
    // converted to a string of bytes once and for all.
    memset(&vs2_hdr, 0, sizeof(vs2_hdr));
#if MSG_HOP_BY_HOP
    vs2_hdr.fwd_mode    = VTSS_VSTAX_FWD_MODE_CPU_ALL; // Frame goes to the CPU...
    vs2_hdr.ttl         = 1; // ...on the neighboring switch...
    vs2_hdr.upsid       = 1;
#if defined(VTSS_FEATURE_VSTAX_V2)
    vs2_hdr.keep_ttl    = 1; // Don't decrement TTL on secondary units in 48-port solutions.
#endif
#else
    vs2_hdr.fwd_mode    = VTSS_VSTAX_FWD_MODE_CPU_UPSID; // Frame goes to the CPU...
    vs2_hdr.upsid       = upsid; // ...on this switch.
    vs2_hdr.ttl         = 31;
#endif
#if defined(VTSS_ARCH_JAGUAR_1) && !MSG_HOP_BY_HOP
    // JR's super-priority queues are too tiny for swift operation.
    // Furthermore, the SP relay buffers are shared with CMEF, which
    // causes problems when CMEF is enabled. Therefore, we transmit
    // on priority 7, which is reserved for stack management.
    vs2_hdr.prio        = 7;
#else
    vs2_hdr.prio        = VTSS_PRIO_SUPER; // ...using this switch's super priority Tx queue and the neighboring switch SP Rx queue (if supported)
#endif
    vs2_hdr.tci.vid     = 1;
    vs2_hdr.tci.cfi     = 0;
    vs2_hdr.tci.tagprio = 7; // If forwarding on super-priority, hop-by-hop, this has no significance. If forwarding VTSS_VSTAX_FWD_MODE_CPU_UPSID non-super-priority, this determines the priority on the receiving switch unit, and if ANA_CL:COMMON:IFH_CFG.VSTAX_OWN_ONLY_ENA == 0, also on the units in between.
    vs2_hdr.glag_no     = VTSS_GLAG_NO_NONE;
    vs2_hdr.port_no     = 0;
    vs2_hdr.queue_no    = PACKET_XTR_QU_STACK; // Let it go to the CPU extraction queue allocated for various stack protocols if the chip doesn't support super-prio Rx.
    rc = vtss_packet_vstax_header2frame(0, 0, &vs2_hdr, bin_hdr);
    VTSS_ASSERT(rc >= 0);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// CX_update_upsid()
/****************************************************************************/
static void CX_update_upsid(msg_mcb_t *mcb, vtss_vstax_upsid_t upsid)
{
    mcb->upsid = upsid;
    CX_create_vs2_hdr(upsid, mcb->vs2_frm_hdr);
}
#endif

/****************************************************************************/
/*                                                                          */
/*  INIT MODULES HELPER INTERNAL FUNCTIONS                                  */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// This one holds the state reported to the user modules.
// The idea is that once a SWITCH_ADD event is passed to the user modules,
// msg_switch_exists() returns TRUE whether or not the switch goes away
// during this event.
// Likewise, once a MASTER_UP event is passed to the user modules,
// msg_switch_is_master() returns TRUE throughout the event whether or not
// the message module protocol knows that it is now a slave.
//
// The structure also holds a cached version of the master ISID.
//
// Besides caching the info from the message module protocol, it also serves
// to avoid race-conditions as follows:
//
// Suppose you have two unconnected switches. Since the switches are not connected
// to each other, they are both masters. Now, connect them and observe the
// following timeline (supposing switch #1 becomes the new master of the stack):
//
// Time Switch #1            Switch #2
// ---- -------------------- --------------------
//    0 No event             No event
//    1 No event             SWITCH_DEL(isid = 2)
//    2 SWITCH_ADD(isid = 2) No event
//    3 No event             MASTER_DOWN
//
// Here, it is assumed that the SWITCH_DEL event on the upcoming slave takes quite
// a while to pump through the user modules. So in time step 0, 1, and 2, the
// upcoming slave still believes it's master, whereas switch #1 thinks that from
// time step 2, switch #2 is slave, which means that switch #2 may discard messages
// from switch #1 in this period of time (at the user module level; at the message
// module level, switch #2 indeed knows that it is slave).
//
// This race condition is solved by holding back transmission of MSYNACKS from
// switch #2 towards switch #1 until the MASTER_DOWN event has been passed to
// all user modules. Switch #1 will retransmit MSYN packets until switch #2
// starts replying. The new timeline will therefore become:
//
// Time Switch #1            Switch #2
// ---- -------------------- --------------------
//    0 No event             No event
//    1 No event             SWITCH_DEL(isid = 2)
//    2 No event             MASTER_DOWN
//    3 No event             (start replying to MSYNs)
//    3 SWITCH_ADD(isid = 2) No event
//
// The structure is protected by MSG_STATE_CRIT_ENTER()/EXIT()
/****************************************************************************/
static struct {
    BOOL        master;
    BOOL        exists[VTSS_ISID_END];
    vtss_isid_t master_isid; // VTSS_ISID_UNKNOWN when not master or when the first SWITCH_ADD event hasn't happened.
    BOOL        ignore_msyns;
} CX_user_state;

/****************************************************************************/
// IM_user_state_update()
/****************************************************************************/
static void IM_user_state_update(vtss_init_data_t *event)
{
    MSG_STATE_CRIT_ENTER();

    switch (event->cmd) {
    case INIT_CMD_MASTER_UP:
        CX_user_state.master = TRUE;
        CX_user_state.master_isid = VTSS_ISID_UNKNOWN; // Hardly needed.
        break;

    case INIT_CMD_MASTER_DOWN:
        CX_user_state.master = FALSE;
        CX_user_state.master_isid = VTSS_ISID_UNKNOWN;
        break;

    case INIT_CMD_SWITCH_ADD:
        CX_user_state.exists[event->isid] = TRUE;
        if (CX_user_state.master_isid == VTSS_ISID_UNKNOWN) {
            // The first added switch is the master.
            CX_user_state.master_isid = event->isid;
        }
        break;

    case INIT_CMD_SWITCH_DEL:
        CX_user_state.exists[event->isid] = FALSE;
        if (CX_user_state.master_isid == event->isid) {
            CX_user_state.master_isid = VTSS_ISID_UNKNOWN;
        }
        break;

    default:
        break;
    }

    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// IM_thread()
// The thread that dispatches state changes to other modules through the
// init_modules() function.
/****************************************************************************/
static void IM_thread(cyg_addrword_t thr_data)
{
    vtss_init_data_t event;




    // We've taken over the role of calling init_modules with INIT_CMD_START
    // from the main_thread in main.c. The reason for this is that if it
    // wasn't so, then it might happen that TOPO was running before the main
    // thread, providing the default master-up to the message module, which
    // in turn would send it to this thread, which in turn would call
    // init_modules with MASTER_UP - possibly before main had called it with
    // INIT_CMD_START.
    CX_event_init(&event);
    event.cmd = INIT_CMD_START;
    (void)init_modules(&event);

    while (1) {



























        (void)cyg_flag_wait(&IM_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);


        // Empty the FIFO.
        MSG_IM_FIFO_CRIT_ENTER();

        while (IM_fifo_cnt > 0) {
            event = IM_fifo[IM_fifo_rd_idx];
            IM_fifo_rd_idx = MSG_IM_FIFO_NEXT(IM_fifo_rd_idx);
            IM_fifo_cnt--;
            MSG_IM_FIFO_CRIT_EXIT();

            IM_user_state_update(&event);

            if (MSG_TRACE_ENABLED(event.isid, -1)) {
                T_IG(TRACE_GRP_INIT_MODULES, "Enter. cmd=%s, isid=%u", control_init_cmd2str(event.cmd), event.isid);
            }
            (void)init_modules(&event);
            if (MSG_TRACE_ENABLED(event.isid, -1)) {
                T_IG(TRACE_GRP_INIT_MODULES, "Exit. cmd=%s, isid=%u", control_init_cmd2str(event.cmd), event.isid);
            }

            if (event.cmd == INIT_CMD_MASTER_DOWN) {
                // Time to no longer ignore MSYNs, since the user modules have
                // been passed the info of a master down (see thoughts about this
                // right above the CX_user_state structure).
                MSG_STATE_CRIT_ENTER();
                CX_user_state.ignore_msyns = FALSE;
                MSG_STATE_CRIT_EXIT();
                T_IG(TRACE_GRP_INIT_MODULES, "Accepting MSYNs");
            }

            // Prepare for next loop
            MSG_IM_FIFO_CRIT_ENTER();
        }

        // Done
        MSG_IM_FIFO_CRIT_EXIT();
    }
}

/******************************************************************************/
// IM_fifo_put()
/******************************************************************************/
static BOOL IM_fifo_put(vtss_init_data_t *new_event)
{
    BOOL result;

    T_IG(TRACE_GRP_INIT_MODULES, "Add(FIFO): cmd=%s, parm=%d", control_init_cmd2str(new_event->cmd), new_event->isid);
    MSG_IM_FIFO_CRIT_ENTER();

    if (IM_fifo_cnt == MSG_IM_FIFO_SZ) {
        // FIFO is full.
        vtss_init_data_t latest_data;
        char             *latest_init_module_func_name;
        control_dbg_latest_init_modules_get(&latest_data, &latest_init_module_func_name);
        T_EG(TRACE_GRP_INIT_MODULES, "IM_fifo is full (cmd=%s, isid=%d). Latest init_modules props: cmd=%s, isid=%u flags=0x%x, init-func=%s", control_init_cmd2str(new_event->cmd), new_event->isid, control_init_cmd2str(latest_data.cmd), latest_data.isid, latest_data.flags, latest_init_module_func_name);
        result = FALSE;
    } else {
        IM_fifo[IM_fifo_wr_idx] = *new_event;
        IM_fifo_wr_idx = MSG_IM_FIFO_NEXT(IM_fifo_wr_idx);
        IM_fifo_cnt++;
        result = TRUE;
    }

    // Wake up the IM_thread()
    cyg_flag_setbits(&IM_flag, 1);
    MSG_IM_FIFO_CRIT_EXIT();
    return result;
}

/****************************************************************************/
/*                                                                          */
/*  COMMON INTERNAL FUNCTIONS, PART 1                                       */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// TX_wake_up_msg_thread()
/****************************************************************************/
static void TX_wake_up_msg_thread(int flag)
{
    // Wake-up the TX_thread().
    cyg_flag_setbits(&TX_msg_flag, flag);
}

/****************************************************************************/
// RX_wake_up_msg_thread()
/****************************************************************************/
static void RX_wake_up_msg_thread(int flag)
{
    // Wake-up the RX_thread().
    cyg_flag_setbits(&RX_msg_flag, flag);
}

/****************************************************************************/
// CX_uptime_secs_get()
// Returns the number of seconds that has elapsed since boot.
/****************************************************************************/
static u32 CX_uptime_secs_get(void)
{
    cyg_tick_count_t uptime_64;
    uptime_64 = cyg_current_time() / (1000 / ECOS_MSECS_PER_HWTICK);
    return (u32)uptime_64;
}

/****************************************************************************/
// CX_uptime_get_crit_taken()
// Returns the up-time measured in seconds of a switch.
// The MSG_STATE_CRIT_ENTER() must have been taken prior to this call.
// See msg_uptime_get() for a thorough description.
/****************************************************************************/
static u32 CX_uptime_get_crit_taken(vtss_isid_t isid)
{
    u32 cur_uptime;

    VTSS_ASSERT(VTSS_ISID_LEGAL(isid) || isid == VTSS_ISID_LOCAL);

    if (isid == VTSS_ISID_LOCAL) {
        // Just use the current time.
        return CX_uptime_secs_get();
    } else if (state.state == MSG_MOD_STATE_MST && mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
        // We're master and the connection is established.
        cur_uptime = CX_uptime_secs_get();
        if (cur_uptime < mcbs[isid][0].u.master.switch_info.mst_uptime_secs) {
            // This may occur at roll over (after 136 years).
            T_W("Current uptime (%u) is smaller than that of connection establishment (%u)", cur_uptime, mcbs[isid][0].u.master.switch_info.mst_uptime_secs);
            cur_uptime = mcbs[isid][0].u.master.switch_info.mst_uptime_secs;
        }

        return (mcbs[isid][0].u.master.switch_info.slv_uptime_secs + (cur_uptime - mcbs[isid][0].u.master.switch_info.mst_uptime_secs));
    }

    // We're not master or the slave is not connected.
    return 0;
}

/******************************************************************************/
// CX_concat_msg_items()
// Concatenates the list pointed to by msg_first to list_last, and updates
// list (if needed) and list_last.
// msg_first may be NULL, i.e. this function can be called even when there are
// no messages to concatenate to an existing or empty list.
/******************************************************************************/
static void CX_concat_msg_items(msg_item_t **list, msg_item_t **list_last, msg_item_t *msg_first, msg_item_t *msg_last)
{
    if (!msg_first) {
        return; // Nothing to concatenate.
    }

    VTSS_ASSERT(msg_last->next == NULL);

    // Link it in
    if (*list_last == NULL) {
        // No items currently in list.
        VTSS_ASSERT(*list == NULL);
        *list      = msg_first;
    } else {
        // Already items in the list. Append to it.
        VTSS_ASSERT(*list && (*list_last)->next == NULL);
        (*list_last)->next = msg_first;
    }

    *list_last = msg_last;
}

/****************************************************************************/
// CX_get_pend_list()
/****************************************************************************/
static msg_item_t *CX_get_pend_list(msg_item_t **pend_list, msg_item_t **pend_list_last)
{
    msg_item_t *result;
    // Get list mutex before messing with the list.
    MSG_PEND_LIST_CRIT_ENTER();
    result = *pend_list;

    if (*pend_list) {
        *pend_list = (*pend_list)->next;
    }

    if (*pend_list == NULL) {
        *pend_list_last = NULL;
    }

    MSG_PEND_LIST_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// CX_put_pend_list()
/****************************************************************************/
static void CX_put_pend_list(msg_item_t **pend_list, msg_item_t **pend_list_last, msg_item_t *msg_list, msg_item_t *msg_list_last)
{
    // Get list mutex before messing with the list.
    MSG_PEND_LIST_CRIT_ENTER();
    CX_concat_msg_items(pend_list, pend_list_last, msg_list, msg_list_last);
    MSG_PEND_LIST_CRIT_EXIT();
}

/******************************************************************************/
// CX_switch_info_valid()
/******************************************************************************/
static BOOL CX_switch_info_valid(init_switch_info_t *info)
{
    if (info->configurable) {
        if (info->port_cnt > VTSS_PORTS) {
            T_E("Invalid port count");
            return FALSE;
        }

        if (info->stack_ports[0] == VTSS_PORT_NO_NONE) {
            // Not stackable. Only possible if the info is ourselves.
            if (info->stack_ports[1] != VTSS_PORT_NO_NONE) {
                T_E("Invalid stack port(1)");
                return FALSE;
            }
        } else if (info->stack_ports[0] >= info->port_cnt ||
                   info->stack_ports[1] >= info->port_cnt ||
                   info->stack_ports[0] == info->stack_ports[1]) {
            T_E("Invalid stack port(s)");
            return FALSE;
        }
    }

    return TRUE;
}

/****************************************************************************/
// CX_init_switch_info_get()
/****************************************************************************/
static void CX_init_switch_info_get(init_switch_info_t *switch_info)
{
    port_isid_info_t port_info;

    // Get local information
    if (port_isid_info_get(VTSS_ISID_LOCAL, &port_info) != VTSS_RC_OK) {
        T_E("port_isid_info_get() failed. Building our own");
        // Only port_count will become wrong here:
        port_info.board_type   = vtss_board_type();
        port_info.port_count   = VTSS_PORTS;
#if VTSS_SWITCH_STACKABLE
        port_info.stack_port_0 = PORT_NO_STACK_0;
        port_info.stack_port_1 = PORT_NO_STACK_1;
#else
        port_info.stack_port_0 = VTSS_PORT_NO_NONE;
        port_info.stack_port_1 = VTSS_PORT_NO_NONE;
#endif
    }

    switch_info->configurable   = TRUE;
    switch_info->port_cnt       = port_info.port_count;
    switch_info->stack_ports[0] = port_info.stack_port_0;
    switch_info->stack_ports[1] = port_info.stack_port_1;
    switch_info->board_type     = port_info.board_type;
    switch_info->api_inst_id    = vtss_api_chipid();

    // Check that what we got from the port module indeed is valid.
    // The function prints the necessary errors.
    (void)CX_switch_info_valid(switch_info);
}

/****************************************************************************/
// CX_local_port_map_get()
/****************************************************************************/
static void CX_local_port_map_get(u32 port_cnt, init_port_map_t *init_port_map)
{
    vtss_port_map_t api_port_map[VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t  port_no;
    vtss_rc         rc;

    VTSS_ASSERT(port_cnt <= VTSS_PORT_ARRAY_SIZE);

    // Initialize to -1 to indicate that the ports don't exist.
    for (port_no = 0; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        // -1 indicates that it doesn't exist.
        init_port_map[port_no].chip_port = -1;
    }

    if ((rc = vtss_port_map_get(NULL, api_port_map)) != VTSS_RC_OK) {
        // The consequence of this is that MAC and aggr modules will fail.
        T_E("Unable to get port map (%s)", error_txt(rc));
        return;
    }

    for (port_no = VTSS_PORT_NO_START; port_no < port_cnt + VTSS_PORT_NO_START; port_no++) {
        init_port_map[port_no].chip_port = api_port_map[port_no].chip_port;
        init_port_map[port_no].chip_no   = api_port_map[port_no].chip_no;
    }
}

/****************************************************************************/
// CX_local_switch_info_get()
/****************************************************************************/
static void CX_local_switch_info_get(msg_switch_info_t *switch_info)
{
    u32 uptime;

    memset(switch_info, 0, sizeof(*switch_info));

    // Local version string
    strncpy(switch_info->version_string, misc_software_version_txt(), MSG_MAX_VERSION_STRING_LEN); // From version.h
    switch_info->version_string[MSG_MAX_VERSION_STRING_LEN - 1] = '\0';

    // Local product name
    strncpy(switch_info->product_name, misc_product_name(), MSG_MAX_PRODUCT_NAME_LEN); // From version.h
    switch_info->product_name[MSG_MAX_PRODUCT_NAME_LEN - 1] = '\0';

    // Local uptime
    uptime = CX_uptime_secs_get();
    switch_info->slv_uptime_secs = uptime;
    switch_info->mst_uptime_secs = uptime; // May not be used.

    // Other info
    CX_init_switch_info_get(&switch_info->info);

    // Logical-to-physical port mapping
    CX_local_port_map_get(switch_info->info.port_cnt, switch_info->port_map);
}

/******************************************************************************/
// CX_flash_do_write()
/******************************************************************************/
static void CX_flash_do_write(msg_flash_switch_info_t *flash_info)
{
    vtss_isid_t isid;

    if (flash_info) {
        *flash_info = CX_switch_info;

        // Copy the shadow configurable flags back into the flash_info structure.
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            flash_info->info[isid].configurable = CX_switch_really_configurable[isid];
        }

        flash_info->version = MSG_FLASH_SWITCH_INFO_VER;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MSG);
    }
}

/******************************************************************************/
// CX_flash_write()
/******************************************************************************/
static void CX_flash_write(void)
{
    msg_flash_switch_info_t *flash_info;
    ulong                   size;

    MSG_STATE_CRIT_ASSERT_LOCKED();

    if ((flash_info = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MSG, &size)) == NULL || size != sizeof(*flash_info)) {
        T_W("Failed to open flash configuration");
        return;
    }

    CX_flash_do_write(flash_info);
}

/******************************************************************************/
// CX_flash_read()
// master_isid is only used if creating defaults, and only if it is a valid isid.
// If it's an invalid ISID, CX_switch_info() is not overwritten if defaults
// are created.
/******************************************************************************/
static void CX_flash_read(vtss_isid_t master_isid)
{
    msg_flash_switch_info_t *flash_info;
    ulong                   size;
    vtss_isid_t             isid;
    BOOL                    create_defaults = FALSE;

    if ((flash_info = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MSG, &size)) == NULL || size != sizeof(*flash_info)) {
        T_W("conf_sec_open() failed or size mismatch. Creating defaults.");
        flash_info = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MSG, sizeof(*flash_info));
        create_defaults = TRUE;
    } else if (flash_info->version != MSG_FLASH_SWITCH_INFO_VER) {
        T_W("Version mismatch. Creating defaults.");
        create_defaults = TRUE;
    }

    if (!create_defaults && flash_info) {
        // Integrity check.
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (!CX_switch_info_valid(&flash_info->info[isid])) {
                create_defaults = TRUE;
                break;
            }
        }
    }

    MSG_STATE_CRIT_ASSERT_LOCKED();

    if (create_defaults) {
        if (VTSS_ISID_LEGAL(master_isid)) {
            // Prevent this function from updating CX_switch_info if
            // called with an illegal isid (since that's for debugging only).
            memset(&CX_switch_info, 0, sizeof(CX_switch_info));
        }
    } else if (flash_info) {
        CX_switch_info = *flash_info;
    }

    // Always update the local switch info section, provided we're called
    // with a valid ISID. If we didn't update it, it could happen that two
    // different builds running on the same board would cause problems
    // when loading the second build, when properties from the first build
    // were successfully loaded from flash.
    if (VTSS_ISID_LEGAL(master_isid)) {
        CX_init_switch_info_get(&CX_switch_info.info[master_isid]);
    }

    // If stacking is disabled, we copy all configurable flags from
    // the "public" structure into a local-only one, and clear
    // the "public" configurable flag for all-but-the-master switch
    // (this local switch). This ensures that we can say "no" to
    // msg_switch_configurable() when invoked with an ISID that is not
    // ourselves in standalone mode. When (if) configuration is saved back
    // into flash, these flags are restored in the flash copy.
    if (VTSS_ISID_LEGAL(master_isid)) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            CX_switch_really_configurable[isid] = CX_switch_info.info[isid].configurable;
            if (isid != master_isid && !vtss_stacking_enabled()) {
                // Pretend that slaves are not configurable in standalone mode
                CX_switch_info.info[isid].configurable = FALSE;
            }
        }
    }

    CX_flash_do_write(flash_info);
}

/****************************************************************************/
// CX_event_conf_def()
/****************************************************************************/
static BOOL CX_event_conf_def(vtss_isid_t isid, init_switch_info_t *new_info)
{
    vtss_init_data_t event;

    CX_event_init(&event);
    event.cmd  = INIT_CMD_CONF_DEF;
    event.isid = isid;
    event.switch_info[isid] = *new_info;
    return IM_fifo_put(&event);
}

/******************************************************************************/
// CX_switch_info_update()
/******************************************************************************/
static BOOL CX_switch_info_update(vtss_isid_t isid, init_switch_info_t *new_info)
{
    init_switch_info_t *cur_info = &CX_switch_info.info[isid];

    MSG_STATE_CRIT_ASSERT_LOCKED();

    if (CX_switch_really_configurable[isid]) {
        // The switch has already been seen previously.
        // Check to see if the new switch has the same configuration
        // as the previous. Check all fields, except for stack ports.
        if (new_info->port_cnt    != cur_info->port_cnt   ||
            new_info->board_type  != cur_info->board_type ||
            new_info->api_inst_id != cur_info->api_inst_id) {
            // There's differences. Gotta generate a conf-def event.
            if (!CX_event_conf_def(isid, new_info)) {
                // Don't even attempt to save the configuration
                // if the FIFO put operation failed, because we
                // really need to generate a conf-def event, and therefore
                // must re-negotiate the connection and try again the
                // next time it's negotiated. If we saved to flash
                // here, there would be no differences the next time
                // we get here, and the conf-def event would miss out.
                return FALSE;
            }
        }
    }

    // We save to flash whenever the info has changed.
    CX_switch_really_configurable[isid] = new_info->configurable;
    if (memcmp(new_info, cur_info, sizeof(*new_info)) != 0) {
        *cur_info = *new_info;
        CX_flash_write();
    }

    return TRUE;
}

/****************************************************************************/
// CX_event_switch_add()
/****************************************************************************/
static BOOL CX_event_switch_add(vtss_isid_t isid, init_switch_info_t *switch_info, init_port_map_t *port_map)
{
    vtss_init_data_t event;

    // First update our flash configuration. This may imply distributing
    // a conf-def event.
    if (!CX_switch_info_update(isid, switch_info)) {
        return FALSE;
    }

    CX_event_init(&event);
    event.cmd  = INIT_CMD_SWITCH_ADD;
    event.isid = isid;
    event.switch_info[isid] = *switch_info;
    memcpy(event.port_map, port_map, sizeof(event.port_map));
    return IM_fifo_put(&event);
}

/****************************************************************************/
// RX_put_list()
/****************************************************************************/
static void RX_put_list(msg_mcb_t *mcb, msg_item_t *msg_list, msg_item_t *msg_list_last)
{
    msg_item_t *msg = msg_list;

    while (msg) {
        mcb->stat.rx_msg++; // Gotta increase the counter here, because when the msg has been moved, we lose the information of origin.
        if (msg->is_tx_msg) {
            // Count loopbacks in the Tx list as well.
            mcb->stat.tx_msg[0]++;
        }
        msg = msg->next;
    }

    CX_put_pend_list(&pend_rx_list, &pend_rx_list_last, msg_list, msg_list_last);

    // Wake up the RX_thread(). This may cause a "spurious" wake-up, since
    // RX_put_list() may be called from the TX_thread() in the loop-back case,
    // but this doesn't matter.
    RX_wake_up_msg_thread(MSG_FLAG_RX_MSG);
}

/****************************************************************************/
// TX_put_done_list()
/****************************************************************************/
static void TX_put_done_list(msg_mcb_t *mcb, msg_item_t *msg_list, msg_item_t *msg_list_last)
{
    msg_item_t *msg = msg_list;

    if (mcb) {
        while (msg) {
            mcb->stat.tx_msg[msg->u.tx.rc == MSG_TX_RC_OK ? 0 : 1]++;
            msg = msg->next;
        }
    }

    CX_put_pend_list(&pend_tx_done_list, &pend_tx_done_list_last, msg_list, msg_list_last);

    TX_wake_up_msg_thread(MSG_FLAG_TX_DONE_MSG);
}

/****************************************************************************/
// TX_put_done_list_front()
// Used to un-get a tx done message if it wasn't acknowledged Tx'd by the FDMA.
/****************************************************************************/
static MSG_INLINE void TX_put_done_list_front(msg_item_t *msg)
{
    if (!msg) {
        return;
    }

    MSG_PEND_LIST_CRIT_ENTER();
    msg->next = pend_tx_done_list;
    if (pend_tx_done_list_last == NULL) {
        pend_tx_done_list_last = msg;
    }

    pend_tx_done_list = msg;
    MSG_PEND_LIST_CRIT_EXIT();
}

/******************************************************************************/
// CX_mcb_flush()
// Moves the mcb's tx_msg_list to the global pend_tx_done_list while assigning it
// an error number. Doing so causes the TX_thread() to callback any user-
// defined Tx done handlers outside the holding of the msg_state_crit
// semaphore.
// The function also releases any received MDs.
// May be called in both slave and master mode.
/******************************************************************************/
static void CX_mcb_flush(msg_mcb_t *mcb, msg_tx_rc_t reason)
{
    msg_item_t *tx_msg, *rx_msg;

    VTSS_ASSERT(reason != MSG_TX_RC_OK);

    MSG_STATE_CRIT_ASSERT_LOCKED();

    // Move the TX message list to the Tx Done list, which also serves
    // as holding messages not Tx'd.
    if (mcb->tx_msg_list) {
        tx_msg = mcb->tx_msg_list;

        // Walk through the list of unsent/unacknowledged user messages and
        // assign the rc member the value in reason, so that the user function
        // can get called back with the right explanation.
        while (tx_msg) {
            tx_msg->u.tx.rc = reason;
            tx_msg = tx_msg->next;
        }

        TX_put_done_list(mcb, mcb->tx_msg_list, mcb->tx_msg_list_last);

        // No more pending messages for this connection.
        mcb->tx_msg_list      = NULL;
        mcb->tx_msg_list_last = NULL;
    }

    // Also flush the RX message list.
    rx_msg = mcb->rx_msg_list;
    while (rx_msg) {
        msg_item_t *rx_msg_next = rx_msg->next;
        VTSS_FREE(rx_msg->usr_msg);
        VTSS_FREE(rx_msg);
        rx_msg = rx_msg_next;
    }

    mcb->rx_msg_list = NULL;
}

/******************************************************************************/
// CX_set_state_mst_no_slv()
/******************************************************************************/
static void CX_set_state_mst_no_slv(msg_mcb_t *mcb)
{
    mcb->state = MSG_CONN_STATE_MST_NO_SLV;
    // Clear MAC address so that we don't get false hits when receiving MSYNACKs or MDs.
    memset(mcb->dmac, 0, sizeof(mac_addr_t));
#if VTSS_SWITCH_STACKABLE
    // Also clear the VStaX2 Header and UPSID.
    memset(mcb->vs2_frm_hdr, 0, sizeof(mcb->vs2_frm_hdr));
    mcb->upsid = VTSS_VSTAX_UPSID_UNDEF;
#endif
}

/******************************************************************************/
// CX_restart()
/******************************************************************************/
static void CX_restart(msg_mcb_t *mcb, vtss_isid_t isid, msg_tx_rc_t slv_reason, msg_tx_rc_t mst_reason)
{
    T_I("Restarting. Master reason: %d, Slave reason: %d", slv_reason, mst_reason);

    if (state.state == MSG_MOD_STATE_SLV) {
        // If we're slave, we go to the "No Master" state.
        // Eventually the master will notice that it cannot get in
        // contact with us and re-negotiate the connection.
        CX_mcb_flush(mcb, slv_reason);
        mcb->state = MSG_CONN_STATE_SLV_NO_MST;
    } else {
        vtss_init_data_t event;
        // In master state, we go to the "start Txing MSYNs" to re-negotiate connection.
        CX_mcb_flush(mcb, mst_reason);
        // Clear the DMAC, so that we don't get false hits when receiving
        // MSYNACKs or MDs.
        memset(mcb->dmac, 0, sizeof(mac_addr_t));
        mcb->state = MSG_CONN_STATE_MST_RDY;
        // Notify user modules of the event
        CX_event_init(&event);
        event.cmd = INIT_CMD_SWITCH_DEL;
        event.isid = isid;
        (void)IM_fifo_put(&event);
    }
}

#if VTSS_SWITCH_STACKABLE && !MSG_HOP_BY_HOP
/******************************************************************************/
// CX_upsid_change()
/******************************************************************************/
static void CX_upsid_change(vtss_isid_t isid)
{
    vtss_vstax_upsid_t new_upsid;
    int                cid;

    // This is a callback handler installed by the message module in Topo.
    // It will be called whenever an UPSID of a given unit changes, but only on the master.
    // This happens rarely, but could e.g. happen when connecting two virgin stacks/units
    // that both use the same UPSID(s). Once the two stacks/units have been connected
    // once, the new UPSIDs will be persisted to flash so that subsequent boots of the
    // newly formed stack will re-use the new UPSIDs and therefore not give rise to
    // subsequent calls into this function.
    // There are a few other cases, where this can happen, but anyway, here's how
    // the case in principle should be handled:
    //
    // When the message module is forwarding non-hop-by-hop, it transmits messages
    // directly to destination switches using the destination switch's UPSID, so
    // if the UPSID changes, that switch will no longer be reachable.
    //
    // So in principle, we should do the following:
    //   1) If #isid is ourselves (master), we need to notify all currently connected slaves of the new UPSID.
    //      This is because the slaves know the UPSID of the master, which is used by a slave to send messages
    //      back to the current master.
    //   2) If #isid is a slave, we just need to update the slave MCB's UPSID and corresponding
    //      VStaX2 header. The slave doesn't require any changes, because it only uses the master's UPSID
    //      (conveyed through MSYN frames).
    //
    // In principle we could invent a new message protocol frame type to handle case 1) to convey
    // the new master UPSID to a slave. Or we could transmit a message with the new UPSID to the
    // slaves. But since this message will come last in the queue of other user messages,
    // the messages before our message will cause the connection to time out because the
    // slaves won't be able to transmit a reply back to us (because they use a wrong UPSID).
    // Sooner or later, we will find out that the slave doesn't respond and attempt to do
    // a re-negotiation of the connection.
    //
    // Since we don't want to invent a new message protocol frame type, we can as well re-negotiate
    // all connections right away. This will cause a number of SWITCH_DELETE and SWITCH_ADD events,
    // but I don't consider this a big deal, since the UPSID change event happens so rarely.

    T_IG(TRACE_GRP_TOPO, "Got UPSID change event for isid = %u", isid);

    MSG_STATE_CRIT_ENTER();
    if (state.state != MSG_MOD_STATE_MST) {
        T_WG(TRACE_GRP_TOPO, "We're not master. Exiting.");
        goto do_exit;
    }

    new_upsid = topo_isid_port2upsid(isid, VTSS_PORT_NO_NONE);
    if (!VTSS_VSTAX_UPSID_LEGAL(new_upsid)) {
        T_E("topo_isid_port2upsid() returned illegal upsid (%d) for isid = %u. Ignoring", new_upsid, isid);
        goto do_exit;
    }

    for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
        // Create new VStaX2 headers to be used for frames sent to the slave (if isid indicates a slave),
        // or to be sent in MSYN packets if isid is ourselves (the master).
        if (mcbs[isid][cid].state != MSG_CONN_STATE_MST_NO_SLV) {
            CX_update_upsid(&mcbs[isid][cid], new_upsid);
        } else {
            T_WG(TRACE_GRP_TOPO, "Skipping assigning upsid (%d) to isid (%d), since we don't have a connection towards it", new_upsid, isid);
            goto do_exit;
        }
    }

    if (isid == state.misid) {
        // We ourselves have changed UPSID. Restart all connections.
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (isid != state.misid) { // Don't re-negotiate connection with ourselves.
                for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                    if (mcbs[isid][cid].state != MSG_CONN_STATE_MST_NO_SLV) {
                        // We must enforce re-negotiation in all states but the MSG_CONN_STATE_MST_NO_SLV,
                        // because that's the only way we can enforce creation of new MSYN frames with updated master UPSID.
                        // The slave return code is not used, so it could be anything.
                        T_IG(TRACE_GRP_TOPO, "Re-negotiating connection towards isid = %u because of new master UPSID (%d)", isid, new_upsid);
                        CX_restart(&mcbs[isid][cid], isid, MSG_TX_RC_WARN_MST_UPSID_CHANGE, MSG_TX_RC_WARN_MST_UPSID_CHANGE);
                    }
                }
            }
        }
    } else {
        T_IG(TRACE_GRP_TOPO, "isid = %u is a slave. Assigning new upsid (%d) to it", isid, new_upsid);
    }

do_exit:
    MSG_STATE_CRIT_EXIT();
}
#endif

/******************************************************************************/
// CX_state2str()
/******************************************************************************/
static char *CX_state2str(msg_conn_state_t astate)
{
    switch (astate) {
    case MSG_CONN_STATE_SLV_NO_MST:
        return "No Master";

    case MSG_CONN_STATE_SLV_WAIT_FOR_MSYNACKS_MACK:
        return "W(MACK)";

    case MSG_CONN_STATE_SLV_ESTABLISHED:
        return "Connected";

    case MSG_CONN_STATE_MST_NO_SLV:
        return "No Slave";

    case MSG_CONN_STATE_MST_RDY:
        return "Connecting";

    case MSG_CONN_STATE_MST_WAIT_FOR_MSYNACK:
        return "W(MSYNACK)";

    case MSG_CONN_STATE_MST_ESTABLISHED:
        return "Connected";

    case MSG_CONN_STATE_MST_STOP:
        return "CID N/A";

    default:
        return "Unknown";
    }
}

/******************************************************************************/
// CX_free_msg()
// Frees the MDs pointed to by @msg, possibly calls back the user-defined
// callback function, possibly frees the user-message, and finally frees
// the msg structure itself.
// Therefore, after this call returns, @msg is no longer valid.
//
// NOTE 1:
// The @msg must be detached from the state and mcbs arrays before this function
// is called, so that other threads don't think it's still there, because it
// won't be after this call.
//
// NOTE 2: This function must not be called while owning the crit_msg_state.
// Failing to observe this rule could cause a deadlock if the user's callback
// function calls msg_tx() or other API functions that attempt to aquire the
// mutex.
/******************************************************************************/
static void CX_free_msg(msg_item_t *msg)
{

    if (msg->is_tx_msg) {
        msg_md_item_t *md;
        vtss_module_id_t dmodid = MIN(msg->dmodid, VTSS_MODULE_ID_NONE);

        // Free the message's list of MDs, which is only valid for TX messages.
        md = msg->u.tx.md_list;
        while (md) {
            msg_md_item_t *md_next = md->next;
            VTSS_FREE(md);
            md = md_next;
        }

        // Count the event in the msg->rc bin for the current module state.
        // state.state holds the current state, which is used as an index
        // into the state.state_stat(istics) array's tx member, which is an array
        // that contains an entry for every reason for the call to to this
        // function, i.e. whether it was Tx'd OK or not.
        MSG_COUNTERS_CRIT_ENTER();
        VTSS_ASSERT(msg->u.tx.rc < MSG_TX_RC_LAST_ENTRY);
        state.usr_stat[msg->state].tx_per_return_code[msg->u.tx.rc]++;
        state.usr_stat[msg->state].tx[dmodid][msg->u.tx.rc == MSG_TX_RC_OK ? 0 : 1]++;
        state.usr_stat[msg->state].txb[dmodid] += msg->len;
        MSG_COUNTERS_CRIT_EXIT();

        // Get mutex
        MSG_STATE_CRIT_ENTER();

        // Also decrement the current outstanding count if shaping the message
        if (msg->u.tx.opt & MSG_TX_OPT_SHAPE) {
            VTSS_ASSERT(msg_shaper[dmodid].current > 0);
            msg_shaper[dmodid].current--;
        }

        MSG_STATE_CRIT_EXIT();

        if (MSG_TRACE_ENABLED(msg->connid, dmodid)) {
            T_IG(TRACE_GRP_TX, "TxDone(Msg): (len=%u, did=%u, dmodid=%s, %s, rc=%u)", msg->len, msg->connid, vtss_module_names[dmodid], (msg->u.tx.opt & MSG_TX_OPT_DONT_FREE) ? "Owner frees" : "Msg Module frees", msg->u.tx.rc);
            T_NG(TRACE_GRP_TX, "len=%u (first %u bytes shown)", msg->len, MIN(96, msg->len));
            T_NG_HEX(TRACE_GRP_TX, msg->usr_msg, MIN(96, msg->len));
        }

        // Callback User Module if it wants the result.
        if (msg->u.tx.cb) {
            if (MSG_TRACE_ENABLED(msg->connid, dmodid)) {
                T_DG(TRACE_GRP_TX, "TxDone(Msg): Callback enter (len = %u, dmodid = %s)", msg->len, vtss_module_names[dmodid]);
            }

            msg->u.tx.cb(msg->u.tx.contxt, msg->usr_msg, msg->u.tx.rc);
            if (MSG_TRACE_ENABLED(msg->connid, msg->dmodid)) {
                T_DG(TRACE_GRP_TX, "TxDone(Msg): Callback exit (len = %u, dmodid = %s)", msg->len, vtss_module_names[dmodid]);
            }
        }

        // Free the memory used by the User Message if asked to.
        if ((msg->u.tx.opt & MSG_TX_OPT_DONT_FREE) == 0) {
            if (MSG_TRACE_ENABLED(msg->connid, dmodid)) {
                T_DG(TRACE_GRP_TX, "TxDone(Msg): Freeing user message");
            }
            VTSS_FREE(msg->usr_msg);
        }
    } else {
        // msg is an Rx message.
        // Free the user message, since it's dynamically allocated.
        VTSS_FREE(msg->usr_msg);
    }

    // Also free the msg structure itself
    VTSS_FREE(msg);
}

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// CX_mac2str()
/****************************************************************************/
static char *CX_mac2str(mac_addr_t mac)
{
    static char s[8][18];
    static uint i = 0;
    i = (i + 1) % 8;

    sprintf(s[i], "%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return s[i];
}
#endif

/****************************************************************************/
/*                                                                          */
/*  TX INTERNAL FUNCTIONS                                                   */
/*                                                                          */
/****************************************************************************/

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_add_byte()
/****************************************************************************/
static MSG_INLINE void TX_add_byte(u8 **ptr, u8 val)
{
    **ptr = val;
    *ptr += 1;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_add_1byte_tlv()
/****************************************************************************/
static void TX_add_1byte_tlv(u8 **ptr, u8 type, u8 val)
{
    MSG_ASSERT(type < 0x80, "type=%d", type);
    TX_add_byte(ptr, type);
    TX_add_byte(ptr, val);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_add_nbyte_tlv()
/****************************************************************************/
static void TX_add_nbyte_tlv(u8 **ptr, u8 type, u8 len, const u8 *val)
{
    MSG_ASSERT(type >= 0x80, "type=%d", type);
    MSG_ASSERT(len > 1, "len=%d", len);
    int i;

    TX_add_byte(ptr, type);
    TX_add_byte(ptr, len);
    for (i = 0; i < len; i++) {
        TX_add_byte(ptr, *val);
        val++;
    }
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_insert_hdrs()
// Inserts DMAC, SMAC, Exbit Protocol, VStaX2 header, Exbit Protocol, SSP
// for Message Protocol, and PDU Type, HTL and CID into location starting
// at *pkt_ptr.
/****************************************************************************/
static void TX_insert_hdrs(u8 **pkt_ptr, mac_addr_t dmac, u8 pdu_type, u8 cid)
{
    // Insert DMAC and SMAC
    memcpy(*pkt_ptr, dmac, sizeof(mac_addr_t));
    *pkt_ptr += sizeof(mac_addr_t);
    memcpy(*pkt_ptr, this_mac, sizeof(mac_addr_t));
    *pkt_ptr += sizeof(mac_addr_t);

    // Now insert the message SSP protocol header.
    memcpy(*pkt_ptr, exbit_protocol_ssp_msg, sizeof(exbit_protocol_ssp_msg));
    *pkt_ptr += sizeof(exbit_protocol_ssp_msg);

    // Finally insert the three Message Protocol mandatory fields
    // (PDU Type, HTL, and CID).
    TX_add_byte(pkt_ptr, pdu_type);
#if MSG_HOP_BY_HOP
    TX_add_byte(pkt_ptr, state.msg_cfg_htl_limit);
#else
    // If sending directly to the switch in question, there are no intermediate hops.
    TX_add_byte(pkt_ptr, 1);
#endif
    TX_add_byte(pkt_ptr, cid);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msyn_tx_done()
// MSYNs are statically allocated. Nothing to free.
/****************************************************************************/
static void TX_msyn_tx_done(void *context, packet_tx_done_props_t *props)
{
    if (!props->tx) {
        T_WG(TRACE_GRP_TX, "Tx(MSYN) failed");
    }
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_props_compose()
/****************************************************************************/
static void TX_props_compose(msg_mcb_t *mcb, packet_tx_props_t *tx_props, u8 *frm, size_t len, vtss_port_no_t dst_port, packet_tx_done_cb_t done_cb, void *context)
{
    packet_tx_props_init(tx_props);
    tx_props->packet_info.modid              = VTSS_MODULE_ID_MSG;
    tx_props->packet_info.frm[0]             = frm;
    tx_props->packet_info.len[0]             = len;
    tx_props->tx_info.dst_port_mask          = VTSS_BIT64(dst_port);
    tx_props->packet_info.tx_done_cb         = done_cb;
    tx_props->packet_info.tx_done_cb_context = context;
#if defined(VTSS_ARCH_JAGUAR_1) && !MSG_HOP_BY_HOP
    // JR's super-priority queues are too tiny for swift operation.
    // Furthermore, the SP relay buffers are shared with CMEF, which
    // causes problems when CMEF is enabled. Therefore, we transmit
    // on priority 7, which is reserved for stack management.
    tx_props->tx_info.cos = 7;
#else
    tx_props->tx_info.cos = 8; // Super-prio Tx.
#endif
    tx_props->tx_info.tx_vstax_hdr = VTSS_PACKET_TX_VSTAX_BIN;
    memcpy(tx_props->tx_info.vstax.bin, mcb->vs2_frm_hdr, sizeof(tx_props->tx_info.vstax.bin));
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_pkt()
/****************************************************************************/
static MSG_INLINE vtss_rc TX_pkt(packet_tx_props_t *tx_props)
{
    // On Jaguar, we use ANA_L3 to capture frames destined to the master
    // switch's MAC address. On a stable stack, all switches' L3 base address
    // register contains the MAC address of the current master switch.
    // Now, all frames with a DMAC that matches the L3 base address will be forwarded
    // to one of two CPU queues (PACKET_XTR_QU_MGMT_MAC and PACKET_XTR_QU_L3_OTHER).
    // On slaves, these two CPU queues will be forwarded by the chip to the current
    // master.
    // Now, suppose we change master. Since the new master's MAC address is forwarded
    // to all switches using the message protocol, the message protocol must
    // be up and running between the new master and the old master, which is now slave.
    // Message frames were traditionally forwarded with a DMAC matching the
    // destination switch. Suppose we continued with that. Then, when the frame
    // arrives at the new slave (which was master before), its DMAC will match
    // the new old master's DMAC, and since it's not an IP frame, it will therefore
    // be marked for forwarding to PACKET_XTR_QU_L3_OTHER. Also, since the frame's
    // VStaX header indicates that it must be forwarded to PACKET_XTR_QU_STACK, two
    // bits and the destination CPU queue mask will be set. The arbiter, however, is
    // already set up (by SPROUT, which runs super priority and hop-by-hop and therefore
    // has no problems in communicating with neighbor switch) to re-direct CPU queues
    // to the new master, and since the PACKET_XTR_QU_L3_OTHER queue bit is set, the
    // arbiter will simply send the frame back to the current master, but not generate
    // a copy for the new slave (old master). Therefore, the message protocol can never
    // negotiate a connection, which in turn means that the stack is not really up and
    // running even though SPROUT thinks so.
    // To overcome this, we have several options, one of which is to let a switch
    // disable L3 upon master down events and wait for the new master to re-enable.
    // Due to race conditions, this might be a bit dangerous.
    // Instead, it has been decided to let the message module use DMACs that can
    // never hit the L3 base address. This is OK because forwarding is based on the
    // VStaX header and not the DMAC. The DMAC is chosen to be the normal DMAC with
    // the multicast bit set. This piece of code takes care of that. The other places
    // that are changes are:
    //   1) TX_mac2port(), which makes sure to clear bit 40 of the DMAC before asking
    //      Topo for a destination port.
    //   2) RX_pkt(), which clears bit 40 before figuring out whether a given frame
    //      really is meant for this switch.
    tx_props->packet_info.frm[0][MSG_DMAC_OFFSET] |=  0x01U;
    tx_props->packet_info.frm[0][MSG_SMAC_OFFSET] &= ~0x01U;
    return packet_tx(tx_props);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_mac2port()
/****************************************************************************/
vtss_port_no_t TX_mac2port(mac_addr_t mac_addr)
{
    vtss_port_no_t port_no;
    u8 ch = mac_addr[0];

    mac_addr[0] &= ~0x01U;
    port_no = topo_mac2port(mac_addr);
    mac_addr[0] = ch;
    return port_no;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msyn_do_it()
/****************************************************************************/
static MSG_INLINE BOOL TX_msyn_do_it(msg_mcb_t *mcb, vtss_isid_t disid)
{
    vtss_port_no_t    port_no;
    packet_tx_props_t tx_props;

    // Get the port to transmit the frame onto
    port_no = TX_mac2port(&mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]);

    if (!PORT_NO_IS_STACK(port_no)) {
        // Probably the slave went down. Count the event and don't transmit
        // since that will cause a fatal error in the packet_tx() function.
        if (MSG_TRACE_ENABLED(disid, -1)) {
            T_WG(TRACE_GRP_TX, "Tx(MSYN): Invalid stack port number (%u). dmac=%s, mseq=%u", port_no, CX_mac2str(&mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), mcb->u.master.msyn.cur_mseq);
        }
        return FALSE;
    }

    TX_props_compose(mcb, &tx_props, &mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES], mcb->u.master.msyn.len, port_no, TX_msyn_tx_done, NULL);

    T_RG_HEX(TRACE_GRP_TX, &mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES], mcb->u.master.msyn.len);
    if (TX_pkt(&tx_props) != VTSS_RC_OK) {
        T_WG(TRACE_GRP_TX, "Tx(MSYN): Transmission failed");
        return FALSE;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msyn()
/****************************************************************************/
static void TX_msyn(msg_mcb_t *mcb, vtss_isid_t disid, cyg_tick_count_t cur_time)
{
    BOOL result;
    mcb->u.master.msyn.last_tx = cur_time;
    u32 tx_cnt_before_this_was_called = mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][0] + mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][1];

    if (tx_cnt_before_this_was_called > 0) {
        if (MSG_TRACE_ENABLED(disid, -1)) {
            T_WG(TRACE_GRP_TX, "Tx(MSYN): Re-transmitting (%u times in total, dmac=%s, mseq=%u, fragsz=%u, mwinsz=%u)", tx_cnt_before_this_was_called + 1, CX_mac2str(&mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), mcb->u.master.msyn.cur_mseq, MD_MAX_PAYLOAD_SZ_BYTES, mcb->swinsz);
        }
    } else {
        if (MSG_TRACE_ENABLED(disid, -1)) {
            T_IG(TRACE_GRP_TX, "Tx(MSYN): dmac=%s, mseq=%u, fragsz=%u, mwinsz=%u", CX_mac2str(&mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), mcb->u.master.msyn.cur_mseq, MD_MAX_PAYLOAD_SZ_BYTES, mcb->swinsz);
        }
    }

    result = TX_msyn_do_it(mcb, disid);

    // Update volatile per-connection statistics
    mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][result ? 0 : 1]++;

    // Update persistent overall statistics
    state.pdu_stat[state.state].tx[MSG_PDU_TYPE_MSYN][tx_cnt_before_this_was_called > 0 ? 1 : 0][result ? 0 : 1]++;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msyn_create()
/****************************************************************************/
static BOOL TX_msyn_create(msg_mcb_t *mcb, vtss_isid_t disid, u8 cid)
{
    u8      *pkt_ptr;
    u16     ush;
    vtss_rc rc;

    // Make a copy of the configuration's window size so that it can change
    // without affecting an existing connection.
    mcb->swinsz = state.msg_cfg_mst_winsz_per_slv;

    // Insert all bytes up to, but not including the PDU Type for the message
    // protocol.
    pkt_ptr = &mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET];

    // Ask Topo for the MAC addresses
    rc = topo_isid2mac(disid, mcb->dmac);
    if (rc < 0) {
        T_WG(TRACE_GRP_TX, "Tx(MSYN): topo_isid2mac() failed (rc=%d)", rc);
        // Cannot send the MSYN, since we don't have a DMAC.
        memset(mcb->dmac, 0, sizeof(mac_addr_t));
        return FALSE;
    }

    TX_insert_hdrs(&pkt_ptr, mcb->dmac, MSG_PDU_TYPE_MSYN, cid);

    // At this point, pkt_ptr must be advanced to the location of
    // the first TLV.
    VTSS_ASSERT(pkt_ptr == &mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES + MSG_TLV_OFFSET]);

    // Fill in the MSYN fields.

    // Source (our own) window size
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYN_TLV_TYPE_MWINSZ, mcb->swinsz);

    // Master's (our own) UPSID.
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYN_TLV_TYPE_MUPSID, (u8)mcbs[state.misid][0].upsid);

    // Source (our own) sequence number
    ush = htons(mcb->next_available_sseq); // Convert to network order.
    // Save the location of the mseq, so that we can quickly find the location
    // to update on possibly retransmits of this frame.
    mcb->u.master.msyn.mseq_tlv_ptr = pkt_ptr;
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYN_TLV_TYPE_MSEQ, 2, (u8 *)&ush);
    mcb->u.master.msyn.cur_mseq = mcb->next_available_sseq;
    mcb->next_available_sseq++;
#if MSG_SEQ_CNT <= 65535UL
    if (mcb->next_available_sseq == MSG_SEQ_CNT) {
        mcb->next_available_sseq = 0;
    }
#endif

    // Source (our own) promised fragment size.
    ush = htons(MD_MAX_PAYLOAD_SZ_BYTES); // Convert to network order.
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYN_TLV_TYPE_MFRAGSZ, 2, (u8 *)&ush);

    // End the TLV sequence
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYN_TLV_TYPE_NULL, 0);

    mcb->u.master.msyn.len = pkt_ptr - &mcb->u.master.msyn.frm[VTSS_FDMA_HDR_SIZE_BYTES];
    // Clear the number of times we've transmitted this MSYN.
    mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][0] = mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][1] = 0;
    // By now, we must not have spent more space than we've allocated.
    // If this one fails, you've added more TLVs. Increase the
    // MSYN_MAX_HDR_SZ_BYTES define accordingly.
    VTSS_ASSERT(mcb->u.master.msyn.len <= MSYN_MAX_SZ_BYTES);
    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msynack_tx_done_static()
// Memory for this frame is statically allocated. Don't release.
/****************************************************************************/
static void TX_msynack_tx_done_static(void *context, packet_tx_done_props_t *props)
{
    if (!props->tx) {
        T_WG(TRACE_GRP_TX, "Tx(MSYNACK - static) failed");
    }
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msynack_tx_done_dynamic()
// Memory for this frame is dynamically allocated. Release.
/****************************************************************************/
static void TX_msynack_tx_done_dynamic(void *context, packet_tx_done_props_t *props)
{
    if (!props->tx) {
        T_WG(TRACE_GRP_TX, "Tx(MSYNACK - dynamic) failed");
    }

    VTSS_ASSERT(context);
    packet_tx_free(context);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msynack_do_it()
// crit_msg_state must be taken prior to this call.
// @frm points to DMAC of frame.
// @frm_len doesn't include VTSS_FDMA_HDR_SIZE_BYTES and FCS
/****************************************************************************/
static MSG_INLINE BOOL TX_msynack_do_it(msg_mcb_t *mcb, u8 *frm, u32 frm_len, u16 sseq, msg_msynack_status_t status, packet_tx_done_cb_t cb)
{
    vtss_port_no_t    port_no;
    packet_tx_props_t tx_props;

    // Get the port to transmit the frame onto
    port_no = TX_mac2port(&frm[MSG_DMAC_OFFSET]);

    if (!PORT_NO_IS_STACK(port_no)) {
        // Probably the master went down. Don't transmit since that will cause a fatal
        // error in the packet_tx() function.
        T_WG(TRACE_GRP_TX, "Tx(MSYNACK): Invalid stack port number (%u)", port_no);
        return FALSE;
    }

    // Not all properties can be shown when transmitting MSYNACK, because it's not always
    // the one held in the MCB that is transmitted (if the CID is invalid).
    TX_props_compose(mcb, &tx_props, frm, frm_len, port_no, cb, frm);

    T_IG(    TRACE_GRP_TX, "Tx(MSYNACK): dmac=%s, sseq=%u, status=%u", CX_mac2str(&frm[MSG_DMAC_OFFSET]), sseq, status);
    T_RG_HEX(TRACE_GRP_TX, frm, frm_len);

    if (TX_pkt(&tx_props) != VTSS_RC_OK) {
        T_WG(TRACE_GRP_TX, "Tx(MSYNACK): Transmission failed");
        return FALSE;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_msynack()
// crit_msg_state must be taken prior to this call.
// @frm points to DMAC of frame.
// @frm_len doesn't include VTSS_FDMA_HDR_SIZE_BYTES and FCS
/****************************************************************************/
static BOOL TX_msynack(msg_mcb_t *mcb, u8 *frm, u32 frm_len, u16 sseq, msg_msynack_status_t status, packet_tx_done_cb_t cb)
{
    BOOL result = TX_msynack_do_it(mcb, frm, frm_len, sseq, status, cb);

    // Update volatile per-connection statistics
    if (mcb) { // In one case, the mcb is NULL, namely when no mcb is associated with the transmission when an MSYN is received with an unsupported CID.
        mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYNACK][result ? 0 : 1]++;
    }

    // Update persistent overall statistics
    state.pdu_stat[state.state].tx[MSG_PDU_TYPE_MSYNACK][0][result ? 0 : 1]++;
    return result;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// TX_msynack_create()
// crit_msg_state must be taken before entry.
// @frm points to DMAC
// Returns length of frame excluding VTSS_FDMA_HDR_SIZE_BYTES and FCS.
// Notes: Since this function may be called both with a static and dynamic
// memory allocation, and since we may not have an MCB, all arguments that need
// go into the frame are passed as arguments to this function and not through
// an MCB.
/******************************************************************************/
static u32 TX_msynack_create(u8 *frm, mac_addr_t dmac, u8 cid, u8 swinsz, u16 mseq, u16 sseq, msg_msynack_status_t status)
{
    u8                *pkt_ptr, log_to_phys_port_map[2 * VTSS_PORTS];
    u16               ush;
    u32               frm_len, ul;
    int               str_len;
    msg_switch_info_t switch_info;
    vtss_port_no_t    port_no;

    CX_local_switch_info_get(&switch_info);

    // Insert all bytes up to, but not including the PDU Type for the message
    // protocol.
    pkt_ptr = &frm[MSG_DMAC_OFFSET];
    TX_insert_hdrs(&pkt_ptr, dmac, MSG_PDU_TYPE_MSYNACK, cid);

    // At this point, pkt_ptr must be advanced to the location of
    // the first TLV.
    VTSS_ASSERT(pkt_ptr == &frm[MSG_TLV_OFFSET]);

    // Fill in the MSYNACK fields.

    // Our own Window Size
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_SWINSZ, swinsz);

    // Status
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_STATUS, status);

    // Our own port count
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_PORT_CNT,     switch_info.info.port_cnt);

    // Our left stack port
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_STACK_PORT_0, switch_info.info.stack_ports[0]);

    // Our right stack port
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_STACK_PORT_1, switch_info.info.stack_ports[1]);

    // Master's SEQ
    ush = htons(mseq); // Convert to network order.
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_MSEQ, 2, (u8 *)&ush);

    // Our own SEQ
    ush = htons(sseq);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_SSEQ, 2, (u8 *)&ush);

    // Version string
    str_len = strlen(switch_info.version_string);
    if (str_len >= 2) {
        // Can only transmit string if it's more than 1 byte long, since it would otherwise be a 1-byte TLV,
        // which is not supported for MSG_MSYNACK_TLV_TYPE_VERSION_STRING.
        TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_VERSION_STRING, MIN(str_len + 1, MSG_MAX_VERSION_STRING_LEN), (const u8 *)switch_info.version_string);
        *(pkt_ptr - 1) = '\0';
    }

    // Product name
    str_len = strlen(switch_info.product_name);
    if (str_len >= 2) {
        // Can only transmit string if it's more than 1 byte long, since it would otherwise be a 1-byte TLV,
        // which is not supported for MSG_MSYNACK_TLV_TYPE_PRODUCT_NAME.
        TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_PRODUCT_NAME, MIN(str_len + 1, MSG_MAX_PRODUCT_NAME_LEN), (const u8 *)switch_info.product_name);
        *(pkt_ptr - 1) = '\0';
    }

    // Uptime
    // We need to convert a number of ticks to a number of seconds, rounding down.
    ul = htonl(switch_info.slv_uptime_secs);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_UPTIME_SECS, 4, (u8 *)&ul);

    // Our own fragment size.
    ush = htons(MD_MAX_PAYLOAD_SZ_BYTES); // Convert to network order.
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_SFRAGSZ, 2, (u8 *)&ush);

    // Our own board type (mandatory in mixed stacking environments)
    ul = htonl(switch_info.info.board_type); // Convert to network order.
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_BOARD_TYPE, 4, (u8 *)&ul);

    // With what ID did we instantiate the Vitesse API?
    ul = htonl(switch_info.info.api_inst_id);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_API_INST_ID, 4, (u8 *)&ul);

    // Send a copy of the logical-to-physical port-map.
    for (port_no = VTSS_PORT_NO_START; port_no < switch_info.info.port_cnt + VTSS_PORT_NO_START; port_no++) {
        u32 idx = 2 * (port_no - VTSS_PORT_NO_START);

        log_to_phys_port_map[idx + 0] = port_no;
        log_to_phys_port_map[idx + 1] = ((switch_info.port_map[port_no].chip_no << 7) & 0x80) | (switch_info.port_map[port_no].chip_port & 0x7F);
    }

    TX_add_nbyte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_PHYS_PORT_MAP, 2 * switch_info.info.port_cnt, log_to_phys_port_map);

    // End the frame
    TX_add_1byte_tlv(&pkt_ptr, MSG_MSYNACK_TLV_TYPE_NULL, 0);

    frm_len = pkt_ptr - frm;

    // By now, we must not have spent more space than we've allocated.
    // If this one fails, you've added more TLVs. Increase the
    // MSYNACK_MAX_HDR_SZ_BYTES define accordingly.
    VTSS_ASSERT(frm_len <= MSYNACK_MAX_SZ_BYTES);
    return frm_len;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// TX_msynack_create_and_tx_dynamic()
/******************************************************************************/
static MSG_INLINE void TX_msynack_create_and_tx_dynamic(mac_addr_t dmac, u8 cid, u16 mseq, msg_msynack_status_t status)
{
    u8  *tx_buf = packet_tx_alloc(MSYNACK_MAX_SZ_BYTES);
    u32 frm_len;

    VTSS_ASSERT(tx_buf);
    frm_len = TX_msynack_create(tx_buf, dmac, cid, state.msg_cfg_slv_winsz, mseq, 0, status);
    (void)TX_msynack(NULL, tx_buf, frm_len, 0, status, TX_msynack_tx_done_dynamic);
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_md_tx_done()
// Don't deallocate anything here. Deallocation occurs when the MD gets
// acknowledged or a topology change occurs.
// @context is actually the MD that we've just transmitted.
/****************************************************************************/
static void TX_md_tx_done(void *context, packet_tx_done_props_t *props)
{
    msg_md_item_t *md = (msg_md_item_t *)context;
    MSG_STATE_CRIT_ENTER();

    // Tell the world that it's now safe to retransmit or
    // free the MD, whatever comes first.
    MSG_ASSERT(md->tx_state == MSG_MD_TX_STATE_TRANSMITTED_BUT_NOT_TXDONE_ACKD, "tx_state=%d, tx=%d", md->tx_state, props->tx);
    md->tx_state = MSG_MD_TX_STATE_TRANSMITTED_AND_TXDONE_ACKD;

    MSG_STATE_CRIT_EXIT();

    // Wake up the TX_thread() to have it re-transmit or free messages
    TX_wake_up_msg_thread(MSG_FLAG_TX_DONE_MD);

    if (!props->tx) {
        T_WG(TRACE_GRP_TX, "Tx(MD) failed");
    }
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_md_do_it()
/****************************************************************************/
static MSG_INLINE BOOL TX_md_do_it(msg_mcb_t *mcb, msg_md_item_t *md)
{
    vtss_port_no_t    port_no;
    packet_tx_props_t tx_props;

    // Get the port to transmit the frame onto
    port_no = TX_mac2port(&md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]);

    if (!PORT_NO_IS_STACK(port_no)) {
        // Probably the master went down. Don't transmit since that will cause a fatal
        // error in the packet_tx() function.
        T_WG(TRACE_GRP_TX, "Tx(MD): Invalid stack port number (port=%u, seq=%u, dmac=%s)", port_no, md->seq, CX_mac2str(&md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]));
        return FALSE;
    } else {
        if (MSG_TRACE_ENABLED(md->dbg_connid, md->dbg_dmodid)) {
            T_DG(    TRACE_GRP_TX, "Tx(MD): dmac=%s, seq=%u, dmodid=%s, totlen=%5u, offset=%5u", CX_mac2str(&md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), md->seq, vtss_module_names[md->dbg_dmodid], md->dbg_totlen, md->dbg_offset);
            T_RG_HEX(TRACE_GRP_TX, &md->hdr[VTSS_FDMA_HDR_SIZE_BYTES], md->hdr_len);
            T_RG_HEX(TRACE_GRP_TX, md->payload, MIN(40, md->payload_len));
        }

        TX_props_compose(mcb, &tx_props, &md->hdr[VTSS_FDMA_HDR_SIZE_BYTES], md->hdr_len, port_no, TX_md_tx_done, md);
        tx_props.packet_info.frm[1] = md->payload;
        tx_props.packet_info.len[1] = md->payload_len;
        if (TX_pkt(&tx_props) != VTSS_RC_OK) {
            T_WG(TRACE_GRP_TX, "Tx(MD): Transmission failed");
            return FALSE;
        }
    }
    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_md()
/****************************************************************************/
static MSG_INLINE BOOL TX_md(msg_mcb_t *mcb, msg_md_item_t *md)
{
    BOOL result = TX_md_do_it(mcb, md);

    // Update volatile per-connection statistics
    mcb->stat.tx_pdu[MSG_PDU_TYPE_MD][result ? 0 : 1]++;
    mcb->stat.tx_md_bytes += md->payload_len;

    // Update persistent overall statistics
    state.pdu_stat[state.state].tx[MSG_PDU_TYPE_MD][md->tx_state != MSG_MD_TX_STATE_NEVER_TRANSMITTED ? 1 : 0][result ? 0 : 1]++;
    state.pdu_stat[state.state].tx_md_bytes += md->payload_len;
    return result;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_mds()
/****************************************************************************/
static BOOL TX_mds(msg_mcb_t *mcb, vtss_isid_t isid, u8 cid, cyg_tick_count_t cur_time)
{
    msg_item_t    *tx_msg;
    msg_md_item_t *md;
    u16           left_most_unackd = 0;
    BOOL          left_most_unackd_found = FALSE;
    BOOL          timed_out;

    if (mcb->tx_msg_list == NULL) {
        return TRUE; // Everything handled OK.
    }

    // Run through all messages to see if we need to (re-)transmit any.
    tx_msg = mcb->tx_msg_list;
    while (tx_msg) {
        md = tx_msg->u.tx.md_list;
        VTSS_ASSERT(md);
        while (md) {
            if (md->acknowledged == FALSE) {
                if (left_most_unackd_found == FALSE) {
                    left_most_unackd = md->seq;
                    left_most_unackd_found = TRUE;
                }

                if (SEQ_DIFF(left_most_unackd, md->seq) < mcb->dwinsz) {
                    // This MD is still within destination's window size.
                    // Check to see if we need to (re-)transmit this.

                    // If md->tx_state is MSG_MD_TX_STATE_NEVER_TRANSMITTED, we can't rely on
                    // timed_out, since last_tx is undefined.
                    // Also, if the previous transmission is not yet acknowledged by the FDMA
                    // we cannot re-transmit it yet. Therefore only MDs that are in the state
                    // MSG_MD_TX_STATE_TRANSMITTED_AND_TXDONE_ACKD can be retransmitted.
                    timed_out = MSG_TIMEDOUT(md->last_tx, cur_time, state.msg_cfg_md_timeout_ms);

                    if (md->tx_state == MSG_MD_TX_STATE_TRANSMITTED_AND_TXDONE_ACKD && timed_out) {
                        // The MD has been transmitted previously, and it has now timed out.
                        // If the number of retransmits left is now 0, we cannot get in
                        // contact with the destination.
                        if (md->retransmits_left == 0) {
                            // Uh-uh. No reply.
                            if (MSG_TRACE_ENABLED(isid, md->dbg_dmodid)) {
                                T_WG(TRACE_GRP_TX, "Tx(MD): No MACK before timeout. Stopping (state=%d, seq=%u, dmac=%s)", state.state, md->seq, CX_mac2str(&md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]));
                            }
                            state.pdu_stat[state.state].tx_md_timeouts++;
                            CX_restart(mcb, isid, MSG_TX_RC_WARN_SLV_NO_ACK, MSG_TX_RC_WARN_MST_NO_ACK);
                            return FALSE;
                        } else {
                            md->retransmits_left--;
                            if (MSG_TRACE_ENABLED(isid, md->dbg_dmodid)) {
                                T_WG(TRACE_GRP_TX, "Tx(MD): No MACK before timeout. Retransmitting (seq=%u, dmac=%s, leftmostunack=%u)", md->seq, CX_mac2str(&md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), left_most_unackd);
                            }
                        }
                    }

                    if (md->tx_state != MSG_MD_TX_STATE_TRANSMITTED_BUT_NOT_TXDONE_ACKD && (md->tx_state == MSG_MD_TX_STATE_NEVER_TRANSMITTED || timed_out)) {
                        // Either this MD is in the process of being transmitted, or it has
                        // never been transmitted, or we need to re-transmit it due to a
                        // timeout.
                        if (!TX_md(mcb, md)) {
                            // We failed to transmit the frame (T_WG() already invoked)
                            CX_restart(mcb, isid, MSG_TX_RC_WARN_SLV_INV_DPORT, MSG_TX_RC_WARN_MST_INV_DPORT);
                            return FALSE;
                        }
                        md->tx_state = MSG_MD_TX_STATE_TRANSMITTED_BUT_NOT_TXDONE_ACKD;
                        md->last_tx = cur_time;
                    }
                } else {
                    return TRUE; // Cannot transmit MDs beyond receiver's window size.
                }
            }
            md = md->next;
        }
        tx_msg = tx_msg->next;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_md_create_hdr()
/****************************************************************************/
static MSG_INLINE void TX_md_create_hdr(msg_md_item_t *md, mac_addr_t dmac, u8 dmodid, u8 cid, u16 seq, u32 totlen, u32 offset)
{
    u8  *pkt_ptr;
    u16 ush;
    u32 ulo;
    int pad_bytes;

    // Insert all bytes up to, but not including the PDU Type for the message
    // protocol.
    pkt_ptr = &md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET];
    TX_insert_hdrs(&pkt_ptr, dmac, MSG_PDU_TYPE_MD, cid);

    // At this point, pkt_ptr must be advanced to the location of
    // the first TLV.
    VTSS_ASSERT(pkt_ptr == &md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_TLV_OFFSET]);

    // Fill in the MD fields.

    // The DMODID is optional for non-start-of-message frames.
    TX_add_1byte_tlv(&pkt_ptr, MSG_MD_TLV_TYPE_DMODID, dmodid);
    md->dbg_dmodid = dmodid;

    // SEQ
    ush = htons(seq);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MD_TLV_TYPE_SEQ, 2, (u8 *)&ush);

    // TOTLEN
    ulo = htonl(totlen);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MD_TLV_TYPE_TOTLEN, 3, ((u8 *)&ulo) + 1);
    md->dbg_totlen = totlen; // For debug only

    // OFFSET
    ulo = htonl(offset);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MD_TLV_TYPE_OFFSET, 3, ((u8 *)&ulo) + 1);
    md->dbg_offset = offset; // For debug only

    // End-of-TLVs
    TX_add_1byte_tlv(&pkt_ptr, MSG_MD_TLV_TYPE_NULL, 0);

    // Pad with 0's to nearest 32-bit boundary
    pad_bytes = 4 - (((u32)pkt_ptr) & 0x3);
    if (pad_bytes != 4) {
        while (pad_bytes > 0) {
            TX_add_byte(&pkt_ptr, 0);
            pad_bytes--;
        }
    }

    md->hdr_len = pkt_ptr - &md->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET];
    VTSS_ASSERT(((((u32)pkt_ptr) & 0x3) == 0) && (md->hdr_len <= MD_MAX_PROTO_SZ_BYTES));
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_md_create()
/****************************************************************************/
static void TX_md_create(msg_item_t *tx_msg, mac_addr_t dmac, u8 cid, u16 *seq, u32 dbg_connid)
{
    u32           payload_sz, payload_left, offset = 0;
    msg_md_item_t *last_md = NULL, *temp_md;
    u8            *payload_ptr;

    payload_left = tx_msg->len;
    VTSS_ASSERT(payload_left);

    payload_ptr = tx_msg->usr_msg;
    VTSS_ASSERT(payload_ptr);

    // No list must have been built so far.
    VTSS_ASSERT(tx_msg->u.tx.md_list == NULL);

    // Fragment if needed.
    while (payload_left > 0) {
        // Allocate the structure to hold this MD.
        temp_md = VTSS_MALLOC(sizeof(msg_md_item_t));
        VTSS_ASSERT(temp_md);

        // Figure out how much of the payload can be held in this MD.
        payload_sz = MIN(payload_left, MD_MAX_PAYLOAD_SZ_BYTES);
        payload_left -= payload_sz;

        // Debug stuff only - to be able to sort in trace output
        temp_md->dbg_connid = dbg_connid;

        TX_md_create_hdr(temp_md, dmac, tx_msg->dmodid, cid, *seq, tx_msg->len, offset);

        temp_md->payload = payload_ptr;
        temp_md->payload_len = payload_sz;
        temp_md->retransmits_left = state.msg_cfg_md_retransmit_limit;
        temp_md->tx_state = MSG_MD_TX_STATE_NEVER_TRANSMITTED;
        temp_md->acknowledged = FALSE; // And therefore never been acknowledged.
        temp_md->next = NULL;          // So far, there's no next MD.
        temp_md->seq = (*seq)++;
        payload_ptr += payload_sz;
        offset      += payload_sz;

        // Update md_list
        if (tx_msg->u.tx.md_list == NULL) {
            // This is the first MD in the list of MDs.
            tx_msg->u.tx.md_list = last_md = temp_md;
        } else {
            VTSS_ASSERT(last_md != NULL); /* To convince Lint */
            last_md->next = temp_md;
            last_md = temp_md;
        }
    }
}
#endif

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// TX_mack()
// crit_msg_state must be taken prior to this call.
// @frm points to DMAC of frame.
// @frm_len doesn't include VTSS_FDMA_HDR_SIZE_BYTES and FCS
/****************************************************************************/
static MSG_INLINE BOOL TX_mack(msg_mcb_t *mcb, u8 *frm, u32 frm_len, u16 seq, msg_mack_status_t status)
{
    vtss_port_no_t    port_no;
    packet_tx_props_t tx_props;

    // Get the port to transmit the frame onto
    port_no = TX_mac2port(&frm[MSG_DMAC_OFFSET]);

    if (!PORT_NO_IS_STACK(port_no)) {
        // Probably the master went down. Don't transmit since that will cause a fatal
        // error in the packet_tx() function.
        T_WG(TRACE_GRP_TX, "Tx(MACK): Invalid stack port number (%u). dmac=%s, seq=%u, status=%u", port_no, CX_mac2str(&frm[MSG_DMAC_OFFSET]), seq, status);
        return FALSE;
    }

    T_DG(    TRACE_GRP_TX, "Tx(MACK): dmac=%s, seq=%u, status=%u", CX_mac2str(&frm[MSG_DMAC_OFFSET]), seq, status);
    T_RG_HEX(TRACE_GRP_TX, frm, frm_len);

    TX_props_compose(mcb, &tx_props, frm, frm_len, port_no, NULL, NULL);

    if (TX_pkt(&tx_props) != VTSS_RC_OK) {
        T_WG(TRACE_GRP_TX, "Tx(MACK): Transmission failed");
        return FALSE;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// TX_mack_create()
/******************************************************************************/
static MSG_INLINE void TX_mack_create(u8 *frm, mac_addr_t dmac, msg_mack_status_t status, u8 cid, u16 seq)
{
    u8  *pkt_ptr;
    u16 ush;
    u32 frm_len = 0;

    // Insert all bytes up to, but not including the PDU Type for the message
    // protocol.
    pkt_ptr = &frm[MSG_DMAC_OFFSET];
    TX_insert_hdrs(&pkt_ptr, dmac, MSG_PDU_TYPE_MACK, cid);

    // At this point, pkt_ptr must be advanced to the location of
    // the first TLV.
    VTSS_ASSERT(pkt_ptr == &frm[MSG_TLV_OFFSET]);

    // Fill in the MSYNACK TLVs.

    // Status
    TX_add_1byte_tlv(&pkt_ptr, MSG_MACK_TLV_TYPE_STATUS, status);

    // SEQ
    ush = htons(seq);
    TX_add_nbyte_tlv(&pkt_ptr, MSG_MACK_TLV_TYPE_SEQ, 2, (u8 *)(&ush));

    // End-of-TLVs
    TX_add_1byte_tlv(&pkt_ptr, MSG_MACK_TLV_TYPE_NULL, 0);

    // By now, we must not have spent more space than we've allocated.
    // If this one fails, the MACK_SZ_BYTES is too small.
    frm_len = pkt_ptr - frm;
    VTSS_ASSERT(frm_len == MACK_SZ_BYTES);
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// TX_mack_create_and_tx()
/******************************************************************************/
static BOOL TX_mack_create_and_tx(msg_mcb_t *mcb, mac_addr_t dmac, msg_mack_status_t status, u8 cid, u16 seq)
{
    BOOL result;
    u8   *tx_buf = packet_tx_alloc(MACK_SZ_BYTES);

    VTSS_ASSERT(tx_buf);
    TX_mack_create(tx_buf, dmac, status, cid, seq);
    result = TX_mack(mcb, tx_buf, MACK_SZ_BYTES, seq, status);

    // Update volatile per-connection statistics
    mcb->stat.tx_pdu[MSG_PDU_TYPE_MACK][result ? 0 : 1]++;

    // Update persistent overall statistics
    state.pdu_stat[state.state].tx[MSG_PDU_TYPE_MACK][0][result ? 0 : 1]++;
    return result;
}
#endif

/****************************************************************************/
/*                                                                          */
/*  RX INTERNAL FUNCTIONS                                                   */
/*                                                                          */
/****************************************************************************/

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_proto_warn()
/******************************************************************************/
static void RX_proto_warn(u8 *frm, const char *fmt, ...)
{
    va_list ap = NULL;
    char s[300];
    int len;

    sprintf(s, "Rx(%s from %s, state=%d, ticks=%llu): ", PDUTYPE2STR(frm[MSG_PDU_TYPE_OFFSET]), CX_mac2str(&frm[MSG_SMAC_OFFSET]), state.state, cyg_current_time());
    len = strlen(s);

    va_start(ap, fmt);
    (void)vsnprintf(s + len, sizeof(s) - len - 1, fmt, ap);
    va_end(ap);

    T_WG(TRACE_GRP_RX, "%s", s);

    // Print frame contents
    T_WG(TRACE_GRP_RX, "IFH and gap:");
    T_WG_HEX(TRACE_GRP_RX, frm - VTSS_FDMA_HDR_SIZE_BYTES, VTSS_FDMA_HDR_SIZE_BYTES);
    T_WG(TRACE_GRP_RX, "Frame contents (First 96 bytes shown):");
    T_WG_HEX(TRACE_GRP_RX, frm, 96);
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_proto_err()
/******************************************************************************/
static void RX_proto_err(u8 *frm, const char *fmt, ...)
{
    va_list ap = NULL;
    char s[300];
    int len;

    sprintf(s, "Rx(%s from %s, state=%d, ticks=%llu): ", PDUTYPE2STR(frm[MSG_PDU_TYPE_OFFSET]), CX_mac2str(&frm[MSG_SMAC_OFFSET]), state.state, cyg_current_time());
    len = strlen(s);

    va_start(ap, fmt);
    (void)vsnprintf(s + len, sizeof(s) - len - 1, fmt, ap);
    va_end(ap);

    T_EG(TRACE_GRP_RX, "%s", s);

    // Print frame contents
    T_EG(TRACE_GRP_RX, "IFH and gap:");
    T_EG_HEX(TRACE_GRP_RX, frm - VTSS_FDMA_HDR_SIZE_BYTES, VTSS_FDMA_HDR_SIZE_BYTES);
    T_EG(TRACE_GRP_RX, "Frame contents (First 96 bytes shown):");
    T_EG_HEX(TRACE_GRP_RX, frm, 96);
}
#endif

/******************************************************************************/
// RX_filter_validate()
/******************************************************************************/
static BOOL RX_filter_validate(const msg_rx_filter_t *filter)
{
    msg_rx_filter_item_t *l = RX_filter_list;

    if (!filter->cb) {
        T_EG(TRACE_GRP_CFG, "Callback function not defined");
        return FALSE;
    }

    // Currently only support one filter. Check to see if other modules have
    // registered for it.
    while (l) {
        if (l->filter.modid == filter->modid) {
            T_EG(TRACE_GRP_CFG, "Another module has already registered for this module ID (%d)", filter->modid);
            return FALSE;
        }

        l = l->next;
    }

    return TRUE;
}

/******************************************************************************/
// RX_filter_insert()
/******************************************************************************/
static BOOL RX_filter_insert(const msg_rx_filter_t *filter)
{
    msg_rx_filter_item_t *l = RX_filter_list, *new_l;

    // Critical region must be obtained when this function is called.
    MSG_CFG_CRIT_ASSERT(1);

    // Allocate a new list item
    if ((new_l = VTSS_MALLOC(sizeof(msg_rx_filter_item_t))) == NULL) {
        T_EG(TRACE_GRP_CFG, "VTSS_MALLOC(msg_rx_filter_item_t) failed");
        return FALSE;
    }

    // Copy the filter
    memcpy(&new_l->filter, filter, sizeof(new_l->filter));

    // Insert the filter in the list. Since there's no inherited priority,
    // we simply insert it in the beginning of the list, because that's easier.
    RX_filter_list = new_l;
    new_l->next = l;

    return TRUE;
}

/******************************************************************************/
// RX_msg_dispatch()
// Pass the message to the registered handler and count the event.
// Deallocation of @msg is not handled by this function!
//
// NOTE 1: The caller of this function must not be the owner of the crit_msg_state!
//
// NOTE 2: This function may be called from different threads. Updating of the
// counters are protected.
/******************************************************************************/
static MSG_INLINE void RX_msg_dispatch(msg_item_t *msg)
{
    BOOL                 handled = FALSE;
    msg_rx_filter_item_t *l;
    u8                   dmodid = msg->dmodid;

    if (MSG_TRACE_ENABLED(msg->connid, dmodid)) {
        if (msg->is_tx_msg) {
            // It's a loopback message
            T_IG(TRACE_GRP_RX, "Rx(Msg): len=%u, mod=%s, connid=%u", msg->len, vtss_module_names[dmodid], msg->connid);
        } else {
            // It's really received on a port
            T_IG(TRACE_GRP_RX, "Rx(Msg): len=%u, mod=%s, connid=%u, seq=[%u,%u]", msg->len, vtss_module_names[dmodid], msg->connid, msg->u.rx.left_seq, msg->u.rx.right_seq);
        }

        T_NG(TRACE_GRP_RX, "len=%u (first %u bytes shown)", msg->len, MIN(96, msg->len));
        T_NG_HEX(TRACE_GRP_RX, msg->usr_msg, MIN(96, msg->len));
    }

    // Protect the filter list.
    MSG_CFG_CRIT_ENTER();
    l = RX_filter_list;
    while (l) {
        if (l->filter.modid == dmodid) {
            // Save some debugging info.
            dbg_latest_rx_modid  = dmodid;
            dbg_latest_rx_len    = msg->len;
            dbg_latest_rx_connid = msg->connid;
            (void)l->filter.cb(l->filter.contxt, msg->usr_msg, msg->len, dmodid, msg->connid);
            handled = TRUE;
            break; // For now, only one cb per dmodid.
        }

        l = l->next;
    }

    MSG_CFG_CRIT_EXIT();

    if (!handled) {
        if (msg->is_tx_msg) {
            // It's a loopback message
            T_WG(TRACE_GRP_RX, "Rx(Msg): Received message (len=%u) to a non-subscribing module (mod=%d=%s, connid=%u)", msg->len, dmodid, vtss_module_names[dmodid], msg->connid);
        } else {
            // It's really received on a port
            T_WG(TRACE_GRP_RX, "Rx(Msg): Received message (len=%u) to a non-subscribing module (mod=%d=%s, seq=[%u,%u], connid=%u)", msg->len, dmodid, vtss_module_names[dmodid], msg->u.rx.left_seq, msg->u.rx.right_seq, msg->connid);
        }
    }

    // Count the event in the right bin.
    if (dmodid > VTSS_MODULE_ID_NONE) {
        // We can only count VTSS_MODULE_ID_NONE+1 modids.
        // The VTSS_MODULE_ID_NONE entry is reserved for
        // modids beyond the pre-allocated.
        dmodid = VTSS_MODULE_ID_NONE;
    }

    // Protect updating the counters, since this function may be called from
    // different threads. Since these counters are global counters, they
    // are never reset, and therefore there're no problems in updating them
    // "outside" the main-state updaters (RX_thread() and msg_topo_event()).
    MSG_COUNTERS_CRIT_ENTER();
    state.usr_stat[msg->state].rx[MIN(dmodid, VTSS_MODULE_ID_NONE)][handled ? 0 : 1]++;
    state.usr_stat[msg->state].rxb[MIN(dmodid, VTSS_MODULE_ID_NONE)] += msg->len;
    MSG_COUNTERS_CRIT_EXIT();
}

/******************************************************************************/
// RX_mac2isid()
// Only valid in master mode.
/******************************************************************************/
static vtss_isid_t RX_mac2isid(mac_addr_t mac)
{
    vtss_isid_t isid;
    int cid;

    for (isid = MSG_MST_ISID_START_IDX; isid <= MSG_MST_ISID_END_IDX; isid++) {
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            if (memcmp(mcbs[isid][cid].dmac, mac, sizeof(mac_addr_t)) == 0) {
                // We know that other connections cannot have the same SMAC - even when state
                // is MSG_CONN_STATE_MST_NO_SLV, because the mcb's dmac is cleared whenever
                // that state is entered.
                return isid;
            }
        }
    }

    return 0; // Invalid ISID.
}

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_seq_overlap()
// first_xxx refers to sequence boundaries of the first message, where
// next_xxx refers to sequence boundaries of the following message.
// xxx_left refers to the leftmost sequence number in its boundary.
// xxx_right refers to the rightmost sequence number in its boundary.
// Returns TRUE if there's an overlap, FALSE if not.
// It is assumed that first_left is the leftmost sequence number among them
// all (not the numerically smallest, but the leftmost).
/******************************************************************************/
static BOOL RX_seq_overlap(u32 first_left, u32 first_right, u32 next_left)
{
    // Normalize according to first_left, so that it's easier to check for window
    // overlaps
    u32 diff = MSG_SEQ_CNT - first_left;

    first_right = (first_right + diff) % MSG_SEQ_CNT;
    next_left   = (next_left   + diff) % MSG_SEQ_CNT;

    return (first_right >= next_left);
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_seq_falls_within()
// Returns true if seq false within [left; right] taking wrap around into
// account, FALSE otherwise.
// The distance between left and right cannot exceed MSG_SEQ_CNT/2 as discussed
// in msg.h under definition of MSG_MIN_FRAGSZ.
/******************************************************************************/
static MSG_INLINE BOOL RX_seq_falls_within(u32 seq, u32 left, u32 right)
{
    u32 dist_left_to_seq  = SEQ_DIFF(left, seq);
    u32 dist_seq_to_right = SEQ_DIFF(seq,  right);

    return (dist_left_to_seq < MSG_SEQ_CNT / 2) && (dist_seq_to_right < MSG_SEQ_CNT / 2);
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_msyn_parse()
/******************************************************************************/
static MSG_INLINE BOOL RX_msyn_parse(u8 *frm, u32 frm_len, u8 *mwinsz, u8 *mupsid, u16 *mseq, u16 *fragsz)
{
    BOOL null_seen = FALSE, mwinsz_seen = FALSE, mseq_seen = FALSE, fragsz_seen = FALSE, break_out = FALSE;
    u8   *pkt_ptr = &frm[MSG_TLV_OFFSET];
    u8   tlv_type;
    u8   tlv_len;
    u8   *tlv_val;
    u8   *frm_end = frm + frm_len;
#if !MSG_HOP_BY_HOP
    BOOL mupsid_seen = FALSE;
#endif

    while (pkt_ptr < frm_end) {
        tlv_type = *(pkt_ptr++);
        if (tlv_type < 0x80) {
            // 1-byte TLV
            tlv_len = 1;
            tlv_val = pkt_ptr;
        } else {
            // n-byte TLV
            tlv_len = *(pkt_ptr++);
            tlv_val = pkt_ptr;
            if (tlv_len < 2) {
                RX_proto_err(frm, "TLV length of n-byte TLV is smaller than 2");
                return FALSE;
            }
        }
        tlv_val = pkt_ptr;
        pkt_ptr += tlv_len;

        switch (tlv_type) {
        case MSG_MSYN_TLV_TYPE_NULL:
            if (*tlv_val != 0) {
                RX_proto_err(frm, "NULL_TLV val is non-zero");
                return FALSE;
            }
            null_seen = TRUE;
            break_out = TRUE;
            break;

        case MSG_MSYN_TLV_TYPE_MWINSZ:
            if (*tlv_val == 0) {
                RX_proto_err(frm, "MWINSZ TLV val is zero");
                return FALSE;
            }
            *mwinsz = *tlv_val;
            mwinsz_seen = TRUE;
            break;

        case MSG_MSYN_TLV_TYPE_MUPSID: {
            vtss_vstax_upsid_t upsid = (vtss_vstax_upsid_t)(*tlv_val);
            if (!VTSS_VSTAX_UPSID_LEGAL(upsid)) {
                RX_proto_err(frm, "MUPSID TLV val is not a valid UPSID (%u)", *tlv_val);
                return FALSE;
            }
            *mupsid = *tlv_val;
#if !MSG_HOP_BY_HOP
            mupsid_seen = TRUE;
#endif
            break;
        }

        case MSG_MSYN_TLV_TYPE_MSEQ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "MSEQ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }
            *mseq = (tlv_val[0] << 8) | tlv_val[1];
            mseq_seen = TRUE;
            break;

        case MSG_MSYN_TLV_TYPE_MFRAGSZ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "MFRAGSZ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }
            *fragsz = (tlv_val[0] << 8) | tlv_val[1];
            if (*fragsz < MSG_MIN_FRAGSZ) {
                RX_proto_err(frm, "MFRAGSZ is smaller than %d (actual = %u)", MSG_MIN_FRAGSZ, *fragsz);
                return FALSE;
            }
            fragsz_seen = TRUE;
            break;

        default:
            // Allow unsupported TLVs.
            RX_proto_warn(frm, "Unsupported TLV type (%d)", tlv_type);
            break;
        }
        if (break_out) {
            break;
        }
    }

    // At this point, the pkt_ptr must not exceed frm_end. It may, however,
    // be smaller, because the len is always at least 60 bytes due to the
    // minimum Ethernet frame size (excl. FCS).
    if ((frm_len <= 60 && pkt_ptr > frm_end) || (frm_len > 60 && pkt_ptr != frm_end)) {
        RX_proto_err(frm, "Frame length didn't match packet contents (frm_len = %u, packet contents size = %u)", frm_len, pkt_ptr - frm);
        return FALSE;
    }

    if (mwinsz_seen == FALSE || mseq_seen == FALSE || fragsz_seen == FALSE || null_seen == FALSE) {
        RX_proto_err(frm, "At least one of the mandatory TLVs were not found in the packet (%d%d%d%d)", mwinsz_seen, mseq_seen, fragsz_seen, null_seen);
        return FALSE;
    }

#if MSG_HOP_BY_HOP
    *mupsid = (u8)VTSS_VSTAX_UPSID_UNDEF;
#else
    if (mupsid_seen == FALSE) {
        RX_proto_err(frm, "MSYN: MUPSID TLV was not found in the packet");
        return FALSE;
    }
#endif

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_msyn()
// Handle reception of an MSYN. crit_msg_state is taken before entry.
// Returns FALSE if the MSYN doesn't follow the protocol or we can't accept it
// for some other reason - like if we're in a wrong state or the CID is greater
// than the number of connections we support.
/******************************************************************************/
static BOOL RX_msyn(u8 *frm, u32 frm_len)
{
    u8                   mwinsz = 0, mupsid = 0, cid;
    u16                  mseq = 0, next_mseq, sseq, fragsz = 0;
    msg_mcb_t            *mcb;
    msg_msynack_status_t msynack_status;

    // Forget the frame if we're in a wrong state.
    if (state.state != MSG_MOD_STATE_SLV) {
        RX_proto_warn(frm, "Discarding because we're master");
        return FALSE;
    }

    cid = frm[MSG_CID_OFFSET];
    if (cid >= MSG_CFG_CONN_CNT) {
        // Unsupported connection. Return MSYNACK with status = MSG_MSYNACK_STATUS_ERR_CID
        // so that the master knows that it shall not continue sending MSYNs on this CID.
        // We only send this type of MSYNACK's once, and it's dynamically allocated in
        // contrast to the MSYNACKs with an OK status. The main reason is that we don't
        // have an MCB that we can hook the MSYNACK up to, but we still want the sender
        // not to spam us with MSYNs, which would be the case if we didn't reply at all.
        RX_proto_warn(frm, "Unsupported CID (%u)", cid);
        TX_msynack_create_and_tx_dynamic(&frm[MSG_SMAC_OFFSET], cid, 0, MSG_MSYNACK_STATUS_ERR_CID);
        return FALSE;
    }

    // Parse frame.
    if (!RX_msyn_parse(frm, frm_len, &mwinsz, &mupsid, &mseq, &fragsz)) {
        return FALSE;
    }

    T_IG(    TRACE_GRP_RX, "Rx(MSYN): smac=%s, mseq=%u, mupsid=%u, fragsz=%u, mwinsz=%u", CX_mac2str(&frm[MSG_SMAC_OFFSET]), mseq, mupsid, fragsz, mwinsz);
    T_RG(    TRACE_GRP_RX, "len=%u (first %u bytes shown)", frm_len, MIN(96, frm_len));
    T_RG_HEX(TRACE_GRP_RX, frm, MIN(96, frm_len));

    mcb = &mcbs[MSG_SLV_ISID_IDX][cid];

    // Flush all user messages that were on their way to the current master (if any),
    // and all MDs received by previous master that were not yet forwarded to the
    // user modules.
    // RBNTBD: Should this also be done for the other CIDs? If not, we may have
    // different masters on different CIDs.
    CX_mcb_flush(mcb, MSG_TX_RC_WARN_SLV_NEW_MASTER);

    if (mcb->state == MSG_CONN_STATE_SLV_NO_MST) {
        msynack_status = MSG_MSYNACK_STATUS_OK;
    } else if (memcmp(mcb->dmac, &frm[MSG_SMAC_OFFSET], VTSS_MAC_ADDR_SZ_BYTES) == 0) {
        // New master is the same as the old.
        msynack_status = MSG_MSYNACK_STATUS_OK_BUT_ALREADY_CONNECTED_SAME;
    } else {
        msynack_status = MSG_MSYNACK_STATUS_OK_BUT_ALREADY_CONNECTED_DIFF;
    }

    // Update MCB
    // Master's MAC Address
    memcpy(mcb->dmac, &frm[MSG_SMAC_OFFSET], VTSS_MAC_ADDR_SZ_BYTES);

    // Master's UPSID
    CX_update_upsid(mcb, mupsid);

    // Get a new connid.
    mcb->connid = state.slv_next_connid++;
    T_IG(TRACE_GRP_RX, "Rx(MSYN): Assigning connection ID %u", mcb->connid);
    if (state.slv_next_connid == 0) {
        // Handle wrap-around.
        state.slv_next_connid = VTSS_ISID_END;
    }

    // Master's SEQ window size goes into the destination winsz
    mcb->dwinsz = mwinsz;

    // Slave's SEQ window size goes into the source winsz.
    // Make a copy of the configuration parameter, so that it can
    // change without affecting the connection, since it's only used
    // by the master.
    mcb->swinsz = state.msg_cfg_slv_winsz;

    // Master's current SEQ number goes into the destination SEQ.
    // The first MD that the master sends (not necessarily the first
    // that the slave received, in case of re-transmission) is expected
    // to hold this sequence number + 1, taking wrap-around into account.
    next_mseq = mseq + 1;
#if MSG_SEQ_CNT <= 65535UL
    if (next_mseq == MSG_SEQ_CNT) {
        next_mseq = 0;
    }
#endif
    mcb->dseq_leftmost_unreceived = mcb->dseq_leftmost_allowed = next_mseq;

    // No MDs have currently been received.
    memset(mcb->dseq_ackd, 0, sizeof(mcb->dseq_ackd));
    mcb->dseq_ackd_idx = 0;  // Doesn't really matter where it starts as long as it starts within bounds of dseq_ackd.

    // The master must use this fragment size for all subsequent fragmented, non-end-of-message-MDs.
    mcb->fragsz = (u32)fragsz;

    // Create and transmit an MSYNACK.
    mcb->u.slave.msynack.dbg_status = msynack_status; // For debug only

    sseq = mcb->next_available_sseq;
    mcb->next_available_sseq++;
#if MSG_SEQ_CNT <= 65535UL
    if (msg->next_available_sseq == MSG_SEQ_CNT) {
        mcb->next_available_sseq = 0;
    }
#endif
    mcb->u.slave.msynack.cur_sseq = sseq;
    mcb->u.slave.msynack.len = TX_msynack_create(&mcb->u.slave.msynack.frm[VTSS_FDMA_HDR_SIZE_BYTES], mcb->dmac, cid, mcb->swinsz, mseq, sseq, msynack_status);
    if (!TX_msynack(mcb, &mcb->u.slave.msynack.frm[VTSS_FDMA_HDR_SIZE_BYTES], mcb->u.slave.msynack.len, sseq, mcb->u.slave.msynack.dbg_status, TX_msynack_tx_done_static)) {
        mcb->state = MSG_CONN_STATE_SLV_NO_MST;
        return FALSE;
    }

    // Proceed to the "Wait for MSYNACK's MACK" state.
    mcb->state = MSG_CONN_STATE_SLV_WAIT_FOR_MSYNACKS_MACK;
    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_msynack_parse()
/******************************************************************************/
static MSG_INLINE BOOL RX_msynack_parse(u8 *frm, u32 frm_len, u8 *swinsz, msg_msynack_status_t *status, u16 *mseq, u16 *sseq, u16 *fragsz, msg_switch_info_t *switch_info)
{
    BOOL null_seen = FALSE, swinsz_seen = FALSE, status_seen = FALSE, mseq_seen = FALSE, sseq_seen = FALSE, fragsz_seen = FALSE, board_type_seen = FALSE, break_out = FALSE;
    u8   *pkt_ptr = &frm[MSG_TLV_OFFSET];
    u8   tlv_type;
    u8   tlv_len;
    u8   *tlv_val;
    u8   *frm_end = frm + frm_len;

    memset(switch_info, 0, sizeof(*switch_info));
    switch_info->info.configurable = TRUE;

    while (pkt_ptr < frm_end) {
        tlv_type = *(pkt_ptr++);

        if (tlv_type < 0x80) {
            // 1-byte TLV
            tlv_len = 1;
            tlv_val = pkt_ptr;
        } else {
            // n-byte TLV
            tlv_len = *(pkt_ptr++);
            tlv_val = pkt_ptr;

            if (tlv_len < 2) {
                RX_proto_err(frm, "TLV length of n-byte TLV is smaller than 2");
                return FALSE;
            }
        }

        tlv_val = pkt_ptr;
        pkt_ptr += tlv_len;

        switch (tlv_type) {
        case MSG_MSYNACK_TLV_TYPE_NULL:
            if (*tlv_val != 0) {
                RX_proto_err(frm, "NULL_TLV val is non-zero");
                return FALSE;
            }

            null_seen = TRUE;
            break_out = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_SWINSZ:
            if (*tlv_val == 0) {
                RX_proto_err(frm, "SWINSZ TLV val is zero");
                return FALSE;
            }

            *swinsz = *tlv_val;
            swinsz_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_STATUS:
            *status = *tlv_val;
            status_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_PORT_CNT:
            switch_info->info.port_cnt = *tlv_val;
            break;

        case MSG_MSYNACK_TLV_TYPE_STACK_PORT_0:
            switch_info->info.stack_ports[0] = *tlv_val;
            break;

        case MSG_MSYNACK_TLV_TYPE_STACK_PORT_1:
            switch_info->info.stack_ports[1] = *tlv_val;
            break;

        case MSG_MSYNACK_TLV_TYPE_MSEQ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "MSEQ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }

            *mseq = (tlv_val[0] << 8) | tlv_val[1];
            mseq_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_SSEQ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "SSEQ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }

            *sseq = (tlv_val[0] << 8) | tlv_val[1];
            sseq_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_VERSION_STRING:
            if (tlv_val != NULL) {
                strncpy(switch_info->version_string, (char *)tlv_val, MSG_MAX_VERSION_STRING_LEN);
                switch_info->version_string[MSG_MAX_VERSION_STRING_LEN - 1] = '\0';
            }

            break;

        case MSG_MSYNACK_TLV_TYPE_PRODUCT_NAME:
            if (tlv_val != NULL) {
                strncpy(switch_info->product_name, (char *)tlv_val, MSG_MAX_PRODUCT_NAME_LEN);
                switch_info->product_name[MSG_MAX_PRODUCT_NAME_LEN - 1] = '\0';
            }

            break;

        case MSG_MSYNACK_TLV_TYPE_UPTIME_SECS:
            if (tlv_len != 4) {
                RX_proto_err(frm, "UPTIME_SECS is not 4 (actually %d)", tlv_len);
                return FALSE;
            }

            switch_info->slv_uptime_secs = (tlv_val[0] << 24) | (tlv_val[1] << 16) | (tlv_val[2] << 8) | tlv_val[3];
            break;

        case MSG_MSYNACK_TLV_TYPE_SFRAGSZ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "SFRAGSZ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }

            *fragsz = (tlv_val[0] << 8) | tlv_val[1];
            if (*fragsz < MSG_MIN_FRAGSZ) {
                RX_proto_err(frm, "SFRAGSZ is smaller than %d (actual = %u)", MSG_MIN_FRAGSZ, *fragsz);
                return FALSE;
            }

            fragsz_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_BOARD_TYPE:
            if (tlv_len != 4) {
                RX_proto_err(frm, "BOARD_TYPE is not 4 (actually %d)", tlv_len);
                return FALSE;
            }

            switch_info->info.board_type = (tlv_val[0] << 24) | (tlv_val[1] << 16) | (tlv_val[2] << 8) | tlv_val[3];
            board_type_seen = TRUE;
            break;

        case MSG_MSYNACK_TLV_TYPE_API_INST_ID:
            if (tlv_len != 4) {
                RX_proto_err(frm, "API_INST_ID is not 4 (actually %d)", tlv_len);
                return FALSE;
            }

            switch_info->info.api_inst_id = (tlv_val[0] << 24) | (tlv_val[1] << 16) | (tlv_val[2] << 8) | tlv_val[3];
            break;

        case MSG_MSYNACK_TLV_TYPE_PHYS_PORT_MAP: {
            vtss_port_no_t port_no;
            u32            idx;

            if (tlv_len < 4 || tlv_len > 254 || tlv_len % 2) {
                RX_proto_err(frm, "PHYS_PORT_MAP is not >= 4 and an even number of bytes (%d)", tlv_len);
                return FALSE;
            }

            // Initialize to -1 to indicate that the ports don't exist.
            for (port_no = 0; port_no < ARRSZ(switch_info->port_map); port_no++) {
                // -1 indicates that it doesn't exist.
                switch_info->port_map[port_no].chip_port = -1;
            }

            for (idx = 0; idx < tlv_len; idx += 2) {
                port_no = tlv_val[idx];
                if (port_no < ARRSZ(switch_info->port_map)) {
                    switch_info->port_map[port_no].chip_port = (tlv_val[idx + 1] & 0x7F) >> 0;
                    switch_info->port_map[port_no].chip_no   = (tlv_val[idx + 1] & 0x80) >> 7;
                } else {
                    T_E("Invalid port number (%u) received", port_no);
                }
            }

            break;
        }

        default:
            // Allow unsupported TLVs.
            RX_proto_warn(frm, "Unsupported TLV type (%d)", tlv_type);
            break;
        }

        if (break_out) {
            break;
        }
    }

    if (!CX_switch_info_valid(&switch_info->info)) {
        return FALSE;
    }

    // At this point, the pkt_ptr must not exceed frm_end. It may, however,
    // be smaller, because the len is always at least 60 bytes due to the
    // minimum Ethernet frame size (excl. FCS).
    if ((frm_len <= 60 && pkt_ptr > frm_end) || (frm_len > 60 && pkt_ptr != frm_end)) {
        RX_proto_err(frm, "Frame length didn't match packet contents (frm_len = %u, packet contents size = %u)", frm_len, pkt_ptr - frm);
        return FALSE;
    }

    if (swinsz_seen == FALSE || status_seen == FALSE || mseq_seen == FALSE || sseq_seen == FALSE || fragsz_seen == FALSE || board_type_seen == FALSE || null_seen == FALSE) {
        RX_proto_err(frm, "At least one of the mandatory TLVs were not found in the packet (%d%d%d%d%d%d%d)", swinsz_seen, status_seen, mseq_seen, sseq_seen, fragsz_seen, board_type_seen, null_seen);
        return FALSE;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_msynack()
// crit_msg_state is taken prior to call.
/******************************************************************************/
static BOOL RX_msynack(u8 *frm, u32 frm_len)
{
    msg_mcb_t            *mcb;
    u8                   swinsz = 0, cid;
    msg_msynack_status_t msynack_status = 0;
    u16                  mseq = 0, sseq = 0, next_sseq, fragsz = 0;
    vtss_isid_t          isid;
    msg_switch_info_t    switch_info;

    // Forget the frame if we're not master.
    if (state.state != MSG_MOD_STATE_MST) {
        RX_proto_warn(frm, "Discarding because we're not master");
        return FALSE;
    }

    // Find the corresponding ISID based on the frame's SMAC
    isid = RX_mac2isid(&frm[MSG_SMAC_OFFSET]);

    if (isid == 0) {
        // No such slave.
        RX_proto_warn(frm, "Unknown slave");
        return FALSE;
    }

    cid = frm[MSG_CID_OFFSET];
    if (cid >= MSG_CFG_CONN_CNT) {
        RX_proto_err(frm, "Invalid CID (%u)", cid);
        return FALSE;
    }

    // Parse frame.
    if (!RX_msynack_parse(frm, frm_len, &swinsz, &msynack_status, &mseq, &sseq, &fragsz, &switch_info)) {
        return FALSE;
    }

    if (MSG_TRACE_ENABLED(isid, -1)) {
        T_IG(    TRACE_GRP_RX, "Rx(MSYNACK): smac=%s, mseq=%u, sseq=%u, fragsz=%u, swinsz=%u, status=%u", CX_mac2str(&frm[MSG_SMAC_OFFSET]), mseq, sseq, fragsz, swinsz, msynack_status);
        T_RG(    TRACE_GRP_RX, "len=%u (first %u bytes shown)", frm_len, MIN(96, frm_len));
        T_RG_HEX(TRACE_GRP_RX, frm, MIN(96, frm_len));
    }

    // Get the MCB
    VTSS_ASSERT(isid <= VTSS_ISID_CNT); // Keep Lint satisfied
    mcb = &mcbs[isid][cid];

    // Branch on the connection's state.
    switch (mcb->state) {
    case MSG_CONN_STATE_MST_NO_SLV:
    case MSG_CONN_STATE_MST_RDY:
        // In neither of these states we should have updated the slave's MAC address.
        MSG_ASSERT(FALSE, "Haven't we made sure that the mcb->dmac is cleared when tearing down a connection (state=%d)?", mcb->state);
        return FALSE; // Unreachable.

    case MSG_CONN_STATE_MST_WAIT_FOR_MSYNACK:
        VTSS_ASSERT(mcb->tx_msg_list == NULL); // No pending messages. These must have been rejected by msg_tx()

        // Check that the MSYNACK's mseq matches the mseq of the latest MSYN we have sent. If not,
        // just discard it, since we will re-transmit the MSYN in a while with a new mseq.
        if (mseq != mcb->u.master.msyn.cur_mseq) {
            RX_proto_warn(frm, "Received MSYNACK with an unexpected mseq (%u). Expected %u", mseq, mcb->u.master.msyn.cur_mseq);
            return FALSE;
        }

        // Got what we were waiting for. Check MSYNACK's status.
        if (msynack_status >= MSG_MSYNACK_STATUS_ERR_LVL) {
            // The slave rejects the connection. If this is for CID #0, it's a protocol
            // error, since all slaves that implement the message protocol must accept
            // at least one connection.
            if (cid == 0) {
                RX_proto_err(frm, "Slave (isid=%d) rejected MSYN with status=%d, even though CID=0", isid, msynack_status);
            }
            mcb->state = MSG_CONN_STATE_MST_STOP; // Stop the MSYN tx attempts.
            return FALSE; // Don't Tx MACKs on errors.
        } else if (msynack_status >= MSG_MSYNACK_STATUS_WRN_LVL) {
            // These are totally normal warnings, that will appear in stacks with more than two switches when a new master is elected,
            // so we won't flag them as warnings, but merely as debug.
            T_DG(TRACE_GRP_RX, "Rx(MSYNACK): Indicates a warning (status=%u, smac=%s)", msynack_status, CX_mac2str(&frm[MSG_SMAC_OFFSET]));
        }

        // Update internal state.
        mcb->u.master.switch_info = switch_info;
        mcb->u.master.switch_info.mst_uptime_secs = CX_uptime_secs_get();

        mcb->dwinsz = swinsz; // The destination's (slave's) window size, so that we don't send too many MDs.
        next_sseq = sseq + 1;
#if MSG_SEQ_CNT <= 65535UL
        if (next_sseq == MSG_SEQ_CNT) {
            next_sseq = 0;
        }
#endif
        mcb->dseq_leftmost_unreceived = mcb->dseq_leftmost_allowed = next_sseq; // The destination's (slave's) sequence number. The next we expect to see is one higher.

        // No MDs have currently been received.
        memset(mcb->dseq_ackd, 0, sizeof(mcb->dseq_ackd));
        mcb->dseq_ackd_idx = 0; // Doesn't really matter where it starts as long as it starts within bounds of dseq_ackd.

        // The slave must use this fragment size for all subsequent fragmented, non-end-of-message-MDs.
        mcb->fragsz = (u32)fragsz;
        (void)TX_mack_create_and_tx(mcb, mcb->dmac, MSG_MACK_STATUS_OK, cid, sseq);

        // Tell User Modules of the new switch through the init_modules thread.
        // We cannot allow ourselves to block here forever, so make tryput().
        // If that one fails, we need to renegotiate the connection. Tough luck.
        if (!CX_event_switch_add(isid, &mcb->u.master.switch_info.info, mcb->u.master.switch_info.port_map)) {
            memset(mcb->dmac, 0, sizeof(mac_addr_t));
            mcb->state = MSG_CONN_STATE_MST_RDY;
            return FALSE;
        } else {
            mcb->estab_time = time(NULL);
            mcb->state = MSG_CONN_STATE_MST_ESTABLISHED;
        }
        return TRUE;

    case MSG_CONN_STATE_MST_ESTABLISHED:
        // Discard all MSYNACKs in the established state.
        RX_proto_warn(frm, "Received MSYNACK (sseq=%u, mseq=%u) in established state", sseq, mseq);
        return FALSE;

    case MSG_CONN_STATE_MST_STOP:
        RX_proto_warn(frm, "Re-received MSYNACK while connection is in stop state (isid=%d, cid=%d)", isid, cid);
        return FALSE;

    default:
        VTSS_ASSERT(FALSE);
        break; // Unreachable
    }

    return FALSE; // Unreachable
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_md_parse()
// Returns NULL on parse error, otherwise a pointer to the payload.
/******************************************************************************/
static MSG_INLINE u8 *RX_md_parse(u8 *frm, u32 frm_len, u8 *dmodid, u16 *seq, u32 *totlen, u32 *offset)
{
    BOOL null_seen = FALSE, dmodid_seen = FALSE, seq_seen = FALSE, totlen_seen = FALSE, offset_seen = FALSE, break_out = FALSE;
    u8   *pkt_ptr = &frm[MSG_TLV_OFFSET];
    u8   tlv_type;
    u8   tlv_len;
    u8   *tlv_val;
    u8   *frm_end = frm + frm_len;

    *dmodid = 0xFF;

    while (pkt_ptr < frm_end) {
        tlv_type = *(pkt_ptr++);
        if (tlv_type < 0x80) {
            // 1-byte TLV
            tlv_len = 1;
            tlv_val = pkt_ptr;
        } else {
            // n-byte TLV
            tlv_len = *(pkt_ptr++);
            tlv_val = pkt_ptr;
            if (tlv_len < 2) {
                RX_proto_err(frm, "TLV length of n-byte TLV is smaller than 2");
                return NULL;
            }
        }
        tlv_val = pkt_ptr;
        pkt_ptr += tlv_len;

        switch (tlv_type) {
        case MSG_MD_TLV_TYPE_NULL:
            if (*tlv_val != 0) {
                RX_proto_err(frm, "NULL_TLV val is non-zero");
                return NULL;
            }
            null_seen = TRUE;
            break_out = TRUE;
            break;

        case MSG_MD_TLV_TYPE_DMODID:
            *dmodid = *tlv_val;
            dmodid_seen = TRUE;
            break;

        case MSG_MD_TLV_TYPE_SEQ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "SEQ length is not 2 (actually %d)", tlv_len);
                return NULL;
            }
            *seq = (tlv_val[0] << 8) | tlv_val[1];
            seq_seen = TRUE;
            break;

        case MSG_MD_TLV_TYPE_TOTLEN:
            if (tlv_len != 3) {
                RX_proto_err(frm, "TOTLEN length is not 3 (actually %d)", tlv_len);
                return NULL;
            }
            *totlen = (tlv_val[0] << 16) | (tlv_val[1] << 8) | tlv_val[2];
            totlen_seen = TRUE;

            if (*totlen == 0 || *totlen > MSG_MAX_LEN_BYTES) {
                RX_proto_err(frm, "TOTLEN is zero or larger than max allowed msg size (%u)", *totlen);
                return NULL;
            }
            break;

        case MSG_MD_TLV_TYPE_OFFSET:
            if (tlv_len != 3) {
                RX_proto_err(frm, "OFFSET length is not 3 (actually %d)", tlv_len);
                return NULL;
            }
            *offset = (tlv_val[0] << 16) | (tlv_val[1] << 8) | tlv_val[2];
            offset_seen = TRUE;
            break;

        default:
            // Allow unsupported TLVs.
            RX_proto_warn(frm, "Unsupported TLV type (%d)", tlv_type);
            break;
        }

        if (break_out) {
            break;
        }
    }

    // We must advance the pkt_ptr to the next 32-bit boundary, which is the pointer to the payload.
    pkt_ptr = (u8 *)(4 * ((((u32)pkt_ptr) + 3) / 4));

    // Check that the mandatory fields are present in the packet.
    if (seq_seen == FALSE || totlen_seen == FALSE || offset_seen == FALSE || null_seen == FALSE) {
        RX_proto_err(frm, "At least one of the mandatory TLVs were not found in the packet (%d%d%d%d)", seq_seen, totlen_seen, offset_seen, null_seen);
        return NULL;
    }

    // Check that the conditionally mandatory fields are present.
    if (*offset == 0 && dmodid_seen == FALSE) {
        RX_proto_err(frm, "DMODID TLV not found in packet, though it was a start-of-message MD (SEQ = %u)", *seq);
        return NULL;
    }

    // The offset cannot exceed the total length
    if (*offset >= *totlen) {
        RX_proto_err(frm, "Got an MD with an offset (%u) that was >= totlen (%u)", *offset, *totlen);
        return NULL;
    }

    return pkt_ptr;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_md()
/******************************************************************************/
static BOOL RX_md(u8 *frm, u32 frm_len, u32 *payload_len)
{
    u16         seq = 0, dseq_rightmost_allowed;
    u8          dmodid;
    u8          *payload;
    BOOL        seq_out_of_range, already_received, received_out_of_order, handled;
    u8          cid;
    msg_mcb_t   *mcb;
    vtss_isid_t isid = 0;
    u32         offset = 0, totlen = 0;
    msg_item_t  *rx_msg, *temp_msg, *last_msg;
    u8          *frm_end, *payload_end;

    *payload_len = 0;

    cid = frm[MSG_CID_OFFSET];
    if (cid >= MSG_CFG_CONN_CNT) {
        RX_proto_err(frm, "Invalid CID (%u)", cid);
        return FALSE;
    }

    if ((payload = RX_md_parse(frm, frm_len, &dmodid, &seq, &totlen, &offset)) == NULL) {
        return FALSE;
    }

    T_DG(    TRACE_GRP_RX, "Rx(MD): smac=%s, seq=%u, dmodid=%s, totlen=%5u, offset=%5u", CX_mac2str(&frm[MSG_SMAC_OFFSET]), seq, vtss_module_names[dmodid], totlen, offset);
    T_RG(    TRACE_GRP_RX, "len=%u (first %u bytes shown)", frm_len, MIN(96, frm_len));
    T_RG_HEX(TRACE_GRP_RX, frm, MIN(96, frm_len));

    *payload_len = totlen - offset;

    if (state.state == MSG_MOD_STATE_MST) {
        // Master. Lookup isid.
        isid = RX_mac2isid(&frm[MSG_SMAC_OFFSET]);
        if (isid == 0) {
            // No such slave.
            RX_proto_warn(frm, "Unknown slave");
            return FALSE;
        }
        mcb = &mcbs[isid][cid];
        if (mcb->state != MSG_CONN_STATE_MST_ESTABLISHED) {
            RX_proto_warn(frm, "Rx'd MD as master on non-established connection");
            return FALSE;
        }
    } else {
        // We're slave. Check MD's SMAC
        mcb = &mcbs[MSG_SLV_ISID_IDX][cid];
        if (memcmp(mcb->dmac, &frm[MSG_SMAC_OFFSET], VTSS_MAC_ADDR_SZ_BYTES) != 0) {
            RX_proto_warn(frm, "Rx'd MD from unknown master. Currently expected master=%s", CX_mac2str(mcb->dmac));
            return FALSE;
        }
        if (mcb->state != MSG_CONN_STATE_SLV_ESTABLISHED) {
            RX_proto_warn(frm, "Rx'd MD on non-established connection", CX_mac2str(mcb->dmac));
            return FALSE;
        }
    }

    // The remainder is (almost) identical for master and slave.

    // We couldn't check that the frame size was correct until now, because we needed the MCB's fragsz
    // member to compute it.
    if (*payload_len > mcb->fragsz) {
        *payload_len = mcb->fragsz;
    }

    // The offset must always be a multiple of the fragment size reported by the transmitting switch.
    if ((offset % mcb->fragsz) != 0) {
        // RBNTBD: Enter STOP state?
        RX_proto_err(frm, "The OFFSET (%u) was not a multiple of the negotiated fragment size (%u)", offset, mcb->fragsz);
        return FALSE;
    }

    frm_end = frm + frm_len;
    payload_end = payload + *payload_len;
    if ((frm_len <= 60 && payload_end > frm_end) || (frm_len > 60 && payload_end != frm_end)) {
        RX_proto_err(frm, "Frame length doesn't match packet contents (frm_len=%u, hdr_size=%u, payload_len=%u)", frm_len, payload - frm, *payload_len);
        return FALSE;
    }

    // --------------------------------------------------------------------------
    // Check sequence numbers
    // --------------------------------------------------------------------------

    // The MD.SEQ must be within valid range. If SEQ is more left than
    // dseq_leftmost_allowed or further to the right than dseq_leftmost_unreceived,
    // then SEQ is out of bounds, and we transmit a MACK with status indicating that.
    // The valid range of SEQ is
    //        [dseq_leftmost_allowed; dseq_leftmost_unreceived + swinsz - 1]
    // taking wrap-around into account.
    dseq_rightmost_allowed = mcb->dseq_leftmost_unreceived + mcb->swinsz - 1;
    seq_out_of_range = FALSE;
    if (mcb->dseq_leftmost_allowed <= dseq_rightmost_allowed) {
        // Normal, non-wrapping window.
        if (seq < mcb->dseq_leftmost_allowed || seq > dseq_rightmost_allowed) {
            seq_out_of_range = TRUE;
        }
    } else {
        // The current window wraps, so that the rightmost is smaller than the leftmost.
        if (seq < mcb->dseq_leftmost_allowed && seq > dseq_rightmost_allowed) {
            seq_out_of_range = TRUE;
        }
    }

    if (seq_out_of_range) {
        // In the early versions of the protocol, the remedy was to:
        //   In slave mode: Transmit a MACK indicating this. That would in turn
        //                  cause the master to re-negotiate the connection.
        //   In master mode: Simply re-negotiate the connection right away.
        // The problem with this approach is that an MD may have been on its
        // way for a long time travelling some obscure path of the stack,
        // sitting in the Tx queue system until the port came up, etc. If in
        // the meantime the MD was re-transmitted, and the re-transmitted MD
        // arrived together with a bunch of subsequent MDs, before the original
        // MD arrived, the sequence counter may be too far to the left, causing
        // a re-negotiation of the connection, even though nothing was wrong.
        // The newer versions of the protocol have changed this behavior, so that
        // out-of-range sequence counters are counted and give rise to a warning,
        // but otherwise ignored. If an MD doesn't receive a MACK, the master
        // will re-negotiate the connection anyway, so that deadlocks don't
        // occur.
        RX_proto_warn(frm, "SEQ (%u) out of range ([left; right] = [%u; %u])", seq, mcb->dseq_leftmost_allowed, dseq_rightmost_allowed);
        return FALSE;
    }

    // If we get here, the SEQ is OK.
    // The next step is to check if the MD has already been received.
    // Here we check if SEQ lies left of dseq_leftmost_unreceived.
    already_received = FALSE;
    // If dseq_leftmost_allowed == dseq_leftmost_unreceived, then
    // it's not already received, so no special check for that.
    if (mcb->dseq_leftmost_allowed < mcb->dseq_leftmost_unreceived) {
        // Non-wrapping window
        if (seq < mcb->dseq_leftmost_unreceived) {
            // RBNTBD: Remove the following T_IG() when debugging is done
            T_IG(TRACE_GRP_RX, "Rx(MD): Already Received case 1 (seq=%u, l.m.a.=%u, l.m.u.=%u)", seq, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived);
            already_received = TRUE;
        }
    } else if (mcb->dseq_leftmost_allowed > mcb->dseq_leftmost_unreceived) {
        // Wrapping window
        if (seq >= mcb->dseq_leftmost_allowed || seq < mcb->dseq_leftmost_unreceived) {
            // RBNTBD: Remove the following T_IG() when debugging is done
            T_IG(TRACE_GRP_RX, "Rx(MD): Already Received case 2 (seq=%u, l.m.a.=%u, l.m.u.=%u)", seq, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived);
            already_received = TRUE;
        }
    }

    received_out_of_order = FALSE;

    if (!already_received) {
        // SEQ falls within [dseq_leftmost_unreceived; dseq_leftmost_unreceived + swinsz[
        // Check the dseq_ackd array to see if it's already received.
        // Find the idx into the dseq_ackd array.
        u32 an_idx;
        an_idx = SEQ_DIFF(mcb->dseq_leftmost_unreceived, seq);
        received_out_of_order = (an_idx != 0);

        if (received_out_of_order && MSG_TRACE_ENABLED(isid, dmodid)) {
            T_IG(TRACE_GRP_RX, "Rx(MD): ************* Received out of order. smac=%s, seq=%u, dmodid=%s, totlen=%u, offset=%u, sld. win before upd=[%u,%u,%u]", CX_mac2str(&frm[MSG_SMAC_OFFSET]), seq, vtss_module_names[dmodid], totlen, offset, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived, mcb->dseq_leftmost_unreceived + mcb->swinsz - 1);
        }

        // And now add the current dseq_ackd_idx, which is here to avoid
        // shifting the dseq_ackd array everytime an MD is received.
        an_idx += mcb->dseq_ackd_idx;
        // Wrap around
        if (an_idx >= MSG_CFG_MAX_WINSZ) {
            an_idx -= MSG_CFG_MAX_WINSZ;
        }
        // VTSS_BF_xxx() macros are defined in main.h
        if (VTSS_BF_GET(mcb->dseq_ackd, an_idx)) {
            // RBNTBD: Remove the following T_IG() when debugging is done
            T_IG(TRACE_GRP_RX, "Rx(MD): Already Received case 3 (seq=%u, l.m.a.=%u, l.m.u.=%u, a.i.=%u, r.o.o.o=%d, ds.a.i.=%u, ds.a.=%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)",
                 seq, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived, an_idx, received_out_of_order, mcb->dseq_ackd_idx,
                 mcb->dseq_ackd[0], mcb->dseq_ackd[1], mcb->dseq_ackd[2], mcb->dseq_ackd[3],
                 mcb->dseq_ackd[4], mcb->dseq_ackd[5], mcb->dseq_ackd[6], mcb->dseq_ackd[7]);
            already_received = TRUE;
        } else {
            VTSS_BF_SET(mcb->dseq_ackd, an_idx, 1);
        }
    }

    if (already_received) {
        if (MSG_TRACE_ENABLED(isid, dmodid)) {
            T_IG(TRACE_GRP_RX, "Rx(MD): Already received. smac=%s, seq=%u, dmodid=%s, totlen=%u, offset=%u, sld. win before upd=[%u,%u,%u]", CX_mac2str(&frm[MSG_SMAC_OFFSET]), seq, vtss_module_names[dmodid], totlen, offset, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived, mcb->dseq_leftmost_unreceived + mcb->swinsz - 1);
        }

        // RBNTBD: Take action if we cannot TX MACK.
        (void)TX_mack_create_and_tx(mcb, &frm[MSG_SMAC_OFFSET], MSG_MACK_STATUS_OK_BUT_ALREADY_RECEIVED, cid, seq);
        return TRUE;
    }

    // --------------------------------------------------------------------------
    // Update the sequence numbers
    // --------------------------------------------------------------------------
    while (VTSS_BF_GET(mcb->dseq_ackd, mcb->dseq_ackd_idx)) {
        u32 temp_idx;

        // dseq_ackd_idx
        mcb->dseq_ackd_idx++;
        if (mcb->dseq_ackd_idx == MSG_CFG_MAX_WINSZ) {
            mcb->dseq_ackd_idx = 0;
        }

        // Clear entry swinsz away in the dseq_ackd array
        temp_idx = mcb->dseq_ackd_idx + mcb->swinsz - 1;
        if (temp_idx >= MSG_CFG_MAX_WINSZ) {
            temp_idx -= MSG_CFG_MAX_WINSZ;
        }
        VTSS_BF_SET(mcb->dseq_ackd, temp_idx, 0);

        // dseq_leftmost_unreceived
        mcb->dseq_leftmost_unreceived++;
#if MSG_SEQ_CNT <= 65535UL
        if (mcb->dseq_leftmost_unreceived == MSG_SEQ_CNT) {
            mcb->dseq_leftmost_unreceived = 0;
        }
#endif

        // Only move leftmost_allowed if we've received beyond mcb->swinsz MDs
        // since last connection negotiation.
        if (SEQ_DIFF(mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived) > mcb->swinsz) {
            mcb->dseq_leftmost_allowed++;
#if MSG_SEQ_CNT <= 65535UL
            if (mcb->dseq_leftmost_allowed == MSG_SEQ_CNT) {
                mcb->dseq_leftmost_allowed = 0;
            }
#endif
        }
    }

    // --------------------------------------------------------------------------
    // Insert MD in correct msg
    // --------------------------------------------------------------------------
    // Figure out which message this MD belongs to.
    handled = FALSE;
    temp_msg = mcb->rx_msg_list;
    while (temp_msg) {
        if (RX_seq_falls_within(seq, temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq)) {
            // Found correct message. See if totlen matches.
            if (temp_msg->len != totlen) {
                // Oops. Invalid total length (not all fragments in the message
                // matched the length of the first fragment, that was received).
                // Report and discard this message.
                // RBNTBD: Re-negotiate connection?
                RX_proto_err(frm, "Not all MD fragments used the same TOTLEN (MD.TOTLEN = %u; MSG.TOTLEN = %u). temp_msg.l,u=%u,%u, seq=%u. Discarding", totlen, temp_msg->len, temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq, seq);
                return FALSE;
            }

            // Copy the payload into it.
            memcpy(&temp_msg->usr_msg[offset], payload, *payload_len);
            temp_msg->u.rx.frags_received++;
            handled = TRUE;
            if (offset == 0) {
                temp_msg->dmodid = dmodid; // We can only rely on the dmodid if it's the first MD of the user message, since DMODID is optional in the remaining.
            }
            break;
        } else {
            temp_msg = temp_msg->next;
        }
    }

    // Avoid Lint Warning 593: Custodial pointer 'rx_msg' possibly not freed or returned
    /*lint --e{593} */
    if (!handled) {
        // The MD wasn't part of an already created msg. Create a new one.
        rx_msg = VTSS_MALLOC(sizeof(msg_item_t));
        VTSS_ASSERT(rx_msg);

        // Allocate memory for the final user message.
        rx_msg->usr_msg = VTSS_MALLOC(totlen);
        VTSS_ASSERT(rx_msg->usr_msg);
        rx_msg->len = totlen;

        rx_msg->is_tx_msg = FALSE;
        rx_msg->next = NULL;
        rx_msg->u.rx.frags_received = 1;

        // Copy the payload into the user message
        memcpy(&rx_msg->usr_msg[offset], payload, *payload_len);

        if (offset == 0) {
            rx_msg->dmodid = dmodid;  // We can only rely on the dmodid if it's the first MD of the user message, since DMODID is optional in the remaining.
        }

        if (state.state == MSG_MOD_STATE_MST) {
            // When we're master, the connection ID is the slave's ISID.
            rx_msg->connid = isid;
        } else {
            // When we're slave, the connection ID is the "arbitrarily" chosen mcb->connid.
            rx_msg->connid = mcb->connid;
        }

        // Compute the left and right sequence numbers making up this message.
        if (totlen <= mcb->fragsz) {
            // That was the easy one, since the user message only takes on MD.
            rx_msg->u.rx.left_seq = rx_msg->u.rx.right_seq = seq;
        } else {
            // The message takes up more than one MD.
            int total_seq_cnt = (totlen + mcb->fragsz - 1) / mcb->fragsz;
            int dist_from_cur_seq_to_left = offset / mcb->fragsz;
            if (seq >= dist_from_cur_seq_to_left) {
                // No wrap
                rx_msg->u.rx.left_seq = seq - dist_from_cur_seq_to_left;
            } else {
                // Wrap
                rx_msg->u.rx.left_seq = MSG_SEQ_CNT - (dist_from_cur_seq_to_left - seq);
            }

            // Rightmost SEQ
            rx_msg->u.rx.right_seq = rx_msg->u.rx.left_seq + total_seq_cnt - 1;
#if MSG_SEQ_CNT <= 65535UL
            if (rx_msg->u.rx.right_seq >= MSG_SEQ_CNT) {
                rx_msg->u.rx.right_seq -= MSG_SEQ_CNT;
            }
#endif

            // If the leftmost sequence count is smaller than the minimum allowed sequence count,
            // we will never be able to receive the MDs for those due to the window size, and
            // messages will never be forwarded to the user modules.
            // Strictly speaking we should also check for leftmost_allowed here:
            if (dist_from_cur_seq_to_left > mcb->swinsz) {
                RX_proto_err(frm, "Rx'd a fragmented user message with an OFFSET (%u) so high that it is impossible to receive the previous MDs making up the message without exceeding the window size (%u). leftseq=%u, actseq=%u, Fragment size = (%u)", offset, mcb->swinsz, rx_msg->u.rx.left_seq, seq, mcb->fragsz);
                VTSS_FREE(rx_msg->usr_msg);
                VTSS_FREE(rx_msg);
                return FALSE;
            }
        }

        if (MSG_TRACE_ENABLED(isid, dmodid)) {
            T_IG(TRACE_GRP_RX, "Rx(MD - first): smac=%s, seq=%u, dmodid=%s, totlen=%u, offset=%u, seq=[%u,%u], sld. win after upd=[%u,%u,%u]", CX_mac2str(&frm[MSG_SMAC_OFFSET]), seq, vtss_module_names[dmodid], totlen, offset, rx_msg->u.rx.left_seq, rx_msg->u.rx.right_seq, mcb->dseq_leftmost_allowed, mcb->dseq_leftmost_unreceived, mcb->dseq_leftmost_unreceived + mcb->swinsz - 1);
        }

        // Find the location in mcb->rx_msg_list to insert the current message.
        temp_msg = mcb->rx_msg_list;
        last_msg = NULL;
        while (temp_msg) {
            u32 seq_diff = SEQ_DIFF(rx_msg->u.rx.left_seq, temp_msg->u.rx.left_seq);
            if (seq_diff < MSG_SEQ_CNT / 2) {
                // The rx_msg must be inserted before temp_msg.

                // If it's smaller than half the total sequence space, it must also be
                // smaller than the window size, because a previously received
                // message cannot be more than WINSZ away from the current.
                if (seq_diff == 0 || seq_diff >= mcb->swinsz) {
                    RX_proto_err(frm, "Previously received an MD with (l,r=%u,%u), which is more than WINSZ (%u) away from currently received left-most (l,r=%u,%u). MD.SEQ = %u", temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq, mcb->swinsz, rx_msg->u.rx.left_seq, rx_msg->u.rx.right_seq, seq);
                    VTSS_FREE(rx_msg->usr_msg);
                    VTSS_FREE(rx_msg);
                    return FALSE;
                }

                // There must not be an overlap between the two message sequences.
                if (RX_seq_overlap(rx_msg->u.rx.left_seq, rx_msg->u.rx.right_seq, temp_msg->u.rx.left_seq)) {
                    RX_proto_err(frm, "Received an MD, where SEQ reaches into previously received, but out-of-order, MD. cur.l = %u, cur.r = %u, prev.l = %u, prev.r = %u, MD.SEQ = %u", rx_msg->u.rx.left_seq, rx_msg->u.rx.right_seq, temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq, seq);
                    VTSS_FREE(rx_msg->usr_msg);
                    VTSS_FREE(rx_msg);
                    return FALSE;
                }

                // Insert the newly created message before temp_msg.
                rx_msg->next = temp_msg;
                if (last_msg) {
                    last_msg->next = rx_msg;
                } else {
                    mcb->rx_msg_list = rx_msg;
                }
                handled = TRUE;
                break;
            } else {
                // The rx_msg must be inserted somewhere after temp_msg.

                // Check that the new message's SEQ window lies after the temp_msg's
                // SEQ window.
                if (RX_seq_overlap(temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq, rx_msg->u.rx.left_seq)) {
                    RX_proto_err(frm, "Previously received an MD, whose sequence window reached into the current MD's sequence window. cur.l = %u, cur.r = %u, prev.l = %u, prev.r = %u, MD.SEQ = %u", rx_msg->u.rx.left_seq, rx_msg->u.rx.right_seq, temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq, seq);
                    VTSS_FREE(rx_msg->usr_msg);
                    VTSS_FREE(rx_msg);
                    return FALSE;
                }

                // Save a "prev" pointer.
                last_msg = temp_msg;
                temp_msg = temp_msg->next;
            }
        }

        // Check if the new message was inserted anywhere in the list
        if (!handled) {
            // No. It wasn't inserted. Add it to the end of the list (which is pointed to by last_msg).
            if (last_msg) {
                last_msg->next = rx_msg;
            } else {
                mcb->rx_msg_list = rx_msg;
            }
        }
    }

    // Time to acknowledge, update SEQ numbers and move the fully received messages to the rx list for dispatching.
    // RBNTBD: Take action if we cannot TX MACK.
    (void)TX_mack_create_and_tx(mcb, &frm[MSG_SMAC_OFFSET], received_out_of_order ? MSG_MACK_STATUS_OK_BUT_OUT_OF_ORDER : MSG_MACK_STATUS_OK, cid, seq);

    // --------------------------------------------------------------------------
    // Take off those messages that can be dispatched now
    // --------------------------------------------------------------------------

    // Send the fully received messages off for dispatching.
    // At this point the sequence counters are already updated.
    temp_msg = mcb->rx_msg_list;
    while (temp_msg) {
        u32 dist_right_to_unreceived = SEQ_DIFF(temp_msg->u.rx.right_seq, mcb->dseq_leftmost_unreceived);
        if (dist_right_to_unreceived > 0 && dist_right_to_unreceived <= mcb->swinsz) {
            u32 expected_frags = SEQ_DIFF(temp_msg->u.rx.left_seq, temp_msg->u.rx.right_seq) + 1;
            MSG_ASSERT(temp_msg->u.rx.frags_received == expected_frags, "Number of received fragments (%u) doesn't match expected (%u)", temp_msg->u.rx.frags_received, expected_frags);

            // Attach the msg to the pending Rx list, so that it will get forwarded
            // to the relevant user module in the RX_thread().
            mcb->rx_msg_list = temp_msg->next;
            temp_msg->next = NULL;
            temp_msg->state = state.state; // To be able to count in correct statistics bin.
            RX_put_list(mcb, temp_msg, temp_msg);
            temp_msg = mcb->rx_msg_list;
        } else {
            break;
        }
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_mack_parse()
/******************************************************************************/
static MSG_INLINE BOOL RX_mack_parse(u8 *frm, u32 frm_len, msg_mack_status_t *status, u16 *seq)
{
    BOOL null_seen = FALSE, status_seen = FALSE, seq_seen = FALSE, break_out = FALSE;
    u8   *pkt_ptr = &frm[MSG_TLV_OFFSET];
    u8   tlv_type;
    u8   tlv_len;
    u8   *tlv_val;
    u8   *frm_end = frm + frm_len;

    while (pkt_ptr < frm_end) {
        tlv_type = *(pkt_ptr++);
        if (tlv_type < 0x80) {
            // 1-byte TLV
            tlv_len = 1;
            tlv_val = pkt_ptr;
        } else {
            // n-byte TLV
            tlv_len = *(pkt_ptr++);
            tlv_val = pkt_ptr;
            if (tlv_len < 2) {
                RX_proto_err(frm, "TLV length of n-byte TLV is smaller than 2");
                return FALSE;
            }
        }
        tlv_val = pkt_ptr;
        pkt_ptr += tlv_len;

        switch (tlv_type) {
        case MSG_MACK_TLV_TYPE_NULL:
            if (*tlv_val != 0) {
                RX_proto_err(frm, "NULL_TLV val is non-zero");
                return FALSE;
            }
            null_seen = TRUE;
            break_out = TRUE;
            break;

        case MSG_MACK_TLV_TYPE_STATUS:
            *status = *tlv_val;
            status_seen = TRUE;
            break;

        case MSG_MACK_TLV_TYPE_SEQ:
            if (tlv_len != 2) {
                RX_proto_err(frm, "SEQ length is not 2 (actually %d)", tlv_len);
                return FALSE;
            }
            *seq = (tlv_val[0] << 8) | tlv_val[1];
            seq_seen = TRUE;
            break;

        default:
            // Allow unsupported TLVs.
            RX_proto_warn(frm, "Unsupported TLV type (%d)", tlv_type);
            break;
        }

        if (break_out) {
            break;
        }
    }

    // Check that the mandatory fields are present in the packet.
    if (status_seen == FALSE || seq_seen == FALSE || null_seen == FALSE) {
        RX_proto_err(frm, "At least one of the mandatory TLVs were not found in the packet (%d%d%d)", status_seen, seq_seen, null_seen);
        return FALSE;
    }

    // Check the frame length
    if ((frm_len <= 60 && pkt_ptr > frm_end) || (frm_len > 60 && pkt_ptr != frm_end)) {
        RX_proto_err(frm, "Frame length didn't match packet contents (frm_len = %u, packet contents size = %u)", frm_len, pkt_ptr - frm);
        return FALSE;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_mack()
/******************************************************************************/
static BOOL RX_mack(u8 *frm, u32 frm_len)
{
    u16               seq = 0;
    u8                cid;
    msg_mack_status_t mack_status = 0;
    msg_mcb_t         *mcb;
    msg_md_item_t     *md;
    msg_item_t        *tx_msg;
    vtss_isid_t       isid = 0;
    BOOL              stop;

    cid = frm[MSG_CID_OFFSET];
    if (cid >= MSG_CFG_CONN_CNT) {
        RX_proto_err(frm, "Invalid CID (%u)", cid);
        return FALSE;
    }

    if (!RX_mack_parse(frm, frm_len, &mack_status, &seq)) {
        return FALSE;
    }

    if (state.state == MSG_MOD_STATE_MST) {
        // Master. Lookup isid.
        // We need the isid before we print trace output
        // because the user may have chosen to filter it.
        isid = RX_mac2isid(&frm[MSG_SMAC_OFFSET]);
    }

    if (MSG_TRACE_ENABLED(isid, -1)) {
        T_DG(    TRACE_GRP_RX, "Rx(MACK): smac=%s, seq=%u, status=%u", CX_mac2str(&frm[MSG_SMAC_OFFSET]), seq, mack_status);
        T_RG(    TRACE_GRP_RX, "len=%u (first %u bytes shown)", frm_len, MIN(96, frm_len));
        T_RG_HEX(TRACE_GRP_RX, frm, MIN(96, frm_len));
    }

    if (state.state == MSG_MOD_STATE_MST) {
        if (isid == 0) {
            // No such slave.
            RX_proto_warn(frm, "Rx'd MACK as master from unknown slave (seq=%u)", seq);
            return FALSE;
        }
        mcb = &mcbs[isid][cid];
        if (mcb->state != MSG_CONN_STATE_MST_ESTABLISHED) {
            if (MSG_TRACE_ENABLED(isid, -1)) {
                RX_proto_warn(frm, "Rx'd MACK as master on a non-established connection (seq=%u)", seq);
            }
            return FALSE;
        }
    } else {
        // Slave. Check MACK's SMAC
        mcb = &mcbs[MSG_SLV_ISID_IDX][cid];
        if (memcmp(mcb->dmac, &frm[MSG_SMAC_OFFSET], VTSS_MAC_ADDR_SZ_BYTES) != 0) {
            RX_proto_warn(frm, "Rx'd MACK as slave from unknown master (seq=%u). Expected master=%s", seq, CX_mac2str(mcb->dmac));
            return FALSE;
        }
        if (mcb->state == MSG_CONN_STATE_SLV_WAIT_FOR_MSYNACKS_MACK) {
            if (seq != mcb->u.slave.msynack.cur_sseq) {
                RX_proto_warn(frm, "Unexpected SEQ (waiting for MSYNACK'S MACK). SEQ in frm=%u, expected SEQ=%u", seq, mcb->u.slave.msynack.cur_sseq);
                return FALSE;
            }
            if (mack_status >= MSG_MACK_STATUS_ERR_LVL) {
                RX_proto_warn(frm, "Waiting for MSYNACK'S MACK. Got MACK with error status=%u (seq=%u)", mack_status, seq);
                return FALSE;
            } else if (mack_status >= MSG_MACK_STATUS_WRN_LVL) {
                RX_proto_warn(frm, "Waiting for MSYNACK'S MACK. Got MACK with warning status=%u (seq=%u)", mack_status, seq);
            }
            mcb->state = MSG_CONN_STATE_SLV_ESTABLISHED;
            mcb->estab_time = time(NULL);
            return TRUE;
        } else if (mcb->state != MSG_CONN_STATE_SLV_ESTABLISHED) {
            RX_proto_warn(frm, "Rx'd MACK as slave on non-established connection (seq=%u)", seq);
            return FALSE;
        }
    }

    // The remainder is (almost) the same for master and slaves.

    // Match the MACK's SEQ against an MD
    tx_msg = mcb->tx_msg_list;
    md = NULL;
    stop = FALSE;
    while (tx_msg) {
        md = tx_msg->u.tx.md_list;
        while (md) {
            if (md->seq == seq) {
                if (md->acknowledged) {
                    if (MSG_TRACE_ENABLED(isid, md->dbg_dmodid)) {
                        RX_proto_warn(frm, "seq=%u already received", seq);
                    }
                }
                {
                    // Keep track of the longest roundtrip time for this connection.
                    cyg_tick_count_t cur_roundtrip_tick_cnt = cyg_current_time() - md->last_tx;
                    if (cur_roundtrip_tick_cnt > mcb->stat.max_roundtrip_tick_cnt) {
                        mcb->stat.max_roundtrip_tick_cnt = cur_roundtrip_tick_cnt;
                    }
                }
                md->acknowledged = TRUE;
                stop = TRUE;
                break;
            }
            md = md->next;
        }
        if (stop) {
            break;
        }
        tx_msg = tx_msg->next;
    }

    if (!md) {
        if (MSG_TRACE_ENABLED(isid, -1)) {
            RX_proto_warn(frm, "No MD matching seq=%d was found", seq);
        }
        return FALSE;
    }

    if (mack_status >= MSG_MACK_STATUS_ERR_LVL) {
        // Something went wrong. Cancel all MDs and return to MST_RDY to enforce a new connection.
        if (state.state == MSG_MOD_STATE_SLV) {
            RX_proto_err(frm, "Received MACK with error status (%u) from a master, which is illegal", mack_status);
        } else {
            if (MSG_TRACE_ENABLED(isid, md->dbg_dmodid)) {
                RX_proto_err(frm, "Rx'd MACK with error status (seq=%u, status=%u). Re-negotiating connection", seq, mack_status);
            }
        }
        CX_restart(mcb, isid, MSG_TX_RC_ERR_SLV_MACK_STATUS, MSG_TX_RC_ERR_MST_MACK_STATUS);
        return FALSE;
    }

    // Check if the whole message has been transmitted.
    if (tx_msg != mcb->tx_msg_list) {
        // It's not the first message in the list, so we cannot acknowledge the
        // transmission, but the MACK was successfully received, so return TRUE.
        return TRUE;
    }

    // Move all fully acknowledged messages to the pend_tx_done_list, which is
    // protected by a separate semaphore, so that the tx_done callback
    // functions can call msg_tx() without running into deadlocks.
    stop = FALSE;
    while (tx_msg) {
        md = tx_msg->u.tx.md_list;
        while (md) {
            if (md->acknowledged == FALSE) {
                if (md->tx_state == MSG_MD_TX_STATE_NEVER_TRANSMITTED) {
                    // The last MD within winsz has been acknowledged.
                    // Wake up the TX_thread(), so that it can transmit
                    // some more rather than timing out.
                    TX_wake_up_msg_thread(MSG_FLAG_TX_MORE_MDS);
                }
                stop = TRUE;
                break;
            }
            md = md->next;
        }

        if (stop) {
            break;
        }

        // We must continue with the next message in the list, because it may
        // be that we've previously received a MACK out of order.
        mcb->tx_msg_list = tx_msg->next;

        // Prepare for the insertion.
        tx_msg->next = NULL;

        // tx_msg is fully acknowledged. Move it to the Tx Done list.
        TX_put_done_list(mcb, tx_msg, tx_msg);

        tx_msg = mcb->tx_msg_list;
    }

    if (mcb->tx_msg_list == NULL) {
        mcb->tx_msg_list_last = NULL;
    }

    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE && !defined(MSG_RELAY_ALLOC) && MSG_HOP_BY_HOP
/******************************************************************************/
// RX_relay_tx_done()
// Called back after a relayed frame is transmitted. Since we've reused
// the extraction FDMA buffers, we need to call a special packet module
// function to release the buffers back to the FDMA.
/******************************************************************************/
static void RX_relay_tx_done(void *context, packet_tx_done_props_t *props)
{
    if (!props->tx) {
        T_WG(TRACE_GRP_RELAY, "Tx failed");
    }

    packet_rx_release_fdma_buffers(context);
}
#endif

#if VTSS_SWITCH_STACKABLE && MSG_HOP_BY_HOP
/******************************************************************************/
// RX_relay()
// Returns TRUE if the frame was successfully relayed, FALSE otherwise.
/******************************************************************************/
static BOOL RX_relay(u8 *frm, u32 len, u32 src_port, void *fdma_buffers)
{
    vtss_port_no_t    port_no;
    packet_tx_props_t tx_props;

#ifdef MSG_RELAY_ALLOC
    u8 *new_frm;
#endif

    int stat_idx = 1;

    if (src_port == PORT_NO_STACK_0) {
        stat_idx = 0;
    }

    MSG_STATE_CRIT_ENTER();
    state.relay_stat[stat_idx].rx++;
    MSG_STATE_CRIT_EXIT();

    // If the HTL field is dropping to zero, count and discard.
    if ((--frm[MSG_HTL_OFFSET]) == 0) {
        T_WG(TRACE_GRP_RELAY, "HTL is dropping to zero. Discarding. First 48 bytes of packet:");
        T_WG_HEX(TRACE_GRP_RELAY, frm, 48);
        MSG_STATE_CRIT_ENTER();
        state.relay_stat[stat_idx].htl_err++;
        MSG_STATE_CRIT_EXIT();
        return FALSE;
    }

    port_no = TX_mac2port(&frm[MSG_DMAC_OFFSET]);

    if (!PORT_NO_IS_STACK(port_no)) {
        // Probably the destination is down. Count the event and don't transmit
        // since that will cause a fatal error in the packet_tx() function.
        T_WG(TRACE_GRP_RELAY, "Invalid stack port number returned from Topo (%u). Not relaying frame", port_no);
        MSG_STATE_CRIT_ENTER();
        state.relay_stat[stat_idx].inv_port_err++; // Counted on ingress idx, naturally
        MSG_STATE_CRIT_EXIT();
        return FALSE;
    }

    stat_idx = 1;
    if (port_no == PORT_NO_STACK_0) {
        stat_idx = 0;
    }

#ifdef MSG_RELAY_ALLOC
    new_frm = packet_tx_alloc(len);
    VTSS_ASSERT(new_frm);
    memcpy(new_frm, frm, len);
#endif

    // In hop-by-hop mode, the VStaX2 header is fixed and can be found in mcbs[0][0].vs2_frm_hdr.
#ifdef MSG_RELAY_ALLOC
    TX_props_compose(&mcbs[MSG_SLV_ISID_IDX][0], &tx_props, new_frm, len, port_no, NULL,             NULL);
#else
    TX_props_compose(&mcbs[MSG_SLV_ISID_IDX][0], &tx_props, frm,     len, port_no, RX_relay_tx_done, fdma_buffers);
#endif

    T_DG(TRACE_GRP_RELAY, "RX Relay. port_no=%u, len=%u", port_no, len);

#ifdef MSG_RELAY_ALLOC
    T_RG_HEX(TRACE_GRP_RELAY, new_frm, len);
#else
    T_RG_HEX(TRACE_GRP_RELAY, frm, len);
#endif

    if (TX_pkt(&tx_props) != VTSS_RC_OK) {
        MSG_STATE_CRIT_ENTER();
        state.relay_stat[stat_idx].tx_err++;
        MSG_STATE_CRIT_EXIT();
        T_WG(TRACE_GRP_RELAY, "packet_tx failed");
        return FALSE;
    } else {
        MSG_STATE_CRIT_ENTER();
        state.relay_stat[stat_idx].tx_ok++;
        MSG_STATE_CRIT_EXIT();
    }
    return TRUE;
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_pkt_analyze()
/******************************************************************************/
static void RX_pkt_analyze(u8 *frm, u32 len)
{
    msg_pdu_type_t pdu_type = frm[MSG_PDU_TYPE_OFFSET];
    BOOL           result;
    u32            payload_len; // Only for statistics on MD byte count.

    MSG_STATE_CRIT_ENTER();

    switch (pdu_type) {
    case MSG_PDU_TYPE_MSYN:
        // Ignore MSYNs until the master-down event has been passed to all user modules,
        // in order to avoid race conditions.
        if (!CX_user_state.ignore_msyns) {
            result = RX_msyn(frm, len);
        } else {
            T_IG(TRACE_GRP_INIT_MODULES, "Ignoring MSYN");
            result = TRUE; // Count it in good bin, even though we haven't parsed it.
        }
        break;

    case MSG_PDU_TYPE_MSYNACK:
        result = RX_msynack(frm, len);
        break;

    case MSG_PDU_TYPE_MD:
        result = RX_md(frm, len, &payload_len);
        state.pdu_stat[state.state].rx_md_bytes[result ? 0 : 1] += payload_len;
        break;

    case MSG_PDU_TYPE_MACK:
        result = RX_mack(frm, len);
        break;

    default:
        RX_proto_warn(frm, "Unknown PDU type (%d)", pdu_type);
        result = FALSE;
        break;
    }

    // Count the event (even though PDU types of 0x00 aren't used, we count it in that
    // bin. When obtaining the total number of unknown PDU type events, add
    // the two bins indexed by 0 and MSG_PDU_TYPE_LAST_ENTRY.
    state.pdu_stat[state.state].rx[MIN(pdu_type, MSG_PDU_TYPE_LAST_ENTRY)][result ? 0 : 1]++;

    MSG_STATE_CRIT_EXIT();
}
#endif

#if VTSS_SWITCH_STACKABLE
/******************************************************************************/
// RX_pkt()
// Receive a Message Protocol frame.
/******************************************************************************/
#if MSG_HOP_BY_HOP && !defined(MSG_RELAY_ALLOC)
static BOOL RX_pkt(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info, const void *const fdma_buffers)
#else
static BOOL RX_pkt(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info)
#endif
{
#if MSG_HOP_BY_HOP
    BOOL release_fdma_buffers = TRUE;
#if defined(MSG_RELAY_ALLOC)
    void *fdma_buffers = NULL;
#endif
#endif
    u8 *non_read_only_frm = (u8 *)frm;

    non_read_only_frm[MSG_DMAC_OFFSET] &= ~0x01;

    // Check the DMAC to see if this frame is for us.
    BOOL for_us = memcmp(&frm[MSG_DMAC_OFFSET], this_mac, sizeof(mac_addr_t)) == 0;

    if (!for_us) {
#if MSG_HOP_BY_HOP
        // Not for us. Relay it. The function returns TRUE if the frame was successfully
        // sent to the FDMA, in which case we must not release the FDMA buffers here,
        // since it's done in the RX_relay_tx_done() function.

        // RX_relay() dereferences mcbs[][], but it's fixed entries, so tell lint that.
        /*lint -esym(459, RX_pkt) */
        release_fdma_buffers = !RX_relay((u8 *)frm, rx_info->length, rx_info->port_no, (void *)fdma_buffers);
#else
        // Odd. It's not for us, and we're not using hop-by-hop relaying.
        T_W("Received a message that was not for this switch");
#endif
    } else {
        // For us. Analyze it.
        RX_pkt_analyze((u8 *)frm, rx_info->length);
    }

#if MSG_HOP_BY_HOP && !defined(MSG_RELAY_ALLOC)
    if (release_fdma_buffers) {
        packet_rx_release_fdma_buffers((void *)fdma_buffers);
    }
#endif
    return TRUE; // Don't allow other subscribers to receive the packet (return value is not used when using the cb_adv callback function).
}
#endif

/****************************************************************************/
/*                                                                          */
/*  COMMON INTERNAL FUNCTIONS, PART 2                                       */
/*                                                                          */
/****************************************************************************/

/******************************************************************************/
// CX_thread_post_init()
/******************************************************************************/
static void CX_thread_post_init(void)
{
#if VTSS_SWITCH_STACKABLE
    packet_rx_filter_t rx_filter;
    void *filter_id;
    vtss_port_no_t stack_0, stack_1;
#endif
    vtss_rc rc;

    // Get this switch's MAC address */
    rc = conf_mgmt_mac_addr_get(this_mac, 0);
    VTSS_ASSERT(rc >= 0);

#if VTSS_SWITCH_STACKABLE
    // If stacking is disabled, we shouldn't register for message protocol packets.
    if (vtss_stacking_enabled()) {
#if MSG_HOP_BY_HOP
        // For relaying messages we need to have a predefined VStaX2 header. Whether or not
        // we are master, we use mcbs[0][0].vs2_frm_hdr, which is OK because the VStaX2 header
        // is fixed and never changing when we're in hop-by-hop mode.
        CX_create_vs2_hdr(VTSS_VSTAX_UPSID_UNDEF, mcbs[MSG_SLV_ISID_IDX][0].vs2_frm_hdr);
#else
        // If not in hop-by-hop mode, we need to register for changes in UPSID, because
        // the VStaX2 header must be changed if a switch changes upsid.
        if ((rc = topo_upsid_change_callback_register(CX_upsid_change, VTSS_MODULE_ID_MSG)) != VTSS_RC_OK) {
            T_E("topo_upsid_change_callback_register() failed: %s", error_txt(rc));
        }
#endif

        // Subscribe to Message Protocol Frames received on stack port A or B, only.
        // If not subscribing to these two ports, only, we'd have a security hole.
        memset(&rx_filter, 0, sizeof(rx_filter));
        rx_filter.modid  = VTSS_MODULE_ID_MSG;
        rx_filter.match  = PACKET_RX_FILTER_MATCH_SSPID | PACKET_RX_FILTER_MATCH_SRC_PORT;
        rx_filter.prio   = PACKET_RX_FILTER_PRIO_SUPER;
#if defined(MSG_RELAY_ALLOC) || !MSG_HOP_BY_HOP
        rx_filter.cb     = RX_pkt;
#else
        rx_filter.cb_adv = RX_pkt;
#endif
        rx_filter.sspid  = 0x0002; // Message Protocol Switch Stack Protocol ID
        stack_0 = PORT_NO_STACK_0;
        stack_1 = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(rx_filter.src_port_mask, stack_0, 1);
        VTSS_PORT_BF_SET(rx_filter.src_port_mask, stack_1, 1);
        rc = packet_rx_filter_register(&rx_filter, &filter_id);
        MSG_ASSERT(rc >= 0, "rc=%d", rc);
    }
#endif

    // Resume the RX_thread
    cyg_thread_resume(RX_msg_thread_handle);
}

/****************************************************************************/
// TX_handle_tx()
// crit_msg_state taken prior to this call.
/****************************************************************************/
static void TX_handle_tx(msg_mcb_t *mcb, vtss_isid_t disid, u8 cid, cyg_tick_count_t cur_time)
{
#if VTSS_SWITCH_STACKABLE
    u16 ush;
    u8  *uch_ptr;
#endif

    switch (mcb->state) {
    case MSG_CONN_STATE_MST_NO_SLV:
        // Slave not present. Nothing to do.
        break;

#if VTSS_SWITCH_STACKABLE
    case MSG_CONN_STATE_MST_RDY:
        // Slave present. Start transmitting MSYNs.
        if (TX_msyn_create(mcb, disid, cid)) {
            TX_msyn(mcb, disid, cur_time);
            mcb->state = MSG_CONN_STATE_MST_WAIT_FOR_MSYNACK;
        } else {
            // Couldn't get slave's MAC address.
            state.pdu_stat[state.state].tx[MSG_PDU_TYPE_MSYN][0][1]++;
            mcb->stat.tx_pdu[MSG_PDU_TYPE_MSYN][1]++;
        }
        break;

    case MSG_CONN_STATE_MST_WAIT_FOR_MSYNACK:
        if (MSG_TIMEDOUT(mcb->u.master.msyn.last_tx, cur_time, state.msg_cfg_msyn_timeout_ms)) {
            // Gotta update the frame with a new MSEQ before re-transmitting it.
            ush = htons(mcb->next_available_sseq);
            uch_ptr = mcb->u.master.msyn.mseq_tlv_ptr;
            TX_add_nbyte_tlv(&uch_ptr, MSG_MSYN_TLV_TYPE_MSEQ, 2, (u8 *)&ush);
            mcb->u.master.msyn.cur_mseq = mcb->next_available_sseq;
            mcb->next_available_sseq++;
#if MSG_SEQ_CNT <= 65535UL
            if (mcb->next_available_sseq == MSG_SEQ_CNT) {
                mcb->next_available_sseq = 0;
            }
#endif
            TX_msyn(mcb, disid, cur_time);
        }
        break;
#endif

    case MSG_CONN_STATE_MST_ESTABLISHED:
        if (disid == state.misid) {
            // Loopback!
            // Since we're not allowed to call back the Tx Done User Module
            // callback function while we own the crit_msg_state (as we do here), we
            // transfer the list of messages to the pend_rx_list, which can only
            // be updated by the RX_thread and therefore doesn't need mutex-
            // protection.
            if (mcb->tx_msg_list) {
                RX_put_list(mcb, mcb->tx_msg_list, mcb->tx_msg_list_last);
                mcb->tx_msg_list      = NULL;
                mcb->tx_msg_list_last = NULL;
            }
        } else {
#if VTSS_SWITCH_STACKABLE
            (void)TX_mds(mcb, disid, cid, cur_time);
#endif
        }
        break;

    case MSG_CONN_STATE_MST_STOP:
        // Slave doesn't support this CID. Nothing to do.
        break;

    case MSG_CONN_STATE_SLV_NO_MST:
        break;

    case MSG_CONN_STATE_SLV_WAIT_FOR_MSYNACKS_MACK:
        break;

    case MSG_CONN_STATE_SLV_ESTABLISHED:
#if VTSS_SWITCH_STACKABLE
        (void)TX_mds(mcb, 0, cid, cur_time);
#endif
        break;

    default:
        VTSS_ASSERT(FALSE);
        break; // Unreachable
    }
}

/****************************************************************************/
// TX_thread()
// Handles connection establishment, transmission, and Tx-done callback for
// non-looped-back messages.
//
// Originally, the TX_thread and RX_thread were one single thread, but it
// turned out to give rise to a deadlock for the following reason:
//   Some user modules have a semaphore that protects one single "reply
//   message". This reply message's semaphore is waited for in the
//   user module's msg RX callback function if the Rx'd message calls for
//   a reply. The semaphore is released in the user module's Tx Done
//   function. If two requests arrived quickly after each other, the
//   first invokation of the RX callback function would acquire the semaphore
//   and Tx a reply. If the second invokation of the RX callback function
//   occurs before the first reply was sent, the RX callback function would
//   wait for the reply buffer semaphore, and because the TxDone was called
//   from the same thread as the Rx callback, the TxDone would never get
//   called, resulting in a deadlock.
//   Now, with two threads, the TxDone will eventually occur, causing the
//   RX callback to be able to continue execution.
//   Even with two threads, the loopback case will still fail, since
//   looped back messages must be RX called back before they can be TxDone.
//   called back. Since there is a strict order on these two events, it
//   doesn't help to move the TxDone call to the Tx thread. The remedy is to
//   implement another another msg_tx option, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK,
//   which, when not used, causes the msg_tx_adv() function to VTSS_MALLOC() and
//   memcpy the user message, then callback the TxDone function, before
//   actually calling the RX function.
/****************************************************************************/
static void TX_thread(cyg_addrword_t data)
{
    int i, cid;
    msg_item_t *msg;
    cyg_tick_count_t cur_time;
    int start_idx, end_idx, msgs_freed;

    // Load configuration and initialize some global variables.
    CX_thread_post_init();

    // Now, we're ready to accept calls.
    /*lint --e{455} */
    MSG_CFG_CRIT_EXIT();
    MSG_STATE_CRIT_EXIT();
    MSG_COUNTERS_CRIT_EXIT();
    MSG_PEND_LIST_CRIT_EXIT();
    MSG_IM_FIFO_CRIT_EXIT();

    while (1) {
        // Wait until we get an event or we timeout.
        // We timeout every MSG_SAMPLE_TIME_MS msecs.
        (void)cyg_flag_timed_wait(&TX_msg_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, cyg_current_time() + (MSG_SAMPLE_TIME_MS / ECOS_MSECS_PER_HWTICK));

        // Get the mutex, so that we can safely manipulate the internal state.
        MSG_STATE_CRIT_ENTER();

        // Sample this one once, only and use it throughout the below code
        cur_time = cyg_current_time();

        /*****************************************************************/
        // MUTEX OWNED. DO NOT CALL BACK ANY USER MODULE FUNCTION
        /*****************************************************************/

        // Loop through all the MCBs defined in this state and
        // handle connection establishment, timeout, retransmission, etc.
        if (state.state == MSG_MOD_STATE_MST) {
            start_idx = MSG_MST_ISID_START_IDX;
            end_idx   = MSG_MST_ISID_END_IDX;
        } else {
            start_idx = MSG_SLV_ISID_IDX;
            end_idx   = MSG_SLV_ISID_IDX;
        }

        for (i = start_idx; i <= end_idx; i++) {
            for (cid = MSG_CFG_CONN_CNT - 1; cid >= 0; cid--) {
                // Start from behind due to the inherited priority
                TX_handle_tx(&mcbs[i][cid], i, cid, cur_time);
            }
        }

        // Release the mutex, so that we can callback User Module functions.
        MSG_STATE_CRIT_EXIT();

        /*****************************************************************/
        // MUTEX RELEASED. NOW CALL BACK USER MODULE FUNCTIONS AS NEEDED
        // BUT BE AWARE WHEN UPDATING PER-CONNECTION OR STATE-DEPENDENT
        // COUNTERS. THIS MAY HAPPEN IN A WRONG BIN, BECAUSE AT THIS EXACT
        // MOMENT IN TIME, A TOPOLOGY CHANGE MAY OCCUR.
        /*****************************************************************/

        msgs_freed = 0;

        // Loop through the list pending to be Tx-done called back.
        while ((msg = CX_get_pend_list(&pend_tx_done_list, &pend_tx_done_list_last)) != NULL) {
            BOOL tx_done = TRUE;
            // A msg may end up in the pend_tx_done_list even if it's not transmitted.
            // This may happen when a topology change occurs or e.g. an MSYN arrives.
            // If an MD has been attempted transmitted, but not yet returned from the
            // packet module, we must not deallocate it, since this may result in garbage
            // being transmitted or - even worse - the front port queue system to get
            // corrupted, because an invalid IFH is injected.
            // Therefore, we check if all MDs making up the message that have been sent
            // off to the FDMA are actually acknowledged by the FDMA (i.e. TX_md_tx_done()
            // has been called back). If so, we can safely free the message and its
            // corresponding MDs. If not, we need to store it back in the front of the
            // pend_tx_done_list and go back to sleep. The TX_md_tx_done() function
            // will cause us to be woken up again later on.
            if (msg && msg->is_tx_msg && msg->u.tx.md_list) {
                // Temporarily re-acuiqre the MSG_STATE_CRIT, which is used by TX_md_tx_done()
                // to signal that it's tx'd.
                msg_md_item_t *md = msg->u.tx.md_list;
                MSG_STATE_CRIT_ENTER();
                while (md) {
                    if (md->tx_state == MSG_MD_TX_STATE_TRANSMITTED_BUT_NOT_TXDONE_ACKD) {
                        // Message hasn't been returned by the FDMA yet. Go back to sleep.
                        tx_done = FALSE;
                        break;
                    }
                    md = md->next;
                }
                MSG_STATE_CRIT_EXIT();
            }
            if (tx_done) {
                // We can safely free it.
                CX_free_msg(msg);
                if (++msgs_freed == 10) {
                    // 10 is arbitrarily chosen. Give TX_handle_tx() a chance to run now,
                    // by signaling to ourselves that we want to run again ASAP.
                    // If we didn't do this, we might end up in a situation, with almost 100%
                    // CPU load spent by other threads that are kept awake by the fact that
                    // we release messages to them but don't actually send messages out,
                    // eventually causing a memory depletion. This is primarily a problem
                    // with looped messages, which may be freed before their Rx handler
                    // is called back (with another copy of the message).
                    TX_wake_up_msg_thread(MSG_FLAG_TX_MORE_WORK);
                    break;
                }
            } else {
                // Put it back into the pend_tx_done_list (in the front)
                // and go back to sleep.
                TX_put_done_list_front(msg);
                break;
            }
        }
    }
}

/****************************************************************************/
// RX_thread()
// Handles Rx callback for all messages and Tx-done for looped back messages.
/****************************************************************************/
static void RX_thread(cyg_addrword_t data)
{
    msg_item_t *msg;

    while (1) {
        // Wait until we get an event or we timeout.
        // We timeout every MSG_SAMPLE_TIME_MS msecs.
        (void)cyg_flag_wait(&RX_msg_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        // Loop through the list pending to be Rx called back (and
        // Tx-done called back when the Rx is done, in case of loopback).
        while ((msg = CX_get_pend_list(&pend_rx_list, &pend_rx_list_last)) != NULL) {
            // msg is now safely removed from the list.
            RX_msg_dispatch(msg);
            CX_free_msg(msg); // Causes a Tx Done callback if loopback
        }
    }
}

/****************************************************************************/
/*                                                                          */
/*  DEBUG INTERNAL FUNCTIONS                                                */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// DBG_cmd_syntax_error()
/****************************************************************************/
static void DBG_cmd_syntax_error(msg_dbg_printf_t dbg_printf, const char *fmt, ...)
{
    va_list ap = NULL;
    char s[200] = "Command syntax error: ";
    int len;

    len = strlen(s);

    va_start(ap, fmt);

    (void)vsnprintf(s + len, sizeof(s) - len - 1, fmt, ap);
    (void)dbg_printf("%s\n", s);

    va_end(ap);
}

/****************************************************************************/
// DBG_cmd_stat_usr_msg_print()
// cmd_text   : "Print User Message Statistics"
// arg_syntax : NULL,
// max_arg_cnt: 0,
/****************************************************************************/
static void DBG_cmd_stat_usr_msg_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int modid, state_idx, rc_idx;
    BOOL rx_without_subscribers_exists = FALSE;
    BOOL rx_with_subscribers_exists    = FALSE;
    BOOL rc_exists                     = FALSE;
    BOOL rc_exists_per_rc[MSG_TX_RC_LAST_ENTRY];
    BOOL print_this_modid[VTSS_MODULE_ID_NONE + 1];

    memset(print_this_modid, 0, sizeof(print_this_modid));
    memset(rc_exists_per_rc, 0, sizeof(rc_exists_per_rc));

    MSG_COUNTERS_CRIT_ENTER();

    // First figure out what to print
    for (modid = 0; modid <= VTSS_MODULE_ID_NONE; modid++) {
        for (state_idx = 0; state_idx < MSG_MOD_STATE_LAST_ENTRY; state_idx++) {
            if (state.usr_stat[state_idx].rx[modid][0] != 0 || state.usr_stat[state_idx].tx[modid][0] != 0 || state.usr_stat[state_idx].tx[modid][1] != 0) {
                print_this_modid[modid] = TRUE;
                rx_with_subscribers_exists = TRUE;
            }
            if (state.usr_stat[state_idx].rx[modid][1] != 0) {
                rx_without_subscribers_exists = TRUE;
            }
        }
    }

    (void)dbg_printf("Received and transmitted User Messages:\n");
    (void)dbg_printf("                        ---------------------- As Master --------------------- ---------------------- As Slave ----------------------\n");
    (void)dbg_printf("Module              ID  Rx OK      Tx OK      Tx Err     Rx Bytes   Tx Bytes   Rx OK      Tx OK      Tx Err     Rx Bytes   Tx Bytes  \n");
    (void)dbg_printf("------------------- --- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n");
    if (rx_with_subscribers_exists) {
        for (modid = 0; modid <= VTSS_MODULE_ID_NONE; modid++) {
            if (print_this_modid[modid]) {
                (void)dbg_printf("%-19s %3d %10u %10u %10u %10u %10u %10u %10u %10u %10u %10u\n", vtss_module_names[modid], modid,
                                 state.usr_stat[MSG_MOD_STATE_MST].rx[modid][0],
                                 state.usr_stat[MSG_MOD_STATE_MST].tx[modid][0],
                                 state.usr_stat[MSG_MOD_STATE_MST].tx[modid][1],
                                 state.usr_stat[MSG_MOD_STATE_MST].rxb[modid],
                                 state.usr_stat[MSG_MOD_STATE_MST].txb[modid],
                                 state.usr_stat[MSG_MOD_STATE_SLV].rx[modid][0],
                                 state.usr_stat[MSG_MOD_STATE_SLV].tx[modid][0],
                                 state.usr_stat[MSG_MOD_STATE_SLV].tx[modid][1],
                                 state.usr_stat[MSG_MOD_STATE_SLV].rxb[modid],
                                 state.usr_stat[MSG_MOD_STATE_SLV].txb[modid]);
            }
        }
    } else {
        (void)dbg_printf("<none>\n");
    }

    (void)dbg_printf("\nReceived User Messages without subscribers:\n");
    (void)dbg_printf("Module              ID  As Master  As Slave  \n");
    (void)dbg_printf("------------------- --- ---------- ----------\n");
    if (rx_without_subscribers_exists) {
        for (modid = 0; modid <= VTSS_MODULE_ID_NONE; modid++) {
            if (state.usr_stat[MSG_MOD_STATE_MST].rx[modid][1] != 0 || state.usr_stat[MSG_MOD_STATE_SLV].rx[modid][1] != 0) {
                (void)dbg_printf("%-19s %3d %10u %10u\n", vtss_module_names[modid], modid, state.usr_stat[MSG_MOD_STATE_MST].rx[modid][1], state.usr_stat[MSG_MOD_STATE_SLV].rx[modid][1]);
            }
        }
    } else {
        (void)dbg_printf("<none>\n");
    }

    for (rc_idx = 0; rc_idx < MSG_TX_RC_LAST_ENTRY; rc_idx++) {
        if (state.usr_stat[MSG_MOD_STATE_MST].tx_per_return_code[rc_idx] != 0 || state.usr_stat[MSG_MOD_STATE_SLV].tx_per_return_code[rc_idx] != 0) {
            rc_exists_per_rc[rc_idx] = TRUE;
            rc_exists = TRUE;
        }
    }

    (void)dbg_printf("\nTx Done return code (rc) counters:\n");
    (void)dbg_printf("rc As Master  As Slave\n");
    (void)dbg_printf("-- ---------- ----------\n");
    if (rc_exists) {
        for (rc_idx = 0; rc_idx < MSG_TX_RC_LAST_ENTRY; rc_idx++) {
            if (rc_exists_per_rc[rc_idx]) {
                (void)dbg_printf("%2d %10u %10u\n", rc_idx, state.usr_stat[MSG_MOD_STATE_MST].tx_per_return_code[rc_idx], state.usr_stat[MSG_MOD_STATE_SLV].tx_per_return_code[rc_idx]);
            }
        }
    } else {
        (void)dbg_printf("<none>\n");
    }

    (void)dbg_printf("\n");
    MSG_COUNTERS_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_stat_protocol_print()
// cmd_text   : "Print Message Protocol Statistics"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_protocol_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_isid_t isid;
    int cid;
    int state_idx;
    int pdu_type_idx;

    MSG_STATE_CRIT_ENTER();

    (void)dbg_printf("\nOverall, persistent statistics:\n");
    (void)dbg_printf("PDU Type State  Rx OK      Tx OK      Re-Tx OK   Rx Err     Tx Err     Re-Tx Err \n");
    (void)dbg_printf("-------- ------ ---------- ---------- ---------- ---------- ---------- ----------\n");
    for (state_idx = 0; state_idx < 2; state_idx++) {
        for (pdu_type_idx = MSG_PDU_TYPE_MSYN; pdu_type_idx <= MSG_PDU_TYPE_LAST_ENTRY; pdu_type_idx++) {
            u32 rx_good, tx_good, retx_good, rx_bad, tx_bad, retx_bad;

            rx_good = state.pdu_stat[state_idx].rx[pdu_type_idx][0];
            rx_bad  = state.pdu_stat[state_idx].rx[pdu_type_idx][1];

            if (pdu_type_idx < MSG_PDU_TYPE_LAST_ENTRY) {
                tx_good   = state.pdu_stat[state_idx].tx[pdu_type_idx][0][0];
                retx_good = state.pdu_stat[state_idx].tx[pdu_type_idx][1][0];
                tx_bad    = state.pdu_stat[state_idx].tx[pdu_type_idx][0][1];
                retx_bad  = state.pdu_stat[state_idx].tx[pdu_type_idx][1][1];
            } else {
                // Unknown PDU types are counted in both bin 0 and the last bin.
                rx_good += state.pdu_stat[state_idx].rx[0][0];
                rx_bad  += state.pdu_stat[state_idx].rx[0][1];
                // There's no such thing as Tx'd unknown PDUs
                tx_good   = 0;
                retx_good = 0;
                tx_bad    = 0;
                retx_bad  = 0;
            }
            (void)dbg_printf("%-8s %-6s %10u %10u %10u %10u %10u %10u\n", PDUTYPE2STR(pdu_type_idx), state_idx == MSG_MOD_STATE_SLV ? "Slave" : "Master", rx_good, tx_good, retx_good, rx_bad, tx_bad, retx_bad);
        }
    }

    (void)dbg_printf("\n\nOverall, persistent payload statistics (in bytes):\n");
    (void)dbg_printf("State  Rx OK      Tx OK      Rx Err     Timeouts  \n");
    (void)dbg_printf("------ ---------- ---------- ---------- ----------\n");
    for (state_idx = 0; state_idx < 2; state_idx++) {
        (void)dbg_printf("%-6s %10llu %10llu %10llu %10u\n",
                         state_idx == MSG_MOD_STATE_SLV ? "Slave" : "Master",
                         state.pdu_stat[state_idx].rx_md_bytes[0],
                         state.pdu_stat[state_idx].tx_md_bytes,
                         state.pdu_stat[state_idx].rx_md_bytes[1],
                         state.pdu_stat[state_idx].tx_md_timeouts);
    }

    (void)dbg_printf("\n\nPer-connection, volatile statistics (counted in user messages - not PDUs):\n");
    if (state.state == MSG_MOD_STATE_MST) {
        (void)dbg_printf("ISID M State      Exists UPSID Established @             Uptime [s] Rx OK     Tx OK       Tx Err     Roundtrip [ms]");
        if (MSG_CFG_CONN_CNT > 1) {
            (void)dbg_printf(" CID");
        }
        (void)dbg_printf("\n");

        (void)dbg_printf("---- - ---------- ------ ----- ------------------------- ---------- ---------- ---------- ---------- --------------");
        if (MSG_CFG_CONN_CNT > 1) {
            (void)dbg_printf(" ---");
        }
        (void)dbg_printf("\n");

        for (isid = MSG_MST_ISID_START_IDX; isid <= MSG_MST_ISID_END_IDX; isid++) {
            for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                msg_mcb_t *mcb = &mcbs[isid][cid];
                if (mcb->state != MSG_CONN_STATE_MST_NO_SLV) {
#if VTSS_SWITCH_STACKABLE
                    (void)dbg_printf("%4d %1s %10s %-6s %5d ",   isid, isid == state.misid ? "Y" : "N", CX_state2str(mcb->state), CX_user_state.exists[isid] ? "Y" : "N", mcb->upsid);
#else
                    (void)dbg_printf("%4d %1s %10s %-6s N/A   ", isid, isid == state.misid ? "Y" : "N", CX_state2str(mcb->state), CX_user_state.exists[isid] ? "Y" : "N");
#endif
                    if (mcb->state == MSG_CONN_STATE_MST_ESTABLISHED) {
                        (void)dbg_printf("%25s ", misc_time2str(mcb->estab_time == 0 ? 1 : mcb->estab_time));
                    } else {
                        (void)dbg_printf("%25s ", "N/A");
                    }
                    (void)dbg_printf("%10u %10u %10u %10u %14llu",
                                     CX_uptime_get_crit_taken(isid),
                                     mcb->stat.rx_msg,
                                     mcb->stat.tx_msg[0],
                                     mcb->stat.tx_msg[1],
                                     mcb->stat.max_roundtrip_tick_cnt * ECOS_MSECS_PER_HWTICK);

                    if (MSG_CFG_CONN_CNT > 1) {
                        (void)dbg_printf(" %3d", cid);
                    }
                    (void)dbg_printf("\n");
                }
            }
        }
    } else {
        (void)dbg_printf("CONID State      UPSID Established @             Uptime [s] Rx OK     Tx OK       Tx Err     Roundtrip [ms]");
        if (MSG_CFG_CONN_CNT > 1) {
            (void)dbg_printf(" CID");
        }
        (void)dbg_printf("\n");
        (void)dbg_printf("----- ---------- ----- ------------------------- ---------- ---------- ---------- ---------- --------------");
        if (MSG_CFG_CONN_CNT > 1) {
            (void)dbg_printf(" ---");
        }
        (void)dbg_printf("\n");
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            msg_mcb_t *mcb = &mcbs[MSG_SLV_ISID_IDX][cid];
            if (mcb->state != MSG_CONN_STATE_SLV_NO_MST) {
#if VTSS_SWITCH_STACKABLE
                (void)dbg_printf("%5u %10s %5d ", mcb->connid, CX_state2str(mcb->state), mcb->upsid);
#else
                (void)dbg_printf("%5u %10s N/A   ", mcb->connid, CX_state2str(mcb->state));
#endif
                if (mcb->state == MSG_CONN_STATE_SLV_ESTABLISHED) {
                    (void)dbg_printf("%25s ", misc_time2str(mcb->estab_time == 0 ? 1 : mcb->estab_time));
                } else {
                    (void)dbg_printf("%25s ", "N/A");
                }
                (void)dbg_printf("%10u %10u %10u %10u %14llu",
                                 CX_uptime_get_crit_taken(VTSS_ISID_LOCAL),
                                 mcb->stat.rx_msg, mcb->stat.tx_msg[0],
                                 mcb->stat.tx_msg[1],
                                 mcb->stat.max_roundtrip_tick_cnt * ECOS_MSECS_PER_HWTICK);
            }
            if (MSG_CFG_CONN_CNT > 1) {
                (void)dbg_printf(" %3d", cid);
            }
            (void)dbg_printf("\n");
        }
    }


#if VTSS_SWITCH_STACKABLE
    // Print relay statistics
    (void)dbg_printf("\n");
    (void)dbg_printf("Relay counters Port %u    Port %u\n", PORT_NO_STACK_0, PORT_NO_STACK_1);
    (void)dbg_printf("-------------- ---------- ----------\n");
    (void)dbg_printf("Rx             %10u %10u\n", state.relay_stat[0].rx,           state.relay_stat[1].rx);
    (void)dbg_printf("Tx OK          %10u %10u\n", state.relay_stat[0].tx_ok,        state.relay_stat[1].tx_ok);
    (void)dbg_printf("HTL Drops      %10u %10u\n", state.relay_stat[0].htl_err,      state.relay_stat[1].htl_err);
    (void)dbg_printf("Inv Port Drps  %10u %10u\n", state.relay_stat[0].inv_port_err, state.relay_stat[1].inv_port_err);
    (void)dbg_printf("Tx Errors      %10u %10u\n", state.relay_stat[0].tx_err,       state.relay_stat[1].tx_err);
#endif /* VTSS_SWITCH_STACKABLE */

    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_stat_last_rx_cb_print()
// cmd_text   : "Print last callback to Msg Rx"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_last_rx_cb_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    // Typically when this info is needed it's because something is wrong, so
    // we don't take any semaphores before printing this out (the right one to
    // take if we needed one is the MSG_CFG_CRIT_ENTER()).
    (void)dbg_printf("Latest call to Msg Rx callback: mod=%d=%s, len=%u, connid=%u\n", dbg_latest_rx_modid, vtss_module_names[dbg_latest_rx_modid], dbg_latest_rx_len, dbg_latest_rx_connid);
}

/****************************************************************************/
// DBG_cmd_stat_last_im_cb_print()
// cmd_text   : "Print last callback to init_modules()"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_last_im_cb_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_init_data_t latest_data;
    char             *latest_init_module_func_name;
    control_dbg_latest_init_modules_get(&latest_data, &latest_init_module_func_name);
    (void)dbg_printf("Latest call to init_modules(): cmd=%s, isid=%u, flags=0x%x, init-func=%s\n", control_init_cmd2str(latest_data.cmd), latest_data.isid, latest_data.flags, latest_init_module_func_name);
}

/****************************************************************************/
// DBG_cmd_stat_im_max_cb_time_print()
// cmd_text   : "Print longest init_modules() callback time"
// arg_syntax : "[clear]"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_stat_im_max_cb_time_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    extern void control_dbg_init_modules_callback_time_max_print(msg_dbg_printf_t dbg_printf, BOOL clear);
    control_dbg_init_modules_callback_time_max_print(dbg_printf, parms_cnt == 1 ? TRUE : FALSE);
}

/****************************************************************************/
// DBG_cmd_stat_pool_print()
// cmd_text   : "Print message pool statistics"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_pool_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    msg_buf_pool_t *pool_iter;
    MSG_BUF_CRIT_ENTER();
    pool_iter = MSG_buf_pool;

    (void)dbg_printf("Module              Description      Buf size   Max Bufs   Min Bufs   Cur Bufs   Allocs\n");
    (void)dbg_printf("------------------- ---------------- ---------- ---------- ---------- ---------- ----------\n");
    while (pool_iter) {
        (void)dbg_printf("%-19s %-16s %10u %10u %10u %10u %10u\n", vtss_module_names[pool_iter->module_id], pool_iter->dscr, pool_iter->buf_size, pool_iter->buf_cnt_init, pool_iter->buf_cnt_min, pool_iter->buf_cnt_cur, pool_iter->allocs);
        pool_iter = pool_iter->next;
    }
    MSG_BUF_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_switch_info_print()
// cmd_text   : "Print switch info"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_switch_info_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_isid_t             isid;
    msg_flash_switch_info_t local_info;
    BOOL                    local_really_configurable[VTSS_ISID_END];
    BOOL                    is_slave;

    MSG_STATE_CRIT_ENTER();
    local_info = CX_switch_info;
    is_slave = state.state != MSG_MOD_STATE_MST;
    memcpy(local_really_configurable, CX_switch_really_configurable, sizeof(local_really_configurable));
    MSG_STATE_CRIT_EXIT();

    (void)dbg_printf("ISID Configurable Exists Port Count Stack Port 0 Stack Port 1 Board Type API Instance\n");
    (void)dbg_printf("---- ------------ ------ ---------- ------------ ------------ ---------- ------------\n");
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        init_switch_info_t *info = &local_info.info[isid];
        if (local_really_configurable[isid]) {
            (void)dbg_printf("%4u %-12s %-6s %10u %12u %12u %10u %12x\n",
                             isid,
                             info->configurable ? "Yes" : "(Yes)", // Print in parentheses for slaves in standalone mode
                             is_slave ? "N/A" : msg_switch_exists(isid) ? "Yes" : "No",
                             info->port_cnt,
                             iport2uport(info->stack_ports[0]),
                             iport2uport(info->stack_ports[1]),
                             info->board_type,
                             info->api_inst_id);
        } else {
            (void)dbg_printf("%4u No\n", isid);
        }
    }
}

/****************************************************************************/
// DBG_cmd_switch_info_reload()
// cmd_text   : "Reload switch info from flash (has an effect on slaves only)"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_switch_info_reload(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    MSG_STATE_CRIT_ENTER();
    CX_flash_read(VTSS_ISID_GLOBAL); // By calling with an invalid ISID, the flash will not be updated in case it contains invalid info.
    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_cfg_flash_clear()
// cmd_text   : Erase message module's knowledge about other switches. Takes effect upon next boot.
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_cfg_flash_clear(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    (void)conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MSG, 0);
}

/****************************************************************************/
// DBG_cmd_stat_shaper_print()
// cmd_text   : "Print shaper status"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_shaper_print(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    u32 cnt = 0;
    vtss_module_id_t modid;
    MSG_STATE_CRIT_ENTER();
    for (modid = 0; modid < VTSS_MODULE_ID_NONE; modid++) {
        if (msg_shaper[modid].drops) {
            if (cnt++ == 0) {
                (void)dbg_printf("Module              Current    Limit      Drops\n");
                (void)dbg_printf("------------------- ---------- ---------- ----------\n");
            }
            (void)dbg_printf("%-19s %10u %10u %10u\n", vtss_module_names[modid], msg_shaper[modid].current, msg_shaper[modid].limit, msg_shaper[modid].drops);
        }
    }
    MSG_STATE_CRIT_EXIT();

    if (cnt == 0) {
        (void)dbg_printf("No modules have been shaped so far.\n");
    }
}

/****************************************************************************/
// DBG_cmd_stat_usr_msg_clear()
// cmd_text   : "Clear User Message Statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0,
/****************************************************************************/
static void DBG_cmd_stat_usr_msg_clear(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    MSG_COUNTERS_CRIT_ENTER();
    memset(&state.usr_stat[0], 0, sizeof(state.usr_stat));
    MSG_COUNTERS_CRIT_EXIT();

    (void)dbg_printf("User Message statistics cleared!\n");
}

/****************************************************************************/
// DBG_cmd_stat_protocol_clear()
// cmd_text   : "Clear Protocol Statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0,
/****************************************************************************/
static void DBG_cmd_stat_protocol_clear(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_isid_t isid;
    int cid;

    // Reset statistics

    MSG_STATE_CRIT_ENTER();

    // Connection statistics
    for (isid = 0; isid < VTSS_ISID_CNT + 1; isid++) {
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            memset(&mcbs[isid][cid].stat, 0, sizeof(mcbs[isid][cid].stat));
        }
    }

    // Master/slave statistics
    memset(&state.pdu_stat[0], 0, sizeof(state.pdu_stat));

    // Relay statistics
    memset(&state.relay_stat[0], 0, sizeof(state.relay_stat));

    MSG_STATE_CRIT_EXIT();

    (void)dbg_printf("Message protocol statistics cleared!\n");
}

/****************************************************************************/
// DBG_cmd_cfg_trace_isid_set()
// cmd_text   : "Configure trace output - only used when master"
// arg_syntax : "[<isid> [<enable> (0 or 1)]] - Use <isid> = 0 to enable or disable all"
// max_arg_cnt: 2
/****************************************************************************/
static void DBG_cmd_cfg_trace_isid_set(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_isid_t isid;
    if (parms_cnt == 0) {
        (void)dbg_printf("ISID Trace Enabled\n");
        (void)dbg_printf("---- -------------\n");
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            (void)dbg_printf("%4d %d\n", isid, msg_trace_enabled_per_isid[isid]);
        }
        return;
    }

    if (parms_cnt != 2) {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 0 or 2 arguments, not %d", parms_cnt);
        return;
    }

    if (parms[0] >= VTSS_ISID_END) {
        DBG_cmd_syntax_error(dbg_printf, "ISIDs must reside in interval [%d; %d] or be 0 (meaning change all)", VTSS_ISID_START, VTSS_ISID_END - 1);
        return;
    }

    if (parms[0] == 0) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            msg_trace_enabled_per_isid[isid] = parms[1];
        }
    } else {
        msg_trace_enabled_per_isid[parms[0]] = parms[1];
    }
}

/****************************************************************************/
// DBG_cmd_cfg_trace_modid_set()
// cmd_text   : "Configure trace output per module"
// arg_syntax : "[<module_id> [<enable> (0 or 1)]] - Use <module_id> = -1 to enable or disable all"
// max_arg_cnt: 2
/****************************************************************************/
static void DBG_cmd_cfg_trace_modid_set(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    vtss_module_id_t modid;
    if (parms_cnt == 0) {
        (void)dbg_printf("Module              ID  Trace Enabled\n");
        (void)dbg_printf("------------------- --- -------------\n");
        for (modid = 0; modid < VTSS_MODULE_ID_NONE; modid++) {
            (void)dbg_printf("%-19s %3d %d\n", vtss_module_names[modid], modid, msg_trace_enabled_per_modid[modid]);
        }
        return;
    }

    if (parms_cnt != 2) {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 0 or 2 arguments, not %d", parms_cnt);
        return;
    }

    if (parms[0] != (u32)(-1) && parms[0] >= VTSS_MODULE_ID_NONE) {
        DBG_cmd_syntax_error(dbg_printf, "Module IDs must reside in interval [0; %d] or be -1 (meaning change all)", VTSS_MODULE_ID_NONE - 1);
        return;
    }

    if (parms[0] == (u32)(-1)) {
        for (modid = 0; modid < VTSS_MODULE_ID_NONE; modid++) {
            msg_trace_enabled_per_modid[modid] = parms[1];
        }
    } else {
        msg_trace_enabled_per_modid[parms[0]] = parms[1];
    }
}

/****************************************************************************/
// DBG_cmd_cfg_timeout()
// cmd_text   : "Configure timeouts"
// arg_syntax : "[<MSYN timeout> [<MD timeout>]] - all in milliseconds (0 = no change)"
// max_arg_cnt: 2
/****************************************************************************/
static void DBG_cmd_cfg_timeout(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int i;

    MSG_STATE_CRIT_ENTER();

    if (parms_cnt == 0) {
        (void)dbg_printf("PDU Type Timeout [ms]\n");
        (void)dbg_printf("-------- ------------\n");
        (void)dbg_printf("MSYN     %5u\n", state.msg_cfg_msyn_timeout_ms);
        (void)dbg_printf("MD       %5u\n", state.msg_cfg_md_timeout_ms);
        goto do_exit;
    }

    if (parms_cnt != 2) {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 0 or 2 arguments, not %d", parms_cnt);
        goto do_exit;
    }

    for (i = 0; i < 2; i++) {
        if (parms[i] != 0  && (parms[i] < MSG_CFG_MIN_TIMEOUT_MS || parms[i] > MSG_CFG_MAX_TIMEOUT_MS)) {
            DBG_cmd_syntax_error(dbg_printf, "Timeouts must reside in interval [%d; %d] or be 0 (meaning no change)", MSG_CFG_MIN_TIMEOUT_MS, MSG_CFG_MAX_TIMEOUT_MS);
            goto do_exit;
        }
    }

    if (parms[0] != 0) {
        state.msg_cfg_msyn_timeout_ms = parms[0];
    }
    if (parms[2] != 0) {
        state.msg_cfg_md_timeout_ms = parms[2];
    }
    (void)dbg_printf("Changes take effect immediately!\n");

do_exit:
    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_cfg_retransmit()
// cmd_text   : "Configure retransmits"
// arg_syntax : "[<MD retransmits>]"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_cfg_retransmit(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    MSG_STATE_CRIT_ENTER();

    if (parms_cnt == 0) {
        (void)dbg_printf("PDU Type Retransmits\n");
        (void)dbg_printf("-------- -----------\n");
        (void)dbg_printf("MD       %11u\n", state.msg_cfg_md_retransmit_limit);
        goto do_exit;
    }

    if (parms[0] > MSG_CFG_MAX_RETRANSMIT_LIMIT) {
        DBG_cmd_syntax_error(dbg_printf, "Retransmit limit must reside in interval [0; %d]", MSG_CFG_MAX_RETRANSMIT_LIMIT);
        goto do_exit;
    }

    state.msg_cfg_md_retransmit_limit = parms[0];
    (void)dbg_printf("Changes take effect on next connection negotiation!\n");

do_exit:
    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_cfg_winsz()
// cmd_text   : "Configure window sizes"
// arg_syntax : "[[<master per slave> [<slave>]] (0 = no change)",
// max_arg_cnt: 2
/****************************************************************************/
static void DBG_cmd_cfg_winsz(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int i;

    MSG_STATE_CRIT_ENTER();

    if (parms_cnt == 0) {
        (void)dbg_printf("MWINSZ: %3u\n", state.msg_cfg_mst_winsz_per_slv);
        (void)dbg_printf("SWINSZ: %3u\n", state.msg_cfg_slv_winsz);
        goto do_exit;
    }

    if (parms_cnt != 2) {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 0 or 2 arguments, not %d", parms_cnt);
        goto do_exit;
    }

    for (i = 0; i < 2; i++) {
        if (parms[i] != 0  && (parms[i] < MSG_CFG_MIN_WINSZ || parms[i] > MSG_CFG_MAX_WINSZ)) {
            DBG_cmd_syntax_error(dbg_printf, "Window sizes must reside in interval [%d; %d] or be 0 (meaning no change)", MSG_CFG_MIN_WINSZ, MSG_CFG_MAX_WINSZ);
            goto do_exit;
        }
    }

    if (parms[0] != 0) {
        state.msg_cfg_mst_winsz_per_slv = parms[0];
    }
    if (parms[1] != 0) {
        state.msg_cfg_slv_winsz = parms[1];
    }
    (void)dbg_printf("Changes take effect on next connection negotiation.!\n");

do_exit:
    MSG_STATE_CRIT_EXIT();
}

#if MSG_HOP_BY_HOP
/****************************************************************************/
// DBG_cmd_cfg_htl()
// cmd_text   : "Configure hops-to-live (1 = only to neighboring switch)"
// arg_syntax : "[<HTL>]"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_cfg_htl(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    MSG_STATE_CRIT_ENTER();

    if (parms_cnt == 0) {
        (void)dbg_printf("HTL = %u\n", state.msg_cfg_htl_limit);
        goto do_exit;
    }

    if (parms[0] < MSG_CFG_MIN_HTL_LIMIT || parms[0] > MSG_CFG_MAX_HTL_LIMIT) {
        DBG_cmd_syntax_error(dbg_printf, "HTL limits must reside in interval [%d; %d]", MSG_CFG_MIN_HTL_LIMIT, MSG_CFG_MAX_HTL_LIMIT);
        goto do_exit;
    }

    state.msg_cfg_htl_limit = parms[0];
    (void)dbg_printf("Changes take effect immediately!\n");

do_exit:
    MSG_STATE_CRIT_EXIT();
}
#endif

/****************************************************************************/
// DBG_cmd_cfg_test_run()
// cmd_text   : "Run a test case. Omit parameters to see which TC are available. Some TCs take arguments"
// arg_syntax : "[<TC> [<TC arg>]]"
// max_arg_cnt: 3
/****************************************************************************/
static void DBG_cmd_test_run(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    u32 arg0, arg1, arg2;

    arg0 = (parms_cnt >= 2) ? parms[1] : 0;
    arg1 = (parms_cnt >= 3) ? parms[2] : 0;
    arg2 = (parms_cnt >= 4) ? parms[3] : 0;

    msg_test_suite(parms[0], arg0, arg1, arg2, dbg_printf);
}

/****************************************************************************/
// DBG_cmd_test_renegotiate()
// cmd_text   : "Re-negotiate a connection towards a slave (master only)"
// arg_syntax : "[<ISID>] - leave argument out to re-negotiate all"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_test_renegotiate(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int min_idx, max_idx, idx, cid;

    if (parms_cnt == 0) {
        min_idx = VTSS_ISID_START;
        max_idx = VTSS_ISID_END - 1;
    } else {
        if (parms[0] < VTSS_ISID_START || parms[0] >= VTSS_ISID_END) {
            DBG_cmd_syntax_error(dbg_printf, "ISIDs must reside in interval [%d; %d]", VTSS_ISID_START, VTSS_ISID_END - 1);
            return;
        }
        min_idx = max_idx = parms[0];
    }

    MSG_STATE_CRIT_ENTER();

    if (state.state != MSG_MOD_STATE_MST) {
        (void)dbg_printf("Error: This command is only available on the master\n");
        MSG_STATE_CRIT_EXIT();
        return;
    }

    for (idx = min_idx; idx <= max_idx; idx++) {
        if (idx != state.misid) { // We cannot re-negotiate the loopback connection
            for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                if (mcbs[idx][cid].state == MSG_CONN_STATE_MST_ESTABLISHED) {
                    // The slave return code is not used, so it could be anything.
                    CX_restart(&mcbs[idx][cid], idx, MSG_TX_RC_ERR_MST_DBG_RENEGOTIATION, MSG_TX_RC_ERR_MST_DBG_RENEGOTIATION);
                }
            }
        }
    }

    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
/*                                                                          */
/*  MODULE EXTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/******************************************************************************/
// msg_topo_event()
// Called back by the topology module when a change in the stack occurs.
// Do not make this static, as it is called from msg_test module.
/******************************************************************************/
void msg_topo_event(msg_topo_event_t topo_event, vtss_isid_t new_isid)
{
    int                cid;
    vtss_isid_t        isid;
    mac_addr_t         mac;
    vtss_rc            rc;
    vtss_init_data_t   event;
#if VTSS_SWITCH_STACKABLE
    vtss_vstax_upsid_t upsid;
#endif
    CX_event_init(&event);

    switch (topo_event) {
    case MSG_TOPO_EVENT_CONF_DEF:
        // The easiest way for Topo to serialize switch delete and restore configuration
        // defaults is to let it go through the Message Module.
        // Once this event occurs, the user has explicitly deleted the switch through management.
        T_IG(TRACE_GRP_TOPO, "Got Conf Default Event, ISID = %d", new_isid);
        MSG_ASSERT(new_isid >= VTSS_ISID_START && new_isid < VTSS_ISID_END, "Invalid new_isid: %d", (int)new_isid);

        MSG_STATE_CRIT_ENTER();

        // Cannot receive this event unless we're master.
        VTSS_ASSERT(state.state == MSG_MOD_STATE_MST);

        // The switch pointed to by new_isid must not currently be in the stack.
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            VTSS_ASSERT(mcbs[new_isid][cid].state == MSG_CONN_STATE_MST_NO_SLV);
        }

        // The switch no longer exists and is not configurable anymore.
        CX_switch_info.info[new_isid].configurable = FALSE;
        CX_switch_really_configurable[new_isid]    = FALSE;
        CX_flash_write();

        // Generate a CONF-DEF event.
        (void)CX_event_conf_def(new_isid, &CX_switch_info.info[new_isid]);

        MSG_STATE_CRIT_EXIT();
        break;

    case MSG_TOPO_EVENT_MASTER_UP:
        T_IG(TRACE_GRP_TOPO, "Got Master Up Event, ISID = %d", new_isid);
        MSG_ASSERT(new_isid >= VTSS_ISID_START && new_isid < VTSS_ISID_END, "Invalid new_isid: %d", (int)new_isid);

        MSG_STATE_CRIT_ENTER();

        // Cannot receive this while we're already master.
        VTSS_ASSERT(state.state != MSG_MOD_STATE_MST);

        // Release all pending User Messages on the slave interface.
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            CX_mcb_flush(&mcbs[MSG_SLV_ISID_IDX][cid], MSG_TX_RC_WARN_SLV_MASTER_UP);
        }

        // Return the slave MCBs to their initial state.
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            mcbs[MSG_SLV_ISID_IDX][cid].state = MSG_CONN_STATE_SLV_NO_MST;
        }

        // Move the master MCBs to the master idle state.
        for (isid = MSG_MST_ISID_START_IDX; isid <= MSG_MST_ISID_END_IDX; isid++) {
            for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                CX_set_state_mst_no_slv(&mcbs[isid][cid]);
            }
        }

        state.state = MSG_MOD_STATE_MST;
        state.misid = new_isid;

        event.cmd  = INIT_CMD_MASTER_UP;
        event.isid = 0;

        // Load board IDs and stack ports from flash, so that module configuration can
        // be applied correctly from ICFG once the master-up event has been through the port module.
        CX_flash_read(new_isid);
        memcpy(event.switch_info, &CX_switch_info.info[0], sizeof(event.switch_info));

        // Notify user modules of master up.
        (void)IM_fifo_put(&event);

        MSG_STATE_CRIT_EXIT();
        break;

    case MSG_TOPO_EVENT_MASTER_DOWN:
        T_IG(TRACE_GRP_TOPO, "Got Master Down Event");

        MSG_STATE_CRIT_ENTER();

        // Can't get master down unless we're already master
        VTSS_ASSERT(state.state == MSG_MOD_STATE_MST);

        // Release all connections towards active slaves
        for (isid = MSG_MST_ISID_START_IDX; isid <= MSG_MST_ISID_END_IDX; isid++) {
            // To ensure proper shut-down, call init_modules() telling them that a slave
            // is on its way out. The init_modules() will also be called for the master itself.
            if (mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
                event.cmd  = INIT_CMD_SWITCH_DEL;
                event.isid = isid;
                (void)IM_fifo_put(&event);
            }
            for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                CX_mcb_flush(&mcbs[isid][cid], MSG_TX_RC_WARN_MST_MASTER_DOWN);
                CX_set_state_mst_no_slv(&mcbs[isid][cid]);
            }
        }

        // Move the slave MCBs to the ready state.
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            mcbs[MSG_SLV_ISID_IDX][cid].state = MSG_CONN_STATE_SLV_NO_MST;
        }

        // We ignore MSYNs until the master-down event has been passed to all user-modules.
        CX_user_state.ignore_msyns = TRUE;

        state.state = MSG_MOD_STATE_SLV;

        event.cmd  = INIT_CMD_MASTER_DOWN;
        event.isid = 0;

        // Also send the master down event
        (void)IM_fifo_put(&event);

        MSG_STATE_CRIT_EXIT();
        break;

    case MSG_TOPO_EVENT_SWITCH_ADD:
        T_IG(TRACE_GRP_TOPO, "Got Switch Add Event, ISID = %d", new_isid);

        MSG_ASSERT(new_isid >= VTSS_ISID_START && new_isid < VTSS_ISID_END, "Invalid new_isid: %d", (int)new_isid);
        MSG_STATE_CRIT_ENTER();

        // We must be master to add switches.
        VTSS_ASSERT(state.state == MSG_MOD_STATE_MST);

        // Check that no other connections use the same MAC address.
        rc = topo_isid2mac(new_isid, mac);
        if (rc < 0) {
            T_W("topo_isid2mac() failed (rc=%d). Topology change on its way?", rc);
            // We need to add the switch anyway, because Topo may give us a switch
            // delete in just a second.
        } else {
            // See if that MAC address is already in use in one of our connections.
            isid = RX_mac2isid(mac);
            MSG_ASSERT(isid == 0, "Switch Add (ISID=%d): A connection with that slave MAC address is already in use on ISID=%d", new_isid, isid);

            // If the new switch is the master, check that topo thinks the MAC address
            // is the same as what we think we have.
            if (new_isid == state.misid) {
                if (memcmp(mac, this_mac, sizeof(mac_addr_t)) != 0) {
                    T_E("Topo and Conf use different local MAC addresses");
                }
            }
        }

#if VTSS_SWITCH_STACKABLE
        upsid = topo_isid_port2upsid(vtss_stacking_enabled() ? new_isid : VTSS_ISID_LOCAL, VTSS_PORT_NO_NONE);
#if !MSG_HOP_BY_HOP
        // If not in hop-by-hop mode, the UPSID must be legal.
        if (!VTSS_VSTAX_UPSID_LEGAL(upsid)) {
            T_E("topo_isid_port2upsid() returned illegal upsid (%d) for isid = %u", upsid, new_isid);
        }
#endif
#endif

        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            // It's illegal to add an already added switch.
            VTSS_ASSERT(mcbs[new_isid][cid].state == MSG_CONN_STATE_MST_NO_SLV);

#if VTSS_SWITCH_STACKABLE
            CX_update_upsid(&mcbs[new_isid][cid], upsid);
#endif

            // If we're adding ourselves (the master), go directly to the established state.
            if (new_isid == state.misid) {
                CX_local_switch_info_get(&mcbs[new_isid][cid].u.master.switch_info);
                mcbs[new_isid][cid].estab_time = time(NULL);
                mcbs[new_isid][cid].state = MSG_CONN_STATE_MST_ESTABLISHED;
            } else {
                mcbs[new_isid][cid].state = MSG_CONN_STATE_MST_RDY;
                // Trigger transmission of an MSYN to the slave right away.
                TX_wake_up_msg_thread(MSG_FLAG_TX_MSYN);
            }

            // Reset the statistics
            memset(&mcbs[new_isid][cid].stat, 0, sizeof(mcbs[new_isid][cid].stat));
        }

        // If this is the master ISID, tell switches right away that there's a slave (ourselves),
        // otherwise this will be called after MSYN/MSYNACK negotiation.
        if (new_isid == state.misid) {
            if (CX_switch_info.info[new_isid].configurable == FALSE) {
                // Seems that something went wrong during read from flash.
                T_E("Something fundamental wrong with flash load");
                CX_init_switch_info_get(&CX_switch_info.info[new_isid]);
            }

            if (!CX_event_switch_add(new_isid, &CX_switch_info.info[new_isid], mcbs[new_isid][0].u.master.switch_info.port_map)) {
                T_E("Something fundamental wrong with the IM FIFO");
            }
        }

        MSG_STATE_CRIT_EXIT();
        break;

    case MSG_TOPO_EVENT_SWITCH_DEL:
        T_IG(TRACE_GRP_TOPO, "Got Switch Delete Event, ISID = %d", new_isid);

        MSG_ASSERT(new_isid >= VTSS_ISID_START && new_isid < VTSS_ISID_END, "Invalid isid: %d", (int)new_isid);

        MSG_STATE_CRIT_ENTER();

        // We must be master!
        VTSS_ASSERT(state.state == MSG_MOD_STATE_MST);

        // We cannot remove the master from the list of active switches.
        // If we need to support this, some changes must be done to the msg_tx() function.
        VTSS_ASSERT(new_isid != state.misid);

        // Release all pending User Messages sent to the slave.
        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            CX_mcb_flush(&mcbs[new_isid][cid], MSG_TX_RC_WARN_MST_SLAVE_DOWN);
        }

        // Notify User Modules if the switch.
        if (mcbs[new_isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
            event.cmd  = INIT_CMD_SWITCH_DEL;
            event.isid = new_isid;
            (void)IM_fifo_put(&event);
        }

        for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
            // The slave switch state from the master's point of view must be 'existing',
            // i.e. previously added.
            VTSS_ASSERT(mcbs[new_isid][cid].state != MSG_CONN_STATE_MST_NO_SLV);
            CX_set_state_mst_no_slv(&mcbs[new_isid][cid]); // No longer present
        }

        MSG_STATE_CRIT_EXIT();
        break;

    default:
        MSG_ASSERT(FALSE, "Unsupported topology change (%d)", topo_event);
        break;
    }
}

/******************************************************************************/
// msg_tx_adv()
/******************************************************************************/
void msg_tx_adv(const void *const contxt, const msg_tx_cb_t cb, msg_tx_opt_t opt, vtss_module_id_t dmodid, u32 did, const void *const msg, size_t len)
{
    msg_tx_rc_t rc = MSG_TX_RC_OK;
    int         cid = 0, i;
    msg_item_t  *usr_msg_item, *tx_done_usr_msg_item;
    msg_mcb_t   *mcb = NULL;
    void        *copy_of_msg;
    u32         did_orig = did;

    MSG_ASSERT(len > 0 && len <= MSG_MAX_LEN_BYTES, "Invalid len = %zu. dmodid = %s", len, vtss_module_names[dmodid]);
    MSG_ASSERT(dmodid < VTSS_MODULE_ID_NONE, "No such module (id = %d)", dmodid);

    MSG_STATE_CRIT_ENTER();
    if (CX_user_state.master) {
        // The user modules are supposed to think that we are master.
        // Check to see if we indeed are:
        if (state.state != MSG_MOD_STATE_MST) {
            // No we aren't. We must have gotten a master down in the meanwhile.
            rc = MSG_TX_RC_WARN_MST_MASTER_DOWN;
            goto exit_func;
        }

        // In master mode, the @did is supposed to be an ISID, but if the User Module
        // thinks we are (still) a slave, it may be out of bounds.
        if (did > MSG_MST_ISID_END_IDX) {
            rc = MSG_TX_RC_WARN_MST_INV_DISID;
            goto exit_func;
        }

        // If the @did is 0, the message must be looped back.
        if (did == 0) {
            did = state.misid;
        }

        // Locate the MCB.
        if (opt & MSG_TX_OPT_PRIO_HIGH) {
            // Currently this code supports at most 2 connections per slave.
            // The only place to change to support more connections is in the
            // API call to msg_tx_adv(), where the @opt member could take more
            // priorities - and here.
            cid = MSG_CFG_CONN_CNT - 1;
        } else {
            cid = 0;
        }

        // If the user module haven't yet heard that the slave exists,
        // we should not send any messages to it, unless the caller
        // just want to send to current master (#did_orig == 0).
        if (did_orig != 0 && !CX_user_state.exists[did]) {
            rc = MSG_TX_RC_WARN_MST_NO_SLV;
            goto exit_func;
        }

        // Locate the highest indexed MCB with connection index <= cid, that
        // is in the established state.
        mcb = NULL;
        for (i = cid; i >= 0; i--) {
            if (mcbs[did][i].state == MSG_CONN_STATE_MST_ESTABLISHED) {
                mcb = &mcbs[did][i];
                break;
            }
        }

        if (mcb == NULL) {
            // No established connections found. This could be because
            // the slave just left the stack, without the User Module
            // has yet been notified about it. Flag it as a warning.
            rc = MSG_TX_RC_WARN_MST_NO_SLV;
            goto exit_func;
        }
    } else {

        // The user modules are supposed to think that we are slave.
        // Check to see if we indeed are:
        if (state.state != MSG_MOD_STATE_SLV) {
            // No we aren't. We must have gotten a master up event in the meanwhile.
            rc = MSG_TX_RC_WARN_SLV_MASTER_UP;
        }

        // In slave mode, the @did is supposed to be a connid. connids and ISIDs
        // are disjunct. If we just changed state from master to slave, but the
        // user module isn't aware of this yet, he might attempt to transmit to
        // a certain slave. Therefore we can only flag this as a warning.
        if (did >= MSG_MST_ISID_START_IDX && did <= MSG_MST_ISID_END_IDX) {
            rc = MSG_TX_RC_WARN_SLV_INV_DCONNID;
            goto exit_func;
        }

        // For slaves the MSG_TX_OPT_PRIO_HIGH is ignored, because solicited
        // messages are transmitted on a certain CID, which intrinsically is
        // a priority, and unsolicited messages need not be sent with high
        // prio.
        // When running tests in slave mode, the msg_test module attempts
        // to set this flag, so we cannot assert it's not being used.

        // If @did is 0, use the lowest numbered connection
        if (did == 0) {
            mcb = &mcbs[MSG_SLV_ISID_IDX][0];
        } else {
            // Locate the connection with the given connid.
            mcb = NULL;
            for (i = 0; i < MSG_CFG_CONN_CNT; i++) {
                if (mcbs[MSG_SLV_ISID_IDX][i].connid == did) {
                    mcb = &mcbs[MSG_SLV_ISID_IDX][i];
                    break;
                }
            }
        }

        if (mcb == NULL || mcb->state != MSG_CONN_STATE_SLV_ESTABLISHED) {
            // No established connections found. This could be because
            // of a master up event, or change of master, or simply
            // because of the user module sending an unsolicited
            // message and there's yet no connection established with the
            // master. The return value only flags a warning, since this
            // is "normal" behavior of a changing stack.
            rc = MSG_TX_RC_WARN_SLV_NO_MST;
            goto exit_func;
        }
    }

exit_func:

    // Allocate a structure for holding the properties
    usr_msg_item = VTSS_MALLOC(sizeof(msg_item_t));
    // It's impossible to pass an out-of-memory on to the message thread,
    // since we need the usr_msg_item to be able to do that.
    VTSS_ASSERT(usr_msg_item);

    // Shape the message if subject to shaping.
    if (opt & MSG_TX_OPT_SHAPE) {
        // Always count, since we always decrement in tx done if #opt contains MSG_TX_OPT_SHAPE
        msg_shaper[dmodid].current++;

        // But only report it as a shaper-drop if nothing else is wrong.
        if (rc == MSG_TX_RC_OK && msg_shaper[dmodid].limit && msg_shaper[dmodid].current > msg_shaper[dmodid].limit) {
            msg_shaper[dmodid].drops++;
            rc = MSG_TX_RC_WARN_SHAPED;
        }
    }

    // If loopback messages were dispatched to the destination module from
    // this function, we might end up in a deadlock if both the module's
    // tx and rx function acquires the same mutex. Therefore, we send
    // loopback messages the same way as other messages, i.e. through
    // the RX_thread.
    // Likewise, if warnings or errors were dispatched back to the user-
    // defined callback from this function, we could end-up in a deadlock
    // if both the module's tx and tx_done function acquires the same mutex.
    // Even if the callback is NULL, and an error occurred, we will pass
    // the call to the TX_thread, because it also increases the statistics,
    // and possibly deletes the memory used by the user message.

    // Fill it
    usr_msg_item->is_tx_msg    = TRUE;
    usr_msg_item->usr_msg      = (u8 *)msg;
    usr_msg_item->len          = len;
    usr_msg_item->dmodid       = dmodid;
    usr_msg_item->connid       = did; // Only used in loopback case (i.e. when we're master).
    usr_msg_item->state        = state.state; // To be able to count in correct statistics bin.
    usr_msg_item->u.tx.opt     = opt;
    usr_msg_item->u.tx.cb      = cb;
    usr_msg_item->u.tx.contxt  = (void *)contxt;
    usr_msg_item->u.tx.rc      = rc;
    usr_msg_item->u.tx.md_list = NULL;
    usr_msg_item->next         = NULL;

    // In case of an error, we put the message on the pend_tx_done_list, which
    // is absorbed by the TX_thread(). It will eventually call-back any
    // user-defined callback function from outside the user's own thread.
    if (rc != MSG_TX_RC_OK) {
        // Error! Whether it's a loopback message or not, simply send it to the
        // Tx Done list. It never ends up in the RX list.
        TX_put_done_list(NULL, usr_msg_item, usr_msg_item);
    } else {
        // If this is not a loop-back message, create the MDs needed for it, so that they're
        // ready to be transmitted in the TX_thread().
        if (state.state != MSG_MOD_STATE_MST || did != state.misid) {
            /* This is not a loop-back message */
#if VTSS_SWITCH_STACKABLE
            VTSS_ASSERT(mcb != NULL); // Keep lint happy
            TX_md_create(usr_msg_item, mcb->dmac, cid, &mcb->next_available_sseq, did);
            if (MSG_TRACE_ENABLED(did, dmodid)) {
                T_IG(TRACE_GRP_TX, "Tx(Msg): len=%u, dmodid=%s, connid=%u, dmac=%s, seq=[%u, %u]", usr_msg_item->len, vtss_module_names[usr_msg_item->dmodid], usr_msg_item->connid, CX_mac2str(&usr_msg_item->u.tx.md_list->hdr[VTSS_FDMA_HDR_SIZE_BYTES + MSG_DMAC_OFFSET]), usr_msg_item->u.tx.md_list->seq, mcb->next_available_sseq - 1);
            }
#else
            if (MSG_TRACE_ENABLED(did, dmodid)) {
                T_IG(TRACE_GRP_TX, "Non-loopback message in a non-stackable system!!!!!");
            }
#endif
        } else {
            // It's a loopback message. We need to consider the MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK
            // option.
            if (opt & MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK) {
                // Do not allocate a new user message. Instead the callbacks must be done
                // as follows: RX, TxDone, then perhaps free original user message.
                // This option will be satisfied by simply transferring the message
                // directly to the mcb's tx_msg_list, so that it is processed in order.
                // The TX_handle_tx() will in turn move it to the rx_msg_list, which
                // in turn will call RX followed by TxDone callbacks. These two callbacks
                // will occur from the same thread, namely the RX_thread.
            } else {
                // Allocate a new user message and make a copy of @msg into it.
                // The order of events is:
                //   Concurrently call TxDone(@msg) and Rx callbacks(copy_of_msg).
                //   After TxDone(@msg), free @msg if told to.
                //   After Rx(copy_of_msg), free copy_of_msg.
                // The TxDone will occur from the TX_thread and the RX will occur
                // from the RX_thread.
                // Due to this possible TxDone before Rx behavior, it might happen that
                // a topology change (MASTER_DOWN) causes the RX never to occur even though
                // the TxDone() return code said OK.

                // The currently filled usr_msg_item is actually the one we're going to send
                // to the TxDone callback.
                tx_done_usr_msg_item = usr_msg_item;

                // We need to create a new one containing the one to send to the Rx callback.
                usr_msg_item = VTSS_MALLOC(sizeof(msg_item_t));
                VTSS_ASSERT(usr_msg_item);

                // Allocate new msg and copy the original @msg to that one.
                copy_of_msg = VTSS_MALLOC(len);
                VTSS_ASSERT(copy_of_msg);
                /* Suppress Lint Warning 670: Possible access beyond array for function 'memcpy(void *, const void *, unsigned int)', argument 3 (size=16777215) exceeds argument 2 (size=360464) */
                /*lint -e{670} */
                memcpy(copy_of_msg, msg, len);

                // Fill the new usr_msg_item with the copy.
                // Disguise it as an Rx message, so that it doesn't get counted twice as TxDone.
                // The only "problem" with that is that the RX_msg_dispatch() function prints a different
                // debug trace for loopback messages than for non-loopback messages.
                usr_msg_item->is_tx_msg  = FALSE; // Disguise it as an Rx message, so that it doesn't get counted twice as TxDone.
                usr_msg_item->usr_msg    = (u8 *)copy_of_msg;
                usr_msg_item->len        = len;
                usr_msg_item->dmodid     = dmodid;
                usr_msg_item->connid     = did;         // Only used in loopback case (i.e. when we're master).
                usr_msg_item->state      = state.state; // To be able to count in correct statistics bin.
                usr_msg_item->u.rx.left_seq = usr_msg_item->u.rx.right_seq = usr_msg_item->u.rx.frags_received = 0; // Only used in trace output.
                usr_msg_item->next       = NULL;

                // Now that we've filled in the copy, we can safely store the tx_done_usr_msg_item in the tx_done list...
                // This also causes a count in "Tx OK"
                TX_put_done_list(mcb, tx_done_usr_msg_item, tx_done_usr_msg_item); // Both this and the TX_wake_up_msg_thread() call below will wake up the message thread, but it doesn't really matter.

                // ... and fall out of this branch. The usr_msg_item is now correct and ready to be
                // stored in the tx_msg_list! The loopback is detected by the TX_handle_tx() and
                // results in a transfer to the RX_msg_list. There's no good reason for sending looped
                // back messages across the TX_thread, other than it simplifies this function.
            }

            if (MSG_TRACE_ENABLED(did, dmodid)) {
                T_IG(TRACE_GRP_TX, "Tx(Msg): len=%u, dmodid=%s, connid=%u", usr_msg_item->len, vtss_module_names[usr_msg_item->dmodid], usr_msg_item->connid);
            }
        }
        T_NG(TRACE_GRP_TX, "len=%u (first %u bytes shown)", usr_msg_item->len, MIN(96, usr_msg_item->len));
        T_NG_HEX(TRACE_GRP_TX, usr_msg_item->usr_msg, MIN(96, usr_msg_item->len));

        VTSS_ASSERT(mcb != NULL); // Keep Lint happy
        CX_concat_msg_items(&mcb->tx_msg_list, &mcb->tx_msg_list_last, usr_msg_item, usr_msg_item);

        // Wake-up the TX_thread()
        TX_wake_up_msg_thread(MSG_FLAG_TX_MSG);
    }

    MSG_STATE_CRIT_EXIT();
}

/******************************************************************************/
// msg_tx()
// This is the simple form of msg_tx_adv(), and it doesn't support callbacks or
// options.
/******************************************************************************/
void msg_tx(vtss_module_id_t dmodid, u32 did, const void *const msg, size_t len)
{
    // Since callers of msg_tx() must always use VTSS_MALLOC() to allocate the message,
    // and the message module will always free it, and there's no TxDone callback,
    // we don't need to make a copy of the message if looping back.
    msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK, dmodid, did, msg, len);
}

/******************************************************************************/
// msg_rx_filter_register()
/******************************************************************************/
vtss_rc msg_rx_filter_register(const msg_rx_filter_t *filter)
{
    vtss_rc rc = MSG_ERROR_PARM;

    // Validate the filter.
    if (!RX_filter_validate(filter)) {
        return rc;
    }

    MSG_CFG_CRIT_ENTER();
    if (!RX_filter_insert(filter)) {
        goto exit_func;
    }

    rc = VTSS_OK;
exit_func:
    MSG_CFG_CRIT_EXIT();
    return rc;
}

/****************************************************************************/
// msg_switch_is_master()
// Once a MASTER_UP event has occurred at the IM_thread()
// this function will return TRUE until a MASTER_DOWN event
// occurs at the same function. This is regardless of whether
// the switch is currently a master as seen from the SPROUT/Msg protocols.
/****************************************************************************/
BOOL msg_switch_is_master(void)
{
    BOOL result;

    MSG_STATE_CRIT_ENTER();
    result = CX_user_state.master;
    MSG_STATE_CRIT_EXIT();

    return result;
}

/****************************************************************************/
// msg_switch_exists()
// Once a SWITCH_ADD event has occurred at the IM_thread()
// this function will return TRUE until a SWITCH_DELETE event
// occurs at the same function. This is regardless of whether
// the switch is actually present in the stack or not (it could
// happen that the switch disappears before the SWITCH_DELETE
// event gets to the IM_thread().
/****************************************************************************/
BOOL msg_switch_exists(vtss_isid_t isid)
{
    BOOL result;

    if (!VTSS_ISID_LEGAL(isid)) {
        T_E("Invalid isid: %u", isid);
        return FALSE;
    }

    MSG_STATE_CRIT_ENTER();
    result = CX_user_state.exists[isid];
    MSG_STATE_CRIT_EXIT();

    return result;
}

/****************************************************************************/
// msg_switch_configurable()
/****************************************************************************/
BOOL msg_switch_configurable(vtss_isid_t isid)
{
    BOOL result;

    if (!VTSS_ISID_LEGAL(isid)) {
        T_E("Invalid isid: %u", isid);
        return FALSE;
    }

    MSG_STATE_CRIT_ENTER();
    result = CX_user_state.master && CX_switch_info.info[isid].configurable;
    MSG_STATE_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// msg_existing_switches()
/****************************************************************************/
u32 msg_existing_switches(void)
{
    u32         result = 0;
    vtss_isid_t isid;

    MSG_STATE_CRIT_ENTER();
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (CX_user_state.exists[isid]) {
            result |= (1 << isid);
        }
    }
    MSG_STATE_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// msg_configurable_switches()
/****************************************************************************/
u32 msg_configurable_switches(void)
{
    u32         result = 0;
    vtss_isid_t isid;

    MSG_STATE_CRIT_ENTER();
    // Unlike msg_existing_switches(), we need to test whether we're master here, because
    // the "configurable" state is not always FALSE when we're slave.
    if (CX_user_state.master) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (CX_switch_info.info[isid].configurable) {
                result |= (1 << isid);
            }
        }
    }
    MSG_STATE_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// msg_switch_is_local()
/****************************************************************************/
BOOL msg_switch_is_local(vtss_isid_t isid)
{
    BOOL result;
    MSG_STATE_CRIT_ENTER();
    result = (CX_user_state.master && isid == CX_user_state.master_isid);
    MSG_STATE_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// msg_master_isid()
// Returns our own isid if we're master, VTSS_ISID_UNKNOWN otherwise.
/****************************************************************************/
vtss_isid_t msg_master_isid(void)
{
    vtss_isid_t result;
    MSG_STATE_CRIT_ENTER();
    result = CX_user_state.master_isid;
    MSG_STATE_CRIT_EXIT();
    return result;
}

/****************************************************************************/
// msg_version_string_get()
// Returns the version string of a given switch in the stack. The call is only
// valid on the master, and the caller must allocate MSG_MAX_VERSION_STRING_LEN
// bytes for @ver_str prior to the call.
// @ver_str[0] is set to '\0' if an error occurred or no version string was
// received from the slave switch.
// The reason for not just returning a pointer is that the stack may change
// right after the pointer is returned, thus resulting in a non-consistent
// version string.
/****************************************************************************/
void msg_version_string_get(vtss_isid_t isid, char *ver_str)
{
    VTSS_ASSERT(ver_str);
    ver_str[0] = '\0';

    if (!VTSS_ISID_LEGAL(isid)) {
        T_E("Invalid isid: %u", isid);
        return;
    }

    MSG_STATE_CRIT_ENTER();
    if (state.state == MSG_MOD_STATE_MST) {
        if (mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
            // mcb->u.master->version_string is guaranteed to be NULL-terminated and less than
            // MSG_MAX_VERSION_STRING_LEN bytes long.
            strcpy(ver_str, mcbs[isid][0].u.master.switch_info.version_string);
        }
    }
    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// msg_product_name_get()
// Returns the product name of a given switch in the stack. The call is only
// valid on the master, and the caller must allocate MSG_MAX_PRODUCT_NAME_LEN
// bytes for @prod_name prior to the call.
// @prod_name[0] is set to '\0' if an error occurred or no product name was
// received from slave switch.
// The reason for not just returning a pointer is that the stack may change
// right after the pointer is returned, thus resulting in a non-consistent
// version string.
/****************************************************************************/
void msg_product_name_get(vtss_isid_t isid, char *prod_name)
{
    VTSS_ASSERT(prod_name);
    prod_name[0] = '\0';

    if (!VTSS_ISID_LEGAL(isid)) {
        T_E("Invalid isid: %u", isid);
        return;
    }

    MSG_STATE_CRIT_ENTER();
    if (state.state == MSG_MOD_STATE_MST) {
        if (mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
            // mcb->u.master->product_name is guaranteed to be NULL-terminated and less than
            // MSG_MAX_PRODUCT_NAME_LEN bytes long.
            strcpy(prod_name, mcbs[isid][0].u.master.switch_info.product_name);
        }
    }
    MSG_STATE_CRIT_EXIT();
}

/****************************************************************************/
// msg_uptime_get()
// Returns the up-time measured in seconds of a switch.
// This function may be called both as a master and as a slave.
// If calling as a slave, @isid must be VTSS_ISID_LOCAL.
// If calling as a master, @isid may be any legal ISID and VTSS_ISID_LOCAL.
// If calling as a master and @isid is not present, 0 is returned.
/****************************************************************************/
time_t msg_uptime_get(vtss_isid_t isid)
{
    time_t uptime;

    MSG_STATE_CRIT_ENTER();
    // CX_uptime_get_crit_taken() returns an u32.
    // time_t is also an u32 and represents a number of seconds, just as
    // CX_uptime_get_crit_taken() does.
    uptime = CX_uptime_get_crit_taken(isid);
    MSG_STATE_CRIT_EXIT();
    return uptime;
}

/****************************************************************************/
// msg_abstime_get()
// As input it takes a relative time measured in seconds since boot and an
// isid. The isid is used to tell on which switch the relative time was
// recorded on. The returned time is a time_t value that takes into account
// the current switch's SNTP-obtained time.
// The return value can be converted to a string using misc_time2str().
//
// If calling as a slave, @isid must be VTSS_ISID_LOCAL.
// If calling as a master, @isid may be any legal ISID and VTSS_ISID_LOCAL.
// If calling as a master and @isid is not present, 0 is returned.
/****************************************************************************/
time_t msg_abstime_get(vtss_isid_t isid, time_t rel_event_time)
{
    time_t cur_abstime, cur_uptime, abs_boot_time_of_this_switch, result, diff_time;

    MSG_STATE_CRIT_ENTER();

    VTSS_ASSERT(VTSS_ISID_LEGAL(isid) || isid == VTSS_ISID_LOCAL);

    // Get the current absolute time on the local switch, which is the
    // one that we convert all times into.
    cur_abstime = time(NULL);

    // Get the number of seconds that this switch has currently been up since boot.
    cur_uptime = CX_uptime_secs_get();

    // "Luckily", time() is measured in seconds, so that the uptime_secs
    // and time_t are compatible.
    if (cur_abstime < cur_uptime) {
        T_W("Current absolute time (%u) is smaller than the switch's uptime (%u)", (u32)cur_abstime, (u32)cur_uptime);
        cur_abstime = cur_uptime;
    }

    // Compute the absolute time that this switch was booted
    abs_boot_time_of_this_switch = cur_abstime - cur_uptime;

    if (isid == VTSS_ISID_LOCAL) {
        // No need to look up difference between master and slave time.
        result = abs_boot_time_of_this_switch + rel_event_time;
    } else if (state.state == MSG_MOD_STATE_MST && mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
        // We're master and the connection is established.
        // We need to take special care of the case where the slave's uptime is
        // bigger than that of this switch's.
        if (mcbs[isid][0].u.master.switch_info.slv_uptime_secs > mcbs[isid][0].u.master.switch_info.mst_uptime_secs) {
            diff_time = mcbs[isid][0].u.master.switch_info.slv_uptime_secs - mcbs[isid][0].u.master.switch_info.mst_uptime_secs;
            // The slave booted diff_time seconds before us.
            // Check if the event happened before this switch was booted
            if (diff_time > rel_event_time) {
                // The event (at rel_event_time) happened before this switch was booted.
                diff_time -= rel_event_time;
                // diff_time now holds the number of seconds that the event happened before
                // this switch was booted. If this is a bigger number than this switch's absolute
                // boot time, we use this switch's absolute boot time as the result. When this
                // happens, it's an indication that this switch hasn't gotten an SNTP time.
                if (diff_time > abs_boot_time_of_this_switch) {
                    result = abs_boot_time_of_this_switch;
                } else {
                    // This switch has probably gotten an SNTP time. The event still
                    // happened before this switch was booted, though.
                    result = abs_boot_time_of_this_switch - diff_time;
                }
            } else {
                // The event happened after the master switch (this switch) was booted.
                result = (rel_event_time - diff_time) + abs_boot_time_of_this_switch;
            }
        } else {
            // The slave switch booted at the same time or after the master (this) switch.
            diff_time = mcbs[isid][0].u.master.switch_info.mst_uptime_secs - mcbs[isid][0].u.master.switch_info.slv_uptime_secs;
            // diff_time now holds the number of seconds after the master switch booted that
            // the slave switch booted.
            // abs_boot_time_of_this_switch + diff_time is the absolute boot time of the slave switch.
            // The event happened rel_event_time seconds after the the slave switch booted.
            result = abs_boot_time_of_this_switch + diff_time + rel_event_time;
        }
    } else {
        // We're not master or the slave is not connected.
        result = 0;
    }

    MSG_STATE_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// msg_switch_info_get()
/******************************************************************************/
vtss_rc msg_switch_info_get(vtss_isid_t isid, msg_switch_info_t *info)
{
    vtss_rc result = VTSS_RC_OK;

    if ((!VTSS_ISID_LEGAL(isid) && isid != VTSS_ISID_LOCAL) || info == NULL) {
        return VTSS_RC_ERROR;
    }

    MSG_STATE_CRIT_ENTER();

    if (isid == VTSS_ISID_LOCAL) {
        isid = state.misid;
    }
    if (state.state == MSG_MOD_STATE_MST && mcbs[isid][0].state == MSG_CONN_STATE_MST_ESTABLISHED) {
        *info = mcbs[isid][0].u.master.switch_info;
    } else {
        result = VTSS_RC_ERROR;
    }

    MSG_STATE_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// msg_buf_pool_create()
/******************************************************************************/
void *msg_buf_pool_create(vtss_module_id_t module_id, i8 *dscr, u32 buf_cnt, u32 bytes_per_buf)
{
    u32            pool_size, buf_size, alloc_size;
    void           *mem;
    msg_buf_pool_t *pool;
    msg_buf_t      *buf_iter;
    int            i;

    // Avoid Lint Warning 429: Custodial pointer 'mem' (line 5745) has not been freed or returned
    /*lint --e{429} */

    if (module_id >= VTSS_MODULE_ID_NONE || buf_cnt == 0 || bytes_per_buf == 0) {
        T_E("Invalid arg");
        return NULL;
    }

    pool_size  = MSG_ALIGN64(sizeof(msg_buf_pool_t));
    buf_size   = MSG_ALIGN64(sizeof(msg_buf_t)) + MSG_ALIGN64(bytes_per_buf);
    alloc_size = pool_size + buf_cnt * buf_size;

    if ((mem = VTSS_MALLOC(alloc_size)) == NULL) {
        T_E("Can't allocate %u bytes", alloc_size);
        return NULL;
    }

    pool = (msg_buf_pool_t *)MSG_ALIGN64(mem);

    // Create a counting semaphore.
    VTSS_OS_SEM_CREATE(&pool->sem, buf_cnt);
    pool->magic        = MSG_BUF_POOL_MAGIC;
    pool->dscr         = dscr;
    pool->buf_cnt_init = buf_cnt;
    pool->buf_cnt_cur  = buf_cnt;
    pool->buf_cnt_min  = buf_cnt;
    pool->buf_size     = bytes_per_buf;
    pool->allocs       = 0;
    pool->module_id    = module_id;
    pool->used         = NULL;

    // Set-up linked lists.
    buf_iter = (msg_buf_t *)((u8 *)pool + pool_size);
    pool->free = buf_iter;
    for (i = 0; i < (int)buf_cnt; i++) {
        buf_iter->pool = pool;
        buf_iter->buf  = (msg_buf_t *)((u8 *)buf_iter + MSG_ALIGN64(sizeof(msg_buf_t)));
        buf_iter->next = (msg_buf_t *)((u8 *)buf_iter->buf + MSG_ALIGN64(bytes_per_buf));
        if (i < (int)(buf_cnt - 1)) {
            buf_iter = buf_iter->next;
        }
    }
    buf_iter->next = NULL;

    if ((u8 *)buf_iter + buf_size != (u8 *)mem + alloc_size) {
        T_E("Bad implementation");
    }

    // Allow this function to be called during INIT_CMD_INIT, so don't
    // use MSG_BUF_CRIT_ENTER/EXIT()() but simply lock and unlock the scheduler.
    cyg_scheduler_lock();
    pool->next = MSG_buf_pool;
    MSG_buf_pool = pool;
    cyg_scheduler_unlock();

    return pool;
}

/******************************************************************************/
// msg_buf_pool_get()
/******************************************************************************/
void *msg_buf_pool_get(void *buf_pool)
{
    msg_buf_pool_t *pool = buf_pool;
    msg_buf_t      *buf_ptr;

    if (!pool || pool->magic != MSG_BUF_POOL_MAGIC) {
        T_E("Invalid pool magic");
        return NULL;
    }
    (void)VTSS_OS_SEM_WAIT(&pool->sem);
    MSG_BUF_CRIT_ENTER();
    buf_ptr = pool->free;
    VTSS_ASSERT(buf_ptr != NULL);

    if (pool->buf_cnt_cur == 0) {
        T_E("Invalid implementation");
    } else {
        pool->buf_cnt_cur--;
        if (pool->buf_cnt_cur < pool->buf_cnt_min) {
            pool->buf_cnt_min = pool->buf_cnt_cur;
        }
    }
    pool->allocs++;
    pool->free       = buf_ptr->next;
    buf_ptr->next    = pool->used;
    buf_ptr->ref_cnt = 1;
    pool->used       = buf_ptr;

    MSG_BUF_CRIT_EXIT();
    VTSS_ASSERT(buf_ptr->buf != NULL);
    return buf_ptr->buf;
}

/******************************************************************************/
// msg_buf_pool_put()
/******************************************************************************/
u32 msg_buf_pool_put(void *buf)
{
    msg_buf_t      *buf_ptr, *iter, *prev;
    msg_buf_pool_t *pool;
    u32            ref_cnt = 0;

    buf_ptr = (msg_buf_t *)((u8 *)buf - MSG_ALIGN64(sizeof(msg_buf_t)));

    if (!buf_ptr || !buf_ptr->pool || buf_ptr->pool->magic != MSG_BUF_POOL_MAGIC) {
        T_E("Invalid buf");
        return ref_cnt;
    }

    pool = buf_ptr->pool;

    MSG_BUF_CRIT_ENTER();

    if (buf_ptr->ref_cnt == 0) {
        T_E("Invalid implementation");
    } else if ((ref_cnt = --buf_ptr->ref_cnt) == 0) {
        iter = pool->used;
        prev = NULL;
        while (iter && iter != buf_ptr) {
            prev = iter;
            iter = iter->next;
        }

        if (!iter) {
            T_E("No such buffer");
            MSG_BUF_CRIT_EXIT();
            return ref_cnt;
        }

        if (prev == NULL) {
            pool->used = buf_ptr->next;
        } else {
            prev->next = buf_ptr->next;
        }

        if (pool->buf_cnt_cur == pool->buf_cnt_init) {
            T_E("Invalid implementation");
        } else {
            pool->buf_cnt_cur++;
        }

        buf_ptr->next = pool->free;
        pool->free = buf_ptr;
        VTSS_OS_SEM_POST(&pool->sem);
    }

    MSG_BUF_CRIT_EXIT();
    return ref_cnt;
}

/******************************************************************************/
// msg_buf_pool_ref_cnt_set()
/******************************************************************************/
void msg_buf_pool_ref_cnt_set(void *buf, u32 ref_cnt)
{
    msg_buf_t *buf_ptr;

    buf_ptr = (msg_buf_t *)((u8 *)buf - MSG_ALIGN64(sizeof(msg_buf_t)));

    if (!buf_ptr || !buf_ptr->pool || buf_ptr->pool->magic != MSG_BUF_POOL_MAGIC) {
        T_E("Invalid buf");
        return;
    }

    if (ref_cnt == 0) {
        MSG_BUF_CRIT_ENTER();
        buf_ptr->ref_cnt = 1;
        MSG_BUF_CRIT_EXIT();
        (void)msg_buf_pool_put(buf);
    } else {
        MSG_BUF_CRIT_ENTER();
        buf_ptr->ref_cnt = ref_cnt;
        MSG_BUF_CRIT_EXIT();
    }
}

/******************************************************************************/
// msg_max_user_prio()
/******************************************************************************/
u32 msg_max_user_prio(void)
{
#if VTSS_SWITCH_STACKABLE && defined(VTSS_ARCH_JAGUAR_1) && !MSG_HOP_BY_HOP
    // Potentially using priority 7 for stack management traffic.
    return vtss_stacking_enabled() ? VTSS_PRIOS - 2 : VTSS_PRIOS - 1;
#else
    return VTSS_PRIOS - 1;
#endif
}

/******************************************************************************/
/******************************************************************************/
typedef enum {
    MSG_DBG_CMD_STAT_USR_MSG_PRINT = 1,
    MSG_DBG_CMD_STAT_PROTOCOL_PRINT,
    MSG_DBG_CMD_STAT_LAST_RX_CB_PRINT,
    MSG_DBG_CMD_STAT_LAST_IM_CB_PRINT,
    MSG_DBG_CMD_STAT_SHAPER_PRINT,
    MSG_DBG_CMD_STAT_IM_MAX_CB_TIME_PRINT,
    MSG_DBG_CMD_STAT_POOL_PRINT,
    MSG_DBG_CMD_SWITCH_INFO_PRINT,
    MSG_DBG_CMD_SWITCH_INFO_RELOAD,
    MSG_DBG_CMD_STAT_USR_MSG_CLEAR = 10,
    MSG_DBG_CMD_STAT_PROTOCOL_CLEAR,
    MSG_DBG_CMD_CFG_TRACE_ISID_SET = 20,
    MSG_DBG_CMD_CFG_TRACE_MODID_SET,
    MSG_DBG_CMD_CFG_TIMEOUT,
    MSG_DBG_CMD_CFG_RETRANSMIT,
    MSG_DBG_CMD_CFG_WINSZ,
    MSG_DBG_CMD_CFG_FLASH_CLEAR,
    MSG_DBG_CMD_CFG_HTL,
    MSG_DBG_CMD_TEST_RUN = 40,
    MSG_DBG_CMD_TEST_RENEGOTIATE,
} msg_dbg_cmd_num_t;

/******************************************************************************/
/******************************************************************************/
typedef struct {
    msg_dbg_cmd_num_t cmd_num;
    char *cmd_txt;
    char *arg_syntax;
    uint max_arg_cnt;
    void (*func)(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms);
} msg_dbg_cmd_t;

/******************************************************************************/
/******************************************************************************/
static msg_dbg_cmd_t msg_dbg_cmds[] = {
    {
        MSG_DBG_CMD_STAT_USR_MSG_PRINT,
        "Print User Message Statistics",
        NULL,
        0,
        DBG_cmd_stat_usr_msg_print
    },
    {
        MSG_DBG_CMD_STAT_PROTOCOL_PRINT,
        "Print Message Protocol Statistics",
        NULL,
        0,
        DBG_cmd_stat_protocol_print
    },
    {
        MSG_DBG_CMD_STAT_LAST_RX_CB_PRINT,
        "Print last callback to Msg Rx",
        NULL,
        0,
        DBG_cmd_stat_last_rx_cb_print
    },
    {
        MSG_DBG_CMD_STAT_LAST_IM_CB_PRINT,
        "Print last callback to init_modules()",
        NULL,
        0,
        DBG_cmd_stat_last_im_cb_print
    },
    {
        MSG_DBG_CMD_STAT_SHAPER_PRINT,
        "Print shaper status",
        NULL,
        0,
        DBG_cmd_stat_shaper_print
    },
    {
        MSG_DBG_CMD_STAT_IM_MAX_CB_TIME_PRINT,
        "Print longest init_modules() callback time",
        "[clear]",
        1,
        DBG_cmd_stat_im_max_cb_time_print
    },
    {
        MSG_DBG_CMD_STAT_POOL_PRINT,
        "Print message pool statistics",
        NULL,
        0,
        DBG_cmd_stat_pool_print
    },
    {
        MSG_DBG_CMD_SWITCH_INFO_PRINT,
        "Print switch info",
        NULL,
        0,
        DBG_cmd_switch_info_print
    },
    {
        MSG_DBG_CMD_SWITCH_INFO_RELOAD,
        "Reload switch info from flash (has an effect on slaves only)",
        NULL,
        0,
        DBG_cmd_switch_info_reload
    },
    {
        MSG_DBG_CMD_STAT_USR_MSG_CLEAR,
        "Clear User Message Statistics",
        NULL,
        0,
        DBG_cmd_stat_usr_msg_clear
    },
    {
        MSG_DBG_CMD_STAT_PROTOCOL_CLEAR,
        "Clear Message Protocol Statistics",
        NULL,
        0,
        DBG_cmd_stat_protocol_clear
    },
    {
        MSG_DBG_CMD_CFG_TRACE_ISID_SET,
        "Configure trace output per ISID - only used when master",
        "[<isid> [<enable> (0 or 1)]] - Use <isid> = 0 to enable or disable all",
        2,
        DBG_cmd_cfg_trace_isid_set
    },
    {
        MSG_DBG_CMD_CFG_TRACE_MODID_SET,
        "Configure trace output per module",
        "[<module_id> [<enable> (0 or 1)]] - Use <module_id> = -1 to enable or disable all",
        2,
        DBG_cmd_cfg_trace_modid_set
    },
    {
        MSG_DBG_CMD_CFG_TIMEOUT,
        "Configure timeouts",
        "[<MSYN timeout> [<MD timeout>]] - all in milliseconds (0 = no change)",
        2,
        DBG_cmd_cfg_timeout
    },
    {
        MSG_DBG_CMD_CFG_RETRANSMIT,
        "Configure retransmits",
        "[<MD retransmits>]",
        1,
        DBG_cmd_cfg_retransmit
    },
    {
        MSG_DBG_CMD_CFG_WINSZ,
        "Configure window sizes",
        "[[<master per slave> [<slave>]] (0 = no change)",
        2,
        DBG_cmd_cfg_winsz
    },
    {
        MSG_DBG_CMD_CFG_FLASH_CLEAR,
        "Erase message module's knowledge about other switches. Takes effect upon next boot.",
        NULL,
        0,
        DBG_cmd_cfg_flash_clear
    },
#if MSG_HOP_BY_HOP
    {
        MSG_DBG_CMD_CFG_HTL,
        "Configure hops-to-live (1 = only to neighboring switch)",
        "[<HTL>]",
        1,
        DBG_cmd_cfg_htl
    },
#endif
    {
        MSG_DBG_CMD_TEST_RUN,
        "Run a test case. Omit parameters to see which TC are available. Some TCs take arguments",
        "[<TC> {<TC arg>}]",
        4,
        DBG_cmd_test_run
    },
    {
        MSG_DBG_CMD_TEST_RENEGOTIATE,
        "Re-negotiate a connection towards a slave (master only)",
        "[<ISID>] - leave argument out to re-negotiate all",
        1,
        DBG_cmd_test_renegotiate
    },
    {
        0,
        NULL,
        NULL,
        0,
        NULL
    }
};

/******************************************************************************/
// msg_dbg()
/******************************************************************************/
void msg_dbg(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int i;
    u32 cmd_num;
    if (parms_cnt == 0) {
        (void)dbg_printf("Usage: debug msg <cmd idx>\n");
        (void)dbg_printf("Most commands show current settings if called without arguments\n\n");
        (void)dbg_printf("Commands:\n");
        i = 0;
        while (msg_dbg_cmds[i].cmd_num != 0) {
            (void)dbg_printf("  %2d: %s\n", msg_dbg_cmds[i].cmd_num, msg_dbg_cmds[i].cmd_txt);
            if (msg_dbg_cmds[i].arg_syntax && msg_dbg_cmds[i].arg_syntax[0]) {
                (void)dbg_printf("      Arguments: %s.\n", msg_dbg_cmds[i].arg_syntax);
            }
            i++;
        }
        return;
    }

    cmd_num = parms[0];

    // Verify that command is known and argument count is correct
    i = 0;
    while (msg_dbg_cmds[i].cmd_num != 0) {
        if (msg_dbg_cmds[i].cmd_num == cmd_num) {
            break;
        }
        i++;
    }
    if (msg_dbg_cmds[i].cmd_num == 0) {
        DBG_cmd_syntax_error(dbg_printf, "Unknown command number: %d", cmd_num);
        return;
    }
    if (parms_cnt - 1 > msg_dbg_cmds[i].max_arg_cnt) {
        DBG_cmd_syntax_error(dbg_printf, "Incorrect number of arguments (%d).\n"
                             "Arguments: %s",
                             parms_cnt - 1,
                             msg_dbg_cmds[i].arg_syntax);
        return;
    }
    if (msg_dbg_cmds[i].func == NULL) {
        (void)dbg_printf("Internal Error: Function for command %u not implemented (yet?)", cmd_num);
        return;
    }

    msg_dbg_cmds[i].func(dbg_printf, parms_cnt - 1, parms + 1);
}

/******************************************************************************/
// msg_init()
// Initialize Message Module
/******************************************************************************/
vtss_rc msg_init(vtss_init_data_t *data)
{
    /*lint --e{454,456} ... We leave the Mutex locked */
    if (data->cmd == INIT_CMD_INIT) {
        int cid;
        vtss_isid_t isid;
        vtss_module_id_t modid;

        for (modid = 0; modid < VTSS_MODULE_ID_NONE; modid++) {
            msg_shaper[modid].limit = MSG_SHAPER_DEFAULT_LIMIT;
        }

        CX_user_state.master_isid = VTSS_ISID_UNKNOWN;

        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        MSG_ASSERT(VTSS_MODULE_ID_NONE < 256, "Message module supports at most 255 module IDs");

        // Nothing will work if ISID==0 is not reserved, and VTSS_ISID_START is not 1,
        // since we use the ISIDs in the API calls as the first index into the
        // mcbs[][] array.
        VTSS_ASSERT(VTSS_ISID_START == 1);

        // Also the selected sampling time must be something bigger than the tick rate.
        VTSS_ASSERT((MSG_SAMPLE_TIME_MS / ECOS_MSECS_PER_HWTICK) > 1);

        pend_rx_list = pend_rx_list_last = pend_tx_done_list = pend_tx_done_list_last = NULL;

        for (isid = 0; isid < VTSS_ISID_CNT + 1; isid++) {
            msg_trace_enabled_per_isid[isid] = TRUE;
        }

        for (modid = 0; modid < VTSS_MODULE_ID_NONE; modid++) {
            msg_trace_enabled_per_modid[modid] = TRUE;
        }

        // Per default disable TOPO's trace output, since it sends periodic updates,
        // which are disturbing.
        msg_trace_enabled_per_modid[VTSS_MODULE_ID_TOPO] = FALSE;

        // Initialize global state.
        state.state = MSG_MOD_STATE_SLV; // Boot in slave mode.
        state.slv_next_connid = VTSS_ISID_END;
        state.msg_cfg_msyn_timeout_ms          = MSG_CFG_DEFAULT_MSYN_TIMEOUT_MS;
        state.msg_cfg_md_timeout_ms            = MSG_CFG_DEFAULT_MD_TIMEOUT_MS;
        state.msg_cfg_md_retransmit_limit      = MSG_CFG_DEFAULT_MD_RETRANSMIT_LIMIT;
        state.msg_cfg_slv_winsz                = MSG_CFG_DEFAULT_SLV_WINSZ;
        state.msg_cfg_mst_winsz_per_slv        = MSG_CFG_DEFAULT_MST_WINSZ_PER_SLV;
        state.msg_cfg_htl_limit                = MSG_CFG_DEFAULT_HTL_LIMIT;

        // Reset statistics
        memset(&state.usr_stat[0], 0, sizeof(state.usr_stat));
        memset(&state.pdu_stat[0], 0, sizeof(state.pdu_stat));
        memset(&state.relay_stat[0], 0, sizeof(state.relay_stat));

        // Initialize MCBs.
        for (isid = 0; isid < VTSS_ISID_CNT + 1; isid++) {
            for (cid = 0; cid < MSG_CFG_CONN_CNT; cid++) {
                msg_mcb_t *mcb = &mcbs[isid][cid];
                if (isid == MSG_SLV_ISID_IDX) {
                    mcb->state = MSG_CONN_STATE_SLV_NO_MST;
                } else {
                    CX_set_state_mst_no_slv(mcb);
                }
                mcb->next_available_sseq = isid * 10000; // Initialized here and never more. Pick different numbers to ease debugging
                mcb->tx_msg_list      = NULL;
                mcb->tx_msg_list_last = NULL;
                mcb->rx_msg_list      = NULL;
                // Remaining fields are initialized as the states are changed.
            }
        }

        // Create semaphore. Initially locked, since we need to load our configuration
        // before we allow others to call us. The conf is loaded in the beginning of
        // TX_thread(), after which the semaphores are released.
        critd_init(&crit_msg_state, "State", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Mutex protecting our counters
        critd_init(&crit_msg_counters, "Counters", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Critical region protecting the message subscription filter list.
        critd_init(&crit_msg_cfg, "Config", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Critical region protecting the Rx and Tx Done callback lists.
        critd_init(&crit_msg_pend_list, "Pending list", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Critical region protecting IM_fifo.
        critd_init(&crit_msg_im_fifo, "Init Mods FIFO", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Mutex protecting message buffers
        critd_init(&crit_msg_buf, "Buffers", VTSS_MODULE_ID_MSG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        MSG_BUF_CRIT_EXIT();

        // Initialize FIFO for transporting init_modules() call messages from any thread to the IM_thread.
        IM_fifo_cnt = IM_fifo_rd_idx = IM_fifo_wr_idx = 0;

        // Create a flag that can wake up the IM_thread.
        cyg_flag_init(&IM_flag);

        // Create a flag that can wake up the TX_thread.
        cyg_flag_init(&TX_msg_flag);

        // Create a flag that can wake up the RX_thread.
        cyg_flag_init(&RX_msg_flag);

        // Create Init Modules Thread
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          IM_thread,
                          0,
                          "Init Modules",
                          IM_thread_stack,
                          sizeof(IM_thread_stack),
                          &IM_thread_handle,
                          &IM_thread_state);
        cyg_thread_resume(IM_thread_handle);

        // Create TX thread.
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          TX_thread,
                          0,
                          "Message TX",
                          TX_msg_thread_stack,
                          sizeof(TX_msg_thread_stack),
                          &TX_msg_thread_handle,
                          &TX_msg_thread_state);
        cyg_thread_resume(TX_msg_thread_handle);

        // Create RX thread. Resumed from TX thread.
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          RX_thread,
                          0,
                          "Message RX",
                          RX_msg_thread_stack,
                          sizeof(RX_msg_thread_stack),
                          &RX_msg_thread_handle,
                          &RX_msg_thread_state);
    }

    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
