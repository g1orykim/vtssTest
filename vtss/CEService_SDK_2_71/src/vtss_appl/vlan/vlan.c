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

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vlan_api.h"
#include "port_api.h"   /* For a.o. PORT_NO_STACK_0 */
#include "misc_api.h"
#if defined(VTSS_SW_OPTION_SYSLOG)
#include "syslog_api.h" /* For definition of S_E()  */
#endif /* defined(VTSS_SW_OPTION_SYSLOG) */
#if defined(VTSS_SW_OPTION_VCLI)
#include "vlan_cli.h"
#endif /* defined(VTSS_SW_OPTION_VCLI) */
#include "mgmt_api.h"
#if defined(VTSS_SW_OPTION_ICFG)
#include "vlan_icfg.h"
#endif /* defined(VTSS_SW_OPTION_ICFG) */
#include "vlan_trace.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_VLAN

#undef VLAN_VOLATILE_USER_PRESENT /* Prevent it from being set from outside */
#if defined(VTSS_SW_OPTION_DOT1X)      || defined(VTSS_SW_OPTION_MSTP) || \
    defined(VTSS_SW_OPTION_MVR)        || defined(VTSS_SW_OPTION_MVRP) || \
    defined(VTSS_SW_OPTION_VOICE_VLAN) || defined(VTSS_SW_OPTION_ERPS) || \
    defined(VTSS_SW_OPTION_MEP)        || defined(VTSS_SW_OPTION_EVC)  || \
    defined(VTSS_SW_OPTION_VCL)
#define VLAN_VOLATILE_USER_PRESENT
#endif /* defined(...) || defined(...) */

#undef VLAN_SAME_USER_SUPPORT /* Prevent it from being set from outside */
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
#define VLAN_SAME_USER_SUPPORT
#endif

#undef VLAN_SINGLE_USER_SUPPORT /* Prevent it from being set from outside */
#if defined(VTSS_SW_OPTION_DOT1X)
#define VLAN_SINGLE_USER_SUPPORT
#endif /* VTSS_SW_OPTION_DOT1X */

#define VLAN_PORT_BLK_VERSION                 0
#define VLAN_NAME_BLK_VERSION                 0
#define VLAN_MEMBERSHIP_BLK_VERSION           1
#define VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION 1

// Enumerate the modules requiring this feature.
// Multi VLAN users can add multiple VLANs and specify
// exactly which ports are members of which VLANs.
typedef enum {
    VLAN_MULTI_STATIC,



#ifdef VTSS_SW_OPTION_GVRP
    VLAN_MULTI_GVRP,
#endif
#ifdef VTSS_SW_OPTION_MEP
    VLAN_MULTI_MEP,
#endif
#ifdef VTSS_SW_OPTION_EVC
    VLAN_MULTI_EVC,
#endif
#ifdef VTSS_SW_OPTION_MVR
    VLAN_MULTI_MVR,
#endif
    VLAN_MULTI_CNT  /**< This must come last */
} multi_user_index_t;

// "Same" VLAN users can configure ports to be members of at most one VLAN.
typedef enum {
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    VLAN_SAME_VOICE_VLAN,
#endif
    VLAN_SAME_CNT
} same_user_index_t;

// "Single" VLAN users can configure different ports to be members
// of different VLANs, but at most one VLAN per port.
typedef enum {
#ifdef VTSS_SW_OPTION_DOT1X
    VLAN_SINGLE_DOT1X,
#endif
    VLAN_SINGLE_CNT
} single_user_index_t;

typedef struct {
    vtss_vid_t vid;                                     /* VLAN ID          */
    u8         ports[VTSS_ISID_END][VTSS_PORT_BF_SIZE]; /* Global Port mask */
} vlan_flash_entry_t;

typedef struct {
    vtss_vid_t   vid;   /* VLAN ID   */
    vlan_ports_t entry; /* Port mask */
} vlan_entry_single_switch_with_vid_t;

#ifdef VTSS_SW_OPTION_VLAN_NAMING
typedef struct {
    u16 vid;                     /* VLAN ID            */
    i8  name[VLAN_NAME_MAX_LEN]; /* VLAN Name if valid */
} vlan_name_conf_t;
#endif

/* VLAN single membership configuration table */
typedef struct {
    // A VID = VTSS_VID_NULL indicates that this [isid][port] is not a member of any VLAN.
    // The [isid] index is zero-based (i.e. idx 0 == VTSS_ISID_START).
    vtss_vid_t vid[VTSS_ISID_CNT][VTSS_PORTS];
} vlan_single_membership_entry_t;

/* VLAN membership configuration table */
typedef struct {
    u32                version;               /* Block version                 */
    u32                count;                 /* Number of entries             */
    u32                size;                  /* Size of each entry            */
    vlan_flash_entry_t table[VLAN_ENTRY_CNT]; /* Entries                       */
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_etype_t       etype;                 /* Ether Type for Custom S-ports */
#endif
} vlan_flash_table_blk_t;

/* VLAN forbidden table */
typedef struct {
    u32                version;               /* Block version      */
    u32                count;                 /* Number of entries  */
    u32                size;                  /* Size of each entry */
    vlan_flash_entry_t table[VLAN_ENTRY_CNT]; /* Entries            */
} vlan_flash_forbidden_table_blk_t;

/* VLAN port configuration table */
typedef struct {
    u32              version;                         /* Block version */
    vlan_port_conf_t conf[VTSS_ISID_CNT][VTSS_PORTS]; /* Entries */
} vlan_flash_port_blk_t;

typedef struct {
    // Zero-based ISIDs.
    u8 ports[VTSS_ISID_CNT][VTSS_PORT_BF_SIZE];
} vlan_entry_t;

typedef struct {
    vtss_vid_t vid;
    vlan_entry_t entry;
} vlan_entry_with_vid_t;

/* VLAN entry */
typedef struct vlan_list_entry_t {
    struct vlan_list_entry_t *next;
    vlan_entry_t user_entries[VLAN_MULTI_CNT];
} vlan_list_entry_t;

/* ================================================================= *
 *  VLAN stack messages
 * ================================================================= */

/* VLAN messages IDs */
typedef enum {
    VLAN_MSG_ID_MEMBERSHIP_SET_REQ,       /* VLAN memberships configuration set request for all VIDs */
    VLAN_MSG_ID_PORT_CONF_ALL_SET_REQ,    /* VLAN port configuration set request for all ports       */
    VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ, /* VLAN port configuration set request for single port     */
    VLAN_MSG_ID_CONF_TPID_REQ             /* Ethertype for Custom S-ports set                        */
} vlan_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    vlan_msg_id_t msg_id;

    union {

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        /* VLAN_MSG_ID_CONF_TPID_REQ */
        struct {
            vtss_etype_t tpid;
        } custom_s;
#endif

        /* VLAN_MSG_ID_PORT_CONF_SET_REQ */
        struct {
            vlan_port_conf_t conf[VTSS_PORTS];
        } port_conf_all;

        /* VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ */
        struct {
            vtss_port_no_t   port; // Port to apply port configuration to
            vlan_port_conf_t conf; // Port configuration
        } port_conf_single;
    } req;
} vlan_msg_req_t;

/* Large request message */
typedef struct {
    /* Message ID */
    vlan_msg_id_t msg_id;

    union {
        /* VLAN_MSG_ID_CONF_SET_REQ */
        struct {
            /**
             * When set, all current VLANs are removed. This is useful during
             * switch add events.
             */
            BOOL flush;

            /**
             * Number of valid entries in #table.
             */
            u32 cnt;

            /**
             * We must be able to send all VLANs in one go, regardless
             * of VLAN_ENTRY_CNT, because it could be that some user
             * modules have configured VLANs that are not saved to flash.
             * Hence, table[VLAN_ID_MAX + 1] rather than table[VLAN_ENTRY_CNT]
             */
            vlan_entry_single_switch_with_vid_t table[VLAN_ID_MAX + 1];
        } set;
    } large_req;
} vlan_msg_large_req_t;

/* Request buffer message pool */
static void *VLAN_request_pool;

/* Request buffer pool for large requests */
static void *VLAN_large_request_pool;

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
#define VLAN_NAME_ENTRY_CNT 64 /**< Will go away in future versions */
/****************************************************************************/
// vlan_name_table_blk_t
/****************************************************************************/
typedef struct {
    u32              version;                    // Version of this structure as read/saved from/to flash.
    u32              count;                      // Number of entries
    u32              size;                       // Size of each entry
    vlan_name_conf_t table[VLAN_NAME_ENTRY_CNT]; // VLAN Name table
} vlan_name_table_blk_t;
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

/****************************************************************************/
// Global variables
/****************************************************************************/

// Structure holding port-conf-change callback registrants
typedef struct {
    vtss_module_id_t                 modid;        // Identifies module (debug purposes)
    vlan_port_conf_change_callback_t cb;           // Callback function
    BOOL                             cb_on_master; // Callback on local switch or on master?
} vlan_port_conf_change_cb_conf_t;

// Global structure
static vlan_port_conf_change_cb_conf_t VLAN_pcc_cb[4]; // Shorthand for "port-conf-change-callback"

typedef struct {
    vtss_module_id_t modid; // Identifies module (debug purposes)

    union {
        vlan_membership_change_callback_t        cb; // Non-bulk callback function
        vlan_membership_bulk_change_callback_t  bcb; // Bulk callback function
        vlan_s_custom_etype_change_callback_t   scb; // S-custom EtherType change callback function
    } u;
} vlan_change_cb_conf_t;

static vlan_change_cb_conf_t VLAN_mc_cb[5];             // Shorthand for "membership-change-callback"
static vlan_change_cb_conf_t VLAN_mc_bcb[2];            // Shorthand for "membership-change-bulk-callback"
static vlan_change_cb_conf_t VLAN_s_custom_etype_cb[2];

/**
 * Points to a list of unused vlan_list_entry_t items.
 */
static vlan_list_entry_t *VLAN_free_list;

/**
 * Storage area for the VLANs that can be configured.
 * There are VLAN_ENTRY_CNT such VLANs, not to be confused
 * with the VLAN IDs that can be configured.
 * During initialization, it is stiched together and
 * a pointer to the first item is stored in #VLAN_free_list,
 * and this table is therefore not directly referred to anymore.
 *
 * The entries are moved back and forth between VLAN_free_list
 * and VLAN_multi_table.
 */
vlan_list_entry_t VLAN_table[VLAN_ENTRY_CNT];
static vlan_list_entry_t *VLAN_multi_table[VLAN_ID_MAX + 1];

// All VIDs (not just VLAN_ENTRY_CNT) must be available in the forbidden list.
static vlan_entry_t VLAN_forbidden_table[VLAN_ID_MAX + 1];

// All VIDs (not just VLAN_ENTRY_CNT) must be available in the combined list.
static vlan_entry_t VLAN_combined_table[VLAN_ID_MAX + 1];

#ifdef VLAN_SAME_USER_SUPPORT
// "Same" VLAN users can configure exactly one VLAN.
static vlan_entry_with_vid_t VLAN_same_table[VLAN_SAME_CNT];
#endif /* VLAN_SAME_USER_SUPPORT */

#ifdef VLAN_SINGLE_USER_SUPPORT
static vlan_single_membership_entry_t VLAN_single_table[VLAN_SINGLE_CNT];
#endif /* VLAN_SINGLE_USER_SUPPORT */

// Allowed VIDs.
// Index 0 is for VLAN_PORT_MODE_TRUNK, index 1 is for VLAN_PORT_MODE_HYBRID
static u8 VLAN_allowed_vids[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE][2][VLAN_BITMASK_LEN_BYTES];

/**
  * This array will store all the VLAN Users port
  * configuration. Notice that one extra VLAN user entry is added here:
  * currently configured port properties are stored in the
  * VLAN_port_conf[VLAN_USER_ALL] entry.
  */
static vlan_port_conf_t VLAN_port_conf[VLAN_USER_CNT][VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];

// ICLI support
static vlan_port_composite_conf_t VLAN_port_composite_conf[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
static vtss_etype_t VLAN_tpid_s_custom_port; /* EtherType for Custom S-ports */
#endif

#ifdef VTSS_SW_OPTION_VLAN_NAMING
/**
 * VLAN names.
 *
 * By default, only VLAN_name_conf[VLAN_ID_DEFAULT] has a non-standard name ("default").
 *
 * All other VLANs have default names, which are "VLANxxxx", where
 * "xxxx" represent four numeric digits (with leading zeroes) equal to the VLAN ID.
 *
 * An empty string indicates that the VLAN has its default name.
 *
 * The code takes care of disallowing changing the name of e.g VID 7 to VLAN0003
 */
static char VLAN_name_conf[VLAN_ID_MAX + 1][VLAN_NAME_MAX_LEN];
#endif

/**
 * This array holds which VLANs the static user has enabled.
 * It's in order to be able to distinguish automatically
 * added memberships due to trunk/hybrid ports from
 * statically added (ICLI: vlan <vlan_list>).
 */
static u8 VLAN_end_user_enabled_vlans[VLAN_BITMASK_LEN_BYTES];

/**
 * The following array tells whether a given user has
 * added non-zero memberships to a VID on a given switch.
 * Use VTSS_BF_GET()/VTSS_BF_SET() operations on the array.
 */
static u8 VLAN_non_zero_membership[VLAN_ID_MAX + 1][VLAN_USER_CNT][VTSS_BF_SIZE(VTSS_ISID_CNT)];

/**
 * This structure is used to capture multiple changes
 * to VLANs in order to minimize the number of calls
 * into VLAN membership subscribers and to msg_tx()
 * with new VLAN memberships.
 *
 * The structure is protected by VLAN_crit.
 */
static struct {
    /**
     * Reference counts VLAN_bulk_begin()/VLAN_bulk_end() calls.
     * When non-zero, all updates are cached.
     * When it goes from 1 to 0, changes are applied and subscribers are called back.
     */
    u32 ref_cnt;

    /**
     * Zero-based indexed per-switch array, holding infor
     * about changes and who to send them to.
     */
    struct {
        /**
         * This tells whether at least one VLAN has changed for this switch.
         * Used when notifying subscribers.
         */
        BOOL dirty;

        /**
         * The following is used when notifying subscribers.
         * It contains a bit for every *possibly* changed VLAN.
         * Due to the re-entrancy nature of the VLAN module, it might be
         * that no changes have really occurred even when a bit in this array is 1.
         */
        u8 dirty_vids[VLAN_BITMASK_LEN_BYTES];

        /**
         * The following is used when notifying subscribers.
         * It contains the membership as they were last time
         * the subscribers were called back.
         *
         * Index 0 == VLAN_USER_STATIC, index 1 == VLAN_USER_FORBIDDEN
         */
        vlan_ports_t old_members[2][VLAN_ID_MAX + 1]; // Ditto

        /**
         * The following are used when figuring out whether to transmit configuration to given switches
         * tx_conf[isid][VTSS_VID_NULL] == 1 indicates that at least one bit is set in the remainder.
         */
        u8 tx_conf[VLAN_BITMASK_LEN_BYTES];

    } s[VTSS_ISID_CNT]; // Zero-based.

    /**
     * The following tells a given switch whether it should flush all entries
     * before applying new. Will be TRUE only upon SWITCH_ADD events.
     */
    BOOL flush[VTSS_ISID_CNT];

} VLAN_bulk;

typedef enum {
    VLAN_BIT_OPERATION_OVERWRITE,
    VLAN_BIT_OPERATION_ADD,
    VLAN_BIT_OPERATION_DEL,
} vlan_bit_operation_t;

static critd_t VLAN_crit;
static critd_t VLAN_cb_crit;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "vlan",
    .descr     = "VLAN table"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CB] = {
        .name      = "callback",
        .descr     = "VLAN Callback",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CLI] = {
        .name      = "CLI",
        .descr     = "Command line interface",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};

#define VLAN_CRIT_ENTER()            critd_enter(        &VLAN_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VLAN_CRIT_EXIT()             critd_exit(         &VLAN_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VLAN_CRIT_ASSERT_LOCKED()    critd_assert_locked(&VLAN_crit,    TRACE_GRP_CRIT,                        __FUNCTION__, __LINE__)
#define VLAN_CB_CRIT_ENTER()         critd_enter(        &VLAN_cb_crit, TRACE_GRP_CB,   VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VLAN_CB_CRIT_EXIT()          critd_exit(         &VLAN_cb_crit, TRACE_GRP_CB,   VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VLAN_CB_CRIT_ASSERT_LOCKED() critd_assert_locked(&VLAN_cb_crit, TRACE_GRP_CB,                          __FUNCTION__, __LINE__)
#else
#define VLAN_CRIT_ENTER()            critd_enter(        &VLAN_crit)
#define VLAN_CRIT_EXIT()             critd_exit(         &VLAN_crit)
#define VLAN_CRIT_ASSERT_LOCKED()    critd_assert_locked(&VLAN_crit)
#define VLAN_CB_CRIT_ENTER()         critd_enter(        &VLAN_cb_crit)
#define VLAN_CB_CRIT_EXIT()          critd_exit(         &VLAN_cb_crit)
#define VLAN_CB_CRIT_ASSERT_LOCKED() critd_assert_locked(&VLAN_cb_crit)
#endif /* VTSS_TRACE_ENABLED */

// VLAN_IN_USE_ON_SWITCH_GET() may be invoked with #vid == VTSS_VID_NULL, but
// VLAN_IN_USE_ON_SWITCH_SET() MUST NOT be invoked with #vid == VTSS_VID_NULL.
#define VLAN_IN_USE_ON_SWITCH_GET(_isid_, _vid_, _user_)        VTSS_BF_GET(VLAN_non_zero_membership[_vid_][_user_], (_isid_) - VTSS_ISID_START)
#define VLAN_IN_USE_ON_SWITCH_SET(_isid_, _vid_, _user_, _val_) VTSS_BF_SET(VLAN_non_zero_membership[_vid_][_user_], (_isid_) - VTSS_ISID_START, _val_)

/******************************************************************************/
// Various local functions
/******************************************************************************/

/******************************************************************************/
// VLAN_user_to_multi_idx()
// If not a valid multi-VLAN user, returns VLAN_MULTI_CNT.
/******************************************************************************/
static inline multi_user_index_t VLAN_user_to_multi_idx(vlan_user_t user)
{
    switch (user) {
    case VLAN_USER_STATIC:
        return VLAN_MULTI_STATIC;




#if defined(VTSS_SW_OPTION_GVRP)
    case VLAN_USER_GVRP:
        return VLAN_MULTI_GVRP;
#endif /* defined(VTSS_SW_OPTION_GVRP) */
#if defined(VTSS_SW_OPTION_MEP)
    case VLAN_USER_MEP:
        return VLAN_MULTI_MEP;
#endif /* defined(VTSS_SW_OPTION_MEP) */
#if defined(VTSS_SW_OPTION_EVC)
    case VLAN_USER_EVC:
        return VLAN_MULTI_EVC;
#endif /* defined(VTSS_SW_OPTION_EVC) */
#if defined(VTSS_SW_OPTION_MVR)
    case VLAN_USER_MVR:
        return VLAN_MULTI_MVR;
#endif /* defined(VTSS_SW_OPTION_MVR) */
    default:
        return VLAN_MULTI_CNT;
    }
}

/******************************************************************************/
// VLAN_multi_idx_to_user()
/******************************************************************************/
static inline vlan_user_t VLAN_multi_idx_to_user(multi_user_index_t multi_idx)
{
    switch (multi_idx) {
    case VLAN_MULTI_STATIC:
        return VLAN_USER_STATIC;




#if defined(VTSS_SW_OPTION_GVRP)
    case VLAN_MULTI_GVRP:
        return VLAN_USER_GVRP;
#endif /* defined(VTSS_SW_OPTION_GVRP) */
#if defined(VTSS_SW_OPTION_MEP)
    case VLAN_MULTI_MEP:
        return VLAN_USER_MEP;
#endif /* defined(VTSS_SW_OPTION_MEP) */
#if defined(VTSS_SW_OPTION_EVC)
    case VLAN_MULTI_EVC:
        return VLAN_USER_EVC;
#endif /* defined(VTSS_SW_OPTION_EVC) */
#if defined(VTSS_SW_OPTION_MVR)
    case VLAN_MULTI_MVR:
        return VLAN_USER_MVR;
#endif /* defined(VTSS_SW_OPTION_MVR) */
    default:
        return VLAN_USER_CNT;
    }
}

#if defined(VLAN_SAME_USER_SUPPORT)
/******************************************************************************/
// VLAN_user_to_same_idx()
// If not a valid same-VLAN user, returns VLAN_SAME_CNT.
/******************************************************************************/
static inline same_user_index_t VLAN_user_to_same_idx(vlan_user_t user)
{
    switch (user) {
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
    case VLAN_USER_VOICE_VLAN:
        return VLAN_SAME_VOICE_VLAN;
#endif /* defined(VTSS_SW_OPTION_VOICE_VLAN) */
    default:
        return VLAN_SAME_CNT;
    }
}
#endif /* defined(VLAN_SAME_USER_SUPPORT) */

#if defined(VLAN_SINGLE_USER_SUPPORT)
/******************************************************************************/
// VLAN_user_to_single_idx()
// If not a valid single-VLAN user, returns VLAN_SINGLE_CNT.
/******************************************************************************/
static inline single_user_index_t VLAN_user_to_single_idx(vlan_user_t user)
{
    switch (user) {
#if defined(VTSS_SW_OPTION_DOT1X)
    case VLAN_USER_DOT1X:
        return VLAN_SINGLE_DOT1X;
#endif /* defined(VTSS_SW_OPTION_DOT1X) */
    default:
        return VLAN_SINGLE_CNT;
    }
}
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

/******************************************************************************/
// VLAN_port_conf_change_callback()
/******************************************************************************/
static void VLAN_port_conf_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *conf)
{
    vlan_port_conf_change_cb_conf_t local_cb[ARRSZ(VLAN_pcc_cb)];
    u32                             i;

    VLAN_CB_CRIT_ENTER();
    // Take a copy in order to avoid deadlock issues
    memcpy(local_cb, VLAN_pcc_cb, sizeof(local_cb));
    VLAN_CB_CRIT_EXIT();

    for (i = 0; i < ARRSZ(local_cb); i++) {
        if (local_cb[i].cb == NULL) {
            // Since there is no un-register support,
            // we know that there are no more registrants
            // when we meet a NULL pointer.
            break;
        }

        if (isid == VTSS_ISID_LOCAL) {
            // Local switch change
            if (local_cb[i].cb_on_master) {
                // Not interested in getting called back
                continue;
            }
        } else {
            // Master switch change
            if (!local_cb[i].cb_on_master) {
                // Not interested in getting called back
                continue;
            }
        }

        T_DG(TRACE_GRP_CB, "%u:%u: Calling back %s", isid, port_no, vtss_module_names[local_cb[i].modid]);
        local_cb[i].cb(isid, port_no, conf);
    }
}

/******************************************************************************/
// VLAN_msg_alloc()
// Allocate message buffer from normal request pool.
/******************************************************************************/
static vlan_msg_req_t *VLAN_msg_alloc(vlan_msg_id_t msg_id, u32 ref_cnt)
{
    vlan_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = (vlan_msg_req_t *)msg_buf_pool_get(VLAN_request_pool);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }
    msg->msg_id = msg_id;
    return msg;
}

