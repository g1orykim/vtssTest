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
#include "port_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

#include "msg_api.h"
#include "misc_api.h"

#include "conf_api.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "critd_api.h"

#ifdef VTSS_SW_OPTION_LACP
#include "l2proto_api.h"
#include "lacp_api.h"
#ifndef _VTSS_LACP_OS_H_
#include "vtss_lacp_os.h"
#endif
#endif /* VTSS_SW_OPTION_LACP */
#if defined(VTSS_SW_OPTION_DOT1X)
#include "dot1x_api.h"
#endif /* VTSS_SW_OPTION_DOT1X */
#include "aggr_api.h"
#include "aggr.h"

#ifdef VTSS_SW_OPTION_CLI
#include "aggr_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICLI
#include "aggr_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_AGGR

#define VTSS_RC(expr)   { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) return __rc__; }
/*
  Synopsis.
  The aggregation module controls static and dynamic (LACP) created aggregations.

  VSTAX_V1:  Luton28 is EOL!
  VSTAX_V2:  Stack configuration as supported by JR.
  STANDALONE:  Standalone configuration as supported by L26, L28 and JR/JR-48 standalone. Only LLAGs

  Configuration.
  The static aggregations are saved to flash while only the LACP configuration is saved to ram and lost after reset.
  If a portlink goes down that port is removed from a dynamic created aggregation (at chiplevel) while the statically
  created will be left untouched (the portmask will be modified by the port-module).

  Aggregation id's.
  VSTAX_V2 (JR): No LLAGs. GLAGs numbered in the same way as AGGR_MGMT_GROUP_NO_START - AGGR_MGMT_GROUP_NO_END.
  All aggregations are identified by AGGR_MGMT_GROUP_NO_START - AGGR_MGMT_GROUP_NO_END.
  The aggregations are kept in one common pool for both static and dynamic usage, i.e. each could occupie all groups.

  Dynamic LLAGs and GLAGs.
  VSTAX_V2  (JR):  Dynamic aggregation are based on 32 GLAGs.

  Limited HW resources.
  The HW limits GLAG to hold max 8 ports while LLAG can hold 16.

  Aggregation map.
  The core of the LACP does not know anything about stacking.  All it's requests are based on aggregation id (aid) and a port number (l2_port).
  It's aid's spans the same range as number of ports, i.e. per default all ports are member of it's own aid.

  Crosshecks.
  The status of Dynamic and static aggregation are checked before they are created.
  This applies both to port member list as well as the aggr id.
  Furthermore dot1x may not coexist with an aggregation and is also checked before an aggregation
  is created.

  Callbacks.
  Some switch features are depended on aggregation status, e.g. STP.  Therefore they can be notified when an aggregation changes
  by use of callback subscription.  See aggr_api for details.

*/

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/
#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "aggr",
    .descr     = "AGGR"
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

#define AGGR_CRIT_ENTER() critd_enter(&aggrglb.aggr_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AGGR_CRIT_EXIT()  critd_exit( &aggrglb.aggr_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AGGR_CB_CRIT_ENTER()  critd_enter( &aggrglb.aggr_cb_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AGGR_CB_CRIT_EXIT()  critd_exit( &aggrglb.aggr_cb_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define AGGR_CRIT_ENTER() critd_enter(&aggrglb.aggr_crit)
#define AGGR_CRIT_EXIT()  critd_exit( &aggrglb.aggr_crit)
#define AGGR_CB_CRIT_ENTER()  critd_enter( &aggrglb.aggr_cb_crit)
#define AGGR_CB_CRIT_EXIT()  critd_exit( &aggrglb.aggr_cb_crit)
#endif /* VTSS_TRACE_ENABLED */

#define AGGRFLAG_ABORT_WAIT         (1 << 0)
#define AGGRFLAG_COULD_NOT_TX       (1 << 1)
#define AGGRFLAG_WAIT_DONE          (1 << 2)

/* Global structure */
static aggr_global_t aggrglb;
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
static vtss_rc aggr_vstax_v2_port_add(vtss_isid_t isid_add, aggr_mgmt_group_no_t glag_no, aggr_mgmt_group_t  *members, BOOL conf);
#endif

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Callback function if aggregation changes */
static void aggr_change_callback(vtss_isid_t isid, int aggr_no)
{
    u32 i;
    T_D("ISID:%d Aggr %d changed", isid, aggr_no);
    AGGR_CB_CRIT_ENTER();
    for (i = 0; i < aggrglb.aggr_callbacks; i++) {
        aggrglb.callback_list[i](isid, aggr_no);
    }
    AGGR_CB_CRIT_EXIT();
}

/* Save the config to flash */
static void aggr_save_to_flash(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    aggr_conf_stack_aggr_t   *aggr_blk;
    if ((aggr_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE, NULL)) == NULL) {
        T_E("failed to open port config table");
    } else {
        AGGR_CRIT_ENTER();
        *aggr_blk = aggrglb.aggr_config_stack;
        AGGR_CRIT_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE);
        T_D("Added new aggr entry to Flash");
    }
#else
    T_N("Silent-upgrade build: No conf flash save");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/****************************************************************************/
/****************************************************************************/
static u32 total_active_ports(vtss_glag_no_t aggr_no)
{
    vtss_isid_t       isid;
    vtss_port_no_t    p;
    u32               members = 0;

    AGGR_CRIT_ENTER();
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
            if (aggrglb.active_aggr_ports[isid][p] == aggr_no) {
                members++;
            }
        }
    }
    AGGR_CRIT_EXIT();
    return members;
}

/****************************************************************************/
/****************************************************************************/
static vtss_aggr_no_t get_aggrglb_config(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_aggr_no_t aggr;
    AGGR_CRIT_ENTER();
    aggr = aggrglb.config_aggr_ports[isid][port_no];
    AGGR_CRIT_EXIT();
    return aggr;
}

/****************************************************************************/
/****************************************************************************/
static void set_aggrglb_config(vtss_isid_t isid, vtss_port_no_t port_no, vtss_aggr_no_t aggr)
{
    AGGR_CRIT_ENTER();
    aggrglb.config_aggr_ports[isid][port_no] = aggr;
    AGGR_CRIT_EXIT();
}

/****************************************************************************/
/****************************************************************************/
static vtss_aggr_no_t get_aggrglb_active(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_aggr_no_t aggr;
    AGGR_CRIT_ENTER();
    aggr = aggrglb.active_aggr_ports[isid][port_no];
    AGGR_CRIT_EXIT();
    return aggr;
}

/****************************************************************************/
/****************************************************************************/
static void set_aggrglb_active(vtss_isid_t isid, vtss_port_no_t port_no, vtss_aggr_no_t aggr)
{
    AGGR_CRIT_ENTER();
    aggrglb.active_aggr_ports[isid][port_no] = aggr;
    AGGR_CRIT_EXIT();
}

/****************************************************************************/
/****************************************************************************/
static vtss_port_speed_t get_aggrglb_spd(vtss_aggr_no_t aggr)
{
    vtss_port_speed_t spd;
    AGGR_CRIT_ENTER();
    spd = aggrglb.aggr_group_speed[VTSS_ISID_LOCAL][aggr];
    AGGR_CRIT_EXIT();
    return spd;
}

/****************************************************************************/
/****************************************************************************/
static void set_aggrglb_spd(vtss_aggr_no_t aggr, vtss_port_speed_t spd)
{
    AGGR_CRIT_ENTER();
    aggrglb.aggr_group_speed[VTSS_ISID_LOCAL][aggr] = spd;
    AGGR_CRIT_EXIT();
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc aggr_add(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t  *members)
{
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    return aggr_vstax_v2_port_add(isid, aggr_no, members, FALSE);
#else
    vtss_port_no_t p;
    AGGR_CRIT_ENTER();
    for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
        if (aggrglb.config_aggr_ports[isid][p] == aggr_no) {
            aggrglb.active_aggr_ports[isid][p] = members->member[p] ? aggr_no : VTSS_AGGR_NO_NONE;
        }
    }
    AGGR_CRIT_EXIT();
    return vtss_aggr_port_members_set(NULL, aggr_no - AGGR_MGMT_GROUP_NO_START, members->member);
#endif
}

/****************************************************************************/
/****************************************************************************/
static void aggr_global_port_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    vtss_aggr_no_t       aggr_no = get_aggrglb_config(isid, port_no);
    vtss_port_no_t       p;
    BOOL                 active_changed = 0;
    vtss_port_speed_t    spd;
    aggr_mgmt_group_t    grp;
    vtss_isid_t          isid_tmp;
    port_status_t        ps;

    /* This function is called if a port state change occurs.
       This could mean that a aggr configured port is getting activated or kicked out of a group.
       Inactive ports (ports that does not have the group speed or HDX) must be blocked by e.g. STP */

    if (aggr_no != VTSS_AGGR_NO_NONE) {
        T_D("Port:%u. aggr_no:%d", port_no, aggr_no);
        AGGR_CRIT_ENTER();
        for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
            grp.member[p] = aggrglb.active_aggr_ports[isid][p] == aggr_no ? 1 : 0;
        }
        AGGR_CRIT_EXIT();
        spd = get_aggrglb_spd(aggr_no);
        if (info->link) {
            if (!info->fdx) {
                /* HDX ports not allowed in an aggregation */
                T_D("Port:%u. HDX ports not allowed - removing port from aggregation", port_no);
                grp.member[port_no] = 0;
            } else if (spd == VTSS_SPEED_UNDEFINED) {
                T_D("Port:%u. Speed is undefined", port_no);
                (void)set_aggrglb_spd(aggr_no, info->speed);
                for (isid_tmp = VTSS_ISID_START; isid_tmp < VTSS_ISID_END; isid_tmp++) {
                    memset(&grp, 0, sizeof(aggr_mgmt_group_t));
                    for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                        if (get_aggrglb_config(isid_tmp, p) == aggr_no) {
                            if (port_mgmt_status_get(isid_tmp, p, &ps) == VTSS_RC_OK) {
                                if (ps.status.link && ps.status.fdx && (ps.status.speed == info->speed)) {
                                    grp.member[p] = 1;
                                    active_changed = 1;
                                }
                            }
                        }
                    }
                    if (active_changed) {
                        T_D("Port is added to new speed group");
                        if (aggr_add(isid_tmp, aggr_no, &grp) != VTSS_RC_OK) {
                            T_E("Could not update AGGR group:%u", aggr_no);
                        }
                        (void)aggr_change_callback(isid_tmp, aggr_no);
                        active_changed = 0;
                    }
                }
                return;
            } else if (info->speed != spd) {
                grp.member[port_no] = 0;
            } else {
                grp.member[port_no] = 1;
            }
        } else {
            grp.member[port_no] = 0;
        }
        active_changed = ((get_aggrglb_active(isid, port_no) == aggr_no && !grp.member[port_no])
                          || (get_aggrglb_active(isid, port_no) != aggr_no && grp.member[port_no]));
        if (active_changed) {
            T_D("Port:%u is active status is changed", port_no);
            if (aggr_add(isid, aggr_no, &grp) != VTSS_RC_OK) {
                T_E("Could not update AGGR group:%u", aggr_no);
            }
            if (total_active_ports(aggr_no) == 0) {
                T_D("No active members left in aggr:%d, i.e. the speed is undef", aggr_no);
                /* No active members left, i.e. the speed is undef */
                (void)set_aggrglb_spd(aggr_no, VTSS_SPEED_UNDEFINED);
                /* Check other ports in the group */
                for (isid_tmp = VTSS_ISID_START; isid_tmp < VTSS_ISID_END; isid_tmp++) {
                    active_changed = 0;
                    memset(&grp, 0, sizeof(aggr_mgmt_group_t));
                    for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                        if (get_aggrglb_config(isid_tmp, p) == aggr_no) {
                            if (port_mgmt_status_get(isid_tmp, p, &ps) == VTSS_RC_OK) {
                                if (ps.status.link && ps.status.fdx && (get_aggrglb_spd(aggr_no) == VTSS_SPEED_UNDEFINED)) {
                                    (void)set_aggrglb_spd(aggr_no, ps.status.speed);
                                }
                                if (ps.status.link && ps.status.fdx && (get_aggrglb_spd(aggr_no) == ps.status.speed)) {
                                    grp.member[p] = 1;
                                    active_changed = 1;
                                }
                            }
                        }
                    }
                    if (active_changed) {
                        if (aggr_add(isid_tmp, aggr_no, &grp) != VTSS_RC_OK) {
                            T_E("Could not update AGGR group:%u", aggr_no);
                        }
                        (void)aggr_change_callback(isid_tmp, aggr_no);
                    }
                }
                return;
            }
            (void)aggr_change_callback(isid, aggr_no);
        }
    }
}


