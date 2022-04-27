/*

 Vitesse Switch API software.

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
#include "cli.h"
#include "cli_api.h"

#include "syslog_api.h"
#include "mgmt_api.h"

#include "ipmc.h"
#include "ipmc_api.h"
#include "ipmc_cli.h"
#include "ipmc_lib_cli.h"


#define VTSS_TRACE_MODULE_ID                VTSS_MODULE_ID_IPMC

typedef struct {
    /* Keywords */
    u32                     ip_version;
    ipmc_operation_action_t op;
    i8                      profile_name[VTSS_IPMC_NAME_MAX_LEN];
    u32                     compatibility;
    ipmc_ctrl_pkt_t         pkt_type;
} ipmc_cli_req_t;


void ipmc_cli_req_init(void)
{
    /* register the size required for ipmc req. structure */
    cli_req_size_register(sizeof(ipmc_cli_req_t));
}

static void cli_cmd_ipmc_conf(cli_req_t *req, BOOL mode, BOOL vlan_op,
                              BOOL vlan_state, BOOL vlan_querier,
                              BOOL port_fast_leave, BOOL rports, BOOL flood,
                              BOOL leave_proxy, BOOL proxy, BOOL throttling, BOOL group_filter, BOOL param_pri,
                              BOOL param_rv, BOOL param_qi, BOOL param_qri, BOOL param_llqi, BOOL param_uri,
                              BOOL param_compat, BOOL ssm_range)
{
    switch_iter_t               sit;
    vtss_usid_t                 usid;
    vtss_isid_t                 isid;
    port_iter_t                 pit;
    vtss_uport_no_t             uport;
    vtss_port_no_t              iport;
    BOOL                        state, querier, first, intf_found, next = TRUE;
    ushort                      vid;
    ipmc_conf_router_port_t     ipmc_rports;
    ipmc_dynamic_router_port_t  ipmc_drports;
    char                        buf[80] = {0}, *p;
    ipmc_conf_fast_leave_port_t ipmc_fast_leave_ports;
#ifdef VTSS_SW_OPTION_SMB_IPMC
    BOOL                                set_param;
    ipmc_prefix_t                       ipmc_prefix;
    ipmc_prot_intf_entry_param_t        intf_param;
    ipmc_conf_port_group_filtering_t    ipmc_cli_pg_filtering;
    ipmc_conf_throttling_max_no_t       ipmc_port_throttling;
    char                                ip_buf[40] = {0};
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    ipmc_cli_req_t              *ipmc_req = req->module_req;
    u8                          ver_break = 1, ver_cnt = 1;
    u32                         ver_array[2];

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    ver_array[ver_cnt - 1] = ipmc_req->ip_version;
    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) {
        if (req->set) {
#ifndef VTSS_SW_OPTION_SMB_IPMC
            ver_array[0] = IPMC_IP_VERSION_IGMP;
#else
            /* Currently, configuring ALL IPMC is not available/suitable */
            CPRINTF("Please specify IPMC version: [mld|igmp]\n");
            return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        } else {
            ver_array[0] = IPMC_IP_VERSION_IGMP;
#ifndef VTSS_SW_OPTION_SMB_IPMC
            ver_break = 1;
#else
            ver_break = 2;

            ver_array[1] = IPMC_IP_VERSION_MLD;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        }
    }

    while (ver_cnt <= ver_break) {
        ipmc_req->ip_version = ver_array[ver_cnt - 1];

        if (ver_cnt > 1) {
            CPRINTF("\n");
        }

        ver_cnt++;

        if (mode) {
            if (req->set) {
                state = req->enable;
                (void) ipmc_mgmt_set_mode(&state, ipmc_req->ip_version);
            } else if (ipmc_mgmt_get_mode(&state, ipmc_req->ip_version) == VTSS_OK) {
                CPRINTF("%s Mode: %s\n",
                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"),
                        cli_bool_txt(state));
            }
        }

#ifdef VTSS_SW_OPTION_SMB_IPMC
        if (ssm_range) {
            if (req->set) {
                memset(&ipmc_prefix, 0x0, sizeof(ipmc_prefix_t));
                if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
                    memcpy(&ipmc_prefix.addr.array.prefix, &req->ipv6_addr, sizeof(vtss_ipv6_t));
                } else {
                    ipmc_prefix.addr.value.prefix = req->ipv4_addr;
                }
                ipmc_prefix.len = req->int_values[0];

                (void) ipmc_mgmt_set_ssm_range(ipmc_req->ip_version, &ipmc_prefix);
            } else if (ipmc_mgmt_get_ssm_range(ipmc_req->ip_version, &ipmc_prefix) == VTSS_OK) {
                if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
                    CPRINTF("MLD SSM Range: %s/%u\n",
                            misc_ipv6_txt(&ipmc_prefix.addr.array.prefix, ip_buf),
                            ipmc_prefix.len);
                } else {
                    CPRINTF("IGMP SSM Range: %s/%u\n",
                            misc_ipv4_txt(ipmc_prefix.addr.value.prefix, ip_buf),
                            ipmc_prefix.len);
                }
            }
        }

        if (leave_proxy) {
            if (req->set) {
                state = req->enable;
                (void) ipmc_mgmt_set_leave_proxy(&state, ipmc_req->ip_version);
            } else if (ipmc_mgmt_get_leave_proxy(&state, ipmc_req->ip_version) == VTSS_OK) {
                CPRINTF("%s Leave Proxy: %s\n",
                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"),
                        cli_bool_txt(state));
            }
        }

        if (proxy) {
            if (req->set) {
                state = req->enable;
                (void) ipmc_mgmt_set_proxy(&state, ipmc_req->ip_version);
            } else if (ipmc_mgmt_get_proxy(&state, ipmc_req->ip_version) == VTSS_OK) {
                CPRINTF("%s Proxy: %s\n",
                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"),
                        cli_bool_txt(state));
            }
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        if (flood) {
            if (req->set) {
                state = req->enable;
                (void) ipmc_mgmt_set_unreg_flood(&state, ipmc_req->ip_version);
            } else if (ipmc_mgmt_get_unreg_flood(&state, ipmc_req->ip_version) == VTSS_OK) {
                CPRINTF("%s Flooding Control: %s\n",
                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"),
                        cli_bool_txt(state));
            }
        }

#ifdef VTSS_SW_OPTION_SMB_IPMC
        if (vlan_op || vlan_state || vlan_querier || param_pri ||
            param_rv || param_qi || param_qri || param_llqi || param_uri || param_compat) {
#else
        if (vlan_op || vlan_state || vlan_querier) {
#endif /* VTSS_SW_OPTION_SMB_IPMC */
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                usid = sit.usid;

                if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                    continue;
                }

                first = TRUE;
                if (!req->set) {
                    first = FALSE;
                    cli_cmd_usid_print(usid, req, 1);
                    CPRINTF("%s Interface Setting\n",
                            (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                            ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                    p = &buf[0];
                    p += sprintf(p, "VID   ");
                    if (vlan_state) {
                        p += sprintf(p, "State     ");
                    }
                    if (vlan_querier) {
                        p += sprintf(p, "Querier   ");
                    }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                    if (param_compat) {
                        p += sprintf(p, "Compatibility  ");
                    }
                    if (param_pri) {
                        p += sprintf(p, "PRI  ");
                    }
                    if (param_rv) {
                        p += sprintf(p, "RV   ");
                    }
                    if (param_qi) {
                        p += sprintf(p, "QI     ");
                    }
                    if (param_qri) {
                        p += sprintf(p, "QRI    ");
                    }
                    if (param_llqi) {
                        p += sprintf(p, "LLQI   ");
                    }
                    if (param_uri) {
                        p += sprintf(p, "URI    ");
                    }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                    CPRINTF("\n");
                    cli_table_header(buf);
                } else {
                    if (vlan_op && ipmc_mgmt_intf_op_allow(isid)) {
                        vtss_rc op_rc;

                        vid = req->vid;
                        op_rc = ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &state, &querier, FALSE, ipmc_req->ip_version);
                        if (op_rc == VTSS_OK) {
                            if (ipmc_req->op == IPMC_OP_ADD) {
                                CPRINTF("%s VLAN Interface already exists!\n",
                                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));

                                continue;
                            }

                            if (ipmc_req->op == IPMC_OP_DEL) {
                                if (ipmc_mgmt_set_intf_state_querier(IPMC_OP_DEL, vid, &state, &querier, ipmc_req->ip_version) != VTSS_OK) {
                                    CPRINTF("Erase %s Interface Failed!!!\n",
                                            (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                            ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));

                                    continue;
                                }
                            }
                        } else {
                            if (ipmc_req->op != IPMC_OP_ADD) {
                                CPRINTF("%s VLAN Interface doesn't exist!\n",
                                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));

                                continue;
                            }

                            state = IPMC_DEF_INTF_STATE_VALUE;
                            querier = IPMC_DEF_INTF_QUERIER_VALUE;
                            op_rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_ADD, vid, &state, &querier, ipmc_req->ip_version);
                            if (op_rc != VTSS_OK) {
                                if (op_rc == IPMC_ERROR_TABLE_IS_FULL) {
                                    CPRINTF("%s VLAN Interface Table Full!!!\n",
                                            (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                            ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                                } else {
                                    CPRINTF("Create %s Interface Failed!!!\n",
                                            (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                            ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                                }

                                continue;
                            }
                        }
                    }
                }

#ifdef VTSS_SW_OPTION_SMB_IPMC
                set_param = FALSE;
                if (param_pri || param_rv || param_qi || param_qri || param_llqi || param_uri || param_compat) {
                    set_param = TRUE;
                }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

                vid = 0;
                if (vlan_op) {
                    intf_found = TRUE;
                } else {
                    intf_found = FALSE;
                }
                while (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &state, &querier, next, ipmc_req->ip_version) == VTSS_OK) {
                    if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                        continue;
                    }

                    intf_found = TRUE;

                    if (req->set) {
                        if (vlan_state) {
                            state = req->enable;
                        }
                        if (vlan_querier) {
                            querier = req->enable;
                        }

                        if (vlan_state || vlan_querier) {
                            if (ipmc_mgmt_set_intf_state_querier(IPMC_OP_UPD, vid, &state, &querier, ipmc_req->ip_version) != VTSS_OK) {
                                CPRINTF("Set %s Interface State/Querier Failed!!!",
                                        (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                        ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                            }
                        }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                        if (set_param) {
                            memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                            if (ipmc_mgmt_get_intf_info(isid, vid, &intf_param, ipmc_req->ip_version) == VTSS_OK) {
                                if (param_compat) {
                                    intf_param.cfg_compatibility = ipmc_req->compatibility;
                                }

                                if (param_pri) {
                                    intf_param.priority = req->int_values[0];
                                }
                                if (param_rv) {
                                    intf_param.querier.RobustVari = req->int_values[0];
                                }
                                if (param_qi) {
                                    intf_param.querier.QueryIntvl = req->int_values[0];
                                }
                                if (param_qri) {
                                    intf_param.querier.MaxResTime = req->int_values[0];
                                }
                                if (param_llqi) {
                                    intf_param.querier.LastQryItv = req->int_values[0];
                                }
                                if (param_uri) {
                                    intf_param.querier.UnsolicitR = req->int_values[0];
                                }

                                if (intf_param.querier.RobustVari == 1) {
                                    CPRINTF("Robustness Variable SHOULD NOT be one.\n");
                                }
                                if (intf_param.querier.MaxResTime >= (intf_param.querier.QueryIntvl * 10)) {
                                    CPRINTF("The number of seconds represented by QRI must be less than QI!\n");
                                    CPRINTF("Please assign a proper QI value (Greater than %u).\n", (intf_param.querier.MaxResTime / 10));

                                    return;
                                }
                                if (ipmc_mgmt_set_intf_info(isid, &intf_param, ipmc_req->ip_version) != VTSS_OK) {
                                    T_W("Set IPMC Interface Parameter Failed!!!");
                                }
                            }
                        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                    } else {
                        if (first) {
                            first = FALSE;
                            cli_cmd_usid_print(usid, req, 1);
                            CPRINTF("%s Interface Setting\n",
                                    (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                    ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                            p = &buf[0];
                            p += sprintf(p, "VID   ");
                            if (vlan_state) {
                                p += sprintf(p, "State     ");
                            }
                            if (vlan_querier) {
                                p += sprintf(p, "Querier   ");
                            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                            if (param_compat) {
                                p += sprintf(p, "Compatibility  ");
                            }
                            if (param_pri) {
                                p += sprintf(p, "PRI  ");
                            }
                            if (param_rv) {
                                p += sprintf(p, "RV   ");
                            }
                            if (param_qi) {
                                p += sprintf(p, "QI     ");
                            }
                            if (param_qri) {
                                p += sprintf(p, "QRI    ");
                            }
                            if (param_llqi) {
                                p += sprintf(p, "LLQI   ");
                            }
                            if (param_uri) {
                                p += sprintf(p, "URI    ");
                            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                            CPRINTF("\n");
                            cli_table_header(buf);
                        }

                        CPRINTF("%-4d  ", vid);
                        if (vlan_state) {
                            CPRINTF("%s  ", cli_bool_txt(state));
                        }
                        if (vlan_querier) {
                            CPRINTF("%s  ", cli_bool_txt(querier));
                        }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                        memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                        if (ipmc_mgmt_get_intf_info(isid, vid, &intf_param, ipmc_req->ip_version) == VTSS_OK) {
                            if (param_compat) {
                                if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                    CPRINTF("%-13s  ",
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) ? "IGMP-Auto" :
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_OLD) ? "Forced IGMPv1" :
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) ? "Forced IGMPv2" :
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) ? "Forced IGMPv3" : "IGMP-N/A");
                                } else if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
                                    CPRINTF("%-13s  ",
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_AUTO) ? "MLD-Auto" :
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) ? "Forced MLDv1" :
                                            (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) ? "Forced MLDv2" : "MLD-N/A");
                                } else {
                                    CPRINTF("%-13s  ", "IPMC-N/A");
                                }
                            }
                            if (param_pri) {
                                CPRINTF("%-3u  ", intf_param.priority);
                            }
                            if (param_rv) {
                                CPRINTF("%-3u  ", intf_param.querier.RobustVari);
                            }
                            if (param_qi) {
                                CPRINTF("%-5u  ", intf_param.querier.QueryIntvl);
                            }
                            if (param_qri) {
                                CPRINTF("%-5u  ", intf_param.querier.MaxResTime);
                            }
                            if (param_llqi) {
                                CPRINTF("%-5u  ", intf_param.querier.LastQryItv);
                            }
                            if (param_uri) {
                                CPRINTF("%-5u  ", intf_param.querier.UnsolicitR);
                            }
                        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

                        CPRINTF("\n");
                    }

                    /* Found */
                    if (req->vid_spec == CLI_SPEC_VAL) {
                        break;
                    }
                } /* while (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &state, &querier, next, ipmc_req->ip_version) == VTSS_OK) */

                if (!intf_found) {
                    CPRINTF("(Please create %s Interfaces)\n",
                            (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                            ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                }
            } /* for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) */
        }

#ifdef VTSS_SW_OPTION_SMB_IPMC
        if (rports || port_fast_leave || throttling) {
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                usid = sit.usid;

                if ((isid = req->stack.isid[usid]) == VTSS_ISID_END ||
                    ipmc_mgmt_get_static_router_port(isid, &ipmc_rports, ipmc_req->ip_version) != VTSS_OK ||
                    ipmc_mgmt_get_dynamic_router_ports(isid, &ipmc_drports, ipmc_req->ip_version) != VTSS_OK ||
                    ipmc_mgmt_get_fast_leave_port(isid, &ipmc_fast_leave_ports, ipmc_req->ip_version) != VTSS_OK ||
                    ipmc_mgmt_get_throttling_max_count(isid, &ipmc_port_throttling, ipmc_req->ip_version) != VTSS_OK) {
                    continue;
                }
#else
        if (rports || port_fast_leave) {
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                usid = sit.usid;

                if ((isid = req->stack.isid[usid]) == VTSS_ISID_END ||
                    ipmc_mgmt_get_static_router_port(isid, &ipmc_rports, ipmc_req->ip_version) != VTSS_OK ||
                    ipmc_mgmt_get_dynamic_router_ports(isid, &ipmc_drports, ipmc_req->ip_version) != VTSS_OK ||
                    ipmc_mgmt_get_fast_leave_port(isid, &ipmc_fast_leave_ports, ipmc_req->ip_version) != VTSS_OK) {
                    continue;
                }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                first = TRUE;
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    uport = iport2uport(iport);
                    if (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                        continue;
                    }
                    if (req->set) {
                        if (rports) {
                            ipmc_rports.ports[iport] = req->enable;
                        }
                        if (port_fast_leave) {
                            ipmc_fast_leave_ports.ports[iport] = req->enable;
                        }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                        if (throttling) {
                            ipmc_port_throttling.ports[iport] = req->value;
                        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                    } else {
                        if (first) {
                            first = 0;
                            cli_cmd_usid_print(usid, req, 1);
                            CPRINTF("%s Port Status (",
                                    (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                    ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                            if (rports) {
                                CPRINTF(" Router-Port ");
                            }
                            if (port_fast_leave) {
                                CPRINTF(" Fast-Leave ");
                            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                            if (throttling) {
                                CPRINTF(" Throttling ");
                            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                            CPRINTF(")\n\n");

                            p = &buf[0];
                            p += sprintf(p, "Port  ");
                            if (rports) {
                                p += sprintf(p, "Router    ");
                                p += sprintf(p, "Dynamic Router  ");
                            }
                            if (port_fast_leave) {
                                p += sprintf(p, "Fast Leave  ");
                            }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                            if (throttling) {
                                p += sprintf(p, "Group Throttling Number");
                            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                            cli_table_header(buf);
                        }

                        CPRINTF("%-2u    ", uport);
                        if (rports) {
                            CPRINTF("%-8s  ", cli_bool_txt(ipmc_rports.ports[iport]));
                            if (ipmc_drports.ports[iport] == 1) {
                                CPRINTF("%s             ", "Yes");
                            } else {
                                CPRINTF("%s              ", "No");
                            }
                        }
                        if (port_fast_leave) {
                            CPRINTF("%s    ", cli_bool_txt(ipmc_fast_leave_ports.ports[iport]));
                        }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                        if (throttling) {
                            if (ipmc_port_throttling.ports[iport] == 0) {
                                CPRINTF("Unlimited  ");
                            } else {
                                CPRINTF("%d  ", ipmc_port_throttling.ports[iport]);
                            }
                        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                        CPRINTF("\n");
                    }
                }
                if (req->set) {
                    if (rports) {
                        (void) ipmc_mgmt_set_router_port(isid, &ipmc_rports, ipmc_req->ip_version);
                    }
                    if (port_fast_leave) {
                        (void) ipmc_mgmt_set_fast_leave_port(isid, &ipmc_fast_leave_ports, ipmc_req->ip_version);
                    }
#ifdef VTSS_SW_OPTION_SMB_IPMC
                    if (throttling) {
                        (void) ipmc_mgmt_set_throttling_max_count(isid, &ipmc_port_throttling, ipmc_req->ip_version);
                    }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
                }
            }
        }

#ifdef VTSS_SW_OPTION_SMB_IPMC
        if (group_filter) {
            int                         str_len = strlen(ipmc_req->profile_name);
            ipmc_lib_profile_mem_t      *pfm;
            ipmc_lib_grp_fltr_profile_t *fltr_profile;

            if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
                CPRINTF("Invalid memory operation.\n");
                return;
            }

            fltr_profile = &pfm->profile;
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                usid = sit.usid;

                if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                    continue;
                }
                first = 1;
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    uport = iport2uport(iport);
                    if (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                        continue;
                    }

                    memset(&ipmc_cli_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
                    if (req->set) {
                        if (str_len) {
                            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                            strncpy(fltr_profile->data.name, ipmc_req->profile_name, str_len);
                            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
                                CPRINTF("Please specify correct filter profile name.\n");
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return;
                            }
                        } else {
                            CPRINTF("Please specify correct filter profile name.\n");
                            IPMC_MEM_PROFILE_MGIVE(pfm);
                            return;
                        }

                        ipmc_cli_pg_filtering.port_no = iport;
                        strncpy(ipmc_cli_pg_filtering.addr.profile.name, ipmc_req->profile_name, str_len);
                        if (ipmc_req->op != IPMC_OP_DEL) {
                            (void) ipmc_mgmt_set_port_group_filtering(isid, &ipmc_cli_pg_filtering, ipmc_req->ip_version);
                        } else {
                            (void) ipmc_mgmt_del_port_group_filtering(isid, &ipmc_cli_pg_filtering, ipmc_req->ip_version);
                        }
                    } else {
                        if (first) {
                            first = 0;
                            cli_cmd_usid_print(usid, req, 1);
                            CPRINTF("%s Filtering Profile Setting\n\n",
                                    (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ? "IGMP" :
                                    ((ipmc_req->ip_version == IPMC_IP_VERSION_MLD) ? "MLD" : "N/A"));
                            p = &buf[0];
                            p += sprintf(p, "Port  ");
                            p += sprintf(p, "Filtering Profile");
                            cli_table_header(buf);
                        }

                        CPRINTF("%-2u    ", uport);
                        ipmc_cli_pg_filtering.port_no = iport;
                        if (ipmc_mgmt_get_port_group_filtering(isid, &ipmc_cli_pg_filtering, ipmc_req->ip_version) == VTSS_OK) {
                            str_len = strlen(ipmc_cli_pg_filtering.addr.profile.name);
                            if (str_len) {
                                CPRINTF("%s\n", ipmc_cli_pg_filtering.addr.profile.name);
                            } else {
                                CPRINTF("No Profile\n");
                            }
                        } else {
                            CPRINTF("No Profile\n");
                        }
                    }
                }
            }

            IPMC_MEM_PROFILE_MGIVE(pfm);
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    } /* while (ver_cnt <= ver_break) */
}

static void cli_cmd_ipmc_gen_config(cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (!req->set) {
        if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
            cli_header("IGMP Configuration", 1);
        } else if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
            cli_header("MLD Configuration", 1);
        } else {
            cli_header("IPMC Configuration", 1);
        }
    } else {
        if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) { /* Currently, configuring ALL IPMC is not available/suitable */
#ifndef VTSS_SW_OPTION_SMB_IPMC
            ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
            CPRINTF("Please specify IPMC version: [mld|igmp]\n");
            return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        }
    }

    cli_cmd_ipmc_conf(req, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
}

static void cli_cmd_ipmc_mode(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_vlan_op(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_vlan_op_add(cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;

    ipmc_req->op = IPMC_OP_ADD;
    cli_cmd_ipmc_vlan_op(req);
}

static void cli_cmd_ipmc_vlan_op_del(cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;

    ipmc_req->op = IPMC_OP_DEL;
    cli_cmd_ipmc_vlan_op(req);
}

static void cli_cmd_ipmc_vlan_state(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_vlan_querier(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_port_fastleave(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_rtr_ports(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_unreg_flood(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static void cli_cmd_ipmc_leave_proxy(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_proxy(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_port_throttle(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_group_filtering(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_param_pri(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_param_rv(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_param_qi(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_param_qri(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_ipmc_param_llqi(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_ipmc_param_uri(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_ipmc_param_compat(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
}

static void cli_cmd_ipmc_ssm_range(cli_req_t *req)
{
    cli_cmd_ipmc_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

#include "netdb.h"
static void cli_cmd_show_ipmc_groups(cli_req_t *req )
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    uchar                               dummy;
    ushort                              vid;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    BOOL                                iports[VTSS_PORT_ARRAY_SIZE], first;
    int                                 i, portCnt;
    char                                v6adrString[40] = {0};
    char                                plistString[MGMT_PORT_BUF_SIZE] = {0};
    ipmc_cli_req_t                      *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) { /* Currently, configuring ALL IPMC is not available/suitable */
#ifndef VTSS_SW_OPTION_SMB_IPMC
        ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
        CPRINTF("Please specify IPMC version: [mld|igmp]\n");
        return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }


        first = 1;
        vid = 0;
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, ipmc_req->ip_version) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                continue;
            }
            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (ipmc_mgmt_get_next_intf_group_info(isid, vid, &intf_group_entry, ipmc_req->ip_version) == VTSS_OK) {
                if (first) {
                    first = 0;
                    cli_cmd_usid_print(usid, req, 0);
                    cli_table_header("VID   Group                                     Ports");
                }

                memset(iports, 0x0, sizeof(iports));
                portCnt = port_isid_port_count(isid);
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;

                    if (i < portCnt) {
                        iports[i] = VTSS_PORT_BF_GET(intf_group_entry.db.port_mask, i);
                    } else {
                        break;
                    }
                }

                if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                    ulong   ip4addr;

                    memcpy((uchar *)&ip4addr, &intf_group_entry.group_addr.addr[12], sizeof(ulong));
                    CPRINTF("%-4d  %-40s  %s\n",
                            vid,
                            misc_ipv4_txt(htonl(ip4addr), v6adrString),
                            cli_iport_list_txt(iports, plistString));
                } else {
                    CPRINTF("%-4d  %-40s  %s\n",
                            vid,
                            misc_ipv6_txt(&intf_group_entry.group_addr, v6adrString),
                            cli_iport_list_txt(iports, plistString));
                }

                if (req->vid_spec == CLI_SPEC_VAL) {
                    break;
                }
            }
        }
    }
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static void cli_cmd_show_ipmc_sfm_info(cli_req_t *req)
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    uchar                               dummy;
    ushort                              vid;
    vtss_uport_no_t                     uport;
    vtss_port_no_t                      iport;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    BOOL                                first, deny_found, sfm_print, amir, smir;
    ulong                               ip4grp, ip4src;
    char                                buf[40] = {0};
    char                                buf1[16] = {0};
    char                                buf2[40] = {0};
    ipmc_sfm_srclist_t                  sfm_src;
    ipmc_cli_req_t                      *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) { /* Currently, configuring ALL IPMC is not available/suitable */
#ifndef VTSS_SW_OPTION_SMB_IPMC
        ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
        CPRINTF("Please specify IPMC version: [mld|igmp]\n");
        return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = TRUE;
        vid = 0;
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, ipmc_req->ip_version) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                continue;
            }

            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (ipmc_mgmt_get_next_intf_group_info(isid, vid, &intf_group_entry, ipmc_req->ip_version) == VTSS_OK ) {
                if (first) {
                    first = FALSE;
                    cli_cmd_usid_print(usid, req, 0);
                    cli_table_header("VID   Group                                     Mode     Ports");
                }

                amir = intf_group_entry.db.asm_in_hw;
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    uport = iport2uport(iport);
                    if (req->uport_list[uport] == 0) {
                        continue;
                    }

                    if (IPMC_LIB_GRP_PORT_DO_SFM(&intf_group_entry.db, iport)) {
                        if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group_entry.db, iport)) {
                            sprintf(buf1, "Exclude");
                        } else {
                            sprintf(buf1, "Include");
                        }

                        deny_found = sfm_print = FALSE;
                        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                        group_srclist_entry.type = TRUE;
                        while (ipmc_mgmt_get_next_group_srclist_info(
                                   isid, ipmc_req->ip_version, vid,
                                   &intf_group_entry.group_addr,
                                   &group_srclist_entry) == VTSS_OK) {
                            if (!group_srclist_entry.cntr) {
                                break;
                            }

                            memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                            if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                                continue;
                            }

                            smir = sfm_src.sfm_in_hw;
                            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(ulong));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            vid,
                                            misc_ipv4_txt(htonl(ip4grp), buf),
                                            buf1,
                                            iport + 1);
                                }

                                CPRINTF("Allow Source Address: %-40s%s\n",
                                        misc_ipv4_txt(htonl(ip4src), buf2),
                                        (amir && smir) ? " (Hardware Filter)" : "");
                            } else {
                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            vid,
                                            misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                            buf1,
                                            iport + 1);
                                }

                                CPRINTF("Allow Source Address: %-40s%s\n",
                                        misc_ipv6_txt(&sfm_src.src_ip, buf2),
                                        (amir && smir) ? " (Hardware Filter)" : "");
                            }
                        }

                        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                        group_srclist_entry.type = FALSE;
                        while (ipmc_mgmt_get_next_group_srclist_info(
                                   isid, ipmc_req->ip_version, vid,
                                   &intf_group_entry.group_addr,
                                   &group_srclist_entry) == VTSS_OK) {
                            if (!group_srclist_entry.cntr) {
                                break;
                            }

                            memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                            if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                                continue;
                            }

                            deny_found = TRUE;

                            smir = sfm_src.sfm_in_hw;
                            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(ulong));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            vid,
                                            misc_ipv4_txt(htonl(ip4grp), buf),
                                            buf1,
                                            iport + 1);
                                }
                                CPRINTF("Deny Source Address : %-40s%s\n",
                                        misc_ipv4_txt(htonl(ip4src), buf2),
                                        (amir && smir) ? " (Hardware Filter)" : "");
                            } else {
                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            vid,
                                            misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                            buf1,
                                            iport + 1);
                                }
                                CPRINTF("Deny Source Address : %-40s%s\n",
                                        misc_ipv6_txt(&sfm_src.src_ip, buf2),
                                        (amir && smir) ? " (Hardware Filter)" : "");
                            }
                        }

                        if (!deny_found) {
                            if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group_entry.db, iport)) {
                                if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                    memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));

                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                                vid,
                                                misc_ipv4_txt(htonl(ip4grp), buf),
                                                buf1,
                                                iport + 1);
                                    }
                                } else {
                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                                vid,
                                                misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                                buf1,
                                                iport + 1);
                                    }
                                }
                                CPRINTF("Deny Source Address: None%s\n",
                                        amir ? " (Hardware Switch)" : "");
                            }
                        }
                    }
                }

            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
        }
    }
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static void _cli_ipmc_dump_interface_info(ipmc_intf_map_t *intf)
{
    port_iter_t                     pit;
    vtss_port_no_t                  iport;
    ipmc_prot_intf_entry_param_t    *param;
    ipmc_querier_sm_t               *querier;
    BOOL                            intf_port[VTSS_PORT_ARRAY_SIZE];
    i8                              buf[MGMT_PORT_BUF_SIZE];

    if (!intf) {
        return;
    }

    param = &intf->param;
    querier = &param->querier;
    memset(intf_port, 0x0, sizeof(intf_port));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        if (VTSS_PORT_BF_GET(intf->vlan_ports, iport)) {
            intf_port[iport] = TRUE;
        }
    }

    memset(buf, 0x0, sizeof(buf));
    CPRINTF("%s(VID-%u) OP=%s/Q-St:%s[%d] VLAN-Port=%s\n",
            ipmc_lib_version_txt(intf->ipmc_version, IPMC_TXT_CASE_UPPER),
            param->vid,
            intf->op_state ? "TRUE" : "FALSE",
            querier->querier_enabled ? "TRUE" : "FALSE",
            querier->state,
            mgmt_iport_list2txt(intf_port, buf));
    memset(buf, 0x0, sizeof(buf));
    CPRINTF("Config Querier Address: %s\n",
            (intf->ipmc_version == IPMC_IP_VERSION_IGMP) ?
            misc_ipv4_txt(intf->QuerierConf.igmp.adrs, buf) :
            misc_ipv6_txt(&intf->QuerierConf.mld.adrs, buf));
    memset(buf, 0x0, sizeof(buf));
    CPRINTF("Active Querier Address: %s\n",
            (intf->ipmc_version == IPMC_IP_VERSION_IGMP) ?
            misc_ipv4_txt(intf->QuerierAdrs.igmp.adrs, buf) :
            misc_ipv6_txt(&intf->QuerierAdrs.mld.adrs, buf));
    CPRINTF("PRI-%u|RV-%u|QI-%u|QRI-%u|SUI-%u|SUC-%u|LQI-%u|LQC-%u|URI-%u\n",
            param->priority,
            querier->RobustVari, querier->QueryIntvl,
            querier->MaxResTime, querier->StartUpItv,
            querier->StartUpCnt, querier->LastQryItv,
            querier->LastQryCnt, querier->UnsolicitR);
    CPRINTF("COMPAT(CFG:%d)*[RTR->%d/OTmr:%u|GTmr:%u|STmr:%u]*[HST->%d/OTmr:%u|GTmr:%u|STmr:%u]\n",
            param->cfg_compatibility,
            param->rtr_compatibility.mode,
            param->rtr_compatibility.old_present_timer,
            param->rtr_compatibility.gen_present_timer,
            param->rtr_compatibility.sfm_present_timer,
            param->hst_compatibility.mode,
            param->hst_compatibility.old_present_timer,
            param->hst_compatibility.gen_present_timer,
            param->hst_compatibility.sfm_present_timer);
}