/******************************************************************************/
// VLAN_msg_large_alloc()
// Allocate message buffer from the request pool meant for "large" messages.
/******************************************************************************/
static vlan_msg_large_req_t *VLAN_msg_large_alloc(vlan_msg_id_t msg_id)
{
    vlan_msg_large_req_t *msg = (vlan_msg_large_req_t *)msg_buf_pool_get(VLAN_large_request_pool);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/******************************************************************************/
// VLAN_msg_id_txt()
/******************************************************************************/
static const char *VLAN_msg_id_txt(vlan_msg_id_t msg_id)
{
    const char *txt;

    switch (msg_id) {
    case VLAN_MSG_ID_MEMBERSHIP_SET_REQ:
        txt = "VLAN_MEMBERSHIP_SET_REQ";
        break;

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    case VLAN_MSG_ID_CONF_TPID_REQ:
        txt = "VLAN_MSG_ID_CONF_TPID_REQ";
        break;
#endif /* defined(VTSS_FEATURED_VLAN_PORT_V2) */

    case VLAN_MSG_ID_PORT_CONF_ALL_SET_REQ:
        txt = "VLAN_PORT_CONF_ALL_SET_REQ";
        break;

    case VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ:
        txt = "VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ";
        break;

    default:
        txt = "?";
        break;
    }
    return txt;
}

/******************************************************************************/
// VLAN_msg_tx_done()
// Free the memory allocated to the message after sending.
/******************************************************************************/
static void VLAN_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    vlan_msg_id_t msg_id = *(vlan_msg_id_t *)msg;

    T_D("msg_id: %d, %s", msg_id, VLAN_msg_id_txt(msg_id));
    (void)msg_buf_pool_put(msg);
}

/******************************************************************************/
// VLAN_msg_tx()
// Send a VLAN message.
/******************************************************************************/
static void VLAN_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    vlan_msg_id_t msg_id = *(vlan_msg_id_t *)msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, VLAN_msg_id_txt(msg_id), len, isid);

    // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of MSG_TX_DATA_HDR_LEN_MAX() below
    /*lint -e(506) */
    msg_tx_adv(NULL, VLAN_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_VLAN, isid, msg, len + MSG_TX_DATA_HDR_LEN_MAX(vlan_msg_req_t, req, vlan_msg_large_req_t, large_req));
}

/******************************************************************************/
// VLAN_get()
//
// Returns a specific user's (next) VLAN membership/VID.
//
// #isid: Legal ISID or VTSS_ISID_GLOBAL. If legal ISID, also get membership, otherwise only get (next) VID.
// #vid: [0; VLAN_ID_MAX], 0 only if #next == TRUE.
// #next: TRUE if find a VID > #vid. FALSE if getting membership-enabledness for #vid.
// #user: Can be anthing from [VLAN_USER_STATIC; VLAN_USER_ALL]
// #result: May be NULL if not interested in memberships.
// #user_enabled_vlans_only: For Trunk and Hybrid ports, a VLAN may
//                           get auto-added even though not added by end-user.
//                           If TRUE, the function will not report a VLAN as found
//                           (for user = VLAN_USER_STATIC) unless it is an end-user-
//                           enabled VLAN.
//
// Returns VTSS_VID_NULL if no such (next) entry was found, and a valid VID if it
// was found. In that case, #result is updated if isid != VTSS_ISID_GLOBAL.
/******************************************************************************/
static vtss_vid_t VLAN_get(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user, BOOL next, vlan_ports_t *result, BOOL user_enabled_vlans_only)
{
    BOOL        found = FALSE;
    vtss_isid_t isid_iter, isid_min, isid_max;
    vtss_vid_t  vid_min, vid_max;
    u32         multi_user_idx = VLAN_user_to_multi_idx(user);
#if defined(VLAN_SAME_USER_SUPPORT)
    u32         same_user_idx  = VLAN_user_to_same_idx(user);
#endif /* defined(VLAN_SAME_USER_SUPPORT) */
#if defined(VLAN_SINGLE_USER_SUPPORT)
    u32         single_user_idx = VLAN_user_to_single_idx(user);
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

    VLAN_CRIT_ASSERT_LOCKED();

    T_D("isid %d, vid %d, next %d, user %s", isid, vid, next, vlan_mgmt_user_to_txt(user));

    // Zero it out to start with
    if (result) {
        memset(result, 0, sizeof(*result));
    }

    if (next == FALSE && (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX)) {
        T_E("Ehh? (%d)", vid);
        return VTSS_VID_NULL;
    }

    if (user > VLAN_USER_ALL) {
        T_E("Invalid user %d", user);
        return VTSS_VID_NULL;
    }

    if (isid == VTSS_ISID_GLOBAL) {
        isid_min = VTSS_ISID_START;
        isid_max = VTSS_ISID_END - 1;
    } else {
        isid_min = isid_max = isid;
    }

    if (next) {
        vid_min = vid + 1;
        vid_max = VLAN_ID_MAX;
    } else {
        vid_min = vid_max = vid;
    }

    // Loop through to find the (next) valid VID.
    for (vid = vid_min; vid <= vid_max; vid++) {
        for (isid_iter = isid_min; isid_iter <= isid_max; isid_iter++) {
            if (VLAN_IN_USE_ON_SWITCH_GET(isid_iter, vid, user)) {
                if (user == VLAN_USER_STATIC && user_enabled_vlans_only && !VTSS_BF_GET(VLAN_end_user_enabled_vlans, vid)) {
                    // The static user can easily have a VLAN in use on a given switch
                    // without that meaning that the VLAN is enabled. This could happen
                    // if a port is set-up in trunk or hybrid mode. In these modes the port
                    // is member of all allowed VLANs. We do not wish to report these
                    // back to the caller in VLAN_get().
                    continue;
                }

                found = TRUE;
                break;
            }
        }

        if (found) {
            break;
        }
    }

    // Now, if #found is TRUE, #vid points to the VID we're looking for.

    if (result == NULL || isid == VTSS_ISID_GLOBAL || !found) {
        // If #isid is not a legal ISID, the caller only wants to know
        // whether the entry exists, so we can exit now.
        // Also, if an entry was not found, we can exit.
        return found ? vid : VTSS_VID_NULL;
    }

    // If we get here, the caller wants us to fill in the membership info as well for a specific ISID.

    // Forbidden, Combined, Multi-, and Same- VLAN users all use the same structure.
    if (user == VLAN_USER_FORBIDDEN || user == VLAN_USER_ALL || multi_user_idx != VLAN_MULTI_CNT
#if defined(VLAN_SAME_USER_SUPPORT)
        || same_user_idx != VLAN_SAME_CNT
#endif /* defined(VLAN_SAME_USER_SUPPORT) */
       ) {
        // Caller is interested in getting forbidden, combined, multi- or same-user port membership for VID #vid.
        vlan_entry_t *entry;

        if (user == VLAN_USER_FORBIDDEN) {
            entry = &VLAN_forbidden_table[vid];
        } else if (user == VLAN_USER_ALL) {
            entry = &VLAN_combined_table[vid];
        } else if (multi_user_idx != VLAN_MULTI_CNT) {
            if (VLAN_multi_table[vid] == NULL) {
                // in_use table and membership table are out of sync.
                // This should not be possible.
                T_E("Internal error");
                return VTSS_VID_NULL;
            }

            entry = &VLAN_multi_table[vid]->user_entries[multi_user_idx];
        }

#if defined(VLAN_SAME_USER_SUPPORT)
        else if (same_user_idx != VLAN_SAME_CNT) {
            // "Same" user. Such users can only configure one single VLAN.
            if (VLAN_same_table[same_user_idx].vid != vid) {
                // in_use table and configured VID are out of sync.
                // This should not be possible.
                T_E("Internal error (vid=%d, %d). User %s", VLAN_same_table[same_user_idx].vid, vid, vlan_mgmt_user_to_txt(user));
                return VTSS_VID_NULL;
            }

            entry = &VLAN_same_table[same_user_idx].entry;
        }
#endif /* defined(VLAN_SAME_USER_SUPPORT) */

        else {
            T_E("How did this happen?");
            return VTSS_VID_NULL;
        }

        memcpy(result->ports, entry->ports[isid - VTSS_ISID_START], sizeof(result->ports));
    }

#if defined(VLAN_SINGLE_USER_SUPPORT)
    else if (single_user_idx != VLAN_SINGLE_CNT) {
        // A "single" user is a user that can configure different ports to be members of different VLANs,
        // but at most one VLAN per port.
        vlan_single_membership_entry_t *entry = &VLAN_single_table[single_user_idx];
        port_iter_t                    pit;

        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (entry->vid[isid - VTSS_ISID_START][pit.iport] == vid) {
                VTSS_BF_SET(result->ports, pit.iport, TRUE);
            }
        }

        // It is not mandated that any ports are now members, because it's possible to create an empty VLAN.
    }
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

    else {
        // Here, we should have handled the user already.
        T_E("Invalid VLAN User %d", user);
    }

    return vid;
}

/******************************************************************************/
// VLAN_bulk_begin()
/******************************************************************************/
static void VLAN_bulk_begin(void)
{
    VLAN_CRIT_ASSERT_LOCKED();

    // These days, VLAN configuration change commands can be quite powerful,
    // and cause a lot of changes with just a few simple clicks with the mouse
    // or ICLI commands.
    // In order to minimize the number of calls to the VLAN membership API
    // function and to the callback registrants, we do a reference counting
    // mechanism so that when e.g. Web or CLI know that a lot of changes are to
    // be applied to the VLAN module, it calls vlan_bulk_update_begin(), which
    // increases the reference count, and when it's done it calls
    // vlan_bulk_update_end(), which decreases the reference count. Once
    // the reference count reaches 0, any changes applied while the ref.
    // count was greater than 0 are now applied to subscribers and to H/W.
    //
    // Now, the problem is that we cannot hold the VLAN_crit while calling
    // back subscribers, because they might need to call back into the
    // VLAN module. This means that we have to let go of VLAN_crit everytime
    // we call back. This, in turn, means that we must never clear the dirty
    // arrays when ref_cnt increases from 0 to 1. Instead we need to re-run
    // all dirty arrays in the called context whenever ref-count decreases
    // to zero.
    VLAN_bulk.ref_cnt++;
    T_I("New ref. count = %u", VLAN_bulk.ref_cnt);
}

/******************************************************************************/
// VLAN_bulk_end()
/******************************************************************************/
static void VLAN_bulk_end(void)
{
    vtss_vid_t            vid;
    vtss_isid_t           isid, zisid;
    BOOL                  local_flush[VTSS_ISID_CNT];
    BOOL                  at_least_one_change = FALSE;
    vlan_change_cb_conf_t local_vid_cb[ARRSZ(VLAN_mc_cb)], local_bulk_cb[ARRSZ(VLAN_mc_bcb)];
    int                   i;

    // It confuses Lint that we exit then enter instead of enter then exit.
    //lint --e{454,455,456}

    VLAN_CB_CRIT_ENTER();
    // Take a copy in order to avoid deadlock issues
    memcpy(local_vid_cb,  VLAN_mc_cb,  sizeof(local_vid_cb));
    memcpy(local_bulk_cb, VLAN_mc_bcb, sizeof(local_bulk_cb));
    VLAN_CB_CRIT_EXIT();

    VLAN_CRIT_ASSERT_LOCKED();

    if (VLAN_bulk.ref_cnt == 0) {
        T_E("Bulk reference count is already 0. Can't decrease it further");
        return;
    } else {
        VLAN_bulk.ref_cnt--;

        T_I("New ref. count = %u", VLAN_bulk.ref_cnt);

        if (VLAN_bulk.ref_cnt > 0) {
            return;
        }
    }

    T_I("New ref. count = %u", VLAN_bulk.ref_cnt);

    // Take a snapshot of the flush-array, because we need to clear it again
    // and because we need the info when sending notifications back to registrants.
    memcpy(local_flush, VLAN_bulk.flush, sizeof(local_flush));

    // ------------------------oOo------------------------
    // Update H/W
    // ------------------------oOo------------------------
    // Send new configuration to switches in question.
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        zisid = isid - VTSS_ISID_START;

        VLAN_bulk.flush[zisid] = FALSE;

        // Bit 0 (VTSS_VID_NULL) in the array tells whether we should transmit a message or not.
        if (VTSS_BF_GET(VLAN_bulk.s[zisid].tx_conf, VTSS_VID_NULL)) {
            vlan_msg_large_req_t *msg;
            u32                  cnt;

            VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, VTSS_VID_NULL, FALSE);

            if (!msg_switch_exists(isid)) {
                // Nothing to do, since it doesn't exist.
                continue;
            }

            msg = VLAN_msg_large_alloc(VLAN_MSG_ID_MEMBERSHIP_SET_REQ);
            msg->large_req.set.flush = local_flush[zisid];
            cnt = 0;

            for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
                if (VTSS_BF_GET(VLAN_bulk.s[zisid].tx_conf, vid)) {
                    VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, vid, FALSE);

                    // Get this VID's configuration. This may be an empty configuration, so don't
                    // check the return value, which just tells you whether the VID exists or not.
                    msg->large_req.set.table[cnt].vid = vid;
                    (void)VLAN_get(isid, vid, VLAN_USER_ALL, FALSE, &msg->large_req.set.table[cnt++].entry, FALSE);
                }
            }

            msg->large_req.set.cnt = cnt;

            VLAN_msg_tx(msg, isid, sizeof(msg->large_req.set) - (VLAN_ENTRY_CNT - cnt) * sizeof(vlan_entry_single_switch_with_vid_t));

            // VLAN bulk subscribers must be invoked whenever a VLAN changes, whether it's due
            // to VLAN_USER_STATIC or some other user (non-bulk-subscribers only get called
            // back if VLAN_USER_STATIC or VLAN_USER_FORBIDDEN changes).
            at_least_one_change = TRUE;
        }
    }

    // ------------------------oOo------------------------
    // Update subscribers
    // ------------------------oOo------------------------
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        vlan_membership_change_t changes;

        zisid = isid - VTSS_ISID_START;

        if (!VLAN_bulk.s[zisid].dirty) {
            // Nothing to do for this switch.
            continue;
        }

        VLAN_bulk.s[zisid].dirty = FALSE;

        if (local_flush[zisid]) {
            // Pretend that all ports have changed for this VID
            memset(&changes.changed_ports, 0xff, sizeof(changes.changed_ports));
        }

        for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
            BOOL notify;

            if (!VTSS_BF_GET(VLAN_bulk.s[zisid].dirty_vids, vid)) {
                continue;
            }

            VTSS_BF_SET(VLAN_bulk.s[zisid].dirty_vids, vid, FALSE);

            // We always notify if flushing remote VLAN table, because that
            // means that all possible old VLANs are getting changed.
            notify = local_flush[zisid];

            changes.static_vlan_exists = VTSS_BF_GET(VLAN_end_user_enabled_vlans, vid);
            (void)VLAN_get(isid, vid, VLAN_USER_STATIC,    FALSE, &changes.static_ports,    FALSE);
            (void)VLAN_get(isid, vid, VLAN_USER_FORBIDDEN, FALSE, &changes.forbidden_ports, FALSE);

            if (!local_flush[zisid]) {
                // Gotta compute a change mask
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    if ((changes.changed_ports.ports[i] = (VLAN_bulk.s[zisid].old_members[0][vid].ports[i] ^ changes.static_ports.ports[i]) | (VLAN_bulk.s[zisid].old_members[1][vid].ports[i] ^ changes.forbidden_ports.ports[i])) != 0) {
                        notify = TRUE;
                    }
                }
            }

            // Call back per-VID subscribers.
            if (notify) {
                // Temporarily exit our crit while calling back in order for the call back functions to
                // be able to call into the VLAN module again.
                VLAN_CRIT_EXIT();
                for (i = 0; i < ARRSZ(local_vid_cb); i++) {
                    if (local_vid_cb[i].u.cb == NULL) {
                        // Since there is no un-register support,
                        // we know that there are no more registrants
                        // when we meet a NULL pointer.
                        break;
                    }

                    T_IG(TRACE_GRP_CB, "%u:%u: Calling back %s", isid, vid, vtss_module_names[local_vid_cb[i].modid]);
                    local_vid_cb[i].u.cb(isid, vid, &changes);
                }

                VLAN_CRIT_ENTER();
            }
        }
    }

    if (at_least_one_change) {
#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        VLAN_conf_save();
        VLAN_forbidden_conf_save();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        VLAN_CRIT_EXIT();
        // Call back bulk subscribers.
        for (i = 0; i < ARRSZ(local_bulk_cb); i++) {
            if (local_bulk_cb[i].u.bcb == NULL) {
                // Since there is no un-register support,
                // we know that there are no more registrants
                // when we meet a NULL pointer.
                break;
            }

            T_IG(TRACE_GRP_CB, "Calling back %s", vtss_module_names[local_bulk_cb[i].modid]);
            local_bulk_cb[i].u.bcb();
        }

        VLAN_CRIT_ENTER();
    }

    VLAN_CRIT_ASSERT_LOCKED();
}

/******************************************************************************/
// VLAN_s_custom_etype_change_callback()
/******************************************************************************/
static void VLAN_s_custom_etype_change_callback(vtss_etype_t tpid)
{
    vlan_change_cb_conf_t local_cb[ARRSZ(VLAN_s_custom_etype_cb)];
    int                   i;

    VLAN_CB_CRIT_ENTER();
    // Take a copy in order to avoid deadlock issues
    memcpy(local_cb, VLAN_s_custom_etype_cb, sizeof(local_cb));
    VLAN_CB_CRIT_EXIT();

    for (i = 0; i < ARRSZ(local_cb); i++) {
        if (local_cb[i].u.scb == NULL) {
            // Since there is no un-register support,
            // we know that there are no more registrants
            // when we meet a NULL pointer.
            break;
        }

        T_IG(TRACE_GRP_CB, "Calling back %s with EtherType %u", vtss_module_names[local_cb[i].modid], tpid);
        local_cb[i].u.scb(tpid);
    }
}

/******************************************************************************/
// VLAN_port_conf_api_set()
// Setup VLAN port configuration via switch API.
/******************************************************************************/
static void VLAN_port_conf_api_set(vtss_port_no_t port_no, vlan_port_conf_t *conf)
{
    vtss_rc               rc;
    vtss_vlan_port_conf_t api_conf;

    if (conf->pvid < VLAN_ID_MIN || conf->pvid > VLAN_ID_MAX) {
        T_E("%u: Invalid PVID (%u). Setting to default", port_no, conf->pvid);
        conf->pvid = VLAN_ID_DEFAULT;
    }

    if (conf->tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS || conf->tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS) {
        if (conf->untagged_vid < VLAN_ID_MIN || conf->untagged_vid > VLAN_ID_MAX) {
            T_E("Invalid UVID (%d)", conf->untagged_vid);
        }
    }

    // To be able to seemlessly support future API enhancements,
    // we need to get the current configuration prior to setting
    // a new.
    if ((rc = vtss_vlan_port_conf_get(NULL, port_no, &api_conf)) != VTSS_RC_OK) {
        T_E("Huh (%d)?", port_no);
        memset(&api_conf, 0, sizeof(api_conf)); // What else can we do than resetting to all-zeros?
    }

#if defined(VTSS_FEATURE_VLAN_PORT_V1)
    api_conf.stag = 0;
    api_conf.aware = conf->port_type;
#endif /* VTSS_FEATURE_VLAN_PORT_V1 */

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    switch (conf->port_type) {
    case VLAN_PORT_TYPE_UNAWARE:
        api_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;
        break;

    case VLAN_PORT_TYPE_C:
        api_conf.port_type = VTSS_VLAN_PORT_TYPE_C;
        break;

    case VLAN_PORT_TYPE_S:
        api_conf.port_type = VTSS_VLAN_PORT_TYPE_S;
        break;

    case VLAN_PORT_TYPE_S_CUSTOM:
        api_conf.port_type = VTSS_VLAN_PORT_TYPE_S_CUSTOM;
        break;

    default:
        T_E("Invalid port type %d", conf->port_type);
        break;
    }
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */

    api_conf.pvid = conf->pvid;

    switch (conf->tx_tag_type) {
    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
        api_conf.untagged_vid = conf->untagged_vid;
        break;

    case VLAN_TX_TAG_TYPE_TAG_THIS:
        // This is the least prioritized tag-type. If PVID must be tagged, we can never end here.
        // #conf->untagged_vid contains a VLAN ID that we must tag (SIC!).
        // If #conf->untagged_vid == #conf->pvid, then we tag everything by setting #api_conf.untagged_vid to VTSS_VID_NULL.
        // Otherwise we set it to #conf->pvid.
        api_conf.untagged_vid = conf->untagged_vid == conf->pvid ? VTSS_VID_NULL : conf->pvid;
        break;

    case VLAN_TX_TAG_TYPE_TAG_ALL:
        api_conf.untagged_vid = VTSS_VID_NULL;
        break;

    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
        api_conf.untagged_vid = VTSS_VID_ALL;
        break;

    default:
        T_E("Invalid tx_tag_type (%d)", conf->tx_tag_type);
        return;
    }

    api_conf.frame_type = conf->frame_type;
#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
    // Option available & controllable
    api_conf.ingress_filter = conf->ingress_filter;
#else
    api_conf.ingress_filter = TRUE; // Always enable
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */

    T_D("VLAN Port change port %d, pvid %d", port_no, api_conf.pvid);
    if ((rc = vtss_vlan_port_conf_set(NULL, port_no, &api_conf)) != VTSS_RC_OK) {
        T_E("%u: %s", iport2uport(port_no), error_txt(rc));
        return;
    }

    // Call-back those modules interested in configuration changes on the local switch.
    VLAN_port_conf_change_callback(VTSS_ISID_LOCAL, port_no, conf);
}

/******************************************************************************/
// VLAN_port_conf_api_get()
// Get VLAN port configuration directly from switch API.
/******************************************************************************/
static vtss_rc VLAN_port_conf_api_get(vtss_port_no_t port_no, vlan_port_conf_t *conf)
{
    vtss_rc               rc;
    vtss_vlan_port_conf_t api_conf;

    memset(conf, 0, sizeof(*conf));
    rc = vtss_vlan_port_conf_get(NULL, port_no, &api_conf);

#if defined(VTSS_FEATURE_VLAN_PORT_V1)
    conf->port_type = (api_conf.aware ? VLAN_PORT_TYPE_C : VLAN_PORT_TYPE_UNAWARE);
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V1) */

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    switch (api_conf.port_type) {
    case VTSS_VLAN_PORT_TYPE_UNAWARE:
        conf->port_type = VLAN_PORT_TYPE_UNAWARE;
        break;

    case VTSS_VLAN_PORT_TYPE_C:
        conf->port_type = VLAN_PORT_TYPE_C;
        break;

    case VTSS_VLAN_PORT_TYPE_S:
        conf->port_type = VLAN_PORT_TYPE_S;
        break;

    case VTSS_VLAN_PORT_TYPE_S_CUSTOM:
        conf->port_type = VLAN_PORT_TYPE_S_CUSTOM;
        break;

    default:
        T_E("Invalid API port type %d", api_conf.port_type);
        break;
    }
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

    conf->pvid = api_conf.pvid;
    conf->untagged_vid = api_conf.untagged_vid;
    if (conf->untagged_vid == VTSS_VID_NULL) {
        conf->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
    } else if (conf->untagged_vid == VTSS_VID_ALL) {
        conf->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_ALL;
    } else if (conf->pvid == conf->untagged_vid) {
        conf->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_THIS;
    } else {
        conf->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_THIS;
    }

    conf->frame_type = api_conf.frame_type;
#if defined(VTSS_SW_OPTION_VLAN_INGRESS_FILTERING)
    conf->ingress_filter = api_conf.ingress_filter;
#endif  /* defined(VTSS_SW_OPTION_VLAN_INGRESS_FILTERING) */

    return rc;
}

/******************************************************************************/
// VLAN_bit_to_bool()
// Converts a bit-array of ports to a boolean array of ports.
// The number of bits to convert depends on #isid.
// You may call this with VTSS_ISID_LOCAL or a legal ISID.
// Any stack ports are lost in this conversion.
// Returns TRUE if at least one port is set in the result, FALSE otherwise.
/******************************************************************************/
static BOOL VLAN_bit_to_bool(vtss_isid_t isid, vlan_ports_t *src, vlan_mgmt_entry_t *dst, u64 *port_mask)
{
    port_iter_t pit;
    BOOL        at_least_one_bit_set = FALSE;

    memset(dst->ports, 0, sizeof(dst->ports));

    *port_mask = 0;

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (VTSS_BF_GET(src->ports, pit.iport)) {
            dst->ports[pit.iport] = at_least_one_bit_set = TRUE;
            *port_mask |= VTSS_BIT64(pit.iport);
        }
    }

    return at_least_one_bit_set;
}