/****************************************************************************/
// Get config from chip API
/****************************************************************************/
static vtss_rc aggr_ports_get(aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t  *members)
{
    vtss_rc        rc = VTSS_RC_OK;
    vtss_port_no_t port_no;
    u32            port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    memset(members, 0, sizeof(aggr_mgmt_group_t));
    if (AGGR_MGMT_GROUP_IS_LAG(aggr_no)) {
        rc = vtss_aggr_port_members_get(NULL, aggr_no - AGGR_MGMT_GROUP_NO_START, members->member);

    } else if (AGGR_MGMT_GROUP_IS_GLAG(aggr_no)) {
        T_D("GET:Got glag %u for agg get.\n", aggr_no);
        AGGR_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (aggrglb.port_glag_member[port_no] == aggr_no) {
                members->member[port_no] = 1;
            } else {
                members->member[port_no] = 0;
            }
        }
        AGGR_CRIT_EXIT();
    } else {
        T_D("Fail. Got %u for port get.\n", aggr_no);
        rc = VTSS_INVALID_PARAMETER;
    }

    return rc;
}

/****************************************************************************/
// Set aggregation mode in chip API
/****************************************************************************/
static vtss_rc aggr_mode_set(vtss_aggr_mode_t *mode)
{
    T_D("Setting aggr mode:smac:%d, dmac:%d, IP:%d, port:%d.\n", mode->smac_enable,
        mode->dmac_enable, mode->sip_dip_enable, mode->sport_dport_enable);

    return vtss_aggr_mode_set(NULL, mode);
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
static vtss_rc apply_glag_ports_to_api(aggr_mgmt_group_no_t glag_no, aggr_glag_members_t *members, BOOL conf)
{
    u32 gentry;
    vtss_rc rc;
    vtss_vstax_conf_t ups_conf;
    vtss_vstax_upsid_t upsid;
    vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t  port_no;
    u32             port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    if (conf) {
        /* This is GLAG configuration belongs to this unit */
        if ((rc = vtss_vstax_conf_get(NULL, &ups_conf)) != VTSS_RC_OK) {
            return rc;
        }

        /* Get the port map for converting UPSPN to API port */
        if ((rc = vtss_port_map_get(NULL, port_map)) != VTSS_RC_OK) {
            return rc;
        }
        AGGR_CRIT_ENTER();
        /* Remove old GLAG info */
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            if (aggrglb.port_glag_member[port_no] == glag_no) {
                aggrglb.port_glag_member[port_no] = VTSS_GLAG_NO_NONE;
                aggrglb.port_active_glag_member[port_no] = VTSS_GLAG_NO_NONE;
            }
        }

        /* Convert the UPSPN to API port and save it in this local unit */
        for (gentry = VTSS_GLAG_PORT_START; gentry < VTSS_GLAG_PORT_END; gentry++) {
            if (members->entries[gentry].upspn == VTSS_UPSPN_NONE) {
                break;
            }

            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (port_map[port_no].chip_no == 0) {
                    upsid = ups_conf.upsid_0;
                } else {
                    upsid = ups_conf.upsid_1;
                }
                if ((port_map[port_no].chip_port == members->entries[gentry].upspn) && (members->entries[gentry].upsid == upsid)) {
                    T_D("Port:%u is a member of GLAG:%u upsid:%d upspn:%u",
                        port_no, glag_no, members->entries[gentry].upsid , members->entries[gentry].upspn);
                    aggrglb.port_glag_member[port_no] = glag_no;
                    aggrglb.port_active_glag_member[port_no] = glag_no;
                    break; /* Found the API port no for the upspn */
                }
            }
        }
        AGGR_CRIT_EXIT();
    }

    /* Update Chip masks */
    T_N("Updating GLAG masks");
    return vtss_vstax_glag_set(NULL, glag_no - AGGR_MGMT_GROUP_NO_START + VTSS_GLAG_NO_START, members->entries);
}

#endif /* VTSS_FEATURE_VSTAX_V2 */

/****************************************************************************/
/* Sets the aggregation mode.  The mode is used by all the aggregation groups */
/****************************************************************************/
static vtss_rc aggr_local_mode_set(vtss_aggr_mode_t *mode)
{
    vtss_rc rc;

    /* Add to chip */
    if ((rc = aggr_mode_set(mode) != VTSS_RC_OK)) {
        return rc;
    }
    /* Add to local config */
    AGGR_CRIT_ENTER();
    aggrglb.aggr_config.mode = *mode;
    AGGR_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// Gets ports in an aggregation group.
// The 'member' pointer points to the updated member list.
/****************************************************************************/
static vtss_rc aggr_local_port_members_get(aggr_mgmt_group_no_t aggr_no,  aggr_mgmt_group_member_t *members, BOOL next)
{
    vtss_rc           rc;
    vtss_port_no_t    port_no;
    BOOL              found_group = false;
    aggr_mgmt_group_t local;
    u32               port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    T_D("Enter");

    /* Get the first active aggregation group */
    if (aggr_no == 0 && next) {
        for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
            AGGR_CRIT_ENTER();
            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (VTSS_PORT_BF_GET(aggrglb.aggr_config.groups[aggr_no - AGGR_MGMT_GROUP_NO_START].member, (port_no - VTSS_PORT_NO_START))) {
                    found_group = 1;
                    break;
                }
            }
            AGGR_CRIT_EXIT();
            if (found_group) {
                break;
            }
        }
        if (!found_group) {
            return AGGR_ERROR_ENTRY_NOT_FOUND;
        }

    } else if (aggr_no != 0 && next) {
        /* Get the next active aggregation group */
        for (aggr_no = (aggr_no + 1); aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
            AGGR_CRIT_ENTER();
            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (VTSS_PORT_BF_GET(aggrglb.aggr_config.groups[aggr_no - AGGR_MGMT_GROUP_NO_START].member, (port_no - VTSS_PORT_NO_START))) {
                    found_group = 1;
                    break;
                }
            }
            AGGR_CRIT_EXIT();
            if (found_group) {
                break;
            }
        }
        if (!found_group) {
            return AGGR_ERROR_ENTRY_NOT_FOUND;
        }
    }

    if (AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
        /* Get from chip */
        rc = aggr_ports_get(aggr_no, &local);
        members->aggr_no = aggr_no;
        members->entry = local;
        return rc;
    } else {
        T_E("%u is not a legal aggregation group.", aggr_no);
        return VTSS_INVALID_PARAMETER;
    }
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/* Update and sort the global RAM Glag array */
/****************************************************************************/
static vtss_rc update_glag_array(vtss_isid_t isid, aggr_mgmt_group_no_t glag_no, aggr_mgmt_group_t *members)
{
    u32                      gentry, i, x, y;
    vtss_vstax_glag_entry_t  t_ups;
    vtss_isid_t              t_isid;
    u32                      port_count = port_isid_port_count(isid);
    vtss_port_no_t           port_no;
    vtss_glag_no_t           api_glag = glag_no - AGGR_MGMT_GLAG_START;

    if (!msg_switch_exists(isid)) {
        /* The switch has left this world, remove all members */
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            members->member[port_no] = 0;
        }
    }
    AGGR_CRIT_ENTER();
    if (aggrglb.upsid_table[isid].upsid[0] == VTSS_VSTAX_UPSID_UNDEF) {
        /* The switch does not exists, remove all members */
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            members->member[port_no] = 0;
        }
    }

    /* All GLAG ports for this ISID are included in the new member list */
    /* Remove old upsid entries before the new one is added */
    for (gentry = VTSS_GLAG_PORT_START, i = VTSS_GLAG_PORT_START; gentry < VTSS_GLAG_PORT_END; gentry++) {
        if (aggrglb.glag_ram_entries[api_glag][gentry].isid == VTSS_PORT_NO_NONE) {
            break;
        }

        if (aggrglb.glag_ram_entries[api_glag][gentry].isid != (vtss_isid_t)isid) {
            aggrglb.glag_ram_entries[api_glag][i] = aggrglb.glag_ram_entries[api_glag][gentry];
            i++;
        }
    }
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (aggrglb.active_aggr_ports[isid][port_no] == glag_no) {
            aggrglb.active_aggr_ports[isid][port_no] = VTSS_AGGR_NO_NONE;
        }
    }

    /* Convert <ISID/port_no> to <UPSPN/UPSID>. The UPSPN/UPSID is added to all units in the stack */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (members->member[port_no]) {
            aggrglb.glag_ram_entries[api_glag][i].ups.upspn = aggrglb.port_map_global[isid][port_no].chip_port;
            if (aggrglb.upsid_table[isid].upsid[1] == VTSS_VSTAX_UPSID_UNDEF) {
                aggrglb.glag_ram_entries[api_glag][i].ups.upsid = aggrglb.upsid_table[isid].upsid[0];
            } else {
                aggrglb.glag_ram_entries[api_glag][i].ups.upsid = aggrglb.upsid_table[isid].upsid[aggrglb.port_map_global[isid][port_no].chip_no];
            }
            aggrglb.glag_ram_entries[api_glag][i].isid = (vtss_isid_t)isid;
            T_D("Port:%u is an active GLAG member. Upsid:%d Upspn:%u",
                port_no, aggrglb.glag_ram_entries[api_glag][i].ups.upsid, aggrglb.glag_ram_entries[api_glag][i].ups.upspn);
            i++;
            aggrglb.active_aggr_ports[isid][port_no] = glag_no;
        }
    }
    aggrglb.glag_ram_entries[api_glag][i].ups.upspn = VTSS_UPSPN_NONE;
    aggrglb.glag_ram_entries[api_glag][i].ups.upsid = VTSS_VSTAX_UPSID_UNDEF;
    aggrglb.glag_ram_entries[api_glag][i].isid = VTSS_PORT_NO_NONE;

    /* Sort the Array: lowest to highest upsid */
    if (i > 0) {
        for (x = VTSS_GLAG_PORT_START; x < i - 1; x++) {
            for (y = 0; y < i - x - 1; y++) {
                if (aggrglb.glag_ram_entries[api_glag][y].isid > aggrglb.glag_ram_entries[api_glag][y + 1].isid) {
                    t_ups = aggrglb.glag_ram_entries[api_glag][y].ups;
                    t_isid = aggrglb.glag_ram_entries[api_glag][y].isid;
                    aggrglb.glag_ram_entries[api_glag][y] = aggrglb.glag_ram_entries[api_glag][y + 1];
                    aggrglb.glag_ram_entries[api_glag][y + 1].ups = t_ups;
                    aggrglb.glag_ram_entries[api_glag][y + 1].isid = t_isid;
                }
            }
        }
    }
    AGGR_CRIT_EXIT();
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_VSTAX_V2 */


