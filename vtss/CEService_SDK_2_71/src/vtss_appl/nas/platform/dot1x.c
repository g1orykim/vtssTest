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

// All Base-lib functions and base-lib callout functions are thread safe, as they are called
// with DOT1X_CRIT() taken, but Lint cannot see that in its final wrap-up (thread walk)
/*lint -esym(459, DOT1X_psec_loop_through_callback) */
/*lint -esym(459, DOT1X_rx_bpdu)                    */

/****************************************************************************/
// Includes
/****************************************************************************/
#include <time.h>                  /* For time_t, time()           */
#include <misc_api.h>              /* For misc_time2str()          */
#include <network.h>               /* For socket functions         */
#include "dot1x.h"                 /* For semi-public functions    */
#include "main.h"                  /* For init_cmd_t def           */
#include "critd_api.h"             /* For semaphore wrapper        */
#include "dot1x_api.h"             /* For public structs           */
#include "packet_api.h"            /* For rx and tx of frames      */
#include "conf_api.h"              /* For MAC address              */
#include "port_api.h"              /* For port_no_is_stack(), etc. */
#include "msg_api.h"               /* For Tx/Rx of msgs            */

#include "vtss_nas_platform_api.h" /* For core NAS lib             */
#include "vtss_radius_api.h"       /* For RADIUS services          */
#include "vtss_common_os.h"        /* For vtss_os_get_portmac()    */
#include "vlan_api.h"              /* For VLAN awareness setting   */
#include "l2proto_api.h"           /* For L2PORT2PORT()            */
#ifdef NAS_USES_PSEC
#include "psec_api.h"              /* For Port Security services   */
#endif
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
#include "dot1x_acct.h"            /* For dot1x_acct_XXX()         */
#endif
#ifdef VTSS_SW_OPTION_VCLI
#include "dot1x_cli.h"
#endif
#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"              /* For Aggr port config         */
#endif
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"            /* For S_W() macro              */
#endif
#ifdef VTSS_SW_OPTION_MSTP
#include "mstp_api.h"              /* For MSTP port config         */
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
#include "mgmt_api.h"              /* For mgmt_prio2txt()          */
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM
#include "nas_qos_custom_api.h"    /* For NAS_QOS_CUSTOM_xxx       */
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"              /* For topo_isid2usid()        */
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "dot1x_icli_functions.h" // For dot1x_icfg_init
#endif
/****************************************************************************/
// Various defines
/****************************************************************************/
#define DOT1X_FLASH_CFG_VERSION 2

/****************************************************************************/
//
/****************************************************************************/
enum {
    DOT1X_PORT_STATE_FLAGS_LINK_UP    = (1 << 0), /**< '0' = link is down, '1' = Link is up                                               */
    DOT1X_PORT_STATE_FLAGS_AUTHORIZED = (1 << 1), /**< '0' = port auth-state set to unauthorized, '1' = port auth-state set to authorized */
}; // To satisfy Lint, we make this enum anonymous and whereever it's used, we declare an u8. I would've liked to call this "dot1x_port_state_flags_t".

typedef struct {
    u8 flags; /**< A combination of the DOT1X_PORT_STATE_FLAGS_xxx flags */
} dot1x_port_state_t;

typedef struct {
    BOOL switch_exists;
    dot1x_port_state_t port_state[VTSS_PORTS];
} dot1x_switch_state_t;

typedef struct {
    dot1x_switch_state_t switch_state[VTSS_ISID_CNT];
} dot1x_stack_state_t;

dot1x_stack_state_t DOT1X_stack_state;

/****************************************************************************/
// Overall configuration (valid on master only).
/****************************************************************************/
typedef struct {
    // One instance of the global configuration
    dot1x_glbl_cfg_t glbl_cfg;

    // One instance per switch in the stack of the switch configuration.
    // Indices are in the range [0; VTSS_ISID_CNT[, so all derefs must
    // subtract VTSS_ISID_START from @isid to index correctly.
    dot1x_switch_cfg_t switch_cfg[VTSS_ISID_CNT];
} dot1x_stack_cfg_t;

/****************************************************************************/
// Overall configuration as saved in flash.
/****************************************************************************/
typedef struct {
    // Current version of the configuration in flash.
    ulong version;

    // Overall config
    dot1x_stack_cfg_t cfg;
} dot1x_flash_cfg_t;

/****************************************************************************/
// Trace definitions
/****************************************************************************/
#include "dot1x_trace.h"
#include <vtss_trace_api.h>

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by dot1x_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "nas",
    .descr     = "NAS module"
};

#ifndef DOT1X_DEFAULT_TRACE_LVL
#define DOT1X_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = DOT1X_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_BASE] = {
        .name      = "base",
        .descr     = "Base-lib calls",
        .lvl       = DOT1X_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = DOT1X_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_ACCT] = {
        .name      = "acct",
        .descr     = "Accounting Module Calls",
        .lvl       = DOT1X_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_ICLI] = {
        .name      = "icli",
        .descr     = "iCLI",
        .lvl       = DOT1X_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/******************************************************************************/
// Semaphore stuff.
/******************************************************************************/
static critd_t DOT1X_crit;

// Macros for accessing semaphore functions
// -----------------------------------------
#if VTSS_TRACE_ENABLED
#define DOT1X_CRIT_ENTER()         critd_enter(        &DOT1X_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DOT1X_CRIT_EXIT()          critd_exit(         &DOT1X_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DOT1X_CRIT_ASSERT_LOCKED() critd_assert_locked(&DOT1X_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define DOT1X_CRIT_ENTER()         critd_enter(        &DOT1X_crit)
#define DOT1X_CRIT_EXIT()          critd_exit(         &DOT1X_crit)
#define DOT1X_CRIT_ASSERT_LOCKED() critd_assert_locked(&DOT1X_crit)
#endif

/******************************************************************************/
// Configuration and state
/******************************************************************************/
static dot1x_stack_cfg_t DOT1X_stack_cfg; // Configuration for whole stack (used when we're master, only).

// If not debugging, set DOT1X_INLINE to inline
#define DOT1X_INLINE inline

/******************************************************************************/
// Thread variables
/******************************************************************************/
static cyg_handle_t DOT1X_thread_handle;
static cyg_thread   DOT1X_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char         DOT1X_thread_stack[2 * (THREAD_DEFAULT_STACK_SIZE)];

// Value to use for supplicant timeout
#define SUPPLICANT_TIMEOUT 10

/******************************************************************************/
//
// Message handling functions, structures, and state.
//
/******************************************************************************/

/****************************************************************************/
// Message IDs
/****************************************************************************/
typedef enum {
    DOT1X_MSG_ID_MST_TO_SLV_PORT_STATE = 1, // Tell slave to set port state on a single port.
    DOT1X_MSG_ID_MST_TO_SLV_SWITCH_STATE,   // Tell slave to set port states on all ports of the switch
    DOT1X_MSG_ID_MST_TO_SLV_EAPOL,          // Tell slave to send an EAPOL frame (BPDU) on one of its ports
    DOT1X_MSG_ID_SLV_TO_MST_EAPOL,          // Slave received an EAPOL frame (BPDU) on one of its front ports and is now sending it to the master.
} dot1x_msg_id_t;

/******************************************************************************/
// Current version of 802.1X messages (1-based).
// Future revisions of this module should support previous versions if applicable.
/******************************************************************************/
#define DOT1X_MSG_VERSION 3

/******************************************************************************/
// Mst->Slv. State of single port.
// Sent when the authorized state or the configuration of the port changes.
/******************************************************************************/
typedef struct {
    vtss_port_no_t api_port;
    BOOL           authorized;
} dot1x_msg_port_state_t;

/******************************************************************************/
// Msg->Slv. State of all switch ports.
// Sent when the configuration of a switch changes.
/******************************************************************************/
typedef struct {
    BOOL authorized[VTSS_PORTS];
} dot1x_msg_switch_state_t;

/****************************************************************************/
// Mst->Slv: Tx this EAPOL on a front a port.
// Slv->Mst: This EAPOL was received on a front port.
// Whether it's for the master or the slave is given by DOT1X_MSG_ID_xxx_EAPOL.
/****************************************************************************/
typedef struct {
    vtss_port_no_t api_port;                                 // Front port to receive/transmit the frame onto.
    size_t         len;                                      // Length of frame
    vtss_vid_t     vid;                                      // For Slv->Mst: The VLAN ID that this frame was received on.
    u8             frm[NAS_IEEE8021X_MAX_EAPOL_PACKET_SIZE]; // Actual frame (this field must come last, since we normally won't fill out all of the frame before sending it as a message).
} dot1x_msg_eapol_t;

/****************************************************************************/
// Message Identification Header
/****************************************************************************/
typedef struct {
    // Message Version Number
    u32 version; // Set to DOT1X_MSG_VERSION

    // Message ID
    dot1x_msg_id_t msg_id;
} dot1x_msg_hdr_t;

/****************************************************************************/
// Message.
// This struct contains a union, whose primary purpose is to give the
// size of the biggest of the contained structures.
/****************************************************************************/
typedef struct {
    // Header stuff
    dot1x_msg_hdr_t hdr;

    // Request message
    union {
        dot1x_msg_port_state_t   port_state;
        dot1x_msg_switch_state_t switch_state;
        dot1x_msg_eapol_t        eapol;
    } u;
} dot1x_msg_t;

/****************************************************************************/
// Message buffer pool. Statically allocated and protected by a semaphore.
/****************************************************************************/
typedef struct {
    // Buffers and semaphores
    vtss_os_sem_t sem;
    dot1x_msg_t   msg;
} dot1x_msg_buf_pool_t;

/****************************************************************************/
// Generic message structure, that can be used for both request and reply.
/****************************************************************************/
typedef struct {
    vtss_os_sem_t *sem;
    void          *msg;
} dot1x_msg_buf_t;

// Static, semaphore-protected message transmission buffer(s).
static dot1x_msg_buf_pool_t DOT1X_msg_buf_pool;

/******************************************************************************/
// DOT1X_msg_buf_alloc()
// Blocks until a buffer is available, then takes and returns it.
/******************************************************************************/
static dot1x_msg_t *DOT1X_msg_buf_alloc(dot1x_msg_buf_t *buf, dot1x_msg_id_t msg_id)
{
    dot1x_msg_t *msg = &DOT1X_msg_buf_pool.msg;
    buf->sem = &DOT1X_msg_buf_pool.sem;
    buf->msg = msg;
    (void)VTSS_OS_SEM_WAIT(buf->sem);
    msg->hdr.version = DOT1X_MSG_VERSION;
    msg->hdr.msg_id = msg_id;
    return msg;
}

/******************************************************************************/
// DOT1X_msg_id_to_str()
/******************************************************************************/
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_NOISE)
static char *DOT1X_msg_id_to_str(dot1x_msg_id_t msg_id)
{
    switch (msg_id) {
    case DOT1X_MSG_ID_MST_TO_SLV_PORT_STATE:
        return "MST_TO_SLV_PORT_STATE";

    case DOT1X_MSG_ID_MST_TO_SLV_SWITCH_STATE:
        return "MST_TO_SLV_SWITCH_STATE";

    case DOT1X_MSG_ID_MST_TO_SLV_EAPOL:
        return "MST_TO_SLV_EAPOL";

    case DOT1X_MSG_ID_SLV_TO_MST_EAPOL:
        return "SLV_TO_MST_EAPOL";

    default:
        return "***Unknown Message ID***";
    }
}
#endif /* VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_NOISE */

/******************************************************************************/
// DOT1X_msg_tx_done()
// Called when message successfully or unsuccessfully transmitted.
/******************************************************************************/
static void DOT1X_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    // The context contains a pointer to the semaphore
    // protecting the transmitted buffer. Release it.
    VTSS_OS_SEM_POST(contxt);
}

/******************************************************************************/
// DOT1X_msg_tx()
// Do transmit a message.
/******************************************************************************/
static void DOT1X_msg_tx(dot1x_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(buf->sem, DOT1X_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_DOT1X, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(dot1x_msg_t, u));
}

/******************************************************************************/
// DOT1X_msg_tx_port_state()
/******************************************************************************/
static void DOT1X_msg_tx_port_state(vtss_isid_t isid, vtss_port_no_t api_port, BOOL authorized)
{
    dot1x_msg_buf_t buf;
    dot1x_msg_t     *msg;

    // Update the flags
    if (authorized) {
        DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags |=  DOT1X_PORT_STATE_FLAGS_AUTHORIZED;
    } else {
        DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags &= ~DOT1X_PORT_STATE_FLAGS_AUTHORIZED;
    }

    if (!msg_switch_exists(isid)) {
        return;
    }

    // Get a buffer.
    msg = DOT1X_msg_buf_alloc(&buf, DOT1X_MSG_ID_MST_TO_SLV_PORT_STATE);

    // Copy state to buffer
    msg->u.port_state.api_port   = api_port;
    msg->u.port_state.authorized = authorized;

    T_D("%d:%d: Tx state (%s)", isid, iport2uport(api_port), authorized ? "Auth" : "Unauth");

    // Transmit it.
    DOT1X_msg_tx(&buf, isid, sizeof(msg->u.port_state));
}

/******************************************************************************/
// DOT1X_msg_tx_switch_state()
/******************************************************************************/
static void DOT1X_msg_tx_switch_state(vtss_isid_t isid, BOOL *authorized)
{
    dot1x_msg_buf_t buf;
    dot1x_msg_t     *msg;

    if (!msg_switch_exists(isid)) {
        return;
    }

    // Get a buffer.
    msg = DOT1X_msg_buf_alloc(&buf, DOT1X_MSG_ID_MST_TO_SLV_SWITCH_STATE);

    // Copy state to buffer
    memcpy(msg->u.switch_state.authorized, authorized, sizeof(msg->u.switch_state.authorized));

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG) /* Compile check */
    if (TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, VTSS_TRACE_GRP_DEFAULT, VTSS_TRACE_LVL_DEBUG)) { /* Runtime check */
        port_iter_t pit;
        char        dbg_buf[VTSS_PORTS + 1];
        memset(dbg_buf, 0, sizeof(dbg_buf));

        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            dbg_buf[pit.iport] = authorized[pit.iport] ? '1' : '0';
        }
        T_D("%d:<all>: Tx state (%s)", isid, dbg_buf);
    }
#endif /* VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG */

    // Transmit it.
    DOT1X_msg_tx(&buf, isid, sizeof(msg->u.switch_state));
}

/******************************************************************************/
// DOT1X_msg_tx_eapol()
/******************************************************************************/
static void DOT1X_msg_tx_eapol(vtss_isid_t isid, vtss_port_no_t api_port, dot1x_msg_id_t msg_id, vtss_vid_t vid, const u8 *const frm, size_t len)
{
    dot1x_msg_buf_t buf;
    dot1x_msg_t     *msg;

    if (len > sizeof(msg->u.eapol.frm)) {
        T_E("%d:%d: Attempting to send a frame whose length is larger than we can handle (%zu > %zu)", isid, iport2uport(api_port), len, sizeof(msg->u.eapol.frm));
        return;
    }

    // Get a buffer
    msg = DOT1X_msg_buf_alloc(&buf, msg_id);

    // Copy frame to message buffer
    msg->u.eapol.api_port = api_port;
    msg->u.eapol.len      = len;
    msg->u.eapol.vid      = vid;
    memcpy(&msg->u.eapol.frm[0], frm, len);

    T_N("%d:%d: Sending BPDU to %s, len=%zu, vid=%d", isid, iport2uport(api_port), msg_id == DOT1X_MSG_ID_SLV_TO_MST_EAPOL ? "master" : "slave", len, vid);

    // Send message
    DOT1X_msg_tx(&buf, isid, sizeof(dot1x_msg_eapol_t) - (sizeof(msg->u.eapol.frm) - len));
}

/******************************************************************************/
// DOT1X_tx_switch_state()
/******************************************************************************/
static void DOT1X_tx_switch_state(vtss_isid_t isid, BOOL glbl_enabled)
{
    BOOL        authorized[VTSS_PORTS];
    port_iter_t pit;

    memset(authorized, 0, sizeof(authorized));

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        nas_port_control_t admin_state = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[pit.iport].admin_state;
        authorized[pit.iport] = !glbl_enabled || NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state) || admin_state == NAS_PORT_CONTROL_FORCE_AUTHORIZED;
        if (authorized[pit.iport]) {
            DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[pit.iport].flags |= DOT1X_PORT_STATE_FLAGS_AUTHORIZED;
        } else {
            DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[pit.iport].flags &= ~DOT1X_PORT_STATE_FLAGS_AUTHORIZED;
        }
    }

    DOT1X_msg_tx_switch_state(isid, authorized);
}

/******************************************************************************/
// DOT1X_tx_port_state()
/******************************************************************************/
static void DOT1X_tx_port_state(vtss_isid_t isid, vtss_port_no_t api_port, BOOL glbl_enabled, nas_port_control_t admin_state)
{
    // Check to see if we must transmit the new port control
    dot1x_port_state_t *port_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START];
    BOOL               cur_authorized = (port_state->flags & DOT1X_PORT_STATE_FLAGS_AUTHORIZED) ? TRUE : FALSE;
    BOOL               new_authorized = !glbl_enabled || NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state) || admin_state == NAS_PORT_CONTROL_FORCE_AUTHORIZED;

    if (new_authorized != cur_authorized) {
        DOT1X_msg_tx_port_state(isid, api_port, new_authorized);
    }
}

/******************************************************************************/
// DOT1X_cfg_valid_glbl()
/******************************************************************************/
static vtss_rc DOT1X_cfg_valid_glbl(dot1x_glbl_cfg_t *glbl_cfg)
{
    if (glbl_cfg->reauth_period_secs < DOT1X_REAUTH_PERIOD_SECS_MIN || glbl_cfg->reauth_period_secs > DOT1X_REAUTH_PERIOD_SECS_MAX) {
        return DOT1X_ERROR_INVALID_REAUTH_PERIOD;
    }

    // In case someone changes DOT1X_EAPOL_TIMEOUT_SECS_MAX to something smaller than what eapol_timeout_secs
    // can cope with, we need to do the test for glbl_cfg->eapol_timeout_secs > DOT1X_EAPOL_TIMEOUT_SECS_MAX,
    // so tell Lint not to report "Warning -- Relational operator '>' always evaluates to 'false')
    /*lint -e{685} */
    if (glbl_cfg->eapol_timeout_secs < DOT1X_EAPOL_TIMEOUT_SECS_MIN || glbl_cfg->eapol_timeout_secs > DOT1X_EAPOL_TIMEOUT_SECS_MAX) {
        return DOT1X_ERROR_INVALID_EAPOL_TIMEOUT;
    }

#ifdef NAS_USES_PSEC
    if (glbl_cfg->psec_aging_period_secs < NAS_PSEC_AGING_PERIOD_SECS_MIN || glbl_cfg->psec_aging_period_secs > NAS_PSEC_AGING_PERIOD_SECS_MAX) {
        return DOT1X_ERROR_INVALID_AGING_PERIOD;
    }

    if (glbl_cfg->psec_hold_time_secs < NAS_PSEC_HOLD_TIME_SECS_MIN || glbl_cfg->psec_hold_time_secs > NAS_PSEC_HOLD_TIME_SECS_MAX) {
        return DOT1X_ERROR_INVALID_HOLD_TIME;
    }
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    if (glbl_cfg->guest_vid < 1 || glbl_cfg->guest_vid >= VTSS_VIDS) {
        return DOT1X_ERROR_INVALID_GUEST_VLAN_ID;
    }

    if (glbl_cfg->reauth_max < 1 || glbl_cfg->reauth_max > 255) {
        return DOT1X_ERROR_INVALID_REAUTH_MAX;
    }
#endif

    return VTSS_RC_OK;
}

