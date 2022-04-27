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

*/

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/drv_api.h> /* For cyg_drv_interrupt_XXX() */
#include <network.h>

#include "packet_api.h"
#include "packet.h"
#include "vtss_fifo_api.h"    /* Contains a number of functions for managing a FIFO whose items are of fixed size (sizeof(void *)) */
#include "vtss_fifo_cp_api.h" /* Contains a number of functions for managing a FIFO whose items are of variable size */
#include "port_api.h"
#include "misc_api.h"         /* For iport2uport() */
#include "msg_api.h"          /* For msg_max_user_prio() */
#include "packet_cli.h"
#include "vtss_api_if_api.h"   /* For vtss_api_if_chip_count() */

#if defined(VTSS_SW_OPTION_MEP) && (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC))
#include "mep_api.h"    /* For MEP_INSTANCE_MAX */
#endif /* defined(VTSS_SW_OPTION_MEP) && (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PACKET

#define PACKET_INTERNAL_FLAGS_AFI_CANCEL 0x1

#if (VTSS_TRACE_ENABLED)
// Trace registration. Initialized by packet_init()
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "packet",
    .descr     = "Packet module"
};

#if !defined(PACKET_DEFAULT_TRACE_LVL)
#define PACKET_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_WARNING
#endif /* !defined(PACKET_DEFAULT_TRACE_LVL) */

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = PACKET_DEFAULT_TRACE_LVL,
        .timestamp = TRUE,
        .usec      = TRUE,
    },
    [TRACE_GRP_RX] = {
        .name      = "rx",
        .descr     = "Dump of received packets (info => hdr, noise => data).",
        .lvl       = PACKET_DEFAULT_TRACE_LVL,
        .timestamp = TRUE,
        .usec      = TRUE,
    },
    [TRACE_GRP_TX] = {
        .name      = "tx",
        .descr     = "Dump of transmitted packets (lvl = noise).",
        .lvl       = PACKET_DEFAULT_TRACE_LVL,
        .timestamp = TRUE,
        .usec      = TRUE,
    },
    [TRACE_GRP_TX_DONE] = {
        .name      = "tx_done",
        .descr     = "Dump of done-acks from FDMA",
        .lvl       = PACKET_DEFAULT_TRACE_LVL,
        .timestamp = TRUE,
        .usec      = TRUE,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = PACKET_DEFAULT_TRACE_LVL,
        .timestamp = TRUE,
        .usec      = TRUE,
    },
};
#endif /* VTSS_TRACE_ENABLED */

// Packet Module semaphores worth spending time on using critd_t rather than
// vtss_crit_t, because it's with the RX_filter_crit we risk getting into
// a deadlock if one of the filter-subscribers call any of the
// packet_rx_filter_XXX() functions from their callback function.
static critd_t RX_filter_crit;

// Macros for accessing semaphore functions
#if VTSS_TRACE_ENABLED
#define PACKET_RX_FILTER_CRIT_ENTER()         critd_enter(        &RX_filter_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PACKET_RX_FILTER_CRIT_EXIT()          critd_exit(         &RX_filter_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PACKET_RX_FILTER_CRIT_ASSERT_LOCKED() critd_assert_locked(&RX_filter_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define PACKET_RX_FILTER_CRIT_ENTER()         critd_enter(        &RX_filter_crit)
#define PACKET_RX_FILTER_CRIT_EXIT()          critd_exit(         &RX_filter_crit)
#define PACKET_RX_FILTER_CRIT_ASSERT_LOCKED() critd_assert_locked(&RX_filter_crit)
#endif /* VTSS_TRACE_ENABLED */

#if defined(VTSS_ARCH_LUTON28)
#define FDMA_INTERRUPT CYGNUM_HAL_INT_FDMA
#else
#define FDMA_INTERRUPT CYGNUM_HAL_INTERRUPT_FDMA
#endif /* defined(VTSS_ARCH_xxx) */

/*lint -esym(459, CX_stack_trace_ena) This is a debug thing. Don't bother protecting it */
/*lint -esym(459, TX_pend_cond)       Calls to cyg_cond_signal() do not require ownership of the corresponding mutex */
/*lint -esym(459, TX_dcb_cond)        Calls to cyg_cond_signal() do not require ownership of the corresponding mutex */
/*lint -esym(457, CX_module_counters) Both TX_do_tx() and RX_do_callback() have access to the structure, but not the same fields */
/*lint -esym(459, RX_fifo)            RX_fifo accessed from RX_fdma_packet(), which is fine, because it's in DSR context */
/*lint -esym(459, TX_done_fifo)       TX_fifo accessed from TX_fdma_done(), which is fine, because it's in DSR context */

/***************************************************/
// Common Static Data
/***************************************************/
static cyg_interrupt            CX_fdma_intr_object;
static cyg_handle_t             CX_fdma_intr_handle;
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
static cyg_interrupt            RX_secondary_intr_object;
static cyg_handle_t             RX_secondary_intr_handle;
#endif /* defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR) */

static vtss_os_crit_t           CX_cfg_crit;    // Used to protect common configuration
static packet_port_counters_t   CX_port_counters;
static packet_module_counters_t CX_module_counters[VTSS_MODULE_ID_NONE + 1];
static BOOL                     CX_stack_trace_ena; /* Let it be disabled by default */

/***************************************************/
// Tx Static Data
/***************************************************/
// In lack of a better way to find an optimum Tx buffer count value,
// we use the number of ports on this chip and scale it with some number.
#define TX_BUF_CNT (12 * VTSS_PORTS)

static packet_tx_dcb_t TX_dcbs[TX_BUF_CNT];
static cyg_mutex_t     TX_dcb_mutex;
static cyg_cond_t      TX_dcb_cond;
static packet_tx_dcb_t *TX_dcb_head;

// TX done thread variables
static cyg_handle_t TX_done_thread_handle;
static cyg_thread   TX_done_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         TX_done_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// This holds a list of transmitted packets produced by TX_fdma_done(),
// which is running in DSR context and consumed by TX_done_thread(),
// which is running in thread context, so that the user-defined callback
// function can release the tx buffer if needed.
static vtss_fifo_t TX_done_fifo;
static cyg_flag_t  TX_done_flag; // This flag is used to synchronize between Tx DSR and Tx done thread context.

static vtss_fifo_cp_t TX_pend_fifo; // Fifo of frames pending for Tx, which cannot be sent because of lack of Tx DCBs.
static cyg_mutex_t TX_pend_mutex;
static cyg_cond_t  TX_pend_cond;

// TX pending thread variables
static cyg_handle_t TX_pend_thread_handle;
static cyg_thread   TX_pend_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         TX_pend_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// A couple of statistic counters
static u32          TX_alloc_calls;
static u32          TX_free_calls;

/***************************************************/
// AFI Static Data
/***************************************************/
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
#if defined(MEP_INSTANCE_MAX) && MEP_INSTANCE_MAX > 0
#define AFI_BUF_CNT (2 * MEP_INSTANCE_MAX)
#else
#define AFI_BUF_CNT 100
#endif /* defined(MEP_INSTANCE_MAX) && MEP_INSTANCE_MAX > 0 */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)  */

/***************************************************/
// Rx Static Data
/***************************************************/
// In lack of a better way to find an optimum Rx buffer count value,
// we use the number of ports on this chip and scale it with some number.
#define RX_BUF_CNT (16 * VTSS_PORTS)

// RX thread variables
static cyg_handle_t RX_thread_handle;
static cyg_thread   RX_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         RX_thread_stack[2 * THREAD_DEFAULT_STACK_SIZE];

// This holds a list of received packets produced by RX_fdma_packet(),
// which is running in DSR context and consumed by RX_thread(),
// which is running in thread context.
static vtss_fifo_t             RX_fifo;
static cyg_flag_t              RX_flag; // This flag is used to synchronize between Rx DSR and thread context.
static packet_rx_filter_item_t *RX_filter_list = NULL;
static BOOL                    RX_filter_list_changed; // Tells RX_dispatch() to update its local list of packet Rx listeners.

// Rx counters. The per-rx-filter counters are held in the relevant list items.
// These two hold the counters for packets with no subscribers.
static u64 rx_bytes_no_subscribers = 0;
static u32 rx_pkts_no_subscribers  = 0;

#define PACKET_INLINE inline

/****************************************************************************/
/*                                                                          */
/*  NAMING CONVENTION FOR INTERNAL FUNCTIONS:                               */
/*    RX_<function_name> : Functions related to Rx (extraction).            */
/*    TX_<function_name> : Functions related to Tx (injection).             */
/*    CX_<function_name> : Functions related to both Rx and Tx (common).    */
/*    DBG_<function_name>: Functions related to debugging.                  */
/*                                                                          */
/*  NAMING CONVENTION FOR EXTERNAL (API) FUNCTIONS:                         */
/*    packet_rx_<function_name>: Functions related to Rx (extraction).      */
/*    packet_tx_<function_name>: Functions related to Tx (injection).       */
/*    packet_<function_name>   : Functions related to both Rx and Tx.       */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// CX_ntohs()
// Takes a pointer to the first byte in a 2-byte series that should be
// converted to an u16. The 2-byte series is in network order.
/****************************************************************************/
static PACKET_INLINE u16 CX_ntohs(u8 *p)
{
    return (((u16)p[0] << 8) | (u16)p[1]);
}

/****************************************************************************/
// RX_fdma_packet()
// This function is called in DSR context when a frame has arrived.
// It's sole purpose is to store it on a FIFO and signal a flag. This flag
// is intercepted by the RX_thread() function, which takes care of
// dispatching the frame to subscribers.
/****************************************************************************/
static vtss_fdma_list_t *RX_fdma_packet(void *cntxt, vtss_fdma_list_t *list)
{
    // Cannot call trace functions from DSR context, because T_x() use the time_str() function,
    // which in turn calls time(), which in turn calls wallclock->get_current_time(), which
    // in turn calls cyg_drv_mutex_lock(wallclock_lock), which is illegal from DSR context.
    // T_DG(TRACE_GRP_RX, "DSR: Rx packet from qu %d", qu);

    // Store the frame in the rx_fifo. It is completely safe
    // to do it without any mutexes around the call, because
    // this function is un-interruptible (called in DSR context).
    if (vtss_fifo_wr(&RX_fifo, list) == VTSS_OK) {
        // The frame is now stored in the FIFO, so we cannot return it
        // to the FDMA code as available storage. This must be done
        // from RX_thread().

        // Signal the RX flag to wake up the packet receiver (RX_thread()).
        cyg_flag_setbits(&RX_flag, 1);

        return NULL; // List is not available for FDMA yet.
    }

    // If we get here, there wasn't room in the FIFO, so we can safely re-use
    // the buffer right away (and thereby drop the frame).
    return list;
}

/****************************************************************************/
// DBG_cmd_syntax_error()
/****************************************************************************/
static void DBG_cmd_syntax_error(packet_dbg_printf_t dbg_printf, const char *fmt, ...)
{
    va_list ap = NULL;
    char    s[200] = "Command syntax error: ";
    int     len;

    len = strlen(s);

    va_start(ap, fmt);

    (void)vsnprintf(s + len, sizeof(s) - len - 1, fmt, ap);
    (void)dbg_printf("%s\n", s);

    va_end(ap);
}

/******************************************************************************/
// RX_filter_validate()
/******************************************************************************/
static BOOL RX_filter_validate(const packet_rx_filter_t *filter)
{
    if (!filter->cb && !filter->cb_adv) {
        T_E("None of the callback functions defined");
        return FALSE;
    }

    if (filter->cb && filter->cb_adv) {
        T_E("Only one of the callback functions may be defined at a time");
        return FALSE;
    }

    if (filter->prio == 0) {
        T_E("Module %s: Filter priority 0 is reserved. Use one of the PACKET_RX_FILTER_PRIO_xxx definitions", vtss_module_names[filter->modid]);
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_SRC_PORT)) {
        u32 i;
        BOOL non_zero_found = FALSE;
        for (i = 0; i < sizeof(filter->src_port_mask); i++) {
            if (filter->src_port_mask[i] != 0) {
                non_zero_found = TRUE;
                break;
            }
        }
        if (!non_zero_found) {
            T_E("Module %s: Filter is matching empty source port mask", vtss_module_names[filter->modid]);
            return FALSE;
        }
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_VID) && (filter->vid & 0xF000)) {
        T_E(".vid cannot be greater than 4095");
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_VID) && (filter->vid & filter->vid_mask)) {
        T_E("Filter contains non-zero bits in .vid that are not matched against");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_DMAC)) {
        u32 i;
        for (i = 0; i < sizeof(filter->dmac); i++) {
            if (filter->dmac[i] & filter->dmac_mask[i]) {
                T_E("Filter contains non-zero bits in .dmac that are not matched against");
                return FALSE;
            }
        }
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_SMAC)) {
        u32 i;
        for (i = 0; i < sizeof(filter->smac); i++) {
            if (filter->smac[i] & filter->smac_mask[i]) {
                T_E("Filter contains non-zero bits in .smac that are not matched against");
                return FALSE;
            }
        }
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_ETYPE) && (filter->etype & filter->etype_mask)) {
        T_E("Filter contains non-zero bits in .etype that are not matched against");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) && (filter->ipv4_proto & filter->ipv4_proto_mask)) {
        T_E("Filter contains non-zero bits in .ipv4_proto that are not matched against");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_UDP_SRC_PORT) && (filter->udp_src_port_min > filter->udp_src_port_max)) {
        T_E("Filter matches UDP source port, but min is greater than max");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_UDP_DST_PORT) && (filter->udp_dst_port_min > filter->udp_dst_port_max)) {
        T_E("Filter matches UDP destination port, but min is greater than max");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_TCP_SRC_PORT) && (filter->tcp_src_port_min > filter->tcp_src_port_max)) {
        T_E("Filter matches TCP source port, but min is greater than max");
        return FALSE;
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_TCP_DST_PORT) && (filter->tcp_dst_port_min > filter->tcp_dst_port_max)) {
        T_E("Filter matches TCP destination port, but min is greater than max");
        return FALSE;
    }

    if (filter->match & PACKET_RX_FILTER_MATCH_SSPID) {
#if VTSS_SWITCH_STACKABLE
        if (filter->sspid & filter->sspid_mask) {
            T_E("Filter contains non-zero bits in .sspid that are not matched against");
            return FALSE;
        }
#else
        T_W("This architecture does not support match against SSPID");
#endif /* VTSS_SWITCH_STACKABLE */
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) && (filter->match & PACKET_RX_FILTER_MATCH_ETYPE) && (filter->etype != ETYPE_IPV4 || filter->etype_mask != 0)) {
        T_E("Filter matches against an IPv4 protocol, but ETYPE is not set to ETYPE_IPV4");
        return FALSE;
    }

    if (((filter->match & PACKET_RX_FILTER_MATCH_UDP_SRC_PORT) || (filter->match & PACKET_RX_FILTER_MATCH_UDP_DST_PORT)) && (filter->match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) && (filter->ipv4_proto != IP_PROTO_UDP || filter->ipv4_proto_mask != 0)) {
        T_E("Filter matches against UDP source or destination port range, but ipv4_proto is not set to IP_PROTO_UDP");
        return FALSE;
    }

    if (((filter->match & PACKET_RX_FILTER_MATCH_TCP_SRC_PORT) || (filter->match & PACKET_RX_FILTER_MATCH_TCP_DST_PORT)) && (filter->match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) && (filter->ipv4_proto != IP_PROTO_TCP || filter->ipv4_proto_mask != 0)) {
        T_E("Filter matches against TCP source or destination port range, but ipv4_proto is not set to IP_PROTO_TCP");
        return FALSE;
    }

    if (filter->match & PACKET_RX_FILTER_MATCH_SSPID) {
#if VTSS_SWITCH_STACKABLE
        if ((filter->match & PACKET_RX_FILTER_MATCH_ETYPE) && (filter->etype != VTSS_ETYPE_VTSS || filter->etype_mask != 0)) {
            T_E("Filter matches against an SSPID, but ETYPE is not set to VTSS_ETYPE_VTSS");
            return FALSE;
        }
#else
        // T_W("This architecture does not support match against SSPID"); Already printed once above.
#endif /* VTSS_SWITCH_STACKABLE */
    }

    if ((filter->match & PACKET_RX_FILTER_MATCH_DEFAULT) && ((filter->match & ~PACKET_RX_FILTER_MATCH_DEFAULT) != 0)) {
        T_E("When the PACKET_RX_FILTER_MATCH_DEFAULT is used, no other PACKET_RX_FILTER_MATCH_xxx may be used");
        return FALSE;
    }

    return TRUE;
}

