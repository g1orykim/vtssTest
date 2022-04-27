/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "port_api.h"
#include "critd_api.h"

#include "misc_api.h"
#include "mgmt_api.h"

#include "vtss_mvr.h"

/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */
#define MVR_MAX_FLOODING_COUNT              MVR_NUM_OF_SUPPORTED_INTF


/* ****************************************************************************
 *
 *
 * Public data
 *
 *
 * ***************************************************************************/


/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 *
 * ************************************************************************ */
#define MVR_PORT_PROFILE_GET(w, x, y, z)    do {        \
    memset((w), 0x0, sizeof(specific_grps_fltr_t));     \
    IPMC_LIB_ADRS_CPY(&(w)->src, &(x)->src_ip_addr);    \
    IPMC_LIB_ADRS_CPY(&(w)->dst, &(x)->group_addr);     \
    (w)->port = (z);                                    \
    mvr_specific_port_fltr_set((y), (w));               \
} while (0)

static vtss_mvr_global_t        mvr_global;
static BOOL vtss_mvr_global_done_init = FALSE;
static ipmc_db_ctrl_hdr_t       vtss_mvr_mgroup_entry_list;
static BOOL vtss_mvr_mgroup_entry_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t       vtss_mvr_intf_entry_list;
static BOOL vtss_mvr_intf_entry_list_created_done = FALSE;

/* For Group Timer List */
static ipmc_db_ctrl_hdr_t       vtss_mvr_grp_rxmt_tmr_list;
static BOOL vtss_mvr_grp_rxmt_tmr_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t       vtss_mvr_grp_fltr_tmr_list;
static BOOL vtss_mvr_grp_fltr_tmr_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t       vtss_mvr_grp_srct_tmr_list;
static BOOL vtss_mvr_grp_srct_tmr_list_created_done = FALSE;

static ipmc_db_ctrl_hdr_t       *mvr_intf_tree = &vtss_mvr_intf_entry_list;
static ipmc_db_ctrl_hdr_t       *mvr_mgrp_tree = &vtss_mvr_mgroup_entry_list;
static ipmc_db_ctrl_hdr_t       *mvr_rxmt_tree = &vtss_mvr_grp_rxmt_tmr_list;
static ipmc_db_ctrl_hdr_t       *mvr_fltr_tree = &vtss_mvr_grp_fltr_tmr_list;
static ipmc_db_ctrl_hdr_t       *mvr_srct_tree = &vtss_mvr_grp_srct_tmr_list;

static u16                      mvr_query_suppression_timer;
static u16                      mvr_query_flooding_cnt;
static int                      mvr_stp_port_status[VTSS_PORT_ARRAY_SIZE];


static i32 vtss_mvr_mgroup_entry_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_entry_t  *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("MVR_ASSERT(vtss_mvr_mgroup_entry_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_group_entry_t *)elm1;
    element2 = (ipmc_group_entry_t *)elm2;
    if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
        return -1;
    } else if (element1->ipmc_version > element2->ipmc_version) {
        return 1;
    } else if (element1->ipmc_version < element2->ipmc_version) {
        return -1;
    } else {
        int cmp;

        if (element1->ipmc_version == IPMC_IP_VERSION_IGMP) {
            cmp = IPMC_LIB_ADRS_CMP4(element1->group_addr, element2->group_addr);
        } else {
            cmp = IPMC_LIB_ADRS_CMP6(element1->group_addr, element2->group_addr);
        }

        if (cmp > 0) {
            return 1;
        } else if (cmp < 0) {
            return -1;
        } else {
            return 0;
        }
    }
}

static i32 vtss_ipmcmvr_grp_rxmt_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_info_t   *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcmvr_grp_rxmt_tmr_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_group_info_t *)elm1;
    element2 = (ipmc_group_info_t *)elm2;

    tmr1 = &element1->min_tmr;
    tmr2 = &element2->min_tmr;
    if (IPMC_TIMER_GREATER(tmr1, tmr2)) {
        return 1;
    } else if (IPMC_TIMER_LESS(tmr1, tmr2)) {
        return -1;
    } else {
        return vtss_mvr_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);
    }
}

static i32 vtss_ipmcmvr_grp_fltr_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_db_t     *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcmvr_grp_fltr_tmr_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_group_db_t *)elm1;
    element2 = (ipmc_group_db_t *)elm2;

    tmr1 = &element1->min_tmr;
    tmr2 = &element2->min_tmr;
    if (IPMC_TIMER_GREATER(tmr1, tmr2)) {
        return 1;
    } else if (IPMC_TIMER_LESS(tmr1, tmr2)) {
        return -1;
    } else {
        return vtss_mvr_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);
    }
}

static i32 vtss_ipmcmvr_grp_srct_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_sfm_srclist_t  *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcmvr_grp_srct_tmr_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_sfm_srclist_t *)elm1;
    element2 = (ipmc_sfm_srclist_t *)elm2;

    tmr1 = &element1->min_tmr;
    tmr2 = &element2->min_tmr;
    if (IPMC_TIMER_GREATER(tmr1, tmr2)) {
        return 1;
    } else if (IPMC_TIMER_LESS(tmr1, tmr2)) {
        return -1;
    } else {
        int grp_cmp = vtss_mvr_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);
        if (grp_cmp > 0) {
            return 1;
        } else if (grp_cmp < 0) {
            return -1;
        } else {
            return ipmc_lib_srclist_cmp_func(elm1, elm2);
        }
    }
}

static i32 vtss_mvr_intf_entry_cmp_func(void *elm1, void *elm2)
{
    vtss_mvr_interface_t    *element1, *element2;
    ipmc_intf_entry_t       *intf1, *intf2;

    if (!elm1 || !elm2) {
        T_W("MVR_ASSERT(vtss_mvr_intf_entry_cmp_func)");
        for (;;) {}
    }

    element1 = (vtss_mvr_interface_t *)elm1;
    element2 = (vtss_mvr_interface_t *)elm2;
    intf1 = &element1->basic;
    intf2 = &element2->basic;

    if (intf1->param.vid > intf2->param.vid) {
        return 1;
    } else if (intf1->param.vid < intf2->param.vid) {
        return -1;
    } else if (intf1->ipmc_version > intf2->ipmc_version) {
        return 1;
    } else if (intf1->ipmc_version < intf2->ipmc_version) {
        return -1;
    } else {
        return 0;
    }
}

static void vtss_mvr_get_router_port_mask(u32 port_no, ipmc_port_bfs_t *port_mask)
{
    port_iter_t pit;
    char        mvrBufPort[MGMT_PORT_BUF_SIZE];
    BOOL        mvrFwdMap[VTSS_PORT_ARRAY_SIZE];
    u32         i;

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        if (i != port_no) {
            VTSS_PORT_BF_SET(port_mask->member_ports,
                             i,
                             VTSS_PORT_BF_GET(mvr_global.router_port, i));
        } else {
            VTSS_PORT_BF_SET(port_mask->member_ports, i, FALSE);
        }
    }

    memset(mvrFwdMap, 0x0, sizeof(mvrFwdMap));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        if (VTSS_PORT_BF_GET(port_mask->member_ports, i)) {
            mvrFwdMap[i] = TRUE;
        }
    }
    T_D("Router Ports=%s", mgmt_iport_list2txt(mvrFwdMap, mvrBufPort));
}

static void mvr_del_group_member_with_lnk(ipmc_ip_version_t version, u8 idx)
{
    ipmc_group_entry_t      *grp, *grp_ptr;
    ipmc_group_db_t         *grp_db;
    ipmc_bf_status          bfs;
    ipmc_time_t             mvrtmr_with_lnk;
    vtss_mvr_interface_t    intf;
    ipmc_intf_entry_t       *entry;
    BOOL                    intf_found;

    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(mvr_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if ((version != IPMC_IP_VERSION_ALL) && (grp->ipmc_version != version)) {
            continue;
        }

        grp_db = &grp->info->db;
        if (!VTSS_PORT_BF_GET(grp_db->port_mask, idx)) {
            continue;
        }

        VTSS_PORT_BF_SET(grp_db->port_mask, idx, FALSE);
        VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, idx, VTSS_IPMC_SF_STATUS_DISABLED);

        memset(&intf, 0x0, sizeof(vtss_mvr_interface_t));
        entry = &intf.basic;
        entry->ipmc_version = grp->ipmc_version;
        entry->param.vid = grp->vid;
        intf_found = (vtss_mvr_interface_get(&intf) == VTSS_OK);
        /* delete entry if no more ports */
        bfs = ipmc_lib_bf_status_check(grp_db->port_mask);
        if (bfs == IPMC_BF_EMPTY) {
            if (intf_found) {
                (void) ipmc_lib_group_delete(entry,
                                             mvr_mgrp_tree,
                                             mvr_rxmt_tree,
                                             mvr_fltr_tree,
                                             mvr_srct_tree,
                                             grp,
                                             FALSE,
                                             FALSE);
            } else {
                (void) ipmc_lib_group_delete(NULL,
                                             mvr_mgrp_tree,
                                             mvr_rxmt_tree,
                                             mvr_fltr_tree,
                                             mvr_srct_tree,
                                             grp,
                                             FALSE,
                                             FALSE);
            }
        } else {
            port_iter_t pit;
            u32         i;
            ipmc_time_t *llqt = &mvrtmr_with_lnk;

            IPMC_TIMER_LLQT_GET(entry, llqt);

            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(grp_db->port_mask, i) && IPMC_TIMER_LESS(&grp_db->tmr.fltr_timer.t[i], llqt)) {
                    VTSS_PORT_BF_SET(grp_db->port_mask, i, FALSE);
                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, i, VTSS_IPMC_SF_STATUS_DISABLED);
                }
            }

            /* replace the MAC entry (if different) */
            (void) ipmc_lib_group_update(mvr_mgrp_tree, grp);

            if (bfs == IPMC_BF_SEMIEMPTY) {
                /* TO-DO */
            }
        }
    }
}