/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
static vtss_rc aggr_vstax_v2_port_add(vtss_isid_t isid_add, aggr_mgmt_group_no_t glag_no, aggr_mgmt_group_t *members, BOOL conf)
{
    aggr_msg_add_req_t       *msg;
    u32                      gentry;
    switch_iter_t            sit;
    vtss_rc                  rc;

    if (!AGGR_MGMT_GROUP_IS_AGGR(glag_no)) {
        T_E("(aggr_stack_port_add) Agg group:%u is not a legal aggr id", glag_no);
        return AGGR_ERROR_PARM;
    }

    if ((rc = update_glag_array(isid_add, glag_no, members)) != VTSS_RC_OK) {
        T_E("(aggr_stack_port_add) Could not update array");
        return rc;
    }

    /* Add the new GLAG entry (UPSPN/UPSID format) to all existing ISIDs */
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        msg = VTSS_MALLOC(sizeof(*msg));
        if (msg) {
            msg->msg_id = AGGR_MSG_ID_ADD_REQ;
            msg->aggr_no = glag_no;
            msg->conf = (sit.isid == isid_add) ? conf : 0;
            AGGR_CRIT_ENTER();
            for (gentry = VTSS_GLAG_PORT_START; gentry < VTSS_GLAG_PORT_END; gentry++) {
                msg->members.entries[gentry] = aggrglb.glag_ram_entries[glag_no - AGGR_MGMT_GLAG_START][gentry].ups;
            }
            AGGR_CRIT_EXIT();
            msg_tx(VTSS_MODULE_ID_AGGR, sit.isid, msg, sizeof(*msg));
        }
    }

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE */

static vtss_rc aggr_group_add(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members)
{
    vtss_port_no_t    port_no;
    port_status_t     ps;
    u32               port_count = port_isid_port_count(isid);
    vtss_port_speed_t spd = VTSS_SPEED_UNDEFINED;
    aggr_mgmt_group_t *m, loc_members = *members;

    m = &loc_members;
    T_D("Adding new group %d ", aggr_no);
    /* In case of mixed port speeds find the highest */
    if (get_aggrglb_spd(aggr_no) == VTSS_SPEED_UNDEFINED) {
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (m->member[port_no]) {
                if (port_mgmt_status_get(isid, port_no, &ps) == VTSS_RC_OK) {
                    if (ps.status.link && ps.status.fdx) {
                        if (ps.status.speed > spd) {
                            spd = ps.status.speed;
                        }
                    }
                }
            }
        }
        T_D("Setting speed of group %d to %d ", aggr_no, spd);
        (void)set_aggrglb_spd(aggr_no, spd);
    }
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (get_aggrglb_config(isid, port_no) == aggr_no) {
            (void)set_aggrglb_config(isid, port_no, VTSS_AGGR_NO_NONE);
            (void)set_aggrglb_active(isid, port_no, VTSS_AGGR_NO_NONE);
        }
        if (m->member[port_no]) {
            (void)set_aggrglb_config(isid, port_no, aggr_no);
            if (port_mgmt_status_get(isid, port_no, &ps) != VTSS_RC_OK) {
                T_E("Could not get port info");
                return VTSS_UNSPECIFIED_ERROR;
            }
            /* Activate ports with the same speed and FDX, others remain inactive until the changes spd/dplx  */
            if (ps.status.fdx && ps.status.link && (get_aggrglb_spd(aggr_no) == ps.status.speed)) {
                m->member[port_no] = 1;
                (void)set_aggrglb_active(isid, port_no, aggr_no);
            } else {
                m->member[port_no] = 0;
            }
        }
    }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    /* JR Stackable - Convert to to UPSID/UPSPN and apply to stack */
    return aggr_vstax_v2_port_add(isid, aggr_no, m, TRUE);
#else
    /* JR/L26 Standalones - Apply to chip */
    return vtss_aggr_port_members_set(NULL, aggr_no - AGGR_MGMT_GROUP_NO_START, m->member);
#endif
}

static vtss_rc aggr_del(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no)
{

    vtss_port_no_t    port_no;
    u32               port_count = port_isid_port_count(isid);
    aggr_mgmt_group_t m;

    memset(&m, 0, sizeof(aggr_mgmt_group_t));
    T_D("Deleting group %d ", aggr_no);
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (get_aggrglb_config(isid, port_no) == aggr_no) {
            (void)set_aggrglb_config(isid, port_no, VTSS_AGGR_NO_NONE);
            (void)set_aggrglb_active(isid, port_no, VTSS_AGGR_NO_NONE);
        }
    }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    /* JR Stackable */
    return aggr_vstax_v2_port_add(isid, aggr_no, &m, TRUE);
#else
    /* JR/L26 Standalones */
    return vtss_aggr_port_members_set(NULL, aggr_no - AGGR_MGMT_GROUP_NO_START, m.member);
#endif // !defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE

}

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
/****************************************************************************/
static char *aggr_msg_id_txt(aggr_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case AGGR_MSG_ID_ADD_REQ:
        txt = "AGGR_MSG_ID_ADD_REQ";
        break;
    case AGGR_MSG_ID_MODE_SET_REQ:
        txt = "AGGR_MSG_ID_MODE_SET_REQ";
        break;
    case AGGR_MSG_ID_GLAG_UPDATE_REQ:
        txt = "AGGR_MSG_ID_GLAG_UPDATE_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_SWITCH_STACKABLE */

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// Receive message from the msg module and prepare a response
/****************************************************************************/
static BOOL aggr_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    aggr_msg_id_t msg_id = *(aggr_msg_id_t *)rx_msg;

    T_N("Rx: %s, len: %zd, isid: %u", aggr_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case AGGR_MSG_ID_ADD_REQ: {
        aggr_msg_add_req_t *msg_rx = (aggr_msg_add_req_t *)rx_msg;

        if (!AGGR_MGMT_GROUP_IS_AGGR(msg_rx->aggr_no)) {
            T_E("(aggr_msg_rx) Agg group:%u is not a legal aggr id", msg_rx->aggr_no);
            break;
        }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        if ((apply_glag_ports_to_api(msg_rx->aggr_no, &msg_rx->members, msg_rx->conf)) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not glag entry");
        }
#else
        if (aggr_local_port_members_add(msg_rx->aggr_no, &msg_rx->members) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not add llag entry");
        }
#endif
        break;
    }

    case AGGR_MSG_ID_MODE_SET_REQ: {
        aggr_msg_mode_set_req_t *msg_rx = (aggr_msg_mode_set_req_t *)rx_msg;

        if (aggr_local_mode_set(&msg_rx->mode) != VTSS_RC_OK) {
            T_W("(mac_msg_rx)Could not set aggr mode");
        }
        break;
    }

    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}
#endif /* VTSS_SWITCH_STACKABLE */

#if VTSS_SWITCH_STACKABLE
/****************************************************************************/
// Register the module to receive messages
/****************************************************************************/
static vtss_rc aggr_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = aggr_msg_rx;
    filter.modid = VTSS_MODULE_ID_AGGR;
    return msg_rx_filter_register(&filter);
}
#endif /* VTSS_SWITCH_STACKABLE */

/****************************************************************************/
// Remove an aggregation group in a switch in the stack
/****************************************************************************/
static void aggr_group_del(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no)
{
    aggr_mgmt_group_no_t  group_no;

    if (aggr_no == AGGR_ALL_GROUPS) {
        for (group_no = AGGR_MGMT_GROUP_NO_START; group_no < AGGR_MGMT_GROUP_NO_END; group_no++) {
            if (aggr_del(isid, group_no) != VTSS_RC_OK) {
                T_W("Could not del GLAG group");
            }
            if (total_active_ports(group_no) == 0) {
                (void)set_aggrglb_spd(group_no, VTSS_SPEED_UNDEFINED);
            }
        }
    } else {
        if (aggr_del(isid, aggr_no) != VTSS_RC_OK) {
            T_W("Could not del group:%u", aggr_no);
        }

        if (total_active_ports(aggr_no) == 0) {
            (void)set_aggrglb_spd(aggr_no, VTSS_SPEED_UNDEFINED);
        }
    }
}

/****************************************************************************/
// Set the aggr mode in a switch in the stack
/****************************************************************************/
static void aggr_stack_mode_set(vtss_isid_t isid, vtss_aggr_mode_t *mode)
{
    aggr_msg_mode_set_req_t *msg;

    if (msg_switch_is_local(isid)) {
        /* As this is the local swith we can take a shortcut  */

        if (aggr_local_mode_set(mode) != VTSS_RC_OK) {
            T_E("(aggr_stack_mode_set) Could not set mode");
        }
    } else {
        msg = VTSS_MALLOC(sizeof(*msg));
        if (msg) {
            msg->msg_id = AGGR_MSG_ID_MODE_SET_REQ;
            msg->mode = *mode;
            msg_tx(VTSS_MODULE_ID_AGGR, isid, msg, sizeof(*msg));
        }
    }
}

