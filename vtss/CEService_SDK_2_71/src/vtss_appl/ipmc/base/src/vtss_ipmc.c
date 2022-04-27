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

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "port_api.h"
#include "critd_api.h"

#include "misc_api.h"
#include "mgmt_api.h"

#include "vtss_ipmc.h"

/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID            VTSS_MODULE_ID_IPMC
#define IPMC_MAX_FLOODING_COUNT         SNP_NUM_OF_SUPPORTED_INTF

#define SNP_PORT_PROFILE_GET(v, w, x, y, z) do {        \
    memset((v), 0x0, sizeof(specific_grps_fltr_t));     \
    IPMC_LIB_ADRS_CPY(&(v)->src, &(x)->src_ip_addr);    \
    IPMC_LIB_ADRS_CPY(&(v)->dst, &(x)->group_addr);     \
    (v)->vid = (y);                                     \
    (v)->port = (z);                                    \
    ipmc_specific_port_fltr_set((w), (v));              \
} while (0)

static ipmc_db_ctrl_hdr_t               vtss_snp_mgroup_entry_list;
static BOOL vtss_snp_mgroup_entry_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t               vtss_ipmc_proxy_report_entry_list;
static BOOL vtss_ipmc_proxy_report_entry_list_created_done = FALSE;

/* For Group Timer List */
static ipmc_db_ctrl_hdr_t               vtss_snp_grp_rxmt_tmr_list;
static BOOL vtss_snp_grp_rxmt_tmr_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t               vtss_snp_grp_fltr_tmr_list;
static BOOL vtss_snp_grp_fltr_tmr_list_created_done = FALSE;
static ipmc_db_ctrl_hdr_t               vtss_snp_grp_srct_tmr_list;
static BOOL vtss_snp_grp_srct_tmr_list_created_done = FALSE;

static  ipmc_db_ctrl_hdr_t              *snp_prxy_tree = &vtss_ipmc_proxy_report_entry_list;
static  ipmc_db_ctrl_hdr_t              *snp_mgrp_tree = &vtss_snp_mgroup_entry_list;
static  ipmc_db_ctrl_hdr_t              *snp_rxmt_tree = &vtss_snp_grp_rxmt_tmr_list;
static  ipmc_db_ctrl_hdr_t              *snp_fltr_tree = &vtss_snp_grp_fltr_tmr_list;
static  ipmc_db_ctrl_hdr_t              *snp_srct_tree = &vtss_snp_grp_srct_tmr_list;


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
static BOOL ipmc_enabled[IPMC_IP_VERSION_MAX];
static BOOL ipmc_leave_proxy_enabled[IPMC_IP_VERSION_MAX];
static BOOL ipmc_proxy_enabled[IPMC_IP_VERSION_MAX];
static BOOL ipmc_unreg_flood_enabled[IPMC_IP_VERSION_MAX];

static BOOL ipmcsnp_fast_leave_ports[IPMC_IP_VERSION_MAX][VTSS_PORT_BF_SIZE];

/* IPMC per VLAN */
static ipmc_intf_entry_t ipmc_vlan_entries[SNP_NUM_OF_SUPPORTED_INTF];

/* general router port mask, common to all VLANs */
static BOOL static_router_port_mask[IPMC_IP_VERSION_MAX][VTSS_PORT_BF_SIZE];
static BOOL next_router_port_mask[IPMC_IP_VERSION_MAX][VTSS_PORT_BF_SIZE];
static u16 router_port_timer[IPMC_IP_VERSION_MAX];

static ipmc_port_throttling_t       ipmc_static_port_throttling[IPMC_IP_VERSION_MAX];
static ipmc_port_throttling_t       ipmc_current_port_throttling_status[IPMC_IP_VERSION_MAX];
static ipmc_port_group_filtering_t  ipmc_static_port_group_filtering[IPMC_IP_VERSION_MAX];

static u16 query_suppression_timer;
static u16 query_flooding_cnt;
static int ipmc_stp_port_status[VTSS_PORT_ARRAY_SIZE];

static BOOL vtss_ipmc_port_bf_set_with_throttling(ipmc_ip_version_t version, u8 bit_no, BOOL op, u8 *dst_ptr)
{
    BOOL    rc = FALSE;

    if (op) {   /* ADD */
        if (VTSS_PORT_BF_GET(dst_ptr, bit_no)) {
            return TRUE;
        } else {
            if (ipmc_static_port_throttling[version].max_no[bit_no] != 0) {
                if (ipmc_current_port_throttling_status[version].max_no[bit_no] < ipmc_static_port_throttling[version].max_no[bit_no]) {
                    ipmc_current_port_throttling_status[version].max_no[bit_no]++;
                } else {
                    return FALSE;
                }
            }
        }

        VTSS_PORT_BF_SET(dst_ptr, bit_no, TRUE);
        rc = TRUE;
    } else {            /* DEL */
        if (VTSS_PORT_BF_GET(dst_ptr, bit_no)) {
            if (ipmc_current_port_throttling_status[version].max_no[bit_no]) {
                ipmc_current_port_throttling_status[version].max_no[bit_no]--;
            }
        } else {
            return TRUE;
        }

        VTSS_PORT_BF_SET(dst_ptr, bit_no, FALSE);
        rc = TRUE;
    }

    return rc;
}

static i32 vtss_ipmc_mgroup_entry_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_entry_t  *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmc_mgroup_entry_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_group_entry_t *)elm1;
    element2 = (ipmc_group_entry_t *)elm2;

    if (element1->ipmc_version > element2->ipmc_version) {
        return 1;
    } else if (element1->ipmc_version < element2->ipmc_version) {
        return -1;
    } else if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
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

static i32 vtss_ipmcsnp_grp_rxmt_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_info_t   *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcsnp_grp_rxmt_tmr_cmp_func)");
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
        return vtss_ipmc_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);
    }
}

static i32 vtss_ipmcsnp_grp_fltr_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_group_db_t     *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcsnp_grp_fltr_tmr_cmp_func)");
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
        return vtss_ipmc_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);
    }
}

static i32 vtss_ipmcsnp_grp_srct_tmr_cmp_func(void *elm1, void *elm2)
{
    ipmc_sfm_srclist_t  *element1, *element2;
    ipmc_time_t         *tmr1, *tmr2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmcsnp_grp_srct_tmr_cmp_func)");
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
        int grp_cmp = vtss_ipmc_mgroup_entry_cmp_func((void *)element1->grp, (void *)element2->grp);

        if (grp_cmp > 0) {
            return 1;
        } else if (grp_cmp < 0) {
            return -1;
        } else {
            return ipmc_lib_srclist_cmp_func(elm1, elm2);
        }
    }
}