#if 0 /* etliang */
static void mvr_group_garbage_collection(void)
{
    ipmc_group_entry_t  *grp, *grp_ptr;
    ipmc_group_db_t     *grp_db;

    /* we must go through all groups */
    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(mvr_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        grp_db = &grp->info->db;
        /* delete entry if no more ports */
        if (IPMC_LIB_BFS_EMPTY(grp_db->port_mask)) {
            (void) ipmc_lib_group_delete(grp->info->interface,
                                         mvr_mgrp_tree,
                                         mvr_rxmt_tree,
                                         mvr_fltr_tree,
                                         mvr_srct_tree,
                                         grp,
                                         FALSE,
                                         FALSE);
        } else {
            /* replace the MAC entry (if different) */
            (void) ipmc_lib_group_update(mvr_mgrp_tree, grp);
        }
    }
}
#endif /* etliang */

static BOOL _mvr_interface_add(vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t            *entry;
    ipmc_intf_entry_t               *intf;
    ipmc_prot_intf_entry_param_t    *param;
    BOOL                            retVal;
    u16                             vid;
    u8                              vlan_ports[VTSS_BF_SIZE(VTSS_PORT_ARRAY_SIZE)];
    ipmc_ip_version_t               version;

    if (!mvr_intf) {
        return FALSE;
    }
    if (!IPMC_MEM_SYSTEM_MTAKE(entry, sizeof(vtss_mvr_interface_t))) {
        return FALSE;
    }

    /* Leave INDEX & VLAN-Map alone */
    vid = mvr_intf->basic.param.vid;
    version = mvr_intf->basic.ipmc_version;
    memcpy(vlan_ports, mvr_intf->basic.vlan_ports, sizeof(vlan_ports));

    memset(entry, 0x0, sizeof(vtss_mvr_interface_t));

    retVal = TRUE;
    intf = &entry->basic;
    param = &intf->param;

    intf->op_state = TRUE;
    param->vid = vid;
    intf->ipmc_version = version;

    memcpy(entry->name, mvr_intf->name, sizeof(entry->name));
    entry->mode = mvr_intf->mode;
    memcpy(entry->ports, mvr_intf->ports, sizeof(entry->ports));
    memcpy(intf->vlan_ports, vlan_ports, sizeof(intf->vlan_ports));
    param->mvr = VTSS_IPMC_TRUE;
    param->priority = entry->priority = mvr_intf->priority;
    param->vtag = entry->vtag = mvr_intf->vtag;
    entry->profile_index = mvr_intf->profile_index;
    IPMC_LIB_ADRS_SET(&param->active_querier, 0x0);

    if (version == IPMC_IP_VERSION_IGMP) {
        param->querier.QuerierAdrs4 = mvr_intf->basic.param.querier.QuerierAdrs4;
    } else {
        param->querier.QuerierAdrs4 = MVR_QUERIER_ADDRESS4;
    }
    param->querier.state = IPMC_QUERIER_IDLE;
    param->querier.timeout = IPMC_TIMER_QI(intf);
    param->querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(intf);
    param->querier.RobustVari = MVR_QUERIER_ROBUST_VARIABLE;
    param->querier.QueryIntvl = MVR_QUERIER_QUERY_INTERVAL;
    param->querier.MaxResTime = MVR_QUERIER_MAX_RESP_TIME;
    param->querier.LastQryItv = mvr_intf->basic.param.querier.LastQryItv;
    param->querier.LastQryCnt = param->querier.RobustVari;
    param->querier.UnsolicitR = MVR_QUERIER_UNSOLICIT_REPORT;

    param->cfg_compatibility = IPMC_PARAM_DEF_COMPAT;
    param->rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
    param->hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;

    if (!IPMC_LIB_DB_SET(mvr_intf_tree, entry)) {
        IPMC_MEM_SYSTEM_MGIVE(entry);
        retVal = FALSE;
    }

    return retVal;
}

static BOOL _mvr_interface_del(vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t    *mvif;

    if (!mvr_intf) {
        return FALSE;
    }

    mvif = mvr_intf;
    if (IPMC_LIB_DB_GET(mvr_intf_tree, mvif)) {
        if (!IPMC_LIB_DB_DEL(mvr_intf_tree, mvif)) {
            return FALSE;
        } else {
            IPMC_MEM_SYSTEM_MGIVE(mvif);
        }
    } else {
        return FALSE;
    }

    return TRUE;
}

