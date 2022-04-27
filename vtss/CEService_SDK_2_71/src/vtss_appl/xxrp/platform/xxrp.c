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

/*lint -sem(vtss_mrp_crit_enter, thread_lock) */
/*lint -sem(vtss_mrp_crit_exit, thread_unlock) */

#include "xxrp_api.h"
#include "vtss_xxrp_callout.h"
#include "critd_api.h"
#include "misc_api.h"
#include "conf_api.h"
#include "port_api.h"
#include "packet_api.h"
#include "l2proto_api.h"
#include "vlan_api.h"
#include "msg_api.h"
#include "vtss_bip_buffer_api.h"





#include "mstp_api.h"                   /* For Mstp state */
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "../base/src/vtss_garp.h"
#include "../base/src/vtss_gvrp.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_XXRP

/***************************************************************************************************
 * Trace definitions
 **************************************************************************************************/
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_XXRP
#define VTSS_TRACE_GRP_DEFAULT 0 /* Used by base part */
#define TRACE_GRP_PLATFORM     1
#define TRACE_GRP_TIMER        2
#define TRACE_GRP_RX           3
#define TRACE_GRP_TX           4
#define TRACE_GRP_CRIT         5
#define TRACE_GRP_GVRP         6
#define TRACE_GRP_CNT          7

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "xxrp",
    .descr     = "XXRP module (base)"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (XXRP base)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_PLATFORM] = {
        .name      = "platform",
        .descr     = "Platform calls",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_TIMER] = {
        .name      = "timer",
        .descr     = "Timer calls",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_RX] = {
        .name      = "rx",
        .descr     = "Rx calls",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_TX] = {
        .name      = "tx",
        .descr     = "Tx calls",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_GVRP] = {
        .name      = "gvrp",
        .descr     = "GVRP operations",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/***************************************************************************************************
 * Macros for accessing semaphore functions
 **************************************************************************************************/
#if VTSS_TRACE_ENABLED
#define XXRP_BASE_CRIT_ENTER()             critd_enter(        &XXRP_base_crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_BASE_CRIT_EXIT()              critd_exit(         &XXRP_base_crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_BASE_CRIT_ASSERT_LOCKED()     critd_assert_locked(&XXRP_base_crit,     TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define XXRP_PLATFORM_CRIT_ENTER()         critd_enter(        &XXRP_platform_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_PLATFORM_CRIT_EXIT()          critd_exit(         &XXRP_platform_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_PLATFORM_CRIT_ASSERT_LOCKED() critd_assert_locked(&XXRP_platform_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
/* XXRP_mstp_platform_crit is required as we see conflicts (deadlocks) btw platform and mstp mutexes */
#define XXRP_MSTP_PLATFORM_CRIT_ENTER()    critd_enter(&XXRP_mstp_platform_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_MSTP_PLATFORM_CRIT_EXIT()     critd_exit( &XXRP_mstp_platform_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define XXRP_MSTP_PLATFORM_CRIT_ASSERT_LOCKED() critd_assert_locked(&XXRP_mstp_platform_crit, TRACE_GRP_CRIT,          __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define XXRP_BASE_CRIT_ENTER()                      critd_enter(        &XXRP_base_crit)
#define XXRP_BASE_CRIT_EXIT()                       critd_exit(         &XXRP_base_crit)
#define XXRP_BASE_CRIT_ASSERT_LOCKED()              critd_assert_locked(&XXRP_base_crit)
#define XXRP_PLATFORM_CRIT_ENTER()                  critd_enter(        &XXRP_platform_crit)
#define XXRP_PLATFORM_CRIT_EXIT()                   critd_exit(         &XXRP_platform_crit)
#define XXRP_PLATFORM_CRIT_ASSERT_LOCKED()          critd_assert_locked(&XXRP_platform_crit)
#define XXRP_MSTP_PLATFORM_CRIT_ENTER()             critd_enter(        &XXRP_mstp_platform_crit)
#define XXRP_MSTP_PLATFORM_CRIT_EXIT()              critd_exit(         &XXRP_mstp_platform_crit)
#define XXRP_MSTP_PLATFORM_CRIT_ASSERT_LOCKED()     critd_assert_locked(&XXRP_mstp_platform_crit)
#endif

/***************************************************************************************************
 * Configuration definitions
 **************************************************************************************************/
#define XXRP_MVRP_CONF_VERSION  1 /* MVRP flash configuration version */
#define XXRP_GVRP_CONF_VERSION  1 /* GVRP flash configuration version */

// Generic configuration structure (master only)
// The master contains an instance of this for each application (MVRP, GVRP, etc)
// Remember to subtract VTSS_ISID_START and VTSS_PORT_NO_START from indices
typedef struct {
    BOOL                  global_enable;
    u8                    port_enable[VTSS_ISID_CNT][VTSS_PORT_BF_SIZE]; // Using bit field saves memory
    vtss_mrp_timer_conf_t timers[VTSS_ISID_CNT][VTSS_PORTS];
    u8                    periodic_tx[VTSS_ISID_CNT][VTSS_PORT_BF_SIZE]; // Using bit field saves memory
    u32                   applicant_adm[VTSS_ISID_CNT][VTSS_PORTS];      // Bit field - up to 32 attribute types where attribute type 1 is LSB and FALSE == normal participant
} xxrp_stack_conf_t;

// Overall configuration as saved in flash.
// The master contains an instance of this for each application (MVRP, GVRP, etc)
typedef struct {
    u32               version;    // Current version of the configuration in flash.
    xxrp_stack_conf_t stack_conf; // Configuration for the whole stack
} xxrp_flash_conf_t;

// Generic structure used for local enable/disable configuration on each switch
// Each switch contains an instance of this for each application (MVRP, GVRP, etc)
// The purpose of this is to avoid sending MRPDUs to the master for a disabled port
typedef struct {
    BOOL global_enable;
    u8   port_enable[VTSS_PORT_BF_SIZE]; // Using bit field saves memory
} xxrp_local_conf_t;

#define XXRP_MSTP_PORT_STATE_FORWARDING     1
#define XXRP_MSTP_PORT_STATE_DISCARDING     0
typedef struct {
    u8 port_state_changed;    //Flag to check if port state has changed
    u8 port_state;            //MSTP port state
    u8 port_role_changed;     //Flag to check if port role has changed
    u8 port_role;             //MSTP port role
} xxrp_mstp_port_conf_t;
/***************************************************************************************************
 * Msg definitions
 **************************************************************************************************/
typedef enum {
    XXRP_MSG_ID_LOCAL_CONF_SET,   // Local configuration set request (no reply)
} xxrp_msg_id_t;

// Message for configuration sent by master.
typedef struct {
    xxrp_msg_id_t     msg_id;      // Message ID
    vtss_mrp_appl_t   appl;        // Application for which this configuration applies
    xxrp_local_conf_t switch_conf; // Configuration that is local for a switch
} xxrp_msg_local_conf_t;

/***************************************************************************************************
 * Event flags
 **************************************************************************************************/
#define XXRP_EVENT_FLAG_DOWN                        (1 << 0) /* Master down event */
#define XXRP_EVENT_FLAG_MRPDU                       (1 << 1) /* MRPDU has been received */
#define XXRP_EVENT_FLAG_KICK                        (1 << 2) /* Wake up the thread imediately */
#define XXRP_EVENT_FLAG_MSTP_PORT_STATE_CHANGE      (1 << 3) /* MSTP port state change */
#define XXRP_EVENT_FLAG_MSTP_PORT_ROLE_CHANGE       (1 << 4) /* MSTP port role change */
#define XXRP_EVENT_FLAG_VLAN_2_MSTI_MAP_CHANGE      (1 << 5) /* VLAN->MSTI mapping change */

// A little helper macro. No timeout if w == 0
#define XXRP_FLAG_WAIT(f, w) ((w) ?                                     \
                              cyg_flag_timed_wait(f, 0xfffffff, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, w) : \
                              cyg_flag_wait(      f, 0xfffffff, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR))

/***************************************************************************************************
 * RX buffer
 **************************************************************************************************/
#define XXRP_RX_BUFFER_SIZE_BYTES   40000

/***************************************************************************************************
 * Application specific parameter table
 **************************************************************************************************/
// TBD: Move tp base module?
#define XXRP_MVRP_ETH_TYPE   (0x88f5)
#define XXRP_MVRP_MAC_ADDR   {0x01, 0x80, 0xc2, 0x00, 0x00, 0x21}
#define XXRP_GARP_LLC_HEADER {0x42, 0x42, 0x03}

static const uchar llc_header[] = XXRP_GARP_LLC_HEADER;

typedef struct {
    mac_addr_t    dmac;                 // Application specific destination MAC address
    u16           etype;                // Application specific ether-type. Set to 0 when using LLC header (GARP only)
    conf_blk_id_t conf_block;           // Application specific configuration block in flash
    u32           conf_version;         // Application specific version of configuration block in flash
    u8            attr_type_cnt;        // Maximum number of attribute types supported in this application
} xxrp_mrp_appl_parm_t;

static const xxrp_mrp_appl_parm_t XXRP_mrp_appl_parm[VTSS_MRP_APPL_MAX] = {










#ifdef VTSS_SW_OPTION_GVRP
    [VTSS_GARP_APPL_GVRP] = {
        XXRP_MVRP_MAC_ADDR,
        0, // len/type indicate LLC
        CONF_BLK_GVRP_CONF_TABLE,
        XXRP_GVRP_CONF_VERSION,
        1                               // Only one: VID Vector
    },
#endif

};

/***************************************************************************************************
 * Global variables
 **************************************************************************************************/
static critd_t                  XXRP_base_crit;                     // Critical section for base
static critd_t                  XXRP_platform_crit;                 // Critical section for platform
static critd_t                  XXRP_mstp_platform_crit;            // Critical section for platform code related to mstp

static xxrp_local_conf_t        XXRP_local_conf[VTSS_MRP_APPL_MAX]; // One instance per application (all switches)
static xxrp_stack_conf_t        XXRP_stack_conf[VTSS_MRP_APPL_MAX]; // One instance per application (master only)

