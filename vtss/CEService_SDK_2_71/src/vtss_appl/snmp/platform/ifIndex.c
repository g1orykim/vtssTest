/*

 Vitesse Switch Software.

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

#include <main.h>
#include "misc_api.h"
#include "mgmt_api.h"
#include "ifIndex_api.h"

#ifdef VTSS_SW_OPTION_IP2
#include "ip2_utils.h"
#endif /* VTSS_SW_OPTION_IP2 */

#include <vtss_module_id.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SNMP

#define IFINDEX_ERROR -1

/* mib2/ifTable - ifIndex */
#define IFTABLE_IFINDEX_SWITCH_INTERVAL 1000
#define IFTABLE_IFINDEX_LPOAG_START     0
#define IFTABLE_IFINDEX_LPOAG_END       15999
#define IFTABLE_IFINDEX_GLAG_START      40000
#define IFTABLE_IFINDEX_VLAN_START      50000
#define IFTABLE_IFINDEX_IP_START        60000

#define IfIndex2Isid(_IFINDEX)                      ((_IFINDEX) / (IFTABLE_IFINDEX_SWITCH_INTERVAL))
#define IfIndex2Interface(_IFINDEX, _START_ID)      (((_IFINDEX) > (_START_ID)) ? ((_IFINDEX) - (_START_ID)) : 0)
#define Interface2IfIndex(_INTERFACE, _START_ID)    ((_INTERFACE) + (_START_ID))

#ifdef VTSS_SW_OPTION_IP2

static int cmp_ip(vtss_if_status_t *data, vtss_ip_addr_t *key)
{
    int cmp;
    vtss_ip_type_t data_type = data->type == VTSS_IF_STATUS_TYPE_IPV6 ? VTSS_IP_TYPE_IPV6 : VTSS_IP_TYPE_IPV4;
    if ( data_type > key->type ) {
        return 1;
    } else if ( data_type < key->type ) {
        return -1;
    }

    if (data_type == VTSS_IP_TYPE_IPV4) {
        cmp = (data->u.ipv4.net.address > key->addr.ipv4) ? 1 : (data->u.ipv4.net.address < key->addr.ipv4) ? -1 : 0;
    } else {
        cmp = memcmp(&data->u.ipv6.net.address, &key->addr.ipv6, sizeof(vtss_ipv6_t));
    }

    return cmp;
}

static vtss_rc get_next_ipv6_status_by_if (vtss_if_id_vlan_t if_id, vtss_ip_addr_t *ip_addr, vtss_if_status_t *v6_st)
{
#define MAX_OBJS 16
    u32                 if_st_cnt = 0;
    vtss_if_status_t    status[MAX_OBJS], *ptr = status;
    vtss_ip_addr_t      tmp;
    int                 i = 0, j = 0;
    vtss_rc             found = VTSS_RC_ERROR;

    tmp.type = VTSS_IP_TYPE_IPV6;
    memset(&tmp.addr.ipv6, 0xff, sizeof(tmp.addr.ipv6));

    if ( VTSS_RC_OK != vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV6, if_id, MAX_OBJS,
                                              &if_st_cnt, status) || if_st_cnt == 0 ) {
        return VTSS_RC_ERROR;
    }

    for (i = 0, ptr = status; i < (int) if_st_cnt; i++, ptr++) {
        if ( (cmp_ip(ptr, &tmp) > 0 ) ||
             (ip_addr != NULL && (cmp_ip( ptr, ip_addr ) <= 0 )) ) {
            continue;
        }

        (void) vtss_if_status_to_ip( ptr, &tmp);
        j = i;
        found = VTSS_RC_OK;
    }

    if ( VTSS_RC_OK == found ) {
        *v6_st = status[j];
    }

#undef MAX_OBJS
    return found;
}