/******************************************************************************/
/******************************************************************************/
static BOOL RX_filter_remove(const void *filter_id)
{
    packet_rx_filter_item_t *l = RX_filter_list, *parent = NULL;

    // Critical region must be obtained when this function is called.
    PACKET_RX_FILTER_CRIT_ASSERT_LOCKED();

    // Find the filter.
    while (l) {
        if (l == filter_id) {
            break;
        }
        parent = l;
        l = l->next;
    }

    if (!l) {
        T_E("Filter ID not found");
        return FALSE;
    }

    // Remove it from the list.
    if (parent == NULL) {
        RX_filter_list = l->next;
    } else {
        parent->next = l->next;
    }

    RX_filter_list_changed = TRUE;

    // And free it.
    VTSS_FREE(l);

    return TRUE;
}

/******************************************************************************/
/******************************************************************************/
static BOOL RX_filter_insert(const packet_rx_filter_t *filter, void **filter_id)
{
    packet_rx_filter_item_t *l = RX_filter_list, *parent = NULL, *new_l;
    u32 i;

    // Critical region must be obtained when this function is called.
    PACKET_RX_FILTER_CRIT_ASSERT_LOCKED();

    // Figure out where to place this filter in the list, based on the priority.
    // The lower value of prio the higher priority.
    while (l) {
        if (l->filter.prio > filter->prio) {
            break;
        }
        parent = l;
        l = l->next;
    }

    // Allocate a new list item
    if ((new_l = VTSS_MALLOC(sizeof(packet_rx_filter_item_t))) == NULL) {
        T_E("VTSS_MALLOC(subscr_list_item_t) failed");
        return FALSE;
    }

    T_I("Caller: %d = %s", filter->modid, vtss_module_names[filter->modid]);

    // Copy the filter
    memcpy(&new_l->filter, filter, sizeof(new_l->filter));

    // Optimize filter. The user calls us with zeros for the places that
    // must match, but internally we use ones to be able to bit-wise AND.
    new_l->filter.vid_mask       |= 0xF000; // Still inverse polarity (match on bits set to 0), but we must only match on - at most - the 12 LSbits.
    new_l->filter.vid_mask        = ~new_l->filter.vid_mask;
    new_l->filter.etype_mask      = ~new_l->filter.etype_mask;
    new_l->filter.ipv4_proto_mask = ~new_l->filter.ipv4_proto_mask;
    new_l->filter.sspid_mask      = ~new_l->filter.sspid_mask;
    for (i = 0; i < sizeof(new_l->filter.dmac_mask); i++) {
        new_l->filter.dmac_mask[i] = ~new_l->filter.dmac_mask[i];
        new_l->filter.smac_mask[i] = ~new_l->filter.smac_mask[i];
    }

    // If user has specified one of the complex match options,
    // help him in specifying the remaining.
    if ((new_l->filter.match & PACKET_RX_FILTER_MATCH_UDP_SRC_PORT) || (new_l->filter.match & PACKET_RX_FILTER_MATCH_UDP_DST_PORT)) {
        new_l->filter.match          |= PACKET_RX_FILTER_MATCH_IPV4_PROTO;
        new_l->filter.ipv4_proto      = IP_PROTO_UDP;
        new_l->filter.ipv4_proto_mask = 0xFF;
    }
    if ((new_l->filter.match & PACKET_RX_FILTER_MATCH_TCP_SRC_PORT) || (new_l->filter.match & PACKET_RX_FILTER_MATCH_TCP_DST_PORT)) {
        new_l->filter.match          |= PACKET_RX_FILTER_MATCH_IPV4_PROTO;
        new_l->filter.ipv4_proto      = IP_PROTO_TCP;
        new_l->filter.ipv4_proto_mask = 0xFF;
    }
    if (new_l->filter.match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) {
        new_l->filter.match     |= PACKET_RX_FILTER_MATCH_ETYPE;
        new_l->filter.etype      = ETYPE_IPV4;
        new_l->filter.etype_mask = 0xFFFF;
    }
#if VTSS_SWITCH_STACKABLE
    if (new_l->filter.match & PACKET_RX_FILTER_MATCH_SSPID) {
        new_l->filter.match     |= PACKET_RX_FILTER_MATCH_ETYPE;
        new_l->filter.etype      = VTSS_ETYPE_VTSS;
        new_l->filter.etype_mask = 0xFFFF;
    }
#endif /* VTSS_SWITCH_STACKABLE */

    // Insert the filter in the list.
    new_l->next = l;
    if (parent == NULL) {
        RX_filter_list = new_l;
    } else {
        parent->next = new_l;
    }

    RX_filter_list_changed = TRUE;

    // The filter_id may be used later on to unsubscribe.
    *filter_id = new_l;

    return TRUE;
}

/****************************************************************************/
// RX_vstax_hdr_details_trace()
/****************************************************************************/
static PACKET_INLINE i8 *RX_vstax_hdr_details_trace(vtss_fdma_list_t *packet)
{
#if VTSS_SWITCH_STACKABLE && defined(VTSS_ARCH_JAGUAR_1)
    static i8 RX_trace_buf[2000]; // It's fine to have it static, since this function only gets called from a single thread.
    u8  *vstax_hdr = packet->frm_ptr - 16; // Well, this is where we think the VStaX header is located.
    u64 vstax;
    u32 qos_cl_dp, qos_sp, qos_cl_qos, ingr_drop_mode, ttl, lrn_mode, fwd_mode;
    u32 cl_pcp, cl_dei, cl_vid, was_tagged, tag_type, ingr_port_type;
    u32 dst_port_type, dst_upsid, dst_pn;
    u32 src_addr_mode, src_port_type, src_upsid, src_upspn, src_intpn, cnt = 0;

    cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                    "\n"
                    "VSX.valid              = %u",
                    packet->rx_info->vstax.valid);

    if (!packet->rx_info->vstax.valid) {
        return RX_trace_buf;
    }

    vstax = ((u64)vstax_hdr[2] << 56) | ((u64)vstax_hdr[3] << 48) | ((u64)vstax_hdr[4] << 40) | ((u64)vstax_hdr[5] << 32) |
            ((u64)vstax_hdr[6] << 24) | ((u64)vstax_hdr[7] << 16) | ((u64)vstax_hdr[8] <<  8) | ((u64)vstax_hdr[9] <<  0);

    qos_cl_dp      = VTSS_EXTRACT_BITFIELD64(vstax, 60,  2);
    qos_sp         = VTSS_EXTRACT_BITFIELD64(vstax, 59,  1);
    qos_cl_qos     = VTSS_EXTRACT_BITFIELD64(vstax, 56,  3);
    ingr_drop_mode = VTSS_EXTRACT_BITFIELD64(vstax, 55,  1);
    ttl            = VTSS_EXTRACT_BITFIELD64(vstax, 48,  5);
    lrn_mode       = VTSS_EXTRACT_BITFIELD64(vstax, 47,  1);
    fwd_mode       = VTSS_EXTRACT_BITFIELD64(vstax, 44,  3);
    cl_pcp         = VTSS_EXTRACT_BITFIELD64(vstax, 29,  3);
    cl_dei         = VTSS_EXTRACT_BITFIELD64(vstax, 28,  1);
    cl_vid         = VTSS_EXTRACT_BITFIELD64(vstax, 16, 12);
    was_tagged     = VTSS_EXTRACT_BITFIELD64(vstax, 15,  1);
    tag_type       = VTSS_EXTRACT_BITFIELD64(vstax, 14,  1);
    ingr_port_type = VTSS_EXTRACT_BITFIELD64(vstax, 12,  2);
    src_port_type  = VTSS_EXTRACT_BITFIELD64(vstax, 11,  1);
    src_addr_mode  = VTSS_EXTRACT_BITFIELD64(vstax, 10,  1);
    src_upsid      = VTSS_EXTRACT_BITFIELD64(vstax,  5,  5);
    src_upspn      = VTSS_EXTRACT_BITFIELD64(vstax,  0,  5);
    src_intpn      = VTSS_EXTRACT_BITFIELD64(vstax,  0,  4);

    cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                    "\n"
                    "   .qos.cl_dp          = %u\n"
                    "       .sp             = %u\n"
                    "       .iprio          = %u\n"
                    "       .ingr_drop_mode = %u\n"
                    "   .gen.ttl            = %u\n"
                    "       .lrn_mode       = %u\n"
                    "       .fwd_mode       = %s\n"
                    "   .tag.cl_pcp         = %u\n"
                    "   .tag.cl_dei         = %u\n"
                    "   .tag.cl_vid         = %u\n"
                    "   .tag.was_tagged     = %u\n"
                    "   .tag.tag_type       = %u\n"
                    "   .tag.ingr_port_type = %u\n",
                    qos_cl_dp,
                    qos_sp,
                    qos_cl_qos,
                    ingr_drop_mode,
                    ttl,
                    lrn_mode,
                    fwd_mode == 0 ? "LLookup" : fwd_mode == 1 ? "Logical" : fwd_mode == 2 ? "Physical" : fwd_mode == 3 ? "Multicast" : fwd_mode == 4 ? "GMirror" : fwd_mode == 5 ? "GCPU, UPS" : fwd_mode == 6 ? "GCPU, ALL" : "Unknown",
                    cl_pcp,
                    cl_dei,
                    cl_vid,
                    was_tagged,
                    tag_type,
                    ingr_port_type);

    if (src_addr_mode == 0 && src_port_type == 0) {
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "   .src.addr_mode      = <upsid, upspn>\n"
                        "       .upsid          = %u\n"
                        "       .upspn          = %u",
                        src_upsid,
                        src_upspn);
    } else if (src_addr_mode == 0 && src_port_type == 1) {
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "   .src.addr_mode      = <upsid, intpn>\n"
                        "       .upsid          = %u\n"
                        "       .intpn          = %u",
                        src_upsid,
                        src_intpn);
    } else {
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "   .src.addr_mode      = GLAG\n"
                        "       .glagid         = %u",
                        src_upspn);
    }

    dst_port_type = VTSS_EXTRACT_BITFIELD64(vstax, 42,  1);
    dst_upsid     = VTSS_EXTRACT_BITFIELD64(vstax, 37,  5);
    dst_pn        = VTSS_EXTRACT_BITFIELD64(vstax, 32,  5);

    if (fwd_mode == 1) {
        // Logical
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "\n"
                        "   .dst.port_type      = %s\n"
                        "       .upsid          = %u\n"
                        "       .pn             = %u",
                        dst_port_type == 0 ? "Front" : "Internal",
                        dst_upsid,
                        dst_pn);
    } else if (fwd_mode == 2) {
        // Physical
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "\n"
                        "   .dst.upsid          = %u\n"
                        "       .pn             = %u",
                        dst_upsid,
                        dst_pn);
    } else if (fwd_mode == 3) {
        // Multicast
        u32 mc_idx = VTSS_EXTRACT_BITFIELD64(vstax, 32, 10);
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "\n"
                        "   .dst.mc_idx         = %u",
                        mc_idx);

    } else if (fwd_mode == 5) {
        // GCPU_UPS
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "\n"
                        "   .dst.upsid          = %u\n"
                        "       .pn             = %u",
                        dst_upsid,
                        dst_pn & 0xF);
    } else if (fwd_mode == 6) {
        // GCPU_ALL
        u32 ttl_keep = VTSS_EXTRACT_BITFIELD64(vstax, 41, 1);
        cnt += snprintf(RX_trace_buf + cnt, MAX(((int)sizeof(RX_trace_buf) - cnt), 0),
                        "\n"
                        "   .dst.ttl_keep       = %u\n"
                        "       .pn             = %u",
                        ttl_keep,
                        dst_pn & 0xF);
    }

    return RX_trace_buf;
#else
    return (i8 *)"";
#endif /* VTSS_SWITCH_STACKABLE && defined(VTSS_ARCH_JAGUAR_1) */
}

/****************************************************************************/
// RX_do_callback()
/****************************************************************************/
static int RX_do_callback(packet_rx_filter_item_t *l, u8 *data_ptr, vtss_fdma_list_t *packet)
{
    int                      result   = 0; // Pass it on to the next by default
    cyg_tick_count_t         tick_cnt = cyg_current_time();
    packet_module_counters_t *cntrs   = &CX_module_counters[l->filter.modid];
    u64                      max_time_ms;

    // Update statistics for this filter.
    cntrs->rx_bytes += packet->rx_info->length;
    cntrs->rx_pkts++;

    if (l->filter.cb) {
        // Use the normal callback function.
        if (l->filter.cb(l->filter.contxt, data_ptr, packet->rx_info)) {
            result = 1; // Consumed
        }
    } else {
        // Call the advanced callback function, which always consumes the packet.
        (void)l->filter.cb_adv(l->filter.contxt, data_ptr, packet->rx_info, packet);
        result = 2; // Consumed and detach_buffers
    }

    tick_cnt = cyg_current_time() - tick_cnt;
    if (tick_cnt > cntrs->longest_rx_callback_ticks) {
        cntrs->longest_rx_callback_ticks = tick_cnt;
    }

    if (l->filter.modid == VTSS_MODULE_ID_MSG) {
        // The message module is pretty busy when adding 16 switches to a stack,
        // so allow it a bit more time.
        max_time_ms = 5000;
    } else {
        max_time_ms = 3000;
    }

    if (tick_cnt > VTSS_OS_MSEC2TICK(max_time_ms)) {
        T_W("Module %s has spent more than %llu msecs (%llu msecs) in its Packet Rx callback", vtss_module_names[l->filter.modid], max_time_ms, VTSS_OS_TICK2MSEC(tick_cnt));
    }

    return result;
}