static cyg_flag_t               XXRP_tm_thread_flag;                // Flag for signalling events to timer thread
static cyg_handle_t             XXRP_tm_thread_handle;
static cyg_thread               XXRP_tm_thread_state;
static char                     XXRP_tm_thread_stack[THREAD_DEFAULT_STACK_SIZE * 4]; // TBD: Check this!!!

static cyg_flag_t               XXRP_rx_thread_flag;                // Flag for signalling events to rx thread
static cyg_handle_t             XXRP_rx_thread_handle;
static cyg_thread               XXRP_rx_thread_state;
static char                     XXRP_rx_thread_stack[THREAD_DEFAULT_STACK_SIZE * 4]; // TBD: Check this!!!

static vtss_bip_buffer_t        XXRP_rx_buffer;                     // Buffer for received MRPDUs (master only)

static u8                       XXRP_p2p_cache[VTSS_ISID_CNT][VTSS_PORT_BF_SIZE]; // Bit == TRUE means p2p
static xxrp_mstp_port_conf_t    xxrp_mstp_port_conf[L2_MAX_PORTS + 1][N_L2_MSTI_MAX + 1]; //MSTP port configuration
/***************************************************************************************************
 * Private functions
 **************************************************************************************************/

/***************************************************************************************************
 * XXRP_base_port_timer_set_specific()
 * Update base module with port timer configuration for specific application.
 * If check == TRUE it verifies the ports enabled state.
 **************************************************************************************************/
static void XXRP_base_port_timer_set_specific(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL check)
{
    BOOL enabled = TRUE;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if (check) {
        enabled = (XXRP_stack_conf[appl].global_enable && VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport));
    }

    if (enabled) {
        u32 brc;
        u32 l2port = L2PORT2PORT(isid, iport);
        if ((brc = vtss_xxrp_timers_conf_set(appl, l2port,
                                             &XXRP_stack_conf[appl].timers[isid - VTSS_ISID_START][iport - VTSS_PORT_NO_START])) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to update timer conf for appl %u port %u (brc %u)", appl, l2port, brc);
        }
    }
}

/***************************************************************************************************
 * XXRP_base_port_periodic_tx_set_specific()
 * Update base module with port periodix tx configuration for specific application.
 * If check == TRUE it verifies the ports enabled state.
 **************************************************************************************************/
static void XXRP_base_port_periodic_tx_set_specific(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL check)
{
    BOOL enabled = TRUE;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if (check) {
        enabled = (XXRP_stack_conf[appl].global_enable && VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport));
    }

    if (enabled) {
        u32 brc;
        u32 l2port = L2PORT2PORT(isid, iport);
        if ((brc = vtss_xxrp_periodic_transmission_control_conf_set(appl, l2port,
                                                                    VTSS_PORT_BF_GET(XXRP_stack_conf[appl].periodic_tx[isid - VTSS_ISID_START], iport))) != VTSS_XXRP_RC_OK) {
            T_DG(TRACE_GRP_PLATFORM, "Unable to update periodic tx conf for appl %u port %u (brc %u)", appl, l2port, brc);
            return;
        }
    }
}

/***************************************************************************************************
 * XXRP_base_port_applicant_adm_set_specific()
 * Update base module with port applicant adm configuration for specific application.
 * If check == TRUE it verifies the ports enabled state.
 **************************************************************************************************/
static void XXRP_base_port_applicant_adm_set_specific(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL check)
{
    BOOL enabled = TRUE;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if (check) {
        enabled = (XXRP_stack_conf[appl].global_enable && VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport));
    }

    if (enabled) {
        int adm_ix;
        u32 brc;
        u32 l2port = L2PORT2PORT(isid, iport);
        u32 applicant_adm = XXRP_stack_conf[appl].applicant_adm[isid - VTSS_ISID_START][iport - VTSS_PORT_NO_START];

        for (adm_ix = 0; adm_ix < 32; adm_ix++) { // Loop through all applicant_adm bits
            BOOL                      participant;
            vtss_mrp_attribute_type_t attr_type;

            if (adm_ix >= XXRP_mrp_appl_parm[appl].attr_type_cnt) {
                break; // No more attribute types defined for this MRP application
            }

            participant = !(applicant_adm & (1 << adm_ix)); // bit == 1 in applicant_adm means non-participant - base module expects the inverse
            attr_type.dummy = adm_ix + 1;





            if ((brc = vtss_xxrp_applicant_admin_control_conf_set(appl, l2port, attr_type, participant))) {
                T_EG(TRACE_GRP_PLATFORM, "Unable to update applicant adm conf for appl %u port %u (brc %u)", appl, l2port, brc);
                return;
            }
        }
    }
}

/***************************************************************************************************
 * XXRP_base_port_sync_specific()
 * Synchronize base module with specific port configuration and state for a given application.
 * This function expects that the application is globally enabled.
 **************************************************************************************************/
static void XXRP_base_port_sync_specific(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl)
{
    u32  brc;
    BOOL enabled_base;
    BOOL enabled_conf = VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport);
    u32  l2port = L2PORT2PORT(isid, iport);

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if ((brc = vtss_xxrp_port_control_conf_get(appl, l2port, &enabled_base)) != VTSS_XXRP_RC_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Unable to get port conf for appl %u port %u (brc %u)", appl, l2port, brc);
        return;
    }

    if (enabled_conf != enabled_base) { // Synchronize port enabled
        if ((brc = vtss_xxrp_port_control_conf_set(appl, l2port, enabled_conf)) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to sync port conf for appl %u port %u (brc %u)", appl, l2port, brc);
            return;
        }
    }

    if (enabled_conf) { // Update all other port configuration and state
        //uchar msti;
        XXRP_base_port_timer_set_specific(isid, iport, appl, FALSE);
        XXRP_base_port_periodic_tx_set_specific(isid, iport, appl, FALSE);
        XXRP_base_port_applicant_adm_set_specific(isid, iport, appl, FALSE);

#if 0   /* This will be taken care in vtss_mrp_port_control_conf_set() */
        for (msti = 0; msti < N_L2_MSTI_MAX; msti++) {
            vtss_stp_state_t state = l2_get_msti_stpstate(msti, l2port);

            if ((brc = vtss_mrp_mstp_port_state_change_handler(l2port, msti,
                                                               (state == VTSS_STP_STATE_FORWARDING) ? VTSS_MRP_MSTP_PORT_ADD : VTSS_MRP_MSTP_PORT_DELETE)) != VTSS_XXRP_RC_OK) {
                T_EG(TRACE_GRP_PLATFORM, "Unable to set mstp state for port %u (brc %u)", l2port, brc);
            }
        }
#endif

        T_IG(TRACE_GRP_PLATFORM, "Update base module with p2p state for port %u state %s)", l2port,
             VTSS_PORT_BF_GET(XXRP_p2p_cache[isid - VTSS_ISID_START], iport) ? "p2p" : "shared");

        /*
          TBD Missing:
          Call vtss_mrp_mstp_port_role_change_handler()
          Update base module from XXRP_p2p_cache
        */
    }
}

/***************************************************************************************************
 * XXRP_base_switch_sync_specific()
 * Called on one of the following events:
 * 1: A switch has been added (via XXRP_base_switch_sync())
 * 2: Configuration reset to default (whole stack) (via XXRP_base_switch_sync())
 * 3: An MRP application has been globally enabled or disabled
 * Synchronize base module with actual configuration.
 **************************************************************************************************/
static void XXRP_base_switch_sync_specific(vtss_isid_t isid, vtss_mrp_appl_t appl)
{
    u32  brc;
    BOOL global_enabled_base;
    BOOL global_enabled_conf = XXRP_stack_conf[appl].global_enable;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if ((brc = vtss_mrp_global_control_conf_get(appl, &global_enabled_base)) != VTSS_XXRP_RC_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Unable to get global conf for appl %u (brc %u)", appl, brc);
        return;
    }

    T_N("?tf? XXRP_base_switch_sync_specific(%d,%d) %d %d\n", isid, appl, global_enabled_base, global_enabled_conf);

    if (global_enabled_conf != global_enabled_base) { // Synchronize global enabled

        if ((brc = vtss_mrp_global_control_conf_set(appl, global_enabled_conf)) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to sync global conf for appl %u (brc %u)", appl, brc);
            return;
        }
    }

    if (global_enabled_conf) { // Synchronize all ports
        switch_iter_t sit;
        port_iter_t   pit;

        (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
        (void)port_iter_init(&pit, &sit, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            XXRP_base_port_sync_specific(sit.isid, pit.iport, appl);
        }
    }
}

/***************************************************************************************************
 * XXRP_base_switch_sync()
 * Call XXRP_base_switch_add_specific() for all applications
 **************************************************************************************************/
static void XXRP_base_switch_sync(vtss_isid_t isid)
{
    vtss_mrp_appl_t       appl;

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        XXRP_base_switch_sync_specific(isid, appl);
    }
}

/***************************************************************************************************
 * XXRP_base_master_down
 * We are not master anymore.
 * Delete all MRP applications.
 **************************************************************************************************/
static void XXRP_base_master_down(void)
{
    vtss_mrp_appl_t appl;
    u32             brc;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        BOOL global_enabled;

        if ((brc = vtss_mrp_global_control_conf_get(appl, &global_enabled)) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to get global conf for appl %u (brc %u)", appl, brc);
            continue; // Try next application
        }

        if (!global_enabled) {
            continue; // Nothing to do for this application
        }
        if ((brc = vtss_mrp_global_control_conf_set(appl, FALSE)) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to disable appl %u (brc %u)", appl, brc);
        }
    }
}

/***************************************************************************************************
 * XXRP_base_switch_del
 * A switch has left the stack.
 * Disable all MRP enabled ports on all enabled MRP applications.
 **************************************************************************************************/