/******************************************************************************/
// VLAN_bool_to_bit()
// Converts a boolean array of ports to a bit-array of ports.
// The number of entries to convert depends on #isid.
// You may call this with VTSS_ISID_LOCAL or a legal ISID.
// Any stack ports are lost in this conversion.
/******************************************************************************/
static void VLAN_bool_to_bit(vtss_isid_t isid, vlan_mgmt_entry_t *src, vlan_ports_t *dst)
{
    port_iter_t pit;
    memset(dst, 0, sizeof(*dst));

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        VTSS_BF_SET(dst->ports, pit.iport, src->ports[pit.iport]);
    }
}

/******************************************************************************/
// VLAN_membership_api_set()
// Add/Delete VLAN entry to/from switch API.
/******************************************************************************/
static vtss_rc VLAN_membership_api_set(vtss_vid_t vid, vlan_ports_t *conf)
{
    vtss_rc           rc;
    vlan_mgmt_entry_t dst;

    // Figure out if user is adding or deleting VID
    if (conf) {
        u64 port_mask;
        (void)VLAN_bit_to_bool(VTSS_ISID_LOCAL, conf, &dst, &port_mask);
        T_I("VLAN Add: vid %u port_mask = 0x%08" PRIx64, vid, port_mask);
    } else {
        memset(&dst, 0, sizeof(dst));
        T_I("VLAN Delete: vid %u", vid);
    }

    if ((rc = vtss_vlan_port_members_set(NULL, vid, dst.ports)) != VTSS_RC_OK) {
        T_D("vtss_vlan_port_member_set(): %s", error_txt(rc));
        return rc;
    }

    // Enable port isolation for this VLAN
    return vtss_isolated_vlan_set(NULL, vid, conf != NULL);
}

/******************************************************************************/
// VLAN_api_get()
// Retrieve (next) VID and memberships directly from H/W.
// This is as opposed to VLAN_get(), which works on the software state.
//
// Returns VTSS_VID_NULL if no such (next) entry was found, and a valid VID if
// it was found. In that case, #result is updated.
/******************************************************************************/
static vtss_vid_t VLAN_api_get(vtss_vid_t vid, BOOL next, vlan_ports_t *result)
{
    vtss_rc          rc = VTSS_RC_ERROR;
    vtss_vid_t       vid_min, vid_max;
    BOOL             ports[VTSS_PORT_ARRAY_SIZE];
    BOOL             empty_member[VTSS_PORT_ARRAY_SIZE];
    BOOL             found = FALSE;
    vtss_port_no_t   port_no;

    if (next) {
        vid_min = vid + 1;
        vid_max = VLAN_ID_MAX;
    } else {
        vid_min = vid_max = vid;
    }

    memset(result,       0, sizeof(*result));
    memset(empty_member, 0, sizeof(empty_member));
    memset(ports,        0, sizeof(ports));

    for (vid = vid_min; vid <= vid_max; vid++) {
        if ((rc = vtss_vlan_port_members_get(NULL, vid, ports)) != VTSS_RC_OK) {
            T_E("VLAN_membership_api_get(): %s", error_txt(rc));
            return VTSS_VID_NULL;
        }

        if (memcmp(ports, empty_member, VTSS_PORT_ARRAY_SIZE) != 0) {
            found = TRUE;
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
                VTSS_BF_SET(result->ports, port_no, ports[port_no]);
            }

            break;
        }
    }

    return found ? vid : VTSS_VID_NULL;
}

/******************************************************************************/
// VLAN_combined_update()
// #isid must be a legal ISID.
// #vid must be a legal VID ([VLAN_ID_MIN; VLAN_ID_MAX]).
/******************************************************************************/
static void VLAN_combined_update(vtss_isid_t isid, vtss_vid_t vid)
{
    vlan_ports_t combined;
    vtss_isid_t  zisid = isid - VTSS_ISID_START;
    vlan_entry_t *entry = &VLAN_combined_table[vid];
    BOOL         found = FALSE;
    vlan_user_t  user;
    int          i;

    VLAN_CRIT_ASSERT_LOCKED();

    memset(&combined, 0, sizeof(combined));

    for (user = VLAN_USER_STATIC; user < VLAN_USER_ALL; user++) {
        vlan_ports_t user_contrib;

        if (!VLAN_IN_USE_ON_SWITCH_GET(isid, vid, user)) {
            // This user is not contributing to final VLAN.
            continue;
        }

        if (VLAN_get(isid, vid, user, FALSE, &user_contrib, FALSE) == VTSS_VID_NULL) {
            T_E("Odd. User %s is contributing to VID %u, but VLAN_get() says it's not", vlan_mgmt_user_to_txt(user), vid);
            continue;
        }

        if (user == VLAN_USER_FORBIDDEN) {
            // Forbidden VLANs override everything. The "VLAN_USER_FORBIDDEN" enumeration must
            // be the last in the iteration.
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                combined.ports[i] &= ~user_contrib.ports[i];
            }
        } else {
            // A forbidden VLAN does not contribute to whether the VLAN exists or not,
            // so only set #found to TRUE on non-forbidden users.
            found = TRUE;
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                combined.ports[i] |= user_contrib.ports[i];
            }
        }
    }

    // Gotta update the bulk changes for later membership subscriber callback and H/W update.
    if (memcmp(entry->ports[zisid], combined.ports, sizeof(entry->ports[zisid])) != 0) {
        VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, vid, TRUE);

        // Use vid == VTSS_VID_NULL to indicate that there's something to transmit for this ISID.
        VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, VTSS_VID_NULL, TRUE);
    }

    if (found) {
        memcpy(VLAN_combined_table[vid].ports[zisid], combined.ports, VTSS_PORT_BF_SIZE);
    } else {
        memset(VLAN_combined_table[vid].ports[zisid], 0, sizeof(VLAN_combined_table[vid].ports[zisid]));
    }

    VLAN_IN_USE_ON_SWITCH_SET(isid, vid, VLAN_USER_ALL, found);
}

/******************************************************************************/
// VLAN_add()
// Adds or overwrites a VLAN to #isid.
// #isid must be a legal ISID.
// #vid must be a valid VID.
// #user must be in range [VLAN_USER_STATIC; VLAN_USER_ALL[.
//
// If #user == VLAN_USER_STATIC, #entry may contain the empty port set.
// If #user != VLAN_USER_STATIC, #entry must have at least one port set.
//
// This function automatically updates VLAN_USER_ALL with new combined state.
/******************************************************************************/
static vtss_rc VLAN_add(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user, vlan_ports_t *entry)
{
    vtss_rc     rc = VTSS_RC_OK;
    u32         user_idx;
    vtss_isid_t zisid = isid - VTSS_ISID_START;

    VLAN_CRIT_ASSERT_LOCKED();

    T_D("VLAN Add: isid %u vid %u user %s", isid, vid, vlan_mgmt_user_to_txt(user));

    // Various updates for notifications to subscribers and switch updates.
    if (user == VLAN_USER_STATIC || user == VLAN_USER_FORBIDDEN) {
        if (VTSS_BF_GET(VLAN_bulk.s[zisid].dirty_vids, vid) == FALSE) {
            // This VLAN ID has not been touched before on this switch,
            // so we better take a snapshot of the current configuration, so that
            // we can send out any changes to subscribers.
            // The following will also clear the returned port array if the VLAN is currently not existing.
            (void)VLAN_get(isid, vid, VLAN_USER_STATIC,    FALSE, &VLAN_bulk.s[zisid].old_members[0][vid], FALSE);
            (void)VLAN_get(isid, vid, VLAN_USER_FORBIDDEN, FALSE, &VLAN_bulk.s[zisid].old_members[1][vid], FALSE);

            // Now, this VID is going to be changed and hence dirty
            VTSS_BF_SET(VLAN_bulk.s[zisid].dirty_vids, vid, TRUE);

            // And also tell that this switch is now dirty.
            VLAN_bulk.s[zisid].dirty = TRUE;
        }
    }

    if (user == VLAN_USER_FORBIDDEN) {
        memcpy(VLAN_forbidden_table[vid].ports[zisid], entry->ports, VTSS_PORT_BF_SIZE);
    } else if ((user_idx = VLAN_user_to_multi_idx(user)) != VLAN_MULTI_CNT) {
        // A multi-user is a VLAN user that is allowed to set membership of all ports
        // on all VIDs (as long as there are free entries).
        vlan_list_entry_t *the_vlan;

        if ((the_vlan = VLAN_multi_table[vid]) == NULL) {

            if (VLAN_free_list == NULL) {
                rc = VLAN_ERROR_VLAN_TABLE_FULL;
                goto exit_func;
            }

            // Pick the first item from the free list.
            the_vlan = VLAN_free_list;
            VLAN_free_list = VLAN_free_list->next;
            memset(the_vlan, 0, sizeof(*the_vlan));
            VLAN_multi_table[vid] = the_vlan;
        }

        memcpy(the_vlan->user_entries[user_idx].ports[zisid], entry->ports, VTSS_PORT_BF_SIZE);
    }

#if defined(VLAN_SAME_USER_SUPPORT)
    else if ((user_idx = VLAN_user_to_same_idx(user)) != VLAN_SAME_CNT) {
        // "Same" VLAN users can configure one single VLAN.
        vlan_entry_with_vid_t *same_member_entry = &VLAN_same_table[user_idx];

        // Check if the user entry is previously configured
        if (same_member_entry->vid != VTSS_VID_NULL && same_member_entry->vid != vid) {
            T_E("\"Same\" user %s has already configured a VLAN (%d). Attempting to configure %d", vlan_mgmt_user_to_txt(user), same_member_entry->vid, vid);
            rc = VLAN_ERROR_USER_PREVIOUSLY_CONFIGURED;
            goto exit_func;
        }

        // Update the user database
        same_member_entry->vid = vid;

        // Overwrite the entire portion for this ISID
        memcpy(same_member_entry->entry.ports[zisid], entry->ports, VTSS_PORT_BF_SIZE);
    }
#endif /* defined(VLAN_SAME_USER_SUPPORT) */

#if defined(VLAN_SINGLE_USER_SUPPORT)
    else if ((user_idx = VLAN_user_to_single_idx(user)) != VLAN_SINGLE_CNT) {
        // A "single" user is a user that can configure different ports to be members of different VLANs,
        // but at most one VLAN per port.
        vlan_single_membership_entry_t *single_member_entry = &VLAN_single_table[user_idx];
        port_iter_t                    pit;

        // Loop through and set our database's VID for ports set in the
        // users port list. Before setting, check that the entry is not
        // currently used, since that's not allowed for SINGLE users.
        // Also remember to clear out ports that match the user request's
        // VID, but are not set in the user's request (anymore).

        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            vtss_vid_t *vid_ptr = &single_member_entry->vid[zisid][pit.iport];
            if (VTSS_BF_GET(entry->ports, pit.iport)) {
                // User wants to make this port member of #vid. Check if it's OK.
                if (*vid_ptr != VTSS_VID_NULL && *vid_ptr != vid) {
                    T_E("User %s has already a VID (%d) configured on port %u:%u. New vid = %u", vlan_mgmt_user_to_txt(user), *vid_ptr, isid, pit.uport, vid);
                    rc = VLAN_ERROR_USER_PREVIOUSLY_CONFIGURED;
                    goto exit_func;
                }

                *vid_ptr = vid;
            } else if (*vid_ptr == vid) {
                // User no longer wants this port to be member of the VLAN ID.
                *vid_ptr = VTSS_VID_NULL;
            }
        }
    }
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */
    else {
        T_E("Ehh (user = %d)", user);
        rc = VLAN_ERROR_USER;
    }

exit_func:
    if (rc == VTSS_RC_OK) {
        // Success. Update this user's in-use flag.
        VLAN_IN_USE_ON_SWITCH_SET(isid, vid, user, TRUE);

        // And go on and update the combined user's membership and in-use flag.
        // Unfortunately, it's not good enough to simply OR the new user membership
        // onto the current combined memership, because the new membership may
        // have cleared some ports while adding others on this ISID. And there is
        // no guarantee that just because this user clears a port, that the port
        // should be cleared in the combined membership (another user may have it set).
        VLAN_combined_update(isid, vid);
    }

    return rc;
}

/******************************************************************************/
// VLAN_del()
// #isid must be a legal ISID.
// #vid  must be a legal VID.
// #user must be in range [VLAN_USER_STATIC; VLAN_USER_ALL[
/******************************************************************************/
static void VLAN_del(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user)
{
    u32         user_idx;
    BOOL        in_use;
    vtss_isid_t isid_iter;
    vlan_user_t user_iter;
    vtss_isid_t zisid = isid - VTSS_ISID_START;

    VLAN_CRIT_ASSERT_LOCKED();

    if (!VLAN_IN_USE_ON_SWITCH_GET(isid, vid, user)) {
        // #user does not have a share on the final VID on this switch.
        return;
    }

    // Various updates for notifications to subscribers.
    if (user == VLAN_USER_STATIC || user == VLAN_USER_FORBIDDEN) {
        if (VTSS_BF_GET(VLAN_bulk.s[zisid].dirty_vids, vid) == FALSE) {
            // Get the old members before it was deleted, so that we can tell subscribers about changes.
            (void)VLAN_get(isid, vid, VLAN_USER_STATIC,    FALSE, &VLAN_bulk.s[zisid].old_members[0][vid], FALSE);
            (void)VLAN_get(isid, vid, VLAN_USER_FORBIDDEN, FALSE, &VLAN_bulk.s[zisid].old_members[1][vid], FALSE);

            // Now, this VID is going to be changed and hence dirty
            VTSS_BF_SET(VLAN_bulk.s[zisid].dirty_vids, vid, TRUE);

            // And also tell that this switch is now dirty.
            VLAN_bulk.s[zisid].dirty = TRUE;
        }
    }

    // #user has a share in VLAN membership's final value.
    // Let's see if it changes anything to back out of this share.
    VLAN_IN_USE_ON_SWITCH_SET(isid, vid, user, FALSE);

    in_use = FALSE;
    for (isid_iter = VTSS_ISID_START; isid_iter < VTSS_ISID_END; isid_iter++) {
        if (VLAN_IN_USE_ON_SWITCH_GET(isid_iter, vid, user)) {
            // Still in use on another switch.
            in_use = TRUE;
            break;
        }
    }

    if (!in_use) {
        // The VID is no longer in use. Do various updates.
        if (VLAN_user_to_multi_idx(user) != VLAN_MULTI_CNT) {
            // If #user is a multi-user, it could be that we should move the entry to the free list.

            if (VLAN_multi_table[vid] == NULL) {
                // Must not be possible to have a NULL entry in this table
                // by now (since VLAN_IN_USE_ON_SWITCH_GET() above returned TRUE).
                T_E("Internal error");
                return;
            }

            in_use = FALSE;
            for (user_idx = 0; user_idx < VLAN_MULTI_CNT; user_idx++) {
                user_iter = VLAN_multi_idx_to_user(user_idx);
                if (user_iter == user) {
                    // We already know that #user is no longer in use. Only check other multi-users
                    continue;
                }

                for (isid_iter = VTSS_ISID_START; isid_iter < VTSS_ISID_END; isid_iter++) {
                    if (VLAN_IN_USE_ON_SWITCH_GET(isid_iter, vid, user_iter)) {
                        // At least one other multi-user is currently using this entry. Can't free it.
                        in_use = TRUE;
                        break;
                    }
                }

                if (in_use) {
                    // No need to iterate any further
                    break;
                }
            }

            if (!in_use) {
                // Free it.
                VLAN_multi_table[vid]->next = VLAN_free_list;
                VLAN_free_list = VLAN_multi_table[vid];
                VLAN_multi_table[vid] = NULL;
            }

#if defined(VLAN_SAME_USER_SUPPORT)
        } else if ((user_idx = VLAN_user_to_same_idx(user)) != VLAN_SAME_CNT) {
            // If #user is a same-user, we must reset the VID to VTSS_VID_NULL if this was the last switch he deleted the VLAN for.
            VLAN_same_table[user_idx].vid = VTSS_VID_NULL;
#endif /* defined(VLAN_SAME_USER_SUPPORT) */
        }
    }

    // Update the new final membership value.
    VLAN_combined_update(isid, vid);
}

/******************************************************************************/
// VLAN_msg_rx()
// VLAN message receive handler.
/******************************************************************************/
static BOOL VLAN_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const u32 isid)
{
    vlan_msg_req_t *msg = (vlan_msg_req_t *)rx_msg; // Plain and simple, when we only have request messages in this module (well, for the large_req, we override it in VLAN_MSG_ID_MEMBERSHIP_SET_REQ, but - by design - the message ID is at the same location)
    vtss_rc        rc   = VTSS_RC_OK;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg->msg_id, VLAN_msg_id_txt(msg->msg_id), len, isid);

    switch (msg->msg_id) {
    case VLAN_MSG_ID_MEMBERSHIP_SET_REQ: {
        // Sets one or more VLAN entries. Contains a BOOL that indicates
        // whether we need to flush the current VLAN table prior to
        // changing the embedded entries.
        vlan_msg_large_req_t *large_msg = (vlan_msg_large_req_t *)rx_msg;
        vtss_vid_t           vid;
        u32                  j = 0, cnt;

        if (large_msg->large_req.set.flush) {
            // We need to delete all currently existing VLANs.
            // This will shortly give rise to no configured VLANs, but that's ignorable.
            T_D("VLAN Table flush");

            for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
                if ((rc = VLAN_membership_api_set(vid, NULL)) != VTSS_RC_OK) {
                    T_E("VLAN_membership_api_set(del, %u): %d", vid, rc);
                    return FALSE;
                }
            }
        }

        cnt = large_msg->large_req.set.cnt;
        if (cnt > VLAN_ENTRY_CNT) {
            T_E("Invalid VLAN cnt (%u)", cnt);
            cnt = VLAN_ENTRY_CNT;
        }

        // Add the requested VLANs
        for (j = 0; j < cnt; j++) {
            // Update the H/W
            vlan_entry_single_switch_with_vid_t *e = &large_msg->large_req.set.table[j];

            T_D("VLAN Add: vid = %u", e->vid);
            if ((rc = VLAN_membership_api_set(e->vid, &e->entry)) != VTSS_RC_OK) {
                T_E("VLAN_membership_api_set(add, %u): %d", e->vid, rc);
            }
        }

        break;
    }

    case VLAN_MSG_ID_PORT_CONF_ALL_SET_REQ: {
        port_iter_t pit;

        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            VLAN_port_conf_api_set(pit.iport, &msg->req.port_conf_all.conf[pit.iport]);
        }
        break;
    }

    case VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ: {
        VLAN_port_conf_api_set(msg->req.port_conf_single.port, &msg->req.port_conf_single.conf);
        break;
    }

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    case VLAN_MSG_ID_CONF_TPID_REQ: {
        vtss_vlan_conf_t conf;

        conf.s_etype = msg->req.custom_s.tpid;
        T_D("EType = 0x%x", conf.s_etype);
        if (vtss_vlan_conf_set(NULL, &conf) != VTSS_RC_OK) {
            T_E("Setting TPID Configuration failed");
        }
        break;
    }
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

    default:
        T_W("unknown message ID: %d", msg->msg_id);
        break;
    }

    return rc == VTSS_RC_OK ? TRUE : FALSE;
}

/******************************************************************************/
// VLAN_msg_rx_register()
// Register for VLAN messages.
/******************************************************************************/
static vtss_rc VLAN_msg_rx_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = VLAN_msg_rx;
    filter.modid = VTSS_MODULE_ID_VLAN;
    return msg_rx_filter_register(&filter);
}

/******************************************************************************/
// VLAN_msg_tx_port_conf_all()
// Transmit whole switch's port configuration.
/******************************************************************************/
static void VLAN_msg_tx_port_conf_all(vtss_isid_t isid)
{
    vlan_msg_req_t *msg;
    port_iter_t    pit;

    msg = VLAN_msg_alloc(VLAN_MSG_ID_PORT_CONF_ALL_SET_REQ, 1);
    VLAN_CRIT_ENTER();
    memcpy(msg->req.port_conf_all.conf, VLAN_port_conf[VLAN_USER_ALL][isid - VTSS_ISID_START], sizeof(msg->req.port_conf_all.conf));
    VLAN_CRIT_EXIT();

    // In order not to iteratively take VLAN_CRIT_ENTER() for each and every port,
    // we use what we have in msg->req.port_conf_all.conf[] when calling back
    // registrants interested in port configuration changes.
    // We have to do that prior to the call to VLAN_msg_tx(), because
    // there's no guarantee that msg is valid anymore after the call.
    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        // Call-back those modules interested in configuration changes on the master.
        VLAN_port_conf_change_callback(isid, pit.iport, &msg->req.port_conf_all.conf[pit.iport]);
    }

    VLAN_msg_tx(msg, isid, sizeof(msg->req.port_conf_all));
}

/******************************************************************************/
// VLAN_msg_tx_port_conf_single()
// Transmit single port's configuration.
/******************************************************************************/
static void VLAN_msg_tx_port_conf_single(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *conf)
{
    vlan_msg_req_t *msg;

    // Only send to existing switches
    if (!msg_switch_exists(isid)) {
        return;
    }

    // Only send to existing ports
    if (port_no >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, port_no)) {
        return;
    }

    msg = VLAN_msg_alloc(VLAN_MSG_ID_PORT_CONF_SINGLE_SET_REQ, 1);
    msg->req.port_conf_single.port = port_no;
    msg->req.port_conf_single.conf = *conf;

    VLAN_msg_tx(msg, isid, sizeof(msg->req.port_conf_single));

    // Call-back those modules interested in configuration changes on the master.
    VLAN_port_conf_change_callback(isid, port_no, conf);
}

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
/******************************************************************************/
// VLAN_msg_tx_tpid_conf()
// Transmit TPID configuraiton to all existing switches in the stack.
/******************************************************************************/
static void VLAN_msg_tx_tpid_conf(vtss_etype_t tpid)
{
    vlan_msg_req_t *msg;
    switch_iter_t  sit;

    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);

    // Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE.
    if ((msg = VLAN_msg_alloc(VLAN_MSG_ID_CONF_TPID_REQ, sit.remaining)) != NULL) {
        msg->req.custom_s.tpid = tpid;
        while (switch_iter_getnext(&sit)) {
            VLAN_msg_tx(msg, sit.isid, sizeof(msg->req.custom_s));
        }
    }
}
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

/******************************************************************************/
// VLAN_msg_tx_membership_all()
// Transmit all VLAN memberships to switch pointed to by #isid,
// which is a legal ISID.
/******************************************************************************/
static void VLAN_msg_tx_membership_all(vtss_isid_t isid)
{
    vlan_ports_t dummy;
    vtss_vid_t   vid = VTSS_VID_NULL;
    vtss_isid_t  zisid = isid - VTSS_ISID_START;

    T_D("enter, isid: %d", isid);

    VLAN_CRIT_ENTER();

    // Start capturing changes.
    VLAN_bulk_begin();

    // The following causes VLAN_bulk_end() to send notifications to all subscribers
    // about all ports (when VLAN_bulk.flush is also set).
    memset(VLAN_bulk.s[zisid].dirty_vids, 0xff, sizeof(VLAN_bulk.s[zisid].dirty_vids));

    // And then the global one for fast iterations in VLAN_bulk_end();
    VLAN_bulk.s[zisid].dirty = TRUE;

    // Since the new switch may have old VLANs installed, we must ask it to flush
    // prior to adding new memberships.
    VLAN_bulk.flush[zisid] = TRUE;

    while ((vid = VLAN_get(isid, vid, VLAN_USER_ALL, TRUE, &dummy, FALSE)) != VTSS_VID_NULL) {
        // This tells VLAN_bulk_end() to transmit new configuration to #isid.
        VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, vid, TRUE);
    }

    // Use vid == VTSS_VID_NULL to indicate that there's something to transmit for this ISID.
    // Whether or not the switch in question has a VID to add, we need to
    // enforce VLAN_bulk_end() to transmit a message so that it gets old VLANs cleared.
    VTSS_BF_SET(VLAN_bulk.s[zisid].tx_conf, VTSS_VID_NULL, TRUE);

    // This is where the real work happens
    VLAN_bulk_end();
    VLAN_CRIT_EXIT();

    T_D("exit, isid: %d", isid);
}

