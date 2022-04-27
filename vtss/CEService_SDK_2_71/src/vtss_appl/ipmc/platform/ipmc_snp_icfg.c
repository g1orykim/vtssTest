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

#include "icfg_api.h"
#include "topo_api.h"
#include "misc_api.h"
#include "ipmc.h"
#include "ipmc_api.h"
#include "ipmc_snp_icfg.h"


/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC

#define IPMC_SNP_ICFG_REG(x, y, z, w)  (((x) = vtss_icfg_query_register((y), (z), (w))) == VTSS_OK)

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
static vtss_rc _ipmc_snp_icfg_global(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result,
                                     ipmc_ip_version_t version)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        BOOL                            intf_found, ipmc_icli_intf_state, ipmc_icli_intf_querier;
        vtss_vid_t                      ipmc_icli_intf_vid;
#if defined(VTSS_SW_OPTION_SMB_IPMC)
        ipmc_prefix_t                   ipmc_icli_prefix;
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        /*
            COMMAND = ip/ipv6 igmp/mld snooping vlan <vlan_list>
            COMMAND = no ip/ipv6 igmp/mld snooping vlan [<vlan_list>]
            --- VTSS_SW_OPTION_SMB_IPMC ---
            COMMAND = ip igmp ssm-range <ipv4_mcast> <ipv4_prefix_length:4-32>
            COMMAND = ipv6 mld ssm-range <ipv6_mcast> <ipv6_prefix_length:8-128>
        */
#if defined(VTSS_SW_OPTION_SMB_IPMC)
        if (ipmc_mgmt_get_ssm_range(version, &ipmc_icli_prefix) == VTSS_OK) {
            if (req->all_defaults || ipmc_mgmt_def_ssm_range_chg(version, &ipmc_icli_prefix)) {
                i8  adrString[40];

                memset(adrString, 0x0, sizeof(adrString));
                if (version == IPMC_IP_VERSION_IGMP) {
                    rc |= vtss_icfg_printf(result, "ip igmp ssm-range %s %u\n",
                                           misc_ipv4_txt(ipmc_icli_prefix.addr.value.prefix, adrString),
                                           ipmc_icli_prefix.len);
                } else {
                    rc |= vtss_icfg_printf(result, "ipv6 mld ssm-range %s %u\n",
                                           misc_ipv6_txt(&ipmc_icli_prefix.addr.array.prefix, adrString),
                                           ipmc_icli_prefix.len);
                }
            }
        } else {
            if (version == IPMC_IP_VERSION_IGMP) {
                rc |= vtss_icfg_printf(result, "ip igmp ssm-range 232.0.0.0 8\n");
            } else {
                rc |= vtss_icfg_printf(result, "ipv6 mld ssm-range ff3e:: 96\n");
            }
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        intf_found = FALSE;
        ipmc_icli_intf_vid = 0x0;
        while (ipmc_mgmt_get_intf_state_querier(TRUE, &ipmc_icli_intf_vid,
                                                &ipmc_icli_intf_state,
                                                &ipmc_icli_intf_querier,
                                                TRUE, version) == VTSS_OK) {
            if (!intf_found) {
                rc |= vtss_icfg_printf(result, "%s %s snooping vlan %u",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_icli_intf_vid);
            } else {
                rc |= vtss_icfg_printf(result, ",%u", ipmc_icli_intf_vid);
            }

            intf_found = TRUE;
        }

        if (intf_found) {
            rc |= vtss_icfg_printf(result, "\n");
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, "no %s %s snooping vlan\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        }
    }

    return rc;
}

