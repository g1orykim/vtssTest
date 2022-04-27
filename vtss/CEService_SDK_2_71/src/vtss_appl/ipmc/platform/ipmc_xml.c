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
#include "conf_xml_api.h"
#include "misc_api.h"
#include "port_api.h"   /* For VTSS_FRONT_PORT_COUNT */
#include "ipmc.h"
#include "ipmc_api.h"
#include "vlan_api.h"

#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_IPMC,
    CX_TAG_MLD,
    CX_TAG_IGMP,

    /* Group Tags */
    CX_TAG_SSM_RANGE,
    CX_TAG_PORT_TABLE,
    CX_TAG_VLAN_TABLE,
    CX_TAG_FILTER_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,
    CX_TAG_FLOOD,
    CX_TAG_LEAVE_PROXY,
    CX_TAG_PROXY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t ipmc_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_IPMC] = {
        .name  = "ipmc",
        .descr = "MLD/IGMP Snooping",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_MLD] = {
        .name  = "mld",
        .descr = "Multicast Listener Discovery",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_IGMP] = {
        .name  = "igmp",
        .descr = "Internet Group Management Protocol",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SSM_RANGE] = {
        .name  = "ssm_range",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_VLAN_TABLE] = {
        .name  = "vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_FILTER_TABLE] = {
        .name  = "filter_grps_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_FLOOD] = {
        .name  = "flood",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LEAVE_PROXY] = {
        .name  = "leave_proxy",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PROXY] = {
        .name  = "proxy",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

#ifdef VTSS_SW_OPTION_SMB_IPMC
/* Keyword for IPMC Compatibility */
static const cx_kw_t cx_kw_igmp_compatibility[] = {
    { "auto",   VTSS_IPMC_COMPAT_MODE_AUTO },
    { "v1",     VTSS_IPMC_COMPAT_MODE_OLD },
    { "v2",     VTSS_IPMC_COMPAT_MODE_GEN },
    { "v3",     VTSS_IPMC_COMPAT_MODE_SFM },
    { NULL,     0 }
};

static const cx_kw_t cx_kw_mld_compatibility[] = {
    { "auto",   VTSS_IPMC_COMPAT_MODE_AUTO },
    { "v1",     VTSS_IPMC_COMPAT_MODE_GEN },
    { "v2",     VTSS_IPMC_COMPAT_MODE_SFM },
    { NULL,     0 }
};

static vtss_rc cx_add_stx_port_single(cx_get_state_t *s)
{
    u32     portCnt;
    char    buf[80] = {0};

    if (s->isid == VTSS_ISID_GLOBAL) {
        portCnt = VTSS_PORTS;
    } else {
        portCnt = port_isid_port_count(s->isid);
    }
    sprintf(buf, "%u-%lu", 1, portCnt);
    return cx_add_attr_txt(s, "port", buf);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static BOOL cx_mld_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    ipmc_conf_router_port_t         router_port;
    ipmc_conf_fast_leave_port_t     fastleave_port;
    ipmc_conf_throttling_max_no_t   ipmc_throttling_max_no;

    return (ipmc_mgmt_get_static_router_port(context->isid, &router_port, IPMC_IP_VERSION_MLD) == VTSS_OK &&
            ipmc_mgmt_get_fast_leave_port(context->isid, &fastleave_port, IPMC_IP_VERSION_MLD) == VTSS_OK &&
            ipmc_mgmt_get_throttling_max_count(context->isid, &ipmc_throttling_max_no, IPMC_IP_VERSION_MLD) == VTSS_OK &&
            router_port.ports[port_a] == router_port.ports[port_b] &&
            fastleave_port.ports[port_a] == fastleave_port.ports[port_b] &&
            ipmc_throttling_max_no.ports[port_a] == ipmc_throttling_max_no.ports[port_b]);
}

static vtss_rc cx_mld_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    ipmc_conf_router_port_t         router_port;
    ipmc_conf_fast_leave_port_t     fastleave_port;
    ipmc_conf_throttling_max_no_t   ipmc_throttling_max_no;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "router"));
        CX_RC(cx_add_stx_bool(s, "fastleave"));
        CX_RC(cx_add_stx_ulong(s, "limit_group_number", 0, 10));
        return cx_add_stx_end(s);
    }

    CX_RC(ipmc_mgmt_get_static_router_port(context->isid, &router_port, IPMC_IP_VERSION_MLD));
    CX_RC(ipmc_mgmt_get_fast_leave_port(context->isid, &fastleave_port, IPMC_IP_VERSION_MLD));
    CX_RC(ipmc_mgmt_get_throttling_max_count(context->isid, &ipmc_throttling_max_no, IPMC_IP_VERSION_MLD));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "router", router_port.ports[port_no]));
    CX_RC(cx_add_attr_bool(s, "fastleave", fastleave_port.ports[port_no]));
    CX_RC(cx_add_attr_ulong(s, "limit_group_number", ipmc_throttling_max_no.ports[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static BOOL cx_igmp_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    ipmc_conf_router_port_t         router_port;
    ipmc_conf_fast_leave_port_t     fastleave_port;
    ipmc_conf_throttling_max_no_t   ipmc_throttling_max_no;

    return (ipmc_mgmt_get_static_router_port(context->isid, &router_port, IPMC_IP_VERSION_IGMP) == VTSS_OK &&
            ipmc_mgmt_get_fast_leave_port(context->isid, &fastleave_port, IPMC_IP_VERSION_IGMP) == VTSS_OK &&
            ipmc_mgmt_get_throttling_max_count(context->isid, &ipmc_throttling_max_no, IPMC_IP_VERSION_IGMP) == VTSS_OK &&
            router_port.ports[port_a] == router_port.ports[port_b] &&
            fastleave_port.ports[port_a] == fastleave_port.ports[port_b] &&
            ipmc_throttling_max_no.ports[port_a] == ipmc_throttling_max_no.ports[port_b]);
}

static vtss_rc cx_igmp_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    ipmc_conf_router_port_t         router_port;
    ipmc_conf_fast_leave_port_t     fastleave_port;
    ipmc_conf_throttling_max_no_t   ipmc_throttling_max_no;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "router"));
        CX_RC(cx_add_stx_bool(s, "fastleave"));
        CX_RC(cx_add_stx_ulong(s, "limit_group_number", 0, 10));
        return cx_add_stx_end(s);
    }

    CX_RC(ipmc_mgmt_get_static_router_port(context->isid, &router_port, IPMC_IP_VERSION_IGMP));
    CX_RC(ipmc_mgmt_get_fast_leave_port(context->isid, &fastleave_port, IPMC_IP_VERSION_IGMP));
    CX_RC(ipmc_mgmt_get_throttling_max_count(context->isid, &ipmc_throttling_max_no, IPMC_IP_VERSION_IGMP));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "router", router_port.ports[port_no]));
    CX_RC(cx_add_attr_bool(s, "fastleave", fastleave_port.ports[port_no]));
    CX_RC(cx_add_attr_ulong(s, "limit_group_number", ipmc_throttling_max_no.ports[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static cx_tag_id_t ipmc_cx_parse_get_previous_tag(cx_set_state_t *s, int pre_level)
{
    cx_tag_id_t id = 0;
    int         i;

    /* Lookup the previous tag ID (mld or igmp)
       For entry data, the previous level is 2.
       For other parameters, the previous level is 1. */
    for (i = 0; i < s->tag_count; i++) {
        if (s->tag[i].module == s->mod_id && s->tag[i].id == s->id) {
            id = s->tag[i - pre_level].id;
            break;
        }
    }

    return id;
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static vtss_rc cx_parse_ipmc_profile_name(cx_set_state_t *s, char *name, char *profile_name)
{
    u32     name_len, idx;
    BOOL    valid_flag;

    CX_RC(cx_parse_txt(s, name, profile_name, VTSS_IPMC_NAME_STRING_MAX_LEN));

    if ((name_len = strlen(profile_name)) != 0) {
        valid_flag = FALSE;
        for (idx = 0; idx < name_len; idx++) {
            if ((profile_name[idx] < 48) || (profile_name[idx] > 122)) {
                valid_flag = FALSE;
                break;
            } else {
                if (((profile_name[idx] > 64) && (profile_name[idx] < 91)) ||
                    (profile_name[idx] > 96)) {
                    valid_flag = TRUE;
                } else {
                    if (profile_name[idx] > 57) {
                        valid_flag = FALSE;
                        break;
                    }
                }
            }
        }
    } else {
        valid_flag = TRUE;
    }

    if (!valid_flag) {
        CX_RC(cx_parm_invalid(s));
    }

    return s->rc;
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static vtss_rc ipmc_cx_parse_func(cx_set_state_t *s)
{
    switch ( s->cmd ) {
    case CX_PARSE_CMD_PARM: {
        ipmc_ip_version_t                   ipmc_ip_version;
        BOOL                                state = FALSE, querier = FALSE;
        vtss_vid_t                          vid, chk_vid;
        BOOL                                chk_st, chk_qu;
        port_iter_t                         pit;
        vtss_port_no_t                      port_idx;
        BOOL                                port_list[VTSS_PORT_ARRAY_SIZE + 1];
        BOOL                                global;
#ifdef VTSS_SW_OPTION_SMB_IPMC
        ipmc_conf_port_group_filtering_t    ipmc_xml_parse_pg_filtering;
        ipmc_prefix_t                       pfx;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        ipmc_prot_intf_entry_param_t        intf_param;

        global = (s->isid == VTSS_ISID_GLOBAL);

        if (s->id == CX_TAG_ENTRY) {
            ipmc_ip_version = ipmc_cx_parse_get_previous_tag(s, 2) == CX_TAG_IGMP ? IPMC_IP_VERSION_IGMP : IPMC_IP_VERSION_MLD;
        } else {
            ipmc_ip_version = ipmc_cx_parse_get_previous_tag(s, 1) == CX_TAG_IGMP ? IPMC_IP_VERSION_IGMP : IPMC_IP_VERSION_MLD;
        }

#ifndef VTSS_SW_OPTION_SMB_IPMC
        if (ipmc_ip_version == IPMC_IP_VERSION_MLD) {
            sprintf(s->msg, "Not support MLD Snooping!");
            s->rc = CONF_XML_ERROR_FILE_PARM;
            return s->rc;
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        switch ( s->id ) {
        case CX_TAG_MODE:
            if (global) {
                if (cx_parse_val_bool(s, &state, 1) == VTSS_OK && s->apply) {
                    CX_RC(ipmc_mgmt_set_mode(&state, ipmc_ip_version));
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_FLOOD:
            if (global) {
                if (cx_parse_val_bool(s, &state, 1) == VTSS_OK && s->apply) {
                    CX_RC(ipmc_mgmt_set_unreg_flood(&state, ipmc_ip_version));
                }
            } else {
                s->ignored = 1;
            }
            break;
#ifdef VTSS_SW_OPTION_SMB_IPMC
        case CX_TAG_SSM_RANGE:
            if (!global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_LEAVE_PROXY:
            if (global) {
                if (cx_parse_val_bool(s, &state, 1) == VTSS_OK && s->apply) {
                    CX_RC(ipmc_mgmt_set_leave_proxy(&state, ipmc_ip_version));
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PROXY:
            if (global) {
                if (cx_parse_val_bool(s, &state, 1) == VTSS_OK && s->apply) {
                    CX_RC(ipmc_mgmt_set_proxy(&state, ipmc_ip_version));
                }
            } else {
                s->ignored = 1;
            }
            break;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        case CX_TAG_VLAN_TABLE:
            if (!global) {
                s->ignored = 1;
            } else {
                if (s->apply) {
                    chk_vid = 0;
                    while (ipmc_mgmt_get_intf_state_querier(TRUE, &chk_vid, &chk_st, &chk_qu, TRUE, ipmc_ip_version) == VTSS_OK) {
                        CX_RC(ipmc_mgmt_set_intf_state_querier(IPMC_OP_DEL, chk_vid, &chk_st, &chk_qu, ipmc_ip_version));
                    }
                }
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
#ifdef VTSS_SW_OPTION_SMB_IPMC
        case CX_TAG_FILTER_TABLE:
            if (global) {
                s->ignored = 1;
            } else if (s->apply) {
                /* Flush filtering group table */
                (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    memset(&ipmc_xml_parse_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
                    ipmc_xml_parse_pg_filtering.port_no = pit.iport;
                    if (ipmc_mgmt_get_port_group_filtering(s->isid, &ipmc_xml_parse_pg_filtering, ipmc_ip_version) == VTSS_OK) {
                        CX_RC(ipmc_mgmt_del_port_group_filtering(s->isid, &ipmc_xml_parse_pg_filtering, ipmc_ip_version));
                    }
                }
            }
            break;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        case CX_TAG_ENTRY:
            if (global && s->group == CX_TAG_VLAN_TABLE &&
                cx_parse_vid(s, &vid, 1) == VTSS_OK) {
                u32 kw_val, intf_pri = IPMC_DEF_INTF_PRI;

                memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                intf_param.vid = vid;
                intf_param.cfg_compatibility = IPMC_DEF_INTF_COMPAT;
                intf_param.priority = IPMC_DEF_INTF_PRI;
                intf_param.querier.QuerierAdrs4 = IPMC_DEF_INTF_QUERIER_ADRS4;
                intf_param.querier.RobustVari = IPMC_DEF_INTF_RV;
                intf_param.querier.QueryIntvl = IPMC_DEF_INTF_QI;
                intf_param.querier.MaxResTime = IPMC_DEF_INTF_QRI;
                intf_param.querier.LastQryItv = IPMC_DEF_INTF_LMQI;
                intf_param.querier.UnsolicitR = IPMC_DEF_INTF_URI;
                kw_val = intf_param.cfg_compatibility;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
#ifdef VTSS_SW_OPTION_SMB_IPMC
                    if (ipmc_ip_version == IPMC_IP_VERSION_IGMP) {
                        if ((cx_parse_bool(s, "state", &state, 1) != VTSS_OK) &&
                            (cx_parse_bool(s, "querier", &querier, 1) != VTSS_OK) &&
                            (cx_parse_ipv4(s, "address", &intf_param.querier.QuerierAdrs4, NULL, FALSE) != VTSS_OK) &&
                            (cx_parse_kw(s, "compat", cx_kw_igmp_compatibility, &kw_val, 1) != VTSS_OK) &&
                            (cx_parse_ulong(s, "priority", &intf_pri, 0, 7) != VTSS_OK) &&
                            (cx_parse_ulong(s, "rv", &intf_param.querier.RobustVari, 1, 255) != VTSS_OK) &&
                            (cx_parse_ulong(s, "qi", &intf_param.querier.QueryIntvl, 1, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "qri", &intf_param.querier.MaxResTime, 0, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "llqi", &intf_param.querier.LastQryItv, 0, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "uri", &intf_param.querier.UnsolicitR, 0, 31744) != VTSS_OK)) {
                            CX_RC(cx_parm_unknown(s));
                        }
                    } else {
                        if ((cx_parse_bool(s, "state", &state, 1) != VTSS_OK) &&
                            (cx_parse_bool(s, "querier", &querier, 1) != VTSS_OK) &&
                            (cx_parse_kw(s, "compat", cx_kw_mld_compatibility, &kw_val, 1) != VTSS_OK) &&
                            (cx_parse_ulong(s, "priority", &intf_pri, 0, 7) != VTSS_OK) &&
                            (cx_parse_ulong(s, "rv", &intf_param.querier.RobustVari, 1, 255) != VTSS_OK) &&
                            (cx_parse_ulong(s, "qi", &intf_param.querier.QueryIntvl, 1, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "qri", &intf_param.querier.MaxResTime, 0, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "llqi", &intf_param.querier.LastQryItv, 0, 31744) != VTSS_OK) &&
                            (cx_parse_ulong(s, "uri", &intf_param.querier.UnsolicitR, 0, 31744) != VTSS_OK)) {
                            CX_RC(cx_parm_unknown(s));
                        }
                    }
#else
                    if (ipmc_ip_version == IPMC_IP_VERSION_IGMP) {
                        if ((cx_parse_bool(s, "state", &state, 1) != VTSS_OK) &&
                            (cx_parse_ipv4(s, "address", &intf_param.querier.QuerierAdrs4, NULL, FALSE) != VTSS_OK) &&
                            (cx_parse_bool(s, "querier", &querier, 1) != VTSS_OK)) {
                            CX_RC(cx_parm_unknown(s));
                        }
                    } else {
                        if ((cx_parse_bool(s, "state", &state, 1) != VTSS_OK) &&
                            (cx_parse_bool(s, "querier", &querier, 1) != VTSS_OK)) {
                            CX_RC(cx_parm_unknown(s));
                        }
                    }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                }
                intf_param.cfg_compatibility = kw_val;

                if (s->apply &&
                    (ipmc_mgmt_set_intf_state_querier(IPMC_OP_ADD, vid, &state, &querier, ipmc_ip_version) == VTSS_OK)) {
                    intf_param.priority = (u8)(intf_pri & 0xFF);
                    CX_RC(ipmc_mgmt_set_intf_info(s->isid, &intf_param, ipmc_ip_version));
                }
            } else if (!global && s->group == CX_TAG_PORT_TABLE &&
                       cx_parse_ports(s, port_list, TRUE) == VTSS_OK) {
                ipmc_conf_router_port_t         router_port;
                ipmc_conf_fast_leave_port_t     fastleave_port;
                ipmc_conf_throttling_max_no_t   throttling_max_no;
                BOOL                            r_fnd, l_fnd, t_fnd, rtr, lve;
                u32                             trt;

                r_fnd = l_fnd = t_fnd = FALSE;
                rtr = lve = FALSE;
                trt = 0;
                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "router", &rtr, 1) == VTSS_OK) {
                        r_fnd = TRUE;
                    } else if (cx_parse_bool(s, "fastleave", &lve, 1) == VTSS_OK) {
                        l_fnd = TRUE;
                    } else if (cx_parse_ulong(s, "limit_group_number", &trt, 0, 10) == VTSS_OK) {
                        t_fnd = TRUE;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                T_D("%s / %s-RTR / %s-LVE / %s-TRT",
                    s->apply ? "APPLY" : "SKIP",
                    r_fnd ? "Get" : "Miss",
                    l_fnd ? "Get" : "Miss",
                    t_fnd ? "Get" : "Miss");
                if (s->apply &&
                    ipmc_mgmt_get_static_router_port(s->isid, &router_port, ipmc_ip_version) == VTSS_OK &&
                    ipmc_mgmt_get_fast_leave_port(s->isid, &fastleave_port, ipmc_ip_version) == VTSS_OK &&
                    ipmc_mgmt_get_throttling_max_count(s->isid, &throttling_max_no, ipmc_ip_version) == VTSS_OK) {
                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        port_idx = pit.iport;

                        if (port_list[iport2uport(port_idx)]) {
                            if (r_fnd) {
                                router_port.ports[port_idx] = rtr;
                            }
                            if (l_fnd) {
                                fastleave_port.ports[port_idx] = lve;
                            }
                            if (t_fnd) {
                                throttling_max_no.ports[port_idx] = trt;
                            }
                        }
                    }

                    CX_RC(ipmc_mgmt_set_router_port(s->isid, &router_port, ipmc_ip_version));
                    CX_RC(ipmc_mgmt_set_fast_leave_port(s->isid, &fastleave_port, ipmc_ip_version));
                    CX_RC(ipmc_mgmt_set_throttling_max_count(s->isid, &throttling_max_no, ipmc_ip_version));
                }
            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
            else if (global && s->group == CX_TAG_SSM_RANGE) {
                BOOL    pfx_adr_fnd, pfx_len_fnd;

                memset(&pfx, 0x0, sizeof(ipmc_prefix_t));
                if (ipmc_mgmt_get_ssm_range(ipmc_ip_version, &pfx) != VTSS_OK) {
                    if (ipmc_ip_version == IPMC_IP_VERSION_IGMP) {
                        pfx.addr.value.prefix = VTSS_IPMC_SSM4_RANGE_PREFIX;
                        pfx.len = VTSS_IPMC_SSM4_RANGE_LEN;
                    } else {
                        pfx.addr.array.prefix.addr[0] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 24) & 0xFF;
                        pfx.addr.array.prefix.addr[1] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 16) & 0xFF;
                        pfx.addr.array.prefix.addr[2] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 8) & 0xFF;
                        pfx.addr.array.prefix.addr[3] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 0) & 0xFF;
                        pfx.len = VTSS_IPMC_SSM6_RANGE_LEN;
                    }
                }

                pfx_adr_fnd = pfx_len_fnd = FALSE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (ipmc_ip_version == IPMC_IP_VERSION_IGMP) {
                        if (cx_parse_ipmcv4(s, "ssm_prefix", &pfx.addr.value.prefix) == VTSS_OK) {
                            pfx_adr_fnd = TRUE;
                        } else if (cx_parse_ulong(s, "ssm_length", &pfx.len, 4, 32) == VTSS_OK) {
                            pfx_len_fnd = TRUE;
                        } else {
                            CX_RC(cx_parm_unknown(s));
                        }
                    } else {
                        if (cx_parse_ipmcv6(s, "ssm_prefix", &pfx.addr.array.prefix) == VTSS_OK) {
                            pfx_adr_fnd = TRUE;
                        } else if (cx_parse_ulong(s, "ssm_length", &pfx.len, 8, 128) == VTSS_OK) {
                            pfx_len_fnd = TRUE;
                        } else {
                            CX_RC(cx_parm_unknown(s));
                        }
                    }
                }

                T_D("%s / %s-PFX_ADR / %s-PFX_LEN",
                    s->apply ? "APPLY" : "SKIP",
                    pfx_adr_fnd ? "Get" : "Miss",
                    pfx_len_fnd ? "Get" : "Miss");
                if (s->apply && pfx_adr_fnd && pfx_len_fnd) {
                    CX_RC(ipmc_mgmt_set_ssm_range(ipmc_ip_version, &pfx));
                }
            } else if (!global && s->group == CX_TAG_FILTER_TABLE) {
                BOOL            pdx, port_found;
                vtss_uport_no_t uport;

                memset(&ipmc_xml_parse_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
                pdx = port_found = FALSE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_ulong(s, "port", &uport, 1, VTSS_PORTS) == VTSS_OK) {
#if VTSS_SWITCH_STACKABLE
                        if (!port_isid_port_no_is_stack(s->isid, uport2iport(uport))) {
                            port_found = TRUE;
                        }
#else
                        port_found = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */
                    } else if (cx_parse_ipmc_profile_name(s, "profile", ipmc_xml_parse_pg_filtering.addr.profile.name) == VTSS_OK)  {
                        if (strlen(ipmc_xml_parse_pg_filtering.addr.profile.name)) {
                            pdx = TRUE;
                        }
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                T_D("%s / %s-PORT(%lu) / %s-PROFILE",
                    s->apply ? "APPLY" : "SKIP",
                    port_found ? "Get" : "Miss", uport2iport(uport),
                    pdx ? "Get" : "Miss");
                if (s->apply && port_found && pdx) {
                    ipmc_xml_parse_pg_filtering.port_no = uport2iport(uport);
                    if (ipmc_mgmt_set_port_group_filtering(s->isid, &ipmc_xml_parse_pg_filtering, ipmc_ip_version) != VTSS_OK) {
                        s->rc = CONF_XML_ERROR_FILE_PARM;
                        break;
                    }
                }
            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
            else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        break;
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc ipmc_cx_add_port_table(cx_get_state_t *s, ipmc_ip_version_t ipmc_ip_version)
{
    if (ipmc_ip_version == IPMC_IP_VERSION_IGMP) {
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_igmp_match, cx_igmp_print));
    } else {
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_mld_match, cx_mld_print));
    }
    return VTSS_OK;
}

static vtss_rc ipmc_cx_gen_func(cx_get_state_t *s)
{
    ipmc_ip_version_t                   ipmc_version;
    BOOL                                state, querier;
    u16                                 vid;
    ipmc_prot_intf_entry_param_t        intf_param;
#ifdef VTSS_SW_OPTION_SMB_IPMC
    ipmc_prefix_t                       pfx;
    port_iter_t                         pit;
    ipmc_conf_port_group_filtering_t    ipmc_xml_gen_pg_filtering;
    ipmc_ip_version_t                   supported_ipmc_version = IPMC_IP_VERSION_MLD;
#else
    ipmc_ip_version_t                   supported_ipmc_version = IPMC_IP_VERSION_IGMP;
#endif /* VTSS_SW_OPTION_SMB_IPMC */

    switch ( s->cmd ) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - IPMC */
        T_D("global - ipmc");
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC, 0));

        for (ipmc_version = IPMC_IP_VERSION_IGMP; ipmc_version <= supported_ipmc_version; ipmc_version++) {
            /* Module start tag */
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_tag_line(s, CX_TAG_IGMP, 0));
            } else {
                CX_RC(cx_add_tag_line(s, CX_TAG_MLD, 0));
            }

            /* Fill global parameters */
            if (ipmc_mgmt_get_mode(&state, ipmc_version) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, state));
            }
            if (ipmc_mgmt_get_unreg_flood(&state, ipmc_version) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_FLOOD, state));
            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
            CX_RC(cx_add_tag_line(s, CX_TAG_SSM_RANGE, 0));
            CX_RC(cx_add_stx_start(s));
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_attr_txt(s, "ssm_prefix", "IPv4 Multicast Address"));
                CX_RC(cx_add_stx_ulong(s, "ssm_length", 4, 32));
            } else {
                CX_RC(cx_add_attr_txt(s, "ssm_prefix", "IPv6 Multicast Address"));
                CX_RC(cx_add_stx_ulong(s, "ssm_length", 8, 128));
            }
            CX_RC(cx_add_stx_end(s));
            memset(&pfx, 0x0, sizeof(ipmc_prefix_t));
            if (ipmc_mgmt_get_ssm_range(ipmc_version, &pfx) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    CX_RC(cx_add_attr_ipv4(s, "ssm_prefix", pfx.addr.value.prefix));
                } else {
                    CX_RC(cx_add_attr_ipv6(s, "ssm_prefix", pfx.addr.array.prefix));
                }
                CX_RC(cx_add_attr_ulong(s, "ssm_length", pfx.len));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_SSM_RANGE, 1));

            if (ipmc_mgmt_get_leave_proxy(&state, ipmc_version) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_LEAVE_PROXY, state));
            }
            if (ipmc_mgmt_get_proxy(&state, ipmc_version) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_PROXY, state));
            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

            /* Fill global table entry */
            CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 0));
            // Fill entry syntax
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
            CX_RC(cx_add_stx_bool(s, "state"));
            CX_RC(cx_add_stx_bool(s, "querier"));
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_attr_txt(s, "address", "IPv4 Unicast Address"));
            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_stx_kw(s, "compat", cx_kw_igmp_compatibility));
            } else {
                CX_RC(cx_add_stx_kw(s, "compat", cx_kw_mld_compatibility));
            }
            CX_RC(cx_add_stx_ulong(s, "priority", 0, 7));
            CX_RC(cx_add_stx_ulong(s, "rv", 1, 255));
            CX_RC(cx_add_stx_ulong(s, "qi", 1, 31744));
            CX_RC(cx_add_stx_ulong(s, "qri", 0, 31744));
            CX_RC(cx_add_stx_ulong(s, "llqi", 0, 31744));
            CX_RC(cx_add_stx_ulong(s, "uri", 0, 31744));