/****************************************************************************/
// RX_dispatch()
// Dispatch packet to subscribers.
// Return value:
//   If the packet is detached from the FDMA (the filter that consumed the
//   frame used the cb_adv() callback function), then this function returns
//   TRUE, FALSE otherwise.
/****************************************************************************/
static PACKET_INLINE BOOL RX_dispatch(vtss_fdma_list_t *packet)
{
    // We keep an internal list of Rx subscribers so that they can subscribe/unsubscribe while being called back.
    static packet_rx_filter_item_t *local_filter_list;
    static u32                     local_filter_list_len;
    packet_rx_filter_item_t        *l;
    packet_rx_filter_t             *filter;
    u32                            match, i, filter_idx;
    BOOL                           match_default = FALSE, at_least_one_match = FALSE;
    vtss_etype_t                   etype;
    BOOL                           detach_buffers = FALSE, is_ip_any;
    BOOL                           super_prio = FALSE;
    u32                            port_bucket, prio_bucket;
    BOOL                           sflow_subscribers_only = FALSE;
    int                            callback_result;
    vtss_packet_rx_info_t          *rx_info;

    // Fragmented frames are not supported (because this module uses zero-copy, and user modules don't support receiving that)
    // Such frames should never arrive at the Packet module due to FDMA driver configuration.
    PACKET_CHECK(packet->next == NULL, return detach_buffers;);

    rx_info = packet->rx_info;

#if defined(VTSS_ARCH_JAGUAR_1)
    // In one scenario (sFlow), the source port number may be VTSS_PORT_NO_NONE, in which
    // case the frame is received on an interconnect, which is fine for sFlow Tx sampled frames.
    if (rx_info->port_no == VTSS_PORT_NO_NONE && vtss_api_if_chip_count() == 2) {
        sflow_subscribers_only = TRUE;
    } else
#endif /* defined(VTSS_ARCH_JAGUAR_1) */
    {
        PACKET_CHECK(rx_info->port_no != VTSS_PORT_NO_NONE, return detach_buffers;);
    }

#if defined(VTSS_SW_OPTION_SFLOW)
    // One more case, where the frame must only come to sFlow subscribers:
    // If the extraction queue mask from the extraction header indicates that the only
    // reason for sending the frame to the CPU was for sFlow sampling, we need
    // to prevent it from getting to other listeners.
    // This implies that the sFlow Rx queue must only be used for sFlow frames, nothing else.
    if (rx_info->sflow_type != VTSS_SFLOW_TYPE_NONE) {
        if ((rx_info->xtr_qu_mask & ~VTSS_BIT(PACKET_XTR_QU_SFLOW)) == 0) {
            // Only got here due to sFlow sampling. Prevent it from being sent to other modules.
            sflow_subscribers_only = TRUE;
        }
    }
#endif /* VTSS_SW_OPTION_SFLOW */

    etype = CX_ntohs(&packet->frm_ptr[ETYPE_POS]);

    if (CX_stack_trace_ena || etype != 0x8880) {
        i8 glag_buf[30] = {0};

        if (TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, TRACE_GRP_RX, VTSS_TRACE_LVL_INFO)) {
            if (rx_info->glag_no == VTSS_GLAG_NO_NONE) {
                strcpy(glag_buf, "None");
            } else {
                sprintf(glag_buf, "%u", rx_info->glag_no);
            }
        }

        T_NG(    TRACE_GRP_RX, "Packet (first %u bytes shown):", MIN(96, rx_info->length));
        T_NG_HEX(TRACE_GRP_RX, packet->frm_ptr, MIN(96, rx_info->length));

        // Avoid Lint Warning 436: Apparent preprocessor directive in invocation of macro 'T_IG'
        /*lint --e{436} */
        T_IG(TRACE_GRP_RX,
             "\n"
             "DMAC                   = %02x-%02x-%02x-%02x-%02x-%02x\n"
             "SMAC                   = %02x-%02x-%02x-%02x-%02x-%02x\n"
             "EtherType              = 0x%04x\n"
             "Length (w/o FCS)       = %u\n"
             "IFH.iport              = %u\n"
             "   .uport              = %u\n"
             "   .glag               = %s\n"
             "   .tag_type           = %s\n"
             "   .class_tag.pcp      = %u\n"
             "             .dei      = %u\n"
             "             .vid      = %u\n"
             "   .strip_tag.tpid     = 0x%04x\n"
             "             .pcp      = %u\n"
             "             .dei      = %u\n"
             "             .vid      = %u\n"
             "   .qu_mask            = 0x%x\n"
             "   .cos                = %u\n"
             "   .acl_hit            = %u\n"
             "   .acl_idx            = %u\n"
             "   .sw_tstamp          = %u\n"
             "   .tstamp_id          = %u\n"
             "   .hw_tstamp          = 0x%08x\n"
#if defined(VTSS_SW_OPTION_SFLOW)
             "   .sflow_type         = %s\n"
             "   .sflow_port         = %u\n"
#endif /* defined(VTSS_SW_OPTION_SFLOW) */
             "   .oam_info           = %llu\n"
             "   .isdx               = %u"
             "%s",
             packet->frm_ptr[DMAC_POS + 0], packet->frm_ptr[DMAC_POS + 1], packet->frm_ptr[DMAC_POS + 2], packet->frm_ptr[DMAC_POS + 3], packet->frm_ptr[DMAC_POS + 4], packet->frm_ptr[DMAC_POS + 5],
             packet->frm_ptr[SMAC_POS + 0], packet->frm_ptr[SMAC_POS + 1], packet->frm_ptr[SMAC_POS + 2], packet->frm_ptr[SMAC_POS + 3], packet->frm_ptr[SMAC_POS + 4], packet->frm_ptr[SMAC_POS + 5],
             etype,
             rx_info->length,
             rx_info->port_no,
             iport2uport(rx_info->port_no),
             glag_buf,
             rx_info->tag_type == VTSS_TAG_TYPE_UNTAGGED ? "Untagged" : rx_info->tag_type == VTSS_TAG_TYPE_C_TAGGED ? "C" : rx_info->tag_type == VTSS_TAG_TYPE_S_TAGGED ? "S" : rx_info->tag_type == VTSS_TAG_TYPE_S_CUSTOM_TAGGED ? "S-custom" : "Unknown",
             rx_info->tag.pcp,
             rx_info->tag.dei,
             rx_info->tag.vid,
             rx_info->stripped_tag.tpid, // tpid == 0x00 means that no tag was stripped
             rx_info->stripped_tag.pcp,
             rx_info->stripped_tag.dei,
             rx_info->stripped_tag.vid,
             rx_info->xtr_qu_mask,
             rx_info->cos,
             rx_info->acl_hit,
             rx_info->acl_idx,
             rx_info->sw_tstamp.hw_cnt,
             rx_info->tstamp_id,
             rx_info->hw_tstamp,
#if defined(VTSS_SW_OPTION_SFLOW)
             rx_info->sflow_type == VTSS_SFLOW_TYPE_NONE ? "None" : rx_info->sflow_type == VTSS_SFLOW_TYPE_RX ? "Rx" : rx_info->sflow_type == VTSS_SFLOW_TYPE_TX ? "Tx" : rx_info->sflow_type == VTSS_SFLOW_TYPE_ALL ? "All" : "Unknown",
             rx_info->sflow_port_no,
#endif /* defined(VTSS_SW_OPTION_SFLOW) */
             rx_info->oam_info,
             rx_info->isdx,
             RX_vstax_hdr_details_trace(packet));
    }

#if defined(VTSS_SW_OPTION_IPV6)
    is_ip_any = (etype == ETYPE_IPV4 || etype == ETYPE_ARP || etype == ETYPE_IPV6);
#else
    is_ip_any = (etype == ETYPE_IPV4 || etype == ETYPE_ARP);
#endif /* defined(VTSS_SW_OPTION_IPV6) */

#if VTSS_SWITCH_STACKABLE
    if (rx_info->vstax.valid) {
        // Figure out if this is received as a super-prio frame.
        super_prio = rx_info->vstax.sp;
    }
