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

#include "ipmc_snp_icli.h"

#define VTSS_TRACE_MODULE_ID            VTSS_MODULE_ID_IPMC


/* FUNCTION_BEGIN */
static char *_icli_ipmc_snp_compat_txt(ipmc_ip_version_t version,
                                       ipmc_compat_mode_t compat,
                                       ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( compat ) {
    case VTSS_IPMC_COMPAT_MODE_OLD:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "forced igmpv1";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "FORCED IGMPV1";
            } else {
                txt = "Forced IGMPv1";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "unknown";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "UNKNOWN";
            } else {
                txt = "Unknown";
            }
        }
        break;
    case VTSS_IPMC_COMPAT_MODE_GEN:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "forced igmpv2";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "FORCED IGMPV2";
            } else {
                txt = "Forced IGMPv2";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "forced mldv1";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "FORCED MLDV1";
            } else {
                txt = "Forced MLDv1";
            }
        }
        break;
    case VTSS_IPMC_COMPAT_MODE_SFM:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "forced igmpv3";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "FORCED IGMPV3";
            } else {
                txt = "Forced IGMPv3";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "forced mldv2";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "FORCED MLDV2";
            } else {
                txt = "Forced MLDv2";
            }
        }
        break;
    case VTSS_IPMC_COMPAT_MODE_AUTO:
    default:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "igmp-auto";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "IGMP-AUTO";
            } else {
                txt = "IGMP-Auto";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "mld-auto";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "MLD-AUTO";
            } else {
                txt = "MLD-Auto";
            }
        }
        break;
    }

    return txt;
}

static char *_icli_ipmc_snp_compver_txt(ipmc_ip_version_t version,
                                        u32 compver,
                                        ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( compver ) {
    case VTSS_IPMC_VERSION1:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "version 1";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "VERSION 1";
            } else {
                txt = "Version 1";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "unknown";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "UNKNOWN";
            } else {
                txt = "Unknown";
            }
        }
        break;
    case VTSS_IPMC_VERSION2:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "version 2";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "VERSION 2";
            } else {
                txt = "Version 2";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "version 1";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "VERSION 1";
            } else {
                txt = "Version 1";
            }
        }
        break;
    case VTSS_IPMC_VERSION3:
        if (version == IPMC_IP_VERSION_IGMP) {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "version 3";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "VERSION 3";
            } else {
                txt = "Version 3";
            }
        } else {
            if (cap == IPMC_TXT_CASE_LOWER) {
                txt = "version 2";
            } else if (cap == IPMC_TXT_CASE_UPPER) {
                txt = "VERSION 2";
            } else {
                txt = "Version 2";
            }
        }
        break;
    case VTSS_IPMC_VERSION_DEFAULT:
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "default";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "DEFAULT";
        } else {
            txt = "Default";
        }
        break;
    }

    return txt;
}