static void cli_cmd_debug_ipmc_pkt_reset(cli_req_t *req)
{
#if 1 /* etliang */
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    ipmc_lib_packet_max_time_reset();
#else
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    vtss_rc     rc;
    int         exe_count, max_idx, min_idx;
    BOOL        fwd_map[VTSS_PORT_ARRAY_SIZE];
    ipmc_time_t exe_time_base, exe_time_diff;
    ipmc_time_t exe_time_min, exe_time_max;
    ipmc_time_t exe_time_total_s, exe_time_total_e;
    ipmc_time_t exe_time_critical;
    vtss_ipv4_t test_dip, test_sip;
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    ipmc_lib_packet_max_time_reset();

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    min_idx = max_idx = -1;
    memset(&exe_time_max, 0x0, sizeof(ipmc_time_t));
    exe_time_min.sec = 3600;
    memset(&exe_time_critical, 0x0, sizeof(ipmc_time_t));
    (void) ipmc_lib_time_curr_get(&exe_time_base);
    ipmc_lib_time_cpy(&exe_time_total_s, &exe_time_base);
    for (exe_count = 0; exe_count < 1000; exe_count++) {
        test_dip = 0xE1000001 + exe_count;
        fwd_map[exe_count % 24] = exe_count % 2;
        test_sip = 0;

        if ((rc = vtss_ipv4_mc_add(NULL, 1, test_sip, test_dip, fwd_map)) != VTSS_OK) {
            CPRINTF("\n\rFailure in vtss_ipv4_mc_add");
        }

        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &exe_time_diff);
        (void) ipmc_lib_time_curr_get(&exe_time_base);
        if ((exe_count + 1) == 1000) {
            ipmc_lib_time_cpy(&exe_time_critical, &exe_time_diff);
        }

        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_max) == IPMC_LIB_TIME_CMP_GREATER) {
            ipmc_lib_time_cpy(&exe_time_max, &exe_time_diff);
            max_idx = exe_count + 1;
        }
        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_min) == IPMC_LIB_TIME_CMP_LESS) {
            ipmc_lib_time_cpy(&exe_time_min, &exe_time_diff);
            min_idx = exe_count + 1;
        }
    }

    (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_total_s, &exe_time_total_e);
    CPRINTF("\n\rMaximun(#%d) ASM4-API-ADD Time Consume: %lu%um%uu\n",
            max_idx, exe_time_max.sec, exe_time_max.msec, exe_time_max.usec);
    CPRINTF("Minimun(#%d) ASM4-API-ADD Time Consume: %lu.%um%uu\n",
            min_idx, exe_time_min.sec, exe_time_min.msec, exe_time_min.usec);
    CPRINTF("Total Time Consume in 1K ASM4-API-ADD: %lu.%um%uu\n",
            exe_time_total_e.sec, exe_time_total_e.msec, exe_time_total_e.usec);
    CPRINTF("Critical(#1K) Time Consume in 1K ASM4-API-ADD: %lu.%um%uu\n",
            exe_time_critical.sec, exe_time_critical.msec, exe_time_critical.usec);

    min_idx = max_idx = -1;
    memset(&exe_time_max, 0x0, sizeof(ipmc_time_t));
    exe_time_min.sec = 3600;
    memset(&exe_time_critical, 0x0, sizeof(ipmc_time_t));
    (void) ipmc_lib_time_curr_get(&exe_time_base);
    ipmc_lib_time_cpy(&exe_time_total_s, &exe_time_base);
    for (exe_count = 0; exe_count < 1000; exe_count++) {
        test_dip = 0xE1000001 + exe_count;
        fwd_map[exe_count % 24] = exe_count % 2;
        if (exe_count % 2) {
            test_sip = 0xA123456;
        } else {
            test_sip = 0xA654321;
        }

        if ((rc = vtss_ipv4_mc_add(NULL, 1, test_sip, test_dip, fwd_map)) != VTSS_OK) {
            CPRINTF("\n\rFailure in vtss_ipv4_mc_add");
        }

        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &exe_time_diff);
        (void) ipmc_lib_time_curr_get(&exe_time_base);
        if ((exe_count + 1) == 1000) {
            ipmc_lib_time_cpy(&exe_time_critical, &exe_time_diff);
        }

        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_max) == IPMC_LIB_TIME_CMP_GREATER) {
            ipmc_lib_time_cpy(&exe_time_max, &exe_time_diff);
            max_idx = exe_count + 1;
        }
        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_min) == IPMC_LIB_TIME_CMP_LESS) {
            ipmc_lib_time_cpy(&exe_time_min, &exe_time_diff);
            min_idx = exe_count + 1;
        }
    }

    (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_total_s, &exe_time_total_e);
    CPRINTF("\n\rMaximun(#%d) SFM4-API-ADD Time Consume: %lu.%um%uu\n",
            max_idx, exe_time_max.sec, exe_time_max.msec, exe_time_max.usec);
    CPRINTF("Minimun(#%d) SFM4-API-ADD Time Consume: %lu.%um%uu\n",
            min_idx, exe_time_min.sec, exe_time_min.msec, exe_time_min.usec);
    CPRINTF("Total Time Consume in 1K SFM4-API-ADD: %lu.%um%uu\n",
            exe_time_total_e.sec, exe_time_total_e.msec, exe_time_total_e.usec);
    CPRINTF("Critical(#1K) Time Consume in 1K SFM4-API-ADD: %lu.%um%uu\n",
            exe_time_critical.sec, exe_time_critical.msec, exe_time_critical.usec);

    min_idx = max_idx = -1;
    memset(&exe_time_max, 0x0, sizeof(ipmc_time_t));
    exe_time_min.sec = 3600;
    memset(&exe_time_critical, 0x0, sizeof(ipmc_time_t));
    (void) ipmc_lib_time_curr_get(&exe_time_base);
    ipmc_lib_time_cpy(&exe_time_total_s, &exe_time_base);
    for (exe_count = 1000; exe_count > 0; exe_count--) {
        test_dip = 0xE1000001 + (exe_count - 1);
        if ((exe_count - 1) % 2) {
            test_sip = 0xA123456;
        } else {
            test_sip = 0xA654321;
        }

        if ((rc = vtss_ipv4_mc_del(NULL, 1, test_sip, test_dip)) != VTSS_OK) {
            CPRINTF("\n\rFailure in vtss_ipv4_mc_del");
        }

        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &exe_time_diff);
        (void) ipmc_lib_time_curr_get(&exe_time_base);
        if (exe_count == 1000) {
            ipmc_lib_time_cpy(&exe_time_critical, &exe_time_diff);
        }

        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_max) == IPMC_LIB_TIME_CMP_GREATER) {
            ipmc_lib_time_cpy(&exe_time_max, &exe_time_diff);
            max_idx = exe_count;
        }
        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_min) == IPMC_LIB_TIME_CMP_LESS) {
            ipmc_lib_time_cpy(&exe_time_min, &exe_time_diff);
            min_idx = exe_count + 1;
        }
    }

    (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_total_s, &exe_time_total_e);
    CPRINTF("\n\rMaximun(#%d) SFM4-API-DEL Time Consume: %lu.%um%uu\n",
            max_idx, exe_time_max.sec, exe_time_max.msec, exe_time_max.usec);
    CPRINTF("Minimun(#%d) SFM4-API-DEL Time Consume: %lu.%um%uu\n",
            min_idx, exe_time_min.sec, exe_time_min.msec, exe_time_min.usec);
    CPRINTF("Total Time Consume in 1K SFM4-API-DEL: %lu.%um%uu\n",
            exe_time_total_e.sec, exe_time_total_e.msec, exe_time_total_e.usec);
    CPRINTF("Critical(#1K) Time Consume in 1K SFM4-API-DEL: %lu.%um%uu\n",
            exe_time_critical.sec, exe_time_critical.msec, exe_time_critical.usec);

    min_idx = max_idx = -1;
    memset(&exe_time_max, 0x0, sizeof(ipmc_time_t));
    exe_time_min.sec = 3600;
    memset(&exe_time_critical, 0x0, sizeof(ipmc_time_t));
    (void) ipmc_lib_time_curr_get(&exe_time_base);
    ipmc_lib_time_cpy(&exe_time_total_s, &exe_time_base);
    for (exe_count = 1000; exe_count > 0; exe_count--) {
        test_dip = 0xE1000001 + (exe_count - 1);
        test_sip = 0;

        if ((rc = vtss_ipv4_mc_del(NULL, 1, test_sip, test_dip)) != VTSS_OK) {
            CPRINTF("\n\rFailure in vtss_ipv4_mc_del");
        }

        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &exe_time_diff);
        (void) ipmc_lib_time_curr_get(&exe_time_base);
        if (exe_count == 1000) {
            ipmc_lib_time_cpy(&exe_time_critical, &exe_time_diff);
        }

        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_max) == IPMC_LIB_TIME_CMP_GREATER) {
            ipmc_lib_time_cpy(&exe_time_max, &exe_time_diff);
            max_idx = exe_count;
        }
        if (ipmc_lib_time_cmp(&exe_time_diff, &exe_time_min) == IPMC_LIB_TIME_CMP_LESS) {
            ipmc_lib_time_cpy(&exe_time_min, &exe_time_diff);
            min_idx = exe_count + 1;
        }
    }

    (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_total_s, &exe_time_total_e);
    CPRINTF("\n\rMaximun(#%d) ASM4-API-DEL Time Consume: %lu.%um%uu\n",
            max_idx, exe_time_max.sec, exe_time_max.msec, exe_time_max.usec);
    CPRINTF("Minimun(#%d) ASM4-API-DEL Time Consume: %lu.%um%uu\n",
            min_idx, exe_time_min.sec, exe_time_min.msec, exe_time_min.usec);
    CPRINTF("Total Time Consume in 1K ASM4-API-DEL: %lu.%um%uu\n",
            exe_time_total_e.sec, exe_time_total_e.msec, exe_time_total_e.usec);
    CPRINTF("Critical(#1K) Time Consume in 1K ASM4-API-DEL: %lu.%um%uu\n",
            exe_time_critical.sec, exe_time_critical.msec, exe_time_critical.usec);
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */
#endif /* etliang */
}