static BOOL _mvr_interface_upd(vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t    *mvif;

    if (!mvr_intf) {
        return FALSE;
    }

    mvif = mvr_intf;
    if (IPMC_LIB_DB_GET(mvr_intf_tree, mvif)) {
        if (mvif != mvr_intf) {
            memcpy(mvif, mvr_intf, sizeof(vtss_mvr_interface_t));
        }

        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL _mvr_itera_intf_add(vtss_mvr_interface_t *mvr_intf,
                                vtss_mvr_interface_t *entry,
                                ipmc_ip_version_t ver_start,
                                ipmc_ip_version_t ver_end)
{
    ipmc_ip_version_t   idx;

    for (idx = ver_start; idx <= ver_end; idx++) {
        memcpy(entry, mvr_intf, sizeof(vtss_mvr_interface_t));
        entry->basic.ipmc_version = idx;

        if (vtss_mvr_interface_get(entry) == VTSS_OK) {
            return FALSE;
        }

        if (_mvr_interface_add(entry) == FALSE) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL _mvr_itera_intf_del(vtss_mvr_interface_t *mvr_intf,
                                vtss_mvr_interface_t *entry,
                                u32 *prev_pdx,
                                ipmc_ip_version_t ver_start,
                                ipmc_ip_version_t ver_end)
{
    ipmc_ip_version_t   idx;

    for (idx = ver_start; idx <= ver_end; idx++) {
        memcpy(entry, mvr_intf, sizeof(vtss_mvr_interface_t));
        entry->basic.ipmc_version = idx;

        if (vtss_mvr_interface_get(entry) != VTSS_OK) {
            return FALSE;
        }

        *prev_pdx = entry->profile_index;
        if (_mvr_interface_del(entry) == FALSE) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL _mvr_itera_intf_upd(vtss_mvr_interface_t *mvr_intf,
                                vtss_mvr_interface_t *entry,
                                u32 *prev_pdx,
                                ipmc_ip_version_t ver_start,
                                ipmc_ip_version_t ver_end)
{
    ipmc_ip_version_t               idx;
    ipmc_prot_intf_entry_param_t    *param;

    for (idx = ver_start; idx <= ver_end; idx++) {
        memcpy(entry, mvr_intf, sizeof(vtss_mvr_interface_t));
        entry->basic.ipmc_version = idx;

        if (vtss_mvr_interface_get(entry) != VTSS_OK) {
            return FALSE;
        }

        memcpy(&entry->basic, &mvr_intf->basic, sizeof(ipmc_intf_entry_t));
        param = &entry->basic.param;

        entry->basic.ipmc_version = idx;
        memcpy(entry->name, mvr_intf->name, sizeof(entry->name));
        entry->mode = mvr_intf->mode;
        param->mvr = VTSS_IPMC_TRUE;
        param->vtag = entry->vtag = mvr_intf->vtag;
        param->priority = entry->priority = mvr_intf->priority;
        *prev_pdx = entry->profile_index;
        entry->profile_index = mvr_intf->profile_index;
        memcpy(entry->ports, mvr_intf->ports, sizeof(entry->ports));
        if (entry->basic.ipmc_version != IPMC_IP_VERSION_IGMP) {
            param->querier.QuerierAdrs4 = MVR_QUERIER_ADDRESS4;
        }

        if (_mvr_interface_upd(entry) == FALSE) {
            return FALSE;
        }
    }

    return TRUE;
}

static vtss_mvr_interface_t mvr_intf4intf_set;
/*
    vtss_mvr_interface_set - Set interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_set(ipmc_operation_action_t action, vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t            *entry;
    ipmc_intf_entry_t               *intf;
    ipmc_prot_intf_entry_param_t    *param;
    u32                             prev_pdx;
    ipmc_ip_version_t               start, end;
    port_iter_t                     pit;
    char                            mvrBufPort[MGMT_PORT_BUF_SIZE];
    BOOL                            mvrFwdMap[VTSS_PORT_ARRAY_SIZE];
    vtss_rc                         retVal;

    if (!mvr_intf) {
        return VTSS_RC_ERROR;
    }

    intf = &mvr_intf->basic;
    param = &intf->param;
    T_N("Set Version:%d VID:%u with Action:%d", intf->ipmc_version, param->vid, action);

    /* Parameter Validation */
    if (param->querier.RobustVari == 0) {
        /* RFC-3810 9.1 */
        T_W("Failed in Violating RFC-3810 9.1(RV)");
        return VTSS_RC_ERROR;
    }
    if (param->querier.MaxResTime >= (param->querier.QueryIntvl * 10)) {
        /* RFC-3810 9.3 */
        T_W("Failed in Violating RFC-3810 9.3(QI)");
        return VTSS_RC_ERROR;
    }

    if (param->cfg_compatibility != IPMC_PARAM_DEF_COMPAT) {
        T_W("Failed in Violating Compatibility: %d", param->cfg_compatibility);
        return VTSS_RC_ERROR;
    }

    if (param->querier.QuerierAdrs4) {
        u8  adrc = (param->querier.QuerierAdrs4 >> 24) & 0xFF;

        if ((adrc == 127) || (adrc > 223)) {
            i8  adrString[40];

            memset(adrString, 0x0, sizeof(adrString));
            T_W("Invalid %s address: %s",
                ipmc_lib_version_txt(intf->ipmc_version, IPMC_TXT_CASE_UPPER),
                misc_ipv4_txt(param->querier.QuerierAdrs4, adrString));
            return VTSS_RC_ERROR;
        }
    }

    entry = &mvr_intf4intf_set;
    retVal = VTSS_RC_ERROR;
    if (intf->ipmc_version == IPMC_IP_VERSION_ALL) {
        start = IPMC_IP_VERSION_IGMP;
        end = IPMC_IP_VERSION_MLD;
    } else {
        start = end = intf->ipmc_version;
    }

    memset(mvrFwdMap, 0x0, sizeof(mvrFwdMap));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        if (mvr_intf->ports[pit.iport] != MVR_PORT_ROLE_INACT) {
            mvrFwdMap[pit.iport] = TRUE;
        }
    }

    prev_pdx = 0;
    switch ( action ) {
    case IPMC_OP_ADD:
        T_I("Start(%d/%d) _mvr_itera_intf_add(%u) with Ports=%s",
            start, end, param->vid,
            mgmt_iport_list2txt(mvrFwdMap, mvrBufPort));
        if (_mvr_itera_intf_add(mvr_intf, entry, start, end)) {
            T_I("Done _mvr_itera_intf_add");
            retVal = VTSS_OK;
        }

        break;
    case IPMC_OP_DEL:
        T_I("Start(%d/%d) _mvr_itera_intf_del(%u)", start, end, param->vid);
        if (_mvr_itera_intf_del(mvr_intf, entry, &prev_pdx, start, end)) {
            T_I("Done _mvr_itera_intf_del");
            retVal = VTSS_OK;
        }

        break;
    case IPMC_OP_UPD:
    case IPMC_OP_SET:
        T_N("Start(%d/%d) _mvr_itera_intf_upd(%u) with Ports=%s",
            start, end, param->vid,
            mgmt_iport_list2txt(mvrFwdMap, mvrBufPort));
        if (_mvr_itera_intf_upd(mvr_intf, entry, &prev_pdx, start, end)) {
            T_N("Done _mvr_itera_intf_upd");
            retVal = VTSS_OK;
        }

        break;
    default:

        break;
    }

    if (retVal == VTSS_OK) {
        /* Set Profile VID to denote MVR MVID */
        if (action != IPMC_OP_DEL) {
            if (action != IPMC_OP_ADD) {
                if (prev_pdx != mvr_intf->profile_index) {
                    if (!ipmc_lib_profile_tree_vid_set(prev_pdx, VTSS_IPMC_VID_NULL)) {
                        retVal = IPMC_ERROR_PARM;
                    } else {
                        if (!ipmc_lib_profile_tree_vid_set(mvr_intf->profile_index, param->vid)) {
                            retVal = IPMC_ERROR_PARM;
                        }
                    }
                }
            } else {
                if (mvr_intf->profile_index && !ipmc_lib_profile_tree_vid_set(mvr_intf->profile_index, param->vid)) {
                    retVal = IPMC_ERROR_PARM;
                }
            }
        } else {
            if (prev_pdx && !ipmc_lib_profile_tree_vid_set(prev_pdx, VTSS_IPMC_VID_NULL)) {
                retVal = IPMC_ERROR_PARM;
            }
        }
    }

    T_N("Complete with RC=%d", retVal);
    return retVal;
}

/*
    vtss_mvr_interface_get - Get interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_get(vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t    *mvif;

    if (!mvr_intf) {
        return VTSS_RC_ERROR;
    }

    mvif = mvr_intf;
    if (IPMC_LIB_DB_GET(mvr_intf_tree, mvif)) {
        if (mvif != mvr_intf) {
            memcpy(mvr_intf, mvif, sizeof(vtss_mvr_interface_t));
        }

        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

/*
    vtss_mvr_interface_get_next - GetNext interface settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_interface_get_next(vtss_mvr_interface_t *mvr_intf)
{
    vtss_mvr_interface_t    *mvif;

    if (!mvr_intf) {
        return VTSS_RC_ERROR;
    }

    mvif = mvr_intf;
    if (IPMC_LIB_DB_GET_NEXT(mvr_intf_tree, mvif)) {
        if (mvif != mvr_intf) {
            memcpy(mvr_intf, mvif, sizeof(vtss_mvr_interface_t));
        }

        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

BOOL vtss_mvr_intf_group_get(ipmc_group_entry_t *grp)
{
    i8  buf[40];

    if (!grp) {
        return FALSE;
    }

    T_D("vtss_mvr_intf_group_get VID:%u/VER:%d, group_address = %s",
        grp->vid, grp->ipmc_version, misc_ipv6_txt(&grp->group_addr, buf));

    return ipmc_lib_group_get(mvr_mgrp_tree, grp);
}

BOOL vtss_mvr_intf_group_get_next(ipmc_group_entry_t *grp)
{
    i8  buf[40];

    if (!grp) {
        return FALSE;
    }

    T_D("vtss_mvr_intf_group_get VID:%u/VER:%d, group_address = %s",
        grp->vid, grp->ipmc_version, misc_ipv6_txt(&grp->group_addr, buf));

    return ipmc_lib_group_get_next(mvr_mgrp_tree, grp);
}

#ifndef VTSS_SW_OPTION_IPMC
/*
    vtss_mvr_upd_unknown_fwdmsk - Update unknown flooding mask.
*/
void vtss_mvr_upd_unknown_fwdmsk(void)
{
    ipmc_activate_t action;
    BOOL            member4[VTSS_PORT_ARRAY_SIZE];
    BOOL            member6[VTSS_PORT_ARRAY_SIZE];
    port_iter_t     pit;
    u32             idx;

#if VTSS_SWITCH_STACKABLE
    action = IPMC_ACTIVATE_RUN;
#else
    if (mvr_global.unregistered_flood) {
        action = IPMC_ACTIVATE_RUN;
    } else {
        action = IPMC_ACTIVATE_OFF;
    }
#endif /* VTSS_SWITCH_STACKABLE */

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        if (action != IPMC_ACTIVATE_RUN) {
            member6[idx] = ipmc_lib_get_port_rpstatus(IPMC_IP_VERSION_MLD, idx);
            member4[idx] = ipmc_lib_get_port_rpstatus(IPMC_IP_VERSION_IGMP, idx);
        } else {
            member6[idx] = member4[idx] = TRUE;
        }
    }

    if (!ipmc_lib_unregistered_flood_set(IPMC_OWNER_MVR4, action, member4)) {
        T_W("Failure in ipmc_lib_unregistered_flood_set");
    }
    if (!ipmc_lib_unregistered_flood_set(IPMC_OWNER_MVR6, action, member6)) {
        T_W("Failure in ipmc_lib_unregistered_flood_set");
    }
}
#endif /* VTSS_SW_OPTION_IPMC */

/*
    Port state callback function
    This function is called if a LOCAL port state change occur.
*/
void vtss_mvr_port_state_change_handle(vtss_port_no_t port_no, port_info_t *info)
{
    T_D("vtss_mvr_port_state_change_handle->port_no: %u. link %s\n", port_no, info->link ? "up" : "down");

    if (info->link == 0) {
        if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, port_no)) {
            /* clear dynamic router when it links down */
            vtss_mvr_set_router_ports(port_no, FALSE);

            mvr_del_group_member_with_lnk(IPMC_IP_VERSION_ALL, port_no);
        }
    }
}

/*
    STP-Port state callback function
    This function is called if a LOCAL STP state change occur.
*/
void vtss_mvr_stp_port_state_change_handle(ipmc_ip_version_t version, vtss_port_no_t port_no, vtss_common_stpstate_t new_state)
{
    T_D("vtss_mvr_stp_port_state_change_handle->port_no: %u  link %s\n", port_no, (new_state == VTSS_STP_STATE_DISCARDING) ? "STP_STATE_DISCARDING" : "STP_STATE_FORWARDING");

    if ((mvr_stp_port_status[port_no] == VTSS_COMMON_STPSTATE_DISCARDING) &&
        (new_state == VTSS_COMMON_STPSTATE_FORWARDING)) {
        vtss_mvr_interface_t    intf;
        ipmc_intf_entry_t       *entry;
        ipmc_port_bfs_t         port_bit_mask;
        vtss_ipv6_t             dst_ip6;

        VTSS_PORT_BF_CLR(port_bit_mask.member_ports);
        VTSS_PORT_BF_SET(port_bit_mask.member_ports, port_no, TRUE);

        memset(&intf, 0x0, sizeof(vtss_mvr_interface_t));
        while (vtss_mvr_interface_get_next(&intf) == VTSS_OK) {
            entry = &intf.basic;
            if ((version != IPMC_IP_VERSION_ALL) && (entry->ipmc_version != version)) {
                continue;
            }

            if (VTSS_PORT_BF_GET(entry->vlan_ports, port_no) && VTSS_PORT_BF_GET(port_bit_mask.member_ports, port_no)) {
                ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                (void) ipmc_lib_packet_tx_helping_query(entry,
                                                        &dst_ip6,
                                                        &port_bit_mask,
                                                        (entry->param.vtag == IPMC_INTF_UNTAG) ? TRUE : FALSE,
                                                        FALSE);
            }
        }
    }

    mvr_stp_port_status[port_no] = new_state;
}

void vtss_mvr_process_glag(u32 port, u32 vid, const u8 *const frame, u32 frame_len, ipmc_ip_version_t ipmc_version)
{
    ipmc_mld_packet_t   *mld = NULL;
    ipmc_igmp_packet_t  *igmp = NULL;

    if (ipmc_version == IPMC_IP_VERSION_MLD) {
        mld_ip6_hbh_hdr  *ip6HbH = (mld_ip6_hbh_hdr *) (frame + sizeof(ipmc_ip_eth_hdr) + IPV6_HDR_FIXED_LEN);

        if (ip6HbH->HdrExtLen) {
            mld = (ipmc_mld_packet_t *) (frame + sizeof(ipmc_ip_eth_hdr) + IPV6_HDR_FIXED_LEN + ip6HbH->HdrExtLen);
        } else {
            mld = (ipmc_mld_packet_t *) (frame + sizeof(ipmc_ip_eth_hdr) + IPV6_HDR_FIXED_LEN + 8);
        }
    } else {
        igmp_ip4_hdr    *ip4Hdr = (igmp_ip4_hdr *) (frame + sizeof(ipmc_ip_eth_hdr));
        igmp = (ipmc_igmp_packet_t *) (frame + ntohs(ip4Hdr->PayloadLen));
    }

    if (mvr_global.state) {
        ipmc_group_entry_t grp_buf, *grp;
        vtss_ipv4_t        ip4sip, ip4dip;
        vtss_ipv6_t        ip6sip, ip6dip;
        port_iter_t        pit;
        u32                i;
        BOOL               fwd_map[VTSS_PORT_ARRAY_SIZE] = {FALSE};

        grp_buf.vid = vid;
        grp_buf.ipmc_version = ipmc_version;
        if (ipmc_version == IPMC_IP_VERSION_MLD) {
            if (mld &&
                (mld->common.type == IPMC_MLD_MSG_TYPE_V1REPORT ||
                 mld->common.type == IPMC_MLD_MSG_TYPE_V2REPORT)) {
                memcpy(&grp_buf.group_addr, &mld->sfminfo.usual.group_address, sizeof(vtss_ipv6_t));
            }
        } else {
            if (igmp &&
                (igmp->common.type == IPMC_IGMP_MSG_TYPE_V1JOIN ||
                 igmp->common.type == IPMC_IGMP_MSG_TYPE_V2JOIN ||
                 igmp->common.type == IPMC_IGMP_MSG_TYPE_V3JOIN)) {
                memcpy(&grp_buf.group_addr.addr[12], &igmp->sfminfo.usual.group_address, sizeof(ipmcv4addr));
            }
        }

        if ((grp = ipmc_lib_group_ptr_get(mvr_mgrp_tree, &grp_buf)) != NULL) {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(grp->info->db.port_mask, i) ||
                    ipmc_lib_get_port_rpstatus(ipmc_version, i)) {
                    fwd_map[i] = TRUE;
                }
            }

#if VTSS_SWITCH_STACKABLE
            fwd_map[PORT_NO_STACK_0] = TRUE;
            fwd_map[PORT_NO_STACK_1] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

            /* ASM-UPDATE(ADD) */
            if (ipmc_version == IPMC_IP_VERSION_MLD) {
                memset((u8 *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                memset((u8 *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
                memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
            } else {
                ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
                memcpy((u8 *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
            }

            if (ipmc_lib_porting_set_chip(TRUE, mvr_mgrp_tree, grp, ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map) != VTSS_OK) {
                T_D("ipmc_lib_porting_set_chip ADD failed");
            }
            /* SHOULD Notify MC-Routing + */
        }
    }
}

void vtss_mvr_calculate_dst_ports(BOOL compat_bypass,
                                  BOOL inc_rcver,
                                  vtss_vid_t vid,
                                  u8 port_no,
                                  ipmc_port_bfs_t *port_mask,
                                  ipmc_ip_version_t version)
{
    vtss_mvr_interface_t    intf;
    ipmc_intf_entry_t       *entry;
    port_iter_t             pit;
    u32                     i;

    memset(&intf, 0x0, sizeof(vtss_mvr_interface_t));
    entry = &intf.basic;
    entry->ipmc_version = version;
    entry->param.vid = vid;
    if (vtss_mvr_interface_get(&intf) == VTSS_OK) {
        if (!compat_bypass || (intf.mode != MVR_INTF_MODE_COMP)) {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;
                if (i == port_no) {
                    continue;
                }

                if (inc_rcver) {
                    if (intf.ports[i] != MVR_PORT_ROLE_INACT) {
                        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
                    }
                } else {
                    if (intf.ports[i] == MVR_PORT_ROLE_SOURC) {
                        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
                    }
                }
            }
        }
    }

#if VTSS_SWITCH_STACKABLE
    if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, port_no)) {
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
    } else {
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, FALSE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */
}

static void vtss_mvr_update_mld_statistics(ipmc_intf_entry_t *entry, ipmc_mld_packet_t *mld)
{
    switch ( mld->common.type ) {
    case IPMC_MLD_MSG_TYPE_QUERY:
        entry->param.stats.mld_queries++;

        break;
    case IPMC_MLD_MSG_TYPE_V1REPORT:
        entry->param.stats.mld_v1_membership_report++;

        break;
    case IPMC_MLD_MSG_TYPE_V2REPORT:
        entry->param.stats.mld_v2_membership_report++;

        break;
    case IPMC_MLD_MSG_TYPE_DONE:
        entry->param.stats.mld_v1_membership_done++;

        break;
    default: /* Unknown/Un-Classified */
        entry->param.stats.mld_error_pkt++;

        break;
    }
}

static void vtss_mvr_update_igmp_statistics(ipmc_intf_entry_t *entry, ipmc_igmp_packet_t *igmp)
{
    switch ( igmp->common.type ) {
    case IPMC_IGMP_MSG_TYPE_QUERY:
        entry->param.stats.igmp_queries++;

        break;
    case IPMC_IGMP_MSG_TYPE_V1JOIN:
        entry->param.stats.igmp_v1_membership_join++;

        break;
    case IPMC_IGMP_MSG_TYPE_V2JOIN:
        entry->param.stats.igmp_v2_membership_join++;

        break;
    case IPMC_IGMP_MSG_TYPE_V3JOIN:
        entry->param.stats.igmp_v3_membership_join++;

        break;
    case IPMC_IGMP_MSG_TYPE_LEAVE:
        entry->param.stats.igmp_v2_membership_leave++;

        break;
    default: /* Unknown/Un-Classified */
        entry->param.stats.igmp_error_pkt++;

        break;
    }
}

void vtss_mvr_clear_stat_counter(vtss_vid_t vid)
{
    vtss_mvr_interface_t    intf;
    ipmc_intf_entry_t       *entry;

    if ((vid == VTSS_IPMC_VID_NULL) ||
        ((vid != VTSS_IPMC_VID_ALL) && (vid > VTSS_IPMC_VID_MAX))) {
        return;
    }

    /* clear statistics */
    memset(&intf, 0x0, sizeof(vtss_mvr_interface_t));
    while (vtss_mvr_interface_get_next(&intf) == VTSS_OK) {
        entry = &intf.basic;
        if ((vid != VTSS_IPMC_VID_ALL) && (entry->param.vid != vid)) {
            continue;
        }

        memset(&entry->param.stats, 0x0, sizeof(ipmc_statistics_t));
        entry->param.querier.ipmc_queries_sent = 0;
        entry->param.querier.group_queries_sent = 0;

        if (vtss_mvr_interface_set(IPMC_OP_SET, &intf) != VTSS_OK) {
            T_D("vtss_mvr_interface_set(IPMC_OP_SET) Failed!");
        }
    }
}

void vtss_mvr_set_mode(BOOL mode)
{
    if (mvr_global.state != mode) {
        if (mode == FALSE) {
            port_iter_t pit;

            /* if disabled, clear dynamic router ports */
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                vtss_mvr_set_router_ports(pit.iport, FALSE);
            }
        }

        (void) vtss_mvr_purge(VTSS_ISID_LOCAL, mode, TRUE);

        mvr_global.state = mode;
#ifndef VTSS_SW_OPTION_IPMC
        vtss_mvr_upd_unknown_fwdmsk();
#endif /* VTSS_SW_OPTION_IPMC */
    }
}

void vtss_mvr_set_unreg_flood(BOOL enabled)
{
    if (mvr_global.unregistered_flood != enabled) {
        mvr_global.unregistered_flood = enabled;
#ifndef VTSS_SW_OPTION_IPMC
        vtss_mvr_upd_unknown_fwdmsk();
#endif /* VTSS_SW_OPTION_IPMC */
    }
}

void vtss_mvr_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *prefix)
{
    ipmc_prefix_t   ssm_range;

    if (!prefix) {
        return;
    } else {
        memcpy(&ssm_range, prefix, sizeof(ipmc_prefix_t));
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ssm_range.addr.value.prefix = htonl(prefix->addr.value.prefix);
        memcpy(&mvr_global.ssm4_fltr, &ssm_range, sizeof(ipmc_prefix_t));
    } else {
        memcpy(&mvr_global.ssm6_fltr, &ssm_range, sizeof(ipmc_prefix_t));
    }

    ipmc_lib_set_ssm_range(ipmc_version, &ssm_range);
}

void vtss_mvr_set_router_ports(u32 idx, BOOL status)
{
    /* Set & Refresh */
    if (status) {
        mvr_global.router_port_timer[idx] = ROUTER_PORT_TIMEOUT;
    } else {
        mvr_global.router_port_timer[idx] = IPMC_PARAM_VALUE_INIT;
    }

    VTSS_PORT_BF_SET(mvr_global.router_port, idx, status);
}

void vtss_mvr_set_fast_leave_ports(ipmc_port_bfs_t *port_mask)
{
    port_iter_t pit;
    u32         i;

    if (!port_mask) {
        return;
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        VTSS_PORT_BF_SET(mvr_global.fast_leave,
                         i,
                         VTSS_PORT_BF_GET(port_mask->member_ports, i));
    }
}

/*
    vtss_mvr_get_fast_leave_ports - Get fast leave status of a port in vtss_mvr application module.
*/
BOOL vtss_mvr_get_fast_leave_ports(u32 port)
{
    return VTSS_PORT_BF_GET(mvr_global.fast_leave, port);
}

/*
    vtss_mvr_global_set - Set global settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_global_set(vtss_mvr_global_t *global_entry)
{
    ipmc_port_bfs_t fast_leave;
    port_iter_t     pit;
    u32             idx;

    if (!global_entry) {
        return VTSS_RC_ERROR;
    }

    VTSS_PORT_BF_CLR(fast_leave.member_ports);

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        VTSS_PORT_BF_SET(fast_leave.member_ports,
                         idx,
                         VTSS_PORT_BF_GET(global_entry->fast_leave, idx));
    }

    vtss_mvr_set_fast_leave_ports(&fast_leave);
    vtss_mvr_set_unreg_flood(global_entry->unregistered_flood);
    vtss_mvr_set_mode(global_entry->state);

    return VTSS_OK;
}

/*
    vtss_mvr_global_get - Get global settings of vtss_mvr application module.
*/
vtss_rc vtss_mvr_global_get(vtss_mvr_global_t *global_entry)
{
    if (!global_entry) {
        return VTSS_RC_ERROR;
    }

    memcpy(global_entry, &mvr_global, sizeof(vtss_mvr_global_t));

    return VTSS_OK;
}

#ifdef VTSS_SW_OPTION_PACKET
static ipmc_pkt_attribute_t mvr_pkt_attrib;
static void mvr_specific_port_fltr_set(vtss_mvr_interface_t *intf, specific_grps_fltr_t *fltr)
{
    u32 pdx;

    if (!intf || !fltr) {
        return;
    }

    fltr->vid = intf->basic.param.vid;

#if IPMC_LIB_FLTR_MULTIPLE_PROFILE
#else
    pdx = intf->profile_index;
    if (pdx) {
        fltr->pdx[0] = pdx;
        ++fltr->filter_cnt;
    }
#endif /* IPMC_LIB_FLTR_MULTIPLE_PROFILE */
}

static ipmc_group_entry_t           grp_rxi_mvr;
static vtss_mvr_interface_t         mvr_intf4rxi;
static ipmc_lib_grp_fltr_profile_t  mvr_profile4rxi;
vtss_rc RX_ipmcmvr(ipmc_ip_version_t version,
                   u16 ingress_vid,
                   void *contxt,
                   const u8 *const frame,
                   const vtss_packet_rx_info_t *const rx_info,
                   ipmc_port_bfs_t *ret_fwd,
                   vtss_mvr_intf_tx_t pcp[MVR_NUM_OF_INTF_PER_VERSION])
{
    vtss_rc                     rc, retVal;
    u8                          *content, msgType;
    vtss_mvr_interface_t        *intf;
    ipmc_intf_entry_t           *entry;

    u32                         i, j, intf_cnt, local_port_cnt;
    u16                         intf_mvid, mvr_vlan[MVR_NUM_OF_INTF_PER_VERSION];
    mvr_intf_mode_t             intf_mode, mvr_mode[MVR_NUM_OF_INTF_PER_VERSION];
    mvr_port_role_t             port_role, mvr_role[MVR_NUM_OF_INTF_PER_VERSION];
    u32                         src_port, frame_len, magic;
    BOOL                        sfmQuery, oldQuery;
    BOOL                        pass_compat, update_statistics;
    BOOL                        source_drop, op_interrupt, from_fast_leave;
    ipmc_compat_mode_t          compatibility;
    ipmc_operation_action_t     grp_op;
    int                         actual_throttling;
    ipmc_group_entry_t          *grp;
    ipmc_group_info_t           *grp_info;
    ipmc_group_db_t             *grp_db;
    specific_grps_fltr_t        mvr_specific_port_fltr;
    ipmc_lib_grp_fltr_profile_t *profile;

    intf = &mvr_intf4rxi;
    profile = &mvr_profile4rxi;
    T_D("RX_ipmcmvr enter(%s):%s",
        ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
        mvr_global.state ? "Global-Enabled" : "Global-Disabled");

    frame_len = rx_info->length;
    src_port = rx_info->port_no;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();

    memset(mvr_vlan, 0x0, sizeof(mvr_vlan));
    memset(mvr_mode, 0xFF, sizeof(mvr_mode));
    memset(mvr_role, 0x0, sizeof(mvr_role));
    memset(intf, 0x0, sizeof(vtss_mvr_interface_t));
    i = intf_cnt = 0;
    from_fast_leave = source_drop = FALSE;
    while (vtss_mvr_interface_get_next(intf) == VTSS_OK) {
        if (i >= MVR_NUM_OF_INTF_PER_VERSION) {
            break;
        }

        entry = &intf->basic;

#if VTSS_SWITCH_STACKABLE
        if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port) &&
            (intf->ports[src_port] == MVR_PORT_ROLE_INACT)) {
            T_D("MVR-%s(%u)-Port-%u Inactive",
                ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                entry->param.vid,
                src_port);
            continue;
        }
#else
        if (intf->ports[src_port] == MVR_PORT_ROLE_INACT) {
            T_D("MVR-%s(%u)-Port-%u Inactive",
                ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                entry->param.vid,
                src_port);
            continue;
        }
#endif /* VTSS_SWITCH_STACKABLE */

        if (entry->ipmc_version != version) {
            T_D("%s-MVID-%u Version Mismatch",
                ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                entry->param.vid);
            continue;
        }

        if (entry->param.vid != ingress_vid) {
#if VTSS_SWITCH_STACKABLE
            if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port)) {
                source_drop = TRUE;
                T_D("MVR-%s(%u)-StackingPort-%u Source-VID Mismatched",
                    ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                    entry->param.vid,
                    src_port);
                continue;
            } else {
                if (intf->ports[src_port] == MVR_PORT_ROLE_SOURC) {
                    source_drop = TRUE;
                    T_D("MVR-%s(%u)-Port-%u Source-VID Mismatched",
                        ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                        entry->param.vid,
                        src_port);
                    continue;
                }
            }