/******************************************************************************/
// VLAN_isid_port_check()
/******************************************************************************/
static vtss_rc VLAN_isid_port_check(vtss_isid_t isid, vtss_port_no_t port_no, BOOL allow_local, BOOL check_port)
{
    if ((isid != VTSS_ISID_LOCAL && !VTSS_ISID_LEGAL(isid)) || (isid == VTSS_ISID_LOCAL && !allow_local)) {
        T_E("Invalid ISID (%u). Allow local = %d", isid, allow_local);
        return VLAN_ERROR_ISID;
    }

    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        // Possibly transient phenomenon
        T_D("Not master");
        return VLAN_ERROR_MUST_BE_MASTER;
    }

    if (check_port && (port_no >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, port_no))) {
        T_E("Invalid port %u", port_no);
        return VLAN_ERROR_PORT;
    }

    return VTSS_RC_OK;
}

#if defined(VLAN_VOLATILE_USER_PRESENT)
/******************************************************************************/
// VLAN_conflict_check()
/******************************************************************************/
static void VLAN_conflict_check(vlan_user_t user, vlan_port_conflicts_t *conflicts, vlan_port_conf_t *resulting_conf, const vlan_port_conf_t *second_conf, u8 flag)
{
    if (flag == VLAN_PORT_FLAGS_PVID) {
        if (second_conf->flags & VLAN_PORT_FLAGS_PVID) {
            // This VLAN user wants to control the port's pvid
            if (resulting_conf->flags & VLAN_PORT_FLAGS_PVID) {
                // So did a previous user.
                if (resulting_conf->pvid != second_conf->pvid) {
                    // And the previous module wanted another value of the pvid
                    // flag than we do.  But since he came before us in the
                    // vlan_user_t enumeration, he wins.
                    conflicts->port_flags |= VLAN_PORT_FLAGS_PVID;
                    conflicts->users[VLAN_PORT_FLAGS_IDX_PVID] |= (1 << (u8)user);
                    T_D("VLAN PVID Conflict");
                } else {
                    // Luckily, the previous module wants to set pvid to the
                    // same value as us. No conflict.
                }
            } else {
                // VLAN pvid not previously overridden, but this user wants
                // to override it.
                resulting_conf->pvid = second_conf->pvid;
                resulting_conf->flags |= VLAN_PORT_FLAGS_PVID; // Overridden
            }
        }
    } else if (flag == VLAN_PORT_FLAGS_AWARE) {
        if (second_conf->flags & VLAN_PORT_FLAGS_AWARE) {
            // This VLAN user wants to control the port's VLAN awareness
            if (resulting_conf->flags & VLAN_PORT_FLAGS_AWARE) {
                // So did a previous user.
                if (resulting_conf->port_type != second_conf->port_type) {
                    // And the previous module wanted another value of the
                    // awareness flag than we do. But since he came before
                    // us in the vlan_user_t enumeration, he wins.
                    conflicts->port_flags |= VLAN_PORT_FLAGS_AWARE;
                    conflicts->users[VLAN_PORT_FLAGS_IDX_AWARE] |= (1 << (u8)user);
                    T_D("VLAN Awareness Conflict");
                } else {
                    // Luckily, the previous module wants to set awareness
                    // to the same value as us. No conflict.
                }
            } else {
                // VLAN awareness not previously overridden, but this user
                // wants to override it.
                resulting_conf->port_type = second_conf->port_type;
                resulting_conf->flags |= VLAN_PORT_FLAGS_AWARE; // Overridden
            }
        }
    } else if (flag == VLAN_PORT_FLAGS_INGR_FILT) {
        if (second_conf->flags & VLAN_PORT_FLAGS_INGR_FILT) {
            // This VLAN user wants to control the port's VLAN ingress_filter
            if (resulting_conf->flags & VLAN_PORT_FLAGS_INGR_FILT) {
                // So did a previous user.
                if (resulting_conf->ingress_filter != second_conf->ingress_filter) {
                    // And the previous module wanted another value of the
                    // ingress_filter flag than we do. But since he came
                    // before us in the vlan_user_t enumeration, he wins.
                    conflicts->port_flags |= VLAN_PORT_FLAGS_INGR_FILT;
                    conflicts->users[VLAN_PORT_FLAGS_IDX_INGR_FILT] |= (1 << (u8)user);
                    T_D("VLAN Ingress Filter Conflict");
                } else {
                    // Luckily, the previous module wants to set ingress_filter
                    // to the same value as us. No conflict.
                }
            } else {
                // VLAN ingress_filter not previously overridden, but this
                // user wants to override it.
                resulting_conf->ingress_filter = second_conf->ingress_filter;
                resulting_conf->flags |= VLAN_PORT_FLAGS_INGR_FILT; // Overridden
            }
        }
    } else if (flag == VLAN_PORT_FLAGS_RX_TAG_TYPE) {
        if (second_conf->flags & VLAN_PORT_FLAGS_RX_TAG_TYPE) {
            // This VLAN user wants to control the port's ingress frame_type
            if (resulting_conf->flags & VLAN_PORT_FLAGS_RX_TAG_TYPE) {
                // So did a previous user.
                if (resulting_conf->frame_type != second_conf->frame_type) {
                    // And the previous module wanted another value of the
                    // ingress frame_type flag than we do. But since he came
                    // before us in the vlan_user_t enumeration, he wins.
                    conflicts->port_flags |= VLAN_PORT_FLAGS_RX_TAG_TYPE;
                    conflicts->users[VLAN_PORT_FLAGS_IDX_RX_TAG_TYPE] |= (1 << (u8)user);
                    T_D("VLAN Frame_Type Conflict");
                } else {
                    // Luckily, the previous module wants to set ingress
                    // frame_type to the same value as us. No conflict.
                }
            } else {
                // VLAN ingress frame_type not previously overridden,
                // but this user wants to override it.
                resulting_conf->frame_type = second_conf->frame_type;
                resulting_conf->flags |= VLAN_PORT_FLAGS_RX_TAG_TYPE; // Overridden
            }
        }
    } else {
        T_E("Invalid input flag");
    }
}
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

#if defined(VLAN_VOLATILE_USER_PRESENT)
/******************************************************************************/
// VLAN_conflict_check_uvid()
/******************************************************************************/
static void VLAN_conflict_check_uvid(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conflicts_t *conflicts, vlan_port_conf_t *resulting_conf)
{
    BOOL             at_least_one_module_wants_to_send_a_specific_vid_tagged = FALSE; // This one is used to figure out whether we're allowed to send all frames untagged or not.
    vlan_user_t      user;
    vlan_port_conf_t temp_conf;

    // Check for .tx_tag_type. In the loop below, resulting_conf->pvid must
    // be resolved already . That's the reason for iterating over the VLAN users
    // in two tempi.
    for (user = (VLAN_USER_STATIC + 1); user < VLAN_USER_ALL; user++) {
        temp_conf = VLAN_port_conf[user][isid - VTSS_ISID_START][port - VTSS_PORT_NO_START];

        if (temp_conf.flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) {
            // This VLAN user wants to control the Tx Tagging on this port
            if (resulting_conf->flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) {
                // So did a higher-prioritized user. Let's see if this gives
                // rise to conflicts.
                switch (resulting_conf->tx_tag_type) {
                case VLAN_TX_TAG_TYPE_UNTAG_THIS:
                    switch (temp_conf.tx_tag_type) {
                    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
                        // A previous module wants to send untagged, and so do we. If the .uvid of
                        // the previous one is the same as ours, we're OK with it.
                        if (resulting_conf->untagged_vid != temp_conf.untagged_vid) {
                            // The previous module wants to untag a specific VLAN ID,
                            // which is not the same as the one we want to untag.
                            if (at_least_one_module_wants_to_send_a_specific_vid_tagged) {
                                // At the same time, another user module wants to send a specific VID
                                // tagged. This is not possible, because we want to resolve the
                                // "two-different-untagged-VIDs" conflict by sending all frames untagged.
                                conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                                conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                            } else {
                                resulting_conf->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_ALL;
                            }
                        }
                        break;

                    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
                        if (at_least_one_module_wants_to_send_a_specific_vid_tagged) {
                            // At the same time, another user module wants to send
                            // a specific VID tagged. This is not possible.
                            conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                            conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        } else {
                            resulting_conf->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_ALL;
                        }
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_THIS:
                        // A previous module wants to send untagged, but we want to send tagged. This is possible if
                        // not two or more previous modules want to send untagged (indicated by .uvid = VTSS_VID_ALL),
                        // and if the .uvid we want to send tagged is different from the .uvid that a previous
                        // user wants to send untagged.
                        if (resulting_conf->untagged_vid == temp_conf.untagged_vid) {
                            conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                            conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        } else {
                            // Do not overwrite resulting_conf->uvid, but keep track of the fact that at least one
                            // module wants to send a specific VID tagged.
                            at_least_one_module_wants_to_send_a_specific_vid_tagged = TRUE;
                        }
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_ALL:
                        // A previous module wants to untag a specific VID, but we want to tag all.
                        // This is impossible.
                        conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                        conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        break;

                    default:
                        T_E("Actual config tx untag this, current config Tx Tag error");
                        return;
                    }
                    break;

                case VLAN_TX_TAG_TYPE_UNTAG_ALL:
                    switch (temp_conf.tx_tag_type) {
                    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
                    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
                        // A previous user wants to send all vids untagged and current user either wants to untag one vid or all vids.
                        break;
                    case VLAN_TX_TAG_TYPE_TAG_THIS:
                    case VLAN_TX_TAG_TYPE_TAG_ALL:
                        // A previous user wants to send all vids untagged. Hence current user cannot tag a vid. Impossible.
                        conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                        conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        break;
                    }
                    break;

                case VLAN_TX_TAG_TYPE_TAG_THIS:
                    switch (temp_conf.tx_tag_type) {
                    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
                        // A previous module wants to send a specific VID tagged, and we want to send a specific VID untagged.
                        // This is possible if the .uvids are not the same.
                        if (resulting_conf->untagged_vid != temp_conf.untagged_vid) {
                            // Here we have to change the tag-type to "untag", and set the uvid to the untagged uvid.
                            resulting_conf->tx_tag_type   = VLAN_TX_TAG_TYPE_UNTAG_THIS;
                            resulting_conf->untagged_vid  = temp_conf.untagged_vid;
                            // And remember that at least one module wanted to send a specific VID tagged.
                            at_least_one_module_wants_to_send_a_specific_vid_tagged = TRUE;
                        } else {
                            // Cannot send the same VID tagged and untagged at the same time.
                            conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                            conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        }
                        break;

                    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
                        // Cannot send the untag_all and tag a vid at the same time.
                        conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                        conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_THIS:
                        if (resulting_conf->untagged_vid != temp_conf.untagged_vid) {
                            // Tell the API to send all frames tagged.
                            resulting_conf->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
                        } else {
                            // Both a previous module and we want to tag a specific VID. No problem.
                        }
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_ALL:
                        // A previous user wants to tag a specific VID, and we want to tag all.
                        // No problem, but change the tx_tag_type to this more restrictive.
                        resulting_conf->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
                        break;

                    default:
                        T_E("Actual config tx tag this, current config Tx Tag error");
                        return;
                    }
                    break;

                case VLAN_TX_TAG_TYPE_TAG_ALL:
                    switch (temp_conf.tx_tag_type) {
                    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
                    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
                        // A previous module wants all VIDs to be tagged, but this module wants a specific VID
                        // or all vids to be untagged. Impossible.
                        conflicts->port_flags |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                        conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] |= (1 << (u8)user);
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_THIS:
                        // A previous module wants all VIDs to be tagged, and we want to tag a specific. No problem.
                        break;

                    case VLAN_TX_TAG_TYPE_TAG_ALL:
                        // Both a previous module and we want to tag all frames. No problem.
                        break;

                    default:
                        T_E("Actual config tx tag all, current config Tx Tag error");
                        return;
                    }
                    break;

                default:
                    T_E("actual config tx tag error");
                    return;
                }
            } else {
                // We're the first user module with Tx tagging requirements
                resulting_conf->tx_tag_type    = temp_conf.tx_tag_type;
                resulting_conf->untagged_vid   = temp_conf.untagged_vid;
                resulting_conf->flags         |= VLAN_PORT_FLAGS_TX_TAG_TYPE;
                if (temp_conf.tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS) {
                    at_least_one_module_wants_to_send_a_specific_vid_tagged = TRUE;
                }
            }
        }
    }
}
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

/******************************************************************************/
// VLAN_port_conflict_resolver()
/******************************************************************************/
static BOOL VLAN_port_conflict_resolver(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conflicts_t *conflicts, vlan_port_conf_t *resulting_conf)
{
    BOOL        changed;
#if defined(VLAN_VOLATILE_USER_PRESENT)
    vlan_user_t user;
#endif /* defined(VLAN_VOLATILE_PRESENT) */

    // Critical section should have been taken by now.
    VLAN_CRIT_ASSERT_LOCKED();

    memset(conflicts, 0, sizeof(*conflicts));

    // Start out with the static user's configuration
    *resulting_conf = VLAN_port_conf[VLAN_USER_STATIC][isid - VTSS_ISID_START][port - VTSS_PORT_NO_START];

    // All the static configuration can be overridden
    resulting_conf->flags = 0;

#if defined(VLAN_VOLATILE_USER_PRESENT)
    // This code is assuming that VLAN_USER_STATIC is the very first in the vlan_user_t enum.
    for (user = (VLAN_USER_STATIC + 1); user < VLAN_USER_ALL; user++) {
        vlan_port_conf_t *temp_conf = &VLAN_port_conf[user][isid - VTSS_ISID_START][port - VTSS_PORT_NO_START];

        // Check for aware conflicts.
        VLAN_conflict_check(user, conflicts, resulting_conf, temp_conf, VLAN_PORT_FLAGS_AWARE);

        // Check for ingress_filter conflicts.
        VLAN_conflict_check(user, conflicts, resulting_conf, temp_conf, VLAN_PORT_FLAGS_INGR_FILT);

        // Check for pvid conflicts.
        VLAN_conflict_check(user, conflicts, resulting_conf, temp_conf, VLAN_PORT_FLAGS_PVID);

        // Check for acceptable frame type conflicts.
        VLAN_conflict_check(user, conflicts, resulting_conf, temp_conf, VLAN_PORT_FLAGS_RX_TAG_TYPE);
    }

    // Finally check untagged_vid and tx_tag_type (VLAN_PORT_FLAGS_TX_TAG_TYPE)
    VLAN_conflict_check_uvid(isid, port, conflicts, resulting_conf);
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    // The final port configuration is now held in resulting_conf.
    // It should always have all the port flags set.
    resulting_conf->flags = VLAN_PORT_FLAGS_ALL;

    changed = memcmp(resulting_conf, &VLAN_port_conf[VLAN_USER_ALL][isid - VTSS_ISID_START][port - VTSS_PORT_NO_START], sizeof(*resulting_conf)) ? TRUE : FALSE;

    // Save it back into the port_conf array as VLAN_USER_ALL.
    VLAN_port_conf[VLAN_USER_ALL][isid - VTSS_ISID_START][port - VTSS_PORT_NO_START] = *resulting_conf;

    return changed;
}

#if defined(VLAN_VOLATILE_USER_PRESENT) && defined(VTSS_SW_OPTION_SYSLOG)
/******************************************************************************/
// VLAN_conflict_log()
// This function compares the previous and present conflicts and logs possible
// conflicts to RAM-based syslog.
/******************************************************************************/
static void VLAN_conflict_log(vlan_port_conflicts_t *prev_conflicts, vlan_port_conflicts_t *current_conflicts)
{
    vlan_user_t usr;

    // This condition checks if there are any PVID conflicts.
    if (current_conflicts->port_flags & VLAN_PORT_FLAGS_PVID) {
        for (usr = (VLAN_USER_STATIC + 1); usr < VLAN_USER_ALL; usr++) {
            if ((current_conflicts->users[VLAN_PORT_FLAGS_IDX_PVID] & (1 << (u8)usr))  && !(prev_conflicts->users[VLAN_PORT_FLAGS_IDX_PVID] & (1 << (u8)usr))) {
                S_E("VLAN Port Configuration PVID Conflict - %s", vlan_mgmt_user_to_txt(usr));
            }
        }
    }

    // This condition checks if there are any Aware conflicts.
    if (current_conflicts->port_flags & VLAN_PORT_FLAGS_AWARE) {
        for (usr = (VLAN_USER_STATIC + 1); usr < VLAN_USER_ALL; usr++) {
            if ((current_conflicts->users[VLAN_PORT_FLAGS_IDX_AWARE] & (1 << (u8)usr))  && !(prev_conflicts->users[VLAN_PORT_FLAGS_IDX_AWARE] & (1 << (u8)usr))) {
                S_E("VLAN Port Configuration Awareness Conflict - %s", vlan_mgmt_user_to_txt(usr));
            }
        }
    }

    // This condition checks if there are any Ingr Filter conflicts.
    if (current_conflicts->port_flags & VLAN_PORT_FLAGS_INGR_FILT) {
        for (usr = (VLAN_USER_STATIC + 1); usr < VLAN_USER_ALL; usr++) {
            if ((current_conflicts->users[VLAN_PORT_FLAGS_IDX_INGR_FILT] & (1 << (u8)usr))  && !(prev_conflicts->users[VLAN_PORT_FLAGS_IDX_INGR_FILT] & (1 << (u8)usr))) {
                S_E("VLAN Port Configuration Ingress Filter Conflict - %s", vlan_mgmt_user_to_txt(usr));
            }
        }
    }

    // This condition checks if there are any Frame Type conflicts.
    if (current_conflicts->port_flags & VLAN_PORT_FLAGS_RX_TAG_TYPE) {
        for (usr = (VLAN_USER_STATIC + 1); usr < VLAN_USER_ALL; usr++) {
            if ((current_conflicts->users[VLAN_PORT_FLAGS_IDX_RX_TAG_TYPE] & (1 << (u8)usr))  && !(prev_conflicts->users[VLAN_PORT_FLAGS_IDX_RX_TAG_TYPE] & (1 << (u8)usr))) {
                S_E("VLAN Port Configuration Frame Type Conflict - %s", vlan_mgmt_user_to_txt(usr));
            }
        }
    }

    // This condition checks if there are any UVID conflicts.
    if (current_conflicts->port_flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) {
        for (usr = (VLAN_USER_STATIC + 1); usr < VLAN_USER_ALL; usr++) {
            if ((current_conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] & (1 << (u8)usr))  && !(prev_conflicts->users[VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE] & (1 << (u8)usr))) {
                S_E("VLAN Port Configuration UVID Conflict - %s", vlan_mgmt_user_to_txt(usr));
            }
        }
    }
}
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) && defined(VTSS_SW_OPTION_SYSLOG) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// VLAN_name_default()
/******************************************************************************/
static void VLAN_name_default(void)
{
    VLAN_CRIT_ENTER();
    memset(VLAN_name_conf, 0, sizeof(VLAN_name_conf));
    VLAN_CRIT_EXIT();
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING) && !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
/******************************************************************************/
// VLAN_name_conf_save()
/******************************************************************************/
static void VLAN_name_conf_save(void)
{
    vlan_name_table_blk_t *vlan_name_blk;
    ulong                 size;
    vtss_vid_t            vid;

    VLAN_CRIT_ASSERT_LOCKED();

    if ((vlan_name_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_VLAN_NAME_TABLE, &size)) == NULL || size != sizeof(*vlan_name_blk)) {
        T_W("Failed to open VLAN Name table");
        return;
    }

    memset(vlan_name_blk, 0, sizeof(*vlan_name_blk));

    vlan_name_blk->size    = sizeof(vlan_name_conf_t);
    vlan_name_blk->count   = 0;
    vlan_name_blk->version = VLAN_NAME_BLK_VERSION;

    // At most the first VLAN_NAME_ENTRY_CNT entries can be saved to flash.
    // This code is getting obsolete as time goes by.
    for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
        if (VLAN_name_conf[vid][0] != '\0') {
            vlan_name_blk->table[vlan_name_blk->count].vid = vid;
            strcpy(vlan_name_blk->table[blk->count].name, VLAN_name_conf[vid]);
            vlan_name_blk->count++;

            if (vlan_name_blk->count == VLAN_NAME_ENTRY_CNT) {
                break;
            }
        }
    }

    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_VLAN_NAME_TABLE);
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) && !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// VLAN_name_conf_read()
/******************************************************************************/
static void VLAN_name_conf_read(BOOL create)
{
    vlan_name_table_blk_t *vlan_name_blk;
    BOOL                  do_create = create;
    ulong                 size;
    u32                   i;

    T_D("enter, create: %d", create);

    // Read/create VLAN table configuration
    if (misc_conf_read_use()) {
        if ((vlan_name_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_VLAN_NAME_TABLE, &size)) == NULL || size != sizeof(*vlan_name_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            vlan_name_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_VLAN_NAME_TABLE, sizeof(*vlan_name_blk));
            do_create = TRUE;
        } else if (vlan_name_blk->version != VLAN_NAME_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        }
    } else {
        vlan_name_blk = NULL;
        do_create     = TRUE;
    }

    // Always create defaults before attempting to load new VLAN names from flash
    VLAN_name_default();

    if (do_create == FALSE && vlan_name_blk != NULL) {

        for (i = 0; i < vlan_name_blk->count; i++) {
            vtss_vid_t vid = vlan_name_blk->table[i].vid;

            if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX || vlan_name_blk->table[i].name[0] == '\0') {
                continue;
            }

            strncpy(VLAN_name_conf[vid], vlan_name_blk->table[i].name, VLAN_NAME_MAX_LEN - 1);
        }
    }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    // Back to flash
    VLAN_CRIT_ENTER();
    VLAN_name_conf_save();
    VLAN_CRIT_EXIT();
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

