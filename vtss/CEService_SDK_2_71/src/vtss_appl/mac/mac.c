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

#include "main.h"
#include "conf_api.h"
#include "critd_api.h"
#include "port_api.h"
#include "msg_api.h"
#include "mac_api.h"
#include "mac.h"
#include "packet_api.h"
#include "misc_api.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "mac_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICLI
#include "mac_icfg.h"
#endif
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

/*lint -sem(mac_local_learn_mode_set, thread_protected) Well, not really, but we live with it. */

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Thread variables */
static cyg_handle_t       mac_thread_handle;
static cyg_thread         mac_thread_block;
static char               mac_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_flag_t         age_flags, getnext_flags, stats_flags, mac_table_flags;

#define MACFLAG_ABORT_AGE                (1 << 0)
#define MACFLAG_START_AGE                (1 << 1)
#define MACFLAG_WAIT_GETNEXT_DONE        (1 << 2)
#define MACFLAG_WAIT_STATS_DONE          (1 << 3)
#define MACFLAG_WAIT_GETNEXTSTACK_DONE   (1 << 4)
#define MACFLAG_WAIT_DONE                (1 << 5)
#define MACFLAG_COULD_NOT_TX             (1 << 8)


/* Global buffer used for exhanging Master/Slave info   */
static mac_global_t              mac_config;

/* Static-Mac list for non-volatile configured mac-addresses - saved in flash  */
static mac_static_t       *mac_used, *mac_free, mac_list[MAC_ADDR_NON_VOLATILE_MAX];
/* Static-Mac list for volatile configured mac-addresses - not saved in flash */
static mac_static_t       *mac_used_vol, *mac_free_vol, mac_list_vol[MAC_ADDR_VOLATILE_MAX];

/* Temporary and stored age time */
static age_time_temp_t           age_time_temp;

/* Learn mode info - kept in flash  */
static mac_conf_learn_mode_t       mac_learn_mode;

/* Forced learning mode - only kept in memory */
static struct {
    BOOL enable[VTSS_ISID_END][VTSS_PORTS + 1];
    vtss_learn_mode_t learn_mode;
} learn_force;

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/* Local copy of the UPSID to ISID mapping */
static struct {
    vtss_vstax_upsid_t upsid[VTSS_ISID_END][2];
    vtss_port_no_t     port_no; /* First port on second UPSID */
} upsid_table;
static init_port_map_t mac_port_map_global[VTSS_ISID_END][VTSS_PORTS];
#endif /* VTSS_FEATURE_VSTAX_V2 */

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "mac",
    .descr     = "MAC"
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
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define MAC_CRIT_ENTER()     critd_enter(&mac_config.crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MAC_CRIT_EXIT()      critd_exit( &mac_config.crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MAC_RAM_CRIT_ENTER() critd_enter(&mac_config.ram_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MAC_RAM_CRIT_EXIT()  critd_exit( &mac_config.ram_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define MAC_CRIT_ENTER()     critd_enter(&mac_config.crit)
#define MAC_CRIT_EXIT()      critd_exit( &mac_config.crit)
#define MAC_RAM_CRIT_ENTER() critd_enter(&mac_config.ram_crit)
#define MAC_RAM_CRIT_EXIT()  critd_exit( &mac_config.ram_crit)
#endif /* VTSS_TRACE_ENABLED */

static vtss_rc mac_table_add(vtss_isid_t isid, mac_mgmt_addr_entry_t *entry);
static vtss_rc mac_table_del(vtss_isid_t isid, vtss_vid_mac_t *conf, BOOL vol);

/****************************************************************************/
/*  Chip API functions and various local functions                          */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
static vtss_rc age_time_set(mac_age_conf_t *conf)
{
    vtss_rc           rc;
    T_D("enter, agetime set:%u", conf->mac_age_time);
    // Set the age time in the chip
    rc = vtss_mac_table_age_time_set(NULL, conf->mac_age_time);
    // Keep the agetime in the API

    MAC_CRIT_ENTER();
    mac_config.conf = *conf;
    MAC_CRIT_EXIT();
    T_D("exit, agetime set:%u", conf->mac_age_time);
    return rc;
}

/****************************************************************************/
// Determine if mac configuration has changed
/****************************************************************************/
static int mac_age_changed(mac_age_conf_t *old, mac_age_conf_t *new)
{
    return (new->mac_age_time != old->mac_age_time);
}

/****************************************************************************/
/****************************************************************************/
static vtss_isid_t get_local_isid (void)
{
    vtss_isid_t      isid;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists(isid)) {
            continue;
        }
        if (msg_switch_is_local(isid)) {
            break;
        }
    }
    return isid;
}

/****************************************************************************/
// Temp aging thread
/****************************************************************************/
static void mac_thread(cyg_addrword_t data)
{
    age_time_temp_t local_temp;
    mac_age_conf_t age_tmp;

    T_D("Aging thread start");
    while (1) {
        while (msg_switch_is_master()) {
            T_D("Aging thread waiting for start signal");
            cyg_flag_value_t flags = cyg_flag_wait( &age_flags,
                                                    MACFLAG_START_AGE | MACFLAG_ABORT_AGE,
                                                    CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
            if (flags & MACFLAG_START_AGE) {
                /* Store current age time */
                if (mac_mgmt_age_time_get(&age_tmp) != VTSS_OK) {
                    T_E("Could not store age time");
                }
                MAC_CRIT_ENTER();
                /* Take a local copy */
                local_temp = age_time_temp;
                age_time_temp.stored_conf = age_tmp;
                MAC_CRIT_EXIT();
                /* Modify aging time in chip */
                if (age_time_set(&local_temp.temp_conf) != VTSS_OK) {
                    T_E("Could not set age time");
                }

                T_D("%llu: Changed age timer to %u sec. Will change it back after %u sec.",
                    cyg_current_time(), local_temp.temp_conf.mac_age_time, local_temp.age_period);

                /* Wait until timeout or aborted */
                while (local_temp.age_period > 0) {
                    flags = cyg_flag_timed_wait( &age_flags,
                                                 MACFLAG_ABORT_AGE,
                                                 CYG_FLAG_WAITMODE_OR,
                                                 cyg_current_time() + VTSS_OS_MSEC2TICK(1000));
                    if (flags & MACFLAG_ABORT_AGE) {
                        T_I("%llu: Aging aborted", cyg_current_time());
                        break;
                    } else {
                        local_temp.age_period--;
                    }
                }


                MAC_CRIT_ENTER();
                age_tmp = age_time_temp.stored_conf;
                MAC_CRIT_EXIT();
                /* Back to orginal aging value */
                if (age_time_set(&age_tmp) != VTSS_OK) {
                    T_E("Could not set age time");
                }
            }
        }
        T_I("Suspending MAC aging thread (became slave)");
        cyg_thread_suspend(mac_thread_handle);
        T_I("Restarting MAC aging thread (became master)");
    }

    /* NOTREACHED */
}

/****************************************************************************/
/****************************************************************************/
static void pr_entry(char name[100], mac_mgmt_addr_entry_t *entry)
{
    vtss_port_no_t port_no;
    BOOL first = 1;
    char dest[VTSS_PORTS * 3];

    memset(dest, 0, sizeof(dest));
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (entry->destination[port_no]) {
            sprintf(dest, "%s%s%u", dest, first ? "" : "-", port_no);
            first = 0;
        }
    }
    T_D("(%s:)Entry:%02x-%02x-%02x-%02x-%02x-%02x vid:%d Ports:%s", name,
        entry->vid_mac.mac.addr[0], entry->vid_mac.mac.addr[1], entry->vid_mac.mac.addr[2],
        entry->vid_mac.mac.addr[3], entry->vid_mac.mac.addr[4], entry->vid_mac.mac.addr[5], entry->vid_mac.vid, dest);
}

/****************************************************************************/
/****************************************************************************/
static BOOL mac_vid_compare(vtss_vid_mac_t *entry1, vtss_vid_mac_t *entry2)
{
    if (entry1->vid != MAC_ALL_VLANS && entry2->vid != MAC_ALL_VLANS) {
        if (entry1->vid != entry2->vid) {
            return 0;
        }
    }

    return memcmp(entry1->mac.addr, entry2->mac.addr, 6) == 0;
}

/****************************************************************************/
// Larger = 1, smaller = -1, equal = 0
/****************************************************************************/
static int vid_mac_cmp(vtss_vid_mac_t *entry1, vtss_vid_mac_t *entry2)
{
    int i;

    if (entry1->vid > entry2->vid) {
        return 1;
    } else if (entry1->vid < entry2->vid) {
        return -1;
    }

    for (i = 0; i < 6; i++) {
        if (entry1->mac.addr[i] > entry2->mac.addr[i]) {
            return 1;
        } else if (entry1->mac.addr[i] < entry2->mac.addr[i]) {
            return -1;
        }
    }
    return 0;
}

/****************************************************************************/
/****************************************************************************/
static bool vid_mac_bigger(vtss_vid_mac_t *entry1, vtss_vid_mac_t *entry2)
{
    return ((vid_mac_cmp(entry1, entry2) == 1) ? 1 : 0);
}

/****************************************************************************/
// Add MAC address via switch API
/****************************************************************************/
static vtss_rc mac_entry_add(mac_mgmt_addr_entry_t *entry
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
                             , vtss_vstax_upsid_t upsid, vtss_vstax_upspn_t upspn
#endif
                            )
{
    vtss_mac_table_entry_t mac_entry;
    vtss_rc                rc;
    vtss_port_no_t         port_no;
    u32                    port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    /* Convert the mac mgmt data type (smaller) to the Switch API data type (larger)*/
    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.copy_to_cpu = entry->copy_to_cpu;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
    mac_entry.cpu_queue = PACKET_XTR_QU_MAC;
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
    mac_entry.locked = 1;
    mac_entry.vid_mac = entry->vid_mac;
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        mac_entry.destination[port_no] = entry->destination[port_no];
    }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    mac_entry.vstax2.enable = 1;
    mac_entry.vstax2.upspn = upspn;
    mac_entry.vstax2.upsid = upsid;
    /* Add the entry through the switch API */
    T_I("Add %s, upspn:%d upsid:%d through chip API", misc_mac2str(mac_entry.vid_mac.mac.addr), upspn, upsid);
#endif

    rc = vtss_mac_table_add(NULL, &mac_entry);

    /*  Add the address to the  local table    */
    if (!msg_switch_is_master()) {

        (void)(mac_table_add(VTSS_ISID_LOCAL, entry));
    }

    /* Notify the mac memory thread */
    cyg_flag_setbits(&mac_table_flags, 1);
    return rc;
}

/****************************************************************************/
// Delete MAC address via switch API
/****************************************************************************/
static vtss_rc mac_entry_del(vtss_vid_mac_t *vid_mac, BOOL vol)
{
    vtss_rc                rc;
    vtss_vid_mac_t         vid_mac_tmp;
    vtss_mac_table_entry_t entry;

    T_N("Deleting mac entry through API:%s", misc_mac2str(vid_mac->mac.addr));

    if (vid_mac->vid == MAC_ALL_VLANS) {
        /* ignore Vlans */
        memset(&vid_mac_tmp, 0, sizeof(vid_mac_tmp));
        while (1) {
            rc = vtss_mac_table_get_next(NULL, &vid_mac_tmp, &entry);
            if (rc != VTSS_OK) {
                break;
            }

            if (mac_vid_compare(&entry.vid_mac, vid_mac)) {
                rc = vtss_mac_table_del(NULL, &entry.vid_mac);
                if (rc != VTSS_OK) {
                    T_N("(mac_msg_rx)Could not del entry,rc:%d", rc);
                }
            }

            vid_mac_tmp = entry.vid_mac;
        }
    } else {
        rc = vtss_mac_table_del(NULL, vid_mac);
        if (rc != VTSS_OK) {
            T_N("(mac_msg_rx)Could not del entry,rc:%d", rc);
        }
    }


    /*  Delete the address from the local table    */
    if (!msg_switch_is_master()) {

        if (mac_table_del(VTSS_ISID_LOCAL, vid_mac, vol) != VTSS_OK) {
            T_E("Could not add entry");
        }
    }

    /* Notify the mac memory thread */
    cyg_flag_setbits(&mac_table_flags, 1);

    return VTSS_OK;
}

/****************************************************************************/
/****************************************************************************/
static void build_mac_list(void)
{
    mac_static_t     *mac;
    int              i;

    /* Build list of free static MACs */
    MAC_CRIT_ENTER();
    mac_free = NULL;
    for (i = 0; i < MAC_ADDR_NON_VOLATILE_MAX; i++) {
        mac = &mac_list[i];
        mac->next = mac_free;
        mac_free = mac;
    }
    mac_used = NULL;

    /* Build list of free volatile MACs */

    mac_free_vol = NULL;
    for (i = 0; i < MAC_ADDR_VOLATILE_MAX; i++) {
        mac = &mac_list_vol[i];
        mac->next = mac_free_vol;
        mac_free_vol = mac;
    }
    mac_used_vol = NULL;
    MAC_CRIT_EXIT();
}

/****************************************************************************/
// Get the next MAC address on the local switch
/****************************************************************************/
static vtss_rc mac_local_table_get_next(vtss_vid_mac_t *vid_mac,
                                        vtss_mac_table_entry_t *entry, BOOL next)
{
    vtss_rc rc;
    vtss_vid_mac_t         vid_mac_tmp;

    if (next) {
        rc = vtss_mac_table_get_next(NULL, vid_mac, entry);
    } else {
        if (vid_mac->vid == MAC_ALL_VLANS) {
            memset(&vid_mac_tmp, 0, sizeof(vid_mac_tmp));

            while (1) {
                rc = vtss_mac_table_get_next(NULL, &vid_mac_tmp, entry);
                if (rc != VTSS_OK) {
                    break;
                }
                if (mac_vid_compare(&entry->vid_mac, vid_mac)) {
                    return rc;
                }
                vid_mac_tmp = entry->vid_mac;
            }


        } else {
            rc = vtss_mac_table_get(NULL, vid_mac, entry);
        }
    }
    T_N("Return: vid: %d, mac: %02x-%02x-%02x-%02x-%02x-%02x Next:%d, rc:%d\n",
        entry->vid_mac.vid,
        entry->vid_mac.mac.addr[0], entry->vid_mac.mac.addr[1], entry->vid_mac.mac.addr[2],
        entry->vid_mac.mac.addr[3], entry->vid_mac.mac.addr[4], entry->vid_mac.mac.addr[5], next, rc);
    return rc;
}

#if defined VTSS_SW_OPTION_PSEC && defined VTSS_FEATURE_QOS_POLICER_CPU_SWITCH
/****************************************************************************/
/****************************************************************************/
static void mac_set_learn_policer(BOOL enabled, vtss_port_no_t port_no)
{
    vtss_qos_conf_t qos;

    T_N("%s learn policer rate (due to port change on port %d)", enabled ? "Enabling" : "Disabling", port_no);

    // Change the current mode.
    if (vtss_qos_conf_get(NULL, &qos) == VTSS_OK) {
        qos.policer_learn = enabled ? PACKET_XTR_POLICER_RATE : VTSS_PACKET_RATE_DISABLED;
        if (vtss_qos_conf_set(NULL, &qos) != VTSS_OK) {
            T_W("Could not set policer learn mode");
        }

    }
}
#endif /* VTSS_SW_OPTION_PSEC && defined VTSS_FEATURE_QOS_POLICER_CPU_SWITCH */

/****************************************************************************/
/****************************************************************************/
static vtss_rc mac_local_learn_mode_set(vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode)
{
    vtss_rc rc;
#ifdef VTSS_SW_OPTION_PSEC
    // When the Port Security module is enabled, we need to enable learn policers
    // when at least one port is in secure learning with CPU copy. If we didn't
    // we might lose stack traffic due to a chip-bug in EStaX-34.

    // Static variables are initialized to all-zeros at boot.
    static BOOL  qos_learn_policer_enabled_per_port[VTSS_PORTS + 1];
    static uchar qos_learn_policer_currently_enabled;
    uchar at_least_one_still_enabled = 0; // Using uchar instead of BOOL, so that we don't rely on actual values of TRUE and FALSE, since we're xor-ing below
    BOOL do_change_settings = 0;
    vtss_port_no_t p;
    u32     port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    // Enabled for a given port when secure learning and copying to CPU.
    qos_learn_policer_enabled_per_port[port_no] = learn_mode->automatic == 0 && learn_mode->discard == 1 && learn_mode->cpu == 1;

    for (p = VTSS_PORT_NO_START; p < port_count; p++) {
        if (qos_learn_policer_enabled_per_port[p]) {
            at_least_one_still_enabled = 1;
            break;
        }
    }

    do_change_settings = (at_least_one_still_enabled ^ qos_learn_policer_currently_enabled) == 1;
    qos_learn_policer_currently_enabled = at_least_one_still_enabled;
    if (do_change_settings && at_least_one_still_enabled) {
        // Enable learn policer *BEFORE* setting the learn mode below, so that it is enabled
        // already before we allow learn frames to the CPU.
#if defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH)
        mac_set_learn_policer(at_least_one_still_enabled, port_no);
#endif /* VTSS_FEATURE_QOS_POLICER_CPU_SWITCH */
    }
#endif

    rc = vtss_learn_port_mode_set(NULL, port_no,  learn_mode);
    T_D("Local. Port:%d auto:%d, cpu:%d, discard:%d", port_no, learn_mode->automatic, learn_mode->cpu, learn_mode->discard);

#if defined VTSS_SW_OPTION_PSEC && defined VTSS_FEATURE_QOS_POLICER_CPU_SWITCH
    if (do_change_settings && !at_least_one_still_enabled) {
        // Disable learn policer *AFTER* setting the learn mode above, so that
        // we're sure that learn frames to CPU is disabled before we stop policing them.
        mac_set_learn_policer(at_least_one_still_enabled, port_no);
    }
#endif /* VTSS_SW_OPTION_PSEC && defined VTSS_FEATURE_QOS_POLICER_CPU_SWITCH */

    return rc;
}

/****************************************************************************/
// Get MAC address statistics
/****************************************************************************/
static vtss_rc mac_local_table_get_stats(mac_table_stats_t *stats)
{
    int                    i;
    vtss_vid_mac_t         vid_mac;
    vtss_mac_table_entry_t mac_entry;
    vtss_port_no_t         port_no;
    vtss_rc                rc;
    BOOL                   next = 1;
    u32                    port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    T_N("enter mac_local_table_get_stats");

    // initial search address is 00-00-00-00-00-00, VID: 1
    vid_mac.vid = 1;
    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = 0;
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        stats->learned[port_no] = 0;
    }
    stats->static_total = 0;
    stats->learned_total = 0;;

    // Run through all existing entries and gather stats
    for (;;) {
        rc = mac_local_table_get_next(&vid_mac, &mac_entry, next);
        if (rc != VTSS_OK) {
            break;
        }

        // Search again from the found entry
        vid_mac = mac_entry.vid_mac;

#if defined(VTSS_FEATURE_VSTAX_V2)
        if (mac_entry.vstax2.remote_entry) {
            continue;
        }
#endif

        if (mac_entry.locked) {
            stats->static_total++;
        } else {
            stats->learned_total++;
            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (mac_entry.destination[port_no]) {
                    stats->learned[port_no]++;
                    /* Count only for the 1.first port (to avoid counting aggregation destinations) */
                    break;
                }
            }
        }

        if (stats->static_total + stats->learned_total > MAC_LEARN_MAX) {
            break;
        }

    }

    T_N("exit mac_local_table_get_stats");
    return VTSS_OK;
}