#endif /* VTSS_SWITCH_STACKABLE */

    // Count the frame
    if (super_prio) {
        prio_bucket = VTSS_PRIOS; // Count in the last bucket.
    } else if (rx_info->tag.pcp >= VTSS_PRIOS) {
        prio_bucket = VTSS_PRIOS - 1; // Count in the next-to-last-bucket if the priority field is out of bounds.
    } else {
        prio_bucket = rx_info->tag.pcp;
    }
    // Avoid Lint Warning 685: Relational operator '>=' always evaluates to 'true'
    /*lint -e{568, 685} */
    if (rx_info->glag_no >= VTSS_GLAG_NO_START && rx_info->glag_no < VTSS_GLAG_NO_START + VTSS_GLAGS) {
        // Received on a GLAG. Count it in one of the GLAG buckets.
        port_bucket = rx_info->glag_no - VTSS_GLAG_NO_START + VTSS_PORTS;
    } else if (rx_info->port_no >= VTSS_PORTS) {
        // Unknown source port. Count it in the last bucket.
        port_bucket = VTSS_PORTS + VTSS_GLAGS;
    } else {
        port_bucket = rx_info->port_no;
    }

    // Check to see if the RX filter list (subscriber list) has changed.
    // If so, create a new local list.
    // This will work because RX_dispatch() is called from one single context (the one and only incarnation of RX_thread()).
    PACKET_RX_FILTER_CRIT_ENTER();
    if (RX_filter_list_changed) {
        T_IG(TRACE_GRP_RX, "Updating local subscriber list");

        // The list has changed. Free the old, local one and build a new.
        VTSS_FREE(local_filter_list);

        // We need to figure out how many items we need.
        local_filter_list_len = 0;
        l = RX_filter_list;
        while (l) {
            local_filter_list_len++;
            l = l->next;
        }

        if (local_filter_list_len > 0) {
            local_filter_list = VTSS_MALLOC(local_filter_list_len * sizeof(packet_rx_filter_item_t));
        } else {
            local_filter_list = NULL;
        }

        // Copy items.
        l = RX_filter_list;
        filter_idx = 0;
        while (l) {
            local_filter_list[filter_idx++] = *l;
            l = l->next;
        }

        RX_filter_list_changed = FALSE;
    }

    // Update the port counters while we have the Rx filter crit.
    CX_port_counters.rx_pkts[port_bucket][prio_bucket]++;

    PACKET_RX_FILTER_CRIT_EXIT();

    for (filter_idx = 0; filter_idx < local_filter_list_len; filter_idx++) {
        l      = &local_filter_list[filter_idx];
        filter = &l->filter;
        match  = filter->match;

        if (sflow_subscribers_only && !(match & PACKET_RX_FILTER_MATCH_SFLOW)) {
            // When sflow_subscribers_only is TRUE, only subscribers matching
            // the sflow flag may get the packet because the src_port
            // may be invalid if received on internal ports on JR-48, and
            // because the frame is a random sample of the traffic on the
            // front ports.
            goto no_match;
        }

        if (match & PACKET_RX_FILTER_MATCH_SRC_PORT) {
            if (rx_info->port_no != VTSS_PORT_NO_NONE && VTSS_PORT_BF_GET(filter->src_port_mask, rx_info->port_no) == 0) {
                goto no_match; // Hard to code without goto statements (matching = FALSE; break; in all checks)
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_ACL) {
            if (!rx_info->acl_hit) {
                goto no_match;
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_VID) {
            if ((rx_info->tag.vid & filter->vid_mask) != filter->vid) {
                goto no_match;
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_DMAC) {
            for (i = 0; i < sizeof(filter->dmac); i++) {
                if ((packet->frm_ptr[DMAC_POS + i] & filter->dmac_mask[i]) != filter->dmac[i]) {
                    goto no_match;
                }
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_SMAC) {
            for (i = 0; i < sizeof(filter->smac); i++) {
                if ((packet->frm_ptr[SMAC_POS + i] & filter->smac_mask[i]) != filter->smac[i]) {
                    goto no_match;
                }
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_ETYPE) {
            if ((etype & filter->etype_mask) != filter->etype) {
                goto no_match;
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) {
            // No need to check for ETYPE == ETYPE_IPV4, since it's already embedded in
            // the filter (was added automatically in the RX_filter_insert()
            // function).
            if ((packet->frm_ptr[IPV4_PROTO_POS] & filter->ipv4_proto_mask) != filter->ipv4_proto) {
                goto no_match;
            }

            if ((match & PACKET_RX_FILTER_MATCH_UDP_SRC_PORT)) {
                u16 hlen, udp_src_port;
                hlen = packet->frm_ptr[IPV4_HLEN_POS] & 0xF;
                hlen *= 4; // Now in bytes

                udp_src_port = (packet->frm_ptr[UDP_SRC_PORT_POS(hlen)] << 8) | (packet->frm_ptr[UDP_SRC_PORT_POS(hlen) + 1]);
                if (udp_src_port < filter->udp_src_port_min || udp_src_port > filter->udp_src_port_max) {
                    goto no_match;
                }
            }

            if ((match & PACKET_RX_FILTER_MATCH_UDP_DST_PORT)) {
                u16 hlen, udp_dst_port;
                hlen = packet->frm_ptr[IPV4_HLEN_POS] & 0xF;
                hlen *= 4; // Now in bytes

                udp_dst_port = (packet->frm_ptr[UDP_DST_PORT_POS(hlen)] << 8) | (packet->frm_ptr[UDP_DST_PORT_POS(hlen) + 1]);
                if (udp_dst_port < filter->udp_dst_port_min || udp_dst_port > filter->udp_dst_port_max) {
                    goto no_match;
                }
            }

            if ((match & PACKET_RX_FILTER_MATCH_TCP_SRC_PORT)) {
                u16 hlen, tcp_src_port;
                hlen = packet->frm_ptr[IPV4_HLEN_POS] & 0xF;
                hlen *= 4; // Now in bytes

                tcp_src_port = (packet->frm_ptr[TCP_SRC_PORT_POS(hlen)] << 8) | (packet->frm_ptr[TCP_SRC_PORT_POS(hlen) + 1]);
                if (tcp_src_port < filter->tcp_src_port_min || tcp_src_port > filter->tcp_src_port_max) {
                    goto no_match;
                }
            }

            if ((match & PACKET_RX_FILTER_MATCH_TCP_DST_PORT)) {
                u16 hlen, tcp_dst_port;
                hlen = packet->frm_ptr[IPV4_HLEN_POS] & 0xF;
                hlen *= 4; // Now in bytes

                tcp_dst_port = (packet->frm_ptr[TCP_DST_PORT_POS(hlen)] << 8) | (packet->frm_ptr[TCP_DST_PORT_POS(hlen) + 1]);
                if (tcp_dst_port < filter->tcp_dst_port_min || tcp_dst_port > filter->tcp_dst_port_max) {
                    goto no_match;
                }
            }
        }

        if (match & PACKET_RX_FILTER_MATCH_IP_ANY) {
            if (!is_ip_any) {
                goto no_match;
            }
        }

#if defined(VTSS_SW_OPTION_SFLOW)
        if (match & PACKET_RX_FILTER_MATCH_SFLOW) {
            if (rx_info->sflow_type == VTSS_SFLOW_TYPE_NONE) {
                goto no_match;
            }
        }
#endif /* VTSS_SW_OPTION_SFLOW */

#if VTSS_SWITCH_STACKABLE
        if (match & PACKET_RX_FILTER_MATCH_SSPID) {
            // No need to check for ETYPE == VTSS_ETYPE_VTSS, since it's already embedded in
            // the filter (was added automatically in the RX_filter_insert() function).
            u16 epid = CX_ntohs(&packet->frm_ptr[EPID_POS]);
            u16 sspid;

            if (epid != SSP_PROT_EPID) {
                goto no_match; // Not a Switch Stacking Protocol frame.
            }

            sspid = CX_ntohs(&packet->frm_ptr[SSPID_POS]);
            if ((sspid & filter->sspid_mask) != filter->sspid) {
                goto no_match;
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */

        if (match & PACKET_RX_FILTER_MATCH_DEFAULT) {
            // Skip this one for now. It's only used if no other
            // filters matched.
            match_default = TRUE;
            goto no_match;
        }

        // If we get here, it's either because the match filter was empty,
        // i.e. match == PACKET_RX_FILTER_MATCH_ANY, or the filter matched
        // all the way through.
        at_least_one_match = TRUE;

        callback_result = RX_do_callback(l, packet->frm_ptr, packet);
        if (callback_result == 1) {
            // Packet consumed.
            break;
        } else if (callback_result == 2) {
            // Packet consumed and called-back function has claimed the DMA buffers.
            detach_buffers = TRUE;
            break;
        }
no_match:
        continue;
    }

    // If the frame didn't match any filters and a least one default consumer
    // was found, we gotta run through it all again.
    if (match_default && !at_least_one_match) {
        for (filter_idx = 0; filter_idx < local_filter_list_len; filter_idx++) {
            l = &local_filter_list[filter_idx];

            if (l->filter.match & PACKET_RX_FILTER_MATCH_DEFAULT) {
                at_least_one_match = TRUE;

                callback_result = RX_do_callback(l, packet->frm_ptr, packet);
                if (callback_result == 1) {
                    // Packet consumed.
                    break;
                } else if (callback_result == 2) {
                    // Packet consumed and called-back function has claimed the DMA buffers.
                    detach_buffers = TRUE;
                    break;
                }
            }
        }
    }

    // Update "no-subscriber" statistics
    if (!at_least_one_match) {
        rx_bytes_no_subscribers += rx_info->length;
        rx_pkts_no_subscribers++;
    }

    return detach_buffers;
}

/****************************************************************************/
// RX_thread()
// This is the entry point for the packet dispatcher, which receives packets
// from the DSR context (RX_fdma_packet) and dispatches them to subscribers
// in a thread context. The function runs indefinitely and in thread context.
/****************************************************************************/
static void RX_thread(cyg_addrword_t data)
{
    vtss_fdma_list_t *packet;

    while (1) {
        // Wait for packets in the Rx FIFO.

        // Prevent the DSR (RX_fdma_packet()) from running
        cyg_scheduler_lock();

        // If there're no frames in the FIFO, re-allow the scheduler
        // to schedule (a.o.) DSRs and wait for RX_fdma_packet()
        // to signal the semaphore, indicating that (at least) one
        // packet is ready.
        while (vtss_fifo_cnt(&RX_fifo) == 0) {
            cyg_scheduler_unlock();
            (void)cyg_flag_wait(&RX_flag, 1, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR); // This waits for bit 0 in the flags to become 1 and clears them when that happens
            cyg_scheduler_lock();
        }

        // When we get here we the scheduler is locked,
        // so that the DSR is prevented from running.
        // Get the next frame in the FIFO.
        packet = (vtss_fdma_list_t *)vtss_fifo_rd(&RX_fifo);

        // Now that we've gotten the frame and updated pointers,
        // allow the scheduler to switch tasks again.
        cyg_scheduler_unlock();

        if (!packet) {
            continue;
        }

        // Dispatch packet to subscribers.
        if (!RX_dispatch(packet)) {
            // If RX_dispatch() returns FALSE, we must send back the buffer to the
            // FDMA for re-use. Otherwise it's up to the called back function (cb_adv)
            // to release them.
            PACKET_CHECK(vtss_fdma_dcb_release(0, packet) == VTSS_RC_OK,);
        }
    }
}

/****************************************************************************/
// TX_release_dcbs()
/****************************************************************************/
static void TX_release_dcbs(packet_tx_dcb_t *packet_dcbs)
{
    packet_tx_dcb_t *eof_dcb;

    PACKET_CHECK(packet_dcbs, return;);

    eof_dcb = packet_dcbs;
    while (eof_dcb->next) {
        eof_dcb = eof_dcb->next;
    }

    (void)cyg_mutex_lock(&TX_dcb_mutex);
    if (TX_dcb_head) {
        eof_dcb->next = TX_dcb_head;
    }
    TX_dcb_head = packet_dcbs;

    // Signal threads waiting for resources
    cyg_cond_signal(&TX_dcb_cond);
    cyg_mutex_unlock(&TX_dcb_mutex);
}

/****************************************************************************/
// TX_alloc_dcb()
// This function obtains dcb_cnt FDMA DCBs and one Packet module DCB.
// The packet module DCB is saved in the SOF FDMA DCB's #user member.
// The function is blocking, and therefore only returns when the DCBs have been
// obtained.
/****************************************************************************/
static PACKET_INLINE vtss_fdma_list_t *TX_alloc_dcb(u32 dcb_cnt, vtss_fdma_dcb_type_t dcb_type)
{
    vtss_rc          rc;
    vtss_fdma_list_t *list;
    packet_tx_dcb_t  *packet_dcb;

    // Get the mutex.
    (void)cyg_mutex_lock(&TX_dcb_mutex);

    // The TX_dcb_cond condition can be signalled from both the TX_release_dcbs()
    // and TX_fdma_done() functions.
    // It gets signalled in TX_release_dcbs() when the user tx_done()
    // has been called back, meaning that there now are new Packet Module DCBs.
    // It gets signalled from TX_fdma_done() when the FDMA has called us back,
    // meaning that there are now new FDMA DCBs.

    // Get FDMA DCB(s)
    while ((rc = vtss_fdma_dcb_get(NULL, dcb_cnt, dcb_type, &list)) == VTSS_RC_INCOMPLETE) {
        // Out of FDMA DCBs. Wait for the FDMA to release some.
        (void)cyg_cond_wait(&TX_dcb_cond);
    }

    // Mutex is locked here.

    if (rc != VTSS_RC_OK) {
        T_E("Internal error");
        cyg_mutex_unlock(&TX_dcb_mutex);
        return NULL;
    }

    while (TX_dcb_head == NULL) {
        // Out of Packet Module DCBs. Wait for it to release some.
        (void)cyg_cond_wait(&TX_dcb_cond);
    }

    // Mutex is locked here.

    if (TX_dcb_head == NULL) {
        T_E("Internal error");
        cyg_mutex_unlock(&TX_dcb_mutex);
        return NULL;
    }

    packet_dcb  = TX_dcb_head;
    TX_dcb_head = TX_dcb_head->next;
    list->user  = packet_dcb;

    cyg_mutex_unlock(&TX_dcb_mutex);

    memset(packet_dcb, 0, sizeof(*packet_dcb));
    return list;
}

/****************************************************************************/
// TX_done_thread()
// This is the thread context counter-part of the TX_fdma_done() DSR function,
// which cannot signal back to the callers of packet_tx() by itself, because
// this must be done in thread context rather than DSR context, because the
// called back functions may wait for semaphores or mutexes, which is not
// allowed in DSR context.
/****************************************************************************/
static void TX_done_thread(cyg_addrword_t data)
{
    packet_tx_dcb_t        *packet_dcb;
    void                   *context;
    packet_tx_done_cb_t    cb;
    packet_tx_done_props_t tx_done_props;

    while (1) {
        // Wait for packets in the Tx Done FIFO.

        // Prevent the DSR (TX_fdma_done()) from running
        cyg_scheduler_lock();

        // If there're no frames in the FIFO, re-allow the scheduler
        // to schedule (a.o.) DSRs and wait for TX_fdma_done()
        // to signal the semaphore, indicating that (at least) one
        // packet is ready.
        while (vtss_fifo_cnt(&TX_done_fifo) == 0) {
            cyg_scheduler_unlock();
            (void)cyg_flag_wait(&TX_done_flag, 1, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR); // This waits for bit 0 in the flags to become 1 and clears them when that happens
            cyg_scheduler_lock();
        }

        // When we get here we the scheduler is locked,
        // so that the DSR is prevented from running.
        // Get the next frame in the FIFO. The items in the FIFO
        // are of type packet_tx_dcb_t *.
        packet_dcb = (packet_tx_dcb_t *)vtss_fifo_rd(&TX_done_fifo);

        // Now that we've gotten the frame and updated pointers,
        // allow the scheduler to switch tasks again.
        cyg_scheduler_unlock();

        if (!packet_dcb) {
            T_E("Internal error");
            continue;
        }

        if (!packet_dcb->tx_done_props.tx) {
            // Can't really say much more.
            T_W("Dropped a packet");
        }

        T_DG(TRACE_GRP_TX_DONE, "Done");

        // Before calling back, save what is needed in local variables and
        // release the DCBs. This is needed in order to avoid a deadlock, which
        // may occur if the called back function attempts to transmit a new frame
        // with the blocking option!
        tx_done_props         = packet_dcb->tx_done_props;
        tx_done_props.frm_cnt = tx_done_props.tx ? (packet_dcb->afi ? packet_dcb->tx_done_props.frm_cnt : 1) : 0;
        cb                    = packet_dcb->tx_done_cb;
        context               = packet_dcb->tx_done_cb_context;

        if (packet_dcb->afi && tx_done_props.tx == FALSE) {
            // If tx_result is not OK for AFI, then the trace error is likely to have been copied to the
            // trace ring buffer, because it's likely that the FDMA driver has disabled interrupts in which case
            // the trace error can't go to the console.
            // Print an error here so that the user can get the real error message.
            T_EG(TRACE_GRP_TX_DONE, "An error occurred during AFI injection. If not already printed, use \"/Debug Trace RingBuffer Print\" to see it.");
        }

        TX_release_dcbs(packet_dcb);

        if (cb) {
            // If the user specifies a callback function, he must also handle the freeing.
            cb(context, &tx_done_props);
        } else {
            // The user has not specified a callback function, so we must handle the freeing.
            // @data_ptr must point to first byte of DMAC of frame before calling packet_tx_free().
            packet_tx_free(tx_done_props.frm_ptr[0]);
        }
    }
}

/****************************************************************************/
// TX_push_tx_done_fifo()
/****************************************************************************/
static void TX_push_tx_done_fifo(packet_tx_dcb_t *packet_dcb)
{
    // Store the packet DCB in the TX_done_fifo.
    // It is completely safe to do it without any mutexes around the call, because
    // this function is un-interruptible (called in DSR context).
    if (vtss_fifo_wr(&TX_done_fifo, packet_dcb) == VTSS_OK) {
        // The frame is now stored in the FIFO.
        // Signal the TX flag to wake up TX_done_thread().
        cyg_flag_setbits(&TX_done_flag, 1);
    } else {
        // It's a major flaw if we couldn't store the frame in the fifo
        // because that means that we can never release the DCB and
        // associated data area (it's unsafe to do it from DSR context).
        /*lint -e{506} */
        PACKET_CHECK(FALSE, return;);
    }
}

/****************************************************************************/
// Called back by FDMA when injection is complete.
// Context: DSR
// Synchronize to thread context, so that the callback function can release
// buffers and call mutexes as it pleases.
// list->user points to a packet_tx_dcb_t structure.
/****************************************************************************/
static void TX_fdma_done(void *contxt, vtss_fdma_list_t *list, vtss_rc rc)
{
    packet_tx_dcb_t *packet_dcb = list->user;
    if (packet_dcb != NULL) {
        // Get various output parameters before returning, because #list gets recycled
        // as soon as we return from this function.
        packet_dcb->tx_done_props.tx                       = rc == VTSS_RC_OK ? TRUE : FALSE;
        packet_dcb->tx_done_props.sw_tstamp                = list->sw_tstamp;
#if defined(VTSS_FEATURE_AFI_FDMA)
        packet_dcb->tx_done_props.frm_cnt                  = rc == VTSS_RC_OK ? list->afi_frm_cnt        : 0;
        packet_dcb->tx_done_props.afi_last_sequence_number = rc == VTSS_RC_OK ? list->afi_seq_number - 1 : 0;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    }

    // Even if packet_dcb should be NULL, put it in the FIFO, so that we can print an error when it
    // gets to thread context.
    TX_push_tx_done_fifo(packet_dcb);

    // And wake up the pending thread, which might wait for injection DCBs,
    // which will be released by the FDMA when we return here.
    cyg_cond_signal(&TX_dcb_cond);
}

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// TX_afi_done()
// This function is only here to ease debugging. One could as well just have
// specified TX_fdma_done as the callback function when configuring the FDMA.
/****************************************************************************/
static void TX_afi_done(void *contxt, vtss_fdma_list_t *list, vtss_rc rc)
{
    TX_fdma_done(contxt, list, rc);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

/****************************************************************************/
// Always called from TX_pend_thread context.
// Allows for a user-defined pre-tx callback.
/****************************************************************************/
static void TX_pre_tx_cb(void *ctxt1, void *ctxt2, vtss_fdma_list_t *list)
{
    packet_tx_pre_cb_t tx_pre_cb = ctxt1;
    if (tx_pre_cb) {
        tx_pre_cb(ctxt2, list->frm_ptr, list->act_len);
    }
}

/****************************************************************************/
// Always called from TX_pend_thread context. May block if no DCBs available.
/****************************************************************************/
static void TX_do_tx(packet_tx_props_t *tx_props)
{
    vtss_fdma_list_t         *fdma_dcbs;
    packet_tx_dcb_t          *packet_dcb;
    packet_module_counters_t *cntrs = &CX_module_counters[tx_props->packet_info.modid];
    vtss_fdma_dcb_type_t     dcb_type = VTSS_FDMA_DCB_TYPE_INJ;

    if (CX_stack_trace_ena || tx_props->packet_info.frm[0][ETYPE_POS] != 0x88 || tx_props->packet_info.frm[0][ETYPE_POS + 1] != 0x80) {
        T_NG(    TRACE_GRP_TX, "Port mask = 0x%llx, length = %zu", tx_props->tx_info.dst_port_mask, tx_props->packet_info.len[0] + tx_props->packet_info.len[1]);
        T_NG(    TRACE_GRP_TX, "Packet (first %zu bytes shown):", MIN(96, tx_props->packet_info.len[0]));
        T_NG_HEX(TRACE_GRP_TX, tx_props->packet_info.frm[0], MIN(96, tx_props->packet_info.len[0]));
        if (tx_props->packet_info.frm[1]) {
            T_NG_HEX(TRACE_GRP_TX, tx_props->packet_info.frm[1], MIN(96, tx_props->packet_info.len[1]));
        }
    }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (tx_props->packet_info.internal_flags & PACKET_INTERNAL_FLAGS_AFI_CANCEL) {
        // If it fails, the frame was not found.
        PACKET_CHECK(vtss_fdma_afi_cancel(NULL, tx_props->packet_info.frm[0]) == VTSS_RC_OK, return;);
        return; // Done.
    }

    if (tx_props->fdma_info.afi_fps > 0) {
        dcb_type = VTSS_FDMA_DCB_TYPE_AFI;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
    fdma_dcbs = TX_alloc_dcb(tx_props->packet_info.frm[1] == NULL ? 1 : 2, dcb_type);
    if (fdma_dcbs == NULL || fdma_dcbs->user == NULL) {
        return; // Internal error. Error already printed.
    }

    // Save info that has to survive until the user's Tx done gets called back.
    packet_dcb = (packet_tx_dcb_t *)fdma_dcbs->user;
    packet_dcb->tx_done_props.frm_ptr[0] = tx_props->packet_info.frm[0];
    packet_dcb->tx_done_props.frm_ptr[1] = tx_props->packet_info.frm[1];
    packet_dcb->tx_done_cb               = tx_props->packet_info.tx_done_cb;
    packet_dcb->tx_done_cb_context       = tx_props->packet_info.tx_done_cb_context;

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (tx_props->fdma_info.afi_fps > 0) {
        packet_dcb->afi = TRUE;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    // Data. The user must have used packet_tx_alloc() to allocate frm[0], which
    // ensures that there is VTSS_FDMA_HDR_SIZE_BYTES bytes ahead of frm1 for the FDMA
    // to use. frm[1] does not need to have VTSS_FDMA_HDR_SIZE_BYTES ahead of it.
    fdma_dcbs->frm_ptr = tx_props->packet_info.frm[0];
    fdma_dcbs->act_len = tx_props->packet_info.len[0];
    if (tx_props->packet_info.frm[1]) {
        fdma_dcbs->next->frm_ptr = tx_props->packet_info.frm[1];
        fdma_dcbs->next->act_len = tx_props->packet_info.len[1];
    }

    // Take over any pre-Tx callback.
    if (tx_props->packet_info.tx_pre_cb != NULL) {
        tx_props->fdma_info.pre_cb_ctxt1 = tx_props->packet_info.tx_pre_cb;
        tx_props->fdma_info.pre_cb_ctxt2 = tx_props->packet_info.tx_pre_cb_context;
        tx_props->fdma_info.pre_cb       = TX_pre_tx_cb;
    }

    cntrs->tx_bytes += (tx_props->packet_info.len[0] + tx_props->packet_info.len[1]);
    cntrs->tx_pkts++;

    // Initiate the injection.
    if (vtss_fdma_tx(0, fdma_dcbs, &tx_props->fdma_info, &tx_props->tx_info) != VTSS_RC_OK) {
        // Gotta free packet_dcb by pushing the frame onto the TX_done_fifo.
        // The FDMA dcbs get auto-released by the FDMA driver if it returns
        // a value different from VTSS_RC_OK.
        packet_dcb->tx_done_props.tx = 0;

        // The following call is not thread safe, so we better lock the scheduler before doing it.
        cyg_scheduler_lock();
        TX_push_tx_done_fifo(packet_dcb);
        cyg_scheduler_unlock();
    }
}

/****************************************************************************/
// Thread that is woken up on two occassions:
//   1) If a frame is scheduled for transmission by packet_tx() and there
//      are free DCBs.
//   2) If a Tx DCB is freed and at least one frame has been previously
//      scheduled for transmission by packet_tx()
//
// Due to the way that eCos' BSD IP stack is made, the packet_tx() must
// be unblocking *always*, or we may end up in a deadlock, because its
// splx_mutex cannot be released. Therefore, a separate thread for
// transmitting frames is needed. The packet_tx() function may be called
// from any thread, and it never blocks. It causes this thread to wake
// up and get a DCB (blocking or non-blocking as specified in options).
// Since it's a completely separate thread, the block operation doesn't
// hurt the IP stack.
/****************************************************************************/
static void TX_pend_thread(cyg_addrword_t data)
{
    packet_tx_props_t tx_props;

    (void)cyg_mutex_lock(&TX_pend_mutex);

    while (1) {
        // When we get here, we must have the TX_pend_mutex.

        // First we need to wait for a frame.
        if (vtss_fifo_cp_cnt(&TX_pend_fifo) == 0) {
            (void)cyg_cond_wait(&TX_pend_cond); // Unlocks the TX_pend_mutex and waits for condition variable to be signaled, at which point in time it locks the mutex again.
        }
        // Mutex is locked at this point in time.

        // We need to service all frames in FIFO before going back to the sleeping state.
        while (vtss_fifo_cp_cnt(&TX_pend_fifo) != 0) {
            if (vtss_fifo_cp_rd(&TX_pend_fifo, &tx_props) != VTSS_OK) {
                VTSS_ASSERT(FALSE);
            }
            cyg_mutex_unlock(&TX_pend_mutex);

            // When injecting the frame or waiting for a DCB, we should not have the TX_pend_mutex.
            TX_do_tx(&tx_props);

            // We must lock the FIFO before returning to check if it's empty.
            (void)cyg_mutex_lock(&TX_pend_mutex);
        }

        // Mutex is locked at this point in time.
    }
}

/****************************************************************************/
// DBG_cmd_stat_module_print()
// cmd_text   : "Print per-module statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_module_print(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    u32 tx_fifo_max_cnt, tx_fifo_total_cnt, tx_fifo_cur_cnt, tx_fifo_cur_sz, tx_fifo_overruns, tx_fifo_underruns;
    u32 rx_fifo_max_cnt, rx_fifo_total_cnt, rx_fifo_cur_cnt, rx_fifo_cur_sz, rx_fifo_overruns, rx_fifo_underruns;
    u32 a, f;
    int i;

    // Statistics
    (void)dbg_printf("\nModule              Rx Pkts    Rx Bytes     Tx Pkts    Tx Bytes     Max Rx Callback [ms]\n");
    (void)dbg_printf(  "------------------- ---------- ------------ ---------- ------------ --------------------\n");

    PACKET_RX_FILTER_CRIT_ENTER();

    for (i = 0; i <= VTSS_MODULE_ID_NONE; i++) {
        packet_module_counters_t *cntrs = &CX_module_counters[i];
        if (cntrs->rx_pkts != 0 || cntrs->tx_pkts != 0) {
            (void)dbg_printf("%-19s %10u %12llu %10u %12llu %20llu\n", vtss_module_names[i], cntrs->rx_pkts, cntrs->rx_bytes, cntrs->tx_pkts, cntrs->tx_bytes, cntrs->longest_rx_callback_ticks * ECOS_MSECS_PER_HWTICK);
        }
    }
    (void)dbg_printf("%-19s %10u %12llu\n\n", "<no subscriber>", rx_pkts_no_subscribers, rx_bytes_no_subscribers);

    PACKET_RX_FILTER_CRIT_EXIT();

    // TX FIFO counters
    (void)cyg_mutex_lock(&TX_pend_mutex);
    vtss_fifo_get_statistics(&RX_fifo, &rx_fifo_max_cnt, &rx_fifo_total_cnt, &rx_fifo_cur_cnt, &rx_fifo_cur_sz, &rx_fifo_overruns, &rx_fifo_underruns);
    vtss_fifo_cp_get_statistics(&TX_pend_fifo, &tx_fifo_max_cnt, &tx_fifo_total_cnt, &tx_fifo_cur_cnt, &tx_fifo_cur_sz, &tx_fifo_overruns, &tx_fifo_underruns);
    cyg_mutex_unlock(&TX_pend_mutex);

    (void)dbg_printf("          RX FIFO    TX FIFO\n");
    (void)dbg_printf("--------- ---------- ----------\n");
    (void)dbg_printf("Max       %10u %10u\n",   rx_fifo_max_cnt,   tx_fifo_max_cnt);
    (void)dbg_printf("Total     %10u %10u\n",   rx_fifo_total_cnt, tx_fifo_total_cnt);
    (void)dbg_printf("Cur Count %10u %10u\n",   rx_fifo_cur_cnt,   tx_fifo_cur_cnt);
    (void)dbg_printf("Cur Size  %10u %10u\n",   rx_fifo_cur_sz,    tx_fifo_cur_sz);
    (void)dbg_printf("Overruns  %10u %10u\n",   rx_fifo_overruns,  tx_fifo_overruns);
    (void)dbg_printf("Underruns %10u %10u\n\n", rx_fifo_underruns, tx_fifo_underruns);

    cyg_scheduler_lock();
    a = TX_alloc_calls;
    f = TX_free_calls;
    cyg_scheduler_unlock();

    (void)dbg_printf("Tx packet buffers:\n");
    (void)dbg_printf("Allocations  : %10u\n", a);
    (void)dbg_printf("Deallocations: %10u\n", f);
    (void)dbg_printf("Outstanding  : %10u\n\n", a - f);
}

/****************************************************************************/
// DBG_cmd_stat_fdma_print()
// cmd_text   : "Print FDMA statistics"
// arg_syntax : NULL,
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_fdma_print(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    (void)dbg_printf("Obsolete. Use 'debug api fdma'\n");
}

/****************************************************************************/
// DBG_cmd_stat_port_print()
// cmd_text   : "Print port statistics"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_port_print(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int iport, prio;

    // Port Statistics
    (void)dbg_printf("\nPort      ");
    for (prio = 0; prio < VTSS_PRIOS; prio++) {
        (void)dbg_printf("Rx Prio %d  ", prio);
    }
    (void)dbg_printf("Rx Super   Tx\n");

    (void)dbg_printf("--------- ");
    for (prio = 0; prio <= VTSS_PRIOS + 1; prio++) {
        (void)dbg_printf("---------- ");
    }
    (void)dbg_printf("\n");

    PACKET_RX_FILTER_CRIT_ENTER();

    for (iport = 0; iport <= VTSS_PORTS; iport++) {
        u32 idx = iport;
        if (iport == VTSS_PORTS) {
            idx = VTSS_PORTS + VTSS_GLAGS;
            (void)dbg_printf("Unknown   "); // Typically sFlow frames.
        } else {
            (void)dbg_printf("%9u ", iport2uport(iport));
        }
        for (prio = 0; prio <= VTSS_PRIOS; prio++) {
            (void)dbg_printf("%10u ", CX_port_counters.rx_pkts[idx][prio]);
        }
        if (iport == VTSS_PORTS) {
            (void)dbg_printf("%10s ", "N/A");
        } else {
            (void)dbg_printf("%10u ", CX_port_counters.tx_pkts[iport]);
        }
        (void)dbg_printf("\n");
    }
    (void)dbg_printf("Switched  ");
    for (prio = 0; prio <= VTSS_PRIOS; prio++) {
        (void)dbg_printf("%10s ", "N/A");
    }
    (void)dbg_printf("%10u ", CX_port_counters.tx_pkts[VTSS_PORTS]);
    (void)dbg_printf("\nMulticast ");
    for (prio = 0; prio <= VTSS_PRIOS; prio++) {
        (void)dbg_printf("%10s ", "N/A");
    }
    (void)dbg_printf("%10u ", CX_port_counters.tx_pkts[VTSS_PORTS + 1]);
#if VTSS_SWITCH_STACKABLE
    for (iport = VTSS_GLAG_NO_START; iport < VTSS_GLAG_NO_START + VTSS_GLAGS; iport++) {
        (void)dbg_printf("\nGLAG #%-2d  ", iport);
        for (prio = 0; prio <= VTSS_PRIOS; prio++) {
            (void)dbg_printf("%10u ", CX_port_counters.rx_pkts[iport - VTSS_GLAG_NO_START + VTSS_PORTS][prio]);
        }
        (void)dbg_printf("%10s ", "N/A");
    }
#endif /* VTSS_SWITCH_STACKABLE */
    (void)dbg_printf("\n");
    PACKET_RX_FILTER_CRIT_EXIT();
}

/******************************************************************************/
// RX_subscriber_match_str()
/******************************************************************************/
char *RX_subscriber_match_str(char *buf, size_t size, u32 match)
{
    int cnt = 0;

    if (!buf || size == 0) {
        return NULL;
    }

#define RX_MATCH_STR(_str_) do {cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%s" vtss_xstr(_str_), cnt != 0 ? ", " : "");} while (0);

    buf[0] = '\0';

    if (match == 0) {
        RX_MATCH_STR(Any);
    }
    if (match & PACKET_RX_FILTER_MATCH_SRC_PORT) {
        RX_MATCH_STR(Ingress Port);
    }
    if (match & PACKET_RX_FILTER_MATCH_ACL) {
        RX_MATCH_STR(ACL);
    }
    if (match & PACKET_RX_FILTER_MATCH_VID) {
        RX_MATCH_STR(VID);
    }
    if (match & PACKET_RX_FILTER_MATCH_DMAC) {
        RX_MATCH_STR(DMAC);
    }
    if (match & PACKET_RX_FILTER_MATCH_SMAC) {
        RX_MATCH_STR(SMAC);
    }
    if (match & PACKET_RX_FILTER_MATCH_ETYPE) {
        RX_MATCH_STR(EtherType);
    }
    if (match & PACKET_RX_FILTER_MATCH_IPV4_PROTO) {
        RX_MATCH_STR(IPv4 Protocol);
    }
    if (match & PACKET_RX_FILTER_MATCH_SSPID) {
        RX_MATCH_STR(SSPID);
    }
    if (match & PACKET_RX_FILTER_MATCH_UDP_SRC_PORT) {
        RX_MATCH_STR(UDP Src. Port);
    }
    if (match & PACKET_RX_FILTER_MATCH_UDP_DST_PORT) {
        RX_MATCH_STR(UDP Dest. Port);
    }
    if (match & PACKET_RX_FILTER_MATCH_TCP_SRC_PORT) {
        RX_MATCH_STR(TCP Src. Port);
    }
    if (match & PACKET_RX_FILTER_MATCH_TCP_DST_PORT) {
        RX_MATCH_STR(TCP Dest. Port);
    }
    if (match & PACKET_RX_FILTER_MATCH_SFLOW) {
        RX_MATCH_STR(sFlow);
    }
    if (match & PACKET_RX_FILTER_MATCH_IP_ANY) {
        RX_MATCH_STR(IPv4 + IPv6 + ARP);
    }
    if (match & PACKET_RX_FILTER_MATCH_DEFAULT) {
        RX_MATCH_STR(Default);
    }

#undef RX_MATCH_STR

    if (cnt == 0) {
        (void)snprintf(buf, size, "<none>");
    }
    return buf;
}

/****************************************************************************/
// DBG_cmd_subscribers_print()
// cmd_text   : "Print subscriber list"
// arg_syntax : NULL
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_subscribers_print(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    packet_rx_filter_item_t *l;
    int cnt = 0;

    (void)dbg_printf("\nModule              Priority   Match\n");
    (void)dbg_printf(  "------------------- ---------- ---------------------------------------------\n");

    PACKET_RX_FILTER_CRIT_ENTER();
    l = RX_filter_list;
    while (l) {
        char buffer[200];
        (void)dbg_printf("%-19s %10u %s\n", vtss_module_names[l->filter.modid], l->filter.prio, RX_subscriber_match_str(buffer, sizeof(buffer), l->filter.match));
        cnt++;
        l = l->next;
    }

    if (cnt == 0) {
        (void)dbg_printf("<none>\n");
    }

    PACKET_RX_FILTER_CRIT_EXIT();
}

/****************************************************************************/
// DBG_cmd_stat_packet_clear()
// cmd_text   : "Clear per-module statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_packet_clear(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    // Clear Rx Statistics
    PACKET_RX_FILTER_CRIT_ENTER();

    memset(CX_module_counters, 0, sizeof(CX_module_counters));
    rx_pkts_no_subscribers  = 0;
    rx_bytes_no_subscribers = 0;

    PACKET_RX_FILTER_CRIT_EXIT();

    // RX and TX FIFO counters
    (void)cyg_mutex_lock(&TX_pend_mutex);
    vtss_fifo_clr_statistics(&RX_fifo);
    vtss_fifo_cp_clr_statistics(&TX_pend_fifo);
    cyg_mutex_unlock(&TX_pend_mutex);

    (void)dbg_printf("Packet module statistics cleared!\n");
}

/****************************************************************************/
// DBG_cmd_stat_fdma_clear()
// cmd_text   : "Clear FDMA statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_fdma_clear(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    PACKET_CHECK(vtss_fdma_stats_clr(0) == VTSS_RC_OK, return;);
    (void)dbg_printf("FDMA statistics cleared!\n");
}

/****************************************************************************/
// DBG_cmd_stat_port_clear()
// cmd_text   : "Clear port statistics",
// arg_syntax : NULL,
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_stat_port_clear(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    PACKET_RX_FILTER_CRIT_ENTER();
    memset(&CX_port_counters, 0, sizeof(CX_port_counters));
    PACKET_RX_FILTER_CRIT_EXIT();
    (void)dbg_printf("Port statistics cleared!\n");
}

/****************************************************************************/
// DCB_cmd_cfg_stack_trace()
// cmd_text   : "Enable or disable stack trace"
// arg_syntax : "0: Disable, 1: Enable"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_cfg_stack_trace(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    if (parms_cnt == 0) {
        (void)dbg_printf("Stack trace is currently %sabled\n", CX_stack_trace_ena ? "en" : "dis");
    } else if (parms_cnt == 1) {
        CX_stack_trace_ena = parms[0] != 0;
    } else {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 1 parameter, which must be 0 or 1");
    }
}

/****************************************************************************/
// DBG_cmd_cfg_signal_tx_pend_cond()
// cmd_text   : "Signal TX Pending Condition. USE WITH CARE!"
// arg_syntax : ""
// max_arg_cnt: 0
/****************************************************************************/
static void DBG_cmd_cfg_signal_tx_pend_cond(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    cyg_cond_signal(&TX_pend_cond);
}

/****************************************************************************/
// DBG_cmd_test_syslog()
// cmd_text   : "Generate error or fatal. This is only to test the SYSLOG, and has nothing to do with the packet module"
// arg_syntax : "0: Generate error, 1: Generate assert (never returns)"
// max_arg_cnt: 1
/****************************************************************************/
static void DBG_cmd_test_syslog(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    static int err_cnt = 0;

    /*lint -esym(459, DBG_cmd_test_syslog) unprotected access to err_cnt. Fine */
    if (parms_cnt == 1 && parms[0] == 0) {
        // Generate error
        T_E("Test Error #%d", ++err_cnt);
    } else if (parms_cnt == 1 && parms[0] == 1) {
        T_E("Generating Assertion");
        VTSS_ASSERT(FALSE);
    } else {
        DBG_cmd_syntax_error(dbg_printf, "This function takes 1 parameter, which must be 0 or 1");
    }
}

/****************************************************************************/
// CX_fdma_isr()
// Context:
//   ISR (upper half)
// Description:
//   Called back in ISR context whenever an FDMA interrupt occurs.
/****************************************************************************/
static cyg_uint32 CX_fdma_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);              // Block this interrupt until the dsr completes
    cyg_drv_interrupt_acknowledge(vector);       // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR); // Call the DSR
}

/****************************************************************************/
// CX_fdma_dsr()
// Context:
//   DSR (lower half)
// Description:
//   Called back in DSR context whenever an FDMA interrupt occurs.
/****************************************************************************/
static void CX_fdma_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Context switches to other DSRs or threads cannot occur within this function.
    // This behavior is required by the FDMA IRQ handler.
    PACKET_CHECK(vtss_fdma_irq_handler(0, NULL) == VTSS_RC_OK, return;);

    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(vector);
}

/****************************************************************************/
// CX_init()
/****************************************************************************/
static PACKET_INLINE void CX_init(void)
{
    // Initialize and register trace ressources
    VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
    VTSS_TRACE_REGISTER(&trace_reg);

    memset(&CX_port_counters,  0, sizeof(CX_port_counters));
    memset(CX_module_counters, 0, sizeof(CX_module_counters));
}

/****************************************************************************/
/****************************************************************************/
static PACKET_INLINE void TX_init(void)
{
    int i;

    // -------------------------oOo-------------------------
    // TX Pending stuff
    // -------------------------oOo-------------------------
    // Create a FIFO for transporting frames to be Txd from user thread context to Tx thread context,
    // so that every call to packet_tx() in essense becomes non-blocking.
    PACKET_CHECK(vtss_fifo_cp_init(&TX_pend_fifo, sizeof(packet_tx_props_t), TX_BUF_CNT, TX_BUF_CNT, 0, TRUE) == VTSS_OK, return;);

    // Initialize synchronization data
    cyg_mutex_init(&TX_pend_mutex);
    cyg_cond_init(&TX_pend_cond, &TX_pend_mutex);

    // Create thread that is woken up when DCBs are free to be matched to a pending frame.
    cyg_thread_create(THREAD_ABOVE_NORMAL_PRIO,
                      TX_pend_thread,
                      0,
                      "Packet TX Pending",
                      TX_pend_thread_stack,
                      sizeof(TX_pend_thread_stack),
                      &TX_pend_thread_handle,
                      &TX_pend_thread_state);

    // -------------------------oOo-------------------------
    // TX Done stuff
    // -------------------------oOo-------------------------
    // Create a FIFO for transporting transmitted frame buffers from DSR to thread context, so that
    // they can be de-allocated in thread context.
    if (vtss_fifo_init(&TX_done_fifo, TX_BUF_CNT, TX_BUF_CNT, 0, TRUE) != VTSS_OK) {
        /*lint -e{506} */
        PACKET_CHECK(FALSE, return;);
    }

    // Create a flag that can be signalled from DSR context and cleared from thread context.
    cyg_flag_init(&TX_done_flag);

    // Create thread that receives "tx done" messages and callback packet txers in thread context
    // The thread priority must be the same as the Tx thread's priority because some modules
    // (e.g. the IP stack) uses the Tx done to schedule another packet Tx.
    cyg_thread_create(THREAD_ABOVE_NORMAL_PRIO,
                      TX_done_thread,
                      0,
                      "Packet TX Done",
                      TX_done_thread_stack,
                      sizeof(TX_done_thread_stack),
                      &TX_done_thread_handle,
                      &TX_done_thread_state);

    // -------------------------oOo-------------------------
    // DCB stuff
    // -------------------------oOo-------------------------
    // Initialize synchronization data
    cyg_mutex_init(&TX_dcb_mutex);
    cyg_cond_init(&TX_dcb_cond, &TX_dcb_mutex);

    // Stitch together the TX DCBs
    for (i = 0; i < (TX_BUF_CNT - 1); i++) {
        TX_dcbs[i].next = &TX_dcbs[i + 1];
    }
    TX_dcb_head = &TX_dcbs[0];
}

/****************************************************************************/
// Initialize DSR->Thread FIFO and mutex/condition variables used in that
// synchronization, together with the thread itself.
/****************************************************************************/
static void RX_init(void)
{
    // Create a FIFO for transporting frames from DSR to thread context.
    // The size corresponds to the number of Rx buffer's we have asked the FDMA
    // to allocate.
    if (vtss_fifo_init(&RX_fifo, RX_BUF_CNT, RX_BUF_CNT, 0, TRUE) != VTSS_OK) {
        /*lint -e{506} */
        PACKET_CHECK(FALSE, return;);
    }

    // Critical region protecting the packet subscription filter list (initially locked).
    critd_init(&RX_filter_crit, "crit_packet_rx_filter", VTSS_MODULE_ID_PACKET, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    PACKET_RX_FILTER_CRIT_EXIT();

    // Critical region protecting other Rx configuration
    VTSS_OS_CRIT_CREATE(&CX_cfg_crit);

    // Create a flag that can be signalled from DSR context and cleared from thread context.
    cyg_flag_init(&RX_flag);

    // Create packet dispatcher thread. Cannot set the priority lower than DEFAULT
    // because all busy-waiters with higher priority (e.g. ping) will then time out.
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      RX_thread,
                      0,
                      "Packet RX",
                      RX_thread_stack,
                      sizeof(RX_thread_stack),
                      &RX_thread_handle,
                      &RX_thread_state);
}

#if !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// RX_throttle_tick()
/****************************************************************************/
static void RX_throttle_tick(cyg_handle_t alarm, cyg_addrword_t data)
{
    (void)vtss_fdma_throttle_tick(NULL);
}
#endif /* !defined(VTSS_ARCH_LUTON28) */

#if !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// RX_throttle_timer_set()
/****************************************************************************/
static void RX_throttle_timer_set(BOOL start)
{
    static cyg_handle_t RX_throttle_counter;
    static cyg_alarm    RX_throttle_alarm;
    static cyg_handle_t RX_throttle_alarm_handle;

    /*lint -esym(459, RX_throttle_counter)      Well, might fail if called simultaneously from two different CLI threads, but this is debug functionality!! */
    /*lint -esym(459, RX_throttle_alarm)        Well, might fail if called simultaneously from two different CLI threads, but this is debug functionality!! */
    /*lint -esym(459, RX_throttle_alarm_handle) Well, might fail if called simultaneously from two different CLI threads, but this is debug functionality!! */

    if (RX_throttle_alarm_handle == 0) {
        // Create the throttle tick timer
        cyg_clock_to_counter(cyg_real_time_clock(), &RX_throttle_counter);
        cyg_alarm_create(RX_throttle_counter,
                         RX_throttle_tick,
                         0,
                         &RX_throttle_alarm_handle,
                         &RX_throttle_alarm);
        cyg_alarm_initialize(RX_throttle_alarm_handle, cyg_current_time() + 1, PACKET_THROTTLE_PERIOD_MS / ECOS_MSECS_PER_HWTICK);
    }

    if (start) {
        cyg_alarm_enable(RX_throttle_alarm_handle);
    } else {
        cyg_alarm_disable(RX_throttle_alarm_handle);
    }
}
#endif /* !defined(VTSS_ARCH_LUTON28) */

#if !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// RX_throttle_init()
/****************************************************************************/
static PACKET_INLINE void RX_throttle_init(void)
{
    vtss_fdma_throttle_cfg_t throttle_cfg;

    memset(&throttle_cfg, 0, sizeof(throttle_cfg));

    // This will set the sFlow extraction queue to a maximum of 300 frames per second.
    // A suspend tick count of 0 means that it will extract up to 300 / FREQ_HZ frames per tick, then suspend and
    // re-open upon the next tick.
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_SFLOW]      = 300  /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_SFLOW]        = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

    // This will set the broadcast queue to a maximum of 500 frames per second.
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_BC]         = 500  /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_BC]           = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