static i32 vtss_ipmc_proxy_report_entry_cmp_func(void *elm1, void *elm2)
{
    ipmc_proxy_report_entry_t   *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("IPMC_ASSERT(vtss_ipmc_proxy_report_entry_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_proxy_report_entry_t *)elm1;
    element2 = (ipmc_proxy_report_entry_t *)elm2;

    if (element1->ipmc_version > element2->ipmc_version) {
        return 1;
    } else if (element1->ipmc_version < element2->ipmc_version) {
        return -1;
    } else if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
        return -1;
    } else {
        int cmp;

        if (element1->ipmc_version == IPMC_IP_VERSION_IGMP) {
            cmp = IPMC_LIB_ADRS_CMP4(element1->group_address, element2->group_address);
        } else {
            cmp = IPMC_LIB_ADRS_CMP6(element1->group_address, element2->group_address);
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

/* DEBUG */
BOOL vtss_ipmc_debug_pkt_tx(ipmc_intf_entry_t *entry, ipmc_ctrl_pkt_t type, vtss_ipv6_t *dst, u8 idx, BOOL untag)
{
    BOOL                retVal;
    ipmc_group_entry_t  grp_buf;
    ipmc_port_bfs_t     fwd_mask;

    if (!entry || !dst) {
        return FALSE;
    }

    retVal = TRUE;
    switch ( type ) {
    case IPMC_PKT_TYPE_IGMP_GQ:
    case IPMC_PKT_TYPE_MLD_GQ:
        if (ipmc_lib_packet_tx_gq(entry, dst, untag, TRUE) != VTSS_OK) {
            retVal = FALSE;
        }

        break;
    case IPMC_PKT_TYPE_IGMP_SQ:
    case IPMC_PKT_TYPE_MLD_SQ:
        memset(&grp_buf, 0x0, sizeof(ipmc_group_entry_t));
        grp_buf.vid = entry->param.vid;
        grp_buf.ipmc_version = entry->ipmc_version;
        memcpy(&grp_buf.group_addr, dst, sizeof(vtss_ipv6_t));
        (void) ipmc_lib_group_get(snp_mgrp_tree, &grp_buf);
        if (ipmc_lib_packet_tx_sq(NULL, IPMC_SND_GO, &grp_buf, entry, idx, untag) != VTSS_OK) {
            retVal = FALSE;
        }

        break;
    case IPMC_PKT_TYPE_IGMP_SSQ:
    case IPMC_PKT_TYPE_MLD_SSQ:
        memset(&grp_buf, 0x0, sizeof(ipmc_group_entry_t));
        grp_buf.vid = entry->param.vid;
        grp_buf.ipmc_version = entry->ipmc_version;
        memcpy(&grp_buf.group_addr, dst, sizeof(vtss_ipv6_t));
        (void) ipmc_lib_group_get(snp_mgrp_tree, &grp_buf);
        if (ipmc_lib_packet_tx_sq(NULL, IPMC_SND_GO, &grp_buf, entry, idx, untag) != VTSS_OK) {
            retVal = FALSE;
        }

        break;
    case IPMC_PKT_TYPE_IGMP_V1JOIN:
    case IPMC_PKT_TYPE_IGMP_V2JOIN:
    case IPMC_PKT_TYPE_IGMP_V3JOIN:
    case IPMC_PKT_TYPE_MLD_V1REPORT:
    case IPMC_PKT_TYPE_MLD_V2REPORT:
        memset(&fwd_mask, 0x0, sizeof(ipmc_port_bfs_t));
        VTSS_PORT_BF_SET(fwd_mask.member_ports, idx, TRUE);
        if ((type == IPMC_PKT_TYPE_IGMP_V3JOIN) ||
            (type == IPMC_PKT_TYPE_MLD_V2REPORT)) {
            if (ipmc_lib_packet_tx_join_report(FALSE,
                                               VTSS_IPMC_COMPAT_MODE_AUTO,
                                               entry,
                                               dst,
                                               &fwd_mask,
                                               entry->ipmc_version,
                                               untag, FALSE, TRUE) != VTSS_OK) {
                retVal = FALSE;
            }
        } else {
            if (ipmc_lib_packet_tx_join_report(FALSE,
                                               VTSS_IPMC_COMPAT_MODE_GEN,
                                               entry,
                                               dst,
                                               &fwd_mask,
                                               entry->ipmc_version,
                                               untag, FALSE, TRUE) != VTSS_OK) {
                retVal = FALSE;
            }
        }

        break;
    case IPMC_PKT_TYPE_IGMP_LEAVE:
    case IPMC_PKT_TYPE_MLD_DONE:
        memset(&fwd_mask, 0x0, sizeof(ipmc_port_bfs_t));
        VTSS_PORT_BF_SET(fwd_mask.member_ports, idx, TRUE);
        if (ipmc_lib_packet_tx_group_leave(entry, dst, &fwd_mask, untag, FALSE) != VTSS_OK) {
            retVal = FALSE;
        }
        break;
    default:
        break;
    }

    return retVal;
}

static void _ipmc_clear_intf_statistics(ipmc_intf_entry_t *entry)
{
    ipmc_prot_intf_entry_param_t    *param;

    if (!entry || ((param = &entry->param) == NULL)) {
        return;
    }

    param->querier.ipmc_queries_sent = 0;
    param->querier.group_queries_sent = 0;
    memset(&param->stats, 0x0, sizeof(ipmc_statistics_t));
}

static void _ipmc_reset_intf_parameters(BOOL init, ipmc_intf_entry_t *entry)
{
    ipmc_prot_intf_entry_param_t    *param;
    ipmc_querier_sm_t               *querier;

    if (!entry || ((param = &entry->param) == NULL)) {
        return;
    }

    querier = &param->querier;
    if (init) {
        if (param->priority == IPMC_PARAM_PRIORITY_NULL) {
            param->priority = IPMC_PARAM_DEF_PRIORITY;
        }
        if (querier->QuerierAdrs4 == IPMC_PARAM_VALUE_NULL) {
            querier->QuerierAdrs4 = IPMC_QUERIER_ADDRESS4;
        }
        if (querier->RobustVari == IPMC_PARAM_VALUE_NULL) {
            querier->RobustVari = IPMC_QUERIER_ROBUST_VARIABLE;
            querier->LastQryCnt = IPMC_QUERIER_ROBUST_VARIABLE;
        }
        if (querier->QueryIntvl == IPMC_PARAM_VALUE_NULL) {
            querier->QueryIntvl = IPMC_QUERIER_QUERY_INTERVAL;
        }
        if (querier->MaxResTime == IPMC_PARAM_VALUE_NULL) {
            querier->MaxResTime = IPMC_QUERIER_MAX_RESP_TIME;
        }
        if (querier->LastQryItv == IPMC_PARAM_VALUE_NULL) {
            querier->LastQryItv = IPMC_QUERIER_LAST_Q_INTERVAL;
        }
        if (querier->UnsolicitR == IPMC_PARAM_VALUE_NULL) {
            querier->UnsolicitR = IPMC_QUERIER_UNSOLICIT_REPORT;
        }
        entry->proxy_report_timeout = 0;
        param->cfg_compatibility = IPMC_PARAM_DEF_COMPAT;
        param->rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
        param->rtr_compatibility.old_present_timer = 0;
        param->rtr_compatibility.gen_present_timer = 0;
        param->rtr_compatibility.sfm_present_timer = 0;
        param->hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_AUTO;
        param->hst_compatibility.old_present_timer = 0;
        param->hst_compatibility.gen_present_timer = 0;
        param->hst_compatibility.sfm_present_timer = 0;
    } else {
        entry->ipmc_version = IPMC_IP_VERSION_INIT;
        entry->proxy_report_timeout = 0;

        param->vid = 0;
        param->priority = IPMC_PARAM_PRIORITY_NULL;
        param->cfg_compatibility = IPMC_PARAM_DEF_COMPAT;
        param->rtr_compatibility.mode = IPMC_PARAM_DEF_COMPAT;
        param->rtr_compatibility.old_present_timer = 0;
        param->rtr_compatibility.gen_present_timer = 0;
        param->rtr_compatibility.sfm_present_timer = 0;
        param->hst_compatibility.mode = IPMC_PARAM_DEF_COMPAT;
        param->hst_compatibility.old_present_timer = 0;
        param->hst_compatibility.gen_present_timer = 0;
        param->hst_compatibility.sfm_present_timer = 0;
        IPMC_LIB_ADRS_SET(&param->active_querier, 0x0);

        querier->querier_enabled = FALSE;
        querier->state = IPMC_QUERIER_IDLE;
        querier->StartUpItv = IPMC_QUERIER_QUERY_INTERVAL / 4;
        querier->LastQryCnt = IPMC_QUERIER_ROBUST_VARIABLE;
        querier->OtherQuerierTimeOut = IPMC_TIMER_OQPT(entry);
        querier->QuerierAdrs4 = IPMC_PARAM_VALUE_NULL;

        querier->RobustVari = IPMC_PARAM_VALUE_NULL;
        querier->QueryIntvl = IPMC_PARAM_VALUE_NULL;
        querier->MaxResTime = IPMC_PARAM_VALUE_NULL;
        querier->LastQryItv = IPMC_PARAM_VALUE_NULL;
        querier->UnsolicitR = IPMC_PARAM_VALUE_NULL;

        querier->timeout = 0;
    }

    _ipmc_clear_intf_statistics(entry);
}

static ipmc_intf_entry_t *vtss_ipmc_new_intf_entry(vtss_vid_t vid, ipmc_ip_version_t ipmc_version)
{
    u16 i, igmp_cnt, mld_cnt;

    igmp_cnt = mld_cnt = 0;
    for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
        /* it's not a good idea to use index as a valid status */
        if (ipmc_vlan_entries[i].param.vid == 0) {
            if ((ipmc_version == IPMC_IP_VERSION_MLD) && (mld_cnt >= SNP_NUM_OF_INTF_PER_VERSION)) {
                return NULL;
            }
            if ((ipmc_version == IPMC_IP_VERSION_IGMP) && (igmp_cnt >= SNP_NUM_OF_INTF_PER_VERSION)) {
                return NULL;
            }

            ipmc_vlan_entries[i].ipmc_version = ipmc_version;
            ipmc_vlan_entries[i].param.vid = vid;
            _ipmc_reset_intf_parameters(TRUE, &ipmc_vlan_entries[i]);

            return &ipmc_vlan_entries[i];
        } else {
            if (ipmc_vlan_entries[i].ipmc_version == IPMC_IP_VERSION_IGMP) {
                igmp_cnt++;
            } else if (ipmc_vlan_entries[i].ipmc_version == IPMC_IP_VERSION_MLD) {
                mld_cnt++;
            }
        }
    }

    return NULL;
}

ipmc_intf_entry_t *vtss_ipmc_get_intf_entry(vtss_vid_t vid, ipmc_ip_version_t version)
{
    int i;

    if ((version != IPMC_IP_VERSION_IGMP) &&
        (version != IPMC_IP_VERSION_MLD)) {
        return NULL;
    }

    for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
        if ((ipmc_vlan_entries[i].param.vid == vid) &&
            (ipmc_vlan_entries[i].ipmc_version == version)) {
            return &ipmc_vlan_entries[i];
        }
    }

    return NULL;
}

ipmc_intf_entry_t *vtss_ipmc_get_next_intf_entry(vtss_vid_t vid, ipmc_ip_version_t version)
{
    int                 i;
    vtss_vid_t          vidTmp = 0;
    ipmc_intf_entry_t   *entry = NULL;

    for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
        if (ipmc_vlan_entries[i].ipmc_version != version) {
            continue;
        }

        /* it's not a good idea to use index as a valid status */
        if ((ipmc_vlan_entries[i].param.vid != 0) && (vid < ipmc_vlan_entries[i].param.vid)) {
            if (entry == NULL) {
                entry = &ipmc_vlan_entries[i];
                vidTmp = ipmc_vlan_entries[i].param.vid;
            } else {
                if (vidTmp > ipmc_vlan_entries[i].param.vid) {
                    entry = &ipmc_vlan_entries[i];
                    vidTmp = ipmc_vlan_entries[i].param.vid;
                }
            }
        }
    }

    return entry;
}

BOOL vtss_ipmc_get_intf_group_entry(vtss_vid_t vid, ipmc_group_entry_t *grp, ipmc_ip_version_t version)
{
    BOOL                found = FALSE;

    if (!grp) {
        return FALSE;
    }

    if (vtss_ipmc_get_intf_entry(vid, version) != NULL) {
        grp->vid = vid;
        grp->ipmc_version = version;
        if (ipmc_lib_group_get(snp_mgrp_tree, grp)) {
            found = TRUE;
        }
    }

    return found;
}

BOOL vtss_ipmc_get_next_intf_group_entry(vtss_vid_t vid, ipmc_group_entry_t *grp, ipmc_ip_version_t version)
{
    i8      buf[40];
    BOOL    found = FALSE;

    if (!grp) {
        return FALSE;
    }

    T_D("vtss_ipmc_get_next_intf_group_entry VID:%u/VER:%d, group_address = %s", vid, version, misc_ipv6_txt(&grp->group_addr, buf));

    if (vtss_ipmc_get_intf_entry(vid, version) != NULL) {
        grp->vid = vid;
        grp->ipmc_version = version;
        while (ipmc_lib_group_get_next(snp_mgrp_tree, grp)) {
            if (version != grp->ipmc_version) {
                continue;
            }

            if (grp->vid == vid) {
                found = TRUE;
            }

            break;
        }
    }

    return found;
}

static void set_querying_enable(ipmc_intf_entry_t *ipmc_intf, BOOL mode)
{
    if (ipmc_intf == NULL) {
        return;
    }

    if (ipmc_intf->param.querier.querier_enabled != mode) {
        ipmc_intf->param.querier.querier_enabled = mode;
        ipmc_intf->param.querier.state = IPMC_QUERIER_IDLE;
        IPMC_LIB_ADRS_SET(&ipmc_intf->param.active_querier, 0x0);

        if (mode) {
            ipmc_intf->param.querier.state = IPMC_QUERIER_INIT;
            ipmc_intf->param.querier.timeout = 0;

            ipmc_intf->param.querier.QuerierUpTime = 0;
            ipmc_intf->param.querier.OtherQuerierTimeOut = 0;

            ipmc_intf->param.querier.StartUpItv = IPMC_TIMER_SQI(ipmc_intf);
            ipmc_intf->param.querier.StartUpCnt = IPMC_TIMER_SQC(ipmc_intf);
            ipmc_intf->param.querier.LastQryCnt = IPMC_TIMER_LLQC(ipmc_intf);
        } else {
            ipmc_intf->param.querier.timeout = IPMC_TIMER_QI(ipmc_intf);

            ipmc_intf->param.querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(ipmc_intf);
        }

        ipmc_intf->param.querier.proxy_query_timeout = 0x0;
    }
}

static void _ipmc_clear_intf_running_tables(ipmc_intf_entry_t *entry)
{
    vtss_vid_t          vid;
    ipmc_ip_version_t   version;
    ipmc_group_entry_t  *grp, *grp_ptr;
    BOOL                proxy_status;

    if (!entry) {
        return;
    }

    version = entry->ipmc_version;
    vid = entry->param.vid;
    proxy_status = (ipmc_leave_proxy_enabled[version] || ipmc_proxy_enabled[version]);
    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(snp_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if (grp->ipmc_version != version) {
            continue;
        }

        if (grp->vid == vid) {
            ipmc_group_db_t *grp_db = &grp->info->db;
            port_iter_t     inpit;
            u32             chk;

            (void) port_iter_init(&inpit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&inpit)) {
                chk = inpit.iport;
                if (!VTSS_PORT_BF_GET(grp_db->port_mask, chk) ||
                    !ipmc_current_port_throttling_status[version].max_no[chk]) {
                    continue;
                }

                --ipmc_current_port_throttling_status[version].max_no[chk];
            }
            VTSS_PORT_BF_CLR(grp_db->port_mask);
            /* force deletion of group regardless of other groups */
            (void) ipmc_lib_group_delete(entry, snp_mgrp_tree, snp_rxmt_tree, snp_fltr_tree, snp_srct_tree, grp, proxy_status, TRUE);
        }
    }
}