/****************************************************************************/
// Get MAC address VLAN statistics
/****************************************************************************/
static vtss_rc mac_local_table_get_vlan_stats(vtss_vid_t vlan, mac_table_stats_t *stats)
{

    int                    i;
    vtss_vid_mac_t         vid_mac;
    vtss_mac_table_entry_t mac_entry;
    vtss_port_no_t         port_no;
    vtss_rc                rc;
    BOOL                   next = 1;
    u32                    port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    T_N("enter mac_local_table_get_vlan_stats");

    // initial search address is 00-00-00-00-00-00, VID: 1
    vid_mac.vid = vlan;
    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = 0;
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        stats->learned[port_no] = 0;
    }
    stats->static_total = 0;
    stats->learned_total = 0;;

    // Run through all existing entries and gather stats
    for (;;) {
        rc = mac_local_table_get_next(&vid_mac, &mac_entry, next);
        if (rc != VTSS_OK || (vid_mac.vid != vlan)) {
            break;
        }

        if (mac_entry.locked) {
            stats->static_total++;
        } else {
            stats->learned_total++;
            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (mac_entry.destination[port_no]) {
                    stats->learned[port_no]++;
                    /* Count only for the 1.first port (to avoid counting aggregation destinations) */
                    break;
                }
            }
        }

        if (stats->static_total + stats->learned_total > MAC_LEARN_MAX) {
            break;
        }

        // Search again from the found entry
        vid_mac = mac_entry.vid_mac;
    }

    T_N("exit mac_local_table_get_vlan_stats");
    return VTSS_OK;
}

/****************************************************************************/
// Flush the RAM address table
/****************************************************************************/
static vtss_rc mac_local_del_locked(void)
{

    mac_static_t           *mac;

    if (msg_switch_is_master()) {
        return VTSS_OK;
    }

    T_N("enter mac_local_table_del_locked");

    MAC_CRIT_ENTER();
    /*  Volatile      */
    for (mac = mac_used_vol; mac != NULL; mac = mac->next) {
        T_N("Deleting mac's:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
            mac->conf.vid_mac.mac.addr[0], mac->conf.vid_mac.mac.addr[1], mac->conf.vid_mac.mac.addr[2],
            mac->conf.vid_mac.mac.addr[3], mac->conf.vid_mac.mac.addr[4], mac->conf.vid_mac.mac.addr[5], mac->conf.vid_mac.vid);

        (void)vtss_mac_table_del(NULL, &mac->conf.vid_mac);
    }

    /*  .. and non Volatile      */
    for (mac = mac_used; mac != NULL; mac = mac->next) {
        T_N("Deleting mac's:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
            mac->conf.vid_mac.mac.addr[0], mac->conf.vid_mac.mac.addr[1], mac->conf.vid_mac.mac.addr[2],
            mac->conf.vid_mac.mac.addr[3], mac->conf.vid_mac.mac.addr[4], mac->conf.vid_mac.mac.addr[5], mac->conf.vid_mac.vid);

        (void)vtss_mac_table_del(NULL, &mac->conf.vid_mac);
    }
    MAC_CRIT_EXIT();

    /* Reset the linked MAC list */
    build_mac_list();

    T_N("exit mac_local_table_del_locked");
    return VTSS_OK;
}

/****************************************************************************/
// Flush all static addresses we know of from chip
/****************************************************************************/
static void flush_mac_list(BOOL force)
{
    mac_static_t           *mac;

    /* Flush the Master MAC table   */
    if (msg_switch_is_master() || force) {

        /*  Volatile      */
        for (mac = mac_used_vol; mac != NULL; mac = mac->next) {
            (void)mac_entry_del(&mac->conf.vid_mac, 1);
        }

        /*  .. and non Volatile      */
        for (mac = mac_used; mac != NULL; mac = mac->next) {
            (void)mac_entry_del(&mac->conf.vid_mac, 0);
        }
    }
}

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
static char *mac_msg_id_txt(mac_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case MAC_MSG_ID_AGE_SET_REQ:
        txt = "MAC_MSG_ID_AGE_SET_REQ";
        break;
    case MAC_MSG_ID_GET_NEXT_REQ:
        txt = "MAC_MSG_ID_GET_NEXT_REQ";
        break;
    case MAC_MSG_ID_GET_NEXT_REP:
        txt = "MAC_MSG_ID_GET_NEXT_REP";
        break;
    case MAC_MSG_ID_GET_NEXT_STACK_REQ:
        txt = "MAC_MSG_ID_GET_NEXT_STACK_REQ";
        break;
    case MAC_MSG_ID_GET_STATS_REQ:
        txt = "MAC_MSG_ID_GET_STATS_REQ";
        break;
    case MAC_MSG_ID_GET_STATS_REP:
        txt = "MAC_MSG_ID_GET_STATS_REP";
        break;
    case MAC_MSG_ID_ADD_REQ:
        txt = "MAC_MSG_ID_ADD_REQ";
        break;
    case MAC_MSG_ID_DEL_REQ:
        txt = "MAC_MSG_ID_DEL_REQ";
        break;
    case MAC_MSG_ID_LEARN_REQ:
        txt = "MAC_MSG_ID_LEARN_REQ";
        break;
    case MAC_MSG_ID_FLUSH_REQ:
        txt = "MAC_MSG_ID_FLUSH_REQ";
        break;
    case MAC_MSG_ID_LOCKED_DEL_REQ:
        txt = "MAC_MSG_ID_LOCKED_DEL_REQ";
        break;
    case MAC_MSG_ID_UPSID_FLUSH_REQ:
        txt = "MAC_MSG_ID_UPSID_FLUSH_REQ";
        break;
    case MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ:
        txt = "MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}

/****************************************************************************/
// Allocate request/reply buffer
/****************************************************************************/
static mac_msg_req_t *mac_msg_req_alloc(mac_msg_id_t msg_id, u32 ref_cnt)
{
    mac_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(mac_config.request);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }

    msg->msg_id = msg_id;
    return msg;
}