/******************************************************************************/
// VLAN_port_composite_to_conf()
/******************************************************************************/
static vtss_rc VLAN_port_composite_to_conf(vlan_port_composite_conf_t *composite_conf, vlan_port_conf_t *port_conf)
{
    switch (composite_conf->mode) {
    case VLAN_PORT_MODE_ACCESS:
        port_conf->tx_tag_type    = VLAN_TX_TAG_TYPE_UNTAG_THIS;
        port_conf->frame_type     = VTSS_VLAN_FRAME_ALL;
        port_conf->ingress_filter = TRUE;
        port_conf->port_type      = VLAN_PORT_TYPE_C;
        port_conf->pvid           = composite_conf->access_vid;
        port_conf->untagged_vid   = composite_conf->access_vid;
        break;

    case VLAN_PORT_MODE_TRUNK:
        port_conf->pvid            = composite_conf->native_vid;
        port_conf->untagged_vid    = port_conf->pvid;
        port_conf->port_type       = VLAN_PORT_TYPE_C;
        port_conf->ingress_filter  = TRUE;
        if (composite_conf->tag_native_vlan) {
            port_conf->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
            port_conf->frame_type  = VTSS_VLAN_FRAME_TAGGED;
        } else {
            port_conf->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_THIS;
            port_conf->frame_type  = VTSS_VLAN_FRAME_ALL;
        }
        break;

    case VLAN_PORT_MODE_HYBRID:
        *port_conf                 = composite_conf->hyb_port_conf;
        break;

    default:
        T_E("Que? %d)", composite_conf->mode);
        return VTSS_RC_ERROR;
    }

    port_conf->flags = VLAN_PORT_FLAGS_ALL;

    return VTSS_RC_OK;
}

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
/******************************************************************************/
// VLAN_table_to_flash()
/******************************************************************************/
static u32 VLAN_table_to_flash(vlan_user_t user, vlan_flash_entry_t *flash_table)
{
    u32           cnt = 0;
    switch_iter_t sit;
    vlan_ports_t  entry;
    vtss_vid_t    vid = VTSS_VID_NULL;

    VLAN_CRIT_ENTER();
    // Search globally for the next VID.
    while ((vid = VLAN_get(VTSS_ISID_GLOBAL, vid, user, TRUE, NULL, TRUE)) != VTSS_VID_NULL) {
        // Found one. Now loop through all configurable switches for this VID and copy result to flash block.
        flash_table[cnt].vid = vid;

        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
        while (switch_iter_getnext(&sit)) {
            (void)VLAN_get(sit.isid, vid, user, FALSE, &entry, TRUE);
            memcpy(flash_table[cnt].ports[sit.isid], entry.ports, VTSS_PORT_BF_SIZE);
        }
        cnt++;

        if (cnt == VLAN_ENTRY_CNT) {
            // Full.
            break;
        }
    }
    VLAN_CRIT_EXIT();

    return cnt;
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
/******************************************************************************/
// VLAN_conf_save()
/******************************************************************************/
static void VLAN_conf_save(void)
{
    conf_blk_id_t          blk_id = CONF_BLK_VLAN_TABLE;
    vlan_flash_table_blk_t *blk;

    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("Failed to open VLAN member table");
        return;
    }

    memset(blk, 0, sizeof(*blk));
    blk->size = sizeof(vlan_flash_entry_t);
    blk->version = VLAN_MEMBERSHIP_BLK_VERSION;
    blk->count = VLAN_table_to_flash(VLAN_USER_STATIC, blk->table);

    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
/******************************************************************************/
// VLAN_forbidden_conf_save()
/******************************************************************************/
static void VLAN_forbidden_conf_save(void)
{
    conf_blk_id_t                    blk_id = CONF_BLK_VLAN_FORBIDDEN;
    vlan_flash_forbidden_table_blk_t *blk;

    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("Failed to open VLAN member table");
        return;
    }

    memset(blk, 0, sizeof(*blk));
    blk->size = sizeof(vlan_flash_entry_t);
    blk->version = VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION;
    blk->count = VLAN_table_to_flash(VLAN_USER_FORBIDDEN, blk->table);

    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

/******************************************************************************/
// VLAN_add_del_core()
// #isid: Legal ISID
// #vid:  Legal VID
// #user: [VLAN_USER_STATIC; VLAN_USER_ALL[
/******************************************************************************/
static vtss_rc VLAN_add_del_core(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user, const vlan_ports_t *const ports, vlan_bit_operation_t operation)
{
    vtss_rc      rc;
    u32          i;
    vlan_ports_t resulting_ports; // Don't overwrite caller's ports
    BOOL         delete_rather_than_add = TRUE;
    BOOL         force_delete = ports == NULL;

    VLAN_CRIT_ASSERT_LOCKED();

    resulting_ports.ports[0] = 0; // Satisfy Lint

    if (!force_delete) {
        switch (operation) {
        case VLAN_BIT_OPERATION_OVERWRITE:
            // What's in #ports is what must be written to state.
            resulting_ports = *ports;
            break;

        case VLAN_BIT_OPERATION_ADD:
            // What's in #ports must be added to the current setting.
            (void)VLAN_get(isid, vid, user, FALSE, &resulting_ports, FALSE);
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                resulting_ports.ports[i] |= ports->ports[i];
            }

            delete_rather_than_add = FALSE;
            break;

        case VLAN_BIT_OPERATION_DEL:
            // What's in #ports must be removed from the current settings.
            (void)VLAN_get(isid, vid, user, FALSE, &resulting_ports, FALSE);
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                resulting_ports.ports[i] &= ~ports->ports[i];
            }
            break;

        default:
            T_E("Huh?");
            return VTSS_RC_ERROR;
        }
    }

    if (!force_delete && delete_rather_than_add) {
        // It might now be that the VLAN is empty. If the VLAN is not enabled
        // by the end-user, it is because it has been auto-created by the fact
        // that one or more ports are in trunk or hybrid mode and therefore
        // members of all allowed VLANs.
        if (user == VLAN_USER_STATIC && VTSS_BF_GET(VLAN_end_user_enabled_vlans, vid)) {
            // Nope. This is an end-user-enabled VLAN. Keep adding.
            delete_rather_than_add = FALSE;
        } else {
            // It's not an end-user-enabled VLAN. Check to see if there
            // are any member ports.
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                if (resulting_ports.ports[i]) {
                    // There is at least one member port. Keep adding.
                    delete_rather_than_add = FALSE;
                    break;
                }
            }
        }
    }

    if (force_delete || delete_rather_than_add) {
        VLAN_del(isid, vid, user);
    } else {
        if ((rc = VLAN_add(isid, vid, user, &resulting_ports)) != VTSS_RC_OK) {
            T_E("Huh?");
            // Shouldn't be possible
            return rc;
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_membership_update()
/******************************************************************************/
static vtss_rc VLAN_membership_update(vtss_isid_t isid, vlan_ports_t *ports, vtss_vid_t old_vid, vtss_vid_t new_vid, u8 old_allowed_vids[VLAN_BITMASK_LEN_BYTES], u8 new_allowed_vids[VLAN_BITMASK_LEN_BYTES])
{
    vtss_vid_t           vid, check_vid;
    u8                   *allowed_vids;
    vlan_bit_operation_t oper;

    VLAN_CRIT_ASSERT_LOCKED();

    if ((old_allowed_vids == NULL && new_allowed_vids != NULL) ||
        (old_allowed_vids != NULL && new_allowed_vids == NULL)) {
        // Either going from a single-VID port to a multi-VID port or vice versa.
        BOOL going_to_multi_vid = new_allowed_vids != NULL;

        allowed_vids = old_allowed_vids ? old_allowed_vids : new_allowed_vids;
        check_vid    = old_vid          ? old_vid          : new_vid;

        if (allowed_vids == NULL) {
            // Satisfy Lint
            T_E("Huh?");
            return VTSS_RC_ERROR;
        }

        // Gotta traverse all VIDs and:
        // 1) if going to a multi-VID port, remove old single-VID and add all new multi-VIDs.
        // 2) if going to a single-VID port, remove old multi-VIDs and add single-VID.
        for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
            // Read "if (VTSS_BF_GET() == TRUE)" as "must_be_member" when going to multi-VID and as "was_member" when going to single-VID.
            // Read #check_vid as "old_member_that_must_be_removed" when going to multi-VID and as "new_member_that_must_be_added" when going to single-VID.
            if (VTSS_BF_GET(allowed_vids, vid)) {
                // If going from single to multi VID: Add if not already added (which is the case if vid == check_vid)
                // If going from multi to single VID: Remove unless it's going to be the new VID (which is the case if vid == check_vid).
                if (vid != check_vid) {
                    oper = going_to_multi_vid ? VLAN_BIT_OPERATION_ADD : VLAN_BIT_OPERATION_DEL;
                } else {
                    continue;
                }
            } else if (vid == check_vid) {
                // If going from single to multi VID: Since it's not a member of the new allowed VIDs, delete the old PVID.
                // If going from multi to single VID: Since it wasn't a member before, add it if this is the new PVID.
                if (!going_to_multi_vid) {
                    // Going to single VID. Only add if end-user has enabled this VLAN.
                    if (!VTSS_BF_GET(VLAN_end_user_enabled_vlans, vid)) {
                        continue;
                    }
                }

                oper = going_to_multi_vid ? VLAN_BIT_OPERATION_DEL : VLAN_BIT_OPERATION_ADD;
            } else  {
                continue;
            }

            VTSS_RC(VLAN_add_del_core(isid, vid, VLAN_USER_STATIC, ports, oper));
        }
    } else if (old_allowed_vids == NULL && new_allowed_vids == NULL) {
        // Going from one single-VID to another single-VID mode.
        if (old_vid != new_vid) {
            if (VTSS_BF_GET(VLAN_end_user_enabled_vlans, old_vid)) {
                // Remove membership from old VID.
                VTSS_RC(VLAN_add_del_core(isid, old_vid, VLAN_USER_STATIC, ports, VLAN_BIT_OPERATION_DEL));
            }
            if (VTSS_BF_GET(VLAN_end_user_enabled_vlans, new_vid)) {
                // Add membership of new VID.
                VTSS_RC(VLAN_add_del_core(isid, new_vid, VLAN_USER_STATIC, ports, VLAN_BIT_OPERATION_ADD));
            }
        }
    } else {
        // Last case is when we go from one multi-VID to another multi-VID.
        if (old_allowed_vids == NULL || new_allowed_vids == NULL) {
            // Satisfy Lint
            T_E("Huh?");
            return VTSS_RC_ERROR;
        }

        for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
            BOOL was_member, must_be_member;

            was_member     = VTSS_BF_GET(old_allowed_vids, vid);
            must_be_member = VTSS_BF_GET(new_allowed_vids, vid);

            if (was_member != must_be_member) {
                VTSS_RC(VLAN_add_del_core(isid, vid, VLAN_USER_STATIC, ports, was_member ? VLAN_BIT_OPERATION_DEL : VLAN_BIT_OPERATION_ADD));
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_port_conf_get()
// This function doesn't check isid:port_no on purpose.
/******************************************************************************/
static vtss_rc VLAN_port_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *conf, vlan_user_t user)
{
    if (conf == NULL) {
        return VLAN_ERROR_PARM;
    }

    if (user > VLAN_USER_ALL) {
        return VLAN_ERROR_USER;
    }

    if (user != VLAN_USER_ALL && !vlan_mgmt_user_is_port_conf_changer(user)) {
        T_E("Unexpected user calling this function: %s. Will get allowed, but code should be updated", vlan_mgmt_user_to_txt(user));
        // Continue execution
    }

    // Slaves do not store VLAN port configuration for each VLAN User.
    // Hence, combined information can only be fetched on the slaves from the VTSS API.
    if (isid == VTSS_ISID_LOCAL) {
        if (user != VLAN_USER_ALL) {
            T_E("VLAN User should always be VLAN_USER_ALL when isid is VTSS_ISID_LOCAL");
            return VLAN_ERROR_USER;
        }

        // Get port configuration directly from API.
        if (VLAN_port_conf_api_get(port_no, conf) != VTSS_RC_OK) {
            return VTSS_RC_ERROR;
        }
        return VTSS_RC_OK;
    }

    VLAN_CRIT_ENTER();
    *conf = VLAN_port_conf[user][isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START];
    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_port_conf_check()
/******************************************************************************/
static vtss_rc VLAN_port_conf_check(vlan_port_conf_t *conf, vlan_user_t user)
{
    if (user == VLAN_USER_STATIC && (conf->flags & VLAN_PORT_FLAGS_ALL) != VLAN_PORT_FLAGS_ALL) {
        // The static VLAN user must set all fields in one chunk, because
        // the whole #conf structure is copied to running config in one go.
        // Other VLAN users may have all flags cleared, indicating that they
        // no longer wish to override a particular port feature.
        return VLAN_ERROR_FLAGS;
    }

    if ((conf->flags & VLAN_PORT_FLAGS_PVID) && (conf->pvid < VLAN_ID_MIN || conf->pvid > VLAN_ID_MAX)) {
        return VLAN_ERROR_PVID;
    }

    if (conf->flags & VLAN_PORT_FLAGS_INGR_FILT) {
        // Nothing to do
    }

    if ((conf->flags & VLAN_PORT_FLAGS_RX_TAG_TYPE) && conf->frame_type > VTSS_VLAN_FRAME_UNTAGGED) {
        return VLAN_ERROR_FRAME_TYPE;
    }

    if (conf->flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) {
        if (conf->tx_tag_type > VLAN_TX_TAG_TYPE_UNTAG_ALL) {
            return VLAN_ERROR_TX_TAG_TYPE;
        }

        if ((conf->tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS || conf->tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS) && (conf->untagged_vid < VLAN_ID_MIN || conf->untagged_vid > VLAN_ID_MAX)) {
            return VLAN_ERROR_UVID;
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_port_conf_set()
// This function doesn't check isid:port_no on purpose.
/******************************************************************************/
static vtss_rc VLAN_port_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *new_conf, vlan_user_t user)
{
    vlan_port_conf_t      *old_conf;
    vlan_port_conflicts_t new_conflicts;
    vlan_port_conf_t      resulting_conf;
    BOOL                  changed;
#if defined(VLAN_VOLATILE_USER_PRESENT) && defined(VTSS_SW_OPTION_SYSLOG)
    vlan_port_conflicts_t old_conflicts;
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    T_D("Enter. %d:%d:%d", isid, port_no, user);

    if (user >= VLAN_USER_ALL) {
        return VLAN_ERROR_USER;
    }

    if (!vlan_mgmt_user_is_port_conf_changer(user)) {
        T_E("Unexpected user calling this function: %s. Will get allowed, but code should be updated", vlan_mgmt_user_to_txt(user));
        // Continue execution
    }

    if (new_conf == NULL) {
        return VLAN_ERROR_PARM;
    }

    // Check contents of new_conf
    VTSS_RC(VLAN_port_conf_check(new_conf, user));

    VLAN_CRIT_ENTER();

    // Get the current VLAN port configuration.
    old_conf = &VLAN_port_conf[user][isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START];

    if (memcmp(new_conf, old_conf, sizeof(*new_conf)) == 0) {
        VLAN_CRIT_EXIT();
        return VTSS_RC_OK;
    }

#if defined(VLAN_VOLATILE_USER_PRESENT) && defined(VTSS_SW_OPTION_SYSLOG)
    // Get old port conflicts for syslog purposes
    (void)VLAN_port_conflict_resolver(isid, port_no, &old_conflicts, &resulting_conf);
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    // Time to update our internal state
    *old_conf = *new_conf;

    // Get new port conflicts
    changed = VLAN_port_conflict_resolver(isid, port_no, &new_conflicts, &resulting_conf);

    if (changed) {
        // Pass on this port configuration in the stack. Better do this inside of VLAN_CRIT_xxx()
        // because the actual H/W configuration could otherwise get different from
        // the S/W configuration.
        VLAN_msg_tx_port_conf_single(isid, port_no, &resulting_conf);
    }

    VLAN_CRIT_EXIT();

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    if (changed) {
        // Store the static configuration in flash.
        if (user == VLAN_USER_STATIC) {
            // Save changed configuration
            vlan_flash_port_blk_t *port_blk;
            conf_blk_id_t         blk_id = CONF_BLK_VLAN_PORT_TABLE;

            if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
                T_W("failed to open VLAN port table");
            } else {
                port_blk->conf[isid - VTSS_ISID_START][port_no] = *new_conf;
                conf_sec_close(CONF_SEC_GLOBAL, blk_id);
            }
        }
    }
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */

#if defined(VLAN_VOLATILE_USER_PRESENT) && defined(VTSS_SW_OPTION_SYSLOG)
    // This function will dump the current conflicts to the RAM based syslog.
    VLAN_conflict_log(&old_conflicts, &new_conflicts);
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_port_composite_conf_set()
// This function doesn't check for isid:port_no on purpose.
/******************************************************************************/
static vtss_rc VLAN_port_composite_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_composite_conf_t *new_conf)
{
    vlan_port_composite_conf_t *old_conf;
    vlan_ports_t               ports;
    vtss_isid_t                zisid = isid - VTSS_ISID_START;
    vtss_rc                    rc    = VTSS_RC_OK;
    vlan_port_conf_t           port_conf;
    vtss_vid_t                 old_pvid;

#define EXIT_RC(expr)   { rc = (expr); if (rc != VTSS_RC_OK) {goto do_exit;}}

    if (new_conf == NULL) {
        return VLAN_ERROR_PARM;
    }

    if (new_conf->mode >= VLAN_PORT_MODE_CNT) {
        return VLAN_ERROR_PORT_MODE;
    }

    if (new_conf->access_vid < VLAN_ID_MIN || new_conf->access_vid > VLAN_ID_MAX || new_conf->native_vid < VLAN_ID_MIN || new_conf->native_vid > VLAN_ID_MAX) {
        return VLAN_ERROR_VID;
    }

    // Better set these flags on behalf of caller, since he shouldn't bother.
    new_conf->hyb_port_conf.flags = VLAN_PORT_FLAGS_ALL;

    // Check contents of hybrid port configuration
    VTSS_RC(VLAN_port_conf_check(&new_conf->hyb_port_conf, VLAN_USER_STATIC));

    VLAN_CRIT_ENTER();

    old_conf = &VLAN_port_composite_conf[zisid][port_no];

    if (memcmp(old_conf, new_conf, sizeof(*old_conf)) == 0) {
        VLAN_CRIT_EXIT();
        return VTSS_RC_OK; // Do *NOT* goto do_exit, since that part of the code also applies the configuration.
    }

    // Start capturing changes
    VLAN_bulk_begin();

    // Configuration that is not directly related to a specific VID
    // Snoop into the normal underlying port configuration.
    // Doing it this early avoids a Lint warning that old_pvid and port_conf may not be initialized.
    port_conf       = VLAN_port_conf[VLAN_USER_STATIC][zisid][port_no];
    old_pvid        = port_conf.pvid;
    port_conf.flags = VLAN_PORT_FLAGS_ALL;

    // Prepare the port array with ourselves as the only bit set.
    memset(&ports, 0, sizeof(ports));
    VTSS_BF_SET(ports.ports, port_no, TRUE);

    // Gotta investigate changes one by one, because not only may the
    // port configuration change, but also the VLAN memberships.
    if (old_conf->mode != new_conf->mode) {
        if (old_conf->mode == VLAN_PORT_MODE_ACCESS) {
            // Going from a single-VID mode to a multi-VID mode.
            // Gotta remove membership from the single-VID mode and add membership of all defined VLANs that are allowed in the multi-VID mode in question.
            EXIT_RC(VLAN_membership_update(isid, &ports,
                                           old_conf->access_vid,                                                                    // Old VID
                                           VTSS_VID_NULL,
                                           NULL,
                                           VLAN_allowed_vids[zisid][port_no][new_conf->mode == VLAN_PORT_MODE_TRUNK ? 0 : 1]));     // New allowed VIDs
        } else {
            if (new_conf->mode == VLAN_PORT_MODE_ACCESS) {
                // Going from a multi-VID mode to a single-VID mode.
                // Gotta remove membership from all defined VLANs in the multi-VID mode question and add membership of the single-VID.
                EXIT_RC(VLAN_membership_update(isid, &ports,
                                               VTSS_VID_NULL,
                                               new_conf->access_vid,                                                                // New VID
                                               VLAN_allowed_vids[zisid][port_no][old_conf->mode == VLAN_PORT_MODE_TRUNK ? 0 : 1],   // Old allowed VIDs
                                               NULL));
            } else {
                // Going from one multi-VID mode to another multi-VID mode.
                // Gotta remove membership from all defined VLANs in the old multi-VID mode and add membership to all defined VLANs in the new multi-VID mode.
                EXIT_RC(VLAN_membership_update(isid, &ports,
                                               VTSS_VID_NULL,
                                               VTSS_VID_NULL,
                                               VLAN_allowed_vids[zisid][port_no][old_conf->mode == VLAN_PORT_MODE_TRUNK ? 0 : 1],   // Old allowed VIDs
                                               VLAN_allowed_vids[zisid][port_no][new_conf->mode == VLAN_PORT_MODE_TRUNK ? 0 : 1])); // New allowed VIDs
            }
        }
    } else {
        // We're staying in the same mode, but it could be that the access VID is changing
        if (new_conf->mode == VLAN_PORT_MODE_ACCESS && old_conf->access_vid != new_conf->access_vid) {
            // It is. Gotta change membeship
            EXIT_RC(VLAN_membership_update(isid, &ports,
                                           old_conf->access_vid, // Old VID
                                           new_conf->access_vid, // New VID
                                           NULL,
                                           NULL));
        }
    }

    // Time to update the port configuration

    EXIT_RC(VLAN_port_composite_to_conf(new_conf, &port_conf));

    // And then - finally - time to update our state.
    *old_conf = *new_conf;

#undef EXIT_RC
do_exit:
    VLAN_bulk_end();
    VLAN_CRIT_EXIT();

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    if (port_conf.pvid != old_pvid) {
#if defined(VTSS_SW_OPTION_ACL)
        // RBNTBD: What's the purpose of calling mgmt_vlan_pvid_change()?
        // RBNTBD: If one of the following calls fail, we should revert the VLAN_port_composite_conf[][].XXX
        // RBNTBD: In the previous version of the code, mgmt_vlan_pvid_change() was only called for Trunk and Hybrid modes. Why?
        // RBNTBD: Why is the ACL module not a snooper on port-conf-change?
        // RBNTBD: Is it because it only want's to know STATIC_USER PVID changes?
        if ((rc = mgmt_vlan_pvid_change(isid, port_no, port_conf.pvid, old_pvid)) != VTSS_RC_OK) {
            T_E("%u/%u: %s", isid, port_no, error_txt(rc));
        }
#endif
    }

    return VLAN_port_conf_set(isid, port_no, &port_conf, VLAN_USER_STATIC);
}

/******************************************************************************/
// VLAN_port_composite_allowed_vids_get()
// This function doesn't check for isid:port_no on purpose.
/******************************************************************************/
static vtss_rc VLAN_port_composite_allowed_vids_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 *vid_mask)
{
    if (port_mode != VLAN_PORT_MODE_TRUNK && port_mode != VLAN_PORT_MODE_HYBRID) {
        return VLAN_ERROR_PORT_MODE;
    }

    if (vid_mask == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();
    memcpy(vid_mask, VLAN_allowed_vids[isid - VTSS_ISID_START][port_no][port_mode == VLAN_PORT_MODE_TRUNK ? 0 : 1], sizeof(VLAN_allowed_vids[isid - VTSS_ISID_START][port_no][0]));
    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_port_composite_allowed_vids_set()
// This function doesn't check isid:port_no on purpose.
// #vid_mask is a pointer to array of VLAN_BITMASK_LEN_BYTES bits.
/******************************************************************************/
static vtss_rc VLAN_port_composite_allowed_vids_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 *vid_mask)
{
    vtss_isid_t zisid = isid - VTSS_ISID_START;
    vtss_rc     rc    = VTSS_RC_OK;

#define EXIT_RC(expr)   { rc = (expr); if (rc != VTSS_RC_OK) {goto do_exit;}}

    if (port_mode != VLAN_PORT_MODE_TRUNK && port_mode != VLAN_PORT_MODE_HYBRID) {
        return VLAN_ERROR_PORT_MODE;
    }

    if (vid_mask == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();
    VLAN_bulk_begin();

    // This may give rise to VLAN membership changes.
    if (VLAN_port_composite_conf[zisid][port_no].mode == port_mode) {
        vlan_ports_t ports;

        // Prepare the port array with ourselves as the only bit set.
        memset(&ports, 0, sizeof(ports));
        VTSS_BF_SET(ports.ports, port_no, TRUE);

        // The mode that the user is changing allowed VIDs for is the mode that is currently active.
        EXIT_RC(VLAN_membership_update(isid, &ports,
                                       VTSS_VID_NULL,
                                       VTSS_VID_NULL,
                                       VLAN_allowed_vids[zisid][port_no][port_mode == VLAN_PORT_MODE_TRUNK ? 0 : 1], // Old allowed VIDs
                                       vid_mask));                                                                   // New allowed VIDs
    }

    memcpy(VLAN_allowed_vids[zisid][port_no][port_mode == VLAN_PORT_MODE_TRUNK ? 0 : 1], vid_mask, sizeof(VLAN_allowed_vids[zisid][port_no][0]));

#undef EXIT_RC
do_exit:
    VLAN_bulk_end();
    VLAN_CRIT_EXIT();

    return rc;
}

/******************************************************************************/
// VLAN_port_default()
/******************************************************************************/
static void VLAN_port_default(vtss_isid_t isid, vlan_flash_port_blk_t *port_blk)
{
    vlan_port_composite_conf_t composite_conf;
    u8                         allowed_trunk_vids[VLAN_BITMASK_LEN_BYTES], allowed_hybrid_vids[VLAN_BITMASK_LEN_BYTES];
    switch_iter_t              sit;
    vtss_port_no_t             iport;
    vtss_rc                    rc;

    (void)vlan_mgmt_port_composite_conf_default_get(&composite_conf);
    (void)vlan_mgmt_port_composite_allowed_vids_default_get(VLAN_PORT_MODE_TRUNK,  allowed_trunk_vids);
    (void)vlan_mgmt_port_composite_allowed_vids_default_get(VLAN_PORT_MODE_HYBRID, allowed_hybrid_vids);

    // Default all switches pointed out by #isid, configurable as well as non-configurable
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    while (switch_iter_getnext(&sit)) {
        // Configure all ports, including stack ports.
        // We cannot use the port iterator unless we are master, which we aren't during INIT_CMD_START.
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORTS; iport++) {
            // Set default composite conf
            if ((rc = VLAN_port_composite_conf_set(sit.isid, iport, &composite_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.isid, iport, error_txt(rc));
            }

            // Set default allowed VIDs in trunk mode
            if ((rc = VLAN_port_composite_allowed_vids_set(sit.isid, iport, VLAN_PORT_MODE_TRUNK, allowed_trunk_vids)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.isid, iport, error_txt(rc));
            }

            // Set default allowed VIDs in hybrid mode
            if ((rc = VLAN_port_composite_allowed_vids_set(sit.isid, iport, VLAN_PORT_MODE_HYBRID, allowed_hybrid_vids)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.isid, iport, error_txt(rc));
            }
        }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        if (port_blk) {
            // If the old flash-based configuration is enabled, we gotta
            // save back whatever the default now is. Take it directly
            // from the non-composite port conf.
            if ((rc = VLAN_port_conf_get(sit.isid, iport, &port_blk->conf[sit.isid - VTSS_ISID_START][iport], VLAN_USER_STATIC)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.isid, iport, error_txt(rc));
            }

            port_blk->version = VLAN_PORT_BLK_VERSION;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_VLAN_PORT_TABLE);

        }
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */
    }
}

/******************************************************************************/
// VLAN_port_conf_read()
// If VTSS_ISID_LEGAL(#isid), then create defaults and transmit conf to #isid.
// Otherwise read port conf.
/******************************************************************************/
static void VLAN_port_conf_read(vtss_isid_t isid)
{
    conf_blk_id_t              blk_id;
    vlan_flash_port_blk_t      *port_blk = NULL;
    vlan_port_composite_conf_t composite_conf;
    u8                         allowed_hybrid_vids[VLAN_BITMASK_LEN_BYTES];
    BOOL                       do_create;
    ulong                      size;
    switch_iter_t              sit;
    port_iter_t                pit;
    vtss_rc                    rc;
#if defined(VLAN_VOLATILE_USER_PRESENT)
    vlan_user_t                user;
    vlan_port_conf_t           new_conf;
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    T_D("enter, isid: %d", isid);
    // Read/create VLAN port configuration
    blk_id = CONF_BLK_VLAN_PORT_TABLE;
    if (misc_conf_read_use()) {
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL || size != sizeof(*port_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            port_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_blk));
            do_create = TRUE;
        } else if (port_blk->version != VLAN_PORT_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = VTSS_ISID_LEGAL(isid);
        }
    } else {
        // Not doing silent upgrade => we're using ICFG => Create defaults.
        do_create = TRUE;
    }

    vlan_bulk_update_begin();

#if defined(VLAN_VOLATILE_USER_PRESENT)
    // Whether we create defaults or read from flash, start by defaulting all VLAN users' port configuration.
    // The reason we do it through the VLAN_port_conf_set() function is that we want possible
    // change-subscribers to get notified about any changes that may occur.
    new_conf.flags = 0; // This causes the VLAN module to forget everything about a particular user's override.

    // Here, we only "un"-configure configurable switches, because these are the only ones that can
    // have volatile configuration attached.
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            // Static user is handled separately, because it must use VLAN_port_composite_conf_set().
            for (user = VLAN_USER_STATIC + 1; user < VLAN_USER_ALL; user++) {
                if (!vlan_mgmt_user_is_port_conf_changer(user)) {
                    continue;
                }

                if ((rc = VLAN_port_conf_set(sit.isid, pit.iport, &new_conf, user)) != VTSS_RC_OK) {
                    T_E("%u:%u: Say what? %s", sit.usid, pit.uport, error_txt(rc));
                }
            }
        }
    }
#endif /* defined(VLAN_VOLATILE_USER_PRESENT) */

    if (do_create || port_blk == NULL) {
        // Asked to create defaults or we couldn't open flash block.
        // Either way, do create defaults for static user.
        VLAN_port_default(isid, port_blk);
    } else if (port_blk) {
        // Gotta convert whatever is in flash to the new composite conf format.
        // The thing is that the only possible mapping from the old conf format
        // to the new composite conf format is through the Hybrid mode.
        // The hybrid mode, however, has all VLANs added by default, which is not
        // how the old mode works, and the only way we can control this is through
        // the allowed_vids. So if we load from flash, we start out with no allowed VIDs.
        // Only when the VLANs get created (VLAN_conf_read()), do we add to the allowed VIDs.
        // This is a bit dangerous, because if we can read the port conf, but we can't read
        // the VLAN conf, we will have a problem...
        memset(allowed_hybrid_vids, 0, VLAN_BITMASK_LEN_BYTES);

        (void)vlan_mgmt_port_composite_conf_default_get(&composite_conf);
        composite_conf.mode = VLAN_PORT_MODE_HYBRID;

        // Read all switches pointed out by #isid, configurable as well as non-configurable
        (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        while (switch_iter_getnext(&sit)) {
            // Read all ports, including stack ports. Only when we apply the configuration, we skip stack ports.
            (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                composite_conf.hyb_port_conf = port_blk->conf[sit.isid - VTSS_ISID_START][pit.iport];

                // tx_tag_type == TAG_THIS is an illegal setting for the static user, so set it to tag all.
                if (composite_conf.hyb_port_conf.tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS) {
                    composite_conf.hyb_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
                }

                // Back in the days, there was a Port VLAN Mode called "None", which was meant to
                // cause all frames to be tagged, including PVID. In this mode, tx_tag_type was always set
                // to "Untag PVID" while untagged VID was set to 0.
                // This is an illegal setting nowadays, so if detected, we set tx_tag_type to Tag All and UVID to PVID.
                if (composite_conf.hyb_port_conf.untagged_vid == 0) {
                    composite_conf.hyb_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
                    composite_conf.hyb_port_conf.untagged_vid = composite_conf.hyb_port_conf.pvid;
                }

                if ((rc = VLAN_port_composite_conf_set(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                    T_E("%u:%u: Say what? %s", sit.usid, pit.uport, error_txt(rc));
                }

                // Set allowed VIDs in hybrid mode to none (they get added once we add VLANs in VLAN_conf_read()).
                if ((rc = VLAN_port_composite_allowed_vids_set(sit.isid, pit.iport, VLAN_PORT_MODE_HYBRID, allowed_hybrid_vids)) != VTSS_RC_OK) {
                    T_E("%u:%u: Say what? %s", sit.usid, pit.uport, error_txt(rc));
                }
            }
        }
    }

    vlan_bulk_update_end();
}

/******************************************************************************/
// VLAN_table_init()
// Stitches together the linked list of free VLAN multi-membership entries.
/******************************************************************************/
static void VLAN_table_init(void)
{
    u32 i;

    VLAN_CRIT_ENTER();

    memset(VLAN_table, 0, sizeof(VLAN_table));
    for (i = 0; i < ARRSZ(VLAN_table) - 1; i++) {
        VLAN_table[i].next = &VLAN_table[i + 1];
    }

    VLAN_free_list = &VLAN_table[0];
    VLAN_table[ARRSZ(VLAN_table) - 1].next = NULL;

    VLAN_CRIT_EXIT();
}

/******************************************************************************/
// VLAN_flash_to_table()
/******************************************************************************/
static void VLAN_flash_to_table(vlan_flash_entry_t *flash_entry)
{
    switch_iter_t     sit;
    port_iter_t       pit;
    vtss_vid_t        vid = flash_entry->vid;
    vlan_mgmt_entry_t membership;
    u8                vid_mask[VLAN_BITMASK_LEN_BYTES];
    vtss_rc           rc;

    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        flash_entry->vid = VTSS_VID_NULL; // Indicates invalid/unused entry.
        return;
    }

    // Since there is no direct correlation between the old way of adding a VLAN
    // as member to a port and the new way of doing it, we need to
    // 1) Make sure the VLAN is created and
    // 2) use the allowed_vids to restrict the VLANs that a port can become a member of.
    membership.vid = flash_entry->vid;
    if ((rc = vlan_mgmt_vlan_add(VTSS_ISID_GLOBAL, &membership, VLAN_USER_STATIC)) != VTSS_RC_OK) {
        T_E("Say what? %s", error_txt(rc));
    }

    // Update allowed VIDs. Unfortunately, the flash_entry is per VLAN, whereas
    // allowed_vids is per-port, so this may take a while. Luckily, it's a one-time
    // thing (once updated to ICFG, we never need to do it again).
    // Update all switches (configurable or not) and all ports (stackable or not)
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    while (switch_iter_getnext(&sit)) {
        (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if ((rc = VLAN_port_composite_allowed_vids_get(sit.isid, pit.iport, VLAN_PORT_MODE_HYBRID, vid_mask)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.usid, pit.uport, error_txt(rc));
            }

            if (VTSS_BF_GET(flash_entry->ports[sit.isid], pit.iport)) {
                VTSS_BF_SET(vid_mask, vid, TRUE);
            }

            if ((rc = VLAN_port_composite_allowed_vids_set(sit.isid, pit.iport, VLAN_PORT_MODE_HYBRID, vid_mask)) != VTSS_RC_OK) {
                T_E("%u:%u: Say what? %s", sit.usid, pit.uport, error_txt(rc));
            }
        }
    }
}

/******************************************************************************/
// VLAN_flash_to_forbidden_table()
/******************************************************************************/
static void VLAN_flash_to_forbidden_table(vlan_flash_entry_t *flash_entry)
{
    switch_iter_t     sit;
    vtss_vid_t        vid = flash_entry->vid;
    vlan_mgmt_entry_t membership;
    u64               dummy;
    vtss_rc           rc;

    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        flash_entry->vid = VTSS_VID_NULL; // Indicates invalid/unused entry.
        return;
    }

    membership.vid = vid;
    // Update all switches (configurable or not)
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    while (switch_iter_getnext(&sit)) {
        if (!VLAN_bit_to_bool(sit.isid, (vlan_ports_t *)flash_entry->ports[sit.isid], &membership, &dummy)) {
            // No bits set. Next
            continue;
        }

        if ((rc = vlan_mgmt_vlan_add(sit.isid, &membership, VLAN_USER_FORBIDDEN)) != VTSS_RC_OK) {
            T_E("%u: Say what? %s", sit.usid, error_txt(rc));
        }
    }
}

/******************************************************************************/
// VLAN_conf_read()
// Read/create VLAN stack configuration
/******************************************************************************/
static void VLAN_conf_read(BOOL create)
{
    conf_blk_id_t                    blk_id;
    vlan_flash_table_blk_t           *vlan_blk    = NULL;
    vlan_flash_forbidden_table_blk_t *vlan_fb_blk = NULL;
    u32                              i, vlan_idx;
    BOOL                             do_create = create;
    ulong                            size;
    vlan_mgmt_entry_t                membership;
    vtss_rc                          rc;

    /****************************************************/
    // VLAN CONF
    /****************************************************/
    blk_id = CONF_BLK_VLAN_TABLE;
    if (misc_conf_read_use()) {
        if ((vlan_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL || size != sizeof(*vlan_blk)) {
            T_W("conf_sec_open() failed or size mismatch, creating defaults");
            vlan_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vlan_blk));
            do_create = TRUE;
        } else if (vlan_blk->version != 0 && vlan_blk->version != VLAN_MEMBERSHIP_BLK_VERSION) {
            // Version is neither the newest (VLAN_MEMBERSHIP_BLK_VERSION) nor the older (0), which we still support.
            T_W("Version mismatch (got %u, expected 0 or %d). Creating defaults", vlan_blk->version, VLAN_MEMBERSHIP_BLK_VERSION);
            do_create = TRUE;
        }
    } else {
        // Not doing silent upgrade => we're using ICFG => Create defaults.
        do_create = TRUE;
    }

    vlan_bulk_update_begin();

    // We need to remove all existing VLANs and notify all modules about the removal.
    // Volatile VLAN users expect us to also reset their volatile configuration.
    // The reason we do it through the vlan_mgmt_vlan_del() function is that we want possible
    // change-subscribers to get notified about any changes that may occur.

    for (i = 0; i < 2; i++) {
        // Forbidden VLANs are not removed by removing VLAN_USER_ALL, so we need to do this in two tempi.
        vlan_user_t user = i == 0 ? VLAN_USER_ALL : VLAN_USER_FORBIDDEN;

        membership.vid = VTSS_VID_NULL;

        while (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, membership.vid, &membership, TRUE, user) == VTSS_RC_OK) {
            if ((rc = vlan_mgmt_vlan_del(VTSS_ISID_GLOBAL, membership.vid, user)) != VTSS_RC_OK) {
                T_E("Say what? %u, %s, %s", membership.vid, vlan_mgmt_user_to_txt(user), error_txt(rc));
            }
        }
    }

    if (do_create || vlan_blk == NULL) {
        // Asked to create defaults or we couldn't open flash block.
        // Either way, do create defaults.
        membership.vid = VTSS_VID_DEFAULT;
        if ((rc = vlan_mgmt_vlan_add(VTSS_ISID_GLOBAL, &membership, VLAN_USER_STATIC)) != VTSS_RC_OK) {
            T_E("Say what? %s", error_txt(rc));
        }

        if ((rc = vlan_mgmt_s_custom_etype_set(VLAN_CUSTOM_S_TAG_DEFAULT)) != VTSS_RC_OK) {
            T_E("Say what? %s", error_txt(rc));
        }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        // Gotta update the flash block if it exists.
        if (vlan_blk) {
            vlan_blk->version = VLAN_MEMBERSHIP_BLK_VERSION;
            vlan_blk->count = 1;
            vlan_blk->size = sizeof(vlan_flash_entry_t);
            vlan_blk->table[0].vid = VLAN_ID_DEFAULT;
            // By default, all ports are members of VLAN 1.
            memset(vlan_blk->table[0].ports, 0xff, sizeof(vlan_blk->table[0].ports));

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
            vlan_blk->etype = VLAN_CUSTOM_S_TAG_DEFAULT;
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */
    } else if (vlan_blk) {
        // Gotta create the VLANs that we just read from flash.
        // Back in the days, VLAN membership was controlled per port.
        // In a kind of funny way, this is still the case, but the ports
        // are now configured in hybrid modes and their memberships are
        // controlled through the allowed VIDs for that mode. Therefore,
        // we need to add VLANs we find in the conf block and also add them
        // to the allowed VIDs for all ports on which they are allowed.

        // Add new VLANs
        if (vlan_blk->version == 0) {
            for (i = vlan_blk->count; i != 0; i--) {
                VLAN_flash_to_table(&vlan_blk->table[i - 1]);
            }
        } else {
            // Newer version
            for (vlan_idx = 0; vlan_idx < VLAN_ENTRY_CNT; vlan_idx++) {
                VLAN_flash_to_table(&vlan_blk->table[vlan_idx]);
            }
        }

        vlan_blk->version = VLAN_MEMBERSHIP_BLK_VERSION;

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        if ((rc = vlan_mgmt_s_custom_etype_set(vlan_blk->etype)) != VTSS_RC_OK) {
            T_E("Say what? %s", error_txt(rc));
            vlan_blk->etype = VLAN_CUSTOM_S_TAG_DEFAULT;
        }
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */
    }

    /****************************************************/
    // FORBIDDEN CONF
    /****************************************************/
    blk_id = CONF_BLK_VLAN_FORBIDDEN;
    do_create = create;
    if (misc_conf_read_use()) {
        if ((vlan_fb_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL || size != sizeof(*vlan_fb_blk)) {
            T_W("conf_sec_open() failed or size mismatch, creating defaults");
            vlan_fb_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vlan_fb_blk));
            do_create = TRUE;
        } else if (vlan_fb_blk->version != 0 && vlan_fb_blk->version != VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION) {
            // Version is neither the newest (VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION) nor the older (0), which we still support.
            T_W("Version mismatch (forbidden table, got %u, expected 0 or %d). Creating defaults", vlan_fb_blk->version, VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION);
            do_create = TRUE;
        }
    } else {
        // Not doing silent upgrade => we're using ICFG => Create defaults.
        do_create = TRUE;
    }

    if (do_create) {
        // Default is no forbidden VLANs.
        // This has already been applied at the beginning of this function
        // by removing all forbidden VLANs.

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        if (vlan_fb_blk) {
            memset(vlan_fb_blk, 0, sizeof(*vlan_fb_blk));
            vlan_fb_blk->size = sizeof(vlan_flash_entry_t);
            vlan_fb_blk->version = VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */

    } else if (vlan_fb_blk != NULL) {
        if (vlan_fb_blk->version == 0) {
            if (vlan_fb_blk->count >= VLAN_ENTRY_CNT) {
                // Something serious is wrong. Kill configuration.
                vlan_fb_blk->count = 0;
            }

            for (i = vlan_fb_blk->count; i != 0; i--) {
                VLAN_flash_to_forbidden_table(&vlan_fb_blk->table[i - 1]);
            }
        } else {
            // New version. Gotta traverse all entries. count is not used. Instead, vid indicates whether an entry is valid or not.
            for (vlan_idx = 0; vlan_idx < VLAN_ENTRY_CNT; vlan_idx++) {
                VLAN_flash_to_forbidden_table(&vlan_fb_blk->table[vlan_idx]);
            }
        }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        vlan_fb_blk->version = VLAN_FORBIDDEN_MEMBERSHIP_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */
    }

    vlan_bulk_update_end();
}

/******************************************************************************/
//
// Public functions
//
/******************************************************************************/

/******************************************************************************/
// vlan_error_txt()
/******************************************************************************/
const char *vlan_error_txt(vtss_rc rc)
{
    switch (rc) {
    case VLAN_ERROR_GEN:
        return "VLAN generic error";

    case VLAN_ERROR_PARM:
        return "VLAN parameter error";

    case VLAN_ERROR_ISID:
        return "Invalid Switch ID";

    case VLAN_ERROR_PORT:
        return "Invalid port number";

    case VLAN_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case VLAN_ERROR_NOT_CONFIGURABLE:
        return "Switch not configurable";

    case VLAN_ERROR_USER:
        return "Invalid VLAN User";

    case VLAN_ERROR_VID:
        return "Invalid VLAN ID";

    case VLAN_ERROR_FLAGS:
        return "At least one field to override must be set";

    case VLAN_ERROR_PVID:
        return "Invalid Port VLAN ID";

    case VLAN_ERROR_FRAME_TYPE:
        return "Invalid frame type";

    case VLAN_ERROR_TX_TAG_TYPE:
        return "Invalid Tx tag type";

    case VLAN_ERROR_UVID:
        return "Invalid untagged VID";

    case VLAN_ERROR_TPID:
        return "Invalid Custom-S TPID";

    case VLAN_ERROR_PORT_MODE:
        return "Invalid port mode";

    case VLAN_ERROR_ENTRY_NOT_FOUND:
        return "Entry not found";

    case VLAN_ERROR_VLAN_TABLE_FULL:
        return "VLAN table full";

    case VLAN_ERROR_USER_PREVIOUSLY_CONFIGURED:
        return "Previously configured";

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
    case VLAN_ERROR_NAME_ALREADY_EXISTS:
        return "A VLAN with that name already exists";

    case VLAN_ERROR_NAME_RESERVED:
        return "The VLAN name is reserved for another VLAN ID";

    case VLAN_ERROR_NAME_DEFAULT_VLAN:
        return "The default VLAN's name cannot be changed";

    case VLAN_ERROR_NAME_DOES_NOT_EXIST:
        return "VLAN name does not exist";
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

    default:
        return "Unknown VLAN error";
    }
}

/******************************************************************************/
// vlan_mgmt_port_composite_conf_default_get()
// Returns a default composite VLAN configuration.
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_conf_default_get(vlan_port_composite_conf_t *conf)
{
    if (conf == NULL) {
        return VLAN_ERROR_PARM;
    }

    // These are the default port mode values
    conf->mode                         = VLAN_PORT_MODE_ACCESS;
    conf->access_vid                   = VLAN_ID_DEFAULT;
    conf->native_vid                   = VLAN_ID_DEFAULT;
    conf->tag_native_vlan              = FALSE;
    conf->hyb_port_conf.pvid           = VLAN_ID_DEFAULT;
    conf->hyb_port_conf.untagged_vid   = conf->hyb_port_conf.pvid;
    conf->hyb_port_conf.frame_type     = VTSS_VLAN_FRAME_ALL;
    conf->hyb_port_conf.ingress_filter = FALSE;
    conf->hyb_port_conf.tx_tag_type    = VLAN_TX_TAG_TYPE_UNTAG_THIS;
    conf->hyb_port_conf.port_type      = VLAN_PORT_TYPE_C;
    conf->hyb_port_conf.flags          = VLAN_PORT_FLAGS_ALL;

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_vid_bitmask_to_txt()
// Inspired by mgmt_list2txt()
/******************************************************************************/
char *vlan_mgmt_vid_bitmask_to_txt(u8 bitmask[VLAN_BITMASK_LEN_BYTES], char *txt)
{
    vtss_vid_t vid;
    BOOL       member, first = TRUE;
    u32        count = 0;
    char       *p = txt;

    txt[0] = '\0';

    for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
        member = VTSS_BF_GET(bitmask, vid);

        if ((member && (count == 0 || vid == VLAN_ID_MAX)) || (!member && count > 1)) {
            p += sprintf(p, "%s%d",
                         first ? "" : count > (member ? 1 : 2) ? "-" : ",",
                         member ? vid : vid - 1);
            first = FALSE;
        }

        count = member ? count + 1 : 0;
    }

    return txt;
}

/******************************************************************************/
// vlan_mgmt_bitmasks_identical()
// This function relies heavily on the fact that VTSS_BF_GET()/VTSS_BF_SET()
// macros place e.g. index 0, 8, 16, 24 into bit 0 of a given byte,
// index 1, 9, 17, 25, etc. into bit 1 of a given byte, and so on.
/******************************************************************************/
BOOL vlan_mgmt_bitmasks_identical(u8 *bitmask1, u8 *bitmask2)
{
    vtss_vid_t vid;

    if (bitmask1 == NULL || bitmask2 == NULL) {
        T_E("Invalid argument(s)");
        return FALSE;
    }

    // Now be careful in the comparison.
    //
    // Bits from [0 to VLAN_ID_MIN[ are not used, and should not
    // be part of the comparison. The following computes the number
    // of bytes to skip at the beginning of the bit array.
#define VLAN_HEAD_BYTES_TO_SKIP ((VLAN_ID_MIN + 7) / 8)
    //
    // Also, bits from ]VLAN_ID_MAX; next-multiple-of-8[ are also
    // not used, and should therefore not be part of the comparison.
    // If not all bits are used in the last byte, we skip the last byte
    // alltogether.
#if ((VLAN_ID_MAX + 1) % 8) == 0
#define VLAN_TAIL_BYTES_TO_SKIP 0
#else
#define VLAN_TAIL_BYTES_TO_SKIP 1
#endif

#if VLAN_BITMASK_LEN_BYTES > VLAN_HEAD_BYTES_TO_SKIP + VLAN_TAIL_BYTES_TO_SKIP
    // Only call memcmp() if the number of fully valid bytes is greater than zero.
    if (memcmp(bitmask1 + VLAN_HEAD_BYTES_TO_SKIP, bitmask2 + VLAN_HEAD_BYTES_TO_SKIP, VLAN_BITMASK_LEN_BYTES - VLAN_HEAD_BYTES_TO_SKIP - VLAN_TAIL_BYTES_TO_SKIP) != 0) {
        return FALSE;
    }
#endif /* VLAN_BITMASK_LEN_BYTES > VLAN_HEAD_BYTES_TO_SKIP + VLAN_TAIL_BYTES_TO_SKIP */

#if (VLAN_ID_MIN % 8) != 0
    // VLAN_ID_MIN is not located on bit 0 of a given byte.
    // Gotta check from [VLAN_ID_MIN; next-byte-boundary[ manually
    for (vid = VLAN_ID_MIN; vid < 8 * ((VLAN_ID_MIN + 7) / 8); vid++) {
        if (VTSS_BF_GET(bitmask1, vid) != VTSS_BF_GET(bitmask2, vid)) {
            return FALSE;
        }
    }
#endif /* (VLAN_ID_MIN % 8) != 0 */

#if VLAN_TAIL_BYTES_TO_SKIP != 0
    // VLAN_ID_MAX is not located on bit 7 of a given byte.
    // Gotta check from ]prev-byte_boundary; VLAN_ID_MAX] manually.
    for (vid = 8 * (VLAN_ID_MAX / 8); vid <= VLAN_ID_MAX; vid++) {
        if (VTSS_BF_GET(bitmask1, vid) != VTSS_BF_GET(bitmask2, vid)) {
            return FALSE;
        }
    }
#endif /* VLAN_TAIL_BYTES_TO_SKIP != 0 */

    return TRUE;
}

/******************************************************************************/
// vlan_mgmt_user_to_txt()
/******************************************************************************/
const char *vlan_mgmt_user_to_txt(vlan_user_t user)
{
    switch (user) {
    case VLAN_USER_STATIC:
        return "Admin";
#if defined(VTSS_SW_OPTION_DOT1X)
    case VLAN_USER_DOT1X:
        return "NAS";
#endif /* defined(VTSS_SW_OPTION_DOT1X) */
#if defined(VTSS_SW_OPTION_MSTP)
    case VLAN_USER_MSTP:
        return "MSTP";
#endif /* defined(VTSS_SW_OPTION_MSTP) */




#if defined(VTSS_SW_OPTION_GVRP)
    case VLAN_USER_GVRP:
        return "GVRP";
#endif /* defined(VTSS_SW_OPTION_GVRP) */
#if defined(VTSS_SW_OPTION_MVR)
    case VLAN_USER_MVR:
        return "MVR";
#endif /* defined(VTSS_SW_OPTION_MVR) */
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
    case VLAN_USER_VOICE_VLAN:
        return "Voice VLAN";
#endif /* defined(VTSS_SW_OPTION_VOICE_VLAN) */
#if defined(VTSS_SW_OPTION_ERPS)
    case VLAN_USER_ERPS:
        return "ERPS";
#endif /* defined(VTSS_SW_OPTION_ERPS) */
#if defined(VTSS_SW_OPTION_MEP)
    case VLAN_USER_MEP:
        return "MEP";
#endif /* defined(VTSS_SW_OPTION_MEP) */
#if defined(VTSS_SW_OPTION_EVC)
    case VLAN_USER_EVC:
        return "EVC";
#endif /* defined(VTSS_SW_OPTION_EVC) */
#if defined(VTSS_SW_OPTION_VCL)
    case VLAN_USER_VCL:
        return "VCL";
#endif /* defined(VTSS_SW_OPTION_VCL) */
    case VLAN_USER_FORBIDDEN:
        return "Forbidden VLANs";
    case VLAN_USER_ALL:
        return "Combined";
    case VLAN_USER_CNT:
    default:
        T_E("Invoked with user = %u", user);
        return "Unknown";
    }
}

/******************************************************************************/
// vlan_mgmt_port_type_to_txt()
/******************************************************************************/
const char *vlan_mgmt_port_type_to_txt(vlan_port_type_t port_type)
{
    switch (port_type) {
    case VLAN_PORT_TYPE_UNAWARE:
        return "Unaware";
    case VLAN_PORT_TYPE_C:
        return "C-Port";
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    case VLAN_PORT_TYPE_S:
        return "S-Port";
    case VLAN_PORT_TYPE_S_CUSTOM:
        return "S-Custom-Port";
#endif
    default:
        return "Unknown";
    }
}

/******************************************************************************/
// vlan_mgmt_frame_type_to_txt()
// Acceptable frame type.
/******************************************************************************/
const char *vlan_mgmt_frame_type_to_txt(vtss_vlan_frame_t frame_type)
{
    switch (frame_type) {
    case VTSS_VLAN_FRAME_ALL:
        return "All";
    case VTSS_VLAN_FRAME_TAGGED:
        return "Tagged";
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    case VTSS_VLAN_FRAME_UNTAGGED:
        return "Untagged";
#endif
    default:
        return "Unknown";
    }
}

/******************************************************************************/
// vlan_mgmt_tx_tag_type_to_txt()
/******************************************************************************/
const char *vlan_mgmt_tx_tag_type_to_txt(vlan_tx_tag_type_t tx_tag_type, BOOL can_be_any_uvid)
{
    switch (tx_tag_type) {
    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
        return can_be_any_uvid ? "Untag UVID" : "Untag PVID";
    case VLAN_TX_TAG_TYPE_TAG_THIS:
        return can_be_any_uvid ? "Tag UVID"   : "Tag PVID";
    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
        return "Untag All";
    case VLAN_TX_TAG_TYPE_TAG_ALL:
        return "Tag All";
    }
    return "Unknown";
}

/******************************************************************************/
// vlan_mgmt_user_is_port_conf_changer()
/******************************************************************************/
BOOL vlan_mgmt_user_is_port_conf_changer(vlan_user_t user)
{
    switch (user) {
    case VLAN_USER_STATIC:
#if defined(VTSS_SW_OPTION_DOT1X)
    case VLAN_USER_DOT1X:
#endif /* defined(VTSS_SW_OPTION_DOT1X) */
#if defined(VTSS_SW_OPTION_MVR)
    case VLAN_USER_MVR:
#endif /* defined(VTSS_SW_OPTION_MVR) */
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
    case VLAN_USER_VOICE_VLAN:
#endif /* defined(VTSS_SW_OPTION_VOICE_VLAN) */
#if defined(VTSS_SW_OPTION_MSTP)
    case VLAN_USER_MSTP:
#endif /* defined(VTSS_SW_OPTION_MSTP) */
#if defined(VTSS_SW_OPTION_ERPS)
    case VLAN_USER_ERPS:
#endif /* defined(VTSS_SW_OPTION_ERPS) */
#if defined(VTSS_SW_OPTION_VCL)
    case VLAN_USER_VCL:
#endif /* defined(VTSS_SW_OPTION_VCL) */
#if defined(VTSS_SW_OPTION_EVC)
    case VLAN_USER_EVC:
#endif /* defined(VTSS_SW_OPTION_EVC) */
#if defined(VTSS_SW_OPTION_GVRP)
    case VLAN_USER_GVRP:
#endif /* defined(VTSS_SW_OPTION_GVRP) */
        return TRUE;

    default:
        return FALSE;
    }
}

/******************************************************************************/
// vlan_mgmt_user_is_membership_changer()
/******************************************************************************/
BOOL vlan_mgmt_user_is_membership_changer(vlan_user_t user)
{
    if (VLAN_user_to_multi_idx(user) != VLAN_MULTI_CNT) {
        return TRUE; // Allowed.
    }

    if (user == VLAN_USER_FORBIDDEN) {
        return TRUE; // Allowed.
    }

#if defined(VLAN_SAME_USER_SUPPORT)
    if (VLAN_user_to_same_idx(user) != VLAN_SAME_CNT) {
        return TRUE; // Allowed.
    }
#endif /* defined(VLAN_SAME_USER_SUPPORT) */

#if defined(VLAN_SINGLE_USER_SUPPORT)
    if (VLAN_user_to_single_idx(user) != VLAN_SINGLE_CNT) {
        return TRUE; // Allowed.
    }
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

    // Wasn't any of the allowed users.
    return FALSE;
}

/******************************************************************************/
// vlan_port_conf_change_register()
// Some modules need to know when vlan configuration changes. For
// doing this a callback function is provided. At the time of
// writing there is no need for being able to unregister.
/******************************************************************************/
void vlan_port_conf_change_register(vtss_module_id_t modid, vlan_port_conf_change_callback_t cb, BOOL cb_on_master)
{
    u32 i;

    if (cb == NULL) {
        T_EG(TRACE_GRP_CB, "Invalid parameter, cb");
        return;
    }

    if (modid < 0 || modid >= VTSS_MODULE_ID_NONE) {
        T_EG(TRACE_GRP_CB, "Invalid module ID, %d", modid);
        return;
    }

    T_DG(TRACE_GRP_CB, "%s: Registering for %s VLAN port configuration changes", vtss_module_names[modid], cb_on_master ? "master" : "local switch");

    VLAN_CB_CRIT_ENTER();
    for (i = 0; i < ARRSZ(VLAN_pcc_cb); i++) {
        if (VLAN_pcc_cb[i].cb == NULL) {
            VLAN_pcc_cb[i].modid        = modid;
            VLAN_pcc_cb[i].cb           = cb;
            VLAN_pcc_cb[i].cb_on_master = cb_on_master;
            break;
        }
    }
    VLAN_CB_CRIT_EXIT();

    if (i == ARRSZ(VLAN_pcc_cb)) {
        T_EG(TRACE_GRP_CB, "Ran out of registration entries");
    }
}

/******************************************************************************/
// vlan_membership_change_register()
/******************************************************************************/
void vlan_membership_change_register(vtss_module_id_t modid, vlan_membership_change_callback_t cb)
{
    u32 i;

    if (cb == NULL) {
        T_EG(TRACE_GRP_CB, "Invalid parameter, cb");
        return;
    }

    if (modid < 0 || modid >= VTSS_MODULE_ID_NONE) {
        T_EG(TRACE_GRP_CB, "Invalid module ID, %d", modid);
        return;
    }

    T_DG(TRACE_GRP_CB, "%s: Registering for VLAN membership changes", vtss_module_names[modid]);

    VLAN_CB_CRIT_ENTER();
    for (i = 0; i < ARRSZ(VLAN_mc_cb); i++) {
        if (VLAN_mc_cb[i].u.cb == NULL) {
            VLAN_mc_cb[i].modid = modid;
            VLAN_mc_cb[i].u.cb   = cb;
            break;
        }
    }
    VLAN_CB_CRIT_EXIT();

    if (i == ARRSZ(VLAN_mc_cb)) {
        T_EG(TRACE_GRP_CB, "Ran out of registration entries");
    }
}

/******************************************************************************/
// vlan_membership_bulk_change_register()
/******************************************************************************/
void vlan_membership_bulk_change_register(vtss_module_id_t modid, vlan_membership_bulk_change_callback_t cb)
{
    u32 i;

    if (cb == NULL) {
        T_EG(TRACE_GRP_CB, "Invalid parameter, cb");
        return;
    }

    if (modid < 0 || modid >= VTSS_MODULE_ID_NONE) {
        T_EG(TRACE_GRP_CB, "Invalid module ID, %d", modid);
        return;
    }

    T_DG(TRACE_GRP_CB, "%s: Registering for *bulk* VLAN membership changes", vtss_module_names[modid]);

    VLAN_CB_CRIT_ENTER();
    for (i = 0; i < ARRSZ(VLAN_mc_bcb); i++) {
        if (VLAN_mc_bcb[i].u.bcb == NULL) {
            VLAN_mc_bcb[i].modid = modid;
            VLAN_mc_bcb[i].u.bcb = cb;
            break;
        }
    }
    VLAN_CB_CRIT_EXIT();

    if (i == ARRSZ(VLAN_mc_bcb)) {
        T_EG(TRACE_GRP_CB, "Ran out of registration entries");
    }
}

/******************************************************************************/
// vlan_bulk_update_begin()
/******************************************************************************/
void vlan_bulk_update_begin(void)
{
    VLAN_CRIT_ENTER();
    VLAN_bulk_begin();
    VLAN_CRIT_EXIT();
}

/******************************************************************************/
// vlan_bulk_update_end()
/******************************************************************************/
void vlan_bulk_update_end(void)
{
    VLAN_CRIT_ENTER();
    VLAN_bulk_end();
    VLAN_CRIT_EXIT();
}

/******************************************************************************/
// vlan_bulk_update_ref_cnt_get()
/******************************************************************************/
u32 vlan_bulk_update_ref_cnt_get(void)
{
    u32 result;
    VLAN_CRIT_ENTER();
    result = VLAN_bulk.ref_cnt;
    VLAN_CRIT_EXIT();

    return result;
}

/******************************************************************************/
// vlan_s_custom_etype_change_register()
/******************************************************************************/
void vlan_s_custom_etype_change_register(vtss_module_id_t modid, vlan_s_custom_etype_change_callback_t cb)
{
    u32 i;

    if (cb == NULL) {
        T_EG(TRACE_GRP_CB, "Invalid parameter, cb");
        return;
    }

    if (modid < 0 || modid >= VTSS_MODULE_ID_NONE) {
        T_EG(TRACE_GRP_CB, "Invalid module ID, %d", modid);
        return;
    }

    T_DG(TRACE_GRP_CB, "%s: Registering for S-custom EtherTYpe changes", vtss_module_names[modid]);

    VLAN_CB_CRIT_ENTER();
    for (i = 0; i < ARRSZ(VLAN_s_custom_etype_cb); i++) {
        if (VLAN_s_custom_etype_cb[i].u.scb == NULL) {
            VLAN_s_custom_etype_cb[i].modid = modid;
            VLAN_s_custom_etype_cb[i].u.scb = cb;
            break;
        }
    }
    VLAN_CB_CRIT_EXIT();

    if (i == ARRSZ(VLAN_s_custom_etype_cb)) {
        T_EG(TRACE_GRP_CB, "Ran out of registration entries");
    }
}

/******************************************************************************/
// vlan_mgmt_port_conf_get()
// Only accessing switch module linked list
/******************************************************************************/
vtss_rc vlan_mgmt_port_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *conf, vlan_user_t user)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, TRUE, TRUE));

    return VLAN_port_conf_get(isid, port_no, conf, user);
}