#if PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_ACL_REDIR]  = 300  /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_ACL_REDIR]    = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;
#endif /* PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY */

    // This will set the management queue to a maximum of 3000 frames per second.
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_MGMT_MAC]   = 3000 /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_MGMT_MAC]     = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_L3_OTHER]   = 500  /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_L3_OTHER]     = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

#if defined(VTSS_SW_OPTION_MEP)
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_OAM]        = 1300 /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_OAM]          = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;
#endif /* VTSS_SW_OPTION_MEP */

    // This will set the BPDU/IGMP queue to a maximum of 1000 frames per second.
    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_BPDU]       = 1000 /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_BPDU]         = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

    throttle_cfg.frm_limit_per_tick[PACKET_XTR_QU_LRN_ALL]    = 500  /* frames per second */ / PACKET_THROTTLE_FREQ_HZ;
    throttle_cfg.suspend_tick_cnt[PACKET_XTR_QU_LRN_ALL]      = 0    /* milliseconds      */ / PACKET_THROTTLE_PERIOD_MS;

    PACKET_CHECK(packet_rx_throttle_cfg_set(&throttle_cfg) == VTSS_RC_OK,;);
}
#endif /* !defined(VTSS_ARCH_LUTON28) */

/****************************************************************************/
// CX_start_fdma()
/****************************************************************************/
static void CX_start_fdma(void)
{
    vtss_packet_rx_conf_t rx_conf;
    vtss_fdma_cfg_t       fdma_cfg;

#if defined(VTSS_ARCH_LUTON28)
    {
        // Additional Lu28 stuff.
        vtss_qos_conf_t qos;

        // Limit total CPU traffic using policers
        (void)vtss_qos_conf_get(0, &qos);
        qos.policer_mac = PACKET_XTR_POLICER_RATE;
        qos.policer_cat = PACKET_XTR_POLICER_RATE;
        // qos.policer_learn = PACKET_XTR_POLICER_RATE; If learn frames are sent to CPU
        PACKET_CHECK(vtss_qos_conf_set(0, &qos) == VTSS_RC_OK, return;);
    }
#endif /* defined(VTSS_ARCH_LUTON28) */

    memset(&fdma_cfg, 0, sizeof(fdma_cfg));

    // Cx
    fdma_cfg.enable      = TRUE;

    // Rx
    fdma_cfg.rx_mtu      = 1518 + 2 * 4; /* Max Etherframe + a couple of tags */
    fdma_cfg.rx_buf_cnt  = RX_BUF_CNT;
    fdma_cfg.rx_cb       = RX_fdma_packet;

    // Tx
    fdma_cfg.tx_buf_cnt  = TX_BUF_CNT;
    fdma_cfg.tx_done_cb  = TX_fdma_done;

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    // AFI
    fdma_cfg.afi_buf_cnt = AFI_BUF_CNT;
    fdma_cfg.afi_done_cb = TX_afi_done;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_SW_OPTION_BUILD_CE)
    // Some CE modules (in particular L2CP) use ingress VCAPs to get frames to
    // the CPU with e.g. S-tags even though the port is configured as a C-tagged port,
    // so we need to ask the FDMA driver not to discard such frames.
    fdma_cfg.rx_allow_vlan_tag_mismatch = TRUE;
