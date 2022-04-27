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

// These are used unprotected in calls to cyg_flag_maskbits() and cyg_flag_timed_wait(), only
/*lint -esym(459, lldp_msg_id_get_entries_req_flags)      */
/*lint -esym(459, lldp_msg_id_get_stat_req_flags)         */
/*lint -esym(459, lldp_if_crit)                           */

#include "vtss_common_os.h"
#include "critd_api.h"
#include "main.h"
#include "msg_api.h"
#include "vtss_lldp.h"
#include "vtss_api.h"
#include "vtss_ecos_mutex_api.h"
#include "conf_api.h"
#include "led_api.h"
#include "lldp.h"
#include "control_api.h"
#include "misc_api.h"
#include "sysutil_api.h"

#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif

#ifdef VTSS_SW_OPTION_IP2
#include "ip2_api.h"
#endif

#ifdef VTSS_SW_OPTION_CDP
#include "vlan_api.h"
#include "cdp_analyse.h"
#endif

#ifdef VTSS_SW_OPTION_SNMP
#include "dot1ab_lldp_api.h"
#endif /* VTSS_SW_OPTION_SNMP */

#ifdef VTSS_SW_OPTION_VCLI
#include "lldp_cli.h"
#endif

#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_rx.h"
#include "lldpmed_tx.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "lldpmed_cli.h"
#endif
#ifdef VTSS_SW_OPTION_SECURE_SNMP
#include "lldpXMedMIB_api.h"
#endif
#endif

#ifdef VTSS_SW_OPTION_VOICE_VLAN
#include "voice_vlan_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "lldp_icli_functions.h" // For lldp_icfg_init
#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_icli_functions.h" // For lldpmed_icfg_init
#endif

#endif //VTSS_SW_OPTION_ICFG

/* Critical region protection protecting the following block of variables */
static critd_t lldp_crit;
static critd_t lldp_if_crit;

//****************************************
// TRACE
//****************************************
#if VTSS_TRACE_ENABLED

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "lldp",
    .descr     = "Link Layer Discovery Protocol"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_STAT] = {
        .name      = "stat",
        .descr     = "Statistics",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_SNMP] = {
        .name      = "snmp",
        .descr     = "LLDP SNMP",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_TX] = {
        .name      = "tx",
        .descr     = "LLDP Transmit",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_MED_TX] = {
        .name      = "med_tx",
        .descr     = "LLDP-MED Transmit",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_MED_RX] = {
        .name      = "med_rx",
        .descr     = "LLDP-MED Rx",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_RX] = {
        .name      = "rx",
        .descr     = "LLDP RX",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CONF] = {
        .name      = "conf",
        .descr     = "LLDP configuration",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_POE] = {
        .name      = "poe",
        .descr     = "LLDP PoE",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CDP] = {
        .name      = "cdp",
        .descr     = "Cisco Discovery Protocol",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
#ifdef VTSS_SW_OPTION_EEE
    [TRACE_GRP_EEE] = {
        .name      = "EEE",
        .descr     = "Energy Efficient Ethernet",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
#endif
    [TRACE_GRP_CLI] = {
        .name      = "cli",
        .descr     = "Command line interface",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },

    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};

#define LLDP_CRIT_ENTER()            critd_enter(        &lldp_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_CRIT_EXIT()             critd_exit(         &lldp_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_CRIT_ASSERT_LOCKED()    critd_assert_locked(&lldp_crit,    TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define LLDP_IF_CRIT_ENTER()         critd_enter(        &lldp_if_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_IF_CRIT_EXIT()          critd_exit(         &lldp_if_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_IF_CRIT_ASSERT_LOCKED() critd_assert_locked(&lldp_if_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
#define LLDP_CRIT_ENTER()            critd_enter(        &lldp_crit)
#define LLDP_CRIT_EXIT()             critd_exit (        &lldp_crit)
#define LLDP_CRIT_ASSERT_LOCKED()    critd_assert_locked(&lldp_crit)
#define LLDP_IF_CRIT_ENTER()         critd_enter(        &lldp_if_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_IF_CRIT_EXIT()          critd_exit(         &lldp_if_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LLDP_IF_CRIT_ASSERT_LOCKED() critd_assert_locked(&lldp_if_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#endif /* VTSS_TRACE_ENABLED */

// Define time out for slave to reply upon requests to 5 sec
#define LLDP_REQ_TIMEOUT 10

/************************/
/* messages             */
/************************/
typedef enum {
    LLDP_MSG_ID_CONF_SET_REQ,    // Configuration set request (no reply)
    LLDP_MSG_ID_GET_STAT_REQ,    // Get statistics, global counters, and last time entry was changed from a slave.
    LLDP_MSG_ID_GET_STAT_REP,    // Slave's reply to LLDP_MSG_ID_GET_STAT_REQ.
    LLDP_MSG_ID_STAT_CLR,        // Signal to clear the statistics counters
    LLDP_MSG_ID_GLOBAL_STAT_CLR, // Signal to clear the statistics global counters
    LLDP_MSG_ID_GET_ENTRIES_REQ, // Master's request to get LLDP entries from a slave
    LLDP_MSG_ID_GET_ENTRIES_REP, // Slave's reply upon a get entries
#ifdef VTSS_SW_OPTION_SNMP
    LLDP_MSG_ID_SEND_TRAP, // Slave's reply upon a get entries
#endif /*   REMOVE_STATS_IN_TRAP_BINDING_VAR    */
} lldp_msg_id_t;

// Request message
typedef struct {
    lldp_msg_id_t msg_id;

    union {
        // LLDP_MSG_ID_CONF_SET_REQ
        struct {
            lldp_struc_0_t  conf;                          // Configuration for one switch
            char            somethingChangedLocal;         // Set to 1 when system name/mac address or ip address have changed.
            char            sys_name[VTSS_SYS_STRING_LEN]; // Since system name is only present at the master, we must push the name to the slaves.
        } set_conf;

        // LLDP_MSG_ID_GET_STAT_REQ
        // No data

        // LLDP_MSG_ID_STAT_CLR
        // No data

        // LLDP_MSG_ID_GLOBAL_STAT_CLR
        // No data

        // LLDP_MSG_ID_GET_ENTRIES_REQ
        // No data

#ifdef VTSS_SW_OPTION_SNMP
        // LLDP_MSG_ID_SEND_TRAP
        struct {
            lldp_port_t         iport;
            int                 notification_interval;
        } send_trap;
#endif /*   REMOVE_STATS_IN_TRAP_BINDING_VAR    */
    } req;
} lldp_msg_req_t;

// Reply message
typedef struct {
    lldp_msg_id_t msg_id;

    union {
        // LLDP_MSG_ID_GET_ENTRIES_REP
        struct {
            u32 count;
            lldp_remote_entry_t entries[LLDP_REMOTE_ENTRIES];
        } remote_entries;

        // LLDP_MSG_ID_GET_STAT_REP
        struct {
            lldp_counters_rec_t stat_cnt[LLDP_PORTS];
            lldp_mib_stats_t    global_cnts;
            time_t              last_change_time_ago;
        } stat;
    } rep;
} lldp_msg_rep_t;

/* Master's configuration */
typedef struct {
    lldp_master_flash_conf_t   flash;   // Configuration that is stored in flash
    lldp_conf_run_time_t       run_time;  // Settings that are found at runtime
} lldp_master_conf_t;

//***********************************************
// MISC
//***********************************************
/* Thread variables */
static cyg_handle_t lldp_thread_handle;
static cyg_thread   lldp_thread_block;
static char         lldp_thread_stack[THREAD_DEFAULT_STACK_SIZE * 2];

static lldp_master_conf_t  lldp_global;                        // Configuration for all switch (configurable from Master)
static lldp_struc_0_t      local_conf;                         // Current configuration for this switch
static lldp_counters_rec_t local_stat_cnt[LLDP_PORTS];         // Stat counter for this switch
static lldp_mib_stats_t    global_cnt;
static lldp_remote_entry_t local_entries[LLDP_REMOTE_ENTRIES]; // Neighbor entries for this switch

static cyg_flag_t lldp_msg_id_get_stat_req_flags;              // Flag for synch. of request and reply of stat counters.
static cyg_flag_t lldp_msg_id_get_entries_req_flags;           // Flag for synch. of request and reply of entries.

static void *lldp_msg_request_pool;
static void *lldp_msg_reply_pool;

//************************************************
// Variables
//************************************************
static lldp_bool_t system_conf_has_changed = 1; // Set by callback function if systemname or ip address changes.
static time_t last_entry_change_time_ago_slave_switch = 0;
static time_t last_entry_change_time_this_switch  = 0;
static lldp_bool_t switch_add = FALSE;

// Function that copies the global configuration to the format the base code is using.
// In/Out: conf - Pointer to where to copy the current global configuration to.
//         isid - The switch in question
static void lldp_global_conf2local_conf(lldp_struc_0_t *conf, vtss_isid_t isid)
{
    conf->reInitDelay = lldp_global.flash.common.reInitDelay;
    conf->msgTxHold   = lldp_global.flash.common.msgTxHold;
    conf->msgTxInterval = lldp_global.flash.common.msgTxInterval;
    conf->txDelay = lldp_global.flash.common.txDelay;
    conf->timingChanged = lldp_global.run_time.timingChanged;
    memcpy(conf->admin_state, lldp_global.flash.port[isid].admin_states, sizeof(admin_state_t));
    memcpy(conf->optional_tlv, lldp_global.flash.port[isid].optional_tlvs, sizeof(optional_tlv_t));

#ifdef VTSS_SW_OPTION_CDP
    memcpy(conf->cdp_aware, lldp_global.flash.port[isid].cdp_aware, sizeof(cdp_aware_t));
#endif
    conf->mac_addr = lldp_global.run_time.mac_addr;

#ifdef VTSS_SW_OPTION_LLDP_MED
    memcpy(conf->policies_table, lldp_global.flash.common.policies_table, sizeof(lldpmed_policies_table_t));
    memcpy(conf->ports_policies, lldp_global.flash.common.ports_policies, sizeof(lldpmed_ports_policies_t));
    memcpy(&conf->location_info, &lldp_global.flash.common.location_info, sizeof(lldpmed_location_info_t));
    conf->medFastStartRepeatCount = lldp_global.flash.common.medFastStartRepeatCount;
    memcpy(conf->lldpmed_optional_tlv, lldp_global.flash.common.lldpmed_optional_tlv, sizeof(lldpmed_optional_tlv_t));
#endif
}


// Function that copies configuration in base code to a global common configuration format (common configuration for all switches in a stack).
// In : conf        - Pointer to the configuration which is going to converted to the current common configuration structure.
// Out: common_conf - Pointer to the common going to contain the common configuration
static void lldp_local_conf2common_conf(lldp_struc_0_t *conf, lldp_common_conf_t *common_conf)
{
    common_conf->reInitDelay = conf->reInitDelay;
    common_conf->msgTxHold = conf->msgTxHold;
    common_conf->msgTxInterval = conf->msgTxInterval;
    common_conf->txDelay = conf->txDelay ;

#ifdef VTSS_SW_OPTION_LLDP_MED
    memcpy(common_conf->policies_table, conf->policies_table, sizeof(lldpmed_policies_table_t));
    memcpy(common_conf->ports_policies, conf->ports_policies, sizeof(lldpmed_ports_policies_t));
    memcpy(&common_conf->location_info, &conf->location_info, sizeof(lldpmed_location_info_t));
    memcpy(common_conf->lldpmed_optional_tlv, conf->lldpmed_optional_tlv, sizeof(lldpmed_optional_tlv_t));
#endif

}

// Function that copies configuration in base code to the global configuration format.
// In/Out: conf - Pointer to the new configuration which is going to copied to the current global configuration.
//         isid - The switch in question
static void lldp_local_conf2global_conf(lldp_struc_0_t *conf, vtss_isid_t isid)
{
    lldp_global.run_time.timingChanged = conf->timingChanged;
    memcpy(lldp_global.flash.port[isid].admin_states, conf->admin_state, sizeof(admin_state_t));
    memcpy(lldp_global.flash.port[isid].optional_tlvs, conf->optional_tlv, sizeof(optional_tlv_t));

#ifdef VTSS_SW_OPTION_CDP
    memcpy(lldp_global.flash.port[isid].cdp_aware, conf->cdp_aware, sizeof(cdp_aware_t));
#endif
    lldp_global.run_time.mac_addr = conf->mac_addr;

#ifdef VTSS_SW_OPTION_LLDP_MED
    lldp_global.flash.common.medFastStartRepeatCount = conf->medFastStartRepeatCount;
#endif

    lldp_local_conf2common_conf(conf, &lldp_global.flash.common);
}

//
// Converts error to printable text
//
// In : rc - The error type
//
// Retrun : Error text
//
char *lldp_error_txt(vtss_rc rc)
{
    switch (rc) {
    case LLDP_ERROR_ISID:
        return "Invalid Switch ID";

    case LLDP_ERROR_FLASH:
        return "Could not store configuration in flash";

    case LLDP_ERROR_SLAVE:
        return "Could not get data from slave switch";

    case LLDP_ERROR_NOT_MASTER:
        return "Switch is not master";

    case LLDP_ERROR_ISID_LOCAL:
        return "Switch id must not be VTSS_ISID_LOCAL";

    case LLDP_ERROR_VOICE_VLAN_CONFIGURATION_MISMATCH:
        return "Cannot set LLDP port mode to disabled or TX only when Voice-VLAN supports LLDP discovery protocol.";

    case LLDP_ERROR_NOTIFICATION_VALUE:
        return "Notification enable must be either 1 (enable) or 2 (disable)";

    case LLDP_ERROR_NOTIFICATION_INTERVAL_VALUE:
        return "Notification enable must be between 5 and 3600";

    case LLDP_ERROR_TX_DELAY:
        return "txDelay must not be larger than 0.25 * TxInterval - IEEE 802.1AB-clause 10.5.4.2 - Configuration ignored";

    case LLDP_ERROR_NULL_POINTER:
        return "Unexpected reference to NULL pointer";

    case VTSS_RC_OK:
        return "";
    }

    T_I("rc:%d", rc);
    return "Unknown LLDP error";
}

#ifdef VTSS_SW_OPTION_CDP
// Because the CDP frame is a special multicast frame that the chip doesn't know
// how to handle, we need to add the MAC address to the MAC table manually.
// The hash entry to the MAC table consists of both the DMAC and VLAN VID.
// Since the port VLAN ID can be changed by the user this function MUST be called
// every time the port VLAN VID changes.
static void cdp_add_to_mac_table(vtss_isid_t isid /* unused */, vtss_port_no_t port_no, const vlan_port_conf_t *vlan_conf)
{
    static vtss_mac_table_entry_t entry[LLDP_PORTS];
    const vtss_mac_t              vtss_cdp_slowmac = {{0x01, 0x00, 0x0C, 0xCC, 0xCC, 0xCC}};
    vlan_port_conf_t              conf;
    port_iter_t                   pit;
    vtss_port_no_t                i;

    vtss_port_no_t port_index = port_no - VTSS_PORT_NO_START;

    if (port_index < LLDP_PORTS) {
        LLDP_CRIT_ENTER();
        vtss_vid_t  pvid;   // Port VLAN ID for the port that has changed configuration
        lldp_bool_t remove_entry_from_mac_table = TRUE;

        // Get the PVID for the port that has changed configuration
        (void)vlan_mgmt_port_conf_get(VTSS_ISID_LOCAL, port_no, &conf, VLAN_USER_ALL);
        pvid = conf.pvid;

        // We need to make sure that no other ports uses the same PVID before
        // we can remove the entry from the mac table.
        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            (void)vlan_mgmt_port_conf_get(VTSS_ISID_LOCAL, pit.iport, &conf, VLAN_USER_ALL);
            if (conf.pvid == entry[port_index].vid_mac.vid && local_conf.cdp_aware[pit.iport - VTSS_PORT_NO_START]) {
                T_RG(TRACE_GRP_CDP, "Another port uses the same PVID (%u) - Leaving entry in mac table", entry[port_index].vid_mac.vid);
                remove_entry_from_mac_table = FALSE; // There is another port that uses the same PVID, so we can not remove the entry.
            }
        }

        // Do the remove of the entry.
        if (remove_entry_from_mac_table) {
            T_NG(TRACE_GRP_CDP, "removing mac entry");
            (void)vtss_mac_table_del(NULL, &entry[port_index].vid_mac);
        }

        if (local_conf.cdp_aware[port_index]) {
            // Add the new entry to the mac table
            T_DG_PORT(TRACE_GRP_CDP, port_index, "Adding CDP to MAC table, pvid = %u", pvid);
            memset(entry, 0, sizeof(vtss_mac_table_entry_t));
            entry[port_index].vid_mac.vid = pvid;
            entry[port_index].vid_mac.mac = vtss_cdp_slowmac;
            entry[port_index].copy_to_cpu = 1;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
            entry[port_index].cpu_queue = PACKET_XTR_QU_MAC;
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
            entry[port_index].locked = 1;
            entry[port_index].aged = 0;
            for (i = 1; i <= (VTSS_PORT_ARRAY_SIZE - 1); i++) {
                entry[port_index].destination[i] = 0;
            }
            (void)vtss_mac_table_add(NULL, &entry[port_index]);
        }
        LLDP_CRIT_EXIT();
    }
}
#endif /* VTSS_SW_OPTION_CDP */