BOOL get_next_ip(vtss_ip_addr_t *ip_addr, vtss_if_id_vlan_t   *if_id)
{
    vtss_rc             rc;
    vtss_if_status_t    st, st_v6_first;
    vtss_ip_addr_t      tmp, tmp_v6_first;
    vtss_if_id_vlan_t   key = VTSS_VID_NULL;
    vtss_if_id_vlan_t   ifid, ifid_v6_first = VTSS_VID_NULL;
    BOOL                found = FALSE, found_v6_first = FALSE;

    if ( ip_addr == NULL || ip_addr->type > VTSS_IP_TYPE_IPV6) {
        return FALSE;
    }

    if ( ip_addr->type < VTSS_IP_TYPE_IPV6) {
        tmp.type = VTSS_IP_TYPE_IPV4;
        tmp.addr.ipv4 = 0;
    } else {
        tmp.type = VTSS_IP_TYPE_IPV6;
        memset(&tmp.addr.ipv6, 0, sizeof(tmp.addr.ipv6));
    }

    tmp_v6_first.type = VTSS_IP_TYPE_IPV6;
    memset(&tmp_v6_first.addr.ipv6, 0xff, sizeof(tmp_v6_first.addr.ipv6));

    while (VTSS_RC_OK == vtss_ip2_if_id_next( key, &ifid)) {
        key = ifid;
        if (ip_addr->type == VTSS_IP_TYPE_IPV4) {
            if ( get_next_ipv6_status_by_if(ifid, NULL, &st_v6_first) == VTSS_RC_OK ) {
                if (cmp_ip(&st_v6_first, &tmp_v6_first) < 0 ) {
                    memcpy(&tmp_v6_first.addr.ipv6, &st_v6_first.u.ipv6.net.address, sizeof(st_v6_first.u.ipv6.net.address));
                    found_v6_first = TRUE;
                    ifid_v6_first = ifid;
                }
            }
            rc = vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV4, ifid, &st);
        } else {
            rc = get_next_ipv6_status_by_if(ifid, ip_addr, &st);
        }

        if (rc != VTSS_RC_OK || cmp_ip(&st, ip_addr) <= 0) {
            continue;
        }

        if (found == FALSE || cmp_ip(&st, &tmp) < 0) {
            if (st.type == VTSS_IF_STATUS_TYPE_IPV4) {
                tmp.type = VTSS_IP_TYPE_IPV4;
                tmp.addr.ipv4 = st.u.ipv4.net.address;
            } else {
                tmp.type = VTSS_IP_TYPE_IPV6;
                memcpy(&tmp.addr.ipv6, &st.u.ipv6.net.address, sizeof(st.u.ipv6.net.address));
            }
            if (if_id) {
                *if_id = ifid;
            }
            found = TRUE;
        }
    }

    if ( TRUE == found ) {
        *ip_addr = tmp;
    } else if ( TRUE == found_v6_first ) {
        if (if_id) {
            *if_id = ifid_v6_first;
        }
        *ip_addr = tmp_v6_first;
        found = TRUE;
    }

    return found;
}

BOOL get_next_ip_status(u32 *if_id, vtss_if_status_t *status)
{
    vtss_if_status_t    if_status;
    vtss_if_id_vlan_t   ifid;
    u32                 ipcnt;

    if ( VTSS_RC_OK != vtss_ip2_if_id_next( *if_id, &ifid) ||
         vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_ANY, ifid, 1, &ipcnt, &if_status) != VTSS_RC_OK ||
         if_status.type == VTSS_IF_STATUS_TYPE_INVALID ) {
        return FALSE;
    }
    *if_id = ifid;
    if ( status ) {
        *status = if_status;
        T_D("ifid = %d, type = %s, in_packets = %u", ifid, if_status.type == VTSS_IP_TYPE_IPV4 ? "IPV4" : if_status.type == VTSS_IP_TYPE_IPV6 ? "IPV6" : "NONE", (u32)status->u.link_stat.in_packets);
    } else {
        T_D("ifid = %d, type = %s", ifid, if_status.type == VTSS_IP_TYPE_IPV4 ? "IPV4" : if_status.type == VTSS_IP_TYPE_IPV6 ? "IPV6" : "NONE");
    }
    return TRUE;
}
#endif /* VTSS_SW_OPTION_IP2 */

typedef BOOL    (ifIndex_cb_t) ( ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist );

typedef struct {
    ifIndex_id_t startIdx;
    ifIndex_type_t type;
    ifIndex_cb_t *get_next_cb_fn;
} ifIndex_cb_table_t;


//Add a temporary solution to deny the switch access (configurable but non-existing) via SNMP
//This solution is for v3.40 only.
#define IFINDEX_TEMP_SOLUTION

#ifdef IFINDEX_TEMP_SOLUTION
#include <ucd-snmp/config.h>
#include <ucd-snmp/asn1.h>
#include <ucd-snmp/snmp_api.h>
#include <ucd-snmp/snmpd.h>
#endif

static BOOL port_get_next(ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist)
{
    vtss_isid_t     isid = topo_usid2isid(IfIndex2Isid(info->ifIndex) + 1);
    u_long          if_id = IfIndex2Interface(info->ifIndex, startIdx);

    if ( isid >= VTSS_ISID_END || !msg_switch_configurable(isid) || if_id >= port_isid_port_count(isid) ) {
        return FALSE;
    }

#ifdef IFINDEX_TEMP_SOLUTION
    cyg_handle_t    netsnmp_handle;
    cyg_net_snmp_thread_handle_get(&netsnmp_handle);
    if ((cyg_thread_self() == netsnmp_handle) && !msg_switch_exists(isid)) {
        return FALSE;
    }
#endif

//    T_D("enter startIdx = %d , isid = %d, if_id = %d, port_count = %d", (int)startIdx, (int)isid, (int)if_id, (int)port_isid_port_count(isid));
    info->isid = isid;
    info->if_id = uport2iport(++if_id);
    info->ifIndex = Interface2IfIndex(if_id, startIdx);
//    T_D("isid %d PORT %d ifIndex %d %s ", (int)info->isid, (int)info->if_id, (int)info->ifIndex, TRUE == check_exist ? "exist" : "valid");
    return TRUE;
}

static BOOL llag_get_next(ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist)
{
#if (AGGR_LLAG_CNT == 0)
    return FALSE;
#else
    vtss_isid_t                 isid = topo_usid2isid(IfIndex2Isid(info->ifIndex) + 1);
    u_long                      if_id = IfIndex2Interface(info->ifIndex, startIdx);
    aggr_mgmt_group_no_t        aggr_id = 0;
    aggr_mgmt_group_member_t    aggr_members;
//    T_D("enter isid = %d, if_id = %d, port_count = %d", (int)isid, (int)if_id, (int)port_isid_port_count(isid));

    if ( isid >= VTSS_ISID_END || !msg_switch_configurable(isid) ) {
        return FALSE;
    }

#ifdef IFINDEX_TEMP_SOLUTION
    cyg_handle_t    netsnmp_handle;
    cyg_net_snmp_thread_handle_get(&netsnmp_handle);
    if ((cyg_thread_self() == netsnmp_handle) && !msg_switch_exists(isid)) {
        return FALSE;
    }
#endif

    if (!AGGR_MGMT_GROUP_IS_LAG((aggr_mgmt_group_no_t)(if_id + 1))) {
        return FALSE;
    }

    aggr_id = if_id;

    if (TRUE == check_exist) {
        if ((aggr_mgmt_port_members_get(isid, aggr_id >= MGMT_AGGR_ID_START ? mgmt_aggr_id2no(aggr_id) : (MGMT_AGGR_ID_START - 1), &aggr_members, TRUE) != VTSS_OK)
#ifdef VTSS_SW_OPTION_LACP
            && (aggr_mgmt_lacp_members_get(isid, aggr_id >= MGMT_AGGR_ID_START ? mgmt_aggr_id2no(aggr_id) : (MGMT_AGGR_ID_START - 1), &aggr_members, TRUE) != VTSS_OK)
#endif /* VTSS_SW_OPTION_LACP */
           ) {
            return FALSE;
        }
        aggr_id = mgmt_aggr_no2id(aggr_members.aggr_no);
    } else {
        aggr_id = if_id + 1;
    }

    if (!AGGR_MGMT_GROUP_IS_LAG(aggr_id)) {
        T_E("aggr_id valid");
        return FALSE;
    }

    info->if_id = mgmt_aggr_id2no(aggr_id);
    info->isid = isid;
    info->ifIndex = Interface2IfIndex(aggr_id, startIdx);
//    T_D("isid %d LLAG %d ifIndex %d %s ", (int)info->isid, (int)info->if_id, (int)info->ifIndex, TRUE == check_exist ? "exist" : "valid");

    return TRUE;
#endif
}