/****************************************************************************/
/****************************************************************************/
static mac_msg_rep_t *mac_msg_rep_alloc(mac_msg_id_t msg_id)
{
    mac_msg_rep_t *msg = msg_buf_pool_get(mac_config.reply);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/****************************************************************************/
// Release mac message buffer
/****************************************************************************/
static void mac_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    mac_msg_id_t msg_id = *(mac_msg_id_t *)msg;
    (void)msg_buf_pool_put(msg);

    if (rc != MSG_TX_RC_OK) {
        T_D("Could not transmit %s. Got return code %d from msg module", mac_msg_id_txt(msg_id), rc);

        /* Release the sync flag */
        if (msg_id == MAC_MSG_ID_GET_NEXT_REQ) {
            cyg_flag_setbits(&getnext_flags, MACFLAG_COULD_NOT_TX);
        } else if (msg_id == MAC_MSG_ID_LEARN_REQ) {
            cyg_flag_setbits(&stats_flags, MACFLAG_COULD_NOT_TX);
        }
    }
}

/****************************************************************************/
// Send mac message
/****************************************************************************/
static void mac_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    mac_msg_id_t msg_id = *(mac_msg_id_t *)msg;

    T_N("Tx: %s, len: %zd, isid: %d", mac_msg_id_txt(msg_id), len, isid);
    // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of MSG_TX_DATA_HDR_LEN_MAX() below
    /*lint -e(506) */
    msg_tx_adv(NULL, mac_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_MAC, isid, msg, len + MSG_TX_DATA_HDR_LEN_MAX(mac_msg_req_t, req, mac_msg_rep_t, rep));
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc mac_do_set_local_learn_mode(vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode)
{
    vtss_rc rc = VTSS_OK;
    u32     port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    if (port_no == MAC_ALL_PORTS) {
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (port_no_is_stack(port_no)) {
                continue;
            }
            rc = mac_local_learn_mode_set(port_no, learn_mode);
        }
    } else {
        rc = mac_local_learn_mode_set(port_no, learn_mode);
    }

    return rc;
}

/****************************************************************************/
/****************************************************************************/
static BOOL mac_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    mac_msg_id_t   msg_id  = *(mac_msg_id_t *)rx_msg;

    T_N("Rx: %s, len: %zd, isid: %u", mac_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case MAC_MSG_ID_AGE_SET_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;

        T_D("Got message: change aging to %u", req_msg->req.age_set.conf.mac_age_time);
        if (age_time_set(&req_msg->req.age_set.conf) != VTSS_OK) {
            T_E("Could not set time");
        }
        break;
    }

    case MAC_MSG_ID_GET_NEXT_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;
        mac_msg_rep_t *rep_msg = mac_msg_rep_alloc(MAC_MSG_ID_GET_NEXT_REP);

        /* Do a local Get Next */
        rep_msg->rep.get_next.rc = mac_local_table_get_next(&req_msg->req.get_next.vid_mac, &rep_msg->rep.get_next.entry, req_msg->req.get_next.next);

        T_N("Return mac entry to Master:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
            rep_msg->rep.get_next.entry.vid_mac.mac.addr[0], rep_msg->rep.get_next.entry.vid_mac.mac.addr[1], rep_msg->rep.get_next.entry.vid_mac.mac.addr[2],
            rep_msg->rep.get_next.entry.vid_mac.mac.addr[3], rep_msg->rep.get_next.entry.vid_mac.mac.addr[4], rep_msg->rep.get_next.entry.vid_mac.mac.addr[5],
            rep_msg->rep.get_next.entry.vid_mac.vid);

        /* Transmit the get_next reply */
        mac_msg_tx(rep_msg, isid, sizeof(rep_msg->rep.get_next));
        break;
    }

    case MAC_MSG_ID_GET_NEXT_REP: {
        mac_msg_rep_t *rep_msg = (mac_msg_rep_t *)rx_msg;

        MAC_CRIT_ENTER();
        /* Store the info in the global struct */
        mac_config.get_next.entry = rep_msg->rep.get_next.entry;
        mac_config.get_next.rc    = rep_msg->rep.get_next.rc;
        MAC_CRIT_EXIT();

        /* Release the sync flag */
        cyg_flag_setbits(&getnext_flags, MACFLAG_WAIT_GETNEXT_DONE);
        break;
    }

    case MAC_MSG_ID_GET_STATS_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;
        mac_msg_rep_t *rep_msg = mac_msg_rep_alloc(MAC_MSG_ID_GET_STATS_REP);

        /* Do a local Get Stats */
        if (req_msg->req.stats.vlan > 0) {
            rep_msg->rep.stats.rc = mac_local_table_get_vlan_stats(req_msg->req.stats.vlan, &rep_msg->rep.stats.stats);
        } else {
            rep_msg->rep.stats.rc = mac_local_table_get_stats(&rep_msg->rep.stats.stats);
        }

        /* Transmit the get_next reply */
        mac_msg_tx(rep_msg, isid, sizeof(rep_msg->rep.stats));
        break;
    }

    case MAC_MSG_ID_GET_STATS_REP: {
        mac_msg_rep_t *rep_msg = (mac_msg_rep_t *)rx_msg;

        /* Store the info in the global struct */
        MAC_CRIT_ENTER();
        mac_config.get_stats.stats = rep_msg->rep.stats.stats;
        mac_config.get_stats.rc    = rep_msg->rep.stats.rc;
        MAC_CRIT_EXIT();

        /* Release the sync flag */
        cyg_flag_setbits(&stats_flags, MACFLAG_WAIT_STATS_DONE);
        break;
    }

    case MAC_MSG_ID_ADD_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;
        vtss_rc       rc;

        /* Do a local add mac */
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        if ((rc = mac_entry_add(&req_msg->req.add.entry, req_msg->req.add.upsid, req_msg->req.add.upspn)) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not add entry, rc = %d", rc);
        }
#else
        if ((rc = mac_entry_add(&req_msg->req.add.entry)) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not add entry, rc = %d", rc);
        }
#endif
        break;
    }

    case MAC_MSG_ID_DEL_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;
        vtss_rc       rc;

        /* Do a local del mac */
        if ((rc = mac_entry_del(&req_msg->req.del.vid_mac, req_msg->req.del.vol)) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not del entry, rc = %d", rc);
        }
        break;
    }

    case MAC_MSG_ID_LEARN_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;
        vtss_rc       rc;

        /* Do a local learnmode */
        if ((rc = mac_do_set_local_learn_mode(req_msg->req.learn.port_no, &req_msg->req.learn.learn_mode)) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could set learn mode, rc = %d", rc);
        }

        break;
    }

    case MAC_MSG_ID_FLUSH_REQ: {
        /* Do a local flush mac */
        if (vtss_mac_table_flush(NULL) != VTSS_OK) {
            T_W("Could not flush");
        }
        break;
    }

    case MAC_MSG_ID_LOCKED_DEL_REQ: {
        T_D("Got message: Remove all static addresses");
        if (mac_local_del_locked() != VTSS_OK) {
            T_E("Could not delete addresses");
        }
        break;
    }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    case MAC_MSG_ID_UPSID_FLUSH_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;

        /* Do a API local upsid flush  */
        if (vtss_mac_table_upsid_flush(NULL, req_msg->req.upsid_flush.upsid) != VTSS_OK) {
            T_W("Could not flush");
        }
        break;
    }

    case MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ: {
        mac_msg_req_t *req_msg = (mac_msg_req_t *)rx_msg;

        /* Do a local flush upsid/upspn */
        if (vtss_mac_table_upsid_upspn_flush(NULL, req_msg->req.upspn_flush.upsid, req_msg->req.upspn_flush.upspn) != VTSS_OK) {
            T_W("Could not flush");
        }
        break;
    }

#endif /* VTSS_FEATURE_VSTAX_V2 */

    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc mac_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = mac_msg_rx;
    filter.modid = VTSS_MODULE_ID_MAC;
    return msg_rx_filter_register(&filter);
}

/****************************************************************************/
// Remove any old static mac addresses
/****************************************************************************/
static vtss_rc mac_remove_locked(vtss_isid_t isid)
{
    mac_msg_req_t *msg;

    T_D("Enter mac_remove_locked isid:%d", isid);
    if (msg_switch_exists(isid)) {
        msg = mac_msg_req_alloc(MAC_MSG_ID_LOCKED_DEL_REQ, 1);
        mac_msg_tx(msg, isid, 0);
    }
    return VTSS_OK;
}

/****************************************************************************/
// Set mac age configuration
/****************************************************************************/
static vtss_rc mac_age_stack_conf_set(vtss_isid_t isid)
{
    mac_msg_req_t *msg;

    if (msg_switch_exists(isid)) {
        /* Set MAC aging  */
        T_D("isid: %d. set age_time to %u", isid, mac_config.conf.mac_age_time);

        msg = mac_msg_req_alloc(MAC_MSG_ID_AGE_SET_REQ, 1);

        /* Use the stored value in case of the sprout is doing a temp aging */
        MAC_CRIT_ENTER();
        msg->req.age_set.conf = age_time_temp.stored_conf;
        MAC_CRIT_EXIT();

        mac_msg_tx(msg, isid, sizeof(msg->req.age_set));
    }
    return VTSS_OK;
}

/****************************************************************************/
/****************************************************************************/
static BOOL mac_mgmt_port_sid_invalid(vtss_isid_t isid, vtss_port_no_t port_no, BOOL set, BOOL check_port)
{
    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        T_D("not master");
        return 1;
    }

    if (check_port && port_isid_port_no_is_stack(isid, port_no)) {
        T_D("port_no: %u is stack port ", port_no);
        return 1;
    }

    if (port_no >= port_isid_port_count(isid)) {
        T_D("illegal port_no: %u", port_no);
        return 1;
    }

    if (isid >= VTSS_ISID_END) {
        T_D("illegal isid: %d", isid);
        return 1;
    }

    if (set && isid == VTSS_ISID_LOCAL) {
        T_D("SET not allowed, isid: %d", isid);
        return 1;
    }

    return 0;
}

/****************************************************************************/
// Set the learn mode to a switch in the stack
/****************************************************************************/
static vtss_rc mac_stack_learn_mode_set(vtss_isid_t isid, vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode)
{
    mac_msg_req_t *msg;
    vtss_rc       rc = VTSS_RC_OK;

    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

    if (msg_switch_exists(isid)) {
        if (msg_switch_is_local(isid)) {
            // Bypass the message module for fast setting of learn mode (needed by DOT1X module).
            rc = mac_do_set_local_learn_mode(port_no, learn_mode);
        } else {
            msg = mac_msg_req_alloc(MAC_MSG_ID_LEARN_REQ, 1);
            msg->req.learn.port_no = port_no;
            msg->req.learn.learn_mode = *learn_mode;
            mac_msg_tx(msg, isid, sizeof(msg->req.learn));
        }
    }
    return rc;
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
static void mac_add_stack(vtss_isid_t isid, mac_mgmt_addr_entry_t *entry)
{
    mac_msg_req_t      *msg;
    switch_iter_t      sit;
    vtss_port_no_t     p;
    vtss_vstax_upsid_t upsid = VTSS_VSTAX_UPSID_UNDEF;
    vtss_vstax_upspn_t upspn = VTSS_UPSPN_NONE;
    u32                port_count = port_isid_port_count(isid);

    MAC_CRIT_ENTER();
    for (p = VTSS_PORT_NO_START; p < port_count; p++) {
        if (entry->destination[p]) {
            upspn = mac_port_map_global[isid][p].chip_port;
            upsid = upsid_table.upsid[isid][mac_port_map_global[isid][p].chip_no];
            break;
        }
    }
    MAC_CRIT_EXIT();
    if (upsid == VTSS_VSTAX_UPSID_UNDEF) {
        return;
    }
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        msg = mac_msg_req_alloc(MAC_MSG_ID_ADD_REQ, 1);
        msg->req.add.upsid = upsid;
        msg->req.add.upspn = upspn;
        msg->req.add.entry = *entry;
        T_I("Add through stack: %s sit.isid:%d upsid:%d upspn:%d",
            misc_mac2str(entry->vid_mac.mac.addr), sit.isid, msg->req.add.upsid, msg->req.add.upspn);
        mac_msg_tx(msg, sit.isid, sizeof(msg->req.add));
    }

    pr_entry("mac_add_stack", entry);
    T_D("Exit mac_add_stack");
}