/****************************************************************************/
// Add configuration to the new switch in the stack
/****************************************************************************/
static void aggr_switch_add(vtss_isid_t isid)
{
    aggr_mgmt_group_no_t group_no;
    BOOL                 active_group = 0;
    aggr_mgmt_group_t    mgmt_members;
    port_iter_t          pit;
    vtss_aggr_mode_t     mode;

    if (!msg_switch_exists(isid)) {
        return;
    }

    memset(&mgmt_members, 0, sizeof(mgmt_members));

    /* Remove all groups */
    (void)aggr_group_del(isid, AGGR_ALL_GROUPS);

    /* Run through the groups and update if there is a group with members */
    for (group_no = AGGR_MGMT_GROUP_NO_START; group_no < AGGR_MGMT_GROUP_NO_END; group_no++) {

        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            AGGR_CRIT_ENTER();
            mgmt_members.member[pit.iport] = VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][group_no - AGGR_MGMT_GROUP_NO_START].member, (pit.iport - VTSS_PORT_NO_START));

            if (mgmt_members.member[pit.iport]) {
                active_group = 1;
            }
            AGGR_CRIT_EXIT();
        }

        if (active_group) {
            /* Add the group */
            if (!AGGR_MGMT_GROUP_IS_AGGR(group_no)) {
                T_E("(aggr_switch_add) Group:%u is not an legal aggr id", group_no);
                return;
            }
            T_I("Adding a group %u from FLASH ", group_no);
            if (aggr_group_add(isid, group_no, &mgmt_members) != VTSS_RC_OK) {
                return;
            }
            active_group = 0;
            /* Inform subscribers of aggregation changes */
            (void)aggr_change_callback(isid, group_no);
        }
    }

    /* Set the aggregation Mode  */
    AGGR_CRIT_ENTER();
    mode = aggrglb.aggr_config_stack.mode;
    AGGR_CRIT_EXIT();
    aggr_stack_mode_set(isid, &mode);
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
// Delete a unit
/****************************************************************************/
static void aggr_switch_del(vtss_isid_t isid)
{
    aggr_mgmt_group_t     glag_members;
    aggr_mgmt_group_no_t  glag_no;
    vtss_port_no_t        port_no;
    BOOL                  member;

    memset(&glag_members, 0, sizeof(glag_members));
    for (glag_no = AGGR_MGMT_GROUP_NO_START; glag_no < AGGR_MGMT_GROUP_NO_END; glag_no++) {
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            AGGR_CRIT_ENTER();
            member = VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][glag_no - AGGR_MGMT_GROUP_NO_START].member, (port_no - VTSS_PORT_NO_START));
            AGGR_CRIT_EXIT();
            if (member) {
                break;
            }
        }
        if (member) {
            if (aggr_vstax_v2_port_add(isid, glag_no, &glag_members, 0) != VTSS_RC_OK) {
                T_E("(aggr_switch_del) Could not delete switch");
            }
        }
    }
}

#endif /* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
static BOOL aggr_group_exists(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members)
{
    l2_port_no_t       l2port;
    BOOL               found_group = 0;
    vtss_port_no_t     port_no = 0;
    u32                aid, port_count = port_isid_port_count(isid);

    AGGR_CRIT_ENTER();
    for (aid = 0; aid < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); aid++) {
        if (aggrglb.aggr_lacp[aid].aggr_no == aggr_no) {
            for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                if (port_isid_port_no_is_stack(isid, port_no)) {
                    continue;
                }
                l2port = L2PORT2PORT(isid, port_no);
                if ((members->member[port_no] = aggrglb.aggr_lacp[aid].members[l2port]) == 1) {
                    found_group = 1;
                }
            }
        }
    }
    AGGR_CRIT_EXIT();
    return found_group;
}
#endif

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
// Find the next vacant aggregation id, LLAG or GLAG
/****************************************************************************/
static aggr_mgmt_group_no_t aggr_find_group(vtss_isid_t isid, aggr_mgmt_group_no_t start_group)
{
    aggr_mgmt_group_no_t  aggr_no;
    aggr_mgmt_group_member_t group;
    int                    aggr_taken = 0;
    switch_iter_t          sit;


    for (aggr_no = start_group; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {

        if (AGGR_MGMT_GROUP_IS_LAG(start_group)) {

            /* Static groups */
            if (aggr_mgmt_port_members_get(isid, aggr_no, &group, 0) == VTSS_RC_OK) {
                continue;
            }

            /* LACP groups */
            if (aggr_mgmt_lacp_members_get(isid, aggr_no, &group, 0) == VTSS_RC_OK) {
                continue;
            }
        } else {
            /* For GLAGs we need to run through all existing ISIDs */
            (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                aggr_taken = 0;
                if (aggr_mgmt_lacp_members_get(sit.isid, aggr_no, &group, 0) == VTSS_RC_OK) {
                    aggr_taken = 1;
                    break;
                }

                if (aggr_mgmt_port_members_get(sit.isid, aggr_no, &group, 0) == VTSS_RC_OK) {
                    aggr_taken = 1;
                    break;
                }
            }

            if (aggr_taken) {
                continue;
            }
        }

        return aggr_no;
    }

    /* No vacancy */
    return 0;
}
#endif /* VTSS_SW_OPTION_LACP */

/****************************************************************************/
/****************************************************************************/
static vtss_rc aggr_isid_port_valid(vtss_isid_t isid, vtss_port_no_t port_no, aggr_mgmt_group_no_t aggr_no, BOOL set)
{
    if (!AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
        T_W("%u is not a legal aggregation group.", aggr_no);
        return AGGR_ERROR_INVALID_ID;
    }

    if (isid == VTSS_ISID_LOCAL) {
        if (set) {
            T_W("SET not allowed, isid: %d", isid);
            return AGGR_ERROR_GEN;
        }
    } else if (!msg_switch_is_master()) {
        T_W("Not master");
        return AGGR_ERROR_NOT_MASTER;
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("Illegal isid %d", isid);
        return AGGR_ERROR_INVALID_ISID;
    } else if (!msg_switch_configurable(isid)) {
        T_W("Switch not configurable %d", isid);
        return AGGR_ERROR_INVALID_ISID;
    }

    if (port_no >= port_isid_port_count(isid)) {
        T_W("Illegal port_no %u", port_no);
        return AGGR_ERROR_INVALID_PORT;
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
static BOOL verify_max_members(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members)
{
    vtss_port_no_t  p;
    u32             port_count = port_isid_port_count(isid),  new_members = 0;

    for (p = VTSS_PORT_NO_START; p < port_count; p++) {
        if (members->member[p]) {
            new_members++;
        }
    }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    vtss_isid_t     isid_tmp;
    u32             conf_members = 0;
    {
        AGGR_CRIT_ENTER();
        for (isid_tmp = VTSS_ISID_START; isid_tmp < VTSS_ISID_END; isid_tmp++) {
            if (isid_tmp == isid) {
                continue;
            }
            for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                if (aggrglb.config_aggr_ports[isid_tmp][p] == aggr_no) {
                    conf_members++;
                }
            }
        }
        AGGR_CRIT_EXIT();
        if ((conf_members + new_members) > AGGR_MGMT_GLAG_PORTS_MAX) {
            return 0;
        }
    }
#else
    if (new_members > AGGR_MGMT_GLAG_PORTS_MAX) {
        return 0;
    }
#endif
    return 1;
}
/****************************************************************************/
/****************************************************************************/
#ifdef VTSS_SW_OPTION_LACP
vtss_rc check_for_lacp(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members)
{
    vtss_lacp_port_config_t  conf;
    vtss_port_no_t           port_no;
    aggr_mgmt_group_member_t members_tmp;
    u32                      port_count = port_isid_port_count(isid);
    int                      aid;
    vtss_rc                  rc;

    /* Check if the ports are occupied by LACP */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (members->member[port_no]) {
            if ((rc = lacp_mgmt_port_conf_get(isid, port_no, &conf)) != VTSS_RC_OK) {
                return rc;
            }
            if (conf.enable_lacp) {
                return AGGR_ERROR_LACP_ENABLED;
            }
        }
    }

    /* Check if the group is occupied by LACP */
    if (AGGR_MGMT_GROUP_IS_LAG(aggr_no)) {
        if (aggr_mgmt_lacp_members_get(isid, aggr_no,  &members_tmp, 0) == VTSS_RC_OK) {
            return AGGR_ERROR_GROUP_IN_USE;
        }
    } else {
        AGGR_CRIT_ENTER();
        for (aid = VTSS_PORT_NO_START; aid < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); aid++) {
            if (aggrglb.aggr_lacp[aid].aggr_no < AGGR_MGMT_GROUP_NO_START) {
                continue;
            }
            if (aggrglb.aggr_lacp[aid].aggr_no == aggr_no) {
                AGGR_CRIT_EXIT();
                return AGGR_ERROR_GROUP_IN_USE;
            }
        }
        AGGR_CRIT_EXIT();
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_LACP */

/****************************************************************************/
/****************************************************************************/
/* static vtss_rc aggr_return_error(vtss_rc rc, vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no) */
/* { */
/*     (void)set_aggrglb_spd(aggr_no, VTSS_SPEED_UNDEFINED); */
/*     return rc; */
/* } */

/****************************************************************************/
// Removes all members from a aggregation group, but doesn't save to flash.
/****************************************************************************/
static vtss_rc aggr_group_del_no_flash(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no)
{
    uint i;

    T_D("Doing a aggr del group %u at switch isid:%d", aggr_no, isid);

#ifdef VTSS_SW_OPTION_LACP
    aggr_mgmt_group_member_t members_tmp;
    /* Check if the group or ports is occupied by LACP in this isid */
    if (aggr_mgmt_lacp_members_get(isid, aggr_no, &members_tmp, 0) == VTSS_RC_OK) {
        return AGGR_ERROR_GROUP_IN_USE;
    }
#endif /* VTSS_SW_OPTION_LACP */

    (void)aggr_group_del(isid, aggr_no);
    AGGR_CRIT_ENTER();
    /* Delete from stack config */
    for (i = 0; i < VTSS_PORTS; i++) {
        VTSS_BF_SET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member, i, 0);
    }
    AGGR_CRIT_EXIT();

    /* Inform subscribers of aggregation changes */
    (void)aggr_change_callback(isid, aggr_no);

    return VTSS_RC_OK;
}

/****************************************************************************/
// Read/create and activate aggregation configuration
/****************************************************************************/
static void aggr_conf_stack_read(BOOL force_defaults)
{
    aggr_conf_stack_aggr_t   *aggr_blk;
    aggr_conf_2_80e_t        *aggr_blk_280 = NULL;
    ulong                    size;
    BOOL                     create = force_defaults, conf_280 = FALSE;
    u32                      i, aggr_no, group;
    aggr_mgmt_group_member_t members;
    vtss_aggr_mode_t         mode;
    switch_iter_t            sit;
    u32                      aid;
    vtss_isid_t              isid;
    vtss_port_no_t           port_no;

    T_I("Enter, force defaults: %d", force_defaults);

    if (misc_conf_read_use()) {
        /* Restore. Open or create configuration block */
        if ((aggr_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE, &size)) != NULL) {
            if (size == sizeof(*aggr_blk_280)) {
                if ((aggr_blk_280 = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE, &size)) != NULL) {
                    conf_280 = TRUE;
                    T_I("Found 280 conf");
                    if (aggr_blk_280->version != AGGR_CONF_VERSION) {
                        T_W("Version mismatch, creating defaults");
                        create = TRUE;
                    }
                } else {
                    T_W("conf_sec_open() failed. Creating defaults.");
                    create = TRUE;
                }
            } else if (size != sizeof(*aggr_blk)) {
                T_W("size mismatch. Creating defaults.");
                aggr_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE, sizeof(*aggr_blk));
                create = TRUE;
            } else if (aggr_blk->version != AGGR_CONF_VERSION) {
                T_W("Version mismatch, creating defaults");
                create = TRUE;
            }
        } else {
            T_W("conf_sec_open() failed. Creating defaults.");
            create = TRUE;
        }
    } else {
        // ICFG in action and not silent upgrading. Create defaults.
        aggr_blk = NULL;
        create   = TRUE;
    }

