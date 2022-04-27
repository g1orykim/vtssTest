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

// See PSEC_msg_rx() for an in-depth discussion of why this is OK.
/*lint -esym(459, PSEC_msg_rx) */

#include "critd_api.h"       /* For semaphore wrapper                                */
#include "psec_api.h"        /* To get access to our own structures and enumerations */
#ifdef VTSS_SW_OPTION_VCLI
#include "psec_cli.h"        /* For CLI initialization function                      */
#endif
#include "psec.h"            /* For our own semi-public structures                   */
#include "msg_api.h"         /* For message transmission and reception functions.    */
#include "psec_rate_limit.h" /* For rate-limiter                                     */
#include "misc_api.h"        /* For misc_mac2str()                                   */
#include "mac_api.h"         /* mac_mgmt_addr_entry_t and MAC functions              */
#include "packet_api.h"      /* For packet_rx_filter_XXX()                           */
#include "port_api.h"        /* For port iterators                                   */
#include "main.h"            /* For vtss_xstr()                                      */
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"      /* For S_W()                                            */
#endif
#include "psec_trace.h"
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PSEC

/****************************************************************************/
// Trace definitions
/****************************************************************************/
#include <vtss_trace_api.h>

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by psec_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "psec",
    .descr     = "Port Security module"
};

#ifndef PSEC_DEFAULT_TRACE_LVL
#define PSEC_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = PSEC_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = PSEC_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_MAC_MODULE] = {
        .name      = "mac",
        .descr     = "MAC Module Calls",
        .lvl       = PSEC_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/******************************************************************************/
// Semaphore stuff.
/******************************************************************************/
static critd_t crit_psec;

// Macros for accessing semaphore functions
// -----------------------------------------
#if VTSS_TRACE_ENABLED
#define PSEC_CRIT_ENTER()         critd_enter(        &crit_psec, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PSEC_CRIT_EXIT()          critd_exit(         &crit_psec, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PSEC_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit_psec, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define PSEC_CRIT_ENTER()         critd_enter(        &crit_psec)
#define PSEC_CRIT_EXIT()          critd_exit(         &crit_psec)
#define PSEC_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit_psec)
#endif

/**
  * \brief Reasons for checking whether secure learning with/without CPU copy should be enabled or disabled
  */
typedef enum {
    PSEC_LEARN_CPU_REASON_MAC_ADDRESS_ALLOCATED,
    PSEC_LEARN_CPU_REASON_MAC_ADDRESS_FREED,
    PSEC_LEARN_CPU_REASON_SWITCH_UP_OR_DOWN,
    PSEC_LEARN_CPU_REASON_OTHER,
} psec_learn_cpu_reason_t;

typedef struct {
    BOOL switch_exists; // Cached version of msg_switch_exists(). Used to speed up.
    psec_port_state_t port_state[VTSS_PORTS];
} psec_switch_state_t;

// We need to keep track of the whole stack
typedef struct {
    /**
      * The per-user-module aging period measured in seconds.
      * A given module's aging period will only be used if the module is enabled on a given port.
      * A value of 0 means disable aging.
      * Used when an entry is forwarding on a port (i.e. psec_on_mac_add_callback()
      * returned PSEC_ADD_METHOD_FORWARD).
      */
    u32 aging_period_secs[PSEC_USER_CNT];

    /**
      * The per-user-module MAC block period measured in seconds.
      * Used while keeping a non-forwarding entry in the MAC table.
      * A given module's block time will only be used if the module is enabled on a given port.
      * A value of 0 is invalid.
      * Used when an entry is not forwarding on a port (i.e. psec_on_mac_add_callback()
      * returned PSEC_ADD_MEDHOD_BLOCK).
     */
    u32 hold_time_secs[PSEC_USER_CNT];

    /**
      * The per-user-module On-MAC-Add callback function.
      * If non-NULL and the user is enabled on the port on which a MAC
      * address is going to be added, then the callback will be called.
      */
    psec_on_mac_add_callback_f *on_mac_add_callbacks[PSEC_USER_CNT];

    /**
      * The per-user-module On-MAC-Del callback function.
      * If non-NULL and the user is enabled on the port on which a MAC
      * address is going to be deleted, then the callback will be called.
      */
    psec_on_mac_del_callback_f *on_mac_del_callbacks[PSEC_USER_CNT];

    /**
      * The current switch state - one entry per switch (0-based)
      */
    psec_switch_state_t switch_state[VTSS_ISID_CNT];
} psec_stack_state_t;

static psec_stack_state_t PSEC_stack_state;
static psec_mac_state_t   PSEC_mac_states[PSEC_MAC_ADDR_ENTRY_CNT];
static psec_mac_state_t   *PSEC_mac_state_free_pool;
static u32                PSEC_mac_states_left;

#if VTSS_PORTS > 64
#error "This module supports at most 64 front ports"
#endif

// Fast reference of which ports need to be aged/held-counted.
// One u64 per switch, each bit represents a port. Port 0 is LSbit.
static u64                PSEC_age_hold_enabled_ports[VTSS_ISID_CNT];

// Local configuration
static u8                 PSEC_copy_to_master[VTSS_PORT_BF_SIZE];
static packet_rx_filter_t PSEC_frame_rx_filter;
static void               *PSEC_frame_rx_filter_id = NULL;

/******************************************************************************/
// Thread variables
/******************************************************************************/
static cyg_handle_t PSEC_thread_handle;
static cyg_thread   PSEC_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         PSEC_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// If not debugging, set PSEC_INLINE to inline
#define PSEC_INLINE inline

/******************************************************************************/
//
// Message handling functions, structures, and state.
//
/******************************************************************************/

/****************************************************************************/
// Message IDs
/****************************************************************************/
typedef enum {
    PSEC_MSG_ID_MST_TO_SLV_RATE_LIMIT_CFG, // Tell the slave the current rate-limiter setup.
    PSEC_MSG_ID_MST_TO_SLV_PORT_CFG,       // Tell the slave whether to copy frames to the master for one port.
    PSEC_MSG_ID_MST_TO_SLV_SWITCH_CFG,     // Tell the slave whether to copy frames to the master for the whole switch.
    PSEC_MSG_ID_SLV_TO_MST_FRAME,          // Tell the master the MAC address and VID of a frame received on a Port Security enabled port.
} psec_msg_id_t;

/****************************************************************************/
// Mst->Slv: Rate limiter setup.
/****************************************************************************/
typedef struct {
    psec_rate_limit_cfg_t rate_limit; // Rate limit configuration.
} psec_msg_rate_limit_cfg_t;

/****************************************************************************/
// Mst->Slv: Tell the slave to enable or disable registration for frames on
// a given port. Once set frames will be forwarded to the master if enabled.
/****************************************************************************/
typedef struct {
    vtss_port_no_t port;
    BOOL copy_to_master;
} psec_msg_port_cfg_t;

/****************************************************************************/
// Mst->Slv: Tell the slave on which ports to register for frames. Once set
// frames will be forwarded to the master if enabled.
/****************************************************************************/
typedef struct {
    u8 copy_to_master[VTSS_PORT_BF_SIZE];
} psec_msg_switch_cfg_t;

/****************************************************************************/
// Slv->Mst: Whenever any frame is seen on an enabled port, the MAC
// address and VLAN ID along with whether it's a learn frame is sent to
// the master, if there's a reason for it.
/****************************************************************************/
typedef struct {
    vtss_port_no_t port;
    vtss_vid_mac_t vid_mac;
    BOOL           is_learn_frame;
} psec_msg_frame_t;

/****************************************************************************/
// Message Identification Header
/****************************************************************************/
typedef struct {
    // Message Version Number
    u32 version; // Set to PSEC_MSG_VERSION

    // Message ID
    psec_msg_id_t msg_id;
} psec_msg_hdr_t;

/****************************************************************************/
// Message.
// This struct contains a union, whose primary purpose is to give the
// size of the biggest of the contained structures.
/****************************************************************************/
typedef struct {
    // Header stuff
    psec_msg_hdr_t hdr;

    // Request message
    union {
        psec_msg_rate_limit_cfg_t rate_limit_cfg;
        psec_msg_port_cfg_t       port_cfg;
        psec_msg_switch_cfg_t     switch_cfg;
        psec_msg_frame_t          frame;
    } u; // Anonymous unions are not allowed in C99 :(
} psec_msg_t;

/****************************************************************************/
// Frame Message
// This struct is needed because transmission of learn frames is not done
// using the normal buffer alloc structure, because that may cause a deadlock
// since its done from the Packet Rx thread.
/****************************************************************************/
typedef struct {
    psec_msg_hdr_t   hdr;
    psec_msg_frame_t frame;
} psec_msg_hdr_and_frame_t;

static void *PSEC_msg_buf_pool; // Static, semaphore-protected message transmission buffer(s).

// Current version of Port Security messages (1-based).
// Future revisions of this module should support previous versions if applicable.
#define PSEC_MSG_VERSION 1

/******************************************************************************/
// PSEC_msg_buf_alloc()
// Blocks until a buffer is available, then takes and returns it.
/******************************************************************************/
static psec_msg_t *PSEC_msg_buf_alloc(psec_msg_id_t msg_id)
{
    psec_msg_t *msg = msg_buf_pool_get(PSEC_msg_buf_pool);
    VTSS_ASSERT(msg);
    msg->hdr.version = PSEC_MSG_VERSION;
    msg->hdr.msg_id = msg_id;
    return msg;
}

/******************************************************************************/
// PSEC_msg_tx_done()
// Called when message is successfully or unsuccessfully transmitted.
/******************************************************************************/
static void PSEC_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    // Release the message back to the message buffer pool
    (void)msg_buf_pool_put(msg);
}

/******************************************************************************/
// PSEC_msg_tx()
// Do transmit a message.
/******************************************************************************/
static void PSEC_msg_tx(psec_msg_t *msg, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(NULL, PSEC_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_PSEC, isid, msg, len + MSG_TX_DATA_HDR_LEN(psec_msg_t, u));
}

/******************************************************************************/
// PSEC_msg_tx_rate_limit_cfg()
/******************************************************************************/
static void PSEC_msg_tx_rate_limit_cfg(vtss_isid_t isid, psec_rate_limit_cfg_t *cfg)
{
    psec_msg_t *msg;

    if (!msg_switch_exists(isid)) {
        return;
    }

    // Get a buffer.
    msg = PSEC_msg_buf_alloc(PSEC_MSG_ID_MST_TO_SLV_RATE_LIMIT_CFG);

    // Copy cfg to buffer
    msg->u.rate_limit_cfg.rate_limit = *cfg;

    T_D("Transmitting rate-limit cfg to isid=%d", isid);

    // Transmit it.
    PSEC_msg_tx(msg, isid, sizeof(msg->u.rate_limit_cfg));
}

/******************************************************************************/
// PSEC_msg_tx_port_cfg()
// Transmit enabledness for @port to @isid.
/******************************************************************************/
static void PSEC_msg_tx_port_cfg(vtss_isid_t isid, vtss_port_no_t port, BOOL enable)
{
    psec_msg_t *msg;

    if (!msg_switch_exists(isid)) {
        return;
    }

    // Get a buffer.
    msg = PSEC_msg_buf_alloc(PSEC_MSG_ID_MST_TO_SLV_PORT_CFG);

    // Copy cfg to buffer
    msg->u.port_cfg.port           = port;
    msg->u.port_cfg.copy_to_master = enable;

    T_D("Tx port cfg to isid:port:ena=%d:%d:%d", isid, port, enable);

    // Transmit it.
    PSEC_msg_tx(msg, isid, sizeof(msg->u.port_cfg));
}

/******************************************************************************/
// PSEC_msg_tx_switch_cfg()
// Transmit enabledness for all ports on switch @isid to @isid.
/******************************************************************************/
static PSEC_INLINE void PSEC_msg_tx_switch_cfg(vtss_isid_t isid)
{
    psec_msg_t          *msg;
    psec_switch_state_t *switch_state;
    port_iter_t         pit;

    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL) != VTSS_RC_OK) {
        // Don't wanna allocate a message buffer if the switch doesn't exist.
        return;
    }

    // Get a buffer.
    msg = PSEC_msg_buf_alloc(PSEC_MSG_ID_MST_TO_SLV_SWITCH_CFG);

    // Copy cfg to buffer
    VTSS_PORT_BF_CLR(msg->u.switch_cfg.copy_to_master);
    switch_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START];
    while (port_iter_getnext(&pit)) {
        if (switch_state->port_state[pit.iport - VTSS_PORT_NO_START].ena_mask) {
            // At least one user is enabled on this port.
            VTSS_PORT_BF_SET(msg->u.switch_cfg.copy_to_master, (pit.iport + VTSS_PORT_NO_START), 1);
        }
    }

    T_D("Tx switch cfg to isid %d", isid);

    // Transmit it.
    PSEC_msg_tx(msg, isid, sizeof(msg->u.switch_cfg));
}

/******************************************************************************/
// PSEC_msg_tx_frame()
// Transmit (learn) frame properties to current master
/******************************************************************************/
static PSEC_INLINE void PSEC_msg_tx_frame(vtss_port_no_t port, vtss_vid_mac_t *vid_mac, BOOL learn_flag)
{
    psec_msg_hdr_and_frame_t *msg;

    // Do not wait for buffers here, since that may lead to a deadlock, causing the
    // msg module not to receive MACKs on the master, because this is called from the
    // packet rx thread, and holding up that thread causes the message module to
    // go dead.

    // The msg is freed by the message module
    msg = VTSS_MALLOC(sizeof(*msg));

    if (!msg) {
        T_W("Unable to allocate %zu bytes for learn frame", sizeof(*msg));
        return;
    }

    msg->hdr.version          = PSEC_MSG_VERSION;
    msg->hdr.msg_id           = PSEC_MSG_ID_SLV_TO_MST_FRAME;
    msg->frame.port           = port;
    msg->frame.vid_mac        = *vid_mac;
    msg->frame.is_learn_frame = learn_flag;

    // Let the message module free the buffer.
    // These frames are subject to shaping.
    msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK | MSG_TX_OPT_SHAPE, VTSS_MODULE_ID_PSEC, 0, msg, sizeof(*msg));
}