// Function for clearing the local counters
static void lldp_local_cnt_clear(void)
{
    // Clear local counters
    LLDP_CRIT_ENTER();
    int port_idx; // Port index starting from 0.
    for (port_idx = 0; port_idx < VTSS_PORT_NO_END - VTSS_PORT_NO_START; port_idx++) {
        if (port_idx < LLDP_PORTS) { // Make sure that we don't go out of bounds
            lldp_statistics_clear(port_idx);
        }
    }
    LLDP_CRIT_EXIT();
}

static void lldp_global_cnt_clear(void)
{
    // Clear global counters
    LLDP_CRIT_ENTER(); // Protect last_entry_change_time_this_switch
    last_entry_change_time_this_switch = msg_uptime_get(VTSS_ISID_LOCAL); // Set the last changed time to current time (equals last time changed is 0 sec.)
    lldp_mib_stats_clear();
    LLDP_CRIT_EXIT();
}

/* Allocate request buffer */
static lldp_msg_req_t *lldp_msg_req_alloc(lldp_msg_id_t msg_id, u32 ref_cnt)
{
    lldp_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(lldp_msg_request_pool);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }
    msg->msg_id = msg_id;
    return msg;
}

/* Allocate reply buffer */
static lldp_msg_rep_t *lldp_msg_rep_alloc(lldp_msg_id_t msg_id)
{
    lldp_msg_rep_t *msg = msg_buf_pool_get(lldp_msg_reply_pool);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Release message buffer */
static void lldp_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    (void)msg_buf_pool_put(msg);
}

/* Send message */
static void lldp_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of MSG_TX_DATA_HDR_LEN_MAX() below
    /*lint -e(506) */
    msg_tx_adv(NULL, lldp_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_LLDP, isid, msg, len + MSG_TX_DATA_HDR_LEN_MAX(lldp_msg_req_t, req, lldp_msg_rep_t, rep));
}