static BOOL glag_get_next(ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist)
{

#if (AGGR_GLAG_CNT == 0)
    return FALSE;
#else
    u_long                      if_id = IfIndex2Interface(info->ifIndex, startIdx);
    aggr_mgmt_group_no_t        glag_id = if_id, aggr;
    aggr_mgmt_group_member_t    aggr_members;
    vtss_isid_t                 isid = 0;
    BOOL                        found = FALSE;

    memset(&aggr_members, 0, sizeof(aggr_members));
//    T_D("enter isid %d glag %d ", (int)info->isid, (int)glag_id);
    if ( glag_id >= AGGR_GLAG_CNT ) {
        return FALSE;
    }

    if (TRUE == check_exist) {
        for (aggr = AGGR_MGMT_GLAG_START + glag_id - AGGR_MGMT_GROUP_NO_START; aggr < AGGR_MGMT_GROUP_NO_END; aggr++) {
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                if (!msg_switch_configurable(isid)) {
                    continue;
                }

#ifdef IFINDEX_TEMP_SOLUTION
                cyg_handle_t    netsnmp_handle;
                cyg_net_snmp_thread_handle_get(&netsnmp_handle);
                if ((cyg_thread_self() == netsnmp_handle) && !msg_switch_exists(isid)) {
                    continue;
                }
#endif

                if ((aggr_mgmt_port_members_get(isid, aggr + 1, &aggr_members, FALSE) != VTSS_OK)
#ifdef VTSS_SW_OPTION_LACP
                    && (aggr_mgmt_lacp_members_get(isid, aggr + 1, &aggr_members, FALSE) != VTSS_OK)
#endif /* VTSS_SW_OPTION_LACP */
                   ) {
                    continue;
                }
                found = TRUE;
                break;
            }
            if ( TRUE == found ) {
                break;
            }
        }

        if (FALSE == found) {
            return FALSE;
        }
        glag_id = mgmt_aggr_no2id(aggr_members.aggr_no);
    } else {
        glag_id++;
    }

    info->if_id = mgmt_aggr_id2no(glag_id);
    info->ifIndex = Interface2IfIndex(glag_id, startIdx);
    info->isid = VTSS_ISID_UNKNOWN;
//    T_D("isid %d GLAG %d ifIndex %d %s ", (int)info->isid, (int)info->if_id, (int)info->ifIndex, TRUE == check_exist ? "exist" : "valid");
    return TRUE;
#endif
}

static BOOL vlan_get_next(ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist)
{
    u_long          if_id = IfIndex2Interface(info->ifIndex, startIdx);
//    T_D("enter isid %d ID %d ", (int)info->isid, (int)if_id);

    if (if_id >= VLAN_ID_MAX) {
        return FALSE;
    }

    if ( FALSE == check_exist ) {
        info->isid = VTSS_ISID_UNKNOWN;
        info->if_id = ++if_id;
        info->ifIndex = Interface2IfIndex(if_id, startIdx);
    } else {
        vlan_mgmt_entry_t vlan_mgmt_entry;

        /* search valid interface */
        vlan_mgmt_entry.vid = if_id;
        if (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, vlan_mgmt_entry.vid, &vlan_mgmt_entry, TRUE, VLAN_USER_ALL) != VTSS_OK) {
            return FALSE;
        }

        info->isid = VTSS_ISID_UNKNOWN;
        info->if_id = vlan_mgmt_entry.vid;
        info->ifIndex = Interface2IfIndex(vlan_mgmt_entry.vid, startIdx);
    }