/******************************************************************************/
// PSEC_mac_module_chg()
// Ask the MAC module to add a new entry (or change an existing).
/******************************************************************************/
static BOOL PSEC_mac_module_chg(vtss_isid_t isid, vtss_port_no_t port, psec_mac_state_t *mac_state, int called_from)
{
    mac_mgmt_addr_entry_t entry;
    vtss_rc               rc;

    T_DG(TRACE_GRP_MAC_MODULE, "MAC Add/Chg(%s on %u) to isid:port = %d:%d. flags = 0x%x. Called from line %d", misc_mac2str(mac_state->vid_mac.mac.addr), mac_state->vid_mac.vid, isid, port, mac_state->flags, called_from);

    memset(&entry, 0, sizeof(entry));
    entry.vid_mac         = mac_state->vid_mac;
    entry.not_stack_ports = 1; // Always
    entry.volatil         = 1; // Always
    entry.copy_to_cpu     = (mac_state->flags & PSEC_MAC_STATE_FLAGS_CPU_COPYING) ? TRUE : FALSE;
    mac_state->changed_time_secs = msg_uptime_get(VTSS_ISID_LOCAL);

    if (!(mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED)) {
        entry.destination[port] = 1;
    }

    if ((rc = mac_mgmt_table_add(isid, &entry)) != VTSS_RC_OK) {
        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE;
        // If other modules are adding volatile MAC addresses, this may be quite normal, hence the T_D() rather than T_W()
        T_DG(TRACE_GRP_MAC_MODULE, "MAC Add(%s on %u) failed (code = %d, str=%s, called from line %d)", misc_mac2str(mac_state->vid_mac.mac.addr), mac_state->vid_mac.vid, rc, error_txt(rc), called_from);
    } else {
        mac_state->flags |= PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE;
    }

    return rc == VTSS_RC_OK;
}

/******************************************************************************/
// PSEC_mac_module_del()
/******************************************************************************/
static void PSEC_mac_module_del(vtss_isid_t isid, psec_mac_state_t *mac_state)
{
    vtss_rc rc;

    // Only to this if the entry is known to be in the MAC table.
    if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
        // Unregister volatile MAC entry from the MAC table
        T_DG(TRACE_GRP_MAC_MODULE, "Unregistering MAC address %s on %u", misc_mac2str(mac_state->vid_mac.mac.addr), mac_state->vid_mac.vid);
        rc = mac_mgmt_table_del(isid, &mac_state->vid_mac, TRUE);
        if (rc != VTSS_RC_OK && rc != MAC_ERROR_STACK_STATE) { // When going from master to slave, the MAC_ERROR_STACK_STATE is very likely to be returned.
            T_WG(TRACE_GRP_MAC_MODULE, "mac_mgmt_table_del(%s on %u) failed (code = %d, str=%s)", misc_mac2str(mac_state->vid_mac.mac.addr), mac_state->vid_mac.vid, rc, error_txt(rc));
        }
        // Now it's no longer there.
        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE;
    }
}

/******************************************************************************/
// PSEC_mac_module_sec_learn_cpu_copy()
/******************************************************************************/
static PSEC_INLINE void PSEC_mac_module_sec_learn_cpu_copy(vtss_isid_t isid, vtss_port_no_t port, BOOL enable, BOOL cpu_copy)
{
    vtss_rc rc;
    psec_port_state_t *port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];

    T_IG(TRACE_GRP_MAC_MODULE, "Setting %d:%d to SEC_LEARN = %d, CPU_COPY = %d", isid, port, enable, cpu_copy);

    if (enable) {
        port_state->flags |= PSEC_PORT_STATE_FLAGS_SEC_LEARNING;
        if (cpu_copy) {
            port_state->flags |= PSEC_PORT_STATE_FLAGS_CPU_COPYING;
        } else {
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_CPU_COPYING;
        }
        if ((rc = mac_mgmt_learn_mode_force_secure(isid, port, cpu_copy)) != VTSS_RC_OK) {
            T_WG(TRACE_GRP_MAC_MODULE, "mac_mgmt_learn_mode_force_secure() failed (error=%s)", error_txt(rc));
        }
    } else {
        port_state->flags &= ~PSEC_PORT_STATE_FLAGS_SEC_LEARNING;
        port_state->flags &= ~PSEC_PORT_STATE_FLAGS_CPU_COPYING; // Superfluous
        if ((rc = mac_mgmt_learn_mode_revert(isid, port)) != VTSS_RC_OK) {
            T_WG(TRACE_GRP_MAC_MODULE, "mac_mgmt_learn_mode_revert() failed (error=%s)", error_txt(rc));
        }
    }
}

/******************************************************************************/
// PSEC_next_unique()
/******************************************************************************/
static PSEC_INLINE u32 PSEC_next_unique(void)
{
    static u32 unique = 1;
    u32 result = unique++;

    if (unique == 0) {
        // 0 is reserved for a non-allocated item.
        unique++;
    }
    return result;
}

/******************************************************************************/
// PSEC_mac_state_alloc()
/******************************************************************************/
static PSEC_INLINE psec_mac_state_t *PSEC_mac_state_alloc(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac)
{
    psec_mac_state_t  *result;
    psec_port_state_t *port_state;

    if (!PSEC_mac_states_left) {
        return NULL;
    }

    result = PSEC_mac_state_free_pool;
    port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];

    PSEC_mac_state_free_pool = result->next;
    memset(result, 0, sizeof(*result));
    PSEC_mac_states_left--;

    memcpy(&result->vid_mac, vid_mac, sizeof(result->vid_mac));
    result->creation_time_secs = msg_uptime_get(VTSS_ISID_LOCAL);
    result->unique = PSEC_next_unique();

    // Add to the beginning of the port's list.
    result->next = port_state->macs;
    if (port_state->macs) {
        port_state->macs->prev = result;
    }
    port_state->macs = result;
    port_state->mac_cnt++;

    return result;
}

/******************************************************************************/
// PSEC_ensure_zero_padding()
// The vtss_vid_mac_t type contains padding fields that may not be set to
// zero by callers into this module. This may cause memcmp() of the whole
// structure to fail, and therefore unexpected side-effects.
// This function ensures that @vid_mac indeed contains zeros in all padding
// fields on return.
/******************************************************************************/
static void PSEC_ensure_zero_padding(vtss_vid_mac_t *vid_mac)
{
    vtss_vid_mac_t new_vid_mac;

    memset(&new_vid_mac, 0, sizeof(new_vid_mac));
    new_vid_mac.vid       = vid_mac->vid;
    // Gotta point out the field within vid_mac->mac. If not, it would still copy too much, because vid_mac->mac is 8 bytes, whereas vid_mac->mac.addr is 6 bytes.
    memcpy(new_vid_mac.mac.addr, vid_mac->mac.addr, sizeof(new_vid_mac.mac.addr));
    *vid_mac              = new_vid_mac;
}

/******************************************************************************/
// PSEC_sec_learn_cpu_copy_check()
// Reason can be one of the following:
//   PSEC_LEARN_CPU_REASON_MAC_ADDRESS_ALLOCATED
//   PSEC_LEARN_CPU_REASON_MAC_ADDRESS_FREED
//   PSEC_LEARN_CPU_REASON_SWITCH_UP_OR_DOWN
//   PSEC_LEARN_CPU_REASON_OTHER
/******************************************************************************/
static void PSEC_sec_learn_cpu_copy_check(vtss_isid_t isid, vtss_port_no_t port, psec_learn_cpu_reason_t reason, int called_from)
{
    vtss_isid_t    isid_start, isid_end;
    vtss_port_no_t port_start, port_end;

    T_DG(TRACE_GRP_MAC_MODULE, "%d:%d: Reason = %d, called from line %d", isid, port, reason, called_from);

    // We use isid_start, isid_end, port_start, and port_end as 0-based in this function.
    switch (reason) {
    case PSEC_LEARN_CPU_REASON_MAC_ADDRESS_ALLOCATED:
        // In this case, it might be that we have run out of MAC addresses by
        // this new allocation. This will affect all ports in the stack.
        if (PSEC_mac_states_left == 0) {
            // We did run out
            isid_start = 0;
            isid_end   = VTSS_ISID_CNT - 1;
            port_start = 0;
            port_end   = VTSS_PORTS - 1;
        } else {
            // We didn't. Then it only affects this port.
            isid_start = isid_end = isid - VTSS_ISID_START;
            port_start = port_end = port - VTSS_PORT_NO_START;
        }
        break;

    case PSEC_LEARN_CPU_REASON_MAC_ADDRESS_FREED:
        // In this case, it might be that the recently freed MAC address entry
        // caused the free list to go from empty to non-empty.
        if (PSEC_mac_states_left == 1) {
            // Got a new entry.
            isid_start = 0;
            isid_end   = VTSS_ISID_CNT - 1;
            port_start = 0;
            port_end   = VTSS_PORTS - 1;
        } else {
            // We didn't. Then it only affects this port.
            isid_start = isid_end = isid - VTSS_ISID_START;
            port_start = port_end = port - VTSS_PORT_NO_START;
        }
        break;

    case PSEC_LEARN_CPU_REASON_SWITCH_UP_OR_DOWN:
        // In this case we need to check the switch in question and (un)register
        // all currently registered secure learnings.
        isid_start = isid_end = isid - VTSS_ISID_START;
        port_start = 0;
        port_end   = VTSS_PORTS - 1;
        break;

    case PSEC_LEARN_CPU_REASON_OTHER:
        // Only check this isid:port.
        isid_start = isid_end = isid - VTSS_ISID_START;
        port_start = port_end = port - VTSS_PORT_NO_START;
        break;

    default:
        T_E("Unknown reason (%d)", reason);
        return;
    }

    // Loop through all the switches and ports we need to check
    for (isid = isid_start; isid <= isid_end; isid++) {
        psec_switch_state_t *switch_state = &PSEC_stack_state.switch_state[isid];
        for (port = port_start; port <= port_end; port++) {
            psec_port_state_t *port_state;
            BOOL new_enable_secure_learning;
            BOOL new_enable_cpu_copying;
            BOOL old_enable_secure_learning;
            BOOL old_enable_cpu_copying;

            if (port >= port_isid_port_count(isid + VTSS_ISID_START) || port_isid_port_no_is_stack(isid + VTSS_ISID_START, port + VTSS_PORT_NO_START)) {
                continue;
            }

            port_state = &switch_state->port_state[port];
            new_enable_cpu_copying = FALSE;

            // We should enable secure learning if the switch exists, the port is up, and
            // at least one user-module is enabled.
            new_enable_secure_learning = (switch_state->switch_exists)                       &&
                                         (port_state->ena_mask != 0)                         &&
                                         (port_state->flags & PSEC_PORT_STATE_FLAGS_LINK_UP);

            if (new_enable_secure_learning) {
                // We should then enable CPU-copying if the limit is not reached,
                // the port is not shut-down (by PSEC LIMIT module), and there are
                // no H/W or S/W add failures (still) detected on the port.
                // In addition, we should enable CPU-copying if no enabled user modules
                // require the port to be in the not-enable CPU-copying state.
                // Well, in fact, if it wasn't for the fact that BPDUs are not even redirected
                // to the CPU if CPU-copying is disabled, this was true.
                // Unfortunately, there is a bug in E-StaX-34 that causes
                // BPDUs to be killed if CPU-copying of learn frames is disabled.
                // This is controlled by the PSEC_FIX_GNATS_6935 define.
                // If PSEC_FIX_GNATS_6935 is not defined, we disable CPU copying if at least
                // one enabled user module wants to keep the port blocked.
                // If PSEC_FIX_GNATS_6935 is defined, we do not disable CPU copying if at least
                // one enabled user module wants to keep the port blocked. Instead we let
                // the PSEC_handle_frame_reception() function filter out frames before passing
                // them on to the user modules.
                BOOL keep_port_blocked  = (port_state->ena_mask & port_state->mode_mask) ? TRUE : FALSE;
#ifdef PSEC_FIX_GNATS_6935
                // The Force CPU-copy-bit can be set whether the user is enabled on the port or not.
                BOOL force_cpu_copy = port_state->force_cpu_copy_mask ? TRUE : FALSE;
                if (keep_port_blocked || force_cpu_copy) {
                    // We're requested to keep the port blocked, but in fact, we open the port (SIC!)
                    // to get BPDUs to the CPU. In all cases except the one where the port is shut down
                    // due to a limit exceed, we therefore open the port.
                    // When PSEC_FIX_GNATS_6935 is undefined, the Force CPU-copy is no longer present,
                    // because it's not needed in the code, because BPDUs will get copied to the CPU anyway.
                    new_enable_cpu_copying = !(port_state->flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN);
                } else
#endif
                {
                    new_enable_cpu_copying = !((port_state->flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) ||
                                               (port_state->flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN)     ||
                                               (port_state->flags & PSEC_PORT_STATE_FLAGS_HW_ADD_FAILED) ||
                                               (port_state->flags & PSEC_PORT_STATE_FLAGS_SW_ADD_FAILED) ||
                                               (keep_port_blocked));
                }
            }

            // Now check against the current settings before calling any MAC module API
            // functions.
            old_enable_secure_learning = (port_state->flags & PSEC_PORT_STATE_FLAGS_SEC_LEARNING) ? TRUE : FALSE;
            old_enable_cpu_copying     = (port_state->flags & PSEC_PORT_STATE_FLAGS_CPU_COPYING)  ? TRUE : FALSE;

            if ((new_enable_secure_learning != old_enable_secure_learning) ||
                (new_enable_secure_learning && (old_enable_cpu_copying != new_enable_cpu_copying))) {
                PSEC_mac_module_sec_learn_cpu_copy(isid + VTSS_ISID_START, port + VTSS_PORT_NO_START, new_enable_secure_learning, new_enable_cpu_copying);
            }
        }
    }
}

/******************************************************************************/
// PSEC_msg_id_to_str()
/******************************************************************************/
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *PSEC_msg_id_to_str(psec_msg_id_t msg_id)
{
    switch (msg_id) {
    case PSEC_MSG_ID_MST_TO_SLV_RATE_LIMIT_CFG:
        return "MST_TO_SLV_RATE_LIMIT_CFG";

    case PSEC_MSG_ID_MST_TO_SLV_PORT_CFG:
        return "MST_TO_SLV_PORT_CFG";

    case PSEC_MSG_ID_MST_TO_SLV_SWITCH_CFG:
        return "MST_TO_SLV_SWITCH_CFG";

    case PSEC_MSG_ID_SLV_TO_MST_FRAME:
        return "SLV_TO_MST_FRAME";

    default:
        return "***Unknown Message ID***";
    }
}
#endif /* VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG */

/******************************************************************************/
// PSEC_update_age_hold_enabledness()
/******************************************************************************/
static void PSEC_update_age_hold_enabledness(vtss_isid_t isid, vtss_port_no_t port, psec_port_state_t *port_state)
{
    psec_mac_state_t *mac_state = port_state->macs;
    BOOL             enable     = FALSE;
    u64              port_mask  = 1ULL << (port - VTSS_PORT_NO_START);

    while (mac_state) {
        // The entry is subject to aging/holding if the KEEP_BLOCKED flag is cleared or
        // if the current age_or_hold_time_secs counter is non-0 (0 means aging disabled).
        if (!((mac_state->flags & PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED) || (mac_state->age_or_hold_time_secs == 0))) {
            enable = TRUE;
            break;
        }
        mac_state = mac_state->next;
    }

    if (enable) {
        // Set the corresponding bit in the quick-access array
        PSEC_age_hold_enabled_ports[isid - VTSS_ISID_START] |= port_mask;
    } else {
        PSEC_age_hold_enabled_ports[isid - VTSS_ISID_START] &= ~port_mask;
    }
}

/******************************************************************************/
// PSEC_del_callback()
// Unlike PSEC_add_callback(), we DON'T release our mutex during the callbacks.
// If we did, we should bump mac_state->unique before releasing the mutex to
// let the PSEC_add_callback() know that something had happened to the mac_state
// during the callbacks.
// The primary reason for not releasing our mutex is that a lot of state
// changes occur *after* the callback, and these must be in integrity with
// the current delete execution flow.
// Another (primary) reason is that if deleting a mac_state wasn't an undividable
// operation, then both port-down events and switch-delete events might get to
// enter the PSEC_del_callback() for the same MAC address, thus causing multiple
// delete events as seen from the users' point of view. This is avoided by
// doing the full delete in one undividiable operation.
// Race-conditions may still occur. See prolog of PSEC_add_callback() for a
// description of this.
// If psec_mgmt_mac_del() is the source of the deletion, the user calling
// psec_mgmt_mac_del() will *not* be called back. This is taken care of prior
// to the call to this function (and is yet another reason that this must be
// an undividable operation).
/******************************************************************************/
static PSEC_INLINE void PSEC_del_callback(vtss_isid_t isid, vtss_port_no_t port, psec_port_state_t *port_state, psec_mac_state_t *mac_state, psec_del_reason_t reason)
{
    psec_users_t user;

    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        if (PSEC_stack_state.on_mac_del_callbacks[user] && PSEC_USER_ENA_GET(port_state, user)) {
            // The user has installed a callback and he's enabled on this port.
            PSEC_stack_state.on_mac_del_callbacks[user](isid, port, &mac_state->vid_mac, reason, PSEC_FORWARD_DECISION_GET(mac_state, user));
        }
    }
}

/******************************************************************************/
// PSEC_mac_del()
// Removes the MAC address from internal list and MAC table, and calls back those
// who are interested in this. Finally it checks if this delete gives rise to
// changing the CPU copy state.
/******************************************************************************/
static void PSEC_mac_del(vtss_isid_t isid, vtss_port_no_t port, psec_mac_state_t *mac_state, psec_del_reason_t reason)
{
    BOOL              is_zombie;
    psec_port_state_t *port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    PSEC_CRIT_ASSERT_LOCKED();

    // If deleting a zombie (an entry that is not in the MAC table due to S/W or H/W failure),
    // then we don't call back the user modules, since they have no clue that it exists, since
    // they have already been called back when it was determined that there was a S/W or
    // H/W failure. Back then, the entry was just marked as a zombie, but it wasn't really
    // deleted from the list.
    if ((mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED) ||
        (mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED)) {
        // We are deleting a zombie. Check to see if this is the last zombie on this port,
        // and if so, change the port state so that we possibly re-enable CPU copying again.
        psec_mac_state_t *temp_mac_state = port_state->macs;
        BOOL             zombie_found    = FALSE;
        while (temp_mac_state) {
            // We don't care about the one we're currently deleting.
            if (mac_state != temp_mac_state) {
                if ((temp_mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED) ||
                    (temp_mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED)) {
                    zombie_found = TRUE; // Still some zombies on this port.
                    break;
                }
            }
            temp_mac_state = temp_mac_state->next;
        }

        if (!zombie_found) {
            // We are about to delete the last zombie on this port. Clear the port zombie state flags,
            // so that we possibly re-enable CPU-copying on this port.
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_HW_ADD_FAILED;
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_SW_ADD_FAILED;
        }
        is_zombie = TRUE;
    } else {
        // The entry we're deleting is not a zombie.
        // Callback all users enabled on this port and tell them why we delete this entry.
        PSEC_del_callback(isid, port, port_state, mac_state, reason);

        // Deleting an entry that indeed *is* in the MAC table causes the limit not to be reached anymore.
        // Deleting a zombie, doesn't affect the LIMIT_REACHED state.
        port_state->flags &= ~PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
        is_zombie = FALSE;
    }

    // Now, if the reason this function is called is due to detection of a zombie, we
    // don't actually remove it from the list of MAC addresses on this port, because
    // we need to disable CPU-copying on the port for the duration of the zombie's hold-time.
    // So the PSEC_thread() will still need to count down its hold time and only remove it
    // when it reaches zero.
    if (reason == PSEC_DEL_REASON_HW_ADD_FAILED ||
        reason == PSEC_DEL_REASON_SW_ADD_FAILED) {
        // Special cases where the entry is actually not deleted. Instead a flag is
        // set in both the port's state and the entry that indicates that it couldn't
        // be added to the MAC table. This causes the subsequent call to the function
        // that checks whether to re-enable CPU copying on this port to disable it.
        if (reason == PSEC_DEL_REASON_HW_ADD_FAILED) {
            mac_state->flags  |= PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED;
            port_state->flags |= PSEC_PORT_STATE_FLAGS_HW_ADD_FAILED;
            // First call the function that potentially disables CPU copying on this port.
            PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);
            // Then delete this entry from the MAC module's MAC table if it's present there
            PSEC_mac_module_del(isid, mac_state);
        } else {
            // Since the MAC module couldn't add this in the first place, it's already not located in the MAC module's
            // MAC table.
            mac_state->flags  |= PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED;
            port_state->flags |= PSEC_PORT_STATE_FLAGS_SW_ADD_FAILED;
            // Since we've just taken a MAC address from the free list, and since it's still taken even
            // though the MAC module returned an error, we must call the cpu-copy check function
            // with MAC_ADDRESS_ALLOCATED, because allocating a MAC address potentially causes
            // all free entries to be taken by now.
            PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_MAC_ADDRESS_ALLOCATED, __LINE__);
        }
        // Clear the keep-blocked flag, so that we can age it.
        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED;

        // Tell the PSEC_thread() to remove this after some time.
        mac_state->age_or_hold_time_secs = PSEC_ZOMBIE_HOLD_TIME_SECS;

        // The number of MAC addresses actually in the H/W MAC table is one less now (but this entry stays in
        // our software-list).
        port_state->mac_cnt--;
    } else {
        // Unlink it from this port.
        if (mac_state->next) {
            mac_state->next->prev = mac_state->prev;
        }
        if (mac_state->prev == NULL) {
            port_state->macs = mac_state->next;
        } else {
            mac_state->prev->next = mac_state->next;
        }

        // Only count down the mac_cnt if this is not a zombie (because if it was a zombie,
        // then it's already not included in the mac_cnt.
        if (!is_zombie) {
            port_state->mac_cnt--;
        }

        // Move the entry to the free list and count up the number of free entries.
        mac_state->next = PSEC_mac_state_free_pool;
        mac_state->unique = 0; // No longer in use.
        PSEC_mac_state_free_pool = mac_state;
        PSEC_mac_states_left++;

        // First delete this entry from the MAC module's MAC table if it's present there.
        // In the rare case where the PSEC LIMIT module tells us to shut down a port,
        // it's done before the entry is actually added to the MAC module, but all
        // users have been notified that it eventually would (but then again - wouldn't).
        PSEC_mac_module_del(isid, mac_state);

        // Then check to see if this gave rise to re-enabling CPU-copying on this port.
        PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_MAC_ADDRESS_FREED, __LINE__);
    }

    // Check if this port has at least one MAC address that needs to be aged/held.
    PSEC_update_age_hold_enabledness(isid, port, port_state);
}

/******************************************************************************/
// PSEC_mac_del_all()
// Removes all MAC addresses on a specific port from the internal list and MAC
// table. Since it uses the PSEC_mac_del() function,
// all users that are enabled will be called back with the reason, and the
// secure learning state and CPU copying state will be refreshed.
/******************************************************************************/
static void PSEC_mac_del_all(vtss_isid_t isid, vtss_port_no_t port, psec_del_reason_t reason)
{
    psec_port_state_t *port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    psec_mac_state_t  *mac_state = port_state->macs;

    while (mac_state) {
        psec_mac_state_t *next_mac_state = mac_state->next;
        PSEC_mac_del(isid, port, mac_state, reason);
        mac_state = next_mac_state;
    }
}