// Getting message from the message module.
static lldp_bool_t lldp_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    lldp_msg_id_t msg_id = *(lldp_msg_id_t *)rx_msg;

    T_NG(TRACE_GRP_CONF, "Entering lldp_msg_rx msg_id: %d,  len: %zd, isid: %u", msg_id, len, isid);

    switch (msg_id) {
    case LLDP_MSG_ID_CONF_SET_REQ: {
        lldp_msg_req_t *msg = (lldp_msg_req_t *)rx_msg;
#ifdef VTSS_SW_OPTION_CDP
        port_iter_t pit;
#endif

        T_DG(TRACE_GRP_CONF, "msg_id = LLDP_MSG_ID_CONF_SET_REQ");

        LLDP_CRIT_ENTER();
        lldp_admin_state_changed(local_conf.admin_state, msg->req.set_conf.conf.admin_state);
        local_conf = msg->req.set_conf.conf; // Update configuration for this switch.

        if (local_conf.timingChanged) {
            lldp_global.run_time.timingChanged = 0;
            lldp_set_timing_changed();
        }
        if (msg->req.set_conf.somethingChangedLocal) {
            T_IG(TRACE_GRP_CONF, "New message - system name = %s", msg->req.set_conf.sys_name);
            lldp_system_name(msg->req.set_conf.sys_name, 0); // Set local copy of master's system name.
            lldp_something_changed_local();
        }
        LLDP_CRIT_EXIT();

#ifdef VTSS_SW_OPTION_CDP
        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            cdp_add_to_mac_table(VTSS_ISID_LOCAL, pit.iport, NULL);
        }
#endif // VTSS_SW_OPTION_CDP
        break;
    }

    case LLDP_MSG_ID_GET_STAT_REQ: {
        // Master has requested a slave'statistics.
        // Send a message back with the statistics
        lldp_msg_rep_t *msg;

        T_I("Sending STAT ALL reply, isid = %u", isid);

        msg = lldp_msg_rep_alloc(LLDP_MSG_ID_GET_STAT_REP);

        LLDP_CRIT_ENTER(); // Protect lldp_sm (in lldp_get_stat_counters)
        lldp_get_stat_counters(&msg->rep.stat.stat_cnt[0]);
        lldp_get_mib_stats(&msg->rep.stat.global_cnts);
        msg->rep.stat.last_change_time_ago = msg_uptime_get(VTSS_ISID_LOCAL) - last_entry_change_time_this_switch;
        LLDP_CRIT_EXIT();

        lldp_msg_tx(msg, isid, sizeof(msg->rep.stat));
        break;
    }

    case LLDP_MSG_ID_GET_STAT_REP: {
        lldp_msg_rep_t *msg = (lldp_msg_rep_t *)rx_msg;

        T_I("Got STAT ALL reply");
        if (VTSS_ISID_LEGAL(isid)) {
            LLDP_CRIT_ENTER();
            memcpy(&local_stat_cnt[0], &msg->rep.stat.stat_cnt[0], sizeof(local_stat_cnt));
            memcpy(&global_cnt, &msg->rep.stat.global_cnts, sizeof(global_cnt));
            last_entry_change_time_ago_slave_switch = msg->rep.stat.last_change_time_ago;
            LLDP_CRIT_EXIT();
            cyg_flag_setbits(&lldp_msg_id_get_stat_req_flags, 1 << isid); // Signal that the message has been received
        }
        break;
    }

    case LLDP_MSG_ID_STAT_CLR:
        T_I("Rx LLDP_MSG_ID_STAT_CLR msg");
        lldp_local_cnt_clear();
        break;

    case LLDP_MSG_ID_GLOBAL_STAT_CLR:
        T_I("Rx LLDP_MSG_ID_GLOBAL STAT_CLR msg");
        lldp_global_cnt_clear();
        break;

    case LLDP_MSG_ID_GET_ENTRIES_REQ: {
        // Master has requested a slave's LLDP neighbor entries.
        // Send a message back with the configuration
        lldp_msg_rep_t      *msg;
        lldp_remote_entry_t *entries;
        u32                 i;

        T_IG(TRACE_GRP_CONF, "Sending GET-ENTRIES reply");

        msg = lldp_msg_rep_alloc(LLDP_MSG_ID_GET_ENTRIES_REP);
        msg->rep.remote_entries.count = 0;

        LLDP_CRIT_ENTER();// Protect remote_entries (in lldp_remote_get_entries())

        T_IG(TRACE_GRP_CONF, "Getting ENTRIES");
        entries = lldp_remote_get_entries();

        // Only transmit those entries that are valid (otherwise we'd always send approx. 0.5 MByte in e.g. a Jaguar stack).
        for (i = 0; i < LLDP_REMOTE_ENTRIES; i++, entries++) {
            if (entries->in_use) {
                memcpy(&msg->rep.remote_entries.entries[msg->rep.remote_entries.count++], &entries[0], sizeof(lldp_remote_entry_t));
            }
        }

        LLDP_CRIT_EXIT();

        T_DG(TRACE_GRP_CONF, "Sending %u ENTRIES", msg->rep.remote_entries.count);

        lldp_msg_tx(msg, isid, sizeof(msg->rep.remote_entries) - (LLDP_REMOTE_ENTRIES - msg->rep.remote_entries.count) * sizeof(lldp_remote_entry_t));
        break;
    }

    case LLDP_MSG_ID_GET_ENTRIES_REP: {
        lldp_msg_rep_t *msg = (lldp_msg_rep_t *)rx_msg;

        T_IG(TRACE_GRP_CONF, "Got ENTRIES reply, isid = %u", isid);
        if (VTSS_ISID_LEGAL(isid)) {
            LLDP_CRIT_ENTER();
            memset(&local_entries[0], 0, sizeof(local_entries));
            if (msg->rep.remote_entries.count > 0) {
                memcpy(&local_entries[0], &msg->rep.remote_entries.entries[0], msg->rep.remote_entries.count * sizeof(lldp_remote_entry_t));
            }
            LLDP_CRIT_EXIT();
            cyg_flag_setbits(&lldp_msg_id_get_entries_req_flags, 1 << isid); // Signal that the message has been received
        } else {
            T_WG(TRACE_GRP_CONF, "Invalid ISID:%u", isid);
        }
        break;
    }
#ifdef VTSS_SW_OPTION_SNMP
    case LLDP_MSG_ID_SEND_TRAP: {
        lldp_msg_req_t *msg = (lldp_msg_req_t *)rx_msg;
        lldp_mib_stats_t stat;
        int snmp_notification_ena;

        LLDP_CRIT_ENTER();
        snmp_notification_ena = lldp_global.flash.port[isid].snmp_notification_ena[msg->req.send_trap.iport];
        lldp_get_mib_stats(&stat);
        LLDP_CRIT_EXIT();
        T_D("Got LLDP_MSG_ID_SEND_TRAP message, isid = %u, iport = %u, enable = %d", isid, msg->req.send_trap.iport, snmp_notification_ena);
        if ( 1 == snmp_notification_ena ) {
            snmpLLDPNotificationChange(isid, msg->req.send_trap.iport, &stat, msg->req.send_trap.notification_interval);
#ifdef VTSS_SW_OPTION_SECURE_SNMP
#ifdef VTSS_SW_OPTION_LLDP_MED
            snmpLLDPXemMIBNotificationChange(isid, msg->req.send_trap.iport);
#endif
#endif
        }
        break;
    }
#endif
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    T_NG(TRACE_GRP_CONF, "Done isid = %u", isid);
    return TRUE;
}

#if defined(VTSS_SW_OPTION_IP2)
// Function for getting the IP address for a specific port
// IN : iport - Port in question
// Return - IPv4 address.
vtss_ipv4_t lldp_ip_addr_get(vtss_port_no_t iport)
{
    LLDP_CRIT_ASSERT_LOCKED();   // Check that we are semaphore locked. For protection of local_conf

    vtss_ip_addr_t vtss_ip_addr;
    // This is a little be clumsy, because this function is always called within a semaphore, and we do not want be within a semaphore lock when accessing the IP module (in order not to get involved with any deadlock within that module), so we exits the semaphore and re-enter afterward
    // Avoid Lint Warning 455: A thread mutex that had not been locked is being unlocked
    /*lint -e(455) */
    LLDP_CRIT_EXIT();
    // Avoid Lint Warning 454: A thread mutex has been locked but not unlocked
    /*lint --e{454} */

    if (vtss_ip2_ip_by_port(iport, VTSS_IP_TYPE_IPV4, &vtss_ip_addr) == VTSS_RC_OK) {
        T_I_PORT(iport, "Got IP:0x%X", vtss_ip_addr.addr.ipv4);
        LLDP_CRIT_ENTER();
        return vtss_ip_addr.addr.ipv4;
    } else {
        T_I_PORT(iport, "Setting IP to 0");
        LLDP_CRIT_ENTER();
        return 0;
    }
}
#endif // defined(VTSS_SW_OPTION_IP2)

// Function that transmits the master's configuration to a slave or to all slaves
static void master_conf2slave_conf(vtss_isid_t the_isid)
{
    lldp_msg_req_t *msg;
    switch_iter_t  sit;

    T_RG(TRACE_GRP_CONF, "Entering master_conf2slave_conf, isid = %d ", the_isid);

    (void)switch_iter_init(&sit, the_isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        T_DG(TRACE_GRP_CONF, "Transmitting new configuration to isid: %d", sit.isid);

        // Prepare for message tranfer
        msg = lldp_msg_req_alloc(LLDP_MSG_ID_CONF_SET_REQ, 1);
        LLDP_CRIT_ENTER();

        lldp_global_conf2local_conf(&msg->req.set_conf.conf, sit.isid);


        if (system_conf_has_changed) {
            lldp_system_name(&msg->req.set_conf.sys_name[0], 1); // Get master's system name
            msg->req.set_conf.somethingChangedLocal = 1;         // Signal to slaves that some system information has changed.
        } else {
            msg->req.set_conf.somethingChangedLocal = 0;
        }
        T_DG(TRACE_GRP_CONF, "TX New message - system name = %s", msg->req.set_conf.sys_name);

        LLDP_CRIT_EXIT();
        // Transmit the message
        lldp_msg_tx(msg, sit.isid, sizeof(msg->req.set_conf));
    }
    T_RG(TRACE_GRP_CONF, "Exiting master_conf2slave_conf, isid = %d ", the_isid);
}

// Function for getting or setting the local usid (used for stacking as part of the port description TLV). This function is call from the stack module every time usid changes-
// IN : set - TRUE if local usid shall be updated with new usid number.
//      new_set - if set is TRUE local usid is updated with this number
// Return - Current usid for the switch.
vtss_isid_t lldp_local_usid_set_get(BOOL set, vtss_isid_t  new_usid)
{
    static vtss_isid_t usid = 1;

    if (set) {
        usid = new_usid;
    }

    T_NG(TRACE_GRP_CONF, "set:%d, new_usid:%d, usid:%d", set, new_usid, usid);
    return usid;
}

void lldp_something_has_changed(void)
{
    system_conf_has_changed = 1;
}

// Function that checks if configuration has changed and call for an update of all switches. This function shall only be called is the switch is master.
static void chk_for_conf_change(void)
{
    T_RG(TRACE_GRP_CONF, "Enter");

    // Update all switches id configuration has changed
    vtss_common_macaddr_t current_mac_addr;
    system_conf_t         sys_conf;

    if (system_conf_has_changed && msg_switch_is_master()) {
        // Updated MAC address
        vtss_os_get_systemmac(&current_mac_addr);
        T_D("MAC %02x-%02x-%02x-%02x-%02x-%02x ", current_mac_addr.macaddr[0], current_mac_addr.macaddr[1], current_mac_addr.macaddr[2], current_mac_addr.macaddr[3], current_mac_addr.macaddr[4], current_mac_addr.macaddr[5]);


        LLDP_CRIT_ENTER(); // Protect lldp_global
        lldp_global.run_time.mac_addr = current_mac_addr; // update the master mac address

        // Update system name to the slaves (we don't know, but it could have been)
        if (system_get_config(&sys_conf) != VTSS_OK) {
            T_E("Didn't get the system configuration");
            lldp_system_name("Unkown system name", 0); // Set a default system name (should never happen).
        } else {
            lldp_system_name(&sys_conf.sys_name[0], 0); // Copy the master's system name to a local copy
            T_D(" Setting system name to %s", &sys_conf.sys_name[0]);
        }
        LLDP_CRIT_EXIT();

        //Send new configuration to the slaves
        master_conf2slave_conf(VTSS_ISID_GLOBAL);

        LLDP_CRIT_ENTER();
        system_conf_has_changed = 0;
        LLDP_CRIT_EXIT();
    }
}