#else
/****************************************************************************/
/* Add a mac entry to a switch (isid_add) in the stack */
/****************************************************************************/
static void mac_add_stack(vtss_isid_t isid_add, mac_mgmt_addr_entry_t *entry)
{
    mac_msg_req_t         *msg;
    int                   port_no;
    mac_mgmt_addr_entry_t stack_entry, local_entry, *tx_entry, mac_entry;
    switch_iter_t         sit;

    /*  Add the address to all stack port in the whole stack */
    stack_entry = *entry;
    local_entry = *entry;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (port_isid_port_no_is_stack(isid_add, port_no)) {
            local_entry.destination[port_no] = 1;
            stack_entry.destination[port_no] = 1;
        } else {
            stack_entry.destination[port_no] = 0;
        }
    }

    /* Run through the switch list and add the addresses to the front ports + the stack ports
      or just the stack ports  */

    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (!entry->not_stack_ports) {
            if (sit.isid == isid_add) {
                /* Add entry to the front ports and stack ports  */
                tx_entry = &local_entry;
                T_D("Adding address to front ports at isid:%d\n", sit.isid);
            } else if (mac_mgmt_static_get_next(sit.isid, &entry->vid_mac, &mac_entry, 0, entry->volatil) != VTSS_OK) {
                for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                    stack_entry.destination[port_no] = port_isid_port_no_is_stack(sit.isid, port_no) ? 1 : 0;
                }
                /* Add entry only to stack ports  */
                tx_entry = &stack_entry;
                T_D("Adding address to stack ports at isid:%d\n", sit.isid);
            } else {
                /* Entry exist at this isid. Skip it. */
                T_D(" Entry exist. Skip isid:%d\n", sit.isid);
                continue;
            }
        } else {
            /* Do not include stack ports or other ISIDs */
            if (sit.isid != isid_add) {
                continue;
            }

            tx_entry = entry;
        }
        pr_entry("mac_add_stack", tx_entry);
        msg = mac_msg_req_alloc(MAC_MSG_ID_ADD_REQ, 1);
        msg->req.add.entry = *tx_entry;
        mac_msg_tx(msg, sit.isid, sizeof(msg->req.add));
    }
    T_D("Exit mac_add_stack");
}
#endif

/****************************************************************************/
// Delete a mac entry from a switch (isid_del) in the stack
/****************************************************************************/
static vtss_rc mac_del_stack(vtss_isid_t isid_del, vtss_vid_mac_t *entry, BOOL vol)
{
    BOOL                  mac_exists_in_stack = 0;
    int                   port_no;
    mac_mgmt_addr_entry_t del_entry, mac_entry;
    switch_iter_t         sit;

    /* Run through the switch list and check if the address
       is known on other frontports in the stack*/

    if (mac_mgmt_port_sid_invalid(isid_del, VTSS_PORT_NO_START, 1, FALSE)) {
        return MAC_ERROR_GEN;
    }

    memset(&del_entry, 0, sizeof(mac_mgmt_addr_entry_t));
    memset(&mac_entry, 0, sizeof(mac_mgmt_addr_entry_t));

    // Skip this part for volatile entries.
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit) && !vol) {
        if (sit.isid != isid_del) {
            if (mac_mgmt_static_get_next(sit.isid, entry, &mac_entry, 0, vol) == VTSS_OK) {
                mac_exists_in_stack = 1;
                break;
            }
        }
    }

    if (mac_exists_in_stack) {
        mac_msg_req_t *msg;

        if (!msg_switch_exists(isid_del)) {
            return VTSS_OK;
        }

        T_D("Only deleting from isid %d\n", isid_del);

        /* Mac also exists elsewhere in the stack. Only delete from isid's frontports */
        del_entry.vid_mac = *entry;
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            del_entry.destination[port_no] = 0;
        }
#if VTSS_SWITCH_STACKABLE
        del_entry.destination[PORT_NO_STACK_0] = 1;
        del_entry.destination[PORT_NO_STACK_1] = 1;
#endif /* VTSS_SWITCH_STACKABLE */
        msg = mac_msg_req_alloc(MAC_MSG_ID_ADD_REQ, 1); /* Using Add to modify portmask */
        msg->req.add.entry = del_entry;
        mac_msg_tx(msg, isid_del, sizeof(msg->req.add));
    } else {
        mac_msg_req_t *msg;

        /* Mac only exists at ISIDs frontports. Delete from all switches */
        T_D("Mac only exists at isid:%d frontports. Deleting from whole stack\n", isid_del);

        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);

        /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
        if ((msg = mac_msg_req_alloc(MAC_MSG_ID_DEL_REQ, sit.remaining)) != NULL) {
            msg->req.del.vid_mac = *entry;
            msg->req.del.vol = vol;

            while (switch_iter_getnext(&sit)) {
                mac_msg_tx(msg, sit.isid, sizeof(msg->req.del));
            }
        }
    }
    return VTSS_OK;
}

/****************************************************************************/
// Set the learn mode to the new switch in the stack
/****************************************************************************/
static void mac_learnmode_switch_add(vtss_isid_t isid)
{
    vtss_learn_mode_t mode;
    vtss_learn_mode_t mode_cmp;
    vtss_port_no_t    port_no;
    u32               port_count = port_isid_port_count(isid);
    BOOL              lf;

    /* Need to convert to the larger 'vtss_learn_mode_t' to chip api struct */
    MAC_CRIT_ENTER();
    mode.automatic = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][1], LEARN_AUTOMATIC);
    mode.cpu = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][1], LEARN_CPU);
    mode.discard = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][1], LEARN_DISCARD);
    MAC_CRIT_EXIT();

    /* Most of the time all ports have the same learning mode */
    /* To save time, all ports are set to port 1's mode and then only  ports that differs from this mode are changed */
    if (mac_stack_learn_mode_set(isid, MAC_ALL_PORTS, &mode) != VTSS_OK) {
        T_W("Could not set stack learn mode\n");
    }


    /* Check if any port have a different value than already configured with  */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        MAC_CRIT_ENTER();
        lf = learn_force.enable[isid][port_no];
        MAC_CRIT_EXIT();
        if (lf) {
            mode_cmp.automatic = 0;
            mode_cmp.cpu = 1;
            mode_cmp.discard = 1;
            if ((mac_stack_learn_mode_set(isid, port_no, &mode_cmp)) != VTSS_OK) {
                T_W("Could not set stack learn mode\n");
            }
        } else {
            MAC_CRIT_ENTER();
            mode_cmp.automatic = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_AUTOMATIC);
            mode_cmp.cpu = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_CPU);
            mode_cmp.discard = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_DISCARD);
            MAC_CRIT_EXIT();
            if (memcmp(&mode, &mode_cmp, sizeof(mode)) != 0) {
                /* Configure this port */
                if ((mac_stack_learn_mode_set(isid, port_no, &mode_cmp)) != VTSS_OK) {
                    T_W("Could not set stack learn mode\n");
                }
            }
        }
    }
}

/****************************************************************************/
// Add a static mac address to a switch in the stack
/****************************************************************************/
static void mac_static_switch_add(vtss_isid_t isid_add)
{
    mac_mgmt_addr_entry_t mac_entry;
    vtss_vid_mac_t        vid_mac;
    int                   vol;
    switch_iter_t         sit;

    T_I("Enter mac_static_switch_add:%d", isid_add);
    /* Add static address to the new unit, both volatile and non-volatile */
    /* All addresses must be refreshed as all addresses must be known in every unit.*/
    for (vol = 0; vol <= 1; vol++) {
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            memset(&vid_mac, 0, sizeof(vid_mac));
            while (mac_mgmt_static_get_next(sit.isid, &vid_mac, &mac_entry, 1, vol) == VTSS_OK) {
                vid_mac = mac_entry.vid_mac;
                (void)mac_add_stack(sit.isid, &mac_entry);
            }
        }
    }
    T_I("Exit mac_static_switch_add:%d", isid_add);
}