/******************************************************************************/
// DOT1X_cfg_valid_port()
/******************************************************************************/
static DOT1X_INLINE vtss_rc DOT1X_cfg_valid_port(dot1x_port_cfg_t *port_cfg)
{
    if (
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        port_cfg->admin_state != NAS_PORT_CONTROL_DOT1X_SINGLE     &&
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        port_cfg->admin_state != NAS_PORT_CONTROL_DOT1X_MULTI      &&
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        port_cfg->admin_state != NAS_PORT_CONTROL_MAC_BASED        &&
#endif
        port_cfg->admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED &&
        port_cfg->admin_state != NAS_PORT_CONTROL_AUTO             &&
        port_cfg->admin_state != NAS_PORT_CONTROL_FORCE_UNAUTHORIZED) {
        T_W("Invalid administrative state");
        return DOT1X_ERROR_INVALID_ADMIN_STATE;
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// DOT1X_cfg_valid_switch()
/******************************************************************************/
static vtss_rc DOT1X_cfg_valid_switch(dot1x_switch_cfg_t *switch_cfg)
{
    vtss_port_no_t api_port;
    vtss_rc        rc;

    for (api_port = 0; api_port < VTSS_PORTS; api_port++) {
        if ((rc = DOT1X_cfg_valid_port(&switch_cfg->port_cfg[api_port])) != VTSS_RC_OK) {
            return rc;
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// DOT1X_cfg_valid()
// Checks if the configuration is valid.
// Returns TRUE if so, FALSE otherwise.
/******************************************************************************/
static DOT1X_INLINE vtss_rc DOT1X_cfg_valid(dot1x_stack_cfg_t *cfg)
{
    vtss_isid_t zisid;
    vtss_rc     rc;

    if ((rc = DOT1X_cfg_valid_glbl(&cfg->glbl_cfg)) == VTSS_RC_OK) {
        for (zisid = 0; zisid < VTSS_ISID_CNT; zisid++) {
            if ((rc = DOT1X_cfg_valid_switch(&cfg->switch_cfg[zisid])) != VTSS_RC_OK) {
                break;
            }
        }
    }
    if (rc != VTSS_RC_OK) {
        T_W(dot1x_error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// DOT1X_cfg_default_switch()
// Initialize per-switch settings to defaults.
/******************************************************************************/
void DOT1X_cfg_default_switch(dot1x_switch_cfg_t *cfg)
{
    vtss_port_no_t api_port;

    memset(cfg, 0, sizeof(dot1x_switch_cfg_t));
    for (api_port = 0; api_port < VTSS_PORTS; api_port++) {
        dot1x_port_cfg_t *port_cfg = &cfg->port_cfg[api_port];

        port_cfg->admin_state = NAS_PORT_CONTROL_FORCE_AUTHORIZED;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        port_cfg->qos_backend_assignment_enabled = FALSE;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        port_cfg->vlan_backend_assignment_enabled = FALSE;
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        port_cfg->guest_vlan_enabled = FALSE;
#endif
    }
}

/******************************************************************************/
// DOT1X_cfg_default_glbl()
// Initialize global settings to defaults.
/******************************************************************************/
void DOT1X_cfg_default_glbl(dot1x_glbl_cfg_t *cfg)
{
    // First reset whole structure...
    memset(cfg, 0, sizeof(dot1x_glbl_cfg_t));

    // ...then override specific fields
    cfg->reauth_period_secs = DOT1X_REAUTH_PERIOD_SECS_DEFAULT;
    cfg->eapol_timeout_secs = DOT1X_EAPOL_TIMEOUT_SECS_DEFAULT;


#ifdef NAS_USES_PSEC
    cfg->psec_aging_enabled     = NAS_PSEC_AGING_ENABLED_DEFAULT;
    cfg->psec_aging_period_secs = NAS_PSEC_AGING_PERIOD_SECS_DEFAULT;
    cfg->psec_hold_enabled      = NAS_PSEC_HOLD_ENABLED_DEFAULT;
    cfg->psec_hold_time_secs    = NAS_PSEC_HOLD_TIME_SECS_DEFAULT;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    cfg->qos_backend_assignment_enabled = FALSE;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    cfg->vlan_backend_assignment_enabled = FALSE;
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    cfg->guest_vlan_enabled      = FALSE;
    cfg->guest_vid               = 1;
    cfg->reauth_max              = 2;
    cfg->guest_vlan_allow_eapols = FALSE;
#endif
}

/******************************************************************************/
// DOT1X_cfg_default_all()
// Initialize global and per-switch settings
/******************************************************************************/
static void DOT1X_cfg_default_all(dot1x_stack_cfg_t *cfg)
{
    vtss_isid_t zisid;

    DOT1X_cfg_default_glbl(&cfg->glbl_cfg);
    // Create defaults per switch.
    for (zisid = 0; zisid < VTSS_ISID_CNT; zisid++) {
        DOT1X_cfg_default_switch(&cfg->switch_cfg[zisid]);
    }
}

/******************************************************************************/
// DOT1X_vlan_force_unaware_get()
// Return TRUE if we should force the port in VLAN unaware mode.
// FALSE otherwise.
/******************************************************************************/
static DOT1X_INLINE BOOL DOT1X_vlan_force_unaware_get(nas_port_control_t admin_state)
{
    // All BPDU-based modes require VLAN unawareness.
    // In principle, port-based 802.1X wouldn't necessarily require it unless RADIUS-assigned VLANs were enabled,
    // but for simplicity, we set it to VLAN unaware.
    return NAS_PORT_CONTROL_IS_BPDU_BASED(admin_state);
}

/******************************************************************************/
// DOT1X_vlan_awareness_set()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_vlan_awareness_set(nas_port_t nas_port, nas_port_control_t admin_state)
{
    vtss_isid_t     isid     = DOT1X_NAS_PORT_2_ISID(nas_port);
    vtss_port_no_t  api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);
    vtss_uport_no_t uport    = iport2uport(api_port);
    vtss_rc         rc;

    vlan_port_conf_t vlan_cfg;
    memset(&vlan_cfg, 0, sizeof(vlan_cfg));
    if (DOT1X_vlan_force_unaware_get(admin_state)) {
        T_D("%d:%d: Forcing VLAN unaware", isid, uport);
        vlan_cfg.flags |= VLAN_PORT_FLAGS_AWARE;
    } else {
        T_D("%d:%d: Unsetting VLAN unawareness", isid, uport);
    }
    if (msg_switch_configurable(isid)) {
        if ((rc = vlan_mgmt_port_conf_set(isid, api_port, &vlan_cfg, VLAN_USER_DOT1X)) != VTSS_RC_OK) {
            T_E("%u:%d: Unable to change VLAN awareness (%s)", isid, uport, error_txt(rc));
        }
    }
}

#ifdef NAS_USES_VLAN
/******************************************************************************/
// DOT1X_vlan_membership_set()
/******************************************************************************/
static void DOT1X_vlan_membership_set(vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_t vid, BOOL add_membership)
{
    vtss_uport_no_t   uport = iport2uport(api_port);
    vlan_mgmt_entry_t entry;

    if (vid != 0) {
        // Gotta set or clear this port's membership for new_vid.
        // Get the current port mask for this VID.
        if (vlan_mgmt_vlan_get(isid, vid, &entry, FALSE, VLAN_USER_DOT1X) != VTSS_RC_OK) {
            if (add_membership == FALSE) {
                // Only print a warning if we expect the VID to be there (i.e. when we're trying
                // to remove our membership).
                T_W("%u:%d: Unable to get current membership ports of vid = %d", isid, uport, vid);
            }
            memset(&entry, 0, sizeof(entry));
            entry.vid = vid;
        } else {
            if (entry.vid != vid) {
                T_W("%u:%d: The port membership entry's VID (%d) is not what we requested (%d)", isid, uport, entry.vid, vid);
                entry.vid = vid;
            }

            // If going to add membership, we expect that it previously was cleared (and hence have to compare with TRUE)
            if (entry.ports[api_port] == add_membership) {
                T_W("%u:%d: Expected port member of vid = %d to be %s, but it isn't", isid, uport, vid, add_membership ? "cleared" : "set");
            }
        }

        entry.ports[api_port] = add_membership;
        if (vlan_mgmt_vlan_add(isid, &entry, VLAN_USER_DOT1X) != VTSS_RC_OK) {
            T_W("%u:%d: Couldn't %s port bit in membership entry (vid = %d)", isid, uport, add_membership ? "set" : "clear", vid);
        }
    }
}
#endif

#ifdef NAS_USES_VLAN
/******************************************************************************/
// DOT1X_vlan_port_set()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_vlan_port_set(nas_port_info_t *port_info, vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_t new_vid)
{
    vlan_port_conf_t vlan_cfg;
    vtss_rc          rc;

    memset(&vlan_cfg, 0, sizeof(vlan_cfg));

    if (new_vid != 0) {
        vlan_cfg.pvid   = new_vid;
        vlan_cfg.flags |= VLAN_PORT_FLAGS_PVID;
    }
    if (DOT1X_vlan_force_unaware_get(port_info->port_control)) {
        vlan_cfg.flags |= VLAN_PORT_FLAGS_AWARE;
    }

    if ((rc = vlan_mgmt_port_conf_set(isid, api_port, &vlan_cfg, VLAN_USER_DOT1X)) != VTSS_RC_OK) {
        T_E("%u:%d: Unable to set PVID volatile to %d (%s)", isid, iport2uport(api_port), new_vid, error_txt(rc));
    }
}
#endif /* NAS_USES_VLAN */

#ifdef NAS_USES_VLAN
/******************************************************************************/
// DOT1X_vlan_set()
/******************************************************************************/
static void DOT1X_vlan_set(nas_port_t nas_port, vtss_vid_t new_vid, nas_vlan_type_t vlan_type)
{
    nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));

    DOT1X_CRIT_ASSERT_LOCKED();

    if (new_vid != port_info->current_vid) {
        vtss_isid_t     isid       = DOT1X_NAS_PORT_2_ISID(nas_port);
        vtss_port_no_t  api_port   = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);

        T_I("%d:%d: Changing PVID from %d to %d", isid, iport2uport(api_port), port_info->current_vid, new_vid);

        DOT1X_vlan_port_set(port_info, isid, api_port, new_vid);
        // First clear the old membership
        DOT1X_vlan_membership_set(isid, api_port, port_info->current_vid, FALSE);
        // Then add the new.
        DOT1X_vlan_membership_set(isid, api_port, new_vid,                TRUE);
        port_info->current_vid = new_vid;
    }

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    // This can only survive one single call to this function.
    port_info->backend_assigned_vid = 0;
#endif

    // Change the type
    port_info->vlan_type = vlan_type;
}
#endif

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_psec_loop_through_callback()
// This function will be called back by the Port Security module when we enable
// or disable port security on a port.
// There are three cases to consider:
// 1) Going from a non-MAC-table-based to a MAC-table-based mode:
//   Delete all existing entries. Entries shouldn't be found in 802.1X' list
//   of SMs.
// 2) Going from a MAC-table-based to another MAC-table based mode:
//   Delete all existing entries. All entries should be found in 802.1X' list
//   of SMs.
// 3) Going from a MAC-table-based to a non-MAC-table based mode:
//   Delete all existing entries. All entries should be found in 802.1X' list
//   of SMs.
/******************************************************************************/
static psec_add_method_t DOT1X_psec_loop_through_callback(void                       *user_ctx,
                                                          vtss_isid_t                isid,
                                                          vtss_port_no_t             api_port,
                                                          vtss_vid_mac_t             *vid_mac,
                                                          u32                        mac_cnt_before_callback,
                                                          BOOL                       *keep,
                                                          psec_loop_through_action_t *action)
{
    nas_sm_t          *sm;
    nas_port_t        nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
    nas_stop_reason_t *stop_reason = user_ctx;

    DOT1X_CRIT_ASSERT_LOCKED();

    // Remove this entry.
    *keep = FALSE;

    // Free the corresponding SM.
    sm = nas_get_sm_from_vid_mac_port(vid_mac, nas_port);

    if (sm) {
        // Everything is OK. Free our own resources.
        T_D("%d:%d: Freeing SM", isid, iport2uport(api_port));
        nas_free_sm(sm, *stop_reason);
    }

    return PSEC_ADD_METHOD_FORWARD; // Doesn't matter when @keep is set to FALSE
}
#endif /* NAS_USES_PSEC */

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_psec_use_chg()
/******************************************************************************/
static void DOT1X_psec_use_chg(vtss_isid_t isid, vtss_port_no_t api_port, nas_stop_reason_t stop_reason, BOOL enable, psec_port_mode_t port_mode)
{
    vtss_rc rc;
    if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_DOT1X, (void *)&stop_reason, isid, api_port, enable, FALSE, DOT1X_psec_loop_through_callback, port_mode)) != VTSS_RC_OK) {
        if (rc == PSEC_ERROR_MUST_BE_MASTER) {
            // In some scenarios, where a switch is going from being a master to being a slave, it's OK to get a PSEC_ERROR_MUST_BE_MASTER error code.
            T_D("%d:%d: psec fail: %s", isid, iport2uport(api_port), error_txt(rc));
        } else {
            T_E("%d:%d: psec fail: %s", isid, iport2uport(api_port), error_txt(rc));
        }
    }
}
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_guest_vlan_enter()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_guest_vlan_enter(vtss_isid_t isid, vtss_port_no_t api_port, nas_port_info_t *port_info)
{
    T_D("%d:%d: Entering Guest VLAN", isid, iport2uport(api_port));

    // If MAC-table-based, exit PSEC enable. Here, we should ask the PSEC module for
    // keeping the port open even though we're not a member, so that we can get
    // BPDUs.
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(port_info->port_control)) {
        // This will delete all SMs currently on the port and cause the base lib
        // to auto-allocate a request identity SM, which is the one we will
        // massage further down in this function.
        T_D("%d:%d: Changing PSEC enable to FALSE - start", isid, iport2uport(api_port));
        DOT1X_psec_use_chg(isid, api_port, NAS_STOP_REASON_GUEST_VLAN_ENTER, FALSE, PSEC_PORT_MODE_KEEP_BLOCKED);
        T_D("%d:%d: Changing PSEC enable to FALSE - done", isid, iport2uport(api_port));
    }

    // Put it into Guest VLAN
    DOT1X_vlan_set(port_info->port_no, DOT1X_stack_cfg.glbl_cfg.guest_vid, NAS_VLAN_TYPE_GUEST_VLAN);

    // Gotta set the port in authorized state (only needed for port-based), and only
    // needed because the nas_set_fake_force_authorized() doesn't call-back the
    // nas_os_set_authorized(), because that would start accounting.
    if (port_info->port_control == NAS_PORT_CONTROL_AUTO) {
        DOT1X_msg_tx_port_state(isid, api_port, TRUE);
    }

    // Put the SM into fake force authorized, so that it doesn't reauthenticate, re-initialize, or react on anything.
    nas_set_fake_force_authorized(port_info->top_sm, TRUE);
}
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_guest_vlan_exit()
/******************************************************************************/
static void DOT1X_guest_vlan_exit(vtss_isid_t isid, vtss_port_no_t api_port, nas_port_info_t *port_info)
{
    if (port_info->vlan_type != NAS_VLAN_TYPE_GUEST_VLAN) {
        return;
    }

    T_D("%d:%d: Exiting Guest VLAN", isid, iport2uport(api_port));

    // Take it out of the Guest VLAN mode.
    DOT1X_vlan_set(port_info->port_no, 0, NAS_VLAN_TYPE_NONE);

    // If MAC-table-based, re-enter PSEC enable. All MAC-addresses that were learned by
    // other PSEC users will get deleted.
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(port_info->port_control)) {
        DOT1X_psec_use_chg(isid, api_port, NAS_STOP_REASON_GUEST_VLAN_EXIT, TRUE, PSEC_PORT_MODE_KEEP_BLOCKED);
    }

    // Re-initialize state machine. This will cause the SM to call-back
    // nas_os_set_authorized(), which in turn will cause the new port state
    // to be set to unauthorized for port-based 802.1X.
    nas_set_fake_force_authorized(port_info->top_sm, FALSE);
}
#endif

/******************************************************************************/
// DOT1X_set_port_control()
// Free existing SMs and set the port control
/******************************************************************************/
static void DOT1X_set_port_control(nas_port_t nas_port, nas_port_control_t new_admin_state, nas_stop_reason_t stop_reason)
{
#if defined(VTSS_SW_OPTION_PSEC) || defined(VTSS_SW_OPTION_NAS_GUEST_VLAN)
    vtss_isid_t    isid     = DOT1X_NAS_PORT_2_ISID(nas_port);
    vtss_port_no_t api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));
    DOT1X_guest_vlan_exit(isid, api_port, port_info);
    if (stop_reason == NAS_STOP_REASON_PORT_MODE_CHANGED ||
        stop_reason == NAS_STOP_REASON_PORT_LINK_DOWN    ||
        stop_reason == NAS_STOP_REASON_SWITCH_DOWN       ||
        stop_reason == NAS_STOP_REASON_PORT_SHUT_DOWN) {
        port_info->eapol_frame_seen = FALSE;
    }
#endif

#ifdef VTSS_SW_OPTION_PSEC
    {
        // Due to a bug in EStaX-34, we have to inform the PSEC module not to stop CPU-copying - unless the port is shut down -
        // if we're in a mode that requires BPDUs. This is whether we're enabling PSEC above or not (i.e. whether we're in a port-based or another BPDU-requiring mode or not).
        // Even in port-based 802.1X, we must get the BPDUs, and if another module has enabled PSEC, it might happen that we didn't get these
        // BPDUs if the limit was reached.
        // The function will always return VTSS_RC_OK on non-buggy chips.
        vtss_rc rc;
        if ((rc = psec_mgmt_force_cpu_copy(PSEC_USER_DOT1X, isid, api_port, NAS_PORT_CONTROL_IS_BPDU_BASED(new_admin_state)))) {
            T_D("%d:%d: Error from PSEC: %s", isid + VTSS_ISID_START, iport2uport(api_port + VTSS_PORT_NO_START), error_txt(rc));
        }
    }
#endif

#ifdef NAS_USES_PSEC
    {
        // Update the Port Security module with our new state.
        BOOL new_is_mac_table_based  = NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(new_admin_state);
        BOOL new_is_bpdu_based       = NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(new_admin_state);

        // If either the old or the new was MAC table based, we need to inform the Port Security module to either
        // enable our use and delete all current entries (deleting all entries is done through the
        // DOT1X_psec_loop_through_callback() callback function), or to disable our use. Whether one or the other
        // the DOT1X_psec_loop_through_callback() callback function will be called and all learned MAC addresses
        // will get removed.
        // Any attached SMs will be deleted through the call to DOT1X_psec_loop_through_callback().
        // The @ctx parameter holds the reason for deleting the SM.
        // Also, if we're in an 802.1X mode (single or multi, that is) and not a MAC-based mode, we must
        // ask the PSEC module to not do CPU copying initially, since we have to gain control of this flag
        // and add the MAC addresses ourselves based on BPDUs.
        DOT1X_psec_use_chg(isid, api_port, stop_reason, new_is_mac_table_based, new_is_bpdu_based ? PSEC_PORT_MODE_KEEP_BLOCKED : PSEC_PORT_MODE_NORMAL);
    }
#endif

    nas_free_all_sms(nas_port, stop_reason);
    nas_set_port_control(nas_port, new_admin_state);

    DOT1X_vlan_awareness_set(nas_port, new_admin_state);
}

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
/******************************************************************************/
// DOT1X_qos_set()
/******************************************************************************/
static void DOT1X_qos_set(nas_port_t nas_port, vtss_prio_t qos_class)
{
    nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));

    DOT1X_CRIT_ASSERT_LOCKED();

    if (port_info->qos_class != qos_class) {
        vtss_isid_t     isid     = DOT1X_NAS_PORT_2_ISID(nas_port);
        vtss_port_no_t  api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);
        vtss_uport_no_t uport    = iport2uport(api_port);
        if (qos_class == QOS_PORT_PRIO_UNDEF) {
            T_D("%d:%d: Unsetting QoS class", isid, uport);
        } else {
            T_D("%d:%d: Setting QoS class to %u", isid, uport, qos_class);
        }
        if (qos_port_volatile_set_default_prio(isid, api_port, qos_class) != VTSS_RC_OK) {
            T_W("%d:%d: Qos setting to %u failed", isid, uport, qos_class);
        }
        port_info->qos_class = qos_class;
    }
}
#endif