void lldp_system_name(char *sys_name, char get_name)
{
    static char local_system_name[VTSS_SYS_STRING_LEN]; // Local copy of the system name, send by master to slaves.

    if (get_name) {
        strcpy(sys_name, local_system_name);
    } else {
        // Got new name from master - update the local copy.
        misc_strncpyz(local_system_name, sys_name, VTSS_SYS_STRING_LEN);
    }
    T_N("set_name = %d, local_system_name = %s, sys_name  = %s", get_name, local_system_name, sys_name);
}

/*************************************************************************
** Message module functions
*************************************************************************/
/* Send request and wait for response */
static lldp_bool_t lldp_stack_req_timeout(vtss_isid_t isid, lldp_msg_id_t msg_id, cyg_flag_t *flags)
{
    lldp_msg_req_t   *msg;
    cyg_flag_value_t flag;
    cyg_tick_count_t time_tick;

    T_D("enter, isid: %d", isid);

    // Setup sync flag.
    flag = (1 << isid);
    cyg_flag_maskbits(flags, ~flag);

    if (msg_switch_exists(isid)) {

        // Send the request for getting stat counters
        msg = lldp_msg_req_alloc(msg_id, 1);

        lldp_msg_tx(msg, isid, 0);

        // Wait for timeout or sync flag to be set.
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(LLDP_REQ_TIMEOUT * 1000);
        return (cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1);
    } else {
        T_W("Skipped lldp_msg_tx due to isid:%d msg switch doesn't exist", isid);
        return TRUE;
    }
}

static BOOL lldp_port_is_authorized(lldp_port_t port_idx)
{
    vtss_auth_state_t auth_state;
    (void)vtss_auth_port_state_get(NULL, port_idx, &auth_state);
    return auth_state == VTSS_AUTH_STATE_BOTH;
}

//
// LLDP call back function that is called when a lldp frame is received
//
static lldp_bool_t lldp_rx_pkt(void  *contxt,
                               const lldp_u8_t *const frm_p,
                               const vtss_packet_rx_info_t *const rx_info)
{
    T_IG(TRACE_GRP_RX, "src_port:%u, len:%u, glag_no:%u", rx_info->port_no, rx_info->length, rx_info->glag_no);
    if (lldp_port_is_authorized(rx_info->port_no)) {
        T_IG(TRACE_GRP_RX, "Done src_port:%u, len:%u, glag_no:%u", rx_info->port_no, rx_info->length, rx_info->glag_no);
        LLDP_CRIT_ENTER();
        T_IG(TRACE_GRP_RX, "Done src_port:%u, len:%u, glag_no:%u", rx_info->port_no, rx_info->length, rx_info->glag_no);
        lldp_frame_received(rx_info->port_no - VTSS_PORT_NO_START, frm_p, rx_info->length, rx_info->glag_no); // Calls same procedure as lldp_1sec_timer_tick
        T_IG(TRACE_GRP_RX, "Done src_port:%u, len:%u, glag_no:%u", rx_info->port_no, rx_info->length, rx_info->glag_no);
        LLDP_CRIT_EXIT();
    }
    T_IG(TRACE_GRP_RX, "Done src_port:%u, len:%u, glag_no:%u", rx_info->port_no, rx_info->length, rx_info->glag_no);
    return 0; // Allow other subscribers to receive the packet
}

//
// Call back function that is called when a CDP frame is received
//
#ifdef VTSS_SW_OPTION_CDP
static lldp_bool_t cdp_rx_pkt(void  *contxt,
                              const uchar *const frm_p,
                              const vtss_packet_rx_info_t *const rx_info)
{
    lldp_bool_t eat_frm = 0; // Allow other subscribers to receive the packet

    if (!lldp_port_is_authorized(rx_info->port_no)) {
        return eat_frm;
    }

    LLDP_CRIT_ENTER();
    T_NG(TRACE_GRP_CDP, "Got CDP frame");

    // PID shall be 0x2000 for CDP frame - See http://wiki.wireshark.org/CDP
    if (*(frm_p + 20) == 0x20 && *(frm_p + 21) == 0x00) {
        if (local_conf.cdp_aware[rx_info->port_no - VTSS_PORT_NO_START]) {
            (void)cdp_frame_decode(rx_info->port_no - VTSS_PORT_NO_START, frm_p, rx_info->length); // Calls same procedure as lldp_1sec_timer_tick
        }
    }
    LLDP_CRIT_EXIT();
    return eat_frm;
}
#endif // VTSS_SW_OPTION_CDP

//
// Procedure for subscribing for LLDP packets.
//
static void lldp_pkt_subscribe(void)
{
    packet_rx_filter_t rx_filter;
    static void *filter_id = NULL;
    const vtss_common_macaddr_t vtss_lldp_slowmac = {{0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E}};

    /* LLDP frames registration */
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_LLDP;
    rx_filter.match = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
    rx_filter.cb = lldp_rx_pkt;

    memcpy(rx_filter.dmac, vtss_lldp_slowmac.macaddr, sizeof(rx_filter.dmac));
    rx_filter.etype  = 0x88CC;
    rx_filter.prio   = PACKET_RX_FILTER_PRIO_NORMAL;

    if (packet_rx_filter_register(&rx_filter, &filter_id) != VTSS_OK) {
        T_E("Not possible to register for LLDP packets. LLDP will not work !");
    }
}

#ifdef VTSS_SW_OPTION_CDP
// Subscribe for CDP frames from the packet module (the CDP frame must be added top
// to the MAC table for this to work, see the cdp_add_to_mac_table function).
static void cdp_pkt_subscribe(void)
{
    packet_rx_filter_t rx_filter;
    static void *filter_id = NULL;

    /* CDP frames registration */
    const vtss_mac_t vtss_cdp_slowmac = {{0x01, 0x00, 0x0C, 0xCC, 0xCC, 0xCC}};
    T_DG(TRACE_GRP_CDP, "Registering CDP frames");
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_LLDP;
    rx_filter.match = PACKET_RX_FILTER_MATCH_DMAC;
    rx_filter.cb = cdp_rx_pkt; // Setup callback function

    memcpy(rx_filter.dmac, vtss_cdp_slowmac.addr, sizeof(rx_filter.dmac));
    rx_filter.prio   = PACKET_RX_FILTER_PRIO_NORMAL;

    if (packet_rx_filter_register(&rx_filter, &filter_id) != VTSS_OK) {
        T_E("Not possible to register for CDP packets. CDP will not work !");
    }

    // For getting the CDP frames to the CPU we need to add the CDP DMAC to the MAC table. The MAC
    // table needs to be updated every time the PVID changes, so we registers a callback function
    // for getting informed every time the VLAN port configuration changes.
    vlan_port_conf_change_register(VTSS_MODULE_ID_LLDP, cdp_add_to_mac_table, FALSE /* get called back on local switch after the change has occurred */);
}
#endif // VTSS_SW_OPTION_CDP

//
// Function called when booting
//
static void lldp_start(void)
{
    /* Register for stack messages */
    msg_rx_filter_t filter;

    T_R("entering lldp_start");

    lldp_pkt_subscribe(); // Subscribe for dicovery packets (LLDP)

#ifdef VTSS_SW_OPTION_CDP
    cdp_pkt_subscribe(); // Subscribe for dicovery packets (CDP)
#endif // VTSS_SW_OPTION_CDP

    memset(&filter, 0, sizeof(filter));
    filter.cb = lldp_msg_rx;
    filter.modid = VTSS_TRACE_MODULE_ID;
    T_R("Exiting lldp_start");
    (void)msg_rx_filter_register(&filter);
}

/****************************************************************************
* Module thread
****************************************************************************/
static void lldp_thread(cyg_addrword_t data)
{
    cyg_flag_t       one_sec_timer_flag;
    cyg_tick_count_t time_tick;
    port_iter_t      pit;

    cyg_flag_init(&one_sec_timer_flag);

    // Wait for switch add event has happen, because we don't want the thread to start before we have a valid configuration.
    while (!switch_add) {
        VTSS_OS_MSLEEP(1000);
        T_I("Waiting for switch_add");
    }

    time_tick = cyg_current_time();

    // ***** Go into loop **** //
    T_R("Entering lldp_thread");
    while (1) {
        time_tick += VTSS_OS_MSEC2TICK(1000); // Setup time to wait 1 sec..
        (void)cyg_flag_timed_wait(&one_sec_timer_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, time_tick);

        chk_for_conf_change();

        // Do the one sec timer tick - for front ports only
        LLDP_CRIT_ENTER();
        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);

        while (port_iter_getnext(&pit)) {
            lldp_1sec_timer_tick(pit.iport);
        }

        lldp_remote_1sec_timer();
        LLDP_CRIT_EXIT();
    }
}

// Store the current configuration in flash
static vtss_rc store_conf(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vtss_rc            rc = LLDP_ERROR_FLASH;
    lldp_master_flash_conf_t *blk;
    ulong              size;

    T_DG(TRACE_GRP_CONF, "Storing LLDP configuration in flash");

    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LLDP_CONF, &size)) != NULL) {
        if (size == sizeof(*blk)) {
            LLDP_CRIT_ENTER();
            *blk = lldp_global.flash;
            blk->version = LLDP_CONF_VERSION; // Make sure that version matches
            LLDP_CRIT_EXIT();
            rc = VTSS_OK;
        } else {
            T_W("Could not store LLDP configuration - Size did not match");
        }
        T_DG(TRACE_GRP_CONF, "Flash version:%lu", blk->version);
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LLDP_CONF);
    } else {
        T_W("Could not store LLDP configuration");
    }

    return rc;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