#else
            if (intf->ports[src_port] == MVR_PORT_ROLE_SOURC) {
                source_drop = TRUE;
                T_D("MVR-%s(%u)-Port-%u Source-VID Mismatched",
                    ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER),
                    entry->param.vid,
                    src_port);
                continue;
            }
#endif /* VTSS_SWITCH_STACKABLE */
        }

        pcp[i].vid = mvr_vlan[i] = entry->param.vid;
        pcp[i].vtag = intf->vtag;
        pcp[i].priority = intf->priority;
        switch ( intf->ports[src_port] ) {
        case MVR_PORT_ROLE_INACT:
            pcp[i].src_type = IPMC_PKT_SRC_MVR_INACT;
            break;
        case MVR_PORT_ROLE_SOURC:
            pcp[i].src_type = IPMC_PKT_SRC_MVR_SOURC;
            break;
        case MVR_PORT_ROLE_RECVR:
            pcp[i].src_type = IPMC_PKT_SRC_MVR_RECVR;
            break;
        case MVR_PORT_ROLE_STACK:
            pcp[i].src_type = IPMC_PKT_SRC_MVR_STACK;
            break;
        default:
            pcp[i].src_type = IPMC_PKT_SRC_MVR;
            break;
        }
        mvr_mode[i] = intf->mode;
        mvr_role[i] = intf->ports[src_port];

        magic = 0;
        if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port)) {
            memcpy((u8 *)&magic, frame + (frame_len - IPMC_PKT_PRIVATE_PAD_LEN), IPMC_PKT_PRIVATE_PAD_LEN);

            magic = htonl(magic);
            if (((magic >> 8) << 8) == IPMC_PKT_PRIVATE_PAD_MAGIC) {
                if (magic & 0xF0) {
                    from_fast_leave = TRUE;
                }

                switch ( (magic & 0xF) ) {
                case IPMC_PKT_SRC_MVR_SOURC:
                    mvr_role[i] = MVR_PORT_ROLE_SOURC;
                    pcp[i].src_type = IPMC_PKT_SRC_MVR_SOURC;
                    break;
                case IPMC_PKT_SRC_MVR_RECVR:
                    mvr_role[i] = MVR_PORT_ROLE_RECVR;
                    pcp[i].src_type = IPMC_PKT_SRC_MVR_RECVR;
                    break;
                default:
                    break;
                }
            }
        }

        T_D("(%s%u-MAGIC%u)pcp.vid=%u pcp.vtag=%d pcp.priority=%u pcp.src_type=%d mvr_mode=%d mvr_role=%d",
            ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER), i, magic,
            mvr_vlan[i], pcp[i].vtag, pcp[i].priority, pcp[i].src_type, mvr_mode[i], mvr_role[i]);

        i++;
    }

    if (i == 0) {
        for (i = 0; i < local_port_cnt; i++) {
            if (i != src_port) {
                if (source_drop) {
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                } else {
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, TRUE);
                }
            } else {
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
            }
        }

        return IPMC_ERROR_VLAN_NOT_FOUND;
    }

    retVal = rc = VTSS_OK;
    op_interrupt = FALSE;
    intf_cnt = i;
    for (j = 0; (mvr_global.state && (j < intf_cnt)); j++) {
        if (!mvr_vlan[j]) {
            break;
        }

        intf_mvid = mvr_vlan[j];
        intf_mode = mvr_mode[j];
        port_role = mvr_role[j];

        T_D("(RetRC/CurRC:%d/%d)Start Processing %s-VID-%u",
            retVal, rc, ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER), intf_mvid);

        entry = &intf->basic;
        entry->ipmc_version = version;
        entry->param.vid = intf_mvid;
        rc = vtss_mvr_interface_get(intf);
        if (rc != VTSS_OK) {
            T_D("MVR VID-%u Not Exist", intf_mvid);
            retVal |= rc;
            continue;
        }

        memset(&mvr_pkt_attrib, 0x0, sizeof(ipmc_pkt_attribute_t));
        rc = ipmc_lib_packet_rx(entry, frame, rx_info, &mvr_pkt_attrib);
        if (rc != VTSS_OK) {
            retVal |= rc;
            continue;
        }

        msgType = mvr_pkt_attrib.msgType;
        if ((msgType == IPMC_MLD_MSG_TYPE_V1REPORT) ||
            (msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) ||
            (msgType == IPMC_IGMP_MSG_TYPE_V2JOIN) ||
            (msgType == IPMC_MLD_MSG_TYPE_V2REPORT) ||
            (msgType == IPMC_IGMP_MSG_TYPE_V3JOIN) ||
            (msgType == IPMC_MLD_MSG_TYPE_DONE) ||
            (msgType == IPMC_IGMP_MSG_TYPE_LEAVE)) {
            profile->data.index = intf->profile_index;
            if (!ipmc_lib_fltr_profile_get(profile, FALSE)) {
                T_D("%s-MVID-%u has no channel", ipmc_lib_version_txt(entry->ipmc_version, IPMC_TXT_CASE_UPPER), entry->param.vid);
                retVal |= rc;
                continue;
            }
        }

        T_D("ipmc_lib_packet_rx(%d) with %s-VID-%u",
            rc, ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER), intf_mvid);
        T_D_HEX(frame, frame_len);

        sfmQuery = oldQuery = FALSE;
        update_statistics = TRUE;
        compatibility = IPMC_COMPATIBILITY(entry);

        switch ( msgType ) {
        case IPMC_MLD_MSG_TYPE_QUERY:
        case IPMC_IGMP_MSG_TYPE_QUERY:
            if (version == IPMC_IP_VERSION_MLD) {
                if (mvr_pkt_attrib.ipmc_pkt_len >= MLD_SFM_MIN_PAYLOAD_LEN) {
                    sfmQuery = TRUE;
                }

                if (compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) {
                    pass_compat = TRUE;
                } else if (compatibility == VTSS_IPMC_COMPAT_MODE_GEN) {
                    if (sfmQuery) {
                        pass_compat = FALSE;
                    } else {
                        pass_compat = TRUE;
                    }
                } else if (compatibility == VTSS_IPMC_COMPAT_MODE_SFM) {
                    if (sfmQuery) {
                        pass_compat = TRUE;
                    } else {
                        pass_compat = FALSE;
                    }
                } else {
                    T_D("Incorrect Compatibility(%u)!!!", compatibility);
                    rc = IPMC_ERROR_PKT_COMPATIBILITY;
                    break;
                }
            } else if (version == IPMC_IP_VERSION_IGMP) {
                if (mvr_pkt_attrib.ipmc_pkt_len >= IGMP_SFM_MIN_PAYLOAD_LEN) {
                    sfmQuery = TRUE;
                } else {
                    if (!mvr_pkt_attrib.max_resp_time) {
                        oldQuery = TRUE;
                    }
                }

                if (compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) {
                    pass_compat = TRUE;
                } else if (compatibility == VTSS_IPMC_COMPAT_MODE_OLD) {
                    if (oldQuery) {
                        pass_compat = TRUE;
                    } else {
                        pass_compat = FALSE;
                    }
                } else if (compatibility == VTSS_IPMC_COMPAT_MODE_GEN) {
                    if (sfmQuery || oldQuery) {
                        pass_compat = FALSE;
                    } else {
                        pass_compat = TRUE;
                    }
                } else if (compatibility == VTSS_IPMC_COMPAT_MODE_SFM) {
                    if (sfmQuery) {
                        pass_compat = TRUE;
                    } else {
                        pass_compat = FALSE;
                    }
                } else {
                    T_D("Incorrect Compatibility(%u)!!!", compatibility);
                    rc = IPMC_ERROR_PKT_COMPATIBILITY;
                    break;
                }
            } else {
                T_D("Incorrect version(%u)!!!", version);
                rc = IPMC_ERROR_PKT_VERSION;
                break;
            }

            if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port) && pass_compat) {
                T_D("IGMP/MLD query len is %d (resp = %d)",
                    (int)(frame_len - mvr_pkt_attrib.offset),
                    (int)mvr_pkt_attrib.max_resp_time);

                if (!ipmc_lib_isaddr6_all_zero(&mvr_pkt_attrib.src_ip_addr)) {
                    if (mvr_pkt_attrib.msgType == IPMC_MLD_MSG_TYPE_QUERY) {
                        vtss_ipv6_t ipLinkLocalSrc;

                        /* get src address */
                        if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                            if (memcmp(&mvr_pkt_attrib.src_ip_addr, &ipLinkLocalSrc, sizeof(vtss_ipv6_t)) < 0) {
                                entry->param.querier.state = IPMC_QUERIER_IDLE;

                                IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                entry->param.querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(entry);
                            } else {
                                if (!entry->param.querier.querier_enabled) {
                                    if (ipmc_lib_isaddr6_all_zero(&entry->param.active_querier)) {
                                        IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                    } else {
                                        if (IPMC_LIB_ADRS_CMP6(mvr_pkt_attrib.src_ip_addr, entry->param.active_querier) < 0) {
                                            IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                        }
                                    }
                                }
                            }
                        } else {
                            T_D("Get Current IP Address failed !!!");
                        }
                    }

                    if (mvr_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_QUERY) {
                        vtss_ipv4_t ip4addr = 0;

                        /* get src address */
                        if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                            ip4addr = htonl(ip4addr);
                            if (memcmp(&mvr_pkt_attrib.src_ip_addr.addr[12], &ip4addr, sizeof(ipmcv4addr)) < 0) {
                                entry->param.querier.state = IPMC_QUERIER_IDLE;

                                IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                entry->param.querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(entry);
                            } else {
                                if (!entry->param.querier.querier_enabled) {
                                    if (ipmc_lib_isaddr6_all_zero(&entry->param.active_querier)) {
                                        IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                    } else {
                                        if (IPMC_LIB_ADRS_CMP4(mvr_pkt_attrib.src_ip_addr, entry->param.active_querier) < 0) {
                                            IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &mvr_pkt_attrib.src_ip_addr);
                                        }
                                    }
                                }
                            }
                        } else {
                            T_D("Get Current IP Address failed !!!");
                        }
                    }
                }
            } /* if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port) && pass_compat) */

            mvr_query_flooding_cnt++;
            if (mvr_query_flooding_cnt < MVR_MAX_FLOODING_COUNT) {
                /* we basically want to flood this to all ports within VLAN, PVLAN ... */
                if (port_role == MVR_PORT_ROLE_SOURC) {
                    vtss_mvr_calculate_dst_ports(FALSE, TRUE, intf_mvid, src_port, ret_fwd, version);
                } else {
                    vtss_mvr_calculate_dst_ports(FALSE, FALSE, intf_mvid, src_port, ret_fwd, version);
                }
                rc = IPMC_ERROR_PKT_IS_QUERY;
            } else {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);
                rc = IPMC_ERROR_PKT_TOO_MUCH_QUERY;
            }

            if (!pass_compat) {
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            /* Is this a general query ? */
            if (!ipmc_lib_isaddr6_all_zero(&mvr_pkt_attrib.group_addr)) {
                vtss_mvr_set_router_ports(src_port, TRUE);

                if (entry->param.querier.state == IPMC_QUERIER_IDLE) { /* Non-Querier */
                    grp_rxi_mvr.vid = intf_mvid;
                    grp_rxi_mvr.ipmc_version = entry->ipmc_version;
                    memcpy(&grp_rxi_mvr.group_addr, &mvr_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
                    if ((grp = ipmc_lib_group_ptr_get(mvr_mgrp_tree, &grp_rxi_mvr)) != NULL) {
                        grp_info = grp->info;
                        grp_db = &grp_info->db;

                        for (i = 0; i < local_port_cnt; i++) {
                            if (IPMC_LIB_CHK_LISTENER_GET(grp, i)) {
                                if (!mvr_pkt_attrib.no_of_sources) {
                                    (void) ipmc_lib_protocol_lower_filter_timer(mvr_fltr_tree, grp, entry, i);
                                }
                            }
                        }

                        /* state transition */
                        grp_info->state = IPMC_OP_CHK_LISTENER;
                    } /* if (ipmc_lib_group_get(mvr_mgrp_tree, grp)) */
                } /* Non-Querier */
            }

            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                entry->param.rtr_compatibility.mode = compatibility;
                break;
            }

            if (oldQuery) {
                entry->param.rtr_compatibility.old_present_timer = IPMC_TIMER_OVQPT(entry);

                entry->param.rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_OLD;
            } else {
                if (!sfmQuery) {
                    entry->param.rtr_compatibility.gen_present_timer = IPMC_TIMER_OVQPT(entry);

                    if (entry->param.rtr_compatibility.mode == IPMC_PARAM_DEF_COMPAT ||
                        entry->param.rtr_compatibility.mode >= VTSS_IPMC_COMPAT_MODE_GEN) {
                        entry->param.rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_GEN;
                    }
                } else {
                    entry->param.rtr_compatibility.sfm_present_timer = IPMC_TIMER_OVQPT(entry);

                    if (entry->param.rtr_compatibility.mode == IPMC_PARAM_DEF_COMPAT ||
                        entry->param.rtr_compatibility.mode >= VTSS_IPMC_COMPAT_MODE_SFM) {
                        entry->param.rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
                    }
                }
            }

            break;
        case IPMC_MLD_MSG_TYPE_V1REPORT:
        case IPMC_IGMP_MSG_TYPE_V1JOIN:
        case IPMC_IGMP_MSG_TYPE_V2JOIN:
            if ((intf_mode != MVR_INTF_MODE_DYNA) &&
                (port_role == MVR_PORT_ROLE_SOURC)) {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
#endif /* VTSS_SWITCH_STACKABLE */
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            pass_compat = TRUE;
            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                if ((mvr_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_OLD)) {
                    pass_compat = FALSE;
                }

                if ((mvr_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V2JOIN) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_GEN)) {
                    pass_compat = FALSE;
                }

                if ((mvr_pkt_attrib.msgType == IPMC_MLD_MSG_TYPE_V1REPORT) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_GEN)) {
                    pass_compat = FALSE;
                }
            }

            if (!pass_compat) {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (u8 *)(frame + mvr_pkt_attrib.offset);
            grp_op = IPMC_OP_SET;
            actual_throttling = IPMC_REPORT_NORMAL;
            MVR_PORT_PROFILE_GET(&mvr_specific_port_fltr, &mvr_pkt_attrib, intf, src_port);
            rc = ipmc_lib_protocol_do_sfm_report(mvr_mgrp_tree,
                                                 mvr_rxmt_tree,
                                                 mvr_fltr_tree,
                                                 mvr_srct_tree,
                                                 entry,
                                                 content,
                                                 src_port,
                                                 mvr_pkt_attrib.msgType,
                                                 mvr_pkt_attrib.ipmc_pkt_len,
                                                 &mvr_specific_port_fltr,
                                                 &actual_throttling,
                                                 FALSE,
                                                 &grp_op);
            if (grp_op == IPMC_OP_INT) {
                op_interrupt = TRUE;
            }

            if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port) && VTSS_PORT_BF_GET(mvr_global.fast_leave, src_port)) {
                if (rc != VTSS_OK) {
                    VTSS_PORT_BF_CLR(ret_fwd->member_ports);
                } else {
                    vtss_mvr_get_router_port_mask(src_port, ret_fwd);
                }
            } else {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
            }
            vtss_mvr_calculate_dst_ports(TRUE, FALSE, intf_mvid, src_port, ret_fwd, version);