/******************************************************************************/
// PSEC_local_port_valid()
/******************************************************************************/
static BOOL PSEC_local_port_valid(vtss_port_no_t port)
{
    // In case someone changes VTSS_PORT_NO_START back to 1, we better survive that
    // so tell lint to not report "Relational operator '<' always evaluates to 'false'"
    // and "non-negative quantity is never less than zero".
    /*lint -e{685, 568} */
    if (port < VTSS_PORT_NO_START || port - VTSS_PORT_NO_START >= port_isid_port_count(VTSS_ISID_LOCAL) || port_isid_port_no_is_stack(VTSS_ISID_LOCAL, port)) {
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************/
// PSEC_frame_rx()
// Well in fact, it's not only learn frames that we receive here.
// With the software-based aging, we receive all frames from a particular
// source port when the CPU_COPY flag is set in the MAC table.
// We may have a problem with DoS attacks when the CPU_COPY flag is set:
//   Also frames towards the given source port are forwarded to the CPU
//   extraction queues, extracted by the FDMA, and dispatched in the packet
//   module, but since there (probably) ain't any subscribers to these frames,
//   they are discarded. But they end up in the same extraction queue as the
//   management traffic, and may thus cause management traffic to be discarded
//   due to queue overflow. And we won't clear the CPU_COPY flag if a frame
//   destined for the MAC-address is received, since that doesn't guarantee
//   that the specific client is still there.
/******************************************************************************/
static BOOL PSEC_frame_rx(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    vtss_port_no_t port = rx_info->port_no;
    vtss_vid_mac_t vid_mac;
    BOOL           forward_to_master = FALSE;
    BOOL           frame_consumed    = FALSE;
    vtss_isid_t    master_isid;

    // If the frame was not received on a front port, ignore it.
    if (!PSEC_local_port_valid(port)) {
        T_D("Received learn frame on invalid source port number (%u). Ignoring it", port);
        return frame_consumed;
    }

    // Gotta reset the vid_mac structure to zeroes in order to be able to memcmp()
    // when comparing to other MACs.
    // The reason is that the vid_mac->mac is 6 bytes, which is padded with 2 bytes
    // to make the next field dword aligned.
    memset(&vid_mac, 0, sizeof(vid_mac));
    vid_mac.vid = rx_info->tag.vid;
    memcpy(vid_mac.mac.addr, &frm[6], sizeof(vid_mac.mac.addr));

    PSEC_CRIT_ENTER();

    // Check if we're enabled on this port, and if so, check
    // that it's not subject to being dropped by the rate-limiter.
    if (VTSS_PORT_BF_GET(PSEC_copy_to_master, port)) {
        // We're PSEC-enabled.

        // Check whether we should forward this frame to current master.
        forward_to_master = !psec_rate_limit_drop(port, &vid_mac);

        // We gotta determine whether to discard the frame or not. Unfortunately,
        // PSEC is a centralized module, which means that slaves don't have insight
        // into MAC addresses and their forwarding decision. Luckily, the frame
        // consumption is only of relevance on the master, because we potentially
        // need only to disallow e.g. IP frames from getting further in the packet
        // rx chain on the master.
        if ((master_isid = msg_master_isid()) != VTSS_ISID_UNKNOWN) {
            // We're master, so PSEC_stack_state is valid.
            psec_port_state_t *port_state = &PSEC_stack_state.switch_state[master_isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
            // We need to consult the MAC table array before checking the port state, because
            // the port state may say block while the MAC state says forward, and the MAC state then wins.
            psec_mac_state_t *mac_state = port_state->macs;
            while (mac_state) {
                if (memcmp(&vid_mac, &mac_state->vid_mac, sizeof(vid_mac)) == 0) {
                    break;
                }
                mac_state = mac_state->next;
            }

            if (mac_state) {
                frame_consumed = mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED;
            } else if ((port_state->ena_mask & port_state->mode_mask) || (port_state->flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN)) {
                // Port should be kept blocked.
                frame_consumed = TRUE;
            }
        }
    }

    PSEC_CRIT_EXIT();

    if (forward_to_master) {
        // The rate-limiter tells us to send it to the master.
        // We don't need the semaphore anymore, since PSEC_msg_tx_frame() has all the info it needs in the args
        PSEC_msg_tx_frame(port, &vid_mac, 0);
    }

    T_N("<slv>:%u: Rx Frame (mac=%s, vid=%u, forw. to master=%s, frame consumed=%s)", port, misc_mac2str(&frm[6]), rx_info->tag.vid, forward_to_master ? "yes" : "no", frame_consumed  ? "yes" : "no");
    T_R_HEX(frm, 16);

    return frame_consumed;
}

/******************************************************************************/
// PSEC_learn_frame_rx_register()
/******************************************************************************/
static void PSEC_learn_frame_rx_register(void)
{
    BOOL           chg                    = FALSE;
    BOOL           at_least_one_with_copy = FALSE;
    BOOL           old_copy, new_copy;
    port_iter_t    pit;

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

    while (port_iter_getnext(&pit)) {
        old_copy = VTSS_PORT_BF_GET(PSEC_frame_rx_filter.src_port_mask, pit.iport);
        new_copy = VTSS_PORT_BF_GET(PSEC_copy_to_master,                pit.iport);

        if (new_copy) {
            at_least_one_with_copy = TRUE;
        }

        if (new_copy != old_copy) {
            chg = TRUE;
            VTSS_PORT_BF_SET(PSEC_frame_rx_filter.src_port_mask, pit.iport, new_copy);
        }
    }

    if (chg) {
        // A change is detected. Gotta register, re-register, or un-register our packet filter.
        if (at_least_one_with_copy) {
            // At least one port is still in copy-to-master mode.
            if (PSEC_frame_rx_filter_id) {
                // Re-register filter
                (void)packet_rx_filter_change(&PSEC_frame_rx_filter, &PSEC_frame_rx_filter_id);
            } else {
                //  Register new filter
                (void)packet_rx_filter_register(&PSEC_frame_rx_filter, &PSEC_frame_rx_filter_id);
            }
        } else {
            // No more ports in copy-to-master-mode.
            // Unregister filter.
            if (PSEC_frame_rx_filter_id) {
                (void)packet_rx_filter_unregister(PSEC_frame_rx_filter_id);
                PSEC_frame_rx_filter_id = NULL;
            }
        }
    }
}

/******************************************************************************/
// PSEC_frame_rx_init()
// Receive all kinds of frames. See note in PSEC_frame_rx() header.
/******************************************************************************/
static PSEC_INLINE void PSEC_frame_rx_init(void)
{
    psec_rate_limit_init();
    memset(&PSEC_frame_rx_filter, 0, sizeof(PSEC_frame_rx_filter));
    PSEC_frame_rx_filter.modid = VTSS_MODULE_ID_PSEC;
    PSEC_frame_rx_filter.match = PACKET_RX_FILTER_MATCH_SRC_PORT;
    PSEC_frame_rx_filter.prio  = PACKET_RX_FILTER_PRIO_BELOW_NORMAL; // We need to be able to stop all but protocol frames
    PSEC_frame_rx_filter.cb    = PSEC_frame_rx;
    // Do not register the filter until we get enabled on at least one port.
}

/******************************************************************************/
// PSEC_mac_address_add_failed()
/******************************************************************************/
static void PSEC_mac_address_add_failed(vtss_vid_mac_t *vid_mac)
{
#ifdef VTSS_SW_OPTION_SYSLOG
    S_W("MAC Table Full. Could not add <VLAN ID = %u, MAC Addr = %s> to MAC Table", vid_mac->vid, misc_mac2str(vid_mac->mac.addr));
#endif
    T_W("MAC Table Full. Could not add <VLAN ID = %u, MAC Addr = %s> to MAC Table", vid_mac->vid, misc_mac2str(vid_mac->mac.addr));
}

/******************************************************************************/
// PSEC_lookup()
/******************************************************************************/
static psec_mac_state_t *PSEC_lookup(vtss_isid_t *looked_up_isid, vtss_port_no_t *looked_up_port, vtss_vid_mac_t *vid_mac)
{
    vtss_isid_t       isid;
    psec_port_state_t *port_state;
    psec_mac_state_t  *mac_state;

    for (isid = 0; isid < VTSS_ISID_CNT; isid++) {
        psec_switch_state_t *switch_state = &PSEC_stack_state.switch_state[isid];
        port_iter_t pit;
        if (!switch_state->switch_exists) {
            continue;
        }
        (void)port_iter_init(&pit, NULL, isid + VTSS_ISID_START, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            port_state = &switch_state->port_state[pit.iport - VTSS_PORT_NO_START];
            mac_state  = port_state->macs;
            while (mac_state) {
                if (memcmp(vid_mac, &mac_state->vid_mac, sizeof(*vid_mac)) == 0) {
                    *looked_up_isid = isid + VTSS_ISID_START;
                    *looked_up_port = pit.iport;
                    return mac_state;
                }
                mac_state = mac_state->next;
            }
        }
    }
    return NULL;
}

/******************************************************************************/
// PSEC_age_frame_seen()
/******************************************************************************/
static PSEC_INLINE void PSEC_age_frame_seen(vtss_isid_t isid, vtss_port_no_t port, psec_mac_state_t *mac_state)
{
    if (mac_state->flags & PSEC_MAC_STATE_FLAGS_CPU_COPYING) {
        mac_state->flags |=  PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN;
        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING;
        (void)PSEC_mac_module_chg(isid, port, mac_state, __LINE__);
    }
}

/******************************************************************************/
// PSEC_mac_chg()
// Find the enabled user that contributes with the 'worst' forward decision.
// By 'worst' is meant in the order ('worst' in the bottom):
//   Forward
//   Block
//   Keep blocked
// Then update age/hold-times if needed and change or add the MAC address entry
// in the MAC table, if needed.
//
// This function will NOT determine whether CPU copying of learn frames should
// occur or not.
//
// The function can only return FALSE if it calls PSEC_mac_module_chg() and
// if that call fails, which it only can (I think, if we're no longer master),
// since the MAC address that we change already exists in the MAC module's
// list of MAC addresses.
// In all other cases, it returns TRUE.
/******************************************************************************/
static BOOL PSEC_mac_chg(vtss_isid_t isid, vtss_port_no_t port, psec_port_state_t *port_state, psec_mac_state_t *mac_state, BOOL update_age_hold_times_only)
{
    psec_add_method_t new_add_method     = PSEC_ADD_METHOD_FORWARD;
    psec_users_t      user;
    u32               shortest_age_time  = PSEC_AGE_TIME_MAX + 1;
    u32               shortest_hold_time = PSEC_HOLD_TIME_MAX;
    BOOL              update_mac_entry   = FALSE;
    BOOL              result             = TRUE;

    if ((mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED) ||
        (mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED)) {
        // We cannot change this entry in the MAC table, since it's not
        // there. Also, when one of these flags are set, the forwarding
        // decision set by the user modules is of no good, and the
        // age/hold-time counter is counting the number of seconds to keep
        // the port closed rather than anything user-defined. When
        // that timer expires, this zombie entry will be deleted and the port
        // will be re-enabled.
        return TRUE;
    }

    // All users' forward decision bits are encoded in an u8 array, so we need to use a macro to handle them.
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        if (PSEC_USER_ENA_GET(port_state, user)) {
            psec_add_method_t add_method = PSEC_FORWARD_DECISION_GET(mac_state, user);
            if (add_method > new_add_method) {
                new_add_method = add_method;
            }
            if (PSEC_stack_state.aging_period_secs[user] != 0 && PSEC_stack_state.aging_period_secs[user] < shortest_age_time) {
                // The age period is the shortest non-zero amongst the enabled users.
                // If all have disabled aging (0), then we resort to that.
                shortest_age_time = PSEC_stack_state.aging_period_secs[user];
            }
            if (PSEC_stack_state.hold_time_secs[user] < shortest_hold_time) {
                // The hold times cannot be 0 (disabled)
                shortest_hold_time = PSEC_stack_state.hold_time_secs[user];
            }
        }
    }

    if (shortest_age_time == PSEC_AGE_TIME_MAX + 1) {
        // All enabled users have disabled aging
        shortest_age_time = 0;
    }

    if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
        psec_add_method_t cur_add_method;
        // Synthesize the current add method to see if we're gonna
        // update it in the MAC table.
        if (mac_state->flags & PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED) {
            cur_add_method = PSEC_ADD_METHOD_KEEP_BLOCKED;
        } else if (mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED) {
            cur_add_method = PSEC_ADD_METHOD_BLOCK;
        } else {
            cur_add_method = PSEC_ADD_METHOD_FORWARD;
        }

        if (update_age_hold_times_only && cur_add_method != new_add_method) {
            T_E("Internal error");
        }

        switch (cur_add_method) {
        case PSEC_ADD_METHOD_FORWARD:
            switch (new_add_method) {
            case PSEC_ADD_METHOD_FORWARD:
                // Going from forwarding to forwarding.
                // If the previous age period is greater than the new,
                // then use the new (otherwise keep going from where the
                // previous took off).
                // If the new age period is disabled, then make sure
                // CPU copying gets disabled.
                if ((mac_state->age_or_hold_time_secs == 0 && shortest_age_time != 0) || mac_state->age_or_hold_time_secs > shortest_age_time) {
                    // Adjust the age time if necessary.
                    mac_state->age_or_hold_time_secs = shortest_age_time;
                    if (shortest_age_time == 0) {
                        if (mac_state->flags & PSEC_MAC_STATE_FLAGS_CPU_COPYING) {
                            mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING;
                            if (update_age_hold_times_only) {
                                // Since we're not going into the update loop below, when
                                // update_age_hold_times_only is set, we need to disable CPU copying
                                // here.
                                (void)PSEC_mac_module_chg(isid, port, mac_state, __LINE__);
                            } else {
                                update_mac_entry = TRUE;
                            }
                        }
                    }
                }
                break;

            case PSEC_ADD_METHOD_BLOCK:
                // Going from forwarding to blocking. Update the hold time.
                mac_state->age_or_hold_time_secs = shortest_hold_time;
            // Fall through

            case PSEC_ADD_METHOD_KEEP_BLOCKED:
            default:
                // Going from forwarding to blocking or keep blocking.
                // Never copy to CPU, but always update the MAC entry.
                mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING;
                update_mac_entry = TRUE;
                break;
            }
            break;

        case PSEC_ADD_METHOD_BLOCK:
            switch (new_add_method) {
            case PSEC_ADD_METHOD_FORWARD:
                // Going from block to forward
                // Always update the MAC entry in the MAC table,
                // but also restart aging and pretend that the frame
                // was received OK in the current age period.
                mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING;
                mac_state->flags |= PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN;
                mac_state->age_or_hold_time_secs = shortest_age_time;
                update_mac_entry = TRUE;
                break;

            case PSEC_ADD_METHOD_BLOCK:
                // Going from block to block. Adjust the remaining hold time
                // if needed.
                if (mac_state->age_or_hold_time_secs > shortest_hold_time) {
                    mac_state->age_or_hold_time_secs = shortest_hold_time;
                }
                break;

            case PSEC_ADD_METHOD_KEEP_BLOCKED:
#ifdef PSEC_FIX_GNATS_6935
                // Gotta update the CPU copy flags.
                update_mac_entry = TRUE;
#endif
                break;


            default:
                // Nothing to do when going from blocked to keep blocked.
                break;
            }
            break;

        case PSEC_ADD_METHOD_KEEP_BLOCKED:
        default:
            switch (new_add_method) {
            case PSEC_ADD_METHOD_FORWARD:
                // Going from keep-blocked to forward.
                // Always update the MAC entry in the MAC table,
                // but also restart aging and pretend that the frame
                // was received OK in the current age period.
                mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING;
                mac_state->flags |= PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN;
                mac_state->age_or_hold_time_secs = shortest_age_time;
                update_mac_entry = TRUE;
                break;

            case PSEC_ADD_METHOD_BLOCK:
                // Going from keep-blocked to blocked.
                // Start the hold timer.
                mac_state->age_or_hold_time_secs = shortest_hold_time;
                break;

            case PSEC_ADD_METHOD_KEEP_BLOCKED:
            default:
                // Nothing to do
                break;
            }
            break;
        } /* switch (cur_add_method) */
    } else {
        // The MAC address is currently not in the table (it's brandnew)
        update_mac_entry = TRUE;
        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_CPU_COPYING; // Superfluous.
        switch (new_add_method) {
        case PSEC_ADD_METHOD_FORWARD:
            // Pretend an age frame is seen in the first period.
            mac_state->flags |= PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN;
            mac_state->age_or_hold_time_secs = shortest_age_time;
            break;

        case PSEC_ADD_METHOD_BLOCK:
            mac_state->age_or_hold_time_secs = shortest_hold_time;
            break;

        case PSEC_ADD_METHOD_KEEP_BLOCKED:
        default:
            break;
        }
    }

    if (!update_age_hold_times_only) {
        switch (new_add_method) {
        case PSEC_ADD_METHOD_FORWARD:
            mac_state->flags &= ~(PSEC_MAC_STATE_FLAGS_BLOCKED | PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED);
            break;

        case PSEC_ADD_METHOD_KEEP_BLOCKED:
            // KEEP_BLOCKED will keep it in the table indefinitely (not subject to 'aging').
            mac_state->flags |= PSEC_MAC_STATE_FLAGS_BLOCKED | PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED;

            // If the port for which we're adding this has told us to force CPU copy,
            // it's due to a bug in EStaX34, controlled by PSEC_FIX_GNATS_6935.
            // In this mode, BPDUs are not forwarded to the CPU if the MAC address
            // that sends these CPUs are in the MAC table with CPU copy bit cleared.
#ifdef PSEC_FIX_GNATS_6935
            if (port_state->force_cpu_copy_mask) {
                mac_state->flags |= PSEC_MAC_STATE_FLAGS_CPU_COPYING;
            }
#endif
            break;

        case PSEC_ADD_METHOD_BLOCK:
            // BLOCK is used to block the entry for a while (subject to 'aging').
            mac_state->flags |= PSEC_MAC_STATE_FLAGS_BLOCKED;
            mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED;
            break;

        default:
            T_E("Invalid decision");
            return FALSE;
        }

        if (update_mac_entry) {
            result = PSEC_mac_module_chg(isid, port, mac_state, __LINE__);
        }
    }

    // Check if this port has at least one MAC address that needs to be aged/held.
    PSEC_update_age_hold_enabledness(isid, port, port_state);

    return result;
}

/******************************************************************************/
// PSEC_add_callback()
// When calling back the users, we must have released our mutex, otherwise
// a deadlock can occur, as follows:
//   The psec_msg_rx() thread receives a new MAC address from a slave switch
//   while another switch gets deleted, a port goes down, or a user module
//   decides that a given MAC address is no longer valid (the NAS module may
//   decide that based on authentication result or timeout).
//   And suppose that one of the MAC Add callback users is e.g. NAS, which is
//   currently handling its statemachines and therefore owns its own mutex.
//   The "new MAC" event causes the PSEC module to grab its own mutex and start
//   calling back the enabled users. Since one of them is the NAS module, the
//   PSEC module will wait for the NAS module to release its mutex so that it
//   can be grabbed by the PSEC module (in NAS's callback function). Now, suppose
//   that NAS figures out that a given MAC address must be deleted from the MAC
//   table. Then it calls the psec_mgmt_mac_del() function. This function will
//   attempt to get the PSEC mutex, which is already taken by the thread waiting
//   for the NAS module => DEADLOCK.
// To avoid the deadlock, we make use of a unique number which is set before
// we release the mutex and checked after we get it back. If the two numbers
// match, nothing has happened to this mac_state, otherwise it has been deleted
// (and possibly reused) while calling back the users. The @unique member of
// mac_state is the unique number we check on.
// The biggest problem with the @unique-way of ensuring state-integrity is that
// race conditions may arise.
// For instance, if a new MAC address was received just as a port was on its way down,
// then a user's delete callback function may be called before the add callback is
// really called. This may cause the user module to have a wrong picture of the
// MAC addresses learned on a port.
// There are solutions to such problems, but they require fundamental changes to the
// implementation: All state changes could be handled from the same thread rather
// than allowing different threads to call-in (psec_mgmt_mac_add(), psec_mgmt_mac_del(),
// switch-up/down, port up/down, etc.) and alter the state directly.
// Having a single thread handle state changes poses other problems, though. Suddenly
// for example, psec_mgmt_mac_add() would be asynchronous rather than synchronous - unless
// special provisioning to make it seem synchronous was taken.
// For now, we live with the fact that race conditions may occur.
/******************************************************************************/
static PSEC_INLINE BOOL PSEC_add_callback(psec_users_t calling_user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_port_state_t *port_state, psec_mac_state_t *mac_state, psec_add_method_t add_method_by_calling_user, psec_add_action_t *worst_case_add_action)
{
    psec_on_mac_add_callback_f *auto_on_mac_add_callbacks[PSEC_USER_CNT];
    psec_port_state_t           auto_port_state;
    psec_mac_state_t            auto_mac_state;
    psec_users_t                user;

    PSEC_CRIT_ASSERT_LOCKED();

    // Copy to stack
    auto_port_state           = *port_state;
    auto_mac_state            = *mac_state;
    *worst_case_add_action    = PSEC_ADD_ACTION_NONE;
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        auto_on_mac_add_callbacks[user] = PSEC_stack_state.on_mac_add_callbacks[user];
    }

    // Lint finds it odd that we first exit then enter the mutex.
    // Lint says: "(Warning -- A thread mutex that had not been locked is being unlocked)"
    /*lint -e(455) */
    PSEC_CRIT_EXIT();

    // At this point, others may manipulate the mac_state, which is *so* bad.
    // This can happen if e.g. the switch or port goes down or a user changes
    // his enabledness.
    // After the callbacks, we enter the critical section again and check
    // (using the @unique member) whether the entry is still the same as we thought it was.
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        if (auto_on_mac_add_callbacks[user] && PSEC_USER_ENA_GET(&auto_port_state, user)) {
            psec_add_method_t user_add_method;
            // This user is enabled on the port. Ask him what to do with the new MAC address.
            psec_add_action_t user_add_action = PSEC_ADD_ACTION_NONE;
            // Since we've just allocated a MAC address and the arg to on_mac_add_callbacks is the number of MAC addresses already
            // in the table, we must subtract 1 from port_state->mac_cnt in the callback.
            if (user != calling_user) {
                user_add_method = auto_on_mac_add_callbacks[user](isid, port, vid_mac, auto_port_state.mac_cnt - 1, &user_add_action);
            } else {
                user_add_method = add_method_by_calling_user;
            }
            PSEC_FORWARD_DECISION_SET(&auto_mac_state, user, user_add_method);

            // Gotta run through all users even if user_add_action is "shut-down port", because we'll call
            // *all* back with the On-MAC-Del method if that's the case.
            if (user_add_action > *worst_case_add_action) {
                *worst_case_add_action = user_add_action;
            }
        }
    }

    PSEC_CRIT_ENTER();

    if (auto_mac_state.unique == mac_state->unique) {
        // Hey - no change in our mac_state during the callback. Update it and continue.
        memcpy(mac_state->forward_decision_mask, auto_mac_state.forward_decision_mask, sizeof(mac_state->forward_decision_mask));
        // Lint finds it odd that we first exit then enter the mutex.
        // Lint says: "(Warning -- A thread mutex has been locked but not unlocked [Reference: file ../../vtss_appl/psec/psec.c: line 1470])
        /*lint -e{454} */
        return TRUE;
    } else {
        // The mac_state was deleted or reused for another MAC during the above callback.
        T_I("%d:%d: <MAC, VID>=<%s, %d> was changed during add callback", isid, iport2uport(port), misc_mac2str(vid_mac->mac.addr), vid_mac->vid);
        // Lint finds it odd that we first exit then enter the mutex.
        // Lint says: "(Warning -- A thread mutex has been locked but not unlocked [Reference: file ../../vtss_appl/psec/psec.c: line 1470])
        /*lint -e{454} */
        return FALSE;
    }
}