static void cli_cmd_debug_ipmc_statistics(cli_req_t *req)
{
    ipmc_mem_info_t ipmc_mem_info;
    ipmc_time_t     ipmc_pkt_time;
    ipmc_intf_map_t ipmc_intf;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    CPRINTF("\nIPMC Interface Table --->\n");
    memset(&ipmc_intf, 0x0, sizeof(ipmc_intf_map_t));
    ipmc_intf.ipmc_version = IPMC_IP_VERSION_IGMP;
    while (ipmc_mgmt_intf_walk(TRUE, &ipmc_intf) == VTSS_OK) {
        _cli_ipmc_dump_interface_info(&ipmc_intf);
    }
    memset(&ipmc_intf, 0x0, sizeof(ipmc_intf_map_t));
    ipmc_intf.ipmc_version = IPMC_IP_VERSION_MLD;
    while (ipmc_mgmt_intf_walk(TRUE, &ipmc_intf) == VTSS_OK) {
        _cli_ipmc_dump_interface_info(&ipmc_intf);
    }
    CPRINTF("<--- IPMC Interface Table\n\n");

    memset(&ipmc_mem_info, 0x0, sizeof(ipmc_mem_info_t));
    if (ipmc_debug_mem_get_info(&ipmc_mem_info)) {
        ipmc_mem_t          idx;
        ipmc_memory_info_t  *info_ptr;

        CPRINTF("\n\rCurrent ipmc_mem_cnt: %d (Summary Listed Below)\n", ipmc_mem_info.mem_current_cnt);
        for (idx = 0; idx < IPMC_MEM_TYPE_MAX; idx++) {
            info_ptr = &ipmc_mem_info.mem_pool_info[idx];
            CPRINTF("%s->\nTotal: %d/Free: %d/Size: %d/BlockSZ: %d/MaxFree: %d\n",
                    ipmc_lib_mem_id_txt(idx),
                    info_ptr->totalmem,
                    info_ptr->freemem,
                    info_ptr->size,
                    info_ptr->blocksize,
                    info_ptr->maxfree);
        }
    }

    if (ipmc_lib_packet_max_time_get(&ipmc_pkt_time)) {
        CPRINTF("\n\rCurrent IPMC PKT-Dispatch consumes MAX:%u.%um%uu\n",
                ipmc_pkt_time.sec, ipmc_pkt_time.msec, ipmc_pkt_time.usec);
    }
}