static void XXRP_base_switch_del(vtss_isid_t isid)
{
    vtss_mrp_appl_t appl;
    u32             brc;

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        BOOL          global_enabled;
        switch_iter_t sit;
        port_iter_t   pit;

        if ((brc = vtss_mrp_global_control_conf_get(appl, &global_enabled)) != VTSS_XXRP_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to get global conf for appl %u (brc %u)", appl, brc);
            continue; // Try next application
        }

        if (!global_enabled) {
            continue; // Nothing to do for this application
        }

        (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
        (void)port_iter_init(&pit, &sit, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            BOOL enabled;
            u32  l2port = L2PORT2PORT(sit.isid, pit.iport);
            if ((brc = vtss_xxrp_port_control_conf_get(appl, l2port, &enabled)) != VTSS_XXRP_RC_OK) {
                T_EG(TRACE_GRP_PLATFORM, "Unable to get port conf for appl %u port %u (brc %u)", appl, l2port, brc);
                continue; // Try next port
            }

            if (!enabled) {
                continue; // Nothing to do for this port
            }

            if ((brc = vtss_xxrp_port_control_conf_set(appl, l2port, FALSE)) != VTSS_XXRP_RC_OK) {
                T_EG(TRACE_GRP_PLATFORM, "Unable to disable appl %u port %u (brc %u)", appl, l2port, brc);
            }
        }
    }
}

/***************************************************************************************************
 * XXRP_forwarding_control()
 * Setup forwarding or copy to CPU for all MRPDUs on local switch
 **************************************************************************************************/
void XXRP_forwarding_control(void)
{
    vtss_rc               rc;
    vtss_mrp_appl_t       appl;
    vtss_packet_rx_conf_t conf;

    T_N("?tf? XXRP_forwarding_control");

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    memset(&conf, 0, sizeof(vtss_packet_rx_conf_t));

    vtss_appl_api_lock();

    if ((rc = vtss_packet_rx_conf_get(NULL, &conf)) != VTSS_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Failed to get packet_rx_conf (%s)", error_txt(rc));
        goto do_exit;
    }

    // First run: Disable copying to CPU for all included MRP applications
    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        int dmac_ix = XXRP_mrp_appl_parm[appl].dmac[5] & 0x0f; // Get the last nibble in the dmac
        conf.reg.garp_cpu_only[dmac_ix] = FALSE;
    }

    // Second run: Enable copying to CPU for all enabled and included MRP applications
    // This is neccessary as DMAC addresses can be shared by MRP application
    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        int dmac_ix = XXRP_mrp_appl_parm[appl].dmac[5] & 0x0f; // Get the last nibble in the dmac
        T_N("appl=%d, %d", appl, XXRP_local_conf[appl].global_enable);
        if (XXRP_local_conf[appl].global_enable) {
            conf.reg.garp_cpu_only[dmac_ix] = TRUE;
        }
    }

    if ((rc = vtss_packet_rx_conf_set(NULL, &conf)) != VTSS_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Failed to set packet_rx_conf (%s)", error_txt(rc));
    }
do_exit:
    vtss_appl_api_unlock();
}

/***************************************************************************************************
 * XXRP_flash_write_specific()
 * Write the MRP application specific configuration to flash.
 **************************************************************************************************/
static vtss_rc XXRP_flash_write_specific(vtss_mrp_appl_t appl)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vtss_rc            rc = XXRP_ERROR_FLASH;
    xxrp_flash_conf_t *flash_cfg;
    ulong              size;

    if (appl >=  VTSS_MRP_APPL_MAX) {
        T_EG(TRACE_GRP_PLATFORM, "appl >=  VTSS_MRP_APPL_MAX");
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, XXRP_mrp_appl_parm[appl].conf_block, &size))) {
        if (size == sizeof(*flash_cfg)) {
            flash_cfg->stack_conf = XXRP_stack_conf[appl];
            rc = VTSS_OK;
        } else {
            T_WG(TRACE_GRP_PLATFORM, "Could not save MRP configuration - size did not match");
        }
        conf_sec_close(CONF_SEC_GLOBAL, XXRP_mrp_appl_parm[appl].conf_block);
    } else {
        T_WG(TRACE_GRP_PLATFORM, "Failed to open MRP configuration");
    }
    return rc;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/***************************************************************************************************
 * XXRP_conf_default()
 * Initialize generic configuration to default.
 **************************************************************************************************/
static void XXRP_conf_default(xxrp_stack_conf_t *cfg)
{
    switch_iter_t       sit;
    port_iter_t         pit;

    memset(cfg, 0, sizeof(xxrp_stack_conf_t));
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    (void)port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        vtss_mrp_timer_conf_t *timers = &cfg->timers[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START];
        timers->join_timer      = VTSS_MRP_JOIN_TIMER_DEF;
        timers->leave_timer     = VTSS_MRP_LEAVE_TIMER_DEF;
        timers->leave_all_timer = VTSS_MRP_LEAVEALL_TIMER_DEF;
    }

#ifdef VTSS_SW_OPTION_GVRP
    GVRP_CRIT_ENTER();
    vtss_gvrp_destruct(TRUE);
    GVRP_CRIT_EXIT();
#endif

}

/***************************************************************************************************
 * XXRP_conf_timers_valid()
 * Validate the timer values for a single port configuration.
 * Return TRUE if ok.
 **************************************************************************************************/
static BOOL XXRP_conf_timers_valid(const vtss_mrp_timer_conf_t *cfg)
{
    /* VTSS_MRP_JOIN_TIMER_MIN is currently zero, but may change in future. Hence ignoring lint error */
    /*lint --e{685, 568} */
    if ((cfg->join_timer < VTSS_MRP_JOIN_TIMER_MIN) || (cfg->join_timer > VTSS_MRP_JOIN_TIMER_MAX)) {
        return FALSE;
    }
    if ((cfg->leave_timer < VTSS_MRP_LEAVE_TIMER_MIN) || (cfg->leave_timer > VTSS_MRP_LEAVE_TIMER_MAX)) {
        return FALSE;
    }
    if ((cfg->leave_all_timer < VTSS_MRP_LEAVEALL_TIMER_MIN) || (cfg->leave_all_timer > VTSS_MRP_LEAVEALL_TIMER_MAX)) {
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************************************
 * XXRP_flash_read_specific()
 * Read the MRP application specific configuration from flash.
 * If read fails or create_default == TRUE then create default configuration.
 **************************************************************************************************/
static void XXRP_flash_read_specific(vtss_mrp_appl_t appl, BOOL create_defaults)
{
    xxrp_flash_conf_t *flash_conf;
    ulong              size;
    BOOL               do_create = create_defaults;
    switch_iter_t      sit;
    port_iter_t        pit;

    if (appl >=  VTSS_MRP_APPL_MAX) {
        T_EG(TRACE_GRP_PLATFORM, "appl >=  VTSS_MRP_APPL_MAX");
        return;
    }

    XXRP_PLATFORM_CRIT_ASSERT_LOCKED();

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, XXRP_mrp_appl_parm[appl].conf_block, &size)) == NULL || (size != sizeof(xxrp_flash_conf_t))) {
            T_WG(TRACE_GRP_PLATFORM, "conf_sec_open() failed or size mismatch, creating defaults");
            flash_conf = conf_sec_create(CONF_SEC_GLOBAL, XXRP_mrp_appl_parm[appl].conf_block, sizeof(xxrp_flash_conf_t));
            do_create = TRUE;
        } else if (flash_conf->version != XXRP_mrp_appl_parm[appl].conf_version) {
            T_WG(TRACE_GRP_PLATFORM, "Version mismatch, creating defaults");
            do_create = TRUE;
        }
        // Verify existing configuration
        if (!do_create && flash_conf) {
            (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
            (void)port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                if (!XXRP_conf_timers_valid(&flash_conf->stack_conf.timers[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START])) {
                    T_WG(TRACE_GRP_PLATFORM, "Invalid configuration, creating defaults");
                    do_create = TRUE;
                    break;
                }
            }
        }
    } else {
        flash_conf = NULL;
        do_create  = TRUE;
    }


    if (do_create) { // Create new default configuration
        XXRP_conf_default(&XXRP_stack_conf[appl]);
        if (flash_conf) {
            flash_conf->stack_conf = XXRP_stack_conf[appl];
        }
    } else { // Read current configuration
        if (flash_conf) { // Make lint happy. It is never NULL here
            XXRP_stack_conf[appl] = flash_conf->stack_conf;
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (flash_conf) {
        flash_conf->version = XXRP_mrp_appl_parm[appl].conf_version;
        conf_sec_close(CONF_SEC_GLOBAL, XXRP_mrp_appl_parm[appl].conf_block);
    } else {
        T_WG(TRACE_GRP_PLATFORM, "Failed to open flash configuration");
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/***************************************************************************************************
 * XXRP_flash_read()
 * Read all configurations from flash.
 **************************************************************************************************/
static void XXRP_flash_read(BOOL create_defaults)
{
    vtss_mrp_appl_t       appl;

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        XXRP_flash_read_specific(appl, create_defaults);
    }
}

/***************************************************************************************************
 * XXRP_msg_tx_local_conf_specific()
 * Transmit application specific local configuration to one or all switches.
 **************************************************************************************************/
static void XXRP_msg_tx_local_conf_specific(vtss_isid_t isid, vtss_mrp_appl_t appl)
{
    switch_iter_t sit;

    if (appl >=  VTSS_MRP_APPL_MAX) {
        T_EG(TRACE_GRP_PLATFORM, "appl >=  VTSS_MRP_APPL_MAX");
        return;
    }

    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        xxrp_msg_local_conf_t *msg = VTSS_MALLOC(sizeof(xxrp_msg_local_conf_t));
        if (msg == NULL) {
            T_EG(TRACE_GRP_PLATFORM, "Allocation failed.\n");
            return;
        }
        msg->msg_id = XXRP_MSG_ID_LOCAL_CONF_SET;
        msg->appl   = appl;
        XXRP_PLATFORM_CRIT_ENTER();
        T_N("?tf? XXRP_msg_tx_local_conf_specific appl=%d, eba=%d", appl, XXRP_stack_conf[msg->appl].global_enable);
        msg->switch_conf.global_enable = XXRP_stack_conf[msg->appl].global_enable;
        memcpy(msg->switch_conf.port_enable, XXRP_stack_conf[msg->appl].port_enable[sit.isid - VTSS_ISID_START], VTSS_PORT_BF_SIZE);
        XXRP_PLATFORM_CRIT_EXIT();
        msg_tx(VTSS_MODULE_ID_XXRP, sit.isid, msg, sizeof(*msg));
    }
}

/***************************************************************************************************
 * XXRP_msg_tx_local_conf()
 * Transmit all application specific local configurations to one or all switches.
 **************************************************************************************************/
static void XXRP_msg_tx_local_conf(vtss_isid_t isid)
{
    vtss_mrp_appl_t       appl;

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        XXRP_msg_tx_local_conf_specific(isid, appl);
    }
}