//************************************************
// Configuration
//************************************************
static void lldp_conf_default_set(void)
{
    vtss_common_macaddr_t current_mac_addr;
    lldp_16_t port_index;
    lldp_16_t sid_index;
    // Set default configuration

#ifdef VTSS_SW_OPTION_LLDP_MED
    lldp_16_t civic_index;
    lldp_16_t policy_index;
#endif

    LLDP_CRIT_ENTER();
    // Set everything to 0. Non-zero default values will be set below.
    memset(&lldp_global, 0, sizeof(lldp_global));

    for (sid_index = 0; sid_index < VTSS_ISID_END; sid_index++) {
        for (port_index = 0; port_index < LLDP_PORTS; port_index++) {
            lldp_global.flash.port[sid_index].optional_tlvs[port_index] = 0x1F;
            lldp_global.flash.port[sid_index].snmp_notification_ena[port_index] = 2; // Setting notification to disabled (2).
            lldp_global.flash.port[sid_index].admin_states[port_index] = LLDP_ADMIN_STATE_DEFAULT;
#ifdef VTSS_SW_OPTION_CDP
            lldp_global.flash.port[sid_index].cdp_aware[port_index] = LLDP_CDP_AWARE_DEFAULT;
#endif

#ifdef VTSS_SW_OPTION_LLDP_MED
            // Just for information : When we went from vCLI to iCLI we had to change to the default value for optional LLDP-MED
            // from enabled to disabled in order to be able to make a logical iCLI "no command".
            lldp_global.flash.common.lldpmed_optional_tlv[port_index] = 0x3F;
            lldp_global.flash.port[sid_index].lldpmed_snmp_notification_ena[port_index] = 2; // Setting notification to disabled (2).
#endif
        }
    }

#ifdef VTSS_SW_OPTION_LLDP_MED
    // Reserve a "space" for each ca type
    for (civic_index = 0; civic_index < LLDPMED_CATYPE_CNT; civic_index++) {
        lldp_global.flash.common.location_info.civic.civic_ca_type_array[civic_index] = (lldpmed_catype_t)civic_index;
    }

    lldp_global.flash.common.location_info.altitude_type = LLDPMED_ALTITUDE_TYPE_DEFAULT;
    lldp_global.flash.common.location_info.altitude      = LLDPMED_ALTITUDE_DEFAULT;

    lldp_global.flash.common.location_info.longitude_dir = LLDPMED_LONGITUDE_DIR_DEFAULT;
    lldp_global.flash.common.location_info.longitude     = LLDPMED_LONGITUDE_DEFAULT;


    lldp_global.flash.common.location_info.latitude_dir  = LLDPMED_LATITUDE_DIR_DEFAULT;
    lldp_global.flash.common.location_info.latitude      = LLDPMED_LATITUDE_DEFAULT;

    lldp_global.flash.common.medFastStartRepeatCount     = FAST_START_REPEAT_COUNT_DEFAULT;
    lldp_global.flash.common.location_info.datum         = LLDPMED_DATUM_DEFAULT;

    for (policy_index = LLDPMED_POLICY_MIN; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
        lldp_global.flash.common.policies_table[policy_index].application_type = VOICE;
    }
#endif
    lldp_global.flash.common.reInitDelay                 = LLDP_REINIT_DEFAULT;
    lldp_global.flash.common.msgTxHold                   = LLDP_TX_HOLD_DEFAULT;
    lldp_global.flash.common.txDelay                     = LLDP_TX_DELAY_DEFAULT;
    lldp_global.flash.common.msgTxInterval               = LLDP_TX_INTERVAL_DEFAULT;
    lldp_global.flash.common.snmp_notification_interval  = 5; // Suggested value by the IEEE standard.
    vtss_os_get_systemmac(&current_mac_addr);
    lldp_global.run_time.mac_addr                        = current_mac_addr; // update the master mac address

    LLDP_CRIT_EXIT();
}

#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
// Function for applying a configuration done with 2.80 release to newer release.
static void silent_upgrade(lldp_master_flash_conf_t *blk)
{
    lldp_master_conf_280_t *lldp_master_conf_280;

    T_IG(TRACE_GRP_CONF, "LLDP silent upgrade");

    // This func is called on the Init Modules thread which has a small stack and the 2.80 block is too large for it.
    // Thus, we malloc.

    lldp_master_conf_280 = (lldp_master_conf_280_t *)VTSS_MALLOC(sizeof(*lldp_master_conf_280));
    if (!lldp_master_conf_280) {
        T_WG(TRACE_GRP_CONF, "LLDP silent upgrade failed; out of memory");
        return;
    }

    memcpy(lldp_master_conf_280, blk, sizeof(*lldp_master_conf_280)); // blk do at this point in time contain the 2.80 release flash layout
    memset(blk, 0, sizeof(*blk)); //Default all configuration to a known value.
    T_IG(TRACE_GRP_CONF, "lldp_master_conf_280.version:%lu, blk->version:%lu", lldp_master_conf_280->version, blk->version);
    blk->version                           = lldp_master_conf_280->version;
    blk->common.reInitDelay                = lldp_master_conf_280->conf.reInitDelay;
    blk->common.msgTxHold                  = lldp_master_conf_280->conf.msgTxHold;
    blk->common.msgTxInterval              = lldp_master_conf_280->conf.msgTxInterval;
    blk->common.txDelay                    = lldp_master_conf_280->conf.txDelay;
    blk->common.snmp_notification_interval = lldp_master_conf_280->snmp_notification_interval;

    vtss_isid_t isid;
    vtss_port_no_t port_no;
    for (isid = 0; isid < VTSS_ISID_END; isid++) {
        for (port_no = 0; port_no < LLDP_PORTS; port_no++) {
            blk->port[isid].admin_states[port_no]          = lldp_master_conf_280->admin_states[isid][port_no];
            blk->port[isid].optional_tlvs[port_no]         = lldp_master_conf_280->optional_tlvs[isid][port_no];
            blk->port[isid].snmp_notification_ena[port_no] = lldp_master_conf_280->snmp_notification_ena[isid][port_no];

#ifdef VTSS_SW_OPTION_CDP
            blk->port[isid].cdp_aware[port_no]             = lldp_master_conf_280->cdp_aware[isid][port_no];
#endif
        }
    }

#ifdef VTSS_SW_OPTION_LLDP_MED
    memcpy(&blk->common.policies_table[0], &lldp_master_conf_280->conf.policies_table[0], sizeof(lldpmed_policies_table_t));
    memcpy(&blk->common.location_info, &lldp_master_conf_280->conf.location_info, sizeof(lldpmed_location_info_t));

    memcpy(&blk->common.lldpmed_optional_tlv[0], &lldp_master_conf_280->conf.lldpmed_optional_tlv[0], sizeof(lldpmed_optional_tlv_t));
    blk->common.medFastStartRepeatCount = lldp_master_conf_280->conf.medFastStartRepeatCount;

    for (isid = 0; isid < VTSS_ISID_END; isid++) {
        for (port_no = 0; port_no < LLDP_PORTS; port_no++) {
            blk->port[isid].lldpmed_snmp_notification_ena[port_no] = lldp_master_conf_280->lldpmed_snmp_notification_ena[isid][port_no];
            u8 policy_id;
            for (policy_id = 0; policy_id < LLDPMED_POLICIES_CNT; policy_id++) {
                blk->common.ports_policies[port_no][policy_id] = lldp_master_conf_280->lldpmed_port_policies[isid][port_no][policy_id];
            }
        }
    }
#endif
    VTSS_FREE(lldp_master_conf_280);
}
#endif

static void lldp_conf_read(lldp_bool_t create)
{
    lldp_master_flash_conf_t *blk;
    ulong              size;
    lldp_bool_t        do_create = FALSE;
    if (misc_conf_read_use()) {
        T_DG(TRACE_GRP_CONF, "misc_conf_read_use");
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LLDP_CONF, &size)) == NULL || size != sizeof(*blk)) {
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            T_DG(TRACE_GRP_CONF, "size:%d, sizeof(lldp_master_conf_280_t):%d, sizeof(lldp_master_flash_conf_t):%d", size, sizeof(lldp_master_conf_280_t), sizeof(lldp_master_flash_conf_t));
            T_IG(TRACE_GRP_CONF, "lldp_struc_0_t:%d, lldp_struc_0_280_t:%d", sizeof(lldp_struc_0_t), sizeof(lldp_struc_0_280_t));
            if (blk != NULL && size == sizeof(lldp_master_conf_280_t)) {
                silent_upgrade(blk);
            } else {
#endif
                blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_LLDP_CONF, sizeof(*blk));
                T_W("conf_sec_open failed or size mismatch, creating defaults");
                do_create = TRUE;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            }
#endif
        } else if (blk->version != LLDP_CONF_VERSION) {
            T_W("version mismatch, creating defaults, flash_ver:%lu, current_ver:%u", blk->version, LLDP_CONF_VERSION);
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        blk        = NULL;
        do_create  = TRUE;
    }

    if (do_create) {
        T_DG(TRACE_GRP_CONF, "Restore to default value - block size =  %zu", sizeof(*blk));
        lldp_conf_default_set();
        system_conf_has_changed = 1; // Signal that new configuration has been done (making sure that system name is updated as well).
    } else {
        if (blk != NULL) {
            LLDP_CRIT_ENTER();
            lldp_global.flash = *blk;
            LLDP_CRIT_EXIT();
        }
    }

    LLDP_CRIT_ENTER();
    lldp_global_conf2local_conf(&local_conf, VTSS_ISID_LOCAL);
    LLDP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (blk) {
        LLDP_CRIT_ENTER(); // Protect lldp_global
        T_DG(TRACE_GRP_CONF, "size of lldp_global = %zu", sizeof(lldp_global));
        *blk = lldp_global.flash;
        LLDP_CRIT_EXIT();
        blk->version = LLDP_CONF_VERSION;
        T_DG(TRACE_GRP_CONF, "Closing CONF_BLK_LLDP_CONF, version:%lu", blk->version);
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LLDP_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_DG(TRACE_GRP_CONF, "Done");
}

#ifdef VTSS_SW_OPTION_VOICE_VLAN
// Check for Voice VLAN cross-reference for misconfiguration
static vtss_rc lldp_voice_vlan_chk(vtss_isid_t isid, const admin_state_t admin_state)
{
    lldp_port_t iport;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {

        // Make sure that we don't get out of bounds
        if (iport >= LLDP_PORTS) {
            continue;
        }
        T_DG_PORT(TRACE_GRP_CONF, iport, "Check for Voice Vlan configuration mismatch, isid = %d", isid);
        if (voice_vlan_is_supported_LLDP_discovery(isid, iport)) {
            T_DG_PORT(TRACE_GRP_CONF, iport, "voice_vlan_is_supported_LLDP_discovery");
            if (admin_state[iport] == LLDP_ENABLED_TX_ONLY || admin_state[iport] == LLDP_DISABLED) {
                T_DG_PORT(TRACE_GRP_CONF, iport, "Voice Vlan configuration mismatch %d", LLDP_ERROR_VOICE_VLAN_CONFIGURATION_MISMATCH);
                return LLDP_ERROR_VOICE_VLAN_CONFIGURATION_MISMATCH;
            }
        }
    }

    return VTSS_RC_OK;
}
#endif