#endif

    PACKET_CHECK(vtss_fdma_cfg(0, &fdma_cfg) == VTSS_RC_OK, return;);

#if !defined(VTSS_ARCH_LUTON28)
    RX_throttle_init();
#endif /* !defined(VTSS_ARCH_LUTON28) */

    // Lock for get-modify-set API operation
    vtss_appl_api_lock();

    // Get Rx packet configuration */
    PACKET_CHECK(vtss_packet_rx_conf_get(0, &rx_conf) == VTSS_RC_OK, goto do_exit;);

    // Setup Rx queue mapping */
    rx_conf.map.bpdu_queue      = PACKET_XTR_QU_BPDU;
    rx_conf.map.garp_queue      = PACKET_XTR_QU_BPDU;
    rx_conf.map.learn_queue     = PACKET_XTR_QU_LEARN;
    rx_conf.map.igmp_queue      = PACKET_XTR_QU_IGMP;
    rx_conf.map.ipmc_ctrl_queue = PACKET_XTR_QU_IGMP;
    rx_conf.map.mac_vid_queue   = PACKET_XTR_QU_MAC;
#if VTSS_SWITCH_STACKABLE
    rx_conf.map.stack_queue     = PACKET_XTR_QU_STACK;
#endif /* VTSS_SWITCH_STACAKBLE */
    rx_conf.map.lrn_all_queue   = PACKET_XTR_QU_LRN_ALL;
#if defined(VTSS_SW_OPTION_SFLOW)
    rx_conf.map.sflow_queue     = PACKET_XTR_QU_SFLOW;
#else
    // Do not change the sflow_queue.
#endif /* VTSS_SW_OPTION_SFLOW */

#if defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)
    rx_conf.map.l3_uc_queue     = PACKET_XTR_QU_MGMT_MAC;
    rx_conf.map.l3_other_queue  = PACKET_XTR_QU_L3_OTHER;
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/

    // Setup CPU queue sizes.
    rx_conf.queue[PACKET_XTR_QU_LOWEST  - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_LOWER   - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_LOW     - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_NORMAL  - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_MEDIUM  - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_HIGH    - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    rx_conf.queue[PACKET_XTR_QU_HIGHER  - VTSS_PACKET_RX_QUEUE_START].size = 12 * 1024;
    rx_conf.queue[PACKET_XTR_QU_HIGHEST - VTSS_PACKET_RX_QUEUE_START].size = 16 * 1024;

    // Set Rx packet configuration */
    PACKET_CHECK(vtss_packet_rx_conf_set(0, &rx_conf) == VTSS_RC_OK, goto do_exit;);

do_exit:
    // Unlock for get-modify-set API operation
    vtss_appl_api_unlock();
}

/****************************************************************************/
// CX_start_interrupts()
/****************************************************************************/
static void CX_start_interrupts(void)
{
    // Hook the FDMA interrupt
    cyg_drv_interrupt_create(
        FDMA_INTERRUPT,           // Interrupt Vector
        0,                        // Interrupt Priority
        (cyg_addrword_t)NULL,
        CX_fdma_isr,
        CX_fdma_dsr,
        &CX_fdma_intr_handle,
        &CX_fdma_intr_object);

    cyg_drv_interrupt_attach(CX_fdma_intr_handle);

    // Enable FDMA interrupts
    cyg_drv_interrupt_unmask(FDMA_INTERRUPT);

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    // Potentially a JAG_CU48 board.
    if (vtss_api_if_chip_count() == 2) {
        // It's definitely a JAG_CU48 board.
        // Since we have to manually extract frames from the secondary JR, we need to
        // hook us up to the secondary chip's extraction group interrupts.
        // Each extraction group that we use must be set-up separately. Luckily we only
        // use one (which is number 0).
        cyg_vector_t intr_vector = CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY0 + 0;
        cyg_drv_interrupt_create(
            intr_vector,            // Interrupt Vector
            0,                      // Interrupt Priority
            (cyg_addrword_t)NULL,   // Additional user data
            CX_fdma_isr,            // Re-use normal FDMA interrupt ISR
            CX_fdma_dsr,            // Re-use normal FDMA interrupt DSR
            &RX_secondary_intr_handle,
            &RX_secondary_intr_object
        );
        cyg_drv_interrupt_attach(RX_secondary_intr_handle);
        cyg_drv_interrupt_unmask(intr_vector);
    }
#endif /* defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR) */
}

/******************************************************************************/
// TX_pend_fifo_post()
// Store tx_props in Tx pending fifo.
/******************************************************************************/
static vtss_rc TX_pend_fifo_post(packet_tx_props_t *tx_props)
{
    (void)cyg_mutex_lock(&TX_pend_mutex);
    if (vtss_fifo_cp_wr(&TX_pend_fifo, tx_props) != VTSS_OK) {
        // FIFO is full!
        cyg_mutex_unlock(&TX_pend_mutex);
        return VTSS_RC_ERROR;
    }

    // Signal TX_pend_thread() that a new frame is here.
    cyg_cond_signal(&TX_pend_cond);
    cyg_mutex_unlock(&TX_pend_mutex);
    return VTSS_RC_OK;
}

/****************************************************************************
 * TX_mask_analyze()
 * Get the number of bits set in #mask.
 * Returns 0 if no bits are set, 1 if exactly one bit is set, 2 if at least
 * two bits are set.
 * If exactly one bit is set, bit_pos holds the bit position.
 ****************************************************************************/
u32 TX_mask_analyze(u64 mask, u32 *bit_pos)
{
    u32 i, w, p, cnt = 0;

    if (mask == 0) {
        return 0;
    }

    for (i = 0; i < 2; i ++) {
        w = (u32)(mask >> (32 * i));

        if ((p = VTSS_OS_CTZ(w)) < 32) {
            w &= ~VTSS_BIT(p);
            if (w) {
                // Still bits set in w.
                return 2;
            }
            cnt++;
            *bit_pos = p + 32 * i;
        }
    }

    return cnt > 1 ? 2 : cnt;
}

/****************************************************************************/
/*                                                                          */
/*  MODULE EXTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/******************************************************************************/
// packet_tx_props_init()
/******************************************************************************/
void packet_tx_props_init(packet_tx_props_t *tx_props)
{
    if (tx_props != NULL) {
        (void)vtss_packet_tx_info_init(NULL, &tx_props->tx_info);
        (void)vtss_fdma_tx_info_init(NULL, &tx_props->fdma_info);
        memset(&tx_props->packet_info, 0, sizeof(tx_props->packet_info));

        // Enforce modules to identify themselves.
        tx_props->packet_info.modid = VTSS_MODULE_ID_NONE;

        // Default to transmitting with highest priority.
        tx_props->tx_info.cos = msg_max_user_prio();
    }
}