#ifdef NAS_USES_PSEC
/****************************************************************************/
// DOT1X_psec_chg()
/****************************************************************************/
static void DOT1X_psec_chg(nas_port_t nas_port, vtss_isid_t isid, vtss_port_no_t api_port, nas_port_info_t *port_info, nas_eap_info_t *eap_info, nas_client_info_t *client_info, psec_add_method_t new_method)
{
    vtss_uport_no_t uport = iport2uport(api_port);
    vtss_rc         rc;

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    vtss_vid_t      next_pvid, next_mac_vid;
    nas_vlan_type_t vlan_type;

    if (new_method == PSEC_ADD_METHOD_FORWARD) {
        next_pvid = port_info->backend_assigned_vid;
    } else {
        next_pvid = 0;
    }

    vlan_type = next_pvid ? NAS_VLAN_TYPE_BACKEND_ASSIGNED : NAS_VLAN_TYPE_NONE;

    // Sanity checks
    // The MAC's VID must be the current overridden - if overridden.
    if (port_info->current_vid != 0 && port_info->current_vid != client_info->vid_mac.vid) {
        T_E("%d:%d(%s): Invalid state. Current VID = %d, MAC's VID = %d", isid, uport, client_info->mac_addr_str, port_info->current_vid, client_info->vid_mac.vid);
    }

    // If overridden, then the revert VID must be non-zero.
    if (port_info->current_vid != 0 && eap_info->revert_vid == 0) {
        T_E("%d:%d(%s): Invalid revert VID (0). Current VID = %d.", isid, uport, client_info->mac_addr_str, port_info->current_vid);
    }

    if (port_info->current_vid == next_pvid) {
        // Going from non-overridden to non-overridden or
        // from overriden to the same overridden. Either way,
        // just change the MAC address' forward state.
        next_mac_vid = client_info->vid_mac.vid;
    } else if (port_info->current_vid == 0) {
        // Changing from not overridden to overridden.
        // Save the current MAC's VID in revert VID.
        eap_info->revert_vid = client_info->vid_mac.vid;
        next_mac_vid = next_pvid;
    } else if (next_pvid == 0) {
        // Changing from overridden to not overridden.
        // Go back to the revert VID.
        next_mac_vid = eap_info->revert_vid;
    } else {
        // Changing from one overridden to another overridden
        next_mac_vid = next_pvid;
    }

    if (client_info->vid_mac.vid != next_mac_vid) {
        // Change VID by first deleting the current entry and then adding a new.
        T_D("%d:%d(%s): VID change from %d to %d. New method: %s", isid, uport, client_info->mac_addr_str, client_info->vid_mac.vid, next_mac_vid, psec_add_method_to_str(new_method));

        // Gotta delete before adding for the sake of PSEC Limit, or we may saturate the port, causing a trap.
        if ((rc = psec_mgmt_mac_del(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac)) != VTSS_RC_OK) {
            T_E("%d:%d: Couldn't delete %s:%d from MAC table. Error=%s", isid, uport, client_info->mac_addr_str, client_info->vid_mac.vid, error_txt(rc));
        }

        client_info->vid_mac.vid = next_mac_vid;
        if ((rc = psec_mgmt_mac_add(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac, new_method)) != VTSS_RC_OK) {
            T_E("%d:%d: Couldn't add %s:%d to MAC table. Error=%s", isid, uport, client_info->mac_addr_str, client_info->vid_mac.vid, error_txt(rc));
            // Ask base lib to delete this SM upon the next timer tick (we're in the middle of a nas_os_XXX() call, so we can't
            // just free it.
            eap_info->delete_me = TRUE;
        }
    } else
#endif
    {
        // The new MAC VID is the same, so just change the current entry.
        T_D("%d:%d(%s:%d): Change method to %s", isid, uport, client_info->mac_addr_str, client_info->vid_mac.vid, psec_add_method_to_str(new_method));

        if ((rc = psec_mgmt_mac_chg(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac, new_method)) != VTSS_RC_OK) {
            T_E("%d:%d(%s): Error from PSEC module: %s", isid, uport, client_info->mac_addr_str, error_txt(rc));
        }
    }

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    // Always set the port VID, since that call also clears the backend_assigned_vid, which is necessary
    // in order to go back to non-overridden if e.g. the network adminstrator disables VLAN assignment.
    DOT1X_vlan_set(nas_port, next_pvid, vlan_type);
#endif
}
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// DOT1X_guest_vlan_enter_check()
/******************************************************************************/
static DOT1X_INLINE BOOL DOT1X_guest_vlan_enter_check(vtss_isid_t isid, vtss_port_no_t api_port, nas_port_info_t *port_info)
{
    // If either we're not in Port-based, Single- or Multi-802.1X mode, or
    // guest VLAN is not globally enabled, or
    // guest VLAN is not enabled on this port, or
    // we're not allowed to enter Guest VLAN if at least one EAPOL frame is seen, and one EAPOL frame *is* seen,
    // then there's nothing to do.
    if (NAS_PORT_CONTROL_IS_BPDU_BASED(port_info->port_control) == FALSE                                                       ||
        DOT1X_stack_cfg.glbl_cfg.guest_vlan_enabled             == FALSE                                                       ||
        DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].guest_vlan_enabled == FALSE ||
        (DOT1X_stack_cfg.glbl_cfg.guest_vlan_allow_eapols == FALSE && port_info->eapol_frame_seen)) {
        return FALSE;
    }

    DOT1X_guest_vlan_enter(isid, api_port, port_info);
    return TRUE;
}
#endif

/******************************************************************************/
// DOT1X_apply_cfg()
/******************************************************************************/
static vtss_rc DOT1X_apply_cfg(const vtss_isid_t isid, dot1x_stack_cfg_t *new_cfg, BOOL switch_cfg_may_be_changed, nas_stop_reason_t stop_reason)
{
    vtss_isid_t       zisid_start, zisid_end, zisid;
    vtss_port_no_t    api_port;
    dot1x_stack_cfg_t *old_cfg = &DOT1X_stack_cfg;
    BOOL              old_enabled = old_cfg->glbl_cfg.enabled;
    BOOL              new_enabled = new_cfg->glbl_cfg.enabled;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    BOOL              old_qos_glbl_enabled = old_cfg->glbl_cfg.qos_backend_assignment_enabled;
    BOOL              new_qos_glbl_enabled = new_cfg->glbl_cfg.qos_backend_assignment_enabled;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    BOOL              old_backend_vlan_glbl_enabled = old_cfg->glbl_cfg.vlan_backend_assignment_enabled;
    BOOL              new_backend_vlan_glbl_enabled = new_cfg->glbl_cfg.vlan_backend_assignment_enabled;
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    BOOL              old_guest_vlan_glbl_enabled = old_cfg->glbl_cfg.guest_vlan_enabled;
    BOOL              new_guest_vlan_glbl_enabled = new_cfg->glbl_cfg.guest_vlan_enabled;
    BOOL              old_guest_vlan_allow_eapols = old_cfg->glbl_cfg.guest_vlan_allow_eapols;
    BOOL              new_guest_vlan_allow_eapols = new_cfg->glbl_cfg.guest_vlan_allow_eapols;
    vtss_vid_t        old_guest_vid               = old_cfg->glbl_cfg.guest_vid;
    vtss_vid_t        new_guest_vid               = new_cfg->glbl_cfg.guest_vid;
#endif
#ifdef NAS_USES_PSEC
    vtss_rc           rc;
#endif

    DOT1X_CRIT_ASSERT_LOCKED();

    // Change the age- and hold-times if requested to.
    if (isid == VTSS_ISID_GLOBAL) {
        // When just booted, it's really not necessary to apply the re-auth param to the underlying SMs, since
        // there are no such SMs attached, since the SMs call nas_os_get_reauth_timer() whenever one is allocated.
        // This means that we can do with ONLY calling nas_reauth_param_changed() when the parameters have
        // actually changed from the current configuration.
        if (new_cfg->glbl_cfg.reauth_enabled     != old_cfg->glbl_cfg.reauth_enabled ||
            new_cfg->glbl_cfg.reauth_period_secs != old_cfg->glbl_cfg.reauth_period_secs) {
            // This will loop through all attached SMs and change the reAuthEnabled and reAuthWhen settings.
            nas_reauth_param_changed(new_cfg->glbl_cfg.reauth_enabled, new_cfg->glbl_cfg.reauth_period_secs);
        }

#ifdef NAS_USES_PSEC
        // This is a fast call if nothing has changed.
        if ((rc = psec_mgmt_time_cfg_set(
                      PSEC_USER_DOT1X,
                      new_cfg->glbl_cfg.psec_aging_enabled ? new_cfg->glbl_cfg.psec_aging_period_secs : 0,
                      new_cfg->glbl_cfg.psec_hold_enabled  ? new_cfg->glbl_cfg.psec_hold_time_secs    : PSEC_HOLD_TIME_MAX)) != VTSS_RC_OK) {
            return rc;
        }
#endif
    }

    // In the following, we use zero-based port- and isid-counters.
    if ((switch_cfg_may_be_changed && isid == VTSS_ISID_GLOBAL) || (old_enabled != new_enabled)
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        || (old_qos_glbl_enabled != new_qos_glbl_enabled)
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        || (old_backend_vlan_glbl_enabled != new_backend_vlan_glbl_enabled)
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        || (old_guest_vlan_glbl_enabled != new_guest_vlan_glbl_enabled)
        || (new_guest_vlan_glbl_enabled && (old_guest_vid != new_guest_vid || old_guest_vlan_allow_eapols != new_guest_vlan_allow_eapols))
#endif
       ) {
        // If one of the global enable/disable configuration options change,
        // then we must apply the enable/disable thing to all ports.
        zisid_start = 0;
        zisid_end   = VTSS_ISID_CNT - 1;
    } else if (switch_cfg_may_be_changed && isid != VTSS_ISID_GLOBAL) {
        zisid_start = zisid_end = isid - VTSS_ISID_START;
    } else {
        // Nothing more to do.
        return VTSS_RC_OK;
    }

    for (zisid = zisid_start; zisid <= zisid_end; zisid++) {
        if (old_enabled != new_enabled) {
            // 'Guess' the best new mode, so that we send as few messages as possible.
            DOT1X_tx_switch_state(zisid + VTSS_ISID_START, new_enabled);
        }

        dot1x_switch_cfg_t *new_switch_cfg = &new_cfg->switch_cfg[zisid];
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
        dot1x_switch_cfg_t *old_switch_cfg = &old_cfg->switch_cfg[zisid];
#endif
        for (api_port = 0; api_port < VTSS_PORTS; api_port++) {
            // The base lib may not agree on the admin_state, because if there's no link on the port, it will be set to NAS_PORT_CONTROL_DISABLED to remove all SMs from the port.
            nas_port_t         nas_port;
            nas_port_control_t cur_admin_state;
            nas_port_control_t new_admin_state;
            BOOL               reauthenticate = FALSE;
#ifdef NAS_USES_VLAN
            nas_sm_t           *top_sm;
            nas_port_info_t    *port_info;
#endif

            if (api_port >= port_isid_port_count(zisid + VTSS_ISID_START)) {
                break;
            }
            if (port_isid_port_no_is_stack(zisid + VTSS_ISID_START, api_port)) {
                continue;
            }

            // The base lib may not agree on the admin_state, because if there's no link on the port, it will be set to NAS_PORT_CONTROL_DISABLED to remove all SMs from the port.
            nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(zisid + VTSS_ISID_START, api_port + VTSS_PORT_NO_START);
            cur_admin_state = nas_get_port_control(nas_port);
            new_admin_state = new_enabled ? new_switch_cfg->port_cfg[api_port].admin_state : NAS_PORT_CONTROL_DISABLED;
#ifdef NAS_USES_VLAN
            top_sm    = nas_get_top_sm(nas_port);
            port_info = nas_get_port_info(top_sm);
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
            {
                BOOL old_qos_enabled = old_qos_glbl_enabled && old_switch_cfg->port_cfg[api_port].qos_backend_assignment_enabled;
                BOOL new_qos_enabled = new_qos_glbl_enabled && new_switch_cfg->port_cfg[api_port].qos_backend_assignment_enabled;
                if (NAS_PORT_CONTROL_IS_SINGLE_CLIENT(cur_admin_state) && new_qos_enabled != old_qos_enabled) {
                    // The port is a single-client port (which allows for overriding QoS), and the
                    // new mode is different from the old one. We need to revert any override of the QoS and request a reauthentication.
                    reauthenticate = TRUE;
                    DOT1X_qos_set(nas_port, QOS_PORT_PRIO_UNDEF);
                }
            }
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
            {
                BOOL old_backend_vlan_enabled = old_backend_vlan_glbl_enabled && old_switch_cfg->port_cfg[api_port].vlan_backend_assignment_enabled;
                BOOL new_backend_vlan_enabled = new_backend_vlan_glbl_enabled && new_switch_cfg->port_cfg[api_port].vlan_backend_assignment_enabled;
                if (NAS_PORT_CONTROL_IS_SINGLE_CLIENT(cur_admin_state) && old_backend_vlan_enabled != new_backend_vlan_enabled) {
                    // The port is a single-client port (which allows for overriding PVID), and the
                    // new mode is different from the old one. We need to revert any override of the VLAN and request a reauthentication.
                    reauthenticate = TRUE;

                    // Gotta revert the VLAN on the port (if set).
                    if (port_info->vlan_type == NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
                        nas_sm_t *sm = nas_get_next(top_sm);
                        if (sm) {
                            if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(cur_admin_state)) {
#ifdef NAS_USES_PSEC
                                // In principle, I should also set whether the MAC should go into BLOCK or KEEP_BLOCKED if unauthorized, but that's
                                // impossible as of now, and I don't think it really matters.
                                DOT1X_psec_chg(nas_port, zisid + VTSS_ISID_START, api_port + VTSS_PORT_NO_START, port_info, nas_get_eap_info(sm), nas_get_client_info(sm), nas_get_port_status(nas_port) == NAS_PORT_STATUS_AUTHORIZED ? PSEC_ADD_METHOD_FORWARD : PSEC_ADD_METHOD_BLOCK);
#endif
                            } else {
                                DOT1X_vlan_set(nas_port, 0, NAS_VLAN_TYPE_NONE);
                            }
                        }
                    }
                }
            }
#endif

            // Check if we also need to tell the new admin state to the base lib.
            if (cur_admin_state != new_admin_state && DOT1X_stack_state.switch_state[zisid].port_state[api_port].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
                T_D("%d:%d: Chg admin state from %s to %s", zisid + VTSS_ISID_START, iport2uport(api_port + VTSS_PORT_NO_START), dot1x_port_control_to_str(cur_admin_state, FALSE), dot1x_port_control_to_str(new_admin_state, FALSE));
                DOT1X_tx_port_state(zisid + VTSS_ISID_START, api_port + VTSS_PORT_NO_START, new_enabled, new_admin_state);
                DOT1X_set_port_control(nas_port, new_admin_state, stop_reason);
            }

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
            {
                BOOL old_guest_vlan_enabled = old_guest_vlan_glbl_enabled && old_switch_cfg->port_cfg[api_port].guest_vlan_enabled;
                BOOL new_guest_vlan_enabled = new_guest_vlan_glbl_enabled && new_switch_cfg->port_cfg[api_port].guest_vlan_enabled;
                if ((old_guest_vlan_enabled && !new_guest_vlan_enabled) || (new_guest_vlan_enabled && old_guest_vlan_allow_eapols && !new_guest_vlan_allow_eapols && port_info->eapol_frame_seen)) {
                    // Gotta revert the guest VLAN on the port (if set).
                    DOT1X_guest_vlan_exit(zisid + VTSS_ISID_START, api_port + VTSS_PORT_NO_START, port_info);
                } else if (port_info->vlan_type == NAS_VLAN_TYPE_GUEST_VLAN && old_guest_vid != new_guest_vid) {
                    DOT1X_vlan_set(nas_port, new_guest_vid, NAS_VLAN_TYPE_GUEST_VLAN);
                } else if (!old_guest_vlan_enabled && new_guest_vlan_enabled) {
                    // Reset the reauth count.
                    nas_reset_reauth_cnt(top_sm);
                }
            }
#endif

            if (reauthenticate) {
                nas_reauthenticate(nas_port);
            }
        } /* for (api_port...) */
    } /* for (zisid... */
    return VTSS_RC_OK;
}

/******************************************************************************/
// DOT1X_cfg_flash_read()
// Read/create and activate port configuration
//
// If @isid_add == [VTSS_ISID_START; VTSS_ISID_END[ then
//   If reading flash fails then /* Called from either INIT_CMD_CONF_DEF or INIT_CMD_MASTER_UP */
//     create defaults for all switches, including global settings. Only transfer to slave
//     switches if @create_defaults is TRUE, since a master-up event shouldn't cause a message Tx
//   else if @create_defaults == TRUE then /* Called from INIT_CMD_CONF_DEF */
//     create defaults for @isid_add, only and transfer message to @isid_add switch if necessary.
//   else
//     copy flash configuration to RAM cfg for @isid_add, only. Don't Tx messages.
// else /* @isid_add == VTSS_ISID_GLOBAL */
//   if reading flash fails then /* Called from either INIT_CMD_CONF_DEF or INIT_CMD_MASTER_UP */
//     create defaults for all switches, including global settings. Only transfer to slave
//     switches if @create_defaults is TRUE, since a master-up event shouldn't cause a message Tx.
//   else if @create_defaults is TRUE then /* Called from INIT_CMD_CONF_DEF */
//     only create global configuration defaults, and transfer messages to all slave switches if changed.
//   else /* Called from INIT_CMD_MASTER_UP */
//     special case. Copy *both* global and per-switch configuration to RAM cfg.
//     Don't Tx messages to any switches.
/******************************************************************************/
static void DOT1X_cfg_flash_read(vtss_isid_t isid_add, BOOL create_defaults)
{
    dot1x_flash_cfg_t *flash_cfg;
    dot1x_stack_cfg_t tmp_stack_cfg;
    BOOL              flash_read_failed = FALSE;
    BOOL              switch_cfg_may_be_changed;
    ulong             size;

    T_D("%d:<all>: Enter, create_defaults = %d", isid_add, create_defaults);

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_DOT1X, &size)) == NULL ||
            size != sizeof(dot1x_flash_cfg_t)) {
            T_W("%d:<all>: conf_sec_open() failed or size mismatch, creating defaults", isid_add);
            flash_cfg = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_DOT1X, sizeof(dot1x_flash_cfg_t));
            flash_read_failed = TRUE;
        } else if (flash_cfg->version != DOT1X_FLASH_CFG_VERSION) {
            T_W("%d:<all>: Version mismatch, creating defaults", isid_add);
            flash_read_failed = TRUE;
        }
    } else {
        flash_cfg         = NULL;
        flash_read_failed = TRUE;
    }

    DOT1X_CRIT_ASSERT_LOCKED();
    // Get the current settings into tmp_stack_cfg, so that we can compare changes later on.
    tmp_stack_cfg = DOT1X_stack_cfg;

    if (flash_read_failed || (flash_cfg && (DOT1X_cfg_valid(&flash_cfg->cfg) != VTSS_RC_OK))) {
        // We need to create new defaults for both local and global settings if flash read failed
        // or the configuration read from flash wasn't invalid.
        DOT1X_cfg_default_all(&tmp_stack_cfg);
        isid_add = VTSS_ISID_GLOBAL;
        switch_cfg_may_be_changed = TRUE;
    } else if (create_defaults) {
        // We could read the flash, but were asked to default either global or per-switch settings.
        if (isid_add == VTSS_ISID_GLOBAL) {
            // Default the global settings.
            DOT1X_cfg_default_glbl(&tmp_stack_cfg.glbl_cfg);
            switch_cfg_may_be_changed = FALSE;
        } else {
            // Default per-switch settings.
            DOT1X_cfg_default_switch(&tmp_stack_cfg.switch_cfg[isid_add - VTSS_ISID_START]);
            switch_cfg_may_be_changed = TRUE;
        }
    } else {
        // Flash read succeeded, the contents is valid, and we're not forced to create defaults.
        isid_add = VTSS_ISID_GLOBAL;
        switch_cfg_may_be_changed = FALSE;
        if (flash_cfg != NULL) {     // Quiet lint
            tmp_stack_cfg = flash_cfg->cfg;
        }
    }

    // Apply the new configuration
    (void)DOT1X_apply_cfg(isid_add, &tmp_stack_cfg, switch_cfg_may_be_changed, NAS_STOP_REASON_PORT_MODE_CHANGED);

    // Copy our temporary settings to the real settings.
    DOT1X_stack_cfg = tmp_stack_cfg;

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (flash_cfg) {
        flash_cfg->version = DOT1X_FLASH_CFG_VERSION;
        flash_cfg->cfg = tmp_stack_cfg;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_DOT1X);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