/****************************************************************************/
// Switch delete
/****************************************************************************/
static void mac_static_switch_del(vtss_isid_t isid_del)
{
    mac_mgmt_addr_entry_t        mac_entry;
    vtss_vid_mac_t               vid_mac;
    int                          vol;

    T_I("Enter mac_static_switch_del:%d", isid_del);

    /* Need to run through all static address for the deleted switch
       and remove the entries from other switches stack ports      */
    memset(&vid_mac, 0, sizeof(vid_mac));    /* start with all zero */
    for (vol = 0; vol <= 1; vol++) {
        while (mac_mgmt_static_get_next(isid_del, &vid_mac, &mac_entry, 1, vol) == VTSS_OK) {
            if (!msg_switch_is_master()) {
                return;
            }
            vid_mac = mac_entry.vid_mac;
            if (mac_del_stack(isid_del, &vid_mac, vol) != VTSS_OK) {
                T_W("Could not delete switch");
            }
        }
    }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    {
        mac_msg_req_t *msg;
        switch_iter_t sit;

        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        if (upsid_table.upsid[isid_del][0] == VTSS_VSTAX_UPSID_UNDEF) {
            return;
        }
        while (switch_iter_getnext(&sit)) {
            msg = mac_msg_req_alloc(MAC_MSG_ID_UPSID_FLUSH_REQ, 1);
            msg->req.upsid_flush.upsid = upsid_table.upsid[isid_del][0];
            mac_msg_tx(msg, sit.isid, sizeof(msg->req.upsid_flush));
            /* Check for a second UPSID */
            if (upsid_table.upsid[isid_del][1] != VTSS_VSTAX_UPSID_UNDEF) {
                msg = mac_msg_req_alloc(MAC_MSG_ID_UPSID_FLUSH_REQ, 1);
                msg->req.upsid_flush.upsid = upsid_table.upsid[isid_del][1];
                mac_msg_tx(msg, sit.isid, sizeof(msg->req.upsid_flush));
            }
        }
    }
#endif /* VTSS_FEATURE_VSTAX_V2 */

    T_I("Exit mac_static_switch_del:%d", isid_del);
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc mac_table_add(vtss_isid_t isid, mac_mgmt_addr_entry_t *entry)
{
    mac_static_t           *mac, *prev, *new = NULL, **mac_used_p, **mac_free_p;
    mac_entry_conf_t       conf;
    mac_mgmt_addr_entry_t  return_mac;
    vtss_mac_table_entry_t found_mac;
    uint                   i = 0;
    BOOL                   place_found = 0, newentry = 0, vol = entry->volatil;
    u32                    port_count = port_isid_port_count(isid);
    switch_iter_t          sit;

    T_N("Doing a table add at switch isid:%d", isid);

    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    {
        u32 destinations = 0;
        for (i = VTSS_PORT_NO_START; i < port_count; i++) {
            if (entry->destination[i]) {
                destinations++;
            }
        }
        if (destinations > 1) {
            return MAC_ERROR_MAC_ONE_DESTINATION_ALLOWED;
        }
    }
#endif

    /* If the address exist then exit with an proper error code */
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if ( mac_mgmt_static_get_next(sit.isid, &entry->vid_mac, &return_mac, 0, 0) == VTSS_OK) {  // non-volatile
            return MAC_ERROR_MAC_EXIST;
        }
        if (!vol) {
            if ( mac_mgmt_static_get_next(sit.isid, &entry->vid_mac, &return_mac, 0, 1) == VTSS_OK) {  // volatile
                return MAC_ERROR_MAC_VOL_EXIST;
            }
        }
    }

    /* Adresses added by other modules may not be changed, unless it's a volatile
     * entry, so that we know it's for debug purposes and not end-user controlled. */
    if (!vol) {
        if (mac_mgmt_table_get_next(isid, &entry->vid_mac, &found_mac, 0) == VTSS_OK) {
            if (found_mac.locked) {
                return MAC_ERROR_MAC_SYSTEM_EXIST;
            }
        }
    }

    MAC_CRIT_ENTER();
    if (vol) {
        mac_used_p = &mac_used_vol;
        mac_free_p = &mac_free_vol;
    } else {
        mac_used_p = &mac_used;
        mac_free_p = &mac_free;
    }

    for (mac = *mac_used_p; mac != NULL; mac = mac->next) {
        T_N("In list before add:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
            mac->conf.vid_mac.mac.addr[0], mac->conf.vid_mac.mac.addr[1], mac->conf.vid_mac.mac.addr[2],
            mac->conf.vid_mac.mac.addr[3], mac->conf.vid_mac.mac.addr[4], mac->conf.vid_mac.mac.addr[5], mac->conf.vid_mac.vid);
    }

    /*  Search for existing entry and if found replace with new entry data */
    for (mac = *mac_used_p, prev = NULL; mac != NULL; prev = mac, mac = mac->next) {
        if (mac_vid_compare(&mac->conf.vid_mac, &entry->vid_mac) && mac->conf.isid == isid) {
            new = mac;
            break;
        }
    }

    /* Convert datatype to the smaller Conf API */
    memset(&conf, 0, sizeof(conf));
    conf.vid_mac = entry->vid_mac;
    conf.isid = isid;
    conf.copy_to_cpu = entry->copy_to_cpu;
    if (entry->dynamic) {
        conf.mode = MAC_ADDR_DYNAMIC;
    }

    if (entry->not_stack_ports) {
        conf.mode = conf.mode | MAC_ADDR_NOT_INCL_STACK;
    }


    for (i = 0; i < port_count; i++) {
        VTSS_BF_SET(conf.destination, i, entry->destination[i + VTSS_PORT_NO_START]);
    }


    if (new == NULL && *mac_free_p == NULL) {
        T_W("Static MAC Table is full\n");
        MAC_CRIT_EXIT();
        return MAC_ERROR_REG_TABLE_FULL;
    } else {
        /* Add the new entry to the used list */
        if (new == NULL) {
            new = *mac_free_p;
            *mac_free_p = new->next;
            new->next = *mac_used_p;
            *mac_used_p = new;
            newentry = 1;
            T_N("Added new entry to local MAC Table");
        }
        new->conf = conf;

        if (newentry) {
            /* Find the right placement for the address in the pointerlist, lowest to highest */
            for (mac = *mac_used_p, prev = NULL; mac != NULL; prev = mac, mac = mac->next) {
                if (vid_mac_bigger(&mac->conf.vid_mac, &new->conf.vid_mac )) {
                    place_found = 1;
                    break;
                }
            }
            if (place_found && (prev != new)) {
                T_N("Not first and not last in the pointer list.");
                *mac_used_p = new->next;
                new->next = prev->next;
                prev->next = new;
            } else if (!place_found && (*mac_used_p)->next != NULL) {
                T_N("Using last place");
                *mac_used_p = new->next;
                prev->next = new;
                new->next = NULL;
            } else {
                T_N("Using the default 1.place");
            }
        }
    }

    for (mac = *mac_used_p; mac != NULL; mac = mac->next) {
        T_N("In list after add:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
            mac->conf.vid_mac.mac.addr[0], mac->conf.vid_mac.mac.addr[1], mac->conf.vid_mac.mac.addr[2],
            mac->conf.vid_mac.mac.addr[3], mac->conf.vid_mac.mac.addr[4], mac->conf.vid_mac.mac.addr[5], mac->conf.vid_mac.vid);
    }
    MAC_CRIT_EXIT();

    /* If adding to the local slave table the return now  */
    if (msg_switch_is_master()) {
        pr_entry("mac_table_add", entry);
        mac_add_stack(isid, entry);

        T_N("Added new entry to chip (ISID:%d)", isid);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* If non-volatile Update flash configuration */
        if (!vol) {
            mac_static_table_t *static_blk;

            MAC_CRIT_ENTER();
            i = 0;
            if ((static_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE, NULL)) == NULL) {
                T_E("failed to open port config table");
            } else {
                for (mac = mac_used; mac != NULL; mac = mac->next) {
                    static_blk->table[i] = mac->conf;
                    i++;
                }
                if (newentry) {
                    static_blk->count++;
                }

                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE);
                T_N("Added new entry to Flash");
            }
            MAC_CRIT_EXIT();
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    return VTSS_OK;
}

/****************************************************************************/
// Remove volatile or non-volatile static mac address from chip, flash and local configuration
/****************************************************************************/
static vtss_rc mac_table_del(vtss_isid_t isid, vtss_vid_mac_t *conf, BOOL vol)
{

    mac_static_t       *mac, *prev, **mac_used_p, **mac_free_p;
    mac_mgmt_addr_entry_t return_mac;
    BOOL  entry_found = 0;

    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

    MAC_CRIT_ENTER();
    if (vol) {
        mac_used_p = &mac_used_vol;
        mac_free_p = &mac_free_vol;
    } else {
        mac_used_p = &mac_used;
        mac_free_p = &mac_free;
    }
    MAC_CRIT_EXIT();

    if (msg_switch_is_master()) {
        /* Remove the entry from the chip through the API */
        if ( mac_mgmt_static_get_next(isid, conf, &return_mac, 0, vol) == VTSS_OK)
            if (mac_del_stack(isid, conf, vol) != VTSS_OK) {
                T_W("Could not del address");
            }
    }

    MAC_CRIT_ENTER();
    /* Find and remove the entry from the local list */
    for (mac = *mac_used_p, prev = NULL; mac != NULL; prev = mac, mac = mac->next) {

        if (mac_vid_compare(&mac->conf.vid_mac, conf) && mac->conf.isid == isid) {
            /* Found entry */
            entry_found = 1;
            if (prev == NULL) { /* if first entry */
                *mac_used_p = mac->next;
                mac->next = *mac_free_p;
                *mac_free_p = mac;
                T_N("Deleted first entry.");
            } else { /* Remove entry and add it to the free list */
                prev->next = mac->next;
                mac->next = *mac_free_p;
                *mac_free_p = mac;
                T_N("Deleted one entry.");
            }
            break;
        }
    }

    /* If deleting from the local slave table the return now  */
    if (msg_switch_is_master()) {
        /* If non-volatile update flash configuration */
        if (entry_found) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            if (!vol) {
                mac_static_table_t *static_blk;
                uint i = 0;

                if ((static_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE, NULL)) == NULL) {
                    T_E("Failed to open mac config table");
                } else {
                    static_blk->count--;
                    for (mac = mac_used; mac != NULL; mac = mac->next) {
                        static_blk->table[i] = mac->conf;
                        i++;
                    }
                    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE);
                    T_N("Deleted one entry from flash.");
                }
            }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        } else {
            T_D("Entry for removal not found.");
            MAC_CRIT_EXIT();
            return MAC_ERROR_MAC_NOT_EXIST;
        }

        T_N("Deleted one entry from chip.");
    }

    MAC_CRIT_EXIT();
    return VTSS_OK;
}

/****************************************************************************/
// Read/create mac age configuration
/****************************************************************************/
static void mac_conf_age_read(BOOL force_defaults)
{
    mac_age_conf_t conf;
    mac_conf_blk_t *age_blk;
    vtss_isid_t    isid;
    u32            size;
    BOOL           do_create = force_defaults;

    if (misc_conf_read_use()) {
        /* Open or create MAC Age configuration block */
        if ((age_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_AGE_CONF_TABLE, &size)) == NULL ||
            size != sizeof(*age_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");

            age_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MAC_AGE_CONF_TABLE, sizeof(*age_blk));
            do_create = TRUE;
        } else if (age_blk->version != MAC_CONF_VERSION) {

            T_W("Version mismatch, creating defaults");
            do_create = TRUE;
        }
    } else {
        age_blk   = NULL;
        do_create = TRUE;
    }

    if (do_create) {
        /* Use default values */
        conf.mac_age_time = MAC_AGE_TIME_DEFAULT;
        if (age_blk != NULL) {
            age_blk->conf = conf;
            age_blk->version = MAC_CONF_VERSION;
        }
    } else {
        /* Use stored configuration */
        if (age_blk != NULL) {  // Quiet lint
            T_D("Getting stored age time config");
            conf = age_blk->conf;
        } else {
            conf.mac_age_time = MAC_AGE_TIME_DEFAULT;
        }
    }

    MAC_CRIT_ENTER();
    mac_config.conf = conf;
    age_time_temp.stored_conf = conf;
    MAC_CRIT_EXIT();

    if (force_defaults) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++)  {
            if (!msg_switch_exists(isid)) {
                continue;
            }

            if (mac_age_stack_conf_set(isid) != VTSS_OK) {
                T_W("Could not set aging");
            }
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (age_blk == NULL) {
        T_E("Failed to open MAC config table");
    } else {
        age_blk->version = MAC_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_AGE_CONF_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/****************************************************************************/
// Read/create static MAC configuration
/****************************************************************************/
static void mac_conf_static_read(BOOL force_defaults)
{
    mac_static_t       *mac;
    mac_static_table_t *static_blk;
    BOOL               do_create = force_defaults;
    switch_iter_t      sit;
    u16                i;
    u32                size;

    if (misc_conf_read_use()) {
        /* Open or create MAC Static configuration block */
        if ((static_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE, &size)) == NULL ||
            size != sizeof(*static_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");

            static_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE, sizeof(*static_blk));
            do_create = TRUE;
        } else if (static_blk->version != MAC_STATIC_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = TRUE;
        }
    } else {
        static_blk = NULL;
        do_create  = TRUE;
    }

    if (do_create) {
        T_D("Setting mac static conf to default");
        if (static_blk) {
            /* Use start values */
            static_blk->count = 0;
            static_blk->version = MAC_STATIC_VERSION;
            static_blk->size = sizeof(*static_blk);
        }

        /* Delete old static volatile MACs from chip i.e. default the stack */

        /* Flush the master switch list */
        flush_mac_list(FALSE);

        /* Let each existing switch flush their addresses locally */
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            if (mac_remove_locked(sit.isid) != VTSS_OK) {
                T_W("Could not remove locked addresses isid: %d", sit.isid);
            }
        }

        /* Reset the linked mac list */
        build_mac_list();
    } else if (static_blk != NULL) {
        /* Restore from flash  */
        T_D("Getting stored static mac addresses");
        /* Move entries from flash to local structure and chip */
        MAC_CRIT_ENTER();
        for (i = static_blk->count; i != 0; i--) {
            if ((mac = mac_free) == NULL) {
                break;
            }

            mac_free = mac->next;
            mac->next = mac_used;
            mac_used = mac;
            mac->conf = static_blk->table[i - 1];
        }
        MAC_CRIT_EXIT();
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (static_blk == NULL) {
        T_E("Failed to open MAC config table");
    } else {
        static_blk->version = MAC_STATIC_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_STATIC_CONF_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/****************************************************************************/
// Read/create mac learn configuration
/****************************************************************************/
static void mac_conf_learn_read(BOOL force_defaults)
{
    mac_conf_learn_mode_t *learn_blk;
    vtss_learn_mode_t     learn_mode;
    BOOL                  do_create = force_defaults, learn_force_en = FALSE;
    vtss_port_no_t        port_no;
    u32                   size, port_count;
    switch_iter_t         sit;

    if (misc_conf_read_use()) {
        /* Open or create MAC learn mode configuration block */
        if ((learn_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_LEARN_MODE_TABLE, &size)) == NULL ||
            size != sizeof(*learn_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            learn_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MAC_LEARN_MODE_TABLE, sizeof(*learn_blk));
            do_create = TRUE;
        } else if (learn_blk->version != MAC_LEARN_MODE_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = TRUE;
        }
    } else {
        learn_blk = NULL;
        do_create = TRUE;
    }

    MAC_CRIT_ENTER();
    if (do_create) {
        mac_learn_mode.version = MAC_LEARN_MODE_VERSION;

        /* Automatic learning mode is default */
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        while (switch_iter_getnext(&sit)) {
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                VTSS_BF_SET(mac_learn_mode.learn_mode[sit.isid][port_no], LEARN_AUTOMATIC, 1);
                VTSS_BF_SET(mac_learn_mode.learn_mode[sit.isid][port_no], LEARN_CPU, 0);
                VTSS_BF_SET(mac_learn_mode.learn_mode[sit.isid][port_no], LEARN_DISCARD, 0);
            }
        }

        T_D("Setting mac learn mode to default");

        /* Default all switches in the stack */
        learn_mode.automatic = 1;
        learn_mode.cpu       = 0;
        learn_mode.discard   = 0;

        /* If 'learn_force' is enabled then that port must not be defaulted */
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        while (switch_iter_getnext(&sit)) {
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                if (learn_force.enable[sit.isid][port_no]) {
                    learn_force_en = TRUE;
                    break;
                }
            }
        }

        if (learn_force_en) {
            // Send configuration to all existing switches.
            (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                port_count = port_isid_port_count(sit.isid);
                for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                    if (!learn_force.enable[sit.isid][port_no]) {
                        MAC_CRIT_EXIT();
                        if (mac_stack_learn_mode_set(sit.isid, port_no, &learn_mode) != VTSS_OK) {
                            T_W("Could not set learn mode");
                        }
                        MAC_CRIT_ENTER();
                    }
                }
            }
        } else {
            // Send configuration to all existing switches.
            (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                MAC_CRIT_EXIT();
                if (mac_stack_learn_mode_set(sit.isid, MAC_ALL_PORTS, &learn_mode) != VTSS_OK) {
                    T_W("Could not set learn mode");
                }
                MAC_CRIT_ENTER();
            }
        }

        if (learn_blk != NULL) {
            *learn_blk = mac_learn_mode;
        }
    } else  if (learn_blk != NULL) {
        /* Restore from flash */
        mac_learn_mode = *learn_blk;
        T_D("Getting learn mode config from flash auto:%d", VTSS_BF_GET(mac_learn_mode.learn_mode[1][1], LEARN_AUTOMATIC));
    }

    MAC_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (learn_blk == NULL) {
        T_E("Failed to open MAC learn mode table");
    } else {
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_LEARN_MODE_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/****************************************************************************/
// Read/create mac module configuration
/****************************************************************************/
static void mac_conf_read(BOOL force_defaults)
{
    T_D("Enter");

    mac_conf_age_read(force_defaults);
    mac_conf_static_read(force_defaults);
    mac_conf_learn_read(force_defaults);

    if (force_defaults) {
        /* Flush the dynamic MAC table */
        if (mac_mgmt_table_flush() != VTSS_OK) {
            T_W("Could not flush");
        }
    }

    T_D("Exit");
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
static void topo_upsid_change_callback(vtss_isid_t isid)
{
    /* Re-build the configuration to get the new UPSID incorporated */
    flush_mac_list(TRUE);
    build_mac_list();
    mac_conf_read(FALSE);
}
#endif /* defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE */

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
// Port state callback function.
// This function is called if a GLOBAL port state change occur.
/****************************************************************************/
static void mac_port_global_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    mac_msg_req_t      *msg;
    vtss_isid_t        isid_tmp;
    u32                upspn;
    vtss_vstax_upsid_t upsid;

    if (msg_switch_is_master() && !info->stack) {
        if (!info->link) {
            MAC_CRIT_ENTER();
            upspn = mac_port_map_global[isid][port_no].chip_port;
            upsid = upsid_table.upsid[isid][mac_port_map_global[isid][port_no].chip_no];
            MAC_CRIT_EXIT();

            if (upsid == VTSS_VSTAX_UPSID_UNDEF) {
                return;
            }
            /* Port is down, flush the MAC table for that port, based on UPSID, UPSPN, on all units */
            /* Note that the local port on isid_tmp=isid is already flushed  */
            for (isid_tmp = VTSS_ISID_START; isid_tmp < VTSS_ISID_END; isid_tmp++) {
                if (!msg_switch_exists(isid_tmp) || (isid_tmp == isid)) {
                    continue;
                }
                msg = mac_msg_req_alloc(MAC_MSG_ID_UPSID_UPSPN_FLUSH_REQ, 1);
                msg->req.upspn_flush.upspn = upspn;
                msg->req.upspn_flush.upsid = upsid;
                mac_msg_tx(msg, isid_tmp, sizeof(msg->req.upspn_flush));
            }
        }
    }
}
#endif /* defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE */

/****************************************************************************/
/****************************************************************************/
static void mac_port_state_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    if (!info->link) {
        /* Port is down, flush the MAC table for that port */
        (void)vtss_mac_table_port_flush(NULL, port_no);
    }
}