/***************************************************************************************************
 * XXRP_msg_rx()
 **************************************************************************************************/
static BOOL XXRP_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    xxrp_msg_id_t msg_id = *(xxrp_msg_id_t *)rx_msg;

    switch (msg_id) {
    case XXRP_MSG_ID_LOCAL_CONF_SET: {
        xxrp_msg_local_conf_t *msg = (xxrp_msg_local_conf_t *)rx_msg;

        VTSS_ASSERT(msg->appl < VTSS_MRP_APPL_MAX);
        XXRP_PLATFORM_CRIT_ENTER();
        XXRP_local_conf[msg->appl].global_enable = msg->switch_conf.global_enable;
        memcpy(XXRP_local_conf[msg->appl].port_enable, msg->switch_conf.port_enable, VTSS_PORT_BF_SIZE);
        XXRP_forwarding_control(); // Setup forwarding or copy to CPU.
        XXRP_PLATFORM_CRIT_EXIT();
        break;
    }

    default:
        T_WG(TRACE_GRP_PLATFORM, "Unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

/***************************************************************************************************
 * XXRP_msg_register()
 * Register for XXRP messages
 **************************************************************************************************/
static void XXRP_msg_register(void)
{
    msg_rx_filter_t filter;
    vtss_rc         rc;

    memset(&filter, 0, sizeof(filter));
    filter.cb = XXRP_msg_rx;
    filter.modid = VTSS_MODULE_ID_XXRP;
    if ((rc = msg_rx_filter_register(&filter)) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Failed to register for XXRP messages (%s)", error_txt(rc));
    }
}

/***************************************************************************************************
 * XXRP_packet_rx()
 * MRPDU packet reception on local switch.
 * Send it to the current master via l2proto if MRP app is global enabled and enabled on port.
 **************************************************************************************************/
static BOOL XXRP_packet_rx(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    vtss_mrp_appl_t appl;
    BOOL            send_to_master = FALSE, rc = FALSE; // Allow other subscribers to receive the packet

    T_NG(TRACE_GRP_RX, "MRPDU rx %s on port %u from %s", contxt ? "MRP" : "GARP", iport2uport(rx_info->port_no), misc_mac2str(&frm[6]));

    if (rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED) {
        T_DG(TRACE_GRP_RX, "Tagged MRPDU received on port %u", iport2uport(rx_info->port_no));
        return rc;
    }

    // Lookup the MRP application
    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {
        if ( (!contxt && XXRP_mrp_appl_parm[appl].etype) ||
             (contxt && !XXRP_mrp_appl_parm[appl].etype) ) {
            continue;
        }


        if (memcmp(&frm[0], XXRP_mrp_appl_parm[appl].dmac, 6)) {
            continue; // No match on DMAC
        }

        if (contxt) { // Try to match on ETYPE
            if (((frm[12] << 8) + frm[13]) != XXRP_mrp_appl_parm[appl].etype) {
                continue; // No match on ETYPE
            }
        } else { // Try to match on LLC header
            if (memcmp(&frm[14], llc_header, sizeof(llc_header))) {
                continue; // No match on LLC header
            }
        }
        T_NG(TRACE_GRP_RX, "Matching application found (%d)", appl);
        break; // Matching application found
    }

    if (appl >= VTSS_MRP_APPL_MAX) {
        T_DG(TRACE_GRP_RX, "No matching application found");
        return rc;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    send_to_master = (XXRP_local_conf[appl].global_enable &&
                      VTSS_PORT_BF_GET(XXRP_local_conf[appl].port_enable, rx_info->port_no));
    XXRP_PLATFORM_CRIT_EXIT();

    if (send_to_master) {
        // Port is enabled - send to master
        T_NG(TRACE_GRP_RX, "MRPDU forward to master");
        l2_receive_indication(VTSS_MODULE_ID_XXRP, frm, rx_info->length, rx_info->port_no, rx_info->tag.vid, VTSS_GLAG_NO_NONE);
        return rc;
    }
    // Port is disabled - discard frame
    T_NG(TRACE_GRP_RX, "MRPDU discarded");
    return rc;
}

/***************************************************************************************************
 * XXRP_packet_register()
 * Register with the packet module for receiving XXRP packets (MRPDUs).
 * There are two different kind of registrations:
 * 1: Match on DMAC and ETYPE. Used for MRP apps which are always associated with an ETYPE.
 * 2: Match on DMAC only. Used for GARP apps which are always identified by a LLC header (42-42-03).
 * Registrations with an ETYPE has higher priority than registrations with a LLC header.
 * The same cb is used for both types of registrations and the contxt parameter is used to
 * distinguish them from each other (*contxt != 0 means with ETYPE).
 * Unregistration is not neccessary as MRPDUs are only delivered to the CPU if the specific MRP
 * application is globally enabled.
 **************************************************************************************************/
static void XXRP_packet_register(void)
{
    packet_rx_filter_t rx_filter;
    void               *rx_filter_id;
    vtss_mrp_appl_t    appl;
    vtss_rc            rc;

    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_XXRP;
    rx_filter.cb    = XXRP_packet_rx;

    for (appl = 0; appl < VTSS_MRP_APPL_MAX; appl++) {

        memcpy(rx_filter.dmac, XXRP_mrp_appl_parm[appl].dmac, sizeof(rx_filter.dmac));
        if (XXRP_mrp_appl_parm[appl].etype) { // MRP application

            rx_filter.match  = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
            rx_filter.prio   = PACKET_RX_FILTER_PRIO_NORMAL;
            rx_filter.etype  = XXRP_mrp_appl_parm[appl].etype;
            rx_filter.contxt = (void *)1; // Any non NULL value will be ok here

        } else { // GARP application

            rx_filter.match  = PACKET_RX_FILTER_MATCH_DMAC;
            rx_filter.prio   = PACKET_RX_FILTER_PRIO_LOW;
            rx_filter.etype  = 0;
            rx_filter.contxt = NULL;
        }

        if ((rc = packet_rx_filter_register(&rx_filter, &rx_filter_id)) != VTSS_RC_OK) {
            T_EG(TRACE_GRP_PLATFORM, "Failed to register for XXRP packets (%s)", error_txt(rc));
        }
    }
}

/***************************************************************************************************
 * XXRP_l2_rx()
 * MRPDU packet reception on master from all the slaves (including the master itself).
 * Send it to the rx buffer for later processing by the rx thread.
 **************************************************************************************************/
static void XXRP_l2_rx(const void *packet, size_t len, vtss_vid_t vid, l2_port_no_t l2port)
{
    u32    *buf;
    size_t total_len = len + (3 * sizeof(u32)); // We reserve two extra u32 for total length and l2 port
    size_t delta;
    T_NG(TRACE_GRP_RX, "MRPDU rx on l2 port %u", l2port);

    // Ensure, that buffers, i.e., 'buf' will always be 4 byte alligned.
    if ((delta = total_len % 4)) {
        total_len += 4 - delta;
    }

    do {

        XXRP_PLATFORM_CRIT_ENTER();

        if ((buf = (u32 *)vtss_bip_buffer_reserve(&XXRP_rx_buffer, total_len)) != NULL) {
            buf[0] = total_len;                      // Save the total length first (in bytes)
            buf[1] = l2port;                         // Then the l2 port it came from
            buf[2] = len;
            memcpy(&buf[3], packet, len);            // Then the data
            vtss_bip_buffer_commit(&XXRP_rx_buffer); // Tell it to the BIP buffer.
        }
        XXRP_PLATFORM_CRIT_EXIT();
        if (buf) {
            cyg_flag_setbits(&XXRP_rx_thread_flag, XXRP_EVENT_FLAG_MRPDU);
        } else {
            // Force scheduling, so that some bip_buffers can come back
            cyg_thread_yield();
        }

    } while (!buf);

}

/***************************************************************************************************
 * XXRP_mstp_state_change()
 * Callback function for mstp state changes.
 **************************************************************************************************/
static void XXRP_mstp_state_change(vtss_common_port_t l2port, uchar msti, vtss_common_stpstate_t new_state)
{
    xxrp_mstp_port_conf_t   *conf;

    T_DG(TRACE_GRP_PLATFORM, "MSTP state on l2 port %u, msti %u = %u ", l2port, msti, new_state);

    if ((new_state == VTSS_STP_STATE_FORWARDING) || (new_state == VTSS_STP_STATE_DISCARDING)) {
        XXRP_MSTP_PLATFORM_CRIT_ENTER();
        conf = &xxrp_mstp_port_conf[l2port][msti];
        conf->port_state_changed = TRUE;
        conf->port_state = ((new_state == VTSS_STP_STATE_FORWARDING) ? XXRP_MSTP_PORT_STATE_FORWARDING : XXRP_MSTP_PORT_STATE_DISCARDING);
        cyg_flag_setbits(&XXRP_rx_thread_flag, XXRP_EVENT_FLAG_MSTP_PORT_STATE_CHANGE);
        XXRP_MSTP_PLATFORM_CRIT_EXIT();
    }
}

/***************************************************************************************************
 * XXRP_mstp_state_change_handler()
 * Callback function for mstp state changes.
 **************************************************************************************************/
static void XXRP_mstp_state_change_handler(void)
{
    u32                                       brc;
    vtss_mrp_mstp_port_state_change_type_t    new_state;
    vtss_common_port_t                        l2port;
    u8                                        msti;
    BOOL                                      state_changed = FALSE;

    for (l2port = 0; l2port <= L2_MAX_PORTS; l2port++) {
        for (msti = 0; msti < N_L2_MSTI_MAX; msti++) {
            XXRP_MSTP_PLATFORM_CRIT_ENTER();

            state_changed = xxrp_mstp_port_conf[l2port][msti].port_state_changed;
            if (state_changed) {
                xxrp_mstp_port_conf[l2port][msti].port_state_changed = FALSE;
            }

            new_state = ((xxrp_mstp_port_conf[l2port][msti].port_state == XXRP_MSTP_PORT_STATE_FORWARDING) ?
                         VTSS_MRP_MSTP_PORT_ADD : VTSS_MRP_MSTP_PORT_DELETE);

            XXRP_MSTP_PLATFORM_CRIT_EXIT();

            if (state_changed == TRUE) {
                T_DG(TRACE_GRP_PLATFORM, "MSTP state on l2 port %u, msti %u = %u ", l2port, msti, new_state);

                if ((brc = vtss_xxrp_mstp_port_state_change_handler(l2port, msti, new_state)) != VTSS_XXRP_RC_OK) {
                    T_EG(TRACE_GRP_PLATFORM, "Unable to set mstp state for port %u (brc %u)", l2port, brc);
                } /* if ((brc = vtss_mrp_mstp_port_state_change_handler(l2port, msti, new_state)) != VTSS_XXRP_RC_OK) */
            } /* if (xxrp_mstp_port_conf[l2port][msti].port_state_changed == TRUE) */
        } /* for (msti = 0; msti < N_L2_MSTI_MAX; msti++) */
    } /* for (l2port = 0; l2port <= L2_MAX_PORTS; l2port++) */
}

//#ifdef VTSS_SW_OPTION_MVRP
BOOL XXRP_mvrp_registrar_check_is_change_allowed(u32 port_no, vtss_vid_t vlan_id)
{
    vtss_isid_t              isid;
    vtss_port_no_t           iport_no;
    /* TODO: performance impact */
#if 0
    vlan_registration_type_t registrar_conf[VTSS_PORTS];
#endif

    if (l2port2port(port_no, &isid, &iport_no) != TRUE) {
        T_D("l2port2port failed");
        return FALSE;
    }
#if 0
    /* TODO: performance may be issue here - may need to change below function */
    if ((vlan_mgmt_vlan_registration_get(isid, vlan_id, registrar_conf, VLAN_USER_MVRP) == VTSS_OK) &&
        (registrar_conf[iport_no] == VLAN_REGISTRATION_TYPE_NORMAL)) {
        return TRUE;
    }
#endif
    return FALSE;
}

static u32  XXRP_vlan_port_membership_add(u32 port, vtss_vid_t vid, vlan_user_t vlan_user)
{
    //u32               registrar_mgt = 0;
    vtss_isid_t       isid;
    vtss_port_no_t    iport_no;
    vlan_mgmt_entry_t vlan_mgmt_entry;

    T_D("Enter VID: %d, port = %u", vid, port);
#if 0
    if (mvrp_mgmt_get_attribute_index(vid, &mad_index)) {
        T_D("mvrp_mgmt_get_attribute_index failed");
    }
    /* Check the registration Mgmt State of the vid */
    (void) mrp_mgmt_get_mad_registrar_admin_state (port_mad, mad_index, &registrar_mgt);

    if ( registrar_mgt == VTSS_MRP_REGISTRATION_FIXED) {
        /* You need not do anything here, simple return TRUE */
        return VTSS_RC_OK;
    }
#endif
    memset(&vlan_mgmt_entry, 0 , sizeof(vlan_mgmt_entry));

    if (l2port2port(port, &isid, &iport_no) != TRUE) {
        T_D("l2port2port failed");
        return FALSE;
    }

    if (vlan_mgmt_vlan_get(isid, vid, &vlan_mgmt_entry, FALSE, vlan_user) != VTSS_RC_OK) {
        T_D("vlan_mgmt_vlan_get failed");
    }
    vlan_mgmt_entry.vid = vid;           /* VLAN ID   */
    vlan_mgmt_entry.ports[iport_no] = 1; /* Port mask */
    if (vlan_mgmt_vlan_add(isid, &vlan_mgmt_entry, vlan_user) != VTSS_RC_OK) {
        T_D("vlan_mgmt_vlan_add failed");
    }
    T_D("Exit");

    return VTSS_RC_OK;
}








#ifdef VTSS_SW_OPTION_GVRP
u32  XXRP_gvrp_vlan_port_membership_add(u32 port, vtss_vid_t vid)
{
    u32 rc;

    rc = XXRP_vlan_port_membership_add(port, vid, VLAN_USER_GVRP);

    return rc;
}
#endif

static u32  XXRP_vlan_port_membership_del(u32 port, vtss_vid_t vid, vlan_user_t vlan_user)
{
    vtss_isid_t       isid;
    vtss_port_no_t    iport_no;
    vlan_mgmt_entry_t vlan_mgmt_entry;
    static const u8   empty_ports[VTSS_PORT_ARRAY_SIZE]; /* This will initialize empty_ports array to zeroes */

    T_D("Enter VID: %d, port = %u", vid, port);
    memset(&vlan_mgmt_entry, 0 , sizeof(vlan_mgmt_entry));

    if (l2port2port(port, &isid, &iport_no) != TRUE) {
        T_D("l2port2port failed");
        return FALSE;
    }

    if (vlan_mgmt_vlan_get(isid, vid, &vlan_mgmt_entry, FALSE, vlan_user) != VTSS_RC_OK) {
        T_D("vlan_mgmt_vlan_get failed");
    }
    vlan_mgmt_entry.vid = vid;              /* VLAN ID   */
    vlan_mgmt_entry.ports[iport_no] = 0;    /* Port mask */
    if (!memcmp(vlan_mgmt_entry.ports, empty_ports, sizeof(empty_ports))) {
        if (vlan_mgmt_vlan_del(isid, vid, vlan_user) != VTSS_RC_OK) {
            T_D("vlan_mgmt_vlan_del failed");
        }
    } else {
        if (vlan_mgmt_vlan_add(isid, &vlan_mgmt_entry, vlan_user) != VTSS_RC_OK) {
            T_D("vlan_mgmt_vlan_add failed");
        }
    }
    T_D("Exit");

    return VTSS_RC_OK;
}








#ifdef VTSS_SW_OPTION_GVRP
u32  XXRP_gvrp_vlan_port_membership_del(u32 port, vtss_vid_t vid)
{
    return XXRP_vlan_port_membership_del(port, vid, VLAN_USER_GVRP);
}
#endif

// Callback function for vlan module
static void XXRP_vlan_membership_change_callback(vtss_isid_t isid, vtss_vid_t vid, vlan_membership_change_t *changes)
{
    port_iter_t pit;

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        vlan_registration_type_t t;
        u32                      l2port = L2PORT2PORT(isid, pit.iport);

        if (!VTSS_BF_GET(changes->changed_ports.ports, pit.iport)) {
            // No changes on this port.
            continue;
        }

        if (VTSS_BF_GET(changes->forbidden_ports.ports, pit.iport)) {
            t = VLAN_REGISTRATION_TYPE_FORBIDDEN;
        } else {
            t = VTSS_BF_GET(changes->static_ports.ports, pit.iport) ? VLAN_REGISTRATION_TYPE_FIXED : VLAN_REGISTRATION_TYPE_NORMAL;
        }




#ifdef VTSS_SW_OPTION_GVRP
        (void)vtss_gvrp_registrar_administrative_control(l2port, vid, t);
#endif
    }
}

BOOL XXRP_mvrp_is_vlan_present(u32 port, vtss_vid_t vid)
{
    vtss_isid_t                 isid;
    vtss_port_no_t              iport_no;
    vlan_mgmt_entry_t           vlan_conf;

    if (l2port2port(port, &isid, &iport_no) != TRUE) {
        T_D("l2port2port failed");
        return FALSE;
    }
    if (vlan_mgmt_vlan_get(isid, vid, &vlan_conf, FALSE, VLAN_USER_STATIC) == VTSS_RC_OK) {
        if (vlan_conf.ports[iport_no] == 1) {
            T_N("vid = %u, isid = %u, port = %u", vid, isid, iport_no);
            return TRUE;
        } /* if (vlan_conf.ports[pit.iport]) */
    } /* while (vlan_mgmt_vlan_get(sit.isid, vid, &vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) */

    return FALSE;
}
#if 0
void XXRP_mvrp_initial_vlan_handler(vtss_isid_t isid, BOOL is_add)
{
    vtss_vid_t                  vid = 0;
    switch_iter_t               sit;
    port_iter_t                 pit;
    u32                         l2port;
    vlan_mgmt_entry_t           vlan_conf;

    (void) switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (!msg_switch_exists(sit.isid)) {
            continue;
        }
        while (vlan_mgmt_vlan_get(sit.isid, vid, &vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) {
            vid = vlan_conf.vid;
            (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (vlan_conf.ports[pit.iport]) {
                    T_N("vid = %u, ports[%lu] = %u", vid, pit.iport, vlan_conf.ports[pit.iport]);
                    l2port = L2PORT2PORT(sit.isid, pit.iport);
                    (void)vtss_mrp_vlan_change_handler(vid, l2port, is_add);
                } /* if (vlan_conf.ports[pit.iport]) */
            } /* while (port_iter_getnext(&pit)) */
        } /* while (vlan_mgmt_vlan_get(sit.isid, vid, &vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) */
    } /* while (switch_iter_getnext(&sit)) */
}
#endif
//#endif

BOOL XXRP_is_port_point2point(u32 port)
{
    vtss_isid_t       isid;
    vtss_port_no_t    iport_no;
    port_conf_t       conf;

    if (l2port2port(port, &isid, &iport_no) != TRUE) {
        T_D("l2port2port failed");
        return FALSE;
    }
    if (port_mgmt_conf_get(isid, iport_no, &conf) == VTSS_OK) {
        if (conf.fdx == TRUE) {
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************************************
 * XXRP_port_state_change()
 * Callback function for port state changes.
 * This is how we get the point-to-point (full duplex) state of a port.
 * Update the base module if the port is enabled.
 **************************************************************************************************/
static void XXRP_port_state_change(vtss_isid_t isid, vtss_port_no_t iport, port_info_t *info)
{
    u32 l2port = L2PORT2PORT(isid, iport);

    if (info->link) {
        XXRP_PLATFORM_CRIT_ENTER();
        T_DG(TRACE_GRP_PLATFORM, "Port state on l2 port %u = %s", l2port, info->fdx ? "p2p" : "shared");
        VTSS_PORT_BF_SET(XXRP_p2p_cache[isid - VTSS_ISID_START], iport, info->fdx); // Update the cache
        XXRP_PLATFORM_CRIT_EXIT();
    }
}

/***************************************************************************************************
 * XXRP_tm_thread()
 * The timer knows that the time resolution is 10mS (1cS).
 * The check below verifies that this is still true.
 **************************************************************************************************/
#if (ECOS_MSECS_PER_HWTICK != 10)
#error "Tick period not 10mS. Change code!"
#endif

static void XXRP_tm_thread(cyg_addrword_t data)
{
    for (;;) {
        if (msg_switch_is_master()) {
            // Initialize master state
            cyg_tick_count_t wakeup_prev = 0;
            cyg_tick_count_t wakeup_now  = 0;
            cyg_tick_count_t wakeup_next = 0; // 0 == no timeout
            uint             delay = 0;
            cyg_flag_value_t flags;

            while (msg_switch_is_master()) {
                // Process while being master
                if ((flags = XXRP_FLAG_WAIT(&XXRP_tm_thread_flag, wakeup_next))) {
                    if (flags & XXRP_EVENT_FLAG_DOWN) {
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_DOWN");
                        // No action
                    }
                    if (flags & XXRP_EVENT_FLAG_KICK) {
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_KICK");
                        wakeup_now = cyg_current_time();
                        delay = wakeup_next ?  wakeup_now - wakeup_prev : 0;
                        if ((delay = vtss_xxrp_timer_tick(delay))) {
                            T_IG(TRACE_GRP_PLATFORM, "Timer restarted (delay = %d)", delay);
                            wakeup_prev = wakeup_now;
                            wakeup_next = wakeup_now + delay; // Restart timer with new value
                        } else {
                            wakeup_next = 0; // Stop timer
                        }
                    }
                } else { // Timer expired
                    if ((delay = vtss_xxrp_timer_tick(delay))) {
                        T_IG(TRACE_GRP_PLATFORM, "Timer restarted (delay = %d)", delay);
                        wakeup_prev = wakeup_next;
                        wakeup_next += delay; // Restart timer with new value
                    } else {
                        T_IG(TRACE_GRP_PLATFORM, "Timer stopped");
                        wakeup_next = 0; // Stop timer
                    }
                }
            }
        }
        T_IG(TRACE_GRP_PLATFORM, "Suspending XXRP timer thread (became slave)");
        cyg_thread_suspend(XXRP_tm_thread_handle);
        T_IG(TRACE_GRP_PLATFORM, "Restarting XXRP timer thread (became master)");
    }
}

/***************************************************************************************************
 * XXRP_rx_thread()
 * Process received MRPDUs.
 **************************************************************************************************/
static void XXRP_rx_thread(cyg_addrword_t data)
{
    for (;;) {
        if (msg_switch_is_master()) {
            // Initialize master state
            cyg_tick_count_t wakeup_next = 0; // 0 == no timeout
            XXRP_PLATFORM_CRIT_ENTER();
            vtss_bip_buffer_clear(&XXRP_rx_buffer); // Start with a cleared buffer
            XXRP_PLATFORM_CRIT_EXIT();

            while (msg_switch_is_master()) {
                cyg_flag_value_t flags;
                // Process while beeing master
                if ((flags = XXRP_FLAG_WAIT(&XXRP_rx_thread_flag, wakeup_next))) {

                    if (flags & XXRP_EVENT_FLAG_DOWN) {
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_DOWN");
                        // No action
                    }

                    if (flags & XXRP_EVENT_FLAG_MRPDU) {
                        u32 *buf;
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_MRPDU");
                        do { // Process all received MRPDUs
                            int contiguous_block_size;
                            XXRP_PLATFORM_CRIT_ENTER();
                            buf = (u32 *)vtss_bip_buffer_get_contiguous_block(&XXRP_rx_buffer, &contiguous_block_size);
                            XXRP_PLATFORM_CRIT_EXIT();
                            if (buf) {
                                u32 total_len = buf[0]; // First dword is the buffer length in bytes including the length and the l2 port fields themselves.
                                u32 l2port    = buf[1]; // Second dword is the l2 port and the MRPDU starts in 4th dword
                                u32 pdu_len   = buf[2];
                                if ((total_len <= (3 * sizeof(u32))) || (total_len > (u32)contiguous_block_size)) {
                                    T_EG(TRACE_GRP_PLATFORM, "Invalid RX buffer entry total_len=%d contiguous_block_size=%d",
                                         (int)total_len, contiguous_block_size);
                                }
                                (void)vtss_mrp_mrpdu_rx(l2port, (u8 *)&buf[3], pdu_len);
                                XXRP_PLATFORM_CRIT_ENTER();
                                vtss_bip_buffer_decommit_block(&XXRP_rx_buffer, total_len);
                                XXRP_PLATFORM_CRIT_EXIT();
                            }
                        } while (buf);
                    }

                    if (flags & XXRP_EVENT_FLAG_MSTP_PORT_STATE_CHANGE) {
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_MSTP_PORT_STATE_CHANGE");
                        XXRP_mstp_state_change_handler();
                    }

                    if (flags & XXRP_EVENT_FLAG_MSTP_PORT_ROLE_CHANGE) {
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_MSTP_PORT_ROLE_CHANGE");
                    }//tftf

                    if (flags & XXRP_EVENT_FLAG_VLAN_2_MSTI_MAP_CHANGE) {
                        vtss_gvrp_update_vlan_to_msti_mapping();
                        T_IG(TRACE_GRP_PLATFORM, "Received XXRP_EVENT_FLAG_MSTP_PORT_ROLE_CHANGE");
                    }

                }
            }
        }
        T_IG(TRACE_GRP_PLATFORM, "Suspending XXRP rx thread (became slave)");
        cyg_thread_suspend(XXRP_rx_thread_handle);
        T_IG(TRACE_GRP_PLATFORM, "Restarting XXRP rx thread (became master)");
    }
}

/***************************************************************************************************
 * XXRP_check_isid_port()
 * Validate isid and port
 **************************************************************************************************/
static vtss_rc XXRP_check_isid_port(vtss_isid_t isid, vtss_port_no_t port, BOOL allow_local)
{
    if ((isid != VTSS_ISID_LOCAL && !VTSS_ISID_LEGAL(isid)) || (isid == VTSS_ISID_LOCAL && !allow_local)) {
        return XXRP_ERROR_ISID;
    }
    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        return XXRP_ERROR_NOT_MASTER;
    }
    if (port >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, port)) {
        return XXRP_ERROR_PORT;
    }
    return VTSS_OK;
}

/***************************************************************************************************
 *
 * PUBLIC FUNCTIONS
 *
 **************************************************************************************************/

/***************************************************************************************************
 * Callout function implementations
 **************************************************************************************************/

/***************************************************************************************************
 * vtss_mrp_timer_kick()
 * Called when the base module needs to (re)start the timer.
 **************************************************************************************************/
void vtss_mrp_timer_kick(void)
{
    T_N("Kick timer");
    cyg_flag_setbits(&XXRP_tm_thread_flag, XXRP_EVENT_FLAG_KICK);
}

/***************************************************************************************************
 * vtss_mrp_crit_enter()
 * Called when the base module wants to enter the critical section.
 **************************************************************************************************/
void vtss_mrp_crit_enter(vtss_mrp_appl_t app)
{
    // Avoid Lint warning: A thread mutex has been locked but not unlocked
    /*lint -e(454) */
    XXRP_BASE_CRIT_ENTER();
}

/***************************************************************************************************
 * vtss_mrp_crit_exit()
 * Called when the base module wants to exit the critical section.
 **************************************************************************************************/
void vtss_mrp_crit_exit(vtss_mrp_appl_t app)
{
    // Avoid Lint warning: A thread mutex that had not been locked is being unlocked
    /*lint -e(455) */
    XXRP_BASE_CRIT_EXIT();
}

/***************************************************************************************************
 * vtss_mrp_crit_assert_locked()
 * Called when the base module wants to verify that the critical section has been entered.
 **************************************************************************************************/
void vtss_mrp_crit_assert_locked(vtss_mrp_appl_t app)
{
    XXRP_BASE_CRIT_ASSERT_LOCKED();
}

/***************************************************************************************************
 * mrp_mstp_port_status_get()
 * Called when the base module wants to get the port msti status.
 **************************************************************************************************/
vtss_rc mrp_mstp_port_status_get(u8 msti, u32 l2port)
{
    mstp_port_mgmt_status_t ps;
    vtss_rc                 rc;

    memset(&ps, 0, sizeof(mstp_port_mgmt_status_t));
    if (mstp_get_port_status(msti, l2port, &ps) && ps.active &&
        (strncmp(ps.core.statestr, "Forwarding", sizeof("Forwarding")) == 0)) {
        rc = VTSS_RC_OK;
    } else {
        rc = VTSS_RC_ERROR;
    }

    return rc;
}

/* attr_index    :  attribute index. Actually VID.                            */
/* msti     :  msti information                                               */
/* Retrives the msti information for a given VLAN.                            */
vtss_rc mrp_mstp_index_msti_mapping_get(u32 attr_index, u8 *msti)
{
    mstp_msti_config_t msti_config;

    if (mstp_get_msti_config (&msti_config, NULL) == FALSE) {
        T_D("Unable to get vlan msti mapping\n\r");
        return VTSS_RC_ERROR;
    }
    *msti = msti_config.map.map[attr_index];
    return VTSS_RC_OK;
}

vtss_rc vtss_xxrp_port_mac_conf_get(u32 port_no, u8 *port_mac_addr)
{
    vtss_rc         rc = VTSS_RC_OK;
    vtss_isid_t     isid;
    vtss_port_no_t  switch_port;
    u8              sys_mac_addr[VTSS_XXRP_MAC_ADDR_LEN];

    do {
        if (port_mac_addr == NULL) {
            rc = XXRP_ERROR_VALUE;
            break;
        }
        if (l2port2port(port_no, &isid, &switch_port) == FALSE) {
            rc = XXRP_ERROR_VALUE;
            break;
        }
        //Get the system MAC address
        (void)conf_mgmt_mac_addr_get(sys_mac_addr, 0);
        misc_instantiate_mac(port_mac_addr, sys_mac_addr, switch_port + 1 - VTSS_PORT_NO_START);

    } while (0);

    return rc;

}
/***************************************************************************************************
 * vtss_mrp_mrpdu_tx_alloc()
 * Called when the base module wants to allocate a transmit buffer.
 **************************************************************************************************/
void *vtss_mrp_mrpdu_tx_alloc(u32 port_no, u32 length, vtss_mrp_tx_context_t *context)
{
    void *p;
    p = vtss_os_alloc_xmit(port_no, length, context);
    return p;
}

/***************************************************************************************************
 * vtss_mrp_mrpdu_tx_free()
 * Called when the base module wants to free a transmit buffer without transmitting it.
 * TBD: Consider to implement this functionality inside the l2proto module.
 **************************************************************************************************/
void vtss_mrp_mrpdu_tx_free(void *mrpdu, vtss_mrp_tx_context_t context)
{
    if (context) {
        VTSS_FREE(context);
    } else {
        packet_tx_free(mrpdu);
    }
}

/***************************************************************************************************
 * vtss_mrp_mrpdu_tx()
 * Called when the base module wants to transmit an MRPDU.
 * The SMAC address is inserted.
 **************************************************************************************************/
void dump_packet(int len, const u8 *p)
{
    int i;
#define BLINE 14
    for (i = 0; i < (len < BLINE ? len : BLINE); ++i) {
        printf("%2.2x ", p[i]);
    }

    for (i = BLINE; i < len; ++i) {
        if ( (i - BLINE) % 16 == 0 ) {
            printf("\n");
        }
        printf("%2.2x ", p[i]);
    }
    printf("\n");
}


BOOL vtss_mrp_mrpdu_tx(u32 port_no, void *mrpdu, u32 length, vtss_mrp_tx_context_t context)
{
    BOOL rc;
    u8 port_mac[VTSS_XXRP_MAC_ADDR_LEN];
    // TBD: Add SMAC address here!
    (void)vtss_xxrp_port_mac_conf_get(port_no, port_mac);
    memcpy(((u8 *)mrpdu + VTSS_XXRP_MAC_ADDR_LEN), port_mac, sizeof(port_mac));

    rc = vtss_os_xmit(port_no, mrpdu, length, context);

    return rc;
}

/***************************************************************************************************
 * xxrp_error_txt()
 * Converts XXRP error to printable text
 **************************************************************************************************/
char *xxrp_error_txt(vtss_rc rc)
{
    switch (rc) {
    case XXRP_ERROR_ISID:
        return "Invalid Switch ID";
    case XXRP_ERROR_PORT:
        return "Invalid port number";
    case XXRP_ERROR_FLASH:
        return "Could not store configuration in flash";
    case XXRP_ERROR_SLAVE:
        return "Could not get data from slave switch";
    case XXRP_ERROR_NOT_MASTER:
        return "Switch must be master";
    case XXRP_ERROR_VALUE:
        return "Invalid value";
    default:
        return "";
    }
}

vtss_rc xxrp_mgmt_global_enabled_get(vtss_mrp_appl_t appl, BOOL *enable)
{
    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    if (msg_switch_is_master()) {
        *enable = XXRP_stack_conf[appl].global_enable; // Get from master configuration
    } else {
        *enable = XXRP_local_conf[appl].global_enable; // Get from local configuration
    }
    XXRP_PLATFORM_CRIT_EXIT();

    return VTSS_OK;
}

vtss_rc xxrp_mgmt_global_enabled_set(vtss_mrp_appl_t appl, BOOL enable)
{
    vtss_rc rc = VTSS_OK;
    BOOL    changed;

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    if ((changed = (XXRP_stack_conf[appl].global_enable != enable))) {
        XXRP_stack_conf[appl].global_enable = enable;
        rc = XXRP_flash_write_specific(appl);
        XXRP_base_switch_sync_specific(VTSS_ISID_GLOBAL, appl);
    }
    XXRP_PLATFORM_CRIT_EXIT();

    if (changed) {
        XXRP_msg_tx_local_conf_specific(VTSS_ISID_GLOBAL, appl); // Send configuration to all existing switches
    }

    return rc;
}

vtss_rc xxrp_mgmt_enabled_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL *enable)
{
    vtss_rc rc;

    if ((rc = XXRP_check_isid_port(isid, iport, TRUE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    if (isid != VTSS_ISID_LOCAL) {
        *enable = VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport); // Get from master configuration
    } else {
        *enable = VTSS_PORT_BF_GET(XXRP_local_conf[appl].port_enable, iport); // Get from local configuration
    }
    XXRP_PLATFORM_CRIT_EXIT();

    return rc;
}

vtss_rc xxrp_mgmt_enabled_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL enable)
{
    vtss_rc rc = VTSS_OK;
    BOOL    changed;

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    if ((changed = (VTSS_PORT_BF_GET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport) != enable))) {
        VTSS_PORT_BF_SET(XXRP_stack_conf[appl].port_enable[isid - VTSS_ISID_START], iport, enable);
        rc = XXRP_flash_write_specific(appl);
        if (XXRP_stack_conf[appl].global_enable) {
            XXRP_base_port_sync_specific(isid, iport, appl);
        }
    }
    XXRP_PLATFORM_CRIT_EXIT();

    if (changed) {
        XXRP_msg_tx_local_conf_specific(VTSS_ISID_GLOBAL, appl); // Send configuration to all existing switches
    }

    return rc;
}

vtss_rc xxrp_mgmt_periodic_tx_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL *enable)
{
    vtss_rc rc;

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    *enable = VTSS_PORT_BF_GET(XXRP_stack_conf[appl].periodic_tx[isid - VTSS_ISID_START], iport);
    XXRP_PLATFORM_CRIT_EXIT();

    return rc;
}

vtss_rc xxrp_mgmt_periodic_tx_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL enable)
{
    vtss_rc rc = VTSS_OK;

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    if (VTSS_PORT_BF_GET(XXRP_stack_conf[appl].periodic_tx[isid - VTSS_ISID_START], iport) != enable) {
        VTSS_PORT_BF_SET(XXRP_stack_conf[appl].periodic_tx[isid - VTSS_ISID_START], iport, enable);
        rc = XXRP_flash_write_specific(appl);
        XXRP_base_port_periodic_tx_set_specific(isid, iport, appl, TRUE);
    }
    XXRP_PLATFORM_CRIT_EXIT();

    return rc;
}