ipmc_intf_entry_t *vtss_ipmc_set_intf_entry(vtss_vid_t vid, BOOL state, BOOL querier, ipmc_port_bfs_t *vlan_ports, ipmc_ip_version_t ipmc_version)
{
    BOOL                new_intf;
    ipmc_intf_entry_t   *entry;

    new_intf = FALSE;
    /* look if already present, and re-use it if available */
    entry = vtss_ipmc_get_intf_entry(vid, ipmc_version);
    if (entry == NULL) {
        /* look for unused entry */
        entry = vtss_ipmc_new_intf_entry(vid, ipmc_version);
        if (entry == NULL) {
            if (ipmc_version == IPMC_IP_VERSION_MLD) {
                T_D("\n\rThe IPMC Interface Table for MLD is full !!!");
            } else {
                T_D("\n\rThe IPMC Interface Table for IGMP is full !!!");
            }

            /* no free entries */
            return NULL;
        }

        new_intf = TRUE;
        /* Start IPMC (snooping) in this interface (VLAN) */
    }

    /* Always update vlan_ports of ipmc_intf_entry_t */
    if (vlan_ports) {
        port_iter_t pit;
        u32         i;

        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            VTSS_PORT_BF_SET(entry->vlan_ports, i, VTSS_PORT_BF_GET(vlan_ports->member_ports, i));
        }

#if VTSS_SWITCH_STACKABLE
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(entry->vlan_ports, i, TRUE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(entry->vlan_ports, i, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */
    }

    if (!new_intf) {
        if (IPMC_LIB_BFS_HAS_MEMBER(entry->vlan_ports)) {
            _ipmc_clear_intf_running_tables(entry);
        }

        if (!state) {
            _ipmc_clear_intf_statistics(entry);
        }
    }

    /* always update this parameter */
    entry->op_state = state;
    T_D("vid:%d/state:%s/ver:%d", vid, state ? "TRUE" : "FALSE", ipmc_version);

    set_querying_enable(entry, querier);

    return entry;
}

void vtss_ipmc_del_intf_entry(vtss_vid_t vid, ipmc_ip_version_t ipmc_version)
{
    ipmc_intf_entry_t   *entry;

    entry = vtss_ipmc_get_intf_entry(vid, ipmc_version);
    if (entry != NULL) {
        _ipmc_clear_intf_running_tables(entry);

        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), TRUE, TRUE, NULL);
        ipmc_lib_proc_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(entry), TRUE, TRUE, NULL);

        /* and finally clear the entry */
        _ipmc_reset_intf_parameters(FALSE, entry);
    }
}

BOOL vtss_ipmc_upd_intf_entry(ipmc_prot_intf_basic_t *intf_entry, ipmc_ip_version_t ipmc_version)
{
    ipmc_intf_entry_t   *entry;

    entry = vtss_ipmc_get_intf_entry(intf_entry->vid, ipmc_version);
    if (entry == NULL) {
        return FALSE;
    }

    /* Parameter Validation */
    if (intf_entry->querier4_address) {
        u8  adrc = (intf_entry->querier4_address >> 24) & 0xFF;

        if ((adrc == 127) || (adrc > 223)) {
            return FALSE;
        }
    }
    if (intf_entry->priority > IPMC_PARAM_MAX_PRIORITY) {
        return FALSE;
    }
    if (intf_entry->robustness_variable == 0) {
        /* RFC-3810 9.1 */
        return FALSE;
    }
    if (intf_entry->query_response_interval >= (intf_entry->query_interval * 10)) {
        /* RFC-3810 9.3 */
        return FALSE;
    }

    if ((ipmc_version == IPMC_IP_VERSION_MLD) &&
        (intf_entry->compatibility == VTSS_IPMC_COMPAT_MODE_OLD)) {
        /* MLD shouldn't have this compatibility mode */
        return FALSE;
    }

    switch ( intf_entry->compatibility ) {
    case VTSS_IPMC_COMPAT_MODE_AUTO:
    case VTSS_IPMC_COMPAT_MODE_GEN:
    case VTSS_IPMC_COMPAT_MODE_SFM:
        break;
    case VTSS_IPMC_COMPAT_MODE_OLD:
        if (ipmc_version == IPMC_IP_VERSION_MLD) {
            return FALSE;
        }

        break;
    default:
        return FALSE;
    }
    if (intf_entry->compatibility != entry->param.cfg_compatibility) {
        ipmc_compat_mode_t  compatibility;

        entry->param.cfg_compatibility = intf_entry->compatibility;
        compatibility = IPMC_COMPATIBILITY(entry);
        if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
            entry->param.rtr_compatibility.mode = entry->param.cfg_compatibility;
            entry->param.hst_compatibility.mode = entry->param.cfg_compatibility;
        } else {
            entry->param.rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
            entry->param.hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_AUTO;
        }

        entry->param.rtr_compatibility.old_present_timer = 0;
        entry->param.rtr_compatibility.gen_present_timer = 0;
        entry->param.rtr_compatibility.sfm_present_timer = 0;
        entry->param.hst_compatibility.old_present_timer = 0;
        entry->param.hst_compatibility.gen_present_timer = 0;
        entry->param.hst_compatibility.sfm_present_timer = 0;
    }

    if (intf_entry->querier4_address != entry->param.querier.QuerierAdrs4) {
        entry->param.querier.QuerierAdrs4 = intf_entry->querier4_address;
    }
    if (intf_entry->priority != entry->param.priority) {
        entry->param.priority = intf_entry->priority;
    }
    if (intf_entry->robustness_variable != entry->param.querier.RobustVari) {
        entry->param.querier.RobustVari = intf_entry->robustness_variable;
        entry->param.querier.LastQryCnt = intf_entry->robustness_variable;
    }
    if (intf_entry->query_interval != entry->param.querier.QueryIntvl) {
        entry->param.querier.QueryIntvl = intf_entry->query_interval;
    }
    if (intf_entry->query_response_interval != entry->param.querier.MaxResTime) {
        entry->param.querier.MaxResTime = intf_entry->query_response_interval;
    }
    if (intf_entry->last_listener_query_interval != entry->param.querier.LastQryItv) {
        entry->param.querier.LastQryItv = intf_entry->last_listener_query_interval;
    }
    if (intf_entry->unsolicited_report_interval != entry->param.querier.UnsolicitR) {
        entry->param.querier.UnsolicitR = intf_entry->unsolicited_report_interval;
    }

    return TRUE;
}

static void ipmc_update_router_port(ipmc_ip_version_t version)
{
    ipmc_group_entry_t  *grp, *grp_ptr;
    ipmc_group_db_t     *grp_db;
    BOOL                proxy_status;

    proxy_status = (ipmc_leave_proxy_enabled[version] || ipmc_proxy_enabled[version]);
    /* we must go through all groups */
    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(snp_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if (grp->ipmc_version != version) {
            continue;
        }

        grp_db = &grp->info->db;
        /* delete entry if no more ports */
        if (IPMC_LIB_BFS_EMPTY(grp_db->port_mask)) {
            port_iter_t inpit;
            u32         chk;

            (void) port_iter_init(&inpit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&inpit)) {
                chk = inpit.iport;
                if (!VTSS_PORT_BF_GET(grp_db->port_mask, chk) ||
                    !ipmc_current_port_throttling_status[version].max_no[chk]) {
                    continue;
                }

                --ipmc_current_port_throttling_status[version].max_no[chk];
            }

            (void) ipmc_lib_group_delete(vtss_ipmc_get_intf_entry(grp->vid, grp->ipmc_version),
                                         snp_mgrp_tree,
                                         snp_rxmt_tree,
                                         snp_fltr_tree,
                                         snp_srct_tree,
                                         grp,
                                         proxy_status,
                                         FALSE);
        } else {
            /* replace the MAC entry (if different) */
            (void) ipmc_lib_group_update(snp_mgrp_tree, grp);
        }
    }
}

static void ipmc_del_group_member_with_lnk(ipmc_ip_version_t version, u8 idx)
{
    ipmc_group_entry_t  *grp, *grp_ptr;
    ipmc_group_db_t     *grp_db;
    ipmc_bf_status      bfs;
    ipmc_intf_entry_t   *intf;
    BOOL                proxy_status;
    ipmc_time_t         snptmr_with_lnk;

    proxy_status = (ipmc_leave_proxy_enabled[version] || ipmc_proxy_enabled[version]);
    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(snp_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if ((version != IPMC_IP_VERSION_ALL) && (grp->ipmc_version != version)) {
            continue;
        }

        grp_db = &grp->info->db;
        if (!VTSS_PORT_BF_GET(grp_db->port_mask, idx)) {
            continue;
        }

        (void)vtss_ipmc_port_bf_set_with_throttling(grp->ipmc_version, idx, FALSE, grp_db->port_mask);
        VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, idx, VTSS_IPMC_SF_STATUS_DISABLED);

        intf = vtss_ipmc_get_intf_entry(grp->vid, grp->ipmc_version);
        /* delete entry if no more ports */
        bfs = ipmc_lib_bf_status_check(grp_db->port_mask);
        if (bfs == IPMC_BF_EMPTY) {
            (void) ipmc_lib_group_delete(intf,
                                         snp_mgrp_tree,
                                         snp_rxmt_tree,
                                         snp_fltr_tree,
                                         snp_srct_tree,
                                         grp,
                                         proxy_status,
                                         FALSE);
        } else {
            port_iter_t         pit;
            int                 i;
            ipmc_time_t         *llqt = &snptmr_with_lnk;

            if (intf != NULL) {
                IPMC_TIMER_LLQT_GET(intf, llqt);
            }

            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(grp_db->port_mask, i) && IPMC_TIMER_LESS(&grp_db->tmr.fltr_timer.t[i], llqt)) {
                    (void)vtss_ipmc_port_bf_set_with_throttling(grp->ipmc_version, i, FALSE, grp_db->port_mask);
                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, i, VTSS_IPMC_SF_STATUS_DISABLED);
                }
            }

            /* replace the MAC entry (if different) */
            (void) ipmc_lib_group_update(snp_mgrp_tree, grp);

            if (bfs == IPMC_BF_SEMIEMPTY) {
                /* TO-DO */
            }
        }
    }
}

static void ipmc_del_group_member_with_num(ipmc_ip_version_t version, u8 idx, u8 diff)
{
    ipmc_group_entry_t  *grp, *grp_ptr;
    ipmc_group_db_t     *grp_db;
    u8                  cnt;
    ipmc_intf_entry_t   *intf;
    BOOL                proxy_status;
    ipmc_time_t         tmr_with_num;

    proxy_status = (ipmc_leave_proxy_enabled[version] || ipmc_proxy_enabled[version]);
    cnt = diff;
    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(snp_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if (grp->ipmc_version != version) {
            continue;
        }

        grp_db = &grp->info->db;
        if (!VTSS_PORT_BF_GET(grp_db->port_mask, idx)) {
            continue;
        }

        if (!cnt) {
            break;
        } else {
            cnt--;
        }

        (void)vtss_ipmc_port_bf_set_with_throttling(version, idx, FALSE, grp_db->port_mask);
        VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, idx, VTSS_IPMC_SF_STATUS_DISABLED);

        intf = vtss_ipmc_get_intf_entry(grp->vid, grp->ipmc_version);
        /* delete entry if no more ports */
        if (IPMC_LIB_BFS_EMPTY(grp_db->port_mask)) {
            (void) ipmc_lib_group_delete(intf,
                                         snp_mgrp_tree,
                                         snp_rxmt_tree,
                                         snp_fltr_tree,
                                         snp_srct_tree,
                                         grp,
                                         proxy_status,
                                         FALSE);
        } else {
            port_iter_t         pit;
            int                 i;
            ipmc_time_t         *llqt = &tmr_with_num;

            if (intf != NULL) {
                IPMC_TIMER_LLQT_GET(intf, llqt);
            }

            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(grp_db->port_mask, i) && IPMC_TIMER_LESS(&grp_db->tmr.fltr_timer.t[i], llqt)) {
                    (void)vtss_ipmc_port_bf_set_with_throttling(version, i, FALSE, grp_db->port_mask);
                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, i, VTSS_IPMC_SF_STATUS_DISABLED);
                }
            }

            /* replace the MAC entry (if different) */
            (void) ipmc_lib_group_update(snp_mgrp_tree, grp);
        }
    }
}