static void cli_cmd_debug_ipmc_sfm_info(cli_req_t *req)
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    uchar                               dummy;
    ushort                              vid;
    vtss_uport_no_t                     uport;
    vtss_port_no_t                      iport;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    BOOL                                first, deny_found, sfm_print, amir, smir;
    int                                 fto;
    ulong                               ip4grp, ip4src, grpCnt;
    char                                buf[40] = {0};
    char                                buf1[16] = {0};
    char                                buf2[40] = {0};
    ipmc_sfm_srclist_t                  sfm_src;
    ipmc_cli_req_t                      *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) { /* Currently, configuring ALL IPMC is not available/suitable */
#ifndef VTSS_SW_OPTION_SMB_IPMC
        ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
        CPRINTF("Please specify IPMC version [mld|igmp] for more information\n");
        return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    }

    CPRINTF("\nIPMC SFM Group Table\n");

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = TRUE;
        vid = 0;
        grpCnt = 0;
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, ipmc_req->ip_version) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                continue;
            }

            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (ipmc_mgmt_get_next_intf_group_info(isid, vid, &intf_group_entry, ipmc_req->ip_version) == VTSS_OK ) {
                if (first) {
                    first = FALSE;
                    cli_cmd_usid_print(usid, req, 0);
                    cli_table_header("VID   Group                                     Mode     G/F_Timer  InHW  Ports");
                }

                amir = intf_group_entry.db.asm_in_hw;
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    uport = iport2uport(iport);
                    if (req->uport_list[uport] == 0) {
                        continue;
                    }

                    if (IPMC_LIB_GRP_PORT_DO_SFM(&intf_group_entry.db, iport)) {
                        char    buf4[16] = {0};

                        if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group_entry.db, iport)) {
                            sprintf(buf1, "Exclude");
                            fto = ipmc_lib_mgmt_calc_delta_time(isid, &intf_group_entry.db.tmr.delta_time.v[iport]);
                        } else {
                            sprintf(buf1, "Include");
                            fto = -1;
                        }

                        (fto >= 0) ? sprintf(buf4, "%d", fto) : sprintf(buf4, "Dont Care");
                        deny_found = sfm_print = FALSE;
                        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                        group_srclist_entry.type = TRUE;
                        while (ipmc_mgmt_get_next_group_srclist_info(
                                   isid, ipmc_req->ip_version, vid,
                                   &intf_group_entry.group_addr,
                                   &group_srclist_entry) == VTSS_OK) {
                            if (!group_srclist_entry.cntr) {
                                break;
                            }

                            memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                            if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                                continue;
                            }

                            smir = sfm_src.sfm_in_hw;
                            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(ulong));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            vid,
                                            misc_ipv4_txt(htonl(ip4grp), buf),
                                            buf1,
                                            buf4,
                                            amir ? "Y/" : "N/",
                                            amir ? (smir ? "Y " : "N*") : (smir ? "Y*" : "N*"),
                                            iport + 1);
                                }

                                if (fto == -1) {
                                    CPRINTF("Allow Source Address (Timer->%d): %-40s\n",
                                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                            misc_ipv4_txt(htonl(ip4src), buf2));
                                } else {
                                    CPRINTF("Request Source Address (Timer->%d): %-40s\n",
                                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                            misc_ipv4_txt(htonl(ip4src), buf2));
                                }
                            } else {
                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            vid,
                                            misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                            buf1,
                                            buf4,
                                            amir ? "Y/" : "N/",
                                            amir ? (smir ? "Y " : "N*") : (smir ? "Y*" : "N*"),
                                            iport + 1);
                                }

                                if (fto == -1) {
                                    CPRINTF("Allow Source Address (Timer->%d): %-40s\n",
                                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                            misc_ipv6_txt(&sfm_src.src_ip, buf2));
                                } else {
                                    CPRINTF("Request Source Address (Timer->%d): %-40s\n",
                                            ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                            misc_ipv6_txt(&sfm_src.src_ip, buf2));
                                }
                            }
                        }

                        memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                        group_srclist_entry.type = FALSE;
                        while (ipmc_mgmt_get_next_group_srclist_info(
                                   isid, ipmc_req->ip_version, vid,
                                   &intf_group_entry.group_addr,
                                   &group_srclist_entry) == VTSS_OK) {
                            if (!group_srclist_entry.cntr) {
                                break;
                            }

                            memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                            if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                                continue;
                            }

                            deny_found = TRUE;

                            smir = sfm_src.sfm_in_hw;
                            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(ulong));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            vid,
                                            misc_ipv4_txt(htonl(ip4grp), buf),
                                            buf1,
                                            buf4,
                                            amir ? "Y/" : "N/",
                                            amir ? (smir ? "Y " : "N*") : (smir ? "Y*" : "N*"),
                                            iport + 1);
                                }
                                CPRINTF("Deny Source Address (Timer->%d): %-40s\n",
                                        ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                        misc_ipv4_txt(htonl(ip4src), buf2));
                            } else {
                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            vid,
                                            misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                            buf1,
                                            buf4,
                                            amir ? "Y/" : "N/",
                                            amir ? (smir ? "Y " : "N*") : (smir ? "Y*" : "N*"),
                                            iport + 1);
                                }
                                CPRINTF("Deny Source Address (Timer->%d): %-40s\n",
                                        ipmc_lib_mgmt_calc_delta_time(isid, &sfm_src.tmr.delta_time.v[iport]),
                                        misc_ipv6_txt(&sfm_src.src_ip, buf2));
                            }
                        }

                        if (!deny_found) {
                            if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group_entry.db, iport)) {
                                if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                                    memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(ulong));

                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                                vid,
                                                misc_ipv4_txt(htonl(ip4grp), buf),
                                                buf1,
                                                buf4,
                                                amir ? "Y/" : "N/",
                                                amir ? "N " : "N*",
                                                iport + 1);
                                    }
                                } else {
                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                                vid,
                                                misc_ipv6_txt(&intf_group_entry.group_addr, buf),
                                                buf1,
                                                buf4,
                                                amir ? "Y/" : "N/",
                                                amir ? "N " : "N*",
                                                iport + 1);
                                    }
                                }
                                CPRINTF("Deny Source Address: None\n");
                            }
                        }
                    }
                }

                grpCnt++;
            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
        }

        CPRINTF("\nTotal SFM Group Count for ISID-%d: %u\n", isid, grpCnt);
    }
}