vtss_rc xxrp_mgmt_timers_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_timer_conf_t *timers)
{
    vtss_rc rc;

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    *timers = XXRP_stack_conf[appl].timers[isid - VTSS_ISID_START][iport - VTSS_PORT_NO_START];
    XXRP_PLATFORM_CRIT_EXIT();

    return rc;
}

vtss_rc xxrp_mgmt_timers_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, const vtss_mrp_timer_conf_t *timers)
{
    vtss_rc rc = VTSS_OK;

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    XXRP_PLATFORM_CRIT_ENTER();
    XXRP_stack_conf[appl].timers[isid - VTSS_ISID_START][iport - VTSS_PORT_NO_START] = *timers;
    rc = XXRP_flash_write_specific(appl);
    XXRP_base_port_timer_set_specific(isid, iport, appl, TRUE);
    XXRP_PLATFORM_CRIT_EXIT();

    return rc;
}

vtss_rc xxrp_mgmt_applicant_adm_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_attribute_type_t attr_type, BOOL *participant)
{
    return VTSS_OK;
}

vtss_rc xxrp_mgmt_applicant_adm_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_attribute_type_t attr_type, BOOL participant)
{
    XXRP_base_port_applicant_adm_set_specific(isid, iport, appl, TRUE);
    return VTSS_OK;
}