static BOOL ipmc_proxy_proc_specific_group(ipmc_intf_entry_t *intf, vtss_ipv6_t *group_addr, BOOL leave)
{
    ipmc_proxy_report_entry_t   *pr_ptr, *pr_ptr_bak;

    if (!intf || !group_addr) {
        return FALSE;
    }
    if (!IPMC_MEM_SYSTEM_MTAKE(pr_ptr, sizeof(ipmc_proxy_report_entry_t))) {
        return FALSE;
    }
    pr_ptr_bak = pr_ptr;

    pr_ptr->ipmc_version = intf->ipmc_version;
    pr_ptr->vid = intf->param.vid;
    memcpy(&pr_ptr->group_address, group_addr, sizeof(vtss_ipv6_t));
    pr_ptr->leave = leave;
    pr_ptr->compat = intf->param.rtr_compatibility.mode;
    if (!IPMC_LIB_DB_SET(snp_prxy_tree, pr_ptr)) {
        i8  buf[40];

        T_D("Failure in creating %s Proxy %s for %s in vid-%u",
            ipmc_lib_version_txt(intf->ipmc_version, IPMC_TXT_CASE_UPPER),
            leave ? "Leave/Done" : "Join/Report",
            misc_ipv6_txt(group_addr, buf),
            intf->param.vid);

        IPMC_MEM_SYSTEM_MGIVE(pr_ptr_bak);
    }

    return TRUE;
}

/*
    vtss_ipmc_upd_unknown_fwdmsk - Update unknown flooding mask.
*/
void vtss_ipmc_upd_unknown_fwdmsk(ipmc_ip_version_t ipmc_version)
{
    BOOL                member[VTSS_PORT_ARRAY_SIZE];
    ipmc_ip_version_t   ver_idx, ver_end;
    ipmc_activate_t     action;
    port_iter_t         pit;
    u32                 idx;

    if (ipmc_version == IPMC_IP_VERSION_ALL) {
        ver_idx = IPMC_IP_VERSION_IGMP;
        ver_end = IPMC_IP_VERSION_MLD;
    } else {
        ver_idx = ver_end = ipmc_version;
    }

    for (; ver_idx <= ver_end; ver_idx++) {
#if VTSS_SWITCH_STACKABLE
        action = IPMC_ACTIVATE_RUN;
#else
        if (ipmc_unreg_flood_enabled[ver_idx]) {
            action = IPMC_ACTIVATE_RUN;
        } else {
            if (ipmc_enabled[ver_idx]) {
                action = IPMC_ACTIVATE_RUN;
            } else {
                action = IPMC_ACTIVATE_OFF;
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */

        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            idx = pit.iport;

            if (ipmc_unreg_flood_enabled[ver_idx]) {
                member[idx] = TRUE;
            } else {
                if (ipmc_enabled[ver_idx]) {
                    member[idx] = VTSS_PORT_BF_GET(static_router_port_mask[ver_idx], idx);
                    member[idx] |= ipmc_lib_get_port_rpstatus(ver_idx, idx);
                } else {
                    member[idx] = FALSE;
                }
            }
        }

#if VTSS_SWITCH_STACKABLE
        idx = PORT_NO_STACK_0;
        member[idx] = TRUE;
        idx = PORT_NO_STACK_1;
        member[idx] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

        switch ( ver_idx ) {
        case IPMC_IP_VERSION_IGMP:
            if (!ipmc_lib_unregistered_flood_set(IPMC_OWNER_SNP4, action, member)) {
                T_W("Failure in ipmc_lib_unregistered_flood_set");
            }
#ifdef VTSS_SW_OPTION_MVR
#ifndef VTSS_SW_OPTION_SMB_IPMC
            if (ipmc_enabled[ver_idx] && !ipmc_unreg_flood_enabled[ver_idx]) {
                (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                while (port_iter_getnext(&pit)) {
                    member[pit.iport] = TRUE;
                }
            }

            if (!ipmc_lib_unregistered_flood_set(IPMC_OWNER_SNP6, action, member)) {
                T_W("Failure in ipmc_lib_unregistered_flood_set");
            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
#endif /* VTSS_SW_OPTION_MVR */

            break;
        case IPMC_IP_VERSION_MLD:
            if (!ipmc_lib_unregistered_flood_set(IPMC_OWNER_SNP6, action, member)) {
                T_W("Failure in ipmc_lib_unregistered_flood_set");
            }

            break;
        default:

            break;
        }
    }
}

/*
    Port state callback function
    This function is called if a LOCAL port state change occur.
*/
void vtss_ipmc_port_state_change_handle(vtss_port_no_t port_no, port_info_t *info)
{
    T_D("vtss_ipmc_port_state_change_handle->port_no: %u. link %s\n", port_no, info->link ? "up" : "down");

    if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, port_no)) {
        return;
    }

    if (info->link == 0) {
        /* clear router when it links down */
        ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, port_no, FALSE);
        ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_MLD, port_no, FALSE);
        VTSS_PORT_BF_SET(next_router_port_mask[IPMC_IP_VERSION_IGMP], port_no, FALSE);
        VTSS_PORT_BF_SET(next_router_port_mask[IPMC_IP_VERSION_MLD], port_no, FALSE);

        ipmc_del_group_member_with_lnk(IPMC_IP_VERSION_ALL, port_no);

        /* update iflodmsk */
        vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_IGMP);
        vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_MLD);
    } else {
        /* first restore (static) router when link comes back */
        if (VTSS_PORT_BF_GET(static_router_port_mask[IPMC_IP_VERSION_IGMP], port_no)) {
            ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, port_no, TRUE);
        }
        if (VTSS_PORT_BF_GET(static_router_port_mask[IPMC_IP_VERSION_MLD], port_no)) {
            ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_MLD, port_no, TRUE);
        }
    }
}

/*
    STP-Port state callback function
    This function is called if a LOCAL STP state change occur.
*/
void vtss_ipmc_stp_port_state_change_handle(ipmc_ip_version_t version, vtss_port_no_t port_no, vtss_common_stpstate_t new_state)
{
    T_D("vtss_ipmc_stp_port_state_change_handle->port_no: %u  link %s\n", port_no, (new_state == VTSS_STP_STATE_DISCARDING) ? "STP_STATE_DISCARDING" : "STP_STATE_FORWARDING");

    if ((ipmc_stp_port_status[port_no] == VTSS_COMMON_STPSTATE_DISCARDING) && (new_state == VTSS_COMMON_STPSTATE_FORWARDING)) {
        ipmc_port_bfs_t     port_bit_mask_tmp;
        ipmc_intf_entry_t   *entry;
        u16                 i;
        vtss_ipv6_t         dst_ip6;

        VTSS_PORT_BF_CLR(port_bit_mask_tmp.member_ports);
        VTSS_PORT_BF_SET(port_bit_mask_tmp.member_ports, port_no, TRUE);

        i = 0;
        while ((entry = vtss_ipmc_get_next_intf_entry(i, version)) != NULL) {
            i = entry->param.vid;
            version = entry->ipmc_version;

            if (VTSS_PORT_BF_GET(entry->vlan_ports, port_no) && VTSS_PORT_BF_GET(port_bit_mask_tmp.member_ports, port_no)) {
                switch ( entry->param.querier.state ) {
                case IPMC_QUERIER_ACTIVE:
                    ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                    (void) ipmc_lib_packet_tx_helping_query(entry, &dst_ip6, &port_bit_mask_tmp, FALSE, FALSE);

                    break;
                case IPMC_QUERIER_IDLE:
                    if (ipmc_proxy_enabled[version]) {
                        ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                        (void) ipmc_lib_packet_tx_helping_query(entry, &dst_ip6, &port_bit_mask_tmp, FALSE, FALSE);
                    }

                    break;
                case IPMC_QUERIER_INIT:
                default:

                    break;
                }
            }
        }
    }

    ipmc_stp_port_status[port_no] = new_state;
}

void vtss_ipmc_process_glag(ulong port, vtss_vid_t vid, const uchar *const frame, ulong frame_len, ipmc_ip_version_t ipmc_version)
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

    if (ipmc_enabled[ipmc_version]) {
        ipmc_group_entry_t  grp_buf, *grp;
        vtss_ipv4_t         ip4sip, ip4dip;
        vtss_ipv6_t         ip6sip, ip6dip;
        BOOL                fwd_map[VTSS_PORT_ARRAY_SIZE] = {FALSE};

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

        if ((grp = ipmc_lib_group_ptr_get(snp_mgrp_tree, &grp_buf)) != NULL) {
            port_iter_t pit;
            u32         i;

            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(grp->info->db.port_mask, i) ||
                    ipmc_lib_get_port_rpstatus(ipmc_version, i) ||
                    VTSS_PORT_BF_GET(static_router_port_mask[ipmc_version], i)) {
                    fwd_map[i] = TRUE;
                }
            }

#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            fwd_map[i] = TRUE;
            i = PORT_NO_STACK_1;
            fwd_map[i] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

            /* ASM-UPDATE(ADD) */
            if (ipmc_version == IPMC_IP_VERSION_MLD) {
                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
                memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
            } else {
                ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
                memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
            }

            if (ipmc_lib_porting_set_chip(TRUE, snp_mgrp_tree, grp, ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map) != VTSS_OK) {
                T_D("ipmc_lib_porting_set_chip ADD failed");
            }
            /* SHOULD Notify MC-Routing + */
        }
    }
}

static void ipmc_get_router_port_mask(ipmc_ip_version_t version, u16 port_no, ipmc_port_bfs_t *port_mask)
{
    port_iter_t pit;
    u32         i;

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        VTSS_PORT_BF_SET(port_mask->member_ports, i, FALSE);
        if (i != port_no) {
            VTSS_PORT_BF_SET(port_mask->member_ports,
                             i,
                             (VTSS_PORT_BF_GET(static_router_port_mask[version], i) | ipmc_lib_get_port_rpstatus(version, i)));
        }
    }
}

void vtss_ipmc_calculate_dst_ports(vtss_vid_t vid, uchar port_no, ipmc_port_bfs_t *port_mask, ipmc_ip_version_t version)
{
    port_iter_t         pit;
    u32                 i;
    ipmc_intf_entry_t   *entry;

    entry = vtss_ipmc_get_intf_entry(vid, version);
    if (entry != NULL) {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (i == port_no) {
                VTSS_PORT_BF_SET(port_mask->member_ports, i, FALSE);
            } else {
                VTSS_PORT_BF_SET(port_mask->member_ports, i, VTSS_PORT_BF_GET(entry->vlan_ports, i));
            }
        }
    }

#if VTSS_SWITCH_STACKABLE
    if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, port_no)) {
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(port_mask->member_ports, i, TRUE);
    }
#endif /* VTSS_SWITCH_STACKABLE */
}

static void update_mld_statistics(ipmc_intf_entry_t *entry, ipmc_mld_packet_t *mld)
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

static void update_igmp_statistics(ipmc_intf_entry_t *entry, ipmc_igmp_packet_t *igmp)
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