static void cli_cmd_debug_ipmc_pkt_send(cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    port_iter_t     pit;
    vtss_uport_no_t uport;
    vtss_port_no_t  iport;
    vtss_ipv6_t     dst6;
    u32             dst4;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    memset(&dst6, 0x0, sizeof(vtss_ipv6_t));
    if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
        dst4 = htonl(req->ipv4_addr);
        memcpy(&dst6.addr[12], (u8 *)&dst4, sizeof(u32));
    } else {
        memcpy(&dst6, &req->ipv6_addr, sizeof(vtss_ipv6_t));
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;

        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0) {
            continue;
        }

        if (ipmc_debug_pkt_send(ipmc_req->pkt_type, ipmc_req->ip_version, req->vid, &dst6, iport, req->value, FALSE)) {
            CPRINTF("iPort-%u: ipmc_debug_pkt_send OK\n\r", iport);
        } else {
            CPRINTF("iPort-%u: ipmc_debug_pkt_send NG\n\r", iport);
        }
    }
}

static void cli_cmd_show_ipmc_status(cli_req_t *req)
{
    switch_iter_t                   sit;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;
    uchar                           intf_state, dummy;
    ushort                          vid;
    ipmc_prot_intf_entry_param_t    intf_param;
    BOOL                            first, ipmc_mode;
    ipmc_cli_req_t                  *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) {
#ifndef VTSS_SW_OPTION_SMB_IPMC
        ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
        CPRINTF("Please specify IPMC version: [mld|igmp]\n");
        return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    }

    if (ipmc_mgmt_get_mode(&ipmc_mode, ipmc_req->ip_version) != VTSS_OK) {
        ipmc_mode = FALSE;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = TRUE;
        vid = (req->vid_spec == CLI_SPEC_VAL ? (req->vid - 1) : 0);
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &intf_state, &dummy, TRUE, ipmc_req->ip_version) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                break;
            }

            /* Don't show disabled interfaces, since no meaningful running info */
            if (!intf_state) {
                continue;
            }

            memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
            if (ipmc_mgmt_get_intf_info(isid, vid, &intf_param, ipmc_req->ip_version) != VTSS_OK) {
                continue;
            }
            if (first) {
                first = 0;
                cli_cmd_usid_print(usid, req, 0);

                if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                    CPRINTF("      Querier  Rx         Tx         Rx         Rx         Rx         Rx\n");
                    CPRINTF("VID   Status   Query      Query      V1 Join    V2 Join    V3 Join    V2 Leave\n");
                    CPRINTF("----  -------  ---------- ---------- ---------- ---------- ---------- ----------\n");
                } else {
                    CPRINTF("      Querier  Rx         Tx         Rx         Rx         Rx\n");
                    CPRINTF("VID   Status   Query      Query      V1 Report  V2 Report  V1 Done\n");
                    CPRINTF("----  -------  ---------- ---------- ---------- ---------- ----------\n");
                }
            }

            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                CPRINTF("%-4d  %-7s  %-10d %-10d %-10d %-10d %-10d %-10d\n",
                        vid,
                        (!ipmc_mode || !intf_state) ? "DISABLE" : (intf_param.querier.state == IPMC_QUERIER_IDLE ? "IDLE" : "ACTIVE"),
                        intf_param.stats.igmp_queries,
                        intf_param.querier.ipmc_queries_sent,
                        intf_param.stats.igmp_v1_membership_join,
                        intf_param.stats.igmp_v2_membership_join,
                        intf_param.stats.igmp_v3_membership_join,
                        intf_param.stats.igmp_v2_membership_leave);
            } else {
                CPRINTF("%-4d  %-7s  %-10d %-10d %-10d %-10d %-10d\n",
                        vid,
                        (!ipmc_mode || !intf_state) ? "DISABLE" : (intf_param.querier.state == IPMC_QUERIER_IDLE ? "IDLE" : "ACTIVE"),
                        intf_param.stats.mld_queries,
                        intf_param.querier.ipmc_queries_sent,
                        intf_param.stats.mld_v1_membership_report,
                        intf_param.stats.mld_v2_membership_report,
                        intf_param.stats.mld_v1_membership_done);
            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
        }
    }
}