//    T_D("isid %d VLAN %d ifIndex %d %s ", (int)info->isid, (int)info->if_id, (int)info->ifIndex, TRUE == check_exist ? "exist" : "valid");
    return TRUE;
}

static BOOL ip_get_next(ifIndex_id_t startIdx, iftable_info_t *info, BOOL check_exist)
{
    u32 if_id = IfIndex2Interface(info->ifIndex, startIdx);

//    T_D("enter isid %d ID %d ", (int)info->isid, (int)if_id);

#if defined(VTSS_SW_OPTION_IP2)
    if (TRUE == check_exist) {
        if ( FALSE == get_next_ip_status(&if_id, NULL) ) {
            return FALSE;
        }
        info->if_id = if_id;
    } else {
        if (if_id > VLAN_ID_MAX) {
            return FALSE;
        }
        info->if_id = ++if_id;
    }
#else
    if (if_id >= 1) {
        return FALSE;
    }
    info->if_id = ++if_id;
#endif /* VTSS_SW_OPTION_IP2 */

    info->ifIndex = Interface2IfIndex(if_id, startIdx);
    info->isid = VTSS_ISID_UNKNOWN;
//    T_D("isid %d IP %d ifIndex %d %s ", (int)info->isid, (int)info->if_id, (int)info->ifIndex, TRUE == check_exist ? "exist" : "valid");
    return TRUE;
}