/******************************************************************************/
// vlan_mgmt_port_conf_set()
/******************************************************************************/
vtss_rc vlan_mgmt_port_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_conf_t *new_conf, vlan_user_t user)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, FALSE, TRUE));

    if (!msg_switch_configurable(isid)) {
        return VLAN_ERROR_NOT_CONFIGURABLE;
    }

    return VLAN_port_conf_set(isid, port_no, new_conf, user);
}

/******************************************************************************/
// vlan_mgmt_port_composite_conf_get()
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_composite_conf_t *conf)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, FALSE, TRUE));

    if (conf == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();
    *conf = VLAN_port_composite_conf[isid - VTSS_ISID_START][port_no];
    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_port_composite_conf_set()
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_composite_conf_t *new_conf)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, FALSE, TRUE));

    if (!msg_switch_configurable(isid)) {
        return VLAN_ERROR_NOT_CONFIGURABLE;
    }

    return VLAN_port_composite_conf_set(isid, port_no, new_conf);
}

/******************************************************************************/
// vlan_mgmt_port_composite_allowed_vids_default_get()
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_allowed_vids_default_get(vlan_port_mode_t port_mode, u8 *vid_mask)
{
    if (port_mode != VLAN_PORT_MODE_TRUNK && port_mode != VLAN_PORT_MODE_HYBRID) {
        T_E("Invalid port mode (%d)", port_mode);
        return VLAN_ERROR_PORT_MODE;
    }

    if (vid_mask == NULL) {
        T_E("Invalid vid_mask");
        return VLAN_ERROR_PARM;
    }

    // #port_mode is for future use in case there should be difference between trunk and hybrid defaults
    memset(vid_mask, 0xFF, VLAN_BITMASK_LEN_BYTES);

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_port_composite_allowed_vids_get()
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_allowed_vids_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 *vid_mask)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, FALSE, TRUE));

    return VLAN_port_composite_allowed_vids_get(isid, port_no, port_mode, vid_mask);
}