#ifdef VTSS_SW_OPTION_PACKET
static int vtss_snp_get_report_throttling(ipmc_ip_version_t version, u8 *content, u8 msgType, u32 port)
{
    ipmc_mld_packet_t   *mld;
    ipmc_igmp_packet_t  *igmp;
    u16                 retVal = IPMC_REPORT_NORMAL;
    u32                 static_throttling = ipmc_static_port_throttling[version].max_no[port];
    u32                 current_throttling = ipmc_current_port_throttling_status[version].max_no[port];

    T_D("CUR/CFG-THROTTLED:%u/%u", current_throttling, static_throttling);
    switch ( msgType ) {
    case IPMC_IGMP_MSG_TYPE_V1JOIN:
    case IPMC_MLD_MSG_TYPE_V1REPORT:
    case IPMC_IGMP_MSG_TYPE_V2JOIN:
        if (static_throttling != 0) {
            if (current_throttling >= static_throttling) {
                return IPMC_REPORT_THROTTLED;
            } else {
                retVal = 0x1;
            }
        }

        break;
    case IPMC_MLD_MSG_TYPE_V2REPORT:
    case IPMC_IGMP_MSG_TYPE_V3JOIN:
        if (static_throttling != 0) {
            if (current_throttling < static_throttling) {
                if (version == IPMC_IP_VERSION_IGMP) {
                    igmp = (ipmc_igmp_packet_t *)content;
                    retVal = ntohs(igmp->sfminfo.sfm_report.number_of_record);
                } else {
                    mld = (ipmc_mld_packet_t *)content;
                    retVal = ntohs(mld->common.number_of_record);
                }

                if ((current_throttling + (u32)retVal) > static_throttling) {
                    retVal = static_throttling - current_throttling;
                }
            } else {
                return IPMC_REPORT_THROTTLED;
            }
        }

        break;
    case IPMC_MLD_MSG_TYPE_DONE:
    case IPMC_IGMP_MSG_TYPE_LEAVE:
    default:

        break;
    }

    return (int)retVal;
}

static ipmc_pkt_attribute_t snp_pkt_attrib;
static void ipmc_specific_port_fltr_set(ipmc_ip_version_t version, specific_grps_fltr_t *fltr)
{
    u32                         pdx;
    ipmc_port_group_filtering_t *grp_filter_ptr;

    if (!fltr) {
        return;
    }

#if IPMC_LIB_FLTR_MULTIPLE_PROFILE
#else
    grp_filter_ptr = &ipmc_static_port_group_filtering[version];
    pdx = grp_filter_ptr->profile_index[fltr->port];
    if (pdx) {
        fltr->pdx[0] = pdx;
        ++fltr->filter_cnt;
    }
#endif /* IPMC_LIB_FLTR_MULTIPLE_PROFILE */
}

static ipmc_group_entry_t   grp_rxi_snp;
vtss_rc RX_ipmcsnp(ipmc_ip_version_t version, void *contxt, const u8 *const frame, const vtss_packet_rx_info_t *const rx_info, ipmc_port_bfs_t *ret_fwd)
{
    vtss_rc                     rc;
    uchar                       *content;
    ipmc_intf_entry_t           *entry;

    u32                         i, local_port_cnt, magic, src_port, frame_len;
    vtss_vid_t                  vid;
    char                        buf[40];
    ipmc_group_entry_t          *grp;
    ipmc_group_info_t           *grp_info;
    ipmc_group_db_t             *grp_db;
    specific_grps_fltr_t        ipmc_specific_port_fltr;
    BOOL                        from_fast_leave;

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    rc = VTSS_OK;
    frame_len = rx_info->length;
    src_port = rx_info->port_no;
    vid = rx_info->tag.vid;

    magic = 0;
    from_fast_leave = FALSE;
    entry = vtss_ipmc_get_intf_entry(vid, version);
    if (entry == NULL) {
        for (i = 0; i < local_port_cnt; i++) {
            if (i != src_port) {
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, TRUE);
            } else {
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, FALSE);
            }
        }

        return IPMC_ERROR_VLAN_NOT_FOUND;
    } else {
        if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port)) {
            memcpy((u8 *)&magic, frame + (frame_len - IPMC_PKT_PRIVATE_PAD_LEN), IPMC_PKT_PRIVATE_PAD_LEN);

            magic = htonl(magic);
            if (((magic >> 8) << 8) == IPMC_PKT_PRIVATE_PAD_MAGIC) {
                if (magic & 0xF0) {
                    from_fast_leave = TRUE;
                }
            }
        }

        memset(&snp_pkt_attrib, 0x0, sizeof(ipmc_pkt_attribute_t));
        rc = ipmc_lib_packet_rx(entry, frame, rx_info, &snp_pkt_attrib);
    }

    if (rc == VTSS_OK && ipmc_enabled[version]) {
        ipmc_operation_action_t grp_op;
        ipmc_prefix_t           grp_prefix, ssm_ver_prefix;
        int                     report_throttling, actual_throttling;
        u16                     ref_timeout;
        BOOL                    sfmQuery = FALSE, oldQuery = FALSE;
        BOOL                    proxy_status, has_grp, pass_compat, in_range, update_statistics = TRUE;
        ipmc_compat_mode_t      compatibility = IPMC_COMPATIBILITY(entry);
        BOOL                    from_stacking = port_isid_port_no_is_stack(VTSS_ISID_LOCAL, src_port);
#if VTSS_SWITCH_STACKABLE
        BOOL                    incl_stacking = !from_stacking;
#endif /* VTSS_SWITCH_STACKABLE */

        T_D_HEX(frame, frame_len);

        switch ( snp_pkt_attrib.msgType ) {
        case IPMC_MLD_MSG_TYPE_QUERY:
        case IPMC_IGMP_MSG_TYPE_QUERY:
            if (version == IPMC_IP_VERSION_MLD) {
                if (snp_pkt_attrib.ipmc_pkt_len >= MLD_SFM_MIN_PAYLOAD_LEN) {
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
                    break;
                }
            } else if (version == IPMC_IP_VERSION_IGMP) {
                if (snp_pkt_attrib.ipmc_pkt_len >= IGMP_SFM_MIN_PAYLOAD_LEN) {
                    sfmQuery = TRUE;
                } else {
                    if (!snp_pkt_attrib.max_resp_time) {
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
                    break;
                }
            } else {
                T_D("Incorrect version(%u)!!!", version);
                break;
            }

            if (!from_stacking && pass_compat) {
                T_D("IGMP/MLD query len is %d (resp = %d)",
                    (int)(frame_len - snp_pkt_attrib.offset),
                    (int)snp_pkt_attrib.max_resp_time);
                /* src_port is a router port */
                ipmc_lib_set_discovered_router_port_mask(version, src_port, TRUE);

                /* remember this one */
                VTSS_PORT_BF_SET(next_router_port_mask[version], src_port, TRUE);

                /* update iflodmsk */
                vtss_ipmc_upd_unknown_fwdmsk(version);

                if (!ipmc_lib_isaddr6_all_zero(&snp_pkt_attrib.src_ip_addr)) {
                    if (snp_pkt_attrib.msgType == IPMC_MLD_MSG_TYPE_QUERY) {
                        vtss_ipv6_t ipLinkLocalSrc;

                        /* get src address */
                        if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                            if (memcmp(&snp_pkt_attrib.src_ip_addr, &ipLinkLocalSrc, sizeof(vtss_ipv6_t)) < 0) {
                                entry->param.querier.state = IPMC_QUERIER_IDLE;

                                IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                entry->param.querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(entry);
                            } else {
                                if (!entry->param.querier.querier_enabled) {
                                    if (ipmc_lib_isaddr6_all_zero(&entry->param.active_querier)) {
                                        IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                    } else {
                                        if (IPMC_LIB_ADRS_CMP6(snp_pkt_attrib.src_ip_addr, entry->param.active_querier) < 0) {
                                            IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                        }
                                    }
                                }
                            }
                        } else {
                            T_D("Get Current IPv6 Address failed !!!");
                        }
                    }

                    if (snp_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_QUERY) {
                        vtss_ipv4_t ip4addr = 0;

                        /* get src address */
                        if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                            ip4addr = htonl(ip4addr);
                            if (memcmp(&snp_pkt_attrib.src_ip_addr.addr[12], &ip4addr, sizeof(ipmcv4addr)) < 0) {
                                entry->param.querier.state = IPMC_QUERIER_IDLE;

                                IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                entry->param.querier.OtherQuerierTimeOut = IPMC_TIMER_OQPT(entry);
                            } else {
                                if (!entry->param.querier.querier_enabled) {
                                    if (ipmc_lib_isaddr6_all_zero(&entry->param.active_querier)) {
                                        IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                    } else {
                                        if (IPMC_LIB_ADRS_CMP4(snp_pkt_attrib.src_ip_addr, entry->param.active_querier) < 0) {
                                            IPMC_LIB_ADRS_CPY(&entry->param.active_querier, &snp_pkt_attrib.src_ip_addr);
                                        }
                                    }
                                }
                            }
                        } else {
                            T_D("Get Current IPv4 Address failed !!!");
                        }
                    }
                }
            } /* if (!from_stacking && pass_compat) */

            query_flooding_cnt++;
            if (query_flooding_cnt < IPMC_MAX_FLOODING_COUNT) {
                /* we basically want to flood this to all ports within VLAN, PVLAN ... */
                vtss_ipmc_calculate_dst_ports(vid, src_port, ret_fwd, version);
                rc = IPMC_ERROR_PKT_IS_QUERY;
            } else {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);
                rc = IPMC_ERROR_PKT_TOO_MUCH_QUERY;
            }