// DOT1X_handle_bpdu_reception()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_handle_bpdu_reception(vtss_isid_t isid, vtss_port_no_t api_port, u8 *frm, vtss_vid_t vid, size_t len)
{
    nas_port_t           nas_port      = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
    dot1x_switch_state_t *switch_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START];
    nas_port_control_t   admin_state   = nas_get_port_control(nas_port); // Gotta use the base-libs notion of port control, since the platform lib's port_cfg not necessarily reflects the actual.
    vtss_uport_no_t      uport         = iport2uport(api_port);
    char                 prefix_str[100];

    sprintf(prefix_str, "%d:%d(%s:%d): ", isid, uport, misc_mac2str(&frm[6]), vid);

    if (api_port >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, api_port)) {
        T_E("%s:Invalid port number. Dropping", prefix_str);
        return;
    }

    if (switch_state->switch_exists == FALSE || (switch_state->port_state[api_port - VTSS_PORT_NO_START].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) == 0) {
        // Received either too early or too late.
        T_D("%sBPDU rxd on non-existing switch or on linked-down port. Dropping", prefix_str);
        return;
    }

    if (!NAS_PORT_CONTROL_IS_BPDU_BASED(admin_state)) {
        // Received on a port that doesn't need BPDUs.
        T_D("%sBPDU rxd on port that isn't in a BPDU-mode. Dropping", prefix_str);
        return;
    }

    if (!DOT1X_stack_cfg.glbl_cfg.enabled) {
        T_D("%sDropping BPDU as NAS is globally disabled", prefix_str);
        return;
    }

    // Here, we're either in Port-based, Single-, or Multi- 802.1X

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    {
        nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));
        DOT1X_guest_vlan_exit(isid, api_port, nas_get_port_info(nas_get_top_sm(nas_port)));
        port_info->eapol_frame_seen = TRUE;
    }
#endif

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state)) {
        vtss_vid_mac_t vid_mac;
        nas_sm_t       *sm;
        vtss_rc        rc;

        // Here we're in Single- or Multi- 802.1X.

        // Received on a port that requires PSEC module intervention.
        // Check to see if we already have a state machine for this MAC address.
        memset(&vid_mac, 0, sizeof(vid_mac)); // Zero-out unused/padding bytes.
        memcpy(vid_mac.mac.addr, &frm[6], sizeof(vid_mac.mac.addr));
        vid_mac.vid = vid;
        // Only use the MAC address for lookup. And lookup on all ports.
        // Since we're in control of adding MAC addresses to the MAC table
        // (since we're in Single- or Multi-802.1X), we're also in charge
        // of detecting port-changes (and thereby possible VID changes).
        // In one of these modes, we only tolerate the same MAC address
        // once - despite the VID it was received on.
        sm = nas_get_sm_from_mac(&vid_mac);

        if (sm) {
            nas_port_info_t   *port_info   = nas_get_port_info(sm);
            nas_client_info_t *client_info = nas_get_client_info(sm);

            T_D("%sFound existing SM", prefix_str);

            // Check if it's moved.
            if (port_info->port_no != nas_port) {
                T_D("%sBPDU rxd, which was previously found on %d:%d", prefix_str, DOT1X_NAS_PORT_2_ISID(port_info->port_no), DOT1X_NAS_PORT_2_SWITCH_API_PORT(port_info->port_no));

                // Delete the old SM - both from the MAC table and the base lib
                if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(port_info->port_control)) {
                    // This call may fail if the port that the MAC address came from is
                    // MAC-based (as opposed to MAC-table and BPDU-based).
                    (void)psec_mgmt_mac_del(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac);
                }

                // The following call also takes care of unregistering volatile QoS and VLAN if needed.
                nas_free_sm(sm, NAS_STOP_REASON_STATION_MOVED);
                sm = NULL; // Cause the next if() to create a new SM.
            } else if (client_info->vid_mac.vid != vid_mac.vid) {
                // Received on another VLAN ID. We don't really care right now.
                T_D("%sBPDU rxd, which is another VID than what we thought (%d)", prefix_str, client_info->vid_mac.vid);
            }
        }

        if (!sm) {
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
            // The SM doesn't exist. We need to create another one - if allowed.
            nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));

            // If we're in single 802.1X and one SM is already allocated, drop it.
            // The SM may time-out after some time, and get deleted so that others
            // get a chance.
            if (admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE && port_info->cur_client_cnt >= 1) {
                T_D("%sBPDU dropped because another supplicant is using the Single 802.1X SM", prefix_str);
                return;
            }
#endif

            // Check if we can add it to the PSEC module.
            T_D("%sAdding as keep blocked", prefix_str);
            rc = psec_mgmt_mac_add(PSEC_USER_DOT1X, isid, api_port, &vid_mac, PSEC_ADD_METHOD_KEEP_BLOCKED);
            if (rc != VTSS_RC_OK) {
                // Either one of the other modules denied this MAC address, or it's a zombie (a MAC address that couldn't be
                // added to the MAC table due to its hash layout, or because the MAC module ran out of software entries).
                // Either way, we attempt to delete it (in case its a zombie).
                T_W("%sPSEC add failed: \"%s\". Deleting it from PSEC", prefix_str, error_txt(rc));
                (void)psec_mgmt_mac_del(PSEC_USER_DOT1X, isid, api_port, &vid_mac);

                if (rc == PSEC_ERROR_PORT_IS_SHUT_DOWN) {
                    // Since DOT1X_on_mac_del_callback() won't get called back if the Limit module determines to shut down the port
                    // during the above call to psec_mgmt_mac_add() (because the PSEC module don't call the calling user back to avoid
                    // mutex problems), we have to trap this event and remove all state machines on the port.
                    // The easiest way to do this is to fake an admin_state change.
                    T_D("%sRemoving all SMs from port", prefix_str);
                    DOT1X_set_port_control(nas_port, admin_state, NAS_STOP_REASON_PORT_SHUT_DOWN);
                }
                return;
            }

            // Now allocate an SM for it.
            T_D("%sAllocating SM", prefix_str);
            if ((sm = nas_alloc_sm(nas_port, &vid_mac)) == NULL) {
                T_E("%sCan't allocate SM. Moving entry to blocked w/ hold-time-expiration)", prefix_str);
                // We're out of SMs. Tell the PSEC module to time it out.
                // This will result in another error when the MAC address is removed
                // in the PSEC module (in the callback call to DOT1X_on_mac_del_callback()).
                (void)psec_mgmt_mac_chg(PSEC_USER_DOT1X, isid, api_port, &vid_mac, PSEC_ADD_METHOD_BLOCK);
                return;
            }
        }

        // Now the SM definitely exists. Give the BPDU to it.
        nas_ieee8021x_eapol_frame_received_sm(sm, frm, vid, len);
    } else
#endif
    {
        // Here we're in Port-based 802.1X
        nas_ieee8021x_eapol_frame_received(nas_port, frm, vid, len);
    }
}

/******************************************************************************/
// DOT1X_msg_rx()
/******************************************************************************/
static BOOL DOT1X_msg_rx(void *contxt, const void *const the_rxd_msg, size_t len, vtss_module_id_t modid, ulong isid)
{
    vtss_rc     rc;
    dot1x_msg_t *rx_msg = (dot1x_msg_t *)the_rxd_msg;

    T_N("%u:<any>: msg_id: %d, %s, ver: %u, len: %zu", isid, rx_msg->hdr.msg_id, DOT1X_msg_id_to_str(rx_msg->hdr.msg_id), rx_msg->hdr.version, len);

    // Check if we support this version of the message. If not, print a warning and return.
    if (rx_msg->hdr.version != DOT1X_MSG_VERSION) {
        T_W("%u:<any>: Unsupported version of the message (%u)", isid, rx_msg->hdr.version);
        return TRUE;
    }

    switch (rx_msg->hdr.msg_id) {
    case DOT1X_MSG_ID_MST_TO_SLV_PORT_STATE: {
        // Set port state.
        dot1x_msg_port_state_t *msg  = &rx_msg->u.port_state;
        if (port_no_is_stack(msg->api_port) || msg->api_port >= port_isid_port_count(VTSS_ISID_LOCAL)) {
            T_E("VTSS_ISID_LOCAL:%d: Invalid port number", iport2uport(msg->api_port));
            break;
        }
        if ((rc = vtss_auth_port_state_set(NULL, msg->api_port, msg->authorized ? VTSS_AUTH_STATE_BOTH : VTSS_AUTH_STATE_NONE)) != VTSS_RC_OK) {
            T_E("VTSS_ISID_LOCAL:%d: API err: %s", iport2uport(msg->api_port), error_txt(rc));
        }
        break;
    }

    case DOT1X_MSG_ID_MST_TO_SLV_SWITCH_STATE: {
        // Set all port states.
        dot1x_msg_switch_state_t *msg = &rx_msg->u.switch_state;
        port_iter_t              pit;

        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            T_D("VTSS_ISID_LOCAL:%d: Changing port state to %s", pit.uport, msg->authorized[pit.iport] ? "Auth" : "Unauth");
            if ((rc = vtss_auth_port_state_set(NULL, pit.iport, msg->authorized[pit.iport] ? VTSS_AUTH_STATE_BOTH : VTSS_AUTH_STATE_NONE)) != VTSS_RC_OK) {
                T_E("VTSS_ISID_LOCAL:%d: API err: %s", pit.uport, error_txt(rc));
            }
        }
        break;
    }

    case DOT1X_MSG_ID_MST_TO_SLV_EAPOL: {
        // Transmit an EAPOL frame (BPDU) on a front port.
        dot1x_msg_eapol_t *msg = &rx_msg->u.eapol;
        uchar             *frm;
        size_t            tx_len;
        packet_tx_props_t tx_props;

        // Allocate a buffer for the frame.
        tx_len = MAX(VTSS_FDMA_MIN_FRAME_SIZE_BYTES - FCS_SIZE_BYTES, msg->len);
        frm = packet_tx_alloc(tx_len);
        if (frm == NULL) {
            T_W("%u:%d: Unable to allocate %zu bytes", isid, iport2uport(msg->api_port), tx_len);
            break;
        }

        if (tx_len > msg->len) {
            // Make sure unused bytes of the frame are zeroed out.
            memset(frm + msg->len, 0, tx_len - msg->len);
        }
        memcpy(frm, msg->frm, msg->len);
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid     = VTSS_MODULE_ID_DOT1X;
        tx_props.packet_info.frm[0]    = frm;
        tx_props.packet_info.len[0]    = msg->len;
        tx_props.tx_info.dst_port_mask = VTSS_BIT64(msg->api_port);
        (void)packet_tx(&tx_props);
        break;
    }

    case DOT1X_MSG_ID_SLV_TO_MST_EAPOL: {
        // Received a BPDU on some front port in the stack.
        // We need to be master in order to handle it.
        dot1x_msg_eapol_t *msg = &rx_msg->u.eapol;

        if (!msg_switch_is_master()) {
            break;
        }
        T_N("%u:%d: Received BPDU, len=%zu, vid=%d", isid, iport2uport(msg->api_port), msg->len, msg->vid);
        if (!VTSS_ISID_LEGAL(isid)) {
            T_W("%u:%d: Invalid ISID. Ignoring", isid, iport2uport(msg->api_port));
            break;
        }

        DOT1X_CRIT_ENTER();
        DOT1X_handle_bpdu_reception(isid, msg->api_port, msg->frm, msg->vid, msg->len);
        DOT1X_CRIT_EXIT();
        break;
    }

    default:
        T_D("%u:<any>: Unknown message ID: %d", isid, rx_msg->hdr.msg_id);
        break;
    }
    return TRUE;
}

/******************************************************************************/
// DOT1X_cfg_flash_write()
/******************************************************************************/
static void DOT1X_cfg_flash_write(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    dot1x_flash_cfg_t *flash_cfg;

    DOT1X_CRIT_ASSERT_LOCKED();

    if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_DOT1X, NULL)) == NULL) {
        T_W("Failed to open flash configuration");
    } else {
        flash_cfg->cfg = DOT1X_stack_cfg;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_DOT1X);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
// DOT1X_rx_bpdu()
// 802.1X BPDU packet reception on local switch.
// Send it to the current master.
/******************************************************************************/
static BOOL DOT1X_rx_bpdu(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    BOOL rc = FALSE; // Allow other subscribers to receive the packet

    T_N("<slv>:%d: Rx'd BPDU from %s", iport2uport(rx_info->port_no), misc_mac2str(&frm[6]));
    // If the frame was received on a GLAG, ignore it.
    if (rx_info->glag_no != VTSS_GLAG_NO_NONE) {
        T_D("<slv>:%d: Received BPDU on GLAG (%d). Ignoring it", iport2uport(rx_info->port_no), rx_info->glag_no);
        return rc;
    }

    // If the frame was not received on a front port, ignore it.
    // In case someone someday makes VTSS_PORT_NO_START count from something different than
    // zero, we need to make the "rx_info->port_no < VTSS_PORT_NO_START" comparison, so suppress the lint errors:
    //   Warning -- Relational operator '<' always evaluates to 'false')
    //   Warning -- non-negative quantity is never less than zero)
    /*lint -e{685, 568} */
    if (rx_info->port_no < VTSS_PORT_NO_START || PORT_NO_IS_STACK(rx_info->port_no)) {
        T_D("<slv>:%d: Received BPDU on invalid port. Ignoring it", iport2uport(rx_info->port_no));
        return rc;
    }

    // Check if we are able to transport it.
    if (rx_info->length > NAS_IEEE8021X_MAX_EAPOL_PACKET_SIZE) {
        T_E("<slv>:%d: Received BPDU with length (%u) larger than supported (%d). Dropping it", iport2uport(rx_info->port_no), rx_info->length, NAS_IEEE8021X_MAX_EAPOL_PACKET_SIZE);
        return rc;
    }

    // Transmit the frame to the master
    DOT1X_msg_tx_eapol(0, rx_info->port_no, DOT1X_MSG_ID_SLV_TO_MST_EAPOL, rx_info->tag.vid, frm, rx_info->length);
    return rc;
}

/******************************************************************************/
// DOT1X_thread()
/******************************************************************************/
static void DOT1X_thread(cyg_addrword_t data)
{
    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {

                // We should timeout every one second (1000 ms)
                VTSS_OS_MSLEEP(1000);
                if (!msg_switch_is_master()) {
                    break;
                }

                // Timeout.
                DOT1X_CRIT_ENTER();
                if (DOT1X_stack_cfg.glbl_cfg.enabled) {
                    nas_1sec_timer_tick();
                }
                DOT1X_CRIT_EXIT();
            }
        }

        // No longer master. Time to bail out.
        // No reason for using CPU ressources when we're a slave
        T_I("Suspending 802.1X thread");
        cyg_thread_suspend(DOT1X_thread_handle);
        T_I("Resumed 802.1X thread");
    } // while (1)
}

/******************************************************************************/
// DOT1X_msg_rx_init()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_msg_rx_init(void)
{
    msg_rx_filter_t filter;
    vtss_rc         rc;

    memset(&filter, 0, sizeof(filter));
    filter.cb = DOT1X_msg_rx;
    filter.modid = VTSS_MODULE_ID_DOT1X;
    if ((rc = msg_rx_filter_register(&filter)) != VTSS_RC_OK) {
        T_E("Unable to register for messages (%s)", error_txt(rc));
    }
}

/******************************************************************************/
// DOT1X_bpdu_rx_init()
/******************************************************************************/
static DOT1X_INLINE void DOT1X_bpdu_rx_init(void)
{
    packet_rx_filter_t rx_filter;
    void               *rx_filter_id;
    vtss_rc            rc;
    mac_addr_t         bpdu_mac = NAS_IEEE8021X_MAC_ADDR;

    // BPDUs are already being captured (initialized from
    // ../port.c::port_api_init() by call to vtss_init()).
    // We need to register for 802.1X BPDUs from the packet module
    // so that they get to us as they arrive.
    memset(&rx_filter, 0, sizeof(rx_filter));
    memcpy(rx_filter.dmac, bpdu_mac, sizeof(rx_filter.dmac));
    rx_filter.modid = VTSS_MODULE_ID_DOT1X;
    rx_filter.match = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
    rx_filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
    rx_filter.etype = NAS_IEEE8021X_ETH_TYPE;
    rx_filter.cb    = DOT1X_rx_bpdu;
    if ((rc = packet_rx_filter_register(&rx_filter, &rx_filter_id)) != VTSS_RC_OK) {
        T_E("Unable to register for BPDUs (%s)", error_txt(rc));
    }
}