#endif /* VTSS_SW_OPTION_SMB_IPMC */
            CX_RC(cx_add_stx_end(s));
            // Fill entry database
            vid = 0;
            while (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &state, &querier, TRUE, ipmc_version) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "vid", vid));
                CX_RC(cx_add_attr_bool(s, "state", state));
                CX_RC(cx_add_attr_bool(s, "querier", querier));

                memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                if (ipmc_mgmt_get_intf_info(VTSS_ISID_GLOBAL, vid, &intf_param, ipmc_version) == VTSS_OK) {
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        CX_RC(cx_add_attr_ipv4(s, "address", intf_param.querier.QuerierAdrs4));
                    }

#ifdef VTSS_SW_OPTION_SMB_IPMC
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        CX_RC(cx_add_attr_kw(s, "compat", cx_kw_igmp_compatibility, intf_param.cfg_compatibility));
                    } else {
                        CX_RC(cx_add_attr_kw(s, "compat", cx_kw_mld_compatibility, intf_param.cfg_compatibility));
                    }
                    CX_RC(cx_add_attr_ulong(s, "priority", intf_param.priority));
                    CX_RC(cx_add_attr_ulong(s, "rv", intf_param.querier.RobustVari));
                    CX_RC(cx_add_attr_ulong(s, "qi", intf_param.querier.QueryIntvl));
                    CX_RC(cx_add_attr_ulong(s, "qri", intf_param.querier.MaxResTime));
                    CX_RC(cx_add_attr_ulong(s, "llqi", intf_param.querier.LastQryItv));
                    CX_RC(cx_add_attr_ulong(s, "uri", intf_param.querier.UnsolicitR));
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                } else {
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        CX_RC(cx_add_attr_ipv4(s, "address", 0));
                    }
                }

                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 1));

            /* Module end target */
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_tag_line(s, CX_TAG_IGMP, 1));
            } else {
                CX_RC(cx_add_tag_line(s, CX_TAG_MLD, 1));
            }
        }

        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - IPMC */
        T_D("switch - ipmc");
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC, 0));

        for (ipmc_version = IPMC_IP_VERSION_IGMP; ipmc_version <= supported_ipmc_version; ipmc_version++) {
            /* Module start tag */
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_tag_line(s, CX_TAG_IGMP, 0));
            } else {
                CX_RC(cx_add_tag_line(s, CX_TAG_MLD, 0));
            }

            /* Fill switch port entry */
            (void) ipmc_cx_add_port_table(s, ipmc_version);