static BOOL _icli_ipmc_snp_chk_by_vlan(vtss_vid_t vdx, icli_unsigned_range_t *vlist)
{
    u32         idx;
    vtss_vid_t  vidx, bnd;

    if (!vlist) {
        return FALSE;
    }

    for (idx = 0; idx < vlist->cnt; idx++) {
        bnd = vlist->range[idx].max;
        for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
            if (vdx == vidx) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static BOOL _icli_ipmc_snp_chk_by_port_msk(vtss_isid_t isx, u8 *ptx, icli_stack_port_range_t *plist)
{
    u32             rdx;
    u16             idx, bgn, cnt;
    vtss_isid_t     isid;
    vtss_port_no_t  iport;

    if (!ptx || !plist) {
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (isid != isx) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (VTSS_PORT_BF_GET(ptx, iport)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static BOOL _icli_ipmc_snp_chk_by_port_val(vtss_isid_t isx, vtss_port_no_t ptx, icli_stack_port_range_t *plist)
{
    u32             rdx;
    u16             idx, bgn, cnt;
    vtss_isid_t     isid;
    vtss_port_no_t  iport;

    if (!plist) {
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (isid != isx) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (ptx == iport) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static void _icli_ipmc_snp_db_display(i32 session_id,
                                      ipmc_prot_intf_group_entry_t *grp,
                                      vtss_isid_t isid,
                                      BOOL by_port, icli_stack_port_range_t *plist,
                                      BOOL inc_sfm, BOOL detail)
{
    icli_switch_port_range_t    icli_port;
    vtss_port_no_t              iport;

    ipmc_ip_version_t           version;
    vtss_vid_t                  vid;
    vtss_ipv6_t                 ip6grp;
    vtss_ipv4_t                 ip4grp, ip4src;
    ipmc_group_db_t             *grp_db;
    ipmc_prot_group_srclist_t   group_srclist_entry;
    ipmc_sfm_srclist_t          *sfm_src;

    BOOL                        deny_found, prted;
    i32                         fto;
    i8                          adrString[40], portString[MGMT_PORT_BUF_SIZE];

    if (!grp || (by_port && !plist)) {
        return;
    }
    version = grp->ipmc_version;
    vid = grp->vid;
    IPMC_LIB_ADRS_CPY(&ip6grp, &grp->group_addr);
    IPMC_LIB_ADRS_6TO4_SET(ip6grp, ip4grp);
    grp_db = &grp->db;

    memset(adrString, 0x0, sizeof(adrString));
    if (version == IPMC_IP_VERSION_IGMP) {
        (void) icli_ipv4_to_str(htonl(ip4grp), adrString);
    } else {
        (void) icli_ipv6_to_str(ip6grp, adrString);
    }
    ICLI_PRINTF("\n\r%s is registered on VLAN %u\n", adrString, vid);

    prted = FALSE;
    memset(&icli_port, 0x0, sizeof(icli_switch_port_range_t));
    icli_port.switch_id = icli_isid2switchid(isid);
    while (icli_switch_port_get_next(&icli_port)) {
        iport = icli_port.begin_iport;
        if (VTSS_PORT_BF_GET(grp_db->port_mask, iport)) {
            memset(portString, 0x0, sizeof(portString));
            ICLI_PRINTF("%s%s",
                        prted ? "," : "Port Members: ",
                        icli_port_info_txt_short(icli_port.usid, icli_port.begin_uport, portString));
            prted = TRUE;
        }
    }
    if (prted) {
        ICLI_PRINTF("\n");
    }

    if (detail) {
        ICLI_PRINTF("Hardware Switch: %s\n", grp_db->asm_in_hw ? "Yes" : "No");
    }

    if (!inc_sfm) {
        return;
    }

    memset(&icli_port, 0x0, sizeof(icli_switch_port_range_t));
    icli_port.switch_id = icli_isid2switchid(isid);
    while (icli_switch_port_get_next(&icli_port)) {
        iport = icli_port.begin_iport;
        if (by_port && !_icli_ipmc_snp_chk_by_port_val(isid, iport, plist)) {
            continue;
        }
        if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, iport)) {
            continue;
        }

        if (IPMC_LIB_GRP_PORT_SFM_EX(grp_db, iport)) {
            fto = ipmc_lib_mgmt_calc_delta_time(isid, &grp_db->tmr.delta_time.v[iport]);
        } else {
            fto = -1;
        }

        memset(portString, 0x0, sizeof(portString));
        ICLI_PRINTF("%s Mode is %s",
                    icli_port_info_txt_short(icli_port.usid, icli_port.begin_uport, portString),
                    (fto < 0) ? "Include" : "Exclude");
        if (!detail || (fto < 0)) {
            ICLI_PRINTF("\n");
        } else {
            ICLI_PRINTF(" (Filter Timer: %d)\n", fto);
        }

        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
        group_srclist_entry.type = TRUE;    /* Allow|Include List */
        while (ipmc_mgmt_get_next_group_srclist_info(isid, version, vid,
                                                     &ip6grp,
                                                     &group_srclist_entry) == VTSS_OK) {
            if (!group_srclist_entry.cntr) {
                break;
            }

            sfm_src = &group_srclist_entry.srclist;
            if (!VTSS_PORT_BF_GET(sfm_src->port_mask, iport)) {
                continue;
            }

            memset(adrString, 0x0, sizeof(adrString));
            if (version == IPMC_IP_VERSION_IGMP) {
                IPMC_LIB_ADRS_6TO4_SET(sfm_src->src_ip, ip4src);
                (void) icli_ipv4_to_str(htonl(ip4src), adrString);
            } else {
                (void) icli_ipv6_to_str(sfm_src->src_ip, adrString);
            }

            if (fto < 0) {
                ICLI_PRINTF("Allow Source Address  : %s", adrString);
            } else {
                ICLI_PRINTF("Request Source Address: %s", adrString);
            }
            if (detail) {
                ICLI_PRINTF(" (Timer->%d)\n\rHardware Filter: %s\n",
                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src->tmr.delta_time.v[iport]),
                            sfm_src->sfm_in_hw ? "Yes" : "No");
            } else {
                ICLI_PRINTF("\n");
            }
        }

        deny_found = FALSE;
        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
        group_srclist_entry.type = FALSE;   /* Deny|Exclude List */
        while (ipmc_mgmt_get_next_group_srclist_info(isid, version, vid,
                                                     &ip6grp,
                                                     &group_srclist_entry) == VTSS_OK) {
            if (!group_srclist_entry.cntr) {
                break;
            }

            sfm_src = &group_srclist_entry.srclist;
            if (!VTSS_PORT_BF_GET(sfm_src->port_mask, iport)) {
                continue;
            }

            deny_found = TRUE;
            memset(adrString, 0x0, sizeof(adrString));
            if (version == IPMC_IP_VERSION_IGMP) {
                IPMC_LIB_ADRS_6TO4_SET(sfm_src->src_ip, ip4src);
                (void) icli_ipv4_to_str(htonl(ip4src), adrString);
            } else {
                (void) icli_ipv6_to_str(sfm_src->src_ip, adrString);
            }

            ICLI_PRINTF("Deny Source Address   : %s\n", adrString);
            if (detail) {
                ICLI_PRINTF(" (Timer->%d)\n\rHardware Filter: %s\n",
                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src->tmr.delta_time.v[iport]),
                            sfm_src->sfm_in_hw ? "Yes" : "No");
            } else {
                ICLI_PRINTF("\n");
            }
        }
        if (!deny_found && (fto >= 0)) {
            ICLI_PRINTF("Deny Source Address: None\n");
        }
    }
}

static u32 _icli_ipmc_snp_ovpt_get(ipmc_compatibility_t *ovpt)
{
    u32 ptimeout = 0;

    if (!ovpt) {
        return ptimeout;
    }

    switch ( ovpt->mode ) {
    case VTSS_IPMC_COMPAT_MODE_OLD:
        ptimeout = ovpt->old_present_timer;

        break;
    case VTSS_IPMC_COMPAT_MODE_GEN:
        ptimeout = ovpt->gen_present_timer;

        break;
    case VTSS_IPMC_COMPAT_MODE_SFM:
        ptimeout = ovpt->sfm_present_timer;

        break;
    case VTSS_IPMC_COMPAT_MODE_AUTO:
    default:

        break;
    }

    return ptimeout;
}

static void _icli_ipmc_snp_compver_show(ipmc_ip_version_t version,
                                        i32 session_id, vtss_isid_t isid,
                                        ipmc_prot_intf_entry_param_t *param,
                                        BOOL detail)
{
    u32                             ovpt;
    ipmc_intf_query_host_version_t  compver;

    if (!param) {
        return;
    }

    memset(&compver, 0x0, sizeof(ipmc_intf_query_host_version_t));
    compver.vid = param->vid;
    if (ipmc_mgmt_get_intf_version(isid, &compver, version) == VTSS_OK) {
        ICLI_PRINTF("Compatibility:%s / Querier Version:%s / Host Version:%s\n",
                    _icli_ipmc_snp_compat_txt(version, param->cfg_compatibility, IPMC_TXT_CASE_CAPITAL),
                    _icli_ipmc_snp_compver_txt(version, compver.query_version, IPMC_TXT_CASE_CAPITAL),
                    _icli_ipmc_snp_compver_txt(version, compver.host_version, IPMC_TXT_CASE_CAPITAL));

        if (detail && (param->cfg_compatibility == VTSS_IPMC_COMPAT_MODE_AUTO)) {
            ovpt = _icli_ipmc_snp_ovpt_get(&param->rtr_compatibility);
            ICLI_PRINTF("Older Version Querier Present Timeout: %u second%s\n",
                        ovpt, (ovpt > 1) ? "s" : "");

            ovpt = _icli_ipmc_snp_ovpt_get(&param->hst_compatibility);
            ICLI_PRINTF("Older Version Host Present Timeout: %u second%s\n",
                        ovpt, (ovpt > 1) ? "s" : "");
        }
    }
}

static BOOL _icli_ipmc_snp_intf_status_show(ipmc_ip_version_t version,
                                            i32 session_id, vtss_isid_t isid,
                                            BOOL ctrl, BOOL state,
                                            ipmc_prot_intf_entry_param_t *param,
                                            BOOL detail)
{
    BOOL                admin;
    vtss_vid_t          vidx;
    ipmc_querier_sm_t   *q;
    ipmc_statistics_t   *s;

    if (!param) {
        return FALSE;
    }
    q = &param->querier;
    s = &param->stats;
    vidx = param->vid;
    if (ctrl && state) {
        admin = TRUE;
    } else {
        admin = FALSE;
    }

    ICLI_PRINTF("\n\r%s snooping VLAN %u interface is %s.\n",
                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                vidx,
                state ? "enabled" : "disabled");
    /* Querier Status */
    ICLI_PRINTF("Querier status is %s",
                admin ? ((q->state == IPMC_QUERIER_IDLE) ? "IDLE" : "ACTIVE") : "DISABLED");
    if (detail) {
        if (q->querier_enabled) {
            ICLI_PRINTF(" (Administrative Control: %s)\n", "Join Querier-Election");
        } else {
            ICLI_PRINTF(" (Administrative Control: %s)\n", "Forced Non-Querier");
        }

        if (admin) {
            if (q->state == IPMC_QUERIER_ACTIVE) {
                ICLI_PRINTF("Querier Up time: %u second%s; Query Interval: %u second%s\n",
                            q->QuerierUpTime,
                            (q->QuerierUpTime > 1) ? "s" : "",
                            q->timeout,
                            (q->timeout > 1) ? "s" : "");
            } else if (q->state == IPMC_QUERIER_INIT) {
                ICLI_PRINTF("Startup Query Interval: %u second%s; Startup Query Count: %u\n",
                            q->timeout,
                            (q->timeout > 1) ? "s" : "",
                            q->StartUpCnt);

            } else {
                ICLI_PRINTF("Querier Expiry Time: %u second%s\n",
                            q->OtherQuerierTimeOut,
                            (q->OtherQuerierTimeOut > 1) ? "s" : "");
            }
        }
    } else {
        ICLI_PRINTF("\n");
    }

    /* Timers & Parameters & Counters */
    if (detail) {
        i8  adrString[40];

        if (q->QuerierAdrs4) {
            memset(adrString, 0x0, sizeof(adrString));
            ICLI_PRINTF("Querier address is set to %s\n",
                        misc_ipv4_txt(q->QuerierAdrs4, adrString));
        } else {
            ICLI_PRINTF("Querier address %swill use %s address of this interface.\n",
                        (version == IPMC_IP_VERSION_IGMP) ?
                        "is not set and " :
                        "",
                        (version == IPMC_IP_VERSION_IGMP) ?
                        "system's IP" :
                        "Link-Local");
        }

        memset(adrString, 0x0, sizeof(adrString));
        if (version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t qadr;

            IPMC_LIB_ADRS_6TO4_SET(param->active_querier, qadr);
            (void) icli_ipv4_to_str(htonl(qadr), adrString);
        } else {
            (void) icli_ipv6_to_str(param->active_querier, adrString);
        }
        ICLI_PRINTF("Active %s Querier Address is %s\n",
                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                    adrString);

        ICLI_PRINTF("PRI:%u / RV:%u / QI:%u / QRI:%u / LMQI:%u / URI:%u\n",
                    param->priority,
                    q->RobustVari,
                    q->QueryIntvl,
                    q->MaxResTime,
                    q->LastQryItv,
                    q->UnsolicitR);
    }

    if (admin) {
        if (version == IPMC_IP_VERSION_IGMP) {
            ICLI_PRINTF("RX IGMP Query:%u V1Join:%u V2Join:%u V3Join:%u V2Leave:%u\n",
                        s->igmp_queries,
                        s->igmp_v1_membership_join,
                        s->igmp_v2_membership_join,
                        s->igmp_v3_membership_join,
                        s->igmp_v2_membership_leave);
        } else {
            ICLI_PRINTF("RX MLD Query:%u V1Report:%u V2Report:%u V1Done:%u\n",
                        s->mld_queries,
                        s->mld_v1_membership_report,
                        s->mld_v2_membership_report,
                        s->mld_v1_membership_done);
        }
        ICLI_PRINTF("TX %s Query:%u / (Source) Specific Query:%u\n",
                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                    q->ipmc_queries_sent,
                    q->group_queries_sent);

        if (detail) {
            ipmc_prot_intf_group_entry_t    intf_group_entry;
            u16                             grpCnt;

            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            intf_group_entry.ipmc_version = version;
            intf_group_entry.vid = vidx;
            grpCnt = 0;
            while (ipmc_mgmt_get_next_intf_group_info(isid, vidx, &intf_group_entry, version) == VTSS_OK) {
                if ((intf_group_entry.ipmc_version != version) ||
                    (intf_group_entry.vid != vidx)) {
                    break;
                }

                grpCnt++;
            }

            if (version == IPMC_IP_VERSION_IGMP) {
                ICLI_PRINTF("IGMP RX Errors:%u; Group Registration Count:%u\n",
                            s->igmp_error_pkt, grpCnt);
            } else {
                ICLI_PRINTF("MLD RX Errors:%u; Group Registration Count:%u\n",
                            s->mld_error_pkt, grpCnt);
            }
        }
    }

    /* Compatibility|Version */
    _icli_ipmc_snp_compver_show(version, session_id, isid, param, detail);

    return TRUE;
}

static void _icli_ipmc_snp_intf_statistics_display(ipmc_ip_version_t version, i32 session_id,
                                                   icli_unsigned_range_t *vlist, BOOL detail)
{
    switch_iter_t                   sit;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;
    BOOL                            prt_title;
    BOOL                            ctrl, state, dummy;
    vtss_vid_t                      vidx;
    ipmc_prot_intf_entry_param_t    param;

    if (ipmc_mgmt_get_mode(&ctrl, version) != VTSS_OK) {
        ctrl = FALSE;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;
        usid = sit.usid;

        prt_title = TRUE;
        if (vlist) {
            u32         idx;
            vtss_vid_t  st_vid, bnd;

            for (idx = 0; idx < vlist->cnt; idx++) {
                bnd = vlist->range[idx].max;
                for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
                    st_vid = vidx;
                    memset(&param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                    if ((ipmc_mgmt_get_intf_state_querier(FALSE, &st_vid, &state, &dummy, FALSE, version) != VTSS_OK) ||
                        (ipmc_mgmt_get_intf_info(isid, vidx, &param, version) != VTSS_OK)) {
                        continue;
                    }

                    if (prt_title) {
                        ICLI_PRINTF("\nSwitch-%u %s Interface Status\n",
                                    usid,
                                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
                        prt_title = FALSE;
                    }

                    if (!_icli_ipmc_snp_intf_status_show(version, session_id, isid, ctrl, state, &param, detail)) {
                        ICLI_PRINTF("%% Failed to display %s VLAN %u statistics.\n\n",
                                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                    vidx);
                    }
                }
            }
        } else {
            vidx = 0x0;
            while (ipmc_mgmt_get_intf_state_querier(FALSE, &vidx, &state, &dummy, TRUE, version) == VTSS_OK) {
                memset(&param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                if (ipmc_mgmt_get_intf_info(isid, vidx, &param, version) != VTSS_OK) {
                    continue;
                }

                if (prt_title) {
                    ICLI_PRINTF("\nSwitch-%u %s Interface Status\n",
                                usid,
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
                    prt_title = FALSE;
                }

                if (!_icli_ipmc_snp_intf_status_show(version, session_id, isid, ctrl, state, &param, detail)) {
                    ICLI_PRINTF("%% Failed to display %s VLAN %u statistics.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                }
            }
        }
    }
}

static BOOL _icli_ipmc_snp_mrouter_show(ipmc_ip_version_t version,
                                        i32 session_id,
                                        vtss_isid_t isid, vtss_usid_t usid,
                                        ipmc_conf_router_port_t *sta_mr,
                                        ipmc_dynamic_router_port_t *dyn_mr,
                                        BOOL detail)
{
    BOOL            prt_title;
    port_iter_t     pit;
    vtss_port_no_t  iport;
    i8              portString[MGMT_PORT_BUF_SIZE];

    if (!sta_mr || !dyn_mr) {
        return FALSE;
    }

    prt_title = TRUE;
    if (detail) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    } else {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    }
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        if (sta_mr->ports[iport] || dyn_mr->ports[iport]) {
            if (prt_title) {
                ICLI_PRINTF("\nSwitch-%u %s Router Port Status\n",
                            usid,
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
                prt_title = FALSE;
            }

            memset(portString, 0x0, sizeof(portString));
            if (sta_mr->ports[iport] && dyn_mr->ports[iport]) {
                ICLI_PRINTF("%s: Static and Dynamic Router Port\n",
                            port_isid_port_no_is_stack(isid, iport) ? "*Stacking Port" :
                            icli_port_info_txt_short(usid, pit.uport, portString));
            } else {
                ICLI_PRINTF("%s: %s Router Port\n",
                            port_isid_port_no_is_stack(isid, iport) ? "*Stacking Port" :
                            icli_port_info_txt_short(usid, pit.uport, portString),
                            sta_mr->ports[iport] ? "Static" : "Dynamic");
            }
        }
    }

    return TRUE;
}

static void _icli_ipmc_snp_mrouter_display(ipmc_ip_version_t version, i32 session_id, BOOL detail)
{
    switch_iter_t               sit;
    vtss_usid_t                 usid;
    vtss_isid_t                 isid;
    ipmc_conf_router_port_t     ipmc_rports;
    ipmc_dynamic_router_port_t  ipmc_drports;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;
        if ((ipmc_mgmt_get_static_router_port(isid, &ipmc_rports, version) != VTSS_OK) ||
            (ipmc_mgmt_get_dynamic_router_ports(isid, &ipmc_drports, version) != VTSS_OK)) {
            continue;
        }

        usid = sit.usid;
        if (!_icli_ipmc_snp_mrouter_show(version, session_id, isid, usid, &ipmc_rports, &ipmc_drports, detail)) {
            ICLI_PRINTF("%% Failed to display %s router ports on Switch-%u.\n\n",
                        ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                        usid);
        }
    }
}

BOOL icli_ipmc_snp_check_present(IN u32 session_id, IN icli_runtime_ask_t ask, OUT icli_runtime_t *runtime)
{
    icli_privilege_t    current_priv;

    switch ( ask ) {
    case ICLI_ASK_PRESENT:
        if (ICLI_PRIVILEGE_GET(&current_priv) == ICLI_RC_OK) {
            if (current_priv < ICLI_PRIVILEGE_15) {
                runtime->present = FALSE;
            } else {
                runtime->present = TRUE;
            }

            return TRUE;
        }

        break;
    case ICLI_ASK_BYWORD:
    case ICLI_ASK_HELP:
    case ICLI_ASK_RANGE:
    default:

        break;
    }

    return FALSE;
}

static void _icli_ipmc_snp_show_global_status(ipmc_ip_version_t version, i32 session_id,
                                              BOOL from_db, BOOL detail)
{
    BOOL    ipmc_state;

    if (ipmc_mgmt_get_mode(&ipmc_state, version) != VTSS_OK) {
        ipmc_state = FALSE;
    }
    ICLI_PRINTF("\n\r%s Snooping is %s snooping %s control plane.\n\r",
                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                ipmc_state ? "enabled to start" : "disabled to stop",
                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));

    if (detail) {
        /*
            The flooding control takes effect only when Snooping is enabled.
            When Snooping is disabled, unregistered traffic flooding is always
            active in spite of this setting.
        */
        if (ipmc_state) {
#if defined(VTSS_SW_OPTION_SMB_IPMC)
            BOOL    pxy_state;

            if (ipmc_mgmt_get_proxy(&pxy_state, version) != VTSS_OK) {
                pxy_state = FALSE;
            }

            if (pxy_state) {
                ICLI_PRINTF("(%s proxy for JOIN/LEAVE mechanism is active)\n\r",
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
            } else {
                if (ipmc_mgmt_get_leave_proxy(&pxy_state, version) != VTSS_OK) {
                    pxy_state = FALSE;
                }

                if (pxy_state) {
                    ICLI_PRINTF("(%s proxy for LEAVE mechanism is active)\n\r",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
                }
            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

            if (ipmc_mgmt_get_unreg_flood(&ipmc_state, version) != VTSS_OK) {
                ipmc_state = FALSE;
            }
        } else {
            ipmc_state = TRUE;
        }
        ICLI_PRINTF("Multicast streams destined to unregistered %s groups will be %s.\n\r",
                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                    ipmc_state ? "flooding" : "blocking");

        if (from_db) {
            ipmc_prefix_t   ipmc_prefix;

            if (ipmc_mgmt_get_ssm_range(version, &ipmc_prefix) == VTSS_OK) {
                i8  adrString[40];

                memset(adrString, 0x0, sizeof(adrString));
                ICLI_PRINTF("Groups in range %s/%u follow %s SSM registration service model.\n\r",
                            (version == IPMC_IP_VERSION_IGMP) ?
                            misc_ipv4_txt(ipmc_prefix.addr.value.prefix, adrString) :
                            misc_ipv6_txt(&ipmc_prefix.addr.array.prefix, adrString),
                            ipmc_prefix.len,
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
            }
        }
    }
}

BOOL icli_ipmc_snp_show_statistics(ipmc_ip_version_t version, i32 session_id,
                                   BOOL by_vlan, icli_unsigned_range_t *vlist,
                                   BOOL mrouter, BOOL detail)
{
    if (by_vlan && !vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    _icli_ipmc_snp_show_global_status(version, session_id, FALSE, detail);

    if (by_vlan && vlist) {
        _icli_ipmc_snp_intf_statistics_display(version, session_id, vlist, detail);
    } else if (mrouter) {
        _icli_ipmc_snp_mrouter_display(version, session_id, detail);
    } else {
        _icli_ipmc_snp_intf_statistics_display(version, session_id, NULL, detail);
    }

    return TRUE;
}

BOOL icli_ipmc_snp_clear_statistics(ipmc_ip_version_t version, i32 session_id,
                                    BOOL by_vlan, icli_unsigned_range_t *vlist)
{
    if (by_vlan && !vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    if (by_vlan && vlist) {
        u32         idx;
        vtss_vid_t  vidx, bnd;

        for (idx = 0; idx < vlist->cnt; idx++) {
            bnd = vlist->range[idx].max;
            for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
                if (ipmc_mgmt_clear_stat_counter(VTSS_ISID_GLOBAL, version, vidx) != VTSS_OK) {
                    ICLI_PRINTF("%% Failed to clear %s VLAN %u statistics.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                }
            }
        }
    } else {
        if (ipmc_mgmt_clear_stat_counter(VTSS_ISID_GLOBAL, version, VTSS_IPMC_VID_ALL) != VTSS_OK) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL icli_ipmc_snp_show_db(ipmc_ip_version_t version, i32 session_id,
                           BOOL by_vlan, icli_unsigned_range_t *vlist,
                           BOOL by_port, icli_stack_port_range_t *plist,
                           BOOL inc_sfm, BOOL detail)
{
    icli_switch_port_range_t        switch_range;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;

    BOOL                            dummy, bypassing;
    vtss_vid_t                      vid;

    ipmc_prot_intf_group_entry_t    intf_group_entry;
    ipmc_group_db_t                 *grp_db;
    u32                             grpCnt;

    if ((by_vlan && !vlist) || (by_port && !plist)) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    _icli_ipmc_snp_show_global_status(version, session_id, TRUE, detail);

    if (by_port && plist) {
        if (!icli_cmd_port_range_exist(session_id, plist, TRUE, TRUE) &&
            plist->cnt == 0) {
            //Stop process since the port list counter is zero
            return TRUE;
        }
    }

    ICLI_PRINTF("\n%s Group Database\n", ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));

    bypassing = FALSE;
    memset(&switch_range, 0, sizeof(switch_range));
    while (icli_switch_get_next(&switch_range)) {
        if (!msg_switch_exists(switch_range.isid)) {
            continue;
        }
        isid = switch_range.isid;
        usid = switch_range.usid;

        vid = 0;
        grpCnt = 0;
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, version) == VTSS_OK) {
            if (by_vlan && !_icli_ipmc_snp_chk_by_vlan(vid, vlist)) {
                continue;
            }

            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (ipmc_mgmt_get_next_intf_group_info(isid, vid, &intf_group_entry, version) == VTSS_OK) {
                grp_db = &intf_group_entry.db;
                if (by_port && !_icli_ipmc_snp_chk_by_port_msk(isid, grp_db->port_mask, plist)) {
                    continue;
                }
                if (!grpCnt) {
                    ICLI_PRINTF("\nSwitch-%u %s Group Table\n",
                                usid,
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));
                }
                grpCnt++;

                _icli_ipmc_snp_db_display(session_id, &intf_group_entry, isid, by_port, plist, inc_sfm, detail);
                bypassing = (icli_session_printf(session_id, "%s", "") == ICLI_RC_ERR_BYPASS);
                if (bypassing) {
                    break;
                }
            }

            if (bypassing) {
                break;
            }
        }

        if (bypassing) {
            break;
        }

        ICLI_PRINTF("\nSwitch-%u %s Group Count: %u\n",
                    usid,
                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                    grpCnt);
    }

    return TRUE;
}

BOOL icli_ipmc_snp_intf_config(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist)
{
    vtss_rc     intf_rc;
    u32         idx;
    vtss_vid_t  vidx, bnd, icli_ipmcsnp_intf_vid;
    BOOL        icli_ipmcsnp_intf_state, icli_ipmcsnp_intf_querier;

    if (!vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (idx = 0; idx < vlist->cnt; idx++) {
        bnd = vlist->range[idx].max;
        for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
            icli_ipmcsnp_intf_vid = vidx;
            if (ipmc_mgmt_get_intf_state_querier(TRUE, &icli_ipmcsnp_intf_vid,
                                                 &icli_ipmcsnp_intf_state,
                                                 &icli_ipmcsnp_intf_querier,
                                                 FALSE, version) != VTSS_OK) {
                icli_ipmcsnp_intf_state = IPMC_DEF_INTF_STATE_VALUE;
                icli_ipmcsnp_intf_querier = IPMC_DEF_INTF_QUERIER_VALUE;
                intf_rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_ADD, vidx,
                                                           &icli_ipmcsnp_intf_state,
                                                           &icli_ipmcsnp_intf_querier,
                                                           version);
                if (intf_rc == VTSS_OK) {
                    continue;
                }

                if (intf_rc != IPMC_ERROR_TABLE_IS_FULL) {
                    ICLI_PRINTF("%% Failed to create %s VLAN %u.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                } else {
                    ICLI_PRINTF("%% %s VLAN Interface Table is Full!\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER));

                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

BOOL icli_ipmc_snp_intf_delete(ipmc_ip_version_t version, i32 session_id, BOOL chk, icli_unsigned_range_t *vlist)
{
    vtss_vid_t  icli_ipmcsnp_intf_vid;
    BOOL        icli_ipmcsnp_intf_state, icli_ipmcsnp_intf_querier;

    if (chk && !vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    if (chk && vlist) {
        u32         idx;
        vtss_vid_t  vidx, bnd;

        for (idx = 0; idx < vlist->cnt; idx++) {
            bnd = vlist->range[idx].max;
            for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
                icli_ipmcsnp_intf_vid = vidx;
                if (ipmc_mgmt_get_intf_state_querier(TRUE, &icli_ipmcsnp_intf_vid,
                                                     &icli_ipmcsnp_intf_state,
                                                     &icli_ipmcsnp_intf_querier,
                                                     FALSE, version) == VTSS_OK) {
                    if (ipmc_mgmt_set_intf_state_querier(IPMC_OP_DEL, icli_ipmcsnp_intf_vid,
                                                         &icli_ipmcsnp_intf_state,
                                                         &icli_ipmcsnp_intf_querier,
                                                         version) != VTSS_OK) {
                        ICLI_PRINTF("%% Failed to delete %s VLAN %u.\n\n",
                                    ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                    vidx);
                    }
                } else {
                    ICLI_PRINTF("%% Invalid %s VLAN %u!\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                }
            }
        }
    } else {
        icli_ipmcsnp_intf_vid = 0x0;
        while (ipmc_mgmt_get_intf_state_querier(TRUE, &icli_ipmcsnp_intf_vid,
                                                &icli_ipmcsnp_intf_state,
                                                &icli_ipmcsnp_intf_querier,
                                                TRUE, version) == VTSS_OK) {
            if (ipmc_mgmt_set_intf_state_querier(IPMC_OP_DEL, icli_ipmcsnp_intf_vid,
                                                 &icli_ipmcsnp_intf_state,
                                                 &icli_ipmcsnp_intf_querier,
                                                 version) != VTSS_OK) {
                ICLI_PRINTF("%% Failed to delete %s VLAN %u.\n\n",
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                            icli_ipmcsnp_intf_vid);
            }
        }
    }

    return TRUE;
}

static BOOL _icli_ipmc_snp_intf_ctrl_set(ipmc_ip_version_t version,
                                         i32 session_id,
                                         icli_unsigned_range_t *vlist,
                                         BOOL state, BOOL querier,
                                         BOOL val)
{
    u32         idx;
    vtss_vid_t  vidx, bnd, icli_ipmcsnp_intf_vid;
    BOOL        icli_ipmcsnp_intf_state, icli_ipmcsnp_intf_querier;

    if (!vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (idx = 0; idx < vlist->cnt; idx++) {
        bnd = vlist->range[idx].max;
        for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
            icli_ipmcsnp_intf_vid = vidx;
            if (ipmc_mgmt_get_intf_state_querier(TRUE, &icli_ipmcsnp_intf_vid,
                                                 &icli_ipmcsnp_intf_state,
                                                 &icli_ipmcsnp_intf_querier,
                                                 FALSE, version) == VTSS_OK) {
                if (state) {
                    icli_ipmcsnp_intf_state = val;
                }
                if (querier) {
                    icli_ipmcsnp_intf_querier = val;
                }
                if (ipmc_mgmt_set_intf_state_querier(IPMC_OP_UPD, icli_ipmcsnp_intf_vid,
                                                     &icli_ipmcsnp_intf_state,
                                                     &icli_ipmcsnp_intf_querier,
                                                     version) != VTSS_OK) {
                    ICLI_PRINTF("%% Failed to control %s VLAN %u.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                icli_ipmcsnp_intf_vid);
                }
            } else {
                ICLI_PRINTF("%% Invalid %s VLAN %u!\n\n",
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                            vidx);
            }
        }
    }

    return TRUE;
}

BOOL icli_ipmc_snp_intf_state_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, BOOL val)
{
    return _icli_ipmc_snp_intf_ctrl_set(version,
                                        session_id,
                                        vlist,
                                        1, 0,
                                        val);
}

BOOL icli_ipmc_snp_intf_querier_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, BOOL val)
{
    return _icli_ipmc_snp_intf_ctrl_set(version,
                                        session_id,
                                        vlist,
                                        0, 1,
                                        val);
}

BOOL icli_ipmc_snp_intf_querier_adrs_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, vtss_ipv4_t val)
{
    u32                             idx;
    vtss_vid_t                      vidx, bnd;
    ipmc_prot_intf_entry_param_t    icli_ipmcsnp_intf_param;

    if (!vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (idx = 0; idx < vlist->cnt; idx++) {
        bnd = vlist->range[idx].max;
        for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
            if (ipmc_mgmt_get_intf_info(VTSS_ISID_GLOBAL, vidx, &icli_ipmcsnp_intf_param, version) == VTSS_OK) {
                icli_ipmcsnp_intf_param.querier.QuerierAdrs4 = val;
                if (ipmc_mgmt_set_intf_info(VTSS_ISID_GLOBAL, &icli_ipmcsnp_intf_param, version) != VTSS_OK) {
                    ICLI_PRINTF("%% Failed to set %s VLAN %u's PARAM.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                }
            } else {
                ICLI_PRINTF("%% Invalid %s VLAN %u!\n\n",
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                            vidx);
            }
        }
    }

    return TRUE;
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static BOOL _icli_ipmc_snp_intf_param_set(ipmc_ip_version_t version,
                                          i32 session_id,
                                          icli_unsigned_range_t *vlist,
                                          BOOL compat, BOOL pri, BOOL rv, BOOL qi, BOOL qri, BOOL llqi, BOOL uri,
                                          i32 val)
{
    u32                             idx, cur_qri, cur_qi;
    vtss_vid_t                      vidx, bnd;
    BOOL                            go4setting;
    ipmc_prot_intf_entry_param_t    icli_ipmcsnp_intf_param;

    if (!vlist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (idx = 0; idx < vlist->cnt; idx++) {
        bnd = vlist->range[idx].max;
        for (vidx = vlist->range[idx].min; vidx <= bnd; vidx++) {
            if (ipmc_mgmt_get_intf_info(VTSS_ISID_GLOBAL, vidx, &icli_ipmcsnp_intf_param, version) == VTSS_OK) {
                go4setting = TRUE;
                cur_qri = icli_ipmcsnp_intf_param.querier.MaxResTime;
                cur_qi = icli_ipmcsnp_intf_param.querier.QueryIntvl;

                if (compat) {
                    icli_ipmcsnp_intf_param.cfg_compatibility = val;
                }
                if (pri) {
                    icli_ipmcsnp_intf_param.priority = (val & 0xFF);
                }
                if (rv) {
                    icli_ipmcsnp_intf_param.querier.RobustVari = val;
                }
                if (qi) {
                    icli_ipmcsnp_intf_param.querier.QueryIntvl = val;
                }
                if (qri) {
                    icli_ipmcsnp_intf_param.querier.MaxResTime = val;
                }
                if (llqi) {
                    icli_ipmcsnp_intf_param.querier.LastQryItv = val;
                }
                if (uri) {
                    icli_ipmcsnp_intf_param.querier.UnsolicitR = val;
                }

                if (icli_ipmcsnp_intf_param.querier.RobustVari < 2) {
                    ICLI_PRINTF("%% Robustness Variable %s.\n",
                                icli_ipmcsnp_intf_param.querier.RobustVari ?
                                "SHOULD NOT be one" :
                                "MUST NOT be zero");

                    go4setting = FALSE;
                }

                if (icli_ipmcsnp_intf_param.querier.MaxResTime >=
                    (icli_ipmcsnp_intf_param.querier.QueryIntvl * 10)) {
                    ICLI_PRINTF("%% The number of seconds represented by QRI must be less than QI.\n");
                    ICLI_PRINTF("%% (%s QRI is %u.%u second%s / %s QI is %u second%s)\n",
                                qri ? "Expected" : "Current",
                                qri ? (icli_ipmcsnp_intf_param.querier.MaxResTime / 10) : (cur_qri / 10),
                                qri ? (icli_ipmcsnp_intf_param.querier.MaxResTime % 10) : (cur_qri % 10),
                                qri ? (icli_ipmcsnp_intf_param.querier.MaxResTime > 10 ? "s" : "") : (cur_qri > 10 ? "s" : ""),
                                qi ? "Expected" : "Current",
                                qi ? icli_ipmcsnp_intf_param.querier.QueryIntvl : cur_qi,
                                qi ? (icli_ipmcsnp_intf_param.querier.QueryIntvl > 1 ? "s" : "") : (cur_qi > 1 ? "s" : ""));

                    go4setting = FALSE;
                }

                if (!go4setting ||
                    ipmc_mgmt_set_intf_info(VTSS_ISID_GLOBAL, &icli_ipmcsnp_intf_param, version) != VTSS_OK) {
                    ICLI_PRINTF("%% Failed to set %s VLAN %u's parameters.\n\n",
                                ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                                vidx);
                }
            } else {
                ICLI_PRINTF("%% Invalid %s VLAN %u!\n\n",
                            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
                            vidx);
            }
        }
    }

    return TRUE;
}

BOOL icli_ipmc_snp_intf_compat_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         1, 0, 0, 0, 0, 0, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_pri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 1, 0, 0, 0, 0, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_rv_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 0, 1, 0, 0, 0, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_qi_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 0, 0, 1, 0, 0, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_qri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 0, 0, 0, 1, 0, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_lmqi_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 0, 0, 0, 0, 1, 0,
                                         val);
}

BOOL icli_ipmc_snp_intf_uri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val)
{
    return _icli_ipmc_snp_intf_param_set(version,
                                         session_id,
                                         vlist,
                                         0, 0, 0, 0, 0, 0, 1,
                                         val);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

BOOL icli_ipmc_snp_immediate_leave_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, BOOL state)
{
    u32                         rdx;
    u16                         idx, bgn, cnt;
    vtss_isid_t                 isid;
    vtss_port_no_t              iport;
    ipmc_conf_fast_leave_port_t icli_ipmcsnp_imd_leave;

    if (!plist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (ipmc_mgmt_get_fast_leave_port(isid, &icli_ipmcsnp_imd_leave, version) != VTSS_OK) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            icli_ipmcsnp_imd_leave.ports[iport] = state;
        }

        (void) ipmc_mgmt_set_fast_leave_port(isid, &icli_ipmcsnp_imd_leave, version);
    }

    return TRUE;
}

BOOL icli_ipmc_snp_mrouter_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, BOOL state)
{
    u32                     rdx;
    u16                     idx, bgn, cnt;
    vtss_isid_t             isid;
    vtss_port_no_t          iport;
    ipmc_conf_router_port_t icli_ipmcsnp_mrouter;

    if (!plist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (ipmc_mgmt_get_static_router_port(isid, &icli_ipmcsnp_mrouter, version) != VTSS_OK) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            icli_ipmcsnp_mrouter.ports[iport] = state;
        }

        (void) ipmc_mgmt_set_router_port(isid, &icli_ipmcsnp_mrouter, version);
    }

    return TRUE;
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
BOOL icli_ipmc_snp_port_throttle_clear(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist)
{
    u32                             rdx;
    u16                             idx, bgn, cnt;
    vtss_isid_t                     isid;
    vtss_port_no_t                  iport;
    ipmc_conf_throttling_max_no_t   icli_ipmcsnp_pg_throttling;

    if (!plist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (ipmc_mgmt_get_throttling_max_count(isid, &icli_ipmcsnp_pg_throttling, version) != VTSS_OK) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            icli_ipmcsnp_pg_throttling.ports[iport] = IPMC_DEF_THROLLTING_VALUE;
        }

        (void) ipmc_mgmt_set_throttling_max_count(isid, &icli_ipmcsnp_pg_throttling, version);
    }

    return TRUE;
}

BOOL icli_ipmc_snp_port_throttle_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, i32 maxg)
{
    u32                             rdx;
    u16                             idx, bgn, cnt;
    vtss_isid_t                     isid;
    vtss_port_no_t                  iport;
    ipmc_conf_throttling_max_no_t   icli_ipmcsnp_pg_throttling;

    if (!plist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        if (ipmc_mgmt_get_throttling_max_count(isid, &icli_ipmcsnp_pg_throttling, version) != VTSS_OK) {
            continue;
        }

        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            icli_ipmcsnp_pg_throttling.ports[iport] = maxg;
        }

        (void) ipmc_mgmt_set_throttling_max_count(isid, &icli_ipmcsnp_pg_throttling, version);
    }

    return TRUE;
}

BOOL icli_ipmc_snp_port_filter_clear(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist)
{
    u32                                 rdx;
    u16                                 idx, bgn, cnt;
    vtss_isid_t                         isid;
    vtss_port_no_t                      iport;
    ipmc_conf_port_group_filtering_t    icli_ipmcsnp_pg_filtering;

    if (!plist) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            icli_ipmcsnp_pg_filtering.port_no = iport;
            if (ipmc_mgmt_get_port_group_filtering(isid, &icli_ipmcsnp_pg_filtering, version) == VTSS_OK) {
                (void) ipmc_mgmt_del_port_group_filtering(isid, &icli_ipmcsnp_pg_filtering, version);
            }
        }
    }

    return TRUE;
}

BOOL icli_ipmc_snp_port_filter_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, i8 *profile_name)
{
    u32                                 rdx;
    i32                                 str_len;
    u16                                 idx, bgn, cnt;
    vtss_isid_t                         isid;
    vtss_port_no_t                      iport;
    ipmc_conf_port_group_filtering_t    icli_ipmcsnp_pg_filtering;
    ipmc_lib_profile_mem_t              *pfm;
    ipmc_lib_grp_fltr_profile_t         *fltr_profile;

    if (!plist || !profile_name ||
        !IPMC_LIB_NAME_IDX_CHECK(profile_name, VTSS_IPMC_NAME_MAX_LEN) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        ICLI_PRINTF("%% Invalid operation.\n\n");
        return FALSE;
    }

    str_len = strlen(profile_name);
    fltr_profile = &pfm->profile;
    memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
    strncpy(fltr_profile->data.name, profile_name, str_len);
    if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        ICLI_PRINTF("%% Please specify correct filter profile name.\n\n");
        return FALSE;
    }
    IPMC_MEM_PROFILE_MGIVE(pfm);

    for (rdx = 0; rdx < plist->cnt; rdx++) {
        isid = topo_usid2isid(plist->switch_range[rdx].usid);
        cnt = plist->switch_range[rdx].port_cnt;
        bgn = plist->switch_range[rdx].begin_uport;
        for (idx = 0; idx < cnt; idx++) {
            iport = uport2iport(bgn + idx);
            if (port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            memset(&icli_ipmcsnp_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
            icli_ipmcsnp_pg_filtering.port_no = iport;
            strncpy(icli_ipmcsnp_pg_filtering.addr.profile.name, profile_name, str_len);
            (void) ipmc_mgmt_set_port_group_filtering(isid, &icli_ipmcsnp_pg_filtering, version);
        }
    }

    return TRUE;
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */
/* FUNCTION_END */