/*lint -esym(459,ifIndex_cb_tbl)*/
/* This table is read only. */
static ifIndex_cb_table_t ifIndex_cb_tbl[] = {

    /* Local port/aggr type*/
    {0   ,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {500 ,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
#ifdef VTSS_SWITCH_STACKABLE
    {1000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {1500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {2000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {2500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {3000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {3500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {4000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {4500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {5000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {5500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {6000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {6500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {7000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {7500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {8000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {8500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {9000,      IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {9500,      IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {10000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {10500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {11000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {11500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {12000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {12500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {13000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {13500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {14000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {14500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
    {15000,     IFTABLE_IFINDEX_TYPE_PORT,      port_get_next},
    {15500,     IFTABLE_IFINDEX_TYPE_LLAG,      llag_get_next},
#endif

    /* Global aggr type */
    {IFTABLE_IFINDEX_GLAG_START,        IFTABLE_IFINDEX_TYPE_GLAG,      glag_get_next},

    /* VLAN type */
    {IFTABLE_IFINDEX_VLAN_START,        IFTABLE_IFINDEX_TYPE_VLAN,      vlan_get_next},

    /* IP type */
    {IFTABLE_IFINDEX_IP_START,          IFTABLE_IFINDEX_TYPE_IP,      ip_get_next},

    /* Undefine type */
    {IFTABLE_IFINDEX_END,               IFTABLE_IFINDEX_TYPE_UNDEF,     NULL}
};


static int ifIndex_cb_size = sizeof(ifIndex_cb_tbl) / sizeof(ifIndex_cb_table_t);

static int ifIndex_cb_get(ifIndex_id_t ifIndex)
{
    int i = 0;
    ifIndex_cb_table_t *entry = NULL;

    for ( entry = &ifIndex_cb_tbl[0]; entry->type != IFTABLE_IFINDEX_TYPE_UNDEF; entry++, i++ )  {
        if (ifIndex >= entry->startIdx && ifIndex < (entry + 1)->startIdx) {
            return i;
        }
    }
    return IFINDEX_ERROR;
}

static int ifIndex_cb_get_by_type(ifIndex_type_t type)
{
    int i = 0;
    ifIndex_cb_table_t *entry = NULL;

    for ( entry = &ifIndex_cb_tbl[0]; entry->type != IFTABLE_IFINDEX_TYPE_UNDEF; entry++, i++ )  {
        if (type == entry->type) {
            return i;
        }
    }
    return IFINDEX_ERROR;
}

static int ifIndex_cb_get_next_by_type(ifIndex_type_t type, int *ifIndex_tbl_id)
{
    int i = *ifIndex_tbl_id + 1;
    ifIndex_cb_table_t *entry = NULL;

    if (i >= ifIndex_cb_size) {
        return IFINDEX_ERROR;
    }

    for ( entry = &ifIndex_cb_tbl[i]; entry->type != IFTABLE_IFINDEX_TYPE_UNDEF; entry++, i++ )  {
        if (type == entry->type) {
            *ifIndex_tbl_id = i;
            return i;
        }
    }
    return IFINDEX_ERROR;
}

static ifIndex_id_t ifIndex_get_next_by_cb( int cb_index, ifIndex_id_t ifIndex, BOOL check_exist, iftable_info_t *output )
{
    iftable_info_t info;
    ifIndex_cb_table_t *cb_entry = &ifIndex_cb_tbl[cb_index];

    info.ifIndex = ifIndex ? ifIndex : cb_entry->startIdx;

    if ( !cb_entry->get_next_cb_fn || FALSE == cb_entry->get_next_cb_fn(cb_entry->startIdx, &info, check_exist)) {
        return IFTABLE_IFINDEX_END;
    }

    info.type = cb_entry->type;
    if ( output ) {
        if ( info.isid != VTSS_ISID_UNKNOWN ) {
            *output = info;
        } else {
            output->ifIndex = info.ifIndex;
            output->type = info.type;
            output->if_id = info.if_id;
        }
    }
    return info.ifIndex;
}

static int ifIndex_get_by_cb(int cb_index, iftable_info_t *info, BOOL check_exist )
{

    if ( info->ifIndex != ifIndex_get_next_by_cb( cb_index, info->ifIndex ? (info->ifIndex - 1) : 0, check_exist, info ) ) {
        return IFTABLE_IFINDEX_END;
    }


    return info->ifIndex;
}

/**
  * \brief Get the existent IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [IN] ifIndex: The ifIndex
  *                       [OUT] type: The interface type
  *                       [OUT] isid: The ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get( iftable_info_t *info )
{
    int ifIndex_cb_id = ifIndex_cb_get( info->ifIndex );
    iftable_info_t tmp;

    if ( ifIndex_cb_id < 0 ) {
        return FALSE;
    }

    tmp.ifIndex = info->ifIndex;
    if ( info->ifIndex != ifIndex_get_by_cb( ifIndex_cb_id, &tmp, TRUE ) ) {
        return FALSE;
    }
    *info = tmp;

    return TRUE;
}

/**
  * \brief Get the valid IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [IN] ifIndex: The ifIndex
  *                       [OUT] type: The interface type
  *                       [OUT] isid: The ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_valid( iftable_info_t *info )
{
//    T_D("enter ifIndex = %lu", info->ifIndex);
    int ifIndex_cb_id = ifIndex_cb_get( info->ifIndex );
    iftable_info_t tmp;

    if ( ifIndex_cb_id < 0 ) {
        return FALSE;
    }

    tmp.ifIndex = info->ifIndex;
    if ( info->ifIndex != ifIndex_get_by_cb( ifIndex_cb_id, &tmp, FALSE ) ) {
        return FALSE;
    }
    *info = tmp;

    return TRUE;

}

/**
  * \brief Get the next existent IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [INOUT] ifIndex: The next ifIndex
  *                       [OUT] type: The next interface type
  *                       [OUT] isid: The next ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: the next interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_next( iftable_info_t *info )
{
//    T_D("enter ifIndex = %lu", info->ifIndex);
    int ifIndex_tbl_id = ifIndex_cb_get( info->ifIndex );
    int i = 0;

    if ( ifIndex_tbl_id < 0 ) {
        return FALSE;
    }

    if ( ifIndex_get_next_by_cb( ifIndex_tbl_id, info->ifIndex, TRUE, info ) != IFTABLE_IFINDEX_END ) {
        return TRUE;
    }

    for (i = ifIndex_tbl_id + 1; i < ifIndex_cb_size; i++) {
        if ( ifIndex_get_next_by_cb( i, 0, TRUE, info ) != IFTABLE_IFINDEX_END ) {
            return TRUE;
        }
    }

    return FALSE;
}

/**
  * \brief Get the next valid IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [INOUT] ifIndex: The next ifIndex
  *                       [OUT] type: The next interface type
  *                       [OUT] isid: The next ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: the next interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_next_valid( iftable_info_t *info )
{
    int ifIndex_tbl_id = ifIndex_cb_get( info->ifIndex );
    int i = 0;

    if ( ifIndex_tbl_id < 0 ) {
        return FALSE;
    }

    if ( ifIndex_get_next_by_cb( ifIndex_tbl_id, info->ifIndex, FALSE, info ) != IFTABLE_IFINDEX_END ) {
        return TRUE;
    }

    for (i = ifIndex_tbl_id + 1; i < ifIndex_cb_size; i++) {
        if ( ifIndex_get_next_by_cb( i, 0, FALSE, info ) != IFTABLE_IFINDEX_END ) {
            return TRUE;
        }
    }

    return FALSE;
}

/**
  * \brief Get the first existent IfIndex in the specific type.
  *
  * \param info [INOUT]:    The info parameter has following members\n
  *                       [IN] type: The interface type
  *                       [OUT] ifIndex: The first ifIndex, if any
  *                       [OUT] isid: The first ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The first interface ID
  * \return
  *    FALSE if there is no available ifIndex.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_first_by_type( iftable_info_t *info )
{
    int ifIndex_tbl_id = ifIndex_cb_get_by_type( info->type );

    if ( IFINDEX_ERROR == ifIndex_tbl_id ) {
        return FALSE;
    }

    if (ifIndex_get_next_by_cb( ifIndex_tbl_id, 0, TRUE, info ) != IFTABLE_IFINDEX_END) {
        return TRUE;
    }

    while ( IFINDEX_ERROR != ifIndex_cb_get_next_by_type( info->type, &ifIndex_tbl_id)) {
        if ( ifIndex_get_next_by_cb( ifIndex_tbl_id, 0, TRUE, info ) != IFTABLE_IFINDEX_END ) {
            return TRUE;
        }
    }

    return FALSE;
}

/**
  * \brief Get the IfIndex in the specific type.
  *
  * \param info [INOUT]:    The info parameter has following members\n
  *                       [IN] if_id: The interface ID
  *                       [IN] type: The interface type
  *                       [IN] isid: The ISID
  *                       [OUT] ifIndex: The ifIndex
  * \return
  *    FALSE if there is no available ifIndex.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_by_interface( iftable_info_t *info )
{
    ifIndex_id_t startIdx;
    ifIndex_id_t ifIndex;
    u_long       uif_id = info->if_id;
    vtss_usid_t  usid;
    int ifIndex_cb_id ;
    switch (info->type) {
    case IFTABLE_IFINDEX_TYPE_PORT:
        usid = topo_isid2usid(info->isid);
        startIdx = ( usid - 1 ) * IFTABLE_IFINDEX_SWITCH_INTERVAL;
        uif_id = iport2uport(info->if_id);
        break;
    case IFTABLE_IFINDEX_TYPE_LLAG:
        usid = topo_isid2usid(info->isid);
        startIdx = ( usid - 1 ) * IFTABLE_IFINDEX_SWITCH_INTERVAL + 500;
        uif_id = mgmt_aggr_no2id(info->if_id);
        break;
    case IFTABLE_IFINDEX_TYPE_GLAG:
        startIdx = IFTABLE_IFINDEX_GLAG_START;
        break;
    case IFTABLE_IFINDEX_TYPE_VLAN:
        startIdx = IFTABLE_IFINDEX_VLAN_START;
        break;
    case IFTABLE_IFINDEX_TYPE_IP:
        startIdx = IFTABLE_IFINDEX_IP_START;
        break;
    default:
        return FALSE;
    }

    ifIndex = Interface2IfIndex(uif_id, startIdx);
    ifIndex_cb_id = ifIndex_cb_get( ifIndex );

    if ( ifIndex_cb_id < 0 || ifIndex != ifIndex_get_next_by_cb(ifIndex_cb_id, startIdx == ifIndex ? startIdx : ifIndex - 1, FALSE, info)) {
        return FALSE;
    }

    return TRUE;
}