/******************************************************************************/
// PSEC_do_add_mac()
/******************************************************************************/
static vtss_rc PSEC_do_add_mac(psec_users_t calling_user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_add_method_t add_method_by_calling_user)
{
    psec_switch_state_t *switch_state;
    psec_port_state_t   *port_state;
    psec_mac_state_t    *mac_state;
    psec_add_action_t   worst_case_add_action;

    if (!PSEC_mac_states_left) {
        return PSEC_ERROR_OUT_OF_MAC_STATES;
    }

    switch_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START];

    // Only add it if the switch actually exists. The reason why we can end here is
    // that the entry may have been stored in the H/W Rx Queue before we were
    // notified that the switch was deleted.
    if (!switch_state->switch_exists) {
        return PSEC_ERROR_SWITCH_IS_DOWN;
    }

    port_state = &switch_state->port_state[port - VTSS_PORT_NO_START];

    // The port must have link and the limit must not have been reached and the port
    // must not have been shut-down by the PSEC LIMIT module, and at least one user must
    // be enabled on this port.
    // It is perfectly normal to get here even if port is shut-down or the limit is reached,
    // i.e. even if CPU copying is disabled. The reason is that broadcast frames are sent
    // to the CPU due to a statically entered MAC address in the MAC table.
    if (!(port_state->flags & PSEC_PORT_STATE_FLAGS_LINK_UP)) {
        return PSEC_ERROR_LINK_IS_DOWN;
    }

    if (port_state->flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) {
        return PSEC_ERROR_LIMIT_IS_REACHED;
    }

    if (port_state->flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN) {
        return PSEC_ERROR_PORT_IS_SHUT_DOWN;
    }

    if (port_state->ena_mask == 0) {
        return PSEC_ERROR_NO_USERS_ENABLED;
    }

    // Port is up, limit is not reached, port is not administratively shut down,
    // and at least one user module is enabled.
    // Ask all enabled modules on this port how we should treat this new entry.

    // Allocate a new entry and attach it to the port. The function will clear
    // all fields and set those that must be set.
    if ((mac_state = PSEC_mac_state_alloc(isid, port, vid_mac)) == NULL) {
        // This should succeed.
        T_E("Internal error");
        return PSEC_ERROR_INTERNAL_ERROR;
    }

    // We gotta step out of the PSEC_CRIT before calling back to avoid deadlocks.
    // This is a bit cumbersome for this module, because not only is the MAC add
    // callback an event, it also requires user-feedback, and - even worse - the
    // recently allocated mac_state may be deleted by switch delete/port link-down
    // events. This is handled through the mac_state's @unique member. Sigh!
    if (!PSEC_add_callback(calling_user, isid, port, vid_mac, port_state, mac_state, add_method_by_calling_user, &worst_case_add_action)) {
        // A change occurred while the users were called back. Stop!
        return PSEC_ERROR_STATE_CHG_DURING_CALLBACK;
    }

    switch (worst_case_add_action) {
    case PSEC_ADD_ACTION_NONE:
        // Hey, the PSEC LIMIT is OK with this entry.
        break;

    case PSEC_ADD_ACTION_LIMIT_REACHED:
        // Well, add this entry, but stop CPU copying.
        T_D("Limit Reached");
        port_state->flags |= PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
        break;

    case PSEC_ADD_ACTION_SHUT_DOWN:
        T_D("Port Shut Down");
        // Ouch. This one caused an overflow of allowed MAC addresses.
        // We have to delete all entries on this port. The port will
        // not be usable again until the psec_mgmt_reopen_port() function is
        // called (or this switch is booted or stack changes master).
        port_state->flags |= PSEC_PORT_STATE_FLAGS_SHUT_DOWN;
        // The following call will also cause the CPU copying state to be updated.
        // We need to mask out the calling user before calling this function, because
        // otherwise he would get called back, and the calling user would rather
        // get it in the return code to psec_mgmt_mac_add().
        // And we're already damn sure that the user is actually enabled on this port.
        if (calling_user != PSEC_USER_CNT) {
            // Avoid "Warning -- Constant value Boolean" Lint error
            /*lint -e{506} */
            PSEC_USER_ENA_SET(port_state, calling_user, FALSE);
        }
        PSEC_mac_del_all(isid, port, PSEC_DEL_REASON_PORT_SHUT_DOWN);
        if (calling_user != PSEC_USER_CNT) {
            // Avoid "Warning -- Constant value Boolean" Lint error
            /*lint -e{506} */
            PSEC_USER_ENA_SET(port_state, calling_user, TRUE);
        }
        // Nothing more to do here.
        return PSEC_ERROR_PORT_IS_SHUT_DOWN;

    default:
        T_E("User-module returned invalid action");
        return PSEC_ERROR_INTERNAL_ERROR;
    }

    // If we get here, we should try to add the MAC address to the MAC module.
    // This function will also determine how the MAC address should be added,
    // i.e. blocked or forwarding, based on the user-decisions, and it will
    // determine the aging/hold.
    if (PSEC_mac_chg(isid, port, port_state, mac_state, FALSE)) {
        // Update the cpu-copy enable/disable state
        PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_MAC_ADDRESS_ALLOCATED, __LINE__);
    } else {
        // The add failed due to the MAC module (S/W fail).
        // The call of the following function will also update the cpu-copying enable/disable.
        PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_SW_ADD_FAILED);
    }
    return VTSS_RC_OK;
}

/******************************************************************************/
// PSEC_handle_frame_reception()
/******************************************************************************/
static PSEC_INLINE void PSEC_handle_frame_reception(psec_msg_frame_t *msg, vtss_isid_t isid)
{
    vtss_port_no_t   port = msg->port;
    psec_mac_state_t *mac_state;
    vtss_port_no_t   looked_up_port;
    vtss_isid_t      looked_up_isid;
    BOOL             add_new = TRUE;

    T_N("Frame received from port %d:%d, smac=%s, vid=%d, learn=%d", isid, port, misc_mac2str(msg->vid_mac.mac.addr), msg->vid_mac.vid, msg->is_learn_frame);

    // Received a frame, which is now forwarded to the master.
    // Check to see if we have that MAC address in our list already.
    mac_state = PSEC_lookup(&looked_up_isid, &looked_up_port, &msg->vid_mac);
    if (mac_state) {
        // Already assumed added to the MAC table.
        // Check if we received it on a new port or on another VID.
        if (looked_up_isid != isid || looked_up_port != port) {
            // Received it on another port. Unlink the current.
            PSEC_mac_del(looked_up_isid, looked_up_port, mac_state, PSEC_DEL_REASON_STATION_MOVED);
        } else if (msg->is_learn_frame) {
            // If it's a learn frame, then the reason for getting here is twofold:
            // 1) The client got to send two or more frames before we reacted on the first
            // 2) There wasn't room in the MAC table
            add_new = FALSE; // Do not give rise to adding it again.
            if ((mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED) ||
                (mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED)) {
                // Silently discard this, because we're already aware that this is
                // a MAC address that cannot be added to the MAC table (hash overflow or MAC module failure).
            } else if (msg_uptime_get(VTSS_ISID_LOCAL) - mac_state->creation_time_secs >= 7) {
                // It's more than 7 seconds ago we added this MAC address to the table.
                // For testing, use these 5 MAC addresses, which all map to row 21 of the MAC table.
                // 00-00-00-00-00-05 1
                // 00-00-00-00-08-04 1
                // 00-00-00-00-10-07 1
                // 00-00-00-00-18-06 1
                // 00-00-00-00-20-01 1
                // Only report this once per MAC address per hold time.
                PSEC_mac_address_add_failed(&mac_state->vid_mac);
                PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_HW_ADD_FAILED);
            }
        } else {
            // It's not a learn frame, but we do have information stored about this MAC address.
            // This means that we're probably aging the entry. Stop the CPU copying.
            PSEC_age_frame_seen(isid, port, mac_state);
            add_new = FALSE;
        }
    }

    if (add_new) {
        // Gotta check if not at least one user module wants the port to be blocking.
        // If so, we silently discard this frame, because all MAC addresses in that
        // case must be learned through the psec_mgmt_mac_add() function call,
        // which in turn then calls all user modules and asks for their opinion.
        psec_port_state_t *port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[msg->port - VTSS_PORT_NO_START];
        if (port_state->ena_mask & port_state->mode_mask) { // Yes, it's a bitwise AND
            // In fact this should not happen so often on chipsets other than E-StaX-34 due to
            // the BPDU-redirect error (search for PSEC_FIX_GNATS_6935 in psec.h and this file).
            add_new = FALSE;
        }
    }

    // Only add it if it's not discarded by the above checks (add_new) and if there're
    // state machines left. If there aren't any state machines left, then this MAC
    // address is one that were stored in the H/W Rx Queue before CPU copying got to
    // be turned off.
    if (add_new) {
        (void)PSEC_do_add_mac(PSEC_USER_CNT /* Indicates that all enabled users should be asked */, isid, msg->port, &msg->vid_mac, PSEC_ADD_METHOD_FORWARD /* Dummy method */);
    }
}