/* Module start */
static void mac_start(void)
{
    T_D("Enter Mac start");

    /* Register for stack messages */
    if (mac_stack_register() != VTSS_OK) {
        T_W("Could not register for stack messages");
    }

    /* Register for Port LOCAL change callback */
    (void)port_change_register(VTSS_MODULE_ID_MAC, mac_port_state_change_callback);

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    /* Register for Port GLOBAL change callback */
    (void)port_global_change_register(VTSS_MODULE_ID_MAC, mac_port_global_state_change_callback);
    /* Register for UPSID change */
    (void)topo_upsid_change_callback_register(topo_upsid_change_callback, VTSS_MODULE_ID_MAC);
#endif

    /* Disable the force secure system mode */
    MAC_CRIT_ENTER();
    memset(&learn_force, 0, sizeof(learn_force));
    MAC_CRIT_EXIT();

    /* Reset the linked mac list */
    build_mac_list();

    T_D("exit");
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/****************************************************************************/
// Get the age time locally
/****************************************************************************/
vtss_rc mac_mgmt_age_time_get(mac_age_conf_t *conf)
{

    MAC_CRIT_ENTER();
    *conf = mac_config.conf;
    MAC_CRIT_EXIT();
    return VTSS_OK;
}

/****************************************************************************/
// Set the age time to all swithces in the stack
/****************************************************************************/
vtss_rc mac_mgmt_age_time_set(mac_age_conf_t *conf)
{
    int                  changed = 0;
    vtss_rc              rc = VTSS_OK;
    vtss_isid_t          isid;

    T_D("enter, age time set %u", conf->mac_age_time);

    if (!msg_switch_is_master()) {
        T_D("not master");
        return MAC_ERROR_STACK_STATE;
    }
    if ((conf->mac_age_time >= MAC_AGE_TIME_MIN &&
         conf->mac_age_time <= MAC_AGE_TIME_MAX) ||
        conf->mac_age_time == MAC_AGE_TIME_DISABLE  ) {

        MAC_CRIT_ENTER();
        changed = mac_age_changed(&mac_config.conf, conf);
        mac_config.conf = *conf;
        age_time_temp.stored_conf = *conf;;
        MAC_CRIT_EXIT();

        if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            mac_conf_blk_t *age_blk;
            if ((age_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_AGE_CONF_TABLE, NULL)) == NULL) {
                T_E("conf_sec_open failed.");
            } else  {
                age_blk->conf = *conf;
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_AGE_CONF_TABLE);
            }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                if (!msg_switch_exists(isid)) {
                    continue;
                }
                rc = mac_age_stack_conf_set(isid);
                T_D("Updated agetime in switch usid:%d", isid);
            }
        }
    } else {
        rc = VTSS_INVALID_PARAMETER;
    }
    return rc;
}

/****************************************************************************/
// Use another age time for specified time
/****************************************************************************/
vtss_rc mac_age_time_set(ulong mac_age_time, ulong age_period)
{
    MAC_CRIT_ENTER();
    age_time_temp.temp_conf.mac_age_time = mac_age_time;
    age_time_temp.age_period = age_period;
    /* Abort any aging + restart */
    T_D("%s(%u, %u)", __FUNCTION__, mac_age_time, age_period);
    cyg_flag_setbits( &age_flags, MACFLAG_ABORT_AGE | MACFLAG_START_AGE);
    MAC_CRIT_EXIT();

    return VTSS_OK;
}

/****************************************************************************/
// Get the next address from a switch in the Stack
/****************************************************************************/
vtss_rc mac_mgmt_table_get_next(vtss_isid_t isid, vtss_vid_mac_t *vid_mac, vtss_mac_table_entry_t *entry, BOOL next)
{
    mac_msg_req_t *msg;
    vtss_rc       rc = VTSS_OK;

    T_N("Doing a getnext at switch isid:%d", isid );

    if (mac_mgmt_port_sid_invalid(isid, VTSS_PORT_NO_START, 0, FALSE)) {
        return MAC_ERROR_GEN;
    }

    if (!msg_switch_is_master()) {
        rc = mac_local_table_get_next(vid_mac, entry, next);
    } else {
        msg = mac_msg_req_alloc(MAC_MSG_ID_GET_NEXT_REQ, 1);

        msg->req.get_next.vid_mac = *vid_mac;
        msg->req.get_next.next = next;

        mac_msg_tx(msg, isid, sizeof(msg->req.get_next));

        /* Wait for reply from stack */
        cyg_flag_value_t flags = cyg_flag_timed_wait( &getnext_flags, MACFLAG_WAIT_GETNEXT_DONE | MACFLAG_COULD_NOT_TX,
                                                      CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
                                                      cyg_current_time() + VTSS_OS_MSEC2TICK(MAC_REQ_TIMEOUT * 1000));
        if (flags == MACFLAG_COULD_NOT_TX) {
            return MAC_ERROR_STACK_STATE;
        }
        if (flags == 0) {
            T_W("Timeout or could not tx, MAC_MSG_ID_GET_NEXT_REQ ISID:%d", isid);
            return MAC_ERROR_REQ_TIMEOUT;
        }
        MAC_CRIT_ENTER();
        *entry = mac_config.get_next.entry;
        rc = mac_config.get_next.rc;
        MAC_CRIT_EXIT();
    }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    if (rc == VTSS_OK && entry->vstax2.remote_entry) {
        topo_sid_port_t sid_port;
        vtss_port_no_t  port_no, remote_port_no = VTSS_PORT_NO_NONE;

        if (entry->vstax2.upspn == VTSS_UPSPN_CPU) {
            /* Address is from remote CPU */
            entry->copy_to_cpu = 1;
        } else if (topo_upsid_upspn2sid_port(entry->vstax2.upsid, entry->vstax2.upspn, &sid_port) == VTSS_OK) {
            /* Address is from valid port on remote switch */
            remote_port_no = sid_port.port_no;
        }
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            entry->destination[port_no] = (port_no == remote_port_no ? 1 : 0);
        }
    }
#endif

    return rc;

}


/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_stack_get_next(vtss_vid_mac_t *vid_mac, mac_mgmt_table_stack_t *entry, mac_mgmt_addr_type_t *type, BOOL next)
{
    vtss_mac_table_entry_t       next_entry;
    vtss_isid_t                  local_isid = get_local_isid();
    u32                          i;
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    vtss_port_no_t               port_no, remote_port_no = VTSS_PORT_NO_NONE;
    vtss_isid_t                  remote_isid;
    topo_sid_port_t              sid_port;
#endif

    if (!msg_switch_is_master()) {
        T_D("not master");
        return MAC_ERROR_STACK_STATE;
    }
    memset(entry, 0, sizeof(mac_mgmt_table_stack_t));

    while (1) {
        if (next) {
            if (vtss_mac_table_get_next(NULL, vid_mac, &next_entry) != VTSS_OK) {
                break;
            }
        } else {
            if (vtss_mac_table_get(NULL, vid_mac, &next_entry) != VTSS_OK) {
                break;
            }
        }
        T_D("vid = %d, mac = %x:%x:%x:%x:%x:%x Next:%d", next_entry.vid_mac.vid,
            next_entry.vid_mac.mac.addr[1], next_entry.vid_mac.mac.addr[2], next_entry.vid_mac.mac.addr[3], next_entry.vid_mac.mac.addr[4],
            next_entry.vid_mac.mac.addr[4], next_entry.vid_mac.mac.addr[5], next);
        /* These we don't want */
        if ((type->only_this_vlan && (next_entry.vid_mac.vid != vid_mac->vid)) ||
            (type->not_dynamic    && !next_entry.locked) ||
            (type->not_static     && next_entry.locked) ||
            (type->not_cpu        && next_entry.copy_to_cpu) ||
            (type->not_mc         && (next_entry.vid_mac.mac.addr[0] & 0x1)) ||
            (type->not_uc         && !(next_entry.vid_mac.mac.addr[0] & 0x1))) {
            if (!next) {
                return MAC_ERROR_NOT_FOUND;
            }
            *vid_mac = next_entry.vid_mac;
            continue;
        }

        entry->vid_mac = next_entry.vid_mac;
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        if (next_entry.vstax2.remote_entry) {

            remote_isid = topo_upsid2isid(next_entry.vstax2.upsid);

            if (next_entry.vstax2.upspn == VTSS_UPSPN_CPU) {
                /* Address is from remote CPU */
                entry->copy_to_cpu = 1;
            } else if (topo_upsid_upspn2sid_port(next_entry.vstax2.upsid, next_entry.vstax2.upspn, &sid_port) == VTSS_OK) {
                /* Address is from valid port on remote switch */
                remote_port_no = sid_port.port_no;
            }
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                entry->destination[remote_isid][port_no] = (port_no == remote_port_no ? 1 : 0);
            }
        } else
#endif
        {
            for (i = 0; i < VTSS_PORTS; i++) {
                entry->destination[local_isid][i] = next_entry.destination[i];
            }
            entry->copy_to_cpu = next_entry.copy_to_cpu;
            entry->locked = next_entry.locked;
        }
        return VTSS_OK;
    }

    return MAC_ERROR_NOT_FOUND;
}