vtss_rc xxrp_mgmt_port_stats_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_statistics_t *stats)
{
    vtss_rc rc;
    u32     l2port = L2PORT2PORT(isid, iport);

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    if (stats == NULL) {
        return XXRP_ERROR_VALUE;
    }

    if ((vtss_mrp_statistics_get(appl, l2port, stats)) != VTSS_XXRP_RC_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Unable to get statistics for appl %u port %u", appl, l2port);
        return XXRP_ERROR_VALUE;
    }

    return VTSS_XXRP_RC_OK;
}

vtss_rc xxrp_mgmt_port_stats_clear(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl)
{
    vtss_rc rc;
    u32     l2port = L2PORT2PORT(isid, iport);

    if ((rc = XXRP_check_isid_port(isid, iport, FALSE)) != VTSS_OK) {
        return rc;
    }

    if (appl >=  VTSS_MRP_APPL_MAX) {
        return XXRP_ERROR_VALUE;
    }

    if ((vtss_mrp_statistics_clear(appl, l2port)) != VTSS_XXRP_RC_OK) {
        T_EG(TRACE_GRP_PLATFORM, "Unable to clear statistics for appl %u port %u", appl, l2port);
        return XXRP_ERROR_VALUE;
    }

    return VTSS_XXRP_RC_OK;
}