/******************************************************************************/
// vlan_mgmt_port_composite_allowed_vids_set()
// #vid_mask is a pointer to array of VLAN_BITMASK_LEN_BYTES bits.
/******************************************************************************/
vtss_rc vlan_mgmt_port_composite_allowed_vids_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 *vid_mask)
{
    VTSS_RC(VLAN_isid_port_check(isid, port_no, FALSE, TRUE));

    return VLAN_port_composite_allowed_vids_set(isid, port_no, port_mode, vid_mask);
}

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
/******************************************************************************/
// vlan_mgmt_s_custom_etype_get()
/******************************************************************************/
vtss_rc vlan_mgmt_s_custom_etype_get(vtss_etype_t *tpid)
{
    if (tpid == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();
    *tpid = VLAN_tpid_s_custom_port;
    VLAN_CRIT_EXIT();
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
/******************************************************************************/
// vlan_mgmt_s_custom_etype_set()
/******************************************************************************/
vtss_rc vlan_mgmt_s_custom_etype_set(vtss_etype_t tpid)
{
    BOOL update = FALSE;

    T_I("TPID = 0x%x", tpid);

    if (tpid < 0x600) {
        T_E("EtherType should always be greater than or equal to 0x0600");
        return VLAN_ERROR_TPID;
    }

    VLAN_CRIT_ENTER();
    if (tpid != VLAN_tpid_s_custom_port) {
        VLAN_tpid_s_custom_port = tpid;
        update = TRUE;
    }
    VLAN_CRIT_EXIT();

    if (update) {
        // Send the configuration to all switches in the stack
        VLAN_msg_tx_tpid_conf(tpid);
        VLAN_s_custom_etype_change_callback(tpid);
    }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    if (update) {
        conf_blk_id_t          blk_id = CONF_BLK_VLAN_TABLE;
        vlan_flash_table_blk_t *vlan_blk;

        // Save TPID to flash
        if ((vlan_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open VLAN table");
            return VLAN_ERROR_GEN;
        }

        vlan_blk->etype = tpid;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

/******************************************************************************/
// vlan_mgmt_vlan_get()
/******************************************************************************/
vtss_rc vlan_mgmt_vlan_get(vtss_isid_t isid, vtss_vid_t vid, vlan_mgmt_entry_t *membership, BOOL next, vlan_user_t user)
{
    vlan_ports_t entry;
    vtss_rc      rc = VTSS_RC_OK;

    if (isid == VTSS_ISID_GLOBAL) {
        // Allow VTSS_ISID_GLOBAL, but only on master
        if (!msg_switch_is_master()) {
            return VLAN_ERROR_MUST_BE_MASTER;
        }
    } else {
        VTSS_RC(VLAN_isid_port_check(isid, VTSS_PORT_NO_START, TRUE, FALSE));
    }

    if (next) {
        // When asking for next VID, allow all positive VIDs,
        // but shortcut the function if it's at or greater than the
        // maximum possible VID.
        if (vid >= VLAN_ID_MAX) {
            return VLAN_ERROR_ENTRY_NOT_FOUND;
        }
    } else if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        // If asking for a specific VID, only allow VIDs in range.
        return VLAN_ERROR_VID;
    }

    if (membership == NULL || user > VLAN_USER_ALL) {
        return VLAN_ERROR_PARM;
    }

    if (isid == VTSS_ISID_LOCAL && user != VLAN_USER_ALL) {
        // When requesting what's in H/W, #user must be VLAN_USER_ALL
        T_E("When called with VTSS_ISID_LOCAL, the user must be VLAN_USER_ALL and not user=%s", vlan_mgmt_user_to_txt(user));
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();

    if (isid == VTSS_ISID_LOCAL) {
        // Read the H/W.
        if ((vid = VLAN_api_get(vid, next, &entry)) == VTSS_VID_NULL) {
            rc = VLAN_ERROR_ENTRY_NOT_FOUND;
        }
    } else {
        // Now handle legal ISIDs and VTSS_ISID_GLOBAL.
        if ((vid = VLAN_get(isid, vid, user, next, &entry, TRUE)) == VTSS_VID_NULL) {
            rc = VLAN_ERROR_ENTRY_NOT_FOUND;
        }
    }

    VLAN_CRIT_EXIT();

    membership->vid = vid;

    if (isid == VTSS_ISID_GLOBAL) {
        // Entries are not filled in when invoked with this ISID.
        memset(membership->ports, 0, sizeof(membership->ports));
    } else {
        u64 dummy;
        (void)VLAN_bit_to_bool(isid, &entry, membership, &dummy);
    }

    return rc;
}

/******************************************************************************/
// vlan_mgmt_vlan_add()
// VLAN members as boolean port list.
// #isid must be legal or VTSS_ISID_GLOBAL.
//
// The VLAN will be added to all switches, but on some switches it may be
// with zero member ports.
/******************************************************************************/
vtss_rc vlan_mgmt_vlan_add(vtss_isid_t isid, vlan_mgmt_entry_t *membership, vlan_user_t user)
{
    switch_iter_t sit;
    vtss_vid_t    vid;
    vtss_rc       rc = VTSS_RC_OK;

    if (membership == NULL) {
        return VLAN_ERROR_PARM;
    }

    // If #user != VLAN_USER_STATIC, an empty port set actually means "delete VLAN".
    // In this case, we only "delete" the VLAN globally if the user doesn't have shares
    // on other switches.
    if (user != VLAN_USER_STATIC) {
        port_iter_t pit;
        BOOL        at_least_one_port_included = FALSE;

        if (isid == VTSS_ISID_GLOBAL) {
            vtss_port_no_t iport;

            // Don't know which switch the user means, so we have to traverse all
            // ports, and not just those that are present on the switch in question.
            for (iport = 0; iport < ARRSZ(membership->ports); iport++) {
                if (membership->ports[iport]) {
                    at_least_one_port_included = TRUE;
                    break;
                }
            }
        } else {
            (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (membership->ports[pit.iport]) {
                    at_least_one_port_included = TRUE;
                    break;
                }
            }
        }

        if (!at_least_one_port_included) {
            // The non-static user no longer wants to add membership on this switch.
            return vlan_mgmt_vlan_del(isid, membership->vid, user);
        }
    }

    if (user == VLAN_USER_STATIC) {
        // Gotta add on all switches according to the ports' configuration.
        isid = VTSS_ISID_GLOBAL;
    }

    if (isid == VTSS_ISID_GLOBAL) {
        // Allow VTSS_ISID_GLOBAL, but only on master
        if (!msg_switch_is_master()) {
            return VLAN_ERROR_MUST_BE_MASTER;
        }
    } else {
        VTSS_RC(VLAN_isid_port_check(isid, VTSS_PORT_NO_START, FALSE, FALSE));
    }

    if (user >= VLAN_USER_ALL) {
        return VLAN_ERROR_USER;
    }

    if (!vlan_mgmt_user_is_membership_changer(user)) {
        T_E("Unexpected user calling this function: %s. Will get allowed, but code should be updated", vlan_mgmt_user_to_txt(user));
        // Continue execution
    }

    vid = membership->vid;
    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        return VLAN_ERROR_VID;
    }

    T_D("isid %u, vid %u, user %s", isid, membership->vid, vlan_mgmt_user_to_txt(user));

    VLAN_CRIT_ENTER();
    VLAN_bulk_begin();

    if (user == VLAN_USER_STATIC) {
        // In order to be able to distinguish end-user-enabled VLANs
        // from auto-added VLANs (due to trunking), we must keep track
        // of the end-user-enabled.
        VTSS_BF_SET(VLAN_end_user_enabled_vlans, vid, TRUE);
    }

    // When adding a VLAN, it must be added to all switches, even if there are no
    // members on a particular switch.
    // So we iterate over VTSS_ISID_GLOBAL and *all* ISIDs, so that when a brand-new
    // switch enters the stack, it will automatically receive the required VLANs
    // and memberships.
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    while (switch_iter_getnext(&sit)) {
        vlan_ports_t ports;

        // If it's the static user that is adding VLANs, we need to auto-add
        // ports depending on their port mode.
        // The value of #membership->ports when this function got invoked is not
        // used for anything in that case, because we know the whole story already.
        if (user == VLAN_USER_STATIC) {
            port_iter_t pit;

            // Loop over *all* ports (not just those existing on the switch). The
            // reason for this is that we must also cater for switches that have
            // never been seen in the stack in which case we don't know how many ports it has
            // or even where the stack ports are located.
            (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                vlan_port_composite_conf_t *conf;
                conf = &VLAN_port_composite_conf[sit.isid - VTSS_ISID_START][pit.iport];

                if (conf->mode == VLAN_PORT_MODE_ACCESS) {
                    membership->ports[pit.iport] = vid == conf->access_vid;
                } else {
                    // Trunk or hybrid. Membership depends on allowed VIDs.
                    membership->ports[pit.iport] = VTSS_BF_GET(VLAN_allowed_vids[sit.isid - VTSS_ISID_START][pit.iport][conf->mode == VLAN_PORT_MODE_TRUNK ? 0 : 1], vid);
                }
            }
        }

        // Figure out if memberships are empty for this particular switch.
        // Notice that we need to convert the array for each and every switch,
        // because the resulting #ports may differ depending on number of
        // ports and location of stack ports on switch in question.
        if (isid == VTSS_ISID_GLOBAL || isid == sit.isid) {
            // Either all switches must have this configuration (isid == VTSS_ISID_GLOBAL)
            // or we're currently updating the ISID that the user has requested.
            VLAN_bool_to_bit(sit.isid, membership, &ports);

            // Now that we have updated the memberships for VLAN_USER_STATIC,
            // do the actual updating of the VLANs. This is handled in a
            // separate function, because it is also required by the functions
            // that can change port mode and allowed VIDs.
            if ((rc = VLAN_add_del_core(sit.isid, vid, user, &ports, VLAN_BIT_OPERATION_OVERWRITE)) != VTSS_RC_OK) {
                // Shouldn't be possible to return here.
                break;
            }
        }
    }

    VLAN_CRIT_EXIT();
    vlan_bulk_update_end();

    return rc;
}

/******************************************************************************/
// vlan_mgmt_vlan_del()
// #isid must be legal or VTSS_ISID_GLOBAL.
// #user must be in range [VLAN_USER_STATIC; VLAN_USER_ALL], i.e. VLAN_USER_ALL
// is also valid! If VLAN_USER_ALL, #isid must be VTSS_ISID_GLOBAL.
// The function returns VTSS_RC_OK even if deleting VLANs that don't exist beforehand.
//
// The VLAN will not be marked as deleted until the last switch has gotten the
// VLAN deleted. Listeners will only get called back when either
// 1) The last switch gets the VLAN deleted, or
// 2) at least one port was member on the switch in question.
/******************************************************************************/
vtss_rc vlan_mgmt_vlan_del(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user)
{
    switch_iter_t sit;
    vlan_user_t   user_iter, user_min, user_max;

    if (isid == VTSS_ISID_GLOBAL) {
        // Allow VTSS_ISID_GLOBAL, but only on master
        if (!msg_switch_is_master()) {
            // Possibly a transient phenomenon.
            T_D("Not master");
            return VLAN_ERROR_MUST_BE_MASTER;
        }
    } else {
        VTSS_RC(VLAN_isid_port_check(isid, VTSS_PORT_NO_START, FALSE, FALSE));
    }

    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        T_E("Invalid VID (%u)", vid);
        return VLAN_ERROR_VID;
    }

    // Allow VLAN_USER_ALL
    if (user > VLAN_USER_ALL) {
        T_E("Invalid user (%d)", user);
        return VLAN_ERROR_USER;
    }

    if (user != VLAN_USER_ALL && !vlan_mgmt_user_is_membership_changer(user)) {
        T_E("Unexpected user calling this function: %s. Will get allowed, but code should be updated", vlan_mgmt_user_to_txt(user));
        // Continue execution
    }

    if (user == VLAN_USER_ALL) {
        user_min = VLAN_USER_STATIC;
        user_max = (vlan_user_t)(VLAN_USER_ALL - 1);
        // Since the static user requires #isid to be VTSS_ISID_GLOBAL,
        // we also need it here, because all users include the static user.
        isid = VTSS_ISID_GLOBAL;
    } else {
        user_min = user_max = user;
    }

    if (user == VLAN_USER_STATIC) {
        // Always remove everything.
        isid = VTSS_ISID_GLOBAL;
    }

    T_D("isid %u, vid %d, user %s)", isid, vid, vlan_mgmt_user_to_txt(user));

    VLAN_CRIT_ENTER();
    VLAN_bulk_begin();

    if (user_min == VLAN_USER_STATIC) {
        // In order to be able to distinguish end-user-enabled VLANs
        // from auto-added VLANs (due to trunking), we must keep track
        // of the end-user-enabled.
        // We must set it prior to entering the loop below, because
        // VLAN_add_del_core() checks it.
        VTSS_BF_SET(VLAN_end_user_enabled_vlans, vid, FALSE);
    }

    // Make sure to get all in-use bits cleared in case we've been called with VTSS_ISID_GLOBAL.
    // This is ensured by iterating over *all* switches (if #isid == VTSS_ISID_GLOBAL).
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    while (switch_iter_getnext(&sit)) {

        // Iterate over all requested users.
        for (user_iter = user_min; user_iter <= user_max; user_iter++) {
            BOOL         at_least_one_change = TRUE, force_delete = TRUE;
            vlan_ports_t ports;

            if (user != VLAN_USER_FORBIDDEN && user_iter == VLAN_USER_FORBIDDEN) {
                // We must not remove forbidden membership unless we're invoked
                // with #user == VLAN_USER_FORBIDDEN.
                continue;
            }

            if (user_iter == VLAN_USER_STATIC) {
                // Static user is pretty special, because deleting a VLAN
                // doesn't necessarily mean that all ports belonging to that VLAN
                // should have their membership removed.
                // Hybrid and Trunk ports should retain their current membership,
                // whereas access ports whose PVID is the removed VLAN should
                // be taken out. Therefore, we need to traverse the current
                // port configuration and take proper action to only update
                // access ports.
                vtss_port_no_t port;

                at_least_one_change = FALSE;
                force_delete = FALSE;
                memset(&ports, 0, sizeof(ports));
                for (port = 0; port < VTSS_PORTS; port++) {
                    vlan_port_composite_conf_t *port_conf = &VLAN_port_composite_conf[sit.isid - VTSS_ISID_START][port];
                    if (port_conf->mode == VLAN_PORT_MODE_ACCESS && vid == port_conf->access_vid) {
                        // Gotta remove this ports from the VLAN.
                        VTSS_BF_SET(ports.ports, port, TRUE);
                        at_least_one_change = TRUE;
                    }
                }
            } else {
                ports.ports[0] = 0; // Satisfy Lint
            }

            if (at_least_one_change) {
                vtss_rc rc;
                if ((rc = VLAN_add_del_core(sit.isid, vid, user_iter, force_delete ? NULL : &ports, VLAN_BIT_OPERATION_DEL)) != VTSS_RC_OK) {
                    T_E("What happened? %s", error_txt(rc));
                }
            }
        }
    }

    VLAN_CRIT_EXIT();

    // This will cause subscribers and switch(es) to get updated.
    vlan_bulk_update_end();

    // Takes some serious effort to make this function fail.
    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_membership_per_port_get()
/******************************************************************************/
vtss_rc vlan_mgmt_membership_per_port_get(vtss_isid_t isid, vtss_port_no_t port, vlan_user_t user, u8 vid_mask[VLAN_BITMASK_LEN_BYTES])
{
    vtss_isid_t zisid = isid - VTSS_ISID_START;
    vtss_vid_t  vid;
    u32         multi_user_idx  = VLAN_user_to_multi_idx(user);
#if defined(VLAN_SAME_USER_SUPPORT)
    u32         same_user_idx   = VLAN_user_to_same_idx(user);
#endif /* defined(VLAN_SAME_USER_SUPPORT) */
#if defined(VLAN_SINGLE_USER_SUPPORT)
    u32         single_user_idx = VLAN_user_to_single_idx(user);
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

    VTSS_RC(VLAN_isid_port_check(isid, port, FALSE, TRUE));

    if (user > VLAN_USER_ALL) {
        return VLAN_ERROR_USER;
    }

    memset(vid_mask, 0, VLAN_BITMASK_LEN_BYTES);

    // Same and Single VLAN users can only have one bit set in the resulting array.
#if defined(VLAN_SAME_USER_SUPPORT)
    if (same_user_idx != VLAN_SAME_CNT) {
        VLAN_CRIT_ENTER();

        // "Same" users can only configure one single VLAN.
        vid = VLAN_same_table[same_user_idx].vid;

        // Always check in-use array, because the contents of VLAN_same_table[] may
        // not have been updated when a user unregistered himself.
        if (VLAN_IN_USE_ON_SWITCH_GET(isid, vid, user)) {
            VTSS_BF_SET(vid_mask, vid, TRUE);
        }

        VLAN_CRIT_EXIT();

        return VTSS_RC_OK;
    }
#endif /* defined(VLAN_SAME_USER_SUPPORT) */

#if defined(VLAN_SINGLE_USER_SUPPORT)
    if (single_user_idx != VLAN_SINGLE_CNT) {
        VLAN_CRIT_ENTER();

        // A "single" user is a user that can configure different ports to be members of different VLANs,
        // but at most one VLAN per port.
        vid = VLAN_single_table[single_user_idx].vid[zisid][port];

        // Always check in-use array, because the contents of VLAN_single_table[] may
        // not have been updated when a user unregistered himself.
        if (VLAN_IN_USE_ON_SWITCH_GET(isid, vid, user)) {
            VTSS_BF_SET(vid_mask, vid, TRUE);
        }

        VLAN_CRIT_EXIT();

        return VTSS_RC_OK;
    }
#endif /* defined(VLAN_SINGLE_USER_SUPPORT) */

    // If we get here, #user is either forbidden or multi-vlan-user
    VLAN_CRIT_ENTER();

    for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
        if (!VLAN_IN_USE_ON_SWITCH_GET(isid, vid, user)) {
            continue;
        }

        if (user == VLAN_USER_FORBIDDEN) {
            VTSS_BF_SET(vid_mask, vid, VTSS_BF_GET(VLAN_forbidden_table[vid].ports[zisid], port));
        } else if (multi_user_idx != VLAN_MULTI_CNT) {
            VTSS_BF_SET(vid_mask, vid, VTSS_BF_GET(VLAN_multi_table[vid]->user_entries[multi_user_idx].ports[zisid], port));
        } else {
            // VLAN_USER_ALL
            VTSS_BF_SET(vid_mask, vid, VTSS_BF_GET(VLAN_combined_table[vid].ports[zisid], port));
        }
    }

    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_registration_per_port_get()
/******************************************************************************/
vtss_rc vlan_mgmt_registration_per_port_get(vtss_isid_t isid, vtss_port_no_t port, vlan_registration_type_t reg[VLAN_ID_MAX + 1])
{
    vtss_rc    rc;
    u8         static_vid_mask[VLAN_BITMASK_LEN_BYTES];
    u8         forbidden_vid_mask[VLAN_BITMASK_LEN_BYTES];
    vtss_vid_t vid;

    // Let vlan_mgmt_membership_per_port_get() verify the arguments to this function.

    if ((rc = vlan_mgmt_membership_per_port_get(isid, port, VLAN_USER_STATIC, static_vid_mask)) != VTSS_RC_OK) {
        return rc;
    }

    if ((rc = vlan_mgmt_membership_per_port_get(isid, port, VLAN_USER_FORBIDDEN, forbidden_vid_mask)) != VTSS_RC_OK) {
        return rc;
    }

    for (vid = VLAN_ID_MIN; vid <= VLAN_ID_MAX; vid++) {
        if (VTSS_BF_GET(forbidden_vid_mask, vid)) {
            reg[vid] = VLAN_REGISTRATION_TYPE_FORBIDDEN;
        } else {
            reg[vid] = VTSS_BF_GET(static_vid_mask, vid) ? VLAN_REGISTRATION_TYPE_FIXED : VLAN_REGISTRATION_TYPE_NORMAL;
        }
    }

    return VTSS_RC_OK;
}

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// VLAN_reserved_name_to_vid()
// Returns the VLAN ID corresponding to #name if #name is one of the reserved
// names (VLAN_NAME_DEFAULT or matches "VLANxxxx" template).
// Otherwise returns VTSS_VID_NULL.
/******************************************************************************/
static vtss_vid_t VLAN_reserved_name_to_vid(const char name[VLAN_NAME_MAX_LEN])
{
    if (name[0] == '\0') {
        return VTSS_VID_NULL;
    }

    // Check to see if #name is either VLAN_NAME_DEFAULT or on the form "VLANxxxx".
    // If so, return the VLAN ID corresponding to these.
    // Otherwise return VTSS_VID_NULL.
    if (strcmp(name, VLAN_NAME_DEFAULT) == 0) {
        return VLAN_ID_DEFAULT;
    }

    // Check against "VLANxxxx" template.
    if (strlen(name) == 8 && strncmp(name, "VLAN", 4) == 0) {
        vtss_vid_t v;
        int        i;

        v = 0;
        for (i = 0; i < 4; i++) {
            char c = name[4 + i];

            if (c < '0' || c > '9') {
                // Doesn't match template
                return VTSS_VID_NULL;
            }

            v *= 10;
            v += c - '0';;
        }

        // The name might be reserved, because it adheres to the "VLANxxxx" template.
        if (v >= VLAN_ID_MIN && v <= VLAN_ID_MAX) {
            // It's a valid VLAN ID
            return v;
        }
    }

    return VTSS_VID_NULL;
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// VLAN_name_lookup()
/******************************************************************************/
static vtss_vid_t VLAN_name_lookup(const char name[VLAN_NAME_MAX_LEN])
{
    vtss_vid_t v;

    VLAN_CRIT_ASSERT_LOCKED();

    if (name[0] != '\0') {
        for (v = VLAN_ID_MIN; v <= VLAN_ID_MAX; v++) {
            if (strcmp(VLAN_name_conf[v], name) == 0) {
                return v;
            }
        }
    }

    return VTSS_VID_NULL;
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */


#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// vlan_mgmt_name_get()
/******************************************************************************/
vtss_rc vlan_mgmt_name_get(vtss_vid_t vid, char name[VLAN_NAME_MAX_LEN], BOOL *is_default)
{
    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        return VLAN_ERROR_VID;
    }

    if (name == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();

    if (is_default) {
        // #is_default is optional and may be NULL.
        *is_default = VLAN_name_conf[vid][0] == '\0';
    }

    if (VLAN_name_conf[vid][0] == '\0') {
        if (vid == VLAN_ID_DEFAULT) {
            strcpy(name, VLAN_NAME_DEFAULT);
        } else {
            sprintf(name, "VLAN%04d", vid);
        }
    } else {
        strcpy(name, VLAN_name_conf[vid]);
    }

    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// vlan_mgmt_name_set()
/******************************************************************************/
vtss_rc vlan_mgmt_name_set(vtss_vid_t vid, const char name[VLAN_NAME_MAX_LEN])
{
    vtss_rc    rc        = VTSS_RC_OK;
    BOOL       set_empty = FALSE;
    vtss_vid_t v;
    u32        i, len;

    if (vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        return VLAN_ERROR_VID;
    }

    if (name == NULL) {
        return VLAN_ERROR_PARM;
    }

    len = strlen(name);
    if (len >= VLAN_NAME_MAX_LEN) {
        return VLAN_ERROR_PARM;
    }

    for (i = 0; i < len; i++) {
        if (name[i] < 33 || name[i] > 126) {
            return VLAN_ERROR_NAME_INVALID;
        }
    }

    // Try to convert #name to a VID using default name rules.
    if ((v = VLAN_reserved_name_to_vid(name)) != VTSS_VID_NULL) {
        // #name matches a reserved name.
        if (v != vid) {
            // The VLAN name is reserved for another VID.
            return VLAN_ERROR_NAME_RESERVED;
        } else {
            // The name that is attempted set, corresponds to the default name of this VLAN.
            // Clear it.
            set_empty = TRUE;
        }
    }

    if (name[0] == '\0') {
        set_empty = TRUE;
    }

    if (!set_empty && vid == VLAN_ID_DEFAULT) {
        // It's illegal to change the name of the default VLAN.
        // The default VLAN supports two "set"-names, namely "default" and "VLAN0001",
        // but it only supports one "get"-name, namely "default".
        return VLAN_ERROR_NAME_DEFAULT_VLAN;
    }

    // If we're still here, we need to either set the entry empty or look through the
    // VLAN_name_conf[] table to see if another VLAN has this name already.

    VLAN_CRIT_ENTER();

    if (set_empty) {
        // Back to default name
        VLAN_name_conf[vid][0] = '\0';
    } else {
        if ((v = VLAN_name_lookup(name)) == VTSS_VID_NULL) {
            // Not already found.
            strcpy(VLAN_name_conf[vid], name);
        } else if (v != vid) {
            // Already exists under another VLAN ID.
            rc = VLAN_ERROR_NAME_ALREADY_EXISTS;
        }
    }

#if !defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    VLAN_name_conf_save();
#endif /* !defined(VTSS_SW_OPTION_SILENT_UPGRADE) */

    VLAN_CRIT_EXIT();

    return rc;
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */


#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/******************************************************************************/
// vlan_mgmt_name_to_vid()
/******************************************************************************/
vtss_rc vlan_mgmt_name_to_vid(const char name[VLAN_NAME_MAX_LEN], vtss_vid_t *vid)
{
    if (name == NULL || vid == NULL) {
        return VLAN_ERROR_PARM;
    }

    // Try to convert #name to a VID using reserved name rules.
    if ((*vid = VLAN_reserved_name_to_vid(name)) != VTSS_VID_NULL) {
        // It matched the reserved name for a VLAN.
        return VTSS_RC_OK;
    }

    // No match so far. Gotta look it up in the array of VLAN names.

    VLAN_CRIT_ENTER();
    *vid = VLAN_name_lookup(name);
    VLAN_CRIT_EXIT();

    return *vid == VTSS_VID_NULL ? (vtss_rc)VLAN_ERROR_NAME_DOES_NOT_EXIST : VTSS_RC_OK;
}
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */


/******************************************************************************/
// vlan_mgmt_conflicts_get()
// Get the current conflicts from VLAN port configuration database.
/******************************************************************************/
vtss_rc vlan_mgmt_conflicts_get(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conflicts_t *conflicts)
{
    vlan_port_conf_t dummy;

    VTSS_RC(VLAN_isid_port_check(isid, port, FALSE, TRUE));

    if (conflicts == NULL) {
        return VLAN_ERROR_PARM;
    }

    VLAN_CRIT_ENTER();
    (void)VLAN_port_conflict_resolver(isid, port, conflicts, &dummy);
    VLAN_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// vlan_mgmt_vid_gets_tagged()
/******************************************************************************/
BOOL vlan_mgmt_vid_gets_tagged(vlan_port_conf_t *p, vtss_vid_t vid)
{
    if (p == NULL || vid < VLAN_ID_MIN || vid > VLAN_ID_MAX) {
        T_E("Invalid params");
        return FALSE;
    }

    if (p->tx_tag_type  == VLAN_TX_TAG_TYPE_UNTAG_ALL                             ||
        (p->tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS && p->untagged_vid == vid) ||
        (p->tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS   && p->pvid         == vid && p->untagged_vid != p->pvid)) {
        // Port is configured to either
        // 1) Untag all frames,
        // 2) Untag a particular VID, which happens to be the VID we're currently looking at.
        // 3) Tag a particular VID. Two cases to consider:
        //      If the VID that is requested tagged is the PVID, all frames are tagged.
        //      If the VID that is requested tagged is NOT the PVID, all frames but PVID are tagged.
        //      Therefore, if the VID we're looking at now is the PVID and the VID to tag is not the PVID,
        //      the VID we're looking at right now (the PVID) gets untagged.
        return FALSE;
    }

    return TRUE;
}

/******************************************************************************/
// vlan_init()
// Initialize module.
/******************************************************************************/
vtss_rc vlan_init(vtss_init_data_t *data)
{
    vtss_isid_t                isid = data->isid;
    vtss_rc                    rc   = VTSS_RC_OK;

    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Initialize message buffer pools.
        // Essentially, we could live with one pool of two VLANs due to the two requests made in SWITCH_ADD event.
        // However, the VLAN_MSG_ID_MEMBERSHIP_SET_REQ is up to 500 KBytes big (depending on VLAN_ENTRY_CNT), so allocating two
        // of those would be overkill. Therefore, we split the pools into two: One with normal requests and one with
        // the large VLAN configuration set request.
        // For the sake of conf_xml, instead of setting the normal request pool to 1, we increase it to 10, which
        // allows for having more requests in the pipeline. At the end of the day, it's the message module's window
        // size that determines how many outstanding requests the master cna have towards a given slave.
        VLAN_request_pool       = msg_buf_pool_create(VTSS_MODULE_ID_VLAN, "Request",       10, sizeof(vlan_msg_req_t));
        VLAN_large_request_pool = msg_buf_pool_create(VTSS_MODULE_ID_VLAN, "Large Request", 1,  sizeof(vlan_msg_large_req_t));

        // Create semaphore for critical regions
        critd_init(&VLAN_crit,    "VLAN crit",    VTSS_MODULE_ID_VLAN, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        critd_init(&VLAN_cb_crit, "VLAN cb_crit", VTSS_MODULE_ID_VLAN, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        VLAN_CRIT_EXIT();
        VLAN_CB_CRIT_EXIT();

#if defined(VTSS_SW_OPTION_VCLI)
        (void)vlan_cli_req_init();
#endif /* defined(VTSS_SW_OPTION_VCLI) */

#if defined(VTSS_SW_OPTION_ICFG)
        if ((rc = VLAN_icfg_init()) != VTSS_RC_OK) {
            T_D("Calling vlan_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif /* defined(VTSS_SW_OPTION_ICFG) */
        break;

    case INIT_CMD_START:

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
        VLAN_name_default();
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

        // Stitch together the VLAN table.
        VLAN_table_init();

        // Initialize VLAN port configuration. This one takes the VLAN crit itself.
        VLAN_port_default(VTSS_ISID_GLOBAL, NULL);

        // Register for stack messages
        rc = VLAN_msg_rx_register();
        break;

    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (VTSS_ISID_LEGAL(isid)) {
            VLAN_port_conf_read(isid);
        } else if (isid == VTSS_ISID_GLOBAL) {
            // Reset stack configuration
            VLAN_conf_read(TRUE);
#if defined(VTSS_SW_OPTION_VLAN_NAMING)
            VLAN_name_conf_read(TRUE);
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        VLAN_port_conf_read(VTSS_ISID_GLOBAL);

        // Read stack and switch configuration
        VLAN_conf_read(FALSE);
#if defined(VTSS_SW_OPTION_VLAN_NAMING)
        VLAN_name_conf_read(FALSE);
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */
        break;

    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);

        // Apply all configuration to switch.
        VLAN_msg_tx_port_conf_all(isid);
        VLAN_msg_tx_membership_all(isid);
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        {
            vtss_etype_t tpid;

            VLAN_CRIT_ENTER();
            tpid = VLAN_tpid_s_custom_port;
            VLAN_CRIT_EXIT();

            VLAN_msg_tx_tpid_conf(tpid);
        }
#endif
        break;

    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;

    default:
        break;
    }

    T_D("exit");
    return rc;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