static void cli_cmd_show_ipmc_version(cli_req_t *req)
{
    switch_iter_t                   sit;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;
    uchar                           intf_state, dummy;
    ushort                          vid;
    BOOL                            first;
    ipmc_intf_query_host_version_t  vlan_version_entry;
    ipmc_cli_req_t                  *ipmc_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmc_req) {
        return;
    }

    if (ipmc_req->ip_version == IPMC_IP_VERSION_ALL) {
#ifndef VTSS_SW_OPTION_SMB_IPMC
        ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
#else
        CPRINTF("Please specify IPMC version: [mld|igmp]\n");
        return;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = TRUE;
        vid = (req->vid_spec == CLI_SPEC_VAL ? (req->vid - 1) : 0);
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &intf_state, &dummy, TRUE, ipmc_req->ip_version) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && vid != req->vid) {
                break;
            }

            /* Don't show disabled interfaces, since no meaningful running info */
            if (!intf_state) {
                continue;
            }

            memset(&vlan_version_entry, 0x0, sizeof(ipmc_intf_query_host_version_t));
            vlan_version_entry.vid = vid;
            if (ipmc_mgmt_get_intf_version(isid, &vlan_version_entry, ipmc_req->ip_version) != VTSS_OK) {
                continue;
            }

            if (first) {
                first = 0;
                cli_cmd_usid_print(usid, req, 0);
                CPRINTF("VID   Query Version  Host Version\n");
                CPRINTF("----  -------------  ------------\n");
            }
            CPRINTF("%-4d", vlan_version_entry.vid);

            switch ( ipmc_req->ip_version ) {
            case IPMC_IP_VERSION_IGMP:
                if (vlan_version_entry.query_version == VTSS_IPMC_IGMP_VERSION1) {
                    CPRINTF("  %-13s", "Version 1");
                } else if (vlan_version_entry.query_version == VTSS_IPMC_IGMP_VERSION2) {
                    CPRINTF("  %-13s", "Version 2");
                } else if (vlan_version_entry.query_version == VTSS_IPMC_IGMP_VERSION3) {
                    CPRINTF("  %-13s", "Version 3");
                } else {
                    CPRINTF("  %-13s", "DEFAULT");
                }

                if (vlan_version_entry.host_version == VTSS_IPMC_IGMP_VERSION1) {
                    CPRINTF("  %-12s\n", "Version 1");
                } else if (vlan_version_entry.host_version == VTSS_IPMC_IGMP_VERSION2) {
                    CPRINTF("  %-12s\n", "Version 2");
                } else if (vlan_version_entry.host_version == VTSS_IPMC_IGMP_VERSION3) {
                    CPRINTF("  %-12s\n", "Version 3");
                } else {
                    CPRINTF("  %-12s\n", "DEFAULT");
                }

                break;
            case IPMC_IP_VERSION_MLD:
                if (vlan_version_entry.query_version == VTSS_IPMC_MLD_VERSION1) {
                    CPRINTF("  %-13s", "Version 1");
                } else if (vlan_version_entry.query_version == VTSS_IPMC_MLD_VERSION2) {
                    CPRINTF("  %-13s", "Version 2");
                } else {
                    CPRINTF("  %-13s", "DEFAULT");
                }

                if (vlan_version_entry.host_version == VTSS_IPMC_MLD_VERSION1) {
                    CPRINTF("  %-12s\n", "Version 1");
                } else if (vlan_version_entry.host_version == VTSS_IPMC_MLD_VERSION2) {
                    CPRINTF("  %-12s\n", "Version 2");
                } else {
                    CPRINTF("  %-12s\n", "DEFAULT");
                }

                break;
            default:
                break;
            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
        }
    }
}

static int32_t
cli_ipmc_intf_ifid_parse(char *cmd, char *cmd2,
                         char *stx, char *cmd_org,
                         cli_req_t *req)
{
    int error = cli_parm_parse_vid(cmd, cmd2, stx, cmd_org, req);

    if (error) {
        return (cli_parse_wc(cmd, &req->vid_spec));
    }

    return error;
}

static int32_t
cli_ipmc_intf_vlan_parse(char *cmd, char *cmd2,
                         char *stx, char *cmd_org,
                         cli_req_t *req)
{
    return cli_parm_parse_vid(cmd, cmd2, stx, cmd_org, req);
}

static int32_t cli_ipmc_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                      cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    char            *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && ipmc_req) {
        if (!strncmp(found, "add", 3)) {
            ipmc_req->op = IPMC_OP_ADD;
        } else if (!strncmp(found, "del", 3)) {
            ipmc_req->op = IPMC_OP_DEL;
        } else if (!strncmp(found, "upd", 3)) {
            ipmc_req->op = IPMC_OP_UPD;
#ifdef VTSS_SW_OPTION_SMB_IPMC
        } else if (!strncmp(found, "mld", 3)) {
            ipmc_req->ip_version = IPMC_IP_VERSION_MLD;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        } else if (!strncmp(found, "igmp", 4)) {
            ipmc_req->ip_version = IPMC_IP_VERSION_IGMP;
        } else {
#ifdef VTSS_SW_OPTION_SMB_IPMC
            if (strncmp(found, "range", 5)) {
                found = NULL;
            }
#else
            found = NULL;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        }
    }

    return (found == NULL ? 1 : 0);
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static int32_t
cli_ipmc_ssm_range_parse_keyword(char *cmd, char *cmd2,
                                 char *stx, char *cmd_org,
                                 cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    ulong           error = 0;

    req->parm_parsed = 1;
    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (ipmc_req) {
        switch ( ipmc_req->ip_version ) {
        case IPMC_IP_VERSION_MLD:
            if ((req->int_values[0] < IPMC_ADDR_V6_MIN_BIT_LEN) ||
                (req->int_values[0] > IPMC_ADDR_V6_MAX_BIT_LEN)) {
                error = 1;
            }

            break;
        case IPMC_IP_VERSION_IGMP:
            if ((req->int_values[0] < IPMC_ADDR_V4_MIN_BIT_LEN) ||
                (req->int_values[0] > IPMC_ADDR_V4_MAX_BIT_LEN)) {
                error = 1;
            }

            break;
        default:
            error = 1;

            break;
        }
    }

    return error;
}

static int32_t
cli_ipmc_port_throttle_parse_keyword(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org,
                                     cli_req_t *req)
{
    int32_t error = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 0, 10);

    return error;
}

static int32_t
cli_ipmc_parse_param_pri(char *cmd, char *cmd2,
                         char *stx, char *cmd_org,
                         cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_PRI;
        }
    }

    if ((req->int_values[0] < 0) || (req->int_values[0] > 7)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_rv(char *cmd, char *cmd2,
                        char *stx, char *cmd_org,
                        cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_RV;
        }
    }

    if ((req->int_values[0] < 1) || (req->int_values[0] > 255)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_qi(char *cmd, char *cmd2,
                        char *stx, char *cmd_org,
                        cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_QI;
        }
    }

    if ((req->int_values[0] < 1) || (req->int_values[0] > 31744)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_qri(char *cmd, char *cmd2,
                         char *stx, char *cmd_org,
                         cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_QRI;
        }
    }

    if ((req->int_values[0] < 0) || (req->int_values[0] > 31744)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_llqi(char *cmd, char *cmd2,
                          char *stx, char *cmd_org,
                          cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_LMQI;
        }
    }

    if ((req->int_values[0] < 0) || (req->int_values[0] > 31744)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_uri(char *cmd, char *cmd2,
                         char *stx, char *cmd_org,
                         cli_req_t *req)
{
    int     error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = IPMC_DEF_INTF_URI;
        }
    }

    if ((req->int_values[0] < 0) || (req->int_values[0] > 31744)) {
        error = 1;
    }

    return error;
}