#ifdef VTSS_SW_OPTION_LACP
    /* Only reset the aggrglb.aggr_lacp stucture when becoming a master */
    /* When 'System Restore Default' is issued then LACP will delete */
    /* all of its members and notify subscribers.  */
    if (!force_defaults) {
        // Became master
        for (aid = 0; aid < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); aid++) {
            memset(&aggrglb.aggr_lacp[aid], 0, sizeof(aggrglb.aggr_lacp[0]));
        }
    }
#endif /* VTSS_SW_OPTION_LACP */

    if (create) {
        T_I("Defaulting aggr module");
        AGGR_CRIT_ENTER();
        // Enable default aggregation code contributions
        aggrglb.aggr_config_stack.mode.smac_enable = 1;
        aggrglb.aggr_config_stack.mode.dmac_enable = 0;
        aggrglb.aggr_config_stack.mode.sip_dip_enable = 1;
        aggrglb.aggr_config_stack.mode.sport_dport_enable = 1;
        aggrglb.aggr_config_stack.version = AGGR_CONF_VERSION;
        mode = aggrglb.aggr_config_stack.mode;
        AGGR_CRIT_EXIT();

        // First run through existing aggregations and delete them gracefully
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            // Only existing switches when we get here, and only as master.
            group = 0;
            while (aggr_mgmt_port_members_get(sit.isid, group, &members, TRUE) == VTSS_RC_OK) {
                group = members.aggr_no;
                // aggr_group_del_no_flash() also informs subscribers of aggreation changes
                if (aggr_group_del_no_flash(sit.isid, group) != VTSS_RC_OK) {
                    T_W("Could not delete group %d at isid %d", group, sit.isid);
                }
            }

            // Better safe than sorry
            (void)aggr_group_del(sit.isid, AGGR_ALL_GROUPS);

            // Also set the default aggregation mode
            aggr_stack_mode_set(sit.isid, &mode);
        }

        // Clear configuration for *all* switches
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        while (switch_iter_getnext(&sit)) {
            /* Reset the local memory configuration */
            AGGR_CRIT_ENTER();
            for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
                for (i = 0; i < VTSS_PORTS; i++) {
                    VTSS_BF_SET(aggrglb.aggr_config_stack.switch_id[sit.isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member, i, 0);
                }
            }
            AGGR_CRIT_EXIT();
        }
    } else {
        // Flash read succeeded and we've not created defaults. Use whatever we got from flash.
        // 2_80e has a different Flash layout need to do a workaround to support it.
        if (conf_280 && aggr_blk_280 != NULL) {
            T_I("Using conf from 2_80 FLASH");
            AGGR_CRIT_ENTER();
            aggrglb.aggr_config_stack.version = aggr_blk_280->version;
            aggrglb.aggr_config_stack.mode = aggr_blk_280->mode;
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
                    for (i = 0; i < VTSS_BF_SIZE(VTSS_PORTS); i++) {
                        aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member[i] =
                            aggr_blk_280->switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member[i];
                    }
                }
            }
            AGGR_CRIT_EXIT();
        } else {
            T_I("Using conf from FLASH");
            AGGR_CRIT_ENTER();
            if (aggr_blk != NULL) {
                aggrglb.aggr_config_stack = *aggr_blk;
            }
            AGGR_CRIT_EXIT();
        }
    }

    // Clear all (but LACP) runtime state, whether or not this is master-up or restore to defaults.
    for (isid = 0; isid < VTSS_ISID_END; isid++) {
        for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
            aggrglb.aggr_group_speed[isid][aggr_no] = VTSS_SPEED_UNDEFINED;
        }
        for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
            aggrglb.active_aggr_ports[isid][port_no] = VTSS_AGGR_NO_NONE;
            aggrglb.config_aggr_ports[isid][port_no] = VTSS_AGGR_NO_NONE;
        }
    }

    if (!force_defaults) {
        // In master-up, also clear LACP runtime state
        for (isid = 0; isid < VTSS_ISID_END; isid++) {
            for (aid = 0; aid < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); aid++) {
                memset(&aggrglb.aggr_lacp[aid], 0, sizeof(aggrglb.aggr_lacp[0]));
            }
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (aggr_blk != NULL) {
        AGGR_CRIT_ENTER();
        *aggr_blk = aggrglb.aggr_config_stack;
        AGGR_CRIT_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_AGGR_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
}

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
/****************************************************************************/
// Re-build the configuration to get the new UPSID incorporated
/****************************************************************************/
static void topo_upsid_change_callback(vtss_isid_t isid)
{
    switch_iter_t sit;

    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        (void)aggr_switch_del(sit.isid);
    }
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        (void)aggr_switch_add(sit.isid);
    }
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

/****************************************************************************/
/****************************************************************************/
static void aggr_start(BOOL init)
{
    aggr_mgmt_group_no_t aggr;
    vtss_isid_t          isid;
    vtss_port_no_t       port_no;

    if (init) {
        /* Initialize global area */
        memset(&aggrglb, 0, sizeof(aggrglb));

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&aggrglb.request.sem, 1);

        /* Reset the switch aggr conf */
        for (isid = 0; isid < VTSS_ISID_END; isid++) {
            for (aggr = AGGR_MGMT_GROUP_NO_START; aggr < AGGR_MGMT_GROUP_NO_END; aggr++) {
                aggrglb.aggr_group_speed[isid][aggr] = VTSS_SPEED_UNDEFINED;
            }

            for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
                aggrglb.active_aggr_ports[isid][port_no] = VTSS_AGGR_NO_NONE;
                aggrglb.config_aggr_ports[isid][port_no] = VTSS_AGGR_NO_NONE;
            }
        }

        /* Open up switch API after initialization */
        critd_init(&aggrglb.aggr_crit, "aggr_crit", VTSS_MODULE_ID_AGGR, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        AGGR_CRIT_EXIT();
        critd_init(&aggrglb.aggr_cb_crit, "aggr_cb_crit", VTSS_MODULE_ID_AGGR, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        AGGR_CB_CRIT_EXIT();

    } else {
        for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
            aggrglb.port_glag_member[port_no] = VTSS_GLAG_NO_NONE;
            aggrglb.port_aggr_member[port_no] = VTSS_AGGR_NO_NONE;
//            aggrglb.port_active_aggr_member[port_no] = VTSS_AGGR_NO_NONE;
        }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
            aggrglb.port_active_glag_member[port_no] = VTSS_GLAG_NO_NONE;
        }
        for (aggr = AGGR_MGMT_GROUP_NO_START - AGGR_MGMT_GROUP_NO_START; aggr < AGGR_MGMT_GROUP_NO_END - AGGR_MGMT_GROUP_NO_START; aggr++) {
            aggrglb.glag_ram_entries[aggr][VTSS_GLAG_PORT_START].ups.upsid = VTSS_VSTAX_UPSID_UNDEF;
            aggrglb.glag_ram_entries[aggr][VTSS_GLAG_PORT_START].ups.upspn = VTSS_PORT_NO_NONE;
            aggrglb.glag_ram_entries[aggr][VTSS_GLAG_PORT_START].isid = VTSS_PORT_NO_NONE;
        }
#endif /* VTSS_FEATURE_VSTAX_V2 */

        /* Register for Port GLOBAL change callback */
        (void)port_global_change_register(VTSS_MODULE_ID_AGGR, aggr_global_port_state_change_callback);

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        /* Register for UPSID change */
        (void)topo_upsid_change_callback_register(topo_upsid_change_callback, VTSS_MODULE_ID_AGGR);
#endif /* VTSS_FEATURE_VSTAX_V2 */

#if VTSS_SWITCH_STACKABLE
        /* Register for stack messages */
        (void)aggr_stack_register();
#endif /* VTSS_SWITCH_STACKABLE */
    }
}

void aggr_mgmt_dump(aggr_dbg_printf_t dbg_printf)
{
#if defined(VTSS_FEATURE_VSTAX_V2)
    (void)dbg_printf("VTSS_FEATURE_VSTAX_V2 is defined, \nVTSS_SWITCH_STACKABLE:%d\n", VTSS_SWITCH_STACKABLE);
#endif
#if !defined(VTSS_FEATURE_VSTAX_V2)
    (void)dbg_printf("VTSS_FEATURE_VSTAX_V2 is not defined. VTSS_SWITCH_STACKABLE:%d\n", VTSS_SWITCH_STACKABLE);
#endif
    (void)dbg_printf("VTSS_PORTS:%d \nVTSS_AGGRS:%d \nAGGR_LLAG_CNT:%d \nAGGR_GLAG_CNT:%d \nAGGR_MGMT_LAG_PORTS_MAX:%d \nAGGR_MGMT_GLAG_PORTS_MAX:%d\n",
                     VTSS_PORTS, VTSS_AGGRS, AGGR_LLAG_CNT, AGGR_GLAG_CNT, AGGR_MGMT_LAG_PORTS_MAX, AGGR_MGMT_GLAG_PORTS_MAX);

    (void)dbg_printf("AGGR_MGMT_GROUP_NO_START:%d  \nAGGR_MGMT_GROUP_NO_END:%d\n", AGGR_MGMT_GROUP_NO_START, AGGR_MGMT_GROUP_NO_END);


#if !VTSS_SWITCH_STACKABLE
    (void)dbg_printf("LLAGs:\n");
#else
    (void)dbg_printf("GLAGs:\n");
#endif

    aggr_mgmt_group_no_t aggr_no;
    vtss_isid_t isid;
    vtss_port_no_t port_no;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
            if (get_aggrglb_config(isid, port_no) != VTSS_AGGR_NO_NONE) {
                (void)dbg_printf("ISID:%d Port:%u is configured aggr:%u  Active:%s\n", isid, port_no, get_aggrglb_config(isid, port_no), get_aggrglb_active(isid, port_no) != VTSS_AGGR_NO_NONE ? "Yes" : "No");
            }
        }
    }

    for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
        if (get_aggrglb_spd(aggr_no) != VTSS_SPEED_UNDEFINED) {
            (void)dbg_printf("Group:%u speed:%d\n", aggr_no, aggrglb.aggr_group_speed[VTSS_ISID_LOCAL][aggr_no]);
        }
    }
}