/****************************************************************************/
// Flush MAC address table
/****************************************************************************/
vtss_rc mac_mgmt_table_flush(void)
{
    mac_msg_req_t *msg;
    switch_iter_t sit;

    T_D("Flush mac table");

    if (!msg_switch_is_master()) {
        T_D("not master");
        return MAC_ERROR_STACK_STATE;
    }

    /* Run through all switches and flush the MAC table */
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);

    if ((msg = mac_msg_req_alloc(MAC_MSG_ID_FLUSH_REQ, sit.remaining)) != NULL) {
        while (switch_iter_getnext(&sit)) {
            mac_msg_tx(msg, sit.isid, 0);
        }
    }
    return VTSS_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_table_vlan_stats_get(vtss_isid_t isid, vtss_vid_t vlan, mac_table_stats_t *stats)
{
    mac_msg_req_t *msg;
    vtss_rc       rc = VTSS_OK;

    T_N("Doing a get vlan stats at switch isid:%d", isid);

    if (mac_mgmt_port_sid_invalid(isid, VTSS_PORT_NO_START, 0, FALSE)) {
        return MAC_ERROR_STACK_STATE;
    }

    if (msg_switch_is_local(isid)) {
        rc = mac_local_table_get_vlan_stats(vlan, stats);
    } else {
        msg = mac_msg_req_alloc(MAC_MSG_ID_GET_STATS_REQ, 1);
        msg->req.stats.vlan = vlan;
        mac_msg_tx(msg, isid, sizeof(msg->req.stats));

        /* Wait for reply from stack */
        cyg_flag_value_t flags = cyg_flag_timed_wait( &stats_flags, MACFLAG_WAIT_STATS_DONE | MACFLAG_COULD_NOT_TX,
                                                      CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
                                                      cyg_current_time() + VTSS_OS_MSEC2TICK(MAC_REQ_TIMEOUT * 1000));
        if (flags == MACFLAG_COULD_NOT_TX) {
            return MAC_ERROR_STACK_STATE;
        }
        if (flags == 0) {
            T_W("Timeout or could not tx, MAC_MSG_ID_GET_NEXT_REQ ISID:%d", isid);
            return MAC_ERROR_REQ_TIMEOUT;
        }

        MAC_CRIT_ENTER();
        *stats = mac_config.get_stats.stats;
        rc = mac_config.get_stats.rc;
        MAC_CRIT_EXIT();
    }
    return rc;
}

/****************************************************************************/
// Get the MAC statistics from a switch (isid) in the stack
/****************************************************************************/
vtss_rc mac_mgmt_table_stats_get(vtss_isid_t isid, mac_table_stats_t *stats)
{
    mac_msg_req_t *msg;
    vtss_rc       rc = VTSS_OK;

    T_N("Doing a get stats at switch isid:%d", isid );

    if (mac_mgmt_port_sid_invalid(isid, VTSS_PORT_NO_START, 0, FALSE)) {
        return MAC_ERROR_STACK_STATE;
    }

    if (msg_switch_is_local(isid)) {
        rc =  mac_local_table_get_stats(stats);
    } else {
        msg = mac_msg_req_alloc(MAC_MSG_ID_GET_STATS_REQ, 1);
        msg->req.stats.vlan = 0;

        mac_msg_tx(msg, isid, sizeof(msg->req.stats));

        /* Wait for reply from stack */
        cyg_flag_value_t flags = cyg_flag_timed_wait( &stats_flags, MACFLAG_WAIT_STATS_DONE | MACFLAG_COULD_NOT_TX,
                                                      CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
                                                      cyg_current_time() + VTSS_OS_MSEC2TICK(MAC_REQ_TIMEOUT * 1000));
        if (flags == MACFLAG_COULD_NOT_TX) {
            return MAC_ERROR_STACK_STATE;
        }

        if (flags == 0) {
            T_W("Timeout or could not tx, MAC_MSG_ID_GET_NEXT_REQ ISID:%d", isid);
            return MAC_ERROR_REQ_TIMEOUT;
        }
        MAC_CRIT_ENTER();
        *stats = mac_config.get_stats.stats;
        rc = mac_config.get_stats.rc;
        MAC_CRIT_EXIT();
    }
    return rc;
}

/****************************************************************************/
// Add volatile or non-volatile static mac address to chip
/****************************************************************************/
vtss_rc mac_mgmt_table_add(vtss_isid_t isid, mac_mgmt_addr_entry_t *entry)
{
    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

    if (msg_switch_is_master()) {
        T_D("Mac add %s, VLAN %u, vol:%u", misc_mac2str(entry->vid_mac.mac.addr), entry->vid_mac.vid, entry->volatil);
        return mac_table_add(isid, entry);
    } else {
        T_D("not master");
        return MAC_ERROR_STACK_STATE;
    }
}

/****************************************************************************/
// Delete all volatile or non-volatile static mac addresses from the port. Per switch.
/****************************************************************************/
vtss_rc mac_mgmt_table_port_del(vtss_isid_t isid, vtss_port_no_t port_no, BOOL vol)
{
    mac_mgmt_addr_entry_t mac_entry;
    vtss_vid_mac_t        vid_mac;
    vtss_rc               rc = VTSS_OK;

    if (mac_mgmt_port_sid_invalid(isid, port_no, 1, TRUE)) {
        return MAC_ERROR_GEN;
    }
    T_D("Port mac del, Port %u, vol:%u", port_no, vol);
    memset(&vid_mac, 0, sizeof(vid_mac));
    while (mac_mgmt_static_get_next(isid, &vid_mac, &mac_entry, 1, vol) == VTSS_OK) {
        if (mac_entry.destination[port_no]) {
            rc = mac_mgmt_table_del(isid, &mac_entry.vid_mac, vol);
        }

        vid_mac = mac_entry.vid_mac;
    }

    return rc;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_table_del(vtss_isid_t isid, vtss_vid_mac_t *conf, BOOL vol)
{
    if (mac_mgmt_port_sid_invalid(isid, VTSS_PORT_NO_START, 1, FALSE)) {
        return MAC_ERROR_GEN;
    }
    T_D("Mac del %s, VLAN %u, vol:%u", misc_mac2str(conf->mac.addr), conf->vid, vol);
    return mac_table_del(isid, conf, vol);
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_static_get_next(vtss_isid_t isid, vtss_vid_mac_t *search_mac, mac_mgmt_addr_entry_t *return_mac, BOOL next, BOOL vol)
{
    mac_static_t       *mac, *temp, *mac_used_p, *mac_prev;
    uint               port_no, i;
    vtss_rc            rc;


    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

    MAC_CRIT_ENTER();
    if (vol) {
        mac_used_p = mac_used_vol;
    } else {
        mac_used_p = mac_used;
    }

    /* Flashed addresses are always locked, with stack ports included  */
    return_mac->volatil = vol;
//    return_mac->dynamic = 0;
//    return_mac->not_stack_ports = 0;

    T_D("getting mac entry:%02x-%02x-%02x-%02x-%02x-%02x vid:%d, Next:%d, Volatile:%d, isid:%d",
        search_mac->mac.addr[0], search_mac->mac.addr[1], search_mac->mac.addr[2],
        search_mac->mac.addr[3], search_mac->mac.addr[4], search_mac->mac.addr[5], search_mac->vid, next, vol, isid);

    if (next) {
        T_N("Doing a get next at switch isid:%d. Volatile:%d", isid, vol);

        /* If the table is empty */
        if (mac_used_p == NULL) {
            rc = VTSS_UNSPECIFIED_ERROR;
            goto exit_func;
        }

        for (mac = mac_used_p, i = 1; mac != NULL; mac = mac->next, i++) {
            T_N("Entry %d Isid:%d:%02x-%02x-%02x-%02x-%02x-%02x vid:%d", i, mac->conf.isid,
                mac->conf.vid_mac.mac.addr[0], mac->conf.vid_mac.mac.addr[1], mac->conf.vid_mac.mac.addr[2],
                mac->conf.vid_mac.mac.addr[3], mac->conf.vid_mac.mac.addr[4], mac->conf.vid_mac.mac.addr[5], mac->conf.vid_mac.vid);
        }

        /* Search the list for a matching entry and return the next entry */
        for (mac = mac_used_p; mac != NULL; mac = mac->next) {
            if (mac_vid_compare(&mac->conf.vid_mac, search_mac) && mac->conf.isid == isid) {

                if (mac->next == NULL) {
                    rc = VTSS_UNSPECIFIED_ERROR;
                    goto exit_func;
                }

                /* Look for the next entry with the same isid */
                for (mac_prev = mac, mac = mac->next; mac != NULL; mac_prev = mac, mac = mac->next) {
                    if (mac->conf.isid == isid) {
                        mac = mac_prev;
                        break;
                    }
                }

                if (mac == NULL) {
                    rc = VTSS_UNSPECIFIED_ERROR;
                    goto exit_func;
                }

                if (mac->next != NULL) {
                    temp = mac->next;
                    return_mac->vid_mac = temp->conf.vid_mac;
                    return_mac->dynamic = (temp->conf.mode & MAC_ADDR_DYNAMIC) ? 1 : 0;
                    return_mac->not_stack_ports = (temp->conf.mode & MAC_ADDR_NOT_INCL_STACK) ? 1 : 0;
                    return_mac->copy_to_cpu = temp->conf.copy_to_cpu;
                    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                        return_mac->destination[port_no] = VTSS_PORT_BF_GET(temp->conf.destination, port_no);
                    }

                    T_D("Found entry. Isid:%d. Returning next mac entry:%02x-%02x-%02x-%02x-%02x-%02x vid:%d", temp->conf.isid,
                        return_mac->vid_mac.mac.addr[0], return_mac->vid_mac.mac.addr[1], return_mac->vid_mac.mac.addr[2],
                        return_mac->vid_mac.mac.addr[3], return_mac->vid_mac.mac.addr[4], return_mac->vid_mac.mac.addr[5], return_mac->vid_mac.vid);

                    rc = VTSS_OK;
                    goto exit_func;
                } else {
                    /* last entry */
                    rc = VTSS_UNSPECIFIED_ERROR;
                    goto exit_func;
                }
            } else {
                /* if this address is larger than the search address then return it */
                if (vid_mac_bigger(&mac->conf.vid_mac, search_mac) && mac->conf.isid == isid) {
                    return_mac->vid_mac = mac->conf.vid_mac;
                    return_mac->dynamic = (mac->conf.mode & MAC_ADDR_DYNAMIC) ? 1 : 0;
                    return_mac->not_stack_ports = (mac->conf.mode & MAC_ADDR_NOT_INCL_STACK) ? 1 : 0;
                    return_mac->copy_to_cpu = mac->conf.copy_to_cpu;
                    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                        return_mac->destination[port_no] = VTSS_PORT_BF_GET(mac->conf.destination, port_no);
                    }


                    T_D("Did not find entry. Return the next mac entry:%02x-%02x-%02x-%02x-%02x-%02x vid:%d",
                        return_mac->vid_mac.mac.addr[0], return_mac->vid_mac.mac.addr[1], return_mac->vid_mac.mac.addr[2],
                        return_mac->vid_mac.mac.addr[3], return_mac->vid_mac.mac.addr[4], return_mac->vid_mac.mac.addr[5], return_mac->vid_mac.vid);
                    rc = VTSS_OK;
                    goto exit_func;
                }
            }
        }

        if (mac == NULL) {
            rc = VTSS_UNSPECIFIED_ERROR;
            goto exit_func;
        }

        /* If the entry was not found in the list - should not happen.. */
        T_W("Entry not found!? Remember to start with search address 00:00:00:00:00:00.");
        rc = VTSS_UNSPECIFIED_ERROR;
        goto exit_func;
    } else {
        /* Do a Lookup */
        T_N("Do a lookup");
        for (mac = mac_used_p; mac != NULL; mac = mac->next) {
            if (mac_vid_compare(&mac->conf.vid_mac, search_mac) && mac->conf.isid == isid) {
                return_mac->vid_mac = mac->conf.vid_mac;
                return_mac->dynamic = (mac->conf.mode & MAC_ADDR_DYNAMIC) ? 1 : 0;
                return_mac->not_stack_ports = (mac->conf.mode & MAC_ADDR_NOT_INCL_STACK) ? 1 : 0;
                return_mac->copy_to_cpu = mac->conf.copy_to_cpu;
                for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                    return_mac->destination[port_no] = VTSS_PORT_BF_GET(mac->conf.destination, port_no);
                }
                T_D("Lookup Found");
                rc = VTSS_OK;
                goto exit_func;
            }
        }
        T_D("Lookup NOT Found");
        rc = VTSS_UNSPECIFIED_ERROR;
        goto exit_func;
    }