static vtss_rc _ipmc_snp_icfg_intf(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result,
                                   ipmc_ip_version_t version)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        BOOL        ipmc_icli_intf_state, ipmc_icli_intf_querier;
        vtss_vid_t  ipmc_icli_intf_vid;

        /*
            SUB-MODE: interface vlan <vlan_id>
            COMMAND = ip/ipv6 igmp/mld snooping
            COMMAND = ip igmp querier {election | address <ipv4_ucast>}
            COMMAND = ipv6 mld querier election
            --- VTSS_SW_OPTION_SMB_IPMC ---
            COMMAND = ip igmp snooping compatibility {auto | v1 | v2 | v3}
            COMMAND = ipv6 mld snooping compatibility {auto | v1 | v2}
            COMMAND = ip/ipv6 igmp/mld snooping priority <cos_priority:0-7>
            COMMAND = ip/ipv6 igmp/mld snooping robustness-variable <ipmc_rv:1-255>
            COMMAND = ip/ipv6 igmp/mld snooping query-interval <ipmc_qi:1-31744>
            COMMAND = ip/ipv6 igmp/mld snooping query-max-response-time <ipmc_qri:0-31744>
            COMMAND = ip/ipv6 igmp/mld snooping last-member-query-interval <ipmc_lmqi:0-31744>
            COMMAND = ip/ipv6 igmp/mld snooping unsolicited-report-interval <ipmc_uri:0-31744>
        */
        ipmc_icli_intf_vid = req->instance_id.vlan;

        if (ipmc_mgmt_get_intf_state_querier(TRUE, &ipmc_icli_intf_vid,
                                             &ipmc_icli_intf_state,
                                             &ipmc_icli_intf_querier,
                                             FALSE, version) == VTSS_OK) {
            vtss_isid_t                     isid;
            ipmc_prot_intf_entry_param_t    ipmc_icli_intf_param;

            rc |= vtss_icfg_printf(result, " %s%s %s snooping\n",
                                   ipmc_icli_intf_state ? "" : "no ",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));

            rc |= vtss_icfg_printf(result, " %s%s %s snooping querier election\n",
                                   ipmc_icli_intf_querier ? "" : "no ",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));

            isid = topo_usid2isid(req->instance_id.port.usid);
            memset(&ipmc_icli_intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
            if (ipmc_mgmt_get_intf_info(isid, ipmc_icli_intf_vid, &ipmc_icli_intf_param, version) == VTSS_OK) {
                ipmc_querier_sm_t   *q = &ipmc_icli_intf_param.querier;

                if (version == IPMC_IP_VERSION_IGMP) {
                    if (req->all_defaults || (q->QuerierAdrs4 != IPMC_DEF_INTF_QUERIER_ADRS4)) {
                        i8  adrString[40];

                        memset(adrString, 0x0, sizeof(adrString));
                        rc |= vtss_icfg_printf(result, " %s%s %s snooping querier %s%s\n",
                                               q->QuerierAdrs4 ? "" : "no ",
                                               ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                               ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                               q->QuerierAdrs4 ? "address " : "address",
                                               q->QuerierAdrs4 ? misc_ipv4_txt(q->QuerierAdrs4, adrString) : "");
                    }
                }

#if defined(VTSS_SW_OPTION_SMB_IPMC)
                if (version == IPMC_IP_VERSION_MLD) {
                    rc |= vtss_icfg_printf(result, " %s %s snooping compatibility %s\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) ? "auto" :
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) ? "v1" :
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) ? "v2" : "unknown");
                } else {
                    rc |= vtss_icfg_printf(result, " %s %s snooping compatibility %s\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) ? "auto" :
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_OLD) ? "v1" :
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) ? "v2" :
                                           (ipmc_icli_intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) ? "v3" : "unknown");
                }
                rc |= vtss_icfg_printf(result, " %s %s snooping priority %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_icli_intf_param.priority);
                rc |= vtss_icfg_printf(result, " %s %s snooping robustness-variable %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       q->RobustVari);
                rc |= vtss_icfg_printf(result, " %s %s snooping query-interval %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       q->QueryIntvl);
                rc |= vtss_icfg_printf(result, " %s %s snooping query-max-response-time %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       q->MaxResTime);
                rc |= vtss_icfg_printf(result, " %s %s snooping last-member-query-interval %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       q->LastQryItv);
                rc |= vtss_icfg_printf(result, " %s %s snooping unsolicited-report-interval %u\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                       q->UnsolicitR);
#endif /* VTSS_SW_OPTION_SMB_IPMC */
            } else {
                if (req->all_defaults) {
                    if (version == IPMC_IP_VERSION_IGMP) {
                        rc |= vtss_icfg_printf(result, " no %s %s snooping querier address\n",
                                               ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                               ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
                    }

#if defined(VTSS_SW_OPTION_SMB_IPMC)
                    rc |= vtss_icfg_printf(result, " %s %s snooping compatibility auto\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
                    rc |= vtss_icfg_printf(result, " %s %s snooping priority %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_PRI);
                    rc |= vtss_icfg_printf(result, " %s %s snooping robustness-variable %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_RV);
                    rc |= vtss_icfg_printf(result, " %s %s snooping query-interval %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_QI);
                    rc |= vtss_icfg_printf(result, " %s %s snooping query-max-response-time %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_QRI);
                    rc |= vtss_icfg_printf(result, " %s %s snooping last-member-query-interval %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_LMQI);
                    rc |= vtss_icfg_printf(result, " %s %s snooping unsolicited-report-interval %u\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           IPMC_DEF_INTF_URI);
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                }
            }
        }
    }

    return rc;
}

static vtss_rc _ipmc_snp_icfg_port(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result,
                                   ipmc_ip_version_t version)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        vtss_isid_t                         isid;
        vtss_port_no_t                      iport;
#if defined(VTSS_SW_OPTION_SMB_IPMC)
        int                                 var_val;
        ipmc_conf_port_group_filtering_t    ipmc_icli_port_profile;
        ipmc_conf_throttling_max_no_t       ipmc_icli_port_throttling;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        BOOL                                var_bol;
        ipmc_conf_router_port_t             ipmc_icli_port_mrouter;
        ipmc_conf_fast_leave_port_t         ipmc_icli_port_immediate_leave;

        /*
            SUB-MODE: interface ethernet <port_id>
            COMMAND = ip/ipv6 igmp/mld snooping immediate-leave
            COMMAND = ip/ipv6 igmp/mld snooping mrouter
            --- VTSS_SW_OPTION_SMB_IPMC ---
            COMMAND = ip/ipv6 igmp/mld snooping max-groups <throttling:1-10>
            COMMAND = ip/ipv6 igmp/mld snooping filter <profile_name:word16>
        */
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);

#if defined(VTSS_SW_OPTION_SMB_IPMC)
        memset(&ipmc_icli_port_profile, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
        ipmc_icli_port_profile.port_no = iport;
        if (ipmc_mgmt_get_port_group_filtering(isid, &ipmc_icli_port_profile, version) == VTSS_OK) {
            var_val = strlen(ipmc_icli_port_profile.addr.profile.name);

            if (req->all_defaults || (var_val != IPMC_DEF_FLTR_PROFILE_STRLEN_VALUE)) {
                if (var_val) {
                    rc |= vtss_icfg_printf(result, " %s %s snooping filter %s\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_icli_port_profile.addr.profile.name);
                } else {
                    rc |= vtss_icfg_printf(result, " no %s %s snooping filter\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
                }
            }
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, " no %s %s snooping filter\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        }

        if (ipmc_mgmt_get_throttling_max_count(isid, &ipmc_icli_port_throttling, version) == VTSS_OK) {
            var_val = ipmc_icli_port_throttling.ports[iport];

            if (req->all_defaults || (var_val != IPMC_DEF_THROLLTING_VALUE)) {
                if (var_val) {
                    rc |= vtss_icfg_printf(result, " %s %s snooping max-groups %d\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER),
                                           var_val);
                } else {
                    rc |= vtss_icfg_printf(result, " no %s %s snooping max-groups\n",
                                           ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                           ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
                }
            }
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, " no %s %s snooping max-groups\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        if (ipmc_mgmt_get_static_router_port(isid, &ipmc_icli_port_mrouter, version) == VTSS_OK) {
            var_bol = ipmc_icli_port_mrouter.ports[iport];

            if (req->all_defaults || (var_bol != IPMC_DEF_RTR_PORT_VALUE)) {
                rc |= vtss_icfg_printf(result, " %s%s %s snooping mrouter\n",
                                       var_bol ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, " no %s %s snooping mrouter\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        }

        if (ipmc_mgmt_get_fast_leave_port(isid, &ipmc_icli_port_immediate_leave, version) == VTSS_OK) {
            var_bol = ipmc_icli_port_immediate_leave.ports[iport];

            if (req->all_defaults || (var_bol != IPMC_DEF_IMMEDIATE_LEAVE_VALUE)) {
                rc |= vtss_icfg_printf(result, " %s%s %s snooping immediate-leave\n",
                                       var_bol ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, " no %s %s snooping immediate-leave\n",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        }
    }

    return rc;
}

static vtss_rc _ipmc_snp_icfg_ctrl(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result,
                                   ipmc_ip_version_t version)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        BOOL    state;

        /*
            COMMAND = ip/ipv6 igmp/mld snooping
            COMMAND = ip/ipv6 igmp/mld unknown-flooding
            --- VTSS_SW_OPTION_SMB_IPMC ---
            COMMAND = ip/ipv6 igmp/mld host-proxy [leave-proxy]
        */
#if defined(VTSS_SW_OPTION_SMB_IPMC)
        if (ipmc_mgmt_get_leave_proxy(&state, version) == VTSS_OK) {
            if (req->all_defaults || (state != IPMC_DEF_LEAVE_PROXY_VALUE)) {
                rc |= vtss_icfg_printf(result, "%s%s %s host-proxy leave-proxy\n",
                                       state ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            rc |= vtss_icfg_printf(result, "no %s %s host-proxy leave-proxy\n",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
        }

        if (ipmc_mgmt_get_proxy(&state, version) == VTSS_OK) {
            if (req->all_defaults || (state != IPMC_DEF_PROXY_VALUE)) {
                rc |= vtss_icfg_printf(result, "%s%s %s host-proxy\n",
                                       state ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            rc |= vtss_icfg_printf(result, "no %s %s host-proxy\n",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        if (ipmc_mgmt_get_unreg_flood(&state, version) == VTSS_OK) {
            if (req->all_defaults || (state != IPMC_DEF_UNREG_FLOOD_VALUE)) {
                rc |= vtss_icfg_printf(result, "%s%s %s unknown-flooding\n",
                                       state ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            rc |= vtss_icfg_printf(result, "no %s %s unknown-flooding\n",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
        }

        if (ipmc_mgmt_get_mode(&state, version) == VTSS_OK) {
            if (req->all_defaults || (state != IPMC_DEF_GLOBAL_STATE_VALUE)) {
                rc |= vtss_icfg_printf(result, "%s%s %s snooping\n",
                                       state ? "" : "no ",
                                       ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                       ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
            }
        } else {
            rc |= vtss_icfg_printf(result, "no %s %s snooping\n",
                                   ipmc_lib_ip_txt(version, IPMC_TXT_CASE_LOWER),
                                   ipmc_lib_version_txt(version, IPMC_TXT_CASE_LOWER));
        }
    }

    return rc;
}

/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_igmp_ctrl(const vtss_icfg_query_request_t *req,
                                        vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_ctrl(req, result, IPMC_IP_VERSION_IGMP);
}

#if defined(VTSS_SW_OPTION_SMB_IPMC)
/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_mld_ctrl(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_ctrl(req, result, IPMC_IP_VERSION_MLD);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_igmp_intf(const vtss_icfg_query_request_t *req,
                                        vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_intf(req, result, IPMC_IP_VERSION_IGMP);
}

#if defined(VTSS_SW_OPTION_SMB_IPMC)
/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_mld_intf(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_intf(req, result, IPMC_IP_VERSION_MLD);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_igmp_port(const vtss_icfg_query_request_t *req,
                                        vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_port(req, result, IPMC_IP_VERSION_IGMP);
}

#if defined(VTSS_SW_OPTION_SMB_IPMC)
/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_mld_port(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_port(req, result, IPMC_IP_VERSION_MLD);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_igmp_global(const vtss_icfg_query_request_t *req,
                                          vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_global(req, result, IPMC_IP_VERSION_IGMP);
}

#if defined(VTSS_SW_OPTION_SMB_IPMC)
/* ICFG callback functions */
static vtss_rc _ipmc_snp_icfg_mld_global(const vtss_icfg_query_request_t *req,
                                         vtss_icfg_query_result_t *result)
{
    return _ipmc_snp_icfg_global(req, result, IPMC_IP_VERSION_MLD);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

/*
******************************************************************************

    Public functions

******************************************************************************
*/
/* Initialization function */
vtss_rc ipmc_snp_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module. */
    if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_IGMP, "ip-igmp-snooping", _ipmc_snp_icfg_igmp_global)) {
        if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_IGMP_PORT, "ip-igmp-snooping-port", _ipmc_snp_icfg_igmp_port)) {
            if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_IGMP_INTF, "ip-igmp-snooping-vlan", _ipmc_snp_icfg_igmp_intf)) {
                if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_IGMP_CTRL, "ip-igmp-snooping", _ipmc_snp_icfg_igmp_ctrl)) {
                    T_I("ipmc_snp(IGMP) ICFG done");
                }
            }
        }
    }

#if defined(VTSS_SW_OPTION_SMB_IPMC)
    /* Register callback functions to ICFG module. */
    if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_MLD, "ipv6-mld-snooping", _ipmc_snp_icfg_mld_global)) {
        if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_MLD_PORT, "ipv6-mld-snooping-port", _ipmc_snp_icfg_mld_port)) {
            if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_MLD_INTF, "ipv6-mld-snooping-vlan", _ipmc_snp_icfg_mld_intf)) {
                if (IPMC_SNP_ICFG_REG(rc, VTSS_ICFG_IPMC_SNP_MLD_CTRL, "ipv6-mld-snooping", _ipmc_snp_icfg_mld_ctrl)) {
                    T_I("ipmc_snp(MLD) ICFG done");
                }
            }
        }
    }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

    return rc;
}