/****************************************************************************/
/*  Management/API functions                                                */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
char *aggr_error_txt(vtss_rc rc)
{
    switch (rc) {
    case AGGR_ERROR_GEN:
        return "Aggregation generic error";
    case AGGR_ERROR_PARM:
        return "Illegal parameter";
    case AGGR_ERROR_REG_TABLE_FULL:
        return "Registration table full";
    case AGGR_ERROR_REQ_TIMEOUT:
        return "Timeout on message request";
    case AGGR_ERROR_STACK_STATE:
        return "Illegal MASTER/SLAVE state";
    case AGGR_ERROR_GROUP_IN_USE:
        return "Group already in use";
    case AGGR_ERROR_PORT_IN_GROUP:
        return "Port already in another group";
    case AGGR_ERROR_LACP_ENABLED:
        return "LACP aggregation is enabled";
    case AGGR_ERROR_DOT1X_ENABLED:
        return "DOT1X is enabled";
    case AGGR_ERROR_ENTRY_NOT_FOUND:
        return "Entry not found";
    case AGGR_ERROR_HDX_SPEED_ERROR:
        return "Illegal duplex or speed state";
    case AGGR_ERROR_MEMBER_OVERFLOW:
        return "To many port members";
    case AGGR_ERROR_INVALID_ID:
        return "Invalid group id";
    default:
        return "Aggregation unknown error";
    }
}

/****************************************************************************/
// Adds ports to an aggregation.
// All ports are considered valid to get added to an aggr group but
// only ports with the same speed/fdx are actually activated in the aggregation.
// In case of different speeds then the port with the highest speed controls the speed of the group.
/****************************************************************************/

vtss_rc aggr_mgmt_port_members_add(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members)
{
    vtss_port_no_t           port_no;
    BOOL                     members_incl = FALSE;
    u32                      port_count = port_isid_port_count(isid);
    vtss_rc                  rc;
    aggr_mgmt_group_no_t     gr;

    T_D("Doing a aggr add group %u at switch isid %d", aggr_no, isid);

    VTSS_RC(aggr_isid_port_valid(isid, VTSS_PORT_NO_START, aggr_no, TRUE /* Set command */));

    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (members->member[port_no]) {
            members_incl = 1;
        }
        /* Check for stack port */
        if (members->member[port_no] && port_isid_port_no_is_stack(isid, port_no)) {
            return AGGR_ERROR_PARM;
        }

        /* Check if the member port is already part of another group */
        for (gr = AGGR_MGMT_GROUP_NO_START; gr < AGGR_MGMT_GROUP_NO_END; gr++) {
            if (gr == aggr_no) {
                continue;
            }
            AGGR_CRIT_ENTER();
            if (VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START] \
                                 [gr - AGGR_MGMT_GROUP_NO_START].member, (port_no - VTSS_PORT_NO_START)) && members->member[port_no]) {
                AGGR_CRIT_EXIT();
                return AGGR_ERROR_PORT_IN_GROUP;
            }
            AGGR_CRIT_EXIT();
        }
    }
    if (!members_incl) {
        return (aggr_mgmt_group_del(isid, aggr_no));
    }

#ifdef VTSS_SW_OPTION_LACP
    if ((rc = check_for_lacp(isid, aggr_no, members)) != VTSS_RC_OK)  {
        return rc;
    }
#endif /* VTSS_SW_OPTION_LACP */

#if defined(VTSS_SW_OPTION_DOT1X)
    dot1x_switch_cfg_t switch_cfg;

    /* Check if dot1x is enabled */
    if (dot1x_mgmt_switch_cfg_get(isid, &switch_cfg) != VTSS_RC_OK) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (members->member[port_no] && switch_cfg.port_cfg[port_no - VTSS_PORT_NO_START].admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
            return AGGR_ERROR_DOT1X_ENABLED;
        }
    }
#endif /* VTSS_SW_OPTION_DOT1X */

    if (!verify_max_members(isid, aggr_no, members)) {
        return AGGR_ERROR_MEMBER_OVERFLOW;
    }

    /* Apply the configuration to the chip - if the switch exist */
    if (msg_switch_exists(isid) && (rc = aggr_group_add(isid, aggr_no, members)) != VTSS_RC_OK) {
        return rc;
    }

    /* Add to stack config */
    AGGR_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        VTSS_BF_SET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member,
                    (port_no - VTSS_PORT_NO_START), members->member[port_no]);
    }
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if ( VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member,
                              (port_no - VTSS_PORT_NO_START))) {
            T_D("Aggr:%u  port:%u are conf agg members\n", aggr_no, port_no);
        }
    }
    AGGR_CRIT_EXIT();

    (void)aggr_save_to_flash();

    /* Inform subscribers of aggregation changes */
    (void)aggr_change_callback(isid, aggr_no);

    return VTSS_RC_OK;
}

/****************************************************************************/
// Removes all members from a aggregation group.
/****************************************************************************/
vtss_rc aggr_mgmt_group_del(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no)
{
    VTSS_RC(aggr_isid_port_valid(isid, VTSS_PORT_NO_START, aggr_no, TRUE /* Set command */));
    VTSS_RC(aggr_group_del_no_flash(isid, aggr_no));
    (void)aggr_save_to_flash();
    return VTSS_RC_OK;
}

/****************************************************************************/
// Get members in a given aggr group.
// 'Next' is used to browes trough active groups.
// Configuration version. See aggr_mgmt_members_get() for runtime version.
/****************************************************************************/
vtss_rc aggr_mgmt_port_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_member_t *members, BOOL next)
{
    vtss_rc            rc = AGGR_ERROR_ENTRY_NOT_FOUND;
    vtss_port_no_t     port_no;
    BOOL               found_group = FALSE;
    port_iter_t        pit;

    VTSS_RC(aggr_isid_port_valid(isid, VTSS_PORT_NO_START, 1, FALSE /* Get command, VTSS_ISID_LOCAL is OK */));

    if (isid == VTSS_ISID_LOCAL) {
        rc = aggr_local_port_members_get(aggr_no, members, next);
    } else {
        /* Get the first active aggregation group */
        if (aggr_no == 0 && next) {
            aggr_no = AGGR_MGMT_GROUP_NO_START;
        } else if (aggr_no != 0 && next) {
            aggr_no++;
        }

        while (next && aggr_no < AGGR_MGMT_GROUP_NO_END) {

            AGGR_CRIT_ENTER();
            (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member, (pit.iport - VTSS_PORT_NO_START))) {
                    T_N("Found (next) static members in aggr:%u, isid:%d", aggr_no, isid);
                    found_group = TRUE;
                    break;
                }
            }
            AGGR_CRIT_EXIT();

            if (found_group) {
                break;
            }

            aggr_no++;
        }

        if (next && !found_group) {
            return AGGR_ERROR_ENTRY_NOT_FOUND;
        }

        members->aggr_no = aggr_no;
        if (AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
            AGGR_CRIT_ENTER();
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                members->entry.member[port_no] = VTSS_PORT_BF_GET(aggrglb.aggr_config_stack.switch_id[isid - VTSS_ISID_START][aggr_no - AGGR_MGMT_GROUP_NO_START].member, (port_no - VTSS_PORT_NO_START));

                if (members->entry.member[port_no]) {
                    rc = VTSS_RC_OK;
                }
            }
            AGGR_CRIT_EXIT();
        } else {
            return AGGR_ERROR_PARM;
        }
    }
    return rc;
}