#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
            i = PORT_NO_STACK_1;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

            if (!pass_compat) {
                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            proxy_status = ipmc_proxy_enabled[version];
            /* Is this a general query ? */
            if (ipmc_lib_isaddr6_all_zero(&snp_pkt_attrib.group_addr)) {
                if (proxy_status) {
                    if (oldQuery) {
                        ref_timeout = 0x1;
                    } else {
                        ref_timeout = IPMC_TIMER_QRI(entry);
                    }

                    if (!ipmc_lib_listener_set_reporting_timer(&entry->proxy_report_timeout,
                                                               snp_pkt_attrib.max_resp_time / 10,
                                                               ref_timeout)) {
                        entry->proxy_report_timeout = 0x1;
                    }

                    ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                    i = PORT_NO_STACK_0;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                    i = PORT_NO_STACK_1;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */
                }
            } else {
                has_grp = FALSE;
                grp_rxi_snp.vid = vid;
                memcpy(&grp_rxi_snp.group_addr, &snp_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
                grp_rxi_snp.ipmc_version = entry->ipmc_version;
                if ((grp = ipmc_lib_group_ptr_get(snp_mgrp_tree, &grp_rxi_snp)) != NULL) {
                    grp_info = grp->info;
                    has_grp = TRUE;

                    if (entry->param.querier.state == IPMC_QUERIER_IDLE) { /* Non-Querier */
                        for (i = 0; i < local_port_cnt; i++) {
                            if (IPMC_LIB_CHK_LISTENER_GET(grp, i)) {
                                if (!snp_pkt_attrib.no_of_sources) {
                                    (void) ipmc_lib_protocol_lower_filter_timer(snp_fltr_tree, grp, entry, i);
                                }
                            }
                        }

                        /* state transition */
                        grp_info->state = IPMC_OP_CHK_LISTENER;
                    } /* Non-Querier */
                }

                if (proxy_status) {
                    if (has_grp && !ipmc_proxy_proc_specific_group(entry, &snp_pkt_attrib.group_addr, FALSE)) {
                        T_D("Failed in REPLY ipmc_proxy_proc_specific_group(%s)", misc_ipv6_txt(&snp_pkt_attrib.group_addr, buf));
                    }

                    ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                    i = PORT_NO_STACK_0;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                    i = PORT_NO_STACK_1;
                    VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */
                }
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
            pass_compat = TRUE;
            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                if ((snp_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_OLD)) {
                    pass_compat = FALSE;
                }

                if ((snp_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V2JOIN) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_GEN)) {
                    pass_compat = FALSE;
                }

                if ((snp_pkt_attrib.msgType == IPMC_MLD_MSG_TYPE_V1REPORT) &&
                    (compatibility != VTSS_IPMC_COMPAT_MODE_GEN)) {
                    pass_compat = FALSE;
                }
            }

            in_range = TRUE;
            if (ipmc_lib_get_ssm_range(version, &ssm_ver_prefix)) {
                memcpy(&grp_prefix.addr, &snp_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
                grp_prefix.len = ssm_ver_prefix.len;
                if (!ipmc_lib_prefix_matching(version, TRUE, &ssm_ver_prefix, &grp_prefix)) {
                    in_range = FALSE;
                }
            }

            if (!pass_compat || in_range) {
                ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (uchar *)(frame + snp_pkt_attrib.offset);
            report_throttling = vtss_snp_get_report_throttling(version,
                                                               content,
                                                               snp_pkt_attrib.msgType,
                                                               src_port);
            proxy_status = ipmc_proxy_enabled[version];
            grp_op = IPMC_OP_SET;
            actual_throttling = report_throttling;
            SNP_PORT_PROFILE_GET(&ipmc_specific_port_fltr, version, &snp_pkt_attrib, vid, src_port);
            if ((rc = ipmc_lib_protocol_do_sfm_report(snp_mgrp_tree,
                                                      snp_rxmt_tree,
                                                      snp_fltr_tree,
                                                      snp_srct_tree,
                                                      entry,
                                                      content,
                                                      src_port,
                                                      snp_pkt_attrib.msgType,
                                                      snp_pkt_attrib.ipmc_pkt_len,
                                                      &ipmc_specific_port_fltr,
                                                      &actual_throttling,
                                                      proxy_status,
                                                      &grp_op)) == VTSS_OK) {
                if ((report_throttling != IPMC_REPORT_THROTTLED) &&
                    (report_throttling != IPMC_REPORT_NORMAL)) {
                    if (actual_throttling == IPMC_REPORT_THROTTLED) {
                        ipmc_current_port_throttling_status[version].max_no[src_port] += report_throttling;
                    } else {
                        ipmc_current_port_throttling_status[version].max_no[src_port] += (report_throttling - actual_throttling);
                    }

                    if (ipmc_static_port_throttling[version].max_no[src_port] < ipmc_current_port_throttling_status[version].max_no[src_port]) {
                        T_D("Invalid calculation of port_throttling");
                        ipmc_current_port_throttling_status[version].max_no[src_port] = ipmc_static_port_throttling[version].max_no[src_port];
                    }
                }
            }

            if (rc != VTSS_OK) {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);
                update_statistics = FALSE;
            } else {
                if (!proxy_status) {
                    ipmc_get_router_port_mask(version, src_port, ret_fwd);
                } else {
                    VTSS_PORT_BF_CLR(ret_fwd->member_ports);

                    if ((grp_op == IPMC_OP_ADD) && !ipmc_proxy_proc_specific_group(entry, &snp_pkt_attrib.group_addr, FALSE)) {
                        T_D("Failed in JOIN ipmc_proxy_proc_specific_group(%s)", misc_ipv6_txt(&snp_pkt_attrib.group_addr, buf));
                    }
                }
            }
#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
            i = PORT_NO_STACK_1;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

            if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
                entry->param.hst_compatibility.mode = compatibility;
                break;
            }

            if (snp_pkt_attrib.msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) {
                entry->param.hst_compatibility.old_present_timer = IPMC_TIMER_OVHPT(entry);
            } else {
                entry->param.hst_compatibility.gen_present_timer = IPMC_TIMER_OVHPT(entry);
            }

            break;
        case IPMC_MLD_MSG_TYPE_V2REPORT:
        case IPMC_IGMP_MSG_TYPE_V3JOIN:
            pass_compat = TRUE;
            if ((compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) &&
                (compatibility != VTSS_IPMC_COMPAT_MODE_SFM)) {
                pass_compat = FALSE;
            }

            if (!pass_compat) {
                ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (uchar *)(frame + snp_pkt_attrib.offset);
            report_throttling = vtss_snp_get_report_throttling(version,
                                                               content,
                                                               snp_pkt_attrib.msgType,
                                                               src_port);
            proxy_status = ipmc_proxy_enabled[version];
            grp_op = IPMC_OP_SET;
            actual_throttling = report_throttling;
            SNP_PORT_PROFILE_GET(&ipmc_specific_port_fltr, version, &snp_pkt_attrib, vid, src_port);
            if ((rc = ipmc_lib_protocol_do_sfm_report(snp_mgrp_tree,
                                                      snp_rxmt_tree,
                                                      snp_fltr_tree,
                                                      snp_srct_tree,
                                                      entry,
                                                      content,
                                                      src_port,
                                                      snp_pkt_attrib.msgType,
                                                      snp_pkt_attrib.ipmc_pkt_len,
                                                      &ipmc_specific_port_fltr,
                                                      &actual_throttling,
                                                      proxy_status,
                                                      &grp_op)) == VTSS_OK) {
                if ((report_throttling != IPMC_REPORT_THROTTLED) &&
                    (report_throttling != IPMC_REPORT_NORMAL)) {
                    if (actual_throttling == IPMC_REPORT_THROTTLED) {
                        ipmc_current_port_throttling_status[version].max_no[src_port] += report_throttling;
                    } else {
                        ipmc_current_port_throttling_status[version].max_no[src_port] += (report_throttling - actual_throttling);
                    }

                    if (ipmc_static_port_throttling[version].max_no[src_port] < ipmc_current_port_throttling_status[version].max_no[src_port]) {
                        T_D("Invalid calculation of port_throttling");
                        ipmc_current_port_throttling_status[version].max_no[src_port] = ipmc_static_port_throttling[version].max_no[src_port];
                    }
                }
            }

            if (rc != VTSS_OK) {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);
                update_statistics = FALSE;
            } else {
                if (!proxy_status) {
                    ipmc_get_router_port_mask(version, src_port, ret_fwd);
                } else {
                    VTSS_PORT_BF_CLR(ret_fwd->member_ports);

                    if ((grp_op == IPMC_OP_ADD) && !ipmc_proxy_proc_specific_group(entry, &snp_pkt_attrib.group_addr, FALSE)) {
                        T_D("Failed in REPORT ipmc_proxy_proc_specific_group(%s)", misc_ipv6_txt(&snp_pkt_attrib.group_addr, buf));
                    }
                }
            }
#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
            i = PORT_NO_STACK_1;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
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

            in_range = TRUE;
            if (ipmc_lib_get_ssm_range(version, &ssm_ver_prefix)) {
                memcpy(&grp_prefix.addr, &snp_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
                grp_prefix.len = ssm_ver_prefix.len;
                if (!ipmc_lib_prefix_matching(version, TRUE, &ssm_ver_prefix, &grp_prefix)) {
                    in_range = FALSE;
                }
            }

            if (!pass_compat || in_range) {
                ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            content = (uchar *)(frame + snp_pkt_attrib.offset);

            proxy_status = (ipmc_leave_proxy_enabled[version] || ipmc_proxy_enabled[version]);
            has_grp = FALSE;
            grp_rxi_snp.vid = vid;
            grp_rxi_snp.ipmc_version = version;
            memcpy(&grp_rxi_snp.group_addr, &snp_pkt_attrib.group_addr, sizeof(vtss_ipv6_t));
            grp = ipmc_lib_group_ptr_get(snp_mgrp_tree, &grp_rxi_snp);
            if (grp && grp->info && IPMC_LIB_GRP_PORT_DO_SFM(&grp->info->db, src_port)) {
                has_grp = TRUE;
                grp_info = grp->info;
                grp_db = &grp_info->db;

                grp_op = IPMC_OP_SET;
                actual_throttling = IPMC_REPORT_NORMAL;
                SNP_PORT_PROFILE_GET(&ipmc_specific_port_fltr, version, &snp_pkt_attrib, vid, src_port);
                rc = ipmc_lib_protocol_do_sfm_report(snp_mgrp_tree,
                                                     snp_rxmt_tree,
                                                     snp_fltr_tree,
                                                     snp_srct_tree,
                                                     entry,
                                                     content,
                                                     src_port,
                                                     snp_pkt_attrib.msgType,
                                                     snp_pkt_attrib.ipmc_pkt_len,
                                                     &ipmc_specific_port_fltr,
                                                     &actual_throttling,
                                                     proxy_status,
                                                     &grp_op);

                /* fast leave */
                if ((rc == VTSS_OK) &&
                    (from_fast_leave || VTSS_PORT_BF_GET(ipmcsnp_fast_leave_ports[version], src_port))) {
                    vtss_ipv4_t ip4sip, ip4dip;
                    vtss_ipv6_t ip6sip, ip6dip;
                    BOOL        fwd_map[VTSS_PORT_ARRAY_SIZE] = {FALSE};

                    for (i = 0; i < local_port_cnt; i++) {
                        if (i != src_port) {
                            if (VTSS_PORT_BF_GET(grp_db->port_mask, i) ||
                                ipmc_lib_get_port_rpstatus(version, i) ||
                                VTSS_PORT_BF_GET(static_router_port_mask[version], i)) {
                                fwd_map[i] = TRUE;
                            }
                        } else {
                            if (ipmc_lib_get_port_rpstatus(version, i) ||
                                VTSS_PORT_BF_GET(static_router_port_mask[version], i)) {
                                fwd_map[i] = TRUE;
                            }
                        }
                    }

#if VTSS_SWITCH_STACKABLE
                    i = PORT_NO_STACK_0;
                    fwd_map[i] = TRUE;
                    i = PORT_NO_STACK_1;
                    fwd_map[i] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

                    /* ASM-UPDATE(ADD) */
                    if (version == IPMC_IP_VERSION_MLD) {
                        memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                        memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                        ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
                        memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
                    } else {
                        ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
                        memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                        memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                        memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                    }

                    if (ipmc_lib_porting_set_chip(TRUE, snp_mgrp_tree, grp, version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map) != VTSS_OK) {
                        T_D("ipmc_lib_porting_set_chip ADD failed");
                    }
                    /* SHOULD Notify MC-Routing + */

                    /* Workaround for Fast-Aging without Querier */
                    if (entry->param.querier.state == IPMC_QUERIER_IDLE) {
                        (void) ipmc_lib_protocol_lower_filter_timer(snp_fltr_tree, grp, entry, src_port);
                    }
                } /* fast leave */

                /* group state machine */
                if ((rc == VTSS_OK) && VTSS_PORT_BF_GET(grp_db->port_mask, src_port)) {
                    if (ipmc_lib_get_sq_ssq_action(proxy_status, entry, src_port) != IPMC_SND_HOLD) { /* Querier */
                        /* state transition */
                        grp_info->state = IPMC_OP_CHK_LISTENER;

                        /* start rxmt timer */
                        grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                        IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], snp_rxmt_tree, grp_info);
                    }
                } /* VTSS_PORT_BF_GET(grp_db->port_mask, src_port) */
            } else {
                /* If the LEAVE group is not existed, forward it to upstream directly */
                ipmc_get_router_port_mask(version, src_port, ret_fwd);
#if VTSS_SWITCH_STACKABLE
                i = PORT_NO_STACK_0;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
                i = PORT_NO_STACK_1;
                VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

                update_statistics = FALSE;
                /* Should LOG Here! */
                break;
            }

            if (has_grp && proxy_status) {
                VTSS_PORT_BF_CLR(ret_fwd->member_ports);

                if (entry && (entry->param.rtr_compatibility.mode != VTSS_IPMC_COMPAT_MODE_OLD) &&
                    !ipmc_proxy_proc_specific_group(entry, &snp_pkt_attrib.group_addr, TRUE)) {
                    T_D("Failed in LEAVE ipmc_proxy_proc_specific_group(%s)", misc_ipv6_txt(&snp_pkt_attrib.group_addr, buf));
                }
            } else {
                ipmc_get_router_port_mask(version, src_port, ret_fwd);
            }
#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
            i = PORT_NO_STACK_1;
            VTSS_PORT_BF_SET(ret_fwd->member_ports, i, incl_stacking);
#endif /* VTSS_SWITCH_STACKABLE */

            break;
        default:
            /* unknown type, we shall flood this one */
            vtss_ipmc_calculate_dst_ports(vid, src_port, ret_fwd, version);

            break;
        }

        /* update statistics (only for packets comming from front ports) */
        if (update_statistics && !from_stacking) {
            if (version == IPMC_IP_VERSION_MLD) {
                T_D("MLD update_statistics for msgType(%u) on Port-%u", snp_pkt_attrib.msgType, src_port);
                update_mld_statistics(entry, (ipmc_mld_packet_t *)(frame + snp_pkt_attrib.offset));
            }

            if (version == IPMC_IP_VERSION_IGMP) {
                T_D("IGMP update_statistics for msgType(%u) on Port-%u", snp_pkt_attrib.msgType, src_port);
                update_igmp_statistics(entry, (ipmc_igmp_packet_t *)(frame + snp_pkt_attrib.offset));
            }
        }
    }

    return rc;
}
#endif /* VTSS_SW_OPTION_PACKET */