/****************************************************************************/
/*  API functions                                                           */
/****************************************************************************/

// Because we want to get the whole LLDP entry table/statistics from the switch, and since the entry table can be quite large (280K) at
// the time of writing, we give the applications the possibility to take the semaphore, and work directly with the entry table in the LLDP module,
// instead of having to copy the table between the different modules.
void lldp_mgmt_get_lock(void)
{
    // Avoid Lint Warning 454: A thread mutex has been locked but not unlocked
    /*lint --e{454} */
    LLDP_IF_CRIT_ENTER();
}

void lldp_mgmt_get_unlock(void)
{
    // Avoid Lint Warning 455: A thread mutex that had not been locked is being unlocked
    /*lint -e(455) */
    LLDP_IF_CRIT_EXIT();
}

void lldp_mgmt_last_change_ago_to_str(time_t last_change_ago, char *last_change_str)
{
    time_t last_change_time = msg_uptime_get(VTSS_ISID_LOCAL) - last_change_ago;
    sprintf(last_change_str, "%s  (%u secs. ago)", misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, last_change_time)), last_change_ago);
}

// Returns statistics for a switch, and global counters and last change for the stack as a whole.
vtss_rc lldp_mgmt_stat_get(vtss_isid_t isid, lldp_counters_rec_t *stat_cnt, lldp_mib_stats_t *global_stat_cnt, time_t *last_change_ago)
{
    switch_iter_t sit;
    vtss_isid_t   req_isid;

    if (stat_cnt == NULL && global_stat_cnt == NULL && last_change_ago == NULL) {
        return VTSS_RC_OK;
    }

    if (isid == VTSS_ISID_LOCAL || !VTSS_ISID_LEGAL(isid)) {
        LLDP_CRIT_ENTER();
        if (stat_cnt != NULL) {
            lldp_get_stat_counters(&local_stat_cnt[0]);
            memcpy(&stat_cnt[0], &local_stat_cnt[0], sizeof(local_stat_cnt));
        }
        if (global_stat_cnt != NULL) {
            lldp_get_mib_stats(global_stat_cnt);
        }
        if (last_change_ago != NULL) {
            *last_change_ago = msg_uptime_get(VTSS_ISID_LOCAL) - last_entry_change_time_this_switch;
        }
        LLDP_CRIT_EXIT();
        return VTSS_ISID_LEGAL(isid) ? VTSS_OK : LLDP_ERROR_ISID;
    }

    if (stat_cnt != NULL) {
        lldp_clr_stat_counters(stat_cnt);
    }

    if (global_stat_cnt != NULL) {
        memset(global_stat_cnt, 0, sizeof(*global_stat_cnt));
    }

    if (last_change_ago != NULL) {
        *last_change_ago = 0;
    }

    req_isid = (global_stat_cnt != NULL || last_change_ago != NULL) ? VTSS_ISID_GLOBAL : isid;

    (void)switch_iter_init(&sit, req_isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (lldp_stack_req_timeout(sit.isid, LLDP_MSG_ID_GET_STAT_REQ, &lldp_msg_id_get_stat_req_flags)) {
            T_W("Timeout, STAT_REQ(isid = %d)", sit.isid);
        } else {
            LLDP_CRIT_ENTER();
            // Add the global counter from the switch to the total sum.
            if (global_stat_cnt != NULL) {
                global_stat_cnt->table_inserts += global_cnt.table_inserts;
                global_stat_cnt->table_deletes += global_cnt.table_deletes;
                global_stat_cnt->table_drops   += global_cnt.table_drops;
                global_stat_cnt->table_ageouts += global_cnt.table_ageouts;
            }

            if (last_change_ago != NULL) {
                if ((*last_change_ago == 0) || (*last_change_ago > last_entry_change_time_ago_slave_switch)) {
                    *last_change_ago = last_entry_change_time_ago_slave_switch; // The reply from this switch shows that the entry were the last one changed.
                }
            }

            if (stat_cnt != NULL && sit.isid == isid) {
                memcpy(&stat_cnt[0], &local_stat_cnt[0], sizeof(local_stat_cnt));    // Ok - Reply from slave was ok - local_stat_cnt is updated when the message was received.
            }
            LLDP_CRIT_EXIT();
        }
    }

    return VTSS_RC_OK;
}

void lldp_mgmt_stat_clr(vtss_isid_t isid)
{
    lldp_msg_req_t *msg;
    switch_iter_t  sit;

    if (isid == VTSS_ISID_LOCAL) {
        lldp_local_cnt_clear();
        return;
    }

    // Signal to the switch to clear it's local counters
    if (msg_switch_exists(isid)) {
        T_DG(TRACE_GRP_STAT, "Transmitting clear LLDP statistics to isid: %d", isid);
        // Prepare for message tranfer
        msg = lldp_msg_req_alloc(LLDP_MSG_ID_STAT_CLR, 1);
        lldp_msg_tx(msg, isid, 0);
    }

    // Signal to all slaves to clear their global counters
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = lldp_msg_req_alloc(LLDP_MSG_ID_GLOBAL_STAT_CLR, sit.remaining)) != NULL) {
        while (switch_iter_getnext(&sit)) {
            lldp_msg_tx(msg, isid, 0);
        }
    } else {
        T_I("Skipped lldp_msg_tx because we're no longer master");
    }
}

// Returns the neighbor entries for a switch
lldp_remote_entry_t *lldp_mgmt_get_entries(vtss_isid_t isid)
{
    lldp_remote_entry_t *entries = NULL;

    if (isid == VTSS_ISID_LOCAL) {
        T_DG(TRACE_GRP_CONF, "Getting entries for Local SID");
        LLDP_CRIT_ENTER();
        entries =  lldp_remote_get_entries();
        LLDP_CRIT_EXIT();
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("isid:%d not legal", isid);
        LLDP_CRIT_ENTER();
        entries =  lldp_remote_get_entries();
        LLDP_CRIT_EXIT();
    } else {
        if (lldp_stack_req_timeout(isid, LLDP_MSG_ID_GET_ENTRIES_REQ, &lldp_msg_id_get_entries_req_flags)) {
            // We did not get a reply from the slave
            T_W("Timeout, Entries data for switch id:%d can not be trusted", isid);
        }
        LLDP_CRIT_ENTER();
        entries  = &local_entries[0];
        LLDP_CRIT_EXIT();
    }

    return entries;      // Ok - Reply from slave was ok - local_entries is updated when the message was received.
}

//
// Function that returns the current configuration. You must always be semaphore locked when calling this function.
//
// In : isid - isid for the switch the shall return its configuration. if isid == VTSS_ISID_GLOBAL, admin_state,
//             optional_tlv, port_policies, and cdp_aware are not valid in the returned conf.
//
// In/out : conf - Pointer to configuration struct where the current configuration is copied to.
//
void lldp_get_config(lldp_struc_0_t *conf, vtss_isid_t isid)
{
    LLDP_CRIT_ASSERT_LOCKED();   // Check that we are semaphore locked.
    if (isid != VTSS_ISID_LOCAL) {
        T_NG(TRACE_GRP_CONF, "isid not VTSS_ISID_LOCAL");
        if (VTSS_ISID_LEGAL(isid)) {
            lldp_global_conf2local_conf(conf, isid);
        }
    } else {
        *conf = local_conf;
        T_RG(TRACE_GRP_CONF, "isid is VTSS_ISID_LOCAL");
    }
}

// Management function for returning current configuration. Simply semphore locking before calling the
// real get_config function.
void lldp_mgmt_get_config(lldp_struc_0_t *conf, vtss_isid_t isid)
{
    LLDP_CRIT_ENTER();
    lldp_get_config(conf, isid);
    LLDP_CRIT_EXIT();
}

//
// Function for getting an unique port id for each port in a stack.
//
// In : iport - Internal port number (starting from 0)
// Return : Unique port id
vtss_port_no_t lldp_mgmt_get_unique_port_id(vtss_port_no_t iport)
{
    vtss_port_no_t unique_port_id;
    LLDP_CRIT_ASSERT_LOCKED();   // Check that we are semaphore locked (for protecting local_conf).
    unique_port_id = GET_UNIQUE_PORT_ID_USID(lldp_local_usid_set_get(FALSE, 0), iport);
    return unique_port_id;
}

// Function for checking if a configuration is valid
static vtss_rc lldp_conf_valid(lldp_common_conf_t *common_conf)
{
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        return LLDP_ERROR_NOT_MASTER;
    }

    // txDelay must not be larger than 0.25 * TxInterval - IEEE 802.1AB-clause 10.5.4.2
    if (common_conf->txDelay > (common_conf->msgTxInterval / 4)) {
        return LLDP_ERROR_TX_DELAY;
    }

    return VTSS_RC_OK;
}

// Function for setting configuration that is common for all switches in a stack
// In - common_conf : Pointer to where to new configuration
vtss_rc lldp_mgmt_set_common_config(lldp_common_conf_t *common_conf)
{

    VTSS_RC(lldp_conf_valid(common_conf));

    LLDP_CRIT_ENTER(); // Protect common_conf
    lldp_global.run_time.timingChanged = 1; /* Indicate timer change */
    memcpy(&lldp_global.flash.common, common_conf, sizeof(lldp_common_conf_t));
    LLDP_CRIT_EXIT(); // Protect common_conf

    master_conf2slave_conf(VTSS_ISID_GLOBAL); // Update all switches with the new configuration

    // Store the new configuration
    return store_conf();
}

// Function for getting configuration that is common for all switches in a stack
// In - common_conf : Pointer to where to put the configuration
void lldp_mgmt_get_common_config(lldp_common_conf_t *common_conf)
{
    LLDP_CRIT_ENTER(); // Protect common_conf
    memcpy(common_conf, &lldp_global.flash.common, sizeof(lldp_common_conf_t));
    LLDP_CRIT_EXIT(); // Protect common_conf
}