/******************************************************************************/
// DOT1X_isid_port_check()
// Returns VTSS_RC_OK if we're master, and isid and api_port are legal values, or
// if we're not master but VTSS_ISID_LOCAL is allowed and api_port is legal.
/******************************************************************************/
static vtss_rc DOT1X_isid_port_check(vtss_isid_t isid, vtss_port_no_t api_port, BOOL allow_local, BOOL check_port)
{
    if ((isid != VTSS_ISID_LOCAL && !VTSS_ISID_LEGAL(isid)) || (isid == VTSS_ISID_LOCAL && !allow_local)) {
        return DOT1X_ERROR_ISID;
    }
    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        return DOT1X_ERROR_MUST_BE_MASTER;
    }
    if (check_port && (api_port >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, api_port))) {
        return DOT1X_ERROR_PORT;
    }
    return VTSS_RC_OK;
}

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675
/****************************************************************************/
// DOT1X_backend_check_qos_rfc4675()
/****************************************************************************/
static DOT1X_INLINE vtss_prio_t DOT1X_backend_check_qos_rfc4675(u8 radius_handle)
{
#define USER_PRIO_TABLE_LEN 8
    u16         len = USER_PRIO_TABLE_LEN;
    u8          attr[USER_PRIO_TABLE_LEN];
    int         i;
    vtss_prio_t result = QOS_PORT_PRIO_UNDEF;

    if (vtss_radius_tlv_get(radius_handle, VTSS_RADIUS_ATTRIBUTE_USER_PRIORITY_TABLE, &len, attr) == VTSS_RC_OK) {
        // Attribute is present. Check to see if its valid.

        // Exactly USER_PRIO_TABLE_LEN bytes in attribute
        if (len != USER_PRIO_TABLE_LEN) {
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned QoS: Invalid RADIUS attribute length (got %u, expected %d)", len, USER_PRIO_TABLE_LEN);
#endif
            T_D("QoS(RFC4675): Invalid RADIUS attribute length (got %u, expected %d)", len, USER_PRIO_TABLE_LEN);
            return result;
        }

        // All USER_PRIO_TABLE_LEN octets must be the same
        for (i = 1; i < USER_PRIO_TABLE_LEN; i++) {
            if (attr[i - 1] != attr[i]) {
#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("RADIUS-assigned QoS: The %d octets are not the same", USER_PRIO_TABLE_LEN);
#endif
                T_D("QoS(RFC4675): The %d octets are not the same", USER_PRIO_TABLE_LEN);
                return result;
            }
        }

        // Must be in range [0; VTSS_PRIOS[.
        if (attr[0] < '0' || attr[0] >= '0' + VTSS_PRIOS) {
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned QoS: The QoS class is out of range (act: %d, allowed: [0; %d])", attr[0] - '0', VTSS_PRIOS - 1);
#endif
            T_D("QoS(RFC4675): The QoS class is out of range (act: 0x%x, allowed: [0x30; 0x%x])", attr[0], '0' + VTSS_PRIOS - 1);
            return result;
        }

        result = attr[0] - '0';
        T_D("QoS(RFC4675): Setting QoS class to %u", result);
    }
#undef USER_PRIO_TABLE_LEN

    return result;
}
#endif /* VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675 */

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN)
/****************************************************************************/
// DOT1X_isdigits()
/****************************************************************************/
static DOT1X_INLINE BOOL DOT1X_isdigits(u8 *ptr, int len)
{
    while (len-- > 0) {
        if (!isdigit(*(ptr++))) {
            return FALSE;
        }
    }
    return TRUE;
}
#endif

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN)
/****************************************************************************/
// DOT1X_strtoul()
// It is assumed that the string only contains ASCII digits in range '0'-'9'.
/****************************************************************************/
static DOT1X_INLINE BOOL DOT1X_strtoul(u8 *ptr, int len, u32 max, u32 *result)
{
    *result = 0;

    // Skip leading zeros
    while (len > 0) {
        // Skip leading zeros
        if (*ptr == '0') {
            len--;
            ptr++;
        } else {
            break;
        }
    }

    // For each iteration check if we'we exceeded max,
    // so that we don't run into wrap-around problems
    // if the number represented in the string is too
    // long.
    while (len-- > 0) {
        u8 ch = *(ptr++);
        *result *= 10;
        *result += ch - '0';
        if (*result > max) {
            return FALSE;
        }
    }
    return TRUE;
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM
/****************************************************************************/
// DOT1X_backend_check_qos_custom()
/****************************************************************************/
static DOT1X_INLINE vtss_prio_t DOT1X_backend_check_qos_custom(u8 radius_handle)
{
    // The attribute values come from nas_qos_custom_api.h, which can be overridden by the customer.
    vtss_radius_attributes_e avp_type;
    u8                       avp_len;
    u8                       *avp_ptr;
    u8                       *avp_start;
    BOOL                     start_over = TRUE;
    size_t                   prefix_len = strlen(NAS_QOS_CUSTOM_VSA_PREFIX);
    u32                      val;
    vtss_prio_t              result = QOS_PORT_PRIO_UNDEF;
    BOOL                     one_found = FALSE;

    // We have to iterate over the TLVs, since the same custom type may be present
    // more than once among the TLVs (AVP pairs) in the RADIUS frame.
    while (vtss_radius_tlv_iterate(radius_handle, &avp_type, &avp_len, (const u8 **)&avp_start, start_over) == VTSS_RC_OK) {
        // Next time, get the next TLV
        start_over = FALSE;

        // Use avp_ptr until we reach the decoding of the QoS Class.
        avp_ptr = avp_start;

        // Check if it's the one we're looking for.
        if (avp_type != NAS_QOS_CUSTOM_AVP_TYPE) {
            // Nope. Go on with the next attribute.
            continue;
        }

        // avp_len contains the number of octets in avp_ptr. This must be greater than
        // AVP-VendorID + VSA-Type + VSA-Length + X + Y = 4 + 1 + 1 + X + Y.
        if (avp_len <= 6 + prefix_len) {
            // Too short to be ours
            continue;
        }

        // Check Vendor ID. MSByte must be zero.
        if (avp_ptr[0] != 0                                         ||
            avp_ptr[1] != ((NAS_QOS_CUSTOM_VENDOR_ID >> 16) & 0xFF) ||
            avp_ptr[2] != ((NAS_QOS_CUSTOM_VENDOR_ID >>  8) & 0xFF) ||
            avp_ptr[3] != ((NAS_QOS_CUSTOM_VENDOR_ID >>  0) & 0xFF)) {
            // Doesn't match vendor ID. Next.
            continue;
        }

        // Next field
        avp_ptr += 4;

        // Check VSA-Type
        if (avp_ptr[0] != NAS_QOS_CUSTOM_VSA_TYPE) {
            // Not the right type.
            continue;
        }

        // Next field
        avp_ptr++;

        // Check the VSA-Length.
        // The VSA-Length includes the VSA-Type, itself, the prefix and the value.
        // The @avp_len includes the same lengths, but the vtss_radius API has
        // already removed 2 bytes for the AVP type and AVP length, so we expect
        // the VSA-length to be 4 bytes (corresponding the the Vendor ID) longer
        // than the VSA length.
        if (avp_ptr[0] != avp_len - 4) {
            // Doesn't match the way that we lay out vendor specific attributes.
            continue;
        }

        // Next field
        avp_ptr++;

        // Check the prefix string
        if (strncmp((char *)avp_ptr, NAS_QOS_CUSTOM_VSA_PREFIX, prefix_len) != 0) {
            // Doesn't match the prefix string. Next.
            continue;
        }

        // Proceed to the first char of the actual vendor-specific attribute value.
        avp_ptr += prefix_len;

        // This is the number of chars left for the actual QoS class.
        avp_len -= (avp_ptr - avp_start);

#if VTSS_PRIOS == 4
        // The QoS class can be either of "low", "normal", "medium", and "high".
        if (avp_len == 3 && (strncasecmp((char *)avp_ptr, "low", 3)) == 0) {
            result    = 0;
            one_found = TRUE;
            continue;
        } else if (avp_len == 6 && (strncasecmp((char *)avp_ptr, "normal", 6)) == 0) {
            result    = 1;
            one_found = TRUE;
            continue;
        } else if (avp_len == 6 && (strncasecmp((char *)avp_ptr, "medium", 6)) == 0) {
            result    = 2;
            one_found = TRUE;
            continue;
        } else if (avp_len == 4 && (strncasecmp((char *)avp_ptr, "high", 4)) == 0) {
            result    = 3;
            one_found = TRUE;
            continue;
        }
#endif

        // The Qos Class must be a decimal string between 0 and VTSS_PRIOS - 1.
        // Since the buffer isn't terminated, we iterate until there are no more chars.
        // First check if it contains only valid decimal chars.
        if (!DOT1X_isdigits(avp_ptr, avp_len)) {
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned QoS: Invalid character found in decimal ASCII string");
#endif
            T_D("QoS(Custom): Invalid char found in decimal ASCII string");
            return QOS_PORT_PRIO_UNDEF;
        }

        // Then get the value.
        if (!DOT1X_strtoul(avp_ptr, avp_len, VTSS_PRIOS - 1, &val)) {
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned QoS: The QoS class is out of range ([0; %d])", VTSS_PRIOS - 1);
#endif
            T_D("QoS(Custom): The QoS class is out of range ([0; %d])", VTSS_PRIOS - 1);
            return QOS_PORT_PRIO_UNDEF;
        }

        // Success
        if (one_found) {
            // There shall only be one.
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned QoS: Two or more QoS class specifications found");
#endif
            T_D("QoS(Custom): Two or more QoS class specifications found");
            return QOS_PORT_PRIO_UNDEF;
        } else {
            result = val;
            one_found = TRUE;
        }
    }

    if (result != QOS_PORT_PRIO_UNDEF) {
        T_D("QoS(Custom): Setting QoS class to %lu", result);
    }

    return result;
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
/****************************************************************************/
// DOT1X_backend_check_qos()
/****************************************************************************/
static DOT1X_INLINE void DOT1X_backend_check_qos(nas_sm_t *sm, u8 radius_handle)
{
    vtss_prio_t     qos_class = QOS_PORT_PRIO_UNDEF;
    nas_port_info_t *port_info = nas_get_port_info(sm);
    vtss_isid_t     isid;
    vtss_port_no_t  api_port;

    DOT1X_CRIT_ASSERT_LOCKED();

    if (!NAS_PORT_CONTROL_IS_SINGLE_CLIENT(port_info->port_control)) {
        return; // Don't care when not in Port-based or Single 802.1X
    }

    isid     = DOT1X_NAS_PORT_2_ISID(port_info->port_no);
    api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(port_info->port_no);

    if (!DOT1X_stack_cfg.glbl_cfg.qos_backend_assignment_enabled || !DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].qos_backend_assignment_enabled) {
        return; // Nothing to do.
    }

    // One of three methods supported:
    //   a) RFC4675 (VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675
    //   b) Vitesse-specific (VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM)
    //   c) Customer-specific (VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM)
    // The difference between b) and c) is only the contents of nas_qos_custom_api.h.
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675)
    qos_class = DOT1X_backend_check_qos_rfc4675(radius_handle);
#elif defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM)
    qos_class = DOT1X_backend_check_qos_custom(radius_handle);
#else
#error "At least one of the methods RFC4675 or Custom must be selected"
#endif

    DOT1X_qos_set(port_info->port_no, qos_class);
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
/****************************************************************************/
// DOT1X_backend_check_vlan_rfc2868_rfc3580()
/****************************************************************************/
// I think there's a bug in FlexeLint. It states that:
//   error 438: (Warning -- Last value assigned to variable 'start_over' (defined at line 1731) not used)
//   error 830: (Info -- Location cited in prior message)
// But I'm pretty sure that all assignments to start_over are used.
/*lint -e{438} */
static vtss_vid_t DOT1X_backend_check_vlan_rfc2868_rfc3580(u8 radius_handle)
{
    BOOL                     tunnel_type_seen                  = FALSE;
    BOOL                     matching_tunnel_medium_type_seen  = FALSE;
    BOOL                     start_over                        = TRUE;
    u8                       *tlv_tunnel_type_ptr              = NULL;
    u8                       tag                               = 0;
    vtss_vid_t               result                            = VTSS_VID_NULL;
    vtss_radius_attributes_e tlv_type;
    u8                       tlv_len;
    u8                       *tlv_ptr;
    u32                      val;

    // Gotta traverse the RADIUS attributes a couple of times because we need
    // info from the Tunnel-Type and Tunnel-Medium-Type attributes before being
    // able to assess the Private-Group-ID
    while (vtss_radius_tlv_iterate(radius_handle, &tlv_type, &tlv_len, (const u8 **)&tlv_ptr, start_over) == VTSS_RC_OK) {
        // Next time, get the next TLV
        start_over = FALSE;

        // Check to see if we're trying to continue from previous iteration
        // in case we haven't found a matching Tunnel-Medium-Type attribute
        if (tlv_tunnel_type_ptr != NULL && tlv_ptr <= tlv_tunnel_type_ptr) {
            continue;
        }

        // Look for the Tunnel-Type (64; RFC2868)
        if (tlv_type != VTSS_RADIUS_ATTRIBUTE_TUNNEL_TYPE) {
            continue;
        }

        // Check length. Two bytes are already subtracted by the RADIUS module (type and length)
        if (tlv_len != 4) {
            continue;
        }

        // Get the Tag. This must be a number in range [0x00; 0x1F].
        if ((tag = tlv_ptr[0]) > 0x1F) {
            // Invalid.
            continue;
        }

        // Check Tunnel Type. This must be "VLAN" (13 = 0x0D) as defined in RFC3580
        if (tlv_ptr[1] != 0x00 ||
            tlv_ptr[2] != 0x00 ||
            tlv_ptr[3] != 0x0D) {
            continue;
        }

        tunnel_type_seen = TRUE;

        // Keep a snap-shot of this attribute's TLV ptr for use in case we couldn't
        // find a matching Tunnel-Medium-Type.
        tlv_tunnel_type_ptr = tlv_ptr;

        // Now search for the corresponding Tunnel-Media-Type
        matching_tunnel_medium_type_seen = FALSE;
        start_over                       = TRUE;
        while (vtss_radius_tlv_iterate(radius_handle, &tlv_type, &tlv_len, (const u8 **)&tlv_ptr, start_over) == VTSS_RC_OK) {
            // Next time, get the next TLV
            start_over = FALSE;

            // Look for the Tunnel-Medium-Type (65; RFC2868)
            if (tlv_type != VTSS_RADIUS_ATTRIBUTE_TUNNEL_MEDIUM_TYPE) {
                continue;
            }

            // Check length. Two bytes are already subtracted by the RADIUS module (type and length)
            if (tlv_len != 4) {
                continue;
            }

            // Get the Tag. This must be the same as for the Tunnel-Type. Otherwise
            // we haven't found the matching attribute.
            if (tag != tlv_ptr[0]) {
                continue;
            }

            // Check Medium Type. This must be set to 0x00 0x00 0x06 for IEEE-802
            if (tlv_ptr[1] != 0x00 ||
                tlv_ptr[2] != 0x00 ||
                tlv_ptr[3] != 0x06) {
                continue;
            }

            // Got it.
            matching_tunnel_medium_type_seen = TRUE;
            break;
        }

        if (!matching_tunnel_medium_type_seen) {
            // We gotta find the next Tunnel-Type and see if that one
            // has a matching Tunnel-Medium-Type.
            // The tlv_tunnel_type_ptr is now non-NULL, which means
            // that the outer TLV iterator skips all the TLVs up to
            // and including the Tunnel-Type that didn't have a
            // Tunnel-Medium-Type match.
            tunnel_type_seen = FALSE;
            start_over       = TRUE;
        } else {
            break;
        }
    }

    if (tunnel_type_seen == FALSE || matching_tunnel_medium_type_seen == FALSE) {
        // No matching Tunnel-Type and Tunnel-Type-Medium attributes found.
        return 0;
    }

    // Now that we have the Tag, go and get the VLAN ID from the Tunnel-Private-Group-ID
    start_over = TRUE;
    while (vtss_radius_tlv_iterate(radius_handle, &tlv_type, &tlv_len, (const u8 **)&tlv_ptr, start_over) == VTSS_RC_OK) {
        // Next time, get the next TLV
        start_over = FALSE;

        // Look for the Tunnel-Private-Group-ID (81; RFC2868)
        if (tlv_type != VTSS_RADIUS_ATTRIBUTE_TUNNEL_PRIVATE_GROUP_ID) {
            continue;
        }

        // Check length. Two bytes are already subtracted by the RADIUS module (type and length)
        // The length must accommodate at least one byte.
        if (tlv_len == 0) {
            continue;
        }

        // Check the tag.
        // If Tag from Tunnel-Type and Tunnel-Medium-Type is non-zero, then the
        // tag of the Tunnel-Private-Group-ID must be the same.
        // If Tag from Tunnel-Type and Tunnel-Medium-Type is zero, then the
        // tag of the Tunnel-Private-Group-ID can be anything. If it's greater
        // than 0x1F, then it's considered the first byte of the VLAN ID.
        if (tag != 0x00) {
            if (tlv_ptr[0] != tag) {
                // When Tag is non-zero, the Tunnel-Private-Group-ID tag must be the same.
                continue;
            } else {
                // Skip the tag
                tlv_ptr++;
                tlv_len--;
            }
        } else if (tlv_ptr[0] <= 0x1F) {
            tlv_ptr++;
            tlv_len--;
        }

        // If there are no bytes left after eating a possible tag, try the next TLV.
        if (tlv_len == 0) {
            continue;
        }

        // Now it's time to interpret the VLAN ID.
        // Check if it's all digits.
        if (!DOT1X_isdigits(tlv_ptr, tlv_len)) {
            // It's not.
            // If the switch supports VLAN naming, try to look it up.
#ifdef VTSS_SW_OPTION_VLAN_NAMING
            char       vlan_name[VLAN_NAME_MAX_LEN];
            vtss_vid_t vid;

            vlan_name[sizeof(vlan_name) - 1] = '\0';
            if (tlv_len > sizeof(vlan_name) - 1) {
#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("RADIUS-assigned VLAN: VLAN name (%s) too long. Supported length: %u characters", tlv_ptr, sizeof(vlan_name) - 1);
#endif
                T_D("RADIUS-assigned VLAN: VLAN name (%s) too long. Supported length: %u characters", tlv_ptr, sizeof(vlan_name) - 1);
            } else {
                strncpy(vlan_name, (char *)tlv_ptr, tlv_len);
                if (vlan_mgmt_name_to_vid(vlan_name, &vid) != VTSS_RC_OK) {
#ifdef VTSS_SW_OPTION_SYSLOG
                    S_W("RADIUS-assigned VLAN: VLAN name (%s) not found", vlan_name);
#endif
                    T_D("RADIUS-assigned VLAN: VLAN name (%s) not found", vlan_name);
                } else {
                    result = vid;
                }
            }
            break;
#else /* !defined(VTSS_SW_OPTION_VLAN_NAMING) */
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned VLAN: Invalid character found in decimal ASCII string");
#endif
            T_D("RADIUS-assigned VLAN: Invalid character found in decimal ASCII string");
            break;
#endif /* !defined(VTSS_SW_OPTION_VLAN_NAMING) */
        }

        // Parse decimal string
        // Then get the value.
        if (!DOT1X_strtoul(tlv_ptr, tlv_len, VTSS_VIDS - 1, &val) || val == 0) {
#ifdef VTSS_SW_OPTION_SYSLOG
            S_W("RADIUS-assigned VLAN: The VLAN ID is out of range ([1; %d])", VTSS_VIDS - 1);
#endif
            T_D("RADIUS-assigned VLAN: The VLAN ID is out of range ([1; %d])", VTSS_VIDS - 1);
            break;
        }

        result = val;
        break;
    }

    if (result != VTSS_VID_NULL) {
        T_D("RADIUS-assigned VLAN: Setting VLAN ID to %d", result);
    }

    return result;
}
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
/****************************************************************************/
// DOT1X_backend_check_vlan()
/****************************************************************************/
static void DOT1X_backend_check_vlan(nas_sm_t *sm, u8 radius_handle)
{
    nas_port_info_t *port_info = nas_get_port_info(sm);
    vtss_isid_t     isid;
    vtss_port_no_t  api_port;

    DOT1X_CRIT_ASSERT_LOCKED();

    if (!NAS_PORT_CONTROL_IS_SINGLE_CLIENT(port_info->port_control)) {
        return; // Don't care when not in Port-based or Single 802.1X
    }

    isid     = DOT1X_NAS_PORT_2_ISID(port_info->port_no);
    api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(port_info->port_no);

    if (!DOT1X_stack_cfg.glbl_cfg.vlan_backend_assignment_enabled || !DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].vlan_backend_assignment_enabled) {
        return; // Nothing to do.
    }

    // Defer setting the actual VID until the call to nas_os_set_authorized()
    nas_get_port_info(sm)->backend_assigned_vid = DOT1X_backend_check_vlan_rfc2868_rfc3580(radius_handle);
}
#endif

/****************************************************************************/
// DOT1X_radius_rx_callback()
/****************************************************************************/
static void DOT1X_radius_rx_callback(u8 handle, void *ctx, vtss_radius_access_codes_e code, vtss_radius_rx_callback_result_e res)
{
    nas_sm_t        *sm;
    nas_port_t      nas_port;
    u16             tlv_len;
    nas_eap_info_t  *eap_info;
    vtss_isid_t     isid;
    vtss_uport_no_t uport;

    // We use the nas_port as context, rather than the state machine itself, because the
    // state machine may have been used for other purposes once we get called back. Furthermore,
    // if we had used dynamic memory allocation for statemachines (we don't), then the ctx
    // may have been out-of-date (freed) once called back.
    // The combination of port number and RADIUS identifier should uniquely identify the SM.
    nas_port = (nas_port_t)ctx;
    isid     = DOT1X_NAS_PORT_2_ISID(nas_port);
    uport    = iport2uport(DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port));

    DOT1X_CRIT_ENTER();

    // Look up the state-machine based on <port, handle> tuple.
    sm = nas_get_sm_from_radius_handle(nas_port, (int)((unsigned int)handle));
    if (!sm) {
        // It has been removed, probably due to port down or configuration change. Even if
        // we have called vtss_radius_free(), it may be that we get called back because of an unavoidable
        // race-condition.
        T_I("%d:%d: Couldn't find matching SM for RADIUS handle=%u", isid, uport, handle);
        goto do_exit;
    }

    eap_info = nas_get_eap_info(sm);
    VTSS_ASSERT(eap_info);
    eap_info->radius_handle = -1;

    if (!DOT1X_stack_cfg.glbl_cfg.enabled) {
        goto do_exit;
    }

    if (res == VTSS_RADIUS_RX_CALLBACK_OK) {
        switch (code) {
        case VTSS_RADIUS_CODE_ACCESS_ACCEPT:
        case VTSS_RADIUS_CODE_ACCESS_REJECT:
        case VTSS_RADIUS_CODE_ACCESS_CHALLENGE: {
            // Get the TLVs.
            tlv_len = sizeof(eap_info->radius_state_attribute);
            if (vtss_radius_tlv_get(handle, VTSS_RADIUS_ATTRIBUTE_STATE, &tlv_len, eap_info->radius_state_attribute) != VTSS_RC_OK) {
                eap_info->radius_state_attribute_len = 0;
            } else {
                eap_info->radius_state_attribute_len = tlv_len;
            }
            tlv_len = sizeof(eap_info->last_frame);
            if (vtss_radius_tlv_get(handle, VTSS_RADIUS_ATTRIBUTE_EAP_MESSAGE, &tlv_len, eap_info->last_frame) != VTSS_RC_OK) {
                eap_info->last_frame_len = 0;
            } else {
                eap_info->last_frame_len = tlv_len;
            }

            // Register this frame type
            eap_info->last_frame_type = FRAME_TYPE_EAPOL;

            if (code == VTSS_RADIUS_CODE_ACCESS_ACCEPT) {
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                DOT1X_backend_check_qos(sm, handle);
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                DOT1X_backend_check_vlan(sm, handle);
#endif
            }

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
            dot1x_acct_radius_rx(handle, eap_info);
#endif

            // All required info has now been saved in SM's eap_info, so it's up
            // to the SM to do the rest.
            nas_backend_frame_received(sm, (nas_backend_code_t)code);
            break;
        }
        default:
            T_E("%u:%d: Unknown RADIUS type received: %d", DOT1X_NAS_PORT_2_ISID(nas_port), iport2uport(DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port)), code);
            break;
        }
    } else {
        // Treat all error messages as a backend server timeout.
        T_W("%d:%d: RADIUS timeout occurred (code=%d)", isid, uport, res);
#ifdef VTSS_SW_OPTION_SYSLOG
#if VTSS_SWITCH_STACKABLE
        S_W("NAS (Switch %d Port %d): RADIUS timeout while authenticating user", topo_isid2usid(isid), uport);
#else
        S_W("NAS (Port %d): RADIUS timeout while authenticating user", uport);
#endif
#endif
        nas_backend_server_timeout_occurred(sm);
    }

do_exit:
    DOT1X_CRIT_EXIT();
}