void vtss_ipmc_clear_stat_counter(ipmc_ip_version_t ipmc_version, vtss_vid_t vid)
{
    ipmc_intf_entry_t   *entry;

    /* clear statistics */
    if (vid == VTSS_IPMC_VID_ALL) {
        vtss_vid_t          idx = 0;
        ipmc_ip_version_t   version = ipmc_version;

        while ((entry = vtss_ipmc_get_next_intf_entry(idx, version)) != NULL) {
            if (version != ipmc_version) {
                break;
            }

            idx = entry->param.vid;
            version = entry->ipmc_version;

            _ipmc_clear_intf_statistics(entry);
        }
    } else {
        if ((entry = vtss_ipmc_get_intf_entry(vid, ipmc_version)) != NULL) {
            _ipmc_clear_intf_statistics(entry);
        }
    }
}

void vtss_ipmc_set_mode(BOOL mode, ipmc_ip_version_t ipmc_version)
{
    T_D("Version=%d|OldMode=%s|NewMode=%s",
        ipmc_version,
        ipmc_enabled[ipmc_version] ? "TRUE" : "FALSE",
        mode ? "TRUE" : "FALSE");

    if (ipmc_enabled[ipmc_version] != mode) {
        port_iter_t pit;

        if (mode == FALSE) {
            ipmc_enabled[ipmc_version] = FALSE;

            /* if disabled, clear router ports */
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                ipmc_lib_set_discovered_router_port_mask(ipmc_version, pit.iport, FALSE);
            }

            query_suppression_timer = QUERY_SUPPRESSION_TIMEOUT;
            query_flooding_cnt = 0;
        } else {
            u32                 i;
            ipmc_intf_entry_t   *entry;

            /* if enabled, first restore (static) router ports */
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (VTSS_PORT_BF_GET(static_router_port_mask[ipmc_version], i)) {
                    ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, TRUE);
                }
            }

#if VTSS_SWITCH_STACKABLE
            i = PORT_NO_STACK_0;
            ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, TRUE);
            i = PORT_NO_STACK_1;
            ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

            ipmc_enabled[ipmc_version] = TRUE;

            for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
                entry = &ipmc_vlan_entries[i];

                if (!entry || (entry->ipmc_version != ipmc_version) ||
                    !entry->param.vid || !entry->op_state) {
                    continue;
                }

                entry->param.querier.proxy_query_timeout = 0x0;
            }
        }

        if (ipmc_version == IPMC_IP_VERSION_MLD) {
            if (!ipmc_lib_mc6_ctrl_flood_set(IPMC_OWNER_MLD, ipmc_enabled[ipmc_version])) {
                T_D("Failure in ipmc_lib_mc6_ctrl_flood_set");
            }
        }
    }

    vtss_ipmc_upd_unknown_fwdmsk(ipmc_version);
}

void vtss_ipmc_set_leave_proxy(BOOL mode, ipmc_ip_version_t ipmc_version)
{
    ipmc_leave_proxy_enabled[ipmc_version] = mode;
}

void vtss_ipmc_set_proxy(BOOL mode, ipmc_ip_version_t ipmc_version)
{
    ipmc_intf_entry_t   *entry;
    u32                 i;

    if (ipmc_proxy_enabled[ipmc_version] != mode) {
        ipmc_proxy_enabled[ipmc_version] = mode;

        for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
            entry = &ipmc_vlan_entries[i];

            if (!entry || (entry->ipmc_version != ipmc_version) ||
                !entry->param.vid || !entry->op_state) {
                continue;
            }

            entry->param.querier.proxy_query_timeout = 0x0;
        }
    }
}

void vtss_ipmc_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *prefix)
{
    ipmc_prefix_t   ssm_range;

    if (!prefix) {
        return;
    } else {
        memcpy(&ssm_range, prefix, sizeof(ipmc_prefix_t));
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ssm_range.addr.value.prefix = htonl(prefix->addr.value.prefix);
    }

    ipmc_lib_set_ssm_range(ipmc_version, &ssm_range);
}

void vtss_ipmc_set_static_router_ports(ipmc_port_bfs_t *port_mask, ipmc_ip_version_t ipmc_version)
{
    port_iter_t pit;
    u32         i;
    BOOL        rpstatus;

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        rpstatus = VTSS_PORT_BF_GET(port_mask->member_ports, i);
        VTSS_PORT_BF_SET(static_router_port_mask[ipmc_version], i, rpstatus);
        ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, rpstatus);
    }

    ipmc_update_router_port(ipmc_version);

#if VTSS_SWITCH_STACKABLE
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(static_router_port_mask[ipmc_version], i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, TRUE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(static_router_port_mask[ipmc_version], i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(ipmc_version, i, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

    vtss_ipmc_upd_unknown_fwdmsk(ipmc_version);
}

void vtss_ipmc_set_static_fast_leave_ports(ipmc_port_bfs_t *port_mask, ipmc_ip_version_t ipmc_version)
{
    port_iter_t pit;
    u32         i;

    if (!port_mask) {
        return;
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        VTSS_PORT_BF_SET(ipmcsnp_fast_leave_ports[ipmc_version],
                         i,
                         VTSS_PORT_BF_GET(port_mask->member_ports, i));
    }
}

/**
 * vtss_ipmc_get_static_fast_leave_ports - Get static fast leave of a port in vtss_ipmc protocol module.
 */
BOOL vtss_ipmc_get_static_fast_leave_ports(u32 port, ipmc_ip_version_t ipmc_version)
{
    return VTSS_PORT_BF_GET(ipmcsnp_fast_leave_ports[ipmc_version], port);
}

void vtss_ipmc_set_static_port_throttling_max_no(ipmc_port_throttling_t *ipmc_port_throttling, ipmc_ip_version_t ipmc_version)
{
    port_iter_t pit;
    u32         i;

    if (!ipmc_port_throttling) {
        return;
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        if (ipmc_static_port_throttling[ipmc_version].max_no[i] != ipmc_port_throttling->max_no[i]) {
            if (ipmc_static_port_throttling[ipmc_version].max_no[i] > ipmc_port_throttling->max_no[i]) {
                ipmc_del_group_member_with_num(ipmc_version, i, (ipmc_static_port_throttling[ipmc_version].max_no[i] - ipmc_port_throttling->max_no[i]));
            }

            ipmc_static_port_throttling[ipmc_version].max_no[i] = ipmc_port_throttling->max_no[i];
        }
    }
}

void vtss_ipmc_set_static_port_group_filtering(ipmc_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version)
{
    if (!ipmc_port_group_filtering) {
        return;
    } else {
        memcpy(&ipmc_static_port_group_filtering[ipmc_version], ipmc_port_group_filtering, sizeof(ipmc_port_group_filtering_t));
    }
}

void vtss_ipmc_set_unreg_flood(BOOL enabled, ipmc_ip_version_t ipmc_version)
{
    ipmc_unreg_flood_enabled[ipmc_version] = enabled;
    vtss_ipmc_upd_unknown_fwdmsk(ipmc_version);
}

static void general_query_timeout4proxy(ipmc_intf_entry_t *entry)
{
    ipmc_group_entry_t  *grp, *grp_ptr;
    ipmc_port_bfs_t     fwd_mask;

    if (!entry) {
        return;
    }

    grp_ptr = NULL;
    while ((grp = ipmc_lib_group_ptr_get_next(snp_mgrp_tree, grp_ptr)) != NULL) {
        grp_ptr = grp;

        if ((grp->vid == entry->param.vid) && (grp->ipmc_version == entry->ipmc_version)) {
            VTSS_PORT_BF_CLR(fwd_mask.member_ports);
            ipmc_lib_get_discovered_router_port_mask(entry->ipmc_version, &fwd_mask);
            (void) ipmc_lib_packet_tx_join_report(FALSE,
                                                  entry->param.rtr_compatibility.mode,
                                                  entry,
                                                  &grp->group_addr,
                                                  &fwd_mask,
                                                  entry->ipmc_version,
                                                  FALSE,
                                                  TRUE,
                                                  FALSE);
        }
    }
}

void vtss_ipmc_tick_gen(void)
{
    if (ipmc_enabled[IPMC_IP_VERSION_IGMP] || ipmc_enabled[IPMC_IP_VERSION_MLD]) {
        port_iter_t pit;
        u32         i;
        BOOL        rp_status;

        /* query flooding suppression */
        (void) ipmc_lib_protocol_suppression(&query_suppression_timer,
                                             QUERY_SUPPRESSION_TIMEOUT,
                                             &query_flooding_cnt);

        /* router port timeout */
        ipmc_lib_set_changed_router_port(TRUE, IPMC_IP_VERSION_IGMP, 0);
        if (router_port_timer[IPMC_IP_VERSION_IGMP] && (--router_port_timer[IPMC_IP_VERSION_IGMP] == 0)) {
            router_port_timer[IPMC_IP_VERSION_IGMP] = ROUTER_PORT_TIMEOUT;

            /* update router port mask with what we saw in the last period */
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (ipmc_lib_get_port_rpstatus(IPMC_IP_VERSION_IGMP, i) !=
                    VTSS_PORT_BF_GET(next_router_port_mask[IPMC_IP_VERSION_IGMP], i)) {
                    ipmc_lib_set_changed_router_port(FALSE, IPMC_IP_VERSION_IGMP, i);
                }

                rp_status = VTSS_PORT_BF_GET(next_router_port_mask[IPMC_IP_VERSION_IGMP], i) |
                            VTSS_PORT_BF_GET(static_router_port_mask[IPMC_IP_VERSION_IGMP], i);
                ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, i, rp_status);
            }

            VTSS_PORT_BF_CLR(next_router_port_mask[IPMC_IP_VERSION_IGMP]);

            /* update iflodmsk */
            vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_IGMP);
        }
        ipmc_lib_set_changed_router_port(TRUE, IPMC_IP_VERSION_MLD, 0);
        if (router_port_timer[IPMC_IP_VERSION_MLD] && (--router_port_timer[IPMC_IP_VERSION_MLD] == 0)) {
            router_port_timer[IPMC_IP_VERSION_MLD] = ROUTER_PORT_TIMEOUT;

            /* update router port mask with what we saw in the last period */
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                if (ipmc_lib_get_port_rpstatus(IPMC_IP_VERSION_MLD, i) !=
                    VTSS_PORT_BF_GET(next_router_port_mask[IPMC_IP_VERSION_MLD], i)) {
                    ipmc_lib_set_changed_router_port(FALSE, IPMC_IP_VERSION_MLD, i);
                }

                rp_status = VTSS_PORT_BF_GET(next_router_port_mask[IPMC_IP_VERSION_MLD], i) |
                            VTSS_PORT_BF_GET(static_router_port_mask[IPMC_IP_VERSION_MLD], i);
                ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_MLD, i, rp_status);
            }

            VTSS_PORT_BF_CLR(next_router_port_mask[IPMC_IP_VERSION_MLD]);

            /* update iflodmsk */
            vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_MLD);
        }
        /* router port timeout */
    } /* ipmc_enabled */
}