static int32_t
cli_ipmc_parse_param_compat(char *cmd, char *cmd2,
                            char *stx, char *cmd_org,
                            cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    char            *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && ipmc_req) {
        if (!strncmp(found, "auto", 4)) {
            if ((ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) ||
                (ipmc_req->ip_version == IPMC_IP_VERSION_MLD)) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_AUTO;
            } else {
                found = NULL;
            }
        } else if (!strncmp(found, "v1", 2)) {
            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_OLD;
            } else if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_GEN;
            } else {
                found = NULL;
            }
        } else if (!strncmp(found, "v2", 2)) {
            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_GEN;
            } else if (ipmc_req->ip_version == IPMC_IP_VERSION_MLD) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_SFM;
            } else {
                found = NULL;
            }
        } else if (!strncmp(found, "v3", 2)) {
            if (ipmc_req->ip_version == IPMC_IP_VERSION_IGMP) {
                ipmc_req->compatibility = VTSS_IPMC_COMPAT_MODE_SFM;
            } else {
                found = NULL;
            }
        } else {
            found = NULL;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_ipmc_parse_profile_name(char *cmd, char *cmd2,
                                           char *stx,
                                           char *cmd_org,
                                           cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req;
    BOOL            has_alpha;
    int             i, len;

    if (cli_parse_text(cmd_org, req->parm, VTSS_IPMC_NAME_MAX_LEN)) {
        return 1;
    }

    req->parm_parsed = 1;
    len = strlen(req->parm);
    if (len >= VTSS_IPMC_NAME_MAX_LEN) {
        return 1;
    }

    has_alpha = FALSE;
    for (i = 0; i < len; i++) {
        if (isalpha(req->parm[i])) {
            has_alpha = TRUE;
        } else {
            if (!isdigit(req->parm[i])) {
                if (req->parm[i] != 32) {
                    CPRINTF("Only space/blank, alphabets and numbers are allowed in profile name.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        CPRINTF("At least one alphabet must be present.\n");
        return 1;
    }

    ipmc_req = req->module_req;
    strncpy(ipmc_req->profile_name, req->parm, len);

    return 0;
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static int32_t
cli_ipmc_grp_addr_parse_keyword(char *cmd, char *cmd2,
                                char *stx, char *cmd_org,
                                cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    ulong           error = 0;

    req->parm_parsed = 1;

    if (ipmc_req) {
        switch ( ipmc_req->ip_version ) {
#ifdef VTSS_SW_OPTION_SMB_IPMC
        case IPMC_IP_VERSION_MLD:
            error = cli_parse_ipmcv6_addr(cmd, &req->ipv6_addr, &req->ipv6_addr_spec);
            break;
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        case IPMC_IP_VERSION_IGMP:
            error = cli_parse_ipmcv4_addr(cmd, &req->ipv4_addr, &req->ipv4_addr_spec);
            break;
        default:
            error = 1;
            break;
        }
    }

    return error;
}

static int32_t
cli_ipmc_parse_pkt_type(char *cmd, char *cmd2,
                        char *stx, char *cmd_org,
                        cli_req_t *req)
{
    ipmc_cli_req_t  *ipmc_req = req->module_req;
    ulong           error = 0;

    req->parm_parsed = 1;
    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (ipmc_req) {
        switch ( ipmc_req->ip_version ) {
        case IPMC_IP_VERSION_IGMP:
            if (req->int_values[0] == 0) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_GQ;
            } else if (req->int_values[0] == 1) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_SQ;
            } else if (req->int_values[0] == 2) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_SSQ;
            } else if (req->int_values[0] == 3) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_V1JOIN;
            } else if (req->int_values[0] == 4) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_V2JOIN;
            } else if (req->int_values[0] == 5) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_V3JOIN;
            } else if (req->int_values[0] == 6) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_IGMP_LEAVE;
            } else {
                error = 1;
            }

            break;
        case IPMC_IP_VERSION_MLD:
            if (req->int_values[0] == 0) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_GQ;
            } else if (req->int_values[0] == 1) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_SQ;
            } else if (req->int_values[0] == 2) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_SSQ;
            } else if (req->int_values[0] == 3) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_V1REPORT;
            } else if (req->int_values[0] == 4) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_V2REPORT;
            } else if (req->int_values[0] == 6) {
                ipmc_req->pkt_type = IPMC_PKT_TYPE_MLD_DONE;
            } else {
                error = 1;
            }

            break;
        default:
            error = 1;
            break;
        }
    }

    return error;
}

static int32_t
cli_ipmc_parse_pkt_cntr(char *cmd, char *cmd2,
                        char *stx, char *cmd_org,
                        cli_req_t *req)
{
    ulong   error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 1, 255);

    return error;
}

cli_parm_t ipmc_parm_table[] = {
    {
#ifdef VTSS_SW_OPTION_SMB_IPMC
        "mld|igmp",
        "\nmld : IPMC for IPv6 MLD \n"
        "igmp: IPMC for IPv4 IGMP \n",
#else
        "igmp",
        "\nigmp: IPMC for IPv4 IGMP \n",
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        CLI_PARM_FLAG_NONE,
        cli_ipmc_parse_keyword,
        NULL
    },

    {
        "enable|disable",
        "enable : Enable IPMC snooping \n"
        "disable: Disable IPMC snooping \n"
        "(default: Show global IPMC snooping mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_mode
    },

    {
        "vid",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_vlan_parse,
        cli_cmd_ipmc_vlan_op_add
    },
    {
        "vid",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_vlan_parse,
        cli_cmd_ipmc_vlan_op_del
    },

    {
        "enable|disable",
        "enable : Enable MLD snooping\n"
        "disable: Disable MLD snooping\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_vlan_state
    },
    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_vlan_state
    },

    {
        "enable|disable",
        "enable  : Enable IPMC querier\n"
        "disable : Disable IPMC querier\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_vlan_querier
    },
    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_vlan_querier
    },

    {
        "enable|disable",
        "enable  : Enable IPMC fast leave\n"
        "disable : Disable IPMC fast leave\n"
        "(default: Show IPMC fast leave mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_port_fastleave
    },

    {
        "enable|disable",
        "enable  : Enable IPMC router port\n"
        "disable : Disable IPMC router port\n"
        "(default: Show IPMC router port mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_rtr_ports
    },

    {
        "enable|disable",
        "enable : Enable IPMC flooding\n"
        "disable: Disable IPMC flooding\n"
        "(default: Show IPMC flooding mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_unreg_flood
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_show_ipmc_status
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_show_ipmc_groups
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_show_ipmc_version
    },

#ifdef VTSS_SW_OPTION_SMB_IPMC
    {
        "enable|disable",
        "enable : Enable IPMC Leave Proxy\n"
        "disable: Disable IPMC Leave Proxy\n"
        "(default: Show IPMC Leave Proxy mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_leave_proxy
    },

    {
        "enable|disable",
        "enable : Enable IPMC Proxy\n"
        "disable: Disable IPMC Proxy\n"
        "(default: Show IPMC Proxy mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmc_proxy
    },

    {
        "range",
        "SSM Range keyword",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_parse_keyword,
        NULL,
    },
    {
        "<prefix>",
        "IPv4/IPv6 multicast group address, accordingly",
        CLI_PARM_FLAG_SET,
        cli_ipmc_grp_addr_parse_keyword,
        cli_cmd_ipmc_ssm_range
    },
    {
        "<mask_len>",
        "Mask length for IPv4(4 ~ 32)/IPv6(8 ~ 128) ssm range, accordingly\n",
        CLI_PARM_FLAG_SET,
        cli_ipmc_ssm_range_parse_keyword,
        cli_cmd_ipmc_ssm_range
    },

    {
        "limit_group_number",
        "0       : No limit\n"
        "1~10    : Group learn limit\n"
        "(default: Show IPMC Port Throttling)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_ipmc_port_throttle_parse_keyword,
        cli_cmd_ipmc_port_throttle
    },

    {
        "add|del|upd",
        "add  : Associate IPMC profile for designated ports\n"
        "del  : Remove IPMC profile for designated ports\n"
        "upd  : Update IPMC profile for designated ports\n"
        "(default: Show IPMC port group filtering list)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_ipmc_parse_keyword,
        cli_cmd_ipmc_group_filtering
    },
    {
        "profile_name",
        "IPMC filtering profile name (Maximum of 16 characters)\n"
        "profile_name: Identifier of the designated profile",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_ipmc_parse_profile_name,
        cli_cmd_ipmc_group_filtering
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_show_ipmc_sfm_info
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_pri
    },
    {
        "ipmc_param_pri",
        "\n-1      : Default Value (0)\n"
        "0~7     : Interface Priority\n"
        "(default: Show IPMC Interface Priority)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_pri,
        cli_cmd_ipmc_param_pri
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_rv
    },
    {
        "ipmc_param_rv",
        "\n-1      : Default Value (2)\n"
        "1~255   : Robustness Variable\n"
        "(default: Show IPMC Interface Robustness Variable)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_rv,
        cli_cmd_ipmc_param_rv
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_qi
    },
    {
        "ipmc_param_qi",
        "\n-1      : Default Value (125)\n"
        "1~31744 : Query Interval in seconds\n"
        "(default: Show IPMC Interface Query Interval)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_qi,
        cli_cmd_ipmc_param_qi
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_qri
    },
    {
        "ipmc_param_qri",
        "\n-1      : Default Value (100)\n"
        "0~31744 : Query Response Interval in tenths of seconds\n"
        "(default: Show IPMC Interface Query Response Interval)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_qri,
        cli_cmd_ipmc_param_qri
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_llqi
    },
    {
        "ipmc_param_llqi",
        "\n-1      : Default Value (10)\n"
        "0~31744 : Last Listener Query Interval in tenths of seconds\n"
        "(default: Show IPMC Interface Last Listener Query Interval)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_llqi,
        cli_cmd_ipmc_param_llqi
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_uri
    },
    {
        "ipmc_param_uri",
        "\n-1      : Default Value (1)\n"
        "0~31744 : Unsolicited Report Interval in seconds\n"
        "(default: Show IPMC Interface Unsolicited Report Interval)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_uri,
        cli_cmd_ipmc_param_uri
    },

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_ipmc_param_compat
    },
    {
        "auto|v1|v2|v3",
        "\nauto    : Auto Compatibility (Default Value)\n"
        "v1      : Forced Compatibility of IGMPv1 or MLDv1\n"
        "v2      : Forced Compatibility of IGMPv2 or MLDv2\n"
        "v3      : Forced Compatibility of IGMPv3\n"
        "(default: Show IPMC Interface Compatibility)",
        CLI_PARM_FLAG_SET,
        cli_ipmc_parse_param_compat,
        cli_cmd_ipmc_param_compat
    },
#endif /* VTSS_SW_OPTION_SMB_IPMC */

    {
        "<vid>",
        "VLAN ID (1-4095) or 'any', default: Show all VLANs",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_intf_ifid_parse,
        cli_cmd_debug_ipmc_pkt_send
    },
    {
        "<ipmc_pkt_type>",
        "Control Packet Type: 0(GQ) 1(SQ) 2(SSQ) 3(V1JOIN) 4(V2JOIN) 5(V3JOIN) 6(DONE)",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_ipmc_parse_pkt_type,
        cli_cmd_debug_ipmc_pkt_send
    },
    {
        "<ipmc_pkt_cnt>",
        "Number of TX; At Min.=1 Max.=255",
        CLI_PARM_FLAG_NONE,
        cli_ipmc_parse_pkt_cntr,
        cli_cmd_debug_ipmc_pkt_send
    },
    {
        "group_addr",
        "IPv4/IPv6 multicast group address, accordingly",
        CLI_PARM_FLAG_SET,
        cli_ipmc_grp_addr_parse_keyword,
        cli_cmd_debug_ipmc_pkt_send
    },

    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