//
// Setting the current configuration
//
vtss_rc lldp_mgmt_set_config(lldp_struc_0_t *conf, vtss_isid_t isid)
{
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    vtss_rc rc;
#endif

    T_RG(TRACE_GRP_CONF, "Entering lldp_set_config");

    lldp_common_conf_t common_conf;
    lldp_local_conf2common_conf(conf, &common_conf);
    VTSS_RC(lldp_conf_valid(&common_conf));

    if (isid == VTSS_ISID_LOCAL) {
        return LLDP_ERROR_ISID_LOCAL;
    }

#ifdef VTSS_SW_OPTION_VOICE_VLAN
    if (VTSS_ISID_LEGAL(isid) && (rc = lldp_voice_vlan_chk(isid, &conf->admin_state[0])) != VTSS_RC_OK) {
        return rc;
    }
#endif

    // Ok now we can do the configuration
    LLDP_CRIT_ENTER(); // Protect local_conf
    lldp_local_conf2global_conf(conf, isid); // Update the common configuration for all switches
    LLDP_CRIT_EXIT();

    master_conf2slave_conf(VTSS_ISID_GLOBAL); // Update all switches with the new configuration

    // Store the new configuration
    return store_conf();
}

//
// Setting Admin state
//
vtss_rc lldp_mgmt_set_admin_state(const admin_state_t admin_state, vtss_isid_t isid)
{
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    vtss_rc rc;
#endif

    T_RG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_admin_state");
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_I("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_I("Isid:%d invalid", isid);
        return LLDP_ERROR_ISID;
    }

    // Check for Voice VLAN crossreference for misconfiguration.
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    if ((rc = lldp_voice_vlan_chk(isid, admin_state)) != VTSS_RC_OK) {
        return rc;
    }
#endif

    // Ok we are allowed to do configuration changes
    T_RG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_admin_state");
    LLDP_CRIT_ENTER();
    memcpy(lldp_global.flash.port[isid].admin_states, &admin_state[0], sizeof(lldp_global.flash.port[isid].admin_states)); // Update the configuration for the switch in question
    LLDP_CRIT_EXIT();

    master_conf2slave_conf(isid); // Use message module to tranfer the new configuration from master to slave

    // Store the new configuration
    return store_conf();
}

//
// Setting CDP Awareness
//
#ifdef VTSS_SW_OPTION_CDP
vtss_rc lldp_mgmt_set_cdp_aware(cdp_aware_t cdp_aware, vtss_isid_t isid)
{
    T_RG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_cdp_aware");
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("Isid:%d invalid", isid);
        return LLDP_ERROR_ISID;
    }

    // Ok we are allowed to do configuration changes
    LLDP_CRIT_ENTER();
    memcpy(lldp_global.flash.port[isid].cdp_aware, &cdp_aware[0], sizeof(lldp_global.flash.port[isid].cdp_aware)); // Update the configuration for the switch in question
    LLDP_CRIT_EXIT();

    master_conf2slave_conf(isid); // Use message module to tranfer the new configuration from master to slave

    // Store the new configuration
    return store_conf();
}
#endif // VTSS_SW_OPTION_CDP

//
// Optional TLVs.
//
vtss_rc lldp_mgmt_set_optional_tlvs(optional_tlv_t optional_tlv, vtss_isid_t isid)
{
    T_RG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_optional_tlvs");

    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("Isid:%d invalid", isid);
        return LLDP_ERROR_ISID;
    }

    // Ok we are allowed to do configuration changes
    LLDP_CRIT_ENTER();
    memcpy(lldp_global.flash.port[isid].optional_tlvs, &optional_tlv[0], sizeof(lldp_global.flash.port[isid].optional_tlvs)); // Update the configuration for the switch in question
    LLDP_CRIT_EXIT();

    master_conf2slave_conf(isid); // Use message module to tranfer the new configuration from master to slave

    // Store the new configuration
    return store_conf();
}

//
// Notification interval for SNMP Trap.
//
// Setting a new notification interval.
vtss_rc lldp_mgmt_set_notification_interval(int notification_interval)
{
    T_NG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_notification_interval");

    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (notification_interval < 5 || notification_interval > 3600) {
        // Interval defined by IEEE is 5-3600 sec.
        return LLDP_ERROR_NOTIFICATION_INTERVAL_VALUE;
    }


    // Ok we are allowed to do configuration changes
    LLDP_CRIT_ENTER();
    lldp_global.flash.common.snmp_notification_interval = notification_interval;
    LLDP_CRIT_EXIT();

    // Store the new configuration
    return store_conf();
}

// Getting current notification interval.
int lldp_mgmt_get_notification_interval(BOOL crit_region_not_set)
{
    int current_notification_interval = 0;


    // get the current value
    if (crit_region_not_set) {
        LLDP_CRIT_ENTER();
        current_notification_interval = lldp_global.flash.common.snmp_notification_interval;
        LLDP_CRIT_EXIT();
    } else {
        LLDP_CRIT_ASSERT_LOCKED();
        current_notification_interval = lldp_global.flash.common.snmp_notification_interval;
    }

    // return the value.
    return current_notification_interval;
}

// Setting a new notification enable.
vtss_rc lldp_mgmt_set_notification_ena(int notification_ena, lldp_port_t port, vtss_isid_t isid)
{
    T_NG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_notification_interval_ena");

    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (notification_ena < 1 || notification_ena > 2) {
        return LLDP_ERROR_NOTIFICATION_VALUE;
    }

    // Ok we are allowed to do configuration changes
    LLDP_CRIT_ENTER();
    lldp_global.flash.port[isid].snmp_notification_ena[port] = notification_ena;
    LLDP_CRIT_EXIT();

    // Store the new configuration
    return store_conf();
}

// Getting current notification interval.
int lldp_mgmt_get_notification_ena(lldp_port_t port, vtss_isid_t isid, BOOL crit_region_not_set)
{
    int current_notification_ena = 1;

    T_NG(TRACE_GRP_CONF, "Entering lldp_mgmt_get_notification_ena");
    // get the current value
    if (crit_region_not_set) {
        LLDP_CRIT_ENTER();
        current_notification_ena = lldp_global.flash.port[isid].snmp_notification_ena[port];
        LLDP_CRIT_EXIT();
    } else {
        LLDP_CRIT_ASSERT_LOCKED();
        current_notification_ena = lldp_global.flash.port[isid].snmp_notification_ena[port];
    }

    return current_notification_ena;
}

#ifdef VTSS_SW_OPTION_LLDP_MED
// lldpXMedMIB.c needs to get the location tlv information, and
// these functions must be called within a crtital region, so therefore
// there the 3 mgmt function below is made in order not to do direct calls.
lldp_u8_t lldp_mgmt_lldpmed_coordinate_location_tlv_add(lldp_u8_t *buf)
{
    lldp_u8_t result;
    LLDP_CRIT_ENTER();
    result = lldpmed_coordinate_location_tlv_add(buf);
    LLDP_CRIT_EXIT();
    return result;
}

lldp_u8_t lldp_mgmt_lldpmed_civic_location_tlv_add(lldp_u8_t *buf)
{
    lldp_u8_t result;
    LLDP_CRIT_ENTER();
    result = lldpmed_civic_location_tlv_add(buf);
    LLDP_CRIT_EXIT();
    return result;
}

lldp_u8_t lldp_mgmt_lldpmed_ecs_location_tlv_add(lldp_u8_t *buf)
{
    lldp_u8_t result;
    LLDP_CRIT_ENTER();
    result = lldpmed_ecs_location_tlv_add(buf);
    LLDP_CRIT_EXIT();
    return result;
}

//
// Setting notification enable bit for a port.
//
vtss_rc lldpmed_mgmt_set_notification_ena(int notification_ena, lldp_port_t port, vtss_isid_t isid)
{
    T_NG(TRACE_GRP_CONF, "Entering lldp_mgmt_set_notification_interval_ena");

    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return LLDP_ERROR_NOT_MASTER;
    } else if (notification_ena < 1 || notification_ena > 2) {
        T_W("Notification enable must be either 1 (enable) or 2 (disable) ");
        return LLDP_ERROR_NOTIFICATION_VALUE;
    }

    // Ok we are allowed to do configuration changes
    LLDP_CRIT_ENTER();
    lldp_global.flash.port[isid].lldpmed_snmp_notification_ena[port] = notification_ena;
    LLDP_CRIT_EXIT();

    // Store the new configuration
    return store_conf();
}

// Getting current notification interval.
int lldpmed_mgmt_get_notification_ena(lldp_port_t port, vtss_isid_t isid)
{
    int current_notification_ena = 1;
    // get the current value
    LLDP_CRIT_ENTER();
    current_notification_ena = lldp_global.flash.port[isid].lldpmed_snmp_notification_ena[port];
    LLDP_CRIT_EXIT();
    return current_notification_ena;
}
#endif// VTSS_SW_OPTION_LLDP_MED

//
// Management function for gettting if a optional tlv is enabled (semaphore protected)
//
// In : tlv_t : The TLV to check is it is enabled.
//      port  : The port to check.
//      isid  : the Switch to check.
//
// Return : 1 = TLV is enabled, 0 TLV is disabled
lldp_u8_t lldp_mgmt_get_opt_tlv_enabled(lldp_tlv_t tlv_t, lldp_u8_t port, vtss_isid_t isid)
{
    LLDP_CRIT_ENTER();
    lldp_u8_t result = lldp_os_get_optional_tlv_enabled (tlv_t, port, isid);
    LLDP_CRIT_EXIT();
    return result;
}

//
// Callback list containing registered callback functions to be
// called when an entry is updated
//
typedef struct {
    lldp_callback_t callback_ptr; // Pointer to the callback function
    lldp_bool_t     active;       // Signaling that this entry in the list is assigned to a callback function
} lldp_entry_updated_callback_list_t;

static lldp_entry_updated_callback_list_t callback_list[LLDP_REMOTE_ENTRIES];  // Array containing the callback functions

//
// Function that loops through all the callback list and calls registered functions
//
// In : iport : The iport that the entry was mapped to.
//      entry : The entry that has been changed
//
static void lldp_call_entry_updated_callbacks(lldp_port_t iport, lldp_remote_entry_t *entry)
{
    lldp_16_t callback_list_index;

    LLDP_CRIT_ASSERT_LOCKED(); // We MUST be crit locked in this function. Verify that we are locked.

    // Loop through all registered callbacks and execute the callback that.
    for (callback_list_index = 0; callback_list_index < (int) ARRSZ(callback_list); callback_list_index++)   {
        if (callback_list[callback_list_index].active) {
            if (callback_list[callback_list_index].callback_ptr == NULL) {
                // Should never happen.
                T_E("Callback for callback_list_index=%d is NULL", callback_list_index);
            } else {
                // Execute the callback function
                callback_list[callback_list_index].callback_ptr(iport, entry);
            }
        }
    }
}