vtss_rc xxrp_mgmt_vlan_state(vtss_common_port_t l2port, vlan_registration_type_t *array /* with VLAN_ID_MAX + 1 elements */)
{
    vtss_isid_t  isid;
    vtss_port_no_t port;
    vtss_rc rc = VTSS_RC_ERROR;

    if (l2port2port(l2port, &isid, &port)) {
        rc = vlan_mgmt_registration_per_port_get(isid, port, array);
    }

    if (rc != VTSS_RC_OK) {
        // Better clear it in case of errors if caller should happen to
        // attempt to interpret it despite the error return code.
        memset(array, 0, (VLAN_ID_MAX + 1) * sizeof(vlan_registration_type_t));
    }

    return rc;
}

#if 1 /* DEBUG */
vtss_rc xxrp_mgmt_print_connected_ring(u8 msti)
{
#ifdef VTSS_MRP_APPL_MVRP
    (void)vtss_mrp_port_ring_print(VTSS_MRP_APPL_MVRP, msti);
#endif
    return VTSS_OK;
}

vtss_rc xxrp_mgmt_pkt_dump_set(BOOL pkt_control)
{
    xxrp_pkt_dump_set(pkt_control);
    return VTSS_OK;
}

vtss_rc xxrp_mgmt_mad_port_print(vtss_isid_t isid, vtss_port_no_t iport, u32 machine_index)
{
    u32 l2port = L2PORT2PORT(isid, iport);
    vtss_mrp_port_mad_print(l2port, machine_index);
    return VTSS_OK;
}
#endif