/******************************************************************************/
// DOT1X_link_state_change_callback()
/******************************************************************************/
static void DOT1X_link_state_change_callback(vtss_isid_t isid, vtss_port_no_t api_port, port_info_t *info)
{
    dot1x_switch_state_t *switch_state;
    nas_port_t           nas_port;
    nas_port_control_t   cur_state;

    if (!msg_switch_exists(isid)) {
        // Note that if the switch doesn't exist at all, we
        // will also simply return, and not react on switch-deletes here. Switch-delete events
        // are therefore handled separately in then INIT_CMD_SWITCH_DEL section of dot1x_init().
        return;
    }

    if (port_isid_port_no_is_stack(isid, api_port)) {
        return; // We don't care about stack ports.
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);

    DOT1X_CRIT_ENTER();
    switch_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START];
    cur_state = nas_get_port_control(nas_port);

    if (info->link) {
        switch_state->port_state[api_port - VTSS_PORT_NO_START].flags |= DOT1X_PORT_STATE_FLAGS_LINK_UP;

        // Postpone the setting of the admin_state until the INIT_CMD_SWITCH_ADD has been called, if it's
        // not already called. This can happen if the port module comes before the dot1x module in the
        // array of modules being called back with switch-add/switch-delete events.
        if (switch_state->switch_exists) {
            // If the port is MAC-table based, the Port Security module already knows this and
            // may already have started adding MAC addresses on us. If so, the port may already be in link-up state,
            // since the on-mac-add function may have set it to that.
            nas_port_control_t admin_state = DOT1X_stack_cfg.glbl_cfg.enabled ? DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].admin_state : NAS_PORT_CONTROL_DISABLED;
            if (cur_state != admin_state) {
                DOT1X_tx_port_state(isid, api_port, DOT1X_stack_cfg.glbl_cfg.enabled, admin_state);
                DOT1X_set_port_control(nas_port, admin_state, NAS_STOP_REASON_PORT_MODE_CHANGED);
            }
        }
    } else {
        // Link down
        switch_state->port_state[api_port - VTSS_PORT_NO_START].flags &= ~DOT1X_PORT_STATE_FLAGS_LINK_UP;
        // If it's MAC-table based and have at least one attached SM, do nothing, but wait for the Port Security module to
        // delete the entries. Otherwise set the port control to NAS_PORT_CONTROL_DISABLED.
        if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(cur_state) == FALSE || nas_get_client_cnt(nas_port) == 0) {
            DOT1X_set_port_control(nas_port, NAS_PORT_CONTROL_DISABLED, NAS_STOP_REASON_PORT_LINK_DOWN);
        }
    }

    DOT1X_CRIT_EXIT();
}

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_on_mac_add_callback()
/******************************************************************************/
static psec_add_method_t DOT1X_on_mac_add_callback(vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_mac_t *vid_mac, u32 mac_cnt_before_callback, psec_add_action_t *action)
{
    dot1x_switch_state_t *switch_state;
    dot1x_port_state_t   *port_state;
    nas_port_control_t   admin_state, cur_state;
    nas_port_t           nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
    psec_add_method_t    result = PSEC_ADD_METHOD_KEEP_BLOCKED; // Whenever the MAC address is authenticated, the state will change. Do not age it until then.

    DOT1X_CRIT_ENTER();
    switch_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START];
    port_state   = &switch_state->port_state[api_port - VTSS_PORT_NO_START];
    admin_state  = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].admin_state;

    if (!NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state)) {
        T_E("%d:%d: Internal error", isid, iport2uport(api_port));
        goto do_exit;
    }

    if (NAS_PORT_CONTROL_IS_BPDU_BASED(admin_state)) {
        // We're currently in Single or Multi 802.1X mode (MAC-table-
        // and BPDU-based). This means that all MAC addresses added
        // to the MAC table come through BPDUs, and not from other
        // modules.
        // In the event that another PSEC user module is in the
        // PSEC_PORT_MODE_KEEP_BLOCKED mode, that other module
        // may add MAC addresses to the PSEC module, thus causing
        // the PSEC module to callback all other modules - among
        // those us. Since we're in charge of what is going to
        // be added to the MAC table (802.1X takes precedence),
        // we must add it as blocked with a timeout.
        // When the entry times out, we can't find a corresponding
        // state machine and will print a warning. No problem.
        // At the time of writing no such other module exists.
        result = PSEC_ADD_METHOD_BLOCK;
        goto do_exit;
    }

    if (switch_state->switch_exists == FALSE || !(port_state->flags & DOT1X_PORT_STATE_FLAGS_LINK_UP)) {
        // This may happen if we get called before the switch-add or link-up events have
        // propagated to us.
        T_I("%d:%d: Artificially setting switch and link up", isid, iport2uport(api_port));
        switch_state->switch_exists  = TRUE;
        port_state->flags           |= DOT1X_PORT_STATE_FLAGS_LINK_UP;
        cur_state = nas_get_port_control(nas_port);
        if (cur_state != admin_state) {
            DOT1X_tx_port_state(isid, api_port, DOT1X_stack_cfg.glbl_cfg.enabled, admin_state);
            DOT1X_set_port_control(nas_port, admin_state, NAS_STOP_REASON_PORT_MODE_CHANGED);
        }
    }

    if (!nas_alloc_sm(nas_port, vid_mac)) {
        // This is a very rare event, but it may happen if all the SMs
        // are in use, which may happen if - say - one port is in a multi-client
        // mode and another is in a single-client mode. The total number of
        // SMs correspond to the number of times this function can be called,
        // so if one of the SMs is taken by a port in single-client mode,
        // nas_alloc_sm() may fail here.
        // If that happens, block the MAC address with a hold-timeout.
        // This will give one more T_E() error when the MAC address
        // times out in the DOT1X_on_mac_del_callback() function, because
        // we can't find the corresponding state machine.
        T_E("%d:%d: Can't allocate SM", isid, iport2uport(api_port));
        result = PSEC_ADD_METHOD_BLOCK;
        goto do_exit;
    }

do_exit:
    DOT1X_CRIT_EXIT();
    return result;
}
#endif /* NAS_USES_PSEC */

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_psec_free_to_nas_stop_reason()
/******************************************************************************/
static DOT1X_INLINE nas_stop_reason_t DOT1X_psec_free_to_nas_stop_reason(psec_del_reason_t psec_free_reason)
{
    switch (psec_free_reason) {
    case PSEC_DEL_REASON_HW_ADD_FAILED:
        return NAS_STOP_REASON_MAC_TABLE_ERROR;
    case PSEC_DEL_REASON_SW_ADD_FAILED:
        return NAS_STOP_REASON_MAC_TABLE_ERROR;
    case PSEC_DEL_REASON_SWITCH_DOWN:
        return NAS_STOP_REASON_SWITCH_DOWN;
    case PSEC_DEL_REASON_PORT_LINK_DOWN:
        return NAS_STOP_REASON_PORT_LINK_DOWN;
    case PSEC_DEL_REASON_STATION_MOVED:
        return NAS_STOP_REASON_STATION_MOVED;
    case PSEC_DEL_REASON_AGED_OUT:
        return NAS_STOP_REASON_AGED_OUT;
    case PSEC_DEL_REASON_HOLD_TIME_EXPIRED:
        return NAS_STOP_REASON_HOLD_TIME_EXPIRED;
    case PSEC_DEL_REASON_USER_DELETED:
        return NAS_STOP_REASON_PORT_MODE_CHANGED;
    case PSEC_DEL_REASON_PORT_SHUT_DOWN:
        return NAS_STOP_REASON_PORT_SHUT_DOWN;
    case PSEC_DEL_REASON_NO_MORE_USERS:
        return NAS_STOP_REASON_PORT_MODE_CHANGED;
    default:
        T_E("Unknown PSEC delete reason (%d)", psec_free_reason);
        return NAS_STOP_REASON_UNKNOWN;
    }
}
#endif

#ifdef NAS_USES_PSEC
/******************************************************************************/
// DOT1X_on_mac_del_callback()
/******************************************************************************/
static void DOT1X_on_mac_del_callback(vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_mac_t *vid_mac, psec_del_reason_t reason, psec_add_method_t add_method)
{
    nas_sm_t           *sm;
    nas_port_control_t cur_state;
    nas_port_t         nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
    const char         *mac_str = misc_mac2str(vid_mac->mac.addr);
    vtss_uport_no_t    uport = iport2uport(api_port);

    DOT1X_CRIT_ENTER();

    cur_state = nas_get_port_control(nas_port);

    T_D("%d:%d: Unregistering MAC address (%s on %d) because: %s", isid, uport, mac_str, vid_mac->vid, psec_del_reason_to_str(reason));

    if (!NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(cur_state)) {
        T_E("%d:%d: Port is not in MAC-table-based mode", isid, uport);
        goto do_exit;
    }

    sm = nas_get_sm_from_vid_mac_port(vid_mac, nas_port);

    if (!sm) {
        T_W("%d:%d: Don't know anything about this MAC address (%s on %d)", isid, uport, mac_str, vid_mac->vid);
        goto do_exit;
    }

    nas_free_sm(sm, DOT1X_psec_free_to_nas_stop_reason(reason));

    // If this was the last and the port link is in link-down, then change the port mode to disabled.
    if (nas_get_client_cnt(nas_port) == 0 && !(DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP)) {
        DOT1X_set_port_control(nas_port, NAS_PORT_CONTROL_DISABLED, NAS_STOP_REASON_PORT_MODE_CHANGED /* doesn't matter */);
    }

do_exit:
    DOT1X_CRIT_EXIT();
}
#endif /* NAS_USES_PSEC */

/******************************************************************************/
// DOT1X_fill_statistics()
// Fill in everything, but the admin_status and status fields.
// The status field differs depending on whether sm is a top of sub SM,
// and the admin_state requires api_port and isid, since we must take
// that from what the user has configured and not what the NAS base lib
// thinks.
/******************************************************************************/
static void DOT1X_fill_statistics(nas_sm_t *sm, dot1x_statistics_t *statistics)
{
    nas_eapol_counters_t   *eapol_counters;
    nas_backend_counters_t *backend_counters;
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
    nas_port_info_t        *port_info = nas_get_port_info(sm);
#endif

    DOT1X_CRIT_ASSERT_LOCKED();

    // Get the EAPOL counters (if this mode supports it).
    if ((eapol_counters = nas_get_eapol_counters(sm)) != NULL) {
        statistics->eapol_counters = *eapol_counters;
    }

    // Get the backend counters (if this mode supports it).
    if ((backend_counters = nas_get_backend_counters(sm)) != NULL) {
        statistics->backend_counters = *backend_counters;
    }

    // Get the supplicant/client info (either last supplicant/client (if top-sm) or actual supplicant/client (if sub-sm)
    statistics->client_info = *nas_get_client_info(sm);

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    // The QoS class that this port is assigned to by the backend server.
    // QOS_PORT_PRIO_UNDEF if unassigned.
    statistics->qos_class = port_info->qos_class;
#endif

#ifdef NAS_USES_VLAN
    // The VLAN ID of this port (either backend-assigned or Guest VLAN)
    // 0 if unassigned.
    statistics->vlan_type = port_info->vlan_type;
    statistics->vid       = port_info->current_vid;
#endif
}

/****************************************************************************/
// DOT1X_auth_fail_reason_to_str()
/****************************************************************************/
static char *DOT1X_auth_fail_reason_to_str(nas_stop_reason_t reason)
{
    switch (reason) {
    case NAS_STOP_REASON_NONE:
        return "None";
    case NAS_STOP_REASON_UNKNOWN:
        return "Unknown";
    case NAS_STOP_REASON_INITIALIZING:
        return "(Re-)initializing";
    case NAS_STOP_REASON_AUTH_FAILURE:
        return "Backend server sent an Authentication Failure";
    case NAS_STOP_REASON_AUTH_NOT_CONFIGURED:
        return "Backend server not configured";
    case NAS_STOP_REASON_AUTH_TOO_MANY_ROUNDS:
        return "Too many authentication rounds";
    case NAS_STOP_REASON_AUTH_TIMEOUT:
        return "Backend server didn't reply";
    case NAS_STOP_REASON_EAPOL_START:
        return "Supplicant sent EAPOL start frame";
    case NAS_STOP_REASON_EAPOL_LOGOFF:
        return "Supplicant sent EAPOL logoff frame";
    case NAS_STOP_REASON_REAUTH_COUNT_EXCEEDED:
        return "Reauth count exceeded";
    case NAS_STOP_REASON_FORCED_UNAUTHORIZED:
        return "Port mode forced unauthorized";
    case NAS_STOP_REASON_MAC_TABLE_ERROR:
        return "Couldn't add MAC address to MAC Table";
    case NAS_STOP_REASON_SWITCH_DOWN:
        return "The switch went down";
    case NAS_STOP_REASON_PORT_LINK_DOWN:
        return "Port-link went down";
    case NAS_STOP_REASON_STATION_MOVED:
        return "The supplicant moved from one port to another";
    case NAS_STOP_REASON_AGED_OUT:
        return "The MAC address aged out";
    case NAS_STOP_REASON_HOLD_TIME_EXPIRED:
        return "The hold-time expired";
    case NAS_STOP_REASON_PORT_MODE_CHANGED:
        return "The port mode changed";
    case NAS_STOP_REASON_PORT_SHUT_DOWN:
        return "The port was shut down by the Port Security Limit Control Module";
    case NAS_STOP_REASON_SWITCH_REBOOT:
        return "Switch is about to reboot";
    default:
        return "Unknown stop reason";
    }
}

/******************************************************************************/
//
// IMPLEMENTATION OF NAS CORE LIBRARY OS-DEPENDENT FUNCTIONS.
// THE API HEADER FILE FOR THIS IS LOCATED IN sw_nas/nas_os.h (core/nas_os.h)
//
/******************************************************************************/

/******************************************************************************/
// nas_os_encode_auth_unauth()
// The pendant of dot1x_mgmt_decode_auth_unauth()
/******************************************************************************/
nas_port_status_t nas_os_encode_auth_unauth(u32 auth_cnt, u32 unauth_cnt)
{
    return (nas_port_status_t)(((unauth_cnt + 1) << 16) | (auth_cnt + 1));
}

/****************************************************************************/
// nas_os_freeing_sm()
/****************************************************************************/
void nas_os_freeing_sm(nas_sm_t *sm)
{
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN)
    nas_port_info_t *port_info = nas_get_port_info(sm);
    if (NAS_PORT_CONTROL_IS_SINGLE_CLIENT(port_info->port_control) && port_info->cur_client_cnt == 1) {
        // The last client on this single-client port.

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        DOT1X_qos_set(port_info->port_no, QOS_PORT_PRIO_UNDEF);
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        DOT1X_vlan_set(port_info->port_no, 0, NAS_VLAN_TYPE_NONE);
#endif
    }
#endif
}

/****************************************************************************/
// nas_os_ieee8021x_send_eapol_frame()
// Send a BPDU on external port number
/****************************************************************************/
void nas_os_ieee8021x_send_eapol_frame(nas_port_t nas_port, u8 *frame, size_t len)
{
    DOT1X_msg_tx_eapol(DOT1X_NAS_PORT_2_ISID(nas_port), DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port), DOT1X_MSG_ID_MST_TO_SLV_EAPOL, 1 /* Doesn't matter (unused) */, frame, len);
}

/****************************************************************************/
// nas_os_get_port_mac()
/****************************************************************************/
void nas_os_get_port_mac(nas_port_t nas_port, u8 *frame)
{
    // Convert from NAS core library port number to Layer 2 port number before
    // calling vtss_os_get_portmac(), which expects an L2 port number.
    vtss_isid_t    isid     = DOT1X_NAS_PORT_2_ISID(nas_port);
    vtss_port_no_t api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);

    DOT1X_CRIT_ASSERT_LOCKED();

    vtss_os_get_portmac(L2PORT2PORT(isid, api_port), (vtss_common_macaddr_t *)frame);
}

/****************************************************************************/
// nas_os_set_authorized()
// if @sm is port-based, then this function causes a message to be sent to
// the relevant switch where its port state is changed.
// If @sm is mac-based, then this function causes a MAC address to be added
// or removed from the MAC table using the MAC module.
// If @chgd, then the previous call to this function had @authorized set to its
// opposite value.
/****************************************************************************/
void nas_os_set_authorized(struct nas_sm *sm, BOOL authorized, BOOL chgd)
{
    nas_port_t         nas_port     = nas_get_port_info(sm)->port_no;
    vtss_isid_t        isid         = DOT1X_NAS_PORT_2_ISID(nas_port);
    vtss_port_no_t     api_port     = DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port);
    vtss_uport_no_t    uport        = iport2uport(api_port);
    nas_eap_info_t     *eap_info    = nas_get_eap_info(sm);
    nas_client_info_t  *client_info = nas_get_client_info(sm);
    nas_port_info_t    *port_info   = nas_get_port_info(sm);
#ifdef NAS_USES_PSEC
    vtss_rc            rc           = VTSS_RC_OK;
    BOOL               psec_chg     = FALSE;
    psec_add_method_t  chg_method   = PSEC_ADD_METHOD_BLOCK; // Initialize it to keep the compiler happy
#endif
    nas_port_control_t admin_state;
    i8                 mac_or_port_str[30];

    // The critical section must be taken by now, since this must be called back from the base lib as a response
    // to this module calling the base lib.
    DOT1X_CRIT_ASSERT_LOCKED();

    admin_state = port_info->port_control;

    // Update the time of (successful/unsuccessful) authentication.
    client_info->rel_auth_time_secs = nas_os_get_uptime_secs();

#ifdef NAS_USES_PSEC
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state)) {
        // Show MAC address in subsequent T_x output.
        (void)snprintf(mac_or_port_str, sizeof(mac_or_port_str) - 1, "%s:%d", client_info->mac_addr_str, client_info->vid_mac.vid);
    } else
#endif
    {
        // Show "port" in subsequent T_x output.
        strcpy(mac_or_port_str, "port");
    }

    T_I("%d:%d: Setting %s to %s (reason = %s)", isid, uport, mac_or_port_str, authorized ? "Auth" : "Unauth", authorized ? "Success" : DOT1X_auth_fail_reason_to_str(eap_info->stop_reason));

    if (!DOT1X_stack_cfg.glbl_cfg.enabled) {
        T_E("%d:%d: 802.1X not enabled", isid, uport);
    }

    if (authorized && admin_state == NAS_PORT_CONTROL_FORCE_UNAUTHORIZED) {
        T_E("%d:%d: Base lib told to authorize, but the admin state indicates FU", isid, uport);
    }
    if (!authorized && admin_state == NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
        T_E("%d:%d: Base lib told to unauthorize, but the admin state indicates FA", isid, uport);
    }
    if (authorized && !(DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP)) {
        T_E("%d:%d: Base lib told us authorize %s, but the cached link state says Down", isid, uport, mac_or_port_str);
    }

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    if (NAS_PORT_CONTROL_IS_SINGLE_CLIENT(admin_state) && !authorized) {
        DOT1X_qos_set(nas_port, QOS_PORT_PRIO_UNDEF);
    }
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    if (port_info->vlan_type == NAS_VLAN_TYPE_GUEST_VLAN) {
        // Nothing more to do here.
        return;
    }
    if (eap_info->stop_reason == NAS_STOP_REASON_REAUTH_COUNT_EXCEEDED && NAS_PORT_CONTROL_IS_BPDU_BASED(admin_state)) {
        if (DOT1X_guest_vlan_enter_check(isid, api_port, port_info)) {
            // Just entered guest VLAN. Nothing more to do.
            return;
        }
    }
#endif