/******************************************************************************/
// packet_tx()
/******************************************************************************/
vtss_rc packet_tx(packet_tx_props_t *tx_props)
{
    vtss_rc        result;
    vtss_etype_t   saved_tpid;
    u32            port_cnt = 0;
    vtss_port_no_t port_no  = VTSS_PORT_NO_NONE;

    // Sanity checks:

    // tx_props must not be NULL
    PACKET_CHECK(tx_props != NULL, return VTSS_RC_ERROR;);

    // modid must be within range
    PACKET_CHECK(tx_props->packet_info.modid <= VTSS_MODULE_ID_NONE, return VTSS_RC_ERROR;);

    // The first frame fragment must be specified.
    PACKET_CHECK(tx_props->packet_info.frm[0] != NULL, return VTSS_RC_ERROR;);

#if !defined(VTSS_ARCH_SERVAL)
    // Masquerading only supported on Serval.
    PACKET_CHECK(tx_props->tx_info.masquerade_port == VTSS_PORT_NO_NONE, return VTSS_RC_ERROR;);
#endif /* !defined(VTSS_ARCH_SERVAL) */

    if (tx_props->tx_info.switch_frm) {
#if VTSS_SWITCH_STACKABLE
        // Cannot send VStaX header frames switched.
        PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE, return VTSS_RC_ERROR;);
#endif /* VTSS_SWITCH_STACKABLE */

        // Super-prio injection not supported when switching frames.
        PACKET_CHECK(tx_props->tx_info.cos < 8, return VTSS_RC_ERROR;);

    } else {
        // Get info from the destination port mask.
        port_cnt = TX_mask_analyze(tx_props->tx_info.dst_port_mask, &port_no);

        // At least one bit must be set if not switching frame
        PACKET_CHECK(port_cnt >= 1, return VTSS_RC_ERROR;);

        // And if exactly one bit is set, it must be within valid port range.
        PACKET_CHECK(port_cnt > 1 || port_no < VTSS_PORTS, return VTSS_RC_ERROR;);

        // Frames must not be sent masqueraded when not switching.
        PACKET_CHECK(tx_props->tx_info.masquerade_port == VTSS_PORT_NO_NONE, return VTSS_RC_ERROR;);

#if VTSS_SWITCH_STACKABLE
        {
            vtss_port_no_t stack_left              = PORT_NO_STACK_0;
            BOOL           injecting_to_stack_port = stack_left != VTSS_PORT_NO_NONE /* stacking enabled */ && ((tx_props->tx_info.dst_port_mask & VTSS_BIT64(stack_left) /* left stack port selected */) || (tx_props->tx_info.dst_port_mask & VTSS_BIT64(PORT_NO_STACK_1) /* right stack port selected */));

            if (injecting_to_stack_port) {
                // Injecting to stack port
                // Exactly one bit must be set in the port mask
                PACKET_CHECK(port_cnt == 1, return VTSS_RC_ERROR;);

                // VStaX header must be specified.
                PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr != VTSS_PACKET_TX_VSTAX_NONE, return VTSS_RC_ERROR;);

            } else {
                // Not injecting to stack port. No VStaX header must be specified.
                PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE, return VTSS_RC_ERROR;);
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */

        if (tx_props->tx_info.cos == 8) {
            // With super-priority injection, frames must be directed towards a specific front port.
            PACKET_CHECK(port_cnt == 1, return VTSS_RC_ERROR;);
            // Also, due to a bypass of timestamping circuit in Jaguar, it's not possible to
            // have the frames timestamped on egress.
            PACKET_CHECK(tx_props->tx_info.latch_timestamp == 0, return VTSS_RC_ERROR;);
        }
    }

    // If the frame is sent switched without masquerading, then the VID must be non-zero unless masquerading, where it's optional.
    PACKET_CHECK(tx_props->tx_info.masquerade_port != VTSS_PORT_NO_NONE || tx_props->tx_info.switch_frm == FALSE || (tx_props->tx_info.tag.vid & 0xFFF) != 0, return VTSS_RC_ERROR;);

    // The QoS class must be in range [0; 8].
    PACKET_CHECK(tx_props->tx_info.cos <= 8, return VTSS_RC_ERROR;);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    PACKET_CHECK(tx_props->fdma_info.afi_fps <= VTSS_AFI_FPS_MAX, return VTSS_RC_ERROR;);
    if (tx_props->fdma_info.afi_fps > 0) {
        // If the frame must be sent repetitively, then
        //  ...the frame cannot be sent switched
        PACKET_CHECK(tx_props->tx_info.switch_frm == FALSE, return VTSS_RC_ERROR;);
        // ...the frame cannot be transmitted on several front ports
        PACKET_CHECK(port_cnt == 1, return VTSS_RC_ERROR;);
        // ...the frame cannot be sent with super priority
        PACKET_CHECK(tx_props->tx_info.cos < 8, return VTSS_RC_ERROR;);
        // ...the frame must be sent as one single fragment, because we need
        // to pre-allocate enough DCBs to handle all possible flows, but mainly
        // because the time-distance between DCBs is fixed and determined
        // by the channel's frequency. If inserting a frame with two DCBs,
        // the timing will slide and that frame will be cut into two time slots.
        PACKET_CHECK(tx_props->packet_info.frm[1] == NULL, return VTSS_RC_ERROR;);
#if defined(VTSS_FEATURE_AFI_FDMA)
        // If sequence numbering is enabled (only possible on FDMA-based AFI),
        // the update offset must reside four bytes before the end of the frame (excluding FCS).
        PACKET_CHECK(tx_props->fdma_info.afi_enable_sequence_numbering == FALSE || tx_props->fdma_info.afi_sequence_number_offset <= tx_props->packet_info.len[0] - 4 /* The size of the field to update */, return VTSS_RC_ERROR;);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
#if VTSS_SWITCH_STACKABLE
        // ...and the frame cannot be sent with a stack header.
        PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE, return VTSS_RC_ERROR;);
#endif /* VTSS_SWITCH_STACKABLE */
        // ...and the Pre-Tx callback function must be disabled.
        PACKET_CHECK(tx_props->packet_info.tx_pre_cb == NULL, return VTSS_RC_ERROR;);

        // These were the static checks. The non-static checks are made by TX_do_tx() and
        // the FDMA driver. These include DCB depletion, invalid multiplier (if frames
        // already being injected have a multiplier that is not a multiple of this frame's
        // multiplier or vice versa, etc.)
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    if (tx_props->packet_info.frm[1] != NULL) {
        // If the second frame fragment is specified, then...
        //   ...the tx_done_cb function must be specified, because the user has to
        //   determine how to deallocate the second fragment.
        PACKET_CHECK(tx_props->packet_info.tx_done_cb != NULL, return VTSS_RC_ERROR;);
        //   ...the first fragment's length must be >= 16 bytes and a multiple of
        //   4 bytes (for the sake of the FDMA).
        PACKET_CHECK(tx_props->packet_info.len[0] >= 14 && (tx_props->packet_info.len[0] & 0x3) == 0, return VTSS_RC_ERROR;);
        //   ...the second fragment's length must be > 0 bytes.
        PACKET_CHECK(tx_props->packet_info.len[1] > 0, return VTSS_RC_ERROR;);
        // Do not adjust the total length to become a minimum Ethernet-sized frame. This is done by the FDMA driver.
    } else {
        // The length must be >= 14 bytes.
        PACKET_CHECK(tx_props->packet_info.len[0] >= 14, return VTSS_RC_ERROR;);
        // Length of second fragment must be 0
        PACKET_CHECK(tx_props->packet_info.len[1] == 0, return VTSS_RC_ERROR;);
        // Do not adjust the total length to become a minimum Ethernet-sized frame. This is done by the FDMA driver.
    }

#if defined(VTSS_ARCH_LUTON26)
    // #ptp_action must be 0, 1, 2, or 3.
    PACKET_CHECK(tx_props->tx_info.ptp_action <= VTSS_PACKET_PTP_ACTION_ONE_AND_TWO_STEP, return VTSS_RC_ERROR;);
    // #ptp_id must be 0, 1, 2, or 3.
    PACKET_CHECK(tx_props->tx_info.ptp_id <= 3, return VTSS_RC_ERROR;);
#elif defined(VTSS_ARCH_SERVAL)
    // #ptp_action must be 0, 1, 2 or 4.
    PACKET_CHECK(tx_props->tx_info.ptp_action <= VTSS_PACKET_PTP_ACTION_TWO_STEP || tx_props->tx_info.ptp_action == VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP, return VTSS_RC_ERROR;);
    // #ptp_id must be 0, 1, 2, or 3.
    PACKET_CHECK(tx_props->tx_info.ptp_id <= 3, return VTSS_RC_ERROR;);
#else
    // #ptp_action not supported. Must be 0.
    PACKET_CHECK(tx_props->tx_info.ptp_action == VTSS_PACKET_PTP_ACTION_NONE, return VTSS_RC_ERROR;);
#endif /* defined(VTSS_ARCH_xxx) */

    // #ptp_action != 0 not supported with #oam_type != 0.
    PACKET_CHECK(tx_props->tx_info.oam_type == VTSS_PACKET_OAM_TYPE_NONE || tx_props->tx_info.ptp_action == VTSS_PACKET_PTP_ACTION_NONE, return VTSS_RC_ERROR;);

    if (tx_props->packet_info.tx_pre_cb != NULL) {
        // The FDMA might insert a VLAN tag into the frame if
        // filtering is enabled, so that the called back
        // function cannot rely on the retuned pointer pointing
        // to the start of the frame.
        PACKET_CHECK(tx_props->packet_info.filter.enable == FALSE, return VTSS_RC_ERROR;);

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // The FDMA inserts a VLAN tag into the frame, thus causing
        // the frame start to be moved, so that the called back
        // function cannot rely on the returned pointer pointing
        // to the start of the frame. If masquerading, a tag is only inserted if TPID != 0.
        PACKET_CHECK(tx_props->tx_info.switch_frm == FALSE || (tx_props->tx_info.masquerade_port != VTSS_PORT_NO_NONE && tx_props->tx_info.tag.vid == 0), return VTSS_RC_ERROR;);
#endif /* defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL) */
#if defined(VTSS_ARCH_LUTON28) && VTSS_SWITCH_STACKABLE
        // The FDMA inserts the VStaX2 header into the frame
        // (at least if VTSS_OPT_FDMA_VER >= 2, which is assumed in this code),
        // so that the called back function cannot rely on the returned
        // pointer pointing to the start of the frame.
        PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE, return VTSS_RC_ERROR;);
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_SWITCH_STACKABLE */
    }

#if defined(VTSS_ARCH_JAGUAR_1)
    // #latch_timestamp must be 0, 1, 2, or 3.
    PACKET_CHECK(tx_props->tx_info.latch_timestamp <= 3, return VTSS_RC_ERROR;);
#else
    // #latch_timestamp not supported. Must be 0.
    PACKET_CHECK(tx_props->tx_info.latch_timestamp == 0, return VTSS_RC_ERROR;);
#endif /* defined(VTSS_ARCH_xxx) */

    // Save a copy of user's tpid so that we haven't modified
    // the user's structure when this function returns
    saved_tpid = tx_props->tx_info.tag.tpid;

    if (tx_props->packet_info.filter.enable) {
        vtss_packet_port_info_t info;
        vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];

        // Avoid Lint Warning 676: Possibly negative subscript (-1) in operator
        // Lint can't see that #port_no indeed is assigned when tx_info.switch_frm == FALSE
        /*lint --e{676} */

        // When Tx filtering is enabled, #switch_frm must be FALSE and we can only transmit to one single port at a time.
        PACKET_CHECK(tx_props->tx_info.switch_frm == FALSE && port_cnt == 1, return VTSS_RC_ERROR;);

#if VTSS_SWITCH_STACKABLE
        // ...and the frame cannot be sent with a stack header or to a stack port.
        PACKET_CHECK(tx_props->tx_info.tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE && !PORT_NO_IS_STACK(port_no), return VTSS_RC_ERROR;);
#endif /* VTSS_SWITCH_STACKABLE */

        (void)vtss_packet_port_info_init(&info);
        info.port_no = tx_props->packet_info.filter.src_port;
#if defined(VTSS_FEATURE_AGGR_GLAG)
        info.glag_no = tx_props->packet_info.filter.glag_no;