#if VTSS_SWITCH_STACKABLE
            if (intf->mode == MVR_INTF_MODE_COMP) {
                if (rc != VTSS_OK) {
                    i = PORT_NO_STACK_0;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                    i = PORT_NO_STACK_1;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                }
            }
#endif /* VTSS_SWITCH_STACKABLE */

            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                entry->param.hst_compatibility.mode = compatibility;
                break;
            }

            if (mvr_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) {
                entry->param.hst_compatibility.old_present_timer = IPMC_TIMER_OVHPT(entry);
            } else {
                entry->param.hst_compatibility.gen_present_timer = IPMC_TIMER_OVHPT(entry);
            }

            break;
        case IPMC_MLD_MSG_TYPE_V2REPORT:
        case IPMC_IGMP_MSG_TYPE_V3JOIN:
            if ((intf_mode != MVR_INTF_MODE_DYNA) &&
                (port_role == MVR_PORT_ROLE_SOURC)) {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
#endif /* VTSS_SWITCH_STACKABLE */
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            pass_compat = TRUE;
            if ((compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) &&
                (compatibility != VTSS_IPMC_COMPAT_MODE_SFM)) {
                pass_compat = FALSE;
            }

            if (!pass_compat) {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (u8 *)(frame + mvr_pkt_attrib.offset);
            grp_op = IPMC_OP_SET;
            actual_throttling = IPMC_REPORT_NORMAL;
            MVR_PORT_PROFILE_GET(&mvr_specific_port_fltr, &mvr_pkt_attrib, intf, src_port);
            rc = ipmc_lib_protocol_do_sfm_report(mvr_mgrp_tree,
                                                 mvr_rxmt_tree,
                                                 mvr_fltr_tree,
                                                 mvr_srct_tree,
                                                 entry,
                                                 content,
                                                 src_port,
                                                 mvr_pkt_attrib.msgType,
                                                 mvr_pkt_attrib.ipmc_pkt_len,
                                                 &mvr_specific_port_fltr,
                                                 &actual_throttling,
                                                 FALSE,
                                                 &grp_op);
            if (grp_op == IPMC_OP_INT) {
                op_interrupt = TRUE;
            }

            if (rc != VTSS_OK) {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);
            } else {
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);
            }
            vtss_mvr_calculate_dst_ports(TRUE, FALSE, intf_mvid, src_port, ret_fwd, version);