/******************************************************************************/
// PSEC_msg_rx()
/******************************************************************************/
static BOOL PSEC_msg_rx(void *contxt, const void *const the_rxd_msg, size_t len, vtss_module_id_t modid, ulong isid)
{
    psec_msg_t *rx_msg = (psec_msg_t *)the_rxd_msg;

    T_N("msg_id: %d, %s, ver: %u, len: %zd, isid: %u", rx_msg->hdr.msg_id, PSEC_msg_id_to_str(rx_msg->hdr.msg_id), rx_msg->hdr.version, len, isid);

    // Check if we support this version of the message. If not, print a warning and return.
    if (rx_msg->hdr.version != PSEC_MSG_VERSION) {
        T_W("Unsupported version of the message (%u)", rx_msg->hdr.version);
        return TRUE;
    }

    switch (rx_msg->hdr.msg_id) {
    case PSEC_MSG_ID_MST_TO_SLV_RATE_LIMIT_CFG: {
        psec_msg_rate_limit_cfg_t *msg = &rx_msg->u.rate_limit_cfg;
        PSEC_CRIT_ENTER();
        psec_rate_limit_cfg_set(&msg->rate_limit);
        PSEC_CRIT_EXIT();
        break;
    }

    case PSEC_MSG_ID_MST_TO_SLV_PORT_CFG: {
        // Set what and when to copy to master.
        psec_msg_port_cfg_t *msg = &rx_msg->u.port_cfg;
        if (!PSEC_local_port_valid(msg->port)) {
            break;
        }

        // Since we call the PSEC_learn_frame_rx_register(), which in turn waits
        // for the packet_cfg crit, we may end up in a deadlock if we used
        // PSEC_CRIT_ENTER() here, because it may be that the following sequence of events
        // takes place:
        //  1) Right here we take and get psec_crit.
        //  2) A learn frame arrives in the packet module, and packet_cfg crit gets taken in RX_dispatch()
        //  3) RX_dispatch() calls back PSEC module (PSEC_frame_rx()), which starts by taking the psec_crit, that is, it waits for this piece of code to finish.
        //  4) We call PSEC_learn_frame_rx_register(), which waits for packet_cfg crit, which is already taken by RX_dispatch().
        //  5) Deadlock!
        // Solution: Let's assume that we take psec_crit and update PSEC_copy_to_master[] array, release psec_crit and call PSEC_learn_frame_rx_register().
        // There are two things to consider here:
        //   a) Protection of PSEC_frame_rx_filter_id. This is inherintly protected since the PSEC_learn_frame_rx_register() can only be called
        //      from within the PSEC_msg_rx() function, which is in the Msg Rx thread context.
        //   b) Protection of the PSEC_copy_to_master[] array. Only one thread can write that array (after boot), and that's the this function (PSEC_msg_rx()).
        //      Whether PSEC_frame_rx() reads a 0 or a 1 from PSEC_copy_to_master[] when it receives a frame is not critical. It's critical, however, that
        //      PSEC_learn_frame_rx_register() actually detects changes and calls the packet_rx_filter_register()/unregister() function appropriately, but
        //      - again - since that function is only called from this function (PSEC_msg_rx()), and since this function can only be called from the Msg Rx
        //      thread, we should be safe.
        PSEC_CRIT_ENTER();
        VTSS_PORT_BF_SET(PSEC_copy_to_master, msg->port, msg->copy_to_master);
        // Ask the rate-limiter to clear its own filter for this port.
        psec_rate_limit_filter_clr(msg->port);
        PSEC_CRIT_EXIT();
        PSEC_learn_frame_rx_register(); // Figures out changes itself.
        break;
    }

    case PSEC_MSG_ID_MST_TO_SLV_SWITCH_CFG: {
        // Set what and when to copy to master.
        psec_msg_switch_cfg_t *msg = &rx_msg->u.switch_cfg;

        PSEC_CRIT_ENTER();
        memcpy(PSEC_copy_to_master, msg->copy_to_master, sizeof(PSEC_copy_to_master));
        // Ask the rate-limiter to clear its own filter for the whole switch.
        psec_rate_limit_filter_clr(VTSS_PORTS + VTSS_PORT_NO_START);
        PSEC_CRIT_EXIT();
        PSEC_learn_frame_rx_register(); // Figures out changes itself.
        break;
    }

    case PSEC_MSG_ID_SLV_TO_MST_FRAME: {
        if (!msg_switch_is_master()) {
            return TRUE;
        }
        PSEC_CRIT_ENTER();
        PSEC_handle_frame_reception(&rx_msg->u.frame, isid);
        PSEC_CRIT_EXIT();
        break;
    }

    default:
        T_D("Unknown message ID: %d", rx_msg->hdr.msg_id);
        break;
    }
    return TRUE;
}

/******************************************************************************/
// PSEC_msg_rx_init()
/******************************************************************************/
static void PSEC_msg_rx_init(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = PSEC_msg_rx;
    filter.modid = VTSS_MODULE_ID_PSEC;
    (void)msg_rx_filter_register(&filter);
}

/******************************************************************************/
// PSEC_link_state_change_callback()
/******************************************************************************/
static void PSEC_link_state_change_callback(vtss_isid_t isid, vtss_port_no_t port, port_info_t *info)
{
    if (!msg_switch_exists(isid)) {
        // Note that if the switch doesn't exist at all, we
        // will also simply return, and not react on switch-deletes here. Switch-delete events
        // are therefore handled separately in then INIT_CMD_SWITCH_DEL section of psec_init().
        return;
    }

    if (info->stack) {
        return; // We don't care about stack ports.
    }

    PSEC_CRIT_ENTER();
    if (info->link) {
        // Hey - there's link
        PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START].flags |= PSEC_PORT_STATE_FLAGS_LINK_UP;
    } else {
        PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START].flags &= ~PSEC_PORT_STATE_FLAGS_LINK_UP;
        // Link went down. Remove all registered MAC addresses on the port.
        PSEC_mac_del_all(isid, port, PSEC_DEL_REASON_PORT_LINK_DOWN);
    }

    // This may have given rise to enabling secure learning on that port.
    PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);

    PSEC_CRIT_EXIT();
}

/******************************************************************************/
// PSEC_master_user_isid_port_check()
// Returns VTSS_RC_OK if we're master and isid, port, and user are legal.
/******************************************************************************/
static vtss_rc PSEC_master_user_isid_port_check(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port)
{
    if (!msg_switch_is_master()) {
        return PSEC_ERROR_MUST_BE_MASTER;
    }

    if (user < (psec_users_t)0 || user >= PSEC_USER_CNT) {
        return PSEC_ERROR_INV_USER;
    }

    if (!VTSS_ISID_LEGAL(isid)) {
        return PSEC_ERROR_INV_ISID;
    }

    // In case someone changes VTSS_PORT_NO_START back to 1, we better survive that
    // so tell lint to not report this error:
    /*lint -e{685, 568} */
    if (port < VTSS_PORT_NO_START || port >= VTSS_PORTS + VTSS_PORT_NO_START) {
        // Note that we do allow stack ports and port numbers higher than the isid's
        // because the API allows for configuring this module before a given
        // switch exists in the stack, and therefore we don't know the port count
        // of the switch or the stack ports of the switch at configuration time.
        return PSEC_ERROR_INV_PORT;
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// PSEC_do_age_or_hold()
/******************************************************************************/
static PSEC_INLINE void PSEC_do_age_or_hold(vtss_isid_t isid, vtss_port_no_t port)
{
    psec_port_state_t *port_state        = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    psec_mac_state_t  *mac_state         = port_state->macs;
    psec_mac_state_t  *mac_state_next;
    BOOL              found_at_least_one = FALSE;

    while (mac_state) {
        mac_state_next = mac_state->next; // Keep a pointer to the next, because it may be that we delete the current below.

        if (mac_state->age_or_hold_time_secs && !(mac_state->flags & PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED)) {
            found_at_least_one = TRUE;
            // Subject to aging/holding
            if (--mac_state->age_or_hold_time_secs == 0) {
                // Aging or holding timed out.
                if (mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED ||
                    mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED ||
                    mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED) {
                    // It was due to hold time. Remove the entry
                    PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_HOLD_TIME_EXPIRED);
                } else {
                    // Aging timed out.
                    if (mac_state->flags & PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN) {
                        // An frame was seen during this aging period.
                        // Keep entry, but re-enable CPU-copying and update the age time.
                        // The following call will only update the age_or_hold_time_secs (due to the TRUE parameter)
                        // If we had called the function with FALSE instead, then the function would have cleared
                        // the CPU_COPYING flag, set the AGE_FRAME_SEEN flag and called the PSEC_mac_module_chg()
                        // function itself. We want the opposite to happen.
                        (void)PSEC_mac_chg(isid, port, port_state, mac_state, TRUE);
                        // So we need to restart aging ourselves.
                        mac_state->flags |= PSEC_MAC_STATE_FLAGS_CPU_COPYING;
                        mac_state->flags &= ~PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN;
                        (void)PSEC_mac_module_chg(isid, port, mac_state, __LINE__);
                    } else {
                        // Aging timed out, but the station has not sent new frames in the aging period.
                        // Unregister it.
                        PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_AGED_OUT);
                    }
                }
            }
        }
        mac_state = mac_state_next;
    }

    if (!found_at_least_one) {
        // If this function is called, at least one must be subject to aging/holding
        T_E("Internal error");
    }
}

/******************************************************************************/
// PSEC_thread()
// The only purpose of this thread is to age learned entries (both aging and
// holding).
/******************************************************************************/
static void PSEC_thread(cyg_addrword_t data)
{
    vtss_isid_t isid;
    u64         enabled_ports;

    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {

                // We should timeout every one second (1000 ms)
                VTSS_OS_MSLEEP(1000);
                if (!msg_switch_is_master()) {
                    break;
                }

                PSEC_CRIT_ENTER();
                // Timeout. Go through all ports that have at least one MAC address that needs aging/holding.
                for (isid = 0; isid < VTSS_ISID_CNT; isid++) {
                    enabled_ports = PSEC_age_hold_enabled_ports[isid];
                    if (enabled_ports) {
                        // At least one port is subject to aging/holding on this switch.
                        u32 port, port_cnt = port_isid_port_count(isid + VTSS_ISID_START);
                        u64 port_mask = 1ULL;
                        for (port = 0; port_cnt != 0 && port < port_cnt; port++) {
                            if (enabled_ports & port_mask) {
                                PSEC_do_age_or_hold(isid + VTSS_ISID_START, port + VTSS_PORT_NO_START);
                            }
                            port_mask <<= 1ULL;
                        }
                    }
                }
                PSEC_CRIT_EXIT();
            } // while (msg_switch_is_master())
        } // if (msg_switch_is_master())

        // No longer master. Time to bail out.
        // No reason for using CPU ressources when we're a slave
        T_D("Suspending PSEC thread");
        cyg_thread_suspend(PSEC_thread_handle);
        T_D("Resumed PSEC thread");
    } // while (1)
}