#endif /* defined(VTSS_FEATURE_AGGR_GLAG) */
        info.vid     = tx_props->tx_info.tag.vid;

        if (vtss_packet_port_filter_get(NULL, &info, filter) != VTSS_RC_OK) {
            // Don't risk the vtss_packet_port_filter_get() function returning VTSS_RC_INV_STATE,
            // so take control of actual return value.
            return VTSS_RC_ERROR;
        }

        if (filter[port_no].filter == VTSS_PACKET_FILTER_DISCARD) {
            return VTSS_RC_INV_STATE; // Special return value indicating that we're actually not sending the frame due to filtering.
        }

        // By setting TPID to a non-zero value, the FDMA driver will insert a VLAN tag according to tx_info.tag
        // into the frame prior to transmitting it.
        if (filter[port_no].filter == VTSS_PACKET_FILTER_TAGGED) {
            tx_props->tx_info.tag.tpid = filter[port_no].tpid;
        } else {
            tx_props->tx_info.tag.vid = VTSS_VID_NULL; // In order not to get rewriter enabled.
        }
    }

    if (CX_stack_trace_ena || tx_props->packet_info.frm[0][ETYPE_POS] != 0x88 || tx_props->packet_info.frm[0][ETYPE_POS + 1] != 0x80) {
        T_DG(TRACE_GRP_TX, "Tx (%02x-%02x-%02x-%02x-%02x-%02x) by %s",
             tx_props->packet_info.frm[0][DMAC_POS + 0],
             tx_props->packet_info.frm[0][DMAC_POS + 1],
             tx_props->packet_info.frm[0][DMAC_POS + 2],
             tx_props->packet_info.frm[0][DMAC_POS + 3],
             tx_props->packet_info.frm[0][DMAC_POS + 4],
             tx_props->packet_info.frm[0][DMAC_POS + 5],
             vtss_module_names[tx_props->packet_info.modid]);
    }

    // Post the frame info to the Tx pending FIFO (a copy of tx_props occurs)
    result = TX_pend_fifo_post(tx_props);

    cyg_scheduler_lock();
    CX_port_counters.tx_pkts[tx_props->tx_info.switch_frm ? VTSS_PORTS : port_cnt > 1 ? VTSS_PORTS + 1 : port_no]++;
    cyg_scheduler_unlock();

    // Restore the user's src_port before exiting.
    tx_props->tx_info.tag.tpid = saved_tpid;
    return result;
}

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/******************************************************************************/
// packet_tx_afi_cancel()
/******************************************************************************/
vtss_rc packet_tx_afi_cancel(u8 *frm_ptr)
{
    packet_tx_props_t tx_props;

    PACKET_CHECK(frm_ptr, return VTSS_RC_ERROR;);

    // We must enforce a task switch to the packet TX pending thread
    // for the real cancel to be carried out, because otherwise
    // a check against the currently used list of AFI DCBs would be
    // prone to errors if the user module calls packet_tx() immediately
    // followed by a packet_tx_afi_cancel() since the packet_tx() call
    // isn't necessarily completed, because it's an asynchronous call.
    // So in the case where the caller has higher priority than
    // the packet tx pending thread, we could run into problems.

    // In order to enforce the task switch, we create a dummy frame
    // to be sent to packet_tx(). The TX_do_tx() discovers this dummy
    // frame and performs the real cancelling.
    packet_tx_props_init(&tx_props);
    tx_props.packet_info.frm[0]   = frm_ptr;
    tx_props.packet_info.internal_flags = PACKET_INTERNAL_FLAGS_AFI_CANCEL;
    return TX_pend_fifo_post(&tx_props);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/******************************************************************************/
// packet_tx_afi_frm_cnt()
/******************************************************************************/
vtss_rc packet_tx_afi_frm_cnt(u8 *frm, u64 *frm_cnt)
{
    return vtss_fdma_afi_frm_cnt(NULL, frm, frm_cnt);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/******************************************************************************/
// packet_tx_alloc()
// Size argument should not include IFH, CMD, and FCS
/******************************************************************************/
u8 *packet_tx_alloc(size_t size)
{
    u8 *buffer;
    size = MAX(VTSS_FDMA_MIN_FRAME_SIZE_BYTES - FCS_SIZE_BYTES, size);
    if ((buffer = VTSS_MALLOC(size + VTSS_FDMA_HDR_SIZE_BYTES /* IFH */ + 4 /* FCS */))) {
        buffer += VTSS_FDMA_HDR_SIZE_BYTES;
        cyg_scheduler_lock();
        TX_alloc_calls++;
        cyg_scheduler_unlock();
    }
    return buffer;
}

/******************************************************************************/
// packet_tx_alloc_extra()
// The difference between this function and the packet_tx_alloc() function is
// that the user is able to reserve a number of 32-bit words at the beginning
// of the packet, which is useful when some state must be saved between the call
// to the packet_tx() function and the callback function.
// Args:
//   @size              : Size exluding IFH, CMD, and FCS
//   @extra_size_dwords : Number of 32-bit words to reserve room for.
//   @extra_ptr         : Pointer that after the call will contain the pointer to the additional space.
// Returns:
//   Pointer to the location that the DMAC should be stored. If function fails,
//   the function returns NULL.
// Use packet_tx_free_extra() when freeing the packet rather than packet_tx_free().
/******************************************************************************/
u8 *packet_tx_alloc_extra(size_t size, size_t extra_size_dwords, u8 **extra_ptr)
{
    u8 *buffer;
    // The user must call us with the number of 32-bit words he wants extra or
    // we might end up with an IFH that is not 32-bit aligned.
    size_t extra_size_bytes = 4 * extra_size_dwords;
    PACKET_CHECK(extra_ptr, return NULL;);
    size = MAX(VTSS_FDMA_MIN_FRAME_SIZE_BYTES - FCS_SIZE_BYTES, size);
    if ((buffer = VTSS_MALLOC(size + VTSS_FDMA_HDR_SIZE_BYTES /* IFH */ + 4 /* FCS */ + extra_size_bytes))) {
        *extra_ptr = buffer;
        buffer    += VTSS_FDMA_HDR_SIZE_BYTES + extra_size_bytes;
        cyg_scheduler_lock();
        TX_alloc_calls++;
        cyg_scheduler_unlock();
    }
    return buffer;
}

/******************************************************************************/
// packet_tx_free()
/******************************************************************************/
void packet_tx_free(u8 *buffer)
{
    PACKET_CHECK(buffer != NULL, return;);
    buffer -= VTSS_FDMA_HDR_SIZE_BYTES;
    // Avoid Lint Warning 424: Inappropriate deallocation (free) for 'modified' data
    /*lint -e(424) */
    VTSS_FREE(buffer);
    cyg_scheduler_lock();
    TX_free_calls++;
    cyg_scheduler_unlock();
}

/******************************************************************************/
// packet_tx_free_extra()
// This function is the counter-part to the packet_tx_alloc_extra() function.
// It must be called with the value returned in packet_tx_alloc_extra()'s
// extra_ptr argument.
/******************************************************************************/
void packet_tx_free_extra(u8 *extra_ptr)
{
    PACKET_CHECK(extra_ptr != NULL, return;);
    VTSS_FREE(extra_ptr);
    cyg_scheduler_lock();
    TX_free_calls++;
    cyg_scheduler_unlock();
}

/******************************************************************************/
// packet_rx_filter_register()
// Context: Thread only
/******************************************************************************/
vtss_rc packet_rx_filter_register(const packet_rx_filter_t *filter, void **filter_id)
{
    vtss_rc rc = PACKET_ERROR_PARM;

    if (!filter_id) {
        T_E("filter_id must not be a NULL pointer");
        return rc;
    }

    // Validate the filter.
    if (!RX_filter_validate(filter)) {
        return rc;
    }

    PACKET_RX_FILTER_CRIT_ENTER();
    if (!RX_filter_insert(filter, filter_id)) {
        goto exit_func;
    }

    rc = VTSS_OK;
exit_func:
    PACKET_RX_FILTER_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// packet_rx_filter_unregister()
// Context: Thread only
// Remove a subscription.
/******************************************************************************/
vtss_rc packet_rx_filter_unregister(void *filter_id)
{
    vtss_rc rc = PACKET_ERROR_GEN;

    PACKET_RX_FILTER_CRIT_ENTER();

    if (!RX_filter_remove(filter_id)) {
        goto exit_func;
    }

    rc = VTSS_OK;

exit_func:
    PACKET_RX_FILTER_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// packet_rx_filter_change()
// Context: Thread only
// The change function allows a subscriber to change his filter on the
// fly. This is useful for e.g. the 802.1X protocol which just needs to
// update the source mask now and then as ports get in and out of
// authentication.
// If an error occurs, the current subscription is not changed.
// If an error doesn't occur, the filter_id may change.
// The call of this function corresponds to atomar calls to unregister()
// and register().
/******************************************************************************/
vtss_rc packet_rx_filter_change(const packet_rx_filter_t *filter, void **filter_id)
{
    vtss_rc rc = PACKET_ERROR_GEN;

    if (!filter_id) {
        T_E("filter_id must not be a NULL pointer");
        return rc;
    }

    if (!RX_filter_validate(filter)) {
        return rc;
    }

    PACKET_RX_FILTER_CRIT_ENTER();

    // Unplug the current filter ID
    // The reason for not just changing the current filter
    // is that the user may have changed priority, so that
    // it must be moved from one position to another in the
    // list, which is easily handled by first removing, then
    // inserting again.
    if (!RX_filter_remove(*filter_id)) {
        goto exit_func;
    }

    if (!RX_filter_insert(filter, filter_id)) {
        goto exit_func;
    }

    rc = VTSS_OK;

exit_func:
    PACKET_RX_FILTER_CRIT_EXIT();
    return rc;
}

/****************************************************************************/
// packet_uninit()
/****************************************************************************/
void packet_uninit(void)
{










}

/******************************************************************************/
// packet_rx_release_fdma_buffers()
/******************************************************************************/
void packet_rx_release_fdma_buffers(void *fdma_buffers)
{
    // Send back the buffer to the FDMA for reuse
    PACKET_CHECK(vtss_fdma_dcb_release(0, fdma_buffers) == VTSS_RC_OK, return;);
}

#if !defined(VTSS_ARCH_LUTON28)
/******************************************************************************/
// packet_rx_throttle_cfg_set()
// Function only to be used by CLI debug, hence not publicized in packet_api.h
/******************************************************************************/
vtss_rc packet_rx_throttle_cfg_set(vtss_fdma_throttle_cfg_t *cfg)
{
    vtss_packet_rx_queue_t xtr_qu;
    BOOL                   start_timer = FALSE;

    if (!cfg) {
        return VTSS_RC_ERROR;
    }

    // Start or stop the throttle tick timer based on the new configuration.
    // Since CLI doesn't (yet have support for specifying the byte limit, we do it for it).
    for (xtr_qu = 0; xtr_qu < ARRSZ(cfg->frm_limit_per_tick); xtr_qu++) {
        cfg->byte_limit_per_tick[xtr_qu] = cfg->frm_limit_per_tick[xtr_qu] * 1518;
        if (cfg->frm_limit_per_tick[xtr_qu] > 0) {
            start_timer = TRUE;
        }
    }


    if (vtss_fdma_throttle_cfg_set(NULL, cfg) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    RX_throttle_timer_set(start_timer);
    return VTSS_RC_OK;
}
#endif /* !defined(VTSS_ARCH_LUTON28) */

#if !defined(VTSS_ARCH_LUTON28)
/******************************************************************************/
// packet_rx_queue_usage()
/******************************************************************************/
char *packet_rx_queue_usage(u32 xtr_qu, char *buf, size_t size)
{
    int cnt = 0;

    if (!buf || size == 0) {
        return NULL;
    }

    buf[0] = '\0';

    if (xtr_qu >= VTSS_PACKET_RX_QUEUE_CNT) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sError: Invalid extraction queue (%u)", cnt != 0 ? ", " : "", xtr_qu);
    }

#if VTSS_SWITCH_STACKABLE
    if (xtr_qu == 8 || xtr_qu == 9) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sSPROUT", cnt != 0 ? ", " : "");
    }
    if (xtr_qu == PACKET_XTR_QU_STACK) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%stack messages", cnt != 0 ? ", s" : "S");
    }
#endif /* VTSS_SWITCH_STACKABLE */

    if (xtr_qu == PACKET_XTR_QU_BPDU) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sBPDUs", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_IGMP) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sIPMC", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_MGMT_MAC) {
#if defined(VTSS_ARCH_JAGUAR_1)
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sanagement DMAC and B/C with IP payload", cnt != 0 ? ", m" : "M");
#else
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sanagement DMAC", cnt != 0 ? ", m" : "M");
#endif
    }

    if (xtr_qu == PACKET_XTR_QU_L3_OTHER) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sanagement DMAC w/ TTL expiration or IP options", cnt != 0 ? ", m" : "M");
    }

    if (xtr_qu == PACKET_XTR_QU_MAC) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sort security", cnt != 0 ? ", p" : "P");
    }

    if (xtr_qu == PACKET_XTR_QU_OAM) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sOAM", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_L2CP) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sL2CP", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_BC) {
#if defined(VTSS_ARCH_JAGUAR_1)
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sroadcast w/o IP payload", cnt != 0 ? ", b" : "B");
#else
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sroadcast", cnt != 0 ? ", b" : "B");
#endif
    }

    if (xtr_qu == PACKET_XTR_QU_LEARN) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%secure learning", cnt != 0 ? ", s" : "S");
    }

    if (xtr_qu == PACKET_XTR_QU_ACL_COPY) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sACL Copy", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_ACL_REDIR) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sACL Redirect", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_SFLOW) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%ssFlow", cnt != 0 ? ", " : "");
    }

    if (xtr_qu == PACKET_XTR_QU_LRN_ALL) {
        cnt += snprintf(buf + cnt, MAX((int)size - cnt, 0), "%sLearn-All", cnt != 0 ? ", " : "");
    }

    if (cnt == 0) {
        (void)snprintf(buf, size, "<none>");
    }

    return buf;
}
#endif /* !defined(VTSS_ARCH_LUTON28) */

/******************************************************************************/
/******************************************************************************/
typedef enum {
    PACKET_DBG_CMD_STAT_PACKET_PRINT       =  1,
    PACKET_DBG_CMD_STAT_FDMA_PRINT,
    PACKET_DBG_CMD_STAT_PORT_PRINT,
    PACKET_DBG_CMD_SUBSCRIBERS_PRINT,
    PACKET_DBG_CMD_STAT_PACKET_CLEAR       = 10,
    PACKET_DBG_CMD_STAT_FDMA_CLEAR,
    PACKET_DBG_CMD_STAT_PORT_CLEAR,
    PACKET_DBG_CMD_CFG_STACK_TRACE         = 20,
    PACKET_DBG_CMD_CFG_SIGNAL_TX_PEND_COND = 22,
    PACKET_DBG_CMD_TEST_SYSLOG             = 40,
} packet_dbg_cmd_num_t;

/******************************************************************************/
/******************************************************************************/
typedef struct {
    packet_dbg_cmd_num_t  cmd_num;
    char                 *cmd_txt;
    char                 *arg_syntax;
    uint                  max_arg_cnt;
    void (*func)(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms);
} packet_dbg_cmd_t;

/******************************************************************************/
/******************************************************************************/
static const packet_dbg_cmd_t packet_dbg_cmds[] = {
    {
        PACKET_DBG_CMD_STAT_PACKET_PRINT,
        "Print per-module statistics",
        NULL,
        0,
        DBG_cmd_stat_module_print
    },
    {
        PACKET_DBG_CMD_STAT_FDMA_PRINT,
        "Print FDMA statistics",
        NULL,
        0,
        DBG_cmd_stat_fdma_print
    },
    {
        PACKET_DBG_CMD_STAT_PORT_PRINT,
        "Print per-port statistics",
        NULL,
        0,
        DBG_cmd_stat_port_print
    },
    {
        PACKET_DBG_CMD_SUBSCRIBERS_PRINT,
        "Print subscriber list",
        NULL,
        0,
        DBG_cmd_subscribers_print
    },
    {
        PACKET_DBG_CMD_STAT_PACKET_CLEAR,
        "Clear per-module statistics",
        NULL,
        0,
        DBG_cmd_stat_packet_clear
    },
    {
        PACKET_DBG_CMD_STAT_FDMA_CLEAR,
        "Clear FDMA statistics",
        NULL,
        0,
        DBG_cmd_stat_fdma_clear
    },
    {
        PACKET_DBG_CMD_STAT_PORT_CLEAR,
        "Clear port statistics",
        NULL,
        0,
        DBG_cmd_stat_port_clear
    },
    {
        PACKET_DBG_CMD_CFG_STACK_TRACE,
        "Enable or disable stack trace",
        "0: Disable, 1: Enable",
        1,
        DBG_cmd_cfg_stack_trace
    },
    {
        PACKET_DBG_CMD_CFG_SIGNAL_TX_PEND_COND,
        "Signal TX Pending Condition. USE WITH CARE!!",
        "",
        0,
        DBG_cmd_cfg_signal_tx_pend_cond
    },
    {
        PACKET_DBG_CMD_TEST_SYSLOG,
        "Generate error or fatal. This is only to test the SYSLOG, and has nothing to do with the packet module",
        "0: Generate error, 1: Generate assert (never returns)",
        1,
        DBG_cmd_test_syslog
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
// packet_dbg()
/******************************************************************************/
void packet_dbg(packet_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms)
{
    int i;
    u32 cmd_num;

    if (parms_cnt == 0) {
        (void)dbg_printf("Usage: debug packet <cmd idx>\n");
        (void)dbg_printf("Most commands show current settings if called without arguments\n\n");
        (void)dbg_printf("Commands:\n");
        i = 0;

        while (packet_dbg_cmds[i].cmd_num != 0) {
            (void)dbg_printf("  %2d: %s\n", packet_dbg_cmds[i].cmd_num, packet_dbg_cmds[i].cmd_txt);
            if (packet_dbg_cmds[i].arg_syntax && packet_dbg_cmds[i].arg_syntax[0]) {
                (void)dbg_printf("      Arguments: %s.\n", packet_dbg_cmds[i].arg_syntax);
            }
            i++;
        }
        return;
    }

    cmd_num = parms[0];

    // Verify that command is known and argument count is correct
    i = 0;
    while (packet_dbg_cmds[i].cmd_num != 0) {
        if (packet_dbg_cmds[i].cmd_num == cmd_num) {
            break;
        }
        i++;
    }

    if (packet_dbg_cmds[i].cmd_num == 0) {
        DBG_cmd_syntax_error(dbg_printf, "Unknown command number: %d", cmd_num);
        return;
    }

    if (parms_cnt - 1 > packet_dbg_cmds[i].max_arg_cnt) {
        DBG_cmd_syntax_error(dbg_printf, "Incorrect number of arguments (%d).\n"
                             "Arguments: %s",
                             parms_cnt - 1,
                             packet_dbg_cmds[i].arg_syntax);
        return;
    }

    if (packet_dbg_cmds[i].func == NULL) {
        (void)dbg_printf("Internal Error: Function for command %u not implemented (yet?)", cmd_num);
        return;
    }

    packet_dbg_cmds[i].func(dbg_printf, parms_cnt - 1, parms + 1);
}

/******************************************************************************/
// Initialize packet module
/******************************************************************************/
vtss_rc packet_init(vtss_init_data_t *data)
{
    if (data->cmd == INIT_CMD_INIT) {

#if defined(VTSS_SW_OPTION_VCLI)
        packet_cli_init();
#endif /* VTSS_SW_OPTION_VCLI */

        // Initialize common part
        CX_init();

        // Initialize injection part
        TX_init();

        // Initialize extraction part
        RX_init();
    } else if (data->cmd == INIT_CMD_START) {

        cyg_scheduler_lock();

        // Start
        CX_start_fdma();

        // Hook interrupts
        CX_start_interrupts();

        cyg_scheduler_unlock();

        // Resume threads
        cyg_thread_resume(RX_thread_handle);
        cyg_thread_resume(TX_done_thread_handle);
        cyg_thread_resume(TX_pend_thread_handle);
    }
    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