#if defined(VTSS_SW_OPTION_GVRP) && defined(VTSS_SW_OPTION_ICFG)
/*
 * Functions for generating running-config
 */
static vtss_rc vtss_icfg_query_func_gvrp_global(const vtss_icfg_query_request_t *req,
                                                vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_RC_OK;
    BOOL enable;

    // --- Get GVRP global enable state
    if ( (rc = xxrp_mgmt_global_enabled_get(VTSS_GARP_APPL_GVRP, &enable)) ) {
        return rc;
    }

    if (enable) {
        rc = vtss_icfg_printf(result, "gvrp max-vlans %d\n", vtss_gvrp_max_vlans() );
    } else {
        if (req->all_defaults) {
            rc = vtss_icfg_printf(result, "no gvrp\n");
        }
    }

    // --- Get protocol timers
    if (enable || req->all_defaults) {
        rc = vtss_icfg_printf(result, "%sgvrp time join-time %d leave-time %d leave-all-time %d\n", enable ? "" : "no ",
                              vtss_gvrp_get_timer(GARP_TC__transmitPDU),
                              vtss_gvrp_get_timer(GARP_TC__leavetimer),
                              vtss_gvrp_get_timer(GARP_TC__leavealltimer));
    }

    return rc;
}

static vtss_rc vtss_icfg_query_func_gvrp_per_interface(const vtss_icfg_query_request_t *req,
                                                       vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_RC_OK;
    BOOL enable;

    const icli_switch_port_range_t *plist = &req->instance_id.port;

    u16 isid = plist->isid;

    u16 portBegin = plist->begin_iport;
    u16 portEnd = portBegin + plist->port_cnt;
    u16 iport;

    for (iport = portBegin; iport < portEnd; ++iport) {

        if ( (rc = xxrp_mgmt_enabled_get(isid, iport, VTSS_GARP_APPL_GVRP, &enable)) ) {
            return rc == XXRP_ERROR_PORT ? VTSS_RC_OK : rc;     // Ignore stack port errors
        }

        if (enable || req->all_defaults) {
            rc = vtss_icfg_printf(result, " %sgvrp\n", enable ? "" : "no ");
        }

    }

    return rc;
}
#endif /* defined(VTSS_SW_OPTION_GVRP) && defined(VTSS_SW_OPTION_ICFG) */


static void XXRP_mstp_register_config_change_cb(void)
{
    cyg_flag_setbits(&XXRP_rx_thread_flag, XXRP_EVENT_FLAG_VLAN_2_MSTI_MAP_CHANGE);
}


/***************************************************************************************************
 * xxrp_init()
 **************************************************************************************************/
vtss_rc xxrp_init(vtss_init_data_t *data)
{
    vtss_isid_t               isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }
    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_DG(TRACE_GRP_PLATFORM, "INIT");

#ifdef VTSS_SW_OPTION_VCLI



#endif

        // Create the rx buffer for received MRPDUs (master only)
        if (!vtss_bip_buffer_init(&XXRP_rx_buffer, XXRP_RX_BUFFER_SIZE_BYTES)) {
            T_EG(TRACE_GRP_PLATFORM, "Unable to create rx buffer");
        }
        // Create and release base crit
        critd_init(&XXRP_base_crit, "crit_base", VTSS_MODULE_ID_XXRP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        XXRP_BASE_CRIT_EXIT();

        // Create and release platform crit
        critd_init(&XXRP_platform_crit, "crit_platform", VTSS_MODULE_ID_XXRP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        XXRP_PLATFORM_CRIT_EXIT();

        // Create and release mstp platform crit
        critd_init(&XXRP_mstp_platform_crit, "crit_xxrp_mstp_platform", VTSS_MODULE_ID_XXRP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        XXRP_MSTP_PLATFORM_CRIT_EXIT();
        //Initialize the data structures.
        vtss_mrp_init();

        // Create timer thread related stuff
        cyg_flag_init(&XXRP_tm_thread_flag);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          XXRP_tm_thread,
                          0,
                          "XXRP Timer",
                          XXRP_tm_thread_stack,
                          sizeof(XXRP_tm_thread_stack),
                          &XXRP_tm_thread_handle,
                          &XXRP_tm_thread_state);

        // Create rx thread related stuff
        cyg_flag_init(&XXRP_rx_thread_flag);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          XXRP_rx_thread,
                          0,
                          "XXRP RX",
                          XXRP_rx_thread_stack,
                          sizeof(XXRP_rx_thread_stack),
                          &XXRP_rx_thread_handle,
                          &XXRP_rx_thread_state);
        break;
    case INIT_CMD_START:
        T_DG(TRACE_GRP_PLATFORM, "START");
        if (l2_stp_msti_state_change_register(XXRP_mstp_state_change) != VTSS_OK) {
            T_EG(TRACE_GRP_PLATFORM, "l2_stp_msti_state_change_register failed");
        }
        if (port_global_change_register(VTSS_MODULE_ID_XXRP, XXRP_port_state_change) != VTSS_OK) {
            T_EG(TRACE_GRP_PLATFORM, "port_global_change_register failed");
        }

        XXRP_msg_register();
        l2_receive_register(VTSS_MODULE_ID_XXRP, XXRP_l2_rx);
        XXRP_packet_register();

#ifdef VTSS_SW_OPTION_GVRP
        /* VLAN config change register */
        // vlan_change_mvrp_register(XXRP_mvrp_vlan_change_callback);
        vlan_membership_change_register(VTSS_MODULE_ID_XXRP, XXRP_vlan_membership_change_callback);
#if defined(VTSS_SW_OPTION_ICFG)
        (void)vtss_icfg_query_register(VTSS_ICFG_GVRP,
                                       "GVRP",
                                       vtss_icfg_query_func_gvrp_global);

        (void)vtss_icfg_query_register(VTSS_ICFG_GVRP_INTERFACE_CONF,
                                       "GVRP",
                                       vtss_icfg_query_func_gvrp_per_interface);
#endif /* defined(VTSS_SW_OPTION_ICFG) */
#endif
        break;
    case INIT_CMD_CONF_DEF:
        T_DG(TRACE_GRP_PLATFORM, "CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_GLOBAL) { // Reset global configuration (no local or per switch configuration here)
            XXRP_PLATFORM_CRIT_ENTER();
            XXRP_flash_read(TRUE);
            XXRP_base_switch_sync(VTSS_ISID_GLOBAL); // Synchronize base module with actual configuration for all existing switches
            XXRP_PLATFORM_CRIT_EXIT();
            XXRP_msg_tx_local_conf(VTSS_ISID_GLOBAL); // Send configurations to all existing switches
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_DG(TRACE_GRP_PLATFORM, "MASTER_UP");
        /* Read stack configuration */
        XXRP_PLATFORM_CRIT_ENTER();
        XXRP_flash_read(FALSE);
        XXRP_PLATFORM_CRIT_EXIT();
        // Resume threads for master processing
        cyg_thread_resume(XXRP_tm_thread_handle);
        cyg_thread_resume(XXRP_rx_thread_handle);

        if (!mstp_register_config_change_cb(XXRP_mstp_register_config_change_cb)) {
            T_EG(TRACE_GRP_PLATFORM, "mstp_register_config_change_cb failed in MASTER_UP");
        }

        break;
    case INIT_CMD_MASTER_DOWN:
        T_DG(TRACE_GRP_PLATFORM, "MASTER_DOWN");

        if (!mstp_register_config_change_cb(NULL)) {
            T_EG(TRACE_GRP_PLATFORM, "mstp_register_config_change_cb failed in MASTER_DOWN");
        }

        // Wake up threads for master down processing
        cyg_flag_setbits(&XXRP_tm_thread_flag, XXRP_EVENT_FLAG_DOWN);
        cyg_flag_setbits(&XXRP_rx_thread_flag, XXRP_EVENT_FLAG_DOWN);
        XXRP_PLATFORM_CRIT_ENTER();
        XXRP_base_master_down(); // Delete all MRP applications.
        XXRP_PLATFORM_CRIT_EXIT();
        break;
    case INIT_CMD_SWITCH_ADD:
        T_DG(TRACE_GRP_PLATFORM, "SWITCH_ADD, isid: %d", isid);
        XXRP_msg_tx_local_conf(isid); // Send configurations to new switch
        XXRP_PLATFORM_CRIT_ENTER();
        XXRP_base_switch_sync(isid); // Synchronize base module with actual configuration for this switch
        XXRP_PLATFORM_CRIT_EXIT();
        break;
    case INIT_CMD_SWITCH_DEL:
        T_DG(TRACE_GRP_PLATFORM, "SWITCH_DEL, isid: %d", isid);
        XXRP_PLATFORM_CRIT_ENTER();
        XXRP_base_switch_del(isid); // Disable all MRP enabled ports on all enabled MRP applications.
        XXRP_PLATFORM_CRIT_EXIT();
        break;
    default:
        break;
    }

    T_DG(TRACE_GRP_PLATFORM, "exit");
    return VTSS_OK;
}