void vtss_ipmc_tick_intf_tmr(u32 i)
{
    ipmc_intf_entry_t   *intf = &ipmc_vlan_entries[i];

    if (!intf || !ipmc_enabled[intf->ipmc_version]) {
        return;
    }

    if (ipmc_lib_protocol_intf_tmr(ipmc_proxy_enabled[intf->ipmc_version], intf) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_intf_tmr for VLAN %d", intf->param.vid);
    }
}

void vtss_ipmc_tick_intf_rxmt(void)
{
    if (ipmc_lib_protocol_intf_rxmt(snp_mgrp_tree,
                                    snp_rxmt_tree,
                                    snp_fltr_tree) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_intf_rxmt");
    }
}

void vtss_ipmc_tick_group_tmr(void)
{
    if (ipmc_lib_protocol_group_tmr(FALSE,
                                    snp_mgrp_tree,
                                    snp_rxmt_tree,
                                    snp_fltr_tree,
                                    snp_srct_tree,
                                    ipmc_current_port_throttling_status,
                                    ipmc_proxy_enabled,
                                    ipmc_leave_proxy_enabled) != VTSS_OK) {
        T_D("Failure in ipmc_lib_protocol_group_tmr");
    }
}

void vtss_ipmc_tick_proxy(BOOL local_service)
{
    ipmc_proxy_report_entry_t   *pr_ptr;
    ipmc_intf_entry_t           *ipmc_intf;
    ipmc_querier_sm_t           *querier;
    ipmc_ip_version_t           version;
    ipmc_port_bfs_t             fwd_mask;
    u32                         i;
    vtss_ipv6_t                 dst_ip6;

    if (ipmc_enabled[IPMC_IP_VERSION_IGMP] || ipmc_enabled[IPMC_IP_VERSION_MLD]) {
        for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
            ipmc_intf = &ipmc_vlan_entries[i];
            version = ipmc_intf->ipmc_version;
            if (!ipmc_enabled[version] ||
                !ipmc_intf->param.vid || !ipmc_intf->op_state ||
                !ipmc_proxy_enabled[version]) {
                continue;
            }

            querier = &ipmc_intf->param.querier;
            if (querier->proxy_query_timeout) {
                querier->proxy_query_timeout--;
            }
            if (querier->proxy_query_timeout == 0) {
                ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                (void) ipmc_lib_packet_tx_proxy_query(ipmc_intf, &dst_ip6, FALSE);

                /* re-start timer */
                querier->proxy_query_timeout = IPMC_TIMER_QI(ipmc_intf);
            }
        } /* for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) */

        pr_ptr = NULL;
        while (IPMC_LIB_DB_GET_NEXT(snp_prxy_tree, pr_ptr)) {
            version = pr_ptr->ipmc_version;
            /* Sending proxy LEAVE will be handled by group deletion */
            if (ipmc_proxy_enabled[version] && !pr_ptr->leave) {
                VTSS_PORT_BF_CLR(fwd_mask.member_ports);
                ipmc_lib_get_discovered_router_port_mask(version, &fwd_mask);
                (void) ipmc_lib_packet_tx_join_report(FALSE,
                                                      pr_ptr->compat,
                                                      vtss_ipmc_get_intf_entry(pr_ptr->vid, version),
                                                      &pr_ptr->group_address,
                                                      &fwd_mask,
                                                      version,
                                                      FALSE,
                                                      TRUE,
                                                      FALSE);
            }

            if (!IPMC_LIB_DB_DEL(snp_prxy_tree, pr_ptr)) {
                T_D("IPMC_LIB_DB_DEL() fail");
            } else {
                IPMC_MEM_SYSTEM_MGIVE(pr_ptr);
            }
        }

        for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
            ipmc_intf = &ipmc_vlan_entries[i];
            version = ipmc_intf->ipmc_version;
            if (!ipmc_enabled[version] ||
                !ipmc_intf->param.vid || !ipmc_intf->op_state ||
                !ipmc_proxy_enabled[version]) {
                continue;
            }

            /* Reply General-Query */
            if (ipmc_intf->proxy_report_timeout &&
                (--ipmc_intf->proxy_report_timeout == 0)) {
                T_D("General Query timeout for VLAN %d", ipmc_intf->param.vid);
                general_query_timeout4proxy(ipmc_intf);
            }
        }
    } /* ipmc_enabled */
}

void vtss_ipmc_init(void)
{
    port_iter_t pit;
    u32         i;

    (void) ipmc_lib_protocol_init();

    /* explicitly clear all IPMC entries */
    for (i = 0; i < SNP_NUM_OF_SUPPORTED_INTF; i++) {
        ipmc_vlan_entries[i].param.vid = 0;
        ipmc_vlan_entries[i].ipmc_version = IPMC_IP_VERSION_INIT;

        ipmc_vlan_entries[i].param.priority = IPMC_PARAM_PRIORITY_NULL;
        ipmc_vlan_entries[i].param.querier.QuerierAdrs4 = IPMC_PARAM_VALUE_NULL;
        ipmc_vlan_entries[i].param.querier.RobustVari = IPMC_PARAM_VALUE_NULL;
        ipmc_vlan_entries[i].param.querier.QueryIntvl = IPMC_PARAM_VALUE_NULL;
        ipmc_vlan_entries[i].param.querier.MaxResTime = IPMC_PARAM_VALUE_NULL;
        ipmc_vlan_entries[i].param.querier.LastQryItv = IPMC_PARAM_VALUE_NULL;
        ipmc_vlan_entries[i].param.querier.UnsolicitR = IPMC_PARAM_VALUE_NULL;
    }

    /* create the multicast group table */
    if (!vtss_snp_mgroup_entry_list_created_done) {
        /* create data base for storing IPMCSNP group entry */
        if (!IPMC_LIB_DB_TAKE("SNP_GRP_TABLE", snp_mgrp_tree,
                              IPMC_LIB_SUPPORTED_SNP_GROUPS,
                              sizeof(ipmc_group_entry_t),
                              vtss_ipmc_mgroup_entry_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_snp_mgroup_entry_list) failed");
        } else {
            snp_mgrp_tree->mflag = TRUE;
            vtss_snp_mgroup_entry_list_created_done = TRUE;
        }
    }

    /* create data base for storing IPMC Proxy Report entry */
    if (!vtss_ipmc_proxy_report_entry_list_created_done) {
        if (!IPMC_LIB_DB_TAKE("SNP_PRXY_TABLE", snp_prxy_tree,
                              IPMC_LIB_SUPPORTED_SNP_GROUPS,
                              sizeof(ipmc_proxy_report_entry_t),
                              vtss_ipmc_proxy_report_entry_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_ipmc_proxy_report_entry_list) failed");
        } else {
            snp_prxy_tree->mflag = TRUE;
            vtss_ipmc_proxy_report_entry_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast group (filter) timer */
    if (!vtss_snp_grp_fltr_tmr_list_created_done) {
        /* create data base for storing multicast group (filter) timer */
        if (!IPMC_LIB_DB_TAKE("SNP_GRP_FLTR", snp_fltr_tree,
                              IPMC_LIB_MAX_SNP_FLTR_TMR_LIST,
                              sizeof(ipmc_group_db_t),
                              vtss_ipmcsnp_grp_fltr_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_snp_grp_fltr_tmr_list) failed");
        } else {
            snp_fltr_tree->mflag = TRUE;
            vtss_snp_grp_fltr_tmr_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast source timer */
    if (!vtss_snp_grp_srct_tmr_list_created_done) {
        /* create data base for storing multicast source timer */
        if (!IPMC_LIB_DB_TAKE("SNP_GRP_SRCT", snp_srct_tree,
                              IPMC_LIB_MAX_SNP_SRCT_TMR_LIST,
                              sizeof(ipmc_sfm_srclist_t),
                              vtss_ipmcsnp_grp_srct_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_snp_grp_srct_tmr_list) failed");
        } else {
            snp_srct_tree->mflag = TRUE;
            vtss_snp_grp_srct_tmr_list_created_done = TRUE;
        }
    }

    /* create the timer list table for multicast rxmt timer */
    if (!vtss_snp_grp_rxmt_tmr_list_created_done) {
        /* create data base for storing multicast rxmt timer */
        if (!IPMC_LIB_DB_TAKE("SNP_GRP_RXMT", snp_rxmt_tree,
                              IPMC_LIB_MAX_SNP_RXMT_TMR_LIST,
                              sizeof(ipmc_group_info_t),
                              vtss_ipmcsnp_grp_rxmt_tmr_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(vtss_snp_grp_rxmt_tmr_list) failed");
        } else {
            snp_rxmt_tree->mflag = TRUE;
            vtss_snp_grp_rxmt_tmr_list_created_done = TRUE;
        }
    }

    /* initialize RouterPorts, FastLeavePorts, ProxyAccessPorts */
    for (i = 0; i < IPMC_IP_VERSION_MAX; i++) {
        VTSS_PORT_BF_CLR(static_router_port_mask[i]);
        VTSS_PORT_BF_CLR(next_router_port_mask[i]);
        router_port_timer[i] = 0;

        VTSS_PORT_BF_CLR(ipmcsnp_fast_leave_ports[i]);
    }

#if VTSS_SWITCH_STACKABLE
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(static_router_port_mask[IPMC_IP_VERSION_IGMP], i, TRUE);
    VTSS_PORT_BF_SET(static_router_port_mask[IPMC_IP_VERSION_MLD], i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_IGMP,  i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_MLD,  i, TRUE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(static_router_port_mask[IPMC_IP_VERSION_IGMP], i, TRUE);
    VTSS_PORT_BF_SET(static_router_port_mask[IPMC_IP_VERSION_MLD], i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_IGMP,  i, TRUE);
    ipmc_lib_set_discovered_router_port_mask(IPMC_IP_VERSION_MLD,  i, TRUE);
#endif /* VTSS_SWITCH_STACKABLE */

    router_port_timer[IPMC_IP_VERSION_IGMP] = ROUTER_PORT_TIMEOUT;
    router_port_timer[IPMC_IP_VERSION_MLD] = ROUTER_PORT_TIMEOUT;

    /* update unknown flood mask */
    vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_IGMP);
    vtss_ipmc_upd_unknown_fwdmsk(IPMC_IP_VERSION_MLD);

    for (i = 0; i < IPMC_IP_VERSION_MAX; i++) {
        /* initialize IPMC global (proxy) mode */
        ipmc_enabled[i] = FALSE;
        ipmc_proxy_enabled[i] = FALSE;
        ipmc_leave_proxy_enabled[i] = FALSE;
        /* initialize IPMC unregistered flooding control */
        ipmc_unreg_flood_enabled[i] = TRUE;

        /* initialize IPMC port group throttling */
        memset(&ipmc_static_port_throttling[i], 0x0, sizeof(ipmc_port_throttling_t));
        memset(&ipmc_current_port_throttling_status[i], 0x0, sizeof(ipmc_port_throttling_t));

        /* initialize IPMC port group filtering */
        memset(&ipmc_static_port_group_filtering[i], 0x0, sizeof(ipmc_port_group_filtering_t));
    }

    query_suppression_timer = QUERY_SUPPRESSION_TIMEOUT;
    query_flooding_cnt = 0;
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        ipmc_stp_port_status[pit.iport] = VTSS_COMMON_STPSTATE_FORWARDING;
    }
}