#if VTSS_SWITCH_STACKABLE
            if (intf->mode == MVR_INTF_MODE_COMP) {
                if (rc != VTSS_OK) {
                    i = PORT_NO_STACK_0;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                    i = PORT_NO_STACK_1;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
                }
            }
#endif /* VTSS_SWITCH_STACKABLE */

            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                entry->param.hst_compatibility.mode = compatibility;
                break;
            }

            if (!entry->param.hst_compatibility.sfm_present_timer) {
                entry->param.hst_compatibility.sfm_present_timer = IPMC_TIMER_OVHPT(entry);
            }

            break;
        case IPMC_MLD_MSG_TYPE_DONE:
        case IPMC_IGMP_MSG_TYPE_LEAVE:
            pass_compat = TRUE;
            if ((compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) &&
                (compatibility != VTSS_IPMC_COMPAT_MODE_GEN)) {
                pass_compat = FALSE;
            }

            if (!pass_compat) {
#if VTSS_SWITCH_STACKABLE
                u32 tmp_val;
#endif /* VTSS_SWITCH_STACKABLE */
                vtss_mvr_get_router_port_mask(src_port, ret_fwd);

#if VTSS_SWITCH_STACKABLE
                tmp_val = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, tmp_val, TRUE);
                tmp_val = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, tmp_val, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (u8 *)(frame + mvr_pkt_attrib.offset);

            grp_rxi_mvr.vid = intf_mvid;
            grp_rxi_mvr.ipmc_version = version;
            memcpy(&grp_rxi_mvr.group_addr, &mvr_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
            grp = ipmc_lib_group_ptr_get(mvr_mgrp_tree, &grp_rxi_mvr);
            if (grp && grp->info && IPMC_LIB_GRP_PORT_DO_SFM(&grp->info->db, src_port)) {
                grp_info = grp->info;
                grp_db = &grp_info->db;

                grp_op = IPMC_OP_SET;
                actual_throttling = IPMC_REPORT_NORMAL;
                MVR_PORT_PROFILE_GET(&mvr_specific_port_fltr, &mvr_pkt_attrib, intf, src_port);
                rc = ipmc_lib_protocol_do_sfm_report(mvr_mgrp_tree,
                                                     mvr_rxmt_tree,
                                                     mvr_fltr_tree,
                                                     mvr_srct_tree,
                                                     entry,
                                                     content,
                                                     src_port,
                                                     mvr_pkt_attrib.msgType,
                                                     mvr_pkt_attrib.ipmc_pkt_len,
                                                     &mvr_specific_port_fltr,
                                                     &actual_throttling,
                                                     FALSE,
                                                     &grp_op);
                if (grp_op == IPMC_OP_INT) {
                    op_interrupt = TRUE;
                }

                /* fast leave */
                if ((rc == VTSS_OK) &&
                    (from_fast_leave || VTSS_PORT_BF_GET(mvr_global.fast_leave, src_port))) {
                    vtss_ipv4_t ip4sip, ip4dip;
                    vtss_ipv6_t ip6sip, ip6dip;
                    BOOL        fwd_map[VTSS_PORT_ARRAY_SIZE];

                    for (i = 0; i < local_port_cnt; i++) {
                        if (i != src_port) {
                            if (VTSS_PORT_BF_GET(grp_db->port_mask, i) ||
                                VTSS_PORT_BF_GET(mvr_global.router_port, i)) {
                                fwd_map[i] = TRUE;
                            } else {
                                fwd_map[i] = FALSE;
                            }
                        } else {
                            if (VTSS_PORT_BF_GET(mvr_global.router_port, i)) {
                                fwd_map[i] = TRUE;
                            } else {
                                fwd_map[i] = FALSE;
                            }
                        }
                    }

#if VTSS_SWITCH_STACKABLE
                    fwd_map[PORT_NO_STACK_0] = TRUE;
                    fwd_map[PORT_NO_STACK_1] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

                    /* ASM-UPDATE(ADD) */
                    if (version == IPMC_IP_VERSION_MLD) {
                        memset((u8 *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                        memset((u8 *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                        ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
                        memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
                    } else {
                        ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
                        memcpy((u8 *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                        memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                        memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                    }

                    if (ipmc_lib_porting_set_chip(TRUE, mvr_mgrp_tree, grp, version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map) != VTSS_OK) {
                        T_D("ipmc_lib_porting_set_chip ADD failed");
                    }
                    /* SHOULD Notify MC-Routing + */

                    /* Workaround for Fast-Aging without Querier */
                    if (entry->param.querier.state == IPMC_QUERIER_IDLE) {
                        (void) ipmc_lib_protocol_lower_filter_timer(mvr_fltr_tree, grp, entry, src_port);
                    }
                } /* fast leave */

                /* group state machine */
                if ((rc == VTSS_OK) && VTSS_PORT_BF_GET(grp_db->port_mask, src_port)) {
                    if (entry->param.querier.state != IPMC_QUERIER_IDLE) { /* Querier */
                        /* state transition */
                        grp_info->state = IPMC_OP_CHK_LISTENER;

                        /* start rxmt timer */
                        grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                        IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], mvr_rxmt_tree, grp_info);
                    }
                } /* VTSS_PORT_BF_GET(grp_db->port_mask, src_port) */
            } else {
                rc = IPMC_ERROR_PKT_GROUP_NOT_FOUND;
            }

            vtss_mvr_get_router_port_mask(src_port, ret_fwd);
            vtss_mvr_calculate_dst_ports(FALSE, FALSE, intf_mvid, src_port, ret_fwd, version);

            break;
        default:
            /* unknown type, we shall flood this one */
            vtss_mvr_calculate_dst_ports(FALSE, FALSE, intf_mvid, src_port, ret_fwd, version);

            break;
        }

        /* update statistics (only for packets comming from front ports) */
        if (update_statistics && !port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port)) {
            if (version == IPMC_IP_VERSION_MLD) {
                T_D("MLD update_statistics for msgType(%u) on Port-%u", mvr_pkt_attrib.msgType, src_port);
                vtss_mvr_update_mld_statistics(entry, (ipmc_mld_packet_t *)(frame + mvr_pkt_attrib.offset));
            }

            if (version == IPMC_IP_VERSION_IGMP) {
                T_D("IGMP update_statistics for msgType(%u) on Port-%u", mvr_pkt_attrib.msgType, src_port);
                vtss_mvr_update_igmp_statistics(entry, (ipmc_igmp_packet_t *)(frame + mvr_pkt_attrib.offset));
            }
        }

        if (vtss_mvr_interface_set(IPMC_OP_SET, intf) != VTSS_OK) {
            T_D("vtss_mvr_interface_set(IPMC_OP_SET) Failed!");
        }

        retVal |= rc;
    }

    if (op_interrupt && (retVal == VTSS_OK)) {
        return IPMC_ERROR_PKT_GROUP_FILTER;
    } else {
        return retVal;
    }
}
#endif /* VTSS_SW_OPTION_PACKET */

static vtss_mvr_interface_t intf4purge;
/*
    vtss_mvr_purge - Purge internal database.
*/
vtss_rc vtss_mvr_purge(vtss_isid_t isid, BOOL mode, BOOL grp_adr_only)
{
    port_iter_t         pit;
    u32                 idx;
    ipmc_intf_entry_t   *entry;
    ipmc_group_entry_t  *grp, *grp_ptr;

    if (!vtss_mvr_global_done_init) {
        return VTSS_RC_ERROR;
    }

    T_D("Start purging MVR Internal(Global:%s)", mode ? "Enabled" : "Disabled");

    if (!ipmc_lib_mc6_ctrl_flood_set(IPMC_OWNER_MVR, mode)) {
        T_D("Failure in ipmc_lib_mc6_ctrl_flood_set");
    }

    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(mvr_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        entry = &intf4purge.basic;
        entry->ipmc_version = grp->ipmc_version;
        entry->param.vid = grp->vid;
        if (vtss_mvr_interface_get(&intf4purge) != VTSS_OK) {
            (void) ipmc_lib_group_delete(NULL, mvr_mgrp_tree, mvr_rxmt_tree, mvr_fltr_tree, mvr_srct_tree, grp, FALSE, TRUE);
        } else {
            (void) ipmc_lib_group_delete(entry, mvr_mgrp_tree, mvr_rxmt_tree, mvr_fltr_tree, mvr_srct_tree, grp, FALSE, TRUE);
        }
    }

    if (grp_adr_only) {
        return VTSS_OK;
    }

    memset(&intf4purge, 0x0, sizeof(vtss_mvr_interface_t));
    while (vtss_mvr_interface_get_next(&intf4purge) == VTSS_OK) {
        (void) vtss_mvr_interface_set(IPMC_OP_DEL, &intf4purge);
    }

    mvr_global.state = mvr_global.unregistered_flood = VTSS_IPMC_DISABLE;
    memset(&mvr_global.ssm4_fltr, 0x0, sizeof(ipmc_prefix_t));
    memset(&mvr_global.ssm6_fltr, 0x0, sizeof(ipmc_prefix_t));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        VTSS_PORT_BF_SET(mvr_global.router_port, idx, FALSE);
        VTSS_PORT_BF_SET(mvr_global.fast_leave, idx, FALSE);
        mvr_global.router_port_timer[idx] = IPMC_PARAM_VALUE_INIT;
        mvr_stp_port_status[idx] = VTSS_COMMON_STPSTATE_FORWARDING;
    }

#if VTSS_SWITCH_STACKABLE
    idx = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(mvr_global.router_port, idx, TRUE);
    idx = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(mvr_global.router_port, idx, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

    mvr_query_suppression_timer = QUERY_SUPPRESSION_TIMEOUT;
    mvr_query_flooding_cnt = 0;

    return VTSS_OK;
}

void vtss_mvr_tick_gen(void)
{
    port_iter_t pit;
    u32         i;

    if (mvr_global.state) {
        (void) ipmc_lib_protocol_suppression(&mvr_query_suppression_timer,
                                             QUERY_SUPPRESSION_TIMEOUT,
                                             &mvr_query_flooding_cnt);

        /* router port timeout */
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (mvr_global.router_port_timer[i] == IPMC_PARAM_VALUE_INIT) {
                continue;
            }

            if (--mvr_global.router_port_timer[i] == 0) {
                mvr_global.router_port_timer[i] = IPMC_PARAM_VALUE_INIT;
                VTSS_PORT_BF_SET(mvr_global.router_port, i, FALSE);
            }
        }
        /* router port timeout */
    } /* mvr_global.state */
}