exit_func:
    MAC_CRIT_EXIT();
    return rc;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_learn_mode_set(vtss_isid_t isid, vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode)
{
    vtss_rc rc = VTSS_OK;
    BOOL    lf;

    T_D("Learn mode set, port:%u auto:%d cpu:%d discard:%d", port_no, learn_mode->automatic, learn_mode->cpu, learn_mode->discard);

    if (port_isid_port_no_is_stack(isid, port_no)) {
        return VTSS_OK;
    }

    if (mac_mgmt_port_sid_invalid(isid, port_no, 1, TRUE)) {
        return MAC_ERROR_STACK_STATE;
    }

    MAC_CRIT_ENTER();
    lf = learn_force.enable[isid][port_no];

    if (lf) {
        // We need to check that the new learn mode is not different from the old, since
        // the user shouldn't be able to change it when a particular learn mode is forced
        // by another module (notably the Port Security (PSEC) module).
        if (VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_AUTOMATIC) != learn_mode->automatic ||
            VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_CPU      ) != learn_mode->cpu       ||
            VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_DISCARD  ) != learn_mode->discard) {
            MAC_CRIT_EXIT();
            return MAC_ERROR_LEARN_FORCE_SECURE;
        } else {
            MAC_CRIT_EXIT();
            return VTSS_OK; // No changes.
        }
    }
    MAC_CRIT_EXIT();

    if (isid == VTSS_ISID_LOCAL) {
        isid = get_local_isid();
    }

    rc = mac_stack_learn_mode_set(isid, port_no, learn_mode);

    if (rc == VTSS_OK && !lf) {
        MAC_CRIT_ENTER();
        VTSS_BF_SET(mac_learn_mode.learn_mode[isid][port_no], LEARN_AUTOMATIC, learn_mode->automatic);
        VTSS_BF_SET(mac_learn_mode.learn_mode[isid][port_no], LEARN_CPU,       learn_mode->cpu);
        VTSS_BF_SET(mac_learn_mode.learn_mode[isid][port_no], LEARN_DISCARD,   learn_mode->discard);
        MAC_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save to flash */
        mac_conf_learn_mode_t *learn_blk;

        if ((learn_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MAC_LEARN_MODE_TABLE, NULL)) == NULL) {
            T_E("Failed to open mac config table");
        } else {
            T_D("Learn mode. Saving to flash and mem isid:%d. auto:%d\n", isid, learn_mode->automatic);
            MAC_CRIT_ENTER();
            *learn_blk = mac_learn_mode;
            MAC_CRIT_EXIT();
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MAC_LEARN_MODE_TABLE);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    return rc;
}

/****************************************************************************/
/****************************************************************************/
void mac_mgmt_learn_mode_get(vtss_isid_t isid, vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode, BOOL *chg_allowed)
{
    /* Return the learn mode, as we know it from our conf. */
    *chg_allowed = 1;

    if (VTSS_ISID_LEGAL(isid)) {
        // Always return the user's preferences, i.e. if some Module
        // has overridden the current mode, then just set the chg_allowed to FALSE.
        MAC_CRIT_ENTER();
        if (learn_force.enable[isid][port_no]) {
            *chg_allowed = 0;
        }
        learn_mode->automatic = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_AUTOMATIC);
        learn_mode->cpu =       VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_CPU);
        learn_mode->discard =   VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_DISCARD);
        MAC_CRIT_EXIT();
    } else {
        T_D("isid:%d is not a legal isid", isid);
    }
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_learn_mode_force_secure(vtss_isid_t isid, vtss_port_no_t port_no, BOOL cpu_copy)
{
    T_D("Setting isid:%d port:%d t forced secure learning cpu and discard = 1", isid, port_no);

    if (mac_mgmt_port_sid_invalid(isid, port_no, 1, TRUE)) {
        return MAC_ERROR_STACK_STATE;
    }
    MAC_CRIT_ENTER();
    learn_force.enable[isid][port_no] = 1;
    learn_force.learn_mode.automatic = 0;
    learn_force.learn_mode.discard = 1;
    learn_force.learn_mode.cpu = cpu_copy;
    MAC_CRIT_EXIT();
    if (mac_stack_learn_mode_set(isid, port_no, &learn_force.learn_mode) != VTSS_OK) {
        T_W("Could not set learn mode mode force");
        return VTSS_RC_ERROR;
    }
    return VTSS_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc mac_mgmt_learn_mode_revert(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_learn_mode_t learn_mode;

    T_D("Learn mode revert, port:%u", port_no);

    if (mac_mgmt_port_sid_invalid(isid, port_no, 1, TRUE)) {
        return MAC_ERROR_STACK_STATE;
    }

    MAC_CRIT_ENTER();
    if (!learn_force.enable[isid][port_no]) {
        MAC_CRIT_EXIT();
        return VTSS_OK;
    }
    learn_force.enable[isid][port_no] = 0;

    /* Get the saved learn mode */
    learn_mode.automatic = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_AUTOMATIC);
    learn_mode.cpu = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_CPU);
    learn_mode.discard = VTSS_BF_GET(mac_learn_mode.learn_mode[isid][port_no], LEARN_DISCARD);
    MAC_CRIT_EXIT();
    if (mac_stack_learn_mode_set(isid, port_no, &learn_mode) != VTSS_OK) {
        T_W("Could not set learn mode");
        return VTSS_RC_ERROR;
    }

    return VTSS_OK;
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
vtss_usid_t mac_mgmt_upsid2usid(const vtss_vstax_upsid_t upsid)
{
    vtss_isid_t isid;
    if (msg_switch_is_master()) {
        isid = topo_upsid2isid(upsid);
        if (isid == VTSS_ISID_UNKNOWN) {
            return isid;
        } else {
            return topo_isid2usid(isid);
        }
    } else {
        return 0;
    }
}
#endif /* VTSS_FEATURE_VSTAX_V2  */

/****************************************************************************/
// Return an error text string based on a return code.
/****************************************************************************/
char *mac_error_txt(vtss_rc rc)
{
    switch (rc) {
    case MAC_ERROR_GEN:
        return "General error";

    case MAC_ERROR_MAC_RESERVED:
        return "MAC address is reserved";

    case MAC_ERROR_REG_TABLE_FULL:
        return "Registration table full";

    case MAC_ERROR_REQ_TIMEOUT:
        return "Timeout on message request";

    case MAC_ERROR_STACK_STATE:
        return "Illegal MASTER/SLAVE state";

    case MAC_ERROR_MAC_EXIST:
        return "MAC address already exists";

    case MAC_ERROR_MAC_SYSTEM_EXIST:
        return "MAC address exists (system address)";

    case MAC_ERROR_MAC_VOL_EXIST:
        return "Volatile mac address already exists";

    case MAC_ERROR_MAC_ONE_DESTINATION_ALLOWED:
        return "Only one destination is allowed";

    default:
        return "MAC: Unknown error code";
    }
}

/****************************************************************************/
// Initialize module
/****************************************************************************/
vtss_rc mac_init(vtss_init_data_t *data)
{
    vtss_isid_t        isid = data->isid;
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    vtss_vstax_upsid_t upsid;
    vtss_port_no_t     port_no;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT_CMD_INIT");
        critd_init(&mac_config.crit,     "mac_config.crit",     VTSS_MODULE_ID_MAC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        critd_init(&mac_config.ram_crit, "mac_config.ram_crit", VTSS_MODULE_ID_MAC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        /* The request buffer pool consists of the greater of VTSS_ISID_CNT or four buffers because of the four requests sent upon a SWITCH_ADD event,
           and because of the heavy use of request buffers to all possible slaves whenever a port has link-down or a switch leaves the stack */
        // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of MSG_TX_DATA_HDR_LEN_MAX() below
        /*lint -e{506} */
        mac_config.request = msg_buf_pool_create(VTSS_MODULE_ID_MAC, "Request", VTSS_ISID_CNT > 4 ? VTSS_ISID_CNT : 4, sizeof(mac_msg_req_t));
        mac_config.reply   = msg_buf_pool_create(VTSS_MODULE_ID_MAC, "Reply",   1, sizeof(mac_msg_rep_t));

#ifdef VTSS_SW_OPTION_VCLI
        mac_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if (mac_icfg_init() != VTSS_OK) {
            T_D("Calling mac_icfg_init() failed");
        }
#endif
        cyg_flag_init(&age_flags);
        cyg_flag_init(&getnext_flags);
        cyg_flag_init(&stats_flags);
        cyg_flag_init(&mac_table_flags);
        MAC_CRIT_EXIT();
        MAC_RAM_CRIT_EXIT();

        /* Create aging thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          mac_thread,
                          0,
                          "Mac Age Control",
                          mac_thread_stack,
                          sizeof(mac_thread_stack),
                          &mac_thread_handle,
                          &mac_thread_block);
        break;

    case INIT_CMD_START:
        T_D("INIT_CMD_START");
        mac_start();
        break;

    case INIT_CMD_CONF_DEF:
        T_D("INIT_CMD_CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset configuration to default (local config on specific switch or global config for whole stack) */
            mac_conf_read(TRUE);
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP");
        flush_mac_list(FALSE);
        build_mac_list();
        mac_conf_read(FALSE);
        cyg_thread_resume(mac_thread_handle);
        break;

    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN");
        cyg_flag_setbits( &age_flags, MACFLAG_ABORT_AGE);
        flush_mac_list(TRUE);
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        // Get the logical-to-physical port map from the #data argument to this function.
        if (sizeof(mac_port_map_global[isid]) != sizeof(data->port_map)) {
            T_E("%u vs. %u", sizeof(mac_port_map_global[isid]), sizeof(data->port_map));
        }

        memcpy(mac_port_map_global[isid], data->port_map, sizeof(mac_port_map_global[isid]));

        /* Store the upsid for later use */
        if ((upsid = topo_isid_port2upsid(isid, VTSS_PORT_NO_START)) == VTSS_VSTAX_UPSID_UNDEF) {
            upsid_table.upsid[isid][0] = upsid;
            T_D("Could not get UPSID from Topo during Switch ADD, isid: %d", isid);
            break;
        }
        upsid_table.upsid[isid][0] = upsid;
        upsid_table.upsid[isid][1] = VTSS_VSTAX_UPSID_UNDEF;

        for (port_no = VTSS_PORT_NO_START + 1; (port_no < data->switch_info[isid].port_cnt) && (port_no != data->switch_info[isid].stack_ports[0]); port_no++) {
            if ((upsid = topo_isid_port2upsid(isid, port_no)) == VTSS_VSTAX_UPSID_UNDEF) {
                T_D("Could not get UPSID from Topo during Switch ADD, isid: %d", isid);
                break;
            }
            if (upsid_table.upsid[isid][0] != upsid) {
                upsid_table.upsid[isid][1] = upsid;
                break;
            }
        }
#endif /* VTSS_FEATURE_VSTAX_V2 */
        /* Configure the Age time */
        (void)mac_age_stack_conf_set(isid);
        /* Add static mac addresses */
        mac_static_switch_add(isid);
        /* Set the learn mode */
        mac_learnmode_switch_add(isid);
        break;

    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        mac_static_switch_del(isid);
        break;

    default:
        break;
    }

    T_D("exit mac_init");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