/****************************************************************************/
// Get the speed of the group
/****************************************************************************/
vtss_port_speed_t aggr_mgmt_speed_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no)
{
    if (!AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
        T_E("Illegal aggr_no:%d", aggr_no);
        return VTSS_SPEED_UNDEFINED;
    }
    return get_aggrglb_spd(aggr_no);
}

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
vtss_rc aggr_mgmt_lacp_members_add(uint aid, int new_port)
{
    aggr_mgmt_group_no_t     aggr_no = 0;
    int                      start_group, llag = 0, member_cnt = 0;
    vtss_isid_t              isid;
    vtss_port_no_t           isid_port, port_no;
    aggr_mgmt_group_t        entry;
    u32                      port_count;

    memset(&entry, 0, sizeof(entry));
    T_D("aggr_mgmt_lacp_members_add.  Aid:%d port:%d", aid, new_port);

    /* Port to join a aggregation must wait until another port joins before the group is created */
    /* When 2 port are ready to join, a group is created with the first vacant Aggr id available */

    /* Convert new_port to isid,port */
    if (!l2port2port(new_port, &isid, &isid_port)) {
        T_E("Could not find a isid,port for l2_proto_port_t:%d\n", new_port);
        return AGGR_ERROR_PARM;
    }
    port_count = port_isid_port_count(isid);
    AGGR_CRIT_ENTER();
    if (!aggrglb.aggr_lacp[aid].aggr_no) {
        /* First port to join. Find the real aggr_id when the next port joins. */
        aggrglb.aggr_lacp[aid].aggr_no = LACP_ONLY_ONE_MEMBER;
        aggrglb.aggr_lacp[aid].members[new_port] = 1;
        AGGR_CRIT_EXIT();
        return VTSS_RC_OK;
    } else if (aggrglb.aggr_lacp[aid].aggr_no == LACP_ONLY_ONE_MEMBER) {
        /* Second port to join. Find the aggregation id and create the aggregation */
        T_D("Creating aggregation with 2 ports at aid:%d\n", aid);

        /* Check if the 2 ports are within the same ISID */
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            l2_port_no_t l2port;
            if (port_isid_port_no_is_stack(isid, port_no)) {
                continue;
            }
            l2port = L2PORT2PORT(isid, port_no);
            if (aggrglb.aggr_lacp[aid].members[l2port]) {
                llag = 1;    /* Found the port, create a llag */
            }
        }

        if (!llag) {
            start_group = AGGR_MGMT_GLAG_START;
        } else {
            start_group = AGGR_MGMT_GROUP_NO_START;
        }

    } else {
        /* New port wants to join an existing aggregation */
        /* This could mean that we have to  move LLAG to GLAG. */
        /* If the new port is within a different switch than the existing ports */
        /* the LLAG will be deleted and GLAG created - if there is a vacant GLAG  */
        l2port_iter_t l2pit;

        (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
        while (l2port_iter_getnext(&l2pit)) {
            if (aggrglb.aggr_lacp[aid].members[l2pit.l2port]) {
                member_cnt++;
            }
        }

        if (AGGR_MGMT_GROUP_IS_LAG(aggrglb.aggr_lacp[aid].aggr_no)) {
            if (member_cnt >= AGGR_MGMT_LAG_PORTS_MAX) {
                T_W("There can only be %d ports in a LLAG", AGGR_MGMT_LAG_PORTS_MAX);
                AGGR_CRIT_EXIT();
                return AGGR_ERROR_REG_TABLE_FULL;
            }
        } else {
            if (member_cnt >= AGGR_MGMT_GLAG_PORTS_MAX) {
                T_W("There can only be %d ports in a GLAG", AGGR_MGMT_GLAG_PORTS_MAX);
                AGGR_CRIT_EXIT();
                return AGGR_ERROR_REG_TABLE_FULL;
            }
        }

        start_group = AGGR_MGMT_GROUP_NO_END; /* Disable new group search */
        if (AGGR_MGMT_GROUP_IS_LAG(aggrglb.aggr_lacp[aid].aggr_no)) {
            /* Check wether the new member is within the same ISID as the existing members */
            (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
            while (l2port_iter_getnext(&l2pit)) {
                if (aggrglb.aggr_lacp[aid].members[l2pit.l2port]) {
                    if (l2pit.isid != isid) {

                        AGGR_CRIT_EXIT();
                        if (!aggr_find_group(VTSS_ISID_LOCAL, AGGR_MGMT_GLAG_START)) {
                            T_W("All GLAGs in use!");
                            return AGGR_ERROR_REG_TABLE_FULL;
                        }
                        if (member_cnt >= AGGR_MGMT_GLAG_PORTS_MAX) {
                            T_W("There can only be %d ports in a GLAG", AGGR_MGMT_GLAG_PORTS_MAX);
                            return AGGR_ERROR_REG_TABLE_FULL;
                        }
                        AGGR_CRIT_ENTER();
                        aggrglb.aggr_lacp[aid].members[new_port] = 1;
                        T_D("Deleting LLAG id %u and creating a GLAG instead.\n", aggrglb.aggr_lacp[aid].aggr_no);
                        /* Delete the LAG */
                        (void)aggr_group_del(l2pit.isid, aggrglb.aggr_lacp[aid].aggr_no);

                        /* Inform subscribers of aggregation changes */
                        (void)aggr_change_callback(l2pit.isid, aggrglb.aggr_lacp[aid].aggr_no);

                        /* Find a GLAG id and create the GLAG */
                        start_group = AGGR_MGMT_GLAG_START;
                    }
                    break;
                }
            }
        }
        aggrglb.aggr_lacp[aid].members[new_port] = 1;
    }
    AGGR_CRIT_EXIT();
    if (start_group != AGGR_MGMT_GROUP_NO_END) {
        aggr_no = aggr_find_group(isid, start_group);
        if (aggr_no == 0) {
            T_W("All %s in use!", (start_group == AGGR_MGMT_GLAG_START) ? "GLAGs" : "LLAGs and GLAGs");
            return AGGR_ERROR_REG_TABLE_FULL;
        } else {
            AGGR_CRIT_ENTER();
            aggrglb.aggr_lacp[aid].aggr_no =  aggr_no;
            aggrglb.aggr_lacp[aid].members[new_port] = 1;
            AGGR_CRIT_EXIT();
        }
    }

    T_D("Using aggr no:%u for aid:%d\n", aggr_no, aid);
    AGGR_CRIT_ENTER();
    switch_iter_t sit;
    /* If the group is new and is a GLAG then we have to go through all members of all ISIDs */
    if (start_group == AGGR_MGMT_GLAG_START) {
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    } else {
        /* Add a new port to an existing aggr group (lag or glag) */
        (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    }
    while (switch_iter_getnext(&sit)) {
        l2port_iter_t l2unit;
        /* For this isid, setup all ports being members of aggegation */
        (void)l2port_iter_init(&l2unit, sit.isid, L2PORT_ITER_TYPE_PHYS);
        memset(entry.member, 0, sizeof(entry.member));
        while (l2port_iter_getnext(&l2unit)) {
            entry.member[l2unit.iport] = aggrglb.aggr_lacp[aid].members[l2unit.l2port];
            T_N("l2port %d: ISID %d Port %d member of aggr %d = %d", l2unit.l2port, l2unit.isid, l2unit.iport, aid, entry.member[l2unit.iport]);
        }
        /* Apply the aggregation to the chip */
        AGGR_CRIT_EXIT();
        if (aggr_group_add(sit.isid, aggrglb.aggr_lacp[aid].aggr_no, &entry) != VTSS_RC_OK) { /* lacp member add */
            return VTSS_UNSPECIFIED_ERROR;
        }
        AGGR_CRIT_ENTER();
    }
    AGGR_CRIT_EXIT();
    T_D("Added isid:%d, port:%u to aggr id %u\n", isid, isid_port, aggrglb.aggr_lacp[aid].aggr_no);
    /* Inform subscribers of aggregation changes */
    (void)aggr_change_callback(isid, aggrglb.aggr_lacp[aid].aggr_no);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_LACP */

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
void aggr_mgmt_lacp_member_del(uint aid, int old_port)
{
    bool empty = 1;
    int                  member_cnt = 0, last_port = 0;
    vtss_port_no_t       isid_port, port_no;
    vtss_isid_t          isid, isid_cmp = 0;
    l2_port_no_t         l2port;
    l2port_iter_t        l2pit;
    aggr_mgmt_group_t    entry;
    BOOL                 first = 1, del_glag = TRUE;
    aggr_mgmt_group_no_t aggr_no, tmp_no;
    u32                  port_count;

    memset(&entry, 0, sizeof(entry));
    T_D("aggr_mgmt_lacp_members_del.  Aid:%d port:%d", aid, old_port);

    if (!l2port2port(old_port, &isid, &isid_port)) {
        T_E("Could not find a isid,port for l2_proto_port_t:%d\n", old_port);
        return;
    }
    AGGR_CRIT_ENTER();
    port_count = port_isid_port_count(isid);
    aggrglb.aggr_lacp[aid].members[old_port] = 0;

    if (aggrglb.aggr_lacp[aid].aggr_no == LACP_ONLY_ONE_MEMBER) {
        aggrglb.aggr_lacp[aid].aggr_no = 0;
        AGGR_CRIT_EXIT();
        return;
    }

    if (!AGGR_MGMT_GROUP_IS_AGGR(aggrglb.aggr_lacp[aid].aggr_no)) {
        T_E("%u is not a legal aggregation group.", aggrglb.aggr_lacp[aid].aggr_no);
        AGGR_CRIT_EXIT();
        return;
    }

    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
    while (l2port_iter_getnext(&l2pit)) {
        if (aggrglb.aggr_lacp[aid].members[l2pit.l2port]) {
            empty = 0; /* Group not empty */
            member_cnt++;
            last_port = l2pit.l2port;
        }
    }

    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (port_isid_port_no_is_stack(isid, port_no)) {
            continue;
        }
        l2port = L2PORT2PORT(isid, port_no);
        entry.member[port_no] = aggrglb.aggr_lacp[aid].members[l2port];
    }

    AGGR_CRIT_EXIT();
    if (!empty) {
        /* Modify the aggregation */
        if (aggr_group_add(isid, aggrglb.aggr_lacp[aid].aggr_no, &entry) != VTSS_RC_OK) {
            return;
        }
    } else {
        /* Remove the aggregation */
        (void)aggr_group_del(isid, aggrglb.aggr_lacp[aid].aggr_no);
    }
    AGGR_CRIT_ENTER();
    T_D("Removed port %u from aggr id %u\n", isid_port, aggrglb.aggr_lacp[aid].aggr_no);

    tmp_no = aggrglb.aggr_lacp[aid].aggr_no;
    AGGR_CRIT_EXIT();
    /* Check if there is only one port left int the group.  If that is, remove the group */
    if (member_cnt == 1) {
        /* Find the last port ISID */
        if (!l2port2port(last_port, &isid, &isid_port)) {
            T_E("Could not find a isid,port for l2_proto_port_t:%d\n", old_port);
            return;
        }
        T_D("Only one port:%d is left.  Removing the aggregation:%d but keeping the last port on 'stand by'", last_port, aid);
        (void)aggr_group_del(isid, aggrglb.aggr_lacp[aid].aggr_no);
        /* LACP considers the last port to be member though   */
        AGGR_CRIT_ENTER();
        aggrglb.aggr_lacp[aid].aggr_no = LACP_ONLY_ONE_MEMBER;
        aggrglb.aggr_lacp[aid].members[last_port] = 1;
        AGGR_CRIT_EXIT();
    }

    AGGR_CRIT_ENTER();
    if (empty) {
        T_D("aggr id %u disabled\n", aggrglb.aggr_lacp[aid].aggr_no);
        aggrglb.aggr_lacp[aid].aggr_no = 0;
    } else if (member_cnt == 1) {
        /* Do nothing  */
    } else {

        /* A port is now removed from the AGGR. */
        /* If the AGGR is a GLAG, check if the rest of the ports are connected to the same switch i.e. same isid. */
        /* If they are, change the GLAG to a LLAG  */

        if (AGGR_MGMT_GROUP_IS_GLAG(aggrglb.aggr_lacp[aid].aggr_no)) {

            (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
            while (l2port_iter_getnext(&l2pit)) {
                if (aggrglb.aggr_lacp[aid].members[l2pit.l2port]) {

                    if (!l2port2port(l2pit.l2port, &isid, &port_no)) {
                        T_W("Could not convert l2port\n");
                        AGGR_CRIT_EXIT();
                        return;
                    }

                    if (first) {
                        isid_cmp = isid;
                        first = 0;
                    } else {
                        if (isid_cmp != isid) {
                            /* Keep the GLAG */
                            del_glag = FALSE;
                            break;
                        }
                    }
                }
            }

            if (del_glag) {
                T_D("Deleting GLAG id %u and creating a LLAG instead.\n", aggrglb.aggr_lacp[aid].aggr_no);
                AGGR_CRIT_EXIT();
                aggr_no = aggr_find_group(VTSS_ISID_LOCAL, AGGR_MGMT_GROUP_NO_START);
                AGGR_CRIT_ENTER();

                if (!AGGR_MGMT_GROUP_IS_LAG(aggr_no)) {
                    AGGR_CRIT_EXIT();
                    return; /* No LLAG vacancy - stick with the GLAG */
                }

                /* Delete the GLAG from all ISID's that have members */
                isid_cmp = 0;
                (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
                while (l2port_iter_getnext(&l2pit)) {
                    if (aggrglb.aggr_lacp[aid].members[l2pit.l2port]) {
                        if (!l2port2port(l2pit.l2port, &isid, &port_no)) {
                            T_W("Could not convert l2port\n");
                            AGGR_CRIT_EXIT();
                            return;
                        }
                        if (isid != isid_cmp) {
                            (void)aggr_group_del(isid, aggrglb.aggr_lacp[aid].aggr_no);
                            /* Inform subscribers of aggregation changes */
                            (void)aggr_change_callback(isid, aggrglb.aggr_lacp[aid].aggr_no);
                            isid_cmp = isid;
                        }
                    }
                }

                if (!VTSS_ISID_LEGAL(isid)) {
                    AGGR_CRIT_EXIT();
                    return;
                }

                /* Use the new LLAG id */
                aggrglb.aggr_lacp[aid].aggr_no = aggr_no;

                for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                    if (port_isid_port_no_is_stack(isid, port_no)) {
                        continue;
                    }

                    l2port = L2PORT2PORT(isid, port_no);
                    entry.member[port_no] = aggrglb.aggr_lacp[aid].members[l2port];
                }

                /* Apply the new aggregation to the chip */
                if (aggr_group_add(isid, aggrglb.aggr_lacp[aid].aggr_no, &entry) != VTSS_RC_OK) {
                    AGGR_CRIT_EXIT();
                    return;
                }

                /* Inform subscribers of aggregation changes */
                (void)aggr_change_callback(isid, aggrglb.aggr_lacp[aid].aggr_no);
            }
        }
    }
    AGGR_CRIT_EXIT();
    /* Inform subscribers of aggregation changes */
    (void)aggr_change_callback(isid, tmp_no);
}
#endif /* VTSS_SW_OPTION_LACP */

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
vtss_rc aggr_mgmt_lacp_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_member_t *members, BOOL next)
{
    vtss_rc            rc = AGGR_ERROR_ENTRY_NOT_FOUND;
    vtss_port_no_t     port_no = 0;
    BOOL               found_group = false;
    u32                port_count = port_isid_port_count(isid);
    aggr_mgmt_group_t  local_members;

    VTSS_RC(aggr_isid_port_valid(isid, VTSS_PORT_NO_START, 1, FALSE /* Get command, VTSS_ISID_LOCAL OK */));

    memset(&local_members, 0, sizeof(local_members));
    memset(members, 0, sizeof(aggr_mgmt_group_member_t));
    /* Get the first active aggregation group */
    if (aggr_no == 0 && next) {
        for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
            if ((found_group = aggr_group_exists(isid, aggr_no, &local_members) == 1)) {
                rc = VTSS_RC_OK;
                break;
            }
        }
        if (!found_group) {
            return AGGR_ERROR_ENTRY_NOT_FOUND;
        }
    } else if (aggr_no != 0 && next) {
        /* Get the next active aggregation group */
        for (aggr_no = (aggr_no + 1); aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
            if ((found_group = aggr_group_exists(isid, aggr_no, &local_members) == 1)) {
                rc = VTSS_RC_OK;
                break;
            }
        }
        if (!found_group) {
            return AGGR_ERROR_ENTRY_NOT_FOUND;
        }
    }

    members->aggr_no = aggr_no;
    /* Check if the group is known */
    if (!next) {
        if (aggr_group_exists(isid, aggr_no, &local_members)) {
            /* Its there */
            rc = VTSS_RC_OK;
        }
    }
    /* Return the portlist (empty or not)  */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        members->entry.member[port_no] = local_members.member[port_no];
    }

    return rc;
}
#endif /* VTSS_SW_OPTION_LACP */

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
aggr_mgmt_group_no_t lacp_to_aggr_id(int aid)
{
    aggr_mgmt_group_no_t aggr;
    AGGR_CRIT_ENTER();
    aggr = aggrglb.aggr_lacp[aid].aggr_no;
    AGGR_CRIT_EXIT();
    return aggr;
}
#endif /* VTSS_SW_OPTION_LACP */