//
// Management function for registering a callback function.
//
// in : cb : The function that shall be called when an entry has been updated
//
void lldp_mgmt_entry_updated_callback_register(lldp_callback_t cb)
{
    lldp_16_t   free_idx = 0, idx;
    lldp_bool_t free_idx_found = FALSE;

    T_RG(TRACE_GRP_CONF, "Entering lldp_mgmt_entry_updated_callback_register");
    LLDP_CRIT_ENTER();

    for (idx = 0; idx < (int) ARRSZ(callback_list); idx++) {
        // Search for a free index.
        if (callback_list[idx].active == 0) {
            free_idx = idx;
            free_idx_found = TRUE;
            break;
        }
    }

    if (!free_idx_found) {
        T_E("Trying to register too many lldp entry updated callbacks, please increase the callback array");
    } else {
        callback_list[free_idx].callback_ptr = cb; // Register the callback function
        callback_list[free_idx].active = 1;
    }
    LLDP_CRIT_EXIT();
}

//
// Management function for unregistering a callback function.
//
// in : cb : The function that shall be called when an entry has been updated
//
void lldp_mgmt_entry_updated_callback_unregister(lldp_callback_t cb)
{
    lldp_16_t   cb_idx = 0, idx;
    lldp_bool_t cb_idx_found = FALSE;

    LLDP_CRIT_ENTER();

    for (idx = 0; idx < (int) ARRSZ(callback_list); idx++) {
        // Search for a cb index.
        if (callback_list[cb_idx].callback_ptr == cb) {
            cb_idx = idx;
            cb_idx_found = TRUE;
            break;
        }
    }

    if (!cb_idx_found) {
        T_E("Trying to un-register a callback function that hasn't been registered");
    } else {
        callback_list[cb_idx].active = 0;
    }
    LLDP_CRIT_EXIT();
}

//
// Debug functions
//

#ifdef VTSS_SW_OPTION_LLDP_MED_DEBUG
//
// Management function for setting the memTransmitEnabled global variables (semaphore protected)
//
// In : Port index : The port index for the corresponding memTransmitEnabled variable
//
void lldp_mgmt_set_med_transmit_var(lldp_port_t iport, lldp_bool_t tx_enable)
{
    LLDP_CRIT_ENTER();
    lldpmed_medTransmitEnabled_set(iport, tx_enable);
    LLDP_CRIT_EXIT();
}

//
// Management function for gettting the memTransmitEnabled global variables (semaphore protected)
//
// In : Port index : The port index for the corresponding memTransmitEnabled variable
//
// Return : memTransmitEnabled variable for the selected port (TRUE = enable)
lldp_bool_t lldp_mgmt_get_med_transmit_var(lldp_port_t iport, lldp_bool_t tx_enable)
{
    LLDP_CRIT_ENTER();
    lldp_bool_t result = lldpmed_medTransmitEnabled_get(iport);
    LLDP_CRIT_EXIT();
    return result;
}
#endif // VTSS_SW_OPTION_LLDP_MED_DEBUG


//
// Statistics
//

// When ever an entry is added or deleted this function is called.
// This is used for "stat counters" to show the time of the last entry change
static void lldp_stat_update_last_entry_time(lldp_port_t port_index)
{
#ifdef VTSS_SW_OPTION_SNMP
    lldp_msg_req_t *msg;
#endif
    LLDP_CRIT_ASSERT_LOCKED(); // We MUST be crit locked in this function. Verify that we are locked.
    T_NG(TRACE_GRP_STAT, "Transmitting LLDP entries changed");

    last_entry_change_time_this_switch = msg_uptime_get(VTSS_ISID_LOCAL);
#ifdef VTSS_SW_OPTION_SNMP
    msg = lldp_msg_req_alloc(LLDP_MSG_ID_SEND_TRAP, 1);
    msg->req.send_trap.iport = port_index;
    msg->req.send_trap.notification_interval = lldp_global.flash.common.snmp_notification_interval;

    T_D("Send LLDP_MSG_ID_SEND_TRAP message, iport = %u", msg->req.send_trap.iport);
    lldp_msg_tx(msg, 0, sizeof(msg->req.send_trap));
    T_D("Send LLDP_MSG_ID_SEND_TRAP message done");
#endif
}

//
// When ever an entry is added or deleted this function MUST be called.
//
// In : entry : The entry that has been changed.
//
void lldp_entry_changed(lldp_remote_entry_t *entry)
{
    LLDP_CRIT_ASSERT_LOCKED(); // We MUST be crit locked in this function. Verify that we are locked.
    lldp_stat_update_last_entry_time(entry->receive_port); //
    lldp_call_entry_updated_callbacks(entry->receive_port, entry);
}

void lldp_send_frame(lldp_port_t port_idx, lldp_u8_t *frame, lldp_u16_t len)
{
    packet_tx_props_t tx_props;

    T_NG_PORT(TRACE_GRP_TX, (vtss_port_no_t)port_idx, "LLDP frame transmission start");

    if (!lldp_port_is_authorized(port_idx)) {
        return;
    }

    T_RG_PORT(TRACE_GRP_TX, (vtss_port_no_t) port_idx, "LLDP frame transmission: Port is authorized");

    // Allocate frame buffer. Exclude room for FCS.
#ifdef VTSS_SW_OPTION_PACKET
    uchar *frm_p;
    if ((frm_p = packet_tx_alloc(len)) == NULL) {
        T_W("LLDP packet_tx_alloc failed.");
        return;
    } else {
        if (len < 64) {
            // Do manual padding since FDMA isn't able to do it, and the FDMA requires minimum 64 bytes.
            memset(frm_p, 0, 64);
            memcpy(frm_p, frame, len);
            len = 64;
        } else {
            memcpy(frm_p, frame, len);
        }

    }

    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid     = VTSS_MODULE_ID_LLDP;
    tx_props.packet_info.frm[0]    = frm_p;
    tx_props.packet_info.len[0]    = len;
    tx_props.tx_info.dst_port_mask = VTSS_BIT64(port_idx);
    if (packet_tx(&tx_props) != VTSS_RC_OK) {
        T_E("LLDP frame was not transmitted correctly");
    } else {
        T_NG_PORT(TRACE_GRP_TX, port_idx, "LLDP frame transmistion done");
    }
#endif
}

// When the switch reboots, a LLDP shutdown frame must be transmitted. This function is call upon reboot
static void pre_shutdown(vtss_restart_t restart)
{
    LLDP_CRIT_ENTER();
    int port;
    // loop through all port and send a shutdown frame
    for (port = 0; port < LLDP_PORTS; port++) {
        lldp_pre_port_disabled(port);
    }
    LLDP_CRIT_EXIT();
    T_D("Going down..... ");
}

static void lldp_port_shutdown(vtss_port_no_t port_no)
{
    LLDP_CRIT_ENTER();
    lldp_pre_port_disabled(port_no - 1);
    LLDP_CRIT_EXIT();
    T_D("port_no %u, shut down", port_no);
}

// Callback function for when a port changes state.
static void lldp_port_link(vtss_port_no_t port_no, port_info_t *info)
{
    LLDP_CRIT_ENTER();
    lldp_set_port_enabled(port_no, info->link);
    LLDP_CRIT_EXIT();
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
vtss_rc lldp_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid; // Get switch id

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        // Initialize message buffers
        lldp_msg_request_pool = msg_buf_pool_create(VTSS_MODULE_ID_LLDP, "Request", VTSS_ISID_CNT + 1, sizeof(lldp_msg_req_t));
        lldp_msg_reply_pool   = msg_buf_pool_create(VTSS_MODULE_ID_LLDP, "Reply",   1, sizeof(lldp_msg_rep_t));

        // Initialize critical regions
        critd_init(&lldp_crit, "lldp_crit", VTSS_MODULE_ID_LLDP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        cyg_flag_init(&lldp_msg_id_get_stat_req_flags);
        cyg_flag_init(&lldp_msg_id_get_entries_req_flags);
        LLDP_CRIT_EXIT();

        // ICLI initialization
#ifdef VTSS_SW_OPTION_VCLI
        lldp_cli_req_init();
#ifdef VTSS_SW_OPTION_LLDP_MED
        lldpmed_cli_req_init();
#endif
#endif

        // Initialize ICLI "show running" configuration
#ifdef VTSS_SW_OPTION_ICFG
        // Initialize ICFG
        VTSS_RC(lldp_icfg_init());
#ifdef VTSS_SW_OPTION_LLDP_MED
        VTSS_RC(lldpmed_icfg_init());
#endif
#endif //VTSS_SW_OPTION_ICFG

        critd_init(&lldp_if_crit, "lldp_if_crit", VTSS_MODULE_ID_LLDP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        LLDP_IF_CRIT_EXIT();

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          lldp_thread,
                          0,
                          "LLDP",
                          lldp_thread_stack,
                          sizeof(lldp_thread_stack),
                          &lldp_thread_handle,
                          &lldp_thread_block);
        cyg_thread_resume(lldp_thread_handle);

        T_D("enter, cmd=INIT");
        break;

    case INIT_CMD_START:
        T_D("enter, cmd=START");
        lldp_start();
        control_system_reset_register(pre_shutdown);        // Prepare callback function for switch reboot
        (void)port_shutdown_register(VTSS_MODULE_ID_LLDP, lldp_port_shutdown);  // Prepare callback function for port disable
        (void)port_change_register(VTSS_MODULE_ID_LLDP, lldp_port_link);        // Prepare callback function for link up/down for ports
        LLDP_CRIT_ENTER();
        sw_lldp_init();
        LLDP_CRIT_EXIT();
        break;

    case INIT_CMD_CONF_DEF:
        T_D("enter, isid = %u", isid);
        if (isid == VTSS_ISID_GLOBAL) {
            /* Reset configuration for all switches */
            lldp_conf_read(1);
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_N("INIT_CMD_MASTER_UP");
        lldp_conf_read(0);
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        master_conf2slave_conf(isid);
        switch_add = LLDP_TRUE;
        break;

    case INIT_CMD_SWITCH_DEL:
        T_N("SWITCH_DEL, isid: %d", isid);
        break;

    default:
        break;
    }

    return VTSS_OK;
}
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