#ifdef NAS_USES_PSEC
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(admin_state)) {
        // This is a Single- or Multi- 802.1X SM.
        if (client_info->vid_mac.vid == 0) {
            // Indicates that this has not yet been assigned. Nothing to do, since we're waiting for the first client to attach.
            T_D("%d:%d: Doing nothing. Awaiting a client to attach", isid, uport);
        } else if (!authorized && (eap_info->stop_reason == NAS_STOP_REASON_INITIALIZING || eap_info->stop_reason == NAS_STOP_REASON_EAPOL_START)) {
            // Client is attached and either the network administrator is forcing a "reauthentication now" or the client has sent an EAPOL Start frame.
            // Either way, we need to keep the MAC address blocked, so that it doesn't age out while authenticating.
            psec_chg   = TRUE;
            chg_method = PSEC_ADD_METHOD_KEEP_BLOCKED;
        } else if (!authorized && eap_info->stop_reason == NAS_STOP_REASON_EAPOL_LOGOFF) {
            // Client sent an EAPOL Logoff frame. Gotta remove the SM and the entry from the MAC table.
            // The VLAN will be reverted once its actually deleted.
            rc = psec_mgmt_mac_del(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac);
            eap_info->delete_me = TRUE;
            T_D("%d:%d(%s): Deleting MAC and asking baselib to delete SM (rc=%s)", isid, uport, mac_or_port_str, error_txt(rc));
        } else {
            // If any other reason, we add it with either forwarding or blocked with hold timeout.
            psec_chg = TRUE;
            chg_method = authorized ? PSEC_ADD_METHOD_FORWARD : PSEC_ADD_METHOD_BLOCK;
        }
        if (psec_chg) {
            // It really doesn't matter if this function is also called for multi-802.1X even though
            // backend-assigned VLAN is only supported for single- and port-based 802.1X.
            DOT1X_psec_chg(nas_port, isid, api_port, port_info, eap_info, client_info, chg_method);
        }
    } else if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(admin_state)) {
        // Only restart aging if we're going from unauthorized to authorized. If we didn't have this check, and
        // reauthentication is enabled and the reauthentication period is shorter than the age time,
        // then the age timer will never expire.
        if (chgd || !authorized) {
            // Age and hold timers are updated automatically when changing the entry.
            if ((rc = psec_mgmt_mac_chg(PSEC_USER_DOT1X, isid, api_port, &client_info->vid_mac, authorized ? PSEC_ADD_METHOD_FORWARD : PSEC_ADD_METHOD_BLOCK)) != VTSS_RC_OK) {
                T_E("%d:%d: psec_mgmt_mac_chg(): %s", isid, uport, error_txt(rc));
            }
        }
    } else
#endif
    {
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        if (authorized) {
            DOT1X_vlan_set(nas_port, port_info->backend_assigned_vid, port_info->backend_assigned_vid ? NAS_VLAN_TYPE_BACKEND_ASSIGNED : NAS_VLAN_TYPE_NONE);
        } else {
            DOT1X_vlan_set(nas_port, 0, NAS_VLAN_TYPE_NONE);
        }
#endif
        if (chgd) {
            DOT1X_msg_tx_port_state(isid, api_port, authorized);
        }
    }

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    dot1x_acct_authorized_changed(admin_state, sm, authorized);
#endif
}

/******************************************************************************/
// nas_os_backend_server_free_resources()
/******************************************************************************/
void nas_os_backend_server_free_resources(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = nas_get_eap_info(sm);

    DOT1X_CRIT_ASSERT_LOCKED();

    if (eap_info->radius_handle != -1) {
        (void)vtss_radius_free(eap_info->radius_handle);
        eap_info->radius_handle = -1;
    }
}

/****************************************************************************/
// nas_os_backend_server_tx_request()
/****************************************************************************/
BOOL nas_os_backend_server_tx_request(nas_sm_t *sm, int *old_handle, u8 *eap, u16 eap_len, u8 *state, u8 state_len, i8 *user, nas_port_t nas_port, u8 *mac_addr)
{
    vtss_rc               res;
    u8                    handle;
    i8                    temp_str[20];
    vtss_common_macaddr_t sys_mac;
    nas_eap_info_t        *eap_info = nas_get_eap_info(sm);
    vtss_isid_t           isid = DOT1X_NAS_PORT_2_ISID(nas_port);
    vtss_uport_no_t       uport = iport2uport(DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port));

    if (!eap_info) {
        T_E("%d:%d: Called from non-backend-server-aware SM", isid, uport);
        return FALSE;
    }

    DOT1X_CRIT_ASSERT_LOCKED();

    if (*old_handle != -1) {
        T_E("%d:%d: The previous RADIUS handle (%d) was not freed", isid, uport, *old_handle);
        // Better free it.
        (void)vtss_radius_free(*old_handle);
        *old_handle = -1;
    }

    // Allocate a RADIUS handle (i.e. a RADIUS ID).
    if ((res = vtss_radius_alloc(&handle, VTSS_RADIUS_CODE_ACCESS_REQUEST)) != VTSS_RC_OK) {
        if (res != VTSS_RADIUS_ERROR_NOT_INITIALIZED) {
            // "Not Initialized" is not a real error. The module is just not ready yet.
            T_E("%d:%d: Got \"%s\" from vtss_radius_alloc()", isid, uport, vtss_radius_error_txt(res));
        } else {
            T_D("%d:%d: Got \"%s\" from vtss_radius_alloc()", isid, uport, vtss_radius_error_txt(res));
        }
        return FALSE;
    }

    // Add the required attributes
    if (((res = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_EAP_MESSAGE, eap_len,      eap,        TRUE)) != VTSS_RC_OK) ||
        ((res = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_STATE,       state_len,    state,      TRUE)) != VTSS_RC_OK) ||
        ((res = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_USER_NAME,   strlen(user), (u8 *)user, TRUE)) != VTSS_RC_OK)) {
        T_E("%d:%d: Got \"%s\" from vtss_tlv_set() on required attribute", isid, uport, vtss_radius_error_txt(res));
        return FALSE;
    }

    // Add the optional attributes.
#define DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(t, l, v)                                                                \
  res = vtss_radius_tlv_set(handle, t, l, v, FALSE);                                                             \
  if (res != VTSS_RC_OK && res != VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV) {                                          \
    T_E("%d:%d: Got \"%s\" from vtss_tlv_set() on optional attribute", isid, uport, vtss_radius_error_txt(res)); \
    return FALSE;                                                                                                \
  }

    // NAS-Port-Type
    temp_str[0] = temp_str[1] = temp_str[2] = 0;
    temp_str[3] = VTSS_RADIUS_NAS_PORT_TYPE_ETHERNET;
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT_TYPE, 4, (u8 *)temp_str);

    // NAS-Port-Id
    temp_str[0] = temp_str[1] = temp_str[2] = 0;
    temp_str[3] = nas_port;
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT, 4, (u8 *)temp_str);

    (void)snprintf(temp_str, sizeof(temp_str) - 1, "Port %u", nas_port);
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT_ID, strlen(temp_str), (u8 *)temp_str);

    // Calling-Station-Id
    nas_os_mac2str(mac_addr, temp_str);
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_CALLING_STATION_ID, strlen(temp_str), (u8 *)temp_str);

    // Called-Station-Id
    vtss_os_get_systemmac(&sys_mac);
    nas_os_mac2str(sys_mac.macaddr, temp_str);
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_CALLED_STATION_ID, strlen(temp_str), (u8 *)temp_str);

#undef DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    if (!dot1x_acct_append_radius_tlv(handle, eap_info)) {
        return FALSE;
    }
#endif

    // Transmit the RADIUS frame and ask to be called back whenever a response arrives.
    // The RADIUS module takes care of retransmitting, changing server, etc.
    // We use the nas_port as context, rather than the state machine itself, because the
    // state machine may have been used for other purposes once we get called back. Furthermore,
    // if we had used dynamic memory allocation for statemachines (we don't), then the ctx
    // may have been out-of-date (freed) once called back.
    // The combination of port number and RADIUS identifier should uniquely identify the SM.
    if ((res = vtss_radius_tx(handle, (void *)nas_port, DOT1X_radius_rx_callback)) != VTSS_RC_OK) {
        T_E("%d:%d: vtss_radius_tx() returned \"%s\"", isid, uport, vtss_radius_error_txt(res));
        return FALSE;
    }
    eap_info->radius_handle = handle;

    return TRUE;
}

/****************************************************************************/
// nas_os_mac2str()
// For MAC-based authentication, this is also used to generate the
// supplicant ID from the MAC address. Therefore the separator cannot be
// a colon, since IAS doesn't support colons in its user name.
/****************************************************************************/
void nas_os_mac2str(u8 *mac, i8 *str)
{
    sprintf(str, "%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/****************************************************************************/
// nas_os_get_reauth_timer()
/****************************************************************************/
u16 nas_os_get_reauth_timer(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return DOT1X_stack_cfg.glbl_cfg.reauth_period_secs;
}

/****************************************************************************/
// nas_os_get_reauth_enabled()
/****************************************************************************/
BOOL nas_os_get_reauth_enabled(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return DOT1X_stack_cfg.glbl_cfg.reauth_enabled;
}

/****************************************************************************/
// nas_os_get_reauth_max()
/****************************************************************************/
nas_counter_t nas_os_get_reauth_max(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    return DOT1X_stack_cfg.glbl_cfg.reauth_max;
#else
    return 2;
#endif
}

/****************************************************************************/
// nas_os_get_eapol_challenge_timeout()
// All EAPOL request packets (except Request Identity) sent to the supplicant
// are subject to retransmission after this timeout.
// Request Identity Timeout is returned with
// nas_os_get_eapol_request_identity_timeout().
/****************************************************************************/
u16 nas_os_get_eapol_challenge_timeout(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return SUPPLICANT_TIMEOUT;
}

/****************************************************************************/
// nas_os_get_eapol_request_identity_timeout()
// The timeout between retransmission of Request Identity EAPOL frames to
// the supplicant.
// Timeout of other requests to the supplicant is returned with
// nas_os_get_eapol_challenge_timeout().
/****************************************************************************/
u16 nas_os_get_eapol_request_identity_timeout(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return  DOT1X_stack_cfg.glbl_cfg.eapol_timeout_secs;
}

/****************************************************************************/
// nas_os_get_uptime_secs()
/****************************************************************************/
time_t nas_os_get_uptime_secs(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return msg_uptime_get(VTSS_ISID_LOCAL);
}

/******************************************************************************/
// nas_os_init()
/******************************************************************************/
void nas_os_init(nas_port_info_t *port_info)
{
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    port_info->qos_class = QOS_PORT_PRIO_UNDEF;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    port_info->backend_assigned_vid = 0;
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    port_info->eapol_frame_seen = FALSE;
#endif

#ifdef NAS_USES_VLAN
    port_info->current_vid = 0;
    port_info->vlan_type   = NAS_VLAN_TYPE_NONE;
#endif
}

/******************************************************************************/
//
// SEMI-PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// dot1x_crit_enter()
/******************************************************************************/
void dot1x_crit_enter(void)
{
    // Avoid Lint warning: A thread mutex has been locked but not unlocked
    /*lint --e{454} */
    DOT1X_CRIT_ENTER();
}

/******************************************************************************/
// dot1x_crit_exit()
/******************************************************************************/
void dot1x_crit_exit(void)
{
    // Avoid Lint warning: A thread mutex that had not been locked is being unlocked
    /*lint -e(455) */
    DOT1X_CRIT_EXIT();
}

/******************************************************************************/
// dot1x_crit_assert_locked()
/******************************************************************************/
void dot1x_crit_assert_locked(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
}

/****************************************************************************/
// dot1x_disable_due_to_soon_boot()
/****************************************************************************/
void dot1x_disable_due_to_soon_boot(void)
{
    dot1x_stack_cfg_t artificial_cfg;
    DOT1X_CRIT_ENTER();
    if (DOT1X_stack_cfg.glbl_cfg.enabled) {
        artificial_cfg = DOT1X_stack_cfg;
        artificial_cfg.glbl_cfg.enabled = FALSE;
        (void)DOT1X_apply_cfg(VTSS_ISID_GLOBAL, &artificial_cfg, FALSE, NAS_STOP_REASON_SWITCH_REBOOT);
        T_D("Disabling 802.1X");
    }
    DOT1X_CRIT_EXIT();
}

/****************************************************************************/
// dot1x_glbl_enabled(void)
/****************************************************************************/
BOOL dot1x_glbl_enabled(void)
{
    DOT1X_CRIT_ASSERT_LOCKED();
    return DOT1X_stack_cfg.glbl_cfg.enabled;
}

/******************************************************************************/
// dot1x_port_control_to_str()
// Transforms a nas_port_control_t to string
// IN : Brief - TRUE to return text in a brief format in order to show all status within 80 characters
/******************************************************************************/
char *dot1x_port_control_to_str(nas_port_control_t port_control, BOOL brief)
{
    switch (port_control) {
    case NAS_PORT_CONTROL_DISABLED:
        return brief ? "Dis"   : "NAS Disabled";
    case NAS_PORT_CONTROL_FORCE_AUTHORIZED:
        return brief ? "Auth"  : "Force Authorized";
    case NAS_PORT_CONTROL_AUTO:
        return brief ? "Port"  : "Port-based 802.1X";
    case NAS_PORT_CONTROL_FORCE_UNAUTHORIZED:
        return brief ? "UnAut" : "Force Unauthorized";
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    case NAS_PORT_CONTROL_DOT1X_SINGLE:
        return brief ? "Sigle" : "Single 802.1X";
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    case NAS_PORT_CONTROL_DOT1X_MULTI:
        return brief ? "Multi" : "Multi 802.1X";
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    case NAS_PORT_CONTROL_MAC_BASED:
        return brief ? "MAC"   : "MAC-Based Auth";
#endif
    default:
        return "Unknown";
    }
}

/******************************************************************************/
//
// PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// dot1x_mgmt_glbl_cfg_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_glbl_cfg_get(dot1x_glbl_cfg_t *glbl_cfg)
{
    vtss_rc rc;

    if (!glbl_cfg) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    // Use the stack config.
    DOT1X_CRIT_ENTER();
    *glbl_cfg = DOT1X_stack_cfg.glbl_cfg;
    DOT1X_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// dot1x_mgmt_glbl_cfg_set()
/******************************************************************************/
vtss_rc dot1x_mgmt_glbl_cfg_set(dot1x_glbl_cfg_t *glbl_cfg)
{
    vtss_rc           rc;
    dot1x_stack_cfg_t tmp_stack_cfg;

    if (!glbl_cfg) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    if ((rc = DOT1X_cfg_valid_glbl(glbl_cfg)) != VTSS_RC_OK) {
        return rc;
    }

    DOT1X_CRIT_ENTER();

    // We need to create a new structure with the current config
    // and only replace the glbl_cfg member.
    tmp_stack_cfg          = DOT1X_stack_cfg;
    tmp_stack_cfg.glbl_cfg = *glbl_cfg;

    // Apply the configuration. The function will check differences between old and new config
    if ((rc = DOT1X_apply_cfg(VTSS_ISID_GLOBAL, &tmp_stack_cfg, FALSE, NAS_STOP_REASON_PORT_MODE_CHANGED)) == VTSS_RC_OK) {
        // Copy the user's configuration to our configuration
        DOT1X_stack_cfg.glbl_cfg = *glbl_cfg;

        // Save the configuration to flash
        DOT1X_cfg_flash_write();
    } else {
        // Roll back to previous settings without checking the return code
        (void)DOT1X_apply_cfg(VTSS_ISID_GLOBAL, &DOT1X_stack_cfg, FALSE, NAS_STOP_REASON_PORT_MODE_CHANGED);
    }

    DOT1X_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// dot1x_mgmt_switch_cfg_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_cfg_get(vtss_isid_t isid, dot1x_switch_cfg_t *switch_cfg)
{
    vtss_rc rc;

    if (!switch_cfg) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(isid, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    DOT1X_CRIT_ENTER();
    *switch_cfg = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START];
    DOT1X_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// dot1x_mgmt_switch_cfg_set()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_cfg_set(vtss_isid_t isid, dot1x_switch_cfg_t *switch_cfg)
{
    vtss_rc           rc;
    dot1x_stack_cfg_t tmp_stack_cfg;
    port_iter_t       pit;

    if (!switch_cfg) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(isid, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    if ((rc = DOT1X_cfg_valid_switch(switch_cfg)) != VTSS_RC_OK) {
        return rc;
    }

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (switch_cfg->port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
            // Inter-protocol check #1.
            // If aggregation is enabled (LACP or static), then we cannot set a given
            // port to anything but Force Authorized. This is independent of the value
            // of 802.1X globally enabled.
#ifdef VTSS_SW_OPTION_AGGR
            switch (aggr_mgmt_port_participation(isid, pit.iport)) { // This function always exists independent of VTSS_SW_OPTION_LACP
            case 1:
                return DOT1X_ERROR_STATIC_AGGR_ENABLED;

            case 2:
                return DOT1X_ERROR_DYNAMIC_AGGR_ENABLED;

            default:
                break;
            }
#endif /* VTSS_SW_OPTION_AGGR */

            // Inter-protocol check #2.
            // If spanning tree is enabled, then we cannot set a given port
            // to anything but Force Authorized. This is independent of the value
            // of 802.1X globally enabled.
#ifdef VTSS_SW_OPTION_MSTP
            {
                mstp_port_param_t rstp_conf;
                BOOL              stp_enabled = FALSE;
                if (!mstp_get_port_config(isid, pit.iport, &stp_enabled, &rstp_conf)) {
                    return DOT1X_ERROR_STP_ENABLED; // What else to return, when the module returns a BOOL?
                }
                if (stp_enabled) {
                    return DOT1X_ERROR_STP_ENABLED;
                }
            }
#endif /* VTSS_SW_OPTION_MSTP */
        }
    }

    DOT1X_CRIT_ENTER();

    // We need to create a new structure with the current config
    // and only replace this switch's member.
    tmp_stack_cfg = DOT1X_stack_cfg;
    tmp_stack_cfg.switch_cfg[isid - VTSS_ISID_START] = *switch_cfg;

    // Apply the configuration. The function will check differences between old and new config
    if ((rc = DOT1X_apply_cfg(isid, &tmp_stack_cfg, TRUE, NAS_STOP_REASON_PORT_MODE_CHANGED)) == VTSS_RC_OK) {
        // Copy the user's configuration to our configuration
        DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START] = *switch_cfg;

        // Save the configuration to flash
        DOT1X_cfg_flash_write();
    } else {
        // Roll back to previous settings without checking the return code
        (void)DOT1X_apply_cfg(isid, &DOT1X_stack_cfg, TRUE, NAS_STOP_REASON_PORT_MODE_CHANGED);
    }

    DOT1X_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// dot1x_mgmt_port_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_port_status_get(vtss_isid_t isid, vtss_port_no_t api_port, nas_port_status_t *port_status)
{
    vtss_rc rc;

    if (!port_status) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(isid, api_port, TRUE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    DOT1X_CRIT_ENTER();
    if (isid == VTSS_ISID_LOCAL) {
        port_status_t     port_module_status;
        vtss_auth_state_t auth_state = VTSS_AUTH_STATE_NONE;

        // For the local, we can only identify whether there's link or not, and whether the port is authorized or not.
        // It's not possible to see if NAS is globally enabled or not.
        if (port_mgmt_status_get(isid, api_port, &port_module_status) == VTSS_RC_OK && port_module_status.status.link) {
            (void)vtss_auth_port_state_get(NULL, api_port, &auth_state);
            *port_status = auth_state == VTSS_AUTH_STATE_BOTH ? NAS_PORT_STATUS_AUTHORIZED : NAS_PORT_STATUS_UNAUTHORIZED;
        } else {
            // An error occurred in call to port_mgmt_status_get() or the port is linked down.
            *port_status = NAS_PORT_STATUS_LINK_DOWN;
        }
    } else if (DOT1X_stack_cfg.glbl_cfg.enabled) {
        if (DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
            // Link is up. Check for authorized/Non authorized state (this may not be correct, because we may
            // just have got link up, but the core lib may not have been updated on that).
            *port_status = nas_get_port_status(DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port));
        } else {
            // The port is linked down
            *port_status = NAS_PORT_STATUS_LINK_DOWN;
        }
    } else {
        // 802.1X is disabled
        *port_status = NAS_PORT_STATUS_DISABLED;
    }

    DOT1X_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// dot1x_mgmt_switch_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_status_get(vtss_isid_t isid, dot1x_switch_status_t *switch_status)
{
    port_iter_t pit;
    vtss_rc     rc;

    if (!switch_status) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(isid, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    DOT1X_CRIT_ENTER();

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        switch_status->admin_state[pit.iport] = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[pit.iport].admin_state;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        switch_status->qos_class[pit.iport] = QOS_PORT_PRIO_UNDEF;
#endif
#ifdef NAS_USES_VLAN
        switch_status->vlan_type[pit.iport] = NAS_VLAN_TYPE_NONE;
        switch_status->vid[pit.iport]       = 0;
#endif

        if (DOT1X_stack_cfg.glbl_cfg.enabled) {
            if (DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[pit.iport].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
                nas_port_t nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, pit.iport + VTSS_PORT_NO_START);
#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS) || defined(NAS_USES_VLAN)
                nas_port_info_t *port_info = nas_get_port_info(nas_get_top_sm(nas_port));
#endif

                // Link is up. Check for authorized/Non authorized state (this may not be correct, because we may
                // just have got link up, but the core lib may not have been updated on that).
                switch_status->status[pit.iport] = nas_get_port_status(nas_port);
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                switch_status->qos_class[pit.iport] = port_info->qos_class;
#endif
#ifdef NAS_USES_VLAN
                switch_status->vlan_type[pit.iport] = port_info->vlan_type;
                switch_status->vid[pit.iport]       = port_info->current_vid;
#endif
            } else {
                // The port has link down
                switch_status->status[pit.iport] = NAS_PORT_STATUS_LINK_DOWN;
            }
        } else {
            // 802.1X is globally disabled
            switch_status->status[pit.iport] = NAS_PORT_STATUS_DISABLED;
        }
    }

    DOT1X_CRIT_EXIT();
    return VTSS_RC_OK;
}

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// dot1x_mgmt_multi_client_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_multi_client_status_get(vtss_isid_t isid, vtss_port_no_t api_port, dot1x_multi_client_status_t *client_status, BOOL *found, BOOL start_all_over)
{
    nas_port_t         nas_port;
    vtss_rc            rc;
    nas_sm_t           *sm;
    nas_client_info_t  *client_info = NULL;
    nas_port_control_t admin_state;

    if (!found || !client_status) {
        return DOT1X_ERROR_INV_PARAM;
    }

    *found = FALSE;

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);

    DOT1X_CRIT_ENTER();

    admin_state = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].admin_state;
    if (DOT1X_stack_cfg.glbl_cfg.enabled == FALSE || !NAS_PORT_CONTROL_IS_MULTI_CLIENT(admin_state)) {
        rc = VTSS_RC_OK; // Not available. This is not an error, so just return.
        goto do_exit;
    }

    sm = nas_get_top_sm(nas_port);
    for (sm = nas_get_next(sm); sm; sm = nas_get_next(sm)) {
        if (start_all_over) {
            *found = TRUE;
            break;
        }

        // start_all_over is FALSE, so we've been called before.
        client_info = nas_get_client_info(sm);
        if (memcmp(&client_status->client_info.vid_mac, &client_info->vid_mac, sizeof(client_status->client_info.vid_mac)) == 0) {
            nas_sm_t *temp_sm = nas_get_next(sm);
            *found = temp_sm != NULL;
            sm = temp_sm;
            break;
        }
    }

    if (*found) {
        client_status->client_info = *nas_get_client_info(sm);
        client_status->status      = nas_get_status(sm);
    }

    rc = VTSS_RC_OK;

do_exit:
    DOT1X_CRIT_EXIT();
    return rc;
}
#endif

/******************************************************************************/
// dot1x_mgmt_decode_auth_unauth()
// The pendant of nas_os_encode_auth_unauth().
/******************************************************************************/
void dot1x_mgmt_decode_auth_unauth(nas_port_status_t status, u32 *auth_cnt, u32 *unauth_cnt)
{
    u32 status_as_u32 = (u32)status;
    if (!auth_cnt || !unauth_cnt) {
        return;
    }

    if (status < NAS_PORT_STATUS_CNT) {
        *auth_cnt = *unauth_cnt = 0;
    } else {
        *unauth_cnt = ((status_as_u32 >> 16) & 0xFFFF) - 1;
        *auth_cnt   = ((status_as_u32 >>  0) & 0xFFFF) - 1;
    }
}

/******************************************************************************/
// dot1x_mgmt_statistics_get()
// If @vid_mac == NULL or vid_mac->vid == 0, get the port statistics,
// otherwise get the statistics given by @vid_mac.
/******************************************************************************/
vtss_rc dot1x_mgmt_statistics_get(vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_mac_t *vid_mac, dot1x_statistics_t *statistics)
{
    nas_port_t             nas_port;
    vtss_rc                rc;
    nas_sm_t               *sm;

    if (!statistics) {
        return DOT1X_ERROR_INV_PARAM;
    }

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);

    memset(statistics, 0, sizeof(*statistics));

    DOT1X_CRIT_ENTER();

    if (vid_mac == NULL || vid_mac->vid == 0) {
        // Get port-statistics.
        sm = nas_get_top_sm(nas_port);

        // Compose a port-status
        if (DOT1X_stack_cfg.glbl_cfg.enabled) {
            if (DOT1X_stack_state.switch_state[isid - VTSS_ISID_START].port_state[api_port - VTSS_PORT_NO_START].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
                statistics->status = nas_get_port_status(nas_port); // Composes auth+unauth if multi-client all by itself
            } else {
                statistics->status = NAS_PORT_STATUS_LINK_DOWN;
            }
        } else {
            statistics->status = NAS_PORT_STATUS_DISABLED;
        }
    } else {
        // Asking for specific <MAC, VID>
        if ((sm = nas_get_sm_from_vid_mac_port(vid_mac, nas_port)) == NULL) {
            rc = DOT1X_ERROR_MAC_ADDRESS_NOT_FOUND;
            goto do_exit;
        }
        statistics->status = nas_get_status(sm);
    }

    DOT1X_fill_statistics(sm, statistics);

    // Get the admin_state. Do not use that of the NAS base lib (nas_get_port_info(sm)->port_control),
    // because it may return NAS_PORT_CONTROL_DISABLED, and we want what the user has configured.
    statistics->admin_state = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port].admin_state;

    rc = VTSS_RC_OK;

do_exit:
    DOT1X_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// dot1x_mgmt_statistics_clear()
// If @vid_mac == NULL or vid_mac->vid == 0, clear everything on that port,
// otherwise only clear entry given by @vid_mac.
/******************************************************************************/
vtss_rc dot1x_mgmt_statistics_clear(vtss_isid_t isid, vtss_port_no_t api_port, vtss_vid_mac_t *vid_mac)
{
    nas_port_t nas_port;
    vtss_rc    rc;

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);

    DOT1X_CRIT_ENTER();
    nas_clear_statistics(nas_port, vid_mac);
    DOT1X_CRIT_EXIT();

    return VTSS_RC_OK;
}

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// dot1x_mgmt_multi_client_statistics_get()
// For the sake of CLI, we pass a dot1x_statistics_t structure to this
// function.
/******************************************************************************/
vtss_rc dot1x_mgmt_multi_client_statistics_get(vtss_isid_t isid, vtss_port_no_t api_port, dot1x_statistics_t *statistics, BOOL *found, BOOL start_all_over)
{
    nas_port_t         nas_port;
    vtss_rc            rc;
    nas_sm_t           *sm;
    nas_client_info_t  *client_info = NULL;
    nas_port_control_t admin_state;

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
    *found = FALSE;

    DOT1X_CRIT_ENTER();

    admin_state = DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port - VTSS_PORT_NO_START].admin_state;
    if (DOT1X_stack_cfg.glbl_cfg.enabled == FALSE || !NAS_PORT_CONTROL_IS_MULTI_CLIENT(admin_state)) {
        memset(statistics, 0, sizeof(*statistics));
        rc = VTSS_RC_OK; // Not available. This is not an error, so just return.
        goto do_exit;
    }

    sm = nas_get_next(nas_get_top_sm(nas_port));
    if (!start_all_over) {
        while (sm) {
            client_info = nas_get_client_info(sm);
            sm = nas_get_next(sm);
            if (memcmp(&statistics->client_info.vid_mac, &client_info->vid_mac, sizeof(client_info->vid_mac)) == 0) {
                break;
            }
        }
    }

    memset(statistics, 0, sizeof(*statistics));

    if (sm) {
        *found = TRUE;
        DOT1X_fill_statistics(sm, statistics);
        // Status is not filled in by DOT1X_fill_statistics(), because it differs between top- and sub-SMs.
        // Here, @sm is a sub-SM.
        statistics->status = nas_get_status(sm);
        // Get the admin_state. Do not use that of the NAS base lib (nas_get_port_info(sm)->port_control),
        // because it may return NAS_PORT_CONTROL_DISABLED, and we want what the user has configured.
        statistics->admin_state = admin_state;
    }

    rc = VTSS_RC_OK;

do_exit:
    DOT1X_CRIT_EXIT();
    return rc;
}
#endif