/****************************************************************************/
/*                                                                          */
/*  SEMI-PUBLIC FUNCTIONS                                                   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// psec_mgmt_port_state_get()
/****************************************************************************/
vtss_rc psec_mgmt_port_state_get(vtss_isid_t isid, vtss_port_no_t port, psec_port_state_t *state, BOOL get_macs)
{
    vtss_rc rc;
    if ((rc = PSEC_master_user_isid_port_check((psec_users_t)0 /* doesn't matter */, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    if (!state) {
        return PSEC_ERROR_INV_PARAM;
    }

    PSEC_CRIT_ENTER();
    *state = PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    if (get_macs) {
        // Caller wants the individual MAC states. We must VTSS_MALLOC() and copy our current states
        // because otherwise the caller would have to obtain the psec mutex while traversing the list
        // of MACs, and in case the list is very long, at least the Web handler would risk ending up
        // in a deadlock situation because new MAC addresses were coming in causing the rx thread to be blocked,
        // and subsequently causing the Web thread to not getting acks to it's TCP frames because
        // cyg_httpd_write_chunked() may block.

        // Traverse the list once to get a number of MACs held in it (port->mac_cnt doesn't work here, because
        // it doesn't necessarily encompass all MACs - see definition).
        u32 mac_cnt = 0;
        psec_mac_state_t *macs = state->macs;
        psec_mac_state_t *new_macs = NULL;
        if (macs) {
            while (macs) {
                mac_cnt++;
                macs = macs->next;
            }
        }

        if (mac_cnt) {
            psec_mac_state_t *new_mac_iter;
            new_macs = VTSS_MALLOC(mac_cnt * sizeof(psec_mac_state_t));
            if (new_macs) {
                macs = state->macs;
                new_mac_iter = new_macs;
                while (macs) {
                    *new_mac_iter = *macs;
                    new_mac_iter->prev = NULL; // Caller can't use prev pointer
                    if (macs->next) {
                        new_mac_iter->next = new_mac_iter + 1;
                    }
                    new_mac_iter = new_mac_iter->next;
                    macs = macs->next;
                }
            } else {
                T_E("Unable to allocate memory for MACs");
            }
        }

        state->macs = new_macs;
    } else {
        // Caller doesn't want the individual MAC states. NULLify it to avoid the caller using it.
        state->macs = NULL;
    }

    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_dbg_shaper_cfg_set()
/****************************************************************************/
vtss_rc psec_dbg_shaper_cfg_set(psec_rate_limit_cfg_t *cfg)
{
    if (!cfg) {
        return PSEC_ERROR_INV_PARAM;
    }

    if (cfg->fill_level_min >= cfg->fill_level_max) {
        return PSEC_ERROR_INV_SHAPER_FILL_LEVEL;
    }

    if (cfg->rate == 0) {
        return PSEC_ERROR_INV_SHAPER_RATE;
    }

    if (msg_switch_is_master()) {
        vtss_isid_t isid;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            PSEC_msg_tx_rate_limit_cfg(isid, cfg);
        }
    } else {
        PSEC_CRIT_ENTER();
        psec_rate_limit_cfg_set(cfg);
        PSEC_CRIT_EXIT();
    }
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_dbg_shaper_cfg_get()
/****************************************************************************/
vtss_rc psec_dbg_shaper_cfg_get(psec_rate_limit_cfg_t *cfg)
{
    if (!cfg) {
        return PSEC_ERROR_INV_PARAM;
    }

    PSEC_CRIT_ENTER();
    psec_rate_limit_cfg_get(cfg);
    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_dbg_shaper_stat_get()
/****************************************************************************/
vtss_rc psec_dbg_shaper_stat_get(psec_rate_limit_stat_t *stat, u64 *cur_fill_level)
{
    if (!stat || !cur_fill_level) {
        return PSEC_ERROR_INV_PARAM;
    }

    PSEC_CRIT_ENTER();
    psec_rate_limit_stat_get(stat, cur_fill_level);
    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_dbg_shaper_stat_clr()
/****************************************************************************/
vtss_rc psec_dbg_shaper_stat_clr(void)
{
    PSEC_CRIT_ENTER();
    psec_rate_limit_stat_clr();
    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_user_name()
/****************************************************************************/
char *psec_user_name(psec_users_t user)
{
    switch (user) {
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    case PSEC_USER_PSEC_LIMIT:
        return "Limit Control";
#endif

#ifdef VTSS_SW_OPTION_DOT1X
    case PSEC_USER_DOT1X:
        return "802.1X";
#endif

#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
    case PSEC_USER_DHCP_SNOOPING:
        return "DHCP Snooping";
#endif

#ifdef VTSS_SW_OPTION_VOICE_VLAN
    case PSEC_USER_VOICE_VLAN:
        return "Voice VLAN";
#endif

    default:
        return "Unknown";
    }
}

/****************************************************************************/
// psec_user_abbr()
/****************************************************************************/
char psec_user_abbr(psec_users_t user)
{
    switch (user) {
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    case PSEC_USER_PSEC_LIMIT:
        return 'L';
#endif

#ifdef VTSS_SW_OPTION_DOT1X
    case PSEC_USER_DOT1X:
        return '8';
#endif

#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
    case PSEC_USER_DHCP_SNOOPING:
        return 'D';
#endif

#ifdef VTSS_SW_OPTION_VOICE_VLAN
    case PSEC_USER_VOICE_VLAN:
        return 'V';
#endif

    default:
        return 'U'; // Unknown
    }
}

/****************************************************************************/
/*                                                                          */
/*  PUBLIC FUNCTIONS                                                        */
/*                                                                          */
/****************************************************************************/

/******************************************************************************/
// psec_mgmt_time_cfg_set()
/******************************************************************************/
vtss_rc psec_mgmt_time_cfg_set(psec_users_t user, u32 aging_period_secs, u32 hold_time_secs)
{
    vtss_isid_t isid;

    if (user < (psec_users_t)0 || user >= PSEC_USER_CNT) {
        return PSEC_ERROR_INV_USER;
    }

    if (aging_period_secs != 0 && (aging_period_secs < PSEC_AGE_TIME_MIN || aging_period_secs > PSEC_AGE_TIME_MAX)) {
        return PSEC_ERROR_INV_AGING_PERIOD;
    }

    if (hold_time_secs < PSEC_HOLD_TIME_MIN || hold_time_secs > PSEC_HOLD_TIME_MAX) {
        return PSEC_ERROR_INV_HOLD_TIME;
    }

    PSEC_CRIT_ENTER();

    if (PSEC_stack_state.aging_period_secs[user] == aging_period_secs &&
        PSEC_stack_state.hold_time_secs[user]    == hold_time_secs) {
        // No change
        goto do_exit;
    }

    PSEC_stack_state.aging_period_secs[user] = aging_period_secs;
    PSEC_stack_state.hold_time_secs[user]    = hold_time_secs;

    // Now check if this affects any of the already registered MAC addresses.
    for (isid = 0; isid < VTSS_ISID_CNT; isid++) {
        psec_switch_state_t *switch_state = &PSEC_stack_state.switch_state[isid];
        port_iter_t pit;
        if (!switch_state->switch_exists) {
            continue;
        }
        (void)port_iter_init(&pit, NULL, isid + VTSS_ISID_START, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            psec_port_state_t *port_state = &switch_state->port_state[pit.iport - VTSS_PORT_NO_START];
            psec_mac_state_t  *mac_state  = port_state->macs;

            if (!PSEC_USER_ENA_GET(port_state, user)) {
                continue;
            }

            mac_state = port_state->macs;
            while (mac_state) {
                // The PSEC_mac_chg() function will update the age and hold times of
                // the running counter.
                // The TRUE indicates that we don't want to change the forward
                // decision, but only the hold or age time based on the currently
                // enabled users and their forwarding decision.
                if (!PSEC_mac_chg(isid + VTSS_ISID_START, pit.iport, port_state, mac_state, TRUE)) {
                    T_E("Internal error");
                    goto do_exit;
                }
                mac_state = mac_state->next;
            }
        }
    }

do_exit:
    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// psec_mgmt_port_cfg_set()
/******************************************************************************/
vtss_rc psec_mgmt_port_cfg_set(psec_users_t                        user,
                               void                                *user_ctx,
                               vtss_isid_t                         isid,
                               vtss_port_no_t                      port,
                               BOOL                                enable,
                               BOOL                                reopen_port,
                               psec_on_mac_loop_through_callback_f *loop_through_callback,
                               psec_port_mode_t                    port_mode)
{
    vtss_rc           rc;
    psec_port_state_t *port_state;
    psec_mac_state_t  *mac_state;
    BOOL              first_user;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_CRIT_ENTER();

    port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];

    first_user = port_state->ena_mask == 0;
    PSEC_USER_ENA_SET(port_state, user, enable);

    if (enable) {
        mac_state = port_state->macs;
        while (mac_state) {
            psec_add_method_t          add_method      = PSEC_ADD_METHOD_FORWARD;
            BOOL                       keep            = TRUE;
            psec_loop_through_action_t action          = PSEC_LOOP_THROUGH_ACTION_NONE;
            psec_mac_state_t           *temp_mac_state = mac_state->next; // Keep a pointer to the next before possible deleting the current.

            if (loop_through_callback && (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE)) {
                // Skip those entries not in the MAC table (due to H/W add failure), since these entries
                // are not known by anyone. And don't call the loop-through callback if the user
                // hasn't specified it. In that case, simply add it as allowed as seen from that
                // user module's perspective.
                add_method = loop_through_callback(user_ctx, isid, port, &mac_state->vid_mac, port_state->mac_cnt, &keep, &action);
            }

            switch (action) {
            case PSEC_LOOP_THROUGH_ACTION_NONE:
                break;

            case PSEC_LOOP_THROUGH_ACTION_CLEAR_SHUT_DOWN:
                port_state->flags &= ~PSEC_PORT_STATE_FLAGS_SHUT_DOWN;
            // Fall through!

            case PSEC_LOOP_THROUGH_ACTION_CLEAR_LIMIT_REACHED:
                port_state->flags &= ~PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
                break;

            case PSEC_LOOP_THROUGH_ACTION_SET_LIMIT_REACHED:
                port_state->flags |= PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
                break;
            }

            if (keep) {
                // Change the MAC address' forward decision for this user.
                PSEC_FORWARD_DECISION_SET(mac_state, user, add_method);

                // This may give rise to another forwarding decision for this MAC address.
                // The FALSE indicates that we also want to change the forwarding decision,
                // and not only the age and hold time.
                (void)PSEC_mac_chg(isid, port, port_state, mac_state, TRUE);
            } else {
                // Temporarily disable this user from the enabled state, so that he doesn't get
                // called back (if he's already enabled) when unregistering this MAC address.
                // Avoid Lint warning "Constant value Boolean" for the last arg to PSEC_USER_ENA_SET()
                /*lint --e{506} */
                PSEC_USER_ENA_SET(port_state, user, FALSE);
                PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_USER_DELETED);
                PSEC_USER_ENA_SET(port_state, user, TRUE);
            }
            mac_state = temp_mac_state;
        }

        // Set the user's preferred Secure Learning CPU copy method.
        PSEC_PORT_MODE_SET(port_state, user, port_mode);

        if (first_user) {
            // This is the first user to enable on this port. Send port-configuration.
            // Even when PSEC_FIX_GNATS_6935 is defined, we need to copy frames to the
            // master for the sake of aging.
            PSEC_msg_tx_port_cfg(isid, port, TRUE);
        }

        if (reopen_port) {
            // Should only be set by PSEC LIMIT module. If the limit was reached or the
            // port was shut down, then it should be re-opened when the PSEC LIMIT disables.
            // This could have been done by checking whether user == PSEC_USER_PSEC_LIMIT,
            // but in order to support future security modules, we have it in the API function,
            // so that we only need to change this file in case more than one module can set this
            // parameter.
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_SHUT_DOWN;
        }

        // Now, with the new user and the taken actions, another learn/CPU copy
        // state may have to be applied to the port.
        PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);
    } else {
        // The user wants to back out.
        // Call his loop-through callback function so that he can clean up his state,
        // if he wants to, but don't use the return values or \@keep or \@action for
        // anything.
        if (loop_through_callback) {
            mac_state = port_state->macs;
            while (mac_state) {
                BOOL                       keep   = TRUE;
                psec_loop_through_action_t action = PSEC_LOOP_THROUGH_ACTION_NONE;
                if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
                    (void)loop_through_callback(user_ctx, isid, port, &mac_state->vid_mac, port_state->mac_cnt, &keep, &action);
                }
                mac_state = mac_state->next;
            }
        }

        if (reopen_port) {
            // Should only be set by PSEC LIMIT module. If the limit was reached or the
            // port was shut down, then it should be re-opened when the PSEC LIMIT disables.
            // This could have been done by checking whether user == PSEC_USER_PSEC_LIMIT,
            // but in order to support future security modules, we have it in the API function,
            // so that we only need to change this file in case more than one module can set this
            // parameter.
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_LIMIT_REACHED;
            port_state->flags &= ~PSEC_PORT_STATE_FLAGS_SHUT_DOWN;

            if (port_state->ena_mask != 0) {
                // Doing this may cause the port to be re-enabled if there were other users.
                PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);
            }
        }

        if (port_state->ena_mask != 0) {
            // If there are still enabled users on this port, disabling one user may cause
            // blocked MAC addresses to become unblocked.
            // Loop through all entries attached to this port and check if they still need
            // to be blocked or can be unblocked.
            mac_state = port_state->macs;
            while (mac_state) {
                // The FALSE indicates that we also want to change the forwarding decision,
                // and not only the age and hold time.
                (void)PSEC_mac_chg(isid, port, port_state, mac_state, FALSE);
                mac_state = mac_state->next;
            }
        } else {

            // This is the last user enabled on this port. Send port-configuration.
            PSEC_msg_tx_port_cfg(isid, port, FALSE);

            // There are no more enabled users on this port. Remove all MAC addresses learned
            PSEC_mac_del_all(isid, port, PSEC_DEL_REASON_NO_MORE_USERS);

            // And disable secure learning.
            PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);
        }
    }

    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// psec_mgmt_mac_chg()
/******************************************************************************/
vtss_rc psec_mgmt_mac_chg(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_add_method_t new_method)
{
    vtss_rc           rc;
    vtss_isid_t       looked_up_isid;
    vtss_port_no_t    looked_up_port;
    psec_mac_state_t  *mac_state;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_ensure_zero_padding(vid_mac);

    PSEC_CRIT_ENTER();

    if (((mac_state = PSEC_lookup(&looked_up_isid, &looked_up_port, vid_mac)) == NULL) ||
        (isid != looked_up_isid)                                                       ||
        (port != looked_up_port)) {
        rc = PSEC_ERROR_MAC_VID_NOT_FOUND;
        goto do_exit;
    }

    // In reality we should also check if this user is enabled on this port, but
    // since it doesn't change anything in the forward decision if he isn't, we
    // don't care.

    // Set the user's new forward decision
    PSEC_FORWARD_DECISION_SET(mac_state, user, new_method);

    // This may give rise to another forwarding decision for this MAC address.
    // The FALSE indicates that we also want to change the forwarding decision,
    // and not only the age and hold time.
    (void)PSEC_mac_chg(isid, port, &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START], mac_state, FALSE);

    // No need to re-investigate whether the secure learning/CPU copy should be altered.

    rc = VTSS_RC_OK;

do_exit:
    PSEC_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// psec_mgmt_mac_add()
/******************************************************************************/
vtss_rc psec_mgmt_mac_add(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_add_method_t method)
{
    vtss_rc           rc;
    vtss_isid_t       looked_up_isid;
    vtss_port_no_t    looked_up_port;
    psec_mac_state_t  *mac_state;
    psec_port_state_t *port_state;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_ensure_zero_padding(vid_mac);

    PSEC_CRIT_ENTER();

    mac_state = PSEC_lookup(&looked_up_isid, &looked_up_port, vid_mac);
    if (mac_state != NULL) {
        T_W("%d:%d: <MAC, VID>=<%s, %d> already found on %d:%d", isid, iport2uport(port), misc_mac2str(vid_mac->mac.addr), vid_mac->vid, looked_up_isid, iport2uport(looked_up_port));
        rc = PSEC_ERROR_MAC_VID_ALREADY_FOUND;
        goto do_exit;
    }

    port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    // Only users that have proclaimed that the port should remain in non-CPU-copy mode are allowed to call this function.
    if (PSEC_PORT_MODE_GET(port_state, user) != PSEC_PORT_MODE_KEEP_BLOCKED || !PSEC_USER_ENA_GET(port_state, user)) {
        T_E("%d:%d: Called by user (%s) that hasn't proclaimed correct port mode", isid, iport2uport(port), psec_user_name(user));
        rc = PSEC_ERROR_INV_USER_MODE;
        goto do_exit;
    }

    rc = PSEC_do_add_mac(user, isid, port, vid_mac, method);

do_exit:
    PSEC_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// psec_mgmt_mac_del()
/******************************************************************************/
vtss_rc psec_mgmt_mac_del(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac)
{
    vtss_rc           rc;
    vtss_isid_t       looked_up_isid;
    vtss_port_no_t    looked_up_port;
    psec_mac_state_t  *mac_state;
    psec_port_state_t *port_state;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_ensure_zero_padding(vid_mac);

    PSEC_CRIT_ENTER();

    if (((mac_state = PSEC_lookup(&looked_up_isid, &looked_up_port, vid_mac)) == NULL) ||
        (isid != looked_up_isid)                                                       ||
        (port != looked_up_port)) {
        rc = PSEC_ERROR_MAC_VID_NOT_FOUND;
        goto do_exit;
    }

    port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];
    // Only users that have proclaimed that the port should remain in non-CPU-copy mode are allowed to call this function.
    if (PSEC_PORT_MODE_GET(port_state, user) != PSEC_PORT_MODE_KEEP_BLOCKED || !PSEC_USER_ENA_GET(port_state, user)) {
        T_E("%d:%d: Called by user (%s) that hasn't proclaimed correct port mode", isid, iport2uport(port), psec_user_name(user));
        rc = PSEC_ERROR_INV_USER_MODE;
        goto do_exit;
    }

    // Temporarily disable this user from the enabled state, so that he doesn't get
    // called back (if he's already enabled) when unregistering this MAC address.
    // Avoid Lint warning "Constant value Boolean" for the last arg to PSEC_USER_ENA_SET()
    /*lint --e{506} */
    PSEC_USER_ENA_SET(port_state, user, FALSE);
    PSEC_mac_del(isid, port, mac_state, PSEC_DEL_REASON_USER_DELETED);
    PSEC_USER_ENA_SET(port_state, user, TRUE);

    rc = VTSS_RC_OK;

do_exit:
    PSEC_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// psec_mgmt_register_callbacks()
/******************************************************************************/
vtss_rc psec_mgmt_register_callbacks(psec_users_t user, psec_on_mac_add_callback_f *on_mac_add_callback_func, psec_on_mac_del_callback_f *on_mac_del_callback_func)
{
    if (user < (psec_users_t)0 || user >= PSEC_USER_CNT) {
        return PSEC_ERROR_INV_USER;
    }

    // Allow this function on any switch (master or slave), and allow both
    // NULL and non-NULL callbacks.
    PSEC_CRIT_ENTER();
    PSEC_stack_state.on_mac_add_callbacks[user] = on_mac_add_callback_func;
    PSEC_stack_state.on_mac_del_callbacks[user] = on_mac_del_callback_func;
    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// psec_mgmt_reopen_port()
/******************************************************************************/
vtss_rc psec_mgmt_reopen_port(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port)
{
    vtss_rc rc;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_CRIT_ENTER();

    PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START].flags &= ~PSEC_PORT_STATE_FLAGS_SHUT_DOWN;

    // This may have given rise to reopening the port for CPU-copy traffic.
    PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);

    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_mgmt_switch_status_get()
/****************************************************************************/
vtss_rc psec_mgmt_switch_status_get(vtss_isid_t isid, psec_switch_status_t *switch_status)
{
    vtss_rc             rc;
    vtss_port_no_t      port;
    psec_switch_state_t *switch_state;
    psec_port_state_t   *port_state;

    if ((rc = PSEC_master_user_isid_port_check((psec_users_t)0, isid, VTSS_PORT_NO_START)) != VTSS_RC_OK) {
        return rc;
    }

    if (!switch_status) {
        return PSEC_ERROR_INV_PARAM;
    }

    PSEC_CRIT_ENTER();

    switch_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START];

    for (port = 0; port < port_isid_port_count(isid); port++) {
        port_state = &switch_state->port_state[port];
        switch_status->port_status[port].limit_reached = (port_state->flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) != 0;
        switch_status->port_status[port].shutdown      = (port_state->flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN)     != 0;
    }

    PSEC_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// psec_mgmt_force_cpu_copy()
/****************************************************************************/
vtss_rc psec_mgmt_force_cpu_copy(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, BOOL enable)
{
#ifdef PSEC_FIX_GNATS_6935
    vtss_rc           rc;
    psec_port_state_t *port_state;

    if ((rc = PSEC_master_user_isid_port_check(user, isid, port)) != VTSS_RC_OK) {
        return rc;
    }

    PSEC_CRIT_ENTER();

    port_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START].port_state[port - VTSS_PORT_NO_START];

    // Set the user's force-CPU-copying bit.
    // Speed it up by testing against the current settings.
    if (PSEC_FORCE_CPU_COPY_GET(port_state, user) != enable) {
        PSEC_FORCE_CPU_COPY_SET(port_state, user, enable);
        PSEC_sec_learn_cpu_copy_check(isid, port, PSEC_LEARN_CPU_REASON_OTHER, __LINE__);
    }

    PSEC_CRIT_EXIT();
#endif
    return VTSS_RC_OK;
}
/****************************************************************************/
// psec_error_txt()
/****************************************************************************/
char *psec_error_txt(vtss_rc rc)
{
    switch (rc) {
    case PSEC_ERROR_INV_USER:
        return "Invalid user";

    case PSEC_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case PSEC_ERROR_INV_ISID:
        return "Invalid Switch ID";

    case PSEC_ERROR_INV_PORT:
        return "Invalid port number";

    case PSEC_ERROR_INV_AGING_PERIOD:
        // Not nice to use specific values in this string, but
        // much easier than constructing a constant string dynamically.
        return "Aging period is out of bounds (0 or [" vtss_xstr(PSEC_AGE_TIME_MIN) "; " vtss_xstr(PSEC_AGE_TIME_MAX) "])";

    case PSEC_ERROR_INV_HOLD_TIME:
        // Not nice to use specific values in this string, but
        // much easier than constructing a constant string dynamically.
        return "Hold time is out of bounds ([" vtss_xstr(PSEC_HOLD_TIME_MIN) "; " vtss_xstr(PSEC_HOLD_TIME_MAX) "])";

    case PSEC_ERROR_MAC_VID_NOT_FOUND:
        return "The <MAC, VID> was not found on the specified <switch, port>";

    case PSEC_ERROR_MAC_VID_ALREADY_FOUND:
        return "The <MAC, VID> was already found on a port";

    case PSEC_ERROR_INV_USER_MODE:
        return "The PSEC user is not allowed to call this function";

    case PSEC_ERROR_SWITCH_IS_DOWN:
        return "The selected switch doesn't exist";

    case PSEC_ERROR_LINK_IS_DOWN:
        return "The selected port's link is down";

    case PSEC_ERROR_OUT_OF_MAC_STATES:
        return "Out of state machines";

    case PSEC_ERROR_PORT_IS_SHUT_DOWN:
        return "The port has been shut down";

    case PSEC_ERROR_LIMIT_IS_REACHED:
        return "The limit is reached (no more MAC addresses can be added to this port)";

    case PSEC_ERROR_NO_USERS_ENABLED:
        return "No users are enabled on the port";

    case PSEC_ERROR_STATE_CHG_DURING_CALLBACK:
        return "Switch, port, or MAC address state change during callback";

    case PSEC_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    case PSEC_ERROR_INV_SHAPER_FILL_LEVEL:
        return "Maximum fill-level must be greater than the minimum";

    case PSEC_ERROR_INV_SHAPER_RATE:
        return "The shaper rate must be greater than 0";

    case PSEC_ERROR_INTERNAL_ERROR:
        return "An internal error occurred";

    default:
        return "PSEC: Unknown error code";
    }
}

/******************************************************************************/
// psec_del_reason_to_str()
/******************************************************************************/
char *psec_del_reason_to_str(psec_del_reason_t reason)
{
    switch (reason) {
    case PSEC_DEL_REASON_HW_ADD_FAILED:
        return "MAC Table add failed (H/W)";
    case PSEC_DEL_REASON_SW_ADD_FAILED:
        return "MAC Table add failed (S/W)";
    case PSEC_DEL_REASON_SWITCH_DOWN:
        return "The switch went down";
    case PSEC_DEL_REASON_PORT_LINK_DOWN:
        return "The port link went down";
    case PSEC_DEL_REASON_STATION_MOVED:
        return "The MAC was suddenly seen on another port";
    case PSEC_DEL_REASON_AGED_OUT:
        return "The entry aged out";
    case PSEC_DEL_REASON_HOLD_TIME_EXPIRED:
        return "The hold time expired";
    case PSEC_DEL_REASON_USER_DELETED:
        return "The entry was deleted by another module";
    case PSEC_DEL_REASON_PORT_SHUT_DOWN:
        return "Shut down by Limit Control module";
    case PSEC_DEL_REASON_NO_MORE_USERS:
        return "No more users";
    default:
        return "Unknown";
    }
}

/******************************************************************************/
// psec_add_method_to_str()
/******************************************************************************/
char *psec_add_method_to_str(psec_add_method_t add_method)
{
    switch (add_method) {
    case PSEC_ADD_METHOD_FORWARD:
        return "Forward";
    case PSEC_ADD_METHOD_BLOCK:
        return "Block with timeout";
    case PSEC_ADD_METHOD_KEEP_BLOCKED:
        return "Keep blocked";
    default:
        return "Unknown";
    }
}

/******************************************************************************/
// psec_init()
// Initialize Port Security Module
/******************************************************************************/
vtss_rc psec_init(vtss_init_data_t *data)
{
    vtss_isid_t    isid = data->isid;
    vtss_port_no_t port;
    int            i;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Initialize and register trace resources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

#ifdef VTSS_SW_OPTION_VCLI
        // Initialize our CLI stuff.
        psec_cli_init();
#endif

        // Initialize message buffer(s)
        // We need one per port to avoid stalling user modules. The buffers aren't that big (32 bytes each at the time of writing).
        PSEC_msg_buf_pool = msg_buf_pool_create(VTSS_MODULE_ID_PSEC, "Request", VTSS_PORTS, sizeof(psec_msg_t));

        // Clear our state
        memset(&PSEC_stack_state, 0, sizeof(PSEC_stack_state));
        memset(PSEC_mac_states,   0, sizeof(PSEC_mac_states)); // Clears @unique

        // Initialize free MAC addresses.
        // Loop over all but the first and the last
        for (i = 1; i < PSEC_MAC_ADDR_ENTRY_CNT - 1; i++) {
            PSEC_mac_states[i].prev = &PSEC_mac_states[i - 1];
            PSEC_mac_states[i].next = &PSEC_mac_states[i + 1];
        }

        // Compile and run-time checks
#if PSEC_MAC_ADDR_ENTRY_CNT <= 0
#error "Invalid PSEC_MAC_ADDR_ENTRY_CNT"
#endif

        // Avoid Lint warning "Constant value Boolean". This is intended to be a compile time check
        /*lint -e{506} */
        if (PSEC_USER_CNT > (psec_users_t)32) {
            T_E("This module supports at most 32 users due to the ena_mask");
        }

        // Avoid Lint warning "Constant value Boolean". This is intended to be a compile time check
        /*lint -e{506} */
        if (PSEC_ADD_METHOD_CNT > (psec_add_method_t)3) {
            T_E("This module expects at most two bits for the add_method_t");
        }

        PSEC_mac_states[0].prev = NULL;
#if PSEC_MAC_ADDR_ENTRY_CNT > 1
        PSEC_mac_states[0].next = &PSEC_mac_states[1];
#else
        PSEC_mac_states[0].next = NULL;
#endif

#if PSEC_MAC_ADDR_ENTRY_CNT > 1
        PSEC_mac_states[PSEC_MAC_ADDR_ENTRY_CNT - 1].prev = &PSEC_mac_states[PSEC_MAC_ADDR_ENTRY_CNT - 2];
        PSEC_mac_states[PSEC_MAC_ADDR_ENTRY_CNT - 1].next = NULL;
#endif

        PSEC_mac_state_free_pool = &PSEC_mac_states[0];
        PSEC_mac_states_left = PSEC_MAC_ADDR_ENTRY_CNT;

        memset(PSEC_age_hold_enabled_ports, 0, sizeof(PSEC_age_hold_enabled_ports));

        VTSS_PORT_BF_CLR(PSEC_copy_to_master);

        // Initialize the thread
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          PSEC_thread,
                          0,
                          "Port Security",
                          PSEC_thread_stack,
                          sizeof(PSEC_thread_stack),
                          &PSEC_thread_handle,
                          &PSEC_thread_state);

        // Initialize sempahore.
        critd_init(&crit_psec, "crit_psec", VTSS_MODULE_ID_PSEC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        // When created, the semaphore was initially locked.
        PSEC_CRIT_EXIT();

        break;

    case INIT_CMD_START:
        PSEC_msg_rx_init();
        PSEC_frame_rx_init();
        // Register for port link-state change events
        (void)port_global_change_register(VTSS_MODULE_ID_PSEC, PSEC_link_state_change_callback);
        break;

    case INIT_CMD_CONF_DEF:
        // We don't have any configuration.
        break;

    case INIT_CMD_MASTER_UP: {
        // Wake up the thread, so that aging/holding can occur
        T_D("Resuming PSEC thread");
        cyg_thread_resume(PSEC_thread_handle);
        break;
    }

    case INIT_CMD_MASTER_DOWN:
        break;

    case INIT_CMD_SWITCH_ADD: {
        psec_rate_limit_cfg_t rate_limit_cfg;

        // Send the whole switch configuration to the new switch.
        // It is assumed that this module is already told the whole story about
        // all user modules' enabledness. If not, it will result in many, many
        // small messages sent in the psec_mgmt_port_cfg_set() function.
        PSEC_CRIT_ENTER();
        PSEC_stack_state.switch_state[isid - VTSS_ISID_START].switch_exists = TRUE;
        PSEC_msg_tx_switch_cfg(isid);

        // About race condition concerns: One could be concerned that this module won't get the
        // switch add event until the port module has sent link-up events for the new switch.
        // This may occur if this module comes after the port module in the array of modules.
        // The good thing is that we (in PSEC_link_state_change_callback()) react and cache the
        // link state even before this piece of code is called. This means that the link-state
        // may already be updated when we get here, so we don't have to ask the port module for
        // its link-state here.

        // Check if this event gave rise to changing the secure learning on one or more ports on the new switch.
        PSEC_sec_learn_cpu_copy_check(isid, VTSS_PORTS /* Doesn't matter */, PSEC_LEARN_CPU_REASON_SWITCH_UP_OR_DOWN, __LINE__);

        PSEC_CRIT_EXIT();

        // Also send the configured rate-limit to the new switch (outside the crit sect).
        psec_rate_limit_cfg_get(&rate_limit_cfg);
        PSEC_msg_tx_rate_limit_cfg(isid, &rate_limit_cfg);
        break;
    }

    case INIT_CMD_SWITCH_DEL: {
        psec_switch_state_t *switch_state = &PSEC_stack_state.switch_state[isid - VTSS_ISID_START];
        PSEC_CRIT_ENTER();
        switch_state->switch_exists = FALSE;
        // We cannot send the switch configuration, since the switch doesn't exist anymore, but
        // we can delete all attached MAC addresses.

        // About race condition concerns: Whether the port module or this module receives the
        // switch delete event first doesn't matter. If the port module receives it first, then
        // the PSEC_link_state_change_callback() function will be called. That function will
        // first check if the switch exists, and since the message module doesn't think it exists,
        // the callback function will simply return. Later on, this piece of code will be called
        // and set the switch in question's link states to 'down'.

        // Don't use port_iter_t here, because the switch may be deleted due to an upcoming
        // master-down event in which case the port iterator will fail, and we won't get
        // our state set correctly.
        for (port = 0; port < port_isid_port_count(isid); port++) {
            if (port_isid_port_no_is_stack(isid, port + VTSS_PORT_NO_START) == FALSE) {
                switch_state->port_state[port].flags &= ~PSEC_PORT_STATE_FLAGS_LINK_UP;
                PSEC_mac_del_all(isid, port + VTSS_PORT_NO_START, PSEC_DEL_REASON_SWITCH_DOWN);
            }
        }

        // Check if the switch-delete event gives rise to changing the secure learning mode on the deleted switch.
        // In fact, we do this to keep our own state up to date. The MAC module may give a warning if trying to
        // do it on a switch that doesn't exist. We ignore that warning.
        PSEC_sec_learn_cpu_copy_check(isid, VTSS_PORTS /* Doesn't matter */, PSEC_LEARN_CPU_REASON_SWITCH_UP_OR_DOWN, __LINE__);
        PSEC_CRIT_EXIT();
        break;
    }

    default:
        break;
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