enum {
    PRIO_IPMC_CONF = PRIO_IPMC_LIB_CLI_END + 1,
    PRIO_IPMC_MODE,
    PRIO_IPMC_UNREG_FLOOD,
    PRIO_IPMC_LEAVE_PROXY,
    PRIO_IPMC_PROXY,
    PRIO_IPMC_SSM_RANGE,

    PRIO_IPMC_VLAN_OP_ADD,
    PRIO_IPMC_VLAN_OP_DEL,
    PRIO_IPMC_VLAN_STATE,
    PRIO_IPMC_VLAN_QUERIER,
    PRIO_IPMC_VLAN_COMPAT,

    PRIO_IPMC_PARAM_PRIORITY,
    PRIO_IPMC_PARAM_RV,
    PRIO_IPMC_PARAM_QI,
    PRIO_IPMC_PARAM_QRI,
    PRIO_IPMC_PARAM_LLQI,
    PRIO_IPMC_PARAM_URI,

    PRIO_IPMC_PORT_FASTLEAVE,
    PRIO_IPMC_PORT_THROTTLING,
    PRIO_IPMC_PORT_GROUP_FILTERING,
    PRIO_IPMC_RPORTS,

    PRIO_IPMC_STATUS,
    PRIO_IPMC_GROUPS,
    PRIO_IPMC_VERSION,
    PRIO_IPMC_SFM_INFO,

    PRIO_IPMC_DEBUG = CLI_CMD_SORT_KEY_DEFAULT
};

/* Command table entries */
#ifndef VTSS_SW_OPTION_SMB_IPMC
cli_cmd_tab_entry (
    "IPMC Configuration [igmp]",
    NULL,
    "Show IPMC snooping configuration",
    PRIO_IPMC_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_gen_config,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IPMC Mode [igmp]",
    "IPMC Mode [igmp] [enable|disable]",
    "Set or show the IPMC snooping mode",
    PRIO_IPMC_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_mode,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IPMC VLAN Add [igmp] <vid>",
    "Add the IPMC snooping VLAN interface",
    PRIO_IPMC_VLAN_OP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_op_add,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC VLAN Delete [igmp] <vid>",
    "Delete the IPMC snooping VLAN interface",
    PRIO_IPMC_VLAN_OP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_op_del,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC State [igmp] [<vid>]",
    "IPMC State [igmp] [<vid>] [enable|disable]",
    "Set or show the IPMC snooping state for VLAN",
    PRIO_IPMC_VLAN_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_state,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Querier [igmp] [<vid>]",
    "IPMC Querier [igmp] [<vid>] [enable|disable]",
    "Set or show the IPMC snooping querier mode for VLAN",
    PRIO_IPMC_VLAN_QUERIER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_querier,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Fastleave [igmp] [<port_list>]",
    "IPMC Fastleave [igmp] [<port_list>] [enable|disable]",
    "Set or show the IPMC snooping fast leave port mode",
    PRIO_IPMC_PORT_FASTLEAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_port_fastleave,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Router [igmp] [<port_list>]",
    "IPMC Router [igmp] [<port_list>] [enable|disable]",
    "Set or show the IPMC snooping router port mode",
    PRIO_IPMC_RPORTS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_rtr_ports,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Flooding [igmp]",
    "IPMC Flooding [igmp] [enable|disable]",
    "Set or show the IPMC unregistered addresses flooding operation",
    PRIO_IPMC_UNREG_FLOOD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_unreg_flood,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Status [igmp] [<vid>]",
    NULL,
    "Show IPMC operational status, accordingly",
    PRIO_IPMC_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_status,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Groups [igmp] [<vid>]",
    NULL,
    "Show IPMC group addresses, accordingly",
    PRIO_IPMC_GROUPS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_groups,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Version [igmp] [<vid>]",
    NULL,
    "Show IPMC Versions",
    PRIO_IPMC_VERSION,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_version,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug IPMC SFM [igmp] [<vid>] [<port_list>]",
    NULL,
    "Debug SFM related information in IPMC",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_sfm_info,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug IPMC PKT [igmp] <vid> <ipmc_pkt_type> <port_list> <ipmc_pkt_cnt> [group_addr]",
    "Debug Control Packet TX in IPMC",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_pkt_send,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);
#else /* VTSS_SW_OPTION_SMB_IPMC */
cli_cmd_tab_entry (
    "IPMC Configuration [mld|igmp]",
    NULL,
    "Show IPMC snooping configuration",
    PRIO_IPMC_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_gen_config,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IPMC Mode [mld|igmp]",
    "IPMC Mode [mld|igmp] [enable|disable]",
    "Set or show the IPMC snooping mode",
    PRIO_IPMC_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_mode,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IPMC VLAN Add [mld|igmp] <vid>",
    "Add the IPMC snooping VLAN interface",
    PRIO_IPMC_VLAN_OP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_op_add,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC VLAN Delete [mld|igmp] <vid>",
    "Delete the IPMC snooping VLAN interface",
    PRIO_IPMC_VLAN_OP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_op_del,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC State [mld|igmp] [<vid>]",
    "IPMC State [mld|igmp] [<vid>] [enable|disable]",
    "Set or show the IPMC snooping state for VLAN",
    PRIO_IPMC_VLAN_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_state,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Querier [mld|igmp] [<vid>]",
    "IPMC Querier [mld|igmp] [<vid>] [enable|disable]",
    "Set or show the IPMC snooping querier mode for VLAN",
    PRIO_IPMC_VLAN_QUERIER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_vlan_querier,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Fastleave [mld|igmp] [<port_list>]",
    "IPMC Fastleave [mld|igmp] [<port_list>] [enable|disable]",
    "Set or show the IPMC snooping fast leave port mode",
    PRIO_IPMC_PORT_FASTLEAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_port_fastleave,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Router [mld|igmp] [<port_list>]",
    "IPMC Router [mld|igmp] [<port_list>] [enable|disable]",
    "Set or show the IPMC snooping router port mode",
    PRIO_IPMC_RPORTS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_rtr_ports,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Flooding [mld|igmp]",
    "IPMC Flooding [mld|igmp] [enable|disable]",
    "Set or show the IPMC unregistered addresses flooding operation",
    PRIO_IPMC_UNREG_FLOOD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_unreg_flood,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Status [mld|igmp] [<vid>]",
    NULL,
    "Show IPMC operational status, accordingly",
    PRIO_IPMC_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_status,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Groups [mld|igmp] [<vid>]",
    NULL,
    "Show IPMC group addresses, accordingly",
    PRIO_IPMC_GROUPS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_groups,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Version [mld|igmp] [<vid>]",
    NULL,
    "Show IPMC Versions",
    PRIO_IPMC_VERSION,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_version,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Leave Proxy [mld|igmp]",
    "IPMC Leave Proxy [mld|igmp] [enable|disable]",
    "Set or show the mode of IPMC Leave Proxy",
    PRIO_IPMC_LEAVE_PROXY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_leave_proxy,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Proxy [mld|igmp]",
    "IPMC Proxy [mld|igmp] [enable|disable]",
    "Set or show the mode of IPMC Proxy",
    PRIO_IPMC_PROXY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_proxy,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC SSM [mld|igmp]",
    "IPMC SSM [mld|igmp] [(Range <prefix> <mask_len>)]",
    "Set or show the IPMC SSM Range",
    PRIO_IPMC_SSM_RANGE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_ssm_range,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Throttling [mld|igmp] [<port_list>]",
    "IPMC Throttling [mld|igmp] [<port_list>] [limit_group_number]",
    "Set or show the IPMC port throttling status",
    PRIO_IPMC_PORT_THROTTLING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_port_throttle,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Filtering [mld|igmp] [<port_list>]",
    "IPMC Filtering [mld|igmp] [<port_list>] [add|del|upd] [profile_name]",
    "Set or show the IPMC port group filtering list",
    PRIO_IPMC_PORT_GROUP_FILTERING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_group_filtering,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC SFM [mld|igmp] [<vid>] [<port_list>]",
    NULL,
    "Show SFM (including SSM) related information for IPMC",
    PRIO_IPMC_SFM_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_show_ipmc_sfm_info,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter Priority [mld|igmp] [<vid>]",
    "IPMC Parameter Priority [mld|igmp] [<vid>] [ipmc_param_pri]",
    "Set or show the IPMC interface priority",
    PRIO_IPMC_PARAM_PRIORITY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_pri,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter RV [mld|igmp] [<vid>]",
    "IPMC Parameter RV [mld|igmp] [<vid>] [ipmc_param_rv]",
    "Set or show the IPMC Robustness Variable",
    PRIO_IPMC_PARAM_RV,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_rv,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter QI [mld|igmp] [<vid>]",
    "IPMC Parameter QI [mld|igmp] [<vid>] [ipmc_param_qi]",
    "Set or show the IPMC Query Interval",
    PRIO_IPMC_PARAM_QI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_qi,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter QRI [mld|igmp] [<vid>]",
    "IPMC Parameter QRI [mld|igmp] [<vid>] [ipmc_param_qri]",
    "Set or show the IPMC Query Response Interval",
    PRIO_IPMC_PARAM_QRI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_qri,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter LLQI [mld|igmp] [<vid>]",
    "IPMC Parameter LLQI [mld|igmp] [<vid>] [ipmc_param_llqi]",
    "Set or show the IPMC Last Listener Query Interval",
    PRIO_IPMC_PARAM_LLQI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_llqi,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Parameter URI [mld|igmp] [<vid>]",
    "IPMC Parameter URI [mld|igmp] [<vid>] [ipmc_param_uri]",
    "Set or show the IPMC Unsolicited Report Interval",
    PRIO_IPMC_PARAM_URI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_uri,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "IPMC Compatibility [mld|igmp] [<vid>]",
    "IPMC Compatibility [mld|igmp] [<vid>] [auto|v1|v2|v3]",
    "Set or show the IPMC Compatibility",
    PRIO_IPMC_VLAN_COMPAT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC,
    cli_cmd_ipmc_param_compat,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug IPMC SFM [mld|igmp] [<vid>] [<port_list>]",
    NULL,
    "Debug SFM related information in IPMC",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_sfm_info,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug IPMC PKT [mld|igmp] <vid> <ipmc_pkt_type> <port_list> <ipmc_pkt_cnt> [group_addr]",
    "Debug Control Packet TX in IPMC",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_pkt_send,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_SMB_IPMC */

cli_cmd_tab_entry (
    NULL,
    "Debug IPMC RESET_PKT",
    "Reset IPMC PKT Statistics",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_pkt_reset,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug IPMC Statistics",
    NULL,
    "Debug IPMC Statistics",
    PRIO_IPMC_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_statistics,
    NULL,
    ipmc_parm_table,
    CLI_CMD_FLAG_NONE
);