#ifdef VTSS_SW_OPTION_SMB_IPMC
            /* Fill switch filtering entry */
            // Fill entry syntax
            CX_RC(cx_add_tag_line(s, CX_TAG_FILTER_TABLE, 0));
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_port_single(s));
            CX_RC(cx_add_attr_txt(s, "profile", "Name of associated profile"));
            CX_RC(cx_add_stx_end(s));
            // Fill entry database
            (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                memset(&ipmc_xml_gen_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
                ipmc_xml_gen_pg_filtering.port_no = pit.iport;
                if ((ipmc_mgmt_get_port_group_filtering(s->isid, &ipmc_xml_gen_pg_filtering, ipmc_version) == VTSS_OK) &&
                    strlen(ipmc_xml_gen_pg_filtering.addr.profile.name)) {
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "port", iport2uport(ipmc_xml_gen_pg_filtering.port_no)));
                    CX_RC(cx_add_attr_txt(s, "profile", ipmc_xml_gen_pg_filtering.addr.profile.name));
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_FILTER_TABLE, 1));
#endif /* VTSS_SW_OPTION_SMB_IPMC */

            /* Module end target */
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                CX_RC(cx_add_tag_line(s, CX_TAG_IGMP, 1));
            } else {
                CX_RC(cx_add_tag_line(s, CX_TAG_MLD, 1));
            }
        }

        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_IPMC, ipmc_cx_tag_table,
                    0, 0,
                    NULL,                   /* init function       */
                    ipmc_cx_gen_func,       /* Generation fucntion */
                    ipmc_cx_parse_func);    /* parse fucntion      */