/******************************************************************************/
// dot1x_mgmt_port_last_supplicant_info_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_port_last_supplicant_info_get(vtss_isid_t isid, vtss_port_no_t api_port, nas_client_info_t *last_supplicant_info)
{
    nas_port_t nas_port;
    vtss_rc    rc;

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);

    DOT1X_CRIT_ENTER();
    *last_supplicant_info = *nas_get_client_info(nas_get_top_sm(nas_port));
    DOT1X_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// dot1x_mgmt_reauth()
// @now == TRUE  => Reinitialize
// @now == FALSE => Reauthenticate (only affects already authenticated clients)
/******************************************************************************/
vtss_rc dot1x_mgmt_reauth(vtss_isid_t isid, vtss_port_no_t api_port, BOOL now)
{
    nas_port_t nas_port;
    vtss_rc    rc;

    if ((rc = DOT1X_isid_port_check(isid, api_port, FALSE, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    T_D("%d:%d: Enter (now=%d)", isid, iport2uport(api_port), now);

    DOT1X_CRIT_ENTER();
    if (DOT1X_stack_cfg.glbl_cfg.enabled) {
        nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port);
        if (now) {
            nas_reinitialize(nas_port);
        } else {
            nas_reauthenticate(nas_port);
        }
    }
    DOT1X_CRIT_EXIT();

    return VTSS_RC_OK;
}

/****************************************************************************/
// dot1x_error_txt()
/****************************************************************************/
char *dot1x_error_txt(vtss_rc rc)
{
    switch (rc) {
    case DOT1X_ERROR_INV_PARAM:
        return "Invalid parameter";

    case DOT1X_ERROR_INVALID_REAUTH_PERIOD:
        // Not nice to use specific values in this string, but
        // much easier than constructing a constant string dynamically.
        return "Reauthentication period out of bounds ([1; 3600] seconds)";

    case DOT1X_ERROR_INVALID_EAPOL_TIMEOUT:
        // Not nice to use specific values in this string, but
        // much easier than constructing a constant string dynamically.
        return "EAPOL timeout out of bounds ([1; 255] seconds)";

    case DOT1X_ERROR_INVALID_ADMIN_STATE:
        return "Invalid administrative state";

    case DOT1X_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case DOT1X_ERROR_ISID:
        return "Invalid Switch ID";

    case DOT1X_ERROR_PORT:
        return "Invalid port number";

    case DOT1X_ERROR_STATIC_AGGR_ENABLED:
        return "The 802.1X Admin State must be set to Authorized for ports that are enabled for static aggregation";

    case DOT1X_ERROR_DYNAMIC_AGGR_ENABLED:
        return "The 802.1X Admin State must be set to Authorized for ports that are enabled for LACP";

    case DOT1X_ERROR_STP_ENABLED:
        return "The 802.1X Admin State must be set to Authorized for ports that are enabled for Spanning Tree";

    case DOT1X_ERROR_MAC_ADDRESS_NOT_FOUND:
        return "MAC Address not found on specified port";

#ifdef NAS_USES_PSEC
    case DOT1X_ERROR_INVALID_HOLD_TIME:
        return "Hold time for clients whose authentication failed is out of bounds";

    case DOT1X_ERROR_INVALID_AGING_PERIOD:
        return "Aging period for clients whose authentication succeeded is out of bounds";
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    case DOT1X_ERROR_INVALID_GUEST_VLAN_ID:
        return "Guest VLAN ID is out of bounds";

    case DOT1X_ERROR_INVALID_REAUTH_MAX:
        return "Reauth-Max is out of bounds";
#endif

    default:
        return "802.1X: Unknown error code";
    }
}

/****************************************************************************/
// dot1x_qos_class_to_str()
/****************************************************************************/
void dot1x_qos_class_to_str(vtss_prio_t iprio, char *str)
{
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    if (iprio == QOS_PORT_PRIO_UNDEF) {
        strcpy(str, "-");
    } else {
        sprintf(str, "%s", mgmt_prio2txt(iprio, FALSE));
    }
#else
    str[0] = '\0';
#endif
}

/****************************************************************************/
// dot1x_vlan_to_str()
/****************************************************************************/
void dot1x_vlan_to_str(vtss_vid_t vid, char *str)
{
#ifdef NAS_USES_VLAN
    if (vid == 0) {
        strcpy(str, "-");
    } else {
        sprintf(str, "%d", vid);
    }
#else
    str[0] = '\0';
#endif
}

/******************************************************************************/
// dot1x_init()
// Initialize 802.1X Module
/******************************************************************************/
vtss_rc dot1x_init(vtss_init_data_t *data)
{
    vtss_isid_t    isid = data->isid;
    vtss_rc        rc;
    nas_port_t     nas_port;
    vtss_port_no_t api_port;

    /*lint --e{454,456} ... We leave the Mutex locked */
    switch (data->cmd) {
    case INIT_CMD_INIT:

#ifdef VTSS_SW_OPTION_VCLI
        dot1x_cli_init();
#endif

        // Initialize and register trace resources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
        dot1x_acct_init();
#endif

        memset(&DOT1X_stack_state, 0, sizeof(DOT1X_stack_state));

        // Gotta default our configuration, so that we avoid race-conditions
        // (e.g. port-up before we get our master-up event).
        DOT1X_cfg_default_all(&DOT1X_stack_cfg);
        nas_init();

        // Initialize sempahores.
        critd_init(&DOT1X_crit, "crit_dot1x", VTSS_MODULE_ID_DOT1X, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // Initialize message buffer(s)
        VTSS_OS_SEM_CREATE(&DOT1X_msg_buf_pool.sem, 1);

        // Initialize the thread
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          DOT1X_thread,
                          0,
                          "802.1X",
                          DOT1X_thread_stack,
                          sizeof(DOT1X_thread_stack),
                          &DOT1X_thread_handle,
                          &DOT1X_thread_state);
        break;

    case INIT_CMD_START:
        // Register for messages sent from the 802.1X module on other switches in the stack.
        DOT1X_msg_rx_init();

        // Register for 802.1X BPDUs
        DOT1X_bpdu_rx_init();

        // Register for port link-state change events
        if ((rc = port_global_change_register(VTSS_MODULE_ID_DOT1X, DOT1X_link_state_change_callback)) != VTSS_RC_OK) {
            T_E("Unable to hook link-state change events (%s)", error_txt(rc));
        }

#ifdef NAS_USES_PSEC
        if ((rc = psec_mgmt_register_callbacks(PSEC_USER_DOT1X, DOT1X_on_mac_add_callback, DOT1X_on_mac_del_callback)) != VTSS_RC_OK) {
            T_E("Unable to register callbacks (%s)", error_txt(rc));
        }
#endif

#ifdef VTSS_SW_OPTION_ICFG
        VTSS_RC(dot1x_icfg_init()); // iCFG initialization (Show running)
#endif
        // Release ourselves for the first time.
        /*lint -e(455) */
        DOT1X_CRIT_EXIT();
        break;

    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_LOCAL) {
            // Reset local configuration
            // No such configuration for this module
        } else if (VTSS_ISID_LEGAL(isid) || isid == VTSS_ISID_GLOBAL) {
            // Reset switch or stack configuration
            DOT1X_CRIT_ENTER();
            DOT1X_cfg_flash_read(isid, TRUE);
            DOT1X_CRIT_EXIT();
        }
        break;

    case INIT_CMD_MASTER_UP: {
        // In this state, we are 100% sure that DOT1X_stack_cfg contains
        // default values so that we can safely get called back by the
        // port module before we've actually read the current config
        // from flash.
        DOT1X_CRIT_ENTER();
        DOT1X_cfg_flash_read(VTSS_ISID_GLOBAL, FALSE);
        DOT1X_CRIT_EXIT();

        // Wake up the thread
        T_I("Resuming DOT1X thread");
        cyg_thread_resume(DOT1X_thread_handle);
        break;
    }

    case INIT_CMD_MASTER_DOWN:
        // Default our configuration, so that we avoid race-conditions on a sub-sequent
        // master up (port-up events before we're really ready)
        DOT1X_CRIT_ENTER();
        DOT1X_cfg_default_all(&DOT1X_stack_cfg);
        nas_init();
        DOT1X_CRIT_EXIT();
        break;

    case INIT_CMD_SWITCH_ADD: {
        dot1x_switch_state_t *switch_state;

        DOT1X_CRIT_ENTER();
        switch_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START];
        switch_state->switch_exists = TRUE;

        // Send the whole switch state to the new switch.
        DOT1X_tx_switch_state(isid, DOT1X_stack_cfg.glbl_cfg.enabled);

        // Loop through all ports on this switch and check if one of them seem to have link
        // and set the port state accordingly. This may happen if the port module calls
        // the DOT1X_link_state_change_callback() before this piece of code gets executed.
        for (api_port = 0; api_port < VTSS_PORTS; api_port++) {
            if (switch_state->port_state[api_port].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
                nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port + VTSS_PORT_NO_START);
                nas_port_control_t admin_state = DOT1X_stack_cfg.glbl_cfg.enabled ? DOT1X_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[api_port].admin_state : NAS_PORT_CONTROL_DISABLED;
                nas_port_control_t cur_state   = nas_get_port_control(nas_port);
                if (cur_state != admin_state) {
                    DOT1X_tx_port_state(isid, api_port + VTSS_PORT_NO_START, DOT1X_stack_cfg.glbl_cfg.enabled, admin_state);
                    DOT1X_set_port_control(nas_port, admin_state, NAS_STOP_REASON_UNKNOWN /* Doesn't matter as there aren't any clients by now */);
                }
            }
        }

        DOT1X_CRIT_EXIT();
        break;
    }

    case INIT_CMD_SWITCH_DEL: {
        dot1x_switch_state_t *switch_state;
        DOT1X_CRIT_ENTER();
        switch_state = &DOT1X_stack_state.switch_state[isid - VTSS_ISID_START];
        switch_state->switch_exists = FALSE;
        // We cannot send the switch configuration, since the switch doesn't exist anymore.
        // Race conditions may occur as follows:
        // Suppose a port is in a MAC-table based mode and suppose that at least one
        // MAC address is attached to it. The port security module will callback the on-mac-del
        // callback function when it receives the switch-delete event. But suppose the port
        // security module comes before the dot1x module in the array of modules. In that case
        // the callback from the port security module will cause all SMs to be deleted and in
        // this piece of code we should just change the admin state to NAS_PORT_CONTROL_DISABLED,
        // so that no SMs are attached or can be attached at all.
        // However, it may be that this module comes before the port security module in the array
        // of modules to be called with the switch delete event. In this case, we may still have
        // SMs attached to the port.
        // The short story is that MAC-table based admin states should be handled as follows:
        //   - On switch delete and at least one SM is attached to the port, do nothing here,
        //     but wait for the Port Security module to call the on-mac-del callback function.
        //     When the last MAC address is removed, change the port to NAS_PORT_CONTROL_DISABLED
        //     if the switch doesn't exist or the PHY doesn't have link.
        //   - On switch delete and no SMs are currently attached to the port, change the
        //     port to NAS_PORT_CONTROL_DISABLED right here.
        // Non-MAC-table based admin states should be handled as follows:
        //   - On switch delete, change the port to NAS_PORT_CONTROL_DISABLED right here.
        // In the DOT1X_link_state_change_callback(), do the following:
        //   If the switch doesn't exist or is unmanaged, do nothing. This is OK, since
        //   this function sets the cached link state to FALSE on the switch-delete event, either
        //   before or after the Port Module has been called with the switch-delete event.
        //   Otherwise (the switch exists), if the link goes down we detach SMs if the port
        //   is non-MAC-table based. If the link goes down and we're MAC-table based and MAC addresses
        //   are attached, we set the cached link state to FALSE and wait for the on-mac-del callback
        //   to be called. The event of that callback will set the port to NAS_PORT_CONTROL_DISABLED
        //   when the last SM is removed and the link is down.
        //   Otherwise, if the link goes down, the port is MAC-table based and no SMs are attached,
        //   we set the NAS_PORT_CONTROL_DISABLED directly.
        for (api_port = 0; api_port < VTSS_PORTS; api_port++) {
            nas_port = DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port + VTSS_PORT_NO_START);
            if (switch_state->port_state[api_port].flags & DOT1X_PORT_STATE_FLAGS_LINK_UP) {
                // If it's MAC-table based and have at least one attached SM, do nothing, but wait for the Port Security module to
                // delete the entries. Otherwise set the port control to NAS_PORT_CONTROL_DISABLED.
                if (NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(nas_get_port_control(nas_port)) == FALSE || nas_get_client_cnt(nas_port) == 0) {
                    // Don't transmit port state, but just set our underlying SM in a valid state.
                    DOT1X_set_port_control(nas_port, NAS_PORT_CONTROL_DISABLED, NAS_STOP_REASON_SWITCH_DOWN);
                }
                switch_state->port_state[api_port].flags &= ~DOT1X_PORT_STATE_FLAGS_LINK_UP;
            }
        }
        DOT1X_CRIT_EXIT();
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