#ifdef VTSS_SW_OPTION_LACP
/****************************************************************************/
/****************************************************************************/
vtss_rc aggr_mgmt_lacp_id_get_next(int *search_aid, int *return_aid)
{
    int aid, search;
    if (search_aid == NULL) {
        search = 0;
    } else {
        search = *search_aid;
        search++;
    }

    AGGR_CRIT_ENTER();
    for (aid = search; aid < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); aid++) {
        if (aggrglb.aggr_lacp[aid].aggr_no != 0) {
            *return_aid = aid;
            break;
        }
    }
    AGGR_CRIT_EXIT();
    if (aid >= (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START)) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_LACP */

/****************************************************************************/
// Get members in a given aggr group for both LACP and STATIC.
// 'Next' is used to browse through active groups.
// Note! Only members with portlink will be returned.
// If less than 2 members are found, 'AGGR_ERROR_ENTRY_NOT_FOUND' will be returned.
// Runtime version. See aggr_mgmt_port_members_get() for static configuration
// version.
/****************************************************************************/
vtss_rc aggr_mgmt_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no,  aggr_mgmt_group_member_t *members, BOOL next)
{
    vtss_port_no_t port_no;
    u32            port_count = port_isid_port_count(isid);

    VTSS_RC(aggr_isid_port_valid(isid, VTSS_PORT_NO_START, 1, FALSE /* Get command, VTSS_ISID_LOCAL OK */));
    T_N("Enter aggr_mgmt_members_get (Static and LACP get) isid:%d aggr_no:%u", isid, aggr_no);

    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        if (get_aggrglb_active(isid, port_no) == aggr_no) {
            members->entry.member[port_no] = 1;
        } else {
            members->entry.member[port_no] = 0;
        }
    }
    if (total_active_ports(aggr_no) > 1) {
        return VTSS_RC_OK;
    }
    memset(members, 0, sizeof(*members));
    return AGGR_ERROR_ENTRY_NOT_FOUND;
}

/****************************************************************************/
// Returns the aggr number for a port.  Returns 0 if the port is not a member or if the link is down.
/****************************************************************************/
aggr_mgmt_group_no_t aggr_mgmt_get_aggr_id(vtss_isid_t isid, vtss_port_no_t port_no)
{
    aggr_mgmt_group_member_t members;
    aggr_mgmt_group_no_t aggr_no;

    for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
        if (aggr_mgmt_members_get(isid, aggr_no,  &members, 0) == VTSS_RC_OK) {
            if (members.entry.member[port_no]) {
                return aggr_no;
            }
        }
    }

    return 0;
}

/****************************************************************************/
/****************************************************************************/
aggr_mgmt_group_no_t aggr_mgmt_get_port_aggr_id(vtss_isid_t isid, vtss_port_no_t port_no)
{
    aggr_mgmt_group_member_t members;
    aggr_mgmt_group_no_t aggr_no;

    for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
        if (aggr_mgmt_port_members_get(isid, aggr_no,  &members, 0) == VTSS_RC_OK) {
            if (members.entry.member[port_no]) {
                return aggr_no;
            }
        }
    }

    return 0;
}

/****************************************************************************/
// Returns information if the port is participating in LACP or Static aggregation
// 0 = No participation
// 1 = Static aggregation participation
// 2 = LACP aggregation participation
/****************************************************************************/
int aggr_mgmt_port_participation(vtss_isid_t isid, vtss_port_no_t port_no)
{
    aggr_mgmt_group_member_t members;
    aggr_mgmt_group_no_t aggr_no;

#ifdef VTSS_SW_OPTION_LACP
    vtss_lacp_port_config_t  conf;

    /* Check if the group or ports is occupied by LACP in this isid */
    if  (lacp_mgmt_port_conf_get(isid, port_no, &conf) == VTSS_RC_OK) {
        if (conf.enable_lacp) {
            return 2;
        }
    }
#endif

    /* Check if the group or ports is occupied by Static aggr in this isid */
    for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
        if (aggr_mgmt_port_members_get(isid, aggr_no, &members, 0) == VTSS_RC_OK) {
            if (members.entry.member[port_no]) {
                return 1;
            }
        }
    }

    return 0;
}

/****************************************************************************/
// Sets the aggregation mode. The mode is used by all the aggregation groups
/****************************************************************************/
vtss_rc aggr_mgmt_aggr_mode_set(vtss_aggr_mode_t *mode)
{
    switch_iter_t          sit;

    if (!msg_switch_is_master()) {
        T_W("not master");
        return AGGR_ERROR_STACK_STATE;
    }

    /* Add to local and stack config */
    AGGR_CRIT_ENTER();
    aggrglb.aggr_config.mode = *mode;
    aggrglb.aggr_config_stack.mode = *mode;
    AGGR_CRIT_EXIT();

    // Loop over existing switches and send them the new configuration.
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        aggr_stack_mode_set(sit.isid, mode);
    }

    (void)aggr_save_to_flash();
    return VTSS_RC_OK;
}

/****************************************************************************/
// Gets the aggregation mode.  The 'mode' points to the updated mode type
/****************************************************************************/
vtss_rc aggr_mgmt_aggr_mode_get(vtss_aggr_mode_t *mode)
{
    /* Get from local config */
    AGGR_CRIT_ENTER();
    *mode = aggrglb.aggr_config.mode;
    AGGR_CRIT_EXIT();
    return VTSS_RC_OK;
}

/****************************************************************************/
// Registration for callbacks if aggregation changes
/****************************************************************************/
void aggr_change_register(aggr_change_callback_t cb)
{
    VTSS_ASSERT(aggrglb.aggr_callbacks < ARRSZ(aggrglb.callback_list));
    cyg_scheduler_lock();
    aggrglb.callback_list[aggrglb.aggr_callbacks++] = cb;
    cyg_scheduler_unlock();
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
vtss_rc aggr_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    vtss_vstax_upsid_t upsid;
    vtss_port_no_t port_no;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initilize the module */
        aggr_start(1);
#ifdef VTSS_SW_OPTION_CLI
        aggr_cli_req_init();
#endif
        break;

    case INIT_CMD_START:
#ifdef VTSS_SW_OPTION_ICFG
        if (aggr_icfg_init() != VTSS_RC_OK) {
            T_D("Calling aggr_icfg_init() failed");
        }
#endif
        aggr_start(0);
        break;

    case INIT_CMD_CONF_DEF:
        T_D("INIT_CMD_CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset configuration to default */
            aggr_conf_stack_read(TRUE);
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        aggr_conf_stack_read(FALSE);
        break;

    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        // Get the logical-to-physical port map from the #data argument to this function.
        if (sizeof(aggrglb.port_map_global[isid]) != sizeof(data->port_map)) {
            T_E("%u vs. %u", sizeof(aggrglb.port_map_global[isid]), sizeof(data->port_map));
        }

        memcpy(aggrglb.port_map_global[isid], data->port_map, sizeof(aggrglb.port_map_global[isid]));

        /* Store the upsid for later use */
        if ((upsid = topo_isid_port2upsid(isid, VTSS_PORT_NO_START)) == VTSS_VSTAX_UPSID_UNDEF) {
            aggrglb.upsid_table[isid].upsid[0] = upsid;
            T_D("Could not get UPSID from Topo during Switch ADD, isid: %d", isid);
            break;
        }
        aggrglb.upsid_table[isid].upsid[0] = upsid;
        aggrglb.upsid_table[isid].upsid[1] = VTSS_VSTAX_UPSID_UNDEF;
        T_D("Switch ADD, isid: %d upsid:%d\n", isid, aggrglb.upsid_table[isid].upsid[0]);

        for (port_no = VTSS_PORT_NO_START + 1; (port_no < data->switch_info[isid].port_cnt) && (port_no != data->switch_info[isid].stack_ports[0]); port_no++) {
            if ((upsid = topo_isid_port2upsid(isid, port_no)) == VTSS_VSTAX_UPSID_UNDEF) {
                T_D("Could not get UPSID from Topo during Switch ADD, isid: %d", isid);
                break;
            }
            if (aggrglb.upsid_table[isid].upsid[0] != upsid) {
                aggrglb.upsid_table[isid].upsid[1] = upsid;
                break;
            }
        }
#endif /* (VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE */

        /* Configure the new switch */
        aggr_switch_add(isid);
        break;

    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        aggr_switch_del(isid);
#endif
        break;

    default:
        break;
    }

    T_D("exit");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/