void vtss_mvr_tick_intf_tmr(ipmc_intf_entry_t *intf)
{
    if (!intf || !mvr_global.state) {
        return;
    }

    if (ipmc_lib_protocol_intf_tmr(FALSE, intf) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_intf_tmr for VLAN %d", intf->param.vid);
    }
}

void vtss_mvr_tick_intf_rxmt(void)
{
    if (ipmc_lib_protocol_intf_rxmt(mvr_mgrp_tree,
                                    mvr_rxmt_tree,
                                    mvr_fltr_tree) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_intf_rxmt");
    }
}

void vtss_mvr_tick_group_tmr(void)
{
    if (!mvr_global.state) {
        return;
    }

    if (ipmc_lib_protocol_group_tmr(TRUE,
                                    mvr_mgrp_tree,
                                    mvr_rxmt_tree,
                                    mvr_fltr_tree,
                                    mvr_srct_tree,
                                    NULL,
                                    NULL,
                                    NULL) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_group_tmr");
    }
}

void vtss_mvr_init(void)
{
    port_iter_t pit;
    u32         i;

    (void) ipmc_lib_protocol_init();

    memset(&mvr_global, 0x0, sizeof(vtss_mvr_global_t));

    /* create the interface table; including V4 & V6 at the same time */
    if (!vtss_mvr_intf_entry_list_created_done) {
        /* create data base for storing MVR interface entry */
        if (!IPMC_LIB_DB_TAKE("MVR_INTF_TABLE", mvr_intf_tree,
                              MVR_NUM_OF_SUPPORTED_INTF,
                              sizeof(vtss_mvr_interface_t),
                              vtss_mvr_intf_entry_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_mvr_intf_entry_list) failed");
        } else {
            mvr_intf_tree->mflag = TRUE;
            mvr_global.interfaces = mvr_intf_tree;
            vtss_mvr_intf_entry_list_created_done = TRUE;
        }
    }

    /* create the multicast group table */
    if (!vtss_mvr_mgroup_entry_list_created_done) {
        /* create data base for storing IPMCMVR group entry */
        if (!IPMC_LIB_DB_TAKE("MVR_GRP_TABLE", mvr_mgrp_tree,
                              MVR_NO_OF_SUPPORTED_GROUPS,
                              sizeof(ipmc_group_entry_t),
                              vtss_mvr_mgroup_entry_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_mvr_mgroup_entry_list) failed");
        } else {
            mvr_mgrp_tree->mflag = TRUE;
            mvr_global.groups = mvr_mgrp_tree;
            vtss_mvr_mgroup_entry_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast group (filter) timer */
    if (!vtss_mvr_grp_fltr_tmr_list_created_done) {
        /* create data base for storing multicast group (filter) timer */
        if (!IPMC_LIB_DB_TAKE("MVR_GRP_FLTR", mvr_fltr_tree,
                              IPMC_LIB_MAX_MVR_FLTR_TMR_LIST,
                              sizeof(ipmc_group_db_t),
                              vtss_ipmcmvr_grp_fltr_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_mvr_grp_fltr_tmr_list) failed");
        } else {
            mvr_fltr_tree->mflag = TRUE;
            vtss_mvr_grp_fltr_tmr_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast source timer */
    if (!vtss_mvr_grp_srct_tmr_list_created_done) {
        /* create data base for storing multicast source timer */
        if (!IPMC_LIB_DB_TAKE("MVR_GRP_SRCT", mvr_srct_tree,
                              IPMC_LIB_MAX_MVR_SRCT_TMR_LIST,
                              sizeof(ipmc_sfm_srclist_t),
                              vtss_ipmcmvr_grp_srct_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_mvr_grp_srct_tmr_list) failed");
        } else {
            mvr_srct_tree->mflag = TRUE;
            vtss_mvr_grp_srct_tmr_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast rxmt timer */
    if (!vtss_mvr_grp_rxmt_tmr_list_created_done) {
        /* create data base for storing multicast rxmt timer */
        if (!IPMC_LIB_DB_TAKE("MVR_GRP_RXMT", mvr_rxmt_tree,
                              IPMC_LIB_MAX_MVR_RXMT_TMR_LIST,
                              sizeof(ipmc_group_info_t),
                              vtss_ipmcmvr_grp_rxmt_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_mvr_grp_rxmt_tmr_list) failed");
        } else {
            mvr_rxmt_tree->mflag = TRUE;
            vtss_mvr_grp_rxmt_tmr_list_created_done = TRUE;
        }
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        mvr_global.router_port_timer[i] = IPMC_PARAM_VALUE_INIT;
        mvr_stp_port_status[i] = VTSS_COMMON_STPSTATE_FORWARDING;
    }

#if VTSS_SWITCH_STACKABLE
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(mvr_global.router_port, i, TRUE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(mvr_global.router_port, i, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

    mvr_query_suppression_timer = QUERY_SUPPRESSION_TIMEOUT;
    mvr_query_flooding_cnt = 0;

    vtss_mvr_global_done_init = TRUE;
}
